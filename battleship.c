#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "board.h"
#include "gameMessage.h"
#include "socket.h"

void run_server(unsigned short port);
void run_client(char *server_name, unsigned short port);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <role> [<server_name> <port>]\n", argv[0]);
        fprintf(stderr, "Role: server or client\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "server") == 0) {
        // Server role
        unsigned short port = (argc == 3) ? atoi(argv[2]) : 8080; // Default port: 8080
        printf("Starting server on port %u...\n", port);
        run_server(port);
    } else if (strcmp(argv[1], "client") == 0) {
        // Client role
        if (argc < 4) {
            fprintf(stderr, "Usage for client: %s client <server_name> <port>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        char *server_name = argv[2];
        unsigned short port = atoi(argv[3]);
        printf("Connecting to server %s on port %u...\n", server_name, port);
        run_client(server_name, port);
    } else {
        fprintf(stderr, "Invalid role. Use 'server' or 'client'.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

void run_server(unsigned short port) {
    int server_socket_fd = server_socket_open(&port);
    if (server_socket_fd == -1) {
        perror("Failed to open server socket");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %u\n", port);

    if (listen(server_socket_fd, 1) == -1) {
        perror("Failed to listen on server socket");
        exit(EXIT_FAILURE);
    }

    int client_socket_fd = server_socket_accept(server_socket_fd);
    if (client_socket_fd == -1) {
        perror("Failed to accept client connection");
        exit(EXIT_FAILURE);
    }

    printf("Client connected!\n");

    // Game logic for Player 1
    board_t player1_board, player2_board;
    initBoard(&player1_board);
    initBoard(&player2_board);

    printf("Player 1: Place your ships.\n");
    makeBoard(&player1_board);
    send_message(client_socket_fd, "READY");

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
        // Handle game logic (similar to player1.c)
        // Attack and defend logic here...
    }

    close(client_socket_fd);
    close(server_socket_fd);
}

void run_client(char *server_name, unsigned short port) {
    int socket_fd = socket_connect(server_name, port);
    if (socket_fd == -1) {
        perror("Failed to connect to server");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server!\n");

    // Game logic for Player 2
    board_t player2_board, player1_board;
    initBoard(&player2_board);
    initBoard(&player1_board);

    printf("Player 2: Place your ships.\n");
    makeBoard(&player2_board);
    send_message(socket_fd, "READY");

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
        // Handle game logic (similar to player2.c)
        // Attack and defend logic here...
    }

    close(socket_fd);
}
