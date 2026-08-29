#ifndef CORE_MACHINE_TRACE_INTERFACE_H
#define CORE_MACHINE_TRACE_INTERFACE_H


#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

typedef enum core_machine_trace_event_type {
    CORE_MACHINE_TRACE_RESET = 1,
    CORE_MACHINE_TRACE_PORT_READ,
    CORE_MACHINE_TRACE_PORT_WRITE,
    CORE_MACHINE_TRACE_MEMORY_READ,
    CORE_MACHINE_TRACE_MEMORY_WRITE,
    CORE_MACHINE_TRACE_CPU_RETIRE,
    CORE_MACHINE_TRACE_EXTERNAL_TIME,
    CORE_MACHINE_TRACE_DMA_ADVANCE,
    CORE_MACHINE_TRACE_PIT_ADVANCE,
    CORE_MACHINE_TRACE_PIC_REFRESH,
    CORE_MACHINE_TRACE_RTC_ADVANCE,
    CORE_MACHINE_TRACE_KBC_ADVANCE,
    CORE_MACHINE_TRACE_VADP_ADVANCE,
    CORE_MACHINE_TRACE_RUN_BOUNDARY,
    CORE_MACHINE_TRACE_STOP,
    CORE_MACHINE_TRACE_FAULT,
    CORE_MACHINE_TRACE_FDC_ADVANCE,
    CORE_MACHINE_TRACE_HDC_ADVANCE,
    CORE_MACHINE_TRACE_TRANSACTION_BEGIN,
    CORE_MACHINE_TRACE_TRANSACTION_COMMIT,
    CORE_MACHINE_TRACE_TRANSACTION_CANCEL,
    CORE_MACHINE_TRACE_TRANSACTION_HOLD_REQUEST,
    CORE_MACHINE_TRACE_TRANSACTION_HOLD_ACKNOWLEDGE,
    CORE_MACHINE_TRACE_TRANSACTION_HOLD_RELEASE,
    CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_BEGIN,
    CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_COMMIT,
    CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_CANCEL,
    CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_OVERLAP_DECLARE
} core_machine_trace_event_type;

typedef struct core_machine_trace_event {
    core_machine_trace_event_type type;
    type_unsigned_64 sequence;
    type_unsigned_64 elapsed_ticks;
    type_unsigned_64 timeline_ticks;
    type_unsigned_32 linear_pc;
    type_unsigned_32 address;
    type_unsigned_32 value;
    type_unsigned_32 detail;
} core_machine_trace_event;

typedef C_VOID (*core_machine_trace_event_provider)(
    C_VOID *context,
    const core_machine_trace_event *event);

typedef struct core_machine_trace_provider {
    core_machine_trace_event_provider callback;
    C_VOID *context;
} core_machine_trace_provider;

type_status core_machine_set_trace_provider(
    core_machine *machine,
    const core_machine_trace_provider *provider);

#ifdef __cplusplus
}
#endif

#endif
