#pragma once

#include <stdio.h>
#include <stdlib.h>

int load_map(const char *path, wall_t* walls, sector_t* sectors, uint8_t *wall_count, uint8_t *sect_count)
{
    FILE *f = fopen(path, "r");
	if (!f) printf("failed to open map file\n");
    char ln[256];
	uint8_t w, s;
    while (fgets(ln, 256, f))
    {
		printf("new line!\n");
        switch (ln[0])
        {
        case 'w':
			*wall_count += 1;
            w = *wall_count;
            walls = realloc(walls, sizeof(wall_t) * (w));
			w -= 1;
            walls[w ] = (wall_t){0};
            sscanf(ln,
                   "w %f %f %f %f %hx",
                   &walls[w].x1,
                   &walls[w].y1,
                   &walls[w].x2,
                   &walls[w].y2,
                   &walls[w].color);
            break;
        case 's':
			*sect_count += 1;
            s = *sect_count;
            sectors = realloc(sectors, sizeof(sector_t) * (s));
			s -= 1;
            sectors[s] = (sector_t){0};
            sscanf(ln,
                   "s %hhu %hhu %f %f %hx",
                   &sectors[s].idx,
                   &sectors[s].end,
                   &sectors[s].z1,
                   &sectors[s].z2,
                   &sectors[s].color);
            break;
        default:
            break;
        }
        printf("%s\n", ln);
    }
    fclose(f);
    return 1;
}
