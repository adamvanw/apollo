#include "includes.h"

using namespace std;

const int width = 1280, height = 720;
const int menu = 20, sidebar = 250;
const Uint64 refreshDelay = (Uint64)1000 / 360;   // 360 is temporary, but marks the rate the program is refreshed.

typedef enum KeyFlags {LCTRL, RCTRL, LALT, RALT, LSHIFT, RSHIFT, FUNCTION} KeyFlags;

int main() {
    SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);
    if (TTF_Init() == -1) {
        return -1;
    }

    TTF_Font* font = TTF_OpenFont("Acme9Regular.ttf", 8);
    if (font == nullptr) {
        SDL_LogError(-1, "Could not load font!");
    }

    SDL_Window* window = SDL_CreateWindow("Apollo", 1280 + sidebar, 720 + menu + 50, SDL_WINDOW_RESIZABLE & SDL_WINDOW_SURFACE_VSYNC_ADAPTIVE & SDL_WINDOW_OPENGL);

    if(checkNull(window)) return 0;

    for (int i = 0; i < SDL_GetNumRenderDrivers(); ++i) {
        SDL_Log("%s", SDL_GetRenderDriver(i));
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, "opengl");
    if(checkNull(renderer)) return -1;

    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    io.Fonts->AddFontDefault();

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    SDL_Color bgColor = {0x88, 0x88, 0x88, 0xFF};
    Uint32 white = 0xFFFFFFFF;
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_Event event;

    stack<Action*> UndoStack;
    stack<Action*> RedoStack;

    unsigned int currentLayerNum = 0;
    unsigned int currentFrameNum = 0;

    vector<Point2> points;

    SDL_Texture* currentLayerT = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);

    SDL_Surface* currentLayer = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);

    // this will be the layer for any strokes in progress of being made. This will be cleared when a stroke/action is completed.
    SDL_Surface* workLayer = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    SDL_Texture* workLayerT = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);

    // this will be the layer for a complete stroke to be made, then altered for opacity, to then be blitted onto the actual frame.
    // TODO: implement this please.
    SDL_Surface* finalizeLayer = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);

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

    SDL_Surface* newFrame = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    ClearPixels(newFrame);

    SDL_FillSurfaceRect(currentLayer, canvasR, SDL_MapRGBA(SDL_GetPixelFormatDetails(currentLayer->format), SDL_GetSurfacePalette(currentLayer), 0, 0, 0, 0));
    SDL_FillSurfaceRect(workLayer, canvasR, SDL_MapRGBA(SDL_GetPixelFormatDetails(currentLayer->format), SDL_GetSurfacePalette(currentLayer), 0, 0, 0, 0));
    SDL_FillSurfaceRect(backg, canvasR, SDL_MapRGBA(SDL_GetPixelFormatDetails(backg->format), SDL_GetSurfacePalette(backg), 0, 0, 0, 255));

    vector<Layer> layers;
    Frame frame = Frame(QOISaveFromSurface(currentLayer), 1, false);
    Layer layer = Layer(frame);
    layers.emplace_back(layer);

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

    Uint32 currentColor = SDL_MapRGBA(SDL_GetPixelFormatDetails(currentLayer->format), SDL_GetSurfacePalette(currentLayer), 0, 0, 0, 255);
    float colorFloats[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    SDL_SetTextureScaleMode(backgT, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(currentLayerT, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(workLayerT, SDL_SCALEMODE_NEAREST);

    auto* format = const_cast<SDL_PixelFormatDetails *>(SDL_GetPixelFormatDetails(workLayer->format));

    while(!escape) {
        Uint64 start = SDL_GetPerformanceCounter();

        SDL_GetMouseState(&mouseX, &mouseY);

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);

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
                        currentColor = SDL_MapRGBA(SDL_GetPixelFormatDetails(currentLayer->format), SDL_GetSurfacePalette(currentLayer), 0, 0, 0, 255);
                        break;
                    case SDLK_KP_MINUS:
                        angle -= 5.0;
                        SDL_Log("%lf, %lf", canvasCenterFP->x, canvasCenterFP->y);
                        break;
                    case SDLK_N:
                        SDL_Log("New frame created!");
                        SDL_Log("%p", &layers[currentLayerNum].frames[0]);
                        layers[currentLayerNum].addFrame(new Frame(QOISaveFromSurface(newFrame)));
                        SDL_Log("%p", &layers[currentLayerNum].frames[0]);
                        break;
                    case SDLK_RIGHT:
                        if (currentFrameNum < layers[currentLayerNum].frames.size() - 1) {
                            currentFrameNum++;

                            SDL_DestroySurface(currentLayer);
                            currentLayer = IMG_LoadQOI_IO(SDL_IOFromMem(layers[currentLayerNum].frames[currentFrameNum]->image->getData(), layers[currentLayerNum].frames[currentFrameNum]->image->getBytes()));
                            SDL_UpdateTexture(currentLayerT, updateArea, currentLayer->pixels, currentLayer->pitch);
                            SDL_Log("Entered frame %d.", currentFrameNum + 1);
                        }
                        break;
                    case SDLK_LEFT:
                        if (currentFrameNum > 0) {
                            currentFrameNum--;

                            SDL_DestroySurface(currentLayer);
                            currentLayer = IMG_LoadQOI_IO(SDL_IOFromMem(layers[currentLayerNum].frames[currentFrameNum]->image->getData(), layers[currentLayerNum].frames[currentFrameNum]->image->getBytes()));
                            SDL_UpdateTexture(currentLayerT, updateArea, currentLayer->pixels, currentLayer->pitch);
                            SDL_Log("Entered frame %d.", currentFrameNum + 1);
                        }
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
                        currentColor = SDL_MapRGBA(SDL_GetPixelFormatDetails(currentLayer->format), SDL_GetSurfacePalette(currentLayer), 0, 0, 0, 0);
                        break;
                    case SDLK_ESCAPE:
                        escape = true;
                        break;
                    case SDLK_Z:
                        // undo code
                        if (lctrl && !UndoStack.empty()) {
                            SDL_Log("Starting undo...");
                            Action* action = UndoStack.top();
                            auto* frameEdit = dynamic_cast<FrameEditAction*>(action);
                            SDL_Log("Casted frameEdit");

                            if (frameEdit) {
                                RedoStack.push(frameEdit);
                                swap(frameEdit->framePointer->image, frameEdit->save);

                                if (frameEdit->framePointer == layers[currentLayerNum].frames[currentFrameNum]) {
                                    SDL_DestroySurface(currentLayer);
                                    currentLayer = IMG_LoadQOI_IO(SDL_IOFromMem(layers[currentLayerNum].frames[currentFrameNum]->image->getData(), layers[currentLayerNum].frames[currentFrameNum]->image->getBytes()));
                                    SDL_UpdateTexture(currentLayerT, updateArea, currentLayer->pixels, currentLayer->pitch);
                                }
                            }

                            UndoStack.pop();
                        }
                        break;
                    case SDLK_Y:
                        // redo code
                        if (lctrl && !RedoStack.empty()) {
                            Action* action = RedoStack.top();
                            auto* frameEdit = dynamic_cast<FrameEditAction*>(action);

                            if (frameEdit) {
                                UndoStack.push(frameEdit);
                                swap(frameEdit->framePointer->image, frameEdit->save);

                                if (frameEdit->framePointer == layers[currentLayerNum].frames[currentFrameNum]) {
                                    SDL_DestroySurface(currentLayer);
                                    currentLayer = IMG_LoadQOI_IO(SDL_IOFromMem(layers[currentLayerNum].frames[currentFrameNum]->image->getData(), layers[currentLayerNum].frames[currentFrameNum]->image->getBytes()));
                                    SDL_UpdateTexture(currentLayerT, updateArea, currentLayer->pixels, currentLayer->pitch);
                                }
                            }

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
                            IMG_SavePNG(currentLayer, result);
                        }
                        break;
                }
            }

            // start drawing
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                switch (event.button.button) {
                    case SDL_BUTTON_LEFT:
                        if (!io.WantCaptureMouse && !isDrawing && (mouseX >= 0 && mouseX < width && mouseY - menu >= 0 && mouseY - menu < height)) {
                            Point2 mouse = MapPoint(mouseX, mouseY, canvasCenterFP, angle, scale, SDL_FLIP_NONE);

                            points.push_back({mouse.x, mouse.y});

                            SDL_LockTextureToSurface(currentLayerT, updateArea, &tempLayer);
                            SDL_LockTextureToSurface(workLayerT, updateArea, &tempWorkLayer);

                            DrawPixel_CircleBrush(workLayer, points[0], strokeSize, currentColor);
                            updateWorkRect->x = mouse.x - (strokeSize + 1);
                            updateWorkRect->y = mouse.y - (strokeSize + 1);
                            updateWorkRect->w = (strokeSize + 1) * 2;
                            updateWorkRect->h = (strokeSize + 1) * 2;
                            SDL_BlitSurface(workLayer, updateWorkRect, tempWorkLayer, updateWorkRect);

                            UndoStack.push(new FrameEditAction(FRAME_EDIT, layers[currentLayerNum].frames[currentFrameNum], currentLayer));

                            while (!RedoStack.empty()) {
                                Action* ptr = RedoStack.top();

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
                    DrawPixel_Line(workLayer, points[points.size() - 1], points[points.size() - 2], strokeSize, currentColor);

                    updateWorkRect->x = min(points[points.size() - 1].x, points[points.size() - 2].x) - (strokeSize + 1);
                    updateWorkRect->y = min(points[points.size() - 1].y, points[points.size() - 2].y) - (strokeSize + 1);
                    updateWorkRect->w = abs(points[points.size() - 1].x - points[points.size() - 2].x) + ((strokeSize + 1) * 2);
                    updateWorkRect->h = abs(points[points.size() - 1].y - points[points.size() - 2].y) + ((strokeSize + 1) * 2);

                    SDL_BlitSurface(workLayer, updateWorkRect, tempWorkLayer, updateWorkRect);

                    if (points.size() > 500) {
                        auto* arr = (Point2*)malloc(sizeof(Point2) * points.size());
                        copy(points.begin(), points.end(), arr);
                        FitCurve(arr, points.size(), 5.0, currentLayer, currentColor, strokeSize);

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
                                DrawPixel_CircleBrush(currentLayer, firstPoint, strokeSize, currentColor);
                            } else {
                                auto *arr = (Point2 *) malloc(sizeof(Point2) * points.size());
                                copy(points.begin(), points.end(), arr);
                                FitCurve(arr, points.size(), 5.0, currentLayer, currentColor, strokeSize);

                                free(arr);
                            }

                            ClearPixels(tempLayer);
                            SDL_BlitSurface(currentLayer, nullptr, tempLayer, nullptr);

                            SDL_UnlockTexture(workLayerT);
                            SDL_UnlockTexture(currentLayerT);

                            layers[currentLayerNum].frames[currentFrameNum]->image = QOISaveFromSurface(currentLayer);

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

        if (SDL_GetTicks() % 4 == 0 ) {

            if (isDrawing) SDL_UnlockTexture(workLayerT);

            ImGui_ImplSDLRenderer3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            {
                ImGui::Begin("Color Panel");

                ImGui::Text("Select Color");
                ImGui::ColorPicker4("Color", colorFloats);

                ImGui::SliderInt("Stroke", reinterpret_cast<int *>(&strokeSize), 1, 256);

                ImGui::End();
            }
            currentColor = SDL_MapRGBA(format, nullptr, colorFloats[0]*255, colorFloats[1]*255, colorFloats[2]*255, colorFloats[3]*255);

            ImGui::Render();

            error = SDL_RenderClear(renderer);
            if (checkError(error, "RenderClear()")) return -1;

            error = SDL_RenderTextureRotated(renderer, backgT, nullptr, canvasArea, angle, nullptr, SDL_FLIP_NONE);
            if (checkError(error, "RenderTextureRotated(backgT)")) return -1;

            error = SDL_RenderTextureRotated(renderer, currentLayerT, nullptr, canvasArea, angle, nullptr, SDL_FLIP_NONE);
            if (checkError(error, "RenderTextureRotated(layerT)")) return -1;

            error = SDL_RenderTextureRotated(renderer, workLayerT, nullptr, canvasArea, angle, nullptr, SDL_FLIP_NONE);
            if (checkError(error, "RenderTextureRotated(workLayerT)")) return -1;

            // very temporary frame indicator
            for (int i = 0; i < layers[0].frames.size(); ++i) {
                if (layers[0].frames[i]->deleted) continue;

                if (i == currentFrameNum) SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                else SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

                error = SDL_RenderRect(renderer, new SDL_FRect{(float)i * 25, menu + height + 25, 25, 25});
                if (checkError(error, "SDL_RenderRect")) return -1;
                SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
            }

            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

            error = SDL_RenderPresent(renderer);
            if (checkError(error, "RenderPresent()")) return -1;

            if (isDrawing) SDL_LockTextureToSurface(workLayerT, updateArea, &tempWorkLayer);

        }

        Uint64 frameTime = (SDL_GetPerformanceCounter() - start) / SDL_GetPerformanceFrequency() * 1000;

        if (frameTime > refreshDelay) {
            SDL_Log("Missed frame refresh. Time taken: %llu ms... Error: %s", frameTime, SDL_GetError());
            SDL_Delay(refreshDelay);
        } else {
            SDL_Delay(refreshDelay - frameTime);
        }

    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    while (!RedoStack.empty()) {
        Action *ptr = RedoStack.top();

        ptr->free();

        RedoStack.pop();
        delete ptr;
    }

    while (!UndoStack.empty()) {
        Action *ptr = UndoStack.top();

        ptr->free();

        UndoStack.pop();
        delete ptr;
    }

    delete canvasArea;
    delete updateArea;
    delete updateWorkRect;

    SDL_DestroySurface(currentLayer);
    SDL_DestroyTexture(currentLayerT);

    SDL_DestroySurface(workLayer);
    SDL_DestroyTexture(workLayerT);

    SDL_DestroySurface(backg);
    SDL_DestroyTexture(backgT);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
