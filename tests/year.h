#include <cipher.h>
#include <unistd.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <unistr.h>
#include <uninorm.h>
#include "cipher/ciphers.h"
#include "utils.h"

#define _test_year(input, expected, year, mask) _test_year2(input, strlen((const char*)input), expected, strlen((const char*)expected), year, mask)

void _test_year2(
  const uint8_t* input, size_t input_len,
  const uint8_t* expected, size_t expected_len,
  uint8_t year[const 4],
  uint32_t mask
) {
  ciph_str_t output = ciph_str_create(input_len * 2);
  ciph_err_t err = ciph_year(input, input_len, year, mask, &output);
  CU_ASSERT(err == CIPH_OK);

  dbgout2(output.data, output.len, expected, expected_len);
  CU_ASSERT(u8_cmp2(output.data, output.len, expected, expected_len) == 0);

  ciph_str_free(&output);
}

void test_year(void) {
  uint8_t year[] = { 1, 9, 9, 6 };
  _test_year(
    (const uint8_t*)"GA NU DADELIJK TERUG NAAR HET LOKAAL",
    (const uint8_t*)"GAJAOK NKRA UTHA DEEL ARTX DULX EGX LNX IAX",
    year,
    CIPH_CHAR_INCLUDE_LETTERS
  );
}

void test_year_doc(void) {
  uint8_t year[] = { 1, 9, 9, 6 };
  _test_year(
    (const uint8_t*)"hello world how are you today",
    (const uint8_t*)"heht loo lwd oaa wry oeX ry lo du",
    year,
    CIPH_CHAR_INCLUDE_LETTERS
  );
}

void test_year_sentences(void) {
  uint8_t year[] = { 2, 0, 2, 6 };
  _test_year(
    (const uint8_t*)"This is a test. With multiple, sentences!",
    (const uint8_t*)"Tiit hssX a t e s. Wtmlsns ihueetX le tn ic pe!",
    year,
    CIPH_CHAR_INCLUDE_LETTERS
  );
}

CU_SuiteInfo suite_year(void) {
  static CU_TestInfo tests[] = {
    { "year", test_year },
    { "year doc", test_year_doc },
    { "year sentences", test_year_sentences }
  };
  return (CU_SuiteInfo){ "year", NULL, NULL, NULL, NULL, tests };
}
