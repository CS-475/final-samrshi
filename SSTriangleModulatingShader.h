#ifndef SSTriangleModulatingShader_DEFINED
#define SSTriangleModulatingShader_DEFINED

#include "include/GMatrix.h"
#include "include/GShader.h"
#include "SSTriangleColorShader.h"
#include "SSTriangleTextureShader.h"
#include "SSBlendModeHelpers.h"

class SSTriangleModulatingShader : public GShader {
public:
    std::shared_ptr<SSTriangleColorShader> colorShader;
    std::shared_ptr<SSTriangleTextureShader> textureShader;

    SSTriangleModulatingShader(
        std::shared_ptr<SSTriangleColorShader> colorShader,
        std::shared_ptr<SSTriangleTextureShader> textureShader
    )
        : colorShader(colorShader)
        , textureShader(textureShader)
    {}

    void updateShader(
        const GPoint p0,
        const GPoint p1,
        const GPoint p2,
        const GColor color0,
        const GColor color1,
        const GColor color2,
        const GPoint texturePoint0,
        const GPoint texturePoint1,
        const GPoint texturePoint2
    ) {
        colorShader->updateShader(p0, p1, p2, color0, color1, color2);
        textureShader->updateShader(p0, p1, p2, texturePoint0, texturePoint1, texturePoint2);
    }

    /// Return true iff all of the GPixels that may be returned by this shader will be opaque.
    bool isOpaque() override {
        return colorShader->isOpaque() && textureShader->isOpaque();
    }

    /// The draw calls in GCanvas must call this with the CTM before any calls to shadeSpan().
    bool setContext(const GMatrix& ctm) override {
        return colorShader->setContext(ctm) && textureShader->setContext(ctm);
    }

    /// Given a row of pixels in device space [x, y] ... [x + count - 1, y], return the
    /// corresponding src pixels in row[0...count - 1]. The caller must ensure that row[]
    /// can hold at least [count] entries.
    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GPixel row1[count];
        GPixel row2[count];

        colorShader->shadeRow(x, y, count, row1);
        textureShader->shadeRow(x, y, count, row2);

        for (int i = 0; i < count; i++) {
            row[i] = blendModulate(row1[i], &row2[i]);
        }
    }
};

std::shared_ptr<SSTriangleModulatingShader> SSCreateModulatingShader(
    std::shared_ptr<SSTriangleColorShader> colorShader,
    std::shared_ptr<SSTriangleTextureShader> textureShader
);

#endif // SSTriangleModulatingShader_DEFINED