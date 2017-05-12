#include "types.h"
#include "user.h"
#include "uproc.h"

//helper function to print leading zeros on thenths place
void
printnum(const uint num)
{
  if(num % 100 > 9)
    printf(2, "%d.%d\t", num / 100, num % 100);
  else
    printf(2, "%d.%d%d\t", num / 100, 0, num % 100);

}

int
main(int argc, char * argv[])
{
  //maximum processes possible
  uint max = 64;

  struct uproc * table = (struct uproc *) malloc(sizeof(struct uproc) * max);
  int ret = getprocs(max, table);

  if (ret < 0){
     printf(1, "Failure", ret);
  } 
  else {
    // note: not wrapping print statements in conditional compilation
    printf(2, "\nPID\tName\tUID\tGID\tPPID\tPrio\tELapsed\tCPU\tState\tSize\n");
    for(uint i = 0; i < ret; i++){  
      printf(2, "%d\t%s\t%d\t%d\t%d\t%d\t", table[i].pid, table[i].name, table[i].uid, table[i].gid, table[i].ppid, table[i].priority);
      printnum(table[i].elapsed_ticks); 
      printnum(table[i].cpu_total_ticks);
      printf(2, "%s\t%d\n", table[i].state, table[i].size);
    }
  }
  free(table);
  exit();
}
