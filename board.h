#pragma once
#include <stdbool.h>
#include <curses.h>
#include <stddef.h>

#define NROWS 10 //rows for game board
#define NCOLS 10 //columns for game board
#define NDIFSHIPS 5 //the number of different types of ships
#define INIT_CURSOR 1 //vertical start index for the cursor of the user input window

/*
* shipType struct, stores details about a specific ship, including name, size, and sunk status.
*/
typedef struct shipType {
  char* name;
  int size;
  bool sunk;
} shipType_t;

//  Each player gets 5 ships, a destroyer of size 2, a submarine of size 3, a cruiser of size 3, 
//  a battleship of size 4, and an aircraft carrier of size 5.
extern const shipType_t shipArray[];

// Declare cursor as an external variable to prevent multiple definition errors
extern size_t cursor;
extern char * most_recent_prompt;
extern int space;

/*
* cell struct, stores details about a specific cell on the game board, including occupied status, guessed status, hit status, and the ship occupying the cell.
*/
typedef struct cell{
    bool occupied;
    bool guessed;
    bool hit;
    shipType_t ship;
} cell_t;

/*
* Returns a boolean, takes in a cell c and determines if c is a currently occupied cell.
*/
bool isOccupied(cell_t c);

/**
 * board struct, stores an 2d array of cells of size 11*11
 */
typedef struct board{
    cell_t array[NROWS+1][NCOLS+1];
}board_t;

//different possible orientations
enum Orientation {
  HORIZONTAL,
  VERTICAL,
  INVALID
};

/**
 * shipLocation struct, stores the ship, its starting indexs, its orientation, and sunk status
 */
typedef struct shipLocation{
    shipType_t shipType;
    int startx;
    int starty;
    enum Orientation orientation;
    bool sunk;
}shipLocation_t;



/**checkBounds
 *  checkBounds takes a player's proposal shipLocation (including origin, size, 
 *  and orientation) and returns true if the boundaries for the proposed ship 
 *  location are not crossed (i.e., the ship isn't off the board)
 */
bool checkBounds (struct shipLocation proposal);


/** updateBoardAfterGuess
 *  Function that updates the board based on the player's guess
 *  Takes a board, coordinates, bools giving information about the
 *  specified cell (to be updated), and the user's input window
 */
void updateBoardAfterGuess(board_t *board, int x, int y, bool *isHit, bool *isSunk, WINDOW * window);

/**
 * Function to check if all ships on a player's board have been sunk
 * 
 * @param board The player's game board
 * @return true if all ships are sunk, false otherwise.
 */
bool checkVictory(board_t* board);

// Function that initializes a players game board
void initBoard(board_t *board); 

/**printStatus
 *  used by us during debugging to print the occupation status of each cell
 */
void printStatus(board_t board, WINDOW * window, char* filename);

/**checkOverlap
 *  checkOverlap takes a player's board and proposal shipLocation.
 *  It returns true if there's an overlap present.
 *  Assumptions: proposal's coordinates and orientation are valid
 */
bool checkOverlap(board_t * board, struct shipLocation proposal);

/**validOrt
 *  validOrt takes the user input window 
 *  validOrt instructs the user to give us an orientation (either "V" or "H") and loops until 
 *  the user inputs a valid orientation. It then returns that valid orientation.
 *  Since we handle user input here, the function is mostly error checking.
 */
enum Orientation validOrt(WINDOW * window);

/**validCoords
 *  validCoords takes a pointer to an int array that it will fill with the two valid coordinate
 *  values. It also takes the user input window since it deals with user input. Since this input
 *  is more structured and complicated, most of this function is error checking.
 */
int* validCoords(int * yay, WINDOW * window, char * prompt);

/**makeBoard
 *  makeBoard takes the input window and the player's board window.
 *  makeBoard loops through all of the ships from the above shipArray and places them on the board based on
 *  user input from the helper functions seen above. It returns the initialized board on success and an empty 
 *  board on failure, but it shouldn't be able to fail.
 */
board_t makeBoard(WINDOW * window, WINDOW * playerWindow);


/**
 * Start the thread to monitor and reset the prompt window 
 * 
 * @param prompt_win The prompt window to monitor
 */
void start_cursor_tracking(WINDOW* prompt_win);

/**
 * Stop the start_cursor_tracking thread
 */
void stop_cursor_tracking();

/**
 * Start the victory tracking thread.
 * 
 * @param player1_board The board of Player 1.
 * @param player2_board The board of Player 2.
 */
void start_victory_tracking(board_t* player1_board, board_t* player2_board, WINDOW* prompt_win);

/**
 * Stop the victory tracking thread.
 */
void stop_victory_tracking();
