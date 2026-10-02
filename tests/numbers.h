#include <cipher.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <unistd.h>
#include <unistr.h>
#include <uninorm.h>
#include "utils.h"

void _test_numbers2(const uint8_t* input, size_t input_len, const uint8_t* expected, size_t expected_len, bool copy_non_encodable_characters) {
  ciph_str_t out = ciph_str_create(expected_len);
  CU_ASSERT(
    ciph_numbers(input, input_len, copy_non_encodable_characters, &out) == CIPH_OK
  );

  dbgout2(out.data, out.len, expected, expected_len);

  CU_ASSERT(out.len == expected_len);
  CU_ASSERT(u8_cmp2(out.data, expected_len, expected, expected_len) == 0);

  ciph_str_free(&out);
}

void _test_numbers(const uint8_t* input, const uint8_t* expected, bool copy_non_encodable_characters) {
  _test_numbers2(input, strlen((char*)input), expected, strlen((char*)expected), copy_non_encodable_characters);
}

void test_numbers(void) {
  _test_numbers((const uint8_t*)"ABCX", (const uint8_t*)"1 2 3 24", false);
}

void test_numbers_sentence(void) {
  _test_numbers((const uint8_t*)"ABCX Z", (const uint8_t*)"1 2 3 24 / 26", false);
}

void test_numbers_accent(void) {
  size_t len;
  uint8_t* norm = u8_normalize(UNINORM_NFD, (const uint8_t*)"à", strlen("à"), NULL, &len);
  _test_numbers2(norm, len, (const uint8_t*)"1", 1, false);
}

void test_numbers_accent_copy(void) {
  size_t len;
  uint8_t* norm = u8_normalize(UNINORM_NFC, (const uint8_t*)"à", strlen("à"), NULL, &len);
  _test_numbers2(norm, len, norm, len, true);
}

CU_SuiteInfo suite_numbers(void) {
  static CU_TestInfo tests[] = {
    { "numbers", test_numbers },
    { "numbers sentence", test_numbers_sentence },
    { "numbers accent", test_numbers_accent },
    { "numbers accent copy", test_numbers_accent_copy },
  };
  return (CU_SuiteInfo){ "numbers", NULL, NULL, NULL, NULL, tests };
}
