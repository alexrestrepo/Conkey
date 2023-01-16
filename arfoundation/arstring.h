//
//  arstring.h
//  Created by Alex Restrepo on 1/14/23.
//

#ifndef _arstring_h_
#define _arstring_h_

#include <stdio.h>

#include "ARRange.h"

typedef struct ARString *ARStringRef;

void ARStringInitialize(void);

ARStringRef ARStringCreateWithFormat(const char *fmt, ...) __printflike(1, 2);
ARStringRef ARStringWithFormat(const char *fmt, ...) __printflike(1, 2);
ARStringRef ARStringAppendFormat(ARStringRef str, const char *fmt, ...) __printflike(2, 3);
size_t ARStringLength(ARStringRef str);
const char *ARStringCString(ARStringRef str);

#endif /* arstring_h */
