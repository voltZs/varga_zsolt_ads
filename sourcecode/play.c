#include <stdio.h>
#include <stdlib.h>

#define MARK_X 1
#define MARK_O 2

#define GAME_WON 1
#define GAME_TIE 2

#define GAME 1
#define PREFERENCES 2
#define LEADERBOARD 3

#define UNDO 1
#define REDO 2

#define EXIT 0

typedef enum { false, true } bool;

struct player{
    int mark;
    bool automated;
    char name[20];
    int best_row;
    int best_column;
};

struct turn{
    struct player * owner;
    int row;
    int column;
    struct turn * next;
};

void setupGame();
void switchPlayer(struct player *** ,struct player **, struct player **);
void initBoard(int***, int);
void traverse_board(int**, int);
void free_board(int ***, int);
void push_turn(struct turn **, struct turn *);
void traverse_turns(struct turn *);
void flush_turns(struct turn **);
struct turn * pop_turn(struct turn **);
void playTurn(struct player**, struct turn**, struct turn**, int**, int);
int checkGameCommands(char);
int playUndo(struct turn **, struct turn **, int **);
int playRedo(struct turn **, struct turn **, int **);
void checkGameState(int **, int, int, int *, struct player **);
void checkTie(int **, int, int *);
void checkHorizontal(int**, int **, int, int, int *, struct player **);
void checkVertical(int**, int **, int, int, int *, struct player **);
void checkDiagonal(int**, int **, int, int, int *, struct player **);
void push_scope(int*, int, int);
void ai_set_best_move(int**, int, struct player **);
void ai_add_weights(int**, int **, int, int *, int *, int);
int ai_check_patterns(int*, int, int);
void flush_scope(int*, int);
int opposite_mark(int);
void displayBoard(int **, int);
void printDivider(int);
void printTop(int);
void printLine(int, int*, int);
void printRow(char, int, int*);
int readinput(char s[], int);
int rowToInt(char);
char getSign(int);
void printHelpPage();
void print_image(FILE *fptr);
#define MAX_LEN 50


////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////  MAIN  ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int main(){
    char *filename = "title_ascii.txt";
    FILE *fptr = NULL;

    if((fptr = fopen(filename,"r")) == NULL)
    {
        fprintf(stderr,"error opening %s\n",filename);
        return 1;
    }

    print_image(fptr);

    fclose(fptr);
    int gb_size = 3;
    int ** board;
    initBoard(&board, gb_size);

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
    playerTwo -> automated = true;

    struct player ** currPlayer = &playerTwo;

    //setup game - ask for name, preferred mark, 1 vs 1 or 1 vs computer
    //
    while(sessionstate)
    {
        if(sessionstate == GAME)
        {
            while(1){
                displayBoard(board, gb_size);
                traverse_board(board, gb_size);
                printf("\n");
                switchPlayer(&currPlayer, &playerOne, &playerTwo);
                checkGameState(board, gb_size, winscore, &gamestate, currPlayer);
                if(gamestate)
                {
                    switchPlayer(&currPlayer, &playerOne, &playerTwo);
                    break;
                }
                playTurn(currPlayer, &history, &undoneHistory, board, gb_size);
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

void print_image(FILE *fptr)
{
   char read_string[MAX_LEN];

   while(fgets(read_string,sizeof(read_string),fptr) != NULL)
       printf("%s",read_string);
}

void switchPlayer(struct player *** currPlayer ,struct player ** playerOne, struct player ** playerTwo){
    if(**currPlayer == *playerOne){
        *currPlayer = playerTwo;
    }else{
        *currPlayer = playerOne;
    }
}



void setupGame()
{

}

void playTurn(struct player ** currPlayer, struct turn ** history, struct turn ** undoneHistory, int** board, int size){
    int column = 0;
    int row = 0;
    bool automated = (*currPlayer) -> automated;

    if(automated){
        printf("Computer making a move.\n");
        printf("Best row: %d", (*currPlayer) -> best_row);
        printf("Best column: %d", (*currPlayer) -> best_column);
        row = (*currPlayer) -> best_row;
        column = (*currPlayer) -> best_column;
        // LOGIC FOR PICKING ROW AND COLUMN AS COMPUTER
    } else {
        char input[3];
        char rowChar = 0;
        printf("Player %d (%c): Enter row followed by column\n", (*currPlayer) -> mark, getSign((*currPlayer) -> mark));
        readinput(input, 3);
        if(input[0] == ':')
        {
            int gamecommand = checkGameCommands(input[1]);
            if(gamecommand){
                if(gamecommand == UNDO){
                    if(playUndo(history, undoneHistory, board)){
                        return;
                    }
                } else if(gamecommand == REDO){
                    if(playRedo(history, undoneHistory, board)){
                        return;
                    }
                }
            }
            playTurn(currPlayer, history, undoneHistory, board, size);
            return;
        }
        rowChar = input[0];
        row = rowToInt(rowChar)-1;
        column = atoi(&input[1])-1;
    }

    if(row < 0 || row > size || column < 0 || column > size)
    {
        printf("Not a valid input. Try again.\n");
        playTurn(currPlayer, history, undoneHistory, board, size);
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
        flush_turns(undoneHistory);
        board[row][column] = (*currPlayer) -> mark;
    }  else {
        printf("This tile is already taken. Make a different move.\n");
        playTurn(currPlayer, history, undoneHistory, board, size);
        return;
    }
    return;
}

int checkGameCommands(char letter)
{
    if(letter == 'h' || letter == 'H')
    {
        printHelpPage();
    } else if(letter == 'r' || letter == 'R'  ){
        return REDO;
    } else if(letter == 'u' || letter == 'U'  ){
        return UNDO;
    }
    return 0;
}

int playUndo(struct turn ** history, struct turn ** undoneHistory, int ** board)
{
    if(*history == NULL){
        printf("No moves to undo\n");
        // return 0 if failed to undo
        return 0;
    }
    struct turn * last = pop_turn(history);
    printf("Popped turn: %c(%d,%d)\n ", getSign((last->owner)->mark), last-> row, last->column);
    push_turn(undoneHistory, last);
    printf("Pushed into undone:  %c(%d,%d)\n", getSign(((*undoneHistory)->owner)->mark), (*undoneHistory)->row, (*undoneHistory)->column);
    board[last->row][last->column] = 0;

    return 1;
}

int playRedo(struct turn ** history, struct turn ** undoneHistory, int ** board)
{
    if(*undoneHistory == NULL){
        printf("No moves to redo\n");
        // return 0 if failed to undo
        return 0;
    }
    struct turn * lastUndone = pop_turn(undoneHistory);
    printf("Popped turn: %c(%d,%d)\n ", getSign((lastUndone->owner)->mark), lastUndone-> row, lastUndone->column);
    push_turn(history, lastUndone);
    printf("Pushed into history:  %c(%d,%d)\n", getSign(((*history)->owner)->mark), (*history)->row, (*history)->column);
    board[lastUndone->row][lastUndone->column] = (lastUndone->owner)->mark;

    return 1;
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
    if(*phistory != NULL){
        if(((*phistory)->next) != NULL)
        {
            *phistory = (*phistory)-> next;
            flush_turns(phistory);
        } else {
            free(*phistory);
            (*phistory) = NULL;
        }
    }
}

void initBoard(int *** board, int size)
{
    // *board is a ** int after dereferencing
    *board = malloc(sizeof(int *) * size);;
    for(int row = 0; row<size; row++){
        (*board)[row] = malloc(sizeof(int) * size);
        for(int column = 0; column<size; column++){
            (*board)[row][column] = 0;
        }
    }
}

void traverse_board(int ** board, int size)
{
    for(int row = 0; row<size; row++)
    {
        for(int column = 0; column<size; column++){
            printf("%d ", board[row][column]);

        }
        printf("\n");
    }
}

void free_board(int *** board, int size)
{
    for(int row = 0; row<size; row++){
        free((*board)[row]);
    }
    free(*board);
}

char getSign(int mark)
{
    if(mark == MARK_X) return 'X';
    return 'O';
}

int rowToInt(char row)
{
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

void checkGameState(int** board, int size, int winscore, int * gamestate, struct player ** player)
{
    // move theee in the if above
    int ** weightboard;
    initBoard(&weightboard, size);

    checkTie(board, size, gamestate);

    checkHorizontal(board, weightboard, size, winscore, gamestate, player);
    checkVertical(board, weightboard, size, winscore, gamestate, player);
    checkDiagonal(board, weightboard, size, winscore, gamestate, player);
    if((*player) -> automated){
        ai_set_best_move(weightboard, size, player);
    }

    traverse_board(weightboard, size);
    free_board(&weightboard, size);

    return;
}

void checkTie(int ** board, int size, int * gamestate)
{
    int freeSpaces = 0;
    for(int row = 0; row<size; row++)
    {
        for(int column = 0; column<size; column++){
            if(board[row][column] == 0) freeSpaces++;
        }
    }
    if(freeSpaces == 0) *gamestate = GAME_TIE;
    return;
}


void checkHorizontal(int** board, int ** weightboard, int size, int winscore, int * gamestate, struct player ** player)
{
    if(*gamestate){
        return;
    }

    int consecutive = 1;
    int current_sign;
    int previous_sign;

    bool automated = (*player) -> automated;
    int * ai_scope = calloc(winscore, sizeof(int));
    int * ai_scope_row = calloc(winscore, sizeof(int));
    int * ai_scope_column = calloc(winscore, sizeof(int));

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
            }

            if(automated){
                push_scope(ai_scope, winscore, current_sign);
                push_scope(ai_scope_row, winscore, row);
                push_scope(ai_scope_column, winscore, column);
                // only do pattern checs if we are winsize elements into the array
                if((column+1) >= winscore){
                    int weight = ai_check_patterns(ai_scope, winscore, (*player)->mark);
                    // printf("weight: %d\n", weight);
                    ai_add_weights(board, weightboard, weight, ai_scope_row, ai_scope_column, winscore);
                }
            }

            previous_sign = current_sign;
        }
    }
    free(ai_scope);
    free(ai_scope_row);
    free(ai_scope_column);
}

void checkVertical(int** board, int ** weightboard, int size, int winscore, int * gamestate, struct player ** player)
{
    if(*gamestate){
        return;
    }

    int consecutive = 1;
    int current_sign;
    int previous_sign;

    bool automated = (*player) -> automated;
    int * ai_scope = calloc(winscore, sizeof(int));
    int * ai_scope_row = calloc(winscore, sizeof(int));
    int * ai_scope_column = calloc(winscore, sizeof(int));

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

            if(automated){
                push_scope(ai_scope, winscore, current_sign);
                push_scope(ai_scope_row, winscore, row);
                push_scope(ai_scope_column, winscore, column);
                // only do pattern checs if we are winsize elements into the array
                if((row+1) >= winscore){
                    int weight = ai_check_patterns(ai_scope, winscore, (*player)->mark);
                    // printf("weight: %d\n", weight);
                    ai_add_weights(board, weightboard, weight, ai_scope_row, ai_scope_column, winscore);
                }
            }

            previous_sign = current_sign;
        }
    }
    free(ai_scope);
    free(ai_scope_row);
    free(ai_scope_column);
}
void checkDiagonal(int** board, int ** weightboard, int size, int winscore, int * gamestate, struct player ** player)
{
    if(*gamestate){
        return;
    }

    int consecutive = 1;
    int current_sign;
    int previous_sign;

    bool automated = (*player) -> automated;
    int * ai_scope = calloc(winscore, sizeof(int));
    int * ai_scope_row = calloc(winscore, sizeof(int));
    int * ai_scope_column = calloc(winscore, sizeof(int));

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

            if(iterations == size) increment *= -1;

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

                if(automated){
                    push_scope(ai_scope, winscore, current_sign);
                    push_scope(ai_scope_row, winscore, row);
                    push_scope(ai_scope_column, winscore, column);
                    // only do pattern checs if we are winsize elements into the array
                    if((index+1) >= winscore){
                        int weight = ai_check_patterns(ai_scope, winscore, (*player)->mark);
                        // printf("weight: %d\n", weight);
                        ai_add_weights(board, weightboard, weight, ai_scope_row, ai_scope_column, winscore);
                    }
                }

                // printf("consecutive: %d, ", consecutive);
                previous_sign = current_sign;

                if(direction == 1)
                    column++;
                else if(direction ==2)
                    column--;
                row++;
            }
            iterations += increment;
        }
    }

    free(ai_scope);
    free(ai_scope_row);
    free(ai_scope_column);
}


void push_scope(int * ai_scope, int scopesize, int value)
{
    for(int i=(scopesize-1); i>0; i--){
        ai_scope[i] = ai_scope[i-1];
    }
    ai_scope[0] = value;
}

void flush_scope(int * ai_scope, int scopesize)
{
    for(int i=0; i<scopesize; i++){
        ai_scope[i] = 0;
    }
}

void ai_set_best_move(int ** weightboard, int size, struct player ** ai_player)
{
    int best_value = 0;
    int num_of_bests = 0;
    int * best_rows = malloc(sizeof(int) * size *size);
    int * best_columns = malloc(sizeof(int) * size *size);

    for(int row=0; row<size; row++){
        for(int column=0; column<size; column++){
            if (weightboard[row][column] == best_value){
                num_of_bests ++;
                best_rows[num_of_bests] =  row;
                best_columns[num_of_bests] =  column;
                best_value = weightboard[row][column];
                printf("%d", best_value);

            } else if(weightboard[row][column] > best_value)
            {
                printf("\nbest_value: ");
                num_of_bests = 0;
                best_rows[num_of_bests] = row;
                best_columns[num_of_bests] =  column;
                best_value = weightboard[row][column];
                printf("%d", best_value);
            }
        }
    }
    printf("\n");

    int random = rand() % (num_of_bests+1);

    (*ai_player) -> best_row = best_rows[random];
    (*ai_player) -> best_column = best_columns[random];
    printf("picking: %d at position(%d,%d)\n", weightboard[best_rows[random]][best_columns[random]], best_rows[random], best_columns[random]);

    free(best_rows);
    free(best_columns);
}

void ai_add_weights(int ** board, int ** weightboard, int weight, int * ai_scope_row, int * ai_scope_column, int scopesize)
{
    for(int i = 0; i<scopesize; i++){
        int row = ai_scope_row[i];
        int column = ai_scope_column[i];
        // printf("r%d c%d,\t", row+1, column+1);
        if(board[row][column] != 0) weightboard[row][column] = -1;
        if(weight > weightboard[row][column] && weightboard[row][column] >= 0){
            weightboard[row][column] = weight;
        }
    }
    // printf("\n");
}

int ai_check_patterns(int * ai_scope, int scopesize, int player_mark)
{
    int player_count = 0;
    int opponent_count = 0;

    // printf("Checking for patterns on scope:\t");
    // for(int i=0; i<scopesize; i++){
    //     printf("%d", ai_scope[i]);
    // }
    // printf("\n");

    for(int i=0; i<scopesize; i++){
        if(ai_scope[i] == player_mark)
            player_count++;
        else if(ai_scope[i] == opposite_mark(player_mark))
            opponent_count++;
    }
    if(player_count>0 && opponent_count == 0)
        return (player_count*2)+1;
    else if(player_count == 0 && opponent_count > 0)
        return (opponent_count*2);

    return 0;
}

int opposite_mark(int mark){
    if(mark == MARK_X)
        return MARK_O;
    else
        return MARK_X;
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
    printf("\t\t");
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

    printf("\t\t");
    printLine(size, values, 1);

    printf("\t");
    for(int i=0; i<8; i++){
        if(i==6){
            printf("%c", letter);
        } else {
            printf(" ");
        }
    }
    printLine(size, values, 2);

    printf("\t\t");
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
    printf("\t\t");
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

void printHelpPage()
{
    printf("THIS IS THE HELP PAGE\n");
}
