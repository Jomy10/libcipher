#include <cipher.h>
#include <cipher/internal/utils.h>
#include <string.h>
#include "unictype.h"
#include "unistr.h"
#include "unitypes.h"

enum NumbersPrev {
  ENCODABLE,
  TERMINAL,
  WRDBRK
};

// TODO: add wordbreaks and sentence breaks like morse
ciph_err_t ciph_numbers(
  const uint8_t* nonnil input, size_t input_len,
  uint8_t* nonnil output, size_t output_len,
  bool copy_non_encodable_characters,
  const uint8_t* nilable * nilable out_input_left, size_t* nilable out_input_len_left,
  size_t* out_output_len
) {
  int clen;
  ucs4_t uc;
  int cstrlen;
  int res;
  const uint8_t* input_ptr = input;
  size_t input_left = input_len;
  uint8_t* output_ptr = output;
  size_t output_left = output_len;

  enum NumbersPrev prev = ENCODABLE;
  size_t prev_wrdbrk_size = 0;

  while (input_left > 0) {
    clen = u8_mbtouc(&uc, input_ptr, input_left);
    if (clen == -1) return CIPH_ERR_ENCODING;
    if (clen == 0) { // NUL
      if (copy_non_encodable_characters) {
        *output_ptr = 0;
        output_ptr += 1;
      }

      input_left -= 1;
      input_ptr += 1;
      continue;
    }
    if (uc_is_property_sentence_terminal(uc) || uc_is_property_terminal_punctuation(uc)) {
      if (prev == TERMINAL) {
        goto NEXT;
      } else if (prev == WRDBRK) {
        if (output_left < 1) { // reverse last and reencode next pass
          input_ptr -= prev_wrdbrk_size;
          input_left += prev_wrdbrk_size;
          output_left += 2;
          output_ptr -= 2;
          goto RET_NEXT;
        } else {
          *(output_ptr - 1) = '/';
          *output_ptr = ' ';
          output_left -= 1;
          output_ptr += 1;
          goto NEXT;
        }
      }
      if (output_left < 3) goto RET_NEXT;
      memcpy(output_ptr, "// ", 3);
      output_ptr += 3;
      output_left -= 3;
      prev = TERMINAL;
      goto NEXT;
    }
    if (ciph_uc_is_wordbreak(uc)) {
      if (prev != ENCODABLE) goto NEXT;
      if (output_left < 2) goto RET_NEXT;
      memcpy(output_ptr, "/ ", 2);
      output_ptr += 2;
      output_left -= 2;
      prev = WRDBRK;
      prev_wrdbrk_size = clen;
      goto NEXT;
    }

    if (clen == 1 && ((*input_ptr >= 'A' && *input_ptr <= 'Z') || (*input_ptr >= 'a' && *input_ptr <= 'z'))) {
      if (*input_ptr >= 'a') {
        res = *input_ptr - 'a' + 1;
      } else {
        res = *input_ptr - 'A' + 1;
      }
      if (res / 10 > 0) {
        cstrlen = 2;
      } else {
        cstrlen = 1;
      }
      if (input_left != clen) { // add a space if not end of input
        cstrlen += 1;
      }
      if (output_left < cstrlen) goto RET_NEXT;

      if (res / 10 > 0) {
        *output_ptr = (res / 10) + '0';
        output_ptr += 1;
      }
      *output_ptr = (res % 10) + '0';
      output_ptr += 1;
      if (input_left != clen) {
        *output_ptr = ' ';
        output_ptr += 1;
      }
      output_left -= cstrlen;
    } else if (copy_non_encodable_characters) {
      if (output_left - clen < 0) goto RET_NEXT;
      memcpy(output_ptr, input_ptr, clen);
      output_ptr += clen;
      output_len -= clen;
    }

  NEXT:
    input_left -= clen;
    input_ptr += clen;
  }

  if (out_input_left != nil) *out_input_left = NULL;
  if (out_input_len_left != nil) *out_input_len_left = 0;
  if (out_output_len != nil) *out_output_len = output_len;

  return CIPH_OK;

  // There is more input to be parsed
RET_NEXT:
  if (out_input_left != nil) *out_input_left = input;
  if (out_input_len_left != nil) *out_input_len_left = input_left;
  if (out_output_len != nil) *out_output_len = output_len;
  return CIPH_OK;
}
