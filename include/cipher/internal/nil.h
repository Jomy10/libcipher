#ifndef nil
#define nil (void*)0
#endif

#ifndef nonnil
  #if defined(__clang__)
    #define nonnil _Nonnull
  #elif defined(__GNUC__)
    #define nonnil __attribute__((nonnull))
  #else
    #define nonnil
  #endif
#endif

#ifndef nilable
  #if defined(__clang__)
    #define nilable _Nullable
  #else
    #define nilable
  #endif
#endif
