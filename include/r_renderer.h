#pragma once

#include <SDL3/SDL.h>
#include <stdlib.h>
#include <math.h>

#include "g_game_state.h"

void renderer_init(SDL_Renderer* renderer);

void renderer_render(SDL_Renderer* renderer);

// void renderer_destroy();
