//
//  ARRange.h
//  conkey
//
//  Created by Alex Restrepo on 1/16/23.
//

#ifndef _ARRange_h_
#define _ARRange_h_

#include <stdbool.h>
#include <stdint.h>

#include "armacros.h"

typedef struct {
    uint64_t location;
    uint64_t length;
} ARRange;

AR_INLINE ARRange ARMakeRange(uint64_t loc, uint64_t len) {
    return (ARRange){loc, len};
}

AR_INLINE uint64_t ARMaxRange(ARRange range) {
    return range.location + range.length;
}

AR_INLINE bool ARLocationInRange(uint64_t loc, ARRange range) {
    return loc - range.location < range.length;
}

AR_INLINE bool AREqualRanges(ARRange range1, ARRange range2) {
    return range1.location == range2.location && range1.length == range2.length;
}

AR_INLINE ARRange NSUnionRange(ARRange range1, ARRange range2) {
    uint64_t loc = MIN(range1.location, range2.location);
    uint64_t max = MAX(ARMaxRange(range1), ARMaxRange(range2));
    return ARMakeRange(loc, max - loc);
}

AR_INLINE ARRange NSIntersectionRange(ARRange range1, ARRange range2) {
    uint64_t loc = MAX(range1.location, range2.location);
    uint64_t max = MIN(ARMaxRange(range1), ARMaxRange(range2));
    if (max <= loc) {
        return ARMakeRange(0, 0);
    }
    return ARMakeRange(loc, max - loc);
}

#endif /* ARRange_h */
