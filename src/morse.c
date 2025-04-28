#include <string.h>
#include <cipher.h>
#include <cipher/internal/utils.h>
#include <unictype.h>
#include <unigbrk.h>
#include <unistd.h>
#include <unistr.h>
#include <unicase.h>

#ifdef CIPH_DIT
#define DIT CIPH_DIT
#else
#define DIT "·"
#endif

#ifdef CIPH_DAH
#define DAH CIPH_DAH
#else
#define DAH "—"
#endif

#define MAX(A, B) (A > B) ? (A) : (B)

enum { MORSE_CHAR_MAX = MAX(strlen(DIT), strlen(DAH)) * 7 }; // this is an enum so that MORSE_CHAR_MAX is an actual const. See also: https://stackoverflow.com/a/18435398/14874405

#define CIPH_MORSE_CHAR(lit) len = strlen(lit); memcpy(morse_char, lit, len); return len;

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
  uint8_t* nonnil output, size_t output_len,
  bool copy_non_encodable_characters,
  const uint8_t* nilable * nilable _input_left, size_t* nilable _input_len_left,
  size_t* nilable _output_len
) {
  const uint8_t* input_ptr = input;
  const uint8_t* input_end = input + input_len;
  const uint8_t* next;
  int grapheme_len = 0;

  uint8_t* output_ptr = output;
  size_t output_left = output_len;

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
    if (first_codepoint_len == -1) return CIPH_ERR_ENCODING;
    if (first_codepoint_len == 0) { // NUL
      if (copy_non_encodable_characters) {
        *output_ptr = 0;
        output_ptr += 1;
        output_left -= 1;
      }
      input_ptr += 1;
      continue;
    }
    if (uc_is_property_sentence_terminal(first_codepoint) || uc_is_property_terminal_punctuation(first_codepoint)) {
      if (prev == TERMINAL) {
        goto NEXT;
      } else if (prev == WRDBRK) {
        if (output_left < 1) { // reverse last and reencode next pass
          input_ptr -= prev_wrdbrk_size;
          output_left += 2;
          output_ptr -= 2;
          break;
        } else {
          *(output_ptr - 1) = '/'; // replace space with a / to turn / into //
          *output_ptr = ' ';
          output_left -= 1;
          output_ptr += 1;
          goto NEXT;
        }
      }
      if (output_left < 3) break;
      memcpy(output_ptr, "// ", 3);
      output_ptr += 3;
      output_left -= 3;
      prev = TERMINAL;
      goto NEXT;
    }
    if (ciph_uc_is_wordbreak(first_codepoint)) {
      if (prev != ENCODABLE) {
        goto NEXT;
      };
      if (output_left < 2) break;
      *output_ptr = '/';
      output_ptr += 1;
      *output_ptr = ' ';
      output_ptr += 1;
      output_left -= 2;
      prev = WRDBRK;
      prev_wrdbrk_size = grapheme_len;
      goto NEXT;
    }

    morse_char_len = _ciph_morse_char(first_codepoint, morse_char);
    if (morse_char_len == -1) {
      if (copy_non_encodable_characters) {
        if (output_left < grapheme_len) break; // end of output
        memcpy(output_ptr, input_ptr, grapheme_len);
        output_ptr += grapheme_len;
        output_left -= grapheme_len;
      }
    } else {
      if (next != input_end) {
        if (output_left < morse_char_len + 1) break;
      } else {
        if (output_left < morse_char_len) break;
      }

      memcpy(output_ptr, morse_char, morse_char_len);
      output_ptr += morse_char_len;
      output_left -= morse_char_len;
      if (next != input_end) {
        *output_ptr = ' ';
        output_ptr += 1;
        output_left -= 1;
      }
    }

  NEXT:
    input_ptr = next;
  }

  if (input_ptr == input_end) {
    if (_input_left != nil) *_input_left = nil;
    if (_input_len_left != nil) *_input_len_left = 0;
    if (_output_len != nil) *_output_len = output_len - output_left;
  } else {
    if (_input_left != nil) *_input_left = input_ptr;
    if (_input_len_left != nil) *_input_len_left = (size_t) (input_end - input_ptr);
    if (_output_len != nil) *_output_len = output_len - output_left;
  }

  return CIPH_OK;
}
