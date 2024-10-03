/**
 * io-redir.c
 *
 * Demonstrates redirecting I/O with the dup system call. Some of the printf()s
 * will be redirected to a file, while others will still display. Try modifying
 * this so that it will redirect stderr instead.
 *
 * @author mmalensek
 */

#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  // Let's redirect all output to a file: log.txt
  char *log_file = "log.txt";

  /**
   * Flags
   * - O_RDWR - open for reading and writing (could use O_WRONLY instead)
   * - O_CREATE - create file if it does not exist
   * - O_TRUNC - truncate size to 0
   */
  int open_flags = O_RDWR | O_CREATE | O_TRUNC;

  // First, open a file to send text to.
  fprintf(2, "Opening file: %s...\n", log_file);
  int fd = open(log_file, open_flags);
  if (fd == -1) {
    fprintf(2, "Could not open file\n");
    return 1;
  }

  // Do redirection. We close stdout, then dup 'fd'. dup looks for the
  // lowest unused file descriptor (stdout) to make the copy.
  // [!] This tends to be the most confusing part of redirection!
  fprintf(2, "Duplicating file descriptor...\n");
  close(1); // close stdout
  if (dup(fd) == -1) {
    fprintf(2, "dup failed\n");
    return 1;
  }

  fprintf(2, "Begin print tests!\n\n");

  printf("This is going to go to the file!\n");
  printf("Amazing stuff, right? We can just redirect everything\n");
  printf("going to stdout somewhere else with this.\n");
  // Note: the printfs up at the top of main() WILL print to stdout because we
  // didn't redirect yet.

  fprintf(2, "This is headed to stderr though... Will it print?\n");
  fprintf(2, "Your guess is as good as mine!\n");

  fprintf(1, "Printing back to stdout... Goodbye!\n");

  return 0;
}
