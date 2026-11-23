#ifndef XV6_VCPU_H
#define XV6_VCPU_H

#include "types.h"

// RISC-V has 32 general-purpose registers.
// Only registers marked in valid_gpr_mask
// have actually been captured.

struct vcpu_state
{
    uint64 gpr[32];

    // Registers that have valid saved values
    uint64 valid_gpr_mask;

    // Guest PC at VM Exit
    uint64 pc;

    // Hypervisor / Guest trap information
    uint64 hstatus;
    uint64 scause;
    uint64 stval;
    uint64 htval;
};

#define VCPU_REG_SP 2
#define VCPU_REG_A0 10

#endif
