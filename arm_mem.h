#include <stdint.h>

uint8_t *bios;
uint8_t *wram;
uint8_t *iwram;
uint8_t *pram;
uint8_t *vram;
uint8_t *oam;
uint8_t *rom;
uint8_t *eeprom;
uint8_t *sram;
uint8_t *flash;

uint32_t palette[0x200];

uint32_t bios_op;

int64_t cart_rom_size;
uint32_t cart_rom_mask;

uint16_t eeprom_idx;

typedef enum {
    NON_SEQ,
    SEQUENTIAL
} access_type_e;

uint8_t arm_readb(uint32_t address);
uint32_t arm_readh(uint32_t address);
uint32_t arm_read(uint32_t address);
uint8_t arm_readb_n(uint32_t address);
uint32_t arm_readh_n(uint32_t address);
uint32_t arm_read_n(uint32_t address);
uint8_t arm_readb_s(uint32_t address);
uint32_t arm_readh_s(uint32_t address);
uint32_t arm_read_s(uint32_t address);

void arm_writeb(uint32_t address, uint8_t value);
void arm_writeh(uint32_t address, uint16_t value);
void arm_write(uint32_t address, uint32_t value);
void arm_writeb_n(uint32_t address, uint8_t value);
void arm_writeh_n(uint32_t address, uint16_t value);
void arm_write_n(uint32_t address, uint32_t value);
void arm_writeb_s(uint32_t address, uint8_t value);
void arm_writeh_s(uint32_t address, uint16_t value);
void arm_write_s(uint32_t address, uint32_t value);