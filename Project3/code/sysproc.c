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
sys_change_proc_queue (void) {
  int pid;
  int queue;

  argint (0, &pid);
  argint (1, &queue);

  return change_queue (pid, queue);
}

int
sys_set_bjf_proc (void) {
  int pid;
  float priority_rate;
  float coming_time_rate;
  float exec_c_rate;

  argint (0, &pid);
  argfloat (1, &priority_rate);
  argfloat (2, &coming_time_rate);
  argfloat (3, &exec_c_rate);

  return set_bjf_proc (pid, priority_rate, coming_time_rate, exec_c_rate);
}

int sys_set_sys_bjf (void) {
  float priority_rate;
  float coming_time_rate;
  float exec_c_rate;

  argfloat (0, &priority_rate);
  argfloat (1, &coming_time_rate);
  argfloat (2, &exec_c_rate);

  return set_sys_bjf (priority_rate, coming_time_rate, exec_c_rate);
}
