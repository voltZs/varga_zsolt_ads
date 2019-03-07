#include <stdio.h>
#include <stdlib.h>

#define MARK_X 1
#define MARK_O 2
#define GAME_WON 1
#define GAME_TIE 2
#define GAME 1
#define PREFERENCES 2
#define LEADERBOARD 3
#define EXIT 0

typedef enum { false, true } bool;

struct player{
    int mark;
    bool automated;
    char name[20];
};

struct turn{
    struct player * owner;
    int row;
    int column;
    struct turn * next;
};

void setupGame();
void initBoard(int***, int);
void traverse_board(int**, int);
void push_turn(struct turn **, struct turn *);
void traverse_turns(struct turn *);
void flush_turns(struct turn **);
struct turn * pop_turn(struct turn **);
void playTurn(struct player**, struct turn**, bool, int**, int);
void makeMove();
void checkGameState(int **, int, int, int *, struct player **);
void displayBoard(int **, int);
void printDivider(int);
void printTop(int);
void printLine(int, int*, int);
void printRow(char, int, int*);
int readinput(char s[], int);
int rowToInt(char);
char getSign(int);

// 0 => NO INPUT, 1 => X, 2 => O
// int board[size][size]= {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}};

//other prompts: new players, end game, show leaderboard,




////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////  MAIN  ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int main(){
    int gb_size = 4;
    int ** board;
    initBoard(&board, gb_size);
    traverse_board(board, gb_size);

    int winscore = 3;
    int gamestate = 0;
    int sessionstate = GAME;
    // both implemented as stacks
    struct turn * history;
    history = NULL;
    struct turn * undoneHistory;
    undoneHistory = NULL;

    struct player * playerOne;
    struct player * playerTwo;

    playerOne = malloc(sizeof(struct player));
    playerOne -> mark = MARK_X;
    playerOne -> automated = false;

    playerTwo = malloc(sizeof(struct player));
    playerTwo -> mark = MARK_O;
    playerTwo -> automated = false;

    struct player ** currPlayer = &playerTwo;

    //setup game - ask for name, preferred mark, 1 vs 1 or 1 vs computer
    //
    while(sessionstate)
    {
        if(sessionstate == GAME)
        {
            while(1){
                displayBoard(board, gb_size);
                checkGameState(board, gb_size, winscore, &gamestate, currPlayer);

                if(gamestate)
                {
                    break;
                } else {
                    if(*currPlayer == playerOne){
                        currPlayer = &playerTwo;
                    }else{
                        currPlayer = &playerOne;
                    }
                }

                playTurn(currPlayer, &history, (*currPlayer)->automated, board, gb_size);
            }

            if(gamestate == GAME_WON){
                printf("Player %d (%c) won!\n", (*currPlayer) -> mark, getSign((*currPlayer) -> mark));
            } else if(gamestate == GAME_TIE){
                printf("It's a tie!\n");
            }

            // test/ demontration of using pop
            // struct turn * abcd = pop_turn(&history);
            // printf("Popped turn: %c(%d,%d);;; ", getSign((abcd->owner)->mark), abcd-> row, abcd->column);
            // however when popping we would just add it straight to undoneHistory:
            // push(&undoneHistory, pop_turn(&history));

            // traverse_turns(history);
            // flush_turns(&history);
            // traverse_turns(history);

        }

        if(sessionstate == LEADERBOARD)
        {

        }

        if(sessionstate == PREFERENCES)
        {

        }


        sessionstate = EXIT;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////





void setupGame()
{

}

void playTurn(struct player ** currPlayer, struct turn ** history, bool automated, int** board, int size){
    int column = 0;
    int row = 0;

    if(automated){
        printf("Computer making a move.\n");
        // LOGIC FOR PICKING ROW AND COLUMN AS COMPUTER
    } else {
        char input[3];
        char rowChar = 0;
        printf("Player %d (%c): Enter row followed by column\n", (*currPlayer) -> mark, getSign((*currPlayer) -> mark));
        readinput(input, 3);
        rowChar = input[0];
        row = rowToInt(rowChar)-1;
        column = atoi(&input[1])-1;
    }

    if(row < 0 || row > size || column < 0 || column > size)
    {
        printf("Not a valid input. Try again.\n");
        playTurn(currPlayer, history, automated, board, size);
        return;
    }

    // check if chosen tile is free
    if(board[row][column]  == 0)
    {
        struct turn * currTurn;
        currTurn = malloc(sizeof(struct turn));
        currTurn -> owner = (*currPlayer);
        currTurn -> row = row;
        currTurn -> column = column;
        push_turn(history, currTurn);
        board[row][column] = (*currPlayer) -> mark;
    }  else {
        printf("This tile is already taken. Make a different move.\n");
        playTurn(currPlayer, history, automated, board, size);
        return;
    }
    return;
}

void push_turn(struct turn ** phistory, struct turn * turn)
{

    if(*phistory == NULL){
        turn -> next = NULL;
        *phistory = turn;
    } else {
        turn -> next = *phistory;
        *phistory = turn;
    }
}

struct turn * pop_turn(struct turn ** phistory)
{
    struct turn * tmp;
    if(*phistory == NULL){
        printf("Stack is empty\n");
    } else {
        tmp = (*phistory);
        (*phistory) = (*phistory) -> next;
    }
    return tmp;
}

void traverse_turns(struct turn * history)
{
    while(history != NULL)
    {
        printf("%c: (%d,%d); ", getSign((history-> owner) -> mark),history -> row,  history -> column);
        history = history -> next;
    }
    printf("NULL\n");
}

void flush_turns(struct turn ** phistory)
{
    if(((*phistory)->next) != NULL)
    {
        *phistory = (*phistory)-> next;
        flush_turns(phistory);
    } else {
        free(*phistory);
        (*phistory) = NULL;
    }
}

void initBoard(int *** board, int size){
    // *board is a ** int after dereferencing
    *board = malloc(sizeof(int *) * size);;
    for(int row = 0; row<size; row++){
        (*board)[row] = malloc(sizeof(int) * size);
        for(int column = 0; column<size; column++){
            (*board)[row][column] = 0;
        }
    }
}

void traverse_board(int ** board, int size){
    for(int row = 0; row<size; row++)
    {
        for(int column = 0; column<size; column++){
            printf("%d\t", board[row][column]);

        }
        printf("\n");
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

void checkGameState(int** board, int size, int winscore, int * gamestate, struct player ** player){
    int consecutive = 1;
    int current_sign;
    int previous_sign;
    int freeSpaces = 0;

    bool automated = (*player) -> automated;


    // check horizontal - ALSO CHECKS IF GAME IS A TIE
    for(int row = 0; row<size; row++){
        // reset values on new row
        previous_sign = 0;
        consecutive = 1;
        for(int column = 0; column<size; column++){
            current_sign = board[row][column];
            if(current_sign == MARK_X || current_sign == MARK_O){
                if(current_sign == previous_sign){
                    consecutive++;
                    if(consecutive == winscore){
                        *gamestate = GAME_WON;
                        return;
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
    for(int column = 0; column<size; column++){
        // reset values on new row
        previous_sign = 0;
        consecutive = 1;
        for(int row = 0; row<size; row++){
            current_sign = board[row][column];
            if(current_sign == MARK_X || current_sign == MARK_O){
                if(current_sign == previous_sign){
                    consecutive++;
                    if(consecutive == winscore){
                        *gamestate = GAME_WON;
                        return;
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
        iterations = winscore;
        while(iterations >= winscore){
            previous_sign = 0;
            consecutive = 1;

            if(iterations == size){
                increment *= -1;
            }

            if(direction == 1){
                // while rising
                if(increment == 1){
                    column = size-(iterations-1)-1;
                    row = 0;
                // while decreasing
                } else if(increment == -1){
                    column = 0;
                    row = size-(iterations-1)-1;
                }
            } else if(direction ==2){
                // while rising
                if(increment == 1){
                    column = (iterations-1);
                    row = 0;
                // while decreasing
                } else if(increment == -1){
                    column = size-1;
                    row = size-iterations;
                }
            }

            for(int index = 0; index<iterations; index++){
                current_sign = board[row][column];
                if(current_sign == MARK_X || current_sign == MARK_O){
                    if(current_sign == previous_sign){
                        consecutive++;
                        if(consecutive == winscore){
                            *gamestate = GAME_WON;
                            return;
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
        *gamestate = GAME_TIE;
        return;
    }

    return;
}

void displayBoard(int ** board, int size)
{
    printf("\n");

    char letter = 'A';
    printTop(size);

    for(int i = 0; i< size; i++){
        printRow(letter+i, size, board[i]);
    }
    printDivider(size);
    printf("\n");
}

void printTop(int size)
{
    printf("\t");
    printf("    ");
    for(int i=0; i<size; i++){
        printf("%d", i+1);
        printf("       ");
    }
    printf("\n");
}

void printRow(char letter, int size, int* values)
{
    printDivider(size);

    printf("\t");
    printLine(size, values, 1);

    for(int i=0; i<8; i++){
        if(i==6){
            printf("%c", letter);
        } else {
            printf(" ");
        }
    }
    printLine(size, values, 2);

    printf("\t");
    printLine(size, values, 3);
}

void printLine(int size, int* values, int vertical)
{
    for(int i=0; i<(size); i++){
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

void printDivider(int size)
{
    printf("\t");
    for(int i=0; i<((size*7)+(size+1)); i++){
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
