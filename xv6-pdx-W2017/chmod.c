//User command to modify mode bits on a file in the xv6 file system
//usage: chmod <MODE> <FILE> where FILE specifies the path to the file
//       MODE is a 4 digit number split into flags for SETUID
//       USER, GROUP, and OTHER (ex. 0755)
#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
//conditionaly compile code for P5
#ifdef CS333_P5
  if(argc != 3){
    printf(1, "Usage: chmod MODE TARGET\n");
    exit();
  }
  if(strlen(argv[1]) != 4){
    printf(1, "MODE must be 4 digits\n");
  }
 
  //Per convo with Enis and Morrissey, atoo() offers
  //strict enough enforcement for octal conversion
  if(chmod(argv[2], atoo(argv[1])) < 0) 
    printf(1, "Failed to update %s\n", argv[2]);
#endif
  exit();
}

