
#ifndef GSTAGE_H
#define GSTAGE_H

#include "types.h"

struct guest;

int gstage_build(struct guest *g);
int gstage_check(struct guest *g);
uint64 gstage_root_pa(void);

#endif
