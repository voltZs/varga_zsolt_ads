#include <stdio.h>
#include <stdlib.h>

#define GB_SIZE 3
#define MARK_X 1
#define MARK_O 2
#define WIN_SCORE 3
#define GAME_WON 1
#define GAME_TIE 2

typedef enum { false, true } bool;

struct turn{
    int player;
    int positionX;
    int positionY;
    struct turn * next;
};

struct player{
    int mark;
    char name[20];
};

void setupGame();
void playTurn();
void makeMove();
int checkGameState();
void displayBoard();
void printDivider(int);
void printTop(int);
void printLine(int, int*, int);
void printRow(char, int, int*);
int readinput(char s[], int);
int rowToInt(char);
char getSign(int);

int gamestate = 0;
bool runningSession = true;
int whosTurn = MARK_X;
int player1 = MARK_X;
int player2 = MARK_O;
// 0 => NO INPUT, 1 => X, 2 => O
int board[GB_SIZE][GB_SIZE]= {{0,0,0}, {0,0,0}, {0,0,0}};

//other prompts: new players, end game, show leaderboard,




////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////  MAIN  ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int main(){
    // both implemented as stacks
    struct turn * history;
    struct turn * undoneHistory;

    //setup game - ask for name, preferred mark, 1 vs 1 or 1 vs computer
    //
    while(runningSession)
    {
        // printf("%c\n", 'a'-32);
        while(!gamestate){
            // different logic for both modes, duh
            // ask about turn
            displayBoard();
            playTurn();
        }
        displayBoard();
        if(gamestate == GAME_WON){
            printf("Player %d (%c) won!\n", whosTurn, getSign(whosTurn));
        } else if(gamestate == GAME_TIE){
            printf("It's a tie!\n");
        }

        // prompt whether they want to continue playing or naw
        runningSession = false;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////





void setupGame()
{

}

void playTurn(){
    char row = 0;
    int rowNumeral = 0;
    int column = 0;
    char input[3];
    printf("Player %d (%c): Enter row followed by column\n", whosTurn, getSign(whosTurn));
    readinput(input, 3);
    row = input[0];
    rowNumeral = rowToInt(row);
    column = atoi(&input[1]);


    if(rowNumeral <= 0 || rowNumeral > GB_SIZE || column <= 0 || column > GB_SIZE)
    {
        printf("Not a valid input. Try again.\n");
        playTurn();
        return;
    }

    // check if chosen tile is free
    if(board[rowNumeral-1][column-1]  == 0)
    {
        board[rowNumeral-1][column-1] = whosTurn;
    }  else {
        printf("This tile is already taken. Make a different move.\n");
        playTurn();
        return;
    }

    gamestate = checkGameState();
    if(gamestate){
        return;
        // return so that whosTurn below is not swapped!
    }

    if(whosTurn == MARK_X){
        whosTurn = MARK_O;
    }else{
        whosTurn = MARK_X;
    }

}

char getSign(int mark){
    if(mark == MARK_X){
        return 'X';
    }
    return 'O';
}

int rowToInt(char row){
    int numeric = (int)row;
    if(numeric >= 65 && numeric <= 90){
        // do nothing
    } else if(numeric >= 97 || numeric <=122){
        // if lowercase turn to uppercase
        numeric -= 32;
    } else {
        numeric = 0;
    }
    numeric -=64;
    return numeric;
}

int checkGameState(){
    int consecutive = 1;
    int current_sign;
    int previous_sign;
    int freeSpaces = 0;

    // check horizontal - ALSO CHECKS IF GAME IS A TIE
    for(int row = 0; row<GB_SIZE; row++){
        // reset values on new row
        previous_sign = 0;
        consecutive = 1;
        for(int column = 0; column<GB_SIZE; column++){
            current_sign = board[row][column];
            if(current_sign == MARK_X || current_sign == MARK_O){
                if(current_sign == previous_sign){
                    consecutive++;
                    if(consecutive == WIN_SCORE){
                        return GAME_WON;
                    }
                } else {
                    consecutive = 1;
                }
            } else {
                consecutive = 1;
                freeSpaces++;
            }
            previous_sign = current_sign;
        }
    }

    // check vertical
    for(int column = 0; column<GB_SIZE; column++){
        // reset values on new row
        previous_sign = 0;
        consecutive = 1;
        for(int row = 0; row<GB_SIZE; row++){
            current_sign = board[row][column];
            if(current_sign == MARK_X || current_sign == MARK_O){
                if(current_sign == previous_sign){
                    consecutive++;
                    if(consecutive == WIN_SCORE){
                        return GAME_WON;
                    }
                } else {
                    consecutive = 1;
                }
            } else {
                consecutive = 1;
            }
            previous_sign = current_sign;
        }
    }

    // check diagonal
    int increment;
    int iterations;
    int column;
    int row;
    // diagonal needs to be checked left to right and the opposite direction as WELL
    // hence the wrapping in the first for loop!
    // direction 1 -> left to right direction 2 -> right to left (both top to bottom)
    for(int direction = 1; direction<3; direction++){
        increment = 1;
        iterations = WIN_SCORE;
        while(iterations >= WIN_SCORE){
            previous_sign = 0;
            consecutive = 1;

            if(iterations == GB_SIZE){
                increment *= -1;
            }

            if(direction == 1){
                // while rising
                if(increment == 1){
                    column = GB_SIZE-(iterations-1)-1;
                    row = 0;
                // while decreasing
                } else if(increment == -1){
                    column = 0;
                    row = GB_SIZE-(iterations-1)-1;
                }
            } else if(direction ==2){
                // while rising
                if(increment == 1){
                    column = (iterations-1);
                    row = 0;
                // while decreasing
                } else if(increment == -1){
                    column = GB_SIZE-1;
                    row = GB_SIZE-iterations;
                }
            }

            for(int index = 0; index<iterations; index++){
                current_sign = board[row][column];
                if(current_sign == MARK_X || current_sign == MARK_O){
                    if(current_sign == previous_sign){
                        consecutive++;
                        if(consecutive == WIN_SCORE){
                            return GAME_WON;
                        }
                    } else {
                        consecutive = 1;
                    }
                } else {
                    consecutive = 1;
                }
                // printf("consecutive: %d, ", consecutive);
                previous_sign = current_sign;

                if(direction == 1){
                    column++;
                } else if(direction ==2){
                    column--;
                }
                row++;
            }
            iterations += increment;
        }
    }


    if(!freeSpaces){
        return GAME_TIE;
    }

    return 0;
}

void displayBoard(){
    int width = (sizeof(board[0]))/(sizeof(board[0][0]));
    int height = (sizeof(board))/(sizeof(board[0]));
    printf("\n");

    char letter = 'A';
    printTop(width);

    for(int i = 0; i< height; i++){
        printRow(letter+i, width, board[i]);
    }
    printDivider(width);
    printf("\n");
}

void printTop(int width){
    printf("\t");
    printf("    ");
    for(int i=0; i<width; i++){
        printf("%d", i+1);
        printf("       ");
    }
    printf("\n");
}

void printRow(char letter, int width, int* values){
    printDivider(width);

    printf("\t");
    printLine(width, values, 1);

    for(int i=0; i<8; i++){
        if(i==6){
            printf("%c", letter);
        } else {
            printf(" ");
        }
    }
    printLine(width, values, 2);

    printf("\t");
    printLine(width, values, 3);
}

void printLine(int width, int* values, int vertical){
    for(int i=0; i<(width); i++){
        printf("|");
        //EMPTY
        if(values[i]==0){
            printf("\t");
        }
        //X
        if(values[i]==1){
            // TOP OR BOTTOM ROW OF X
            if(vertical==1){
                printf(" \\\\ // ");
            }
            // SECOND ROW OF X
            if(vertical ==2){
                printf("   X   ");
            }
            // BOTTOM ROW OF X
            if(vertical==3){
                printf(" // \\\\ ");
            }
        }
        //O
        if(values[i]==2){
            // TOP OR BOTTOM ROW OF 0
            if(vertical==1 || vertical==3){
                printf("  ooo  ");
            }
            // SECOND ROW OF 0
            if(vertical ==2){
                printf(" O   O ");
            }
            // THIRD ROW OF 0
            if(vertical ==3){

            }
        }
    }
    printf("|\n");
}

void printDivider(int width){
    printf("\t");
    for(int i=0; i<((width*7)+(width+1)); i++){
        printf("-");
    }
    printf("\n");
}

int readinput(char s[], int maxlen)
{
    char ch;
    int i = 0;
    int keep_reading = 1;

    while(keep_reading)
    {
        ch = getchar();
        if((ch == '\n') || (ch == EOF))
        {
            keep_reading = 0;
        } else if(i < maxlen-1){
            s[i] = ch;
            i++;
        }
    }
    s[i] = '\0';
    return i;
}
