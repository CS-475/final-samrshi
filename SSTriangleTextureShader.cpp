#include "SSTriangleTextureShader.h"

std::shared_ptr<SSTriangleTextureShader> SSCreateTriangleTextureShader(
    std::shared_ptr<GShader> baseShader,
    const GPoint point0,
    const GPoint point1,
    const GPoint point2,
    const GPoint texturePoint0,
    const GPoint texturePoint1,
    const GPoint texturePoint2
) {
    return std::unique_ptr<SSTriangleTextureShader>(new SSTriangleTextureShader(
        baseShader,
        point0, point1, point2,
        texturePoint0, texturePoint1, texturePoint2
    ));
}