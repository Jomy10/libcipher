#include <cipher/internal/unistring_config.h>
#include <cipher/internal/utils.h>
#include <uniwbrk.h>

bool ciph_uc_is_wordbreak(ucs4_t uc) {
  switch (uc_wordbreak_property(uc)) {
    case WBP_CR:
    case WBP_LF:
    case WBP_NEWLINE:
    case WBP_ZWJ:
    case WBP_FORMAT:
    case WBP_SQ: // SINGLE QUOTE
    case WBP_DQ: // DOUBLE QUOTE
    case WBP_MIDNUM: // punct
    case WBP_MIDLETTER: // ...
    case WBP_MIDNUMLET: // ...
    case WBP_EXTENDNUMLET:
    case WBP_WSS: // WSegSpace
      return true;
    default:
      return false;
  }
}

#ifdef __EMSCRIPTEN__
#include <cipher/ciphers.h>

uint32_t ciph_year_include_mask_letters() {
  return CIPH_YEAR_INCLUDE_MASK_LETTERS;
}
uint32_t ciph_year_include_mask_letters_and_numbers() {
  return CIPH_YEAR_INCLUDE_MASK_LETTERS_AND_NUMBERS;
}
uint32_t ciph_year_include_mask_with_symbols() {
  return CIPH_YEAR_INCLUDE_MASK_WITH_SYMBOLS;
}
uint32_t ciph_year_include_mask_with_symbols_and_dashes() {
  return CIPH_YEAR_INCLUDE_MASK_WITH_SYMBOLS_AND_DASHES;
}
#endif
