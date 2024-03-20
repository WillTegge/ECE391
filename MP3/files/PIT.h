#include "types.h"

// Each 8 bit data port is the same, and is used to set the counter's 16 bit reload value or read the channel's 16 bit current count 
#define CHANNEL_0           0x40
#define CHANNEL_1           0x41
#define CHANNEL_2           0x42

// The Mode/Command register at I/O address 0x43 contains the following:
// Bits         Usage
// 6 and 7      Select channel :
//                 0 0 = Channel 0
//                 0 1 = Channel 1
//                 1 0 = Channel 2
//                 1 1 = Read-back command (8254 only)
// 4 and 5      Access mode :
//                 0 0 = Latch count value command
//                 0 1 = Access mode: lobyte only
//                 1 0 = Access mode: hibyte only
//                 1 1 = Access mode: lobyte/hibyte
// 1 to 3       Operating mode :
//                 0 0 0 = Mode 0 (interrupt on terminal count)
//                 0 0 1 = Mode 1 (hardware re-triggerable one-shot)
//                 0 1 0 = Mode 2 (rate generator)
//                 0 1 1 = Mode 3 (square wave generator)
//                 1 0 0 = Mode 4 (software triggered strobe)
//                 1 0 1 = Mode 5 (hardware triggered strobe)
//                 1 1 0 = Mode 2 (rate generator, same as 010b)
//                 1 1 1 = Mode 3 (square wave generator, same as 011b)
// 0            BCD/Binary mode: 0 = 16-bit binary, 1 = four-digit BCD
#define COMMAND_REGISTER    0x43
#define LOW_BYTE 0x00FF
#define SET_HIGH 8
#define MODE3_LOWHIGH_CHANNEL0 0x36
#define COUNTER 0x2E9C
#define PIT_IRQ 0
#define FREQ 100
#define COUNT_MAX 1191380

#define NUMBER_OF_TERMS 3


// global structs to save esp, ebp, ss0, and esp0
typedef struct __attribute__((packed)) saved_tasks_t{
    uint32_t saved_esp;
    uint32_t saved_ebp;
    uint32_t saved_esp0;
    uint16_t saved_ss0;
}saved_tasks_t;

saved_tasks_t saved_tasks[3];



void PIT_init();

void PIT_handler();

int32_t PIT_read(int32_t fd, void* buf, int32_t nbytes);

int32_t PIT_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t PIT_open(const uint8_t* filename);

int32_t PIT_close(int32_t fd);
