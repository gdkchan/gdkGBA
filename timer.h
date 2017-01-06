#define TMR_CASCADE  (1 << 2)
#define TMR_IRQ      (1 << 6)
#define TMR_ENB      (1 << 7)

uint32_t tmr_base_cycles;

uint32_t tmr_icnt[4];

uint8_t tmr_enb;
uint8_t tmr_irq;
uint8_t tmr_ie;

void tick_timers();