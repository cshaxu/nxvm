/* Provider callbacks injected into the core machine block interface. */
#ifndef CORE_MACHINE_BLOCK_PROVIDER_H
#define CORE_MACHINE_BLOCK_PROVIDER_H


#include "type.h"
#include "core/machine/block_interface.h"

typedef C_INT (*core_machine_block_transfer_provider)(C_VOID *context, type_unsigned_8 cylinder,
    type_unsigned_8 head, type_unsigned_8 sector, C_VOID *buffer, type_native_unsigned byte_count);
typedef C_VOID (*core_machine_block_geometry_provider)(C_VOID *context,
    core_machine_block_geometry *out_geometry);

typedef struct core_machine_block_provider_slot {
    C_VOID *context;
    core_machine_block_geometry_provider geometry_provider;
    core_machine_block_transfer_provider read_provider;
    core_machine_block_transfer_provider write_provider;
    type_bool frozen;
} core_machine_block_provider_slot;

C_VOID core_machine_block_provider_slot_initialize(
    core_machine_block_provider_slot *slot);
C_VOID core_machine_block_provider_slot_bind(
    core_machine_block_provider_slot *slot, C_VOID *context,
    core_machine_block_geometry_provider geometry_provider,
    core_machine_block_transfer_provider read_provider,
    core_machine_block_transfer_provider write_provider);
C_VOID core_machine_block_provider_slot_freeze(
    core_machine_block_provider_slot *slot);
C_VOID core_machine_block_provider_slot_finalize(
    core_machine_block_provider_slot *slot);

#endif
