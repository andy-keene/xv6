//User command to modify gid of a file in the xv6 file system
//usage: chgrp <GROUP> <FILE> where FILE specifies the path to the file
//       GROUP is an integer representing the GID (group process ID)
//       GROUP value bounds is checked in chgrp()
#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  if(argc != 3){
    printf(1, "Usage: chgrp GROUP TARGET\n");
    exit();
  }
  
  //invoke sys_call after using atoi()
  //to translate the GROUP given
  if(chgrp(argv[2], atoi(argv[1])) < 0) 
    printf(1, "Failed to update %s\n", argv[2]);

  exit();
}
