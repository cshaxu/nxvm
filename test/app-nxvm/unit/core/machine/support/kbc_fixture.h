#ifndef TEST_NXVM_KBC_FIXTURE_H
#define TEST_NXVM_KBC_FIXTURE_H
#include "app-nxvm/devices/kbc.h"
#include "app-nxvm/devices/port.h"

/* Fixture command replies use the production ports and configured deadline.
 * The bounded loop accommodates the profile's status-poll delivery contract. */
static lib_i32 kbc_test_read_reply(t_kbc *kbc, t_port *port)
{
    lib_u8 attempt;
    for (attempt = 0u; attempt < 8u; ++attempt) {
        lib_u64 ticks;
        if ((core_machine_port_read(port, 0x64u) & VKBC_STATUS_OBF) != 0u) {
            return (lib_u8)core_machine_port_read(port, 0x60u);
        }
        if (core_machine_kbc_ticks_until_event(kbc, &ticks) == LIB_STATUS_OK) {
            core_machine_kbc_advance(kbc, ticks);
        }
    }
    return -1;
}

static lib_bool kbc_test_command_matches(t_kbc *kbc, t_port *port,
    lib_u8 command, lib_u8 mask, lib_u8 expected)
{
    lib_i32 reply;
    core_machine_port_write(port, 0x64u, command);
    reply = kbc_test_read_reply(kbc, port);
    return reply >= 0 && (reply & mask) == expected;
}
#endif
