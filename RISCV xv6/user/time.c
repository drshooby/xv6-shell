#include "../kernel/types.h"
#include "user.h"

int main(void) {

  uint64 seconds;
  uint64 minutes;
  uint64 hours;
  uint32 days;
  uint32 years;

  seconds = unixtime() / 1000000000;
  printf("***********\n*UNIX TIME*\n***********\n");
  printf("Time in seconds: %d\n", seconds);
  minutes = seconds / 60;
  printf("Time in minutes: %d\n", minutes);
  hours = minutes / 60;
  printf("Time in hours: %d\n", hours);
  days = hours / 24;
  printf("Time in days: %d\n", days);
  years = days / 365;
  printf("Guessing the current year????: %d\n", years + 1970);

  return 0;
}
