// 将追踪的掩码赋值给当前进程
uint64 sys_trace(void) {
  int n;

  if(argint(0, &n) < 0)
    return -1;

  myproc()->trace = n;
  return 0;
}