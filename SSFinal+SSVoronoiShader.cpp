#include "SSFinal.h"

#include "include/GShader.h"
#include "GPoint+SSHelpers.h"
#include "SSBlendModeHelpers.h"

class SSVoronoiShader : public GShader {
private:
    std::vector<GPoint> points;
    std::vector<GColor> colors;
    int colorCount;

    bool constIsOpaque;
    GMatrix inverseCTM;

public:
    SSVoronoiShader(
        const GPoint points[],
        const GColor colors[],
        int colorCount
    )
        : colorCount(colorCount)
    {
        // Copy points and colors into vectors, calculate isOpaque
        bool isOpaque = true;

        for (int i = 0; i < colorCount; i++) {
            this->points.push_back(points[i]);
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

            // Find closest point and color
            float minDistance = FLT_MAX;
            GColor closestColor;

            for (int colorIndex = 0; colorIndex < colorCount; colorIndex++) {
                GPoint point = points[colorIndex];
                float distance = GPoint_distance(originalDevicePoint, point);

                if (distance < minDistance) {
                    minDistance = distance;
                    closestColor = colors[colorIndex];
                }
            }

            assert(minDistance < FLT_MAX);

            // Set color
            row[i] = colorToPixel(closestColor);
        }
    }
};

/// The vornoi shader is defined by an array of points, each with an associated color.
/// The color any any (x,y) is the color of the closest point from the array.
std::shared_ptr<GShader> SSFinal::createVoronoiShader(
    const GPoint points[],
    const GColor colors[],
    int count
) {
    return std::unique_ptr<SSVoronoiShader>(new SSVoronoiShader(points, colors, count));
}