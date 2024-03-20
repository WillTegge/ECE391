
#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"

#define IRQ1  1

// int newline_flag;
// int char_count;

// added these functions to interact with the global variables / flags used in this file
// mainly to get rid of wierd warnings about types
extern uint8_t get_char_count(void);
extern void set_char_count(uint8_t value);
extern uint8_t get_newline_flag(void);
extern void set_newline_flag(uint8_t value);

void keyboard_init(void);

extern void keyboard_handler(void);

uint8_t flags(uint8_t key);

extern int get_active_terminal();

#endif /*_KEYBOARD_H*/
