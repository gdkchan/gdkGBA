#include "arm.h"
#include "arm_mem.h"

#include "io.h"

#define EEPROM_WRITE  2
#define EEPROM_READ   3

uint32_t flash_bank = 0;

typedef enum {
    IDLE,
    ERASE,
    WRITE,
    BANK_SWITCH
} flash_mode_e;

flash_mode_e flash_mode = IDLE;

bool flash_id_mode = false;

bool eeprom_used = false;
bool eeprom_read = false;

uint32_t eeprom_addr      = 0;
uint32_t eeprom_addr_read = 0;

uint8_t eeprom_buff[0x100];

static const uint8_t bus_size_lut[16]  = { 4, 4, 2, 4, 4, 2, 2, 4, 2, 2, 2, 2, 2, 2, 1, 1 };

static void arm_access(uint32_t address, access_type_e at) {
    uint8_t cycles = 1;

    if (address & 0x08000000) {
        if (at == NON_SEQ)
            cycles += ws_n[(address >> 25) & 3];
        else
            cycles += ws_s[(address >> 25) & 3];
    } else if ((address >> 24) == 2) {
        cycles += 2;
    }

    arm_cycles += cycles;
}

static void arm_access_bus(uint32_t address, uint8_t size, access_type_e at) {
    uint8_t lut_idx = (address >> 24) & 0xf;
    uint8_t bus_sz = bus_size_lut[lut_idx];

    if (bus_sz < size) {
        arm_access(address + 0, at);
        arm_access(address + 2, SEQUENTIAL);
    } else {
        arm_access(address, at);
    }
}

//Memory read
static uint8_t bios_read(uint32_t address) {
    if ((address | arm_r.r[15]) < 0x4000)
        return bios[address & 0x3fff];
    else
        return bios_op;
}

static uint8_t wram_read(uint32_t address) {
    return wram[address & 0x3ffff];
}

static uint8_t iwram_read(uint32_t address) {
    return iwram[address & 0x7fff];
}

static uint8_t pram_read(uint32_t address) {
    return pram[address & 0x3ff];
}

static uint8_t vram_read(uint32_t address) {
    return vram[address & (address & 0x10000 ? 0x17fff : 0x1ffff)];
}

static uint8_t oam_read(uint32_t address) {
    return oam[address & 0x3ff];
}

static uint8_t rom_read(uint32_t address) {
    return rom[address & cart_rom_mask];
}

static uint8_t rom_eep_read(uint32_t address, uint8_t offset) {
    if (eeprom_used &&
        ((cart_rom_size >  0x1000000 && (address >>  8) == 0x0dffff) ||
         (cart_rom_size <= 0x1000000 && (address >> 24) == 0x00000d))) {
         if (!offset) {
             uint8_t mode = eeprom_buff[0] >> 6;

             switch (mode) {
                 case EEPROM_WRITE: return 1;
                 case EEPROM_READ: {
                    uint8_t value = 0;

                    if (eeprom_idx >= 4) {
                        uint8_t idx = ((eeprom_idx - 4) >> 3) & 7;
                        uint8_t bit = ((eeprom_idx - 4) >> 0) & 7;

                        value = (eeprom[eeprom_addr_read | idx] >> (bit ^ 7)) & 1;
                    }

                    eeprom_idx++;

                    return value;
                 }
             }
         }
    } else {
        return rom_read(address);
    }

    return 0;
}

static uint8_t flash_read(uint32_t address) {
    if (flash_id_mode) {
        //This is the Flash ROM ID, we return Sanyo ID code
        switch (address) {
            case 0x0e000000: return 0x62;
            case 0x0e000001: return 0x13;
        }
    } else {
        return flash[flash_bank | (address & 0xffff)];
    }

    return 0;
}

static uint8_t arm_read_(uint32_t address, uint8_t offset) {
    switch (address >> 24) {
        case 0x0: return bios_read(address);
        case 0x2: return wram_read(address);
        case 0x3: return iwram_read(address);
        case 0x4: return io_read(address);
        case 0x5: return pram_read(address);
        case 0x6: return vram_read(address);
        case 0x7: return oam_read(address);

        case 0x8:
        case 0x9:
            return rom_read(address);

        case 0xa:
        case 0xb:
            return rom_read(address);

        case 0xc:
        case 0xd:
            return rom_eep_read(address, offset);

        case 0xe:
        case 0xf:
            return flash_read(address);
    }

    return 0;
}

#define IS_OPEN_BUS(a)  (((a) >> 28) || ((a) >= 0x00004000 && (a) < 0x02000000))

uint8_t arm_readb(uint32_t address) {
    uint8_t value = arm_read_(address, 0);

    if (!(address & 0x08000000)) {
        io_open_bus &= ((address >> 24) == 4);

        if (IS_OPEN_BUS(address) || io_open_bus)
            value = arm_pipe[1];
    }

    return value;
}

uint32_t arm_readh(uint32_t address) {
    uint32_t a = address & ~1;
    uint8_t  s = address &  1;

    uint32_t value =
        arm_read_(a | 0, 0) << 0 |
        arm_read_(a | 1, 1) << 8;

    if (!(a & 0x08000000)) {
        io_open_bus &= ((a >> 24) == 4);

        if (a < 0x4000 && arm_r.r[15] >= 0x4000)
            value = bios_op     & 0xffff;
        else if (IS_OPEN_BUS(a) || io_open_bus)
            value = arm_pipe[1] & 0xffff;
    }

    return ROR(value, s << 3);
}

uint32_t arm_read(uint32_t address) {
    uint32_t a = address & ~3;
    uint8_t  s = address &  3;

    uint32_t value =
        arm_read_(a | 0, 0) <<  0 |
        arm_read_(a | 1, 1) <<  8 |
        arm_read_(a | 2, 2) << 16 |
        arm_read_(a | 3, 3) << 24;

    if (!(a & 0x08000000)) {
        io_open_bus &= ((a >> 24) == 4);

        if (a < 0x4000 && arm_r.r[15] >= 0x4000)
            value = bios_op;
        else if (IS_OPEN_BUS(a) || io_open_bus)
            value = arm_pipe[1];
    }

    return ROR(value, s << 3);
}

uint8_t arm_readb_n(uint32_t address) {
    arm_access_bus(address, ARM_BYTE_SZ, NON_SEQ);

    return arm_readb(address);
}

uint32_t arm_readh_n(uint32_t address) {
    arm_access_bus(address, ARM_HWORD_SZ, NON_SEQ);

    return arm_readh(address);
}

uint32_t arm_read_n(uint32_t address) {
    arm_access_bus(address, ARM_WORD_SZ, NON_SEQ);

    return arm_read(address);
}

uint8_t arm_readb_s(uint32_t address) {
    arm_access_bus(address, ARM_BYTE_SZ, SEQUENTIAL);

    return arm_readb(address);
}

uint32_t arm_readh_s(uint32_t address) {
    arm_access_bus(address, ARM_HWORD_SZ, SEQUENTIAL);

    return arm_readh(address);
}

uint32_t arm_read_s(uint32_t address) {
    arm_access_bus(address, ARM_WORD_SZ, SEQUENTIAL);

    return arm_read(address);
}

//Memory write
static void wram_write(uint32_t address, uint8_t value) {
    wram[address & 0x3ffff] = value;
}

static void iwram_write(uint32_t address, uint8_t value) {
    iwram[address & 0x7fff] = value;
}

static void pram_write(uint32_t address, uint8_t value) {
    pram[address & 0x3ff] = value;

    address &= 0x3fe;

    uint16_t pixel = pram[address] | (pram[address + 1] << 8);

    uint8_t r = ((pixel >>  0) & 0x1f) << 3;
    uint8_t g = ((pixel >>  5) & 0x1f) << 3;
    uint8_t b = ((pixel >> 10) & 0x1f) << 3;

    uint32_t rgba = 0xff;

    rgba |= (r | (r >> 5)) <<  8;
    rgba |= (g | (g >> 5)) << 16;
    rgba |= (b | (b >> 5)) << 24;

    palette[address >> 1] = rgba;
}

static void vram_write(uint32_t address, uint8_t value) {
    vram[address & (address & 0x10000 ? 0x17fff : 0x1ffff)] = value;
}

static void oam_write(uint32_t address, uint8_t value) {
    oam[address & 0x3ff] = value;
}

static void eeprom_write(uint32_t address, uint8_t offset, uint8_t value) {
    if (!offset &&
    ((cart_rom_size >  0x1000000 && (address >>  8) == 0x0dffff) ||
     (cart_rom_size <= 0x1000000 && (address >> 24) == 0x00000d))) {
        if (eeprom_idx == 0) {
            //First write, erase buffer
            eeprom_read = false;

            uint16_t i;

            for (i = 0; i < 0x100; i++)
                eeprom_buff[i] = 0;
        }

        uint8_t idx = (eeprom_idx >> 3) & 0xff;
        uint8_t bit = (eeprom_idx >> 0) & 0x7;

        eeprom_buff[idx] |= (value & 1) << (bit ^ 7);

        if (++eeprom_idx == dma_ch[3].count.w) {
            //Last write, process buffer
            uint8_t mode = eeprom_buff[0] >> 6;

            //Value is only valid if bit 1 is set (2 or 3, READ = 2)
            if (mode & EEPROM_READ) {
                bool eep_512b = (eeprom_idx == 2 + 6 + (mode == EEPROM_WRITE ? 64 : 0) + 1);

                if (eep_512b)
                    eeprom_addr =   eeprom_buff[0] & 0x3f;
                else
                    eeprom_addr = ((eeprom_buff[0] & 0x3f) << 8) | eeprom_buff[1];

                eeprom_addr <<= 3;

                if (mode == EEPROM_WRITE) {
                    //Perform write to actual EEPROM buffer
                    uint8_t buff_addr = eep_512b ? 1 : 2;

                    uint64_t value = *(uint64_t *)(eeprom_buff + buff_addr);

                    *(uint64_t *)(eeprom + eeprom_addr) = value;
                } else {
                    eeprom_addr_read = eeprom_addr;
                }

                eeprom_idx = 0;
            }
        }

        eeprom_used = true;
    }
}

static void flash_write(uint32_t address, uint8_t value) {
    if (flash_mode == WRITE) {
        flash[address & 0xffff] = value;
    } else if (flash_mode == BANK_SWITCH && address == 0x0e000000) {
        flash_bank = (value & 1) << 16;
    } else if (sram[0x5555] == 0xaa && sram[0x2aaa] == 0x55) {
        if (address == 0x0e005555) {
            //Command to do something on Flash ROM
            switch (value) {
                //Erase all
                case 0x10:
                    if (flash_mode == ERASE) {
                        uint32_t idx;

                        for (idx = 0; idx < 0x20000; idx++) {
                            flash[idx] = 0xff;
                        }
                    }
                break;

                case 0x80: flash_mode    = ERASE;       break;
                case 0x90: flash_id_mode = true;        break;
                case 0xa0: flash_mode    = WRITE;       break;
                case 0xb0: flash_mode    = BANK_SWITCH; break;
                case 0xf0: flash_id_mode = false;       break;
            }
        } else if (flash_mode == ERASE && (address & 0xfff) == 0) {
            uint32_t bank_s = address & 0xf000;
            uint32_t bank_e = bank_s + 0x1000;
            uint32_t idx;

            for (idx = bank_s; idx < bank_e; idx++) {
                flash[idx] = 0xff;
            }
        }
    }

    sram[address & 0xffff] = value;

    flash_mode = IDLE;
}

static void arm_write_(uint32_t address, uint8_t offset, uint8_t value) {
    switch (address >> 24) {
        case 0x2: wram_write(address, value); break;
        case 0x3: iwram_write(address, value); break;
        case 0x4: io_write(address, value); break;
        case 0x5: pram_write(address, value); break;
        case 0x6: vram_write(address, value); break;
        case 0x7: oam_write(address, value); break;

        case 0xc:
        case 0xd:
            eeprom_write(address, offset, value); break;

        case 0xe:
        case 0xf:
            flash_write(address, value); break;
    }
}

void arm_writeb(uint32_t address, uint8_t value) {
    uint8_t ah = address >> 24;

    if (ah == 7) return; //OAM doesn't supposrt 8 bits writes

    if (ah > 4 && ah < 8) {
        arm_write_(address + 0, 0, value);
        arm_write_(address + 1, 1, value);
    } else {
        arm_write_(address, 0, value);
    }
}

void arm_writeh(uint32_t address, uint16_t value) {
    uint32_t a = address & ~1;

    arm_write_(a | 0, 0, (uint8_t)(value >> 0));
    arm_write_(a | 1, 1, (uint8_t)(value >> 8));
}

void arm_write(uint32_t address, uint32_t value) {
    uint32_t a = address & ~3;

    arm_write_(a | 0, 0, (uint8_t)(value >>  0));
    arm_write_(a | 1, 1, (uint8_t)(value >>  8));
    arm_write_(a | 2, 2, (uint8_t)(value >> 16));
    arm_write_(a | 3, 3, (uint8_t)(value >> 24));
}

void arm_writeb_n(uint32_t address, uint8_t value) {
    arm_access_bus(address, ARM_BYTE_SZ, NON_SEQ);

    arm_writeb(address, value);
}

void arm_writeh_n(uint32_t address, uint16_t value) {
    arm_access_bus(address, ARM_HWORD_SZ, NON_SEQ);

    arm_writeh(address, value);
}

void arm_write_n(uint32_t address, uint32_t value) {
    arm_access_bus(address, ARM_WORD_SZ, NON_SEQ);

    arm_write(address, value);
}

void arm_writeb_s(uint32_t address, uint8_t value) {
    arm_access_bus(address, ARM_BYTE_SZ, SEQUENTIAL);

    arm_writeb(address, value);
}

void arm_writeh_s(uint32_t address, uint16_t value) {
    arm_access_bus(address, ARM_HWORD_SZ, SEQUENTIAL);

    arm_writeh(address, value);
}

void arm_write_s(uint32_t address, uint32_t value) {
    arm_access_bus(address, ARM_WORD_SZ, SEQUENTIAL);

    arm_write(address, value);
}