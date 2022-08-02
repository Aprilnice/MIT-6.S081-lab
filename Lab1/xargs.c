#include "kernel/types.h"
#include "user/user.h"

// == Test xargs == xargs: OK (1.9s)

int main(int argc, char *argv[]) {
    char buf[512], r;
    char *params[4];
    int count = 0;
    params[0] = argv[1];
    params[1] = argv[2];
    params[3] = 0;

    while(read(0, &r, 1) != 0) {
        if(r == '\n'){
            buf[count] = '\0';
            params[2] = buf;
            if(fork() == 0) {
                exec(params[0], params);
                exit(0);
            }
            count = 0;
            continue;
        }
        buf[count] = r;
        count++;
    }
    wait((int *) 0);
    exit(0);
}