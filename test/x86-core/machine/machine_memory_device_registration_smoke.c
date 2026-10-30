#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/machine.h"

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
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    const lib_u8 *value = (const lib_u8 *)opaque;

    (void)physical;
    (void)access;
    return value != LIB_NULL && bytes == 1u && *value != 0u ? LIB_STATUS_OK :
        LIB_STATUS_UNSUPPORTED;
}

static void registration_observer(void *owner, lib_u32 physical, lib_uptr bytes)
{
    (void)owner;
    (void)physical;
    (void)bytes;
}

static void registration_fault(void *owner, lib_u32 physical)
{
    (void)owner;
    (void)physical;
}

typedef struct priority_provider {
    lib_u8 value;
    lib_bool decline;
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
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    priority_provider *provider = (priority_provider *)owner;

    if (provider == LIB_NULL || physical != 0x8000u || bytes != 1u ||
        access != X86_CORE_MACHINE_MEMORY_ACCESS_READ) return LIB_STATUS_INTERNAL_ERROR;
    return provider->decline ? LIB_STATUS_UNSUPPORTED : LIB_STATUS_OK;
}

static lib_i32 registration_growth_and_priority(void)
{
    t_ram memory = {0};
    x86_core_machine_memory_test_allocation allocation = {LIB_TRUE, 0u};
    priority_provider first = {0x3cu, LIB_FALSE};
    priority_provider overlay = {0xa5u, LIB_FALSE};
    lib_u8 filler = 0u;
    lib_u8 value = 0u;
    lib_i32 failed = 0;

    if (x86_core_machine_memory_initialize_for(&memory, 16u * 1024u * 1024u,
            LIB_NULL) != LIB_STATUS_OK) return 1;
    for (lib_uptr index = 0u; index <
            X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY; ++index) {
        failed |= x86_core_machine_memory_register_device_provider(&memory,
            0x1000u + (lib_u32)(index * 0x100u), 1u, overlay_read,
            overlay_write, overlay_query, &filler, LIB_NULL) != LIB_STATUS_OK;
    }
    memory.connect.device_provider_test_allocation = &allocation;
    failed |= x86_core_machine_memory_register_device_provider(&memory, 0x1c00u,
        1u, overlay_read, overlay_write, overlay_query, &filler,
        LIB_NULL) != LIB_STATUS_NO_MEMORY;
    failed |= allocation.attempts != 1u ||
        memory.connect.device_provider_count !=
            X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY ||
        memory.connect.device_provider_capacity !=
            X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY;
    memory.connect.device_provider_test_allocation = LIB_NULL;
    failed |= x86_core_machine_memory_register_device_provider(&memory, 0x1c00u,
        1u, overlay_read, overlay_write, overlay_query, &filler,
        LIB_NULL) != LIB_STATUS_OK;
    failed |= memory.connect.device_provider_count !=
        X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY + 1u ||
        memory.connect.device_provider_capacity <=
            X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY;
    x86_core_machine_memory_finalize(&memory);

    if (x86_core_machine_memory_initialize_for(&memory, 16u * 1024u * 1024u,
            LIB_NULL) != LIB_STATUS_OK) return 1;
    failed |= x86_core_machine_memory_allocate_for(&memory, 0x10000u) != LIB_STATUS_OK;
    failed |= x86_core_machine_memory_register_device_provider(&memory, 0x8000u,
        1u, priority_read, overlay_write, priority_query, &first,
        LIB_NULL) != LIB_STATUS_OK;
    failed |= x86_core_machine_memory_register_overlay_device_provider(&memory,
        0x8000u, 1u, priority_read, overlay_write, priority_query, &overlay,
        LIB_NULL) != LIB_STATUS_OK;
    x86_core_machine_memory_freeze_mappings(&memory);
    failed |= x86_core_machine_memory_read_physical(&memory, 0x8000u,
        (lib_uptr)&value, 1u, LIB_TRUE) != LIB_STATUS_OK || value != first.value;
    first.decline = LIB_TRUE;
    value = 0u;
    failed |= x86_core_machine_memory_read_physical(&memory, 0x8000u,
        (lib_uptr)&value, 1u, LIB_TRUE) != LIB_STATUS_OK || value != overlay.value;
    failed |= x86_core_machine_memory_register_device_provider(&memory, 0x9000u,
        1u, overlay_read, overlay_write, overlay_query, &filler,
        LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= memory.connect.device_provider_count != 2u;
    x86_core_machine_memory_finalize(&memory);
    return failed;
}

static lib_i32 registration_standard_transaction(void)
{
    x86_core_machine machine = {.lifecycle = X86_CORE_MACHINE_INITIALIZED};
    t_ram *memory = &machine.executor_memory;
    lib_u8 filler = 0u;
    lib_u8 candidate_owner = 1u;
    const x86_core_machine_memory_device_route routes[2] = {
        {0x8000u, 1u, {overlay_read, overlay_write, overlay_query},
            X86_CORE_MACHINE_MEMORY_PROVIDER_STANDARD, LIB_NULL},
        {0x9000u, 1u, {overlay_read, overlay_write, overlay_query},
            X86_CORE_MACHINE_MEMORY_PROVIDER_STANDARD, LIB_NULL}
    };
    lib_i32 failed = 0;

    if (x86_core_machine_memory_initialize_for(memory, 16u * 1024u * 1024u,
            LIB_NULL) != LIB_STATUS_OK) return 1;
    for (lib_uptr index = 0u; index <
            X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT - 1u; ++index) {
        failed |= x86_core_machine_memory_register_device_provider(memory,
            0x1000u + (lib_u32)(index * 0x100u), 1u, overlay_read,
            overlay_write, overlay_query, &filler, LIB_NULL) != LIB_STATUS_OK;
    }
    failed |= x86_core_machine_install_memory_device_routes(&machine, routes, 2u,
        registration_observer, LIB_NULL, &candidate_owner) != LIB_STATUS_NO_MEMORY;
    failed |= memory->connect.write_observer_count != 0u ||
        memory->connect.device_provider_count !=
            X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT - 1u;
    x86_core_machine_memory_unregister_owner(memory, &filler);
    failed |= x86_core_machine_install_memory_device_routes(&machine, routes, 2u,
        registration_observer, LIB_NULL, &candidate_owner) != LIB_STATUS_OK;
    failed |= memory->connect.device_provider_count != 2u ||
        memory->connect.write_observer_count != 1u;
    failed |= x86_core_machine_install_memory_device_routes(&machine, routes, 2u,
        registration_observer, LIB_NULL, &candidate_owner) != LIB_STATUS_INVALID_STATE ||
        memory->connect.device_provider_count != 2u ||
        memory->connect.write_observer_count != 1u;
    failed |= x86_core_machine_remove_memory_device_routes(&machine,
        &candidate_owner) != LIB_STATUS_OK ||
        memory->connect.device_provider_count != 0u ||
        memory->connect.write_observer_count != 0u;
    x86_core_machine_memory_finalize(memory);
    return failed;
}

typedef struct instruction_read_provider {
    lib_u8 data;
    lib_u8 instruction;
    lib_bool decline_instruction;
} instruction_read_provider;

static lib_status instruction_read_data(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    instruction_read_provider *provider = owner;

    (void)observe_only;
    if (provider == LIB_NULL || physical != 0x000c0000u || destination == 0u ||
        bytes != 1u) return LIB_STATUS_UNSUPPORTED;
    *(lib_u8 *)destination = provider->data;
    return LIB_STATUS_OK;
}

static lib_status instruction_read_executable(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    instruction_read_provider *provider = owner;

    (void)observe_only;
    if (provider == LIB_NULL || physical != 0x000c0000u || destination == 0u ||
        bytes != 1u || provider->decline_instruction) return LIB_STATUS_UNSUPPORTED;
    *(lib_u8 *)destination = provider->instruction;
    return LIB_STATUS_OK;
}

static lib_status instruction_read_query(void *owner, lib_u32 physical,
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    (void)access;
    return owner != LIB_NULL && physical == 0x000c0000u && bytes == 1u ?
        LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static lib_i32 instruction_read_contract(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    instruction_read_provider provider = {0x11u, 0x22u, LIB_FALSE};
    const x86_core_machine_memory_device_route route = {0x000c0000u, 1u,
        {instruction_read_data, overlay_write, instruction_read_query},
        X86_CORE_MACHINE_MEMORY_PROVIDER_STANDARD, LIB_NULL};
    x86_core_machine *machine = LIB_NULL;
    lib_u8 value = 0u;
    lib_i32 failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    if (!failed) failed |= x86_core_machine_set_memory_instruction_read(machine,
        &provider, instruction_read_executable) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_core_machine_install_memory_device_routes(machine, &route, 1u,
            LIB_NULL, LIB_NULL, &provider) != LIB_STATUS_OK ||
        x86_core_machine_set_memory_instruction_read(machine, &provider,
            instruction_read_executable) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK;
    if (!failed) failed |= x86_core_machine_cpu_bus.read_memory(machine, 0x000c0000u,
        &value, 1u, X86_CORE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 0u) !=
            LIB_STATUS_OK || value != provider.data;
    value = 0u;
    if (!failed) failed |= x86_core_machine_cpu_bus.read_memory(machine, 0x000c0000u,
        &value, 1u, X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH, LIB_FALSE,
        LIB_FALSE, 0u) != LIB_STATUS_OK || value != provider.instruction;
    provider.decline_instruction = LIB_TRUE;
    value = 0u;
    if (!failed) failed |= x86_core_machine_cpu_bus.read_memory(machine, 0x000c0000u,
        &value, 1u, X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH, LIB_FALSE,
        LIB_FALSE, 0u) != LIB_STATUS_OK || value != provider.data;
    x86_core_machine_destroy(machine);
    return failed;
}

static lib_i32 registration_mixed_tiers(void)
{
    t_ram memory = {0};
    priority_provider raw = {0x11u, LIB_FALSE};
    priority_provider replacement = {0x22u, LIB_FALSE};
    priority_provider ordinary = {0x33u, LIB_FALSE};
    priority_provider fallback = {0x44u, LIB_FALSE};
    lib_u8 value = 0u;
    lib_i32 failed = 0;

    if (x86_core_machine_memory_initialize_for(&memory, 0x10000u, LIB_NULL) !=
            LIB_STATUS_OK) return 1;
    failed |= x86_core_machine_memory_allocate_for(&memory, 0x10000u) != LIB_STATUS_OK;
    failed |= x86_core_machine_memory_register_fallback_device_provider(&memory, 0x8000u,
        1u, priority_read, overlay_write, priority_query, &fallback, LIB_NULL) != LIB_STATUS_OK;
    failed |= x86_core_machine_memory_register_overlay_device_provider(&memory, 0x8000u,
        1u, priority_read, overlay_write, priority_query, &ordinary, LIB_NULL) != LIB_STATUS_OK;
    failed |= x86_core_machine_memory_register_replacement_device_provider(&memory, 0x8000u,
        1u, priority_read, overlay_write, priority_query, &replacement, LIB_NULL) != LIB_STATUS_OK;
    failed |= x86_core_machine_memory_register_raw_overlay_device_provider(&memory, 0x8000u,
        1u, priority_read, overlay_write, priority_query, &raw, LIB_NULL) != LIB_STATUS_OK;
    x86_core_machine_memory_freeze_mappings(&memory);
    failed |= x86_core_machine_memory_read_physical(&memory, 0x8000u,
        (lib_uptr)&value, 1u, LIB_TRUE) != LIB_STATUS_OK || value != raw.value;
    raw.decline = LIB_TRUE;
    failed |= x86_core_machine_memory_read_physical(&memory, 0x8000u,
        (lib_uptr)&value, 1u, LIB_TRUE) != LIB_STATUS_OK || value != replacement.value;
    replacement.decline = LIB_TRUE;
    failed |= x86_core_machine_memory_read_physical(&memory, 0x8000u,
        (lib_uptr)&value, 1u, LIB_TRUE) != LIB_STATUS_OK || value != ordinary.value;
    ordinary.decline = LIB_TRUE;
    failed |= x86_core_machine_memory_read_physical(&memory, 0x8000u,
        (lib_uptr)&value, 1u, LIB_TRUE) != LIB_STATUS_OK || value != fallback.value;
    x86_core_machine_memory_finalize(&memory);
    return failed;
}

static lib_i32 registration_matching_capacity(void)
{
    t_ram memory = {0};
    priority_provider providers[X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT] = {0};
    lib_u8 value = 0u;
    lib_i32 failed = 0;

    if (x86_core_machine_memory_initialize_for(&memory, 0x10000u, LIB_NULL) !=
            LIB_STATUS_OK) return 1;
    failed |= x86_core_machine_memory_allocate_for(&memory, 0x10000u) != LIB_STATUS_OK;
    for (lib_uptr index = 0u; index < X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT; ++index) {
        providers[index].value = (lib_u8)index;
        providers[index].decline = index + 1u != X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT;
        failed |= x86_core_machine_memory_register_overlay_device_provider(&memory, 0x8000u,
            1u, priority_read, overlay_write, priority_query, &providers[index], LIB_NULL) !=
                LIB_STATUS_OK;
    }
    x86_core_machine_memory_freeze_mappings(&memory);
    failed |= x86_core_machine_memory_read_physical(&memory, 0x8000u,
        (lib_uptr)&value, 1u, LIB_TRUE) != LIB_STATUS_OK ||
        value != X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT - 1u;
    providers[0].decline = LIB_FALSE;
    failed |= x86_core_machine_memory_read_physical(&memory, 0x8000u,
        (lib_uptr)&value, 1u, LIB_TRUE) != LIB_STATUS_OK || value != 0u;
    x86_core_machine_memory_finalize(&memory);
    return failed;
}

/* Registry representation is checked here, at Machine's owner. Outer device
 * tests exercise their own configuration and callbacks through public APIs. */
static lib_i32 registration_atomicity(lib_u32 mode)
{
    x86_core_machine machine = {.lifecycle = X86_CORE_MACHINE_INITIALIZED};
    t_ram *memory = &machine.executor_memory;
    x86_core_machine_memory_device_route routes[2] = {
        {0x80000000u, 1u, {overlay_read, overlay_write, overlay_query},
            X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT, LIB_NULL},
        {0x80000100u, 1u, {overlay_read, overlay_write, overlay_query},
            X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT, LIB_NULL}
    };
    const x86_core_machine_memory_parity_config parity = {
        2u * 1024u * 1024u, registration_fault
    };
    lib_u8 filler = 0u;
    lib_u8 owner = 1u;
    lib_size providers_before;
    lib_size observers_before;
    lib_uptr parity_before;
    lib_i32 failed = 0;
    const lib_status expected = mode == 2u || mode == 4u ?
        LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_NO_MEMORY;

    if (x86_core_machine_memory_initialize_for(memory, parity.bytes, LIB_NULL) !=
            LIB_STATUS_OK) return 1;
    if (mode <= 1u) {
        const lib_size count = X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT -
            (mode == 0u ? 0u : 1u);

        for (lib_size index = 0u; index < count; ++index)
            failed |= x86_core_machine_memory_register_device_provider(memory,
                0x1000u + (lib_u32)index * 0x100u, 1u, overlay_read,
                overlay_write, overlay_query, &filler, LIB_NULL) != LIB_STATUS_OK;
    } else if (mode == 2u) {
        failed |= x86_core_machine_memory_enable_parity(memory, 1024u,
            registration_fault, &filler) != LIB_STATUS_OK;
    } else if (mode == 3u) {
        for (lib_size index = 0u; index <
                X86_CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY; ++index)
            failed |= x86_core_machine_memory_register_write_observer(memory,
                registration_observer, &filler) != LIB_STATUS_OK;
    } else {
        routes[1].mode = (x86_core_machine_memory_provider_mode)LIB_UINT32_MAX;
    }
    providers_before = memory->connect.device_provider_count;
    observers_before = memory->connect.write_observer_count;
    parity_before = memory->connect.parity;
    failed |= x86_core_machine_install_memory_device_routes(&machine, routes, 2u,
        registration_observer, &parity, &owner) != expected ||
        memory->connect.device_provider_count != providers_before ||
        memory->connect.write_observer_count != observers_before ||
        memory->connect.parity != parity_before;

    x86_core_machine_memory_unregister_owner(memory, &filler);
    if (mode == 2u) x86_core_machine_memory_release_parity(memory);
    routes[1].mode = X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT;
    failed |= x86_core_machine_install_memory_device_routes(&machine, routes, 2u,
        registration_observer, &parity, &owner) != LIB_STATUS_OK ||
        memory->connect.device_provider_count != 2u ||
        memory->connect.write_observer_count != 1u ||
        memory->connect.parity_owner != &owner ||
        memory->connect.device_providers[0].owner != &owner ||
        memory->connect.device_providers[1].owner != &owner ||
        !memory->connect.device_providers[0].replacement ||
        !memory->connect.device_providers[1].replacement ||
        memory->connect.write_observers[0].owner != &owner;
    failed |= x86_core_machine_remove_memory_device_routes(&machine, &owner) !=
        LIB_STATUS_OK || memory->connect.device_provider_count != 0u ||
        memory->connect.write_observer_count != 0u || memory->connect.parity != 0u;
    x86_core_machine_memory_finalize(memory);
    return failed;
}

static lib_i32 registration_allocation_failure(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    const x86_core_machine_memory_device_route route = {
        0xa0000u, 1u, {overlay_read, overlay_write, overlay_query},
        X86_CORE_MACHINE_MEMORY_PROVIDER_FALLBACK, LIB_NULL
    };
    x86_core_machine_memory_test_allocation allocation = {LIB_TRUE, 0u};
    x86_core_machine *machine = LIB_NULL;
    lib_u8 owner = 0x3cu;
    lib_i32 failed;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK)
        return 1;
    machine->executor_memory.connect.device_provider_test_allocation = &allocation;
    failed = x86_core_machine_install_memory_device_routes(machine, &route, 1u,
        LIB_NULL, LIB_NULL, &owner) != LIB_STATUS_NO_MEMORY ||
        allocation.attempts != 1u ||
        machine->executor_memory.connect.device_provider_count != 0u ||
        machine->executor_memory.connect.device_provider_capacity != 0u;
    machine->executor_memory.connect.device_provider_test_allocation = LIB_NULL;
    failed |= x86_core_machine_install_memory_device_routes(machine, &route, 1u,
        LIB_NULL, LIB_NULL, &owner) != LIB_STATUS_OK ||
        machine->executor_memory.connect.device_provider_count != 1u ||
        machine->executor_memory.connect.device_providers[0].owner != &owner;
    failed |= x86_core_machine_remove_memory_device_routes(machine, &owner) !=
        LIB_STATUS_OK || machine->executor_memory.connect.device_provider_count != 0u;
    x86_core_machine_destroy(machine);
    return failed;
}

typedef struct registration_failure_provider {
    lib_status query_status;
    lib_status read_status;
    lib_status write_status;
    lib_u32 failure_count;
    x86_core_machine_memory_access failure_access;
    lib_status failure_status;
} registration_failure_provider;

static lib_status registration_failure_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    registration_failure_provider *provider = owner;

    (void)physical;
    (void)destination;
    (void)bytes;
    (void)observe_only;
    return provider == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : provider->read_status;
}

static lib_status registration_failure_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    registration_failure_provider *provider = owner;

    (void)physical;
    (void)source;
    (void)bytes;
    return provider == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : provider->write_status;
}

static lib_status registration_failure_query(void *owner, lib_u32 physical,
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    registration_failure_provider *provider = owner;

    (void)physical;
    (void)bytes;
    (void)access;
    return provider == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : provider->query_status;
}

static void registration_failure_note(void *owner, lib_u32 physical, lib_uptr bytes,
    x86_core_machine_memory_access access, lib_status status)
{
    registration_failure_provider *provider = owner;

    (void)physical;
    (void)bytes;
    ++provider->failure_count;
    provider->failure_access = access;
    provider->failure_status = status;
}

static lib_i32 registration_failure_observer_contract(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    registration_failure_provider provider = {
        .query_status = LIB_STATUS_IO_ERROR,
        .read_status = LIB_STATUS_IO_ERROR,
        .write_status = LIB_STATUS_IO_ERROR
    };
    const x86_core_machine_memory_device_route route = {
        0x8000u, 1u,
        {registration_failure_read, registration_failure_write,
            registration_failure_query},
        X86_CORE_MACHINE_MEMORY_PROVIDER_STANDARD, registration_failure_note
    };
    x86_core_machine *machine = LIB_NULL;
    lib_u8 value = 0u;
    lib_i32 failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    if (!failed) failed |= x86_core_machine_install_memory_device_routes(machine, &route,
        1u, LIB_NULL, LIB_NULL, &provider) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x8000u, &value, 1u) !=
            LIB_STATUS_IO_ERROR || provider.failure_count != 1u ||
        provider.failure_access != X86_CORE_MACHINE_MEMORY_ACCESS_READ ||
        provider.failure_status != LIB_STATUS_IO_ERROR;
    provider.failure_count = 0u;
    provider.query_status = LIB_STATUS_UNSUPPORTED;
    if (!failed) failed |= x86_core_machine_memory_read(machine, 0x8000u, &value, 1u) !=
        LIB_STATUS_OK || provider.failure_count != 0u;
    provider.query_status = LIB_STATUS_OK;
    if (!failed) failed |= x86_core_machine_memory_read(machine, 0x8000u, &value, 1u) !=
        LIB_STATUS_IO_ERROR || provider.failure_count != 1u ||
        provider.failure_access != X86_CORE_MACHINE_MEMORY_ACCESS_READ;
    provider.failure_count = 0u;
    if (!failed) failed |= x86_core_machine_memory_write(machine, 0x8000u, &value, 1u) !=
        LIB_STATUS_IO_ERROR || provider.failure_count != 1u ||
        provider.failure_access != X86_CORE_MACHINE_MEMORY_ACCESS_WRITE;
    x86_core_machine_destroy(machine);
    return failed;
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

static lib_i32 dma_memory_contract(x86_core_machine *machine, lib_u8 *overlay)
{
    dma_probe probe = {0};
    lib_u16 value = 0u;
    for (lib_u8 bytes = 1u; bytes <= 2u; ++bytes) {
        if (x86_core_machine_dma_memory_cycle(machine, 0x200u, bytes, 2u,
                X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, &value, dma_before, dma_after, &probe) != LIB_STATUS_OK ||
            x86_core_machine_dma_memory_cycle(machine, 0x200u, bytes, 2u,
                X86_CORE_MACHINE_MEMORY_ACCESS_READ, &value, dma_before, dma_after, &probe) != LIB_STATUS_OK ||
            (value & (bytes == 1u ? 0xffu : 0xffffu)) != (bytes == 1u ? 0x34u : 0x1234u) ||
            probe.before != bytes * 2u || probe.after != bytes * 2u) return 1;
    }
    if (x86_core_machine_dma_memory_cycle(machine, 0x200u, 3u, 2u,
            X86_CORE_MACHINE_MEMORY_ACCESS_READ, &value, dma_before, dma_after, &probe) != LIB_STATUS_INVALID_ARGUMENT ||
        probe.before != 4u || probe.after != 4u) return 1;
    *overlay = 0xffu;
    if (x86_core_machine_dma_memory_cycle(machine, 0x000f0000u, 1u, 2u,
            X86_CORE_MACHINE_MEMORY_ACCESS_READ, &value, dma_before, dma_after, &probe) != LIB_STATUS_IO_ERROR ||
        probe.before != 5u || probe.after != 4u) return 1;
    *overlay = 0x3cu;
    /* A failed memory stage must release the transaction for the next cycle. */
    if (x86_core_machine_dma_memory_cycle(machine, 0x000f0000u, 1u, 2u,
            X86_CORE_MACHINE_MEMORY_ACCESS_READ, &value, dma_before, dma_after, &probe) != LIB_STATUS_OK ||
        (probe.observed & 0xffu) != 0x3cu || probe.before != 6u || probe.after != 5u) return 1;
    return 0;
}

typedef struct parity_delivery {
    lib_u32 faults;
    lib_u32 physical;
    lib_u32 writes;
} parity_delivery;

static void parity_delivery_fault(void *owner, lib_u32 physical)
{
    parity_delivery *delivery = owner;
    ++delivery->faults;
    delivery->physical = physical;
}

static void parity_delivery_write(void *owner, lib_u32 physical, lib_uptr bytes)
{
    parity_delivery *delivery = owner;
    if (physical == 0x12346u && bytes == 1u) ++delivery->writes;
}

static lib_i32 parity_delivery_contract(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    const x86_core_machine_memory_parity_config parity = {
        1024u * 1024u, parity_delivery_fault
    };
    parity_delivery delivery = {0};
    x86_core_machine *machine = LIB_NULL;
    lib_u8 value = 0u;
    lib_i32 failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    if (!failed) failed |= x86_core_machine_install_memory_device_routes(machine,
        LIB_NULL, 0u, parity_delivery_write, &parity, &delivery) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x12345u, &(lib_u8){0x5au}, 1u) !=
            LIB_STATUS_OK || machine->executor_memory.connect.parity == 0u;
    if (!failed) {
        ((lib_u8 *)machine->executor_memory.connect.parity)[0x12345u] ^= 1u;
        failed |= x86_core_machine_memory_read(machine, 0x12345u, &value, 1u) !=
            LIB_STATUS_OK || value != 0x5au || delivery.faults != 1u ||
            delivery.physical != 0x12345u || delivery.writes != 0u ||
            x86_core_machine_memory_write(machine, 0x12346u,
                &(lib_u8){0xa5u}, 1u) != LIB_STATUS_OK || delivery.writes != 1u;
    }
    x86_core_machine_destroy(machine);
    return failed;
}

static lib_i32 cpu_overlay_contract(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_80386};
    const x86_core_machine_memory_alias_config alias = {0x200u, 0x300u, 16u};
    const x86_core_machine_memory_device_route route = {
        0x200u, 1u, {overlay_read, overlay_write, overlay_query},
        X86_CORE_MACHINE_MEMORY_PROVIDER_CPU_OVERLAY, LIB_NULL};
    x86_core_machine *machine = LIB_NULL;
    lib_u8 overlay = 0u;
    lib_u8 read = 0u;
    lib_u16 dma = 0u;
    lib_i32 failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    if (!failed) failed |= x86_core_machine_install_memory_aliases(machine,
            &alias, 1u, LIB_TRUE) != LIB_STATUS_OK ||
        x86_core_machine_install_memory_device_routes(machine, &route, 1u,
            LIB_NULL, LIB_NULL, &overlay) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x200u, &(lib_u8){0x11u}, 1u) != LIB_STATUS_OK;
    overlay = 0x33u;
    if (!failed) failed |= x86_core_machine_memory_read(machine, 0x200u, &read, 1u) !=
            LIB_STATUS_OK || read != 0x33u ||
        x86_core_machine_memory_read_instruction_physical(&machine->executor_memory,
            0x200u, (lib_uptr)&read, 1u) != LIB_STATUS_OK || read != 0x33u ||
        x86_core_machine_dma_memory_cycle(machine, 0x200u, 1u, 2u,
            X86_CORE_MACHINE_MEMORY_ACCESS_READ, &dma, LIB_NULL, LIB_NULL, LIB_NULL) !=
            LIB_STATUS_OK || dma != 0x11u;
    dma = 0x66u;
    if (!failed) failed |= x86_core_machine_dma_memory_cycle(machine, 0x200u, 1u, 2u,
        X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, &dma, LIB_NULL, LIB_NULL, LIB_NULL) !=
            LIB_STATUS_OK || overlay != 0x33u;
    overlay = 0xffu;
    if (!failed) failed |= x86_core_machine_memory_read(machine, 0x200u, &read, 1u) !=
            LIB_STATUS_IO_ERROR ||
        x86_core_machine_dma_memory_cycle(machine, 0x200u, 1u, 2u,
            X86_CORE_MACHINE_MEMORY_ACCESS_READ, &dma, LIB_NULL, LIB_NULL, LIB_NULL) !=
            LIB_STATUS_OK || dma != 0x66u;
    overlay = 0u;
    if (!failed) failed |= x86_core_machine_memory_read(machine, 0x200u, &read, 1u) !=
        LIB_STATUS_OK || read != 0x66u;
    x86_core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    x86_core_machine_memory_device_route provider = {
        0x000f0000u, 1u,
        { overlay_read, overlay_write, overlay_query },
        X86_CORE_MACHINE_MEMORY_PROVIDER_OVERLAY, LIB_NULL
    };
    const lib_u8 rom = 0x5au;
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_memory_route route;
    lib_u8 overlay = 0u;
    lib_u8 observed = 0u;
    lib_u8 write = 0xa5u;
    lib_i32 failed = parity_delivery_contract() || registration_allocation_failure() ||
        registration_growth_and_priority() || registration_mixed_tiers() ||
        registration_matching_capacity() ||
        registration_standard_transaction() ||
        registration_failure_observer_contract() || instruction_read_contract() ||
        cpu_overlay_contract();

    for (lib_u32 mode = 0u; mode < 5u; ++mode)
        failed |= registration_atomicity(mode);
    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK || machine == LIB_NULL;
    if (!failed) failed |= x86_core_machine_install_memory_device_routes(machine,
        &provider, 1u, LIB_NULL, LIB_NULL, &overlay) != LIB_STATUS_OK ||
        x86_core_machine_register_immutable_rom_mapping(machine, 0x000f0000u, &rom,
            sizeof(rom)) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x000f0000u, &observed,
            sizeof(observed)) != LIB_STATUS_OK || observed != rom;
    overlay = 0x3cu;
    if (!failed) failed |= x86_core_machine_memory_query(machine, 0x000f0000u, 1u,
        X86_CORE_MACHINE_MEMORY_ACCESS_READ, &route) != LIB_STATUS_OK ||
        route != X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER ||
        x86_core_machine_memory_read(machine, 0x000f0000u, &observed,
            sizeof(observed)) != LIB_STATUS_OK || observed != overlay ||
        x86_core_machine_memory_write(machine, 0x000f0000u, &write,
            sizeof(write)) != LIB_STATUS_OK || overlay != write;
    /* Wide RAM remains direct; a one-byte provider inside the same width
     * must still receive its own byte instead of disappearing into RAM. */
    for (lib_u32 index = 0u; !failed && index < 2u; ++index) {
        const lib_u32 address = index == 0u ? 0x200u : 0xeffffu;
        const lib_u8 pair[] = {0x11u, 0x22u};
        lib_u8 actual[2] = {0};

        failed |= x86_core_machine_memory_write(machine, address, pair,
                sizeof(pair)) != LIB_STATUS_OK ||
            x86_core_machine_memory_read(machine, address, actual,
                sizeof(actual)) != LIB_STATUS_OK ||
            lib_memory_compare(pair, actual, sizeof(pair)) != 0;
        if (index != 0u) failed |= overlay != pair[1];
    }
    for (lib_u32 index = 0u; !failed && index < 4u; ++index) {
        const lib_u32 addresses[] = {0x200u, 0xefff0u, 0xf0000u, 0x200000u};
        lib_u32 decoded = 0u;
        lib_bool fill = LIB_FALSE;

        failed |= x86_core_machine_memory_query_cpu_cache(&machine->executor_memory,
            addresses[index], LIB_FALSE, LIB_TRUE, &decoded, &fill) != LIB_STATUS_OK ||
            decoded != addresses[index] || fill != (index < 2u);
    }
    overlay = 0u;
    provider.physical_start = 0x000f1000u;
    if (!failed) failed |= x86_core_machine_memory_read(machine, 0x000f0000u, &observed,
        sizeof(observed)) != LIB_STATUS_OK || observed != rom ||
        x86_core_machine_install_memory_device_routes(machine, &provider, 1u,
            LIB_NULL, LIB_NULL, &overlay) != LIB_STATUS_INVALID_STATE;
    if (!failed) failed |= dma_memory_contract(machine, &overlay);
    overlay = 0x3cu;
    if (!failed) failed |= x86_core_machine_remove_memory_device_routes(LIB_NULL, &overlay) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_core_machine_remove_memory_device_routes(machine, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_core_machine_remove_memory_device_routes(machine, &overlay) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, 0x000f0000u, &observed, 1u) != LIB_STATUS_OK || observed != rom;
    lib_u32 clear_mask = 0u;
    if (!failed) failed |= x86_core_machine_signal_address_clear_mask(LIB_NULL, 0u) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_core_machine_observe_address_clear_mask(machine, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_core_machine_signal_address_clear_mask(machine, 0x00100000u) != LIB_STATUS_OK ||
        x86_core_machine_observe_address_clear_mask(machine, &clear_mask) != LIB_STATUS_OK || clear_mask != 0x00100000u ||
        x86_core_machine_signal_address_clear_mask(machine, 0u) != LIB_STATUS_OK ||
        x86_core_machine_observe_address_clear_mask(machine, &clear_mask) != LIB_STATUS_OK || clear_mask != 0u;
    if (!failed) lib_c_printf("CORE-MEMORY-DEVICE:OK\n");
    x86_core_machine_destroy(machine);
    return failed ? 1 : 0;
}
