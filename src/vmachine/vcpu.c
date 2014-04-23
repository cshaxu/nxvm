/* This file is a part of NXVM project. */

#include "memory.h"

#include "vcpuins.h"
#include "vcpu.h"

t_cpu vcpu;

void vcpuInit()
{
	vcpuinsInit();
}
void vcpuReset()
{
	memset(&vcpu, 0, sizeof(t_cpu));

	vcpu.eip = 0x0000fff0;
	vcpu.eflags = 0x00000002;

	vcpu.cs.base = 0xffff0000;
	vcpu.cs.dpl = 0x00;
	vcpu.cs.limit = 0xffffffff;
	vcpu.cs.seg.accessed = 1;
	vcpu.cs.seg.executable = 1;
	vcpu.cs.seg.exec.conform = 0;
	vcpu.cs.seg.exec.defsize = 0;
	vcpu.cs.seg.exec.readable = 1;
	vcpu.cs.selector = 0xf000;
	vcpu.cs.sregtype = SREG_CODE;
	vcpu.cs.flagvalid = 1;

	vcpu.ss.base = 0x00000000;
	vcpu.ss.dpl = 0x00;
	vcpu.ss.limit = 0x0000ffff;
	vcpu.ss.seg.accessed = 1;
	vcpu.ss.seg.executable = 0;
	vcpu.ss.seg.data.big = 0;
	vcpu.ss.seg.data.expdown = 0;
	vcpu.ss.seg.data.writable = 1;
	vcpu.ss.selector = 0x0000;
	vcpu.ss.sregtype = SREG_STACK;
	vcpu.ss.flagvalid = 1;

	vcpu.ds.base = 0x00000000;
	vcpu.ds.dpl = 0x00;
	vcpu.ds.limit = 0x0000ffff;
	vcpu.ds.seg.accessed = 1;
	vcpu.ss.seg.executable = 0;
	vcpu.ds.seg.data.big = 0;
	vcpu.ds.seg.data.expdown = 0;
	vcpu.ds.seg.data.writable = 1;
	vcpu.ds.selector = 0x0000;
	vcpu.ds.sregtype = SREG_DATA;
	vcpu.ds.flagvalid = 1;
	vcpu.gs = vcpu.fs = vcpu.es = vcpu.ds;

	vcpu.ldtr.base = 0x00000000;
	vcpu.ldtr.dpl = 0x00;
	vcpu.ldtr.limit = 0x0000ffff;
	vcpu.ldtr.selector = 0x0000;
	vcpu.ldtr.sregtype = SREG_LDTR;
	vcpu.ldtr.sys.type = VCPU_DESC_SYS_TYPE_LDT;
	vcpu.ldtr.flagvalid = 1;

	vcpu.tr.base = 0x00000000;
	vcpu.tr.dpl = 0x00;
	vcpu.tr.limit = 0x0000ffff;
	vcpu.tr.selector = 0x0000;
	vcpu.tr.sregtype = SREG_TR;
	vcpu.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_16_AVL;
	vcpu.tr.flagvalid = 1;

	vcpu.idtr.base = 0x00000000;
	vcpu.idtr.limit = 0x03ff;
	vcpu.idtr.sregtype = SREG_IDTR;
	vcpu.idtr.flagvalid = 1;

	vcpu.gdtr.base = 0x00000000;
	vcpu.gdtr.limit = 0xffff;
	vcpu.gdtr.sregtype = SREG_GDTR;
	vcpu.gdtr.flagvalid = 1;

	vcpuinsReset();
}
void vcpuRefresh()
{
	vcpuinsRefresh();
}
void vcpuFinal()
{
	vcpuinsFinal();
}
