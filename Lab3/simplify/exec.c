int
exec(char *path, char **argv)
{
  // ...
  stackbase = sp - PGSIZE;

  // 将用户映射复制到内核页表
  uvmcopy_new(pagetable, p->kpagetable, 0, sz);

  // Push argument strings, prepare rest of stack in ustack.
}