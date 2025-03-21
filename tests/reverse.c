#include <string.h>
#include <cipher/cipher.h>
#include <CUnit/Basic.h>
#include "utils.h"

void test_reverse(void) {
  char* input = "ABC DEF. XYZ. C G";
  char* expected = "CBA FED. ZYX. C G";

  eprintf("\nexpected: %s\n", expected);

  char output[strlen(input) + 1];
  CU_ASSERT(ciph_reverse_words((const uint8_t*) input, strlen(input), output) == CIPH_OK);
  eprintf("got: %s\n", output);

  CU_ASSERT(strcmp(output, expected) == 0);
}
