#pragma once

#include <SDL3/SDL.h>
#include <stdlib.h>
#include <math.h>
#include <SDL3_image/SDL_image.h>

#include "g_game_state.h"

#include "utils.h"

#define FOCAL_LENGTH 200.0f

void renderer_init(SDL_Renderer* renderer);

void renderer_render(SDL_Renderer* renderer);

void renderer_destroy(void);


void draw_sector(int i,
                 int px1, int px2, int pb1, int pb2, int pt1, int pt2);
