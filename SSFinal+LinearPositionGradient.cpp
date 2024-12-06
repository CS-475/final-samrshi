#include "SSFinal.h"

#include "SSMath.h"
#include "SSBlendModeHelpers.h"
#include "include/GMatrix.h"

class SSLinearPositionGradient : public GShader {
public:
    SSLinearPositionGradient(
        GPoint p0,
        GPoint p1,
        const GColor colors[],
        const float positions[],
        int count
    ) 
        : p0(p0)
        , p1(p1)
        , colorCount(count)
    {
        assert(positions[0] == 0.0f);
        assert(positions[count - 1] == 1.0f);

        // Copy colors into local std::vector
        for (int i = 0; i < count; i++) {
            this->colors.push_back(colors[i]);
        }

        // Copy positions into local std::vector
        for (int i = 0; i < count; i++) {
            this->positions.push_back(positions[i]);
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

    /// Shade a row in this linear position gradient shader
    void shadeRow(int x, int y, int rowWidth, GPixel row[]) override {
        for (int i = 0; i < rowWidth; i++) {
            // Convert device coordinate to unit space
            GPoint devicePoint = { x + i + 0.5, y + 0.5 };
            GPoint unitPoint = inverseMatrix * devicePoint;

            // Clamp unit x to 0...1
            float unitX = SSClamp(unitPoint.x, 0, 1);

            // Find surrounding colors
            float leftPosition, rightPosition;
            GColor leftColor, rightColor;

            for (int colorIndex = 0; colorIndex < colorCount; colorIndex++) {
                if (positions[colorIndex] < unitX) {
                    leftPosition = positions[colorIndex];
                    leftColor = colors[colorIndex];
                }

                if (positions[colorIndex] > unitX) {
                    rightPosition = positions[colorIndex];
                    rightColor = colors[colorIndex];
                    break;
                }
            }

            // Calculate unit distances
            float leftDistance = unitX - leftPosition;
            float rightDistance = rightPosition - unitX;
            float totalDistance = rightPosition - leftPosition;

            // Calculate left and right contributions
            float leftContribution = rightDistance / totalDistance;
            float rightContribution = leftDistance / totalDistance;

            // Lerp left and right colors to find color
            GColor color = (leftContribution * leftColor) + (rightContribution * rightColor);
            row[i] = colorToPixel(color);
        }
    }

private:
    const GPoint p0;
    const GPoint p1;
    const int colorCount;

    std::vector<GColor> colors;
    std::vector<float> positions;
    bool constIsOpaque;

    GMatrix unitToDeviceMatrix;
    GMatrix inverseMatrix;
};

/// Returns a new type of linear gradient. In this variant, the "count" colors are
/// positioned along the line p0...p1 not "evenly", but according to the pos[] array.
///
/// pos[] holds "count" values, each 0...1, which specify the percentage along the
/// line where the each color lies.
///
/// e.g. pos[] = {0, 0.25, 1} would mean 3 colors positioned as follows:
///
///     color[0] ..... color[1] ..... ..... ..... color[2]
///
/// color[i] is positioned by computing (1 - pos[i])*p0 + pos[i]*p1
///
/// For this API, pos[] will always be monotonic, with p[0] == 0 and p[count-1] == 1.0
///
/// For simplicity, assume that we're using "clamp" tiling.
std::shared_ptr<GShader> SSFinal::createLinearPosGradient(
    GPoint p0, GPoint p1,
    const GColor colors[],
    const float pos[],
    int count
) {
    return std::unique_ptr<SSLinearPositionGradient>(new SSLinearPositionGradient(p0, p1, colors, pos, count));
}