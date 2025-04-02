#include <cipher.h>

char* ciph_strerror(ciph_err_t err) {
  switch (err) {
    case CIPH_OK: return "OK";
    case CIPH_ERR_ENCODING: return "Input is not UTF-8 encoded";
  }
}
