#ifndef NXVM_PRODUCT_NXVM_DEBUGGER_H
#define NXVM_PRODUCT_NXVM_DEBUGGER_H

#include "core/machine/debug.h"

typedef struct nxvm_product_nxvm_debugger {
    nxvm_core_machine *machine;
    int command_boundary_open;
} nxvm_product_nxvm_debugger;

nxvm_core_status nxvm_product_nxvm_debugger_initialize(
    nxvm_product_nxvm_debugger *debugger, nxvm_core_machine *machine);
nxvm_core_status nxvm_product_nxvm_debugger_open_command_boundary(
    nxvm_product_nxvm_debugger *debugger);
nxvm_core_status nxvm_product_nxvm_debugger_read_cpu(
    const nxvm_product_nxvm_debugger *debugger, nxvm_core_cpu_state *out_state);
nxvm_core_status nxvm_product_nxvm_debugger_read_memory(
    const nxvm_product_nxvm_debugger *debugger, uint32_t physical,
    void *out_data, size_t size);
nxvm_core_status nxvm_product_nxvm_debugger_step(
    nxvm_product_nxvm_debugger *debugger, nxvm_core_run_result *out_result);
nxvm_core_status nxvm_product_nxvm_debugger_continue(
    nxvm_product_nxvm_debugger *debugger, nxvm_core_run_budget budget,
    nxvm_core_run_result *out_result);
void nxvm_product_nxvm_debugger_close_command_boundary(
    nxvm_product_nxvm_debugger *debugger);

#endif
