#include "core/machine/block_interface.h"

static void *coreBlockContext;
static core_machine_block_geometry_provider coreMachineBlockGeometryProvider;
static core_machine_block_transfer_provider coreMachineBlockReadProvider;
static core_machine_block_transfer_provider coreMachineBlockWriteProvider;

void core_machine_block_bind_provider(void *context,
    core_machine_block_geometry_provider geometry_provider,
    core_machine_block_transfer_provider read_provider,
    core_machine_block_transfer_provider write_provider)
{
    coreBlockContext = context;
    coreMachineBlockGeometryProvider = geometry_provider;
    coreMachineBlockReadProvider = read_provider;
    coreMachineBlockWriteProvider = write_provider;
}

void core_machine_block_get_geometry(core_machine_block_geometry *out_geometry)
{
    MEMSET(out_geometry, Zero8, sizeof(*out_geometry));
    if (coreMachineBlockGeometryProvider != NULL) {
        coreMachineBlockGeometryProvider(coreBlockContext, out_geometry);
    }
}

int core_machine_block_read(t_nubit8 cylinder, t_nubit8 head, t_nubit8 sector,
    void *buffer, t_nubitcc byte_count)
{
    return coreMachineBlockReadProvider != NULL && coreMachineBlockReadProvider(coreBlockContext,
        cylinder, head, sector, buffer, byte_count);
}

int core_machine_block_write(t_nubit8 cylinder, t_nubit8 head, t_nubit8 sector,
    void *buffer, t_nubitcc byte_count)
{
    return coreMachineBlockWriteProvider != NULL && coreMachineBlockWriteProvider(coreBlockContext,
        cylinder, head, sector, buffer, byte_count);
}
