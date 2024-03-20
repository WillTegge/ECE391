
.data					# section declaration

        # Useful offset constants for accessing members of a 
        # struct mp1_blink_struct structure
        LOCATION   = 0    
        ON_CHAR    = 2
        OFF_CHAR   = 3 
        ON_LENGTH  = 4
        OFF_LENGTH = 6
        COUNTDOWN  = 8
        STATUS     = 10
        NEXT       = 12

        STRUCT_SIZE = 16

# Pointer to head of list (initialized to NULL)
mp1_list_head:
        .long   0

.text					# section declaration

# Export the function symbol names

.global mp1_rtc_tasklet
.global mp1_ioctl

# void mp1_poke(void);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX
mp1_poke:
        
	movl    vmem_base_addr(,1),%edx
	movb    %cl,(%edx,%eax,1)
	ret



# helper function that searches for the element in the list with the passes in location
# returns ptr to the element with the matching location 
# SEARCH_FOR_LOCATION:
#
#        # set up stack frame
#        pushl %ebp
#        movl %esp, %ebp
#
#        # callee-save registers
#        pushl %ebx
#        pushl %edi
#        pushl %esi
#
#        xorl %edi, %edi
#        movw 8(%ebp), %di              # store location we are searching for into edi
#
#        movl mp1_list_head, %esi
#        cmpl $0, %esi   
#        je SEARCH_FOR_LOCATION_END
#        jmp SEARCH_CHECK_NEXT
#
# SEARCH_START_OF_LOOP:
#
#        cmpl $0, %esi   
#        je SEARCH_ERROR
#
#        cmpw LOCATION(%esi), %di       # check if current location matches the passed in location
#        jne SEARCH_CHECK_NEXT           # jumps to next element if the locations are not equal
#
#        movl %esi, %eax                # return ptr to element with matching location  
#        jmp SEARCH_FOR_LOCATION_END
#
# SEARCH_CHECK_NEXT:
#        movl NEXT(%esi), %esi
#        jmp SEARCH_START_OF_LOOP
#
# SEARCH_ERROR:
#        movl $-1, %eax
#
# SEARCH_FOR_LOCATION_END:
#
#        # callee-restore
#        popl %esi
#        popl %edi
#        popl %ebx
#
#        leave
#	ret



# Interface: Updates the state of the fishes
#    Inputs: unsigned long (not used though)
#   Outputs: The fish will blink between "on" and "off" on the Text-mode video screen 
# Registers: Clobbers EDX, EAX, ECX
mp1_rtc_tasklet:

        # set up stack frame
        pushl %ebp
        movl %esp, %ebp

        # callee-save registers
        pushl %ebx
        pushl %edi
        pushl %esi

        # get the head of the linked list and check if it is NULL
        # beginning of loop

        movl mp1_list_head, %esi
        cmpl $0, %esi   
        je TASKLET_END


START_OF_LOOP:

        cmpl $0, %esi   
        je TASKLET_END

        # decrement countdown field
        decw COUNTDOWN(%esi)

         # check if countdown = 0, if it is then check status field
        cmpw $0, COUNTDOWN(%esi)
        jne CHECK_NEXT              # BUG HERE

        cmpw $0, STATUS(%esi)           # check if its off, if it is, then turn it on
        je TURN_ON
        
        cmpw $1, STATUS(%esi)           # check if its off, if it is, then turn it on
        je TURN_OFF

        jmp CHECK_NEXT

TURN_ON:                                # if status = 0, the location is currently off. turn if on by outputting the on_char with mp1_poke 
                                        # copy on_length to countdown


        xorl %eax, %eax
        movw ON_LENGTH(%esi), %ax
        movw %ax, COUNTDOWN(%esi)

        movw $1, STATUS(%esi)           # set STATUS = 1 (turning it on in this case)


        xorl %eax, %eax                 # put location into eax for mp1_poke
        movw LOCATION(%esi), %ax
        shlw %ax

        xorl %ecx, %ecx                 # get the on_char into cl for mp1_poke
        xorl %edx, %edx
        movb ON_CHAR(%esi), %cl

        call mp1_poke

        jmp CHECK_NEXT

TURN_OFF:                               # if status = 1, the location is currently on. turn off by outputting the off_char with mp1_poke
                                        # copy off_length to countdown 

        xorl %eax, %eax                 # put location into eax for mp1_poke
        movw OFF_LENGTH(%esi), %ax
        movw %ax, COUNTDOWN(%esi)

        movw $0, STATUS(%esi)           # STATUS = 0 (turning it on in this case)

        xorl %eax, %eax
        movw LOCATION(%esi), %ax
        shlw %ax

        xorl %ecx, %ecx                  # get the off_char into cl for mp1_poke
        xorl %edx, %edx
        movb OFF_CHAR(%esi), %cl

        call mp1_poke

        jmp CHECK_NEXT


CHECK_NEXT:     # go to next element in the list
        
        movl NEXT(%esi), %esi
        jmp START_OF_LOOP


TASKLET_END:

        # callee-restore
        popl %esi
        popl %edi
        popl %ebx

        leave
	ret



# Interface: Dispatcher that jumps to ioctls based on the cmd value
#    Inputs: cmd, arg (both unsigned longs)
#   Outputs: Does not output anything, returns -1 if the cmd value is invalid 
# Registers: Clobbers ECX
mp1_ioctl:              # TESTED AND WORKING!

        movl 8(%esp), %ecx      # get cmd value

        # movl 4(%esp), %edi                      # get the pointer to struct into edi            # EDI IS NULL SO ITS NOT DRAWING!!!
        # cmpl $0, %edi                           #  edi - 0 (NULL)
        # je ERROR_SO_FREE_MEMORY                 # jumps if pointer is 0 (NULL)

        cmpl $3, %ecx           # ecx - 3       (3 is upper bound since cmd can only be 0,1,2 or 3)
        jg INVALID              # jumps if cmd > 3
       
        cmpl $0, %ecx           # ecx - 0       (0 is lower bound since cmd can only be 0,1,2 or 3)
        jl INVALID              # jumps if cmd < 0

        jmp *jump_table(,%ecx,4)        # jump to function based on cmd value

        INVALID:
                movl $-1, %eax  # cmd is invalid so return -1
                ret

jump_table: .long mp1_ioctl_add, mp1_ioctl_remove, mp1_ioctl_find, mp1_ioctl_sync



# Interface: Inserts a new element into the linkedlist
#    Inputs: user-level pointer to an mp1_blink_struct structure
#   Outputs: The ascii fish will appear on the Text-mode video screen 
# Registers: Clobbers ECX, EDX
mp1_ioctl_add:                  # TESTED AND WORKING!

        # set up stack frame
        pushl %ebp
        movl %esp, %ebp

        # callee-save registers
        pushl %ebx
        pushl %edi
        pushl %esi

        movl 8(%ebp), %edi                      # get the pointer to struct into edi
        cmpl $0, %edi                           #  edi - 0 (NULL)
        je MP1_MALLOC_FAILED                 # jumps if pointer is 0 (NULL)

        # allocate memory to store copy of the structure (use mp1_malloc) (check for error)
        pushl $16                      # push args
        call mp1_malloc
        addl $4, %esp                           # pop args

        cmpl $0, %eax                          
        je MP1_MALLOC_FAILED                    # jumps if malloc returned 0 (NULL)

        # copy the mp1_blink_struct from user-space to the newly-allocated memory (use mp1_copy_from_user) (check for error)
        movl %eax, %ebx                         # get the pointer to new memory

        pushl $16                               # push args
        pushl %edi                              # void* from
        pushl %ebx                              # void* to

        call mp1_copy_from_user

        popl %ebx
        popl %edi
        addl $4, %esp                          # pop args

        cmpl $0, %eax                           # check if return value is 0
        jne ERROR_SO_FREE_MEMORY                # jumps if the return value doesnt equal 0

        # check location and set the countdown field to be equal to the on_length field    
        xorl %eax, %eax
        # movw $80, %ax
        # imulw $25, %ax, %ax
        # addw $-1, %ax                   # eax should contain 80*25-1
        # movw $1999, %ax
        
        # xorl %esi, %esi
        movw LOCATION(%ebx), %ax       # put location value into esi

        cmpw $1999, %ax                 # ax - 1999 (1999 is upper bound -> 80*25-1 = 1999)
        jg ERROR_SO_FREE_MEMORY         # jumps if ax > 1999 (location > 80*25-1)
        cmpw $0, %ax                    # ax - 0
        jl ERROR_SO_FREE_MEMORY         # jumps if ax < 0 (location < 0)

        xorl %eax, %eax
        movw ON_LENGTH(%ebx), %ax
        movw %ax, COUNTDOWN(%ebx)

        # set the status field to 1
        movw $1, STATUS(%ebx)

        # insert new structure at the head of linked list using the mp1_list_head pointer
        movl mp1_list_head, %esi
        movl %esi, NEXT(%ebx)
        movl %ebx, mp1_list_head

       
        # call mp1_poke to display character on video screen
        xorl %eax, %eax
        movw LOCATION(%ebx), %ax
        shlw %ax

        xorl %ecx, %ecx
        xorl %edx, %edx
        movb ON_CHAR(%ebx), %cl

        call mp1_poke                   
        
        movl $0, %eax                   # return 0 if the add was successful
        jmp ADD_END

ERROR_SO_FREE_MEMORY:
        pushl %ebx
        call mp1_free
        popl %ebx
       
MP1_MALLOC_FAILED:
        movl $-1, %eax

ADD_END:
        popl %esi
        popl %edi
        popl %ebx

        leave
        ret

        
# Interface: Removes an element whose location value matches a given one from the linkedlist
#    Inputs: unsigned long arg which is an integer location 
#   Outputs: returns 0 if successful and -1 if it failed. 
# Registers: Clobbers ECX, EDX
mp1_ioctl_remove:

        # set up stack frame
        pushl %ebp
        movl %esp, %ebp

        # callee-save registers
        pushl %ebx
        pushl %edi
        pushl %esi

        movw 8(%ebp), %cx               # get integer location arg from the stack into ecx

        movl mp1_list_head, %edi
        cmpl $0, %edi
        je REMOVE_FAILED

        cmpw LOCATION(%edi), %cx        # check if we need to remove the head
        je REMOVE_HEAD_CASE

        movl %edi, %ebx                                # Store previous NEXT ptr into ebx
        movl NEXT(%edi), %edi
        cmpl $0, %edi
        je REMOVE_FAILED

REMOVE_LOOP_START:

        cmpw LOCATION(%edi), %cx                  # CAN YOU DO THIS SYNTAX ???
        je REMOVE_FOUND

        movl %edi, %ebx                                # Store previous NEXT ptr into ebx
        movl NEXT(%edi), %edi
        cmpl $0, %edi
        je REMOVE_FAILED

        jmp REMOVE_LOOP_START


REMOVE_FOUND:
        # at this point current is in edi
        movl NEXT(%edi), %esi           # save NEXT ptr into esi
        movl %edi, %eax                 # save a copy of the ptr to the element we need to delete/free

        
        movl %esi, %edi                 # set the current to point to what the next pointed to
        movl %esi, NEXT(%ebx)
        # movl %esi, %ebx 
        movl %eax, %ebx                 # put ptr that you want to free into ebx for mp1_free
        jmp REMOVE_FREE_MEMORY


REMOVE_HEAD_CASE:

        movl NEXT(%edi), %esi
        movl %esi, mp1_list_head
        movl %edi, %ebx

REMOVE_FREE_MEMORY:

        pushl %ebx
        call mp1_free
        popl %ebx

        movl $0, %eax
        jmp REMOVE_END

REMOVE_FAILED:

        movl $-1, %eax

REMOVE_END:

        # callee-restore
        popl %esi
        popl %edi
        popl %ebx
        leave
	ret
       


# Interface: Find an element whose location value matches a given one from the linkedlist
#    Inputs: user-level pointer to an mp1_blink_struct structure
#   Outputs: returns 0 if successful and -1 if it failed. 
# Registers: Clobbers ECX, EDX
mp1_ioctl_find:                 # FINSIHED BUT NOT TESTED

        # set up stack frame
        pushl %ebp
        movl %esp, %ebp

        # callee-save registers
        pushl %ebx
        pushl %edi
        pushl %esi

        movl 8(%ebp), %edi                      # get the pointer to struct passed in into edi
        cmpl $0, %edi                           #  edi - 0 (NULL)
        je RETURN_NEG_ONE

        # get the location from user-level structure passed in
        xorl %eax, %eax
        movw LOCATION(%edi), %ax
 
        # get the mp1_list_head and check if it is NULL
        movl mp1_list_head, %esi
        cmpl $0, %esi
        je RETURN_NEG_ONE
        # jmp FIND_CHECK_NEXT           # COMMENTED OUT BC MIGHT BE SKIPPING FIRST ELEMENT !!!

SEARCH_FOR_ELEMENT:

        cmpl $0, %esi
        je RETURN_NEG_ONE
 
        # get the current location into a register
        cmpw LOCATION(%esi), %ax
        je FOUND_ELEMENT
 
FIND_CHECK_NEXT:     # go to next element in the list
         
        movl NEXT(%esi), %esi
        jmp SEARCH_FOR_ELEMENT


FOUND_ELEMENT:

        # use mp1_copy_to_user to copy the entire element (mp1_blink_Struct) to the user-level structure that was passed in
 
        pushl $16                               # push args             struct size in bytes
        pushl %esi                              # void* from            should be ptr to mp1_blink_Struct
        pushl %edi                              # void* to              should be user-level structure that was passed in
 
        call mp1_copy_to_user

        popl %edi
        popl %esi
        addl $4, %esp                          # pop args
 
        cmpl $0, %eax                           # check if return value is 0
        jne FREE_MEMORY                      # jumps if the return value doesnt equal 0 (means mp1_copy_to_user failed to copy)

        # return 0
        movl $0, %eax 
        jmp FIND_END
 

FREE_MEMORY:
        pushl %edi
        call mp1_free
        popl %edi

        
RETURN_NEG_ONE:
 
        movl $-1, %eax
 
FIND_END:
 
        popl %esi
        popl %edi
        popl %ebx
 
        leave
        ret




# Interface: Syncronizes the blinks at two given locations passed in
#    Inputs: unsigned long argument which is really two two-byte unsigned short integers packed into 32 bits
#   Outputs: returns 0 if successful and -1 if it failed. The I/M should blink and eventually blink whenever the fish does (synchronizes with fish)
# Registers: Clobbers ECX, EDX
mp1_ioctl_sync:

        # arg passed in is two-byte unsigned short integers packed into 4-byte argument
        # upper 16 bits - first integer (location of first blinking character)
        # lower 16 bits - second integer (location of the second blinking character) 
        # (need to syncronize the second one with the first)


        # set up stack frame
        pushl %ebp
        movl %esp, %ebp

        # callee-save registers
        pushl %ebx
        pushl %edi
        pushl %esi

        # extract both integers
        movl 8(%ebp), %ecx              
        cmpl $0, %ecx
        je SYNC_FAILED


        andl $0xFFFF0000, %ecx          # extract the first integer (upper 16 bits)
        rorl $16, %ecx                  # rotate it to get the first integer into the lsb of ecx (rotate by 16 bits in this case)
        

# THIS CODE FINDS THE FIRST LOCATION

        # search for element with the location
        movl mp1_list_head, %edi
        cmpl $0, %edi   
        je SYNC_FAILED

        # cmpw LOCATION(%edi), %cx       # check if current location matches the passed in location
        # jne SEARCH_CHECK_NEXT           # jumps to next element if the locations are not equal
        # movl %edi, %eax                # return ptr to element with matching location  
        # jmp SEARCH_FOR_LOCATION_END
        
        # check if location is valid
        # cmpw $1999, %cx                 # ax - 1999
        # jg SYNC_FAILED         # jumps if ax > 1999 (location > 80*25-1)
        # cmpw $0, %cx                   # ax - 0
        # jl SYNC_FAILED         # jumps if ax < 0 (location < 0)


SEARCH_START_OF_LOOP:

        cmpw LOCATION(%edi), %cx       # check if current location matches the passed in location
        je SEARCH_FOR_LOCATION_END           # jumps to end if the locations are equal

        # movl %edi, %esi                # return ptr to element with matching location  
SEARCH_CHECK_NEXT:
        movl NEXT(%edi), %edi
        cmpl $0, %edi   
        je SYNC_FAILED
        jmp SEARCH_START_OF_LOOP


SEARCH_FOR_LOCATION_END:
                                        # GDB SAYS IT DOESNT GET PASSED HERE !!!

        # At this point, the ptr to the element with the first integer location is in esi
        movl %edi, %esi                # return ptr to element with matching location 

        # movl %eax, %esi                 # store ptr to element with first integer location into esi


# THIS CODE FINDS THE SECOND LOCATION
        movl 8(%ebp), %ecx
        andl $0x0000FFFF, %ecx          # get the second integer (lower 16 bits)
        # rorl $16, %ecx                  # rotate right by 16 bits

        # search for element with the location
        movl mp1_list_head, %edi
        # cmpl $0, %edi   
        # je SYNC_FAILED
        
SEARCH_START_OF_LOOP_2:

        cmpl $0, %edi   
        je SYNC_FAILED

        cmpw LOCATION(%edi), %cx       # check if current location matches the passed in location
        je SEARCH_FOR_LOCATION_END_2           # jumps to next element if the locations are not equal

SEARCH_CHECK_NEXT_2:
        movl NEXT(%edi), %edi
        jmp SEARCH_START_OF_LOOP_2


SEARCH_FOR_LOCATION_END_2:

        # at this point -> edi holds ptr to 2nd, 
        # esi holds ptr to first

        # copy ON_LENGTH of first to the second
        xorl %ebx, %ebx
        movw ON_LENGTH(%esi), %bx
        movw %bx, ON_LENGTH(%edi)

        # copy OFF_LENGTH of first to the second
        xorl %ebx, %ebx
        movw OFF_LENGTH(%esi), %bx
        movw %bx, OFF_LENGTH(%edi)

        # copy COUNTDOWN of first to the second
        xorl %ebx, %ebx
        movw COUNTDOWN(%esi), %bx
        movw %bx, COUNTDOWN(%edi)

        # copy STATUS of the first to the second
        xorl %ebx, %ebx
        movw STATUS(%esi), %bx
        movw %bx, STATUS(%edi)


        # write the element at 2nd location to the screen using mp1_poke

        cmpw $0, STATUS(%edi)           # check if its off
        je DRAW_OFF

DRAW_ON:                                # if status = 1, the location is currently on. draw it by outputting the on_char with mp1_poke 

        xorl %eax, %eax                 # get the location into eax
        movw LOCATION(%edi), %ax
        shlw %ax

        xorl %ecx, %ecx                 # get the ON_CHAR character into cl
        xorl %edx, %edx
        movb ON_CHAR(%edi), %cl

        pushl %edx
        call mp1_poke
        popl %edx

        movl $0, %eax
        jmp SYNC_END


DRAW_OFF:                               # if status = 0, the location is currently off. draw off by outputting the off_char with mp1_poke

        xorl %eax, %eax                 # get the location into eax
        movw LOCATION(%edi), %ax
        shlw %ax

        xorl %ecx, %ecx                 # get the OFF_CHAR character into cl
        xorl %edx, %edx
        movb OFF_CHAR(%edi), %cl

        pushl %edx
        call mp1_poke
        popl %edx

        movl $0, %eax
        jmp SYNC_END


SYNC_FAILED:

        movl $-1, %eax

SYNC_END:

        popl %esi
        popl %edi
        popl %ebx
 
        leave
	ret


.end
