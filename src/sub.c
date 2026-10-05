#include "cipher/internal/trie.h"
#include "unitypes.h"
#include <cipher.h>
#include <unistr.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

struct _ciph_sub_ud {
  int prio;
  const uint8_t* substitution;
  size_t substitution_len;
};

struct _ciph_sub_cat {
  bool (*is_cat)(ucs4_t);
  const uint8_t* substitution;
  size_t substitution_len;
};

struct _ciph_sub_internal {
  struct _ciph_sub_ud* uds;
  _ciph_trie_t trie;

  struct _ciph_sub_cat* cats;
  size_t cats_cap;
  size_t cats_len;

  bool cat_singular;
  bool is_plus;
};

struct _ciph_sub_internal_plus {
  struct _ciph_sub_internal internal;
  void* ptr;
};

union _ciph_sub_internal_union {
  struct _ciph_sub_internal data;
  struct _ciph_sub_internal_plus plus;
};

#ifdef __EMSCRIPTEN__
const ciph_sub_entry_t* ciph_sub_entries_create(int size) {
  return CIPH_MALLOC(size * sizeof(ciph_sub_entry_t));
}

#include <stdio.h>

void ciph_sub_entries_add_entry(
  ciph_sub_entry_t* entries,
  const uint8_t* lookup, size_t lookup_len,
  const uint8_t* substitution, size_t substitution_len,
  int i
) {
  entries[i] = (ciph_sub_entry_t){
    lookup, lookup_len,
    substitution, substitution_len
  };
}

void ciph_sub_entries_free(const ciph_sub_entry_t* entries) {
  CIPH_FREE((void*)entries);
}
#endif

ciph_err_t _ciph_sub_parse_internal(
  const ciph_sub_entry_t* entries, size_t entries_len,
  bool plus,
  ciph_sub_t* sub
) {
  ciph_err_t err;
  const ciph_sub_entry_t* entry;
  uint8_t ch;

  _ciph_trie_t trie;
  err = _ciph_trie_create(&trie);
  if (err != CIPH_OK) return err;

  struct _ciph_sub_ud* uds = CIPH_MALLOC(entries_len * sizeof(struct _ciph_sub_ud));
  if (uds == NULL) {
    _ciph_trie_free(&trie);
    return CIPH_ERR_ALLOC;
  }

  for (int i = 0; i < entries_len; i++) {
    entry = &entries[i];
    uds[i] = (struct _ciph_sub_ud){ (entries_len - i), entry->substitution, entry->substitution_len };
    err = _ciph_trie_add_text_with_data(&trie, (const char*)entry->input, entry->input_len, &uds[i]);
    if (err != CIPH_OK) {
      _ciph_trie_free(&trie);
      CIPH_FREE(uds);
      return err;
    }
  }

  union _ciph_sub_internal_union* u;

  if (plus) {
    struct _ciph_sub_internal_plus* v = CIPH_CALLOC(1, sizeof(struct _ciph_sub_internal_plus));
    u = (union _ciph_sub_internal_union*)v;
  } else {
    struct _ciph_sub_internal* v = CIPH_CALLOC(1, sizeof(struct _ciph_sub_internal));
    u = (union _ciph_sub_internal_union*)v;
  }

  struct _ciph_sub_internal* v = &u->data;

  if (v == NULL) {
    _ciph_trie_free(&trie);
    CIPH_FREE(uds);
    return CIPH_ERR_ALLOC;
  }
  v->uds = uds;
  v->trie = trie;

  *((struct _ciph_sub_internal**)sub) = v;

  return CIPH_OK;
}

ciph_err_t ciph_sub_parse(
  const ciph_sub_entry_t* entries, size_t entries_len,
  ciph_sub_t* sub
) {
  return _ciph_sub_parse_internal(entries, entries_len, false, sub);
}

ciph_err_t ciph_sub_add_cat(
  ciph_sub_t sub,
  bool (*is_cat)(ucs4_t),
  const uint8_t* subs, size_t subs_len
) {
  struct _ciph_sub_internal* v = sub;

  if (v->cats_cap == 0) {
    v->cats = CIPH_MALLOC(2 * sizeof(struct _ciph_sub_cat));
    if (v->cats == NULL) return CIPH_ERR_ALLOC;
    v->cats_cap = 2;
  } else if (v->cats_cap == v->cats_len) {
    size_t new_cap = v->cats_cap * 2;
    void* tmp = CIPH_REALLOC(v->cats, new_cap * sizeof(struct _ciph_sub_cat));
    if (tmp == NULL) return CIPH_ERR_ALLOC;
    v->cats_cap = new_cap;
    v->cats = tmp;
  }

  v->cats[v->cats_len++] = (struct _ciph_sub_cat){ is_cat, subs, subs_len };

  return CIPH_OK;
}

void ciph_sub_set_cat_singular(
  ciph_sub_t sub,
  bool singular
) {
  struct _ciph_sub_internal* v = sub;
  v->cat_singular = singular;
}

void ciph_sub_free(ciph_sub_t sub) {
  struct _ciph_sub_internal* v = sub;

  _ciph_trie_free(&v->trie);
  CIPH_FREE(v->uds);

  if (v->cats_cap != 0) {
    CIPH_FREE(v->cats);
  }

  if (v->is_plus) {
    CIPH_FREE(((union _ciph_sub_internal_union*)v)->plus.ptr);
  }

  CIPH_FREE(v);
}

EXPORT ciph_err_t ciph_sub_kenny_lang(ciph_sub_t* sub) {
  ciph_sub_entry_t* entries = malloc(52 * sizeof(ciph_sub_entry_t));
  if (entries == 0) return CIPH_ERR_ALLOC;
  entries[0] = (const ciph_sub_entry_t){ (const uint8_t*)"A", 1, (const uint8_t*)"Mmm", 3  };
  entries[1] = (const ciph_sub_entry_t){ (const uint8_t*)"B", 1, (const uint8_t*)"Mmp", 3  };
  entries[2] = (const ciph_sub_entry_t){ (const uint8_t*)"C", 1, (const uint8_t*)"Mmf", 3  };
  entries[3] = (const ciph_sub_entry_t){ (const uint8_t*)"D", 1, (const uint8_t*)"Mpm", 3  };
  entries[4] = (const ciph_sub_entry_t){ (const uint8_t*)"E", 1, (const uint8_t*)"Mpp", 3  };
  entries[5] = (const ciph_sub_entry_t){ (const uint8_t*)"F", 1, (const uint8_t*)"Mpf", 3  };
  entries[6] = (const ciph_sub_entry_t){ (const uint8_t*)"G", 1, (const uint8_t*)"Mfm", 3  };
  entries[7] = (const ciph_sub_entry_t){ (const uint8_t*)"H", 1, (const uint8_t*)"Mfp", 3  };
  entries[8] = (const ciph_sub_entry_t){ (const uint8_t*)"I", 1, (const uint8_t*)"Mff", 3  };
  entries[9] = (const ciph_sub_entry_t){ (const uint8_t*)"J", 1, (const uint8_t*)"Pmm", 3  };
  entries[10] = (const ciph_sub_entry_t){ (const uint8_t*)"K", 1, (const uint8_t*)"Pmp", 3  };
  entries[11] = (const ciph_sub_entry_t){ (const uint8_t*)"L", 1, (const uint8_t*)"Pmf", 3  };
  entries[12] = (const ciph_sub_entry_t){ (const uint8_t*)"M", 1, (const uint8_t*)"Ppm", 3  };
  entries[13] = (const ciph_sub_entry_t){ (const uint8_t*)"N", 1, (const uint8_t*)"Ppp", 3  };
  entries[14] = (const ciph_sub_entry_t){ (const uint8_t*)"O", 1, (const uint8_t*)"Ppf", 3  };
  entries[15] = (const ciph_sub_entry_t){ (const uint8_t*)"P", 1, (const uint8_t*)"Pfm", 3  };
  entries[16] = (const ciph_sub_entry_t){ (const uint8_t*)"Q", 1, (const uint8_t*)"Pfp", 3  };
  entries[17] = (const ciph_sub_entry_t){ (const uint8_t*)"R", 1, (const uint8_t*)"Pff", 3  };
  entries[18] = (const ciph_sub_entry_t){ (const uint8_t*)"S", 1, (const uint8_t*)"Fmm", 3  };
  entries[19] = (const ciph_sub_entry_t){ (const uint8_t*)"T", 1, (const uint8_t*)"Fmp", 3  };
  entries[20] = (const ciph_sub_entry_t){ (const uint8_t*)"U", 1, (const uint8_t*)"Fmf", 3  };
  entries[21] = (const ciph_sub_entry_t){ (const uint8_t*)"V", 1, (const uint8_t*)"Fpm", 3  };
  entries[22] = (const ciph_sub_entry_t){ (const uint8_t*)"W", 1, (const uint8_t*)"Fpp", 3  };
  entries[23] = (const ciph_sub_entry_t){ (const uint8_t*)"X", 1, (const uint8_t*)"Fpf", 3  };
  entries[24] = (const ciph_sub_entry_t){ (const uint8_t*)"Y", 1, (const uint8_t*)"Ffm", 3  };
  entries[25] = (const ciph_sub_entry_t){ (const uint8_t*)"Z", 1, (const uint8_t*)"Ffp", 3  };
  entries[26] = (const ciph_sub_entry_t){ (const uint8_t*)"a", 1, (const uint8_t*)"mmm", 3  };
  entries[27] = (const ciph_sub_entry_t){ (const uint8_t*)"b", 1, (const uint8_t*)"mmp", 3  };
  entries[28] = (const ciph_sub_entry_t){ (const uint8_t*)"c", 1, (const uint8_t*)"mmf", 3  };
  entries[29] = (const ciph_sub_entry_t){ (const uint8_t*)"d", 1, (const uint8_t*)"mpm", 3  };
  entries[30] = (const ciph_sub_entry_t){ (const uint8_t*)"e", 1, (const uint8_t*)"mpp", 3  };
  entries[31] = (const ciph_sub_entry_t){ (const uint8_t*)"f", 1, (const uint8_t*)"mpf", 3  };
  entries[32] = (const ciph_sub_entry_t){ (const uint8_t*)"g", 1, (const uint8_t*)"mfm", 3  };
  entries[33] = (const ciph_sub_entry_t){ (const uint8_t*)"h", 1, (const uint8_t*)"mfp", 3  };
  entries[34] = (const ciph_sub_entry_t){ (const uint8_t*)"i", 1, (const uint8_t*)"mff", 3  };
  entries[35] = (const ciph_sub_entry_t){ (const uint8_t*)"j", 1, (const uint8_t*)"pmm", 3  };
  entries[36] = (const ciph_sub_entry_t){ (const uint8_t*)"k", 1, (const uint8_t*)"pmp", 3  };
  entries[37] = (const ciph_sub_entry_t){ (const uint8_t*)"l", 1, (const uint8_t*)"pmf", 3  };
  entries[38] = (const ciph_sub_entry_t){ (const uint8_t*)"m", 1, (const uint8_t*)"ppm", 3  };
  entries[39] = (const ciph_sub_entry_t){ (const uint8_t*)"n", 1, (const uint8_t*)"ppp", 3  };
  entries[40] = (const ciph_sub_entry_t){ (const uint8_t*)"o", 1, (const uint8_t*)"ppf", 3  };
  entries[41] = (const ciph_sub_entry_t){ (const uint8_t*)"p", 1, (const uint8_t*)"pfm", 3  };
  entries[42] = (const ciph_sub_entry_t){ (const uint8_t*)"q", 1, (const uint8_t*)"pfp", 3  };
  entries[43] = (const ciph_sub_entry_t){ (const uint8_t*)"r", 1, (const uint8_t*)"pff", 3  };
  entries[44] = (const ciph_sub_entry_t){ (const uint8_t*)"s", 1, (const uint8_t*)"fmm", 3  };
  entries[45] = (const ciph_sub_entry_t){ (const uint8_t*)"t", 1, (const uint8_t*)"fmp", 3  };
  entries[46] = (const ciph_sub_entry_t){ (const uint8_t*)"u", 1, (const uint8_t*)"fmf", 3  };
  entries[47] = (const ciph_sub_entry_t){ (const uint8_t*)"v", 1, (const uint8_t*)"fpm", 3  };
  entries[48] = (const ciph_sub_entry_t){ (const uint8_t*)"w", 1, (const uint8_t*)"fpp", 3  };
  entries[49] = (const ciph_sub_entry_t){ (const uint8_t*)"x", 1, (const uint8_t*)"fpf", 3  };
  entries[50] = (const ciph_sub_entry_t){ (const uint8_t*)"y", 1, (const uint8_t*)"ffm", 3  };
  entries[51] = (const ciph_sub_entry_t){ (const uint8_t*)"z", 1, (const uint8_t*)"ffp", 3  };

  ciph_err_t err = _ciph_sub_parse_internal(
    entries, 52,
    true,
    sub
  );
  if (err != CIPH_OK) return err;

  ((union _ciph_sub_internal_union*)*sub)->plus.ptr = (void*)entries;

  return CIPH_OK;
}

struct _ciph_sub_replacement {
  struct _ciph_sub_ud* data;
  size_t source_len;
};

struct _ciph_sub_replacement _ciph_sub_check_replacement(
  _ciph_trie_node_ref* nodes, size_t nodes_len,
  const uint8_t* input, size_t input_len,
  size_t acc_len
) {
  _ciph_trie_node_ref node;
  for (int s = 0; s < nodes_len; s++) {
    node = nodes[s];

    if (input_len < node->text_len) continue;

    if (u8_cmp((const uint8_t*)node->text, input, node->text_len) != 0) continue;

    struct _ciph_sub_replacement curr_repl = (struct _ciph_sub_replacement) {
      (struct _ciph_sub_ud*)node->data,
      acc_len + node->text_len
    };

    if (node->next_len == 0) {
      return curr_repl;
    } else {
      struct _ciph_sub_replacement ret = _ciph_sub_check_replacement(
        node->next, node->next_len,
        input + node->text_len, input_len - node->text_len,
        acc_len + node->text_len
      );

      // No further matches found -> return this node
      if (ret.data == NULL) {
        return curr_repl;
      }

      // This match was higher up the list, so return this match instead
      if (node->data == NULL || ret.data->prio > ((struct _ciph_sub_ud*)node->data)->prio) {
        return ret;
      } else {
        return curr_repl;
      }
    }
  }

  return (struct _ciph_sub_replacement) {
    NULL,
    0
  };
}

/// Push category substitution to output
ciph_err_t _ciph_sub_add_cat(
  struct _ciph_sub_internal* sub,
  ucs4_t uc,
  ciph_str_t* output,
  int s,
  int* prev_cat_prio,
  int* prev_cat_len
) {
  ciph_err_t err = ciph_str_push_str(output, sub->cats[s].substitution, sub->cats[s].substitution_len);
  if (err != CIPH_OK) return err;
  *prev_cat_prio = sub->cats_len - s;
  *prev_cat_len = sub->cats[s].substitution_len;
  return CIPH_OK;
}

ciph_err_t ciph_sub(
  const uint8_t* input, size_t input_len,
  ciph_sub_t _sub,
  ciph_str_t* output
) {
  struct _ciph_sub_internal* sub = _sub;
  _ciph_trie_t* trie = &sub->trie;

  ciph_err_t err;
  struct _ciph_sub_replacement repl;

  ucs4_t uc;
  int uc_len;

  int prev_cat_prio = -1;
  int prev_cat_len = 0;

  size_t i = 0;
  while (i < input_len) {
    repl = _ciph_sub_check_replacement(trie->roots, trie->roots_len, input + i, input_len - i, 0);
    if (repl.data != NULL) {
      err = ciph_str_push_str(output, repl.data->substitution, repl.data->substitution_len);
      if (err != CIPH_OK) return err;
      i += repl.source_len;
      prev_cat_prio = -1;
    } else {
      uc_len = u8_mbtouc(&uc, input + i, input_len - i);

      if (sub->cats_len != 0) {
        if (sub->cat_singular) {
          for (int s = 0; s < sub->cats_len; s++) {
            if (sub->cats[s].is_cat(uc)) {
              if (prev_cat_prio != -1) {
                if ((sub->cats_len - s) > prev_cat_prio) {
                  // revert previous to replace with current matched
                  ciph_str_popn(output, prev_cat_len);
                } else {
                  continue;
                }
              }
              err = _ciph_sub_add_cat(sub, uc, output, s, &prev_cat_prio, &prev_cat_len);
              if (err != CIPH_OK) return err;
              break;
            }
          }
        } else {
          prev_cat_prio = -1;
          for (int s = 0; s < sub->cats_len; s++) {
            if (sub->cats[s].is_cat(uc)) {
              err = _ciph_sub_add_cat(sub, uc, output, s, &prev_cat_prio, &prev_cat_len);
              if (err != CIPH_OK) return err;
              break;
            }
          }
        }
      }

      if (prev_cat_prio == -1) { // didn't match a category
        err = ciph_str_push_str(output, input + i, uc_len);
        if (err != CIPH_OK) return err;
      }

      i += uc_len;
    }
  }

  return CIPH_OK;
}
