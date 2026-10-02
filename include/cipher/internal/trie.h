#ifndef _CIPH_INTERNAL_TRIE_H
#define _CIPH_INTERNAL_TRIE_H

#include <cipher/internal/nil.h>
#include <cipher/error.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __ciph_trie_node* nonnil _ciph_trie_node_ref;

struct __ciph_trie_node {
  const char* nonnil text;
  size_t text_len;

  void* nilable data;

  _ciph_trie_node_ref* nilable next;
  int next_cap;
  int next_len;
};

typedef struct {
  struct __ciph_trie_node* nonnil * nonnil data;
  int data_cap;
  int data_len;

  _ciph_trie_node_ref* nonnil roots;
  int roots_cap;
  int roots_len;
} _ciph_trie_t;

ciph_err_t _ciph_trie_create(_ciph_trie_t* nonnil t);

void _ciph_trie_free(_ciph_trie_t* nonnil trie);

ciph_err_t _ciph_trie_add_text_with_data(
  _ciph_trie_t* nonnil trie,
  const char* nonnil text, size_t text_len,
  void* nilable ud
);

#ifdef DEBUG
#define DEBUG_PRINT
#endif

#ifdef DEBUG_PRINT

#include <stdio.h>

void _ciph_trie_print(FILE* nonnil out, const _ciph_trie_t* nonnil trie);

#endif

#ifdef __cplusplus
}
#endif

#endif // _CIPH_UTILS_H
