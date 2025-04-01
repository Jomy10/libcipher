#include <stdio.h>

#ifndef eprintf
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#endif

#ifndef dbgout
#define dbgout(got, expected) \
  eprintf("\nexpected: '%s'\n", expected); \
  eprintf("got     : '%s'\n", got)
#endif

#ifndef dbgout2
#define dbgout2(got, gotlen, expected, explen) \
  write(fileno(stderr), "\nexpected: ", strlen("\nexpected: ")); \
  write(fileno(stderr), expected, explen); \
  write(fileno(stderr), "\n", 1); \
  \
  write(fileno(stderr), "got     : ", strlen("got     : ")); \
  write(fileno(stderr), got, gotlen); \
  write(fileno(stderr), "\n", 1);
#endif
