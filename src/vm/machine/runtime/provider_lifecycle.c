/* Copyright 2012-2014 Neko. */

/* Preserves the retained ordering between VM-only devices and profile firmware. */

#include "type.h"

#include "vm/machine/runtime/machine_private.h"

#include "vm/machine/runtime/machine_devices.h"
#include "vm/machine/runtime/provider_lifecycle.h"
#include "vm/machine/runtime/machine_interface.h"

type_status vm_machine_provider_lifecycle_initialize(vm_machine *session)
{
    type_status status;

    if (session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;

    status = vm_machine_devices_initialize_media(session);
    if (status != TYPE_STATUS_OK) return status;
    return vm_machine_devices_bind_media(session);
}

C_VOID vm_machine_provider_lifecycle_reset(vm_machine *session)
{
    if (session == STD_NULL) return;
    vm_machine_devices_reset(session);
}

C_VOID vm_machine_provider_lifecycle_finalize(vm_machine *session)
{
    if (session == STD_NULL) return;
    vm_machine_devices_finalize(session);
}
