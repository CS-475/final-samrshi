#include "SSCanvas.h"
#include "include/GPathBuilder.h"
#include "include/GShader.h"

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    // Colors
    GColor orange1 = GColor::RGBA(0.94f, 0.66f, 0.38f, 1.0f);
    GColor orange2 = GColor::RGBA(0.92f, 0.52f, 0.20f, 1.0f);
    GColor orange3 = GColor::RGBA(0.87f, 0.48f, 0.18f, 1.0f);
    GColor orange4 = GColor::RGBA(0.67f, 0.37f, 0.13f, 1.0f);
    GColor black = GColor::RGBA(0, 0, 0, 1);

    // Orange background
    GPoint verts[] = {
        { 0, 0 },
        { dim.width, 0 },
        { dim.width, dim.height },
        { 0, dim.height }
    };

    GColor colors[] = {
        orange1, orange2, orange3, orange4
    };

    canvas->drawQuad(
        verts,
        colors,
        nullptr,
        12,
        GPaint()
    );

    // Eyes
    GPoint eye[] = {
        { 2*32 + 8, 1*32 },
        { 3*32 + 16, 4*32 },
        { 1*32, 4*32 },
    };

    GPathBuilder eyesBuilder;
    eyesBuilder.addPolygon(eye, 3);
    canvas->drawPath(*eyesBuilder.detach(), GPaint(black));
    canvas->translate(3*32 + 16, 0);
    eyesBuilder.addPolygon(eye, 3);
    canvas->drawPath(*eyesBuilder.detach(), GPaint(black));
    canvas->restore();

    // Teeth
    GPathBuilder teethBuilder;
    teethBuilder.moveTo(1 * 32, 5 * 32);

    for (int i = 0; i < 6; i++) {
        teethBuilder.lineTo((1 + i) * 32 + 16, 5 * 32 + 12);
        teethBuilder.lineTo((2 + i) * 32, 5 * 32);
    }

    for (int i = 5; i >= 0; i--) {
        teethBuilder.lineTo((2 + i) * 32, 7 * 32);
        teethBuilder.lineTo((1 + i) * 32 + 16, 7 * 32 - 12);
    }

    teethBuilder.lineTo(1 * 32, 7 * 32);
    teethBuilder.lineTo(1 * 32, 5 * 32);
    canvas->drawPath(*teethBuilder.detach(), GPaint(black));

    return "PUMPKIN";
}