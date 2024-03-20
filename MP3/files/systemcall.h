#ifndef _SYS_H
#define _SYS_H

#include "types.h"
#include "filesystem.h"
#include "rtc.h"
#include "terminal.h"



// magic bytes that show if a file is an executable
#define MAGIC_BYTE_0    0x7f
#define MAGIC_BYTE_1    0x45
#define MAGIC_BYTE_2    0x4c
#define MAGIC_BYTE_3    0x46
#define VIDEO_MEM_ADDR 0xB800
#define FILE_DESCRIPTOR_NUM 8


 
#define PID_OFFSET 2        // offset the file descriptor because of stdin and stdout 

#define MAX_PROCESSES 6

#define PAGE_SIZE_OF_4MB   0x400000
#define PAGE_SIZE_OF_8MB   0x800000
#define PAGE_SIZE_OF_8KB   0x2000

#define FOTOP_NUM 5
#define FD_START 2
#define FD_ARRAY_LENGTH 8

#define ELF_BYTES_24_TO_27 4
#define ELF_ENTRY_POINT 24

#define ELF_FOUR_BYTES 4

#define USER_MEMORY_START   0x8000000

#define HALT_EXCEPTION
#define HALT_EXCEPTION_RETURN
#define PROG_IMG_START 0x08048000

#define MAX_FD 7
#define MIN_FD 1

#define FOUR 4

#define ELF_BYTES_SIZE 32

#define NUM_PAGES 1024

#define NUM_TERMINALS 3

// file operation table pointer struct
// table that points to the "generic" handlers
typedef struct __attribute__((packed)) FOTP_t{

    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(int32_t fd);

} FOTP_t;

// FOTP_t rtc_fn = {.read = rtc_read, .write = rtc_write, .open = rtc_open, .close = rtc_close};

// FOTP_t file_fn = {.read = file_read, .write = file_write, .open = file_open, .close = file_close};

// FOTP_t dir_fn = {.read = dir_read, .write = dir_write, .open = dir_open, .close = dir_close};

// FOTP_t stdin_fn = {.read = terminal_read, .open = terminal_open, .close = terminal_close};

// FOTP_t stdout_fn = {.write = terminal_write, .open = terminal_open, .close = terminal_close};


typedef struct __attribute__((packed)) file_descriptor_t{
    //add type
    FOTP_t* FOTP;
    uint32_t inode;
    uint32_t file_pos;
    uint32_t flags;
} file_descriptor_t;

// per-task data structure -> Process Control Block (PCB)
typedef struct __attribute__((packed)) PCB_t{

    file_descriptor_t fd_array[FILE_DESCRIPTOR_NUM];

    // pid -> process ID
    // need to save the user and 
    int parents_pid; 
    int current_pid;

    // save the ebp and esp of the execute function
    uint32_t executes_ebp;
    uint32_t executes_esp;

    // when we return from this process we want to go to the next instruction that would have occured 
    // save the user eip and esp to do this
    uint32_t users_eip;
    uint32_t users_esp;

    // save the tss so that we can do a context switch 
    uint32_t tss_esp_0;

    uint16_t tss_ss0;       // ADDED THIS FOR SCHEDULING

    uint8_t arguments[FILENAMESIZE];

}PCB_t;

PCB_t* PCB[NUM_TERMINALS];
int pid_array[MAX_PROCESSES];

// checkpoint 3 functions
int32_t halt (uint32_t status);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t execute(const uint8_t* command);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);
int32_t systemcall_handler();



// extern void systemcall_handler();

#endif /* _SYS_H */
