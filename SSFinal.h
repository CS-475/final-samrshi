#ifndef SSFinal_DEFINED
#define SSFinal_DEFINED

#include "include/GFinal.h"

class SSFinal : public GFinal {
    /// Returns a new type of linear gradient. In this variant, the "count" colors are
    /// positioned along the line p0...p1 not "evenly", but according to the pos[] array.
    ///
    /// pos[] holds "count" values, each 0...1, which specify the percentage along the
    /// line where the each color lies.
    ///
    /// e.g. pos[] = {0, 0.25, 1} would mean 3 colors positioned as follows:
    ///
    ///     color[0] ..... color[1] ..... ..... ..... color[2]
    ///
    /// color[i] is positioned by computing (1 - pos[i])*p0 + pos[i]*p1
    ///
    /// For this API, pos[] will always be monotonic, with p[0] == 0 and p[count-1] == 1.0
    ///
    /// For simplicity, assume that we're using "clamp" tiling.
    std::shared_ptr<GShader> createLinearPosGradient(
        GPoint p0, GPoint p1,
        const GColor colors[],
        const float pos[],
        int count
    );

    /// Returns an instance to a shader that will proxy to a "realShader", and transform
    /// its output using the GColorMatrix provided.
    ///
    /// Note: the GColorMatrix is defined to operate on unpremul GColors
    ///
    /// Note: the resulting colors (after applying the colormatrix) may be out of bounds
    ///       for color componets. If this happens they should be clamped to legal values.
    virtual std::shared_ptr<GShader> createColorMatrixShader(
        const GColorMatrix&,
        GShader* realShader
    );

    /// The vornoi shader is defined by an array of points, each with an associated color.
    /// The color any any (x,y) is the color of the closest point from the array.
    virtual std::shared_ptr<GShader> createVoronoiShader(
        const GPoint points[],
        const GColor colors[],
        int count
    );
};

#endif