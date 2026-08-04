/* Copyright 2012-2014 Neko. */

/* VKBC is Keyboard Controller: Intel 8042 not implemented yet. */

#include "type.h"

#include "core/machine/port.h"

#include "core/machine/kbc.h"

static C_VOID core_machine_kbc_read_status(t_port *port, type_unsigned_16 port_id,
    C_VOID *owner)
{
    (C_VOID)port_id;
    (C_VOID)owner;
    port->data.ioByte = VKBC_STATUS_KE;
}

C_VOID core_machine_kbc_register_ports(t_kbc *controller, t_port *port)
{
    core_machine_port_add_read(port, 0x0064,
        core_machine_kbc_read_status, controller);
}

C_VOID core_machine_kbc_initialize(t_kbc *controller, t_port *port) {
    if (controller == STD_NULL || port == STD_NULL) return;
    STD_MEMSET(controller, TYPE_ZERO_8, sizeof(*controller));
    core_machine_kbc_register_ports(controller, port);
}
C_VOID core_machine_kbc_reset(t_kbc *controller) { (C_VOID)controller; }
C_VOID core_machine_kbc_refresh(t_kbc *controller) { (C_VOID)controller; }
C_VOID core_machine_kbc_finalize(t_kbc *controller) { (C_VOID)controller; }
