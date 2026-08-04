/* Copyright 2012-2014 Neko. */

#ifndef VM_SESSION_PROFILE_FIRMWARE_H
#define VM_SESSION_PROFILE_FIRMWARE_H

#include "type.h"

typedef struct vm_session vm_session;

C_VOID vm_session_profile_firmware_initialize(vm_session *session);
C_VOID vm_session_profile_firmware_register_cmos(vm_session *session);
C_VOID vm_session_profile_firmware_register_keyboard(vm_session *session);
C_VOID vm_session_profile_firmware_register_dma(vm_session *session);
C_VOID vm_session_profile_firmware_register_fdc(vm_session *session);
C_VOID vm_session_profile_firmware_register_hdc(vm_session *session);
C_VOID vm_session_profile_firmware_register_core_posts(vm_session *session);
C_VOID vm_session_profile_firmware_refresh(vm_session *session);
C_VOID vm_session_profile_firmware_reset(vm_session *session);
C_VOID vm_session_profile_firmware_finalize(vm_session *session);

#endif
