#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "guest.h"
#include "gstage.h"
volatile static int started = 0;
static struct guest demo_guest ;
// start() jumps here in supervisor mode on all CPUs.
void
main()
{
  if (cpuid() == 0) {
    consoleinit();
    printkinit();
    printk("\n[H-TEST] Starting H-extension CSR test\n");

    printk("[H-TEST] Reading hstatus...\n");
    uint64 hstatus = r_hstatus();
    printk("[W3-H] hstatus = 0x%lx\n", hstatus);

    printk("\n");
    // printk("My first xv6 kernel modification by le xuan ngoc!\n");
    printk("[H-TEST] Reading hgatp...\n");
    uint64 hgatp = r_hgatp();
    printk("[H-TEST] hgatp = 0x%lx\n", hgatp);

    printk("[H-TEST] CSR test completed successfully\n");
    printk("\n");

    // printk("QEMU RISC-V & xv6 Environment Initialized!\n"); //Cm luồng boot QEMU->OpenSBI->xv6
    kinit();            // physical page allocator
    printk("bat dau khoi chay ram test\n");
    if(guest_ram_init(&demo_guest) < 0 ){
      panic("guest ram phan bo that bai");
    }
    printk("khoi tao test ram o guest thanh cong ");

    printk("\n[W4-GSTAGE] Starting G-stage mapping test\n");

    if(gstage_build(&demo_guest) < 0){
        panic("[W4-GSTAGE] Build failed");
    }

    if(gstage_check(&demo_guest) < 0){
        panic("[W4-GSTAGE] Verification failed");
    }

    printk("\n[W4-HGATP] Starting hgatp configuration test\n");

    // Lay dia chi bang trang goc G-stage
    uint64 root_pa = gstage_root_pa();

    // Kiem tra can chinh 16 KiB
    if(root_pa & (16384UL - 1)){
        panic("[W4-HGATP] Root alignment error");
    }

    // Luu gia tri hgatp ban dau
    uint64 old_hgatp = r_hgatp();

    // MODE = 8 (Sv39x4), VMID = 0
    // PPN = root_pa >> 12
    uint64 new_hgatp =
        (8ULL << 60) |
        ((root_pa >> 12) & ((1ULL << 44) - 1));

    printk("[W4-HGATP] Root PA = 0x%lx\n", root_pa);
    printk("[W4-HGATP] Old hgatp = 0x%lx\n", old_hgatp);
    printk("[W4-HGATP] Expected hgatp = 0x%lx\n", new_hgatp);

    // Ghi thanh ghi
    w_hgatp(new_hgatp);

    // Doc lai de kiem tra
    uint64 readback = r_hgatp();

    // Khoi phuc ngay gia tri cu
    w_hgatp(old_hgatp);

    printk("[W4-HGATP] Readback = 0x%lx\n", readback);
    printk("[W4-HGATP] Restored = 0x%lx\n", r_hgatp());

    // Kiem tra MODE va PPN
    uint64 mode = (readback >> 60) & 0xF;
    uint64 ppn = readback & ((1ULL << 44) - 1);

    if(mode != 8 || ppn != (root_pa >> 12)){
        panic("[W4-HGATP] Configuration failed");
    }

    if(r_hgatp() != old_hgatp){
        panic("[W4-HGATP] Restore failed");
    }

    printk("[W4-HGATP] MODE = %ld\n", mode);
    printk("[W4-HGATP] PPN = 0x%lx\n", ppn);
    printk("[W4-HGATP] TEST PASS\n");


    kvminit();          // create kernel page table
    kvminithart();      // turn on paging

    trapinit();
    trapinithart();

    // THUC HANH 6: CHAY GUEST VS-MODE
    printk("\n[W4-VS] Starting VS-mode test\n");

    if(guest_run(&demo_guest) < 0){
        panic("[W4-VS] Guest test failed");
    }

    printk("[W4-VS] Returned to Host successfully\n");

    procinit();         // process table
    trapinit();         // trap vectors
    trapinithart();     // install kernel trap vector
    plicinit();         // set up interrupt controller
    plicinithart();     // ask PLIC for device interrupts
    binit();            // buffer cache
    iinit();            // inode table
    fileinit();         // file table
    virtio_disk_init(); // emulated hard disk
    userinit();         // first user process
    w2_test_vm();
    __atomic_store_n(&started, 1, __ATOMIC_RELEASE);
  } else {
    while (__atomic_load_n(&started, __ATOMIC_ACQUIRE) == 0)
      ;

    printk("hart %d starting\n", cpuid());
    kvminithart();  // turn on paging
    trapinithart(); // install kernel trap vector
    plicinithart(); // ask PLIC for device interrupts
  }

  scheduler();
}
