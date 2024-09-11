#ifndef APOLLO_INCLUDES_H
#define APOLLO_INCLUDES_H

// MASTER INCLUDES FILE
// Following order of C/C++ built-in libraries -> SDL -> format headers -> general headers -> program specific headers
// C libraries
#include <cmath>
#include <cstdio>
#include <cstdlib>

// C++ containers
#include <vector>
#include <stack>
#include <string>
#include <utility>
#include <memory>

// SDL
#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"
#include "SDL3_ttf/SDL_ttf.h"

#include "qoi.h"

#include "general.h"
#include "fitcurve.h"
#include "oneeuro.h"

#include "action.h"
#include "general.h"
#include "draw.h"



#endif //APOLLO_INCLUDES_H
