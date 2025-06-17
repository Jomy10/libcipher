#ifndef _CIPH_ERR_H
#define _CIPH_ERR_H

typedef enum {
  CIPH_OK = 0,

  /// Need more memory in the output buffer
  CIPH_GROW,

  /// The input had an invalid encoding
  CIPH_ERR_ENCODING,

  /// An error in the digits;
  /// - all digits are 0
  CIPH_ERR_YEAR_DIGITS,

  /// An invalid character was found in the input
  CIPH_ERR_MORSE_AUDIO_INVALID_CHAR
} ciph_err_t;

#ifdef __cplusplus
extern "C" {
#endif

char* ciph_strerr(ciph_err_t err);

#ifdef __cplusplus
}
#endif

#endif
