#include "type.h"

#include "core/machine/machine_interface.h"

static type_status overlay_read(C_VOID *opaque, type_unsigned_32 physical,
    type_virtual_address destination, type_native_unsigned bytes)
{
    type_unsigned_8 *value = (type_unsigned_8 *)opaque;

    (C_VOID)physical;
    if (value == STD_NULL || destination == 0u || bytes != 1u || *value == 0u) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    *(type_unsigned_8 *)destination = *value;
    return TYPE_STATUS_OK;
}

static type_status overlay_write(C_VOID *opaque, type_unsigned_32 physical,
    type_virtual_address source, type_native_unsigned bytes)
{
    type_unsigned_8 *value = (type_unsigned_8 *)opaque;

    (C_VOID)physical;
    if (value == STD_NULL || source == 0u || bytes != 1u || *value == 0u) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    *value = *(const type_unsigned_8 *)source;
    return TYPE_STATUS_OK;
}

static type_status overlay_query(C_VOID *opaque, type_unsigned_32 physical,
    type_native_unsigned bytes, core_machine_memory_access access)
{
    const type_unsigned_8 *value = (const type_unsigned_8 *)opaque;

    (C_VOID)physical;
    (C_VOID)access;
    return value != STD_NULL && bytes == 1u && *value != 0u ? TYPE_STATUS_OK :
        TYPE_STATUS_UNSUPPORTED;
}

C_INT main(C_VOID)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    const core_machine_memory_device_callbacks callbacks = {
        overlay_read, overlay_write, overlay_query
    };
    const type_unsigned_8 rom = 0x5au;
    core_machine *machine = STD_NULL;
    core_machine_memory_route route;
    type_unsigned_8 overlay = 0u;
    type_unsigned_8 observed = 0u;
    type_unsigned_8 write = 0xa5u;
    C_INT failed = 0;

    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK || machine == STD_NULL;
    if (!failed) failed |= core_machine_register_memory_device(machine, 0x000f0000u,
        1u, &callbacks, &overlay) != TYPE_STATUS_OK ||
        core_machine_register_immutable_rom_mapping(machine, 0x000f0000u, &rom,
            sizeof(rom)) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_read(machine, 0x000f0000u, &observed,
            sizeof(observed)) != TYPE_STATUS_OK || observed != rom;
    overlay = 0x3cu;
    if (!failed) failed |= core_machine_memory_query(machine, 0x000f0000u, 1u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route) != TYPE_STATUS_OK ||
        route != CORE_MACHINE_MEMORY_ROUTE_PROVIDER ||
        core_machine_memory_read(machine, 0x000f0000u, &observed,
            sizeof(observed)) != TYPE_STATUS_OK || observed != overlay ||
        core_machine_memory_write(machine, 0x000f0000u, &write,
            sizeof(write)) != TYPE_STATUS_OK || overlay != write;
    overlay = 0u;
    if (!failed) failed |= core_machine_memory_read(machine, 0x000f0000u, &observed,
        sizeof(observed)) != TYPE_STATUS_OK || observed != rom ||
        core_machine_register_memory_device(machine, 0x000f1000u, 1u,
            &callbacks, &overlay) != TYPE_STATUS_INVALID_STATE;
    if (!failed) STD_PRINTF("M5:T386:S16:CORE-MEMORY-DEVICE:OK\n");
    core_machine_destroy(machine);
    return failed ? 1 : 0;
}