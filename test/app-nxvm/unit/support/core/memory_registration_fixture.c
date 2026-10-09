#include "memory_registration_fixture.h"
#include "core/x86/machine.h"

static lib_status unused_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    (void)owner;
    (void)physical;
    (void)destination;
    (void)bytes;
    (void)observe_only;
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status unused_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    (void)owner;
    (void)physical;
    (void)source;
    (void)bytes;
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status unused_query(void *owner, lib_u32 physical,
    lib_uptr bytes, core_machine_memory_access access)
{
    (void)owner;
    (void)physical;
    (void)bytes;
    (void)access;
    return LIB_STATUS_UNSUPPORTED;
}

static void unused_observer(void *owner, lib_u32 physical, lib_uptr bytes)
{
    (void)owner;
    (void)physical;
    (void)bytes;
}

static void unused_fault(void *owner, lib_u32 physical)
{
    (void)owner;
    (void)physical;
}

lib_i32 test_core_memory_registration_case(lib_u32 mode, void *owner,
    test_core_memory_registration_configure configure,
    test_core_memory_registration_exercise exercise, void *context)
{
    core_machine machine = { .lifecycle = CORE_MACHINE_INITIALIZED };
    t_ram *memory = &machine.executor_memory;
    lib_size providers_before;
    lib_size observers_before;
    lib_uptr parity_before;
    lib_i32 filler = 0;
    lib_i32 failed = 0;
    lib_status expected = mode == 2u ? LIB_STATUS_INVALID_ARGUMENT :
        LIB_STATUS_NO_MEMORY;

    if (core_machine_memory_initialize_for(memory, 2u * 1024u * 1024u,
            LIB_NULL) != LIB_STATUS_OK) return 1;
    if (mode <= 1u) {
        const lib_size count = CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT -
            (mode == 0u ? 0u : 1u);

        for (lib_size index = 0u; index < count; ++index)
            failed |= core_machine_memory_register_device_provider(memory,
                0x1000u + (lib_u32)(index * 0x100u), 1u,
                unused_read, unused_write, unused_query, &filler) != LIB_STATUS_OK;
    } else if (mode == 2u) {
        failed |= core_machine_memory_enable_parity(memory, 1024u,
            unused_fault, &filler) != LIB_STATUS_OK;
    } else {
        for (lib_size index = 0u; index <
                CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY; ++index)
            failed |= core_machine_memory_register_write_observer(memory,
                unused_observer, &filler) != LIB_STATUS_OK;
    }
    providers_before = memory->connect.device_provider_count;
    observers_before = memory->connect.write_observer_count;
    parity_before = memory->connect.parity;
    failed |= configure(&machine, context) != expected ||
        memory->connect.device_provider_count != providers_before ||
        memory->connect.write_observer_count != observers_before ||
        memory->connect.parity != parity_before;

    core_machine_memory_unregister_owner(memory, &filler);
    if (mode == 2u) core_machine_memory_release_parity(memory);
    failed |= configure(&machine, context) != LIB_STATUS_OK ||
        memory->connect.device_provider_count != 2u ||
        memory->connect.write_observer_count != 1u ||
        memory->connect.parity_owner != owner ||
        memory->connect.device_providers[0].owner != owner ||
        memory->connect.device_providers[1].owner != owner ||
        !memory->connect.device_providers[0].replacement ||
        !memory->connect.device_providers[1].replacement ||
        memory->connect.write_observers[0].owner != owner;
    failed |= exercise(&machine, context);
    failed |= core_machine_remove_memory_device_routes(&machine, owner) !=
        LIB_STATUS_OK || memory->connect.device_provider_count != 0u ||
        memory->connect.write_observer_count != 0u ||
        memory->connect.parity != 0u;
    core_machine_memory_finalize(memory);
    return failed;
}
