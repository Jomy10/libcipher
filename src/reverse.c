#include <cipher.h>
#include <cipher/internal/utils.h>
#include <string.h>
#include <unitypes.h>
#include <unistr.h>
#include <uniwbrk.h>
#include <unigbrk.h>

/// `*output` will point to the index after the last character of the reversed word
static inline void _ciph_reverse_one_word(const uint8_t* nonnil word, size_t word_len, uint8_t** nonnil output) {
  const uint8_t* word_ptr = word;
  const uint8_t* word_end = word + word_len;
  const uint8_t* next;
  int grapheme_len = 0;
  uint8_t* output_end = *output + word_len;

  while (true) {
    next = u8_grapheme_next(word_ptr, word_end);
    if (next == NULL) break;
    grapheme_len = next - word_ptr;

    output_end -= grapheme_len;
    memcpy(output_end, word_ptr, grapheme_len);

    word_ptr = next;
  }

  *output += word_len;
}

ciph_err_t ciph_reverse_words(const uint8_t* nonnil input, size_t input_len, uint8_t* nonnil output) {
  ucs4_t uc;
  int uc_len;
  const uint8_t* input_ptr = input;
  size_t input_left = input_len;
  uint8_t* output_ptr = output;

  // word start
  const uint8_t* start_ptr = input;
  int word_size = 0;

  while (input_left > 0) {
    uc_len = u8_mbtouc(&uc, input_ptr, input_left); // we might be able to use u8_mtouc_unsafe instead if we are certain `input` is valid UTF-8
    if (uc == 0xfffd) {
      return CIPH_ERR_ENCODING;
    }

    if (ciph_uc_is_wordbreak(uc)) {
        // wordbreak or special character -> copy
        if (word_size > 0) {
          _ciph_reverse_one_word(start_ptr, word_size, &output_ptr);
          word_size = 0;
        }

        // copy whitespace/special character
        memcpy(output_ptr, input_ptr, uc_len);
        output_ptr += uc_len;

        start_ptr = input_ptr + 1;
    } else {
        // letter -> reverse
        word_size += 1;
    }

    input_left -= uc_len;
    input_ptr += uc_len;
  }

  if (word_size > 0) {
    _ciph_reverse_one_word(start_ptr, word_size, &output_ptr);
  }

  return CIPH_OK;
}
