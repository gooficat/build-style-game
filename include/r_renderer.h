#pragma once

#include <SDL3/SDL.h>
#include <stdlib.h>
#include <math.h>
#include <SDL3_image/SDL_image.h>

#include "g_game_state.h"

#define FOCAL_LENGTH 200.0f

void renderer_init(SDL_Renderer* renderer);

void renderer_render(SDL_Renderer* renderer);

void renderer_destroy(void);
