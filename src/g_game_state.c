#include "g_game_state.h"
#include <SDL3/SDL_video.h>


struct game_state state;

void game_init(int width, int height, int fps) {
    state.win_width = width;
    state.win_height = height;
    state.target_frame_time = 1000.0 / fps;

    SDL_CreateWindowAndRenderer("by gooficat", width, height, 0, &state.window, &state.renderer);
}

void g_frame_start(void) {
    state.frame_time = SDL_GetTicks();
	
	vec3 dir = {0};
	
	
	if (state.keys[SDL_SCANCODE_W])
		dir.y++;
	if (state.keys[SDL_SCANCODE_S])
		dir.y--;
	if (state.keys[SDL_SCANCODE_A])
		dir.x--;
	if (state.keys[SDL_SCANCODE_D])
		dir.x++;
	if (state.keys[SDL_SCANCODE_LSHIFT])
		dir.z++;
	if (state.keys[SDL_SCANCODE_SPACE])
		dir.z--;
	if (state.keys[SDL_SCANCODE_LEFT])
		state.origin.rotation -= 0.002f * state.delta_time;
	if (state.keys[SDL_SCANCODE_RIGHT])
		state.origin.rotation += 0.002f * state.delta_time;
	
	state.origin.sin_rotation = sinf(state.origin.rotation);
	state.origin.cos_rotation = cosf(state.origin.rotation);
	
	
	
	state.origin.position.x += (dir.x * state.origin.cos_rotation + dir.y * state.origin.sin_rotation) * 0.2f * state.delta_time;
	state.origin.position.y += (dir.y * state.origin.cos_rotation - dir.x * state.origin.sin_rotation) * 0.2f * state.delta_time;
	state.origin.position.z += dir.z * 0.2f * state.delta_time;
}
void g_frame_end(void) {
    state.delta_time = SDL_GetTicks() - state.frame_time;
    if (state.delta_time < state.target_frame_time) {
        SDL_Delay(state.target_frame_time - state.delta_time);
        state.delta_time = state.target_frame_time;
    }
    SDL_UpdateWindowSurface(state.window);
}
