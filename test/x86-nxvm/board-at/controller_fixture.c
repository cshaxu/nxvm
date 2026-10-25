#include "kbc_fixture.h"
#include "controller_fixture.h"

static lib_bool reject_kbc_routes;

static lib_status kbc_route_install(x86_core_machine *machine,
    const x86_core_machine_port_route *routes, lib_size count)
{
    return reject_kbc_routes ? LIB_STATUS_NO_MEMORY :
        x86_core_machine_install_port_routes(machine, routes, count);
}

#define x86_core_machine_install_port_routes kbc_route_install
#include "x86-nxvm/board-at/kbc.c"
#undef x86_core_machine_install_port_routes

static lib_status admit_byte(x86_keyboard *keyboard, lib_u8 byte)
{
    lib_u8 output;
    lib_size count;
    return x86_keyboard_admit(keyboard, &byte, 1u, &output, 1u, &count);
}

lib_bool test_kbc_has_repeat(const t_kbc *kbc)
{
    lib_u64 ticks;
    return x86_keyboard_ticks_until_repeat(kbc->connect.keyboard, &ticks) == LIB_STATUS_OK;
}

static lib_status keyboard_repeat_byte(void *context, const lib_u8 *bytes, lib_size count)
{
    if (count != 1u) return LIB_STATUS_INVALID_ARGUMENT;
    *(lib_u8 *)context = bytes[0];
    return LIB_STATUS_OK;
}

lib_bool test_kbc_cadence_is(t_kbc *kbc, lib_u64 initial, lib_u64 interval)
{
    x86_keyboard *keyboard = kbc->connect.keyboard;
    lib_u64 ticks = 0u;
    lib_u8 byte = 0u;
    lib_bool failed = admit_byte(keyboard, 0x1cu) != LIB_STATUS_OK;
    failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_OK || ticks != initial;
    failed |= !x86_keyboard_advance(keyboard, initial, keyboard_repeat_byte, &byte) || byte != 0x1cu;
    failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_OK || ticks != interval;
    failed |= admit_byte(keyboard, 0xf0u) != LIB_STATUS_OK;
    failed |= admit_byte(keyboard, 0x1cu) != LIB_STATUS_OK;
    return !failed;
}

lib_bool test_kbc_repeat_byte(t_kbc *kbc, lib_u8 *byte)
{
    return x86_keyboard_advance(kbc->connect.keyboard, 1u, keyboard_repeat_byte, byte);
}

x86_keyboard_signals test_kbc_signals(const t_kbc *kbc)
{
    return x86_keyboard_get_signals(kbc->connect.keyboard);
}

lib_bool test_kbc_aux_enabled(const t_kbc *kbc)
{
    return x86_kbc8042_aux_enabled(kbc->chip);
}

void test_kbc_without_aux(t_kbc *kbc)
{
    x86_kbc8042_set_aux_present(kbc->chip, LIB_FALSE);
}

void test_kbc_set_test_inputs(t_kbc *kbc, lib_u8 inputs)
{
    x86_nxvm_board_kbc_set_test_inputs(kbc, inputs);
}

static lib_bool test_kbc_unpublished(const t_kbc *kbc)
{
    return kbc->connect.aux_device == LIB_NULL &&
        kbc->connect.keyboard == LIB_NULL && kbc->chip == LIB_NULL;
}

lib_bool test_kbc_bat_ready(const t_kbc *kbc)
{
    return x86_keyboard_get_signals(kbc->connect.keyboard).bat_ready;
}

static lib_status kbc_existing_write(void *owner, lib_u16 address,
    lib_u8 access_bytes, lib_u32 value)
{
    (void)access_bytes;
    (void)address;
    (void)value;
    *(lib_u8 *)owner = 0x5au;
    return LIB_STATUS_OK;
}

lib_bool test_kbc_construction_rollback(void)
{
    t_kbc kbc = {0};
    x86_core_machine *machine = test_kbc_create_executor();
    lib_bool failed;

    reject_kbc_routes = LIB_TRUE;
    failed = x86_nxvm_board_kbc_initialize(&kbc, machine) != LIB_STATUS_NO_MEMORY ||
        !test_kbc_unpublished(&kbc);
    reject_kbc_routes = LIB_FALSE;
    failed |= x86_nxvm_board_kbc_initialize(&kbc, machine) != LIB_STATUS_OK;
    if (!failed) {
        test_kbc_port_write(machine, 0x64u, 0xaau);
        failed |= test_kbc_port_read(machine, 0x60u) != 0x55u;
    }
    failed |= x86_core_machine_remove_port_routes(machine, &kbc) != LIB_STATUS_OK;
    x86_nxvm_board_kbc_finalize(&kbc);
    x86_nxvm_board_kbc_finalize(&kbc);
    failed |= x86_core_machine_bus_read(machine, 0x60u, &(lib_u32){0u}) !=
        LIB_STATUS_UNSUPPORTED;
    x86_core_machine_destroy(machine);
    machine = test_kbc_create_executor();
    {
        lib_u8 marker = 0u;
        const x86_core_machine_port_provider provider = {LIB_NULL, kbc_existing_write};

        failed |= x86_core_machine_install_port_provider(machine, 0x64u, 0x64u,
            &provider, &marker) != LIB_STATUS_OK ||
            x86_nxvm_board_kbc_initialize(&kbc, machine) != LIB_STATUS_INVALID_STATE ||
            !test_kbc_unpublished(&kbc) ||
            x86_nxvm_board_kbc_initialize(&kbc, machine) != LIB_STATUS_INVALID_STATE ||
            !test_kbc_unpublished(&kbc) ||
            x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x60u, &(lib_u32){0u}) != LIB_STATUS_UNSUPPORTED ||
            x86_core_machine_bus_write(machine, 0x60u, 0u) != LIB_STATUS_UNSUPPORTED ||
            x86_core_machine_bus_read(machine, 0x64u, &(lib_u32){0u}) != LIB_STATUS_UNSUPPORTED ||
            x86_core_machine_bus_write(machine, 0x64u, 0u) != LIB_STATUS_OK || marker != 0x5au;
        x86_nxvm_board_kbc_finalize(&kbc);
    }
    x86_core_machine_destroy(machine);
    return failed;
}
