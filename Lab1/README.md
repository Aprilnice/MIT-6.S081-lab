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



## 3. primes（Moderate/Hard）

## 4. find（Moderate）

## 5.  xargs（Moderate）