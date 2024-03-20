#ifndef _TERM_H
#define _TERM_H

#include "types.h"
#include "systemcall.h"

#define MAX_BUFFER_SIZE 128
#define NUMBER_OF_TERMINALS 3 
// this buffer will hold the characters of the buttons pressed on the keyboard
// char keyboard_buffer[NUMBER_OF_TERMINALS][MAX_BUFFER_SIZE];       // must only hold up to 128 characters

int cur_terminal_x[NUMBER_OF_TERMINALS];
int cur_terminal_y[NUMBER_OF_TERMINALS];

int display_terminal;   // what is being shown
int active_terminal;    //what is actually being executed/sheduled, used by PIT

typedef struct terminal_t{
    int latest_pid;
    int prev_pid;
    uint32_t screen_x;
    uint32_t screen_y;
    uint32_t char_count;
    uint32_t caps;
    uint32_t newline;
    uint32_t enter;
    uint32_t ebp;
    uint32_t esp;
    char attribute;
    char keyboard_buffer[MAX_BUFFER_SIZE];

}terminal_t;

terminal_t terminal[NUMBER_OF_TERMINALS];

terminal_t cur_terminal;

int32_t terminal_init(void);

int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t terminal_open(const uint8_t* filename);

int32_t terminal_close(int32_t fd);

#endif
