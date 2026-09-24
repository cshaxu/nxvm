#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/debug_interface.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/machine_interface.h"
#include "support/rom/session_assets.h"
#include "app-nxvm/machine/machine_private.h"

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    lib_u8 value = 0x5au;
    lib_u8 read_value = 0u;
    lib_u32 port_value = 0u;
    lib_i32 failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || !session->active || session->core_machine == LIB_NULL) {
        return 1;
    }
    vm_machine_reset(session);
    failed |= core_machine_debug_write_port(session->core_machine, 0x03ceu, 6u) != LIB_STATUS_OK;
    failed |= core_machine_debug_read_port(session->core_machine, 0x03cfu, &port_value) != LIB_STATUS_OK || port_value != 0x05u;
    failed |= core_machine_debug_write_port(session->core_machine, 0x03cfu, 0x09u) != LIB_STATUS_OK;
    failed |= core_machine_debug_read_port(session->core_machine, 0x03cfu, &port_value) != LIB_STATUS_OK || port_value != 0x09u;
    failed |= core_machine_memory_write(session->core_machine, 0x000b0000u,
        &value, sizeof(value)) != LIB_STATUS_OK;
    failed |= core_machine_memory_read(session->core_machine, 0x000b0000u,
        &read_value, sizeof(read_value)) != LIB_STATUS_OK || read_value != value;
    failed |= core_machine_debug_write_port(session->core_machine, 0x03c0u, 0x32u) != LIB_STATUS_OK || core_machine_debug_write_port(session->core_machine, 0x03c0u, 0xffu) != LIB_STATUS_OK;
    failed |= core_machine_debug_read_port(session->core_machine, 0x03c1u, &port_value) != LIB_STATUS_OK || port_value != 0x0fu;
    (void)core_machine_debug_read_port(session->core_machine, 0x03dau, &port_value);
    failed |= core_machine_debug_write_port(session->core_machine, 0x03c0u, 0x10u) != LIB_STATUS_OK || core_machine_debug_read_port(session->core_machine, 0x03c1u, &port_value) != LIB_STATUS_OK || port_value != 0x01u;
    vm_machine_destroy(session);
    if (failed) return 1;
    printf("M5:T236:S3:EGA-CONTROLLER:SYSTEM:OK\n");
    printf("M5:T480:S3:REGRESSIONS:OK\n");
    return 0;
}
