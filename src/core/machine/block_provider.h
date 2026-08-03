/* Provider callbacks injected into the core machine block interface. */
#ifndef NTVDM64_CORE_MACHINE_BLOCK_PROVIDER_H
#define NTVDM64_CORE_MACHINE_BLOCK_PROVIDER_H


#include "type.h"
#include "core/machine/block_interface.h"

typedef C_INT (*core_machine_block_transfer_provider)(C_VOID *context, ntvdm64_type_unsigned_8 cylinder,
    ntvdm64_type_unsigned_8 head, ntvdm64_type_unsigned_8 sector, C_VOID *buffer, ntvdm64_type_native_unsigned byte_count);
typedef C_VOID (*core_machine_block_geometry_provider)(C_VOID *context,
    core_machine_block_geometry *out_geometry);

typedef struct core_machine_block_provider_slot {
    C_VOID *context;
    core_machine_block_geometry_provider geometry_provider;
    core_machine_block_transfer_provider read_provider;
    core_machine_block_transfer_provider write_provider;
    ntvdm64_type_bool frozen;
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
