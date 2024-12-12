/**
 * References for programming with 'curses'
 * 
 * Curses Library Guide - https://www.ibm.com/docs/en/aix/7.1?topic=concepts-curses-library
 * Charlie's worm lab - https://curtsinger.cs.grinnell.edu/teaching/2024F/CSC213/labs/worm/
 * 
 */

#pragma once

#include <pthread.h>
#include <curses.h>
#include <stdbool.h>
#include "board.h"

/**
 * Initializes the curses environment
 */
void init_curses();

/**
 * Creates a new window to represent a player's or opponent's game board.
 * 
 * @param start_x  The starting column position for the window.
 * @param start_y  The starting row position for the window.
 * @param title    The title to display at the top of the window.
 * 
 * @return A pointer to the created window.
 */
WINDOW* create_board_window(int start_x, int start_y, const char* title);

/**
 * Draws the player's board, showing ships and their current state.
 *
 * @param win   The window where the board will be drawn.
 * @param board The player's game board array.
 */
void draw_player_board(WINDOW* win, cell_t board[NROWS + 1][NCOLS + 1]);

/**
 * Draws the opponent's board, hiding ships and showing only guesses.
 *
 * @param win   The window where the board will be drawn.
 * @param board The opponent's game board array.
 */
void draw_opponent_board(WINDOW* win, cell_t board[NROWS + 1][NCOLS + 1]);

/**
 * Creates a new window to display prompts and handle user input.
 * 
 * @param start_x  The starting column position for the window.
 * @param start_y  The starting row position for the window.
 * 
 * @return A pointer to the created window.
 */
WINDOW* create_prompt_window(int start_x, int start_y);

// /**
//  * Displays a prompt message in the given window and clears the input field.
//  * 
//  * @param win     The window where the prompt is displayed.
//  * @param message The message to display.
//  */
// void display_prompt(WINDOW* win, const char* prompt, char* input, int max_len);

// /**
//  * Start the thread to monitor and reset the prompt window 
//  * 
//  * @param prompt_win The prompt window to monitor
//  */
// void start_cursor_tracking(WINDOW* prompt_win);

// /**
//  * Stop the start_cursor_tracking thread
//  */
// void stop_cursor_tracking();

/**
 * End the curses environment
 */
void end_curses();