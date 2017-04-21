#include "types.h"
#include "user.h"
#include "uproc.h"

const uint TPS = 100;

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

int
main(int argc, char*argv[])
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
  exit();
}
