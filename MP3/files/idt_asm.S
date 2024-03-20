#define ASM     1
#include "idt.h"
#include "x86_desc.h"

.text

.global keyboard_handler_asm
.global rtc_handler_asm
.global pit_handler_asm

# keyboard_handler_asm
# DESCRIPTION: This is the keyboard handler that handles keyboard interrupts
# INPUTS: NONE
# OUTPUTS: NONE
# RETURN VALUE: NONE
# SIDE EFFECTS: NONE
    keyboard_handler_asm:
        pushal
        call keyboard
        popal
        iret

# rtc_handler_asm
# DESCRIPTION: This is the rtc handler that handles rtc interrupts
# INPUTS: NONE
# OUTPUTS: NONE
# RETURN VALUE: NONE
# SIDE EFFECTS: May change the frequency of the rtc
    rtc_handler_asm:
        pushal
        call rtc
        popal
        iret

# pit_handler_asm
# DESCRIPTION: This is the rtc handler that handles rtc interrupts
# INPUTS: NONE
# OUTPUTS: NONE
# RETURN VALUE: NONE
# SIDE EFFECTS: May change the frequency of the rtc
    pit_handler_asm:
        pushal
        call pit
        popal
        iret

