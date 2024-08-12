#include "general.h"
#include "action.h"
#include "draw.h"
#include <stack>
#include <queue>

using namespace std;

#ifndef SDL_RENDER_H
#define SDL_RENDER_H
#include "SDL3/SDL_render.h"
#endif


#ifndef SDL_TTF
#define SDL_TTF
#include "SDL3_ttf/SDL_ttf.h"
#endif

#ifndef SDL_IMAGE
#define SDL_IMAGE
#include "SDL3_image/SDL_image.h"
#endif


#ifndef QOI
#define QOI
#include "qoi.h"
#endif

const int width = 1280, height = 720;
const int menu = 20, sidebar = 250;
const Uint64 refreshDelay = (Uint64)1000 / 360;   // 360 is temporary, but marks the rate the program is refreshed.

int main() {
    SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Apollo", width + sidebar, height + menu, SDL_WINDOW_RESIZABLE);

    if(checkNull(window)) return 0;

    for (int i = 0; i < SDL_GetNumRenderDrivers(); ++i) {
        SDL_Log("%s", SDL_GetRenderDriver(i));
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, "direct3d12");
    if(checkNull(renderer)) return -1;

    SDL_Color bgColor = {0x88, 0x88, 0x88, 0xFF};
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_Event event;

    stack<QOISave*> UndoStack;
    stack<QOISave*> RedoStack;

    vector<Vector2> points;
    vector<Vector2> displacements;
    vector<Vector2> velocities;

    SDL_Texture* layerT = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);

    SDL_Surface* layer = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    SDL_Surface* backg = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);

    auto* canvasR = (SDL_Rect*)SDL_malloc(sizeof(SDL_Rect));
    canvasR->x = 0;
    canvasR->y = 0;
    canvasR->w = width;
    canvasR->h = height;


    SDL_FillSurfaceRect(layer, canvasR, SDL_MapRGBA(SDL_GetPixelFormatDetails(layer->format), SDL_GetSurfacePalette(layer), 0, 0, 0, 0));
    SDL_FillSurfaceRect(backg, canvasR, SDL_MapRGBA(SDL_GetPixelFormatDetails(backg->format), SDL_GetSurfacePalette(backg), 0, 0, 0, 255));

    bool isDrawing = false;
    unsigned int strokeSize = 5;

    Notification currentNotif("Starting up...", 1);
    bool escape = false, lctrl = false;
    int error;
    float mouseX, mouseY;

    // TODO: Optimize code (rectangle can be smaller for updating, use previous mouse position...)
    auto* updateArea = (SDL_Rect*)malloc(sizeof(SDL_Rect));
    updateArea->w = width;
    updateArea->h = height;
    updateArea->x = 0;
    updateArea->y = menu;

    auto* canvasArea = (SDL_FRect*)malloc(sizeof(SDL_FRect));
    canvasArea->w = (float)width;
    canvasArea->h = (float)height;
    canvasArea->x = 0.00f;
    canvasArea->y = (float)menu;

    Uint32 currentColor = SDL_MapRGBA(SDL_GetPixelFormatDetails(layer->format), SDL_GetSurfacePalette(layer), 0, 0, 0, 255);

    while(!escape) {
        Uint64 start = SDL_GetPerformanceCounter();

        // points start at p0, p1, p2, p3...
        SDL_GetMouseState(&mouseX, &mouseY);
        points.push_back({mouseX, mouseY});

        if (points.size() >= 2) {
            displacements.push_back(points[points.size() - 1] - points[points.size() - 2]);
        }
        if (points.size() >= 3) {
            velocities.push_back((displacements[displacements.size() - 1] + displacements[displacements.size() - 2]) * 0.5f);
        }

        if (points.size() > 4) points.erase(points.begin());
        if (displacements.size() > 3) displacements.erase(displacements.begin());
        if (velocities.size() > 2) velocities.erase(velocities.begin());


        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_KEY_DOWN) {
                switch (event.key.key) {
                    case SDLK_LCTRL:
                        lctrl = true;
                        break;
                    case SDLK_KP_PLUS:
                        strokeSize += 4;
                        currentNotif = Notification("Stroke increased", 1.5);
                        break;
                    case SDLK_B:
                        currentColor = SDL_MapRGBA(SDL_GetPixelFormatDetails(layer->format), SDL_GetSurfacePalette(layer), 0, 0, 0, 255);
                        currentNotif = Notification("Brush tool selected...", 1.5);
                        break;
                    case SDLK_KP_MINUS:
                        strokeSize = (strokeSize < 4) ? 1 : strokeSize - 4;
                        currentNotif = Notification("Stroke decreased...", 1.5);
                        break;
                    case SDLK_E:
                        currentColor = SDL_MapRGBA(SDL_GetPixelFormatDetails(layer->format), SDL_GetSurfacePalette(layer), 0, 0, 0, 0);
                        currentNotif = Notification("Eraser tool selected...", 1.5);
                        break;
                    case SDLK_ESCAPE:
                        escape = true;
                        break;
                    case SDLK_Z:
                        // undo code
                        if (lctrl && !UndoStack.empty()) {
                            QOISave* qoisave = UndoStack.top();

                            RedoStack.push(QOISaveFromSurface(layer));
                            SDL_DestroySurface(layer);
                            layer = IMG_LoadQOI_IO(SDL_IOFromMem(qoisave->getData(), qoisave->getBytes()));
                            SDL_Log("Bytes: %d", qoisave->getBytes());

                            qoisave->free();

                            delete qoisave;
                            UndoStack.pop();
                        }
                        break;
                    case SDLK_Y:
                        // redo code
                        if (lctrl && !RedoStack.empty()) {
                            QOISave* qoisave = RedoStack.top();

                            UndoStack.push(QOISaveFromSurface(layer));
                            SDL_DestroySurface(layer);
                            layer = IMG_LoadQOI_IO(SDL_IOFromMem(qoisave->getData(), qoisave->getBytes()));

                            qoisave->free();

                            delete qoisave;
                            RedoStack.pop();
                        }
                        break;
                }
            }
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                switch (event.button.button) {
                    case SDL_BUTTON_LEFT:
                        if (!isDrawing && (mouseX >= 0 && mouseX < 1280 && mouseY - 3*menu >= 0 && mouseY - 3*menu < 720)) {
                            SDL_Log("Detected mouse!");

                            UndoStack.push(QOISaveFromSurface(layer));
                            while (!RedoStack.empty()) {
                                QOISave *ptr = RedoStack.top();
                                
                                ptr->free();

                                RedoStack.pop();
                                delete ptr;
                            }

                            isDrawing = true;

                            DrawPixel_CircleBrush(layer, {mouseX, mouseY - 3 * menu}, strokeSize, currentColor);
                        }

                        // drawBrushStroke()...

                        break;
                }

            }
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                SDL_GetMouseState(&mouseX, &mouseY);
                if (points.size() >= 4 && velocities.size() >= 2 && isDrawing) DrawPixel_CubicCurve(layer, {points[points.size() - 2].x, points[points.size() - 2].y - 3 * menu}, velocities[1], {points[points.size() - 3].x, points[points.size() - 3].y - 3 * menu}, velocities[0], strokeSize, currentColor);
                isDrawing = false;
            }
            else if (event.type == SDL_EVENT_KEY_UP) {
                if (event.key.key == SDLK_LCTRL) {
                    lctrl = false;
                }
            }
        }

        if (isDrawing) {
            if (points.size() >= 4 && velocities.size() >= 2) DrawPixel_CubicCurve(layer, {points[points.size() - 2].x, points[points.size() - 2].y - 3 * menu}, velocities[1], {points[points.size() - 3].x, points[points.size() - 3].y - 3 * menu}, velocities[0], strokeSize, currentColor);
        }

        if (currentNotif.time >= 10) {
            unsigned char opacity = (currentNotif.time > 1) ? 255 : (currentNotif.time < 0) ? 0 : currentNotif.time * 255;
            // DrawText replacement: DrawText(currentNotif.text, width, -15, 15, {0, 0, 0, opacity});
            // GetFrameTime() replacement: currentNotification.time -= GetFrameTime();
        }

        error = SDL_UpdateTexture(layerT, updateArea, layer->pixels, layer->pitch);
        if (checkError(error, 177)) return -1;

        error = SDL_RenderClear(renderer);
        if (checkError(error, 180)) return -1;

        error = SDL_SetRenderViewport(renderer, updateArea);
        if (checkError(error, 183)) return -1;

        error = SDL_RenderTexture(renderer, layerT, nullptr, canvasArea);
        if (checkError(error, 186)) return -1;

        error = SDL_RenderPresent(renderer);
        if (checkError(error, 189)) return -1;

        Uint64 frameTime = (SDL_GetPerformanceCounter() - start) / SDL_GetPerformanceFrequency() * 1000;

        if (frameTime > refreshDelay) {
            SDL_Log("Missed frame refresh. Time taken: %llu ms... Error: %s", frameTime, SDL_GetError());
            SDL_Delay(refreshDelay);
        } else {
            SDL_Delay(refreshDelay - frameTime);
        }

    }

    delete canvasArea;
    delete updateArea;

    SDL_DestroySurface(layer);
    SDL_DestroyTexture(layerT);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
