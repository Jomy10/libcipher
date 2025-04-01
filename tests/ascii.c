#include <string.h>
#include <CUnit/Basic.h>
#include <cipher/cipher.h>
#include "utils.h"

void test_ascii(void) {
  char* input = "ABCX z";
  char* expected = "065 066 067 088 032 122";

  char output[strlen(input) * 4];
  CU_ASSERT(ciph_ascii(input, strlen(input), output) == CIPH_OK);

  dbgout(output, expected);

  CU_ASSERT(strcmp(output, expected) == 0);
}
