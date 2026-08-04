/* Product-neutral block capability bound by root composition. */
#ifndef CORE_MACHINE_BLOCK_H
#define CORE_MACHINE_BLOCK_H

#include "type.h"

typedef struct core_machine_block_geometry {
    type_bool present;
    type_unsigned_16 cylinders;
    type_unsigned_8 heads;
    type_unsigned_8 sectors;
    type_unsigned_16 bytes_per_sector;
} core_machine_block_geometry;

typedef struct core_machine_block_provider_slot core_machine_block_provider_slot;

C_VOID core_machine_block_get_geometry_from(
    const core_machine_block_provider_slot *slot,
    core_machine_block_geometry *out_geometry);
C_INT core_machine_block_read_from(const core_machine_block_provider_slot *slot,
    type_unsigned_8 cylinder, type_unsigned_8 head, type_unsigned_8 sector, C_VOID *buffer,
    type_native_unsigned byte_count);
C_INT core_machine_block_write_from(const core_machine_block_provider_slot *slot,
    type_unsigned_8 cylinder, type_unsigned_8 head, type_unsigned_8 sector, C_VOID *buffer,
    type_native_unsigned byte_count);

#endif
