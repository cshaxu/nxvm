#include "lib/types/types_interface.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_board_state.h"

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

static lib_i32 run_case(lib_u32 mode)
{
    const core_machine_d4_memory_config config = { LIB_TRUE, 0xf7u, 0x80u, 0x0002u };
    core_machine machine = { .lifecycle = CORE_MACHINE_INITIALIZED };
    core_machine_board_state board = {0};
    t_ram *memory = &machine.executor_memory;
    lib_size providers_before;
    lib_size observers_before;
    lib_uptr parity_before;
    lib_u8 value = 0u;
    lib_i32 filler = 0;
    lib_i32 failed = 0;
    lib_status expected = mode == 2u ? LIB_STATUS_INVALID_ARGUMENT :
        LIB_STATUS_NO_MEMORY;

    machine.board = &board;

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
    failed |= core_machine_d4_memory_configure(&machine, &config) != expected ||
        machine.board->d4_memory.configured ||
        memory->connect.device_provider_count != providers_before ||
        memory->connect.write_observer_count != observers_before ||
        memory->connect.parity != parity_before;

    core_machine_memory_unregister_owner(memory, &filler);
    if (mode == 2u) core_machine_memory_release_parity(memory);
    failed |= core_machine_d4_memory_configure(&machine, &config) != LIB_STATUS_OK ||
        !machine.board->d4_memory.configured ||
        memory->connect.device_provider_count != 2u ||
        memory->connect.write_observer_count != 1u ||
        memory->connect.parity_owner != &machine ||
        memory->connect.device_providers[0].owner != &machine ||
        memory->connect.device_providers[1].owner != &machine ||
        !memory->connect.device_providers[0].replacement ||
        !memory->connect.device_providers[1].replacement ||
        memory->connect.write_observers[0].owner != &machine;
    failed |= core_machine_memory_read_physical(memory, 0x80c00000u,
        (lib_uptr)&value, 1u) != LIB_STATUS_OK || value != 0xf7u;
    machine.board->d4_memory.parity_fault_mask = 1u;
    machine.board->d4_memory.ram_setup = 1u;
    core_machine_d4_memory_reset(&machine);
    failed |= machine.board->d4_memory.parity_fault_mask != 0u ||
        machine.board->d4_memory.ram_setup != config.ram_setup ||
        machine.board->d4_memory.control != 0xffu;
    failed |= core_machine_remove_memory_device_routes(&machine, &machine) !=
        LIB_STATUS_OK || memory->connect.device_provider_count != 0u ||
        memory->connect.write_observer_count != 0u ||
        memory->connect.parity != 0u;
    core_machine_memory_finalize(memory);
    return failed;
}

lib_i32 main(void)
{
    lib_i32 failed = 0;

    for (lib_u32 mode = 0u; mode < 4u; ++mode) failed |= run_case(mode);
    if (!failed) printf("M5:T540:S17:D4-MEMORY-TRANSACTION:OK\n");
    return failed ? 1 : 0;
}
