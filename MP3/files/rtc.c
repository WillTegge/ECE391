#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "tests.h"


// algorithm taken from os dev rtc section
// disable_ints();			        // disable interrupts
// outportb(0x70, 0x8B);		    // select register B, and disable NMI
// char prev=inportb(0x71);	        // read the current value of register B
// outportb(0x70, 0x8B);		    // set the index again (a read will reset the index to register D)
// outportb(0x71, prev | 0x40);	    // write the previous value ORed with 0x40. This turns on bit 6 of register B
// enable_ints();

/*
 * rtc_init
 *   DESCRIPTION: Initialize the RTC device
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: NONE
 *   SIDE EFFECTS: Disables interrupts, and sets the RTC Status Registers (A, B, C)
 */
void rtc_init(void) {

    disable_irq(IRQ8);
    
    outb(STATUS_REGISTER_B, OUT_PORT);
    
    char prev = inb(IN_PORT);

    outb(STATUS_REGISTER_B, OUT_PORT);
    
    outb(prev | TURN_ON_BIT_6, IN_PORT);


    rtc_max_counter = MAX_FREQUENCY / MIN_FREQUENCY;
    
    enable_irq(IRQ8);

    rtc_set_specific_frequency(MAX_FREQUENCY);

    
}


// notes on rtc_read
// Unless otherwise specified, successful calls should return 0, and failed calls should return -1
// this call should always return 0, but only after an interrupt has occurred 
// (set a flag and wait until the interrupt handler clears it, then return 0).
// must only return once the RTC interrupt occurs

/*
 * rtc_read
 *   DESCRIPTION: Wait until an interrupt occurs 
 *   INPUTS: fd - file descriptor
 *           buf - buffer 
 *           nbytes - number of bytes 
 *   OUTPUTS: NONE
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: Messes with the rtc_interrupt_occured_flag
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    sti();
    rtc_interrupt_occured_flag = 0;

	while(1){               // "Spin" until an interrupt is recieved 

		if(rtc_interrupt_occured_flag == 1){
            break;
        }

	}

	return 0;

}


// notes on rtc_write
// writes data to the rtc device
// should always accept only a 4-byte integer specifying the interrupt rate in Hz
// should set the rate of periodic interrupts accordingly
// must get its input parameter through a buffer and not read the value directly

/*
 * rtc_write(int32_t fd, const void* buf, int32_t nbytes)
 *   DESCRIPTION: Writes a specific fequency to the rtc device
 *   INPUTS: fd - file descriptor
 *           buf - buffer containing the frequency we want to write to the rtc device
 *           nbytes - number of bytes 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 or -1
 *   SIDE EFFECTS: Changes the frequency of the rtc device
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){                         // what is fd and nbytes used for???

    // calculate new frequency to set
    // set the new calculated frequency
    // return 0 or -1

    // added checks to see if it fixes anything
    if(buf == NULL){

        return -1;

    }

    if(nbytes != sizeof(int32_t)){

        return -1;

    }

    int32_t freq = *((int32_t*) buf);           // cast buf as a pointer to a int32_t pointer to the buf         
                                                // am I getting the correct freq/arg from buffer?

    // check if the freq is valid
    if(freq > MAX_FREQUENCY || freq < MIN_FREQUENCY){

        return -1;

    }

    // make sure freq is a power of two
    if((freq & (freq - 1)) != 0){

        return -1;

    } 

	return rtc_set_specific_frequency(freq);        // should return 0 or -1 ???

}


// should reset the frequency to 2Hz
// provides access to the file system
// set up any data necessary to handle the given type of file
// If the named file does not exist or no descriptors are free, the call returns -1

/*
 * rtc_open
 *   DESCRIPTION: Opens and initializes the rtc device with a frequency of 2
 *   INPUTS: filename - name of a file 
 *   OUTPUTS: NONE
 *   RETURN VALUE: 0 or -1
 *   SIDE EFFECTS: Sets the frequency of the rtc device to 2
 */
int32_t rtc_open(const uint8_t* filename){

    rtc_init();
    enable_irq(IRQ8);
    // reset the frequency to 2Hz
    int32_t result = 0;
    result = rtc_set_specific_frequency(2);
    open_frequency = MIN_FREQUENCY;
    return result;
    
}

// does nothing
/*
 * rtc_close
 *   DESCRIPTION: does nothing for the rtc (returns 0)
 *   INPUTS: fd - file descriptor
 *   OUTPUTS: NONE
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: NONE
 */
int32_t rtc_close(int32_t fd){              // fd is file descriptor #

    return 0;
 
}




// NOTES ON SETTING THE FREQUENCY
// The lower 4 bits of register A is the divider value. The default is 0110b, or 6. The setting must be a value from 1 to 15. A value of 0 disables the interrupt.
// if freq = 1024 -> rate is 10 ?       since 2^10 = 1024
// frequency =  32768 >> (rate-1);

// The RTC device itself can only generate interrupts at a rate that is a power of 2 (do a parameter check), and only
// up to 8192 Hz. 

// Your kernel should limit this further to 1024 Hz 
// since an operating system shouldn’t allow user space programs to generate more than 1024 interrupts per second by default

// frequency must be a power of 2

/*
 * rtc_set_specific_frequency
 *   DESCRIPTION: Sets the frequency of the rtc device to the given value
 *   INPUTS: freq - frequency that we want to set the rtc device to 
 *   OUTPUTS: NONE
 *   RETURN VALUE: 0 or -1
 *   SIDE EFFECTS: Changes the frequency of the rtc device to the given value
 */
int32_t rtc_set_specific_frequency(int32_t freq){

    // check for valid frequency  (min of 2 and max of 1024 for the kernel)
    if(freq < MIN_FREQUENCY || freq > MAX_FREQUENCY){

        return -1;                  // invalid so return -1

    }

    // check for a valid power of 2 for the frequency (turn it into a power of 2)
    int32_t freq_as_power_of_2;
    if((freq != 0) && ((freq & (freq - 1)) == 0)){              // this checks if the freq is a power of 2 or not

        freq_as_power_of_2 = rtc_calculate_power_of_2(freq);            // get the power of 2 of the current frequency

    }
    else{

        return -1;

    }


    // algorithm taken from os dev
    // to change the frequency:
    //              rate &= 0x0F;			// rate must be above 2 and not over 15
    //              disable_ints();
    //              outportb(0x70, 0x8A);		// set index to register A, disable NMI
    //              char prev=inportb(0x71);	// get initial value of register A
    //              outportb(0x70, 0x8A);		// reset index to A
    //              outportb(0x71, (prev & 0xF0) | rate); //write only our rate to A. Note, rate is the bottom 4 bits.
    //              enable_ints();

    // rate must be above 2 and not over 15
    int32_t rate = MAX_RATE - freq_as_power_of_2 + 1;               // (os dev) -> frequency =  32768 >> (rate-1);
    rate = rate & HEX_16_VALUE;                                     // get the least significant 4 bits

    if(rate < MIN_RATE){

        return -1; 

    }

    cli();

    outb(STATUS_REGISTER_A, OUT_PORT);

    char prev = inb(IN_PORT);

    outb(STATUS_REGISTER_A, OUT_PORT);

    outb(((prev & MASK_TO_GET_TOP_4_BITS) | rate), IN_PORT);

    sti();


    return 0;
    
}



// The log base 2 to a number N in algebra is equal to the exponent value of 2 which gives the number N
// if this function is called, freq should already be a power of two
// need to find out what that power is exactly (2^x?)

// frequency must be a power of 2
/*
 * rtc_calculate_power_of_2
 *   DESCRIPTION: Helper function to find the log base 2 of a given number. finds it recursively
 *   INPUTS: freq - frequency that we want to set the rtc device to 
 *   OUTPUTS: NONE
 *   RETURN VALUE: Log base 2 of given frequency
 *   SIDE EFFECTS: NONE
 */
int32_t rtc_calculate_power_of_2(int32_t freq){

    if(freq == 1){

        return 0;

    }
    
    freq = freq / MIN_FREQUENCY;            // divide frequency by 2
    
    return 1 + rtc_calculate_power_of_2(freq);          // recursive call

}




