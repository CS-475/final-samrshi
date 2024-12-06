#ifndef SSColorMatrixHelpers_DEFINED
#define SSColorMatrixHelpers_DEFINED

#include "include/GFinal.h"

static inline GColor operator*(const GColorMatrix& matrix, const GColor& color) {
    float baseR = color.r;
    float baseG = color.g;
    float baseB = color.b;
    float baseA = color.a;

    float transformedR = (matrix[ 0] * baseR) + (matrix[ 4] * baseG) + (matrix[ 8] * baseB) + (matrix[12] * baseA) + matrix[16];
    float transformedG = (matrix[ 1] * baseR) + (matrix[ 5] * baseG) + (matrix[ 9] * baseB) + (matrix[13] * baseA) + matrix[17];
    float transformedB = (matrix[ 2] * baseR) + (matrix[ 6] * baseG) + (matrix[10] * baseB) + (matrix[14] * baseA) + matrix[18];
    float transformedA = (matrix[ 3] * baseR) + (matrix[ 7] * baseG) + (matrix[11] * baseB) + (matrix[15] * baseA) + matrix[19];
    
    GColor newColor = GColor::RGBA(transformedR, transformedG, transformedB, transformedA);
    newColor = newColor.pinToUnit();
    return newColor;
}

#endif