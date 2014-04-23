/* This file is a part of NXVM project. */

// Not completely simulated.

#include "memory.h"

#include "system/vapi.h"

#include "vcpu.h"
#include "vpic.h"
#include "vcpuins.h"

#include "vpit.h"

t_pit vpit;

#define GetSC(cw)  (((cw) & 0xc0)>>0x06)
#define GetRW(cw)  (((cw) & 0x30)>>0x04)
#define GetM(cw)   (((cw) & 0x0e)>>0x01)
#define GetBCD(cw) (((cw) & 0x01)>>0x00)

static clock_t t1,t2;
static int i;	// now result is 68000 @ ora.xha

void vpitIntTick()
{
	t_float64 dt;
	t2 = clock();
	dt = (t_float64)((t2-t1)*1e3/((t_float64)CLOCKS_PER_SEC));
	i++; 
	if(dt >= VPIT_TICK) {
		vapiPrint("%lf,%d\n",dt,i);
		i = 0; 
		vpicSetIRQ(0x00);
		vapiPrint("%lf\n",dt); 
		t1 = t2;
	}
}

void IO_Read_0040() {}
void IO_Read_0041() {}
void IO_Read_0042() {}
void IO_Write_0040() {}
void IO_Write_0041() {}
void IO_Write_0042() {}
void IO_Write_0043()
{
	vpit.cw[GetSC(vcpu.al)] = vcpu.al;
}
#ifdef VPIT_DEBUG
void IO_Read_FF40() /* prnit all info */
{}
#endif

void vpitRefresh()
{

}
#ifdef VPIT_DEBUG
#define mov(n) (vcpu.al=n)
#define out(n) FUNEXEC(vcpuinsOutPort[n])
#endif
void vpitInit()
{
	memset(&vpit,0,sizeof(t_pit));
	t1 = clock();
	vcpuinsInPort[0x0040] = (t_faddrcc)IO_Read_0040;
	vcpuinsInPort[0x0041] = (t_faddrcc)IO_Read_0041;
	vcpuinsInPort[0x0042] = (t_faddrcc)IO_Read_0042;
	vcpuinsOutPort[0x0040] = (t_faddrcc)IO_Write_0040;
	vcpuinsOutPort[0x0041] = (t_faddrcc)IO_Write_0041;
	vcpuinsOutPort[0x0042] = (t_faddrcc)IO_Write_0042;
	vcpuinsOutPort[0x0043] = (t_faddrcc)IO_Write_0043;
#ifdef VPIT_DEBUG
	vcpuinsInPort[0xff40] = (t_faddrcc)IO_Read_FF40;
	mov(0x36); /* al=0011 0110: Mode=3, Counter=0, 16b */
	out(0x43);
	mov(0x00);
	out(0x40);
	out(0x40);
	mov(0x54); /* al=0101 0100: Mode=2, Counter=1, LSB */
	out(0x43);
	mov(  18);
    out(0x41);
#endif
}
void vpitFinal() {}
