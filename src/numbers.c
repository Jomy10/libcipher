#include "cipher/uc_cat.h"
#include "cipher/utils.h"
#include <cipher.h>

#include <stdio.h>
#include <unistr.h>
#include <unitypes.h>
#include <unictype.h>

enum NumbersPrev {
  ENCODABLE,
  TERMINAL,
  WRDBRK
};

ciph_err_t ciph_numbers(
  const uint8_t * _Nonnull input, size_t input_len,
  bool copy_non_encodable_characters,
  ciph_str_t *output
) {
  ucs4_t uc;
  int uc_len;
  int res;

  const uint8_t* input_ptr = input;
  size_t input_left = input_len;

  enum NumbersPrev prev = ENCODABLE;
  size_t prev_wrdbrk_size = 0;

  while (input_left > 0) {
    uc_len = u8_mbtouc(&uc, input_ptr, input_left);
    if (uc_len == -1) {
      return CIPH_ERR_ENCODING;
    } else if (uc_len == 0) { // NUL
      if (copy_non_encodable_characters) {
        if (ciph_str_push_char(output, 0) != CIPH_OK) return CIPH_ERR_ALLOC;

        input_left -= 1;
        input_ptr += 1;
        continue;
      }
    } else if (uc_is_property_sentence_terminal(uc) || uc_is_property_terminal_punctuation(uc)) {
      if (prev == WRDBRK) {
        if (ciph_str_push_str(output, (const uint8_t*)" /", 2) != CIPH_OK) return CIPH_ERR_ALLOC;
      } else if (prev != TERMINAL) {
        if (ciph_str_push_str(output, (const uint8_t*)" //", 3) != CIPH_OK) return CIPH_ERR_ALLOC;
      }
      prev = TERMINAL;
    } else if (ciph_uc_is_wordbreak(uc)) {
      if (prev == ENCODABLE) {
        if (ciph_str_push_str(output, (const uint8_t*)" /", 2) != CIPH_OK) return CIPH_ERR_ALLOC;
        prev = WRDBRK;
      }
    } else {
      if (uc_len == 1 && ((uc >= 'A' && uc <= 'Z') || (uc >= 'a' && uc <= 'z'))) {
        if (*input_ptr >= 'a') {
          res = *input_ptr - 'a' + 1;
        } else {
          res = *input_ptr - 'A' + 1;
        }

        if (input_ptr != input) { // add a space if not start of string
          ciph_str_push_char(output, ' ');
        }

        if (res / 10 > 0) {
          ciph_str_push_char(output, (res / 10) + '0');
        }
        ciph_str_push_char(output, (res % 10) + '0');
      } else if (copy_non_encodable_characters) {
        ciph_str_push_str(output, input_ptr, uc_len);
      }

      prev = ENCODABLE;
    }

    input_left -= uc_len;
    input_ptr += uc_len;
  }

  return CIPH_OK;
}
