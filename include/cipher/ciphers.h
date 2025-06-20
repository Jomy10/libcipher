#ifndef _CIPH_H
#define _CIPH_H

#include <stddef.h>
#include <stdbool.h>
#include <unitypes.h>
#include <unictype.h>
#include "error.h"
#include "internal/nil.h"
#include "internal/defines.h"

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
/// - `output`: the buffer to output to. This buffer should have a size of `input_len * 4 - 1`.
///
/// # Returns
/// `CIPH_OK`
EXPORT ciph_err_t ciph_ascii(const char* nonnil input, size_t input_len, char* nonnil output);

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
EXPORT ciph_err_t ciph_reverse_words(const uint8_t* nonnil input, size_t input_len, uint8_t* nonnil output);

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
EXPORT ciph_err_t ciph_caesar(const uint8_t* nonnil input, size_t input_len, int shift, uint8_t* nonnil output);

//=== Start Alphabet Lookup ===//

/// regular alphabet
extern const uint8_t CIPH_ALPHABET[26];

/// Returns the atbash alphabet
///
/// `buffer` should be 26 bytes
EXPORT void ciph_alphabet_atbash(uint8_t* nonnil buffer);

typedef enum {
  CIPH_LVAL_OK,

  /// A character is used twice
  CIPH_LVAL_DOUBLE_CHAR,
  /// The word is longer than 26 characters
  CIPH_LVAL_TOO_LONG,
} ciph_lookup_validation_t;

/// Validate a word to be valid for use in vignère cipher
EXPORT ciph_lookup_validation_t ciph_alphabet_vignere_validate(const uint8_t* nonnil word, size_t word_len);

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
/// - `alphabet`: optional. Will write the alphabet as shown in the example. Should
///   be 26 bytes.
///
/// # Validation
/// In debug builds, the function will crash if the word is invalid. If you want
/// to check user input for the word, then `ciph_alphabet_vignere_validate` can be used.
EXPORT void ciph_alphabet_vignere(const uint8_t* nonnil word, size_t word_len, uint8_t* nonnil buffer, uint8_t* nilable alphabet);

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
EXPORT ciph_err_t ciph_alphabet_lookup(const uint8_t* nonnil input, size_t input_len, const uint8_t* nonnil lookup, uint8_t* nonnil output);

//=== End Alphabet Lookup ===//

/// Encode a message in morse code using "·" and "-", separated by spaces.
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
///   before encoding. A good place to start is `input_len * 4`. When the output
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
EXPORT ciph_err_t ciph_morse(
  const uint8_t* nonnil input, size_t input_len,
  uint8_t* nonnil output, size_t output_len,
  bool copy_non_encodable_characters,
  const uint8_t* nilable * nilable input_left, size_t* nilable input_len_left,
  size_t* nilable boutput_len
);

#ifdef CIPH_AUDIO
/// Turn morse code into audio.
///
/// The output is 16 bits mono
///
/// # Parameters
/// - `morse_code`: The string containing the morse code (only `DIT`, `DAH` and the standard separators ' ' and '/' are allowed,
///   see `ciph_morse` for more info)
/// - `morse_code_len`: the length, in bytes, of `morse_code`
/// - `secs_per_dit`: the amount of seconds one dit lasts. 0.25 is a good default
/// - `sample_rate`: the output sample rate
/// - `wave_data`: the output raw wave data
/// - `wave_data_len`: the amount of bytes available in `wave_data`
/// - `out_input_end_ptr`: the ptr in of `morse_code` where encoding stopped,
///   is equal to `morse_code + morse_code_len` if encoding ended
/// - `out_output_written`: the amount of bytes written to the output buffer
EXPORT ciph_err_t ciph_morse_to_audio(
  const uint8_t* nonnil morse_code, size_t morse_code_len,
  double secs_per_dit, int sample_rate,
  unsigned char* nonnil wave_data, size_t wave_data_len,
  const uint8_t* nilable * nilable out_input_end_ptr,
  size_t* nonnil out_output_written
);
#endif

/// Substitute letters by their corresponding number in the alphabet
///
/// # Parameters
/// - `input`: the input text to encode
/// - `input_len`: the amount of bytes in `input`
/// - `output`: the buffer in which to output.
/// - `output_len`: the amount of bytes available in the output buffer
/// - `copy_non_encodable_characters`: Non-encodable characters will be copied to
///   the output if `copy_non_encodable_characters` is true. Otherwise they are ignored.
/// - `input_left`: will be set the point in `input` where encoding has stopped,
///   otherwise is set to NULL.
/// - `input_len_left`: the amount of bytes left to encode
/// - `boutput_len`: the amount of bytes in the output buffer that have
///   been written to
///
/// # Returns
/// - `CIPH_OK`
/// - `CIPH_ERR_ENCODING`: if input is invalid UTF-8
EXPORT ciph_err_t ciph_numbers(
  const uint8_t* nonnil input, size_t input_len,
  uint8_t* nonnil output, size_t output_len,
  bool copy_non_encodable_characters,
  const uint8_t* nilable * nilable input_left, size_t* nilable input_len_left,
  size_t* nilable boutput_len
);

/// Transforms words into blocks and reads them column by column.
///
/// This function works on grapheme clusters. A grapheme cluster
/// will be seen as one letter in the schema below. (e.g. P)
///
/// # Example
/// word: Pionierhout
/// becomes:
///   PION
///   IERH
///   OUTX
///   XXXX
/// encoded: PIOXIEUXORTXNHXX
///
/// # Parameters
/// - `input`: the input text to encode
/// - `input_len`: the amount of bytes in `input`
/// - `output`: the buffer in which to output.
/// - `output_len`: the amount of bytes available in the output buffer
/// - `input_left`: will be set the point in `input` where encoding has stopped,
///   otherwise is set to NULL.
/// - `input_len_left`: the amount of bytes left to encode
/// - `out_output_len`: the amount of bytes in the output buffer that have
///   been written to
///
/// # Returns
/// - `CIPH_OK`
/// - `CIPH_ERR_ENCODING`: if input is invalid UTF-8
EXPORT ciph_err_t ciph_block_method(
  const uint8_t* nonnil input, size_t input_len,
  uint8_t* nonnil output, size_t output_len,
  const uint8_t* nilable * nilable input_left, size_t* nilable input_len_left,
  size_t* nilable out_output_len
);

typedef struct {
  const uint8_t* nonnil sub;
  size_t len;
} ciph_SubAlphabetElement_t;

/// A substitution cipher. Like alphabet lookup, but supports substituting one
/// character by many other characters.
///
/// This version of the substitution separates characters and words by a separator.
///
/// # Parameters
/// - `input`: the input text to encode
/// - `input_len`: the amount of bytes in `input`
/// - `output`: the buffer in which to output.
/// - `output_len`: the amount of bytes available in the output buffer
/// - `substituion_alphabet`: the characters which will replace the characters
///   in the alphabet (from A to Z). This buffer should have 26 elements.
/// - `char_sep`: the separator to use between characters
/// - `char_sep_len`: the length of `char_sep` in bytes
/// - `word_sep`: the separator to use between words
/// - `word_sep_len`: the length of `word_sep` in bytes
/// - `sentence_sep`: the separator to use between sentences.
/// - `sentence_sep_len`: the length of `sentence_sep` in bytes
/// - `copy_non_encodable_characters`: wether to copy characters that aren't encoded
///   and don't constiute a word or sentence break.
/// - `out_input_left`: will be set the point in `input` where encoding has stopped,
///   otherwise is set to NULL.
/// - `out_input_len_left`: the amount of bytes left to encode
/// - `out_output_len`: the amount of bytes in the output buffer that have
///   been written to
///
/// # Returns
/// - `CIPH_OK`
/// - `CIPH_ERR_ENCODING`: if input is invalid UTF-8
EXPORT ciph_err_t ciph_char_alph_sub(
  const uint8_t* nonnil input, size_t input_len,
  uint8_t* nonnil output, size_t output_len,

  const ciph_SubAlphabetElement_t* nonnil substitution_alphabet,

  const uint8_t* nilable char_sep, size_t char_sep_len,
  const uint8_t* nilable word_sep, size_t word_sep_len,
  const uint8_t* nilable sentence_sep, size_t sentence_sep_len,

  bool copy_non_encodable_characters,

  const uint8_t* nilable * nilable out_input_left, size_t* nilable out_input_len_left,
  size_t* nilable out_output_len
);

typedef struct {
  /// Wether the input has been fully parsed into year components
  bool completed;

  struct _ciph_YearComponent* nilable comps;
  int comps_len;
  int max_char_len;

  int char_idx;
  int comp_idx;
} ciph_year_ires_t;

#define CIPH_YEAR_INCLUDE_MASK_LETTERS UC_CATEGORY_MASK_L
#define CIPH_YEAR_INCLUDE_MASK_LETTERS_AND_NUMBERS UC_CATEGORY_MASK_L | UC_CATEGORY_MASK_N
#define CIPH_YEAR_INCLUDE_MASK_WITH_SYMBOLS UC_CATEGORY_MASK_L | UC_CATEGORY_MASK_N | UC_CATEGORY_MASK_Sm
#define CIPH_YEAR_INCLUDE_MASK_WITH_SYMBOLS_AND_DASHES UC_CATEGORY_MASK_L | UC_CATEGORY_MASK_N | UC_CATEGORY_MASK_Pd | UC_CATEGORY_MASK_Sm

#ifdef __EMSCRIPTEN__
EXPORT uint32_t ciph_year_include_mask_letters();
EXPORT uint32_t ciph_year_include_mask_letters_and_numbers();
EXPORT uint32_t ciph_year_include_mask_with_symbols();
EXPORT uint32_t ciph_year_include_mask_with_symbols_and_dashes();
#endif

/// A cipher where the code is a year (4 digit number).
///
/// # Parameters
/// - `input`: the input text to encode
/// - `input_len`: the amount of bytes in `input`
/// - `output`: the buffer in which to output.
/// - `output_len`: the amount of bytes available in the output buffer
/// - `year`: a 4 digit number
/// - `ir`: the intermediate result. As long is the output has not been completely
///   written out (return CIPH_GROW), this memory will not be freed. When CIPH_OK
///   (encoding has finished), or an error (!= CIPH_GROW) is returned, `ir` will
///   be freed.
///   At least `completed` variable needs to be set to 0 (false)
/// - `out_output_written`: the amount of bytes in the output buffer that have
///   been written to
///
/// # Returns
/// - `CIPH_OK`
/// - `CIPH_ERR_ENCODING`: when the input is invalid UTF-8
EXPORT ciph_err_t ciph_year(
  const uint8_t* nonnil input, size_t input_len,
  uint8_t year[nonnil 4],
  uint32_t char_include_bitmask,
  uint8_t* nonnil output, size_t output_len,
  ciph_year_ires_t* nonnil ir,
  size_t* nonnil out_output_written
);

#ifdef __cplusplus
}
#endif

#endif // include guard
