#include "arm.h"
#include "arm_mem.h"

#include "dma.h"
#include "io.h"
#include "sdl.h"
#include "sound.h"

#define PIXELS_PER_LINE  240
#define BYTES_PER_PIXEL  4

#define LINES_VISIBLE  160
#define LINES_TOTAL    228

#define CYC_LINE_TOTAL  1232
#define CYC_LINE_HBLK0  1006
#define CYC_LINE_HBLK1  (CYC_LINE_TOTAL - CYC_LINE_HBLK0)

#define OAM_LUT_WINOBJ  4

static void *screen;

static uint32_t layer1[PIXELS_PER_LINE];
static uint32_t layer2[PIXELS_PER_LINE];

static uint8_t layerobj[PIXELS_PER_LINE];

static uint64_t oam_attrs[5][128];

static uint32_t oam_counts[5];

static const uint8_t x_tiles_lut[16] = { 1, 2, 4, 8, 2, 4, 4, 8, 1, 1, 2, 4, 0, 0, 0, 0 };
static const uint8_t y_tiles_lut[16] = { 1, 2, 4, 8, 1, 1, 2, 4, 2, 4, 4, 8, 0, 0, 0, 0 };

typedef enum {
    FAILED = 0,
    SUCCESS,
    X_RESULT
} win_y_e;

static bool win_show_x(uint8_t layer, uint8_t x, win_y_e yres) {
    if (yres != X_RESULT) return yres;

    uint8_t mask = 1 << layer;

    bool win0_in = win_in.b.b0 & mask;
    bool win1_in = win_in.b.b1 & mask;

    bool winobj_in = win_out.b.b1 & mask;

    bool win_out_ = win_out.b.b0 & mask;

    bool inside_win0   = false;
    bool inside_win1   = false;
    bool inside_winobj = false;

    if (disp_cnt.w & WIN0_ENB) {
        inside_win0 = x >= win0_h.b.b1 &&
                      x <  win0_h.b.b0;
    }

    if (disp_cnt.w & WIN1_ENB) {
        inside_win1 = x >= win1_h.b.b1 &&
                      x <  win1_h.b.b0;
    }

    if (disp_cnt.w & WINOBJ_ENB) {
        inside_winobj = layerobj[x];
    }

    if (win0_in && inside_win0) return true;
    if (win1_in && inside_win1) return true;

    if (winobj_in && inside_winobj) return true;

    if (win_out_ && !(inside_win0 || inside_win1 || inside_winobj)) return true;

    return false;
}

static win_y_e win_show_y(uint8_t layer, uint8_t y) {
    uint8_t mask = 1 << layer;

    bool win0_in = win_in.b.b0 & mask;
    bool win1_in = win_in.b.b1 & mask;

    bool winobj_in = win_out.b.b1 & mask;

    bool win_out_ = win_out.b.b0 & mask;

    bool inside_win0   = false;
    bool inside_win1   = false;
    bool inside_winobj = false;

    if (disp_cnt.w & WIN0_ENB) {
        inside_win0 = y >= win0_v.b.b1 &&
                      y <  win0_v.b.b0;
    }

    if (disp_cnt.w & WIN1_ENB) {
        inside_win1 = y >= win1_v.b.b1 &&
                      y <  win1_v.b.b0;
    }

    if (disp_cnt.w & WINOBJ_ENB) {
        //This need to be checked per X pixel as it isn't a range,
        //so in this case we should aways return true so the X check is made
        inside_winobj = true;
    }

    if (win0_in && inside_win0) return X_RESULT;
    if (win1_in && inside_win1) return X_RESULT;

    if (winobj_in && inside_winobj) return X_RESULT;

    if (win_out_ && !(inside_win0 || inside_win1 || inside_winobj)) return SUCCESS;

    return win_out_ ? X_RESULT : FAILED;
}

#define MIN(x, y)  ((x) > (y) ? (y) : (x))

#define MUL_COEFF(x, coeff)  (((x) * (coeff)) >> 4)

const uint32_t mask_r = 0xff000000;
const uint32_t mask_g = 0x00ff0000;
const uint32_t mask_b = 0x0000ff00;

static uint32_t get_bright_inc_color(uint64_t color) {
    uint32_t evy = MIN(bld_bright.w & 0x1f, 16);

    return (uint32_t)(color +
        ((MUL_COEFF(mask_r - (color & mask_r), evy) & mask_r) |
         (MUL_COEFF(mask_g - (color & mask_g), evy) & mask_g) |
         (MUL_COEFF(mask_b - (color & mask_b), evy) & mask_b)));
}

static uint32_t get_bright_dec_color(uint64_t color) {
    uint32_t evy = MIN(bld_bright.w & 0x1f, 16);

    return (uint32_t)(color -
        ((MUL_COEFF(color & mask_r, evy) & mask_r) |
         (MUL_COEFF(color & mask_g, evy) & mask_g) |
         (MUL_COEFF(color & mask_b, evy) & mask_b)));
}

static void render_obj(uint8_t prio) {
    if (!(disp_cnt.w & OBJ_ENB)) return;

    bool win = disp_cnt.w & WINANY_ENB;

    win_y_e obj_win_y = 0;
    win_y_e eff_win_y = 0;

    if (win) {
        obj_win_y = win_show_y(LAYER_OBJ, v_count.w);

        if (!obj_win_y) return;

        eff_win_y = win_show_y(LAYER_EFF, v_count.w);
    }

    uint8_t eff = (bld_cnt.w >> 6) & 3;

    bool do_eff = true;

    uint8_t obj_index;

    uint8_t count = oam_counts[prio];

    uint32_t surf_addr = v_count.w * PIXELS_PER_LINE * BYTES_PER_PIXEL;

    for (obj_index = 0; obj_index < count; obj_index++) {
        uint64_t attr = oam_attrs[prio][obj_index];

        int16_t  obj_y    = (attr >>  0) & 0xff;
        bool     affine   = (attr >>  8) & 0x1;
        bool     dbl_size = (attr >>  9) & 0x1;
        uint8_t  obj_shp  = (attr >> 14) & 0x3;
        uint8_t  affine_p = (attr >> 25) & 0x1f;
        uint8_t  obj_size = (attr >> 30) & 0x3;

        int16_t pa, pb, pc, pd;

        pa = pd = 0x100; //1.0
        pb = pc = 0x000; //0.0

        if (affine) {
            uint32_t p_base = affine_p * 32;

            pa = *(int16_t *)(oam + p_base + 0x06);
            pb = *(int16_t *)(oam + p_base + 0x0e);
            pc = *(int16_t *)(oam + p_base + 0x16);
            pd = *(int16_t *)(oam + p_base + 0x1e);
        }

        uint8_t lut_idx = obj_size | (obj_shp << 2);

        uint8_t x_tiles = x_tiles_lut[lut_idx];
        uint8_t y_tiles = y_tiles_lut[lut_idx];

        int32_t rcx = x_tiles * 4;
        int32_t rcy = y_tiles * 4;

        if (affine && dbl_size) {
            rcx *= 2;
            rcy *= 2;
        }

        if (obj_y + rcy * 2 > 0xff) obj_y -= 0x100;

        uint8_t  obj_mode = (attr >> 10) & 0x3;
        bool     mosaic   = (attr >> 12) & 0x1;
        bool     is_256   = (attr >> 13) & 0x1;
        bool     flip_x   = (attr >> 28) & 0x1;
        bool     flip_y   = (attr >> 29) & 0x1;
        uint16_t chr_numb = (attr >> 32) & 0x3ff;
        uint8_t  chr_pal  = (attr >> 44) & 0xf;

        int32_t obj_x = (int32_t)(attr << 7) >> 23;

        uint32_t chr_base = 0x10000 | chr_numb * 32;

        int32_t x = 0, y = v_count.w - obj_y;

        if (!affine && flip_y) y ^= (y_tiles * 8) - 1;

        uint8_t tsz = is_256 ? 64 : 32; //Tile block size (in bytes, = (8 * 8 * bpp) / 8)
        uint8_t lsz = is_256 ?  8 :  4; //Pixel line row size (in bytes)

        int32_t ox = pa * -rcx + pb * (y - rcy) + (x_tiles << 10);
        int32_t oy = pc * -rcx + pd * (y - rcy) + (y_tiles << 10);

        if (!affine && flip_x) {
            ox = (x_tiles * 8 - 1) << 8;
            pa = -0x100;
        }

        uint32_t tys = (disp_cnt.w & MAP_1D_FLAG) ? x_tiles * tsz : 1024; //Tile row stride

        uint32_t address = surf_addr + obj_x * BYTES_PER_PIXEL;

        if (obj_x < 0) {
            int32_t minus_obj_x = -obj_x;

            x        = minus_obj_x;
            ox      += minus_obj_x * pa;
            oy      += minus_obj_x * pc;
            address += minus_obj_x * BYTES_PER_PIXEL;
        }

        for (; x < rcx * 2; x++, ox += pa, oy += pc, address += 4) {
            if (obj_x + x >= PIXELS_PER_LINE) break;

            uint32_t pixel = *(uint32_t *)(screen + address);

            if (pixel && eff != EFF_ALPHA_BLD) continue;

            uint32_t vram_addr;
            uint32_t pal_idx;

            uint16_t tile_x = ox >> 11;
            uint16_t tile_y = oy >> 11;

            if (ox < 0 || tile_x >= x_tiles) continue;
            if (oy < 0 || tile_y >= y_tiles) continue;

            uint16_t chr_x = (ox >> 8) & 7;
            uint16_t chr_y = (oy >> 8) & 7;

            uint32_t chr_addr = chr_base + tile_y * tys + chr_y * lsz;

            if (is_256) {
                vram_addr = chr_addr + tile_x * 64 + chr_x;

                pal_idx = vram[vram_addr & 0x17fff];
            } else {
                vram_addr = chr_addr + tile_x * 32 + (chr_x >> 1);

                pal_idx = (vram[vram_addr & 0x17fff] >> (chr_x & 1) * 4) & 0xf;
            }

            if (pal_idx) {
                if (obj_mode != OBJ_MODE_WIN) {
                    if (win) {
                        if (!win_show_x(LAYER_OBJ, x + obj_x, obj_win_y)) continue;

                        do_eff = win_show_x(LAYER_EFF, x + obj_x, eff_win_y);

                        if (!do_eff && pixel) continue;
                    }

                    uint32_t pal_addr = 0x100 | pal_idx | (!is_256 ? chr_pal * 16 : 0);

                    if (eff == EFF_NONE || !do_eff) {
                        *(uint32_t *)(screen + address) = palette[pal_addr];
                    } else if (eff == EFF_ALPHA_BLD) {
                        if (!pixel && ((bld_cnt.b.b0 & BLD_OBJ_MSK) || obj_mode == OBJ_MODE_BLD)) {
                            if (!layer1[x + obj_x])
                                 layer1[x + obj_x] = palette[pal_addr];
                        } else if (bld_cnt.b.b1 & BLD_OBJ_MSK) {
                            if (!layer2[x + obj_x])
                                 layer2[x + obj_x] = palette[pal_addr];
                        }

                        if (!pixel) *(uint32_t *)(screen + address) = palette[pal_addr];
                    } else if (eff == EFF_BRIGHT_INC && (bld_cnt.b.b0 & BLD_OBJ_MSK)) {
                        *(uint32_t *)(screen + address) = get_bright_inc_color(palette[pal_addr]);
                    } else if (eff == EFF_BRIGHT_DEC && (bld_cnt.b.b0 & BLD_OBJ_MSK)) {
                        *(uint32_t *)(screen + address) = get_bright_dec_color(palette[pal_addr]);
                    } else {
                        *(uint32_t *)(screen + address) = palette[pal_addr];
                    }
                } else {
                    layerobj[x + obj_x] = pal_idx;
                }
            }
        }
    }
}

static const uint8_t bg_enb[3] = { 0xf, 0x7, 0xc };

#define ORDER_OBJ_FLG  4

#define INSERT_ORDER(x)  (*order = (*order << 4) | (x))

static void fill_prio(uint32_t *order, uint8_t prio) {
    int8_t bg_idx;

    for (bg_idx = 3; bg_idx >= 0; bg_idx--) {
        //Pack bg index on integer according to the
        //order of rendering, lower = higher priority
        uint8_t bg_prio = bg[bg_idx].ctrl.w & 3;

        if (bg_prio == prio) {
            INSERT_ORDER(bg_idx);
        }
    }

    //Bit 2 set is used to indicate that a obj should be
    //rendered, this is always added after a bg layer is rendered.
    INSERT_ORDER(ORDER_OBJ_FLG | prio);
}

static void render_bg() {
    uint32_t address;
    uint8_t x;
    uint8_t mode = disp_cnt.w & 7;
    uint8_t enb = disp_cnt.b.b1 & bg_enb[mode];

    uint32_t surf_addr = v_count.w * PIXELS_PER_LINE * BYTES_PER_PIXEL;

    uint8_t eff = (bld_cnt.w >> 6) & 3;

    win_y_e eff_win_y = win_show_y(LAYER_EFF, v_count.w);

    bool do_eff = true;

    bool win = disp_cnt.w & WINANY_ENB;

    switch (mode) {
        case 0:
        case 1:
        case 2: {
            //This holds the indices of the bgs in order of rendering
            //on each nibble, and 0xf indicates end of list
            uint32_t order = 0;

            fill_prio(&order, 3);
            fill_prio(&order, 2);
            fill_prio(&order, 1);
            fill_prio(&order, 0);

            while (order) {
                uint32_t curr_order = order;

                order >>= 4;

                if (curr_order & ORDER_OBJ_FLG) {
                    render_obj(curr_order & 3);

                    continue;
                }

                uint8_t bg_idx = curr_order & 3;
                uint8_t bg_mask = 1 << bg_idx;

                if (!(enb & bg_mask)) continue;

                win_y_e bg_win_y = 0;

                if (win) {
                    bg_win_y = win_show_y(bg_idx, v_count.w);

                    if (!bg_win_y) continue;
                }

                uint32_t chr_base  = ((bg[bg_idx].ctrl.w >>  2) & 0x3)  << 14;
                bool     is_256    =  (bg[bg_idx].ctrl.w >>  7) & 0x1;
                uint16_t scrn_base = ((bg[bg_idx].ctrl.w >>  8) & 0x1f) << 11;
                bool     aff_wrap  =  (bg[bg_idx].ctrl.w >> 13) & 0x1;
                uint16_t scrn_size =  (bg[bg_idx].ctrl.w >> 14);

                bool affine = mode == 2 || (mode == 1 && bg_idx == 2);

                address = surf_addr;

                uint32_t px_count = 0;

                if (affine) {
                    int16_t pa = bg_pa[bg_idx].w;
                    int16_t pb = bg_pb[bg_idx].w;
                    int16_t pc = bg_pc[bg_idx].w;
                    int16_t pd = bg_pd[bg_idx].w;

                    int32_t ox = ((int32_t)bg_refxi[bg_idx].w << 4) >> 4;
                    int32_t oy = ((int32_t)bg_refyi[bg_idx].w << 4) >> 4;

                    bg_refxi[bg_idx].w += pb;
                    bg_refyi[bg_idx].w += pd;

                    uint8_t tms = 16 << scrn_size;
                    uint8_t tmsk = tms - 1;

                    for (x = 0; x < PIXELS_PER_LINE; x++, ox += pa, oy += pc, address += BYTES_PER_PIXEL) {
                        uint32_t pixel = *(uint32_t *)(screen + address);

                        if (win) {
                            if (!win_show_x(bg_idx, x, bg_win_y)) continue;

                            do_eff = win_show_x(LAYER_EFF, x, eff_win_y);

                            if (!do_eff && pixel) {
                                continue;

                                px_count++;
                            }
                        }

                        if (pixel && eff != EFF_ALPHA_BLD) {
                            px_count++;

                            continue;
                        }

                        int16_t tmx = ox >> 11;
                        int16_t tmy = oy >> 11;

                        if (aff_wrap) {
                            tmx &= tmsk;
                            tmy &= tmsk;
                        } else {
                            if (tmx < 0 || tmx >= tms) continue;
                            if (tmy < 0 || tmy >= tms) continue;
                        }

                        uint16_t chr_x = (ox >> 8) & 7;
                        uint16_t chr_y = (oy >> 8) & 7;

                        uint32_t map_addr = scrn_base + tmy * tms + tmx;

                        uint32_t vram_addr = chr_base + vram[map_addr] * 64 + chr_y * 8 + chr_x;

                        uint8_t pal_idx = vram[vram_addr];

                        if (pal_idx) {
                            if (eff == EFF_NONE || !do_eff) {
                                *(uint32_t *)(screen + address) = palette[pal_idx];
                            } else if (eff == EFF_ALPHA_BLD) {
                                if (!pixel && (bld_cnt.b.b0 & bg_mask)) {
                                    if (!layer1[x])
                                         layer1[x] = palette[pal_idx];
                                } else if (bld_cnt.b.b1 & bg_mask) {
                                    if (!layer2[x])
                                         layer2[x] = palette[pal_idx];
                                }

                                if (!pixel) *(uint32_t *)(screen + address) = palette[pal_idx];
                            } else if (eff == EFF_BRIGHT_INC && (bld_cnt.b.b0 & bg_mask)) {
                                *(uint32_t *)(screen + address) = get_bright_inc_color(palette[pal_idx]);
                            } else if (eff == EFF_BRIGHT_DEC && (bld_cnt.b.b0 & bg_mask)) {
                                *(uint32_t *)(screen + address) = get_bright_dec_color(palette[pal_idx]);
                            } else {
                                *(uint32_t *)(screen + address) = palette[pal_idx];
                            }

                            px_count++;
                        }
                    }
                } else {
                    uint16_t oy = v_count.w + bg[bg_idx].yofs.w;

                    uint16_t tmy = oy >> 3;

                    uint16_t scrn_y = (tmy >> 5) & 1;

                    for (x = 0; x < PIXELS_PER_LINE; x++, address += BYTES_PER_PIXEL) {
                        uint32_t pixel = *(uint32_t *)(screen + address);

                        if (win) {
                            if (!win_show_x(bg_idx, x, bg_win_y)) continue;

                            do_eff = win_show_x(LAYER_EFF, x, eff_win_y);

                            if (!do_eff && pixel) {
                                continue;

                                px_count++;
                            }
                        }

                        if (pixel && eff != EFF_ALPHA_BLD) {
                            px_count++;

                            continue;
                        }

                        uint16_t ox = x + bg[bg_idx].xofs.w;

                        uint16_t tmx = ox >> 3;

                        uint16_t scrn_x = (tmx >> 5) & 1;

                        uint16_t chr_x = ox & 7;
                        uint16_t chr_y = oy & 7;

                        uint16_t pal_idx;
                        uint16_t pal_base = 0;

                        uint32_t map_addr = scrn_base + (tmy & 0x1f) * 32 * 2 + (tmx & 0x1f) * 2;

                        switch (scrn_size) {
                            case 1: map_addr += scrn_x * 2048; break;
                            case 2: map_addr += scrn_y * 2048; break;
                            case 3: map_addr += scrn_x * 2048 + scrn_y * 4096; break;
                        }

                        uint16_t tile = *(uint16_t *)(vram + map_addr);

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

                            pal_idx = vram[vram_addr];
                        } else {
                            vram_addr = chr_base + chr_numb * 32 + chr_y * 4 + (chr_x >> 1);

                            pal_idx = (vram[vram_addr] >> (chr_x & 1) * 4) & 0xf;
                        }

                        uint32_t pal_addr = pal_base | pal_idx;

                        if (pal_idx) {
                            if (eff == EFF_NONE || !do_eff) {
                                *(uint32_t *)(screen + address) = palette[pal_addr];
                            } else if (eff == EFF_ALPHA_BLD) {
                                if (!pixel && (bld_cnt.b.b0 & bg_mask)) {
                                    if (!layer1[x])
                                         layer1[x] = palette[pal_addr];
                                } else if (bld_cnt.b.b1 & bg_mask) {
                                    if (!layer2[x])
                                         layer2[x] = palette[pal_addr];
                                }

                                if (!pixel) *(uint32_t *)(screen + address) = palette[pal_addr];
                            } else if (eff == EFF_BRIGHT_INC && (bld_cnt.b.b0 & bg_mask)) {
                                *(uint32_t *)(screen + address) = get_bright_inc_color(palette[pal_addr]);
                            } else if (eff == EFF_BRIGHT_DEC && (bld_cnt.b.b0 & bg_mask)) {
                                *(uint32_t *)(screen + address) = get_bright_dec_color(palette[pal_addr]);
                            } else {
                                *(uint32_t *)(screen + address) = palette[pal_idx];
                            }

                            px_count++;
                        }
                    }
                }

                //If the pixels count equals the number of pixels per line,
                //then we have no more transparent pixels on this line
                if (px_count == PIXELS_PER_LINE && eff != EFF_ALPHA_BLD) return;
            }
        }
        break;

        case 3: {
            address = surf_addr;

            uint32_t frm_addr = v_count.w * 480;

            win_y_e win_y = win_show_y(LAYER_BG2, v_count.w);

            for (x = 0; x < PIXELS_PER_LINE; x++, address += 4) {
                uint32_t pixel = *(uint32_t *)(screen + address);

                if (pixel) continue;

                if (win) {
                    if (!win_show_x(LAYER_BG2, x, win_y)) continue;

                    do_eff = win_show_x(LAYER_EFF, x, eff_win_y);
                }

                uint16_t packed = *(uint16_t *)(vram + frm_addr * 2);

                uint8_t r = ((packed >>  0) & 0x1f) << 3;
                uint8_t g = ((packed >>  5) & 0x1f) << 3;
                uint8_t b = ((packed >> 10) & 0x1f) << 3;

                uint32_t rgba = 0xff;

                rgba |= (r | (r >> 5)) <<  8;
                rgba |= (g | (g >> 5)) << 16;
                rgba |= (b | (b >> 5)) << 24;

                if (eff == EFF_NONE || !do_eff) {
                    *(uint32_t *)(screen + address) = rgba;
                } else if (eff == EFF_ALPHA_BLD) {
                    if (!pixel && (bld_cnt.b.b0 & BLD_BG2_MSK)) {
                        if (!layer1[x])
                             layer1[x] = rgba;
                    }

                    if (!pixel) *(uint32_t *)(screen + address) = rgba;
                } else if (eff == EFF_BRIGHT_INC && (bld_cnt.b.b0 & BLD_BG2_MSK)) {
                    *(uint32_t *)(screen + address) = get_bright_inc_color(rgba);
                } else if (eff == EFF_BRIGHT_DEC && (bld_cnt.b.b0 & BLD_BG2_MSK)) {
                    *(uint32_t *)(screen + address) = get_bright_dec_color(rgba);
                } else {
                    *(uint32_t *)(screen + address) = rgba;
                }
            }
        }
        break;

        case 4: {
            address = surf_addr;

            uint8_t frame = (disp_cnt.w >> 4) & 1;

            uint32_t frm_addr = 0xa000 * frame + v_count.w * 240;

            win_y_e win_y = win_show_y(LAYER_BG2, v_count.w);

            for (x = 0; x < PIXELS_PER_LINE; x++, address += 4) {
                uint32_t pixel = *(uint32_t *)(screen + address);

                if (pixel) continue;

                if (win) {
                    if (!win_show_x(LAYER_BG2, x, win_y)) continue;

                    do_eff = win_show_x(LAYER_EFF, x, eff_win_y);
                }

                uint8_t pal_idx = vram[frm_addr + x];

                if (eff == EFF_NONE || !do_eff) {
                    *(uint32_t *)(screen + address) = palette[pal_idx];
                } else if (eff == EFF_ALPHA_BLD) {
                    if (!pixel && (bld_cnt.b.b0 & BLD_BG2_MSK)) {
                        if (!layer1[x])
                             layer1[x] = palette[pal_idx];
                    }

                    if (!pixel) *(uint32_t *)(screen + address) = palette[pal_idx];
                } else if (eff == EFF_BRIGHT_INC && (bld_cnt.b.b0 & BLD_BG2_MSK)) {
                    *(uint32_t *)(screen + address) = get_bright_inc_color(palette[pal_idx]);
                } else if (eff == EFF_BRIGHT_DEC && (bld_cnt.b.b0 & BLD_BG2_MSK)) {
                    *(uint32_t *)(screen + address) = get_bright_dec_color(palette[pal_idx]);
                } else {
                    *(uint32_t *)(screen + address) = palette[pal_idx];
                }
            }
        }
        break;
    }

    //Fill the portions of the screen that are still transparent
    //with the backdrop color
    address = surf_addr;

    uint32_t bd_color = palette[0];

    if (bld_cnt.b.b0 & BLD_BD_MSK) {
        if (eff == EFF_BRIGHT_INC) {
            bd_color = get_bright_inc_color(bd_color);
        } else if (eff == EFF_BRIGHT_DEC) {
            bd_color = get_bright_dec_color(bd_color);
        }
    }

    for (x = 0; x < PIXELS_PER_LINE; x++, address += 4) {
        if (eff == EFF_ALPHA_BLD) {
            uint64_t col1 = layer1[x];
            uint64_t col2 = layer2[x];

            if (!col1 && (bld_cnt.b.b0 & BLD_BD_MSK)) {
                 col1 = palette[0];
            }

            if (!col2 && (bld_cnt.b.b1 & BLD_BD_MSK)) {
                 col2 = palette[0];
            }

            //Alpha blending only occurs if both color are non-transparent,
            //otherwise it is just rendered as normal
            if (col1 && col2) {
                uint32_t eva = MIN((bld_alpha.w >> 0) & 0x1f, 16);
                uint32_t evb = MIN((bld_alpha.w >> 8) & 0x1f, 16);

                uint64_t mix_col =
                    MIN((MUL_COEFF(col1 & mask_r, eva) & mask_r) +
                        (MUL_COEFF(col2 & mask_r, evb) & mask_r), mask_r) |
                    MIN((MUL_COEFF(col1 & mask_g, eva) & mask_g) +
                        (MUL_COEFF(col2 & mask_g, evb) & mask_g), mask_g) |
                    MIN((MUL_COEFF(col1 & mask_b, eva) & mask_b) +
                        (MUL_COEFF(col2 & mask_b, evb) & mask_b), mask_b) | 0xff;

                *(uint32_t *)(screen + address) = (uint32_t)mix_col;

                continue;
            }
        }

        if (!*(uint32_t *)(screen + address))
             *(uint32_t *)(screen + address) = bd_color;
    }
}

static void fill_oam_lut() {
    oam_counts[0] = 0;
    oam_counts[1] = 0;
    oam_counts[2] = 0;
    oam_counts[3] = 0;
    oam_counts[4] = 0;

    if (!(disp_cnt.w & OBJ_ENB)) return;

    uint8_t obj_index;

    for (obj_index = 0; obj_index < 128; obj_index++) {
        uint64_t attr = *(uint64_t *)(oam + obj_index * 8);

        int16_t obj_y    = (attr >>  0) & 0xff;
        uint8_t obj_mode = (attr >> 10) & 0x3;
        uint8_t obj_shp  = (attr >> 14) & 3;
        uint8_t obj_size = (attr >> 30) & 3;
        uint8_t chr_prio = (attr >> 42) & 3;

        uint32_t aff_flg = attr & 0x300;

        bool hidden = aff_flg == 0x200;

        if (hidden && obj_mode != OBJ_MODE_WIN) continue;

        bool dbl_size = aff_flg == 0x300;

        uint8_t lut_idx = obj_size | (obj_shp << 2);

        uint8_t x_tiles = x_tiles_lut[lut_idx];
        uint8_t y_tiles = y_tiles_lut[lut_idx];

        int32_t rcx = x_tiles * 4;
        int32_t rcy = y_tiles * 4;

        if (dbl_size) {
            rcx *= 2;
            rcy *= 2;
        }

        int16_t obj_h = rcy * 2;

        if (obj_y + obj_h > 0xff) obj_y -= 0x100;

        if ((uint32_t)(v_count.w - obj_y) <= obj_h) {
            if (obj_mode == OBJ_MODE_WIN) {
                chr_prio = OAM_LUT_WINOBJ;
            }

            lut_idx = oam_counts[chr_prio]++;

            oam_attrs[chr_prio][lut_idx] = attr;
        }
    }
}

static void render_line() {
    uint32_t line_offset = v_count.w * PIXELS_PER_LINE * BYTES_PER_PIXEL;

    memset(screen + line_offset, 0, PIXELS_PER_LINE * BYTES_PER_PIXEL);

    memset(layer1, 0, PIXELS_PER_LINE * sizeof(uint32_t));
    memset(layer2, 0, PIXELS_PER_LINE * sizeof(uint32_t));

    memset(layerobj, 0, PIXELS_PER_LINE * sizeof(uint8_t));

    fill_oam_lut();

    render_obj(OAM_LUT_WINOBJ);

    if ((disp_cnt.w & 7) > 2) {
        render_obj(0);
        render_obj(1);
        render_obj(2);
        render_obj(3);
        render_bg(0);
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
            bg_refxi[2].w = bg_refxe[2].w;
            bg_refyi[2].w = bg_refye[2].w;

            bg_refxi[3].w = bg_refxe[3].w;
            bg_refyi[3].w = bg_refye[3].w;

            vblank_start();
            dma_transfer(VBLANK);
        }

        arm_exec(CYC_LINE_HBLK0);

        //H-Blank start
        if (v_count.w < LINES_VISIBLE) {
            render_line();
            dma_transfer(HBLANK);
        }

        hblank_start();

        arm_exec(CYC_LINE_HBLK1);

        sound_clock(CYC_LINE_TOTAL);
    }

    SDL_UnlockTexture(texture);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    sound_buffer_wrap();
}