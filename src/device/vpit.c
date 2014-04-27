/* Copyright 2012-2014 Neko. */

/* VPIT implements Programmable Interval Timer Intel 8254. */

#include "vmachine.h"
#include "vport.h"
#include "vbios.h"
#include "vpic.h"
#include "vpit.h"

t_pit vpit;

void vpitIntSystemTimer() {vpicSetIRQ(0x00);}

/* Initializes counter when status is ready */
static void LoadInit(t_nubit8 id) {
	if (vpit.flagwrite[id] == VPIT_STATUS_RW_READY) {
		vpit.count[id] = vpit.init[id];
		vpit.flagready[id] = 1;
	}
}

/* Decreases count */
static void Decrease(t_nubit8 id) {
	vpit.count[id] -= 0x0001;
	if (VPIT_GetBCD(vpit.cw[id])) {
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
	if (vpit.flaglatch[id]) {
		if (vpit.flagread[id] == VPIT_STATUS_RW_MSB) {
			vport.iobyte = (t_nubit8)(vpit.latch[id]>>0x08);
			vpit.flagread[id] = VPIT_STATUS_RW_READY;
			vpit.flaglatch[id] = 0;
		} else {
			vport.iobyte = (t_nubit8)(vpit.latch[id] & 0xff);
			vpit.flagread[id] = VPIT_STATUS_RW_MSB;
		}
	} else {
		switch (VPIT_GetRW(vpit.cw[id])) {
		case 0x00:
			break;
		case 0x01:
			vport.iobyte = (t_nubit8)(vpit.count[id] & 0xff);
			vpit.flagread[id] = VPIT_STATUS_RW_READY;
			break;
		case 0x02:
			vport.iobyte = (t_nubit8)(vpit.count[id]>>0x08);
			vpit.flagread[id] = VPIT_STATUS_RW_READY;
			break;
		case 0x03:
			if (vpit.flagread[id] == VPIT_STATUS_RW_MSB) {
				vport.iobyte = (t_nubit8)(vpit.count[id]>>0x08);
				vpit.flagread[id] = VPIT_STATUS_RW_READY;
			} else {
				vport.iobyte = (t_nubit8)(vpit.count[id] & 0xff);
				vpit.flagread[id] = VPIT_STATUS_RW_MSB;
			}
			break;
		default:
			break;
		}
	}
}

static void io_write_004x(t_nubit8 id) {
	switch (VPIT_GetRW(vpit.cw[id])) {
	case 0x00:
		return;
		break;
	case 0x01:
		vpit.init[id] = (t_nubit16)vport.iobyte;
		vpit.flagwrite[id] = VPIT_STATUS_RW_READY;
		break;
	case 0x02:
		vpit.init[id] = (vport.iobyte<<0x08);
		vpit.flagwrite[id] = VPIT_STATUS_RW_READY;
		break;
	case 0x03:
		if (vpit.flagwrite[id] == VPIT_STATUS_RW_MSB) {
			vpit.init[id] = (vport.iobyte<<0x08) | (vpit.init[id] & 0xff);
			vpit.flagwrite[id] = VPIT_STATUS_RW_READY;
		} else {
			vpit.init[id] = (t_nubit16)vport.iobyte;
			vpit.flagwrite[id] = VPIT_STATUS_RW_MSB;
		}
	default:
		break;
	}
	switch (VPIT_GetM(vpit.cw[id])) {
	case 0x00:
		LoadInit(id);
		break;
	case 0x01:
		break;
	case 0x02:
	case 0x06:
		if (!vpit.flagready[id]) LoadInit(id);
		break;
	case 0x03:
	case 0x07:
		if (!vpit.flagready[id]) LoadInit(id);
		break;
	case 0x04:
		if (!vpit.flagready[id]) LoadInit(id);
		break;
	case 0x05:
		break;
	default:
		break;
	}
}
                              
/* read counter 0 */
static void io_read_0040() {io_read_004x(0);}
/* read counter 1 */
static void io_read_0041() {io_read_004x(1);}
/* read counter 2 */
static void io_read_0042() {io_read_004x(2);}
/* write counter 0 */
static void io_write_0040() {io_write_004x(0);}
/* write counter 1 */
static void io_write_0041() {io_write_004x(1);}
/* write counter 2 */
static void io_write_0042() {io_write_004x(2);}
/* write control word */
static void io_write_0043() {
	t_nubit8 id = VPIT_GetSC(vport.iobyte);
	if (id == 0x03) {
		/* read-back command */
		vpit.cw[id] = vport.iobyte;
		/* TODO: implement read-back functionalities */
	} else {
		vpit.flaglatch[id] = 0; /* unlatch when counter is re-programmed */
		switch (VPIT_GetRW(vport.iobyte)) {
		case 0x00:
			/* latch command */
			vpit.flaglatch[id] = 1;
			vpit.latch[id] = vpit.count[id];
			vpit.flagread[id] = VPIT_STATUS_RW_LSB;
			break;
		case 0x01:
			/* LSB */
			vpit.cw[id] = vport.iobyte;
			vpit.flagready[id] = 0;
			vpit.flagread[id] = VPIT_STATUS_RW_LSB;
			vpit.flagwrite[id] = VPIT_STATUS_RW_LSB;
			break;
		case 0x02:
			/* MSB */
			vpit.cw[id] = vport.iobyte;
			vpit.flagready[id] = 0;
			vpit.flagread[id] = VPIT_STATUS_RW_MSB;
			vpit.flagwrite[id] = VPIT_STATUS_RW_MSB;
			break;
		case 0x03:
			/* 16-bit */
			vpit.cw[id] = vport.iobyte;
			vpit.flagready[id] = 0;
			vpit.flagread[id] = VPIT_STATUS_RW_LSB;
			vpit.flagwrite[id] = VPIT_STATUS_RW_LSB;
			break;
		default:
			break;
		}
		if (VPIT_GetM(vpit.cw[id]) != 0x00 && vpit.out[id]) {
			ExecFun(vpit.out[id]);
		}
	}
}

/* set gate value and load init */
void vpitSetGate(t_nubit8 id, t_bool flaggate) {
	if (VPIT_GetM(vpit.cw[id]) != 0) {
		if (vpit.flaggate[id] == 0 && flaggate == 1) {
			LoadInit(id);
		}
	}
	vpit.flaggate[id] = flaggate;
}

#define vpitRefDRAM NULL
static void init() {
	memset(&vpit,0,sizeof(t_pit));
	/* GATE for counter 0 and 1 are connected */
	vpit.flaggate[0] = vpit.flaggate[1] = 1;
	vpit.out[0] = (t_faddrcc) vpitIntSystemTimer;
	vpit.out[1] = (t_faddrcc) vpitRefDRAM;
	vport.in[0x0040] = (t_faddrcc) io_read_0040;
	vport.in[0x0041] = (t_faddrcc) io_read_0041;
	vport.in[0x0042] = (t_faddrcc) io_read_0042;
	vport.out[0x0040] = (t_faddrcc) io_write_0040;
	vport.out[0x0041] = (t_faddrcc) io_write_0041;
	vport.out[0x0042] = (t_faddrcc) io_write_0042;
	vport.out[0x0043] = (t_faddrcc) io_write_0043;
	vpit.flaggate[0] = vpit.flaggate[1] = 1;
    
    vbiosAddPost(VPIT_POST);
}

static void reset() {
	t_nubit8 i;
	for(i = 0;i < 3;++i) {
		vpit.cw[i] = 0x00;
		vpit.init[i] = vpit.count[i] = vpit.latch[i] = 0x0000;
		vpit.flagready[i] = vpit.flaglatch[i] = 1;
		vpit.flagread[i] = vpit.flagwrite[i] = VPIT_STATUS_RW_READY;
	}
	vpit.cw[3] = 0x00;
}

static void refresh() {
	t_nubit8 i;
	for (i = 0;i < 3;++i) {
		switch (VPIT_GetM(vpit.cw[i])) {
		case 0x00:
			if (vpit.flagready[i]) {
				if (vpit.flaggate[i]) {
					Decrease(i);
					if (vpit.count[i] == 0x00) {
						if (vpit.out[i]) {
							ExecFun(vpit.out[i]);
						}
						vpit.flagready[i] = 0;
					}
				}
			}
			break;
		case 0x01:
			if (vpit.flagready[i]) {
				Decrease(i);
				if (vpit.count[i] == 0x00) {
					if (vpit.out[i]) {
						ExecFun(vpit.out[i]);
					}
					vpit.flagready[i] = 0;
				}
			}
			break;
		case 0x02:
		case 0x06:
			if (vpit.flagready[i]) {
				if (vpit.flaggate[i]) {
					Decrease(i);
					if (vpit.count[i] == 0x01) {
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
			if (vpit.flagready[i]) {
				if (vpit.flaggate[i]) {
					Decrease(i);
					if (vpit.count[i] == 0x00) {
						if (vpit.out[i]) {
							ExecFun(vpit.out[i]);
						}
						LoadInit(i);
					}
				}
			}
			break;
		case 0x04:
			if (vpit.flagready[i]) {
				if (vpit.flaggate[i]) {
					Decrease(i);
					if (vpit.count[i] == 0x00) {
						if (vpit.out[i]) {
							ExecFun(vpit.out[i]);
						}
						vpit.flagready[i] = 0;
					}
				}
			}
			break;
		case 0x05:
			if (vpit.flagready[i]) {
				Decrease(i);
				if (vpit.count[i] == 0x00) {
					if (vpit.out[i]) {
						ExecFun(vpit.out[i]);
					}
					vpit.flagready[i] = 0;
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
	vmachine.deviceTable[VMACHINE_DEVICE_INIT][vmachine.numDevices] = (t_faddrcc) init;
	vmachine.deviceTable[VMACHINE_DEVICE_RESET][vmachine.numDevices] = (t_faddrcc) reset;
	vmachine.deviceTable[VMACHINE_DEVICE_REFRESH][vmachine.numDevices] = (t_faddrcc) refresh;
	vmachine.deviceTable[VMACHINE_DEVICE_FINAL][vmachine.numDevices] = (t_faddrcc) final;
	vmachine.numDevices++;
}
