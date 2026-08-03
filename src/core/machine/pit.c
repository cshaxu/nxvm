/* Copyright 2012-2014 Neko. */

/* VPIT implements Programmable Interval Timer Intel 8254. */

#include "type.h"

#include "core/machine/pic.h"

#include "core/machine/port.h"
#include "core/machine/pit.h"

/* Initializes counter when status is ready */
static C_VOID LoadInit(t_pit *pit, ntvdm64_type_unsigned_8 id) {
    if (pit->data.flagWrite[id] == VPIT_STATUS_RW_READY) {
        pit->data.count[id] = pit->data.init[id];
        pit->data.flagReady[id] = NTVDM64_TYPE_TRUE;
    }
}
/* Decreases count */
static C_VOID Decrease(t_pit *pit, ntvdm64_type_unsigned_8 id) {
    pit->data.count[id]--;
    if (NTVDM64_TYPE_GET_BIT(pit->data.cw[id], VPIT_CW_BCD)) {
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

static C_VOID io_read_004x(t_pit *pit, t_port *port, ntvdm64_type_unsigned_8 id) {
    if (pit->data.flagLatch[id]) {
        if (pit->data.flagRead[id] == VPIT_STATUS_RW_MSB) {
            port->data.ioByte = NTVDM64_TYPE_MASK_UNSIGNED_8(pit->data.latch[id] >> 8);
            pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
            pit->data.flagLatch[id] = NTVDM64_TYPE_FALSE; /* finish reading latch */
        } else {
            port->data.ioByte = NTVDM64_TYPE_MASK_UNSIGNED_8(pit->data.latch[id]);
            pit->data.flagRead[id] = VPIT_STATUS_RW_MSB;
            pit->data.flagLatch[id] = NTVDM64_TYPE_TRUE; /* latch msb to be read */
        }
    } else {
        switch (VPIT_GetCW_RW(pit->data.cw[id])) {
        case 0x00:
            break;
        case 0x01:
            port->data.ioByte = NTVDM64_TYPE_MASK_UNSIGNED_8(pit->data.count[id]);
            pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
            break;
        case 0x02:
            port->data.ioByte = NTVDM64_TYPE_MASK_UNSIGNED_8(pit->data.count[id] >> 8);
            pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
            break;
        case 0x03:
            if (pit->data.flagRead[id] == VPIT_STATUS_RW_MSB) {
                port->data.ioByte = NTVDM64_TYPE_MASK_UNSIGNED_8(pit->data.count[id] >> 8);
                pit->data.flagRead[id] = VPIT_STATUS_RW_READY;
            } else {
                port->data.ioByte = NTVDM64_TYPE_MASK_UNSIGNED_8(pit->data.count[id]);
                pit->data.flagRead[id] = VPIT_STATUS_RW_MSB;
            }
            break;
        default:
            break;
        }
    }
}
static C_VOID io_write_004x(t_pit *pit, t_port *port, ntvdm64_type_unsigned_8 id) {
    switch (VPIT_GetCW_RW(pit->data.cw[id])) {
    case 0x00:
        return;
        break;
    case 0x01:
        pit->data.init[id] = NTVDM64_TYPE_MASK_UNSIGNED_16(port->data.ioByte);
        pit->data.flagWrite[id] = VPIT_STATUS_RW_READY;
        break;
    case 0x02:
        pit->data.init[id] = NTVDM64_TYPE_MASK_UNSIGNED_16(port->data.ioByte << 8);
        pit->data.flagWrite[id] = VPIT_STATUS_RW_READY;
        break;
    case 0x03:
        if (pit->data.flagWrite[id] == VPIT_STATUS_RW_MSB) {
            pit->data.init[id] = NTVDM64_TYPE_MASK_UNSIGNED_16(port->data.ioByte << 8) | NTVDM64_TYPE_MASK_UNSIGNED_8(pit->data.init[id]);
            pit->data.flagWrite[id] = VPIT_STATUS_RW_READY;
        } else {
            pit->data.init[id] = NTVDM64_TYPE_MASK_UNSIGNED_16(port->data.ioByte);
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

static C_VOID io_read_0040(t_port *port, ntvdm64_type_unsigned_16 port_id, C_VOID *owner) {
    (C_VOID)port_id;
    io_read_004x((t_pit *)owner, port, 0);
}

static C_VOID Emit(t_pit *pit, ntvdm64_type_unsigned_8 id) {
    if (pit->connect.output[id] != STD_NULL) {
        pit->connect.output[id](pit->connect.output_owner[id]);
    }
}
static C_VOID io_read_0041(t_port *port, ntvdm64_type_unsigned_16 port_id, C_VOID *owner) {
    (C_VOID)port_id;
    io_read_004x((t_pit *)owner, port, 1);
}
static C_VOID io_read_0042(t_port *port, ntvdm64_type_unsigned_16 port_id, C_VOID *owner) {
    (C_VOID)port_id;
    io_read_004x((t_pit *)owner, port, 2);
}
static C_VOID io_write_0040(t_port *port, ntvdm64_type_unsigned_16 port_id, C_VOID *owner) {
    (C_VOID)port_id;
    io_write_004x((t_pit *)owner, port, 0);
}
static C_VOID io_write_0041(t_port *port, ntvdm64_type_unsigned_16 port_id, C_VOID *owner) {
    (C_VOID)port_id;
    io_write_004x((t_pit *)owner, port, 1);
}
static C_VOID io_write_0042(t_port *port, ntvdm64_type_unsigned_16 port_id, C_VOID *owner) {
    (C_VOID)port_id;
    io_write_004x((t_pit *)owner, port, 2);
}
/* write control word */
static C_VOID io_write_0043(t_port *port, ntvdm64_type_unsigned_16 port_id, C_VOID *owner) {
    t_pit *pit = (t_pit *)owner;
    (C_VOID)port_id;
    ntvdm64_type_unsigned_8 id = VPIT_GetCW_SC(port->data.ioByte);
    if (id == (VPIT_CW_SC >> 6)) {
        /* read-back command */
        pit->data.cw[id] = port->data.ioByte;
        /* TODO(Medium): Implement 8254 read-back status/count latching. */
    } else {
        pit->data.flagLatch[id] = NTVDM64_TYPE_FALSE; /* unlatch when counter is re-programmed */
        switch (VPIT_GetCW_RW(port->data.ioByte)) {
        case 0x00:
            /* latch command */
            pit->data.flagLatch[id] = NTVDM64_TYPE_TRUE;
            pit->data.latch[id] = pit->data.count[id];
            pit->data.flagRead[id] = VPIT_STATUS_RW_LSB;
            break;
        case 0x01:
            /* LSB */
            pit->data.cw[id] = port->data.ioByte;
            pit->data.flagReady[id] = NTVDM64_TYPE_FALSE;
            pit->data.flagRead[id] = VPIT_STATUS_RW_LSB;
            pit->data.flagWrite[id] = VPIT_STATUS_RW_LSB;
            break;
        case 0x02:
            /* MSB */
            pit->data.cw[id] = port->data.ioByte;
            pit->data.flagReady[id] = NTVDM64_TYPE_FALSE;
            pit->data.flagRead[id] = VPIT_STATUS_RW_MSB;
            pit->data.flagWrite[id] = VPIT_STATUS_RW_MSB;
            break;
        case 0x03:
            /* 16-bit */
            pit->data.cw[id] = port->data.ioByte;
            pit->data.flagReady[id] = NTVDM64_TYPE_FALSE;
            pit->data.flagRead[id] = VPIT_STATUS_RW_LSB;
            pit->data.flagWrite[id] = VPIT_STATUS_RW_LSB;
            break;
        default:
            break;
        }
        if (VPIT_GetCW_M(pit->data.cw[id]) != NTVDM64_TYPE_ZERO_8) {
            Emit(pit, id);
        }
    }
}

C_VOID core_machine_pit_set_output(t_pit *pit, ntvdm64_type_unsigned_8 id,
    core_machine_pit_output_provider provider, C_VOID *owner) {
    if (pit == STD_NULL || id >= 3u) return;
    pit->connect.output[id] = provider;
    pit->connect.output_owner[id] = owner;
    pit->connect.flagGate[id] = NTVDM64_TYPE_TRUE;
}

C_VOID core_machine_pit_initialize(t_pit *pit, t_port *port)
{
    if (pit == STD_NULL || port == STD_NULL) return;
    STD_MEMSET((C_VOID *)pit, NTVDM64_TYPE_ZERO_8, sizeof(*pit));
    core_machine_port_add_read(port, 0x0040, io_read_0040, pit);
    core_machine_port_add_read(port, 0x0041, io_read_0041, pit);
    core_machine_port_add_read(port, 0x0042, io_read_0042, pit);
    core_machine_port_add_write(port, 0x0040, io_write_0040, pit);
    core_machine_port_add_write(port, 0x0041, io_write_0041, pit);
    core_machine_port_add_write(port, 0x0042, io_write_0042, pit);
    core_machine_port_add_write(port, 0x0043, io_write_0043, pit);
}
C_VOID core_machine_pit_reset(t_pit *pit) {
    ntvdm64_type_native_unsigned i;
    if (pit == STD_NULL) return;
    STD_MEMSET((C_VOID *)(&pit->data), NTVDM64_TYPE_ZERO_8, sizeof(t_pit_data));
    for (i = 0; i < 3; ++i) {
        pit->data.flagReady[i] = pit->data.flagLatch[i] = NTVDM64_TYPE_TRUE;
        pit->data.flagRead[i] = pit->data.flagWrite[i] = VPIT_STATUS_RW_READY;
    }
}
C_VOID core_machine_pit_refresh(t_pit *pit) {
    ntvdm64_type_native_unsigned i;
    if (pit == STD_NULL) return;
    for (i = 0; i < 3; ++i) {
        switch (VPIT_GetCW_M(pit->data.cw[i])) {
        case 0x00:
            if (pit->data.flagReady[i]) {
                if (pit->connect.flagGate[i]) {
                    Decrease(pit, NTVDM64_TYPE_MASK_UNSIGNED_8(i));
                    if (pit->data.count[i] == NTVDM64_TYPE_ZERO_16) {
                        Emit(pit, i);
                        pit->data.flagReady[i] = NTVDM64_TYPE_FALSE;
                    }
                }
            }
            break;
        case 0x01:
            if (pit->data.flagReady[i]) {
                Decrease(pit, NTVDM64_TYPE_MASK_UNSIGNED_8(i));
                if (pit->data.count[i] == NTVDM64_TYPE_ZERO_16) {
                    Emit(pit, i);
                    pit->data.flagReady[i] = NTVDM64_TYPE_FALSE;
                }
            }
            break;
        case 0x02:
        case 0x06:
            if (pit->data.flagReady[i]) {
                if (pit->connect.flagGate[i]) {
                    Decrease(pit, NTVDM64_TYPE_MASK_UNSIGNED_8(i));
                    if (pit->data.count[i] == 0x0001) {
                        Emit(pit, i);
                        LoadInit(pit, NTVDM64_TYPE_MASK_UNSIGNED_8(i));
                    }
                }
            }
            break;
        case 0x03:
        case 0x07:
            if (pit->data.flagReady[i]) {
                if (pit->connect.flagGate[i]) {
                    Decrease(pit, NTVDM64_TYPE_MASK_UNSIGNED_8(i));
                    if (pit->data.count[i] == NTVDM64_TYPE_ZERO_16) {
                        Emit(pit, i);
                        LoadInit(pit, NTVDM64_TYPE_MASK_UNSIGNED_8(i));
                    }
                }
            }
            break;
        case 0x04:
            if (pit->data.flagReady[i]) {
                if (pit->connect.flagGate[i]) {
                    Decrease(pit, NTVDM64_TYPE_MASK_UNSIGNED_8(i));
                    if (pit->data.count[i] == NTVDM64_TYPE_ZERO_16) {
                        Emit(pit, i);
                        pit->data.flagReady[i] = NTVDM64_TYPE_FALSE;
                    }
                }
            }
            break;
        case 0x05:
            if (pit->data.flagReady[i]) {
                Decrease(pit, NTVDM64_TYPE_MASK_UNSIGNED_8(i));
                if (pit->data.count[i] == NTVDM64_TYPE_ZERO_16) {
                    Emit(pit, i);
                    pit->data.flagReady[i] = NTVDM64_TYPE_FALSE;
                }
            }
            break;
        default:
            break;
        }
    }
}
C_VOID core_machine_pit_finalize(t_pit *pit) { (C_VOID)pit; }
