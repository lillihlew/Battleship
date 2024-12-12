#pragma once
#include <stdbool.h>
#include <curses.h>

#define NROWS 10
#define NCOLS 10
#define NDIFSHIPS 5 //the number of different types of ships

typedef struct shipType {
  char* name;
  int size;
} shipType_t;

//  Each player gets 5 ships, a destroyer of size 2, a submarine of size 3, a cruiser of size 3, 
//  a battleship of size 4, and an aircraft carrier of size 5.

extern const shipType_t shipArray[];

typedef struct cell{
    // coordinate coordinate;
    bool occupied;
    bool guessed;
    bool hit;
    shipType_t ship;
} cell_t;

bool isOccupied(cell_t c);

typedef struct board{
    cell_t array[NROWS+1][NCOLS+1];
}board_t;


enum Orientation {
  HORIZONTAL,
  VERTICAL,
  INVALID
};

typedef struct shipLocation{
    shipType_t shipType;
    int startx;
    int starty;
    enum Orientation orientation;
    bool sunk;
}shipLocation_t;




bool checkBounds (struct shipLocation proposal);

// Function to handle the logic of processing an attack on the opponent's board
void updateBoardAfterGuess(board_t *board, int x, int y, bool *isHit, bool *isSunk, WINDOW * window);

// Function that iterates through all cells on the board, checking for any unsunk ship parts.
//      If any cell is occupied by a ship and not marked as hit, the game is not over
bool checkVictory(board_t *board);

// Function that initializes a players game board
void initBoard(board_t *board); 

void printStatus(board_t board, WINDOW * window);

bool checkOverlap(board_t * board, struct shipLocation proposal);

enum Orientation validOrt(WINDOW * window);

int* validCoords(int * yay, WINDOW * window, char * prompt);

board_t makeBoard(WINDOW * window, WINDOW * playerWindow);

char * hitOrMiss(int attackCoordsArray[2], board_t * board);

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
void start_victory_tracking(board_t* player1_board, board_t* player2_board);

/**
 * Stop the victory tracking thread.
 */
void stop_victory_tracking();
