#include "type.h"

#include "vm/composition/session/session.h"

#include "core/machine/block_provider.h"

#include "vm/machine/hdc.h"

#include "vm/composition/session/session_interface.h"

static C_VOID vmBlockGeometry(C_VOID *context, core_machine_block_geometry *out_geometry)
{
    vm_machine_hdc_get_geometry((const vm_machine_hdc *)context, out_geometry);
}

C_VOID vm_session_bind_block(vm_session *machine)
{
    if (machine == STD_NULL) return;
    core_machine_block_provider_slot_bind(&machine->block_provider, &machine->hdc,
        vmBlockGeometry, STD_NULL, STD_NULL);
    core_machine_block_provider_slot_freeze(&machine->block_provider);
}
