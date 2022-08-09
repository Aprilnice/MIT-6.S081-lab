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