#ifndef _CIPH_ERR_H
#define _CIPH_ERR_H

typedef enum {
  CIPH_OK = 0,

  /// The input had an invalid encoding
  CIPH_ERR_ENCODING,
} ciph_err_t;

char* ciph_strerr(ciph_err_t err);

#endif
