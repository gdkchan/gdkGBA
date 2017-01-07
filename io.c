#include "arm.h"

#include "dma.h"
#include "io.h"
#include "timer.h"

uint8_t io_read(uint32_t address) {
    switch (address) {
        case 0x04000000: return disp_cnt.b.b0;
        case 0x04000001: return disp_cnt.b.b1;
        case 0x04000004: return disp_stat.b.b0;
        case 0x04000005: return disp_stat.b.b1;
        case 0x04000006: return v_count.b.b0;
        case 0x04000007: return v_count.b.b1;

        case 0x04000008: return bg[0].ctrl.b.b0;
        case 0x04000009: return bg[0].ctrl.b.b1;
        case 0x0400000a: return bg[1].ctrl.b.b0;
        case 0x0400000b: return bg[1].ctrl.b.b1;
        case 0x0400000c: return bg[2].ctrl.b.b0;
        case 0x0400000d: return bg[2].ctrl.b.b1;
        case 0x0400000e: return bg[3].ctrl.b.b0;
        case 0x0400000f: return bg[3].ctrl.b.b1;

        case 0x04000048: return win_in.b.b0;
        case 0x04000049: return win_in.b.b1;
        case 0x0400004a: return win_out.b.b0;
        case 0x0400004b: return win_out.b.b1;

        case 0x04000050: return bld_cnt.b.b0;
        case 0x04000051: return bld_cnt.b.b1;

        case 0x04000060: return snd_ch[0].cnt_l.b.b0;
        case 0x04000061: return snd_ch[0].cnt_l.b.b1;
        case 0x04000062: return snd_ch[0].cnt_h.b.b0;
        case 0x04000063: return snd_ch[0].cnt_h.b.b1;
        case 0x04000064: return snd_ch[0].cnt_x.b.b0;
        case 0x04000065: return snd_ch[0].cnt_x.b.b1;

        case 0x04000068: return snd_ch[1].cnt_l.b.b0;
        case 0x04000069: return snd_ch[1].cnt_l.b.b1;
        case 0x0400006c: return snd_ch[1].cnt_h.b.b0;
        case 0x0400006d: return snd_ch[1].cnt_h.b.b1;

        case 0x04000070: return snd_ch[2].cnt_l.b.b0;
        case 0x04000071: return snd_ch[2].cnt_l.b.b1;
        case 0x04000072: return snd_ch[2].cnt_h.b.b0;
        case 0x04000073: return snd_ch[2].cnt_h.b.b1;
        case 0x04000074: return snd_ch[2].cnt_x.b.b0;
        case 0x04000075: return snd_ch[2].cnt_x.b.b1;

        case 0x04000078: return snd_ch[3].cnt_l.b.b0;
        case 0x04000079: return snd_ch[3].cnt_l.b.b1;
        case 0x0400007c: return snd_ch[3].cnt_h.b.b0;
        case 0x0400007d: return snd_ch[3].cnt_h.b.b1;

        case 0x04000080: return sound_cnt_l.b.b0;
        case 0x04000081: return sound_cnt_l.b.b1;
        case 0x04000082: return sound_cnt_h.b.b0;
        case 0x04000083: return sound_cnt_h.b.b1;
        case 0x04000084: return sound_cnt_x.b.b0;
        case 0x04000085: return sound_cnt_x.b.b1;
        case 0x04000088: return sound_bias.b.b0;
        case 0x04000089: return sound_bias.b.b1;

        case 0x040000ba: return dma_ch[0].ctrl.b.b0;
        case 0x040000bb: return dma_ch[0].ctrl.b.b1;

        case 0x040000c6: return dma_ch[1].ctrl.b.b0;
        case 0x040000c7: return dma_ch[1].ctrl.b.b1;

        case 0x040000d2: return dma_ch[2].ctrl.b.b0;
        case 0x040000d3: return dma_ch[2].ctrl.b.b1;

        case 0x040000de: return dma_ch[3].ctrl.b.b0;
        case 0x040000df: return dma_ch[3].ctrl.b.b1;

        case 0x04000100: return tmr[0].count.b.b0;
        case 0x04000101: return tmr[0].count.b.b1;
        case 0x04000102: return tmr[0].ctrl.b.b0;
        case 0x04000103: return tmr[0].ctrl.b.b1;

        case 0x04000104: return tmr[1].count.b.b0;
        case 0x04000105: return tmr[1].count.b.b1;
        case 0x04000106: return tmr[1].ctrl.b.b0;
        case 0x04000107: return tmr[1].ctrl.b.b1;

        case 0x04000108: return tmr[2].count.b.b0;
        case 0x04000109: return tmr[2].count.b.b1;
        case 0x0400010a: return tmr[2].ctrl.b.b0;
        case 0x0400010b: return tmr[2].ctrl.b.b1;

        case 0x0400010c: return tmr[3].count.b.b0;
        case 0x0400010d: return tmr[3].count.b.b1;
        case 0x0400010e: return tmr[3].ctrl.b.b0;
        case 0x0400010f: return tmr[3].ctrl.b.b1;

        case 0x04000120: return sio_data32.b.b0;
        case 0x04000121: return sio_data32.b.b1;
        case 0x04000122: return sio_data32.b.b2;
        case 0x04000123: return sio_data32.b.b3;
        case 0x04000128: return sio_cnt.b.b0;
        case 0x04000129: return sio_cnt.b.b1;
        case 0x0400012a: return sio_data8.b.b0;
        case 0x04000134: return r_cnt.b.b0;
        case 0x04000135: return r_cnt.b.b1;

        case 0x04000130: return key_input.b.b0;
        case 0x04000131: return key_input.b.b1;

        case 0x04000200: return int_enb.b.b0;
        case 0x04000201: return int_enb.b.b1;
        case 0x04000202: return int_ack.b.b0;
        case 0x04000203: return int_ack.b.b1;
        case 0x04000204: return wait_cnt.b.b0;
        case 0x04000205: return wait_cnt.b.b1;
        case 0x04000206: return wait_cnt.b.b2;
        case 0x04000207: return wait_cnt.b.b3;
        case 0x04000208: return int_enb_m.b.b0;
        case 0x04000209: return int_enb_m.b.b1;
        case 0x0400020a: return int_enb_m.b.b2;
        case 0x0400020b: return int_enb_m.b.b3;

        case 0x04000300: return post_boot;
    }

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

void io_write(uint32_t address, uint8_t value) {
    switch (address) {
        case 0x04000000: disp_cnt.b.b0        =  value; break;
        case 0x04000001: disp_cnt.b.b1        =  value; break;
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

        case 0x04000028: bg_refxl[2].b.b0     =  value; break;
        case 0x04000029: bg_refxl[2].b.b1     =  value; break;
        case 0x0400002a: bg_refxh[2].b.b0     =  value; break;
        case 0x0400002b: bg_refxh[2].b.b1     =  value; break;
        case 0x0400002c: bg_refyl[2].b.b0     =  value; break;
        case 0x0400002d: bg_refyl[2].b.b1     =  value; break;
        case 0x0400002e: bg_refyh[2].b.b0     =  value; break;
        case 0x0400002f: bg_refyh[2].b.b1     =  value; break;

        case 0x04000038: bg_refxl[3].b.b0     =  value; break;
        case 0x04000039: bg_refxl[3].b.b1     =  value; break;
        case 0x0400003a: bg_refxh[3].b.b0     =  value; break;
        case 0x0400003b: bg_refxh[3].b.b1     =  value; break;
        case 0x0400003c: bg_refyl[3].b.b0     =  value; break;
        case 0x0400003d: bg_refyl[3].b.b1     =  value; break;
        case 0x0400003e: bg_refyh[3].b.b0     =  value; break;
        case 0x0400003f: bg_refyh[3].b.b1     =  value; break;

        case 0x04000048: win_in.b.b0          =  value; break;
        case 0x04000049: win_in.b.b1          =  value; break;
        case 0x0400004a: win_out.b.b0         =  value; break;
        case 0x0400004b: win_out.b.b1         =  value; break;

        case 0x04000050: bld_cnt.b.b0         =  value; break;
        case 0x04000051: bld_cnt.b.b1         =  value; break;

        case 0x04000060: snd_ch[0].cnt_l.b.b0 =  value; break;
        case 0x04000061: snd_ch[0].cnt_l.b.b1 =  value; break;
        case 0x04000062: snd_ch[0].cnt_h.b.b0 =  value; break;
        case 0x04000063: snd_ch[0].cnt_h.b.b1 =  value; break;
        case 0x04000064: snd_ch[0].cnt_x.b.b0 =  value; break;
        case 0x04000065: snd_ch[0].cnt_x.b.b1 =  value; break;

        case 0x04000068: snd_ch[1].cnt_l.b.b0 =  value; break;
        case 0x04000069: snd_ch[1].cnt_l.b.b1 =  value; break;
        case 0x0400006c: snd_ch[1].cnt_h.b.b0 =  value; break;
        case 0x0400006d: snd_ch[1].cnt_h.b.b1 =  value; break;

        case 0x04000070: snd_ch[2].cnt_l.b.b0 =  value; break;
        case 0x04000071: snd_ch[2].cnt_l.b.b1 =  value; break;
        case 0x04000072: snd_ch[2].cnt_h.b.b0 =  value; break;
        case 0x04000073: snd_ch[2].cnt_h.b.b1 =  value; break;
        case 0x04000074: snd_ch[2].cnt_x.b.b0 =  value; break;
        case 0x04000075: snd_ch[2].cnt_x.b.b1 =  value; break;

        case 0x04000078: snd_ch[3].cnt_l.b.b0 =  value; break;
        case 0x04000079: snd_ch[3].cnt_l.b.b1 =  value; break;
        case 0x0400007c: snd_ch[3].cnt_h.b.b0 =  value; break;
        case 0x0400007d: snd_ch[3].cnt_h.b.b1 =  value; break;

        case 0x04000080: sound_cnt_l.b.b0     =  value; break;
        case 0x04000081: sound_cnt_l.b.b1     =  value; break;
        case 0x04000082: sound_cnt_h.b.b0     =  value; break;
        case 0x04000083: sound_cnt_h.b.b1     =  value; break;
        case 0x04000084: sound_cnt_x.b.b0     =  value; break;
        case 0x04000085: sound_cnt_x.b.b1     =  value; break;
        case 0x04000088: sound_bias.b.b0      =  value; break;
        case 0x04000089: sound_bias.b.b1      =  value; break;

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

void trigger_irq(uint8_t flag) {
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