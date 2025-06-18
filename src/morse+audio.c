#ifdef CIPH_AUDIO

#include <cipher.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include <cipher/internal/utils.h>
#include <string.h>
#include <unicase.h>
#include <unictype.h>
#include <unigbrk.h>
#include <unistd.h>
#include <unistr.h>

#define UC_DIT 46
#define UC_DAH 45
#define UC_SPACE 32
#define UC_WORD_DIVIDER 47

#define MAX(A, B) ((A > B) ? (A) : (B))

// #define SAMPLE_RATE 44100
#define BITRATE 16
#define BYTE_RATE (BITRATE / 8)
#define TONE_FREQ 600//hz --> https://www.reddit.com/r/morse/comments/qyrr0u/standard_tone_frequency_and_speed/

static void gen_tonewave(int sample_rate, double freq, int num_samples, unsigned char* output) {
  double sample[num_samples];
  for (int i = 0; i < num_samples; i++) {
    sample[i] = sin(2 * M_PI * i / (sample_rate / freq));
  }

  // function expects correct samples amount (see comment later)
  assert(num_samples == num_samples - (num_samples % BYTE_RATE));

  int i = 0;
  while (i < num_samples) {
    // scale to max amplitude
    short val = (short)(sample[i] * 32767.0);

    // format: 16-bit WAV PCM (first byte = low order byte)
    output[i++] = (unsigned char) (val & 0x00ff);
    output[i++] = (unsigned char) ((val & 0xff00) >> 8);
  }
}

ciph_err_t ciph_morse_to_audio(
  const unistring_uint8_t* nonnil morse_code, size_t morse_code_len,
  double secs_per_dit,
  int sample_rate,
  unsigned char* nonnil wave_data, size_t wave_data_len,
  const uint8_t* nilable * nilable out_input_end_ptr,
  size_t* nonnil out_output_written
) {
  ciph_err_t err = CIPH_OK;

  const uint8_t* input_ptr = morse_code;
  const uint8_t* input_end = morse_code + morse_code_len;
  int input_left;
  ucs4_t uc;
  int uc_size;
  ucs4_t uc_next;
  ucs4_t uc_prev = 0;

  uint8_t* output_ptr = wave_data;
  const uint8_t* output_end = wave_data + wave_data_len;
  int output_left;

  // const double secs_per_dit = 0.25;
  size_t samples_per_dit = (size_t)round(((double)sample_rate) * secs_per_dit);
  // amount of samples of a specific tone always needs to be divisable by BYTE_RATE,
  // otherwise we will get misaligned samples and things will quickly get painful
  samples_per_dit += (samples_per_dit % BYTE_RATE);
  const int dahs_per_dit = 3;
  // const double secs_per_dah = secs_per_Dit * dahs_per_dit;
  const double secs_silence_between_dits = secs_per_dit;
  size_t samples_silence_between_dits = (size_t)round(((double)sample_rate) * secs_silence_between_dits);
  samples_silence_between_dits += (samples_silence_between_dits % BYTE_RATE);
  const double secs_silence_between_letters = secs_per_dit * 3;
  size_t samples_silence_between_letters = (size_t)round(((double)sample_rate) * secs_silence_between_letters);
  samples_silence_between_letters += (samples_silence_between_letters % BYTE_RATE);
  const double secs_silence_between_words = secs_per_dit * 7;
  size_t samples_silence_between_words = (size_t)round(((double)sample_rate) * secs_silence_between_words);
  samples_silence_between_words += (samples_silence_between_words % BYTE_RATE);

  unsigned char* dit_tone = malloc(samples_per_dit);
  gen_tonewave(sample_rate, TONE_FREQ, samples_per_dit, dit_tone);

  while (input_end != input_ptr) {
    input_left = input_end - input_ptr;
    output_left = output_end - output_ptr;
    assert(output_left > 0);
    uc_size = u8_mbtouc(&uc, input_ptr, input_left);
    if (uc == 0xfffd) {
      err = CIPH_ERR_ENCODING;
      goto cleanup;
    }
    if (input_left - uc_size > 0) {
      u8_mbtouc(&uc_next, input_ptr + uc_size, input_left - uc_size);
    } else {
      uc_next = 0;
    }

    switch (uc) {
      case UC_DIT:
        if (samples_per_dit + samples_silence_between_dits >= output_left) {
          err = CIPH_GROW;
          goto cleanup;
        }

        memcpy(output_ptr, dit_tone, samples_per_dit);
        output_ptr += samples_per_dit;
        if (uc_next == UC_DIT || uc_next == UC_DAH || uc_next == 0) {
          memset(output_ptr, 0, samples_silence_between_dits);
          output_ptr += samples_silence_between_dits;
        }
        break;
      case UC_DAH:
        if (samples_per_dit * dahs_per_dit + samples_silence_between_dits >= output_left) {
          err = CIPH_GROW;
          goto cleanup;
        }

        for (int i = 0; i < dahs_per_dit; i++) {
          memcpy(output_ptr, dit_tone, samples_per_dit);
          output_ptr += samples_per_dit;
        }
        if (uc_next == UC_DIT || uc_next == UC_DAH || uc_next == 0) {
          memset(output_ptr, 0, samples_silence_between_dits);
          output_ptr += samples_silence_between_dits;
        }
        break;
      case UC_SPACE: {
        // if prev was also space or / or next is / -> skip
        if (uc_next != UC_WORD_DIVIDER && uc_prev != UC_SPACE && uc_prev != UC_WORD_DIVIDER) {
          if (samples_silence_between_letters >= output_left) {
            err = CIPH_GROW;
            goto cleanup;
          }

          memset(output_ptr, 0, samples_silence_between_letters);
          output_ptr += samples_silence_between_letters;
        }
      } break;
      case UC_WORD_DIVIDER:
        if (samples_silence_between_words >= output_left) {
          err = CIPH_GROW;
          goto cleanup;
        }

        memset(output_ptr, 0, samples_silence_between_words);
        output_ptr += samples_silence_between_words;
        break;
      default:
        err = CIPH_ERR_MORSE_AUDIO_INVALID_CHAR;
        goto cleanup;
    }

    input_ptr += uc_size;
    uc_prev = uc;
  }

cleanup:
  free(dit_tone);

  *out_output_written = output_ptr - wave_data;
  if (out_input_end_ptr) {
    *out_input_end_ptr = input_ptr;
  }

  return err;
}

#endif
