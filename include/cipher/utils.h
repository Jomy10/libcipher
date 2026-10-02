#ifndef _CIPH_UTILS_H
#define _CIPH_UTILS_H

#include <stddef.h>
#include <stdint.h>

#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

/// A growable UTF-8 string.
typedef struct {
  uint8_t* data;
  size_t cap;
  size_t len;
  void*(*realloc)(void*, size_t);
} ciph_str_t;

/// Create an empty string with a certain capacity.
/// When capacity is 0 or allocation fails, `data` will be NULL.
ciph_str_t ciph_str_create(size_t capacity);

void ciph_str_free(ciph_str_t*);

void ciph_str_null_encode(ciph_str_t* str);

ciph_err_t ciph_str_realloc(ciph_str_t* str, size_t new_cap);

ciph_err_t ciph_str_push_char(ciph_str_t* str, char c);

ciph_err_t ciph_str_push_str(ciph_str_t* str, const uint8_t* strb, size_t len);

/// Returns the character popped.
uint8_t ciph_str_pop(ciph_str_t* str);

/// Returns the pointer just after the new length of the string.
/// Note that editing the str will invalidate this string.
uint8_t* ciph_str_popn(ciph_str_t* str, size_t n);

#define ciph_data_t ciph_str_t
#define ciph_data_create ciph_str_create
#define ciph_data_realloc ciph_str_realloc
#define ciph_data_push_byte ciph_str_push_char
#define ciph_data_push_bytes ciph_str_push_str

/// Add byte `byte` to the end of `self` `n` times.
ciph_err_t ciph_data_set_byte(ciph_data_t* self, unsigned char byte, size_t n);

#ifdef __cplusplus
}
#endif


#endif
