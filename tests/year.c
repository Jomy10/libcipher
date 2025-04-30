#include <cipher.h>
#include <unistd.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <unistr.h>
#include <unictype.h>
#include "utils.h"

void test_year(void) {
  uint8_t* input = (uint8_t*)"GA NU DADELIJK TERUG NAAR HET LOKAAL";
  uint8_t* expected = (uint8_t*)"GAJAOK NKRA UTHA DEEL ARTX DULX EGX LNX IAX";
  uint8_t year[4] = { 1, 9, 9, 6 };
  uint32_t mask = UC_CATEGORY_MASK_L | UC_CATEGORY_MASK_N | UC_CATEGORY_MASK_Pd | UC_CATEGORY_MASK_Sm;

  uint8_t* output = NULL;
  size_t output_len = 0;
  ciph_err_t err = ciph_alloc_year(input, strlen((char*)input), year, mask, &output, &output_len);
  CU_ASSERT(err == CIPH_OK);

  CU_ASSERT(output_len > 0);

  dbgout2(output, output_len, expected, strlen((char*)expected));
  CU_ASSERT(u8_cmp2(output, output_len, expected, strlen((char*)expected)) == 0);
}
