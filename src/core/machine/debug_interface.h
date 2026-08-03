#ifndef NTVDM64_CORE_MACHINE_DEBUG_INTERFACE_H
#define NTVDM64_CORE_MACHINE_DEBUG_INTERFACE_H

#include "type.h"



#include "core/machine/cpu_interface.h"

#include "core/machine/machine_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

ntvdm64_status core_machine_debug_read_cpu(
    const core_machine *machine,
    core_machine_cpu_state *out_state);
ntvdm64_status core_machine_debug_read_memory(
    const core_machine *machine,
    uint32_t physical,
    C_VOID *out_data,
    STD_SIZE_T size);
ntvdm64_status core_machine_debug_step(
    core_machine *machine,
    core_machine_run_result *out_result);
ntvdm64_status core_machine_debug_continue(
    core_machine *machine,
    core_machine_run_budget budget,
    core_machine_run_result *out_result);

#ifdef __cplusplus
}
#endif

#endif
