/* Copyright 2012-2014 Neko. */

/* Preserves the retained ordering between VM-only devices and profile firmware. */

#include "type.h"

#include "vm/composition/session/session_private.h"

#include "vm/composition/session/media.h"
#include "vm/composition/session/machine_devices.h"
#include "vm/composition/session/profile_firmware.h"
#include "vm/composition/session/provider_lifecycle.h"
#include "vm/composition/session/session_interface.h"

type_status vm_session_provider_lifecycle_initialize(vm_session *session)
{
    type_status status;

    if (session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;

    status = vm_session_machine_devices_initialize_media(session);
    if (status != TYPE_STATUS_OK) return status;
    if (session->firmware_kind != VM_SESSION_FIRMWARE_DEFAULT_PC_AT) {
        return vm_session_bind_media(session);
    }
    status = vm_session_profile_firmware_initialize(session);
    if (status != TYPE_STATUS_OK) return status;
    vm_session_profile_firmware_register_cmos(session);
    vm_session_profile_firmware_register_keyboard(session);
    vm_session_profile_firmware_register_dma(session);
    status = vm_session_bind_media(session);
    if (status != TYPE_STATUS_OK) return status;
    vm_session_profile_firmware_register_fdc(session);
    vm_session_profile_firmware_register_hdc(session);
    vm_session_profile_firmware_register_core_posts(session);
    return TYPE_STATUS_OK;
}

C_VOID vm_session_provider_lifecycle_refresh(vm_session *session)
{
    if (session != STD_NULL) vm_session_machine_devices_refresh(session);
}

C_VOID vm_session_provider_lifecycle_reset(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_session_machine_devices_reset(session);
    if (session->model40_private) {
        vm_profile_model40_d4_memory_reset(&session->model40_d4_memory);
    }
}

C_VOID vm_session_provider_lifecycle_finalize(vm_session *session)
{
    if (session == STD_NULL) return;
    if (session->firmware_kind == VM_SESSION_FIRMWARE_DEFAULT_PC_AT) {
        vm_session_profile_firmware_finalize(session);
    }
    vm_session_machine_devices_finalize(session);
}
