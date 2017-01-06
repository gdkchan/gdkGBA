#include <stdio.h>

#include "arm.h"
#include "arm_mem.h"

#include "dma.h"
#include "io.h"

void dma_transfer(dma_timing_e timing) {
	uint8_t ch;

	for (ch = 0; ch < 4; ch++) {
		if (!(dma_ch[ch].ctrl.w & DMA_ENB) ||
			((dma_ch[ch].ctrl.w >> 12) & 3) != timing)
			continue;

		int8_t unit_size = (dma_ch[ch].ctrl.w & DMA_32) ? 4 : 2;

		bool dst_reload = false;

		int8_t dst_inc = 0;
		int8_t src_inc = 0;

		switch ((dma_ch[ch].ctrl.w >> 5) & 3) {
			case 0: dst_inc =  unit_size; break;
			case 1: dst_inc = -unit_size; break;
			case 3:
				dst_inc = unit_size;
				dst_reload = true;
				break;
		}

		switch ((dma_ch[ch].ctrl.w >> 7) & 3) {
			case 0: src_inc =  unit_size; break;
			case 1: src_inc = -unit_size; break;
		}

		if (timing == SPECIAL) {
			dma_count[ch] = 4;
			dst_inc       = 0;
		}

		if (dma_src_addr[ch] & dma_dst_addr[ch] & 0x08000000)
			arm_cycles += 4;
		else
			arm_cycles += 2;

		while (dma_count[ch]--) {
			if (dma_ch[ch].ctrl.w & DMA_32)
				arm_write_s(dma_dst_addr[ch],  arm_read_s(dma_src_addr[ch]));
			else
				arm_writeh_s(dma_dst_addr[ch], arm_readh_s(dma_src_addr[ch]));

			dma_dst_addr[ch] += dst_inc;
			dma_src_addr[ch] += src_inc;
		}

		if (dma_ch[ch].ctrl.w & DMA_IRQ) {
			trigger_irq(DMA0_FLAG << ch);
		}

		if (dma_ch[ch].ctrl.w & DMA_REP) {
			dma_count[ch] = dma_ch[ch].count.w;

			if (dst_reload) {
				dma_dst_addr[ch] = dma_ch[ch].dst.w;
			}

			continue;
		}

		dma_ch[ch].ctrl.w &= ~DMA_ENB;
	}
}