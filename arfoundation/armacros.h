#ifndef _armacros_h_
#define _armacros_h_

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