
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "guest.h"

// Khoi tao va cap phat RAM cho Guest
int guest_ram_init(struct guest *g)
{
    if(g == 0)
        return -1;

    memset(g, 0, sizeof(*g));

    g->gpa_base = GUEST_BASE;
    g->ram_size = GUEST_RAM_PAGES * PGSIZE;

    printk("[W4-GUEST] Bat dau khoi tao Guest RAM\n");

    for(int i = 0; i < GUEST_RAM_PAGES; i++)
    {
        void *page = kalloc();

        if(page == 0)
        {
            printk("[W4-GUEST] Cap phat RAM that bai\n");
            guest_ram_free(g);
            return -1;
        }

        memset(page, 0, PGSIZE);

        g->ram_pages[i] = page;

        uint64 gpa = g->gpa_base + (uint64)i * PGSIZE;

        printk("[W4-GUEST] Page %d: GPA=0x%lx HostPA=0x%lx\n",
               i, gpa, (uint64)page);
    }

    printk("[W4-GUEST] RAM SIZE = %ld bytes\n",
           g->ram_size);

    printk("[W4-GUEST] Phan bo RAM thanh cong\n");

    return 0;
}

// Giai phong RAM cua Guest
void guest_ram_free(struct guest *g)
{
    if(g == 0)
        return;

    for(int i = 0; i < GUEST_RAM_PAGES; i++)
    {
        if(g->ram_pages[i] != 0)
        {
            kfree(g->ram_pages[i]);
            g->ram_pages[i] = 0;
        }
    }

    g->ram_size = 0;
}
