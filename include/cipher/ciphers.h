#ifndef _CIPH_H
#define _CIPH_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <unictype.h>

#include "internal/defines.h"
#include "internal/nil.h"
#include "error.h"
#include "utils.h"

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
///   This is an ASCII-encoded string.
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
///   It is UTF-8 encoded.
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
///   at offset `input_len`. The output string is UTF-8 encoded.
///
/// # Returns
/// - `CIPH_OK` on success
/// - `CIPH_ERR_ENCODING` when the input is not valid UTF-8
EXPORT ciph_err_t ciph_caesar(const uint8_t* nonnil input, size_t input_len, int shift, uint8_t* nonnil output);

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
/// - `output`: The output buffer. A good capacity to start with is `input_len * 4`.
//    The buffer is UTF-8 encoded and not NULL-terminated. It can be using `ciph_str_null_encode`.
/// - `copy_non_encodable_characters`: Non-encodable characters will be copied to
///   the output if `copy_non_encodable_characters` is true. Otherwise they are ignored.
///
/// # Returns
/// - `CIPH_OK` on success
/// - `CIPH_ERR_ENCODING` if the input contains invalid UTF-8
/// - `CIPH_ERR_ALLOC`: when there was an error reallocating the output buffer
EXPORT ciph_err_t ciph_morse(
  const uint8_t* nonnil input, size_t input_len,
  ciph_str_t* nonnil output,
  bool copy_non_encodable_characters
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
///
/// # Returns
/// - `CIPH_OK`
/// - `CIPH_ERR_MORSE_AUDIO_INVALID_CHAR`: if the input contains an unexpected character
/// - `CIPH_ERR_ALLOC`: when there was an error reallocating the output buffer
EXPORT ciph_err_t ciph_morse_to_audio(
  const uint8_t* nonnil morse_code, size_t morse_code_len,
  double secs_per_dit, int sample_rate,
  ciph_data_t* nonnil wave_data
);
#endif

/// Substitute letters by their corresponding number in the alphabet.
///
/// # Note on accents on letters
/// Substituting à for 1 can be done by normalizing using NFD and setting
/// `copy_non_encodable_characters` to false. Otherwise, to keep à as-is,
/// normalize using NFC and set `copy_non_encodable_characters` to true.
///
/// # Parameters
/// - `input`: the input text to encode
/// - `input_len`: the amount of bytes in `input`
/// - `output`: the buffer in which to output.
/// - `output_len`: the amount of bytes available in the output buffer
/// - `copy_non_encodable_characters`: Non-encodable characters will be copied to
///   the output if `copy_non_encodable_characters` is true. Otherwise they are ignored.
///
/// # Returns
/// - `CIPH_OK`
/// - `CIPH_ERR_ENCODING`: if input is invalid UTF-8
/// - `CIPH_ERR_ALLOC`: when there was an error reallocating the output buffer
EXPORT ciph_err_t ciph_numbers(
  const uint8_t* nonnil input, size_t input_len,
  ciph_str_t* nonnil output,
  bool copy_non_encodable_characters
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
///   XXX
/// encoded: PIOXIEUXORTXNHXX
///
/// # Parameters
/// - `input`: the input text to encode
/// - `input_len`: the amount of bytes in `input`
/// - `output`: the buffer in which to output.
///
/// # Returns
/// - `CIPH_OK`
/// - `CIPH_ERR_ENCODING`: if input is invalid UTF-8
/// - `CIPH_ERR_ALLOC`: when there was an error reallocating the output buffer
EXPORT ciph_err_t ciph_block_method(
  const uint8_t* nonnil input, size_t input_len,
  ciph_str_t* nonnil output
);

/// A cipher where the code is a year (4 digit number).
///
/// Example:
/// code = 1996
/// text = hello world how are you today
///
/// Text is placed in rows with the number of letters being the letter of the year and filled with X.
/// 1 h
/// 9 e l l o w o r l d
/// 9 h o w a r e y o u
/// 6 t o d a y X
///
/// Now it is read from top to bottom.
/// Result = heht loo lwd oaa wry oeX ry lo du
///
/// # Parameters
/// - `input`: the input text to encode
/// - `input_len`: the amount of bytes in `input`
/// - `year`: a 4 digit number (e.g. { 1, 9, 9, 6 })
/// - `char_include_mask`: the types of characters to include in the output (see `CIPH_CHAR_INCLUDE_XXX`).
/// - `output`: the buffer in which to output.
///
/// # Returns
/// - `CIPH_OK`
/// - `CIPH_ERR_ENCODING`: when the input is invalid UTF-8
/// - `CIPH_ERR_ALLOC`: when there was an error reallocating the output buffer
EXPORT ciph_err_t ciph_year(
  const uint8_t* nonnil input, size_t input_len,
  uint8_t year[nonnil 4],
  uint32_t char_include_mask,
  ciph_str_t* nonnil output
);

typedef struct {
  const uint8_t* nonnil input;
  size_t input_len;
  const uint8_t* nonnil substitution;
  size_t substitution_len;
} ciph_sub_entry_t;

typedef void* nonnil ciph_sub_t;

/// Parse a list of substitutions to be used in `ciph_sub`.
///
/// For substitution to work properly, both substitution and input should
/// be normalized the same way.
///
/// # Parameters
/// - `entries`: an ordered list of substitutions.
/// - `entries_len`: the amount of substitutions in `entries`
/// - `sub`: the output parsed substitution to be used in `ciph_sub`
///
/// # Returns
/// - `CIPH_OK`
/// - `CIPH_ERR_ALLOC`: when there was an error reallocating the output buffer
EXPORT ciph_err_t ciph_sub_parse(
  const ciph_sub_entry_t* nonnil entries, size_t entries_len,
  ciph_sub_t* nonnil sub
);

/// Free memory allocated by `ciph_sub_parse`
void ciph_sub_free(ciph_sub_t* nonnil);

/// Substitution cipher
///
/// # Parameters
/// - `input`: the input text
/// - `input_len`: the length of the input text
/// - `sub`: the substitutions. Can be obtained from a list of substitution by
///   calling `ciph_sub_parse`.
/// - `output`: the buffer in which to output
///
/// # Returns
/// - `CIPH_OK`
/// - `CIPH_ERR_ENCODING`: when the input is invalid UTF-8
/// - `CIPH_ERR_ALLOC`: when there was an error reallocating the output buffer
EXPORT ciph_err_t ciph_sub(
  const uint8_t* nonnil input, size_t input_len,
  ciph_sub_t nonnil sub,
  ciph_str_t* nonnil ouput
);

// Substitution cipher
// TODO: list of substitutions (highest = higher priority)
// Check all inputs on the current position -> replace if match found -> advance past the input
// Should create a tree-like structure. If input is A -> go to A start -> next character -> follow tree
// until end node

/// The standard alphabet (in uppercase letters)
extern const uint8_t CIPH_ALPHABET[26];

/// Returns the atbash alphabet to be used in `ciph_alphabet_lookup` as `lookup` parameter.
///
/// # Parameters
/// - `buffer`: should be 26 bytes.
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
/// `ciph_alphabet_lookup` as the `alphabet` parameter.
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
/// than a combination of 2). To enable replacing characters with diacritics,
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

// Define a set a characters to include in output of certain functions
#define CIPH_CHAR_INCLUDE_LETTERS UC_CATEGORY_MASK_L
#define CIPH_CHAR_INCLUDE_NUMBERS UC_CATEGORY_MASK_N
#define CIPH_CHAR_INCLUDE_SYMBOLS UC_CATEGORY_MASK_Sm
#define CIPH_CHAR_INCLUDE_DASHES  UC_CATEGORY_MASK_Pd

#ifdef __cplusplus
}
#endif

#endif
