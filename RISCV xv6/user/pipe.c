/**
 * pipe.c
 *
 * Demonstrates using pipes for IPC.
 *
 * @author mmalensek
 */

#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void) {
  // Creates a pipe.
  int fd[2];
  if (pipe(fd) == -1) {
    fprintf(2, "Could not create pipe.\n");
    return 1;
  }

  int pid = fork();
  if (pid == -1) {
    fprintf(2, "Fork failed!\n");
    return 1;
  } else if (pid == 0) {
    // Child
    close(fd[1]); // close write end of pipe
    close(0); // stdin
    dup(fd[0]); // stdin now comes from the pipe

    char buf[100];
    while (true) {
      fgets(buf, 100, 0); // Will block here and wait for data from pipe

      // Print out what we've received.
      printf("Receiving <- %s", buf);
    }
  } else {
    // Parent. Will write data to the pipe.
    close(fd[0]); // close the read end of the pipe
    close(1); // stdout
    dup(fd[1]); // stdout now goes to the pipe
    int counter = 0;
    while(true) {
      fprintf(2, "Sending -> %d\n", counter);
      printf("%d\n", counter++);

      // Let's make this slow so we can actually see what's happening
      sleep(10);
    }
  }

  close(fd[0]);
  close(fd[1]);

  return 0;
}
