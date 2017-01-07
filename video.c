#include <stdio.h>

#include "arm.h"
#include "arm_mem.h"

#include "dma.h"
#include "io.h"
#include "sdl.h"

#define CPU_FREQ_HZ  16777216

#define LINES_VISIBLE  160
#define LINES_TOTAL    228

#define CYC_LINE_TOTAL  1232
#define CYC_LINE_HBLK0  1006
#define CYC_LINE_HBLK1  (CYC_LINE_TOTAL - CYC_LINE_HBLK0)

void *screen;

static const uint8_t x_tiles_lut[16] = { 1, 2, 4, 8, 2, 4, 4, 8, 1, 1, 2, 4, 0, 0, 0, 0 };
static const uint8_t y_tiles_lut[16] = { 1, 2, 4, 8, 1, 1, 2, 4, 2, 4, 4, 8, 0, 0, 0, 0 };

static void render_obj(uint8_t prio) {
    if (!(disp_cnt.w & OBJ_ENB)) return;

    uint8_t obj_index;
    uint32_t offset = 0x3f8;

    for (obj_index = 0; obj_index < 128; obj_index++) {
        uint16_t attr0 = oam[offset + 0] | (oam[offset + 1] << 8);
        uint16_t attr1 = oam[offset + 2] | (oam[offset + 3] << 8);
        uint16_t attr2 = oam[offset + 4] | (oam[offset + 5] << 8);

        offset -= 8;

        int16_t  obj_y    = (attr0 >>  0) & 0xff;
        bool     affine   = (attr0 >>  8) & 0x1;
        bool     dbl_size = (attr0 >>  9) & 0x1;
        bool     hidden   = (attr0 >>  9) & 0x1;
        uint8_t  obj_shp  = (attr0 >> 14) & 0x3;
        uint8_t  affine_p = (attr1 >>  9) & 0x1f;
        uint8_t  obj_size = (attr1 >> 14) & 0x3;
        uint8_t  chr_prio = (attr2 >> 10) & 0x3;

        if (chr_prio != prio || (!affine && hidden)) continue;

        int16_t pa, pb, pc, pd;

        if (affine) {
            uint32_t p_base = affine_p * 32;

            pa = oam[p_base + 0x06] | (oam[p_base + 0x07] << 8);
            pb = oam[p_base + 0x0e] | (oam[p_base + 0x0f] << 8);
            pc = oam[p_base + 0x16] | (oam[p_base + 0x17] << 8);
            pd = oam[p_base + 0x1e] | (oam[p_base + 0x1f] << 8);
        }

        uint8_t lut_idx = obj_size | (obj_shp << 2);

        uint8_t x_tiles = x_tiles_lut[lut_idx];
        uint8_t y_tiles = y_tiles_lut[lut_idx];

        uint16_t tw = x_tiles * 8;
        uint16_t th = y_tiles * 8;

        int32_t rcx = x_tiles * 4;
        int32_t rcy = y_tiles * 4;

        if (affine && dbl_size) {
            rcx *= 2;
            rcy *= 2;
        }

        if (obj_y + rcy * 2 > 0xff) obj_y -= 0x100;

        if (obj_y <= (int32_t)v_count.w && (obj_y + rcy * 2 > v_count.w)) {
            uint8_t  obj_mode = (attr0 >> 10) & 0x3;
            bool     mosaic   = (attr0 >> 12) & 0x1;
            bool     is_256   = (attr0 >> 13) & 0x1;
            int16_t  obj_x    = (attr1 >>  0) & 0x1ff;
            bool     flip_x   = (attr1 >> 12) & 0x1;
            bool     flip_y   = (attr1 >> 13) & 0x1;
            uint16_t chr_numb = (attr2 >>  0) & 0x3ff;
            uint8_t  chr_pal  = (attr2 >> 12) & 0xf;

            obj_x <<= 7;
            obj_x >>= 7;

            if (is_256) chr_numb >>= 1;

            int32_t x, y = v_count.w - obj_y;

            if (!affine && flip_y) y ^= (y_tiles * 8) - 1;

            uint8_t tsz = is_256 ? 64 : 32; //Tile block size (in bytes, = (8 * 8 * bpp) / 8)
            uint8_t lsz = is_256 ?  8 :  4; //Pixel line row size (in bytes)

            uint32_t tys = (disp_cnt.w & MAP_1D_FLAG) ? x_tiles * tsz : 1024; //Tile row stride

            for (x = 0; x < rcx * 2; x++) {
                if (obj_x + x < 0) continue;
                if (obj_x + x >= 240) break;

                uint32_t vram_addr;
                uint32_t pal_idx;

                int32_t _x = x;
                int32_t _y = y;

                if (!affine && flip_x) _x ^= (x_tiles * 8) - 1;

                if (affine) {
                    _x = (pa * (x - rcx) + pb * (y - rcy) + (tw << 7)) >> 8;
                    _y = (pc * (x - rcx) + pd * (y - rcy) + (th << 7)) >> 8;
                }

                if (_x < 0 || _x >= tw) continue;
                if (_y < 0 || _y >= th) continue;

                uint16_t tile_x = _x >> 3;
                uint16_t tile_y = _y >> 3;

                uint16_t chr_x  = _x &  7;
                uint16_t chr_y  = _y &  7;

                uint32_t chr_addr =
                    0x10000        +
                    chr_numb * tsz +
                    tile_y   * tys +
                    chr_y    * lsz;

                if (is_256) {
                    vram_addr = chr_addr + tile_x * 64 + chr_x;
                    pal_idx   = vram[vram_addr];
                } else {
                    vram_addr = chr_addr + tile_x * 32 + (chr_x >> 1);
                    pal_idx   = (vram[vram_addr] >> (chr_x & 1) * 4) & 0xf;
                }

                uint32_t surf_addr = v_count.w * 240 * 4 + (obj_x + x) * 4;

                uint32_t pal_addr = 0x100 | pal_idx | (!is_256 ? chr_pal * 16 : 0);

                if (pal_idx) *(uint32_t *)(screen + surf_addr) = palette[pal_addr];
            }
        }
    }
}

static const uint8_t bg_enb[3] = { 0xf, 0x7, 0xc };

static void render_bg() {
    uint8_t mode = disp_cnt.w & 7;
    uint8_t enb = (disp_cnt.w >> 8) & bg_enb[mode];

    uint32_t surf_addr = v_count.w * 240 * 4;

    switch (mode) {
        case 0:
        case 1:
        case 2: {
            int8_t prio, bg_idx;

            for (prio = 3; prio >= 0; prio--) {
                for (bg_idx = 3; bg_idx >= 0; bg_idx--) {
                    if (!(enb & (1 << bg_idx))) continue;
                    if ((bg[bg_idx].ctrl.w & 3) != prio) continue;

                    uint32_t chr_base  = ((bg[bg_idx].ctrl.w >>  2) & 0x3)  << 14;
                    bool     is_256    =  (bg[bg_idx].ctrl.w >>  7) & 0x1;
                    uint16_t scrn_base = ((bg[bg_idx].ctrl.w >>  8) & 0x1f) << 11;
                    uint16_t scrn_size =  (bg[bg_idx].ctrl.w >> 14);

                    bool affine = mode == 2 || (mode == 1 && bg_idx == 2);

                    int32_t xofs, yofs;

                    if (affine) {
                        xofs = (bg_refxl[bg_idx].w | (((int32_t)bg_refxh[bg_idx].w << 20) >> 4)) >> 8;
                        yofs = (bg_refyl[bg_idx].w | (((int32_t)bg_refyh[bg_idx].w << 20) >> 4)) >> 8;
                    } else {
                        xofs = bg[bg_idx].xofs.w;
                        yofs = bg[bg_idx].yofs.w;
                    }

                    uint16_t oy     = v_count.w + yofs;
                    uint16_t tmy    = oy >> 3;
                    uint16_t scrn_y = (tmy >> 5) & 1;

                    uint8_t x;

                    uint32_t address = surf_addr;

                    for (x = 0; x < 240; x++) {
                        uint16_t ox     = x + xofs;
                        uint16_t tmx    = ox >> 3;
                        uint16_t scrn_x = (tmx >> 5) & 1;

                        uint16_t chr_x = ox & 7;
                        uint16_t chr_y = oy & 7;

                        uint16_t pal_idx;
                        uint16_t pal_base = 0;

                        if (affine) {
                            //TODO: Implement Rotation/Scafling that will need a change in this logic
                            uint8_t tms = (16 << scrn_size);
                            uint8_t tmsk = tms - 1;

                            uint32_t map_addr = scrn_base + (tmy & tmsk) * tms + (tmx & tmsk);

                            uint32_t vram_addr = chr_base + vram[map_addr] * 64 + chr_y * 8 + chr_x;

                            pal_idx = vram[vram_addr];
                        } else {
                            uint32_t map_addr = scrn_base + (tmy & 0x1f) * 32 * 2 + (tmx & 0x1f) * 2;

                            switch (scrn_size) {
                                case 1: map_addr += scrn_x * 2048; break;
                                case 2: map_addr += scrn_y * 2048; break;
                                case 3: map_addr += scrn_x * 2048 + scrn_y * 4096; break;
                            }

                            uint16_t tile = vram[map_addr + 0] | (vram[map_addr + 1] << 8);

                            uint16_t chr_numb = (tile >>  0) & 0x3ff;
                            bool     flip_x   = (tile >> 10) & 0x1;
                            bool     flip_y   = (tile >> 11) & 0x1;
                            uint8_t  chr_pal  = (tile >> 12) & 0xf;

                            if (!is_256) pal_base = chr_pal * 16;

                            if (flip_x) chr_x ^= 7;
                            if (flip_y) chr_y ^= 7;

                            uint32_t vram_addr;

                            if (is_256) {
                                vram_addr = chr_base + chr_numb * 64 + chr_y * 8 + chr_x;
                                pal_idx   = vram[vram_addr];
                            } else {
                                vram_addr = chr_base + chr_numb * 32 + chr_y * 4 + (chr_x >> 1);
                                pal_idx   = (vram[vram_addr] >> (chr_x & 1) * 4) & 0xf;
                            }
                        }

                        uint32_t pal_addr = pal_idx | pal_base;

                        if (pal_idx) *(uint32_t *)(screen + address) = palette[pal_addr];

                        address += 4;
                    }
                }

                render_obj(prio);
            }
        }
        break;

        case 3: {
            uint8_t x;
            uint32_t frm_addr = v_count.w * 480;

            for (x = 0; x < 240; x++) {
                uint16_t pixel = vram[frm_addr + 0] | (vram[frm_addr + 1] << 8);

                uint8_t r = ((pixel >>  0) & 0x1f) << 3;
                uint8_t g = ((pixel >>  5) & 0x1f) << 3;
                uint8_t b = ((pixel >> 10) & 0x1f) << 3;

                uint32_t rgba = 0xff;

                rgba |= (r | (r >> 5)) <<  8;
                rgba |= (g | (g >> 5)) << 16;
                rgba |= (b | (b >> 5)) << 24;

                *(uint32_t *)(screen + surf_addr) = rgba;

                surf_addr += 4;

                frm_addr += 2;
            }
        }
        break;

        case 4: {
            uint8_t x, frame = (disp_cnt.w >> 4) & 1;
            uint32_t frm_addr = 0xa000 * frame + v_count.w * 240;

            for (x = 0; x < 240; x++) {
                uint8_t pal_idx = vram[frm_addr++];

                *(uint32_t *)(screen + surf_addr) = palette[pal_idx];

                surf_addr += 4;
            }
        }
        break;
    }
}

static void render_line() {
    uint32_t addr;

    uint32_t addr_s = v_count.w * 240 * 4;
    uint32_t addr_e = addr_s + 240 * 4;

    for (addr = addr_s; addr < addr_e; addr += 0x10) {
        *(uint32_t *)(screen + (addr | 0x0)) = palette[0];
        *(uint32_t *)(screen + (addr | 0x4)) = palette[0];
        *(uint32_t *)(screen + (addr | 0x8)) = palette[0];
        *(uint32_t *)(screen + (addr | 0xc)) = palette[0];
    }

    if ((disp_cnt.w & 7) > 2) {
        render_bg(0);
        render_obj(0);
        render_obj(1);
        render_obj(2);
        render_obj(3);
    } else {
        render_bg();
    }
}

static void vblank_start() {
    if (disp_stat.w & VBLK_IRQ) trigger_irq(VBLK_FLAG);

    disp_stat.w |= VBLK_FLAG;
}

static void hblank_start() {
    if (disp_stat.w & HBLK_IRQ) trigger_irq(HBLK_FLAG);

    disp_stat.w |= HBLK_FLAG;
}

static void vcount_match() {
    if (disp_stat.w & VCNT_IRQ) trigger_irq(VCNT_FLAG);

    disp_stat.w |= VCNT_FLAG;
}

void run_frame() {
    disp_stat.w &= ~VBLK_FLAG;

    SDL_LockTexture(texture, NULL, &screen, &tex_pitch);

    for (v_count.w = 0; v_count.w < LINES_TOTAL; v_count.w++) {
        disp_stat.w &= ~(HBLK_FLAG | VCNT_FLAG);

        //V-Count match and V-Blank start
        if (v_count.w == disp_stat.b.b1) vcount_match();

        if (v_count.w == LINES_VISIBLE) {
            vblank_start();
            dma_transfer(VBLANK);
        }

        arm_exec(CYC_LINE_HBLK0);

        //H-Blank start
        hblank_start();

        if (v_count.w < LINES_VISIBLE) {
            render_line();
            dma_transfer(HBLANK);
        }

        arm_exec(CYC_LINE_HBLK1);
    }

    SDL_UnlockTexture(texture);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}