#ifndef CORE_MACHINE_CLOCK_H
#define CORE_MACHINE_CLOCK_H

#include "core/machine/machine_interface.h"

typedef struct core_machine_clock_domain {
    uint32_t numerator;
    uint32_t denominator;
    uint32_t reset_phase;
    uint32_t phase;
    uint64_t delivered_ticks;
} core_machine_clock_domain;

C_INT core_machine_clock_ratio_is_valid(const core_machine_clock_ratio *ratio);
type_status core_machine_clock_domain_initialize(core_machine_clock_domain *domain,
    const core_machine_clock_ratio *ratio);
C_VOID core_machine_clock_domain_reset(core_machine_clock_domain *domain);
uint64_t core_machine_clock_domain_advance(core_machine_clock_domain *domain,
    uint64_t elapsed_ticks);

#endif
