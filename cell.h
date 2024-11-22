#pragma once
#include <stdlib.h>
#include <stdbool.h>

// Assuming we won't need this since we can access based on array, but might be nice to have later?
// typedef struct coordinate{
//     int num;
//     char letter;
// }coordinate;

typedef struct cell{
    // coordinate coordinate;
    bool occupied;
    bool guessed;
    bool hit;
} cell;

bool isOccupied(cell c);