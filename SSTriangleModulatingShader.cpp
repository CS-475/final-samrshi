#include "SSTriangleModulatingShader.h"

std::shared_ptr<SSTriangleModulatingShader> SSCreateModulatingShader(
    std::shared_ptr<SSTriangleColorShader> colorShader,
    std::shared_ptr<SSTriangleTextureShader> textureShader
) {
    return std::unique_ptr<SSTriangleModulatingShader>(new SSTriangleModulatingShader(colorShader, textureShader));
}