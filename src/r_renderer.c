#include "r_renderer.h"
#include <SDL3/SDL_render.h>

SDL_Texture* buffer_texture;
uint16_t* buffer;

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

void draw_wall(int x1, int x2, int b1, int b2, int t1, int t2,
			   texture_t* texture,
			   int px1, int px2, int pb1, int pb2, int pt1, int pt2,
			   uint8_t to, lut_t* floor_lut, lut_t* ceil_lut) {
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
		backface = true;
	}
	else backface = false;

	int db = b2 - b1;
	int dt = t2 - t1;
	int dx = x2 - x1; if (!dx) dx = 1;
	int sx = x1;

	int dpb = pb2 - pb1;
	int dpt = pt2 - pt1;


	float ht = 0;
	float ht_step = (float)texture->w / (float)(x2 - x1);

	if (x1 < 0) {
		ht -= ht_step * x1;
	}

	if (x1 >= px2) x1 = px2;
	if (x2 >= px2) x2 = px2;
	if (x1 < px1) x1 = px1;
	if (x2 < px1) x2 = px1;
	
	for (int x = x1; x < x2; x++) {
		int y1 = db * (x - sx + 0.5) / dx + b1;
		int y2 = dt * (x - sx + 0.5) / dx + t1;

		int py2 = dpb * (x - sx + 0.5) / dx + pb1;
		int py1 = dpt * (x - sx + 0.5) / dx + pt1;

		float vt = 0;
		float vt_step = (float)texture->h / (float)(y2 - y1);

		if (y1 < 0) {
			vt -= vt_step * y1;
		}

		if (y1 >= py2) y1 = py2;
		if (y2 >= py2) y2 = py2;
		if (y1 < py1) y1 = py1;
		if (y2 < py1) y2 = py1;

		if (!backface) {
			ceil_lut->t[x] = y1;
			floor_lut->b[x] = y2;
		}
		else {
			ceil_lut->b[x] = y1;
			floor_lut->t[x] = y2;

			if (!to) {
				for (int y = y1; y < y2; y++) {
					uint16_t color = texture->pixels[(int)vt * texture->w + (int)ht];
					pixel(x, y, color);
					vt += vt_step;
				}
			}
		}
		ht += ht_step;
	}
}

void clip_line(int *x1, int *y1, int *z1, int x2, int y2, int z2) {
	double da = *y1;
	double d = da - y2; if (!d) d = 1;
	double s = da / (da - y2);
	*x1 += s * (x2 - *x1);
	*y1 += s * (y2 - *y1); if (!*y1) *y1 = 1;
	*z1 += s * (z2 - *z1);
}


#define dist(x, y) sqrt((x*x) + (y*y))

void renderer_init(SDL_Renderer* renderer) {
    buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STREAMING, state.win_width, state.win_height);
	buffer = (uint16_t*)malloc(state.win_width * state.win_height * sizeof(uint16_t));
}

void draw_sector(int i,
				int px1, int px2, int pb1, int pb2, int pt1, int pt2) {
	memset(sectors[i].ceil_lut.t, -1, state.win_width * sizeof(int16_t));
	memset(sectors[i].ceil_lut.b, -1, state.win_width * sizeof(int16_t));
	memset(sectors[i].floor_lut.t, -1, state.win_width * sizeof(int16_t));
	memset(sectors[i].floor_lut.b, -1, state.win_width * sizeof(int16_t));

	int wx[4], wy[4], wz[4];

	for (int j = sectors[i].idx; j < sectors[i].end; j++) {
		{
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
		}

		int x1 = wx[0] * FOCAL_LENGTH / wy[0] + state.win_width/2;
		int x2 = wx[1] * FOCAL_LENGTH / wy[1] + state.win_width/2;
		int b1 = wz[0] * FOCAL_LENGTH / wy[0] + state.win_height/2;
		int b2 = wz[1] * FOCAL_LENGTH / wy[1] + state.win_height/2;
		int t1 = wz[2] * FOCAL_LENGTH / wy[2] + state.win_height/2;
		int t2 = wz[3] * FOCAL_LENGTH / wy[3] + state.win_height/2;

		draw_wall(
			x1, x2,
			b1, b2,
			t1, t2,
			&textures[walls[j].texture],
			px1, px2,
			pb1, pb2,
			pt1, pt2,
			walls[j].to,
			&sectors[i].floor_lut,
			&sectors[i].ceil_lut
		);


		if (walls[j].to && x2 < x1) {
			draw_sector(i + walls[j].to, px1, px2, pb1, pb2, pt1, pt2);
			printf("%hhd\n to %hhd\n", walls[j].to, i + walls[j].to);
		}
		else {
			printf("not portal\n");
		}
	}
	for (int px = 0; px < state.win_width; px++) {
		if (sectors[i].ceil_lut.t[px] == -1 && sectors[i].ceil_lut.b[px] == -1) goto next;
		if (sectors[i].ceil_lut.t[px] == -1) {
			sectors[i].ceil_lut.t[px] = 0;
		}
		else if (sectors[i].ceil_lut.b[px] == -1) {
			sectors[i].ceil_lut.b[px] = state.win_height - 1;
		}
		for (int py = sectors[i].ceil_lut.t[px]; py < sectors[i].ceil_lut.b[px]; py++) {
			pixel(px, py, 0xF063);
		}
		next:
		if (sectors[i].floor_lut.t[px] == -1 && sectors[i].floor_lut.b[px] == -1) continue;
		if (sectors[i].floor_lut.t[px] == -1) {
			sectors[i].floor_lut.t[px] = 0;
		}
		else if (sectors[i].floor_lut.b[px] == -1) {
			sectors[i].floor_lut.b[px] = state.win_height - 1;
		}
		for (int py = sectors[i].floor_lut.t[px]; py < sectors[i].floor_lut.b[px]; py++) {
			pixel(px, py, 0x200F);
		}
	}
}


void renderer_render(SDL_Renderer* renderer) {
	SDL_RenderClear(renderer);
	memset(buffer, 0, state.win_width * state.win_height * sizeof(int16_t));

	draw_sector(state.origin.current_sector,
					0, state.win_width-1,
					state.win_height-1, state.win_height-1,
					0, 0);

    SDL_UpdateTexture(buffer_texture, NULL, buffer, state.win_width * sizeof(uint16_t));
    SDL_RenderTexture(renderer, buffer_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}


void renderer_destroy(void) {
	free(buffer);
	SDL_DestroyTexture(buffer_texture);
}
