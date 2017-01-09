typedef union {
    uint32_t w;

    struct {
        uint8_t b0;
        uint8_t b1;
        uint8_t b2;
        uint8_t b3;
    } b;
} io_reg;

#define VBLK_FLAG  (1 <<  0)
#define HBLK_FLAG  (1 <<  1)
#define VCNT_FLAG  (1 <<  2)
#define TMR0_FLAG  (1 <<  3)
#define TMR1_FLAG  (1 <<  4)
#define TMR2_FLAG  (1 <<  5)
#define TMR3_FLAG  (1 <<  6)
#define DMA0_FLAG  (1 <<  8)
#define DMA1_FLAG  (1 <<  9)
#define DMA2_FLAG  (1 << 10)
#define DMA3_FLAG  (1 << 11)

#define MAP_1D_FLAG  (1 <<  6)
#define BG0_ENB      (1 <<  8)
#define BG1_ENB      (1 <<  9)
#define BG2_ENB      (1 << 10)
#define BG3_ENB      (1 << 11)
#define OBJ_ENB      (1 << 12)

#define VBLK_IRQ  (1 <<  3)
#define HBLK_IRQ  (1 <<  4)
#define VCNT_IRQ  (1 <<  5)

io_reg disp_cnt;
io_reg disp_stat;
io_reg v_count;

typedef struct {
    io_reg ctrl;
    io_reg xofs;
    io_reg yofs;
} bg_t;

bg_t bg[4];

io_reg bg_pa[4];
io_reg bg_pb[4];
io_reg bg_pc[4];
io_reg bg_pd[4];

io_reg bg_refxe[4];
io_reg bg_refye[4];

io_reg bg_refxi[4];
io_reg bg_refyi[4];

io_reg win_in;
io_reg win_out;

io_reg bld_cnt;

typedef struct {
    io_reg cnt_l;
    io_reg cnt_h;
    io_reg cnt_x;
} snd_ch_t;

snd_ch_t snd_ch[4];

io_reg sound_cnt_l;
io_reg sound_cnt_h;
io_reg sound_cnt_x;
io_reg sound_bias;

typedef struct {
    io_reg src;
    io_reg dst;
    io_reg count;
    io_reg ctrl;
} dma_ch_t;

dma_ch_t dma_ch[4];

#define BTN_A    (1 << 0)
#define BTN_B    (1 << 1)
#define BTN_SEL  (1 << 2)
#define BTN_STA  (1 << 3)
#define BTN_R    (1 << 4)
#define BTN_L    (1 << 5)
#define BTN_U    (1 << 6)
#define BTN_D    (1 << 7)
#define BTN_RT   (1 << 8)
#define BTN_LT   (1 << 9)

typedef struct {
    io_reg count;
    io_reg reload;
    io_reg ctrl;
} tmr_t;

tmr_t tmr[4];

io_reg r_cnt;
io_reg sio_cnt;
io_reg sio_data8;
io_reg sio_data32;

io_reg key_input;

io_reg int_enb;
io_reg int_ack;
io_reg wait_cnt;
io_reg int_enb_m;

uint8_t ws_n[4];
uint8_t ws_s[4];

uint8_t ws_n_arm[4];
uint8_t ws_s_arm[4];

uint8_t ws_n_t16[4];
uint8_t ws_s_t16[4];

uint8_t post_boot;

uint8_t io_read(uint32_t address);
void io_write(uint32_t address, uint8_t value);

void trigger_irq(uint8_t flag);

void update_ws();