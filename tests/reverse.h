#include <cipher.h>
#include <unistd.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <unistr.h>
#include "utils.h"

void test_reverse(void) {
  uint8_t* input = (uint8_t*)"ABC DEF. XYZ. C G";
  uint8_t* expected = (uint8_t*)"CBA FED. ZYX. C G";

  uint8_t output[strlen((char*)input)];
  CU_ASSERT(ciph_reverse_words((const uint8_t*) input, strlen((char*)input), output) == CIPH_OK);

  dbgout2(output, strlen((char*)input), expected, strlen((char*)expected));

  CU_ASSERT(u8_cmp2(output, strlen((char*)input), expected, strlen((char*)expected)) == 0);
}

void test_reverse_grapheme(void) {
  const uint8_t* input = (uint8_t*)"A😍Bs̀";
  const uint8_t* expected = (uint8_t*)"s̀B😍A";

  uint8_t output[strlen((char*)input)];
  CU_ASSERT(ciph_reverse_words(input, strlen((char*)input), output) == CIPH_OK);

  dbgout2(output, strlen((char*)input), expected, strlen((char*)expected));

  CU_ASSERT(u8_cmp2(output, strlen((char*)input), expected, strlen((char*)expected)) == 0);
}

CU_SuiteInfo suite_reverse(void) {
  static CU_TestInfo tests[] = {
    { "reverse", test_reverse },
    { "reverse grapheme", test_reverse_grapheme },
  };
  return (CU_SuiteInfo){ "reverse", NULL, NULL, NULL, NULL, tests };
}
