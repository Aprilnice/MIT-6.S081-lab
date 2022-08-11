# Lab2: system calls

## 1.System call tracing（moderate）

用户态的trace函数已经给出了（user/trace.c），只需要实现内核态的trace功能即可。这里跳过添加函数原型和存根之类的步骤，直接看代码。

首先需要在**proch.h**中修改proc 结构体。这个结构体保存了进程的各种信息。添加一个uint64类型的trace字段，用于存放该进程需要追踪的系统调用号。

```c
struct proc {
  struct spinlock lock;

  // p->lock must be held when using these:
  enum procstate state;        // Process state
  struct proc *parent;         // Parent process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  int xstate;                  // Exit status to be returned to parent's wait
  int pid;                     // Process ID

  // these are private to the process, so p->lock need not be held.
  uint64 kstack;               // Virtual address of kernel stack
  uint64 sz;                   // Size of process memory (bytes)
  pagetable_t pagetable;       // User page table
  struct trapframe *trapframe; // data page for trampoline.S
  struct context context;      // swtch() here to run process
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)

  // 添加trace字段，用于指定追踪类型
  uint64 trace;
};
```

显然，trace是需要实现对于子进程系统调用的追踪的，因此需要修改**proc.c**中的fork函数，将父进程的trace字段赋值给子进程的trace字段。

```c
int fork(void) {
    // ...
    struct proc *np;
    struct proc *p = myproc();
    
    // ...
    np->trace = p->trace;
    
    // ...
}
```

同时，还需要获取用户态调用的参数，即确定需要追踪哪个系统调用。事实上，这个获取参数的函数可以放在其他文件当中，这里选择添加到**sysproc.c**。

```c
// 将追踪的掩码赋值给当前进程
uint64 sys_trace(void) {
  int n;

  if(argint(0, &n) < 0)
    return -1;

  myproc()->trace = n;
  return 0;
}
```

argint就是获取一个int参数，同时通过myproc函数获取当前进程的proc结构体，修改trace字段即可。

但到这里还是不够的，用户只能使用内核的系统调用，因此需要把trace设置为一种系统调用。

修改**syscall.h**

```c
// 添加trace的系统调用编号
#define SYS_trace  22
```

修改**syscall.c**

```c
// 增加函数引用(函数名就是添加到sysproc.c文件中的函数名)
extern uint64 sys_trace(void);

// 增加系统调用
static uint64 (*syscalls[])(void) = {
    // ... 
    // 增加trace
	[SYS_trace]   sys_trace,
}

// 索引数组,用于打印对应的系统调用
char index[][22] = {
"fork",
"exit",
"wait",
"pipe",
"read",
"kill",
"exec",
"fstat",
"chdir",
"dup",
"getpid",
"sbrk",
"sleep",
"uptime",
"open",
"write",
"mknod",
"unlink",
"link",
"mkdir",
"close",
"trace",
};

// 修改syscall函数
void syscall(void)
{
  int num;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    p->trapframe->a0 = syscalls[num]();
    
    // 修改部分
    // 打印需要追踪的系统调用
    if(p->trace != 0 && (1<<num) == p->trace){
      printf("%d: syscall %s -> %d\n", p->pid, index[num-1], p->trapframe->a0);
    }
    // 修改结束
    
  } else {
    printf("%d %s: unknown sys call %d\n",
            p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}
```

修改syscall函数本质上就是在每一次系统调用执行时，判断执行的系统调用是否与需要追踪的一致，一致则打印输出。

## 2.Sysinfo（moderate）

首先在kalloc.c文件中添加一个计算空闲容量的函数。

```c
// 计算空闲内存
uint64 kfreemem(void) {
  struct run *r;
  uint64 n = 0;

  acquire(&kmem.lock);
  r = kmem.freelist;
  while(r) {
    n += PGSIZE;
    r = r->next;
  }
  release(&kmem.lock);
  return n;
}
```

kmem是一个kalloc.c文件中定义的全局变量，携带了与内存相关的各种信息。

需要获得kmem的锁之后才可以进行下一步操作。

kmem.freelist指向一个空的页表，每个页表的容量为PGSIZE（即4096字节），只需要使用一个while循环就可以算出还有多少空闲容量。

要记得将锁释放，以避免一些意料之外的错误。

然后再proc.c中获取当前进程数量。

```c
// 计算进程数量
uint64 procnum(void) {
  struct proc *p;
  uint64 n = 0;

  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if(p->state != UNUSED) {
      n++;
    }
    release(&p->lock);
  }
  return n;
}
```

proc是一个定义在proc.c的全局变量，这个数组保存了所有进程的proc结构体，NPROC是定义好的最大进程数量常数。一次判断每个进程的状态，返回状态不为UNUSED的数量即可。

将上面的两个函数原型添加到defs.h中以便其他文件可以调用。

```c
// 添加头文件
// kalloc.c
uint64          kfreemem(void);
// proc.c
uint64          procnum(void);
```

在sysproc.c中添加sysinfo的函数主体。

```c
// 将系统信息返回到用户空间
uint64 sys_info(void) {
  struct sysinfo s;
  uint64 st;
  struct proc *p = myproc();
  if(argaddr(0, &st) < 0)
    return -1;

  // 获取剩余内存字节数
  s.freemem = kfreemem();
  // 获取当前进程数
  s.nproc = procnum();

  if(copyout(p->pagetable, st, (char *)&s, sizeof(s)) < 0)
    return -1;
  return 0;
}
```

因为要求返回的是一个sysinfo结构体。因此需要通过argaddr获取传入结构体的地址，使用copyout将数据写入后返回。

注：这里省略了一些声明的添加，usr.h和syscall.h以及syscall.c按照前面几个实验一样的讨论修改即可。