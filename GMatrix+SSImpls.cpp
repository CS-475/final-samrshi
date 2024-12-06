#include "include/GMatrix.h"

/// Initializes an identity matrix
GMatrix::GMatrix() : GMatrix(
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f
) {}

/// Creates a translation matrix with tx and ty
GMatrix GMatrix::Translate(float tx, float ty) {
    return GMatrix(
        1.0f, 0.0f, tx,
        0.0f, 1.0f, ty
    );
}

/// Creates a scaling matrix with sx and sy
GMatrix GMatrix::Scale(float sx, float sy) {
    return GMatrix(
        sx  , 0.0f, 0.0f,
        0.0f, sy  , 0.0f
    );
}

/// Creates a rotation matrix with radians
GMatrix GMatrix::Rotate(float radians) {
    float sin = std::sin(radians);
    float cos = std::cos(radians);

    return GMatrix(
        cos, -sin, 0.0f,
        sin,  cos, 0.0f
    );
}

/// Return the product of two matrices: a * b
GMatrix GMatrix::Concat(const GMatrix& a, const GMatrix& b) {
    return GMatrix(
        a[0]*b[0] + a[2]*b[1],        // a*a' + c*b' + e*0
        a[0]*b[2] + a[2]*b[3],        // a*c' + c*d' + e*0
        a[0]*b[4] + a[2]*b[5] + a[4], // a*e' + c*f' + e*1

        a[1]*b[0] + a[3]*b[1],        // b*a' + d*b' + f*0
        a[1]*b[2] + a[3]*b[3],        // b*c' + d*d' + f*0
        a[1]*b[4] + a[3]*b[5] + a[5]  // b*e' + d*f' + f*1
    );
}

/// If the inverse exists, return it, else return {} to signal no return value.
nonstd::optional<GMatrix> GMatrix::invert() const {
    // Find determinant (ad - bc)
    float determinant = fMat[0] * fMat[3] - fMat[1] * fMat[2];

    // If matrix isn't invertible, return {}
    if (std::abs(determinant) < 1e-6f) {
        return {};
    }

    // Calculate the inverse matrix and return
    float invDet = 1.0f / determinant;

    return GMatrix(
         fMat[3] * invDet,                                //         d / (ad - bc)
        -fMat[2] * invDet,                                //        -c / (ad - bc)
        (fMat[2] * fMat[5] - fMat[3] * fMat[4]) * invDet, // (cf - de) / (ad - bc)

        -fMat[1] * invDet,                                //        -b / (ad - bc)
         fMat[0] * invDet,                                //         a / (ad - bc)
        (fMat[1] * fMat[4] - fMat[0] * fMat[5]) * invDet  // (be - af) / (ad - bc)
    );
}

/// Transform the set of points in src, storing the resulting points in dst, by applying this
/// matrix. It is the caller's responsibility to allocate dst to be at least as large as src.
///
/// [ a  c  e ] [ x ]     x' = ax + cy + e
/// [ b  d  f ] [ y ]     y' = bx + dy + f
/// [ 0  0  1 ] [ 1 ]
///
/// Note: It is legal for src and dst to point to the same memory (however, they may not
/// partially overlap). Thus the following is supported.
///
/// GPoint pts[] = { ... };
/// matrix.mapPoints(pts, pts, count);
///
void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    for (int i = 0; i < count; i++) {
        GPoint srcPoint = src[i];
        dst[i].x = (fMat[0] * srcPoint.x) + (fMat[2] * srcPoint.y) + fMat[4];
        dst[i].y = (fMat[1] * srcPoint.x) + (fMat[3] * srcPoint.y) + fMat[5];
    }
}