#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    if(addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

struct fork_stats {
  int attempts;
  int successful;
  int blocked;
};

static struct fork_stats fstats_25 = {0,0,0};

uint64
sys_fork_stats_25(void)
{
  int priority, max_children;
  struct proc *p = myproc();
  int children = 0;

  argint(0, &priority);
  argint(1, &max_children);

  fstats_25.attempts++;

  struct proc *pp;
  extern struct proc proc[];
  for(pp = proc; pp < &proc[NPROC]; pp++) {
    if(pp->parent == p && pp->state != UNUSED) {
      children++;
    }
  }

  if(children >= max_children) {
    fstats_25.blocked++;
    printf("fork_stats_25: BLOCKED attempts=%d successful=%d blocked=%d\n",
           fstats_25.attempts, fstats_25.successful, fstats_25.blocked);
    return -1;
  }

  int pid = kfork();

  if(pid < 0){
    fstats_25.blocked++;
    printf("fork_stats_25: FAILED attempts=%d successful=%d blocked=%d\n",
           fstats_25.attempts, fstats_25.successful, fstats_25.blocked);
    return -1;
  }

  if(pid == 0){
    myproc()->priority = priority;
  }

  fstats_25.successful++;

  printf("fork_stats_25: attempts=%d successful=%d blocked=%d\n",
         fstats_25.attempts, fstats_25.successful, fstats_25.blocked);
  
  return pid;       
}
