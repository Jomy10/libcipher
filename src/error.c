#include <cipher.h>

char* ciph_strerror(ciph_err_t err) {
  switch (err) {
    case CIPH_OK: return "OK";
    case CIPH_GROW: return "Output buffer too small";
    case CIPH_ERR_ENCODING: return "Input is not UTF-8 encoded";
    case CIPH_ERR_YEAR_DIGITS: return "The emount of digits in the `year` variable is not 4";
  }
}
