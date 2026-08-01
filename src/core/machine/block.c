#include "core/machine/block.h"

static void *coreBlockContext;
static core_block_geometry_query coreBlockGeometryQuery;
static core_block_transfer coreBlockReadTransfer;
static core_block_transfer coreBlockWriteTransfer;

void coreBlockBind(void *context, core_block_geometry_query geometry_query,
    core_block_transfer read, core_block_transfer write)
{
    coreBlockContext = context;
    coreBlockGeometryQuery = geometry_query;
    coreBlockReadTransfer = read;
    coreBlockWriteTransfer = write;
}

void coreBlockGetGeometry(core_block_geometry *out_geometry)
{
    MEMSET(out_geometry, Zero8, sizeof(*out_geometry));
    if (coreBlockGeometryQuery != NULL) {
        coreBlockGeometryQuery(coreBlockContext, out_geometry);
    }
}

int coreBlockRead(t_nubit8 cylinder, t_nubit8 head, t_nubit8 sector,
    void *buffer, t_nubitcc byte_count)
{
    return coreBlockReadTransfer != NULL && coreBlockReadTransfer(coreBlockContext,
        cylinder, head, sector, buffer, byte_count);
}

int coreBlockWrite(t_nubit8 cylinder, t_nubit8 head, t_nubit8 sector,
    void *buffer, t_nubitcc byte_count)
{
    return coreBlockWriteTransfer != NULL && coreBlockWriteTransfer(coreBlockContext,
        cylinder, head, sector, buffer, byte_count);
}
