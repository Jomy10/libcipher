#include <cipher.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <unistr.h>
#include "utils.h"

void test_ascii(void) {
  char* input = "ABCX z";
  char* expected = "065 066 067 088 032 122";

  char output[strlen(input) * 4 - 1];
  CU_ASSERT(ciph_ascii(input, strlen(input), output) == CIPH_OK);

  dbgout2(output, strlen(input) * 4 - 1, expected, strlen(expected));

  CU_ASSERT(u8_cmp2((uint8_t*)output, strlen(input) * 4 -1, (uint8_t*)expected, strlen(expected)) == 0);
}
