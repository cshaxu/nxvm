
#include "stdio.h"
#include "memory.h"

#include "../vapi.h"
#include "../vpic.h"
#include "../vcpu.h"
#include "../vcpuins.h"

#include "ecpu.h"
#include "ecpuapi.h"
#include "ecpuins.h"

t_ecpu ecpu;
static t_ecpu ecpu2;
#if VGLOBAL_ECPU_MODE != TEST_ECPU
static t_cpu  vcpu2;
#endif

void ecpuapiSyncRegs()
{
	ecpu.flagignore = 0;
	ecpu.ax = _ax;
	ecpu.bx = _bx;
	ecpu.cx = _cx;
	ecpu.dx = _dx;
	ecpu.sp = _sp;
	ecpu.bp = _bp;
	ecpu.si = _si;
	ecpu.di = _di;
	ecpu.ip = _ip;
	ecpu.cs.selector = _cs;
	ecpu.ds.selector = _ds;
	ecpu.es.selector = _es;
	ecpu.ss.selector = _ss;
	ecpu.flags = _flags;
	ecpu2 = ecpu;
#if VGLOBAL_ECPU_MODE != TEST_ECPU
	vcpu2 = vcpu;
#endif
}
t_bool ecpuapiHasDiff()
{
	t_bool flagdiff = 0x00;
	t_nubit16 mask = GetMax16(vcpuins.udf);
	ecpu.flagignore |= vcpuins.flagrespondint;
	if (!ecpu.flagignore) {
		if (vcpuins.flagmem != ecpuins.flagmem) {
			vapiPrint("diff flagmem: E=%01X, V=%01X\n", ecpuins.flagmem, vcpuins.flagmem);
			flagdiff = 0x01;
		}
		if (ecpu.ax != _ax) {vapiPrint("diff ax\n");flagdiff = 0x01;}
		if (ecpu.bx != _bx) {vapiPrint("diff bx\n");flagdiff = 0x01;}
		if (ecpu.cx != _cx) {vapiPrint("diff cx\n");flagdiff = 0x01;}
		if (ecpu.dx != _dx) {vapiPrint("diff dx\n");flagdiff = 0x01;}
		if (ecpu.sp != _sp) {vapiPrint("diff sp\n");flagdiff = 0x01;}
		if (ecpu.bp != _bp) {vapiPrint("diff bp\n");flagdiff = 0x01;}
		if (ecpu.si != _si) {vapiPrint("diff si\n");flagdiff = 0x01;}
		if (ecpu.di != _di) {vapiPrint("diff di\n");flagdiff = 0x01;}
		if (ecpu.ip != _ip) {vapiPrint("diff ip\n");flagdiff = 0x01;}
		if (ecpu.cs.selector != _cs) {vapiPrint("diff cs\n");flagdiff = 0x01;}
		if (ecpu.ds.selector != _ds) {vapiPrint("diff ds\n");flagdiff = 0x01;}
		if (ecpu.es.selector != _es) {vapiPrint("diff es\n");flagdiff = 0x01;}
		if (ecpu.ss.selector != _ss) {vapiPrint("diff ss\n");flagdiff = 0x01;}
		if ((ecpu.flags & ~mask) != (_flags & ~mask)) {
			vapiPrint("diff flags: E=%04X, V=%04X\n", ecpu.flags, _flags);
			flagdiff = 0x01;
		}
	}
	if (flagdiff) {
		vapiPrint("AFTER EXECUTION:\n");
		ecpuapiPrintRegs();
		vapiPrint("VCPU REGISTERS\n");
		vapiCallBackDebugPrintRegs();
		ecpu = ecpu2;
#if VGLOBAL_ECPU_MODE != TEST_ECPU
		vcpu = vcpu2;
#endif
		vapiPrint("BEFORE EXECUTION:\n");
		ecpuapiPrintRegs();
		vapiPrint("VCPU REGISTERS\n");
		vapiCallBackDebugPrintRegs();
		vapiPrint("-----------------------------------------\n");
	}
	return flagdiff;
}
void ecpuapiPrintRegs()
{
	vapiPrint("ECPU REGISTERS\n");
	vapiPrint(  "AX=%04X", ecpu.ax);
	vapiPrint("  BX=%04X", ecpu.bx);
	vapiPrint("  CX=%04X", ecpu.cx);
	vapiPrint("  DX=%04X", ecpu.dx);
	vapiPrint("  SP=%04X", ecpu.sp);
	vapiPrint("  BP=%04X", ecpu.bp);
	vapiPrint("  SI=%04X", ecpu.si);
	vapiPrint("  DI=%04X", ecpu.di);
	vapiPrint("\nDS=%04X", ecpu.ds);
	vapiPrint("  ES=%04X", ecpu.es);
	vapiPrint("  SS=%04X", ecpu.ss);
	vapiPrint("  CS=%04X", ecpu.cs);
	vapiPrint("  IP=%04X", ecpu.ip);
	vapiPrint("   ");
	if(ecpu.flags & VCPU_EFLAGS_OF) vapiPrint("OV ");
	else                            vapiPrint("NV ");
	if(ecpu.flags & VCPU_EFLAGS_DF) vapiPrint("DN ");
	else                            vapiPrint("UP ");
	if(ecpu.flags & VCPU_EFLAGS_IF) vapiPrint("EI ");
	else                            vapiPrint("DI ");
	if(ecpu.flags & VCPU_EFLAGS_SF) vapiPrint("NG ");
	else                            vapiPrint("PL ");
	if(ecpu.flags & VCPU_EFLAGS_ZF) vapiPrint("ZR ");
	else                            vapiPrint("NZ ");
	if(ecpu.flags & VCPU_EFLAGS_AF) vapiPrint("AC ");
	else                            vapiPrint("NA ");
	if(ecpu.flags & VCPU_EFLAGS_PF) vapiPrint("PE ");
	else                            vapiPrint("PO ");
	if(ecpu.flags & VCPU_EFLAGS_CF) vapiPrint("CY ");
	else                            vapiPrint("NC ");
	vapiPrint("\n");
}

t_bool ecpuapiScanINTR()
{
	return vpicScanINTR();
}
t_nubit8 ecpuapiGetINTR()
{
#if (VGLOBAL_ECPU_MODE == TEST_ECPU)
	return vpicGetINTR();
#else
	return vpicPeekINTR();
#endif
}