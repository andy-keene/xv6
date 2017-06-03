# xv6
#### Expanding on the UNIX based xv6 OS for CS 333

Each branch corresponds to the following project description:

**Project 1**: For this assignment I learned about the flow of control for system calls in xv6; how to add a new system call; how to access specific information for each active process; and how to use conditional compilation to enable and disable kernel features.

**Project 2**: For this assignment I learned about locks used for concurrent data structures in the kernel; expanded the process structure to contain ownership information and CPU use-time; implemented new user commands to see current process statuses, and track the run-time of user level commands; and how to use wrappers for system calls that effect files or access the proc table data structure .


**Project 3**: For this assignment I learned about improving process management by implementing the xv6 state tran- sition diagram using lists (per state) to increase run-time efficiency of state transitions while maintaining the invariant that each process may be on one and only one list at a time. I also learned about adding console ctrl commands, using locks to support atomicity, and about using conditional compilation to both produce two functioning versions of the same operating system and to compile code that demonstrates the list invariant.

**Project 4**: For this assignment I learned about improved process scheduling by implementing a Multilevel Feedback Queue (MLFQ) as the scheduling algorithm; this included hand-tuning constants which dictate the pro- motion/demotion intervals of individual processes and making consequential OS design decisions.

**Project 5**: For this assignment I learned about the file system abstraction in xv6; its underlying meta information; transactional operations in a log based file system; and implemented a new simple file protection feature. This feature is conditionally compilable in its entirety and thus can be turned on or off.


The report for project `x` may be found under `Reports/Project x/keene_projectx.pdf`. Note that [QEUMU](http://www.qemu.org) is required to run xv6 and that certain features or projects may depend on conditional compilation flags which require edits to the Makefile to enable. 

*\*To see the xv6 kernel modifications required by CS 333 during Spring 2017, please see the open PR between `project5` and `original`.*
