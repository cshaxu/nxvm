/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include "vapi.h"
#include "vport.h"
#include "vram.h"
#include "vpic.h"
#include "bios/qdbios.h"
#include "vcpuins.h"

#include "debug/aasm.h"
#include "debug/dasm.h"

/*
 * no protected mode
 * no task switch / gates
 * no segmentation
 * no paging
 */

#define i386(n) if (1)
#define VCPUINS_TRACE 1

#define _a_ _Arith_
#define _e_ _Execute_
#define _s_ _Segment_
#define _d_ _Decode_
#define _m_ _Memory_
#define _p_ _Port_
#define _c_ _Check_

#define done static void
#define todo static void /* need to implement */
#define tots static void /* need to test */


#if VCPUINS_TRACE == 1

typedef struct {
	char* blockstack[0x100];
	t_nubit8 bid;
	t_nubit8 cid;
} t_cpuins_trace_block;
typedef struct {
	t_cpuins_trace_block callstack[0x100];
	t_nubit8 cid;
} t_cpuins_trace_call;
t_cpuins_trace_call trace;

static void _Trace_Print_Call()
{
	t_nsbitcc i;
	for (i = 0;i < trace.callstack[trace.cid - 1].bid;++i) {
		vapiPrint("%s",trace.callstack[trace.cid - 1].blockstack[i]);
		if (i != trace.callstack[trace.cid - 1].bid - 1) vapiPrint("::");
	}
	vapiPrint("\n");
}
static void _Trace_Init()
{
	trace.cid = 0x00;
}
static void _Trace_Release(t_bool print)
{
	t_nsbitcc i, j;
	for (i = trace.cid - 1;i >= 0;--i) {
		trace.callstack[i];
		if (print) {
			for (j = 0;j < trace.callstack[i].bid;++j) {
				vapiPrint("%s",trace.callstack[i].blockstack[j]);
				if (j != trace.callstack[i].bid - 1) vapiPrint("::");
			}
			vapiPrint("\n");
		}
	}
	trace.cid = 0;
}
static void _Trace_Call_Begin(t_string s)
{
	if (vcpuins.except) _Trace_Release(1);
	if (trace.cid == 0xff) return;
//	vapiPrint("enter call(%d): %s\n", trace.cid, s);
	trace.callstack[trace.cid].blockstack[0] = s;
	trace.callstack[trace.cid].bid = 1;
	trace.callstack[trace.cid].cid = trace.cid;
	trace.cid++;
}
static void _Trace_Call_End()
{
	if (vcpuins.except) _Trace_Release(1);
	if (!trace.cid) return;
	trace.cid--;
//	vapiPrint("leave call(%d): %s\n", trace.cid, trace.callstack[trace.cid].blockstack[0]);
	if (trace.callstack[trace.cid].bid != 1 || trace.callstack[trace.cid].cid != trace.cid) {
		vapiPrint("\nunbalanced call stack (call = %d, block = %d):\n",
			trace.cid, trace.callstack[trace.cid].bid);
		trace.cid++;
		vcpuins.except |= 0xffffffff;
	}
}
static void _Trace_Block_Begin(t_string s)
{
	if (vcpuins.except) _Trace_Release(1);
	if (trace.callstack[trace.cid - 1].bid < 0xff) {
//		vapiPrint("enter block(%d): %s\n", trace.callstack[trace.cid - 1].bid, s);
		trace.callstack[trace.cid - 1].blockstack[trace.callstack[trace.cid - 1].bid++] = s;
	} else {
		vapiPrint("_Trace_Block_Begin(\"%s\")::failed", s);
	}
}
static void _Trace_Block_End()
{
	if (vcpuins.except) _Trace_Release(1);
	if (trace.callstack[trace.cid - 1].bid) {
		trace.callstack[trace.cid - 1].bid--;
//		vapiPrint("leave block(%d): %s\n", trace.callstack[trace.cid - 1].bid, trace.callstack[trace.cid - 1].blockstack[trace.callstack[trace.cid - 1].bid]);
	} else {
		vapiPrint("_Trace_Block_End()::failed");
	}
}

#define _cb(s) _Trace_Call_Begin(s)
#define _ce    _Trace_Call_End()
#define _bb(s) _Trace_Block_Begin(s)
#define _be    _Trace_Block_End()
#define _chb(n) if (1) {(n);if (vcpuins.except) {_Trace_Release(1);break;}} else
#define _chk(n) if (1) {(n);if (vcpuins.except) {_Trace_Release(1);return;}} else
#define _chr(n) if (1) {(n);if (vcpuins.except) {_Trace_Release(1);return 0;}} else
#define _newins_ if (1) {vapiPrint("NEWINS: ");_Trace_Print_Call();} else
#define _comment(s) if (1) {vapiPrint("COMMENT: %s\n", (s));} else
#else
#define _cb(s)
#define _ce
#define _be
#define _bb(s)
#define _chb(n) if (1) {(n);if (vcpuins.except) break;} else
#define _chk(n) if (1) {(n);if (vcpuins.except) return;} else
#define _chr(n) if (1) {(n);if (vcpuins.except) return 0;} else
#define _newins_
#define _comment
#endif

t_cpuins vcpuins;

/* stack pointer size */
#define _GetStackSize   (vcpu.ss.seg.data.big ? 4 : 2)
/* operand size */
#define _GetOperandSize ((vcpu.cs.seg.exec.defsize ^ vcpuins.prefix_oprsize) ? 4 : 2)
/* address size of the source operand */
#define _GetAddressSize ((vcpu.cs.seg.exec.defsize ^ vcpuins.prefix_addrsize) ? 4 : 2)
/* if opcode indicates a prefix */
static t_bool IsPrefix(t_nubit8 opcode)
{
	switch (opcode) {
	case 0xf0: case 0xf2: case 0xf3:
	case 0x2e: case 0x36: case 0x3e: case 0x26:
	case 0x64: case 0x65: case 0x66: case 0x67:
				return 0x01;break;
	default:	return 0x00;break;
	}
}

#define _SetExcept_PF(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_PF), vcpuins.excode = (n), vapiPrint("#PF(%x) - page fault\n", (n)))
#define _SetExcept_GP(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_GP), vcpuins.excode = (n), vapiPrint("#GP(%x) - general protect\n", (n)))
#define _SetExcept_SS(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_SS), vcpuins.excode = (n), vapiPrint("#SS(%x) - stack segment\n", (n)))
#define _SetExcept_UD(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_UD), vcpuins.excode = (n), vapiPrint("#UD(%x) - undefined\n", (n)))
#define _SetExcept_NP(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_NP), vcpuins.excode = (n), vapiPrint("#NP(%x) - not present\n", (n)))
#define _SetExcept_BR(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_BR), vcpuins.excode = (n), vapiPrint("#BR(%x) - boundary\n", (n)))
#define _SetExcept_TS(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_TS), vcpuins.excode = (n), vapiPrint("#TS(%x) - task state\n", (n)))
#define _SetExcept_DE(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_DE), vcpuins.excode = (n), vapiPrint("#DE(%x) - divide error\n", (n)))
#define _SetExcept_NM(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_NM), vcpuins.excode = (n), vapiPrint("#NM(%x) - divide error\n", (n)))

#define _SetExcept_CE(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_CE), vcpuins.excode = (n), vapiPrint("#CE(%x) - internal case error\n", (n)), vapiCallBackMachineStop())
#define _impossible_     _chk(_SetExcept_CE(1))
#define _impossible_r_   _chr(_SetExcept_CE(1))

/* memory management unit */
/* kernel memory accessing */
/* get physical from linear by paging */
static t_nubit32 _kma_addr_physical_linear(t_nubit32 linear, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force)
{
	/* TODO: paging mechanism not implemented */
	t_nubit32 physical = 0x00000000;
	_cb("_kma_addr_physical_linear");
	_chr(physical = linear);
	_ce;
	return physical;
}
/* get linear from logical by segmentation */
static t_nubit32 _kma_addr_linear_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force)
{
	/* TODO: segmentation mechanism not implemented */
	t_nubit32 linear = 0x00000000;
	_cb("_kma_addr_linear_logical");
	_chr(linear = (rsreg->base + offset));
	if (linear > rsreg->limit) {
		_comment("Bad translation: beyond segment limit");
		_comment("base:");
		_SetExcept_CE(rsreg->base);
		_comment("limit:");
		_SetExcept_CE(rsreg->limit);
		_comment("selector:");
		_SetExcept_CE(rsreg->selector);
		_comment("offset:");
		_SetExcept_CE(offset);
		_comment("linear:");
		_SetExcept_CE(linear);
		_chr(0);
	}
	if (linear != ((rsreg->selector << 4) + GetMax16(offset))) {
		_comment("Bad translation: logical -> linear");
		_comment("base:");
		_SetExcept_CE(rsreg->base);
		_comment("limit:");
		_SetExcept_CE(rsreg->limit);
		_comment("selector:");
		_SetExcept_CE(rsreg->selector);
		_comment("offset:");
		_SetExcept_CE(offset);
		_comment("linear:");
		_SetExcept_CE(linear);
		_chr(0);
	}
	_ce;
	return linear;
}
/* get physical from logical */
static t_nubit32 _kma_addr_physical_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force)
{
	t_nubit32 linear = 0x00000000;
	t_nubit32 physical = 0x00000000;
	_cb("_kma_addr_physical_logical");
	_chr(linear = _kma_addr_linear_logical(rsreg, offset, byte, write, vpl, force));
	_chr(physical = _kma_addr_physical_linear(linear, byte, write, vpl, force));
	_ce;
	return physical;
}
/* reference from physical */
static t_vaddrcc _kma_ref_physical(t_nubit32 physical)
{
	t_vaddrcc ref = 0;
	_cb("_kma_ref_physical");
	_chr(ref = vramAddr(physical));
	_ce;
	return ref;
}
/* reference from linear */
static t_vaddrcc _kma_ref_linear(t_nubit32 linear, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force)
{
	t_vaddrcc ref = 0;
	t_nubit32 physical = 0x00000000;
	_cb("_kma_ref_linear");
	_chr(physical = _kma_addr_physical_linear(linear, byte, write, vpl, force));
	_chr(ref = _kma_ref_physical(physical));
	_ce;
	return ref;
}
/* reference from logical */
static t_vaddrcc _kma_ref_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force)
{
	t_vaddrcc ref = 0;
	t_nubit32 physical = 0x00000000;
	_cb("_kma_ref_logical");
	_chr(physical = _kma_addr_physical_logical(rsreg, offset, byte, write, vpl, force));
	_chr(ref = _kma_ref_physical(physical));
	_ce;
	return ref;
}
/* read content from reference */
static t_nubit64 _kma_read_ref(t_vaddrcc ref, t_nubit8 byte)
{
	t_nubit64 result = 0x0000000000000000;
	_cb("_kma_read_ref");
	switch (byte) {
	case 0: break;
	case 1: result = d_nubit8(ref);break;
	case 2: result = d_nubit16(ref);break;
	case 4: result = d_nubit32(ref);break;
	case 6: result = GetMax48(d_nubit64(ref));break;
	case 8: result = d_nubit64(ref);break;
	default: _bb("byte");
		_chr(_SetExcept_CE(byte));
		_be;break;
	}
	_ce;
	return result;
}
/* read content from physical */
static t_nubit64 _kma_read_physical(t_nubit32 physical, t_nubit8 byte)
{
	t_vaddrcc ref = 0;
	t_nubit64 result = 0x0000000000000000;
	_cb("_kma_read_physical");
	_chr(ref = _kma_ref_physical(physical));
	_chr(result = _kma_read_ref(ref, byte));
	return result;
}
/* read content from linear */
static t_nubit64 _kma_read_linear(t_nubit32 linear, t_nubit8 byte, t_nubit8 vpl, t_bool force)
{
	t_vaddrcc ref = 0;
	t_nubit64 result = 0x0000000000000000;
	_cb("_kma_read_linear");
	_chr(ref = _kma_ref_linear(linear, byte, 0, vpl, force));
	_chr(result = _kma_read_ref(ref, byte));
	_ce;
	return result;
}
/* read content from logical */
static t_nubit64 _kma_read_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_nubit8 vpl, t_bool force)
{
	t_vaddrcc ref = 0;
	t_nubit64 result = 0x0000000000000000;
	_cb("_kma_read_logical");
	_chr(ref = _kma_ref_logical(rsreg, offset, byte, 0, vpl, force));
	_chr(result = _kma_read_ref(ref, byte));
	_ce;
	return result;
}
/* write content to reference */
static void _kma_write_ref(t_vaddrcc ref, t_nubit64 data, t_nubit8 byte)
{
	_cb("_kma_write_ref");
	switch (byte) {
	case 0: break;
	case 1: d_nubit8(ref) = GetMax8(data);break;
	case 2: d_nubit16(ref) = GetMax16(data);break;
	case 4: d_nubit32(ref) = GetMax32(data);break;
	case 6:
		d_nubit32(ref) = GetMax32(data);
		d_nubit16(ref + 4) = GetMax16(data >> 32);
		break;
	case 8: d_nubit64(ref) = GetMax64(data);break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_ce;
}
/* write content to physical */
static void _kma_write_physical(t_nubit32 physical, t_nubit64 data, t_nubit8 byte)
{
	t_vaddrcc ref = 0;
	_cb("_kma_write_physical");
	_chk(ref = _kma_ref_physical(physical));
	_chk(_kma_write_ref(ref, data, byte));
	_ce;
}
/* write content to linear */
static void _kma_write_linear(t_nubit32 linear, t_nubit64 data, t_nubit8 byte, t_nubit8 vpl, t_bool force)
{
	t_vaddrcc ref = 0;
	_cb("_kma_write_linear");
	_chk(ref = _kma_ref_linear(linear, byte, 1, vpl, force));
	_chk(_kma_write_ref(ref, data, byte));
	_ce;
}
/* write content to logical */
static void _kma_write_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit64 data, t_nubit8 byte, t_nubit8 vpl, t_bool force)
{
	t_vaddrcc ref = 0;
	_cb("_kma_write_logical");
	_chk(ref = _kma_ref_logical(rsreg, offset, byte, 1, vpl, force));
	_chk(_kma_write_ref(ref, data, byte));
	_ce;
}
/* general memory accessing */
static t_nubit32 _m_addr_physical_linear(t_nubit32 linear, t_nubit8 byte, t_bool write)
{
	t_nubit32 physical = 0x00000000;
	_cb("_m_addr_physical_linear");
	_chr(physical = _kma_addr_physical_linear(linear, byte, write, _GetCPL, 0));
	_ce;
	return physical;
}
static t_nubit32 _m_addr_linear_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write)
{
	t_nubit32 linear = 0x00000000;
	_cb("_m_addr_linear_logical");
	_chr(linear = _kma_addr_linear_logical(rsreg, offset, byte, write, _GetCPL, 0));
	_ce;
	return linear;
}
static t_nubit32 _m_addr_physical_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write)
{
	t_nubit32 physical = 0x00000000;
	_cb("_m_addr_physical_logical");
	_chr(physical = _kma_addr_physical_logical(rsreg, offset, byte, write, _GetCPL, 0));
	_ce;
	return physical;
}
static t_vaddrcc _m_ref_physical(t_nubit32 physical)
{
	t_vaddrcc ref = 0;
	_cb("_m_ref_physical");
	_chr(ref = _kma_ref_physical(physical));
	_ce;
	return ref;
}
static t_vaddrcc _m_ref_linear(t_nubit32 linear, t_nubit8 byte, t_bool write)
{
	t_vaddrcc ref = 0;
	_cb("_kma_ref_linear");
	_chr(ref = _kma_ref_linear(linear, byte, write, _GetCPL, 0));
	_ce;
	return ref;
}
static t_vaddrcc _m_ref_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write)
{
	t_vaddrcc ref = 0;
	_cb("_m_ref_logical");
	_chr(ref = _kma_ref_logical(rsreg, offset, byte, write, _GetCPL, 0));
	_ce;
	return ref;
}
static t_nubit64 _m_read_ref(t_vaddrcc ref, t_nubit8 byte)
{
	t_nubit64 result = 0x0000000000000000;
	_cb("_m_read_physical");
	_chr(result = _kma_read_ref(ref, byte));
	_ce;
	return result;
}
static t_nubit64 _m_read_physical(t_nubit32 physical, t_nubit8 byte)
{
	t_nubit64 result = 0x0000000000000000;
	_cb("_m_read_physical");
	_chr(result = _kma_read_physical(physical, byte));
	_ce;
	return result;
}
static t_nubit64 _m_read_linear(t_nubit32 linear, t_nubit8 byte)
{
	t_nubit64 result = 0x0000000000000000;
	_cb("_m_read_linear");
	_chr(result = _kma_read_linear(linear, byte, _GetCPL, 0));
	_ce;
	return result;
}
static t_nubit64 _m_read_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit8 byte)
{
	t_nubit64 result = 0x0000000000000000;
	_cb("_m_read_logical");
	_chr(result = _kma_read_logical(rsreg, offset, byte, _GetCPL, 0));
	_ce;
	return result;
}
done _m_write_ref(t_vaddrcc ref, t_nubit64 data, t_nubit8 byte)
{
	_cb("_m_write_physical");
	_chk(_kma_write_ref(ref, data, byte));
	_ce;
}
done _m_write_physical(t_nubit32 physical, t_nubit64 data, t_nubit8 byte)
{
	_cb("_m_write_physical");
	_chk(_kma_write_physical(physical, data, byte));
	_ce;
}
done _m_write_linear(t_nubit32 linear, t_nubit32 offset, t_nubit64 data, t_nubit8 byte)
{
	_cb("_m_write_linear");
	_chk(_kma_write_linear(linear, data, byte, _GetCPL, 0));
	_ce;
}
done _m_write_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit64 data, t_nubit8 byte)
{
	_cb("_m_write_logical");
	_chk(_kma_write_logical(rsreg, offset, data, byte, _GetCPL, 0));
	_ce;
}
done _m_test_linear(t_nubit32 linear, t_nubit8 byte, t_bool write)
{
	t_vaddrcc ref = 0;
	_cb("_m_test_linear");
	_chk(ref = _kma_ref_linear(linear, byte, write, _GetCPL, 0));
	_ce;
}
done _m_test_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write)
{
	t_vaddrcc ref = 0;
	_cb("_m_test_logical");
	_chk(ref = _kma_ref_logical(rsreg, offset, byte, write, _GetCPL, 0));
	_ce;
}

/* segment accessing unit: _s_ */
/* kernel segment accessing */
tots _ksa_descriptor_gdt(t_nubit16 selector, t_bool write)
{
	_cb("_ksa_descriptor_gdt");
	if (_GetSelector_TI(selector)) {
		_bb("Selector_TI(1)");
		_chk(_SetExcept_GP(selector));
		_be;
	}
	if (!_GetSelector_Index(selector)) {
		_bb("Selector(null)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	if ((_GetSelector_Offset(selector) + 7) > _GetGDTR_Limit) {
		_bb("Selector_Offset(>GDTR_Limit)");
		_chk(_SetExcept_GP(selector));
		_be;
	}
	vcpuins.ldesc = (t_nubit32)(_GetGDTR_Base + _GetSelector_Offset(selector));
	_chk(vcpuins.pdesc = _kma_addr_physical_linear(vcpuins.ldesc, 8, write, 0, 1));
	_chk(vcpuins.rdesc = _kma_ref_physical(vcpuins.pdesc));
	_chk(vcpuins.cdesc = _kma_read_ref(vcpuins.rdesc, 8));
	_ce;
}
tots _ksa_descriptor_ldt(t_nubit16 selector, t_bool write)
{
	_cb("_ksa_descriptor_ldt");
	if (!_GetSelector_TI(selector)) {
		_bb("Selector_TI(0)");
		_chk(_SetExcept_GP(selector));
		_be;
	}
	if ((_GetSelector_Offset(selector) + 7) > (t_nubit16)_GetLDTR_Limit) {
		_bb("Selector_Offset(>LDTR_Limit)");
		_chk(_SetExcept_GP(selector));
		_be;
	}
	vcpuins.ldesc = _GetLDTR_Base + _GetSelector_Offset(selector);
	_chk(vcpuins.pdesc = _kma_addr_physical_linear(vcpuins.ldesc, 8, write, 0, 1));
	_chk(vcpuins.rdesc = _kma_ref_physical(vcpuins.pdesc));
	_chk(vcpuins.cdesc = _kma_read_ref(vcpuins.rdesc, 8));
	_ce;
}
tots _ksa_descriptor(t_nubit16 selector, t_bool write)
{
	_cb("_ksa_descriptor");
	if (_GetSelector_TI(selector))
		_chk(_ksa_descriptor_ldt(selector, write));
	else
		_chk(_ksa_descriptor_gdt(selector, write));
	_ce;
}
tots _ksa_load_seg(t_cpu_sreg *rsreg, t_nubit16 selector)
{
	_cb("_ksa_load_seg");
	switch (rsreg->sregtype) {
	case SREG_CODE:
		/* note: privilege checking not performed */
		_bb("sregtype(SREG_CODE)");
		if (_IsProtected) {
			_bb("Protected(1)");
			if (_IsSelectorNull(selector)) {
				_bb("selector(null)");
				_chk(_SetExcept_GP(0));
				_be;
			}
			_chk(_ksa_descriptor(selector, 1));
			if (!_IsDescCode(vcpuins.cdesc)) {
				_bb("DescUser_Type(!Code)");
				_chk(_SetExcept_GP(selector));
				_be;
			}
			if (!_IsDescPresent(vcpuins.cdesc)) {
				_bb("Desc_P(0)");
				_chk(_SetExcept_NP(selector));
				_be;
			}
			_SetDescUserAccessed(d_nubit64(vcpuins.rdesc));
			_chk(vcpuins.cdesc = (t_nubit64)_m_read_ref(vcpuins.rdesc, 8));
			rsreg->selector = selector;
			rsreg->base = (t_nubit32)_GetDescSeg_Base(vcpuins.cdesc);
			rsreg->dpl = (t_nubit4)_GetDesc_DPL(vcpuins.cdesc);
			rsreg->limit = (t_nubit32)(rsreg->base +
				(_IsDescSegGranularLarge(vcpuins.cdesc) ?
				((_GetDescSeg_Limit(vcpuins.cdesc) << 12) | 0x0fff) : (_GetDescSeg_Limit(vcpuins.cdesc))));
			rsreg->seg.accessed = (t_bool)_IsDescUserAccessed(vcpuins.cdesc);
			rsreg->seg.executable = (t_bool)_IsDescUserExecutable(vcpuins.cdesc);
			rsreg->seg.exec.defsize = (t_bool)_IsDescCode32(vcpuins.cdesc);
			rsreg->seg.exec.conform = (t_bool)_IsDescCodeConform(vcpuins.cdesc);
			rsreg->seg.exec.readable = (t_bool)_IsDescCodeReadable(vcpuins.cdesc);
			_be;
		} else {
			rsreg->selector = selector;
			rsreg->limit -= rsreg->base;
			rsreg->base = (selector << 4);
			rsreg->limit += rsreg->base;
		}
		_be;break;
	case SREG_DATA:
		_bb("sregtype(SREG_DATA)");
		if (_IsProtected) {
			_bb("Protected(1)");
			if (_IsSelectorNull(selector)) {
				_bb("selector(null)");
				rsreg->selector = selector;
				_be;
			} else {
				_bb("selector(!null)");
				_chk(_ksa_descriptor(selector, 1));
				if (!_IsDescData(vcpuins.cdesc) && !_IsDescCodeReadable(vcpuins.cdesc)) {
					_bb("DescUser_Type(!Data,!CodeReadable)");
					_chk(_SetExcept_GP(selector));
					_be;
				}
				if (_IsDescData(vcpuins.cdesc) || _IsDescCodeNonConform(vcpuins.cdesc)) {
					_bb("DescUser_Type(Data/CodeNonConform)");
					if (_GetSelector_RPL(selector) > _GetDesc_DPL(vcpuins.cdesc) ||
						_GetCPL > _GetDesc_DPL(vcpuins.cdesc)) {
						_bb("PL(fail)");
						_chk(_SetExcept_GP(selector));
						_be;
					}
					_be;
				}
				if (!_IsDescPresent(vcpuins.cdesc)) {
					_bb("Desc_P(0)");
					_chk(_SetExcept_NP(selector));
					_be;
				}
				_SetDescUserAccessed(d_nubit64(vcpuins.rdesc));
				_chk(vcpuins.cdesc = (t_nubit64)_m_read_ref(vcpuins.rdesc, 8));
				rsreg->selector = selector;
				rsreg->base = (t_nubit32)_GetDescSeg_Base(vcpuins.cdesc);
				rsreg->dpl = (t_nubit4)_GetDesc_DPL(vcpuins.cdesc);
				rsreg->limit = (t_nubit32)(rsreg->base +
					(_IsDescSegGranularLarge(vcpuins.cdesc) ?
					((_GetDescSeg_Limit(vcpuins.cdesc) << 12) | 0x0fff) : (_GetDescSeg_Limit(vcpuins.cdesc))));
				rsreg->seg.accessed = (t_bool)_IsDescUserAccessed(vcpuins.cdesc);
				rsreg->seg.executable = (t_bool)_IsDescUserExecutable(vcpuins.cdesc);
				if (rsreg->seg.executable) {
					rsreg->seg.exec.defsize = (t_bool)_IsDescCode32(vcpuins.cdesc);
					rsreg->seg.exec.conform = (t_bool)_IsDescCodeConform(vcpuins.cdesc);
					rsreg->seg.exec.readable = (t_bool)_IsDescCodeReadable(vcpuins.cdesc);
				} else {
					rsreg->seg.data.big = (t_bool)_IsDescDataBig(vcpuins.cdesc);
					rsreg->seg.data.expdown = (t_bool)_IsDescDataExpDown(vcpuins.cdesc);
					rsreg->seg.data.writable = (t_bool)_IsDescDataWritable(vcpuins.cdesc);
				}
				_be;
			}
			_be;
		} else {
			rsreg->selector = selector;
			rsreg->limit -= rsreg->base;
			rsreg->base = (selector << 4);
			rsreg->limit += rsreg->base;
		}
		_be;break;
	case SREG_STACK:
		_bb("sregtype(SREG_STACK)");
		if (_IsProtected) {
			_bb("Protected(1)");
			if (_IsSelectorNull(selector)) {
				_bb("selector(null)");
				_chk(_SetExcept_GP(0));
				_be;
			}
			if (_GetSelector_RPL(selector) != _GetCPL) {
				_bb("Selector_RPL(!CPL)");
				_chk(_SetExcept_GP(selector));
				_be;
			}
			_chk(_ksa_descriptor(selector, 1));
			if (!_IsDescDataWritable(vcpuins.cdesc)) {
				_bb("vcpuins.cdesc(!DATASEG/!TYPE_W)");
				_chk(_SetExcept_GP(selector));
				_be;
			}
			if (_GetDesc_DPL(vcpuins.cdesc) != _GetCPL) {
				_bb("Desc_DPL(!CPL)");
				_chk(_SetExcept_GP(selector));
				_be;
			}
			if (!_IsDescPresent(vcpuins.cdesc)) {
				_bb("Desc_P(0)");
				_chk(_SetExcept_SS(selector));
				_be;
			}
			_SetDescUserAccessed(d_nubit64(vcpuins.rdesc));
			_chk(vcpuins.cdesc = (t_nubit64)_m_read_ref(vcpuins.rdesc, 8));
			rsreg->selector = selector;
			rsreg->base = (t_nubit32)_GetDescSeg_Base(vcpuins.cdesc);
			rsreg->dpl = (t_nubit4)_GetDesc_DPL(vcpuins.cdesc);
			rsreg->limit = (t_nubit32)(rsreg->base +
				(_IsDescSegGranularLarge(vcpuins.cdesc) ?
				((_GetDescSeg_Limit(vcpuins.cdesc) << 12) | 0x0fff) : (_GetDescSeg_Limit(vcpuins.cdesc))));
			rsreg->seg.accessed = (t_bool)_IsDescUserAccessed(vcpuins.cdesc);
			rsreg->seg.executable = (t_bool)_IsDescUserExecutable(vcpuins.cdesc);
			rsreg->seg.data.big = (t_bool)_IsDescDataBig(vcpuins.cdesc);
			rsreg->seg.data.expdown = (t_bool)_IsDescDataExpDown(vcpuins.cdesc);
			rsreg->seg.data.writable = (t_bool)_IsDescDataWritable(vcpuins.cdesc);
			_be;
		} else {
			rsreg->selector = selector;
			rsreg->limit -= rsreg->base;
			rsreg->base = (selector << 4);
			rsreg->limit += rsreg->base;
		}
		_be;break;
	case SREG_TR:
		_bb("sregtype(SREG_TR)");
		if (_GetCPL) {
			_bb("CPL(!0)");
			_chk(_SetExcept_GP(0));
			_be;
		}
		_chk(_ksa_descriptor_gdt(selector, 1));
		if (_IsDescTSSAvl(vcpuins.cdesc)) {
			_bb("descriptor(!TSSAvl)");
			_chk(_SetExcept_GP(selector));
			_be;
		}
		if (!_IsDescPresent(vcpuins.cdesc)) {
			_bb("descriptor(!P)");
			_chk(_SetExcept_NP(selector));
			_be;
		}
		_SetDescTSSBusy(d_nubit64(vcpuins.rdesc));
		_chk(vcpuins.cdesc = (t_nubit64)_m_read_ref(vcpuins.rdesc, 8));
		rsreg->selector = selector;
		rsreg->base = (t_nubit32)_GetDescSeg_Base(vcpuins.cdesc);
		rsreg->dpl = (t_nubit4)_GetDesc_DPL(vcpuins.cdesc);
		rsreg->limit = (t_nubit32)(rsreg->base + \
			(_IsDescSegGranularLarge(vcpuins.cdesc) ? \
				(_GetDescSeg_Limit(vcpuins.cdesc) << 12 | 0x0fff) : \
				(_GetDescSeg_Limit(vcpuins.cdesc))));
		rsreg->sys.type = (t_nubit4)_GetDesc_Type(vcpuins.cdesc);
		_be;break;
	case SREG_LDTR:
		_bb("sregtype(SREG_LDTR)");
		if (_GetCPL) {
			_bb("CPL(!0)");
			_chk(_SetExcept_GP(0));
			_be;
		}
		_chk(_ksa_descriptor_gdt(selector, 0));
		if (!_IsDescLDT(vcpuins.cdesc)) {
			_bb("descriptor(!LDT)");
			_chk(_SetExcept_GP(selector));
			_be;
		}
		if (!_IsDescPresent(vcpuins.cdesc)) {
			_bb("descriptor(!P)");
			_chk(_SetExcept_NP(selector));
			_be;
		}
		rsreg->selector = selector;
		rsreg->base = (t_nubit32)_GetDescSeg_Base(vcpuins.cdesc);
		rsreg->dpl = (t_nubit4)_GetDesc_DPL(vcpuins.cdesc);
		rsreg->limit = (t_nubit32)(rsreg->base + \
			(_IsDescSegGranularLarge(vcpuins.cdesc) ? \
				(_GetDescSeg_Limit(vcpuins.cdesc) << 12 | 0x0fff) : \
				(_GetDescSeg_Limit(vcpuins.cdesc))));
		rsreg->sys.type = (t_nubit4)_GetDesc_Type(vcpuins.cdesc);
		_be;break;
	default:_impossible_;break;}
	_ce;
}
/* regular segment accessing */
/* 0 = succ, 1 = fail */
static t_bool _s_check_selector(t_nubit16 selector)
{
	_cb("_s_check_selector");
	if (_IsSelectorNull(selector)) {
		_ce;
		return 1;
	}
	if ((_GetSelector_Offset(selector) + 7) >
		(_GetSelector_TI(selector) ? _GetLDTR_Limit : _GetGDTR_Limit)) {
		_ce;
		return 1;
	}
	_ce;
	return 0;
}
done _s_descriptor(t_nubit16 selector, t_bool write)
{
	_cb("_s_read_descriptor");
	_chk(_ksa_descriptor(selector, write));
	_ce;
}
static t_nubit64 _s_read_idt(t_nubit8 intid)
{
	/* TODO: protected mode idt read not implemented */
	t_nubit64 result = 0x0000000000000000;
	t_vaddrcc ref = 0;
	_cb("_s_read_idt");
	if (!_GetCR0_PE) {
		_bb("CR0_PE(0)");
		if ((intid * 4 + 3) > _GetIDTR_Limit) {
			_bb("intid(>idtr.limit)");
			_chr(_SetExcept_GP(0));
			_be;
		}
		_chr(ref = (t_vaddrcc)_m_ref_linear(((t_nubit32)_GetIDTR_Base + intid * 4), 4, 0));
		_chr(result = GetMax32(_m_read_ref(ref, 4)));
		_be;
	} else {
		_bb("CR0_PE(1)");
		_newins_;
		_comment("_s_read_idt::Protected: not implemented");
		_chr(_SetExcept_UD(0));
		_be;
	}
	_ce;
	return result;
}
static t_nubit64 _s_read_cs(t_nubit32 offset, t_nubit8 byte)
{
	t_nubit64 result = 0x0000000000000000;
	_cb("_s_read_cs");
	_chr(result = _kma_read_logical(&vcpu.cs, offset, byte, 0x00, 1));
	_ce;
	return result;
}
static t_nubit64 _s_read_ss(t_nubit32 offset, t_nubit8 byte)
{
	t_nubit64 result = 0x0000000000000000;
	_cb("_s_read_ss");
	_chr(result = _kma_read_logical(&vcpu.ss, offset, byte, _GetCPL, 0));
	_ce;
	return result;
}
todo _s_test_idt(t_nubit8 intid)
{
	/* TODO: protected mode idt limit check not implemented */
	_cb("_s_test_idt");
	if (!_GetCR0_PE) {
		_bb("CR0_PE(0)");
		if ((intid * 4 + 3) > _GetIDTR_Limit) {
			_bb("intid(>idtr.limit)");
			_chk(_SetExcept_GP(0));
			_be;
		}
		_be;
	} else {
		_bb("CR0_PE(1)");
		_newins_;
		_comment("_s_test_idt::Protected: not implemented");
		_chk(_SetExcept_UD(0));
		_be;
	}
	_ce;
}
done _s_test_cs(t_nubit32 offset)
{
	_cb("_s_test_cs");
	_chk(_kma_addr_linear_logical(&vcpu.cs, offset, 0x01, 0, 0x00, 1));
	_ce;
}
done _s_test_ss_push(t_nubit8 byte)
{
	t_nubit32 cesp = 0x00000000;
	_cb("_s_test_ss_push");
	switch (_GetStackSize) {
	case 2: _bb("StackSize(2)");
		if (vcpu.sp && vcpu.sp < byte)
			_chk(_SetExcept_SS(0));
		_chk(_m_test_logical(&vcpu.ss, GetMax16(vcpu.sp - byte), byte, 1));
		_be;break;
	case 4: _bb("StackSize(4)");
		if (vcpu.esp && vcpu.esp < byte)
			_chk(_SetExcept_SS(0));
		_chk(_m_test_logical(&vcpu.ss, GetMax32(vcpu.esp - byte), byte, 1));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
done _s_test_ss_pop(t_nubit8 byte)
{
	t_nubit32 cesp = 0x00000000;
	_cb("_s_test_ss_pop");
	switch (_GetStackSize) {
	case 2: _bb("StackSize(2)");
		_chk(_m_test_logical(&vcpu.ss, vcpu.sp, byte, 0));
		_be;break;
	case 4: _bb("StackSize(4)");
		_chk(_m_test_logical(&vcpu.ss, vcpu.esp, byte, 0));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
done _s_load_seg(t_cpu_sreg *rsreg, t_nubit16 selector)
{
	_cb("_s_load_seg");
	_chk(_ksa_load_seg(rsreg, selector));
	_ce;
}
tots _s_load_gdtr(t_nubit32 base, t_nubit16 limit, t_nubit8 byte)
{
	_cb("_s_load_gdtr");
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	switch (byte) {
	case 2: _LoadGDTR16(base, limit);break;
	case 4: _LoadGDTR32(base, limit);break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;}
	_ce;
}
tots _s_load_idtr(t_nubit32 base, t_nubit16 limit, t_nubit8 byte)
{
	_cb("_s_load_idtr");
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	switch (byte) {
	case 2: _LoadIDTR16(base, limit);break;
	case 4: _LoadIDTR32(base, limit);break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;}
	_ce;
}
done _s_load_ldtr(t_nubit16 selector)
{
	_cb("_s_load_ldtr");
	_chk(_ksa_load_seg(&vcpu.ldtr, selector));
	_ce;
}
done _s_load_tr(t_nubit16 selector)
{
	_cb("_s_load_tr");
	_chk(_ksa_load_seg(&vcpu.tr, selector));
	_ce;
}
tots _s_load_cr0_msw(t_nubit16 msw)
{
	_cb("_s_load_cr0_msw");
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	if (!_GetCR0_PE) {
		_bb("CR0_PE(0)");
		vcpu.cr0 = (vcpu.cr0 & 0xfffffff0) | (msw & 0x000f);
		_be;
	} else {
		_bb("CR0_PE(1)");
		vcpu.cr0 = (vcpu.cr0 & 0xfffffff0) | (msw & 0x000e) | 0x01;
		_be;
	}
	_ce;
}
done _s_load_cs(t_nubit16 newcs)
{
	_cb("_s_load_cs");
	_chk(_ksa_load_seg(&vcpu.cs, newcs));
	_ce;
}
done _s_load_ss(t_nubit16 newss)
{
	_cb("_s_load_ss");
	_chk(_ksa_load_seg(&vcpu.ss, newss));
	_ce;
}
done _s_load_ds(t_nubit16 newds)
{
	_cb("_s_load_ds");
	_chk(_ksa_load_seg(&vcpu.ds, newds));
	_ce;
}
done _s_load_es(t_nubit16 newes)
{
	_cb("_s_load_es");
	_chk(_ksa_load_seg(&vcpu.es, newes));
	_ce;
}
done _s_load_fs(t_nubit16 newfs)
{
	_cb("_s_load_fs");
	_chk(_ksa_load_seg(&vcpu.fs, newfs));
	_ce;
}
done _s_load_gs(t_nubit16 newgs)
{
	_cb("_s_load_gs");
	_chk(_ksa_load_seg(&vcpu.gs, newgs));
	_ce;
}
done _s_test_eip()
{
	_cb("_s_test_eip");
	_chk(_kma_addr_linear_logical(&vcpu.cs, vcpu.eip, 0x01, 0, 0x00, 1));
	_ce;
}
done _s_test_esp()
{
	t_nubit32 cesp = 0x00000000;
	_cb("_s_test_esp");
	switch (_GetStackSize) {
	case 2: cesp = GetMax16(vcpu.esp);break;
	case 4: cesp = GetMax32(vcpu.esp);break;
	default:_impossible_;break;}
	_chk(_kma_addr_linear_logical(&vcpu.ss, cesp, 0x01, 0, 0x00, 1));
	_ce;
}

/* portid accessing unit */
/* kernel portid accessing */
todo _kpa_test_iomap(t_nubit16 portid, t_nubit8 byte)
{
	/* TODO: iomap tester not implemented */
	_cb("_kpa_test_iomap");
	_ce;
}
done _kpa_test_mode(t_nubit16 portid, t_nubit8 byte)
{
	_cb("_p_test");
	if (_GetCR0_PE && (_GetCPL > (t_nubit8)_GetEFLAGS_IOPL || _GetEFLAGS_VM)) {
		_bb("CR0_PE(1),(CPL>IOPL/EFLAGS_VM(1))");
		_chk(_kpa_test_iomap(portid, byte));
		_be;
	}
	_ce;
}
/* regular portid accessing */
static t_nubit32 _p_input(t_nubit16 portid, t_nubit8 byte)
{
	t_nubit32 result = 0x00000000;
	_cb("_p_input");
	_chr(_kpa_test_mode(portid, byte));
	ExecFun(vport.in[portid]);
	switch (byte) {
	case 1: result = vport.iobyte;break;
	case 2: result = vport.ioword;break;
	case 4: result = vport.iodword;break;
	default: _bb("byte");
		_chr(_SetExcept_CE(byte));
		_be;break;
	}
	_ce;
	return result;
}
done _p_output(t_nubit16 portid, t_nubit32 data, t_nubit8 byte)
{
	_cb("_p_output");
	_chk(_kpa_test_mode(portid, byte));
	switch (byte) {
	case 1: vport.iobyte = (t_nubit8)data;break;
	case 2: vport.ioword = (t_nubit16)data;break;
	case 4: vport.iodword = (t_nubit32)data;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	ExecFun(vport.out[portid]);
	_ce;
}

/* decoding unit */
#define _GetModRM_MOD(modrm) (((modrm) & 0xc0) >> 6)
#define _GetModRM_REG(modrm) (((modrm) & 0x38) >> 3)
#define _GetModRM_RM(modrm)  (((modrm) & 0x07) >> 0)
#define _GetSIB_SS(sib)      (((sib) & 0xc0) >> 6)
#define _GetSIB_Index(sib)   (((sib) & 0x38) >> 3)
#define _GetSIB_Base(sib)    (((sib) & 0x07) >> 0)
/* kernel decoding function */
done _kdf_skip(t_nubit8 byte)
{
	_cb("_kdf_skip");
	_chk(vcpu.eip += byte);
	_ce;
}
static t_nubit64 _kdf_code(t_nubit8 byte)
{
	t_nubit64 result = 0x0000000000000000;
	_cb("_kdf_code");
	_chr(result = _s_read_cs(vcpu.eip, byte));
	_chr(_kdf_skip(byte));
	_ce;
	return result;
}
tots _kdf_modrm(t_nubit8 regbyte, t_nubit8 rmbyte, t_bool write)
{
	t_nsbit8 disp8 = 0x00;
	t_nubit16 disp16 = 0x0000;
	t_nubit32 disp32 = 0x00000000;
	t_nubit32 sibindex = 0x00000000;
	t_nubit8 modrm = 0x00, sib = 0x00;
	_cb("_kdf_modrm");
	_chk(modrm = (t_nubit8)_kdf_code(1));
	vcpuins.erm = 0x00000000;
	vcpuins.flagmem = 1;
	vcpuins.flagmss = 0;
	vcpuins.crm = vcpuins.cr = 0x0000000000000000;
	vcpuins.rrm = vcpuins.rr = (t_vaddrcc)NULL;
	vcpuins.prm = 0x00000000;
	vcpuins.lrm = 0x00000000;
	switch (_GetAddressSize) {
	case 2: _bb("AddressSize(2)");
		switch (_GetModRM_MOD(modrm)) {
		case 0: _bb("ModRM_MOD(0)");
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.erm = GetMax16(vcpu.bx + vcpu.si); vcpuins.flagmss = 0;break;
			case 1: vcpuins.erm = GetMax16(vcpu.bx + vcpu.di); vcpuins.flagmss = 0;break;
			case 2: vcpuins.erm = GetMax16(vcpu.bp + vcpu.si); vcpuins.flagmss = 1;break;
			case 3: vcpuins.erm = GetMax16(vcpu.bp + vcpu.di); vcpuins.flagmss = 1;break;
			case 4: vcpuins.erm = GetMax16(vcpu.si); vcpuins.flagmss = 0;break;
			case 5: vcpuins.erm = GetMax16(vcpu.di); vcpuins.flagmss = 0;break;
			case 6: _bb("ModRM_RM(6)");
				_chk(disp16 = (t_nubit16)_kdf_code(2));
				vcpuins.erm = GetMax16(disp16); vcpuins.flagmss = 0;
				_be;break;
			case 7: vcpuins.erm = GetMax16(vcpu.bx); vcpuins.flagmss = 0;break;
			default:_impossible_;break;}
			_be;break;
		case 1: _bb("ModRM_MOD(1)");
			_chk(disp8 = (t_nsbit8)_kdf_code(1));
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.erm = GetMax16(vcpu.bx + vcpu.si + disp8); vcpuins.flagmss = 0;break;
			case 1: vcpuins.erm = GetMax16(vcpu.bx + vcpu.di + disp8); vcpuins.flagmss = 0;break;
			case 2: vcpuins.erm = GetMax16(vcpu.bp + vcpu.si + disp8); vcpuins.flagmss = 1;break;
			case 3: vcpuins.erm = GetMax16(vcpu.bp + vcpu.di + disp8); vcpuins.flagmss = 1;break;
			case 4: vcpuins.erm = GetMax16(vcpu.si + disp8); vcpuins.flagmss = 0;break;
			case 5: vcpuins.erm = GetMax16(vcpu.di + disp8); vcpuins.flagmss = 0;break;
			case 6: vcpuins.erm = GetMax16(vcpu.bp + disp8); vcpuins.flagmss = 1;break;
			case 7: vcpuins.erm = GetMax16(vcpu.bx + disp8); vcpuins.flagmss = 0;break;
			default:_impossible_;break;}
			_be;break;
		case 2: _bb("ModRM_MOD(2)");
			_chk(disp16 = (t_nubit16)_kdf_code(2));
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.erm = GetMax16(vcpu.bx + vcpu.si + disp16); vcpuins.flagmss = 0;break;
			case 1: vcpuins.erm = GetMax16(vcpu.bx + vcpu.di + disp16); vcpuins.flagmss = 0;break;
			case 2: vcpuins.erm = GetMax16(vcpu.bp + vcpu.si + disp16); vcpuins.flagmss = 1;break;
			case 3: vcpuins.erm = GetMax16(vcpu.bp + vcpu.di + disp16); vcpuins.flagmss = 1;break;
			case 4: vcpuins.erm = GetMax16(vcpu.si + disp16); vcpuins.flagmss = 0;break;
			case 5: vcpuins.erm = GetMax16(vcpu.di + disp16); vcpuins.flagmss = 0;break;
			case 6: vcpuins.erm = GetMax16(vcpu.bp + disp16); vcpuins.flagmss = 1;break;
			case 7: vcpuins.erm = GetMax16(vcpu.bx + disp16); vcpuins.flagmss = 0;break;
			default:_impossible_;break;}
			_be;break;
		case 3:
			break;
		default:
			_impossible_;
			break;
		}
		_be;break;
	case 4: _bb("AddressSize(4)");
		_newins_;
		if (_GetModRM_MOD(modrm) != 3 && _GetModRM_RM(modrm) == 4) {
			_bb("ModRM_MOD(!3),ModRM_RM(4)");
			_chk(sib = (t_nubit8)_kdf_code(1));
			switch (_GetSIB_Index(sib)) {
			case 0: sibindex = vcpu.eax;   break;
			case 1: sibindex = vcpu.ecx;   break;
			case 2: sibindex = vcpu.edx;   break;
			case 3: sibindex = vcpu.ebx;   break;
			case 4: sibindex = 0x00000000;break;
			case 5: sibindex = vcpu.ebp;   break;
			case 6: sibindex = vcpu.esi;   break;
			case 7: sibindex = vcpu.edi;   break;
			default:_impossible_;break;}
			sibindex <<= _GetSIB_SS(sib);
			_be;
		}
		switch (_GetModRM_MOD(modrm)) {
		case 0:
			_bb("ModRM_MOD(0)");
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.erm = vcpu.eax; vcpuins.flagmss = 0;break;
			case 1: vcpuins.erm = vcpu.ecx; vcpuins.flagmss = 0;break;
			case 2: vcpuins.erm = vcpu.edx; vcpuins.flagmss = 0;break;
			case 3: vcpuins.erm = vcpu.ebx; vcpuins.flagmss = 0;break;
			case 4:
				_bb("ModRM_RM(4)");
				switch (_GetSIB_Base(sib)) {
				case 0: vcpuins.erm = vcpu.eax + sibindex; vcpuins.flagmss = 0;break;
				case 1: vcpuins.erm = vcpu.ecx + sibindex; vcpuins.flagmss = 0;break;
				case 2: vcpuins.erm = vcpu.edx + sibindex; vcpuins.flagmss = 0;break;
				case 3: vcpuins.erm = vcpu.ebx + sibindex; vcpuins.flagmss = 0;break;
				case 4: vcpuins.erm = vcpu.esp + sibindex; vcpuins.flagmss = 1;break;
				case 5: _bb("SIB_Base(5)");
					_chk(disp32 = (t_nubit32)_kdf_code(4));
					vcpuins.erm = disp32 + sibindex; vcpuins.flagmss = 0; 
					_be;break;
				case 6: vcpuins.erm = vcpu.esi + sibindex; vcpuins.flagmss = 0;break;
				case 7: vcpuins.erm = vcpu.edi + sibindex; vcpuins.flagmss = 0;break;
				default:_impossible_;break;}
				_be;break;
			case 5: _bb("ModRM_RM(5)");
				_chk(disp32 = (t_nubit32)_kdf_code(4));
				vcpuins.erm = disp32; vcpuins.flagmss = 0;
				_be;break;
			case 6: vcpuins.erm = vcpu.esi; vcpuins.flagmss = 0;
			case 7: vcpuins.erm = vcpu.edi; vcpuins.flagmss = 0;
			default:_impossible_;break;}
			_be;break;
		case 1: _bb("ModRM_MOD(1)");
			_chk(disp8 = (t_nsbit8)_kdf_code(1));
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.erm = vcpu.eax + disp8; vcpuins.flagmss = 0;break;
			case 1: vcpuins.erm = vcpu.ecx + disp8; vcpuins.flagmss = 0;break;
			case 2: vcpuins.erm = vcpu.edx + disp8; vcpuins.flagmss = 0;break;
			case 3: vcpuins.erm = vcpu.ebx + disp8; vcpuins.flagmss = 0;break;
			case 4: _bb("ModRM_RM(4)");
				switch (_GetSIB_Base(sib)) {
				case 0: vcpuins.erm = vcpu.eax + sibindex + disp8; vcpuins.flagmss = 0;break;
				case 1: vcpuins.erm = vcpu.ecx + sibindex + disp8; vcpuins.flagmss = 0;break;
				case 2: vcpuins.erm = vcpu.edx + sibindex + disp8; vcpuins.flagmss = 0;break;
				case 3: vcpuins.erm = vcpu.ebx + sibindex + disp8; vcpuins.flagmss = 0;break;
				case 4: vcpuins.erm = vcpu.esp + sibindex + disp8; vcpuins.flagmss = 1;break;
				case 5: vcpuins.erm = vcpu.ebp + sibindex + disp8; vcpuins.flagmss = 1;break;
				case 6: vcpuins.erm = vcpu.esi + sibindex + disp8; vcpuins.flagmss = 0;break;
				case 7: vcpuins.erm = vcpu.edi + sibindex + disp8; vcpuins.flagmss = 0;break;
				default:_impossible_;break;}
				_be;break;
			case 5: vcpuins.erm = vcpu.ebp + disp8; vcpuins.flagmss = 1;break;
			case 6: vcpuins.erm = vcpu.esi + disp8; vcpuins.flagmss = 0;break;
			case 7: vcpuins.erm = vcpu.edi + disp8; vcpuins.flagmss = 0;break;
			default:_impossible_;break;}
			_be;break;
		case 2: _bb("ModRM_MOD(2)");
			_chk(disp32 = (t_nubit32)_kdf_code(4));
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.erm = vcpu.eax + disp32; vcpuins.flagmss = 0;break;
			case 1: vcpuins.erm = vcpu.ecx + disp32; vcpuins.flagmss = 0;break;
			case 2: vcpuins.erm = vcpu.edx + disp32; vcpuins.flagmss = 0;break;
			case 3: vcpuins.erm = vcpu.ebx + disp32; vcpuins.flagmss = 0;break;
			case 4: _bb("ModRM_RM(4)");
				switch (_GetSIB_Base(sib)) {
				case 0: vcpuins.erm = vcpu.eax + sibindex + disp32; vcpuins.flagmss = 0;break;
				case 1: vcpuins.erm = vcpu.ecx + sibindex + disp32; vcpuins.flagmss = 0;break;
				case 2: vcpuins.erm = vcpu.edx + sibindex + disp32; vcpuins.flagmss = 0;break;
				case 3: vcpuins.erm = vcpu.ebx + sibindex + disp32; vcpuins.flagmss = 0;break;
				case 4: vcpuins.erm = vcpu.esp + sibindex + disp32; vcpuins.flagmss = 1;break;
				case 5: vcpuins.erm = vcpu.ebp + sibindex + disp32; vcpuins.flagmss = 1;break;
				case 6: vcpuins.erm = vcpu.esi + sibindex + disp32; vcpuins.flagmss = 0;break;
				case 7: vcpuins.erm = vcpu.edi + sibindex + disp32; vcpuins.flagmss = 0;break;
				default:_impossible_;break;}
				_be;break;
			case 5: vcpuins.erm = vcpu.ebp + disp32; vcpuins.flagmss = 1;break;
			case 6: vcpuins.erm = vcpu.esi + disp32; vcpuins.flagmss = 0;break;
			case 7: vcpuins.erm = vcpu.edi + disp32; vcpuins.flagmss = 0;break;
			default:_impossible_;break;}
			_be;break;
		case 3:
			break;
		default:
			_impossible_;
			break;
		}
		_be;break;
	default:
		_impossible_;
		break;
	}
	if (_GetModRM_MOD(modrm) == 3) {
		_bb("ModRM_MOD(3)");
		vcpuins.flagmem = 0;
		switch (rmbyte) {
		case 1:
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.rrm = (t_vaddrcc)(&vcpu.al);break;
			case 1: vcpuins.rrm = (t_vaddrcc)(&vcpu.cl);break;
			case 2: vcpuins.rrm = (t_vaddrcc)(&vcpu.dl);break;
			case 3: vcpuins.rrm = (t_vaddrcc)(&vcpu.bl);break;
			case 4: vcpuins.rrm = (t_vaddrcc)(&vcpu.ah);break;
			case 5: vcpuins.rrm = (t_vaddrcc)(&vcpu.ch);break;
			case 6: vcpuins.rrm = (t_vaddrcc)(&vcpu.dh);break;
			case 7: vcpuins.rrm = (t_vaddrcc)(&vcpu.bh);break;
			default:_impossible_;break;}
			break;
		case 2:
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.rrm = (t_vaddrcc)(&vcpu.ax);break;
			case 1: vcpuins.rrm = (t_vaddrcc)(&vcpu.cx);break;
			case 2: vcpuins.rrm = (t_vaddrcc)(&vcpu.dx);break;
			case 3: vcpuins.rrm = (t_vaddrcc)(&vcpu.bx);break;
			case 4: vcpuins.rrm = (t_vaddrcc)(&vcpu.sp);break;
			case 5: vcpuins.rrm = (t_vaddrcc)(&vcpu.bp);break;
			case 6: vcpuins.rrm = (t_vaddrcc)(&vcpu.si);break;
			case 7: vcpuins.rrm = (t_vaddrcc)(&vcpu.di);break;
			default:_impossible_;break;}
			break;
		case 4:
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.rrm = (t_vaddrcc)(&vcpu.eax);break;
			case 1: vcpuins.rrm = (t_vaddrcc)(&vcpu.ecx);break;
			case 2: vcpuins.rrm = (t_vaddrcc)(&vcpu.edx);break;
			case 3: vcpuins.rrm = (t_vaddrcc)(&vcpu.ebx);break;
			case 4: vcpuins.rrm = (t_vaddrcc)(&vcpu.esp);break;
			case 5: vcpuins.rrm = (t_vaddrcc)(&vcpu.ebp);break;
			case 6: vcpuins.rrm = (t_vaddrcc)(&vcpu.esi);break;
			case 7: vcpuins.rrm = (t_vaddrcc)(&vcpu.edi);break;
			default:_impossible_;break;}
			break;
		default: _bb("rmbyte");
			_chk(_SetExcept_CE(rmbyte));
			_be;break;
		}
		_chk(vcpuins.crm = _m_read_ref(vcpuins.rrm, rmbyte));
		_be;
	}
	switch (regbyte) {
	case 0:
		/* reg is operation or segment */
		vcpuins.cr = _GetModRM_REG(modrm);
		break;
	case 1:
		switch (_GetModRM_REG(modrm)) {
		case 0: vcpuins.rr = (t_vaddrcc)(&vcpu.al);break;
		case 1: vcpuins.rr = (t_vaddrcc)(&vcpu.cl);break;
		case 2: vcpuins.rr = (t_vaddrcc)(&vcpu.dl);break;
		case 3: vcpuins.rr = (t_vaddrcc)(&vcpu.bl);break;
		case 4: vcpuins.rr = (t_vaddrcc)(&vcpu.ah);break;
		case 5: vcpuins.rr = (t_vaddrcc)(&vcpu.ch);break;
		case 6: vcpuins.rr = (t_vaddrcc)(&vcpu.dh);break;
		case 7: vcpuins.rr = (t_vaddrcc)(&vcpu.bh);break;
		default:_impossible_;break;}
		_chk(vcpuins.cr = _m_read_ref(vcpuins.rr, rmbyte));
		break;
	case 2:
		switch (_GetModRM_REG(modrm)) {
		case 0: vcpuins.rr = (t_vaddrcc)(&vcpu.ax);break;
		case 1: vcpuins.rr = (t_vaddrcc)(&vcpu.cx);break;
		case 2: vcpuins.rr = (t_vaddrcc)(&vcpu.dx);break;
		case 3: vcpuins.rr = (t_vaddrcc)(&vcpu.bx);break;
		case 4: vcpuins.rr = (t_vaddrcc)(&vcpu.sp);break;
		case 5: vcpuins.rr = (t_vaddrcc)(&vcpu.bp);break;
		case 6: vcpuins.rr = (t_vaddrcc)(&vcpu.si);break;
		case 7: vcpuins.rr = (t_vaddrcc)(&vcpu.di);break;
		default:_impossible_;break;}
		_chk(vcpuins.cr = _m_read_ref(vcpuins.rr, rmbyte));
		break;
	case 4:
		switch (_GetModRM_REG(modrm)) {
		case 0: vcpuins.rr = (t_vaddrcc)(&vcpu.eax);break;
		case 1: vcpuins.rr = (t_vaddrcc)(&vcpu.ecx);break;
		case 2: vcpuins.rr = (t_vaddrcc)(&vcpu.edx);break;
		case 3: vcpuins.rr = (t_vaddrcc)(&vcpu.ebx);break;
		case 4: vcpuins.rr = (t_vaddrcc)(&vcpu.esp);break;
		case 5: vcpuins.rr = (t_vaddrcc)(&vcpu.ebp);break;
		case 6: vcpuins.rr = (t_vaddrcc)(&vcpu.esi);break;
		case 7: vcpuins.rr = (t_vaddrcc)(&vcpu.edi);break;
		default:_impossible_;break;}
		_chk(vcpuins.cr = _m_read_ref(vcpuins.rr, rmbyte));
		break;
	default: _bb("regbyte");
		_chk(_SetExcept_CE(regbyte));
		_be;break;
	}
	_ce;
}
done _d_skip(t_nubit8 byte)
{
	_cb("_d_skip");
	_chk(_kdf_skip(byte));
	_ce;
}
static t_nubit64 _d_code(t_nubit8 byte)
{
	t_nubit64 result = 0x0000000000000000;
	_cb("_d_code");
	_chr(result = _kdf_code(byte));
	_ce;
	return result;
}
done _d_imm(t_nubit8 byte)
{
	_cb("_d_imm");
	vcpuins.eimm = vcpu.eip;
	_chk(vcpuins.limm = _kma_addr_linear_logical(&vcpu.cs, vcpuins.eimm, byte, 0x00, 0x00, 0x01));
	_chk(vcpuins.pimm = _kma_addr_physical_linear(vcpuins.limm, byte, 0x00, 0x00, 0x01));
	_chk(vcpuins.rimm = _kma_ref_physical(vcpuins.pimm));
	_chk(vcpuins.cimm = _kma_read_ref(vcpuins.rimm, byte));
	_chk(_d_skip(byte));
	_ce;
}
done _d_moffs(t_nubit8 byte, t_bool write)
{
	t_nubit32 offset = 0x00000000;
	_cb("_d_moffs");
	switch (_GetAddressSize) {
	case 2: _bb("AddressSize(2)");
		_chk(vcpuins.erm = GetMax16(_d_code(2)));
		_be;break;
	case 4: _bb("AddressSize(4)");
		_chk(vcpuins.erm = GetMax32(_d_code(4)));
		_be;break;
	default:_impossible_;break;}
	_chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpuins.erm, byte, write));
	_chk(vcpuins.prm = _m_addr_physical_linear(vcpuins.lrm, byte, write));
	_chk(vcpuins.rrm = _m_ref_physical(vcpuins.prm));
	_chk(vcpuins.crm = _m_read_ref(vcpuins.rrm, byte));
	_ce;
}
done _d_modrm_sreg(t_nubit8 rmbyte, t_bool write)
{
	_cb("_d_modrm_sreg");
	_chk(_kdf_modrm(0, rmbyte, write));
	if (vcpuins.flagmem) {
		_bb("flagmem(1)");
		_chk(vcpuins.lrm = _m_addr_linear_logical((vcpuins.flagmss ? vcpuins.roverss : vcpuins.roverds),
			vcpuins.erm, rmbyte, write));
		_chk(vcpuins.prm = _m_addr_physical_linear(vcpuins.lrm, rmbyte, write));
		_chk(vcpuins.rrm = _m_ref_physical(vcpuins.prm));
		_chk(vcpuins.crm = _m_read_ref(vcpuins.rrm, rmbyte));
		_be;
	}
	vcpuins.rmovsreg = NULL;
	switch (vcpuins.cr) {
	case 0: vcpuins.rmovsreg = &vcpu.es;break;
	case 1: vcpuins.rmovsreg = &vcpu.cs;break;
	case 2: vcpuins.rmovsreg = &vcpu.ss;break;
	case 3: vcpuins.rmovsreg = &vcpu.ds;break;
	case 4: vcpuins.rmovsreg = &vcpu.fs;break;
	case 5: vcpuins.rmovsreg = &vcpu.gs;break;
	default: _bb("vcpuins.cr");
		_chk(_SetExcept_UD(0));
		_be;break;
	}
	_ce;
}
done _d_modrm_ea(t_nubit8 regbyte, t_nubit8 rmbyte)
{
	t_nubit8 modrm = 0x00;
	_cb("_d_modrm_ea");
	_chk(_kdf_modrm(regbyte, rmbyte, 0));
	if (!vcpuins.flagmem) {
		_bb("flagmem(0)");
		_chk(_SetExcept_UD(0));
		_be;
	}
	_ce;
}
done _d_modrm(t_nubit8 regbyte, t_nubit8 rmbyte, t_bool write)
{
	t_nubit8 modrm = 0x00;
	_cb("_d_modrm");
	_chk(_kdf_modrm(regbyte, rmbyte, write));
	if (vcpuins.flagmem) {
		_bb("flagmem(1)");
		_chk(vcpuins.lrm = _m_addr_linear_logical((vcpuins.flagmss ? vcpuins.roverss : vcpuins.roverds),
			vcpuins.erm, rmbyte, write));
		_chk(vcpuins.prm = _m_addr_physical_linear(vcpuins.lrm, rmbyte, write));
		_chk(vcpuins.rrm = _m_ref_physical(vcpuins.prm));
		_chk(vcpuins.crm = _m_read_ref(vcpuins.rrm, rmbyte));
		_be;
	} else if (vcpu.flaglock) {
		_bb("flagmem(0),flaglock(1)");
		_chk(_SetExcept_UD(0));
		_be;
	}
	_ce;
}

/* execution control unit: _e_ */
/* kernel execution control */
tots _kec_push(t_vaddrcc rsrc, t_nubit8 byte)
{
	t_nubit64 data = 0x0000000000000000;
	_cb("_kec_push");
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_s_test_ss_push(2));
		data = d_nubit16(rsrc);
		_be;break;
	case 4: _bb("byte(4)");
		_newins_;
		_chk(_s_test_ss_push(4));
		data = d_nubit32(rsrc);
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	switch (_GetStackSize) {
	case 2: _bb("StackSize(2)");
		vcpu.sp -= byte;
		_chk(_m_write_logical(&vcpu.ss, vcpu.sp, data, byte));
		_be;break;
	case 4: _bb("StackSize(4)");
		_newins_;
		vcpu.esp -= byte;
		_chk(_m_write_logical(&vcpu.ss, vcpu.esp, data, byte));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
tots _kec_pop(t_vaddrcc rdest, t_nubit8 byte)
{
	t_nubit64 result = 0x0000000000000000;
	_cb("_kec_pop");
	switch (_GetStackSize) {
	case 2: _bb("StackSize(2)");
		_chk(result = _m_read_logical(&vcpu.ss, vcpu.sp, byte));
		vcpu.sp += byte;
		_be;break;
	case 4: _bb("StackSize(4)");
		_newins_;
		_chk(result = _m_read_logical(&vcpu.ss, vcpu.esp, byte));
		vcpu.esp += byte;
		_be;break;
	default:_impossible_;break;}
	switch (byte) {
	case 2: d_nubit16(rdest) = GetMax16(result);break;
	case 4: d_nubit32(rdest) = GetMax32(result);break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_ce;
}
tots _kec_call_far(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte)
{
	t_cpu_sreg ccs = vcpu.cs;
	t_nubit32 oldcs = vcpu.cs.selector;
	_cb("_kec_call_far");
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_s_test_ss_push(4));
		neweip = GetMax16(neweip);
		_chk(_ksa_load_seg(&ccs, newcs));
		_chk(_kma_addr_linear_logical(&ccs, neweip, 0x01, 0, 0x00, 1));
		_chk(_kec_push((t_vaddrcc)&oldcs, 2));
		_chk(_kec_push((t_vaddrcc)&vcpu.ip, 2));
		_be;break;
	case 4: _bb("byte(4)");
		_newins_;
		_chk(_s_test_ss_push(8));
		neweip = GetMax32(neweip);
		_chk(_ksa_load_seg(&ccs, newcs));
		_chk(_kma_addr_linear_logical(&ccs, neweip, 0x01, 0, 0x00, 1));
		_chk(_kec_push((t_vaddrcc)&oldcs, 4));
		_chk(_kec_push((t_vaddrcc)&vcpu.eip, 4));
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	vcpu.cs = ccs;
	vcpu.eip = neweip;
	_ce;
}
tots _kec_call_near(t_nubit32 neweip, t_nubit8 byte)
{
	_cb("_kec_call_near");
	switch (byte) {
	case 2: _bb("byte(2)");
		neweip = GetMax16(neweip);
		_chk(_s_test_cs(neweip));
		_chk(_kec_push((t_vaddrcc)&vcpu.ip, 2));
		_be;break;
	case 4: _bb("byte(4)");
		_newins_;
		neweip = GetMax32(neweip);
		_chk(_s_test_cs(neweip));
		_chk(_kec_push((t_vaddrcc)&vcpu.eip, 4));
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	vcpu.eip = neweip;
	_ce;
}
todo _kec_task_switch(t_nubit16 newtss);
done _kec_jmp_far(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte)
{
	t_cpu_sreg ccs = vcpu.cs;
	_cb("_kec_jmp_far");
	switch (byte) {
	case 2: neweip = GetMax16(neweip);break;
	case 4: neweip = GetMax32(neweip);break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_chk(_ksa_load_seg(&ccs, newcs));
	_chk(_kma_addr_linear_logical(&ccs, neweip, 0x01, 0, 0x00, 1));
	vcpu.cs = ccs;
	vcpu.eip = neweip;
	_ce;
}
done _kec_jmp_near(t_nubit32 neweip, t_nubit8 byte)
{
	_cb("_kec_jmp_near");
	switch (byte) {
	case 2: neweip = GetMax16(neweip);break;
	case 4: neweip = GetMax32(neweip);break;
	default: _bb("bit");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_chk(_s_test_cs(neweip));
	vcpu.eip = neweip;
	_ce;
}
tots _kec_ret_near(t_nubit16 parambyte, t_nubit8 byte)
{
	t_nubit32 neweip = 0x00000000;
	_cb("_kec_ret_near");
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_kec_pop((t_vaddrcc)&neweip, 2));
		_chk(_s_test_cs(GetMax16(neweip)));
		_chk(vcpu.eip = GetMax16(neweip));
		_be;break;
	case 4: _bb("byte(4)");
		_newins_;
		_chk(_kec_pop((t_vaddrcc)&neweip, 4));
		_chk(_s_test_cs(GetMax32(neweip)));
		_chk(vcpu.eip = GetMax32(neweip));
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	switch (_GetStackSize) {
	case 2: vcpu.sp += parambyte;break;
	case 4: vcpu.esp += parambyte;break;
	default:_impossible_;break;}
	_ce;
}
tots _kec_ret_far(t_nubit16 parambyte, t_nubit16 byte)
{
	t_cpu_sreg ccs = vcpu.cs;
	t_nubit32 neweip = 0x00000000;
	t_nubit32 newcs = 0x00000000;
	_cb("_kec_ret_far");
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_s_test_ss_pop(4));
		_chk(_kec_pop((t_vaddrcc)&neweip, 2));
		_chk(_kec_pop((t_vaddrcc)&newcs, 2));
		neweip = GetMax16(neweip);
		newcs = GetMax16(newcs);
		_be;break;
	case 4: _bb("byte(4)");
		_newins_;
		_chk(_s_test_ss_pop(8));
		_chk(_kec_pop((t_vaddrcc)&neweip, 4));
		_chk(_kec_pop((t_vaddrcc)&newcs, 4));
		neweip = GetMax32(neweip);
		newcs = GetMax16(newcs);
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_chk(_ksa_load_seg(&ccs, newcs));
	_chk(_kma_addr_linear_logical(&ccs, neweip, 0x01, 0, 0x00, 1));
	vcpu.cs = ccs;
	vcpu.eip = neweip;
	switch (_GetStackSize) {
	case 2: vcpu.sp += parambyte;break;
	case 4: vcpu.esp += parambyte;break;
	default:_impossible_;break;}
	_ce;
}
/* sub execution routine */
done _ser_call_far_real(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte)
{
	_cb("_ser_call_far_real");
	_chk(_kec_call_far(newcs, neweip, byte));
	_ce;
}
todo _ser_call_far_call_gate(t_nubit16 cgsel);
todo _ser_call_far_task_gate(t_nubit16 tgsel);
todo _ser_call_far_tss(t_nubit16 newtss);
done _ser_int_real(t_nubit8 intid, t_nubit8 byte)
{
	t_nubit32 oldcs = vcpu.cs.selector;
	t_nubit16 cip = 0x0000;
	t_nubit32 vector = 0x00000000;
	_cb("_ser_int_real");
	_chk(_s_test_idt(intid));
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_s_test_ss_push(6));
		_chk(_kec_push((t_vaddrcc)&vcpu.flags, 2));
		_ClrEFLAGS_IF;
		_ClrEFLAGS_TF;
		_chk(_kec_push((t_vaddrcc)&oldcs, 2));
		_chk(_kec_push((t_vaddrcc)&vcpu.ip, 2));
		_be;break;
	case 4: _bb("byte(4)");
		_newins_;
		_chk(_s_test_ss_push(12));
		_chk(_kec_push((t_vaddrcc)&vcpu.eflags, 4));
		_ClrEFLAGS_IF;
		_ClrEFLAGS_TF;
		_chk(_kec_push((t_vaddrcc)&oldcs, 4));
		_chk(_kec_push((t_vaddrcc)&vcpu.eip, 4));
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_chk(vector = (t_nubit32)_s_read_idt(intid));
	cip = GetMax16(vector);
	_chk(_s_test_cs(cip));
	vcpu.eip = cip;
	_chk(_s_load_cs(GetMax16(vector >> 16)));
	_ce;
}
done _ser_iret_real(t_nubit8 byte)
{
	t_nubit32 neweip = 0x00000000;
	t_nubit32 newcs = 0x00000000;
	t_nubit32 neweflags = 0x00000000;
	_cb("_ser_iret_real");
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_s_test_ss_pop(6));
		_chk(_kec_pop((t_vaddrcc)&neweip, 2));
		_chk(vcpu.eip = GetMax16(neweip));
		_chk(_kec_pop((t_vaddrcc)&newcs, 2));
		_chk(_s_load_cs(GetMax16(newcs)));
		_chk(_kec_pop((t_vaddrcc)&neweflags, 2));
		_chk(vcpu.eflags = (neweflags & 0x0000ffff) | (vcpu.eflags & 0xffff0000));
		_be;break;
	case 4: _bb("byte(4)");
		_newins_;
		_chk(_s_test_ss_pop(12));
		_chk(_kec_pop((t_vaddrcc)&neweip, 4));
		_chk(vcpu.eip = GetMax32(neweip));
		_chk(_kec_pop((t_vaddrcc)&newcs, 4));
		_chk(_s_load_cs(GetMax16(newcs)));
		_chk(_kec_pop((t_vaddrcc)&neweflags, 4));
		_chk(vcpu.eflags = (neweflags & 0x00257fd5) | (vcpu.eflags & 0x001a0000));
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_ce;
}
done _ser_jmp_far_real(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte)
{
	_cb("_ser_jmp_far_real");
	_chk(_kec_jmp_far(newcs, neweip, byte));
	_ce;
}
done _ser_ret_far_real(t_nubit16 parambyte, t_nubit16 byte)
{
	_cb("_ser_ret_far_real");
	_chk(_kec_ret_far(parambyte, byte));
	_ce;
}
/* regular execute control */
done _e_push(t_vaddrcc rsrc, t_nubit8 byte)
{
	_cb("_e_push");
	_chk(_kec_push(rsrc, byte));
	_ce;
}
done _e_pop(t_vaddrcc rdest, t_nubit8 byte)
{
	_cb("_e_pop");
	_chk(_kec_pop(rdest, byte));
	_ce;
}
todo _e_call_far(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte)
{
	_cb("_e_call_far");
	if (!_IsProtected) {
		_bb("!Protected");
		_chk(_ser_call_far_real(newcs, neweip, byte));
		_be;
	} else {
		_bb("Protected");
		_newins_;
		_comment("_e_call_far::Protected: not implemented");
		_chk(_SetExcept_CE(byte));
		_be;
	}
	_ce;
}
done _e_call_near(t_nubit32 neweip, t_nubit8 byte)
{
	_cb("_e_call_near");
	_chk(_kec_call_near(neweip, byte));
	_ce;
}
todo _e_int3(t_nubit8 byte)
{
	_cb("_e_int3");
	if (!_GetCR0_PE) {
		_bb("CR0_PE(0)");
		_chk(_ser_int_real(0x03, byte));
		_be;
	} else {
		_bb("CR0_PE(1)");
		_newins_;
		_comment("_e_int3::Protected: not implemented");
		_chk(_SetExcept_UD(0));
		_be;
	}
	_ce;
}
todo _e_into(t_nubit8 byte)
{
	_cb("_e_into");
	if (_GetEFLAGS_OF) {
		_bb("EFLAGS_OF(1)");
		if (!_GetCR0_PE) {
			_bb("CR0_PE(0)");
			_chk(_ser_int_real(0x04, byte));
			_be;
		} else {
			_bb("CR0_PE(1)");
			_newins_;
			_comment("_e_into::Protected: not implemented");
			_chk(_SetExcept_UD(0));
			_be;
		}
		_be;
	}
	_ce;
}
todo _e_int_n(t_nubit8 intid, t_nubit8 byte)
{
	_cb("_e_int_n");
	if (!_GetCR0_PE) {
		_bb("CR0_PE(0)");
		_chk(_ser_int_real(intid, byte));
		_be;
	} else {
		_bb("CR0_PE(1)");
		_newins_;
		_comment("_e_int_n::Protected: not implemented");
		_chk(_SetExcept_UD(0));
		_be;
	}
	_ce;
}
todo _e_iret(t_nubit8 byte)
{
	_cb("_e_iret");
	if (!_GetCR0_PE) {
		_bb("CR0_PE(0)");
		_chk(_ser_iret_real(byte));
		_be;
	} else {
		_bb("CR0_PE(1)");
		_newins_;
		_comment("_e_iret::Protected: not implemented");
		_chk(_SetExcept_UD(0));
		_be;
	}
	_ce;
}
tots _e_jcc(t_vaddrcc rsrc, t_nubit8 byte, t_bool condition)
{
	t_nubit32 neweip = vcpu.eip;
	_cb("_e_jcc");
	if (condition) {
		_bb("condition(1)");
		switch (byte) {
		case 1: neweip += d_nsbit8(rsrc);break;
		case 2: neweip += d_nsbit16(rsrc);break;
		case 4: neweip += d_nsbit32(rsrc);break;
		default: _bb("byte");
			_chk(_SetExcept_CE(byte));
			_be;break;
		}
		_be;
		_chk(_kec_jmp_near(neweip, _GetOperandSize));
	}
	_ce;
}
todo _e_jmp_far(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte)
{
	_cb("_e_jmp_far");
	if (!_IsProtected) {
		_bb("Protected(0)");
		_chk(_ser_jmp_far_real(newcs, neweip, byte));
		_be;
	} else {
		_bb("Protected(1)");
		_newins_;
		_comment("_e_jmp_far::Protected: not implemented");
		_chk(_SetExcept_CE(0));
		_be;
	}
	_ce;
}
done _e_jmp_near(t_nubit32 neweip, t_nubit8 byte)
{
	_cb("_e_jmp_near");
	_chk(_kec_jmp_near(neweip, byte));
	_ce;
}
done _e_load_far(t_cpu_sreg *rsreg, t_vaddrcc rdest, t_nubit16 selector, t_nubit32 offset, t_nubit8 byte)
{
	_cb("_e_load_far");
	_chk(_ksa_load_seg(rsreg, selector));
	switch (byte) {
	case 2: d_nubit16(rdest) = GetMax16(offset);break;
	case 4: d_nubit32(rdest) = GetMax32(offset);break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_ce;
}
done _e_loopcc(t_vaddrcc rsrc, t_bool condition)
{
	t_nubit32 cecx = 0x00000000;
	t_nubit32 neweip = vcpu.eip;
	_cb("_e_loopcc");
	switch (_GetAddressSize) {
	case 2:
		vcpu.cx--;
		cecx = GetMax16(vcpu.ecx);
		break;
	case 4:
		vcpu.ecx--;
		cecx = GetMax32(vcpu.ecx);
		break;
	default:_impossible_;break;}
	if (cecx && condition) {
		_bb("cecx(!0),condition(1)");
		neweip += d_nsbit8(rsrc);
		_chk(_kec_jmp_near(neweip, _GetOperandSize));
		_be;
	}
	_ce;
}
done _e_ret_near(t_nubit16 parambyte, t_nubit8 byte)
{
	_cb("_e_ret_near");
	_chk(_kec_ret_near(parambyte, byte));
	_ce;
}
todo _e_ret_far(t_nubit16 parambyte, t_nubit16 byte)
{
	_cb("_e_ret_far");
	if (!_IsProtected) {
		_bb("!Protected");
		_chk(_ser_ret_far_real(parambyte, byte));
		_be;
	} else {
		_bb("Protected");
		_newins_;
		_comment("_e_ret_far::Protected: not implemented");
		_chk(_SetExcept_CE(0));
		_be;
	}
	_ce;
}

/* arithmetic unit */
/* kernel arithmetic flags */
#define ADD_FLAG  (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF)
#define OR_FLAG   (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define ADC_FLAG  (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF)
#define SBB_FLAG  (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF)
#define AND_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SUB_FLAG  (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF)
#define XOR_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define CMP_FLAG  (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF)
#define INC_FLAG  (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_PF)
#define DEC_FLAG  (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_PF)
#define TEST_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SHL_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SHR_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SAL_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SAR_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define AAM_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define AAD_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define DAA_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define DAS_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)

done _kaf_calc_CF()
{
	_cb("_kaf_calc_CF");
	switch (vcpuins.type) {
	case ADC8:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (
			(_GetEFLAGS_CF && vcpuins.opr2 == Max8) ?
			1 : ((vcpuins.result < vcpuins.opr1) || (vcpuins.result < vcpuins.opr2))));
		break;
	case ADC16:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (
			(_GetEFLAGS_CF && vcpuins.opr2 == Max16) ?
			1 : ((vcpuins.result < vcpuins.opr1) || (vcpuins.result < vcpuins.opr2))));
		break;
	case ADC32:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (
			(_GetEFLAGS_CF && vcpuins.opr2 == Max32) ?
			1 : ((vcpuins.result < vcpuins.opr1) || (vcpuins.result < vcpuins.opr2))));
		break;
	case ADD8:
	case ADD16:
	case ADD32:
		MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,(vcpuins.result < vcpuins.opr1) || (vcpuins.result < vcpuins.opr2));
		break;
	case SBB8:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (vcpuins.opr1 < vcpuins.result) ||
			(_GetEFLAGS_CF && (vcpuins.opr2 == Max8)));
		break;
	case SBB16:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (vcpuins.opr1 < vcpuins.result) ||
			(_GetEFLAGS_CF && (vcpuins.opr2 == Max16)));
		break;
	case SBB32:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (vcpuins.opr1 < vcpuins.result) ||
			(_GetEFLAGS_CF && (vcpuins.opr2 == Max32)));
		break;
	case SUB8:
	case SUB16:
	case SUB32:
	case CMP8:
	case CMP16:
	case CMP32:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, vcpuins.opr1 < vcpuins.opr2);
		break;
	default: _bb("type");
		_chk(_SetExcept_CE(vcpuins.type));
		_be;break;
	}
	_ce;
}
done _kaf_calc_OF()
{
	_cb("_kaf_calc_OF");
	switch (vcpuins.type) {
	case ADC8:
	case ADD8:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
			(GetMSB8(vcpuins.opr1) == GetMSB8(vcpuins.opr2)) && (GetMSB8(vcpuins.opr1) != GetMSB8(vcpuins.result)));
		break;
	case ADC16:
	case ADD16:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
			(GetMSB16(vcpuins.opr1) == GetMSB16(vcpuins.opr2)) && (GetMSB16(vcpuins.opr1) != GetMSB16(vcpuins.result)));
		break;
	case ADC32:
	case ADD32:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
			(GetMSB32(vcpuins.opr1) == GetMSB32(vcpuins.opr2)) && (GetMSB32(vcpuins.opr1) != GetMSB32(vcpuins.result)));
		break;
	case SBB8:
	case SUB8:
	case CMP8:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
			(GetMSB8(vcpuins.opr1) != GetMSB8(vcpuins.opr2)) && (GetMSB8(vcpuins.opr2) == GetMSB8(vcpuins.result)));
		break;
	case SBB16:
	case SUB16:
	case CMP16:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
			(GetMSB16(vcpuins.opr1) != GetMSB16(vcpuins.opr2)) && (GetMSB16(vcpuins.opr2) == GetMSB16(vcpuins.result)));
		break;
	case SBB32:
	case SUB32:
	case CMP32:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
			(GetMSB32(vcpuins.opr1) != GetMSB32(vcpuins.opr2)) && (GetMSB32(vcpuins.opr2) == GetMSB32(vcpuins.result)));
		break;
	default: _bb("type");
		_chk(_SetExcept_CE(vcpuins.type));
		_be;break;
	}
	_ce;
}
done _kaf_calc_AF()
{
	_cb("_kaf_calc_AF");
	MakeBit(vcpu.eflags, VCPU_EFLAGS_AF, ((vcpuins.opr1 ^ vcpuins.opr2) ^ vcpuins.result) & 0x10);
	_ce;
}
done _kaf_calc_PF()
{
	t_nubit8 res8 = GetMax8(vcpuins.result);
	t_bool even = 1;
	_cb("_kaf_calc_PF");
	while (res8) {
		even = 1 - even;
		res8 &= res8-1;
	}
	MakeBit(vcpu.eflags, VCPU_EFLAGS_PF, even);
	_ce;
}
done _kaf_calc_ZF()
{
	_cb("_kaf_calc_ZF");
	MakeBit(vcpu.eflags, VCPU_EFLAGS_ZF, !vcpuins.result);
	_ce;
}
done _kaf_calc_SF()
{
	_cb("_kaf_calc_SF");
	switch (vcpuins.bit) {
	case 8: MakeBit(vcpu.eflags, VCPU_EFLAGS_SF, GetMSB8(vcpuins.result));break;
	case 16:MakeBit(vcpu.eflags, VCPU_EFLAGS_SF, GetMSB16(vcpuins.result));break;
	case 32:MakeBit(vcpu.eflags, VCPU_EFLAGS_SF, GetMSB32(vcpuins.result));break;
	default: _bb("bit");
		_chk(_SetExcept_CE(vcpuins.bit));
		_be;break;
	}
	_ce;
}
done _kaf_set_flags(t_nubit16 flags)
{
	_cb("_kaf_set_flags");
	if(flags & VCPU_EFLAGS_CF) _chk(_kaf_calc_CF());
	if(flags & VCPU_EFLAGS_PF) _chk(_kaf_calc_PF());
	if(flags & VCPU_EFLAGS_AF) _chk(_kaf_calc_AF());
	if(flags & VCPU_EFLAGS_ZF) _chk(_kaf_calc_ZF());
	if(flags & VCPU_EFLAGS_SF) _chk(_kaf_calc_SF());
	if(flags & VCPU_EFLAGS_OF) _chk(_kaf_calc_OF());
	_ce;
}
done _kas_move_index(t_nubit8 byte, t_bool flagsi, t_bool flagdi)
{
	_cb("_kas_move_index");
	switch (_GetAddressSize) {
	case 2:
		if(_GetEFLAGS_DF) {
			if (flagdi) vcpu.di -= byte;
			if (flagsi) vcpu.si -= byte;
		} else {
			if (flagdi) vcpu.di += byte;
			if (flagsi) vcpu.si += byte;
		}
		break;
	case 4:
		if(_GetEFLAGS_DF) {
			if (flagdi) vcpu.edi -= byte;
			if (flagsi) vcpu.esi -= byte;
		} else {
			if (flagdi) vcpu.edi += byte;
			if (flagsi) vcpu.esi += byte;
		}
		break;
	default:_impossible_;break;}
	_ce;
}

tots _a_add(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	_cb("_a_add");
	switch (bit) {
	case 8: _bb("bit(8+8)");
		vcpuins.bit = 8;
		vcpuins.type = ADD8;
		vcpuins.opr1 = GetMax8(d_nubit8(rdest));
		vcpuins.opr2 = GetMax8(d_nubit8(rsrc));
		vcpuins.result = GetMax8(vcpuins.opr1 + vcpuins.opr2);
		d_nubit8(rdest) = (t_nubit8)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 12: _bb("bit(16+8)");
		vcpuins.bit = 16;
		vcpuins.type = ADD16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nsbit8(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 + vcpuins.opr2);
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 16: _bb("bit(16+16)");
		vcpuins.bit = 16;
		vcpuins.type = ADD16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nubit16(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 + vcpuins.opr2);
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 20: _bb("bit(32+8)");
		_newins_;
		vcpuins.bit = 32;
		vcpuins.type = ADD32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nsbit8(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 + vcpuins.opr2);
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 32: _bb("bit(32+32)");
		_newins_;
		vcpuins.bit = 32;
		vcpuins.type = ADD32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nubit32(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 + vcpuins.opr2);
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_chk(_kaf_set_flags(ADD_FLAG));
	_ce;
}
tots _a_adc(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	_cb("_a_adc");
	switch (bit) {
	case 8: _bb("bit(8+8)");
		vcpuins.bit = 8;
		vcpuins.type = ADC8;
		vcpuins.opr1 = GetMax8(d_nubit8(rdest));
		vcpuins.opr2 = GetMax8(d_nubit8(rsrc));
		vcpuins.result = GetMax8(vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF);
		d_nubit8(rdest) = (t_nubit8)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 12: _bb("bit(16+8)");
		vcpuins.bit = 16;
		vcpuins.type = ADC16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nsbit8(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF);
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 16: _bb("bit(16+16)");
		vcpuins.bit = 16;
		vcpuins.type = ADC16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nubit16(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF);
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 20: _bb("bit(32+8)");
		_newins_;
		vcpuins.bit = 32;
		vcpuins.type = ADC32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nsbit8(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF);
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 32: _bb("bit(32+32)");
		_newins_;
		vcpuins.bit = 32;
		vcpuins.type = ADC32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nubit32(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF);
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_chk(_kaf_set_flags(ADC_FLAG));
	_ce;
}
tots _a_and(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	_cb("_a_and");
	switch (bit) {
	case 8: _bb("bit(8+8)");
		vcpuins.bit = 8;
		//vcpuins.type = AND8;
		vcpuins.opr1 = GetMax8(d_nubit8(rdest));
		vcpuins.opr2 = GetMax8(d_nubit8(rsrc));
		vcpuins.result = GetMax8(vcpuins.opr1 & vcpuins.opr2);
		d_nubit8(rdest) = (t_nubit8)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 12: _bb("bit(16+8)");
		vcpuins.bit = 16;
		//vcpuins.type = AND16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nsbit8(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 & vcpuins.opr2);
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 16: _bb("bit(16+16)");
		vcpuins.bit = 16;
		//vcpuins.type = AND16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nubit16(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 & vcpuins.opr2);
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 20: _bb("bit(32+8)");
		_newins_;
		vcpuins.bit = 32;
		//vcpuins.type = AND32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nsbit8(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 & vcpuins.opr2);
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 32: _bb("bit(32+32)");
		_newins_;
		vcpuins.bit = 32;
		//vcpuins.type = AND32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nubit32(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 & vcpuins.opr2);
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_chk(_kaf_set_flags(AND_FLAG));
	_ClrEFLAGS_OF;
	_ClrEFLAGS_CF;
	vcpuins.udf |= VCPU_EFLAGS_AF;
	_ce;
}
tots _a_or(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	_cb("_a_or");
	switch (bit) {
	case 8: _bb("bit(8+8)");
		vcpuins.bit = 8;
		//vcpuins.type = OR8;
		vcpuins.opr1 = GetMax8(d_nubit8(rdest));
		vcpuins.opr2 = GetMax8(d_nubit8(rsrc));
		vcpuins.result = GetMax8(vcpuins.opr1 | vcpuins.opr2);
		d_nubit8(rdest) = (t_nubit8)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 12: _bb("bit(16+8)");
		vcpuins.bit = 16;
		//vcpuins.type = OR16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nsbit8(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 | vcpuins.opr2);
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 16: _bb("bit(16+16)");
		vcpuins.bit = 16;
		//vcpuins.type = OR16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nubit16(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 | vcpuins.opr2);
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 20: _bb("bit(32+8)");
		_newins_;
		vcpuins.bit = 32;
		//vcpuins.type = OR32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nsbit8(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 | vcpuins.opr2);
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 32: _bb("bit(32+32)");
		_newins_;
		vcpuins.bit = 32;
		//vcpuins.type = OR32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nubit32(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 | vcpuins.opr2);
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_chk(_kaf_set_flags(OR_FLAG));
	_ClrEFLAGS_OF;
	_ClrEFLAGS_CF;
	vcpuins.udf |= VCPU_EFLAGS_AF;
	_ce;
}
tots _a_sbb(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	_cb("_a_sbb");
	switch (bit) {
	case 8: _bb("bit(8+8)");
		vcpuins.bit = 8;
		vcpuins.type = SBB8;
		vcpuins.opr1 = GetMax8(d_nubit8(rdest));
		vcpuins.opr2 = GetMax8(d_nubit8(rsrc));
		vcpuins.result = GetMax8(vcpuins.opr1 - (vcpuins.opr2 + _GetEFLAGS_CF));
		d_nubit8(rdest) = (t_nubit8)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 12: _bb("bit(16+8)");
		vcpuins.bit = 16;
		vcpuins.type = SBB16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nsbit8(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 - (vcpuins.opr2 + _GetEFLAGS_CF));
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 16: _bb("bit(16+16)");
		vcpuins.bit = 16;
		vcpuins.type = SBB16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nubit16(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 - (vcpuins.opr2 + _GetEFLAGS_CF));
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 20: _bb("bit(32+8)");
		_newins_;
		vcpuins.bit = 32;
		vcpuins.type = SBB32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nsbit8(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 - (vcpuins.opr2 + _GetEFLAGS_CF));
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 32: _bb("bit(32+32)");
		_newins_;
		vcpuins.bit = 32;
		vcpuins.type = SBB32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nubit32(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 - (vcpuins.opr2 + _GetEFLAGS_CF));
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_chk(_kaf_set_flags(SBB_FLAG));
	_ce;
}
tots _a_sub(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	_cb("_a_sub");
	switch (bit) {
	case 8: _bb("bit(8+8)");
		vcpuins.bit = 8;
		vcpuins.type = SUB8;
		vcpuins.opr1 = GetMax8(d_nubit8(rdest));
		vcpuins.opr2 = GetMax8(d_nubit8(rsrc));
		vcpuins.result = GetMax8(vcpuins.opr1 - vcpuins.opr2);
		d_nubit8(rdest) = (t_nubit8)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 12: _bb("bit(16+8)");
		vcpuins.bit = 16;
		vcpuins.type = SUB16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nsbit8(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 - vcpuins.opr2);
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 16: _bb("bit(16+16)");
		vcpuins.bit = 16;
		vcpuins.type = SUB16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nubit16(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 - vcpuins.opr2);
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 20: _bb("bit(32+8)");
		_newins_;
		vcpuins.bit = 32;
		vcpuins.type = SUB32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nsbit8(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 - vcpuins.opr2);
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 32: _bb("bit(32+32)");
		_newins_;
		vcpuins.bit = 32;
		vcpuins.type = SUB32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nubit32(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 - vcpuins.opr2);
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_chk(_kaf_set_flags(SUB_FLAG));
	_ce;
}
tots _a_xor(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	_cb("_a_xor");
	switch (bit) {
	case 8: _bb("bit(8+8)");
		vcpuins.bit = 8;
		//vcpuins.type = XOR8;
		vcpuins.opr1 = GetMax8(d_nubit8(rdest));
		vcpuins.opr2 = GetMax8(d_nubit8(rsrc));
		vcpuins.result = GetMax8(vcpuins.opr1 ^ vcpuins.opr2);
		d_nubit8(rdest) = (t_nubit8)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 12: _bb("bit(16+8)");
		vcpuins.bit = 16;
		//vcpuins.type = XOR16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nsbit8(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 ^ vcpuins.opr2);
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 16: _bb("bit(16+16)");
		vcpuins.bit = 16;
		//vcpuins.type = XOR16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nubit16(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 ^ vcpuins.opr2);
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 20: _bb("bit(32+8)");
		_newins_;
		vcpuins.bit = 32;
		//vcpuins.type = XOR32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nsbit8(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 ^ vcpuins.opr2);
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	case 32: _bb("bit(32+32)");
		_newins_;
		vcpuins.bit = 32;
		//vcpuins.type = XOR32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nubit32(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 ^ vcpuins.opr2);
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		/* note: should not use rsrc after this point */
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_chk(_kaf_set_flags(XOR_FLAG));
	_ClrEFLAGS_OF;
	_ClrEFLAGS_CF;
	vcpuins.udf |= VCPU_EFLAGS_AF;
	_ce;
}
tots _a_cmp(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	_cb("_a_cmp");
	switch (bit) {
	case 8: _bb("bit(8+8)");
		vcpuins.bit = 8;
		vcpuins.type = CMP8;
		vcpuins.opr1 = GetMax8(d_nubit8(rdest));
		vcpuins.opr2 = GetMax8(d_nsbit8(rsrc));
		vcpuins.result = GetMax8(((t_nubit8)vcpuins.opr1 - (t_nsbit8)vcpuins.opr2));
		_be;break;
	case 12: _bb("bit(16+8)");
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nsbit8(rsrc));
		vcpuins.result = GetMax16((t_nubit16)vcpuins.opr1 - (t_nsbit8)vcpuins.opr2);
		_be;break;
	case 16: _bb("bit(16+16)");
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nsbit16(rsrc));
		vcpuins.result = GetMax16((t_nubit16)vcpuins.opr1 - (t_nsbit16)vcpuins.opr2);
		_be;break;
	case 20: _bb("bit(32+8)");
		_newins_;
		vcpuins.bit = 32;
		vcpuins.type = CMP32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nsbit8(rsrc));
		vcpuins.result = GetMax32((t_nubit32)vcpuins.opr1 - (t_nsbit8)vcpuins.opr2);
		_be;break;
	case 32: _bb("bit(32+32)");
		_newins_;
		vcpuins.bit = 32;
		vcpuins.type = CMP32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nsbit32(rsrc));
		vcpuins.result = GetMax32((t_nubit32)vcpuins.opr1 - (t_nsbit32)vcpuins.opr2);
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_chk(_kaf_set_flags(CMP_FLAG));
	_ce;
}
tots _a_inc(t_vaddrcc rdest, t_nubit8 bit)
{
	_cb("_a_inc");
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		vcpuins.type = ADD8;
		vcpuins.opr1 = GetMax8(d_nubit8(rdest));
		vcpuins.opr2 = 0x01;
		vcpuins.result = GetMax8(vcpuins.opr1 + vcpuins.opr2);
		d_nubit8(rdest) = GetMax8(vcpuins.result);
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		vcpuins.type = ADD16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = 0x0001;
		vcpuins.result = GetMax16(vcpuins.opr1 + vcpuins.opr2);
		d_nubit16(rdest) = GetMax16(vcpuins.result);
		_be;break;
	case 32: _bb("bit(32)");
		_newins_;
		vcpuins.bit = 32;
		vcpuins.type = ADD32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = 0x00000001;
		vcpuins.result = GetMax32(vcpuins.opr1 + vcpuins.opr2);
		d_nubit32(rdest) = GetMax32(vcpuins.result);
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_chk(_kaf_set_flags(INC_FLAG));
	_ce;
}
tots _a_dec(t_vaddrcc rdest, t_nubit8 bit)
{
	_cb("_a_dec");
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		vcpuins.type = SUB8;
		vcpuins.opr1 = GetMax8(d_nubit8(rdest));
		vcpuins.opr2 = 0x01;
		vcpuins.result = GetMax8(vcpuins.opr1 - vcpuins.opr2);
		d_nubit8(rdest) = GetMax8(vcpuins.result);
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		vcpuins.type = SUB16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = 0x0001;
		vcpuins.result = GetMax16(vcpuins.opr1 - vcpuins.opr2);
		d_nubit16(rdest) = GetMax16(vcpuins.result);
		_be;break;
	case 32: _bb("bit(32)");
		_newins_;
		vcpuins.bit = 32;
		vcpuins.type = SUB32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = 0x00000001;
		vcpuins.result = GetMax32(vcpuins.opr1 - vcpuins.opr2);
		d_nubit32(rdest) = GetMax32(vcpuins.result);
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_chk(_kaf_set_flags(DEC_FLAG));
	_ce;
}
tots _a_test(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	_cb("_a_test");
	switch (bit) {
	case 8: _bb("bit(8+8)");
		vcpuins.bit = 8;
		//vcpuins.type = TEST8;
		vcpuins.opr1 = GetMax8(d_nubit8(rdest));
		vcpuins.opr2 = GetMax8(d_nubit8(rsrc));
		vcpuins.result = GetMax8(vcpuins.opr1 & vcpuins.opr2);
		/* note: should not use rsrc after this point */
		_be;break;
	case 12: _bb("bit(16+8)");
		vcpuins.bit = 16;
		//vcpuins.type = TEST16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nsbit8(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 & vcpuins.opr2);
		/* note: should not use rsrc after this point */
		_be;break;
	case 16: _bb("bit(16+16)");
		vcpuins.bit = 16;
		//vcpuins.type = TEST16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nubit16(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 & vcpuins.opr2);
		/* note: should not use rsrc after this point */
		_be;break;
	case 20: _bb("bit(32+8)");
		_newins_;
		vcpuins.bit = 32;
		//vcpuins.type = TEST32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nsbit8(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 & vcpuins.opr2);
		/* note: should not use rsrc after this point */
		_be;break;
	case 32: _bb("bit(32+32)");
		_newins_;
		vcpuins.bit = 32;
		//vcpuins.type = TEST32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nubit32(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 & vcpuins.opr2);
		/* note: should not use rsrc after this point */
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_chk(_kaf_set_flags(TEST_FLAG));
	_ClrEFLAGS_OF;
	_ClrEFLAGS_CF;
	vcpuins.udf |= VCPU_EFLAGS_AF;
	_ce;
}
tots _a_xchg(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	_cb("_a_xchg");
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		vcpuins.opr1 = GetMax8(d_nubit8(rdest));
		vcpuins.opr2 = GetMax8(d_nubit8(rsrc));
		d_nubit8(rdest) = (t_nubit8)vcpuins.opr2;
		d_nubit8(rsrc) = (t_nubit8)vcpuins.opr1;
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nubit16(rsrc));
		d_nubit16(rdest) = (t_nubit16)vcpuins.opr2;
		d_nubit16(rsrc) = (t_nubit16)vcpuins.opr1;
		_be;break;
	case 32: _bb("bit(32)");
		_newins_;
		vcpuins.bit = 32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nubit32(rsrc));
		d_nubit32(rdest) = (t_nubit32)vcpuins.opr2;
		d_nubit32(rsrc) = (t_nubit32)vcpuins.opr1;
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
tots _m_mov(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 byte)
{
	_cb("_m_mov");
	switch (byte) {
	case 1:
		vcpuins.bit = 8;
		d_nubit8(rdest) = d_nubit8(rsrc);
		break;
	case 2:
		vcpuins.bit = 16;
		d_nubit16(rdest) = d_nubit16(rsrc);
		break;
	case 4:
		vcpuins.bit = 32;
		d_nubit32(rdest) = d_nubit32(rsrc);
		break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_ce;
}
tots _a_rol(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nubit8 count = 0x00;
	t_nubit8 tempcount = 0x00;
	t_nubit32 flagcf = 0x0000000000000000;
	_cb("_a_rol");
	if(rsrc) count = d_nubit8(rsrc);
	else count = 1;
	switch (bit) {
	case 8: _bb("bit(8)");
		tempcount = (count & 0x07);
		vcpuins.bit = 8;
		while (tempcount) {
			flagcf = !!GetMSB8(d_nubit8(rdest));
			d_nubit8(rdest) = (d_nubit8(rdest) << 1) | flagcf;
			tempcount--;
		}
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!GetLSB8(d_nubit8(rdest)));
		if(count == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB8(d_nubit8(rdest))) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		_be;break;
	case 16: _bb("bit(16)");
		tempcount = (count & 0x0f);
		vcpuins.bit = 16;
		while (tempcount) {
			flagcf = !!GetMSB16(d_nubit16(rdest));
			d_nubit16(rdest) = (d_nubit16(rdest) << 1) | flagcf;
			tempcount--;
		}
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!GetLSB16(d_nubit16(rdest)));
		if(count == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB16(d_nubit16(rdest))) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		_be;break;
	case 32: _bb("bit(32)");
		_newins_;
		tempcount = (count & 0x1f);
		vcpuins.bit = 32;
		while (tempcount) {
			flagcf = !!GetMSB32(d_nubit32(rdest));
			d_nubit32(rdest) = (d_nubit32(rdest) << 1) | flagcf;
			tempcount--;
		}
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!GetLSB32(d_nubit32(rdest)));
		if(count == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB32(d_nubit32(rdest))) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
tots _a_ror(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nubit8 count = 0x00;
	t_nubit8 tempcount = 0x00;
	t_nubit32 flagcf = 0x0000000000000000;
	_cb("_a_ror");
	if(rsrc) count = d_nubit8(rsrc);
	else count = 1;
	switch (bit) {
	case 8: _bb("bit(8)");
		tempcount = (count & 0x07);
		vcpuins.bit = 8;
		while (tempcount) {
			flagcf = GetLSB8(d_nubit8(rdest)) ? MSB8 : 0;
			d_nubit8(rdest) = (d_nubit8(rdest) >> 1) | flagcf;
			tempcount--;
		}
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!GetMSB8(d_nubit8(rdest)));
		if(count == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
			((!!GetMSB8(d_nubit8(rdest))) ^ (!!GetMSB7(d_nubit8(rdest)))));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		_be;break;
	case 16: _bb("bit(16)");
		tempcount = (count & 0x0f);
		vcpuins.bit = 16;
		while (tempcount) {
			flagcf = GetLSB16(d_nubit16(rdest)) ? MSB16 : 0;
			d_nubit16(rdest) = (d_nubit16(rdest) >> 1) | flagcf;
			tempcount--;
		}
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!GetMSB16(d_nubit16(rdest)));
		if(count == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
			((!!GetMSB16(d_nubit16(rdest))) ^ (!!GetMSB15(d_nubit16(rdest)))));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		_be;break;
	case 32: _bb("bit(32)");
		_newins_;
		tempcount = (count & 0x1f);
		vcpuins.bit = 32;
		while (tempcount) {
			flagcf = GetLSB32(d_nubit32(rdest)) ? MSB32 : 0;
			d_nubit32(rdest) = (d_nubit32(rdest) >> 1) | flagcf;
			tempcount--;
		}
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!GetMSB32(d_nubit32(rdest)));
		if(count == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
			((!!GetMSB32(d_nubit32(rdest))) ^ (!!GetMSB31(d_nubit32(rdest)))));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
tots _a_rcl(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nubit8 count = 0x00;
	t_nubit8 tempcount = 0x00;
	t_nubit32 flagcf = 0x0000000000000000;
	_cb("_a_rcl");
	if(rsrc) count = d_nubit8(rsrc);
	else count = 1;
	switch (bit) {
	case 8: _bb("bit(8)");
		tempcount = (count & 0x1f) % 9;
		vcpuins.bit = 8;
		while (tempcount) {
			flagcf = !!GetMSB8(d_nubit8(rdest));
			d_nubit8(rdest) = (d_nubit8(rdest) << 1) | _GetEFLAGS_CF;
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, flagcf);
			tempcount--;
		}
		if(count == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB8(d_nubit8(rdest))) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		_be;break;
	case 16: _bb("bit(16)");
		tempcount = (count & 0x1f) % 17;
		vcpuins.bit = 16;
		while (tempcount) {
			flagcf = !!GetMSB16(d_nubit16(rdest));
			d_nubit16(rdest) = (d_nubit16(rdest) << 1) | _GetEFLAGS_CF;
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, flagcf);
			tempcount--;
		}
		if(count == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB16(d_nubit16(rdest))) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		_be;break;
	case 32: _bb("bit(32)");
		_newins_;
		tempcount = (count & 0x1f);
		vcpuins.bit = 32;
		while (tempcount) {
			flagcf = !!GetMSB32(d_nubit32(rdest));
			d_nubit32(rdest) = (d_nubit32(rdest) << 1) | _GetEFLAGS_CF;
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, flagcf);
			tempcount--;
		}
		if(count == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB32(d_nubit32(rdest))) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
tots _a_rcr(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nubit8 count = 0x00;
	t_nubit8 tempcount = 0x00;
	t_nubit32 flagcf = 0x0000000000000000;
	_cb("_a_rcr");
	if(rsrc) count = d_nubit8(rsrc);
	else count = 1;
	switch (bit) {
	case 8: _bb("bit(8)");
		tempcount = (count & 0x1f) % 9;
		vcpuins.bit = 8;
		if(count == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB8(d_nubit8(rdest))) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		while (tempcount) {
			flagcf = GetLSB8(d_nubit8(rdest));
			d_nubit8(rdest) = (d_nubit8(rdest) >> 1) | (_GetEFLAGS_CF ? MSB8 : 0);
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!flagcf);
			tempcount--;
		}
		_be;break;
	case 16: _bb("bit(16)");
		tempcount = (count & 0x1f) % 17;
		vcpuins.bit = 16;
		if(count == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB16(d_nubit16(rdest))) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		while (tempcount) {
			flagcf = GetLSB16(d_nubit16(rdest));
			d_nubit16(rdest) = (d_nubit16(rdest) >> 1) | (_GetEFLAGS_CF ? MSB16 : 0);
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!flagcf);
			tempcount--;
		}
		_be;break;
	case 32: _bb("bit(32)");
		_newins_;
		tempcount = (count & 0x1f);
		vcpuins.bit = 32;
		if(count == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB32(d_nubit32(rdest))) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		while (tempcount) {
			flagcf = GetLSB32(d_nubit32(rdest));
			d_nubit32(rdest) = (d_nubit32(rdest) >> 1) | (_GetEFLAGS_CF ? MSB32 : 0);
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!flagcf);
			tempcount--;
		}
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
tots _a_shl(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nubit8 count = 0x00;
	t_nubit8 tempcount = 0x00;
	_cb("_a_shl");
	if(rsrc) count = d_nubit8(rsrc);
	else count = 1;
	tempcount = count & 0x1f;
	if (count >= bit) vcpuins.udf |= VCPU_EFLAGS_CF;
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		while (tempcount) {
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!GetMSB8(d_nubit8(rdest)));
			d_nubit8(rdest) <<= 1;
			tempcount--;
		}
		if(count == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB8(d_nubit8(rdest))) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		if(count != 0) {
			_bb("count(!0)");
			vcpuins.result = d_nubit8(rdest);
			_chk(_kaf_set_flags(SHL_FLAG));
			vcpuins.udf |= VCPU_EFLAGS_AF;
			_be;
		}
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		while (tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF, !!GetMSB16(d_nubit16(rdest)));
			d_nubit16(rdest) <<= 1;
			tempcount--;
		}
		if(count == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB16(d_nubit16(rdest))) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		if(count != 0) {
			_bb("count(!0)");
			vcpuins.result = d_nubit16(rdest);
			_chk(_kaf_set_flags(SHL_FLAG));
			vcpuins.udf |= VCPU_EFLAGS_AF;
			_be;
		}
		_be;break;
	case 32: _bb("bit(32)");
		_newins_;
		vcpuins.bit = 32;
		while (tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF, !!GetMSB32(d_nubit32(rdest)));
			d_nubit32(rdest) <<= 1;
			tempcount--;
		}
		if(count == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB32(d_nubit32(rdest))) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		if(count != 0) {
			_bb("count(!0)");
			vcpuins.result = d_nubit32(rdest);
			_chk(_kaf_set_flags(SHL_FLAG));
			vcpuins.udf |= VCPU_EFLAGS_AF;
			_be;
		}
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
tots _a_shr(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nubit8 count = 0x00;
	t_nubit8 tempcount = 0x00;
	t_nubit32 tempdest = 0x00000000;
	_cb("_a_shr");
	if(rsrc) count = d_nubit8(rsrc);
	else count = 1;
	tempcount = count & 0x1f;
	if (count >= bit) vcpuins.udf |= VCPU_EFLAGS_CF;
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		tempdest = d_nubit8(rdest);
		while (tempcount) {
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (!!GetLSB8(d_nubit8(rdest))));
			d_nubit8(rdest) >>= 1;
			tempcount--;
		}
		if(count == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF, (!!GetMSB8(tempdest)));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		if(count != 0) {
			_bb("count(!0)");
			vcpuins.result = d_nubit8(rdest);
			_chk(_kaf_set_flags(SHR_FLAG));
			vcpuins.udf |= VCPU_EFLAGS_AF;
			_be;
		}
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		tempdest = d_nubit16(rdest);
		while (tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF, (!!GetLSB16(d_nubit16(rdest))));
			d_nubit16(rdest) >>= 1;
			tempcount--;
		}
		if(count == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF, (!!GetMSB16(tempdest)));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		if(count != 0) {
			_bb("count(!0)");
			vcpuins.result = d_nubit16(rdest);
			_chk(_kaf_set_flags(SHR_FLAG));
			vcpuins.udf |= VCPU_EFLAGS_AF;
			_be;
		}
		_be;break;
	case 32: _bb("bit(32)");
		_newins_;
		vcpuins.bit = 32;
		tempdest = d_nubit32(rdest);
		while (tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF, (!!GetLSB32(d_nubit32(rdest))));
			d_nubit32(rdest) >>= 1;
			tempcount--;
		}
		if(count == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF, (!!GetMSB32(tempdest)));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		if(count != 0) {
			_bb("count(!0)");
			vcpuins.result = d_nubit32(rdest);
			_chk(_kaf_set_flags(SHR_FLAG));
			vcpuins.udf |= VCPU_EFLAGS_AF;
			_be;
		}
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
tots _a_sar(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nubit8 count = 0x00;
	t_nubit8 tempcount = 0x00;
	t_nubit32 tempdest = 0x00000000;
	_cb("_a_shr");
	if(rsrc) count = d_nubit8(rsrc);
	else count = 1;
	tempcount = count & 0x1f;
	switch (bit) {
	case 8: _bb("bit(8)");
		_newins_;
		vcpuins.bit = 8;
		tempdest = d_nsbit8(rdest);
		while (tempcount) {
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (!!GetLSB8(d_nsbit8(rdest))));
			d_nsbit8(rdest) >>= 1;
			tempcount--;
		}
		if(count == 1) _ClrEFLAGS_OF;
		else vcpuins.udf |= VCPU_EFLAGS_OF;
		if(count != 0) {
			_bb("count(!0)");
			vcpuins.result = d_nsbit8(rdest);
			_chk(_kaf_set_flags(SAR_FLAG));
			vcpuins.udf |= VCPU_EFLAGS_AF;
			_be;
		}
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		tempdest = d_nsbit16(rdest);
		while (tempcount) {
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (!!GetLSB16(d_nsbit16(rdest))));
			d_nsbit16(rdest) >>= 1;
			tempcount--;
		}
		if(count == 1) _ClrEFLAGS_OF;
		else vcpuins.udf |= VCPU_EFLAGS_OF;
		if(count != 0) {
			_bb("count(!0)");
			vcpuins.result = d_nsbit16(rdest);
			_chk(_kaf_set_flags(SAR_FLAG));
			vcpuins.udf |= VCPU_EFLAGS_AF;
			_be;
		}
		_be;break;
	case 32: _bb("bit(32)");
		_newins_;
		vcpuins.bit = 32;
		tempdest = d_nsbit32(rdest);
		while (tempcount) {
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (!!GetLSB32(d_nsbit32(rdest))));
			d_nsbit32(rdest) >>= 1;
			tempcount--;
		}
		if(count == 1) _ClrEFLAGS_OF;
		else vcpuins.udf |= VCPU_EFLAGS_OF;
		if(count != 0) {
			_bb("count(!0)");
			vcpuins.result = d_nsbit32(rdest);
			_chk(_kaf_set_flags(SAR_FLAG));
			vcpuins.udf |= VCPU_EFLAGS_AF;
			_be;
		}
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
tots _a_not(t_vaddrcc rdest, t_nubit8 bit)
{
	_cb("_a_not");
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		d_nubit8(rdest) = ~d_nubit8(rdest);
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		d_nubit16(rdest) = ~d_nubit16(rdest);
		_be;break;
	case 32: _bb("bit(32)");
		vcpuins.bit = 32;
		d_nubit32(rdest) = ~d_nubit32(rdest);
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
tots _a_neg(t_vaddrcc rdest, t_nubit8 bit)
{
	t_nubit64 res = 0x0000000000000000;
	_cb("_a_neg");
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		_chk(_a_sub((t_vaddrcc)&res, rdest, 8));
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!d_nubit8(rdest));
		d_nubit8(rdest) = GetMax8(res);
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		_chk(_a_sub((t_vaddrcc)&res, rdest, 16));
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!d_nubit16(rdest));
		d_nubit16(rdest) = GetMax16(res);
		_be;break;
	case 32: _bb("bit(32)");
		vcpuins.bit = 32;
		_chk(_a_sub((t_vaddrcc)&res, rdest, 32));
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!d_nubit32(rdest));
		d_nubit32(rdest) = GetMax32(res);
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
tots _a_mul(t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nubit64 cdest = 0x0000000000000000;
	_cb("_a_mul");
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		cdest = GetMax16(vcpu.al * d_nubit8(rsrc));
		vcpu.ax = GetMax16(cdest);
		MakeBit(vcpu.eflags, VCPU_EFLAGS_OF, !!vcpu.ah);
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!vcpu.ah);
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		cdest = GetMax32(vcpu.ax * d_nubit16(rsrc));
		vcpu.dx = GetMax16(cdest >> 16);
		vcpu.ax = GetMax16(cdest);
		MakeBit(vcpu.eflags, VCPU_EFLAGS_OF, !!vcpu.dx);
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!vcpu.dx);
		_be;break;
	case 32: _bb("bit(32)");
		_newins_;
		vcpuins.bit = 32;
		cdest = GetMax64(vcpu.eax * d_nubit32(rsrc));
		vcpu.edx = GetMax32(cdest >> 32);
		vcpu.eax = GetMax32(cdest);
		MakeBit(vcpu.eflags, VCPU_EFLAGS_OF, !!vcpu.edx);
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!vcpu.edx);
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	vcpuins.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
		VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
	_ce;
}
tots _a_imul(t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nsbit64 cdest = 0x0000000000000000;
	_cb("_a_imul");
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		cdest = GetMax16((t_nsbit8)vcpu.al * d_nsbit8(rsrc));
		vcpu.ax = GetMax16(cdest);
		if (GetMax16(cdest) == (t_nsbit16)((t_nsbit8)vcpu.al)) {
			_ClrEFLAGS_CF;
			_ClrEFLAGS_OF;
		} else {
			_SetEFLAGS_CF;
			_SetEFLAGS_OF;
		}
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		cdest = GetMax32((t_nsbit16)vcpu.ax * d_nsbit16(rsrc));
		vcpu.ax = GetMax16(cdest);
		vcpu.dx = GetMax16(cdest >> 16);
		if (GetMax32(cdest) == (t_nsbit32)((t_nsbit16)vcpu.ax)) {
			_ClrEFLAGS_CF;
			_ClrEFLAGS_OF;
		} else {
			_SetEFLAGS_CF;
			_SetEFLAGS_OF;
		}
		_be;break;
	case 32: _bb("bit(32)");
		_newins_;
		vcpuins.bit = 32;
		cdest = GetMax64((t_nsbit32)vcpu.eax * d_nsbit32(rsrc));
		vcpu.eax = GetMax32(cdest);
		vcpu.edx = GetMax32(cdest >> 32);
		if (GetMax64(cdest) == (t_nsbit64)((t_nsbit32)vcpu.eax)) {
			_ClrEFLAGS_CF;
			_ClrEFLAGS_OF;
		} else {
			_SetEFLAGS_CF;
			_SetEFLAGS_OF;
		}
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	vcpuins.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
		VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
	_ce;
}
tots _a_imul3(t_vaddrcc rdest, t_vaddrcc rsrc1, t_vaddrcc rsrc2, t_nubit8 bit)
{
	t_nsbit64 cdest = 0x0000000000000000;
	_cb("_a_imul3");
	_newins_;
	switch (bit) {
	case 12: _bb("bit(16+8)");
		vcpuins.bit = 16;
		cdest = GetMax32(d_nsbit16(rsrc1) * d_nsbit8(rsrc2));
		d_nsbit16(rdest) = GetMax16(cdest);
		/* note: should not use rsrc1 after this point */
		if (GetMax32(cdest) != (t_nsbit32)(d_nsbit16(rdest))) {
			_SetEFLAGS_CF;
			_SetEFLAGS_OF;
		} else {
			_ClrEFLAGS_CF;
			_ClrEFLAGS_OF;
		}
		_be;break;
	case 16: _bb("bit(16+16)");
		vcpuins.bit = 16;
		cdest = GetMax32(d_nsbit16(rsrc1) * d_nsbit16(rsrc2));
		d_nsbit16(rdest) = GetMax16(cdest);
		/* note: should not use rsrc1 after this point */
		if (GetMax32(cdest) != (t_nsbit32)(d_nsbit16(rdest))) {
			_SetEFLAGS_CF;
			_SetEFLAGS_OF;
		} else {
			_ClrEFLAGS_CF;
			_ClrEFLAGS_OF;
		}
		_be;break;
	case 20: _bb("bit(32+8)");
		vcpuins.bit = 32;
		cdest = GetMax64(d_nsbit32(rsrc1) * d_nsbit8(rsrc2));
		d_nsbit32(rdest) = GetMax32(cdest);
		/* note: should not use rsrc1 after this point */
		if (GetMax64(cdest) != (t_nsbit64)(d_nsbit32(rdest))) {
			_SetEFLAGS_CF;
			_SetEFLAGS_OF;
		} else {
			_ClrEFLAGS_CF;
			_ClrEFLAGS_OF;
		}
		_be;break;
	case 32: _bb("bit(32+32");
		vcpuins.bit = 32;
		cdest = GetMax64(d_nsbit32(rsrc1) * d_nsbit32(rsrc2));
		d_nsbit32(rdest) = GetMax32(cdest);
		/* note: should not use rsrc1 after this point */
		if (GetMax64(cdest) != (t_nsbit64)(d_nsbit32(rdest))) {
			_SetEFLAGS_CF;
			_SetEFLAGS_OF;
		} else {
			_ClrEFLAGS_CF;
			_ClrEFLAGS_OF;
		}
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	vcpuins.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
		VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
	_ce;
}
tots _a_div(t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nubit32 csrc = 0x00000000;
	t_nubit64 cdest = 0x0000000000000000;
	t_nubit64 temp = 0x0000000000000000;
	_cb("_a_div");
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		cdest = GetMax16(vcpu.ax);
		csrc = GetMax8(d_nubit8(rsrc));
		if(!csrc) {
			_bb("csrc(0)");
			_chk(_SetExcept_DE(0));
			_be;
		} else {
			_bb("csrc(!0)");
			temp = GetMax16((t_nubit16)cdest / (t_nubit8)csrc);
			if (temp > Max8) {
				_bb("temp(>0xff)");
				_chk(_SetExcept_DE(0));
				_be;
			} else {
				vcpu.al = GetMax8(temp);
				vcpu.ah = GetMax8((t_nubit16)cdest % (t_nubit8)csrc);
			}
			_be;
		}
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		cdest = GetMax32((vcpu.dx << 16) | vcpu.ax);
		csrc = GetMax16(d_nubit16(rsrc));
		if(!csrc) {
			_bb("csrc(0)");
			_chk(_SetExcept_DE(0));
			_be;
		} else {
			_bb("csrc(!0)");
			temp = GetMax32((t_nubit32)cdest / (t_nubit16)csrc);
			if (temp > Max16) {
				_bb("temp(>0xffff)");
				_chk(_SetExcept_DE(0));
				_be;
			} else {
				vcpu.ax = GetMax16(temp);
				vcpu.dx = GetMax16((t_nubit32)cdest % (t_nubit16)csrc);
			}
			_be;
		}
		_be;break;
	case 32: _bb("bit(32)");
		_newins_;
		vcpuins.bit = 32;
		cdest = GetMax64(((t_nubit64)vcpu.edx << 32) | vcpu.eax);
		csrc = GetMax32(d_nubit32(rsrc));
		if(!csrc) {
			_bb("csrc(0)");
			_chk(_SetExcept_DE(0));
			_be;
		} else {
			_bb("csrc(!0)");
			temp = GetMax64((t_nubit64)cdest / (t_nubit32)csrc);
			if (temp > Max32) {
				_bb("temp(>0xffffffff)");
				_chk(_SetExcept_DE(0));
				_be;
			} else {
				vcpu.eax = GetMax32(temp);
				vcpu.edx = GetMax32((t_nubit64)cdest % (t_nubit32)csrc);
			}
			_be;
		}
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	vcpuins.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF |
		VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_OF);
	_ce;
}
tots _a_idiv(t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nsbit8 csrc8 = 0x00;
	t_nsbit16 csrc16 = 0x0000;
	t_nsbit32 csrc32 = 0x00000000;
	t_nsbit16 cdest16 = 0x0000;
	t_nsbit32 cdest32 = 0x00000000;
	t_nsbit64 cdest64 = 0x0000000000000000;
	t_nsbit64 temp = 0x0000000000000000;
	_cb("_a_idiv");
	switch (bit) {
	case 8: _bb("bit(8)");
		_newins_;
		vcpuins.bit = 8;
		cdest16 = (t_nsbit16)vcpu.ax;
		csrc8 = d_nsbit8(rsrc);
		if(!csrc8) {
			_bb("csrc8(0)");
			_chk(_SetExcept_DE(0));
			_be;
		} else {
			_bb("csrc8(!0)");
			temp = GetMax16(cdest16 / csrc8);
			if ((temp > 0 && GetMax8(temp) > 0x7f) ||
				(temp < 0 && GetMax8(temp) < 0x80)) {
				_bb("temp(>0x7f/<0x80)");
				_chk(_SetExcept_DE(0));
				_be;
			} else {
				vcpu.al = GetMax8(temp);
				vcpu.ah = GetMax8(cdest16 % csrc8);
			}
			_be;
		}
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		cdest32 = (t_nsbit32)((vcpu.dx << 16) | vcpu.ax);
		csrc16 = d_nsbit16(rsrc);
		if(!csrc16) {
			_bb("csrc16(0)");
			_chk(_SetExcept_DE(0));
			_be;
		} else {
			_bb("csrc16(!0)");
			temp = GetMax32(cdest32 / csrc16);
			if ((temp > 0 && GetMax16(temp) > 0x7fff) ||
				(temp < 0 && GetMax16(temp) < 0x8000)) {
				_bb("temp(>0x7fff/<0x8000)");
				_chk(_SetExcept_DE(0));
				_be;
			} else {
				vcpu.ax = GetMax16(temp);
				vcpu.dx = GetMax16(cdest32 % csrc16);
			}
			_be;
		}
		_be;break;
	case 32: _bb("bit(32)");
		_newins_;
		vcpuins.bit = 32;
		cdest64 = (t_nsbit64)(((t_nubit64)vcpu.edx << 32) | vcpu.eax);
		csrc32 = d_nsbit32(rsrc);
		if(!csrc32) {
			_bb("csrc32(0)");
			_chk(_SetExcept_DE(0));
			_be;
		} else {
			_bb("csrc32(!0)");
			temp = GetMax64(cdest64 / csrc32);
			if ((temp > 0 && GetMax32(temp) > 0x7fffffff) ||
				(temp < 0 && GetMax32(temp) < 0x80000000)) {
				_bb("temp(>0x7fffffff/<0x80000000)");
				_chk(_SetExcept_DE(0));
				_be;
			} else {
				vcpu.eax = GetMax32(temp);
				vcpu.edx = GetMax32(cdest64 % csrc32);
			}
			_be;
		}
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	vcpuins.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF |
		VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_OF);
	_ce;
}
tots _p_ins(t_nubit8 byte)
{
	t_nubit32 data = 0x00000000;
	t_nubit32 cedi = 0x00000000;
	_cb("_p_ins");
	switch (_GetAddressSize) {
	case 2: cedi = vcpu.di;break;
	case 4: cedi = vcpu.edi;break;
	default:_impossible_;break;}
	switch (byte) {
	case 1: _bb("byte(1)");
		_newins_;
		vcpuins.bit = 8;
		_chk(_m_test_logical(&vcpu.es, cedi, 1, 1));
		_chk(data = (t_nubit8)_p_input(vcpu.dx, 1));
		_chk(_m_write_logical(&vcpu.es, cedi, data, 1));
		_chk(_kas_move_index(1, 0, 1));
		_be;break;
	case 2: _bb("byte(2)");
		_newins_;
		vcpuins.bit = 16;
		_chk(_m_test_logical(&vcpu.es, cedi, 2, 1));
		_chk(data = (t_nubit16)_p_input(vcpu.dx, 2));
		_chk(_m_write_logical(&vcpu.es, cedi, data, 2));
		_chk(_kas_move_index(2, 0, 1));
		_be;break;
	case 4: _bb("byte(4)");
		_newins_;
		vcpuins.bit = 32;
		_chk(_m_test_logical(&vcpu.es, cedi, 4, 1));
		_chk(data = (t_nubit32)_p_input(vcpu.dx, 4));
		_chk(_m_write_logical(&vcpu.es, cedi, data, 4));
		_chk(_kas_move_index(4, 0, 1));
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_ce;
}
tots _p_outs(t_nubit8 byte)
{
	t_nubit32 data = 0x00000000;
	t_nubit32 cesi = 0x00000000;
	_cb("_p_outs");
	switch (_GetAddressSize) {
	case 2: cesi = vcpu.si;break;
	case 4: cesi = vcpu.esi;break;
	default:_impossible_;break;}
	switch (byte) {
	case 1: _bb("byte(1)");
		_newins_;
		vcpuins.bit = 8;
		_chk(data = (t_nubit8)_m_read_logical(vcpuins.roverds, cesi, 1));
		_chk(_p_output(vcpu.dx, data, 1));
		_chk(_kas_move_index(1, 1, 0));
		_be;break;
	case 2: _bb("byte(2)");
		_newins_;
		vcpuins.bit = 16;
		_chk(data = (t_nubit16)_m_read_logical(vcpuins.roverds, cesi, 2));
		_chk(_p_output(vcpu.dx, data, 2));
		_chk(_kas_move_index(2, 1, 0));
		_be;break;
	case 4: _bb("byte(4)");
		_newins_;
		vcpuins.bit = 32;
		_chk(data = (t_nubit32)_m_read_logical(vcpuins.roverds, cesi, 4));
		_chk(_p_output(vcpu.dx, data, 4));
		_chk(_kas_move_index(4, 1, 0));
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_ce;
}
tots _m_movs(t_nubit8 byte)
{
	t_nubit32 data = 0x00000000;
	t_nubit32 cesi = 0x00000000, cedi = 0x00000000;
	_cb("_m_movs");
	switch (_GetAddressSize) {
	case 2:
		cesi = vcpu.si;
		cedi = vcpu.di;
		break;
	case 4:
		cesi = vcpu.esi;
		cedi = vcpu.edi;
		break;
	default:_impossible_;break;}
	switch (byte) {
	case 1: _bb("byte(1)");
		vcpuins.bit = 8;
		_chk(data = (t_nubit8)_m_read_logical(vcpuins.roverds, cesi, 1));
		_chk(_m_write_logical(&vcpu.es, cedi, data, 1));
		_chk(_kas_move_index(1, 1, 1));
		_be;break;
	case 2: _bb("byte(2)");
		vcpuins.bit = 16;
		_chk(data = (t_nubit16)_m_read_logical(vcpuins.roverds, cesi, 2));
		_chk(_m_write_logical(&vcpu.es, cedi, data, 2));
		_chk(_kas_move_index(2, 1, 1));
		_be;break;
	case 4: _bb("byte(4)");
		_newins_;
		vcpuins.bit = 32;
		_chk(data = (t_nubit32)_m_read_logical(vcpuins.roverds, cesi, 4));
		_chk(_m_write_logical(&vcpu.es, cedi, data, 4));
		_chk(_kas_move_index(4, 1, 1));
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_ce;
}
tots _a_cmps(t_nubit8 bit)
{
	t_nubit32 cesi = 0x00000000, cedi = 0x00000000;
	_cb("_a_cmps");
	switch (_GetAddressSize) {
	case 2:
		cesi = vcpu.si;
		cedi = vcpu.di;
		break;
	case 4:
		cesi = vcpu.esi;
		cedi = vcpu.edi;
		break;
	default:_impossible_;break;}
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		vcpuins.type = CMP8;
		_chk(vcpuins.opr1 = (t_nubit8)_m_read_logical(vcpuins.roverds, cesi, 1));
		_chk(vcpuins.opr2 = (t_nubit8)_m_read_logical(&vcpu.es, cedi, 1));
		vcpuins.result = GetMax8(vcpuins.opr1 - vcpuins.opr2);
		_chk(_kas_move_index(1, 1, 1));
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		_chk(vcpuins.opr1 = (t_nubit16)_m_read_logical(vcpuins.roverds, cesi, 2));
		_chk(vcpuins.opr2 = (t_nubit16)_m_read_logical(&vcpu.es, cedi, 2));
		vcpuins.result = GetMax16(vcpuins.opr1 - vcpuins.opr2);
		_chk(_kas_move_index(2, 1, 1));
		_be;break;
	case 32: _bb("bit(32)");
		_newins_;
		vcpuins.bit = 32;
		vcpuins.type = CMP32;
		_chk(vcpuins.opr1 = (t_nubit32)_m_read_logical(vcpuins.roverds, cesi, 4));
		_chk(vcpuins.opr2 = (t_nubit32)_m_read_logical(&vcpu.es, cedi, 4));
		vcpuins.result = GetMax32(vcpuins.opr1 - vcpuins.opr2);
		_chk(_kas_move_index(4, 1, 1));
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_chk(_kaf_set_flags(CMP_FLAG));
	_ce;
}
tots _m_stos(t_nubit8 byte)
{
	t_nubit32 cedi = 0x00000000;
	_cb("_m_stos");
	switch (_GetAddressSize) {
	case 2: cedi = vcpu.di;break;
	case 4: cedi = vcpu.edi;break;
	default:_impossible_;break;}
	switch (byte) {
	case 1: _bb("byte(1)");
		vcpuins.bit = 8;
		_chk(_m_write_logical(&vcpu.es, cedi, vcpu.al, 1));
		_chk(_kas_move_index(1, 0, 1));
		_be;break;
	case 2: _bb("byte(2)");
		vcpuins.bit = 16;
		_chk(_m_write_logical(&vcpu.es, cedi, vcpu.ax, 2));
		_chk(_kas_move_index(2, 0, 1));
		_be;break;
	case 4: _bb("byte(4)");
		_newins_;
		vcpuins.bit = 32;
		_chk(_m_write_logical(&vcpu.es, cedi, vcpu.eax, 4));
		_chk(_kas_move_index(4, 0, 1));
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_ce;
}
tots _m_lods(t_nubit8 byte)
{
	t_nubit32 cesi = 0x00000000;
	_cb("_m_lods");
	switch (_GetAddressSize) {
	case 2: cesi = vcpu.si; break;
	case 4: cesi = vcpu.esi; break;
	default:_impossible_;break;}
	switch (byte) {
	case 1: _bb("byte(1)");
		vcpuins.bit = 8;
		_chk(vcpu.al = (t_nubit8)_m_read_logical(vcpuins.roverds, cesi, 1));
		_chk(_kas_move_index(1, 1, 0));
		_be;break;
	case 2: _bb("byte(2)");
		vcpuins.bit = 16;
		_chk(vcpu.ax = (t_nubit16)_m_read_logical(vcpuins.roverds, cesi, 2));
		_chk(_kas_move_index(2, 1, 0));
		_be;break;
	case 4: _bb("byte(4)");
		_newins_;
		vcpuins.bit = 32;
		_chk(vcpu.eax = (t_nubit32)_m_read_logical(vcpuins.roverds, cesi, 4));
		_chk(_kas_move_index(4, 1, 0));
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_ce;
}
tots _a_scas(t_nubit8 bit)
{
	t_nubit32 cedi = 0x00000000;
	_cb("_a_scas");
	switch (_GetAddressSize) {
	case 2: cedi = vcpu.di;break;
	case 4: cedi = vcpu.edi;break;
	default:_impossible_;break;}
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		vcpuins.type = CMP8;
		_chk(vcpuins.opr1 = vcpu.al);
		_chk(vcpuins.opr2 = (t_nubit8)_m_read_logical(&vcpu.es, cedi, 1));
		vcpuins.result = GetMax8(vcpuins.opr1 - vcpuins.opr2);
		_chk(_kas_move_index(1, 0, 1));
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		_chk(vcpuins.opr1 = vcpu.ax);
		_chk(vcpuins.opr2 = (t_nubit16)_m_read_logical(&vcpu.es, cedi, 2));
		vcpuins.result = GetMax16(vcpuins.opr1 - vcpuins.opr2);
		_chk(_kas_move_index(2, 0, 1));
		_be;break;
	case 32: _bb("bit(32)");
		_newins_;
		vcpuins.bit = 32;
		vcpuins.type = CMP32;
		_chk(vcpuins.opr1 = vcpu.eax);
		_chk(vcpuins.opr2 = (t_nubit32)_m_read_logical(&vcpu.es, cedi, 4));
		vcpuins.result = GetMax32(vcpuins.opr1 - vcpuins.opr2);
		_chk(_kas_move_index(4, 0, 1));
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_chk(_kaf_set_flags(CMP_FLAG));
	_ce;
}

#define _adv _chk(_d_skip(1))

#define JCC(rsrc, condition, bit) _chk(_e_jcc((rsrc), ((bit) >> 3), (condition)))
#define MOV(rdest, rsrc, bit)  _chk(_m_mov((t_vaddrcc)(rdest), (t_vaddrcc)(rsrc), ((bit) >> 3)))
#define PUSH(rsrc, bit) _chk(_e_push((t_vaddrcc)(rsrc), ((bit) >> 3)))
#define POP(rdest, bit) _chk(_e_pop((t_vaddrcc)(rdest), ((bit) >> 3)))

done UndefinedOpcode()
{
	_cb("UndefinedOpcode");
	vcpu.cs = vcpuins.oldcs;
	vcpu.eip = vcpuins.oldeip;
	if (!_GetCR0_PE) {
		vapiPrint("The NXVM CPU has encountered an illegal instruction.\n");
		vapiPrint("CS:%04X IP:%04X OP:%02X %02X %02X %02X\n",
			vcpu.cs.selector, vcpu.eip, _s_read_cs(vcpu.eip+0, 1),
			_s_read_cs(vcpu.eip+1, 1),  _s_read_cs(vcpu.eip+2, 1),
			_s_read_cs(vcpu.eip+3, 1),  _s_read_cs(vcpu.eip+4, 1));
		vapiCallBackMachineStop();
	}
	_chk(_SetExcept_UD(0));
	_ce;
}
done ADD_RM8_R8()
{
	_cb("ADD_RM8_R8");
	i386(0x00) {
		_adv;
		_chk(_d_modrm(1, 1, 1));
		_chk(_a_add(vcpuins.rrm, vcpuins.rr, 8));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_a_add(vcpuins.rrm, vcpuins.rr, 8);
	}
	_ce;
}
done ADD_RM16_R16()
{
	_cb("ADD_RM16_R16");
	i386(0x01) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 1));
		_chk(_a_add(vcpuins.rrm, vcpuins.rr, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_a_add(vcpuins.rrm, vcpuins.rr, 16);
	}
	_ce;
}
done ADD_R8_RM8()
{
	_cb("ADD_R8_RM8");
	i386(0x02) {
		_adv;
		_chk(_d_modrm(1, 1, 0));
		_chk(_a_add(vcpuins.rr, vcpuins.rrm, 8));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_a_add(vcpuins.rr, vcpuins.rrm, 8);
	}
	_ce;
}
done ADD_R16_RM16()
{
	_cb("ADD_R16_RM16");
	i386(0x03) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
		_chk(_a_add(vcpuins.rr, vcpuins.rrm, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_a_add(vcpuins.rr, vcpuins.rrm, 16);
	}
	_ce;
}
done ADD_AL_I8()
{
	_cb("ADD_AL_I8");
	i386(0x04) {
		_adv;
		_chk(_d_imm(1));
		_chk(_a_add((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8));
	} else {
		vcpu.ip++;
		_d_imm(1);
		_a_add((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8);
	}
	_ce;
}
done ADD_AX_I16()
{
	_cb("ADD_AX_I16");
	i386(0x05) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_a_add((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_d_imm(4));
			_chk(_a_adc((t_vaddrcc)&vcpu.eax, vcpuins.rimm, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(2);
		_a_add((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16);
	}
	_ce;
}
done PUSH_ES()
{
	/* note: not sure if operand size is 32,
		push/pop selector only or with higher 16 bit */
	t_nubit32 oldes = vcpu.es.selector;
	_cb("PUSH_ES");
	i386(0x06) {
		_adv;
		_chk(_e_push((t_vaddrcc)&oldes, _GetOperandSize));
	} else {
		vcpu.ip++;
		_e_push((t_vaddrcc)&vcpu.es.selector, 2);
	}
	_ce;
}
done POP_ES()
{
	/* note: not sure if operand size is 32,
		push/pop selector only or with higher 16 bit */
	t_nubit32 newes = 0x00000000;
	_cb("POP_ES");
	i386(0x07) {
		_adv;
		_chk(_e_pop((t_vaddrcc)&newes, _GetOperandSize));
		_chk(_s_load_es(GetMax16(newes)));
	} else {
		vcpu.ip++;
		_e_pop((t_vaddrcc)&vcpu.es.selector, 2);
		_s_load_es(vcpu.es.selector);
	}
	_ce;
}
done OR_RM8_R8()
{
	_cb("OR_RM8_R8");
	i386(0x08) {
		_adv;
		_chk(_d_modrm(1, 1, 1));
		_chk(_a_or(vcpuins.rrm, vcpuins.rr, 8));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_a_or(vcpuins.rrm, vcpuins.rr, 8);
	}
	_ce;
}
done OR_RM16_R16()
{
	_cb("OR_RM16_R16");
	i386(0x09) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 1));
		_chk(_a_or(vcpuins.rrm, vcpuins.rr, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_a_or(vcpuins.rrm, vcpuins.rr, 16);
	}
	_ce;
}
done OR_R8_RM8()
{
	_cb("OR_R8_RM8");
	i386(0x0a) {
		_adv;
		_chk(_d_modrm(1, 1, 0));
		_chk(_a_or(vcpuins.rr, vcpuins.rrm, 8));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_a_or(vcpuins.rr, vcpuins.rrm, 8);
	}
	_ce;
}
done OR_R16_RM16()
{
	_cb("OR_R16_RM16");
	i386(0x0b) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
		_chk(_a_or(vcpuins.rr, vcpuins.rrm, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_a_or(vcpuins.rr, vcpuins.rrm, 16);
	}
	_ce;
}
done OR_AL_I8()
{
	_cb("OR_AL_I8");
	i386(0x0c) {
		_adv;
		_chk(_d_imm(1));
		_chk(_a_or((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8));
	} else {
		vcpu.ip++;
		_d_imm(1);
		_a_or((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8);
	}
	_ce;
}
tots OR_AX_I16()
{
	_cb("OR_AX_I16");
	i386(0x0d) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_a_or((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_d_imm(4));
			_chk(_a_adc((t_vaddrcc)&vcpu.eax, vcpuins.rimm, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(2);
		_a_or((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16);
	}
	_ce;
}
done PUSH_CS()
{
	/* note: not sure if operand size is 32,
		push/pop selector only or with higher 16 bit */
	t_nubit32 oldcs = vcpu.cs.selector;
	_cb("PUSH_CS");
	i386(0x0e) {
		_adv;
		_chk(_e_push((t_vaddrcc)&oldcs, _GetOperandSize));
	} else {
		vcpu.ip++;
		_e_push((t_vaddrcc)&vcpu.cs.selector, 2);
	}
	_ce;
}
tots POP_CS()
{
	t_nubit32 newcs = 0x00000000;
	_cb("POP_CS");
	_newins_;
	i386(0x0f) {
		_bb("i386");
		_chk(_SetExcept_CE(0x0f));
		_be;
	} else {
		_adv;
		_chk(_e_pop((t_vaddrcc)&newcs, _GetOperandSize));
		_chk(_s_load_cs(GetMax16(newcs)));
	}
	_ce;
}
done INS_0F()
{
	t_nubit8 opcode = 0x00;
	_cb("INS_0F");
	i386(0x0f) {
		_adv;
		_chk(opcode = (t_nubit8)_s_read_cs(vcpu.eip, 1));
		_chk(ExecFun(vcpuins.table_0f[opcode]));
	} else
		POP_CS();
	_ce;
}
tots ADC_RM8_R8()
{
	_cb("ADC_RM8_R8");
	_newins_;
	i386(0x10) {
		_adv;
		_chk(_d_modrm(1, 1, 1));
		_chk(_a_adc(vcpuins.rrm, vcpuins.rr, 8));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_a_adc(vcpuins.rrm, vcpuins.rr, 8); 
	}
	_ce;
}
done ADC_RM16_R16()
{
	_cb("ADC_RM16_R16");
	i386(0x11) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 1));
		_chk(_a_adc(vcpuins.rrm, vcpuins.rr, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_a_adc(vcpuins.rrm, vcpuins.rr, 16);
	}
	_ce;
}
done ADC_R8_RM8()
{
	_cb("ADC_R8_RM8");
	i386(0x12) {
		_adv;
		_chk(_d_modrm(1, 1, 0));
		_chk(_a_adc(vcpuins.rr, vcpuins.rrm, 8));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_a_adc(vcpuins.rr, vcpuins.rrm, 8);
	}
	_ce;
}
done ADC_R16_RM16()
{
	_cb("ADC_R16_RM16");
	i386(0x13) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
		_chk(_a_adc(vcpuins.rr, vcpuins.rrm, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_a_adc(vcpuins.rr, vcpuins.rrm, 16);
	}
	_ce;
}
done ADC_AL_I8()
{
	_cb("ADC_AL_I8");
	i386(0x14) {
		_adv;
		_chk(_d_imm(1));
		_chk(_a_adc((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8));
	} else {
		vcpu.ip++;
		_d_imm(1);
		_a_adc((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8);
	}
	_ce;
}
tots ADC_AX_I16()
{
	_cb("ADC_AX_I16");
	i386(0x15) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_a_adc((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_d_imm(4));
			_chk(_a_adc((t_vaddrcc)&vcpu.eax, vcpuins.rimm, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(2);
		_a_adc((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16);
	}
	_ce;
}
done PUSH_SS()
{
	/* note: not sure if operand size is 32,
		push/pop selector only or with higher 16 bit */
	t_nubit32 oldss = vcpu.ss.selector;
	_cb("PUSH_SS");
	i386(0x16) {
		_adv;
		_chk(_e_push((t_vaddrcc)&oldss, _GetOperandSize));
	} else {
		vcpu.ip++;
		_e_push((t_vaddrcc)&vcpu.ss.selector, 2);
	}
	_ce;
}
done POP_SS()
{
	/* note: not sure if operand size is 32,
		push/pop selector only or with higher 16 bit */
	t_nubit32 newss = 0x00000000;
	_cb("POP_SS");
	i386(0x17) {
		_adv;
		_chk(_e_pop((t_vaddrcc)&newss, _GetOperandSize));
		_chk(_s_load_ss(GetMax16(newss)));
	} else {
		vcpu.ip++;
		_e_pop((t_vaddrcc)&vcpu.ss.selector, 2);
		_s_load_ss(vcpu.ss.selector);
	}
	_ce;
}
tots SBB_RM8_R8()
{
	_cb("SBB_RM8_R8");
	_newins_;
	i386(0x18) {
		_adv;
		_chk(_d_modrm(1, 1, 1));
		_chk(_a_sbb(vcpuins.rrm, vcpuins.rr, 8));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_a_sbb(vcpuins.rrm, vcpuins.rr, 8); 
	}
	_ce;
}
done SBB_RM16_R16()
{
	_cb("SBB_RM16_R16");
	i386(0x19) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 1));
		_chk(_a_sbb(vcpuins.rrm, vcpuins.rr, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_a_sbb(vcpuins.rrm, vcpuins.rr, 16);
	}
	_ce;
}
done SBB_R8_RM8()
{
	_cb("SBB_R8_RM8");
	i386(0x1a) {
		_adv;
		_chk(_d_modrm(1, 1, 0));
		_chk(_a_sbb(vcpuins.rr, vcpuins.rrm, 8));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_a_sbb(vcpuins.rr, vcpuins.rrm, 8);
	}
	_ce;
}
done SBB_R16_RM16()
{
	_cb("SBB_R16_RM16");
	i386(0x1b) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
		_chk(_a_sbb(vcpuins.rr, vcpuins.rrm, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_a_sbb(vcpuins.rr, vcpuins.rrm, 16);
	}
	_ce;
}
tots SBB_AL_I8()
{
	_cb("SBB_AL_I8");
	_newins_;
	i386(0x1c) {
		_adv;
		_chk(_d_imm(1));
		_chk(_a_sbb((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8));
	} else {
		vcpu.ip++;
		_d_imm(1);
		_a_sbb((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8);
	}
	_ce;
}
tots SBB_AX_I16()
{
	_cb("SBB_AX_I16");
	_newins_;
	i386(0x1d) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_a_sbb((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_d_imm(4));
			_chk(_a_sbb((t_vaddrcc)&vcpu.eax, vcpuins.rimm, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(2);
		_a_sbb((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16);
	}
	_ce;
}
done PUSH_DS()
{
	/* note: not sure if operand size is 32,
		push/pop selector only or with higher 16 bit */
	t_nubit32 oldds = vcpu.ds.selector;
	_cb("PUSH_DS");
	i386(0x1e) {
		_adv;
		_chk(_e_push((t_vaddrcc)&oldds, _GetOperandSize));
	} else {
		vcpu.ip++;
		_e_push((t_vaddrcc)&vcpu.ds.selector, 2);
	}
	_ce;
}
done POP_DS()
{
	/* note: not sure if operand size is 32,
		push/pop selector only or with higher 16 bit */
	t_nubit32 newds = 0x00000000;
	_cb("POP_DS");
	i386(0x1f) {
		_adv;
		_chk(_e_pop((t_vaddrcc)&newds, _GetOperandSize));
		_chk(_s_load_ds(GetMax16(newds)));
	} else {
		vcpu.ip++;
		_e_pop((t_vaddrcc)&vcpu.ds.selector, 2);
		_s_load_ds(vcpu.ds.selector);
	}
	_ce;
}
done AND_RM8_R8()
{
	_cb("AND_RM8_R8");
	i386(0x20) {
		_adv;
		_chk(_d_modrm(1, 1, 1));
		_chk(_a_and(vcpuins.rrm, vcpuins.rr, 8));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_a_and(vcpuins.rrm, vcpuins.rr, 8);
	}
	_ce;
}
done AND_RM16_R16()
{
	_cb("AND_RM16_R16");
	i386(0x21) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 1));
		_chk(_a_and(vcpuins.rrm, vcpuins.rr, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_a_and(vcpuins.rrm, vcpuins.rr, 16);
	}
	_ce;
}
done AND_R8_RM8()
{
	_cb("AND_R8_RM8");
	i386(0x22) {
		_adv;
		_chk(_d_modrm(1, 1, 0));
		_chk(_a_and(vcpuins.rr, vcpuins.rrm, 8));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_a_and(vcpuins.rr, vcpuins.rrm, 8);
	}
	_ce;
}
done AND_R16_RM16()
{
	_cb("AND_R16_RM16");
	i386(0x23) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
		_chk(_a_and(vcpuins.rr, vcpuins.rrm, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_a_and(vcpuins.rr, vcpuins.rrm, 16);
	}
	_ce;
}
done AND_AL_I8()
{
	_cb("AND_AL_I8");
	i386(0x24) {
		_adv;
		_chk(_d_imm(1));
		_chk(_a_and((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8));
	} else {
		vcpu.ip++;
		_d_imm(1);
		_a_and((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8);
	}
	_ce;
}
tots AND_AX_I16()
{
	_cb("AND_AX_I16");
	i386(0x25) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_a_and((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_imm(4));
			_chk(_a_and((t_vaddrcc)&vcpu.eax, vcpuins.rimm, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(2);
		_a_and((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16);
	}
	_ce;
}
done PREFIX_ES()
{
	_cb("PREFIX_ES");
	i386(0x26) {
		_adv;
		vcpuins.roverds = &vcpu.es;
		vcpuins.roverss = &vcpu.es;
	} else {
		vcpu.ip++;
		vcpuins.roverds = &vcpu.es;
		vcpuins.roverss = &vcpu.es;
	}
	_ce;
}
done DAA()
{
	t_nubit8 oldAL = vcpu.al;
	_cb("DAA");
	i386(0x27)
		_adv;
	else
		vcpu.ip++;
	if(((vcpu.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
		vcpu.al += 0x06;
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF,
			_GetEFLAGS_CF || ((vcpu.al < oldAL) || (vcpu.al < 0x06)));
		_SetEFLAGS_AF;
	} else _ClrEFLAGS_AF;
	if(((vcpu.al & 0xf0) > 0x90) || _GetEFLAGS_CF) {
		vcpu.al += 0x60;
		_SetEFLAGS_CF;
	} else _ClrEFLAGS_CF;
	vcpuins.bit = 8;
	vcpuins.result = (t_nubit32)vcpu.al;
	_chk(_kaf_set_flags(DAA_FLAG));
	vcpuins.udf |= VCPU_EFLAGS_OF;
	_ce;
}
done SUB_RM8_R8()
{
	_cb("SUB_RM8_R8");
	i386(0x28) {
		_adv;
		_chk(_d_modrm(1, 1, 1));
		_chk(_a_sub(vcpuins.rrm, vcpuins.rr, 8));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_a_sub(vcpuins.rrm, vcpuins.rr, 8); 
	}
	_ce;
}
done SUB_RM16_R16()
{
	_cb("SUB_RM16_R16");
	i386(0x29) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 1));
		_chk(_a_sub(vcpuins.rrm, vcpuins.rr, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_a_sub(vcpuins.rrm, vcpuins.rr, 16);
	}
	_ce;
}
done SUB_R8_RM8()
{
	_cb("SUB_R8_RM8");
	i386(0x2a) {
		_adv;
		_chk(_d_modrm(1, 1, 0));
		_chk(_a_sub(vcpuins.rr, vcpuins.rrm, 8));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_a_sub(vcpuins.rr, vcpuins.rrm, 8);
	}
	_ce;
}
done SUB_R16_RM16()
{
	_cb("SUB_R16_RM16");
	i386(0x2b) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
		_chk(_a_sub(vcpuins.rr, vcpuins.rrm, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_a_sub(vcpuins.rr, vcpuins.rrm, 16);
	}
	_ce;
}
done SUB_AL_I8()
{
	_cb("SUB_AL_I8");
	i386(0x2c) {
		_adv;
		_chk(_d_imm(1));
		_chk(_a_sub((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8));
	} else {
		vcpu.ip++;
		_d_imm(1);
		_a_sub((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8);
	}
	_ce;
}
tots SUB_AX_I16()
{
	_cb("SUB_AX_I16");
	i386(0x2d) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_a_sub((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_d_imm(4));
			_chk(_a_sub((t_vaddrcc)&vcpu.eax, vcpuins.rimm, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(2);
		_a_sub((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16);
	}
	_ce;
}
done PREFIX_CS()
{
	_cb("PREFIX_CS");
	i386(0x2e) {
		_adv;
		vcpuins.roverds = &vcpu.cs;
		vcpuins.roverss = &vcpu.cs;
	} else {
		vcpu.ip++;
		vcpuins.roverds = &vcpu.cs;
		vcpuins.roverss = &vcpu.cs;
	}
	_ce;
}
tots DAS()
{
	t_nubit8 oldAL = vcpu.al;
	_cb("DAS");
	_newins_;
	i386(0x2f)
		_adv;
	else
		vcpu.ip++;
	if(((vcpu.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
		vcpu.al -= 0x06;
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF,
			_GetEFLAGS_CF || (oldAL < 0x06));
		_SetEFLAGS_AF;
	} else _ClrEFLAGS_AF;
	if((vcpu.al > 0x9f) || _GetEFLAGS_CF) {
		vcpu.al -= 0x60;
		_SetEFLAGS_CF;
	} else _ClrEFLAGS_CF;
	vcpuins.bit = 8;
	vcpuins.result = (t_nubit32)vcpu.al;
	_chk(_kaf_set_flags(DAS_FLAG));
	vcpuins.udf |= VCPU_EFLAGS_OF;
	_ce;
}
done XOR_RM8_R8()
{
	_cb("XOR_RM8_R8");
	i386(0x30) {
		_adv;
		_chk(_d_modrm(1, 1, 1));
		_chk(_a_xor(vcpuins.rrm, vcpuins.rr, 8));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_a_xor(vcpuins.rrm, vcpuins.rr, 8);
	}
	_ce;
}
done XOR_RM16_R16()
{
	_cb("XOR_RM16_R16");
	i386(0x31) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 1));
		_chk(_a_xor(vcpuins.rrm, vcpuins.rr, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_a_xor(vcpuins.rrm, vcpuins.rr, 16);
	}
	_ce;
}
done XOR_R8_RM8()
{
	_cb("XOR_R8_RM8");
	i386(0x32) {
		_adv;
		_chk(_d_modrm(1, 1, 0));
		_chk(_a_xor(vcpuins.rr, vcpuins.rrm, 8));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_a_xor(vcpuins.rr, vcpuins.rrm, 8);
	}
	_ce;
}
done XOR_R16_RM16()
{
	_cb("XOR_R16_RM16");
	i386(0x33) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
		_chk(_a_xor(vcpuins.rr, vcpuins.rrm, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_a_xor(vcpuins.rr, vcpuins.rrm, 16);
	}
	_ce;
}
tots XOR_AL_I8()
{
	_cb("XOR_AL_I8");
	_newins_;
	i386(0x34) {
		_adv;
		_chk(_d_imm(1));
		_chk(_a_xor((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8));
	} else {
		vcpu.ip++;
		_d_imm(1);
		_a_xor((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8);
	}
	_ce;
}
tots XOR_AX_I16()
{
	_cb("XOR_AX_I16");
	i386(0x35) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_a_xor((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_d_imm(4));
			_chk(_a_xor((t_vaddrcc)&vcpu.eax, vcpuins.rimm, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(2);
		_a_xor((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16);
	}
	_ce;
}
done PREFIX_SS()
{
	_cb("PREFIX_SS");
	i386(0x36) {
		_adv;
		vcpuins.roverds = &vcpu.ss;
		vcpuins.roverss = &vcpu.ss;
	} else {
		vcpu.ip++;
		vcpuins.roverds = &vcpu.ss;
		vcpuins.roverss = &vcpu.ss;
	}
	_ce;
}
tots AAA()
{
	_cb("AAA");
	_newins_;
	i386(0x37) {
		_adv;
	} else {
		vcpu.ip++;
	}
	if(((vcpu.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
		vcpu.al += 0x06;
		vcpu.ah += 0x01;
		_SetEFLAGS_AF;
		_SetEFLAGS_CF;
	} else {
		_ClrEFLAGS_AF;
		_ClrEFLAGS_CF;
	}
	vcpu.al &= 0x0f;
	vcpuins.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF);
	_ce;
}
done CMP_RM8_R8()
{
	_cb("CMP_RM8_R8");
	i386(0x38) {
		_adv;
		_chk(_d_modrm(1, 1, 0));
		_chk(_a_cmp(vcpuins.rrm, vcpuins.rr, 8));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_a_cmp(vcpuins.rrm, vcpuins.rr, 8);
	}
	_ce;
}
done CMP_RM16_R16()
{
	_cb("CMP_RM16_R16");
	i386(0x39) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
		_chk(_a_cmp(vcpuins.rrm, vcpuins.rr, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_a_cmp(vcpuins.rrm, vcpuins.rr, 16);
	}
	_ce;
}
done CMP_R8_RM8()
{
	_cb("CMP_R8_RM8");
	i386(0x3a) {
		_adv;
		_chk(_d_modrm(1, 1, 0));
		_chk(_a_cmp(vcpuins.rr, vcpuins.rrm, 8));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_a_cmp(vcpuins.rr, vcpuins.rrm, 8);
	}
	_ce;
}
done CMP_R16_RM16()
{
	_cb("CMP_R16_RM16");
	i386(0x3b) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
		_chk(_a_cmp(vcpuins.rr, vcpuins.rrm, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_a_cmp(vcpuins.rr, vcpuins.rrm, 16);
	}
	_ce;
}
done CMP_AL_I8()
{
	_cb("CMP_AL_I8");
	i386(0x3c) {
		_adv;
		_chk(_d_imm(1));
		_chk(_a_cmp((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8));
	} else {
		vcpu.ip++;
		_d_imm(1);
		_a_cmp((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8);
	}
	_ce;
}
tots CMP_AX_I16()
{
	_cb("CMP_AX_I16");
	i386(0x3d) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_a_cmp((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_d_imm(4));
			_chk(_a_cmp((t_vaddrcc)&vcpu.eax, vcpuins.rimm, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(2);
		_a_cmp((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16);
	}
	_ce;
}
done PREFIX_DS()
{
	_cb("PREFIX_DS");
	i386(0x3e) {
		_adv;
		vcpuins.roverds = &vcpu.ds;
		vcpuins.roverss = &vcpu.ds;
	} else {
		vcpu.ip++;
		vcpuins.roverds = &vcpu.ds;
		vcpuins.roverss = &vcpu.ds;
	}
	_ce;
}
tots AAS()
{
	_cb("AAS");
	_newins_;
	i386(0x3f) {
		_adv;
	} else {
		vcpu.ip++;
	}
	if(((vcpu.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
		vcpu.al -= 0x06;
		vcpu.ah -= 0x01;
		_SetEFLAGS_AF;
		_SetEFLAGS_CF;
	} else {
		_ClrEFLAGS_CF;
		_ClrEFLAGS_AF;
	}
	vcpu.al &= 0x0f;
	vcpuins.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF);
	_ce;
}
done INC_AX()
{
	_cb("INC_AX");
	i386(0x40) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_a_inc((t_vaddrcc)&vcpu.ax, 16));break;
		case 4: _chk(_a_inc((t_vaddrcc)&vcpu.eax, 32));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_inc((t_vaddrcc)&vcpu.ax, 16);
	}
	_ce;
}
done INC_CX()
{
	_cb("INC_CX");
	i386(0x41) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_a_inc((t_vaddrcc)&vcpu.cx, 16));break;
		case 4: _chk(_a_inc((t_vaddrcc)&vcpu.ecx, 32));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_inc((t_vaddrcc)&vcpu.cx, 16);
	}
	_ce;
}
done INC_DX()
{
	_cb("INC_DX");
	i386(0x42) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_a_inc((t_vaddrcc)&vcpu.dx, 16));break;
		case 4: _chk(_a_inc((t_vaddrcc)&vcpu.edx, 32));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_inc((t_vaddrcc)&vcpu.dx, 16);
	}
	_ce;
}
done INC_BX()
{
	_cb("INC_BX");
	i386(0x43) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_a_inc((t_vaddrcc)&vcpu.bx, 16));break;
		case 4: _chk(_a_inc((t_vaddrcc)&vcpu.ebx, 32));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_inc((t_vaddrcc)&vcpu.bx, 16);
	}
	_ce;
}
done INC_SP()
{
	_cb("INC_SP");
	i386(0x44) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_a_inc((t_vaddrcc)&vcpu.sp, 16));break;
		case 4: _chk(_a_inc((t_vaddrcc)&vcpu.esp, 32));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_inc((t_vaddrcc)&vcpu.sp, 16);
	}
	_ce;
}
done INC_BP()
{
	_cb("INC_BP");
	i386(0x45) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_a_inc((t_vaddrcc)&vcpu.bp, 16));break;
		case 4: _chk(_a_inc((t_vaddrcc)&vcpu.ebp, 32));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_inc((t_vaddrcc)&vcpu.bp, 16);
	}
	_ce;
}
done INC_SI()
{
	_cb("INC_SI");
	i386(0x46) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_a_inc((t_vaddrcc)&vcpu.si, 16));break;
		case 4: _chk(_a_inc((t_vaddrcc)&vcpu.esi, 32));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_inc((t_vaddrcc)&vcpu.si, 16);
	}
	_ce;
}
done INC_DI()
{
	_cb("INC_DI");
	i386(0x47) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_a_inc((t_vaddrcc)&vcpu.di, 16));break;
		case 4: _chk(_a_inc((t_vaddrcc)&vcpu.edi, 32));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_inc((t_vaddrcc)&vcpu.di, 16);
	}
	_ce;
}
done DEC_AX()
{
	_cb("DEC_AX");
	i386(0x48) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_a_dec((t_vaddrcc)&vcpu.ax, 16));break;
		case 4: _chk(_a_dec((t_vaddrcc)&vcpu.eax, 32));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_dec((t_vaddrcc)&vcpu.ax, 16);
	}
	_ce;
}
done DEC_CX()
{
	_cb("DEC_CX");
	i386(0x49) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_a_dec((t_vaddrcc)&vcpu.cx, 16));break;
		case 4: _chk(_a_dec((t_vaddrcc)&vcpu.ecx, 32));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_dec((t_vaddrcc)&vcpu.cx, 16);
	}
	_ce;
}
done DEC_DX()
{
	_cb("DEC_DX");
	i386(0x4a) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_a_dec((t_vaddrcc)&vcpu.dx, 16));break;
		case 4: _chk(_a_dec((t_vaddrcc)&vcpu.edx, 32));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_dec((t_vaddrcc)&vcpu.dx, 16);
	}
	_ce;
}
done DEC_BX()
{
	_cb("DEC_BX");
	i386(0x4b) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_a_dec((t_vaddrcc)&vcpu.bx, 16));break;
		case 4: _chk(_a_dec((t_vaddrcc)&vcpu.ebx, 32));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_dec((t_vaddrcc)&vcpu.bx, 16);
	}
	_ce;
}
done DEC_SP()
{
	_cb("DEC_SP");
	i386(0x4c) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_a_dec((t_vaddrcc)&vcpu.sp, 16));break;
		case 4: _chk(_a_dec((t_vaddrcc)&vcpu.esp, 32));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_dec((t_vaddrcc)&vcpu.sp, 16);
	}
	_ce;
}
done DEC_BP()
{
	_cb("DEC_BP");
	i386(0x4d) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_a_dec((t_vaddrcc)&vcpu.bp, 16));break;
		case 4: _chk(_a_dec((t_vaddrcc)&vcpu.ebp, 32));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_dec((t_vaddrcc)&vcpu.bp, 16);
	}
	_ce;
}
done DEC_SI()
{
	_cb("DEC_SI");
	i386(0x4e) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_a_dec((t_vaddrcc)&vcpu.si, 16));break;
		case 4: _chk(_a_dec((t_vaddrcc)&vcpu.esi, 32));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_dec((t_vaddrcc)&vcpu.si, 16);
	}
	_ce;
}
done DEC_DI()
{
	_cb("DEC_DI");
	i386(0x4f) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_a_dec((t_vaddrcc)&vcpu.di, 16));break;
		case 4: _chk(_a_dec((t_vaddrcc)&vcpu.edi, 32));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_dec((t_vaddrcc)&vcpu.di, 16);
	}
	_ce;
}
done PUSH_AX()
{
	_cb("PUSH_AX");
	i386(0x50) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_push((t_vaddrcc)&vcpu.ax, 2));break;
		case 4: _chk(_e_push((t_vaddrcc)&vcpu.eax, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_push((t_vaddrcc)&vcpu.ax, 2);
	}
	_ce;
}
done PUSH_CX()
{
	_cb("PUSH_CX");
	i386(0x51) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_push((t_vaddrcc)&vcpu.cx, 2));break;
		case 4: _chk(_e_push((t_vaddrcc)&vcpu.ecx, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_push((t_vaddrcc)&vcpu.cx, 2);
	}
	_ce;
}
done PUSH_DX()
{
	_cb("PUSH_DX");
	i386(0x52) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_push((t_vaddrcc)&vcpu.dx, 2));break;
		case 4: _chk(_e_push((t_vaddrcc)&vcpu.edx, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_push((t_vaddrcc)&vcpu.dx, 2);
	}
	_ce;
}
done PUSH_BX()
{
	_cb("PUSH_BX");
	i386(0x53) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_push((t_vaddrcc)&vcpu.bx, 2));break;
		case 4: _chk(_e_push((t_vaddrcc)&vcpu.ebx, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_push((t_vaddrcc)&vcpu.bx, 2);
	}
	_ce;
}
done PUSH_SP()
{
	_cb("PUSH_SP");
	i386(0x54) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_push((t_vaddrcc)&vcpu.sp, 2));break;
		case 4: _chk(_e_push((t_vaddrcc)&vcpu.esp, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_push((t_vaddrcc)&vcpu.sp, 2);
	}
	_ce;
}
done PUSH_BP()
{
	_cb("PUSH_BP");
	i386(0x55) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_push((t_vaddrcc)&vcpu.bp, 2));break;
		case 4: _chk(_e_push((t_vaddrcc)&vcpu.ebp, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_push((t_vaddrcc)&vcpu.bp, 2);
	}
	_ce;
}
done PUSH_SI()
{
	_cb("PUSH_SI");
	i386(0x56) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_push((t_vaddrcc)&vcpu.si, 2));break;
		case 4: _chk(_e_push((t_vaddrcc)&vcpu.esi, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_push((t_vaddrcc)&vcpu.si, 2);
	}
	_ce;
}
done PUSH_DI()
{
	_cb("PUSH_DI");
	i386(0x57) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_push((t_vaddrcc)&vcpu.di, 2));break;
		case 4: _chk(_e_push((t_vaddrcc)&vcpu.edi, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_push((t_vaddrcc)&vcpu.di, 2);
	}
	_ce;
}
done POP_AX()
{
	_cb("POP_AX");
	i386(0x58) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_pop((t_vaddrcc)&vcpu.ax, 2));break;
		case 4: _chk(_e_pop((t_vaddrcc)&vcpu.eax, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_pop((t_vaddrcc)&vcpu.ax, 2);
	}
	_ce;
}
done POP_CX()
{
	_cb("POP_CX");
	i386(0x59) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_pop((t_vaddrcc)&vcpu.cx, 2));break;
		case 4: _chk(_e_pop((t_vaddrcc)&vcpu.ecx, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_pop((t_vaddrcc)&vcpu.cx, 2);
	}
	_ce;
}
done POP_DX()
{
	_cb("POP_DX");
	i386(0x5a) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_pop((t_vaddrcc)&vcpu.dx, 2));break;
		case 4: _chk(_e_pop((t_vaddrcc)&vcpu.edx, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_pop((t_vaddrcc)&vcpu.dx, 2);
	}
	_ce;
}
done POP_BX()
{
	_cb("POP_BX");
	i386(0x5b) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_pop((t_vaddrcc)&vcpu.bx, 2));break;
		case 4: _chk(_e_pop((t_vaddrcc)&vcpu.ebx, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_pop((t_vaddrcc)&vcpu.bx, 2);
	}
	_ce;
}
tots POP_SP()
{
	_cb("POP_SP");
	_newins_;
	i386(0x5c) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_pop((t_vaddrcc)&vcpu.sp, 2));break;
		case 4: _chk(_e_pop((t_vaddrcc)&vcpu.esp, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_pop((t_vaddrcc)&vcpu.sp, 2);
	}
	_ce;
}
done POP_BP()
{
	_cb("POP_BP");
	i386(0x5d) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_pop((t_vaddrcc)&vcpu.bp, 2));break;
		case 4: _chk(_e_pop((t_vaddrcc)&vcpu.ebp, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_pop((t_vaddrcc)&vcpu.bp, 2);
	}
	_ce;
}
done POP_SI()
{
	_cb("POP_SI");
	i386(0x5e) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_pop((t_vaddrcc)&vcpu.si, 2));break;
		case 4: _chk(_e_pop((t_vaddrcc)&vcpu.esi, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_pop((t_vaddrcc)&vcpu.si, 2);
	}
	_ce;
}
done POP_DI()
{
	_cb("POP_DI");
	i386(0x5f) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_pop((t_vaddrcc)&vcpu.di, 2));break;
		case 4: _chk(_e_pop((t_vaddrcc)&vcpu.edi, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_pop((t_vaddrcc)&vcpu.di, 2);
	}
	_ce;
}
tots PUSHA()
{
	t_nubit32 cesp = 0x00000000;
	_cb("PUSHA");
	_newins_;
	i386(0x60) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			cesp = vcpu.sp;
			_chk(_e_push((t_vaddrcc)&vcpu.ax, 2));
			_chk(_e_push((t_vaddrcc)&vcpu.cx, 2));
			_chk(_e_push((t_vaddrcc)&vcpu.dx, 2));
			_chk(_e_push((t_vaddrcc)&vcpu.bx, 2));
			_chk(_e_push((t_vaddrcc)&cesp,    2));
			_chk(_e_push((t_vaddrcc)&vcpu.bp, 2));
			_chk(_e_push((t_vaddrcc)&vcpu.si, 2));
			_chk(_e_push((t_vaddrcc)&vcpu.di, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			cesp = vcpu.esp;
			_chk(_e_push((t_vaddrcc)&vcpu.eax, 4));
			_chk(_e_push((t_vaddrcc)&vcpu.ecx, 4));
			_chk(_e_push((t_vaddrcc)&vcpu.edx, 4));
			_chk(_e_push((t_vaddrcc)&vcpu.ebx, 4));
			_chk(_e_push((t_vaddrcc)&cesp,     4));
			_chk(_e_push((t_vaddrcc)&vcpu.ebp, 4));
			_chk(_e_push((t_vaddrcc)&vcpu.esi, 4));
			_chk(_e_push((t_vaddrcc)&vcpu.edi, 4));
			_be;break;
		default:_impossible_;break;}
	} else
		UndefinedOpcode();
	_ce;
}
tots POPA()
{
	t_nubit32 cesp = 0x00000000;
	_cb("POPA");
	_newins_;
	i386(0x61) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_e_pop((t_vaddrcc)&vcpu.di, 2));
			_chk(_e_pop((t_vaddrcc)&vcpu.si, 2));
			_chk(_e_pop((t_vaddrcc)&vcpu.bp, 2));
			_chk(_e_pop((t_vaddrcc)&cesp,    2));
			_chk(_e_pop((t_vaddrcc)&vcpu.bx, 2));
			_chk(_e_pop((t_vaddrcc)&vcpu.dx, 2));
			_chk(_e_pop((t_vaddrcc)&vcpu.cx, 2));
			_chk(_e_pop((t_vaddrcc)&vcpu.ax, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_e_pop((t_vaddrcc)&vcpu.edi, 4));
			_chk(_e_pop((t_vaddrcc)&vcpu.esi, 4));
			_chk(_e_pop((t_vaddrcc)&vcpu.ebp, 4));
			_chk(_e_pop((t_vaddrcc)&cesp,     4));
			_chk(_e_pop((t_vaddrcc)&vcpu.ebx, 4));
			_chk(_e_pop((t_vaddrcc)&vcpu.edx, 4));
			_chk(_e_pop((t_vaddrcc)&vcpu.ecx, 4));
			_chk(_e_pop((t_vaddrcc)&vcpu.eax, 4));
			_be;break;
		default:_impossible_;break;}
	} else
		UndefinedOpcode();
	_ce;
}
tots BOUND_R16_M16_16()
{
	t_nsbit16 a16,l16,u16;
	t_nsbit32 a32,l32,u32;
	_cb("BOUND_R16_M16_16");
	_newins_;
	i386(0x62) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2, 0x00));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(16)");
			a16 = d_nsbit16(vcpuins.rr);
			l16 = d_nsbit16(vcpuins.rrm);
			u16 = d_nsbit16(vcpuins.rrm + 2);
			if (a16 < l16 || a16 > u16)
				_chk(_SetExcept_BR(0));
			_be;break;
		case 4: _bb("OperandSize(32)");
			a32 = d_nsbit32(vcpuins.rr);
			l32 = d_nsbit32(vcpuins.rrm);
			u32 = d_nsbit32(vcpuins.rrm + 4);
			if (a32 < l32 || a32 > u32)
				_chk(_SetExcept_BR(0));
			_be;break;
		default:_impossible_;break;}
	} else
		UndefinedOpcode();
	_ce;
}
tots ARPL_RM16_R16()
{
	t_nubit8 drpl, srpl;
	_cb("ARPL_RM16_R16");
	_newins_;
	i386(0x63) {
		if (_IsProtected) {
			_adv;
			_chk(_d_modrm(2, 2, 0x01));
			drpl = _GetSelector_RPL(d_nubit16(vcpuins.rrm));
			srpl = _GetSelector_RPL(d_nubit16(vcpuins.rr));
			if (drpl < srpl) {
				_SetEFLAGS_ZF;
				d_nubit16(vcpuins.rrm) = (d_nubit16(vcpuins.rrm) & ~VCPU_SELECTOR_RPL) | srpl;
			} else
				_ClrEFLAGS_ZF;
		} else
			_chk(UndefinedOpcode());
	} else
		UndefinedOpcode();
	_ce;
}
tots PREFIX_FS()
{
	_cb("PREFIX_FS");
	_newins_;
	i386(0x64) {
		_adv;
		vcpuins.roverds = &vcpu.fs;
		vcpuins.roverss = &vcpu.fs;
	} else
		UndefinedOpcode();
	_ce;
}
tots PREFIX_GS()
{
	_cb("PREFIX_GS");
	_newins_;
	i386(0x65) {
		_adv;
		vcpuins.roverds = &vcpu.gs;
		vcpuins.roverss = &vcpu.gs;
	} else
		UndefinedOpcode();
	_ce;
}
tots PREFIX_OprSize()
{
	_cb("PREFIX_OprSize");
	_newins_;
	i386(0x66) {
		_adv;
		vcpuins.prefix_oprsize = 0x01;
	} else
		UndefinedOpcode();
	_ce;
}
tots PREFIX_AddrSize()
{
	_cb("PREFIX_AddrSize");
	_newins_;
	i386(0x67) {
		_adv;
		vcpuins.prefix_addrsize = 0x01;
	} else
		UndefinedOpcode();
	_ce;
}
tots PUSH_I16()
{
	_cb("PUSH_I16");
	_newins_;
	i386(0x68) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		_chk(_e_push(vcpuins.rimm, _GetOperandSize));
	} else
		UndefinedOpcode();
	_ce;
}
tots IMUL_R16_RM16_I16()
{
	_cb("IMUL_R16_RM16_I16");
	_newins_;
	i386(0x69) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
		_chk(_d_imm(_GetOperandSize));
		_chk(_a_imul3(vcpuins.rr, vcpuins.rrm, vcpuins.rimm, _GetOperandSize * 8));
	} else
		UndefinedOpcode();
	_ce;
}
tots PUSH_I8()
{
	_cb("PUSH_I8");
	_newins_;
	i386(0x6a) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_push(vcpuins.rimm, 1));
	} else
		UndefinedOpcode();
	_ce;
}
tots IMUL_R16_RM16_I8()
{
	_cb("IMUL_R16_RM16_I8");
	_newins_;
	i386(0x6b) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
		_chk(_d_imm(1));
		_chk(_a_imul3(vcpuins.rr, vcpuins.rrm, vcpuins.rimm, ((_GetOperandSize * 8 + 8) >> 1)));
	} else
		UndefinedOpcode();
	_ce;
}
tots INSB()
{
	_cb("INSB");
	_newins_;
	i386(0x6c) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_p_ins(1));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_p_ins(1));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx) vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_p_ins(1));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx) vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else
		UndefinedOpcode();
	_ce;
}
tots INSW()
{
	_cb("INSW");
	_newins_;
	i386(0x6d) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_p_ins(_GetOperandSize));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_p_ins(_GetOperandSize));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx) vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_p_ins(_GetOperandSize));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx) vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else
		UndefinedOpcode();
	_ce;
}
tots JO_REL8()
{
	_cb("JO_REL8");
	_newins_;
	i386(0x70) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_jcc(vcpuins.rimm, 1, _GetEFLAGS_OF));
	} else {
		vcpu.ip++;
		_d_imm(1);
		JCC(vcpuins.rimm, _GetEFLAGS_OF, 8);
	}
	_ce;
}
tots JNO_REL8()
{
	_cb("JNO_REL8");
	_newins_;
	i386(0x71) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_jcc(vcpuins.rimm, 1, !_GetEFLAGS_OF));
	} else {
		vcpu.ip++;
		_d_imm(1);
		JCC(vcpuins.rimm, !_GetEFLAGS_OF, 8);
	}
	_ce;
}
done JC_REL8()
{
	_cb("JC_REL8");
	i386(0x72) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_jcc(vcpuins.rimm, 1, _GetEFLAGS_CF));
	} else {
		vcpu.ip++;
		_d_imm(1);
		JCC(vcpuins.rimm, _GetEFLAGS_CF, 8);
	}
	_ce;
}
done JNC_REL8()
{
	_cb("JNC_REL8");
	i386(0x73) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_jcc(vcpuins.rimm, 1, !_GetEFLAGS_CF));
	} else {
		vcpu.ip++;
		_d_imm(1);
		JCC(vcpuins.rimm, !_GetEFLAGS_CF, 8);
	}
	_ce;
}
done JZ_REL8()
{
	_cb("JZ_REL8");
	i386(0x74) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_jcc(vcpuins.rimm, 1, _GetEFLAGS_ZF));
	} else {
		vcpu.ip++;
		_d_imm(1);
		JCC(vcpuins.rimm, _GetEFLAGS_ZF, 8);
	}
	_ce;
}
done JNZ_REL8()
{
	_cb("JNZ_REL8");
	i386(0x75) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_jcc(vcpuins.rimm, 1, !_GetEFLAGS_ZF));
	} else {
		vcpu.ip++;
		_d_imm(1);
		JCC(vcpuins.rimm, !_GetEFLAGS_ZF, 8);
	}
	_ce;
}
done JNA_REL8()
{
	_cb("JNA_REL8");
	i386(0x76) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_jcc(vcpuins.rimm, 1,
			(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
	} else {
		vcpu.ip++;
		_d_imm(1);
		JCC(vcpuins.rimm,
			(_GetEFLAGS_CF || _GetEFLAGS_ZF), 8);
	}
	_ce;
}
done JA_REL8()
{
	_cb("JA_REL8");
	i386(0x77) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_jcc(vcpuins.rimm, 1,
			!(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
	} else {
		vcpu.ip++;
		_d_imm(1);
		JCC(vcpuins.rimm,
			!(_GetEFLAGS_CF || _GetEFLAGS_ZF), 8);
	}
	_ce;
}
done JS_REL8()
{
	_cb("JS_REL8");
	i386(0x78) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_jcc(vcpuins.rimm, 1, _GetEFLAGS_SF));
	} else {
		vcpu.ip++;
		_d_imm(1);
		JCC(vcpuins.rimm, _GetEFLAGS_SF, 8);
	}
	_ce;
}
done JNS_REL8()
{
	_cb("JNS_REL8");
	i386(0x79) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_jcc(vcpuins.rimm, 1, !_GetEFLAGS_SF));
	} else {
		vcpu.ip++;
		_d_imm(1);
		JCC(vcpuins.rimm, !_GetEFLAGS_SF, 8);
	}
	_ce;
}
tots JP_REL8()
{
	_cb("JP_REL8");
	_newins_;
	i386(0x7a) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_jcc(vcpuins.rimm, 1, _GetEFLAGS_PF));
	} else {
		vcpu.ip++;
		_d_imm(1);
		JCC(vcpuins.rimm, _GetEFLAGS_PF, 8);
	}
	_ce;
}
tots JNP_REL8()
{
	_cb("JNP_REL8");
	_newins_;
	i386(0x7b) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_jcc(vcpuins.rimm, 1, !_GetEFLAGS_PF));
	} else {
		vcpu.ip++;
		_d_imm(1);
		JCC(vcpuins.rimm, !_GetEFLAGS_PF, 8);
	}
	_ce;
}
done JL_REL8()
{
	_cb("JL_REL8");
	i386(0x7c) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_jcc(vcpuins.rimm, 1, (_GetEFLAGS_SF != _GetEFLAGS_OF)));
	} else {
		vcpu.ip++;
		_d_imm(1);
		JCC(vcpuins.rimm, (_GetEFLAGS_SF != _GetEFLAGS_OF), 8);
	}
	_ce;
}
done JNL_REL8()
{
	_cb("JNL_REL8");
	i386(0x7d) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_jcc(vcpuins.rimm, 1, (_GetEFLAGS_SF == _GetEFLAGS_OF)));
	} else {
		vcpu.ip++;
		_d_imm(1);
		JCC(vcpuins.rimm, (_GetEFLAGS_SF == _GetEFLAGS_OF), 8);
	}
	_ce;
}
done JNG_REL8()
{
	_cb("JNG_REL8");
	i386(0x7e) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_jcc(vcpuins.rimm, 1,
			 (_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF))));
	} else {
		vcpu.ip++;
		_d_imm(1);
		JCC(vcpuins.rimm,
			(_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF)), 8);
	}
	_ce;
}
done JG_REL8()
{
	_cb("JG_REL8");
	i386(0x7f) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_jcc(vcpuins.rimm, 1,
			(!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF))));
	} else {
		vcpu.ip++;
		_d_imm(1);
		JCC(vcpuins.rimm,
			(!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF)), 8);
	}
	_ce;
}
tots INS_80()
{
	t_nubit8 modrm = 0x00;
	_cb("INS_80");
	i386 (0x80) {
		_adv;
		_chk(modrm = (t_nubit8)_s_read_cs(vcpu.eip, 1));
		switch (_GetModRM_REG(modrm)) {
		case 0: /* ADD_RM8_I8 */
			_bb("ADD_RM8_I8");
			_chk(_d_modrm(0, 1, 1));
			_chk(_d_imm(1));
			_chk(_a_add(vcpuins.rrm, vcpuins.rimm, 8));
			_be;break;
		case 1: /* OR_RM8_I8 */
			_bb("OR_RM8_I8");
			_chk(_d_modrm(0, 1, 1));
			_chk(_d_imm(1));
			_chk(_a_or(vcpuins.rrm, vcpuins.rimm, 8));
			_be;break;
		case 2: /* ADC_RM8_I8 */
			_bb("ADC_RM8_I8");
			_chk(_d_modrm(0, 1, 1));
			_chk(_d_imm(1));
			_chk(_a_adc(vcpuins.rrm, vcpuins.rimm, 8));
			_be;break;
		case 3: /* SBB_RM8_I8 */
			_bb("SBB_RM8_I8");
			_newins_;
			_chk(_d_modrm(0, 1, 1));
			_chk(_d_imm(1));
			_chk(_a_sbb(vcpuins.rrm, vcpuins.rimm, 8));
			_be;break;
		case 4: /* AND_RM8_I8 */
			_bb("AND_RM8_I8");
			_chk(_d_modrm(0, 1, 1));
			_chk(_d_imm(1));
			_chk(_a_and(vcpuins.rrm, vcpuins.rimm, 8));
			_be;break;
		case 5: /* SUB_RM8_I8 */
			_bb("SUB_RM8_I8");
			_chk(_d_modrm(0, 1, 1));
			_chk(_d_imm(1));
			_chk(_a_sub(vcpuins.rrm, vcpuins.rimm, 8));
			_be;break;
		case 6: /* XOR_RM8_I8 */
			_bb("XOR_RM8_I8");
			_chk(_d_modrm(0, 1, 1));
			_chk(_d_imm(1));
			_chk(_a_xor(vcpuins.rrm, vcpuins.rimm, 8));
			_be;break;
		case 7: /* CMP_RM8_I8 */
			_bb("CMP_RM8_I8");
			_chk(_d_modrm(0, 1, 0));
			_chk(_d_imm(1));
			_chk(_a_cmp(vcpuins.rrm, vcpuins.rimm, 8));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_modrm(0, 1, 1);
		_d_imm(1);
		switch (vcpuins.cr) {
		case 0: _a_add(vcpuins.rrm, vcpuins.rimm, 8);break;
		case 1: _a_or(vcpuins.rrm, vcpuins.rimm, 8);break;
		case 2: _a_adc(vcpuins.rrm, vcpuins.rimm, 8);break;
		case 3: _a_sbb(vcpuins.rrm, vcpuins.rimm, 8);break;
		case 4: _a_and(vcpuins.rrm, vcpuins.rimm, 8);break;
		case 5: _a_sub(vcpuins.rrm, vcpuins.rimm, 8);break;
		case 6: _a_xor(vcpuins.rrm, vcpuins.rimm, 8);break;
		case 7: _a_cmp(vcpuins.rrm, vcpuins.rimm, 8);break;
		default:_impossible_;break;}
	}
	_ce;
}
tots INS_81()
{
	t_nubit8 modrm = 0x00;
	_cb("INS_81");
	i386(0x81) {
		_adv;
		_chk(modrm = (t_nubit8)_s_read_cs(vcpu.eip, 1));
		switch (_GetModRM_REG(modrm)) {
		case 0: /* ADD_RM16_I16 */
			_bb("ADD_RM16_I16");
			_chk(_d_modrm(0, _GetOperandSize, 1));
			_chk(_d_imm(_GetOperandSize));
			_chk(_a_add(vcpuins.rrm, vcpuins.rimm, _GetOperandSize * 8));
			_be;break;
		case 1: /* OR_RM16_I16 */
			_bb("OR_RM16_I16");
			_chk(_d_modrm(0, _GetOperandSize, 1));
			_chk(_d_imm(_GetOperandSize));
			_chk(_a_or(vcpuins.rrm, vcpuins.rimm, _GetOperandSize * 8));
			_be;break;
		case 2: /* ADC_RM16_I16 */
			_bb("ADC_RM16_I16");
			_chk(_d_modrm(0, _GetOperandSize, 1));
			_chk(_d_imm(_GetOperandSize));
			_chk(_a_adc(vcpuins.rrm, vcpuins.rimm, _GetOperandSize * 8));
			_be;break;
		case 3: /* SBB_RM16_I16 */
			_bb("SBB_RM16_I16");
			_newins_;
			_chk(_d_modrm(0, _GetOperandSize, 1));
			_chk(_d_imm(_GetOperandSize));
			_chk(_a_sbb(vcpuins.rrm, vcpuins.rimm, _GetOperandSize * 8));
			_be;break;
		case 4: /* AND_RM16_I16 */
			_bb("AND_RM16_I16");
			_chk(_d_modrm(0, _GetOperandSize, 1));
			_chk(_d_imm(_GetOperandSize));
			_chk(_a_and(vcpuins.rrm, vcpuins.rimm, _GetOperandSize * 8));
			_be;break;
		case 5: /* SUB_RM16_I16 */
			_bb("SUB_RM16_I16");
			_chk(_d_modrm(0, _GetOperandSize, 1));
			_chk(_d_imm(_GetOperandSize));
			_chk(_a_sub(vcpuins.rrm, vcpuins.rimm, _GetOperandSize * 8));
			_be;break;
		case 6: /* XOR_RM16_I16 */
			_bb("XOR_RM16_I16");
			_chk(_d_modrm(0, _GetOperandSize, 1));
			_chk(_d_imm(_GetOperandSize));
			_chk(_a_xor(vcpuins.rrm, vcpuins.rimm, _GetOperandSize * 8));
			_be;break;
		case 7: /* CMP_RM16_I16 */
			_bb("CMP_RM16_I16");
			_chk(_d_modrm(0, _GetOperandSize, 0));
			_chk(_d_imm(_GetOperandSize));
			_chk(_a_cmp(vcpuins.rrm, vcpuins.rimm, _GetOperandSize * 8));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_modrm(0, 2, 1);
		_d_imm(2);
		switch (vcpuins.cr) {
		case 0: _a_add(vcpuins.rrm, vcpuins.rimm, 16);break;
		case 1: _a_or(vcpuins.rrm, vcpuins.rimm, 16);break;
		case 2: _a_adc(vcpuins.rrm, vcpuins.rimm, 16);break;
		case 3: _a_sbb(vcpuins.rrm, vcpuins.rimm, 16);break;
		case 4: _a_and(vcpuins.rrm, vcpuins.rimm, 16);break;
		case 5: _a_sub(vcpuins.rrm, vcpuins.rimm, 16);break;
		case 6: _a_xor(vcpuins.rrm, vcpuins.rimm, 16);break;
		case 7: _a_cmp(vcpuins.rrm, vcpuins.rimm, 16);break;
		default:_impossible_;break;}
	}
	_ce;
}
tots INS_83()
{
	t_nubit8 bit = (_GetOperandSize * 8 + 8) >> 1;
	t_nubit8 modrm = 0x00;
	_cb("INS_83");
	i386(0x83) {
		_adv;
		_chk(modrm = (t_nubit8)_s_read_cs(vcpu.eip, 1));
		switch (_GetModRM_REG(modrm)) {
		case 0: /* ADD_RM16_I8 */
			_bb("ADD_RM16_I8");
			_chk(_d_modrm(0, _GetOperandSize, 1));
			_chk(_d_imm(1));
			_chk(_a_add(vcpuins.rrm, vcpuins.rimm, bit));
			_be;break;
		case 1: /* OR_RM16_I8 */
			_bb("OR_RM16_I8");
			_chk(_d_modrm(0, _GetOperandSize, 1));
			_chk(_d_imm(1));
			_chk(_a_or(vcpuins.rrm, vcpuins.rimm, bit));
			_be;break;
		case 2: /* ADC_RM16_I8 */
			_bb("ADC_RM16_I8");
			_chk(_d_modrm(0, _GetOperandSize, 1));
			_chk(_d_imm(1));
			_chk(_a_adc(vcpuins.rrm, vcpuins.rimm, bit));
			_be;break;
		case 3: /* SBB_RM16_I8 */
			_bb("SBB_RM16_I8");
			_chk(_d_modrm(0, _GetOperandSize, 1));
			_chk(_d_imm(1));
			_chk(_a_sbb(vcpuins.rrm, vcpuins.rimm, bit));
			_be;break;
		case 4: /* AND_RM16_I8 */
			_bb("AND_RM16_I8");
			_chk(_d_modrm(0, _GetOperandSize, 1));
			_chk(_d_imm(1));
			_chk(_a_and(vcpuins.rrm, vcpuins.rimm, bit));
			_be;break;
		case 5: /* SUB_RM16_I8 */
			_bb("SUB_RM16_I8");
			_chk(_d_modrm(0, _GetOperandSize, 1));
			_chk(_d_imm(1));
			_chk(_a_sub(vcpuins.rrm, vcpuins.rimm, bit));
			_be;break;
		case 6: /* XOR_RM16_I8 */
			_bb("XOR_RM16_I8");
			_chk(_d_modrm(0, _GetOperandSize, 1));
			_chk(_d_imm(1));
			_chk(_a_xor(vcpuins.rrm, vcpuins.rimm, bit));
			_be;break;
		case 7: /* CMP_RM16_I8 */
			_bb("CMP_RM16_I8");
			_chk(_d_modrm(0, _GetOperandSize, 0));
			_chk(_d_imm(1));
			_chk(_a_cmp(vcpuins.rrm, vcpuins.rimm, bit));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_modrm(0, 2, 1);
		_d_imm(1);
		switch (vcpuins.cr) {
		case 0: _a_add(vcpuins.rrm, vcpuins.rimm, bit);break;
		case 1: _a_or(vcpuins.rrm, vcpuins.rimm, bit);break;
		case 2: _a_adc(vcpuins.rrm, vcpuins.rimm, bit);break;
		case 3: _a_sbb(vcpuins.rrm, vcpuins.rimm, bit);break;
		case 4: _a_and(vcpuins.rrm, vcpuins.rimm, bit);break;
		case 5: _a_sub(vcpuins.rrm, vcpuins.rimm, bit);break;
		case 6: _a_xor(vcpuins.rrm, vcpuins.rimm, bit);break;
		case 7: _a_cmp(vcpuins.rrm, vcpuins.rimm, bit);break;
		default:_impossible_;break;}
	}
	_ce;
}
done TEST_RM8_R8()
{
	_cb("TEST_RM8_R8");
	i386(0x84) {
		_adv;
		_chk(_d_modrm(1, 1, 0));
		_chk(_a_test(vcpuins.rrm, vcpuins.rr, 8));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_a_test(vcpuins.rrm, vcpuins.rr, 8);
	}
	_ce;
}
done TEST_RM16_R16()
{
	_cb("TEST_RM16_R16");
	i386(0x85) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
		_chk(_a_test(vcpuins.rrm, vcpuins.rr, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_a_test(vcpuins.rrm, vcpuins.rr, 16);
	}
	_ce;
}
done XCHG_RM8_R8()
{
	_cb("XCHG_RM8_R8");
	i386(0x86) {
		_adv;
		_chk(_d_modrm(1, 1, 1));
		_chk(_a_xchg(vcpuins.rrm, vcpuins.rr, 8));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_a_xchg(vcpuins.rrm, vcpuins.rr, 8);
	}
	_ce;
}
done XCHG_RM16_R16()
{
	_cb("XCHG_RM16_R16");
	i386(0x87) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 1));
		_chk(_a_xchg(vcpuins.rrm, vcpuins.rr, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_a_xchg(vcpuins.rrm, vcpuins.rr, 16);
	}
	_ce;
}
done MOV_RM8_R8()
{
	_cb("MOV_RM8_R8");
	i386(0x88) {
		_adv;
		_chk(_d_modrm(1, 1, 1));
		_chk(_m_mov(vcpuins.rrm, vcpuins.rr, 1));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		MOV(vcpuins.rrm, vcpuins.rr, 8);
	}
	_ce;
}
done MOV_RM16_R16()
{
	_cb("MOV_RM16_R16");
	i386(0x89) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 1));
		_chk(_m_mov(vcpuins.rrm, vcpuins.rr, _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		MOV(vcpuins.rrm, vcpuins.rr, 16);
	}
	_ce;
}
done MOV_R8_RM8()
{
	_cb("MOV_R8_RM8");
	i386(0x8a) {
		_adv;
		_chk(_d_modrm(1, 1, 0));
		_chk(_m_mov(vcpuins.rr, vcpuins.rrm, 1));
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		MOV(vcpuins.rr, vcpuins.rrm, 8);
	}
	_ce;
}
done MOV_R16_RM16()
{
	_cb("MOV_R16_RM16");
	i386(0x8b) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
		_chk(_m_mov(vcpuins.rr, vcpuins.rrm, _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		MOV(vcpuins.rr, vcpuins.rrm, 16);
	}
	_ce;
}
done MOV_RM16_SREG()
{
	_cb("MOV_RM16_SREG");
	i386(0x8c) {
		_adv;
		_chk(_d_modrm_sreg(2, 1));
		_chk(_m_mov(vcpuins.rrm, (t_vaddrcc)(&(vcpuins.rmovsreg->selector)), 2));
	} else {
		vcpu.ip++;
		_d_modrm_sreg(2, 1);
		_m_mov(vcpuins.rrm, (t_vaddrcc)(&(vcpuins.rmovsreg->selector)), 2);
	}
	_ce;
}
done LEA_R16_M16()
{
	_cb("LEA_R16_M16");
	i386(0x8d) {
		_adv;
		_chk(_d_modrm_ea(_GetOperandSize, _GetOperandSize));
		switch (_GetOperandSize) {
		case 2: d_nubit16(vcpuins.rr) = GetMax16(vcpuins.erm);break;
		case 4: d_nubit32(vcpuins.rr) = GetMax32(vcpuins.erm);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_modrm_ea(_GetOperandSize, _GetOperandSize));
		d_nubit16(vcpuins.rr) = GetMax16(vcpuins.erm);
	}
	_ce;
}
done MOV_SREG_RM16()
{
	_cb("MOV_SREG_RM16");
	i386(0x8e) {
		_adv;
		_chk(_d_modrm_sreg(2, 0));
		if (vcpuins.rmovsreg->sregtype == SREG_CODE) {
			_bb("sregtype(SREG_CODE)");
			_chk(_SetExcept_UD(0));
			_be;
		}
		_chk(_s_load_seg(vcpuins.rmovsreg, GetMax16(vcpuins.crm)));
		if (vcpuins.rmovsreg->sregtype == SREG_STACK)
			vcpuins.flagmaskint = 1;
	} else {
		vcpu.ip++;
		_d_modrm_sreg(2, 0);
		_s_load_seg(vcpuins.rmovsreg, GetMax16(vcpuins.crm));
	}
	_ce;
}
done INS_8F()
{
	_cb("INS_8F");
	i386(0x8f) {
		_adv;
		_chk(_d_modrm(0, _GetOperandSize, 1));
		switch (vcpuins.cr) {
		case 0: /* POP_RM16 */
			_bb("POP_RM16");
			_chk(_e_pop(vcpuins.rrm, _GetOperandSize));
			_be;break;
		case 1: _bb("cr(1)");_chk(UndefinedOpcode());_be;break;
		case 2: _bb("cr(2)");_chk(UndefinedOpcode());_be;break;
		case 3: _bb("cr(3)");_chk(UndefinedOpcode());_be;break;
		case 4: _bb("cr(4)");_chk(UndefinedOpcode());_be;break;
		case 5: _bb("cr(5)");_chk(UndefinedOpcode());_be;break;
		case 6: _bb("cr(6)");_chk(UndefinedOpcode());_be;break;
		case 7: _bb("cr(7)");_chk(UndefinedOpcode());_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_modrm(0, 2, 1);
		switch (vcpuins.cr) {
		case 0:
			POP(vcpuins.rrm,16);
			break;
		default:_impossible_;break;}
	}
	_ce;
}
done NOP()
{
	_cb("NOP");
	i386(0x90) {
		_adv;
	} else
		vcpu.ip++;
	_ce;
}
tots XCHG_CX_AX()
{
	_cb("XCHG_CX_AX");
	i386(0x91) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_xchg((t_vaddrcc)&vcpu.cx, (t_vaddrcc)&vcpu.ax, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_a_xchg((t_vaddrcc)&vcpu.ecx, (t_vaddrcc)&vcpu.eax, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_xchg((t_vaddrcc)&vcpu.cx, (t_vaddrcc)&vcpu.ax, 16);
	}
	_ce;
}
tots XCHG_DX_AX()
{
	_cb("XCHG_DX_AX");
	i386(0x92) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_xchg((t_vaddrcc)&vcpu.dx, (t_vaddrcc)&vcpu.ax, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_a_xchg((t_vaddrcc)&vcpu.edx, (t_vaddrcc)&vcpu.eax, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_xchg((t_vaddrcc)&vcpu.dx, (t_vaddrcc)&vcpu.ax, 16);
	}
	_ce;
}
tots XCHG_BX_AX()
{
	_cb("XCHG_BX_AX");
	i386(0x93) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_xchg((t_vaddrcc)&vcpu.bx, (t_vaddrcc)&vcpu.ax, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_a_xchg((t_vaddrcc)&vcpu.ebx, (t_vaddrcc)&vcpu.eax, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_xchg((t_vaddrcc)&vcpu.bx, (t_vaddrcc)&vcpu.ax, 16);
	}
	_ce;
}
tots XCHG_SP_AX()
{
	_cb("XCHG_SP_AX");
	_newins_;
	i386(0x94) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_xchg((t_vaddrcc)&vcpu.sp, (t_vaddrcc)&vcpu.ax, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_a_xchg((t_vaddrcc)&vcpu.esp, (t_vaddrcc)&vcpu.eax, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_xchg((t_vaddrcc)&vcpu.sp, (t_vaddrcc)&vcpu.ax, 16);
	}
	_ce;
}
tots XCHG_BP_AX()
{
	_cb("XCHG_BP_AX");
	i386(0x95) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_xchg((t_vaddrcc)&vcpu.bp, (t_vaddrcc)&vcpu.ax, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_a_xchg((t_vaddrcc)&vcpu.ebp, (t_vaddrcc)&vcpu.eax, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_xchg((t_vaddrcc)&vcpu.bp, (t_vaddrcc)&vcpu.ax, 16);
	}
	_ce;
}
tots XCHG_SI_AX()
{
	_cb("XCHG_SI_AX");
	i386(0x96) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_xchg((t_vaddrcc)&vcpu.si, (t_vaddrcc)&vcpu.ax, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_a_xchg((t_vaddrcc)&vcpu.esi, (t_vaddrcc)&vcpu.eax, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_xchg((t_vaddrcc)&vcpu.si, (t_vaddrcc)&vcpu.ax, 16);
	}
	_ce;
}
tots XCHG_DI_AX()
{
	_cb("XCHG_DI_AX");
	i386(0x97) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_xchg((t_vaddrcc)&vcpu.di, (t_vaddrcc)&vcpu.ax, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_a_xchg((t_vaddrcc)&vcpu.edi, (t_vaddrcc)&vcpu.eax, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_a_xchg((t_vaddrcc)&vcpu.di, (t_vaddrcc)&vcpu.ax, 16);
	}
	_ce;
}
done CBW()
{
	_cb("CBW");
	i386(0x98) {
		_adv;
		switch (_GetOperandSize) {
		case 2: vcpu.ax = (t_nsbit8)vcpu.al;break;
		case 4: vcpu.eax = (t_nsbit16)vcpu.ax;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		vcpu.ax = (t_nsbit8)vcpu.al;
	}
	_ce;
}
done CWD()
{
	_cb("CWD");
	i386(0x99) {
		_adv;
		switch (_GetOperandSize) {
		case 2: vcpu.dx = GetMSB16(vcpu.ax) ? 0xffff : 0x0000;break;
		case 4: vcpu.edx = GetMSB32(vcpu.eax) ? 0xffffffff : 0x00000000;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		if (vcpu.ax & 0x8000) vcpu.dx = 0xffff;
		else vcpu.dx = 0x0000;
	}
	_ce;
}
done CALL_PTR16_16()
{
	t_nubit16 newcs;
	t_nubit32 neweip;
	_cb("CALL_PTR16_16");
	i386(0x9a) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			neweip = GetMax16(vcpuins.cimm);
			_chk(_d_imm(2));
			newcs = GetMax16(vcpuins.cimm);
			_chk(_e_call_far(newcs, neweip, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_imm(4));
			neweip = GetMax32(vcpuins.cimm);
			_chk(_d_imm(2));
			newcs = GetMax16(vcpuins.cimm);
			_chk(_e_call_far(newcs, neweip, 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(2);
		neweip = d_nubit16(vcpuins.rimm);
		_d_imm(2);
		newcs = d_nubit16(vcpuins.rimm);
		PUSH((t_vaddrcc)&vcpu.cs.selector,16);
		PUSH((t_vaddrcc)&vcpu.eip,16);
		vcpu.ip = GetMax16(neweip);
		_s_load_cs(newcs);
	}
	_ce;
}
todo WAIT()
{
	/* not implemented */
	_cb("WAIT");
	_newins_;
	i386(0x9b) {
		_adv;
		if (_GetCR0_TS) {
			_bb("CR0_TS(1)");
			_chk(_SetExcept_NM(0));
			_be;
		}
	} else {
		vcpu.ip++;
	}
	_ce;
}
done PUSHF()
{
	t_nubit32 ceflags = 0x00000000;
	_cb("PUSHF");
	i386(0x9c) {
		_adv;
		if (!_GetCR0_PE || (_GetCR0_PE && (!_GetEFLAGS_VM || (_GetEFLAGS_VM && (_GetEFLAGS_IOPL == 3))))) {
			_bb("Real/Protected/(V86,IOPL(3))");
			switch (_GetOperandSize) {
			case 2: _bb("OperandSize(2)");
				ceflags = vcpu.flags;
				_chk(_e_push((t_vaddrcc)&ceflags, 2));
				_be;break;
			case 4: _bb("OperandSize(4)");
				_newins_;
				ceflags = vcpu.eflags & ~(VCPU_EFLAGS_VM | VCPU_EFLAGS_RF);
				_chk(_e_push((t_vaddrcc)&ceflags, 4));
				_be;break;
			default:_impossible_;break;}
			_be;
		} else {
			_bb("EFLAGS_VM(1),EFLAGS_IOPL(!3)");
			_chk(_SetExcept_GP(0));
			_be;
		}
	} else {
		vcpu.ip++;
		_e_push((t_vaddrcc)&vcpu.eflags, 2);
	}
	_ce;
}
done POPF()
{
	t_nubit32 mask = VCPU_EFLAGS_RESERVED;
	t_nubit32 ceflags = 0x00000000;
	_cb("POPF");
	i386(0x9d) {
		_adv;
		if (!_GetCR0_PE || !_GetEFLAGS_VM) {
			_bb("CR0_PE(0)/EFLAGS_VM(0)");
			if (!_GetCPL) {
				_bb("CPL(0)");
				switch (_GetOperandSize) {
				case 2: _bb("OperandSize(2)");
					_chk(_e_pop((t_vaddrcc)&ceflags, 2));
					mask |= 0xffff0000;
					_be;break;
				case 4: _bb("OperandSize(4)");
					_chk(_e_pop((t_vaddrcc)&ceflags, 4));
					mask |= VCPU_EFLAGS_VM;
					_be;break;
				default:_impossible_;break;}
				_be;
			} else {
				_bb("CPL(!0)");
				_newins_;
				switch (_GetOperandSize) {
				case 2: _bb("OperandSize(2)");
					_chk(_e_pop((t_vaddrcc)&ceflags, 2));
					mask |= VCPU_EFLAGS_IOPL;
					_be;break;
				case 4: _bb("OperandSize(4)");
					_chk(_e_pop((t_vaddrcc)&ceflags, 4));
					mask |= (VCPU_EFLAGS_VM | VCPU_EFLAGS_RF | VCPU_EFLAGS_IOPL);
					_be;break;
				default:_impossible_;break;}
				_be;
			}
			_be;
		} else {
			_bb("CR0_PE(1),EFLAGS_VM(1)");
			_newins_;
			if (vcpuins.prefix_oprsize) {
				_bb("prefix_oprsize(1)");
				_chk(_SetExcept_GP(0));
				_be;
			}
			if (_GetEFLAGS_IOPL == 0x03) {
				_bb("EFLAGS_IOPL(3)");
				switch (_GetOperandSize) {
				case 2: _bb("OperandSize(2)");
					_chk(_e_pop((t_vaddrcc)&ceflags, 2));
					mask |= VCPU_EFLAGS_IOPL;
					_be;break;
				case 4: _bb("OperandSize(4)");
					_chk(_e_pop((t_vaddrcc)&ceflags, 4));
					mask |= (VCPU_EFLAGS_VM | VCPU_EFLAGS_RF | VCPU_EFLAGS_IOPL);
					_be;break;
				default:_impossible_;break;}
				_be;
			} else {
				_bb("EFLAGS_IOPL(!3)");
				_chk(_SetExcept_GP(0));
				_be;
			}
			_be;
		}
		vcpu.eflags = (ceflags & ~mask) | (vcpu.eflags & mask);
	} else {
		vcpu.ip++;
		_e_pop((t_vaddrcc)&vcpu.flags, 2);
	}
	_ce;
}
done SAHF()
{
	t_nubit32 mask = (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
		VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_CF);
	_cb("SAHF");
	i386(0x9e) {
		_adv;
		vcpu.eflags = (vcpu.ah & mask) | (vcpu.eflags & ~mask);
	} else {
		vcpu.ip++;
		d_nubit8(&vcpu.eflags) = vcpu.ah;
	}
	_ce;
}
done LAHF()
{
	_cb("LAHF");
	i386(0x9f) {
		_adv;
		vcpu.ah = GetMax8(vcpu.flags);
	} else {
		vcpu.ip++;
		vcpu.ah = d_nubit8(&vcpu.eflags);
	}
	_ce;
}
done MOV_AL_MOFFS8()
{
	_cb("MOV_AL_MOFFS8");
	i386(0xa0) {
		_adv;
		_chk(_d_moffs(1, 0));
		_chk(_m_mov((t_vaddrcc)&vcpu.al, vcpuins.rrm, 1));
	} else {
		vcpu.ip++;
		_d_moffs(1, 0);
		_m_mov((t_vaddrcc)&vcpu.al, vcpuins.rrm, 1);
	}
	_ce;
}
done MOV_AX_MOFFS16()
{
	_cb("MOV_AX_MOFFS16");
	i386(0xa1) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_moffs(2, 0));
			_chk(_m_mov((t_vaddrcc)&vcpu.ax, vcpuins.rrm, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_moffs(4, 0));
			_chk(_m_mov((t_vaddrcc)&vcpu.eax, vcpuins.rrm, 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_moffs(2, 0);
		_m_mov((t_vaddrcc)&vcpu.ax, vcpuins.rrm, 2);
	}
	_ce;
}
done MOV_MOFFS8_AL()
{
	_cb("MOV_MOFFS8_AL");
	i386(0xa2) {
		_adv;
		_chk(_d_moffs(1, 1));
		_chk(_m_mov(vcpuins.rrm, (t_vaddrcc)&vcpu.al, 1));
	} else {
		vcpu.ip++;
		_d_moffs(1, 1);
		_m_mov(vcpuins.rrm, (t_vaddrcc)&vcpu.al, 1);
	}
	_ce;
}
tots MOV_MOFFS16_AX()
{
	_cb("MOV_MOFFS16_AX");
	i386(0xa3) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_moffs(2, 1));
			_chk(_m_mov(vcpuins.rrm, (t_vaddrcc)&vcpu.ax, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_d_moffs(4, 1));
			_chk(_m_mov(vcpuins.rrm, (t_vaddrcc)&vcpu.eax, 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_moffs(2, 1);
		_m_mov(vcpuins.rrm, (t_vaddrcc)&vcpu.ax, 2);
	}
	_ce;
}
done MOVSB()
{
	_cb("MOVSB");
	i386(0xa4) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_m_movs(1));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_m_movs(1));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx) vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_m_movs(1));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx) vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _m_movs(1);
		else {
			if (vcpu.cx) {
				_m_movs(1);
				vcpu.cx--;
			}
			if (vcpu.cx) vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
done MOVSW()
{
	_cb("MOVSW");
	i386(0xa5) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_m_movs(_GetOperandSize));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_m_movs(_GetOperandSize));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx) vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_m_movs(_GetOperandSize));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx) vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _m_movs(2);
		else {
			if (vcpu.cx) {
				_m_movs(2);
				vcpu.cx--;
			}
			if (vcpu.cx) vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
done CMPSB()
{
	_cb("CMPSB");
	i386(0xa6) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_a_cmps(8));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_a_cmps(8));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
					 vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_a_cmps(8));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
					 vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _a_cmps(8);
		else {
			if (vcpu.cx) {
				_a_cmps(8);
				vcpu.cx--;
			}
			if (vcpu.cx &&
				!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
				!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
				 vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
done CMPSW()
{
	_cb("CMPSW");
	i386(0xa7) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_a_cmps(_GetOperandSize * 8));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_a_cmps(_GetOperandSize * 8));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
					 vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_a_cmps(_GetOperandSize * 8));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
					 vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _a_cmps(16);
		else {
			if (vcpu.cx) {
				_a_cmps(16);
				vcpu.cx--;
			}
			if (vcpu.cx &&
				!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
				!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
				 vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
done TEST_AL_I8()
{
	_cb("TEST_AL_I8");
	i386(0xa8) {
		_adv;
		_chk(_d_imm(1));
		_chk(_a_test((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8));
	} else {
		vcpu.ip++;
		_d_imm(1);
		_a_test((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8);
	}
	_ce;
}
tots TEST_AX_I16()
{
	_cb("TEST_AX_I16");
	i386(0xa9) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_a_test((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_d_imm(4));
			_chk(_a_test((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(2);
		_a_test((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16);
	}
	_ce;
}
done STOSB()
{
	_cb("STOSB");
	i386(0xaa) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_m_stos(1));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_m_stos(1));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx) vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_m_stos(1));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx) vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _m_stos(1);
		else {
			if (vcpu.cx) {
				_m_stos(1);
				vcpu.cx--;
			}
			if (vcpu.cx) vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
done STOSW()
{
	_cb("STOSW");
	i386(0xab) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_m_stos(_GetOperandSize));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_m_stos(_GetOperandSize));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx) vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_m_stos(_GetOperandSize));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx) vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _m_stos(2);
		else {
			if (vcpu.cx) {
				_m_stos(2);
				vcpu.cx--;
			}
			if (vcpu.cx) vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
done LODSB()
{
	_cb("LODSB");
	i386(0xac) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_m_lods(1));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_m_lods(1));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx) vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_m_lods(1));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx) vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _m_lods(1);
		else {
			if (vcpu.cx) {
				_m_lods(1);
				vcpu.cx--;
			}
			if (vcpu.cx) vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
done LODSW()
{
	_cb("LODSW");
	i386(0xad) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_m_lods(_GetOperandSize));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_m_lods(_GetOperandSize));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx) vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_m_lods(_GetOperandSize));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx) vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _m_lods(2);
		else {
			if (vcpu.cx) {
				_m_lods(2);
				vcpu.cx--;
			}
			if (vcpu.cx) vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
done SCASB()
{
	_cb("SCASB");
	i386(0xae) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_a_scas(8));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_a_scas(8));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
					 vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_a_scas(8));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
					 vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _a_scas(8);
		else {
			if (vcpu.cx) {
				_a_scas(8);
				vcpu.cx--;
			}
			if (vcpu.cx &&
				!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
				!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
				 vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
done SCASW()
{
	_cb("SCASW");
	i386(0xaf) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_a_scas(_GetOperandSize * 8));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_a_scas(_GetOperandSize * 8));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
					 vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_a_scas(_GetOperandSize * 8));
					vcpu.ecx--;
					_be;
				}
				if (vcpu.ecx &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
					!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
					 vcpuins.flaginsloop = 1;
				_be;break;
			default:_impossible_;break;}
			_be;
		}
	} else {
		vcpu.ip++;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) _a_scas(16);
		else {
			if (vcpu.cx) {
				_a_scas(16);
				vcpu.cx--;
			}
			if (vcpu.cx &&
				!(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
				!(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
				 vcpuins.flaginsloop = 0x01;
		}
	}
	_ce;
}
done MOV_AL_I8()
{
	_cb("MOV_AL_I8");
	i386(0xb0) {
		_adv;
		_chk(_d_imm(1));
		_chk(_m_mov((t_vaddrcc)&vcpu.al, vcpuins.rimm, 1));
	} else {
		vcpu.ip++;
		_d_imm(1);
		MOV((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8);
	}
	_ce;
}
done MOV_CL_I8()
{
	_cb("MOV_CL_I8");
	i386(0xb1) {
		_adv;
		_chk(_d_imm(1));
		_chk(_m_mov((t_vaddrcc)&vcpu.cl, vcpuins.rimm, 1));
	} else {
		vcpu.ip++;
		_d_imm(1);
		MOV((t_vaddrcc)&vcpu.cl, vcpuins.rimm, 8);
	}
	_ce;
}
done MOV_DL_I8()
{
	_cb("MOV_DL_I8");
	i386(0xb2) {
		_adv;
		_chk(_d_imm(1));
		_chk(_m_mov((t_vaddrcc)&vcpu.dl, vcpuins.rimm, 1));
	} else {
		vcpu.ip++;
		_d_imm(1);
		MOV((t_vaddrcc)&vcpu.dl, vcpuins.rimm, 8);
	}
	_ce;
}
done MOV_BL_I8()
{
	_cb("MOV_BL_I8");
	i386(0xb3) {
		_adv;
		_chk(_d_imm(1));
		_chk(_m_mov((t_vaddrcc)&vcpu.bl, vcpuins.rimm, 1));
	} else {
		vcpu.ip++;
		_d_imm(1);
		MOV((t_vaddrcc)&vcpu.bl, vcpuins.rimm, 8);
	}
	_ce;
}
done MOV_AH_I8()
{
	_cb("MOV_AH_I8");
	i386(0xb4) {
		_adv;
		_chk(_d_imm(1));
		_chk(_m_mov((t_vaddrcc)&vcpu.ah, vcpuins.rimm, 1));
	} else {
		vcpu.ip++;
		_d_imm(1);
		MOV((t_vaddrcc)&vcpu.ah, vcpuins.rimm, 8);
	}
	_ce;
}
done MOV_CH_I8()
{
	_cb("MOV_CH_I8");
	i386(0xb5) {
		_adv;
		_chk(_d_imm(1));
		_chk(_m_mov((t_vaddrcc)&vcpu.ch, vcpuins.rimm, 1));
	} else {
		vcpu.ip++;
		_d_imm(1);
		MOV((t_vaddrcc)&vcpu.ch, vcpuins.rimm, 8);
	}
	_ce;
}
done MOV_DH_I8()
{
	_cb("MOV_DH_I8");
	i386(0xb6) {
		_adv;
		_chk(_d_imm(1));
		_chk(_m_mov((t_vaddrcc)&vcpu.dh, vcpuins.rimm, 1));
	} else {
		vcpu.ip++;
		_d_imm(1);
		MOV((t_vaddrcc)&vcpu.dh, vcpuins.rimm, 8);
	}
	_ce;
}
done MOV_BH_I8()
{
	_cb("MOV_BH_I8");
	i386(0xb7) {
		_adv;
		_chk(_d_imm(1));
		_chk(_m_mov((t_vaddrcc)&vcpu.bh, vcpuins.rimm, 1));
	} else {
		vcpu.ip++;
		_d_imm(1);
		MOV((t_vaddrcc)&vcpu.bh, vcpuins.rimm, 8);
	}
	_ce;
}
done MOV_AX_I16()
{
	_cb("MOV_AX_I16");
	i386(0xb8) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_m_mov((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_imm(4));
			_chk(_m_mov((t_vaddrcc)&vcpu.eax, vcpuins.rimm, 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(2);
		MOV((t_vaddrcc)&vcpu.ax, vcpuins.rimm,16);
	}
	_ce;
}
done MOV_CX_I16()
{
	_cb("MOV_CX_I16");
	i386(0xb9) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_m_mov((t_vaddrcc)&vcpu.cx, vcpuins.rimm, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_imm(4));
			_chk(_m_mov((t_vaddrcc)&vcpu.ecx, vcpuins.rimm, 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(2);
		MOV((t_vaddrcc)&vcpu.cx, vcpuins.rimm,16);
	}
	_ce;
}
done MOV_DX_I16()
{
	_cb("MOV_DX_I16");
	i386(0xba) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_m_mov((t_vaddrcc)&vcpu.dx, vcpuins.rimm, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_imm(4));
			_chk(_m_mov((t_vaddrcc)&vcpu.edx, vcpuins.rimm, 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(2);
		MOV((t_vaddrcc)&vcpu.dx, vcpuins.rimm,16);
	}
	_ce;
}
done MOV_BX_I16()
{
	_cb("MOV_BX_I16");
	i386(0xbb) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_m_mov((t_vaddrcc)&vcpu.bx, vcpuins.rimm, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_imm(4));
			_chk(_m_mov((t_vaddrcc)&vcpu.ebx, vcpuins.rimm, 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(2);
		MOV((t_vaddrcc)&vcpu.bx, vcpuins.rimm,16);
	}
	_ce;
}
done MOV_SP_I16()
{
	_cb("MOV_SP_I16");
	i386(0xbc) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_m_mov((t_vaddrcc)&vcpu.sp, vcpuins.rimm, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_imm(4));
			_chk(_m_mov((t_vaddrcc)&vcpu.esp, vcpuins.rimm, 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(2);
		MOV((t_vaddrcc)&vcpu.sp, vcpuins.rimm,16);
	}
	_ce;
}
done MOV_BP_I16()
{
	_cb("MOV_BP_I16");
	i386(0xbd) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_m_mov((t_vaddrcc)&vcpu.bp, vcpuins.rimm, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_imm(4));
			_chk(_m_mov((t_vaddrcc)&vcpu.ebp, vcpuins.rimm, 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(2);
		MOV((t_vaddrcc)&vcpu.bp, vcpuins.rimm,16);
	}
	_ce;
}
done MOV_SI_I16()
{
	_cb("MOV_SI_I16");
	i386(0xbe) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_m_mov((t_vaddrcc)&vcpu.si, vcpuins.rimm, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_imm(4));
			_chk(_m_mov((t_vaddrcc)&vcpu.esi, vcpuins.rimm, 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(2);
		MOV((t_vaddrcc)&vcpu.si, vcpuins.rimm,16);
	}
	_ce;
}
done MOV_DI_I16()
{
	_cb("MOV_DI_I16");
	i386(0xbf) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_m_mov((t_vaddrcc)&vcpu.di, vcpuins.rimm, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_imm(4));
			_chk(_m_mov((t_vaddrcc)&vcpu.edi, vcpuins.rimm, 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(2);
		MOV((t_vaddrcc)&vcpu.di, vcpuins.rimm,16);
	}
	_ce;
}
tots INS_C0()
{
	_cb("INS_C0");
	_newins_;
	i386(0xc0) {
		_adv;
		_chk(_d_modrm(0, 1, 1));
		_chk(_d_imm(1));
		switch (vcpuins.cr) {
		case 0: /* ROL_RM8_I8 */
			_bb("ROL_RM8_I8");
			_chk(_a_rol(vcpuins.rrm, vcpuins.rimm, 8));
			_be;break;
		case 1: /* ROR_RM8_I8 */
			_bb("ROR_RM8_I8");
			_chk(_a_ror(vcpuins.rrm, vcpuins.rimm, 8));
			_be;break;
		case 2: /* RCL_RM8_I8 */
			_bb("RCL_RM8_I8");
			_chk(_a_rcl(vcpuins.rrm, vcpuins.rimm, 8));
			_be;break;
		case 3: /* RCR_RM8_I8 */
			_bb("RCR_RM8_I8");
			_chk(_a_rcr(vcpuins.rrm, vcpuins.rimm, 8));
			_be;break;
		case 4: /* SHL_RM8_I8 */
			_bb("SHL_RM8_I8");
			_chk(_a_shl(vcpuins.rrm, vcpuins.rimm, 8));
			_be;break;
		case 5: /* SHR_RM8_I8 */
			_bb("SHR_RM8_I8");
			_chk(_a_shr(vcpuins.rrm, vcpuins.rimm, 8));
			_be;break;
		case 6: /* UndefinedOpcode */
			_bb("vcpuins.cr(6)");
			_chk(UndefinedOpcode());
			_be;break;
		case 7: /* SAR_RM8_I8 */
			_bb("SAR_RM8_I8");
			_chk(_a_sar(vcpuins.rrm, vcpuins.rimm, 8));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_modrm(0, 1, 1);
		_d_imm(1);
		switch (vcpuins.cr) {
		case 0:	_a_rol(vcpuins.rrm, vcpuins.rimm, 8);break;
		case 1:	_a_ror(vcpuins.rrm, vcpuins.rimm, 8);break;
		case 2:	_a_rcl(vcpuins.rrm, vcpuins.rimm, 8);break;
		case 3:	_a_rcr(vcpuins.rrm, vcpuins.rimm, 8);break;
		case 4:	_a_shl(vcpuins.rrm, vcpuins.rimm, 8);break;
		case 5:	_a_shr(vcpuins.rrm, vcpuins.rimm, 8);break;
		case 6:	UndefinedOpcode();break;
		case 7:	_a_sar(vcpuins.rrm, vcpuins.rimm, 8);break;
		default:_impossible_;break;}
	}
	_ce;
}
tots INS_C1()
{
	_cb("INS_C1");
	_newins_;
	i386(0xc1) {
		_adv;
		_chk(_d_modrm(0, _GetOperandSize, 1));
		_chk(_d_imm(1));
		switch (vcpuins.cr) {
		case 0: /* ROL_RM16_I8 */
			_bb("ROL_RM16_I8");
			_chk(_a_rol(vcpuins.rrm, vcpuins.rimm, _GetOperandSize * 8));
			_be;break;
		case 1: /* ROR_RM16_I8 */
			_bb("ROR_RM16_I8");
			_chk(_a_ror(vcpuins.rrm, vcpuins.rimm, _GetOperandSize * 8));
			_be;break;
		case 2: /* RCL_RM16_I8 */
			_bb("RCL_RM16_I8");
			_chk(_a_rcl(vcpuins.rrm, vcpuins.rimm, _GetOperandSize * 8));
			_be;break;
		case 3: /* RCR_RM16_I8 */
			_bb("RCR_RM16_I8");
			_chk(_a_rcr(vcpuins.rrm, vcpuins.rimm, _GetOperandSize * 8));
			_be;break;
		case 4: /* SHL_RM16_I8 */
			_bb("SHL_RM16_I8");
			_chk(_a_shl(vcpuins.rrm, vcpuins.rimm, _GetOperandSize * 8));
			_be;break;
		case 5: /* SHR_RM16_I8 */
			_bb("SHR_RM16_I8");
			_chk(_a_shr(vcpuins.rrm, vcpuins.rimm, _GetOperandSize * 8));
			_be;break;
		case 6: /* UndefinedOpcode */
			_bb("vcpuins.cr(6)");
			_chk(UndefinedOpcode());
			_be;break;
		case 7: /* SAR_RM16_I8 */
			_bb("SAR_RM16_I8");
			_chk(_a_sar(vcpuins.rrm, vcpuins.rimm, _GetOperandSize * 8));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_modrm(0, 2, 1);
		_d_imm(1);
		switch (vcpuins.cr) {
		case 0:	_a_rol(vcpuins.rrm, vcpuins.rimm,16);break;
		case 1:	_a_ror(vcpuins.rrm, vcpuins.rimm,16);break;
		case 2:	_a_rcl(vcpuins.rrm, vcpuins.rimm,16);break;
		case 3:	_a_rcr(vcpuins.rrm, vcpuins.rimm,16);break;
		case 4:	_a_shl(vcpuins.rrm, vcpuins.rimm,16);break;
		case 5:	_a_shr(vcpuins.rrm, vcpuins.rimm,16);break;
		case 6:	UndefinedOpcode();break;
		case 7:	_a_sar(vcpuins.rrm, vcpuins.rimm,16);break;
		default:_impossible_;break;}
	}
	_ce;
}
done RET_I16()
{
	t_nubit16 addsp;
	_cb("RET_I16");
	i386(0xc2) {
		_adv;
		_chk(_d_imm(2));
		_chk(_e_ret_near(GetMax16(vcpuins.cimm), _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_imm(2);
		addsp = d_nubit16(vcpuins.rimm);
		POP((t_vaddrcc)&vcpu.eip,16);
		vcpu.sp += addsp;
	}
	_ce;
}
done RET()
{
	_cb("RET");
	i386(0xc3) {
		_adv;
		_chk(_e_ret_near(0, _GetOperandSize));
	} else {
		vcpu.ip++;
		_e_pop((t_vaddrcc)&vcpu.ip, 2);
	}
	_ce;
}
done LES_R16_M16_16()
{
	t_nubit16 selector = 0x0000;
	t_nubit32 offset = 0x00000000;
	_cb("LES_R16_M16_16");
	i386(0xc4) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2, 0));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		switch (_GetOperandSize) {
		case 2:
			offset = d_nubit16(vcpuins.rrm);
			selector = d_nubit16(vcpuins.rrm + 2);
			break;
		case 4:
			_newins_;
			offset = d_nubit32(vcpuins.rrm);
			selector = d_nubit16(vcpuins.rrm + 4);
			break;
		default:_impossible_;break;}
		_chk(_e_load_far(&vcpu.es, vcpuins.rr, selector, offset, _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		d_nubit16(vcpuins.rr) = d_nubit16(vcpuins.rrm);
		_s_load_es(d_nubit16(vcpuins.rrm + 2));
	}
	_ce;
}
done LDS_R16_M16_16()
{
	t_nubit16 selector = 0x0000;
	t_nubit32 offset = 0x00000000;
	_cb("LDS_R16_M16_16");
	i386(0xc5) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2, 0));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		switch (_GetOperandSize) {
		case 2:
			offset = d_nubit16(vcpuins.rrm);
			selector = d_nubit16(vcpuins.rrm + 2);
			break;
		case 4:
			_newins_;
			offset = d_nubit32(vcpuins.rrm);
			selector = d_nubit16(vcpuins.rrm + 4);
			break;
		default:_impossible_;break;}
		_chk(_e_load_far(&vcpu.ds, vcpuins.rr, selector, offset, _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		d_nubit16(vcpuins.rr) = d_nubit16(vcpuins.rrm);
		_s_load_ds(d_nubit16(vcpuins.rrm + 2));
	}
	_ce;
}
done INS_C6()
{
	_cb("INS_C6");
	i386(0xc6) {
		_adv;
		_chk(_d_modrm(0, 1, 1));
		_chk(_d_imm(1));
		switch (vcpuins.cr) {
		case 0: /* MOV_RM8_I8 */
			_bb("MOV_RM8_I8");
			_chk(_m_mov(vcpuins.rrm, vcpuins.rimm, 1));
			_be;break;
		case 1: _bb("cr(1)");_chk(UndefinedOpcode());_be;break;
		case 2: _bb("cr(2)");_chk(UndefinedOpcode());_be;break;
		case 3: _bb("cr(3)");_chk(UndefinedOpcode());_be;break;
		case 4: _bb("cr(4)");_chk(UndefinedOpcode());_be;break;
		case 5: _bb("cr(5)");_chk(UndefinedOpcode());_be;break;
		case 6: _bb("cr(6)");_chk(UndefinedOpcode());_be;break;
		case 7: _bb("cr(7)");_chk(UndefinedOpcode());_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_modrm(1, 1, 1);
		_d_imm(1);
		MOV(vcpuins.rrm, vcpuins.rimm, 8);
	}
	_ce;
}
done INS_C7()
{
	_cb("INS_C7");
	i386(0xc7) {
		_adv;
		_chk(_d_modrm(0, _GetOperandSize, 1));
		_chk(_d_imm(_GetOperandSize));
		switch (vcpuins.cr) {
		case 0: /* MOV_RM16_I16 */
			_bb("MOV_RM16_I16");
			_chk(_m_mov(vcpuins.rrm, vcpuins.rimm, _GetOperandSize));
			_be;break;
		case 1: _bb("cr(1)");_chk(UndefinedOpcode());_be;break;
		case 2: _bb("cr(2)");_chk(UndefinedOpcode());_be;break;
		case 3: _bb("cr(3)");_chk(UndefinedOpcode());_be;break;
		case 4: _bb("cr(4)");_chk(UndefinedOpcode());_be;break;
		case 5: _bb("cr(5)");_chk(UndefinedOpcode());_be;break;
		case 6: _bb("cr(6)");_chk(UndefinedOpcode());_be;break;
		case 7: _bb("cr(7)");_chk(UndefinedOpcode());_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_modrm(2, 2, 1);
		_d_imm(2);
		MOV(vcpuins.rrm, vcpuins.rimm,16);
	}
	_ce;
}
tots ENTER()
{
	t_nubit32 data = 0x00000000;
	t_nubit32 temp = 0x00000000;
	t_nubit32 i = 0x00000000;
	t_nubit16 size = 0x0000;
	t_nubit8 level = 0x00;
	_cb("ENTER");
	_newins_;
	i386(0xc8) {
		_adv;
		_chk(_d_imm(2));
		size = (t_nubit16)vcpuins.cimm;
		_chk(_d_imm(1));
		level = (t_nubit8)vcpuins.cimm;
		level %= 32;
		switch (_GetStackSize) {
		case 2: _bb("StackSize(2)");
			_chk(_e_push((t_vaddrcc)&vcpu.bp, 2));
			_comment("ENTER: fetch sp after pushing bp");
			temp = vcpu.sp;
			_be;break;
		case 4: _bb("StackSize(4)");
			_chk(_e_push((t_vaddrcc)&vcpu.ebp, 4));
			_comment("ENTER: fetch esp after pushing ebp");
			temp = vcpu.esp;
			_be;break;
		default:_impossible_;break;}
		if (level) {
			_bb("level(!0)");
			_comment("ENTER: this instruction is too inefficient and error-prone.");
			for (i = 0;i < level;++i) {
				_bb("for");
				switch (_GetOperandSize) {
				case 2: _bb("OperandSize(2)");
					switch (_GetStackSize) {
					case 2: _bb("StackSize(2)");
						vcpu.bp -= 2;
						_chk(data = (t_nubit16)_s_read_ss(vcpu.bp, 2));
						_chk(_e_push((t_vaddrcc)&data, 2));
						_be;break;
					case 4: _bb("StackSize(4)");
						vcpu.ebp -= 2;
						_chk(data = (t_nubit16)_s_read_ss(vcpu.ebp, 2));
						_chk(_e_push((t_vaddrcc)&data, 2));
						_be;break;
					default:_impossible_;break;}
					_be;break;
				case 4: _bb("OperandSize(4)");
					switch (_GetStackSize) {
					case 2: _bb("StackSize(2)");
						vcpu.bp -= 4;
						_chk(data = (t_nubit32)_s_read_ss(vcpu.bp, 4));
						_chk(_e_push((t_vaddrcc)&data, 4));
						_be;break;
					case 4: _bb("StackSize(4)");
						vcpu.ebp -= 4;
						_chk(data = (t_nubit32)_s_read_ss(vcpu.ebp, 4));
						_chk(_e_push((t_vaddrcc)&data, 4));
						_be;break;
					default:_impossible_;break;}
					_be;break;
				default:_impossible_;break;}
				_be;
			}
			switch (_GetOperandSize) {
			case 2: _bb("OperandSize(2)");
				_chk(_e_push((t_vaddrcc)&temp, 2));
				_be;break;
			case 4: _bb("OperandSize(4)");
				_chk(_e_push((t_vaddrcc)&temp, 4));
				_be;break;
			default:_impossible_;break;}
			_be;
		}
		switch (_GetStackSize) {
		case 2: _bb("StackSize(2)");
			vcpu.bp = GetMax16(temp);
			vcpu.sp = vcpu.bp - size;
			_be;break;
		case 4: _bb("StackSize(4)");
			vcpu.ebp = GetMax32(temp);
			vcpu.esp = vcpu.ebp - size;
			_be;break;
		default:_impossible_;break;}
	} else
		UndefinedOpcode();
	_ce;
}
tots LEAVE()
{
	_cb("LEAVE");
	_newins_;
	i386(0xc9) {
		if (!_IsProtected && vcpu.ebp > 0x0000ffff) {
			_bb("Protected(0),ebp(>0000ffff)");
			_chk(_SetExcept_GP(0));
			_be;
		}
		_chk(_m_test_logical(&vcpu.ss, vcpu.ebp, _GetOperandSize, 1));
		switch (_GetStackSize) {
		case 2: _bb("StackSize(2)");
			vcpu.sp = vcpu.bp;
			_be;break;
		case 4: _bb("StackSize(4)");
			vcpu.esp = vcpu.ebp;
			_be;break;
		default:_impossible_;break;}
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_e_pop((t_vaddrcc)&vcpu.bp, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_e_pop((t_vaddrcc)&vcpu.ebp, 4));
			_be;break;
		default:_impossible_;break;}
	} else
		UndefinedOpcode();
	_ce;
}
done RETF_I16()
{
	t_nubit16 addsp;
	_cb("RETF_I16");
	i386(0xca) {
		_adv;
		_chk(_d_imm(2));
		_chk(_e_ret_far(GetMax16(vcpuins.cimm), _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_imm(2);
		addsp = d_nubit16(vcpuins.rimm);
		POP((t_vaddrcc)&vcpu.eip,16);
		POP((t_vaddrcc)&vcpu.cs.selector,16);
		_s_load_cs(vcpu.cs.selector);
		vcpu.sp += addsp;
	}
	_ce;
}
done RETF()
{
	_cb("RETF");
	i386(0xcb) {
		_adv;
		_chk(_e_ret_far(0, _GetOperandSize));
	} else {
		vcpu.ip++;
		_e_pop((t_vaddrcc)&vcpu.eip, 2);
		_e_pop((t_vaddrcc)&vcpu.cs.selector, 2);
		_s_load_cs(vcpu.cs.selector);
	}
	_ce;
}
tots INT3()
{
	_cb("INT3");
	_newins_;
	i386(0xcc) {
		_adv;
		_chk(_e_int3(_GetOperandSize));
	} else {
		vcpu.ip++;
		_e_int_n(0x03, _GetOperandSize);
	}
	_ce;
}
todo INT_I8()
{
	_cb("INT_I8");
	i386(0xcc) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_int_n((t_nubit8)vcpuins.cimm, _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_imm(1);
		_e_int_n(d_nubit8(vcpuins.rimm), _GetOperandSize);
	}
	_ce;
}
tots INTO()
{
	_cb("INTO");
	_newins_;
	i386(0xce) {
		_adv;
		_chk(_e_into(_GetOperandSize));
	} else {
		vcpu.ip++;
		if(_GetEFLAGS_OF) _e_int_n(0x04, _GetOperandSize);
	}
	_ce;
}
todo IRET()
{
	_cb("IRET");
	i386(0xcf) {
		_adv;
		_chk(_e_iret(_GetOperandSize));
	} else {
		vcpu.ip++;
		_e_pop((t_vaddrcc)&vcpu.eip, 2);
		_e_pop((t_vaddrcc)&vcpu.cs.selector, 2);
		_s_load_cs(vcpu.cs.selector);
		_e_pop((t_vaddrcc)&vcpu.eflags, 2);
	}
	_ce;
}
done INS_D0()
{
	_cb("INS_D0");
	i386(0xd0) {
		_adv;
		_chk(_d_modrm(0, 1, 1));
		switch (vcpuins.cr) {
		case 0: /* ROL_RM8 */
			_bb("ROL_RM8");
			_chk(_a_rol(vcpuins.rrm, (t_vaddrcc)NULL, 8));
			_be;break;
		case 1: /* ROR_RM8 */
			_bb("ROR_RM8");
			_chk(_a_ror(vcpuins.rrm, (t_vaddrcc)NULL, 8));
			_be;break;
		case 2: /* RCL_RM8 */
			_bb("RCL_RM8");
			_chk(_a_rcl(vcpuins.rrm, (t_vaddrcc)NULL, 8));
			_be;break;
		case 3: /* RCR_RM8 */
			_bb("RCR_RM8");
			_chk(_a_rcr(vcpuins.rrm, (t_vaddrcc)NULL, 8));
			_be;break;
		case 4: /* SHL_RM8 */
			_bb("SHL_RM8");
			_chk(_a_shl(vcpuins.rrm, (t_vaddrcc)NULL, 8));
			_be;break;
		case 5: /* SHR_RM8 */
			_bb("SHR_RM8");
			_chk(_a_shr(vcpuins.rrm, (t_vaddrcc)NULL, 8));
			_be;break;
		case 6: /* UndefinedOpcode */
			_bb("vcpuins.cr(6)");
			_chk(UndefinedOpcode());
			_be;break;
		case 7: /* SAR_RM8 */
			_bb("SAR_RM8");
			_chk(_a_sar(vcpuins.rrm, (t_vaddrcc)NULL, 8));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_modrm(0, 1, 1);
		switch (vcpuins.cr) {
		case 0:	_a_rol(vcpuins.rrm, (t_vaddrcc)NULL, 8);break;
		case 1:	_a_ror(vcpuins.rrm, (t_vaddrcc)NULL, 8);break;
		case 2:	_a_rcl(vcpuins.rrm, (t_vaddrcc)NULL, 8);break;
		case 3:	_a_rcr(vcpuins.rrm, (t_vaddrcc)NULL, 8);break;
		case 4:	_a_shl(vcpuins.rrm, (t_vaddrcc)NULL, 8);break;
		case 5:	_a_shr(vcpuins.rrm, (t_vaddrcc)NULL, 8);break;
		case 6:	UndefinedOpcode();break;
		case 7:	_a_sar(vcpuins.rrm, (t_vaddrcc)NULL, 8);break;
		default:_impossible_;break;}
	}
	_ce;
}
done INS_D1()
{
	_cb("INS_D1");
	i386(0xd1) {
		_adv;
		_chk(_d_modrm(0, _GetOperandSize, 1));
		switch (vcpuins.cr) {
		case 0: /* ROL_RM16 */
			_bb("ROL_RM16");
			_chk(_a_rol(vcpuins.rrm, (t_vaddrcc)NULL, _GetOperandSize * 8));
			_be;break;
		case 1: /* ROR_RM16 */
			_bb("ROR_RM16");
			_chk(_a_ror(vcpuins.rrm, (t_vaddrcc)NULL, _GetOperandSize * 8));
			_be;break;
		case 2: /* RCL_RM16 */
			_bb("RCL_RM16");
			_chk(_a_rcl(vcpuins.rrm, (t_vaddrcc)NULL, _GetOperandSize * 8));
			_be;break;
		case 3: /* RCR_RM16 */
			_bb("RCR_RM16");
			_chk(_a_rcr(vcpuins.rrm, (t_vaddrcc)NULL, _GetOperandSize * 8));
			_be;break;
		case 4: /* SHL_RM16 */
			_bb("SHL_RM16");
			_chk(_a_shl(vcpuins.rrm, (t_vaddrcc)NULL, _GetOperandSize * 8));
			_be;break;
		case 5: /* SHR_RM16 */
			_bb("SHR_RM16");
			_chk(_a_shr(vcpuins.rrm, (t_vaddrcc)NULL, _GetOperandSize * 8));
			_be;break;
		case 6: /* UndefinedOpcode */
			_bb("vcpuins.cr(6)");
			_chk(UndefinedOpcode());
			_be;break;
		case 7: /* SAR_RM16 */
			_bb("SAR_RM16");
			_chk(_a_sar(vcpuins.rrm, (t_vaddrcc)NULL, _GetOperandSize * 8));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_modrm(0, 2, 1);
		switch (vcpuins.cr) {
		case 0:	_a_rol(vcpuins.rrm,(t_vaddrcc)NULL,16);break;
		case 1:	_a_ror(vcpuins.rrm,(t_vaddrcc)NULL,16);break;
		case 2:	_a_rcl(vcpuins.rrm,(t_vaddrcc)NULL,16);break;
		case 3:	_a_rcr(vcpuins.rrm,(t_vaddrcc)NULL,16);break;
		case 4:	_a_shl(vcpuins.rrm,(t_vaddrcc)NULL,16);break;
		case 5:	_a_shr(vcpuins.rrm,(t_vaddrcc)NULL,16);break;
		case 6:	UndefinedOpcode();break;
		case 7:	_a_sar(vcpuins.rrm,(t_vaddrcc)NULL,16);break;
		default:_impossible_;break;}
	}
	_ce;
}
done INS_D2()
{
	_cb("INS_D2");
	i386(0xd2) {
		_adv;
		_chk(_d_modrm(0, 1, 1));
		switch (vcpuins.cr) {
		case 0: /* ROL_RM8_CL */
			_bb("ROL_RM8_CL");
			_chk(_a_rol(vcpuins.rrm, (t_vaddrcc)&vcpu.cl, 8));
			_be;break;
		case 1: /* ROR_RM8_CL */
			_bb("ROR_RM8_CL");
			_chk(_a_ror(vcpuins.rrm, (t_vaddrcc)&vcpu.cl, 8));
			_be;break;
		case 2: /* RCL_RM8_CL */
			_bb("RCL_RM8_CL");
			_chk(_a_rcl(vcpuins.rrm, (t_vaddrcc)&vcpu.cl, 8));
			_be;break;
		case 3: /* RCR_RM8_CL */
			_bb("RCR_RM8_CL");
			_chk(_a_rcr(vcpuins.rrm, (t_vaddrcc)&vcpu.cl, 8));
			_be;break;
		case 4: /* SHL_RM8_CL */
			_bb("SHL_RM8_CL");
			_chk(_a_shl(vcpuins.rrm, (t_vaddrcc)&vcpu.cl, 8));
			_be;break;
		case 5: /* SHR_RM8_CL */
			_bb("SHR_RM8_CL");
			_chk(_a_shr(vcpuins.rrm, (t_vaddrcc)&vcpu.cl, 8));
			_be;break;
		case 6: /* UndefinedOpcode */
			_bb("vcpuins.cr(6)");
			_chk(UndefinedOpcode());
			_be;break;
		case 7: /* SAR_RM8_CL */
			_bb("SAR_RM8_CL");
			_chk(_a_sar(vcpuins.rrm, (t_vaddrcc)&vcpu.cl, 8));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_modrm(0, 1, 1);
		switch (vcpuins.cr) {
		case 0:	_a_rol(vcpuins.rrm,(t_vaddrcc)&vcpu.cl, 8);break;
		case 1:	_a_ror(vcpuins.rrm,(t_vaddrcc)&vcpu.cl, 8);break;
		case 2:	_a_rcl(vcpuins.rrm,(t_vaddrcc)&vcpu.cl, 8);break;
		case 3:	_a_rcr(vcpuins.rrm,(t_vaddrcc)&vcpu.cl, 8);break;
		case 4:	_a_shl(vcpuins.rrm,(t_vaddrcc)&vcpu.cl, 8);break;
		case 5:	_a_shr(vcpuins.rrm,(t_vaddrcc)&vcpu.cl, 8);break;
		case 6:	UndefinedOpcode();break;
		case 7:	_a_sar(vcpuins.rrm,(t_vaddrcc)&vcpu.cl, 8);break;
		default:_impossible_;break;}
	}
	_ce;
}
done INS_D3()
{
	_cb("INS_D3");
	i386(0xd3) {
		_adv;
		_chk(_d_modrm(0, _GetOperandSize, 1));
		switch (vcpuins.cr) {
		case 0: /* ROL_RM16_CL */
			_bb("ROL_RM16_CL");
			_chk(_a_rol(vcpuins.rrm, (t_vaddrcc)&vcpu.cl, _GetOperandSize * 8));
			_be;break;
		case 1: /* ROR_RM16_CL */
			_bb("ROR_RM16_CL");
			_chk(_a_ror(vcpuins.rrm, (t_vaddrcc)&vcpu.cl, _GetOperandSize * 8));
			_be;break;
		case 2: /* RCL_RM16_CL */
			_bb("RCL_RM16_CL");
			_chk(_a_rcl(vcpuins.rrm, (t_vaddrcc)&vcpu.cl, _GetOperandSize * 8));
			_be;break;
		case 3: /* RCR_RM16_CL */
			_bb("RCR_RM16_CL");
			_chk(_a_rcr(vcpuins.rrm, (t_vaddrcc)&vcpu.cl, _GetOperandSize * 8));
			_be;break;
		case 4: /* SHL_RM16_CL */
			_bb("SHL_RM16_CL");
			_chk(_a_shl(vcpuins.rrm, (t_vaddrcc)&vcpu.cl, _GetOperandSize * 8));
			_be;break;
		case 5: /* SHR_RM16_CL */
			_bb("SHR_RM16_CL");
			_chk(_a_shr(vcpuins.rrm, (t_vaddrcc)&vcpu.cl, _GetOperandSize * 8));
			_be;break;
		case 6: /* UndefinedOpcode */
			_bb("vcpuins.cr(6)");
			_chk(UndefinedOpcode());
			_be;break;
		case 7: /* SAR_RM16_CL */
			_bb("SAR_RM16_CL");
			_chk(_a_sar(vcpuins.rrm, (t_vaddrcc)&vcpu.cl, _GetOperandSize * 8));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_modrm(0, 2, 1);
		switch (vcpuins.cr) {
		case 0:	_a_rol(vcpuins.rrm,(t_vaddrcc)&vcpu.cl,16);break;
		case 1:	_a_ror(vcpuins.rrm,(t_vaddrcc)&vcpu.cl,16);break;
		case 2:	_a_rcl(vcpuins.rrm,(t_vaddrcc)&vcpu.cl,16);break;
		case 3:	_a_rcr(vcpuins.rrm,(t_vaddrcc)&vcpu.cl,16);break;
		case 4:	_a_shl(vcpuins.rrm,(t_vaddrcc)&vcpu.cl,16);break;
		case 5:	_a_shr(vcpuins.rrm,(t_vaddrcc)&vcpu.cl,16);break;
		case 6:	UndefinedOpcode();break;
		case 7:	_a_sar(vcpuins.rrm,(t_vaddrcc)&vcpu.cl,16);break;
		default:_impossible_;break;}
	}
	_ce;
}
done AAM()
{
	t_nubit8 base;
	_cb("AAM");
	i386(0xd4) {
		_adv;
		_chk(_d_imm(1));
	} else {
		vcpu.ip++;
		_d_imm(1);
	}
	base = d_nubit8(vcpuins.rimm);
	vcpu.ah = vcpu.al / base;
	vcpu.al = vcpu.al % base;
	vcpuins.bit = 0x08;
	vcpuins.result = GetMax8(vcpu.al);
	_chk(_kaf_set_flags(AAM_FLAG));
	vcpuins.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF);
	_ce;
}
done AAD()
{
	t_nubit8 base;
	_cb("AAD");
	i386(0xd5) {
		_adv;
		_chk(_d_imm(1));
	} else {
		vcpu.ip++;
		_d_imm(1);
	}
	base = d_nubit8(vcpuins.rimm);
	vcpu.al = GetMax8(vcpu.al + (vcpu.ah * base));
	vcpu.ah = 0x00;
	vcpuins.bit = 0x08;
	vcpuins.result = GetMax8(vcpu.al);
	_chk(_kaf_set_flags(AAD_FLAG));
	vcpuins.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF);
	_ce;
}
tots XLAT()
{
	_cb("XLAT");
	i386(0xd7) {
		_adv;
		switch (_GetAddressSize) {
		case 2: _bb("AddressSize(2)");
			_chk(vcpu.al = (t_nubit8)_m_read_logical(vcpuins.roverds, (vcpu.bx + vcpu.al), 1));
			_be;break;
		case 4: _bb("AddressSize(4)");
			_newins_;
			_chk(vcpu.al = (t_nubit8)_m_read_logical(vcpuins.roverds, (vcpu.ebx + vcpu.al), 1));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(vcpu.al = (t_nubit8)_m_read_logical(vcpuins.roverds, (vcpu.bx + vcpu.al), 1));
	}
	_ce;
}
done LOOPNZ()
{
	_cb("LOOPNZ");
	i386(0xe0) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_loopcc(vcpuins.rimm, !_GetEFLAGS_ZF));
	} else {
		vcpu.ip++;
		_d_imm(1);
		vcpu.cx--;
		if(vcpu.cx && !_GetEFLAGS_ZF) vcpu.eip += d_nsbit8(vcpuins.rimm);
	}
	_ce;
}
done LOOPZ()
{
	_cb("LOOPZ");
	i386(0xe1) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_loopcc(vcpuins.rimm, _GetEFLAGS_ZF));
	} else {
		vcpu.ip++;
		_d_imm(1);
		vcpu.cx--;
		if(vcpu.cx && _GetEFLAGS_ZF) vcpu.eip += d_nsbit8(vcpuins.rimm);
	}
	_ce;
}
done LOOP()
{
	_cb("LOOP");
	i386(0xe2) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_loopcc(vcpuins.rimm, 1));
	} else {
		vcpu.ip++;
		_d_imm(1);
		vcpu.cx--;
		if(vcpu.cx) vcpu.eip += d_nsbit8(vcpuins.rimm);
	}
	_ce;
}
done JCXZ_REL8()
{
	t_nubit32 cecx = 0x00000000;
	_cb("JCXZ_REL8");
	i386(0xe3) {
		_adv;
		_chk(_d_imm(1));
		switch (_GetAddressSize) {
		case 2:	cecx = vcpu.cx;break;
		case 4: cecx = vcpu.ecx;break;
		default:_impossible_;break;}
		_chk(_e_jcc(vcpuins.rimm, 1, !cecx));
	} else {
		vcpu.ip++;
		_d_imm(1);
		JCC(vcpuins.rimm,!vcpu.cx, 8);
	}
	_ce;
}
done IN_AL_I8()
{
	_cb("IN_AL_I8");
	i386(0xe4) {
		_adv;
		_chk(_d_imm(1));
		_chk(vcpu.al = _p_input(GetMax8(vcpuins.cimm), 1));
	} else {
		vcpu.ip++;
		_d_imm(1);
		ExecFun(vport.in[d_nubit8(vcpuins.rimm)]);
		vcpu.al = vport.iobyte;
	}
	_ce;
}
tots IN_AX_I8()
{
	_cb("IN_AX_I8");
	_newins_;
	i386(0xe5) {
		_adv;
		_chk(_d_imm(1));
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(vcpu.ax = (t_nubit16)_p_input(GetMax8(vcpuins.cimm), 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(vcpu.eax = (t_nubit32)_p_input(GetMax8(vcpuins.cimm), 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(1);
		ExecFun(vport.in[d_nubit8(vcpuins.rimm)]);
		vcpu.ax = vport.ioword;
	}
	_ce;
}
done OUT_I8_AL()
{
	_cb("OUT_I8_AL");
	i386(0xe6) {
		_adv;
		_chk(_d_imm(1));
		_chk(_p_output(GetMax8(vcpuins.cimm), vcpu.al, 1));
	} else {
		vcpu.ip++;
		_d_imm(1);
		vport.iobyte = vcpu.al;
		ExecFun(vport.out[d_nubit8(vcpuins.rimm)]);
	}
	_ce;

}
tots OUT_I8_AX()
{
	_cb("OUT_I8_AX");
	_newins_;
	i386(0xe7) {
		_adv;
		_chk(_d_imm(1));
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_p_output(GetMax8(vcpuins.cimm), vcpu.ax, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_p_output(GetMax8(vcpuins.cimm), vcpu.eax, 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(1);
		vport.ioword = vcpu.ax;
		ExecFun(vport.out[d_nubit8(vcpuins.rimm)]);
	}
	_ce;
}
done CALL_REL16()
{
	t_nsbit16 rel16;
	t_nsbit32 rel32;
	_cb("CALL_REL16");
	i386(0xe8) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			rel16 = d_nsbit16(vcpuins.rimm);
			_chk(_e_call_near(GetMax16(vcpu.ip + rel16), 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_d_imm(4));
			rel32 = d_nsbit32(vcpuins.rimm);
			_chk(_e_call_near(GetMax32(vcpu.eip + rel32), 4));
			_be;break;
		default: _bb("OperandSize");
			_chk(_SetExcept_CE(_GetOperandSize));
			_be;break;
		}
	} else {
		vcpu.ip++;
		_d_imm(2);
		rel16 = d_nsbit16(vcpuins.rimm);
		PUSH((t_vaddrcc)&vcpu.ip,16);
		vcpu.ip += rel16;
	}
	_ce;
}
done JMP_REL16()
{
	_cb("JMP_REL16");
	i386(0xe9) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		_chk(_e_jcc(vcpuins.rimm, _GetOperandSize, 1));
	} else {
		vcpu.ip++;
		_d_imm(2);
		vcpu.eip += d_nsbit16(vcpuins.rimm);
		vcpu.eip &= 0x0000ffff;
	}
	_ce;
}
done JMP_PTR16_16()
{
	t_nubit16 newcs = 0x0000;
	t_nubit32 neweip = 0x00000000;
	_cb("JMP_PTR16_16");
	i386(0xea) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			neweip = (t_nubit16)vcpuins.cimm;
			_chk(_d_imm(2));
			newcs = (t_nubit16)vcpuins.cimm;
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_d_imm(4));
			neweip = (t_nubit32)vcpuins.cimm;
			_chk(_d_imm(2));
			newcs = (t_nubit16)vcpuins.cimm;
			_be;break;
		default:_impossible_;break;}
		_chk(_e_jmp_far(newcs, neweip, _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_imm(2);
		neweip = d_nubit16(vcpuins.rimm);
		_d_imm(2);
		newcs = d_nubit16(vcpuins.rimm);
		vcpu.eip = neweip;
		_s_load_cs(newcs);
	}
	_ce;
}
done JMP_REL8()
{
	_cb("JMP_REL8");
	i386(0xeb) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_jcc(vcpuins.rimm, 1, 1));
	} else {
		vcpu.ip++;
		_d_imm(1);
		vcpu.eip += d_nsbit8(vcpuins.rimm);
	}
	_ce;
}
done IN_AL_DX()
{
	_cb("IN_AL_DX");
	i386(0xec) {
		_adv;
		_chk(vcpu.al = (t_nubit8)_p_input(vcpu.dx, 1));
	} else {
		vcpu.ip++;
		ExecFun(vport.in[vcpu.dx]);
		vcpu.al = vport.iobyte;
	}
	_ce;
}
tots IN_AX_DX()
{
	_cb("IN_AX_DX");
	_newins_;
	i386(0xed) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(vcpu.ax = (t_nubit16)_p_input(vcpu.dx, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(vcpu.eax = (t_nubit32)_p_input(vcpu.dx, 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		ExecFun(vport.in[vcpu.dx]);
		vcpu.ax = vport.ioword;
	}
	_ce;
}
done OUT_DX_AL()
{
	_cb("OUT_DX_AL");
	i386(0xee) {
		_adv;
		_chk(_p_output(vcpu.dx, vcpu.al, 1));
	} else {
		vcpu.ip++;
		vport.iobyte = vcpu.al;
		ExecFun(vport.out[vcpu.dx]);
	}
	_ce;
}
tots OUT_DX_AX()
{
	_cb("OUT_DX_AX");
	_newins_;
	i386(0xef) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_p_output(vcpu.dx, vcpu.ax, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_p_output(vcpu.dx, vcpu.eax, 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		vport.ioword = vcpu.ax;
		ExecFun(vport.out[vcpu.dx]);
	}
	_ce;
}
tots PREFIX_LOCK()
{
	t_nubit8 opcode = 0x00;
	t_nubit8 modrm = 0x00;
	t_nubit8 opcode_0f = 0x00;
	t_nubit32 ceip = vcpu.eip;
	_cb("PREFIX_LOCK");
	_newins_;
	i386(0xf0) {
		_adv;
		do {
			_chk(opcode = (t_nubit8)_s_read_cs(ceip, 1));
			ceip++;
		} while (IsPrefix(opcode));
		switch (opcode) {
		case 0x00: case 0x01: /* ADD */
		case 0x08: case 0x09: /* OR */
		case 0x10: case 0x11: /* ADC */
		case 0x18: case 0x19: /* SBB */
		case 0x20: case 0x21: /* AND */
		case 0x28: case 0x29: /* SUB */
		case 0x30: case 0x31: /* XOR */
		case 0x86: case 0x87: /* XCHG */
		case 0xf6: case 0xf7: /* NOT, NEG */
		case 0xfe: case 0xff: /* DEC, INC */
			vcpu.flaglock = 1;
			break;
		case 0x80: case 0x81: case 0x83:
			_bb("opcode(0x80/0x81/0x83)");
			_chk(modrm = (t_nubit8)_s_read_cs(ceip, 1));
			if (_GetModRM_REG(modrm) != 7)
				vcpu.flaglock = 1;
			else {
				_bb("ModRM_REG(7)");
				_chk(_SetExcept_UD(0));
				_be;
			}
			_be;break;
		case 0x0f: _bb("opcode(0f)");
			_chk(opcode_0f = (t_nubit8)_s_read_cs(ceip, 1));
			switch (opcode_0f) {
			case 0xa3: /* BT */
			case 0xab: /* BTS */
			case 0xb3: /* BTR */
			case 0xbb: /* BTC */
			case 0xba:
				vcpu.flaglock = 1;
				break;
			default: _bb("opcode_0f");
				_chk(_SetExcept_UD(0));
				_be;break;
			}
			_be;break;
		default: _bb("opcode");
			_chk(_SetExcept_UD(0));
			_be;break;
		}
	} else _adv;
	_ce;
}
done PREFIX_REPNZ()
{
	_cb("PREFIX_REPNZ");
	i386(0xf2) {
		_adv;
		vcpuins.prefix_rep = PREFIX_REP_REPZNZ;
	} else {
		vcpu.ip++;
		vcpuins.prefix_rep = PREFIX_REP_REPZNZ;
	}
	_ce;
}
done PREFIX_REPZ()
{
	_cb("PREFIX_REPZ");
	i386(0xf3) {
		_adv;
		vcpuins.prefix_rep = PREFIX_REP_REPZ;
	} else {
		vcpu.ip++;
		vcpuins.prefix_rep = PREFIX_REP_REPZ;
	}
	_ce;
}
tots HLT()
{
	_cb("HLT");
	_newins_;
	if (_GetCR0_PE && _GetCPL) {
		_bb("CR0_PE(1),CPL(!0)");
		_SetExcept_GP(0);
		_be;
	}
	_adv;
	vcpu.flaghalt = 1;
	_ce;
}
done CMC()
{
	_cb("CMC");
	i386(0xf5) {
		_adv;
		vcpu.eflags ^= VCPU_EFLAGS_CF;
	} else {
		vcpu.ip++;
		vcpu.eflags ^= VCPU_EFLAGS_CF;
	}
	_ce;
}
tots INS_F6()
{
	t_nubit8 modrm = 0x00;
	_cb("INS_F6");
	i386(0xf6) {
		_adv;
		_chk(modrm = (t_nubit8)_s_read_cs(vcpu.eip, 1));
		switch (_GetModRM_REG(modrm)) {
		case 0: /* TEST_RM8_I8 */
			_bb("TEST_RM8_I8");
			_chk(_d_modrm(0, 1, 0));
			_chk(_d_imm(1));
			_chk(_a_test(vcpuins.rrm, vcpuins.rimm, 8));
			_be;break;
		case 1: /* UndefinedOpcode */
			_bb("ModRM_REG(1)");
			_chk(UndefinedOpcode());
			_be;break;
		case 2: /* NOT_RM8 */
			_bb("NOT_RM8");
			_chk(_d_modrm(0, 1, 1));
			_chk(_a_not(vcpuins.rrm, 8));
			_be;break;
		case 3: /* NEG_RM8 */
			_bb("NEG_RM8");
			_chk(_d_modrm(0, 1, 1));
			_chk(_a_neg(vcpuins.rrm, 8));
			_be;break;
		case 4: /* MUL_RM8 */
			_bb("MUL_RM8");
			_chk(_d_modrm(0, 1, 0));
			_chk(_a_mul(vcpuins.rrm, 8));
			_be;break;
		case 5: /* IMUL_RM8 */
			_bb("IMUL_RM8");
			_chk(_d_modrm(0, 1, 0));
			_chk(_a_imul(vcpuins.rrm, 8));
			_be;break;
		case 6: /* DIV_RM8 */
			_bb("DIV_RM8");
			_chk(_d_modrm(0, 1, 0));
			_chk(_a_div(vcpuins.rrm, 8));
			_be;break;
		case 7: /* IDIV_RM8 */
			_bb("IDIV_RM8");
			_newins_;
			_chk(_d_modrm(0, 1, 0));
			_chk(_a_idiv(vcpuins.rrm, 8));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_modrm(0, 1, 1);
		switch (vcpuins.cr) {
		case 0: _d_imm(1);
			_a_test(vcpuins.rrm, vcpuins.rimm, 8);
			break;
		case 2: _a_not (vcpuins.rrm, 8);break;
		case 3: _a_neg (vcpuins.rrm, 8);break;
		case 4: _a_mul (vcpuins.rrm, 8);break;
		case 5: _a_imul(vcpuins.rrm, 8);break;
		case 6: _a_div(vcpuins.rrm, 8);break;
		case 7: _a_idiv(vcpuins.rrm, 8);break;
		default:_impossible_;break;}
	}
	_ce;
}
tots INS_F7()
{
	t_nubit8 modrm = 0x00;
	_cb("INS_F7");
	i386(0xf7) {
		_adv;
		_chk(modrm = (t_nubit8)_s_read_cs(vcpu.eip, 1));
		switch (_GetModRM_REG(modrm)) {
		case 0: /* TEST_RM16_I16 */
			_bb("TEST_RM16_I16");
			_chk(_d_modrm(0, _GetOperandSize, 0));
			_chk(_d_imm(_GetOperandSize));
			_chk(_a_test(vcpuins.rrm, vcpuins.rimm, _GetOperandSize * 8));
			_be;break;
		case 1: /* UndefinedOpcode */
			_bb("ModRM_REG(1)");
			_chk(UndefinedOpcode());
			_be;break;
		case 2: /* NOT_RM16 */
			_bb("NOT_RM16");
			_chk(_d_modrm(0, _GetOperandSize, 1));
			_chk(_a_not(vcpuins.rrm, _GetOperandSize * 8));
			_be;break;
		case 3: /* NEG_RM16 */
			_bb("NEG_RM16");
			_chk(_d_modrm(0, _GetOperandSize, 1));
			_chk(_a_neg(vcpuins.rrm, _GetOperandSize * 8));
			_be;break;
		case 4: /* MUL_RM16 */
			_bb("MUL_RM16");
			_chk(_d_modrm(0, _GetOperandSize, 0));
			_chk(_a_mul(vcpuins.rrm, _GetOperandSize * 8));
			_be;break;
		case 5: /* IMUL_RM16 */
			_bb("IMUL_RM16");
			_chk(_d_modrm(0, _GetOperandSize, 0));
			_chk(_a_imul(vcpuins.rrm, _GetOperandSize * 8));
			_be;break;
		case 6: /* DIV_RM16 */
			_bb("DIV_RM16");
			_chk(_d_modrm(0, _GetOperandSize, 0));
			_chk(_a_div(vcpuins.rrm, _GetOperandSize * 8));
			_be;break;
		case 7: /* IDIV_RM16 */
			_bb("IDIV_RM16");
			_chk(_d_modrm(0, _GetOperandSize, 0));
			_chk(_a_idiv(vcpuins.rrm, _GetOperandSize * 8));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_modrm(0, 2, 1);
		switch (vcpuins.cr) {
		case 0: _d_imm(2);
			_a_test(vcpuins.rrm, vcpuins.rimm,16);
			break;
		case 2: _a_not (vcpuins.rrm,16);break;
		case 3: _a_neg (vcpuins.rrm,16);break;
		case 4: _a_mul (vcpuins.rrm,16);break;
		case 5: _a_imul(vcpuins.rrm,16);break;
		case 6: _a_div(vcpuins.rrm, 16);break;
		case 7: _a_idiv(vcpuins.rrm,16);break;
		default:_impossible_;break;}
	}
	_ce;
}
done CLC()
{
	_cb("CLC");
	i386(0xf8) {
		_adv;
		_ClrEFLAGS_CF;
	} else {
		vcpu.ip++;
		_ClrEFLAGS_CF;
	}
	_ce;
}
done STC()
{
	_cb("STC");
	i386(0xf9) {
		_adv;
		_SetEFLAGS_CF;
	} else {
		vcpu.ip++;
		_SetEFLAGS_CF;
	}
	_ce;
}
tots CLI()
{
	_cb("CLI");
	i386(0xfa) {
		_adv;
		if (!_GetCR0_PE)
			_ClrEFLAGS_IF;
		else {
			_bb("CR0_PE(1)");
			_newins_;
			if (!_GetEFLAGS_VM) {
				_bb("EFLAGS_VM(0)");
				if (_GetCPL <= (t_nubit8)(_GetEFLAGS_IOPL))
					_ClrEFLAGS_IF;
				else
					_chk(_SetExcept_GP(0));
				_be;
			} else {
				_bb("EFLAGS_VM(1)");
				if (_GetEFLAGS_IOPL == 3)
					_ClrEFLAGS_IF;
				else
					_chk(_SetExcept_GP(0));
				_be;
			}
			_be;
		}
	} else {
		vcpu.ip++;
		_ClrEFLAGS_IF;
	}
	_ce;
}
tots STI()
{
	_cb("CLI");
	i386(0xfb) {
		_adv;
		if (!_GetCR0_PE)
			_SetEFLAGS_IF;
		else {
			_bb("CR0_PE(1)");
			_newins_;
			if (!_GetEFLAGS_VM) {
				_bb("EFLAGS_VM(0)");
				if (_GetCPL <= (t_nubit8)(_GetEFLAGS_IOPL))
					_SetEFLAGS_IF;
				else
					_chk(_SetExcept_GP(0));
				_be;
			} else {
				_bb("EFLAGS_VM(1)");
				if (_GetEFLAGS_IOPL == 3)
					_SetEFLAGS_IF;
				else
					_chk(_SetExcept_GP(0));
				_be;
			}
			_be;
		}
		vcpuins.flagmaskint = 1;
	} else {
		vcpu.ip++;
		_SetEFLAGS_IF;
	}
	_ce;
}
done CLD()
{
	_cb("CLD");
	i386(0xfc) {
		_adv;
		_ClrEFLAGS_DF;
	} else {
		vcpu.ip++;
		_ClrEFLAGS_DF;
	}
	_ce;
}
done STD()
{
	_cb("CLD");
	i386(0xfd) {
		_adv;
		_SetEFLAGS_DF;
	} else {
		vcpu.ip++;
		_SetEFLAGS_DF;
	}
	_ce;
}
done INS_FE()
{
	_cb("INS_FE");
	i386(0xfe) {
		_adv;
		_chk(_d_modrm(0, 1, 1));
		switch (vcpuins.cr) {
		case 0: /* INC_RM8 */
			_bb("INC_RM8");
			_chk(_a_inc(vcpuins.rrm, 8));
			_be;break;
		case 1: /* DEC_RM8 */
			_bb("DEC_RM8");
			_chk(_a_dec(vcpuins.rrm, 8));
			_be;break;
		case 2: _bb("cr(2)");_chk(UndefinedOpcode());_be;break;
		case 3: _bb("cr(3)");_chk(UndefinedOpcode());_be;break;
		case 4: _bb("cr(4)");_chk(UndefinedOpcode());_be;break;
		case 5: _bb("cr(5)");_chk(UndefinedOpcode());_be;break;
		case 6: _bb("cr(6)");_chk(UndefinedOpcode());_be;break;
		case 7: _bb("cr(7)");_chk(UndefinedOpcode());_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_modrm(0, 1, 1);
		switch (vcpuins.cr) {
		case 0: _a_inc(vcpuins.rrm, 8);break;
		case 1: _a_dec(vcpuins.rrm, 8);break;
		default:_impossible_;break;}
	}
	_ce;
}
done INS_FF()
{
	t_nubit8 modrm = 0x00;
	t_nubit16 newcs = 0x0000;
	t_nubit32 neweip = 0x00000000;
	_cb("INS_FF");
	i386(0xff) {
		_adv;
		_chk(modrm = (t_nubit8)_s_read_cs(vcpu.eip, 1));
		switch (_GetModRM_REG(modrm)) {
		case 0: /* INC_RM16 */
			_bb("INC_RM16");
			_chk(_d_modrm(0, _GetOperandSize, 1));
			_chk(_a_inc(vcpuins.rrm,16));
			_be;break;
		case 1: /* DEC_RM16 */
			_bb("DEC_RM16");
			_chk(_d_modrm(0, _GetOperandSize, 1));
			_chk(_a_dec(vcpuins.rrm, 16));
			_be;break;
		case 2: /* CALL_RM16 */
			_bb("CALL_RM16");
			_chk(_d_modrm(0, _GetOperandSize, 0));
			neweip = ((_GetOperandSize == 2) ? GetMax16(vcpuins.crm) : GetMax32(vcpuins.crm));
			_chk(_e_call_near(neweip, _GetOperandSize));
			_be;break;
		case 3: /* CALL_M16_16 */
			_bb("CALL_M16_16");
			_chk(_d_modrm(0, _GetOperandSize + 2, 0));
			if (!vcpuins.flagmem) {
				_bb("flagmem(0)");
				_chk(UndefinedOpcode());
				_be;
			}
			switch (_GetOperandSize) {
			case 2:
				neweip = d_nubit16(vcpuins.rrm);
				newcs = d_nubit16(vcpuins.rrm + 2);
				break;
			case 4:
				neweip = d_nubit32(vcpuins.rrm);
				newcs = d_nubit16(vcpuins.rrm + 4);
				break;
			default:_impossible_;break;}
			_chk(_e_call_far(newcs, neweip, _GetOperandSize));
			_be;break;
		case 4: /* JMP_RM16 */
			_bb("JMP_RM16");
			_chk(_d_modrm(0, _GetOperandSize, 0));
			neweip = ((_GetOperandSize == 2) ? GetMax16(vcpuins.crm) : GetMax32(vcpuins.crm));
			_chk(_e_jmp_near(neweip, _GetOperandSize));
			_be;break;
		case 5: /* JMP_M16_16 */
			_bb("JMP_M16_16");
			_chk(_d_modrm(0, _GetOperandSize + 2, 0));
			if (!vcpuins.flagmem) {
				_bb("flagmem(0)");
				_chk(UndefinedOpcode());
				_be;
			}
			switch (_GetOperandSize) {
			case 2:
				neweip = d_nubit16(vcpuins.rrm);
				newcs = d_nubit16(vcpuins.rrm + 2);
				break;
			case 4:
				neweip = d_nubit32(vcpuins.rrm);
				newcs = d_nubit16(vcpuins.rrm + 4);
				break;
			default:_impossible_;break;}
			_chk(_e_jmp_far(newcs, neweip, _GetOperandSize));
			_be;break;
		case 6: /* PUSH_RM16 */
			_bb("PUSH_RM16");
			_chk(_d_modrm(0, _GetOperandSize, 0));
			_chk(_e_push(vcpuins.rrm, _GetOperandSize));
			_be;break;
		case 7: /* UndefinedOpcode */
			_bb("ModRM_REG(7)");
			_chk(UndefinedOpcode());
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_modrm(0, 2, 1);
		switch (vcpuins.cr) {
		case 0:	_a_inc(vcpuins.rrm,16);	break;
		case 1:	_a_dec(vcpuins.rrm,16);	break;
		case 2:	/* CALL_RM16 */
			PUSH((t_vaddrcc)&vcpu.eip,16);
			vcpu.eip = d_nubit16(vcpuins.rrm);
			break;
		case 3:	/* CALL_M16_16 */
			PUSH((t_vaddrcc)&vcpu.cs.selector,16);
			PUSH((t_vaddrcc)&vcpu.eip,16);
			vcpu.eip = d_nubit16(vcpuins.rrm);
			_s_load_cs(d_nubit16(vcpuins.rrm+2));
			break;
		case 4:	/* JMP_RM16 */
			vcpu.eip = d_nubit16(vcpuins.rrm);
			break;
		case 5:	/* JMP_M16_16 */
			vcpu.eip = d_nubit16(vcpuins.rrm);
			_s_load_cs(d_nubit16(vcpuins.rrm+2));
			break;
		case 6: /* PUSH_RM16 */
			PUSH(vcpuins.rrm,16);
			break;
		case 7:
			UndefinedOpcode();
			return;
		default:_impossible_;return;}
	}
	_ce;
}

tots _d_modrm_creg()
{
	_cb("_d_modrm_creg");
	_chk(_kdf_modrm(0, 4, 0));
	if (vcpuins.flagmem) {
		_bb("flagmem(1)");
		_chk(_SetExcept_UD(0));
		_be;
	}
	switch (vcpuins.cr) {
	case 0: vcpuins.rr = (t_vaddrcc)&vcpu.cr0; break;
	case 1: _bb("vcpuins.cr(1)");_chk(_SetExcept_UD(0));_be;break;
	case 2: vcpuins.rr = (t_vaddrcc)&vcpu.cr2; break;
	case 3: vcpuins.rr = (t_vaddrcc)&vcpu.cr3; break;
	case 4: _bb("vcpuins.cr(4)");_chk(_SetExcept_UD(0));_be;break;
	case 5: _bb("vcpuins.cr(5)");_chk(_SetExcept_UD(0));_be;break;
	case 6: _bb("vcpuins.cr(6)");_chk(_SetExcept_UD(0));_be;break;
	case 7: _bb("vcpuins.cr(7)");_chk(_SetExcept_UD(0));_be;break;
	default:_impossible_;break;}
	_ce;
}
tots _d_modrm_dreg()
{
	_cb("_d_modrm_dreg");
	_chk(_kdf_modrm(0, 4, 0));
	if (vcpuins.flagmem) {
		_bb("flagmem(1)");
		_chk(_SetExcept_UD(0));
		_be;
	}
	switch (vcpuins.cr) {
	case 0: vcpuins.rr = (t_vaddrcc)&vcpu.dr0; break;
	case 1: vcpuins.rr = (t_vaddrcc)&vcpu.dr1; break;
	case 2: vcpuins.rr = (t_vaddrcc)&vcpu.dr2; break;
	case 3: vcpuins.rr = (t_vaddrcc)&vcpu.dr3; break;
	case 4: _bb("vcpuins.cr(4)");_chk(_SetExcept_UD(0));_be;break;
	case 5: _bb("vcpuins.cr(5)");_chk(_SetExcept_UD(0));_be;break;
	case 6: vcpuins.rr = (t_vaddrcc)&vcpu.dr6; break;
	case 7: vcpuins.rr = (t_vaddrcc)&vcpu.dr7; break;
	default:_impossible_;break;}
	_ce;
}
tots _d_modrm_treg()
{
	_cb("_d_modrm_treg");
	_chk(_kdf_modrm(0, 4, 0));
	if (vcpuins.flagmem) {
		_bb("flagmem(1)");
		_chk(_SetExcept_UD(0));
		_be;
	}
	switch (vcpuins.cr) {
	case 0: _bb("vcpuins.cr(0)");_chk(_SetExcept_UD(0));_be;break;
	case 1: _bb("vcpuins.cr(1)");_chk(_SetExcept_UD(0));_be;break;
	case 2: _bb("vcpuins.cr(2)");_chk(_SetExcept_UD(0));_be;break;
	case 3: _bb("vcpuins.cr(3)");_chk(_SetExcept_UD(0));_be;break;
	case 4: _bb("vcpuins.cr(4)");_chk(_SetExcept_UD(0));_be;break;
	case 5: _bb("vcpuins.cr(5)");_chk(_SetExcept_UD(0));_be;break;
	case 6: vcpuins.rr = (t_vaddrcc)&vcpu.tr6; break;
	case 7: vcpuins.rr = (t_vaddrcc)&vcpu.tr7; break;
	default:_impossible_;break;}
	_ce;
}
tots _d_bit_rmimm(t_nubit8 regbyte, t_nubit8 rmbyte, t_bool write)
{
	/* rrm = actual destination, cimm = (1 << bitoffset) */
	t_nsbit16 bitoff16 = 0;
	t_nsbit32 bitoff32 = 0;
	t_nubit32 bitoperand = 0;
	_cb("_d_bit_rmimm");
	_chk(_d_modrm(regbyte, rmbyte, write));
	if (!regbyte) _chk(_d_imm(1));
	switch (rmbyte) {
	case 2: _bb("rmbyte(2)");
		if (vcpuins.flagmem && regbyte) {
			/* valid for btcc_m16_r16 */
			_bb("flagmem(1),regbyte(1)");
			bitoff16 = d_nsbit16(vcpuins.rr);
			if (bitoff16 >= 0)
				vcpuins.lrm += 2 * (bitoff16 / 16);
			else
				vcpuins.lrm += 2 * ((bitoff16 - 15) / 16);
			bitoperand = ((t_nubit16)bitoff16) % 16;
			_be;
		} else
			bitoperand = (d_nubit16(vcpuins.rimm) % 16);
		_chk(vcpuins.cimm = GetMax16((1 << bitoperand)));
		_chk(vcpuins.rrm = _m_ref_linear(vcpuins.lrm, 2, write));
		_be;break;
	case 4: _bb("rmbyte(4)");
		if (vcpuins.flagmem && regbyte) {
			_bb("flagmem(1),regbyte(1)");
			bitoff32 = d_nsbit32(vcpuins.rr);
			if (bitoff32 >= 0)
				vcpuins.lrm += 4 * (bitoff32 / 32);
			else
				vcpuins.lrm += 4 * ((bitoff32 - 31) / 32);
			bitoperand = ((t_nubit32)bitoff32) % 32;
			_be;
		} else
			bitoperand = (d_nubit32(vcpuins.rimm) % 32);
		_chk(vcpuins.cimm = GetMax32((1 << bitoperand)));
		_chk(vcpuins.rrm = _m_ref_linear(vcpuins.lrm, 4, write));
		_be;break;
	default: _bb("rmbyte");
		_chk(_SetExcept_CE(rmbyte));
		_be;break;
	}
	_ce;
}

#define SHLD_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SHRD_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)

tots _m_movsx(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 destbyte, t_nubit8 srcbyte)
{
	t_nubit32 cdest = 0x00000000;
	_cb("_m_movsx");
	switch (srcbyte) {
	case 1: cdest = d_nsbit8(rsrc);break;
	case 2: cdest = d_nsbit16(rsrc);break;
	case 4: cdest = d_nsbit32(rsrc);break;
	default: _bb("srcbyte");
		_chk(_SetExcept_CE(srcbyte));
		_be;break;
	}
	switch (destbyte) {
	case 2: d_nubit16(rdest) = GetMax16(cdest);break;
	case 4: d_nubit32(rdest) = GetMax32(cdest);break;
	default: _bb("destbyte");
		_chk(_SetExcept_CE(destbyte));
		_be;break;
	}
	_ce;
}
tots _m_movzx(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 destbyte, t_nubit8 srcbyte)
{
	t_nubit32 cdest = 0x00000000;
	_cb("_m_movzx");
	switch (srcbyte) {
	case 1: cdest = d_nubit8(rsrc);break;
	case 2: cdest = d_nubit16(rsrc);break;
	case 4: cdest = d_nubit32(rsrc);break;
	default: _bb("srcbyte");
		_chk(_SetExcept_CE(srcbyte));
		_be;break;
	}
	switch (destbyte) {
	case 2: d_nubit16(rdest) = GetMax16(cdest);break;
	case 4: d_nubit32(rdest) = GetMax32(cdest);break;
	default: _bb("destbyte");
		_chk(_SetExcept_CE(destbyte));
		_be;break;
	}
	_ce;
}
tots _a_bscc(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit, t_bool forward)
{
	t_nubit32 src = 0;
	t_nubit32 temp = 0;
	_cb("_a_bscc");
	if (forward) temp = 0;
	else temp = bit - 1;
	switch (bit) {
	case 16:
		src = d_nubit16(rsrc);
		if (!src)
			_SetEFLAGS_ZF;
		else {
			_ClrEFLAGS_ZF;
			while (!GetBit(src, (1 << temp))) {
				if (forward) temp++;
				else temp--;
			}
			d_nubit16(rdest) = (t_nubit16)temp;
		}
		break;
	case 32:
		src = d_nubit32(rsrc);
		if (!src)
			_SetEFLAGS_ZF;
		else {
			_ClrEFLAGS_ZF;
			while (!GetBit(src, (1 << temp))) {
				if (forward) temp++;
				else temp--;
			}
			d_nubit32(rdest) = (t_nubit32)temp;
		}
		break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
tots _a_bt(t_vaddrcc dest, t_nubit32 bitoperand, t_nubit8 bit)
{
	_cb("_a_bt");
	switch (bit) {
	case 16: MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, GetBit(d_nubit16(dest), bitoperand));break;
	case 32: MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, GetBit(d_nubit32(dest), bitoperand));break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
tots _a_btc(t_vaddrcc dest, t_nubit32 bitoperand, t_nubit8 bit)
{
	_cb("_a_btc");
	switch (bit) {
	case 16:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, GetBit(d_nubit16(dest), bitoperand));
		MakeBit(d_nubit16(dest), bitoperand, !_GetEFLAGS_CF);
		break;
	case 32:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, GetBit(d_nubit32(dest), bitoperand));
		MakeBit(d_nubit32(dest), bitoperand, !_GetEFLAGS_CF);
		break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
tots _a_btr(t_vaddrcc dest, t_nubit32 bitoperand, t_nubit8 bit)
{
	_cb("_a_btr");
	switch (bit) {
	case 16:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, GetBit(d_nubit16(dest), bitoperand));
		ClrBit(d_nubit16(dest), bitoperand);
		break;
	case 32:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, GetBit(d_nubit32(dest), bitoperand));
		ClrBit(d_nubit32(dest), bitoperand);
		break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
tots _a_bts(t_vaddrcc dest, t_nubit32 bitoperand, t_nubit8 bit)
{
	_cb("_a_bts");
	switch (bit) {
	case 16:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, GetBit(d_nubit16(dest), bitoperand));
		SetBit(d_nubit16(dest), bitoperand);
		break;
	case 32:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, GetBit(d_nubit32(dest), bitoperand));
		SetBit(d_nubit32(dest), bitoperand);
		break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
tots _a_imul2(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	t_nsbit64 cdest = 0x0000000000000000;
	_cb("_a_imul2");
	switch (bit) {
	case 16: _bb("bit(16+16)");
		_newins_;
		vcpuins.bit = 16;
		cdest = GetMax32(d_nsbit16(rdest) * d_nsbit16(rsrc));
		d_nsbit16(rdest) = GetMax16(cdest);
		/* note: should not use rsrc after this point */
		if (GetMax32(cdest) != (t_nsbit32)(d_nsbit16(rdest))) {
			_SetEFLAGS_CF;
			_SetEFLAGS_OF;
		} else {
			_ClrEFLAGS_CF;
			_ClrEFLAGS_OF;
		}
		_be;break;
	case 32: _bb("bit(32+32");
		_newins_;
		vcpuins.bit = 32;
		cdest = GetMax64(d_nsbit32(rdest) * d_nsbit32(rsrc));
		d_nsbit32(rdest) = GetMax32(cdest);
		/* note: should not use rsrc after this point */
		if (GetMax64(cdest) != (t_nsbit64)(d_nsbit32(rdest))) {
			_SetEFLAGS_CF;
			_SetEFLAGS_OF;
		} else {
			_ClrEFLAGS_CF;
			_ClrEFLAGS_OF;
		}
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
tots _a_setcc(t_vaddrcc rdest, t_bool condition)
{
	_cb("_a_setcc");
	d_nubit8(rdest) = !!condition;
	_ce;
}
tots _a_shld(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 count, t_nubit8 bit)
{
	t_bool flagcf = 0;
	t_bool flagbit = 0;
	t_nsbit32 i = 0x00000000;
	_cb("_a_shld");
	count &= 0x1f;
	if (!count) {
		_ce;
		return;
	}
	if (count > bit) {
		/* bad parameters */
		/* dest is undefined */
		/* cf, of, sf, zf, af, pf are undefined */
		vcpuins.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF |
			VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF);
	} else {
		switch (bit) {
		case 16: _bb("bit(16)");
			vcpuins.bit = 16;
			flagcf = !!GetMSB16(d_nubit16(rdest));
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF,
				GetBit(d_nubit16(rdest), (1 << (bit - count))));
			for (i = (t_nsbit32)(bit - 1);i >= (t_nsbit32)count;--i) {
				flagbit = GetBit(d_nubit16(rdest), (1 << (i - count)));
				MakeBit(d_nubit16(rdest), (1 << i), flagbit);
			}
			for (i = (t_nsbit32)(count - 1);i >= 0;--i) {
				flagbit = GetBit(d_nubit16(rsrc), (1 << (i - count + bit)));
				MakeBit(d_nubit16(rdest), (1 << i), flagbit);
			}
			if (count == 1)
				MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
					((!!GetMSB16(d_nubit16(rdest))) ^ flagcf));
			else
				vcpuins.udf |= VCPU_EFLAGS_OF;
			vcpuins.result = GetMax16(d_nubit16(rdest));
			_be;break;
		case 32: _bb("bit(32)");
			flagcf = !!GetMSB32(d_nubit32(rdest));
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF,
				GetBit(d_nubit32(rdest), (1 << (bit - count))));
			for (i = (t_nsbit32)(bit - 1);i >= (t_nsbit32)count;--i) {
				flagbit = GetBit(d_nubit32(rdest), (1 << (i - count)));
				MakeBit(d_nubit32(rdest), (1 << i), flagbit);
			}
			for (i = (t_nsbit32)(count - 1);i >= 0;--i) {
				flagbit = GetBit(d_nubit32(rsrc), (1 << (i - count + bit)));
				MakeBit(d_nubit32(rdest), (1 << i), flagbit);
			}
			if (count == 1)
				MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
					((!!GetMSB32(d_nubit32(rdest))) ^ flagcf));
			else
				vcpuins.udf |= VCPU_EFLAGS_OF;
			vcpuins.result = GetMax32(d_nubit32(rdest));
			_be;break;
		default: _bb("bit");
			_chk(_SetExcept_CE(bit));
			_be;break;
		}
		_chk(_kaf_set_flags(SHLD_FLAG));
		vcpuins.udf |= VCPU_EFLAGS_AF;
	}
	_ce;
}
tots _a_shrd(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 count, t_nubit8 bit)
{
	t_bool flagcf = 0;
	t_bool flagbit = 0;
	t_nsbit32 i = 0x00000000;
	_cb("_a_shrd");
	count &= 0x1f;
	if (!count) {
		_ce;
		return;
	}
	if (count > bit) {
		/* bad parameters */
		/* dest is undefined */
		/* cf, of, sf, zf, af, pf are undefined */
		vcpuins.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF |
			VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF);
	} else {
		switch (bit) {
		case 16: _bb("bit(16)");
			vcpuins.bit = 16;
			flagcf = !!GetMSB16(d_nubit16(rdest));
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF,
				GetBit(d_nubit16(rdest), (1 << (count - 1))));
			for (i = (t_nsbit32)0;i <= (t_nsbit32)(bit - count - 1);++i) {
				flagbit = GetBit(d_nubit16(rdest), (1 << (i + count)));
				MakeBit(d_nubit16(rdest), (1 << i), flagbit);
			}
			for (i = (t_nsbit32)(bit - count);i <= (t_nsbit32)(bit - 1);++i) {
				flagbit = GetBit(d_nubit16(rsrc), (1 << (i + count - bit)));
				MakeBit(d_nubit16(rdest), (1 << i), flagbit);
			}
			if (count == 1)
				MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
					((!!GetMSB16(d_nubit16(rdest))) ^ flagcf));
			else
				vcpuins.udf |= VCPU_EFLAGS_OF;
			vcpuins.result = GetMax16(d_nubit16(rdest));
			_be;break;
		case 32: _bb("bit(32)");
			vcpuins.bit = 32;
			flagcf = !!GetMSB32(d_nubit32(rdest));
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF,
				GetBit(d_nubit32(rdest), (1 << (count - 1))));
			for (i = (t_nsbit32)0;i <= (t_nsbit32)(bit - count - 1);++i) {
				flagbit = GetBit(d_nubit32(rdest), (1 << (i + count)));
				MakeBit(d_nubit32(rdest), (1 << i), flagbit);
			}
			for (i = (t_nsbit32)(bit - count);i <= (t_nsbit32)(bit - 1);++i) {
				flagbit = GetBit(d_nubit32(rsrc), (1 << (i + count - bit)));
				MakeBit(d_nubit32(rdest), (1 << i), flagbit);
			}
			if (count == 1)
				MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
					((!!GetMSB32(d_nubit32(rdest))) ^ flagcf));
			else
				vcpuins.udf |= VCPU_EFLAGS_OF;
			vcpuins.result = GetMax16(d_nubit32(rdest));
			_be;break;
		default: _bb("bit");
			_chk(_SetExcept_CE(bit));
			_be;break;
		}
		_chk(_kaf_set_flags(SHRD_FLAG));
		vcpuins.udf |= VCPU_EFLAGS_AF;
	}
	_ce;
}

tots INS_0F_00()
{
	t_nubit8 modrm = 0x00;
	t_nubit32 csel = 0x00000000;
	_cb("INS_0F_00");
	_newins_;
	_adv;
	if (_IsProtected) {
		_bb("Protected");
		_chk(modrm = (t_nubit8)_s_read_cs(vcpu.eip, 1));
		switch (_GetModRM_REG(modrm)) {
		case 0: /* SLDT_RM16 */
			_bb("SLDT_RM16");
			_chk(_d_modrm(0, ((_GetModRM_MOD(modrm) == 3) ? _GetOperandSize : 2), 1));
			csel = vcpu.ldtr.selector;
			if (_GetOperandSize == 4 && !vcpuins.flagmem)
				d_nubit32(vcpuins.rrm) = GetMax16(csel);
			else
				d_nubit16(vcpuins.rrm) = GetMax16(csel);
			_be;break;
		case 1: /* STR_RM16 */
			_bb("STR_RM16");
			_chk(_d_modrm(0, ((_GetModRM_MOD(modrm) == 3) ? _GetOperandSize : 2), 1));
			csel = vcpu.tr.selector;
			if (_GetOperandSize == 4 && !vcpuins.flagmem)
				d_nubit32(vcpuins.rrm) = GetMax16(csel);
			else
				d_nubit16(vcpuins.rrm) = GetMax16(csel);
		case 2: /* LLDT_RM16 */
			_bb("LLDT_RM16");
			_chk(_d_modrm(0, 2, 0));
			_chk(_s_load_ldtr(GetMax16(vcpuins.crm)));
			_be;break;
		case 3: /* LTR_RM16 */
			_bb("LTR_RM16");
			_chk(_d_modrm(0, 2, 0));
			_chk(_s_load_tr(GetMax16(vcpuins.crm)));
			_be;break;
		case 4: /* VERR_RM16 */
			_bb("VERR_RM16");
			_chk(_d_modrm(0, 2, 0));
			if (_s_check_selector(GetMax16(vcpuins.crm))) {
				_ClrEFLAGS_ZF;
			} else {
				_chk(_s_descriptor(GetMax16(vcpuins.crm), 0));
				if (_IsDescSys(vcpuins.cdesc) ||
						(!_IsDescCodeConform(vcpuins.cdesc) &&
							(_GetCPL > _GetDesc_DPL(vcpuins.cdesc) ||
							_GetSelector_RPL(GetMax16(vcpuins.crm)) > _GetDesc_DPL(vcpuins.cdesc)))) {
					_ClrEFLAGS_ZF;
				} else {
					if (_IsDescData(vcpuins.cdesc) || _IsDescCodeReadable(vcpuins.cdesc)) {
						_SetEFLAGS_ZF;
					} else {
						_ClrEFLAGS_ZF;
					}
				}
			}
			_be;break;
		case 5: /* VERW_RM16 */
			_bb("VERW_RM16");
			_chk(_d_modrm(0, 2, 0));
			if (_s_check_selector(GetMax16(vcpuins.crm))) {
				_ClrEFLAGS_ZF;
			} else {
				_chk(_s_descriptor(GetMax16(vcpuins.crm), 0));
				if (_IsDescSys(vcpuins.cdesc) ||
						(!_IsDescCodeConform(vcpuins.cdesc) &&
							(_GetCPL > _GetDesc_DPL(vcpuins.cdesc) ||
							_GetSelector_RPL(GetMax16(vcpuins.crm)) > _GetDesc_DPL(vcpuins.cdesc)))) {
					_ClrEFLAGS_ZF;
				} else {
					if (_IsDescDataWritable(vcpuins.cdesc)) {
						_SetEFLAGS_ZF;
					} else {
						_ClrEFLAGS_ZF;
					}
				}
			}
			_be;break;
		case 6: _bb("ModRM_REG(6)");_chk(UndefinedOpcode());_be;break;
		case 7: _bb("ModRM_REG(7)");_chk(UndefinedOpcode());_be;break;
		default:_impossible_;break;}
		_be;
	} else {
		_bb("!Protected");
		_chk(UndefinedOpcode());
		_be;
	}
	_ce;
}
tots INS_0F_01()
{
	t_nubit8 modrm = 0x00;
	t_nubit16 limit = 0x0000;
	t_nubit32 base = 0x00000000;
	_cb("INS_0F_01");
	_newins_;
	_adv;
	_chk(modrm = (t_nubit8)_s_read_cs(vcpu.eip, 1));
	switch (_GetModRM_REG(modrm)) {
	case 0: /* SGDT_M16_32 */
		_bb("SGDT_M16_32");
		_chk(_d_modrm(0, 6, 1));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		d_nubit16(vcpuins.rrm) = _GetGDTR_Limit;
		switch (_GetOperandSize) {
		case 2: d_nubit24(vcpuins.rrm + 2) = GetMax24(_GetGDTR_Base);break;
		case 4: d_nubit32(vcpuins.rrm + 2) = GetMax32(_GetGDTR_Base);break;
		default:_impossible_;break;}
		_be;break;
	case 1: /* SIDT_M16_32 */
		_bb("SIDT_M16_32");
		_chk(_d_modrm(0, 6, 1));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		d_nubit16(vcpuins.rrm) = _GetIDTR_Limit;
		switch (_GetOperandSize) {
		case 2: d_nubit24(vcpuins.rrm + 2) = GetMax24(_GetIDTR_Base);break;
		case 4: d_nubit32(vcpuins.rrm + 2) = GetMax32(_GetIDTR_Base);break;
		default:_impossible_;break;}
		_be;break;
	case 2: /* LGDT_M16_32 */
		_bb("LGDT_M16_32");
		_chk(_d_modrm(0, 6, 0));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		limit = d_nubit16(vcpuins.rrm);
		base = d_nubit32(vcpuins.rrm + 2);
		_chk(_s_load_gdtr(base, limit, _GetOperandSize));
		_be;break;
	case 3: /* LIDT_M16_32 */
		_bb("LIDT_M16_32");
		_chk(_d_modrm(0, 6, 0));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		limit = d_nubit16(vcpuins.rrm);
		base = d_nubit32(vcpuins.rrm + 2);
		_chk(_s_load_idtr(base, limit, _GetOperandSize));
		_be;break;
	case 4: /* SMSW_RM16 */
		_bb("SMSW_RM16");
		_chk(_d_modrm(0, ((_GetModRM_MOD(modrm) == 3) ? _GetOperandSize : 2), 1));
		if (_GetOperandSize == 4 && !vcpuins.flagmem)
			d_nubit32(vcpuins.rrm) = GetMax16(vcpu.cr0);
		else
			d_nubit16(vcpuins.rrm) = GetMax16(vcpu.cr0);
		_be;break;
	case 5: _bb("ModRM_REG(5)");_chk(UndefinedOpcode());_be;break;
	case 6: /* LMSW_RM16 */
		_bb("LMSW_RM16");
		_chk(_d_modrm(0, 2, 0));
		_chk(_s_load_cr0_msw(GetMax16(vcpuins.crm)));
		_be;break;
	case 7: _bb("ModRM_REG(7)");_chk(UndefinedOpcode());_be;break;
	default:_impossible_;break;}
	_ce;
}
tots LAR_R16_RM16()
{
	t_nubit16 selector = 0x0000;
	_cb("LAR_R16_RM16");
	_newins_;
	_adv;
	if (_IsProtected) {
		_bb("Protected(1)");
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
		selector = GetMax16(vcpuins.crm);
		if (_s_check_selector(selector)) {
			_ClrEFLAGS_ZF;
		} else {
			_chk(_s_descriptor(selector, 0));
			if (_IsDescUser(vcpuins.cdesc)) {
				if (_IsDescCodeConform(vcpuins.cdesc))
					_SetEFLAGS_ZF;
				else {
					if (_GetCPL > _GetDesc_DPL(vcpuins.cdesc) || _GetSelector_RPL(selector) > _GetDesc_DPL(vcpuins.cdesc))
						_ClrEFLAGS_ZF;
					else
						_SetEFLAGS_ZF;
				}
			} else {
				switch (_GetDesc_Type(vcpuins.cdesc)) {
				case VCPU_DESC_SYS_TYPE_TSS_16_AVL:
				case VCPU_DESC_SYS_TYPE_LDT:
				case VCPU_DESC_SYS_TYPE_TSS_16_BUSY:
				case VCPU_DESC_SYS_TYPE_CALLGATE_16:
				case VCPU_DESC_SYS_TYPE_TASKGATE:
				case VCPU_DESC_SYS_TYPE_TSS_32_AVL:
				case VCPU_DESC_SYS_TYPE_TSS_32_BUSY:
				case VCPU_DESC_SYS_TYPE_CALLGATE_32:
					_SetEFLAGS_ZF;
					break;
				default:
					_ClrEFLAGS_ZF;
				}
			}
		}
		if (_GetEFLAGS_ZF) {
			switch (_GetOperandSize) {
			case 2:
				d_nubit16(vcpuins.rr) = GetMax16(vcpuins.cdesc >> 32) & 0xff00;
				break;
			case 4:
				d_nubit32(vcpuins.rr) = GetMax32(vcpuins.cdesc >> 32) & 0x00ffff00;
				break;
			default:_impossible_;break;}
		}
		_be;
	} else {
		_bb("Protected(0)");
		_chk(UndefinedOpcode());
		_be;
	}
	_ce;
}
tots LSL_R16_RM16()
{
	t_nubit16 selector = 0x0000;
	t_nubit32 limit = 0x00000000;
	_cb("LSL_R16_RM16");
	_newins_;
	_adv;
	if (_IsProtected) {
		_bb("Protected(1)");
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
		selector = GetMax16(vcpuins.crm);
		if (_s_check_selector(selector)) {
			_ClrEFLAGS_ZF;
		} else {
			_chk(_s_descriptor(selector, 0));
			if (_IsDescUser(vcpuins.cdesc)) {
				if (_IsDescCodeConform(vcpuins.cdesc))
					_SetEFLAGS_ZF;
				else {
					if (_GetCPL > _GetDesc_DPL(vcpuins.cdesc) || _GetSelector_RPL(selector) > _GetDesc_DPL(vcpuins.cdesc))
						_ClrEFLAGS_ZF;
					else
						_SetEFLAGS_ZF;
				}
			} else {
				switch (_GetDesc_Type(vcpuins.cdesc)) {
				case VCPU_DESC_SYS_TYPE_TSS_16_AVL:
				case VCPU_DESC_SYS_TYPE_LDT:
				case VCPU_DESC_SYS_TYPE_TSS_16_BUSY:
				case VCPU_DESC_SYS_TYPE_TSS_32_AVL:
				case VCPU_DESC_SYS_TYPE_TSS_32_BUSY:
					_SetEFLAGS_ZF;
					break;
				default:
					_ClrEFLAGS_ZF;
				}
			}
		}
		if (_GetEFLAGS_ZF) {
			limit = _IsDescSegGranularLarge(vcpuins.cdesc) ?
				((_GetDescSeg_Limit(vcpuins.cdesc) << 12) | 0x0fff) : _GetDescSeg_Limit(vcpuins.cdesc);
			switch (_GetOperandSize) {
			case 2: d_nubit16(vcpuins.rr) = GetMax16(limit);break;
			case 4: d_nubit32(vcpuins.rr) = GetMax32(limit);break;
			default:_impossible_;break;}
		}
		_be;
	} else {
		_bb("Protected(0)");
		_chk(UndefinedOpcode());
		_be;
	}
	_ce;
}
tots CLTS()
{
	_cb("CLTS");
	_newins_;
	_adv;
	if (!_GetCR0_PE) {
		_ClrCR0_TS;
	} else {
		_bb("CR0_PE(1)");
		if (_GetCPL > 0)
			_chk(_SetExcept_GP(0));
		else
			_ClrCR0_TS;
		_be;
	}
	_ce;
}
tots MOV_R32_CR()
{
	_cb("MOV_R32_CR");
	_newins_;
	_adv;
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	_chk(_d_modrm_creg());
	_chk(_m_mov(vcpuins.rrm, vcpuins.rr, 4));
	_ce;
}
tots MOV_R32_DR()
{
	_cb("MOV_R32_DR");
	_newins_;
	_adv;
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	_chk(_d_modrm_dreg());
	_chk(_m_mov(vcpuins.rrm, vcpuins.rr, 4));
	_ce;
}
tots MOV_CR_R32()
{
	_cb("MOV_CR_R32");
	_newins_;
	_adv;
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	_chk(_d_modrm_creg());
	_chk(_m_mov(vcpuins.rr, vcpuins.rrm, 4));
	_ce;
}
tots MOV_DR_R32()
{
	_cb("MOV_DR_R32");
	_newins_;
	_adv;
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	_chk(_d_modrm_dreg());
	_chk(_m_mov(vcpuins.rr, vcpuins.rrm, 4));
	_ce;
}
tots MOV_R32_TR()
{
	_cb("MOV_R32_TR");
	_newins_;
	_adv;
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	_chk(_d_modrm_treg());
	_chk(_m_mov(vcpuins.rrm, vcpuins.rr, 4));
	_ce;
}
tots MOV_TR_R32()
{
	_cb("MOV_TR_R32");
	_newins_;
	_adv;
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	_chk(_d_modrm_treg());
	_chk(_m_mov(vcpuins.rr, vcpuins.rrm, 4));
	_ce;
}
tots JO_REL16()
{
	_cb("JO_REL16");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(vcpuins.rimm, _GetOperandSize, _GetEFLAGS_OF));
	_ce;
}
tots JNO_REL16()
{
	_cb("JNO_REL16");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(vcpuins.rimm, _GetOperandSize, !_GetEFLAGS_OF));
	_ce;
}
tots JC_REL16()
{
	_cb("JC_REL16");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(vcpuins.rimm, _GetOperandSize, _GetEFLAGS_CF));
	_ce;
}
tots JNC_REL16()
{
	_cb("JNC_REL16");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(vcpuins.rimm, _GetOperandSize, !_GetEFLAGS_CF));
	_ce;
}
tots JZ_REL16()
{
	_cb("JZ_REL16");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(vcpuins.rimm, _GetOperandSize, _GetEFLAGS_ZF));
	_ce;
}
tots JNZ_REL16()
{
	_cb("JNZ_REL16");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(vcpuins.rimm, _GetOperandSize, !_GetEFLAGS_ZF));
	_ce;
}
tots JNA_REL16()
{
	_cb("JNA_REL16");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(vcpuins.rimm, _GetOperandSize,
		(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
	_ce;
}
tots JA_REL16()
{
	_cb("JA_REL16");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(vcpuins.rimm, _GetOperandSize,
		!(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
	_ce;
}
tots JS_REL16()
{
	_cb("JS_REL16");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(vcpuins.rimm, _GetOperandSize, _GetEFLAGS_SF));
	_ce;
}
tots JNS_REL16()
{
	_cb("JNS_REL16");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(vcpuins.rimm, _GetOperandSize, !_GetEFLAGS_SF));
	_ce;
}
tots JP_REL16()
{
	_cb("JP_REL16");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(vcpuins.rimm, _GetOperandSize, _GetEFLAGS_PF));
	_ce;
}
tots JNP_REL16()
{
	_cb("JNP_REL16");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(vcpuins.rimm, _GetOperandSize, !_GetEFLAGS_PF));
	_ce;
}
tots JL_REL16()
{
	_cb("JL_REL16");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(vcpuins.rimm, _GetOperandSize, (_GetEFLAGS_SF != _GetEFLAGS_OF)));
	_ce;
}
tots JNL_REL16()
{
	_cb("JNL_REL16");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(vcpuins.rimm, _GetOperandSize, (_GetEFLAGS_SF == _GetEFLAGS_OF)));
	_ce;
}
tots JNG_REL16()
{
	_cb("JNG_REL16");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(vcpuins.rimm, _GetOperandSize,
			(_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF))));
	_ce;
}
tots JG_REL16()
{
	_cb("JG_REL16");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(vcpuins.rimm, _GetOperandSize,
		(!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF))));
	_ce;
}
tots SETO_RM8()
{
	_cb("SETO_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1, 1));
	_chk(_a_setcc(vcpuins.rrm, _GetEFLAGS_OF));
	_ce;
}
tots SETNO_RM8()
{
	_cb("SETO_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1, 1));
	_chk(_a_setcc(vcpuins.rrm, !_GetEFLAGS_OF));
	_ce;
}
tots SETC_RM8()
{
	_cb("SETC_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1, 1));
	_chk(_a_setcc(vcpuins.rrm, _GetEFLAGS_CF));
	_ce;
}
tots SETNC_RM8()
{
	_cb("SETNC_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1, 1));
	_chk(_a_setcc(vcpuins.rrm, !_GetEFLAGS_CF));
	_ce;
}
tots SETZ_RM8()
{
	_cb("SETZ_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1, 1));
	_chk(_a_setcc(vcpuins.rrm, _GetEFLAGS_ZF));
	_ce;
}
tots SETNZ_RM8()
{
	_cb("SETNZ_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1, 1));
	_chk(_a_setcc(vcpuins.rrm, !_GetEFLAGS_ZF));
	_ce;
}
tots SETNA_RM8()
{
	_cb("SETNA_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1, 1));
	_chk(_a_setcc(vcpuins.rrm,
		(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
	_ce;
}
tots SETA_RM8()
{
	_cb("SETA_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1, 1));
	_chk(_a_setcc(vcpuins.rrm,
		!(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
	_ce;
}
tots SETS_RM8()
{
	_cb("SETS_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1, 1));
	_chk(_a_setcc(vcpuins.rrm, _GetEFLAGS_SF));
	_ce;
}
tots SETNS_RM8()
{
	_cb("SETNS_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1, 1));
	_chk(_a_setcc(vcpuins.rrm, !_GetEFLAGS_SF));
	_ce;
}
tots SETP_RM8()
{
	_cb("SETP_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1, 1));
	_chk(_a_setcc(vcpuins.rrm, _GetEFLAGS_PF));
	_ce;
}
tots SETNP_RM8()
{
	_cb("SETNP_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1, 1));
	_chk(_a_setcc(vcpuins.rrm, !_GetEFLAGS_PF));
	_ce;
}
tots SETL_RM8()
{
	_cb("SETL_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1, 1));
	_chk(_a_setcc(vcpuins.rrm, (_GetEFLAGS_SF != _GetEFLAGS_OF)));
	_ce;
}
tots SETNL_RM8()
{
	_cb("SETNL_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1, 1));
	_chk(_a_setcc(vcpuins.rrm, (_GetEFLAGS_SF == _GetEFLAGS_OF)));
	_ce;
}
tots SETNG_RM8()
{
	_cb("SETNG_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1, 1));
	_chk(_a_setcc(vcpuins.rrm,
			(_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF))));
	_ce;
}
tots SETG_RM8()
{
	_cb("SETG_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1, 1));
	_chk(_a_setcc(vcpuins.rrm,
		(!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF))));
	_ce;
}
tots PUSH_FS()
{
	/* note: not sure if operand size is 32,
		push/pop selector only or with higher 16 bit */
	t_nubit32 oldfs = vcpu.fs.selector;
	_cb("PUSH_FS");
	_newins_;
	_adv;
	_chk(_e_push((t_vaddrcc)&oldfs, _GetOperandSize));
	_ce;
}
tots POP_FS()
{
	/* note: not sure if operand size is 32,
		push/pop selector only or with higher 16 bit */
	t_nubit32 newfs = 0x00000000;
	_cb("POP_FS");
	_newins_;
	_adv;
	_chk(_e_pop((t_vaddrcc)&newfs, _GetOperandSize));
	_chk(_s_load_fs(GetMax16(newfs)));
	_ce;
}
tots BT_RM16_R16()
{
	t_nsbitcc bitoffset = 0;
	t_vaddrcc dest = (t_vaddrcc)NULL;
	_cb("BT_RM16_R16");
	_newins_;
	_adv;
	_chk(_d_bit_rmimm(_GetOperandSize, _GetOperandSize, 0));
	_chk(_a_bt(vcpuins.rrm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
	_ce;
}
tots SHLD_RM16_R16_I8()
{
	_cb("SHLD_RM16_R16_I8");
	_newins_;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 1));
	_chk(_d_imm(1));
	_chk(_a_shld(vcpuins.rrm, vcpuins.rr, GetMax8(vcpuins.cimm), _GetOperandSize * 8));
	_ce;
}
tots SHLD_RM16_R16_CL()
{
	_cb("SHLD_RM16_R16_CL");
	_newins_;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 1));
	_chk(_d_imm(1));
	_chk(_a_shld(vcpuins.rrm, vcpuins.rr, vcpu.cl, _GetOperandSize * 8));
	_ce;
}
tots PUSH_GS()
{
	/* note: not sure if operand size is 32,
		push/pop selector only or with higher 16 bit */
	t_nubit32 oldgs = vcpu.gs.selector;
	_cb("PUSH_GS");
	_newins_;
	_adv;
	_chk(_e_push((t_vaddrcc)&oldgs, _GetOperandSize));
	_ce;
}
tots POP_GS()
{
	/* note: not sure if operand size is 32,
		push/pop selector only or with higher 16 bit */
	t_nubit32 newgs = 0x00000000;
	_cb("POP_GS");
	_newins_;
	_adv;
	_chk(_e_pop((t_vaddrcc)&newgs, _GetOperandSize));
	_chk(_s_load_gs(GetMax16(newgs)));
	_ce;
}
tots BTS_RM16_R16()
{
	_cb("BTS_RM16_R16");
	_newins_;
	_adv;
	_chk(_d_bit_rmimm(_GetOperandSize, _GetOperandSize, 1));
	_chk(_a_bts(vcpuins.rrm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
	_ce;
}
tots SHRD_RM16_R16_I8()
{
	_cb("SHRD_RM16_R16_I8");
	_newins_;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 1));
	_chk(_d_imm(1));
	_chk(_a_shrd(vcpuins.rrm, vcpuins.rr, GetMax8(vcpuins.cimm), _GetOperandSize * 8));
	_ce;
}
tots SHRD_RM16_R16_CL()
{
	_cb("SHRD_RM16_R16_CL");
	_newins_;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 1));
	_chk(_d_imm(1));
	_chk(_a_shrd(vcpuins.rrm, vcpuins.rr, vcpu.cl, _GetOperandSize * 8));
	_ce;
}
tots IMUL_R16_RM16()
{
	_cb("IMUL_R16_RM16");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
	_chk(_a_imul2(vcpuins.rr, vcpuins.rrm, _GetOperandSize * 8));
	_ce;
}
tots LSS_R16_M16_16()
{
	t_nubit16 selector = 0x0000;
	t_nubit32 offset = 0x00000000;
	_cb("LSS_R16_M16_16");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2, 0));
	if (!vcpuins.flagmem) {
		_bb("flagmem(0)");
		_chk(UndefinedOpcode());
		_be;
	}
	switch (_GetOperandSize) {
	case 2:
		offset = d_nubit16(vcpuins.rrm);
		selector = d_nubit16(vcpuins.rrm + 2);
		break;
	case 4:
		offset = d_nubit32(vcpuins.rrm);
		selector = d_nubit16(vcpuins.rrm + 4);
		break;
	default:_impossible_;break;}
	_chk(_e_load_far(&vcpu.ss, vcpuins.rr, selector, offset, _GetOperandSize));
	_ce;
}
tots BTR_RM16_R16()
{
	_cb("BTR_RM16_R16");
	_newins_;
	_adv;
	_chk(_d_bit_rmimm(_GetOperandSize, _GetOperandSize, 1));
	_chk(_a_btr(vcpuins.rrm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
	_ce;
}
tots LFS_R16_M16_16()
{
	t_nubit16 selector = 0x0000;
	t_nubit32 offset = 0x00000000;
	_cb("LFS_R16_M16_16");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2, 0));
	if (!vcpuins.flagmem) {
		_bb("flagmem(0)");
		_chk(UndefinedOpcode());
		_be;
	}
	switch (_GetOperandSize) {
	case 2:
		offset = d_nubit16(vcpuins.rrm);
		selector = d_nubit16(vcpuins.rrm + 2);
		break;
	case 4:
		offset = d_nubit32(vcpuins.rrm);
		selector = d_nubit16(vcpuins.rrm + 4);
		break;
	default:_impossible_;break;}
	_chk(_e_load_far(&vcpu.fs, vcpuins.rr, selector, offset, _GetOperandSize));
	_ce;
}
tots LGS_R16_M16_16()
{
	t_nubit16 selector = 0x0000;
	t_nubit32 offset = 0x00000000;
	_cb("LGS_R16_M16_16");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2, 0));
	if (!vcpuins.flagmem) {
		_bb("flagmem(0)");
		_chk(UndefinedOpcode());
		_be;
	}
	switch (_GetOperandSize) {
	case 2:
		offset = d_nubit16(vcpuins.rrm);
		selector = d_nubit16(vcpuins.rrm + 2);
		break;
	case 4:
		offset = d_nubit32(vcpuins.rrm);
		selector = d_nubit16(vcpuins.rrm + 4);
		break;
	default:_impossible_;break;}
	_chk(_e_load_far(&vcpu.gs, vcpuins.rr, selector, offset, _GetOperandSize));
	_ce;
}
tots INS_0F_BA()
{
	t_bool write = 0;
	t_nubit8 modrm = 0x00;
	_cb("INS_0F_BA");
	_newins_;
	_adv;
	_chk(modrm = (t_nubit8)_s_read_cs(vcpu.eip, 1));
	if (_GetModRM_REG(modrm) == 4) write = 0;
	else write = 1;
	_chk(_d_modrm(0, _GetOperandSize, write));
	switch (vcpuins.cr) {
	case 0: _bb("cr(0)");_chk(UndefinedOpcode());_be;break;
	case 1: _bb("cr(1)");_chk(UndefinedOpcode());_be;break;
	case 2: _bb("cr(2)");_chk(UndefinedOpcode());_be;break;
	case 3: _bb("cr(3)");_chk(UndefinedOpcode());_be;break;
	case 4: /* BT_RM16_I8 */
		_bb("BT_RM16_I8");
		_chk(_a_bt(vcpuins.rrm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
		_be;break;
	case 5: /* BTS_RM16_I8 */
		_bb("BTS_RM16_I8");
		_chk(_a_bts(vcpuins.rrm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
		_be;break;
	case 6: /* BTR_RM16_I8 */
		_bb("BTR_RM16_I8");
		_chk(_a_btr(vcpuins.rrm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
		_be;break;
	case 7: /* BTC_RM16_I8 */
		_bb("BTC_RM16_I8");
		_chk(_a_btc(vcpuins.rrm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
tots BTC_RM16_R16()
{
	_cb("BTC_RM16_R16");
	_newins_;
	_adv;
	_chk(_d_bit_rmimm(_GetOperandSize, _GetOperandSize, 1));
	_chk(_a_btc(vcpuins.rrm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
	_ce;
}
tots BSF_R16_RM16()
{
	_cb("BSF_R16_RM16");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
	_chk(_a_bscc(vcpuins.rr, vcpuins.rrm, _GetOperandSize * 8, 1));
	_ce;
}
tots BSR_R16_RM16()
{
	_cb("BSR_R16_RM16");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
	_chk(_a_bscc(vcpuins.rr, vcpuins.rrm, _GetOperandSize * 8, 0));
	_ce;
}
tots MOVZX_R16_RM8()
{
	_cb("MOVZX_R16_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, 1, 0))
	_chk(_m_movzx(vcpuins.rr, vcpuins.rrm, _GetOperandSize, 1));
	_ce;
}
tots MOVZX_R32_RM16()
{
	_cb("MOVZX_R32_RM16");
	_newins_;
	_adv;
	_chk(_d_modrm(4, 2, 0))
	_chk(_m_movzx(vcpuins.rr, vcpuins.rrm, 4, 2));
	_ce;
}
tots MOVSX_R16_RM8()
{
	_cb("MOVSX_R16_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, 1, 0))
	_chk(_m_movsx(vcpuins.rr, vcpuins.rrm, _GetOperandSize, 1));
	_ce;
}
tots MOVSX_R32_RM16()
{
	_cb("MOVSX_R32_RM16");
	_newins_;
	_adv;
	_chk(_d_modrm(4, 2, 0))
	_chk(_m_movsx(vcpuins.rr, vcpuins.rrm, 4, 2));
	_ce;
}

static void ExecInsInit()
{
	vcpuins.prefix_rep = PREFIX_REP_NONE;
	vcpuins.prefix_lock = 0x00;
	vcpuins.prefix_oprsize = 0x00;
	vcpuins.prefix_addrsize = 0x00;
	vcpuins.oldcs = vcpu.cs;
	vcpuins.oldss = vcpu.ss;
	vcpuins.oldeip = vcpu.eip;
	vcpuins.oldesp = vcpu.esp;
	vcpuins.except = 0x00000000;
	vcpuins.excode = 0x00000000;
	vcpuins.flagmss = 0;
	vcpuins.flagmem = 0;
	vcpuins.flaginsloop = 0;
	vcpuins.flagmaskint = 0;
	vcpuins.roverds = &vcpu.ds;
	vcpuins.roverss = &vcpu.ss;
	vcpu.flaglock = 0;
	vcpuins.udf = 0x00000000;
}
static void ExecInsFinal()
{
	if (vcpuins.flaginsloop) {
		vcpu.cs = vcpuins.oldcs;
		vcpu.eip = vcpuins.oldeip;
	}
	if (vcpuins.except) {
		vcpu.cs = vcpuins.oldcs;
		vcpu.eip = vcpuins.oldeip;
		vcpu.ss = vcpuins.oldss;
		vcpu.esp = vcpuins.oldesp;
		vapiCallBackMachineStop();
	}
#if VCPUINS_TRACE == 1
	if (trace.cid) {
		vapiPrint("unbalanced call stack at final:\n");
		_Trace_Release(1);
		vcpu.cs = vcpuins.oldcs;
		vcpu.eip = vcpuins.oldeip;
		vcpu.ss = vcpuins.oldss;
		vcpu.esp = vcpuins.oldesp;
		vapiCallBackMachineStop();
	}
#endif
}
static void ExecIns()
{
	t_nubit8 opcode;
	ExecInsInit();
	do {
		_cb("ExecIns");
		_chb(opcode = (t_nubit8)_s_read_cs(vcpu.eip, 1));
		_chb(ExecFun(vcpuins.table[opcode]));
		_chb(_s_test_eip());
		_chb(_s_test_esp());
		_ce;
	} while (IsPrefix(opcode));
	ExecInsFinal();
}
static void ExecInt()
{
	t_nubit8 intr = 0x00;
	/* hardware interrupt handeler */
	if (vcpuins.flagmaskint)
		return;
	if(!vcpu.flagmasknmi && vcpu.flagnmi) {
		vcpu.flaghalt = 0;
		vcpu.flagnmi = 0;
		_e_int_n(0x02, _GetOperandSize);
	}
	vcpuins.flagrespondint = 0;
	if (_GetEFLAGS_IF && vpicScanINTR()) {
		vcpu.flaghalt = 0;
		intr = vpicGetINTR();
		/*printf("vint = %x\n", intr);*/
		_e_int_n(intr, _GetOperandSize);
		vcpuins.flagrespondint = 1;
	}
	if (_GetEFLAGS_TF) {
		vcpu.flaghalt = 0;
		_e_int_n(0x01, _GetOperandSize);
	}
}

done QDX()
{
	t_nubit32 eflags;
	_cb("QDX");
	_adv;
	_chk(_d_imm(1));
	switch (GetMax8(vcpuins.cimm)) {
	case 0x00:
	case 0xff: /* STOP */
		vapiPrint("\nNXVM CPU STOP at CS:%04X IP:%08X INS:QDX IMM:%02X\n",
			vcpu.cs.selector,vcpu.eip, GetMax8(vcpuins.cimm));
		vapiPrint("This happens because of the special instruction.\n");
		vapiCallBackMachineStop();
		break;
	case 0x01:
	case 0xfe: /* RESET */
		vapiPrint("\nNXVM CPU RESET at CS:%04X IP:%08X INS:QDX IMM:%02X\n",
			vcpu.cs.selector,vcpu.eip, GetMax8(vcpuins.cimm));
		vapiPrint("This happens because of the special instruction.\n");
		vapiCallBackMachineReset();
		break;
	case 0x02: /* PRINT_REG16 */
		vapiPrint("\nNXVM CPU PRINT16 at CS:%04X IP:%08X INS:QDX IMM:%02X\n",
			vcpu.cs.selector,vcpu.eip, GetMax8(vcpuins.cimm));
		vapiCallBackDebugPrintRegs(16);
		break;
	case 0x03: /* PRINT_REG32 */
		vapiPrint("\nNXVM CPU PRINT32 at CS:%04X IP:%08X INS:QDX IMM:%02X\n",
			vcpu.cs.selector,vcpu.eip, GetMax8(vcpuins.cimm));
		vapiCallBackDebugPrintRegs(32);
		break;
	case 0x04: /* PRINT_EAX */
		vapiPrint("%08X", vcpu.eax);
		break;
	default: /* QDINT */
		_bb("QDINT");
		qdbiosExecInt(d_nubit8(vcpuins.rimm));
		_chk(eflags = (t_nubit32)_m_read_logical(&vcpu.ss, vcpu.sp + 4, 4));
		MakeBit(eflags, VCPU_EFLAGS_ZF, _GetEFLAGS_ZF);
		MakeBit(eflags, VCPU_EFLAGS_CF, _GetEFLAGS_CF);
		_chk(_m_write_logical(&vcpu.ss, vcpu.sp + 4, eflags, 4));
		_be;break;
	}
	_ce;
}

void vcpuinsInit()
{
#if VCPUINS_TRACE == 1
	_Trace_Init();
#endif
	memset(&vcpuins, 0x00, sizeof(t_cpuins));
	vcpuins.table[0x00] = (t_faddrcc)ADD_RM8_R8;
	vcpuins.table[0x01] = (t_faddrcc)ADD_RM16_R16;
	vcpuins.table[0x02] = (t_faddrcc)ADD_R8_RM8;
	vcpuins.table[0x03] = (t_faddrcc)ADD_R16_RM16;
	vcpuins.table[0x04] = (t_faddrcc)ADD_AL_I8;
	vcpuins.table[0x05] = (t_faddrcc)ADD_AX_I16;
	vcpuins.table[0x06] = (t_faddrcc)PUSH_ES;
	vcpuins.table[0x07] = (t_faddrcc)POP_ES;
	vcpuins.table[0x08] = (t_faddrcc)OR_RM8_R8;
	vcpuins.table[0x09] = (t_faddrcc)OR_RM16_R16;
	vcpuins.table[0x0a] = (t_faddrcc)OR_R8_RM8;
	vcpuins.table[0x0b] = (t_faddrcc)OR_R16_RM16;
	vcpuins.table[0x0c] = (t_faddrcc)OR_AL_I8;
	vcpuins.table[0x0d] = (t_faddrcc)OR_AX_I16;
	vcpuins.table[0x0e] = (t_faddrcc)PUSH_CS;
	vcpuins.table[0x0f] = (t_faddrcc)INS_0F;//
	vcpuins.table[0x10] = (t_faddrcc)ADC_RM8_R8;
	vcpuins.table[0x11] = (t_faddrcc)ADC_RM16_R16;
	vcpuins.table[0x12] = (t_faddrcc)ADC_R8_RM8;
	vcpuins.table[0x13] = (t_faddrcc)ADC_R16_RM16;
	vcpuins.table[0x14] = (t_faddrcc)ADC_AL_I8;
	vcpuins.table[0x15] = (t_faddrcc)ADC_AX_I16;
	vcpuins.table[0x16] = (t_faddrcc)PUSH_SS;
	vcpuins.table[0x17] = (t_faddrcc)POP_SS;
	vcpuins.table[0x18] = (t_faddrcc)SBB_RM8_R8;
	vcpuins.table[0x19] = (t_faddrcc)SBB_RM16_R16;
	vcpuins.table[0x1a] = (t_faddrcc)SBB_R8_RM8;
	vcpuins.table[0x1b] = (t_faddrcc)SBB_R16_RM16;
	vcpuins.table[0x1c] = (t_faddrcc)SBB_AL_I8;
	vcpuins.table[0x1d] = (t_faddrcc)SBB_AX_I16;
	vcpuins.table[0x1e] = (t_faddrcc)PUSH_DS;
	vcpuins.table[0x1f] = (t_faddrcc)POP_DS;
	vcpuins.table[0x20] = (t_faddrcc)AND_RM8_R8;
	vcpuins.table[0x21] = (t_faddrcc)AND_RM16_R16;
	vcpuins.table[0x22] = (t_faddrcc)AND_R8_RM8;
	vcpuins.table[0x23] = (t_faddrcc)AND_R16_RM16;
	vcpuins.table[0x24] = (t_faddrcc)AND_AL_I8;
	vcpuins.table[0x25] = (t_faddrcc)AND_AX_I16;
	vcpuins.table[0x26] = (t_faddrcc)PREFIX_ES;
	vcpuins.table[0x27] = (t_faddrcc)DAA;
	vcpuins.table[0x28] = (t_faddrcc)SUB_RM8_R8;
	vcpuins.table[0x29] = (t_faddrcc)SUB_RM16_R16;
	vcpuins.table[0x2a] = (t_faddrcc)SUB_R8_RM8;
	vcpuins.table[0x2b] = (t_faddrcc)SUB_R16_RM16;
	vcpuins.table[0x2c] = (t_faddrcc)SUB_AL_I8;
	vcpuins.table[0x2d] = (t_faddrcc)SUB_AX_I16;
	vcpuins.table[0x2e] = (t_faddrcc)PREFIX_CS;
	vcpuins.table[0x2f] = (t_faddrcc)DAS;
	vcpuins.table[0x30] = (t_faddrcc)XOR_RM8_R8;
	vcpuins.table[0x31] = (t_faddrcc)XOR_RM16_R16;
	vcpuins.table[0x32] = (t_faddrcc)XOR_R8_RM8;
	vcpuins.table[0x33] = (t_faddrcc)XOR_R16_RM16;
	vcpuins.table[0x34] = (t_faddrcc)XOR_AL_I8;
	vcpuins.table[0x35] = (t_faddrcc)XOR_AX_I16;
	vcpuins.table[0x36] = (t_faddrcc)PREFIX_SS;
	vcpuins.table[0x37] = (t_faddrcc)AAA;
	vcpuins.table[0x38] = (t_faddrcc)CMP_RM8_R8;
	vcpuins.table[0x39] = (t_faddrcc)CMP_RM16_R16;
	vcpuins.table[0x3a] = (t_faddrcc)CMP_R8_RM8;
	vcpuins.table[0x3b] = (t_faddrcc)CMP_R16_RM16;
	vcpuins.table[0x3c] = (t_faddrcc)CMP_AL_I8;
	vcpuins.table[0x3d] = (t_faddrcc)CMP_AX_I16;
	vcpuins.table[0x3e] = (t_faddrcc)PREFIX_DS;
	vcpuins.table[0x3f] = (t_faddrcc)AAS;
	vcpuins.table[0x40] = (t_faddrcc)INC_AX;
	vcpuins.table[0x41] = (t_faddrcc)INC_CX;
	vcpuins.table[0x42] = (t_faddrcc)INC_DX;
	vcpuins.table[0x43] = (t_faddrcc)INC_BX;
	vcpuins.table[0x44] = (t_faddrcc)INC_SP;
	vcpuins.table[0x45] = (t_faddrcc)INC_BP;
	vcpuins.table[0x46] = (t_faddrcc)INC_SI;
	vcpuins.table[0x47] = (t_faddrcc)INC_DI;
	vcpuins.table[0x48] = (t_faddrcc)DEC_AX;
	vcpuins.table[0x49] = (t_faddrcc)DEC_CX;
	vcpuins.table[0x4a] = (t_faddrcc)DEC_DX;
	vcpuins.table[0x4b] = (t_faddrcc)DEC_BX;
	vcpuins.table[0x4c] = (t_faddrcc)DEC_SP;
	vcpuins.table[0x4d] = (t_faddrcc)DEC_BP;
	vcpuins.table[0x4e] = (t_faddrcc)DEC_SI;
	vcpuins.table[0x4f] = (t_faddrcc)DEC_DI;
	vcpuins.table[0x50] = (t_faddrcc)PUSH_AX;
	vcpuins.table[0x51] = (t_faddrcc)PUSH_CX;
	vcpuins.table[0x52] = (t_faddrcc)PUSH_DX;
	vcpuins.table[0x53] = (t_faddrcc)PUSH_BX;
	vcpuins.table[0x54] = (t_faddrcc)PUSH_SP;
	vcpuins.table[0x55] = (t_faddrcc)PUSH_BP;
	vcpuins.table[0x56] = (t_faddrcc)PUSH_SI;
	vcpuins.table[0x57] = (t_faddrcc)PUSH_DI;
	vcpuins.table[0x58] = (t_faddrcc)POP_AX;
	vcpuins.table[0x59] = (t_faddrcc)POP_CX;
	vcpuins.table[0x5a] = (t_faddrcc)POP_DX;
	vcpuins.table[0x5b] = (t_faddrcc)POP_BX;
	vcpuins.table[0x5c] = (t_faddrcc)POP_SP;
	vcpuins.table[0x5d] = (t_faddrcc)POP_BP;
	vcpuins.table[0x5e] = (t_faddrcc)POP_SI;
	vcpuins.table[0x5f] = (t_faddrcc)POP_DI;
	vcpuins.table[0x60] = (t_faddrcc)PUSHA;//
	vcpuins.table[0x61] = (t_faddrcc)POPA;//
	vcpuins.table[0x62] = (t_faddrcc)BOUND_R16_M16_16;//
	vcpuins.table[0x63] = (t_faddrcc)ARPL_RM16_R16;//
	vcpuins.table[0x64] = (t_faddrcc)PREFIX_FS;//
	vcpuins.table[0x65] = (t_faddrcc)PREFIX_GS;//
	vcpuins.table[0x66] = (t_faddrcc)PREFIX_OprSize;//
	vcpuins.table[0x67] = (t_faddrcc)PREFIX_AddrSize;//
	vcpuins.table[0x68] = (t_faddrcc)PUSH_I16;//
	vcpuins.table[0x69] = (t_faddrcc)IMUL_R16_RM16_I16;//
	vcpuins.table[0x6a] = (t_faddrcc)PUSH_I8;//
	vcpuins.table[0x6b] = (t_faddrcc)IMUL_R16_RM16_I8;//
	vcpuins.table[0x6c] = (t_faddrcc)INSB;//
	vcpuins.table[0x6d] = (t_faddrcc)INSW;//
	vcpuins.table[0x6e] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0x6f] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0x70] = (t_faddrcc)JO_REL8;
	vcpuins.table[0x71] = (t_faddrcc)JNO_REL8;
	vcpuins.table[0x72] = (t_faddrcc)JC_REL8;
	vcpuins.table[0x73] = (t_faddrcc)JNC_REL8;
	vcpuins.table[0x74] = (t_faddrcc)JZ_REL8;
	vcpuins.table[0x75] = (t_faddrcc)JNZ_REL8;
	vcpuins.table[0x76] = (t_faddrcc)JNA_REL8;
	vcpuins.table[0x77] = (t_faddrcc)JA_REL8;
	vcpuins.table[0x78] = (t_faddrcc)JS_REL8;
	vcpuins.table[0x79] = (t_faddrcc)JNS_REL8;
	vcpuins.table[0x7a] = (t_faddrcc)JP_REL8;
	vcpuins.table[0x7b] = (t_faddrcc)JNP_REL8;
	vcpuins.table[0x7c] = (t_faddrcc)JL_REL8;
	vcpuins.table[0x7d] = (t_faddrcc)JNL_REL8;
	vcpuins.table[0x7e] = (t_faddrcc)JNG_REL8;
	vcpuins.table[0x7f] = (t_faddrcc)JG_REL8;
	vcpuins.table[0x80] = (t_faddrcc)INS_80;
	vcpuins.table[0x81] = (t_faddrcc)INS_81;
	vcpuins.table[0x82] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0x83] = (t_faddrcc)INS_83;
	vcpuins.table[0x84] = (t_faddrcc)TEST_RM8_R8;
	vcpuins.table[0x85] = (t_faddrcc)TEST_RM16_R16;
	vcpuins.table[0x86] = (t_faddrcc)XCHG_RM8_R8;
	vcpuins.table[0x87] = (t_faddrcc)XCHG_RM16_R16;
	vcpuins.table[0x88] = (t_faddrcc)MOV_RM8_R8;
	vcpuins.table[0x89] = (t_faddrcc)MOV_RM16_R16;
	vcpuins.table[0x8a] = (t_faddrcc)MOV_R8_RM8;
	vcpuins.table[0x8b] = (t_faddrcc)MOV_R16_RM16;
	vcpuins.table[0x8c] = (t_faddrcc)MOV_RM16_SREG;
	vcpuins.table[0x8d] = (t_faddrcc)LEA_R16_M16;
	vcpuins.table[0x8e] = (t_faddrcc)MOV_SREG_RM16;
	vcpuins.table[0x8f] = (t_faddrcc)INS_8F;
	vcpuins.table[0x90] = (t_faddrcc)NOP;
	vcpuins.table[0x91] = (t_faddrcc)XCHG_CX_AX;
	vcpuins.table[0x92] = (t_faddrcc)XCHG_DX_AX;
	vcpuins.table[0x93] = (t_faddrcc)XCHG_BX_AX;
	vcpuins.table[0x94] = (t_faddrcc)XCHG_SP_AX;
	vcpuins.table[0x95] = (t_faddrcc)XCHG_BP_AX;
	vcpuins.table[0x96] = (t_faddrcc)XCHG_SI_AX;
	vcpuins.table[0x97] = (t_faddrcc)XCHG_DI_AX;
	vcpuins.table[0x98] = (t_faddrcc)CBW;
	vcpuins.table[0x99] = (t_faddrcc)CWD;
	vcpuins.table[0x9a] = (t_faddrcc)CALL_PTR16_16;
	vcpuins.table[0x9b] = (t_faddrcc)WAIT;
	vcpuins.table[0x9c] = (t_faddrcc)PUSHF;
	vcpuins.table[0x9d] = (t_faddrcc)POPF;
	vcpuins.table[0x9e] = (t_faddrcc)SAHF;
	vcpuins.table[0x9f] = (t_faddrcc)LAHF;
	vcpuins.table[0xa0] = (t_faddrcc)MOV_AL_MOFFS8;
	vcpuins.table[0xa1] = (t_faddrcc)MOV_AX_MOFFS16;
	vcpuins.table[0xa2] = (t_faddrcc)MOV_MOFFS8_AL;
	vcpuins.table[0xa3] = (t_faddrcc)MOV_MOFFS16_AX;
	vcpuins.table[0xa4] = (t_faddrcc)MOVSB;
	vcpuins.table[0xa5] = (t_faddrcc)MOVSW;
	vcpuins.table[0xa6] = (t_faddrcc)CMPSB;
	vcpuins.table[0xa7] = (t_faddrcc)CMPSW;
	vcpuins.table[0xa8] = (t_faddrcc)TEST_AL_I8;
	vcpuins.table[0xa9] = (t_faddrcc)TEST_AX_I16;
	vcpuins.table[0xaa] = (t_faddrcc)STOSB;
	vcpuins.table[0xab] = (t_faddrcc)STOSW;
	vcpuins.table[0xac] = (t_faddrcc)LODSB;
	vcpuins.table[0xad] = (t_faddrcc)LODSW;
	vcpuins.table[0xae] = (t_faddrcc)SCASB;
	vcpuins.table[0xaf] = (t_faddrcc)SCASW;
	vcpuins.table[0xb0] = (t_faddrcc)MOV_AL_I8;
	vcpuins.table[0xb1] = (t_faddrcc)MOV_CL_I8;
	vcpuins.table[0xb2] = (t_faddrcc)MOV_DL_I8;
	vcpuins.table[0xb3] = (t_faddrcc)MOV_BL_I8;
	vcpuins.table[0xb4] = (t_faddrcc)MOV_AH_I8;
	vcpuins.table[0xb5] = (t_faddrcc)MOV_CH_I8;
	vcpuins.table[0xb6] = (t_faddrcc)MOV_DH_I8;
	vcpuins.table[0xb7] = (t_faddrcc)MOV_BH_I8;
	vcpuins.table[0xb8] = (t_faddrcc)MOV_AX_I16;
	vcpuins.table[0xb9] = (t_faddrcc)MOV_CX_I16;
	vcpuins.table[0xba] = (t_faddrcc)MOV_DX_I16;
	vcpuins.table[0xbb] = (t_faddrcc)MOV_BX_I16;
	vcpuins.table[0xbc] = (t_faddrcc)MOV_SP_I16;
	vcpuins.table[0xbd] = (t_faddrcc)MOV_BP_I16;
	vcpuins.table[0xbe] = (t_faddrcc)MOV_SI_I16;
	vcpuins.table[0xbf] = (t_faddrcc)MOV_DI_I16;
	vcpuins.table[0xc0] = (t_faddrcc)INS_C0;//
	vcpuins.table[0xc1] = (t_faddrcc)INS_C1;//
	vcpuins.table[0xc2] = (t_faddrcc)RET_I16;
	vcpuins.table[0xc3] = (t_faddrcc)RET;
	vcpuins.table[0xc4] = (t_faddrcc)LES_R16_M16_16;
	vcpuins.table[0xc5] = (t_faddrcc)LDS_R16_M16_16;
	vcpuins.table[0xc6] = (t_faddrcc)INS_C6;
	vcpuins.table[0xc7] = (t_faddrcc)INS_C7;
	vcpuins.table[0xc8] = (t_faddrcc)ENTER;//
	vcpuins.table[0xc9] = (t_faddrcc)LEAVE;//
	vcpuins.table[0xca] = (t_faddrcc)RETF_I16;
	vcpuins.table[0xcb] = (t_faddrcc)RETF;
	vcpuins.table[0xcc] = (t_faddrcc)INT3;
	vcpuins.table[0xcd] = (t_faddrcc)INT_I8;
	vcpuins.table[0xce] = (t_faddrcc)INTO;
	vcpuins.table[0xcf] = (t_faddrcc)IRET;
	vcpuins.table[0xd0] = (t_faddrcc)INS_D0;
	vcpuins.table[0xd1] = (t_faddrcc)INS_D1;
	vcpuins.table[0xd2] = (t_faddrcc)INS_D2;
	vcpuins.table[0xd3] = (t_faddrcc)INS_D3;
	vcpuins.table[0xd4] = (t_faddrcc)AAM;
	vcpuins.table[0xd5] = (t_faddrcc)AAD;
	vcpuins.table[0xd6] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0xd7] = (t_faddrcc)XLAT;
	vcpuins.table[0xd8] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0xd9] = (t_faddrcc)UndefinedOpcode;
	//vcpuins.table[0xd9] = (t_faddrcc)INS_D9;
	vcpuins.table[0xda] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0xdb] = (t_faddrcc)UndefinedOpcode;
	//vcpuins.table[0xdb] = (t_faddrcc)INS_DB;
	vcpuins.table[0xdc] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0xdd] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0xde] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0xdf] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0xe0] = (t_faddrcc)LOOPNZ;
	vcpuins.table[0xe1] = (t_faddrcc)LOOPZ;
	vcpuins.table[0xe2] = (t_faddrcc)LOOP;
	vcpuins.table[0xe3] = (t_faddrcc)JCXZ_REL8;
	vcpuins.table[0xe4] = (t_faddrcc)IN_AL_I8;
	vcpuins.table[0xe5] = (t_faddrcc)IN_AX_I8;
	vcpuins.table[0xe6] = (t_faddrcc)OUT_I8_AL;
	vcpuins.table[0xe7] = (t_faddrcc)OUT_I8_AX;
	vcpuins.table[0xe8] = (t_faddrcc)CALL_REL16;
	vcpuins.table[0xe9] = (t_faddrcc)JMP_REL16;
	vcpuins.table[0xea] = (t_faddrcc)JMP_PTR16_16;
	vcpuins.table[0xeb] = (t_faddrcc)JMP_REL8;
	vcpuins.table[0xec] = (t_faddrcc)IN_AL_DX;
	vcpuins.table[0xed] = (t_faddrcc)IN_AX_DX;
	vcpuins.table[0xee] = (t_faddrcc)OUT_DX_AL;
	vcpuins.table[0xef] = (t_faddrcc)OUT_DX_AX;
	vcpuins.table[0xf0] = (t_faddrcc)PREFIX_LOCK;
	vcpuins.table[0xf1] = (t_faddrcc)QDX;
	vcpuins.table[0xf2] = (t_faddrcc)PREFIX_REPNZ;
	vcpuins.table[0xf3] = (t_faddrcc)PREFIX_REPZ;
	vcpuins.table[0xf4] = (t_faddrcc)HLT;
	vcpuins.table[0xf5] = (t_faddrcc)CMC;
	vcpuins.table[0xf6] = (t_faddrcc)INS_F6;
	vcpuins.table[0xf7] = (t_faddrcc)INS_F7;
	vcpuins.table[0xf8] = (t_faddrcc)CLC;
	vcpuins.table[0xf9] = (t_faddrcc)STC;
	vcpuins.table[0xfa] = (t_faddrcc)CLI;
	vcpuins.table[0xfb] = (t_faddrcc)STI;
	vcpuins.table[0xfc] = (t_faddrcc)CLD;
	vcpuins.table[0xfd] = (t_faddrcc)STD;
	vcpuins.table[0xfe] = (t_faddrcc)INS_FE;
	vcpuins.table[0xff] = (t_faddrcc)INS_FF;
	vcpuins.table_0f[0x00] = (t_faddrcc)INS_0F_00;
	vcpuins.table_0f[0x01] = (t_faddrcc)INS_0F_01;
	vcpuins.table_0f[0x02] = (t_faddrcc)LAR_R16_RM16;
	vcpuins.table_0f[0x03] = (t_faddrcc)LSL_R16_RM16;
	vcpuins.table_0f[0x04] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x05] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x06] = (t_faddrcc)CLTS;
	vcpuins.table_0f[0x07] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x08] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x09] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x0a] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x0b] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x0c] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x0d] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x0e] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x0f] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x10] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x11] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x12] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x13] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x14] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x15] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x16] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x17] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x18] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x19] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x1a] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x1b] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x1c] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x1d] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x1e] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x1f] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x20] = (t_faddrcc)MOV_R32_CR;
	vcpuins.table_0f[0x21] = (t_faddrcc)MOV_R32_DR;
	vcpuins.table_0f[0x22] = (t_faddrcc)MOV_CR_R32;
	vcpuins.table_0f[0x23] = (t_faddrcc)MOV_DR_R32;
	vcpuins.table_0f[0x24] = (t_faddrcc)MOV_R32_TR;
	vcpuins.table_0f[0x25] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x26] = (t_faddrcc)MOV_TR_R32;
	vcpuins.table_0f[0x27] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x28] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x29] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x2a] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x2b] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x2c] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x2d] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x2e] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x2f] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x30] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x31] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x32] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x33] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x34] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x35] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x36] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x37] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x38] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x39] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x3a] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x3b] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x3c] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x3d] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x3e] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x3f] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x40] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x41] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x42] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x43] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x44] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x45] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x46] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x47] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x48] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x49] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x4a] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x4b] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x4c] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x4d] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x4e] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x4f] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x50] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x51] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x52] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x53] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x54] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x55] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x56] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x57] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x58] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x59] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x5a] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x5b] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x5c] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x5d] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x5e] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x5f] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x60] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x61] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x62] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x63] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x64] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x65] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x66] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x67] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x68] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x69] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x6a] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x6b] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x6c] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x6d] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x6e] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x6f] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x70] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x71] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x72] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x73] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x74] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x75] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x76] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x77] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x78] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x79] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x7a] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x7b] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x7c] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x7d] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x7e] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x7f] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x80] = (t_faddrcc)JO_REL16;
	vcpuins.table_0f[0x81] = (t_faddrcc)JNO_REL16;
	vcpuins.table_0f[0x82] = (t_faddrcc)JC_REL16;
	vcpuins.table_0f[0x83] = (t_faddrcc)JNC_REL16;
	vcpuins.table_0f[0x84] = (t_faddrcc)JZ_REL16;
	vcpuins.table_0f[0x85] = (t_faddrcc)JNZ_REL16;
	vcpuins.table_0f[0x86] = (t_faddrcc)JNA_REL16;
	vcpuins.table_0f[0x87] = (t_faddrcc)JA_REL16;
	vcpuins.table_0f[0x88] = (t_faddrcc)JS_REL16;
	vcpuins.table_0f[0x89] = (t_faddrcc)JNS_REL16;
	vcpuins.table_0f[0x8a] = (t_faddrcc)JP_REL16;
	vcpuins.table_0f[0x8b] = (t_faddrcc)JNP_REL16;
	vcpuins.table_0f[0x8c] = (t_faddrcc)JL_REL16;
	vcpuins.table_0f[0x8d] = (t_faddrcc)JNL_REL16;
	vcpuins.table_0f[0x8e] = (t_faddrcc)JNG_REL16;
	vcpuins.table_0f[0x8f] = (t_faddrcc)JG_REL16;
	vcpuins.table_0f[0x90] = (t_faddrcc)SETO_RM8;
	vcpuins.table_0f[0x91] = (t_faddrcc)SETNO_RM8;
	vcpuins.table_0f[0x92] = (t_faddrcc)SETC_RM8;
	vcpuins.table_0f[0x93] = (t_faddrcc)SETNC_RM8;
	vcpuins.table_0f[0x94] = (t_faddrcc)SETZ_RM8;
	vcpuins.table_0f[0x95] = (t_faddrcc)SETNZ_RM8;
	vcpuins.table_0f[0x96] = (t_faddrcc)SETNA_RM8;
	vcpuins.table_0f[0x97] = (t_faddrcc)SETA_RM8;
	vcpuins.table_0f[0x98] = (t_faddrcc)SETS_RM8;
	vcpuins.table_0f[0x99] = (t_faddrcc)SETNS_RM8;
	vcpuins.table_0f[0x9a] = (t_faddrcc)SETP_RM8;
	vcpuins.table_0f[0x9b] = (t_faddrcc)SETNP_RM8;
	vcpuins.table_0f[0x9c] = (t_faddrcc)SETL_RM8;
	vcpuins.table_0f[0x9d] = (t_faddrcc)SETNL_RM8;
	vcpuins.table_0f[0x9e] = (t_faddrcc)SETNG_RM8;
	vcpuins.table_0f[0x9f] = (t_faddrcc)SETG_RM8;
	vcpuins.table_0f[0xa0] = (t_faddrcc)PUSH_FS;
	vcpuins.table_0f[0xa1] = (t_faddrcc)POP_FS;
	vcpuins.table_0f[0xa2] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xa3] = (t_faddrcc)BT_RM16_R16;
	vcpuins.table_0f[0xa4] = (t_faddrcc)SHLD_RM16_R16_I8;
	vcpuins.table_0f[0xa5] = (t_faddrcc)SHLD_RM16_R16_CL;
	vcpuins.table_0f[0xa6] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xa7] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xa8] = (t_faddrcc)PUSH_GS;
	vcpuins.table_0f[0xa9] = (t_faddrcc)POP_GS;
	vcpuins.table_0f[0xaa] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xab] = (t_faddrcc)BTS_RM16_R16;
	vcpuins.table_0f[0xac] = (t_faddrcc)SHRD_RM16_R16_I8;
	vcpuins.table_0f[0xad] = (t_faddrcc)SHRD_RM16_R16_CL;
	vcpuins.table_0f[0xae] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xaf] = (t_faddrcc)IMUL_R16_RM16;
	vcpuins.table_0f[0xb0] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb1] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb2] = (t_faddrcc)LSS_R16_M16_16;
	vcpuins.table_0f[0xb3] = (t_faddrcc)BTR_RM16_R16;
	vcpuins.table_0f[0xb4] = (t_faddrcc)LFS_R16_M16_16;
	vcpuins.table_0f[0xb5] = (t_faddrcc)LGS_R16_M16_16;
	vcpuins.table_0f[0xb6] = (t_faddrcc)MOVZX_R16_RM8;
	vcpuins.table_0f[0xb7] = (t_faddrcc)MOVZX_R32_RM16;
	vcpuins.table_0f[0xb8] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb9] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xba] = (t_faddrcc)INS_0F_BA;
	vcpuins.table_0f[0xbb] = (t_faddrcc)BTC_RM16_R16;
	vcpuins.table_0f[0xbc] = (t_faddrcc)BSF_R16_RM16;
	vcpuins.table_0f[0xbd] = (t_faddrcc)BSR_R16_RM16;
	vcpuins.table_0f[0xbe] = (t_faddrcc)MOVSX_R16_RM8;
	vcpuins.table_0f[0xbf] = (t_faddrcc)MOVSX_R32_RM16;
	vcpuins.table_0f[0xc0] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xc1] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xc2] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xc3] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xc4] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xc5] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xc6] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xc7] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xc8] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xc9] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xca] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xcb] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xcc] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xcd] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xce] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xcf] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xd0] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xd1] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xd2] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xd3] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xd4] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xd5] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xd6] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xd7] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xd8] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xd9] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xda] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xdb] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xdc] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xdd] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xde] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xdf] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xe0] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xe1] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xe2] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xe3] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xe4] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xe5] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xe6] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xe7] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xe8] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xe9] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xea] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xeb] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xec] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xed] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xee] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xef] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xf0] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xf1] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xf2] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xf3] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xf4] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xf5] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xf6] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xf7] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xf8] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xf9] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xfa] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xfb] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xfc] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xfd] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xfe] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xff] = (t_faddrcc)UndefinedOpcode;
}
void vcpuinsReset() {}
void vcpuinsRefresh()
{
	if (!vcpu.flaghalt) {
		ExecIns();
	} else {
		vapiSleep(1);
	}
	ExecInt();
}
void vcpuinsFinal()
{
#if VCPUINS_TRACE == 1
	_Trace_Release(0);
#endif
}
