#include <stdio.h>
#include <stdlib.h>

#define GB_WIDTH 3
#define GB_HEIGHT 3
#define MARK_X 1
#define MARK_O 2

typedef enum { false, true } bool;

struct turn{
    int player;
    int positionX;
    int positionY;
    struct turn * next;
};

void playTurn();
void makeMove();
void displayBoard();
void printDivider(int);
void printTop(int);
void printLine(int, int*, int);
void printRow(char, int, int*);

bool gameWon = false;
bool endSession = false;
int whosTurn = MARK_X;
int player1 = MARK_X;
int player2 = MARK_O;
// 0 => NO INPUT, 1 => X, 2 => O
int board[GB_WIDTH][GB_HEIGHT]= {{1,0,1}, {1,2,1}, {2,1,1}};

//other prompts: new players, end game, show leaderboard,





int main(){
    // both implemented as stacks
    struct turn * history;
    struct turn * undoneHistory;

    //setup game - ask for name, preferred



    displayBoard();
    return 0;
}

void playTurn(){

}

void makeMove(){

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
        printf("%d", i);
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
