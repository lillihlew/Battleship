#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "cell.h"
#include "board.h"

// Function to allow a player to place ships
void placeShips(board_t *board);

char* congradulate(){
    char* message = "Congratulations! You sunk a battleship!";
    return message;
}

char* console(){
    char* message = "Oh no! Your battleship was sunk!";
    return message;
}

int main(int argc, char* argv[]) {
    // initialzie game boards for both players
    // display somesort of welcome message; game title, rules, etc.
    // player 1 place ships
    // player 2 place ships
        // will be done in unison (threads and whatnot)
    // blah blah blah main game loop
        // player turns, game/board states, confirmation of hits and misses, etc.
    // check for victory conditions after every turn so game does not run
    //  indefinetly even when a players ships have all been sunk

    // Free and do other shit to clean up and exit the game

    return 0;
}

// Function to allow a player to place ships
    // need a counter for the types of ships that are left to place,
    //      if we are doing multiple of the same ship.
void placeShips(board_t *board) {
    for (int i = 0; i < NDIFSHIPS; i++) {
        shipType_t currentShip = shipArray[i];
        shipLocation_t proposal;
        bool validPlacement = false;

        printf("Placing your %s (size %d).\n", currentShip.name, currentShip.size);

        while (!validPlacement) {
            // prompt starting coords
            // prompt ship orientation
                // set ship orientation based on user input
            
            // Set the current ship type
            proposal.shipType = currentShip;

            // check if placement is valid with 'checkBounds' and '-Overlap'

            // Valid placement
            validPlacement = true;
        }

        printf("Placed %s\n", currentShip.name);
    }
}
