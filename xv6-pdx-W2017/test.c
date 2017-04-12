#include "types.h"
#include "user.h"
#include "uproc.h"

const uint TPS = 35;

static void
set_gid(uint gid)
{
  printf(2, "Setting GID to %d...\n", gid);
  int ret = setgid(gid);
  uint new_gid = getgid();
  if(new_gid == gid)
    printf(2, "SUCCESS: GID is: %d, Return code: %d \n", new_gid, ret);
  else 
    printf(2, "FAIL: UID is: %d, Return code: %d \n", new_gid, ret);
  sleep(5 * TPS);

}

static void
set_uid(uint uid)
{
  printf(2, "Setting UID to %d...\n", uid);
  int ret = setuid(uid);
  uint new_uid = getuid();
  if(new_uid == uid)
    printf(2, "SUCCESS: UID is: %d, Return code: %d \n", new_uid, ret);
  else 
    printf(2, "FAIL: UID is: %d, Return code: %d \n", new_uid, ret);
  sleep(5 * TPS);
}

static void
fork_test(uint guid, uint child_guid)
{
  int pid;

  printf(2, "Setting UID to %d, GID to %d\n", guid, guid);
  if(setuid(guid) < 0)
    printf(2, "Failed to set UID\n");
  if(setgid(guid) < 0)
    printf(2, "Failed to set GID\n");
  
  printf(2, "(Parent) Before fork... UID: %d, GID: %d, PID: %d\n", getuid(), getgid(), getpid());
  pid = fork();
  if(pid == 0 ) { //child fork
    
    printf(2, "(Child Before Set) PID: %d, PPID: %d, UID: %d, GID: %d\n", getpid(), getppid(), getuid(), getgid());
    if(setgid(child_guid) < 0)
      printf(2, "Setting child gid failed\n");
    if(setuid(child_guid) < 0)
      printf(2, "Setting child uid failed\n");
    printf(2, "(Child After Set) PID: %d, PPID: %d, UID: %d, GID: %d\n", getpid(), getppid(), getuid(), getgid());
  }
  else
    sleep(5 * TPS);  //wait for child to exit and return
}

int
testproctable(void)
{
  printf(1, "calling getprocs...\n");

  uint max = 1;
  struct uproc * table = (struct uproc *) malloc(sizeof(struct uproc) * 1);
  table[0].size = 51;
  int ret = getprocs(max, table);

  return ret;
}

int
main(int argc, char*argv[])
{
  int length = 5;
  uint guid1 = 30;
  uint guid2 = 67;
  uint success_nums[] = {0, 100, 12, 2929, 32767};
  uint fail_nums[] = {32768, 40000, 102000, 500000, -1}; //note -1 really means 0xFFF... (max uint)

//  testproctable();
/*  testing cpu_time
  while(1 ==1){
   ;
  }
*/

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
