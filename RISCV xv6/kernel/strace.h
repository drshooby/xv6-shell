/**
* @file Macros to make printing system call traces easier.
*/

#ifndef STRACE_H
#define STRACE_H

// Macro to print an strace for a system call with no arguments
#define STRACE() \
  do { if (myproc()->strace) printf("[%d] [%s] %s()\n", \
           myproc()->pid, myproc()->name, __func__); } while (0)

// Macro to print an strace for a system call with arguments.
// Formatting is the same as with printf, e.g.:
// STRACE_ARGS("path = %s, omode = %d", path, omode);
#define STRACE_ARGS(fmt, ...) \
  do { if (myproc()->strace) printf("[%d] [%s] %s(): " fmt "\n", \
           myproc()->pid, myproc()->name, __func__, __VA_ARGS__); } while (0)

// Macro to print an strace for the return value from a system call
// Call this after the system call returns, e.g.:
// p->trapframe->a0 = syscalls[num]();
// STRACE_RETURN(num);
#define STRACE_RETURN(num) \
  do { if (myproc()->strace) printf("[%d] [syscall #%d] return: %d\n", \
           myproc()->pid, num, myproc()->trapframe->a0); } while (0)

#endif
