#ifndef TEST_CORE_MACHINE_BOARD_FIXTURE_H
#define TEST_CORE_MACHINE_BOARD_FIXTURE_H

#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_interface.h"
#include "composition/debug_fixture.h"
#include "composition/bus_fixture.h"
#include "composition/memory_alias_fixture.h"

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

/* Public Core/Board operations shared by repository-only test receivers. */
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

/* A deliberately receiverless protected exception is architectural CPU
 * shutdown, not a host/provider failure.  Core exposes that state as a
 * waiting boundary so board receivers retain control of reset/NMI policy. */
static inline lib_i32 test_core_machine_fixture_shutdown_wait(
    lib_status status, const core_machine_run_result *result)
{
    return status == LIB_STATUS_OK && result != LIB_NULL &&
        result->reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT &&
        result->detail == VCPUINS_EXCEPT_SHUTDOWN;
}

/*
 * This preserves the corpus' established short-circuit lifecycle: it does
 * not add cleanup or validation policy.  Owner smokes retain all device and
 * instruction-specific setup before or after this fixed sequence.
 */
static inline lib_i32 test_core_machine_fixture_create_bind_freeze_reset(
    const core_machine_config *config,
    const core_machine_execution_provider *provider, void *provider_owner,
    core_machine **out_machine,
    core_machine_board_state **out_board)
{
    return core_machine_create(config, out_machine, out_board) == LIB_STATUS_OK &&
        test_core_machine_fixture_bind_freeze_reset(*out_machine, provider,
            provider_owner);
}


static inline lib_status test_core_machine_fixture_register_memory_device_provider(
    core_machine *machine, lib_u32 physical, lib_size bytes,
    core_machine_memory_device_read read,
    core_machine_memory_device_write write,
    core_machine_memory_device_query query, void *owner)
{
    const core_machine_memory_device_route route = {
        .physical_start = physical,
        .bytes = bytes,
        .callbacks = {read, write, query},
        .mode = CORE_MACHINE_MEMORY_PROVIDER_STANDARD
    };
    return machine == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        core_machine_install_memory_device_routes(machine, &route, 1u,
            LIB_NULL, LIB_NULL, owner);
}

#endif
