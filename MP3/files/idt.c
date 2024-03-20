//Set up for IDT 

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "idt.h"
#include "rtc.h"
#include "keyboard.h"
#include "systemcall.h"
#include "PIT.h"

/*
Each entry in IDT contains pointer to corresponding interrupt handler f(n) to run when recieved

take care of :
    exception handlers (Privelage level = 0)
        DPL = 0
    hardware interrupt handlers (Privelage level = 0)
        DPL = 0
    system call handlers (User-level program executes: int $0x80)
        DPL = 3
each IDT entry contains segment selector field specifiying code segment on GDT; so 
set field to be kernal's code segment descriptor
*/

extern void idt_init(void){ 
    int i;
    memset(idt, 0,sizeof(*idt)*256);
    
    for(i = 0; i < NUM_VEC; i++){
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        idt[i].reserved3 = 0;
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size      = 1;
        idt[i].reserved0 = 0;
        idt[i].dpl       = 0;
        idt[i].present   = 1;
        if(i == system_call_vec){
            idt[i].dpl   = 3;
        }
        
    }

    // Intel Reserved Interruputs
    SET_IDT_ENTRY(idt[0], irq_0);
    SET_IDT_ENTRY(idt[1], irq_1);
    SET_IDT_ENTRY(idt[2], irq_2);
    SET_IDT_ENTRY(idt[3], irq_3);
    SET_IDT_ENTRY(idt[4], irq_4);
    SET_IDT_ENTRY(idt[5], irq_5);
    SET_IDT_ENTRY(idt[6], irq_6);
    SET_IDT_ENTRY(idt[7], irq_7);
    SET_IDT_ENTRY(idt[8], irq_8);
    SET_IDT_ENTRY(idt[9], irq_9);
    SET_IDT_ENTRY(idt[10], irq_10);
    SET_IDT_ENTRY(idt[11], irq_11);
    SET_IDT_ENTRY(idt[12], irq_12);
    SET_IDT_ENTRY(idt[13], irq_13);
    SET_IDT_ENTRY(idt[14], irq_14);
    SET_IDT_ENTRY(idt[16], irq_16);
    SET_IDT_ENTRY(idt[17], irq_17);
    SET_IDT_ENTRY(idt[18], irq_18);
    SET_IDT_ENTRY(idt[19], irq_19);

    //PIC Interrupts
    SET_IDT_ENTRY(idt[primary_pic], pit_handler_asm);
    
    SET_IDT_ENTRY(idt[primary_pic+1], keyboard_handler_asm);

    SET_IDT_ENTRY(idt[secondary_pic], rtc_handler_asm);

    // System Call 
    SET_IDT_ENTRY(idt[system_call_vec], systemcall_handler);  

    lidt(idt_desc_ptr);
}


/*
	rtc: this is the RTC handler in charge of the real time clock,
    sending the end of interrupt and clearing the RTC registers to 
    take another
	Inputs:	NONE
	OUTPUTS: NONE
	Side Effects: NONE
*/
extern void rtc(){
    // printf("RTC")
    cli();
    // if(rtc_count > 0){
    //     // test_interrupts();
    //     rtc_count--;


    //     // added this if statement for rtc checkpoint 2 stuff
    //     if(rtc_count == 0){

            rtc_interrupt_occured_flag = 1;            // added this flag for testing rtc
            // rtc_count = rtc_max_counter;              // added this for testing rtc  
    //         rtc_count = 5;
    //         // printf("resetting the rtc count");

    //     }

        
    // }
    send_eoi(IRQ8);    //use macro
    outb(STATUS_REGISTER_C, OUT_PORT);
    inb(IN_PORT);
    sti();
}

/*
	keyboard: this is the keyboard_handler, we just have an extra 
    layer of calling it from idt
	Inputs:	NONE
	OUTPUTS: NONE
	Side Effects: NONE
*/
extern void keyboard(){
    keyboard_handler();
}

extern void pit(){
    PIT_handler();
}


/* SYSTEM CALLS AND INTERRUPTS*/
extern void sys_call(){
    // saved contents of CS
    printf("System Call");
    while(1);
}

extern void irq_0(){
    // saved contents of CS
    printf("Interrupt 0: Divide Error Exception");
    printf("\n");
    
    halt(exception);
}
extern void irq_1(){
    printf("Interrupt 1: Debug Exception");
    printf("\n");
    halt(exception);
}
extern void irq_2(){
    printf("Interrupt 2: NMI Interrupt");
    printf("\n");
    halt(exception);
}
extern void irq_3(){
    printf("Interrupt 3: Breakpoint Exception");
    printf("\n");
    halt(exception);
}
extern void irq_4(){
    printf("Interrupt 4: Overflow Exception");
    printf("\n");
    halt(exception);
}
extern void irq_5(){
    printf("Interrupt 5: BOUND Range Exceeded Exception");
    printf("\n");
    halt(exception);
}

extern void irq_6(){
    printf("Interrupt 6: Invalid Opcode Exception");
    printf("\n");
    halt(exception);
}

extern void irq_7(){
    printf("Interrupt 7: Device Not Available Exception");
    printf("\n");
    halt(exception);
}

extern void irq_8(){
    printf("Interrupt 8: Double Fault Exception");
    printf("\n");
    halt(exception);
}

extern void irq_9(){
    printf("Interrupt 9: Coprocessor Segement Overrun");
    printf("\n");
    halt(exception);
}

extern void irq_10(){
    printf("Interrupt 10: Invalid TSS Exception");
    printf("\n");
    halt(exception);
}

extern void irq_11(){
    printf("Interrupt 11: Segment Not Present");
    printf("\n");
    halt(exception);
}

extern void irq_12(){
    printf("Interrupt 12: Stack Fault Exception");
    printf("\n");
    halt(exception);
}

extern void irq_13(){
    printf("Interrupt 13: General Protection Exception");
    printf("\n");
    halt(exception);
}

extern void irq_14(){
    printf("Interrupt 14: Page Fault Exception");
    printf("\n");
    halt(exception);
}

extern void irq_16(){
    printf("Interrupt 16: x87 FPU Floating-Point Error");
    printf("\n");
    halt(exception);
}

extern void irq_17(){
    printf("Interrupt 17: Allignment Check Excpetion");
    printf("\n");
    halt(exception);
}

extern void irq_18(){
    printf("Interrupt 18: Machine-Check Exception");
    printf("\n");
    halt(exception);
}

extern void irq_19(){
    printf("Interrupt 19: SIMD Floating-Point Exception");
    printf("\n");
    halt(exception);
}

