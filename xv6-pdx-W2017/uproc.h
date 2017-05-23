#define STRMAX 32

struct uproc {
  uint pid;
  uint uid;
  uint gid;
  uint ppid;
  uint elapsed_ticks;
  uint cpu_total_ticks;
  uint size;
  uint priority;     //prints aren't conditionally compiled, so let's leave this in
  char state[STRMAX];
  char name[STRMAX];
};
