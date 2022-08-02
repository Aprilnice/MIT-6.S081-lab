#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// == Test pingpong == pingpong: OK (1.6s) 

int main(int argc, char *argv[]) {
    int p1[2], p2[2], n;
    char s1, s2;
    // create a pipe
    pipe(p1);
    pipe(p2);
    int pid = fork();
    if (pid == 0) {
        n = read(p1[0], &s2, 1);
        if(n == 0) {
            fprintf(2, "child read error\n");
            exit(1);
        }else {
            printf("%d: received ping\n", pid);
        }

        n = write(p2[1], &s2, sizeof s2);
         if(n == 0) {
            fprintf(2, "child write error\n");
            exit(1);
        }
        exit(0);
    } else {
        n = write(p1[1], "t", 1);
         if(n == 0) {
            fprintf(2, "parent write error\n");
            exit(1);
        }
        n = read(p2[0], &s1, 1);
        if(n == 0) {
            fprintf(2, "parent read error\n");
            exit(1);
        }else {
            printf("%d: received pong\n", pid);
        }
    }
    close(p1[0]);
    close(p1[1]);
    close(p2[0]);
    close(p2[1]);
    exit(0);
}