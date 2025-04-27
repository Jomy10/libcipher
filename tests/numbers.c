#include <string.h>
#include <CUnit/Basic.h>
#include <cipher.h>
#include "unistr.h"
#include "utils.h"

void test_numbers(void) {
  uint8_t* input = (uint8_t*)"ABCX";
  uint8_t* expected = (uint8_t*)"1 2 3 24";

  uint8_t output[strlen((char*)expected)];
  size_t output_len;
  CU_ASSERT(ciph_numbers(
    input, strlen((char*)input),
    output, strlen((char*)expected),
    false,
    NULL, NULL, &output_len
  ) == CIPH_OK);

  dbgout2(output, strlen((char*)expected), expected, strlen((char*)expected));

  // printf("\n%d %d %d\n", output_len, strlen((char*)expected), output_len == strlen((char*)expected));
  CU_ASSERT(output_len == strlen((char*)expected));
  CU_ASSERT(u8_cmp2(output, strlen((char*)expected), expected, strlen((char*)expected)) == 0);
}
