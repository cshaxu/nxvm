#ifndef VM_SESSION_VIRTUAL_TIME_H
#define VM_SESSION_VIRTUAL_TIME_H

#include "type.h"

#include "core/machine/machine_interface.h"

typedef struct vm_session vm_session;

/* Handles one core wait result. The runner is the sole production caller. */
type_status vm_session_virtual_time_on_waiting(vm_session *session,
    const core_machine_run_result *result, C_INT *out_advanced);
C_VOID vm_session_virtual_time_reset(vm_session *session);

#endif
