#include "SSLinearGradientShaderOneColor.h"
#include "SSLinearGradientShaderTwoColors.h"
#include "SSLinearGradientShaderManyColors.h"

/// Return a subclass of GShader that draws the specified gradient of [count] colors between
/// the two points. Color[0] corresponds to p0, and Color[count-1] corresponds to p1, and all
/// intermediate colors are evenly spaced between.
///
/// The gradient colors are GColors, and therefore unpremul. The output colors (in shadeRow)
/// are GPixel, and therefore premul. The gradient has to interpolate between pairs of GColors
/// before "pre" multiplying them into GPixels.
///
/// If count < 1, this should return nullptr.
std::shared_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GTileMode mode) {
    /// TODO: Add support for tile mode
    
    if (count < 1) {
        return nullptr;
    } else if (count == 1) {
        return std::unique_ptr<GShader>(new SSLinearGradientShaderOneColor(p0, p1, colors[0]));
    } else if (count == 2) {
        return std::unique_ptr<GShader>(new SSLinearGradientShaderTwoColors(p0, p1, colors[0], colors[1], mode));
    } else {
        return std::unique_ptr<GShader>(new SSLinearGradientShaderManyColors(p0, p1, colors, count, mode));
    }
}