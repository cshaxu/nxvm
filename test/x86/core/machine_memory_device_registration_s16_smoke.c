#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86/core/machine_interface.h"

static lib_status overlay_read(void *opaque, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    lib_u8 *value = (lib_u8 *)opaque;

    (void)physical;
    (void)observe_only;
    if (value == LIB_NULL || destination == 0u || bytes != 1u || *value == 0u) {
        return LIB_STATUS_UNSUPPORTED;
    }
    if (*value == 0xffu) return LIB_STATUS_IO_ERROR;
    *(lib_u8 *)destination = *value;
    return LIB_STATUS_OK;
}

static lib_status overlay_write(void *opaque, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    lib_u8 *value = (lib_u8 *)opaque;

    (void)physical;
    if (value == LIB_NULL || source == 0u || bytes != 1u || *value == 0u) {
        return LIB_STATUS_UNSUPPORTED;
    }
    *value = *(const lib_u8 *)source;
    return LIB_STATUS_OK;
}

static lib_status overlay_query(void *opaque, lib_u32 physical,
    lib_uptr bytes, core_machine_memory_access access)
{
    const lib_u8 *value = (const lib_u8 *)opaque;

    (void)physical;
    (void)access;
    return value != LIB_NULL && bytes == 1u && *value != 0u ? LIB_STATUS_OK :
        LIB_STATUS_UNSUPPORTED;
}

typedef struct dma_probe { lib_u32 before, after; lib_u16 observed; } dma_probe;
static void dma_before(void *owner, lib_u8 channel, lib_u16 *value)
{
    dma_probe *probe = owner;
    if (channel == 2u) ++probe->before;
    *value = 0x1234u;
}
static void dma_after(void *owner, lib_u8 channel, lib_u16 *value)
{
    dma_probe *probe = owner;
    if (channel == 2u) ++probe->after;
    probe->observed = *value;
}

static lib_i32 dma_memory_contract(core_machine *machine, lib_u8 *overlay)
{
    dma_probe probe = {0};
    lib_u16 value = 0u;
    for (lib_u8 bytes = 1u; bytes <= 2u; ++bytes) {
        if (core_machine_dma_memory_cycle(machine, 0x200u, bytes, 2u,
                CORE_MACHINE_MEMORY_ACCESS_WRITE, &value, dma_before, dma_after, &probe) != LIB_STATUS_OK ||
            core_machine_dma_memory_cycle(machine, 0x200u, bytes, 2u,
                CORE_MACHINE_MEMORY_ACCESS_READ, &value, dma_before, dma_after, &probe) != LIB_STATUS_OK ||
            (value & (bytes == 1u ? 0xffu : 0xffffu)) != (bytes == 1u ? 0x34u : 0x1234u) ||
            probe.before != bytes * 2u || probe.after != bytes * 2u) return 1;
    }
    if (core_machine_dma_memory_cycle(machine, 0x200u, 3u, 2u,
            CORE_MACHINE_MEMORY_ACCESS_READ, &value, dma_before, dma_after, &probe) != LIB_STATUS_INVALID_ARGUMENT ||
        probe.before != 4u || probe.after != 4u) return 1;
    *overlay = 0xffu;
    if (core_machine_dma_memory_cycle(machine, 0x000f0000u, 1u, 2u,
            CORE_MACHINE_MEMORY_ACCESS_READ, &value, dma_before, dma_after, &probe) != LIB_STATUS_IO_ERROR ||
        probe.before != 5u || probe.after != 4u) return 1;
    *overlay = 0x3cu;
    /* A failed memory stage must release the transaction for the next cycle. */
    if (core_machine_dma_memory_cycle(machine, 0x000f0000u, 1u, 2u,
            CORE_MACHINE_MEMORY_ACCESS_READ, &value, dma_before, dma_after, &probe) != LIB_STATUS_OK ||
        (probe.observed & 0xffu) != 0x3cu || probe.before != 6u || probe.after != 5u) return 1;
    return 0;
}

lib_i32 main(void)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    core_machine_memory_device_route provider = {
        0x000f0000u, 1u,
        { overlay_read, overlay_write, overlay_query },
        CORE_MACHINE_MEMORY_PROVIDER_OVERLAY
    };
    const lib_u8 rom = 0x5au;
    core_machine *machine = LIB_NULL;
    core_machine_memory_route route;
    lib_u8 overlay = 0u;
    lib_u8 observed = 0u;
    lib_u8 write = 0xa5u;
    lib_i32 failed = 0;

    failed |= core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK || machine == LIB_NULL;
    if (!failed) failed |= core_machine_install_memory_device_routes(machine,
        &provider, 1u, LIB_NULL, LIB_NULL, &overlay) != LIB_STATUS_OK ||
        core_machine_register_immutable_rom_mapping(machine, 0x000f0000u, &rom,
            sizeof(rom)) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_memory_read(machine, 0x000f0000u, &observed,
            sizeof(observed)) != LIB_STATUS_OK || observed != rom;
    overlay = 0x3cu;
    if (!failed) failed |= core_machine_memory_query(machine, 0x000f0000u, 1u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route) != LIB_STATUS_OK ||
        route != CORE_MACHINE_MEMORY_ROUTE_PROVIDER ||
        core_machine_memory_read(machine, 0x000f0000u, &observed,
            sizeof(observed)) != LIB_STATUS_OK || observed != overlay ||
        core_machine_memory_write(machine, 0x000f0000u, &write,
            sizeof(write)) != LIB_STATUS_OK || overlay != write;
    overlay = 0u;
    provider.physical_start = 0x000f1000u;
    if (!failed) failed |= core_machine_memory_read(machine, 0x000f0000u, &observed,
        sizeof(observed)) != LIB_STATUS_OK || observed != rom ||
        core_machine_install_memory_device_routes(machine, &provider, 1u,
            LIB_NULL, LIB_NULL, &overlay) != LIB_STATUS_INVALID_STATE;
    if (!failed) failed |= dma_memory_contract(machine, &overlay);
    overlay = 0x3cu;
    if (!failed) failed |= core_machine_remove_memory_device_routes(LIB_NULL, &overlay) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_remove_memory_device_routes(machine, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_remove_memory_device_routes(machine, &overlay) != LIB_STATUS_OK ||
        core_machine_memory_read(machine, 0x000f0000u, &observed, 1u) != LIB_STATUS_OK || observed != rom;
    lib_bool a20 = LIB_TRUE;
    if (!failed) failed |= core_machine_signal_a20(LIB_NULL, LIB_TRUE) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_observe_a20(machine, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_signal_a20(machine, LIB_FALSE) != LIB_STATUS_OK ||
        core_machine_observe_a20(machine, &a20) != LIB_STATUS_OK || a20 ||
        core_machine_signal_a20(machine, LIB_TRUE) != LIB_STATUS_OK ||
        core_machine_observe_a20(machine, &a20) != LIB_STATUS_OK || !a20;
    if (!failed) lib_c_printf("M5:T386:S16:CORE-MEMORY-DEVICE:OK\n");
    core_machine_destroy(machine);
    return failed ? 1 : 0;
}
