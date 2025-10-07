#include "r_renderer.h"
#include <SDL3/SDL_render.h>

SDL_Texture* buffer_texture;
uint16_t* buffer;

struct lut {
	int16_t *t, *b;
} ceil_lut, floor_lut;

typedef struct {
	int x, y1, y2, z1, z2;
} wwall_t;

typedef struct {
	int x1, x2;
	int t1, t2;
	int b1, b2;
} swall_t;

void pixel(int x, int y, uint16_t color) {
    buffer[y * state.win_width + x] = color;
}

void drawWall(int x1, int x2, int b1, int b2, int t1, int t2, texture_t* texture) {

	bool backface;
	if (x2 < x1) {
		int swp = x2;
			x2 = x1;
			x1 = swp;
		swp = t2;
			t2 = t1;
			t1 = swp;
		swp = b2;
			b2 = b1;
			b1 = swp;
		backface = false;
	}
	else backface = true;

	int db = b2 - b1;
	int dt = t2 - t1;
	int dx = x2 - x1; if (!dx) dx = 1;
	int sx = x1;

	float ht = 0;
	float ht_step = (float)texture->w / (float)(x2 - x1);

	if (x1 < 0) {
		ht -= ht_step * x1;
	}

	if (x1 >= state.win_width) x1 = state.win_width - 1;
	if (x2 >= state.win_width) x2 = state.win_width - 1;
	if (x1 < 0) x1 = 0;
	if (x2 < 0) x2 = 0;
	
	for (int x = x1; x < x2; x++) {
		int y1 = db * (x - sx + 0.5) / dx + b1;
		int y2 = dt * (x - sx + 0.5) / dx + t1;

		float vt = 0;
		float vt_step = (float)texture->h / (float)(y2 - y1);

		if (y1 < 0) {
			vt -= vt_step * y1;
		}

		if (y1 >= state.win_height) y1 = state.win_height - 1;
		if (y2 >= state.win_height) y2 = state.win_height - 1;
		if (y1 < 0) y1 = 0;
		if (y2 < 0) y2 = 0;

		if (!backface) {
			for (int y = y1; y < y2; y++) {
				uint16_t color = texture->pixels[(int)vt * texture->w + (int)ht];
				pixel(x, y, color);
				vt += vt_step;
			}
		}
		ht += ht_step;

		if (backface) {
			ceil_lut.t[x] = y1;
			floor_lut.b[x] = y2;
		}
		else {
			ceil_lut.b[x] = y1;
			floor_lut.t[x] = y2;
		}

	}
}

void clip_line(int *x1, int *y1, int *z0, int x2, int y2, int z1) {
	double da = *y1;
	double d = da - y2; if (!d) d = 1;
	double s = da / (da - y2);
	*x1 += s * (x2 - *x1);
	*y1 += s * (y2 - *y1); if (!*y1) *y1 = 1;
	*z0 += s * (z1 - *z0);
}


#define dist(x, y) sqrt((x*x) + (y*y))

void renderer_init(SDL_Renderer* renderer) {
    buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STREAMING, state.win_width, state.win_height);
	buffer = (uint16_t*)malloc(state.win_width * state.win_height * sizeof(uint16_t));
	ceil_lut.t = (int16_t*)malloc(state.win_width * sizeof(int16_t));
	ceil_lut.b = (int16_t*)malloc(state.win_width * sizeof(int16_t));
	floor_lut.t = (int16_t*)malloc(state.win_width * sizeof(int16_t));
	floor_lut.b = (int16_t*)malloc(state.win_width * sizeof(int16_t));
}


void sort_sectors(void) {
	double o = sectors[0].pos.x - state.origin.position.x;
	double y = sectors[0].pos.y - state.origin.position.y;
	
	double x = o * state.origin.cos_rotation + y * state.origin.sin_rotation;
		  y = y * state.origin.cos_rotation - o * state.origin.sin_rotation;
	
	sectors[0].dist = dist(x, y);
	
	for (int s = 1; s < sector_count; s++) {
        o = sectors[s].pos.x - state.origin.position.x;
        y = sectors[s].pos.y - state.origin.position.y;

        x = o * state.origin.cos_rotation + y * state.origin.sin_rotation;
        y = y * state.origin.cos_rotation - o * state.origin.sin_rotation;
		
        sectors[s].dist = dist(x, y);
        if (sectors[s].dist > sectors[s - 1].dist)
        {
            sector_t swp;
            swp = sectors[s];
            sectors[s] = sectors[s - 1];
            sectors[s - 1] = swp;
        }
	}
}

void draw_sector(int i) {
	for (int j = sectors[i].idx; j < sectors[i].end; j++) {
		int k = j+1;
		if (k == sectors[i].end) k = sectors[i].idx;
		vec2i a = {
			walls[j].x - state.origin.position.x,
			walls[j].y - state.origin.position.y
		};
		
		vec2i b = {
			walls[k].x - state.origin.position.x,
			walls[k].y - state.origin.position.y
		};
		
		int wx[4], wy[4], wz[4];
		
		wx[2] = wx[0] = a.x * state.origin.cos_rotation - a.y * state.origin.sin_rotation;
		wx[3] = wx[1] = b.x * state.origin.cos_rotation - b.y * state.origin.sin_rotation;
		
		wy[2] = wy[0] = a.x * state.origin.sin_rotation + a.y * state.origin.cos_rotation;
		wy[3] = wy[1] = b.x * state.origin.sin_rotation + b.y * state.origin.cos_rotation;
		
		wz[1] = wz[0] = sectors[i].z1 - state.origin.position.z;
		wz[3] = wz[2] = sectors[i].z2 - state.origin.position.z;
		
		if (wy[0] <= 0 && wy[1] <= 0) {
			continue;
		}
		else if (wy[0] <= 0) {
			clip_line(&wx[0], &wy[0], &wz[0], wx[1], wy[1], wz[1]);
			clip_line(&wx[2], &wy[2], &wz[2], wx[3], wy[3], wz[3]);
		}
		else if (wy[1] <= 0) {
			clip_line(&wx[1], &wy[1], &wz[1], wx[0], wy[0], wz[0]);
			clip_line(&wx[3], &wy[3], &wz[3], wx[2], wy[2], wz[2]);
		}
		
		
		wx[0] = wx[0] * FOCAL_LENGTH / wy[0] + state.win_width/2;
		wx[1] = wx[1] * FOCAL_LENGTH / wy[1] + state.win_width/2;
		
		wy[0] = wz[0] * FOCAL_LENGTH / wy[0] + state.win_height/2;
		wy[1] = wz[1] * FOCAL_LENGTH / wy[1] + state.win_height/2;
		wy[2] = wz[2] * FOCAL_LENGTH / wy[2] + state.win_height/2;
		wy[3] = wz[3] * FOCAL_LENGTH / wy[3] + state.win_height/2;

		
		drawWall(wx[0], wx[1], wy[0], wy[1], wy[2], wy[3], &textures[walls[j].texture]);
	}
	for (int px = 0; px < state.win_width; px++) {
		if (ceil_lut.t[px] == -1 && ceil_lut.b[px] == -1) goto next;
		if (ceil_lut.t[px] == -1) {
			ceil_lut.t[px] = 0;
		}
		else if (ceil_lut.b[px] == -1) {
			ceil_lut.b[px] = state.win_height - 1;
		}
		for (int py = ceil_lut.t[px]; py < ceil_lut.b[px]; py++) {
			pixel(px, py, 0xF0F3);
		}
		next:
		if (floor_lut.t[px] == -1 && floor_lut.b[px] == -1) continue;
		if (floor_lut.t[px] == -1) {
			floor_lut.t[px] = 0;
		}
		else if (floor_lut.b[px] == -1) {
			floor_lut.b[px] = state.win_height - 1;
		}
		for (int py = floor_lut.t[px]; py < floor_lut.b[px]; py++) {
			pixel(px, py, 0xF003);
		}
	}
}


void renderer_render(SDL_Renderer* renderer) {
	memset(buffer, 0x2222, state.win_width * state.win_height * sizeof(uint16_t));

    SDL_RenderClear(renderer);
	
	sort_sectors();
	
	for (int i = 0; i < sector_count; i++) {
		memset(ceil_lut.t, -1, state.win_width * sizeof(int16_t));
		memset(ceil_lut.b, -1, state.win_width * sizeof(int16_t));
		memset(floor_lut.t, -1, state.win_width * sizeof(int16_t));
		memset(floor_lut.b, -1, state.win_width * sizeof(int16_t));
		draw_sector(i);
	}
    SDL_UpdateTexture(buffer_texture, NULL, buffer, state.win_width * sizeof(uint16_t));
    SDL_RenderTexture(renderer, buffer_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}


void renderer_destroy(void) {
	free(buffer);
	SDL_DestroyTexture(buffer_texture);
	free(ceil_lut.t);
	free(ceil_lut.b);
	free(floor_lut.t);
	free(floor_lut.b);
}
