# Lab3: page tables

### 1.Print a page table (easy)

代码添加到kernel/vm.c中

由于xv6中虚拟地址是由三级页表的形式存储的，因此打印页表本质上就是一个三重的for循环，比较简单，直接看**print_a_page_table(vm).c**文件中的注释即可。

### 2.A kernel page table per process (hard)

在proc.h的proc结构体定义中添加内核页表字段。

```c
// Per-process state
struct proc {
  struct spinlock lock;

  // ...
    
  // 内核页表副本
  pagetable_t kpagetable;
};
```

在vm.c中构建一个内核页表的副本函数

```c
// 返回一个内核页表副本
pagetable_t kvminit_new() {
  pagetable_t kpagetable = uvmcreate();
  if (kpagetable == 0) return 0;

  // uart registers
  kvmmap_new(kpagetable, UART0, UART0, PGSIZE, PTE_R | PTE_W);

  // virtio mmio disk interface
  kvmmap_new(kpagetable, VIRTIO0, VIRTIO0, PGSIZE, PTE_R | PTE_W);

  // CLINT
  kvmmap_new(kpagetable, CLINT, CLINT, 0x10000, PTE_R | PTE_W);

  // PLIC
  kvmmap_new(kpagetable, PLIC, PLIC, 0x400000, PTE_R | PTE_W);

  // map kernel text executable and read-only.
  kvmmap_new(kpagetable, KERNBASE, KERNBASE, (uint64)etext-KERNBASE, PTE_R | PTE_X);

  // map kernel data and the physical RAM we'll make use of.
  kvmmap_new(kpagetable, (uint64)etext, (uint64)etext, PHYSTOP-(uint64)etext, PTE_R | PTE_W);

  // map the trampoline for trap entry/exit to
  // the highest virtual address in the kernel.
  kvmmap_new(kpagetable, TRAMPOLINE, (uint64)trampoline, PGSIZE, PTE_R | PTE_X);

  return kpagetable;
}
```

在allocproc中调用这个函数，为新的进程创建内核页表副本，并将该进程的内核栈映射到内核页表副本中（代码参考proc.c中的procinit函数）

```c
static struct proc* allocproc(void) {
    // ...
    
    // An empty user page table.
    p->pagetable = proc_pagetable(p);
    if(p->pagetable == 0){
        freeproc(p);
        release(&p->lock);
        return 0;
    }
    
    // 设置内核页表
    p->kpagetable = kvminit_new();
    if(p->kpagetable == 0){
        freeproc(p);
        release(&p->lock);
        return 0;
    }

    // 将进程内核栈映射到内核页面
    char *pa = kalloc();
    if(pa == 0)
        panic("kalloc");
    uint64 va = KSTACK((int) (p - proc));
    kvmmap_new(p->kpagetable, va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
    p->kstack = va;
  
    // ...
}
```

修改`scheduler()`来加载进程的内核页表到核心的`satp`寄存器

```c
// 切换stap寄存器中的根页表为新进程的内核页表
w_satp(MAKE_SATP(p->kpagetable));
sfence_vma();

swtch(&c->context, &p->context);

// 没有进程运行时使用kernel_pagetable
kvminithart();
```

修改`freeproc()`释放进程的内核页表

```c
static void freeproc(struct proc *p) {
    // ...
    
    // 先释放内核栈映射
    uvmunmap(p->kpagetable, p->kstack, 1, 1);
    p->kstack = 0;
    // 释放内核页表
    if(p->kpagetable)
        freewalk_new(p->kpagetable);
  	p->kpagetable = 0;
}

// 释放页表
void freewalk_new(pagetable_t pagetable)
{
  // there are 2^9 = 512 PTEs in a page table.
  for(int i = 0; i < 512; i++){
    pte_t pte = pagetable[i];
    if(pte & PTE_V){
      pagetable[i] = 0;
      if ((pte & (PTE_R|PTE_W|PTE_X)) == 0){
        uint64 child = PTE2PA(pte);
        freewalk_new((pagetable_t)child);
      }
    }
  }
  kfree((void*)pagetable);
}
```

### 3.Simplify `copyin`/`copyinstr`（hard）