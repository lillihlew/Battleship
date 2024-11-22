#pragma once
#include <stdbool.h>
#include "cell.h"

#define NROWS 10
#define NCOLS 10

typedef struct board{
    cell array[NROWS][NCOLS];
}board;

typedef struct shipLocation{
    shipType shipType;
    int startx;
    int starty;
    Orientation orientation;
    bool sunk;
}ship;

enum Orientation {
  HORIZONTAL,
  VERTICAL
};

typedef struct shipType {
  char* name;
  int size;
};


bool checkBounds (struct board board, char* ship, struct shipLocation proposal);