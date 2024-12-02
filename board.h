#pragma once
#include <stdbool.h>

#define NROWS 10
#define NCOLS 10
#define NDIFSHIPS 5 //the number of different types of ships

typedef struct shipType {
  char* name;
  int size;
} shipType_t;

//  Each player gets 5 ships, a destroyer of size 2, a submarine of size 3, a cruiser of size 3, 
//  a battleship of size 4, and an aircraft carrier of size 5.

const shipType_t shipArray[NDIFSHIPS] = {{"Destroyer", 2} ,{"Submarine",3} ,{"Cruiser", 3} ,{"Battleship", 4} ,{"Aircraft Carrier", 5}};

typedef struct cell{
    // coordinate coordinate;
    bool occupied;
    bool guessed;
    bool hit;
    shipType_t ship;
} cell_t;

bool isOccupied(cell_t c);

typedef struct board{
    cell_t array[NROWS][NCOLS];
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




bool checkBounds (struct board board, char* ship, struct shipLocation proposal);