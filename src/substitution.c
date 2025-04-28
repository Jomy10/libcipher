#include "cipher/ciphers.h"
#include <cipher.h>
#include <cipher/internal/utils.h>
#include <string.h>
#include <unistr.h>
#include <unigbrk.h>
#include <unictype.h>

#define DBG(len) printf("output_left = %lu\n", len)

#define MAX(a, b) ((a > b) ? (a) : (b))

enum SubPrev {
  ENCODABLE,
  TERMINAL,
  WRDBRK,
};

ciph_err_t ciph_char_alph_sub(
  const uint8_t* nonnil input, size_t input_len,
  uint8_t* nonnil output, size_t output_len,

  const ciph_SubAlphabetElement_t* substitution_alphabet,

  const uint8_t* char_sep, size_t char_sep_len,
  const uint8_t* word_sep, size_t word_sep_len,
  const uint8_t* sentence_sep, size_t sentence_sep_len,

  bool copy_non_encodable_characters,

  const uint8_t* nilable * nilable out_input_left, size_t* nilable out_input_len_left,
  size_t* nilable out_output_len
) {
  const uint8_t* input_ptr = input;
  const uint8_t* input_end = input + input_len;

  uint8_t* output_ptr = output;
  size_t output_left = output_len;

  ucs4_t codepoint;
  int codepoint_len;

  enum SubPrev prev = ENCODABLE;
  size_t prev_wrdbrk_size = 0;

  while (true) {
    codepoint_len = u8_mbtouc(&codepoint, input_ptr, input_end - input_ptr);

    if (codepoint_len == -1) return CIPH_ERR_ENCODING;
    if (codepoint_len == 0) { // NUL
      if (copy_non_encodable_characters) {
        *output_ptr = 0;
        output_ptr += 1;
        output_left -= 1;
      }
      input_ptr += 1;
      continue;
    }
    if (uc_is_property_sentence_terminal(codepoint) || uc_is_property_terminal_punctuation(codepoint)) {
      if (prev == TERMINAL) {
        goto NEXT;
      } else if (prev == WRDBRK) {
        if (output_left < MAX(0, sentence_sep_len - word_sep_len)) { // reverse last and reencode next pass
          input_ptr -= prev_wrdbrk_size;
          output_left += word_sep_len;
          output_ptr -= word_sep_len;
          break;
        } else {
          output_ptr -= word_sep_len;
          output_left += word_sep_len;
          if (sentence_sep_len > 0) memcpy(output_ptr, sentence_sep, sentence_sep_len);
          output_ptr += sentence_sep_len;
          output_left -= sentence_sep_len;
          goto NEXT;
        }
      }
      if (input_ptr != input) { // remove . prev word
        output_ptr -= 1;
        output_left += 1;
      }
      if (output_left < sentence_sep_len) break;
      if (sentence_sep_len > 0) memcpy(output_ptr, sentence_sep, sentence_sep_len);
      output_ptr += sentence_sep_len;
      output_left -= sentence_sep_len;
      prev = TERMINAL;
      goto NEXT;
    }
    if (ciph_uc_is_wordbreak(codepoint)) {
      if (prev != ENCODABLE) {
        goto NEXT;
      }
      if (input_ptr != input) { // remove . prev word
        output_ptr -= 1;
        output_left += 1;
      }
      if (output_left < word_sep_len) break;
      if (word_sep_len > 0) memcpy(output_ptr, word_sep, word_sep_len);
      output_ptr += word_sep_len;
      output_left -= word_sep_len;
      prev = WRDBRK;
      prev_wrdbrk_size = codepoint_len;
      goto NEXT;
    }

    prev = ENCODABLE;

    if (codepoint_len == 1 && ((codepoint >= 'A' && codepoint <= 'Z') || (codepoint >= 'a' && codepoint <= 'z'))) {
      int index = (int)((codepoint >= 'a') ? (*input_ptr - 'a') : (*input_ptr - 'A'));
      ciph_SubAlphabetElement_t sub = substitution_alphabet[index];
      if (output_left < sub.len + char_sep_len) break;
      memcpy(output_ptr, sub.sub, sub.len);
      output_ptr += sub.len;
      if (char_sep_len > 0) memcpy(output_ptr, char_sep, char_sep_len);
      output_ptr += char_sep_len;
      output_left -= char_sep_len + sub.len;
    } else if (copy_non_encodable_characters) {
      if (output_left < char_sep_len + codepoint_len) break; // need more capacity
      memcpy(output_ptr, input_ptr, codepoint_len);
      output_ptr += codepoint_len;
      if (char_sep_len > 0) memcpy(output_ptr, char_sep, char_sep_len);
      output_ptr += char_sep_len;
      output_left -= char_sep_len + codepoint_len;
    }

  NEXT:
    input_ptr += codepoint_len;
    if (input_ptr == input_end) break;
  }

  if (input_ptr == input_end) {
    if (prev == ENCODABLE) {
      output_left += 1; // remove trailing .
    }

    if (out_input_left != nil) *out_input_left = nil;
    if (out_input_len_left != nil) *out_input_len_left = 0;
    if (out_output_len != nil) *out_output_len = output_len - output_left;
  } else {
    if (out_input_left != nil) *out_input_left = input_ptr;
    if (out_input_len_left != nil) *out_input_len_left = (size_t) (input_end - input_ptr);
    if (out_output_len != nil) *out_output_len = output_len - output_left;
  }

  return CIPH_OK;
}
