#ifndef SSLinearGradientShaderManyColors_DEFINED
#define SSLinearGradientShaderManyColors_DEFINED

#include "include/GMatrix.h"
#include "include/GShader.h"
#include "SSBlendModeHelpers.h"

class SSLinearGradientShaderManyColors : public GShader {
public:
    /// Instantatiates an SSBitmapShader with a bitmap and local matrix.
    SSLinearGradientShaderManyColors(
        GPoint p0,
        GPoint p1,
        const GColor colors[],
        int count,
        GTileMode tileMode
    ) 
        : p0(p0)
        , p1(p1)
        , colorCount(count)
        , startPixel(colorToPixel(colors[0]))
        , endPixel(colorToPixel(colors[count - 1]))
        , tileMode(tileMode)
    {
        // Copy colors into local std::vector and calculate color differences
        this->colors.reserve(count);
        this->colors.reserve(count - 1);

        for (int i = 0; i < count; i++) {
            this->colors.push_back(colors[i]);

            if (i > 0) {
                this->colorDifferences.push_back(colors[i] - colors[i - 1]);
            }
        }

        // Build unit space –> device coordinates matrix
        float dx = p1.x - p0.x;
        float dy = p1.y - p0.y;

        this->unitToDeviceMatrix = GMatrix(
            dx, -dy, p0.x,
            dy, dx, p0.y
        );

        // Check if all colors are opaque
        bool constIsOpaque = true;

        for (const GColor& color : this->colors) {
            if (color.a != 1) constIsOpaque = false;
        }

        this->constIsOpaque = constIsOpaque;
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
        const float colorGapCount = static_cast<float>(this->colorCount - 1);
        auto clampWithCapture = [colorGapCount](float x) { return SSClamp(x, 0, colorGapCount); };
        auto repeatWithCapture = [colorGapCount](float x) { return repeat(x, colorGapCount); };
        auto mirrorWithCapture = [colorGapCount](float x) { return mirror(x, colorGapCount); };

        if (constIsOpaque) {
            switch (tileMode) {
            case GTileMode::kClamp: {
                shadeRowCommon(x, y, count, row, colorToPixelOpaque, clampWithCapture);
                break;
            }
            case GTileMode::kRepeat: {
                shadeRowCommon(x, y, count, row, colorToPixelOpaque, repeatWithCapture);
                break;
            }
            case GTileMode::kMirror: {
                shadeRowCommon(x, y, count, row, colorToPixelOpaque, mirrorWithCapture);
                break;
            }
            }
        } else {
            switch (tileMode) {
            case GTileMode::kClamp: {
                shadeRowCommon(x, y, count, row, colorToPixel, clampWithCapture);
                break;
            }
            case GTileMode::kRepeat: {
                shadeRowCommon(x, y, count, row, colorToPixel, repeatWithCapture);
                break;
            }
            case GTileMode::kMirror: {
                shadeRowCommon(x, y, count, row, colorToPixel, mirrorWithCapture);
                break;
            }
            }
        }
    }

private:
    const GPoint p0;
    const GPoint p1;
    const int colorCount;

    const GPixel startPixel;
    const GPixel endPixel;

    std::vector<GColor> colors;
    std::vector<GColor> colorDifferences;
    bool constIsOpaque;

    GMatrix unitToDeviceMatrix;
    GMatrix inverseMatrix;

    GTileMode tileMode;

    static float repeat(float x, float scaleAmount) {
        float unitX = x * (1.0f / scaleAmount);
        float repeatedX = unitX - GFloorToInt(unitX);
        return repeatedX * scaleAmount;
    }

    static float mirror(float x, float scaleAmount) {
        float unitX = x * (1.0f / scaleAmount);

        float period = 2.0f;
        float mirroredX = 2 * abs((unitX / period) - GFloorToInt((unitX / period) + 0.5f));

        return mirroredX * scaleAmount;
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

        const float unitX = (inverseMatrix[0] * (fx + 0.5f)) + (inverseMatrix[2] * (fy + 0.5f)) + inverseMatrix[4];

        const float colorGapCount = static_cast<float>(this->colorCount - 1);
        float scaledX = unitX * colorGapCount;
        const float scaledStep = inverseMatrix[0] * colorGapCount;

        for (int i = 0; i < count; i++) {
            // Clamp x to 0...colorGapCount
            float clampedX = clampFunction(scaledX);

            // Figure out which colors if before this point
            int previousColorIndex = GFloorToInt(clampedX);

            // Find how far from the previous color this point is
            float unitDistanceFromPreviousColor = clampedX - previousColorIndex;

            // Figure out color at point
            // - color = (1 - t) * previousColor + t * nextColor
            // - color = previousColor + (nextColor - previousColor) * t
            const GColor& previousColor = colors[previousColorIndex];
            const GColor& colorDiff = colorDifferences[previousColorIndex];
            GColor color = previousColor + colorDiff * unitDistanceFromPreviousColor;
            row[i] = colorToPixelFunction(color);

            // Move to next unitX
            scaledX += scaledStep;
        }
    }
};

#endif // SSLinearGradientShaderManyColors_DEFINED