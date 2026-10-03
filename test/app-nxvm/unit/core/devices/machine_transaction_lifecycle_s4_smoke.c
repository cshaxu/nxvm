#include "lib/types/types_interface.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/transaction.h"
#include "support/core_machine_board_fixture.h"

typedef struct lifecycle_probe {
    core_machine_trace_event events[32];
    lib_u32 count;
    lib_u32 transfers;
    lib_status transfer_status;
    lib_u8 memory_value;
} lifecycle_probe;

static lib_status lifecycle_memory_read(void *opaque, lib_u32 address,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    lifecycle_probe *probe = opaque;
    (void)address;
    if (bytes != 1u) return LIB_STATUS_INVALID_ARGUMENT;
    if (!observe_only) ++probe->transfers;
    *(lib_u8 *)destination = probe->memory_value;
    return probe->transfer_status;
}

static lib_status lifecycle_memory_write(void *opaque, lib_u32 address,
    lib_uptr source, lib_uptr bytes)
{
    lifecycle_probe *probe = opaque;
    (void)address;
    if (bytes != 1u) return LIB_STATUS_INVALID_ARGUMENT;
    ++probe->transfers;
    probe->memory_value = *(const lib_u8 *)source;
    return probe->transfer_status;
}

static lib_status lifecycle_memory_query(void *opaque, lib_u32 address,
    lib_uptr bytes, core_machine_memory_access access)
{
    (void)opaque;
    (void)address;
    (void)access;
    return bytes == 1u ? LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static lib_status lifecycle_port_read(void *opaque, lib_u16 port, lib_u64 tick,
    lib_u32 *value)
{
    (void)tick;
    lifecycle_probe *probe = opaque;
    (void)port;
    ++probe->transfers;
    *value = 0x12345678u;
    return probe->transfer_status;
}

static lib_status lifecycle_port_write(void *opaque, lib_u16 port, lib_u32 value)
{
    lifecycle_probe *probe = opaque;
    (void)port;
    (void)value;
    ++probe->transfers;
    return probe->transfer_status;
}

static void lifecycle_trace(void *opaque,
    const core_machine_trace_event *event)
{
    lifecycle_probe *probe = (lifecycle_probe *)opaque;

    if (probe != LIB_NULL && probe->count < 32u) {
        probe->events[probe->count++] = *event;
    }
}

static lib_i32 lifecycle_find_transaction(const lifecycle_probe *probe,
    core_machine_trace_event_type type, core_machine_transaction_owner owner,
    core_machine_transaction_kind kind, lib_u32 *out_index)
{
    lib_u32 index;

    if (probe == LIB_NULL || out_index == LIB_NULL) return 0;
    for (index = 0u; index < probe->count; ++index) {
        const core_machine_trace_event *event = &probe->events[index];

        if (event->type == type && (event->detail & 0xffu) == owner &&
            ((event->detail >> 8u) & 0xffu) == kind) {
            *out_index = index;
            return 1;
        }
    }
    return 0;
}

static lib_i32 lifecycle_find_event(const lifecycle_probe *probe,
    core_machine_trace_event_type type, lib_u32 *out_index)
{
    lib_u32 index;

    if (probe == LIB_NULL || out_index == LIB_NULL) return 0;
    for (index = 0u; index < probe->count; ++index) {
        if (probe->events[index].type == type) {
            *out_index = index;
            return 1;
        }
    }
    return 0;
}

static lib_i32 lifecycle_cpu_port_boundaries(void)
{
    const core_machine_port_provider ports = { lifecycle_port_read, lifecycle_port_write };
    const core_machine_config config = {0};
    lifecycle_probe probe = {0};
    const core_machine_trace_provider trace = { lifecycle_trace, &probe };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = 0;

    if (core_machine_create(&config, &machine) != LIB_STATUS_OK) return 1;
    failed = core_machine_install_port_provider(machine, 0x1234u, 0x1234u,
            &ports, &probe) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK;
    for (lib_u8 bytes = 1u; bytes <= 4u && !failed; bytes *= 2u) {
        for (lib_u8 write = 0u; write < 2u && !failed; ++write) {
            /* Success, endpoint failure after a side effect, admission failure. */
            for (lib_u8 outcome = 0u; outcome < 3u && !failed; ++outcome) {
                lib_u32 value = 0xabcdef01u;
                const lib_u32 mask = bytes == 1u ? 0xffu :
                    bytes == 2u ? 0xffffu : LIB_UINT32_MAX;
                lib_status status;

                probe = (lifecycle_probe){0};
                probe.transfer_status = outcome == 1u ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
                if (outcome == 2u) {
                    failed |= core_machine_transaction_begin(&machine->transaction,
                        CORE_MACHINE_TRANSACTION_OWNER_DMA,
                        CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, 0u, 0u, 0u) != LIB_STATUS_OK;
                    probe.count = 0u;
                }
                status = core_machine_cpu_bus.transfer_port(machine, 0x1234u,
                    bytes, write, &value);
                if (outcome == 0u) {
                    failed |= status != LIB_STATUS_OK || probe.count != 2u ||
                        machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_CPU;
                    if (!write) failed |= value != (0x12345678u & mask);
                    core_machine_cpu_bus.complete_port(machine, 0x1234u, bytes, write);
                } else {
                    failed |= status != (outcome == 1u ? LIB_STATUS_IO_ERROR :
                        LIB_STATUS_INVALID_ARGUMENT) || value != 0xabcdef01u;
                }
                failed |= probe.transfers != (outcome == 2u ? 0u : 1u) ||
                    probe.count != (outcome == 2u ? 2u : 4u) ||
                    probe.events[0].type != CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_BEGIN ||
                    machine->external_cycle_pending_valid;
                if (outcome == 2u) {
                    failed |= probe.events[1].type != CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_CANCEL ||
                        machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_DMA;
                    core_machine_transaction_cancel(&machine->transaction);
                } else {
                    failed |= probe.events[1].type != CORE_MACHINE_TRACE_TRANSACTION_BEGIN ||
                        probe.events[2].type != (outcome == 0u ? CORE_MACHINE_TRACE_TRANSACTION_COMMIT :
                            CORE_MACHINE_TRACE_TRANSACTION_CANCEL) ||
                        probe.events[3].type != (outcome == 0u ? CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_COMMIT :
                            CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_CANCEL) ||
                        machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE;
                }
            }
        }
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 lifecycle_cpu_memory_boundaries(void)
{
    const core_machine_config config = {0};
    lifecycle_probe probe = {0};
    const core_machine_trace_provider trace = { lifecycle_trace, &probe };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = 0;

    if (core_machine_create(&config, &machine) != LIB_STATUS_OK) return 1;
    failed = test_core_machine_fixture_register_memory_device_provider(machine,
            0x80000u, 1u, lifecycle_memory_read, lifecycle_memory_write,
            lifecycle_memory_query, &probe) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK;
    for (lib_u8 write = 0u; write < 2u && !failed; ++write) {
        for (lib_u8 outcome = 0u; outcome < 3u && !failed; ++outcome) {
            lib_u8 value = 0x5au;
            lib_status status;
            probe = (lifecycle_probe){ .memory_value = 0x3cu,
                .transfer_status = outcome == 1u ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK };
            if (outcome == 2u) {
                failed |= core_machine_transaction_begin(&machine->transaction,
                    CORE_MACHINE_TRANSACTION_OWNER_DMA,
                    CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, 0u, 0u, 0u) != LIB_STATUS_OK;
                probe.count = 0u;
            }
            status = write ? core_machine_cpu_bus.write_memory(machine,
                0x80000u, &value, 1u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) :
                core_machine_cpu_bus.read_memory(machine, 0x80000u, &value, 1u,
                    CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE);
            failed |= status != (outcome == 0u ? LIB_STATUS_OK :
                outcome == 1u ? LIB_STATUS_IO_ERROR : LIB_STATUS_INVALID_ARGUMENT);
            failed |= probe.transfers != (outcome == 2u ? 0u : 1u);
            if (outcome == 2u) {
                failed |= probe.count != 0u || value != 0x5au ||
                    probe.memory_value != 0x3cu ||
                    machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_DMA;
                core_machine_transaction_cancel(&machine->transaction);
            } else {
                /* Cancellation does not undo a provider's completed side effect. */
                failed |= (write ? probe.memory_value != 0x5au : value != 0x3cu) ||
                    probe.count != 2u ||
                    probe.events[0].type != CORE_MACHINE_TRACE_TRANSACTION_BEGIN ||
                    probe.events[1].type != (outcome == 0u ?
                        CORE_MACHINE_TRACE_TRANSACTION_COMMIT : CORE_MACHINE_TRACE_TRANSACTION_CANCEL) ||
                    ((probe.events[0].detail >> 8u) & 0xffu) != (write ?
                        CORE_MACHINE_TRANSACTION_CPU_MEMORY_WRITE : CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ) ||
                    (probe.events[0].detail >> 16u) != CORE_MACHINE_CPU_MEMORY_ACCESS_DATA ||
                    machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE;
            }
        }
    }
    if (!failed) {
        lib_u8 value = 0u;
        probe = (lifecycle_probe){ .memory_value = 0x7eu };
        failed |= core_machine_cpu_bus.read_memory(machine, 0x80000u, &value, 1u,
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_TRUE, LIB_FALSE) != LIB_STATUS_OK ||
            value != 0x7eu || probe.count != 0u || probe.transfers != 0u;
        core_machine_cpu_bus.extension_command(machine, 0xd9u, 0xe8u);
        failed |= probe.count != 2u ||
            probe.events[0].type != CORE_MACHINE_TRACE_TRANSACTION_BEGIN ||
            probe.events[1].type != CORE_MACHINE_TRACE_TRANSACTION_COMMIT ||
            ((probe.events[0].detail >> 8u) & 0xffu) != CORE_MACHINE_TRANSACTION_CPU_FPU_COMMAND;
        failed |= core_machine_transaction_begin(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA,
            CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, 0u, 0u, 0u) != LIB_STATUS_OK;
        probe.count = 0u;
        core_machine_cpu_bus.extension_command(machine, 0xd9u, 0xe8u);
        failed |= probe.count != 0u ||
            machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_DMA;
        core_machine_transaction_cancel(&machine->transaction);
    }
    core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    const lib_u8 nop = 0x90u;
    core_machine *machine = LIB_NULL;
    core_machine_config config = {0};
    core_machine_trace_provider trace;
    core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    lifecycle_probe probe = {0};
    lib_u32 begin = 0u;
    lib_u32 cancel = 0u;
    lib_u32 reset = 0u;
    lib_u32 cpu_begin = 0u;
    lib_u32 cpu_commit = 0u;
    lib_u32 cpu_retire = 0u;
    lib_i32 failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    trace.callback = lifecycle_trace;
    trace.context = &probe;
    failed |= core_machine_create(&config, &machine) != LIB_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK;
    failed |= core_machine_transaction_begin(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_DMA,
        CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, 0x11234u, 1u, 2u) !=
        LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        machine->transaction.committed_count != 0u ||
        machine->transaction.cancelled_count != 0u;
    failed |= !lifecycle_find_transaction(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        CORE_MACHINE_TRANSACTION_OWNER_DMA,
        CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, &begin);
    failed |= !lifecycle_find_transaction(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_CANCEL,
        CORE_MACHINE_TRANSACTION_OWNER_DMA,
        CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, &cancel);
    failed |= !lifecycle_find_event(&probe, CORE_MACHINE_TRACE_RESET, &reset);
    failed |= begin >= cancel || cancel >= reset;

    failed |= core_machine_memory_write(machine, 0xfffffff0u, &nop, 1u) !=
        LIB_STATUS_OK;
    failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        result.elapsed_ticks != 3u;
    failed |= !lifecycle_find_transaction(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        CORE_MACHINE_TRANSACTION_OWNER_CPU,
        CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ, &cpu_begin);
    failed |= !lifecycle_find_transaction(&probe,
        CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
        CORE_MACHINE_TRANSACTION_OWNER_CPU,
        CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ, &cpu_commit);
    failed |= !lifecycle_find_event(&probe, CORE_MACHINE_TRACE_CPU_RETIRE,
        &cpu_retire);
    failed |= reset >= cpu_begin || cpu_begin >= cpu_commit ||
        cpu_commit >= cpu_retire;

    core_machine_destroy(machine);
    failed |= lifecycle_cpu_port_boundaries();
    failed |= lifecycle_cpu_memory_boundaries();
    if (failed) return 1;
    printf("M5:T354:S4:TRANSACTION-LIFECYCLE:OK\n");
    return 0;
}
