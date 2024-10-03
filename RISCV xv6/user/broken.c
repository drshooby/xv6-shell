/**
 * @file broken.c
 * @author mmalensek
 *
 * This program contains a series of buggy, broken, or strange C functions for
 * you to ponder. Your job is to analyze each function, fix whatever bugs the
 * functions might have, and then explain what went wrong. Sometimes the
 * compiler will give you a hint.
 *
 *  ____________
 * < Good luck! >
 *  ------------
 *      \   ^__^
 *       \  (oo)\_______
 *          (__)\       )\/\
 *              ||----w |
 *              ||     ||
 */

#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"

static int func_counter = 1;
#define FUNC_START() printf("\n\n%d.) %s\n", func_counter++, __func__);


/**
 * This awesome code example was taken from the book 'Mastering C Pointers,'
 * one of the famously bad resources on learning C. It was trying to demonstrate
 * how to print 'BNGULAR'... with pointers...? Maybe?
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 *      Hint: where are string literals stored in memory?
 *      Hint: what is '66' in this example? Can we do better?

 Problem was that the program was trying to mutate a string literal, which is illegal C.
 You can also make the code cleaner by using just the character B rather than its ASCII to avoid magic numbers
 */
void
angular(void)
{
  FUNC_START();

  char a[] = "ANGULAR";
  a[0] = 'B';
  printf("%s\n", a);
}

/**
 * This function is the next step after 'Hello world' -- it takes user input and
 * prints it back out! (Wow).
 *
 * But, unfortunately, it doesn't work.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code: 0 is not valid memory
 *
 *   (answer)
 */
void
greeter(void)
{
  FUNC_START();

  char name[128];

  printf("Please enter your name: ");
  gets(name, 128);

  // Remove newline character
  char *p = name;
  for ( ; *p != '\n' && *p != 0; p++) { }
  *p = '\0';

  printf("Hello, %s!\n", name);
}

/**
 * This 'useful' function prints out an array of integers with their indexes, or
 * at least tries to. It even has a special feature where it adds '12' to the
 * array.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code: Ugly code, made it cleaner, also fixed loop limit
 *
 *   (answer)
 */
void
displayer(void)
{
  FUNC_START();
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Waggressive-loop-optimizations"

  int stuff[100] = { 0 };

  /* Can you guess what the following does without running the program? -  Changes value at index 15 */
  /* Rewrite it so it's easier to read. */
  //14[stuff + 1] = 12;
  stuff[15] = 12;

  for (int i = 0; i < 100; ++i) {
    printf("%d: %d\n", i, stuff[i]);
  }

  #pragma GCC diagnostic pop
}

/**
 * Adds up the contents of an array and prints the total. Unfortunately the
 * total is wrong! See main() for the arguments that were passed in.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code: 
 - incorrect array length calculation was terminating loop early
 *
 *   (answer)
 */
void
adder(int arr[], int len)
{
  FUNC_START();

  int total = 0;
  for (int i = 0; i < len; ++i) {
    total += arr[i];
  }

  printf("Total is: %d\n", total);
}

/**
 * This function is supposed to be somewhat like strcat, but it doesn't work.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 - Data was getting freed at the end of the function lifetime which returned an empty pointer
 *
 *   (answer)
 */
char *
suffixer(char *a, char *b, char *res)
{
  FUNC_START();
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wdangling-pointer"

  char buf[128] = { 0 };
  strcpy(buf, a);
  strcpy(buf + strlen(a), b);
  char *buf_start = buf;
  strcpy(res, buf_start);
  return res;

  #pragma GCC diagnostic pop
}

/**
 * This is an excerpt of Elon Musk's Full Self Driving code. Unfortunately, it
 * keeps telling people to take the wrong turn. Figure out how to fix it, and
 * explain what's going on so Elon can get back to work on leaving Earth for
 * good.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 - if(strcmp()) evaluates to false for true statements and true for false statements
 - also there wasn't enough space for a null terminator in piedmont
 *
 *   (answer)
 */
void
driver(void)
{
  FUNC_START();

  char street1[8] = { "fulton" };
  char street2[8] = { "gomery" };
  char street3[9] = { "piedmont" };
  char street4[8] = { "baker" };
  char street5[8] = { "haight" };

  if (strcmp(street1, street2) == 0) {
    char *new_name = "saint agnew ";
    memcpy(street4, new_name, strlen(new_name));
    // usually use strcpy
  }

  printf("Welcome to TeslaOS 0.1!\n");
  printf("Calculating route...\n");
  printf("Turn left at the intersection of %s and %s.\n", street5, street3);
}

/**
 * This function tokenizes a string by space, sort of like a basic strtok or
 * strsep. It has two subtle memory bugs for you to find.
 *
 * (1) Fix the problem.
 * (2) Explain what's wrong with this code:
 - not adding space in malloc for a null terminator
 - not freeing the original allocated memory in "line", just from the update pointer
 *
 *   (answer)
 */
void
tokenizer(void)
{
  FUNC_START();

  char *str = "Hope was a letter I never could send";
  char *line = malloc(strlen(str) + 1);
  if (!line) return; 

  strcpy(line, str);

  char *line_start = line;
  
  char *c = line;

  while (*c != '\0') {

    for ( ; *c != ' ' && *c != '\0'; c++) {
      // find the next space (or end of string)
    }

    *c = '\0';
    printf("%s\n", line);

    line = c + 1;
    c = line;
  }

  free(line_start);
}

/**
* This function should print one thing you like about C, and one thing you
* dislike about it. Assuming you don't mess up, there will be no bugs to find
* here!
*/
void
finisher(void)
{
  FUNC_START();

  printf("Manually handling memory makes me feel smart.\n");
  printf("C should not have strings allowed.\n");
}

int
main(void)
{
  printf("Starting up!");

  angular();

  greeter();

  displayer();

  int nums[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512 };
  int len = sizeof(nums) / sizeof(nums[0]);
  adder(nums, len);

  char result[128]; 
  suffixer("kayak", "ing", result);
  printf("Suffixed: %s\n", result);

  driver();

  tokenizer();

  finisher();

  return 0;
}
