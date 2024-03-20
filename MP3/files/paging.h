#include "types.h"
#define NUM_PAGES 1024
#define NUM_TABLES 1024
#define PAGE_SIZE 4096
#define ENTRIES_SIZE 4096
#define USER_DIRECTORY_INDEX    32
#define USER_DIRECTORY_VIDEO_INDEX 33
#define BIT_SHIFT 12
#define VIDEO_MEM       0xB8000
#define TERMINAL_ONE    0xBA
#define TERMINAL_TWO    0xBB
#define TERMINAL_THREE  0xBC

#define TERMINAL_ONE_ADDR    0xBA000
#define TERMINAL_TWO_ADDR    0xBB000
#define TERMINAL_THREE_ADDR  0xBC000

#define UNIVERSAL_MEM 0xB6000

#ifndef ASM 

/* 
 * Initialize page table from 0MB to 4MB
 * Each page directory has 1024 pages - 4MB
 * Set each page to not present then 
 * set the Video Memory page correctly
 */

 /*
  * Page Directory Structure
  * P   - Present
  * R/W - Read/Write
  * U/S - User/SUpervisor
  * PWT - Write-Through
  * PCD - Cache Disable
  * A   - Accessed
  * AVL - Available
  * PS  - Page Size
  * AVL - Available
  */
typedef struct __attribute__((packed)) page_directory_entry_t {
            uint8_t P                  : 1;
            uint8_t R_W                : 1; 
            uint8_t U_S                : 1;
            uint8_t PWT                : 1;
            uint8_t PCD                : 1;
            uint8_t A                  : 1;
            uint8_t AVL_BIT            : 1;
            uint8_t PS                 : 1;
            uint8_t G                  : 1;
            uint8_t AVL                : 3;
            uint32_t PTBA              : 20;
} page_directory_entry_t;


/*
 * Page Table Structure
 * P   - Present
 * R/W - Read/Write
 * U/S - User/SUpervisor
 * PWT - Write-Through
 * PCD - Cache Disable
 * A   - Accessed
 * D   - Dirty (scandalous)
 * PAT - Page Attribute Table
 * G   - Global
 * AVL - Available
 */
typedef struct __attribute__((packed)) page_table_entry_t{
            uint8_t P                  : 1;
            uint8_t R_W                : 1;
            uint8_t U_S                : 1;
            uint8_t PWT                : 1;
            uint8_t PCD                : 1;
            uint8_t A                  : 1;
            uint8_t D                  : 1;
            uint8_t PAT                : 1;
            uint8_t G                  : 1;
            uint8_t AVL                : 3;
            uint32_t PBA               : 20;
} page_table_entry_t;

/* Initialize Page Directory and Page Table*/
extern page_directory_entry_t page_directory[NUM_TABLES];
extern page_table_entry_t page_table[NUM_PAGES];

/*video memeory page*/
extern page_table_entry_t vid_p_t_[NUM_PAGES];

/* Initialize Paging Functions */
extern void page_init();
extern void loadPageDirectory(uint32_t* base_addr);
extern void enablePaging();
extern void enable_user_page(uint32_t pid);
void enable_video_page();

extern void FlushTheTLB();

extern void terminal_switch(int save_addr, int switch_addr);

#endif /* ASM */
