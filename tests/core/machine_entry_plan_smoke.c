#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/cpu.h"

static C_INT prepare_machine(core_machine **out_machine)
{
    static const type_unsigned_8 rom[] = { 0xf4u };
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    core_machine *machine = STD_NULL;

    if (out_machine == STD_NULL || core_machine_create(&config, &machine) !=
            TYPE_STATUS_OK || core_machine_register_immutable_rom_mapping(machine,
            0x1000u, rom, sizeof(rom)) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK) {
        core_machine_destroy(machine);
        return 1;
    }
    *out_machine = machine;
    return 0;
}

static core_machine_entry_plan make_plan(type_unsigned_16 cs, type_unsigned_16 ip,
    type_unsigned_32 physical, core_machine_memory_route route,
    const core_machine_entry_plan_preload *preloads, STD_SIZE_T preload_count)
{
    core_machine_entry_plan plan;

    STD_MEMSET(&plan, 0, sizeof(plan));
    plan.state.cs = cs;
    plan.state.ds = 0u;
    plan.state.es = 0u;
    plan.state.ss = 0u;
    plan.state.ip = ip;
    plan.state.sp = 0x1000u;
    plan.state.eflags = VCPU_EFLAGS_IF;
    plan.entry_physical = physical;
    plan.entry_route = route;
    plan.preloads = preloads;
    plan.preload_count = preload_count;
    return plan;
}

C_INT main(C_VOID)
{
    static const type_unsigned_8 halt[] = { 0xf4u };
    core_machine *machine = STD_NULL;
    core_machine_cpu_state state;
    core_machine_run_result result;
    const core_machine_run_budget budget = { 8u, 0u };
    core_machine_entry_plan_preload invalid_preloads[2] = {
        { 0x0200u, halt, sizeof(halt) }, { 0x1000u, halt, sizeof(halt) }
    };
    core_machine_entry_plan_preload overlapping_preloads[2] = {
        { 0x0200u, halt, sizeof(halt) }, { 0x0200u, halt, sizeof(halt) }
    };
    core_machine_entry_plan plan;
    type_unsigned_8 observed = 0xffu;
    C_INT failed = prepare_machine(&machine);

    if (!failed) {
        failed |= core_machine_get_cpu_state(machine, &state) != TYPE_STATUS_OK ||
            state.cs != 0xf000u || state.eip != 0x0000fff0u;
        plan = make_plan(0u, 0x1000u, 0x1000u,
            CORE_MACHINE_MEMORY_ROUTE_PROVIDER, STD_NULL, 0u);
        failed |= core_machine_apply_entry_plan(machine, &plan) != TYPE_STATUS_OK;
        failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= core_machine_apply_entry_plan(machine, &plan) != TYPE_STATUS_INVALID_STATE;
        failed |= core_machine_reset(machine) != TYPE_STATUS_OK;

        plan = make_plan(0u, 0x1000u, 0x1000u,
            CORE_MACHINE_MEMORY_ROUTE_PROVIDER, invalid_preloads, 2u);
        failed |= core_machine_apply_entry_plan(machine, &plan) != TYPE_STATUS_INVALID_ARGUMENT;
        failed |= core_machine_memory_read(machine, 0x0200u, &observed,
            sizeof(observed)) != TYPE_STATUS_OK || observed != 0u;
        failed |= core_machine_get_cpu_state(machine, &state) != TYPE_STATUS_OK ||
            state.cs != 0xf000u || state.eip != 0x0000fff0u;

        plan = make_plan(0u, 0x0200u, 0x0200u,
            CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM, overlapping_preloads, 2u);
        failed |= core_machine_apply_entry_plan(machine, &plan) != TYPE_STATUS_INVALID_ARGUMENT;
        failed |= core_machine_memory_read(machine, 0x0200u, &observed,
            sizeof(observed)) != TYPE_STATUS_OK || observed != 0u;
        failed |= core_machine_get_cpu_state(machine, &state) != TYPE_STATUS_OK ||
            state.cs != 0xf000u || state.eip != 0x0000fff0u;

        plan = make_plan(0u, 0x0200u, 0x0200u,
            CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM, invalid_preloads, 1u);
        failed |= core_machine_apply_entry_plan(machine, &plan) != TYPE_STATUS_OK;
        failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
        failed |= core_machine_get_cpu_state(machine, &state) != TYPE_STATUS_OK ||
            state.cs != 0xf000u || state.eip != 0x0000fff0u;
    }
    core_machine_destroy(machine);
    if (failed) return 1;
    puts("M5:T246:S2:ENTRY-PLAN:OK");
    return 0;
}
