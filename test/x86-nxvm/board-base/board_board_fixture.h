#ifndef TEST_X86_NXVM_MACHINE_BOARD_FIXTURE_H
#define TEST_X86_NXVM_MACHINE_BOARD_FIXTURE_H

#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "composition/debug_fixture.h"
#include "composition/bus_fixture.h"

/* PC qualification supplies a complete RAM-backed reset-fetch window. Keep
 * this Board setup independent of the shared package's unit fixtures. */
static inline lib_status test_x86_nxvm_machine_fixture_register_reset_mapping(
    x86_core_machine *machine, lib_u32 linear, lib_u32 physical, lib_size bytes)
{
    x86_core_cpu_profile profile;
    x86_core_machine_memory_alias_config aliases[2] = {
        {linear, physical, bytes < 15u ? 15u : bytes}
    };
    lib_size count = 1u;
    const lib_status status = x86_core_machine_get_cpu_profile(machine, &profile);

    if (status != LIB_STATUS_OK) return status;
    if (linear == 0xfffffff0u && profile <= X86_CORE_CPU_PROFILE_80286) {
        aliases[count++] = (x86_core_machine_memory_alias_config){
            profile <= X86_CORE_CPU_PROFILE_80186 ? 0x000ffff0u : 0x00fffff0u,
            physical, aliases[0].bytes
        };
    }
    return x86_core_machine_install_memory_aliases(machine, aliases, count, LIB_FALSE);
}

static inline lib_i32 test_x86_nxvm_machine_fixture_nmi_prepare(x86_core_machine *machine)
{
    const lib_u8 program[] = {0x90u, 0x90u, 0xf4u};
    const lib_u8 handler = 0xf4u;
    const lib_u8 vector[] = {0x00u, 0x02u, 0x00u, 0x00u};
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP),
        .values = {[X86_CORE_CPU_DEBUG_ESP] = 0x1000u}
    };

    return x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0u, program, sizeof(program)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 8u, vector, sizeof(vector)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x200u, &handler, sizeof(handler)) != LIB_STATUS_OK;
}

static inline lib_i32 test_x86_nxvm_machine_fixture_nmi_execute(x86_core_machine *machine, lib_bool delivered)
{
    x86_core_machine_run_result result;
    x86_core_cpu_state state;

    /* Prove the CPU consumes the board signal through vector 2, rather than
     * reading its private pending bit. Masked execution must stay at the NOP. */
    return x86_core_machine_run(machine,
            (x86_core_machine_run_budget){delivered ? 4u : 1u, 0u}, &result) != LIB_STATUS_OK ||
        x86_core_machine_get_cpu_state(machine, &state) != LIB_STATUS_OK ||
        state.cs != 0u || state.eip != (delivered ? 0x201u : 1u) ||
        state.halted != delivered || result.reason != (delivered ?
            X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT : X86_CORE_MACHINE_STOP_BUDGET);
}

/* Public Core/Board operations shared by repository-only test receivers. */
/* This is the lifecycle tail after owner-local construction/setup. */
static inline lib_i32 test_x86_nxvm_machine_fixture_bind_freeze_reset(
    x86_core_machine *machine, const x86_core_machine_execution_provider *provider,
    void *provider_owner)
{
    return x86_core_machine_bind_execution_provider(machine, provider,
        provider_owner) == LIB_STATUS_OK &&
        x86_core_machine_freeze_execution_providers(machine) == LIB_STATUS_OK &&
        x86_core_machine_reset(machine) == LIB_STATUS_OK;
}

/* A deliberately receiverless protected exception is architectural CPU
 * shutdown, not a host/provider failure.  Core exposes that state as a
 * waiting boundary so board receivers retain control of reset/NMI policy. */
static inline lib_i32 test_x86_nxvm_machine_fixture_shutdown_wait(
    lib_status status, const x86_core_machine_run_result *result)
{
    return status == LIB_STATUS_OK && result != LIB_NULL &&
        result->reason == X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT &&
        result->detail == VCPUINS_EXCEPT_SHUTDOWN;
}

/*
 * This preserves the corpus' established short-circuit lifecycle: it does
 * not add cleanup or validation policy.  Owner smokes retain all device and
 * instruction-specific setup before or after this fixed sequence.
 */
static inline lib_i32 test_x86_nxvm_machine_fixture_create_bind_freeze_reset(
    const x86_nxvm_board_config *config,
    const x86_core_machine_execution_provider *provider, void *provider_owner,
    x86_core_machine **out_machine,
    x86_nxvm_board_state **out_board)
{
    return x86_nxvm_board_create(config, out_machine, out_board) == LIB_STATUS_OK &&
        test_x86_nxvm_machine_fixture_bind_freeze_reset(*out_machine, provider,
            provider_owner);
}


static inline lib_status test_x86_nxvm_machine_fixture_register_memory_device_provider(
    x86_core_machine *machine, lib_u32 physical, lib_size bytes,
    x86_core_machine_memory_device_read read,
    x86_core_machine_memory_device_write write,
    x86_core_machine_memory_device_query query, void *owner)
{
    const x86_core_machine_memory_device_route route = {
        .physical_start = physical,
        .bytes = bytes,
        .callbacks = {read, write, query},
        .mode = X86_CORE_MACHINE_MEMORY_PROVIDER_STANDARD,
        .failure = LIB_NULL
    };
    return machine == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        x86_core_machine_install_memory_device_routes(machine, &route, 1u,
            LIB_NULL, LIB_NULL, owner);
}

#endif
