#include <assert.h>
#include <string.h>
#include <cipher.h>
#include <unitypes.h>
#include <unistr.h>

#define _CIPH_ASCII_TO_UPPER(ASCII_CHAR) ASCII_CHAR + 32

const uint8_t CIPH_ALPHABET[26] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };

ciph_err_t ciph_alphabet_lookup(const uint8_t* nonnil input, size_t input_len, const uint8_t* nonnil lookup, uint8_t* nonnil output) {
  int clen;
  const uint8_t* input_ptr = input;
  size_t input_left = input_len;
  uint8_t* output_ptr = output;

  while (input_left > 0) {
    clen = u8_mblen(input_ptr, input_left);
    if (clen == -1) return CIPH_ERR_ENCODING;
    if (clen == 0) { // NUL
      *output_ptr = 0;
      input_left -= 1;
      input_ptr += 1;
      output_ptr += 1;
      continue;
    }

    if (clen == 1) {
      if (input_ptr[0] >= 'A' && input_ptr[0] <= 'Z') {
        *output_ptr = lookup[input_ptr[0] - 'A'];
      } else if (input_ptr[0] >= 'a' && input_ptr[0] <= 'z') {
        *output_ptr = _CIPH_ASCII_TO_UPPER(lookup[input_ptr[0] - 'a']);
      } else {
        memcpy(output_ptr, input_ptr, clen);
      }
    } else {
      memcpy(output_ptr, input_ptr, clen);
    }

    input_left -= clen;
    input_ptr += clen;
    output_ptr += clen;
  }

  return CIPH_OK;
}

void ciph_alphabet_atbash(uint8_t* nonnil buffer) {
  for (int i = 0; i < 26; i++) {
    buffer[i] = 'Z' - CIPH_ALPHABET[i] + 'A';
  }
}

ciph_lookup_validation_t ciph_alphabet_vignere_validate(const uint8_t* nonnil word, size_t word_len) {
  if (word_len > 26) return CIPH_LVAL_TOO_LONG;

  for (int i = 0; i < word_len; i++) {
    for (int j = 0; j < word_len; j++) {
      if (i == j) continue;

      if (word[i] == word[j]) return CIPH_LVAL_DOUBLE_CHAR;
    }
  }

  return CIPH_LVAL_OK;
}

void ciph_alphabet_vignere(const uint8_t* nonnil word, size_t word_len, uint8_t* nonnil buffer, uint8_t* nilable _alphabet) {
  assert(ciph_alphabet_vignere_validate(word, word_len) == 0);

  uint8_t alphabet_buffer[26];
  uint8_t* alphabet = alphabet_buffer;

  if (word_len == 0) {
    memcpy(alphabet, CIPH_ALPHABET, 26);
    return;
  }

  memcpy(alphabet, word, word_len);
  alphabet += word_len;

  for (int i = 0; i < 26; i++) {
    for (int j = 0; j < word_len; j++) {
      if (word[j] == CIPH_ALPHABET[i]) {
        goto skip;
      }
    }

    *alphabet = CIPH_ALPHABET[i];
    alphabet += 1;

    skip:
    continue;
  }

  for (int i = 0; i < 26; i++) {
    buffer[alphabet_buffer[i] - 'A'] = alphabet_buffer[(i + 13) % 26];
  }

  if (_alphabet != nil) {
    memcpy(_alphabet, alphabet_buffer, 26);
  }
}
