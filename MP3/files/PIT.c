#include "PIT.h"
#include "lib.h"
#include "i8259.h"
#include "tests.h"
#include "systemcall.h"
#include "x86_desc.h"
#include "paging.h"

int counter;
int prev_term;

int terminal_started_flags[NUMBER_OF_TERMS] = {0, 0, 0};

int terminal_addr[NUMBER_OF_TERMS] = {TERMINAL_ONE_ADDR, TERMINAL_TWO_ADDR, TERMINAL_THREE_ADDR};
/*
 * schedule( void)
 *   DESCRIPTION: saves the current esp, ebp, tss, esp0, changes video memory, then restores esp, ebp, tss, esp0
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: NONE
 *   SIDE EFFECTS: changes to the next process 
 */
void schedule(){
    // Superman's sudo code
    // if counter < 3:
    // display_term = counter
    // if counter == 4:
    // display_term = 0

    // creates all of these on the same kernel stack // ORIGINAL
    // if(counter < 3){
    //     display_terminal = counter;
    //     if(display_terminal != 0){
    //         prev_term = display_terminal - 1;
    //         save_screen(prev_term);
// 
    //         terminal_switch(terminal_addr[prev_term],terminal_addr[display_terminal]);
    //         restore_screen(display_terminal);
    //         
    //     }
// 
    //     // initialize all values of the task struct
    //     // use in-line assembly for esp and ebp
    //     // asm("\t movl %%ebp, %0" : "=r"(saved_tasks[display_terminal].saved_ebp));
    //     // asm("\t movl %%esp, %0" : "=r"(saved_tasks[display_terminal].saved_esp));
    //     // // esp and esp0 should be initialized to be the same place       
    //     // saved_tasks[display_terminal].saved_esp0 = saved_tasks[display_terminal].saved_esp;
    //     // saved_tasks[display_terminal].saved_ss0 = 0;
// 
    //     send_eoi(PIT_IRQ);
    //     display_terminal = counter;
// 
    //     execute("shell");
// 
    // }

    if(counter < NUMBER_OF_TERMS){
        display_terminal = counter;


        // if its the first time were starting the terminal, then we save and execute (dont restore)
        if(terminal_started_flags[display_terminal] == 0){           
            terminal_started_flags[display_terminal] = 1;

            if(display_terminal != 0){
                prev_term = display_terminal - 1;
                save_screen(prev_term);

                terminal_switch(terminal_addr[prev_term],terminal_addr[display_terminal]);
                FlushTheTLB(); 
                restore_screen(display_terminal);
            
            }

            // initialize all values of the task struct
            // use in-line assembly for esp and ebp
            asm("\t movl %%ebp, %0" : "=r"(saved_tasks[display_terminal].saved_ebp));
            asm("\t movl %%esp, %0" : "=r"(saved_tasks[display_terminal].saved_esp));
            // esp and esp0 should be initialized to be the same place       
            saved_tasks[display_terminal].saved_esp0 = saved_tasks[display_terminal].saved_esp;
            saved_tasks[display_terminal].saved_ss0 = 0;                    // this should not be 0 but fix later 

            send_eoi(PIT_IRQ);
            display_terminal = counter;

            execute((uint8_t*)"shell");


        }

    }
    else{

        // active_terminal = counter % 3;
        // if(terminal_started_flags[active_terminal] == 1){


        //     /* save esp and ebp from current teminal process in terminal struct to go between schedule processes */
        //     asm("\t movl %%ebp, %0" : "=r"(saved_tasks[active_terminal].saved_ebp));
        //     asm("\t movl %%esp, %0" : "=r"(saved_tasks[active_terminal].saved_esp));
        //     // save esp, ebp, ss0, and esp0 to the global struct
        //     // dont need to save ss0 or esp0 but we do anyways (only need to restore them)
        //     saved_tasks[active_terminal].saved_esp0 = PCB[active_terminal]->tss_esp_0;
        //     saved_tasks[active_terminal].saved_ss0 = PCB[active_terminal]->tss_ss0;


        //     /* remap paging and video memory to change page directory to point to this new process
        //     * modify paging: remapping program page - active pids have their own 4MB program page, easy to do
        //     * remapping video memory: there is an active terminal and a displayed terminal(what user sees will always be on the physical video memory)
        //     * set paging to curr pid for terminal -- ex: if in terminal 3 save pid for terminal 3 and set program img to point to that pid
        //     *
        //     * if displayed and active the same then virtual memory video memory will putc to video memory physical and program image will print there too
        //     * if not then virtual memory video memory will putc to terminal page instead (create a second page for the keyboard for putc) etc
        //     * keyboard always uses putc to print to video, terminal write will use diff putc to print to terminal instead of video memory
        //     */
        //     // if(active_terminal == display_terminal){
        //     // //     //write to the physical address b8000
        //     // //     //once you come back to the process that is in the terminal being shown you remap the virtual address b8000 to the actual physical b8000
        //     // //     // putc(terminal[active_terminal].keyboard_buffer);
        //     // //     //set program image to curr pid for terminal
        //     // //     //vid memory in VM should point to PM
        //     //     page_table[0xb8].PBA = VIDEO_MEM >> 12;
        //     //     FlushTheTLB();
        //     //     enable_user_page(PCB[active_terminal]->current_pid);
        //     //     FlushTheTLB();
        //     // } 
        //     // else{
        //     // //     //remap the virtual memory b8000 such that it now points to current terminal's video memory page
        //     // //     // keyboard_putc(terminal[active_terminal].keyboard_buffer);
        //     // //     //set paging to curr pid for terminal
        //     // //     //how to do this: set program img to point to that pid
        //     // //     //asm volatile("movl %%cr3, %0" : "=r" (page_directory));
        //     // //     //vid mem page in VM should point to buff you created in memory
        //     //     page_table[0xb8].PBA = terminal_addr[active_terminal] >> 12;
        //     //     FlushTheTLB();
        //     //     enable_user_page(PCB[active_terminal]->current_pid);
        //     //     FlushTheTLB();
        //     //     
        //     // }

        //     // need to enable user paging based on the active terminal (should keep switching)
        //     // need several checks to do this correctly
            
        //     //universal mem is a page that the keyboard handler types into that always points to vid mem
        //     if(active_terminal == display_terminal){
        //         memcpy(VIDEO_MEM,terminal_addr[active_terminal], 4096);      //this is the part that we arent sure about
        //         page_table[0xb8].PBA = 0xb8;                                //page in VM -> page in PM
        //         FlushTheTLB();
        //         memcpy(UNIVERSAL_MEM, VIDEO_MEM, 4096);                       //copy whats in the universal buf to vid mem
        //         enable_user_page(PCB[active_terminal]->current_pid);        //process for that terminal runs
        //         FlushTheTLB();
        //     }else{
        //         // this is the portion of copying that we're kinda lost on
        //         memcpy(terminal_addr[display_terminal], VIDEO_MEM, 4096);
        //         page_table[0xb8].PBA = terminal_addr[active_terminal];      //point vid mem in VM -> terminal in PM
        //         FlushTheTLB();
        //         memcpy(VIDEO_MEM, terminal_addr[active_terminal], 4096);      //copy info from active term to vid mem
        //         enable_user_page(PCB[active_terminal]->current_pid);        //process for that terminal runs
        //         FlushTheTLB();
        //     }
            



        //     int next_terminal = active_terminal + 1;
        //     // /* pit interrupts creates context, ret addr, ebp, esp in stack of each terminal kernel stack
        //     // * change tss-esp0 to point to next terminal stack
        //     // */
        //     // if(active_terminal == 2){
        //     //     tss.esp0 = (PAGE_SIZE_OF_8MB - (PAGE_SIZE_OF_8KB * terminal[0].latest_pid) - sizeof(int32_t)); //idk if calc is right lol
        //     // }
        //     // else{
        //     //     tss.esp0 = (PAGE_SIZE_OF_8MB - (PAGE_SIZE_OF_8KB * terminal[(next_terminal)].latest_pid) - sizeof(int32_t)); //idk if calc is right lol
        //     // }


        //     // restore esp, ebp, ss0, and esp0 from the global struct array 




        //     // if(active_terminal == 2){
        //     //     tss.esp0 = saved_tasks[0].saved_esp0;
        //     //     tss.ss0 = saved_tasks[0].saved_ss0;
        //     // }
        //     // else{
        //     //     tss.esp0 = saved_tasks[next_terminal].saved_esp0;
        //     //     tss.ss0 = saved_tasks[next_terminal].saved_ss0;
        //     // }

        //     // if(active_terminal == 2){
        //     //     tss.esp0 = PCB[0]->tss_esp_0;
        //     //     tss.ss0 = PCB[0]->tss_ss0;
        //     // }
        //     // else{
        //     //     tss.esp0 = PCB[active_terminal]->tss_esp_0;
        //     //     tss.ss0 = PCB[active_terminal]->tss_ss0;
        //     // }


        //     // /* use next process ebp and esp to leave and use that context to go to where that process was interrupted */
            // if(active_terminal == 2){
            //     asm volatile("              \n
            //         movl %%ecx, %%ebp       \n
            //         movl %%edx, %%esp       \n
            //         leave                   \n
            //         ret                     \n
            //         "
            //         :
            //         : "c"(saved_tasks[0].saved_ebp), "d"(saved_tasks[0].saved_esp)
            //     );
            // }
            // else{
            //     asm volatile("              \n
            //         movl %%ecx, %%ebp       \n
            //         movl %%edx, %%esp       \n
            //         leave                   \n
            //         ret                     \n
            //         "
            //         :
            //         : "c"(saved_tasks[next_terminal].saved_ebp), "d"(saved_tasks[next_terminal].saved_esp)
            //     );
            // }



        //     tss.esp0 = PCB[active_terminal]->tss_esp_0;
        //     tss.ss0 = PCB[active_terminal]->tss_ss0;

        //     asm volatile("              \n
        //         movl %%ecx, %%ebp       \n
        //         movl %%edx, %%esp       \n
        //         leave                   \n
        //         ret                     \n
        //         "
        //         :
        //         : "c"(saved_tasks[active_terminal].saved_ebp), "d"(saved_tasks[active_terminal].saved_esp)
            // );

        // }
        

    }

}
/*
 * PIT_init(void)
 *   DESCRIPTION: initializes PIT
 *   INPUTS: none
 *   OUTPUTS: NONE
 *   RETURN VALUE: NONE
 *   SIDE EFFECTS: enables PIT interrupt and sets the ports. initializes the counter
 */
void PIT_init(){
    // send the command byte to set the PIT to square wave mode
    // outb(COMMAND_REGISTER, MODE3_LOWHIGH_CHANNEL0);
    // //set counter to 11932 (0x2e9c) to generate a 100 Hz to meet range of 10 to 50 milliseconds
    // outb(CHANNEL_0, COUNTER & LOW_BYTE);  //Sets low byte to 0x9C
    // outb(CHANNEL_0, COUNTER >> SET_HIGH); //Sets high byte to 0x2E

    // 1198130 
    counter = -1;
    int divisor = COUNT_MAX / FREQ;
    outb(MODE3_LOWHIGH_CHANNEL0, COMMAND_REGISTER);
    //set counter to 11932 (0x2e9c) to generate a 100 Hz to meet range of 10 to 50 milliseconds
    outb(divisor & LOW_BYTE, CHANNEL_0);  //Sets low byte to 0x9C
    outb(divisor >> SET_HIGH, CHANNEL_0); //Sets high byte to 0x2E

    //call execute with pid0, pid1, pid2 during intialization to create the contexts aat first for scheduler to use after(launching the 3 shells)
    //initialize terminals; if its 1st time opening shell - switch terminal manually, third time doesnt need to keep doing it manually
    //have an array of boolean variable in an array to see active processes, box out the first three to make sure the first three shells are executed cuz scheduler is non linear????
    //first pit interrupt will create a garbage data set but its later overwritten before its needed anyway
    enable_irq(PIT_IRQ);
    
}
/*
 * PIT_handler(void)
 *   DESCRIPTION: clears interrupts, increments counter, calls schedule function, and sends an eoi to PIC
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: NONE
 *   SIDE EFFECTS: increments counter and schedules next process
 */
void PIT_handler(){
    cli();
    counter++;
    schedule();
    sti();
    send_eoi(PIT_IRQ);
}

/*
 * PIT_read(int32_t fd, void* buf, int32_t nbytes)
 *   DESCRIPTION: NONE
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: NONE
 */
int32_t PIT_read(int32_t fd, void* buf, int32_t nbytes){
    return 0;
}

/*
 * PIT_write(int32_t fd, const void* buf, int32_t nbytes)
 *   DESCRIPTION: NONE
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: NONE
 */
int32_t PIT_write(int32_t fd, const void* buf, int32_t nbytes){
    return 0;
}


/*
 * PIT_open(const uint8_t* filename)
 *   DESCRIPTION: NONE
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: NONE
 */
int32_t PIT_open(const uint8_t* filename){
    return 0;
}

/*
 * PIT_close(int32_t fd)
 *   DESCRIPTION: NONE
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: NONE
 */
int32_t PIT_close(int32_t fd){
    return 0;
}
