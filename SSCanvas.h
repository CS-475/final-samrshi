#ifndef SSCanvas_DEFINED
#define SSCanvas_DEFINED

#include "include/GCanvas.h"
#include "include/GRect.h"
#include "include/GColor.h"
#include "include/GBitmap.h"
#include "include/GShader.h"
#include "include/GPath.h"

class SSCanvas : public GCanvas {
public:
    /// Instantiate an SSCanvas
    SSCanvas(const GBitmap& bitmap) 
        : matrices({GMatrix()})
        , bitmap(bitmap) 
    {}

    /// Save off a copy of the canvas state (CTM), to be later used if the balancing call to
    /// restore() is made. Calls to save/restore can be nested:
    /// save();
    ///     save();
    ///         concat(...);    // this modifies the CTM
    ///         .. draw         // these are drawn with the modified CTM
    ///     restore();          // now the CTM is as it was when the 2nd save() call was made
    ///     ..
    /// restore();              // now the CTM is as it was when the 1st save() call was made
    void save();

    /// Copy the canvas state (CTM) that was record in the correspnding call to save() back into
    /// the canvas. It is an error to call restore() if there has been no previous call to save().
    void restore();

    /// Modifies the CTM by preconcatenating the specified matrix with the CTM. The canvas
    /// is constructed with an identity CTM.
    ///
    /// CTM' = CTM * matrix
    void concat(const GMatrix&);

    /// Fill the entire canvas with the specified color, using SRC porter-duff mode.
    void clear(const GColor&);

    /// Fill the rectangle with the color, using the specified blendmode.
    ///
    /// The affected pixels are those whose centers are "contained" inside the rectangle:
    /// e.g. contained == center > min_edge && center <= max_edge
    void drawRect(const GRect&, const GPaint&);

    /// Fill the convex polygon with the color and blendmode,
    /// following the same "containment" rule as rectangles.
    void drawConvexPolygon(const GPoint[], int count, const GPaint&);    

    /// Fill the path with the paint, interpreting the path using winding-fill (non-zero winding).
    void drawPath(const GPath&, const GPaint&);

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
    void drawMesh(
        const GPoint verts[], const GColor colors[], const GPoint texs[],
        int count, const int indices[], const GPaint& paint);
    
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
    virtual void drawQuad(
        const GPoint verts[4], const GColor colors[4], 
        const GPoint texs[4], int level, const GPaint&);

private:
    /// Get the current transformation matrix from the top of the stack.
    GMatrix getCTM();

    /// Stack of transformation matrices.
    std::vector<GMatrix> matrices;

    /// Shared implementation of drawPath
    template <typename BlendLambda, typename BlitRowFunction>
    void drawPathCommon(const GPath&, const GPaint&, BlendLambda, BlitRowFunction);

    /// Shared implementation of drawConvexPoly
    template <typename BlendFunction, typename BlitRowFunction>
    void blitConvexPolyCommon(const GPoint[], int count, BlendFunction, BlitRowFunction);

    /// Shared implementation of drawConvexPoly
    template <typename MakeShaderFunction, typename UpdateShaderFunction>
    void drawMeshCommon(
        const GPoint verts[], 
        int triangleCount,
        const int indices[],
        const GPaint& paint,
        MakeShaderFunction makeShader,
        UpdateShaderFunction updateShader
    );

    /// The bitmap that this canvas draws to
    const GBitmap bitmap;
};

#endif