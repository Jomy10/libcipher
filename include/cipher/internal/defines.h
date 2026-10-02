#ifndef _CIPH_INTERNAL_DEFINES_H
#define _CIPH_INTERNAL_DEFINES_H

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define EXPORT
#endif

#ifndef CIPH_MALLOC
#define CIPH_MALLOC malloc
#endif

#ifndef CIPH_CALLOC
#define CIPH_CALLOC calloc
#endif

#ifndef CIPH_REALLOC
#define CIPH_REALLOC realloc
#endif

#ifndef CIPH_FREE
#define CIPH_FREE free
#endif

#endif // _CIPH_DEFINES_H
