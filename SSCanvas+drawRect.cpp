#include "SSCanvas.h"
#include "SSBlendModeHelpers.h"
#include "GRect+SSHelpers.h"

template <typename BlendFunction, typename BlitRowFunction>
void blitRectCommon(
    const GIRect& rect,
    const GBitmap& bitmap,
    BlendFunction blend,
    BlitRowFunction blitRow
) {
    for (int y = rect.top; y < rect.bottom; y++) {
        blitRow(rect.left, rect.right, y, blend);
    }
}

/// Fill the rectangle with the color, using the specified blendmode.
///
/// The affected pixels are those whose centers are "contained" inside the rectangle:
/// e.g. contained == center > min_edge && center <= max_edge
void SSCanvas::drawRect(const GRect& rect, const GPaint& paint) {
    // Call through to drawConvexPoly if rect needs to be transformed
    if (getCTM() != GMatrix()) {
        GPoint points[4] = {
            { rect.left, rect.top }, { rect.right, rect.top },
            { rect.right, rect.bottom }, { rect.left, rect.bottom }
        };
    
        drawConvexPolygon(points, 4, paint);
        return;
    }

    // Otherwise, draw rectangle normally
    const GIRect roundedRect = rect.round();

    // Exit early if rectangle completely outside bounds
    GIRect bitmapBounds = GIRect::LTRB(0, 0, bitmap.width(), bitmap.height());
    if (GIRect_isOutside(roundedRect, bitmapBounds)) return;

    // Clip rectangle to bitmap bounds
    int32_t x_min = std::max(roundedRect.left, 0);
    int32_t x_max = std::min(roundedRect.right, bitmap.width());
    int32_t y_min = std::max(roundedRect.top, 0);
    int32_t y_max = std::min(roundedRect.bottom, bitmap.height());
    GIRect clippedRect = GIRect::LTRB(x_min, y_min, x_max, y_max);

    // Exit early if rectangle is empty
    if ((clippedRect.left == clippedRect.right) || (clippedRect.top == clippedRect.bottom)) return;

    // Read color and shader from paint
    const GColor color = paint.getColor();
    const auto shader = paint.peekShader();

    // Simplify blend mode
    GBlendMode simplifiedBlendMode;
    if (shader) {
        simplifiedBlendMode = simplifyBlendMode(paint.getBlendMode(), shader->isOpaque(), false);
    } else {
        simplifiedBlendMode = simplifyBlendMode(paint.getBlendMode(), color.a == 1, color.a == 0);
    }

    // If blend mode is dest, no work to be done
    if (simplifiedBlendMode == GBlendMode::kDst) return;

    if (shader) {
        // Set CTM as context for shader. Return early if it failed
        bool setContextWasSuccessful = shader->setContext(getCTM());
        if (!setContextWasSuccessful) return;

        // Allocate space on stack to store shader results
        GPixel src[clippedRect.right - clippedRect.left];
    
        auto blitRowWithShader = [&src, this, &shader](int left, int right, int y, auto blend) {
            shader->shadeRow(left, y, right - left, src);
            GPixel *row = bitmap.getAddr(left, y);

            for (int i = 0; i < right - left; i++) {
                row[i] = blend(src[i], &row[i]);
            }
        };

        switch (simplifiedBlendMode) {
        case GBlendMode::kClear: {
            blitRectCommon(clippedRect, bitmap, blendClear, blitRowWithShader);
            break;
        }
        case GBlendMode::kSrc: {
            blitRectCommon(clippedRect, bitmap, blendSrc, blitRowWithShader);
            break;
        }
        case GBlendMode::kDst: {
            // No work to do, can return early
            return;
        }
        case GBlendMode::kSrcOver: {
            blitRectCommon(clippedRect, bitmap, blendSrcOver, blitRowWithShader);
            break;
        }
        case GBlendMode::kDstOver: {
            blitRectCommon(clippedRect, bitmap, blendDstOver, blitRowWithShader);
            break;
        }
        case GBlendMode::kSrcIn: {
            blitRectCommon(clippedRect, bitmap, blendSrcIn, blitRowWithShader);
            break;
        }
        case GBlendMode::kDstIn: {
            blitRectCommon(clippedRect, bitmap, blendDstIn, blitRowWithShader);
            break;
        }
        case GBlendMode::kSrcOut: {
            blitRectCommon(clippedRect, bitmap, blendSrcOut, blitRowWithShader);
            break;
        }
        case GBlendMode::kDstOut: {
            blitRectCommon(clippedRect, bitmap, blendDstOut, blitRowWithShader);
            break;
        }
        case GBlendMode::kSrcATop: {
            blitRectCommon(clippedRect, bitmap, blendSrcATop, blitRowWithShader);
            break;
        }
        case GBlendMode::kDstATop: {
            blitRectCommon(clippedRect, bitmap, blendDstATop, blitRowWithShader);
            break;
        }
        case GBlendMode::kXor: {
            blitRectCommon(clippedRect, bitmap, blendXor, blitRowWithShader);
            break;
        }
        }
    } else {
        // Premultiply source color
        GPixel src = colorToPixel(color);

        auto blitRowWithPixel = [&](int left, int right, int y, auto blend) {
            GPixel *row = bitmap.getAddr(left, y);

            for (int x = 0; x < right - left; x++) {
                row[x] = blend(src, &row[x]);
            }
        };

        switch (simplifiedBlendMode) {
        case GBlendMode::kClear: {
            blitRectCommon(clippedRect, bitmap, blendClear, blitRowWithPixel);
            break;
        }
        case GBlendMode::kSrc: {
            blitRectCommon(clippedRect, bitmap, blendSrc, blitRowWithPixel);
            break;
        }
        case GBlendMode::kDst: {
            // No work to do, can return early
            return;
        }
        case GBlendMode::kSrcOver: {
            blitRectCommon(clippedRect, bitmap, blendSrcOver, blitRowWithPixel);
            break;
        }
        case GBlendMode::kDstOver: {
            blitRectCommon(clippedRect, bitmap, blendDstOver, blitRowWithPixel);
            break;
        }
        case GBlendMode::kSrcIn: {
            blitRectCommon(clippedRect, bitmap, blendSrcIn, blitRowWithPixel);
            break;

        }
        case GBlendMode::kDstIn: {
            blitRectCommon(clippedRect, bitmap, blendDstIn, blitRowWithPixel);
            break;
        }
        case GBlendMode::kSrcOut: {
            blitRectCommon(clippedRect, bitmap, blendSrcOut, blitRowWithPixel);
            break;
        }
        case GBlendMode::kDstOut: {
            blitRectCommon(clippedRect, bitmap, blendDstOut, blitRowWithPixel);
            break;
        }
        case GBlendMode::kSrcATop: {
            blitRectCommon(clippedRect, bitmap, blendSrcATop, blitRowWithPixel);
            break;
        }
        case GBlendMode::kDstATop: {
            blitRectCommon(clippedRect, bitmap, blendDstATop, blitRowWithPixel);
            break;
        }
        case GBlendMode::kXor: {
            blitRectCommon(clippedRect, bitmap, blendXor, blitRowWithPixel);
            break;
        }
        }
    }
}