#include "SSFinal.h"

#include "include/GShader.h"
#include "SSBlendModeHelpers.h"
#include "GColorShader+SSHelpers.h"

class SSColorMatrixShader : public GShader {
private:
    const GColorMatrix& colorMatrix;
    GShader *shader;

public:
    SSColorMatrixShader(
        const GColorMatrix& colorMatrix,
        GShader *shader
    ) 
        : colorMatrix(colorMatrix)
        , shader(shader)
    {}

    /// Return true iff all of the GPixels that may be returned by this shader will be opaque.
    bool isOpaque() override {
        /// TODO: Transform by matrix
        return false;
    }

    /// The draw calls in GCanvas must call this with the CTM before any calls to shadeSpan().
    bool setContext(const GMatrix& ctm) override {
        return shader->setContext(ctm);
    }

    /// Given a row of pixels in device space [x, y] ... [x + count - 1, y], return the
    /// corresponding src pixels in row[0...count - 1]. The caller must ensure that row[]
    /// can hold at least [count] entries.
    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GPixel baseRow[count];
        shader->shadeRow(x, y, count, baseRow);

        for (int i = 0; i < count; i++) {
            // Read base color from shaded row
            GPixel basePixel = baseRow[i];
            GColor baseColor = pixelToColor(basePixel);

            // Transform color by color matrix
            GColor newColor = colorMatrix * baseColor;
            row[i] = colorToPixel(newColor);
        }
    }
};

/// Returns an instance to a shader that will proxy to a "realShader", and transform
/// its output using the GColorMatrix provided.
///
/// Note: the GColorMatrix is defined to operate on unpremul GColors
///
/// Note: the resulting colors (after applying the colormatrix) may be out of bounds
///       for color componets. If this happens they should be clamped to legal values.
std::shared_ptr<GShader> SSFinal::createColorMatrixShader(
    const GColorMatrix& colorMatrix,
    GShader* realShader
) {
    return std::unique_ptr<SSColorMatrixShader>(new SSColorMatrixShader(colorMatrix, realShader));
}