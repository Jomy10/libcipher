#include <cipher.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <unistr.h>
#include "cipher/ciphers.h"
#include "cipher/utils.h"
#include "utils.h"

#define _test_sub2(input, expected, sub) _test_sub(input, strlen((const char*)input), expected, strlen((const char*)expected), sub)

void _test_sub(
  const uint8_t* input, size_t input_len,
  const uint8_t* expected, size_t expected_len,
  ciph_sub_t sub
) {
  ciph_str_t output = ciph_str_create(expected_len);
  if (output.data == NULL) {
    CU_FAIL("Alloc error");
    return;
  }
  CU_ASSERT(ciph_sub(input, input_len, sub, &output) == CIPH_OK);

  dbgout2(output.data, output.len, expected, expected_len);
  CU_ASSERT(expected_len == output.len);
  CU_ASSERT(u8_cmp(output.data, expected, expected_len) == 0);
}

void test_sub(void) {
  ciph_sub_entry_t entries[4] = {
    (ciph_sub_entry_t){ (const uint8_t*)"bc", 2, (const uint8_t*)"GFX", 3 },
    (ciph_sub_entry_t){ (const uint8_t*)"cd", 2, (const uint8_t*)"ER", 2 },
    // Checks priority (bc will be replaced, not bcd)
    (ciph_sub_entry_t){ (const uint8_t*)"bcd", 3, (const uint8_t*)"IR", 2 },
    (ciph_sub_entry_t){ (const uint8_t*)"fg", 2, (const uint8_t*)"XY", 2 },
  };
  ciph_sub_t sub;
  ciph_sub_parse(entries, 4, &sub);
  _test_sub2((const uint8_t*)"abcdefg", (const uint8_t*)"aGFXdeXY", sub);
  ciph_sub_free(&sub);
}

CU_SuiteInfo suite_sub(void) {
  static CU_TestInfo tests[] = {
    { "sub", test_sub },
  };
  return (CU_SuiteInfo){ "substitution", NULL, NULL, NULL, NULL, tests };
}
