#pragma once
#include <stdbool.h>
#include "cell.h"

#define NROWS 10
#define NCOLS 10

typedef struct board{
    cell array[NROWS][NCOLS];
}board_t;

typedef struct shipLocation{
    shipType shipType;
    int startx;
    int starty;
    Orientation orientation;
    bool sunk;
}shipLocation_t;

enum Orientation {
  HORIZONTAL,
  VERTICAL
};

typedef struct shipType {
  char* name;
  int size;
} shipType_t;


bool checkBounds (struct board board, char* ship, struct shipLocation proposal);