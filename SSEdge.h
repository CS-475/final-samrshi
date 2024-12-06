#ifndef SSEdge_DEFINED
#define SSEdge_DEFINED

#include "include/GTypes.h"

/// Find m and b (x = my + b) from two points
static inline void find_m_and_b(const GPoint& p0, const GPoint& p1, float& m, float& b) {
    m = (p1.x - p0.x) / (p1.y - p0.y);
    b = p0.x - (m * p0.y);
}

struct SSEdge {
    float m;
    float b;
    int top;
    int bottom;
    int left;
    int right;
    int winding; // must be +1 or -1
    bool isValid;

    static inline SSEdge from_points(const GPoint& p0, const GPoint& p1) { 
        // Find winding value w
        int winding;

        if (p0.y > p1.y) {
            winding = 1;
        } else if (p0.y < p1.y) {
            winding = -1;
        } else {
            winding = 0;
        }

        // Return edge
        return SSEdge::from_points(p0, p1, winding);
    }

    static inline SSEdge from_points(const GPoint& p0, const GPoint& p1, const int& winding) {
        SSEdge edge;

        // Find m and b from x = my + b
        find_m_and_b(p0, p1, edge.m, edge.b);

        // Find top-most and bottom-most y's
        edge.top = GRoundToInt(std::min(p0.y, p1.y));
        edge.bottom = GRoundToInt(std::max(p0.y, p1.y));

        // Find right-most and bottom-most y's
        edge.left = GRoundToInt(std::min(p0.x, p1.x));
        edge.right = GRoundToInt(std::max(p0.x, p1.x));

        // Edge is valid if top is above than bottom
        edge.isValid = edge.top < edge.bottom;

        // Set winding
        edge.winding = winding;

        return edge;
    }

    bool isValidAtY(const int& y) const {
        return top <= y && y < bottom;
    }

    float findXforY(const float& y) const {
        return (m * y) + b;
    }
};

static inline void appendEdgeIfValid(const SSEdge& edge, std::vector<SSEdge>& edges) {
    if (edge.isValid) edges.push_back(edge);
}

static inline float find_corresponding_x(const GPoint& p0, const GPoint& p1, const float& target_y) {
    // Find line equation variables
    float m, b;
    find_m_and_b(p0, p1, m, b);

    // Plug in y to get x
    return (m * target_y) + b;
}

static inline float find_corresponding_y(const GPoint& p0, const GPoint& p1, const float& target_x) {
    // Find line equation variables
    float m, b;
    find_m_and_b(p0, p1, m, b);

    // Plug those and x into y = p0.y + (x - p0.x) / m
    return p0.y + (target_x - p0.x) / m;
}

static inline void lineSegmentToEdges(GPoint p0, GPoint p1, const GRect& bounds, std::vector<SSEdge>& edges) {
    // Ignore line if it is completely below or above bounds
    bool completely_above = p0.y < bounds.top && p1.y < bounds.top;
    bool completely_below = p0.y > bounds.bottom && p1.y > bounds.bottom;

    if (completely_above || completely_below) {
        return;
    }

    // Calculate winding value early
    int winding;

    if (p0.y > p1.y) {
        winding = 1;
    } else if (p0.y < p1.y) {
        winding = -1;
    } else {
        winding = 0;
    }

    // To simplify vertical chop, ensure that p0 is the highest point
    if (p0.y > p1.y) {
        std::swap(p0, p1);
    }

    // If line straddles top, set p0 to intersection between line and top
    if (p0.y < bounds.top) {
        p0.x = find_corresponding_x(p0, p1, bounds.top);
        p0.y = bounds.top;
        // assert(p0.x >= bounds.left && p0.x <= bounds.right);
    }

    // If line straddles bottom, set p1 to intersection between line and bottom
    if (p1.y > bounds.bottom) {
        p1.x = find_corresponding_x(p0, p1, bounds.bottom);
        p1.y = bounds.bottom;
        // assert(p1.x >= bounds.left && p1.x <= bounds.right);
    }

    // To simplify horizontal chop, ensure that p0 is the leftmost point
    if (p0.x > p1.x) {
        std::swap(p0, p1);
    }

    // Snap line to edge if completely left of bounds
    bool completely_left = p0.x < bounds.left && p1.x < bounds.left;

    if (completely_left) {
        GPoint new_p0 = {bounds.left, p0.y};
        GPoint new_p1 = {bounds.left, p1.y};
        SSEdge new_edge = SSEdge::from_points(new_p0, new_p1, winding);
        appendEdgeIfValid(new_edge, edges);
        return;
    }

    // Snap line to edge if completely right of bounds
    bool completely_right = p0.x > bounds.right && p1.x > bounds.right;

    if (completely_right) {
        GPoint new_p0 = {bounds.right, p0.y};
        GPoint new_p1 = {bounds.right, p1.y};
        SSEdge new_edge = SSEdge::from_points(new_p0, new_p1, winding);
        appendEdgeIfValid(new_edge, edges);
        return;
    }

    // If line straddles left, set p0 to intersection between line and left and add extra edge along left boundary
    if (p0.x < bounds.left) {
        float old_p0_y = p0.y;

        // Update p0
        p0.y = find_corresponding_y(p0, p1, bounds.left);
        p0.x = bounds.left;
        // assert(p0.y >= bounds.top && p0.y <= bounds.bottom);

        // Create new edge along left boundary
        GPoint new_edge_p0 = { bounds.left, old_p0_y };
        GPoint new_edge_p1 = { bounds.left, p0.y };
        SSEdge new_edge = SSEdge::from_points(new_edge_p0, new_edge_p1, winding);
        appendEdgeIfValid(new_edge, edges);
    }

    // If line straddles right, set p1 to intersection between line and right and add extra edge along right boundary
    if (p1.x > bounds.right) {
        float old_p1_y = p1.y;

        // Update p1
        p1.y = find_corresponding_y(p0, p1, bounds.right);
        p1.x = bounds.right;
        // assert(p1.y >= bounds.top && p1.y <= bounds.bottom);

        // Create new edge along right boundary
        GPoint new_edge_p0 = { bounds.right, old_p1_y };
        GPoint new_edge_p1 = { bounds.right, p1.y };
        SSEdge new_edge = SSEdge::from_points(new_edge_p0, new_edge_p1, winding);
        appendEdgeIfValid(new_edge, edges);
    }

    // Build edge from p0, p1 and add to result
    SSEdge edge = SSEdge::from_points(p0, p1, winding);
    appendEdgeIfValid(edge, edges);
}

static inline bool compareEdges(const SSEdge& l, const SSEdge& r) {
    return l.top < r.top;
}

static inline std::vector<SSEdge> makeEdges(const GPoint points[], const int& count, const GRect& bounds) {
    std::vector<SSEdge> edges = {};
    edges.reserve(count * 2);

    for (int i = 0; i < count; i++) {
        GPoint p0 = points[i];
        GPoint p1 = points[i == count - 1 ? 0 : i + 1];
        lineSegmentToEdges(p0, p1, bounds, edges);
    }

    std::sort(edges.begin(), edges.end(), compareEdges);

    return edges;
}

static inline float SSEdge_yIntersection(const SSEdge& edge, const float& y) {
    // x = my + b
    return (edge.m * y) + edge.b;
}

#endif // SSEdge_DEFINED