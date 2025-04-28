#include <cipher.h>
#include <stdio.h>
#include <unistd.h>

extern FILE* _stderr;

#ifndef eprintf
#define eprintf(...) fprintf(_stderr, __VA_ARGS__)
#endif

#ifndef dbgout
#define dbgout(got, expected) \
  eprintf("\nexpected: '%s'\n", expected); \
  eprintf("got     : '%s'\n", got)
#endif

#ifndef dbgout2
#define dbgout2(got, gotlen, expected, explen) \
  write(fileno(_stderr), "\nexpected: '", strlen("\nexpected: '")); \
  write(fileno(_stderr), expected, explen); \
  write(fileno(_stderr), "'\n", 2); \
  \
  write(fileno(_stderr), "got     : '", strlen("got     : '")); \
  write(fileno(_stderr), got, gotlen); \
  write(fileno(_stderr), "'\n", 2);
#endif

#ifndef _CIPHER_TEST_UTILS_H
#define _CIPHER_TEST_UTILS_H

#endif
