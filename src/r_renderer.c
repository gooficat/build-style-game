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
    buffer[y * RW + x] = color;
}

void draw_wall(int x1, int x2, int b1, int b2, int t1, int t2,
			   texture_t* texture,
			   int px1, int px2, int pb1, int pb2, int pt1, int pt2,
			   uint8_t to, lut_t* floor_lut, lut_t* ceil_lut,
			   int z2, int z1) {
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


		swp = px2;
			px2 = px1;
			px1 = swp;
		swp = pt2;
			pt2 = pt1;
			pt1 = swp;
		swp = pb2;
			pb2 = pb1;
			pb1 = swp;
		swp = z2;
			z2 = z1;
			z1 = swp;
		backface = true;
	}
	else backface = false;

	int db = b2 - b1;
	int dt = t2 - t1;
	int dx = x2 - x1; if (!dx) dx = 1;
	int sx = x1;

	int dpx = px2 - px1;
	int dpb = pb2 - pb1;
	int dpt = pt2 - pt1;
	int psx = px1;

	int dz = (z2) - (z1);// if (!dz) dz = 1;




	float u = 0;
	float u_step = (float)texture->w / (float)(x2 - x1);

	if (x1 < px1) {
		u -= u_step * (px1 + x1);
	}

	if (x2 >= px2) x2 = px2-1;
	if (x1 < px1) x1 = px1;

	if (x1 < 0) {
		u -= u_step * (0 + x1);
	}

	float z = z1;
	float z_step = (float)dz / (float)(x2 - x1);

	x1 = clamp(x1, 0, RW-1);
	x2 = clamp(x2, 0, RW-1);


	int px = x1;

	
	for (int x = x1; x < x2; x++) {
		px++;
		int y1 = db * (x - sx + 0.5) / dx + b1;
		int y2 = dt * (x - sx + 0.5) / dx + t1;

		int py1 = dpb * (px - psx + 0.5) / dpx + pb1;
		int py2 = dpt * (px - psx + 0.5) / dpx + pt1;

		float v = 0;
		float v_step = (float)texture->h / (float)(y2 - y1);

		if (y1 < py1) {
			v -= v_step * (y1 - py1);
		}

		if (y2 >= py2) y2 = py2-1;
		if (y1 < py1) y1 = py1;

		if (y1 < 0) {
			v -= v_step * y1;
		}

		y1 = clamp(y1, 0, RH-1);
		y2 = clamp(y2, 0, RH-1);


		if (!backface) {
			ceil_lut->t[x] = y1;
			floor_lut->b[x] = y2;
		}
		else {
			ceil_lut->b[x] = y1;
			floor_lut->t[x] = y2;

			if (!to) {
				for (int y = y1; y < y2; y++) {
					uint16_t color = texture->pixels[(int)(v) * texture->w + (int)u];
					pixel(x, y, color);
					v += v_step;
				}
				u += u_step;
			}
		}
		z += z_step;
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
    buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STREAMING, RW, RH);
	buffer = (uint16_t*)malloc(RW * RH * sizeof(uint16_t));
}

void draw_sector(int i,
				int px1, int px2, int pb1, int pb2, int pt1, int pt2) {
	sector_t *s = &sectors[i];
	memset(s->ceil_lut.t, -1, RW * sizeof(int16_t));
	memset(s->ceil_lut.b, -1, RW * sizeof(int16_t));
	memset(s->floor_lut.t, -1, RW * sizeof(int16_t));
	memset(s->floor_lut.b, -1, RW * sizeof(int16_t));

	int wx[4], wy[4], wz[4];

	for (int j = s->idx; j < s->end; j++) {
		{
			int k = j+1;
			if (k == s->end) k = s->idx;

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

			wz[1] = wz[0] = s->z1 - state.origin.position.z;
			wz[3] = wz[2] = s->z2 - state.origin.position.z;

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

		int x1 = wx[0] * FOCAL_LENGTH / wy[0] + RW/2;
		int x2 = wx[1] * FOCAL_LENGTH / wy[1] + RW/2;
		int b1 = wz[0] * FOCAL_LENGTH / wy[0] + RH/2;
		int b2 = wz[1] * FOCAL_LENGTH / wy[1] + RH/2;
		int t1 = wz[2] * FOCAL_LENGTH / wy[2] + RH/2;
		int t2 = wz[3] * FOCAL_LENGTH / wy[3] + RH/2;

		draw_wall(
			x1, x2,
			b1, b2,
			t1, t2,
			&textures[walls[j].texture],
			px1, px2,
			pb1, pb2,
			pt1, pt2,
			walls[j].to,
			&s->floor_lut,
			&s->ceil_lut,
			wy[0], wy[2]
		);

		if (walls[j].to && x2 < x1) {
			draw_sector(i + walls[j].to, x1, x2, b1, b2, t1, t2);
		}
	}
	for (int px = 0; px < RW; px++) {
		if (s->ceil_lut.t[px] == -1 && s->ceil_lut.b[px] == -1) goto next;
		if (s->ceil_lut.t[px] == -1) {
			s->ceil_lut.t[px] = 0;
		}
		else if (s->ceil_lut.b[px] == -1) {
			s->ceil_lut.b[px] = RH - 1;
		}
		for (int py = s->ceil_lut.t[px]; py < s->ceil_lut.b[px]; py++) {
			pixel(px, py, s->c_color);
		}
		next:
		if (s->floor_lut.t[px] == -1 && s->floor_lut.b[px] == -1) continue;
		if (s->floor_lut.t[px] == -1) {
			s->floor_lut.t[px] = 0;
		}
		else if (s->floor_lut.b[px] == -1) {
			s->floor_lut.b[px] = RH - 1;
		}
		for (int py = s->floor_lut.t[px]; py < s->floor_lut.b[px]; py++) {
			pixel(px, py, s->f_color);
		}
	}
}

void renderer_render(SDL_Renderer* renderer) {
	SDL_RenderClear(renderer);
	memset(buffer, 0, RW * RH * sizeof(int16_t));

	draw_sector(state.origin.current_sector,
					RW-1, 0, 0, 0,
					RH-1, RH-1);

    SDL_UpdateTexture(buffer_texture, NULL, buffer, RW * sizeof(uint16_t));
    SDL_RenderTexture(renderer, buffer_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}


void renderer_destroy(void) {
	free(buffer);
	SDL_DestroyTexture(buffer_texture);
}
