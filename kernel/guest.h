
#ifndef GUEST_H
#define GUEST_H

#include "types.h"

#define GUEST_RAM_PAGES 4
#define GUEST_BASE 0x40000000UL

struct guest {
  uint64 gpa_base;
  uint64 ram_size;
  void *ram_pages[GUEST_RAM_PAGES];
};

int guest_ram_init(struct guest *g);
void guest_ram_free(struct guest *g);

#endif
