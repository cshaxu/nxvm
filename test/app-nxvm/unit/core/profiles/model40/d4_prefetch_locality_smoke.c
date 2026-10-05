#include "lib/types/types_interface.h"
#include "app-nxvm/profiles/model40/d4_platform.h"
#include "app-nxvm/profiles/model40/d4_platform_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "x86/core/debug_interface.h"
#include <stdio.h>

static lib_u32 prefetch_grants;

/* Advance through actual one-tick FNINIT retirements, not private time injection. */
static lib_status d4_advance(core_machine *machine, lib_u64 ticks)
{
    const lib_u8 instruction[] = {0xdbu, 0xe3u};
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
        .values = {[CORE_MACHINE_DEBUG_EIP] = 0x0200u}
    };
    for (lib_u64 tick = 0u; tick < ticks; ++tick) {
        core_machine_run_result result;
        if (core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x0200u, instruction, sizeof(instruction)) != LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){1u, 0u}, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u || result.ticks != 1u)
            return LIB_STATUS_INTERNAL_ERROR;
    }
    return LIB_STATUS_OK;
}

/* Only the outgoing scheduler call is redirected; CPU reservations remain real. */
void test_cpu_prefetch_grant(core_machine_cpu_execution_context *cpu)
{
    ++prefetch_grants;
    core_machine_cpu_execution_advance_prefetch_reservation(cpu);
}

typedef struct d4_refresh_trace {
    lib_u32 requests;
    lib_u32 acknowledgements;
    lib_u32 commits;
    lib_u32 releases;
} d4_refresh_trace;

static void d4_trace(void *owner, const core_machine_trace_event *event)
{
    d4_refresh_trace *trace = owner;
    if ((event->detail & 0xffu) != CORE_MACHINE_TRANSACTION_OWNER_REFRESH) return;
    if (event->type == CORE_MACHINE_TRACE_TRANSACTION_HOLD_REQUEST) ++trace->requests;
    if (event->type == CORE_MACHINE_TRACE_TRANSACTION_HOLD_ACKNOWLEDGE) ++trace->acknowledgements;
    if (event->type == CORE_MACHINE_TRACE_TRANSACTION_HOLD_RELEASE) ++trace->releases;
    if (event->type == CORE_MACHINE_TRACE_TRANSACTION_COMMIT &&
        ((event->detail >> 8u) & 0xffu) == CORE_MACHINE_TRANSACTION_REFRESH_MEMORY_CYCLE)
        ++trace->commits;
}

lib_i32 main(void)
{
    const core_machine_d4_platform_config d4 = {CORE_MACHINE_PC_AT_PORT_B, 0u};
    core_machine_config config = {0};
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_d4_platform *platform = LIB_NULL;
    d4_refresh_trace trace = {0};
    const core_machine_trace_provider provider = {d4_trace, &trace};
    lib_u8 address = 0xffu;
    lib_i32 failed;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    config.transaction_contract.cpu_cycle_bus_ready_gate_enabled = LIB_TRUE;
    config.transaction_contract.cpu_prefetch_reservation_enabled = LIB_TRUE;
    config.auxiliary_pit_present = LIB_TRUE;
    config.auxiliary_pit_base_port = 0x48u;
    failed = core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_d4_platform_attach(board, &d4, &platform) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_set_trace_provider(machine, &provider) != LIB_STATUS_OK;
    if (!failed) {
        /* Model40 owns this pending latch; Core remains opaque here. */
        prefetch_grants = 0u;
        failed |= d4_advance(machine, 1u) != LIB_STATUS_OK ||
            prefetch_grants != 1u;
        platform->d4_refresh_hold_pending = LIB_TRUE;
        failed |= d4_advance(machine, 1u) != LIB_STATUS_OK ||
            prefetch_grants != 1u;
        failed |= d4_advance(machine, 1u) != LIB_STATUS_OK ||
            prefetch_grants != 2u;
        failed |= core_machine_reset(machine) != LIB_STATUS_OK;
        prefetch_grants = 0u;
        failed |= d4_advance(machine, 1u) != LIB_STATUS_OK ||
            prefetch_grants != 1u;
        failed |= core_machine_reset(machine) != LIB_STATUS_OK;
        trace = (d4_refresh_trace){0};
        failed |= core_machine_d4_platform_refresh_request(platform, &address);
        /* Retain the original 20-tick interval spanning the real PIT refresh edge. */
        failed |= d4_advance(machine, 20u) != LIB_STATUS_OK ||
            core_machine_d4_platform_refresh_request(platform, &address);
        failed |= trace.requests != 1u || trace.acknowledgements != 1u ||
            trace.commits != 1u || trace.releases != 1u;
        platform->d4_refresh_hold_pending = LIB_TRUE;
        failed |= !core_machine_d4_platform_refresh_request(platform, &address) || address != 1u;
        failed |= core_machine_reset(machine) != LIB_STATUS_OK ||
            platform->d4_refresh_hold_pending || platform->d4_refresh_pulse_active ||
            platform->d4_refresh_address != 0u;
    }
    core_machine_destroy(machine);
    if (failed) {
        fprintf(stderr, "D4 prefetch/refresh: grants=%u request=%u ack=%u commit=%u release=%u\n",
            prefetch_grants, trace.requests, trace.acknowledgements, trace.commits, trace.releases);
        return 1;
    }
    printf("M5:T540:S93:D4-PREFETCH-REFRESH:OK\n");
    return 0;
}
