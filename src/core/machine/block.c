#include "core/machine/block_interface.h"
#include "core/machine/block_provider.h"

void core_machine_block_provider_slot_initialize(
    core_machine_block_provider_slot *slot)
{
    if (slot != NULL) STD_MEMSET(slot, NTVDM64_TYPE_ZERO_8, sizeof(*slot));
}

void core_machine_block_provider_slot_bind(
    core_machine_block_provider_slot *slot, void *context,
    core_machine_block_geometry_provider geometry_provider,
    core_machine_block_transfer_provider read_provider,
    core_machine_block_transfer_provider write_provider)
{
    if (slot == NULL || slot->frozen) return;
    slot->context = context;
    slot->geometry_provider = geometry_provider;
    slot->read_provider = read_provider;
    slot->write_provider = write_provider;
}

void core_machine_block_provider_slot_freeze(
    core_machine_block_provider_slot *slot)
{
    if (slot != NULL) slot->frozen = NTVDM64_TYPE_TRUE;
}

void core_machine_block_provider_slot_finalize(
    core_machine_block_provider_slot *slot)
{
    if (slot != NULL) STD_MEMSET(slot, NTVDM64_TYPE_ZERO_8, sizeof(*slot));
}

void core_machine_block_get_geometry_from(
    const core_machine_block_provider_slot *slot,
    core_machine_block_geometry *out_geometry)
{
    if (out_geometry == NULL) return;
    STD_MEMSET(out_geometry, NTVDM64_TYPE_ZERO_8, sizeof(*out_geometry));
    if (slot != NULL && slot->geometry_provider != NULL) {
        slot->geometry_provider(slot->context, out_geometry);
    }
}

int core_machine_block_read_from(const core_machine_block_provider_slot *slot,
    ntvdm64_type_unsigned_8 cylinder, ntvdm64_type_unsigned_8 head, ntvdm64_type_unsigned_8 sector, void *buffer,
    ntvdm64_type_native_unsigned byte_count)
{
    return slot != NULL && slot->read_provider != NULL &&
        slot->read_provider(slot->context, cylinder, head, sector, buffer,
            byte_count);
}

int core_machine_block_write_from(const core_machine_block_provider_slot *slot,
    ntvdm64_type_unsigned_8 cylinder, ntvdm64_type_unsigned_8 head, ntvdm64_type_unsigned_8 sector, void *buffer,
    ntvdm64_type_native_unsigned byte_count)
{
    return slot != NULL && slot->write_provider != NULL &&
        slot->write_provider(slot->context, cylinder, head, sector, buffer,
            byte_count);
}
