#ifndef APOLLO_DRAW_H
#define APOLLO_DRAW_H

#ifndef SDL3_H
#define SDL3_H
#include "SDL3/SDL.h"
#endif

#ifndef VECTOR
#define VECTOR
#include <vector>
#endif

void DrawPixel(SDL_Surface*, Vector2, Uint32, Uint8);
void DrawPixel_Line(SDL_Surface*, Vector2, Vector2, int, Uint32);
void DrawPixel_QuadCurve(SDL_Surface*, Vector2, Vector2, Vector2, int, Uint32);
void DrawPixel_CircleBrush(SDL_Surface*, Vector2, int, Uint32);

void DrawPixel(SDL_Surface* sur, Vector2 mousePos, Uint32 newColor, Uint8 opacity) { // TODO: Will eventually need a PaintMode parameter
    if (mousePos.x < 0 || mousePos.y < 0 || (int)mousePos.x >= sur->w || (int)mousePos.y >= sur->h) return;
    auto* ptr = (Uint32*)sur->pixels + 1280*(int)mousePos.y + (int)mousePos.x;
    *ptr = newColor;
}

void DrawPixel_Line(SDL_Surface* sur, Vector2 mousePos, Vector2 prevPos, int radius, Uint32 color) {
    double dx = mousePos.x - prevPos.x;
    double dy = mousePos.y - prevPos.y;
    double step;

    step = fabs((fabs(dx) >= fabs(dy)) ? fabs(dx) : fabs(dy));

    dx /= step;
    dy /= step;

    double x = prevPos.x, y = prevPos.y;

    int i = 0;
    while (i++ <= (int)step) {
        if ((int)mousePos.x == (int)x && (int)mousePos.y == (int)y) break;
        DrawPixel_CircleBrush(sur, {x, y}, radius, color);
        x += dx;
        y += dy;
    }
}

void DrawPixel_CircleBrush(SDL_Surface* sur, Vector2 origin, int radius, Uint32 color) {
    for (int y = 0; y <= radius * 2; ++y) {
        for (int x = 0; x <= radius * 2; ++x) {
            int dX = radius - x;
            int dY = radius - y;
            double distance = sqrtf(dX * dX + dY * dY);
            Uint8 opacity = 255;

            if (distance > (float)(1 + radius)) continue;

            if (radius / distance < 0.9) continue;

            DrawPixel(sur, {(double)(x - radius) + origin.x, (double)(y - radius) + origin.y}, color, opacity);
        }
    }
}


void DrawPixel_QuadCurve(SDL_Surface* sur, Vector2 p0, Vector2 p1, Vector2 p2, int radius, Uint32 color) {
    if (p1 == p2) return;

    for (float t = 0; t <= 1; t += 0.025) {
        double x = (1-t)*(1-t)*p0.x + 2*(1-t)*t*p1.x + t*t*p2.x;
        double y = (1-t)*(1-t)*p0.y + 2*(1-t)*t*p1.y + t*t*p2.y;
        DrawPixel_CircleBrush(sur, {x, y}, radius, color);
    }
}

#endif //APOLLO_DRAW_H
