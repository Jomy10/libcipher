#include <cipher.h>
#include <CUnit/Basic.h>
#include <unistr.h>
#include "cipher/utils.h"
#include "utils.h"

void _test_morse(const uint8_t* input, const uint8_t* expected, bool copy_non_encodable_chars) {
  ciph_str_t output = ciph_str_create(strlen((const char*)expected));
  ciph_err_t err = ciph_morse(input, strlen((char*)input), copy_non_encodable_chars, &output);
  if (err != CIPH_OK) {
    CU_FAIL("non zero return code");
    return;
  }
  CU_ASSERT(output.len == strlen((const char*)expected));

  dbgout2(output.data, output.len, expected, strlen((const char*)expected));

  CU_ASSERT(u8_cmp2(output.data, output.len, expected, strlen((const char*)expected)) == 0);

  ciph_str_free(&output);
}

void test_morse(void) {
  _test_morse((const uint8_t*)"ABc", (const uint8_t*)".- -... -.-.", false);
}

void test_morse_small_buffer(void) {
  _test_morse((const uint8_t*)"ABcä", (const uint8_t*)".- -... -.-. .-.-", true);
}

void test_morse_multi_word(void) {
  _test_morse((const uint8_t*)"ABc DeF", (const uint8_t*)".- -... -.-. / -.. . ..-.", true);
}

void test_morse_sentence(void) {
  _test_morse((const uint8_t*)"ABc DeF. AD", (const uint8_t*)".- -... -.-. / -.. . ..-. // .- -..", true);
}

CU_SuiteInfo suite_morse(void) {
  static CU_TestInfo tests[] = {
    { "morse", test_morse },
    { "morse small buffer", test_morse },
    { "morse multi word", test_morse_multi_word },
    { "morse sentence", test_morse_sentence },
  };
  return (CU_SuiteInfo){ "morse", NULL, NULL, NULL, NULL, tests };
}
