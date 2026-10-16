/* Copyright 2012-2014 Neko. */

#ifndef X86_NXVM_MACHINE_LIFECYCLE_H
#define X86_NXVM_MACHINE_LIFECYCLE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"



#include "x86-nxvm/machine/machine_interface.h"
#include "x86-nxvm/machine/control.h"

lib_status x86_nxvm_machine_reset(x86_nxvm_machine *machine);
lib_status x86_nxvm_machine_finish_reset(x86_nxvm_machine *machine, lib_status status);
void x86_nxvm_machine_stop(x86_nxvm_machine *machine);
lib_status x86_nxvm_machine_resume(x86_nxvm_machine *machine);
lib_status x86_nxvm_machine_request_pause(x86_nxvm_machine *machine);

lib_status x86_nxvm_machine_initialize(x86_nxvm_machine *machine);
void x86_nxvm_machine_finalize(x86_nxvm_machine *machine);

lib_status x86_nxvm_machine_bind_execution_provider(x86_nxvm_machine *machine);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
