#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

volatile static int started = 0;

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

    printk("QEMU RISC-V & xv6 Environment Initialized!\n"); //Cm luồng boot QEMU->OpenSBI->xv6

    kinit();            // physical page allocator
    kvminit();          // create kernel page table
    kvminithart();      // turn on paging
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
