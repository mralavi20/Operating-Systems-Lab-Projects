#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
// #include "vm.c"

struct shared_pgtable{
  int pgid;
  void * pageaddr;
  uint refs;
};

struct shared_pgtable sharedmems[100];
struct spinlock memlock;




void shmlockinit(){
    initlock(&memlock,"memlock?");
    acquire(&memlock);
    for(int i =0; i < 100; i++){
        sharedmems[i].refs =0 ;
        sharedmems[i].pgid =0;
        // sharedmems[i].pageaddr =0;
    }
    release(&memlock);
    // cprintf("waeaweaowiea");
}
int open_sharedmem(int id, void **return_addr){
    acquire(&memlock);
    // cprintf("akhdlkasjaslkdjsadlkajdsalkdjsaldkjlkjdalasd");
    int found =0 ;
    for(int i =0; i < 100; i++){
        if (id == sharedmems[i].pgid){
            uint va = (PGROUNDUP(myproc()->sz));
            uint pa = V2P(sharedmems[i].pageaddr);
            myproc()->sz+= PGSIZE;        
            mappages(myproc()->pgdir,(void*)va,PGSIZE,pa,PTE_W|PTE_U);
            sharedmems[i].refs++;

            // *return_addr = (void*) sharedmems[i].pageaddr;
            *return_addr=(void*)va;
            // cprintf("what?");
            found =1;
            cprintf("refs : %d",sharedmems[i].refs);
            break;
        }
    }
    if (found ==0){
        for(int i =0; i <100; i++){
            if(sharedmems[i].refs == 0){
                sharedmems[i].pgid = id;
                sharedmems[i].pageaddr = (void*)kalloc();
                memset(sharedmems[i].pageaddr, 0, PGSIZE);
                sharedmems[i].refs++;
                uint va = myproc()->sz;
                myproc()->sz+= PGSIZE;
                mappages(myproc()->pgdir,(void*)va,PGSIZE,V2P(sharedmems[i].pageaddr),PTE_W|PTE_U);
                *return_addr=(void*)va;
                found =1;
                break;
            }
        }
    }
    release(&memlock);
    if (found == 0){
       return -1;
    }
    else{
        return 0;
    }
}


void unmap(pde_t *pgdir, void *va, uint size) {
  if (size <= 0)
    return;

  char *addr = (char *)va;
  char *end = addr + size;

  for (; addr < end; addr += PGSIZE) {
    pte_t *pte = walkpgdir(pgdir, addr, 0);
    if (pte != 0) {
      *pte = 0;
    }
  }

}

int close_sharedmem(int id){
    acquire(&memlock);
    int found =0;
    for (int i =0; i< 100; i++){
        if (sharedmems[i].pgid == id){
            sharedmems[i].refs--;
            uint va = PGROUNDUP(myproc()->sz) - PGSIZE;
            unmap(myproc()->pgdir, (void *)va, PGSIZE);
            if(sharedmems[i].refs == 0){
                kfree((char*)sharedmems[i].pageaddr);
                sharedmems[i].pageaddr = 0;
                sharedmems[i].pgid = 0;
                found  = 1;
                break;
            }
        }
    }


    release(&memlock);
    if (found == 0){
        return -1;
    }
    else{
        return 0;
    }
}












