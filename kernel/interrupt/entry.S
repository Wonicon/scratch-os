.globl entry
.globl irq_handle

# 暂时是通用的中断门入口
entry:
  cli
  # EFLAGS
  # CS
  # EIP
  pushl %esp
  call irq_handle
  addl $4, %esp
  sti
  iret
