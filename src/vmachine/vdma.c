/* This file is a part of NXVM project. */

// channel is not completely implemented:
// it is only used to store the memory address and word count.

#include "memory.h"

#include "system/vapi.h"
#include "vcpu.h"
#include "vcpuins.h"
#include "vram.h"
#include "vdma.h"

t_dma_latch vdmalatch;
t_dma vdma1,vdma2;

/* command register bits */
#define GetM2M(vdma)     (!!((vdma)->command & 0x01))
                                                  /* memory-to-memory enable */
#define GetC0AD(vdma)    (!!((vdma)->command & 0x02))
                                                   /* channel 0 address hold */
#define GetCTRL(vdma)    (!!((vdma)->command & 0x04))
                                                    /* dma controller enable */
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
/* actual request */
#define GetRealREQ(vdma,id) \
                         (!!(((vdma)->request & ~((vdma)->mask)) \
                          & (0x01 << (id))))
/* status register bits */
#define GetTC(vdma,id)   (!!((vdma)->status  & (0x01 << (id))))
#define GetRQ(vdma,id)   (!!((vdma)->status  & (0x10 << (id))))

static void IO_Read_CurrentAddress(t_dma *vdma, t_nubitcc id)
{
	if(!vdma->flagmsb)
		vcpu.al = (t_nubit8)(vdma->channel[id].curraddr & 0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma->channel[id].curraddr >> 8);
	vdma->flagmsb = 0x01 - vdma->flagmsb;
}
static void IO_Read_CurrentWordCount(t_dma *vdma, t_nubitcc id)
{
	if(!vdma->flagmsb)
		vcpu.al = (t_nubit8)(vdma->channel[id].currwc & 0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma->channel[id].currwc >> 8);
	vdma->flagmsb = 0x01 - vdma->flagmsb;
}
static void IO_Read_Status(t_dma *vdma)
{
	vdma->status = (vdma->request << 4) | (vdma->status & 0x0f);
	vcpu.al = vdma->status;
	vdma->status = 0;
}
#define     IO_Read_Temp(vdma) (vcpu.al = (vdma)->temp)
#define     IO_Read_Page(vdma, id) (vcpu.al = (vdma)->channel[id].page)

static void IO_Write_Address(t_dma *vdma, t_nubitcc id)
{
	if(!vdma->flagmsb)
		vdma->channel[id].baseaddr = (t_nubit16)vcpu.al;
	else
		vdma->channel[id].baseaddr |= (t_nubit16)(vcpu.al << 8);
	vdma->channel[id].curraddr = vdma->channel[id].baseaddr;
	vdma->flagmsb = 0x01 - vdma->flagmsb;
}
static void IO_Write_WordCount(t_dma *vdma, t_nubitcc id)
{
	if(!vdma->flagmsb)
		vdma->channel[id].basewc = (t_nubit16)vcpu.al;
	else
		vdma->channel[id].basewc |= (t_nubit16)(vcpu.al << 8);
	vdma->channel[id].currwc = vdma->channel[id].basewc;
	vdma->flagmsb = 0x01 - vdma->flagmsb;
}
#define     IO_Write_Command(vdma) ((vdma)->command = vcpu.al)
static void IO_Write_Request(t_dma *vdma)
{
	if (vcpu.al & 0x04)
		vdma->request |= 1 << (vcpu.al & 0x03);
	else
		vdma->request &= ~(1 << (vcpu.al & 0x03));
}
static void IO_Write_Mask_Single(t_dma *vdma)
{
	if(vcpu.al & 0x04)
		vdma->mask |= 1 << (vcpu.al & 0x03);
	else
		vdma->mask &= ~(1 << (vcpu.al & 0x03));
}
#define     IO_Write_Mode(vdma) \
             ((vdma)->channel[vcpu.al & 0x03].mode = vcpu.al >> 2)
#define     IO_Write_Flipflop_Clear(vdma) ((vdma)->flagmsb = 0x00)
#define     IO_Write_Reset(vdma) (vdmaReset(vdma))
#define     IO_Write_Mask_Clear(vdma) ((vdma)->mask = 0x00)
#define     IO_Write_Mask_All(vdma) ((vdma)->mask = vcpu.al & 0x0f)
#define     IO_Write_Page(vdma, id)((vdma)->channel[id].page = vcpu.al)

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

void IO_Write_0081() {IO_Write_Page            (&vdma1, 2);}
void IO_Write_0082() {IO_Write_Page            (&vdma1, 3);}
void IO_Write_0083() {IO_Write_Page            (&vdma1, 1);}
void IO_Write_0087() {IO_Write_Page            (&vdma1, 0);}
void IO_Write_0089() {IO_Write_Page            (&vdma2, 2);}
void IO_Write_008A() {IO_Write_Page            (&vdma2, 3);}
void IO_Write_008B() {IO_Write_Page            (&vdma2, 1);}
void IO_Write_008F() {IO_Write_Page            (&vdma2, 0);}

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

static void Transmission(t_dma *vdma, t_nubit8 id)
{
	switch (GetCS(vdma,id)) {
	case 0x00: /* verify */
		/* do nothing */
		vdma->channel[id].currwc--;
		if (GetAIDS(vdma,id)) vdma->channel[id].curraddr--;
		else                  vdma->channel[id].curraddr++;
		break;
	case 0x01:                                                      /* write */
		if (vdma->channel[id].devread) FUNEXEC(vdma->channel[id].devread);
		vramSetByte(((t_nubit16)vdma->channel[id].page << 12),
		            vdma->channel[id].curraddr, vdmalatch.byte);
		vdma->channel[id].currwc--;
		if (GetAIDS(vdma,id)) vdma->channel[id].curraddr--;
		else                  vdma->channel[id].curraddr++;
		break;
	case 0x02:                                                       /* read */
		vdmalatch.byte = vramGetByte(((t_nubit16)vdma->channel[id].page << 12),
		                             vdma->channel[id].curraddr);
		if (vdma->channel[id].devwrite) FUNEXEC(vdma->channel[id].devwrite);
		vdma->channel[id].currwc--;
		if (GetAIDS(vdma,id)) vdma->channel[id].curraddr--;
		else                  vdma->channel[id].curraddr++;
		break;
	case 0x03:                                                    /* illegal */
		break;
	default:
		break;
	}
}

void vdmaSetDRQ(t_nubit8 dreqid)
{
	switch (dreqid) {
	case 0x00: vdma1.request |= 0x01;vdma2.request |= 0x01;break;
	case 0x01: vdma1.request |= 0x02;vdma2.request |= 0x01;break;
	case 0x02: vdma1.request |= 0x04;vdma2.request |= 0x01;break;
	case 0x03: vdma1.request |= 0x08;vdma2.request |= 0x01;break;
/*	case 0x04: vdma2.request |= 0x01;break; */
	case 0x05: vdma2.request |= 0x02;break;
	case 0x06: vdma2.request |= 0x04;break;
	case 0x07: vdma2.request |= 0x08;break;
	default:   break;
	}
}
void vdmaReset(t_dma *vdma)
{
	memset(vdma, 0x00, sizeof(t_dma));
	vdma->mask = 0x0f;
}

void vdmaRefresh()
{
	/* TODO: Do whatever necessary for DMA transmission */
	t_nubit8 id;
	t_nubit8 realdrq1, realdrq2;
	/* get dma requests */
	if (vdma1.request) vdma2.request |= 0x01;
	else vdma2.request &= 0x0e;
	realdrq1 = vdma1.request & ~(vdma1.mask);
	realdrq2 = vdma2.request & ~(vdma2.mask);
	/* get drq id of highest priority */
	if (realdrq2 == 0x00) return;
	id = 0x00;
	realdrq2 = (realdrq2 << (0x04 - (vdma2.drx))) | (realdrq2 >> (vdma2.drx));
	while (!((realdrq2 >> id) & 0x01) && (id < 0x04)) id++;
	id = (id + vdma2.drx) % 0x04;
	if (id == 0x01) {
		if (realdrq1 == 0x00) return;
		id = 0x00;
		realdrq1 = (realdrq1 << (0x04 - (vdma1.drx))) |
		           (realdrq1 >> (vdma1.drx));
		while (!((realdrq1 >> id) & 0x01) && (id < 0x04)) id++;
		id = (id + vdma1.drx) % 0x04;
		/* execute dreq from dma1 */
		switch (GetM(&vdma1,id)) {                /* select mode and command */
		case 0x00:                                                 /* demand */
			while (vdma1.channel[id].currwc != 0xffff && !vdma1.eop
			       && GetRealREQ(&vdma1,id))
				Transmission(&vdma1, id);
			break;
		case 0x01:                                                 /* single */
			Transmission(&vdma1, id);
			break;
		case 0x02:                                                  /* block */
			while (vdma1.channel[id].currwc != 0xffff && !vdma1.eop)
				Transmission(&vdma1, id);
			break;
		case 0x03:                                                /* cascade */
			/* do nothing */
			break;
		default:
			break;
		}
		if (vdma1.channel[id].currwc == 0xffff)
			vdma1.status |= 0x01 << id;
		if (GetAI(&vdma1,id)) {
			if (GetM(&vdma1,id) == 0x00 && vdma1.channel[id].currwc != 0xffff) ;
		/* TODO: Hang here */
		} 
	} else {
		/* execute dreq from dma2 */
	}
	/* finish dma transmission */
}
#ifdef VDMA_DEBUG
#define mov(n) (vcpu.al=n)
#define out(n) FUNEXEC(vcpuinsOutPort[n])
#endif
void vdmaInit()
{
	vdmaReset(&vdma1);
	vdmaReset(&vdma2);
	memset(&vdmalatch, 0x00, sizeof(t_dma_latch));

	vcpuinsInPort[0x0000] = (t_faddrcc)IO_Read_0000;
	vcpuinsInPort[0x0001] = (t_faddrcc)IO_Read_0001;
	vcpuinsInPort[0x0002] = (t_faddrcc)IO_Read_0002;
	vcpuinsInPort[0x0003] = (t_faddrcc)IO_Read_0003;
	vcpuinsInPort[0x0004] = (t_faddrcc)IO_Read_0004;
	vcpuinsInPort[0x0005] = (t_faddrcc)IO_Read_0005;
	vcpuinsInPort[0x0006] = (t_faddrcc)IO_Read_0006;
	vcpuinsInPort[0x0007] = (t_faddrcc)IO_Read_0007;
	vcpuinsInPort[0x0008] = (t_faddrcc)IO_Read_0008;
	vcpuinsInPort[0x000d] = (t_faddrcc)IO_Read_000D;

	vcpuinsOutPort[0x0000] = (t_faddrcc)IO_Write_0000;
	vcpuinsOutPort[0x0001] = (t_faddrcc)IO_Write_0001;
	vcpuinsOutPort[0x0002] = (t_faddrcc)IO_Write_0002;
	vcpuinsOutPort[0x0003] = (t_faddrcc)IO_Write_0003;
	vcpuinsOutPort[0x0004] = (t_faddrcc)IO_Write_0004;
	vcpuinsOutPort[0x0005] = (t_faddrcc)IO_Write_0005;
	vcpuinsOutPort[0x0006] = (t_faddrcc)IO_Write_0006;
	vcpuinsOutPort[0x0007] = (t_faddrcc)IO_Write_0007;
	vcpuinsOutPort[0x0008] = (t_faddrcc)IO_Write_0008;
	vcpuinsOutPort[0x0009] = (t_faddrcc)IO_Write_0009;
	vcpuinsOutPort[0x000a] = (t_faddrcc)IO_Write_000A;
	vcpuinsOutPort[0x000b] = (t_faddrcc)IO_Write_000B;
	vcpuinsOutPort[0x000c] = (t_faddrcc)IO_Write_000C;
	vcpuinsOutPort[0x000d] = (t_faddrcc)IO_Write_000D;
	vcpuinsOutPort[0x000e] = (t_faddrcc)IO_Write_000E;
	vcpuinsOutPort[0x000f] = (t_faddrcc)IO_Write_000F;

	vcpuinsInPort[0x0081] = (t_faddrcc)IO_Read_0081;
	vcpuinsInPort[0x0082] = (t_faddrcc)IO_Read_0082;
	vcpuinsInPort[0x0083] = (t_faddrcc)IO_Read_0083;
	vcpuinsInPort[0x0087] = (t_faddrcc)IO_Read_0087;
	vcpuinsInPort[0x0089] = (t_faddrcc)IO_Read_0089;
	vcpuinsInPort[0x008a] = (t_faddrcc)IO_Read_008A;
	vcpuinsInPort[0x008b] = (t_faddrcc)IO_Read_008B;
	vcpuinsInPort[0x008f] = (t_faddrcc)IO_Read_008F;

	vcpuinsOutPort[0x0081] = (t_faddrcc)IO_Write_0081;
	vcpuinsOutPort[0x0082] = (t_faddrcc)IO_Write_0082;
	vcpuinsOutPort[0x0083] = (t_faddrcc)IO_Write_0083;
	vcpuinsOutPort[0x0087] = (t_faddrcc)IO_Write_0087;
	vcpuinsOutPort[0x0089] = (t_faddrcc)IO_Write_0089;
	vcpuinsOutPort[0x008a] = (t_faddrcc)IO_Write_008A;
	vcpuinsOutPort[0x008b] = (t_faddrcc)IO_Write_008B;
	vcpuinsOutPort[0x008f] = (t_faddrcc)IO_Write_008F;

	vcpuinsInPort[0x00c0] = (t_faddrcc)IO_Read_00C0;
	vcpuinsInPort[0x00c2] = (t_faddrcc)IO_Read_00C2;
	vcpuinsInPort[0x00c4] = (t_faddrcc)IO_Read_00C4;
	vcpuinsInPort[0x00c6] = (t_faddrcc)IO_Read_00C6;
	vcpuinsInPort[0x00c8] = (t_faddrcc)IO_Read_00C8;
	vcpuinsInPort[0x00ca] = (t_faddrcc)IO_Read_00CA;
	vcpuinsInPort[0x00cc] = (t_faddrcc)IO_Read_00CC;
	vcpuinsInPort[0x00ce] = (t_faddrcc)IO_Read_00CE;
	vcpuinsInPort[0x00d0] = (t_faddrcc)IO_Read_00D0;
	vcpuinsInPort[0x00da] = (t_faddrcc)IO_Read_00DA;

	vcpuinsOutPort[0x00c0] = (t_faddrcc)IO_Write_00C0;
	vcpuinsOutPort[0x00c2] = (t_faddrcc)IO_Write_00C2;
	vcpuinsOutPort[0x00c4] = (t_faddrcc)IO_Write_00C4;
	vcpuinsOutPort[0x00c6] = (t_faddrcc)IO_Write_00C6;
	vcpuinsOutPort[0x00c8] = (t_faddrcc)IO_Write_00C8;
	vcpuinsOutPort[0x00ca] = (t_faddrcc)IO_Write_00CA;
	vcpuinsOutPort[0x00cc] = (t_faddrcc)IO_Write_00CC;
	vcpuinsOutPort[0x00ce] = (t_faddrcc)IO_Write_00CE;
	vcpuinsOutPort[0x00d0] = (t_faddrcc)IO_Write_00D0;
	vcpuinsOutPort[0x00d2] = (t_faddrcc)IO_Write_00D2;
	vcpuinsOutPort[0x00d4] = (t_faddrcc)IO_Write_00D4;
	vcpuinsOutPort[0x00d6] = (t_faddrcc)IO_Write_00D6;
	vcpuinsOutPort[0x00d8] = (t_faddrcc)IO_Write_00D8;
	vcpuinsOutPort[0x00dc] = (t_faddrcc)IO_Write_00DC;
	vcpuinsOutPort[0x00de] = (t_faddrcc)IO_Write_00DE;
#ifdef VDMA_DEBUG
	mov(0x00);
	out(0x08);
	out(0xd0);
	mov(0xc0);
	out(0xd6);
#endif
}
void vdmaFinal() {}
