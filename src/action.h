#ifndef APOLLO_ACTION_H
#define APOLLO_ACTION_H

#define SDL3_H

#ifndef SDL3_VIDEO_H
#define SDL3_VIDEO_H
#include "SDL3/SDL_video.h"
#endif

#include <utility>

using namespace std;

#include "qoi.h"

typedef enum ActionType {FRAME_EDIT, FRAME_DELETE, FRAME_LENGTH, FRAME_NEW,
                         LAYER_NEW, LAYER_DELETE, LAYER_OPACITY, LAYER_BLEND} ActionType;


// class for encoding a SDL_Surface into a QOI file in memory.
typedef class QOISave {
public:
    void* data;
    qoi_desc desc;
    int bytes;

    QOISave(void* pixels, qoi_desc description, int size) {
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
    qoi_desc getDesc() {
        return desc;
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

/**
 * Our frame class.
 * Contains the image, frame length, and deleted status.
 * The deleted status exists for undo/redo cycles.
 */
typedef struct Frame {
public:
    QOISave* image;
    bool deleted;
    unsigned length;

    Frame(QOISave* qoi, unsigned length = 1, bool deleted = false) {
        this->image = qoi;
        this->length = length;

        // we're likely never constructing a frame that's deleted on initialization, but just in case...
        this->deleted = deleted;
    }

    void setLength(unsigned length) {
        this->length = length;
    }

    unsigned getLength() {
        return this->length;
    }

} Frame;

/**
 * The layer class.
 * Contains the frames, blending mode, opacity, deleted status, etc...
 *
 */
typedef struct Layer {
    vector<Frame*> frames;
    SDL_BlendMode blendMode;
    Uint8 opacity;
    bool deleted;
    unsigned startPos;
    SDL_Texture* texture;

    Layer(vector<Frame*> layerData, unsigned startPos = 0, SDL_BlendMode blendMode = SDL_BLENDMODE_BLEND, Uint8 opacity = 0xFF, bool deleted = false) {
        this->frames = std::move(layerData);
        this->blendMode = blendMode;
        this->opacity = opacity;
        this->deleted = deleted;
        this->startPos = startPos;
        this->texture = nullptr;
    }

    Layer(Frame* frame, unsigned startPos = 0, SDL_BlendMode blendMode = SDL_BLENDMODE_BLEND, Uint8 opacity = 0xFF, bool deleted = false) {
        vector<Frame*> newFrames;
        newFrames.emplace_back(frame);

        this->frames = newFrames;
        this->blendMode = blendMode;
        this->opacity = opacity;
        this->deleted = deleted;
        this->startPos = startPos;
        this->texture = nullptr;
    }

    void addFrame(Frame* frame, int pos = -1) {
        if (pos < 0) {
            this->frames.push_back(frame);
            return;
        }
        frames.insert(frames.begin() + pos, frame);
    }

    int currentTimelineFrame(unsigned int timelinePos) {
        if (timelinePos < startPos) return -1;
        unsigned int length = startPos;
        for (int i = 0; i < frames.size(); ++i) {
            length += frames[i]->length;
            if (length > timelinePos) {
                return i;
            }
        }
        // could not find frame pos. likely out of range.
        return -1;
    }

    // does not completely delete the frame for sake of undo/redo
    // will not save in project save however
    void deleteFrame(int pos) {
        frames[pos]->deleted = true;
    }

    void undeleteFrame(int pos) {
        frames[pos]->deleted = false;
    }

    void deleteLayer() {
        this->deleted = true;
    }

    void undeleteLayer() {
        this->deleted = false;
    }

    void changeFrameLength(int pos, int newLength) {
        if (newLength < 1) return;
        frames[pos]->length = newLength;
    }

    void setOpacity(Uint8 opacity) {
        this->opacity = opacity;
    }

    void setOpacity(int opacity) {
        if (opacity > 255) this->opacity = 255;
        else if (opacity < 0) this->opacity = 0;
        else this->opacity = (Uint8)opacity;
    }

    // this is called when timeline position changes.
    void refreshTexture(SDL_Renderer* renderer, unsigned int timelinePos) {
        // if layer isn't even visible, texture is null
        if (this->texture != nullptr) SDL_DestroyTexture(this->texture);
        if (this->opacity == 0) {
            this->texture = nullptr;
        }

        int framePos = currentTimelineFrame(timelinePos);
        if (framePos >= 0) {
            Uint64 start = SDL_GetTicksNS();
            SDL_Surface* frameSur = IMG_LoadQOI_IO(SDL_IOFromMem(frames[framePos]->image->getData(), frames[framePos]->image->getBytes()));
            SDL_Log("Time taken to load surface: %lf", float(SDL_GetTicksNS() - start) / 1000000.0f);
            this->texture = SDL_CreateTextureFromSurface(renderer, frameSur);
            SDL_DestroySurface(frameSur);

        } else {
            this->texture = nullptr;
        }
    }

} Layer;


typedef class Action {

public:
    virtual void free() {

    }

protected:

    int data;
    ActionType action;

    Action(ActionType action, int data) {
        this->action = action;
        this->data = data;
    }

} Action;

typedef class LayerAction : public Action {
    Layer* layer;

    LayerAction(ActionType action, Layer* layer = nullptr, int data = 0) : Action(action, data) {
        if (layer == nullptr || static_cast<int>(action) < static_cast<int>(LAYER_NEW)) {
            SDL_LogError(-1, "Improperly stored layer information. Please exit the program.");
            return;
        }
        this->data = data;
        this->layer = layer;
        this->action = action;
    }

    void free() const {
        SDL_free(layer);
    }

} LayerAction;

typedef class FrameAction : public Action {
public:

    Frame* framePointer;

    virtual void free() const {

    }

protected:

    FrameAction(ActionType action, Frame* frame) : Action(action, 0) {

        if (static_cast<int>(action) >= static_cast<int>(LAYER_NEW) || frame == nullptr) {
            SDL_LogError(-1, "Improperly stored frame information. Please exit the program.");
        }

        SDL_Log("data saved successfully");
        this->action = action;
        SDL_Log("action saved successfully");
        this->framePointer = frame;
        // memcpy(this->framePointer, frame, sizeof(Frame));
        SDL_Log("frame pointer saved successfully");
    }

} FrameAction;

typedef struct FrameEditAction : FrameAction {
public:
    QOISave* save = (QOISave*)SDL_malloc(sizeof(QOISave));

    FrameEditAction(ActionType action, Frame* frame, SDL_Surface* sur = nullptr) : FrameAction(action, frame) {

        if (frame == nullptr || action != FRAME_EDIT || sur == nullptr) {
            SDL_LogError(-1, "Improperly saved frame information. Please exit the program.");
        }

        save = QOISaveFromSurface(sur);
        SDL_Log("save saved successfully");
    }

    void free() const override {
        SDL_free(save->data);
        SDL_free(save);
        FrameAction::free();
    }

} FrameEditAction;

int checkNull(void* object) {
    if (object == nullptr) {
        SDL_Log("SDL has errored: %s", SDL_GetError());
        return 1;
    }
    return 0;
}

int checkError(int returnCode, char* string) {
    if (!returnCode) {
        SDL_LogError(-1, "%s failed. SDL has errored: %s", string, SDL_GetError());
        return 1;
    }
    return 0;
}

#endif //APOLLO_ACTION_H
