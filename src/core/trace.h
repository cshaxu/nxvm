#ifndef NXVM_CORE_TRACE_H
#define NXVM_CORE_TRACE_H

#include <stdint.h>

#include "core/status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nxvm_core_machine nxvm_core_machine;

typedef enum nxvm_core_trace_event_type {
    NXVM_CORE_TRACE_RESET = 1,
    NXVM_CORE_TRACE_PORT_READ,
    NXVM_CORE_TRACE_PORT_WRITE,
    NXVM_CORE_TRACE_RUN_BOUNDARY,
    NXVM_CORE_TRACE_STOP,
    NXVM_CORE_TRACE_FAULT
} nxvm_core_trace_event_type;

typedef struct nxvm_core_trace_event {
    nxvm_core_trace_event_type type;
    uint64_t sequence;
    uint32_t linear_pc;
    uint32_t address;
    uint32_t value;
    uint32_t detail;
} nxvm_core_trace_event;

typedef void (*nxvm_core_trace_callback)(
    void *context,
    const nxvm_core_trace_event *event);

typedef struct nxvm_core_trace_sink {
    nxvm_core_trace_callback callback;
    void *context;
} nxvm_core_trace_sink;

nxvm_core_status nxvm_core_machine_set_trace_sink(
    nxvm_core_machine *machine,
    const nxvm_core_trace_sink *sink);

#ifdef __cplusplus
}
#endif

#endif
