#include <unistd.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <cipher.h>
#include <unistr.h>
#include <uninorm.h>
#include "utils.h"

void test_caesar(void) {
  const uint8_t* input = (const uint8_t*)"ABC àZ"; // this is not a à, it's an a followed by ◌̀ (see documentation of the ciph_caesar function for more info)
  int shift = 1;
  const uint8_t* expected = (const uint8_t*)"BCD b̀A";

  uint8_t output[strlen((char*)input)];
  ciph_err_t err = ciph_caesar(input, strlen((char*)input), shift, (uint8_t*)output);
  if (err != CIPH_OK) {
    CU_FAIL("Non zero return code");
    return;
  }

  dbgout2(output, strlen((char*)input), expected, strlen((char*)expected));

  CU_ASSERT(u8_cmp2(output, strlen((char*)input), expected, strlen((char*)expected)) == 0);
}

void test_caesar_negative(void) {
  const uint8_t* input = (const uint8_t*)"ABZ";
  int shift = -1;
  const uint8_t* expected = (const uint8_t*)"ZAY";

  uint8_t output[strlen((char*)input)];
  ciph_err_t err = ciph_caesar(input, strlen((char*)input), shift, output);
  if (err != CIPH_OK) {
    CU_FAIL("Non zero return code");
    return;
  }

  dbgout2(output, strlen((char*)input), expected, strlen((char*)expected));

  CU_ASSERT(u8_cmp2(output, strlen((char*)input), expected, strlen((char*)expected)) == 0);
}
