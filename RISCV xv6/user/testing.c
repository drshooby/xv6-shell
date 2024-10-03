#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "../kernel/fcntl.h"
#include "user.h"

void TestGetLine(char* fn) {
    
  // Case 1, have getline auto-allocate the buffer:
  //uint sz = 0;
  //char *line = NULL;
  //
  // Case 2, provide a small buffer to test resizing:
  uint sz = 10;
  char *line = malloc(sz);

  int fd = open(fn, O_RDONLY);
  while (true) {
    if (getline(&line, &sz, fd) <= 0) {
      break;
    }
    printf("Line: %s\n", line);
  }
  return;
}

void TestFgets(char* fn) {

  struct stat st;
  if (stat(fn, &st) < 0) {
    printf("unable to reach file\n");
    return;
  }

  int fd = open(fn, O_RDONLY);

  char chunk[128];

  while (fgets(chunk, 128, fd) != -1) {
    printf("Line: %s\n", chunk);
  }
}

void TestAppend(char *fn) {

  int fd = open(fn, O_APPEND | O_WRONLY);
  if (fd == -1) {
    printf("cannot open %s\n", fn);
    return;
  }
  fprintf(fd, "\n *TESTING APPEND* \n");
  close(fd);
}


int
main(int argc, char** argv)
{
  if (argc < 3) {
    printf("Probably missing option specifier: 1 = fgets, 2 = getline\n");
    exit(-1);
  }
  
  int choice = atoi(argv[1]);
  switch (choice) {
    case 1:
      printf("testing append\n");
      TestFgets(argv[2]);
      break;
    case 2:
      printf("testing getline\n");
      TestGetLine(argv[2]);
      break;
    case 3:
      printf("testing append\n");
      TestAppend(argv[2]);
      break;
    default:
      printf("Testing w/ bad option: 1 = fgets, 2 = getline\n");
      break;
  }
 
  return 0;
}


