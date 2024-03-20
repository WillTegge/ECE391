#include "paging.h"
#include "lib.h"
#include "systemcall.h"

page_directory_entry_t page_directory[NUM_TABLES] __attribute__ ((aligned(ENTRIES_SIZE)));
page_table_entry_t page_table[NUM_PAGES] __attribute__ ((aligned(PAGE_SIZE)));
page_table_entry_t vid_p_t_[NUM_PAGES] __attribute__ ((aligned(PAGE_SIZE)));


/*
 * page_init()
 *   DESCRIPTION: initialize the paging, creating 
 *   physical memory from video memory
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: creating physical memory for
 *   kernal and video memory
 */

void page_init() {
    int i;
    //physical mem for kernal
    for(i = 0; i < NUM_TABLES; i++){
        page_table[i].P     = 0;
        page_table[i].R_W   = 1;
        page_table[i].U_S   = 0;
        page_table[i].PWT   = 0;
        page_table[i].PCD   = 0;
        page_table[i].A     = 0;
        page_table[i].D     = 0;
        page_table[i].PAT   = 0;
        page_table[i].G     = 0;
        page_table[i].AVL   = 0;
        page_table[i].PBA   = i;
    }

    /*universal putc*/
    page_table[0xb6].P = 1; //<-- 0xB6000
    page_table[0xb6].PBA = VIDEO_MEM >> BIT_SHIFT; //<-- 0xB6000
    
    /*vid mem*/
    page_table[0xb8].P = 1;

    /*terminal pages*/
    page_table[TERMINAL_ONE].P = 1;
    page_table[TERMINAL_TWO].P = 1;
    page_table[TERMINAL_THREE].P = 1;

    for(i = 0; i < NUM_TABLES; i++){
        vid_p_t_[i].P     = 0;
        vid_p_t_[i].R_W   = 1;
        vid_p_t_[i].U_S   = 0;
        vid_p_t_[i].PWT   = 0;
        vid_p_t_[i].PCD   = 0;
        vid_p_t_[i].A     = 0;
        vid_p_t_[i].D     = 0;
        vid_p_t_[i].PAT   = 0;
        vid_p_t_[i].G     = 0;
        vid_p_t_[i].AVL   = 0;
        vid_p_t_[i].PBA   = i;
    }
    

    for(i = 0; i < NUM_TABLES; i++){
        page_directory[i].P         = 0;
        page_directory[i].R_W       = 1;
        page_directory[i].U_S       = 0;
        page_directory[i].PWT       = 0;
        page_directory[i].PCD       = 0;
        page_directory[i].A         = 0;
        page_directory[i].AVL_BIT   = 0;
        page_directory[i].PS        = 0;
        page_directory[i].G         = 0;
        page_directory[i].AVL       = 0;
        page_directory[i].PTBA      = 0;
    }
    
    
    /* loading video mem directory */
    page_directory[0].P = 1;
    page_directory[0].PTBA = (uint32_t)(page_table) >> BIT_SHIFT; //offset to get just the address of page table
    
    /* loading kernel in directory*/
    page_directory[1].P = 1;
    //page_directory[1].PCD = 1;      //set according to descriptors
    page_directory[1].PS = 1;       //page size to 4MB
    page_directory[1].G = 1;        //set according to descriptors <------------------------------just added
    page_directory[1].PTBA = 0x00400;   //offset needed to create a 4mb block for page directory
    
    /*
        note for switching  between the kernal and user spce: 
            the 'G' bit controls the TLB behavior for a particulat page when the TLB is flushed (CR3 is reloaded)
            If the bit is set to 1, the page corresponding to this PDE/PTE is "global" and virtual-> phyiscal translations
            that use this page are visible to ALL processes. So, TLB entries associated with this page will not be cleared
            when CR3 is reloaded. If it is clear (0), the page corresponding to this PDE/PTE is a per-preocessor page and
            the translations will be cleared when CR3 is reloaded. This bit should onl be set if the page mapping is actually
            shared by all processes (this page directory entry exsists in all processes' page directories).
    */
    
    /*loading user directory*/
    // offset needed to create block in 128MB location 

    loadPageDirectory((uint32_t*)page_directory);
    // printf("lololol");
    // while(1);
    enablePaging();
    // printf("lol");
    // while(1);
}

/*
 * enable_user_page(uint32_t pid)
 *   DESCRIPTION: Sets User Page values to enable page
 *   INPUTS: pid - Process ID
 *   OUTPUTS: NONE
 *   RETURN VALUE: NONE
 *   SIDE EFFECTS: User Page is present
 */
void enable_user_page(uint32_t pid){
    page_directory[USER_DIRECTORY_INDEX].P = 1;
    page_directory[USER_DIRECTORY_INDEX].U_S = 1;
    page_directory[USER_DIRECTORY_INDEX].PS = 1;
    page_directory[USER_DIRECTORY_INDEX].PTBA = 0x00800 + (pid * NUM_TABLES);  //offset for this will be offset + PID or sum
    //flush tlb
}
/*
 * enable_video_page(void)
 *   DESCRIPTION: Sets video Page values to enable page
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: NONE
 *   SIDE EFFECTS: video Page is present
 */

void enable_video_page(){
    vid_p_t_[0].P = 1;
    vid_p_t_[0].U_S = 1;
    vid_p_t_[0].PBA = VIDEO_MEM >> 12; //offset to get just the address of page table
    page_directory[USER_DIRECTORY_VIDEO_INDEX].P = 1;
    page_directory[USER_DIRECTORY_VIDEO_INDEX].U_S = 1;
    page_directory[USER_DIRECTORY_VIDEO_INDEX].PTBA = (uint32_t)(vid_p_t_) >> BIT_SHIFT; //offset to get just the address of page directory

}
/*
 * terminal_switch(int save_addr, int switch_addr)
 *   DESCRIPTION: copy memory from screen for terminal we are switching FROM and copy mem from screen of terminal we're switching TO         
 *   INPUTS:save_addr - the address of the terminal we are saving
 *          switch_addr - the address of the terminal we are switching to
 *   OUTPUTS: NONE
 *   RETURN VALUE: NONE
 *   SIDE EFFECTS: terminal switching
 */
void terminal_switch(int save_addr, int switch_addr){

    page_table[save_addr >> BIT_SHIFT].PBA = save_addr >> BIT_SHIFT;          //set the address of current terminal to itself
                                        //flush

    memcpy((void *)save_addr, (void *)UNIVERSAL_MEM, (uint32_t)PAGE_SIZE);                 //copy memory from screen for terminal we are switching FROM              

    memcpy((void *)UNIVERSAL_MEM, (void *)switch_addr, (uint32_t)PAGE_SIZE);               //copy mem from screen of terminal we're switching TO

    page_table[switch_addr >> BIT_SHIFT].PBA = VIDEO_MEM >> BIT_SHIFT;         //set address of terminal to video mem
}
