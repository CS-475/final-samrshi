#ifndef SSPointHelpers_DEFINED
#define SSPointHelpers_DEFINED

#include "include/GPoint.h"

static inline float GPoint_distance(const GPoint& a, const GPoint& b) {
    float xDistance = b.x - a.x;
    float yDistance = b.y - a.y;
    return sqrt(xDistance * xDistance + yDistance * yDistance);
}

#endif