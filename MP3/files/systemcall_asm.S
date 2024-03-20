#define ASM     1
#include "x86_desc.h"

.text


.align 4
systemcall_table:
    .long 0x0           # space holder for spot 0 since there is no number 0 for the system call function numbers
    .long halt
    .long execute
    .long read
    .long write
    .long open
    .long close
    .long getargs
    .long vidmap
    .long set_handler
    .long sigreturn


# systemcall_handler
# DESCRIPTION: This is the system call handler that handles all the system calls
# INPUTS: flags, opcode/offset of system call to execute
# OUTPUTS: NONE
# RETURN VALUE: -1 for an error
# SIDE EFFECTS: returns from an interrupt
.globl systemcall_handler
.align 4
systemcall_handler:
    
    # push all registers but eax

    # pushl %esp
    pushl %ebp
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %ebx
    pushl %esi
    

    # push all flags
    pushfl


    # push args
    pushl %edx
    pushl %ecx
    pushl %ebx


    cmpl $0, %eax
    jle systemcall_error        # error if  <= to 0

    cmpl $10, %eax
    ja systemcall_error         # error if > 10


    # make the call
    call *systemcall_table(, %eax, 4)

    jmp systemcall_done


systemcall_error: 
    movl $-1, %eax              # return -1 for an error


systemcall_done:
    # pop args
    addl $12, %esp                

    # pop all flags
    popfl          


    # pop all registers but eax
    popl %esi
    popl %ebx
    popl %edi
    popl %edx
    popl %ecx
    popl %ebp
    # popl %esp


    # do an iret (interrupt/system call so NOT A RET)
    iret

    




