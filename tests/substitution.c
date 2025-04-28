#include <cipher.h>
#include <CUnit/Basic.h>
#include <unistr.h>
#include "cipher/ciphers.h"
#include "utils.h"

void test_alph_sub(void) {
  const uint8_t* input = (const uint8_t*)"ABc. D EF";
  const uint8_t* expected = (const uint8_t*)"a :).b.c // d / e.f";

  size_t input_len_left = strlen((char*)input);
  size_t add_output_len = 0;
  size_t output_len = 0;
  size_t output_cap = 5;
  uint8_t* output = malloc(output_cap);

  ciph_SubAlphabetElement_t sub[26];
  sub[0] = (ciph_SubAlphabetElement_t){(const uint8_t*)"a :)", strlen("a :)")};
  sub[1] = (ciph_SubAlphabetElement_t){(const uint8_t*)"b", 1};
  sub[2] = (ciph_SubAlphabetElement_t){(const uint8_t*)"c", 1};
  sub[3] = (ciph_SubAlphabetElement_t){(const uint8_t*)"d", 1};
  sub[4] = (ciph_SubAlphabetElement_t){(const uint8_t*)"e", 1};
  sub[5] = (ciph_SubAlphabetElement_t){(const uint8_t*)"f", 1};
  sub[6] = (ciph_SubAlphabetElement_t){(const uint8_t*)"g", 1};
  sub[7] = (ciph_SubAlphabetElement_t){(const uint8_t*)"h", 1};
  sub[8] = (ciph_SubAlphabetElement_t){(const uint8_t*)"i", 1};
  sub[9] = (ciph_SubAlphabetElement_t){(const uint8_t*)"j", 1};
  sub[10] = (ciph_SubAlphabetElement_t){(const uint8_t*)"k", 1};
  sub[11] = (ciph_SubAlphabetElement_t){(const uint8_t*)"l", 1};
  sub[12] = (ciph_SubAlphabetElement_t){(const uint8_t*)"m", 1};
  sub[13] = (ciph_SubAlphabetElement_t){(const uint8_t*)"n", 1};
  sub[14] = (ciph_SubAlphabetElement_t){(const uint8_t*)"o", 1};
  sub[15] = (ciph_SubAlphabetElement_t){(const uint8_t*)"p", 1};
  sub[16] = (ciph_SubAlphabetElement_t){(const uint8_t*)"q", 1};
  sub[17] = (ciph_SubAlphabetElement_t){(const uint8_t*)"r", 1};
  sub[18] = (ciph_SubAlphabetElement_t){(const uint8_t*)"s", 1};
  sub[19] = (ciph_SubAlphabetElement_t){(const uint8_t*)"t", 1};
  sub[20] = (ciph_SubAlphabetElement_t){(const uint8_t*)"u", 1};
  sub[21] = (ciph_SubAlphabetElement_t){(const uint8_t*)"v", 1};
  sub[22] = (ciph_SubAlphabetElement_t){(const uint8_t*)"w", 1};
  sub[23] = (ciph_SubAlphabetElement_t){(const uint8_t*)"x", 1};
  sub[24] = (ciph_SubAlphabetElement_t){(const uint8_t*)"y", 1};
  sub[25] = (ciph_SubAlphabetElement_t){(const uint8_t*)"z", 1};

  printf("\n");
  while (true) {
    ciph_err_t err = ciph_char_alph_sub(
      input, input_len_left,
      output + output_len, output_cap - output_len,

      sub,

      (const uint8_t*)".", 1,
      (const uint8_t*)" / ", 3,
      (const uint8_t*)" // ", 4,

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
  }

  CU_ASSERT(input_len_left == 0);
  CU_ASSERT(output_len == strlen((char*)expected));

  dbgout2(output, output_len, expected, strlen((char*)expected));
  CU_ASSERT(u8_cmp2(output, output_len, expected, strlen((char*)expected)) == 0);
}
