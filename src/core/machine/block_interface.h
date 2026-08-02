/* Product-neutral block capability bound by root composition. */
#ifndef NTVDM64_CORE_MACHINE_BLOCK_H
#define NTVDM64_CORE_MACHINE_BLOCK_H

#include "core/machine/vglobal.h"

typedef struct core_machine_block_geometry {
    t_bool present;
    t_nubit16 cylinders;
    t_nubit8 heads;
    t_nubit8 sectors;
    t_nubit16 bytes_per_sector;
} core_machine_block_geometry;

typedef struct core_machine_block_provider_slot core_machine_block_provider_slot;

void core_machine_block_get_geometry_from(
    const core_machine_block_provider_slot *slot,
    core_machine_block_geometry *out_geometry);
int core_machine_block_read_from(const core_machine_block_provider_slot *slot,
    t_nubit8 cylinder, t_nubit8 head, t_nubit8 sector, void *buffer,
    t_nubitcc byte_count);
int core_machine_block_write_from(const core_machine_block_provider_slot *slot,
    t_nubit8 cylinder, t_nubit8 head, t_nubit8 sector, void *buffer,
    t_nubitcc byte_count);

void core_machine_block_get_geometry(core_machine_block_geometry *out_geometry);
int core_machine_block_read(t_nubit8 cylinder, t_nubit8 head, t_nubit8 sector,
    void *buffer, t_nubitcc byte_count);
int core_machine_block_write(t_nubit8 cylinder, t_nubit8 head, t_nubit8 sector,
    void *buffer, t_nubitcc byte_count);

#endif
