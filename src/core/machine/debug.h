#ifndef NXVM_CORE_DEBUG_H
#define NXVM_CORE_DEBUG_H

#include <stddef.h>

#include "core/machine/machine.h"

#ifdef __cplusplus
extern "C" {
#endif

nxvm_core_status nxvm_core_debug_read_cpu(
    const nxvm_core_machine *machine,
    nxvm_core_cpu_state *out_state);
nxvm_core_status nxvm_core_debug_read_memory(
    const nxvm_core_machine *machine,
    uint32_t physical,
    void *out_data,
    size_t size);
nxvm_core_status nxvm_core_debug_step(
    nxvm_core_machine *machine,
    nxvm_core_run_result *out_result);
nxvm_core_status nxvm_core_debug_continue(
    nxvm_core_machine *machine,
    nxvm_core_run_budget budget,
    nxvm_core_run_result *out_result);

#ifdef __cplusplus
}
#endif

#endif
