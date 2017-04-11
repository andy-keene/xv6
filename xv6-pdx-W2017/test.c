#include "types.h"
#include "user.h"
void
set_uid(uint uid)
{
  printf(2, "Setting UID to %d...\n", uid);
  int ret = setuid(uid);
  uint new_uid = getuid();
  if(new_uid == uid)
    printf(2, "SUCCESS: UID is: %d, Return code: %d \n", new_uid, ret);
  else 
    printf(2, "FAIL: UID is: %d, Return code: %d \n", new_uid, ret);
}

int
main(int argc, char*argv[])
{
//  uint uid; //, gid, ppid;
  int length = 10;
  uint success_nums[] = {0, 100, 12, 30000, 129, 21021, 900, 2013, 2929, 32767};

  printf(1, "**Successfull tests**");
  for(int i = 0; i < length; i++){
    set_uid(success_nums[i]);
  }

  exit();
}
