#include "type.h"

#include "vm/composition/session/session.h"
#include "vm/composition/session/media.h"

#include "core/machine/media_interface.h"

#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"

#include "vm/composition/session/session_interface.h"

C_VOID vm_session_bind_media(vm_session *machine)
{
    if (machine == STD_NULL) return;
    if (core_machine_media_registry_bind(&machine->media_registry,
            VM_SESSION_MEDIA_FDD_ID, &machine->fdd,
            vm_machine_fdd_media_provider()) != TYPE_STATUS_OK ||
        core_machine_media_registry_bind(&machine->media_registry,
            VM_SESSION_MEDIA_HDD_ID, &machine->hdd,
            vm_machine_hdd_media_provider()) != TYPE_STATUS_OK) return;
    (C_VOID)core_machine_media_registry_freeze(&machine->media_registry);
}
