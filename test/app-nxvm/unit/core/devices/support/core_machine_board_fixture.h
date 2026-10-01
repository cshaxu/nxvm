#ifndef TEST_CORE_MACHINE_BOARD_FIXTURE_H
#define TEST_CORE_MACHINE_BOARD_FIXTURE_H

#include "lib/types/types_interface.h"
#include "x86/chips/cpu/cpu.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/debug_interface.h"

static inline lib_i32 test_core_machine_fixture_nmi_prepare(core_machine *machine)
{
    const lib_u8 program[] = {0x90u, 0x90u, 0xf4u};
    const lib_u8 handler = 0xf4u;
    const lib_u8 vector[] = {0x00u, 0x02u, 0x00u, 0x00u};
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP),
        .values = {[CORE_MACHINE_DEBUG_ESP] = 0x1000u}
    };

    return core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0u, program, sizeof(program)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 8u, vector, sizeof(vector)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x200u, &handler, sizeof(handler)) != LIB_STATUS_OK;
}

static inline lib_i32 test_core_machine_fixture_nmi_execute(core_machine *machine, lib_bool delivered)
{
    core_machine_run_result result;
    core_machine_cpu_state state;

    /* Prove the CPU consumes the board signal through vector 2, rather than
     * reading its private pending bit. Masked execution must stay at the NOP. */
    return core_machine_run(machine,
            (core_machine_run_budget){delivered ? 4u : 1u, 0u}, &result) != LIB_STATUS_OK ||
        core_machine_get_cpu_state(machine, &state) != LIB_STATUS_OK ||
        state.cs != 0u || state.eip != (delivered ? 0x201u : 1u) ||
        state.halted != delivered || result.reason != (delivered ?
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT : CORE_MACHINE_STOP_BUDGET);
}

/* Board construction, physical routing and device wiring for board tests. */
/* This is the lifecycle tail after owner-local construction/setup. */
static inline lib_i32 test_core_machine_fixture_bind_freeze_reset(
    core_machine *machine, const core_machine_execution_provider *provider,
    void *provider_owner)
{
    return core_machine_bind_execution_provider(machine, provider,
        provider_owner) == LIB_STATUS_OK &&
        core_machine_freeze_execution_providers(machine) == LIB_STATUS_OK &&
        core_machine_reset(machine) == LIB_STATUS_OK;
}

/*
 * This preserves the corpus' established short-circuit lifecycle: it does
 * not add cleanup or validation policy.  Owner smokes retain all device and
 * instruction-specific setup before or after this fixed sequence.
 */
static inline lib_i32 test_core_machine_fixture_create_bind_freeze_reset(
    const core_machine_config *config,
    const core_machine_execution_provider *provider, void *provider_owner,
    core_machine **out_machine)
{
    return core_machine_create(config, out_machine) == LIB_STATUS_OK &&
        test_core_machine_fixture_bind_freeze_reset(*out_machine, provider,
            provider_owner);
}

static inline lib_status test_core_machine_fixture_register_reset_mapping(
    core_machine *machine, lib_u32 linear, lib_u32 physical,
    lib_size bytes)
{
    lib_status status;
    lib_size mapped_bytes;

    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    /* Instruction refresh can prefetch up to 15 bytes.  A reset fixture which
     * supplies a shorter program must still map that complete window; otherwise
     * the trailing fetch can escape a high-ROM alias before the first opcode. */
    mapped_bytes = bytes < 15u ? 15u : bytes;
    status = core_machine_memory_register_mapping(&machine->executor_memory, linear,
        physical, mapped_bytes, LIB_FALSE);
    /* The corpus names every reset fixture through the 80386 alias.  Each
     * earlier CPU fetches the same bytes through its narrower physical bus. */
    if (status == LIB_STATUS_OK &&
        machine->cpu_profile <= CORE_MACHINE_CPU_PROFILE_80286 &&
        linear == 0xfffffff0u) {
        status = core_machine_memory_register_mapping(&machine->executor_memory,
            machine->cpu_profile <= CORE_MACHINE_CPU_PROFILE_80186 ?
                0x000ffff0u : 0x00fffff0u, physical, mapped_bytes, LIB_FALSE);
    }
    return status;
}

static inline lib_status test_core_machine_fixture_register_memory_device_provider(
    core_machine *machine, lib_u32 physical, lib_size bytes,
    core_machine_memory_device_read read,
    core_machine_memory_device_write write,
    core_machine_memory_device_query query, void *owner)
{
    return machine == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        core_machine_memory_register_device_provider(&machine->executor_memory,
            physical, bytes, read, write, query, owner);
}

static inline void test_core_machine_fixture_program_pit_divider(
    core_machine *machine, lib_u8 control, lib_u16 divisor,
    x86_pit_output_provider output, void *owner)
{
    if (machine == LIB_NULL) return;
    x86_pit_set_output(machine->shared_pit.device, 0u, output, owner);
    core_machine_port_write(&machine->executor_port, 0x0043u, control);
    core_machine_port_write(&machine->executor_port, 0x0040u, divisor & 0xffu);
    core_machine_port_write(&machine->executor_port, 0x0040u, divisor >> 8u);
}

static inline lib_u8 test_core_machine_fixture_read_port(
    const core_machine *machine, lib_u16 address)
{
    return machine == LIB_NULL ? 0u : core_machine_port_read(
        (t_port *)&machine->executor_port, address);
}

static inline lib_status test_core_machine_fixture_query_configuration_memory_route(
    const core_machine *machine, lib_u32 physical, lib_size bytes,
    core_machine_memory_access access, core_machine_memory_route *out_route)
{
    return machine == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        core_machine_memory_query_physical(&machine->executor_memory, physical,
            bytes, access, out_route);
}

/* Preserve the production delivery/handler rounds for corpus tests whose
 * assertions observe the executed exception handler. */
static inline lib_status test_core_machine_fixture_run_after_delivery(
    core_machine *machine, core_machine_run_budget budget,
    core_machine_run_result *out_result)
{
    lib_status status = core_machine_run(machine, budget, out_result);
    core_machine_cpu_diagnostic diagnostic;
    lib_u8 delivered = LIB_FALSE;

    if (status == LIB_STATUS_OK && out_result != LIB_NULL &&
        out_result->reason == CORE_MACHINE_STOP_BUDGET &&
        core_machine_get_cpu_diagnostic(machine, &diagnostic) == LIB_STATUS_OK) {
        delivered = diagnostic.last_delivered_exception.valid;
    }
    if (delivered) {
        status = core_machine_run(machine, budget, out_result);
    }
    return status;
}

#endif
