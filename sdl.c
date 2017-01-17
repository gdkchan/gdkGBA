#include "sdl.h"
#include "sound.h"

void sdl_init() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    window   = SDL_CreateWindow("gdkGBA", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480, 320, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    texture  = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_BGRA8888,
        SDL_TEXTUREACCESS_STREAMING,
        240,
        160);

    tex_pitch = 240 * 4;

    SDL_AudioSpec spec = {
        .freq     = SND_FREQUENCY, //32KHz
        .format   = AUDIO_S16SYS, //Signed 16 bits System endiannes
        .channels = SND_CHANNELS, //Stereo
        .samples  = SND_SAMPLES, //16ms
        .callback = sound_mix,
        .userdata = NULL
    };

    SDL_OpenAudio(&spec, NULL);
    SDL_PauseAudio(0);
}

void sdl_uninit() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_CloseAudio();
    SDL_Quit();
}