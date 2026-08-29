/* Copyright 2012-2014 Neko. */

#ifndef VM_SESSION_MACHINE_DEVICES_H
#define VM_SESSION_MACHINE_DEVICES_H

#include "type.h"
#include "core/machine/machine_interface.h"

typedef struct vm_session vm_session;

type_status vm_session_machine_devices_initialize_media(vm_session *session);
type_status vm_session_machine_devices_materialize_plan(vm_session *session,
    core_machine_plan *plan);
C_VOID vm_session_machine_devices_reset(vm_session *session);
C_VOID vm_session_machine_devices_finalize(vm_session *session);

#endif
