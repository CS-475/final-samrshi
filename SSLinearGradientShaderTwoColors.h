#ifndef SSLinearGradientShaderTwoColors_DEFINED
#define SSLinearGradientShaderTwoColors_DEFINED

#include "include/GMatrix.h"
#include "include/GShader.h"
#include "SSBlendModeHelpers.h"

class SSLinearGradientShaderTwoColors : public GShader {
public:
    /// Instantatiates an SSLinearGradientShaderTwoColors with a bitmap and local matrix.
    SSLinearGradientShaderTwoColors(
        GPoint p0,
        GPoint p1,
        const GColor color0,
        const GColor color1,
        GTileMode tileMode
    ) 
        : p0(p0)
        , p1(p1)
        , color0(color0)
        , color1(color1)
        , colorDifference(color1 - color0)
        , constIsOpaque(color0.a == 1 && color1.a == 1)
        , tileMode(tileMode)
    {
        // Build unit space –> device coordinates matrix
        float dx = p1.x - p0.x;
        float dy = p1.y - p0.y;

        this->unitToDeviceMatrix = GMatrix(
            dx, -dy, p0.x,
            dy, dx, p0.y
        );
    }

    /// Return true iff all of the GPixels that may be returned by this shader will be opaque.
    bool isOpaque() override {
        return constIsOpaque;
    }

    /// The draw calls in GCanvas must call this with the CTM before any calls to shadeSpan().
    bool setContext(const GMatrix& ctm) override {
        auto inverseMatrix = (ctm * unitToDeviceMatrix).invert();

        if (inverseMatrix) {
            this->inverseMatrix = inverseMatrix.value();
            return true;
        } else {
            return false;
        }
    }

    /// Given a row of pixels in device space [x, y] ... [x + count - 1, y], return the
    /// corresponding src pixels in row[0...count - 1]. The caller must ensure that row[]
    /// can hold at least [count] entries.
    void shadeRow(int x, int y, int count, GPixel row[]) override {
        if (constIsOpaque) {
            switch (tileMode) {
            case GTileMode::kClamp: {
                shadeRowCommon(x, y, count, row, colorToPixelOpaque, GPinToUnit);
                break;
            }
            case GTileMode::kRepeat: {
                shadeRowCommon(x, y, count, row, colorToPixelOpaque, repeat);
                break;
            }
            case GTileMode::kMirror: {
                shadeRowCommon(x, y, count, row, colorToPixelOpaque, mirror);
                break;
            }
            }
        } else {
            switch (tileMode) {
            case GTileMode::kClamp: {
                shadeRowCommon(x, y, count, row, colorToPixel, GPinToUnit);
                break;
            }
            case GTileMode::kRepeat: {
                shadeRowCommon(x, y, count, row, colorToPixel, repeat);
                break;
            }
            case GTileMode::kMirror: {
                shadeRowCommon(x, y, count, row, colorToPixel, mirror);
                break;
            }
            }
        }
    }

private:
    const GPoint p0;
    const GPoint p1;
    const GColor color0;
    const GColor color1;
    const GColor colorDifference;
    const bool constIsOpaque;

    GMatrix unitToDeviceMatrix;
    GMatrix inverseMatrix;

    GTileMode tileMode;

    static float repeat(float x) {
        return x - GFloorToInt(x);
    }

    static float mirror(float x) {
        float period = 2.0f;
        return 2 * abs((x / period) - GFloorToInt((x / period) + 0.5f));
    }

    template <typename ColorToPixelFunction, typename ClampFunction>
    void shadeRowCommon(
        int x,
        int y,
        int count,
        GPixel row[],
        ColorToPixelFunction colorToPixelFunction,
        ClampFunction clampFunction
    ) {
        const float fx = static_cast<float>(x);
        const float fy = static_cast<float>(y);

        const float fEndX = static_cast<float>(x + count - 1);

        float unitX = (inverseMatrix[0] * (fx + 0.5f)) + (inverseMatrix[2] * (fy + 0.5f)) + inverseMatrix[4];
        float unitEndX = (inverseMatrix[0] * (fEndX + 0.5f)) + (inverseMatrix[2] * (fy + 0.5f)) + inverseMatrix[4];

        float a = inverseMatrix[0];

        bool startXInUnit = 0 <= unitX && unitX <= 1;
        bool endXInUnit = 0 <= unitEndX && unitEndX <= 1;

        if (startXInUnit && endXInUnit) {
            float dt = a;
            GColor dColor = colorDifference * dt;
            GColor color = color0 + colorDifference * unitX;

            for (int i = 0; i < count; i++) {
                row[i] = colorToPixelFunction(color);
                color = color + dColor;
            }
        } else {
            for (int i = 0; i < count; i++) {
                float clampedX = clampFunction(unitX);

                GColor color = color0 + colorDifference * clampedX;
                row[i] = colorToPixelFunction(color);;

                unitX += a;
            }
        }
    }
};

#endif // SSLinearGradientShaderTwoColors_DEFINED