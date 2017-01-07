#include <stdint.h>
#include <stdbool.h>

#define ROR(val, s)  (((val) >> (s)) | ((val) << (32 - (s))))
#define SBIT(op)     ((op & (1 << 20)) ? 1 : 0)

#define ARM_BYTE_SZ   1
#define ARM_HWORD_SZ  2
#define ARM_WORD_SZ   4
#define ARM_DWORD_SZ  8

//PSR flags
#define ARM_N  (1 << 31) //Negative
#define ARM_Z  (1 << 30) //Zero
#define ARM_C  (1 << 29) //Carry
#define ARM_V  (1 << 28) //Overflow
#define ARM_Q  (1 << 27) //Saturation
#define ARM_A  (1 <<  8) //Abort off
#define ARM_I  (1 <<  7) //IRQ off
#define ARM_F  (1 <<  6) //FIQ off
#define ARM_T  (1 <<  5) //Thumb

//Modes
#define ARM_USR  0b10000 //User
#define ARM_FIQ  0b10001 //Fast IRQ
#define ARM_IRQ  0b10010 //IRQ
#define ARM_SVC  0b10011 //Supervisor Call
#define ARM_MON  0b10110 //Monitor
#define ARM_ABT  0b10111 //Abort
#define ARM_UND  0b11011 //Undefined
#define ARM_SYS  0b11111 //System

//Interrupt addresses
#define ARM_VEC_RESET   0x00 //Reset
#define ARM_VEC_UND     0x04 //Undefined
#define ARM_VEC_SVC     0x08 //Supervisor Call
#define ARM_VEC_PABT    0x0c //Prefetch Abort
#define ARM_VEC_DABT    0x10 //Data Abort
#define ARM_VEC_ADDR26  0x14 //Address exceeds 26 bits (legacy)
#define ARM_VEC_IRQ     0x18 //IRQ
#define ARM_VEC_FIQ     0x1c //Fast IRQ

typedef union {
    int32_t w;

    struct {
        int16_t lo;
        int16_t hi;
    } h;

    struct {
        int8_t b0;
        int8_t b1;
        int8_t b2;
        int8_t b3;
    } b;
} arm_word;

typedef struct {
    uint32_t r[16];

    uint32_t r8_usr;
    uint32_t r9_usr;
    uint32_t r10_usr;
    uint32_t r11_usr;
    uint32_t r12_usr;
    uint32_t r13_usr;
    uint32_t r14_usr;

    uint32_t r8_fiq;
    uint32_t r9_fiq;
    uint32_t r10_fiq;
    uint32_t r11_fiq;
    uint32_t r12_fiq;
    uint32_t r13_fiq;
    uint32_t r14_fiq;

    uint32_t r13_irq;
    uint32_t r14_irq;

    uint32_t r13_svc;
    uint32_t r14_svc;

    uint32_t r13_mon;
    uint32_t r14_mon;

    uint32_t r13_abt;
    uint32_t r14_abt;

    uint32_t r13_und;
    uint32_t r14_und;

    uint32_t cpsr;

    uint32_t spsr_fiq;
    uint32_t spsr_irq;
    uint32_t spsr_svc;
    uint32_t spsr_abt;
    uint32_t spsr_und;
    uint32_t spsr_mon;
} arm_regs_t;

arm_regs_t arm_r;

uint32_t arm_op;
uint32_t arm_pipe[2];
uint32_t arm_cycles;

bool int_halt;
bool pipe_reload;

void arm_init();
void arm_uninit();

void arm_exec(uint32_t target_cycles);

void arm_int(uint32_t address, int8_t mode);

void arm_check_irq();

void arm_reset();