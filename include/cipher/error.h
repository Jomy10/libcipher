#ifndef _CIPH_ERR_H
#define _CIPH_ERR_H

typedef enum {
  CIPH_OK = 0,

  /// The input had an invalid encoding
  CIPH_ERR_ENCODING,
} ciph_err_t;

#ifdef __cplusplus
extern "C" {
#endif

char* ciph_strerr(ciph_err_t err);

#ifdef __cplusplus
}
#endif

#endif
