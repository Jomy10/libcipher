#ifndef CIPH_NO_ALLOCS

#include <cipher.h>
#include <stdlib.h>


ciph_err_t ciph_alloc_char_alph_sub(
  const uint8_t* nonnil input, size_t input_len,
  const ciph_SubAlphabetElement_t* nonnil substitution_alphabet,
  const uint8_t* nilable char_sep, size_t char_sep_len,
  const uint8_t* nilable word_sep, size_t word_sep_len,
  const uint8_t* nilable sentence_sep, size_t sentence_sep_len,
  bool copy_non_encodable_characters,
  uint8_t* nilable * nonnil output, size_t* nonnil out_output_len
) {
  size_t input_len_left = input_len;
  size_t add_output_len = 0;
  size_t output_len = 0;
  size_t output_cap = input_len + input_len * char_sep_len + (word_sep_len + sentence_sep_len) * 2;
  *output = malloc(output_cap);

  ciph_err_t err;
  while (true) {
    err = ciph_char_alph_sub(
      input, input_len,
      *output + output_len, output_cap - output_len,
      substitution_alphabet,
      char_sep, char_sep_len,
      word_sep, word_sep_len,
      sentence_sep, sentence_sep_len,
      copy_non_encodable_characters,
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
    input_len = input_len_left;

    output_cap *= 2;
    *output = realloc(*output, output_cap);
  }

  *out_output_len = output_len;

  return CIPH_OK;
}

#endif
