/* This file is a part of NXVM project. */

#include "memory.h"

#include "vapi.h"

#include "vcpuins.h"
#include "vcpu.h"

#if VGLOBAL_ECPU_MODE != TEST_VCPU
#include "ecpu/ecpu.h"
#endif

#if VGLOBAL_ECPU_MODE != TEST_ECPU
t_cpu vcpu;
#endif

void vcpuInit()
{
	vcpuinsInit();
#if VGLOBAL_ECPU_MODE != TEST_VCPU
	ecpuInit();
#endif
}
void vcpuReset()
{
	memset(&vcpu, 0, sizeof(t_cpu));

	vcpu.eip = 0x0000fff0;
	vcpu.eflags = 0x00000002;

	//vcpu.cs.base = 0xffff0000;
	vcpu.cs.base = 0x000f0000;
	vcpu.cs.dpl = 0x00;
	vcpu.cs.limit = 0xffffffff;
	vcpu.cs.seg.accessed = 1;
	vcpu.cs.seg.executable = 1;
	vcpu.cs.seg.exec.conform = 0;
	vcpu.cs.seg.exec.defsize = 0;
	vcpu.cs.seg.exec.readable = 1;
	vcpu.cs.selector = 0xf000;
	vcpu.cs.sregtype = SREG_CODE;

	vcpu.ss.base = 0x00000000;
	vcpu.ss.dpl = 0x00;
	vcpu.ss.limit = 0x0000ffff;
	vcpu.ss.seg.accessed = 1;
	vcpu.cs.seg.executable = 0;
	vcpu.ss.seg.data.big = 0;
	vcpu.ss.seg.data.expdown = 0;
	vcpu.ss.seg.data.writable = 1;
	vcpu.ss.selector = 0x0000;
	vcpu.ss.sregtype = SREG_STACK;

	vcpu.ds.base = 0x00000000;
	vcpu.ds.dpl = 0x00;
	vcpu.ds.limit = 0xffffffff;//0x0000ffff; // TEST ONLY
	vcpu.ds.seg.accessed = 1;
	vcpu.cs.seg.executable = 0;
	vcpu.ds.seg.data.big = 0;
	vcpu.ds.seg.data.expdown = 0;
	vcpu.ds.seg.data.writable = 1;
	vcpu.ds.selector = 0x0000;
	vcpu.ds.sregtype = SREG_DATA;
	vcpu.gs = vcpu.fs = vcpu.es = vcpu.ds;

	vcpu.ldtr.base = 0x00000000;
	vcpu.ldtr.dpl = 0x00;
	vcpu.ldtr.limit = 0x0000ffff;
	vcpu.ldtr.selector = 0x0000;
	vcpu.ldtr.sregtype = SREG_LDTR;
	vcpu.ldtr.sys.type = VCPU_DESC_SYS_TYPE_LDT;

	vcpu.tr.base = 0x00000000;
	vcpu.tr.dpl = 0x00;
	vcpu.tr.limit = 0x0000ffff;
	vcpu.tr.selector = 0x0000;
	vcpu.tr.sregtype = SREG_TR;
	vcpu.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_16_AVL;

	_LoadIDTR16(0x000000, 0x03ff);
	_LoadGDTR32(0x00000000, 0xffff);

	vcpuinsReset();
#if VGLOBAL_ECPU_MODE != TEST_VCPU
	ecpuReset();
#endif
}
void vcpuRefresh()
{
#if VGLOBAL_ECPU_MODE == TEST_BOTH
	ecpuRefreshInit();
#endif
#if VGLOBAL_ECPU_MODE != TEST_VCPU
	ecpuRefresh();
#endif
#if VGLOBAL_ECPU_MODE != TEST_ECPU
	vcpuinsRefresh();
#endif
#if VGLOBAL_ECPU_MODE == TEST_BOTH
	ecpuRefreshFinal();
#endif
}
void vcpuFinal()
{
#if VGLOBAL_ECPU_MODE != TEST_VCPU
	ecpuFinal();
#endif
	vcpuinsFinal();
}
