#ifndef NTVDM64_CORE_MACHINE_TRACE_INTERFACE_H
#define NTVDM64_CORE_MACHINE_TRACE_INTERFACE_H

#include <stdint.h>

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

typedef enum core_machine_trace_event_type {
    CORE_MACHINE_TRACE_RESET = 1,
    CORE_MACHINE_TRACE_PORT_READ,
    CORE_MACHINE_TRACE_PORT_WRITE,
    CORE_MACHINE_TRACE_RUN_BOUNDARY,
    CORE_MACHINE_TRACE_STOP,
    CORE_MACHINE_TRACE_FAULT
} core_machine_trace_event_type;

typedef struct core_machine_trace_event {
    core_machine_trace_event_type type;
    uint64_t sequence;
    uint32_t linear_pc;
    uint32_t address;
    uint32_t value;
    uint32_t detail;
} core_machine_trace_event;

typedef void (*core_machine_trace_event_provider)(
    void *context,
    const core_machine_trace_event *event);

typedef struct core_machine_trace_provider {
    core_machine_trace_event_provider callback;
    void *context;
} core_machine_trace_provider;

ntvdm64_status core_machine_set_trace_provider(
    core_machine *machine,
    const core_machine_trace_provider *provider);

#ifdef __cplusplus
}
#endif

#endif
