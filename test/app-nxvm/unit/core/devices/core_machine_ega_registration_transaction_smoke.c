#include "lib/types/types_interface.h"
#include "type.h"
#include "app-nxvm/devices/memory.h"
#include "app-nxvm/devices/port.h"
#include "app-nxvm/devices/vadp.h"

static C_INT allocation_failure;
static lib_size allocation_attempts;

C_VOID *test_ega_registration_calloc(lib_size count, lib_size size)
{
    allocation_attempts++;
    return allocation_failure ? LIB_NULL : calloc(count, size);
}

static C_VOID ignored_write(C_VOID *owner, lib_u32 physical,
    type_native_unsigned bytes)
{
    (C_VOID)owner;
    (C_VOID)physical;
    (C_VOID)bytes;
}

static type_status ignored_read(C_VOID *owner, lib_u32 physical,
    type_virtual_address destination, type_native_unsigned bytes)
{
    (C_VOID)owner;
    (C_VOID)physical;
    (C_VOID)destination;
    (C_VOID)bytes;
    return TYPE_STATUS_UNSUPPORTED;
}

static type_status ignored_device_write(C_VOID *owner, lib_u32 physical,
    type_virtual_address source, type_native_unsigned bytes)
{
    (C_VOID)owner;
    (C_VOID)physical;
    (C_VOID)source;
    (C_VOID)bytes;
    return TYPE_STATUS_UNSUPPORTED;
}

static type_status ignored_query(C_VOID *owner, lib_u32 physical,
    type_native_unsigned bytes, core_machine_memory_access access)
{
    (C_VOID)owner;
    (C_VOID)physical;
    (C_VOID)bytes;
    (C_VOID)access;
    return TYPE_STATUS_UNSUPPORTED;
}

typedef struct priority_provider {
    C_UCHAR value;
    type_bool decline;
} priority_provider;

static type_status priority_read(C_VOID *owner, lib_u32 physical,
    type_virtual_address destination, type_native_unsigned bytes)
{
    priority_provider *provider = (priority_provider *)owner;

    if (provider == LIB_NULL || physical != 0x8000u || bytes != 1u) {
        return TYPE_STATUS_FAULT;
    }
    *(C_UCHAR *)destination = provider->value;
    return TYPE_STATUS_OK;
}

static type_status priority_query(C_VOID *owner, lib_u32 physical,
    type_native_unsigned bytes, core_machine_memory_access access)
{
    priority_provider *provider = (priority_provider *)owner;

    if (provider == LIB_NULL || physical != 0x8000u || bytes != 1u ||
        access != CORE_MACHINE_MEMORY_ACCESS_READ) return TYPE_STATUS_FAULT;
    return provider->decline ? TYPE_STATUS_UNSUPPORTED : TYPE_STATUS_OK;
}
static C_INT initialize(t_vadp *adapter, t_ram *memory, t_port *port)
{
    core_machine_port_initialize(port);
    if (core_machine_memory_initialize_for(memory, 16u * 1024u * 1024u,
            LIB_NULL) != TYPE_STATUS_OK) return 0;
    core_machine_vadp_initialize(adapter, port);
    return 1;
}

static C_VOID finalize(t_vadp *adapter, t_ram *memory)
{
    core_machine_vadp_finalize(adapter);
    core_machine_memory_finalize(memory);
}

static C_INT is_unconfigured(const t_vadp *adapter, const t_ram *memory,
    type_native_unsigned observers, type_native_unsigned providers)
{
    return !adapter->data.ega_sequencer_configured &&
        !adapter->data.ega_planar_enabled && adapter->data.ega_planar_vram == 0u &&
        memory->connect.write_observer_count == observers &&
        memory->connect.device_provider_count == providers;
}

static C_INT register_provider_fillers(t_ram *memory, C_VOID *owner,
    type_native_unsigned count)
{
    type_native_unsigned index;

    for (index = 0u; index < count;
            ++index) {
        if (core_machine_memory_register_device_provider(memory,
                0x1000u + (lib_u32)(index * 0x100u), 1u,
                ignored_read, ignored_device_write, ignored_query, owner) !=
            TYPE_STATUS_OK) return 0;
    }
    return 1;
}

static C_INT register_observer_fillers(t_ram *memory, C_VOID *owner)
{
    type_native_unsigned index;

    for (index = 0u; index < CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY;
            ++index) {
        if (core_machine_memory_register_write_observer(memory, ignored_write,
                owner) != TYPE_STATUS_OK) return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    const core_machine_vadp_ega_sequencer_config config = {
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, LIB_TRUE
    };
    t_vadp adapter;
    t_ram memory;
    t_port port;
    C_INT filler = 0;
    C_INT failed = 0;

    if (!initialize(&adapter, &memory, &port)) return 1;
    allocation_failure = 1;
    failed |= core_machine_vadp_configure_ega_sequencer(&adapter, &memory,
        &config) != TYPE_STATUS_NO_MEMORY;
    failed |= allocation_attempts != 1u || !is_unconfigured(&adapter, &memory,
        0u, 0u);
    allocation_failure = 0;
    failed |= core_machine_vadp_configure_ega_sequencer(&adapter, &memory,
        &config) != TYPE_STATUS_OK;
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
            TYPE_STATUS_NO_MEMORY;
        failed |= allocation.attempts != 1u ||
            memory.connect.device_provider_count !=
                CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY ||
            memory.connect.device_provider_capacity !=
                CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY;
        memory.connect.device_provider_test_allocation = LIB_NULL;
        failed |= core_machine_memory_register_device_provider(&memory, 0x1c00u,
            1u, ignored_read, ignored_device_write, ignored_query, &filler) !=
            TYPE_STATUS_OK;
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
        &config) != TYPE_STATUS_NO_MEMORY;
    failed |= !is_unconfigured(&adapter, &memory, 0u,
        CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT);
    finalize(&adapter, &memory);

    if (!initialize(&adapter, &memory, &port)) return 1;
    {
        priority_provider first = { 0x3cu, LIB_FALSE };
        priority_provider overlay = { 0xa5u, LIB_FALSE };
        C_UCHAR value = 0u;

        failed |= core_machine_memory_allocate_for(&memory, 0x10000u) !=
            TYPE_STATUS_OK;
        failed |= core_machine_memory_register_device_provider(&memory, 0x8000u,
            1u, priority_read, ignored_device_write, priority_query, &first) !=
            TYPE_STATUS_OK;
        failed |= core_machine_memory_register_overlay_device_provider(&memory,
            0x8000u, 1u, priority_read, ignored_device_write, priority_query,
            &overlay) != TYPE_STATUS_OK;
        core_machine_memory_freeze_mappings(&memory);
        failed |= core_machine_memory_read_physical(&memory, 0x8000u, (type_virtual_address)&value, 1u) !=
            TYPE_STATUS_OK || value != first.value;
        first.decline = LIB_TRUE;
        value = 0u;
        failed |= core_machine_memory_read_physical(&memory, 0x8000u, (type_virtual_address)&value, 1u) !=
            TYPE_STATUS_OK || value != overlay.value;
        failed |= core_machine_memory_register_device_provider(&memory, 0x9000u,
            1u, ignored_read, ignored_device_write, ignored_query, &filler) !=
            TYPE_STATUS_INVALID_ARGUMENT;
        failed |= memory.connect.device_provider_count != 2u;
    }
    finalize(&adapter, &memory);
    if (!initialize(&adapter, &memory, &port)) return 1;
    failed |= !register_observer_fillers(&memory, &filler);
    failed |= core_machine_vadp_configure_ega_sequencer(&adapter, &memory,
        &config) != TYPE_STATUS_NO_MEMORY;
    failed |= !is_unconfigured(&adapter, &memory,
        CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY, 0u);
    finalize(&adapter, &memory);

    if (failed) return 1;
    puts("M5:T395:S1:ROUTE-REGISTRY-SCALABILITY:OK");
    return 0;
}
