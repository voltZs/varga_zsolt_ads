#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define REPS 10
#define DIMENSION 1000

double averageResult(int);
void code(int);
void checkDiagonal(int**, int, int);
void initBoard(int ***, int);
void traverse_board(int **, int);
void free_board(int ***, int);


int main()
{
    int size_max = 100;
    double * csv_array = malloc(sizeof(double) * size_max);

    for(int i=0; i<size_max; i++){
        csv_array[i] = averageResult(i);
    }
    //
    // printf("Run 1000times, boardsize 6, average time: %f", averageResult(size));
    //
    FILE *file = NULL;
    if((file = fopen("diagonal_check.csv","a")) == NULL)
    {
        printf("Couldn't open file \n");
        return 0;
    }
    for(int i=0; i<size_max; i++){
        if(i == size_max-1)
            fprintf(file, "%f\n", csv_array[i]);
        else
            fprintf(file, "%f,", csv_array[i]);
    }

    fclose(file);


    return 0;
}

double averageResult(int size){
    clock_t t;
    int sum;

    for(int index=0; index<REPS; index++)
    {
        printf("-------Repetition %d-------\n", index+1);
        printf("\tstart: %d \n", (int) (t=clock()));

        code(size);

        printf("\tstop: %d \n", (int) (t=clock()-t));
        sum +=t;
    }

    return (double) sum/REPS/CLOCKS_PER_SEC ;
}

void code(int size)
{
    int ** board;
    initBoard(&board, size);
    int winscore = (int) ((size/5)*3);
    printf("WS %d\n", winscore);

    for(int i=0; i<DIMENSION; i++)
    {
        //insert code to be tested here
        checkDiagonal(board, size, winscore);
    }
    free_board(&board, size);

}

// void checkDiagonal(int** board, int ** weightboard, int size, int winscore,
//                 int * gamestate, struct player ** player)
void checkDiagonal(int** board, int size, int winscore)
{
    // if(*gamestate){
    //     return;
    // }

    int consecutive = 1;
    int current_sign;
    int previous_sign;

    // bool automated = (*player) -> automated;
    // int * ai_scope = calloc(winscore, sizeof(int));
    // int * ai_scope_row = calloc(winscore, sizeof(int));
    // int * ai_scope_column = calloc(winscore, sizeof(int));

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
        iterations = 0;
        while(iterations >= 0){
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
                if(current_sign == 1 || current_sign == 2){
                    if(current_sign == previous_sign){
                        consecutive++;
                        if(consecutive == winscore){
                            // *gamestate = GAME_WON;
                            return;
                        }
                    } else {
                        consecutive = 1;
                    }
                } else {
                    consecutive = 1;
                }

                // if(automated){
                //     push_scope(ai_scope, winscore, current_sign);
                //     push_scope(ai_scope_row, winscore, row);
                //     push_scope(ai_scope_column, winscore, column);
                //     // only do pattern checs if we are winsize elements into the array
                //     if((index+1) >= winscore){
                //         int weight = ai_check_patterns(ai_scope, winscore, (*player)->mark);
                //         // printf("weight: %d\n", weight);
                //         ai_add_weights(board, weightboard, weight, ai_scope_row, ai_scope_column, winscore);
                //     }
                // }

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

    // free(ai_scope);
    // free(ai_scope_row);
    // free(ai_scope_column);
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
