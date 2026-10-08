#include "lib/types/types_interface.h"
#include "x86/core/debug_interface.h"
#include "x86/core/machine.h"
#include "lib/types/file.h"
#include "x86/core/device_support_interface.h"

#include "x86/core/machine_interface.h"
#include "x86/chips/cpu/cpu_interface.h"
#include "exception_fixture.h"

#define TIMING_80286_RESET_LINEAR 0x00fffff0u
#define TIMING_80286_RESET_PHYSICAL 0x000ffff0u

typedef struct timing_80286_state {
    lib_u32 reads;
    lib_u32 writes;
    lib_u64 advanced_ticks;
} timing_80286_state;

static lib_status timing_80286_read(void *owner, lib_u16 port, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)tick;
    timing_80286_state *state = (timing_80286_state *)owner;

    if (state == LIB_NULL || out_value == LIB_NULL || port != 0x00e0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    ++state->reads;
    *out_value = 0x5au;
    return LIB_STATUS_OK;
}

static lib_status timing_80286_write(void *owner, lib_u16 port,
    lib_u32 value)
{
    timing_80286_state *state = (timing_80286_state *)owner;

    if (state == LIB_NULL || port != 0x00e0u || value > 0xffffu)
        return LIB_STATUS_INVALID_ARGUMENT;
    ++state->writes;
    return LIB_STATUS_OK;
}

static const core_machine_port_provider timing_80286_ports = {
    timing_80286_read, timing_80286_write
};

static void timing_80286_reset(void *opaque)
{
    timing_80286_state *state = (timing_80286_state *)opaque;
    if (state != LIB_NULL) state->advanced_ticks = 0u;
}

static void timing_80286_advance(void *opaque, lib_u64 ticks)
{
    timing_80286_state *state = (timing_80286_state *)opaque;
    if (state != LIB_NULL) state->advanced_ticks += ticks;
}

static const core_machine_execution_provider timing_80286_execution = {
    timing_80286_reset, timing_80286_advance
};

static lib_i32 timing_80286_capture(core_machine *machine,
    core_machine_debug_cpu_snapshot *snapshot)
{
    return core_machine_debug_capture_cpu_snapshot(machine,
        CORE_MACHINE_CPU_SNAPSHOT_CURRENT, snapshot) == LIB_STATUS_OK;
}

static lib_i32 timing_80286_patch_register(core_machine *machine,
    core_machine_debug_register register_id, lib_u32 mask, lib_u32 value)
{
    lib_u32 previous;
    return core_machine_debug_read_register(machine, register_id, &previous) ==
        LIB_STATUS_OK && core_machine_debug_write_register(machine, register_id,
            (previous & ~mask) | value) == LIB_STATUS_OK;
}

static lib_i32 timing_80286_register_matches(core_machine *machine,
    core_machine_debug_register register_id, lib_u32 mask, lib_u32 expected)
{
    lib_u32 value;
    return core_machine_debug_read_register(machine, register_id, &value) ==
        LIB_STATUS_OK && (value & mask) == expected;
}

static lib_i32 timing_80286_entry(core_machine *machine, lib_u32 eip)
{
    const core_machine_debug_register_patch patch = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
        .values = {[CORE_MACHINE_DEBUG_EIP] = eip}
    };
    return core_machine_debug_patch_registers(machine, &patch) == LIB_STATUS_OK;
}

static lib_i32 timing_80286_resume(core_machine *machine, lib_u32 eip)
{
    return core_machine_debug_write_register(machine, CORE_MACHINE_DEBUG_EIP,
        eip) == LIB_STATUS_OK;
}

/* Architectural setup consumes the same timeline; the measured row starts later. */
static lib_i32 timing_80286_setup_instruction(core_machine *machine,
    const lib_u8 *code, lib_size bytes, lib_u64 instructions)
{
    core_machine_debug_cpu_snapshot before;
    lib_u8 saved[8];
    core_machine_run_result result;
    lib_u32 address;

    if (bytes > sizeof(saved) || !timing_80286_capture(machine, &before))
        return 0;
    address = before.cs.base + before.eip;
    return core_machine_memory_read(machine, address, saved, bytes) ==
        LIB_STATUS_OK && core_machine_memory_write(machine, address, code,
            bytes) == LIB_STATUS_OK && core_machine_run(machine,
            (core_machine_run_budget){instructions,0u}, &result) ==
            LIB_STATUS_OK && result.reason == CORE_MACHINE_STOP_BUDGET &&
        result.executed == instructions && core_machine_memory_write(machine,
            address, saved, bytes) == LIB_STATUS_OK &&
        core_machine_debug_write_register(machine, CORE_MACHINE_DEBUG_EIP,
            before.eip) == LIB_STATUS_OK &&
        core_machine_debug_write_register(machine, CORE_MACHINE_DEBUG_EAX,
            before.eax) == LIB_STATUS_OK;
}

static lib_i32 timing_80286_load_table(core_machine *machine, lib_bool idt)
{
    const lib_u8 pointer[] = {0x57u,0x13u,0u,0x34u,0x12u,0u};
    const lib_u8 code[] = {0x0fu,0x01u,idt ? 0x1eu : 0x16u,0u,0x17u};
    return core_machine_memory_write(machine, 0x1700u, pointer,
        sizeof(pointer)) == LIB_STATUS_OK &&
        timing_80286_setup_instruction(machine, code, sizeof(code), 1u);
}

static lib_i32 timing_80286_load_system_selector(core_machine *machine, lib_bool task)
{
    core_machine_debug_cpu_snapshot before;
    const lib_u16 selector = task ? 8u : 0x10u;
    const lib_u8 descriptor[] = {task ? 0x2bu : 0x0fu,0u,0u,
        task ? 0x60u : 0x50u,0u,task ? 0x81u : 0x82u,0u,0u};
    const lib_u8 code[] = {0xb8u,(lib_u8)selector,0u,0x0fu,0u,
        task ? 0xd8u : 0xd0u};
    lib_u8 saved[8];
    lib_u32 address;

    if (!timing_80286_capture(machine, &before)) return 0;
    address = before.gdtr.base + selector;
    return core_machine_memory_read(machine, address, saved, sizeof(saved)) ==
        LIB_STATUS_OK && core_machine_memory_write(machine, address, descriptor,
            sizeof(descriptor)) == LIB_STATUS_OK &&
        timing_80286_setup_instruction(machine, code, sizeof(code), 2u) &&
        core_machine_memory_write(machine, address, saved, sizeof(saved)) ==
            LIB_STATUS_OK;
}

static lib_i32 timing_80286_prepare(core_machine **out_machine,
    timing_80286_state *state)
{
    const core_machine_executor_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286,
        .ticks_per_instruction = 29u,
        .instruction_timing = { 29u, 7u, 31u, 37u, 41u, 43u }
    };
    const core_machine_memory_alias_config alias = {
        TIMING_80286_RESET_LINEAR, TIMING_80286_RESET_PHYSICAL, 16u
    };
    core_machine *machine = LIB_NULL;

    if (out_machine == LIB_NULL || state == LIB_NULL ||
        core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        machine == LIB_NULL || core_machine_install_memory_aliases(machine, &alias, 1u, LIB_FALSE) !=
            LIB_STATUS_OK || core_machine_install_port_provider(machine,
            0x00e0u, 0x00e0u, &timing_80286_ports, state) != LIB_STATUS_OK ||
        test_core_exception_block_vector(machine, 6u, state) != LIB_STATUS_OK ||
        core_machine_bind_execution_provider(machine, &timing_80286_execution, state) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static lib_i32 timing_80286_load(core_machine *machine,
    const lib_u8 *program, lib_size bytes)
{
    return core_machine_reset(machine) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, TIMING_80286_RESET_LINEAR,
            program, bytes) == LIB_STATUS_OK;
}

static lib_i32 timing_80286_run(core_machine *machine, timing_80286_state *state,
    lib_u64 instructions, lib_u64 ticks)
{
    const core_machine_run_budget budget = { instructions, 0u };
    core_machine_run_result result = {0};
    const lib_u64 elapsed = machine->elapsed_ticks;
    const lib_u64 advanced = state->advanced_ticks;

    const lib_status status = core_machine_run(machine, budget, &result);
    const lib_bool passed = status == LIB_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET &&
        result.executed == instructions && result.ticks == ticks &&
        result.elapsed_ticks == elapsed + ticks && state->advanced_ticks == advanced + ticks;
    if (!passed) lib_c_fprintf(lib_c_stderr,
        "286 row: status=%u reason=%u instructions=%llu/%llu ticks=%llu/%llu elapsed=%llu/%llu observer=%llu/%llu\n",
        (unsigned)status, (unsigned)result.reason,
        (unsigned long long)result.executed, (unsigned long long)instructions,
        (unsigned long long)result.ticks, (unsigned long long)ticks,
        (unsigned long long)result.elapsed_ticks, (unsigned long long)(elapsed + ticks),
        (unsigned long long)state->advanced_ticks, (unsigned long long)(advanced + ticks));
    return passed;
}

static lib_i32 timing_80286_case(const lib_u8 *program, lib_size bytes,
    lib_u64 ticks)
{
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state) ||
        !timing_80286_load(machine, program, bytes) ||
        !timing_80286_run(machine, &state, 1u, ticks);

    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_xlat(void)
{
    static const lib_u8 program[] = { 0xd7u };
    static const lib_u8 value[] = { 0x5au };
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state) ||
        !timing_80286_load(machine, program, sizeof(program)) ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EBX, 0xffffffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 1u)) ||
        core_machine_memory_write(machine, 0x1001u, value, sizeof(value)) !=
            LIB_STATUS_OK || !timing_80286_run(machine, &state, 1u, 5u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EAX, 0xffu, value[0]) ||
        core_machine_cpu_capture_timing(machine->executor_cpu_execution).
            source_timing_unallocated;

    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_lahf_sahf(void)
{
    static const lib_u8 lahf[] = { 0x9fu };
    static const lib_u8 sahf[] = { 0x9eu };
    const lib_u32 transferred = CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_PF |
        CORE_MACHINE_DEBUG_EFLAGS_AF | CORE_MACHINE_DEBUG_EFLAGS_ZF | CORE_MACHINE_DEBUG_EFLAGS_SF;
    const lib_u32 preserved = CORE_MACHINE_DEBUG_EFLAGS_IF | CORE_MACHINE_DEBUG_EFLAGS_DF |
        CORE_MACHINE_DEBUG_EFLAGS_OF;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, lahf, sizeof(lahf)) ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0x11223344u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EFLAGS, 0xffffffffu, transferred)) ||
        !timing_80286_run(machine, &state, 1u, 2u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0x1122d744u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EFLAGS, 0xffffffffu, transferred);
    if (!failed) failed |= !timing_80286_load(machine, sahf, sizeof(sahf)) ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0x1122d744u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EFLAGS, 0xffffffffu, preserved)) ||
        !timing_80286_run(machine, &state, 1u, 2u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0x1122d744u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EFLAGS, 0xffffffffu, (preserved | transferred));
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_sreg_store(void)
{
    static const lib_u8 store_ds_ax[] = { 0x8cu, 0xd8u };
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, store_ds_ax,
        sizeof(store_ds_ax)) || (!timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0xaabb0000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_DS, 0xffffffffu, 0x1357u)) ||
        !timing_80286_run(machine, &state, 1u, 2u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0xaabb1357u);
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_sreg_load(void)
{
    static const lib_u8 load_es_ax[] = { 0x8eu, 0xc0u };
    static const lib_u8 load_ss_ax[] = { 0x8eu, 0xd0u };
    static const lib_u8 load_ds_ax[] = { 0x8eu, 0xd8u };
    static const lib_u8 load_ds_even[] = { 0x8eu, 0x1eu, 0x00u, 0x10u };
    static const lib_u8 load_ds_odd[] = { 0x8eu, 0x1eu, 0x01u, 0x10u };
    static const lib_u8 load_ds_indexed[] = { 0x8eu, 0x5au, 0x01u };
    const lib_u16 selector = 0x1357u;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine_debug_cpu_snapshot snapshot;
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, load_es_ax,
        sizeof(load_es_ax)) || (!timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0xaabb1357u)) || !timing_80286_run(machine, &state, 1u, 2u) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.es.selector != selector ||
        snapshot.es.base != 0x13570u);
    if (!failed) failed |= !timing_80286_load(machine, load_ss_ax,
        sizeof(load_ss_ax)) || (!timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0xaabb1357u)) || !timing_80286_run(machine, &state, 1u, 2u) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.ss.selector != selector ||
        snapshot.ss.base != 0x13570u);
    if (!failed) failed |= !timing_80286_load(machine, load_ds_ax,
        sizeof(load_ds_ax)) || (!timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0xaabb1357u)) || !timing_80286_run(machine, &state, 1u, 2u) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.ds.selector != selector ||
        snapshot.ds.base != 0x13570u);
    if (!failed) failed |= !timing_80286_load(machine, load_ds_even,
        sizeof(load_ds_even)) || core_machine_memory_write(machine, 0x1000u,
        &selector, sizeof(selector)) != LIB_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 5u) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.ds.selector != selector);
    if (!failed) failed |= !timing_80286_load(machine, load_ds_odd,
        sizeof(load_ds_odd)) || core_machine_memory_write(machine, 0x1001u,
        &selector, sizeof(selector)) != LIB_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 7u) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.ds.selector != selector);
    if (!failed) failed |= !timing_80286_load(machine, load_ds_indexed,
        sizeof(load_ds_indexed)) || (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EBP, 0xffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffu, 0u)) || core_machine_memory_write(
            machine, 0x1001u, &selector, sizeof(selector)) != LIB_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 8u) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.ds.selector != selector);
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_boot_protected(core_machine *machine,
    timing_80286_state *state)
{
    static const lib_u8 gdt_pointer[] = { 0x17u, 0u, 0u, 0x03u, 0u, 0u };
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0
    };
    static const lib_u8 boot[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u, 0xb8u,0x01u,0u,
        0x0fu,0x01u,0xf0u, 0xb8u,0x10u,0u, 0x8eu,0xd0u,
        0x8eu,0xd8u, 0xeau,0u,0u,0x08u,0u
    };
    static const lib_u8 halt[] = { 0xf4u };
    core_machine_run_result result;

    (void)state;
    return core_machine_reset(machine) == LIB_STATUS_OK && timing_80286_entry(machine, 0x0200u) &&
        core_machine_memory_write(machine, 0x100u, gdt_pointer,
            sizeof(gdt_pointer)) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, 0x300u, gdt, sizeof(gdt)) ==
            LIB_STATUS_OK && core_machine_memory_write(machine, 0x0200u, boot,
            sizeof(boot)) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, 0x2000u, halt, sizeof(halt)) ==
            LIB_STATUS_OK && core_machine_run(machine,
            (core_machine_run_budget){7u, 0u}, &result) == LIB_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET && result.executed == 7u ;
}

static lib_i32 timing_80286_boot_protected_system(core_machine *machine,
    timing_80286_state *state)
{
    static const lib_u8 gdt_pointer[] = { 0x37u, 0u, 0u, 0x03u, 0u, 0u };
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x30u,0,0x12u,0,0,
        0xffu,0xffu,0,0x30u,0,0x98u,0,0,
        0x0fu,0,0,0x50u,0,0x82u,0,0,
        0xffu,0xffu,0,0,0,0x89u,0,0
    };
    static const lib_u8 boot[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u, 0xb8u,0x01u,0u,
        0x0fu,0x01u,0xf0u, 0xb8u,0x10u,0u, 0x8eu,0xd0u,
        0x8eu,0xd8u, 0xeau,0u,0u,0x08u,0u
    };
    static const lib_u8 halt[] = { 0xf4u };
    core_machine_run_result result;

    (void)state;
    return core_machine_reset(machine) == LIB_STATUS_OK && timing_80286_entry(machine, 0x0200u) &&
        core_machine_memory_write(machine, 0x100u, gdt_pointer,
            sizeof(gdt_pointer)) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, 0x300u, gdt, sizeof(gdt)) ==
            LIB_STATUS_OK && core_machine_memory_write(machine, 0x0200u, boot,
            sizeof(boot)) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, 0x2000u, halt, sizeof(halt)) ==
            LIB_STATUS_OK && core_machine_run(machine,
            (core_machine_run_budget){7u, 0u}, &result) == LIB_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET && result.executed == 7u ;
}

static lib_i32 timing_80286_sreg_load_protected(void)
{
    static const lib_u8 direct[] = { 0x8eu, 0xd8u };
    static const lib_u8 memory_even[] = { 0x8eu, 0x06u, 0x00u, 0x10u };
    static const lib_u8 memory_odd[] = { 0x8eu, 0x06u, 0x01u, 0x10u };
    static const lib_u8 memory_indexed[] = { 0x8eu, 0x42u, 0x01u };
    const lib_u16 selector = 0x0010u;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine_debug_cpu_snapshot snapshot;
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state) ||
        !timing_80286_boot_protected(machine, &state);

    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, direct,
        sizeof(direct)) != LIB_STATUS_OK || (!timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0xaabb0010u)) || (!timing_80286_resume(
            machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 17u)) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.ds.selector != selector);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, memory_even,
        sizeof(memory_even)) != LIB_STATUS_OK || core_machine_memory_write(machine,
            0x4000u, &selector, sizeof(selector)) != LIB_STATUS_OK ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 19u)) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.es.selector != selector);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, memory_odd,
        sizeof(memory_odd)) != LIB_STATUS_OK || core_machine_memory_write(machine,
            0x4001u, &selector, sizeof(selector)) != LIB_STATUS_OK ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 21u)) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.es.selector != selector);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, memory_indexed,
        sizeof(memory_indexed)) != LIB_STATUS_OK || (!timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_EBP, 0xffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffu, 0u)) ||
        core_machine_memory_write(machine, 0x4001u, &selector, sizeof(selector)) !=
            LIB_STATUS_OK || (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 22u)) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.es.selector != selector);
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_les_lds(void)
{
    static const lib_u8 les_direct[] = { 0xc4u, 0x06u, 0x00u, 0x10u };
    static const lib_u8 lds_direct[] = { 0xc5u, 0x06u, 0x00u, 0x10u };
    static const lib_u8 les_indexed[] = { 0xc4u, 0x42u, 0x01u };
    static const lib_u8 lds_indexed[] = { 0xc5u, 0x42u, 0x01u };
    const lib_u16 real_pointer[] = { 0x3344u, 0x1234u };
    const lib_u16 protected_pointer[] = { 0x3344u, 0x0010u };
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine_debug_cpu_snapshot snapshot;
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, les_direct,
        sizeof(les_direct)) || core_machine_memory_write(machine, 0x1000u,
        real_pointer, sizeof(real_pointer)) != LIB_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 7u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EAX, 0xffffu, 0x3344u) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.es.selector != 0x1234u);
    if (!failed) failed |= !timing_80286_load(machine, lds_direct,
        sizeof(lds_direct)) || core_machine_memory_write(machine, 0x1000u,
        real_pointer, sizeof(real_pointer)) != LIB_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 7u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EAX, 0xffffu, 0x3344u) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.ds.selector != 0x1234u);
    if (!failed) failed |= !timing_80286_load(machine, les_indexed,
        sizeof(les_indexed)) || (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EBP, 0xffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffu, 0u)) || core_machine_memory_write(
            machine, 0x1001u, real_pointer, sizeof(real_pointer)) !=
            LIB_STATUS_OK || !timing_80286_run(machine, &state, 1u, 12u) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.es.selector != 0x1234u);
    if (!failed) failed |= !timing_80286_load(machine, lds_indexed,
        sizeof(lds_indexed)) || (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EBP, 0xffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffu, 0u)) || core_machine_memory_write(
            machine, 0x1001u, real_pointer, sizeof(real_pointer)) !=
            LIB_STATUS_OK || !timing_80286_run(machine, &state, 1u, 12u) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.ds.selector != 0x1234u);
    if (!failed) failed |= !timing_80286_boot_protected(machine, &state);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        les_direct, sizeof(les_direct)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x4000u, protected_pointer,
            sizeof(protected_pointer)) != LIB_STATUS_OK ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 21u)) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EAX, 0xffffu, 0x3344u) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.es.selector != 0x0010u);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        lds_direct, sizeof(lds_direct)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x4000u, protected_pointer,
            sizeof(protected_pointer)) != LIB_STATUS_OK ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 21u)) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EAX, 0xffffu, 0x3344u) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.ds.selector != 0x0010u);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        les_indexed, sizeof(les_indexed)) != LIB_STATUS_OK ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EBP, 0xffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffu, 0u)) || core_machine_memory_write(
            machine, 0x4001u, protected_pointer, sizeof(protected_pointer)) !=
            LIB_STATUS_OK || (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 26u)) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.es.selector != 0x0010u);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        lds_indexed, sizeof(lds_indexed)) != LIB_STATUS_OK ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EBP, 0xffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffu, 0u)) || core_machine_memory_write(
            machine, 0x4001u, protected_pointer, sizeof(protected_pointer)) !=
            LIB_STATUS_OK || (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 26u)) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.ds.selector != 0x0010u);
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_sreg_stack(void)
{
    static const lib_u8 push_ops[] = { 0x06u, 0x0eu, 0x16u, 0x1eu };
    static const lib_u8 pop_ops[] = { 0x07u, 0x17u, 0x1fu };
    static const lib_u16 real_selectors[] = {
        0x1111u, 0x2222u, 0x3333u, 0x4444u
    };
    static const lib_u16 protected_selectors[] = {
        0x0010u, 0x0008u, 0x0010u, 0x0010u
    };
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine_debug_cpu_snapshot snapshot;
    core_machine *machine = LIB_NULL;
    lib_u16 image;
    lib_u8 index;
    lib_i32 failed = !timing_80286_prepare(&machine, &state);

    for (index = 0u; !failed && index < sizeof(push_ops); ++index) {
        failed |= !timing_80286_load(machine, &push_ops[index], 1u) ||
            (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESP, 0xffffffffu, 0x8000u) ||
            !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ES, 0xffffffffu, real_selectors[0]) ||
            !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_CS, 0xffffffffu, real_selectors[1]) ||
            !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_SS, 0xffffffffu, real_selectors[2]) ||
            !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_DS, 0xffffffffu, real_selectors[3])) ||
            core_machine_memory_write(machine, ((lib_u32)real_selectors[1] << 4u) + 0xfff0u,
                &push_ops[index], 1u) != LIB_STATUS_OK ||
            !timing_80286_run(machine, &state, 1u, 3u) ||
            !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_ESP, 0xffffffffu, 0x7ffeu) ||
            core_machine_memory_read(machine, ((lib_u32)real_selectors[2] << 4u) + 0x7ffeu,
                &image, sizeof(image)) !=
                LIB_STATUS_OK || image != real_selectors[index];
    }
    for (index = 0u; !failed && index < sizeof(pop_ops); ++index) {
        const lib_u16 selector = (lib_u16)(0x5555u + index);

        failed |= !timing_80286_load(machine, &pop_ops[index], 1u) ||
            (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESP, 0xffffffffu, 0x8000u)) ||
            core_machine_memory_write(machine, 0x8000u, &selector,
                sizeof(selector)) != LIB_STATUS_OK ||
            !timing_80286_run(machine, &state, 1u, 5u) ||
            !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_ESP, 0xffffffffu, 0x8002u) ||
            !timing_80286_capture(machine, &snapshot) ||
            (index == 0u ? snapshot.es.selector :
            index == 1u ? snapshot.ss.selector : snapshot.ds.selector) != selector;
    }
    if (!failed) failed |= !timing_80286_boot_protected(machine, &state);
    for (index = 0u; !failed && index < sizeof(push_ops); ++index) {
        failed |= core_machine_memory_write(machine, 0x2000u, &push_ops[index],
            1u) != LIB_STATUS_OK || (!timing_80286_patch_register(machine,
            CORE_MACHINE_DEBUG_ESP, 0xffffffffu, 0x8000u) ||
            !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ES, 0xffffffffu, 0x0010u)) ||
            (!timing_80286_resume(machine, 0u) ||
            !timing_80286_run(machine, &state, 1u, 3u)) ||
            !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_ESP, 0xffffffffu, 0x7ffeu) ||
            core_machine_memory_read(machine, 0xaffeu,
                (void *)CORE_MACHINE_REFERENCE_OF(image), sizeof(image)) != LIB_STATUS_OK ||
            image != protected_selectors[index];
    }
    for (index = 0u; !failed && index < sizeof(pop_ops); ++index) {
        const lib_u16 selector = 0x0010u;

        failed |= core_machine_memory_write(machine, 0x2000u, &pop_ops[index],
            1u) != LIB_STATUS_OK || (!timing_80286_patch_register(machine,
            CORE_MACHINE_DEBUG_ESP, 0xffffffffu, 0x8000u)) ||
            core_machine_memory_write(machine, 0xb000u,
                &selector, sizeof(selector)) != LIB_STATUS_OK ||
            (!timing_80286_resume(machine, 0u) ||
            !timing_80286_run(machine, &state, 1u, 20u)) ||
            !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_ESP, 0xffffffffu, 0x8002u) ||
            !timing_80286_capture(machine, &snapshot) ||
            (index == 0u ? snapshot.es.selector :
            index == 1u ? snapshot.ss.selector : snapshot.ds.selector) != selector;
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_bound(void)
{
    static const lib_u8 direct[] = { 0x62u, 0x06u, 0x00u, 0x10u };
    static const lib_u8 indexed[] = { 0x62u, 0x42u, 0x01u };
    const lib_i16 bounds[] = { -2, 3 };
    const lib_u32 flags = CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_ZF;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, direct, sizeof(direct)) ||
        core_machine_memory_write(machine, 0x1000u, bounds, sizeof(bounds)) !=
            LIB_STATUS_OK || (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0xaabb0002u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EFLAGS, 0xffffffffu, flags)) ||
        !timing_80286_run(machine, &state, 1u, 13u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0xaabb0002u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EFLAGS, 0xffffffffu, flags);
    if (!failed) failed |= !timing_80286_load(machine, indexed, sizeof(indexed)) ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EBP, 0xffffffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffffffu, 0u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0xaabb0002u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EFLAGS, 0xffffffffu, flags)) ||
        core_machine_memory_write(machine, 0x1001u, bounds, sizeof(bounds)) !=
            LIB_STATUS_OK || !timing_80286_run(machine, &state, 1u, 13u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0xaabb0002u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EFLAGS, 0xffffffffu, flags);
    if (!failed) failed |= !timing_80286_boot_protected(machine, &state);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, direct,
        sizeof(direct)) != LIB_STATUS_OK || core_machine_memory_write(machine,
        0x4000u, bounds, sizeof(bounds)) != LIB_STATUS_OK ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0xaabb0002u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EFLAGS, 0xffffffffu, flags)) ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 13u)) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0xaabb0002u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EFLAGS, 0xffffffffu, flags);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, indexed,
        sizeof(indexed)) != LIB_STATUS_OK || (!timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_EBP, 0xffffffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffffffu, 0u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0xaabb0002u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EFLAGS, 0xffffffffu, flags)) ||
        core_machine_memory_write(machine, 0x4001u, bounds, sizeof(bounds)) !=
            LIB_STATUS_OK || (!timing_80286_resume(
            machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 13u)) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0xaabb0002u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EFLAGS, 0xffffffffu, flags);
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_arpl(void)
{
    static const lib_u8 register_form[] = { 0x63u, 0xc8u };
    static const lib_u8 direct[] = { 0x63u, 0x0eu, 0x00u, 0x10u };
    static const lib_u8 indexed[] = { 0x63u, 0x4au, 0x01u };
    lib_u16 selector;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state) ||
        !timing_80286_boot_protected(machine, &state);

    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        register_form, sizeof(register_form)) != LIB_STATUS_OK ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0x0001u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ECX, 0xffffffffu, 0x0003u)) ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 10u)) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EAX, 0xffffu, 0x0003u) ||
        !timing_80286_register_matches(machine,
        CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_EFLAGS_ZF, CORE_MACHINE_DEBUG_EFLAGS_ZF);
    selector = 0x0001u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, direct,
        sizeof(direct)) != LIB_STATUS_OK || core_machine_memory_write(machine,
        0x4000u, &selector, sizeof(selector)) != LIB_STATUS_OK ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ECX, 0xffffffffu, 0x0003u)) ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 11u)) ||
        core_machine_memory_read(machine, 0x4000u, &selector, sizeof(selector)) !=
            LIB_STATUS_OK || selector != 0x0003u ||
        !timing_80286_register_matches(machine,
        CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_EFLAGS_ZF, CORE_MACHINE_DEBUG_EFLAGS_ZF);
    selector = 0x0003u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, indexed,
        sizeof(indexed)) != LIB_STATUS_OK || core_machine_memory_write(machine,
        0x4001u, &selector, sizeof(selector)) != LIB_STATUS_OK ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EBP, 0xffffffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffffffu, 0u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ECX, 0xffffffffu, 0x0001u)) ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 11u)) ||
        core_machine_memory_read(machine, 0x4001u, &selector, sizeof(selector)) !=
            LIB_STATUS_OK || selector != 0x0003u ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_EFLAGS_ZF, 0u);
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_verr_verw(void)
{
    static const lib_u8 verr_register[] = { 0x0fu,0x00u,0xe0u };
    static const lib_u8 verw_direct[] = { 0x0fu,0x00u,0x2eu,0x00u,0x10u };
    static const lib_u8 verr_indexed[] = { 0x0fu,0x00u,0x62u,0x01u };
    const lib_u16 selector = 0x0010u;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state) ||
        !timing_80286_boot_protected(machine, &state);

    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        verr_register, sizeof(verr_register)) != LIB_STATUS_OK ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, selector)) ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 14u)) ||
        !timing_80286_register_matches(machine,
        CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_EFLAGS_ZF, CORE_MACHINE_DEBUG_EFLAGS_ZF);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        verw_direct, sizeof(verw_direct)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x4000u, &selector, sizeof(selector)) !=
            LIB_STATUS_OK || (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 16u)) ||
        !timing_80286_register_matches(machine,
        CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_EFLAGS_ZF, CORE_MACHINE_DEBUG_EFLAGS_ZF);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        verr_indexed, sizeof(verr_indexed)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x4001u, &selector, sizeof(selector)) !=
            LIB_STATUS_OK || (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EBP, 0xffffffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffffffu, 0u)) ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 17u)) ||
        !timing_80286_register_matches(machine,
        CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_EFLAGS_ZF, CORE_MACHINE_DEBUG_EFLAGS_ZF);
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_lar(void)
{
    static const lib_u8 register_form[] = { 0x0fu,0x02u,0xc8u };
    static const lib_u8 direct[] = { 0x0fu,0x02u,0x0eu,0x00u,0x10u };
    static const lib_u8 indexed[] = { 0x0fu,0x02u,0x4au,0x01u };
    const lib_u16 selector = 0x0010u;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state) ||
        !timing_80286_boot_protected(machine, &state);

    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        register_form, sizeof(register_form)) != LIB_STATUS_OK ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, selector) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ECX, 0xffffffffu, 0u)) ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 14u)) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_ECX, 0xffffu, 0x9300u) ||
        !timing_80286_register_matches(machine,
        CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_EFLAGS_ZF, CORE_MACHINE_DEBUG_EFLAGS_ZF);
    if (!failed) failed |= (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0x0018u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ECX, 0xffffffffu, 0x3456u)) ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 14u)) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_ECX, 0xffffu, 0x3456u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_EFLAGS_ZF, 0u);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, direct,
        sizeof(direct)) != LIB_STATUS_OK || core_machine_memory_write(machine,
        0x4000u, &selector, sizeof(selector)) != LIB_STATUS_OK ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ECX, 0xffffffffu, 0u)) || (!timing_80286_resume(
        machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 16u)) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_ECX, 0xffffu, 0x9300u) ||
        !timing_80286_register_matches(machine,
        CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_EFLAGS_ZF, CORE_MACHINE_DEBUG_EFLAGS_ZF);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, indexed,
        sizeof(indexed)) != LIB_STATUS_OK || core_machine_memory_write(machine,
        0x4001u, &selector, sizeof(selector)) != LIB_STATUS_OK ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EBP, 0xffffffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffffffu, 0u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ECX, 0xffffffffu, 0u)) ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 17u)) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_ECX, 0xffffu, 0x9300u) ||
        !timing_80286_register_matches(machine,
        CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_EFLAGS_ZF, CORE_MACHINE_DEBUG_EFLAGS_ZF);
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_lsl(void)
{
    static const lib_u8 register_form[] = { 0x0fu,0x03u,0xc8u };
    static const lib_u8 direct[] = { 0x0fu,0x03u,0x0eu,0x00u,0x10u };
    static const lib_u8 indexed[] = { 0x0fu,0x03u,0x4au,0x01u };
    const lib_u16 selector = 0x0010u;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state) ||
        !timing_80286_boot_protected(machine, &state);

    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        register_form, sizeof(register_form)) != LIB_STATUS_OK ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, selector) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ECX, 0xffffffffu, 0u)) ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 14u)) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_ECX, 0xffffu, 0xffffu) ||
        !timing_80286_register_matches(machine,
        CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_EFLAGS_ZF, CORE_MACHINE_DEBUG_EFLAGS_ZF);
    if (!failed) failed |= (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0x0018u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ECX, 0xffffffffu, 0x3456u)) ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 14u)) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_ECX, 0xffffu, 0x3456u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_EFLAGS_ZF, 0u);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, direct,
        sizeof(direct)) != LIB_STATUS_OK || core_machine_memory_write(machine,
        0x4000u, &selector, sizeof(selector)) != LIB_STATUS_OK ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ECX, 0xffffffffu, 0u)) || (!timing_80286_resume(
        machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 16u)) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_ECX, 0xffffu, 0xffffu) ||
        !timing_80286_register_matches(machine,
        CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_EFLAGS_ZF, CORE_MACHINE_DEBUG_EFLAGS_ZF);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, indexed,
        sizeof(indexed)) != LIB_STATUS_OK || core_machine_memory_write(machine,
        0x4001u, &selector, sizeof(selector)) != LIB_STATUS_OK ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EBP, 0xffffffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffffffu, 0u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ECX, 0xffffffffu, 0u)) ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 17u)) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_ECX, 0xffffu, 0xffffu) ||
        !timing_80286_register_matches(machine,
        CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_EFLAGS_ZF, CORE_MACHINE_DEBUG_EFLAGS_ZF);
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_smsw(void)
{
    static const lib_u8 register_form[] = { 0x0fu,0x01u,0xe0u };
    static const lib_u8 direct[] = { 0x0fu,0x01u,0x26u,0x00u,0x10u };
    static const lib_u8 indexed[] = { 0x0fu,0x01u,0x62u,0x01u };
    lib_u16 msw;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, register_form,
        sizeof(register_form)) || (!timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_CR0, 0xffffffffu, 0x000cu) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0u)) || !timing_80286_run(machine,
        &state, 1u, 2u) || !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EAX, 0xffffu, 0xfffcu);
    if (!failed) failed |= !timing_80286_load(machine, direct, sizeof(direct)) ||
        (!timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_CR0, 0xffffffffu, 0x000cu)) || !timing_80286_run(machine,
        &state, 1u, 3u) || core_machine_memory_read(machine, 0x1000u, &msw,
        sizeof(msw)) != LIB_STATUS_OK || msw != 0xfffcu;
    if (!failed) failed |= !timing_80286_load(machine, indexed, sizeof(indexed)) ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_CR0, 0xffffffffu, 0x000cu) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EBP, 0xffffffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffffffu, 0u)) || !timing_80286_run(machine,
        &state, 1u, 4u) || core_machine_memory_read(machine, 0x1001u, &msw,
        sizeof(msw)) != LIB_STATUS_OK || msw != 0xfffcu;
    if (!failed) failed |= !timing_80286_boot_protected(machine, &state) ||
        core_machine_memory_write(machine, 0x2000u, register_form,
        sizeof(register_form)) != LIB_STATUS_OK || (!timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0u)) || (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 2u)) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EAX, 0xffffu, 0xfff1u);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, direct,
        sizeof(direct)) != LIB_STATUS_OK || (!timing_80286_resume(
        machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 3u)) ||
        core_machine_memory_read(machine, 0x4000u, &msw, sizeof(msw)) !=
        LIB_STATUS_OK || msw != 0xfff1u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, indexed,
        sizeof(indexed)) != LIB_STATUS_OK || (!timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_EBP, 0xffffffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffffffu, 0u)) || (!timing_80286_resume(
        machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 4u)) ||
        core_machine_memory_read(machine, 0x4001u, &msw, sizeof(msw)) !=
        LIB_STATUS_OK || msw != 0xfff1u;
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_sldt_str(void)
{
    static const lib_u8 sldt_register[] = { 0x0fu,0x00u,0xc0u };
    static const lib_u8 str_direct[] = { 0x0fu,0x00u,0x0eu,0x00u,0x10u };
    static const lib_u8 sldt_indexed[] = { 0x0fu,0x00u,0x42u,0x01u };
    lib_u16 selector;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state) ||
        !timing_80286_boot_protected(machine, &state);

    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        sldt_register, sizeof(sldt_register)) != LIB_STATUS_OK ||
        (!timing_80286_load_system_selector(machine, LIB_FALSE) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0u)) ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 2u)) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EAX, 0xffffu, 0x0010u);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        str_direct, sizeof(str_direct)) != LIB_STATUS_OK ||
        (!timing_80286_load_system_selector(machine, LIB_TRUE)) ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 3u)) ||
        core_machine_memory_read(machine, 0x4000u, &selector, sizeof(selector)) !=
        LIB_STATUS_OK || selector != 0x0008u;
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u,
        sldt_indexed, sizeof(sldt_indexed)) != LIB_STATUS_OK ||
        (!timing_80286_load_system_selector(machine, LIB_FALSE) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EBP, 0xffffffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffffffu, 0u)) || (!timing_80286_resume(
        machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 4u)) ||
        core_machine_memory_read(machine, 0x4001u, &selector, sizeof(selector)) !=
        LIB_STATUS_OK || selector != 0x0010u;
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_lmsw(void)
{
    static const lib_u8 register_form[] = { 0x0fu,0x01u,0xf0u };
    static const lib_u8 direct[] = { 0x0fu,0x01u,0x36u,0x00u,0x10u };
    static const lib_u8 indexed[] = { 0x0fu,0x01u,0x72u,0x01u };
    const lib_u16 protected_msw = 0x0001u;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, register_form,
        sizeof(register_form)) || (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, 0u)) ||
        !timing_80286_run(machine, &state, 1u, 3u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_CR0, 0xffffu, 0u);
    if (!failed) failed |= !timing_80286_load(machine, direct, sizeof(direct)) ||
        core_machine_memory_write(machine, 0x1000u, &protected_msw,
        sizeof(protected_msw)) != LIB_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 6u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_CR0, 0xffffu, 0x0001u);
    if (!failed) failed |= !timing_80286_boot_protected(machine, &state) ||
        core_machine_memory_write(machine, 0x2000u, register_form,
        sizeof(register_form)) != LIB_STATUS_OK ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, protected_msw)) ||
        (!timing_80286_resume(machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 3u)) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_CR0, 0xffffu, 0x0001u);
    if (!failed) failed |= core_machine_memory_write(machine, 0x2000u, indexed,
        sizeof(indexed)) != LIB_STATUS_OK || core_machine_memory_write(machine,
        0x4001u, &protected_msw, sizeof(protected_msw)) != LIB_STATUS_OK ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EBP, 0xffffffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffffffu, 0u)) || (!timing_80286_resume(
        machine, 0u) ||
        !timing_80286_run(machine, &state, 1u, 7u)) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_CR0, 0xffffu, 0x0001u);
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_table_control(void)
{
    static const lib_u8 sgdt[] = { 0x0fu,0x01u,0x06u,0x00u,0x10u };
    static const lib_u8 sidt[] = { 0x0fu,0x01u,0x0eu,0x00u,0x10u };
    static const lib_u8 lgdt[] = { 0x0fu,0x01u,0x16u,0x00u,0x10u };
    static const lib_u8 lidt[] = { 0x0fu,0x01u,0x1eu,0x00u,0x10u };
    static const lib_u8 clts[] = { 0x0fu,0x06u };
    static const lib_u8 table[] = { 0x57u,0x13u,0x00u,0x34u,0x12u,0u };
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine_debug_cpu_snapshot snapshot;
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, sgdt, sizeof(sgdt)) ||
        (!timing_80286_load_table(machine, LIB_FALSE)) ||
        !timing_80286_run(machine, &state, 1u, 11u);
    if (!failed) failed |= !timing_80286_load(machine, sidt, sizeof(sidt)) ||
        (!timing_80286_load_table(machine, LIB_TRUE)) ||
        !timing_80286_run(machine, &state, 1u, 12u);
    if (!failed) failed |= !timing_80286_load(machine, lgdt, sizeof(lgdt)) ||
        core_machine_memory_write(machine, 0x1000u, table, sizeof(table)) !=
            LIB_STATUS_OK || !timing_80286_run(machine, &state, 1u, 11u) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.gdtr.limit != 0x1357u ||
        snapshot.gdtr.base != 0x00123400u);
    if (!failed) failed |= !timing_80286_load(machine, lidt, sizeof(lidt)) ||
        core_machine_memory_write(machine, 0x1000u, table, sizeof(table)) !=
            LIB_STATUS_OK || !timing_80286_run(machine, &state, 1u, 12u) ||
        (!timing_80286_capture(machine, &snapshot) ||
        snapshot.idtr.limit != 0x1357u ||
        snapshot.idtr.base != 0x00123400u);
    if (!failed) failed |= !timing_80286_load(machine, clts, sizeof(clts)) ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_CR0, 0xffffffffu, VCPU_CR0_TS)) ||
        !timing_80286_run(machine, &state, 1u, 2u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_CR0, VCPU_CR0_TS, 0u);
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_ldt_task_registers(void)
{
    static const lib_u8 lldt_register[] = { 0x0fu,0x00u,0xd0u };
    static const lib_u8 lldt_memory[] = { 0x0fu,0x00u,0x16u,0x00u,0x04u };
    static const lib_u8 ltr_register[] = { 0x0fu,0x00u,0xd8u };
    static const lib_u8 ltr_memory[] = { 0x0fu,0x00u,0x1eu,0x00u,0x04u };
    const lib_u16 ldt_selector = 0x0028u;
    const lib_u16 tss_selector = 0x0030u;
    const lib_u8 *forms[] = { lldt_register, lldt_memory,
        ltr_register, ltr_memory };
    const lib_size sizes[] = { sizeof(lldt_register), sizeof(lldt_memory),
        sizeof(ltr_register), sizeof(ltr_memory) };
    const lib_u16 selectors[] = { ldt_selector, ldt_selector,
        tss_selector, tss_selector };
    const lib_u64 ticks[] = { 17u, 19u, 17u, 19u };
    lib_size index;
    lib_i32 failed = 0;

    for (index = 0u; !failed && index < sizeof(forms) / sizeof(forms[0]); ++index) {
        timing_80286_state state = { 0u, 0u, 0u };
        core_machine_debug_cpu_snapshot snapshot;
        core_machine *machine = LIB_NULL;

        if (!timing_80286_prepare(&machine, &state) ||
            !timing_80286_boot_protected_system(machine, &state)) {
            core_machine_destroy(machine);
            return 1;
        }
        failed |= core_machine_memory_write(machine, 0x2000u, forms[index],
            sizes[index]) != LIB_STATUS_OK ||
            (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffffffu, selectors[index])) ||
            (index & 1u && core_machine_memory_write(machine, 0x3400u,
                &selectors[index], sizeof(selectors[index])) != LIB_STATUS_OK) ||
            (!timing_80286_resume(machine, 0u) ||
            !timing_80286_run(machine, &state, 1u, ticks[index])) ||
            !timing_80286_capture(machine, &snapshot) ||
            (index < 2u ? snapshot.ldtr.selector != ldt_selector :
                snapshot.tr.selector != tss_selector);
        core_machine_destroy(machine);
    }
    return failed;
}

static lib_i32 timing_80286_fpu_interface_transfer(void)
{
    static const lib_u8 fninit[] = { 0xdbu,0xe3u };
    static const lib_u8 fwait[] = { 0x9bu };
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, fninit, sizeof(fninit)) ||
        !timing_80286_run(machine, &state, 1u, 1u);
    if (!failed) failed |= !timing_80286_load(machine, fwait, sizeof(fwait)) ||
        !timing_80286_run(machine, &state, 1u, 3u);
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_memory(void)
{
    static const lib_u8 direct_read[] = { 0x8bu, 0x0eu, 0x00u, 0x10u };
    static const lib_u8 direct_write[] = { 0x89u, 0x0eu, 0x00u, 0x10u };
    static const lib_u8 indexed_odd_read[] = { 0x8bu, 0x4au, 0x01u };
    static const lib_u8 moffs_read[] = { 0xa1u, 0x01u, 0x10u };
    static const lib_u8 moffs_write[] = { 0xa3u, 0x01u, 0x10u };
    static const lib_u8 xlat[] = { 0xd7u };
    static const lib_u8 sreg_store_even[] = { 0x8cu, 0x1eu, 0x00u, 0x10u };
    static const lib_u8 sreg_store_odd[] = { 0x8cu, 0x1eu, 0x01u, 0x10u };
    static const lib_u8 sreg_store_indexed[] = { 0x8cu, 0x5au, 0x01u };
    const lib_u16 value = 0x5aa5u;
    const lib_u16 sreg_value = 0x1357u;
    lib_u16 sreg_read = 0u;
    const lib_u8 xlat_value = 0xa5u;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, direct_read,
        sizeof(direct_read)) || core_machine_memory_write(machine, 0x1000u,
        &value, sizeof(value)) != LIB_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 5u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_ECX, 0xffffu, value);
    if (!failed) failed |= !timing_80286_load(machine, direct_write,
        sizeof(direct_write)) || (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ECX, 0xffffu, value)) ||
        !timing_80286_run(machine, &state, 1u, 3u);
    if (!failed) failed |= !timing_80286_load(machine, indexed_odd_read,
        sizeof(indexed_odd_read)) || (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EBP, 0xffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffu, 0u)) ||
        core_machine_memory_write(machine, 0x1001u, &value, sizeof(value)) !=
            LIB_STATUS_OK || !timing_80286_run(machine, &state, 1u, 8u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_ECX, 0xffffu, value);
    if (!failed) failed |= !timing_80286_load(machine, moffs_read,
        sizeof(moffs_read)) || core_machine_memory_write(machine, 0x1001u,
        &value, sizeof(value)) != LIB_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 7u);
    if (!failed) failed |= !timing_80286_load(machine, moffs_write,
        sizeof(moffs_write)) || (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffffu, value)) ||
        !timing_80286_run(machine, &state, 1u, 5u);
    if (!failed) failed |= !timing_80286_load(machine, xlat, sizeof(xlat)) ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EBX, 0xffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EAX, 0xffu, 1u)) ||
        core_machine_memory_write(machine, 0x1001u, &xlat_value,
            sizeof(xlat_value)) != LIB_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 5u) ||
        !timing_80286_register_matches(machine, CORE_MACHINE_DEBUG_EAX, 0xffu, xlat_value);
    if (!failed) failed |= !timing_80286_load(machine, sreg_store_even,
        sizeof(sreg_store_even)) || (!timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_DS, 0xffffffffu, sreg_value)) || !timing_80286_run(machine, &state, 1u, 3u) ||
        core_machine_memory_read(machine, ((lib_u32)sreg_value << 4u) + 0x1000u, &sreg_read,
            sizeof(sreg_read)) != LIB_STATUS_OK || sreg_read != sreg_value;
    if (!failed) failed |= !timing_80286_load(machine, sreg_store_odd,
        sizeof(sreg_store_odd)) || (!timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_DS, 0xffffffffu, sreg_value)) || !timing_80286_run(machine, &state, 1u, 5u) ||
        core_machine_memory_read(machine, ((lib_u32)sreg_value << 4u) + 0x1001u, &sreg_read,
            sizeof(sreg_read)) != LIB_STATUS_OK || sreg_read != sreg_value;
    if (!failed) failed |= !timing_80286_load(machine, sreg_store_indexed,
        sizeof(sreg_store_indexed)) || (!timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_DS, 0xffffffffu, sreg_value) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EBP, 0xffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffu, 0u)) ||
        !timing_80286_run(machine, &state, 1u, 6u) ||
        core_machine_memory_read(machine, 0x1001u, &sreg_read,
            sizeof(sreg_read)) != LIB_STATUS_OK || sreg_read != sreg_value;
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_control_ports(void)
{
    static const lib_u8 taken[] = { 0x74u, 0x01u, 0x90u, 0x90u };
    static const lib_u8 taken_zero[] = { 0x74u, 0x00u, 0x90u };
    static const lib_u8 not_taken[] = { 0x75u, 0x01u, 0x90u, 0x90u };
    static const lib_u8 leave[] = { 0xc9u };
    static const lib_u8 movsb[] = { 0xa4u };
    static const lib_u8 rep[] = { 0xf3u, 0xa4u };
    static const lib_u8 source[] = { 1u, 2u, 3u };
    static const lib_u8 out_imm[] = { 0xe6u, 0xe0u };
    static const lib_u8 out_dx[] = { 0xeeu };
    static const lib_u8 in_imm[] = { 0xe4u, 0xe0u };
    static const lib_u8 in_dx[] = { 0xecu };
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, taken, sizeof(taken)) ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EFLAGS, 0u, CORE_MACHINE_DEBUG_EFLAGS_ZF)) ||
        !timing_80286_run(machine, &state, 1u, 7u);
    if (!failed) failed |= !timing_80286_load(machine, taken_zero,
        sizeof(taken_zero)) || (!timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_EFLAGS, 0u, CORE_MACHINE_DEBUG_EFLAGS_ZF)) ||
        !timing_80286_run(machine, &state, 1u, 7u);
    if (!failed) failed |= !timing_80286_load(machine, not_taken,
        sizeof(not_taken)) || (!timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_EFLAGS, 0u, CORE_MACHINE_DEBUG_EFLAGS_ZF)) || !timing_80286_run(machine, &state, 1u, 3u);
    if (!failed) failed |= !timing_80286_load(machine, leave, sizeof(leave)) ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EBP,
        0xffffu, 0x2ffeu) || !timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_ESP, 0xffffu, 0x7777u)) || core_machine_memory_write(
        machine, 0x2ffeu, source, 2u) != LIB_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 5u);
    if (!failed) failed |= !timing_80286_load(machine, movsb, sizeof(movsb)) ||
        core_machine_memory_write(machine, 0x1000u, source, 1u) != LIB_STATUS_OK ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EDI, 0xffffu, 0x1100u)) ||
        !timing_80286_run(machine, &state, 1u, 5u);
    if (!failed) failed |= !timing_80286_load(machine, rep, sizeof(rep)) ||
        core_machine_memory_write(machine, 0x1000u, source, sizeof(source)) !=
            LIB_STATUS_OK || (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ECX, 0xffffu, 3u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EDI, 0xffffu, 0x1100u)) ||
        !timing_80286_run(machine, &state, 3u, 17u);
    if (!failed) failed |= !timing_80286_load(machine, out_imm, sizeof(out_imm)) ||
        !timing_80286_run(machine, &state, 1u, 3u) || state.writes != 1u;
    if (!failed) failed |= !timing_80286_load(machine, out_dx, sizeof(out_dx)) ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EDX, 0xffffu, 0x00e0u)) ||
        !timing_80286_run(machine, &state, 1u, 3u) || state.writes != 2u;
    if (!failed) failed |= !timing_80286_load(machine, in_imm, sizeof(in_imm)) ||
        !timing_80286_run(machine, &state, 1u, 5u) || state.reads != 1u;
    if (!failed) failed |= !timing_80286_load(machine, in_dx, sizeof(in_dx)) ||
        (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EDX, 0xffffu, 0x00e0u)) ||
        !timing_80286_run(machine, &state, 1u, 5u) || state.reads != 2u;
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_80286_boundaries(void)
{
    static const lib_u8 nop[] = { 0x90u };
    static const lib_u8 shift_byte[] = { 0xd0u, 0xc0u };
    static const lib_u8 shift_word[] = { 0xd1u, 0xc0u };
    static const lib_u8 shift_memory[] = { 0xd0u, 0x06u, 0x00u, 0x10u };
    static const lib_u8 shift_word_memory[] = { 0xd1u, 0x06u, 0x00u, 0x10u };
    static const lib_u8 shift_indexed_memory[] = { 0xd0u, 0x4au, 0x01u };
    static const lib_u8 shift_cl[] = { 0xd2u, 0xc0u };
    static const lib_u8 shift_count[] = { 0xc1u, 0xc0u, 0x04u };
    static const lib_u8 shift_cl_memory[] = { 0xd2u, 0x06u, 0x00u, 0x10u };
    static const lib_u8 shift_count_memory[] = { 0xc1u, 0x4au, 0x01u, 0x04u };
    static const lib_u8 shift_undefined[] = { 0xd0u, 0xf0u };
    static const lib_u8 fault[] = { 0x66u, 0x90u };
    static const lib_u8 maximum[] = { 0xf3u, 0xa4u };
    static const lib_u8 source[] = { 0x78u };
    const core_machine_run_budget one = { 1u, 0u };
    const core_machine_run_budget insufficient = { 1u, 8u };
    core_machine_run_result result;
    timing_80286_state state = { 0u, 0u, 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_80286_prepare(&machine, &state);

    if (!failed) failed |= !timing_80286_load(machine, shift_byte,
        sizeof(shift_byte)) || !timing_80286_run(machine, &state, 1u, 2u);
    if (!failed) failed |= !timing_80286_load(machine, shift_word,
        sizeof(shift_word)) || !timing_80286_run(machine, &state, 1u, 2u);
    if (!failed) failed |= !timing_80286_load(machine, shift_memory,
        sizeof(shift_memory)) || !timing_80286_run(machine, &state, 1u, 7u);
    if (!failed) failed |= !timing_80286_load(machine, shift_word_memory,
        sizeof(shift_word_memory)) || !timing_80286_run(machine, &state, 1u, 7u);
    if (!failed) failed |= !timing_80286_load(machine, shift_indexed_memory,
        sizeof(shift_indexed_memory)) || (!timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_EBP, 0xffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffu, 0u)) || core_machine_memory_write(
            machine, 0x1001u, source, 1u) != LIB_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 8u);
    if (!failed) failed |= !timing_80286_load(machine, shift_cl,
        sizeof(shift_cl)) || (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ECX, 0xffffu, 4u)) ||
        !timing_80286_run(machine, &state, 1u, 9u);
    if (!failed) failed |= !timing_80286_load(machine, shift_count,
        sizeof(shift_count)) || !timing_80286_run(machine, &state, 1u, 9u);
    if (!failed) failed |= !timing_80286_load(machine, shift_cl_memory,
        sizeof(shift_cl_memory)) || (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ECX, 0xffffu, 4u)) ||
        !timing_80286_run(machine, &state, 1u, 12u);
    if (!failed) failed |= !timing_80286_load(machine, shift_count_memory,
        sizeof(shift_count_memory)) || (!timing_80286_patch_register(machine,
        CORE_MACHINE_DEBUG_EBP, 0xffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffu, 0u)) || core_machine_memory_write(
            machine, 0x1001u, source, 1u) != LIB_STATUS_OK ||
        !timing_80286_run(machine, &state, 1u, 13u);
    if (!failed) failed |= !timing_80286_load(machine, shift_cl,
        sizeof(shift_cl)) || (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ECX, 0xffffu, 0x24u)) ||
        !timing_80286_run(machine, &state, 1u, 9u);
    if (!failed) failed |= !timing_80286_load(machine, shift_undefined,
        sizeof(shift_undefined)) || core_machine_run(machine, one, &result) != LIB_STATUS_INTERNAL_ERROR ||
        result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
        result.ticks != 0u || state.advanced_ticks != 0u;
    if (!failed) failed |= !timing_80286_load(machine, fault, sizeof(fault)) ||
        core_machine_run(machine, one, &result) != LIB_STATUS_INTERNAL_ERROR ||
        result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
        result.ticks != 0u || state.advanced_ticks != 0u;
    if (!failed) failed |= !timing_80286_load(machine, nop, sizeof(nop)) ||
        !timing_80286_run(machine, &state, 1u, 3u) ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        machine->elapsed_ticks != 0u || state.advanced_ticks != 0u ||
        !timing_80286_load(machine, nop, sizeof(nop)) ||
        core_machine_request_stop(machine) != LIB_STATUS_OK ||
        core_machine_run(machine, one, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_REQUESTED || result.executed != 0u ||
        result.ticks != 0u || state.advanced_ticks != 0u;
    if (!failed) failed |= !timing_80286_load(machine, maximum,
        sizeof(maximum)) || (!timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ECX, 0xffffu, 1u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_ESI, 0xffffu, 0x1000u) ||
        !timing_80286_patch_register(machine, CORE_MACHINE_DEBUG_EDI, 0xffffu, 0x1100u)) ||
        core_machine_memory_write(machine, 0x1000u, source, sizeof(source)) !=
            LIB_STATUS_OK ||
        core_machine_run(machine, insufficient, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
        result.ticks != 0u || !timing_80286_run(machine, &state, 1u, 9u);
    if (!failed) failed |= !timing_80286_load(machine, nop, sizeof(nop));
    if (!failed) {
        machine->elapsed_ticks = LIB_UINT64_MAX - 2u;
        state.advanced_ticks = 0u;
        failed |= core_machine_run(machine, one, &result) != LIB_STATUS_INTERNAL_ERROR ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            result.ticks != 0u || machine->elapsed_ticks != LIB_UINT64_MAX - 2u ||
            state.advanced_ticks != 0u;
    }
    core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    static const lib_u8 nop[] = { 0x90u };
    static const lib_u8 clc[] = { 0xf8u };
    static const lib_u8 cmc[] = { 0xf5u };
    static const lib_u8 stc[] = { 0xf9u };
    static const lib_u8 cld[] = { 0xfcu };
    static const lib_u8 std[] = { 0xfdu };
    static const lib_u8 cli[] = { 0xfau };
    static const lib_u8 sti[] = { 0xfbu };
    static const lib_u8 sahf[] = { 0x9eu };
    static const lib_u8 lahf[] = { 0x9fu };
    static const lib_u8 immediate[] = { 0xb8u, 0x34u, 0x12u };
    static const lib_u8 registers[] = { 0x8bu, 0xc1u };

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
    lib_c_printf("80286-INSTRUCTION-TIMING-LEDGER:OK\n");
    return 0;
}
