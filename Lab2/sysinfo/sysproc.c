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