#include "arm.h"

#include "dma.h"
#include "io.h"
#include "sound.h"
#include "timer.h"

static const uint8_t pscale_shift_lut[4]  = { 0, 6, 8, 10 };

void timers_clock(uint32_t cycles) {
    uint8_t idx;
    bool overflow = false;

    for (idx = 0; idx < 4; idx++) {
        if (!(tmr[idx].ctrl.w & TMR_ENB)) {
            overflow = false;
            continue;
        }

        if (tmr[idx].ctrl.w & TMR_CASCADE) {
            if (overflow) tmr[idx].count.w++;
        } else {
            uint8_t shift = pscale_shift_lut[tmr[idx].ctrl.w & 3];
            uint32_t inc = (tmr_icnt[idx] += cycles) >> shift;

            tmr[idx].count.w += inc;
            tmr_icnt[idx] -= inc << shift;
        }

        if ((overflow = (tmr[idx].count.w > 0xffff))) {
            tmr[idx].count.w = tmr[idx].reload.w + (tmr[idx].count.w - 0x10000);

            if (((snd_pcm_vol.w >> 10) & 1) == idx) {
                //DMA Sound A FIFO
                fifo_a_load();

                if (fifo_a_len <= 0x10) dma_transfer_fifo(1);
            }

            if (((snd_pcm_vol.w >> 14) & 1) == idx) {
                //DMA Sound B FIFO
                fifo_b_load();

                if (fifo_b_len <= 0x10) dma_transfer_fifo(2);
            }
        }

        if ((tmr[idx].ctrl.w & TMR_IRQ) && overflow)
            trigger_irq(TMR0_FLAG << idx);
    }
}