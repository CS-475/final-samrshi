#include "include/GPath.h"
#include "include/GPathBuilder.h"

void updateBoundsIfNeeded(GRect& bounds, const GPoint& point) {
    if (point.x < bounds.left)   bounds.left = point.x;
    if (point.y < bounds.top)    bounds.top = point.y;
    if (point.x > bounds.right)  bounds.right = point.x;
    if (point.y > bounds.bottom) bounds.bottom = point.y;
}

void updateBoundsFromQuad(GRect& bounds, const GPoint points[]) {
    // Update bounds with endpoints if needed
    updateBoundsIfNeeded(bounds, points[0]);
    updateBoundsIfNeeded(bounds, points[2]);

    // Find local extrema
    auto curve = [](GPoint A, GPoint B, GPoint C, float t) {
        return (1 - t)*(1 - t)*A + 2*(1 - t)*t*B + t*t*C;
    };

    auto findExtremes = [](float a, float b, float c) {
        /// TODO: how to handle div by zero?
        // assert(2*a - 4*b + 2*c != 0);
        return (2*a - 2*b) / (2*a - 4*b + 2*c);
    };

    // Check x extreme
    float xExtremeT = findExtremes(points[0].x, points[1].x, points[2].x);

    if (xExtremeT >= -0.0001f && xExtremeT <= 1.0001f) {
        GPoint xExtreme = curve(points[0], points[1], points[2], xExtremeT);
        if (xExtreme.x < bounds.left)   bounds.left   = xExtreme.x;
        if (xExtreme.x > bounds.right)  bounds.right  = xExtreme.x;
    }

    // Check y extreme
    float yExtremeT = findExtremes(points[0].y, points[1].y, points[2].y);

    if (yExtremeT >= -0.0001f && yExtremeT <= 1.0001f) {
        GPoint yExtreme = curve(points[0], points[1], points[2], yExtremeT);
        if (yExtreme.y < bounds.top)    bounds.top    = yExtreme.y;
        if (yExtreme.y > bounds.bottom) bounds.bottom = yExtreme.y;
    }
}

void updateBoundsFromCubic(GRect& bounds, const GPoint points[]) {
    // Update bounds with endpoints if needed
    updateBoundsIfNeeded(bounds, points[0]);
    updateBoundsIfNeeded(bounds, points[3]);

    // Find local extrema
    auto curve = [](GPoint A, GPoint B, GPoint C, GPoint D, float t) {
        return (1 - t)*(1 - t)*(1 - t)*A + 3*(1 - t)*(1 - t)*t*B + 3*(1 - t)*t*t*C + t*t*t*D;
    };

    auto findExtremes = [](float a, float b, float c, float d) {
        float quadraticA = -a + 3*b - 3*c + d;
        float quadraticB = 2 * (a - 2*b + c);
        float quadraticC = -a + b;

        if (abs(quadraticA) < 0.0001f) {
            // Solve linear equation Bt + C = 0
            float t = -quadraticC / quadraticB;
            return std::make_pair(t, t);
        } else {
            // Solve quadratic formula (-b ± sqrt(b^2 - 4ac)) / 2a
            float insideSqrt = quadraticB * quadraticB - 4 * quadraticA * quadraticC;
            if (insideSqrt < 0) return std::make_pair(-1.0f, -1.0f);

            float t1 = (-quadraticB + sqrt(insideSqrt)) / (2 * quadraticA);
            float t2 = (-quadraticB - sqrt(insideSqrt)) / (2 * quadraticA);
            return std::make_pair(t1, t2);
        }
    };

    // Check x extremes
    std::pair<float, float> xExtremeTs = findExtremes(points[0].x, points[1].x, points[2].x, points[3].x);

    if (xExtremeTs.first >= -0.0001f && xExtremeTs.first <= 1.0001f) {
        GPoint xExtreme = curve(points[0], points[1], points[2], points[3], xExtremeTs.first);
        if (xExtreme.x < bounds.left)   bounds.left   = xExtreme.x;
        if (xExtreme.x > bounds.right)  bounds.right  = xExtreme.x;
    }

    if (xExtremeTs.second >= -0.0001f && xExtremeTs.second <= 1.0001f) {
        GPoint xExtreme = curve(points[0], points[1], points[2], points[3], xExtremeTs.second);
        if (xExtreme.x < bounds.left)   bounds.left   = xExtreme.x;
        if (xExtreme.x > bounds.right)  bounds.right  = xExtreme.x;
    }

    // Check y extremes
    std::pair<float, float> yExtremeTs = findExtremes(points[0].y, points[1].y, points[2].y, points[3].y);

    if (yExtremeTs.first >= -0.0001f && yExtremeTs.first <= 1.0001f) {
        GPoint yExtreme = curve(points[0], points[1], points[2], points[3], yExtremeTs.first);
        if (yExtreme.y < bounds.top)    bounds.top    = yExtreme.y;
        if (yExtreme.y > bounds.bottom) bounds.bottom = yExtreme.y;
    }

    if (yExtremeTs.second >= -0.0001f && yExtremeTs.second <= 1.0001f) {
        GPoint yExtreme1 = curve(points[0], points[1], points[2], points[3], yExtremeTs.second);
        if (yExtreme1.y < bounds.top)    bounds.top    = yExtreme1.y;
        if (yExtreme1.y > bounds.bottom) bounds.bottom = yExtreme1.y;
    }
}

/// Return the bounds of all of the control-points in the path.
///
/// If there are no points, returns an empty rect (all zeros)
GRect GPath::bounds() const {
    // Return empty rect if there are no points
    if (fPts.empty()) return GRect();

    // Otherwise, check all points
    GRect bounds = GRect::LTRB(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);

    GPath::Iter iterator(*this);
    GPoint points[GPath::kMaxNextPoints];

    while (nonstd::optional<GPathVerb> verb = iterator.next(points)) {
        switch (verb.value()) {
            case GPathVerb::kMove: {
                updateBoundsIfNeeded(bounds, points[0]);
                break;
            }
            case GPathVerb::kLine: {
                updateBoundsIfNeeded(bounds, points[0]);
                updateBoundsIfNeeded(bounds, points[1]);
                break;
            }
            case GPathVerb::kQuad: {
                updateBoundsFromQuad(bounds, points);
                break;
            }
            case GPathVerb::kCubic: {
                updateBoundsFromCubic(bounds, points);
                break;
            }
        }
    }

    return bounds;
}

/// Given 0 < t < 1, subdivide the src[] quadratic bezier at t into two new quadratics in dst[]
/// such that
/// 0...t is stored in dst[0..2]
/// t...1 is stored in dst[2..4]
void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
    GPoint a = src[0];
    GPoint b = src[1];
    GPoint c = src[2];

    auto ab = [a, b](float t) { return a + t*(b - a); };
    auto bc = [b, c](float t) { return b + t*(c - b); };
    auto ab_to_bc = [ab, bc](float t) { return (1 - t)*ab(t) + t*bc(t); };

    dst[0] = a;
    dst[1] = ab(t);
    dst[2] = ab_to_bc(t);
    dst[3] = bc(t);
    dst[4] = c;
}

/// Given 0 < t < 1, subdivide the src[] cubic bezier at t into two new cubics in dst[]
/// such that
/// 0...t is stored in dst[0..3]
/// t...1 is stored in dst[3..6]
void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
    GPoint a = src[0];
    GPoint b = src[1];
    GPoint c = src[2];
    GPoint d = src[3];

    auto ab = [a, b](float t) { return a + t*(b - a); };
    auto bc = [b, c](float t) { return b + t*(c - b); };
    auto cd = [c, d](float t) { return c + t*(d - c); };
    auto ab_to_bc = [ab, bc](float t) { return (1 - t)*ab(t) + t*bc(t); };
    auto bc_to_cd = [bc, cd](float t) { return (1 - t)*bc(t) + t*cd(t); };
    auto abbc_to_bccd = [ab_to_bc, bc_to_cd](float t) { return (1 - t)*ab_to_bc(t) + t*bc_to_cd(t); };

    dst[0] = a;
    dst[1] = ab(t);
    dst[2] = ab_to_bc(t);
    dst[3] = abbc_to_bccd(t);
    dst[4] = bc_to_cd(t);
    dst[5] = cd(t);
    dst[6] = d;
}