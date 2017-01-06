#include <SDL2/SDL.h>

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;

int32_t tex_pitch;

void sdl_init();
void sdl_uninit();