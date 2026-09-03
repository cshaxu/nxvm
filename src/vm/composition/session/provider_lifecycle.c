/* Copyright 2012-2014 Neko. */

/* Preserves the retained ordering between VM-only devices and profile firmware. */

#include "type.h"

#include "vm/composition/session/session_private.h"

#include "vm/composition/session/media.h"
#include "vm/composition/session/machine_devices.h"
#include "vm/composition/session/provider_lifecycle.h"
#include "vm/composition/session/session_interface.h"

type_status vm_session_provider_lifecycle_initialize(vm_session *session)
{
    type_status status;

    if (session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;

    status = vm_session_machine_devices_initialize_media(session);
    if (status != TYPE_STATUS_OK) return status;
    return vm_session_bind_media(session);
}

C_VOID vm_session_provider_lifecycle_reset(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_session_machine_devices_reset(session);
}

C_VOID vm_session_provider_lifecycle_finalize(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_session_machine_devices_finalize(session);
}
