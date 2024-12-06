#include "SSCanvas.h"
#include "SSTriangleColorShader.h"
#include "SSTriangleTextureShader.h"
#include "SSTriangleModulatingShader.h"

template <typename MakeShaderFunction, typename UpdateShaderFunction>
void SSCanvas::drawMeshCommon(
    const GPoint verts[], 
    int triangleCount,
    const int indices[],
    const GPaint& paint,
    MakeShaderFunction makeShader,
    UpdateShaderFunction updateShader
) {
    GPaint newPaint = paint;

    auto newShader = makeShader(0, 1, 2);
    newPaint.setShader(newShader);

    GPoint points[3];

    int indicesCount = triangleCount * 3;
    for (int i = 0; i < indicesCount; i += 3) {
        int index0 = indices[i + 0];
        int index1 = indices[i + 1];
        int index2 = indices[i + 2];
        updateShader(newShader, index0, index1, index2);

        points[0] = verts[index0];
        points[1] = verts[index1];
        points[2] = verts[index2];

        drawConvexPolygon(points, 3, newPaint);
    }
}

/// Draw a mesh of triangles, with optional colors and/or texture-coordinates at each vertex.
///
/// The triangles are specified by successive triples of indices.
///     int n = 0;
///     for (i = 0; i < count; ++i) {
///         point0 = vertx[indices[n+0]]
///         point1 = verts[indices[n+1]]
///         point2 = verts[indices[n+2]]
///         ...
///         n += 3
///     }
///
/// If colors is not null, then each vertex has an associated color, to be interpolated
/// across the triangle. The colors are referenced in the same way as the verts.
///         color0 = colors[indices[n+0]]
///         color1 = colors[indices[n+1]]
///         color2 = colors[indices[n+2]]
///
/// If texs is not null, then each vertex has an associated texture coordinate, to be used
/// to specify a coordinate in the paint's shader's space. If there is no shader on the
/// paint, then texs[] should be ignored. It is referenced in the same way as verts and colors.
///         texs0 = texs[indices[n+0]]
///         texs1 = texs[indices[n+1]]
///         texs2 = texs[indices[n+2]]
///
/// If both colors and texs[] are specified, then at each pixel their values are multiplied
/// together, component by component.
void SSCanvas::drawMesh(
    const GPoint verts[], 
    const GColor colors[],
    const GPoint texs[],
    int triangleCount,
    const int indices[],
    const GPaint& paint
) {
    auto makeColorShader = [&](int index0, int index1, int index2) {
        return SSCreateTriangleColorShader(
            verts[index0], verts[index1], verts[index2],
            colors[index0], colors[index1], colors[index2]
        );
    };

    auto updateColorShader = [&](auto colorShader, int index0, int index1, int index2) {
        colorShader->updateShader(
            verts[index0], verts[index1], verts[index2],
            colors[index0], colors[index1], colors[index2]
        );
    };

    auto makeTextureShader = [&](int index0, int index1, int index2) {
        return SSCreateTriangleTextureShader(
            paint.shareShader(),
            verts[index0], verts[index1], verts[index2],
            texs[index0], texs[index1], texs[index2]
        );
    };

    auto updateTextureShader = [&](auto textureShader, int index0, int index1, int index2) {
        textureShader->updateShader(
            verts[index0], verts[index1], verts[index2],
            texs[index0], texs[index1], texs[index2]
        );
    };

    auto makeModulatingShader = [&](int index0, int index1, int index2) {
        auto colorShader = makeColorShader(index0, index1, index2);
        auto textureShader = makeTextureShader(index0, index1, index2);
        return SSCreateModulatingShader(colorShader, textureShader);
    };

    auto updateModulatingShader = [&](auto modulatingShader, int index0, int index1, int index2) {
        updateColorShader(modulatingShader->colorShader, index0, index1, index2);
        updateTextureShader(modulatingShader->textureShader, index0, index1, index2);
    };

    if (colors != nullptr && texs != nullptr) {
        drawMeshCommon(verts, triangleCount, indices, paint, makeModulatingShader, updateModulatingShader);
    } else if (colors != nullptr) {
        drawMeshCommon(verts, triangleCount, indices, paint, makeColorShader, updateColorShader);
    } else if (texs != nullptr) {
        drawMeshCommon(verts, triangleCount, indices, paint, makeTextureShader, updateTextureShader);
    }
}