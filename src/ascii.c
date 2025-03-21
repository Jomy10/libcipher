#include <stddef.h>
#include <cipher/error.h>
#include <cipher/internal/nil.h>

static inline void _ctoa_ascii_base10(unsigned char in, char* nonnil out) {
  out[0] = ((in / 100) % 10) + '0';
  out[1] = ((in / 10) % 10) + '0';
  out[2] = ((in) % 10) + '0';
}

ciph_err_t ciph_ascii(const char* nonnil input, size_t input_len, char* nonnil output) {
  // char* buf = malloc(input_len * 4);

  for (size_t i = 0; i < input_len; i++) {
    _ctoa_ascii_base10(input[i], &output[i * 4]);
    output[i * 4 + 3] = ' ';
  }

  output[input_len * 4 - 1] = '\0';

  return CIPH_OK;
}
