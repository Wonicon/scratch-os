.globl entry
.globl vec
.globl virtual
entry:
    # EFLAGS
    # CS
    # EIP
    pushl $0   # GS
    pushl $0   # FS
    pushl $0   # DS
    pushl $0   # ES
    pushl $0   # SS
    leal 0x14(%esp), %edi
    pushl %edi
    movl 0x20(%esp), %edi
    pushl %edi # EFLAGS
    pushl $0   # 反正要被改掉
    pushl $0   # 反正要被改掉
    pushl %esp
    call vec
    addl $4, %esp
    movw 0x0013, %ax
    iret
