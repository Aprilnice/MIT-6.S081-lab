// == Test pte printout == pte printout: OK (2.5s) 

void vmprint(pagetable_t pagetable) {
  printf("page table %p\n", pagetable);

  // 每个page最多有512个pte
  for(int i = 0; i < 512; i++){
    pte_t pte1 = pagetable[i];
    
    // 判断该pte有效且是否指向下一级page
    if((pte1 & PTE_V) && (pte1 & (PTE_R|PTE_W|PTE_X)) == 0){
      printf("..%d: pte %p pa %p\n", i, pte1, PTE2PA(pte1));

      // 获取下一级page的物理地址
      pagetable_t child1 = (pagetable_t)PTE2PA(pte1);

      // 再次遍历
      for(int j = 0; j < 512; j++) {
        pte_t pte2 = child1[j];

        // 同样的判断
        if((pte2 & PTE_V) && (pte2 & (PTE_R|PTE_W|PTE_X)) == 0) {
          printf(".. ..%d: pte %p pa %p\n", j, pte2, PTE2PA(pte2));

          // 获取最后一级的物理地址
          pagetable_t child2 = (pagetable_t)PTE2PA(pte2);

          for(int k = 0; k < 512; k++) {
            pte_t pte3 = child2[k];

            if((pte3 & (PTE_R|PTE_W|PTE_X)) == 0) {
              printf(".. .. ..%d: pte %p pa %p\n", k, pte3, PTE2PA(pte3));
            }
          }

        }
      }

    }
  }
}