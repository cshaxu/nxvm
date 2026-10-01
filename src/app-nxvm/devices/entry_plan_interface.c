#include "lib/types/types_interface.h"

#include "app-nxvm/devices/machine.h"


lib_status core_machine_apply_entry_plan(core_machine *machine,
    const core_machine_entry_plan *plan)
{
    core_machine_cpu_prepared_entry *candidate = LIB_NULL;
    core_machine_memory_route route;
    lib_size index;
    lib_u32 expected_physical;
    lib_status status;

    if (machine == LIB_NULL || plan == LIB_NULL ||
        plan->preload_count > CORE_MACHINE_ENTRY_PLAN_PRELOAD_CAPACITY ||
        (plan->preload_count != 0u && plan->preloads == LIB_NULL) ||
        (plan->entry_route != CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM &&
         plan->entry_route != CORE_MACHINE_MEMORY_ROUTE_PROVIDER)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (machine->lifecycle != CORE_MACHINE_STOPPED || machine->entry_plan_applied) {
        return LIB_STATUS_INVALID_STATE;
    }
    expected_physical = ((lib_u32)plan->state.cs << 4) + plan->state.ip;
    if (plan->entry_physical != expected_physical)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = core_machine_cpu_prepare_entry(machine->executor_cpu_execution,
        &plan->state, &candidate);
    if (status != LIB_STATUS_OK) return status;
    if (core_machine_memory_query(machine,
            plan->entry_physical, 1u, CORE_MACHINE_MEMORY_ACCESS_READ, &route) !=
            LIB_STATUS_OK || route != plan->entry_route) {
        goto invalid_plan;
    }
    for (index = 0u; index < plan->preload_count; ++index) {
        const core_machine_entry_plan_preload *preload = &plan->preloads[index];
        lib_size prior;

        if (preload->bytes == LIB_NULL || preload->byte_count == 0u ||
            core_machine_memory_query(machine, preload->physical,
                preload->byte_count, CORE_MACHINE_MEMORY_ACCESS_WRITE, &route) !=
                LIB_STATUS_OK || route != CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM) {
            goto invalid_plan;
        }
        for (prior = 0u; prior < index; ++prior) {
            const core_machine_entry_plan_preload *other = &plan->preloads[prior];
            lib_u64 preload_end = (lib_u64)preload->physical + preload->byte_count;
            lib_u64 other_end = (lib_u64)other->physical + other->byte_count;

            if ((lib_u64)preload->physical < other_end &&
                (lib_u64)other->physical < preload_end) {
                goto invalid_plan;
            }
        }
    }
    core_machine_cpu_finish_entry(candidate, LIB_TRUE);
    for (index = 0u; index < plan->preload_count; ++index) {
        const core_machine_entry_plan_preload *preload = &plan->preloads[index];

        status = core_machine_memory_write(machine, preload->physical,
            preload->bytes, preload->byte_count);
        if (status != LIB_STATUS_OK) return status;
    }
    machine->entry_plan_applied = LIB_TRUE;
    return LIB_STATUS_OK;

invalid_plan:
    core_machine_cpu_finish_entry(candidate, LIB_FALSE);
    return LIB_STATUS_INVALID_ARGUMENT;
}
