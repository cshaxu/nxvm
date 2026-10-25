#include "app-mypc110/pc110_ec_mailbox_interface.h"

#define MYPC110_EC_DATA_PORT 0x15e8u
#define MYPC110_EC_COMMAND_STATUS_PORT 0x15ecu

static lib_status mypc110_ec_mailbox_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    mypc110_ec_mailbox *mailbox = owner;

    (void)tick;
    if (mailbox == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port != MYPC110_EC_COMMAND_STATUS_PORT) return LIB_STATUS_UNSUPPORTED;
    *out_value = 0u;
    return LIB_STATUS_OK;
}

static lib_status mypc110_ec_mailbox_write(void *owner, lib_u16 port, lib_u32 value)
{
    mypc110_ec_mailbox *mailbox = owner;

    if (mailbox == LIB_NULL || value > 0xffu) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == MYPC110_EC_COMMAND_STATUS_PORT) mailbox->command = (lib_u8)value;
    else if (port == MYPC110_EC_DATA_PORT) mailbox->data = (lib_u8)value;
    else return LIB_STATUS_UNSUPPORTED;
    return LIB_STATUS_OK;
}

lib_status mypc110_ec_mailbox_install_routes(x86_core_machine *machine,
    mypc110_ec_mailbox *mailbox)
{
    const x86_core_machine_port_route routes[] = {
        {MYPC110_EC_DATA_PORT, LIB_NULL, mypc110_ec_mailbox_write,
            mailbox, LIB_FALSE, 0u},
        {MYPC110_EC_COMMAND_STATUS_PORT, mypc110_ec_mailbox_read,
            mypc110_ec_mailbox_write, mailbox, LIB_FALSE, 0u}
    };

    if (machine == LIB_NULL || mailbox == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_core_machine_install_port_routes(machine, routes,
        sizeof(routes) / sizeof(routes[0]));
}
