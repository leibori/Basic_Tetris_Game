#include<string.h>
#include <stdio.h>
#include <unistd.h>
#include <termio.h>
#include <stdlib.h>
#include <signal.h>

#define UP              'w'
#define DOWN            's'
#define LEFT            'a'
#define RIGHT           'd'
#define QUIT            'q'
#define DOT_DRAW        "./draw.out"
#define ERROR           -1
#define SYS_CALL_ERROR  "Error in system call\n"

char getch();
void error();
int isKeys(char c);

int main(int argc, char *argv[]) {
    char *args[] = {DOT_DRAW, NULL};
    char c = 0;
    pid_t childPid;
    int pipedes[2];
    pipe(pipedes);
    if ((childPid = fork())==-1) error();

    if (childPid==0) { //son
        close(0);
        dup(pipedes[0]);
        execvp(args[0], args);
        error();
    } else { // father
        close(pipedes[0]);
        while (c!=QUIT) {
            c = getch();
            if (isKeys(c) == 0) continue;
            write(pipedes[1], &c, 1);
            kill(childPid, SIGUSR2);
            if (c==QUIT) break;
        }
        return 0;
    }
    return 0;
}
int isKeys(char c) {
    switch(c) {
        case RIGHT:
        case LEFT:
        case DOWN:
        case UP:
        case QUIT:
            return 1;
        default:
            return 0;
    }
}

/**
 * Called when a system call error happens.
 */
void error() {
    write(2, SYS_CALL_ERROR, strlen(SYS_CALL_ERROR));
    exit(ERROR);
}

/**
 * Gets char from input without pressing ENTER.
 * @return Input char.
 */
char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return (buf);
}