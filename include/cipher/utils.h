#ifndef _CIPH_UTILS_H
#define _CIPH_UTILS_H

#include <stddef.h>
#include <stdint.h>

#include "error.h"
#include "internal/nil.h"

#ifdef __cplusplus
extern "C" {
#endif

/// A growable UTF-8 string.
typedef struct {
  uint8_t* nilable data;
  size_t cap;
  size_t len;
  void* nilable(* nonnil realloc)(void* nonnil, size_t);
} ciph_str_t;

/// Create an empty string with a certain capacity.
/// When capacity is 0 or allocation fails, `data` will be NULL.
EXPORT ciph_str_t ciph_str_create(size_t capacity);

/// Same as `ciph_str_create`, but alloces the return type too.
EXPORT ciph_str_t* nilable ciph_str_new(size_t capacity);

EXPORT void ciph_str_free(ciph_str_t* nonnil);

/// Free string from `ciph_str_new`.
EXPORT void ciph_str_delete(ciph_str_t* nonnil);

EXPORT void ciph_str_null_encode(ciph_str_t* nonnil str);

EXPORT ciph_err_t ciph_str_realloc(ciph_str_t* nonnil str, size_t new_cap);

EXPORT ciph_err_t ciph_str_push_char(ciph_str_t* nonnil str, char c);

EXPORT ciph_err_t ciph_str_push_str(ciph_str_t* nonnil str, const uint8_t* nonnil strb, size_t len);

/// Returns the character popped.
EXPORT uint8_t ciph_str_pop(ciph_str_t* nonnil str);

/// Returns the pointer just after the new length of the string.
/// Note that editing the str will invalidate this string.
EXPORT uint8_t* nilable ciph_str_popn(ciph_str_t* nonnil str, size_t n);

#define ciph_data_t ciph_str_t
#define ciph_data_create ciph_str_create
#define ciph_data_realloc ciph_str_realloc
#define ciph_data_push_byte ciph_str_push_char
#define ciph_data_push_bytes ciph_str_push_str

/// Add byte `byte` to the end of `self` `n` times.
EXPORT ciph_err_t ciph_data_set_byte(ciph_data_t* nonnil self, unsigned char byte, size_t n);

#ifdef __cplusplus
}
#endif


#endif
