/* Copyright 2012-2014 Neko. */

#ifndef X86_NXVM_MACHINE_MACHINE_DEVICES_H
#define X86_NXVM_MACHINE_MACHINE_DEVICES_H
#include "lib/types/types_interface.h"

#include "x86-core/machine/machine_interface.h"

typedef struct x86_nxvm_machine x86_nxvm_machine;

lib_status x86_nxvm_machine_devices_initialize_media(x86_nxvm_machine *session);
lib_status x86_nxvm_machine_devices_bind_media(x86_nxvm_machine *session);
void x86_nxvm_machine_devices_reset(x86_nxvm_machine *session);
void x86_nxvm_machine_devices_finalize(x86_nxvm_machine *session);

#endif
