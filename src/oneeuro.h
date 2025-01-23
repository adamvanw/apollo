#ifndef APOLLO_ONEEURO_H
#define APOLLO_ONEEURO_H

#ifndef GENERAL_H
#define GENERAL_H
#include "general.h"
#endif

#ifndef SDL3_H
#define SDL3_H
#include "SDL3/SDL.h"
#endif

typedef struct {
    double frequency;
    double mincutoff;
    double beta;
    double dcutoff;
} OneEuroFilterConfig;

typedef struct {
    double x;
    double dx;
    double lasttime;
} OneEuroFilterState;

OneEuroFilterState createOneEuroFilterState() {
    OneEuroFilterState state = {0.0, 0.0, 0.0};
    return state;
}

double smoothing_factor(double t, double cutoff) {
    double r = 2.0 * SDL_PI_D * cutoff * t;
    return r / (r + 1.0);
}

double exponential_smoothing(double a, double x, double prev_x) {
    return a * x + (1.0 - a) * prev_x;
}

Vector2 update_one_euro_filter(Vector2 input, OneEuroFilterState* xState, OneEuroFilterState* yState, OneEuroFilterConfig* config, double timestamp) {
    Vector2 output;
    double dt = timestamp - xState->lasttime;

    if (xState->lasttime == 0) {
        output.x = input.x;
        output.y = input.y;
        xState->lasttime = timestamp;
        yState->lasttime = timestamp;
    } else if (dt > 0) {
        double dx = (input.x - xState->x) / dt;
        double dy = (input.y - yState->x) / dt;

        double edx = exponential_smoothing(smoothing_factor(dt, config->dcutoff), dx, xState->dx);
        double edy = exponential_smoothing(smoothing_factor(dt, config->dcutoff), dy, yState->dx);

        double cutoff = config->mincutoff + config->beta * fabs(edx);
        output.x = exponential_smoothing(smoothing_factor(dt, cutoff), input.x, xState->x);

        cutoff = config->mincutoff + config->beta * fabs(edy);
        output.y = exponential_smoothing(smoothing_factor(dt, cutoff), input.y, yState->x);

        xState->x = output.x;
        xState->dx = edx;
        xState->lasttime = timestamp;

        yState->x = output.y;
        yState->dx = edy;
        yState->lasttime = timestamp;
    } else {
        output.x = xState->x;
        output.y = yState->x;
    }

    return output;
}

#endif //APOLLO_ONEEURO_H
