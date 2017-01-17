#include <stdbool.h>

#define CPU_FREQ_HZ       16777216
#define SND_FREQUENCY     32768
#define SND_CHANNELS      2
#define SND_SAMPLES       512
#define SAMP_CYCLES       (CPU_FREQ_HZ / SND_FREQUENCY)
#define BUFF_SAMPLES      ((SND_SAMPLES) * 16 * 2)
#define BUFF_SAMPLES_MSK  ((BUFF_SAMPLES) - 1)

int8_t fifo_a[0x20];
int8_t fifo_b[0x20];

uint8_t fifo_a_len;
uint8_t fifo_b_len;

typedef struct {
    bool     phase;       //Square 1/2 only
    uint16_t lfsr;        //Noise only
    double   samples;     //All
    double   length_time; //All
    double   sweep_time;  //Square 1 only
    double   env_time;    //All except Wave
} snd_ch_state_t;

snd_ch_state_t snd_ch_state[4];

uint8_t wave_position;
uint8_t wave_samples;

void wave_reset();

void sound_mix(void *data, uint8_t *stream, int32_t len);
void sound_clock(uint32_t cycles);

void fifo_a_copy();
void fifo_b_copy();

void fifo_a_load();
void fifo_b_load();