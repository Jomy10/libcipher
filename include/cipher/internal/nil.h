#ifndef nil
#define nil 0
#endif

#ifndef nonnil
  #if defined(__GNUC__)
    #define nonnil __attribute__((nonnull))
  #else
    #define nonnil
  #endif
#endif

#ifndef nilable
  #if defined(__GNUC__)
    #define nilable __attribute__((nullable))
  #else
    #define nilable
  #endif
#endif
