#include <stdlib.h>
#include <stdbool.h>
#include "cell.h"

//Regard this as you would an ancient relic
// typedef struct cell{
//     bool occupied;
//     bool guessed;
//     bool hit;
// } cell;

#define HIT true
#define MISS false

//check if cell is occupied, true if yes false if not
bool isOccupied(cell c){
    return c.occupied;
} 

//check if cell has already been guessed, true if yes false if not
bool isGuessed(cell c){
    return c.guessed;
}

//check if cell has been hit, true if yes false if not
bool hasBeenHit(cell c){
    return c.hit;
}

/**
 * guess(cell c)
 * c: a cell from the opponent's board
 * Returns true if guess was a hit and false if the guess was a miss
 * Updates the hit field to be either hit (true) or missed (false)
 * Updates the guessed field to be true
 */
bool guess(cell c){
    //possibly modify this to guard against cell we've already guessed

    //update guessed field
    c.guessed = true;

    //check if cell is occupied
    if(isOccupied(c)){
        c.hit = HIT;
        return HIT;
    } else {
        c.hit = MISS;
        return MISS;
    }//ifelse
}//guess

