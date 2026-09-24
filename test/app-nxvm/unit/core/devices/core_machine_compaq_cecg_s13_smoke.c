#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/port.h"
#include "app-nxvm/devices/vadp.h"

lib_i32 main(void)
{
    const core_machine_vadp_cecg_config config = {
        0x40u, 0x00u, 0x30u, 0x01u, LIB_TRUE, LIB_FALSE, LIB_TRUE,
        0x06u, 0x01u, LIB_FALSE, LIB_FALSE, LIB_FALSE
    };
    t_port port;
    t_port generic_port;
    t_vadp vadp;
    t_vadp generic_vadp;
    lib_i32 failed = 0;

    core_machine_port_initialize(&port);
    core_machine_port_initialize(&generic_port);
    core_machine_vadp_initialize(&vadp, &port);
    core_machine_vadp_initialize(&generic_vadp, &generic_port);
    core_machine_vadp_configure_ega_ports(&vadp, &port);
    core_machine_vadp_configure_ega_ports(&generic_vadp, &generic_port);
    failed |= core_machine_vadp_configure_ega_personality(&vadp, &port,
        CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) != LIB_STATUS_OK ||
        core_machine_vadp_configure_cecg(&vadp, &config) != LIB_STATUS_OK ||
        core_machine_vadp_configure_ega_personality(&generic_vadp, &generic_port,
        CORE_MACHINE_VADP_EGA_PERSONALITY_GENERIC) != LIB_STATUS_OK;
    failed |= !core_machine_port_has_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT) ||
        !core_machine_port_has_read(&generic_port,
        CORE_MACHINE_VADP_PORT_EGA_INPUT_STATUS_0) ||
        core_machine_port_has_read(&generic_port,
        CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT);
    failed |= core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT) != 0xe0u;
    failed |= (core_machine_port_read(&generic_port,
        CORE_MACHINE_VADP_PORT_EGA_INPUT_STATUS_0) & 0x70u) != 0u;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT,
        0x00u);
    failed |= core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT) != 0xf0u;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT,
        0x04u);
    failed |= core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT) != 0xe0u;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT,
        0x08u);
    failed |= core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT) != 0xe0u;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT,
        0x0cu);
    failed |= core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT) != 0xf0u;
    core_machine_vadp_reset(&vadp);
    failed |= core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT) != 0xe0u;

    core_machine_vadp_finalize(&generic_vadp);
    core_machine_vadp_finalize(&vadp);
    core_machine_port_finalize(&generic_port);
    core_machine_port_finalize(&port);
    if (!failed) {
        printf("M5:T386:S13:CECG-INPUT-STATUS-0:OK\n");
        return 0;
    }
    fprintf(stderr, "M5:T386:S13:CECG-INPUT-STATUS-0:FAIL\n");
    return 1;
}
