#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "board.h"
#include <curses.h>
#include "graphics.h"

//the ships we use in the game
const shipType_t shipArray[NDIFSHIPS] = {{"Destroyer", 2} ,{"Submarine",3} ,{"Cruiser", 3} ,{"Battleship", 4} ,{"Aircraft Carrier", 5}};

//number of characters we read in at a time 
#define BUFFERSIZE 4

//vertical start index for the cursor of the user input window
#define INIT_CURSOR 2 

//track where the cursor is
size_t cursor = INIT_CURSOR;

/**checkBounds
 *  checkBounds takes a player's proposal shipLocation (including origin, size, 
 *  and orientation) and returns true if the boundaries for the proposed ship 
 *  location are not crossed (i.e., the ship isn't off the board)
 */
bool checkBounds (struct shipLocation proposal){
    //save size of ship
    int size = proposal.shipType.size;

    //check if origin is within bounds and return false if not
    if (proposal.startx < 1 || proposal.startx >= (NCOLS+1) || proposal.starty < 1 || proposal.starty >= (NROWS+1)){
        return false;
    }

    //check orientation and then check if ship based on that
    if (proposal.orientation == HORIZONTAL){
        //in horizontal case, check if we pass ends horizontally
        if ((proposal.startx + (size-1)) > NCOLS) return false;
    }else if (proposal.orientation==VERTICAL){
        //in vertical case, check if we pass ends vertically
        if ((proposal.starty + (size-1)) > NROWS) return false;
    }else{
        //invalid orientation
        return false;
    }

    //at this point, bounds must be valid, so return true
    return true;
}



/**checkOverlap
 *  checkOverlap takes a player's board and proposal shipLocation.
 *  It returns true if there's an overlap present.
 *  Assumptions: proposal's coordinates and orientation are valid
 */
bool checkOverlap(board_t * board, struct shipLocation proposal){

    //initialize an array with info from board so we can modify it as we check for overlap
    cell_t array[NROWS+1][NROWS+1];
    memcpy(array, board, sizeof(cell_t)*121);

    //save start coords and info of ship
    int x = proposal.startx;
    int y = proposal.starty;
    shipType_t ship = proposal.shipType;
    enum Orientation direction = proposal.orientation;

    
    //horizontal case
    if (direction == HORIZONTAL){
        //check if proposed locations are already occupied and return true if so
        for(int i = 0; i < proposal.shipType.size; i++){
            if (array[x + i][y].occupied){
                return true;
            }
            //change cell status 
            array[x + i][y].occupied = true;
            array[x + i][y].ship = ship;
        }
    }

    //vertical
    if (direction == VERTICAL){
        //check if proposed locations are already occupied and return true if so
        for(int i = 0; i < proposal.shipType.size; i++){
            if (array[x][y + i].occupied){
                return true;
            }
            //change cell status
            array[x][y + i].occupied = true;
            array[x][y + i].ship = ship;
        }
    }

    //to get here, there was no overlap
    return false;
}



/**validOrt
 *  validOrt takes the user input window 
 *  validOrt instructs the user to give us an orientation (either "V" or "H") and loops until 
 *  the user inputs a valid orientation. It then returns that valid orientation.
 *  Since we handle user input here, the function is mostly error checking.
 */
enum Orientation validOrt(WINDOW * window){
    
    //set ORT to INVALID to control the while loop which will loop until we have a valid orientation
    enum Orientation ORT = INVALID;
    bool invalid = true;
    
    //save this value so that we can print to the correct horizontal cursor location
    int space = strlen("Please input orientation (V/H): ")+1;

    //provide user instructions
    mvwprintw(window, cursor, 1, "Please input orientation (V/H): ");

    //loop until we have valid input
    while (invalid){
        
        //store input
        char orientation[2]; //2 because it's one character and a terminating character
        orientation[1]='\0';
        orientation[0]=wgetch(window);

        //handle case where user input '/n'
        if(orientation[0]=='\n'){
            cursor++;
            mvwprintw(window, cursor++, 1, "Invalid orientation, try again. Please enter 'V' for vertical or 'H' \n");
            mvwprintw(window, cursor, 3, "for horizontal: \n");
            space = strlen("for horizontal: \n")+3;
            continue;
        }

        // save next character user entered, because there's at least one, even if it's a '\n'
        char potentialNewline = wgetch(window);
        bool newline = potentialNewline=='\n';

        //clean up user input 
        while(potentialNewline!='\n'){
            potentialNewline=wgetch(window);
        }

        //print user input (at least the first character) so they can see what they entered
        mvwprintw(window, cursor++, space, "%s", orientation);

        //if we have invalid input that was too long
        if(!newline){
            mvwprintw(window, cursor++, 1, "Invalid orientation, try again. Please enter 'V' for vertical or 'H' \n");
            mvwprintw(window, cursor, 3, "for horizontal: \n");
            space = strlen("for horizontal: \n")+3;
        } else {
            /**
             * we were given one character of input, so check for valid input and, if valid, save it
             * into ORT and flip invalid boolean, otherwise loop again with an informative message
             */
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

    }//while

    //return ORT because it won't reach this line until it's valid
    return ORT;

}//validOrt



/**validCoords
 *  validCoords takes a pointer to an int array that it will fill with the two valid coordinate
 *  values. It also takes the user input window since it deals with user input. Since this input
 *  is more structured and complicated, most of this function is error checking.
 */
int * validCoords(int * yay, WINDOW * window, char * prompt){

    //use this bool to control the while loop to loop until both coordinate values are valid
    bool supa = true; //we used the word valid too much in this method so we picked supa as the bool name

    //save horizontal indentation for cursor 
    int space = strlen(prompt)+1;

    //give user instructions
    mvwprintw(window, cursor, 1, "%s", prompt);

    //loop until we have valid input
    while (supa){

        //store user input
        char coords[BUFFERSIZE];

        /**
         * if we have a 10 as the input numeric value. This case is special because it's a diffent 
         * number of input characters, so we have to hand it specifically and carefully. This bool
         * will only be flipped to true if the user input 10 and it was a valid input.
         */
        bool ten = false;

        //for use later in error checking;
        bool shortInput = false;

        //collect first (up to) three user input characters
        for(int i = 0; i<BUFFERSIZE-1; i++){
            coords[i]=(char)wgetch(window);
            if(coords[i]=='\n') {
                shortInput = true;
                break;
            }
        }
        
        //print user input so they can see what they wrote
        mvwprintw(window, cursor, space, "%s", coords);
        
        //check if input was too short or improperly formatted
        bool noNewlines = ((coords[0]!='\n')&&(coords[1]!='\n')&&(coords[2]!='\n'));
        bool comma = (coords[1]==',');

        //ensure valid string length
        if (strlen(coords) == 3 && noNewlines && comma){
            
            //get the next character - should either be a \n or a 0 (0 in the case of a 10)
            char next = (char) wgetch(window);

            //check for a 10
            if(coords[2]=='1'){
                
                /**ensure that if it was a 10, it was input properly, and then print the 0 (and \n for 
                 * formatting) so the user can see the rest of their input
                 */ 
                if((next=='0')&&(((char) wgetch(window))=='\n')){
                    ten = true;
                    mvwprintw(window, cursor++, space+3, "%c\n", next);
                }else{
                    //print the \n for formatting
                    mvwprintw(window, cursor++, space+sizeof(coords)+1, "\n");
                }

                //if it wasn't a 10 and too long, print informative error message and loop
                if((next!='\n')&&(!ten)){
                    
                    //clean up user input 
                    while(next!='\n'){
                        next=wgetch(window);
                    }

                    //print error message split over three lines to move our cursor
                    mvwprintw(window, cursor++, 1, "Invalid input. Try again. Remember, the format is LETTER,NUMBER\n");
                    mvwprintw(window, cursor++, 1, " with a capital letter, a comma between the letter and number,\n");
                    mvwprintw(window, cursor, 1, " and no spaces! : ");
                    space = strlen("and no spaces! : ")+1;
                    continue;
                }
            }else{
            //in this portion, the first input character was not a 1

                //print a newline to match formatting above
                mvwprintw(window, cursor++, space+sizeof(coords)+1, "\n");

                //if input was too long, print informative error message and loop
                if(next!='\n'){

                    //clean up user input
                    while(next!='\n'){
                        next=wgetch(window);
                    }

                    //print error message split over three lines to move our cursor
                    mvwprintw(window, cursor++, 1, "Invalid input. Try again. Remember, the format is LETTER,NUMBER\n");
                    mvwprintw(window, cursor++, 1, " with a capital letter, a comma between the letter and number,\n");
                    mvwprintw(window, cursor, 1, " and no spaces! : ");
                    space = strlen("and no spaces! : ")+1;
                    continue;
                }
            }

            //to get to this point, the input in coords must be x,x and there may or may not be a 10 input

            //save these for later use
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

            //convert letter to int value
            int intLetter = possibleL - 'A' +1;

            //loop through all possible valid numerical values and break loop if match is found
            for (int i = 0; i < NROWS; i++){
                char array[2] = {number, '\0'};
                if ((atoi(array))==possibleN){
                    validN = true;
                    break;
                }
                possibleN++;
            }

            //replace n value with 10 if there was a 10 input
            if(ten&&validN) possibleN = 10;

            //if both values are valid, end while loop and store values in string to be returned
            if(validL && validN){
                supa=false;
                int validCoordinates[2] = {intLetter, possibleN};
                yay=validCoordinates;
                return yay;
            }
        }else{
            //if user didn't have short input
            if(!shortInput){
                //get the next character - should either be a \n or garbage since here the input did not match the specifications
                char next = (char) wgetch(window);

                //clean up user input 
                while(next!='\n'){
                    next=wgetch(window);
                }
            }

            /**increment cursor from before while loop informative message (and from successive error 
             * messages that don't increment the cursor) */
            cursor++;
        }
        
        //print error message split over three lines to move our cursor
        mvwprintw(window, cursor++, 1, "Invalid input. Try again. Remember, the format is LETTER,NUMBER\n");
        mvwprintw(window, cursor++, 1, " with a capital letter, a comma between the letter and number, \n");
        mvwprintw(window, cursor, 1, " and no spaces! : ");
        space = strlen("and no spaces! : ")+1;
    }

    //this point should never be reached
    return NULL;
} 



/**makeBoard
 *  makeBoard takes the input window and the player's board window.
 *  makeBoard loops through all of the ships from the above shipArray and places them on the board based on
 *  user input from the helper functions seen above. It returns the initialized board on success and an empty 
 *  board on failure, but it shouldn't be able to fail.
 */
board_t makeBoard(WINDOW * window, WINDOW * playerWindow){
    //make a new board and pointer to it
    board_t board;
    // board_t *boardPtr = malloc((sizeof(cell_t))*(NROWS+1)*(NCOLS+1));
    // boardPtr = &board;
    initBoard(&board);

    //make a new ship location proposal to be vetted below
    shipLocation_t proposal;

    //loop through the different types of ships using the ship array.
    for (int i = 0; i < NDIFSHIPS; i++){ 
        //If any validation checks fail, we will print an error message and restart the current iteration of the loop

        //save ship we're on as current
        shipType_t current = shipArray[i]; 
        
        //give user info on which ship we're using 
        mvwprintw(window, cursor++, 1, "Current Ship: %s\n", current.name);
        mvwprintw(window, cursor++, 1, "Ship Length: %d\n", current.size);

        //prompt user to give us their orientation for the ship and save it in bigO
        enum Orientation bigO = INVALID;
        bigO = validOrt(window); //this will not return until it's a valid orientation.
        if(bigO==INVALID) {
            mvwprintw(window, cursor++, 1, "INVALID ORIENTATION. Restarting this ship placement.\n");
            i--;
            continue;
        }

        //prompt user to give us their starting coordinates for the ship and save them into coords
        int coords[2] = {0, 0};
        memcpy(coords, validCoords(coords, window, "Please input coordinates for the start point of your ship (ex: A,1): \0"), (2* sizeof(int)));
        if(coords[0] == 0 || coords[1]==0) {
            mvwprintw(window, cursor++, 1, "INVALID COORDINATES. Restarting this ship placement.\n");
            i--;
            continue;
        }

        //initialize proposal with valid values
        proposal.orientation = bigO;
        proposal.startx = coords[0];
        proposal.starty = coords[1];
        proposal.shipType = current;
        proposal.sunk=false;

        //check if proposal shipLocation will cross bounds of board
        if(!(checkBounds(proposal))) {
            mvwprintw(window, cursor++, 1, "INVALID PLACEMENT-- BOUNDARY CROSSING. Restarting this ship placement.\n");
            i--;
            continue;
        }

        //check if proposal shipLocation will overlap with another ship's placement, and if not, update board
        if(checkOverlap(&board, proposal)){
            mvwprintw(window, cursor++, 1, "INVALID PLACEMENT-- OVERLAP. Restarting this ship placement.\n");
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

        //inform user of success
        mvwprintw(window, cursor++, 1, "Valid ship placement.\n");
       
        //give user option to start board over
        mvwprintw(window, cursor++, 1, "If you would like to reset your board, you may now type in 'R'. Otherwise, hit enter.\n");
        
        //store input
        char input;
        input = wgetch(window);

        //loop until we receive valid input
        while(input != '\n' && input != 'R' && input != 'r'){
            mvwprintw(window, cursor, 1, "Invalid input: please input R to reset or hit enter to continue: ");
            input = (char) wgetch(window);
            mvwprintw(window, cursor++, strlen("Invalid input: please input R to reset or hit enter to continue: ")+1, ": %c\n", input);
        }

        //if user wanted to reset board, wipe the board and reset i to -1 to start the loop all the way over
        if(input == 'R' || input == 'r') {
            board_t cleanBoard;
            initBoard(&cleanBoard);
            memcpy(&board, &cleanBoard, sizeof(cell_t)*121);
            i = -1;
        }

        //update player's board screen and clean the input window
        draw_player_board(playerWindow, board.array);
        werase(window);
        box(window, 0, 0);
        cursor = INIT_CURSOR;

        mvwprintw(window, cursor++, 1, "iteration %d complete\n", i);
    }    
    
    //print exit message
    mvwprintw(window, cursor++, 1, "Game initialized, enjoy the game!\n");

    //reset cursor to top of box
    cursor = INIT_CURSOR;

    //return initialized box
    return board;
}



/**updateBoardAfterGuess
 *  Function that updates the board based on the player's guess
 *  Takes a board, coordinates, bools giving information about the
 *  specified cell (to be updated), and the user's input window
 */
void updateBoardAfterGuess(board_t *board, int x, int y, bool *isHit, bool *isSunk, WINDOW * window) {
    *isHit = false;     // Initialize hit status as false
    *isSunk = false;    // Initialize sunk status as false

    // Validate coordinates to ensure they are within bounds and return if not
    if (x < 1 || x > NCOLS || y < 1 || y > NROWS) {
        mvwprintw(window, cursor++, 1, "Invalid coordinates.\n");
        return;
    }

    // Get the cell at the specified coordinates
    cell_t *cell = &board->array[x][y]; // Access the guessed cell

    // Check if the cell has already been guessed and return if yes
    if (cell->guessed) {
        mvwprintw(window, cursor++, 1, "This cell has already been guessed");
        return;
    }

    // Mark the cell as guessed
    cell->guessed = true;

    // Check if the cell is occupied by a ship
    if (cell->occupied) {

        //update booleans
        *isHit = true;
        cell->hit = true;

        // Save values for loop to check if the entire ship has been sunk
        shipType_t *ship = &cell->ship;
        bool allCellsHit = true;

        /**
         * Scan the board for any remaining parts of this ship and flip allCellsHit 
         * if we find an un-hit cell from the same ship
         */ 
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

        // If all parts of the ship are hit, it is sunk, so update that boolean
        if (allCellsHit) {
            *isSunk = true;
            mvwprintw(window, cursor++, 1, "You sunk a %s", ship->name);
        }

    } else {
        // If the cell is not occupied, it's a miss
        mvwprintw(window, cursor++, 1, "Miss!\n");
    }
}



/**checkVictory
 *  Function that iterates through all cells on the board, checking for any unsunk ship parts.
 *  If any cell is occupied by a ship and not marked as hit, the game is not over
 */
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



/**initBoard
 *  Function that initializes a players game board by setting every cell to unoccupied, not 
 *  guessed, and not hit
 */
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


/**printStatus
 *  used by us during debugging to print the occupation status of each cell
 */
void printStatus(board_t board, WINDOW * window){
    for (int i = 1; i < NROWS+1; i++){
        for (int j = 1; j < NROWS+1; j++){
            mvwprintw(window, cursor++, 1, "Cell %d,%d is occupied (! is true): %d\n", i, j, board.array[i][j].occupied);
        }
    }
}



/**hitOrMiss
 * takes an int array containing coordinates and a board, returns "HIT" if 
 * the cell specified is occupied and hasn't yet been guessed, "GUESSED" if
 * the cell specified has been guessed, and "MISS" if it's unoccupied and 
 * unguessed.
 * Assumptions: coordinates are valid and order is x,y and board is initialized, and a 10 is sent in as a 0
 */
char * hitOrMiss(int attackCoordsArray[2], board_t * board){

    //save specified cell
    cell_t cell = board->array[attackCoordsArray[0]][attackCoordsArray[1]];

    //if cell has already been guessed
    if(cell.guessed) return "GUESSED";
    
    /*at this point, our cell has not been guessed, so return hit if occupied and 
    miss otherwise, and update cell status accordingly*/
    cell.guessed=true;
    if(cell.occupied){
        cell.hit = true;
        return "HIT";
    } else {
        return "MISS";
    }
}

