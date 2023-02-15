.code16       # Tell the assembler that this is 16-bit code
.globl start  # Make our "start" label public for the linker

start:
  mov $0x00, %ah    # Set Video Mode to:
  mov $0x03, %al    # Text, 80x25 characters
  int $0x10
  # Your code can start here
    mov $prompt1, %si	# load the offset of our message into %si
    call print_msg      # asking for first input
    call get_input      # getting val for input 1
    push %dx            # save val to stack for now
    
    mov $prompt2, %si	# load the offset of our message into %si
    call print_msg      # asking for second input
    call get_input      # getting val for input 2

    mov $Operator, %si  # load the offset of our message into %si
    call print_msg      # asking for operator

get_OP:
    movb $0x00, %ah       # read character
    int $0x16             # keyboard mode
    
    movb $0x0E, %ah       # display character
    int $0x10             # see screen of display

    cmp $0x2B, %al        # check addition, '+'
    je addition

    cmp $0x2D, %al        # check subtraction, '-'
    je subtraction

    cmp $0x2A, %al        # check multiplication, '*'
    je multiplication

    cmp $0x2F, %al       # check division, '/'
    je division

    cmp $0x71, %al      # check 'q'
    je done             # DONE
    call failed         # None of the above, error msg

addition:
    call print_result   # printing "Result: "
    pop %bx             # pop first input val to bx
    add %bx, %dx        # add both inputs
    mov %dx, %ax        # move val to ax
    jmp get_result

subtraction:
    call print_result   # printing "Result: "
    pop %bx             # pop first input val to bx
    sub %dx, %bx        # add both inputs
    jo failed           # overflow error
    mov %bx, %ax        # move val to ax
    jmp get_result

multiplication:
    call print_result   # printing "Result: "
    pop %bx             # pop first input val to bx
    mov %bx, %ax        # move val in bx to ax
    imul %dx            # multiply ax with dx, product stored in ax
    jo failed           # overflow error
    jmp get_result

division:
    call print_result   # printing "Result: "
    pop %ax             # get ax val from stack
    mov %dx, %bx        # mov val in dx to bx
    
    cmp $0x00, %bx      # zero division check
    je zero_case        # if true, ERROR

    cmp $0x8000, %ax    # check if numerator is neg
    ja neg_numerator    # if it is, go to fix

    cmp $0x8000, %bx    # check if denominator is neg
    ja neg_denominator  # GO TO FIX cause overflow

    dividing:
        CDQ                 # sign-extended dividend
        idiv %bx            # divide ax by bx, quotient stored in ax
        jmp get_result

get_result:
    xor %cx, %cx                # digit counter
    mov $10, %bx                # bx = 10 (for dividing later)
    
    cmp $0x8000, %ax            # final val negative
    ja print_neg                # add minus sign show its negative

    stack_digits:
        xor %dx, %dx            # set dx = 0
        div %bx                 # quotient stored in ax and remainder stored in dx
        add $'0', %dl           # make char

        push %dx                # save in stack
        inc %cx                 # increment cx
        test %ax, %ax           # test if end of digits
        jnz stack_digits        # if not, loop

    display:
        cmp $0, %cx             # see if cx = 0, end of digits
        je return               # if cx = 0, done
        pop %ax                 # pop to ax to check al

        movb $0x0E, %ah         # display character
        int $0x10               # see screen of display 
        dec %cx                 # decrement cx
        jmp display             # loop until done

done:
  jmp done    # When we're done, loop infinitely    

failed:
    call new_line       # new line
    call carriage       # carriage return
    mov $error, %si	    # load the offset of our message into %si
    call print_msg     
    jmp done

print_result:
    call new_line
    call carriage
    mov $result, %si	     # load the offset of our message into %si
    call print_msg
    ret

print_neg:
    push %ax            # save ax to stack
    xor %ax, %ax        # clear ax
    movb $0x2D, %al     # printing '-' to indicate negative number
    movb $0x0e,%ah 		 # 0x0E is the BIOS code to print the single character
    int $0x10       	 # call into the BIOS using a software interrupt
    pop %ax             # pop original ax back
    neg %ax             # neg ax
    jmp stack_digits    # printing value in ax


print_msg:
    lodsb           	 # loads a single byte from (%si) into %al and increments %si
    testb %al,%al  	 	 # checks to see if the byte is 0
    jz return 		    # if so, jump out (jz jumps if ZF in EFLAGS is set)
    movb $0x0e,%ah 		 # 0x0E is the BIOS code to print the single character
    int $0x10       	 # call into the BIOS using a software interrupt
    jmp print_msg  	 # go back to the start of the loop

new_line:
    movb $0x0A, %al      # line feed
    int $0x10            # call into the BIOS using a software interrupt
    ret

carriage:
    movb $0x0D, %al      # carriage return
    int $0x10            # call into the BIOS using a software interrupt
    ret

get_input:
    xor %cx, %cx            # set cx = 0, to keep track of stack later
    input_loop:
        mov $0x00, %ah       # keyboard mode
        int $0x16            # int 16h

        movb $0x0E, %ah      # display character
        int $0x10            # see screen of display

        cmp $0x0D, %al       # check for enter key
        je continue          # keep typing number until done

        cmp $0x71, %al      # check q
        je done             # DONE

        cmp $0x2D, %al      # check '-'
        je minus_stack      # add '-' as is to stack

        sub $0x30, %ax      # convert to decimal
        xor %ah, %ah        # set ah to 0, so we only have al
        push %ax            # add al to stack
        inc %cx             # increment cx
        jmp input_loop      # loop

continue:    
    movb $0x0A, %al      # line feed
    int $0x10            # call into the BIOS using a software interrupt
    xor %dx, %dx         # reset dx
    mov $1, %bx          # set bx = 1
    jmp input           # initialize input

minus_stack:
    xor %ah, %ah        # set ah to 0, so we only have al
    push %ax            # add al to stack, ('-' in this case)
    inc %cx             # increment cx
    jmp input_loop      # back to input loop

input:
    pop %ax             # Take latest input from the stack
    cmp $0x2D, %ax      # check if negative symbol
    je neg_num          # if negative number, '-' will be at the end of stack

    push %dx            # save dx to stack
    mul %bx             # multiply value of ax with the value of bx
    pop %dx             # remove from stack

    add %ax, %dx        # add the value of ax to value of dx
    mov %bx, %ax        # set value of bx in ax       

    mov $10, %bx        # set bx = 10
    push %dx            # save the dx value in stack again
    mul %bx             # Multiply bx value by 10
    pop %dx             # pop back dx after multiplying
    mov %ax, %bx        # product stored in ax, move to bx
    dec %cx             # decrement cx 

    cmp $0, %cx         # Check if cx = 0
    jne input           # loop

    cmp $127, %dx        # see if dx > 127 (largest postive 8 bit num)
    jg failed            # invalid 8 bit input (bigger than 127)

    ret

neg_num:
    neg %dx
    cmp $128, %dx        # see if dx < -128 (smallest negative 8 bit num)
    jg failed            # invalid 8 bit input (less than -128)
    ret

neg_numerator:
    neg %ax             # neg ax to make it into a "negative"
    cmp $0x8000, %bx    # check if bx is negative
    ja neg_bx           # go to fix if it is
    call print_minus    # print '-'
    jmp dividing        # back to dividing

neg_denominator:
    neg %bx             # neg bx
    call print_minus    # print '-'
    jmp dividing        # back to dividing

neg_bx:
    neg %bx             # neg bx
    cmp %ax, %bx        # cmp to which is greater
    jb dividing
    call print_minus
    jmp dividing

print_minus:
    push %ax            # save ax to stack
    xor %ax, %ax        # clear ax
    movb $0x2D, %al     # printing '-' to indicate negative number
    movb $0x0e,%ah 		 # 0x0E is the BIOS code to print the single character
    int $0x10       	 # call into the BIOS using a software interrupt
    pop %ax             # pop original ax back
    ret
    
zero_case:
    jmp failed
    
return:
    ret

prompt1:
    .string   "First: \0"

prompt2:
    .string    "Second: \0"

Operator:
    .string    "OP: \0"

result:
    .string    "Result: \0"

error:
	.string "Error! \n \r"