
    .section .text
    .balign 4

    .globl guest_enter
    .globl guest_trapvec

# a0 = Guest entry GPA
# a1 = Guest stack GPA
# a2 = Host VM-exit info pointer
guest_enter:
    # Luu cac thanh ghi can bao toan cua Host
    addi sp, sp, -128
    sd ra, 0(sp)
    sd s0, 8(sp)
    sd s1, 16(sp)
    sd s2, 24(sp)
    sd s3, 32(sp)
    sd s4, 40(sp)
    sd s5, 48(sp)
    sd s6, 56(sp)
    sd s7, 64(sp)
    sd s8, 72(sp)
    sd s9, 80(sp)
    sd s10, 88(sp)
    sd s11, 96(sp)

    # Luu con tro den thong tin VM Exit
    sd a2, 104(sp)

    # Luu Host stack de khoi phuc khi VM Exit
    csrw sscratch, sp

    # Chon dia chi bat dau Guest
    csrw sepc, a0

    # Chuyen sang Guest stack
    mv sp, a1
    li a0, 0

    # HS -> VS, phu thuoc CSR da cau hinh
    sret

    .balign 4

# Dia chi nay duoc ghi vao stvec
guest_trapvec:
    # Lay lai Host stack
    csrrw sp, sscratch, sp

    # Lay con tro VM-exit info tu Host stack
    ld t0, 104(sp)

    # Luu thong tin Trap
    csrr t1, scause
    sd t1, 0(t0)

    csrr t1, sepc
    sd t1, 8(t0)

    csrr t1, stval
    sd t1, 16(t0)

    csrr t1, hstatus
    sd t1, 24(t0)

    csrr t1, htval
    sd t1, 32(t0)

    # Luu a0 cua Guest
    sd a0, 40(t0)

    # Luu Guest stack
    csrr t1, sscratch
    sd t1, 48(t0)

    # Khoi phuc cac thanh ghi Host
    ld ra, 0(sp)
    ld s0, 8(sp)
    ld s1, 16(sp)
    ld s2, 24(sp)
    ld s3, 32(sp)
    ld s4, 40(sp)
    ld s5, 48(sp)
    ld s6, 56(sp)
    ld s7, 64(sp)
    ld s8, 72(sp)
    ld s9, 80(sp)
    ld s10, 88(sp)
    ld s11, 96(sp)

    addi sp, sp, 128
    ret
