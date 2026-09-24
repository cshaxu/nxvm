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
    failed |= !core_machine_port_has_write(&port,
        CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX) ||
        !core_machine_port_has_read(&port, CORE_MACHINE_VADP_PORT_MONO_STATUS) ||
        !core_machine_port_has_write(&generic_port,
        CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX) ||
        core_machine_port_has_read(&generic_port,
        CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT);

    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x0eu);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x12u);
    failed |= core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_CRTC_DATA) != 0x12u;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT,
        0x00u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x0eu);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_DATA, 0x56u);
    failed |= core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_CRTC_DATA) != 0u;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_COMPAQ_FEATURE_CONTROL, 0x03u);
    failed |= core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT) !=
        0x00u;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX, 0x0eu);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_MONO_CRTC_DATA, 0x34u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_MONO_STATUS, 0x02u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_MONO_LIGHTPEN_LATCH_SET, 0u);
    failed |= core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_MONO_CRTC_DATA) != 0x34u ||
        core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT) != 0x02u ||
        (core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_MONO_STATUS) & 0x02u) == 0u;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT,
        0x01u);
    failed |= core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_MONO_CRTC_DATA) != 0u ||
        core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_CRTC_DATA) != 0x34u;
    core_machine_vadp_reset(&vadp);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x0eu);
    failed |= core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_MONO_CRTC_DATA) != 0u ||
        core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_CRTC_DATA) != 0x34u ||
        core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT) != 0u;

    core_machine_vadp_finalize(&generic_vadp);
    core_machine_vadp_finalize(&vadp);
    core_machine_port_finalize(&generic_port);
    core_machine_port_finalize(&port);
    if (!failed) {
        printf("M5:T386:S12:CECG-IO-BASE:OK\n");
        return 0;
    }
    fprintf(stderr, "M5:T386:S12:CECG-IO-BASE:FAIL\n");
    return 1;
}
