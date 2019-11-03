 .macro ISR_NOERRCODE num=0  # define a macro, taking one parameter
 .global isr\num
  isr\num:
    cli
    push $0
    push $\num
    jmp isr_common_stub
.endm

.macro ISR_ERRCODE num=0
.global isr\num
  isr\num:
    cli
    push $\num
    jmp isr_common_stub
.endm 

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE 8
ISR_NOERRCODE 9
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_ERRCODE 30
ISR_NOERRCODE 31

isr_common_stub:
  pusha                    # Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax

  movw %ds, %ax             # Lower 16-bits of eax = ds.
  pushl %eax                # save the data segment descriptor

  movw $0x10, %ax          # load the kernel data segment descriptor
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs
  pushl %esp

  call isr_handler
  popl %ebx

  popl %ebx                # reload the original data segment descriptor
  movw %bx, %ds
  movw %bx, %es
  movw %bx, %fs
  movw %bx, %gs

  popa                     # Pops edi,esi,ebp...
  addl $8, %esp              # Cleans up the pushed error code and pushed ISR number
  sti
  iret                     # pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP