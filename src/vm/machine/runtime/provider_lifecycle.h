/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_PROVIDER_LIFECYCLE_H
#define VM_MACHINE_PROVIDER_LIFECYCLE_H

#include "type.h"

typedef struct vm_machine vm_machine;

type_status vm_machine_provider_lifecycle_initialize(vm_machine *session);
C_VOID vm_machine_provider_lifecycle_reset(vm_machine *session);
C_VOID vm_machine_provider_lifecycle_finalize(vm_machine *session);

#endif
