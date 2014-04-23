/* This file is a part of NXVM project. */

#include "memory.h"

#include "vcpu.h"
#include "vcpuins.h"
#include "vdma.h"

t_dmac vdmac1,vdmac2;

void IO_Read_0000()
{
	if(!vdmac1.msb)
		vcpu.al = (t_nubit8)(vdmac1.channel[0].cur_address&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdmac1.channel[0].cur_address>>8);
	vdmac1.msb = 1 - vdmac1.msb;
}
void IO_Read_0001()
{
	if(!vdmac1.msb)
		vcpu.al = (t_nubit8)(vdmac1.channel[0].cur_wordcount&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdmac1.channel[0].cur_wordcount>>8);
	vdmac1.msb = 1 - vdmac1.msb;
}
void IO_Read_0002()
{
	if(!vdmac1.msb)
		vcpu.al = (t_nubit8)(vdmac1.channel[1].cur_address&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdmac1.channel[1].cur_address>>8);
	vdmac1.msb = 1 - vdmac1.msb;
}
void IO_Read_0003()
{
	if(!vdmac1.msb)
		vcpu.al = (t_nubit8)(vdmac1.channel[1].cur_wordcount&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdmac1.channel[1].cur_wordcount>>8);
	vdmac1.msb = 1 - vdmac1.msb;
}
void IO_Read_0004()
{
	if(!vdmac1.msb)
		vcpu.al = (t_nubit8)(vdmac1.channel[2].cur_address&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdmac1.channel[2].cur_address>>8);
	vdmac1.msb = 1 - vdmac1.msb;
}
void IO_Read_0005()
{
	if(!vdmac1.msb)
		vcpu.al = (t_nubit8)(vdmac1.channel[2].cur_wordcount&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdmac1.channel[2].cur_wordcount>>8);
	vdmac1.msb = 1 - vdmac1.msb;
}
void IO_Read_0006()
{
	if(!vdmac1.msb)
		vcpu.al = (t_nubit8)(vdmac1.channel[3].cur_address&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdmac1.channel[3].cur_address>>8);
	vdmac1.msb = 1 - vdmac1.msb;
}
void IO_Read_0007()
{
	if(!vdmac1.msb)
		vcpu.al = (t_nubit8)(vdmac1.channel[3].cur_wordcount&0x00ff);
	else
		vcpu.al = (t_nubit8)(vdmac1.channel[3].cur_wordcount>>8);
	vdmac1.msb = 1 - vdmac1.msb;
}
void IO_Read_0008()
{
	/*Get status here*/
	vcpu.al = vdmac1.status;
	vdmac1.status = 0;
}
void IO_Read_000D()
{
	vcpu.al = vdmac1.temp;
}
void IO_Write_0000()
{
	if(!vdmac1.msb)
		vdmac1.channel[0].base_address = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdmac1.channel[0].base_address |= ((t_nubit16)vcpu.al)<<8;
	vdmac1.msb = 1 - vdmac1.msb;
}
void IO_Write_0001()
{
	if(!vdmac1.msb)
		vdmac1.channel[0].base_wordcount = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdmac1.channel[0].base_wordcount |= ((t_nubit16)vcpu.al)<<8;
	vdmac1.msb = 1 - vdmac1.msb;
}
void IO_Write_0002()
{
	if(!vdmac1.msb)
		vdmac1.channel[1].base_address = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdmac1.channel[1].base_address |= ((t_nubit16)vcpu.al)<<8;
	vdmac1.msb = 1 - vdmac1.msb;
}
void IO_Write_0003()
{
	if(!vdmac1.msb)
		vdmac1.channel[1].base_wordcount = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdmac1.channel[1].base_wordcount |= ((t_nubit16)vcpu.al)<<8;
	vdmac1.msb = 1 - vdmac1.msb;
}
void IO_Write_0004()
{
	if(!vdmac1.msb)
		vdmac1.channel[2].base_address = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdmac1.channel[2].base_address |= ((t_nubit16)vcpu.al)<<8;
	vdmac1.msb = 1 - vdmac1.msb;
}
void IO_Write_0005()
{
	if(!vdmac1.msb)
		vdmac1.channel[2].base_wordcount = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdmac1.channel[2].base_wordcount |= ((t_nubit16)vcpu.al)<<8;
	vdmac1.msb = 1 - vdmac1.msb;
}
void IO_Write_0006()
{
	if(!vdmac1.msb)
		vdmac1.channel[3].base_address = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdmac1.channel[3].base_address |= ((t_nubit16)vcpu.al)<<8;
	vdmac1.msb = 1 - vdmac1.msb;
}
void IO_Write_0007()
{
	if(!vdmac1.msb)
		vdmac1.channel[3].base_wordcount = ((t_nubit16)vcpu.al)&0x00ff;
	else
		vdmac1.channel[3].base_wordcount |= ((t_nubit16)vcpu.al)<<8;
	vdmac1.msb = 1 - vdmac1.msb;
}
void IO_Write_0008()
{
	vdmac1.command = vcpu.al;
}
void IO_Write_0009()
{
	vdmac1.request = vcpu.al;
}
void IO_Write_000A()
{
	if(vcpu.al&0x04) vdmac1.mask |= 1<<(vcpu.al&0x03);
	else vdmac1.mask &= ~(1<<(vcpu.al&0x03));
}
void IO_Write_000B()
{
	vdmac1.channel[vcpu.al&0x03].mode = vcpu.al;
}
void IO_Write_000C()
{
	vdmac1.msb = 0;
}
void IO_Write_000D()
{
	vdmac1.command = vdmac1.status = vdmac1.temp = 0;
	vdmac1.request = 0;
	vdmac1.msb = 0;
	vdmac1.mask = 0x0f;
}
void IO_Write_000E()
{
	vdmac1.mask = 0;
}
void IO_Write_000F()
{
	vdmac1.mask = vcpu.al&0x0f;
}

void IO_Read_00C0() {}
void IO_Read_00C2() {}
void IO_Read_00C4() {}
void IO_Read_00C6() {}
void IO_Read_00C8() {}
void IO_Read_00CA() {}
void IO_Read_00CC() {}
void IO_Read_00CE() {}
void IO_Read_00D0() {}
void IO_Read_00DA() {}
void IO_Write_00C0() {}
void IO_Write_00C2() {}
void IO_Write_00C4() {}
void IO_Write_00C6() {}
void IO_Write_00C8() {}
void IO_Write_00CA() {}
void IO_Write_00CC() {}
void IO_Write_00CE() {}
void IO_Write_00D0() {}
void IO_Write_00D2() {}
void IO_Write_00D4() {}
void IO_Write_00D6() {}
void IO_Write_00D8() {}
void IO_Write_00DC() {}
void IO_Write_00DE() {}

void DMAInit()
{
	memset(&vdmac1,0,sizeof(t_dmac));
	memset(&vdmac2,0,sizeof(t_dmac));
	vdmac1.mask = vdmac2.mask = 0x0f;
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
void DMATerm() {}