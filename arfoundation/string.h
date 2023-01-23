//
//  arstring.h
//  Created by Alex Restrepo on 1/14/23.
//

#ifndef _arstring_h_
#define _arstring_h_

#include <stdio.h>

#include "range.h"

typedef struct ARString *StringRef;

void StringInitialize(void);

StringRef StringCreateWithFormat(const char *fmt, ...) __printflike(1, 2);

StringRef StringWithFormat(const char *fmt, ...) __printflike(1, 2);
StringRef String(void);
StringRef StringWithChars(const char *str);
StringRef StringWithString(StringRef str);

void StringAppendFormat(StringRef str, const char *fmt, ...) __printflike(2, 3);
void StringAppendString(StringRef str, StringRef append);
void StringAppendChars(StringRef str, const char *chars);

size_t StringLength(StringRef str);
const char *CString(StringRef str);

#endif /* arstring_h */
