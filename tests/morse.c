#include <cipher.h>
#include <CUnit/Basic.h>
#include <unistr.h>
#include "utils.h"

void test_morse(void) {
  const uint8_t* input = (const uint8_t*)"ABc";
  const uint8_t* expected = (const uint8_t*)"·- -··· -·-·";

  size_t input_len_left;
  size_t output_len;
  uint8_t output[strlen((char*)expected)];
  memset(output, 0, strlen((char*)expected));
  ciph_err_t err = ciph_morse(
    input, strlen((char*)input),
    output, strlen((char*)expected),
    false,
    NULL, &input_len_left, &output_len
  );
  if (err != CIPH_OK) {
    CU_FAIL("non zero return code");
    return;
  }
  CU_ASSERT(input_len_left == 0);
  CU_ASSERT(output_len == strlen((char*)expected));

  dbgout2(output, output_len, expected, strlen((char*)expected));

  CU_ASSERT(u8_cmp2(output, output_len, expected, strlen((char*)expected)) == 0);
}

void test_morse_small_buffer(void) {
  const uint8_t* input = (const uint8_t*)"ABcä";
  const uint8_t* expected = (const uint8_t*)"·- -··· -·-· ·-·-";

  size_t input_len_left = strlen((char*)input);
  size_t add_output_len = 0;
  size_t output_len = 0;
  size_t output_cap = 5;
  uint8_t* output = malloc(output_cap);

  while (true) {
    ciph_err_t err = ciph_morse(
      input, input_len_left,
      output + output_len, output_cap - output_len,
      true,
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

    eprintf("realloc -> %lu\n", output_cap);
  }

  CU_ASSERT(input_len_left == 0);
  CU_ASSERT(output_len == strlen((char*)expected));

  dbgout2(output, output_len, expected, strlen((char*)expected));
  CU_ASSERT(u8_cmp2(output, output_len, expected, strlen((char*)expected)) == 0);
}

void test_morse_multi_word(void) {
  const uint8_t* input = (const uint8_t*)"ABc DeF";
  const uint8_t* expected = (const uint8_t*)"·- -··· -·-· / -·· · ··-·";

  size_t input_len_left = strlen((char*)input);
  size_t add_output_len = 0;
  size_t output_len = 0;
  size_t output_cap = 5;
  uint8_t* output = malloc(output_cap);
  memset(output, 'A', output_cap);

  while (true) {
    ciph_err_t err = ciph_morse(
      input, input_len_left,
      output + output_len, output_cap - output_len,
      true,
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

    eprintf("realloc -> %lu\n", output_cap);
  }

  CU_ASSERT(input_len_left == 0);
  CU_ASSERT(output_len == strlen((char*)expected));

  dbgout2(output, output_len, expected, strlen((char*)expected));
  CU_ASSERT(u8_cmp2(output, output_len, expected, strlen((char*)expected)) == 0);
}

void test_morse_sentence(void) {
  const uint8_t* input = (const uint8_t*)"ABc DeF. AD";
  const uint8_t* expected = (const uint8_t*)"·- -··· -·-· / -·· · ··-· // ·- -··";

  size_t input_len_left = strlen((char*)input);
  size_t add_output_len = 0;
  size_t output_len = 0;
  size_t output_cap = 5;
  uint8_t* output = malloc(output_cap);
  memset(output, 'A', output_cap);

  while (true) {
    ciph_err_t err = ciph_morse(
      input, input_len_left,
      output + output_len, output_cap - output_len,
      true,
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

    eprintf("realloc -> %lu\n", output_cap);
  }

  CU_ASSERT(input_len_left == 0);
  CU_ASSERT(output_len == strlen((char*)expected));

  dbgout2(output, output_len, expected, strlen((char*)expected));
  CU_ASSERT(u8_cmp2(output, output_len, expected, strlen((char*)expected)) == 0);
}
