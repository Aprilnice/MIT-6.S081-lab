#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

//== Test sleep, no arguments == sleep, no arguments: OK (1.4s) 
//== Test sleep, returns == sleep, returns: OK (0.8s) 
//== Test sleep, makes syscall == sleep, makes syscall: OK (1.1s) 

int main(int argc, char *argv[]) {
    // judge arg
    if(argc <= 1) {
        fprintf(2, "arg error\n");
        exit(1);
    }
    else {
        if(sleep(atoi(argv[1])) == -1) {
            fprintf(2, "sleep error\n");
            exit(1);
        }
    }
    exit(0);
}