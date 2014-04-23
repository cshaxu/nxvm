
#include "vcpu.h"
#include "vcpuins.h"

#include "qdkeyb.h"

void IO_Read_0064()
{
	vcpu.iobyte = 0x10;
}

void qdkeybInit()
{
	vcpuinsInPort[0x0064] = (t_faddrcc)IO_Read_0064;
}
void qdkeybFinal()
{}