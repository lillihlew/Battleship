/**
 * References for programming with 'curses'
 * 
 * Curses Library Guide - https://www.ibm.com/docs/en/aix/7.1?topic=concepts-curses-library
 * Charlie's worm lab - https://curtsinger.cs.grinnell.edu/teaching/2024F/CSC213/labs/worm/
 * 
 */

#include <pthread.h>
#include "graphics.h"
#include "curses.h"

/**
 * Initializes the curses environment
 */
void init_curses() {
    initscr();           // Initialize the screen
    cbreak();            // Disable line buffering
    noecho();            // Disable echoing of input
    curs_set(1);         // Show the cursor
    keypad(stdscr, TRUE); // Enable special keys (e.g., arrows)
    mousemask(0, NULL);
}

/**
 * Creates a new window to represent a player's or opponent's game board.
 * 
 * @param start_x  The starting column position for the window.
 * @param start_y  The starting row position for the window.
 * @param title    The title to display at the top of the window.
 * 
 * @return A pointer to the created window.
 */
WINDOW* create_board_window(int start_x, int start_y, const char* title) {
    WINDOW* win = newwin(15, 35, start_x, start_y); // 10x10 grid + padding
    box(win, 0, 0);                                // Draw a border around the window
    mvwprintw(win, 0, 2, "[ %s ]", title);         // Add a title to the window
    // move(16, 0);
    wrefresh(win);                                 // Refresh the window to display it
    return win;
}


/**
 * Draws the player's board, showing ships and their current state.
 *
 * @param win   The window where the board will be drawn.
 * @param board The player's game board array.
 */
void draw_player_board(WINDOW* win, cell_t board[NROWS + 1][NCOLS + 1]) {
    //setup colors
    use_default_colors();
    initscr();
    start_color();

    //setup indentation for printing to windows
    int left_margin = 6;
    int top_margin = 2;
    int v_space_between_cells = 2;

    //make color pairs
    init_pair(COLOR_GREEN, COLOR_GREEN, -1);//Numbers & letters for coordinates
    init_pair(COLOR_RED, COLOR_RED, -1);//Hits
    init_pair(COLOR_BLUE, COLOR_BLUE, -1);//Misses
    init_pair(COLOR_YELLOW, COLOR_YELLOW, -1);//Ships

    //loop through each cell and pick its symbol and color
    for (int y = 0; y < NROWS + 1; y++) {
        for (int x = 0; x < NCOLS + 1; x++) {
            wattron(win, COLOR_PAIR(0));
            int color = 0;
            char symbol = '~'; // Default empty cell
            //Letters
            if(y==0){
                wattroff(win, COLOR_PAIR(color));
                wattron(win, COLOR_PAIR(COLOR_GREEN));
                color = COLOR_GREEN;
                if(x==0){
                    symbol = ' ';
                }else {
                    symbol = 'A'+x-1;
                }
            }else if (x==0){
                //Numbers
                wattroff(win, COLOR_PAIR(color));
                wattron(win, COLOR_PAIR(COLOR_GREEN));
                color = COLOR_GREEN;
                symbol = y+'0';
            } else if (board[x][y].guessed) {
                //hit
                if(board[x][y].hit){
                    symbol = 'H';
                    wattroff(win, COLOR_PAIR(color));
                    wattron(win, COLOR_PAIR(COLOR_RED));
                    color=COLOR_RED;
                }else{
                    //miss
                    symbol = 'M';
                    wattroff(win, COLOR_PAIR(color));
                    wattron(win, COLOR_PAIR(COLOR_BLUE));
                    color = COLOR_BLUE;
                }
            //ship
            } else if (board[x][y].occupied) {
                symbol = 'S'; // Display ship
                wattroff(win, COLOR_PAIR(color));
                wattron(win, COLOR_PAIR(COLOR_YELLOW));
                color = COLOR_YELLOW;
            }
            //':' is the character after 9 in ASCII, so this is the special handling for 10
            if (symbol==':') {
                mvwprintw(win, y + top_margin, x * v_space_between_cells + (left_margin-1), "%d  ", 10);
            } else mvwprintw(win, y + top_margin, x * v_space_between_cells + left_margin, "%c  ", symbol); // Adjust cell spacing
            wattroff(win, COLOR_PAIR(color));
        }
    }
    wrefresh(win);
}

/**
 * Draws the opponent's board, hiding ships and showing only guesses.
 *
 * @param win   The window where the board will be drawn.
 * @param board The opponent's game board array.
 */
void draw_opponent_board(WINDOW* win, cell_t board[NROWS + 1][NCOLS + 1]) {
    //setup colors
    use_default_colors();
    initscr();
    start_color();

    //setup indentation for printing to windows
    int left_margin = 6;
    int top_margin = 2;
    int v_space_between_cells = 2;

    //make color pairs
    init_pair(COLOR_GREEN, COLOR_GREEN, -1);//Numbers & letters for coordinates
    init_pair(COLOR_RED, COLOR_RED, -1);//Hits
    init_pair(COLOR_BLUE, COLOR_BLUE, -1);//Misses

    //loop through every cell
    for (int y = 0; y < NROWS + 1; y++) {
        for (int x = 0; x < NCOLS + 1; x++) {
            wattron(win, COLOR_PAIR(0));
            int color = 0;
            char symbol = '~'; // Default empty cell
            //numbers & letters of coordinates
            if(y==0){
                //letters
                wattroff(win, COLOR_PAIR(color));
                wattron(win, COLOR_PAIR(COLOR_GREEN));
                color = COLOR_GREEN;
                if(x==0){
                    symbol = ' ';
                }else {
                    symbol = 'A'+x-1;
                }
            }else if (x==0){
                //numbers
                wattroff(win, COLOR_PAIR(color));
                wattron(win, COLOR_PAIR(COLOR_GREEN));
                color = COLOR_GREEN;
                symbol = y+'0';
            } else if (board[x][y].guessed) {
                if(board[x][y].hit){
                    //hit
                    symbol = 'H';
                    wattroff(win, COLOR_PAIR(color));
                    wattron(win, COLOR_PAIR(COLOR_RED));
                    color=COLOR_RED;
                }else{
                    //miss
                    symbol = 'M';
                    wattroff(win, COLOR_PAIR(color));
                    wattron(win, COLOR_PAIR(COLOR_BLUE));
                    color = COLOR_BLUE;
                }
            } 
            //':' is the character after 9 in ASCII, so this is the special handling for 10
            if (symbol==':') {
                mvwprintw(win, y + top_margin, x * v_space_between_cells + (left_margin-1), "%d  ", 10);
            } else mvwprintw(win, y + top_margin, x * v_space_between_cells + left_margin, "%c  ", symbol); // Adjust cell spacing
            wattroff(win, COLOR_PAIR(color));
        }
    }
    wrefresh(win);
}

/**
 * Creates a new window to display prompts and handle user input.
 * 
 * @param start_x  The starting column position for the window.
 * @param start_y  The starting row position for the window.
 * 
 * @return A pointer to the created window.
 */
WINDOW* create_prompt_window(int start_x, int start_y) {
    WINDOW* win = newwin(30, 74, start_x, start_y);
    box(win, 0, 0);
    mvwprintw(win, 0, 2, "[ Prompt ]");
    wrefresh(win);
    return win;
}


/**
 * End the curses environment
 */
void end_curses() {
    endwin(); // End curses mode
}

// /**
//  * Displays a prompt message in the given window and clears the input field.
//  * 
//  * @param win     The window where the prompt is displayed.
//  * @param message The message to display.
//  */
// void display_prompt(WINDOW* win, const char* prompt, char* input, int max_len) {
//     werase(win);
//     box(win, 0, 0);
//     mvwprintw(win, 0, 2, "[ Prompt ]");
//     mvwprintw(win, 2, 2, "%s", prompt);
//     wrefresh(win);

//     // Get user input
//     echo();                                   // Enable input echoing
//     curs_set(1);                              // Show the cursor
//     mvwgetnstr(win, 3, 2, input, max_len);    // Get the user's input
//     noecho();                                 // Disable input echoing
//     curs_set(0);     
// }

// /**
//  * Cursor tracking thread to make sure the cursor resets
//  *      before if goes out of bounds of the prompt window
//  * 
//  * @param arg Placeholder for window
//  */
// static void* cursor_tracking(void* arg) {
//     WINDOW* prompt_win = (WINDOW*)arg;

//     int maxy, maxx;
//     getmaxyx(prompt_win, maxy, maxx);       // Get prompt window size

//     while (tracking_active) {
//         pthread_mutex_lock(&cursor_mutex);
//         if (cursor >= maxy - 1) {
//             werase(prompt_win);     // Clear the window
//             box(prompt_win, 0, 0);  // Redraw the box 
//             wrefresh(prompt_win);   // Refresh the prompt window 
//             cursor = 2;             // Reset the cursor
//         }

//         pthread_mutex_unlock(&cursor_mutex);
//     }
//     return NULL;
// }

// /**
//  * Start the thread to monitor and reset the prompt window 
//  * 
//  * @param prompt_win The prompt window to monitor
//  */
// void start_cursor_tracking(WINDOW* prompt_win) {
//     tracking_active = true;
//     pthread_create(&cursor_thread, NULL, cursor_tracking, prompt_win);
// }

// /**
//  * Stop the start_cursor_tracking thread
//  */
// void stop_cursor_tracking() {
//     tracking_active = false;
//     pthread_join(cursor_thread, NULL);
// }