#include "arm.h"

#include "io.h"
#include "timer.h"

static const uint8_t pscale_shift_lut[4]  = { 0, 6, 8, 10 };

void tick_timers() {
	uint8_t idx;

	bool overflow = false;

	uint32_t cycles = arm_cycles - tmr_base_cycles;

	if (!cycles) return;

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
			tmr[idx].count.w = tmr[idx].reload.w;
		}

		if ((tmr[idx].ctrl.w & TMR_IRQ) && overflow) {
			trigger_irq(TMR0_FLAG << idx);
		}
	}

	tmr_base_cycles = arm_cycles;
}