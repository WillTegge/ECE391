/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
/*
 * i8259_init
 *   DESCRIPTION: Initialize the 8259 PIC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: assigns the master and slave pics. Assigns the ports of the pic and sets the interrupts. 
 *                 enables interrupts on the secondary
 */
void i8259_init(void) {

    outb(MASK_FOR_INTERRUPTS, MASTER_DATA_PORT);

    outb(MASK_FOR_INTERRUPTS, SLAVE_DATA_PORT);

    // setup master
    // send initialization command words
    outb(ICW1, MASTER_8259_PORT);

    outb(ICW2_MASTER, MASTER_DATA_PORT);

    outb(ICW3_MASTER, MASTER_DATA_PORT);

    outb(ICW4, MASTER_DATA_PORT);

    // setup slave
    // send initialization command words
    outb(ICW1, SLAVE_8259_PORT);

    outb(ICW2_SLAVE, SLAVE_DATA_PORT);

    outb(ICW3_SLAVE, SLAVE_DATA_PORT);

    outb(ICW4, SLAVE_DATA_PORT);


    // set masks for the data
    outb(MASK_FOR_INTERRUPTS, MASTER_DATA_PORT);

    outb(MASK_FOR_INTERRUPTS, SLAVE_DATA_PORT);

    // enable secondary pic interrupts
    enable_irq(IRQ2);

}

// code/algorithm found at https://wiki.osdev.org/PIC

/* Enable (unmask) the specified IRQ */
/*
 * enable_irq
 *   DESCRIPTION: Enable (unmask) the specified IRQ
 *   INPUTS: specific IRQ number
 *   OUTPUTS: changes the value of a given port
 *   RETURN VALUE: none
 *   SIDE EFFECTS: enables (unmasks) interrupts on the given IRQ number. 
 */
void enable_irq(uint32_t irq_num) {

    uint16_t port;
    uint8_t value;

    if(irq_num < AMOUNT_OF_PORTS_ON_PIC) {                   // 8 ports on the primary pic
        port =  MASTER_DATA_PORT;
    } else {
        port = SLAVE_DATA_PORT;
        irq_num -= AMOUNT_OF_PORTS_ON_PIC;
    }

    value = inb(port) & ~(1 << irq_num);                // shift by 1
       
    outb(value, port);
}

// code/algorithm found at https://wiki.osdev.org/PIC

/* Disable (mask) the specified IRQ */
/*
 * disable_irq
 *   DESCRIPTION: Disable (mask) the specified IRQ
 *   INPUTS: specific IRQ number
 *   OUTPUTS: changes the value of a given port
 *   RETURN VALUE: none
 *   SIDE EFFECTS: disables (masks) interrupts on the given IRQ number. 
 */
void disable_irq(uint32_t irq_num) {
    

    uint16_t port;
    uint8_t value;

 
    if(irq_num < AMOUNT_OF_PORTS_ON_PIC) {                    // 8 ports on the primary pic
        port = MASTER_DATA_PORT;
    } else {
        port = SLAVE_DATA_PORT;
        irq_num -= AMOUNT_OF_PORTS_ON_PIC;
    }

    value = inb(port) | (1 << irq_num);             // shift by 1

    outb(value, port);   


}




/* Send end-of-interrupt signal for the specified IRQ */
/*
 * send_eoi
 *   DESCRIPTION: Send end-of-interrupt signal for the specified IRQ 
 *   INPUTS: specific IRQ number
 *   OUTPUTS: sends the end-of-interrupt signal
 *   RETURN VALUE: none
 *   SIDE EFFECTS: sends the end of interrupt signal to the given IRQ
 */
void send_eoi(uint32_t irq_num) {

	if(irq_num >= AMOUNT_OF_PORTS_ON_PIC){

		//outb(SLAVE_8259_PORT, PIC_EOI);
        outb(PIC_EOI | (irq_num & 7), SLAVE_8259_PORT);
        outb(PIC_EOI | 2 , MASTER_8259_PORT);                       // OR with 2
        
    }else{
        outb(PIC_EOI | irq_num, MASTER_8259_PORT);
    }
}
