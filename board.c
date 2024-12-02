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