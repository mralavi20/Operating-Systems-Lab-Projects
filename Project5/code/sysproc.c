#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"





int
sys_fork(void)
{
  return fork();
}

// int
// sys_iplock(void){
  
//   // initprioritylock(&plock,"plock");
//   return iplock();
// }

// int sys_aplock(void){
//   // acquirepriority(&plock);
//   return aplock();
// }

// int sys_rplock(void){
//   // releasepriority(&plock);
//   return rplock();
// }



int sys_iplock(void){
  initprioritylock(&plock,"name");
  return 0;
}
int sys_rplock(void){
  releasepriority(&plock);
  return 0;
}

int sys_aplock(void){
  acquirepriority(&plock);
  return 0;
}

int sys_open_sharedmem(void){
  int id;
  void** return_addr;
  argint(0, &id);
  argptr(1, (char **) (&return_addr),4);
  return open_sharedmem(id, return_addr);
}

int sys_close_sharedmem(void){
  int id;
  argint(0, &id);
  return close_sharedmem(id);

}



int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
int
sys_countsyscall(void)
{
  int sys_count[NCPU];
  pushcli();
  for(int i =0; i < NCPU; i++){
    sys_count[i] = cpus[i].syscall_count;
  }
  popcli();
  for (int i =0;i < NCPU; i++){
    cprintf("cpu %d: syscount: %d\n",i+1,sys_count[i]);
  }
  acquire(&count_lock);
  int c = syscall_counter;
  release(&count_lock);
  cprintf("total: %d\n", c);
  return 0;
}


