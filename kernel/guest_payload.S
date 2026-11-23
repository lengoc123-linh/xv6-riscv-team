
    .section .rodata
    .balign 4

    .option push
    .option norvc

    .globl guest_payload_start
    .globl guest_payload_end

guest_payload_start:
    li a0, 123
    li t0, 0x40004000
    ld t1, 0(t0)
    ecall

1:
    j 1b

guest_payload_end:
    .option pop
