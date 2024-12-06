#ifndef SSBitmapShader_DEFINED
#define SSBitmapShader_DEFINED

#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GShader.h"
#include "SSMath.h"

class SSBitmapShader : public GShader {
public:
    /// Instantatiates an SSBitmapShader with a bitmap and local matrix.
    SSBitmapShader(const GBitmap bitmap, const GMatrix localMatrix, GTileMode tileMode) 
        : bitmap(bitmap)
        , localMatrix(localMatrix)
        , maxX(bitmap.width() - 1)
        , maxY(bitmap.height() - 1)
        , fWidth(static_cast<float>(bitmap.width()))
        , fInverseWidth(1.0f / static_cast<float>(bitmap.width()))
        , fHeight(static_cast<float>(bitmap.height()))
        , fInverseHeight(1.0f / static_cast<float>(bitmap.height()))
        , tileMode(tileMode)
    {
        auto inverse = localMatrix.invert();
        if (inverse) this->inverseMatrix = inverse.value();
    }

    /// Return true iff all of the GPixels that may be returned by this shader will be opaque.
    bool isOpaque() override {
        return bitmap.isOpaque();
    }

    /// The draw calls in GCanvas must call this with the CTM before any calls to shadeSpan().
    bool setContext(const GMatrix& ctm) override {
        auto inverse = (ctm * localMatrix).invert();

        if (inverse) {
            this->inverseMatrix = inverse.value();
            return true;
        } else {
            return false;
        }
    }

    /// Given a row of pixels in device space [x, y] ... [x + count - 1, y], return the
    /// corresponding src pixels in row[0...count - 1]. The caller must ensure that row[]
    /// can hold at least [count] entries.
    void shadeRow(int x, int y, int count, GPixel row[]) override {
        auto xyClamp = [](float x, int min, int max) { return SSClamp(x, min, max); };
        auto xRepeat = [&](float x, int min, int max) { return repeat(x, fWidth, fInverseWidth); };
        auto yRepeat = [&](float x, int min, int max) { return repeat(x, fHeight, fInverseHeight); };
        auto xMirror = [&](float x, int min, int max) { return mirror(x, fWidth, fInverseWidth); };
        auto yMirror = [&](float x, int min, int max) { return mirror(x, fHeight, fInverseHeight); };

        switch (tileMode) {
        case GTileMode::kClamp: {
            shadeRowCommon(x, y, count, row, xyClamp, xyClamp);
            break;
        }
        case GTileMode::kRepeat: {
            shadeRowCommon(x, y, count, row, xRepeat, yRepeat);
            break;
        }
        case GTileMode::kMirror: {
            shadeRowCommon(x, y, count, row, xMirror, yMirror);
            break;
        }
        }
    }

private:
    const GBitmap bitmap;
    const GMatrix localMatrix;
    const int maxX;
    const int maxY;
    const float fWidth;
    const float fInverseWidth;
    const float fHeight;
    const float fInverseHeight;
    const GTileMode tileMode;

    GMatrix inverseMatrix;

    static float repeat(const float& x, const float& scaleUpAmount, const float& scaleDownAmount) {
        if (x >= 0.0f && x <= scaleUpAmount) {
            return x;
        } else {
            float unitX = x * scaleDownAmount;
            float repeatedX = unitX - GFloorToInt(unitX);
            return repeatedX * scaleUpAmount;
        }
    }

    static float mirror(const float& x, const float& scaleUpAmount, const float& scaleDownAmount) {
        if (x >= 0.0f && x <= scaleUpAmount) {
            return x;
        } else {
            float unitX = x * scaleDownAmount;
            float periodX = unitX * 0.5f;
            float mirroredX = 2.0f * fabs(periodX - GFloorToInt(periodX + 0.5f));
            return mirroredX * scaleUpAmount;
        }
    }

    template <typename ClampXFunction, typename ClampYFunction>
    void shadeRowCommon(
        int x,
        int y,
        int count,
        GPixel row[],
        ClampXFunction clampXFunction,
        ClampYFunction clampYFunction
    ) {
        const float fx = static_cast<float>(x);
        const float fy = static_cast<float>(y);

        float px = (inverseMatrix[0] * (fx + 0.5f)) + (inverseMatrix[2] * (fy + 0.5f)) + inverseMatrix[4];
        float py = (inverseMatrix[1] * (fx + 0.5f)) + (inverseMatrix[3] * (fy + 0.5f)) + inverseMatrix[5];

        const bool aIsZero = nearlyZero(inverseMatrix[0]);
        const bool bIsZero = nearlyZero(inverseMatrix[1]);

        if (aIsZero && bIsZero) {
            float clamped_x = clampXFunction(px, 0, maxX);
            float clamped_y = clampYFunction(py, 0, maxY);
            int floored_x = SSFloorPositiveToInt(clamped_x);
            int floored_y = SSFloorPositiveToInt(clamped_y);

            for (int i = 0; i < count; i++) {
                row[i] = *bitmap.getAddr(floored_x, floored_y);
            }
        } else if (aIsZero) {
            float clamped_x = clampXFunction(px, 0, maxX);
            int floored_x = SSFloorPositiveToInt(clamped_x);

            for (int i = 0; i < count; i++) {
                float clamped_y = clampYFunction(py, 0, maxY);
                int floored_y = SSFloorPositiveToInt(clamped_y);

                row[i] = *bitmap.getAddr(floored_x, floored_y);

                py += inverseMatrix[1];
            }
        } else if (bIsZero) {
            float clamped_y = clampYFunction(py, 0, maxY);
            int floored_y = SSFloorPositiveToInt(clamped_y);

            for (int i = 0; i < count; i++) {
                float clamped_x = clampXFunction(px, 0, maxX);
                int floored_x = SSFloorPositiveToInt(clamped_x);

                row[i] = *bitmap.getAddr(floored_x, floored_y);

                px += inverseMatrix[0];
            }
        } else {
            for (int i = 0; i < count; i++) {
                float clamped_x = clampXFunction(px, 0, maxX);
                float clamped_y = clampYFunction(py, 0, maxY);
                int floored_x = SSFloorPositiveToInt(clamped_x);
                int floored_y = SSFloorPositiveToInt(clamped_y);

                row[i] = *bitmap.getAddr(floored_x, floored_y);

                px += inverseMatrix[0];
                py += inverseMatrix[1];
            }
        }
    }
};

#endif // SSBitmapShader_DEFINED