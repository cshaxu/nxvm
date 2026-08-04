/* Copyright 2012-2014 Neko. */

#ifndef VM_SESSION_MACHINE_DEVICES_H
#define VM_SESSION_MACHINE_DEVICES_H

#include "type.h"

typedef struct vm_session vm_session;

C_VOID vm_session_machine_devices_initialize_media(vm_session *session);
C_VOID vm_session_machine_devices_initialize_cmos(vm_session *session);
C_VOID vm_session_machine_devices_initialize_fdc(vm_session *session);
C_VOID vm_session_machine_devices_reset_cmos(vm_session *session);
C_VOID vm_session_machine_devices_refresh_cmos(vm_session *session);
C_VOID vm_session_machine_devices_refresh(vm_session *session);
C_VOID vm_session_machine_devices_reset(vm_session *session);
C_VOID vm_session_machine_devices_finalize(vm_session *session);

#endif
