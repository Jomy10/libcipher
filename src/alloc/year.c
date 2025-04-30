#ifndef CIPH_NO_ALLOCS

#include <cipher.h>
#include <stdlib.h>

ciph_err_t ciph_alloc_year(
  const uint8_t* nonnil input, size_t input_len,
  uint8_t year[nonnil 4],
  uint32_t char_include_bitmask,
  uint8_t* nilable * nonnil output, size_t* nonnil out_output_len
) {
  size_t add_output_len = 0;
  size_t output_len = 0;
  size_t output_cap = input_len * 1.5;
  *output = malloc(output_cap);

  ciph_year_ires_t ir = {0};

  ciph_err_t err;
  while (true) {
    err = ciph_year(
      input, input_len,
      year,
      char_include_bitmask,
      *output + output_len, output_cap - output_len,
      &ir,
      &add_output_len
    );

    output_len += add_output_len;

    switch (err) {
      case CIPH_OK:
        *out_output_len = output_len;
        return CIPH_OK;
      case CIPH_GROW:
        output_cap *= 2;
        *output = realloc(*output, output_cap);
        break;
      default:
        free(*output);
        *output = NULL;
        return err;
    }
  }
}

#endif
