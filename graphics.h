/**
 * References for programming with 'curses'
 * 
 * Curses Library Guide - https://www.ibm.com/docs/en/aix/7.1?topic=concepts-curses-library
 * Charlie's worm lab - https://curtsinger.cs.grinnell.edu/teaching/2024F/CSC213/labs/worm/
 * 
 */

#pragma once

#include "board.h"
#include <curses.h>

/**
 * Initializes the curses environment
 */
void init_curses();

/**
 * Creates a new window to represent a player's or opponent's game board.
 * 
 * @param start_y  The starting row position for the window.
 * @param start_x  The starting column position for the window.
 * @param title    The title to display at the top of the window.
 * 
 * @return A pointer to the created window.
 */
WINDOW* create_board_window(int start_y, int start_x, const char* title);

/**
 * Draws the game board for a player or opponent in the given window.
 * 
 * @param win        The window where the board will be drawn.
 * @param board      The 10x10 game board array to display.
 * @param hide_ships If true, hides the ships from view (used for opponent's board).
 */
void draw_board(WINDOW* win, cell_t board[NROWS+1][NCOLS+1], bool hide_ships);

/**
 * End the curses environment
 */
void end_curses();