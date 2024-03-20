#include "terminal.h"
#include "keyboard.h"
#include "idt.h"
#include "tests.h"
#include "lib.h"
#include "i8259.h"
#include "paging.h"
#include "systemcall.h"

//uint8_t char_count_2 = 0;
/*
 * terminal_init
 *   DESCRIPTION: creates the keyboard buffer and initializes it to all 0's and resets the flags used by the terminal
 *   INPUTS: filename - name of a file
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: reinitializes the keyboard buffer, sets the char_count and newline flags to 0
 */
int32_t terminal_init(void){

    int i;
    display_terminal = 0;
    active_terminal = 0;

    /* Accessing and clearing each terminal buffer */
    memset(terminal[0].keyboard_buffer, 0, MAX_BUFFER_SIZE); //0 - Terminal 1
    memset(terminal[1].keyboard_buffer, 0, MAX_BUFFER_SIZE); //1 - Terminal 2
    memset(terminal[2].keyboard_buffer, 0, MAX_BUFFER_SIZE); //2 - Terminal 3
    
    for(i = 0; i < NUM_TERMINALS; i++){
        terminal[i].char_count = 0;
        terminal[i].newline = 0;
        terminal[i].caps = 0;
        terminal[i].enter = 0;
        terminal[i].latest_pid = -1;
        terminal[i].attribute = 0xD+i;
        memset(terminal[i].keyboard_buffer,0, MAX_BUFFER_SIZE);
    }
    for(i = 0; i < (MAX_BUFFER_SIZE - 1); i++){
        
        terminal[0].keyboard_buffer[i] = '\0'; //0 - Terminal 1
        terminal[1].keyboard_buffer[i] = '\0'; //1 - Terminal 2
        terminal[2].keyboard_buffer[i] = '\0'; //2 - Terminal 3
    }

    terminal[0].keyboard_buffer[i] = '\n';  //0 - Terminal 1
    terminal[1].keyboard_buffer[i] = '\n';  //1 - Terminal 2
    terminal[2].keyboard_buffer[i] = '\n';  //2 - Terminal 3

    terminal[display_terminal].char_count = 0;
    set_newline_flag((uint8_t)0);
    return 0;
}

// notes on terminal_read
// Terminal read() reads FROM the keyboard buffer into buf, return number of bytes read
// should return data from one line that has been terminated by pressing Enter, or as much as fits in the buffer from one such line
// The line returned should include the line feed character

/*
 * terminal_read
 *   DESCRIPTION: copies certain amount of bytes from keyboard buffer to terminal buffer
 *   INPUTS: fd - file descriptor
 *          buf - terminal buffer
 *           nbytes - number of bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: number of bytes read or -1
 *   SIDE EFFECTS: changes the terminal buffer
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    sti();
    int i;




    int number_of_bytes_read = 0;
    uint8_t newline_flag_;

    
    newline_flag_ = get_newline_flag();

    if(nbytes > MAX_BUFFER_SIZE){

        nbytes = MAX_BUFFER_SIZE;
        
    }

    if(buf == NULL){

        return -1;

    }

    // fills the buffer until we get a newline (flag set by interrupt handler)
    // only returns when the enter key is pressed (so this while loop is to wait for the enter key to be pressed)
    while(newline_flag_ == 0){

        newline_flag_ = get_newline_flag();

    }

    // char_count_2 = get_char_count();

    // newline_flag = 0;           // newline_flag reset since we already acknowledged that it was raised 
    newline_flag_ = 0;
    // set_newline_flag((uint8_t)0);

    // might not need to cli here but I did since were using flags
    cli();

    //now we want to memcpy our line buffer from keyboard.c into this destination buffer
    // use memcpy here
    //Check the first 5 spots in terminal buffer (0 - 4) for the exit command
        if(terminal[display_terminal].keyboard_buffer[0] == 'e' && terminal[display_terminal].keyboard_buffer[1] == 'x' && terminal[display_terminal].keyboard_buffer[2] == 'i'  && terminal[display_terminal].keyboard_buffer[3] == 't' && terminal[display_terminal].keyboard_buffer[4] == '\n' ){
            terminal[display_terminal].keyboard_buffer[4] = '\0'; //add null to the fourth (4) position in buffer
        }
    
    
    memcpy(buf, terminal[display_terminal].keyboard_buffer, nbytes);
   

    number_of_bytes_read = nbytes;

    for(i = 0; i < (MAX_BUFFER_SIZE - 1); i++){ // 127 is the maximum size of the buffer

        terminal[display_terminal].keyboard_buffer[i] = '\0';

    }



    // since were working with flags
    sti();
    terminal[display_terminal].char_count = 0;
    // set_char_count((uint8_t)0);
    set_newline_flag(0);

    return number_of_bytes_read;


}


// notes on terminal_write
// Terminal write() writes TO the screen from buf, return number of bytes written or -1
// In the case of the terminal, all data should be displayed to the screen immediately
// The call returns the number of bytes written, or -1 on failure

/*
 * terminal_write
 *   DESCRIPTION: writes certain number of bytes from terminal buffer to the screen
 *   INPUTS: fd - file descriptor
             buf - terminal buffer
             nbytes - number of bytes to read
 *   OUTPUTS: outputs characters to the screen
 *   RETURN VALUE: number of bytes written or -1
 *   SIDE EFFECTS: outputs characters to the screen
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){

    int i; 
    char current_char;
    // int counter;
    // check for valid inputs
    if(buf == NULL){
        return -1;
    }

    if(nbytes <= 0){
        return -1;
    }

    // loop over number of bytes (each character in buf that we want to write)
    for(i = 0; i < nbytes; i++){

        current_char = ((char *)buf)[i];       // get the current char out of the buf 

        if(current_char != '\0'){                   // dont want to print out the NULL bytes

            putc(current_char);                     // prints current character to the screen
            // counter++;
            // keyboard_buffer[i] = ((char*)buf)[i];

        }
        
    }
        // if(((char *)buf)[counter] != '\n'){
        //         ((char *)buf)[counter + 1] = '\n';
        //         current_char = ((char *)buf)[counter + 1];
        //         putc(current_char); 
        //     }

    // for(i = 0; i < 127; i++){ // 127 is the maximum size of the buffer
    //     ((char *)buf)[i] = '\0';
    // }
 
    
    return nbytes;
    
}


/*
 * terminal_open
 *   DESCRIPTION: creates the keyboard buffer and initializes it and resets the flags used by the terminal
 *   INPUTS: filename - name of a file
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: reinitializes the keyboard buffer, sets the char_count and newline flags to 0
 */
int32_t terminal_open(const uint8_t* filename){

    memset(terminal[display_terminal].keyboard_buffer, 0, MAX_BUFFER_SIZE);
    terminal[display_terminal].char_count = 0;
    // set_char_count((uint8_t)0);
    terminal[display_terminal].newline = 0;
    // set_newline_flag((uint8_t)0);
    return 0;

}



// does nothing
/*
 * terminal_close
 *   DESCRIPTION: does nothing
 *   INPUTS: fd - file descriptor
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: does nothing
 */
int32_t terminal_close(int32_t fd){

    return 0;

}


//terminal struct - count, newline flag for curr terminal, keyboard buffer, etc. anything that is different between structs
