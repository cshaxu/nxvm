#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "core/board-base/dma_bus_interface.h"
#include "../dma_competition_fixture.h"

typedef struct dma_transaction_probe {
    lib_u32 begins;
    lib_u32 commits;
    lib_u32 cancels;
    lib_bool wrong_transfer;
} dma_transaction_probe;

static void transaction_dma_read(void *owner, t_latch *latch)
{
    if (owner != LIB_NULL && latch != LIB_NULL)
        latch->data.byte = *(const lib_u8 *)owner;
}

static void transaction_dma_trace(void *owner,
    const x86_core_machine_trace_event *event)
{
    dma_transaction_probe *probe = owner;

    if (event->type == X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN)
        ++probe->begins;
    else if (event->type == X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT)
        ++probe->commits;
    else if (event->type == X86_CORE_MACHINE_TRACE_TRANSACTION_CANCEL)
        ++probe->cancels;
    else return;
    probe->wrong_transfer |= event->address != 0x11234u ||
        (event->detail & 0xffu) != X86_CORE_MACHINE_TRANSACTION_OWNER_DMA ||
        ((event->detail >> 8u) & 0xffu) !=
            X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE;
}

lib_i32 main(void)
{
    const core_machine_config config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    const core_machine_dma_channel_provider provider = {
        transaction_dma_read, LIB_NULL, LIB_NULL
    };
    x86_core_machine *machine = LIB_NULL;
    core_machine_dma_bus *dma = LIB_NULL;
    core_machine_dma_request_binding binding = {0};
    dma_transaction_probe probe = {0};
    const x86_core_machine_trace_provider trace = {transaction_dma_trace, &probe};
    lib_u8 source = 0xa5u;
    lib_u8 byte = 0u;
    lib_u16 next_value = 0x66u;
    lib_i32 failed = 1;

    if (test_dma_competition_create(&config, &provider, &source,
            &machine, &dma, &binding) != LIB_STATUS_OK) return 1;
    if (x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        test_dma_competition_program_channel2(machine) != LIB_STATUS_OK ||
        x86_core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK) goto done;
    core_machine_dma_request_assert(dma, &binding);
    /* Normal 8237A service selects the channel, then completes S1..S4.
     * This board test drives the actual controller and its public bus wiring. */
    core_machine_dma_advance_transaction(dma, machine, 5u);
    failed = probe.begins != 1u || probe.commits != 1u || probe.cancels != 0u ||
        probe.wrong_transfer ||
        x86_core_machine_memory_read(machine, 0x11234u, &byte, 1u) != LIB_STATUS_OK ||
        byte != source;
    /* A following accepted cycle proves the controller released bus ownership. */
    failed |= x86_core_machine_dma_memory_cycle(machine, 0x11234u, 1u, 2u,
        X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, &next_value,
        LIB_NULL, LIB_NULL, LIB_NULL) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x11234u, &byte, 1u) != LIB_STATUS_OK ||
        byte != 0x66u || probe.begins != 2u || probe.commits != 2u ||
        probe.cancels != 0u || probe.wrong_transfer;
done:
    x86_core_machine_destroy(machine);
    if (failed) return 1;
    lib_c_printf("DMA-TRANSACTION:OK\n");
    return 0;
}
