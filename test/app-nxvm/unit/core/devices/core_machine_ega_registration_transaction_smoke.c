#include "lib/types/types_interface.h"
#include <stdio.h>
#include "x86/core/memory.h"
#include "x86/core/machine.h"
#include "app-nxvm/devices/vadp.h"
#include "app-nxvm/devices/machine_board_interface.h"

static void ignored_write(void *owner, lib_u32 physical,
    lib_uptr bytes)
{
    (void)owner;
    (void)physical;
    (void)bytes;
}

static void sentinel_read(t_port *port, lib_u16 address, void *owner)
{
    (void)address;
    (void)owner;
    port->data.ioByte = 0x5au;
}

static lib_status ignored_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    (void)owner;
    (void)observe_only;
    (void)physical;
    (void)destination;
    (void)bytes;
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status ignored_device_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    (void)owner;
    (void)physical;
    (void)source;
    (void)bytes;
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status ignored_query(void *owner, lib_u32 physical,
    lib_uptr bytes, core_machine_memory_access access)
{
    (void)owner;
    (void)physical;
    (void)bytes;
    (void)access;
    return LIB_STATUS_UNSUPPORTED;
}

typedef struct priority_provider {
    lib_u8 value;
    lib_u8 decline;
} priority_provider;

static lib_status priority_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    priority_provider *provider = (priority_provider *)owner;

    (void)observe_only;
    if (provider == LIB_NULL || physical != 0x8000u || bytes != 1u) {
        return LIB_STATUS_INTERNAL_ERROR;
    }
    *(lib_u8 *)destination = provider->value;
    return LIB_STATUS_OK;
}

static lib_status priority_query(void *owner, lib_u32 physical,
    lib_uptr bytes, core_machine_memory_access access)
{
    priority_provider *provider = (priority_provider *)owner;

    if (provider == LIB_NULL || physical != 0x8000u || bytes != 1u ||
        access != CORE_MACHINE_MEMORY_ACCESS_READ) return LIB_STATUS_INTERNAL_ERROR;
    return provider->decline ? LIB_STATUS_UNSUPPORTED : LIB_STATUS_OK;
}
static lib_i32 initialize(t_vadp *adapter, t_ram *memory, core_machine *machine)
{
    t_port *port = &machine->executor_port;

    core_machine_port_initialize(port);
    if (core_machine_memory_initialize_for(memory, 16u * 1024u * 1024u,
            LIB_NULL) != LIB_STATUS_OK) return 0;
    if (core_machine_vadp_initialize(adapter, machine) != LIB_STATUS_OK) {
        core_machine_memory_finalize(memory);
        core_machine_port_finalize(port);
        return 0;
    }
    return 1;
}

static void finalize(t_vadp *adapter, t_ram *memory)
{
    t_port *port = &adapter->machine->executor_port;

    core_machine_vadp_finalize(adapter);
    core_machine_memory_finalize(memory);
    core_machine_port_finalize(port);
}

static lib_i32 is_unconfigured(const t_vadp *adapter, const t_ram *memory,
    lib_uptr observers, lib_uptr providers)
{
    return !adapter->configured &&
        !x86_video_ega_aperture_contains(adapter->chip, 0xa0000u, 1u) &&
        memory->connect.write_observer_count == observers &&
        memory->connect.device_provider_count == providers;
}

static lib_i32 register_provider_fillers(t_ram *memory, void *owner,
    lib_uptr count)
{
    lib_uptr index;

    for (index = 0u; index < count;
            ++index) {
        if (core_machine_memory_register_device_provider(memory,
                0x1000u + (lib_u32)(index * 0x100u), 1u,
                ignored_read, ignored_device_write, ignored_query, owner) !=
            LIB_STATUS_OK) return 0;
    }
    return 1;
}

static lib_i32 register_observer_fillers(t_ram *memory, void *owner)
{
    lib_uptr index;

    for (index = 0u; index < CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY;
            ++index) {
        if (core_machine_memory_register_write_observer(memory, ignored_write,
                owner) != LIB_STATUS_OK) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    const core_machine_display_config config = {
        .text_timing = { 3u, 2u, 1u },
        .ega_present = LIB_TRUE,
        .ega_personality = X86_VIDEO_EGA_PERSONALITY_GENERIC,
        .ega_sequencer = { CORE_MACHINE_VADP_EGA_APERTURE_BASE,
            CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
            0x03u, 0x00u, 0x0fu, 0x02u, LIB_TRUE }
    };
    const x86_video_cecg_config compaq = {
        0x40u, 0x00u, 0x30u, 0x01u, LIB_TRUE, LIB_FALSE, LIB_TRUE,
        0x06u, 0x01u, LIB_FALSE, LIB_FALSE, LIB_FALSE };
    t_vadp adapter;
    core_machine machine = {.lifecycle = CORE_MACHINE_INITIALIZED};
    t_ram *memory = &machine.executor_memory;
    t_port *port = &machine.executor_port;
    lib_i32 filler = 0;
    lib_i32 failed = 0;

    if (!initialize(&adapter, memory, &machine)) return 1;
    {
        core_machine_memory_test_allocation allocation = { LIB_TRUE, 0u };

        failed |= !register_provider_fillers(memory, &filler,
            CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY);
        memory->connect.device_provider_test_allocation = &allocation;
        failed |= core_machine_memory_register_device_provider(memory, 0x1c00u,
            1u, ignored_read, ignored_device_write, ignored_query, &filler) !=
            LIB_STATUS_NO_MEMORY;
        failed |= allocation.attempts != 1u ||
            memory->connect.device_provider_count !=
                CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY ||
            memory->connect.device_provider_capacity !=
                CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY;
        memory->connect.device_provider_test_allocation = LIB_NULL;
        failed |= core_machine_memory_register_device_provider(memory, 0x1c00u,
            1u, ignored_read, ignored_device_write, ignored_query, &filler) !=
            LIB_STATUS_OK;
        failed |= memory->connect.device_provider_count !=
            CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY + 1u ||
            memory->connect.device_provider_capacity <=
                CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY;
    }
    finalize(&adapter, memory);
    if (!initialize(&adapter, memory, &machine)) return 1;
    failed |= !register_provider_fillers(memory, &filler,
        CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT);
    failed |= core_machine_vadp_configure(&adapter, &config) != LIB_STATUS_NO_MEMORY;
    failed |= !is_unconfigured(&adapter, memory, 0u,
        CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT);
    finalize(&adapter, memory);

    /* A second Core route can fail after the first was appended. The
     * transaction retains only the unrelated providers and permits retry. */
    if (!initialize(&adapter, memory, &machine)) return 1;
    {
        lib_i32 candidate_owner = 0;
        const core_machine_memory_device_route routes[2] = {
            { 0x8000u, 1u, { ignored_read, ignored_device_write, ignored_query },
                CORE_MACHINE_MEMORY_PROVIDER_STANDARD },
            { 0x9000u, 1u, { ignored_read, ignored_device_write, ignored_query },
                CORE_MACHINE_MEMORY_PROVIDER_STANDARD }
        };

        failed |= !register_provider_fillers(memory, &filler,
            CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT - 1u);
        failed |= core_machine_install_memory_device_routes(&machine, routes, 2u,
            ignored_write, LIB_NULL, &candidate_owner) !=
            LIB_STATUS_NO_MEMORY;
        failed |= !is_unconfigured(&adapter, memory, 0u,
            CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT - 1u);
        core_machine_memory_unregister_owner(memory, &filler);
        failed |= core_machine_install_memory_device_routes(&machine, routes, 2u,
            ignored_write, LIB_NULL, &candidate_owner) != LIB_STATUS_OK;
        failed |= memory->connect.device_provider_count != 2u ||
            memory->connect.write_observer_count != 1u;
        failed |= core_machine_install_memory_device_routes(&machine, routes, 2u,
            ignored_write, LIB_NULL, &candidate_owner) != LIB_STATUS_INVALID_STATE ||
            memory->connect.device_provider_count != 2u ||
            memory->connect.write_observer_count != 1u;
        failed |= core_machine_remove_memory_device_routes(&machine,
            &candidate_owner) != LIB_STATUS_OK ||
            memory->connect.device_provider_count != 0u ||
            memory->connect.write_observer_count != 0u;
    }
    finalize(&adapter, memory);

    if (!initialize(&adapter, memory, &machine)) return 1;
    {
        priority_provider first = { 0x3cu, LIB_FALSE };
        priority_provider overlay = { 0xa5u, LIB_FALSE };
        lib_u8 value = 0u;

        failed |= core_machine_memory_allocate_for(memory, 0x10000u) !=
            LIB_STATUS_OK;
        failed |= core_machine_memory_register_device_provider(memory, 0x8000u,
            1u, priority_read, ignored_device_write, priority_query, &first) !=
            LIB_STATUS_OK;
        failed |= core_machine_memory_register_overlay_device_provider(memory,
            0x8000u, 1u, priority_read, ignored_device_write, priority_query,
            &overlay) != LIB_STATUS_OK;
        core_machine_memory_freeze_mappings(memory);
        failed |= core_machine_memory_read_physical(memory, 0x8000u, (lib_uptr)&value, 1u) !=
            LIB_STATUS_OK || value != first.value;
        first.decline = LIB_TRUE;
        value = 0u;
        failed |= core_machine_memory_read_physical(memory, 0x8000u, (lib_uptr)&value, 1u) !=
            LIB_STATUS_OK || value != overlay.value;
        failed |= core_machine_memory_register_device_provider(memory, 0x9000u,
            1u, ignored_read, ignored_device_write, ignored_query, &filler) !=
            LIB_STATUS_INVALID_ARGUMENT;
        failed |= memory->connect.device_provider_count != 2u;
    }
    finalize(&adapter, memory);
    if (!initialize(&adapter, memory, &machine)) return 1;
    failed |= !register_observer_fillers(memory, &filler);
    failed |= core_machine_vadp_configure(&adapter, &config) != LIB_STATUS_NO_MEMORY;
    failed |= !is_unconfigured(&adapter, memory,
        CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY, 0u);
    finalize(&adapter, memory);


    /* Initial CGA construction also preserves routes installed before it. */
    for (lib_size fail_at = 1u; fail_at <= 8u; ++fail_at) {
        core_machine_port_test_allocation allocation = { fail_at, 0u };

        core_machine_port_initialize(port);
        failed |= core_machine_port_add_read(port, 0x80u, sentinel_read,
            &filler) != LIB_STATUS_OK;
        core_machine_port_set_test_allocation(port, &allocation);
        failed |= core_machine_vadp_initialize(&adapter, &machine) != LIB_STATUS_NO_MEMORY;
        failed |= allocation.attempts != fail_at || adapter.chip != LIB_NULL ||
            core_machine_port_has_write(port, 0x3d4u) ||
            core_machine_port_read(port, 0x80u) != 0x5au;
        allocation.fail_at = 0u;
        failed |= core_machine_vadp_initialize(&adapter, &machine) != LIB_STATUS_OK;
        core_machine_vadp_finalize(&adapter);
        failed |= core_machine_port_has_write(port, 0x3d4u) ||
            core_machine_port_read(port, 0x80u) != 0x5au;
        core_machine_port_finalize(port);
    }

    /* Every EGA/Compaq/VGA port-allocation failure occurs after memory publication.
     * Rollback must retain unrelated routes and permit a complete retry. */
    for (lib_size variant = 0u; variant < 3u; ++variant) {
        core_machine_display_config selected = config;
        selected.vga_present = variant == 1u;
        if (variant == 2u) {
            selected.ega_personality =
                X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR;
            selected.cecg = compaq;
        }
        for (lib_size fail_at = 1u; fail_at <= (variant ? 27u : 20u); ++fail_at) {
            core_machine_port_test_allocation allocation = { fail_at, 0u };
            x86_video *original;

            if (!initialize(&adapter, memory, &machine)) return 1;
            original = adapter.chip;
            failed |= core_machine_port_add_read(port, 0x80u, sentinel_read,
                &filler) != LIB_STATUS_OK;
            failed |= !register_provider_fillers(memory, &filler, 1u);
            failed |= core_machine_memory_register_write_observer(memory,
                ignored_write, &filler) != LIB_STATUS_OK;
            core_machine_port_set_test_allocation(port, &allocation);
            failed |= core_machine_vadp_configure(&adapter, &selected) !=
                LIB_STATUS_NO_MEMORY;
            failed |= allocation.attempts != fail_at || adapter.chip != original ||
                !is_unconfigured(&adapter, memory, 1u, 1u) ||
                core_machine_port_has_write(port, 0x3c4u) ||
                core_machine_port_has_write(port, 0x3c9u) ||
                core_machine_port_has_read(port, 0x7c6u) ||
                !core_machine_port_has_write(port, 0x3d4u) ||
                core_machine_port_read(port, 0x80u) != 0x5au;
            allocation.fail_at = 0u;
            failed |= core_machine_vadp_configure(&adapter, &selected) !=
                LIB_STATUS_OK;
            failed |= memory->connect.device_provider_count != 2u ||
                memory->connect.write_observer_count != 2u ||
                !core_machine_port_has_write(port, 0x3c4u) ||
                core_machine_port_has_write(port, 0x3c9u) != selected.vga_present ||
                core_machine_port_has_read(port, 0x7c6u) != (variant == 2u);
            core_machine_memory_freeze_mappings(memory);
            core_machine_vadp_finalize(&adapter);
            failed |= memory->connect.device_provider_count != 1u ||
                memory->connect.write_observer_count != 1u ||
                core_machine_port_has_write(port, 0x3c4u) ||
                core_machine_port_has_write(port, 0x3c9u) ||
                core_machine_port_has_read(port, 0x7c6u) ||
                core_machine_port_has_write(port, 0x3d4u) ||
                core_machine_port_read(port, 0x80u) != 0x5au;
            core_machine_memory_finalize(memory);
            core_machine_port_finalize(port);
        }
    }

    /* A collision on the final Compaq route cannot publish its earlier
     * routes or replace the active CGA chip. */
    {
        core_machine_display_config selected = config;
        x86_video *original;

        selected.ega_personality =
            X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR;
        selected.cecg = compaq;
        if (!initialize(&adapter, memory, &machine)) return 1;
        original = adapter.chip;
        failed |= core_machine_port_add_read(port, 0xfc6u, sentinel_read,
            &filler) != LIB_STATUS_OK;
        failed |= core_machine_vadp_configure(&adapter, &selected) !=
            LIB_STATUS_INVALID_STATE;
        failed |= adapter.chip != original ||
            !is_unconfigured(&adapter, memory, 0u, 0u) ||
            core_machine_port_has_write(port, 0x3c4u) ||
            core_machine_port_read(port, 0xfc6u) != 0x5au;
        core_machine_port_unregister_owner(port, &filler);
        failed |= core_machine_vadp_configure(&adapter, &selected) !=
            LIB_STATUS_OK;
        finalize(&adapter, memory);
    }

    if (failed) return 1;
    puts("M5:T395:S1:ROUTE-REGISTRY-SCALABILITY:OK");
    return 0;
}
