#include "types.h"
#include "user.h"
#include "uproc.h"
#include "param.h" //for use of NPROC

static void
set_gid(uint gid)
{
  printf(2, "Setting GID to %d...\n", gid);
  int ret = setgid(gid);
  sleep(2 * TPS);
  uint new_gid = getgid();
  if(new_gid == gid)
    printf(2, "SUCCESS: GID is: %d, Return code: %d \n", new_gid, ret);
  else 
    printf(2, "FAIL: UID is: %d, Return code: %d \n", new_gid, ret);

}

static void
set_uid(uint uid)
{
  printf(2, "Setting UID to %d...\n", uid);
  int ret = setuid(uid);
  sleep(2 * TPS);
  uint new_uid = getuid();
  if(new_uid == uid)
    printf(2, "SUCCESS: UID is: %d, Return code: %d \n", new_uid, ret);
  else 
    printf(2, "FAIL: UID is: %d, Return code: %d \n", new_uid, ret);
}

static void
fork_test(uint uid, uint gid)
{
  int pid;

  printf(2, "(parent) Setting UID to %d, GID to %d\n", uid, gid);
  if(setuid(uid) < 0)
    printf(2, "Failed to set UID\n");
  if(setgid(gid) < 0)
    printf(2, "Failed to set GID\n");
  
  printf(2, "(parent) Before fork... UID: %d, GID: %d, PID: %d\n", getuid(), getgid(), getpid());
  pid = fork();
  if(pid == 0 ) { //child fork
    
    printf(2, "(child) UID: %d, GID: %d, PID: %d, PPID:%d \n", getuid(), getgid(), getpid(), getppid() );
    sleep(2 * TPS);
  }
  else
    wait();  //wait for child to exit and return
}

void
testget(void)
{
  uint uid = getuid();
  uint gid = getgid();
  uint ppid = getppid();
  printf(2, "(test) PPID: %d, UID: %d, GID:%d \n", ppid, uid, gid);
  sleep(2*TPS);
}

void p2tests(void)
{
  int length = 5;
  uint guid1 = 30;
  uint guid2 = 67;
  uint success_nums[] = {0, 100, 12, 2929, 32767};
  uint fail_nums[] = {32768, 40000, 102000, 500000, -1}; //note -1 really means 0xFFF... (max uint)

  //test fail success get pid/gid
  sleep(2*TPS);
  set_gid(50);
  set_uid(11);
  sleep(2*TPS);
  exit();

  //test fail setuid(), setgid()
  set_gid(38000);
  set_uid(38000);

  //test fork()
  fork_test(191, 67);

  //test getuid(), getgid(), getppid()
  testget(); 


  printf(1, "**Successfull tests**\n");
  for(int i = 0; i < length; i++){
    set_uid(success_nums[i]);
  }

  printf(1, "**Fail tests**\n");
  for(int i = 0; i < length; i++){
    set_gid(fail_nums[i]);
  }

  printf(1, "**Fork tests**\n");
  fork_test(guid1, guid2);
}

//test free and zombie lists
void free_zombie_tests(void)
{
  int children[NPROC];
  int pid, num_child = 0;
  printf(2, "(Parent, PID: %d) About to begin creating children...\n", getpid());
  sleep(2*TPS);

  //keep forking until fork() fails.
  while( (pid = fork()) != -1){

    if(pid == 0){ 
      while(1)
        ; //spin so the parent can reap as it wishes.
    }
    children[num_child++] = pid; //store in num_child the pid, increment after
  }
  
  //Now fork() has failed so no more processes can be creates
  printf(2, "(Parent) Created %d children, going to sleep for a hot sec.\n", num_child);
  sleep(3*TPS);

  //Now lets reap the children! in reverse order. (decrement num_child to match max index)
  for(num_child -= 1 ; num_child >= 0; num_child--){
    //mark for death, then wait to reap the child.
    kill(children[num_child]);
    printf(2, "Marked [%d] child with PID of %d for death\n", num_child, children[num_child]);
    sleep(2*TPS);

    while(wait() != children[num_child])
      ; //spin waiting to reap child.

    printf(2, "Reaped [%d] child with PID of %d\n", num_child, children[num_child]);
    sleep(2*TPS);
  }
  printf(2, "(Parent) Reaping complete...\n");
  sleep(5*TPS);
}

void sleep_test(void)
{

  printf(2, "Process: %d going to sleep!\n", getpid());
  sleep(10*TPS);
  printf(2, "Process: I'm awake, and I'm going to exit\n");
}

void inf_loops(void)
{
  int pid, num = 5;

  printf(2, "Parent of Infiite Loops: %d\n", getpid());
  while(num > 0){

    pid = fork();
    if(pid == 0){
      while(1) ;
    }
    num -= 1;
  }

  while(1) ;

}


void round_robin(void)
{
  int pid, num = 20;

  while(num > 0){

    pid = fork();
    if(pid == 0){
      while(1)
        ;
    }
    num -= 1;
  }
  printf(2, "(Parent) Children ready and spinning");
  while(1) ;

}


void p3tests(void)
{
//  inf_loops();
//  sleep_test();
//  round_robin();
   free_zombie_tests();

}
void sys_calls(void)
{
  int rc;
  rc = chmod("dir1/dir/chmod.txt", 1111 );
  printf(2, "chmod returned %d\n", rc);
  rc = chown("dir1/dir/chown.txt", 2222);
  printf(2, "chmown returned %d\n", rc);
  rc = chgrp("dir1/dir/chgrp.txt", 3333);
  printf(2, "chgrp returned %d\n", rc);
}

void p5tests(void)
{


}
int
main(int argc, char*argv[])
{
  p3tests();
//  p2tests();

  exit();
}
