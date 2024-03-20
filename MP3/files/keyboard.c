#include "keyboard.h"
#include "lib.h"
#include "multiboot.h"
#include "x86_desc.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "terminal.h"
#include "idt.h"
#include "paging.h"
#include "systemcall.h"

/*header (/)

keyboard.c - C file that echos keyboard input onto screen 

defines special key scancodes when pressed and released, as well as the data port for keyboard.
defines flags for special keys and the corresponding character to key scancodes. 
keyboard_init() enables the keyboard interrupt
flags() sets the flags of special keys according to its scancode.
keyboard_handler() converts the scancodes into a character to output to a screen and places them in a keyboard buffer of max 127 characters
get_char_count(), set_char_count(), get_newline_flag(), and set_newline_flag(), are helper functions to access 
global variables for terminal.c 



*/

#define MAX_BUFFER_SIZE 128
#define KEYBOARD_PORT  0x60

#define BUFF_MAX_SIZE 128
#define CAPS   0x3A 
#define CAPS_RELEASE 0xBA
#define BACKSPACE 0x0E  // 14
#define BACKSPACE_RELEASE 0x14
#define ESC 0x00 //1
#define LSHIFT 0x2A //42
#define LSHIFT_RELEASE 0xAA
#define RSHIFT 0x36 // 54
#define RSHIFT_RELEASE 0xB6 
#define PRTSC 0x37
#define ALT 0x38
#define ALT_RELEASE 0xB8
#define CTRL 0x1D 
#define CTRL_RELEASE  0x9D
#define TAB 0x0F
#define ENTER 0x1C
#define ENTER_RELEASE 0x9C

#define A 0x1E
#define L 0x26
#define M 0x32
#define P 0x19
#define Q 0x10
#define Z 0x2C

#define F_1 0x3B      // switch back later 0x3B
#define F_2 0x3C
#define F_3 0x3D

#define F_1_RELEASE 0xBB
#define F_2_RELEASE 0xBC
#define F_3_RELEASE 0xBD

#define FOUR_SPACES 4
#define SHIFTED_VERSION 2
#define SCANCODES 62
/* flags to indicate if a shift, caps, tab, or control button has been pressed*/
uint8_t caps = 0;
uint8_t lshift = 0;
uint8_t rshift =0;
uint8_t tab = 0;
uint8_t control = 0;
uint8_t backspace = 0;
uint8_t i;
uint8_t alt = 0;
uint8_t f_1 = 0;
uint8_t f_2 = 0;
uint8_t f_3 = 0;


/*counter for character sin keybaord_buffer*/
uint8_t char_count = 0;

// added this for terminal to indicate a newline has been pressed
// uint8_t newline_flag = 0;


/*
 * get_char_count()
 *   DESCRIPTION: Retrieves the char_count variable
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: char_count variable
 *   SIDE EFFECTS: none
 */
uint8_t get_char_count(){
    return terminal[display_terminal].char_count;
    // return char_count;        // may need to do return for specific terminal  <--------- commented this

}

/*
 * set_char_count(uint8_t value)
 *   DESCRIPTION: Sets the char_count variable to the given value
 *   INPUTS: value - value that we want to set the char_count to
 *   OUTPUTS: NONE
 *   RETURN VALUE: NONE
 *   SIDE EFFECTS: changes the char_count variable
 */
void set_char_count(uint8_t value){
    terminal[display_terminal].char_count = value;
    // char_count = value;           //may need to do the return for specific terminal <--------- commented this

}

/*
 * get_newline_flag(void)
 *   DESCRIPTION: Retrieves the new_line flag
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: newline_flag
 *   SIDE EFFECTS: NONE
 */
uint8_t get_newline_flag(void){
    // return cur_terminal.newline;
    // return newline_flag;      //may need to do this for specific terminal
    return terminal[display_terminal].newline;

}

/*
 * set_newline_flag(uint8_t value)
 *   DESCRIPTION: Sets the newline_flag variable to the given value
 *   INPUTS: value - value that we want to set the char_count to
 *   OUTPUTS: NONE
 *   RETURN VALUE: NONE
 *   SIDE EFFECTS: Changes the newline_flag global variable
 */
void set_newline_flag(uint8_t value){
    // cur_terminal.newline = value;
    // newline_flag = value;   //may need to do this for specific terminal
    terminal[display_terminal].newline = value;

    
}



    /*create an array/ a table that holds the keyboard characters, and its shifted characters*/
unsigned char scancode[SCANCODES][SHIFTED_VERSION] = {
        {0x00, 0x00}, {ESC, ESC}, {'1', '!'}, {'2', '@'}, {'3', '#'}, {'4', '$'}, {'5', '%'}, {'6', '^'},
        {'7', '&'}, {'8', '*'},{'9', '('}, {'0', ')'}, {'-', '_'}, {'=', '+'}, {BACKSPACE, BACKSPACE},
        {TAB, TAB}, {'q', 'Q'}, {'w', 'W'}, {'e', 'E'}, {'r', 'R'}, {'t', 'T'}, {'y', 'Y'}, {'u','U'},
        {'i', 'I'}, {'o', 'O'}, {'p', 'P'} ,{'[', '{'}, {']', '}'}, {ENTER, ENTER}, {CTRL,CTRL},
        {'a', 'A'}, {'s', 'S'}, {'d', 'D'} ,{'f', 'F'}, {'g', 'G'}, {'h', 'H'}, {'j', 'J'}, {'k', 'K'}, 
        {'l', 'L'}, {';', ':'}, {'\'', '"'}, {'`', '~'}, {LSHIFT, LSHIFT}, {'\\', '|'}, {'z', 'Z'}, {'x', 'X'},
        {'c', 'C'}, {'v','V'}, {'b', 'B'}, {'n' ,'N'}, {'m','M'}, {',','<'}, {'.','>'}, {'/','?'},
        {RSHIFT, RSHIFT}, {0x00,0x00}, {ALT, ALT}, {' ', ' '}, {CAPS, CAPS}, {F_1, F_1}, {F_2, F_2}, {F_3, F_3}
};


/*
 * keyboard_init(void)
 *   DESCRIPTION: Initialize keyboard interrupt by enabling IR1 in PIC
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: NONE
 *   SIDE EFFECTS: Enabling IR1 in PIC
 */

void keyboard_init(void) {
  enable_irq(IRQ1);    // - this is the interrupt request number for keyboard in IDT;
}

/*
 * flags(uint8_t key)
 *   DESCRIPTION: State function that handles flags of special keys
 *   INPUTS: key - user input from keyboard
 *   OUTPUTS: Flags are set to either 1 or 0;
 *   RETURN VALUE: If a key is flagged, returns 1, otherwise returns 0
 *   SIDE EFFECTS: NONE
 */

uint8_t flags(uint8_t key){

  switch(key){

    case CAPS:
      if(terminal[display_terminal].caps == 1){
        terminal[display_terminal].caps = 0;
        // caps = 0;
      }
      else{ 
        terminal[display_terminal].caps = 1;
      }
    break;

    case ENTER:
      terminal[display_terminal].enter = 1;
      break;

    case ENTER_RELEASE:
      terminal[display_terminal].enter = 0;
      break;
      
    case LSHIFT:
      lshift = 1;
    break;

    case LSHIFT_RELEASE:
      lshift = 0;
    break;

    case RSHIFT:
      rshift = 1;
    break;

    case RSHIFT_RELEASE:
      rshift = 0;
    break;

    case ALT: // do nothing for ALT
      alt  = 1;
    break;

    case ALT_RELEASE:
      alt = 0;
    break;

    case F_1:
      f_1 = 1;
    break;

    case F_1_RELEASE:
      f_1 = 0;
    break;

    case F_2:
      f_2 = 1;
    break;

    case F_2_RELEASE:
      f_2 = 0;
    break;

    case F_3:
      f_3 = 1;
    break;

    case F_3_RELEASE:
      f_3 = 0;
    break;
   
    case ESC: //do nothing for ESC
    break;

    case CTRL:
      control = 1;
    break;

    case CTRL_RELEASE:
      control = 0;
    break;

    default: 
      return 0;
  }

  return 1;

}

/*
 * keyboard_handler(void)
 *   DESCRIPTION: Take user input and converts to char to output to screen
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: Sends eoi and sti()
 *   SIDE EFFECTS: Outputs a character to the screen based on set conditions
 */
void keyboard_handler(void) {

// send_eoi(KEYBOARD_IRQ);
  cli(); // clear interrupt
  
  uint8_t key = inb(KEYBOARD_PORT); // key input from keyboard

  uint8_t flag = flags(key); // check to see if the key is not a letter or numbe key

  if(terminal[display_terminal].enter){
    // newline_flag = 1;
    terminal[display_terminal].newline = 1;
    // cur_terminal.newline = 1;

    if(terminal[display_terminal].char_count >= MAX_BUFFER_SIZE){ // if characters exceed keyboard buffer size, enter a new line at end of buffer
      terminal[display_terminal].keyboard_buffer[MAX_BUFFER_SIZE - 1] = '\n';}
    else{
      terminal[display_terminal].keyboard_buffer[terminal[display_terminal].char_count] = '\n';
    }
    keyboard_putc('\n');
    terminal[display_terminal].char_count = 0;      // when a newline is pressed reset character count
    terminal[display_terminal].enter = 0;
  }

  if(alt && f_1 == 1){
    if(display_terminal == SHIFTED_VERSION){
      save_screen(SHIFTED_VERSION);
      terminal_switch(TERMINAL_THREE_ADDR, TERMINAL_ONE_ADDR);
      FlushTheTLB();    

      restore_screen(0);
      display_terminal = 0;
      // PCB->parents_pid = term_parent_pid[display_terminal];
    }
    else if(display_terminal == 1){
      save_screen(1);
      terminal_switch(TERMINAL_TWO_ADDR, TERMINAL_ONE_ADDR);
      FlushTheTLB();    

      restore_screen(0);
      display_terminal = 0;
      // PCB->parents_pid = term_parent_pid[display_terminal];
    }
    else{
      display_terminal = 0;
    }
  }

  if(alt && f_2 == 1){
    if(display_terminal == 0){
      save_screen(0);
      terminal_switch(TERMINAL_ONE_ADDR,TERMINAL_TWO_ADDR);
      FlushTheTLB(); 
      restore_screen(1);
      display_terminal = 1;
    }
    else if(display_terminal == SHIFTED_VERSION){
      save_screen(SHIFTED_VERSION);
      terminal_switch(TERMINAL_THREE_ADDR,TERMINAL_TWO_ADDR);
      FlushTheTLB(); 
      restore_screen(1);
      display_terminal = 1;
    }
    else{
      display_terminal = 1;
    }
  }

  if(alt && f_3 == 1){

    if(display_terminal == 0){
      save_screen(0);
      terminal_switch(TERMINAL_ONE_ADDR, TERMINAL_THREE_ADDR);
      FlushTheTLB(); 
      restore_screen(SHIFTED_VERSION);
      display_terminal = SHIFTED_VERSION;
    }
    else if(display_terminal == 1){
      save_screen(1);
      terminal_switch(TERMINAL_TWO_ADDR, TERMINAL_THREE_ADDR);
      FlushTheTLB(); 
      restore_screen(SHIFTED_VERSION);
      display_terminal = SHIFTED_VERSION;
    }
    else{
      display_terminal = SHIFTED_VERSION;
    }
  }

  if(flag){ // if it is not, then interrupt completed
    send_eoi(IRQ1);
    sti();
    return;

  }

  //translate key to ascii and output to screen
  if (key < SCANCODES && key > 1 ){ // if key pressed is valid

    switch(key){ 
      // case ENTER:
      //   // need a flag here to indicate newline!!! for terminal
      //   newline_flag = 1;
      //   // cur_terminal.newline = 1;

      //   if(cur_terminal.char_count >= MAX_BUFFER_SIZE){ // if characters exceed keyboard buffer size, enter a new line at end of buffer
      //     cur_terminal.keyboard_buffer[MAX_BUFFER_SIZE - 1] = '\n';}
      //   else{
      //     cur_terminal.keyboard_buffer[cur_terminal.char_count] = '\n';
      //   }
      //   keyboard_putc('\n');
      //    cur_terminal.char_count = 0;      // when a newline is pressed reset character count
      // break;

      // case ENTER_RELEASE:
      // newline_flag = 0;
      
      case TAB:

        keyboard_putc('\t');
        terminal[display_terminal].keyboard_buffer[terminal[display_terminal].char_count] = '\t';
        terminal[display_terminal].char_count++;
        // char_count++;

      break;

      case BACKSPACE:

        if(terminal[display_terminal].char_count){       // if buffer id not empty, backspacing is allowed
          if (terminal[display_terminal].keyboard_buffer[(terminal[display_terminal].char_count)-1]== '\t'){
            for(i = 0; i < FOUR_SPACES; i++){
              keyboard_putc('\b');
              //check this portion b/c tab is 4 spaces, so may need to delete 4 times
            }
          }
          else{
            keyboard_putc('\b');
          }
          terminal[display_terminal].char_count--;
          // char_count--;
          terminal[display_terminal].keyboard_buffer[terminal[display_terminal].char_count] = '\0';
        }
      break;

      default:

        if(terminal[display_terminal].char_count < MAX_BUFFER_SIZE - 1){
          if((A<= key && L >= key) || (Q<= key && P >= key) || (Z<= key && M >= key)){ // if the character is a letter
            if(lshift || rshift){ // if a shift flag is up but the caps flag is up as well
              if(terminal[display_terminal].caps){
                terminal[display_terminal].keyboard_buffer[terminal[display_terminal].char_count] = scancode[key][0];
                keyboard_putc(scancode[key][0]); // print the lower case value, otherwise print the uppercase value
              }
              else{
                terminal[display_terminal].keyboard_buffer[terminal[display_terminal].char_count] = scancode[key][1];
                keyboard_putc(scancode[key][1]);
              }
            }
            else if(terminal[display_terminal].caps){ // else is the caps is the only flag up, print the uppercaser version
              terminal[display_terminal].keyboard_buffer[terminal[display_terminal].char_count] = scancode[key][1];
              keyboard_putc(scancode[key][1]);
            }
            else{
              terminal[display_terminal].keyboard_buffer[terminal[display_terminal].char_count] = scancode[key][0];
              keyboard_putc(scancode[key][0]);
            }
          }

          else if(lshift || rshift){// if the key is any other char, print them accordingly based on shift flags
            terminal[display_terminal].keyboard_buffer[terminal[display_terminal].char_count] = scancode[key][1];
            keyboard_putc(scancode[key][1]);
          }

          else{
            terminal[display_terminal].keyboard_buffer[terminal[display_terminal].char_count] = scancode[key][0];
            keyboard_putc(scancode[key][0]);
          }
          terminal[display_terminal].char_count++;
          // char_count++; //increment the character counter and return

        }

        else{ 
          keyboard_putc('\n'); // if buffer is filled, clear buffer and start a new line 
          terminal[display_terminal].keyboard_buffer[MAX_BUFFER_SIZE - 1] = '\n';
          terminal[display_terminal].char_count = 0;
          // char_count = 0;
        }

        if(control && key == 0x26){  //if ctrl l is press, clear the screen
          clear();
          terminal[display_terminal].char_count = 0;
          // char_count = 0;
        }
    }
  }
  send_eoi(IRQ1);
  sti();
}

int get_active_terminal(){
  if(display_terminal == 0){
    return TERMINAL_ONE_ADDR;
  }else if(display_terminal == 1){
    return TERMINAL_TWO_ADDR;
  }else if(display_terminal == SHIFTED_VERSION){
    return TERMINAL_THREE_ADDR;
  }else{
    return 0xB8000;
  }
}


