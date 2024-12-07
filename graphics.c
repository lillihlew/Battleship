/**
 * References for programming with 'curses'
 * 
 * Curses Library Guide - https://www.ibm.com/docs/en/aix/7.1?topic=concepts-curses-library
 * Charlie's worm lab - https://curtsinger.cs.grinnell.edu/teaching/2024F/CSC213/labs/worm/
 * 
 */

#include "graphics.h"

/**
 * Initializes the curses environment
 */
void init_curses() {
    initscr();           // Initialize the screen
    cbreak();            // Disable line buffering
    noecho();            // Disable echoing of input
    curs_set(0);         // Hide the cursor
    keypad(stdscr, TRUE); // Enable special keys (e.g., arrows)
}

/**
 * Creates a new window to represent a player's or opponent's game board.
 * 
 * @param start_y  The starting row position for the window.
 * @param start_x  The starting column position for the window.
 * @param title    The title to display at the top of the window.
 * 
 * @return A pointer to the created window.
 */
WINDOW* create_board_window(int start_y, int start_x, const char* title) {
    WINDOW* win = newwin(15, 35, start_y, start_x); // 10x10 grid + padding
    box(win, 0, 0);                                // Draw a border around the window
    mvwprintw(win, 0, 2, "[ %s ]", title);         // Add a title to the window
    wrefresh(win);                                 // Refresh the window to display it
    return win;
}

/**
 * Draws the game board for a player or opponent in the given window.
 * 
 * @param win        The window where the board will be drawn.
 * @param board      The 10x10 game board array to display.
 * @param hide_ships If true, hides the ships from view (used for opponent's board).
 */
void draw_board(WINDOW* win, cell_t board[10][10], bool hide_ships) {
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            char symbol = '.'; // Default empty cell
            if (board[y][x].guessed) {
                symbol = board[y][x].hit ? 'X' : 'O'; // Hit or Miss
            } else if (!hide_ships && board[y][x].occupied) {
                symbol = 'S'; // Display ship if not hidden
            }
            mvwprintw(win, y + 2, x * 3 + 2, "%c", symbol); // Adjust cell spacing
        }
    }
    wrefresh(win);
}

/**
 * End the curses environment
 */
void end_curses() {
    endwin(); // End curses mode
}