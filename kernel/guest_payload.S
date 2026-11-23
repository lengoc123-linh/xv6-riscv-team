
    .section .rodata
    .balign 4

    .option push
    .option norvc

    .globl guest_payload_start
    .globl guest_payload_end

guest_payload_start:
    li a0, 123
    ecall

1:
    j 1b

guest_payload_end:
    .option pop
