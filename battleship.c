#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#include "board.h"
#include "gameMessage.h"
#include "socket.h"

/**
 * Initializes the server-side (Player 1) logic for the game 
 * 
 * @param port The port number the server will listen on
 */ 
void run_server(unsigned short port);

/**
 * Initializes the client-side (Player 2) logic for the game
 * 
 * @param server_name The IP or hostname of the server
 * @param port        The port number the server is listening on.
 */
void run_client(char *server_name, unsigned short port);

/**
 * **PUT THE PROTOTYPE IN 'BOARD.H'**
 * 
 * Function that checks to see if at any point the player enters either 'quit' or 'exit' at any point during input prompts,
 *      the game will exit normally.
 * 
 * @param input The user input being read.
 */
void handle_input(char* input);

int main(int argc, char *argv[]) {
    // Validate command-line arguments
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <role> [<server_name> <port>]\n", argv[0]);
        fprintf(stderr, "Role: server or client\n");
        exit(EXIT_FAILURE);
    }

    // Check if the user wants to start as a server
    if (strcmp(argv[1], "server") == 0) {
        unsigned short port = 0;    // Initialize the port
        printf("Starting server...\n");
        run_server(port);
    } 
    // Check if the user wants to start as a client
    else if (strcmp(argv[1], "client") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage for client: %s client <server_name> <port>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        char *server_name = argv[2];
        unsigned short port = atoi(argv[3]);
        printf("Connecting to server %s on port %u...\n", server_name, port);
        run_client(server_name, port);
    } 
    // Invalid role provided
    else {
        fprintf(stderr, "Invalid role. Use 'server' or 'client'.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

/**
 * Initializes the server-side (Player 1) logic for the game 
 * 
 * @param port The port number the server will listen on
 */ 
void run_server(unsigned short port) {
    int server_socket_fd = server_socket_open(&port);
    if (server_socket_fd == -1) {
        perror("Failed to open server socket");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    printf("Server listening on port %u\n", port);

    if (listen(server_socket_fd, 1) == -1) {
        perror("Failed to listen on server socket");
        exit(EXIT_FAILURE);
    }

    // Accept a client connection
    int client_socket_fd = server_socket_accept(server_socket_fd);
    if (client_socket_fd == -1) {
        perror("Failed to accept client connection");
        exit(EXIT_FAILURE);
    }

    printf("Player 2 connected!\n");

    // Initialize game boards for both players
    board_t player1_board, player2_board;
    initBoard(&player1_board);
    initBoard(&player2_board);

    // Player 1 place ships
    printf("Player 1: Place your ships.\n");
    makeBoard(&player1_board);
    send_message(client_socket_fd, "READY");    // Notify the client that the server is ready

    // Wait for the client to be ready
    char *message = receive_message(client_socket_fd);
    if (strcmp(message, "READY") != 0) {
        printf("Client not ready. Exiting.\n");
        free(message);
        close(client_socket_fd);
        close(server_socket_fd);
        exit(EXIT_FAILURE);
    }
    free(message);

    // Main game loop
    bool game_running = true;
    while (game_running) {
        char attack_coords[16];
        int x, y;

        // Player 1's turn
        printf("Player 1: Enter attack coordinates (e.g., A,1): ");
        validCoords(attack_coords);
        handle_input(attack_coords);
        y = attack_coords[0] - 'A';     // Convert letter to row index
        x = attack_coords[1] - '1';     // Convert number to column index

        // Send attack to Player 2
        snprintf(attack_coords, sizeof(attack_coords), "%c,%c", attack_coords[0], attack_coords[1]);
        send_message(client_socket_fd, attack_coords);

        // Receive attack result
        char *attack_result = receive_message(client_socket_fd);
        printf("Player 2: %s\n", attack_result);
        free(attack_result);

        // Check if Player 1 won
        if (checkVictory(&player2_board)) {
            printf("Player 1 wins!\n");
            send_message(client_socket_fd, "WIN");
            break;
        }

        // Player 2's turn
        printf("Waiting for Player 2's attack...\n");
        char *enemy_attack = receive_message(client_socket_fd);
        handle_input(enemy_attack);
        sscanf(enemy_attack, "%c,%c", &attack_coords[0], &attack_coords[1]);
        y = attack_coords[0] - 'A';
        x = attack_coords[1] - '1';
        free(enemy_attack);

        // Update Player 1's board based on Player 2's attack
        bool hit, sunk;
        updateBoardAfterGuess(&player1_board, x, y, &hit, &sunk);

        // Send attack result to Player 2
        char result_message[16];
        snprintf(result_message, sizeof(result_message), "%s%s",
                 hit ? "HIT" : "MISS", sunk ? " (sunk)" : "");
        send_message(client_socket_fd, result_message);

        // Check if Player 2 won
        if (checkVictory(&player1_board)) {
            printf("Player 2 wins!\n");
            send_message(client_socket_fd, "LOSE");
            break;
        }
    }

    // Clear sockets
    close(client_socket_fd);
    close(server_socket_fd);
}

/**
 * Initializes the client-side (Player 2) logic for the game
 * 
 * @param server_name The IP or hostname of the server
 * @param port        The port number the server is listening on.
 */
void run_client(char *server_name, unsigned short port) {
    // Connect to the server
    int socket_fd = socket_connect(server_name, port);
    if (socket_fd == -1) {
        perror("Failed to connect to server");
        exit(EXIT_FAILURE);
    }

    printf("Connected to Player 1!\n");

    // Initialize game boards for both players
    board_t player2_board, player1_board;
    initBoard(&player2_board);
    initBoard(&player1_board);

    // Player 2 place ships
    printf("Player 2: Place your ships.\n");
    makeBoard(&player2_board);
    send_message(socket_fd, "READY");       // Notify the server that the client is ready

    // Wait for the server to be ready
    char *message = receive_message(socket_fd);
    if (strcmp(message, "READY") != 0) {
        printf("Server not ready. Exiting.\n");
        free(message);
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
    free(message);

    // Main game loop
    bool game_running = true;
    while (game_running) {
        char attack_coords[16];
        int x, y;

        // Player 1's turn
        printf("Waiting for Player 1's attack...\n");
        char *enemy_attack = receive_message(socket_fd);
        handle_input(enemy_attack);
        sscanf(enemy_attack, "%c,%c", &attack_coords[0], &attack_coords[1]);
        y = attack_coords[0] - 'A';
        x = attack_coords[1] - '1';
        free(enemy_attack);

        // Update Player 2's board based on Player 1's attack
        bool hit, sunk;
        updateBoardAfterGuess(&player2_board, x, y, &hit, &sunk);

        // Send attack result to Player 1
        char result_message[16];
        snprintf(result_message, sizeof(result_message), "%s%s",
                 hit ? "HIT" : "MISS", sunk ? " (sunk)" : "");
        send_message(socket_fd, result_message);

        // Check if Player 1 won
        if (checkVictory(&player2_board)) {
            printf("Player 1 wins!\n");
            send_message(socket_fd, "LOSE");
            break;
        }

        // Player 2's turn
        printf("Player 2: Enter attack coordinates (e.g., A,1): ");
        validCoords(attack_coords);
        handle_input(attack_coords);
        y = attack_coords[0] - 'A';
        x = attack_coords[1] - '1';

        // Send attack to Player 1
        snprintf(attack_coords, sizeof(attack_coords), "%c,%c", attack_coords[0], attack_coords[1]);
        send_message(socket_fd, attack_coords);

        // Receive attack result
        char *attack_result = receive_message(socket_fd);
        printf("Player 1: %s\n", attack_result);
        free(attack_result);

        // Check if Player 2 won
        if (checkVictory(&player1_board)) {
            printf("Player 2 wins!\n");
            send_message(socket_fd, "WIN");
            break;
        }
    }

    // Close the connection
    close(socket_fd);
}

/** 
 * **PUT THIS IN 'BOARD.C'**
 * 
 * Function that checks to see if at any point the player enters either 'quit' or 'exit' at any point during input prompts,
 *      the game will exit normally.
 * 
 * @param input The user input being read.
 */
void handle_input(char* input) {
    if (strcasecmp(input, "exit") == 0 || strcasecmp(input, "quit") == 0) {
        printf("Exiting the game.\n");
        exit(0);    // Clean exit
    }
}