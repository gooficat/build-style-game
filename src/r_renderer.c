#include "r_renderer.h"
#include <SDL3/SDL_render.h>

SDL_Texture* buffer_texture;
uint16_t* buffer;

void pixel(int x, int y, uint16_t color) {
    if (x >= 0 && x < state.win_width && y >= 0 && y < state.win_height)
        buffer[y * state.win_width + x] = color;
}

void drawWall(int x0, int t0, int b0, int x1, int t1, int b1, uint16_t color) {
	int db = b1 - b0;
	int dt = t1 - t0;
	int dx = x1 - x0; if (!dx) dx = 1;
	int sx = x0;
	if (x0 >= state.win_width) x0 = state.win_width - 1;
	if (x1 >= state.win_width) x1 = state.win_width - 1;
	if (x0 < 0) x0 = 0;
	if (x1 < 0) x1 = 0;
	
	for (int x = x0; x < x1; x++) {
		int y0 = db * (x - sx + 0.5) / dx + b0;
		int y1 = dt * (x - sx + 0.5) / dx + t0;

		if (y0 >= state.win_height) y0 = state.win_height - 1;
		if (y1 >= state.win_height) y1 = state.win_height - 1;
		if (y0 < 0) y0 = 0;
		if (y1 < 0) y1 = 0;

		// pixel(x, y0, color);
		// pixel(x, y1, color);
		for (int y = y0; y < y1; y++) {
			pixel(x, y, color);
		}
	}
}

void clip_quad(float *x0, float *y0, float *z0, float x1, float y1, float z1) {
	float da = *y0;
	float d = da - y1; if (!d) d = 1;
	float s = d / (da - y1);
	*x0 += s * (x1 - *x0);
	*y0 += s * (y1 - *y0);
	*z0 += s * (z1 - *z0);
}

void renderer_init(SDL_Renderer* renderer) {
    buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA4444, SDL_TEXTUREACCESS_STREAMING, state.win_width, state.win_height);
    buffer = (uint16_t*)malloc(state.win_width * state.win_height * sizeof(uint16_t));
}

void renderer_render(SDL_Renderer* renderer) {
	memset(buffer, 0, state.win_width * state.win_height * sizeof(uint16_t));

	float ca = cosf(state.origin.rotation);
	float sa = sinf(state.origin.rotation);
    SDL_RenderClear(renderer);

	float x0 = 0 - state.origin.position.x;
	float y0 = 0 - state.origin.position.y;
	
	float x1 = 0 - state.origin.position.x;
	float y1 = 32- state.origin.position.y;
	
	float wx[4], wy[4], wz[4];
	
	wx[2] = wx[0] = x0 * ca - y0 * sa;
	wx[3] = wx[1] = x1 * ca - y1 * sa;
	
	wy[2] = wy[0] = y0 * ca + x0 * sa;
	wy[3] = wy[1] = y1 * ca + x1 * sa;	
	
	wz[1] = wz[0] = 0 - state.origin.position.z;
	wz[3] = wz[2] = 40 - state.origin.position.z;
	
	if (wy[0] < 1 && wy[1] < 1) return;
	else if (wy[0] < 1) {
		clip_quad(&wx[0], &wy[0], &wz[0], wx[1], wy[1], wz[1]);
		clip_quad(&wx[2], &wy[2], &wz[2], wx[3], wy[3], wz[3]);
	}
	else if (wy[1] < 1) {
		clip_quad(&wx[1], &wy[1], &wz[1], wx[0], wy[0], wz[0]);
		clip_quad(&wx[3], &wy[3], &wz[3], wx[2], wy[2], wz[2]);
	}
	
	
	wx[0] = wx[0] * 200 / wy[0] + state.win_width/2;
	wx[1] = wx[1] * 200 / wy[1] + state.win_width/2;
	
	wx[2] = wx[2] * 200 / wy[2] + state.win_width/2;
	wx[3] = wx[3] * 200 / wy[3] + state.win_width/2;
	
	wy[0] = wz[0] * 200 / wy[0] + state.win_height/2;
	wy[1] = wz[1] * 200 / wy[1] + state.win_height/2;
	
	wy[2] = wz[2] * 200 / wy[2] + state.win_height/2;
	wy[3] = wz[3] * 200 / wy[3] + state.win_height/2;
	
	// drawWall(wx[0], wx[1], wy[0], wy[1], wy[2], wy[3], 0xFFFF);
	// xx tt bb
	drawWall(wx[0], wy[2], wy[0], wx[1], wy[3], wy[1], 0xFF0F);

    SDL_UpdateTexture(buffer_texture, NULL, buffer, state.win_width * sizeof(uint16_t));
    SDL_RenderTexture(renderer, buffer_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}


// void renderer_destroy() {
	// free(buffer);
	// SDL_DestroyTexture(buffer_texture);
// }
