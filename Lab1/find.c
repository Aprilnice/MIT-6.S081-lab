#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

// == Test find, in current directory == find, in current directory: OK (1.5s) 
// == Test find, recursive == find, recursive: OK (1.3s) 

void find(char *goal, char *path) {
    char buf[512], *p;
    int fd;//, i;
    struct dirent de;
    struct stat st;

    if((fd = open(path, 0)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0)
            continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if(strcmp(de.name, goal) == 0) {
            printf("%s/%s\n", path, de.name);
        }

        if(stat(buf, &st) < 0){
            continue;
        }
        if(st.type == T_DIR && strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0) {
            find(goal, buf);
        }
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    find(argv[2], argv[1]);
    exit(0);
}