#include <stdio.h>
#include <stdlib.h>

#include "arm.h"
#include "arm_mem.h"

#include "io.h"
#include "sdl.h"
#include "video.h"

const int64_t max_rom_sz = 32 * 1024 * 1024;

int main(int argc, char* argv[]) {
    printf("gdkGBA - Gameboy Advance emulator made by gdkchan\n");
    printf("This is FREE software released into the PUBLIC DOMAIN\n\n");

    arm_init();

    if (argc < 2) {
        printf("Error: Invalid number of arguments!\n");
        printf("Please specify a ROM file.\n");

        return 0;
    }

    FILE *image;

    image = fopen("gba_bios.bin", "rb");

    if (image == NULL) {
        printf("Error: GBA BIOS not found!\n");
        printf("Place it on this directory with the name \"gba_bios.bin\".\n");

        return 0;
    }

    fread(bios, 16384, 1, image);

    fclose(image);

    image = fopen(argv[1], "rb");

    if (image == NULL) {
        printf("Error: ROM file couldn't be opened.\n");
        printf("Make sure that the file exists and the name is correct.\n");

        return 0;
    }

    fseek(image, 0, SEEK_END);

    cart_rom_size = ftell(image);

    if (cart_rom_size > max_rom_sz) cart_rom_size = max_rom_sz;

    fseek(image, 0, SEEK_SET);
    fread(rom, cart_rom_size, 1, image);

    fclose(image);

    sdl_init();
    arm_reset();

    bool run = true;

    while (run) {
        run_frame();

        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_UP:     key_input.w &= ~BTN_U;   break;
                        case SDLK_DOWN:   key_input.w &= ~BTN_D;   break;
                        case SDLK_LEFT:   key_input.w &= ~BTN_L;   break;
                        case SDLK_RIGHT:  key_input.w &= ~BTN_R;   break;
                        case SDLK_a:      key_input.w &= ~BTN_A;   break;
                        case SDLK_s:      key_input.w &= ~BTN_B;   break;
                        case SDLK_q:      key_input.w &= ~BTN_LT;  break;
                        case SDLK_w:      key_input.w &= ~BTN_RT;  break;
                        case SDLK_TAB:    key_input.w &= ~BTN_SEL; break;
                        case SDLK_RETURN: key_input.w &= ~BTN_STA; break;
                        default:                                   break;
                    }
                break;

                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_UP:     key_input.w |= BTN_U;   break;
                        case SDLK_DOWN:   key_input.w |= BTN_D;   break;
                        case SDLK_LEFT:   key_input.w |= BTN_L;   break;
                        case SDLK_RIGHT:  key_input.w |= BTN_R;   break;
                        case SDLK_a:      key_input.w |= BTN_A;   break;
                        case SDLK_s:      key_input.w |= BTN_B;   break;
                        case SDLK_q:      key_input.w |= BTN_LT;  break;
                        case SDLK_w:      key_input.w |= BTN_RT;  break;
                        case SDLK_TAB:    key_input.w |= BTN_SEL; break;
                        case SDLK_RETURN: key_input.w |= BTN_STA; break;
                        default:                                  break;
                    }
                break;

                case SDL_QUIT: run = false; break;
            }
        }
    }

    sdl_uninit();
    arm_uninit();

    return 0;
}