#ifndef X86_NXVM_MACHINE_FAULT_H
#define X86_NXVM_MACHINE_FAULT_H
#include "lib/types/types_interface.h"


#include "x86-core/machine/machine_interface.h"

typedef struct x86_nxvm_machine x86_nxvm_machine;

typedef struct x86_nxvm_machine_fault_outcome {
    lib_bool valid;
    x86_core_machine_run_result run;
    x86_core_cpu_diagnostic diagnostic;
} x86_nxvm_machine_fault_outcome;

void x86_nxvm_machine_fault_clear(x86_nxvm_machine *session);
void x86_nxvm_machine_fault_capture(x86_nxvm_machine *session,
    const x86_core_machine_run_result *run);
lib_i32 x86_nxvm_machine_fault_get(const x86_nxvm_machine *session,
    x86_nxvm_machine_fault_outcome *out_outcome);

#endif
