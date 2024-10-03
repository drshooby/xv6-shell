#include "../kernel/fcntl.h"
#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"

#define READ_SZ 4096
#define LINE_SZ 128

/*
 - Citation: made in collaboration with ClaudeAI
 - It's basically a compact version of getline without the fgets call
*/

char* better_fgets(int fd) {
    static char buffer[READ_SZ];
    static int buffer_pos = 0;
    static int buffer_size = 0;
    
    char* line = malloc(LINE_SZ);
    int line_size = LINE_SZ;
    int line_pos = 0;
    
    while (1) {
        if (buffer_pos >= buffer_size) {
            buffer_size = read(fd, buffer, READ_SZ);
            buffer_pos = 0;

            if (buffer_size == 0) {
              if (line_pos == 0) {
                free(line);
                return NULL;
              }
              break;
            } else if (buffer_size < 0) {
              free(line);
              return NULL;
            }
            
            if (buffer_size <= 0) break;
        }
        
        char c = buffer[buffer_pos++];
        
        if (c == '\n') break;
        
        if (line_pos + 1 >= line_size) {
            line_size *= 2;
            char* tmp = malloc(line_size);
            if (!tmp) return NULL;
            memcpy(tmp, line, line_pos);
            free(line);
            line = tmp;
        }
        
        line[line_pos++] = c;
    }
    line[line_pos] = '\0';
    return line;
}

int
main(int argc, char *argv[])
{
  if (argc <= 1) {
    fprintf(2, "Usage: %s filename\n", argv[0]);
    return 1;
  }

  int fd = open(argv[1], O_RDONLY);
  char *line;
  int line_count = 0;
  while ((line = better_fgets(fd))) {
    printf("Line %d: %s\n", line_count++, line);
  }

  return 0;
}
