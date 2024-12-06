#include "SSTriangleColorShader.h"

std::shared_ptr<SSTriangleColorShader> SSCreateTriangleColorShader(
    const GPoint p0,
    const GPoint p1,
    const GPoint p2,
    const GColor color0,
    const GColor color1,
    const GColor color2
) {
    return std::unique_ptr<SSTriangleColorShader>(new SSTriangleColorShader(p0, p1, p2, color0, color1, color2));
}