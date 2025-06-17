#include <cipher.h>
#include <stdlib.h>

EXPORT ciph_err_t ciph_alloc_morse_to_audio(
  const uint8_t* nonnil morse_code, size_t morse_code_len,
  double secs_per_dit,
  unsigned char* nonnil * nonnil wave_data,
  size_t* nonnil wave_data_len
) {
  size_t wave_data_cap = (morse_code_len / 2) * 44100;
  *wave_data = malloc(wave_data_cap);
  uint8_t* output_ptr = *wave_data;
  const uint8_t* input_ptr = morse_code;
  const uint8_t* input_end = morse_code + morse_code_len;

  ciph_err_t err;

  while (true) {
    err = ciph_morse_to_audio(
      input_ptr, input_end - input_ptr,
      secs_per_dit,
      output_ptr, wave_data_cap + (output_ptr - *wave_data),
      &input_ptr,
      &output_ptr
    );

    if (err != CIPH_OK) return err;

    if (input_ptr != input_end) {
      wave_data_cap *= 2;
      *wave_data = realloc(*wave_data, wave_data_cap);
    } else {
      break;
    }
  }

  *wave_data_len = (input_ptr - *wave_data);

  return err;
}
