#include "spinlock.h" 
#include "proc.h"

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

// 在自定义页表中添加映射
void kvmmap_new(pagetable_t p, uint64 va, uint64 pa, uint64 sz, int perm) {
  if(mappages(p, va, sz, pa, perm) != 0)
    panic("kvmmapn");
}

uint64
kvmpa(uint64 va)
{
  uint64 off = va % PGSIZE;
  pte_t *pte;
  uint64 pa;
  
  //修改开始
  pte = walk(myproc()->kpagetable, va, 0);
  //修改结束

  if(pte == 0)
    panic("kvmpa");
  if((*pte & PTE_V) == 0)
    panic("kvmpa");
  pa = PTE2PA(*pte);
  return pa+off;
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