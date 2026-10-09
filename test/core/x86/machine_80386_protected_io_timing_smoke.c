#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "core/x86/machine_interface.h"
#include "debug_fixture.h"
#include "port_mode_fixture.h"

#define TIMING_RESET_LINEAR 0xfffffff0u
#define TIMING_RESET_PHYSICAL 0x000ffff0u

typedef struct timing_state {
    lib_u32 reads;
    lib_u32 writes;
    lib_u64 advanced_ticks;
    lib_u64 setup_ticks;
} timing_state;

typedef struct timing_form {
    lib_u8 opcode;
    lib_u64 protected_ticks;
    lib_u64 permission_ticks;
    lib_i32 input;
} timing_form;

static lib_status timing_port_read(void *owner, lib_u16 port, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)tick;
    timing_state *state = (timing_state *)owner;

    if (state == LIB_NULL || out_value == LIB_NULL || port != 0x00e0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    ++state->reads;
    *out_value = 0x5au;
    return LIB_STATUS_OK;
}

static lib_status timing_port_write(void *owner, lib_u16 port,
    lib_u32 value)
{
    timing_state *state = (timing_state *)owner;

    if (state == LIB_NULL || port != 0x00e0u || value > 0xffffu)
        return LIB_STATUS_INVALID_ARGUMENT;
    ++state->writes;
    return LIB_STATUS_OK;
}

static const core_machine_port_provider timing_ports = {
    timing_port_read, timing_port_write
};

static void timing_reset(void *opaque)
{
    timing_state *state = (timing_state *)opaque;
    if (state != LIB_NULL) {
        state->advanced_ticks = 0u;
        state->setup_ticks = 0u;
    }
}

static void timing_advance(void *opaque, lib_u64 ticks)
{
    timing_state *state = (timing_state *)opaque;
    if (state != LIB_NULL) state->advanced_ticks += ticks;
}

static const core_machine_execution_provider timing_execution = {
    timing_reset, timing_advance
};

static lib_i32 timing_is_shutdown(const core_machine_run_result *result)
{
    return result != LIB_NULL &&
        result->reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT &&
        result->detail == VCPUINS_EXCEPT_SHUTDOWN;
}

static lib_i32 timing_prepare(core_machine **out_machine, timing_state *state,
    const core_machine_external_cycle_timing *external_timing)
{
    core_machine_executor_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .ticks_per_instruction = 29u,
        .instruction_timing = { 29u, 7u, 31u, 37u, 41u, 43u }
    };
    const core_machine_memory_alias_config alias = {
        TIMING_RESET_LINEAR, TIMING_RESET_PHYSICAL, 16u
    };
    core_machine *machine = LIB_NULL;

    if (external_timing != LIB_NULL)
        config.transaction_contract.external_cycle_timing = *external_timing;
    if (out_machine == LIB_NULL || state == LIB_NULL ||
        core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_install_memory_aliases(machine, &alias, 1u, LIB_FALSE) !=
            LIB_STATUS_OK || core_machine_install_port_provider(machine,
            0x00e0u, 0x00e0u, &timing_ports, state) != LIB_STATUS_OK ||
        core_machine_bind_execution_provider(machine, &timing_execution,
            state) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static lib_i32 timing_load(core_machine *machine, timing_state *state,
    lib_u8 opcode, lib_i32 mode, lib_u8 bitmap)
{
    lib_u8 code[] = { opcode, 0xe0u };
    lib_size bytes = opcode >= 0xecu ? 1u : sizeof(code);

    return core_machine_reset(machine) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, TIMING_RESET_LINEAR, code, bytes) ==
            LIB_STATUS_OK &&
        test_core_80386_enter_port_mode(machine, mode, code, bytes, bitmap,
            &state->setup_ticks) && state->advanced_ticks == state->setup_ticks;
}

static lib_i32 timing_run_form(const timing_form *form, lib_i32 mode)
{
    timing_state state = { 0 };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine *machine = LIB_NULL;
    lib_u64 ticks = mode == 0 ? form->protected_ticks :
        form->permission_ticks;
    lib_i32 failed = !timing_prepare(&machine, &state, LIB_NULL) ||
        !timing_load(machine, &state, form->opcode, mode, 0u);

    if (!failed) {
        test_core_machine_fixture_write_register(machine, CORE_MACHINE_DEBUG_EAX, 0x11223344u);
        test_core_machine_fixture_write_register(machine, CORE_MACHINE_DEBUG_EDX, 0x000000e0u);
        failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            result.ticks != ticks || result.elapsed_ticks != state.setup_ticks + ticks ||
            state.advanced_ticks != state.setup_ticks + ticks ||
            (form->input ? state.reads != 1u || state.writes != 0u :
                state.reads != 0u || state.writes != 1u);
    }
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 timing_test_success(void)
{
    static const timing_form forms[] = {
        { 0xe4u, 6u, 26u, 1 }, { 0xe5u, 6u, 26u, 1 },
        { 0xecu, 7u, 27u, 1 }, { 0xedu, 7u, 27u, 1 },
        { 0xe6u, 4u, 24u, 0 }, { 0xe7u, 4u, 24u, 0 },
        { 0xeeu, 5u, 25u, 0 }, { 0xefu, 5u, 25u, 0 }
    };
    lib_size index;

    for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
        if (!timing_run_form(&forms[index], 0) ||
            !timing_run_form(&forms[index], 1) ||
            !timing_run_form(&forms[index], 2)) return 0;
    }
    return 1;
}

static lib_i32 timing_test_denied(void)
{
    timing_state state = { 0 };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_prepare(&machine, &state, LIB_NULL) ||
        !timing_load(machine, &state, 0xe4u, 1, 0x01u);
    if (!failed) {
        failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            !timing_is_shutdown(&result) || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != state.setup_ticks ||
            state.advanced_ticks != state.setup_ticks || state.reads != 0u || state.writes != 0u;
    }
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 timing_test_shutdown_external_wait(void)
{
    const core_machine_external_cycle_timing timing = {2048u, 1u, 0u,
        CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED, 0u, 0u};
    timing_state state = { 0 };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_prepare(&machine, &state, &timing) ||
        !timing_load(machine, &state, 0xe4u, 1, 0x01u);

    if (!failed) {
        failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            !timing_is_shutdown(&result) || result.executed != 0u ||
            state.reads != 0u || state.writes != 0u;
    }
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 timing_test_permission_strings(void)
{
    static const timing_form forms[] = {
        { 0x6cu, 0u, 0u, 1 }, { 0x6eu, 0u, 0u, 0 }
    };
    lib_i32 vm86;
    lib_size index;

    for (vm86 = 0; vm86 != 2; ++vm86) {
        for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
            lib_u8 bitmap;

            for (bitmap = 0u; bitmap != 2u; ++bitmap) {
                timing_state state = { 0 };
                const core_machine_run_budget budget = { 1u, 0u };
                core_machine_run_result result;
                core_machine *machine = LIB_NULL;
                lib_u8 source = 0x4au;
                lib_u8 destination = 0u;
                lib_i32 failed = !timing_prepare(&machine, &state, LIB_NULL) ||
                    !timing_load(machine, &state, forms[index].opcode,
                        vm86 ? 2 : 1, bitmap);

                if (!failed) {
                    test_core_machine_fixture_write_register(machine, CORE_MACHINE_DEBUG_EDX, 0x000000e0u);
                    test_core_machine_fixture_write_register(machine, CORE_MACHINE_DEBUG_ESI, 0x00000200u);
                    test_core_machine_fixture_write_register(machine, CORE_MACHINE_DEBUG_EDI, 0x00000100u);
                    failed |= core_machine_memory_write(machine, 0x0200u, &source,
                        sizeof(source)) != LIB_STATUS_OK;
                }
                if (!failed && bitmap == 0u) {
                    failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
                        result.reason != CORE_MACHINE_STOP_BUDGET ||
                        result.executed != 1u ||
                        (forms[index].input ? state.reads != 1u || state.writes != 0u ||
                            test_core_machine_fixture_read_register(machine, CORE_MACHINE_DEBUG_EDI) != 0x00000101u ||
                            core_machine_memory_read(machine, 0x0100u, &destination,
                                sizeof(destination)) != LIB_STATUS_OK || destination != 0x5au :
                            state.reads != 0u || state.writes != 1u ||
                            test_core_machine_fixture_read_register(machine, CORE_MACHINE_DEBUG_ESI) != 0x00000201u);
                }
                if (!failed && bitmap != 0u) {
                    failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
                        !timing_is_shutdown(&result) || result.executed != 0u ||
                        result.ticks != 0u || result.elapsed_ticks != state.setup_ticks ||
                        state.reads != 0u || state.writes != 0u ||
                        test_core_machine_fixture_read_register(machine, CORE_MACHINE_DEBUG_ESI) != 0x00000200u ||
                        test_core_machine_fixture_read_register(machine, CORE_MACHINE_DEBUG_EDI) != 0x00000100u;
                }
                core_machine_destroy(machine);
                if (failed) return 0;
            }
        }
    }
    return 1;
}

static lib_i32 timing_test_permission_budget(void)
{
    const core_machine_run_budget insufficient = { 1u, 26u };
    const core_machine_run_budget sufficient = { 1u, 106u };
    core_machine_run_result result;
    timing_state state = { 0 };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_prepare(&machine, &state, LIB_NULL) ||
        !timing_load(machine, &state, 0xecu, 1, 0u);

    if (!failed) test_core_machine_fixture_write_register(machine, CORE_MACHINE_DEBUG_EDX, 0x000000e0u);
    if (!failed) {
        failed |= core_machine_run(machine, insufficient, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != state.setup_ticks ||
            state.advanced_ticks != state.setup_ticks || state.reads != 0u;
    }
    if (!failed) {
        failed |= core_machine_run(machine, sufficient, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            result.ticks != 27u || result.elapsed_ticks != state.setup_ticks + 27u ||
            state.advanced_ticks != state.setup_ticks + 27u || state.reads != 1u;
    }
    core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!timing_test_success() || !timing_test_denied() ||
        !timing_test_shutdown_external_wait() ||
        !timing_test_permission_strings() ||
        !timing_test_permission_budget())
        return 1;
    lib_c_printf("80386-PROTECTED-IO-TIMING:OK\n");
    lib_c_printf("IO-PERMISSION:OK\n");
    return 0;
}
