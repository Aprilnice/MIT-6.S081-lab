// == Test pte printout == pte printout: OK (2.5s) 

void vmprint(pagetable_t pagetable) {
  printf("page table %p\n", pagetable);
  for(int i = 0; i < 512; i++){
    pte_t pte1 = pagetable[i];
    if((pte1 & PTE_V) && (pte1 & (PTE_R|PTE_W|PTE_X)) == 0){
      printf("..%d: pte %p pa %p\n", i, pte1, PTE2PA(pte1));
      pagetable_t child1 = (pagetable_t)PTE2PA(pte1);

      for(int j = 0; j < 512; j++) {
        pte_t pte2 = child1[j];
        if((pte2 & PTE_V) && (pte2 & (PTE_R|PTE_W|PTE_X)) == 0) {
          printf(".. ..%d: pte %p pa %p\n", j, pte2, PTE2PA(pte2));
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