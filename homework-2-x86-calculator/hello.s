.code16       # Tell the assembler that this is 16-bit code
.globl start  # Make our "start" label public for the linker

start:
  movb $0x00, %ah    # Set Video Mode to:
  movb $0x03, %al    # Text, 80x25 characters
  int $0x10
               ; # Your code can start here
  movw $message, %si  ; # load the offset of our message into %si
 
print_char:
    lodsb   ;    # loads one char (byte) from message into al and increments
    test %al, %al ; # test to see if the byte is 0
 
    jz done   ;  # If true (END OF MSG), jump to done (END OF PROGRAM)
 
    mov $0x0E, %ah ; # else, 0x0E is the BIOS code to print the single character
    int $0x10   ; # print the char located at al
    jmp print_char ; # loop back

done:
    jmp done
    
message:                   # "message" is the address at the start of the
  .ascii "Hello World\r\n" # character buffer

message_len = . - message  # "message_len" is a constant representing the length
                           # of the character buffer
