#ifndef _CIPH_DEFINES_H
#define _CIPH_DEFINES_H

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define EXPORT
#endif

#endif
