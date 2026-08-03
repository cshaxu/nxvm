/* Provider callbacks injected into the core machine block interface. */
#ifndef NTVDM64_CORE_MACHINE_BLOCK_PROVIDER_H
#define NTVDM64_CORE_MACHINE_BLOCK_PROVIDER_H

#include "core/machine/block_interface.h"

typedef int (*core_machine_block_transfer_provider)(void *context, ntvdm64_type_unsigned_8 cylinder,
    ntvdm64_type_unsigned_8 head, ntvdm64_type_unsigned_8 sector, void *buffer, ntvdm64_type_native_unsigned byte_count);
typedef void (*core_machine_block_geometry_provider)(void *context,
    core_machine_block_geometry *out_geometry);

typedef struct core_machine_block_provider_slot {
    void *context;
    core_machine_block_geometry_provider geometry_provider;
    core_machine_block_transfer_provider read_provider;
    core_machine_block_transfer_provider write_provider;
    ntvdm64_type_bool frozen;
} core_machine_block_provider_slot;

void core_machine_block_provider_slot_initialize(
    core_machine_block_provider_slot *slot);
void core_machine_block_provider_slot_bind(
    core_machine_block_provider_slot *slot, void *context,
    core_machine_block_geometry_provider geometry_provider,
    core_machine_block_transfer_provider read_provider,
    core_machine_block_transfer_provider write_provider);
void core_machine_block_provider_slot_freeze(
    core_machine_block_provider_slot *slot);
void core_machine_block_provider_slot_finalize(
    core_machine_block_provider_slot *slot);

#endif
