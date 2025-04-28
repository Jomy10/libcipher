#include <cipher.h>
#include <CUnit/Basic.h>
#include <stdio.h>

extern void test_ascii(void);
extern void test_reverse(void);
extern void test_caesar(void);
extern void test_caesar_negative(void);
extern void test_alphabet_lookup(void);
extern void test_alphabet_atbash(void);
extern void test_alphabet_vignere(void);
extern void test_alphabet_vignere_empty_word(void);
extern void test_morse(void);
extern void test_morse_small_buffer(void);
extern void test_morse_multi_word(void);
extern void test_morse_sentence(void);
extern void test_numbers(void);
extern void test_numbers_sentence(void);
extern void test_block_method(void);
extern void test_block_method_big(void);
extern void test_block_method_one_char(void);

FILE* _stderr;

int main(void) {
  printf("Running tests...\n");

  #ifdef TEST_NO_OUTPUT
  _stderr = fopen("/dev/null", "w");
  #else
  _stderr = stderr;
  #endif

  CU_pSuite pSuite = NULL;

  if (CU_initialize_registry() != CUE_SUCCESS)
    return CU_get_error();

  pSuite = CU_add_suite("libcipher", NULL, NULL);
  if (pSuite == NULL) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  if (
    (CU_add_test(pSuite, "ascii test", test_ascii) == NULL) ||
    (CU_add_test(pSuite, "reverse word", test_reverse) == NULL) ||
    (CU_add_test(pSuite, "caesar", test_caesar) == NULL) ||
    (CU_add_test(pSuite, "caesar negative shift", test_caesar_negative) == NULL) ||
    (CU_add_test(pSuite, "alphabet lookup", test_alphabet_lookup) == NULL) ||
    (CU_add_test(pSuite, "atbash", test_alphabet_atbash) == NULL) ||
    (CU_add_test(pSuite, "vignère", test_alphabet_vignere) == NULL) ||
    (CU_add_test(pSuite, "vignère without word", test_alphabet_vignere_empty_word) == NULL) ||
    (CU_add_test(pSuite, "morse", test_morse) == NULL) ||
    (CU_add_test(pSuite, "morse realloc", test_morse_small_buffer) == NULL) ||
    (CU_add_test(pSuite, "morse multi word", test_morse_multi_word) == NULL) ||
    (CU_add_test(pSuite, "morse sentence", test_morse_sentence) == NULL) ||
    (CU_add_test(pSuite, "numbers", test_numbers) == NULL) ||
    (CU_add_test(pSuite, "numbers sentence", test_numbers_sentence) == NULL) ||
    (CU_add_test(pSuite, "block method", test_block_method) == NULL) ||
    (CU_add_test(pSuite, "block method big word", test_block_method_big) == NULL) ||
    (CU_add_test(pSuite, "block method one char", test_block_method_one_char) == NULL)
  ) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();

  #ifdef TEST_NO_OUTPUT
  fclose(_stderr);
  #endif

  return CU_get_error();
}
