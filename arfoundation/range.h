//
//  range.h
//  conkey
//
//  Created by Alex Restrepo on 1/16/23.
//

#ifndef _range_h_
#define _range_h_

#include <stdbool.h>
#include <stdint.h>

#include "macros.h"

typedef struct {
    uint64_t location;
    uint64_t length;
} Range;

AR_INLINE Range MakeRange(uint64_t loc, uint64_t len) {
    return (Range){loc, len};
}

AR_INLINE uint64_t MaxRange(Range range) {
    return range.location + range.length;
}

AR_INLINE bool IsLocationInRange(uint64_t loc, Range range) {
    return loc - range.location < range.length;
}

AR_INLINE bool AreRangesEqual(Range range1, Range range2) {
    return range1.location == range2.location && range1.length == range2.length;
}

AR_INLINE Range UnionRange(Range range1, Range range2) {
    uint64_t loc = MIN(range1.location, range2.location);
    uint64_t max = MAX(MaxRange(range1), MaxRange(range2));
    return MakeRange(loc, max - loc);
}

AR_INLINE Range IntersectionRange(Range range1, Range range2) {
    uint64_t loc = MAX(range1.location, range2.location);
    uint64_t max = MIN(MaxRange(range1), MaxRange(range2));
    if (max <= loc) {
        return MakeRange(0, 0);
    }
    return MakeRange(loc, max - loc);
}

#endif /* range_h */
