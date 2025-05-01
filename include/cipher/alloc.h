// Versions of the functions in ciphers.h, but which handle allocations
// for you. (This doesn't mean the functions in ciphers.h don't do any allocations,
// it just means they don't return anything you have to allocate)

#ifndef _CIPH_ALLOC_H
#define _CIPH_ALLOC_H

#include "cipher/ciphers.h"
#ifndef CIPH_NO_ALLOCS

#include <stddef.h>
#include <stdbool.h>
#include "internal/defines.h"
#include "internal/nil.h"
#include "error.h"
#include <unitypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/// This version of morse simply calls `ciph_morse`, but sets `*output` to a pointer
/// that needs to be freed by the user.
EXPORT ciph_err_t ciph_alloc_morse(
  const uint8_t* nonnil input, size_t input_len,
  bool copy_non_encodable_characters,
  uint8_t* nilable * nonnil output,
  size_t* nilable output_len
);

EXPORT ciph_err_t ciph_alloc_numbers(
  const uint8_t* nonnil input, size_t input_len,
  bool copy_non_encodable_characters,
  uint8_t* nilable * nonnil output,
  size_t* nilable output_len
);

EXPORT ciph_err_t ciph_alloc_block_method(
  const uint8_t* nonnil input, size_t input_len,
  uint8_t* nilable * nonnil output,
  size_t* nilable output_len
);

EXPORT ciph_err_t ciph_alloc_char_alph_sub(
  const uint8_t* nonnil input, size_t input_len,
  const ciph_SubAlphabetElement_t* nonnil substitution_alphabet,
  const uint8_t* nilable char_sep, size_t char_sep_len,
  const uint8_t* nilable word_sep, size_t word_sep_len,
  const uint8_t* nilable sentence_sep, size_t sentence_sep_len,
  bool copy_non_encodable_characters,
  uint8_t* nilable * nonnil output, size_t* nonnil output_len
);

EXPORT ciph_err_t ciph_alloc_year(
  const uint8_t* nonnil input, size_t input_len,
  uint8_t year[nonnil 4],
  uint32_t char_include_bitmask,
  uint8_t* nilable * nonnil output, size_t* nonnil output_len
);

#endif

#ifdef __cplusplus
}
#endif

#endif
