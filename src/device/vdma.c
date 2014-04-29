/* Copyright 2012-2014 Neko. */

/* VDMA implements two chips of Direct Memory Access Controller: Intel 8237A (Master+Slave). */

#include "vram.h"
#include "vfdc.h"

#include "vbios.h"
#include "vport.h"
#include "vmachine.h"
#include "vdma.h"

t_latch vlatch;
t_dma vdma1, vdma2;

static void doReset(t_dma *vdma) {
	t_nubit8 i;
	vdma->command = 0x00;
	vdma->status  = 0x00;
	vdma->mask    = 0x0f;
	vdma->request = 0x00;
	vdma->temp    = 0x00;
	vdma->flagmsb = 0;
	vdma->drx     = 0x00;
	vdma->flageop = 0;
	vdma->isr     = 0x00;
	for (i = 0;i < 4;++i) {
		vdma->channel[i].baseaddr = vdma->channel[i].curraddr = 0x0000;
		vdma->channel[i].basewc   = vdma->channel[i].currwc = 0x0000;
		vdma->channel[i].mode = 0x0000;
		vdma->channel[i].page = 0x00;
	}
}

static void io_read_CurrentAddress(t_dma *vdma, t_nubit8 id) {
	if (!vdma->flagmsb) {
		vport.iobyte = (t_nubit8)(vdma->channel[id].curraddr & 0x00ff);
	} else {
		vport.iobyte = (t_nubit8)(vdma->channel[id].curraddr >> 8);
	}
	vdma->flagmsb = !vdma->flagmsb;
}
static void io_read_CurrentWordCount(t_dma *vdma, t_nubit8 id) {
	if (!vdma->flagmsb) {
		vport.iobyte = (t_nubit8)(vdma->channel[id].currwc & 0x00ff);
	} else {
		vport.iobyte = (t_nubit8)(vdma->channel[id].currwc >> 8);
	}
	vdma->flagmsb = !vdma->flagmsb;
}
static void io_read_Status(t_dma *vdma) {
	vport.iobyte = vdma->status;
	vdma->status &= 0xf0;
}
#define     io_read_Temp(vdma) (vport.iobyte = (vdma)->temp)
#define     io_read_Page(vdma, id) (vport.iobyte = (vdma)->channel[(id)].page)

static void io_write_Address(t_dma *vdma, t_nubit8 id) {
	if (!vdma->flagmsb) {
		vdma->channel[id].baseaddr  = (t_nubit16)vport.iobyte;
	} else {
		vdma->channel[id].baseaddr |= (t_nubit16)(vport.iobyte << 8);
	}
	vdma->channel[id].curraddr = vdma->channel[id].baseaddr;
	vdma->flagmsb = !vdma->flagmsb;
}
static void io_write_WordCount(t_dma *vdma, t_nubit8 id) {
	if (!vdma->flagmsb) {
		vdma->channel[id].basewc  = (t_nubit16)vport.iobyte;
	} else {
		vdma->channel[id].basewc |= (t_nubit16)(vport.iobyte << 8);
	}
	vdma->channel[id].currwc = vdma->channel[id].basewc;
	vdma->flagmsb = !vdma->flagmsb;
}
#define     io_write_Command(vdma) ((vdma)->command = vport.iobyte)
static void io_write_Request(t_dma *vdma) {
	if (vport.iobyte & 0x04) {
		vdma->request |= 1 << (vport.iobyte & 0x03);
	} else {
		vdma->request &= ~(1 << (vport.iobyte & 0x03));
	}
}
static void io_write_Mask_Single(t_dma *vdma) {
	if (vport.iobyte & 0x04) {
		vdma->mask |= 1 << (vport.iobyte & 0x03);
	} else {
		vdma->mask &= ~(1 << (vport.iobyte & 0x03));
	}
}
#define     io_write_Mode(vdma) \
            ((vdma)->channel[vport.iobyte & 0x03].mode = vport.iobyte >> 2)
#define     io_write_Flipflop_Clear(vdma) ((vdma)->flagmsb = 0)
#define     io_write_Reset(vdma) (doReset(vdma))
#define     io_write_Mask_Clear(vdma) ((vdma)->mask = 0x00)
#define     io_write_Mask_All(vdma) ((vdma)->mask = vport.iobyte & 0x0f)
#define     io_write_Page(vdma, id, m) \
            ((vdma)->channel[(id)].page = vport.iobyte & (m))

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
static void io_write_0009() {io_write_Request         (&vdma1   );}
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

static void io_write_0081() {io_write_Page            (&vdma1, 2, 0xff);}
static void io_write_0082() {io_write_Page            (&vdma1, 3, 0xff);}
static void io_write_0083() {io_write_Page            (&vdma1, 1, 0xff);}
static void io_write_0087() {io_write_Page            (&vdma1, 0, 0xff);}
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
static void io_write_00D2() {io_write_Request         (&vdma2   );}
static void io_write_00D4() {io_write_Mask_Single     (&vdma2   );}
static void io_write_00D6() {io_write_Mode            (&vdma2   );}
static void io_write_00D8() {io_write_Flipflop_Clear  (&vdma2   );}
static void io_write_00DA() {io_write_Reset           (&vdma2   );}
static void io_write_00DC() {io_write_Mask_Clear      (&vdma2   );}
static void io_write_00DE() {io_write_Mask_All        (&vdma2   );}

static t_nubit8 GetRegTopId(t_dma *vdma, t_nubit8 reg) {
	t_nubit8 id = 0x00;
	if (reg == 0x00) {
		return 0x08;
	}
	reg = (reg << (0x04 - (vdma->drx))) | (reg >> (vdma->drx));
	while (!((reg >> id) & 0x01) && (id < 0x04)) {
		id++;
	}
	return (id + vdma->drx) % 0x04;
}
static void IncreaseCurrAddr(t_dma *vdma, t_nubit8 id) {
	vdma->channel[id].curraddr++;
	if (vdma->channel[id].curraddr == 0x0000) {
		vdma->channel[id].page++;
	}
}
static void DecreaseCurrAddr(t_dma *vdma, t_nubit8 id) {
	vdma->channel[id].curraddr--;
	if (vdma->channel[id].curraddr == 0xffff) {
		vdma->channel[id].page--;
	}
}
static void Transmission(t_dma *vdma, t_nubit8 id, t_bool flagword) {
	switch (VDMA_GetCS(vdma,id)) {
	case 0x00:
		/* verify */
		/* do nothing */
		vdma->channel[id].currwc--;
		if (VDMA_GetAIDS(vdma,id)) {
			DecreaseCurrAddr(vdma, id);
		} else {
			IncreaseCurrAddr(vdma, id);
		}
		break;
	case 0x01:
		/* write */
		if (vdma->channel[id].devread) {
			ExecFun(vdma->channel[id].devread);
		}
		if (!flagword) {
			vramByte((vdma->channel[id].page << 16) + vdma->channel[id].curraddr) = vlatch.byte;
		} else {
			vramWord((vdma->channel[id].page << 16) + (vdma->channel[id].curraddr << 1)) = vlatch.word;
		}
		vdma->channel[id].currwc--;
		if (VDMA_GetAIDS(vdma,id)) {
			DecreaseCurrAddr(vdma, id);
		} else {
			IncreaseCurrAddr(vdma, id);
		}
		break;
	case 0x02:
		/* read */
		if (!flagword) {
			vlatch.byte = vramByte((vdma->channel[id].page << 16) + vdma->channel[id].curraddr);
		} else {
			vlatch.word = vramWord((vdma->channel[id].page << 16) + (vdma->channel[id].curraddr << 1));
		}
		if (vdma->channel[id].devwrite) {
			ExecFun(vdma->channel[id].devwrite);
		}
		vdma->channel[id].currwc--;
		if (VDMA_GetAIDS(vdma,id)) {
			DecreaseCurrAddr(vdma, id);
		} else {
			IncreaseCurrAddr(vdma, id);
		}
		break;
	case 0x03:
		/* illegal */
		break;
	default:
		break;
	}
}
static void Execute(t_dma *vdma, t_nubit8 id, t_bool flagword) {
	t_bool flagm2m = ((id == 0x00) && (vdma->request & 0x01) && VDMA_GetM2M(vdma));
	vdma->status  &= ~(0x10 << id);
	vdma->request &= ~(0x01 << id);
	if (VDMA_GetR(vdma)) {
		vdma->drx = (id + 1) % 4;
	}
	if (flagm2m) {
		/* memory-to-memory */
		while (vdma->channel[0x01].currwc != 0xffff && !vdma->flageop) {
			vdma->temp = vramByte((vdma->channel[0].page << 16) + vdma->channel[0].curraddr);
			vramByte((vdma->channel[1].page << 16) + vdma->channel[1].curraddr) = vdma->temp;
			vdma->channel[0x01].currwc--;
			if (VDMA_GetAIDS(vdma,id)) {
				DecreaseCurrAddr(vdma, 0x01);
				if (!VDMA_GetC0AD(vdma)) {
					DecreaseCurrAddr(vdma, 0x00);
				}
			} else{
				IncreaseCurrAddr(vdma, 0x01);
				if (!VDMA_GetC0AD(vdma)) {
					IncreaseCurrAddr(vdma, 0x00);
				}
			}
		}
		if (vdma->channel[0x01].currwc == 0xffff) {
			vdma->status |= 0x01;
			vdma->flageop = 1;
		}
	} else {
		/* select mode and command */
		switch (VDMA_GetM(vdma,id)) {
		case 0x00:
			/* demand */
			while (vdma->channel[id].currwc != 0xffff && !vdma->flageop
				&& VDMA_GetDRQ(vdma,id)) {
				Transmission(vdma, id, flagword);
			}
			break;
		case 0x01:
			/* single */
			Transmission(vdma, id, flagword);
			break;
		case 0x02:
			/* block */
			while (vdma->channel[id].currwc != 0xffff && !vdma->flageop) {
				Transmission(vdma, id, flagword);
			}
			break;
		case 0x03:
			/* cascade */
			/* do nothing */
			vdma->flageop = 1;
			break;
		default:
			break;
		}
		if (vdma->channel[id].currwc == 0xffff) {
			vdma->status |= 0x01 << id; /* set terminate count */
			vdma->flageop = 1;
		}
	}
	if (vdma->flageop) {
		vdma->isr = 0x00;
		if (vdma->channel[id].devfinal) {
			ExecFun(vdma->channel[id].devfinal);
		}
		if (VDMA_GetAI(vdma,id)) {
			vdma->channel[id].curraddr = vdma->channel[id].baseaddr;
			vdma->channel[id].currwc = vdma->channel[id].basewc;
			vdma->mask &= ~(0x01 << id);
		} else {
			vdma->mask |= 0x01 << id;
		}
	}
	vdma->flageop = 0;
}

void vdmaSetDRQ(t_nubit8 dreqid) {
	switch (dreqid) {
	case 0x00: vdma1.status |= 0x10;break;
	case 0x01: vdma1.status |= 0x20;break;
	case 0x02: vdma1.status |= 0x40;break;
	case 0x03: vdma1.status |= 0x80;break;
/*	case 0x04: vdma2.status |= 0x10;break;*/
	case 0x05: vdma2.status |= 0x20;break;
	case 0x06: vdma2.status |= 0x40;break;
	case 0x07: vdma2.status |= 0x80;break;
	default: break;
	}
	if (vdma1.status >> 4) {
		vdma2.status |=  0x10;
	} else {
		vdma2.status &= ~0x10;
	}
}

static void init() {
	memset(&vdma1, 0x00, sizeof(t_dma));
	memset(&vdma2, 0x00, sizeof(t_dma));

	/* connect device io functions with dma channels */
	vdma1.channel[2].devread  = (t_faddrcc) vfdcDMARead;
	vdma1.channel[2].devwrite = (t_faddrcc) vfdcDMAWrite;
	vdma1.channel[2].devfinal = (t_faddrcc) vfdcDMAFinal;

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
	memset(&vlatch, 0x00, sizeof(t_latch));
	doReset(&vdma1);
	doReset(&vdma2);
}
static void refresh() {
	t_nubit8 id;
	t_nubit8 realdrq1, realdrq2;
	if (VDMA_GetCTRL(&vdma2)) {
		return;
	}
	if (VDMA_GetIS(&vdma2)) {
		if (VDMA_GetISR(&vdma2) != 0x00) {
			Execute(&vdma2, VDMA_GetISR(&vdma2), 0x01);
		} else if (VDMA_GetIS(&vdma1)) {
			Execute(&vdma1, VDMA_GetISR(&vdma1), 0x00);
		}
		if (!VDMA_GetIS(&vdma1)) {
			vdma2.isr = 0x00;
		}
	}
	if (!VDMA_GetIS(&vdma2)) {
		realdrq2 = vdma2.request | ((vdma2.status >> 4) & ~vdma2.mask);
		if (realdrq2 == 0x00) {
			return;
		}
		id = GetRegTopId(&vdma2, realdrq2);
		if (id == 0x00) {
			if (VDMA_GetCTRL(&vdma1)) {
				return;
			}
			realdrq1 = vdma1.request | ((vdma1.status >> 4) & ~vdma1.mask);
			if (realdrq1 == 0x00) {
				return;
			}
			id = GetRegTopId(&vdma1, realdrq1);
			vdma2.isr = 0x01;
			vdma1.isr = (id << 4) | 0x01;
			Execute(&vdma1, id, 0x00);
			if (!VDMA_GetIS(&vdma1)) {
				vdma2.isr = 0x00;
			}
			if (!(vdma1.status >> 4)) {
				vdma2.status &= ~0x10;
			}
			if (!vdma1.request) {
				vdma2.request &= ~0x01;
			}
		} else {
			vdma2.isr = (id << 4) | 0x01;
			Execute(&vdma2, id, 0x01);
		}
	}
}
static void final() {}

void vdmaRegister() {
	vmachine.deviceTable[VMACHINE_DEVICE_INIT][vmachine.numDevices] = (t_faddrcc) init;
	vmachine.deviceTable[VMACHINE_DEVICE_RESET][vmachine.numDevices] = (t_faddrcc) reset;
	vmachine.deviceTable[VMACHINE_DEVICE_REFRESH][vmachine.numDevices] = (t_faddrcc) refresh;
	vmachine.deviceTable[VMACHINE_DEVICE_FINAL][vmachine.numDevices] = (t_faddrcc) final;
	vmachine.numDevices++;
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
