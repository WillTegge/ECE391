# #define ASM     1
# #include "PIT.h"
# #include "x86_desc.h"
# 
# 
# # taken from OSDEV
# section .bss
# system_timer_fractions:  resd 1          ; Fractions of 1 ms since timer initialized
# system_timer_ms:         resd 1          ; Number of whole ms since timer initialized
# IRQ0_fractions:          resd 1          ; Fractions of 1 ms between IRQs
# IRQ0_ms:                 resd 1          ; Number of whole ms between IRQs
# IRQ0_frequency:          resd 1          ; Actual frequency of PIT
# PIT_reload_value:        resw 1          ; Current PIT reload value
# section .text
# 
# 
# 
# # handler 
# .globl PIT_handler
# .align 4
# PIT_handler:
# 
#     
# global set_esp
# set_esp:
#    mov esp, [esp+4]  # load the value of the first argument into ESP
#    ret               # return from the function

# global set_ebp
# set_ebp:
#    mov ebp, [esp+4]  # load the value of the first argument into EBP
#    ret

