
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "guest.h"
#include "gstage.h"

// CSR bits
#define TEST_SIE   (1UL << 1)
#define TEST_SPIE  (1UL << 5)
#define TEST_SPP   (1UL << 8)
#define TEST_SPV   (1UL << 7)

// Thu tu cac truong phai trung voi guest_enter.S
struct vmexit_info {
    uint64 scause;
    uint64 sepc;
    uint64 stval;
    uint64 hstatus;
    uint64 htval;
    uint64 guest_a0;
    uint64 guest_sp;
};

extern char guest_payload_start[];
extern char guest_payload_end[];

extern void guest_trapvec(void);
extern void guest_enter(uint64 guest_pc,
                        uint64 guest_sp,
                        struct vmexit_info *info);

static inline uint64
read_stvec(void)
{
    uint64 x;
    asm volatile("csrr %0, stvec" : "=r"(x));
    return x;
}

static inline uint64
read_sscratch(void)
{
    uint64 x;
    asm volatile("csrr %0, sscratch" : "=r"(x));
    return x;
}

static inline void
write_sscratch(uint64 x)
{
    asm volatile("csrw sscratch, %0"
                 :
                 : "r"(x)
                 : "memory");
}

static inline uint64
read_vsatp(void)
{
    uint64 x;
    asm volatile("csrr %0, 0x280" : "=r"(x));
    return x;
}

static inline void
write_vsatp(uint64 x)
{
    asm volatile("csrw 0x280, %0"
                 :
                 : "r"(x)
                 : "memory");
}

static inline uint64
read_hedeleg(void)
{
    uint64 x;
    asm volatile("csrr %0, 0x602" : "=r"(x));
    return x;
}

static inline void
write_hedeleg(uint64 x)
{
    asm volatile("csrw 0x602, %0"
                 :
                 : "r"(x)
                 : "memory");
}

// Ma may cho: hfence.gvma x0, x0
// Dung .word vi toolchain dang build -march=rv64gc
static inline void
fence_guest(void)
{
    asm volatile(".word 0x62000073"
                 :
                 :
                 : "memory");
}

int
guest_run(struct guest *g)
{
    if(g == 0 || g->ram_pages[0] == 0)
        return -1;

    // Kich thuoc chuong trinh Guest
    uint64 code_size =
        (uint64)guest_payload_end -
        (uint64)guest_payload_start;

    if(code_size == 0 || code_size > PGSIZE)
        return -1;

    // Dia chi bang trang goc Sv39x4
    uint64 root_pa = gstage_root_pa();

    if(root_pa & 0x3fff)
        return -1;

    // Nap ma Guest vao trang RAM dau tien
    memmove(g->ram_pages[0],
            guest_payload_start,
            code_size);

    // Dong bo instruction cache
    asm volatile("fence.i" ::: "memory");

    struct vmexit_info info;
    memset(&info, 0, sizeof(info));

    // Luu trang thai Host
    uint64 old_sstatus = r_sstatus();
    uint64 old_sie = r_sie();
    uint64 old_sepc = r_sepc();
    uint64 old_stvec = read_stvec();
    uint64 old_sscratch = read_sscratch();
    uint64 old_hstatus = r_hstatus();
    uint64 old_hgatp = r_hgatp();
    uint64 old_vsatp = read_vsatp();
    uint64 old_hedeleg = read_hedeleg();

    uint64 new_hgatp =
        (8ULL << 60) | (root_pa >> 12);

    printk("[W4-VS] Guest code size = %ld\n",
           code_size);

    printk("[W4-VS] Guest entry GPA = 0x%lx\n",
           g->gpa_base);

    printk("[W4-VS] hgatp = 0x%lx\n",
           new_hgatp);

    // Tam tat cac ngat HS trong bai test
    w_sie(0);
    w_sstatus(old_sstatus & ~TEST_SIE);

    // Khong delegate VS-ECALL vao Guest
    write_hedeleg(old_hedeleg & ~(1ULL << 10));

    // VS-stage = Bare
    write_vsatp(0);

    // Bat G-stage Sv39x4
    w_hgatp(new_hgatp);
    fence_guest();

    // Chon Trap Handler rieng
    w_stvec((uint64)guest_trapvec);

    // Cho phep SRET chuyen sang VS-mode
    w_hstatus(old_hstatus | TEST_SPV);

    // SPP = 1, tat ngat cho bai kiem thu
    w_sstatus((old_sstatus | TEST_SPP) &
              ~(TEST_SIE | TEST_SPIE));

    // Chuyen sang Guest va cho VM Exit
    guest_enter(g->gpa_base,
                g->gpa_base + g->ram_size,
                &info);

    // Da quay ve HS-mode
    // Khoi phuc cac CSR da thay doi
    w_stvec(old_stvec);
    w_hgatp(old_hgatp);
    fence_guest();

    write_vsatp(old_vsatp);
    write_hedeleg(old_hedeleg);
    write_sscratch(old_sscratch);

    w_sepc(old_sepc);
    w_hstatus(old_hstatus);
    w_sie(old_sie);
    w_sstatus(old_sstatus);

    // In thong tin VM Exit
    printk("[W4-VS] VM Exit scause = %ld\n",
           info.scause);

    printk("[W4-VS] VM Exit sepc = 0x%lx\n",
           info.sepc);

    printk("[W4-VS] hstatus at exit = 0x%lx\n",
           info.hstatus);

    printk("[W4-VS] Guest a0 = %ld\n",
           info.guest_a0);

    printk("[W4-VS] stval=0x%lx htval=0x%lx\n",
           info.stval, info.htval);

    // Kiem chung Guest VS-ECALL
    if(info.scause == 10 &&
       (info.hstatus & TEST_SPV) &&
       info.guest_a0 == 123)
    {
        printk("[W4-VS] TEST PASS: Guest executed in VS-mode\n");
        return 0;
    }

    printk("[W4-VS] TEST FAIL\n");
    return -1;
}
