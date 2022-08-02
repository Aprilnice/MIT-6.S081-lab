#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// == Test primes == primes: OK (1.7s)

void output(uint32 i, int r, int w) {
    int n;
    uint32 p;
    while(1) {
        n = read(r, &p, 4);
        if(n == 0) {
            break;
        }
        if(p % i != 0) {
            n = write(w, &p, 4);
        }
    }
    close(r);
    close(w);
}

int main(int argc, char *argv[]) {
    int right[2], tmp, n, p;
    uint32 i,t;
    pipe(right);
    p = fork();
    while(p == 0) {
        tmp = right[0];
        close(right[1]);
        n = read(tmp, &t, 4);
        if(n == 0) {
            close(tmp);
            exit(0);
        }
        printf("prime %d\n", t);
        pipe(right);
        output(t, tmp, right[1]);
        p = fork();
        if(p != 0) {
            wait((int *) 0);
            exit(0);
        }
    }

    for(i = 2; i < 36; i++) {
        write(right[1], &i, 4);
    }
    close(right[1]);
    wait((int *) 0);
    exit(0);
}