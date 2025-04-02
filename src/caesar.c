#include <string.h>
#include <cipher.h>
#include <unitypes.h>
#include <unistr.h>

ciph_err_t ciph_caesar(const uint8_t* nonnil input, size_t input_len, int shift, uint8_t* nonnil output) {
  int clen;
  const uint8_t* input_ptr = input;
  size_t input_left = input_len;
  uint8_t* output_ptr = output;
  int _shift;

  while (input_left > 0) {
    clen = u8_mblen(input_ptr, input_left);
    if (clen == -1) return CIPH_ERR_ENCODING;
    if (clen == 0) { // NUL
      *output_ptr = 0;
      input_left -= 1;
      input_ptr += 1;
      output_ptr += 1;
      continue;
    }

    if (clen == 1) {
      if (input_ptr[0] >= 'a' && input_ptr[0] <= 'z') {
        _shift = (input_ptr[0] - 'a' + shift) % 26;
        if (_shift < 0) _shift = 26 + _shift;
        *output_ptr = 'a' + _shift;
      } else if (input_ptr[0] >= 'A' && input_ptr[0] <= 'Z') {
        _shift = (input_ptr[0] - 'A' + shift) % 26;
        if (_shift < 0) _shift = 26 + _shift;
        *output_ptr = 'A' + _shift;
      } else {
        memcpy(output_ptr, input_ptr, clen);
      }
    } else {
      memcpy(output_ptr, input_ptr, clen);
    }

    input_left -= clen;
    input_ptr += clen;
    output_ptr += clen;
  }

  return CIPH_OK;
}
