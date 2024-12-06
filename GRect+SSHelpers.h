#ifndef SSRectHelpers_DEFINED
#define SSRectHelpers_DEFINED

#include "include/GRect.h"

static inline bool GRect_isInside(const GRect &rect, const GRect &bounds) {
    GIRect roundedRect = rect.round();

    return roundedRect.left >= bounds.left
        && roundedRect.right <= bounds.right
        && roundedRect.top >= bounds.top
        && roundedRect.bottom <= bounds.bottom;
}

static inline bool GRect_isOutside(const GRect &rect, const GRect &bounds) {
    GIRect roundedRect = rect.round();

    return roundedRect.left > bounds.right
        || roundedRect.right < bounds.left
        || roundedRect.top > bounds.bottom
        || roundedRect.bottom < bounds.top;
}

static inline bool GIRect_isInside(const GIRect &rect, const GIRect &bounds) {
    return rect.left >= bounds.left
        && rect.right <= bounds.right
        && rect.top >= bounds.top
        && rect.bottom <= bounds.bottom;
}

static inline bool GIRect_isOutside(const GIRect &rect, const GIRect &bounds) {
    return rect.left > bounds.right
        || rect.right < bounds.left
        || rect.top > bounds.bottom
        || rect.bottom < bounds.top;
}

#endif