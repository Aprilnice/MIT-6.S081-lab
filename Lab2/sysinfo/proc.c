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