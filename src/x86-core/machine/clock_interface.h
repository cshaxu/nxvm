#include "lib/types/types_interface.h"
#ifndef X86_CORE_MACHINE_CLOCK_H
#define X86_CORE_MACHINE_CLOCK_H

#include "x86-core/machine/machine_interface.h"

typedef struct x86_core_machine_clock_domain {
    lib_u32 numerator;
    lib_u32 denominator;
    lib_u32 reset_phase;
    lib_u32 phase;
    lib_u64 delivered_ticks;
} x86_core_machine_clock_domain;

lib_i32 x86_core_machine_clock_ratio_is_valid(const x86_core_machine_clock_ratio *ratio);
lib_status x86_core_machine_clock_domain_initialize(x86_core_machine_clock_domain *domain,
    const x86_core_machine_clock_ratio *ratio);
void x86_core_machine_clock_domain_reset(x86_core_machine_clock_domain *domain);
lib_u64 x86_core_machine_clock_domain_advance(x86_core_machine_clock_domain *domain,
    lib_u64 elapsed_ticks);
lib_status x86_core_machine_clock_domain_source_ticks_until(
    const x86_core_machine_clock_domain *domain, lib_u64 delivered_ticks,
    lib_u64 *out_source_ticks);

#endif
