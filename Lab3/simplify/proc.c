void
userinit(void)
{
  // ...
  p->trapframe->epc = 0;      // user program counter
  p->trapframe->sp = PGSIZE;  // user stack pointer

  // 复制用户映射到内核页表
  uvmcopy_new(p->pagetable, p->kpagetable, 0, p->sz);

 // ...
}

int
growproc(int n)
{
  uint sz;
  struct proc *p = myproc();

  sz = p->sz;
  if(n > 0){

    // 添加PLIC大小限制
    if (PGROUNDUP(sz + n) >= PLIC){
      return -1;
    }

    if((sz = uvmalloc(p->pagetable, sz, sz + n)) == 0) {
        return -1;
    }

    // 复制用户映射到内核页表
    uvmcopy_new(p->pagetable, p->kpagetable, sz - n, sz);

  } else if(n < 0){
    sz = uvmdealloc(p->pagetable, sz, sz + n);
  }
  p->sz = sz;
  return 0;
}

int
fork(void)
{
  // ...

  // increment reference counts on open file descriptors.
  for(i = 0; i < NOFILE; i++)
    if(p->ofile[i])
      np->ofile[i] = filedup(p->ofile[i]);
  np->cwd = idup(p->cwd);

 // 复制到新进程的内核页表
  uvmcopy_new(np->pagetable, np->kpagetable, 0, np->sz);

  // ...
}
