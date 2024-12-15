/**
 * File basis taken from Charlie's networking exercise, and refined for our context - https://curtsinger.cs.grinnell.edu/teaching/2024F/CSC213/exercises/networking/
 */

#include "battleship.h"

size_t cursor = INIT_CURSOR;
// bool win;
// bool lose;


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
    win = false;
    lose = false;
    //FILE* serverInputCoordsFile = fopen("serverInputCoordsFile.txt", "w+");
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
    sleep(1);

    // Initialize curses for graphics
    init_curses();

    // Create graphical windows for the boards
    WINDOW* player_win = create_board_window(1, 1, "Your Board");
    WINDOW* opponent_win = create_board_window(1, 40, "Opponent's Board");
    WINDOW* prompt_win = create_prompt_window(16, 1);

    cursor = INIT_CURSOR;   // Reset the cursor before tracking
    start_cursor_tracking(prompt_win);

    // Display welcome message
    welcome_message(prompt_win);

    // Initialize game boards for both players
    board_t player1_board, player2_board;
    initBoard(&player1_board);
    initBoard(&player2_board);

    // Show the empty boards to the player
    draw_player_board(player_win, player1_board.array);
    draw_opponent_board(opponent_win, player2_board.array);

    // Refresh the windows
    wrefresh(player_win);
    wrefresh(opponent_win);

    // Player 1 places ships
    mvwprintw(prompt_win, cursor++, 1, "**Place your ships**");
    wrefresh(prompt_win);
    player1_board = makeBoard(prompt_win, player_win);
    printStatus(player1_board, prompt_win, "p1Board.txt");
    // memcpy(&player1_board, (makeBoard(prompt_win, player_win)), ((sizeof(cell_t))*(NROWS+1)*(NCOLS+1)));

    // Update the player's board window
    draw_player_board(player_win, player1_board.array);

    // Notify the client that the server is ready
    send_message(client_socket_fd, "READY");
    sleep(1);

    // Wait for the client to finish placing ships
    mvwprintw(prompt_win, cursor++, 1, "Waiting for opponent to place ships...");
    wrefresh(prompt_win);

    char* message = receive_message(client_socket_fd);
    if (strcmp(message, "READY") != 0) {
        printf("Client not ready. Exiting.\n");
        free(message);
        close(client_socket_fd);
        close(server_socket_fd);
        end_curses();
        exit(EXIT_FAILURE);
    } else {
        wrefresh(prompt_win);
        mvwprintw(prompt_win, cursor++, 1, "Opponent is ready! Starting game...");
        wrefresh(prompt_win);
        free(message);
        sleep(1);
    }

    // Start victory tracking thread
    start_victory_tracking(&player1_board, &player2_board, prompt_win);

    // Main game loop
    bool game_running = true;
    while (game_running) {
        int attack_coords[2];
        int x, y;

        // Player 1's turn
        mvwprintw(prompt_win, cursor++, 1, "Your turn to attack!\n");
        free(most_recent_prompt);
        most_recent_prompt = strdup("Your turn to attack!\n");
        wrefresh(prompt_win);

        // Get attack coords from user
        free(most_recent_prompt);
        memcpy(attack_coords, validCoords(attack_coords, prompt_win, "Please input attack coordinates (ex: A,1): \0"), 2*sizeof(int));
        x = attack_coords[0];  // Row index
        y = attack_coords[1];  // Column index

        // Prepare coords to send to client
        char attack_coords_char[3];
        for (int i = 0; i < 2; i++){
            attack_coords_char[i] = (attack_coords[i] == 10) ? '0' : attack_coords[i] + '0';
        }
        attack_coords_char[2] = '\0';

        // Send attack coords to client
        send_message(client_socket_fd, attack_coords_char);



        //HERE
        //Expected format of the message:
        //[SUNKwinAircraft Carrier]
        //[rrrrwwwnnnnnnnnnnnnnnnn]
        //[sunk, hit, or miss // win or empty // name of ship hit or empty]




        // Receive result of the attack
        char* attack_result = receive_message(client_socket_fd);
        if (!attack_result) {
            perror("Failed to receive attack result");
            game_running = false;
            break;
        }

        // const char *idx = strstr(txt, pat);
            
        //     if (idx != NULL) {




        // Update the opponent's board window with the result
        player2_board.array[x][y].guessed = true;
        if (strstr(attack_result, "HIT") != NULL) {
            player2_board.array[x][y].hit = true;
            mvwprintw(prompt_win, cursor++, 1, "You hit a ship at %c,%d!", x + 'A' - 1, y);
            free(most_recent_prompt);
            int strlength = strlen("You hit a ship at  , !") + 3 + 1;
            most_recent_prompt = malloc(sizeof(char)*strlength);
            sprintf(most_recent_prompt, "You hit a ship at %c,%d!", x + 'A' - 1, y);
        }
        if (strstr(attack_result, "sunk")!= NULL) {
            player2_board.array[x][y].hit = true;
            char * sunkShipName="NULL";
            for(int i = 0; i<NDIFSHIPS; i++){
                if(strstr(attack_result, shipArray[i].name)!=NULL) {
                    sunkShipName = shipArray[i].name;
                }
            }
            mvwprintw(prompt_win, cursor++, 1, "You sunk their %s at %c,%d!", sunkShipName, x + 'A' - 1, y);
            free(most_recent_prompt);
            int strlength = strlen("You sunk their at  , !") + 3 + 1 + strlen(sunkShipName);
            most_recent_prompt = malloc(sizeof(char)*strlength);
            sprintf(most_recent_prompt, "You sunk their %s at %c,%d!", sunkShipName, x + 'A' - 1, y);
        }
        if (strstr(attack_result, "MISS") != NULL) {
            mvwprintw(prompt_win, cursor++, 1, "You missed at %c,%d.", x + 'A' - 1, y);
            free(most_recent_prompt);
            int strlength = strlen("You missed at , !") + 3 + 1;
            most_recent_prompt = malloc(sizeof(char)*strlength);
            sprintf(most_recent_prompt, "You missed at %c,%d.", x + 'A' - 1, y);
        }

        if(strstr(attack_result, "win") != NULL){
            win = true;
            FILE * serverFile = fopen("serverInputCoordsFile.txt", "w+");
            fprintf(serverFile, "received 'win' so I won\n");
            fprintf(serverFile, "win and lose: %d and %d (1 is true)\n", win, lose);
            sleep(1);
            fprintf(serverFile, "win and lose: %d and %d (1 is true)\n", win, lose);
            fclose(serverFile);
        //     // werase(prompt_win);
        //     // box(prompt_win, 0, 0);
        //     // mvwprintw(prompt_win, 1, 1, "Player 2 wins. Better luck next time!");
        //     // free(most_recent_prompt);
        //     // most_recent_prompt = strdup("Player 2 wins. Better luck next time!");
        }

        free(attack_result);
        draw_opponent_board(opponent_win, player2_board.array);
        wrefresh(prompt_win);

        // Player 2's turn
        mvwprintw(prompt_win, cursor++, 1, "Waiting for Player 2's attack...\n");
        free(most_recent_prompt);
        most_recent_prompt = strdup("Waiting for Player 2's attack...\n");
        wrefresh(prompt_win);

        // Receive attack from client
        char* enemy_attack_string = receive_message(client_socket_fd);
        if (!enemy_attack_string) {
            perror("Failed to reveive enemy attack");
            game_running = false;
            break;
        }

        // Convert received coords to ints
        int p2_attack_int[2];
        p2_attack_int[0] = (enemy_attack_string[0] == '0') ? 10 : enemy_attack_string[0] - '0';
        p2_attack_int[1] = (enemy_attack_string[1] == '0') ? 10 : enemy_attack_string[1] - '0'; 
        free(enemy_attack_string);

        // Update Player 1's board with attack results
        bool hit, sunk;
        FILE * serverFile = fopen("serverInputCoordsFile.txt", "w+");
        fprintf(serverFile, "lose value before updateBoard: %d \n", lose);
        updateBoardAfterGuess(&player1_board, p2_attack_int[0], p2_attack_int[1], &hit, &sunk, prompt_win);

        //get sunkShipName if player sunk a ship
        char* sunkShip = "NULL";
        if(sunk){
            sleep(1); //hopefully letting checkVictory update lose to true right here
            sunkShip = player1_board.array[p2_attack_int[0]][p2_attack_int[1]].ship.name;
        }

        // Send attack result to Player 2
        char result_message[25];
        snprintf(result_message, sizeof(result_message), "%s%s%s%s", hit ? "HIT" : "MISS", sunk ? " (sunk)" : "", lose ? "win" : "", sunkShip);
        fprintf(serverFile, "sending win if 1: %d \n", lose);
        fclose(serverFile);
        send_message(client_socket_fd, result_message);

        draw_player_board(player_win, player1_board.array);
        wrefresh(prompt_win);
    }

    // Stop the tracking threads
    stop_victory_tracking();
    stop_cursor_tracking();
    
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
    win = false;
    lose = false;
    //FILE* clientInputCoordsFile = fopen("clientInputCoordsFile.txt", "w+");
    // Connect to the server
    int socket_fd = socket_connect(server_name, port);
    if (socket_fd == -1) {
        perror("Failed to connect to server");
        exit(EXIT_FAILURE);
    }

    printf("Connected to Player 1!\n");
    sleep(1);

    // Initialize curses for graphics
    init_curses();

    // Create graphical windows for the boards
    WINDOW* player_win = create_board_window(1, 1, "Your Board");
    WINDOW* opponent_win = create_board_window(1, 40, "Opponent's Board");
    WINDOW* prompt_win = create_prompt_window(16, 1);

    cursor = INIT_CURSOR;   // Reset the cursor before tracking
    start_cursor_tracking(prompt_win);

    // Display welcome message
    welcome_message(prompt_win);

    // Initialize game boards for both players
    board_t player2_board, player1_board;
    initBoard(&player2_board);
    initBoard(&player1_board);

    // Show the empty boards to the player
    draw_player_board(player_win, player2_board.array);
    draw_opponent_board(opponent_win, player1_board.array);

    // Refresh the windows
    wrefresh(player_win);
    wrefresh(opponent_win);

    // Player 2 places ships
    mvwprintw(prompt_win, cursor++, 1, "**Place your ships**");
    wrefresh(prompt_win);
    // memcpy(&player2_board, (makeBoard(prompt_win, player_win)), ((sizeof(cell_t))*(NROWS+1)*(NCOLS+1)));
    player2_board = makeBoard(prompt_win, player_win);
    printStatus(player2_board, prompt_win, "p2Board.txt");

    // Update the player's board window
    draw_player_board(player_win, player2_board.array);

    // Notify the server that the client is ready
    send_message(socket_fd, "READY");
    sleep(1);

    // Wait for the server to finish placing ships
    mvwprintw(prompt_win, cursor++, 1, "Waiting for opponent to place ships...");
    wrefresh(prompt_win);

    char* message = receive_message(socket_fd);
    if (strcmp(message, "READY") != 0) {
        mvwprintw(prompt_win, cursor++, 1, "Server not ready. Exiting.\n");
        wrefresh(prompt_win);
        free(message);
        close(socket_fd);
        end_curses();
        printf("Exiting with exit failure because server was NOT ready\n.");
        printf("'%s'\n", message);
        exit(EXIT_FAILURE);
    } else {
        wrefresh(prompt_win);
        mvwprintw(prompt_win, cursor++, 1, "Opponent is ready! Starting game...");
        wrefresh(prompt_win);
        free(message);
        sleep(1);
    }

    // Start victory tracking thread
    start_victory_tracking(&player1_board, &player2_board, prompt_win);

    // Main game loop
    bool game_running = true;
    while (game_running) {
        int attack_coords[2];
        int x, y;

        // Player 1's turn
        mvwprintw(prompt_win, cursor++, 1, "Waiting for Player 1's attack...\n");
        free(most_recent_prompt);
        most_recent_prompt = strdup("Waiting for Player 1's attack...\n");
        wrefresh(prompt_win);

        // Receive enemy attack from player 1
        char* enemy_attack_string = receive_message(socket_fd);
        if (!enemy_attack_string) {
            perror("Failed ro receive enemy attack");
            game_running = false;
            break;
        }

        // Convert received coords to ints
        x = (enemy_attack_string[0] == '0') ? 10 : enemy_attack_string[0] - '0';
        y = (enemy_attack_string[1] == '0') ? 10 : enemy_attack_string[1] - '0';
        free(enemy_attack_string);

        // Update Player 2's board based on Player 1's attack
        bool hit, sunk;
        updateBoardAfterGuess(&player2_board, x, y, &hit, &sunk, prompt_win);

        char* sunkShip = "NULL";
        if(sunk){
            sleep(1); //hopefully checkVictory updates lose to true right here
            sunkShip = player2_board.array[x][y].ship.name;
        }

        // Send attack result to Player 1
        char result_message[25];
        FILE * clientFile = fopen("clientInputCoordsFile.txt", "w+");
        fprintf(clientFile, "lose value before updateBoard: %d \n", lose);
        snprintf(result_message, sizeof(result_message), "%s%s%s%s", hit ? "HIT" : "MISS", sunk ? " (sunk)" : "", lose ? "win" : "", sunkShip);
        fprintf(clientFile, "sending win if 1: %d \n", lose);
        fclose(clientFile);
        send_message(socket_fd, result_message);

        draw_player_board(player_win, player2_board.array);
        wrefresh(prompt_win);
        
        // Player 2's turn
        mvwprintw(prompt_win, cursor++, 1, "Your turn to attack\n");
        free(most_recent_prompt);
        most_recent_prompt = strdup("Your turn to attack\n");
        wrefresh(prompt_win);

        // Get attacks coords from user
        free(most_recent_prompt);
        memcpy(attack_coords, validCoords(attack_coords, prompt_win, "Please input attack coordinates (ex: A,1): \0"), 2*sizeof(int));
        x = attack_coords[0];   // Row index
        y = attack_coords[1];   // Column index
        
        // Prepare coords to send to user
        char  attack_coords_char[3];
        for (int i = 0; i < 2; i++){
            attack_coords_char[i] = (attack_coords[i] == 10) ? '0' : attack_coords[i] + '0';
        }
        attack_coords_char[2] = '\0';
        
        // Send attack to Player 1
        send_message(socket_fd, attack_coords_char);

        // Receive attack result
        char* attack_result = receive_message(socket_fd);
        if (!attack_result) {
            perror("Failed to receive attack result");
            game_running = false;
            break;
        }

        // Update opponent's board based on attack result
        player1_board.array[x][y].guessed = true;
        if (strstr(attack_result, "HIT") != NULL) {
            player1_board.array[x][y].hit = true;
            mvwprintw(prompt_win, cursor++, 1, "You hit a ship at %c,%d!", x + 'A' - 1, y);
            free(most_recent_prompt);
            int strlength = strlen("You hit a ship at  , !") + 3 + 1;
            most_recent_prompt = malloc(sizeof(char)*strlength);
            sprintf(most_recent_prompt, "You hit a ship at %c,%d!", x + 'A' - 1, y);
        } 
        if (strstr(attack_result, "sunk") != NULL) {
            player1_board.array[x][y].hit = true;
            char* shipWeSunk = "NULL";
            for(int i = 0; i<NDIFSHIPS; i++){
                if(strstr(attack_result, shipArray[i].name)!=NULL) {
                    shipWeSunk = shipArray[i].name;
                }
            }
            mvwprintw(prompt_win, cursor++, 1, "You sunk their %s at %c,%d!", shipWeSunk, x + 'A' - 1, y);
            free(most_recent_prompt);
            int strlength = strlen("You sunk their  at  , !") + 3 + 1 + strlen(shipWeSunk);
            most_recent_prompt = malloc(sizeof(char)*strlength);
            sprintf(most_recent_prompt, "You sunk their %s at %c,%d!", shipWeSunk, x + 'A' - 1, y);
        }
        if(strstr(attack_result, "MISS") != NULL) {
            mvwprintw(prompt_win, cursor++, 1, "You missed at %c,%d.", x + 'A' - 1, y);
            free(most_recent_prompt);
            int strlength = strlen("You missed at  , !") + 3 + 1;
            most_recent_prompt = malloc(sizeof(char)*strlength);
            sprintf(most_recent_prompt, "You missed at %c,%d.", x + 'A' - 1, y);
        }

        if(strstr(attack_result, "win")!= NULL){
            win = true;
            FILE * clientFile = fopen("clientInputCoordsFile.txt", "w+");
            fprintf(clientFile, "received 'win' so I won\n");
            fprintf(clientFile, "win and lose: %d and %d (1 is true)\n", win, lose);
            sleep(1);
            fprintf(clientFile, "win and lose: %d and %d (1 is true)\n", win, lose);
            fclose(clientFile);
        //     // werase(prompt_win);
        //     // box(prompt_win, 0, 0);
        //     // mvwprintw(prompt_win, 1, 1, "Player 1 wins. Better luck next time!");
        //     // free(most_recent_prompt);
        //     // most_recent_prompt = strdup("Player 1 wins. Better luck next time!");
        }

        free(attack_result);
        draw_opponent_board(opponent_win, player1_board.array);
        wrefresh(prompt_win);
    }

    // Stop the tracking threads
    stop_victory_tracking();
    stop_cursor_tracking();

    //free global most recent prompt variable
    // free(most_recent_prompt);

    // Close the connection and end curses
    close(socket_fd);
    end_curses();
}


/**
 * Display a welcome message to the players when they connect to the server.
 * 
 * @param prompt_win The curses window for displaying prompt
 */
void welcome_message(WINDOW* prompt_win) {
    wclear(prompt_win);     // Clear the prompt window
    box(prompt_win, 0, 0);  // Redraw the border
    mvwprintw(prompt_win, 1, 1, "============================================================");
    mvwprintw(prompt_win, 2, 1, "                WELCOME TO BATTLESHIP!");
    mvwprintw(prompt_win, 3, 1, "============================================================");
    mvwprintw(prompt_win, 5, 1, "Rules of the game:");
    mvwprintw(prompt_win, 6, 1, "1. Each player has a 10x10 grid to place 5 ships:");
    mvwprintw(prompt_win, 7, 1, "   - Destroyer (2 spaces)");
    mvwprintw(prompt_win, 8, 1, "   - Submarine (3 spaces)");
    mvwprintw(prompt_win, 9, 1, "   - Cruiser (3 spaces)");
    mvwprintw(prompt_win, 10, 1, "   - Battleship (4 spaces)");
    mvwprintw(prompt_win, 11, 1, "   - Aircraft Carrier (5 spaces)");
    mvwprintw(prompt_win, 13, 1, "2. Players take turns guessing coordinates to attack.");
    mvwprintw(prompt_win, 14, 1, "3. A hit will mark part of a ship as damaged.");
    mvwprintw(prompt_win, 15, 1, "   NOTE: Players do not get consecutive turns if they hit an enemy ship");
    mvwprintw(prompt_win, 16, 1, "4. A ship is sunk when all its parts are hit.");
    mvwprintw(prompt_win, 17, 1, "5. The game ends when all ships of one player are sunk.");
    mvwprintw(prompt_win, 19, 1, "Type 'exit' or 'quit' anytime to leave the game.");
    mvwprintw(prompt_win, 20, 1, "============================================================");
    mvwprintw(prompt_win, 21, 1, "                 LET THE BATTLE BEGIN!");
    mvwprintw(prompt_win, 22, 1, "============================================================");
    mvwprintw(prompt_win, 24, 1, "Press Enter to start the game...");
    wrefresh(prompt_win);       // Refresh the window to display changes
    
    // Wait for the player to press Enter
    int ch;
    do {
        ch = wgetch(prompt_win);
    } while (ch != '\n');

    // Clear the prompt window after Enter is pressed
    wclear(prompt_win);
    box(prompt_win, 0, 0);  // Redraw the border for further prompts
    wrefresh(prompt_win);
}


/**
 * Function that checks to see if at any point the player enters 'Q' at any point during input prompts,
 *      the game will exit normally.
 * 
 * @param prompt_win    The curses window for displaying prompts
 * @param input         The user input being read.
 * @param leave_player  The player that is trying to exit
 * @param oppo_player   The player thats just chillin
 * @param socket_fd     Socket sending the quit message
 */
void player_leave(WINDOW* prompt_win, char* input, const char* leave_player, const char* oppo_player, int socket_fd) {
    // Read input from the user in the prompt window
    wgetnstr(prompt_win, input, 256); // 256 is just the buffer size

    // Trim newline char if present
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
    }

    // Check if the input is 'Q'
    if (strcasecmp(input, "Q") == 0) {
        // Ask the user for confirmation
        wclear(prompt_win);
        mvwprintw(prompt_win, cursor++, 1, "Only losers rage quit. Are you sure you want to leave the game? (Y/N): ");
        wrefresh(prompt_win);

        char confirm[256];
        wgetnstr(prompt_win, confirm, 256);

        // Check confirmation response
        if (strcasecmp(confirm, "Y") == 0) {
            // If confirmed, notify both players and exit
            char quit_message[256];
            snprintf(quit_message, sizeof(quit_message), "%s rage quit. You win!", leave_player);
            send_message(socket_fd, quit_message);      // Notify the opposing player

            wclear(prompt_win);
            mvwprintw(prompt_win, cursor++, 1, "Oh well...%s Wins!", oppo_player);
            wrefresh(prompt_win);
            sleep(2);       // Pause before exiting
            end_curses();   // End the curses environment
            exit(0);        // Exit the program
        } else if (strcasecmp(confirm, "N") == 0) {
            // If not confirmed, clear the prompt and return to the last state of the game
            wclear(prompt_win);
            mvwprintw(prompt_win, cursor++, 1, "Returning to the game...");
            wrefresh(prompt_win);
            sleep(1);   // Pause for clarity
            wclear(prompt_win);
            wrefresh(prompt_win);
            return;        
        } else {
            // Handle input during confirmation
            wclear(prompt_win);
            mvwprintw(prompt_win, cursor++, 1, "Invalid response. Returning to the game...");
            wrefresh(prompt_win);
            sleep(1);   // Pause for clarity
            wclear(prompt_win);
            wrefresh(prompt_win);
            return;
        }
    } 

    // Clear the prompt window after receiving valid input
    wclear(prompt_win);
    wrefresh(prompt_win);
}

// Error: on each of the players second turn, when the player sends an attack to the other, 
//      even though there is part of a ship in that cell, the player will receive a message saying they missed
//      and display an 'M' at that cell on their opponent board. However, the player that received the attack will be 
//      properly displayed a message saying they got hit and their player board will be updated appropriately with an
//      'H' at that cell. Why is this happening?