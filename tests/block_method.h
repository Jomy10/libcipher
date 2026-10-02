#include <cipher.h>
#include <stdint.h>
#include <CUnit/Basic.h>
#include <unistr.h>
#include "utils.h"

#define _test_block_method(input, expected) _test_block_method2((input), strlen((const char*)(input)), (expected), strlen((const char*)(expected)))

void _test_block_method2(
  const uint8_t* input, size_t input_len,
  const uint8_t* expected, size_t expected_len
) {
  ciph_str_t output = ciph_str_create(8);
  ciph_err_t err = ciph_block_method(
    input, input_len,
    &output
  );

  if (err != CIPH_OK) {
    CU_FAIL("non zero return code");
    return;
  }

  CU_ASSERT(output.len == expected_len);

  dbgout2(output.data, output.len, expected, expected_len);
  CU_ASSERT(u8_cmp2(output.data, output.len, expected, expected_len) == 0);
}

void test_block_method(void) {
  _test_block_method(((const uint8_t*)"ABC"), ((const uint8_t*)"ACBX"));
}

void test_block_method_big(void) {
  _test_block_method((const uint8_t*)"ABD HOTSÙMMERs̀☀", (const uint8_t*)"ADBX HÙRXOMs̀XTM☀XSEXX");
}

CU_SuiteInfo suite_block_method(void) {
  static CU_TestInfo tests[] = {
    { "block method", test_block_method },
    { "block method big", test_block_method_big }
  };
  return (CU_SuiteInfo){ "block method", NULL, NULL, NULL, NULL, tests };
}
