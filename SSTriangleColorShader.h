#ifndef SSTriangleColorShader_DEFINED
#define SSTriangleColorShader_DEFINED

#include "include/GMatrix.h"
#include "include/GShader.h"
#include "SSBlendModeHelpers.h"

class SSTriangleColorShader : public GShader {
public:
    SSTriangleColorShader(
        const GPoint p0,
        const GPoint p1,
        const GPoint p2,
        const GColor color0,
        const GColor color1,
        const GColor color2
    ) 
        : p0(p0)
        , p1(p1)
        , p2(p2)
        , color0(color0)
        , color1(color1)
        , color2(color2)
    {
        // Build unit space –> device coordinates matrix
        this->unitToDeviceMatrix = GMatrix(p1 - p0, p2 - p0, p0);
    }

    void updateShader(
        const GPoint p0,
        const GPoint p1,
        const GPoint p2,
        const GColor color0,
        const GColor color1,
        const GColor color2
    ) {
        this->p0 = p0;
        this->p1 = p1;
        this->p2 = p2;
        this->color0 = color0;
        this->color1 = color1;
        this->color2 = color2;
        this->unitToDeviceMatrix = GMatrix(p1 - p0, p2 - p0, p0);
    }

    /// Return true iff all of the GPixels that may be returned by this shader will be opaque.
    bool isOpaque() override {
        return color0.a == 1 && color1.a == 1 && color2.a == 1;
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
        if (isOpaque()) {
            shadeRowCommon(x, y, count, row, colorToPixelOpaque);
        } else {
            shadeRowCommon(x, y, count, row, colorToPixel);
        }
    }

private:
    GPoint p0;
    GPoint p1;
    GPoint p2;
    GColor color0;
    GColor color1;
    GColor color2;

    GMatrix unitToDeviceMatrix;
    GMatrix inverseMatrix;

    template <typename ColorToPixelFunction>
    void shadeRowCommon(
        int x,
        int y,
        int count,
        GPixel row[],
        ColorToPixelFunction colorToPixelFunction
    ) {
        float unitX = (inverseMatrix[0] * (x + 0.5f)) + (inverseMatrix[2] * (y + 0.5f)) + inverseMatrix[4];
        float unitY = (inverseMatrix[1] * (x + 0.5f)) + (inverseMatrix[3] * (y + 0.5f)) + inverseMatrix[5];

        float weight0 = GPinToUnit(1 - unitX - unitY);
        float weight1 = GPinToUnit(unitX);
        float weight2 = GPinToUnit(unitY);

        GColor color = (weight0 * color0) + (weight1 * color1) + (weight2 * color2);
        
        GColor color0Step = color0 * (-inverseMatrix[0] - inverseMatrix[1]);
        GColor color1Step = color1 * inverseMatrix[0];
        GColor color2Step = color2 * inverseMatrix[1];

        GColor colorStep = color0Step + color1Step + color2Step;

        for (int i = 0; i < count; i++) {
            row[i] = colorToPixelFunction(color);
            color += colorStep;
        }
    }
};

std::shared_ptr<SSTriangleColorShader> SSCreateTriangleColorShader(
    const GPoint p0,
    const GPoint p1,
    const GPoint p2,
    const GColor color0,
    const GColor color1,
    const GColor color2
);

#endif // SSTriangleColorShader_DEFINED