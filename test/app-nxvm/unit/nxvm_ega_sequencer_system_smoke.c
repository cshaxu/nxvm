#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/debug_interface.h"
#include "x86-nxvm/board-base/vadp_interface.h"
#include "x86-nxvm/machine/lifecycle.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "../support/rom/session_assets.h"
#include "x86-nxvm/machine/machine_private.h"

lib_i32 main(void)
{
    x86_nxvm_machine *session = LIB_NULL;
    lib_u8 aperture_value = 0x5au;
    lib_u8 read_value = 0u;
    lib_u32 port_value = 0u;
    lib_i32 failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || !session->active || session->x86_core_machine == LIB_NULL) {
        failed = 1;
        goto done;
    }
    failed = x86_nxvm_machine_reset(session) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(session->x86_core_machine, 0x03c4u, &port_value) != LIB_STATUS_UNSUPPORTED || port_value != 0u ||
        x86_core_machine_bus_read(session->x86_core_machine, 0x03c5u, &port_value) != LIB_STATUS_UNSUPPORTED || port_value != 0u ||
        x86_core_machine_debug_write_port(session->x86_core_machine, 0x03c4u, 4u) != LIB_STATUS_OK || x86_core_machine_debug_write_port(session->x86_core_machine, 0x03c5u, 0xffu) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(session->x86_core_machine, 0x03c5u, &port_value) != LIB_STATUS_UNSUPPORTED || port_value != 0u ||
        x86_core_machine_memory_write(session->x86_core_machine,
        X86_NXVM_BOARD_VADP_EGA_APERTURE_BASE, &aperture_value,
        sizeof(aperture_value)) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(session->x86_core_machine,
        X86_NXVM_BOARD_VADP_EGA_APERTURE_BASE, &read_value,
        sizeof(read_value)) != LIB_STATUS_OK || read_value != aperture_value;

done:
    x86_nxvm_machine_destroy(session);
    if (failed) return 1;
    printf("EGA-SEQUENCER:SYSTEM:OK\n");
    return 0;
}
