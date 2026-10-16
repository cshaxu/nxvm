#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/debug_interface.h"
#include "x86-nxvm/machine/lifecycle.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "../support/rom/session_assets.h"
#include "x86-nxvm/machine/machine_private.h"

lib_i32 main(void)
{
    x86_nxvm_machine *session = LIB_NULL;
    lib_u8 value = 0x5au;
    lib_u8 read_value = 0u;
    lib_u32 port_value = 0u;
    lib_i32 failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || !session->active || session->x86_core_machine == LIB_NULL) {
        return 1;
    }
    x86_nxvm_machine_reset(session);
    failed |= x86_core_machine_debug_write_port(session->x86_core_machine, 0x03ceu, 6u) != LIB_STATUS_OK;
    failed |= x86_core_machine_bus_read(session->x86_core_machine, 0x03cfu, &port_value) != LIB_STATUS_UNSUPPORTED || port_value != 0u;
    failed |= x86_core_machine_debug_write_port(session->x86_core_machine, 0x03cfu, 0x09u) != LIB_STATUS_OK;
    failed |= x86_core_machine_bus_read(session->x86_core_machine, 0x03cfu, &port_value) != LIB_STATUS_UNSUPPORTED || port_value != 0u;
    failed |= x86_core_machine_memory_write(session->x86_core_machine, 0x000b0000u,
        &value, sizeof(value)) != LIB_STATUS_OK;
    failed |= x86_core_machine_memory_read(session->x86_core_machine, 0x000b0000u,
        &read_value, sizeof(read_value)) != LIB_STATUS_OK || read_value != value;
    failed |= x86_core_machine_debug_write_port(session->x86_core_machine, 0x03c0u, 0x32u) != LIB_STATUS_OK || x86_core_machine_debug_write_port(session->x86_core_machine, 0x03c0u, 0xffu) != LIB_STATUS_OK;
    failed |= x86_core_machine_bus_read(session->x86_core_machine, 0x03c1u, &port_value) != LIB_STATUS_UNSUPPORTED || port_value != 0u;
    (void)x86_core_machine_debug_read_port(session->x86_core_machine, 0x03dau, &port_value);
    failed |= x86_core_machine_debug_write_port(session->x86_core_machine, 0x03c0u, 0x10u) != LIB_STATUS_OK;
    port_value = 0x5au;
    failed |= x86_core_machine_bus_read(session->x86_core_machine, 0x03c1u, &port_value) != LIB_STATUS_UNSUPPORTED || port_value != 0x5au;
    x86_nxvm_machine_destroy(session);
    if (failed) return 1;
    printf("EGA-CONTROLLER:SYSTEM:OK\n");
    printf("REGRESSIONS:OK\n");
    return 0;
}
