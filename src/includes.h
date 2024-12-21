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
#include "SDL3/SDL_vulkan.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl3.h"
#include "imgui/backends/imgui_impl_sdlrenderer3.h"

#include "qoi.h"

#ifdef _WIN32 // pen tablet fallback for Windows
#define EASYTAB_IMPLEMENTATION
#include "easytab/easytab.h"
#endif

#include "general.h"
#include "fitcurve.h"
#include "oneeuro.h"

#include "action.h"
#include "general.h"
#include "draw.h"
#include "effects.h"
#include "ui.h"



#endif //APOLLO_INCLUDES_H
