#include <cipher.h>
#include <math.h>
#include <assert.h>

#include <unistd.h>
#include <unistr.h>
#include <unitypes.h>
#include <unigbrk.h>

#include "cipher/error.h"
#include "cipher/uc_cat.h"
#include "cipher/utils.h"

static inline size_t _ciph_next_perfect_square(size_t input, size_t* nonnil n) {
  *n = (size_t)ceil(sqrt(input));
  return (*n) * (*n);
}

ciph_err_t _ciph_block_encode_one_word(
  const uint8_t* nonnil word, size_t word_len,
  ciph_str_t* nonnil output
) {
  ciph_err_t err = CIPH_OK;

  size_t breaks_cap = 16;
  size_t prev_breaks_cap = breaks_cap;
  const uint8_t** breaks = malloc(breaks_cap * sizeof(uint8_t*));
  const uint8_t** breaks_ptr = breaks;

  *breaks_ptr = word;
  breaks_ptr += 1;

  const uint8_t* word_end = word + word_len;

  // Collect grapheme clusters in word
  while (true) {
    // u8_grapheme_next -> returns starting position of next cluster. So we store
    // an array of all starting position in breaks.
    *breaks_ptr = (const uint8_t*) u8_grapheme_next(*(breaks_ptr - 1), word_end);
    if (*breaks_ptr == NULL) break; // all grapheme clusters found
    breaks_ptr += 1;
    if (breaks + breaks_cap == breaks_ptr) {
      breaks_cap *= 2;
      breaks = realloc(breaks, breaks_cap * sizeof(uint8_t*));
      breaks_ptr = breaks + prev_breaks_cap;
      prev_breaks_cap = breaks_cap;
    }
  }

  size_t word_size = breaks_ptr - breaks - 1; // in grapheme_clusters
  if (word_size == 1) {
    if (ciph_str_push_char(output, *word) != CIPH_OK
      || ciph_str_push_str(output, (const uint8_t*)"XXX", 3) != CIPH_OK
    ) {
      err = CIPH_ERR_ALLOC;
      goto RETURN;
    }
    goto RETURN;
  }

  size_t n;
  size_t final_word_size = _ciph_next_perfect_square(word_size, &n);

  const uint8_t* cstart;
  const uint8_t* cend;
  size_t csize = 0;
  size_t index;

  for (int i = 0; i < final_word_size; i++) {
    index = n * (i % n) + (i / n);

    if (index >= word_size) {
      if (ciph_str_push_char(output, 'X') != CIPH_OK) {
        err = CIPH_ERR_ALLOC;
        goto RETURN;
      }
    } else {
      cstart = breaks[index];
      cend = breaks[index + 1];

      csize = ((cend == NULL) ? (word + word_len) : cend) - cstart;
      assert(csize > 0);
      if (ciph_str_push_str(output, cstart, csize) != CIPH_OK) {
        err = CIPH_ERR_ALLOC;
        goto RETURN;
      }
    }
  }

RETURN:
  free((void*)breaks);

  return err;
}

ciph_err_t ciph_block_method(
  const uint8_t* nonnil input, size_t input_len,
  ciph_str_t* nonnil output
) {
  ciph_err_t err;

  ucs4_t uc;
  int uc_len;

  const uint8_t* input_ptr = input;
  size_t input_left = input_len;

  const uint8_t* word_start_ptr = input;
  int word_size = 0; // in bytes

  while (input_left > 0) {
    uc_len = u8_mbtouc(&uc, input_ptr, input_left);
    if (uc == 0xfffd) {
      return CIPH_ERR_ENCODING;
    }

    if (ciph_uc_is_wordbreak(uc)) {
      if (word_size > 0) {
        err = _ciph_block_encode_one_word(
          word_start_ptr, word_size,
          output
        );
        if (err != CIPH_OK) {
          return err;
        }
        word_size = 0;
      }

      // copy whitespace/word break character
      ciph_str_push_str(output, input_ptr, uc_len);

      word_start_ptr = input_ptr + 1;
    } else {
      word_size += uc_len;
    }

    input_left -= uc_len;
    input_ptr += uc_len;
  }

  // If ends with word, copy the word to the output
  if (word_size > 0) {
    err = _ciph_block_encode_one_word(
      word_start_ptr, word_size,
      output
    );
    if (err != CIPH_OK) {
      return err;
    }
  }

  return CIPH_OK;
}
