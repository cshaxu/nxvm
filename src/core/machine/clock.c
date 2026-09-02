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

type_unsigned_64 core_machine_clock_domain_advance(core_machine_clock_domain *domain,
    type_unsigned_64 elapsed_ticks)
{
    type_unsigned_64 ticks;
    type_unsigned_64 remainder;
    type_unsigned_64 converted;

    if (domain == STD_NULL || domain->denominator == 0u) return 0u;
    /* Equal numerator/denominator is an exact identity domain, including a
     * nonzero frozen phase. Avoiding the general division path preserves the
     * same delivered ticks and phase for the overwhelmingly common 1:1
     * controller clocks. */
    if (domain->numerator == domain->denominator) {
        domain->delivered_ticks += elapsed_ticks;
        return elapsed_ticks;
    }
    ticks = (elapsed_ticks / domain->denominator) * domain->numerator;
    remainder = (elapsed_ticks % domain->denominator) * domain->numerator +
        domain->phase;
    converted = ticks + remainder / domain->denominator;
    domain->phase = (type_unsigned_32)(remainder % domain->denominator);
    domain->delivered_ticks += converted;
    return converted;
}

type_status core_machine_clock_domain_source_ticks_until(
    const core_machine_clock_domain *domain, type_unsigned_64 delivered_ticks,
    type_unsigned_64 *out_source_ticks)
{
    type_unsigned_64 required;

    if (domain == STD_NULL || out_source_ticks == STD_NULL ||
        delivered_ticks == 0u || domain->numerator == 0u ||
        domain->denominator == 0u ||
        delivered_ticks > UINT64_MAX / domain->denominator) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    required = delivered_ticks * domain->denominator;
    if (required <= domain->phase) {
        *out_source_ticks = 1u;
        return TYPE_STATUS_OK;
    }
    required -= domain->phase;
    *out_source_ticks = required / domain->numerator;
    if (required % domain->numerator != 0u) ++*out_source_ticks;
    return *out_source_ticks == 0u ? TYPE_STATUS_INVALID_STATE : TYPE_STATUS_OK;
}
