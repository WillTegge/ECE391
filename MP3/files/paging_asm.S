#define ASM     1
#include "paging.h"
#include "x86_desc.h"

.text

.globl loadPageDirectory
.globl enablePaging


# loadPageDirectory
# DESCRIPTION: This assembly function loads the page directory
# INPUTS: cr3 base address
# OUTPUTS: NONE
# RETURN VALUE: NONE
# SIDE EFFECTS: loads the page directory at the address in cr3
loadPageDirectory:
    pushl %ebp
    movl %esp, %ebp

    /* give cr3 base address */
    movl 8(%esp), %eax  
    movl %eax, %cr3

    /* enable 4MB in cr4 bit */
    movl %cr4, %eax
    orl $0x10, %eax
    movl %eax, %cr4

    leave
    ret


# enablePaging
# DESCRIPTION: This assembly function enables paging
# INPUTS: NONE
# OUTPUTS: NONE
# RETURN VALUE: NONE
# SIDE EFFECTS: enables our paging
enablePaging:       
    pushl %ebp
    movl %esp, %ebp

    /* enable paging in cr0 */
    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0

    leave
    ret


# added this assembly helper function
# flushes the TLB 
# FlushTheTLB
# DESCRIPTION: This function flushes the tlb
# INPUTS: NONE
# OUTPUTS: NONE
# RETURN VALUE: NONE
# SIDE EFFECTS: flushes the tlb
.globl	FlushTheTLB
.align 4
FlushTheTLB:        # just move cr3 and eax to each other to clear the TLB (according to discussion TA)
    movl %cr3, %eax
    movl %eax, %cr3
    ret
    


    
