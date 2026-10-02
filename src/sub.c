#include "cipher/internal/trie.h"
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

struct _ciph_sub_internal {
  struct _ciph_sub_ud* uds;
  _ciph_trie_t trie;
};

ciph_err_t ciph_sub_parse(const ciph_sub_entry_t* entries, size_t entries_len, ciph_sub_t* sub) {
  ciph_err_t err;
  const ciph_sub_entry_t* entry;
  uint8_t ch;

  _ciph_trie_t trie;
  err = _ciph_trie_create(&trie);
  if (err != CIPH_OK) return err;

  struct _ciph_sub_ud* uds = CIPH_MALLOC(entries_len * sizeof(struct _ciph_sub_ud));
  if (uds == NULL) return CIPH_ERR_ALLOC;

  for (int i = 0; i < entries_len; i++) {
    entry = &entries[i];
    uds[i] = (struct _ciph_sub_ud){ (entries_len - i), entry->substitution, entry->substitution_len };
    err = _ciph_trie_add_text_with_data(&trie, (const char*)entry->input, entry->input_len, &uds[i]);
    if (err != CIPH_OK) return err;
  }

  struct _ciph_sub_internal* v = malloc(sizeof(struct _ciph_sub_internal));
  if (v == NULL) return CIPH_ERR_ALLOC;
  *v = (struct _ciph_sub_internal){ uds, trie };

  *(struct _ciph_sub_internal**)sub = v;

  return CIPH_OK;
}

void ciph_sub_free(ciph_sub_t* sub) {
  struct _ciph_sub_internal* v = *sub;

  _ciph_trie_free(&v->trie);
  CIPH_FREE(v->uds);
  CIPH_FREE(v);

  *sub = NULL;
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

ciph_err_t ciph_sub(
  const uint8_t* input, size_t input_len,
  ciph_sub_t _sub,
  ciph_str_t* output
) {
  struct _ciph_sub_internal* sub = _sub;
  _ciph_trie_t* trie = &sub->trie;

  ciph_err_t err;
  struct _ciph_sub_replacement repl;

  size_t i = 0;
  while (i < input_len) {
  // for (int i = 0; i < input_len; i++) {
    repl = _ciph_sub_check_replacement(trie->roots, trie->roots_len, input + i, input_len - i, 0);
    if (repl.data != NULL) {
      err = ciph_str_push_str(output, repl.data->substitution, repl.data->substitution_len);
      if (err != CIPH_OK) return err;
      i += repl.source_len;
    } else {
      err = ciph_str_push_char(output, input[i]);
      if (err != CIPH_OK) return err;
      i += 1;
    }
  }

  return CIPH_OK;
}
