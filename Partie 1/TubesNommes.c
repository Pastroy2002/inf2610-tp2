#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/wait.h>

void checkStatus(int status) {
    status = WEXITSTATUS(status);
    printf("With named pipes, the cmp output is %d.", status);
    if (status == 0) {
        printf(" The compared texts are the same.\n");
    } else if (status == 1) {
        printf(" The compared texts are different.\n");
    } else {
        printf(" There was an error during the program's execution.\n");
    }
}

void thirdChildActions() {
    int fd = open("pipe1", O_WRONLY);
    dup2(fd, 1);
    close(fd);
    int textFile = open("In.txt", O_RDONLY);
    dup2(textFile, 0);
    close(textFile);
    execlp("rev", "rev", NULL);
}

void secondChildActions() {
    int fd = open("pipe1", O_RDONLY);
    wait(NULL);
    dup2(fd, 0);
    close(fd);
    fd = open("pipe2", O_WRONLY);
    dup2(fd, 1);
    execlp("rev", "rev", NULL);
}

void firstChildActions() {
    int fd = open("pipe2", O_RDONLY);
    while(wait(NULL) > 0);
    dup2(fd, 0);
    close(fd);
    execlp("cmp", "cmp", "-", "In.txt", NULL);
}

int main() {
    mkfifo("pipe1", 0600);
    mkfifo("pipe2", 0600);

    if (fork() == 0) { // P3
        if (fork() == 0) { // P2
            if (fork() == 0) { // P1
                thirdChildActions();
            }
            secondChildActions();
        }
        firstChildActions();
    }

    int status;
    while(wait(&status) > 0);
    checkStatus(status);
    unlink("pipe1");
    unlink("pipe2");

    return 0;
}
