#define SDL_MAIN_HANDLED

#include "g_game_state.h"
#include "r_renderer.h"


#include "g_data.h"
#include "g_loader.h"
#include "g_collision.h"

#define G_WIDTH 960
#define G_HEIGHT 480
#define G_FPS 120

void game_loop(void) {
    g_frame_start();

    sort_sectors();
    state.origin.position.z = sectors[state.origin.current_sector].z2 - 60;

    renderer_render(state.renderer);
    g_frame_end();
}

wall_t *walls;
sector_t *sectors;
uint8_t wall_count = 0;
uint8_t sector_count = 0;
texture_t* textures;
uint8_t texture_count = 0;

int main(void) {
    game_init(G_WIDTH, G_HEIGHT, G_FPS);
	printf("game initialized\n");
	load_map("../map.txt");
	
    renderer_init(state.renderer);

    while (state.state != STATE_EXIT) {
        while (SDL_PollEvent(&state.event)) {
            switch (state.event.type) {
                case SDL_EVENT_QUIT:
                    state.state = STATE_EXIT;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    state.keys[state.event.key.scancode] = true;
                    break;
                case SDL_EVENT_KEY_UP:
                    state.keys[state.event.key.scancode] = false;
                    break;
            }
        }
        game_loop();
    }
    free(walls);
    free(sectors);
    free(textures);

    return 0;
}
