#ifndef _CIPH_INTERNAL_UTILS_H
#define _CIPH_INTERNAL_UTILS_H

#include <unitypes.h>
#include <stdbool.h>

#include "internal/defines.h"
#include "internal/nil.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Returns true if the codepoint is a character that separates words.
EXPORT bool ciph_uc_is_wordbreak(ucs4_t uc);

#ifdef __EMSCRIPTEN__

EXPORT bool ciph_uc_is_sentence_terminal(ucs4_t uc);

EXPORT void* ciph_fnptr_uc_is_sentence_terminal();
EXPORT void* ciph_fnptr_uc_is_wordbreak();

#endif

#ifdef __cplusplus
}
#endif

#endif // _CIPH_UTILS_H
