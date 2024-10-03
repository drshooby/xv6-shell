#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "strace.h"
#include "syscall.h"


uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  STRACE_ARGS("n = %d", n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  STRACE();
  uint64 res = myproc()->pid;
  STRACE_RETURN(SYS_getpid);
  return res;
}

uint64
sys_fork(void)
{
  STRACE();
  uint64 res = fork();
  STRACE_RETURN(SYS_fork);
  return res;
}

uint64
sys_wait(void)
{
  STRACE();
  uint64 p;
  argaddr(0, &p);
  uint64 res = wait(p);
  STRACE_RETURN(SYS_wait);
  return res;
}

uint64
sys_sbrk(void)
{
  STRACE();
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0) {
    STRACE_RETURN(SYS_sbrk);
    return -1;
  }
  STRACE_RETURN(SYS_sbrk);
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  STRACE_ARGS("n = %d", n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      STRACE_RETURN(SYS_sleep);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  STRACE_RETURN(SYS_sleep);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  STRACE_ARGS("pid = %d", pid);
  uint64 res = kill(pid);
  STRACE_RETURN(SYS_kill);
  return res;
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  STRACE();
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  STRACE_RETURN(SYS_uptime);
  return xticks;
}

/* Lab03 syscalls */

uint64
sys_shutdown(void)
{
  STRACE();
  volatile uint32 *test_dev = (uint32 *) VIRT_TEST;
  /* Hex memory location for shutdown */
  *test_dev = 0x5555;
  STRACE_RETURN(SYS_shutdown);
  return 0;
}

uint64
sys_reboot(void)
{
  STRACE();
  volatile uint32 *test_dev = (uint32 *) VIRT_TEST;
    /* Hex memory location for reboot */
  *test_dev = 0x7777;
  STRACE_RETURN(SYS_reboot);
  return 0;
}

uint64
sys_unixtime(void)
{
  STRACE();
  volatile uint64 *time_rtc = (uint64 *) GOLDFISH_RTC;
  STRACE_RETURN(SYS_unixtime);
  return *time_rtc;
}

uint64
sys_strace(void)
{
  // not adding strace to the strace syscall in case there is some infinite loop
  // from being too meta...
  myproc()->strace = 1;
  return 0;
}

uint64
sys_wait2(void)
{
  STRACE();
  uint64 p;
  uint64 count;
  argaddr(0, &p);
  argaddr(1, &count);
  uint64 res = wait2(p, count); // TODO! check this
  STRACE_RETURN(SYS_wait2);
  return res;
}

uint64
sys_benchmark_reset(void)
{
  myproc()->counter = 0;
  return 0;
}

