#include "lib/types/types_interface.h"
#ifndef CORE_MACHINE_CLOCK_H
#define CORE_MACHINE_CLOCK_H

#include "app-nxvm/devices/machine_interface.h"

typedef struct core_machine_clock_domain {
    lib_u32 numerator;
    lib_u32 denominator;
    lib_u32 reset_phase;
    lib_u32 phase;
    lib_u64 delivered_ticks;
} core_machine_clock_domain;

C_INT core_machine_clock_ratio_is_valid(const core_machine_clock_ratio *ratio);
type_status core_machine_clock_domain_initialize(core_machine_clock_domain *domain,
    const core_machine_clock_ratio *ratio);
C_VOID core_machine_clock_domain_reset(core_machine_clock_domain *domain);
lib_u64 core_machine_clock_domain_advance(core_machine_clock_domain *domain,
    lib_u64 elapsed_ticks);
type_status core_machine_clock_domain_source_ticks_until(
    const core_machine_clock_domain *domain, lib_u64 delivered_ticks,
    lib_u64 *out_source_ticks);

#endif
