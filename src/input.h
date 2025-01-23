#ifndef APOLLO_INPUT_H
#define APOLLO_INPUT_H

#ifndef UNORDERED_MAP
#define UNORDERED_MAP
#include <unordered_map>
#endif

#ifndef SDL_H
#define SDL_H
#include "SDL3/SDL.h"
#endif

typedef enum Input {
    ACTION_UNDO,
    ACTION_REDO,
    CANVAS_ROTATE_CLOCKWISE,
    CANVAS_ROTATE_COUNTER_CLOCKWISE,
    CANVAS_ZOOM_IN,
    CANVAS_ZOOM_OUT,
    PROJECT_FRAME_NEW,
    PROJECT_FRAME_DELETE,
    PROJECT_FRAME_LENGTH_INCREASE,
    PROJECT_LAYER_NEW
} Input;

typedef struct InputSet {
    unordered_map<Uint32, Input> inputs;

    InputSet() {
        inputs.insert(std::pair{(SDL_KMOD_CTRL + 65536) + SDLK_Z, ACTION_UNDO});
        inputs.insert(std::pair{(SDL_KMOD_CTRL + 65536) + SDLK_Y, ACTION_REDO});
        inputs.insert(std::pair{SDLK_M, PROJECT_LAYER_NEW});
        inputs.insert(std::pair{SDLK_N, PROJECT_FRAME_NEW});
        inputs.insert(std::pair{SDLK_DELETE, PROJECT_FRAME_DELETE});
        inputs.insert(std::pair{SDLK_J, PROJECT_FRAME_LENGTH_INCREASE});
        inputs.insert(std::pair{SDLK_PLUS, CANVAS_ZOOM_IN});
        inputs.insert(std::pair{SDLK_MINUS, CANVAS_ZOOM_OUT});
        inputs.insert(std::pair{SDLK_LEFTBRACKET, CANVAS_ROTATE_COUNTER_CLOCKWISE});
        inputs.insert(std::pair{SDLK_RIGHTBRACKET, CANVAS_ROTATE_CLOCKWISE});
    }
} InputSet;

void checkInputs(InputSet* inputSet, vector<void*> data) {
    SDL_Event event;
    SDL_Keymod keyMod = SDL_GetModState();
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);

        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
            // TODO: Request user to save work if unsaved, will be implemented once saving is actually implemented.
            break;
        }

        if (event.type == SDL_EVENT_KEY_DOWN) {
            unordered_map<Uint32, Input>::iterator it;
            if ((it = inputSet->inputs.find(event.key.key + (keyMod + 65536))) != inputSet->inputs.end()) {
                switch (it->second) {
                    case ACTION_UNDO:

                        break;
                    case ACTION_REDO:

                        break;
                    case CANVAS_ROTATE_CLOCKWISE:

                        break;
                    case CANVAS_ROTATE_COUNTER_CLOCKWISE:

                        break;
                    case PROJECT_FRAME_DELETE:

                        break;
                    case PROJECT_FRAME_LENGTH_INCREASE:

                        break;
                    case PROJECT_FRAME_NEW:

                        break;
                    case PROJECT_LAYER_NEW:

                        break;
                    case CANVAS_ZOOM_IN:

                        break;
                    case CANVAS_ZOOM_OUT:

                        break;
                }
            }
        }
    }
}

#endif
