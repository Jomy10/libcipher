#include <string.h>
#include <assert.h>

#include <unistr.h>
#include <unigbrk.h>
#include <unicase.h>
#include <unictype.h>

#include <cipher/uc_cat.h>
#include <cipher.h>

#ifdef CIPH_DIT
#define DIT CIPH_DIT
#else
#define DIT "."
#endif

#ifdef CIPH_DAH
#define DAH CIPH_DAH
#else
#define DAH "-"
#endif

#define MAX(A, B) ((A > B) ? (A) : (B))
enum { MORSE_CHAR_MAX = MAX(strlen(DIT), strlen(DAH)) * 7 }; // this is an enum so that MORSE_CHAR_MAX is an actual const. See also: https://stackoverflow.com/a/18435398/14874405

#define CIPH_MORSE_CHAR(lit) len = strlen(lit); assert(len <= MORSE_CHAR_MAX); memcpy(morse_char, lit, len); return len;

static inline int _ciph_morse_char(ucs4_t c, char* morse_char) {
  int len;
  switch (uc_toupper(c)) {
    case u'A': CIPH_MORSE_CHAR(DIT DAH);
    case u'B': CIPH_MORSE_CHAR(DAH DIT DIT DIT);
    case u'C': CIPH_MORSE_CHAR(DAH DIT DAH DIT);
    case u'D': CIPH_MORSE_CHAR(DAH DIT DIT);
    case u'E': CIPH_MORSE_CHAR(DIT);
    case u'F': CIPH_MORSE_CHAR(DIT DIT DAH DIT);
    case u'G': CIPH_MORSE_CHAR(DAH DAH DIT);
    case u'H': CIPH_MORSE_CHAR(DIT DIT DIT DIT);
    case u'I': CIPH_MORSE_CHAR(DIT DIT);
    case u'J': CIPH_MORSE_CHAR(DIT DAH DAH DAH);
    case u'K': CIPH_MORSE_CHAR(DAH DIT DAH);
    case u'L': CIPH_MORSE_CHAR(DIT DAH DIT DIT);
    case u'M': CIPH_MORSE_CHAR(DAH DAH);
    case u'N': CIPH_MORSE_CHAR(DAH DIT);
    case u'O': CIPH_MORSE_CHAR(DAH DAH DAH);
    case u'P': CIPH_MORSE_CHAR(DIT DAH DAH DIT);
    case u'Q': CIPH_MORSE_CHAR(DAH DAH DIT DAH);
    case u'R': CIPH_MORSE_CHAR(DIT DAH DIT);
    case u'S': CIPH_MORSE_CHAR(DIT DIT DIT);
    case u'T': CIPH_MORSE_CHAR(DAH);
    case u'U': CIPH_MORSE_CHAR(DIT DIT DAH);
    case u'V': CIPH_MORSE_CHAR(DIT DIT DIT DAH);
    case u'W': CIPH_MORSE_CHAR(DIT DAH DAH);
    case u'X': CIPH_MORSE_CHAR(DAH DIT DIT DAH);
    case u'Y': CIPH_MORSE_CHAR(DAH DIT DAH DAH);
    case u'Z': CIPH_MORSE_CHAR(DAH DAH DIT DIT);
    case u'0': CIPH_MORSE_CHAR(DAH DAH DAH DAH DAH);
    case u'1': CIPH_MORSE_CHAR(DIT DAH DAH DAH DAH);
    case u'2': CIPH_MORSE_CHAR(DIT DIT DAH DAH DAH);
    case u'3': CIPH_MORSE_CHAR(DIT DIT DIT DAH DAH);
    case u'4': CIPH_MORSE_CHAR(DIT DIT DIT DIT DAH);
    case u'5': CIPH_MORSE_CHAR(DIT DIT DIT DIT DIT);
    case u'6': CIPH_MORSE_CHAR(DAH DIT DIT DIT DIT);
    case u'7': CIPH_MORSE_CHAR(DAH DAH DIT DIT DIT);
    case u'8': CIPH_MORSE_CHAR(DAH DAH DAH DIT DIT);
    case u'9': CIPH_MORSE_CHAR(DAH DAH DAH DAH DIT);
    case u'.': CIPH_MORSE_CHAR(DIT DAH DIT DAH DIT DAH);
    case u',': CIPH_MORSE_CHAR(DAH DAH DIT DIT DAH DAH);
    case u'?': CIPH_MORSE_CHAR(DIT DIT DAH DAH DIT DIT);
    case u'!': CIPH_MORSE_CHAR(DAH DIT DAH DIT DAH DAH);
    case u'-': CIPH_MORSE_CHAR(DAH DIT DIT DIT DIT DAH);
    case 0xd7: CIPH_MORSE_CHAR(DAH DIT DIT DAH);
    case u'/': CIPH_MORSE_CHAR(DAH DIT DIT DAH DIT);
    case u':': CIPH_MORSE_CHAR(DAH DAH DAH DIT DIT DIT);
    case u'\'':CIPH_MORSE_CHAR(DIT DAH DAH DAH DAH DIT);
    case u')': CIPH_MORSE_CHAR(DAH DIT DAH DAH DIT DAH);
    case u';': CIPH_MORSE_CHAR(DAH DIT DAH DIT DAH);
    case u'(': CIPH_MORSE_CHAR(DAH DIT DAH DAH DIT);
    case u'=': CIPH_MORSE_CHAR(DAH DIT DIT DIT DAH);
    case u'@': CIPH_MORSE_CHAR(DIT DAH DAH DIT DAH DIT);
    case u'&': CIPH_MORSE_CHAR(DIT DAH DIT DIT DIT);
    case 0xc5: // Å
    case 0xc0: CIPH_MORSE_CHAR(DIT DAH DAH DIT DAH); // À
    case 0xc4: // Ä
    case 0xc6: CIPH_MORSE_CHAR(DIT DAH DIT DAH); // Æ
    case 0xc7: CIPH_MORSE_CHAR(DAH DIT DAH DIT DIT); // Ç
    case 0xc9: CIPH_MORSE_CHAR(DIT DIT DAH DIT DIT); // É
    case 0xc8: CIPH_MORSE_CHAR(DIT DAH DIT DIT DAH); // È
    case 0xd1: CIPH_MORSE_CHAR(DIT DAH DIT DIT DAH); // Ñ
    case 0xd8: // Ø
    case 0xd6: CIPH_MORSE_CHAR(DAH DAH DAH DIT); // Ö
    case 0xdc: CIPH_MORSE_CHAR(DIT DIT DAH DAH); // Ü
    case u'"': CIPH_MORSE_CHAR(DIT DAH DIT DIT DAH DIT);

    default:
      return -1;
  }

  return len;
}

enum MorsePrev {
  ENCODABLE,
  WRDBRK,
  TERMINAL
};

ciph_err_t ciph_morse(
  const uint8_t* nonnil input, size_t input_len,
  bool copy_non_encodable_characters,
  ciph_str_t* output
) {
  const uint8_t* input_ptr = input;
  const uint8_t* input_end = input + input_len;
  const uint8_t* next;
  int grapheme_len = 0;

  char morse_char[MORSE_CHAR_MAX] = {0};
  int morse_char_len;

  ucs4_t first_codepoint;
  int first_codepoint_len;

  enum MorsePrev prev = ENCODABLE;
  size_t prev_wrdbrk_size = 0;

  while (true) {
    next = u8_grapheme_next(input_ptr, input_end);
    if (next == NULL) break; // end of input
    grapheme_len = next - input_ptr;

    first_codepoint_len = u8_mbtouc(&first_codepoint, input_ptr, grapheme_len);
    if (first_codepoint_len == -1) {
      return CIPH_ERR_ENCODING;
    } else if (first_codepoint_len == 0) { // NUL
      if (copy_non_encodable_characters) {
        if (ciph_str_push_char(output, 0) != CIPH_OK) { return CIPH_ERR_ALLOC; }
      }
      input_ptr += 1;
      continue;
    } else if (uc_is_property_sentence_terminal(first_codepoint) || uc_is_property_terminal_punctuation(first_codepoint)) {
      if (prev == TERMINAL) {
        goto NEXT;
      } else if (prev == WRDBRK) {
        output->data[output->len - 1] = '/'; // replace space with a / to turn / into //
        if (ciph_str_push_char(output, ' ') != CIPH_OK) { return CIPH_ERR_ALLOC; }
        prev = TERMINAL;
      } else {
        if (ciph_str_push_str(output, (uint8_t*)"// ", 3) != CIPH_OK) { return CIPH_ERR_ALLOC; }
        prev = TERMINAL;
      }
    } else if (ciph_uc_is_wordbreak(first_codepoint)) {
      if (prev != ENCODABLE) {
        goto NEXT;
      }

      if (ciph_str_push_str(output, (uint8_t*)"/ ", 2) != CIPH_OK) { return CIPH_ERR_ALLOC; }
      prev = WRDBRK;
      prev_wrdbrk_size = grapheme_len;
    } else {
      prev = ENCODABLE;

      morse_char_len = _ciph_morse_char(first_codepoint, morse_char);
      if (morse_char_len == -1) {
        if (copy_non_encodable_characters) {
          if (ciph_str_push_str(output, input_ptr, grapheme_len) != CIPH_OK) { return CIPH_ERR_ALLOC; }
        }
      } else {
        if (ciph_str_push_str(output, (const uint8_t*)morse_char, morse_char_len) != CIPH_OK) { return CIPH_ERR_ALLOC; }
        if (next != input_end) {
          if (ciph_str_push_char(output, ' ') != CIPH_OK) { return CIPH_ERR_ALLOC; }
        }
      }
    }

    NEXT:
      input_ptr = next;
  } // loop

  return CIPH_OK;
}
