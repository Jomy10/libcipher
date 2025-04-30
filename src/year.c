#include <cipher.h>
#include <stdlib.h>
#include <string.h>
#include <unitypes.h>
#include <unistr.h>
#include <unigbrk.h>
#include <unistd.h>
#include <unictype.h>

#define MAX(a, b) ((a > b) ? (a) : (b))

struct _ciph_yc_chars {
  const uint8_t* nonnil start;
  int len;
};

struct _ciph_YearComponent {
  int nchars;
  struct _ciph_yc_chars* nonnil chars;
};

#ifndef NDEBUG
  #include <stdio.h>

  #define CIPH_PRINT_YEAR(res) _ciph_print_year(res)

  static void _ciph_print_year(ciph_year_ires_t* res) {
    for (int i = 0; i < res->comps_len; i++) {
      printf("%d ", res->comps[i].nchars);
      for (int j = 0; j < res->comps[i].nchars; j++) {
        printf("%.*s", res->comps[i].chars[j].len, res->comps[i].chars[j].start);
      }
      printf("\n");
    }
  }
#else
  #define CIPH_PRINT_YEAR(res)
#endif

ciph_err_t ciph_year(
  const uint8_t* nonnil input, size_t input_len,
  uint8_t year[4],
  uint32_t char_include_bitmask,
  uint8_t* nonnil output, size_t output_len,
  ciph_year_ires_t* nonnil ir,
  size_t* nonnil out_output_written
) {
  if (ir->completed) goto PROCESS_IR;

  const uint8_t* input_ptr = input;
  const uint8_t* input_end = input + input_len;

  const uint8_t* next;
  int grapheme_len;

  if (year[0] == 0 && year[1] == 0 && year[2] == 0 && year[3] == 0) {
    return CIPH_ERR_YEAR_DIGITS;
  }
  int year_idx = 0;

  int comps_cap = MAX(4, input_len / 6);
  int comps_idx = 0; // index for single year component
  int comps_char_idx = 0; // idx inside of year component

  struct _ciph_YearComponent* comps = malloc(comps_cap * sizeof(struct _ciph_YearComponent));

  comps[comps_idx].nchars = year[year_idx];
  year_idx += 1;
  if (comps[comps_idx].nchars != 0) comps[comps_idx].chars = calloc(comps[comps_idx].nchars, sizeof(struct _ciph_yc_chars));

  while (comps[comps_idx].nchars == 0) {
    comps_idx += 1; // should never go out of bounds because we set the comps cap to at least 4 and checked that not all of the components are 0
    comps[comps_idx].nchars = year[year_idx];
    year_idx = (year_idx + 1) % 4;
    if (comps[comps_idx].nchars != 0) comps[comps_idx].chars = calloc(comps[comps_idx].nchars, sizeof(struct _ciph_yc_chars));
  }

  ucs4_t codepoint;
  while (true) {
    next = u8_grapheme_next(input_ptr, input_end);
    if (next == NULL) break;
    grapheme_len = next - input_ptr;

    u8_mbtouc(&codepoint, input_ptr, grapheme_len);
    if (codepoint == 0xfffd) return CIPH_ERR_ENCODING;
    if (!uc_is_general_category_withtable(codepoint, char_include_bitmask)) {
      goto NEXT_CHAR;
    }

    comps[comps_idx].chars[comps_char_idx].start = input_ptr;
    comps[comps_idx].chars[comps_char_idx].len = grapheme_len;
    comps_char_idx += 1;
    if (comps_char_idx == comps[comps_idx].nchars && next != input_end) {
      comps_char_idx = 0;
      comps_idx += 1;
      if (comps_idx == comps_cap) {
        comps_cap *= 2;
        comps = realloc(comps, comps_cap * sizeof(struct _ciph_YearComponent));
      }
      comps[comps_idx].nchars = year[year_idx];
      year_idx = (year_idx + 1) % 4;
      if (comps[comps_idx].nchars != 0) comps[comps_idx].chars = calloc(comps[comps_idx].nchars, sizeof(struct _ciph_yc_chars));

      while (comps[comps_idx].nchars == 0) {
        comps_idx += 1;
        if (comps_idx == comps_cap) {
          comps_cap *= 2;
          comps = realloc(comps, comps_cap * sizeof(struct _ciph_YearComponent));
        }
        comps[comps_idx].nchars = year[year_idx];
        year_idx = (year_idx + 1) % 4;
        if (comps[comps_idx].nchars != 0) comps[comps_idx].chars = calloc(comps[comps_idx].nchars, sizeof(struct _ciph_yc_chars));
      }
    }

  NEXT_CHAR:
    input_ptr = next;
  }

  ir->completed = true;
  ir->max_char_len = MAX(MAX(year[0], year[1]), MAX(year[2], year[3]));
  ir->comps = comps;
  ir->comps_len = comps_idx + 1;
  ir->char_idx = 0;
  ir->comp_idx = 0;

PROCESS_IR:
  {
    uint8_t* output_ptr = output;
    uint8_t* output_end = output + output_len;
    for (; ir->char_idx < ir->max_char_len; ir->char_idx++) {
      if (ir->comp_idx >= ir->comps_len) {
        // add ' ' after each column read
        if (output_ptr == output_end && ir->char_idx + 1 != ir->max_char_len) goto NEED_MORE_ALLOC;
        *output_ptr = ' ';
        output_ptr += 1;

        ir->comp_idx = 0;
      }

      for (; ir->comp_idx < ir->comps_len; ir->comp_idx++) {
        if (ir->char_idx >= comps[ir->comp_idx].nchars) continue;

        int len = ir->comps[ir->comp_idx].chars[ir->char_idx].len;
        if (len == 0) {
          if (output_ptr == output_end) goto NEED_MORE_ALLOC;
          *output_ptr = 'X';
          output_ptr += 1;
        } else {
          if (output_ptr + len >= output_end) goto NEED_MORE_ALLOC;
          memcpy(output_ptr, ir->comps[ir->comp_idx].chars[ir->char_idx].start, len);
          output_ptr += len;
        }
      }
    }

    *out_output_written = output_ptr - output;

    return CIPH_OK;

NEED_MORE_ALLOC:

    *out_output_written = output_ptr - output;

    return CIPH_GROW;
  }
}
