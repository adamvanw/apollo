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

typedef enum PaintMode {DRAW, ERASE, BEHIND};
void DrawPixel(SDL_Surface*, Vector2, Uint32, Uint8);
void DrawPixel_Line(SDL_Surface*, Vector2, Vector2, int, Uint32);
void DrawPixel_QuadCurve(SDL_Surface*, Vector2, Vector2, Vector2, int, Uint32);
void DrawPixel_CircleBrush(SDL_Surface*, Vector2, int, Uint32);

void ClearPixels(SDL_Surface*);

void DrawPixel(SDL_Surface* sur, Vector2 mousePos, Uint32 newColor, Uint8 opacity) { // TODO: Will eventually need a PaintMode parameter
    if (mousePos.x < 0 || mousePos.y < 0 || (int)mousePos.x >= sur->w || (int)mousePos.y >= sur->h) return;
    auto* ptr = (Uint32*)sur->pixels + sur->w*(int)mousePos.y + (int)mousePos.x;
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
    if (p0 == p2) return;

    for (float t = 0; t <= 1; t += 0.025) {
        double x = (1-t)*(1-t)*p0.x + 2*(1-t)*t*p1.x + t*t*p2.x;
        double y = (1-t)*(1-t)*p0.y + 2*(1-t)*t*p1.y + t*t*p2.y;
        DrawPixel_CircleBrush(sur, {x, y}, radius, color);
    }
}

// Maps point to current surface based on rotation, scale, and translation.
Point2 MapPoint(double x, double y, SDL_FPoint* center, float angle, float scale, SDL_FlipMode flipMode) {
    Point2 mouse = {x, y};
    double rads = -angle * SDL_PI_D / 180;
    Point2 offset = {0, 20 * scale};

    mouse.x = mouse.x - center->x;
    mouse.y = mouse.y - center->y;
    Matrix2 rotation = {cos(rads), sin(rads), -sin(rads), cos(rads)};
    mouse = rotation.multiplyV2(mouse);
    mouse = mouse - offset;
    mouse = mouse * (1 / scale);
    mouse.x = mouse.x + center->x;
    mouse.y = mouse.y + center->y;

    return mouse;
}

// Since we know our pixels are always going to be int32, we can make a fast small function that clears the pixels.
void ClearPixels(SDL_Surface* sur) {
    for (int i = 0; i < sur->w; i++) {
        for (int j = 0; j < sur->h; j++) {
            auto* ptr = (Uint32*)sur->pixels + sur->w*j + i;
            *ptr = 0x00000000;
        }
    }
}

void PaintOnTop(SDL_Surface* sur, SDL_Surface* newSur, float alpha) {
    if (alpha == 0.0f) return;
    Uint8 r, g, b, a;
    Uint8 nr, ng, nb, na;
    Uint8 rR, rG, rB, rA;
    Uint32 *ptr, *newPtr;
    Uint8 alpha8 = alpha * 255;
    Uint64 start = SDL_GetTicksNS();

    // assumed sur and newSur have the same format, w, h, etc.
    // if they don't!!! that's bad!!!
    auto* format = SDL_GetPixelFormatDetails(sur->format);
    for (int i = 0; i < sur->w; ++i) {
        for (int j = 0; j < sur->h; ++j) {
            newPtr = (Uint32*)newSur->pixels + newSur->w*j + i;
            SDL_GetRGBA(*newPtr, format, nullptr, &nr, &ng, &nb, &na);

            if (na == 0) continue;

            ptr = (Uint32*)sur->pixels + sur->w*j + i;
            SDL_GetRGBA(*ptr, format, nullptr, &r, &g, &b, &a);

            if (a == 0 || alpha8 == 255) {
                *ptr = SDL_MapRGBA(format, nullptr, nr, ng, nb, alpha8);
                continue;
            } else {
                rR = (Uint8)(r * (1 - alpha) + nr * alpha);
                rG = (Uint8)(g * (1 - alpha) + ng * alpha);
                rB = (Uint8)(b * (1 - alpha) + nb * alpha);
                rA = (Uint8)(a * (1 - alpha) + na * alpha);
                *ptr = SDL_MapRGBA(format, nullptr, rR, rG, rB, rA);
            }
        }
    }
    Uint64 end = SDL_GetTicksNS();
    SDL_Log("Blitting took: %llf", float((end - start) / 1000000));
}

void EraseOnTop(SDL_Surface* sur, SDL_Surface* newSur, float alpha) {
    if (alpha == 0.0f) return;
    Uint8 r, g, b, a;
    Uint8 na;
    Uint32 *ptr, *newPtr;
    Uint64 start = SDL_GetTicksNS();

    // assumed sur and newSur have the same format, w, h, etc.
    // if they don't!!! that's bad!!!
    auto* format = SDL_GetPixelFormatDetails(sur->format);
    for (int i = 0; i < sur->w; ++i) {
        for (int j = 0; j < sur->h; ++j) {
            newPtr = (Uint32*)newSur->pixels + newSur->w*j + i;
            SDL_GetRGBA(*newPtr, format, nullptr, nullptr, nullptr, nullptr, &na);

            // if the working layer has no alpha at that pixel, skip.
            // (there's nothing to erase, it's virtually empty)
            if (na == 0) continue;

            ptr = (Uint32*)sur->pixels + sur->w*j + i;

            // if the working layer has a full alpha layer, set the current pixel to 0x00000000
            if (na == 255 && alpha >= 1.0f) {
                *ptr = 0x00000000;
                continue;
            }

            // otherwise, perform an actual erase operation
            SDL_GetRGBA(*ptr, format, nullptr, &r, &g, &b, &a);

            // r, g, and b are not manipulated in the erase function.
            *ptr = SDL_MapRGBA(format, nullptr, r, g, b, Uint8(a * alpha));
        }
    }

    Uint64 end = SDL_GetTicksNS();
    SDL_Log("Blitting took: %llf", float((end - start) / 1000000));
}


#endif //APOLLO_DRAW_H
