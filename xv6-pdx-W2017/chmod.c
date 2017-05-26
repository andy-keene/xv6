//User command to modify mode bits on a file in the xv6 file system
//usage: chmod <MODE> <FILE> where FILE specifies the path to the file
//       MODE is a 4 digit number split into flags for SETUID
//       USER, GROUP, and OTHER (ex. 0755)
#include "types.h"
#include "user.h"
#include "date.h"


int
main(int argc, char *argv[])
{
  int rc;
  if(argc != 3)
    printf(1, "Usage: chmod MODE TARGET\n");
  if( (rc = chmod(argv[2], atoi(argv[1]))) != 0)
    printf(1, "Failed to update %s\n", argv[2]);
  exit();
}
