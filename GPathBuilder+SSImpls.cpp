#include "include/GPathBuilder.h"

/// Append a new contour to this path, made up of the 4 points of the specified rect,
/// in the specified direction.
/// 
/// In either direction the contour must begin at the top-left corner of the rect.
void GPathBuilder::addRect(const GRect& rect, GPathDirection direction) {
    // Move to top-left corner
    moveTo({ rect.left, rect.top });

    switch (direction) {
    case GPathDirection::kCW: {
        lineTo({ rect.right, rect.top });
        lineTo({ rect.right, rect.bottom });
        lineTo({ rect.left, rect.bottom });
        lineTo({ rect.left, rect.top });
        break;
    }
    case GPathDirection::kCCW: {
        lineTo({ rect.left, rect.bottom });
        lineTo({ rect.right, rect.bottom });
        lineTo({ rect.right, rect.top });
        lineTo({ rect.left, rect.top });
        break;
    }
    }
}

/// Append a new contour to this path with the specified polygon.
/// Calling this is equivalent to calling moveTo(pts[0]), lineTo(pts[1..count-1]).
void GPathBuilder::addPolygon(const GPoint pts[], int count) {
    // Exit early if count is zero
    if (count < 1) return;

    // Move to first point
    moveTo(pts[0]);

    // Draw lines to subsequent points
    for (int i = 1; i < count; i++) {
        lineTo(pts[i]);
    }
}

/// Append a new contour respecting the Direction. The contour should be an approximate
/// circle (8 quadratic curves will suffice) with the specified center and radius.
void GPathBuilder::addCircle(GPoint center, float radius, GPathDirection direction) {
    /// TODO: Test this

    // Create points along unit circle
    const int numPoints = 13;
    float k = (4.0f* sqrt(2.0f) - 4.0f) / 3.0f;

    GPoint counterClockWisePoints[numPoints] = {
        // Quadrant 1 (+, -)
        { 1, 0 },
        { 1, -k },
        { k, -1 },

        // Quadrant 2 (-, -)
        { 0, -1 },
        { -k, -1 },
        { -1, -k },

        // Quadrant 3 (-, +)
        { -1, 0 },
        { -1, k },
        { -k, 1 },

        // Quadrant 4 (+, +)
        { 0, 1 },
        { k, 1 },
        { 1, k },

        // Closing Point
        {1, 0}
    };

    // Scale and translate points to correct size and position
    GMatrix matrix = GMatrix::Translate(center.x, center.y) * GMatrix::Scale(radius, radius);
    matrix.mapPoints(counterClockWisePoints, numPoints);

    // Add points to path, depending on direction
    switch (direction) {
    case GPathDirection::kCW: {
        moveTo(counterClockWisePoints[numPoints - 1]);

        for (int i = numPoints - 2; i > 0; i -= 3) {
            cubicTo(counterClockWisePoints[i], counterClockWisePoints[i-1], counterClockWisePoints[i-2]);
        }
        break;
    }
    case GPathDirection::kCCW: {
        moveTo(counterClockWisePoints[0]);

        for (int i = 1; i < numPoints; i += 3) {
            cubicTo(counterClockWisePoints[i], counterClockWisePoints[i+1], counterClockWisePoints[i+2]);
        }
        break;
    }
    }
}