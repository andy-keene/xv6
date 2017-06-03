//User command to modify uid of a file in the xv6 file system
//usage: chown <OWNER> <FILE> where FILE specifies the path to the file
//       OWNER is an integer representing the UID of the owning process 
#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
#ifdef CS333_P5
  if(argc != 3){
    printf(1, "Usage: chown OWNER TARGET\n");
    exit();
  }
  
  //invoke sys_call after using atoi()
  //to translate the OWNER given
  if(chown(argv[2], atoi(argv[1])) < 0) 
    printf(1, "Failed to update %s\n", argv[2]);
#endif
  exit();
}
