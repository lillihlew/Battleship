#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <curses.h>
#include <unistd.h>

#include "board.h"
#include "graphics.h"

//the ships we use in the game
const shipType_t shipArray[NDIFSHIPS] = {{"Destroyer", 2} ,{"Submarine",3} ,{"Cruiser", 3} ,{"Battleship", 4} ,{"Aircraft Carrier", 5}};

//number of characters we read in at a time 
#define BUFFERSIZE 4

static pthread_t cursor_thread;
static pthread_mutex_t cursor_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool tracking_active = true;

static pthread_t victory_thread;
static pthread_mutex_t victory_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool game_active = true;

//track most recent prompt (not including error messages)
char * most_recent_prompt;

//track space
int space;

//track win/lose
bool win;
bool lose;

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
    space = strlen("Please input orientation (V/H): ")+1;

    //provide user instructions
    mvwprintw(window, cursor, 1, "Please input orientation (V/H): ");
    free(most_recent_prompt);
    most_recent_prompt = strdup("Please input orientation (V/H): ");

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
    most_recent_prompt = strdup(prompt);
    //use this bool to control the while loop to loop until both coordinate values are valid
    bool supa = true; //we used the word valid too much in this method so we picked supa as the bool name

    //save horizontal indentation for cursor 
    space = strlen(prompt)+1;

    //give user instructions
    mvwprintw(window, cursor, 1, "%s", prompt);

    //loop until we have valid input
    while (supa){

        //store user input
        char coords[BUFFERSIZE+1];

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
                coords[i+1] = '\0';
                break;
            } else {
                coords[BUFFERSIZE] = '\0';
            }
        }
        
        //print user input so they can see what they wrote
        mvwprintw(window, cursor, space, "%c%c%c", coords[0], coords[1], coords[2]);
    
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
                // FILE* serverInputCoordsFile = fopen("serverInputCoordsFile.txt", "w+");
                // fprintf(serverInputCoordsFile, "coords in yay: %d,%d\n", yay[0], yay[1]);
                // fclose(serverInputCoordsFile);
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
    most_recent_prompt = strdup("first instance of strdup to avoid bugs! :)");
    //make a new board and pointer to it
    board_t board;
    // board_t *boardPtr = malloc((sizeof(cell_t))*(NROWS+1)*(NCOLS+1));
    // boardPtr = &board;
    initBoard(&board);

    //make a new ship location proposal to be vetted below
    shipLocation_t proposal;

    //loop through the different types of ships using the ship array.
    for (int i = 0; i < 2; i++){ 
        //If any validation checks fail, we will print an error message and restart the current iteration of the loop

        //save ship we're on as current
        shipType_t current = shipArray[i]; 
        
        //give user info on which ship we're using 
        mvwprintw(window, cursor++, 1, "Current Ship: %s\n", current.name);
        mvwprintw(window, cursor++, 1, "Ship Length: %d\n", current.size);
        int nameLen = strlen(current.name);
        int ourWords = strlen("Current Ship: \nShip Length: \n");
        free(most_recent_prompt);
        most_recent_prompt = malloc(ourWords+nameLen+1+1);//extra 1 for ship length
        sprintf(most_recent_prompt, "Current Ship: %s\nShip Length: %d\n", current.name, current.size);



        //prompt user to give us their orientation for the ship and save it in bigO
        enum Orientation bigO = INVALID;
        bigO = validOrt(window); //this will not return until it's a valid orientation.
        if(bigO==INVALID) {
            mvwprintw(window, cursor++, 1, "INVALID ORIENTATION. Restarting this ship placement.\n");
            free(most_recent_prompt);
            most_recent_prompt = strdup("INVALID ORIENTATION. Restarting this ship placement.\n");
            i--;
            continue;
        }

        //prompt user to give us their starting coordinates for the ship and save them into coords
        int coords[2] = {0, 0};
        memcpy(coords, validCoords(coords, window, "Please input coordinates for the start point of your ship (ex: A,1): \0"), (2* sizeof(int)));
        if(coords[0] == 0 || coords[1]==0) {
            mvwprintw(window, cursor++, 1, "INVALID COORDINATES. Restarting this ship placement.\n");
            free(most_recent_prompt);
            most_recent_prompt = strdup("INVALID COORDINATES. Restarting this ship placement.\n");
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
            free(most_recent_prompt);
            most_recent_prompt = strdup("INVALID PLACEMENT-- BOUNDARY CROSSING. Restarting this ship placement.\n");
            i--;
            continue;
        }

        //check if proposal shipLocation will overlap with another ship's placement, and if not, update board
        if(checkOverlap(&board, proposal)){
            mvwprintw(window, cursor++, 1, "INVALID PLACEMENT-- OVERLAP. Restarting this ship placement.\n");
            free(most_recent_prompt);
            most_recent_prompt = strdup("INVALID PLACEMENT-- OVERLAP. Restarting this ship placement.\n");
            i--;
            continue;
        } else {
            for (int i = 0; i < proposal.shipType.size; i++){
                if (bigO == VERTICAL){
                    board.array[proposal.startx][proposal.starty + i].occupied = true;
                    board.array[proposal.startx][proposal.starty + i].ship = proposal.shipType;
                    board.array[proposal.startx][proposal.starty + i].ship.sunk = false;
                } else {
                    board.array[proposal.startx + i][proposal.starty].occupied = true;
                    board.array[proposal.startx + i][proposal.starty].ship = proposal.shipType;
                    board.array[proposal.startx][proposal.starty + i].ship.sunk = false;
                }
            }
        }

        //inform user of success
        mvwprintw(window, cursor++, 1, "Valid ship placement.\n");
        free(most_recent_prompt);
        most_recent_prompt = strdup("Valid ship placement.\n");
       
        //give user option to start board over
        mvwprintw(window, cursor++, 1, "If you would like to reset your board, you may now type in 'R'. Otherwise, hit enter.\n");
        free(most_recent_prompt);
        most_recent_prompt = strdup("If you would like to reset your board, you may now type in 'R'. Otherwise, hit enter.\n");
        
        //store input
        char input;
        input = wgetch(window);

        //loop until we receive valid input
        while(input != '\n' && input != 'R' && input != 'r'){
            mvwprintw(window, cursor, 1, "Invalid input: please input R to reset or hit enter to continue: ");
            input = (char) wgetch(window);
            mvwprintw(window, cursor++, strlen("Invalid input: please input R to reset or hit enter to continue: ")+1, ": %c\n", input);
            free(most_recent_prompt);
            most_recent_prompt = strdup("Invalid input: please input R to reset or hit enter to continue: ");
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
    }
    
    //print exit message
    mvwprintw(window, cursor++, 1, "Board setup complete, enjoy the game!\n");
    free(most_recent_prompt);
    most_recent_prompt = strdup("Board setup complete, enjoy the game!\n");
    // free(most_recent_prompt);

    //reset cursor to top of box
    cursor = INIT_CURSOR;
    
    FILE* boardContent = fopen("finishMakeBoard.txt", "w+");
    fprintf(boardContent, "Make Board exiting\n");
    fclose(boardContent);

    //return initialized box
    return board;
}



/** updateBoardAfterGuess
 *  Function that updates the board based on the player's guess
 *  Takes a board, coordinates, bools giving information about the
 *  specified cell (to be updated), and the user's input window
 */
void updateBoardAfterGuess(board_t *board, int x, int y, bool *isHit, bool *isSunk, WINDOW *window) {
    *isHit = false;     // Initialize hit status as false
    *isSunk = false;    // Initialize sunk status as false

    // Adjust for 0-based coords
    if (x==0) x = 10;
    if (y==0) y = 10;

    // Check if the coordinates are within the valid range of the board
    if (x < 1 || x > NCOLS || y < 1 || y > NROWS) {
        mvwprintw(window, cursor++, 1, "Invalid coordinates.\n");
        free(most_recent_prompt);
        most_recent_prompt = strdup("Invalid coordinates.\n");
        return;
    }

    // Get the cell at the specified coordinates
    cell_t *cell = &board->array[x][y];

    // Check if the cell has already been guessed
    if (cell->guessed) {
        mvwprintw(window, cursor++, 1, "Cell has already been guessed.\n");
        free(most_recent_prompt);
        most_recent_prompt = strdup("Cell has already been guessed.\n");
        return;
    }

    // Mark the cell as guessed
    cell->guessed = true;

    // Check if the cell is occupied by part of a ship
    if (cell->occupied) {
        *isHit = true;  // The attack is a hit
        cell->hit = true;   // Mark the cell as hit

        shipType_t *ship = &cell->ship; // Get the ship occupying that cell
        ship->sunk = true;      // Assume the ship is sunk until proven otherwise

        // Iterate through the board to check if any part of the ship is not hit.
        for (int i = 1; i < NROWS+1; i++) {
            for (int j = 1; j < NCOLS+1; j++) {
                if (board->array[i][j].occupied && board->array[i][j].ship.name == ship->name
                        && !board->array[i][j].hit) {
                    ship->sunk = false;     // The ship is not fully sunk
                    break;
                }
            } 
            if (!ship->sunk) break;     // Exit the outer loop if the ship is not sunk
        }

        // If all parts of the ship are hit, it is sunk, so update that boolean
        if (ship->sunk) {
            *isSunk = true;
        mvwprintw(window, cursor++, 1, "Your %s has been sunk!\n", ship->name);
        free(most_recent_prompt);
        int strlength = strlen("Your  has been sunk!\n") + 1;
        strlength += strlen(ship->name);
        most_recent_prompt = malloc(sizeof(char)*strlength);
        sprintf(most_recent_prompt, "Your %s has been sunk!\n", ship->name);
        } else {
            mvwprintw(window, cursor++, 1, "Your %s got hit!\n", ship->name);
            free(most_recent_prompt);
            int strlength = strlen("Your  has been sunk!\n") + 1;
            strlength += strlen(ship->name);
            most_recent_prompt = malloc(sizeof(char)*strlength);
            sprintf(most_recent_prompt, "Your %s got hit!\n", ship->name);
        }
    } else {
        mvwprintw(window, cursor++, 1, "Your opponent missed!\n");
        free(most_recent_prompt);
        most_recent_prompt = strdup("Your opponent missed!\n");
    }

    wrefresh(window);
}



/**
 * Function to check if all ships on a player's board have been sunk
 * 
 * @param board The player's game board
 * @return true if all ships are sunk, false otherwise.
 */
bool checkVictory(board_t* board) {
    bool hasShips = false;  // Flag to check if the board contains any ships
    bool allSunk = true;

    // Iterate through all cells on the board
    for (int i = 1; i <= NROWS; i++) {
        for (int j = 1; j <= NCOLS; j++) {
            cell_t* cell = &board->array[i][j];

            // Check if the cell is occupised by a ship
            if (cell->occupied) {
                hasShips = true;    // The board has at least one ship

                // If any part of a ship is not hit, victory is false
                if (!cell->hit) {
                    allSunk = false;
                    return false;
                }
            }
        }
    }

    if(allSunk){
        FILE* file = fopen("finishMakeBoard.txt", "w+");
        fprintf(file, "flipping lose to true in checkVictory: before: %d\n", lose);
        lose = true;
        fprintf(file, "flipping lose to true in checkVictory: after: %d\n", lose);
        fclose(file);
    }
    // Return true only if the board has ships and all are sunk
    return hasShips;
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
void printStatus(board_t board, WINDOW * window, char* filename){
    FILE* boardContent = fopen(filename, "w+");
    for (int i = 1; i < NROWS+1; i++){
        for (int j = 1; j < NROWS+1; j++){
            fprintf(boardContent, "Cell %d,%d is occupied (1 is true): %d\n", i, j, board.array[i][j].occupied);
        }
    }
    fclose(boardContent);
}





// /**hitOrMiss
//  * takes an int array containing coordinates and a board, returns "HIT" if 
//  * the cell specified is occupied and hasn't yet been guessed, "GUESSED" if
//  * the cell specified has been guessed, and "MISS" if it's unoccupied and 
//  * unguessed.
//  * Assumptions: coordinates are valid and order is x,y and board is initialized, and a 10 is sent in as a 0
//  */
// char * hitOrMiss(int attackCoordsArray[2], board_t * board){
//     int x = attackCoordsArray[0];
//     if(x==0)x=10;

//     int y = attackCoordsArray[1];
//     if(y==0)y=10;

//     //save specified cell
//     cell_t cell = board->array[x][y];

//     //if cell has already been guessed
//     if(cell.guessed) return "GUESSED";
    
//     /*at this point, our cell has not been guessed, so return hit if occupied and 
//     miss otherwise, and update cell status accordingly*/
//     cell.guessed=true;
//     if(cell.occupied){
//         cell.hit = true;
//         return "HIT";
//     } else {
//         return "MISS";
//     }
// }

/**
 * Cursor tracking thread to make sure the cursor resets
 *      before if goes out of bounds of the prompt window
 * 
 * @param arg Placeholder for window
 */
static void* cursor_tracking(void* arg) {
    WINDOW* prompt_win = (WINDOW*)arg;

    int maxy, maxx;
    getmaxyx(prompt_win, maxy, maxx); // Get prompt window size

    while (tracking_active) {
        pthread_mutex_lock(&cursor_mutex);

        // Check if the cursor has reached or exceeded the bottom of the window
        if (cursor >= maxy - 1) {
            werase(prompt_win);     // Clear the window
            box(prompt_win, 0, 0);  // Redraw the box

            // Print the most recent prompt at the top of the prompt window
            if (most_recent_prompt && strlen(most_recent_prompt) > 0) {
                mvwprintw(prompt_win, INIT_CURSOR, 1, "%s", most_recent_prompt);
                cursor = INIT_CURSOR + 1;   // Set the cursor just below the most recent prompt
                if((strstr(most_recent_prompt, "A,1") != NULL) ||(strstr(most_recent_prompt, "V/H") != NULL))  cursor = INIT_CURSOR;
                space = strlen(most_recent_prompt) + 1;
            } else {
                cursor = INIT_CURSOR;
            }

            // Refresh the prompt window to reflect changes
            wrefresh(prompt_win);
        }

        pthread_mutex_unlock(&cursor_mutex);

        // Sleep briefly to avoid consuming excessive CPU
        usleep(100000); // 100 milliseconds
    }

    return NULL;
}


/** 
 * Start the thread to monitor and reset the prompt window 
 * 
 * @param prompt_win The prompt window to monitor
 */
void start_cursor_tracking(WINDOW* prompt_win) {
    tracking_active = true;
    pthread_create(&cursor_thread, NULL, cursor_tracking, prompt_win);
}

/**
 * Stop the start_cursor_tracking thread
 */
void stop_cursor_tracking() {
    tracking_active = false;
    pthread_join(cursor_thread, NULL);
}

/**
 * Thread function to monitor the game state and ed the game when a player wins
 * 
 * @param arg An array of pointers to the two player boards
 */
static void* victory_tracking(void* arg) {
    board_t** boards = (board_t**)arg;
    board_t* player1_board = boards[0];
    board_t* player2_board = boards[1];
    WINDOW* prompt_win = (WINDOW*)boards[2];

    while (game_active) {
        pthread_mutex_lock(&victory_mutex);

        // Check if Player 1 has won
        if (checkVictory(player2_board)) {
            werase(prompt_win);
            box(prompt_win, 0, 0);
            FILE* file = fopen("losedebugging.txt", "w+");
            fprintf(file, "p1 won flipping lose to true in victory_tracking: before: %d\n", lose);
            lose = true;
            fprintf(file, "p1 won flipping lose to true in victory_tracking: after: %d\n", lose);
            fclose(file);
            mvwprintw(prompt_win, 1, 1, "Player 1 wins!");
            free(most_recent_prompt);
            most_recent_prompt = strdup("Player 1 wins!");
            wrefresh(prompt_win);
            game_active = false;
        }
        // Check if player 2 has won
        else if (checkVictory(player1_board)) {
            werase(prompt_win);
            box(prompt_win, 0, 0);
            FILE* file = fopen("losedebugging.txt", "w+");
            fprintf(file, "p2 won flipping lose to true in victory_tracking: before: %d\n", lose);
            lose = true;
            fprintf(file, "p2 won flipping lose to true in victory_tracking: after: %d\n", lose);
            fclose(file);
            mvwprintw(prompt_win, 1, 1, "Player 2 wins!");
            free(most_recent_prompt);
            most_recent_prompt = strdup("Player 2 wins!");
            wrefresh(prompt_win);
            game_active = false;
        }

        // if(win){
        //     mvwprintw(prompt_win, 1, 1, "You win!");
        //     free(most_recent_prompt);
        //     most_recent_prompt = strdup("You win!");
        //     wrefresh(prompt_win);
        // }

        pthread_mutex_unlock(&victory_mutex);

        // Sleep briefly to avoid consuming excessive CPU
        usleep(100000); // 100 milliseconds    
    }

    return NULL;
}

/**
 * Start the victory-tracking thread.
 *
 * @param player1_board The board of Player 1.
 * @param player2_board The board of Player 2.
 */
void start_victory_tracking(board_t* player1_board, board_t* player2_board, WINDOW* prompt_win) {
    board_t** boards = malloc(3 * sizeof(board_t*));
    boards[0] = player1_board;
    boards[1] = player2_board;
    boards[2] = (board_t*)prompt_win; 

    game_active = true;
    pthread_create(&victory_thread, NULL, victory_tracking, boards);
}

/**
 * Stop the victory-tracking thread.
 */
void stop_victory_tracking() {
    game_active = false;
    pthread_join(victory_thread, NULL);
}