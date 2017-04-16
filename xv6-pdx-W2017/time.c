#include "types.h"
#include "user.h"
//argv + x == &argv[x]

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
    int exec_ret = exec(argv[1], &argv[1]);
    //suppress complaints for null execution
    if(exec_ret < 0 && argv[1]){
      printf(2, "execution of %s failed\n", argv[1]);
      exit();
    }
  } else {
    int wait_ret = wait();

    if(wait_ret >= 0)
      print_results(argv[1], uptime() - start_time);
    else
      printf(2, "something went wrong with %s", argv[1]);
  }

  exit();
}
