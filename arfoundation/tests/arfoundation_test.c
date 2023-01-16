#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../vendor/utest.h"
#include "../arfoundation.h"


UTEST(arfoundation, ARString) {
    ARAutoreleasePoolRef ap = ARAutoreleasePoolCreate();
    
    ARStringRef string = ARStringCreateWithFormat("Hello %s", "world!");
    ARStringRef desc = ARRuntimeDescription(string); // auto
    ARRetain(desc); // +1
    
    fprintf(stderr, "'%s' -> [%s]\n", ARStringCString(string), ARStringCString(desc));
    
    ASSERT_EQ(1, ARRuntimeRefCount(string));
    ASSERT_EQ(2, ARRuntimeRefCount(desc));
    
    ARRetain(string);
    ARRetain(desc);
    
    ASSERT_EQ(2, ARRuntimeRefCount(string));
    ASSERT_EQ(3, ARRuntimeRefCount(desc));
    
    string = ARRelease(string);
    desc = ARRelease(desc);
    
    ASSERT_EQ(1, ARRuntimeRefCount(string));
    ASSERT_EQ(2, ARRuntimeRefCount(desc));
    
    string = ARRelease(string);
    desc = ARRelease(desc);
    ASSERT_EQ(1, ARRuntimeRefCount(desc));
    
    ASSERT_EQ(NULL, string);
    //	ASSERT_EQ(NULL, desc);
    ARRelease(ap);
    
    // desc is dangling here...
} 

UTEST(arfoundation, nonClassRefCount) {
    const char *base = "Hello there, this is fine.";
    ARStringRef someString = ARStringCreateWithFormat("%s", base);
    
    ASSERT_EQ(strlen(base), ARStringLength(someString));
    ASSERT_STREQ(base, ARStringCString(someString));
    
    char *refcounted = ARRuntimeAllocRefCounted(sizeof(char) * ARStringLength(someString) + 1, (ar_class_id){0});
    sprintf(refcounted, "%s", ARStringCString(someString));
    
    ASSERT_EQ(strlen(refcounted), ARStringLength(someString));
    ASSERT_STREQ(refcounted, ARStringCString(someString));
    
    refcounted = ARRelease(refcounted);
    ASSERT_EQ(NULL, refcounted);
    
    someString = ARRelease(someString);
    ASSERT_EQ(NULL, someString);

    ARAutoreleasePoolRef ap = ARAutoreleasePoolCreate();
    someString = ARStringWithFormat("HELLO");
    ASSERT_EQ(5, ARStringLength(someString));
    ASSERT_STREQ("HELLO", ARStringCString(someString));

    refcounted = ARRuntimeAllocRefCounted(sizeof(char) * ARStringLength(someString) + 1, (ar_class_id){0});
    sprintf(refcounted, "%s", ARStringCString(someString));
    ASSERT_EQ(strlen(refcounted), ARStringLength(someString));
    ASSERT_STREQ(refcounted, ARStringCString(someString));
    ASSERT_EQ(1, ARRuntimeRefCount(refcounted));

    fprintf(stderr, "%s -> %s\n", refcounted, ARStringCString(ARRuntimeDescription(refcounted)));
    fprintf(stderr, "%s\n", ARStringCString(ARRuntimeDescription(someString)));

    ARAutorelease(refcounted);
    ARRelease(ap);
}

UTEST(arfoundation, autoreleasePool) {
    ARAutoreleasePoolRef ap = ARAutoreleasePoolCreate();
    
    for (int j = 1; j < 5; j++) {
        ARAutoreleasePoolRef inner = ARAutoreleasePoolCreate();
        ASSERT_EQ(inner, ARAutoreleasePoolGetCurrent());
        
        for (int i = 0; i < 5; i++) {
            ARStringRef str = ARStringWithFormat("hello there");
            ASSERT_EQ(1, ARRuntimeRefCount(str));
        }
        inner = ARRelease(inner);
        ASSERT_EQ(NULL, inner);
    }
    
    ARStringRef str = ARStringWithFormat("hello there");
    ASSERT_EQ(1, ARRuntimeRefCount(str));
    
    str = ARRuntimeMakeConstant(str);
    ASSERT_EQ(AR_RUNTIME_UNRELEASABLE, ARRuntimeRefCount(str));
    
    fprintf(stderr, "str = [%s] %s\n", ARStringCString(ARRuntimeDescription(str)), ARStringCString(str));
    fprintf(stderr, "ap = [%s]\n", ARStringCString(ARRuntimeDescription(ap)));
    
    ap = ARRelease(ap);
    ASSERT_EQ(NULL, ap);
    
    str = ARRelease(str);
    ASSERT_TRUE(str);
}

UTEST_MAIN();
