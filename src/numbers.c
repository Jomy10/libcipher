#include <cipher.h>
#include <string.h>
#include "unistr.h"

ciph_err_t ciph_numbers(
  const uint8_t* nonnil input, size_t input_len,
  uint8_t* nonnil output, size_t output_len,
  bool copy_non_encodable_characters,
  const uint8_t* nilable * nilable out_input_left, size_t* nilable out_input_len_left,
  size_t* out_output_len
) {
  int clen;
  int cstrlen;
  int res;
  const uint8_t* input_ptr = input;
  size_t input_left = input_len;
  uint8_t* output_ptr = output;
  size_t output_left = output_len;

  while (input_left > 0) {
    clen = u8_mblen(input_ptr, input_left);
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

    if (out_input_left != nil) *out_input_left = NULL;
    if (out_input_len_left != nil) *out_input_len_left = 0;
    if (out_output_len != nil) *out_output_len = output_len;
    input_left -= clen;
    input_ptr += clen;
  }

  return CIPH_OK;

  // There is more input to be parsed
RET_NEXT:
  if (out_input_left != nil) *out_input_left = input;
  if (out_input_len_left != nil) *out_input_len_left = input_left;
  if (out_output_len != nil) *out_output_len = output_len;
  return CIPH_OK;
}
