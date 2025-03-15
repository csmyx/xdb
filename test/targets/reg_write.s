.global main

.section .data
hex_format_4: .asciz "%#010x"
hex_format_8: .asciz "%#018llx"
float_format: .asciz "%.2f"
long_float_format: .asciz "%.2Lf"

.section .text

  # call kill(pid, 5)
  .macro trap
    movq $62, %rax
    movq %r12, %rdi
    movq $5, %rsi
    syscall
  .endm

  main:
    push %rbp
    movq %rsp, %rbp

    # save pid to %r12
    movq $39, %rax
    syscall
    movq %rax, %r12

    trap

    # print the value of %rsi
    leaq hex_format_4(%rip), %rdi
    movq $0, %rax
    call printf@plt
    movq $0, %rdi
    call fflush@plt

    trap

    # print the value of %mm0
    movq %mm0, %rsi
    leaq hex_format_8(%rip), %rdi
    movq $0, %rax
    call printf@plt
    movq $0, %rdi
    call fflush@plt

    trap

    # print the value of %xmm0
    leaq float_format(%rip), %rdi
    # indicate there is a vector argument
    movq $1, %rax
    call printf@plt
    movq $0, %rdi
    call fflush@plt

    trap

    # print the value of %st0
    subq $16, %rsp
    fstpt (%rsp)
    leaq long_float_format(%rip), %rdi
    movq $0, %rax
    call printf@plt
    movq $0, %rdi
    call fflush@plt
    addq $16, %rsp

    trap

    popq %rbp
    movq $0, %rax
    ret