#include "../kernel/types.h"
#define NULL ((void *) 0)

#define bool _Bool
#define true 1
#define false 0

#define SEEK_SET        0
#define STDIN_FILENO    0
#define SEEK_CUR        1
#define STDOUT_FILENO   1
#define SEEK_END        2
#define STDERR_FILENO   2

struct command;

struct stat;

// system calls
int fork(void);
int exit(int) __attribute__((noreturn));
int wait(int*);
int wait2(int*, int*);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(const char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int shutdown(void);
int reboot(void);
uint64 unixtime(void);
int strace(void);
int benchmark_reset(void);
int getcwd(char *, int);

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void fprintf(int, const char*, ...);
void printf(const char*, ...);
char* gets(char*, int max);
int fgets(char*, int max, int fd);
int getline(char**, uint*, int);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void malloc_print();
void malloc_setfsm(int);
void malloc_name(char *ap, char *name);
void free(void*);
int atoi(const char*);
int memcmp(const void *, const void *, uint);
void *memcpy(void *, const void *, uint);
uint strspn(const char *str, const char *chars);
uint strcspn(const char *str, const char *chars);
char* next_token(char **str_ptr, const char *delim);
