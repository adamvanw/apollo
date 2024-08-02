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

void DrawPixel(SDL_Surface*, Vector2, Uint32);
void DrawPixel_Line(SDL_Surface*, Vector2, Vector2, int, Uint32);
void DrawPixel_CubicCurve(SDL_Surface*, Vector2, Vector2, Vector2, Vector2, int, Uint32);
void DrawPixel_CircleBrush(SDL_Surface*, Vector2, int, Uint32);
/*
void deCasteljau(SDL_Surface* sur, std::vector<Vector2> points, int depth, Uint32 color) {
    SDL_Log("Depth: %d. Points: %d.", depth, points.size());
    if (depth == 0 || points.size() == 1) {
        SDL_Log("deCasteljau return called! Points amount: %d", points.size());
        DrawPixel(sur, {points[0].x, points[0].y}, color);
        if (points.size() > 1) {
            for (int i = 0; i < points.size() - 1; ++i) {
                DrawPixel_Line(sur, points[i + 1], points[i], color);
            }
        }
        return;
    }

    std::vector<Vector2> newPoints;
    for (size_t i = 0; i < points.size() - 1; ++i) {
        Vector2 p = {(points[i].x + points[i + 1].x) / 2, (points[i].y + points[i + 1].y) / 2};
        newPoints.push_back(p);
    }
    newPoints.insert(newPoints.begin(), points[0]);
    newPoints.push_back(points[points.size() - 1]);

    deCasteljau(sur, newPoints, --depth, color);
}
 */

void DrawPixel(SDL_Surface* sur, Vector2 mousePos, Uint32 color) {
    if (mousePos.x < 0 || mousePos.y < 0 || (int)mousePos.x >= sur->w || (int)mousePos.y >= sur->h) return;
    auto* ptr = (Uint32*)sur->pixels + 1280*(int)mousePos.y + (int)mousePos.x;
    *ptr = color;
}

void DrawPixel_Line(SDL_Surface* sur, Vector2 mousePos, Vector2 prevPos, int radius, Uint32 color) {
    float dx = mousePos.x - prevPos.x;
    float dy = mousePos.y - prevPos.y;
    float step;

    step = fabsf((fabsf(dx) >= fabsf(dy)) ? fabsf(dx) : fabsf(dy));

    dx /= step;
    dy /= step;

    float x = prevPos.x, y = prevPos.y;
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
            float distance = sqrtf(dX * dX + dY * dY);

            if (distance - radius > 1) continue;

            if (radius / distance < 0.9) continue;

            DrawPixel(sur, {(float)(x - radius) + origin.x, (float)(y - radius) + origin.y}, color);
        }
    }
}


void DrawPixel_CubicCurve(SDL_Surface* sur, Vector2 p2, Vector2 v2, Vector2 p1, Vector2 v1, int radius, Uint32 color) {
    if (p1 == p2) {
        DrawPixel_CircleBrush(sur, p1, radius, color);
        return;
    }
    // should return the angle between v2 and the displacement vector between p2 and p1
    // float theta = acosf((v2.x * (p2.x - p1.x) + v2.y * (p2.y - p1.y)) / (sqrtf(powf(v2.x, 2) + powf(v2.y, 2)) * sqrtf(powf(p2.x - p1.x, 2) + powf(p2.y - p1.y, 2))));

    // float x = v2.x, y = v2.y;
    // v2 = {x * -cosf(SDL_PI_F - theta) + y * sinf(SDL_PI_F - theta), x * sinf(SDL_PI_F - theta) - y * cosf(SDL_PI_F - theta)};

    SDL_Log("Theta: {%lf}, p1: {%lf, %lf}, v1: {%lf, %lf}, v2: {%lf, %lf}, p2: {%lf, %lf}", 0.00f, p1.x, p1.y, v1.x, v1.y, v2.x, v2.y, p2.x, p2.y);
    v2.flip();
    v1 = v1 * 0.25f;
    v2 = v2 * 0.25f;
    v2 = v2 + p2;
    v1 = v1 + p1;


    std::vector<Vector2> points;
    float mag = sqrtf(powf(p2.x - p1.x, 2) + powf(p2.y - p1.y, 2));
    int increment = 100;
    // if (mag >= 15.00f) {increment = 50;}
    // if (mag >= 30.00f) {increment = 50;}
    // if (mag >= 50.00f) {increment = 25;}
    // if (mag >= 100.00f) {increment = 10;}
    float t;
    for (int i = 0; i <= 100; i += increment) {
        t = i / 100.0f;
        points.push_back(p1 * powf(1 - t, 3) + v1 * t * (3 * powf(1 - t,2)) + v2 * (3 * (1 - t) * powf(t, 2)) + p2 * powf(t, 3));
    }

    for (int i = 0; i < points.size() - 1; ++i) {
        DrawPixel_Line(sur, points[i + 1], points[i], radius, color);
    }
    // DrawPixel_Line(sur, v2, p2, SDL_MapRGBA(SDL_GetPixelFormatDetails(sur->format), SDL_GetSurfacePalette(sur), 255, 0, 0, 255));
    // DrawPixel_Line(sur, v1, p1, SDL_MapRGBA(SDL_GetPixelFormatDetails(sur->format), SDL_GetSurfacePalette(sur), 0, 0, 255, 255));
}

#endif //APOLLO_DRAW_H
