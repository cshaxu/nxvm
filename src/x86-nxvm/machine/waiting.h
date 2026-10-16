#ifndef X86_NXVM_MACHINE_WAITING_H
#define X86_NXVM_MACHINE_WAITING_H
#include "lib/types/types_interface.h"


#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/machine/machine_interface.h"

lib_status x86_nxvm_machine_waiting_advance(x86_nxvm_machine *session,
    const x86_core_machine_run_result *result, lib_bool *out_advanced);
lib_i32 x86_nxvm_machine_pacing_ratio_compare(lib_u64 left_numerator,
    lib_u64 left_denominator, lib_u64 right_numerator,
    lib_u64 right_denominator);
lib_status x86_nxvm_machine_pacing_wait(x86_nxvm_machine *session);
void x86_nxvm_machine_pacing_reset(x86_nxvm_machine *session);

#endif
