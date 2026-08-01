/* Product-neutral block capability bound by root composition. */
#ifndef NTVDM64_CORE_MACHINE_BLOCK_H
#define NTVDM64_CORE_MACHINE_BLOCK_H

#include "core/machine/vglobal.h"

typedef struct core_block_geometry {
    t_bool present;
    t_nubit16 cylinders;
    t_nubit8 heads;
    t_nubit8 sectors;
    t_nubit16 bytes_per_sector;
} core_block_geometry;

typedef int (*core_block_transfer)(void *context, t_nubit8 cylinder,
    t_nubit8 head, t_nubit8 sector, void *buffer, t_nubitcc byte_count);
typedef void (*core_block_geometry_query)(void *context,
    core_block_geometry *out_geometry);

void coreBlockBind(void *context, core_block_geometry_query geometry_query,
    core_block_transfer read, core_block_transfer write);
void coreBlockGetGeometry(core_block_geometry *out_geometry);
int coreBlockRead(t_nubit8 cylinder, t_nubit8 head, t_nubit8 sector,
    void *buffer, t_nubitcc byte_count);
int coreBlockWrite(t_nubit8 cylinder, t_nubit8 head, t_nubit8 sector,
    void *buffer, t_nubitcc byte_count);

#endif
