/* This file is a part of NXVM project. */

#include "debug/record.h"

#include "vapi.h"
#include "vcpuapi.h"

static t_cpu bcpu, oldbcpu;
static t_bool flagbrec;

/* DEBUGGING OPTIONS ******************************************************* */
#define VCPUAPI_COMPARE 1
#define VCPUAPI_RECORD  1
/* ************************************************************************* */

#ifdef VGLOBAL_BOCHS
static t_bool flagvalid = 0;
#define NEED_CPU_REG_SHORTCUTS 1
#include "d:/bochs-2.6/bochs.h"
#include "d:/bochs-2.6/cpu/cpu.h"

void vapiCallBackMachineStop() {}
void vapiSleep(t_nubit32 milisec) {}

static void PrintReg(t_cpu *rcpu)
{
	t_cpu oldcpu = vcpu;
	vcpu = *rcpu;
	vapiCallBackCpuPrintReg();
	vcpu = oldcpu;
}
static void PrintSreg(t_cpu *rcpu)
{
	t_cpu oldcpu = vcpu;
	vcpu = *rcpu;
	vapiCallBackCpuPrintSreg();
	vcpu = oldcpu;
}
static void PrintCreg(t_cpu *rcpu)
{
	t_cpu oldcpu = vcpu;
	vcpu = *rcpu;
	vapiCallBackCpuPrintCreg();
	vcpu = oldcpu;
}
static void LoadSreg(t_cpu_sreg *rsreg, bx_dbg_sreg_t *rbsreg)
{
	t_nubit64 cdesc;
	rsreg->selector = rbsreg->sel;
	rsreg->flagvalid = rbsreg->valid;
	cdesc = ((t_nubit64)rbsreg->des_h << 32) | rbsreg->des_l;
	rsreg->dpl = (t_nubit4)_GetDesc_DPL(cdesc);
	if (_IsDescUser(cdesc)) {
		rsreg->base = (t_nubit32)_GetDescSeg_Base(cdesc);
		rsreg->limit = (t_nubit32)((_IsDescSegGranularLarge(cdesc) ?
			((_GetDescSeg_Limit(cdesc) << 12) | 0x0fff) : (_GetDescSeg_Limit(cdesc))));
		rsreg->seg.accessed = _IsDescUserAccessed(cdesc);
		rsreg->seg.executable = _IsDescCode(cdesc);
		if (rsreg->seg.executable) {
			rsreg->seg.exec.conform = _IsDescCodeConform(cdesc);
			rsreg->seg.exec.defsize = _IsDescCode32(cdesc);
			rsreg->seg.exec.readable = _IsDescCodeReadable(cdesc);
		} else {
			rsreg->seg.data.big = _IsDescDataBig(cdesc);
			rsreg->seg.data.expdown = _IsDescDataExpDown(cdesc);
			rsreg->seg.data.writable = _IsDescDataWritable(cdesc);
		}
	} else {
		rsreg->base = (t_nubit32)_GetDescSeg_Base(cdesc);
		rsreg->limit = (t_nubit32)((_IsDescSegGranularLarge(cdesc) ?
				(_GetDescSeg_Limit(cdesc) << 12 | 0x0fff) : (_GetDescSeg_Limit(cdesc))));
		rsreg->sys.type = (t_nubit4)_GetDesc_Type(cdesc);
	}
}

static void CopyBochsCpu(t_cpu *rcpu)
{
	bx_dbg_sreg_t bsreg;
	bx_dbg_global_sreg_t bgsreg;
	rcpu->eax = EAX;
	rcpu->ecx = ECX;
	rcpu->edx = EDX;
	rcpu->ebx = EBX;
	rcpu->esp = ESP;
	rcpu->ebp = EBP;
	rcpu->esi = ESI;
	rcpu->edi = EDI;
	rcpu->eip = EIP;
	rcpu->eflags = BX_CPU_THIS_PTR read_eflags();

	rcpu->es.sregtype = SREG_DATA;
	BX_CPU_THIS_PTR dbg_get_sreg(&bsreg, 0);
	LoadSreg(&rcpu->es, &bsreg);

	rcpu->cs.sregtype = SREG_CODE;
	BX_CPU_THIS_PTR dbg_get_sreg(&bsreg, 1);
	LoadSreg(&rcpu->cs, &bsreg);

	rcpu->ss.sregtype = SREG_STACK;
	BX_CPU_THIS_PTR dbg_get_sreg(&bsreg, 2);
	LoadSreg(&rcpu->ss, &bsreg);

	rcpu->ds.sregtype = SREG_DATA;
	BX_CPU_THIS_PTR dbg_get_sreg(&bsreg, 3);
	LoadSreg(&rcpu->ds, &bsreg);

	rcpu->fs.sregtype = SREG_DATA;
	BX_CPU_THIS_PTR dbg_get_sreg(&bsreg, 4);
	LoadSreg(&rcpu->fs, &bsreg);

	rcpu->gs.sregtype = SREG_DATA;
	BX_CPU_THIS_PTR dbg_get_sreg(&bsreg, 5);
	LoadSreg(&rcpu->gs, &bsreg);

	rcpu->ldtr.sregtype = SREG_LDTR;
	BX_CPU_THIS_PTR dbg_get_ldtr(&bsreg);
	LoadSreg(&rcpu->ldtr, &bsreg);
	
	rcpu->tr.sregtype = SREG_TR;
	BX_CPU_THIS_PTR dbg_get_tr(&bsreg);
	LoadSreg(&rcpu->tr, &bsreg);

	rcpu->gdtr.sregtype = SREG_GDTR;
	BX_CPU_THIS_PTR dbg_get_gdtr(&bgsreg);
	rcpu->gdtr.base = GetMax32(bgsreg.base);
	rcpu->gdtr.limit = bgsreg.limit;

	rcpu->idtr.sregtype = SREG_IDTR;
	BX_CPU_THIS_PTR dbg_get_idtr(&bgsreg);
	rcpu->idtr.base = GetMax32(bgsreg.base);
	rcpu->idtr.limit = bgsreg.limit;
	
	rcpu->cr0 = BX_CPU_THIS_PTR cr0.get32();
	rcpu->cr2 = GetMax32(BX_CPU_THIS_PTR cr2);
	rcpu->cr3 = GetMax32(BX_CPU_THIS_PTR cr3);
}
static t_bool CheckDiff()
{
	t_nubitcc i;
	t_bool flagdiff = 0x00;
	t_nubit32 mask = VCPU_EFLAGS_RESERVED | vcpuins.udf;
	if (!vcpu.flagignore) {
		if (vcpu.cr0 != bcpu.cr0) {vapiPrint("diff cr0\n");flagdiff = 0x01;}
		if (vcpu.cr2 != bcpu.cr2) {vapiPrint("diff cr2\n");flagdiff = 0x01;}
		if (vcpu.cr3 != bcpu.cr3) {vapiPrint("diff cr3\n");flagdiff = 0x01;}
		if (vcpu.eax != bcpu.eax) {vapiPrint("diff eax\n");flagdiff = 0x01;}
		if (vcpu.ebx != bcpu.ebx) {vapiPrint("diff ebx\n");flagdiff = 0x01;}
		if (vcpu.ecx != bcpu.ecx) {vapiPrint("diff ecx\n");flagdiff = 0x01;}
		if (vcpu.edx != bcpu.edx) {vapiPrint("diff edx\n");flagdiff = 0x01;}
		if (vcpu.esp != bcpu.esp) {vapiPrint("diff esp\n");flagdiff = 0x01;}
		if (vcpu.ebp != bcpu.ebp) {vapiPrint("diff ebp\n");flagdiff = 0x01;}
		if (vcpu.esi != bcpu.esi) {vapiPrint("diff esi\n");flagdiff = 0x01;}
		if (vcpu.edi != bcpu.edi) {vapiPrint("diff edi\n");flagdiff = 0x01;}
		if (vcpu.eip != bcpu.eip) {vapiPrint("diff eip\n");flagdiff = 0x01;}
		if (vcpu.es.selector != bcpu.es.selector ||
			(vcpu.es.flagvalid && (
			vcpu.es.base != bcpu.es.base ||
			vcpu.es.limit != bcpu.es.limit ||
			vcpu.es.dpl != bcpu.es.dpl))) {
				vapiPrint("diff es (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
					vcpu.es.selector, vcpu.es.base, vcpu.es.limit, vcpu.es.dpl,
					bcpu.es.selector, bcpu.es.base, bcpu.es.limit, bcpu.es.dpl);
				flagdiff = 0x01;
		}
		if (vcpu.cs.selector != bcpu.cs.selector ||
			vcpu.cs.base != bcpu.cs.base ||
			vcpu.cs.limit != bcpu.cs.limit ||
			vcpu.cs.dpl != bcpu.cs.dpl) {
				vapiPrint("diff cs (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
					vcpu.cs.selector, vcpu.cs.base, vcpu.cs.limit, vcpu.cs.dpl,
					bcpu.cs.selector, bcpu.cs.base, bcpu.cs.limit, bcpu.cs.dpl);
				flagdiff = 0x01;
		}
		if (vcpu.ss.selector != bcpu.ss.selector ||
			vcpu.ss.base != bcpu.ss.base ||
			vcpu.ss.limit != bcpu.ss.limit ||
			vcpu.ss.dpl != bcpu.ss.dpl) {
				vapiPrint("diff ss (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
					vcpu.ss.selector, vcpu.ss.base, vcpu.ss.limit, vcpu.ss.dpl,
					bcpu.ss.selector, bcpu.ss.base, bcpu.ss.limit, bcpu.ss.dpl);
				flagdiff = 0x01;
		}
		if (vcpu.ds.selector != bcpu.ds.selector ||
			(vcpu.ds.flagvalid && (
			vcpu.ds.base != bcpu.ds.base ||
			vcpu.ds.limit != bcpu.ds.limit ||
			vcpu.ds.dpl != bcpu.ds.dpl))) {
				vapiPrint("diff ds (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
					vcpu.ds.selector, vcpu.ds.base, vcpu.ds.limit, vcpu.ds.dpl,
					bcpu.ds.selector, bcpu.ds.base, bcpu.ds.limit, bcpu.ds.dpl);
				flagdiff = 0x01;
		}
		if (vcpu.fs.selector != bcpu.fs.selector ||
			(vcpu.fs.flagvalid && (
			vcpu.fs.base != bcpu.fs.base ||
			vcpu.fs.limit != bcpu.fs.limit ||
			vcpu.fs.dpl != bcpu.fs.dpl))) {
				vapiPrint("diff fs (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
					vcpu.fs.selector, vcpu.fs.base, vcpu.fs.limit, vcpu.fs.dpl,
					bcpu.fs.selector, bcpu.fs.base, bcpu.fs.limit, bcpu.fs.dpl);
				flagdiff = 0x01;
		}
		if (vcpu.gs.selector != bcpu.gs.selector ||
			(vcpu.gs.flagvalid && (
			vcpu.gs.base != bcpu.gs.base ||
			vcpu.gs.limit != bcpu.gs.limit ||
			vcpu.gs.dpl != bcpu.gs.dpl))) {
				vapiPrint("diff gs (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
					vcpu.gs.selector, vcpu.gs.base, vcpu.gs.limit, vcpu.gs.dpl,
					bcpu.gs.selector, bcpu.gs.base, bcpu.gs.limit, bcpu.gs.dpl);
				flagdiff = 0x01;
		}
		if (vcpu.tr.selector != bcpu.tr.selector ||
			vcpu.tr.base != bcpu.tr.base ||
			vcpu.tr.limit != bcpu.tr.limit ||
			vcpu.tr.dpl != bcpu.tr.dpl) {
				vapiPrint("diff tr (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
					vcpu.tr.selector, vcpu.tr.base, vcpu.tr.limit, vcpu.tr.dpl,
					bcpu.tr.selector, bcpu.tr.base, bcpu.tr.limit, bcpu.tr.dpl);
				flagdiff = 0x01;
		}
		if (vcpu.ldtr.selector != bcpu.ldtr.selector ||
			vcpu.ldtr.base != bcpu.ldtr.base ||
			vcpu.ldtr.limit != bcpu.ldtr.limit ||
			vcpu.ldtr.dpl != bcpu.ldtr.dpl) {
				vapiPrint("diff ldtr (V=%04X/%08X/%08X/%1X, B=%04X/%08X/%08X/%1X)\n",
					vcpu.ldtr.selector, vcpu.ldtr.base, vcpu.ldtr.limit, vcpu.ldtr.dpl,
					bcpu.ldtr.selector, bcpu.ldtr.base, bcpu.ldtr.limit, bcpu.ldtr.dpl);
				flagdiff = 0x01;
		}
		if (vcpu.gdtr.base != bcpu.gdtr.base ||
			vcpu.gdtr.limit != bcpu.gdtr.limit) {
				vapiPrint("diff gdtr (V=%08X/%08X, B=%08X/%08X)\n",
					vcpu.gdtr.base, vcpu.gdtr.limit,
					bcpu.gdtr.base, bcpu.gdtr.limit);
				flagdiff = 1;
		}
		if (vcpu.idtr.base != bcpu.idtr.base ||
			vcpu.idtr.limit != bcpu.idtr.limit) {
				vapiPrint("diff idtr (V=%08X/%08X, B=%08X/%08X)\n",
					vcpu.idtr.base, vcpu.idtr.limit,
					bcpu.idtr.base, bcpu.idtr.limit);
				flagdiff = 1;
		}
		if ((vcpu.eflags & ~mask) != (bcpu.eflags & ~mask)) {
			vapiPrint("diff flags: V=%08X, B=%08X\n", vcpu.eflags, bcpu.eflags);
			flagdiff = 1;
		}
		if (vcpuins.except) flagdiff = 1;
	}
	if (flagdiff) {
		vapiPrint("BEFORE EXECUTION:\n");
		PrintReg(&oldbcpu);
		PrintSreg(&oldbcpu);
		PrintCreg(&oldbcpu);
		vapiPrint("---------------------------------------------------\n");
		vapiPrint("AFTER EXECUTION:\n");
		vapiPrint("CURRENT BCPU:\n");
		for (i = 0;i < bcpu.msize;++i) {
			vapiPrint("[%c:L%08x/%1d/%08x]\n",
				bcpu.mem[i].flagwrite ? 'W' : 'R', bcpu.mem[i].linear,
				bcpu.mem[i].byte, bcpu.mem[i].data);
		}
		PrintReg(&bcpu);
		PrintSreg(&bcpu);
		PrintCreg(&bcpu);
		vapiPrint("---------------------------------------------------\n");
		vapiPrint("CURRENT VCPU:\n");
		vapiPrint("[E:L%08X]\n", vcpu.linear);
		for (i = 0;i < vcpu.msize;++i) {
			vapiPrint("[%c:L%08x/%1d/%08x]\n",
				vcpu.mem[i].flagwrite ? 'W' : 'R', vcpu.mem[i].linear,
				vcpu.mem[i].byte, vcpu.mem[i].data);
		}
		PrintReg(&vcpu);
		PrintSreg(&vcpu);
		PrintCreg(&vcpu);
		vapiPrint("---------------------------------------------------\n");
	}
	return flagdiff;
}
static void PrintPhysical(t_nubit32 physical, t_vaddrcc rdata, t_nubit8 byte, t_bool write)
{
	vapiPrint("%s phy=%08x, data=", write ? "write" : "read", physical);
	switch (byte) {
	case 1: vapiPrint("%02x",    d_nubit8(rdata)); break;
	case 2: vapiPrint("%04x",    d_nubit16(rdata));break;
	case 3: vapiPrint("%08x",    d_nubit24(rdata));break;
	case 4: vapiPrint("%08x",    d_nubit32(rdata));break;
	case 6: vapiPrint("%016llx", d_nubit48(rdata));break;
	case 8: vapiPrint("%016llx", d_nubit64(rdata));break;
	default:vapiPrint("invalid");break;}
	vapiPrint(", byte=%01x\n", byte);
}
void vcpuapiReadPhysical(t_nubit32 physical, t_vaddrcc rdata, t_nubit8 byte)
{
	BX_CPU_THIS_PTR access_read_physical(physical, byte, (void *)rdata);
	//BX_MEM(0)->readPhysicalPage(BX_CPU(0), physical,byte, (void *)rdata);
	if (0) PrintPhysical(physical, rdata, byte, 0);
}
void vcpuapiWritePhysical(t_nubit32 physical, t_vaddrcc rdata, t_nubit8 byte)
{
	//BX_CPU_THIS_PTR access_write_physical(physical, byte, (void *)rdata);
	//BX_MEM(0)->writePhysicalPage(BX_CPU(0), physical, byte, (void *)rdata);
	if (0) PrintPhysical(physical, rdata, byte, 1);
}

void vcpuapiInit()
{
	vcpuInit();
	bcpu = vcpu;
}
void vcpuapiFinal()
{
	vcpuFinal();
}
void vcpuapiExecBefore()
{
	CopyBochsCpu(&bcpu); /* get init value */
	bcpu.reccs = bcpu.cs.selector;
	bcpu.receip = bcpu.eip;
	bcpu.linear = bcpu.cs.base + bcpu.eip;
	flagbrec = 0;

#if VCPUAPI_RECORD == 1
	if (vcpuinsReadLinear(bcpu.linear, (t_vaddrcc)bcpu.opcodes, 15))
		bcpu.oplen = 0;
	else bcpu.oplen = 15;
#endif
/* 0xa78f, 0x2eab */
	if (bcpu.linear == 0x7c00) {
		flagvalid = 1;
		vapiPrint("NXVM and Bochs comparison starts here.\n");
#if VCPUAPI_RECORD == 1
		vrecord.start = 0;
		vrecord.size = 0;
#endif
	}
	if (bcpu.linear == 0x2eab) {
		flagvalid = 0;
		vapiPrint("NXVM and Bochs comparison stops here.\n");
		BX_CPU_THIS_PTR magic_break = 1;
#if VCPUAPI_RECORD == 1
		recordDump("d:/bx.log");
#endif
	}
	if (flagvalid) {
		vcpu = bcpu;
#if VCPUAPI_COMPARE == 1
		vcpuinsRefresh();
#endif
	}
	oldbcpu = bcpu;
	bcpu.msize = 0;
#if VCPUAPI_RECORD == 1
	flagbrec = 1;
#endif
}
void vcpuapiExecAfter()
{
	if (flagvalid) {
		CopyBochsCpu(&bcpu);
#if VCPUAPI_RECORD == 1
		recordExec(&bcpu);
#endif
#if VCPUAPI_COMPARE == 1
		if (CheckDiff()) BX_CPU_THIS_PTR magic_break = 1;
#endif
	}
}
void vcpuapiMemRec(t_nubit32 linear, t_vaddrcc rdata, t_nubit8 byte, t_bool write)
{
	t_nubitcc i;
	t_nubit64 cdata = 0;
	if (flagbrec) {
		bcpu.mem[bcpu.msize].byte = byte;
		for (i = 0;i < byte;++i)
			d_nubit8(GetRef(cdata) + i) = d_nubit8(rdata + i);
		bcpu.mem[bcpu.msize].data = cdata;
		bcpu.mem[bcpu.msize].linear = linear;
		bcpu.mem[bcpu.msize].flagwrite = write;
		bcpu.msize++;
	}
}
#endif
