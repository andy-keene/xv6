typedef unsigned int uint;
#define STRMAX 32

struct uproc {
  uint pid;
  uint uid;
  uint gid;
  uint ppid;
  uint elapsed_ticks;
  uint cpu_total_ticks;
  uint size;
  char state[STRMAX];
  char name[STRMAX];
};
