# Lab1: Xv6 and Unix utilities

## 1. sleep（Easy）

第一题比较简单，实际上就是获取命令行参数，然后调用提供的sleep函数即可。因此重点在于对输入参数的合法性判断。

```c
if(argc <= 1) {
    fprintf(2, "arg error\n");
    exit(1);
}
```

当参数数量不超过一个时，显然应该打印错误并退出。

```c
if(sleep(atoi(argv[1])) == -1) {
    fprintf(2, "sleep error\n");
    exit(1);
}
```

这是具体的执行过程，xv6系统调用除特殊声明外，均用返回0表示无误，返回-1表示出错，因此这里再判断一下sleep函数的执行情况即可。

## 2. pingpong（Easy）

这一题的主要目的是了解pipe（管道）和fork（进程）的使用。

```c
pipe(p1);
pipe(p2);
```

由于是双向通信，因此需要先创建两个管道，p1用于父进程向子进程传递，p2用于子进程向父进程传递。

```c
int pid = fork();
```

创建一个子进程，用pid接收子进程的PID。

```c
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
}
```

对于父进程，向p1中写入，从p2中读取即可。

```c
else {
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
```

退出程序前，关闭用到的文件描述符

```c
close(p1[0]);
close(p1[1]);
close(p2[0]);
close(p2[1]);
exit(0);
```

补充说明：

这里父进程不需要使用wait来确定子进程已经向p2中写入字节了（当然也可以用）。

这是因为za在xv6中**管道**的read函数与普通的read函数不一样。

对于普通的read函数，会在读到文件结束时返回0。但是管道的read函数会在没有可用数据时进入等待状态，只有在所有指向这个管道写入端的文件描述符都被关闭之后（即不可能有写入之后）才会返回0。（这部分可见xv6手册第一章第三节）

## 3. primes（Moderate/Hard）

这一题的难度稍微大了一点，需要先分析一下题目。

很显然，这里需要创建多个子进程和多个管道。每个进程通过管道从直接父进程中获取数据，并打印一个素数，同时将其余数据通过管道传递给当前进程的直接子进程。为了便于打印，需要保证写入通道的第一个数为素数（通过Eratosthenes的筛选法实现）。

首先，为了让代码易于理解，先写一个专门处理当前进程数字的函数。

```c
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
```

显然，r是外部传递进来的用于读入数据的文件描述符，w是外部传递进来的用于写入数据的文件描述符。显然，由于每个进程都从直接父进程获取数据，那么每个管道的只会在父进程中write、在子进程中read。因此，在最后要关闭r和w以节省有限的文件描述符。

根据题目，很容易联想到使用递归来解决。由于最开始只有写这一操作，而没有读。因此递归从第一个子进程开始。

```c
int main(int argc, char *argv[]) {
    int right[2], tmp, n, p;
    uint32 i,t;
    pipe(right);
    p = fork();
    while(p == 0) {
        // ...
    }

    for(i = 2; i < 36; i++) {
        write(right[1], &i, 4);
    }
    close(right[1]);
    wait((int *) 0);
    exit(0);
}
```

主进程向管道内写入，然后等待所有子孙进程结束。

关键在于循环部分。

```c
while(p == 0) {
    tmp = right[0];
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
```

首先，记录父进程管道的读文件描述符。然后读取一个四字节的数字并打印（Eratosthenes的筛选法确保第一个数为素数）。然后创建一个新的管道，文件描述符也记录在right中，作为与子进程传输的管道。接着再fork一个子进程。每个子进程都在其子孙进程全部退出后才退出。

在output中会关闭来自父进程管道的**读**文件描述符，以及与子进程之间管道的**写**文件描述符。

## 4. find（Moderate）

这题实际上就是自己实现一个常见的shell指令find。

显然，find需要可以遍历所有的文件路径，不管是“广搜”还是“深搜”，都需要抽象出一个find函数。

因此，main函数只作为传递参数的入口。

```c
int main(int argc, char *argv[]) {
    find(argv[2], argv[1]);
    exit(0);
}
```

这里选择使用递归调用的方式实现查找功能。每个find函数可以检查当前路径文件夹下是否有目标文件。

代码主体参考user/ls.c的ls函数。

首先是获取输入路径的状态信息

```c
char buf[512], *p;
int fd;
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
```

然后遍历整个文件夹，寻找目标文件，如果文件是文件夹就调用find函数递归。

```c
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
```

需要注意的是，路径中会有.文件以及..文件，这两个文件夹会导致无限递归，需要单独判断。

## 5.  xargs（Moderate）

首先需要知道的是，实际要做的就是将|前命令的输出作为参数输入到xargs中并执行相应的命令。

先获取已知的参数

```c
char buf[512], r;
char *params[4];
int count = 0;
params[0] = argv[1];
params[1] = argv[2];
params[3] = 0;
```

对于例子find . b | xargs grep hello，params[0]就是grep，params[1]就是hello，params[3]为0表示参数结束。这些都是exec函数需要的参数。

接着是获取前一段命令的输出。

```c
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
```

因为前一个命令的输出可以有多个，且是标准输出（即读文件描述符为0）。因此按字节读取，读到换行符即为一个完整的输出了，此时fork一个子进程通过exe执行命令。循环往复，直到标准输出已经被读完，等到所有子孙进程结束即可。