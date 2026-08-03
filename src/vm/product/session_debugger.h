#ifndef NXVM_PRODUCT_NXVM_DEBUGGER_H
#define NXVM_PRODUCT_NXVM_DEBUGGER_H

#include "core/machine/debug_interface.h"

typedef struct nxvm_product_nxvm_debugger {
    core_machine *machine;
    int command_boundary_open;
} nxvm_product_nxvm_debugger;

ntvdm64_status nxvm_product_nxvm_debugger_initialize(
    nxvm_product_nxvm_debugger *debugger, core_machine *machine);
ntvdm64_status nxvm_product_nxvm_debugger_open_command_boundary(
    nxvm_product_nxvm_debugger *debugger);
ntvdm64_status nxvm_product_nxvm_debugger_read_cpu(
    const nxvm_product_nxvm_debugger *debugger, core_machine_cpu_state *out_state);
ntvdm64_status nxvm_product_nxvm_debugger_read_memory(
    const nxvm_product_nxvm_debugger *debugger, uint32_t physical,
    void *out_data, size_t size);
ntvdm64_status nxvm_product_nxvm_debugger_step(
    nxvm_product_nxvm_debugger *debugger, core_machine_run_result *out_result);
ntvdm64_status nxvm_product_nxvm_debugger_continue(
    nxvm_product_nxvm_debugger *debugger, core_machine_run_budget budget,
    core_machine_run_result *out_result);
void nxvm_product_nxvm_debugger_close_command_boundary(
    nxvm_product_nxvm_debugger *debugger);

#endif
