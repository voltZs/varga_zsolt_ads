#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MARK_X 1
#define MARK_O 2

#define VS_HUMAN 1
#define VS_COMPUTER 2

#define GAME_WON 1
#define GAME_TIE 2
#define GAME_ON 0

#define GAME 1
#define OLD_GAMES 2
#define PREFERENCES 3
#define MENU 4
#define EXIT 0

#define UNDO 1
#define REDO 2

#define MAX_STR_LEN 250
#define MAX_NAME_LEN 13
#define MAX_COMMAND_LEN 4
#define MAX_GAMES_LISTED 10

typedef enum { false, true } bool;

struct player{
    int mark;
    bool automated;
    char * name;
    int wins;
    int best_row;
    int best_column;
};

struct turn{
    struct player * owner;
    int row;
    int column;
    struct turn * next;
};

void chooseMode(int *);
void setupPlayers(struct player *, struct player *);
void changePlayerName(struct player *, char*);
void switchPlayer(struct player *** ,struct player **, struct player **);
void initBoard(int***, int);
void traverse_board(int**, int);
void free_board(int ***, int);
void push_turn(struct turn **, struct turn *);
void traverse_turns(struct turn *);
void flush_turns(struct turn **);
struct turn * pop_turn(struct turn **);
void playTurn(struct player**, struct turn**, int*, struct turn**, int**, int);
int checkGameCommands(char);
void promptNextRound(int*);
int playUndo(struct turn **, int*, struct turn **, int **);
int playRedo(struct turn **, int*, struct turn **, int **);
void saveGame(char *, struct player *,  struct player *,  struct player *, int, int, struct turn **, int*);
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
void browseOldGames(int*, int);
void replayGame(char **, int, int);
void rp_viewboardstate(int, int *, int *, int, int);
void listOldGames(int , int, int , int *, char *** );
void changePreferences(int*, int *, int *, struct player *);
void changeGameboardSize(int *, int *);
void changeWinScore(int *, int *);
void changeGameMode(struct player *);
char * getAutomated(struct player *);
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
void print_ascii(char *);
void wtfile_end(char *, char *);
int get_csv_size(char *);
int * get_csv_linesizes(char *, int);
char *** get_csv_array(char *, int, int*);



////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////  MAIN  ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int main(){
    print_ascii("title_ascii.txt");

    int gb_size = 3;
    int ** board;
    int winscore = 3;
    int gamestate = 0;
    int sessionstate = MENU;
    // both implemented as stacks
    struct turn * history;
    history = NULL;
    int turns_taken = 0;
    struct turn * undoneHistory;
    undoneHistory = NULL;

    struct player * playerOne;
    struct player * playerTwo;
    playerOne = malloc(sizeof(struct player));
    playerOne -> name = NULL;
    playerOne -> mark = MARK_X;
    playerOne -> automated = false;
    playerTwo = malloc(sizeof(struct player));
    playerTwo -> name = NULL;
    playerTwo -> mark = MARK_O;
    playerTwo -> automated = true;

    struct player ** currPlayer = &playerTwo;

    //setup game - ask for name, preferred mark, 1 vs 1 or 1 vs computer
    //
    while(sessionstate == MENU)
    {
        chooseMode(&sessionstate);
        while(sessionstate == GAME)
        {
            if(playerOne -> name == NULL) setupPlayers(playerOne, playerTwo);
            initBoard(&board, gb_size);
            while(gamestate == GAME_ON){
                printf("WINS:\t%s %d --- %d %s\n\n", playerOne -> name, playerOne -> wins, playerTwo-> wins, playerTwo -> name);
                displayBoard(board, gb_size);
                switchPlayer(&currPlayer, &playerOne, &playerTwo);
                checkGameState(board, gb_size, winscore, &gamestate, currPlayer);
                if(gamestate)
                {
                    switchPlayer(&currPlayer, &playerOne, &playerTwo);
                    break;
                }
                playTurn(currPlayer, &history, &turns_taken, &undoneHistory, board, gb_size);
            }
            if(gamestate == GAME_WON){
                printf("Player %d (%c) won!\n", (*currPlayer) -> mark, getSign((*currPlayer) -> mark));
                (*currPlayer) -> wins++;
            } else if(gamestate == GAME_TIE){
                printf("It's a tie!\n");
            }
            saveGame("games_history.txt", playerOne, playerTwo, *currPlayer, gb_size, winscore, &history, &turns_taken);
            free_board(&board, gb_size);
            gamestate = GAME_ON;
            promptNextRound(&sessionstate);
        }
        playerOne -> wins = 0;
        playerTwo -> wins = 0;

        if(sessionstate == OLD_GAMES)
        {
            browseOldGames(&sessionstate, -1);
        }

        if(sessionstate == PREFERENCES)
        {
            changePreferences(&sessionstate, &gb_size, &winscore, playerTwo);
        }
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void browseOldGames(int * sessionstate, int start)
{
    int csv_size = get_csv_size("games_history.txt");
    int * linesizes = get_csv_linesizes("games_history.txt", csv_size);
    char *** csv_array = get_csv_array("games_history.txt", csv_size, linesizes);

    char input[MAX_COMMAND_LEN];
    printf("\n-PREVIOUS GAMES-\n\n");
    printf("\tID\tPlayer 1\tPlayer 2\tWinner\tBoard\tRow\tDate\n");
    printf("\t----------------------------------------------------------------------------------------\n");

    int start_id;
    if(start == -1)
        start_id = csv_size-1;
    else
        start_id = start;
    int final_id = start_id - MAX_GAMES_LISTED;
    listOldGames(start_id, final_id, csv_size, linesizes, csv_array);
    printf("\nEnter game ID to replay game. Move up and down the list by entering W/S.\nEnter Q to exit.\n\n");

    readinput(input, MAX_COMMAND_LEN);
    int id_to_replay = atoi(input);
    if(input[0] == 's' || input[0] == 'S'){
        if(final_id >= 0) browseOldGames(sessionstate, final_id);
        else browseOldGames(sessionstate, start_id);
    }
    else if (input[0] == 'w' || input[0] == 'W'){
        if((start_id+MAX_GAMES_LISTED)<=(csv_size-1)) browseOldGames(sessionstate, start_id+MAX_GAMES_LISTED);
        else browseOldGames(sessionstate, csv_size -1);
    }
    else if (input[0] == 'q' || input[0] == 'Q' )*sessionstate = MENU;
    else if (id_to_replay<csv_size && id_to_replay>0) {
        replayGame(csv_array[id_to_replay-1], linesizes[id_to_replay-1], 1);
        browseOldGames(sessionstate, start_id);
    }
    else browseOldGames(sessionstate, start_id);
}

void replayGame(char ** line, int num_of_items, int position)
{
    int rp_boardsize = atoi(line[6]);
    int rp_mark = atoi(line[8]);
    int rp_num_of_turns = (num_of_items-9)/2;
    int * rp_rows = malloc(sizeof(int) * rp_num_of_turns);
    int * rp_columns = malloc(sizeof(int) * rp_num_of_turns);
    if(position<=1) position = 1;
    if(position > rp_num_of_turns) position = rp_num_of_turns;

    for(int i=0; i< rp_num_of_turns; i++){
        rp_rows[i] = atoi(line[9+(i*2)]);
        rp_columns[i] = atoi(line[9+(i*2)+1]);
    }

    char input[MAX_COMMAND_LEN];
    printf("\tREPLAYING GAME: %s(%c) VS %s(%c)\n", line[0],getSign(atoi(line[1])),line[2],getSign(atoi(line[3])));
    printf("\tWINNER:\t\t(%c)\n",getSign(atoi(line[4])));
    printf("\tCURRENT MOVE:\t");
    for(int i=0; i< position-1; i++) printf(" -");
    if(position%2==0) printf(" %c", getSign(opposite_mark(rp_mark)));
    else printf(" %c", getSign(rp_mark));
    for(int i=0; i<rp_num_of_turns-position; i++) printf(" -");


    rp_viewboardstate(rp_boardsize, rp_rows, rp_columns, position, rp_mark);

    readinput(input, MAX_COMMAND_LEN);
    if(input[0] == 'a' || input[0] == 'A') replayGame(line, num_of_items, position-1);
    else if(input[0] == 'd' || input[0] == 'D') replayGame(line, num_of_items, position+1);
    else if(input[0] == 'q' || input[0] == 'Q') return;
    else replayGame(line, num_of_items, position);
}

void rp_viewboardstate(int boardsize, int * rows, int * columns, int iterations, int mark)
{
    int ** rp_board;
    initBoard(&rp_board, boardsize);

    for(int i=0; i<iterations; i++){
        int row = rows[i];
        int column = columns[i];
        rp_board[row][column] = mark;

        if(mark == MARK_X) mark = MARK_O;
        else mark = MARK_X;
    }

    displayBoard(rp_board, boardsize);
    free_board(&rp_board, boardsize);

}

void listOldGames(int start_id, int final_id , int csv_size, int * linesizes, char *** csv_array)
{
    for(int i = start_id; i>=0; i--){
        if(i==final_id) break;
        char ** line = csv_array[i];
        int num_of_items = linesizes[i];
        printf("\t%d", i+1);
        printf("\t%s (%c)", line[0], getSign(atoi(line[1])));
        printf("\t%s (%c)", line[2], getSign(atoi(line[3])));
        printf("\t%c", getSign(atoi(line[4])));
        printf("\t%sx%s", line[6], line[6]);
        printf("\t%s", line[7]);
        printf("\t%s", line[5]);
        printf("\n");
    }

}


void changePreferences(int * sessionstate, int * gb_size, int * winscore, struct player * playerTwo){
    char input[MAX_COMMAND_LEN];
    printf("\n-PREFERENCES-\n");
    printf("\tENTER OPTION TO MODIFY:\n");
    printf("\t1 | GAMEBOARD SIZE\t(%d)\n", *gb_size);
    printf("\t2 | WIN ROW SIZE\t(%d)\n", *winscore);
    printf("\t3 | OPPONENT\t\t(%s)\n", getAutomated(playerTwo));
    printf("\t4 | BACK TO MAIN MENU\n\n");
    readinput(input, MAX_COMMAND_LEN);
    if(input[0] == '1') changeGameboardSize(gb_size, winscore);
    else if (input[0] == '2') changeWinScore(gb_size, winscore);
    else if (input[0] == '3') changeGameMode(playerTwo);
    else if (input[0] == '4') *sessionstate = MENU;
    if(input[0] != '4'){
        changePreferences(sessionstate, gb_size, winscore, playerTwo);
    }
}

void changeGameboardSize(int * gb_size, int * winscore)
{
    char input[MAX_COMMAND_LEN];
    printf("Enter new gameoard size (3 - 10):\n");
    readinput(input, MAX_COMMAND_LEN);
    int new_size = atoi(input);
    if(new_size <=10 && new_size > 2){
        *gb_size = new_size;
        if(new_size<= *winscore) *winscore = new_size;
    } else {
        printf("This value is not valid. ");
        changeGameboardSize(gb_size, winscore);
    }
}
void changeWinScore(int * gb_size, int * winscore)
{
    char input[MAX_COMMAND_LEN];
    printf("Enter new win score size (3 - 10):\n");
    readinput(input, MAX_COMMAND_LEN);
    int new_size = atoi(input);
    if(new_size <= 10 && new_size > 2){
        *winscore = new_size;
        if(new_size >= *gb_size) *gb_size = new_size;
    } else {
        printf("This value is not valid. ");
        changeWinScore(gb_size, winscore);
    }
}
void changeGameMode(struct player * playerTwo)
{
    char input[MAX_COMMAND_LEN];
    printf("Would you like to play agains person or computer? p/c\n");
    readinput(input, MAX_COMMAND_LEN);
    if(input[0] == 'p' || input[0] == 'P')
        playerTwo->automated = false;
    else if (input[0] == 'c' || input[0] == 'C')
        playerTwo->automated = true;
    else {
        printf("Not a valid input.\n");
        changeGameMode(playerTwo);
    }
}

char * getAutomated(struct player * player)
{
    if(player -> automated)
        return "COMPUTER";
    else
        return "HUMAN";
}

void chooseMode(int * sessionstate)
{
    char input[MAX_COMMAND_LEN];
    printf("\n-MAIN MENU-\n");
    printf("\tENTER OPTION AND PRESS ENTER:\n\t1 | NEW GAME\n\t2 | PREVIOUS GAMES\n\t3 | PREFERENCES\n\t4 | QUIT\n\n");
    readinput(input, MAX_COMMAND_LEN);
    if(input[0] == '1') *sessionstate = GAME;
    else if (input[0] == '2') *sessionstate = OLD_GAMES;
    else if (input[0] == '3') *sessionstate = PREFERENCES;
    else if (input[0] == '4') *sessionstate = EXIT;
    else chooseMode(sessionstate);
}

void promptNextRound(int * sessionstate){
    char input[MAX_COMMAND_LEN];
    printf("Play again? y/n\n");
    readinput(input, MAX_COMMAND_LEN);
    if(input[0] == 'y' || input[0] == 'Y') *sessionstate = GAME;
    else if (input[0] == 'n' || input[0] == 'N') *sessionstate = MENU;
    else promptNextRound(sessionstate);
}

void setupPlayers(struct player * playerOne, struct player * playerTwo)
{
    playerOne -> wins = 0;
    playerTwo -> wins = 0;
    changePlayerName(playerOne, "Player 1");
    if(playerTwo->automated){
        playerTwo->name = "Computer";
    } else {
        changePlayerName(playerTwo, "Player 2");
    }
    printf("\n");

}

void changePlayerName(struct player * player, char* default_name)
{
    char input[MAX_NAME_LEN];
    printf("%s: Enter your name to customise or press enter.\n", default_name);
    readinput(input, MAX_NAME_LEN);
    char * string = malloc(sizeof(char)*MAX_NAME_LEN);
    for(int i=0; i<MAX_NAME_LEN; i++){
        string[i] = input[i];
        if(input[i] == '\0') break;
    }
    if(input[0] == '\0'){
        string = default_name;
        player->name = string;
    }
    player->name = string;

}

void saveGame(char * filename, struct player * playerOne,
            struct player * playerTwo,  struct player * currPlayer,
            int gb_size, int winscore, struct turn ** history, int * turns_taken)
{
    FILE *file = NULL;
    if((file = fopen(filename,"a")) == NULL)
    {
        printf("Couldn't open file %s\n",filename);
        return;
    }

    time_t now;
    time(&now);
    char * timestr = ctime(&now);
    int ind;
    for(ind=0; timestr[ind]!='\n'; ind++);
    timestr[ind] = '\0';

    //player1
    fprintf(file, "%s,", playerOne->name);
    fprintf(file, "%d,", playerOne->mark);
    //player2
    fprintf(file, "%s,", playerTwo->name);
    fprintf(file, "%d,", playerTwo->mark);
    // winner
    fprintf(file, "%d,", currPlayer->mark);

    fprintf(file, "%s,", timestr);
    fprintf(file, "%d,", gb_size);
    fprintf(file, "%d,", winscore);

    struct turn ** turn_array = malloc(sizeof(struct turn*) * (*turns_taken));
    for(int i=((*turns_taken)-1); i>=0; i--){
        turn_array[i] = pop_turn(history);
    }

    // mark of first turn
    fprintf(file, "%d,", (turn_array[0]->owner)->mark);
    for(int i=0; i<(*turns_taken); i++){
        fprintf(file, "%d,", turn_array[i] -> row);
        fprintf(file, "%d", turn_array[i] -> column);
        if(i==(*turns_taken)-1) fprintf(file, "\n");
        else fprintf(file, ",");
        free(turn_array[i]);
    }

    free(turn_array);
    *turns_taken = 0;
    *history = NULL;
    fclose(file);
}

void switchPlayer(struct player *** currPlayer ,
                struct player ** playerOne, struct player ** playerTwo)
{
    if(**currPlayer == *playerOne){
        *currPlayer = playerTwo;
    }else{
        *currPlayer = playerOne;
    }
}

void playTurn(struct player ** currPlayer, struct turn ** history,
                    int* turns_taken, struct turn ** undoneHistory,
                    int** board, int size)
{
    int column = 0;
    int row = 0;
    bool automated = (*currPlayer) -> automated;

    if(automated){
        printf("Computer (%c) made a move.\n", getSign((*currPlayer)->mark));
        row = (*currPlayer) -> best_row;
        column = (*currPlayer) -> best_column;
        // LOGIC FOR PICKING ROW AND COLUMN AS COMPUTER
    } else {
        char input[3];
        char rowChar = 0;
        printf("%s (%c): Enter row followed by column\n", (*currPlayer) -> name, getSign((*currPlayer) -> mark));
        readinput(input, 3);
        if(input[0] == ':')
        {
            int gamecommand = checkGameCommands(input[1]);
            if(gamecommand){
                if(gamecommand == UNDO){
                    if(playUndo(history, turns_taken, undoneHistory, board)){
                        return;
                    }
                } else if(gamecommand == REDO){
                    if(playRedo(history, turns_taken, undoneHistory, board)){
                        return;
                    }
                }
            }
            playTurn(currPlayer, history, turns_taken, undoneHistory, board, size);
            return;
        }
        rowChar = input[0];
        row = rowToInt(rowChar)-1;
        column = atoi(&input[1])-1;
    }

    if(row < 0 || row > size || column < 0 || column > size)
    {
        printf("Not a valid input. Try again.\n");
        playTurn(currPlayer, history, turns_taken, undoneHistory, board, size);
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
        (*turns_taken)++;
        flush_turns(undoneHistory);
        board[row][column] = (*currPlayer) -> mark;
    }  else {
        printf("This tile is already taken. Make a different move.\n");
        playTurn(currPlayer, history, turns_taken, undoneHistory, board, size);
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

int playUndo(struct turn ** history, int *turns_taken,
        struct turn ** undoneHistory, int ** board)
{
    if(*history == NULL){
        printf("No moves to undo\n");
        // return 0 if failed to undo
        return 0;
    }
    struct turn * last = pop_turn(history);
    (*turns_taken)--;
    printf("Popped turn: %c(%d,%d)\n ", getSign((last->owner)->mark), last-> row, last->column);
    push_turn(undoneHistory, last);
    printf("Pushed into undone:  %c(%d,%d)\n", getSign(((*undoneHistory)->owner)->mark), (*undoneHistory)->row, (*undoneHistory)->column);
    board[last->row][last->column] = 0;

    return 1;
}

int playRedo(struct turn ** history, int *turns_taken,
        struct turn ** undoneHistory, int ** board)
{
    if(*undoneHistory == NULL){
        printf("No moves to redo\n");
        // return 0 if failed to undo
        return 0;
    }
    struct turn * lastUndone = pop_turn(undoneHistory);
    (*turns_taken)++;
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

void checkGameState(int** board, int size, int winscore, int * gamestate,
                struct player ** player)
{
    // move theee in the if above
    int ** weightboard;
    initBoard(&weightboard, size);

    checkHorizontal(board, weightboard, size, winscore, gamestate, player);
    checkVertical(board, weightboard, size, winscore, gamestate, player);
    checkDiagonal(board, weightboard, size, winscore, gamestate, player);
    if((*player) -> automated){
        ai_set_best_move(weightboard, size, player);
    }
    checkTie(board, size, gamestate);
    free_board(&weightboard, size);

    return;
}

void checkTie(int ** board, int size, int * gamestate)
{
    if(*gamestate){
        return;
    }
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


void checkHorizontal(int** board, int ** weightboard, int size, int winscore,
                    int * gamestate, struct player ** player)
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

void checkVertical(int** board, int ** weightboard, int size, int winscore,
                    int * gamestate, struct player ** player)
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
void checkDiagonal(int** board, int ** weightboard, int size, int winscore,
                int * gamestate, struct player ** player)
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

            } else if(weightboard[row][column] > best_value)
            {
                num_of_bests = 0;
                best_rows[num_of_bests] = row;
                best_columns[num_of_bests] =  column;
                best_value = weightboard[row][column];
            }
        }
    }
    printf("\n");

    int random = rand() % (num_of_bests+1);

    (*ai_player) -> best_row = best_rows[random];
    (*ai_player) -> best_column = best_columns[random];

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

void print_ascii(char * filename)
{
    FILE *file = NULL;

    if((file = fopen(filename,"r")) == NULL)
    {
        printf("Couldn't open file %s\n",filename);
        return;
    }

    char read_string[10];

    while(fgets(read_string,sizeof(read_string),file) != NULL)
       printf("%s",read_string);

    fclose(file);
}

void wtfile_end(char * text, char * filename)
{
    FILE *file = NULL;
    if((file = fopen(filename,"a")) == NULL)
    {
        printf("Couldn't open file %s\n",filename);
        return;
    }
    fputs(text, file);
    fclose(file);
}

int get_csv_size(char * filename){
    FILE *file = NULL;
    if((file = fopen(filename,"r")) == NULL)
    {
        printf("Couldn't open file %s\n",filename);
        return 0 ;
    }
    // check size of document
    char read_line[250];
    int num_of_lines = 0;
    while(fgets(read_line, sizeof(read_line), file) != 0)
    {
        num_of_lines ++;
    }

    fclose(file);
    return num_of_lines;
}

int * get_csv_linesizes(char * filename, int num_of_lines){
    int * linesizes = malloc(sizeof(int) * num_of_lines);

    FILE *file = NULL;
    if((file = fopen(filename,"r")) == NULL)
    {
        printf("Couldn't open file %s\n",filename);
        return linesizes;
    }

    char read_line[250];
    int curr = 0;
    while(fgets(read_line, sizeof(read_line), file) != 0)
    {
        int num_of_vals = 1;
        for(int i=0; i<250; i++)
        {
            if(read_line[i] == '\n')
                break;
            else if(read_line[i] == ',')
                num_of_vals++;
        }
        linesizes[curr] = num_of_vals;
        curr++;
    }
    fclose(file);
    return linesizes;
}

char *** get_csv_array(char * filename, int csv_size, int * csv_linesizes)
{
    char *** csv_array = malloc(sizeof(char**) * csv_size);

    FILE *file = NULL;
    if((file = fopen(filename,"r")) == NULL)
    {
        printf("Couldn't open file %s\n",filename);
        return csv_array;
    }

    char read_line[250];
    int curr_line =0;
    while(fgets(read_line, sizeof(read_line), file) != 0)
    {
        int num_of_values = csv_linesizes[curr_line];
        int * word_lengths = malloc(sizeof(int) * num_of_values);

        int curr_word = 0;
        int num_of_chars = 0;
        for(int i=0; i<250; i++)
        {
            if(read_line[i] == '\n'){
                word_lengths[curr_word] = num_of_chars;
                break;
            }else if(read_line[i] == ','){
                word_lengths[curr_word] = num_of_chars;
                num_of_chars = 0;
                curr_word++;
            } else {
                num_of_chars++;
            }
        }

        char ** csv_line = malloc(sizeof(char*) * num_of_values);


        // int curr_char = 0;
        int read_chars = 0;
        // char * value = malloc(sizeof(char) * ((word_lengths[curr_word])+1));
        for(int curr_word = 0; curr_word<num_of_values; curr_word++){
            char * value = malloc(sizeof(char) * ((word_lengths[curr_word])+1));
            for(int i=0; i<word_lengths[curr_word]+1; i++)
            {
                if(read_line[read_chars] == '\n' || read_line[read_chars] == ','){
                    value[i] = '\0';
                    csv_line[curr_word] = value;
                    read_chars++;
                } else {
                    value[i] = read_line[read_chars];
                    read_chars++;
                }
            }
        }

        csv_array[curr_line] = csv_line;
        curr_line++;
        free(word_lengths);
    }

    fclose(file);

    return csv_array;
}
