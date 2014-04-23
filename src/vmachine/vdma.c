/* This file is a part of NXVM project. */

// DMAC is not completely implemented:
// it is only used to store the memory address and word count.

#include "memory.h"

#include "system/vapi.h"
#include "vcpu.h"
#include "vcpuins.h"
#include "vdma.h"

t_dma vdma1,vdma2;

static void vdmacBar0x00() {/* DUMMY FUNCTION */}

static void IO_Read_CurrentAddress(t_dma *vdma, t_nubitcc id)
{
	if(!vdma->flagmsb)
		vcpu.al = (t_nubit8)(vdma->dmac[id].curraddr & 0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma->dmac[id].curraddr >> 8);
	vdma->flagmsb = 0x01 - vdma->flagmsb;
}
static void IO_Read_CurrentWordCount(t_dma *vdma, t_nubitcc id)
{
	if(!vdma->flagmsb)
		vcpu.al = (t_nubit8)(vdma->dmac[id].currwc & 0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma->dmac[id].currwc >> 8);
	vdma->flagmsb = 0x01 - vdma->flagmsb;
}
static void IO_Read_Status(t_dma *vdma)
{
	t_nubit4 tc = 0x00; /* TODO IMPORTANT */
	vdma->status = (vdma->request << 4) | (tc & 0x0f);
	vcpu.al = vdma->status;
	vdma->status = 0;
}
#define     IO_Read_Temp(vdma) (vcpu.al = (vdma)->temp)
static void IO_Write_Address(t_dma *vdma, t_nubitcc id)
{
	if(!vdma->flagmsb)
		vdma->dmac[id].baseaddr = (t_nubit16)vcpu.al;
	else
		vdma->dmac[id].baseaddr |= (t_nubit16)(vcpu.al << 8);
	vdma->dmac[id].curraddr = vdma->dmac[id].baseaddr;
	vdma->flagmsb = 0x01 - vdma->flagmsb;
}
static void IO_Write_WordCount(t_dma *vdma, t_nubitcc id)
{
	if(!vdma->flagmsb)
		vdma->dmac[id].basewc = (t_nubit16)vcpu.al;
	else
		vdma->dmac[id].basewc |= (t_nubit16)(vcpu.al << 8);
	vdma->dmac[id].currwc = vdma->dmac[id].basewc;
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
#define     IO_Write_Mode(vdma) ((vdma)->dmac[vcpu.al & 0x03].mode = vcpu.al >> 2)
#define     IO_Write_Flipflop_Clear(vdma) ((vdma)->flagmsb = 0x00)
#define     IO_Write_Reset(vdma) (vdmaReset(vdma))
#define     IO_Write_Mask_Clear(vdma) ((vdma)->mask = 0x00)
#define     IO_Write_Mask_All(vdma) ((vdma)->mask = vcpu.al & 0x0f)
#define     IO_Read_Page(vdma, id) (vcpu.al = (vdma)->dmac[id].page)
#define     IO_Write_Page(vdma, id)((vdma)->dmac[id].page = vcpu.al)

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
void IO_Read_0083()
{vcpu.al = vdma1.dmac[1].page;}
void IO_Read_0087()
{vcpu.al = vdma1.dmac[0].page;}
void IO_Read_0089()
{vcpu.al = vdma2.dmac[2].page;}
void IO_Read_008A()
{vcpu.al = vdma2.dmac[3].page;}
void IO_Read_008B()
{vcpu.al = vdma2.dmac[1].page;}
void IO_Read_008F()
{vcpu.al = vdma2.dmac[0].page;}

void IO_Write_0081()
{vdma1.dmac[2].page = vcpu.al;}
void IO_Write_0082()
{vdma1.dmac[3].page = vcpu.al;}
void IO_Write_0083()
{vdma1.dmac[1].page = vcpu.al;}
void IO_Write_0087()
{vdma1.dmac[0].page = vcpu.al;}
void IO_Write_0089()
{vdma2.dmac[2].page = vcpu.al;}
void IO_Write_008A()
{vdma2.dmac[3].page = vcpu.al;}
void IO_Write_008B()
{vdma2.dmac[1].page = vcpu.al;}
void IO_Write_008F()
{vdma2.dmac[0].page = vcpu.al;}

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

void vdmaReset(t_dma *vdma)
{
	memset(vdma, 0x00, sizeof(t_dma));
	vdma->mask = 0x0f;
}
void vdmaRefresh()
{}
void vdmaInit()
{
	vdmaReset(&vdma1);
	vdmaReset(&vdma2);

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
}
void vdmaFinal() {}
