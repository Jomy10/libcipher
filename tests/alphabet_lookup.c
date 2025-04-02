#include <unistd.h>
#include <CUnit/Basic.h>
#include "CUnit/CUnit.h"
#include "cipher/ciphers.h"
#include <string.h>
#include "uninorm.h"
#include "unistr.h"
#include "utils.h"

void test_alphabet_lookup(void) {
  const uint8_t* input = (uint8_t*)"ABC";
  const uint8_t* expected = (uint8_t*)"ACD";
  uint8_t alphabet[26];
  memcpy(alphabet, CIPH_ALPHABET, 26);
  alphabet[1] = 'C';
  alphabet[2] = 'D';

  uint8_t output[strlen((char*)input)];
  ciph_err_t err = ciph_alphabet_lookup(input, strlen((char*)input), alphabet, output);
  if (err != CIPH_OK) {
    CU_FAIL("non zero return code");
    return;
  }

  dbgout2(output, strlen((char*)input), expected, strlen((char*)expected));

  CU_ASSERT(u8_cmp2(output, strlen((char*)input), expected, strlen((char*)expected)) == 0);
}

void test_alphabet_atbash(void) {
  const uint8_t* input = (uint8_t*)"ABCz";
  const uint8_t* expected = (uint8_t*)"ZYXa";

  uint8_t alphabet[26] = {0};
  ciph_alphabet_atbash(alphabet);

  uint8_t output[strlen((char*)input)];
  ciph_err_t err = ciph_alphabet_lookup(input, strlen((char*)input), alphabet, output);
  if (err != CIPH_OK) {
    CU_FAIL("non zero return code");
    return;
  }

  dbgout2(output, strlen((char*)input), expected, strlen((char*)expected));

  CU_ASSERT(u8_cmp2(output, strlen((char*)input), expected, strlen((char*)expected)) == 0);
}

void test_alphabet_vignere(void) {
  const uint8_t* input = (uint8_t*)"ABCzà";
  const uint8_t* expected = (uint8_t*)"SVWhs̀";

  uint8_t visualize[26] = {0};
  uint8_t alphabet[26] = {0};
  ciph_alphabet_vignere((uint8_t*)"LIMONADE", strlen("LIMONADE"), alphabet, visualize);

  size_t input_len;
  uint8_t* normin = u8_normalize(UNINORM_NFD, input, strlen((char*)input), NULL, &input_len);

  uint8_t output[input_len];
  ciph_err_t err = ciph_alphabet_lookup(normin, input_len, alphabet, output);
  if (err != CIPH_OK) {
    CU_FAIL("non zero return code");
    return;
  }

  dbgout2(output, input_len, expected, strlen((char*)expected));
  write(fileno(_stderr), visualize, 13);
  write(fileno(_stderr), "\n", 1);
  write(fileno(_stderr), visualize + 13, 13);
  write(fileno(_stderr), "\n", 1);
  write(fileno(_stderr), alphabet, 26);
  write(fileno(_stderr), "\n", 1);

  CU_ASSERT(u8_cmp2(output, input_len, expected, strlen((char*)expected)) == 0);

  free((void*)normin);
}
