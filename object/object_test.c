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

    mky_string_t *hello1 = objStringCreate(StringWithFormat("Hello World"));
    mky_string_t *hello2 = objStringCreate(StringWithFormat("Hello World"));
    mky_string_t *diff1 = objStringCreate(StringWithFormat("My name is johnny"));
    mky_string_t *diff2 = objStringCreate(StringWithFormat("My name is johnny"));

    ASSERT_TRUE(HashkeyEquals(OBJ_HASHKEY(hello1), OBJ_HASHKEY(hello2)));
    ASSERT_TRUE(HashkeyEquals(OBJ_HASHKEY(diff1), OBJ_HASHKEY(diff2)));
    ASSERT_FALSE(HashkeyEquals(OBJ_HASHKEY(hello1), OBJ_HASHKEY(diff1)));

    RCRelease(pool);
}

#ifndef AR_COMPOUND_TEST
UTEST_MAIN();
#endif
