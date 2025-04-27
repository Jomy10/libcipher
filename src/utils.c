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
