#ifndef VM_PRODUCT_DEBUGGER_H
#define VM_PRODUCT_DEBUGGER_H

#include "core/machine/debug_interface.h"

typedef struct vm_product_debugger {
    core_machine *machine;
    int command_boundary_open;
} vm_product_debugger;

ntvdm64_status vm_product_debugger_initialize(
    vm_product_debugger *debugger, core_machine *machine);
ntvdm64_status vm_product_debugger_open_command_boundary(
    vm_product_debugger *debugger);
ntvdm64_status vm_product_debugger_read_cpu(
    const vm_product_debugger *debugger, core_machine_cpu_state *out_state);
ntvdm64_status vm_product_debugger_read_memory(
    const vm_product_debugger *debugger, uint32_t physical,
    void *out_data, size_t size);
ntvdm64_status vm_product_debugger_step(
    vm_product_debugger *debugger, core_machine_run_result *out_result);
ntvdm64_status vm_product_debugger_continue(
    vm_product_debugger *debugger, core_machine_run_budget budget,
    core_machine_run_result *out_result);
void vm_product_debugger_close_command_boundary(
    vm_product_debugger *debugger);

#endif
