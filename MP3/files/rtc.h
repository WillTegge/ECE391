#include "types.h"


// RTC uses 2 IO ports -> 0x70 and 0x71
// 0x70: Port 0x70 is used to specify an index or "register number", and to disable NMI
// 0x71: Port 0x71 is used to read or write from/to that byte of CMOS configuration space

#define OUT_PORT       0x70  
#define IN_PORT        0x71
#define IRQ8 8

// Only three bytes of CMOS RAM are used to control the RTC periodic interrupt function
// They are called RTC Status Register A, B, and C. They are at offset 0xA, 0xB, and 0xC in the CMOS RAM
#define STATUS_REGISTER_A      0x8A
#define STATUS_REGISTER_B      0x8B
#define STATUS_REGISTER_C      0x8C

#define TURN_ON_BIT_6   0x40

void rtc_init(void);


// for checkpoint 2

#define MAX_INTEGER     0x7FFFFFFF

#define HEX_16_VALUE   0x0F

#define MASK_TO_GET_TOP_4_BITS   0xF0

#define MAX_FREQUENCY 1024
#define MIN_FREQUENCY 2
#define MAX_RATE    15
#define MIN_RATE    3

// int32_t rtc_interrupt_occured_flag = 0;
int32_t rtc_interrupt_occured_flag;
int32_t rtc_max_counter;

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t rtc_open(const uint8_t* filename);

int32_t rtc_close(int32_t fd);

int32_t rtc_set_specific_frequency(int32_t freq);

int32_t rtc_calculate_power_of_2(int32_t freq);



