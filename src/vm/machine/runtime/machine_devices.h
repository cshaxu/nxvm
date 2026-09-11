/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_MACHINE_DEVICES_H
#define VM_MACHINE_MACHINE_DEVICES_H

#include "type.h"
#include "core/machine/machine_interface.h"

typedef struct vm_machine vm_machine;

type_status vm_machine_devices_initialize_media(vm_machine *session);
type_status vm_machine_devices_materialize_plan(vm_machine *session,
    core_machine_plan *plan);
C_VOID vm_machine_devices_reset(vm_machine *session);
C_VOID vm_machine_devices_finalize(vm_machine *session);

#endif
