#include "user.h"

void benchmark_error(char* err) 
{
  printf("benchmark error: %s\n", err);
}

void print_benchmark_results(uint64 time_elapsed, int count)
{
  printf("------------------\n");
  printf("Benchmark Complete\n");
  printf("Time elapsed: %d ms\n", time_elapsed);
  printf("System calls: %d\n", count);
}

int main(int argc, char** argv)
{
  if (argc < 2) {
    benchmark_error("invalid args");
    exit(1);
  }

  benchmark_reset(); // reset proc counter to 0

  int pid = fork();

  uint64 start = unixtime(), end = 0;

  if (pid < 0) {
    benchmark_error("fork failed");
    exit(1);
  } else if (pid == 0) {
    exec(argv[1], &argv[1]);
    benchmark_error("exec failed");
    exit(1);
  } else {
    int status, syscall_count;
    wait2(&status, &syscall_count);
    end = unixtime();
    print_benchmark_results((end - start) / 1000000, syscall_count);
  }

  return 0;
}
