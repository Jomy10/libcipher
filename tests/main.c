#include <CUnit/Basic.h>

extern void test_ascii(void);
extern void test_reverse(void);
extern void test_caesar(void);
extern void test_caesar_negative(void);

int main(void) {
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
    (CU_add_test(pSuite, "caesar negative shift", test_caesar_negative) == NULL)
  ) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();

  return CU_get_error();
}
