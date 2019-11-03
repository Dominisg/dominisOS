.global gdt_flush
.global idt_flush
                        # gdt_flush sets gdt address, and size of it. 
gdt_flush:
    movl 4(%esp), %eax
    lgdt (%eax)

    movw  $0x10, %ax     # 0x10 is the offset in the GDT to our data segment
    movw %ax, %ds       # Load all data segment selectors
    movw  %ax, %es
    movw  %ax, %fs
    movw  %ax, %gs
    movw  %ax, %ss
                        # Reload cs segment    
    jmp $0x08,$flush2   # 0x08 is the offset to our code segment: Far jump! 
flush2:
    ret                 # Returns back to the C code!


idt_flush:
    movl 4(%esp), %eax   # Get the pointer to the IDT, passed as a parameter.
    lidt (%eax)         # Load the IDT pointer.
    ret