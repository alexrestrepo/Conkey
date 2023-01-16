//
//  arautoreleasepool.h
//  Created by Alex Restrepo on 1/16/23.
//
//  Naive implementation, a real one should have a pool/stack per thread.

#ifndef _arautoreleasepool_h_
#define _arautoreleasepool_h_

#include "arruntime.h"

typedef struct ARAutoreleasePool *ARAutoreleasePoolRef;

void ARAutoreleasePoolInitialize(void);
ARAutoreleasePoolRef ARAutoreleasePoolCreate(void);
ARAutoreleasePoolRef ARAutoreleasePoolGetCurrent(void);
void ARAutoreleasePoolAddObject(ARAutoreleasePoolRef pool, ARObjectRef obj);
void ARAutoreleasePoolDrain(ARAutoreleasePoolRef pool);

#endif /* arautoreleasepool_h */
