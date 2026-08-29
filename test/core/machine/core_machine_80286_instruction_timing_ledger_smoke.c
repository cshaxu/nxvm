#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define TIMING_80286_RESET_LINEAR 0x00fffff0u
#define TIMING_80286_RESET_PHYSICAL 0x000ffff0u

typedef struct timing_80286_state {
    type_unsigned_32 reads;
    type_unsigned_32 writes;
    type_unsigned_64 advanced_ticks;
} timing_80286_state;

static type_status timing_80286_read(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 *out_value)
{
    timing_80286_state *state = (timing_80286_state *)owner;

    if (state == STD_NULL || out_value == STD_NULL || port != 0x00e0u)
        return TYPE_STATUS_INVALID_ARGUMENT;
    ++state->reads;
    *out_value = 0x5au;
    return TYPE_STATUS_OK;
}

static type_status timing_80286_write(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 value)
{
    timing_80286_state *state = (timing_80286_state *)owner;

    if (state == STD_NULL || port != 0x00e0u || value > 0xffffu)
        return TYPE_STATUS_INVALID_ARGUMENT;
    ++state->writes;
    return TYPE_STATUS_OK;
}

static const core_machine_port_provider timing_80286_ports = {
    timing_80286_read, timing_80286_write
};

static C_VOID timing_80286_reset(C_VOID *opaque)
{
    timing_80286_state *state = (timing_80286_state *)opaque;
    if (state != STD_NULL) state->advanced_ticks = 0u;
}

static C_VOID timing_80286_advance(C_VOID *opaque, type_unsigned_64 ticks)
{
    timing_80286_state *state = (timing_80286_state *)opaque;
    if (state != STD_NULL) state->advanced_ticks += ticks;
}

static const core_machine_execution_provider timing_80286_execution = {
    timing_80286_reset, timing_80286_advance
};

static C_INT timing_80286_prepare(core_machine **out_machine,
    timing_80286_state *state)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286,
        .ticks_per_instruction = 29u,
        .instruction_timing = { 29u, 7u, 31u, 37u, 41u, 43u }
    };
    core_machine *machine = STD_NULL;

    if (out_machine == STD_NULL || state == STD_NULL ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_80286_RESET_LINEAR, TIMING_80286_RESET_PHYSICAL, 16u) !=
            TYPE_STATUS_OK || core_machine_install_port_provider(machine,
            0x00e0u, 0x00e0u, &timing_80286_ports, state) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(machine,
            &timing_80286_execution, state)) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static C_INT timing_80286_load(core_machine *machine,
    const type_unsigned_8 *program, STD_SIZE_T bytes)
{
    return core_machine_reset(machine) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, TIMING_80286_RESET_LINEAR,
            program, bytes) == TYPE_STATUS_OK;
}

static C_INT timing_80286_run(core_machine *machine, timing_80286_state *state,
    type_unsigned_64 instructions, type_unsigned_64 ticks)
{
    const core_machine_run_budget budget = { instructions, 0u };
    core_machine_run_result result;

    return core_machine_run(machine, budget, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET &&
        result.executed == instructions && result.ticks == ticks &&
        result.elapsed_ticks == ticks && state->advanced_ticks == ticks;
}

static C_INT timing_80286_case(const type_unsigned_8 *program, STD_SIZE_T bytes,
    type_unsigned_64 ticks)
{
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state) ||
        !timing_80286_load(machine, program, bytes) ||
        !timing_80286_run(machine, &state, 1u, ticks);

    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_xlat(C_VOID)
{
    static const type_unsigned_8 program[] = { 0xd7u };
    static const type_unsigned_8 value[] = { 0x5au };
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state) ||
        !timing_80286_load(machine, program, sizeof(program)) ||
        ((machine->executor_cpu.data.ebx = 0x1000u),
            (machine->executor_cpu.data.eax = 1u), 0) ||
        core_machine_memory_write(machine, 0x1001u, value, sizeof(value)) !=
            TYPE_STATUS_OK || !timing_80286_run(machine, &state, 1u, 5u) ||
        TYPE_MASK_UNSIGNED_8(machine->executor_cpu.data.eax) != value[0] ||
        machine->source_timing_unallocated;

    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_lahf_sahf(C_VOID)
{
    static const type_unsigned_8 lahf[] = { 0x9fu };
    static const type_unsigned_8 sahf[] = { 0x9eu };
    const type_unsigned_32 transferred = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF;
    const type_unsigned_32 preserved = VCPU_EFLAGS_IF | VCPU_EFLAGS_DF |
        VCPU_EFLAGS_OF;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, lahf, sizeof(lahf)) ||
        ((machine->executor_cpu.data.eax = 0x11223344u),
            (machine->executor_cpu.data.eflags = transferred), 0) ||
        !timing_80286_run(machine, &state, 1u, 2u) ||
        machine->executor_cpu.data.eax != 0x1122d744u ||
        machine->executor_cpu.data.eflags != transferred;
    if (!failed) failed |= !timing_80286_load(machine, sahf, sizeof(sahf)) ||
        ((machine->executor_cpu.data.eax = 0x1122d744u),
            (machine->executor_cpu.data.eflags = preserved), 0) ||
        !timing_80286_run(machine, &state, 1u, 2u) ||
        machine->executor_cpu.data.eax != 0x1122d744u ||
        machine->executor_cpu.data.eflags != (preserved | transferred);
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_sreg_store(C_VOID)
{
    static const type_unsigned_8 store_ds_ax[] = { 0x8cu, 0xd8u };
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, store_ds_ax,
        sizeof(store_ds_ax)) || ((machine->executor_cpu.data.eax =
        0xaabb0000u), (machine->executor_cpu.data.ds.selector = 0x1357u), 0) ||
        !timing_80286_run(machine, &state, 1u, 2u) ||
        machine->executor_cpu.data.eax != 0xaabb1357u;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_sreg_load(C_VOID)
{
    static const type_unsigned_8 load_es_ax[] = { 0x8eu, 0xc0u };
    static const type_unsigned_8 load_ss_ax[] = { 0x8eu, 0xd0u };
    static const type_unsigned_8 load_ds_ax[] = { 0x8eu, 0xd8u };
    static const type_unsigned_8 load_ds_even[] = { 0x8eu, 0x1eu, 0x00u, 0x10u };
    static const type_unsigned_8 load_ds_odd[] = { 0x8eu, 0x1eu, 0x01u, 0x10u };
    static const type_unsigned_8 load_ds_indexed[] = { 0x8eu, 0x5au, 0x01u };
    const type_unsigned_16 selector = 0x1357u;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, load_es_ax,
        sizeof(load_es_ax)) || ((machine->executor_cpu.data.eax =
        0xaabb1357u), 0) || !timing_80286_run(machine, &state, 1u, 2u) ||
        machine->executor_cpu.data.es.selector != selector ||
        machine->executor_cpu.data.es.base != 0x13570u;
    if (!failed) failed |= !timing_80286_load(machine, load_ss_ax,
        sizeof(load_ss_ax)) || ((machine->executor_cpu.data.eax =
        0xaabb1357u), 0) || !timing_80286_run(machine, &state, 1u, 2u) ||
        machine->executor_cpu.data.ss.selector != selector ||
        machine->executor_cpu.data.ss.base != 0x13570u;
    if (!failed) failed |= !timing_80286_load(machine, load_ds_ax,
        sizeof(load_ds_ax)) || ((machine->executor_cpu.data.eax =
        0xaabb1357u), 0) || !timing_80286_run(machine, &state, 1u, 2u) ||
        machine->executor_cpu.data.ds.selector != selector ||
        machine->executor_cpu.data.ds.base != 0x13570u;
    if (!failed) failed |= !timing_80286_load(machine, load_ds_even,
        sizeof(load_ds_even)) || core_machine_memory_write(machine, 0x1000u,
        &selector, sizeof(selector)) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 5u) ||
        machine->executor_cpu.data.ds.selector != selector;
    if (!failed) failed |= !timing_80286_load(machine, load_ds_odd,
        sizeof(load_ds_odd)) || core_machine_memory_write(machine, 0x1001u,
        &selector, sizeof(selector)) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 7u) ||
        machine->executor_cpu.data.ds.selector != selector;
    if (!failed) failed |= !timing_80286_load(machine, load_ds_indexed,
        sizeof(load_ds_indexed)) || ((machine->executor_cpu.data.bp = 0x1000u),
        (machine->executor_cpu.data.si = 0u), 0) || core_machine_memory_write(
            machine, 0x1001u, &selector, sizeof(selector)) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 8u) ||
        machine->executor_cpu.data.ds.selector != selector;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_boot_protected(core_machine *machine,
    timing_80286_state *state)
{
    static const type_unsigned_8 gdt_pointer[] = { 0x17u, 0u, 0u, 0x03u, 0u, 0u };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0
    };
    static const type_unsigned_8 boot[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u, 0xb8u,0x01u,0u,
        0x0fu,0x01u,0xf0u, 0xb8u,0x10u,0u, 0x8eu,0xd0u,
        0x8eu,0xd8u, 0xeau,0u,0u,0x08u,0u
    };
    static const type_unsigned_8 halt[] = { 0xf4u };
    core_machine_run_result result;

    machine->executor_cpu.data.cr0 = 0u;
    return test_core_machine_fixture_reset_real_mode(machine) &&
        core_machine_memory_write(machine, 0x100u, gdt_pointer,
            sizeof(gdt_pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, 0x300u, gdt, sizeof(gdt)) ==
            TYPE_STATUS_OK && core_machine_memory_write(machine, 0u, boot,
            sizeof(boot)) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, 0x2000u, halt, sizeof(halt)) ==
            TYPE_STATUS_OK && core_machine_run(machine,
            (core_machine_run_budget){64u, 0u}, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT &&
        ((machine->elapsed_ticks = 0u), (state->advanced_ticks = 0u), 1);
}

static C_INT timing_80286_boot_protected_system(core_machine *machine,
    timing_80286_state *state)
{
    static const type_unsigned_8 gdt_pointer[] = { 0x37u, 0u, 0u, 0x03u, 0u, 0u };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x30u,0,0x12u,0,0,
        0xffu,0xffu,0,0x30u,0,0x98u,0,0,
        0x0fu,0,0,0x50u,0,0x82u,0,0,
        0xffu,0xffu,0,0,0,0x89u,0,0
    };
    static const type_unsigned_8 boot[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u, 0xb8u,0x01u,0u,
        0x0fu,0x01u,0xf0u, 0xb8u,0x10u,0u, 0x8eu,0xd0u,
        0x8eu,0xd8u, 0xeau,0u,0u,0x08u,0u
    };
    static const type_unsigned_8 halt[] = { 0xf4u };
    core_machine_run_result result;

    machine->executor_cpu.data.cr0 = 0u;
    return test_core_machine_fixture_reset_real_mode(machine) &&
        core_machine_memory_write(machine, 0x100u, gdt_pointer,
            sizeof(gdt_pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, 0x300u, gdt, sizeof(gdt)) ==
            TYPE_STATUS_OK && core_machine_memory_write(machine, 0u, boot,
            sizeof(boot)) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, 0x2000u, halt, sizeof(halt)) ==
            TYPE_STATUS_OK && core_machine_run(machine,
            (core_machine_run_budget){64u, 0u}, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT &&
        ((machine->elapsed_ticks = 0u), (state->advanced_ticks = 0u), 1);
}

static C_INT timing_80286_sreg_load_protected(C_VOID)
{
    static const type_unsigned_8 direct[] = { 0x8eu, 0xd8u };
    static const type_unsigned_8 memory_even[] = { 0x8eu, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_8 memory_odd[] = { 0x8eu, 0x06u, 0x01u, 0x10u };
    static const type_unsigned_8 memory_indexed[] = { 0x8eu, 0x42u, 0x01u };
    const type_unsigned_16 selector = 0x0010u;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state) ||
        !timing_80286_boot_protected(machine, &state);

    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, direct,
        sizeof(direct)) != TYPE_STATUS_OK || ((machine->executor_cpu.data.eax =
        0xaabb0010u), 0) || ((test_core_machine_fixture_resume_after_halt_at(
            machine, 0u)), !timing_80286_run(machine, &state, 1u, 17u)) ||
        machine->executor_cpu.data.ds.selector != selector;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, memory_even,
        sizeof(memory_even)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
            0x4000u, &selector, sizeof(selector)) != TYPE_STATUS_OK ||
        ((machine->elapsed_ticks = 0u), (state.advanced_ticks = 0u), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 19u)) ||
        machine->executor_cpu.data.es.selector != selector;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, memory_odd,
        sizeof(memory_odd)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
            0x4001u, &selector, sizeof(selector)) != TYPE_STATUS_OK ||
        ((machine->elapsed_ticks = 0u), (state.advanced_ticks = 0u), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 21u)) ||
        machine->executor_cpu.data.es.selector != selector;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, memory_indexed,
        sizeof(memory_indexed)) != TYPE_STATUS_OK || ((machine->executor_cpu.data.bp =
            0x1000u), (machine->executor_cpu.data.si = 0u), 0) ||
        core_machine_memory_write(machine, 0x4001u, &selector, sizeof(selector)) !=
            TYPE_STATUS_OK || ((machine->elapsed_ticks = 0u),
            (state.advanced_ticks = 0u), 0) ||
            ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 22u)) ||
        machine->executor_cpu.data.es.selector != selector;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_les_lds(C_VOID)
{
    static const type_unsigned_8 les_direct[] = { 0xc4u, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_8 lds_direct[] = { 0xc5u, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_8 les_indexed[] = { 0xc4u, 0x42u, 0x01u };
    static const type_unsigned_8 lds_indexed[] = { 0xc5u, 0x42u, 0x01u };
    const type_unsigned_16 real_pointer[] = { 0x3344u, 0x1234u };
    const type_unsigned_16 protected_pointer[] = { 0x3344u, 0x0010u };
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, les_direct,
        sizeof(les_direct)) || core_machine_memory_write(machine, 0x1000u,
        real_pointer, sizeof(real_pointer)) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 7u) ||
        machine->executor_cpu.data.ax != 0x3344u ||
        machine->executor_cpu.data.es.selector != 0x1234u;
    if (!failed) failed |= !timing_80286_load(machine, lds_direct,
        sizeof(lds_direct)) || core_machine_memory_write(machine, 0x1000u,
        real_pointer, sizeof(real_pointer)) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 7u) ||
        machine->executor_cpu.data.ax != 0x3344u ||
        machine->executor_cpu.data.ds.selector != 0x1234u;
    if (!failed) failed |= !timing_80286_load(machine, les_indexed,
        sizeof(les_indexed)) || ((machine->executor_cpu.data.bp = 0x1000u),
        (machine->executor_cpu.data.si = 0u), 0) || core_machine_memory_write(
            machine, 0x1001u, real_pointer, sizeof(real_pointer)) !=
            TYPE_STATUS_OK || !timing_80286_run(machine, &state, 1u, 12u) ||
        machine->executor_cpu.data.es.selector != 0x1234u;
    if (!failed) failed |= !timing_80286_load(machine, lds_indexed,
        sizeof(lds_indexed)) || ((machine->executor_cpu.data.bp = 0x1000u),
        (machine->executor_cpu.data.si = 0u), 0) || core_machine_memory_write(
            machine, 0x1001u, real_pointer, sizeof(real_pointer)) !=
            TYPE_STATUS_OK || !timing_80286_run(machine, &state, 1u, 12u) ||
        machine->executor_cpu.data.ds.selector != 0x1234u;
    if (!failed) failed |= !timing_80286_boot_protected(machine, &state);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        les_direct, sizeof(les_direct)) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0x4000u, protected_pointer,
            sizeof(protected_pointer)) != TYPE_STATUS_OK ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 21u)) ||
        machine->executor_cpu.data.ax != 0x3344u ||
        machine->executor_cpu.data.es.selector != 0x0010u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        lds_direct, sizeof(lds_direct)) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0x4000u, protected_pointer,
            sizeof(protected_pointer)) != TYPE_STATUS_OK ||
        ((machine->elapsed_ticks = 0u), (state.advanced_ticks = 0u), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 21u)) ||
        machine->executor_cpu.data.ax != 0x3344u ||
        machine->executor_cpu.data.ds.selector != 0x0010u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        les_indexed, sizeof(les_indexed)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.bp = 0x1000u),
        (machine->executor_cpu.data.si = 0u), 0) || core_machine_memory_write(
            machine, 0x4001u, protected_pointer, sizeof(protected_pointer)) !=
            TYPE_STATUS_OK || ((machine->elapsed_ticks = 0u),
            (state.advanced_ticks = 0u), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 26u)) ||
        machine->executor_cpu.data.es.selector != 0x0010u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        lds_indexed, sizeof(lds_indexed)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.bp = 0x1000u),
        (machine->executor_cpu.data.si = 0u), 0) || core_machine_memory_write(
            machine, 0x4001u, protected_pointer, sizeof(protected_pointer)) !=
            TYPE_STATUS_OK || ((machine->elapsed_ticks = 0u),
            (state.advanced_ticks = 0u), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 26u)) ||
        machine->executor_cpu.data.ds.selector != 0x0010u;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_sreg_stack(C_VOID)
{
    static const type_unsigned_8 push_ops[] = { 0x06u, 0x0eu, 0x16u, 0x1eu };
    static const type_unsigned_8 pop_ops[] = { 0x07u, 0x17u, 0x1fu };
    static const type_unsigned_16 real_selectors[] = {
        0x1111u, 0x2222u, 0x3333u, 0x4444u
    };
    static const type_unsigned_16 protected_selectors[] = {
        0x0010u, 0x0008u, 0x0010u, 0x0010u
    };
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    type_unsigned_16 image;
    type_unsigned_8 index;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    for (index = 0u; !failed && index < sizeof(push_ops); ++index) {
        failed |= !timing_80286_load(machine, &push_ops[index], 1u) ||
            ((machine->executor_cpu.data.esp = 0x8000u),
            (machine->executor_cpu.data.es.selector = real_selectors[0]),
            (machine->executor_cpu.data.cs.selector = real_selectors[1]),
            (machine->executor_cpu.data.ss.selector = real_selectors[2]),
            (machine->executor_cpu.data.ds.selector = real_selectors[3]), 0) ||
            !timing_80286_run(machine, &state, 1u, 3u) ||
            machine->executor_cpu.data.esp != 0x7ffeu ||
            core_machine_memory_read(machine, 0x7ffeu, &image, sizeof(image)) !=
                TYPE_STATUS_OK || image != real_selectors[index];
    }
    for (index = 0u; !failed && index < sizeof(pop_ops); ++index) {
        const type_unsigned_16 selector = (type_unsigned_16)(0x5555u + index);

        failed |= !timing_80286_load(machine, &pop_ops[index], 1u) ||
            ((machine->executor_cpu.data.esp = 0x8000u), 0) ||
            core_machine_memory_write(machine, 0x8000u, &selector,
                sizeof(selector)) != TYPE_STATUS_OK ||
            !timing_80286_run(machine, &state, 1u, 5u) ||
            machine->executor_cpu.data.esp != 0x8002u ||
            (index == 0u ? machine->executor_cpu.data.es.selector :
            index == 1u ? machine->executor_cpu.data.ss.selector :
            machine->executor_cpu.data.ds.selector) != selector;
    }
    if (!failed) failed |= !timing_80286_boot_protected(machine, &state);
    for (index = 0u; !failed && index < sizeof(push_ops); ++index) {
        failed |= core_machine_memory_write(machine, 0x2000u, &push_ops[index],
            1u) != TYPE_STATUS_OK || ((machine->executor_cpu.data.esp = 0x8000u),
            (machine->executor_cpu.data.es.selector = 0x0010u),
            (machine->elapsed_ticks = 0u), (state.advanced_ticks = 0u), 0) ||
            ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
            !timing_80286_run(machine, &state, 1u, 3u)) ||
            machine->executor_cpu.data.esp != 0x7ffeu ||
            core_machine_memory_read_physical(&machine->executor_memory, 0xaffeu,
                TYPE_REFERENCE_OF(image), sizeof(image)) != TYPE_STATUS_OK ||
            image != protected_selectors[index];
    }
    for (index = 0u; !failed && index < sizeof(pop_ops); ++index) {
        const type_unsigned_16 selector = 0x0010u;

        failed |= core_machine_memory_write(machine, 0x2000u, &pop_ops[index],
            1u) != TYPE_STATUS_OK || ((machine->executor_cpu.data.esp = 0x8000u),
            (machine->elapsed_ticks = 0u), (state.advanced_ticks = 0u), 0) ||
            core_machine_memory_write_physical(&machine->executor_memory, 0xb000u,
                TYPE_REFERENCE_OF(selector), sizeof(selector)) != TYPE_STATUS_OK ||
            ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
            !timing_80286_run(machine, &state, 1u, 20u)) ||
            machine->executor_cpu.data.esp != 0x8002u ||
            (index == 0u ? machine->executor_cpu.data.es.selector :
            index == 1u ? machine->executor_cpu.data.ss.selector :
            machine->executor_cpu.data.ds.selector) != selector;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_bound(C_VOID)
{
    static const type_unsigned_8 direct[] = { 0x62u, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_8 indexed[] = { 0x62u, 0x42u, 0x01u };
    const type_signed_16 bounds[] = { -2, 3 };
    const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, direct, sizeof(direct)) ||
        core_machine_memory_write(machine, 0x1000u, bounds, sizeof(bounds)) !=
            TYPE_STATUS_OK || ((machine->executor_cpu.data.eax = 0xaabb0002u),
            (machine->executor_cpu.data.eflags = flags), 0) ||
        !timing_80286_run(machine, &state, 1u, 13u) ||
        machine->executor_cpu.data.eax != 0xaabb0002u ||
        machine->executor_cpu.data.eflags != flags;
    if (!failed) failed |= !timing_80286_load(machine, indexed, sizeof(indexed)) ||
        ((machine->executor_cpu.data.ebp = 0x1000u),
        (machine->executor_cpu.data.esi = 0u),
        (machine->executor_cpu.data.eax = 0xaabb0002u),
        (machine->executor_cpu.data.eflags = flags), 0) ||
        core_machine_memory_write(machine, 0x1001u, bounds, sizeof(bounds)) !=
            TYPE_STATUS_OK || !timing_80286_run(machine, &state, 1u, 13u) ||
        machine->executor_cpu.data.eax != 0xaabb0002u ||
        machine->executor_cpu.data.eflags != flags;
    if (!failed) failed |= !timing_80286_boot_protected(machine, &state);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, direct,
        sizeof(direct)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
        0x4000u, bounds, sizeof(bounds)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.eax = 0xaabb0002u),
        (machine->executor_cpu.data.eflags = flags), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 13u)) ||
        machine->executor_cpu.data.eax != 0xaabb0002u ||
        machine->executor_cpu.data.eflags != flags;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, indexed,
        sizeof(indexed)) != TYPE_STATUS_OK || ((machine->executor_cpu.data.ebp =
        0x1000u), (machine->executor_cpu.data.esi = 0u),
        (machine->executor_cpu.data.eax = 0xaabb0002u),
        (machine->executor_cpu.data.eflags = flags),
        (machine->elapsed_ticks = 0u), (state.advanced_ticks = 0u), 0) ||
        core_machine_memory_write(machine, 0x4001u, bounds, sizeof(bounds)) !=
            TYPE_STATUS_OK || ((test_core_machine_fixture_resume_after_halt_at(
            machine, 0u)), !timing_80286_run(machine, &state, 1u, 13u)) ||
        machine->executor_cpu.data.eax != 0xaabb0002u ||
        machine->executor_cpu.data.eflags != flags;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_arpl(C_VOID)
{
    static const type_unsigned_8 register_form[] = { 0x63u, 0xc8u };
    static const type_unsigned_8 direct[] = { 0x63u, 0x0eu, 0x00u, 0x10u };
    static const type_unsigned_8 indexed[] = { 0x63u, 0x4au, 0x01u };
    type_unsigned_16 selector;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state) ||
        !timing_80286_boot_protected(machine, &state);

    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        register_form, sizeof(register_form)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.eax = 0x0001u),
        (machine->executor_cpu.data.ecx = 0x0003u), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 10u)) ||
        (machine->executor_cpu.data.eax & 0xffffu) != 0x0003u ||
        (machine->executor_cpu.data.eflags & VCPU_EFLAGS_ZF) == 0u;
    selector = 0x0001u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, direct,
        sizeof(direct)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
        0x4000u, &selector, sizeof(selector)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.ecx = 0x0003u),
        (machine->elapsed_ticks = 0u), (state.advanced_ticks = 0u), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 11u)) ||
        core_machine_memory_read(machine, 0x4000u, &selector, sizeof(selector)) !=
            TYPE_STATUS_OK || selector != 0x0003u ||
        (machine->executor_cpu.data.eflags & VCPU_EFLAGS_ZF) == 0u;
    selector = 0x0003u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, indexed,
        sizeof(indexed)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
        0x4001u, &selector, sizeof(selector)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.ebp = 0x1000u),
        (machine->executor_cpu.data.esi = 0u), (machine->executor_cpu.data.ecx =
        0x0001u), (machine->elapsed_ticks = 0u), (state.advanced_ticks = 0u), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 11u)) ||
        core_machine_memory_read(machine, 0x4001u, &selector, sizeof(selector)) !=
            TYPE_STATUS_OK || selector != 0x0003u ||
        (machine->executor_cpu.data.eflags & VCPU_EFLAGS_ZF) != 0u;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_verr_verw(C_VOID)
{
    static const type_unsigned_8 verr_register[] = { 0x0fu,0x00u,0xe0u };
    static const type_unsigned_8 verw_direct[] = { 0x0fu,0x00u,0x2eu,0x00u,0x10u };
    static const type_unsigned_8 verr_indexed[] = { 0x0fu,0x00u,0x62u,0x01u };
    const type_unsigned_16 selector = 0x0010u;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state) ||
        !timing_80286_boot_protected(machine, &state);

    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        verr_register, sizeof(verr_register)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.eax = selector), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 14u)) ||
        (machine->executor_cpu.data.eflags & VCPU_EFLAGS_ZF) == 0u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        verw_direct, sizeof(verw_direct)) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0x4000u, &selector, sizeof(selector)) !=
            TYPE_STATUS_OK || ((machine->elapsed_ticks = 0u),
            (state.advanced_ticks = 0u), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 16u)) ||
        (machine->executor_cpu.data.eflags & VCPU_EFLAGS_ZF) == 0u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        verr_indexed, sizeof(verr_indexed)) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0x4001u, &selector, sizeof(selector)) !=
            TYPE_STATUS_OK || ((machine->executor_cpu.data.ebp = 0x1000u),
            (machine->executor_cpu.data.esi = 0u), (machine->elapsed_ticks = 0u),
            (state.advanced_ticks = 0u), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 17u)) ||
        (machine->executor_cpu.data.eflags & VCPU_EFLAGS_ZF) == 0u;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_lar(C_VOID)
{
    static const type_unsigned_8 register_form[] = { 0x0fu,0x02u,0xc8u };
    static const type_unsigned_8 direct[] = { 0x0fu,0x02u,0x0eu,0x00u,0x10u };
    static const type_unsigned_8 indexed[] = { 0x0fu,0x02u,0x4au,0x01u };
    const type_unsigned_16 selector = 0x0010u;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state) ||
        !timing_80286_boot_protected(machine, &state);

    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        register_form, sizeof(register_form)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.eax = selector),
        (machine->executor_cpu.data.ecx = 0u), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 14u)) ||
        (machine->executor_cpu.data.ecx & 0xffffu) != 0x9300u ||
        (machine->executor_cpu.data.eflags & VCPU_EFLAGS_ZF) == 0u;
    if (!failed) failed |= ((machine->executor_cpu.data.eax = 0x0018u),
        (machine->executor_cpu.data.ecx = 0x3456u),
        (machine->elapsed_ticks = 0u), (state.advanced_ticks = 0u), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 14u)) ||
        (machine->executor_cpu.data.ecx & 0xffffu) != 0x3456u ||
        (machine->executor_cpu.data.eflags & VCPU_EFLAGS_ZF) != 0u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, direct,
        sizeof(direct)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
        0x4000u, &selector, sizeof(selector)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.ecx = 0u), (machine->elapsed_ticks = 0u),
        (state.advanced_ticks = 0u), 0) || ((test_core_machine_fixture_resume_after_halt_at(
        machine, 0u)), !timing_80286_run(machine, &state, 1u, 16u)) ||
        (machine->executor_cpu.data.ecx & 0xffffu) != 0x9300u ||
        (machine->executor_cpu.data.eflags & VCPU_EFLAGS_ZF) == 0u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, indexed,
        sizeof(indexed)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
        0x4001u, &selector, sizeof(selector)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.ebp = 0x1000u),
        (machine->executor_cpu.data.esi = 0u), (machine->executor_cpu.data.ecx = 0u),
        (machine->elapsed_ticks = 0u), (state.advanced_ticks = 0u), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 17u)) ||
        (machine->executor_cpu.data.ecx & 0xffffu) != 0x9300u ||
        (machine->executor_cpu.data.eflags & VCPU_EFLAGS_ZF) == 0u;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_lsl(C_VOID)
{
    static const type_unsigned_8 register_form[] = { 0x0fu,0x03u,0xc8u };
    static const type_unsigned_8 direct[] = { 0x0fu,0x03u,0x0eu,0x00u,0x10u };
    static const type_unsigned_8 indexed[] = { 0x0fu,0x03u,0x4au,0x01u };
    const type_unsigned_16 selector = 0x0010u;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state) ||
        !timing_80286_boot_protected(machine, &state);

    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        register_form, sizeof(register_form)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.eax = selector),
        (machine->executor_cpu.data.ecx = 0u), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 14u)) ||
        (machine->executor_cpu.data.ecx & 0xffffu) != 0xffffu ||
        (machine->executor_cpu.data.eflags & VCPU_EFLAGS_ZF) == 0u;
    if (!failed) failed |= ((machine->executor_cpu.data.eax = 0x0018u),
        (machine->executor_cpu.data.ecx = 0x3456u),
        (machine->elapsed_ticks = 0u), (state.advanced_ticks = 0u), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 14u)) ||
        (machine->executor_cpu.data.ecx & 0xffffu) != 0x3456u ||
        (machine->executor_cpu.data.eflags & VCPU_EFLAGS_ZF) != 0u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, direct,
        sizeof(direct)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
        0x4000u, &selector, sizeof(selector)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.ecx = 0u), (machine->elapsed_ticks = 0u),
        (state.advanced_ticks = 0u), 0) || ((test_core_machine_fixture_resume_after_halt_at(
        machine, 0u)), !timing_80286_run(machine, &state, 1u, 16u)) ||
        (machine->executor_cpu.data.ecx & 0xffffu) != 0xffffu ||
        (machine->executor_cpu.data.eflags & VCPU_EFLAGS_ZF) == 0u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, indexed,
        sizeof(indexed)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
        0x4001u, &selector, sizeof(selector)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.ebp = 0x1000u),
        (machine->executor_cpu.data.esi = 0u), (machine->executor_cpu.data.ecx = 0u),
        (machine->elapsed_ticks = 0u), (state.advanced_ticks = 0u), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 17u)) ||
        (machine->executor_cpu.data.ecx & 0xffffu) != 0xffffu ||
        (machine->executor_cpu.data.eflags & VCPU_EFLAGS_ZF) == 0u;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_smsw(C_VOID)
{
    static const type_unsigned_8 register_form[] = { 0x0fu,0x01u,0xe0u };
    static const type_unsigned_8 direct[] = { 0x0fu,0x01u,0x26u,0x00u,0x10u };
    static const type_unsigned_8 indexed[] = { 0x0fu,0x01u,0x62u,0x01u };
    type_unsigned_16 msw;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, register_form,
        sizeof(register_form)) || ((machine->executor_cpu.data.cr0 = 0x000cu),
        (machine->executor_cpu.data.eax = 0u), 0) || !timing_80286_run(machine,
        &state, 1u, 2u) || (machine->executor_cpu.data.eax & 0xffffu) != 0x000cu;
    if (!failed) failed |= !timing_80286_load(machine, direct, sizeof(direct)) ||
        ((machine->executor_cpu.data.cr0 = 0x000cu), 0) || !timing_80286_run(machine,
        &state, 1u, 3u) || core_machine_memory_read(machine, 0x1000u, &msw,
        sizeof(msw)) != TYPE_STATUS_OK || msw != 0x000cu;
    if (!failed) failed |= !timing_80286_load(machine, indexed, sizeof(indexed)) ||
        ((machine->executor_cpu.data.cr0 = 0x000cu),
        (machine->executor_cpu.data.ebp = 0x1000u),
        (machine->executor_cpu.data.esi = 0u), 0) || !timing_80286_run(machine,
        &state, 1u, 4u) || core_machine_memory_read(machine, 0x1001u, &msw,
        sizeof(msw)) != TYPE_STATUS_OK || msw != 0x000cu;
    if (!failed) failed |= !timing_80286_boot_protected(machine, &state) ||
        core_machine_memory_write(machine, 0x2000u, register_form,
        sizeof(register_form)) != TYPE_STATUS_OK || ((machine->executor_cpu.data.eax = 0u),
        0) || ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 2u)) ||
        (machine->executor_cpu.data.eax & 0xffffu) != 0x0001u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, direct,
        sizeof(direct)) != TYPE_STATUS_OK || ((machine->elapsed_ticks = 0u),
        (state.advanced_ticks = 0u), 0) || ((test_core_machine_fixture_resume_after_halt_at(
        machine, 0u)), !timing_80286_run(machine, &state, 1u, 3u)) ||
        core_machine_memory_read(machine, 0x4000u, &msw, sizeof(msw)) !=
        TYPE_STATUS_OK || msw != 0x0001u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, indexed,
        sizeof(indexed)) != TYPE_STATUS_OK || ((machine->executor_cpu.data.ebp = 0x1000u),
        (machine->executor_cpu.data.esi = 0u), (machine->elapsed_ticks = 0u),
        (state.advanced_ticks = 0u), 0) || ((test_core_machine_fixture_resume_after_halt_at(
        machine, 0u)), !timing_80286_run(machine, &state, 1u, 4u)) ||
        core_machine_memory_read(machine, 0x4001u, &msw, sizeof(msw)) !=
        TYPE_STATUS_OK || msw != 0x0001u;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_sldt_str(C_VOID)
{
    static const type_unsigned_8 sldt_register[] = { 0x0fu,0x00u,0xc0u };
    static const type_unsigned_8 str_direct[] = { 0x0fu,0x00u,0x0eu,0x00u,0x10u };
    static const type_unsigned_8 sldt_indexed[] = { 0x0fu,0x00u,0x42u,0x01u };
    type_unsigned_16 selector;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state) ||
        !timing_80286_boot_protected(machine, &state);

    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        sldt_register, sizeof(sldt_register)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.ldtr.selector = 0x0010u),
        (machine->executor_cpu.data.eax = 0u), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 2u)) ||
        (machine->executor_cpu.data.eax & 0xffffu) != 0x0010u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        str_direct, sizeof(str_direct)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.tr.selector = 0x0008u),
        (machine->elapsed_ticks = 0u), (state.advanced_ticks = 0u), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 3u)) ||
        core_machine_memory_read(machine, 0x4000u, &selector, sizeof(selector)) !=
        TYPE_STATUS_OK || selector != 0x0008u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        sldt_indexed, sizeof(sldt_indexed)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.ldtr.selector = 0x0010u),
        (machine->executor_cpu.data.ebp = 0x1000u),
        (machine->executor_cpu.data.esi = 0u), (machine->elapsed_ticks = 0u),
        (state.advanced_ticks = 0u), 0) || ((test_core_machine_fixture_resume_after_halt_at(
        machine, 0u)), !timing_80286_run(machine, &state, 1u, 4u)) ||
        core_machine_memory_read(machine, 0x4001u, &selector, sizeof(selector)) !=
        TYPE_STATUS_OK || selector != 0x0010u;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_lmsw(C_VOID)
{
    static const type_unsigned_8 register_form[] = { 0x0fu,0x01u,0xf0u };
    static const type_unsigned_8 direct[] = { 0x0fu,0x01u,0x36u,0x00u,0x10u };
    static const type_unsigned_8 indexed[] = { 0x0fu,0x01u,0x72u,0x01u };
    const type_unsigned_16 protected_msw = 0x0001u;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, register_form,
        sizeof(register_form)) || ((machine->executor_cpu.data.eax = 0u), 0) ||
        !timing_80286_run(machine, &state, 1u, 3u) ||
        (machine->executor_cpu.data.cr0 & 0xffffu) != 0u;
    if (!failed) failed |= !timing_80286_load(machine, direct, sizeof(direct)) ||
        core_machine_memory_write(machine, 0x1000u, &protected_msw,
        sizeof(protected_msw)) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 6u) ||
        (machine->executor_cpu.data.cr0 & 0xffffu) != 0x0001u;
    if (!failed) failed |= !timing_80286_boot_protected(machine, &state) ||
        core_machine_memory_write(machine, 0x2000u, register_form,
        sizeof(register_form)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.eax = protected_msw), 0) ||
        ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
        !timing_80286_run(machine, &state, 1u, 3u)) ||
        (machine->executor_cpu.data.cr0 & 0xffffu) != 0x0001u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, indexed,
        sizeof(indexed)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
        0x4001u, &protected_msw, sizeof(protected_msw)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.ebp = 0x1000u),
        (machine->executor_cpu.data.esi = 0u), (machine->elapsed_ticks = 0u),
        (state.advanced_ticks = 0u), 0) || ((test_core_machine_fixture_resume_after_halt_at(
        machine, 0u)), !timing_80286_run(machine, &state, 1u, 7u)) ||
        (machine->executor_cpu.data.cr0 & 0xffffu) != 0x0001u;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_table_control(C_VOID)
{
    static const type_unsigned_8 sgdt[] = { 0x0fu,0x01u,0x06u,0x00u,0x10u };
    static const type_unsigned_8 sidt[] = { 0x0fu,0x01u,0x0eu,0x00u,0x10u };
    static const type_unsigned_8 lgdt[] = { 0x0fu,0x01u,0x16u,0x00u,0x10u };
    static const type_unsigned_8 lidt[] = { 0x0fu,0x01u,0x1eu,0x00u,0x10u };
    static const type_unsigned_8 clts[] = { 0x0fu,0x06u };
    static const type_unsigned_8 table[] = { 0x57u,0x13u,0x00u,0x34u,0x12u,0u };
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, sgdt, sizeof(sgdt)) ||
        ((machine->executor_cpu.data.gdtr.limit = 0x1357u),
        (machine->executor_cpu.data.gdtr.base = 0x00123400u), 0) ||
        !timing_80286_run(machine, &state, 1u, 11u);
    if (!failed) failed |= !timing_80286_load(machine, sidt, sizeof(sidt)) ||
        ((machine->executor_cpu.data.idtr.limit = 0x1357u),
        (machine->executor_cpu.data.idtr.base = 0x00123400u), 0) ||
        !timing_80286_run(machine, &state, 1u, 12u);
    if (!failed) failed |= !timing_80286_load(machine, lgdt, sizeof(lgdt)) ||
        core_machine_memory_write(machine, 0x1000u, table, sizeof(table)) !=
            TYPE_STATUS_OK || !timing_80286_run(machine, &state, 1u, 11u) ||
        machine->executor_cpu.data.gdtr.limit != 0x1357u ||
        machine->executor_cpu.data.gdtr.base != 0x00123400u;
    if (!failed) failed |= !timing_80286_load(machine, lidt, sizeof(lidt)) ||
        core_machine_memory_write(machine, 0x1000u, table, sizeof(table)) !=
            TYPE_STATUS_OK || !timing_80286_run(machine, &state, 1u, 12u) ||
        machine->executor_cpu.data.idtr.limit != 0x1357u ||
        machine->executor_cpu.data.idtr.base != 0x00123400u;
    if (!failed) failed |= !timing_80286_load(machine, clts, sizeof(clts)) ||
        ((machine->executor_cpu.data.cr0 = VCPU_CR0_TS), 0) ||
        !timing_80286_run(machine, &state, 1u, 2u) ||
        (machine->executor_cpu.data.cr0 & VCPU_CR0_TS) != 0u;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_ldt_task_registers(C_VOID)
{
    static const type_unsigned_8 lldt_register[] = { 0x0fu,0x00u,0xd0u };
    static const type_unsigned_8 lldt_memory[] = { 0x0fu,0x00u,0x16u,0x00u,0x04u };
    static const type_unsigned_8 ltr_register[] = { 0x0fu,0x00u,0xd8u };
    static const type_unsigned_8 ltr_memory[] = { 0x0fu,0x00u,0x1eu,0x00u,0x04u };
    const type_unsigned_16 ldt_selector = 0x0028u;
    const type_unsigned_16 tss_selector = 0x0030u;
    const type_unsigned_8 *forms[] = { lldt_register, lldt_memory,
        ltr_register, ltr_memory };
    const STD_SIZE_T sizes[] = { sizeof(lldt_register), sizeof(lldt_memory),
        sizeof(ltr_register), sizeof(ltr_memory) };
    const type_unsigned_16 selectors[] = { ldt_selector, ldt_selector,
        tss_selector, tss_selector };
    const type_unsigned_64 ticks[] = { 17u, 19u, 17u, 19u };
    STD_SIZE_T index;
    C_INT failed = 0;

    for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
        timing_80286_state state = { 0u, 0u, 0u };
        core_machine *machine = STD_NULL;

        if (!timing_80286_prepare(&machine, &state) ||
            !timing_80286_boot_protected_system(machine, &state)) {
            core_machine_destroy(machine);
            return 1;
        }
        failed |= core_machine_memory_write(machine, 0x2000u, forms[index],
            sizes[index]) != TYPE_STATUS_OK ||
            ((machine->executor_cpu.data.eax = selectors[index]), 0) ||
            (index & 1u && core_machine_memory_write(machine, 0x3400u,
                &selectors[index], sizeof(selectors[index])) != TYPE_STATUS_OK) ||
            ((test_core_machine_fixture_resume_after_halt_at(machine, 0u)),
            !timing_80286_run(machine, &state, 1u, ticks[index])) ||
            (index < 2u ? machine->executor_cpu.data.ldtr.selector !=
                ldt_selector : machine->executor_cpu.data.tr.selector !=
                tss_selector);
        core_machine_destroy(machine);
    }
    return failed;
}

static C_INT timing_80286_fpu_interface_transfer(C_VOID)
{
    static const type_unsigned_8 fninit[] = { 0xdbu,0xe3u };
    static const type_unsigned_8 fwait[] = { 0x9bu };
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, fninit, sizeof(fninit)) ||
        !timing_80286_run(machine, &state, 1u, 1u);
    if (!failed) failed |= !timing_80286_load(machine, fwait, sizeof(fwait)) ||
        !timing_80286_run(machine, &state, 1u, 3u);
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_memory(C_VOID)
{
    static const type_unsigned_8 direct_read[] = { 0x8bu, 0x0eu, 0x00u, 0x10u };
    static const type_unsigned_8 direct_write[] = { 0x89u, 0x0eu, 0x00u, 0x10u };
    static const type_unsigned_8 indexed_odd_read[] = { 0x8bu, 0x4au, 0x01u };
    static const type_unsigned_8 moffs_read[] = { 0xa1u, 0x01u, 0x10u };
    static const type_unsigned_8 moffs_write[] = { 0xa3u, 0x01u, 0x10u };
    static const type_unsigned_8 xlat[] = { 0xd7u };
    static const type_unsigned_8 sreg_store_even[] = { 0x8cu, 0x1eu, 0x00u, 0x10u };
    static const type_unsigned_8 sreg_store_odd[] = { 0x8cu, 0x1eu, 0x01u, 0x10u };
    static const type_unsigned_8 sreg_store_indexed[] = { 0x8cu, 0x5au, 0x01u };
    const type_unsigned_16 value = 0x5aa5u;
    const type_unsigned_16 sreg_value = 0x1357u;
    type_unsigned_16 sreg_read = 0u;
    const type_unsigned_8 xlat_value = 0xa5u;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, direct_read,
        sizeof(direct_read)) || core_machine_memory_write(machine, 0x1000u,
        &value, sizeof(value)) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 5u) ||
        machine->executor_cpu.data.cx != value;
    if (!failed) failed |= !timing_80286_load(machine, direct_write,
        sizeof(direct_write)) || ((machine->executor_cpu.data.cx = value), 0) ||
        !timing_80286_run(machine, &state, 1u, 3u);
    if (!failed) failed |= !timing_80286_load(machine, indexed_odd_read,
        sizeof(indexed_odd_read)) || ((machine->executor_cpu.data.bp = 0x1000u),
        (machine->executor_cpu.data.si = 0u), 0) ||
        core_machine_memory_write(machine, 0x1001u, &value, sizeof(value)) !=
            TYPE_STATUS_OK || !timing_80286_run(machine, &state, 1u, 8u) ||
        machine->executor_cpu.data.cx != value;
    if (!failed) failed |= !timing_80286_load(machine, moffs_read,
        sizeof(moffs_read)) || core_machine_memory_write(machine, 0x1001u,
        &value, sizeof(value)) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 7u);
    if (!failed) failed |= !timing_80286_load(machine, moffs_write,
        sizeof(moffs_write)) || ((machine->executor_cpu.data.ax = value), 0) ||
        !timing_80286_run(machine, &state, 1u, 5u);
    if (!failed) failed |= !timing_80286_load(machine, xlat, sizeof(xlat)) ||
        ((machine->executor_cpu.data.bx = 0x1000u),
            (machine->executor_cpu.data.al = 1u), 0) ||
        core_machine_memory_write(machine, 0x1001u, &xlat_value,
            sizeof(xlat_value)) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 5u) ||
        machine->executor_cpu.data.al != xlat_value;
    if (!failed) failed |= !timing_80286_load(machine, sreg_store_even,
        sizeof(sreg_store_even)) || ((machine->executor_cpu.data.ds.selector =
        sreg_value), 0) || !timing_80286_run(machine, &state, 1u, 3u) ||
        core_machine_memory_read(machine, 0x1000u, &sreg_read,
            sizeof(sreg_read)) != TYPE_STATUS_OK || sreg_read != sreg_value;
    if (!failed) failed |= !timing_80286_load(machine, sreg_store_odd,
        sizeof(sreg_store_odd)) || ((machine->executor_cpu.data.ds.selector =
        sreg_value), 0) || !timing_80286_run(machine, &state, 1u, 5u) ||
        core_machine_memory_read(machine, 0x1001u, &sreg_read,
            sizeof(sreg_read)) != TYPE_STATUS_OK || sreg_read != sreg_value;
    if (!failed) failed |= !timing_80286_load(machine, sreg_store_indexed,
        sizeof(sreg_store_indexed)) || ((machine->executor_cpu.data.ds.selector =
        sreg_value), (machine->executor_cpu.data.bp = 0x1000u),
        (machine->executor_cpu.data.si = 0u), 0) ||
        !timing_80286_run(machine, &state, 1u, 6u) ||
        core_machine_memory_read(machine, 0x1001u, &sreg_read,
            sizeof(sreg_read)) != TYPE_STATUS_OK || sreg_read != sreg_value;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_control_ports(C_VOID)
{
    static const type_unsigned_8 taken[] = { 0x74u, 0x01u, 0x90u, 0x90u };
    static const type_unsigned_8 not_taken[] = { 0x75u, 0x01u, 0x90u, 0x90u };
    static const type_unsigned_8 movsb[] = { 0xa4u };
    static const type_unsigned_8 rep[] = { 0xf3u, 0xa4u };
    static const type_unsigned_8 source[] = { 1u, 2u, 3u };
    static const type_unsigned_8 out_imm[] = { 0xe6u, 0xe0u };
    static const type_unsigned_8 out_dx[] = { 0xeeu };
    static const type_unsigned_8 in_imm[] = { 0xe4u, 0xe0u };
    static const type_unsigned_8 in_dx[] = { 0xecu };
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, taken, sizeof(taken)) ||
        ((machine->executor_cpu.data.eflags |= VCPU_EFLAGS_ZF), 0) ||
        !timing_80286_run(machine, &state, 1u, 7u);
    if (!failed) failed |= !timing_80286_load(machine, not_taken,
        sizeof(not_taken)) || ((machine->executor_cpu.data.eflags |=
        VCPU_EFLAGS_ZF), 0) || !timing_80286_run(machine, &state, 1u, 3u);
    if (!failed) failed |= !timing_80286_load(machine, movsb, sizeof(movsb)) ||
        core_machine_memory_write(machine, 0x1000u, source, 1u) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.si = 0x1000u),
            (machine->executor_cpu.data.di = 0x1100u), 0) ||
        !timing_80286_run(machine, &state, 1u, 5u);
    if (!failed) failed |= !timing_80286_load(machine, rep, sizeof(rep)) ||
        core_machine_memory_write(machine, 0x1000u, source, sizeof(source)) !=
            TYPE_STATUS_OK || ((machine->executor_cpu.data.cx = 3u),
            (machine->executor_cpu.data.si = 0x1000u),
            (machine->executor_cpu.data.di = 0x1100u), 0) ||
        !timing_80286_run(machine, &state, 3u, 17u);
    if (!failed) failed |= !timing_80286_load(machine, out_imm, sizeof(out_imm)) ||
        !timing_80286_run(machine, &state, 1u, 3u) || state.writes != 1u;
    if (!failed) failed |= !timing_80286_load(machine, out_dx, sizeof(out_dx)) ||
        ((machine->executor_cpu.data.dx = 0x00e0u), 0) ||
        !timing_80286_run(machine, &state, 1u, 3u) || state.writes != 2u;
    if (!failed) failed |= !timing_80286_load(machine, in_imm, sizeof(in_imm)) ||
        !timing_80286_run(machine, &state, 1u, 5u) || state.reads != 1u;
    if (!failed) failed |= !timing_80286_load(machine, in_dx, sizeof(in_dx)) ||
        ((machine->executor_cpu.data.dx = 0x00e0u), 0) ||
        !timing_80286_run(machine, &state, 1u, 5u) || state.reads != 2u;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_boundaries(C_VOID)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    static const type_unsigned_8 shift_byte[] = { 0xd0u, 0xc0u };
    static const type_unsigned_8 shift_word[] = { 0xd1u, 0xc0u };
    static const type_unsigned_8 shift_memory[] = { 0xd0u, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_8 shift_word_memory[] = { 0xd1u, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_8 shift_indexed_memory[] = { 0xd0u, 0x4au, 0x01u };
    static const type_unsigned_8 shift_cl[] = { 0xd2u, 0xc0u };
    static const type_unsigned_8 shift_count[] = { 0xc1u, 0xc0u, 0x04u };
    static const type_unsigned_8 shift_cl_memory[] = { 0xd2u, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_8 shift_count_memory[] = { 0xc1u, 0x4au, 0x01u, 0x04u };
    static const type_unsigned_8 shift_undefined[] = { 0xd0u, 0xf0u };
    static const type_unsigned_8 fault[] = { 0x66u, 0x90u };
    static const type_unsigned_8 maximum[] = { 0xf3u, 0xa4u };
    static const type_unsigned_8 source[] = { 0x78u };
    const core_machine_run_budget one = { 1u, 0u };
    const core_machine_run_budget insufficient = { 1u, 8u };
    core_machine_run_result result;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, shift_byte,
        sizeof(shift_byte)) || !timing_80286_run(machine, &state, 1u, 2u);
    if (!failed) failed |= !timing_80286_load(machine, shift_word,
        sizeof(shift_word)) || !timing_80286_run(machine, &state, 1u, 2u);
    if (!failed) failed |= !timing_80286_load(machine, shift_memory,
        sizeof(shift_memory)) || !timing_80286_run(machine, &state, 1u, 7u);
    if (!failed) failed |= !timing_80286_load(machine, shift_word_memory,
        sizeof(shift_word_memory)) || !timing_80286_run(machine, &state, 1u, 7u);
    if (!failed) failed |= !timing_80286_load(machine, shift_indexed_memory,
        sizeof(shift_indexed_memory)) || ((machine->executor_cpu.data.bp = 0x1000u),
        (machine->executor_cpu.data.si = 0u), 0) || core_machine_memory_write(
            machine, 0x1001u, source, 1u) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 8u);
    if (!failed) failed |= !timing_80286_load(machine, shift_cl,
        sizeof(shift_cl)) || ((machine->executor_cpu.data.cx = 4u), 0) ||
        !timing_80286_run(machine, &state, 1u, 9u);
    if (!failed) failed |= !timing_80286_load(machine, shift_count,
        sizeof(shift_count)) || !timing_80286_run(machine, &state, 1u, 9u);
    if (!failed) failed |= !timing_80286_load(machine, shift_cl_memory,
        sizeof(shift_cl_memory)) || ((machine->executor_cpu.data.cx = 4u), 0) ||
        !timing_80286_run(machine, &state, 1u, 12u);
    if (!failed) failed |= !timing_80286_load(machine, shift_count_memory,
        sizeof(shift_count_memory)) || ((machine->executor_cpu.data.bp = 0x1000u),
        (machine->executor_cpu.data.si = 0u), 0) || core_machine_memory_write(
            machine, 0x1001u, source, 1u) != TYPE_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 13u);
    if (!failed) failed |= !timing_80286_load(machine, shift_cl,
        sizeof(shift_cl)) || ((machine->executor_cpu.data.cx = 0x24u), 0) ||
        !timing_80286_run(machine, &state, 1u, 9u);
    if (!failed) failed |= !timing_80286_load(machine, shift_undefined,
        sizeof(shift_undefined)) || !test_core_machine_fixture_preflight_real_ud_terminal(
            machine) || core_machine_run(machine, one, &result) != TYPE_STATUS_FAULT ||
        result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
        result.ticks != 0u || state.advanced_ticks != 0u;
    if (!failed) failed |= !timing_80286_load(machine, fault, sizeof(fault)) ||
        !test_core_machine_fixture_preflight_real_ud_terminal(machine) ||
        core_machine_run(machine, one, &result) != TYPE_STATUS_FAULT ||
        result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
        result.ticks != 0u || state.advanced_ticks != 0u;
    if (!failed) failed |= !timing_80286_load(machine, nop, sizeof(nop)) ||
        !timing_80286_run(machine, &state, 1u, 3u) ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        machine->elapsed_ticks != 0u || state.advanced_ticks != 0u ||
        !timing_80286_load(machine, nop, sizeof(nop)) ||
        core_machine_request_stop(machine) != TYPE_STATUS_OK ||
        core_machine_run(machine, one, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_REQUESTED || result.executed != 0u ||
        result.ticks != 0u || state.advanced_ticks != 0u;
    if (!failed) failed |= !timing_80286_load(machine, maximum,
        sizeof(maximum)) || ((machine->executor_cpu.data.cx = 1u),
        (machine->executor_cpu.data.si = 0x1000u),
        (machine->executor_cpu.data.di = 0x1100u), 0) ||
        core_machine_memory_write(machine, 0x1000u, source, sizeof(source)) !=
            TYPE_STATUS_OK ||
        core_machine_run(machine, insufficient, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
        result.ticks != 0u || !timing_80286_run(machine, &state, 1u, 9u);
    if (!failed) failed |= !timing_80286_load(machine, nop, sizeof(nop));
    if (!failed) {
        machine->elapsed_ticks = UINT64_MAX - 2u;
        state.advanced_ticks = 0u;
        failed |= core_machine_run(machine, one, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            result.ticks != 0u || machine->elapsed_ticks != UINT64_MAX - 2u ||
            state.advanced_ticks != 0u;
    }
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    static const type_unsigned_8 clc[] = { 0xf8u };
    static const type_unsigned_8 cmc[] = { 0xf5u };
    static const type_unsigned_8 stc[] = { 0xf9u };
    static const type_unsigned_8 cld[] = { 0xfcu };
    static const type_unsigned_8 std[] = { 0xfdu };
    static const type_unsigned_8 cli[] = { 0xfau };
    static const type_unsigned_8 sti[] = { 0xfbu };
    static const type_unsigned_8 sahf[] = { 0x9eu };
    static const type_unsigned_8 lahf[] = { 0x9fu };
    static const type_unsigned_8 immediate[] = { 0xb8u, 0x34u, 0x12u };
    static const type_unsigned_8 registers[] = { 0x8bu, 0xc1u };

    if (timing_80286_case(nop, sizeof(nop), 3u) ||
        timing_80286_case(clc, sizeof(clc), 2u) ||
        timing_80286_case(cmc, sizeof(cmc), 2u) ||
        timing_80286_case(stc, sizeof(stc), 2u) ||
        timing_80286_case(cld, sizeof(cld), 2u) ||
        timing_80286_case(std, sizeof(std), 2u) ||
        timing_80286_case(cli, sizeof(cli), 3u) ||
        timing_80286_case(sti, sizeof(sti), 2u) ||
        timing_80286_case(sahf, sizeof(sahf), 2u) ||
        timing_80286_case(lahf, sizeof(lahf), 2u) ||
        timing_80286_case(immediate, sizeof(immediate), 2u) ||
        timing_80286_case(registers, sizeof(registers), 2u)) return 1;
    if (timing_80286_xlat()) return 22;
    if (timing_80286_lahf_sahf()) return 5;
    if (timing_80286_sreg_store()) return 6;
    if (timing_80286_sreg_load()) return 7;
    if (timing_80286_sreg_load_protected()) return 8;
    if (timing_80286_les_lds()) return 9;
    if (timing_80286_sreg_stack()) return 10;
    if (timing_80286_bound()) return 11;
    if (timing_80286_arpl()) return 12;
    if (timing_80286_verr_verw()) return 13;
    if (timing_80286_lar()) return 14;
    if (timing_80286_lsl()) return 15;
    if (timing_80286_smsw()) return 16;
    if (timing_80286_sldt_str()) return 17;
    if (timing_80286_lmsw()) return 18;
    if (timing_80286_table_control()) return 19;
    if (timing_80286_ldt_task_registers()) return 20;
    if (timing_80286_fpu_interface_transfer()) return 21;
    if (timing_80286_memory()) return 2;
    if (timing_80286_control_ports()) return 3;
    if (timing_80286_boundaries()) return 4;
    STD_PRINTF("M5:T357:S6:80286-INSTRUCTION-TIMING-LEDGER:OK\n");
    return 0;
}
