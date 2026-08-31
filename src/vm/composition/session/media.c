#include "type.h"

#include "vm/composition/session/session_private.h"
#include "vm/composition/session/media.h"

#include "core/machine/media_interface.h"

#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"

#include "vm/composition/session/session_interface.h"

type_status vm_session_bind_media(vm_session *machine)
{
    type_status status;

    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_media_registry_bind(machine->media_registry,
            VM_SESSION_MEDIA_FDD_ID, &machine->fdd,
            vm_machine_fdd_media_provider());
    if (status != TYPE_STATUS_OK) return status;
    if (machine->model40_private || machine->xt_private ||
        (machine->profile != STD_NULL && machine->profile->hdc_present)) {
        status = core_machine_media_registry_bind(machine->media_registry,
                VM_SESSION_MEDIA_HDD_ID, &machine->hdd,
                vm_machine_hdd_media_provider());
        if (status != TYPE_STATUS_OK) return status;
    }
    return core_machine_media_registry_freeze(machine->media_registry);
}
