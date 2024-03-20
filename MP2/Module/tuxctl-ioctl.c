/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */
//

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

/************************ Protocol Implementation *************************/

// added these variables
static volatile int ack = 0;					// acknowledge flag/signal
static volatile uint8_t buttons = 0;			// global variable to save a copy of the state of the buttons	- exactly 1 byte in length
static volatile uint32_t old_LED_values	= 0;	// global variable to save a copy of the LEDs 



// added these functions
int tux_init(struct tty_struct* tty);

int tux_buttons(struct tty_struct* tty, unsigned long* arg);

uint8_t tux_set_leds_helper(uint8_t led_x_set);

int tux_set_leds(struct tty_struct* tty, unsigned long arg);




/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */

  
/*
 * tuxctl_handle_packet
 *   DESCRIPTION: handles a packet given to it 
 *   INPUTS: tty -- given from the dispatcher, struct
			 packet -- 3 byte packet with info/commands to handle
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: reset leds if a reset is called, set the button state if buttons are pressed, and set the acknowledge flag if neccesary
 */

void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];


	// check a for reset, button press, or an acknowledge 
	// MTCP_RESET, MTCP_BIOC_EVENT, MTCP_ACK

	if(a == MTCP_RESET){		// check if we are resetting. 

		// call the tux_init function. reinitialize tux
		// call the set_led function to reset/update the LEDs with the saved old LEDs value (global)
		
		tux_init(tty);							// call init function
		tux_set_leds(tty, old_LED_values);		// set the LEDs to the old value by claling set_led function

	}



// 	; MTCP_BIOC_EVT	
// 	;	Generated when the Button Interrupt-on-change mode is enabled and 
// 	;	a button is either pressed or released.
// 	;
// 	; 	Packet format:
// 	;		Byte 0 - MTCP_BIOC_EVENT
// 	;		byte 1  +-7-----4-+-3-+-2-+-1-+---0---+
// 	;			| 1 X X X | C | B | A | START |
// 	;			+---------+---+---+---+-------+
// 	;		byte 2  +-7-----4-+---3---+--2---+--1---+-0--+
// 	;			| 1 X X X | right | down | left | up |
// 	;			+---------+-------+------+------+----+

	if(a == MTCP_BIOC_EVENT){			// check if a button was pressed. If yes, then we need to format the data

		uint8_t rdlu_data = c & 0xF;	// getting the Right,down,left,up bits from bottom 4 bits of c

		uint8_t down_ = rdlu_data & 0x04;	// & with 0100 to get the bit for down

		uint8_t left_ = rdlu_data & 0x02;	// & with 0010 to get the bit for left

		buttons = ((rdlu_data & 0x8) << 4) | (left_ << 5) | (down_ << 3) | ((rdlu_data & 0x1) << 4) | (0xF & b);
		// shifting the button values based on where they should be in the 8-bit format specified below
		// buttons 8-bit format -> R L D U C B A S

	}

	if(a == MTCP_ACK){

		ack = 1;			// set the ack flag

	}

	

    /*printk("packet : %x %x %x\n", a, b, c); */
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/

 
/*
 * tuxctl_ioctl
 *   DESCRIPTION: calls an ioctl function based on a cmd passed in
 *   INPUTS: tty -- given from the dispatcher, struct
			 file -- file struct
			 cmd -- value to tell which ioctl needs to be called
			 arg -- info given to the ioctls if needed
 *   OUTPUTS: none
 *   RETURN VALUE: returns 0 if it was successful in giving the buf to the kernel
 *   SIDE EFFECTS: turns on button interrupt mode, and puts the LEDs in user set mode
 */

int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
    switch (cmd) {
	case TUX_INIT:
		return tux_init(tty);			// call tux_init function

	case TUX_BUTTONS:
		return tux_buttons(tty, (unsigned long*)arg);			// call buttons function

	case TUX_SET_LED:
		return tux_set_leds(tty, arg);				// call the set_leds function

	case TUX_LED_ACK:
		// return 0;
	case TUX_LED_REQUEST:
		// return 0;
	case TUX_READ_LED:
		// return 0;
	default:
	    return -EINVAL;
    }
}




/*
 * tux_init
 *   DESCRIPTION: initializes the tux driver and all variables and register values we need
 *   INPUTS: tty -- given from the dispatcher
 *   OUTPUTS: none
 *   RETURN VALUE: returns 0 if it was successful in giving the buf to the kernel
 *   SIDE EFFECTS: turns on button interrupt mode, and puts the LEDs in user set mode
 */
int tux_init(struct tty_struct* tty){

	char buf[2];					// this is the buffer we use to pass commands to kernel from the driver

	buf[0] = MTCP_BIOC_ON;			// turns button interrupt mode on
	buf[1] = MTCP_LED_USR;			// puts the LEDs in user mode

	ack = 1;						// initialize the acknowledge flag to 1

	return tuxctl_ldisc_put(tty, buf, 2);		// call ldisc_put to give buffer to the kernel
	
}


/*
 * tux_buttons
 *   DESCRIPTION: gives the button state to the user
 *   INPUTS: tty -- given from the dispatcher
			 arg -- info about which buttons are pressed 
 *   OUTPUTS: none
 *   RETURN VALUE: returns 0 upon success or -EINVAL if the arg is invalid 
 *   SIDE EFFECTS: user has current button state
 */
int tux_buttons(struct tty_struct* tty, unsigned long* arg){

	// check for invalid argument
	if(arg == 0){	

		return -EINVAL;

	}

	// call copy to user to give user the button state 
	copy_to_user((unsigned int *)arg, (const void*)&buttons, 2);			// 2 is how many bytes we are copying		
	
	return 0;

}


//  	Mapping from 7-segment to bits
//  	The 7-segment display is:
// 		  _A
// 		F| |B
// 		  -G
// 		E| |C
// 		  -D .dp
// 
//  	The map from bits to segments is:
//  
//  	__7___6___5___4____3___2___1___0__
//  	| A | E | F | dp | G | C | B | D | 
//  	+---+---+---+----+---+---+---+---+

// helper function that returns the hex values to display each of the values given

/*
 * tux_set_leds_helper
 *   DESCRIPTION: helper function that gets the bit value to set the led
 *   INPUTS: led_x_set -- led value we want to display 
 *   OUTPUTS: none
 *   RETURN VALUE: bit value of the led value passed in
 *   SIDE EFFECTS: none
 */
uint8_t tux_set_leds_helper(uint8_t led_x_set){

	// huge if statment to output the LED bit values based on the value we want to display
	// done all possible combinations from 0 - F
	if(led_x_set == 0){
		return 0xE7;
	}
	else if(led_x_set == 1){
		return 0x06;
	}
	else if(led_x_set == 2){
		return 0xCB;
	}
	else if(led_x_set == 3){
		return 0x8F;
	}
	else if(led_x_set == 4){
		return 0x2E;
	}
	else if(led_x_set == 5){
		return 0xAD;
	}
	else if(led_x_set == 6){
		return 0xED;
	}
	else if(led_x_set == 7){
		return 0x86;
	}
	else if(led_x_set == 8){
		return 0xEF;
	}
	else if(led_x_set == 9){
		return 0xAE;
	}
	else if(led_x_set == 10){
		return 0xEE;
	}
	else if(led_x_set == 11){
		return 0x6D;
	}
	else if(led_x_set == 12){
		return 0xE1;
	}
	else if(led_x_set == 13){
		return 0x4F;
	}
	else if(led_x_set == 14){
		return 0xE9;
	}
	else if(led_x_set == 15){
		return 0xE8;
	}

	return 0x00;

}


/*
 * tux_set_leds
 *   DESCRIPTION: creates a buffer with the led values to be displayed on the leds
 *   INPUTS: tty -- given from the dispatcher
			 arg -- contains info on which leds need to be shown and the values that should be displayed
 *   OUTPUTS: shows value on the leds
 *   RETURN VALUE: returns 0
 *   SIDE EFFECTS: led displays may change
 */
int tux_set_leds(struct tty_struct* tty, unsigned long arg){

	uint8_t led_0_set;			// holds the LED 0 data
	uint8_t led_1_set;			// holds the LED 1 data
	uint8_t led_2_set;			// holds the LED 2 data
	uint8_t led_3_set;			// holds the LED 3 data

	uint16_t displayed;			// holds the values to be displayed on the LEDs
	uint8_t shown;				// variable to hold which LEDs are to be shown
	uint32_t temp;	

	uint8_t decimal_point;		// holds the info about which decimal points to turn on

	char buf[6];				// buffer to hold 6 commands to give to the kernel


	if(ack != 1){			// check the acknowledge flag
		return 0;
	}

	// store the old LED values in case of a reset 
	old_LED_values = arg;


	led_0_set = 0;
	led_1_set = 0;
	led_2_set = 0;
	led_3_set = 0;


	displayed = arg & 0xFFFF;			// get the lower 16 bits of arg (holds values to be displayed on LEDs)

	temp = (arg >> 16) & 0xF;			// bit shift and get the bottom byte into temp

	shown = temp & 0xF;					// get which LEDs should be shown (4 bits)

	decimal_point = (arg & 0x0F000000) >> 24;		// get info about which decimal points should be on


	// need to pass in command to set LEDs
	buf[0] = MTCP_LED_SET;

	// pass in which LEDs we want ON -> TA said set them all to be ON at the start
	buf[1] = 0xF;


	// need a helper function or lookup table to decide what LED bit values to put into the buffer

	if((shown & 0x1) == 1){		// check if the LED0 is turned on.  0x1 to check the least significant bit in the byte
		
		led_0_set = displayed & 0xF;
		led_0_set = tux_set_leds_helper(led_0_set);
		led_0_set = led_0_set | ((decimal_point & 0x1) << 4);			// this determines if the decimal needs to be turned on

	}

	if((shown & 0x2) == 2){		// check if the LED1 is turned on.  0x2 to check the 2nd least significant bit in the byte
		
		led_1_set = (displayed & 0xF0) >> 4;
		led_1_set = tux_set_leds_helper(led_1_set);
		led_1_set = led_1_set | ((decimal_point & 0x2) << 3);			// this determines if the decimal needs to be turned on

	}

	if((shown & 0x4) == 4){		// check if the LED2 is turned on.  0x4 to check the 3rd least significant bit in the byte
		
		led_2_set = (displayed & 0xF00) >> 8;
		led_2_set = tux_set_leds_helper(led_2_set);
		led_2_set = led_2_set | ((decimal_point & 0x4) << 2);			// this determines if the decimal needs to be turned on

	}

	if((shown & 0x8) == 8){		// check if the LED3 is turned on.  0x8 to check the 4th bit in the byte
		
		led_3_set = (displayed & 0xF000) >> 12;
		led_3_set = tux_set_leds_helper(led_3_set);
		led_3_set = led_3_set | ((decimal_point & 0x8) << 1);			// this determines if the decimal needs to be turned on

	}


	// fill in the buffer with these LED settings 
	buf[2] = led_0_set;
	buf[3] = led_1_set;
	buf[4] = led_2_set;
	buf[5] = led_3_set;


	// set the acknowledge flag to 0 so that TUX shows it is busy and the LEDs cant be messed up
	ack = 0;

	tuxctl_ldisc_put(tty, buf, 6);			// call ldisc_put to give buffer to kernel

	return 0;

}


