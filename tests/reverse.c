#include <string.h>
#include <cipher/cipher.h>
#include <CUnit/Basic.h>
#include "utils.h"

void test_reverse(void) {
  char* input = "ABC DEF. XYZ. C G";
  char* expected = "CBA FED. ZYX. C G";

  uint8_t output[strlen(input) + 1];
  CU_ASSERT(ciph_reverse_words((const uint8_t*) input, strlen(input), output) == CIPH_OK);

  dbgout(output, expected);

  CU_ASSERT(strcmp((char*) output, expected) == 0);
}
