#ifndef CORE_MACHINE_CLOCK_H
#define CORE_MACHINE_CLOCK_H

#include "core/machine/machine_interface.h"

typedef struct core_machine_clock_domain {
    type_unsigned_32 numerator;
    type_unsigned_32 denominator;
    type_unsigned_32 reset_phase;
    type_unsigned_32 phase;
    type_unsigned_64 delivered_ticks;
} core_machine_clock_domain;

C_INT core_machine_clock_ratio_is_valid(const core_machine_clock_ratio *ratio);
type_status core_machine_clock_domain_initialize(core_machine_clock_domain *domain,
    const core_machine_clock_ratio *ratio);
C_VOID core_machine_clock_domain_reset(core_machine_clock_domain *domain);
type_unsigned_64 core_machine_clock_domain_advance(core_machine_clock_domain *domain,
    type_unsigned_64 elapsed_ticks);
type_status core_machine_clock_domain_source_ticks_until(
    const core_machine_clock_domain *domain, type_unsigned_64 delivered_ticks,
    type_unsigned_64 *out_source_ticks);

#endif
