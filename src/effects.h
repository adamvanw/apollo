#ifndef EFFECTS_H
#define EFFECTS_H

/**
 * Similar to TVPaint's Grain filter, aka Frosted Glass,
 * this effect is given a probability to move a pixel over a given distance
 *
 * This isn't probably
 */
void Grain(SDL_Surface* sur, unsigned shift, float intensity) {
    SDL_Surface* newSur = SDL_DuplicateSurface(sur);
    ClearPixels(newSur);
    srand(SDL_GetTicks());
    Uint32 *ptr, *newPtr;
    float angle;
    for (int j = 0; j < sur->h; j++) {
        for (int i = 0; i < sur->w; i++) {
            if (rand() % 1000 < intensity * 1000) {
                Vector2 randVec = {(double)shift, 0};
                angle = ((float)rand() / RAND_MAX) * 2 * SDL_PI_F;
                randVec = {randVec.x * SDL_cosf(angle) - randVec.y * SDL_sinf(angle), randVec.x * SDL_sinf(angle) + randVec.y * SDL_cosf(angle)};
                randVec.x += i;
                randVec.y += j;
                if (randVec.x < 0) randVec.x = 0;
                else if (int(randVec.x) >= sur->w) randVec.x = sur->w - 1;
                if (randVec.y < 0) randVec.y = 0;
                else if (int(randVec.y) >= sur->h) randVec.y = sur->h - 1;

                ptr = (Uint32*)sur->pixels + int(randVec.x) % sur->w + int(randVec.y * sur->w) % sur->h;
                newPtr = (Uint32*)newSur->pixels + i + j * sur->w;
                *newPtr = *ptr;
            }
        }
    }

    SDL_DestroySurface(sur);
    sur = SDL_DuplicateSurface(newSur);
    SDL_DestroySurface(newSur);
}

#endif //EFFECTS_H
