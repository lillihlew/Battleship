#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "board.h"

// Networking Constants
#define PORT 8080           // Port number for communication
#define BUFFER_SIZE 1024    // Buffer size for sending/receiving data

// Function prototypes
void handlePlayer1(int server_fd);
void handlePlayer2(const char *server_ip);
void placeShips(board_t *board); 
bool getCoordinates(char *xChar, int *yInput);

int main(int argc, char* argv[]) {
    // Check for valid arguments
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [role] [server_ip (if player 2)]\n", argv[0]);
        fprintf(stderr, "Role 1 for Player 1 (host) 2 for Player 2 (client)\n");
        exit(EXIT_FAILURE);
    }

    int role = atoi(argv[1]);   // Determine the role of the user (PLayer 1 or Player 2)

    if (role == 1) {
        // Player 1 (Host)
        int server_fd = socket(AF_INET,SOCK_STREAM, 0); // Create a socket
        if (server_fd == -1) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        //Set up the server address
        struct sockaddr_in server_addr = {0};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;   // Bind to any available network interface
        server_addr.sin_port = htons(PORT);         // Convert port number to network byte order

        // Bind the socket to the address and port
        if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Bind failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // Start listening for incoming connections
        if (listen(server_fd, 1) < 0) {
            perror("Listen failed.\n");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        printf("Player 1: waiting for Player 2 to connect...\n");
        handlePlayer1(server_fd);   // Handle Player 1's role
        close(server_fd);           // Close the server socket
    } else if (role == 2 && argc == 3) {
        // Player 2 (Client)
        const char *server_ip = argv[2];
        handlePlayer2(server_ip);       // Handle Player 2's role
    } else {
        fprintf(stderr, "Invalid arguments. Ensure Player 2 provides server IP.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

// Function to handle Player 1's logic
void handlePlayer1(int server_fd) {
    // Accept incoming connection from Player 2
    struct sockaddr_in client_addr = {0};
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0) {
        perror("Accept failed");
        return;
    }

    printf("Player 2 connected!\n");

    // Initialize boards for both players
    board_t player1Board, player2Board;
    initBoard(&player1Board);
    initBoard(&player2Board);

    // Ship Placement for Player 1
    printf("Player 1: Place your ships.\n");
    makeBoard(&player1Board);
    send(client_fd, &player1Board, sizeof(board_t), 0); // Send Player 1's board to Player 2

    // Receive Player 2's Board
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
        if (!getCoordinates(&xChar, &yInput)) continue;     // Get and validate input

        int x = xChar - ((xChar >= 'A' && xChar <= 'J') ? 'A' : 'a');       // Convert letter to index
        int y = yInput - 1;         // Convert Y-coordinate to index

        // Send attack coordinates to Player 2
        send(client_fd, &x, sizeof(int), 0);
        send(client_fd, &y, sizeof(int), 0);

        // Recieve result of attack
        recv(client_fd, buffer, BUFFER_SIZE, 0);
        printf("%s\n", buffer);

        // Check for victory
        if (strcmp(buffer, "Player 1 Wins!") == 0) {
            gameRunning = false;
            break;
        }

        // Player 2's turn
        printf("Waiting for Player 2's move...\n");
        recv(client_fd, &x, sizeof(int), 0);
        recv(client_fd, &y, sizeof(int), 0);

        // Update Player 1's board based on Player 2's attack
        bool hit, sunk;
        updateBoardAfterGuess(&player1Board, x, y, &hit, &sunk);

        // Check if Player 2 won
        if (checkVictory(&player1Board)) {
            snprintf(buffer, BUFFER_SIZE, "Player 2 Wins!");
            send(client_fd, buffer, strlen(buffer), 0);
            gameRunning = false;
        } else {
            snprintf(buffer, BUFFER_SIZE, hit ? "Hit!" : "Miss!");
            send(client_fd, buffer, strlen(buffer), 0);
        }
    }

    close(client_fd);   // Close the connection to Player 2
}

// Function to handle Player 2's logic
void handlePlayer2(const char *server_ip) {
    // Connect to Player 1
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Socket creation failed.");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to Player 1!\n");

    // Initialize game boards for both players
    board_t player2Board, player1Board;
    initBoard(&player2Board);
    initBoard(&player1Board);

    // Ship Placement for Player 2
    printf("Player 2: Place your ships.\n");
    makeBoard(&player2Board);
    send(client_fd, &player2Board, sizeof(board_t), 0); // Send Player 2's board to Player 1

    // Receive Player 1's Board
    printf("Waiting for Player 1 to place ships...\n");
    recv(client_fd, &player1Board, sizeof(board_t), 0);

    // Main Game Loop
    bool gameRunning = true;
    char buffer[BUFFER_SIZE] = {0};
    while (gameRunning) {
        // Player 1's turn
        printf("Waiting for Player 1's move...\n");
        int x, y;
        recv(client_fd, &x, sizeof(int), 0);
        recv(client_fd, &y, sizeof(int), 0);

        // Update Player 2's board based on Player 1's attack
        bool hit, sunk;
        updateBoardAfterGuess(&player2Board, x, y, &hit, &sunk);

        // Check if Player 1 won
        if (checkVictory(&player2Board)) {
            snprintf(buffer, BUFFER_SIZE, "Player 1 Wins!");
            send(client_fd, buffer, strlen(buffer), 0);
            gameRunning = false;
        } else {
            snprintf(buffer, BUFFER_SIZE, hit ? "Hit!" : "Miss!");
            send(client_fd, buffer, strlen(buffer), 0);
        }

        // Player 2's turn
        printf("Player 2: Your turn. Enter coordinates to attack (e.g., A 1).\n");
        char xChar;
        int yInput;
        if (!getCoordinates(&xChar, &yInput)) continue;     // Get and validate input

        x = xChar - ((xChar >= 'A' && xChar <= 'J') ? 'A' : 'a');       // Convert letter to index
        y = yInput - 1;         // Convert Y-coordinate to index

        // Send attack coordinates to Player 1
        send(client_fd, &x, sizeof(int), 0);
        send(client_fd, &y, sizeof(int), 0);

        // Recieve result of attack
        recv(client_fd, buffer, BUFFER_SIZE, 0);
        printf("%s\n", buffer);

        // Check for victory
        if (strcmp(buffer, "Player 2 Wins!") == 0) {
            gameRunning = false;
            break;
        }
    }

    close(client_fd);   // Close the connection to Player 1
}

// Function to get valid coordinates from the user
bool getCoordinates(char *xChar, int *yInput) {
    printf("Enter coordinates (e.g., A 1): ");
    scanf(" %c %d", xChar, yInput);

    // Validate X and Y coordinates
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
