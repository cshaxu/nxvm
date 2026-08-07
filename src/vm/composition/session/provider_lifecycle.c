/* Copyright 2012-2014 Neko. */

/* Preserves the retained ordering between VM-only devices and profile firmware. */

#include "type.h"

#include "vm/composition/session/session.h"

#include "vm/composition/session/media.h"
#include "vm/composition/session/machine_devices.h"
#include "vm/composition/session/profile_firmware.h"
#include "vm/composition/session/provider_lifecycle.h"
#include "vm/composition/session/session_interface.h"

C_VOID vm_session_provider_lifecycle_initialize(vm_session *session)
{
    if (session == STD_NULL) return;

    vm_session_machine_devices_initialize_media(session);
    vm_session_profile_firmware_initialize(session);
    vm_session_machine_devices_initialize_cmos(session);
    vm_session_profile_firmware_register_cmos(session);
    vm_session_machine_devices_reset_cmos(session);
    vm_session_machine_devices_refresh_cmos(session);
    vm_session_profile_firmware_register_keyboard(session);
    vm_session_profile_firmware_register_dma(session);
    vm_session_machine_devices_initialize_fdc(session);
    if (!vm_session_machine_devices_initialize_hdc(session)) return;
    vm_session_bind_media(session);
    vm_session_profile_firmware_register_fdc(session);
    vm_session_profile_firmware_register_hdc(session);
    vm_session_profile_firmware_register_core_posts(session);
}

C_VOID vm_session_provider_lifecycle_refresh(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_session_profile_firmware_refresh(session);
    vm_session_machine_devices_refresh(session);
}

C_VOID vm_session_provider_lifecycle_advance(vm_session *session,
    uint64_t elapsed_ticks)
{
    if (session == STD_NULL) return;
    vm_session_machine_devices_advance(session, elapsed_ticks);
}

C_VOID vm_session_provider_lifecycle_reset(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_session_machine_devices_reset(session);
    vm_session_profile_firmware_reset(session);
}

C_VOID vm_session_provider_lifecycle_finalize(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_session_profile_firmware_finalize(session);
    vm_session_machine_devices_finalize(session);
}
