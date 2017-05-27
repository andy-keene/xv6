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
  int mode = 0;
  if(argc != 3){
    printf(1, "Usage: chmod MODE TARGET\n");
    exit();
  }
  if(strlen(argv[1]) != 4){
    printf(1, "MODE must be 4 digits\n");
  }
 
  //Note: strict enforcement on given mode, does not allow signed representations
  //adapted from atoo for octal
  while('0' <= *argv[1] && *argv[1] <= '9')
    mode = 8*mode + *argv[1]++ - '0';

  if(chmod(argv[2], mode) < 0) 
    printf(1, "Failed to update %s\n", argv[2]);

  exit();
}

/* 
  Garbage bin

  //extract invidual permissions, validate, then
  //use bit shifting and or-ing to set flags as a uint
  setuid = (mode / 1000) % 10;
  user = (mode / 100) % 10;
  group = (mode / 10) % 10;
  other = mode % 10;
  if(setuid > 1 || user > 7 || group > 7 || other > 7)
    return -1;

  mode_asInt = (setuid << 9) | (user << 6) | (group << 3) | other;

  OR

  while(*arv[1])
   if(! *argv[1] <= 9 ...)
     not a number!
     exit();
   else
     update mode

*/
