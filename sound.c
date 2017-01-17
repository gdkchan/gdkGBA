#include <stdint.h>

#include "io.h"
#include "sound.h"

#define PSG_MAX   0x7f
#define PSG_MIN  -0x80

//How much time a single sample takes (in seconds)
#define SAMPLE_TIME  (1.0 / (SND_FREQUENCY))

static double duty_lut[4]   = { 0.125, 0.250, 0.500, 0.750 };
static double duty_lut_i[4] = { 0.875, 0.750, 0.500, 0.250 };

static int8_t square_sample(uint8_t ch) {
    if (!(snd_psg_enb.w & (CH_SQR1 << ch))) return 0;

    uint8_t  sweep_time = (sqr_ch[ch].sweep.w >>  4) & 0x7;
    uint8_t  duty       = (sqr_ch[ch].tone.w  >>  6) & 0x3;
    uint8_t  env_step   = (sqr_ch[ch].tone.w  >>  8) & 0x7;
    uint8_t  envelope   = (sqr_ch[ch].tone.w  >> 12) & 0xf;
    uint8_t  snd_len    = (sqr_ch[ch].tone.w  >>  0) & 0x3f;
    uint16_t freq_hz    = (sqr_ch[ch].ctrl.w  >>  0) & 0x7ff;

    //Actual frequency in Hertz
    double frequency = 131072 / (2048 - freq_hz);

    //Full length of the generated wave (if enabled) in seconds
    double length = (64 - snd_len) / 256.0;

    //Frquency sweep change interval in seconds
    double sweep_interval = 0.0078 * (sweep_time + 1);

    //Envelope volume change interval in seconds
    double envelope_interval = env_step / 64.0;

    //Numbers of samples that a single cycle (wave phase change 1 -> 0) takes at output sample rate
    double cycle_samples = SND_FREQUENCY / frequency;

    //Length reached check (if so, just disable the channel and return silence)
    if (sqr_ch[ch].ctrl.w & CH_LEN) {
        snd_ch_state[ch].length_time += SAMPLE_TIME;

        if (snd_ch_state[ch].length_time >= length) {
            //Disable channel
            snd_psg_enb.w &= ~(CH_SQR1 << ch);

            //And return silence
            return 0;
        }
    }

    //Frequency sweep (Square 1 channel only)
    if (ch == 0) {
        snd_ch_state[0].sweep_time += SAMPLE_TIME;

        if (snd_ch_state[0].sweep_time >= sweep_interval) {
            snd_ch_state[0].sweep_time -= sweep_interval;

            //A Sweep Shift of 0 means that Sweep is disabled
            uint8_t sweep_shift = sqr_ch[0].sweep.w & 7;

            if (sweep_shift) {
                uint32_t disp = freq_hz >> sweep_shift;

                if (sqr_ch[0].sweep.w & SWEEP_DEC)
                    freq_hz -= disp;
                else
                    freq_hz += disp;

                if (freq_hz < 0x7ff) {
                    //Update frequency
                    sqr_ch[0].ctrl.w &= ~0x7ff;
                    sqr_ch[0].ctrl.w |= freq_hz;
                } else {
                    //Disable channel
                    snd_psg_enb.w &= ~CH_SQR1;
                }
            }
        }
    }

    //Envelope volume
    if (env_step) {
        snd_ch_state[ch].env_time += SAMPLE_TIME;

        if (snd_ch_state[ch].env_time >= envelope_interval) {
            snd_ch_state[ch].env_time -= envelope_interval;

            if (sqr_ch[ch].tone.w & ENV_INC) {
                if (envelope < 0xf) envelope++;
            } else {
                if (envelope > 0x0) envelope--;
            }

            sqr_ch[ch].tone.w &= ~0xf000;
            sqr_ch[ch].tone.w |= envelope << 12;
        }
    }

    //Phase change (when the wave goes from Low to High or High to Low, the Square Wave pattern)
    snd_ch_state[ch].samples++;

    if (snd_ch_state[ch].phase) {
        //1 -> 0
        double phase_change = cycle_samples * duty_lut[duty];

        if (snd_ch_state[ch].samples >  phase_change) {
            snd_ch_state[ch].samples -= phase_change;
            snd_ch_state[ch].phase = false;
        }
    } else {
        //0 -> 1
        double phase_change = cycle_samples * duty_lut_i[duty];

        if (snd_ch_state[ch].samples >  phase_change) {
            snd_ch_state[ch].samples -= phase_change;
            snd_ch_state[ch].phase = true;
        }
    }

    return snd_ch_state[ch].phase
        ? (envelope / 15.0) * PSG_MAX
        : (envelope / 15.0) * PSG_MIN;
}

static int8_t wave_sample() {
    if (!((snd_psg_enb.w & CH_WAVE) && (wave_ch.wave.w & WAVE_PLAY))) return 0;

    uint8_t  snd_len = (wave_ch.volume.w >>  0) & 0xff;
    uint8_t  volume  = (wave_ch.volume.w >> 13) & 0x7;
    uint16_t freq_hz = (wave_ch.ctrl.w   >>  0) & 0x7ff;

    //Actual frequency in Hertz
    double frequency = 2097152 / (2048 - freq_hz);

    //Full length of the generated wave (if enabled) in seconds
    double length = (256 - snd_len) / 256.0;

    //Numbers of samples that a single "cycle" (all entries on Wave RAM) takes at output sample rate
    double cycle_samples = SND_FREQUENCY / frequency;

    //Length reached check (if so, just disable the channel and return silence)
    if (wave_ch.ctrl.w & CH_LEN) {
        snd_ch_state[2].length_time += SAMPLE_TIME;

        if (snd_ch_state[2].length_time >= length) {
            //Disable channel
            snd_psg_enb.w &= ~CH_WAVE;

            //And return silence
            return 0;
        }
    }

    snd_ch_state[2].samples++;

    if (snd_ch_state[2].samples >= cycle_samples) {
        snd_ch_state[2].samples -= cycle_samples;

        if (--wave_samples)
            wave_position = (wave_position + 1) & 0x3f;
        else
            wave_reset();
    }

    int8_t samp = wave_position & 1
        ? ((wave_ram[(wave_position >> 1) & 0x1f] >> 0) & 0xf) - 8
        : ((wave_ram[(wave_position >> 1) & 0x1f] >> 4) & 0xf) - 8;

    switch (volume) {
        case 0: samp   = 0; break; //Mute
        case 1: samp >>= 0; break; //100%
        case 2: samp >>= 1; break; //50%
        case 3: samp >>= 2; break; //25%
        default: samp = (samp >> 2) * 3; break; //75%
    }

    return samp >= 0
        ? (samp /  7.0) * PSG_MAX
        : (samp / -8.0) * PSG_MIN;
}

static int8_t noise_sample() {
    if (!(snd_psg_enb.w & CH_NOISE)) return 0;

    uint8_t env_step = (noise_ch.env.w  >>  8) & 0x7;
    uint8_t envelope = (noise_ch.env.w  >> 12) & 0xf;
    uint8_t snd_len  = (noise_ch.env.w  >>  0) & 0x3f;
    uint8_t freq_div = (noise_ch.ctrl.w >>  0) & 0x7;
    uint8_t freq_rsh = (noise_ch.ctrl.w >>  4) & 0xf;

    //Actual frequency in Hertz
    double frequency = freq_div
        ? (524288 / freq_div) >> (freq_rsh + 1)
        : (524288 *        2) >> (freq_rsh + 1);

    //Full length of the generated wave (if enabled) in seconds
    double length = (64 - snd_len) / 256.0;

    //Envelope volume change interval in seconds
    double envelope_interval = env_step / 64.0;

    //Numbers of samples that a single cycle (pseudo-random noise value) takes at output sample rate
    double cycle_samples = SND_FREQUENCY / frequency;

    //Length reached check (if so, just disable the channel and return silence)
    if (noise_ch.ctrl.w & CH_LEN) {
        snd_ch_state[3].length_time += SAMPLE_TIME;

        if (snd_ch_state[3].length_time >= length) {
            //Disable channel
            snd_psg_enb.w &= ~CH_NOISE;

            //And return silence
            return 0;
        }
    }

    //Envelope volume
    if (env_step) {
        snd_ch_state[3].env_time += SAMPLE_TIME;

        if (snd_ch_state[3].env_time >= envelope_interval) {
            snd_ch_state[3].env_time -= envelope_interval;

            if (noise_ch.env.w & ENV_INC) {
                if (envelope < 0xf) envelope++;
            } else {
                if (envelope > 0x0) envelope--;
            }

            noise_ch.env.w &= ~0xf000;
            noise_ch.env.w |= envelope << 12;
        }
    }

    uint8_t carry = snd_ch_state[3].lfsr & 1;

    snd_ch_state[3].samples++;

    if (snd_ch_state[3].samples >= cycle_samples) {
        snd_ch_state[3].samples -= cycle_samples;

        snd_ch_state[3].lfsr >>= 1;

        uint8_t high = (snd_ch_state[3].lfsr & 1) ^ carry;

        if (noise_ch.ctrl.w & NOISE_7)
            snd_ch_state[3].lfsr |= (high <<  6);
        else
            snd_ch_state[3].lfsr |= (high << 14);
    }

    return carry
        ? (envelope / 15.0) * PSG_MAX
        : (envelope / 15.0) * PSG_MIN;
}

int16_t snd_buffer[BUFF_SAMPLES];

uint32_t snd_cur_play = 0;
uint32_t snd_cur_write = 0;

void sound_mix(void *data, uint8_t *stream, int32_t len) {
    uint16_t i;

    for (i = 0; i < len; i += 4) {
        *(int16_t *)(stream + (i | 0)) = snd_buffer[snd_cur_play++ & BUFF_SAMPLES_MSK] << 4;
        *(int16_t *)(stream + (i | 2)) = snd_buffer[snd_cur_play++ & BUFF_SAMPLES_MSK] << 4;
    }

    //Avoid desync between the Play cursor and the Write cursor
    snd_cur_play += ((int32_t)(snd_cur_write - snd_cur_play) >> 9) & ~1;

    if ((snd_cur_play & snd_cur_write) >= BUFF_SAMPLES) {
        snd_cur_play  &= BUFF_SAMPLES_MSK;
        snd_cur_write &= BUFF_SAMPLES_MSK;
    }
}

void wave_reset() {
    if (wave_ch.wave.w & WAVE_64) {
        //64 samples (at 4 bits each, uses both banks so initial position is always 0)
        wave_position = 0;
        wave_samples  = 64;
    } else {
        //32 samples (at 4 bits each, bank selectable through Wave Control register)
        wave_position = (wave_ch.wave.w >> 1) & 0x20;
        wave_samples  = 32;
    }
}

void fifo_a_copy() {
    if (fifo_a_len + 4 > 0x20) return; //FIFO A full

    fifo_a[fifo_a_len++] = snd_fifo_a_0;
    fifo_a[fifo_a_len++] = snd_fifo_a_1;
    fifo_a[fifo_a_len++] = snd_fifo_a_2;
    fifo_a[fifo_a_len++] = snd_fifo_a_3;
}

void fifo_b_copy() {
    if (fifo_b_len + 4 > 0x20) return; //FIFO B full

    fifo_b[fifo_b_len++] = snd_fifo_b_0;
    fifo_b[fifo_b_len++] = snd_fifo_b_1;
    fifo_b[fifo_b_len++] = snd_fifo_b_2;
    fifo_b[fifo_b_len++] = snd_fifo_b_3;
}

int8_t fifo_a_samp;
int8_t fifo_b_samp;

void fifo_a_load() {
    if (fifo_a_len) {
        fifo_a_samp = fifo_a[0];
        fifo_a_len--;

        uint8_t i;

        for (i = 0; i < fifo_a_len; i++) {
            fifo_a[i] = fifo_a[i + 1];
        }
    }
}

void fifo_b_load() {
    if (fifo_b_len) {
        fifo_b_samp = fifo_b[0];
        fifo_b_len--;

        uint8_t i;

        for (i = 0; i < fifo_b_len; i++) {
            fifo_b[i] = fifo_b[i + 1];
        }
    }
}

uint32_t snd_cycles = 0;

static int32_t psg_vol_lut[8] = { 0x000, 0x024, 0x049, 0x06d, 0x092, 0x0b6, 0x0db, 0x100 };
static int32_t psg_rsh_lut[4] = { 0xa, 0x9, 0x8, 0x7 };

void sound_clock(uint32_t cycles) {
    snd_cycles += cycles;

    int16_t samp_pcm_l = 0;
    int16_t samp_pcm_r = 0;

    int16_t samp_ch4 = (fifo_a_samp << 1) >> !(snd_pcm_vol.w & 4);
    int16_t samp_ch5 = (fifo_b_samp << 1) >> !(snd_pcm_vol.w & 8);

    if (snd_pcm_vol.w & CH_DMAA_L) samp_pcm_l += samp_ch4;
    if (snd_pcm_vol.w & CH_DMAB_L) samp_pcm_l += samp_ch5;

    if (snd_pcm_vol.w & CH_DMAA_R) samp_pcm_r += samp_ch4;
    if (snd_pcm_vol.w & CH_DMAB_R) samp_pcm_r += samp_ch5;

    while (snd_cycles >= SAMP_CYCLES) {
        int16_t samp_ch0 = square_sample(0);
        int16_t samp_ch1 = square_sample(1);
        int16_t samp_ch2 = wave_sample();
        int16_t samp_ch3 = noise_sample();

        int32_t samp_psg_l = 0;
        int32_t samp_psg_r = 0;

        if (snd_psg_vol.w & CH_SQR1_L)  samp_psg_l += samp_ch0;
        if (snd_psg_vol.w & CH_SQR2_L)  samp_psg_l += samp_ch1;
        if (snd_psg_vol.w & CH_WAVE_L)  samp_psg_l += samp_ch2;
        if (snd_psg_vol.w & CH_NOISE_L) samp_psg_l += samp_ch3;

        if (snd_psg_vol.w & CH_SQR1_R)  samp_psg_r += samp_ch0;
        if (snd_psg_vol.w & CH_SQR2_R)  samp_psg_r += samp_ch1;
        if (snd_psg_vol.w & CH_WAVE_R)  samp_psg_r += samp_ch2;
        if (snd_psg_vol.w & CH_NOISE_R) samp_psg_r += samp_ch3;

        samp_psg_l  *= psg_vol_lut[(snd_psg_vol.w >> 4) & 7];
        samp_psg_r  *= psg_vol_lut[(snd_psg_vol.w >> 0) & 7];

        samp_psg_l >>= psg_rsh_lut[(snd_pcm_vol.w >> 0) & 3];
        samp_psg_r >>= psg_rsh_lut[(snd_pcm_vol.w >> 0) & 3];

        snd_buffer[snd_cur_write++ & BUFF_SAMPLES_MSK] = samp_psg_l + samp_pcm_l;
        snd_buffer[snd_cur_write++ & BUFF_SAMPLES_MSK] = samp_psg_r + samp_pcm_r;

        snd_cycles -= SAMP_CYCLES;
    }
}