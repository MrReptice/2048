#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define SIZE 4
#define MAX_UNDO 500

typedef struct {
    int board[SIZE][SIZE];
    int score;
} game_state_t;

game_state_t current_state;
game_state_t history[MAX_UNDO];
int history_count = 0;
int highscore = 0;

//functions
void print_board();
void add_random();
bool slide_array(int* array, int* score_increment);
bool move_board(char direction);
void save_state();
void undo_move();
void load_highscore();
void save_highscore();
void reset_game();
void clear_screen();

int main() {
    char choice;
    srand((unsigned int)time(NULL));
    
    load_highscore();
    reset_game();

    while (1) {
        print_board();
        printf("\nControls: W/A/S/D (Move) | P (Undo) | R (Restart) | U (Exit)\n");
        printf("Choice: ");
        
        choice = getchar();
        while (getchar() != '\n'); //clear buffer

        bool moved = false;
        switch (choice) {
            case 'w': case 'W':
            case 'a': case 'A':
            case 's': case 'S':
            case 'd': case 'D':
                save_state();
                moved = move_board(choice);
                if (moved) add_random();
                else history_count--; //revert history if move != valid
                break;
            case 'p': case 'P':
                undo_move();
                break;
            case 'r': case 'R':
                reset_game();
                break;
            case 'u': case 'U':
                save_highscore();
                exit(0);
            default:
                printf("\nInvalid key!\n");
                break;
        }

        if (current_state.score > highscore) {
            highscore = current_state.score;
        }
    }
    return 0;
}

//screen wipe
void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void reset_game() {
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            current_state.board[i][j] = 0;
            
    current_state.score = 0;
    history_count = 0;
    add_random();
    add_random(); //standard 2048 start
}

//spawn 2 (90% chance) or 4 (10% chance)
void add_random() {
    int empty_spaces[SIZE * SIZE][2];
    int empty_count = 0;

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (current_state.board[i][j] == 0) {
                empty_spaces[empty_count][0] = i;
                empty_spaces[empty_count][1] = j;
                empty_count++;
            }
        }
    }

    if (empty_count > 0) {
        int idx = rand() % empty_count;
        int value = (rand() % 10 < 9) ? 2 : 4;
        current_state.board[empty_spaces[idx][0]][empty_spaces[idx][1]] = value;
    }
}
//layout
void print_board() {
    clear_screen();
    printf("\n\t\t=============== 2048 ===============\n");
    printf("\t\tSCORE: %-10d HIGH SCORE: %d\n", current_state.score, highscore);
    printf("\t\t------------------------------------\n");

    for (int i = 0; i < SIZE; i++) {
        printf("\t\t|");
        for (int j = 0; j < SIZE; j++) {
            if (current_state.board[i][j] != 0) {
                printf(" %4d |", current_state.board[i][j]);
            } else {
                printf("      |");
            }
        }
        printf("\n\t\t------------------------------------\n");
    }
}

// true if array was modified
bool slide_array(int* array, int* score_increment) {
    bool moved = false;
    int write_idx = 0;
    int last_merged = 0;

    for (int i = 0; i < SIZE; i++) {
        if (array[i] != 0) {
            if (last_merged != 0 && last_merged == array[i]) {
                // merge
                array[write_idx - 1] *= 2;
                *score_increment += array[write_idx - 1];
                last_merged = 0;
                moved = true;
            } else {
                // slide
                if (write_idx != i) moved = true;
                array[write_idx] = array[i];
                last_merged = array[i];
                write_idx++;
            }
        }
    }
    
    // fill remainder with zeros
    while (write_idx < SIZE) {
        if (array[write_idx] != 0) moved = true;
        array[write_idx++] = 0;
    }
    
    return moved;
}

// maps the 2D board to 1D arrays depending on move direction
bool move_board(char direction) {
    bool board_changed = false;
    int temp_arr[SIZE];
    int score_inc = 0;

    for (int i = 0; i < SIZE; i++) {
        // extract 1D array based on direction
        for (int j = 0; j < SIZE; j++) {
            if (direction == 'a' || direction == 'A') temp_arr[j] = current_state.board[i][j];
            else if (direction == 'd' || direction == 'D') temp_arr[j] = current_state.board[i][SIZE - 1 - j];
            else if (direction == 'w' || direction == 'W') temp_arr[j] = current_state.board[j][i];
            else if (direction == 's' || direction == 'S') temp_arr[j] = current_state.board[SIZE - 1 - j][i];
        }

        bool row_changed = slide_array(temp_arr, &score_inc);
        if (row_changed) board_changed = true;

        // Map back to 2D board
        for (int j = 0; j < SIZE; j++) {
            if (direction == 'a' || direction == 'A') current_state.board[i][j] = temp_arr[j];
            else if (direction == 'd' || direction == 'D') current_state.board[i][SIZE - 1 - j] = temp_arr[j];
            else if (direction == 'w' || direction == 'W') current_state.board[j][i] = temp_arr[j];
            else if (direction == 's' || direction == 'S') current_state.board[SIZE - 1 - j][i] = temp_arr[j];
        }
    }
    
    current_state.score += score_inc;
    return board_changed;
}


void save_state() {
    if (history_count < MAX_UNDO) {
        history[history_count++] = current_state;
    } else {
        // shift history left if we reach the max
        for (int i = 1; i < MAX_UNDO; i++) {
            history[i - 1] = history[i];
        }
        history[MAX_UNDO - 1] = current_state;
    }
}

void undo_move() {
    if (history_count > 0) {
        current_state = history[--history_count];
    } else {
        printf("\nNo more moves to undo!\n");
    }
}

void load_highscore() {
    FILE* ptr = fopen("highscore.txt", "r");
    if (ptr) {
        if (fscanf(ptr, "%d", &highscore) != 1) highscore = 0;
        fclose(ptr);
    }
}

void save_highscore() {
    FILE* ptr = fopen("highscore.txt", "w");
    if (ptr) {
        fprintf(ptr, "%d", highscore);
        fclose(ptr);
    }
}
