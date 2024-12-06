#ifndef SSLinearGradientShaderOneColor_DEFINED
#define SSLinearGradientShaderOneColor_DEFINED

#include "include/GShader.h"
#include "include/GMatrix.h"
#include "SSBlendModeHelpers.h"

class SSLinearGradientShaderOneColor : public GShader {
private:
    const GPoint p0;
    const GPoint p1;
    const GColor color;
    const GPixel pixel;

public:
    /// Instantatiates an SSBitmapShader with a bitmap and local matrix.
    SSLinearGradientShaderOneColor(GPoint p0, GPoint p1, const GColor color) 
        : p0(p0)
        , p1(p1)
        , color(color)
        , pixel(colorToPixel(color))
    {}

    /// Return true iff all of the GPixels that may be returned by this shader will be opaque.
    bool isOpaque() override {
        return color.a == 1;
    }

    /// The draw calls in GCanvas must call this with the CTM before any calls to shadeSpan().
    bool setContext(const GMatrix& ctm) override {
        return true;
    }

    /// Given a row of pixels in device space [x, y] ... [x + count - 1, y], return the
    /// corresponding src pixels in row[0...count - 1]. The caller must ensure that row[]
    /// can hold at least [count] entries.
    void shadeRow(int x, int y, int count, GPixel row[]) override {
        const GPixel pixel = this->pixel;

        for (int i = 0; i < count; i++) {
            row[i] = pixel;
        }
    }
};

#endif // SSLinearGradientShaderOneColor_DEFINED