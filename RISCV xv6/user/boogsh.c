#include "../kernel/types.h"
#include "user.h"
#include "../kernel/fcntl.h"

#define CD 1
#define EXIT 2
#define HISTORY 3
#define HISTORY_T 4
#define REPEAT 5
#define SEARCH_BY_NUM 6
#define SEARCH_BY_CMD 7
#define PROGRAM 8
#define MATRIX 9

#define PRINT_TIME 1
#define NO_PRINT_TIME 0

#define END 1
#define NO_END 0

#define BUFFER_SIZE 128

int cmd_num = 1;
int exit_status = 0;
bool scripting = false;
char wd[BUFFER_SIZE];

struct history_pair {
  char *command;
  int val;
  int time;
};

struct history {
  struct history_pair pairs[BUFFER_SIZE];
  int count; // to make sure history entries <= 100 (not always increasing)
  int total_count; // command # (always increasing)
};

void repeat_history(char *buf, struct history *h);
int choose_cmd(char *buf, struct history *h);

void
print_welcome()
{
  if (!scripting) {
    printf("Welcome to Boogshell ʕ•ᴥ•ʔ\n");
  } else {
    printf("Boogshell scripting mode active ʕ◕ᴥ◕ʔ\n");
  }
}

void
boog_error(char *msg)
{
  fprintf(2, "%s\n", msg);
  exit_status = 1;
}

void
panic(char *scary_msg)
{
  fprintf(2, "Fatal error: %s!\n", scary_msg);
  exit(1);
}

int
pork(char *msg_if_fail) // like panic-fork (or maybe "forp" makes more sense?)
{
  int pid = fork();
  if (pid == -1) 
    panic(msg_if_fail);
  return pid;
}

void
change_directory(char *buf)
{
  if (!scripting) {
    buf[strlen(buf)-1] = '\0';  // chop \n
  }
  if(chdir(buf+3) < 0) {
    fprintf(2, "cannot cd %s\n", buf+3);
    exit_status = 1;
    return;
  }
}

int check_built_in(char *buf)
{
  int choice = -1;
  
  if (buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' ') {
    return CD;
  }
  
  uint len = strlen(buf);
  if (len > 0 && buf[len - 1] == '\n') {
    buf[len - 1] = '\0';
  }
  
  if (strcmp(buf, "exit") == 0) {
    choice = EXIT;
  } else if (strcmp(buf, "history -t") == 0) {
    choice = HISTORY_T;
  } else if (strcmp(buf, "history") == 0) {
    choice = HISTORY;
  } else if (strcmp(buf, "!!") == 0) {
    choice = REPEAT;
  } else if (buf[0] == '!') {
    if (buf[1] >= '0' && buf[1] <= '9') {
      choice = SEARCH_BY_NUM;
    } else if (buf[1] >= 'a' && buf[1] <= 'z') {
      choice = SEARCH_BY_CMD;
    }
  } else if (strcmp(buf, "matrix") == 0) {
    choice = MATRIX;
  } else {
    choice = PROGRAM;
  }  
  
  return choice;
}

void 
init_history(struct history *h)
{
  h->count = 0;
  h->total_count = 1;
}

bool
valid_cmd(char *c)
{
  // check if a cmd is blank or not
  uint len = strlen(c);
  if (len == 0) return false;
  
  for (uint i = 0; i < len; i++) {
    if ((c[i] >= 'a' && c[i] <= 'z') ||
        (c[i] >= 'A' && c[i] <= 'Z') ||
        (c[i] >= '0' && c[i] <= '9') ||
        (c[i] >= '!' && c[i] <= '/')) {
        return true;
    }
  }
  return false;
}

void 
add_history(struct history *h, char *c, int time_taken)
{
  if (valid_cmd(c)) { // if the command is valid
    if (h->count == 100) { // only storing 100 items in recent history
      free(h->pairs[0].command);
      for (int i = 0; i < 99; i++) {
        h->pairs[i] = h->pairs[i+1];
      }
    } else {
      h->count++;
    }
    h->pairs[h->count - 1].command = malloc(strlen(c) + 1);
    strcpy(h->pairs[h->count - 1].command, c);
    h->pairs[h->count - 1].val = h->total_count;
    h->pairs[h->count - 1].time = time_taken;
    
    h->total_count++;
  }
}

void 
print_history(struct history *h, int print_time) 
{
  if (h->total_count == 0) {
    boog_error("No command history to display");
    return;
  }
  
  if (print_time == NO_PRINT_TIME) {
    for (int i = 0; i < h->count; i++) {
      printf("%d: %s\n", h->pairs[i].val, h->pairs[i].command);
    }
  } else {
    for (int i = 0; i < h->count; i++) {
      printf("[%d|%dms] %s\n", h->pairs[i].val, h->pairs[i].time, h->pairs[i].command);
    }
  }
}

void
repeat_history(char *buf, struct history *h)
{
  if (h->total_count == 0) {
    boog_error("No previous command to repeat");
    return;
  }
  
  char *prev_cmd = h->pairs[h->total_count - 1].command;
  choose_cmd(prev_cmd, h);
  /*
  * strcpy prev_cmd into buf so that when history prints,
  * the repeat command will execute and be shown in history 
  * as the executed command and not '!!'. The same strategy is used
  * in the two history-searching functions below.
  */
  strcpy(buf, prev_cmd);
}

void
search_by_num(char *buf, struct history *h)
{
  char arg_nums[BUFFER_SIZE];
  int j = 0;
  uint i = 1;
  while (i < strlen(buf) && j < BUFFER_SIZE && buf[i] >= '0' && buf[i] <= '9') {
    arg_nums[j] = buf[i];
    i++;
    j++;
  }

  int val = atoi(arg_nums);
  char *found_cmd = NULL;
  for (int i = 0; i < h->count; i++) {
    if (h->pairs[i].val == val) {
      found_cmd = h->pairs[i].command;
    }
  }
  
  if (!found_cmd) {
    fprintf(2, "%d: ", val);
    boog_error("Command number not found in recent history");
    return;
  }

  choose_cmd(found_cmd, h);
  strcpy(buf, found_cmd);
}

void 
search_by_cmd(char *buf, struct history *h) 
{
  char arg_cmd[BUFFER_SIZE];
  int j = 0;
  uint i = 1;
  
  while (i < strlen(buf) && j < BUFFER_SIZE && (buf[i] >= 'a' && buf[i] <= 'z')) {
    arg_cmd[j] = buf[i];
    i++;
    j++;
  }

  arg_cmd[j] = '\0';
  char *found_cmd = NULL;
  for (int i = 0; i < h->count; i++) {
    char *match = malloc(strlen(h->pairs[i].command) + 1);
    strcpy(match, h->pairs[i].command); // get command to prepare to match

    char *pos = strchr(match, ' ');
    if (!pos) {
      pos = strchr(match, '\n');
    }

    if (pos) {
      *pos = '\0';
    }

    int matching_chars = 0;
    int n = strlen(arg_cmd);

    for (int ci = 0; ci < n; ci++) {
      if (arg_cmd[ci] == match[ci]) {
        matching_chars++;
      }
      if (matching_chars == n) {
        found_cmd = h->pairs[i].command;
        break;
      }
    }

    free(match);
  }

  if (!found_cmd) {
    fprintf(2, "%s: ", arg_cmd);
    boog_error("Command match not found in recent history");
    return;
  }
  
  choose_cmd(found_cmd, h);
  strcpy(buf, found_cmd);
}

void
search_history(char* buf, struct history *h, int choice)
{
  switch (choice) {
    case SEARCH_BY_NUM:
      search_by_num(buf, h);
      break;
    case SEARCH_BY_CMD:
      search_by_cmd(buf, h);
      break;
  }
}

struct command {
  char **tokens;
  bool stdout_pipe;
  bool append;
  char *stdout_file;
  char *stdin_file;
  int token_count; // used for debugging
};

int
execute_pipeline(struct command *cmd)
{
  if (!cmd->stdout_pipe) {
    if (cmd->stdout_file) {
      int fd;
      if (cmd->append) {
        fd = open(cmd->stdout_file, O_RDWR | O_CREATE | O_APPEND);
      } else {
        fd = open(cmd->stdout_file, O_RDWR | O_CREATE);
      }
      if (fd == -1) {
        fprintf(2, "stdout file: %s\n", cmd->stdout_file);
        panic("unable to open^^^");
      }
      if (close(1) == -1) {
        panic("failed to close stdout");
      }
      if (dup(fd) == -1) {
        panic("failed to dup fd for stdout");
      }
      if (close(fd) == -1) {
        panic("failed to close fd after dup");
      }
    }
    if (cmd->stdin_file) {
      int fd = open(cmd->stdin_file, O_RDONLY);
      if (fd == -1) {
        fprintf(2, "stdin file: %s\n", cmd->stdin_file);
        panic("unable to open^^^");
      }
      if (close(0) == -1) {
        panic("failed to close stdin");
      }
      if (dup(fd) == -1) {
        panic("failed to dup fd for stdin");
      }
      if (close(fd) == -1) {
        panic("failed to close fd after dup");
      }
    }
    exec(cmd->tokens[0], cmd->tokens);
    fprintf(2, "failed to exec: %s with token count: %d\n", cmd->tokens[0], cmd->token_count);
    return 1;
  }
  
  int fd[2];
  pipe(fd);
  int pid = pork("failed to fork child process in execute_pipeline");
  if (pid > 0) { // parent
    if (close(fd[1]) == -1) {
      panic("failed to close out-end of pipe in parent");
    }
    if (close(0) == -1) {
      panic("failed to close stdin in parent");
    }
    if (dup(fd[0]) == -1) {
      panic("failed to dup in-end of pipe in parent");
    }
    execute_pipeline(cmd + 1);
    wait(0);
  } else if (pid == 0) { // child
    if (close(fd[0]) == -1) {
      panic("failed to close in-end of pipe in child");
    }
    if (close(1) == -1) {
      panic("failed to close stdout in child");
    }
    if (dup(fd[1]) == -1) {
      panic("failed to dup out-end of pipe in child");
    }
    exec(cmd->tokens[0], cmd->tokens);
    fprintf(2, "failed to exec: %s in pipeline with token count: %d\n", cmd->tokens[0], cmd->token_count);
    exit(1);
  }
  return 0;
}

void
parse_and_run_cmd(char *s)
{
  int cap = 2; // size cap of cmds
  int sz = 0; // current size of cmds
  struct command *cmds = malloc(sizeof(struct command) * cap);
  if (!cmds) panic("failed to create cmds array with malloc");

  char *tok; // whole token chunk
  char *part; // token pieces
  bool found_in = false;
  bool found_out = false;
    
  while ((tok = next_token(&s, "|"))) {
     if (sz == cap - 1) {
       cap *= 2;
       struct command *tmp = malloc(cap * sizeof(struct command));
       if (!tmp) panic("failed to create tmp array with malloc");
       memcpy(tmp, cmds, sizeof(struct command) * sz);
       free(cmds);
       cmds = tmp;
     }

    cmds[sz].tokens = malloc(sizeof(char *) * (strlen(tok))); // tokens array
    // so stdin_file or stdout_file don't get overwritten accidentally
    found_in = false;
    found_out = false;
    
    int curr_tok_idx = 0;
    char *file = NULL;

    cmds[sz].stdin_file = NULL;
    cmds[sz].stdout_file = NULL;
    cmds[sz].stdout_pipe = true;
    cmds[sz].append = false;
    
    while ((part = next_token(&tok, " "))) {
      if (strcmp(part, ">") == 0) {
        file = next_token(&tok, " ");
        if (!file) panic("no file present after \'>\'");
        if (!found_in) {
          cmds[sz].stdin_file = NULL;
        }
        cmds[sz].stdout_pipe = false;
        cmds[sz].stdout_file = file;
        cmds[sz].stdout_pipe = false;
        found_out = true;
        continue;
      } else if (strcmp(part, "<") == 0) {
        file = next_token(&tok, " ");
        if (!file) panic("no file present after \'<\'");
        if (!found_out) {
          cmds[sz].stdout_file = NULL;
        }
        cmds[sz].stdout_pipe = false;
        cmds[sz].stdin_file = file;
        cmds[sz].stdout_pipe = false;
        found_in = true;
        continue;
      } else if (strcmp(part, ">>") == 0) {
        file = next_token(&tok, " ");
        if (!file) panic("no file present after \'>>\'");
        if (!found_in) {
          cmds[sz].stdin_file = NULL;
        }
        cmds[sz].stdout_pipe = false;
        cmds[sz].stdout_file = file;
        cmds[sz].stdout_pipe = false;
        cmds[sz].append = true;
        continue;
      }
      cmds[sz].tokens[curr_tok_idx++] = part;
    }
    cmds[sz].token_count = curr_tok_idx;
    sz++;
  }

  cmds[sz - 1].stdout_pipe = false; // end pipeline
  int status = execute_pipeline(cmds); // run the pipeline
  if (status == 1) {
    exit(1);
  }
  exit(0);
}

void
run_program(char* buf, int background)
{
  int pid = pork("failed to fork child process in run_program");
  
  if(pid == 0) {
    parse_and_run_cmd(buf);
  } else if(pid > 0) {
    if (background) {
      printf("Running job in background, pid = %d\n", pid);
    } else {
      int status;
      while (wait(&status) != pid) { /* do nothing */}
      exit_status = status;
    }
  }
}

void
enter_the_matrix()
{
  printf("\x1b[H\x1b[J"); // clear screen before awesome
  printf("hope you're ready...\n");
  sleep(10);
  printf("♬ cuz we're all in the matrix ♬\n");
  sleep(5);

  // 1 = first zigzag, 2 = second zigzag
  int f1 = 0; // starting digit
  int f2 = 1;
  int d1 = 1; // direction
  int d2 = -1;
  int s1 = 0; // # leading spaces 
  int s2 = 20;

  // max indent for both 
  int max_indent = 20;

  int counter = 0;
  int max_count = 100;

  while (1) {
    // check if user is tired of this awesome display (why would they be?)
    if (counter == max_count) {
      counter = 0;
      char buf[BUFFER_SIZE];
      printf("continue? y for yes, n for no\n"); // typing "y" doesn't actually do anything special, illusion of choice...
      fgets(buf, BUFFER_SIZE, 0);
      if (strcmp(buf, "n\n") == 0 || strcmp(buf, "no\n") == 0) {
        break;
      }
    }
    // first pattern
    for (int s = 0; s < s1; s++) {
      printf(" ");
    }
    for (int i = 0; i < 10; i++) {
      printf("%d", (i + f1) % 2);
    }
    printf("\n");

    // second pattern
    for (int s = 0; s < s2; s++) {
      printf(" ");
    }
    for (int i = 0; i < 10; i++) {
      printf("%d", (i + f2) % 2);
    }
    printf("\n");

    // spaces for the next frame
    s1 += d1;
    s2 += d2;

    // direction switching
    if (s1 == max_indent || s1 == 0) {
      d1 *= -1;
    }
    if (s2 == max_indent || s2 == 0) {
      d2 *= -1;
    }

    // switch digits. 0s become 1s, 1s become 0s
    f1 = 1 - f1;
    f2 = 1 - f2;
    counter++;
    // this sleep isn't here because the program depends on it, it's just painful to look at if the zigzags are really fast
    sleep(1);
  }
  return;
}

int
choose_cmd(char *buf, struct history *h)
{
  int choice = check_built_in(buf);

  int run_in_background = 0;
  int len = strlen(buf);
  if (len == 0) {
    return -1;
  }
  if (len > 0 && buf[len - 1] == '&') {
    run_in_background = 1;
    buf[len - 1] = '\0';
  }
  
  int t_start = unixtime();
  switch (choice) {
    case CD:
      change_directory(buf);
      break;
    case EXIT:
      exit(0);
    case HISTORY:
      print_history(h, NO_PRINT_TIME);
      break;
    case HISTORY_T:
      print_history(h, PRINT_TIME);
      break;
    case REPEAT:
      repeat_history(buf, h);
      break;
    case SEARCH_BY_NUM:
    case SEARCH_BY_CMD:
      search_history(buf, h, choice);
      break;
    case PROGRAM:
      run_program(buf, run_in_background);
      break;
    case MATRIX:
      enter_the_matrix();
      break;
  }
  int t_end = unixtime();
  return (t_end - t_start) / 1000000;
}

void
prompt()
{
  getcwd(wd, BUFFER_SIZE);
  fprintf(0, "[%d]-[%d]-[%s] ඞ-> ", exit_status, cmd_num, wd);
}


void
getcmd(char *buf, int nbuf)
{
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
}

void
check_comment(char *buf)
{
  // allow for comments, if comment then remove from buffer
  char *cp = strchr(buf, '#');
  if (cp)
    *cp = '\0';
}

int
main(int argc, char **argv)
{
  // variables
  char buf[BUFFER_SIZE];
  struct history h;
  init_history(&h);

  // scripting variables + global scripting bool
  int fd = 0;
  
  char *line = NULL;
  uint max = 0;
  
  if (argc > 1 && argv[1] != NULL) {
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
      boog_error("scripting file doesn't exist");
      exit(1);
    }
    scripting = true;
  }

  print_welcome();

  bool read_first_script_line = false;
  
  while(true) {
    if (!scripting) {
      prompt();
      getcmd(buf, sizeof(buf));
      check_comment(buf);
      exit_status = 0; // reset exit status
      cmd_num++;
      int time_taken = choose_cmd(buf, &h);
      if (time_taken == -1) continue;
      if (exit_status == 0) {
        add_history(&h, buf, time_taken);
      }
      continue;
    }

    if (getline(&line, &max, fd) <= 0) {
      break;
    }

    uint line_len = strlen(line);
    line[line_len - 1] = '\0'; // remove newline

    // skip the header, it gets read in exec()
    if (!read_first_script_line) {
      read_first_script_line = true;
      continue; 
    }
    strcpy(buf, line);
    check_comment(buf);
    exit_status =  0;
    int time_taken = choose_cmd(buf, &h);
    if (time_taken == -1) continue;
    if (exit_status == 0) {
      add_history(&h, buf, time_taken);
    }
  }

  return 0;
}
