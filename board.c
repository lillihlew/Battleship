#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "board.h"

//when passed in, proposal should have an unitialized shipType and it will be initialized within this if bounds are valid
bool checkBounds (struct board board, char* ship, struct shipLocation proposal){
    //set as higher than number of ships to catch an error, this value should definitely change in the loop
    int index = NDIFSHIPS+1; 

    //loop through ship array
    for(int i = 0; i < NDIFSHIPS; i++){
        //save index with correct ship to capture the length
        if(strcmp(ship, shipArray[i].name) == 0){
            index = i;
        }//ifcell_t array[NROp name\n");
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


//this returns TRUE if there is an overlap on the board

//it also updates board to occupy cells where proposal is
bool checkOverlap(board_t * board, struct shipLocation proposal){

    //initialize an array with info from board
    cell_t array[10][10];
    memcpy(array, board, sizeof(cell_t)*100);

    //start coords and info of ship
    int x = proposal.startx;
    int y = proposal.starty;
    shipType_t ship = proposal.shipType;
    enum Orientation direction = proposal.orientation;

    //horizontal
    if (direction == HORIZONTAL){
        //check if proposed locations are already occupied
        for(int i = 0; i < proposal.shipType.size; i++){
            if (array[x][y + i].occupied){
                return true;
            }
            //change cell status 
            array[x][y + i].occupied = true;
            array[x][y + i].ship = ship;
        }
    }

    //vertical
    if (direction == VERTICAL){
        //same shit as above 
        for(int i = 0; i < proposal.shipType.size; i++){
            if (array[x + i][y].occupied){
                return true;
            }
            //change cell status
            array[x + i][y].occupied = true;
            array[x + i][y].ship = ship;
        }
    }

    //update the board and return false if everything is good
    memcpy(board, array, sizeof(cell_t)*100);
    return false;
}

enum Orientation validOrt(enum Orientation bigO){
    bool lit = true;

     while (lit){
            printf("Please input orientation (V/H): ");
            char orientation[50];
            fgets(orientation, sizeof(orientation), stdin);
            enum Orientation ORT = INVALID;

            if (orientation[0] == 'H'){
                ORT = HORIZONTAL;
                lit = false;
            }
            else if(orientation[0] == 'V') {
                ORT = VERTICAL;
                lit = false;
            } else {
                printf("Wrong\n");
            }
            printf("\n");
            bigO = ORT;
        }


    return bigO;
}

char* validCoords(char * yay){
    bool supa = true;

     while (supa){
        printf("Please input coordinates (ex: A,1)");
        char coords[3];
        fgets(coords, sizeof(coords), stdin);

        if (strlen(coords) == 3){
            char letter = coords[0];
            char number = coords[2];
            bool validL = false;
            bool validN = false;
            char possibleL = 'A';
            int possibleN = 1;
            for (int i = 0; i < 10; i++){
                if (letter==possibleL) {
                    validL = true;
                    break;
                }
                possibleL++;
            }
            for (int i = 0; i < 10; i++){
                char array[2] = {number, '\0'};
                if (atoi(array)==possibleN){
                    validN = true;
                    break;
                }
                possibleN++;
            }
            if(validL && validN){
                supa=false;
                yay[0] = possibleN;
                yay[1] = possibleL;
                return yay;
            }else{
                printf("Invalid input. Try again.\n");
            }
        }
    }
    return NULL;
}

/*Places ships*/
board_t makeBoard(){
    board_t board;
    shipLocation_t proposal;

    //loop through the diff types of ships
    for (int i = 0; i < NDIFSHIPS; i++){ 
        shipType_t current = shipArray[i]; 
        enum Orientation bigO = INVALID;
        
        printf("Current Ship: %s\nShip Length: %d\n", current.name, current.size);
        bigO = validOrt(bigO);
        char coords[2];
        validCoords(coords);
        
        if(coords == NULL) perror("SOMETHING WENT WroNG");

        proposal.orientation = bigO;
        char arrayX[2] = {coords[0], '\0'};
        char arrayY[2] = {coords[1], '\0'};
        proposal.startx = atoi(arrayX);
        proposal.starty = atoi(arrayY)-64;
        if(!(checkBounds(board, current.name, proposal) && !checkOverlap(&board, proposal))) printf("Houston we have a big fucking problem with our proposed locash\n");
        //display the ship on the board

    }

    return board;
}

// Function that updates the board based on the player's guess
void updateBoardAfterGuess(board_t *board, int x, int y, bool *isHit, bool *isSunk) {
    *isHit = false;     // Initialize hit status as false
    *isSunk = false;    // Initialize sunk status as false

    // Validate coordinates to ensure they are within the bounds
    if (x < 0 || x >= NCOLS || y < 0 || y >= NROWS) {
        printf("Invalid coordinates.\n");
        return;
    }

    // Get the cell at the specified coordinates
    cell_t *cell = &board->array[y][x]; // Access the guessed cell

    // Check if the cell has already been guessed
    if (cell->guessed) {
        printf("This cell has already been guessed");
        return;
    }

    // Mark the cell as guessed
    cell->guessed = true;

    // Check if the cell is occupied by a ship
    if (cell->occupied) {
        *isHit = true;
        cell->hit = true;

        // Check if the entire ship has been sunk
        shipType_t *ship = &cell->ship;
        bool allCellsHit = true;

        // Scan the board for any remaining parts of this ship
        for (int i = 0; i < NROWS; i++) {
            for (int j = 0; j < NCOLS; j++) {
                if (board->array[i][j].occupied && board->array[i][j].ship.name == ship->name
                && !board->array[i][j].hit) {
                    allCellsHit = false;
                    break;
                }
            } 
            if (!allCellsHit) break;
        }

        // If all parts of the ship are hit, it is sunk
        if (allCellsHit) {
            *isSunk = true;
            printf("You sunk a %s", ship->name);
        }
    } else {
        // If the cell is not occupied, it's a miss
        printf("Miss!\n");
    }
}

// Function that iterates through all cells on the board, checking for any unsunk ship parts.
//      If any cell is occupied by a ship and not marked as hit, the game is not over
bool checkVictory(board_t *board) {
    // Iterate through ever cell on the board
    for (int i = 0; i < NROWS; i++) {
        for (int j = 0; j < NCOLS; j++) {
            cell_t *cell = &board->array[i][j];
            //If any ship cell is not hit, the player has not lost yet
            if (cell->occupied && !cell->hit) {
                return false;       // Found an unsunk ship part
            }
        }
    }
    return true;    // All ship parts are sunk
}

// Function that initializes a players game board
void initBoard(board_t *board) {
    // Iterate through each row and column
    for (int i = 0; i < NROWS; i++) {
        for (int j = 0; j < NCOLS; j++) {
            // Access each cell and reset its properties
            board->array[i][j].occupied = false;    // No ship places
            board->array[i][j].guessed = false;     // Not guessed yet
            board->array[i][j].hit = false;     // Not hit yet
        }
    }
}
/* Whats next?

* check if ship location is valid, shouldnt overlap with other ships
    check overlap function that looks to see if one cell has multiple ships in it 
    after checking bouds we want to actually place ships on board
* users place ships on board, save ship location
* set of seperate ships for each user 
* update board after guesses

*/


// while (lit){
        //     printf("Please input orientation (V/H): ");
        //     char orientation[50];
        //     fgets(orientation, sizeof(orientation), stdin);
        //     enum Orientation ORT = NULL;

        //     if (strcmp(orientation, 'H') == 0){
        //         ORT = HORIZONTAL;
        //         lit = false;
        //     }
        //     else if(strcmp(orientation, 'V') == 0) {
        //         ORT = VERTICAL;
        //         lit = false;
        //     } else {
        //         printf("Wrong\n");
        //     }
        //     println();
        //     bigO = ORT;
        // }

        
        // while (supa){
        //     printf("Please input coordinates (ex: A,1)");
        //     char coords[3];
        //     fgets(coords, sizeof(coords), stdin);

        //     if (strlen(coords) == 3){
        //         char letter = coords[0];
        //         char number = coords[2];
        //         bool validL = false;
        //         bool validN = false;
        //         char possibleL = 'A';
        //         int possibleN = 1;
        //         for (int i = 0; i < 10; i++){
        //             if (letter==possibleL) {
        //                 validL = true;
        //                 break;
        //             }
        //             possibleL++;
        //         }
        //         for (int i = 0; i < 10; i++){
        //             if (atoi(number)==possibleN){
        //                 validN = true;
        //                 break;
        //             }
        //             possibleN++;
        //         }
        //         if(validL && validN){
        //             supa=false;
        //             startX = possibleN;
        //             startY = possibleL;
        //         }
        //     }
        // }
