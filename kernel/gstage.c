
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "guest.h"
#include "gstage.h"

// PTE flags
#define GS_V (1UL << 0)
#define GS_R (1UL << 1)
#define GS_W (1UL << 2)
#define GS_X (1UL << 3)
#define GS_U (1UL << 4)
#define GS_A (1UL << 6)
#define GS_D (1UL << 7)

// Sv39x4 root: 2048 entries = 16 KiB
static uint64 gstage_root[2048]
    __attribute__((aligned(16384)));

// Tra ve dia chi bang trang goc
uint64
gstage_root_pa(void)
{
    return (uint64)gstage_root;
}

// Tim PTE ung voi mot GPA
static uint64 *
gstage_walk(uint64 gpa, int alloc)
{
    if(gpa >= (1ULL << 41))
        return 0;

    uint64 *pt = gstage_root;

    for(int level = 2; level > 0; level--)
    {
        uint64 mask = (level == 2) ? 0x7ff : 0x1ff;
        uint64 index = (gpa >> (12 + 9 * level)) & mask;

        uint64 *pte = &pt[index];

        if(*pte & GS_V)
        {
            // Khong ho tro leaf o tang trung gian
            if(*pte & (GS_R | GS_W | GS_X))
                return 0;

            pt = (uint64 *)((*pte >> 10) << 12);
        }
        else
        {
            if(!alloc)
                return 0;

            void *newpt = kalloc();
            if(newpt == 0)
                return 0;

            memset(newpt, 0, PGSIZE);

            *pte = (((uint64)newpt >> 12) << 10) | GS_V;
            pt = (uint64 *)newpt;
        }
    }

    return &pt[(gpa >> 12) & 0x1ff];
}

// Anh xa mot trang GPA sang HostPA
static int
gstage_map_one(uint64 gpa, uint64 hpa, uint64 perm)
{
    if((gpa | hpa) & (PGSIZE - 1))
        return -1;

    uint64 *pte = gstage_walk(gpa, 1);

    if(pte == 0 || (*pte & GS_V))
        return -1;

    uint64 flags = GS_V | GS_U | GS_A | GS_D | perm;

    *pte = ((hpa >> 12) << 10) | flags;

    return 0;
}

// Tao bang trang cho 4 trang RAM Guest
int
gstage_build(struct guest *g)
{
    if(g == 0)
        return -1;

    memset(gstage_root, 0, sizeof(gstage_root));

    uint64 root_pa = gstage_root_pa();

    printk("[W4-GSTAGE] Root PA = 0x%lx\n", root_pa);
    printk("[W4-GSTAGE] Root size = %ld bytes\n",
           (uint64)sizeof(gstage_root));

    if(root_pa & 0x3fff)
        return -1;

    for(int i = 0; i < GUEST_RAM_PAGES; i++)
    {
        if(g->ram_pages[i] == 0)
            return -1;

        uint64 gpa = g->gpa_base + (uint64)i * PGSIZE;
        uint64 hpa = (uint64)g->ram_pages[i];

        uint64 perm;

        if(i == 0)
            perm = GS_R | GS_X;  // Code: Read + Execute
        else
            perm = GS_R | GS_W;  // Data/Stack: Read + Write

        if(gstage_map_one(gpa, hpa, perm) < 0)
            return -1;

        printk("[W5-RWX] Page %d: R=%d W=%d X=%d\\n",
               i,
               (perm & GS_R) != 0,
               (perm & GS_W) != 0,
               (perm & GS_X) != 0);

        printk("[W4-GSTAGE] Mapped page %d\n", i);
    }

    return 0;
}

// Kiem tra lai cac PTE vua tao
int
gstage_check(struct guest *g)
{
    if(g == 0)
        return -1;

    for(int i = 0; i < GUEST_RAM_PAGES; i++)
    {
        uint64 gpa = g->gpa_base + (uint64)i * PGSIZE;
        uint64 *pte = gstage_walk(gpa, 0);

        if(pte == 0 || !(*pte & GS_V))
            return -1;

        uint64 expected_perm;

        if(i == 0)
            expected_perm = GS_R | GS_X;
        else
            expected_perm = GS_R | GS_W;

        if((*pte & (GS_R | GS_W | GS_X)) != expected_perm)
            return -1;

        if((*pte & GS_U) == 0)
            return -1;

        uint64 actual_hpa = (*pte >> 10) << 12;
        uint64 expected_hpa = (uint64)g->ram_pages[i];

        if(actual_hpa != expected_hpa)
            return -1;

        printk("[W4-GSTAGE] PASS page %d: GPA=0x%lx HPA=0x%lx\n",
               i, gpa, actual_hpa);
    }

    printk("[W4-GSTAGE] All mappings verified\\n");
    printk("[W5-RWX] TEST PASS: R/W/X permissions verified\\n");

    return 0;
}
