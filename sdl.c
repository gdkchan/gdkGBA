#include "sdl.h"

void sdl_init() {
    SDL_Init(SDL_INIT_VIDEO);

    window   = SDL_CreateWindow("gdkGBA", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480, 320, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture  = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_BGRA8888,
        SDL_TEXTUREACCESS_STREAMING, 
        240,
        160);

    tex_pitch = 240 * 4;
}

void sdl_uninit() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}