#ifndef VM_SESSION_MODEL40_COMPOSITION_H
#define VM_SESSION_MODEL40_COMPOSITION_H

#include "type.h"
#include "core/machine/machine_interface.h"

typedef struct vm_session vm_session;

type_status vm_session_model40_topology_materialize(vm_session *session,
    core_machine_plan_topology *out_topology);
type_status vm_session_model40_materialize_plan(vm_session *session,
    core_machine_plan *plan);
C_INT vm_session_model40_insert_hdd_at_startup(vm_session *session, const C_CHAR *path);

#endif
