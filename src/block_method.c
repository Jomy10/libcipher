#include <cipher.h>
#include <cipher/internal/utils.h>
#include <math.h>
#include <string.h>
#include <unitypes.h>
#include <unistr.h>
#include <unigbrk.h>

#define ISNULL(ptr, val) ((ptr == NULL) ? (val) : (ptr))

static inline size_t _ciph_next_perfect_square(size_t input, size_t* nonnil n) {
  *n = (size_t)ceil(sqrt(input));
  return (*n) * (*n);
}

static inline int _ciph_encode_one_word(
  const uint8_t* nonnil word, size_t word_len,
  uint8_t** nonnil output, size_t output_len
) {
  size_t breaks_cap = 16;
  const uint8_t** breaks = malloc(breaks_cap);
  const uint8_t** breaks_ptr = breaks;

  *breaks_ptr = word;
  breaks_ptr += 1;

  const uint8_t* word_end = word + word_len;

  while (true) {
    *breaks_ptr = u8_grapheme_next(*(breaks_ptr - 1), word_end);
    if (*breaks_ptr == NULL) break; // all grapheme clusters found
    if (breaks + breaks_cap == breaks_ptr) {
      breaks_cap *= 2;
      breaks = realloc(breaks, breaks_cap);
    }
    breaks_ptr += 1;
  }

  size_t word_size = breaks_ptr - breaks - 1; // in grapheme_clusters
  size_t n;
  size_t final_word_size = _ciph_next_perfect_square(word_size, &n);

  if (output_len < final_word_size) {
    free((void*)breaks);
    return 1; // should allocate more memory
  }

  const uint8_t* cstart;
  const uint8_t* cend;
  size_t csize = 0;
  size_t index;
  for (int i = 0; i < final_word_size; i++) {
    index = n * (i % n) + (i / n);

    if (index >= word_size) {
      **output = 'X';
      *output += 1;
    } else {
      cstart = breaks[index];
      cend = breaks[index + 1];

      csize = ISNULL(cend, word + word_len) - cstart;
      memcpy(*output, cstart, csize);
      *output += csize;
    }
  }

  free((void*)breaks);

  return 0;
}

ciph_err_t ciph_block_method(
  const uint8_t* nonnil input, size_t input_len,
  uint8_t* nonnil output, size_t output_len,
  const uint8_t* nilable * nilable out_input_left, size_t* nilable out_input_len_left,
  size_t* nilable out_output_len
) {
  ucs4_t uc;
  int uc_len;

  const uint8_t* input_ptr = input;
  size_t input_left = input_len;

  uint8_t* output_ptr = output;
  uint8_t* output_end = output + output_len;
  // size_t output_left = output_len;

  const uint8_t* word_start_ptr = input;
  int word_size = 0; // in bytes

  int revert = 0;

  while (input_left > 0) {
    uc_len = u8_mbtouc(&uc, input_ptr, input_left);
    if (uc == 0xfffd) {
      return CIPH_ERR_ENCODING;
    }

    if (ciph_uc_is_wordbreak(uc)) {
      if (word_size > 0) {
        revert = _ciph_encode_one_word(
          word_start_ptr, word_size,
          &output_ptr, output_end - output_ptr
        );
        if (revert != 0) goto REVERT;
        word_size = 0;
      }

      // copy whitespace/special character
      memcpy(output_ptr, input_ptr, uc_len);
      output_ptr += uc_len;

      word_start_ptr = input_ptr + 1;
    } else {
      word_size += uc_len;
    }

    input_left -= uc_len;
    input_ptr += uc_len;
  }

  if (word_size > 0) {
    revert = _ciph_encode_one_word(
      word_start_ptr, word_size,
      &output_ptr, output_end - output_ptr
    );
    if (revert != 0) goto REVERT;
  }

  if (out_input_left != nil) *out_input_left = NULL;
  if (out_input_len_left != nil) *out_input_len_left = 0;
  if (out_output_len != nil) *out_output_len = output_ptr - output;

  return CIPH_OK;

REVERT:
  input_ptr -= word_size;
  input_left += word_size;

  if (out_input_left != nil) *out_input_left = input_ptr;
  if (out_input_len_left != nil) *out_input_len_left = input_left;
  if (out_output_len != nil) *out_output_len = output_ptr - output;

  return CIPH_OK;
}
