#include "lib/types/types_interface.h"

#include "app-nxvm/devices/machine.h"

#define CORE_MACHINE_ENTRY_PLAN_FLAGS (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | \
    VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_TF | \
    VCPU_EFLAGS_IF | VCPU_EFLAGS_DF | VCPU_EFLAGS_OF)

static lib_status core_machine_entry_plan_build_cpu(core_machine *machine,
    const core_machine_entry_plan_state *state, t_cpu *out_cpu)
{
    core_machine_cpu_execution_context candidate;

    if (machine == LIB_NULL || state == LIB_NULL || out_cpu == LIB_NULL ||
        (state->eflags & ~CORE_MACHINE_ENTRY_PLAN_FLAGS) != 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_cpu = machine->executor_cpu;
    candidate = machine->executor_cpu_execution;
    candidate.cpu = out_cpu;
    if (core_machine_cpu_execution_load_segment(&candidate, &out_cpu->data.cs,
            state->cs) || core_machine_cpu_execution_load_segment(&candidate,
            &out_cpu->data.ds, state->ds) ||
        core_machine_cpu_execution_load_segment(&candidate, &out_cpu->data.es,
            state->es) || core_machine_cpu_execution_load_segment(&candidate,
            &out_cpu->data.ss, state->ss)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    out_cpu->data.eip = state->ip;
    out_cpu->data.esp = state->sp;
    out_cpu->data.eax = state->eax;
    out_cpu->data.ebx = state->ebx;
    out_cpu->data.ecx = state->ecx;
    out_cpu->data.edx = state->edx;
    out_cpu->data.esi = state->esi;
    out_cpu->data.edi = state->edi;
    out_cpu->data.ebp = state->ebp;
    out_cpu->data.eflags = state->eflags | 0x00000002u;
    out_cpu->data.flagHalt = LIB_FALSE;
    return LIB_STATUS_OK;
}

lib_status core_machine_apply_entry_plan(core_machine *machine,
    const core_machine_entry_plan *plan)
{
    t_cpu candidate;
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
    if (plan->entry_physical != expected_physical ||
        core_machine_entry_plan_build_cpu(machine, &plan->state, &candidate) !=
            LIB_STATUS_OK || core_machine_memory_query(machine,
            plan->entry_physical, 1u, CORE_MACHINE_MEMORY_ACCESS_READ, &route) !=
            LIB_STATUS_OK || route != plan->entry_route) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0u; index < plan->preload_count; ++index) {
        const core_machine_entry_plan_preload *preload = &plan->preloads[index];
        lib_size prior;

        if (preload->bytes == LIB_NULL || preload->byte_count == 0u ||
            core_machine_memory_query(machine, preload->physical,
                preload->byte_count, CORE_MACHINE_MEMORY_ACCESS_WRITE, &route) !=
                LIB_STATUS_OK || route != CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM) {
            return LIB_STATUS_INVALID_ARGUMENT;
        }
        for (prior = 0u; prior < index; ++prior) {
            const core_machine_entry_plan_preload *other = &plan->preloads[prior];
            lib_u64 preload_end = (lib_u64)preload->physical + preload->byte_count;
            lib_u64 other_end = (lib_u64)other->physical + other->byte_count;

            if ((lib_u64)preload->physical < other_end &&
                (lib_u64)other->physical < preload_end) {
                return LIB_STATUS_INVALID_ARGUMENT;
            }
        }
    }
    machine->executor_cpu = candidate;
    for (index = 0u; index < plan->preload_count; ++index) {
        const core_machine_entry_plan_preload *preload = &plan->preloads[index];

        status = core_machine_memory_write(machine, preload->physical,
            preload->bytes, preload->byte_count);
        if (status != LIB_STATUS_OK) return status;
    }
    machine->entry_plan_applied = LIB_TRUE;
    return LIB_STATUS_OK;
}
