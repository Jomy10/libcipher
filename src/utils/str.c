#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <cipher/utils.h>
#include <cipher/error.h>
#include <cipher/internal/nil.h>

#ifndef CIPH_STR_DEFAULT_CAP
#define CIPH_STR_DEFAULT_CAP 128
#endif

size_t _max(size_t a, size_t b) {
  return (a > b) ? a : b;
}

ciph_err_t _ciph_str_alloc(ciph_str_t* str, size_t cap) {
  str->data = CIPH_MALLOC(cap);
  if (str->data == NULL) {
    return CIPH_ERR_ALLOC;
  }
  str->cap = cap;
  return CIPH_OK;
}

ciph_str_t ciph_str_create(size_t capacity) {
  ciph_str_t str;
  str.len = 0;
  str.cap = capacity;
  str.data = NULL;
  str.realloc = realloc;
  if (capacity != 0)
    _ciph_str_alloc(&str, capacity);
  return str;
}

ciph_str_t* ciph_str_new(size_t capacity) {
  ciph_str_t* str = CIPH_MALLOC(sizeof(ciph_str_t));
  *str = ciph_str_create(capacity);
  if (str->data == NULL && str->cap > 0) {
    CIPH_FREE(str);
    return NULL;
  }

  return str;
}

void ciph_str_free(ciph_str_t* nonnil str) {
  CIPH_FREE(str->data);
  str->data = NULL;
  str->len = 0;
  str->cap = 0;
}

void ciph_str_delete(ciph_str_t* str) {
  ciph_str_free(str);
  CIPH_FREE(str);
}

void ciph_str_null_encode(ciph_str_t* nonnil str) {
  if (str->len >= str->cap) {
    ciph_str_realloc(str, str->cap + 1);
  }

  str->data[str->len] = '\0';
  str->len += 1;
}

ciph_err_t ciph_str_realloc(ciph_str_t* nonnil str, size_t new_cap) {
  void* new_data = str->realloc(str->data, new_cap);
  if (new_data == NULL) {
    return CIPH_ERR_ALLOC;
  }
  str->data = (uint8_t*) new_data;
  str->cap = new_cap;
  return CIPH_OK;
}

ciph_err_t ciph_str_push_char(ciph_str_t* nonnil str, char c) {
  if (str->cap == 0)
    if (_ciph_str_alloc(str, CIPH_STR_DEFAULT_CAP) != CIPH_OK) return CIPH_ERR_ALLOC;

  assert(str != NULL);
  assert(str->data != NULL);

  if (str->len >= str->cap) {
    if (ciph_str_realloc(str, str->cap * 2) != CIPH_OK) {
      return CIPH_ERR_ALLOC;
    }
  }
  str->data[str->len] = c;
  str->len += 1;

  return CIPH_OK;
}

ciph_err_t ciph_str_push_str(ciph_str_t* nonnil str, const uint8_t* nonnil strb, size_t len) {
  if (str->cap == 0)
    if (_ciph_str_alloc(str, CIPH_STR_DEFAULT_CAP) != CIPH_OK) return CIPH_ERR_ALLOC;

  assert(str != NULL);
  assert(str->data != NULL);

  if (str->len + len >= str->cap) {
    if (ciph_str_realloc(str, _max(str->cap == 0 ? CIPH_STR_DEFAULT_CAP : str->cap * 2, str->len + len)) != CIPH_OK) {
      return CIPH_ERR_ALLOC;
    }
  }
  memcpy(str->data + str->len, strb, len);
  str->len += len;

  return CIPH_OK;
}

uint8_t ciph_str_pop(ciph_str_t* str) {
  if (str->len == 0) return 0;
  str->len -= 1;
  return str->data[str->len];
}

uint8_t* ciph_str_popn(ciph_str_t* str, size_t n) {
  if (str->len < n) return NULL;
  str->len -= n;
  return str->data + str->len;
}
