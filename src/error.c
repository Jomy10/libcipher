#include <cipher.h>

char* ciph_strerror(ciph_err_t err) {
  switch (err) {
    case CIPH_OK: return "OK";
    case CIPH_GROW: return "Output buffer too small";
    case CIPH_ERR_ENCODING: return "Input is not UTF-8 encoded";
    case CIPH_ERR_YEAR_DIGITS: return "The amount of digits in the `year` variable is not 4";
    case CIPH_ERR_MORSE_AUDIO_INVALID_CHAR: return "Invalid character found in morse input";
  }
}
