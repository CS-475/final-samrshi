#ifndef SSMath_DEFINED
#define SSMath_DEFINED

#include "include/GMath.h"

const float floatTolerance = 0.001f;

static inline bool nearlyZero(float value) {
    return abs(value) < floatTolerance;
}

static inline bool nearlyEqual(float lhs, float rhs) {
    return abs(lhs - rhs) < floatTolerance;
}

static inline int SSRoundPositiveToInt(const float& f) {
    assert(f > -floatTolerance);
    return (int) (f + 0.5f);
}

static inline int SSFloorPositiveToInt(const float& f) {
    assert(f > -floatTolerance);
    return (int) f;
}

static inline float SSClamp(float value, float min, float max) {
    if (value < min) {
        return min;
    } else if (value > max) {
        return max;
    } else {
        return value;
    }
}

#endif // SSMath_DEFINED