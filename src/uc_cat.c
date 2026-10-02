#include <cipher/uc_cat.h>
#include <uniwbrk.h>
#include <unictype.h>

bool ciph_uc_is_wordbreak(ucs4_t uc) {
  if (uc_is_general_category(uc, UC_DASH_PUNCTUATION)) {
    return false;
  }

  if (uc_is_general_category(uc, UC_PUNCTUATION)) {
    return true;
  }

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

bool ciph_uc_is_sentence_terminal(ucs4_t uc) {
  return uc_is_property_sentence_terminal(uc) || uc_is_property_terminal_punctuation(uc);
}

void* ciph_fnptr_uc_is_sentence_terminal() {
  return (void*)&ciph_uc_is_sentence_terminal;
}

void* ciph_fnptr_uc_is_wordbreak() {
  return (void*)&ciph_uc_is_wordbreak;
}

uint32_t ciph_char_include_letters() {
  return CIPH_CHAR_INCLUDE_LETTERS;
}

uint32_t ciph_char_include_numbers() {
  return CIPH_CHAR_INCLUDE_NUMBERS;
}

uint32_t ciph_char_include_symbols() {
  return CIPH_CHAR_INCLUDE_SYMBOLS;
}

uint32_t ciph_char_include_dashes() {
  return CIPH_CHAR_INCLUDE_DASHES;
}

#endif
