#include <stdio.h>

#ifndef eprintf
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#endif

#ifndef dbgout
#define dbgout(got, expected) \
  eprintf("\nexpected: %s\n", expected); \
  eprintf("got     : %s\n", got)
#endif
