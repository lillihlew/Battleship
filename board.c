#include <stdlib.h>
#include <stdbool.h>
#include "board.h"

//the number of different types of ships
#define NDIFSHIPS

//  Each player gets 5 ships, a destroyer of size 2, a submarine of size 3, a cruiser of size 3, 
//  a battleship of size 4, and an aircraft carrier of size 5.
struct shipType shipArray[NDIFSHIPS]={
    {"Destroyer", 2},
    {"Submarine", 3},
    {"Cruiser", 3},
    {"Battleship", 4},
    {"Aircraft Carrier", 5}
};

//when passed in, proposal should have an unitialized shipType and it will be initialized within this if bounds are valid
bool checkBounds (struct board board, char* ship, struct shipLocation proposal){
    //set as higher than number of ships to catch an error, this value should definitely change in the loop
    int index = NDIFSHIPS+1; 

    //loop through ship array
    for(int i = 0; i < NDIFSHIPS; i++){
        //save index with correct ship to capture the length
        if(strcmp(ship, shipArray[i].name) == 0){
            index = i;
        }//if
    }//for

    //check error
    if(index == NDIFSHIPS+1) {
        printf("Invalid ship name\n");
        return false;
    }//if

    //check origin is within bounds
    if (proposal.startx < 0 || proposal.startx >= NCOLS || proposal.starty < 0 || proposal.starty >= NROWS){
        return false;
    }//if

    //check orientation and then check based on that
    if (proposal.orientation == HORIZONTAL){
        //in horizontal case, check if we pass ends horizontally
        if ((proposal.startx + shipArray[index].size) > NCOLS) return false;
    }else{
        //in vertical case, check if we pass ends vertically
        if ((proposal.starty + shipArray[index].size) > NROWS) return false;
    }//ifelse

    //at this point, bounds must be valid, so initialize ship type and return true
    proposal.shipType=shipArray[index];
    proposal.sunk=false;
    return true;
}
