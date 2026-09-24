/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_MACHINE_DEVICES_H
#define VM_MACHINE_MACHINE_DEVICES_H
#include "lib/types/types_interface.h"

#include "app-nxvm/devices/machine_interface.h"

typedef struct vm_machine vm_machine;

lib_status vm_machine_devices_initialize_media(vm_machine *session);
lib_status vm_machine_devices_bind_media(vm_machine *session);
void vm_machine_devices_reset(vm_machine *session);
void vm_machine_devices_finalize(vm_machine *session);

#endif
