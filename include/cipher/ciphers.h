#ifndef _CIPH_H
#define _CIPH_H

#include <stddef.h>
#include <unitypes.h>
#include "error.h"
#include "internal/nil.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Converts characters into their ASCII equivalents. ASCII values are padded with zeroes
/// to have 3 positions and separated with a space.
///
/// Input is expected to be valid ASCII. If unicode characters are present that aren't
/// representable as ASCII, these are simple encoded as-is.
///
/// # Example
/// 'ABC' -> '065 066 067'
///
/// # Parameters
/// - `input`: the input to encode (must be valid ASCII)
/// - `input_len`: the amount of characters (excluding any nul-terminator) in `input`
/// - `output`: the buffer to output to. This buffer should have a size of `input_len * 4`.
///   The buffer will be terminated by a nul-terminator
///
/// # Returns
/// `CIPH_OK`
ciph_err_t ciph_ascii(const char* nonnil input, size_t input_len, char* nonnil output);

/// Reverse all words in a text
///
/// Use `u8_check` from libunistring to check if the input is valid UTF-8.
///
/// Punctuation is considered not part of a word and is kept at its original position.
///
/// This function works on grapheme clusters. When reversing, grapheme clusters will be
/// moved in its entirety.
///
/// # Example
/// 'ABC DEF' -> 'CBA FED'
///
/// # Parameters
/// - `input`: the text to reverse encoded as valid unicode UTF-8.
/// - `input_len`: the amount of bytes in `input`
/// - `output`: the output buffer. This buffer should have a size of `input_len`.
///
/// # Returns
/// - `CIPH_OK` on success
/// - `CIPH_ERR_ENCODING` when input contains invalid UTF-8 (might be removed in the future)
ciph_err_t ciph_reverse_words(const uint8_t* nonnil input, size_t input_len, uint8_t* nonnil output);

/// Shift all letters by `shift`
///
/// Use `u8_check` from libunistring to check if the input is valid UTF-8.
///
/// Only letters in the roman alphabet are shifted.
///
/// # Parameters
/// - `input`: the text to shift the letters in
/// - `input_len`: the amount of bytes in input
/// - `shift`: the amount of positions to shift characters relative to the alphabet
/// - `output`: the output buffer. This buffer should have a size of `input_len`. If
///   a NUL-byte is required at the end of the string, then this has to be added manually
///   at offset `input_len`.
///
/// # Returns
/// - `CIPH_OK` on success
/// - `CIPH_ERR_ENCODING` when the input is not valid UTF-8
ciph_err_t ciph_caesar(const uint8_t* nonnil input, size_t input_len, int shift, uint8_t* nonnil output);

//=== Start Alphabet Lookup ===//

/// regular alphabet
extern const uint8_t CIPH_ALPHABET[26];

/// Returns the atbash alphabet
///
/// `buffer` should be 26 bytes
void ciph_alphabet_atbash(uint8_t* nonnil buffer);

typedef enum {
  CIPH_LVAL_OK,

  /// A character is used twice
  CIPH_LVAL_DOUBLE_CHAR,
  /// The word is longer than 26 characters
  CIPH_LVAL_TOO_LONG,
} ciph_lookup_validation_t;

/// Validate a word to be valid for use in vignère cipher
ciph_lookup_validation_t ciph_alphabet_vignere_validate(const uint8_t* nonnil word, size_t word_len);

/// Generate an alphabet for vignère encoding. This alphabet can then be used in
/// `ciph_alphabet_lookup`.
///
/// # Example
/// word = lemon
/// alphabet becomes:
/// LEMONABCDFGHI
/// JKPQRSTUVWXYZ
///
/// So L becomes J, P becomes M, ...
///
/// # Parameters
/// - `word`: a word containing only unique characters and only characters
///    between 'A' to 'Z'. Characters should be uppercased
/// - `word_len`: the amount of bytes in `world` this should be no more than 26
///   and bigger than 0
/// - `buffer`: the buffer in which to store the alphabet lookup, should be 26
///   bytes
/// - `alphabet`: optional. Will write the alphabet as shown in the example
///
/// # Validation
/// In debug builds, the function will crash if the word is invalid. If you want
/// to check user input for the word, then `ciph_alphabet_vignere_validate` can be used.
void ciph_alphabet_vignere(const uint8_t* nonnil word, size_t word_len, uint8_t* nonnil buffer, uint8_t* nilable alphabet);

/// Replace all characters in the input with the lookup values in `lookup`. A
/// will be replaced with lookup[0], B with lookup[1], etc. Lowercase characters
/// are replaced with the lowercased value of the lookup value.
///
/// Use `u8_check` from libunistring to check if the input is valid UTF-8.
///
/// This function operates on codepoints, not on grapheme clusters. This means that
/// à (being a + ◌̀, not à) will be replaced (e.g. if the replacement for A is E,
/// then it will become è). à will NOT be replaced (being a single codepoint rather
/// than a combination of 2). To replace enable replacing characters with diacritics,
/// normalize the input using NFD, then normalize the output again with NFC to replace
/// "character + diacritic" to a single codepoint of character with a diacritic.
/// More info can be found at: https://unicode.org/reports/tr15/#Norm_Forms
///
/// # Parameters
/// - `input`: the text to replace characters in
/// - `input_len`: the amount of bytes in input
/// - `lookup`: The characters to replace. These should be uppercased. The lookup
///   may only contain letters 'A' to 'Z'. The lookup needs to always contain 26
///   characters (bytes)
/// - `output`: the output buffer. This buffer should have a size of `input_len`
///
/// # Returns
/// - `CIPH_OK` on success
ciph_err_t ciph_alphabet_lookup(const uint8_t* nonnil input, size_t input_len, const uint8_t* nonnil lookup, uint8_t* nonnil output);

//=== End Alphabet Lookup ===//

/// Encode a message in morse code using "·" and "—", separated by spaces.
/// The characters used to encode morse can be altered at compile time by
/// defining `CIPH_DIT` and `CIPH_DAH` macros to a string containing the
/// required character.
///
/// Supports more characters than the standard alphabet, as indicated on https://nl.wikipedia.org/wiki/Morse#Het_morsealfabet.
///
/// When grapheme clusters are made up of more than one codepoint,
/// only the first codepoint is encoded.
///
/// # Parameters
/// - `input`: the input text to encode
/// - `input_len`: the amount of bytes in `input`
/// - `output`: the output buffer. The length of this output buffer cannot be known
///   before encoding. A good place to start is `input_len * 4 * 3`. When the output
///   buffer does not contain enough space to encode the input buffer, input_left
///   will be set the point in `input` which has not been encoded yet and `input_len_left`
///   will be set to the amount of bytes left to encode. When this occurs, the morse
///   function can be called again with these two parameters as `input` and `input_len`
///   respectively, after reallocating output to be bigger and advancing it by
///   `boutput_len`. An example of using this function with a dynamically reallocated buffer
///   can be found in the examples (`test_morse_small_buffer`).
/// - `copy_non_encodable_characters`: Non-encodable characters will be copied to
///   the output if `copy_non_encodable_characters` is true. Otherwise they are ignored.
/// - `input_left`: will be set the point in `input` where encoding has stopped,
///   otherwise is set to NULL.
/// - `input_len_left`: the amount of bytes left to encode
/// - `boutput_len`: the amount of bytes in the output buffer that have
///   been written to
///
/// # Returns
/// - `CIPH_OK` on success
/// - `CIPH_ERR_ENCODING` if the input contains invalid UTF-8
ciph_err_t ciph_morse(
  const uint8_t* nonnil input, size_t input_len,
  uint8_t* nonnil output, size_t output_len,
  bool copy_non_encodable_characters,
  const uint8_t* nilable * nilable input_left, size_t* nilable input_len_left,
  size_t* nilable boutput_len
);

#ifdef CIPH_AUDIO
/// Turn morse code into audio
///
/// # Parameters
/// - `morse_code`: The string containing the morse code (only `DIT` and `DAH` are allowed,
///   see `ciph_morse` for more info)
/// - ``
ciph_err_t ciph_morse_to_audio(
  const uint8_t* nonnil morse_code, size_t morse_code_len,
  char* nonnil wave_data, size_t wave_data_len,
  const uint8_t* nilable * nilable morse_code_end, size_t* nilable morse_code_len_left,
  size_t* nilable boutput_len
);
#endif

#ifdef __cplusplus
}
#endif

#endif // include guard
