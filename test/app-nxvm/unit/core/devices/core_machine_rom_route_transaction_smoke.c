#include "lib/types/types_interface.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include <stdio.h>

#include "x86/core/machine.h"

typedef struct rom_probe {
    lib_u32 mode;
} rom_probe;

static lib_status filler_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    (void)owner;
    (void)physical;
    (void)destination;
    (void)bytes;
    (void)observe_only;
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status filler_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    (void)owner;
    (void)physical;
    (void)source;
    (void)bytes;
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status filler_query(void *owner, lib_u32 physical,
    lib_uptr bytes, core_machine_memory_access access)
{
    (void)owner;
    (void)physical;
    (void)bytes;
    (void)access;
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status configure_rom(void *owner,
    core_machine_firmware_context *firmware)
{
    static const lib_u8 reset_image[16] = { 0xeau };
    static const lib_u8 ordinary_image[2] = { 0x5au, 0xa5u };
    const rom_probe *probe = (const rom_probe *)owner;
    lib_status status;

    if (probe->mode == 0u || probe->mode == 3u || probe->mode == 4u)
        return core_machine_firmware_register_immutable_rom(firmware,
            0x000ffff0u, reset_image, sizeof(reset_image));
    status = core_machine_firmware_register_immutable_rom(firmware,
        0x000e0000u, ordinary_image, sizeof(ordinary_image));
    if (status != LIB_STATUS_OK) return status;
    return core_machine_firmware_register_immutable_rom_alias(firmware,
        probe->mode == 1u ? 0x000b0000u : 0x000e0000u,
        0x000c0000u, sizeof(ordinary_image));
}

static lib_status reset_rom(void *owner,
    core_machine_firmware_context *firmware)
{
    (void)owner;
    (void)firmware;
    return LIB_STATUS_OK;
}

static const core_machine_firmware_provider rom_provider = {
    configure_rom, reset_rom, LIB_NULL
};

static lib_i32 run_case(lib_u32 mode)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const lib_u8 prior_image = 0x96u;
    core_machine *machine = LIB_NULL;
    rom_probe probe = { mode };
    lib_size provider_count;
    lib_size mapping_count;
    lib_u8 observed = 0u;
    lib_i32 filler = 0;
    lib_i32 failed = 0;
    lib_status expected = mode == 1u ? LIB_STATUS_INVALID_ARGUMENT :
        LIB_STATUS_NO_MEMORY;

    if (core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK) return 1;
    failed |= core_machine_register_immutable_rom_mapping(machine,
        0x000d0000u, &prior_image, 1u) != LIB_STATUS_OK;
    if (mode != 1u) {
        const lib_size target = CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT -
            (mode == 3u ? 0u : 1u);
        const lib_size remaining = target -
            machine->executor_memory.connect.device_provider_count;

        for (lib_size index = 0u; index < remaining; ++index)
            failed |= core_machine_memory_register_device_provider(
                &machine->executor_memory, 0x1000u + (lib_u32)(index * 16u),
                1u, filler_read, filler_write, filler_query, &filler) !=
                LIB_STATUS_OK;
    }
    provider_count = machine->executor_memory.connect.device_provider_count;
    mapping_count = machine->immutable_rom_mapping_count;
    failed |= core_machine_bind_firmware_provider(machine, &rom_provider,
        &probe) != expected ||
        machine->immutable_rom_mapping_count != mapping_count ||
        machine->executor_memory.connect.device_provider_count != provider_count ||
        machine->firmware_provider != LIB_NULL;
    failed |= core_machine_memory_read_physical(&machine->executor_memory,
        0x000d0000u, (lib_uptr)&observed, 1u) != LIB_STATUS_OK ||
        observed != prior_image;

    core_machine_memory_unregister_owner(&machine->executor_memory, &filler);
    probe.mode = 4u;
    failed |= core_machine_bind_firmware_provider(machine, &rom_provider,
        &probe) != LIB_STATUS_OK ||
        machine->immutable_rom_mapping_count != mapping_count + 2u ||
        machine->executor_memory.connect.device_provider_count != 3u;
    observed = 0u;
    failed |= core_machine_memory_read_reset_physical(&machine->executor_memory,
        0xfffffff0u, (lib_uptr)&observed, 1u) != LIB_STATUS_OK ||
        observed != 0xeau;
    core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    lib_i32 failed = 0;

    for (lib_u32 mode = 0u; mode < 4u; ++mode) failed |= run_case(mode);
    if (!failed) puts("M5:T540:S18:ROM-ROUTE-TRANSACTION:OK");
    return failed ? 1 : 0;
}
