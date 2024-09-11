#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "prioritylock.h"




void
initprioritylock (struct prioritylock *lk, char *name)
{
  initlock(&lk->lk, "priority lock");
  // cprintf("lock with name %s initialized",name);
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
  lk->queue_size = 0;
}

int
find_min_pid (struct prioritylock *lk) {
    if (lk->queue_size < 1) {
        return -1;
    }
    return lk->pid_queue[0];
}

void
add_pid (struct prioritylock *lk, int pid) {
    int value;
    int i;

    lk->queue_size = lk->queue_size + 1;
    lk->pid_queue[lk->queue_size - 1] = pid;

    for (i = lk->queue_size - 1; i > 0; i--) {
        if (lk->pid_queue[i] < lk->pid_queue[i - 1]) {
            value = lk->pid_queue[i - 1];
            lk->pid_queue[i - 1] = lk->pid_queue[i];
            lk->pid_queue[i] = value; 
        }
    }
}

void
remove_first_pid (struct prioritylock *lk) {
    int i;

    if (lk->queue_size < 1) {
        return;
    }

    lk->queue_size = lk->queue_size - 1;

    if (lk->queue_size < 1) {
        return;
    }

    for (i = 1; i <= lk->queue_size; i++) {
        lk->pid_queue[i - 1] = lk->pid_queue[i];
    }
}

void
acquirepriority (struct prioritylock *lk)
{
  lk->validnum++;
  acquire(&lk->lk);
  add_pid (lk, myproc ()->pid);
  while (lk->locked || (lk->queue_size > 1 && find_min_pid (lk) != myproc ()->pid)) {
    sleep(lk, &lk->lk);
  }
  remove_first_pid (lk);
  // cprintf("process with pid %d acquired the lock\n",myproc()->pid);
  lk->locked = 1;
  lk->pid = myproc()->pid;
  release(&lk->lk);
}

void
releasepriority (struct prioritylock *lk)
{
  acquire(&lk->lk);
  
  if (lk->pid != myproc ()->pid) {
    release(&lk->lk);
    return;
  }

  lk->locked = 0;

  // cprintf("process with pid %d released the lock\n",myproc()->pid);
  // cprintf("list of other people in list:\n");
  // for(int i =0; i< 1000; i++){
    // if (lk->pid_queue[i] >0 && i < lk->validnum-1)
      // cprintf("procces with pid: %d has rank %d\n",lk->pid_queue[i],i+1);
  // }
  // cprintf("-----------------------");
  lk->pid = 0;
  lk->validnum--;
  wakeup(lk);
  release(&lk->lk);
}