
#include "stdio.h"
#include "memory.h"

#include "../vapi.h"
#include "../vcpuins.h"
#include "../vpic.h"
#include "ecpu.h"
#include "ecpuins.h"
#include "ecpuapi.h"

t_ecpu ecpu;

void ecpuInit()
{
	ecpuinsInit();
	vport.in[0xffff] = (t_faddrcc)ecpuapiPrintRegs;
}
void ecpuReset()
{
	memset(&ecpu,0,sizeof(t_ecpu));
	ecpu.cs.selector = 0xf000;
	ecpu.ip = 0xfff0;
}
void ecpuRefreshInit()
{
	ecpuapiSyncRegs();
}
void ecpuRefresh()
{
	ecpuinsRefresh();
}
void ecpuRefreshFinal()
{
	if (ecpuapiHasDiff()) vapiCallBackMachineStop();
}
void ecpuFinal()
{
	ecpuinsFinal();
}
