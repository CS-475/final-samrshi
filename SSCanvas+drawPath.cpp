#include "SSCanvas.h"
#include "SSBlendModeHelpers.h"
#include "SSEdge.h"
#include "GRect+SSHelpers.h"

#include <climits>
#include <list>

template <typename MakeEdgeFunction>
void addEdgesFromQuad(
    const GPoint& a,
    const GPoint& b,
    const GPoint& c,
    const MakeEdgeFunction makeEdgeFunction
) {
    auto curve = [a, b, c](float t){
        return (1 - t)*(1 - t)*a + 2*(1 - t)*t*b + t*t*c;
    };

    float tolerance = 1.0f / 4.0f;
    GPoint errorVector = (a - 2*b - c) * 0.25f;
    float errorLength = sqrt(errorVector.x * errorVector.x + errorVector.y * errorVector.y);
    int numSegments = GCeilToInt(sqrt(errorLength / tolerance));
    float dt = 1.0f / static_cast<float>(numSegments);

    GPoint p0 = a;

    for (int i = 1; i < numSegments - 1; i++) {
        GPoint p1 = curve(i * dt);
        makeEdgeFunction(p0, p1);
        p0 = p1;
    }

    makeEdgeFunction(p0, c);
}

template <typename MakeEdgeFunction>
void addEdgesFromCubic(
    const GPoint& a,
    const GPoint& b,
    const GPoint& c,
    const GPoint& d,
    const MakeEdgeFunction makeEdgeFunction
) {
    auto curve = [a, b, c, d](float t) {
        return (1 - t)*(1 - t)*(1 - t)*a + 3*(1 - t)*(1 - t)*t*b + 3*(1 - t)*t*t*c + t*t*t*d;
    };

    float tolerance = 1.0f / 4.0f;
    GPoint errorVector0 = a - 2*b + c;
    GPoint errorVector1 = b - 2*c + d;
    float errorX = std::max(abs(errorVector0.x), abs(errorVector1.x));
    float errorY = std::max(abs(errorVector0.y), abs(errorVector1.y));
    float errorLength = sqrt(errorX * errorX + errorY * errorY);
    int numSegments = (int)ceil(sqrt((3 * errorLength)/(4 * tolerance)));
    float dt = 1.0f / static_cast<float>(numSegments);

    GPoint p0 = a;

    for (int i = 1; i < numSegments - 1; i++) {
        GPoint p1 = curve(i * dt);
        makeEdgeFunction(p0, p1);
        p0 = p1;
    }

    makeEdgeFunction(p0, d);
}

std::vector<SSEdge> edgesFromPath(const GPath& path, bool pathIsInsideBounds, const GRect& bitmapBounds) {
    GPath::Edger edger = GPath::Edger(path);
    GPoint points[GPath::kMaxNextPoints];
    std::vector<SSEdge> edges = std::vector<SSEdge>();

    auto makeEdgeNoClip = [&](GPoint p0, GPoint p1) {
        SSEdge edge = SSEdge::from_points(p0, p1);
        if (edge.isValid) edges.push_back(edge); 
    };

    auto clipAndMakeEdges = [&](GPoint p0, GPoint p1) {
        lineSegmentToEdges(p0, p1, bitmapBounds, edges);
    };

    if (pathIsInsideBounds) {
        while (auto verb = edger.next(points)) {
            switch (verb.value()) {
            case GPathVerb::kMove: {
                assert(false);
                break;
            }
            case GPathVerb::kLine: {
                makeEdgeNoClip(points[0], points[1]);
                break;
            }
            case GPathVerb::kQuad: {
                addEdgesFromQuad(points[0], points[1], points[2], makeEdgeNoClip);
                break;
            }
            case GPathVerb::kCubic: {
                addEdgesFromCubic(points[0], points[1], points[2], points[3], makeEdgeNoClip);
                break;
            }
            }
        }
    } else {
        while (auto verb = edger.next(points)) {
            switch (verb.value()) {
            case GPathVerb::kMove: {
                assert(false);
                break;
            }
            case GPathVerb::kLine: {
                clipAndMakeEdges(points[0], points[1]);
                break;
            }
            case GPathVerb::kQuad: {
                addEdgesFromQuad(points[0], points[1], points[2], clipAndMakeEdges);
                break;
            }
            case GPathVerb::kCubic: {
                addEdgesFromCubic(points[0], points[1], points[2], points[3], clipAndMakeEdges);
                break;
            }
            }
        }
    }

    return edges;
}

/// Sort all edges by y, using initial x as tie breaker
void sortEdgesByTopThenX(std::vector<SSEdge>& edges) {
    std::sort(edges.begin(), edges.end(), [](SSEdge a, SSEdge b) {
        if (a.top != b.top) {
            return a.top < b.top;
        } else {
            float xA = a.findXforY(a.top + 0.5f);
            float xB = b.findXforY(b.top + 0.5f);
            return xA < xB;
        } 
    });
}

void sortEdgesInX(std::list<SSEdge>& edges, std::list<SSEdge>::iterator sortEnd, int y) {
    // Find number of elements to sort
    const int numToSort = std::distance(edges.begin(), sortEnd);

    // Convert desired portion of list to vector
    std::vector<SSEdge> edgesToSort(numToSort);

    int i = 0;
    for (auto it = edges.begin(); it != sortEnd; it++) {
        edgesToSort[i] = std::move(*it);
        i += 1;
    }

    // Sort edges in vector
    std::sort(edgesToSort.begin(), edgesToSort.begin() + numToSort, [y](SSEdge a, SSEdge b) {
        float xA = a.findXforY(y + 0.5f);
        float xB = b.findXforY(y + 0.5f);
        return xA < xB;
    });

    // Assign sorted edges back to list
    int j = 0;

    for (auto it = edges.begin(); it != sortEnd; it++) {
        *it = std::move(edgesToSort[j]);
        j+= 1;
    }
}

template <typename BlendLambda, typename BlitRowFunction>
void SSCanvas::drawPathCommon(
    const GPath& path,
    const GPaint& paint,
    BlendLambda blend,
    BlitRowFunction blitRowFunction
) {
    // Transform path by CTM
    auto transformedPath = path.transform(getCTM());

    // Calculate bitmap and transformed path bounds
    const GRect bitmapBounds = GRect::LTRB(0, 0, bitmap.width() - 1, bitmap.height() - 1);
    const GRect transformedPathBounds = transformedPath->bounds();

    // If entire path is outside bitmap, exit early, no work to do.
    if (GRect_isOutside(transformedPathBounds, bitmapBounds)) return;

    // Build edges from path
    bool pathIsInsideBounds = GRect_isInside(transformedPathBounds, bitmapBounds);
    std::vector<SSEdge> edgesVector = edgesFromPath(*transformedPath, pathIsInsideBounds, bitmapBounds);

    // Sort all edges by y, using initial x as tie breaker
    sortEdgesByTopThenX(edgesVector);

    // Convert edges to std::list
    std::list<SSEdge> edges = std::list<SSEdge>(edgesVector.begin(), edgesVector.end());

    // Find min and max y values from edges array
    int minY = INT_MAX;
    int maxY = -INT_MAX;
    int minX = INT_MAX;
    int maxX = -INT_MAX;

    for (const SSEdge &edge : edges) {
        if (edge.top < minY) minY = edge.top;
        if (edge.bottom > maxY) maxY = edge.bottom;
        if (edge.left < minX) minX = edge.left;
        if (edge.right > maxX) maxX = edge.right;
    }

    // Find bounds
    const GIRect bounds = GIRect::LTRB(minX, minY, maxX, maxY);

    // Loop through all y's containing edges
    for (int y = bounds.top; y < bounds.bottom; y++) {
        auto edgesIterator = edges.begin();

        int w = 0;
        int L;

        // Loop through all valid edges for this y
        // while (i < edges.size() && edges[i].isValidAtY(y)) {
        while (edgesIterator != edges.end()) {
            const SSEdge currentEdge = *edgesIterator;

            // Stop loop if we've reached an edge that isn't valid for this y
            if (!currentEdge.isValidAtY(y)) break;

            // Find intersection with ray cast
            int x = GRoundToInt(currentEdge.findXforY(y + 0.5f));

            // If w equals zero, mark this as the start of a segment
            if (w == 0) L = x;

            // Modify w
            w += currentEdge.winding; // +1 or -1

            // If w now equals zero, fill between this x and L
            if (w == 0) {
                blitRowFunction(L, x, y, blend);
            }

            // Remove the current edge if it won't be hittable for next y
            if (currentEdge.isValidAtY(y + 1)) {
                edgesIterator++;
            } else {
                edgesIterator = edges.erase(edgesIterator);
            }
        }

        assert(w == 0);

        // Increase iterator until it accounts for all of edges that will be valid for the next y
        while (edgesIterator != edges.end() && edgesIterator->isValidAtY(y + 1)) {
            edgesIterator++;
        }

        // Sort edges on x intersections of next y
        sortEdgesInX(edges, edgesIterator, y + 1);
    }
}

void SSCanvas::drawPath(const GPath &path, const GPaint &paint) {
    // Read color and shader from paint
    const GColor color = paint.getColor();
    GShader *shader = paint.peekShader();

    // Simplify blend mode
    GBlendMode simplifiedBlendMode;

    if (shader) {
        simplifiedBlendMode = simplifyBlendMode(paint.getBlendMode(), shader->isOpaque(), false);
    } else {
        simplifiedBlendMode = simplifyBlendMode(paint.getBlendMode(), color.a == 1, color.a == 0);
    }

    // If blend mode is dst, no work to be done
    if (simplifiedBlendMode == GBlendMode::kDst) return;

    if (shader) {
        // Set CTM as context for shader. Return early if it failed
        bool setContextWasSuccessful = shader->setContext(getCTM());
        if (!setContextWasSuccessful) return;

        GPixel src[bitmap.width()];

        auto blitRowWithShader = [&src, this, &shader](int left, int right, int y, auto blend) {
            GPixel *row = this->bitmap.getAddr(left, y);
            shader->shadeRow(left, y, right - left, src);

            for (int j = 0; j < right - left; j++) {
                row[j] = blend(src[j], &row[j]);
            }
        };

        switch (simplifiedBlendMode) {
        case GBlendMode::kClear: {
            drawPathCommon(path, paint, blendClear, blitRowWithShader);
            break;
        }
        case GBlendMode::kSrc: {
            drawPathCommon(path, paint, blendSrc, blitRowWithShader);
            break;
        }
        case GBlendMode::kDst: {
            // No work to do, can return early
            return;
        }
        case GBlendMode::kSrcOver: {
            drawPathCommon(path, paint, blendSrcOver, blitRowWithShader);
            break;
        }
        case GBlendMode::kDstOver: {
            drawPathCommon(path, paint, blendDstOver, blitRowWithShader);
            break;
        }
        case GBlendMode::kSrcIn: {
            drawPathCommon(path, paint, blendSrcIn, blitRowWithShader);
            break;
        }
        case GBlendMode::kDstIn: {
            drawPathCommon(path, paint, blendDstIn, blitRowWithShader);
            break;
        }
        case GBlendMode::kSrcOut: {
            drawPathCommon(path, paint, blendSrcOut, blitRowWithShader);
            break;
        }
        case GBlendMode::kDstOut: {
            drawPathCommon(path, paint, blendDstOut, blitRowWithShader);
            break;
        }
        case GBlendMode::kSrcATop: {
            drawPathCommon(path, paint, blendSrcATop, blitRowWithShader);
            break;
        }
        case GBlendMode::kDstATop: {
            drawPathCommon(path, paint, blendDstATop, blitRowWithShader);
            break;
        }
        case GBlendMode::kXor: {
            drawPathCommon(path, paint, blendXor, blitRowWithShader);
            break;
        }
        }
    } else {
        // Premultiply source color
        GPixel src = colorToPixel(color);

        auto blitRowWithPixel = [&](int left, int right, int y, auto blend) {
            GPixel *row = bitmap.getAddr(left, y);

            for (int j = 0; j < right - left; j++) {
                row[j] = blend(src, &row[j]);
            }
        };

        switch (simplifiedBlendMode) {
        case GBlendMode::kClear: {
            drawPathCommon(path, paint, blendClear, blitRowWithPixel);
            break;
        }
        case GBlendMode::kSrc: {
            drawPathCommon(path, paint, blendSrc, blitRowWithPixel);
            break;
        }
        case GBlendMode::kDst: {
            // No work to do, can return early
            return;
        }
        case GBlendMode::kSrcOver: {
            drawPathCommon(path, paint, blendSrcOver, blitRowWithPixel);
            break;
        }
        case GBlendMode::kDstOver: {
            drawPathCommon(path, paint, blendDstOver, blitRowWithPixel);
            break;
        }
        case GBlendMode::kSrcIn: {
            drawPathCommon(path, paint, blendSrcIn, blitRowWithPixel);
            break;
        }
        case GBlendMode::kDstIn: {
            drawPathCommon(path, paint, blendDstIn, blitRowWithPixel);
            break;
        }
        case GBlendMode::kSrcOut: {
            drawPathCommon(path, paint, blendSrcOut, blitRowWithPixel);
            break;
        }
        case GBlendMode::kDstOut: {
            drawPathCommon(path, paint, blendDstOut, blitRowWithPixel);
            break;
        }
        case GBlendMode::kSrcATop: {
            drawPathCommon(path, paint, blendSrcATop, blitRowWithPixel);
            break;
        }
        case GBlendMode::kDstATop: {
            drawPathCommon(path, paint, blendDstATop, blitRowWithPixel);
            break;
        }
        case GBlendMode::kXor: {
            drawPathCommon(path, paint, blendXor, blitRowWithPixel);
            break;
        }
        }
    }
};