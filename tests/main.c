#include <cipher.h>
#include <CUnit/Basic.h>
#include <stdio.h>

#include "CUnit/TestDB.h"

#include "internal.h"
#include "ascii.h"
#include "reverse.h"
#include "morse.h"
#include "caesar.h"
#include "numbers.h"
#include "block_method.h"
#include "alphabet_lookup.h"
#include "year.h"
#include "sub.h"

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

  CU_SuiteInfo suites[] = {
    suite_internal(),
    suite_ascii(),
    suite_morse(),
    suite_caesar(),
    suite_reverse(),
    suite_numbers(),
    suite_block_method(),
    suite_alphabet_lookup(),
    suite_year(),
    suite_sub(),
    CU_SUITE_INFO_NULL
  };

  CU_ErrorCode err = CU_register_nsuites(1, suites);
  if (err != CUE_SUCCESS) {
    CU_cleanup_registry();
    return err;
  }

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();

  #ifdef TEST_NO_OUTPUT
  fclose(_stderr);
  #endif

  return CU_get_error();
}
