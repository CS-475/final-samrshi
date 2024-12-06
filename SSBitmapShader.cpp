#include "SSBitmapShader.h"

std::shared_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix, GTileMode mode) {
    /// TODO: Add support for tile mode
    return std::unique_ptr<GShader>(new SSBitmapShader(bitmap, localMatrix, mode));
}