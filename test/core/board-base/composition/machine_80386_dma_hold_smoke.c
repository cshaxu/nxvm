#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "core/board-base/dma_bus_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "core/board-base/trace_interface.h"
#include "../dma_competition_fixture.h"

typedef struct competition_probe {
    x86_core_machine_trace_event events[256];
    lib_u32 count;
} competition_probe;

typedef struct competition_dma_source {
    lib_u8 value;
} competition_dma_source;

static void competition_trace(void *opaque,
    const x86_core_machine_trace_event *event)
{
    competition_probe *probe = (competition_probe *)opaque;

    if (probe != LIB_NULL && probe->count < 256u) {
        probe->events[probe->count++] = *event;
    }
}

static void competition_dma_read(void *opaque, t_latch *latch)
{
    competition_dma_source *source = (competition_dma_source *)opaque;

    if (source != LIB_NULL && latch != LIB_NULL) latch->data.byte = source->value;
}

static lib_i32 competition_find_event(const competition_probe *probe,
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

static lib_i32 competition_find_event_after(const competition_probe *probe,
    lib_u32 type, lib_u32 start,
    lib_u32 *out_index)
{
    lib_u32 index;

    if (probe == LIB_NULL || out_index == LIB_NULL) return 0;
    for (index = start; index < probe->count; ++index) {
        if (probe->events[index].type == type) {
            *out_index = index;
            return 1;
        }
    }
    return 0;
}

static lib_i32 competition_find_transaction(const competition_probe *probe,
    lib_u32 phase, x86_core_machine_transaction_owner owner,
    x86_core_machine_transaction_kind kind, lib_u32 *out_index)
{
    lib_u32 index;

    if (probe == LIB_NULL || out_index == LIB_NULL) return 0;
    for (index = 0u; index < probe->count; ++index) {
        const x86_core_machine_trace_event *event = &probe->events[index];

        if (event->type == phase && (event->detail & 0xffu) == owner &&
            ((event->detail >> 8u) & 0xffu) == kind) {
            *out_index = index;
            return 1;
        }
    }
    return 0;
}

lib_i32 main(void)
{
    static const core_machine_dma_channel_provider dma_provider = {
        competition_dma_read, LIB_NULL, LIB_NULL
    };
    const lib_u8 nop = 0x90u;
    x86_core_machine *machine = LIB_NULL;
    core_machine_dma_bus *dma = LIB_NULL;
    core_machine_config config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .l1_compatibility_policy = X86_CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS
    };
    const x86_core_machine_memory_alias_config reset_alias = {
        0xfffffff0u, 0x000ffff0u, 16u
    };
    x86_core_machine_trace_provider trace;
    core_machine_dma_request_binding binding = {0};
    x86_core_machine_run_budget budget = {1u, 0u};
    x86_core_machine_run_result result;
    competition_probe probe = {{{0}}, 0u};
    competition_dma_source source = {0xa5u};
    lib_u8 byte = 0u;
    lib_u32 cpu_begin = 0u;
    lib_u32 cpu_commit = 0u;
    lib_u32 cpu_retire = 0u;
    lib_u32 dma_begin = 0u;
    lib_u32 dma_commit = 0u;
    lib_u32 dma_advance = 0u;
    lib_u32 pit_advance = 0u;
    lib_u32 pic_refresh = 0u;
    lib_u32 fdc_advance = 0u;
    lib_u32 hdc_advance = 0u;
    lib_u32 hold_request = 0u;
    lib_u32 hold_acknowledge = 0u;
    lib_u32 hold_release = 0u;
    lib_u8 advanced = LIB_FALSE;
    lib_i32 failed = 0;

    trace.callback = competition_trace;
    trace.context = &probe;
    failed |= test_dma_competition_create(&config, &dma_provider, &source,
        &machine, &dma, &binding) != LIB_STATUS_OK;
    failed |= x86_core_machine_install_memory_aliases(machine, &reset_alias, 1u,
        LIB_FALSE) != LIB_STATUS_OK;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_memory_write(machine, 0xfffffff0u, &nop, 1u) !=
        LIB_STATUS_OK;
    failed |= test_dma_competition_program_channel2(machine) != LIB_STATUS_OK;
    core_machine_dma_request_assert(dma, &binding);
    failed |= x86_core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK;
    failed |= x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    failed |= result.reason != X86_CORE_MACHINE_STOP_BUDGET ||
        result.executed != 1u || result.elapsed_ticks != 3u;
    /* A normal 8237A service selects a channel before S1..S4 commits the
     * byte.  CPU retirement has already occurred; advance only device time. */
    failed |= x86_core_machine_advance_l1_compatibility(machine, &advanced) !=
        LIB_STATUS_OK || !advanced;
    failed |= x86_core_machine_memory_read(machine, 0x11234u, &byte, 1u) !=
        LIB_STATUS_OK || byte != 0xa5u;
    failed |= !competition_find_transaction(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        X86_CORE_MACHINE_TRANSACTION_OWNER_CPU,
        X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ, &cpu_begin);
    failed |= !competition_find_transaction(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
        X86_CORE_MACHINE_TRANSACTION_OWNER_CPU,
        X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ, &cpu_commit);
    failed |= !competition_find_event(&probe, X86_CORE_MACHINE_TRACE_CPU_RETIRE,
        &cpu_retire);
    failed |= !competition_find_transaction(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
        X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
        X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, &dma_begin);
    failed |= !competition_find_transaction(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
        X86_CORE_MACHINE_TRANSACTION_OWNER_DMA,
        X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, &dma_commit);
    failed |= !competition_find_event_after(&probe, X86_CORE_MACHINE_TRACE_DMA_ADVANCE,
        dma_commit, &dma_advance);
    failed |= !competition_find_event_after(&probe, CORE_MACHINE_BOARD_TRACE_PIT_ADVANCE,
        dma_advance, &pit_advance);
    failed |= !competition_find_event_after(&probe, CORE_MACHINE_BOARD_TRACE_PIC_REFRESH,
        pit_advance, &pic_refresh);
    failed |= competition_find_event(&probe, CORE_MACHINE_BOARD_TRACE_FDC_ADVANCE,
        &fdc_advance);
    failed |= competition_find_event(&probe, CORE_MACHINE_BOARD_TRACE_HDC_ADVANCE,
        &hdc_advance);
    failed |= !competition_find_event(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_HOLD_REQUEST, &hold_request);
    failed |= !competition_find_event(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_HOLD_ACKNOWLEDGE, &hold_acknowledge);
    failed |= !competition_find_event(&probe,
        X86_CORE_MACHINE_TRACE_TRANSACTION_HOLD_RELEASE, &hold_release);
    failed |= cpu_begin >= cpu_commit || cpu_commit >= cpu_retire ||
        cpu_retire >= dma_begin || dma_begin >= dma_commit ||
        dma_commit >= dma_advance || dma_advance >= pit_advance ||
        pit_advance >= pic_refresh || hold_request >= hold_acknowledge ||
        hold_acknowledge >= hold_release;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= core_machine_dma_has_pending_request(dma);

    x86_core_machine_destroy(machine);
    if (failed) return 1;
    lib_c_printf("COMPETITION:OK\n");
    lib_c_printf("PCAT-HOLD:OK\n");
    lib_c_printf("80386-DMA-HOLD:OK\n");
    return 0;
}
