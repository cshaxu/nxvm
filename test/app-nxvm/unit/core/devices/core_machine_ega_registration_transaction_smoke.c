#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/memory.h"
#include "app-nxvm/devices/port.h"
#include "app-nxvm/devices/vadp.h"

static lib_i32 allocation_failure;
static lib_size allocation_attempts;

static void *test_ega_registration_allocate_zero(void *context, lib_size count,
    lib_size size)
{
    void *memory;

    (void)context;
    allocation_attempts++;
    if (allocation_failure) return LIB_NULL;
    memory = lib_allocate(count * size);
    if (memory != LIB_NULL) lib_memory_set(memory, 0, count * size);
    return memory;
}

static void ignored_write(void *owner, lib_u32 physical,
    lib_uptr bytes)
{
    (void)owner;
    (void)physical;
    (void)bytes;
}

static lib_status ignored_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes)
{
    (void)owner;
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
    lib_uptr destination, lib_uptr bytes)
{
    priority_provider *provider = (priority_provider *)owner;

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
static lib_i32 initialize(t_vadp *adapter, t_ram *memory, t_port *port)
{
    core_machine_port_initialize(port);
    if (core_machine_memory_initialize_for(memory, 16u * 1024u * 1024u,
            LIB_NULL) != LIB_STATUS_OK) return 0;
    core_machine_vadp_initialize(adapter, port);
    core_machine_vadp_set_allocate_zero(adapter,
        test_ega_registration_allocate_zero, LIB_NULL);
    return 1;
}

static void finalize(t_vadp *adapter, t_ram *memory)
{
    core_machine_vadp_finalize(adapter);
    core_machine_memory_finalize(memory);
}

static lib_i32 is_unconfigured(const t_vadp *adapter, const t_ram *memory,
    lib_uptr observers, lib_uptr providers)
{
    return !adapter->data.ega_sequencer_configured &&
        !adapter->data.ega_planar_enabled && adapter->data.ega_planar_vram == 0u &&
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
    const core_machine_vadp_ega_sequencer_config config = {
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, LIB_TRUE
    };
    t_vadp adapter;
    t_ram memory;
    t_port port;
    lib_i32 filler = 0;
    lib_i32 failed = 0;

    if (!initialize(&adapter, &memory, &port)) return 1;
    allocation_failure = 1;
    failed |= core_machine_vadp_configure_ega_sequencer(&adapter, &memory,
        &config) != LIB_STATUS_NO_MEMORY;
    failed |= allocation_attempts != 1u || !is_unconfigured(&adapter, &memory,
        0u, 0u);
    allocation_failure = 0;
    failed |= core_machine_vadp_configure_ega_sequencer(&adapter, &memory,
        &config) != LIB_STATUS_OK;
    failed |= memory.connect.write_observer_count != 1u ||
        memory.connect.device_provider_count != 1u;
    finalize(&adapter, &memory);

    if (!initialize(&adapter, &memory, &port)) return 1;
    {
        core_machine_memory_test_allocation allocation = { LIB_TRUE, 0u };

        failed |= !register_provider_fillers(&memory, &filler,
            CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY);
        memory.connect.device_provider_test_allocation = &allocation;
        failed |= core_machine_memory_register_device_provider(&memory, 0x1c00u,
            1u, ignored_read, ignored_device_write, ignored_query, &filler) !=
            LIB_STATUS_NO_MEMORY;
        failed |= allocation.attempts != 1u ||
            memory.connect.device_provider_count !=
                CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY ||
            memory.connect.device_provider_capacity !=
                CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY;
        memory.connect.device_provider_test_allocation = LIB_NULL;
        failed |= core_machine_memory_register_device_provider(&memory, 0x1c00u,
            1u, ignored_read, ignored_device_write, ignored_query, &filler) !=
            LIB_STATUS_OK;
        failed |= memory.connect.device_provider_count !=
            CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY + 1u ||
            memory.connect.device_provider_capacity <=
                CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY;
    }
    finalize(&adapter, &memory);
    if (!initialize(&adapter, &memory, &port)) return 1;
    failed |= !register_provider_fillers(&memory, &filler,
        CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT);
    failed |= core_machine_vadp_configure_ega_sequencer(&adapter, &memory,
        &config) != LIB_STATUS_NO_MEMORY;
    failed |= !is_unconfigured(&adapter, &memory, 0u,
        CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT);
    finalize(&adapter, &memory);

    if (!initialize(&adapter, &memory, &port)) return 1;
    {
        priority_provider first = { 0x3cu, LIB_FALSE };
        priority_provider overlay = { 0xa5u, LIB_FALSE };
        lib_u8 value = 0u;

        failed |= core_machine_memory_allocate_for(&memory, 0x10000u) !=
            LIB_STATUS_OK;
        failed |= core_machine_memory_register_device_provider(&memory, 0x8000u,
            1u, priority_read, ignored_device_write, priority_query, &first) !=
            LIB_STATUS_OK;
        failed |= core_machine_memory_register_overlay_device_provider(&memory,
            0x8000u, 1u, priority_read, ignored_device_write, priority_query,
            &overlay) != LIB_STATUS_OK;
        core_machine_memory_freeze_mappings(&memory);
        failed |= core_machine_memory_read_physical(&memory, 0x8000u, (lib_uptr)&value, 1u) !=
            LIB_STATUS_OK || value != first.value;
        first.decline = LIB_TRUE;
        value = 0u;
        failed |= core_machine_memory_read_physical(&memory, 0x8000u, (lib_uptr)&value, 1u) !=
            LIB_STATUS_OK || value != overlay.value;
        failed |= core_machine_memory_register_device_provider(&memory, 0x9000u,
            1u, ignored_read, ignored_device_write, ignored_query, &filler) !=
            LIB_STATUS_INVALID_ARGUMENT;
        failed |= memory.connect.device_provider_count != 2u;
    }
    finalize(&adapter, &memory);
    if (!initialize(&adapter, &memory, &port)) return 1;
    failed |= !register_observer_fillers(&memory, &filler);
    failed |= core_machine_vadp_configure_ega_sequencer(&adapter, &memory,
        &config) != LIB_STATUS_NO_MEMORY;
    failed |= !is_unconfigured(&adapter, &memory,
        CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY, 0u);
    finalize(&adapter, &memory);

    if (failed) return 1;
    puts("M5:T395:S1:ROUTE-REGISTRY-SCALABILITY:OK");
    return 0;
}
