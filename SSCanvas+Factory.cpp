#include "SSCanvas.h"

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    return std::unique_ptr<GCanvas>(new SSCanvas(device));
}