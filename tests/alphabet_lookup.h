#include <cipher.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <unistr.h>
#include "utils.h"

#define _test_alphabet_lookup(input, expected, alphabet) _test_alphabet_lookup2(input, strlen((const char*)input), expected, strlen((const char*)expected), alphabet)

void _test_alphabet_lookup2(
  const uint8_t* input, size_t input_len,
  const uint8_t* expected, size_t expected_len,
  uint8_t* alphabet
) {
  uint8_t output[input_len];
  ciph_err_t err = ciph_alphabet_lookup(input, input_len, alphabet, output);
  if (err != CIPH_OK) {
    CU_FAIL("non zero return code");
    return;
  }

  dbgout2(output, input_len, expected, expected_len);

  CU_ASSERT(u8_cmp2(output, input_len, expected, expected_len) == 0);
}

void test_alphabet_lookup(void) {
  uint8_t alphabet[26];
  memcpy(alphabet, CIPH_ALPHABET, 26);
  alphabet[1] = 'C';
  alphabet[2] = 'D';

  _test_alphabet_lookup((const uint8_t*)"ABC", (const uint8_t*)"ACD", alphabet);
}

void test_alphabet_atbash(void) {
  uint8_t atbash[26] = {0};
  ciph_alphabet_atbash(atbash);

  _test_alphabet_lookup((const uint8_t*)"ABCz", (const uint8_t*)"ZYXa", atbash);
}

void test_alphabet_vignere(void) {
  const uint8_t* input = (uint8_t*)"ABCzà";
  const uint8_t* expected = (uint8_t*)"SVWhs̀";

  char* word = "LIMONADE";
  CU_ASSERT(ciph_alphabet_vignere_validate((const uint8_t*)word, strlen(word)) == CIPH_LVAL_OK);

  uint8_t visualize[26] = {0};
  uint8_t alphabet[26] = {0};
  ciph_alphabet_vignere((const uint8_t*)word, strlen(word), alphabet, visualize);

  write(fileno(_stderr), "\n", 1);
  write(fileno(_stderr), visualize, 13);
  write(fileno(_stderr), "\n", 1);
  write(fileno(_stderr), visualize + 13, 13);
  write(fileno(_stderr), "\n", 1);
  write(fileno(_stderr), alphabet, 26);
  write(fileno(_stderr), "\n", 1);

  size_t input_len;
  uint8_t* normin = u8_normalize(UNINORM_NFD, input, strlen((char*)input), NULL, &input_len);

  _test_alphabet_lookup2(normin, input_len, expected, strlen((const char*)expected), alphabet);

  free((void*)normin);
}

CU_SuiteInfo suite_alphabet_lookup(void) {
  static CU_TestInfo tests[] = {
    { "alphabet lookup", test_alphabet_lookup },
    { "alphabet lookup atbash", test_alphabet_atbash },
    { "alphabet lookup vignere", test_alphabet_vignere },
  };
  return (CU_SuiteInfo){ "alphabet lookup", NULL, NULL, NULL, NULL, tests };
}
