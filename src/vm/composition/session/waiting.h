#ifndef VM_SESSION_WAITING_H
#define VM_SESSION_WAITING_H

#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_interface.h"

type_status vm_session_waiting_advance(vm_session *session,
    const core_machine_run_result *result, C_INT *out_advanced);

#endif
