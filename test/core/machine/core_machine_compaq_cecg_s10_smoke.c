#include "type.h"

#include "core/machine/port.h"
#include "core/machine/vadp.h"

C_INT main(C_VOID)
{
    const core_machine_vadp_cecg_config config = {
        0x40u, 0x05u, 0x30u, 0x01u, TYPE_TRUE, TYPE_FALSE, TYPE_TRUE,
        0x06u, 0x01u, TYPE_FALSE, TYPE_FALSE, TYPE_FALSE
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
    failed |= !core_machine_port_has_write(&port,
        CORE_MACHINE_VADP_PORT_COMPAQ_FEATURE_CONTROL) ||
        !core_machine_port_has_write(&generic_port,
        CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_COLOR) ||
        core_machine_port_has_read(&generic_port,
        CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT) ||
        core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT) !=
            0x05u;
    core_machine_port_write(&generic_port,
        CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_COLOR, 0x02u);
    failed |= generic_vadp.data.ega_feature_control != 0x02u;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_COMPAQ_FEATURE_CONTROL,
        0x03u);
    failed |= core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT) !=
        0x07u;
    core_machine_vadp_reset(&vadp);
    failed |= core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT) !=
        0x05u;

    core_machine_vadp_finalize(&generic_vadp);
    core_machine_vadp_finalize(&vadp);
    core_machine_port_finalize(&generic_port);
    core_machine_port_finalize(&port);
    if (!failed) {
        STD_PRINTF("M5:T386:S10:CECG-FEATURE-ENVIRONMENT:OK\n");
        return 0;
    }
    STD_FPRINTF(STD_STDERR, "M5:T386:S10:CECG-FEATURE-ENVIRONMENT:FAIL\n");
    return 1;
}
