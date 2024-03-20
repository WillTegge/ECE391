/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _I8259_H
#define _I8259_H

#include "types.h"

/* Ports that each PIC sits on */
#define MASTER_8259_PORT    0x20
#define SLAVE_8259_PORT     0xA0

#define MASTER_DATA_PORT         (MASTER_8259_PORT + 1)
#define SLAVE_DATA_PORT          (SLAVE_8259_PORT + 1)



// When you enter protected mode (or even before hand, if you're not using GRUB) 
// the first command you will need to give the two PICs is the initialise command (code 0x11). 
// This command makes the PIC wait for 3 extra "initialisation words" on the data port. 
// These bytes give the PIC:
//                              Its vector offset. (ICW2)
//                              Tell it how it is wired to master/slaves. (ICW3)
//                              Gives additional information about the environment. (ICW4)

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1                0x11
#define ICW2_MASTER         0x20
#define ICW2_SLAVE          0x28
#define ICW3_MASTER         0x04
#define ICW3_SLAVE          0x02
#define ICW4                0x01

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI                 0x60



#define MASK_FOR_INTERRUPTS 0xFF

#define PIC_EOI		0x20		/* End-of-interrupt command code */

#define IRQ2 2

#define AMOUNT_OF_PORTS_ON_PIC  8


/* Externally-visible functions */

/* Initialize both PICs */
void i8259_init(void);
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num);
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num);
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num);

#endif /* _I8259_H */
