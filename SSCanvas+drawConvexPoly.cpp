#include "SSCanvas.h"
#include "SSBlendModeHelpers.h"
#include "SSEdge.h"

void findIntersections(int& left, int& right, int y, SSEdge edge0, SSEdge edge1) {
    // Find edges intersections with ray
    int leftIntersection = GRoundToInt(SSEdge_yIntersection(edge0, (float) y + 0.5f));
    int rightIntersection = GRoundToInt(SSEdge_yIntersection(edge1, (float) y + 0.5f));

    // Round to pixel values and assign to pointers
    left = std::min(leftIntersection, rightIntersection);
    right = std::max(leftIntersection, rightIntersection);
}

void advanceEdgeIfExpiring(SSEdge& edge, int& next, const int& y, const std::vector<SSEdge>& edges) {
     if (y + 1 >= edge.bottom) {
        edge = edges[next];
        next += 1;
    }
}

template <typename BlendFunction, typename BlitRowFunction>
void SSCanvas::blitConvexPolyCommon(
    const GPoint points[],
    int count,
    BlendFunction blend,
    BlitRowFunction blitRow
) {
    // Run points through ctm
    GPoint mappedPoints[count];
    GMatrix ctm = getCTM();
    ctm.mapPoints(mappedPoints, points, count);

    // Get edge list from points
    GRect bitmap_bounds = GRect::LTRB(0, 0, bitmap.width() - 1, bitmap.height() - 1);
    std::vector<SSEdge> edges = makeEdges(mappedPoints, count, bitmap_bounds);

    // Find overall top and bottom y
    int min_y = INT32_MAX;
    int max_y = -INT32_MAX;
    int min_x = INT32_MAX;
    int max_x = -INT32_MAX;

    for (const SSEdge& edge : edges) {
        // Reassign min_y, max_y, min_x, max_x if necessary
        if (edge.top < min_y) min_y = edge.top;
        if (edge.bottom > max_y) max_y = edge.bottom;

        if (edge.left < min_x) min_x = edge.left;
        if (edge.right > max_x) max_x = edge.right;
    }

    // Return if there aren't at least two edges
    if (edges.size() < 2) return;

    SSEdge edge0 = edges[0];
    SSEdge edge1 = edges[1];
    int nextEdgeIndex = 2;

    for (int y = min_y; y < max_y; y++) {
        int left, right;
        findIntersections(left, right, y, edge0, edge1);

        blitRow(left, right, y, blend);
        
        advanceEdgeIfExpiring(edge0, nextEdgeIndex, y, edges);
        advanceEdgeIfExpiring(edge1, nextEdgeIndex, y, edges);
    }
}

/// Fill the convex polygon with the color and blendmode,
/// following the same "containment" rule as rectangles.
void SSCanvas::drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) {
    const GColor color = paint.getColor();
    GShader *shader = paint.peekShader();

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
        GPixel src[bitmap.width()];

        auto blitRowWithShader = [&src, this, &shader](int left, int right, int y, auto blend) {
            shader->shadeRow(left, y, right - left, src);
            GPixel *row = bitmap.getAddr(left, y);

            for (int i = 0; i < right - left; i++) {
                row[i] = blend(src[i], &row[i]);
            }
        };

        switch (simplifiedBlendMode) {
        case GBlendMode::kClear: {
            blitConvexPolyCommon(points, count, blendClear, blitRowWithShader);
            break;
        }
        case GBlendMode::kSrc: {
            blitConvexPolyCommon(points, count, blendSrc, blitRowWithShader);
            break;
        }
        case GBlendMode::kDst: {
            // No work to do, can return early
            return;
        }
        case GBlendMode::kSrcOver: {
            blitConvexPolyCommon(points, count, blendSrcOver, blitRowWithShader);
            break;
        }
        case GBlendMode::kDstOver: {
            blitConvexPolyCommon(points, count, blendDstOver, blitRowWithShader);
            break;
        }
        case GBlendMode::kSrcIn: {
            blitConvexPolyCommon(points, count, blendSrcIn, blitRowWithShader);
            break;
        }
        case GBlendMode::kDstIn: {
            blitConvexPolyCommon(points, count, blendDstIn, blitRowWithShader);
            break;
        }
        case GBlendMode::kSrcOut: {
            blitConvexPolyCommon(points, count, blendSrcOut, blitRowWithShader);
            break;
        }
        case GBlendMode::kDstOut: {
            blitConvexPolyCommon(points, count, blendDstOut, blitRowWithShader);
            break;
        }
        case GBlendMode::kSrcATop: {
            blitConvexPolyCommon(points, count, blendSrcATop, blitRowWithShader);
            break;
        }
        case GBlendMode::kDstATop: {
            blitConvexPolyCommon(points, count, blendDstATop, blitRowWithShader);
            break;
        }
        case GBlendMode::kXor: {
            blitConvexPolyCommon(points, count, blendXor, blitRowWithShader);
            break;
        }
        }
    } else {
        // Premultiply paint color
        const GPixel src = colorToPixel(paint.getColor());

        auto blitRowWithPixel = [&](int left, int right, int y, auto blend) {
            GPixel *row = bitmap.getAddr(left, y);

            for (int x = 0; x < right - left; x++) {
                row[x] = blend(src, &row[x]);
            }
        };

        switch (simplifiedBlendMode) {
        case GBlendMode::kClear: {
            blitConvexPolyCommon(points, count, blendClear, blitRowWithPixel);
            break;
        }
        case GBlendMode::kSrc: {
            blitConvexPolyCommon(points, count, blendSrc, blitRowWithPixel);
            break;
        }
        case GBlendMode::kDst: {
            // No work to do, can return early
            return;
        }
        case GBlendMode::kSrcOver: {
            blitConvexPolyCommon(points, count, blendSrcOver, blitRowWithPixel);
            break;
        }
        case GBlendMode::kDstOver: {
            blitConvexPolyCommon(points, count, blendDstOver, blitRowWithPixel);
            break;
        }
        case GBlendMode::kSrcIn: {
            blitConvexPolyCommon(points, count, blendSrcIn, blitRowWithPixel);
            break;

        }
        case GBlendMode::kDstIn: {
            blitConvexPolyCommon(points, count, blendDstIn, blitRowWithPixel);
            break;
        }
        case GBlendMode::kSrcOut: {
            blitConvexPolyCommon(points, count, blendSrcOut, blitRowWithPixel);
            break;
        }
        case GBlendMode::kDstOut: {
            blitConvexPolyCommon(points, count, blendDstOut, blitRowWithPixel);
            break;
        }
        case GBlendMode::kSrcATop: {
            blitConvexPolyCommon(points, count, blendSrcATop, blitRowWithPixel);
            break;
        }
        case GBlendMode::kDstATop: {
            blitConvexPolyCommon(points, count, blendDstATop, blitRowWithPixel);
            break;
        }
        case GBlendMode::kXor: {
            blitConvexPolyCommon(points, count, blendXor, blitRowWithPixel);
            break;
        }
        }
    }
}