#include "SSCanvas.h"
#include "SSBlendModeHelpers.h"

/// Fill the entire canvas with the specified color, using SRC porter-duff mode.
void SSCanvas::clear(const GColor& color) {
    // Premultiply color
    GPixel new_pixel = colorToPixel(color);

    // Eagerly grab width and height, to help compiler speed up loop
    int width = bitmap.width();
    int height = bitmap.height();

    // Fill entire canvas with new color
    for (int y = 0; y < height; y++) {
        GPixel *row = bitmap.getAddr(0, y);

        for (int x = 0; x < width; x++) {
            row[x] = new_pixel;
        }
    }
}