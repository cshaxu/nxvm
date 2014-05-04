/* Copyright 2012-2014 Neko. */

/* VDMA implements two chips of Direct Memory Access Controller: Intel 8237A (Master+Slave). */

#include "../utils.h"

#include "vram.h"
#include "vfdc.h"

#include "vbios.h"
#include "vport.h"
#include "vmachine.h"
#include "vdma.h"

t_latch vlatch;
t_dma vdma1, vdma2;

static void doReset(t_dma *rdma) {
	t_nubit8 i;
	rdma->command = Zero8;
	rdma->status  = Zero8;
	rdma->mask    = VDMA_MASK_VALID;
	rdma->request = Zero8;
	rdma->temp    = Zero8;
	rdma->flagMSB = False;
	rdma->drx     = Zero8;
	rdma->flagEOP = False;
	rdma->isr     = Zero8;
	for (i = 0;i < VDMA_CHANNEL_COUNT;++i) {
		rdma->channel[i].baseAddr = rdma->channel[i].currAddr = Zero16;
		rdma->channel[i].baseCount   = rdma->channel[i].currCount = Zero16;
		rdma->channel[i].mode = Zero16;
		rdma->channel[i].page = Zero8;
	}
}

static void io_read_CurrentAddress(t_dma *rdma, t_nubit8 id) {
	if (!rdma->flagMSB) {
		vport.ioByte = GetMax8(rdma->channel[id].currAddr);
	} else {
		vport.ioByte = GetMax8(rdma->channel[id].currAddr >> 8);
	}
	rdma->flagMSB = !rdma->flagMSB;
}
static void io_read_CurrentWordCount(t_dma *rdma, t_nubit8 id) {
	if (!rdma->flagMSB) {
		vport.ioByte = GetMax8(rdma->channel[id].currCount);
	} else {
		vport.ioByte = GetMax8(rdma->channel[id].currCount >> 8);
	}
	rdma->flagMSB = !rdma->flagMSB;
}
static void io_read_Status(t_dma *rdma) {
	vport.ioByte = rdma->status;
    ClrBit(rdma->status, VDMA_STATUS_TCS);
}
#define     io_read_Temp(rdma) (vport.ioByte = (rdma)->temp)
#define     io_read_Page(rdma, id) (vport.ioByte = (rdma)->channel[(id)].page)

static void io_write_Address(t_dma *rdma, t_nubit8 id) {
	if (!rdma->flagMSB) {
		rdma->channel[id].baseAddr  = GetMax16(vport.ioByte);
	} else {
		rdma->channel[id].baseAddr |= GetMax16(vport.ioByte << 8);
	}
	rdma->channel[id].currAddr = rdma->channel[id].baseAddr;
	rdma->flagMSB = !rdma->flagMSB;
}
static void io_write_WordCount(t_dma *rdma, t_nubit8 id) {
	if (!rdma->flagMSB) {
		rdma->channel[id].baseCount  = GetMax16(vport.ioByte);
	} else {
		rdma->channel[id].baseCount |= GetMax16(vport.ioByte << 8);
	}
	rdma->channel[id].currCount = rdma->channel[id].baseCount;
	rdma->flagMSB = !rdma->flagMSB;
}
#define     io_write_Command(rdma) ((rdma)->command = vport.ioByte)
static void io_write_Request_Single(t_dma *rdma) {
	MakeBit(rdma->request, VDMA_REQUEST_DRQ(VDMA_GetREQSC_CS(vport.ioByte)),
		GetBit(vport.ioByte, VDMA_REQSC_SR));
}
static void io_write_Mask_Single(t_dma *rdma) {
	MakeBit(rdma->mask, VDMA_MASK_DRQ(VDMA_GetMASKSC_CS(vport.ioByte)),
		GetBit(vport.ioByte, VDMA_MASKSC_SM));
}
#define     io_write_Mode(rdma) \
            ((rdma)->channel[VDMA_GetMODE_CS(vport.ioByte)].mode = vport.ioByte)
#define     io_write_Flipflop_Clear(rdma) ((rdma)->flagMSB = False)
#define     io_write_Reset(rdma) (doReset(rdma))
#define     io_write_Mask_Clear(rdma) ((rdma)->mask = Zero8)
#define     io_write_Mask_All(rdma) ((rdma)->mask = vport.ioByte & VDMA_MASKAC_VALID)
#define     io_write_Page(rdma, id, m) \
            ((rdma)->channel[(id)].page = vport.ioByte & (m))

static void io_read_0000()  {io_read_CurrentAddress   (&vdma1, 0);}
static void io_read_0001()  {io_read_CurrentWordCount (&vdma1, 0);}
static void io_read_0002()  {io_read_CurrentAddress   (&vdma1, 1);}
static void io_read_0003()  {io_read_CurrentWordCount (&vdma1, 1);}
static void io_read_0004()  {io_read_CurrentAddress   (&vdma1, 2);}
static void io_read_0005()  {io_read_CurrentWordCount (&vdma1, 2);}
static void io_read_0006()  {io_read_CurrentAddress   (&vdma1, 3);}
static void io_read_0007()  {io_read_CurrentWordCount (&vdma1, 3);}
static void io_read_0008()  {io_read_Status           (&vdma1   );}
static void io_read_000D()  {io_read_Temp             (&vdma1   );}

static void io_write_0000() {io_write_Address         (&vdma1, 0);}
static void io_write_0001() {io_write_WordCount       (&vdma1, 0);}
static void io_write_0002() {io_write_Address         (&vdma1, 1);}
static void io_write_0003() {io_write_WordCount       (&vdma1, 1);}
static void io_write_0004() {io_write_Address         (&vdma1, 2);}
static void io_write_0005() {io_write_WordCount       (&vdma1, 2);}
static void io_write_0006() {io_write_Address         (&vdma1, 3);}
static void io_write_0007() {io_write_WordCount       (&vdma1, 3);}
static void io_write_0008() {io_write_Command         (&vdma1   );}
static void io_write_0009() {io_write_Request_Single  (&vdma1   );}
static void io_write_000A() {io_write_Mask_Single     (&vdma1   );}
static void io_write_000B() {io_write_Mode            (&vdma1   );}
static void io_write_000C() {io_write_Flipflop_Clear  (&vdma1   );}
static void io_write_000D() {io_write_Reset           (&vdma1   );}
static void io_write_000E() {io_write_Mask_Clear      (&vdma1   );}
static void io_write_000F() {io_write_Mask_All        (&vdma1   );}

static void io_read_0081()  {io_read_Page             (&vdma1, 2);}
static void io_read_0082()  {io_read_Page             (&vdma1, 3);}
static void io_read_0083()  {io_read_Page             (&vdma1, 1);}
static void io_read_0087()  {io_read_Page             (&vdma1, 0);}
static void io_read_0089()  {io_read_Page             (&vdma2, 2);}
static void io_read_008A()  {io_read_Page             (&vdma2, 3);}
static void io_read_008B()  {io_read_Page             (&vdma2, 1);}
static void io_read_008F()  {io_read_Page             (&vdma2, 0);}

static void io_write_0081() {io_write_Page            (&vdma1, 2, Max8);}
static void io_write_0082() {io_write_Page            (&vdma1, 3, Max8);}
static void io_write_0083() {io_write_Page            (&vdma1, 1, Max8);}
static void io_write_0087() {io_write_Page            (&vdma1, 0, Max8);}
static void io_write_0089() {io_write_Page            (&vdma2, 2, 0xfe);}
static void io_write_008A() {io_write_Page            (&vdma2, 3, 0xfe);}
static void io_write_008B() {io_write_Page            (&vdma2, 1, 0xfe);}
static void io_write_008F() {io_write_Page            (&vdma2, 0, 0xfe);}

static void io_read_00C0()  {io_read_CurrentAddress   (&vdma2, 0);}
static void io_read_00C2()  {io_read_CurrentWordCount (&vdma2, 0);}
static void io_read_00C4()  {io_read_CurrentAddress   (&vdma2, 1);}
static void io_read_00C6()  {io_read_CurrentWordCount (&vdma2, 1);}
static void io_read_00C8()  {io_read_CurrentAddress   (&vdma2, 2);}
static void io_read_00CA()  {io_read_CurrentWordCount (&vdma2, 2);}
static void io_read_00CC()  {io_read_CurrentAddress   (&vdma2, 3);}
static void io_read_00CE()  {io_read_CurrentWordCount (&vdma2, 3);}
static void io_read_00D0()  {io_read_Status           (&vdma2   );}
static void io_read_00DA()  {io_read_Temp             (&vdma2   );}

static void io_write_00C0() {io_write_Address         (&vdma2, 0);}
static void io_write_00C2() {io_write_WordCount       (&vdma2, 0);}
static void io_write_00C4() {io_write_Address         (&vdma2, 1);}
static void io_write_00C6() {io_write_WordCount       (&vdma2, 1);}
static void io_write_00C8() {io_write_Address         (&vdma2, 2);}
static void io_write_00CA() {io_write_WordCount       (&vdma2, 2);}
static void io_write_00CC() {io_write_Address         (&vdma2, 3);}
static void io_write_00CE() {io_write_WordCount       (&vdma2, 3);}
static void io_write_00D0() {io_write_Command         (&vdma2   );}
static void io_write_00D2() {io_write_Request_Single  (&vdma2   );}
static void io_write_00D4() {io_write_Mask_Single     (&vdma2   );}
static void io_write_00D6() {io_write_Mode            (&vdma2   );}
static void io_write_00D8() {io_write_Flipflop_Clear  (&vdma2   );}
static void io_write_00DA() {io_write_Reset           (&vdma2   );}
static void io_write_00DC() {io_write_Mask_Clear      (&vdma2   );}
static void io_write_00DE() {io_write_Mask_All        (&vdma2   );}

static t_nubit8 GetRegTopId(t_dma *rdma, t_nubit8 reg) {
	t_nubit8 id = Zero8;
	if (reg == Zero8) {
		return 0x08;
	}
	reg = (reg << (VDMA_CHANNEL_COUNT - (rdma->drx))) | (reg >> (rdma->drx));
	while ((id < VDMA_CHANNEL_COUNT) && !GetMax1(reg >> id)) {
		id++;
	}
	return (id + rdma->drx) % VDMA_CHANNEL_COUNT;
}
static void IncreaseCurrAddr(t_dma *rdma, t_nubit8 id) {
	rdma->channel[id].currAddr++;
	if (rdma->channel[id].currAddr == Zero16) {
		rdma->channel[id].page++;
	}
}
static void DecreaseCurrAddr(t_dma *rdma, t_nubit8 id) {
	rdma->channel[id].currAddr--;
	if (rdma->channel[id].currAddr == Max16) {
		rdma->channel[id].page--;
	}
}
static void Transmission(t_dma *rdma, t_nubit8 id, t_bool flagword) {
	switch (VDMA_GetMODE_TT(rdma->channel[id].mode)) {
	case 0x00:
		/* verify */
		/* do nothing */
		rdma->channel[id].currCount--;
		if (GetBit(rdma->channel[id].mode, VDMA_MODE_AIDS)) {
			DecreaseCurrAddr(rdma, id);
		} else {
			IncreaseCurrAddr(rdma, id);
		}
		break;
	case 0x01:
		/* write */
		if (rdma->channel[id].fpReadDevice) {
			ExecFun(rdma->channel[id].fpReadDevice);
		}
		if (!flagword) {
			vramByte((rdma->channel[id].page << 16) + rdma->channel[id].currAddr) = vlatch.byte;
		} else {
			vramWord((rdma->channel[id].page << 16) + (rdma->channel[id].currAddr << 1)) = vlatch.word;
		}
		rdma->channel[id].currCount--;
		if (GetBit(rdma->channel[id].mode, VDMA_MODE_AIDS)) {
			DecreaseCurrAddr(rdma, id);
		} else {
			IncreaseCurrAddr(rdma, id);
		}
		break;
	case 0x02:
		/* read */
		if (!flagword) {
			vlatch.byte = vramByte((rdma->channel[id].page << 16) + rdma->channel[id].currAddr);
		} else {
			vlatch.word = vramWord((rdma->channel[id].page << 16) + (rdma->channel[id].currAddr << 1));
		}
		if (rdma->channel[id].fpWriteDevice) {
			ExecFun(rdma->channel[id].fpWriteDevice);
		}
		rdma->channel[id].currCount--;
		if (GetBit(rdma->channel[id].mode, VDMA_MODE_AIDS)) {
			DecreaseCurrAddr(rdma, id);
		} else {
			IncreaseCurrAddr(rdma, id);
		}
		break;
	case 0x03:
		/* illegal */
		break;
	default:
		break;
	}
}
static void Execute(t_dma *rdma, t_nubit8 id, t_bool flagword) {
	t_bool flagM2M = ((id == 0) &&
		VDMA_GetREQUEST_DRQ(rdma->request, 1) &&
		GetBit(rdma->command, VDMA_COMMAND_M2M));
    ClrBit(rdma->status, VDMA_STATUS_DRQ(id));
    ClrBit(rdma->request, VDMA_REQUEST_DRQ(id));
	if (GetBit(rdma->command, VDMA_COMMAND_R)) {
		rdma->drx = (id + 1) % VDMA_CHANNEL_COUNT;
	}
	if (flagM2M) {
		/* memory-to-memory */
		while (rdma->channel[0x01].currCount != 0xffff && !rdma->flagEOP) {
			rdma->temp = vramByte((rdma->channel[0].page << 16) + rdma->channel[0].currAddr);
			vramByte((rdma->channel[1].page << 16) + rdma->channel[1].currAddr) = rdma->temp;
			rdma->channel[1].currCount--;
			if (GetBit(rdma->channel[id].mode, VDMA_MODE_AIDS)) {
				DecreaseCurrAddr(rdma, 1);
				if (!GetBit(rdma->command, VDMA_COMMAND_C0AD)) {
					DecreaseCurrAddr(rdma, 0);
				}
			} else{
				IncreaseCurrAddr(rdma, 1);
				if (!GetBit(rdma->command, VDMA_COMMAND_C0AD)) {
					IncreaseCurrAddr(rdma, 0);
				}
			}
		}
		if (rdma->channel[0x01].currCount == Max16) {
            SetBit(rdma->status, VDMA_STATUS_TC(0));
			rdma->flagEOP = True;
		}
	} else {
		/* select mode and command */
		switch (VDMA_GetMODE_M(rdma->channel[id].mode)) {
		case 0x00:
			/* demand */
			while (rdma->channel[id].currCount != Max16 && !rdma->flagEOP
				&& VDMA_GetSTATUS_DRQ(rdma->status, id)) {
				Transmission(rdma, id, flagword);
			}
			break;
		case 0x01:
			/* single */
			Transmission(rdma, id, flagword);
			break;
		case 0x02:
			/* block */
			while (rdma->channel[id].currCount != Max16 && !rdma->flagEOP) {
				Transmission(rdma, id, flagword);
			}
			break;
		case 0x03:
			/* cascade */
			/* do nothing */
			rdma->flagEOP = True;
			break;
		default:
			break;
		}
		if (rdma->channel[id].currCount == Max16) {
            SetBit(rdma->status, VDMA_STATUS_TC(id)); /* set termination count */
			rdma->flagEOP = True;
		}
	}
	if (rdma->flagEOP) {
		rdma->isr = Zero8;
		if (rdma->channel[id].fpCloseDevice) {
			ExecFun(rdma->channel[id].fpCloseDevice);
		}
		if (GetBit(rdma->channel[id].mode, VDMA_MODE_AI)) {
			rdma->channel[id].currAddr = rdma->channel[id].baseAddr;
			rdma->channel[id].currCount = rdma->channel[id].baseCount;
			ClrBit(rdma->mask, VDMA_MASK_DRQ(id));
		} else {
			SetBit(rdma->mask, VDMA_MASK_DRQ(id));
		}
	}
	rdma->flagEOP = False;
}

void vdmaSetDRQ(t_nubit8 drqId) {
	switch (drqId) {
	case 0: case 1: case 2: case 3: 
		SetBit(vdma1.status, VDMA_STATUS_DRQ(drqId));
		break;
	case 5: case 6: case 7:
		SetBit(vdma2.status, VDMA_STATUS_DRQ(drqId - 4)); break;
	case 4: default: break;
	}
	if (vdma1.status & VDMA_STATUS_DRQS) {
        SetBit(vdma2.status, VDMA_STATUS_DRQ(0));
	} else {
        ClrBit(vdma2.status, VDMA_STATUS_DRQ(0));
	}
}

static void init() {
	MEMSET(&vdma1, Zero8, sizeof(t_dma));
	MEMSET(&vdma2, Zero8, sizeof(t_dma));

	/* connect device io functions with dma channels */
	vdma1.channel[2].fpReadDevice  = (t_faddrcc) vfdcDMARead;
	vdma1.channel[2].fpWriteDevice = (t_faddrcc) vfdcDMAWrite;
	vdma1.channel[2].fpCloseDevice = (t_faddrcc) vfdcDMAFinal;

	vport.in[0x0000] = (t_faddrcc) io_read_0000;
	vport.in[0x0001] = (t_faddrcc) io_read_0001;
	vport.in[0x0002] = (t_faddrcc) io_read_0002;
	vport.in[0x0003] = (t_faddrcc) io_read_0003;
	vport.in[0x0004] = (t_faddrcc) io_read_0004;
	vport.in[0x0005] = (t_faddrcc) io_read_0005;
	vport.in[0x0006] = (t_faddrcc) io_read_0006;
	vport.in[0x0007] = (t_faddrcc) io_read_0007;
	vport.in[0x0008] = (t_faddrcc) io_read_0008;
	vport.in[0x000d] = (t_faddrcc) io_read_000D;

	vport.out[0x0000] = (t_faddrcc) io_write_0000;
	vport.out[0x0001] = (t_faddrcc) io_write_0001;
	vport.out[0x0002] = (t_faddrcc) io_write_0002;
	vport.out[0x0003] = (t_faddrcc) io_write_0003;
	vport.out[0x0004] = (t_faddrcc) io_write_0004;
	vport.out[0x0005] = (t_faddrcc) io_write_0005;
	vport.out[0x0006] = (t_faddrcc) io_write_0006;
	vport.out[0x0007] = (t_faddrcc) io_write_0007;
	vport.out[0x0008] = (t_faddrcc) io_write_0008;
	vport.out[0x0009] = (t_faddrcc) io_write_0009;
	vport.out[0x000a] = (t_faddrcc) io_write_000A;
	vport.out[0x000b] = (t_faddrcc) io_write_000B;
	vport.out[0x000c] = (t_faddrcc) io_write_000C;
	vport.out[0x000d] = (t_faddrcc) io_write_000D;
	vport.out[0x000e] = (t_faddrcc) io_write_000E;
	vport.out[0x000f] = (t_faddrcc) io_write_000F;

	vport.in[0x0081] = (t_faddrcc) io_read_0081;
	vport.in[0x0082] = (t_faddrcc) io_read_0082;
	vport.in[0x0083] = (t_faddrcc) io_read_0083;
	vport.in[0x0087] = (t_faddrcc) io_read_0087;
	vport.in[0x0089] = (t_faddrcc) io_read_0089;
	vport.in[0x008a] = (t_faddrcc) io_read_008A;
	vport.in[0x008b] = (t_faddrcc) io_read_008B;
	vport.in[0x008f] = (t_faddrcc) io_read_008F;

	vport.out[0x0081] = (t_faddrcc) io_write_0081;
	vport.out[0x0082] = (t_faddrcc) io_write_0082;
	vport.out[0x0083] = (t_faddrcc) io_write_0083;
	vport.out[0x0087] = (t_faddrcc) io_write_0087;
	vport.out[0x0089] = (t_faddrcc) io_write_0089;
	vport.out[0x008a] = (t_faddrcc) io_write_008A;
	vport.out[0x008b] = (t_faddrcc) io_write_008B;
	vport.out[0x008f] = (t_faddrcc) io_write_008F;

	vport.in[0x00c0] = (t_faddrcc) io_read_00C0;
	vport.in[0x00c2] = (t_faddrcc) io_read_00C2;
	vport.in[0x00c4] = (t_faddrcc) io_read_00C4;
	vport.in[0x00c6] = (t_faddrcc) io_read_00C6;
	vport.in[0x00c8] = (t_faddrcc) io_read_00C8;
	vport.in[0x00ca] = (t_faddrcc) io_read_00CA;
	vport.in[0x00cc] = (t_faddrcc) io_read_00CC;
	vport.in[0x00ce] = (t_faddrcc) io_read_00CE;
	vport.in[0x00d0] = (t_faddrcc) io_read_00D0;
	vport.in[0x00da] = (t_faddrcc) io_read_00DA;

	vport.out[0x00c0] = (t_faddrcc) io_write_00C0;
	vport.out[0x00c2] = (t_faddrcc) io_write_00C2;
	vport.out[0x00c4] = (t_faddrcc) io_write_00C4;
	vport.out[0x00c6] = (t_faddrcc) io_write_00C6;
	vport.out[0x00c8] = (t_faddrcc) io_write_00C8;
	vport.out[0x00ca] = (t_faddrcc) io_write_00CA;
	vport.out[0x00cc] = (t_faddrcc) io_write_00CC;
	vport.out[0x00ce] = (t_faddrcc) io_write_00CE;
	vport.out[0x00d0] = (t_faddrcc) io_write_00D0;
	vport.out[0x00d2] = (t_faddrcc) io_write_00D2;
	vport.out[0x00d4] = (t_faddrcc) io_write_00D4;
	vport.out[0x00d6] = (t_faddrcc) io_write_00D6;
	vport.out[0x00d8] = (t_faddrcc) io_write_00D8;
	vport.out[0x00dc] = (t_faddrcc) io_write_00DC;
	vport.out[0x00de] = (t_faddrcc) io_write_00DE;
    
    vbiosAddPost(VDMA_POST);
}
static void reset() {
	MEMSET(&vlatch, Zero8, sizeof(t_latch));
	doReset(&vdma1);
	doReset(&vdma2);
}
static void refresh() {
	t_nubit8 id;
	t_nubit8 realDRQ1, realDRQ2;
	if (GetBit(vdma2.command, VDMA_COMMAND_CTRL)) {
		return;
	}
	if (GetBit(vdma2.isr, VDMA_ISR_IS)) {
		if (VDMA_GetISR_ISR(vdma2.isr)) {
			Execute(&vdma2, VDMA_GetISR_ISR(vdma2.isr), True);
		} else if (GetBit(vdma1.isr, VDMA_ISR_IS)) {
			Execute(&vdma1, VDMA_GetISR_ISR(vdma1.isr), False);
		}
		if (!GetBit(vdma1.isr, VDMA_ISR_IS)) {
			vdma2.isr = Zero8;
		}
	}
	if (!GetBit(vdma2.isr, VDMA_ISR_IS)) {
		realDRQ2 = vdma2.request | (VDMA_GetSTATUS_DRQS(vdma2.status) & ~vdma2.mask);
		if (realDRQ2 == Zero8) {
			return;
		}
		id = GetRegTopId(&vdma2, realDRQ2);
		if (id == 0) {
			if (GetBit(vdma1.command, VDMA_COMMAND_CTRL)) {
				return;
			}
			realDRQ1 = vdma1.request | (VDMA_GetSTATUS_DRQS(vdma1.status) & ~vdma1.mask);
			if (realDRQ1 == Zero8) {
				return;
			}
			id = GetRegTopId(&vdma1, realDRQ1);
            VDMA_SetISR(vdma2.isr, 0);
            VDMA_SetISR(vdma1.isr, id);
			Execute(&vdma1, id, False);
			if (!GetBit(vdma1.isr, VDMA_ISR_IS)) {
				vdma2.isr = Zero8;
			}
			if (!VDMA_GetSTATUS_DRQS(vdma1.status)) {
                ClrBit(vdma2.status, VDMA_STATUS_DRQ(0));
			}
			if (!vdma1.request) {
                ClrBit(vdma2.request, VDMA_REQUEST_DRQ(0));
			}
		} else {
            VDMA_SetISR(vdma2.isr, id);
			Execute(&vdma2, id, True);
		}
	}
}
static void final() {}

void vdmaRegister() {vmachineAddMe;}

static void printDma(t_dma *rdma) {
	t_nubit8 i;
	PRINTF("Command = %x, status = %x, mask = %x\n",
	          rdma->command, rdma->status, rdma->mask);
	PRINTF("request = %x, temp = %x, flagMSB = %x\n",
	          rdma->request, rdma->temp, rdma->flagMSB);
	PRINTF("drx = %x, flagEOP = %x, isr = %x\n",
	          rdma->drx, rdma->flagEOP, rdma->isr);
	for (i = 0;i < VDMA_CHANNEL_COUNT;++i) {
		PRINTF("Channel %d: baseAddr = %x, baseCount = %x, currAddr = %x, currCount = %x\n",
		          i, rdma->channel[i].baseAddr, rdma->channel[i].baseCount,
		          rdma->channel[i].currAddr, rdma->channel[i].currCount);
		PRINTF("Channel %d: mode = %x, page = %x, fpReadDevice = %x, fpWriteDevice = %x\n",
		          i, rdma->channel[i].mode, rdma->channel[i].page,
		          rdma->channel[i].fpReadDevice, rdma->channel[i].fpWriteDevice);
	}
}

/* Print DMA status */
void devicePrintDma() {
	PRINTF("DMA 1 Info\n==========\n");
	printDma(&vdma1);
	PRINTF("\nDMA 2 Info\n==========\n");
	printDma(&vdma2);
}

/*
debug
FOR FDC READ/WRITE
off00 0  reset dma
o0b 86   mode: block, increase, write, channel 2
         other options: c6(cascade)  46(single) 06(demand)
o0b 8a   mode: block, increase, read, channel 2 
o04 00   write address to: channel 2 low
o04 00   write address to: channel 2 high
o05 00   write word count to: channel 2 low
o05 02   write word count to: channel 2 high
o0a 02   write mask: dma 1, channel 2
od4 00   write mask: dma 2, channel 0
off01 2  generate dreq; otherwise ASK DEVICE TO GENERATE DREQ
off02 0  refresh: exec
d0:0     show results

OTHER TEST CODES
off00 0
e6020 ab
o08 03
o0b 00
o03 5
o03 0
od4 0
o0a 0
o00 30
o00 60
o02 30
o02 80
o09 04
od2 04
off02 0
d6020
d8020
*/
