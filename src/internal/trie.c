#include <cipher/internal/trie.h>

#include <cipher/internal/defines.h>
#include <cipher/error.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define _CIPH_TRIE_DATA_CAP 64

ciph_err_t _ciph_trie_create(_ciph_trie_t* t) {
  t->data = CIPH_MALLOC(1 * sizeof(void*));
  if (t->data == NULL) return CIPH_ERR_ALLOC;
  t->data_cap = 1;
  t->data_len = 0;

  t->data[0] = CIPH_CALLOC(_CIPH_TRIE_DATA_CAP, sizeof(struct __ciph_trie_node));
  if (t->data[0] == NULL) {
    CIPH_FREE(t->data);
    return CIPH_ERR_ALLOC;
  }

  t->roots = CIPH_MALLOC(10 * sizeof(void*));
  if (t->roots == NULL) {
    CIPH_FREE(t->data[0]);
    CIPH_FREE(t->data);
    return CIPH_ERR_ALLOC;
  }
  t->roots_cap = 10;
  t->roots_len = 0;

  return CIPH_OK;
}

void _ciph_trie_free(_ciph_trie_t* trie) {
  for (int i = 0; i < trie->data_cap; i++) {
    CIPH_FREE(trie->data[i]);
  }
  CIPH_FREE(trie->data);
  CIPH_FREE(trie->roots);
}

/// Add new nodes to be used
ciph_err_t _ciph_trie_add_data_buffer(_ciph_trie_t* trie) {
  void* tmp = CIPH_REALLOC(trie->data, (trie->data_cap + 1) * sizeof(void*));
  if (tmp == NULL) return CIPH_ERR_ALLOC;
  trie->data = tmp;

  trie->data[trie->data_cap] = CIPH_CALLOC(_CIPH_TRIE_DATA_CAP, sizeof(struct __ciph_trie_node));
  if (trie->data[trie->data_cap] == NULL) return CIPH_ERR_ALLOC;
  trie->data_cap += 1;

  return CIPH_OK;
}

/// Get a new node
ciph_err_t _ciph_trie_new_node(_ciph_trie_t* trie, _ciph_trie_node_ref* node) {
  if (trie->data_len == trie->data_cap * _CIPH_TRIE_DATA_CAP) {
    ciph_err_t err = _ciph_trie_add_data_buffer(trie);
    if (err != CIPH_OK) return err;
  }

  *node = &(trie->data[trie->data_len / _CIPH_TRIE_DATA_CAP][trie->data_len % _CIPH_TRIE_DATA_CAP]);
  trie->data_len++;

  return CIPH_OK;
}

ciph_err_t _ciph_trie_ensure_roots_cap(_ciph_trie_t* trie, size_t cap) {
  if (trie->roots_cap >= cap) return CIPH_OK;

  size_t new_cap = MAX(trie->roots_cap * 2, cap);
  void* tmp = CIPH_REALLOC(trie->roots, new_cap * sizeof(void*));
  if (tmp == NULL) return CIPH_ERR_ALLOC;

  trie->roots = tmp;
  trie->roots_cap = new_cap;

  return CIPH_OK;
}

ciph_err_t _ciph_trie_ensure_next_cap(_ciph_trie_node_ref node, size_t cap) {
  if (node->next_cap == 0) {
    node->next = CIPH_MALLOC(cap * sizeof(void*));
    if (node->next == NULL) return CIPH_ERR_ALLOC;
    node->next_cap = cap;
    node->next_len = 0;
    return CIPH_OK;
  }

  if (node->next_cap < cap) {
    size_t new_cap = MAX(node->next_cap * 2, cap);
    void* tmp = CIPH_REALLOC(node->next, new_cap * sizeof(void*));
    if (tmp == NULL) return CIPH_ERR_ALLOC;

    node->next = tmp;
    node->next_cap = new_cap;
  }

  return CIPH_OK;
}

/// Append `child` to the children of `parent`, or to the roots if `parent` is NULL
ciph_err_t _ciph_trie_append_child(
  _ciph_trie_t* trie,
  _ciph_trie_node_ref parent,
  _ciph_trie_node_ref child
) {
  ciph_err_t err;

  if (parent == NULL) {
    err = _ciph_trie_ensure_roots_cap(trie, trie->roots_len + 1);
    if (err != CIPH_OK) return err;
    trie->roots[trie->roots_len++] = child;
  } else {
    err = _ciph_trie_ensure_next_cap(parent, parent->next_len + 1);
    if (err != CIPH_OK) return err;
    parent->next[parent->next_len++] = child;
  }

  return CIPH_OK;
}

/// Get the length of the common prefix of 2 strings
size_t _ciph_trie_strcmp(
  const char* a, size_t alen,
  const char* b, size_t blen
) {
  size_t i = 0;

  while (i < alen && i < blen && a[i] == b[i]) i++;

  return i;
}

ciph_err_t _ciph_trie_add_text_with_data(
  _ciph_trie_t* trie,
  const char* text, size_t text_len,
  void* ud
) {
  assert(text_len > 0);

  ciph_err_t err;

  if (text_len == 0) return CIPH_OK;

  _ciph_trie_node_ref parent = NULL;
  _ciph_trie_node_ref* children = trie->roots;
  int children_len = trie->roots_len;

  while (text_len > 0) {
    _ciph_trie_node_ref node = NULL;
    size_t idx = 0;
    for (int i = 0; i < children_len; i++) {
      idx = _ciph_trie_strcmp(children[i]->text, children[i]->text_len, text, text_len);
      if (idx > 0) {
        node = children[i];
        break;
      }
    }

    // No common prefix with any child: add the remaining text as a new leaf
    if (node == NULL) {
      err = _ciph_trie_new_node(trie, &node);
      if (err != CIPH_OK) return err;
      node->text = text;
      node->text_len = text_len;
      node->data = ud;
      return _ciph_trie_append_child(trie, parent, node);
    }

    // Partial match: split the node, moving its suffix, data and children to a new child
    if (idx < node->text_len) {
      _ciph_trie_node_ref* next = CIPH_MALLOC(2 * sizeof(void*));
      if (next == NULL) return CIPH_ERR_ALLOC;

      _ciph_trie_node_ref split;
      err = _ciph_trie_new_node(trie, &split);
      if (err != CIPH_OK) {
        CIPH_FREE(next);
        return err;
      }

      *split = *node;
      split->text += idx;
      split->text_len -= idx;

      node->text_len = idx;
      node->data = NULL;
      node->next = next;
      node->next_cap = 2;
      node->next_len = 1;
      node->next[0] = split;
    }

    text += idx;
    text_len -= idx;
    parent = node;
    children = node->next;
    children_len = node->next_len;
  }

  // The text ends exactly at an existing node
  parent->data = ud;

  return CIPH_OK;
}

#ifdef DEBUG_PRINT

#include <stdio.h>

/// Print `node` and its children, indented by `depth` levels
void _ciph_trie_print_node(FILE* out, _ciph_trie_node_ref node, int depth) {
  for (int i = 0; i < depth; i++) fputs("  ", out);

  fprintf(out, "'%.*s'", (int)node->text_len, node->text);
  if (node->data != NULL) fprintf(out, " -> %p", node->data);
  fputc('\n', out);

  for (int i = 0; i < node->next_len; i++) {
    _ciph_trie_print_node(out, node->next[i], depth + 1);
  }
}

/// Print the trie as an indented tree
void _ciph_trie_print(FILE* out, const _ciph_trie_t* trie) {
  fprintf(out, "trie (%d nodes, %d roots)\n", trie->data_len, trie->roots_len);

  for (int i = 0; i < trie->roots_len; i++) {
    _ciph_trie_print_node(out, trie->roots[i], 1);
  }
}

#endif
