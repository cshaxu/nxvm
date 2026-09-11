#ifndef VM_MACHINE_MODEL40_COMPOSITION_H
#define VM_MACHINE_MODEL40_COMPOSITION_H

#include "type.h"
#include "core/machine/machine_interface.h"

typedef struct vm_machine vm_machine;

type_status vm_machine_model40_topology_materialize(vm_machine *session,
    core_machine_plan_topology *out_topology);
type_status vm_machine_model40_materialize_plan(vm_machine *session,
    core_machine_plan *plan);
C_INT vm_machine_model40_insert_hdd_at_startup(vm_machine *session, const C_CHAR *path);

#endif
