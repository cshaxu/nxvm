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
/*void IO_Read_0000()
{
	if(!vdma1.msb)
		vcpu.al = (t_nubit8)(vdma1.dmac[0].cur_address&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma1.dmac[0].cur_address>>8);
	vdma1.msb = 1 - vdma1.msb;
}
void IO_Read_0001()
{
	if(!vdma1.msb)
		vcpu.al = (t_nubit8)(vdma1.dmac[0].cur_wordcount&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma1.dmac[0].cur_wordcount>>8);
	vdma1.msb = 1 - vdma1.msb;
}
void IO_Read_0002()
{
	if(!vdma1.msb)
		vcpu.al = (t_nubit8)(vdma1.dmac[1].cur_address&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma1.dmac[1].cur_address>>8);
	vdma1.msb = 1 - vdma1.msb;
}
void IO_Read_0003()
{
	if(!vdma1.msb)
		vcpu.al = (t_nubit8)(vdma1.dmac[1].cur_wordcount&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma1.dmac[1].cur_wordcount>>8);
	vdma1.msb = 1 - vdma1.msb;
}
void IO_Read_0004()
{
	if(!vdma1.msb)
		vcpu.al = (t_nubit8)(vdma1.dmac[2].cur_address&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma1.dmac[2].cur_address>>8);
	vdma1.msb = 1 - vdma1.msb;
}
void IO_Read_0005()
{
	if(!vdma1.msb)
		vcpu.al = (t_nubit8)(vdma1.dmac[2].cur_wordcount&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma1.dmac[2].cur_wordcount>>8);
	vdma1.msb = 1 - vdma1.msb;
}
void IO_Read_0006()
{
	if(!vdma1.msb)
		vcpu.al = (t_nubit8)(vdma1.dmac[3].cur_address&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma1.dmac[3].cur_address>>8);
	vdma1.msb = 1 - vdma1.msb;
}
void IO_Read_0007()
{
	if(!vdma1.msb)
		vcpu.al = (t_nubit8)(vdma1.dmac[3].cur_wordcount&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma1.dmac[3].cur_wordcount>>8);
	vdma1.msb = 1 - vdma1.msb;
}
void IO_Read_0008()
{
	// need to get status here
	vcpu.al = vdma1.status;
	vdma1.status = 0;
}
void IO_Read_000D()
{vcpu.al = vdma1.temp;}
void IO_Write_0000()
{
	if(!vdma1.msb)
		vdma1.dmac[0].base_address = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdma1.dmac[0].base_address |= ((t_nubit16)vcpu.al)<<8;
	vdma1.dmac[0].cur_address = vdma1.dmac[0].base_address;
	vdma1.msb = 1 - vdma1.msb;
}
void IO_Write_0001()
{
	if(!vdma1.msb)
		vdma1.dmac[0].base_wordcount = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdma1.dmac[0].base_wordcount |= ((t_nubit16)vcpu.al)<<8;
	vdma1.dmac[0].cur_wordcount = vdma1.dmac[0].base_wordcount;
	vdma1.msb = 1 - vdma1.msb;
}
void IO_Write_0002()
{
	if(!vdma1.msb)
		vdma1.dmac[1].base_address = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdma1.dmac[1].base_address |= ((t_nubit16)vcpu.al)<<8;
	vdma1.dmac[1].cur_address = vdma1.dmac[1].base_address;
	vdma1.msb = 1 - vdma1.msb;
}
void IO_Write_0003()
{
	if(!vdma1.msb)
		vdma1.dmac[1].base_wordcount = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdma1.dmac[1].base_wordcount |= ((t_nubit16)vcpu.al)<<8;
	vdma1.dmac[1].cur_wordcount = vdma1.dmac[1].base_wordcount;
	vdma1.msb = 1 - vdma1.msb;
}*/
void IO_Write_0004()
{
	if(!vdma1.msb)
		vdma1.dmac[2].base_address = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdma1.dmac[2].base_address |= ((t_nubit16)vcpu.al)<<8;
	vdma1.dmac[2].cur_address = vdma1.dmac[2].base_address;
	vdma1.msb = 1 - vdma1.msb;
}
void IO_Write_0005()
{
	if(!vdma1.msb)
		vdma1.dmac[2].base_wordcount = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdma1.dmac[2].base_wordcount |= ((t_nubit16)vcpu.al)<<8;
	vdma1.dmac[2].cur_wordcount = vdma1.dmac[2].base_wordcount;
	vdma1.msb = 1 - vdma1.msb;
}
/*void IO_Write_0006()
{
	if(!vdma1.msb)
		vdma1.dmac[3].base_address = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdma1.dmac[3].base_address |= ((t_nubit16)vcpu.al)<<8;
	vdma1.dmac[3].cur_address = vdma1.dmac[3].base_address;
	vdma1.msb = 1 - vdma1.msb;
}
void IO_Write_0007()
{
	if(!vdma1.msb)
		vdma1.dmac[3].base_wordcount = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdma1.dmac[3].base_wordcount |= ((t_nubit16)vcpu.al)<<8;
	vdma1.dmac[3].cur_wordcount = vdma1.dmac[3].base_wordcount;
	vdma1.msb = 1 - vdma1.msb;
}
void IO_Write_0008()
{vdma1.command = vcpu.al;}
void IO_Write_0009()
{vdma1.request = vcpu.al;}
void IO_Write_000A()
{
	if(vcpu.al&0x04) vdma1.mask |= 1<<(vcpu.al&0x03);
	else vdma1.mask &= ~(1<<(vcpu.al&0x03));
}
void IO_Write_000B()
{vdma1.dmac[vcpu.al&0x03].mode = vcpu.al>>2;}
void IO_Write_000C()
{vdma1.msb = 0;}
void IO_Write_000D()
{
	vdma1.command = vdma1.status = vdma1.temp = 0;
	vdma1.request = 0;
	vdma1.msb = 0;
	vdma1.mask = 0x0f;
}
void IO_Write_000E()
{vdma1.mask = 0;}
void IO_Write_000F()
{vdma1.mask = vcpu.al&0x0f;}

void IO_Read_00C0()
{
	if(!vdma2.msb)
		vcpu.al = (t_nubit8)(vdma2.dmac[0].cur_address&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma2.dmac[0].cur_address>>8);
	vdma2.msb = 1 - vdma2.msb;
}
void IO_Read_00C2()
{
	if(!vdma2.msb)
		vcpu.al = (t_nubit8)(vdma2.dmac[0].cur_wordcount&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma2.dmac[0].cur_wordcount>>8);
	vdma2.msb = 1 - vdma2.msb;
}
void IO_Read_00C4()
{
	if(!vdma2.msb)
		vcpu.al = (t_nubit8)(vdma2.dmac[1].cur_address&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma2.dmac[1].cur_address>>8);
	vdma2.msb = 1 - vdma2.msb;
}
void IO_Read_00C6()
{
	if(!vdma2.msb)
		vcpu.al = (t_nubit8)(vdma2.dmac[1].cur_wordcount&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma2.dmac[1].cur_wordcount>>8);
	vdma2.msb = 1 - vdma2.msb;
}
void IO_Read_00C8()
{
	if(!vdma2.msb)
		vcpu.al = (t_nubit8)(vdma2.dmac[2].cur_address&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma2.dmac[2].cur_address>>8);
	vdma2.msb = 1 - vdma2.msb;
}
void IO_Read_00CA()
{
	if(!vdma2.msb)
		vcpu.al = (t_nubit8)(vdma2.dmac[2].cur_wordcount&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma2.dmac[2].cur_wordcount>>8);
	vdma2.msb = 1 - vdma2.msb;
}
void IO_Read_00CC()
{
	if(!vdma2.msb)
		vcpu.al = (t_nubit8)(vdma2.dmac[3].cur_address&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma2.dmac[3].cur_address>>8);
	vdma2.msb = 1 - vdma2.msb;
}
void IO_Read_00CE()
{
	if(!vdma2.msb)
		vcpu.al = (t_nubit8)(vdma2.dmac[3].cur_wordcount&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdma2.dmac[3].cur_wordcount>>8);
	vdma2.msb = 1 - vdma2.msb;
}
void IO_Read_00D0()
{
	// Get status here
	vcpu.al = vdma2.status;
	vdma2.status = 0;
}
void IO_Read_00DA()
{vcpu.al = vdma2.temp;}
void IO_Write_00C0()
{
	if(!vdma2.msb)
		vdma2.dmac[0].base_address = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdma2.dmac[0].base_address |= ((t_nubit16)vcpu.al)<<8;
	vdma2.dmac[0].cur_address = vdma2.dmac[0].base_address;
	vdma2.msb = 1 - vdma2.msb;
}
void IO_Write_00C2()
{
	if(!vdma2.msb)
		vdma2.dmac[0].base_wordcount = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdma2.dmac[0].base_wordcount |= ((t_nubit16)vcpu.al)<<8;
	vdma2.dmac[0].cur_wordcount = vdma2.dmac[0].base_wordcount;
	vdma2.msb = 1 - vdma2.msb;
}
void IO_Write_00C4()
{
	if(!vdma2.msb)
		vdma2.dmac[1].base_address = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdma2.dmac[1].base_address |= ((t_nubit16)vcpu.al)<<8;
	vdma2.dmac[1].cur_address = vdma2.dmac[1].base_address;
	vdma2.msb = 1 - vdma2.msb;
}
void IO_Write_00C6()
{
	if(!vdma2.msb)
		vdma2.dmac[1].base_wordcount = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdma2.dmac[1].base_wordcount |= ((t_nubit16)vcpu.al)<<8;
	vdma2.dmac[1].cur_wordcount = vdma2.dmac[1].base_wordcount;
	vdma2.msb = 1 - vdma2.msb;
}
void IO_Write_00C8()
{
	if(!vdma2.msb)
		vdma2.dmac[2].base_address = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdma2.dmac[2].base_address |= ((t_nubit16)vcpu.al)<<8;
	vdma2.dmac[2].cur_address = vdma2.dmac[2].base_address;
	vdma2.msb = 1 - vdma2.msb;
}
void IO_Write_00CA()
{
	if(!vdma2.msb)
		vdma2.dmac[2].base_wordcount = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdma2.dmac[2].base_wordcount |= ((t_nubit16)vcpu.al)<<8;
	vdma2.dmac[2].cur_wordcount = vdma2.dmac[2].base_wordcount;
	vdma2.msb = 1 - vdma2.msb;
}
void IO_Write_00CC()
{
	if(!vdma2.msb)
		vdma2.dmac[3].base_address = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdma2.dmac[3].base_address |= ((t_nubit16)vcpu.al)<<8;
	vdma2.dmac[3].cur_address = vdma2.dmac[3].base_address;
	vdma2.msb = 1 - vdma2.msb;
}
void IO_Write_00CE()
{
	if(!vdma2.msb)
		vdma2.dmac[3].base_wordcount = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdma2.dmac[3].base_wordcount |= ((t_nubit16)vcpu.al)<<8;
	vdma2.dmac[3].cur_wordcount = vdma2.dmac[3].base_wordcount;
	vdma2.msb = 1 - vdma2.msb;
}
void IO_Write_00D0()
{vdma2.command = vcpu.al;}
void IO_Write_00D2()
{vdma2.request = vcpu.al;}
void IO_Write_00D4()
{
	if(vcpu.al&0x04) vdma2.mask |= 1<<(vcpu.al&0x03);
	else vdma2.mask &= ~(1<<(vcpu.al&0x03));
}
void IO_Write_00D6()
{vdma2.dmac[vcpu.al&0x03].mode = vcpu.al;}
void IO_Write_00D8()
{vdma2.msb = 0;}
void IO_Write_00DA()
{
	vdma2.command = vdma2.status = vdma2.temp = 0;
	vdma2.request = 0;
	vdma2.msb = 0;
	vdma2.mask = 0x0f;
}
void IO_Write_00DC()
{vdma2.mask = 0;}
void IO_Write_00DE()
{vdma2.mask = vcpu.al&0x0f;}

void IO_Read_0087()
{vcpu.al = vdma1.dmac[0].page;}
void IO_Read_0083()
{vcpu.al = vdma1.dmac[1].page;}
void IO_Read_0081()
{vcpu.al = vdma1.dmac[2].page;}
void IO_Read_0082()
{vcpu.al = vdma1.dmac[3].page;}
void IO_Read_008B()
{vcpu.al = vdma2.dmac[1].page;}
void IO_Read_0089()
{vcpu.al = vdma2.dmac[2].page;}
void IO_Read_008A()
{vcpu.al = vdma2.dmac[3].page;}
void IO_Read_008F()
{vcpu.al = vdma2.dmac[0].page;}
void IO_Write_0087()
{vdma1.dmac[0].page = vcpu.al;}
void IO_Write_0083()
{vdma1.dmac[1].page = vcpu.al;}*/
void IO_Write_0081()
{vdma1.dmac[2].page = vcpu.al;}
/*void IO_Write_0082()
{vdma1.dmac[3].page = vcpu.al;}
void IO_Write_008B()
{vdma2.dmac[1].page = vcpu.al;}
void IO_Write_0089()
{vdma2.dmac[2].page = vcpu.al;}
void IO_Write_008A()
{vdma2.dmac[3].page = vcpu.al;}
void IO_Write_008F()
{vdma2.dmac[0].page = vcpu.al;}*/

void vdmaInit()
{
	memset(&vdma1,0,sizeof(t_dma));
	memset(&vdma2,0,sizeof(t_dma));
	//vdma1.mask = vdma2.mask = 0x0f;
	//vcpuinsInPort[0x0000] = (t_faddrcc)IO_Read_0000;
	//vcpuinsInPort[0x0001] = (t_faddrcc)IO_Read_0001;
	/*vcpuinsInPort[0x0002] = (t_faddrcc)IO_Read_0002;
	vcpuinsInPort[0x0003] = (t_faddrcc)IO_Read_0003;*/
	//vcpuinsInPort[0x0004] = (t_faddrcc)IO_Read_0004;
	//vcpuinsInPort[0x0005] = (t_faddrcc)IO_Read_0005;
	//vcpuinsInPort[0x0006] = (t_faddrcc)IO_Read_0006;
	//vcpuinsInPort[0x0007] = (t_faddrcc)IO_Read_0007;
	//vcpuinsInPort[0x0008] = (t_faddrcc)IO_Read_0008;
	//vcpuinsInPort[0x000d] = (t_faddrcc)IO_Read_000D;
	//vcpuinsOutPort[0x0000] = (t_faddrcc)IO_Write_0000;
	//vcpuinsOutPort[0x0001] = (t_faddrcc)IO_Write_0001;
	//vcpuinsOutPort[0x0002] = (t_faddrcc)IO_Write_0002;
	//vcpuinsOutPort[0x0003] = (t_faddrcc)IO_Write_0003;
	vcpuinsOutPort[0x0004] = (t_faddrcc)IO_Write_0004;
	vcpuinsOutPort[0x0005] = (t_faddrcc)IO_Write_0005;
	//vcpuinsOutPort[0x0006] = (t_faddrcc)IO_Write_0006;
	//vcpuinsOutPort[0x0007] = (t_faddrcc)IO_Write_0007;
	//vcpuinsOutPort[0x0008] = (t_faddrcc)IO_Write_0008;
	//vcpuinsOutPort[0x0009] = (t_faddrcc)IO_Write_0009;
	//vcpuinsOutPort[0x000a] = (t_faddrcc)IO_Write_000A;
	//vcpuinsOutPort[0x000b] = (t_faddrcc)IO_Write_000B;
	//vcpuinsOutPort[0x000c] = (t_faddrcc)IO_Write_000C;
	//vcpuinsOutPort[0x000d] = (t_faddrcc)IO_Write_000D;
	//vcpuinsOutPort[0x000e] = (t_faddrcc)IO_Write_000E;
	//vcpuinsOutPort[0x000f] = (t_faddrcc)IO_Write_000F;
	//vcpuinsInPort[0x00c0] = (t_faddrcc)IO_Read_00C0;
	//vcpuinsInPort[0x00c2] = (t_faddrcc)IO_Read_00C2;
	//vcpuinsInPort[0x00c4] = (t_faddrcc)IO_Read_00C4;
	//vcpuinsInPort[0x00c6] = (t_faddrcc)IO_Read_00C6;
	//vcpuinsInPort[0x00c8] = (t_faddrcc)IO_Read_00C8;
	//vcpuinsInPort[0x00ca] = (t_faddrcc)IO_Read_00CA;
	//vcpuinsInPort[0x00cc] = (t_faddrcc)IO_Read_00CC;
	//vcpuinsInPort[0x00ce] = (t_faddrcc)IO_Read_00CE;
	//vcpuinsInPort[0x00d0] = (t_faddrcc)IO_Read_00D0;
	//vcpuinsInPort[0x00da] = (t_faddrcc)IO_Read_00DA;
	//vcpuinsOutPort[0x00c0] = (t_faddrcc)IO_Write_00C0;
	//vcpuinsOutPort[0x00c2] = (t_faddrcc)IO_Write_00C2;
	//vcpuinsOutPort[0x00c4] = (t_faddrcc)IO_Write_00C4;
	//vcpuinsOutPort[0x00c6] = (t_faddrcc)IO_Write_00C6;
	//vcpuinsOutPort[0x00c8] = (t_faddrcc)IO_Write_00C8;
	//vcpuinsOutPort[0x00ca] = (t_faddrcc)IO_Write_00CA;
	//vcpuinsOutPort[0x00cc] = (t_faddrcc)IO_Write_00CC;
	//vcpuinsOutPort[0x00ce] = (t_faddrcc)IO_Write_00CE;
	//vcpuinsOutPort[0x00d0] = (t_faddrcc)IO_Write_00D0;
	//vcpuinsOutPort[0x00d2] = (t_faddrcc)IO_Write_00D2;
	//vcpuinsOutPort[0x00d4] = (t_faddrcc)IO_Write_00D4;
	//vcpuinsOutPort[0x00d6] = (t_faddrcc)IO_Write_00D6;
	//vcpuinsOutPort[0x00d8] = (t_faddrcc)IO_Write_00D8;
	//vcpuinsOutPort[0x00dc] = (t_faddrcc)IO_Write_00DC;
	//vcpuinsOutPort[0x00de] = (t_faddrcc)IO_Write_00DE;

	//vcpuinsInPort[0x0087] = (t_faddrcc)IO_Read_0087;
	//vcpuinsInPort[0x0083] = (t_faddrcc)IO_Read_0083;
	//vcpuinsInPort[0x0081] = (t_faddrcc)IO_Read_0081;
	//vcpuinsInPort[0x0082] = (t_faddrcc)IO_Read_0082;
	//vcpuinsInPort[0x008b] = (t_faddrcc)IO_Read_008B;
	//vcpuinsInPort[0x0089] = (t_faddrcc)IO_Read_0089;
	//vcpuinsInPort[0x008a] = (t_faddrcc)IO_Read_008A;
	//vcpuinsInPort[0x008f] = (t_faddrcc)IO_Read_008F;
	//vcpuinsOutPort[0x0087] = (t_faddrcc)IO_Write_0087;
	//vcpuinsOutPort[0x0083] = (t_faddrcc)IO_Write_0083;
	vcpuinsOutPort[0x0081] = (t_faddrcc)IO_Write_0081;
	//vcpuinsOutPort[0x0082] = (t_faddrcc)IO_Write_0082;
	//vcpuinsOutPort[0x008b] = (t_faddrcc)IO_Write_008B;
	//vcpuinsOutPort[0x0089] = (t_faddrcc)IO_Write_0089;
	//vcpuinsOutPort[0x008a] = (t_faddrcc)IO_Write_008A;
	//vcpuinsOutPort[0x008f] = (t_faddrcc)IO_Write_008F;
}
void vdmaFinal() {}
