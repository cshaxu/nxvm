/* Copyright 2012-2014 Neko. */

/* VPIT implements Programmable Interval Timer Intel 8254. */

#include "type.h"

#include "core/machine/pic.h"

#include "core/machine/port.h"
#include "core/machine/pit.h"

static t_pit *coreMachinePit;

t_pit *core_machine_pit_current(void) { return coreMachinePit; }
void core_machine_pit_bind_live(t_pit *pit) { coreMachinePit = pit; }
void core_machine_pit_unbind_live(void) { coreMachinePit = NULL; }

/* Initializes counter when status is ready */
static void LoadInit(t_pit *pit, t_nubit8 id) {
    if (pit->data.flagWrite[id] == VPIT_STATUS_RW_READY) {
        pit->data.count[id] = pit->data.init[id];
        pit->data.flagReady[id] = True;
    }
}
/* Decreases count */
static void Decrease(t_pit *pit, t_nubit8 id) {
    pit->data.count[id]--;
    if (GetBit(pit->data.cw[id], VPIT_CW_BCD)) {
        if ((pit->data.count[id] & 0x000f) == 0x000f) {
            pit->data.count[id] = (pit->data.count[id] & 0xfff0) | 0x0009;
        }
        if ((pit->data.count[id] & 0x00f0) == 0x00f0) {
            pit->data.count[id] = (pit->data.count[id] & 0xff0f) | 0x0090;
        }
        if ((pit->data.count[id] & 0x0f00) == 0x0f00) {
            pit->data.count[id] = (pit->data.count[id] & 0xf0ff) | 0x0900;
        }
        if ((pit->data.count[id] & 0xf000) == 0xf000) {
            pit->data.count[id] = (pit->data.count[id] & 0x0fff) | 0x9000;
        }
    }
}

static void io_read_004x(t_pit *pit, t_port *port, t_nubit8 id) {
    if (pit->data.flagLatch[id]) {
        if (pit->data.flagRead[id] == VPIT_STATUS_RW_MSB) {
            port->data.ioByte = GetMax8(pit->data.latch[id] >> 8);
            pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
            pit->data.flagLatch[id] = False; /* finish reading latch */
        } else {
            port->data.ioByte = GetMax8(pit->data.latch[id]);
            pit->data.flagRead[id] = VPIT_STATUS_RW_MSB;
            pit->data.flagLatch[id] = True; /* latch msb to be read */
        }
    } else {
        switch (VPIT_GetCW_RW(pit->data.cw[id])) {
        case 0x00:
            break;
        case 0x01:
            port->data.ioByte = GetMax8(pit->data.count[id]);
            pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
            break;
        case 0x02:
            port->data.ioByte = GetMax8(pit->data.count[id] >> 8);
            pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
            break;
        case 0x03:
            if (pit->data.flagRead[id] == VPIT_STATUS_RW_MSB) {
                port->data.ioByte = GetMax8(pit->data.count[id] >> 8);
                pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
            } else {
                port->data.ioByte = GetMax8(pit->data.count[id]);
                pit->data.flagRead[id] = VPIT_STATUS_RW_MSB;
            }
            break;
        default:
            break;
        }
    }
}
static void io_write_004x(t_pit *pit, t_port *port, t_nubit8 id) {
    switch (VPIT_GetCW_RW(pit->data.cw[id])) {
    case 0x00:
        return;
        break;
    case 0x01:
        pit->data.init[id] = GetMax16(port->data.ioByte);
        pit->data.flagWrite[id] = VPIT_STATUS_RW_READY;
        break;
    case 0x02:
        pit->data.init[id] = GetMax16(port->data.ioByte << 8);
        pit->data.flagWrite[id] = VPIT_STATUS_RW_READY;
        break;
    case 0x03:
        if (pit->data.flagWrite[id] == VPIT_STATUS_RW_MSB) {
            pit->data.init[id] = GetMax16(port->data.ioByte << 8) | GetMax8(pit->data.init[id]);
            pit->data.flagWrite[id] = VPIT_STATUS_RW_READY;
        } else {
            pit->data.init[id] = GetMax16(port->data.ioByte);
            pit->data.flagWrite[id] = VPIT_STATUS_RW_MSB;
        }
    default:
        break;
    }
    switch (VPIT_GetCW_M(pit->data.cw[id])) {
    case 0x00:
        LoadInit(pit, id);
        break;
    case 0x01:
        break;
    case 0x02:
    case 0x06:
        if (!pit->data.flagReady[id]) {
            LoadInit(pit, id);
        }
        break;
    case 0x03:
    case 0x07:
        if (!pit->data.flagReady[id]) {
            LoadInit(pit, id);
        }
        break;
    case 0x04:
        if (!pit->data.flagReady[id]) {
            LoadInit(pit, id);
        }
        break;
    case 0x05:
        break;
    default:
        break;
    }
}

static void io_read_0040(t_port *port, t_nubit16 port_id, void *owner) {
    (void)port_id;
    io_read_004x((t_pit *)owner, port, 0);
}
static void io_read_0041(t_port *port, t_nubit16 port_id, void *owner) {
    (void)port_id;
    io_read_004x((t_pit *)owner, port, 1);
}
static void io_read_0042(t_port *port, t_nubit16 port_id, void *owner) {
    (void)port_id;
    io_read_004x((t_pit *)owner, port, 2);
}
static void io_write_0040(t_port *port, t_nubit16 port_id, void *owner) {
    (void)port_id;
    io_write_004x((t_pit *)owner, port, 0);
}
static void io_write_0041(t_port *port, t_nubit16 port_id, void *owner) {
    (void)port_id;
    io_write_004x((t_pit *)owner, port, 1);
}
static void io_write_0042(t_port *port, t_nubit16 port_id, void *owner) {
    (void)port_id;
    io_write_004x((t_pit *)owner, port, 2);
}
/* write control word */
static void io_write_0043(t_port *port, t_nubit16 port_id, void *owner) {
    t_pit *pit = (t_pit *)owner;
    (void)port_id;
    t_nubit8 id = VPIT_GetCW_SC(port->data.ioByte);
    if (id == (VPIT_CW_SC >> 6)) {
        /* read-back command */
        pit->data.cw[id] = port->data.ioByte;
        /* TODO: implement read-back functionalities */
    } else {
        pit->data.flagLatch[id] = False; /* unlatch when counter is re-programmed */
        switch (VPIT_GetCW_RW(port->data.ioByte)) {
        case 0x00:
            /* latch command */
            pit->data.flagLatch[id] = True;
            pit->data.latch[id] = pit->data.count[id];
            pit->data.flagRead[id] = VPIT_STATUS_RW_LSB;
            break;
        case 0x01:
            /* LSB */
            pit->data.cw[id] = port->data.ioByte;
            pit->data.flagReady[id] = False;
            pit->data.flagRead[id] = VPIT_STATUS_RW_LSB;
            pit->data.flagWrite[id] = VPIT_STATUS_RW_LSB;
            break;
        case 0x02:
            /* MSB */
            pit->data.cw[id] = port->data.ioByte;
            pit->data.flagReady[id] = False;
            pit->data.flagRead[id] = VPIT_STATUS_RW_MSB;
            pit->data.flagWrite[id] = VPIT_STATUS_RW_MSB;
            break;
        case 0x03:
            /* 16-bit */
            pit->data.cw[id] = port->data.ioByte;
            pit->data.flagReady[id] = False;
            pit->data.flagRead[id] = VPIT_STATUS_RW_LSB;
            pit->data.flagWrite[id] = VPIT_STATUS_RW_LSB;
            break;
        default:
            break;
        }
        if (VPIT_GetCW_M(pit->data.cw[id]) != Zero8) {
            ExecFun(pit->connect.fpOut[id]);
        }
    }
}

/* set gate value and load init */
void vpitSetGate(t_nubit8 id, t_bool flagGate) {
    if (VPIT_GetCW_M(vpit.data.cw[id]) != Zero8) {
        if (!vpit.connect.flagGate[id] && flagGate) {
            LoadInit(&vpit, id);
        }
    }
    vpit.connect.flagGate[id] = flagGate;
}
void vpitAddDevice(t_nubit8 id, t_faddrcc fpOut) {
    vpit.connect.fpOut[id] = fpOut;
    /* GATE tells if counter is connected */
    vpit.connect.flagGate[id] = True;
}

void vpitInit() {
    core_machine_pit_initialize(core_machine_pit_current(),
        core_machine_port_current());
}

void core_machine_pit_initialize(t_pit *pit, t_port *port)
{
    if (pit == NULL || port == NULL) return;
    MEMSET((void *)pit, Zero8, sizeof(*pit));
    core_machine_port_add_read(port, 0x0040, io_read_0040, pit);
    core_machine_port_add_read(port, 0x0041, io_read_0041, pit);
    core_machine_port_add_read(port, 0x0042, io_read_0042, pit);
    core_machine_port_add_write(port, 0x0040, io_write_0040, pit);
    core_machine_port_add_write(port, 0x0041, io_write_0041, pit);
    core_machine_port_add_write(port, 0x0042, io_write_0042, pit);
    core_machine_port_add_write(port, 0x0043, io_write_0043, pit);
}
void vpitReset() {
    t_nubitcc i;
    MEMSET((void *)(&vpit.data), Zero8, sizeof(t_pit_data));
    for (i = 0; i < 3; ++i) {
        vpit.data.flagReady[i] = vpit.data.flagLatch[i] = True;
        vpit.data.flagRead[i] = vpit.data.flagWrite[i] = VPIT_STATUS_RW_READY;
    }
}
void vpitRefresh() {
    t_nubitcc i;
    for (i = 0; i < 3; ++i) {
        switch (VPIT_GetCW_M(vpit.data.cw[i])) {
        case 0x00:
            if (vpit.data.flagReady[i]) {
                if (vpit.connect.flagGate[i]) {
                    Decrease(&vpit, GetMax8(i));
                    if (vpit.data.count[i] == Zero16) {
                        ExecFun(vpit.connect.fpOut[i]);
                        vpit.data.flagReady[i] = False;
                    }
                }
            }
            break;
        case 0x01:
            if (vpit.data.flagReady[i]) {
                Decrease(&vpit, GetMax8(i));
                if (vpit.data.count[i] == Zero16) {
                    ExecFun(vpit.connect.fpOut[i]);
                    vpit.data.flagReady[i] = False;
                }
            }
            break;
        case 0x02:
        case 0x06:
            if (vpit.data.flagReady[i]) {
                if (vpit.connect.flagGate[i]) {
                    Decrease(&vpit, GetMax8(i));
                    if (vpit.data.count[i] == 0x0001) {
                        ExecFun(vpit.connect.fpOut[i]);
                        LoadInit(&vpit, GetMax8(i));
                    }
                }
            }
            break;
        case 0x03:
        case 0x07:
            if (vpit.data.flagReady[i]) {
                if (vpit.connect.flagGate[i]) {
                    Decrease(&vpit, GetMax8(i));
                    if (vpit.data.count[i] == Zero16) {
                        ExecFun(vpit.connect.fpOut[i]);
                        LoadInit(&vpit, GetMax8(i));
                    }
                }
            }
            break;
        case 0x04:
            if (vpit.data.flagReady[i]) {
                if (vpit.connect.flagGate[i]) {
                    Decrease(&vpit, GetMax8(i));
                    if (vpit.data.count[i] == Zero16) {
                        ExecFun(vpit.connect.fpOut[i]);
                        vpit.data.flagReady[i] = False;
                    }
                }
            }
            break;
        case 0x05:
            if (vpit.data.flagReady[i]) {
                Decrease(&vpit, GetMax8(i));
                if (vpit.data.count[i] == Zero16) {
                    ExecFun(vpit.connect.fpOut[i]);
                    vpit.data.flagReady[i] = False;
                }
            }
            break;
        default:
            break;
        }
    }
}
void vpitFinal() {}

/* Print PIT status */
void devicePrintPit() {
    t_nubit8 id;
    for (id = 0; id < 3; ++id) {
        PRINTF("PIT INFO %d\n========\n",id);
        PRINTF("Control Word = %x, SC = %d, RW = %d, Mode = %d, BCD=%d\n",
               vpit.data.cw[id], VPIT_GetCW_SC(vpit.data.cw[id]), VPIT_GetCW_RW(vpit.data.cw[id]),
               VPIT_GetCW_M(vpit.data.cw[id]), GetBit(vpit.data.cw[id], VPIT_CW_BCD));
        PRINTF("Init = %x, Count = %x, Latch = %x\n",
               vpit.data.init[id], vpit.data.count[id], vpit.data.latch[id]);
        PRINTF("Flags: ready = %d, latch = %d, read = %d, write = %d, gate = %d, out = %x\n",
               vpit.data.flagReady[id], vpit.data.flagLatch[id], vpit.data.flagRead[id],
               vpit.data.flagWrite[id], vpit.connect.flagGate[id], vpit.connect.fpOut[id]);
    }
    id = 3;
    PRINTF("PIT INFO %d (read-back)\n========\n",id);
    PRINTF("Control Word = %x, SC = %d, RW = %d, Mode = %d, BCD=%d\n",
           vpit.data.cw[id], VPIT_GetCW_SC(vpit.data.cw[id]), VPIT_GetCW_RW(vpit.data.cw[id]),
           VPIT_GetCW_M(vpit.data.cw[id]), GetBit(vpit.data.cw[id], VPIT_CW_BCD));
}
