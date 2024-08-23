#include "includes.h"

using namespace std;

const int width = 1280, height = 720;
const int menu = 20, sidebar = 250;
const Uint64 refreshDelay = (Uint64)1000 / 360;   // 360 is temporary, but marks the rate the program is refreshed.

int main() {
    SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Apollo", width + sidebar, height + menu, SDL_WINDOW_RESIZABLE & SDL_WINDOW_SURFACE_VSYNC_ADAPTIVE);

    if(checkNull(window)) return 0;

    for (int i = 0; i < SDL_GetNumRenderDrivers(); ++i) {
        SDL_Log("%s", SDL_GetRenderDriver(i));
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, "direct3d12");
    if(checkNull(renderer)) return -1;

    SDL_Color bgColor = {0x88, 0x88, 0x88, 0xFF};
    Uint32 white = 0xFFFFFFFF;
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_Event event;

    stack<QOISave*> UndoStack;
    stack<QOISave*> RedoStack;

    vector<Point2> points;

    SDL_Texture* layerT = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);

    SDL_Surface* layer = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    SDL_Surface* backg = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);


    auto* canvasR = (SDL_Rect*)SDL_malloc(sizeof(SDL_Rect));
    canvasR->x = 0;
    canvasR->y = 0;
    canvasR->w = width;
    canvasR->h = height;

    SDL_FillSurfaceRect(backg, canvasR, white);
    SDL_Texture* backgT = SDL_CreateTextureFromSurface(renderer, backg);

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

        SDL_GetMouseState(&mouseX, &mouseY);

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
                            points.push_back({mouseX, mouseY - 3*menu});

                            UndoStack.push(QOISaveFromSurface(layer));
                            while (!RedoStack.empty()) {
                                QOISave *ptr = RedoStack.top();
                                
                                ptr->free();

                                RedoStack.pop();
                                delete ptr;
                            }
                            isDrawing = true;
                        }
                        break;
                }

            }
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                isDrawing = false;
                SDL_GetMouseState(&mouseX, &mouseY);
                points.push_back({mouseX, mouseY - 3*menu});
                for (Point2 point : points) {
                    DrawPixel_CircleBrush(layer, point, 5, currentColor);
                }
                auto* arr = (Point2*)malloc(sizeof(Point2) * points.size());
                copy(points.begin(), points.end(), arr);
                FitCurve(arr, points.size(), 1.0, layer);

                free(arr);
                points.clear();
            }
            else if (event.type == SDL_EVENT_KEY_UP) {
                if (event.key.key == SDLK_LCTRL) {
                    lctrl = false;
                }
            }
        }

        if (isDrawing) {
            points.push_back({mouseX, mouseY - 3*menu});
            if (points.size() > 500) {
                SDL_GetMouseState(&mouseX, &mouseY);
                for (Point2 point : points) {
                    DrawPixel_CircleBrush(layer, point, 5, currentColor);
                }
                auto* arr = (Point2*)malloc(sizeof(Point2) * points.size());
                copy(points.begin(), points.end(), arr);
                FitCurve(arr, points.size(), 1.0, layer);

                free(arr);
                points.clear();
                points.push_back({mouseX, mouseY - 3*menu});
            }
        }

        if (currentNotif.time >= 10) {
            // unsigned char opacity = (currentNotif.time > 1) ? 255 : (currentNotif.time < 0) ? 0 : currentNotif.time * 255;
            // DrawText replacement: DrawText(currentNotif.text, width, -15, 15, {0, 0, 0, opacity});
            // GetFrameTime() replacement: currentNotification.time -= GetFrameTime();
        }

        if (SDL_GetTicks() % 4 == 0) {
            error = SDL_UpdateTexture(layerT, updateArea, layer->pixels, layer->pitch);
            if (checkError(error, "UpdateTexture(layerT)")) return -1;

            error = SDL_RenderClear(renderer);
            if (checkError(error, "RenderClear()")) return -1;

            error = SDL_SetRenderViewport(renderer, updateArea);
            if (checkError(error, "SetRenderViewport()")) return -1;

            error = SDL_RenderTexture(renderer, backgT, nullptr, canvasArea);
            if (checkError(error, "RenderTexture(backgT)")) return -1;

            error = SDL_RenderTexture(renderer, layerT, nullptr, canvasArea);
            if (checkError(error, "RenderTexture(layerT)")) return -1;

            error = SDL_RenderPresent(renderer);
            if (checkError(error, "RenderPresent()")) return -1;
        }

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

    SDL_DestroySurface(backg);
    SDL_DestroyTexture(backgT);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
