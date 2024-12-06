#include "SSFinal.h"
#include "SSBlendModeHelpers.h"
#include "include/GShader.h"

class SSSweepGradientShader : public GShader {
private:
        GPoint center;
        float startRadians;
        std::vector<GColor> colors;
        int colorCount;
        bool constIsOpaque;
        GMatrix inverseCTM;

public:
    SSSweepGradientShader(
        GPoint center,
        float startRadians,
        const GColor colors[],
        int colorCount
    ) 
        : center(center)
        , startRadians(startRadians)
        , colorCount(colorCount)
    {
        // Copy colors into vectors, calculate isOpaque
        bool isOpaque = true;

        for (int i = 0; i < colorCount; i++) {
            this->colors.push_back(colors[i]);

            if (colors[i].a != 1) isOpaque = false;
        }

        this->constIsOpaque = isOpaque;
    }

    /// Return true iff all of the GPixels that may be returned by this shader will be opaque.
    bool isOpaque() override {
        return constIsOpaque;
    }

    /// The draw calls in GCanvas must call this with the CTM before any calls to shadeSpan().
    bool setContext(const GMatrix& ctm) override {
        auto inverseCTM = ctm.invert();

        if (inverseCTM) {
            this->inverseCTM = inverseCTM.value();
            return true;
        } else {
            return false;
        }
    }

    /// Given a row of pixels in device space [x, y] ... [x + count - 1, y], return the
    /// corresponding src pixels in row[0...count - 1]. The caller must ensure that row[]
    /// can hold at least [count] entries.
    void shadeRow(int x, int y, int rowCount, GPixel row[]) override {
        for (int i = 0; i < rowCount; i++) {
            GPoint ctmPoint = { x + i + 0.5f, y + 0.5f };
            GPoint originalDevicePoint = inverseCTM * ctmPoint;

            // Find angle between point and center
            float angleRadians = atan2(originalDevicePoint.y - center.y, originalDevicePoint.x - center.x);
            angleRadians -= startRadians;
            if (angleRadians < 0) angleRadians += 2 * gFloatPI;

            // Scale angle to unit space
            float unitAngle = angleRadians / (2 * gFloatPI);
            unitAngle = GPinToUnit(unitAngle);

            // Scale angle to (0...colorCount)
            float scaledAngle = unitAngle * colorCount;

            // Find previous and next color indices
            int previousIndex = GFloorToInt(scaledAngle);
            int nextIndex = GCeilToInt(scaledAngle);
            if (nextIndex >= colorCount) nextIndex = 0;

            // Lerp colors
            float distanceFromPrevious = scaledAngle - previousIndex;
            GColor color = (1 - distanceFromPrevious) * colors[previousIndex] + distanceFromPrevious * colors[nextIndex];
            row[i] = colorToPixel(color);
        }
    }
};

///  Return a sweep-gradient shader, centered at 'center', starting wiht color[0] at  startRadians
///  and ending with colors[count-1] at startRadians+2pi. The colors are distributed evenly around the sweep.
std::shared_ptr<GShader> SSFinal::createSweepGradient(
    GPoint center,
    float startRadians,
    const GColor colors[],
    int count
) {
    return std::unique_ptr<SSSweepGradientShader>(new SSSweepGradientShader(center, startRadians, colors, count));
}