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
	ARAutoreleasePoolRef pool = ARAutoreleasePoolCreate();
	
	mky_string_t *hello1 = objStringCreate(ARStringWithFormat("Hello World"));
	mky_string_t *hello2 = objStringCreate(ARStringWithFormat("Hello World"));
	mky_string_t *diff1 = objStringCreate(ARStringWithFormat("My name is johnny"));
	mky_string_t *diff2 = objStringCreate(ARStringWithFormat("My name is johnny"));
	
	ASSERT_TRUE(hashkeyEquals(OBJ_HASHKEY(hello1), OBJ_HASHKEY(hello2)));
	ASSERT_TRUE(hashkeyEquals(OBJ_HASHKEY(diff1), OBJ_HASHKEY(diff2)));
	ASSERT_FALSE(hashkeyEquals(OBJ_HASHKEY(hello1), OBJ_HASHKEY(diff1)));
	
	ARRelease(pool);
}

UTEST_MAIN();