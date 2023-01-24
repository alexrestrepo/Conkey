//
//  object_test.c
//  conkey
//
//  Created by Alex Restrepo on 1/20/23.
//

#include "../arfoundation/vendor/utest.h"
#include "../arfoundation/arfoundation.h"
#include "object.h"

UTEST(object, stringHashKeys) {
    AutoreleasePoolRef pool = AutoreleasePoolCreate();

    MkyObject *hello1 = mkyString(StringWithFormat("Hello World"));
    MkyObject *hello2 = mkyString(StringWithFormat("Hello World"));
    MkyObject *diff1 = mkyString(StringWithFormat("My name is johnny"));
    MkyObject *diff2 = mkyString(StringWithFormat("My name is johnny"));

    ASSERT_TRUE(HashkeyEquals(mkyHashKey(hello1), mkyHashKey(hello2)));
    ASSERT_TRUE(HashkeyEquals(mkyHashKey(diff1), mkyHashKey(diff2)));
    ASSERT_FALSE(HashkeyEquals(mkyHashKey(hello1), mkyHashKey(diff1)));

    RCRelease(pool);
}

#ifndef AR_COMPOUND_TEST
UTEST_MAIN();
#endif
