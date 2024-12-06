#ifndef SSTriangleTextureShader_DEFINED
#define SSTriangleTextureShader_DEFINED

#include "include/GMatrix.h"
#include "include/GShader.h"
#include "SSBlendModeHelpers.h"

class SSTriangleTextureShader : public GShader {
public:
    SSTriangleTextureShader(
        std::shared_ptr<GShader> baseShader,
        const GPoint point0,
        const GPoint point1,
        const GPoint point2,
        const GPoint texturePoint0,
        const GPoint texturePoint1,
        const GPoint texturePoint2
    ) {
        this->baseShader = baseShader;

        // Build unit space –> device coordinates matrix
        this->unitToDeviceMatrix = GMatrix(point1 - point0, point2 - point0, point0);

        // Build unit space -> texture coordinates matrix
        this->unitToTextureMatrix = GMatrix(texturePoint1 - texturePoint0, texturePoint2 - texturePoint0, texturePoint0);
    }

    void updateShader(
        const GPoint point0,
        const GPoint point1,
        const GPoint point2,
        const GPoint texturePoint0,
        const GPoint texturePoint1,
        const GPoint texturePoint2
    ) {
        // Build unit space –> device coordinates matrix
        this->unitToDeviceMatrix = GMatrix(point1 - point0, point2 - point0, point0);

        // Build unit space -> texture coordinates matrix
        this->unitToTextureMatrix = GMatrix(texturePoint1 - texturePoint0, texturePoint2 - texturePoint0, texturePoint0);
    }

    /// Return true iff all of the GPixels that may be returned by this shader will be opaque.
    bool isOpaque() override {
        return baseShader->isOpaque();
    }

    /// The draw calls in GCanvas must call this with the CTM before any calls to shadeSpan().
    bool setContext(const GMatrix& ctm) override {
        auto textureToUnitMatrix = unitToTextureMatrix.invert();

        if (textureToUnitMatrix) {
            GMatrix newCTM = ctm * unitToDeviceMatrix * textureToUnitMatrix.value();
            return baseShader->setContext(newCTM);
        } else {
            return false;
        }
    }

    /// Given a row of pixels in device space [x, y] ... [x + count - 1, y], return the
    /// corresponding src pixels in row[0...count - 1]. The caller must ensure that row[]
    /// can hold at least [count] entries.
    void shadeRow(int x, int y, int count, GPixel row[]) override {
        baseShader->shadeRow(x, y, count, row);
    }

private:
    std::shared_ptr<GShader> baseShader;
    GMatrix unitToDeviceMatrix;
    GMatrix unitToTextureMatrix;
};

std::shared_ptr<SSTriangleTextureShader> SSCreateTriangleTextureShader(
    std::shared_ptr<GShader> baseShader,
    const GPoint point0,
    const GPoint point1,
    const GPoint point2,
    const GPoint texturePoint0,
    const GPoint texturePoint1,
    const GPoint texturePoint2
);

#endif // SSTriangleTextureShader_DEFINED