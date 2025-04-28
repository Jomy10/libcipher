#include <CUnit/Basic.h>
#include <cipher.h>
#include <unistr.h>
#include "utils.h"

void test_block_method(void) {
  const uint8_t* input = (const uint8_t*)"ABC";
  const uint8_t* expected = (const uint8_t*)"ACBX";

  size_t input_len_left = strlen((char*)input);
  size_t add_output_len = 0;
  size_t output_len = 0;
  size_t output_cap = 5;
  uint8_t* output = malloc(output_cap);

  while (true) {
    ciph_err_t err = ciph_block_method(
      input, input_len_left,
      output + output_len, output_cap - output_len,
      &input, &input_len_left,
      &add_output_len
    );

    output_len = output_len + add_output_len;

    if (err != CIPH_OK) {
      CU_FAIL("non zero return code");
      return;
    }

    if (input_len_left == 0) break;

    output_cap *= 2;
    output = realloc(output, output_cap);
  }

  CU_ASSERT(input_len_left == 0);
  CU_ASSERT(output_len == strlen((char*)expected));

  dbgout2(output, output_len, expected, strlen((char*)expected));
  CU_ASSERT(u8_cmp2(output, output_len, expected, strlen((char*)expected)) == 0);
}

/// Test with bigger word + emojis and diacritics (check if grapheme clusters working)
void test_block_method_big(void) {
  const uint8_t* input = (const uint8_t*)"ABC HOTSÙMMERs̀☀";
  const uint8_t* expected = (const uint8_t*)"ACBX HÙRXOMs̀XTM☀XSEXX";

  size_t input_len_left = strlen((char*)input);
  size_t add_output_len = 0;
  size_t output_len = 0;
  size_t output_cap = 5;
  uint8_t* output = malloc(output_cap);

  printf("\n");
  while (true) {
    ciph_err_t err = ciph_block_method(
      input, input_len_left,
      output + output_len, output_cap - output_len,
      &input, &input_len_left,
      &add_output_len
    );

    output_len = output_len + add_output_len;

    if (err != CIPH_OK) {
      CU_FAIL("non zero return code");
      return;
    }

    if (input_len_left == 0) break;

    output_cap *= 2;
    output = realloc(output, output_cap);
  }

  CU_ASSERT(input_len_left == 0);
  CU_ASSERT(output_len == strlen((char*)expected));

  dbgout2(output, output_len, expected, strlen((char*)expected));
  CU_ASSERT(u8_cmp2(output, output_len, expected, strlen((char*)expected)) == 0);
}
