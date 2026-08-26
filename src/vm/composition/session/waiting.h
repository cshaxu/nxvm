#ifndef VM_SESSION_WAITING_H
#define VM_SESSION_WAITING_H

#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_interface.h"

type_status vm_session_waiting_advance(vm_session *session,
    const core_machine_run_result *result, C_INT *out_advanced);
C_INT vm_session_pacing_ratio_compare(type_unsigned_64 left_numerator,
    type_unsigned_64 left_denominator, type_unsigned_64 right_numerator,
    type_unsigned_64 right_denominator);
type_status vm_session_pacing_wait(vm_session *session);
C_VOID vm_session_pacing_reset(vm_session *session);

#endif
