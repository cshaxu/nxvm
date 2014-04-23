/* This file is a part of NXVM project. */

#include "memory.h"

#include "vapi.h"
#include "vport.h"
#include "vcpu.h"
#include "vram.h"
#include "vfdc.h"
#include "vdma.h"

t_latch vlatch;
t_dma vdma1,vdma2;

/* command register bits */
#define GetM2M(vdma)     (!!((vdma)->command & 0x01))
                                                  /* memory-to-memory enable */
#define GetC0AD(vdma)    (!!((vdma)->command & 0x02))
                                                   /* channel 0 address hold */
#define GetCTRL(vdma)    (!!((vdma)->command & 0x04))
                                                   /* dma controller disable */
#define GetTM(vdma)      (!!((vdma)->command & 0x08))
                                        /* normal(0) or compressed(1) timing */
#define GetR(vdma)       (!!((vdma)->command & 0x10))
                                        /* normal(0) or rotating(1) priority */
#define GetWS(vdma)      (!!((vdma)->command & 0x20))
                                   /* late(0) or extended(1) write selection */
#define GetDREQSA(vdma)  (!!((vdma)->command & 0x40))
                                      /* dreq sense active high(0) or low(1) */
#define GetDACKSA(vdma)  (!!((vdma)->command & 0x80))
                                      /* dack sense active low(0) or high(1) */
/* mode register bits */
#define GetCS(vdma,id)   (((vdma)->channel[(id)].mode & 0x03))
                           /* verify(0) or write(1) or read(2) or illegal(3) */
#define GetAI(vdma,id)   (!!((vdma)->channel[(id)].mode & 0x04))
                                                /* autoinitialization enable */
#define GetAIDS(vdma,id) (!!((vdma)->channel[(id)].mode & 0x08))
                              /* address increment(0) or decrement(1) select */
#define GetM(vdma,id)    (((vdma)->channel[(id)].mode & 0x30) >> 4)
             /* demand(0) or single(1) or block(2) or cascade(3) mode select */
/* request register bits */
#define GetREQ(vdma,id)  (!!((vdma)->request & (0x01 << (id))))
/* mask register bits */
#define GetMASK(vdma,id) (!!((vdma)->mask    & (0x01 << (id))))
/* status register bits */
#define GetTC(vdma,id)   (!!((vdma)->status  & (0x01 << (id))))
#define GetDRQ(vdma,id)  (!!((vdma)->status  & (0x10 << (id))))
/* in service bits */
#define GetIS(vdma)      ((vdma)->isr & 0x01)
#define GetISR(vdma)     ((vdma)->isr >> 4)

static void IO_Read_CurrentAddress(t_dma *vdma, t_nubitcc id)
{
	if(!vdma->flagmsb)
		vport.iobyte = (t_nubit8)(vdma->channel[id].curraddr & 0x00ff);
	else
		vport.iobyte = (t_nubit8)(vdma->channel[id].curraddr >> 8);
	vdma->flagmsb = 0x01 - vdma->flagmsb;
}
static void IO_Read_CurrentWordCount(t_dma *vdma, t_nubitcc id)
{
	if(!vdma->flagmsb)
		vport.iobyte = (t_nubit8)(vdma->channel[id].currwc & 0x00ff);
	else
		vport.iobyte = (t_nubit8)(vdma->channel[id].currwc >> 8);
	vdma->flagmsb = 0x01 - vdma->flagmsb;
}
static void IO_Read_Status(t_dma *vdma)
{
	vport.iobyte = vdma->status;
	vdma->status &= 0xf0;
}
#define     IO_Read_Temp(vdma) (vport.iobyte = (vdma)->temp)
#define     IO_Read_Page(vdma, id) (vport.iobyte = (vdma)->channel[(id)].page)

static void IO_Write_Address(t_dma *vdma, t_nubitcc id)
{
	if(!vdma->flagmsb)
		vdma->channel[id].baseaddr  = (t_nubit16)vport.iobyte;
	else
		vdma->channel[id].baseaddr |= (t_nubit16)(vport.iobyte << 8);
	vdma->channel[id].curraddr = vdma->channel[id].baseaddr;
	vdma->flagmsb = 0x01 - vdma->flagmsb;
}
static void IO_Write_WordCount(t_dma *vdma, t_nubitcc id)
{
	if(!vdma->flagmsb)
		vdma->channel[id].basewc  = (t_nubit16)vport.iobyte;
	else
		vdma->channel[id].basewc |= (t_nubit16)(vport.iobyte << 8);
	vdma->channel[id].currwc = vdma->channel[id].basewc;
	vdma->flagmsb = 0x01 - vdma->flagmsb;
}
#define     IO_Write_Command(vdma) ((vdma)->command = vport.iobyte)
static void IO_Write_Request(t_dma *vdma)
{
	if (vport.iobyte & 0x04)
		vdma->request |= 1 << (vport.iobyte & 0x03);
	else
		vdma->request &= ~(1 << (vport.iobyte & 0x03));
}
static void IO_Write_Mask_Single(t_dma *vdma)
{
	if(vport.iobyte & 0x04)
		vdma->mask |= 1 << (vport.iobyte & 0x03);
	else
		vdma->mask &= ~(1 << (vport.iobyte & 0x03));
}
#define     IO_Write_Mode(vdma) \
            ((vdma)->channel[vport.iobyte & 0x03].mode = vport.iobyte >> 2)
#define     IO_Write_Flipflop_Clear(vdma) ((vdma)->flagmsb = 0x00)
#define     IO_Write_Reset(vdma) (vdmaReset(vdma))
#define     IO_Write_Mask_Clear(vdma) ((vdma)->mask = 0x00)
#define     IO_Write_Mask_All(vdma) ((vdma)->mask = vport.iobyte & 0x0f)
#define     IO_Write_Page(vdma, id, m) \
            ((vdma)->channel[(id)].page = vport.iobyte & (m))

void IO_Read_0000()  {IO_Read_CurrentAddress   (&vdma1, 0);}
void IO_Read_0001()  {IO_Read_CurrentWordCount (&vdma1, 0);}
void IO_Read_0002()  {IO_Read_CurrentAddress   (&vdma1, 1);}
void IO_Read_0003()  {IO_Read_CurrentWordCount (&vdma1, 1);}
void IO_Read_0004()  {IO_Read_CurrentAddress   (&vdma1, 2);}
void IO_Read_0005()  {IO_Read_CurrentWordCount (&vdma1, 2);}
void IO_Read_0006()  {IO_Read_CurrentAddress   (&vdma1, 3);}
void IO_Read_0007()  {IO_Read_CurrentWordCount (&vdma1, 3);}
void IO_Read_0008()  {IO_Read_Status           (&vdma1   );}
void IO_Read_000D()  {IO_Read_Temp             (&vdma1   );}

void IO_Write_0000() {IO_Write_Address         (&vdma1, 0);}
void IO_Write_0001() {IO_Write_WordCount       (&vdma1, 0);}
void IO_Write_0002() {IO_Write_Address         (&vdma1, 1);}
void IO_Write_0003() {IO_Write_WordCount       (&vdma1, 1);}
void IO_Write_0004() {IO_Write_Address         (&vdma1, 2);}
void IO_Write_0005() {IO_Write_WordCount       (&vdma1, 2);}
void IO_Write_0006() {IO_Write_Address         (&vdma1, 3);}
void IO_Write_0007() {IO_Write_WordCount       (&vdma1, 3);}
void IO_Write_0008() {IO_Write_Command         (&vdma1   );}
void IO_Write_0009() {IO_Write_Request         (&vdma1   );}
void IO_Write_000A() {IO_Write_Mask_Single     (&vdma1   );}
void IO_Write_000B() {IO_Write_Mode            (&vdma1   );}
void IO_Write_000C() {IO_Write_Flipflop_Clear  (&vdma1   );}
void IO_Write_000D() {IO_Write_Reset           (&vdma1   );}
void IO_Write_000E() {IO_Write_Mask_Clear      (&vdma1   );}
void IO_Write_000F() {IO_Write_Mask_All        (&vdma1   );}

void IO_Read_0081()  {IO_Read_Page             (&vdma1, 2);}
void IO_Read_0082()  {IO_Read_Page             (&vdma1, 3);}
void IO_Read_0083()  {IO_Read_Page             (&vdma1, 1);}
void IO_Read_0087()  {IO_Read_Page             (&vdma1, 0);}
void IO_Read_0089()  {IO_Read_Page             (&vdma2, 2);}
void IO_Read_008A()  {IO_Read_Page             (&vdma2, 3);}
void IO_Read_008B()  {IO_Read_Page             (&vdma2, 1);}
void IO_Read_008F()  {IO_Read_Page             (&vdma2, 0);}

void IO_Write_0081() {IO_Write_Page            (&vdma1, 2, 0xff);}
void IO_Write_0082() {IO_Write_Page            (&vdma1, 3, 0xff);}
void IO_Write_0083() {IO_Write_Page            (&vdma1, 1, 0xff);}
void IO_Write_0087() {IO_Write_Page            (&vdma1, 0, 0xff);}
void IO_Write_0089() {IO_Write_Page            (&vdma2, 2, 0xfe);}
void IO_Write_008A() {IO_Write_Page            (&vdma2, 3, 0xfe);}
void IO_Write_008B() {IO_Write_Page            (&vdma2, 1, 0xfe);}
void IO_Write_008F() {IO_Write_Page            (&vdma2, 0, 0xfe);}

void IO_Read_00C0()  {IO_Read_CurrentAddress   (&vdma2, 0);}
void IO_Read_00C2()  {IO_Read_CurrentWordCount (&vdma2, 0);}
void IO_Read_00C4()  {IO_Read_CurrentAddress   (&vdma2, 1);}
void IO_Read_00C6()  {IO_Read_CurrentWordCount (&vdma2, 1);}
void IO_Read_00C8()  {IO_Read_CurrentAddress   (&vdma2, 2);}
void IO_Read_00CA()  {IO_Read_CurrentWordCount (&vdma2, 2);}
void IO_Read_00CC()  {IO_Read_CurrentAddress   (&vdma2, 3);}
void IO_Read_00CE()  {IO_Read_CurrentWordCount (&vdma2, 3);}
void IO_Read_00D0()  {IO_Read_Status           (&vdma2   );}
void IO_Read_00DA()  {IO_Read_Temp             (&vdma2   );}

void IO_Write_00C0() {IO_Write_Address         (&vdma2, 0);}
void IO_Write_00C2() {IO_Write_WordCount       (&vdma2, 0);}
void IO_Write_00C4() {IO_Write_Address         (&vdma2, 1);}
void IO_Write_00C6() {IO_Write_WordCount       (&vdma2, 1);}
void IO_Write_00C8() {IO_Write_Address         (&vdma2, 2);}
void IO_Write_00CA() {IO_Write_WordCount       (&vdma2, 2);}
void IO_Write_00CC() {IO_Write_Address         (&vdma2, 3);}
void IO_Write_00CE() {IO_Write_WordCount       (&vdma2, 3);}
void IO_Write_00D0() {IO_Write_Command         (&vdma2   );}
void IO_Write_00D2() {IO_Write_Request         (&vdma2   );}
void IO_Write_00D4() {IO_Write_Mask_Single     (&vdma2   );}
void IO_Write_00D6() {IO_Write_Mode            (&vdma2   );}
void IO_Write_00D8() {IO_Write_Flipflop_Clear  (&vdma2   );}
void IO_Write_00DA() {IO_Write_Reset           (&vdma2   );}
void IO_Write_00DC() {IO_Write_Mask_Clear      (&vdma2   );}
void IO_Write_00DE() {IO_Write_Mask_All        (&vdma2   );}

static t_nubit8 GetRegTopId(t_dma *vdma, t_nubit8 reg)
{
	t_nubit8 id = 0x00;
	if (reg == 0x00) return 0x08;
	reg = (reg << (0x04 - (vdma->drx))) | (reg >> (vdma->drx));
	while (!((reg >> id) & 0x01) && (id < 0x04)) id++;
	return (id + vdma->drx) % 0x04;
}
static void IncreaseCurrAddr(t_dma *vdma, t_nubit8 id)
{
	vdma->channel[id].curraddr++;
	if (vdma->channel[id].curraddr == 0x0000) vdma->channel[id].page++;
}
static void DecreaseCurrAddr(t_dma *vdma, t_nubit8 id)
{
	vdma->channel[id].curraddr--;
	if (vdma->channel[id].curraddr == 0xffff) vdma->channel[id].page--;
}
static void Transmission(t_dma *vdma, t_nubit8 id, t_bool word)
{
	switch (GetCS(vdma,id)) {
	case 0x00: /* verify */
		/* do nothing */
		vdma->channel[id].currwc--;
		if (GetAIDS(vdma,id)) DecreaseCurrAddr(vdma, id);
		else                  IncreaseCurrAddr(vdma, id);
		break;
	case 0x01:                                                      /* write */
		if (vdma->channel[id].devread) ExecFun(vdma->channel[id].devread);
		if (!word)
			vramVarByte(((t_nubit16)vdma->channel[id].page << 12),
			            vdma->channel[id].curraddr) = vlatch.byte;
		else
			vramVarWord(((t_nubit16)vdma->channel[id].page << 12),
			            vdma->channel[id].curraddr << 1) = vlatch.word;
		vdma->channel[id].currwc--;
		if (GetAIDS(vdma,id)) DecreaseCurrAddr(vdma, id);
		else                  IncreaseCurrAddr(vdma, id);
		break;
	case 0x02:                                                       /* read */
		if (!word)
			vlatch.byte = vramVarByte(
			                 ((t_nubit16)vdma->channel[id].page << 12),
			                 vdma->channel[id].curraddr);
		else
			vlatch.word = vramVarWord(
			                 ((t_nubit16)vdma->channel[id].page << 12),
			                 (vdma->channel[id].curraddr << 1));
		if (vdma->channel[id].devwrite) ExecFun(vdma->channel[id].devwrite);
		vdma->channel[id].currwc--;
		if (GetAIDS(vdma,id)) DecreaseCurrAddr(vdma, id);
		else                  IncreaseCurrAddr(vdma, id);
		break;
	case 0x03:                                                    /* illegal */
		break;
	default:
		break;
	}
}
static void Execute(t_dma *vdma, t_nubit8 id, t_bool word)
{
	t_bool flagm2m = ((id == 0x00) && (vdma->request & 0x01) && GetM2M(vdma));
	vdma->status  &= ~(0x10 << id);
	vdma->request &= ~(0x01 << id);
	if (GetR(vdma)) vdma->drx = (id + 1) % 4;
	if (flagm2m) {
		/* memory-to-memory */
		while (vdma->channel[0x01].currwc != 0xffff && !vdma->flageop) {
			vdma->temp = vramVarByte(
			             ((t_nubit16)vdma->channel[0x00].page << 16),
			             vdma->channel[0x00].curraddr);
			vramVarByte(((t_nubit16)vdma->channel[0x01].page << 16),
			            vdma->channel[0x01].curraddr) = vdma->temp;
			vdma->channel[0x01].currwc--;
			if (GetAIDS(vdma,id)) {
				DecreaseCurrAddr(vdma, 0x01);
				if (!GetC0AD(vdma))
					DecreaseCurrAddr(vdma, 0x00);
			} else{
				IncreaseCurrAddr(vdma, 0x01);
				if (!GetC0AD(vdma))
					IncreaseCurrAddr(vdma, 0x00);
			}
		}
		if (vdma->channel[0x01].currwc == 0xffff) {
			vdma->status |= 0x01;
			vdma->flageop = 0x01;
		}
	} else {
		switch (GetM(vdma,id)) {                  /* select mode and command */
		case 0x00:                                                 /* demand */
			while (vdma->channel[id].currwc != 0xffff && !vdma->flageop
					&& GetDRQ(vdma,id))
				Transmission(vdma, id, word);
			break;
		case 0x01:                                                 /* single */
			Transmission(vdma, id, word);
			break;
		case 0x02:                                                  /* block */
			while (vdma->channel[id].currwc != 0xffff && !vdma->flageop)
				Transmission(vdma, id, word);
			break;
		case 0x03:                                                /* cascade */
			/* do nothing */
			vdma->flageop = 0x01;
			break;
		default:
			break;
		}
		if (vdma->channel[id].currwc == 0xffff) {
			vdma->status |= 0x01 << id;                   /* set terminate count */
			vdma->flageop     = 0x01;
		}
	}
	if (vdma->flageop) {
		vdma->isr = 0x00;
		if (vdma->channel[id].devfinal) ExecFun(vdma->channel[id].devfinal);
		if (GetAI(vdma,id)) {
			vdma->channel[id].curraddr = vdma->channel[id].baseaddr;
			vdma->channel[id].currwc = vdma->channel[id].basewc;
			vdma->mask &= ~(0x01 << id);
		} else
			vdma->mask |= 0x01 << id;
	}
	vdma->flageop = 0x00;
}

void vdmaSetDRQ(t_nubit8 dreqid)
{
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
	if (vdma1.status >> 4) vdma2.status |=  0x10;
	else                   vdma2.status &= ~0x10;
}
void vdmaReset(t_dma *vdma)
{
	vdma->command = 0x00;
	vdma->status  = 0x00;
	vdma->mask    = 0x0f;
	vdma->request = 0x00;
	vdma->temp    = 0x00;
	vdma->flagmsb = 0x00;
	vdma->drx     = 0x00;
	vdma->flageop = 0x00;
	vdma->isr     = 0x00;
}

void vdmaRefresh()
{
	t_nubit8 id;
	t_nubit8 realdrq1, realdrq2;
	if (GetCTRL(&vdma2)) return;
	if (GetIS(&vdma2)) {
		if (GetISR(&vdma2) != 0x00) Execute(&vdma2, GetISR(&vdma2), 0x01);
		else if (GetIS(&vdma1))     Execute(&vdma1, GetISR(&vdma1), 0x00);
		if (!GetIS(&vdma1)) vdma2.isr = 0x00;
	}
	if (!GetIS(&vdma2)) {
		realdrq2 = vdma2.request | ((vdma2.status >> 4) & ~vdma2.mask);
		if (realdrq2 == 0x00) return;
		id = GetRegTopId(&vdma2, realdrq2);
		if (id == 0x00) {
			if (GetCTRL(&vdma1)) return;
			realdrq1 = vdma1.request | ((vdma1.status >> 4) & ~vdma1.mask);
			if (realdrq1 == 0x00) return;
			id = GetRegTopId(&vdma1, realdrq1);
			vdma2.isr = 0x01;
			vdma1.isr = (id << 4) | 0x01;
			Execute(&vdma1, id, 0x00);
			if (!GetIS(&vdma1)) vdma2.isr = 0x00;
			if (!(vdma1.status >> 4)) vdma2.status &= ~0x10;
			if (!vdma1.request) vdma2.request &= ~0x01;
		} else {
			vdma2.isr = (id << 4) | 0x01;
			Execute(&vdma2, id, 0x01);
		}
	}
	//vapiPrint("transmitted! es=%x, bx=%x\n",_es,_bx);
}
#ifdef VDMA_DEBUG
void IO_Read_FF00() /* print all info of dma */
{
	t_nubitcc i;
	vport.iobyte = 0xff;
	vapiPrint("DMA 1 Info\n==========\n");
	vapiPrint("Command = %x, status = %x, mask = %x\n",
	          vdma1.command, vdma1.status, vdma1.mask);
	vapiPrint("request = %x, temp = %x, flagmsb = %x\n",
	          vdma1.request, vdma1.temp, vdma1.flagmsb);
	vapiPrint("drx = %x, flageop = %x, isr = %x\n",
	          vdma1.drx, vdma1.flageop, vdma1.isr);
	for (i = 0;i < 4;++i) {
		vapiPrint("Channel %d: baseaddr = %x, basewc = %x, curraddr = %x, currwc = %x\n",
		          i, vdma1.channel[i].baseaddr, vdma1.channel[i].basewc,
		          vdma1.channel[i].curraddr, vdma1.channel[i].currwc);
		vapiPrint("Channel %d: mode = %x, page = %x, devread = %x, devwrite = %x\n",
		          i, vdma1.channel[i].mode, vdma1.channel[i].page,
		          vdma1.channel[i].devread, vdma1.channel[i].devwrite);
	}
	vapiPrint("\nDMA 2 Info\n==========\n");
	vapiPrint("Command = %x, status = %x, mask = %x\n",
	          vdma2.command, vdma2.status, vdma2.mask);
	vapiPrint("request = %x, temp = %x, flagmsb = %x\n",
	          vdma2.request, vdma2.temp, vdma2.flagmsb);
	vapiPrint("drx = %x, flageop = %x, isr = %x\n",
	          vdma2.drx, vdma2.flageop, vdma2.isr);
	for (i = 0;i < 4;++i) {
		vapiPrint("Channel %d: baseaddr = %x, basewc = %x, curraddr = %x, currwc = %x\n",
		          i, vdma2.channel[i].baseaddr, vdma2.channel[i].basewc,
		          vdma2.channel[i].curraddr, vdma2.channel[i].currwc);
		vapiPrint("Channel %d: mode = %x, page = %x, devread = %x, devwrite = %x\n",
		          i, vdma2.channel[i].mode, vdma2.channel[i].page,
		          vdma2.channel[i].devread, vdma2.channel[i].devwrite);
	}
	vapiPrint("\nLatch: byte = %x, word = %x\n", vlatch.byte, vlatch.word);
}
void IO_Write_FF00() {vdmaReset(&vdma1);vdmaReset(&vdma2);}
void IO_Write_FF01() {vdmaSetDRQ(vport.iobyte);}
void IO_Write_FF02() {vdmaRefresh();}
#define mov(n) (vport.iobyte=(n))
#define out(n) ExecFun(vport.out[(n)])
#endif
void vdmaInit()
{
	memset(&vdma1, 0x00, sizeof(t_dma));
	memset(&vdma2, 0x00, sizeof(t_dma));
	memset(&vlatch, 0x00, sizeof(t_latch));
	vdmaReset(&vdma1);
	vdmaReset(&vdma2);

	/* connect device io functions with dma channels */
	vdma1.channel[2].devread  = (t_faddrcc)vfdcDMARead;
	vdma1.channel[2].devwrite = (t_faddrcc)vfdcDMAWrite;
	vdma1.channel[2].devfinal = (t_faddrcc)vfdcDMAFinal;

	vport.in[0x0000] = (t_faddrcc)IO_Read_0000;
	vport.in[0x0001] = (t_faddrcc)IO_Read_0001;
	vport.in[0x0002] = (t_faddrcc)IO_Read_0002;
	vport.in[0x0003] = (t_faddrcc)IO_Read_0003;
	vport.in[0x0004] = (t_faddrcc)IO_Read_0004;
	vport.in[0x0005] = (t_faddrcc)IO_Read_0005;
	vport.in[0x0006] = (t_faddrcc)IO_Read_0006;
	vport.in[0x0007] = (t_faddrcc)IO_Read_0007;
	vport.in[0x0008] = (t_faddrcc)IO_Read_0008;
	vport.in[0x000d] = (t_faddrcc)IO_Read_000D;

	vport.out[0x0000] = (t_faddrcc)IO_Write_0000;
	vport.out[0x0001] = (t_faddrcc)IO_Write_0001;
	vport.out[0x0002] = (t_faddrcc)IO_Write_0002;
	vport.out[0x0003] = (t_faddrcc)IO_Write_0003;
	vport.out[0x0004] = (t_faddrcc)IO_Write_0004;
	vport.out[0x0005] = (t_faddrcc)IO_Write_0005;
	vport.out[0x0006] = (t_faddrcc)IO_Write_0006;
	vport.out[0x0007] = (t_faddrcc)IO_Write_0007;
	vport.out[0x0008] = (t_faddrcc)IO_Write_0008;
	vport.out[0x0009] = (t_faddrcc)IO_Write_0009;
	vport.out[0x000a] = (t_faddrcc)IO_Write_000A;
	vport.out[0x000b] = (t_faddrcc)IO_Write_000B;
	vport.out[0x000c] = (t_faddrcc)IO_Write_000C;
	vport.out[0x000d] = (t_faddrcc)IO_Write_000D;
	vport.out[0x000e] = (t_faddrcc)IO_Write_000E;
	vport.out[0x000f] = (t_faddrcc)IO_Write_000F;

	vport.in[0x0081] = (t_faddrcc)IO_Read_0081;
	vport.in[0x0082] = (t_faddrcc)IO_Read_0082;
	vport.in[0x0083] = (t_faddrcc)IO_Read_0083;
	vport.in[0x0087] = (t_faddrcc)IO_Read_0087;
	vport.in[0x0089] = (t_faddrcc)IO_Read_0089;
	vport.in[0x008a] = (t_faddrcc)IO_Read_008A;
	vport.in[0x008b] = (t_faddrcc)IO_Read_008B;
	vport.in[0x008f] = (t_faddrcc)IO_Read_008F;

	vport.out[0x0081] = (t_faddrcc)IO_Write_0081;
	vport.out[0x0082] = (t_faddrcc)IO_Write_0082;
	vport.out[0x0083] = (t_faddrcc)IO_Write_0083;
	vport.out[0x0087] = (t_faddrcc)IO_Write_0087;
	vport.out[0x0089] = (t_faddrcc)IO_Write_0089;
	vport.out[0x008a] = (t_faddrcc)IO_Write_008A;
	vport.out[0x008b] = (t_faddrcc)IO_Write_008B;
	vport.out[0x008f] = (t_faddrcc)IO_Write_008F;

	vport.in[0x00c0] = (t_faddrcc)IO_Read_00C0;
	vport.in[0x00c2] = (t_faddrcc)IO_Read_00C2;
	vport.in[0x00c4] = (t_faddrcc)IO_Read_00C4;
	vport.in[0x00c6] = (t_faddrcc)IO_Read_00C6;
	vport.in[0x00c8] = (t_faddrcc)IO_Read_00C8;
	vport.in[0x00ca] = (t_faddrcc)IO_Read_00CA;
	vport.in[0x00cc] = (t_faddrcc)IO_Read_00CC;
	vport.in[0x00ce] = (t_faddrcc)IO_Read_00CE;
	vport.in[0x00d0] = (t_faddrcc)IO_Read_00D0;
	vport.in[0x00da] = (t_faddrcc)IO_Read_00DA;

	vport.out[0x00c0] = (t_faddrcc)IO_Write_00C0;
	vport.out[0x00c2] = (t_faddrcc)IO_Write_00C2;
	vport.out[0x00c4] = (t_faddrcc)IO_Write_00C4;
	vport.out[0x00c6] = (t_faddrcc)IO_Write_00C6;
	vport.out[0x00c8] = (t_faddrcc)IO_Write_00C8;
	vport.out[0x00ca] = (t_faddrcc)IO_Write_00CA;
	vport.out[0x00cc] = (t_faddrcc)IO_Write_00CC;
	vport.out[0x00ce] = (t_faddrcc)IO_Write_00CE;
	vport.out[0x00d0] = (t_faddrcc)IO_Write_00D0;
	vport.out[0x00d2] = (t_faddrcc)IO_Write_00D2;
	vport.out[0x00d4] = (t_faddrcc)IO_Write_00D4;
	vport.out[0x00d6] = (t_faddrcc)IO_Write_00D6;
	vport.out[0x00d8] = (t_faddrcc)IO_Write_00D8;
	vport.out[0x00dc] = (t_faddrcc)IO_Write_00DC;
	vport.out[0x00de] = (t_faddrcc)IO_Write_00DE;
#ifdef VDMA_DEBUG
	vport.in[0xff00] = (t_faddrcc)IO_Read_FF00;
	vport.out[0xff00] = (t_faddrcc)IO_Write_FF00;
	vport.out[0xff01] = (t_faddrcc)IO_Write_FF01;
	vport.out[0xff02] = (t_faddrcc)IO_Write_FF02;
	mov(0x00);
	out(0x08);
	out(0xd0);
	mov(0xc0);
	out(0xd6);
#endif
}
void vdmaFinal() {}
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
