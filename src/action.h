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

#ifndef QOI
#define QOI
#include "qoi.h"
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

// class for encoding a SDL_Surface into a QOI file in memory.
typedef class QOISave {
public:
    void* data;
    qoi_desc desc;
    int bytes;
public:
    explicit QOISave(void* pixels, qoi_desc description, int size) {
        data = pixels;
        desc = description;
        bytes = size;
    }
    void* getData() {
        return data;
    }
    int getBytes() {
        return bytes;
    }
    void free() {
        SDL_free(data);
    }
} QOISave;

QOISave* QOISaveFromSurface(SDL_Surface* sur) {
    auto* ptr = (QOISave*)malloc(sizeof(QOISave));
    ptr->desc = {(unsigned)sur->w, (unsigned)sur->h, 4, 0};
    void* tempPixels = qoi_encode(sur->pixels, &ptr->desc, &ptr->bytes);

    ptr->data = malloc(ptr->bytes);
    memcpy(ptr->data, tempPixels, ptr->bytes);
    SDL_free(tempPixels);

    if (ptr->data == nullptr) SDL_Log("QOI Encoding has failed.");
    return ptr;
}

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
