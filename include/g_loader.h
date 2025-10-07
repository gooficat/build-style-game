#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <SDL3_image/SDL_image.h>


#include "g_game_state.h"

int load_map(const char *path)
{
    FILE *f = fopen(path, "r");
	if (!f) printf("failed to open map file\n");
    char ln[256];
    char* texPath = "\0";
    SDL_Surface* texSurf;
	uint8_t w, s, t;
    while (fgets(ln, 256, f)) {
        switch (ln[0]) {
        case 'w':
			wall_count += 1;
            w = wall_count;
            walls = realloc(walls, sizeof(wall_t) * (w));
			w -= 1;
            walls[w] = (wall_t){0};
            sscanf(ln,
                   "w %d %d %hhu",
                   &walls[w].x,
                   &walls[w].y,
                   &walls[w].texture);
            break;
        case 's':
			sector_count += 1;
            s = sector_count;
            sectors = realloc(sectors, sizeof(sector_t) * (s));
			s -= 1;
            sectors[s] = (sector_t){0};
            sscanf(ln,
                   "s %hhu %hhu %d %d %hx",
                   &sectors[s].idx,
                   &sectors[s].end,
                   &sectors[s].z1,
                   &sectors[s].z2,
                   &sectors[s].color);
            break;
		case 'p':
            sscanf(ln,
                   "p %f %f %f %f %*d",
                   &state.origin.position.x,
                   &state.origin.position.y,
                   &state.origin.position.z,
                   &state.origin.rotation);
            break;
        case 't':
            printf("read texture\n");
            *texPath = '\0';
            texPath = "../textures/Glass.PNG";
            texture_count += 1;
            t = texture_count;
            textures = realloc(textures, sizeof(texture_t) * (t));
            t -= 1;
            texSurf = IMG_Load(texPath);
            textures[t] = (texture_t){
                .w = texSurf->w,
                .h = texSurf->h,
                .pixels = malloc(sizeof(uint16_t) * texSurf->w * texSurf->h)
            };
            SDL_ConvertPixels(textures[t].w, textures[t].h, texSurf->format, texSurf->pixels, texSurf->pitch, SDL_PIXELFORMAT_ARGB4444, textures[t].pixels, sizeof(uint16_t) * textures[t].w);
            break;
        default:
            break;
        }
    }
	//get an average of the vertices 
	for (s = 0; s < sector_count; s++) {
		double vx = 0;
		double vy = 0;
        for (int w = sectors[s].idx; w != sectors[s].end; w += 1) {
			int w2 = w+1;
			if (w2 == sectors[s].end) w2 = sectors[s].idx;
            vx += (walls[w].x + walls[w2].x) / 2.0;
            vy += (walls[w].y + walls[w2].y) / 2.0;
        }
        sectors[s].pos.x = vx / (double)(sectors[s].end - sectors[s].idx);
        sectors[s].pos.y = vy / (double)(sectors[s].end - sectors[s].idx);
	}
    fclose(f);
    return 1;
}
