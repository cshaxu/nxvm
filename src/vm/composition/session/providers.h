/* Copyright 2012-2014 Neko. */

#ifndef NTVDM64_VM_SESSION_PROVIDERS_H
#define NTVDM64_VM_SESSION_PROVIDERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "vm/composition/session/session.h"

#define VM_SESSION_MACHINE_NAME "IBM PC/AT"

C_VOID vm_session_providers_initialize(vm_session *machine);
C_VOID vm_session_providers_refresh(vm_session *machine);
C_VOID vm_session_providers_reset(vm_session *machine);
C_VOID vm_session_providers_finalize(vm_session *machine);
C_VOID vm_session_print_machine(const vm_session *machine);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
