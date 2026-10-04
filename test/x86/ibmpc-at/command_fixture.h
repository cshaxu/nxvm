#ifndef TEST_AT_KBC_COMMAND_FIXTURE_H
#define TEST_AT_KBC_COMMAND_FIXTURE_H
#include "x86/ibmpc-at/kbc_interface.h"
#include "x86/core/port_interface.h"

/* Fixture command replies use the production ports and configured deadline.
 * The bounded loop accommodates the profile's status-poll delivery contract. */
static lib_i32 kbc_test_read_reply(t_kbc *kbc, core_machine *machine)
{
    lib_u8 attempt;
    for (attempt = 0u; attempt < 8u; ++attempt) {
        lib_u64 ticks;
        lib_u32 value;
        if (core_machine_bus_read(machine, 0x64u, &value) != LIB_STATUS_OK) return -1;
        if ((value & 0x01u) != 0u) {
            if (core_machine_bus_read(machine, 0x60u, &value) != LIB_STATUS_OK) return -1;
            return (lib_u8)value;
        }
        if (core_machine_kbc_ticks_until_event(kbc, &ticks) == LIB_STATUS_OK) {
            core_machine_kbc_advance(kbc, ticks);
        }
    }
    return -1;
}

static lib_bool kbc_test_command_matches(t_kbc *kbc, core_machine *machine,
    lib_u8 command, lib_u8 mask, lib_u8 expected)
{
    lib_i32 reply;
    if (core_machine_bus_write(machine, 0x64u, command) != LIB_STATUS_OK) return LIB_FALSE;
    reply = kbc_test_read_reply(kbc, machine);
    return reply >= 0 && (reply & mask) == expected;
}
#endif
