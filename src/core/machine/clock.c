#include "type.h"

#include "core/machine/clock.h"

static const core_machine_clock_ratio core_machine_clock_identity = {
    1u, 1u, 0u
};

C_INT core_machine_clock_ratio_is_valid(const core_machine_clock_ratio *ratio)
{
    if (ratio == STD_NULL) return 0;
    if (ratio->numerator == 0u && ratio->denominator == 0u &&
        ratio->reset_phase == 0u) return 1;
    return ratio->numerator != 0u && ratio->denominator != 0u &&
        ratio->reset_phase < ratio->denominator;
}

type_status core_machine_clock_domain_initialize(core_machine_clock_domain *domain,
    const core_machine_clock_ratio *ratio)
{
    const core_machine_clock_ratio *resolved = ratio;

    if (domain == STD_NULL || !core_machine_clock_ratio_is_valid(ratio)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (ratio->numerator == 0u) resolved = &core_machine_clock_identity;
    domain->numerator = resolved->numerator;
    domain->denominator = resolved->denominator;
    domain->reset_phase = resolved->reset_phase;
    core_machine_clock_domain_reset(domain);
    return TYPE_STATUS_OK;
}

C_VOID core_machine_clock_domain_reset(core_machine_clock_domain *domain)
{
    if (domain == STD_NULL) return;
    domain->phase = domain->reset_phase;
    domain->delivered_ticks = 0u;
}

uint64_t core_machine_clock_domain_advance(core_machine_clock_domain *domain,
    uint64_t elapsed_ticks)
{
    uint64_t ticks;
    uint64_t remainder;
    uint64_t converted;

    if (domain == STD_NULL || domain->denominator == 0u) return 0u;
    ticks = (elapsed_ticks / domain->denominator) * domain->numerator;
    remainder = (elapsed_ticks % domain->denominator) * domain->numerator +
        domain->phase;
    converted = ticks + remainder / domain->denominator;
    domain->phase = (uint32_t)(remainder % domain->denominator);
    domain->delivered_ticks += converted;
    return converted;
}
