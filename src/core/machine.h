#ifndef NXVM_CORE_MACHINE_H
#define NXVM_CORE_MACHINE_H

#include <stdint.h>

#include "core/profile.h"
#include "core/status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nxvm_core_machine nxvm_core_machine;

typedef enum nxvm_core_stop_reason {
    NXVM_CORE_STOP_NONE = 0,
    NXVM_CORE_STOP_BUDGET,
    NXVM_CORE_STOP_PAUSED,
    NXVM_CORE_STOP_GUEST_EXIT,
    NXVM_CORE_STOP_REQUESTED,
    NXVM_CORE_STOP_FAULT
} nxvm_core_stop_reason;

typedef struct nxvm_core_run_budget {
    uint64_t instructions;
    uint32_t ticks;
} nxvm_core_run_budget;

typedef struct nxvm_core_run_result {
    nxvm_core_stop_reason reason;
    uint64_t executed;
    uint32_t linear_pc;
    uint32_t detail;
} nxvm_core_run_result;

nxvm_core_status nxvm_core_machine_create(
    const nxvm_core_machine_config *config,
    nxvm_core_machine **out_machine);

nxvm_core_status nxvm_core_machine_reset(nxvm_core_machine *machine);

nxvm_core_status nxvm_core_machine_run(
    nxvm_core_machine *machine,
    nxvm_core_run_budget budget,
    nxvm_core_run_result *result);

nxvm_core_status nxvm_core_machine_request_stop(nxvm_core_machine *machine);

void nxvm_core_machine_destroy(nxvm_core_machine *machine);

#ifdef __cplusplus
}
#endif

#endif
