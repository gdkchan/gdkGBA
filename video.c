#include "arm.h"
#include "arm_mem.h"

#include "dma.h"
#include "io.h"
#include "sdl.h"
#include "sound.h"

#define LINES_VISIBLE  160
#define LINES_TOTAL    228

#define CYC_LINE_TOTAL  1232
#define CYC_LINE_HBLK0  1006
#define CYC_LINE_HBLK1  (CYC_LINE_TOTAL - CYC_LINE_HBLK0)

void *screen;

static const uint8_t x_tiles_lut[16] = { 1, 2, 4, 8, 2, 4, 4, 8, 1, 1, 2, 4, 0, 0, 0, 0 };
static const uint8_t y_tiles_lut[16] = { 1, 2, 4, 8, 1, 1, 2, 4, 2, 4, 4, 8, 0, 0, 0, 0 };

static bool win_show_x(uint8_t layer, uint8_t x) {
	uint8_t mask = 1 << layer;

	bool win0_in = win_in.b.b0 & mask;
	bool win1_in = win_in.b.b1 & mask;

	bool win_out_ = win_out.b.b0 & mask;

	bool inside_win0 = false;
	bool inside_win1 = false;

	if (disp_cnt.w & WIN0_ENB) {
		inside_win0 = x >= win0_h.b.b1 &&
					  x <  win0_h.b.b0;
	}

	if (disp_cnt.w & WIN1_ENB) {
		inside_win1 = x >= win1_h.b.b1 &&
					  x <  win1_h.b.b0;
	}

	if (win0_in && inside_win0) return true;
	if (win1_in && inside_win1) return true;

	if (win_out_ && !(inside_win0 || inside_win1)) return true;

	return false;
}

static bool win_show_y(uint8_t layer, uint8_t y) {
	uint8_t mask = 1 << layer;

	bool win0_in = win_in.b.b0 & mask;
	bool win1_in = win_in.b.b1 & mask;

	bool win_out_ = win_out.b.b0 & mask;

	bool inside_win0 = false;
	bool inside_win1 = false;

	if (disp_cnt.w & WIN0_ENB) {
		inside_win0 = y >= win0_v.b.b1 &&
					  y <  win0_v.b.b0;
	}

	if (disp_cnt.w & WIN1_ENB) {
		inside_win1 = y >= win1_v.b.b1 &&
					  y <  win1_v.b.b0;
	}

	if (win0_in && inside_win0) return true;
	if (win1_in && inside_win1) return true;

	if (win_out_ && !(inside_win0 || inside_win1)) return true;

	return false;
}

#define MIN(x, y)  ((x) > (y) ? (y) : (x))

static uint32_t get_blended_color(uint32_t new_col, uint32_t old_col) {
	uint8_t eff = (bld_cnt.w >> 6) & 3;

	uint8_t nr = (uint8_t)(new_col >>  8);
	uint8_t ng = (uint8_t)(new_col >> 16);
	uint8_t nb = (uint8_t)(new_col >> 24);

	uint32_t mr, mg, mb;

	switch (eff) {
		case 1: {
			uint32_t eva = MIN((bld_alpha.w >> 0) & 0x1f, 16);
			uint32_t evb = MIN((bld_alpha.w >> 8) & 0x1f, 16);

			uint8_t or = (uint8_t)(old_col >>  8);
			uint8_t og = (uint8_t)(old_col >> 16);
			uint8_t ob = (uint8_t)(old_col >> 24);

			float evaf = eva * (1.0f / 16.0f);
			float evbf = evb * (1.0f / 16.0f);

			mr = (uint32_t)(MIN(nr * evaf + or * evbf, 0xff));
			mg = (uint32_t)(MIN(ng * evaf + og * evbf, 0xff));
			mb = (uint32_t)(MIN(nb * evaf + ob * evbf, 0xff));
		}
		break;

		case 2: {
			uint32_t evy = MIN(bld_bright.w & 0x1f, 16);

			float evyf = evy * (1.0f / 16.0f);

			mr = (uint32_t)(nr + (0xff - nr) * evyf);
			mg = (uint32_t)(ng + (0xff - ng) * evyf);
			mb = (uint32_t)(nb + (0xff - nb) * evyf);
		}
		break;

		case 3: {
			uint32_t evy = MIN(bld_bright.w & 0x1f, 16);

			float evyf = evy * (1.0f / 16.0f);

			mr = (uint32_t)(nr - nr * evyf);
			mg = (uint32_t)(ng - ng * evyf);
			mb = (uint32_t)(nb - nb * evyf);
		}
		break;

		default: return new_col;
	}

	return 0xff | (mr << 8) | (mg << 16) | (mb << 24);
}

static void render_obj(uint8_t prio) {
    if (!(disp_cnt.w & OBJ_ENB)) return;

    uint8_t obj_index;
    uint32_t offset = 0x3f8;

    uint32_t surf_addr = v_count.w * 240 * 4;

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

        pa = pd = 0x100; //1.0
        pb = pc = 0x000; //0.0

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

            uint32_t chr_base = 0x10000 | chr_numb * 32;

            obj_x <<= 7;
            obj_x >>= 7;

            int32_t x, y = v_count.w - obj_y;

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

            uint32_t address = surf_addr + obj_x * 4;

            for (x = 0; x < rcx * 2;
                x++,
                ox += pa,
                oy += pc,
                address += 4) {
                if (obj_x + x < 0) continue;
                if (obj_x + x >= 240) break;

                uint32_t vram_addr;
                uint32_t pal_idx;

                uint16_t tile_x = ox >> 11;
                uint16_t tile_y = oy >> 11;

                if (ox < 0 || tile_x >= x_tiles) continue;
                if (oy < 0 || tile_y >= y_tiles) continue;

                uint16_t chr_x = (ox >> 8) & 7;
                uint16_t chr_y = (oy >> 8) & 7;

                uint32_t chr_addr =
                    chr_base       +
                    tile_y   * tys +
                    chr_y    * lsz;

                if (is_256) {
                    vram_addr = chr_addr + tile_x * 64 + chr_x;
                    pal_idx   = vram[vram_addr];
                } else {
                    vram_addr = chr_addr + tile_x * 32 + (chr_x >> 1);
                    pal_idx   = (vram[vram_addr] >> (chr_x & 1) * 4) & 0xf;
                }

                uint32_t pal_addr = 0x100 | pal_idx | (!is_256 ? chr_pal * 16 : 0);

                if (pal_idx) *(uint32_t *)(screen + address) = palette[pal_addr];
            }
        }
    }
}

static const uint8_t bg_enb[3] = { 0xf, 0x7, 0xc };

static void render_bg() {
    uint8_t mode = disp_cnt.w & 7;

    uint32_t surf_addr = v_count.w * 240 * 4;

	uint8_t eff = (bld_cnt.w >> 6) & 3;

	bool win = disp_cnt.w & 0xe000;

    switch (mode) {
        case 0:
        case 1:
        case 2: {
            uint8_t enb = (disp_cnt.w >> 8) & bg_enb[mode];

            int8_t prio, bg_idx;

            for (prio = 3; prio >= 0; prio--) {
                for (bg_idx = 3; bg_idx >= 0; bg_idx--) {
					int8_t bg_mask = 1 << bg_idx;

                    if (!(enb & bg_mask)) continue;

                    if ((bg[bg_idx].ctrl.w & 3) != prio) continue;

					if (win && !win_show_y(bg_idx, v_count.w)) continue;

                    uint32_t chr_base  = ((bg[bg_idx].ctrl.w >>  2) & 0x3)  << 14;
                    bool     is_256    =  (bg[bg_idx].ctrl.w >>  7) & 0x1;
                    uint16_t scrn_base = ((bg[bg_idx].ctrl.w >>  8) & 0x1f) << 11;
                    bool     aff_wrap  =  (bg[bg_idx].ctrl.w >> 13) & 0x1;
                    uint16_t scrn_size =  (bg[bg_idx].ctrl.w >> 14);

                    bool affine = mode == 2 || (mode == 1 && bg_idx == 2);

					bool blend = eff && (bld_cnt.w & bg_mask);

                    uint32_t address = surf_addr;

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

                        uint8_t x;

                        for (x = 0; x < 240; x++, ox += pa, oy += pc, address += 4) {
							if (win && !win_show_x(bg_idx, x)) continue;

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

                            uint16_t pal_idx = vram[vram_addr];

                            if (pal_idx) {
								if (blend) {
									uint32_t color = get_blended_color(palette[pal_idx], *(uint32_t *)(screen + address));

									*(uint32_t *)(screen + address) = color;
								} else {
									*(uint32_t *)(screen + address) = palette[pal_idx];
								}
							}
                        }
                    } else {
                        uint16_t oy     = v_count.w + bg[bg_idx].yofs.w;
                        uint16_t tmy    = oy >> 3;
                        uint16_t scrn_y = (tmy >> 5) & 1;

                        uint8_t x;

                        for (x = 0; x < 240; x++) {
							if (win && !win_show_x(bg_idx, x)) continue;

                            uint16_t ox     = x + bg[bg_idx].xofs.w;
                            uint16_t tmx    = ox >> 3;
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

                            uint32_t pal_addr = pal_idx | pal_base;

                            if (pal_idx) {
								if (blend) {
									uint32_t color = get_blended_color(palette[pal_addr], *(uint32_t *)(screen + address));

									*(uint32_t *)(screen + address) = color;
								} else {
									*(uint32_t *)(screen + address) = palette[pal_addr];
								}
							}

                            address += 4;
                        }
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