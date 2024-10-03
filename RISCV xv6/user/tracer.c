#include "user.h"

void tracer_error(char* err) 
{
  printf("tracer error: %s\n", err);
}

int main(int argc, char** argv)
{
  if (argc < 2) {
    tracer_error("invalid args");
    exit(1);
  }

  int pid = fork();

  if (pid < 0) {
    tracer_error("fork failed");
    exit(1);
  } else if (pid == 0) {
    strace();
    exec(argv[1], &argv[1]);
    tracer_error("exec failed");
    exit(1);
  } else {
    int status;
    wait(&status);
  }

  return 0;
}
