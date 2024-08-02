#ifndef APOLLO_ACTION_H
#define APOLLO_ACTION_H

#ifndef SDL3
#define SDL3_H
#include "SDL3/SDL.h"
#endif

#ifndef SDL3_VIDEO
#define SDL3_VIDEO
#include "SDL3/SDL_video.h"
#endif

typedef enum ActionType {STROKE, FILL, CROP} ActionType;

typedef class Action {
public:
    ActionType type;
    SDL_Surface* surface;

    Action(ActionType actionType, SDL_Surface* sur) {
        type = actionType;
        surface = sur;
    }
} Action;

typedef class Notification {
public:
    const char* text;
    float time;

    Notification(const char* str, float t) {
        time = t;
        text = str;
    }
} Notification;


int checkNull(void* object) {
    if (object == nullptr) {
        SDL_Log("SDL has errored: %s", SDL_GetError());
        return 1;
    }
    return 0;
}

int checkError(int returnCode, unsigned int line) {
    if (returnCode) {
        SDL_Log("Line %d. SDL has errored: %s", line, SDL_GetError());
        return 1;
    }
    return 0;
}

#endif //APOLLO_ACTION_H
