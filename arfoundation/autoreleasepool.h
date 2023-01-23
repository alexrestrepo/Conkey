//
//  arautoreleasepool.h
//  Created by Alex Restrepo on 1/16/23.
//
//  Naive implementation, a real one should have a pool/stack per thread.

#ifndef _autoreleasepool_h_
#define _autoreleasepool_h_

#include "runtime.h"

#define autoreleasepool(...) \
{ \
    AutoreleasePoolRef _autoreleasepool = AutoreleasePoolCreate(); \
    __VA_ARGS__ \
    RCRelease(_autoreleasepool); \
}

typedef struct AutoreleasePool *AutoreleasePoolRef;

void AutoreleasePoolInitialize(void);
AutoreleasePoolRef AutoreleasePoolCreate(void);
AutoreleasePoolRef CurrentAutoreleasePool(void);
void AutoreleasePoolAddObject(AutoreleasePoolRef pool, RCTypeRef obj);
void AutoreleasePoolDrain(AutoreleasePoolRef pool);

#endif /* arautoreleasepool_h */
