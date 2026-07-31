/* Copyright 2012-2014 Neko. */

/* VPIT implements Programmable Interval Timer Intel 8254. */

#include "../utils.h"

#include "vpic.h"

#include "vbios.h"
#include "vport.h"
#include "vpit.h"

t_pit vpit;

/* Initializes counter when status is ready */
static void LoadInit(t_nubit8 id) {
    if (vpit.data.flagWrite[id] == VPIT_STATUS_RW_READY) {
        vpit.data.count[id] = vpit.data.init[id];
        vpit.data.flagReady[id] = True;
    }
}
/* Decreases count */
static void Decrease(t_nubit8 id) {
    vpit.data.count[id]--;
    if (GetBit(vpit.data.cw[id], VPIT_CW_BCD)) {
        if ((vpit.data.count[id] & 0x000f) == 0x000f) {
            vpit.data.count[id] = (vpit.data.count[id] & 0xfff0) | 0x0009;
        }
        if ((vpit.data.count[id] & 0x00f0) == 0x00f0) {
            vpit.data.count[id] = (vpit.data.count[id] & 0xff0f) | 0x0090;
        }
        if ((vpit.data.count[id] & 0x0f00) == 0x0f00) {
            vpit.data.count[id] = (vpit.data.count[id] & 0xf0ff) | 0x0900;
        }
        if ((vpit.data.count[id] & 0xf000) == 0xf000) {
            vpit.data.count[id] = (vpit.data.count[id] & 0x0fff) | 0x9000;
        }
    }
}

static void io_read_004x(t_nubit8 id) {
    if (vpit.data.flagLatch[id]) {
        if (vpit.data.flagRead[id] == VPIT_STATUS_RW_MSB) {
            vport.data.ioByte = GetMax8(vpit.data.latch[id] >> 8);
            vpit.data.flagRead[id] = VPIT_STATUS_RW_READY;
            vpit.data.flagLatch[id] = False; /* finish reading latch */
        } else {
            vport.data.ioByte = GetMax8(vpit.data.latch[id]);
            vpit.data.flagRead[id] = VPIT_STATUS_RW_MSB;
            vpit.data.flagLatch[id] = True; /* latch msb to be read */
        }
    } else {
        switch (VPIT_GetCW_RW(vpit.data.cw[id])) {
        case 0x00:
            break;
        case 0x01:
            vport.data.ioByte = GetMax8(vpit.data.count[id]);
            vpit.data.flagRead[id] = VPIT_STATUS_RW_READY;
            break;
        case 0x02:
            vport.data.ioByte = GetMax8(vpit.data.count[id] >> 8);
            vpit.data.flagRead[id] = VPIT_STATUS_RW_READY;
            break;
        case 0x03:
            if (vpit.data.flagRead[id] == VPIT_STATUS_RW_MSB) {
                vport.data.ioByte = GetMax8(vpit.data.count[id] >> 8);
                vpit.data.flagRead[id] = VPIT_STATUS_RW_READY;
            } else {
                vport.data.ioByte = GetMax8(vpit.data.count[id]);
                vpit.data.flagRead[id] = VPIT_STATUS_RW_MSB;
            }
            break;
        default:
            break;
        }
    }
}
static void io_write_004x(t_nubit8 id) {
    switch (VPIT_GetCW_RW(vpit.data.cw[id])) {
    case 0x00:
        return;
        break;
    case 0x01:
        vpit.data.init[id] = GetMax16(vport.data.ioByte);
        vpit.data.flagWrite[id] = VPIT_STATUS_RW_READY;
        break;
    case 0x02:
        vpit.data.init[id] = GetMax16(vport.data.ioByte << 8);
        vpit.data.flagWrite[id] = VPIT_STATUS_RW_READY;
        break;
    case 0x03:
        if (vpit.data.flagWrite[id] == VPIT_STATUS_RW_MSB) {
            vpit.data.init[id] = GetMax16(vport.data.ioByte << 8) | GetMax8(vpit.data.init[id]);
            vpit.data.flagWrite[id] = VPIT_STATUS_RW_READY;
        } else {
            vpit.data.init[id] = GetMax16(vport.data.ioByte);
            vpit.data.flagWrite[id] = VPIT_STATUS_RW_MSB;
        }
    default:
        break;
    }
    switch (VPIT_GetCW_M(vpit.data.cw[id])) {
    case 0x00:
        LoadInit(id);
        break;
    case 0x01:
        break;
    case 0x02:
    case 0x06:
        if (!vpit.data.flagReady[id]) {
            LoadInit(id);
        }
        break;
    case 0x03:
    case 0x07:
        if (!vpit.data.flagReady[id]) {
            LoadInit(id);
        }
        break;
    case 0x04:
        if (!vpit.data.flagReady[id]) {
            LoadInit(id);
        }
        break;
    case 0x05:
        break;
    default:
        break;
    }
}

/* read counter 0 */
static void io_read_0040() {
    io_read_004x(0);
}
/* read counter 1 */
static void io_read_0041() {
    io_read_004x(1);
}
/* read counter 2 */
static void io_read_0042() {
    io_read_004x(2);
}
/* write counter 0 */
static void io_write_0040() {
    io_write_004x(0);
}
/* write counter 1 */
static void io_write_0041() {
    io_write_004x(1);
}
/* write counter 2 */
static void io_write_0042() {
    io_write_004x(2);
}
/* write control word */
static void io_write_0043() {
    t_nubit8 id = VPIT_GetCW_SC(vport.data.ioByte);
    if (id == (VPIT_CW_SC >> 6)) {
        /* read-back command */
        vpit.data.cw[id] = vport.data.ioByte;
        /* TODO: implement read-back functionalities */
    } else {
        vpit.data.flagLatch[id] = False; /* unlatch when counter is re-programmed */
        switch (VPIT_GetCW_RW(vport.data.ioByte)) {
        case 0x00:
            /* latch command */
            vpit.data.flagLatch[id] = True;
            vpit.data.latch[id] = vpit.data.count[id];
            vpit.data.flagRead[id] = VPIT_STATUS_RW_LSB;
            break;
        case 0x01:
            /* LSB */
            vpit.data.cw[id] = vport.data.ioByte;
            vpit.data.flagReady[id] = False;
            vpit.data.flagRead[id] = VPIT_STATUS_RW_LSB;
            vpit.data.flagWrite[id] = VPIT_STATUS_RW_LSB;
            break;
        case 0x02:
            /* MSB */
            vpit.data.cw[id] = vport.data.ioByte;
            vpit.data.flagReady[id] = False;
            vpit.data.flagRead[id] = VPIT_STATUS_RW_MSB;
            vpit.data.flagWrite[id] = VPIT_STATUS_RW_MSB;
            break;
        case 0x03:
            /* 16-bit */
            vpit.data.cw[id] = vport.data.ioByte;
            vpit.data.flagReady[id] = False;
            vpit.data.flagRead[id] = VPIT_STATUS_RW_LSB;
            vpit.data.flagWrite[id] = VPIT_STATUS_RW_LSB;
            break;
        default:
            break;
        }
        if (VPIT_GetCW_M(vpit.data.cw[id]) != Zero8) {
            ExecFun(vpit.connect.fpOut[id]);
        }
    }
}

/* set gate value and load init */
void vpitSetGate(t_nubit8 id, t_bool flagGate) {
    if (VPIT_GetCW_M(vpit.data.cw[id]) != Zero8) {
        if (!vpit.connect.flagGate[id] && flagGate) {
            LoadInit(id);
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
    MEMSET((void *)(&vpit), Zero8, sizeof(t_pit));
    vportAddRead(0x0040, (t_faddrcc) io_read_0040);
    vportAddRead(0x0041, (t_faddrcc) io_read_0041);
    vportAddRead(0x0042, (t_faddrcc) io_read_0042);
    vportAddWrite(0x0040, (t_faddrcc) io_write_0040);
    vportAddWrite(0x0041, (t_faddrcc) io_write_0041);
    vportAddWrite(0x0042, (t_faddrcc) io_write_0042);
    vportAddWrite(0x0043, (t_faddrcc) io_write_0043);
    vbiosAddPost(VPIT_POST);
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
                    Decrease(GetMax8(i));
                    if (vpit.data.count[i] == Zero16) {
                        ExecFun(vpit.connect.fpOut[i]);
                        vpit.data.flagReady[i] = False;
                    }
                }
            }
            break;
        case 0x01:
            if (vpit.data.flagReady[i]) {
                Decrease(GetMax8(i));
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
                    Decrease(GetMax8(i));
                    if (vpit.data.count[i] == 0x0001) {
                        ExecFun(vpit.connect.fpOut[i]);
                        LoadInit(GetMax8(i));
                    }
                }
            }
            break;
        case 0x03:
        case 0x07:
            if (vpit.data.flagReady[i]) {
                if (vpit.connect.flagGate[i]) {
                    Decrease(GetMax8(i));
                    if (vpit.data.count[i] == Zero16) {
                        ExecFun(vpit.connect.fpOut[i]);
                        LoadInit(GetMax8(i));
                    }
                }
            }
            break;
        case 0x04:
            if (vpit.data.flagReady[i]) {
                if (vpit.connect.flagGate[i]) {
                    Decrease(GetMax8(i));
                    if (vpit.data.count[i] == Zero16) {
                        ExecFun(vpit.connect.fpOut[i]);
                        vpit.data.flagReady[i] = False;
                    }
                }
            }
            break;
        case 0x05:
            if (vpit.data.flagReady[i]) {
                Decrease(GetMax8(i));
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
