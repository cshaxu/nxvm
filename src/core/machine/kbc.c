/* Copyright 2012-2014 Neko. */

/* VKBC is Keyboard Controller: Intel 8042 not implemented yet. */

#include "core/machine/port.h"
#include "core/machine/kbc.h"

static void core_machine_kbc_read_status(t_port *port, ntvdm64_type_unsigned_16 port_id,
    void *owner)
{
    (void)port_id;
    (void)owner;
    port->data.ioByte = VKBC_STATUS_KE;
}

void core_machine_kbc_register_ports(t_kbc *controller, t_port *port)
{
    core_machine_port_add_read(port, 0x0064,
        core_machine_kbc_read_status, controller);
}

void core_machine_kbc_initialize(t_kbc *controller, t_port *port) {
    if (controller == NULL || port == NULL) return;
    MEMSET(controller, NTVDM64_TYPE_ZERO_8, sizeof(*controller));
    core_machine_kbc_register_ports(controller, port);
}
void core_machine_kbc_reset(t_kbc *controller) { (void)controller; }
void core_machine_kbc_refresh(t_kbc *controller) { (void)controller; }
void core_machine_kbc_finalize(t_kbc *controller) { (void)controller; }
