#include "types.h"
#include "user.h"

void
print_results(const char * cmd, const uint time)
{
  if(time % 100 > 9)
    printf(2, "%s ran in %d.%d seconds\n", cmd, time /100, time % 100);
  else
    printf(2, "%s ran in %d.0%d seconds\n", cmd, time /100, time % 100);
}

int
main(int argc, char * argv[])
{
  uint start_time = uptime();
  int pid = fork();

  if(pid == 0){ //child
    //argv + x == &argv[x]
    exec(argv[1], &argv[1]);
  } else {
    int ret = wait();
    if(ret >= 0)
      print_results(argv[1], uptime() - start_time);
  }

  exit();
}
