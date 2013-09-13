/* This file is a part of NXVM project. */

#include "vapi.h"
#include "vport.h"
#include "vpic.h"
#include "vpit.h"

t_pit vpit;

#define GetSC(cw)  (((cw) & 0xc0)>>0x06)
#define GetRW(cw)  (((cw) & 0x30)>>0x04)
#define GetM(cw)   (((cw) & 0x0e)>>0x01)
#define GetBCD(cw) (((cw) & 0x01))

void vpitIntSystemTimer() {vpicSetIRQ(0x00);}

static void LoadInit(t_nubit8 id)
{
	if (vpit.flagwrite[id] == VPIT_STATUS_RW_READY) {
		vpit.count[id] = vpit.init[id];
		vpit.flagready[id] = 1;
	}
}
static void Decrease(t_nubit8 id)
{
	vpit.count[id] -= 0x0001;
	if (GetBCD(vpit.cw[id])) {
		if ((vpit.count[id] & 0x000f) == 0x000f)
			vpit.count[id] = (vpit.count[id] & 0xfff0) | 0x0009;
		if ((vpit.count[id] & 0x00f0) == 0x00f0)
			vpit.count[id] = (vpit.count[id] & 0xff0f) | 0x0090;
		if ((vpit.count[id] & 0x0f00) == 0x0f00)
			vpit.count[id] = (vpit.count[id] & 0xf0ff) | 0x0900;
		if ((vpit.count[id] & 0xf000) == 0xf000)
			vpit.count[id] = (vpit.count[id] & 0x0fff) | 0x9000;
	}
}

static void IO_Read_004x(t_nubit8 id)
{
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
		switch (GetRW(vpit.cw[id])) {
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
static void IO_Write_004x(t_nubit8 id)
{
	switch (GetRW(vpit.cw[id])) {
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
	switch (GetM(vpit.cw[id])) {
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

void IO_Read_0040() {IO_Read_004x(0);}
void IO_Read_0041() {IO_Read_004x(1);}
void IO_Read_0042() {IO_Read_004x(2);}
void IO_Write_0040() {IO_Write_004x(0);}
void IO_Write_0041() {IO_Write_004x(1);}
void IO_Write_0042() {IO_Write_004x(2);}
void IO_Write_0043()
{
	t_nubit8 id = GetSC(vport.iobyte);
	if (id == 0x03) {                                       /* read-back command */
		vpit.cw[id] = vport.iobyte;
		/* TODO: implement read-back functionalities */
	} else {
		vpit.flaglatch[id] = 0;      /* unlatch when counter is re-programmed */
		switch (GetRW(vport.iobyte)) {
		case 0x00:                                              /* latch command */
			vpit.flaglatch[id] = 1;
			vpit.latch[id] = vpit.count[id];
			vpit.flagread[id] = VPIT_STATUS_RW_LSB;
			break;
		case 0x01:                                                        /* LSB */
			vpit.cw[id] = vport.iobyte;
			vpit.flagready[id] = 0;
			vpit.flagread[id] = VPIT_STATUS_RW_LSB;
			vpit.flagwrite[id] = VPIT_STATUS_RW_LSB;
			break;
		case 0x02:                                                        /* MSB */
			vpit.cw[id] = vport.iobyte;
			vpit.flagready[id] = 0;
			vpit.flagread[id] = VPIT_STATUS_RW_MSB;
			vpit.flagwrite[id] = VPIT_STATUS_RW_MSB;
			break;
		case 0x03:                                                     /* 16-bit */
			vpit.cw[id] = vport.iobyte;
			vpit.flagready[id] = 0;
			vpit.flagread[id] = VPIT_STATUS_RW_LSB;
			vpit.flagwrite[id] = VPIT_STATUS_RW_LSB;
			break;
		default:
			break;
		}
		if (GetM(vpit.cw[id]) != 0x00 && vpit.out[id]) ExecFun(vpit.out[id]); 
	}
}

#ifdef VPIT_DEBUG
void IO_Read_FF40()
{
	t_nubit8 id;
	vport.iobyte = 0xff;
	for (id = 0;id < 3;++id) {
		vapiPrint("PIT INFO %d\n========\n",id);
		vapiPrint("Control Word = %x, SC = %d, RW = %d, Mode = %d, BCD=%d\n",
			vpit.cw[id], GetSC(vpit.cw[id]), GetRW(vpit.cw[id]),
			GetM(vpit.cw[id]), GetBCD(vpit.cw[id]));
		vapiPrint("Init = %x, Count = %x, Latch = %x\n",
			vpit.init[id], vpit.count[id], vpit.latch[id]);
		vapiPrint("Flags: ready = %d, latch = %d, read = %d, write = %d, gate = %d, out = %x\n",
			vpit.flagready[id], vpit.flaglatch[id], vpit.flagread[id],
			vpit.flagwrite[id], vpit.flaggate[id], vpit.out[id]);
	}
	id = 3;
	vapiPrint("PIT INFO %d (read-back)\n========\n",id);
	vapiPrint("Control Word = %x, SC = %d, RW = %d, Mode = %d, BCD=%d\n",
	vpit.cw[id], GetSC(vpit.cw[id]), GetRW(vpit.cw[id]),
		GetM(vpit.cw[id]), GetBCD(vpit.cw[id]));
}
void IO_Write_FF40() {vpitSetGate(vport.iobyte>>0x04,vport.iobyte&0x01);}
void IO_Read_FF41() {vport.iobyte = 0xff;vpitRefresh();}
#endif

void vpitSetGate(t_nubit8 id, t_bool flaggate)
{
	if (GetM(vpit.cw[id]) != 0)
		if (vpit.flaggate[id] == 0 && flaggate == 1)
			LoadInit(id);
	vpit.flaggate[id] = flaggate;
}

#define vpitRefDRAM NULL

void vpitInit()
{
	memset(&vpit,0,sizeof(t_pit));
	vpit.flaggate[0] = vpit.flaggate[1] = 1;
	                               /* GATE for counter 0 and 1 are connected */
	vpit.out[0] = (t_faddrcc)vpitIntSystemTimer;
	vpit.out[1] = (t_faddrcc)vpitRefDRAM;
	vport.in[0x0040] = (t_faddrcc)IO_Read_0040;
	vport.in[0x0041] = (t_faddrcc)IO_Read_0041;
	vport.in[0x0042] = (t_faddrcc)IO_Read_0042;
	vport.out[0x0040] = (t_faddrcc)IO_Write_0040;
	vport.out[0x0041] = (t_faddrcc)IO_Write_0041;
	vport.out[0x0042] = (t_faddrcc)IO_Write_0042;
	vport.out[0x0043] = (t_faddrcc)IO_Write_0043;
	vpit.flaggate[0] = vpit.flaggate[1] = 1;
#ifdef VPIT_DEBUG
	vport.in[0xff40] = (t_faddrcc)IO_Read_FF40;
	vport.out[0xff40] = (t_faddrcc)IO_Write_FF40;
	vport.in[0xff41] = (t_faddrcc)IO_Read_FF41;
#endif
}
void vpitReset()
{
	t_nubit8 i;
	for(i = 0;i < 3;++i) {
		vpit.cw[i] = 0x00;
		vpit.init[i] = vpit.count[i] = vpit.latch[i] = 0x0000;
		vpit.flagready[i] = vpit.flaglatch[i] = 1;
		vpit.flagread[i] = vpit.flagwrite[i] = VPIT_STATUS_RW_READY;
	}
	vpit.cw[3] = 0x00;
}
void vpitRefresh()
{
	t_nubit8 i;
	for (i = 0;i < 3;++i) {
		switch (GetM(vpit.cw[i])) {
		case 0x00:
			if (vpit.flagready[i]) {
				if (vpit.flaggate[i]) {
					Decrease(i);
					if (vpit.count[i] == 0x00) {
						if (vpit.out[i]) ExecFun(vpit.out[i]);
						vpit.flagready[i] = 0;
					}
				}
			}
			break;
		case 0x01:
			if (vpit.flagready[i]) {
				Decrease(i);
				if (vpit.count[i] == 0x00) {
					if (vpit.out[i]) ExecFun(vpit.out[i]);
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
						if (vpit.out[i]) ExecFun(vpit.out[i]);
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
						if (vpit.out[i]) ExecFun(vpit.out[i]);
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
						if (vpit.out[i]) ExecFun(vpit.out[i]);
						vpit.flagready[i] = 0;
					}
				}
			}
			break;
		case 0x05:
			if (vpit.flagready[i]) {
				Decrease(i);
				if (vpit.count[i] == 0x00) {
					if (vpit.out[i]) ExecFun(vpit.out[i]);
					vpit.flagready[i] = 0;
				}
			}
			break;
		default:
			break;
		}
	}
}
void vpitFinal() {}
