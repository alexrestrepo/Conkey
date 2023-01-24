#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../vendor/utest.h"
#include "../arfoundation.h"


UTEST(arfoundation, ARString) {
    AutoreleasePoolRef ap = AutoreleasePoolCreate();
    
    StringRef string = StringCreateWithFormat("Hello %s", "world!");
    StringRef desc = RuntimeDescription(string); // auto
    RCRetain(desc); // +1
    
    //  fprintf(stderr, "'%s' -> [%s]\n", CString(string), CString(desc));
    
    ASSERT_EQ(1, RuntimeRefCount(string));
    ASSERT_EQ(2, RuntimeRefCount(desc));
    
    RCRetain(string);
    RCRetain(desc);
    
    ASSERT_EQ(2, RuntimeRefCount(string));
    ASSERT_EQ(3, RuntimeRefCount(desc));
    
    string = RCRelease(string);
    desc = RCRelease(desc);
    
    ASSERT_EQ(1, RuntimeRefCount(string));
    ASSERT_EQ(2, RuntimeRefCount(desc));
    
    string = RCRelease(string);
    desc = RCRelease(desc);
    ASSERT_EQ(1, RuntimeRefCount(desc));
    
    ASSERT_EQ(NULL, string);
    //	ASSERT_EQ(NULL, desc);
    RCRelease(ap);
    
    // desc is dangling here...
} 

UTEST(arfoundation, nonClassRefCount) {
    const char *base = "Hello there, this is fine.";
    StringRef someString = StringCreateWithFormat("%s", base);
    
    ASSERT_EQ(strlen(base), StringLength(someString));
    ASSERT_STREQ(base, CString(someString));
    
    char *refcounted = RuntimeRCAlloc(sizeof(char) * StringLength(someString) + 1, (RuntimeClassID){0});
    sprintf(refcounted, "%s", CString(someString));
    
    ASSERT_EQ(strlen(refcounted), StringLength(someString));
    ASSERT_STREQ(refcounted, CString(someString));
    
    refcounted = RCRelease(refcounted);
    ASSERT_EQ(NULL, refcounted);
    
    someString = RCRelease(someString);
    ASSERT_EQ(NULL, someString);

    AutoreleasePoolRef ap = AutoreleasePoolCreate();
    someString = StringWithFormat("HELLO");
    ASSERT_EQ(5, StringLength(someString));
    ASSERT_STREQ("HELLO", CString(someString));

    refcounted = RuntimeRCAlloc(sizeof(char) * StringLength(someString) + 1, (RuntimeClassID){0});
    sprintf(refcounted, "%s", CString(someString));
    ASSERT_EQ(strlen(refcounted), StringLength(someString));
    ASSERT_STREQ(refcounted, CString(someString));
    ASSERT_EQ(1, RuntimeRefCount(refcounted));

    //  fprintf(stderr, "%s -> %s\n", refcounted, CString(RuntimeDescription(refcounted)));
    //  fprintf(stderr, "%s\n", CString(RuntimeDescription(someString)));

    RCAutorelease(refcounted);
    RCRelease(ap);
}

UTEST(arfoundation, autoreleasePool) {
    AutoreleasePoolRef ap = AutoreleasePoolCreate();
    
    for (int j = 1; j < 5; j++) {
        AutoreleasePoolRef inner = AutoreleasePoolCreate();
        ASSERT_EQ(inner, CurrentAutoreleasePool());
        
        for (int i = 0; i < 5; i++) {
            StringRef str = StringWithFormat("hello there");
            ASSERT_EQ(1, RuntimeRefCount(str));
        }
        inner = RCRelease(inner);
        ASSERT_EQ(NULL, inner);
    }
    
    StringRef str = StringWithFormat("hello there");
    ASSERT_EQ(1, RuntimeRefCount(str));
    
    str = RuntimeMakeConstant(str);
    ASSERT_EQ(AR_RUNTIME_REFCOUNT_UNRELEASABLE, RuntimeRefCount(str));
    
    //  fprintf(stderr, "str = [%s] %s\n", CString(RuntimeDescription(str)), CString(str));
    //  fprintf(stderr, "ap = [%s]\n", CString(RuntimeDescription(ap)));
    
    ap = RCRelease(ap);
    ASSERT_EQ(NULL, ap);
    
    str = RCRelease(str);
    ASSERT_TRUE(str);
}

UTEST(arfoundation, containers) {
    AutoreleasePoolRef ap = AutoreleasePoolCreate();
    
    StringRef key = StringWithChars("hello");
    ASSERT_EQ(1, RuntimeRefCount(key));
    
    StringRef value = StringWithChars("world");
    ASSERT_EQ(1, RuntimeRefCount(value));
    
    ArrayRef array = ArrayCreate();
    ArrayAppend(array, key);
    ArrayAppend(array, value);
    
    ArrayAppend(array, StringWithChars("autoreleased"));
    ArrayAppend(array, StringWithChars("autoreleased as well."));
    
    ASSERT_EQ(2, RuntimeRefCount(key));
    ASSERT_EQ(2, RuntimeRefCount(value));
    
    ap = RCRelease(ap);
    ASSERT_EQ(1, RuntimeRefCount(key));
    ASSERT_EQ(1, RuntimeRefCount(value));
    
    RCRetain(key);
    RCRetain(value);
    
    array = RCRelease(array);
    ASSERT_EQ(NULL, array);
    
    DictionaryRef dict = DictionaryCreate();
    DictionarySetObjectForKey(dict, key, value);
    ASSERT_EQ(2, RuntimeRefCount(key));
    ASSERT_EQ(2, RuntimeRefCount(value));
    
    ASSERT_EQ(DictionaryObjectForKey(dict, key), value);
    
    StringRef another = StringCreateWithFormat("oh hai");
    DictionarySetObjectForKey(dict, key, another);
    another = RCRelease(another);
    
    ASSERT_EQ(2, RuntimeRefCount(key));
    ASSERT_EQ(1, RuntimeRefCount(value));
    ASSERT_EQ(1, RuntimeRefCount(another));
    
    ASSERT_EQ(DictionaryObjectForKey(dict, key), another);
    RCRetain(another);
    
    StringRef toreplace = StringCreateWithChars("doomed");
    DictionarySetObjectForKey(dict, key, toreplace);
    toreplace = RCRelease(toreplace);
    ASSERT_EQ(1, RuntimeRefCount(toreplace));
    ASSERT_EQ(DictionaryObjectForKey(dict, key), toreplace);
    
    ap = AutoreleasePoolCreate();
    ASSERT_EQ(DictionaryObjectForKey(dict, StringWithChars("hello")), toreplace);
    
    DictionarySetObjectForKey(dict, key, StringWithChars("autoreleased too"));
    ASSERT_NE(DictionaryObjectForKey(dict, key), another);
    ap = RCRelease(ap);
    
    dict = RCRelease(dict);
    ASSERT_EQ(NULL, dict);
    
    key = RCRelease(key);
    ASSERT_EQ(NULL, key);
    
    value = RCRelease(value);
    ASSERT_EQ(NULL, value);
    
    another = RCRelease(another);
    ASSERT_EQ(NULL, another);
}

#ifndef AR_COMPOUND_TEST
UTEST_MAIN();
#endif
