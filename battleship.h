#pragma once

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
 * 
 * @param prompt_win The curses window for displaying prompt
 */
void welcome_message(WINDOW* prompt_win);

/**
 * Function that checks to see if at any point the player enters either 'quit' or 'exit' at any point during input prompts,
 *      the game will exit normally.
 * 
 * @param prompt_win    The curses window for displaying prompts
 * @param input         The user input being read.
 * @param leave_player  The player that is trying to exit
 * @param oppo_player   The player thats just chillin
 * @param socket_fd     Socket sending the quit message
 */
void player_leave(WINDOW* prompt_win, char* input, const char* leave_player, const char* oppo_player, int socket_fd);