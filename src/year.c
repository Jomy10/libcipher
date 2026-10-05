#include "cipher/error.h"
#include <cipher.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <unistr.h>
#include <unictype.h>
#include <unigbrk.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/// A character inside of a year component
struct _ciph_year_comp_char {
  int len;
  const uint8_t* start;
};

struct _ciph_year_comp {
  /// Size in elements of the data part of the component
  int cap;
  /// Amount of years
  int year_count;
  /// Character count inside of `year`
  int char_count;
  /// Length of single element
  uint8_t year;
  struct _ciph_year_comp_char* nonnil chars;
};

ciph_err_t _ciph_yc_push(struct _ciph_year_comp* self, struct _ciph_year_comp_char c) {
  int idx = self->year_count * self->year + self->char_count;
  if (idx >= self->cap) {
    size_t new_cap = self->cap * 2;
    struct _ciph_year_comp_char* tmp = CIPH_REALLOC(self->chars, new_cap * sizeof(struct _ciph_year_comp_char));
    if (tmp == NULL) {
      return CIPH_ERR_ALLOC;
    }
    self->chars = tmp;
    self->cap = new_cap;
  }
  self->chars[idx] = c;
  self->char_count += 1;

  if (self->char_count == self->year) {
    self->char_count = 0;
    self->year_count += 1;
  }

  return CIPH_OK;
}

ciph_err_t _ciph_year_write_out(
  struct _ciph_year_comp* comps,
  ciph_str_t* output
) {
  ciph_err_t err = CIPH_OK;

  int comp_idx = 0;
  struct _ciph_year_comp* curr_comp;
  struct _ciph_year_comp_char* curr_char;

  // Fill the last (incomplete) row with X'es
  int diff;
  for (comp_idx = 0; comp_idx < 4; comp_idx += 1) {
    curr_comp = &comps[comp_idx];
    if (curr_comp->year == 0 || curr_comp->char_count == 0) continue;

    diff = curr_comp->year - curr_comp->char_count;
    for (; diff > 0; diff -= 1) {
      err = _ciph_yc_push(curr_comp, (struct _ciph_year_comp_char){ 1, (const uint8_t*)"X" });
      if (err != CIPH_OK) return err;
    }
  }

  // Amount of rows per component and the length of the longest row used
  int years_count = 0;
  int highest_year_val = 0;
  for (comp_idx = 0; comp_idx < 4; comp_idx += 1) {
    if (comps[comp_idx].year_count == 0) continue;
    years_count = MAX(years_count, comps[comp_idx].year_count);
    highest_year_val = MAX(highest_year_val, comps[comp_idx].year);
  }
  if (years_count == 0) goto RETURN_WRITE;

  comp_idx = 0;
  int char_idx = 0;
  int year_idx = 0;

  // Read column by column (char_idx), going through the rows in order
  // (year_idx, comp_idx)
  while (true) {
    curr_comp = &comps[comp_idx];

    if (char_idx >= curr_comp->year || year_idx >= curr_comp->year_count) goto NEXT_YEAR;

    curr_char = &curr_comp->chars[year_idx * curr_comp->year + char_idx];

    err = ciph_str_push_str(output, curr_char->start, curr_char->len);
    if (err != CIPH_OK) return err;

  NEXT_YEAR:
    comp_idx = (comp_idx + 1) % 4;
    if (comp_idx == 0) {
      year_idx += 1;
      if (year_idx == years_count) {
        year_idx = 0;
        char_idx += 1;
        if (char_idx == highest_year_val) {
          goto RETURN_WRITE;
        }
        err = ciph_str_push_char(output, ' ');
        if (err != CIPH_OK) return err;
      }
    }
  }

RETURN_WRITE:
  // reset
  for (int i = 0; i < 4; i++) {
    comps[i].year_count = 0;
    comps[i].char_count = 0;
  }

  return err;
}

ciph_err_t ciph_year(
  const uint8_t* nonnil input, size_t input_len,
  uint8_t year[nonnil 4],
  uint32_t char_include_mask,
  ciph_str_t* nonnil output
) {
  if (input_len == 0) return CIPH_OK;

  ciph_err_t err = CIPH_OK;

  const uint8_t* input_ptr = input;
  const uint8_t* input_end = input + input_len;

  const uint8_t* next;
  int grapheme_len;

  if (
    (year[0] > 9 || year[1] > 9 || year[3] > 9 || year[4] > 9) ||
    (year[0] == 0 && year[1] == 0 && year[2] == 0 && year[3] == 0)
  ) {
    return CIPH_ERR_YEAR_DIGITS;
  }

  int init_comp_el_cap = MAX(1, input_len / 6 / 4);
  struct _ciph_year_comp comps[4];
  for (int i = 0; i < 4; i++) {
    comps[i].year = year[i];
    comps[i].cap = init_comp_el_cap * comps[i].year;
    comps[i].year_count = 0;
    comps[i].char_count = 0;
    if (comps[i].year != 0) {
      void* c = CIPH_MALLOC(comps[i].cap * sizeof(struct _ciph_year_comp_char));
      if (c == NULL) {
        if (i != 0)
          for (int j = i - 1; j >= 0; j--)
            if (comps[j].year != 0)
              CIPH_FREE(comps[j].chars);
        return CIPH_ERR_ALLOC;
      }
      comps[i].chars = c;
    }
  }

  // First component with a non-zero digit
  int first_comps_idx = 0;
  while (comps[first_comps_idx].year == 0) first_comps_idx += 1;

  int comps_idx = first_comps_idx;
  int year_val = 0;

  struct _ciph_year_comp* curr_comp;

  ucs4_t uc;
  int uc_len;
  while (input_ptr != input_end) {
    next = u8_grapheme_next(input_ptr, input_end);
    assert(next != NULL); // checked in while loop that we still have characters left
    // if (next == NULL) break;
    grapheme_len = next - input_ptr;

    uc_len = u8_mbtouc(&uc, input_ptr, grapheme_len);
    if (uc == 0xfffd) {
      err = CIPH_ERR_ENCODING;
      goto RETURN;
    }
    if (uc_len == 0) {
      input_ptr += 1;
      continue;
    };

    if (uc_is_property_sentence_terminal(uc)) {
      // Reached end of sentence -> write to output & reset comps
      err = _ciph_year_write_out(comps, output);
      if (err != CIPH_OK) goto RETURN;
      err = ciph_str_push_str(output, input_ptr, grapheme_len);
      if (err != CIPH_OK) goto RETURN;
      if (next != input_end) {
        err = ciph_str_push_char(output, ' ');
        if (err != CIPH_OK) goto RETURN;
      }
      comps_idx = first_comps_idx;
      year_val = 0;
      goto NEXT_CHAR;
    }

    if (!uc_is_general_category_withtable(uc, char_include_mask)) {
      goto NEXT_CHAR;
    }

    // Add character to the correct year
    err = _ciph_yc_push(&comps[comps_idx], (struct _ciph_year_comp_char){ grapheme_len, input_ptr });
    if (err != CIPH_OK) goto RETURN;

    year_val += 1;

    if (year_val == comps[comps_idx].year) {
      year_val = 0;
      do {
        comps_idx = (comps_idx + 1) % 4;
      } while (comps[comps_idx].year == 0);
    }

  NEXT_CHAR:
    input_ptr = next;
  }

  err = _ciph_year_write_out(comps, output);

RETURN:
  for (int i = 0; i < 4; i++) {
    if (comps[i].year != 0)
      CIPH_FREE((void*)comps[i].chars);
  }

  return err;
}
