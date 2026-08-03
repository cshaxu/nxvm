/* Product-neutral block capability bound by root composition. */
#ifndef NTVDM64_CORE_MACHINE_BLOCK_H
#define NTVDM64_CORE_MACHINE_BLOCK_H

#include "type.h"

typedef struct core_machine_block_geometry {
    ntvdm64_type_bool present;
    ntvdm64_type_unsigned_16 cylinders;
    ntvdm64_type_unsigned_8 heads;
    ntvdm64_type_unsigned_8 sectors;
    ntvdm64_type_unsigned_16 bytes_per_sector;
} core_machine_block_geometry;

typedef struct core_machine_block_provider_slot core_machine_block_provider_slot;

void core_machine_block_get_geometry_from(
    const core_machine_block_provider_slot *slot,
    core_machine_block_geometry *out_geometry);
int core_machine_block_read_from(const core_machine_block_provider_slot *slot,
    ntvdm64_type_unsigned_8 cylinder, ntvdm64_type_unsigned_8 head, ntvdm64_type_unsigned_8 sector, void *buffer,
    ntvdm64_type_native_unsigned byte_count);
int core_machine_block_write_from(const core_machine_block_provider_slot *slot,
    ntvdm64_type_unsigned_8 cylinder, ntvdm64_type_unsigned_8 head, ntvdm64_type_unsigned_8 sector, void *buffer,
    ntvdm64_type_native_unsigned byte_count);

#endif
