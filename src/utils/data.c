#include <string.h>

#include <cipher/utils.h>
#include <cipher/error.h>

ciph_err_t _ciph_str_alloc(ciph_str_t* str, size_t cap);
size_t _max(size_t a, size_t b);

ciph_err_t ciph_data_set_byte(ciph_data_t* self, unsigned char byte, size_t n) {
  if (self->cap == 0)
    if (_ciph_str_alloc(self, n) != CIPH_OK) return CIPH_ERR_ALLOC;

  if (self->len + n > self->cap) {
    if (ciph_str_realloc(self, _max(self->cap * 2, self->len + n)) != CIPH_OK) return CIPH_ERR_ALLOC;
  }

  memset(self->data + self->len, byte, n);
  self->len += n;

  return CIPH_OK;
}
