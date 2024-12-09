#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "board.h"
#include <curses.h>
#include "graphics.h"

const shipType_t shipArray[NDIFSHIPS] = {{"Destroyer", 2} ,{"Submarine",3} ,{"Cruiser", 3} ,{"Battleship", 4} ,{"Aircraft Carrier", 5}};
#define BUFFERSIZE 4
#define INIT_CURSOR 2;
size_t cursor = INIT_CURSOR;

/**
 * checkBounds takes a player's board and a player's proposal shipLocation and 
 * returns true if the boundaries for the proposed ship location are not crossed 
 * (i.e., the ship isn't off the board)
 */
bool checkBounds (struct shipLocation proposal){
    //save size of ship
    int size = proposal.shipType.size;
    // printf("checkBounds origin: %d,%d\n", proposal.startx, proposal.starty);
    // printf("origin x is %d (1 is true)", (proposal.startx < 1));
    // printf("origin x is %d (1 is true)", (proposal.startx >= (NCOLS+1)));

    //check if origin is within bounds
    if (proposal.startx < 1 || proposal.startx >= (NCOLS+1) || proposal.starty < 1 || proposal.starty >= (NROWS+1)){
        // printf("Invalid origin:");
        // printf("%d %d\n", proposal.startx, proposal.starty);
        return false;
    }//if

    //check orientation and then check based on that
    if (proposal.orientation == HORIZONTAL){
        //in horizontal case, check if we pass ends horizontally
        if ((proposal.startx + (size-1)) > NCOLS) return false;
    }else if (proposal.orientation==VERTICAL){
        //in vertical case, check if we pass ends vertically
        if ((proposal.starty + (size-1)) > NROWS) return false;
    }else{
        // printf("invalid orientation\n");
        return false;
    }
    //ifelse

    //at this point, bounds must be valid, so return true
    return true;
}


/**
 * checkOverlap takes a player's board and proposal shipLocation.
 *  It returns true if there's an overlap present, otherwise it updates
 *  the board to occupy cells where the proposed ship location is.
 */
bool checkOverlap(board_t * board, struct shipLocation proposal){
    // printf("in checkOverlap\n");

    //initialize an array with info from board
    cell_t array[NROWS+1][NROWS+1];
    memcpy(array, board, sizeof(cell_t)*121);

    //start coords and info of ship
    int x = proposal.startx;
    int y = proposal.starty;
    shipType_t ship = proposal.shipType;
    enum Orientation direction = proposal.orientation;

    // int occupationStatus;
    // if (array[x][y].occupied) {
    //     occupationStatus = 1;
    //  } else occupationStatus = 0;

    // printf("proposal cell %d %d .occupied is %d (1 is true)\n", x, y, array[x][y].occupied);

    //horizontal
    if (direction == HORIZONTAL){
        //check if proposed locations are already occupied
        for(int i = 0; i < proposal.shipType.size; i++){
            if (array[x + i][y].occupied){
                // printf("cell %d,%d is occupied\n", (x+i), y);
                return true;
            }
            //change cell status 
            array[x + i][y].occupied = true;
            array[x + i][y].ship = ship;
            // printf("cell %d,%d is now occupied\n", (x+i), y);
        }
    }

    //vertical
    if (direction == VERTICAL){
        //same shit as above 
        for(int i = 0; i < proposal.shipType.size; i++){
            if (array[x][y + i].occupied){
                return true;
            }
            //change cell status
            // printf("cell %d %d occupied status is %d (1 is true)\n", x, y+i, array[x][y+i].occupied);
            array[x][y + i].occupied = true;
            array[x][y + i].ship = ship;
            // printf("cell %d,%d is now occupied\n", x, (y+i));
        }
    }

    //update the board and return false if everything is good
    // printStatus(*board);
    // //memcpy(board, array, sizeof(cell_t)*121);
    // printStatus(*board);
    return false;
}


/**
 * validOrt instructs the user to give us an orientation (either "V" or "H") and loops until 
 * the user inputs a valid orientation. It then returns that valid orientation
 */
enum Orientation validOrt(WINDOW * window){
    
    //set ORT to INVALID to control the while loop which will loop until we have a valid orientation
    enum Orientation ORT = INVALID;
    bool invalid = true;
    int space = strlen("Please input orientation (V/H): ")+1;
    // mvwprintw(window, cursor++, 1, "%d", space);

    //provide user instructions
    mvwprintw(window, cursor, 1, "Please input orientation (V/H): ");

    //loop until we have valid input
    while (invalid){
        // mvwprintw(window, cursor++, 1, "top of loop\n");
        //store input
        char orientation[2]; //2 because it's one character and a terminating character
        wgetnstr(window, orientation, sizeof(orientation)-1);
        //print user input (at least the first character) so they can see what they entered
        mvwprintw(window, cursor++, space, "%s", orientation);
        // mvwprintw(window, cursor++, 1, "%d", space);
        // char extra = fgetc(stdin);
        //commenting this out because we can't access these values
        // char extra = wgetch(window);
        // cursor++;
        // if(extra != '\n'){
        //     mvwprintw(window, cursor++, 1, "Invalid orientation, try again. Please enter 'V' for vertical or 'H' \n");
        //     mvwprintw(window, cursor++, 3, "for horizontal: \n");
        //     // while (fgetc(stdin) != '\n');
        //     while (extra != '\n') {
        //         extra = wgetch(window) != '\n';
        //         mvwprintw(window, cursor, 1, &extra);
        //     }

        //i need to rewrite this to be a while loop that loops until orientation has a terminating character at orientation[1], then check orientation [0]
        // mvwprintw(window, cursor++, 1, "orientation[1]: %c.\n", orientation[1]);

        if(orientation[1]!='\0'){
            mvwprintw(window, cursor++, 1, "Invalid orientation, try again. Please enter 'V' for vertical or 'H' \n");
            mvwprintw(window, cursor, 3, "for horizontal: \n");
            space = strlen("for horizontal: \n")+3;
        } else {
            // mvwprintw(window, cursor++, 1, "in the else\n");
            //check for valid input and save it into ORT if it's valid, otherwise loop again with an informative message
            if (orientation[0] == 'H'){
                ORT = HORIZONTAL;
                invalid = false;
            } else if(orientation[0] == 'V') {
                ORT = VERTICAL;
                invalid = false;
            } else {
                mvwprintw(window, cursor++, 1, "Invalid orientation, try again. Please enter 'V' for vertical or 'H' \n");
                mvwprintw(window, cursor, 3, "for horizontal: \n");
                space = strlen("for horizontal: \n")+3;
            }
        }
        // mvwprintw(window, cursor++, 1, "I made it through the loop and ORT is %d (if 1 invalid)\n", (ORT==INVALID));

    }//while

    //return ORT because it won't reach this line until it's valid
    return ORT;

}//validOrt


/**
 * validCoords takes a pointer to a character array that it will fill with the two valid coordinate values
 */
int * validCoords(int * yay, WINDOW * window){

    //use this bool to control the while loop to loop until both coordinate values are valid
    bool supa = true;

    //give user instructions and store their input into coords[]
    mvwprintw(window, cursor++, 1, "Please input coordinates for the start point of your ship (ex: A,1): ");


     while (supa){
        char coords[BUFFERSIZE];
        // fgets(coords, sizeof(coords), stdin);
        wgetnstr(window, coords, sizeof(coords));
        // printf("coords.: %s.\n",coords);
        // int len = strlen(coords);
        // printf("strlen of input: %d\n", len);

        bool noNewlines = ((coords[0]!='\n')&&(coords[1]!='\n')&&(coords[2]!='\n'));
        bool comma = (coords[1]==',');

        //ensure valid string length
        if (strlen(coords) == 3 && noNewlines && comma){
            bool ten = false;
            // char next = fgetc(stdin);
            char next = wgetch(window);

            // if((coords[2]=='1')&&(next=='0')&&(fgetc(stdin)=='\n')){
            if((coords[2]=='1')&&(next=='0')&&(wgetch(window)=='\n')){
                ten = true;
                }else if(next!='\n'){
                mvwprintw(window, cursor++, 1, "Invalid input! Try again. Remember, the format is LETTER,NUMBER with a capital letter, a comma between the letter and number, and no spaces! : ");
                // while(fgetc(stdin)!='\n');
                while(wgetch(window)!='\n');
                continue;
            }
            // printf(".%c.\n", next);

            //set these for later use
            char letter = coords[0];
            char number = coords[2];
            bool validL = false;
            bool validN = false;
            char possibleL = 'A';
            int possibleN = 1;

            //loop through all possible valid letter values and break loop if match is found
            for (int i = 0; i < NCOLS; i++){
                if (letter==possibleL) {
                    validL = true;
                    break;
                }
                possibleL++;
            }

            int intLetter = possibleL - 'A' +1;
            // printf("Letter %c is %d\n", possibleL, intLetter);

            //loop through all possible valid numerical values and break loop if match is found
            for (int i = 0; i < NROWS; i++){
                char array[2] = {number, '\0'};
                if ((atoi(array))==possibleN){
                    validN = true;
                    break;
                }
                possibleN++;
            }

            if(ten&&validN) possibleN = 10;
            // printf("Number%d\n", possibleN);

            //if both values are valid, end while loop and store values in string to be returned
            if(validL && validN){
                supa=false;
                int validCoordinates[2] = {intLetter, possibleN};
                // printf("coordinates: %d %d\n", validCoordinates[0], validCoordinates[1]);
                yay=validCoordinates;
                return yay;
            }
        } 
        mvwprintw(window, cursor++, 1, "Invalid input. Try again. Remember, the format is LETTER,NUMBER with a capital letter, a comma between the letter and number, and no spaces! : ");
    }
    return NULL;
} 

/**
 * makeBoard loops through all of the ships and places them on the board. 
 * It returns the initialized board on success and an empty board on failure... but it shouldn't be able to fail.
 */
board_t makeBoard(WINDOW * window, WINDOW * playerWindow){
    //make a new board and a proposal ship location
    board_t board;
    board_t *boardPtr = malloc((sizeof(cell_t))*(NROWS+1)*(NCOLS+1));
    boardPtr = &board;
    initBoard(boardPtr);

    // printStatus(board);
    shipLocation_t proposal;

    //loop through the diff types of ships using the ship array
    for (int i = 0; i < NDIFSHIPS; i++){ 

        //save ship we're on as current
        shipType_t current = shipArray[i]; 
        
        //give user info on which ship we're using 
        // mvwprintw(window, cursor++, 1, "Current Ship: %s\nShip Length: %d\n", current.name, current.size);
        mvwprintw(window, cursor++, 1, "Current Ship: %s\n", current.name);
        mvwprintw(window, cursor++, 1, "Ship Length: %d\n", current.size);

        //get user to give us their orientation for the ship
        enum Orientation bigO = INVALID;
        bigO = validOrt(window); //this will not return until it's a valid orientation.
        if(bigO==INVALID) {
            mvwprintw(window, cursor++, 1, "INVALID ORIENTATION. Restarting this ship placement.\n");
            i--;
            continue;
        }

        //get user to give us their starting coordinate for the ship
        int coords[2] = {0, 0};
        memcpy(coords, validCoords(coords, window), (2* sizeof(int)));
        if(coords[0] == 0 || coords[1]==0) {
            mvwprintw(window, cursor++, 1, "INVALID COORDINATES. Restarting this ship placement.\n");
            i--;
            continue;
        }
        // printf("coordinates in makeBoard: %d %d\n", coords[0], coords[1]);

        //initialize proposal with valid values
        proposal.orientation = bigO;
        proposal.startx = coords[0];
        proposal.starty = coords[1];
        proposal.shipType = current;
        proposal.sunk=false;

        if(!(checkBounds(proposal))) {
            mvwprintw(window, cursor++, 1, "INVALID PLACEMENT-- BOUNDARY CROSSING. Restarting this ship placement.\n");
            i--;
            continue;
        }

        if(checkOverlap(boardPtr, proposal)){
            mvwprintw(window, cursor++, 1, "INVALID PLACEMENT-- OVERLAP. Restarting this ship placement.\n");
            i--;
            continue;
        } else {
            for (int i = 0; i < proposal.shipType.size; i++){
                if (bigO == HORIZONTAL){
                    board.array[proposal.startx][proposal.starty + i].occupied = true;
                    board.array[proposal.startx][proposal.starty + i].ship = proposal.shipType;
                } else {
                    board.array[proposal.startx + i][proposal.starty].occupied = true;
                    board.array[proposal.startx + i][proposal.starty].ship = proposal.shipType;
                }
                
            }
            
        }
        mvwprintw(window, cursor++, 1, "Valid ship placement.\n");
       
        //display the ship on the board

        //give user option to start over
        mvwprintw(window, cursor++, 1, "If you would like to reset your board, you may now type in 'R'. Otherwise, hit enter.\n");
        
        char input;
        // input = fgetc(stdin);
        input = wgetch(window);
        //wgetnstr(window, input, 2);


        while(input != '\n' && input != 'R' && input != 'r'){
            mvwprintw(window, cursor++, 1, "Invalid input: please input R to reset or hit enter to continue.\n");
            // input = fgetc(stdin);
            input = (char) wgetch(window);
              
        }

        if(input == 'R' || input == 'r') {
            board_t cleanBoard;
            initBoard(&cleanBoard);
            memcpy(&board, &cleanBoard, sizeof(cell_t)*121);
            i = 0;
        }

        draw_board(playerWindow, board.array, false);
        werase(window);
        box(window, 0, 0);
        cursor = INIT_CURSOR;
    
        //clean the window?

        // printStatus(board);
    }
    
    cursor = INIT_CURSOR;
    return board;
}

// Function that updates the board based on the player's guess
void updateBoardAfterGuess(board_t *board, int x, int y, bool *isHit, bool *isSunk, WINDOW * window) {
    *isHit = false;     // Initialize hit status as false
    *isSunk = false;    // Initialize sunk status as false

    // Validate coordinates to ensure they are within the bounds
    if (x < 1 || x > NCOLS || y < 1 || y > NROWS) {
        mvwprintw(window, cursor++, 1, "Invalid coordinates.\n");
        return;
    }

    // Get the cell at the specified coordinates
    cell_t *cell = &board->array[x][y]; // Access the guessed cell

    // Check if the cell has already been guessed
    if (cell->guessed) {
        mvwprintw(window, cursor++, 1, "This cell has already been guessed");
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
        for (int i = 1; i < NROWS+1; i++) {
            for (int j = 1; j < NCOLS+1; j++) {
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
            mvwprintw(window, cursor++, 1, "You sunk a %s", ship->name);
        }
    } else {
        // If the cell is not occupied, it's a miss
        mvwprintw(window, cursor++, 1, "Miss!\n");
    }
}

// Function that iterates through all cells on the board, checking for any unsunk ship parts.
//      If any cell is occupied by a ship and not marked as hit, the game is not over
bool checkVictory(board_t *board) {
    // Iterate through ever cell on the board
    for (int i = 1; i < NROWS+1; i++) {
        for (int j = 1; j < NCOLS+1; j++) {
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
    for (int i = 1; i < NROWS+1; i++) {
        for (int j = 1; j < NCOLS+1; j++) {
            // Access each cell and reset its properties
            
            board->array[i][j].occupied = false;    // No ship places
            board->array[i][j].guessed = false;     // Not guessed yet
            board->array[i][j].hit = false;     // Not hit yet
        }
    }
}


void printStatus(board_t board, WINDOW * window){
    for (int i = 1; i < NROWS+1; i++){
        for (int j = 1; j < NROWS+1; j++){
            mvwprintw(window, cursor++, 1, "Cell %d,%d is occupied (! is true): %d\n", i, j, board.array[i][j].occupied);
        }
    }
}
/* Whats next?

* users place ships on board, save ship location
* set of seperate ships for each user 
* update board after guesses

* we could have a thread keeping track of cursor and if it's updated to be equal to the edge of the window we could reset it to INIT_CURSOR

*/