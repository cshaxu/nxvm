#include "type.h"

#include "core/machine/port.h"
#include "core/machine/vadp.h"

C_INT main(C_VOID)
{
    const core_machine_vadp_cecg_config config = {
        0x50u, 0x00u, 0x30u, 0x01u, TYPE_TRUE
    };
    t_port port;
    t_port generic_port;
    t_vadp vadp;
    t_vadp generic_vadp;
    C_INT failed = 0;

    core_machine_port_initialize(&port);
    core_machine_port_initialize(&generic_port);
    core_machine_vadp_initialize(&vadp, &port);
    core_machine_vadp_initialize(&generic_vadp, &generic_port);
    core_machine_vadp_configure_ega_ports(&vadp, &port);
    core_machine_vadp_configure_ega_ports(&generic_vadp, &generic_port);
    failed |= core_machine_vadp_configure_ega_personality(&generic_vadp,
        &generic_port, CORE_MACHINE_VADP_EGA_PERSONALITY_GENERIC) != TYPE_STATUS_OK;
    failed |= core_machine_vadp_configure_ega_personality(&vadp, &port,
        CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) != TYPE_STATUS_OK ||
        core_machine_vadp_configure_cecg(&vadp, &config) != TYPE_STATUS_OK;
    failed |= !core_machine_port_has_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) ||
        !core_machine_port_has_write(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) ||
        !core_machine_port_has_write(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_RESET) ||
        !core_machine_port_has_write(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_SET) ||
        core_machine_port_has_read(&generic_port,
        CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE);
    failed |= core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) != 0x50u ||
        core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT) != 0x00u ||
        core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_DISPLAY_TYPE) != 0x30u ||
        core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_INITIAL_MODE) != 0x01u;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE,
        0x7fu);
    failed |= core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) != 0x5fu;
    core_machine_port_write(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_RESET, 0u);
    failed |= (core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_STATUS) & 0x06u) !=
        0x04u;
    core_machine_port_write(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_SET, 0u);
    failed |= (core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_STATUS) & 0x06u) !=
        0x06u;
    core_machine_vadp_reset(&vadp);
    failed |= core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) != 0x50u ||
        (core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_STATUS) & 0x06u) != 0x04u;

    core_machine_vadp_finalize(&generic_vadp);
    core_machine_vadp_finalize(&vadp);
    core_machine_port_finalize(&generic_port);
    core_machine_port_finalize(&port);
    if (!failed) {
        STD_PRINTF("M5:T386:S9:CECG-CONTRACT:OK\n");
        return 0;
    }
    STD_FPRINTF(STD_STDERR, "M5:T386:S9:CECG-CONTRACT:FAIL\n");
    return 1;
}