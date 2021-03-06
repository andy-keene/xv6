#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "uproc.h"

// #defines are located in proc.h

//Will use conditional compilation for all P3/P4...
#ifdef CS333_P3P4
struct StateLists {
  struct proc* ready[MAX+1];
  struct proc* free;
  struct proc* sleep;
  struct proc* zombie;
  struct proc* running;
  struct proc* embryo;
};
#endif

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
  #ifdef CS333_P3P4
  struct StateLists pLists;
  uint PromoteAtTime;
  #endif
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

#ifdef CS333_P3P4
void
printList(struct proc ** stateList);
#ifdef DEBUG
static void
checkProcs(char *s);
#endif //end DEBUG flag
#endif //end P3P4 flag


//Helper functions for P3
// Remove a process from the given list in O(n) time
// both asserting the process is in the given state,
// and that the lock is held; return code accordingly.
// Add a process to the list (at the end) in O(n) time
// Note that the free list will be handled seperately
// since we aren't looking for a specific process

#ifdef CS333_P3P4


// ------ exit(), kill(), wait() helper functions -------
// Traverse given staetList passing children of parent to initproc
// used by exit()
// efficiency: O(n)
static void
abandonChildren(struct proc* stateList, struct proc* parent)
{
  //Assert lock is held, and list is non-empty
  if(!holding(&ptable.lock))
    panic("Not holding lock when accessing state list (remove)");
  
  //for clarity
  struct proc* p = stateList; 

  while(p){
    if(p->parent == parent){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
    p = p->next;
  }
}

// Looks for process with given pid in the given stateList
// on success returns 0 and p now refferencing the process with pid
// on failure returns 0
// efficiency: O(n)
static int
getProcess(struct proc* stateList, struct proc** p, int pid)
{
  struct proc *curr = stateList;

  while(curr){
    if(curr->pid == pid){
      //p will point to the process with pid on return
      *p = curr; 
      return 0;
    } 
    curr = curr->next;
  } 

  //failed to find, return failure
  return -1;
}

// Looks for a process with given pid in the statelists: ready, running, sleeping, and zombie
// on success returns 0 on success with p now refferencing the process
// on failure returns -1
// used by used by kill()
static int 
findProcess(struct proc** p, int pid)
{ 
  //Assert lock is held, and list is non-empty
  if(!holding(&ptable.lock))
    panic("Not holding lock when accessing state list (remove)");

  int rc = -1;

  if(getProcess(ptable.pLists.embryo, p, pid) == 0){
    rc = 0;
  }
  else if(getProcess(ptable.pLists.running, p, pid) == 0){
    rc = 0;
  }
  else if(getProcess(ptable.pLists.sleep, p, pid) == 0){
    rc = 0;
  }
  else if(getProcess(ptable.pLists.zombie, p, pid) == 0){
    rc = 0;
  }
  else {
    for(int i = 0; i < MAX + 1; i++){
      if(getProcess(ptable.pLists.ready[i], p, pid) == 0){     // add queu != NULL &&?
        rc = 0;
        break;  // break at first queue containing pid
      }
    }
  }
  return rc;
}

// Looks for children in given statelist
// on success returns 1
// on failure return 0
// efficiency: O(n)
// note: These are opposite return codes to the usuall 0 == success, -int = failure!
static int
findChild(struct proc* stateList, struct proc* parent)
{
  struct proc *curr = stateList;
  
  while(curr){
    if(curr->parent == parent){
      return 1;
    }
    curr = curr->next;
  } 
  return 0;
}

// Looks for children in ready, sleeping, and running state lists
// on success returns 1
// on failure return 0
// note: These are opposite return codes to the usuall 0 == success, -int = failure!
static int
hasChildren(struct proc* parent)
{
  int rc = 0;

  if(findChild(ptable.pLists.embryo, parent)){
    rc = 1; //added per emailed instructions
  }
  else if(findChild(ptable.pLists.running, parent)){
    rc = 1;
  }
  else if(findChild(ptable.pLists.sleep, parent)){
    rc = 1;
  }
  else { 
    for(int i = 0; i < MAX + 1; i++){
      if(findChild(ptable.pLists.ready[i], parent)){
        rc = 1;
        break; // break at first queue containing child
      }
    }
  }

  return rc;
}

// ------ statelist transition helpers -----
// Removes the given process from the state list
// Returns 0 if the process was in the correct state and removed form the list
// -1 if the process was not found
// efficiency: O(n)
static void
removeFromStateList(struct proc** stateList, struct proc* p, enum procstate state)
{
  //Assert lock is held, and list is non-empty
  if(!holding(&ptable.lock))
    panic("Not holding lock when accessing state list (remove)");
  if(p->state != state)
    panic("Process has incorrect state");

  #ifdef DEBUG
  checkProcs("Calling from removeFromStatelist"); //demonstrate list invariant is held
  #endif

  if(*stateList == p){
    *stateList = (*stateList)->next;
    p->next = 0;
    return;
  }  
  else if(*stateList){
    struct proc *curr = *stateList;
    while(curr->next){
      if(curr->next == p){
        curr->next = curr->next->next;
        p->next = 0;
        return;
      }
      curr = curr->next;
     }
  }
  cprintf("Process: %s not found on statelist: %d\n", p->name, state);
  panic("Process not found");
}

// Asserts lock is held and proc * p is in "state"
// Returns 0 and stateList head as p on success
// or -1 on failure to find a process (empty list)
// efficiency: O(1)
static int
popHeadFromStateList(struct proc** stateList, struct proc** p, enum procstate state)
{
  //Assert lock is held, and list is non-empty
  if(!holding(&ptable.lock))
    panic("Not holding lock when accessing state list (pop)");

  //Try to pop head
  if(!(*stateList))
    return -1;
  *p = *stateList;
  *stateList = (*stateList)->next;
  if((*p)->state != state)
    panic("Process not in correct state");
  return 0;
}

// Only to be called from userinit()
// Places all processes in the table onto the Unused list
// since each is initially set to unused.
// Asserts state of each proccess is "UNUSED"
// efficiency: O(|ptable|)
static void
initUnused(void)
{
  struct proc *p;
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  { 
    //assert state, and add to list
    if(p->state != UNUSED)
      panic("Found USED process in UNUSED init");
    p->next = ptable.pLists.free; 
    ptable.pLists.free = p;
  }
}

// Adds to end of statelist
// No return code as append cannot fail
// efficiency: O(n)
static void
appendToStateList(struct proc** stateList, struct proc* p, enum procstate state)
{
  //Assert lock is held
  if(!holding(&ptable.lock))
    panic("Not holding lock when accessing state list (append)");

  if(!*stateList){
    *stateList = p;
  } else {
    struct proc *curr = *stateList;
    while(curr->next)
      curr = curr->next;
    curr->next = p;
  }
  p->state = state;
  p->next = 0;
  #ifdef DEBUG
  //demonstrate list invariant is held
  checkProcs("Calling from append to statelist");
  #endif
}

// Adds to head of state list
// No return code as prepend cannot fail
// efficiency: O(1)
static void
prependToStateList(struct proc** stateList, struct proc* p, enum procstate state)
{
  //lock-held assert
  if(!holding(&ptable.lock))
    panic("Not holding lock when accessing state list (prepend)");

  p->next = *stateList;
  *stateList = p;
  p->state = state;
  #ifdef DEBUG
  //demonstrate list invariant is held
  checkProcs("Calling from append to statelist"); 
  #endif
}

// ------- priority and budget helper functions for MLFQ --------

// clone of appendToStateList() but does not set state and nullify 
// last ptr; instead simply appends p to stateList
static void
appendToQueue(struct proc** stateList, struct proc* p)
{
  //Assert lock is held
  if(!holding(&ptable.lock))
    panic("Not holding lock when accessing state list (append)");

  if(!*stateList){
    *stateList = p;
  } else {
    struct proc *curr = *stateList;
    while(curr->next)
      curr = curr->next;
    curr->next = p;
  }
}

// For each process on the given a stateList increases it's priority
// by 1 iff the current priority != 0 (i.e subtracts one from p->priority)
// and resets the budget to 
// DEFAULT_BUDGET. Built for high reuseability
// efficiency: O(n) where n is the size of the state list
static void
setPrioBudget(struct proc* stateList) //, uint new_priority)
{
  //just use stateList as curr (is is a copy of the memory address)
  while(stateList){
    stateList->priority -= (stateList->priority == 0) ? 0 : 1;
    stateList->budget = DEFAULT_BUDGET;
    stateList = stateList->next;
  }
}

static void
priorityPromotion(void)
{
  //Assert lock is held, and list is non-empty
  if(!holding(&ptable.lock))
    panic("Not holding lock when accessing state list (remove)");

  // handle priority promotion on ready queues
  for(int i = 0; i < MAX; i++){
    appendToQueue(&ptable.pLists.ready[i], ptable.pLists.ready[i+1]);
    setPrioBudget(ptable.pLists.ready[i]);
    ptable.pLists.ready[i+1] = 0;  // key: nullify next list so the append for i > 0 is done in O(1) time
  }

  // promote sleep and running (2 CPUs)
  // no others need be considered
  setPrioBudget(ptable.pLists.sleep);
  setPrioBudget(ptable.pLists.running);

  // can't call checkprocs from here since a CPU may start before 
  // userinit(). It starts scheduler which calls this. 
}
#endif

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}


//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;
  #ifdef CS333_P3P4
  int rc;
  #endif
 
  acquire(&ptable.lock);
  #ifndef CS333_P3P4
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;
  #else 
  rc = popHeadFromStateList(&ptable.pLists.free, &p, UNUSED);
  if(rc == 0)
    goto found;
  //cprintf("Failed to find free process..\n");
  #endif
  release(&ptable.lock);
  return 0;

found:
  #ifndef CS333_P3P4 
  p->state = EMBRYO;
  #else
  // move p -> EMBRYO
  // set default prio/bdgt while lock is held
  prependToStateList(&ptable.pLists.embryo, p, EMBRYO);
  p->priority = 0;
  p->budget = DEFAULT_BUDGET;
  #endif
  p->pid = nextpid++;
  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    #ifndef CS333_P3P4
    p->state = UNUSED;
    #else
    //we must reaqquire the lock to switch the process back to the free list
    //move p EMBRYO -> RUNNING
    acquire(&ptable.lock);
    removeFromStateList(&ptable.pLists.embryo, p, EMBRYO);
    prependToStateList(&ptable.pLists.free, p, UNUSED);
    release(&ptable.lock);
    #endif
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;
  
  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;
  
  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  //initialize running time counts
  p->cpu_ticks_total = 0;
  p->cpu_ticks_in = 0;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  #ifdef CS333_P3P4 
  acquire(&ptable.lock);
  initUnused();
  ptable.PromoteAtTime = ticks + TICKS_TO_PROMOTE;
  release(&ptable.lock);
  #endif    

  p = allocproc();
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");
  
  // 1-st process UID/GID in param.h
  p->uid = DEFAULT_UID; 
  p->gid = DEFAULT_GID;
  p->start_ticks = ticks; 
  #ifndef CS333_P3P4
  p->state = RUNNABLE;
  #else
  //Move initproc from EMBRYO->RUNNABLE
  acquire(&ptable.lock);
  removeFromStateList(&ptable.pLists.embryo, p, EMBRYO);
  appendToStateList(&ptable.pLists.ready[p->priority], p, RUNNABLE);
  release(&ptable.lock);
  #endif
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  
  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  proc->sz = sz;
  switchuvm(proc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process; on success np in EMBRYO
  if((np = allocproc()) == 0)
    return -1;

  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    #ifndef CS333_P3P4 
    np->state = UNUSED;
    #else
    //Move new proc from EMBRYO->UNUSED
    //added in P5
    acquire(&ptable.lock);
    removeFromStateList(&ptable.pLists.embryo, np, EMBRYO);
    prependToStateList(&ptable.pLists.free, np, UNUSED);
    release(&ptable.lock);
    #endif
    return -1;
  }
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);

  safestrcpy(np->name, proc->name, sizeof(proc->name));
 
  pid = np->pid;
  np->uid = proc->uid;
  np->gid = proc->gid;
  np->start_ticks = ticks;

  #ifndef CS333_P3P4
  // lock to force the compiler to emit the np->state write last.
  acquire(&ptable.lock);
  np->state = RUNNABLE;
  release(&ptable.lock);
  #else
  //Move np from EMBRYO->RUNNABLE
  acquire(&ptable.lock);
  removeFromStateList(&ptable.pLists.embryo, np, EMBRYO);
  appendToStateList(&ptable.pLists.ready[np->priority], np, RUNNABLE);
  release(&ptable.lock);
  #endif
  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
#ifndef CS333_P3P4
void
exit(void)
{
  struct proc *p;
  int fd;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(proc->cwd);
  end_op();
  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }
 
  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}
#else
void
exit(void)
{
  //struct proc *p;
  int fd;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(proc->cwd);
  end_op();
  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Pass abandoned children to init.
  abandonChildren(ptable.pLists.embryo, proc);
  abandonChildren(ptable.pLists.running, proc);
  abandonChildren(ptable.pLists.sleep, proc);
  abandonChildren(ptable.pLists.zombie, proc);
  //Abandon children in each queue in MLFQ
  for(int i = 0; i < MAX + 1; i++){
    abandonChildren(ptable.pLists.ready[i], proc);
  }
  // move proc RUNNING -> ZOMBIE
  removeFromStateList(&ptable.pLists.running, proc, RUNNING);
  prependToStateList(&ptable.pLists.zombie, proc, ZOMBIE);

  // Jump into the scheduler, never to return.
  sched();
  panic("zombie exit");
}
#endif

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
#ifndef CS333_P3P4
int
wait(void)
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}
#else
int
wait(void)
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);

  for(;;){
    // Scan through table looking for zombie children.
    // set flag accoridingly
    havekids = hasChildren(proc);
   
    //treat zombie list differently
    p = ptable.pLists.zombie;
    while(p){
      if(p->parent == proc){ 
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        // move p ZOMIE->UNUSED
        removeFromStateList(&ptable.pLists.zombie, p, ZOMBIE);
        prependToStateList(&ptable.pLists.free, p, UNUSED); 
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        release(&ptable.lock);
        return pid;
      }
      p = p->next;
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}
#endif

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
#ifndef CS333_P3P4
// original xv6 scheduler. Use if CS333_P3P4 NOT defined.
void
scheduler(void)
{
  struct proc *p;
  int idle;  // for checking if processor is idle

  for(;;){
    // Enable interrupts on this processor.
    sti();

    idle = 1;  // assume idle unless we schedule a process
    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      idle = 0;  // not idle this timeslice
      proc = p;
      switchuvm(p);
      p->state = RUNNING;
      //start_ticks before contx swtch
      p->cpu_ticks_in = ticks;
      swtch(&cpu->scheduler, proc->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      proc = 0;
    }
    release(&ptable.lock);
    // if idle, wait for next interrupt
    if (idle) {
      cprintf("Scheduler sleeping...\n");
      sti();
      hlt();
    }
  }
}

#else

void
scheduler(void)
{
  struct proc *p;
  int idle;  // for checking if processor is idle
  int found_proc;

  for(;;){
    // Enable interrupts on this processor.
    sti();
    idle = 1;         // assume idle unless we schedule a process
    found_proc = 0;   // reset flag for finding proc

    acquire(&ptable.lock);
    //check for promotion
    if(ptable.PromoteAtTime <= ticks){
      priorityPromotion();
      ptable.PromoteAtTime = ticks + TICKS_TO_PROMOTE;
    }
    //find next process to run (priority high -> low)
    for(int i = 0; i < MAX + 1; i++){
      if(popHeadFromStateList(&ptable.pLists.ready[i], &p, RUNNABLE) == 0){
        found_proc = 1;
        break;
      }
    }

    if(found_proc == 1){
      prependToStateList(&ptable.pLists.running, p, RUNNING);      
      
      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      idle = 0;  // not idle this timeslice
      proc = p;
      switchuvm(p);
      //start_ticks before contx swtch
      p->cpu_ticks_in = ticks;
      swtch(&cpu->scheduler, proc->context);

      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      proc = 0;
    }
    release(&ptable.lock);
    // if idle, wait for next interrupt
    if (idle) {
      sti();
      hlt();
    }
  }
}
#endif

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
#ifndef CS333_P3P4
void
sched(void)
{
  int intena;

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;
  //add cpu run time before swapping out
  proc->cpu_ticks_total += ticks - proc->cpu_ticks_in;
  swtch(&proc->context, cpu->scheduler);
  
  cpu->intena = intena;
}
#else
void
sched(void)
{
  int intena;
  uint cpu_ticks;

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;

  //update cpu time and budget
  cpu_ticks = ticks - proc->cpu_ticks_in;
  proc->budget -= cpu_ticks;
  proc->cpu_ticks_total += cpu_ticks;

  if(proc->budget <= 0){     //awkward nested ifs, but more performant than flags
    if(proc->state == RUNNABLE && proc->priority < MAX){
      removeFromStateList(&ptable.pLists.ready[proc->priority], proc, RUNNABLE);  
      appendToStateList(&ptable.pLists.ready[proc->priority + 1], proc, RUNNABLE);
    }
    proc->budget = DEFAULT_BUDGET;
    proc->priority += (proc->priority < MAX) ? 1 : 0;
    //although it may already be on new list, invariant is held since we have the lock
  }

  swtch(&proc->context, cpu->scheduler);
  
  cpu->intena = intena;
}
#endif

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  #ifndef CS333_P3P4
  proc->state = RUNNABLE;
  #else
  // move proc RUNNING->RUNNABLE
  removeFromStateList(&ptable.pLists.running, proc, RUNNING);
  appendToStateList(&ptable.pLists.ready[proc->priority], proc, RUNNABLE); 
  #endif
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot 
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }
  
  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
// 2016/12/28: ticklock removed from xv6. sleep() changed to
// accept a NULL lock to accommodate.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){
    acquire(&ptable.lock);
    if (lk) release(lk);
  }

  // Go to sleep.
  proc->chan = chan;
  #ifndef CS333_P3P4
  proc->state = SLEEPING;
  #else
  // move proc RUNNING -> SLEEPING
  removeFromStateList(&ptable.pLists.running, proc, RUNNING);
  prependToStateList(&ptable.pLists.sleep, proc, SLEEPING);
  #endif
  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){ 
    release(&ptable.lock);
    if (lk) acquire(lk);
  }
}

//PAGEBREAK!
#ifndef CS333_P3P4
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}
#else

// wake up all processes sleeping on chann
// efficiency: O(|sleepingList|) 
static void
wakeup1(void *chan)
{
  struct proc *p;
  struct proc *curr = ptable.pLists.sleep;

  //look through sleep list and remove those on given *chan
  while(curr){
    if(curr->chan == chan){
      p = curr;
      curr = curr->next;  //skip ahead since we must delete p
      //move p SLEEPING -> RUNNABLE
      removeFromStateList(&ptable.pLists.sleep, p, SLEEPING);
      appendToStateList(&ptable.pLists.ready[p->priority], p, RUNNABLE);
    }
    else{
      curr = curr->next;
    }
  }

}
#endif

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
#ifndef CS333_P3P4
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}
#else
int
kill(int pid)
{
  struct proc *p;
  int rc = -1;  //assume failure

  acquire(&ptable.lock);

  //use helper function to search ready, running, sleeping, and zombie lists
  if(findProcess(&p, pid) == 0){
    rc = 0;
    p->killed = 1;
    if(p->state == SLEEPING){
      //move p SLEEPING-> RUNNABLE 
      removeFromStateList(&ptable.pLists.sleep, p, SLEEPING);
      appendToStateList(&ptable.pLists.ready[p->priority], p, RUNNABLE); 
    }
  }
  
  release(&ptable.lock);
  return rc;
}
#endif

static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
};

//prints a number with leading zero in hundreths place
void
printnum(const uint num)
{
  if(num % 100 > 9)
    cprintf("%d.%d\t", num / 100, num % 100);
  else
    cprintf("%d.%d%d\t", num / 100, 0, num % 100);

}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
// Note: we must grab start_time at the beginning of the function
//       for consistency between diff. process dumps 
void
procdump(void)
{
  int i;
  struct proc *p;
  char *state;
  uint pc[10];
  uint curr_ticks = ticks;
  #ifdef DEBUG
  cprintf("\nNPROCS = %d\n", NPROC); //to be used for P3/P4 testing
  #endif
  cprintf("\nPID\tName\tUID\tGID\tPPID\tPrio\tELapsed\tCPU\tState\tSize\tPCs\n");
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d\t%s\t%d\t%d\t%d\t%d\t", p->pid, p->name, p->uid, p->gid, (p->parent ? p->parent->pid : p->pid), p->priority);
    printnum(curr_ticks - p->start_ticks); 
    printnum(p->cpu_ticks_total);
    cprintf("%s\t%d\t", state, p->sz);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf("%p ", pc[i]);
    }
    cprintf("\n");
  }
}

int
getprocs(uint max, struct uproc *table)
{
  struct proc *p;
  char *state;
  uint i = 0;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC] && i < max; p++){
    if(p->state == UNUSED)
      continue;  //skip unused processes
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    //both p->name and state < STRMAX
    strncpy(table[i].state, state, STRMAX);
    strncpy(table[i].name, p->name, STRMAX);
    table[i].pid = p->pid; 
    table[i].uid = p->uid;
    table[i].gid = p->gid;
    table[i].ppid = (p->parent ? p->parent->pid : p->pid);
    table[i].priority = p->priority;
    //is it fine to calculate the elapsed time in-line?
    table[i].elapsed_ticks = ticks - p->start_ticks;
    table[i].cpu_total_ticks = p->cpu_ticks_total;
    table[i].size = p->sz;
    i++;
  }
  release(&ptable.lock);

  //return number of elements filled
  return i;
}

#ifdef CS333_P3P4

// helper for sys_setpriority system call
// since we must access the list to find pid
// and may need to make a list transition
int
setpriority(int pid, int priority)
{
  int rc = -1;
  struct proc *p;

  if(priority < 0 || priority > MAX)    
    return -1;
  acquire(&ptable.lock);
  // if process with pid is found, update priority
  // and budget. For reusability we search the
  // zombie and embryo list (see Marks email) 
  if(findProcess(&p, pid) == 0){
    rc = 0;
    if(p->state == RUNNABLE){
      removeFromStateList(&ptable.pLists.ready[p->priority], p, RUNNABLE);
      appendToStateList(&ptable.pLists.ready[priority], p, RUNNABLE);
    }
    p->priority = priority;
    p->budget = DEFAULT_BUDGET;
  }
  release(&ptable.lock);
  return rc;
}

#ifdef DEBUG

// Given debug code: modified to return n
// where n is the number of lists process p is on
static int
findProc(struct proc *p)
{
  struct proc *np;
  int count = 0;
  
  np = ptable.pLists.free;
  while (np != NULL) {
    if (np == p) count += 1;
    np = np->next;
  }
  np = ptable.pLists.embryo;
  while (np != NULL) {
    if (np == p) count += 1;
    np = np->next;
  }
  np = ptable.pLists.running;
  while (np != NULL) {
    if (np == p) count += 1;
    np = np->next;
  }
  np = ptable.pLists.sleep;
  while (np != NULL) {
    if (np == p) count += 1;
    np = np->next;
  }
  np = ptable.pLists.zombie;
  while (np != NULL) {
    if (np == p) count += 1;
    np = np->next;
  } 
  for(int i = 0; i < MAX + 1; i++){
    np = ptable.pLists.ready[i];
    while (np != NULL) {
      if (np == p) count += 1;
      np = np->next;
    }
  }
  return count; // not found
}

// argument 's' is a string to print if
// a process is not on one and only one list.
// asserts list invariant
static void
checkProcs(char *s)
{
  int found;
  int isholding;
  struct proc *p;
  int index = 0;
  isholding = holding(&ptable.lock);
  if (!isholding) acquire(&ptable.lock);
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    found = findProc(p);
    if (found == 1)  //invarient holds. 
      continue;
    if (found == 0)
      cprintf("[Index %d] Name: %s PID: %d State: %d not found on *any* list\n", index, p->name, p->pid, p->state);
    if (found > 1)
      cprintf("Process %s (PID: %d) found on %d lists\n", p->name, p->pid, found);
    index +=1;
    cprintf("Checkprocs diagnostic message: (%s)\n", s);
    panic("Process array and lists inconsistent\n");
  }
  if (!isholding) release(&ptable.lock);
}
#endif  //end DEBUG 

// For console-R
// print PID.1 -> PID.2 -> ... for free list
void
readylistinfo(void)
{
  acquire(&ptable.lock);
  cprintf("Ready List Processes:\n");

  for(int i = 0; i < MAX + 1; i++){
    cprintf("%d: ", i);
    struct proc *curr = ptable.pLists.ready[i];  
    while(curr){
      cprintf("(%d,%d)", curr->pid, curr->budget);
      if(curr->next)
        cprintf(" -> ");
      curr = curr->next;
    }
    cprintf("\n");
  }
  release(&ptable.lock);
  //terminate listing
  cprintf("\n");
}

// For console-F
// prints number of free processes available 
void
freelistinfo(void)
{ 
  acquire(&ptable.lock);
  struct proc *curr = ptable.pLists.free;
  int num = 0;

  while(curr){
    num += 1;
    curr = curr->next;
  }
  release(&ptable.lock);
  cprintf("Free List Size: %d processes\n", num);
}

// For console-S
// print PID.1 -> PID.2 -> ... for sleeping list
void
sleepinglistinfo(void)
{
  acquire(&ptable.lock);
  struct proc *curr = ptable.pLists.sleep;

  cprintf("Sleeping List Processes:\n");
  while(curr){
    cprintf("%d", curr->pid);
    if(curr->next)
      cprintf(" -> ");
    curr = curr->next;
  }
  release(&ptable.lock);
  //terminate listing
  cprintf("\n");
}

// For console-Z
// print (PID.1, PPID.1) -> ... for zombie list
void
zombielistinfo(void)
{
  acquire(&ptable.lock);
  struct proc *curr = ptable.pLists.zombie;

  cprintf("Zombie List Processes:\n");
  while(curr){
    //is checking for init ever possible on zombie?
    cprintf("(%d, %d)", curr->pid, curr->parent ? curr->parent->pid : curr->pid);
    if(curr->next)
      cprintf(" -> ");
    curr = curr->next;
  }
  release(&ptable.lock);
  //terminate listing
  cprintf("\n");

}

#endif //end CS333_P3P4 flag
