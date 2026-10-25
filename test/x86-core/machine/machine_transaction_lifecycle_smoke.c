#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86-core/machine/machine.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/transaction.h"
#include "support/memory_alias_fixture.h"

typedef struct lifecycle_probe {
    x86_core_machine_trace_event events[32];
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
    lib_uptr bytes, x86_core_machine_memory_access access)
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

static lib_status lifecycle_port_write(void *opaque, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    (void)access_bytes;
    lifecycle_probe *probe = opaque;
    (void)port;
    (void)value;
    ++probe->transfers;
    return probe->transfer_status;
}

static void lifecycle_trace(void *opaque,
    const x86_core_machine_trace_event *event)
{
    lifecycle_probe *probe = (lifecycle_probe *)opaque;

    if (probe != LIB_NULL && probe->count < 32u) {
        probe->events[probe->count++] = *event;
    }
}

static lib_i32 lifecycle_find_transaction(const lifecycle_probe *probe,
    lib_u32 type, x86_core_machine_transaction_owner owner,
    x86_core_machine_transaction_kind kind, lib_u32 *out_index)
{
    lib_u32 index;

    if (probe == LIB_NULL || out_index == LIB_NULL) return 0;
    for (index = 0u; index < probe->count; ++index) {
        const x86_core_machine_trace_event *event = &probe->events[index];

        if (event->type == type && (event->detail & 0xffu) == owner &&
            ((event->detail >> 8u) & 0xffu) == kind) {
            *out_index = index;
            return 1;
        }
    }
    return 0;
}

static lib_i32 lifecycle_find_event(const lifecycle_probe *probe,
    lib_u32 type, lib_u32 *out_index)
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

static lib_i32 lifecycle_cpu_port_boundaries(lib_bool sx)
{
    const x86_core_machine_port_provider ports = { lifecycle_port_read, lifecycle_port_write };
    const x86_core_machine_executor_config config = {
        .memory_bytes = 16u * 1024u * 1024u,
        .cpu_profile = sx ? X86_CORE_CPU_PROFILE_80486SX : X86_CORE_CPU_PROFILE_80386
    };
    lifecycle_probe probe = {0};
    const x86_core_machine_trace_provider trace = { lifecycle_trace, &probe };
    x86_core_machine *machine = LIB_NULL;
    lib_i32 failed = 0;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
    failed = x86_core_machine_install_port_provider(machine, 0x1234u, 0x1234u,
            &ports, &probe) != LIB_STATUS_OK ||
        test_x86_core_machine_fixture_register_reset_mapping(machine,
            0xfffffff0u, 0xffff0u, 16u) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK;
    for (lib_u8 bytes = 1u; bytes <= 4u && !failed; bytes *= 2u) {
        for (lib_u8 write = 0u; write < 2u && !failed; ++write) {
            /* Success, endpoint failure after a side effect, admission failure. */
            for (lib_u8 outcome = 0u; outcome < 3u && !failed; ++outcome) {
                lib_u32 value = 0xabcdef01u;
                x86_core_machine_run_result result = {0};
                const lib_u32 mask = bytes == 1u ? 0xffu :
                    bytes == 2u ? 0xffffu : LIB_UINT32_MAX;
                lib_u8 code[3] = {0x90u}, length = 1u;
                const x86_core_cpu_debug_register_patch registers = {
                    .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EAX) |
                        X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDX),
                    .values = {[X86_CORE_CPU_DEBUG_EAX] = 0xabcdef01u,
                        [X86_CORE_CPU_DEBUG_EDX] = 0x1234u}
                };

                failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
                if (bytes == 4u) code[length++] = 0x66u;
                code[length++] = (write ? 0xeeu : 0xecu) + (bytes != 1u);
                failed |= x86_core_machine_memory_write(machine, 0xffff0u, code, length) != LIB_STATUS_OK ||
                    x86_core_cpu_debug_patch_registers(machine->executor_cpu_execution, &registers) != LIB_STATUS_OK;
                failed |= x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,0u},
                    &result) != LIB_STATUS_OK || result.executed != 1u;
                /* NOP warms the actual prefetch queue. Admission failure must
                 * occur at the port, not at an earlier instruction fetch. */
                probe = (lifecycle_probe){0};
                probe.transfer_status = outcome == 1u ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
                if (outcome == 2u) {
                    failed |= x86_core_machine_transaction_begin(&machine->transaction,
                        X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
                        X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, 0u, 0u, 0u) != LIB_STATUS_OK;
                    probe.count = 0u;
                }
                const lib_status run_status = x86_core_machine_run(machine,
                    (x86_core_machine_run_budget){1u,0u}, &result);
                failed |= run_status != (outcome == 0u ? LIB_STATUS_OK : LIB_STATUS_INTERNAL_ERROR) ||
                    result.executed != (outcome == 0u ? 1u : 0u);
                failed |= x86_core_cpu_debug_read_register(machine->executor_cpu_execution,
                    X86_CORE_CPU_DEBUG_EAX, &value) != LIB_STATUS_OK ||
                    value != (outcome == 0u && !write ?
                        (0xabcdef01u & ~mask) | (0x12345678u & mask) : 0xabcdef01u);
                failed |= probe.transfers != (outcome == 2u ? 0u : 1u) ||
                    probe.count != (outcome == 2u ? 3u : outcome == 1u ? 5u : 6u) ||
                    probe.events[0].type != X86_CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_BEGIN ||
                    machine->external_cycle_pending_valid;
                if (outcome != 0u)
                    failed |= probe.count == 0u ||
                        probe.events[probe.count - 1u].type != X86_CORE_MACHINE_TRACE_FAULT ||
                        machine->fault_detail != VCPUINS_EXCEPT_CE;
                else
                    failed |= probe.events[4].type != X86_CORE_MACHINE_TRACE_CPU_RETIRE ||
                        probe.events[5].type != X86_CORE_MACHINE_TRACE_RUN_BOUNDARY;
                if (outcome == 2u) {
                    failed |= probe.events[1].type != X86_CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_CANCEL ||
                        machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_DMA;
                    x86_core_machine_transaction_cancel(&machine->transaction);
                } else {
                    failed |= probe.events[1].type != X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN ||
                        probe.events[2].type != (outcome == 0u ? X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT :
                            X86_CORE_MACHINE_TRACE_TRANSACTION_CANCEL) ||
                        probe.events[3].type != (outcome == 0u ? X86_CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_COMMIT :
                            X86_CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_CANCEL) ||
                        machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE;
                }
                if (failed) {
                    lib_c_printf("Port lifecycle sx=%u bytes=%u write=%u outcome=%u: events=%u transfers=%u value=%x fault=%x\n",
                        (lib_u32)sx, (lib_u32)bytes, (lib_u32)write, (lib_u32)outcome,
                        probe.count, probe.transfers, value, machine->fault_detail);
                    for (lib_u32 event = 0u; event < probe.count; ++event)
                        lib_c_printf("  event %u: type=%u detail=%x\n", event,
                            probe.events[event].type, probe.events[event].detail);
                }
            }
        }
    }
    x86_core_machine_destroy(machine);
    return failed;
}

static lib_i32 lifecycle_cpu_memory_boundaries(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 16u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    lifecycle_probe probe = {0};
    const x86_core_machine_trace_provider trace = { lifecycle_trace, &probe };
    x86_core_machine *machine = LIB_NULL;
    lib_i32 failed = 0;

    const x86_core_machine_memory_device_route route = {
        .physical_start = 0x80000u, .bytes = 1u,
        .callbacks = {lifecycle_memory_read, lifecycle_memory_write,
            lifecycle_memory_query},
        .failure = LIB_NULL
    };

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
    failed = x86_core_machine_install_memory_device_routes(machine, &route, 1u,
            LIB_NULL, LIB_NULL, &probe) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK;
    for (lib_u8 write = 0u; write < 2u && !failed; ++write) {
        for (lib_u8 outcome = 0u; outcome < 3u && !failed; ++outcome) {
            lib_u8 value = 0x5au;
            lib_status status;
            probe = (lifecycle_probe){ .memory_value = 0x3cu,
                .transfer_status = outcome == 1u ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK };
            if (outcome == 2u) {
                failed |= x86_core_machine_transaction_begin(&machine->transaction,
                    X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
                    X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, 0u, 0u, 0u) != LIB_STATUS_OK;
                probe.count = 0u;
            }
            status = write ? x86_core_machine_cpu_bus.write_memory(machine,
                0x80000u, &value, 1u, X86_CORE_CPU_MEMORY_ACCESS_DATA, 0u) :
                x86_core_machine_cpu_bus.read_memory(machine, 0x80000u, &value, 1u,
                    X86_CORE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 0u);
            failed |= status != (outcome == 0u ? LIB_STATUS_OK :
                outcome == 1u ? LIB_STATUS_IO_ERROR : LIB_STATUS_INVALID_ARGUMENT);
            failed |= probe.transfers != (outcome == 2u ? 0u : 1u);
            if (outcome == 2u) {
                failed |= probe.count != 0u || value != 0x5au ||
                    probe.memory_value != 0x3cu ||
                    machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_DMA;
                x86_core_machine_transaction_cancel(&machine->transaction);
            } else {
                /* Cancellation does not undo a provider's completed side effect. */
                failed |= (write ? probe.memory_value != 0x5au : value != 0x3cu) ||
                    probe.count != 2u ||
                    probe.events[0].type != X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN ||
                    probe.events[1].type != (outcome == 0u ?
                        X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT : X86_CORE_MACHINE_TRACE_TRANSACTION_CANCEL) ||
                    ((probe.events[0].detail >> 8u) & 0xffu) != (write ?
                        X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_WRITE : X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ) ||
                    (probe.events[0].detail >> 16u) != X86_CORE_CPU_MEMORY_ACCESS_DATA ||
                    machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE;
            }
        }
    }
    if (!failed) {
        lib_u8 value = 0u;
        probe = (lifecycle_probe){ .memory_value = 0x7eu };
        failed |= x86_core_machine_cpu_bus.read_memory(machine, 0x80000u, &value, 1u,
            X86_CORE_CPU_MEMORY_ACCESS_DATA, LIB_TRUE, LIB_FALSE, 0u) != LIB_STATUS_OK ||
            value != 0x7eu || probe.count != 0u || probe.transfers != 0u;
        x86_core_machine_cpu_bus.extension_command(machine, 0xd9u, 0xe8u);
        failed |= probe.count != 2u ||
            probe.events[0].type != X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN ||
            probe.events[1].type != X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT ||
            ((probe.events[0].detail >> 8u) & 0xffu) != X86_CORE_MACHINE_TRANSACTION_CPU_FPU_COMMAND;
        failed |= x86_core_machine_transaction_begin(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
            X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, 0u, 0u, 0u) != LIB_STATUS_OK;
        probe.count = 0u;
        x86_core_machine_cpu_bus.extension_command(machine, 0xd9u, 0xe8u);
        failed |= probe.count != 0u ||
            machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_DMA;
        x86_core_machine_transaction_cancel(&machine->transaction);
    }
    x86_core_machine_destroy(machine);
    return failed;
}

typedef struct interrupt_probe {
    lifecycle_probe trace;
    lib_bool pending;
    lib_u32 acknowledgements;
    lib_u32 finalized;
} interrupt_probe;

static lib_bool interrupt_pending(void *owner)
{
    return ((interrupt_probe *)owner)->pending;
}

static lib_u8 interrupt_acknowledge(void *owner)
{
    interrupt_probe *probe = owner;
    probe->pending = LIB_FALSE;
    ++probe->acknowledgements;
    return 0x2eu;
}

static void interrupt_finalize(void *owner)
{
    ++((interrupt_probe *)owner)->finalized;
}

static lib_i32 lifecycle_interrupt_boundary(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    interrupt_probe probe = {.pending = LIB_TRUE};
    const x86_core_machine_attachment attachment = {
        .interrupt_pending = interrupt_pending,
        .interrupt_acknowledge = interrupt_acknowledge,
        .finalize_devices = interrupt_finalize,
        .context = &probe
    };
    const x86_core_machine_trace_provider trace = {lifecycle_trace, &probe.trace};
    x86_core_machine *machine = LIB_NULL;
    lib_u8 vector = 0xffu;
    lib_i32 failed = 1;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
    if (x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK ||
        !x86_core_machine_cpu_bus.interrupt_pending(machine) ||
        x86_core_machine_transaction_begin(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
            X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, 0u, 0u, 0u) != LIB_STATUS_OK)
        goto done;
    probe.trace.count = 0u;
    failed = x86_core_machine_cpu_bus.acknowledge_interrupt(machine, &vector) !=
            LIB_STATUS_INVALID_ARGUMENT || vector != 0xffu || probe.trace.count != 0u ||
        machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_DMA ||
        !x86_core_machine_cpu_bus.interrupt_pending(machine) || probe.acknowledgements != 0u;
    x86_core_machine_transaction_cancel(&machine->transaction);
    probe.trace.count = 0u;
    failed |= x86_core_machine_cpu_bus.acknowledge_interrupt(machine, &vector) != LIB_STATUS_OK ||
        vector != 0x2eu || probe.trace.count != 2u || probe.acknowledgements != 1u ||
        x86_core_machine_cpu_bus.interrupt_pending(machine) ||
        probe.trace.events[0].type != X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN ||
        probe.trace.events[1].type != X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT ||
        probe.trace.events[1].value != 0x2eu ||
        ((probe.trace.events[0].detail >> 8u) & 0xffu) !=
            X86_CORE_MACHINE_TRANSACTION_CPU_INTERRUPT_ACKNOWLEDGE ||
        machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE;
done:
    x86_core_machine_destroy(machine);
    return failed || probe.finalized != 1u;
}

static lib_i32 lifecycle_hold_reset(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    lifecycle_probe probe = {0};
    const x86_core_machine_trace_provider trace = {lifecycle_trace, &probe};
    x86_core_machine *machine = LIB_NULL;
    lib_u32 request = 0u, acknowledge = 0u, release = 0u;
    lib_i32 failed = 1;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK)
        return 1;
    if (x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK ||
        x86_core_machine_transaction_hold_request(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) != LIB_STATUS_OK ||
        x86_core_machine_transaction_hold_acknowledge(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_DMA) != LIB_STATUS_OK) goto done;
    failed = x86_core_machine_transaction_begin(&machine->transaction,
        X86_CORE_MACHINE_TRANSACTION_OWNER_CPU,
        X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ, 0u, 0u, 0u) !=
            LIB_STATUS_INVALID_ARGUMENT;
    x86_core_machine_transaction_hold_release(&machine->transaction,
        X86_CORE_MACHINE_TRANSACTION_OWNER_DMA);
    failed |= machine->transaction.hold_owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE;
    probe.count = 0u;
    failed |= x86_core_machine_transaction_hold_request(&machine->transaction,
        X86_CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) != LIB_STATUS_OK ||
        x86_core_machine_transaction_hold_acknowledge(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_DMA) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        machine->transaction.hold_owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        !lifecycle_find_event(&probe,
            X86_CORE_MACHINE_TRACE_TRANSACTION_HOLD_REQUEST, &request) ||
        !lifecycle_find_event(&probe,
            X86_CORE_MACHINE_TRACE_TRANSACTION_HOLD_ACKNOWLEDGE, &acknowledge) ||
        !lifecycle_find_event(&probe,
            X86_CORE_MACHINE_TRACE_TRANSACTION_HOLD_RELEASE, &release) ||
        request >= acknowledge || acknowledge >= release;
done:
    x86_core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    const lib_u8 nop = 0x90u;
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_executor_config config = {
        .memory_bytes = 16u * 1024u * 1024u
    };
    x86_core_machine_trace_provider trace;
    x86_core_machine_run_budget budget = {1u, 0u};
    x86_core_machine_run_result result;
    lifecycle_probe probe = {0};
    lib_u32 begin = 0u;
    lib_u32 cancel = 0u;
    lib_u32 reset = 0u;
    lib_u32 cpu_begin = 0u;
    lib_u32 cpu_commit = 0u;
    lib_u32 cpu_retire = 0u;
    lib_i32 failed = 0;

    config.cpu_profile = X86_CORE_CPU_PROFILE_80286;
    trace.callback = lifecycle_trace;
    trace.context = &probe;
    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
    failed |= test_x86_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != LIB_STATUS_OK;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK;
    failed |= x86_core_machine_transaction_begin(&machine->transaction,
        X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
        X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, 0x11234u, 1u, 2u) !=
        LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE ||
        machine->transaction.committed_count != 0u ||
        machine->transaction.cancelled_count != 0u;
    failed |= !lifecycle_find_transaction(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
        X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, &begin);
    failed |= !lifecycle_find_transaction(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_CANCEL,
        X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
        X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, &cancel);
    failed |= !lifecycle_find_event(&probe, X86_CORE_MACHINE_TRACE_RESET, &reset);
    failed |= begin >= cancel || cancel >= reset;

    failed |= x86_core_machine_memory_write(machine, 0xfffffff0u, &nop, 1u) !=
        LIB_STATUS_OK;
    failed |= x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    if (!failed) failed |= result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        result.elapsed_ticks != 3u;
    failed |= !lifecycle_find_transaction(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        X86_CORE_MACHINE_TRANSACTION_OWNER_CPU,
        X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ, &cpu_begin);
    failed |= !lifecycle_find_transaction(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
        X86_CORE_MACHINE_TRANSACTION_OWNER_CPU,
        X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ, &cpu_commit);
    failed |= !lifecycle_find_event(&probe, X86_CORE_MACHINE_TRACE_CPU_RETIRE,
        &cpu_retire);
    failed |= reset >= cpu_begin || cpu_begin >= cpu_commit ||
        cpu_commit >= cpu_retire;

    x86_core_machine_destroy(machine);
    failed |= lifecycle_cpu_port_boundaries(LIB_FALSE) |
        lifecycle_cpu_port_boundaries(LIB_TRUE);
    failed |= lifecycle_cpu_memory_boundaries();
    failed |= lifecycle_interrupt_boundary();
    failed |= lifecycle_hold_reset();
    if (failed) return 1;
    lib_c_printf("TRANSACTION-LIFECYCLE:OK\n");
    return 0;
}
