#pragma once

#include <SDL3/SDL.h>

#include <stdbool.h>
#include <stdint.h>

#include "g_data.h"

extern struct game_state {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event event;
    bool keys[SDL_SCANCODE_COUNT];
    uint16_t win_width;
    uint16_t win_height;
    double target_frame_time;
    double frame_time;
    double delta_time;
    enum {
        STATE_RUNNING,
        STATE_PAUSED,
        STATE_EXIT
    } state;
    struct inputs {
        vec2 x, y;
        vec2 mouse;
        vec2 mouse_delta;
    } input;
	struct {
		vec3 position;
		float rotation;
	} origin;
} state;

void game_init(int width, int height, int fps);

void g_frame_start(void);
void g_frame_end(void);
