#ifndef NTVDM64_CORE_MACHINE_INTERFACE_H
#define NTVDM64_CORE_MACHINE_INTERFACE_H

#include <stdint.h>

#include "core/machine/cpu_interface.h"
#include "core/machine/lifecycle_interface.h"
#include "core/machine/memory_interface.h"
#include "core/machine/port_interface.h"
#include "core/machine/profile_interface.h"
#include "type.h"
#include "core/machine/trace_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

typedef enum core_machine_stop_reason {
    CORE_MACHINE_STOP_NONE = 0,
    CORE_MACHINE_STOP_BUDGET,
    CORE_MACHINE_STOP_PAUSED,
    CORE_MACHINE_STOP_GUEST_EXIT,
    CORE_MACHINE_STOP_REQUESTED,
    CORE_MACHINE_STOP_RESET_REQUESTED,
    CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT,
    CORE_MACHINE_STOP_FAULT
} core_machine_stop_reason;

typedef struct core_machine_run_budget {
    uint64_t instructions;
    uint32_t ticks;
} core_machine_run_budget;

typedef struct core_machine_run_result {
    core_machine_stop_reason reason;
    uint64_t executed;
    uint32_t linear_pc;
    uint32_t detail;
} core_machine_run_result;

ntvdm64_status core_machine_create(
    const core_machine_config *config,
    core_machine **out_machine);

ntvdm64_status core_machine_reset(core_machine *machine);

ntvdm64_status core_machine_get_lifecycle(
    const core_machine *machine,
    core_machine_lifecycle *out_lifecycle);

ntvdm64_status core_machine_get_cpu_state(
    const core_machine *machine,
    core_machine_cpu_state *out_state);

ntvdm64_status core_machine_run(
    core_machine *machine,
    core_machine_run_budget budget,
    core_machine_run_result *result);

ntvdm64_status core_machine_request_stop(core_machine *machine);

ntvdm64_status core_machine_report_fault(
    core_machine *machine,
    uint32_t detail);

void core_machine_destroy(core_machine *machine);

#ifdef __cplusplus
}
#endif

#endif
