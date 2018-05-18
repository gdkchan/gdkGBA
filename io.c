#include "arm.h"

#include "dma.h"
#include "io.h"
#include "sound.h"
#include "timer.h"

uint8_t io_read(uint32_t address) {
    io_open_bus = false;

    switch (address) {
        case 0x04000000: return disp_cnt.b.b0        & 0xff;
        case 0x04000001: return disp_cnt.b.b1        & 0xff;
        case 0x04000002: return green_inv.b.b0       & 0x01;
        case 0x04000003: return green_inv.b.b1       & 0x00;
        case 0x04000004: return disp_stat.b.b0       & 0xff;
        case 0x04000005: return disp_stat.b.b1       & 0xff;
        case 0x04000006: return v_count.b.b0         & 0xff;
        case 0x04000007: return v_count.b.b1         & 0x00;

        case 0x04000008: return bg[0].ctrl.b.b0      & 0xff;
        case 0x04000009: return bg[0].ctrl.b.b1      & 0xdf;
        case 0x0400000a: return bg[1].ctrl.b.b0      & 0xff;
        case 0x0400000b: return bg[1].ctrl.b.b1      & 0xdf;
        case 0x0400000c: return bg[2].ctrl.b.b0      & 0xff;
        case 0x0400000d: return bg[2].ctrl.b.b1      & 0xff;
        case 0x0400000e: return bg[3].ctrl.b.b0      & 0xff;
        case 0x0400000f: return bg[3].ctrl.b.b1      & 0xff;

        case 0x04000048: return win_in.b.b0          & 0x3f;
        case 0x04000049: return win_in.b.b1          & 0x3f;
        case 0x0400004a: return win_out.b.b0         & 0x3f;
        case 0x0400004b: return win_out.b.b1         & 0x3f;

        case 0x04000050: return bld_cnt.b.b0         & 0xff;
        case 0x04000051: return bld_cnt.b.b1         & 0x3f;
        case 0x04000052: return bld_alpha.b.b0       & 0x1f;
        case 0x04000053: return bld_alpha.b.b1       & 0x1f;

        case 0x04000060: return sqr_ch[0].sweep.b.b0 & 0x7f;
        case 0x04000061: return sqr_ch[0].sweep.b.b1 & 0x00;
        case 0x04000062: return sqr_ch[0].tone.b.b0  & 0xc0;
        case 0x04000063: return sqr_ch[0].tone.b.b1  & 0xff;
        case 0x04000064: return sqr_ch[0].ctrl.b.b0  & 0x00;
        case 0x04000065: return sqr_ch[0].ctrl.b.b1  & 0x40;
        case 0x04000066: return sqr_ch[0].ctrl.b.b2  & 0x00;
        case 0x04000067: return sqr_ch[0].ctrl.b.b3  & 0x00;

        case 0x04000068: return sqr_ch[1].tone.b.b0  & 0xc0;
        case 0x04000069: return sqr_ch[1].tone.b.b1  & 0xff;
        case 0x0400006c: return sqr_ch[1].ctrl.b.b0  & 0x00;
        case 0x0400006d: return sqr_ch[1].ctrl.b.b1  & 0x40;
        case 0x0400006e: return sqr_ch[1].ctrl.b.b2  & 0x00;
        case 0x0400006f: return sqr_ch[1].ctrl.b.b3  & 0x00;

        case 0x04000070: return wave_ch.wave.b.b0    & 0xe0;
        case 0x04000071: return wave_ch.wave.b.b1    & 0x00;
        case 0x04000072: return wave_ch.volume.b.b0  & 0x00;
        case 0x04000073: return wave_ch.volume.b.b1  & 0xe0;
        case 0x04000074: return wave_ch.ctrl.b.b0    & 0x00;
        case 0x04000075: return wave_ch.ctrl.b.b1    & 0x40;
        case 0x04000076: return wave_ch.ctrl.b.b2    & 0x00;
        case 0x04000077: return wave_ch.ctrl.b.b3    & 0x00;

        case 0x04000078: return noise_ch.env.b.b0    & 0x00;
        case 0x04000079: return noise_ch.env.b.b1    & 0xff;
        case 0x0400007a: return noise_ch.env.b.b2    & 0x00;
        case 0x0400007b: return noise_ch.env.b.b3    & 0x00;
        case 0x0400007c: return noise_ch.ctrl.b.b0   & 0xff;
        case 0x0400007d: return noise_ch.ctrl.b.b1   & 0x40;
        case 0x0400007e: return noise_ch.ctrl.b.b2   & 0x00;
        case 0x0400007f: return noise_ch.ctrl.b.b3   & 0x00;

        case 0x04000080: return snd_psg_vol.b.b0     & 0x77;
        case 0x04000081: return snd_psg_vol.b.b1     & 0xff;
        case 0x04000082: return snd_pcm_vol.b.b0     & 0x0f;
        case 0x04000083: return snd_pcm_vol.b.b1     & 0x77;
        case 0x04000084: return snd_psg_enb.b.b0     & 0x8f;
        case 0x04000085: return snd_psg_enb.b.b1     & 0x00;
        case 0x04000086: return snd_psg_enb.b.b2     & 0x00;
        case 0x04000087: return snd_psg_enb.b.b3     & 0x00;
        case 0x04000088: return snd_bias.b.b0        & 0xff;
        case 0x04000089: return snd_bias.b.b1        & 0xc3;
        case 0x0400008a: return snd_bias.b.b2        & 0x00;
        case 0x0400008b: return snd_bias.b.b3        & 0x00;

        case 0x04000090:
        case 0x04000091:
        case 0x04000092:
        case 0x04000093:
        case 0x04000094:
        case 0x04000095:
        case 0x04000096:
        case 0x04000097:
        case 0x04000098:
        case 0x04000099:
        case 0x0400009a:
        case 0x0400009b:
        case 0x0400009c:
        case 0x0400009d:
        case 0x0400009e:
        case 0x0400009f: {
            uint8_t wave_bank = (wave_ch.wave.w >> 2) & 0x10;
            uint8_t wave_idx  = (wave_bank ^ 0x10) | (address & 0xf);

            return wave_ram[wave_idx];
        }

        case 0x040000b8: return dma_ch[0].count.b.b0 & 0x00;
        case 0x040000b9: return dma_ch[0].count.b.b1 & 0x00;
        case 0x040000ba: return dma_ch[0].ctrl.b.b0  & 0xe0;
        case 0x040000bb: return dma_ch[0].ctrl.b.b1  & 0xf7;

        case 0x040000c4: return dma_ch[1].count.b.b0 & 0x00;
        case 0x040000c5: return dma_ch[1].count.b.b1 & 0x00;
        case 0x040000c6: return dma_ch[1].ctrl.b.b0  & 0xe0;
        case 0x040000c7: return dma_ch[1].ctrl.b.b1  & 0xf7;

        case 0x040000d0: return dma_ch[2].count.b.b0 & 0x00;
        case 0x040000d1: return dma_ch[2].count.b.b1 & 0x00;
        case 0x040000d2: return dma_ch[2].ctrl.b.b0  & 0xe0;
        case 0x040000d3: return dma_ch[2].ctrl.b.b1  & 0xf7;

        case 0x040000dc: return dma_ch[3].count.b.b0 & 0x00;
        case 0x040000dd: return dma_ch[3].count.b.b1 & 0x00;
        case 0x040000de: return dma_ch[3].ctrl.b.b0  & 0xe0;
        case 0x040000df: return dma_ch[3].ctrl.b.b1  & 0xff;

        case 0x04000100: return tmr[0].count.b.b0    & 0xff;
        case 0x04000101: return tmr[0].count.b.b1    & 0xff;
        case 0x04000102: return tmr[0].ctrl.b.b0     & 0xc7;
        case 0x04000103: return tmr[0].ctrl.b.b1     & 0x00;

        case 0x04000104: return tmr[1].count.b.b0    & 0xff;
        case 0x04000105: return tmr[1].count.b.b1    & 0xff;
        case 0x04000106: return tmr[1].ctrl.b.b0     & 0xc7;
        case 0x04000107: return tmr[1].ctrl.b.b1     & 0x00;

        case 0x04000108: return tmr[2].count.b.b0    & 0xff;
        case 0x04000109: return tmr[2].count.b.b1    & 0xff;
        case 0x0400010a: return tmr[2].ctrl.b.b0     & 0xc7;
        case 0x0400010b: return tmr[2].ctrl.b.b1     & 0x00;

        case 0x0400010c: return tmr[3].count.b.b0    & 0xff;
        case 0x0400010d: return tmr[3].count.b.b1    & 0xff;
        case 0x0400010e: return tmr[3].ctrl.b.b0     & 0xc7;
        case 0x0400010f: return tmr[3].ctrl.b.b1     & 0x00;

        case 0x04000120: return sio_data32.b.b0      & 0xff;
        case 0x04000121: return sio_data32.b.b1      & 0xff;
        case 0x04000122: return sio_data32.b.b2      & 0xff;
        case 0x04000123: return sio_data32.b.b3      & 0xff;
        case 0x04000128: return sio_cnt.b.b0         & 0xff;
        case 0x04000129: return sio_cnt.b.b1         & 0xff;
        case 0x0400012a: return sio_data8.b.b0       & 0xff;
        case 0x04000134: return r_cnt.b.b0           & 0xff;
        case 0x04000135: return r_cnt.b.b1           & 0xff;

        case 0x04000130: return key_input.b.b0       & 0xff;
        case 0x04000131: return key_input.b.b1       & 0x3f;

        case 0x04000200: return int_enb.b.b0         & 0xff;
        case 0x04000201: return int_enb.b.b1         & 0x3f;
        case 0x04000202: return int_ack.b.b0         & 0xff;
        case 0x04000203: return int_ack.b.b1         & 0x3f;
        case 0x04000204: return wait_cnt.b.b0        & 0xff;
        case 0x04000205: return wait_cnt.b.b1        & 0xdf;
        case 0x04000206: return wait_cnt.b.b2        & 0x00;
        case 0x04000207: return wait_cnt.b.b3        & 0x00;
        case 0x04000208: return int_enb_m.b.b0       & 0x01;
        case 0x04000209: return int_enb_m.b.b1       & 0x00;
        case 0x0400020a: return int_enb_m.b.b2       & 0x00;
        case 0x0400020b: return int_enb_m.b.b3       & 0x00;

        case 0x04000300: return post_boot            & 0x01;
        case 0x04000301: return int_halt             & 0x00;
    }

    io_open_bus = true;

    return 0;
}

static void dma_load(uint8_t ch, uint8_t value) {
    uint8_t old = dma_ch[ch].ctrl.b.b1;

    dma_ch[ch].ctrl.b.b1 = value;

    if ((old ^ value) & value & 0x80) {
        dma_dst_addr[ch] = dma_ch[ch].dst.w;
        dma_src_addr[ch] = dma_ch[ch].src.w;

        if (dma_ch[ch].ctrl.w & DMA_32) {
            dma_dst_addr[ch] &= ~3;
            dma_src_addr[ch] &= ~3;
        } else {
            dma_dst_addr[ch] &= ~1;
            dma_src_addr[ch] &= ~1;
        }

        dma_count[ch] = dma_ch[ch].count.w;

        dma_transfer(IMMEDIATELY);
    }
}

static void tmr_load(uint8_t idx, uint8_t value) {
    uint8_t old = tmr[idx].ctrl.b.b0;

    tmr[idx].ctrl.b.b0 = value;

    if (value & TMR_ENB)
        tmr_enb |=  (1 << idx);
    else
        tmr_enb &= ~(1 << idx);

    if ((old ^ value) & value & TMR_ENB) {
        tmr[idx].count.w = tmr[idx].reload.w;

        tmr_icnt[idx] = 0;
    }
}

static void snd_reset_state(uint8_t ch, bool enb) {
    if (enb) {
        snd_ch_state[ch].phase       = false;
        snd_ch_state[ch].samples     = 0;
        snd_ch_state[ch].length_time = 0;
        snd_ch_state[ch].sweep_time  = 0;
        snd_ch_state[ch].env_time    = 0;

        if (ch == 2) wave_reset();

        if (ch == 3) {
            if (noise_ch.ctrl.w & NOISE_7)
                snd_ch_state[ch].lfsr = 0x007f;
            else
                snd_ch_state[ch].lfsr = 0x7fff;
        }

        snd_psg_enb.w |=  (1 << ch);
    }
}

void io_write(uint32_t address, uint8_t value) {
    switch (address) {
        case 0x04000000: disp_cnt.b.b0        =  value; break;
        case 0x04000001: disp_cnt.b.b1        =  value; break;
        case 0x04000002: green_inv.b.b0       =  value; break;
        case 0x04000003: green_inv.b.b1       =  value; break;
        case 0x04000004:
            disp_stat.b.b0 &=          0x47;
            disp_stat.b.b0 |= value & ~0x47;
        break;
        case 0x04000005: disp_stat.b.b1       =  value; break;

        case 0x04000008: bg[0].ctrl.b.b0      =  value; break;
        case 0x04000009: bg[0].ctrl.b.b1      =  value; break;
        case 0x0400000a: bg[1].ctrl.b.b0      =  value; break;
        case 0x0400000b: bg[1].ctrl.b.b1      =  value; break;
        case 0x0400000c: bg[2].ctrl.b.b0      =  value; break;
        case 0x0400000d: bg[2].ctrl.b.b1      =  value; break;
        case 0x0400000e: bg[3].ctrl.b.b0      =  value; break;
        case 0x0400000f: bg[3].ctrl.b.b1      =  value; break;

        case 0x04000010: bg[0].xofs.b.b0      =  value; break;
        case 0x04000011: bg[0].xofs.b.b1      =  value; break;
        case 0x04000012: bg[0].yofs.b.b0      =  value; break;
        case 0x04000013: bg[0].yofs.b.b1      =  value; break;
        case 0x04000014: bg[1].xofs.b.b0      =  value; break;
        case 0x04000015: bg[1].xofs.b.b1      =  value; break;
        case 0x04000016: bg[1].yofs.b.b0      =  value; break;
        case 0x04000017: bg[1].yofs.b.b1      =  value; break;
        case 0x04000018: bg[2].xofs.b.b0      =  value; break;
        case 0x04000019: bg[2].xofs.b.b1      =  value; break;
        case 0x0400001a: bg[2].yofs.b.b0      =  value; break;
        case 0x0400001b: bg[2].yofs.b.b1      =  value; break;
        case 0x0400001c: bg[3].xofs.b.b0      =  value; break;
        case 0x0400001d: bg[3].xofs.b.b1      =  value; break;
        case 0x0400001e: bg[3].yofs.b.b0      =  value; break;
        case 0x0400001f: bg[3].yofs.b.b1      =  value; break;

        case 0x04000020: bg_pa[2].b.b0        =  value; break;
        case 0x04000021: bg_pa[2].b.b1        =  value; break;
        case 0x04000022: bg_pb[2].b.b0        =  value; break;
        case 0x04000023: bg_pb[2].b.b1        =  value; break;
        case 0x04000024: bg_pc[2].b.b0        =  value; break;
        case 0x04000025: bg_pc[2].b.b1        =  value; break;
        case 0x04000026: bg_pd[2].b.b0        =  value; break;
        case 0x04000027: bg_pd[2].b.b1        =  value; break;

        case 0x04000028:
            bg_refxe[2].b.b0 =
            bg_refxi[2].b.b0 = value;
        break;
        case 0x04000029:
            bg_refxe[2].b.b1 =
            bg_refxi[2].b.b1 = value;
        break;
        case 0x0400002a:
            bg_refxe[2].b.b2 =
            bg_refxi[2].b.b2 = value;
        break;
        case 0x0400002b:
            bg_refxe[2].b.b3 =
            bg_refxi[2].b.b3 = value;
        break;
        case 0x0400002c:
            bg_refye[2].b.b0 =
            bg_refyi[2].b.b0 = value;
        break;
        case 0x0400002d:
            bg_refye[2].b.b1 =
            bg_refyi[2].b.b1 = value;
        break;
        case 0x0400002e:
            bg_refye[2].b.b2 =
            bg_refyi[2].b.b2 = value;
        break;
        case 0x0400002f:
            bg_refye[2].b.b3 =
            bg_refyi[2].b.b3 = value;
        break;

        case 0x04000030: bg_pa[3].b.b0        =  value; break;
        case 0x04000031: bg_pa[3].b.b1        =  value; break;
        case 0x04000032: bg_pb[3].b.b0        =  value; break;
        case 0x04000033: bg_pb[3].b.b1        =  value; break;
        case 0x04000034: bg_pc[3].b.b0        =  value; break;
        case 0x04000035: bg_pc[3].b.b1        =  value; break;
        case 0x04000036: bg_pd[3].b.b0        =  value; break;
        case 0x04000037: bg_pd[3].b.b1        =  value; break;

        case 0x04000038:
            bg_refxe[3].b.b0 =
            bg_refxi[3].b.b0 = value;
        break;
        case 0x04000039:
            bg_refxe[3].b.b1 =
            bg_refxi[3].b.b1 = value;
        break;
        case 0x0400003a:
            bg_refxe[3].b.b2 =
            bg_refxi[3].b.b2 = value;
        break;
        case 0x0400003b:
            bg_refxe[3].b.b3 =
            bg_refxi[3].b.b3 = value;
        break;
        case 0x0400003c:
            bg_refye[3].b.b0 =
            bg_refyi[3].b.b0 = value;
        break;
        case 0x0400003d:
            bg_refye[3].b.b1 =
            bg_refyi[3].b.b1 = value;
        break;
        case 0x0400003e:
            bg_refye[3].b.b2 =
            bg_refyi[3].b.b2 = value;
        break;
        case 0x0400003f:
            bg_refye[3].b.b3 =
            bg_refyi[3].b.b3 = value;
        break;

		case 0x04000040: win0_h.b.b0          =  value; break;
        case 0x04000041: win0_h.b.b1          =  value; break;
		case 0x04000042: win1_h.b.b0          =  value; break;
        case 0x04000043: win1_h.b.b1          =  value; break;

        case 0x04000044: win0_v.b.b0          =  value; break;
        case 0x04000045: win0_v.b.b1          =  value; break;
		case 0x04000046: win1_v.b.b0          =  value; break;
        case 0x04000047: win1_v.b.b1          =  value; break;

        case 0x04000048: win_in.b.b0          =  value; break;
        case 0x04000049: win_in.b.b1          =  value; break;
        case 0x0400004a: win_out.b.b0         =  value; break;
        case 0x0400004b: win_out.b.b1         =  value; break;

        case 0x04000050: bld_cnt.b.b0         =  value; break;
        case 0x04000051: bld_cnt.b.b1         =  value; break;
        case 0x04000052: bld_alpha.b.b0       =  value; break;
        case 0x04000053: bld_alpha.b.b1       =  value; break;
        case 0x04000054: bld_bright.b.b0      =  value; break;
        case 0x04000055: bld_bright.b.b1      =  value; break;
        case 0x04000056: bld_bright.b.b2      =  value; break;
        case 0x04000057: bld_bright.b.b3      =  value; break;

        case 0x04000060:
            if (snd_psg_enb.w & PSG_ENB)
                sqr_ch[0].sweep.b.b0 = value;
        break;
        case 0x04000061:
            if (snd_psg_enb.w & PSG_ENB)
                sqr_ch[0].sweep.b.b1 = value;
        break;
        case 0x04000062:
            if (snd_psg_enb.w & PSG_ENB)
                sqr_ch[0].tone.b.b0 = value;
        break;
        case 0x04000063:
            if (snd_psg_enb.w & PSG_ENB)
                sqr_ch[0].tone.b.b1 = value;
        break;
        case 0x04000064:
            if (snd_psg_enb.w & PSG_ENB)
                sqr_ch[0].ctrl.b.b0 = value;
        break;
        case 0x04000065:
            if (snd_psg_enb.w & PSG_ENB) {
                sqr_ch[0].ctrl.b.b1 = value;

                snd_reset_state(0, value & 0x80);
            }
        break;
        case 0x04000066:
            if (snd_psg_enb.w & PSG_ENB)
                sqr_ch[0].ctrl.b.b2 = value;
        break;
        case 0x04000067:
            if (snd_psg_enb.w & PSG_ENB)
                sqr_ch[0].ctrl.b.b3 = value;
        break;

        case 0x04000068:
            if (snd_psg_enb.w & PSG_ENB)
                sqr_ch[1].tone.b.b0 = value;
        break;
        case 0x04000069:
            if (snd_psg_enb.w & PSG_ENB)
                sqr_ch[1].tone.b.b1 = value;
        break;
        case 0x0400006c:
            if (snd_psg_enb.w & PSG_ENB)
                sqr_ch[1].ctrl.b.b0 = value;
        break;
        case 0x0400006d:
            if (snd_psg_enb.w & PSG_ENB) {
                sqr_ch[1].ctrl.b.b1 = value;

                snd_reset_state(1, value & 0x80);
            }
        break;
        case 0x0400006e:
            if (snd_psg_enb.w & PSG_ENB)
                sqr_ch[1].ctrl.b.b2 = value;
        break;
        case 0x0400006f:
            if (snd_psg_enb.w & PSG_ENB)
                sqr_ch[1].ctrl.b.b3 = value;
        break;

        case 0x04000070:
            if (snd_psg_enb.w & PSG_ENB)
                wave_ch.wave.b.b0 = value;
        break;
        case 0x04000071:
            if (snd_psg_enb.w & PSG_ENB)
                wave_ch.wave.b.b1 = value;
        break;
        case 0x04000072:
            if (snd_psg_enb.w & PSG_ENB)
                wave_ch.volume.b.b0 = value;
        break;
        case 0x04000073:
            if (snd_psg_enb.w & PSG_ENB)
                wave_ch.volume.b.b1 = value;
        break;
        case 0x04000074:
            if (snd_psg_enb.w & PSG_ENB)
                wave_ch.ctrl.b.b0 = value;
        break;
        case 0x04000075:
            if (snd_psg_enb.w & PSG_ENB) {
                wave_ch.ctrl.b.b1 = value;

                snd_reset_state(2, value & 0x80);
            }
        break;
        case 0x04000076:
            if (snd_psg_enb.w & PSG_ENB)
                wave_ch.ctrl.b.b2 = value;
        break;
        case 0x04000077:
            if (snd_psg_enb.w & PSG_ENB)
                wave_ch.ctrl.b.b3 = value;
        break;

        case 0x04000078:
            if (snd_psg_enb.w & PSG_ENB)
                noise_ch.env.b.b0 = value;
        break;
        case 0x04000079:
            if (snd_psg_enb.w & PSG_ENB)
                noise_ch.env.b.b1 = value;
        break;
        case 0x0400007a:
            if (snd_psg_enb.w & PSG_ENB)
                noise_ch.env.b.b2 = value;
        break;
        case 0x0400007b:
            if (snd_psg_enb.w & PSG_ENB)
                noise_ch.env.b.b3 = value;
        break;
        case 0x0400007c:
            if (snd_psg_enb.w & PSG_ENB)
                noise_ch.ctrl.b.b0 = value;
        break;
        case 0x0400007d:
            if (snd_psg_enb.w & PSG_ENB) {
                noise_ch.ctrl.b.b1 = value;

                snd_reset_state(3, value & 0x80);
            }
        break;
        case 0x0400007e:
            if (snd_psg_enb.w & PSG_ENB)
                noise_ch.ctrl.b.b2 = value;
        break;
        case 0x0400007f:
            if (snd_psg_enb.w & PSG_ENB)
                noise_ch.ctrl.b.b3 = value;
        break;

        case 0x04000080:
            if (snd_psg_enb.w & PSG_ENB)
                snd_psg_vol.b.b0 = value;
        break;
        case 0x04000081:
            if (snd_psg_enb.w & PSG_ENB)
                snd_psg_vol.b.b1 = value;
        break;
        case 0x04000082:
            //PCM is not affected by the PSG Enable flag
            snd_pcm_vol.b.b0 = value;
        break;
        case 0x04000083:
            snd_pcm_vol.b.b1 = value;

            if (value & 0x08) fifo_a_len = 0;
            if (value & 0x80) fifo_b_len = 0;
        break;
        case 0x04000084:
            snd_psg_enb.b.b0 &=          0xf;
            snd_psg_enb.b.b0 |= value & ~0xf;

            if (!(value & PSG_ENB)) {
                sqr_ch[0].sweep.w = 0;
                sqr_ch[0].tone.w  = 0;
                sqr_ch[0].ctrl.w  = 0;

                sqr_ch[1].tone.w  = 0;
                sqr_ch[1].ctrl.w  = 0;

                wave_ch.wave.w    = 0;
                wave_ch.volume.w  = 0;
                wave_ch.ctrl.w    = 0;

                noise_ch.env.w    = 0;
                noise_ch.ctrl.w   = 0;

                snd_psg_vol.w     = 0;
                snd_psg_enb.w     = 0;
            }
        break;
        case 0x04000085: snd_psg_enb.b.b1     =  value; break;
        case 0x04000086: snd_psg_enb.b.b2     =  value; break;
        case 0x04000087: snd_psg_enb.b.b3     =  value; break;
        case 0x04000088: snd_bias.b.b0        =  value; break;
        case 0x04000089: snd_bias.b.b1        =  value; break;
        case 0x0400008a: snd_bias.b.b2        =  value; break;
        case 0x0400008b: snd_bias.b.b3        =  value; break;

        case 0x04000090:
        case 0x04000091:
        case 0x04000092:
        case 0x04000093:
        case 0x04000094:
        case 0x04000095:
        case 0x04000096:
        case 0x04000097:
        case 0x04000098:
        case 0x04000099:
        case 0x0400009a:
        case 0x0400009b:
        case 0x0400009c:
        case 0x0400009d:
        case 0x0400009e:
        case 0x0400009f: {
            uint8_t wave_bank = (wave_ch.wave.w >> 2) & 0x10;
            uint8_t wave_idx  = (wave_bank ^ 0x10) | (address & 0xf);

            wave_ram[wave_idx] = value;
        }
        break;

        case 0x040000a0: snd_fifo_a_0         =  value; break;
        case 0x040000a1: snd_fifo_a_1         =  value; break;
        case 0x040000a2: snd_fifo_a_2         =  value; break;
        case 0x040000a3: snd_fifo_a_3         =  value; break;

        case 0x040000a4: snd_fifo_b_0         =  value; break;
        case 0x040000a5: snd_fifo_b_1         =  value; break;
        case 0x040000a6: snd_fifo_b_2         =  value; break;
        case 0x040000a7: snd_fifo_b_3         =  value; break;

        case 0x040000b0: dma_ch[0].src.b.b0   =  value; break;
        case 0x040000b1: dma_ch[0].src.b.b1   =  value; break;
        case 0x040000b2: dma_ch[0].src.b.b2   =  value; break;
        case 0x040000b3: dma_ch[0].src.b.b3   =  value; break;
        case 0x040000b4: dma_ch[0].dst.b.b0   =  value; break;
        case 0x040000b5: dma_ch[0].dst.b.b1   =  value; break;
        case 0x040000b6: dma_ch[0].dst.b.b2   =  value; break;
        case 0x040000b7: dma_ch[0].dst.b.b3   =  value; break;
        case 0x040000b8: dma_ch[0].count.b.b0 =  value; break;
        case 0x040000b9: dma_ch[0].count.b.b1 =  value; break;
        case 0x040000ba: dma_ch[0].ctrl.b.b0  =  value; break;
        case 0x040000bb: dma_load(0, value);            break;

        case 0x040000bc: dma_ch[1].src.b.b0   =  value; break;
        case 0x040000bd: dma_ch[1].src.b.b1   =  value; break;
        case 0x040000be: dma_ch[1].src.b.b2   =  value; break;
        case 0x040000bf: dma_ch[1].src.b.b3   =  value; break;
        case 0x040000c0: dma_ch[1].dst.b.b0   =  value; break;
        case 0x040000c1: dma_ch[1].dst.b.b1   =  value; break;
        case 0x040000c2: dma_ch[1].dst.b.b2   =  value; break;
        case 0x040000c3: dma_ch[1].dst.b.b3   =  value; break;
        case 0x040000c4: dma_ch[1].count.b.b0 =  value; break;
        case 0x040000c5: dma_ch[1].count.b.b1 =  value; break;
        case 0x040000c6: dma_ch[1].ctrl.b.b0  =  value; break;
        case 0x040000c7: dma_load(1, value);            break;

        case 0x040000c8: dma_ch[2].src.b.b0   =  value; break;
        case 0x040000c9: dma_ch[2].src.b.b1   =  value; break;
        case 0x040000ca: dma_ch[2].src.b.b2   =  value; break;
        case 0x040000cb: dma_ch[2].src.b.b3   =  value; break;
        case 0x040000cc: dma_ch[2].dst.b.b0   =  value; break;
        case 0x040000cd: dma_ch[2].dst.b.b1   =  value; break;
        case 0x040000ce: dma_ch[2].dst.b.b2   =  value; break;
        case 0x040000cf: dma_ch[2].dst.b.b3   =  value; break;
        case 0x040000d0: dma_ch[2].count.b.b0 =  value; break;
        case 0x040000d1: dma_ch[2].count.b.b1 =  value; break;
        case 0x040000d2: dma_ch[2].ctrl.b.b0  =  value; break;
        case 0x040000d3: dma_load(2, value);            break;

        case 0x040000d4: dma_ch[3].src.b.b0   =  value; break;
        case 0x040000d5: dma_ch[3].src.b.b1   =  value; break;
        case 0x040000d6: dma_ch[3].src.b.b2   =  value; break;
        case 0x040000d7: dma_ch[3].src.b.b3   =  value; break;
        case 0x040000d8: dma_ch[3].dst.b.b0   =  value; break;
        case 0x040000d9: dma_ch[3].dst.b.b1   =  value; break;
        case 0x040000da: dma_ch[3].dst.b.b2   =  value; break;
        case 0x040000db: dma_ch[3].dst.b.b3   =  value; break;
        case 0x040000dc: dma_ch[3].count.b.b0 =  value; break;
        case 0x040000dd: dma_ch[3].count.b.b1 =  value; break;
        case 0x040000de: dma_ch[3].ctrl.b.b0  =  value; break;
        case 0x040000df: dma_load(3, value);            break;

        case 0x04000100: tmr[0].reload.b.b0   =  value; break;
        case 0x04000101: tmr[0].reload.b.b1   =  value; break;
        case 0x04000102: tmr_load(0, value);            break;
        case 0x04000103: tmr[0].ctrl.b.b1     =  value; break;

        case 0x04000104: tmr[1].reload.b.b0   =  value; break;
        case 0x04000105: tmr[1].reload.b.b1   =  value; break;
        case 0x04000106: tmr_load(1, value);            break;
        case 0x04000107: tmr[1].ctrl.b.b1     =  value; break;

        case 0x04000108: tmr[2].reload.b.b0   =  value; break;
        case 0x04000109: tmr[2].reload.b.b1   =  value; break;
        case 0x0400010a: tmr_load(2, value);            break;
        case 0x0400010b: tmr[2].ctrl.b.b1     =  value; break;

        case 0x0400010c: tmr[3].reload.b.b0   =  value; break;
        case 0x0400010d: tmr[3].reload.b.b1   =  value; break;
        case 0x0400010e: tmr_load(3, value);            break;
        case 0x0400010f: tmr[3].ctrl.b.b1     =  value; break;

        case 0x04000120: sio_data32.b.b0      =  value; break;
        case 0x04000121: sio_data32.b.b1      =  value; break;
        case 0x04000122: sio_data32.b.b2      =  value; break;
        case 0x04000123: sio_data32.b.b3      =  value; break;
        case 0x04000128: sio_cnt.b.b0         =  value; break;
        case 0x04000129: sio_cnt.b.b1         =  value; break;
        case 0x0400012a: sio_data8.b.b0       =  value; break;
        case 0x04000134: r_cnt.b.b0           =  value; break;
        case 0x04000135: r_cnt.b.b1           =  value; break;

        case 0x04000200:
            int_enb.b.b0 = value;
            arm_check_irq();
        break;
        case 0x04000201:
            int_enb.b.b1 = value;
            arm_check_irq();
        break;
        case 0x04000202: int_ack.b.b0        &= ~value; break;
        case 0x04000203: int_ack.b.b1        &= ~value; break;
        case 0x04000204:
            wait_cnt.b.b0 = value;
            update_ws();
        break;
        case 0x04000205:
            wait_cnt.b.b1 = value;
            update_ws();
        break;
        case 0x04000206:
            wait_cnt.b.b2 = value;
            update_ws();
        break;
        case 0x04000207:
            wait_cnt.b.b3 = value;
            update_ws();
        break;
        case 0x04000208:
            int_enb_m.b.b0 = value;
            arm_check_irq();
        break;
        case 0x04000209:
            int_enb_m.b.b1 = value;
            arm_check_irq();
        break;
        case 0x0400020a:
            int_enb_m.b.b2 = value;
            arm_check_irq();
        break;
        case 0x0400020b:
            int_enb_m.b.b3 = value;
            arm_check_irq();
        break;

        case 0x04000300: post_boot            =  value; break;
        case 0x04000301: int_halt             =  true;  break;
    }
}

void trigger_irq(uint16_t flag) {
    int_ack.w |= flag;

    int_halt = false;

    arm_check_irq();
}

static const uint8_t ws0_s_lut[2] = { 2, 1 };
static const uint8_t ws1_s_lut[2] = { 4, 1 };
static const uint8_t ws2_s_lut[2] = { 8, 1 };
static const uint8_t ws_n_lut[4]  = { 4, 3, 2, 8 };

void update_ws() {
    ws_n[0] = ws_n_lut[(wait_cnt.w >> 2) & 3];
    ws_n[1] = ws_n_lut[(wait_cnt.w >> 5) & 3];
    ws_n[2] = ws_n_lut[(wait_cnt.w >> 8) & 3];

    ws_s[0] = ws0_s_lut[(wait_cnt.w >>  4) & 1];
    ws_s[1] = ws1_s_lut[(wait_cnt.w >>  7) & 1];
    ws_s[2] = ws2_s_lut[(wait_cnt.w >> 10) & 1];

    int8_t i;

    for (i = 0; i < 3; i++) {
        ws_n_t16[i] = ws_n[i] + 1;
        ws_s_t16[i] = ws_s[i] + 1;

        ws_n_arm[i] = ws_n_t16[i] + ws_s_t16[i];
        ws_s_arm[i] = ws_s_t16[i] << 1;
    }
}