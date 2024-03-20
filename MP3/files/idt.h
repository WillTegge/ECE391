#ifndef ASM 

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "types.h"

// #define ASM
#define exception 256
#define primary_pic 0x20
#define secondary_pic 0x28
#define system_call_vec 0x80


extern void idt_init(void);
extern void sys_call();
extern void irq_0();
extern void irq_1();
extern void irq_2();
extern void irq_3();
extern void irq_4();
extern void irq_5();
extern void irq_6();
extern void irq_7();
extern void irq_8();
extern void irq_9();
extern void irq_10();
extern void irq_11();
extern void irq_12();
extern void irq_13();
extern void irq_14();
extern void irq_16();
extern void irq_17();
extern void irq_18();
extern void irq_19();


extern void keyboard();
extern void rtc();
extern void pit();

extern void keyboard_handler_asm();
extern void rtc_handler_asm();
extern void pit_handler_asm();

#endif /*asm*/
