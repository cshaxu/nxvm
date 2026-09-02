/* Copyright 2012-2014 Neko. */

#ifndef VM_SESSION_PROFILE_FIRMWARE_H
#define VM_SESSION_PROFILE_FIRMWARE_H

#include "type.h"

typedef struct vm_session vm_session;

type_status vm_session_profile_firmware_initialize(vm_session *session);
C_INT vm_session_profile_firmware_is_external(const vm_session *session);
C_VOID vm_session_profile_firmware_register_cmos(vm_session *session);
C_VOID vm_session_profile_firmware_register_keyboard(vm_session *session);
C_VOID vm_session_profile_firmware_register_dma(vm_session *session);
C_VOID vm_session_profile_firmware_register_fdc(vm_session *session);
C_VOID vm_session_profile_firmware_register_hdc(vm_session *session);
C_VOID vm_session_profile_firmware_register_core_posts(vm_session *session);
C_VOID vm_session_profile_firmware_finalize(vm_session *session);
const core_machine_firmware_provider *vm_session_profile_firmware_provider(C_VOID);

#endif
