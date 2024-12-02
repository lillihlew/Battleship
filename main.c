#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "board.h"

// Define constants for networking
#define PORT 8080
#define BUFFER_SIZE 1024

// Function prototypes
void handlePlayer1(int server_fd);
void handlePlayer2(const char *server_ip);
bool getCoordinates(char *xChar, int *yInput);

// Function to allow a player to place ships
void placeShips(board_t *board);

char* congradulate(){
    char* message = "Congratulations! You sunk a battleship!";
    return message;
}

char* console(){
    char* message = "Oh no! Your battleship was sunk!";
    return message;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [role] [server_ip (if player 2)]\n", argv[0]);
        fprintf(stderr, "Role 1 for Player 1 (host) 2 for Player 2 (client)\n");
        exit(EXIT_FAILURE);
    }

    int role = atoi(argv[1]);
    if (role == 1) {
        // Player 1 (Host)
        int server_fd = socket(AF_INET,SOCK_STREAM, 0);
        if (server_fd == -1) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in server_addr = {0};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);

        if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Bind failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        if (listen(server_fd, 1) < 0) {
            perror("Listen failed.\n");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        printf("Player 1: waiting for Player 2 to connect...\n");
        handlePlayer1(server_fd);
        close(server_fd);
    } else if (role == 2 && argc == 3) {
        // Player 2 (Client)
        const char *server_ip = argv[2];
        handlePlayer2(server_ip);
    } else {
        fprintf(stderr, "Invalid arguments. Ensure Player 2 provides server IP.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

// Function to handle Player 1's logic
void handlePlayer1(int server_fd) {
    struct sockaddr_in client_addr = {0};
    socklen_t client_addr_len = sizeof(client_addr);

    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0) {
        perror("Accept failed");
        return;
    }

    printf("Player 2 connected!\n");

    // Initialize boards
    board_t player1Board, player2Board;
    initBoard(&player1Board);
    initBoard(&player2Board);

    // Ship Placement
    printf("Player 1: Place your ships.\n");
    placeShips(&player1Board);
    send(client_fd, &player1Board, sizeof(board_t), 0);

    printf("Waiting for Player 2 to place ships...\n");
    recv(client_fd, &player2Board, sizeof(board_t), 0);

    // Main Game Loop
    bool gameRunning = true;
    char buffer[BUFFER_SIZE] = {0};
    while (gameRunning) {
        // Player 1's turn
        printf("Player 1: Your turn. Enter coordinates to attack (e.g., A 1).\n");
        char xChar;
        int yInput;
        if (!getCoordinates(&xChar, &yInput)) continue;

        int x = xChar - ((xChar >= 'A' && xChar <= 'J') ? 'A' : 'a');
        int y = yInput - 1;

        send(client_fd, &x, sizeof(int), 0);
        send(client_fd, &y, sizeof(int), 0);

        recv(client_fd, buffer, BUFFER_SIZE, 0);
        printf("%s\n", buffer);

        if (strcmp(buffer, "Player 1 Wins!") == 0) {
            gameRunning = false;
            break;
        }

        // Player 2's turn
        printf("Waiting for Player 2's move...\n");
        recv(client_fd, &x, sizeof(int), 0);
        recv(client_fd, &y, sizeof(int), 0);

        bool hit, sunk;
        updateBoardAfterGuess(&player1Board, x, y, &hit, &sunk);

        if (checkVictory(&player1Board)) {
            snprintf(buffer, BUFFER_SIZE, "Player 2 Wins!");
            send(client_fd, buffer, strlen(buffer), 0);
            gameRunning = false;
        } else {
            snprintf(buffer, BUFFER_SIZE, hit ? "Hit!" : "Miss!");
            send(client_fd, buffer, strlen(buffer), 0);
        }
    }

    close(client_fd);
}

// Function to handle Player 2's logic
void handlePlayer2(const char *server_ip) {

}

// Function to get valid coordinates from the user
bool getCoordinates(char *xChar, int *yInput) {
    printf("Enter coordinates (e.g., A 1): ");
    scanf(" %c %d", xChar, yInput);

    if ((*xChar >= 'A' && *xChar <= 'J') || (*xChar >= 'a' && *xChar <= 'j')) {
        if (*yInput >= 1 && *yInput <= 10) {
            return true;
        } else {
            printf("Invalid Y-coordinate. Please enter a number between 1 and 10.\n");
            return false;
        }
    } else {
        printf("Invalid X-coordinate. Please enter a letter between A and J.\n");
        return false;
    }
}

// ***THIS SHOULD BE IN 'board.c'***
// Function to handle the logic of processing an attack on the opponent's board
void updateBoardAfterGuess(board_t *board, int x, int y, bool *isHit, bool *isSunk) {
    *isHit = false;
    *isSunk = false;

    // Validate the coordinates
    if (x < 0 || x >= NCOLS || y < 0 || y >= NROWS) {
        printf("Invalid coordinates.\n");
        return;
    }

    cell_t *cell = &board->array[y][x]; // Access the guessed cell

    if (cell->guessed) {
        printf("This cell has already been guessed");
        return;
    }

    // Mark the cell as guessed
    cell->guessed = true;

    // Check if the cell is occupied
    if (cell->occupied) {
        *isHit = true;
        cell->hit = true;

        // Check if the entire ship has been sunk
        shipType_t *ship = &cell->ship;
        bool allCellsHit = true;

        // Scan the board for any remaining parts of this ship
        for (int i = 0; i < NROWS; i++) {
            for (int j = 0; j < NCOLS; j++) {
                if (board->array[i][j].occupied && board->array[i][j].ship.name == ship->name
                && !board->array[i][j].hit) {
                    allCellsHit = false;
                    break;
                }
            } 
            if (!allCellsHit) break;
        }

        if (allCellsHit) {
            *isSunk = true;
            printf("You sunk a %s", ship->name);
        }
    } else {
        printf("Miss!\n");
    }
}

// ***THIS SHOULD BE IN 'board.c'***
// Function that iterates through all cells on the board, checking for any unsunk ship parts.
//      If any cell is occupied by a ship and not marked as hit, the game is not over
bool checkVictory(board_t *board) {
    for (int i = 0; i < NROWS; i++) {
        for (int j = 0; j < NCOLS; j++) {
            cell_t *cell = &board->array[i][j];
            if (cell->occupied && !cell->hit) {
                return false;       // Found an unsunk ship part
            }
        }
    }
    return true;    // All ship parts are sunk
}

// ***THIS SHOULD BE IN 'board.c'***
// Function that initializes a players game board
void initBoard(board_t *board) {
    for (int i = 0; i < NROWS; i++) {
        for (int j = 0; j < NCOLS; j++) {
            board->array[i][j].occupied = false;    // No ship places
            board->array[i][j].guessed = false;     // Not guessed yet
            board->array[i][j].hit = false;     // Not hit yet
        }
    }
}
