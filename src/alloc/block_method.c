#ifndef CIPH_NO_ALLOCS

#include <cipher.h>
#include <stdlib.h>

ciph_err_t ciph_alloc_block_method(
  const uint8_t* nonnil input, size_t input_len,
  uint8_t* nilable * nonnil output,
  size_t* nilable out_output_len
) {
  size_t input_len_left = input_len;
  size_t add_output_len = 0;
  size_t output_len = 0;
  size_t output_cap = input_len * 1.5;
  *output = malloc(output_cap);

  ciph_err_t err;
  while (true) {
    err = ciph_block_method(
      input, input_len_left,
      *output + output_len, output_cap - output_len,
      &input, &input_len_left,
      &add_output_len
    );

    output_len += add_output_len;

    if (err != CIPH_OK) {
      free(*output);
      *output = NULL;
      return err;
    }

    if (input_len_left == 0) break;

    output_cap *= 2;
    *output = realloc(*output, output_cap);
  }

  if (out_output_len != NULL) *out_output_len = output_len;

  return CIPH_OK;
}

#endif
