#include <string>
#include "includes.h"

using namespace std;

const int width = 1280, height = 720;
const int menu = 20, sidebar = 250;
const Uint64 refreshDelay = (Uint64)1000 / 360;   // 360 is temporary, but marks the rate the program is refreshed.

int main() {
    SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Apollo", 1280 + sidebar, 720 + menu, SDL_WINDOW_RESIZABLE & SDL_WINDOW_SURFACE_VSYNC_ADAPTIVE & SDL_WINDOW_OPENGL);

    if(checkNull(window)) return 0;

    for (int i = 0; i < SDL_GetNumRenderDrivers(); ++i) {
        SDL_Log("%s", SDL_GetRenderDriver(i));
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, "opengl");
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

    // this will be the layer for any strokes in progress of being made. This will be cleared when a stroke/action is completed.
    SDL_Surface* workLayer = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    SDL_Texture* workLayerT = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);

    // a white surface for a background. can be customized in the future
    SDL_Surface* backg = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);

    Point2 canvasCenter = {width / 2, height / 2 + menu};
    auto* canvasCenterFP = new SDL_FPoint({(float)canvasCenter.x, (float)canvasCenter.y});

    auto* canvasR = (SDL_Rect*)SDL_malloc(sizeof(SDL_Rect));
    canvasR->x = 0;
    canvasR->y = 0;
    canvasR->w = width;
    canvasR->h = height;

    SDL_FillSurfaceRect(backg, canvasR, white);
    SDL_Texture* backgT = SDL_CreateTextureFromSurface(renderer, backg);
    SDL_Surface* tempLayer;
    SDL_Surface* tempWorkLayer;

    SDL_FillSurfaceRect(layer, canvasR, SDL_MapRGBA(SDL_GetPixelFormatDetails(layer->format), SDL_GetSurfacePalette(layer), 0, 0, 0, 0));
    SDL_FillSurfaceRect(workLayer, canvasR, SDL_MapRGBA(SDL_GetPixelFormatDetails(layer->format), SDL_GetSurfacePalette(layer), 0, 0, 0, 0));
    SDL_FillSurfaceRect(backg, canvasR, SDL_MapRGBA(SDL_GetPixelFormatDetails(backg->format), SDL_GetSurfacePalette(backg), 0, 0, 0, 255));

    bool isDrawing = false;
    unsigned int strokeSize = 5;

    bool escape = false, lctrl = false;
    int error;
    float mouseX, mouseY;

    float angle = 0.0f;
    float scale = 1.0f;

    // TODO: Optimize code (rectangle can be smaller for updating, use previous mouse position and stroke size...)
    auto* updateArea = (SDL_Rect*)malloc(sizeof(SDL_Rect));
    updateArea->w = width;
    updateArea->h = height;
    updateArea->x = 0;
    updateArea->y = 0;

    auto* canvasArea = (SDL_FRect*)malloc(sizeof(SDL_FRect));
    canvasArea->w = (float)width;
    canvasArea->h = (float)height;
    canvasArea->x = 0.00f;
    canvasArea->y = (float)menu;

    auto* updateWorkRect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
    updateWorkRect->w = width;
    updateWorkRect->h = height;
    updateWorkRect->x = 0;
    updateWorkRect->y = 0;

    Uint32 currentColor = SDL_MapRGBA(SDL_GetPixelFormatDetails(layer->format), SDL_GetSurfacePalette(layer), 0, 0, 0, 255);

    SDL_SetTextureScaleMode(backgT, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(layerT, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(workLayerT, SDL_SCALEMODE_NEAREST);

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
                        angle += 5.0;
                        break;
                    case SDLK_KP_4:
                        canvasArea->x -= 4;
                        canvasCenter.x -= 4;
                        canvasCenterFP->x -= 4;
                        break;
                    case SDLK_KP_6:
                        canvasArea->x += 4;
                        canvasCenter.x += 4;
                        canvasCenterFP->x += 4;
                        break;
                    case SDLK_B:
                        currentColor = SDL_MapRGBA(SDL_GetPixelFormatDetails(layer->format), SDL_GetSurfacePalette(layer), 0, 0, 0, 255);
                        break;
                    case SDLK_KP_MINUS:
                        angle -= 5.0;
                        SDL_Log("%lf, %lf", canvasCenterFP->x, canvasCenterFP->y);
                        break;
                    case SDLK_KP_MULTIPLY:
                        scale += 0.05f;
                        canvasArea->w = scale * width;
                        canvasArea->h = scale * height;
                        canvasArea->y = canvasCenterFP->y - canvasCenterFP->y * scale + menu * scale;
                        canvasArea->x = canvasCenterFP->x - canvasCenterFP->x * scale;
                        break;
                    case SDLK_KP_DIVIDE:
                        scale -= 0.05f;
                        canvasArea->w = scale * width;
                        canvasArea->h = scale * height;
                        canvasArea->y = canvasCenterFP->y - canvasCenterFP->y * scale + menu * scale;
                        canvasArea->x = canvasCenterFP->x - canvasCenterFP->x * scale;
                        SDL_Log("%lf %lf %lf %lf", canvasArea->w, canvasArea->h, canvasArea->x, canvasArea->y);
                        break;
                    case SDLK_E:
                        currentColor = SDL_MapRGBA(SDL_GetPixelFormatDetails(layer->format), SDL_GetSurfacePalette(layer), 0, 0, 0, 0);
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
                            SDL_UpdateTexture(layerT, updateArea, layer->pixels, layer->pitch);
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
                            SDL_UpdateTexture(layerT, updateArea, layer->pixels, layer->pitch);

                            qoisave->free();

                            delete qoisave;
                            RedoStack.pop();
                        }
                        break;
                    case SDLK_S:
                        if (lctrl) {
                            const char* path = SDL_GetBasePath();
                            const char* file = "test.png";
                            size_t length = strlen(path) + strlen(file) + 1;
                            char* result = (char*)malloc((length) * sizeof(char));

                            // Concatenate the two strings
                            int i;
                            for (i = 0; path[i] != '\0'; i++) {
                                result[i] = path[i];
                            }
                            for (int j = 0; file[j] != '\0'; j++) {
                                result[i++] = file[j];
                            }

                            result[i] = '\0';
                            IMG_SavePNG(layer, result);
                        }
                        break;
                }
            }

            // start drawing
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                switch (event.button.button) {
                    case SDL_BUTTON_LEFT:
                        if (!isDrawing && (mouseX >= 0 && mouseX < width && mouseY - menu >= 0 && mouseY - menu < height)) {
                            Point2 mouse = MapPoint(mouseX, mouseY, canvasCenterFP, angle, scale, SDL_FLIP_NONE);

                            points.push_back({mouse.x, mouse.y});

                            SDL_LockTextureToSurface(layerT, updateArea, &tempLayer);
                            SDL_LockTextureToSurface(workLayerT, updateArea, &tempWorkLayer);

                            DrawPixel_CircleBrush(workLayer, points[0], 5, currentColor);
                            updateWorkRect->x = mouse.x - (5 + 1);
                            updateWorkRect->y = mouse.y - (5 + 1);
                            updateWorkRect->w = (5 + 1) * 2;
                            updateWorkRect->h = (5 + 1) * 2;
                            SDL_BlitSurface(workLayer, updateWorkRect, tempWorkLayer, updateWorkRect);

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

            // continue drawing
            if (event.type == SDL_EVENT_MOUSE_MOTION) {
                if (isDrawing) {
                    mouseX = event.motion.x;
                    mouseY = event.motion.y;

                    Point2 mouse = MapPoint(mouseX, mouseY, canvasCenterFP, angle, scale, SDL_FLIP_NONE);

                    points.push_back({mouse.x, mouse.y});
                    DrawPixel_Line(workLayer, points[points.size() - 1], points[points.size() - 2], 5, currentColor);

                    updateWorkRect->x = min(points[points.size() - 1].x, points[points.size() - 2].x) - (5 + 1);
                    updateWorkRect->y = min(points[points.size() - 1].y, points[points.size() - 2].y) - (5 + 1);
                    updateWorkRect->w = abs(points[points.size() - 1].x - points[points.size() - 2].x) + ((5 + 1) * 2);
                    updateWorkRect->h = abs(points[points.size() - 1].y - points[points.size() - 2].y) + ((5 + 1) * 2);

                    SDL_BlitSurface(workLayer, updateWorkRect, tempWorkLayer, updateWorkRect);

                    if (points.size() > 500) {
                        auto* arr = (Point2*)malloc(sizeof(Point2) * points.size());
                        copy(points.begin(), points.end(), arr);
                        FitCurve(arr, points.size(), 2.0, layer);

                        free(arr);
                        points.clear();
                        points.push_back(mouse);
                    }
                }
            }

            // end drawing
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                switch (event.button.button) {
                    case SDL_BUTTON_LEFT:
                        if (isDrawing) {
                            isDrawing = false;
                            SDL_GetMouseState(&mouseX, &mouseY);

                            Point2 mouse = MapPoint(mouseX, mouseY, canvasCenterFP, angle, scale, SDL_FLIP_NONE);

                            points.push_back({mouse.x, mouse.y});

                            ClearPixels(workLayer);
                            ClearPixels(tempWorkLayer);

                            bool allPointsSame = true;
                            Point2 firstPoint = points[0];
                            for (Vector2 point: points) {
                                if (point != firstPoint) {
                                    allPointsSame = false;
                                    break;
                                }
                            }
                            if (allPointsSame) {
                                DrawPixel_CircleBrush(layer, firstPoint, strokeSize, currentColor);
                            } else {
                                auto *arr = (Point2 *) malloc(sizeof(Point2) * points.size());
                                copy(points.begin(), points.end(), arr);
                                FitCurve(arr, points.size(), 2.0, layer);

                                free(arr);
                            }

                            ClearPixels(tempLayer);
                            SDL_BlitSurface(layer, nullptr, tempLayer, nullptr);

                            SDL_UnlockTexture(workLayerT);
                            SDL_UnlockTexture(layerT);

                            points.clear();
                        }
                        break;
                }

            }
            else if (event.type == SDL_EVENT_KEY_UP) {
                if (event.key.key == SDLK_LCTRL) {
                    lctrl = false;
                }
            }
        }

        if (SDL_GetTicks() % 4 == 0  && SDL_GetWindowFlags(window) % 16 != 8) {
            Uint64 startunlock = SDL_GetTicksNS();
            if (isDrawing) SDL_UnlockTexture(workLayerT);
            Uint64 endunlock = SDL_GetTicksNS() - startunlock;
            SDL_Log("Unlocking took %f ms", static_cast<double>(endunlock) / 1000000.0);

            startunlock = SDL_GetTicksNS();
            error = SDL_RenderClear(renderer);
            if (checkError(error, "RenderClear()")) return -1;
            endunlock = SDL_GetTicksNS() - startunlock;
            SDL_Log("Clearing took %f ms", static_cast<double>(endunlock) / 1000000.0);

            startunlock = SDL_GetTicksNS();
            error = SDL_RenderTextureRotated(renderer, backgT, nullptr, canvasArea, angle, nullptr, SDL_FLIP_NONE);
            if (checkError(error, "RenderTextureRotated(backgT)")) return -1;
            endunlock = SDL_GetTicksNS() - startunlock;
            SDL_Log("Clearing took %f ms", static_cast<double>(endunlock) / 1000000.0);

            startunlock = SDL_GetTicksNS();
            error = SDL_RenderTextureRotated(renderer, workLayerT, nullptr, canvasArea, angle, nullptr, SDL_FLIP_NONE);
            if (checkError(error, "RenderTextureRotated(workLayerT)")) return -1;
            endunlock = SDL_GetTicksNS() - startunlock;
            SDL_Log("Rendering workLayer took %f ms", static_cast<double>(endunlock) / 1000000.0);

            startunlock = SDL_GetTicksNS();
            error = SDL_RenderTextureRotated(renderer, layerT, nullptr, canvasArea, angle, nullptr, SDL_FLIP_NONE);
            if (checkError(error, "RenderTextureRotated(layerT)")) return -1;
            endunlock = SDL_GetTicksNS() - startunlock;
            SDL_Log("Rendering layer took %f ms", static_cast<double>(endunlock) / 1000000.0);

            startunlock = SDL_GetTicksNS();
            error = SDL_RenderPresent(renderer);
            if (checkError(error, "RenderPresent()")) return -1;
            endunlock = SDL_GetTicksNS() - startunlock;
            SDL_Log("Presenting took %f ms", static_cast<double>(endunlock) / 1000000.0);

            startunlock = SDL_GetTicksNS();
            if (isDrawing) SDL_LockTextureToSurface(workLayerT, updateArea, &tempWorkLayer);
            endunlock = SDL_GetTicksNS() - startunlock;
            SDL_Log("Locking took %f ms", static_cast<double>(endunlock) / 1000000.0);

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
    delete updateWorkRect;

    SDL_DestroySurface(layer);
    SDL_DestroyTexture(layerT);

    SDL_DestroySurface(workLayer);
    SDL_DestroyTexture(workLayerT);

    SDL_DestroySurface(backg);
    SDL_DestroyTexture(backgT);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
