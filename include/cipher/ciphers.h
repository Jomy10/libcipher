#include <stddef.h>
#include <unitypes.h>
#include "error.h"
#include "internal/nil.h"

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
/// # Example
/// 'ABC DEF' -> 'CBA FED'
///
/// # Parameters
/// - `input`: the text to reverse encoded as valid unicode UTF-8.
/// - `input_len`: the amount of bytes in `input`
/// - `output`: the output buffer. This buffer should have a size of `input_len + 1`.
///   The last byte will be a nul-terminator.
///
/// # Returns
///
ciph_err_t ciph_reverse_words(const uint8_t* nonnil input, size_t input_len, uint8_t* nonnil output);
