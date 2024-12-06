#include "SSCanvas.h"

template<typename T>
const T* vectorToArray(const std::vector<T>& vec) {
    if (vec.empty()) {
        return nullptr;
    } else {
        return vec.data();
    }
}

/// Draw the quad, with optional color and/or texture coordinate at each corner. Tesselate
/// the quad based on "level":
///     level == 0 --> 1 quad  -->  2 triangles
///     level == 1 --> 4 quads -->  8 triangles
///     level == 2 --> 9 quads --> 18 triangles
///     ...
/// The 4 corners of the quad are specified in this order:
///     top-left --> top-right --> bottom-right --> bottom-left
/// Each quad is triangulated on the diagonal top-right --> bottom-left
///     0---1
///     |  /|
///     | / |
///     |/  |
///     3---2
///
/// colors and/or texs can be null. The resulting triangles should be passed to drawMesh(...).
void SSCanvas::drawQuad(
    const GPoint verts[4],
    const GColor colors[4], 
    const GPoint texs[4],
    int level, 
    const GPaint& paint
) {
    auto quadSample = [](float u, float v, auto a, auto b, auto c, auto d) {
        return (1-u)*(1-v)*a + (u)*(1-v)*b + (1-u)*(v)*d + (u)*(v)*c;
    };

    auto sampleFromArray = [quadSample](auto array[4], float u, float v){
        return quadSample(u, v, array[0], array[1], array[2], array[3]);
    };

    // Derive number of samples, subquads, and triangles from level
    const int samplesPerCol = level + 2;
    const int samplesPerRow = level + 2;

    const int subQuadsPerCol = level + 1;
    const int subQuadsPerRow = level + 1;
    const int totalSubQuads = subQuadsPerCol * subQuadsPerRow;

    const int totalTriangles = totalSubQuads * 2;

    // Build new vertices, colors, and textures arrays
    std::vector<GPoint> quadVertices;
    std::vector<GColor> quadColors;
    std::vector<GPoint> quadTexturePoints;

    const float fSubQuadsPerCol = subQuadsPerCol;
    const float fSubQuadsPerRow = subQuadsPerRow;

    for (int col = 0; col < samplesPerCol; col++) {
        for (int row = 0; row < samplesPerRow; row++) {
            float u = (float) col / fSubQuadsPerCol;
            float v = (float) row / fSubQuadsPerRow;

            quadVertices.push_back(sampleFromArray(verts, u, v));
            if (colors) quadColors.push_back(sampleFromArray(colors, u, v));
            if (texs) quadTexturePoints.push_back(sampleFromArray(texs, u, v));
        }
    }

    // Build indices array
    std::vector<int> quadIndices;

    for (int col = 0; col < subQuadsPerCol; col++) {
        for (int row = 0; row < subQuadsPerRow; row++) {
            // TL   TR
            // *-----*
            // |    /|
            // |   / |
            // |  /  |
            // | /   |
            // |/    |
            // *-----*
            // BL   BR
            int topLeft     = (row * samplesPerRow) + col;
            int topRight    = topLeft + 1;
            int bottomLeft  = topLeft + samplesPerRow;
            int bottomRight = bottomLeft + 1;

            // Build top left triangle in sub-quad
            quadIndices.push_back(topLeft);
            quadIndices.push_back(topRight);
            quadIndices.push_back(bottomLeft);

            // Build  bottom right triangle in sub-quad
            quadIndices.push_back(topRight);
            quadIndices.push_back(bottomRight);
            quadIndices.push_back(bottomLeft);
        }
    }

    // Call drawMesh
    this->drawMesh(
        vectorToArray(quadVertices),
        vectorToArray(quadColors),
        vectorToArray(quadTexturePoints),
        totalTriangles,
        vectorToArray(quadIndices),
        paint);
}