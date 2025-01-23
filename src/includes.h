#ifndef APOLLO_INCLUDES_H
#define APOLLO_INCLUDES_H

// MASTER INCLUDES FILE
// Following order of C/C++ built-in libraries -> SDL -> format headers -> general headers -> program specific headers
// C libraries
#ifndef CMATH
#define CMATH
#include <cmath>
#endif

#ifndef CSTDIO
#define CSTDIO
#include <cstdio>
#endif

#ifndef CSTDLIB
#define CSTDLIB
#include <cstdlib>
#endif



// C++ containers
#ifndef VECTOR
#define VECTOR
#include <vector>
#endif

#ifndef STACK
#define STACK
#include <stack>
#endif

#ifndef STRING
#define STRING
#include <string>
#endif

#ifndef UTILITY
#define UTILITY
#include <utility>
#endif

#ifndef MEMORY
#define MEMORY
#include <memory>
#endif

// SDL
#ifndef SDL_H
#define SDL_H
#include "SDL3/SDL.h"
#endif SDL_H

#ifndef SDL_IMAGE_H
#define SDL_IMAGE_H
#include "SDL3_image/SDL_image.h"
#endif

#ifndef IMGUI_H
#define IMGUI_H
#include "imgui/imgui.h"
#endif

#ifndef IMGUI_IMPL_SDL3_H
#define IMGUI_IMPL_SDL3_H
#include "imgui/backends/imgui_impl_sdl3.h"
#endif

#ifndef IMGUI_IMPL_SDLRENDERER3_H
#define IMGUI_IMPL_SDLRENDERER3_H
#include "imgui/backends/imgui_impl_sdlrenderer3.h"
#endif

#include "qoi.h"

#ifdef _WIN32 // pen tablet fallback for Windows
#define EASYTAB_IMPLEMENTATION
#include "easytab/easytab.h"
#endif


#ifndef GENERAL_H
#define GENERAL_H
#include "general.h"
#endif

#ifndef FITCURVE_H
#define FITCURVE_H
#include "fitcurve.h"
#endif

#ifndef ONEEURO_H
#define ONEEURO_H
#include "oneeuro.h"
#endif

#ifndef ACTION_H
#define ACTION_H
#include "action.h"
#endif

#ifndef CANVAS_H
#define CANVAS_H
#include "canvas.h"
#endif

#ifndef INPUT_H
#define INPUT_H
#include "input.h"
#endif

#ifndef DRAW_H
#define DRAW_H
#include "draw.h"
#endif

#ifndef EFFECTS_H
#define EFFECTS_H
#include "effects.h"
#endif

#ifndef UI_H
#define UI_H
#include "ui.h"
#endif




#endif //APOLLO_INCLUDES_H
