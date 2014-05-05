/* Copyright 2012-2014 Neko. */

/* VPIT implements Programmable Interval Timer Intel 8254. */

#include "../utils.h"

#include "vpic.h"

#include "vbios.h"
#include "vport.h"
#include "vmachine.h"
#include "vpit.h"

t_pit vpit;

void vpitIntSystemTimer() {
    vpicSetIRQ(0x00);
}

/* Initializes counter when status is ready */
static void LoadInit(t_nubit8 id) {
    if (vpit.flagWrite[id] == VPIT_STATUS_RW_READY) {
        vpit.count[id] = vpit.init[id];
        vpit.flagReady[id] = True;
    }
}

/* Decreases count */
static void Decrease(t_nubit8 id) {
    vpit.count[id]--;
    if (GetBit(vpit.cw[id], VPIT_CW_BCD)) {
        if ((vpit.count[id] & 0x000f) == 0x000f) {
            vpit.count[id] = (vpit.count[id] & 0xfff0) | 0x0009;
        }
        if ((vpit.count[id] & 0x00f0) == 0x00f0) {
            vpit.count[id] = (vpit.count[id] & 0xff0f) | 0x0090;
        }
        if ((vpit.count[id] & 0x0f00) == 0x0f00) {
            vpit.count[id] = (vpit.count[id] & 0xf0ff) | 0x0900;
        }
        if ((vpit.count[id] & 0xf000) == 0xf000) {
            vpit.count[id] = (vpit.count[id] & 0x0fff) | 0x9000;
        }
    }
}

static void io_read_004x(t_nubit8 id) {
    if (vpit.flagLatch[id]) {
        if (vpit.flagRead[id] == VPIT_STATUS_RW_MSB) {
            vport.ioByte = GetMax8(vpit.latch[id] >> 8);
            vpit.flagRead[id] = VPIT_STATUS_RW_READY;
            vpit.flagLatch[id] = False; /* finish reading latch */
        } else {
            vport.ioByte = GetMax8(vpit.latch[id]);
            vpit.flagRead[id] = VPIT_STATUS_RW_MSB;
            vpit.flagLatch[id] = True; /* latch msb to be read */
        }
    } else {
        switch (VPIT_GetCW_RW(vpit.cw[id])) {
        case 0x00:
            break;
        case 0x01:
            vport.ioByte = GetMax8(vpit.count[id]);
            vpit.flagRead[id] = VPIT_STATUS_RW_READY;
            break;
        case 0x02:
            vport.ioByte = GetMax8(vpit.count[id] >> 8);
            vpit.flagRead[id] = VPIT_STATUS_RW_READY;
            break;
        case 0x03:
            if (vpit.flagRead[id] == VPIT_STATUS_RW_MSB) {
                vport.ioByte = GetMax8(vpit.count[id] >> 8);
                vpit.flagRead[id] = VPIT_STATUS_RW_READY;
            } else {
                vport.ioByte = GetMax8(vpit.count[id]);
                vpit.flagRead[id] = VPIT_STATUS_RW_MSB;
            }
            break;
        default:
            break;
        }
    }
}

static void io_write_004x(t_nubit8 id) {
    switch (VPIT_GetCW_RW(vpit.cw[id])) {
    case 0x00:
        return;
        break;
    case 0x01:
        vpit.init[id] = GetMax16(vport.ioByte);
        vpit.flagWrite[id] = VPIT_STATUS_RW_READY;
        break;
    case 0x02:
        vpit.init[id] = GetMax16(vport.ioByte << 8);
        vpit.flagWrite[id] = VPIT_STATUS_RW_READY;
        break;
    case 0x03:
        if (vpit.flagWrite[id] == VPIT_STATUS_RW_MSB) {
            vpit.init[id] = GetMax16(vport.ioByte << 8) | GetMax8(vpit.init[id]);
            vpit.flagWrite[id] = VPIT_STATUS_RW_READY;
        } else {
            vpit.init[id] = GetMax16(vport.ioByte);
            vpit.flagWrite[id] = VPIT_STATUS_RW_MSB;
        }
    default:
        break;
    }
    switch (VPIT_GetCW_M(vpit.cw[id])) {
    case 0x00:
        LoadInit(id);
        break;
    case 0x01:
        break;
    case 0x02:
    case 0x06:
        if (!vpit.flagReady[id]) {
            LoadInit(id);
        }
        break;
    case 0x03:
    case 0x07:
        if (!vpit.flagReady[id]) {
            LoadInit(id);
        }
        break;
    case 0x04:
        if (!vpit.flagReady[id]) {
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
    t_nubit8 id = VPIT_GetCW_SC(vport.ioByte);
    if (id == (VPIT_CW_SC >> 6)) {
        /* read-back command */
        vpit.cw[id] = vport.ioByte;
        /* TODO: implement read-back functionalities */
    } else {
        vpit.flagLatch[id] = False; /* unlatch when counter is re-programmed */
        switch (VPIT_GetCW_RW(vport.ioByte)) {
        case 0x00:
            /* latch command */
            vpit.flagLatch[id] = True;
            vpit.latch[id] = vpit.count[id];
            vpit.flagRead[id] = VPIT_STATUS_RW_LSB;
            break;
        case 0x01:
            /* LSB */
            vpit.cw[id] = vport.ioByte;
            vpit.flagReady[id] = False;
            vpit.flagRead[id] = VPIT_STATUS_RW_LSB;
            vpit.flagWrite[id] = VPIT_STATUS_RW_LSB;
            break;
        case 0x02:
            /* MSB */
            vpit.cw[id] = vport.ioByte;
            vpit.flagReady[id] = False;
            vpit.flagRead[id] = VPIT_STATUS_RW_MSB;
            vpit.flagWrite[id] = VPIT_STATUS_RW_MSB;
            break;
        case 0x03:
            /* 16-bit */
            vpit.cw[id] = vport.ioByte;
            vpit.flagReady[id] = False;
            vpit.flagRead[id] = VPIT_STATUS_RW_LSB;
            vpit.flagWrite[id] = VPIT_STATUS_RW_LSB;
            break;
        default:
            break;
        }
        if (VPIT_GetCW_M(vpit.cw[id]) != Zero8 && vpit.out[id]) {
            ExecFun(vpit.out[id]);
        }
    }
}

/* set gate value and load init */
void vpitSetGate(t_nubit8 id, t_bool flagGate) {
    if (VPIT_GetCW_M(vpit.cw[id]) != Zero8) {
        if (!vpit.flagGate[id] && flagGate) {
            LoadInit(id);
        }
    }
    vpit.flagGate[id] = flagGate;
}

#define vpitRefDRAM NULL
static void init() {
    MEMSET(&vpit, Zero8, sizeof(t_pit));
    /* GATE for counter 0 and 1 are connected */
    vpit.flagGate[0] = vpit.flagGate[1] = 1;
    vpit.out[0] = (t_faddrcc) vpitIntSystemTimer;
    vpit.out[1] = (t_faddrcc) vpitRefDRAM;
    vport.in[0x0040] = (t_faddrcc) io_read_0040;
    vport.in[0x0041] = (t_faddrcc) io_read_0041;
    vport.in[0x0042] = (t_faddrcc) io_read_0042;
    vport.out[0x0040] = (t_faddrcc) io_write_0040;
    vport.out[0x0041] = (t_faddrcc) io_write_0041;
    vport.out[0x0042] = (t_faddrcc) io_write_0042;
    vport.out[0x0043] = (t_faddrcc) io_write_0043;
    vpit.flagGate[0] = vpit.flagGate[1] = True;
    vbiosAddPost(VPIT_POST);
}

static void reset() {
    t_nubit8 i;
    for (i = 0; i < 3; ++i) {
        vpit.cw[i] = Zero8;
        vpit.init[i] = vpit.count[i] = vpit.latch[i] = Zero16;
        vpit.flagReady[i] = vpit.flagLatch[i] = True;
        vpit.flagRead[i] = vpit.flagWrite[i] = VPIT_STATUS_RW_READY;
    }
    vpit.cw[3] = Zero8;
}

static void refresh() {
    t_nubit8 i;
    for (i = 0; i < 3; ++i) {
        switch (VPIT_GetCW_M(vpit.cw[i])) {
        case 0x00:
            if (vpit.flagReady[i]) {
                if (vpit.flagGate[i]) {
                    Decrease(i);
                    if (vpit.count[i] == Zero16) {
                        if (vpit.out[i]) {
                            ExecFun(vpit.out[i]);
                        }
                        vpit.flagReady[i] = False;
                    }
                }
            }
            break;
        case 0x01:
            if (vpit.flagReady[i]) {
                Decrease(i);
                if (vpit.count[i] == Zero16) {
                    if (vpit.out[i]) {
                        ExecFun(vpit.out[i]);
                    }
                    vpit.flagReady[i] = False;
                }
            }
            break;
        case 0x02:
        case 0x06:
            if (vpit.flagReady[i]) {
                if (vpit.flagGate[i]) {
                    Decrease(i);
                    if (vpit.count[i] == 0x0001) {
                        if (vpit.out[i]) {
                            ExecFun(vpit.out[i]);
                        }
                        LoadInit(i);
                    }
                }
            }
            break;
        case 0x03:
        case 0x07:
            if (vpit.flagReady[i]) {
                if (vpit.flagGate[i]) {
                    Decrease(i);
                    if (vpit.count[i] == Zero16) {
                        if (vpit.out[i]) {
                            ExecFun(vpit.out[i]);
                        }
                        LoadInit(i);
                    }
                }
            }
            break;
        case 0x04:
            if (vpit.flagReady[i]) {
                if (vpit.flagGate[i]) {
                    Decrease(i);
                    if (vpit.count[i] == Zero16) {
                        if (vpit.out[i]) {
                            ExecFun(vpit.out[i]);
                        }
                        vpit.flagReady[i] = False;
                    }
                }
            }
            break;
        case 0x05:
            if (vpit.flagReady[i]) {
                Decrease(i);
                if (vpit.count[i] == Zero16) {
                    if (vpit.out[i]) {
                        ExecFun(vpit.out[i]);
                    }
                    vpit.flagReady[i] = False;
                }
            }
            break;
        default:
            break;
        }
    }
}

static void final() {}

void vpitRegister() {
    vmachineAddMe;
}

/* Print PIT status */
void devicePrintPit() {
    t_nubit8 id;
    for (id = 0; id < 3; ++id) {
        PRINTF("PIT INFO %d\n========\n",id);
        PRINTF("Control Word = %x, SC = %d, RW = %d, Mode = %d, BCD=%d\n",
               vpit.cw[id], VPIT_GetCW_SC(vpit.cw[id]), VPIT_GetCW_RW(vpit.cw[id]),
               VPIT_GetCW_M(vpit.cw[id]), GetBit(vpit.cw[id], VPIT_CW_BCD));
        PRINTF("Init = %x, Count = %x, Latch = %x\n",
               vpit.init[id], vpit.count[id], vpit.latch[id]);
        PRINTF("Flags: ready = %d, latch = %d, read = %d, write = %d, gate = %d, out = %x\n",
               vpit.flagReady[id], vpit.flagLatch[id], vpit.flagRead[id],
               vpit.flagWrite[id], vpit.flagGate[id], vpit.out[id]);
    }
    id = 3;
    PRINTF("PIT INFO %d (read-back)\n========\n",id);
    PRINTF("Control Word = %x, SC = %d, RW = %d, Mode = %d, BCD=%d\n",
           vpit.cw[id], VPIT_GetCW_SC(vpit.cw[id]), VPIT_GetCW_RW(vpit.cw[id]),
           VPIT_GetCW_M(vpit.cw[id]), GetBit(vpit.cw[id], VPIT_CW_BCD));
}
