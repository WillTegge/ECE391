#include "systemcall.h"
#include "types.h"
#include "paging.h"
#include "x86_desc.h"
#include "lib.h"

// helper assembly function that flushes the TLB for us 


FOTP_t rtc_fn = {.read = rtc_read, .write = rtc_write, .open = rtc_open, .close = rtc_close};

FOTP_t file_fn = {.read = file_read, .write = file_write, .open = file_open, .close = file_close};

FOTP_t dir_fn = {.read = dir_read, .write = dir_write, .open = dir_open, .close = dir_close};

FOTP_t stdin_fn = {.read = terminal_read, .open = terminal_open, .close = terminal_close};

FOTP_t stdout_fn = {.write = terminal_write, .open = terminal_open, .close = terminal_close};

// global variables
// uint32_t pids[FILE_DESCRIPTOR_NUM];     // need 1 file descriptor entry per process ???
                                        // entry will be 1 if that pid is taken/used already -> 0 through 7 (8 total allowed)

int current_pid = -1;           // global to keep track of current pid
int pid_array[MAX_PROCESSES] = {0,0,0,0,0,0};

/*
 * halt (uint8_t status)
 *   DESCRIPTION: Halts process and returns to previous
 *                  process or shell depending on current
 *                  process. 
 *   INPUTS: status
 *   OUTPUTS: NONE
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: Updates PCB, PID, and TSS fields
 */
int32_t halt (uint32_t status){

    int i;
    
    pid_array[terminal[display_terminal].latest_pid] = 0;
    
	//refresh shell if process is at base (shell)
    if(PCB[display_terminal]->parents_pid == -1){
    // if(current_pid == 0 || current_pid == 1 || current_pid ==2){ //  
    //  if(pid_array[current_pid] == 1){
        // active_term->latest_pid = -1
        terminal[display_terminal].latest_pid = -1;
        PCB[display_terminal] = NULL;
        execute((uint8_t*)"shell");
    }

    // update pid value
    PCB_t *tmp = PCB[display_terminal];
    terminal[display_terminal].latest_pid = PCB[display_terminal]->parents_pid;
     
    // if(terminal[display_terminal].latest_pid == 0 || terminal[display_terminal].latest_pid == 1 || terminal[display_terminal].latest_pid == 2){
    //     terminal[display_terminal].prev_pid = -1;   
    // }
    
    
    PCB_t *parent_pcb_ptr = (PCB_t*)(PAGE_SIZE_OF_8MB - PAGE_SIZE_OF_8KB*(terminal[display_terminal].latest_pid+1));

    //restore parent paging
    enable_user_page(tmp->parents_pid); // Change to parent's mapping
    FlushTheTLB();

    //Close all FDs

    for(i=FD_START;i<FILE_DESCRIPTOR_NUM;i++){// 8 is the max fd files there can be
        close(i);
    }

    //Write Parent process' info back to TSS(esp0)
    // set the tss -> priveledge switch
    tss.ss0 = KERNEL_DS;

    // CA told us this calc
    tss.esp0 = PAGE_SIZE_OF_8MB - (PAGE_SIZE_OF_8KB * terminal[display_terminal].latest_pid) - FOUR; 

    //execute return
    //update pcb ptr                                                                                        
    PCB[display_terminal] = parent_pcb_ptr;
    // term_parent_pid[display_terminal] = PCB[display_terminal]->parents_pid;

     // Update esp, ebp and return value 
    
    asm volatile("              \n\
        movl %%ecx, %%esp       \n\
        movl %%edx, %%ebp       \n\
        movl %%ebx, %%eax       \n\
        leave                   \n\
        ret                     \n\
        "
        :
        : "c"(tmp->executes_esp), "d"(tmp->executes_ebp), "b"((uint32_t) status)
    );

    return 0;

}

// The execute system call attempts to load and execute a new program, handing off the processor to the new program until it terminates

// command is a space-separated sequence of words
//       - The first word is the file name of the program to be executed, 
//         and the rest of the command—stripped of leading spaces—should be provided to the new
//         program on request via the getargs system call


//  returns -1 if the command cannot be executed,
//  for example, if the program does not exist or the filename specified is not an executable, 256 if the program dies by an
//  exception, or a value in the range 0 to 255 if the program executes a halt system call, in which case the value returned
//  is that given by the program’s call to halt

/*
 * execute(const uint8_t* command)
 *   DESCRIPTION: attempts to load and execute a new program, handing off the processor to the new program until it terminates
 *   INPUTS: command
 *   OUTPUTS: NONE
 *   RETURN VALUE: 0 or -1
 *   SIDE EFFECTS: Updates PCB, PID, and TSS fields
 */
int32_t execute(const uint8_t* command){

// example of command -> 'filename.txt argument1 argument2 argument3' 
int i;
int k;
int temp = 0;

int command_length = strlen((const int8_t*)command);       // length of the command passed into execute

int start_of_args = 0;                           

int file_name_length = 0;
int number_of_spaces = 0;



// STEP 1: Get Arguments
    uint8_t file_name[FILENAMESIZE];       // this is to get the full file name at the start of command
    uint8_t args_in_command[FILENAMESIZE];  // this is to get the args after the first space

    for(i = 0; i < FILENAMESIZE; i++){
        file_name[i] = '\0';
        args_in_command[i] = '\0';
    }

    // loop to extract the file name
    for(i = 0; i < command_length; i++){
        if(command[i] != ' ' && command[i] != NULL && command[i] != '\n' && command[i] != '\t'){
            file_name[start_of_args] = command[i];
            file_name_length++;
            start_of_args++;
        }
        else{
            number_of_spaces++;
            if(file_name_length > 0){
                file_name[i] = 0;
                break;
            }
        }
    }

    start_of_args = start_of_args + number_of_spaces;

    // loop starts at the first argument in the command passed in
    for(i = start_of_args; i < command_length; i++){
    
        // want to delete all the spaces and just get the arguments 
        if(command[i] != ' '){
            for(k = i; k < command_length; k++){
                args_in_command[temp] = command[k];
                temp++;
            }
            break;
        }   
    }
    /*

        if active_terminal->latest_pid == -1:
            pcb->parent_pid == -1
            pid_arr[disp_terminal] = 1
        new_pid = -1
        for i =3 i < 6; i++:
            if pid_arr[i] == 0
            new_pid = i
        if new_pid == -1 return -1
    */
   int pid_check;
    if(terminal[display_terminal].latest_pid == -1){
        if(pid_array[display_terminal] == 0){            //if bases shells aren't established
                terminal[display_terminal].latest_pid = display_terminal;
                pid_array[display_terminal] = 1;            //turn the process on
                terminal[display_terminal].prev_pid = -1;    //set parent to -1
        }else{
                pid_check = -1;                             //if all are active, set this flag to -1
        }
    }else{
        for(i = 3; i < MAX_PROCESSES; i++){              //check remaining process
            if(pid_array[i] == 0){                      //if a process 3 - 6 is open 
                pid_check = i;                          //set flag
                pid_array[i] = 1;                       //turn process on
                terminal[display_terminal].prev_pid = terminal[display_terminal].latest_pid;
                terminal[display_terminal].latest_pid = i; //set current_pid to process;
                break;
            }
            pid_check = -1;
        }
    }

   if(pid_check == -1){
        return -1;
   }
    
// STEP 2: Get the file

    dentry_t current_dentry;
    uint8_t elf_bytes[ELF_BYTES_SIZE];


    // check if file exists 
    if(read_dentry_by_name(file_name, &current_dentry) == -1){

        // if read_dentry_by_name returns -1, that means the file does not exist so we return -1
        pid_array[terminal[display_terminal].latest_pid] = 0;
        terminal[display_terminal].latest_pid = terminal[display_terminal].prev_pid;
        current_pid--;
        return -1;
    }

    // check if it is an executable file by checking the first 4 bytes of the file
    // read the first 4 bytes of the current file
    if(read_data(current_dentry.inode_num, 0, elf_bytes, ELF_FOUR_BYTES) == -1){         // 32 or 4 here? sizeof(uint32_t) = ?

        // if the return value of read_data is -1, then something went wrong when we tried to read the current file
        // so we return -1
        pid_array[terminal[display_terminal].latest_pid] = 0;
        terminal[display_terminal].latest_pid = terminal[display_terminal].prev_pid;
        current_pid--;
        return -1;
    }


    // if we made it here, this means that we have the first 4 bytes of the current file in the buffer
    // now we check if they are equal to the "magic" bytes/number to see if the file is an executable or not

    // MP3 doc (page 21):
    // first 4 bytes of the file represent a “magic number” that identifies the file as an executable. These
    // bytes are, respectively, 0: 0x7f; 1: 0x45; 2: 0x4c; 3: 0x46. If the magic number is not present, the execute system
    // call should fail.
    if((elf_bytes[0] != MAGIC_BYTE_0) || (elf_bytes[1] != MAGIC_BYTE_1) || (elf_bytes[2] != MAGIC_BYTE_2) || (elf_bytes[3] != MAGIC_BYTE_3)){
    
        // if the magic number is not in the first 4 bytes, then the file is not an executable and we return -1
        pid_array[terminal[display_terminal].latest_pid] = 0;
        terminal[display_terminal].latest_pid = terminal[display_terminal].prev_pid;
        current_pid--;
        return -1;
    }

// STEP 3: Setup Paging

    // setup the process in paging here
    enable_user_page(terminal[display_terminal].latest_pid);

    // dont forget to flush the tlb here!!!
    FlushTheTLB();



// STEP 4: Load the file into the page
    int32_t inode_length = ((inode_t*)bbg + 1 + current_dentry.inode_num)->file_size;

    //memcopy the new information into page
    if(read_data(current_dentry.inode_num, 0, (uint8_t*)PROG_IMG_START,inode_length) == -1){
        pid_array[terminal[display_terminal].latest_pid] = 0;
        terminal[display_terminal].latest_pid =  terminal[display_terminal].prev_pid;
        current_pid--;
        return -1;
    }

    // STEP 5: Bookkeeping Stuff
    // - PCB
    // - pids

    // populate new PCB (save important values to it) 
    PCB[display_terminal] = (PCB_t*)(PAGE_SIZE_OF_8MB - PAGE_SIZE_OF_8KB * (terminal[display_terminal].latest_pid+1));
    PCB[display_terminal]->current_pid = terminal[display_terminal].latest_pid;

    // update or set the parent_pid if necessary
    if(terminal[display_terminal].prev_pid != -1){
        // update parent to be the current parent
        // update the current parent to be the current pid
        // term_parent_pid[display_terminal] = terminal[display_terminal].prev_pid;
        PCB[display_terminal]->parents_pid = terminal[display_terminal].prev_pid;
    }
    else{
        // term_parent_pid[display_terminal] = terminal[display_terminal].latest_pid;
        PCB[display_terminal]->parents_pid = -1;
    }


    // initialize all the descriptors in the file descriptor array
    for(i = 0; i < FILE_DESCRIPTOR_NUM; i++){ 
        PCB[display_terminal]->fd_array[i].flags = 0;                         //set flag -> 0 means unused
        PCB[display_terminal]->fd_array[i].FOTP = NULL;     //file type           //point to file operation table
        PCB[display_terminal]->fd_array[i].inode = 0;    //get inode number from dentry
        PCB[display_terminal]->fd_array[i].file_pos = 0;         
    }
    

    // setup the first 2 spots of the file descriptor array to be stdin and stdout
    // stdin 
    PCB[display_terminal]->fd_array[0].flags = 1;                         //set flag -> 1 because its in use
    PCB[display_terminal]->fd_array[0].FOTP = &stdin_fn;     //file type           //point to file operation table
    PCB[display_terminal]->fd_array[0].inode = 0;                         //get inode number from dentry
    PCB[display_terminal]->fd_array[0].file_pos = 0;

    // stdout
    PCB[display_terminal]->fd_array[1].flags = 1;                         //set flag -> 1 because its in use
    PCB[display_terminal]->fd_array[1].FOTP = &stdout_fn;     //file type           //point to file operation table
    PCB[display_terminal]->fd_array[1].inode = 0;                         //get inode number from dentry
    PCB[display_terminal]->fd_array[1].file_pos = 0;
    

    // copy the arguments over to the pcb
    strncpy((int8_t*)PCB[display_terminal]->arguments, (int8_t*)args_in_command, FILENAMESIZE);
    
    

// STEP 6: Context Switch

    // in this part we are setting up the arguments that get used/popped by IRET to jump to where we want to go
    // set 5 things up: EIP, ESP, CS, SS, FLAGS


    // find the entry point -> bytes 24 through 27 of the ELF -> gives you EIP
    // use this to jump to user program later

    uint8_t user_eip_from_elf[ELF_BYTES_24_TO_27];
    uint32_t user_eip; 
    uint32_t user_esp;


    // read the data and find the beginning of the entry point we need (eip)
    read_data(current_dentry.inode_num, ELF_ENTRY_POINT, user_eip_from_elf, ELF_FOUR_BYTES);

    user_eip = *((int*)user_eip_from_elf);      // need to convert it to 4 bytes
    user_esp = USER_MEMORY_START + PAGE_SIZE_OF_4MB - sizeof(int32_t);                  // double check this calculation later ???

    // save the eip and esp of the user into the current PCB
    PCB[display_terminal]->users_eip = user_eip;
    PCB[display_terminal]->users_esp = user_esp;


    // set the tss -> priveledge switch
    tss.ss0 = KERNEL_DS;

    // CA told us this calc
    tss.esp0 = PAGE_SIZE_OF_8MB - (PAGE_SIZE_OF_8KB * terminal[display_terminal].latest_pid) - sizeof(int32_t);       // double check this calculation
                                                                                            // sizeof(int32_t) should be 4 !!!

    // we need to save the esp0 into our current PCB
    PCB[display_terminal]->tss_esp_0 = tss.esp0;

    // ADDED THIS JUST FOR SCHEDULING 
    PCB[display_terminal]->tss_ss0 = tss.ss0; 


    // use assembly to get the esp and ebp of the execute function
    // can send an assembly register value to a C variable using the example code below (stack overflow):


    uint32_t current_execute_ebp;
    uint32_t current_execute_esp;


    asm("\t movl %%ebp,%0" : "=r"(current_execute_ebp));
    asm("\t movl %%esp,%0" : "=r"(current_execute_esp));

    // then store these into our current PCB 
    PCB[display_terminal]->executes_ebp = current_execute_ebp;
    PCB[display_terminal]->executes_esp = current_execute_esp;

    


    // assembly linkage code to do the context switch 
    // IRET's context
    // needs to be volatile so that the compiler doesnt change/reorder the assembly code (we dont want it to be optimized)

    // disable ints

    /* args to push to stack:

        eax = user_eip 
        edx = user_esp
        ecx = USER_CS
        ebx = USER_DS
        flags

    */ 
    
    asm volatile ("     \n\
        pushl %1        \n\
        pushl %3        \n\
        pushfl          \n\
        pushl %2        \n\
        pushl %0        \n\
        iret            \n\
        "
        :
        : "g"(user_eip), "g"(USER_DS), "g"(USER_CS), "g"(user_esp)
    );

    return 0;

}




/*
 * read (int32_t fd, void* buf, int32_t nbytes)
 *   DESCRIPTION: 
 *   INPUTS: fd - file descriptor
 *           buf - buffer
 *           nbytes -
 *   OUTPUTS: NONE
 *   RETURN VALUE: -1 - error, PCB FOTPs
 *   SIDE EFFECTS: NONE
 */
int32_t read (int32_t fd, void* buf, int32_t nbytes){
    //check fd and buf are valid
    if((fd < 0 || fd == 1 || fd > MAX_FD) || (buf == NULL)){
        return -1;
    }

    //check flag to see if its closed 
    if(PCB[display_terminal]->fd_array[fd].flags == 0){
        return -1;
    }

    return PCB[display_terminal]->fd_array[fd].FOTP->read(fd, buf, nbytes);
}

/*
 * write (int32_t fd, const void* buf, int32_t nbytes)
 *   DESCRIPTION: 
 *   INPUTS: fd - file descriptor
 *           buf - buffer
 *           nbytes -
 *   OUTPUTS: NONE
 *   RETURN VALUE: -1 - error, PCB FOTP
 *   SIDE EFFECTS: NONE
 */
int32_t write (int32_t fd, const void* buf, int32_t nbytes){
    //check fd and buf are valid 
    if((fd < MIN_FD || fd > MAX_FD) || (buf == NULL)){
        return -1;
    }

    //check flag to see if its closed 
    if(PCB[display_terminal]->fd_array[fd].flags == 0){
        return -1;
    }
    
    return PCB[display_terminal]->fd_array[fd].FOTP->write(fd, buf, nbytes);
}

/*
    check if input is null
    initialize entry array for fd into fd_array
    first we use read by name function to read info
*/

/*
 * open (const uint8_t* filename)
 *   DESCRIPTION: Sets a file descriptor array entry
 *                for the file with the given filename
 *   INPUTS: filename
 *   OUTPUTS: NONE
 *   RETURN VALUE: -1 - error, file descriptor
 *   SIDE EFFECTS: NONE
 */
int32_t open (const uint8_t* filename){
    //move to user -> kernel
    //^statically allocate in kernel ; create static string 33 long 
    //static var her for filrname  static = filename
    // strncpy(kernel_filename, filename, 33);

    if(filename == NULL){
        return -1;
    }

    dentry_t temp_dentry;
    //use read dentry 
    //do check for -1
    if(read_dentry_by_name(filename, &temp_dentry) == -1){
        return -1;
    }

    //file position gets set to 0 
    //inode gets set via d_entry
    //flag goes to 1
    int i;
    int ftype = temp_dentry.file_type;

    for(i = 0; i < FD_ARRAY_LENGTH; i++){ 
        if(PCB[display_terminal]->fd_array[i].flags == 0){
            PCB[display_terminal]->fd_array[i].flags = 1;                         //set flag
            PCB[display_terminal]->fd_array[i].inode = temp_dentry.inode_num;    //get inode number from dentry
            PCB[display_terminal]->fd_array[i].file_pos = 0;                      //begin reading at start of file 
            switch(ftype){
                case 0: 
                    PCB[display_terminal]->fd_array[i].FOTP = &rtc_fn; //rtc open
                    break;
                case 1:
                    PCB[display_terminal]->fd_array[i].FOTP = &dir_fn;
                    break;
                case 2:
                    PCB[display_terminal]->fd_array[i].FOTP = &file_fn; 
                    break; 
            }

            PCB[display_terminal]->fd_array[i].FOTP->open(filename);
            return i;
        }
    }
    return -1;
}

/*
 * close (int32_t fd)
 *   DESCRIPTION: Clears the file descriptor array fields are given
 *                filedescriptor.
 *   INPUTS: fd - file descriptor
 *   OUTPUTS: NONE
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: NONE
 */
int32_t close (int32_t fd){
    //check fd valid
    if((fd < FD_START) || (fd > MAX_FD)){
        return -1;
    }
    // check if already closed
    if(PCB[display_terminal]->fd_array[fd].flags == 0){
        return -1;
    }
    // clear directory info for that file descriptor in task array 
    PCB[display_terminal]->fd_array[fd].FOTP = NULL;
    PCB[display_terminal]->fd_array[fd].inode = NULL;
    PCB[display_terminal]->fd_array[fd].file_pos = NULL;
    PCB[display_terminal]->fd_array[fd].flags = NULL;
    return 0;

}






// other functions needed (defined but ill do them later after execute/halt)
/*
 * getargs(uint8_t* buf, int32_t nbytes)
 *   DESCRIPTION: 
 *   INPUTS: buf - buffer
 *           nbytes - number of bytes
 *   OUTPUTS: 0 is successful read and copy 
*             -1 for errors with inputs or arguments
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: arguments from pcb are copied in to input buffer
 */
int32_t getargs(uint8_t* buf, int32_t nbytes){
    //may need to change thsi to PCB[display_terminal]
    PCB_t* pcb_ptr =  (PCB_t*)(PAGE_SIZE_OF_8MB - PAGE_SIZE_OF_8KB*(terminal[display_terminal].latest_pid+1)); // 0x800000 or 8mb is the kenel bottom stack  address
    int i;
    for(i =0; i <nbytes; i++){
        buf[i] = '\0';
    }
// user kenerL stack size 0x2000 or 8kb
//if the buffer is empty or the command argument is invald, or bytes NULL, return -1                                                  
 
    if(pcb_ptr->arguments[0] == NULL || !nbytes ){
        return -1;
    }
//copy argument data to buff
    for( i =0; i < strlen((int8_t*)pcb_ptr->arguments); i++){
        if(pcb_ptr->arguments[i] != '\n'){
              buf[i] = pcb_ptr->arguments[i];
    }
    }
    if(buf[0] == NULL){
        return -1;
    }
//  strncpy((uint8_t*) buf,(int8_t*)pcb_ptr->arguments,nbytes);
    return 0;

}

/* 
 * vidmap(uint8_t** screen_start)
 *   DESCRIPTION:  map video memory to user space
 *   INPUTS: screen_start - the starting addr of virtual memory
 *   OUTPUTS: 0 if succesful mapping
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: new page is created and mapping is changed
 */
int32_t vidmap(uint8_t** screen_start){

//see if location is valid                                // 8KB is x00002000
    if (!screen_start || (uint32_t)screen_start < USER_MEMORY_START || (uint32_t)screen_start > (USER_MEMORY_START + PAGE_SIZE_OF_4MB)){//page addres for user is x8000000, useR page size is 4mb
        return -1;}//should be x08400000

    
//write virtual address into location provided by caller func
    *screen_start = (uint8_t *)(USER_MEMORY_START + PAGE_SIZE_OF_4MB);//video mapping addr is 128 mb plus its page size
    
    //uint32_t PDE_i= (USER_MEMORY_START + PAGE_SIZE_OF_4MB) >> 22; // since the size is 4mb we know the offset is 22

    // add another page mapping 
    enable_video_page();

    // Flush the TLB since a new page has been created 
    //getting rid of old entries

    FlushTheTLB();
    return 0;

}




/*
 * set_handler(int32_t signum, void* handler_address)
 *   DESCRIPTION: our operating system does not support signals, so just return -1
 *   INPUTS: signum - 
 *           handler_address - 
 *   OUTPUTS: -1
 *   RETURN VALUE: -1
 *   SIDE EFFECTS:NONE 
 */
int32_t set_handler(int32_t signum, void* handler_address){

    return -1;

}


/*
 * sigreturn(void)
 *   DESCRIPTION: our operating system does not support signals, so just return -1
 *   INPUTS:  NONE
 *   OUTPUTS: -1 
 *   RETURN VALUE: -1
 *   SIDE EFFECTS:NONE 
 */
int32_t sigreturn(void){

    return -1;

}

// void schedule() {
//     // Add new processes to the queue
//     // ...

//     // Select the next process to run
//     PCB_t* next_pcb = select_next_process();

//     // Restore process state
//     set_esp(next_pcb->registers.esp);
//     set_ebp(next_pcb->registers.ebp);
//     set_eip(next_pcb->registers.eip);
//     set_cs(next_pcb->registers.cs);
//     set_eflags(next_pcb->registers.eflags);
// }

//add inside halt
    // ...

    // // Unmap video memory for the current process
    // if (get_active_terminal() == current_pid) {
    //     unmap_video_memory(PAGE_DIRECTORY_ADDR);
    // }

    // // Remove the process from the scheduler
    // remove_process_from_scheduler(current_pid);

    // // Reset the TSS for this process
    // reset_tss(current_pid);

    // // Update the current process ID
    // current_pid = get_next_process_id();

    // // Map video memory for the new current process
    // if (get_active_terminal() == current_pid) {
    //     map_video_memory(PAGE_DIRECTORY_ADDR);
    // } else {
    //     // Map virtual addresses to backing store
    //     map_backing_store(PAGE_DIRECTORY_ADDR);
    // }

    // // Load the TSS for the new current process
    // load_tss(current_pid);

    // // Switch to the new process
    // switch_to_process(current_pid);

    // ...

// }

//add inside execute
    // ...

    // int pid = create_process(command);

    // // Map video memory for the new process
    // if (get_active_terminal() == pid) {
    //     map_video_memory(PAGE_DIRECTORY_ADDR);
    // } else {
    //     // Map virtual addresses to backing store
    //     map_backing_store(PAGE_DIRECTORY_ADDR);
    // }

    // // Load the TSS for the new process
    // load_tss(pid);

    // // Switch to the new process
    // switch_to_process(pid);

    // ...

// }
