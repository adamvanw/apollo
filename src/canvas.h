#ifndef APOLLO_CANVAS_H
#define APOLLO_CANVAS_H

#include "SDL3/SDL.h"
#include "general.h"
#include "action.h"

void canvasScale(float *scale, SDL_FRect *canvasArea, SDL_FPoint *canvasCenterFP, const int menu, Vector2 resolution, float d) {
    *scale += d;
    canvasArea->w = *scale * float(resolution.x);
    canvasArea->h = *scale * float(resolution.y);
    canvasArea->y = canvasCenterFP->y - canvasCenterFP->y * *scale + menu * *scale;
    canvasArea->x = canvasCenterFP->x - canvasCenterFP->x * *scale;
}

void stackSwap(bool redo, bool lctrl, stack<Action*> *pullStack, stack<Action*> *pushStack, vector<Layer> *layers, SDL_Surface** currentLayer,
               SDL_Texture* currentLayerT, SDL_Rect* updateArea, unsigned int currentLayerNum, unsigned int* currentFrameNum,
               unsigned int currentTimelineNum, SDL_Renderer* renderer) {
    if (lctrl && !pullStack->empty()) {
        Action* action = pullStack->top();
        auto* frameEdit = dynamic_cast<FrameEditAction*>(action);
        auto* frameAction = dynamic_cast<FrameAction*>(action);

        if (frameEdit) {
            pushStack->push(frameEdit);
            swapQOISaveWithSurface(&frameEdit->save, &frameEdit->framePointer->image);

            if (frameEdit->framePointer == layers->at(currentLayerNum).frames[*currentFrameNum]) {
                SDL_DestroySurface(*currentLayer);
                *currentLayer = SDL_DuplicateSurface(layers->at(currentLayerNum).frames[*currentFrameNum]->image);
                SDL_UpdateTexture(currentLayerT, updateArea, (*currentLayer)->pixels, (*currentLayer)->pitch);
            }
            SDL_Log("%s: Edit Frame.", ((redo) ? "Redo" : "Undo"));
        } else if (frameAction) {
            pushStack->push(frameAction);
            if (frameAction->getActionType() == FRAME_DELETE) {
                frameAction->framePointer->deleted = redo;
                SDL_Log("%s: Delete Frame.", ((redo) ? "Redo" : "Undo"));
            }
        } else {
            pushStack->push(action);
        }

        pullStack->pop();
        *currentFrameNum = layers->at(currentLayerNum).currentTimelineFrame(currentTimelineNum);
        refreshAllTextures(layers, renderer, currentTimelineNum);
    }
}

#endif //APOLLO_CANVAS_H
