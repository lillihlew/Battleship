#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "board.h"

const shipType_t shipArray[NDIFSHIPS] = {{"Destroyer", 2} ,{"Submarine",3} ,{"Cruiser", 3} ,{"Battleship", 4} ,{"Aircraft Carrier", 5}};
#define BUFFERSIZE 10

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
enum Orientation validOrt(){
    
    //set ORT to INVALID to control the while loop which will loop until we have a valid orientation
    enum Orientation ORT = INVALID;

    //provide user instructions
    

    //loop until we have valid input
    while (ORT==INVALID){

        printf("Please input orientation (V/H): ");

        //store input
        char orientation[2]; //2 because it's one character and a terminating character
        fgets(orientation, sizeof(orientation), stdin);
        char extra = fgetc(stdin);
        
        if(extra != '\n'){
            printf("Invalid orientation, try again. Please enter 'V' for vertical or 'H' for horizontal.\n");
            while (fgetc(stdin) != '\n');
            // fputc('\n', stdin);
        } else {
            //check for valid input and save it into ORT if it's valid, otherwise loop again with an informative message
            if (orientation[0] == 'H'){
                ORT = HORIZONTAL;
            } else if(orientation[0] == 'V') {
                ORT = VERTICAL;
            } else {
                printf("Invalid orientation, try again. Please enter 'V' for vertical or 'H' for horizontal.\n");
                
            }
        }

    }//while

    //return ORT because it won't reach this line until it's valid
    return ORT;

}//validOrt


/**
 * validCoords takes a pointer to a character array that it will fill with the two valid coordinate values
 */
int * validCoords(int * yay){

    //use this bool to control the while loop to loop until both coordinate values are valid
    bool supa = true;

    //give user instructions and store their input into coords[]
    printf("Please input coordinates for the start point of your ship (ex: A,1): ");


     while (supa){
        char coords[4];
        fgets(coords, sizeof(coords), stdin);
        // printf("coords.: %s.\n",coords);
        // int len = strlen(coords);
        // printf("strlen of input: %d\n", len);

        bool noNewlines = ((coords[0]!='\n')&&(coords[1]!='\n')&&(coords[2]!='\n'));
        bool comma = (coords[1]==',');

        //ensure valid string length
        if (strlen(coords) == 3 && noNewlines && comma){
            bool ten = false;
            char next = fgetc(stdin);
            if((coords[2]=='1')&&(next=='0')&&(fgetc(stdin)=='\n')){
                ten = true;
                }else if(next!='\n'){
                printf("Invalid input--Too long! Try again. Remember, the format is LETTER,NUMBER with a capital letter, a comma between the letter and number, and no spaces!\n");
                while(fgetc(stdin)!='\n');
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
        printf("Invalid input. Try again. Remember, the format is LETTER,NUMBER with a capital letter, a comma between the letter and number, and no spaces!\n");
    }
    return NULL;
} 

/**
 * makeBoard loops through all of the ships and places them on the board. 
 * It returns the initialized board on success and an empty board on failure... but it shouldn't be able to fail.
 */
board_t makeBoard(){
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
        printf("Current Ship: %s\nShip Length: %d\n", current.name, current.size);

        //get user to give us their orientation for the ship
        enum Orientation bigO = INVALID;
        bigO = validOrt(); //this will not return until it's a valid orientation.
        if(bigO==INVALID) {
            printf("INVALID ORIENTATION. Restarting this ship placement.\n");
            i--;
            continue;
        }

        //get user to give us their starting coordinate for the ship
        int coords[2] = {0, 0};
        memcpy(coords, validCoords(coords), (2* sizeof(int)));
        if(coords[0] == 0 || coords[1]==0) {
            printf("INVALID COORDINATES. Restarting this ship placement.\n");
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
            printf("INVALID PLACEMENT-- BOUNDARY CROSSING. Restarting this ship placement.\n");
            i--;
            continue;
        }

        if(checkOverlap(boardPtr, proposal)){
            printf("INVALID PLACEMENT-- OVERLAP. Restarting this ship placement.\n");
            i--;
            continue;
        } else {
            for (int i = 0; i < proposal.shipType.size; i++){
                if (bigO == VERTICAL){
                    board.array[proposal.startx][proposal.starty + i].occupied = true;
                    board.array[proposal.startx][proposal.starty + i].ship = proposal.shipType;
                } else {
                    board.array[proposal.startx + i][proposal.starty].occupied = true;
                    board.array[proposal.startx + i][proposal.starty].ship = proposal.shipType;
                }
                
            }
            
        }
        printf("Valid ship placement.\n");
       
        //display the ship on the board

        //give user option to start over
        printf("If you would like to reset your board, you may now type in 'R'. Otherwise, hit enter.\n");

        char input[BUFFERSIZE];
        fgets(input, BUFFERSIZE, stdin);
        if(strcmp(input,"\n")!=0) {
            board_t cleanBoard;
            memcpy(&cleanBoard, &board, sizeof(cell_t)*121);
            i=0;
            return cleanBoard;
        }
        // printStatus(board);
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
    for (int i = 1; i < NROWS+1; i++) {
        for (int j = 1; j < NCOLS+1; j++) {
            // Access each cell and reset its properties
            
            board->array[i][j].occupied = false;    // No ship places
            board->array[i][j].guessed = false;     // Not guessed yet
            board->array[i][j].hit = false;     // Not hit yet
        }
    }
}

void printStatus(board_t board){
    for (int i = 1; i < NROWS+1; i++){
        for (int j = 1; j < NROWS+1; j++){
            printf("Cell %d,%d is occupied (! is true): %d\n", i, j, board.array[i][j].occupied);
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

*/#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "board.h"

const shipType_t shipArray[NDIFSHIPS] = {{"Destroyer", 2} ,{"Submarine",3} ,{"Cruiser", 3} ,{"Battleship", 4} ,{"Aircraft Carrier", 5}};
#define BUFFERSIZE 10

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
enum Orientation validOrt(){
    
    //set ORT to INVALID to control the while loop which will loop until we have a valid orientation
    enum Orientation ORT = INVALID;

    //provide user instructions
    

    //loop until we have valid input
    while (ORT==INVALID){

        printf("Please input orientation (V/H): ");

        //store input
        char orientation[2]; //2 because it's one character and a terminating character
        fgets(orientation, sizeof(orientation), stdin);
        char extra = fgetc(stdin);
        
        if(extra != '\n'){
            printf("Invalid orientation, try again. Please enter 'V' for vertical or 'H' for horizontal.\n");
            while (fgetc(stdin) != '\n');
            // fputc('\n', stdin);
        } else {
            //check for valid input and save it into ORT if it's valid, otherwise loop again with an informative message
            if (orientation[0] == 'H'){
                ORT = HORIZONTAL;
            } else if(orientation[0] == 'V') {
                ORT = VERTICAL;
            } else {
                printf("Invalid orientation, try again. Please enter 'V' for vertical or 'H' for horizontal.\n");
                
            }
        }

    }//while

    //return ORT because it won't reach this line until it's valid
    return ORT;

}//validOrt


/**
 * validCoords takes a pointer to a character array that it will fill with the two valid coordinate values
 */
int * validCoords(int * yay){

    //use this bool to control the while loop to loop until both coordinate values are valid
    bool supa = true;

    //give user instructions and store their input into coords[]
    printf("Please input coordinates for the start point of your ship (ex: A,1): ");


     while (supa){
        char coords[4];
        fgets(coords, sizeof(coords), stdin);
        // printf("coords.: %s.\n",coords);
        // int len = strlen(coords);
        // printf("strlen of input: %d\n", len);

        bool noNewlines = ((coords[0]!='\n')&&(coords[1]!='\n')&&(coords[2]!='\n'));
        bool comma = (coords[1]==',');

        //ensure valid string length
        if (strlen(coords) == 3 && noNewlines && comma){
            bool ten = false;
            char next = fgetc(stdin);
            if((coords[2]=='1')&&(next=='0')&&(fgetc(stdin)=='\n')){
                ten = true;
                }else if(next!='\n'){
                printf("Invalid input--Too long! Try again. Remember, the format is LETTER,NUMBER with a capital letter, a comma between the letter and number, and no spaces!\n");
                while(fgetc(stdin)!='\n');
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
        printf("Invalid input. Try again. Remember, the format is LETTER,NUMBER with a capital letter, a comma between the letter and number, and no spaces!\n");
    }
    return NULL;
} 

/**
 * makeBoard loops through all of the ships and places them on the board. 
 * It returns the initialized board on success and an empty board on failure... but it shouldn't be able to fail.
 */
board_t makeBoard(){
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
        printf("Current Ship: %s\nShip Length: %d\n", current.name, current.size);

        //get user to give us their orientation for the ship
        enum Orientation bigO = INVALID;
        bigO = validOrt(); //this will not return until it's a valid orientation.
        if(bigO==INVALID) {
            printf("INVALID ORIENTATION. Restarting this ship placement.\n");
            i--;
            continue;
        }

        //get user to give us their starting coordinate for the ship
        int coords[2] = {0, 0};
        memcpy(coords, validCoords(coords), (2* sizeof(int)));
        if(coords[0] == 0 || coords[1]==0) {
            printf("INVALID COORDINATES. Restarting this ship placement.\n");
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
            printf("INVALID PLACEMENT-- BOUNDARY CROSSING. Restarting this ship placement.\n");
            i--;
            continue;
        }

        if(checkOverlap(boardPtr, proposal)){
            printf("INVALID PLACEMENT-- OVERLAP. Restarting this ship placement.\n");
            i--;
            continue;
        } else {
            for (int i = 0; i < proposal.shipType.size; i++){
                if (bigO == VERTICAL){
                    board.array[proposal.startx][proposal.starty + i].occupied = true;
                    board.array[proposal.startx][proposal.starty + i].ship = proposal.shipType;
                } else {
                    board.array[proposal.startx + i][proposal.starty].occupied = true;
                    board.array[proposal.startx + i][proposal.starty].ship = proposal.shipType;
                }
                
            }
            
        }
        printf("Valid ship placement.\n");
       
        //display the ship on the board

        //give user option to start over
        printf("If you would like to reset your board, you may now type in 'R'. Otherwise, hit enter.\n");

        char input[BUFFERSIZE];
        fgets(input, BUFFERSIZE, stdin);
        if(strcmp(input,"\n")!=0) {
            board_t cleanBoard;
            memcpy(&cleanBoard, &board, sizeof(cell_t)*121);
            i=0;
            return cleanBoard;
        }
        // printStatus(board);
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
    for (int i = 1; i < NROWS+1; i++) {
        for (int j = 1; j < NCOLS+1; j++) {
            // Access each cell and reset its properties
            
            board->array[i][j].occupied = false;    // No ship places
            board->array[i][j].guessed = false;     // Not guessed yet
            board->array[i][j].hit = false;     // Not hit yet
        }
    }
}

void printStatus(board_t board){
    for (int i = 1; i < NROWS+1; i++){
        for (int j = 1; j < NROWS+1; j++){
            printf("Cell %d,%d is occupied (! is true): %d\n", i, j, board.array[i][j].occupied);
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