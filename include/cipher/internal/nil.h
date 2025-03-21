#ifndef nil
#define nil 0
#endif

#ifndef nonnil
  #ifdef __nonnull
    #define nonnil __nonnull
  #elif defined(__attribute__)
    #define nonnil __attribute__((nonnull))
  #else
    #define nonnil
  #endif
#endif

#ifndef nilable
  #ifdef __nullable
    #define nilable __nullable
  #elif defined(__attribute__)
    #define nilable __attribute__((nullable))
  #else
    #define nilable
  #endif
#endif
