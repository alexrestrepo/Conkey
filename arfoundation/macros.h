#ifndef _armacros_h_
#define _armacros_h_

#if !defined(AR_INLINE)
#if defined(__GNUC__)
#define AR_INLINE static __inline__ __attribute__((always_inline))
#elif defined(__cplusplus)
#define AR_INLINE static inline
#endif
#endif

#ifndef MIN
#define MIN(x,y) ((x) <= (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y) ((x) >= (y) ? (x) : (y))
#endif

#ifndef PIN
#define PIN(X, A, B) ((X) < (A) ? (A) : ((X) > (B) ? (B) : (X)))
#endif

#define AS_STR(...) #__VA_ARGS__

#endif
