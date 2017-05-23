#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "uproc.h"

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
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
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
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      return -1;
    }
    sleep(&ticks, (struct spinlock *)0);
  }
  return 0;
}

// return how many clock tick interrupts have occurred
// since start. 
int
sys_uptime(void)
{
  uint xticks;
  
  xticks = ticks;
  return xticks;
}

//Turn of the computer
int sys_halt(void){
  cprintf("Shutting down ...\n");
// outw (0xB004, 0x0 | 0x2000);  // changed in newest version of QEMU
  outw( 0x604, 0x0 | 0x2000);
  return 0;
}

//return date
int
sys_date(void)
{
  struct rtcdate *d;
  //see argptr def in syscall.c for reminder
  if(argptr(0, (void*)&d, sizeof(*d)) < 0)
    return -1;
  //pass struct ptr to cmosttime
  cmostime(d);
  return 0; 
}

int
sys_getuid(void)
{
  return proc->uid;
}

int
sys_getgid(void)
{
  return proc->gid;
}

int
sys_getppid(void)
{
  //init proc has no parent
  return proc->parent ? proc->parent->pid : proc->pid;
}

int
sys_setuid(void)
{
  int stack_arg;
  uint uid;

  if(argint(0, &stack_arg) < 0)
    return -1;
  uid = (uint)stack_arg;
  if(uid < 0 || uid > 32767)
    return -1;  //out of bounds
  proc->uid = uid;
  return 0;
}

int
sys_setgid(void)
{
  int stack_arg;
  uint gid;

  if(argint(0, &stack_arg) < 0)
    return -1;
  gid = (uint)stack_arg;
  if(gid < 0 || gid > 32767)
    return -1;
  proc->gid = gid; 
  return 0;
}

int
sys_getprocs(void)
{
  int stack_arg;
  struct uproc *table;
 
  //return failure to retrieve arguments
  if(argint(0, &stack_arg) < 0)
    return -1; 
  if(argptr(1, (void*)&table, sizeof(*table)) < 0)
    return -1;
  //we're just a wrapper for getprocs() in proc.c
  return getprocs((uint)stack_arg, table);
}

// sets the priority of the calling process
// no list transitions since proc must be in RUNNING
int
sys_setpriority(void)
{ 
  int pid, priority;
  if(argint(0, &pid) < 0 || argint(1, &priority) < 0)
    return -1;

  //access to lists are needed, so call into proc.c 
  return setpriority(pid, priority);
}

#ifdef CS333_P5
int
sys_chmod(void)
{
  int mode;
  char *pathname;

  //return failure to retrieve arguments
  if(argptr(0, (void*)&pathname, sizeof(*pathname)) < 0)
    return -1;
  if(argint(1, &mode) < 0)
    return -1; 

//  cprint("Path: %s\nInt: %d", pathname, mode);

  return 0;
}

int
sys_chown(void)
{

  return 0;
}

int
sys_chgrp(void)
{

  return 0;
}

#endif
