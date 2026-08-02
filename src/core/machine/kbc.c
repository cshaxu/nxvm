/* Copyright 2012-2014 Neko. */

/* VKBC is Keyboard Controller: Intel 8042 not implemented yet. */

#include "core/machine/port.h"
#include "core/machine/kbc.h"

static t_kbc *coreMachineKbc;
t_kbc *core_machine_kbc_current(void) { return coreMachineKbc; }
void core_machine_kbc_bind_live(t_kbc *controller) { coreMachineKbc=controller; }
void core_machine_kbc_unbind_live(void) { coreMachineKbc=NULL; }

static void core_machine_kbc_read_status(t_port *port, void *owner)
{
    (void)owner;
    port->data.ioByte = VKBC_STATUS_KE;
}

void core_machine_kbc_register_ports(t_kbc *controller, t_port *port)
{
    core_machine_port_add_read(port, 0x0064,
        core_machine_kbc_read_status, controller);
}

void vkbcInit() {
    core_machine_kbc_register_ports(core_machine_kbc_current(),
        core_machine_port_current());
}
void vkbcReset() {}
void vkbcRefresh() {}
void vkbcFinal() {}
