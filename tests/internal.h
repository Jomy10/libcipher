#include <CUnit/CUnit.h>
#include "unistr.h"
#include "utils.h"

#include <cipher/uc_cat.h>
#include <cipher/internal/trie.h>

#include <string.h>

void test_wrdbrk(void) {
  CU_ASSERT(ciph_uc_is_wordbreak('.'));
  CU_ASSERT(ciph_uc_is_wordbreak('?'));
  CU_ASSERT(ciph_uc_is_wordbreak(')'));
  CU_ASSERT(ciph_uc_is_wordbreak('('));
  CU_ASSERT(ciph_uc_is_wordbreak('"'));
  CU_ASSERT(ciph_uc_is_wordbreak('\''));
  CU_ASSERT(ciph_uc_is_wordbreak('-') == false);
}

#define _ciph_trie_add_text_with_data2(trie, text, data) _ciph_trie_add_text_with_data(trie, text, strlen(text), data)
void test_trie(void) {
  _ciph_trie_t trie;
  CU_ASSERT(_ciph_trie_create(&trie) == CIPH_OK);

  const char* text = "text";
  const char* text2 = "text2";
  CU_ASSERT(_ciph_trie_add_text_with_data2(&trie, text, "data") == CIPH_OK);
  CU_ASSERT(_ciph_trie_add_text_with_data2(&trie, text2, "data2") == CIPH_OK);

  CU_ASSERT(trie.roots_len == 1);
  CU_ASSERT(trie.roots[0]->text_len == strlen(text));
  CU_ASSERT(u8_cmp((const uint8_t*)trie.roots[0]->text, (const uint8_t*)text, trie.roots[0]->text_len) == 0);
  CU_ASSERT(strcmp(trie.roots[0]->data, "data") == 0);

  CU_ASSERT(trie.roots[0]->next_len == 1);
  CU_ASSERT(trie.roots[0]->next[0]->text_len == 1);
  CU_ASSERT(u8_cmp((const uint8_t*)trie.roots[0]->next[0]->text, (const uint8_t*)"2", trie.roots[0]->next[0]->text_len) == 0);
  CU_ASSERT(strcmp(trie.roots[0]->next[0]->data, "data2") == 0);

  _ciph_trie_free(&trie);
}

void test_trie_multiple_roots(void) {
  _ciph_trie_t trie;
    CU_ASSERT(_ciph_trie_create(&trie) == CIPH_OK);

    const char* text1 = "text1";
    const char* text2 = "text2";
    CU_ASSERT(_ciph_trie_add_text_with_data2(&trie, text1, "data1") == CIPH_OK);
    CU_ASSERT(_ciph_trie_add_text_with_data2(&trie, text2, "data2") == CIPH_OK);

    CU_ASSERT(trie.roots_len == 2);
    CU_ASSERT(u8_cmp((const uint8_t*)trie.roots[0]->text, (const uint8_t*)text1, trie.roots[0]->text_len));
    CU_ASSERT(strcmp(trie.roots[0]->data, "data1"));
    eprintf("%s", trie.roots[0]->text);
    CU_ASSERT(u8_cmp((const uint8_t*)trie.roots[1]->text, (const uint8_t*)text2, trie.roots[1]->text_len));
    CU_ASSERT(strcmp(trie.roots[1]->data, "data2"));

    const char* text3 = "text13";
    const char* text4 = "text24";
    const char* text5 = "text135";
    const char* text6 = "text1X6";
    const char* text7 = "text1X7";

    CU_ASSERT(_ciph_trie_add_text_with_data2(&trie, text3, "data3") == CIPH_OK);
    CU_ASSERT(_ciph_trie_add_text_with_data2(&trie, text4, "data4") == CIPH_OK);
    CU_ASSERT(_ciph_trie_add_text_with_data2(&trie, text5, "data5") == CIPH_OK);
    CU_ASSERT(_ciph_trie_add_text_with_data2(&trie, text6, "data6") == CIPH_OK);
    CU_ASSERT(_ciph_trie_add_text_with_data2(&trie, text7, "data7") == CIPH_OK);

    CU_ASSERT(trie.roots_len == 2);
    CU_ASSERT(trie.roots[0]->next_len == 2);
    CU_ASSERT(u8_cmp((const uint8_t*)trie.roots[0]->next[0]->text, (const uint8_t*)"3", trie.roots[0]->next[0]->text_len));
    CU_ASSERT(strcmp(trie.roots[0]->next[0]->data, "data3"));

    // Word split into 2 paths (not end node; data == NULL)
    CU_ASSERT(trie.roots[0]->next[1]->data == NULL);
    CU_ASSERT(u8_cmp((const uint8_t*)trie.roots[0]->next[1]->text, (const uint8_t*)"X", trie.roots[0]->next[0]->text_len));
    CU_ASSERT(trie.roots[0]->next[1]->next_len == 2);

    CU_ASSERT(u8_cmp((const uint8_t*)trie.roots[0]->next[1]->next[0]->text, (const uint8_t*)"6", trie.roots[0]->next[1]->text_len));
    CU_ASSERT(strcmp(trie.roots[0]->next[1]->next[0]->data, "data6"));
    CU_ASSERT(u8_cmp((const uint8_t*)trie.roots[0]->next[1]->next[1]->text, (const uint8_t*)"7", trie.roots[0]->next[1]->text_len));
    CU_ASSERT(strcmp(trie.roots[0]->next[1]->next[1]->data, "data7"));

    CU_ASSERT(trie.roots[0]->next[0]->next_len == 1);
    CU_ASSERT(u8_cmp((const uint8_t*)trie.roots[0]->next[0]->next[0]->text, (const uint8_t*)"5", trie.roots[0]->next[0]->next[0]->text_len));
    CU_ASSERT(strcmp(trie.roots[0]->next[0]->next[0]->data, "data5"));

    CU_ASSERT(trie.roots[1]->next_len == 1);
    CU_ASSERT(u8_cmp((const uint8_t*)trie.roots[1]->next[0]->text, (const uint8_t*)"4", trie.roots[1]->next[0]->text_len));
    CU_ASSERT(strcmp(trie.roots[1]->next[0]->data, "data4"));

    _ciph_trie_free(&trie);
}

CU_SuiteInfo suite_internal(void) {
  static CU_TestInfo tests[] = {
    { "wordbreak", test_wrdbrk },
    { "trie", test_trie }
  };
  return (CU_SuiteInfo){ "internal", NULL, NULL, NULL, NULL, tests };
}
