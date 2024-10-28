#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>

void checkStatus(int status) {
    status = WEXITSTATUS(status);
    printf("With anonymous pipes, the cmp output is %d.", status);
    if (status == 0) {
        printf(" The compared texts are the same.\n");
    } else if (status == 1) {
        printf(" The compared texts are different.\n");
    } else {
        printf(" There was an error during the program's execution.\n");
    }
}

int main() {
    if (fork() == 0) { // P3
        int fd1[2];
        pipe(fd1);

        if (fork() == 0) { // P2
            close(fd1[0]);
            int fd2[2];
            pipe(fd2);

            if (fork() == 0) { // P1
                close(fd1[1]);
                close(fd2[0]);
                int textFile = open("In.txt", O_RDONLY);
                dup2(textFile, 0);
                close(textFile);
                dup2(fd2[1],1);
                close(fd2[1]);
                execlp("rev", "rev", NULL);
            }

            close(fd2[1]);
            wait(NULL);
            dup2(fd2[0], 0);
            close(fd2[0]);
            dup2(fd1[1], 1);
            close(fd1[1]);
            execlp("rev", "rev", NULL);
        }

        close(fd1[1]);
        while(wait(NULL) > 0);
        
        dup2(fd1[0], 0);
        close(fd1[0]);
        execlp("cmp", "cmp", "-", "In.txt", NULL);
    }

    int status;
    while(wait(&status) > 0);
    checkStatus(status);
    return 0;
}
