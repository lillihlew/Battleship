/**
 * File basis taken from Charlie's networking exercise, and refined for our context - https://curtsinger.cs.grinnell.edu/teaching/2024F/CSC213/exercises/networking/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "board.h"
#include "gameMessage.h"
#include "socket.h"
#include "graphics.h"

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
 * Display a welcome message to the players when they connect to the server.
 */
void welcome_message();

/**
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
    // Create and bind the server socket
    int server_socket_fd = server_socket_open(&port);
    if (server_socket_fd == -1) {
        perror("Failed to open server socket");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    printf("Server listening on port %u\n", port);

    if (listen(server_socket_fd, 1) == -1) {
        perror("Failed to listen on server socket");
        close(server_socket_fd);
        exit(EXIT_FAILURE);
    }

    // Accept a client connection
    int client_socket_fd = server_socket_accept(server_socket_fd);
    if (client_socket_fd == -1) {
        perror("Failed to accept client connection");
        close(server_socket_fd);
        exit(EXIT_FAILURE);
    }

    printf("Player 2 connected!\n");

    // Display welcome message
    welcome_message();

    // Wait for players to press 'Enter' to begin
    printf("\nPress Enter to start the game...\n");
    while (getchar() != '\n');

    // Initialize curses for graphics
    init_curses();

    // Create graphical windows for the boards
    WINDOW* player_win = create_board_window(1, 1, "Your Board");
    WINDOW* opponent_win = create_board_window(1, 40, "Opponent's Board");

    // Initialize game boards for both players
    board_t player1_board, player2_board;
    initBoard(&player1_board);
    initBoard(&player2_board);

    // Player 1 places ships
    printf("**Place your ships**\n");
    makeBoard(&player1_board);

    // Update the player's board window
    draw_board(player_win, player1_board.array, false);

    // Notify the client that the server is ready
    send_message(client_socket_fd, "READY");

    // Wait for the client to be ready
    char* message = receive_message(client_socket_fd);
    if (strcmp(message, "READY") != 0) {
        printf("Client not ready. Exiting.\n");
        free(message);
        close(client_socket_fd);
        close(server_socket_fd);
        end_curses();
        exit(EXIT_FAILURE);
    }
    free(message);

    // Main game loop
    bool game_running = true;
    while (game_running) {
        char attack_coords[16];
        int x, y;

        // Player 1's turn
        printf("**Enter attack coordinates (e.g., A,1)**: ");
        // validCoords(attack_coords);
        handle_input(attack_coords); // Check for 'exit' or 'quit'
        y = attack_coords[0] - 'A';  // Convert letter to row index
        x = attack_coords[1] - '1';  // Convert number to column index

        // Send attack to Player 2
        snprintf(attack_coords, sizeof(attack_coords), "%c,%c", attack_coords[0], attack_coords[1]);
        send_message(client_socket_fd, attack_coords);

        // Receive attack result
        char* attack_result = receive_message(client_socket_fd);
        printf("Player 2: %s\n", attack_result);

        // Update the opponent's board window with the result
        if (strcmp(attack_result, "HIT") == 0) {
            player2_board.array[y][x].guessed = true;
            player2_board.array[y][x].hit = true;
        } else if (strcmp(attack_result, "MISS") == 0) {
            player2_board.array[y][x].guessed = true;
        }
        draw_board(opponent_win, player2_board.array, true);

        free(attack_result);

        // Check if Player 1 won
        if (checkVictory(&player2_board)) {
            printf("Player 1 wins!\n");
            send_message(client_socket_fd, "WIN");
            break;
        }

        // Player 2's turn
        printf("Waiting for Player 2's attack...\n");
        char* enemy_attack = receive_message(client_socket_fd);
        handle_input(enemy_attack); // Check for 'exit' or 'quit'
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

        // Update the player's board window with the result
        draw_board(player_win, player1_board.array, false);

        // Check if Player 2 won
        if (checkVictory(&player1_board)) {
            printf("Player 2 wins!\n");
            send_message(client_socket_fd, "LOSE");
            break;
        }
    }

    // Close sockets and end curses
    close(client_socket_fd);
    close(server_socket_fd);
    end_curses();
}

/**
 * Initializes the client-side (Player 2) logic for the game
 * 
 * @param server_name The IP or hostname of the server
 * @param port        The port number the server is listening on.
 */
void run_client(char* server_name, unsigned short port) {
    // Connect to the server
    int socket_fd = socket_connect(server_name, port);
    if (socket_fd == -1) {
        perror("Failed to connect to server");
        exit(EXIT_FAILURE);
    }

    printf("Connected to Player 1!\n");

    // Display welcome message
    welcome_message();

    // Wait for players to press 'Enter' to begin
    printf("\nPress Enter to start the game...\n");
    while (getchar() != '\n');

    // Initialize curses for graphics
    init_curses();

    // Create graphical windows for the boards
    WINDOW* player_win = create_board_window(1, 1, "Your Board");
    WINDOW* opponent_win = create_board_window(1, 40, "Opponent's Board");

    // Initialize game boards for both players
    board_t player2_board, player1_board;
    initBoard(&player2_board);
    initBoard(&player1_board);

    // Player 2 places ships
    printf("**Place your ships**\n");
    makeBoard(&player2_board);

    // Update the player's board window
    draw_board(player_win, player2_board.array, false);

    // Notify the server that the client is ready
    send_message(socket_fd, "READY");

    // Wait for the server to be ready
    char* message = receive_message(socket_fd);
    if (strcmp(message, "READY") != 0) {
        printf("Server not ready. Exiting.\n");
        free(message);
        close(socket_fd);
        end_curses();
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
        char* enemy_attack = receive_message(socket_fd);
        handle_input(enemy_attack); // Check for 'exit' or 'quit'
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

        // Update the player's board window with the result
        draw_board(player_win, player2_board.array, false);

        // Check if Player 1 won
        if (checkVictory(&player2_board)) {
            printf("Player 1 wins!\n");
            send_message(socket_fd, "LOSE");
            break;
        }

        // Player 2's turn
        printf("**Enter attack coordinates (e.g., A,1)**: ");
        // validCoords(attack_coords);
        handle_input(attack_coords); // Check for 'exit' or 'quit'
        y = attack_coords[0] - 'A';
        x = attack_coords[1] - '1';

        // Send attack to Player 1
        snprintf(attack_coords, sizeof(attack_coords), "%c,%c", attack_coords[0], attack_coords[1]);
        send_message(socket_fd, attack_coords);

        // Receive attack result
        char* attack_result = receive_message(socket_fd);
        printf("Player 1: %s\n", attack_result);

        // Update the opponent's board window with the result
        if (strcmp(attack_result, "HIT") == 0) {
            player1_board.array[y][x].guessed = true;
            player1_board.array[y][x].hit = true;
        } else if (strcmp(attack_result, "MISS") == 0) {
            player1_board.array[y][x].guessed = true;
        }
        draw_board(opponent_win, player1_board.array, true);

        free(attack_result);

        // Check if Player 2 won
        if (checkVictory(&player1_board)) {
            printf("Player 2 wins!\n");
            send_message(socket_fd, "WIN");
            break;
        }
    }

    // Close the connection and end curses
    close(socket_fd);
    end_curses();
}

/**
 * Display a welcome message to the players when they connect to the server.
 */
void welcome_message() {
    printf("\n============================================================\n");
    printf("                WELCOME TO BATTLESHIP!\n");
    printf("============================================================\n");
    printf("Rules of the game:\n");
    printf("1. Each player has a 10x10 grid to place 5 ships:\n");
    printf("   - Destroyer (2 spaces)\n");
    printf("   - Submarine (3 spaces)\n");
    printf("   - Cruiser (3 spaces)\n");
    printf("   - Battleship (4 spaces)\n");
    printf("   - Aircraft Carrier (5 spaces)\n");
    printf("2. Players take turns guessing coordinates to attack.\n");
    printf("3. A hit will mark part of a ship as damaged.\n");
    printf("   NOTE: Players do not get consecutive turns if they hit an enemy ship\n");
    printf("4. A ship is sunk when all its parts are hit.\n");
    printf("5. The game ends when all ships of one player are sunk.\n");
    printf("\nType 'exit' or 'quit' anytime to leave the game.\n");
    printf("============================================================\n");
    printf("                 LET THE BATTLE BEGIN!\n");
    printf("============================================================\n\n");
}

/**
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