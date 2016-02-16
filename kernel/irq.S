.globl entry
.globl vec
.globl virtual
entry:
    # EFLAGS
    # CS
    # EIP
    push $0
    push $0x7c00
    movl 8(%esp), %edi
    pushl %edi
    movl 8(%esp), %edi
    pushl %edi
    movl 8(%esp), %edi
    pushl %edi
    push %esp
    call vec
    addl $4, %esp
    iret
