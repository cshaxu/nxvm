/* Copyright 2012-2014 Neko. */

/* VPORT is the hub that connects all devices with the I/O port. */

#include "type.h"

#include "core/machine/port.h"

static t_port *coreMachinePort;

t_port *core_machine_port_current(void)
{
    return coreMachinePort;
}

void core_machine_port_bind_live(t_port *port)
{
    coreMachinePort = port;
}

void core_machine_port_unbind_live(void)
{
    coreMachinePort = NULL;
}

void vportAddRead(t_nubit16 portId, t_faddrcc fpIn) {
    vport.connect.fpIn[portId] = fpIn;
}
void vportAddWrite(t_nubit16 portId, t_faddrcc fpOut) {
    vport.connect.fpOut[portId] = fpOut;
}
void vportExecRead(t_nubit16 portId) {
    core_machine_port_execute_read(core_machine_port_current(), portId);
}
void vportExecWrite(t_nubit16 portId) {
    core_machine_port_execute_write(core_machine_port_current(), portId);
}

void core_machine_port_execute_read(t_port *port, t_nubit16 port_id)
{
    if (port == NULL) return;
    ExecFun(port->connect.fpIn[port_id]);
}

void core_machine_port_execute_write(t_port *port, t_nubit16 port_id)
{
    if (port == NULL) return;
    ExecFun(port->connect.fpOut[port_id]);
}

uint32_t core_machine_port_read(t_port *port, uint16_t port_id)
{
    if (port == NULL) return 0u;
    core_machine_port_execute_read(port, port_id);
    return port->data.ioDWord;
}

void core_machine_port_write(t_port *port, uint16_t port_id, uint32_t value)
{
    if (port == NULL) return;
    port->data.ioDWord = value;
    core_machine_port_execute_write(port, port_id);
}

void vportInit() {
    MEMSET((void *)(&vport), Zero8, sizeof(t_port));
}
void vportReset() {
    MEMSET((void *)(&vport.data), Zero8, sizeof(t_port_data));
}
void vportRefresh() {}
void vportFinal() {}

uint32_t core_machine_port_read_legacy(uint16_t port)
{
    return core_machine_port_read(core_machine_port_current(), port);
}

void core_machine_port_write_legacy(uint16_t port, uint32_t value)
{
    core_machine_port_write(core_machine_port_current(), port, value);
}
