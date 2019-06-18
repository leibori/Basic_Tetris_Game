#include<string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define SYS_CALL_ERROR  "Error in system call\n"
#define ERROR           -1
#define SECOND          1

/* Blocks defines */
#define STAR          '*'
#define SPACE           ' '
#define NORMAL_BLOCK         '-'
#define SPIN_BLOCK         '|'

/* Movement defines */
#define LEFT            'a'
#define RIGHT           'd'
#define UP              'w'
#define DOWN            's'
#define BYE            'q'

/* Game defines */
#define CLEAR           "clear"
#define GAME_HEIGHT     21
#define GAME_WIDTH      22

/* Enums and structs */
char Shape[2] = {NORMAL_BLOCK, SPIN_BLOCK};
typedef struct Block {
    int x_size;
    int y_size;
    int isVertic;
} Block;

typedef struct Board {
    char board_game[GAME_HEIGHT][GAME_WIDTH];
    int hasBlock;
    Block *activeBlock;
} Board;

/* Function definitions */

void changeShape(Board *board);

void alarm_handler(int);


void addBlock(Board *board);

void moveLeft(Board *board);

void moveRight(Board *board);


void MoveIt(Board *board, char c);

void signal_handler(int);


void boardWrite(Board *board);

void moveOneStep(Board *board);

void nextStep(Board *board, int dx);


void clear(Board *board);


void error();

/* Globals */
Board *board;

int main() {
    Board *pBoard = calloc(sizeof(Board), 1);
    if (!pBoard) {
        error();
    }
    board = pBoard;
    //Init all
    int i, j, m;
    for (i = 0; i < GAME_HEIGHT; i++) {
        for (j = 0; j < GAME_WIDTH; j++) {
            if (j % (GAME_WIDTH - 1) == 0)board->board_game[i][j] = STAR;
            else board->board_game[i][j] = SPACE;
        }
    }
    for (m = 0; m < GAME_WIDTH; m++)
        board->board_game[GAME_HEIGHT - 1][m] = STAR;
    board->hasBlock = 0;
    addBlock(board);
    boardWrite(board);
    if (signal(SIGALRM, alarm_handler) == SIG_ERR)
        error();
    alarm(SECOND);
    if (signal(SIGUSR2, signal_handler) == SIG_ERR)
        error();
    while (1) pause();
    return 0;
}

/**
 * Called when a system call error happens.
 */
void error() {
    write(2, SYS_CALL_ERROR, strlen(SYS_CALL_ERROR));
    if (board->activeBlock != NULL) {
        free(board->activeBlock);
    }
    free(board);
    exit(ERROR);
}

void MoveIt(Board *board, char c) {
    switch (c) {
        case LEFT:
            moveLeft(board);
            break;
        case RIGHT:
            moveRight(board);
            break;
        case DOWN:
            break;
        case UP:
            changeShape(board);
            break;
        case BYE:
            if (board->activeBlock != NULL) { free(board->activeBlock); }
            free(board);
            exit(1);
        default:
            break;
    }
}



/**
 * Handles the alarm.
 * @param alarm
 */
void alarm_handler(int al) {
    signal(SIGALRM, alarm_handler);
    alarm(SECOND);
    if (board->hasBlock == 0) addBlock(board);
    moveOneStep(board);
    boardWrite(board);
}

/**
 * Handles the signal from SIGUSR2.
 * @param signal
 */
void signal_handler(int sig) {
    signal(SIGUSR2, signal_handler);
    if (board->hasBlock == 0) {
        addBlock(board);
    }
    char c;
    ssize_t num_bytes = read(STDIN_FILENO, &c, 1);
    if (num_bytes == ERROR) {
        error();
    }
    MoveIt(board, c);
    moveOneStep(board);
    boardWrite(board);
}

/**
 * Changes the shape of the board's block.
 * @param board Board with required block.
 */
void changeShape(Board *board) {
    Block *block = board->activeBlock;
    int hpad = block->isVertic == 0 ? 0 : 1;
    int vpad = block->isVertic == 0 ? 1 : 0;

    if (board->board_game[block->y_size + vpad][block->x_size + hpad] == SPACE
        && board->board_game[block->y_size - vpad][block->x_size - hpad] == SPACE) {
        clear(board);
        block->isVertic = 1 - block->isVertic;
    } else return;

}


/**
 * Adds a block to given board.
 * @param board Board to add a block to.
 */
void addBlock(Board *board) {
    Block *block = calloc(sizeof(Block), 1);
    block->x_size = GAME_WIDTH / 2;
    block->y_size = 0;
    block->isVertic = 0;
    board->board_game[block->y_size][block->x_size] = Shape[block->isVertic];
    board->board_game[block->y_size][block->x_size + 1] = Shape[block->isVertic];
    board->board_game[block->y_size][block->x_size - 1] = Shape[block->isVertic];
    board->activeBlock = block;
    board->hasBlock = 1;
}

/**
 * Prints the board.
 * @param board Board to print.
 */
void boardWrite(Board *board) {
    system(CLEAR);
    int i, j;
    for (i = 0; i < GAME_HEIGHT; i++) {
        for (j = 0; j < GAME_WIDTH; j++)
            printf("%c", board->board_game[i][j]);
        puts("");
    }
}

/**
 * Retires the active block.
 * @param board Playing board.
 */
void kill_block(Board *board) {
    Block *block = board->activeBlock;
    int dx = block->isVertic == 0 ? 1 : 0;
    int dy = block->isVertic == 0 ? 0 : 1;
    board->board_game[block->y_size][block->x_size] = SPACE;
    board->board_game[block->y_size - dy][block->x_size - dx] = SPACE;
    board->board_game[block->y_size + dy][block->x_size + dx] = SPACE;
    free(block);
}

/**
 * Moves the block one step down if possible, or sets HasBlock to false if not possible.
 * @param board Board to move its block.
 */
void moveOneStep(Board *board) {
    Block *block = board->activeBlock;
    int dx = 0, dy = 0;
    dx = block->isVertic == 0 ? 1 : 0;
    dy = block->isVertic == 0 ? 1 : 2;
    if (board->board_game[block->y_size + dy][block->x_size - dx] == SPACE
        && board->board_game[block->y_size + dy][block->x_size] == SPACE
        && board->board_game[block->y_size + dy][block->x_size + dx] == SPACE) {
        clear(board);
        ++block->y_size;
        nextStep(board, dx);
    } else {
        kill_block(board);
        board->hasBlock = 0;
        return;
    }
}

/**
 * Moves the block right.
 * @param board Board to move its block.
 */
void moveRight(Board *board) {
    Block *block = board->activeBlock;
    int hpad = block->isVertic == 0 ? 2 : 1;
    int vpad = block->isVertic == 0 ? 0 : 2;
    if ((board->board_game[block->y_size][block->x_size + hpad] == SPACE)
        && (board->board_game[block->y_size + vpad][block->x_size + hpad] == SPACE)
        && (board->board_game[block->y_size - vpad][block->x_size + hpad] == SPACE)) {
        clear(board);
        board->board_game[block->y_size][++block->x_size] = Shape[block->isVertic];
        board->board_game[block->y_size][block->x_size] = Shape[block->isVertic];
        board->board_game[block->y_size][block->x_size] = Shape[block->isVertic];
    }
}

/**
 * Moves the block left.
 * @param board Board to move its block.
 */
void moveLeft(Board *board) {
    Block *block = board->activeBlock;
    int hpad = block->isVertic == 0 ? 2 : 1;
    int vpad = block->isVertic == 0 ? 0 : 2;
    if ((board->board_game[block->y_size][block->x_size - hpad] == SPACE)
        && (board->board_game[block->y_size + vpad][block->x_size - hpad] == SPACE)
        && (board->board_game[block->y_size - vpad][block->x_size - hpad] == SPACE)) {
        clear(board);
        board->board_game[block->y_size][--block->x_size] = Shape[block->isVertic];
        board->board_game[block->y_size][block->x_size] = Shape[block->isVertic];
        board->board_game[block->y_size][block->x_size] = Shape[block->isVertic];
    }
}

/**
 * Cleanups the previous step of the block. Removes leftovers.
 * @param board Board to clean up its block from previous step.
 */
void clear(Board *board) {
    Block *block = board->activeBlock;
    int dx = 0, dy = 0;
    dx = block->isVertic == 0 ? 1 : 0;
    dy = block->isVertic == 0 ? 0 : 1;

    board->board_game[block->y_size - dy][block->x_size - dx] = SPACE;
    board->board_game[block->y_size][block->x_size] = SPACE;
    board->board_game[block->y_size + dy][block->x_size + dx] = SPACE;

}

/**
 * Makes the blocks next step.
 * @param board Board to move the block.
 * @param dx Indicates if block is HORIZONTAL or VERTICAL.
 */
void nextStep(Board *board, int dx) {
    Block *block = board->activeBlock;
    board->board_game[block->y_size - 1 + dx][block->x_size - dx] = Shape[block->isVertic];
    board->board_game[block->y_size][block->x_size] = Shape[block->isVertic];
    board->board_game[block->y_size + 1 - dx][block->x_size + dx] = Shape[block->isVertic];

}
