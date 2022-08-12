# Lab3: page tables

### 1.Print a page table (easy)

代码添加到kernel/vm.c中

由于xv6中虚拟地址是由三级页表的形式存储的，因此打印页表本质上就是一个三重的for循环，比较简单，直接看**print_a_page_table(vm).c**文件中的注释即可。

### 2.A kernel page table per process (hard)

