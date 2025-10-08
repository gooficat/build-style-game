#include "g_collision.h"


bool line_line(vec2 a1, vec2 a2, vec2 b1, vec2 b2) {

}

bool in_sec(int i) {
    bool collision = false;
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

        if ((a.y > 0) != (b.y > 0) &&
            (0 < (b.x - a.x) * (-a.y) / (b.y - a.y) + a.x)) {
            collision = !collision;
            }
    }
    return collision;
}

void sort_sectors(void) {
    for (int s = 0; s < sector_count; s++) {
        if (in_sec(s)) {
            state.origin.current_sector = s;
        }
    }
}
