#define DMA_REP  (1 <<  9)
#define DMA_32   (1 << 10)
#define DMA_IRQ  (1 << 14)
#define DMA_ENB  (1 << 15)

typedef enum {
    IMMEDIATELY = 0,
    VBLANK      = 1,
    HBLANK      = 2,
    SPECIAL     = 3
} dma_timing_e;

uint32_t dma_src_addr[4];
uint32_t dma_dst_addr[4];

uint32_t dma_count[4];

void dma_transfer(dma_timing_e timing);