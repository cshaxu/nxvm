/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include "vapi.h"
#include "vport.h"
#include "vram.h"
#include "vcpu.h"
#include "vpic.h"
#include "bios/qdbios.h"
#include "vcpuins.h"

#include "debug/aasm.h"
#include "debug/dasm.h"

#define _a_ _Arith_
#define _c_ _Control_
#define _s_ _Stack_
#define _d_ _Decode_
#define _m_ _Memory_


#define VCPUINS_TRACE 1

#if VCPUINS_TRACE == 1
#define _cb(s) _Trace_Call_Begin(s)
#define _ce    _Trace_Call_End()
#define _bb(s) _Trace_Block_Begin(s)
#define _be    _Trace_Block_End()
#define _chb(n) if (1) {(n);if (vcpuins.except) {_Trace_Release(1);break;}} else
#define _chk(n) if (1) {(n);if (vcpuins.except) {_Trace_Release(1);return;}} else
#define _chr(n) if (1) {(n);if (vcpuins.except) {_Trace_Release(1);return 0;}} else
#else
#define _cb(s)
#define _ce
#define _be
#define _bb(s)
#define _chb(n) if (1) {(n);if (vcpuins.except) break;} else
#define _chk(n) if (1) {(n);if (vcpuins.except) return;} else
#define _chr(n) if (1) {(n);if (vcpuins.except) return 0;} else
#endif

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
	if (trace.cid == 0xff) return;
	trace.callstack[trace.cid].blockstack[0] = s;
	trace.callstack[trace.cid].bid = 1;
	trace.callstack[trace.cid].cid = trace.cid;
	trace.cid++;
}
static void _Trace_Call_End()
{
	if (!trace.cid) return;
	trace.cid--;
	if (trace.callstack[trace.cid].bid != 1 || trace.callstack[trace.cid].cid != trace.cid) {
		vapiPrint("\nunbalanced call stack:\n");
		trace.cid++;
		vcpuins.except |= 0xffffffff;
	}
}
static void _Trace_Block_Begin(t_string s)
{
	if (trace.callstack[trace.cid - 1].bid < 0xff)
		trace.callstack[trace.cid - 1].blockstack[trace.callstack[trace.cid - 1].bid++] = s;
}
static void _Trace_Block_End()
{
	if (trace.callstack[trace.cid - 1].bid)
		trace.callstack[trace.cid - 1].bid--;
}

t_cpuins vcpuins;

#define _GetOperandSize ((vcpu.cs.db ^ vcpuins.prefix_oprsize) ? 4 : 2)
#define _GetAddressSize ((vcpu.cs.db ^ vcpuins.prefix_addrsize) ? 4 : 2)


static void CaseError(t_string str)
{
	vapiPrint("The NXVM CPU has encountered an internal case error: \n");
	SetBit(vcpuins.except, VCPUINS_EXCEPT_CE);
	vapiCallBackMachineStop();
}

#define _SetExcept_PF(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_PF), vcpuins.excode = (n), vapiPrint("#PF(%x) - page fault\n", (n)))
#define _SetExcept_GP(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_GP), vcpuins.excode = (n), vapiPrint("#GP(%x) - general protect\n", (n)))
#define _SetExcept_SS(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_SS), vcpuins.excode = (n), vapiPrint("#SS(%x) - stack segment\n", (n)))
#define _SetExcept_UD(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_UD), vcpuins.excode = (n), vapiPrint("#UD(%x) - undefined\n", (n)))
#define _SetExcept_NP(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_NP), vcpuins.excode = (n), vapiPrint("#NP(%x) - not present\n", (n)))
#define _SetExcept_BR(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_BR), vcpuins.excode = (n), vapiPrint("#BR(%x) - boundary\n", (n)))
#define _SetExcept_TS(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_TS), vcpuins.excode = (n), vapiPrint("#TS(%x) - task state\n", (n)))
#define _SetExcept_CE(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_CE), vcpuins.excode = (n), vapiPrint("#CE(%x) - internal case error\n", (n)), vapiCallBackMachineStop())
#define _impossible_     _chk(_SetExcept_CE(1))

/* kernel memory accessing */
/* get physical from linear by paging */
static t_nubit32 _kma_addr_physical_linear(t_nubit32 linear, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force)
{
	t_nubit32 physical = 0x00000000;
	_cb("_kma_addr_physical_linear");
	_chr(physical = linear);
	_ce;
	return physical;
}
/* get linear from logical by segmentation */
static t_nubit32 _kma_addr_linear_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force)
{
	t_nubit32 linear = 0x00000000;
	_cb("_kma_addr_linear_logical");
	_chr(linear = ((rsreg->selector << 4) + GetMax16(offset)));
	_ce;
	return linear;
}
/* get physical from logical by combination of paging and segmentation */
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
/* read content from physical */
static t_nubit64 _kma_read_physical(t_nubit32 physical, t_nubit8 byte)
{
	t_vaddrcc ref = 0;
	t_nubit64 result = 0x0000000000000000;
	_cb("_kma_read_physical");
	_chr(ref = _kma_ref_physical(physical));
	switch (byte) {
	case 1: result = d_nubit8(ref);break;
	case 2: result = d_nubit16(ref);break;
	case 4: result = d_nubit32(ref);break;
	case 6: result = GetMax48(d_nubit64(ref));break;
	case 8: result = d_nubit64(ref);break;
	default: _chr(_SetExcept_CE(0));break;}
	_ce;
	return result;
}
/* read content from linear */
static t_nubit64 _kma_read_linear(t_nubit32 linear, t_nubit8 byte, t_nubit8 vpl, t_bool force)
{
	t_nubit64 result = 0x0000000000000000;
	t_nubit32 physical = 0x00000000;
	_cb("_kma_read_linear");
	_chr(physical = _kma_addr_physical_linear(linear, byte, 0, vpl, force));
	_chr(result = _kma_read_physical(physical, byte));
	_ce;
	return result;
}
/* read content from logical */
static t_nubit64 _kma_read_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_nubit8 vpl, t_bool force)
{
	t_nubit64 result = 0x0000000000000000;
	t_nubit32 physical = 0x00000000;
	_cb("_kma_read_logical");
	_chr(physical = _kma_addr_physical_logical(rsreg, offset, byte, 0, vpl, force));
	_chr(result = _kma_read_physical(physical, byte));
	_ce;
	return result;
}
/* write content to physical */
static void _kma_write_physical(t_nubit32 physical, t_nubit64 data, t_nubit8 byte)
{
	t_vaddrcc ref = 0;
	_cb("_kma_write_physical");
	_chk(ref = _kma_ref_physical(physical));
	switch (byte) {
	case 1: d_nubit8(ref) = GetMax8(data);break;
	case 2: d_nubit16(ref) = GetMax16(data);break;
	case 4: d_nubit32(ref) = GetMax32(data);break;
	case 6:
		d_nubit32(ref) = GetMax32(data);
		d_nubit16(ref + 4) = GetMax16(data >> 32);
		break;
	case 8: d_nubit64(ref) = GetMax64(data);break;
	default: _chk(_SetExcept_CE(0));break;}
	_ce;
}
/* write content to linear */
static void _kma_write_linear(t_nubit32 linear, t_nubit64 data, t_nubit8 byte, t_nubit8 vpl, t_bool force)
{
	t_nubit32 physical = 0x00000000;
	_cb("_kma_write_linear");
	_chk(physical = _kma_addr_physical_linear(linear, byte, 1, vpl, force));
	_chk(_kma_write_physical(physical, data, byte));
	_ce;
}
/* write content to logical */
static void _kma_write_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit64 data, t_nubit8 byte, t_nubit8 vpl, t_bool force)
{
	t_nubit32 physical = 0x00000000;
	_cb("_kma_write_logical");
	_chk(physical = _kma_addr_physical_logical(rsreg, offset, byte, 1, vpl, force));
	_chk(_kma_write_physical(physical, data, byte));
	_ce;
}

/* regular memory accessing */
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
static t_nubit64 _m_read_physical(t_nubit32 physical, t_nubit8 byte)
{
	t_nubit64 result = 0x0000000000000000;
	_cb("_m_read_physical");
	_chr(_kma_read_physical(physical, byte));
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
static void _m_write_physical(t_nubit32 physical, t_nubit64 data, t_nubit8 byte)
{
	_cb("_m_write_physical");
	_chk(_kma_write_physical(physical, data, byte));
	_ce;
}
static void _m_write_linear(t_nubit32 linear, t_nubit32 offset, t_nubit64 data, t_nubit8 byte)
{
	_cb("_m_write_linear");
	_chk(_kma_write_linear(linear, data, byte, _GetCPL, 0));
	_ce;
}
static void _m_write_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit64 data, t_nubit8 byte)
{
	_cb("_m_write_logical");
	_chk(_kma_write_logical(rsreg, offset, data, byte, _GetCPL, 0));
	_ce;
}
static t_nubit64 _m_read_cs(t_nubit32 offset, t_nubit8 byte)
{
	t_nubit64 result = 0x0000000000000000;
	_cb("_m_read_cs");
	_chr(result = _kma_read_logical(&vcpu.cs, offset, byte, 0x00, 0x01));
	_ce;
	return result;
}

/* decode unit */
#define _GetModRM_MOD(modrm) (((modrm) & 0xc0) >> 6)
#define _GetModRM_REG(modrm) (((modrm) & 0x38) >> 3)
#define _GetModRM_RM(modrm)  (((modrm) & 0x07) >> 0)
#define _GetSIB_SS(sib)      (((sib) & 0xc0) >> 6)
#define _GetSIB_Index(sib)   (((sib) & 0x38) >> 3)
#define _GetSIB_Base(sib)    (((sib) & 0x07) >> 0)
static void _d_skip(t_nubit8 byte)
{
	_cb("_d_skip");
	_chk(vcpu.eip += byte);
	_ce;
}
static t_nubit64 _d_code(t_nubit8 byte)
{
	t_nubit64 result = 0x0000000000000000;
	_cb("_d_code");
	_chr(result = _m_read_cs(vcpu.eip, byte));
	_chr(_d_skip(byte));
	_ce;
	return result;
}
static void _d_imm(t_nubit8 byte)
{
	_cb("_d_imm");
	_chk(vcpuins.limm = _kma_addr_linear_logical(&vcpu.cs, vcpu.eip, byte, 0x00, 0x00, 0x01));
	_chk(vcpuins.pimm = _kma_addr_physical_linear(vcpuins.limm, byte, 0x00, 0x00, 0x01));
	_chk(vcpuins.rimm = _kma_ref_physical(vcpuins.pimm));
	_chk(_d_skip(byte));
	_ce;
}
static void _d_modrm(t_nubit8 regbyte, t_nubit8 rmbyte, t_bool write)
{
	t_nsbit8 disp8 = 0x00;
	t_nubit16 disp16 = 0x0000;
	t_nubit32 disp32 = 0x00000000;
	t_nubit32 sibindex = 0x00000000;
	t_nubit8 modrm = 0x00, sib = 0x00;
	_cb("_d_modrm");
	_chk(modrm = (t_nubit8)_d_code(1));
	vcpuins.rrm = vcpuins.rr = (t_vaddrcc)NULL;
	vcpuins.prm = vcpuins.lrm = 0x00000000;
	vcpuins.flagmem = 0x01;
	switch (_GetAddressSize) {
	case 2: _bb("AddressSize(2)");
		switch (_GetModRM_MOD(modrm)) {
		case 0: _bb("ModRM_MOD(0)");
			switch (_GetModRM_RM(modrm)) {
			case 0: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, GetMax16(vcpu.bx+vcpu.si), rmbyte, write)); break;
			case 1: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, GetMax16(vcpu.bx+vcpu.di), rmbyte, write)); break;
			case 2: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverss, GetMax16(vcpu.bp+vcpu.si), rmbyte, write)); break;
			case 3: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverss, GetMax16(vcpu.bp+vcpu.di), rmbyte, write)); break;
			case 4: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, GetMax16(vcpu.si), rmbyte, write)); break;
			case 5: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, GetMax16(vcpu.di), rmbyte, write)); break;
			case 6: _bb("ModRM_RM(6)");
				_chk(disp16 = (t_nubit16)_d_code(2));
				_chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, GetMax16(disp16), rmbyte, write));
				_be;break;
			case 7: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, GetMax16(vcpu.bx), rmbyte, write)); break;
			default:_impossible_;break;}
			_chk(vcpuins.prm = _m_addr_physical_linear(vcpuins.lrm, rmbyte, write));
			_chk(vcpuins.rrm = _m_ref_physical(vcpuins.prm));
			_be;break;
		case 1: _bb("ModRM_MOD(1)");
			_chk(disp8 = (t_nsbit8)_d_code(1));
			switch (_GetModRM_RM(modrm)) {
			case 0: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, GetMax16(vcpu.bx+vcpu.si+disp8), rmbyte, write)); break;
			case 1: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, GetMax16(vcpu.bx+vcpu.di+disp8), rmbyte, write)); break;
			case 2: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverss, GetMax16(vcpu.bp+vcpu.si+disp8), rmbyte, write)); break;
			case 3: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverss, GetMax16(vcpu.bp+vcpu.di+disp8), rmbyte, write)); break;
			case 4: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, GetMax16(vcpu.si+disp8), rmbyte, write)); break;
			case 5: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, GetMax16(vcpu.di+disp8), rmbyte, write)); break;
			case 6: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverss, GetMax16(vcpu.bp+disp8), rmbyte, write)); break;
			case 7: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, GetMax16(vcpu.bx+disp8), rmbyte, write)); break;
			default:_impossible_;break;}
			_chk(vcpuins.prm = _m_addr_physical_linear(vcpuins.lrm, rmbyte, write));
			_chk(vcpuins.rrm = _m_ref_physical(vcpuins.prm));
			_be;break;
		case 2: _bb("ModRM_MOD(2)");
			_chk(disp16 = (t_nubit16)_d_code(2));
			switch (_GetModRM_RM(modrm)) {
			case 0: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, GetMax16(vcpu.bx+vcpu.si+disp16), rmbyte, write)); break;
			case 1: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, GetMax16(vcpu.bx+vcpu.di+disp16), rmbyte, write)); break;
			case 2: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverss, GetMax16(vcpu.bp+vcpu.si+disp16), rmbyte, write)); break;
			case 3: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverss, GetMax16(vcpu.bp+vcpu.di+disp16), rmbyte, write)); break;
			case 4: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, GetMax16(vcpu.si+disp16), rmbyte, write)); break;
			case 5: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, GetMax16(vcpu.di+disp16), rmbyte, write)); break;
			case 6: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverss, GetMax16(vcpu.bp+disp16), rmbyte, write)); break;
			case 7: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, GetMax16(vcpu.bx+disp16), rmbyte, write)); break;
			default:_impossible_;break;}
			_chk(vcpuins.prm = _m_addr_physical_linear(vcpuins.lrm, rmbyte, write));
			_chk(vcpuins.rrm = _m_ref_physical(vcpuins.prm));
			_be;break;
		case 3:
			break;
		default:
			_impossible_;
			break;
		}
		_be;break;
	case 4: _bb("AddressSize(4)");
		if (_GetModRM_MOD(modrm) != 3 && _GetModRM_RM(modrm) == 4) {
			_bb("ModRM_MOD(!3),ModRM_RM(4)");
			_chk(sib = (t_nubit8)_d_code(1));
			switch (_GetSIB_Index(sib)) {
			case 0: sibindex = vcpu.eax;   break;
			case 1: sibindex = vcpu.ecx;   break;
			case 2: sibindex = vcpu.edx;   break;
			case 3: sibindex = vcpu.ebx;   break;
			case 4: sibindex = 0x00000000; break;
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
			case 0: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.eax, rmbyte, write)); break;
			case 1: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.ecx, rmbyte, write)); break;
			case 2: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.edx, rmbyte, write)); break;
			case 3: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.ebx, rmbyte, write)); break;
			case 4:
				_bb("ModRM_RM(4)");
				switch (_GetSIB_Base(sib)) {
				case 0: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.eax + sibindex, rmbyte, write)); break;
				case 1: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.ecx + sibindex, rmbyte, write)); break;
				case 2: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.edx + sibindex, rmbyte, write)); break;
				case 3: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.ebx + sibindex, rmbyte, write)); break;
				case 4: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverss, vcpu.esp + sibindex, rmbyte, write)); break;
				case 5: _bb("SIB_Base(5)");
					_chk(disp32 = (t_nubit32)_d_code(4));
					_chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, disp32 + sibindex, rmbyte, write));
					_be;break;
				case 6: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.esi + sibindex, rmbyte, write)); break;
				case 7: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.edi + sibindex, rmbyte, write)); break;
				default:_impossible_;break;}
				_be;break;
			case 5: _bb("ModRM_RM(5)");
				_chk(disp32 = (t_nubit32)_d_code(4));
				_chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, disp32, rmbyte, write));
				_be;break;
			case 6: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.esi, rmbyte, write)); break;
			case 7: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.edi, rmbyte, write)); break;
			default:_impossible_;break;}
			_chk(vcpuins.prm = _m_addr_physical_linear(vcpuins.lrm, rmbyte, write));
			_chk(vcpuins.rrm = _m_ref_physical(vcpuins.prm));
			_be;break;
		case 1: _bb("ModRM_MOD(1)");
			_chk(disp8 = (t_nsbit8)_d_code(1));
			switch (_GetModRM_RM(modrm)) {
			case 0: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.eax + disp8, rmbyte, write)); break;
			case 1: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.ecx + disp8, rmbyte, write)); break;
			case 2: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.edx + disp8, rmbyte, write)); break;
			case 3: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.ebx + disp8, rmbyte, write)); break;
			case 4: _bb("ModRM_RM(4)");
				switch (_GetSIB_Base(sib)) {
				case 0: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.eax + sibindex + disp8, rmbyte, write)); break;
				case 1: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.ecx + sibindex + disp8, rmbyte, write)); break;
				case 2: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.edx + sibindex + disp8, rmbyte, write)); break;
				case 3: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.ebx + sibindex + disp8, rmbyte, write)); break;
				case 4: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverss, vcpu.esp + sibindex + disp8, rmbyte, write)); break;
				case 5: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverss, vcpu.ebp + sibindex + disp8, rmbyte, write)); break;
				case 6: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.esi + sibindex + disp8, rmbyte, write)); break;
				case 7: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.edi + sibindex + disp8, rmbyte, write)); break;
				default:_impossible_;break;}
				_be;break;
			case 5: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverss, vcpu.ebp + disp8, rmbyte, write)); break;
			case 6: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.esi + disp8, rmbyte, write)); break;
			case 7: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.edi + disp8, rmbyte, write)); break;
			default:_impossible_;break;}
			_chk(vcpuins.prm = _m_addr_physical_linear(vcpuins.lrm, rmbyte, write));
			_chk(vcpuins.rrm = _m_ref_physical(vcpuins.prm));
			_be;break;
		case 2: _bb("ModRM_MOD(2)");
			_chk(disp32 = (t_nubit32)_d_code(4));
			switch (_GetModRM_RM(modrm)) {
			case 0: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.eax + disp32, rmbyte, write)); break;
			case 1: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.ecx + disp32, rmbyte, write)); break;
			case 2: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.edx + disp32, rmbyte, write)); break;
			case 3: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.ebx + disp32, rmbyte, write)); break;
			case 4:
				_bb("ModRM_RM(4)");
				switch (_GetSIB_Base(sib)) {
				case 0: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.eax + sibindex + disp32, rmbyte, write)); break;
				case 1: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.ecx + sibindex + disp32, rmbyte, write)); break;
				case 2: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.edx + sibindex + disp32, rmbyte, write)); break;
				case 3: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.ebx + sibindex + disp32, rmbyte, write)); break;
				case 4: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverss, vcpu.esp + sibindex + disp32, rmbyte, write)); break;
				case 5: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverss, vcpu.ebp + sibindex + disp32, rmbyte, write)); break;
				case 6: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.esi + sibindex + disp32, rmbyte, write)); break;
				case 7: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.edi + sibindex + disp32, rmbyte, write)); break;
				default:_impossible_;break;}
				_be;break;
			case 5: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverss, vcpu.ebp + disp32, rmbyte, write)); break;
			case 6: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.esi + disp32, rmbyte, write)); break;
			case 7: _chk(vcpuins.lrm = _m_addr_linear_logical(vcpuins.roverds, vcpu.edi + disp32, rmbyte, write)); break;
			default:_impossible_;break;}
			_chk(vcpuins.prm = _m_addr_physical_linear(vcpuins.lrm, rmbyte, write));
			_chk(vcpuins.rrm = _m_ref_physical(vcpuins.prm));
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
		vcpuins.flagmem = 0x00;
		switch (rmbyte) {
		case 1:
			switch(_GetModRM_RM(modrm)) {
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
			switch(_GetModRM_RM(modrm)) {
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
			switch(_GetModRM_RM(modrm)) {
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
			_chk(_SetExcept_CE(0));
			_be;break;
		}
		_be;
	}
	switch (regbyte) {
	case 0:
		/* reg is operation or segment */
		vcpuins.rr = _GetModRM_REG(modrm);
		break;
	case 1:
		switch(_GetModRM_REG(modrm)) {
		case 0: vcpuins.rr = (t_vaddrcc)(&vcpu.al);break;
		case 1: vcpuins.rr = (t_vaddrcc)(&vcpu.cl);break;
		case 2: vcpuins.rr = (t_vaddrcc)(&vcpu.dl);break;
		case 3: vcpuins.rr = (t_vaddrcc)(&vcpu.bl);break;
		case 4: vcpuins.rr = (t_vaddrcc)(&vcpu.ah);break;
		case 5: vcpuins.rr = (t_vaddrcc)(&vcpu.ch);break;
		case 6: vcpuins.rr = (t_vaddrcc)(&vcpu.dh);break;
		case 7: vcpuins.rr = (t_vaddrcc)(&vcpu.bh);break;
		default:_impossible_;break;}
		break;
	case 2:
		switch(_GetModRM_REG(modrm)) {
		case 0: vcpuins.rr = (t_vaddrcc)(&vcpu.ax);break;
		case 1: vcpuins.rr = (t_vaddrcc)(&vcpu.cx);break;
		case 2: vcpuins.rr = (t_vaddrcc)(&vcpu.dx);break;
		case 3: vcpuins.rr = (t_vaddrcc)(&vcpu.bx);break;
		case 4: vcpuins.rr = (t_vaddrcc)(&vcpu.sp);break;
		case 5: vcpuins.rr = (t_vaddrcc)(&vcpu.bp);break;
		case 6: vcpuins.rr = (t_vaddrcc)(&vcpu.si);break;
		case 7: vcpuins.rr = (t_vaddrcc)(&vcpu.di);break;
		default:_impossible_;break;}
		break;
	case 4:
		switch(_GetModRM_REG(modrm)) {
		case 0: vcpuins.rr = (t_vaddrcc)(&vcpu.eax);break;
		case 1: vcpuins.rr = (t_vaddrcc)(&vcpu.ecx);break;
		case 2: vcpuins.rr = (t_vaddrcc)(&vcpu.edx);break;
		case 3: vcpuins.rr = (t_vaddrcc)(&vcpu.ebx);break;
		case 4: vcpuins.rr = (t_vaddrcc)(&vcpu.esp);break;
		case 5: vcpuins.rr = (t_vaddrcc)(&vcpu.ebp);break;
		case 6: vcpuins.rr = (t_vaddrcc)(&vcpu.esi);break;
		case 7: vcpuins.rr = (t_vaddrcc)(&vcpu.edi);break;
		default:_impossible_;break;}
		break;
	default: _bb("regbyte");
		_chk(_SetExcept_CE(0));
		_be;break;
	}
	_ce;
}

/* stream control unit: _c_, jmp, call */

/* stack control unit: _s_, push, pop */

/* arithmetic unit */
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
static void _a_CalcCF()
{
	_cb("_a_CalcCF");
	switch(vcpuins.type) {
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
			(_GetEFLAGS_CF && (vcpuins.opr2 == 0xff)));
		break;
	case SBB16:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (vcpuins.opr1 < vcpuins.result) ||
			(_GetEFLAGS_CF && (vcpuins.opr2 == 0xffff)));
		break;
	case SUB8:
	case SUB16:
	case CMP8:
	case CMP16:
		MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,vcpuins.opr1 < vcpuins.opr2);
		break;
	default:_chk(_SetExcept_CE(0));break;}
	_ce;
}
static void _a_CalcOF()
{
	_cb("_a_CalcOF");
	switch(vcpuins.type) {
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
		MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,((vcpuins.opr1&0x0080) != (vcpuins.opr2&0x0080)) && ((vcpuins.opr2&0x0080) == (vcpuins.result&0x0080)));
		break;
	case SBB16:
	case SUB16:
	case CMP16:
		MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,((vcpuins.opr1&0x8000) != (vcpuins.opr2&0x8000)) && ((vcpuins.opr2&0x8000) == (vcpuins.result&0x8000)));
		break;
	default:_chk(_SetExcept_CE(0));break;}
	_ce;
}
static void _a_CalcAF()
{
	MakeBit(vcpu.eflags, VCPU_EFLAGS_AF, ((vcpuins.opr1^vcpuins.opr2)^vcpuins.result) & 0x10);
}
static void _a_CalcPF()
{
	t_nubit8 res8 = GetMax8(vcpuins.result);
	t_nubitcc count = 0;
	while(res8)
	{
		res8 &= res8-1; 
		count++;
	}
	MakeBit(vcpu.eflags, VCPU_EFLAGS_PF, !(count & 0x01));
}
static void _a_CalcZF()
{
	MakeBit(vcpu.eflags, VCPU_EFLAGS_ZF, !vcpuins.result);
}
static void _a_CalcSF()
{
	_cb("_a_CalcSF");
	switch(vcpuins.bit) {
	case 8: MakeBit(vcpu.eflags, VCPU_EFLAGS_SF, GetMSB8(vcpuins.result));break;
	case 16:MakeBit(vcpu.eflags, VCPU_EFLAGS_SF, GetMSB16(vcpuins.result));break;
	case 32:MakeBit(vcpu.eflags, VCPU_EFLAGS_SF, GetMSB32(vcpuins.result));break;
	default:_chk(_SetExcept_CE(0));break;}
	_ce;
}
static void _a_SetFlags(t_nubit16 flags)
{
	_cb("_a_SetFlags");
	if(flags & VCPU_EFLAGS_CF) _chk(_a_CalcCF());
	if(flags & VCPU_EFLAGS_PF) _chk(_a_CalcPF());
	if(flags & VCPU_EFLAGS_AF) _chk(_a_CalcAF());
	if(flags & VCPU_EFLAGS_ZF) _chk(_a_CalcZF());
	if(flags & VCPU_EFLAGS_SF) _chk(_a_CalcSF());
	if(flags & VCPU_EFLAGS_OF) _chk(_a_CalcOF());
	_ce;
}

static void GetMem()
{
	/* returns vcpuins.rrm */
	vcpuins.rrm = vramGetRealAddress(vcpuins.roverds->selector,vramVarWord(vcpu.cs.selector,vcpu.eip));
	vcpu.eip += 2;
}
static void GetImm(t_nubitcc immbit)
{
	// returns vcpuins.rimm
	vcpuins.rimm = vramGetRealAddress(vcpu.cs.selector,vcpu.eip);
	switch(immbit) {
	case 8:  vcpu.eip += 1;break;
	case 16: vcpu.eip += 2;break;
	case 32: vcpu.eip += 4;break;
	default: CaseError("GetImm::immbit");break;}
}
static void GetModRegRM(t_nubitcc regbit,t_nubitcc rmbit)
{
	// returns vcpuins.rrm and vcpuins.rr
	t_nsbit8 disp8 = 0x00;
	t_nubit16 disp16 = 0x0000;
	t_nubit32 disp32 = 0x00000000;
	t_nubit8 modrm = 0x00;
	modrm = (t_nubit8)_d_code(1);
	vcpuins.rrm = vcpuins.rr = (t_vaddrcc)NULL;
	switch(_GetModRM_MOD(modrm)) {
	case 0:
		switch(_GetModRM_RM(modrm)) {
		case 0:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverds->selector,vcpu.bx+vcpu.si);break;
		case 1:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverds->selector,vcpu.bx+vcpu.di);break;
		case 2:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverss->selector,vcpu.bp+vcpu.si);break;
		case 3:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverss->selector,vcpu.bp+vcpu.di);break;
		case 4:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverds->selector,vcpu.si);break;
		case 5:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverds->selector,vcpu.di);break;
		case 6: disp16 = vramVarWord(vcpu.cs.selector,vcpu.eip);vcpu.eip += 2;
			    vcpuins.rrm = vramGetRealAddress(vcpuins.roverds->selector,disp16); break;
		case 7:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverds->selector,vcpu.bx);break;
		default:CaseError("GetModRegRM::MOD0::ModRM_RM");break;}
		break;
	case 1:
		disp8 = (t_nsbit8)vramVarByte(vcpu.cs.selector,vcpu.eip);vcpu.eip += 1;
		switch(_GetModRM_RM(modrm)) {
		case 0:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverds->selector,vcpu.bx+vcpu.si+disp8);break;
		case 1:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverds->selector,vcpu.bx+vcpu.di+disp8);break;
		case 2:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverss->selector,vcpu.bp+vcpu.si+disp8);break;
		case 3:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverss->selector,vcpu.bp+vcpu.di+disp8);break;
		case 4:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverds->selector,vcpu.si+disp8);break;
		case 5:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverds->selector,vcpu.di+disp8);break;
		case 6:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverss->selector,vcpu.bp+disp8);break;
		case 7:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverds->selector,vcpu.bx+disp8);break;
		default:CaseError("GetModRegRM::MOD1::ModRM_RM");break;}
		break;
	case 2:
		disp16 = vramVarWord(vcpu.cs.selector,vcpu.eip);vcpu.eip += 2;
		switch(_GetModRM_RM(modrm)) {
		case 0:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverds->selector,vcpu.bx+vcpu.si+disp16);break;
		case 1:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverds->selector,vcpu.bx+vcpu.di+disp16);break;
		case 2:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverss->selector,vcpu.bp+vcpu.si+disp16);break;
		case 3:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverss->selector,vcpu.bp+vcpu.di+disp16);break;
		case 4:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverds->selector,vcpu.si+disp16);break;
		case 5:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverds->selector,vcpu.di+disp16);break;
		case 6:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverss->selector,vcpu.bp+disp16);break;
		case 7:	vcpuins.rrm = vramGetRealAddress(vcpuins.roverds->selector,vcpu.bx+disp16);break;
		default:CaseError("GetModRegRM::MOD2::ModRM_RM");break;}
		break;
	case 3:
		switch(_GetModRM_RM(modrm)) {
		case 0:	if(rmbit == 8) vcpuins.rrm = (t_vaddrcc)(&vcpu.al); else vcpuins.rrm = (t_vaddrcc)(&vcpu.ax); break;
		case 1:	if(rmbit == 8) vcpuins.rrm = (t_vaddrcc)(&vcpu.cl); else vcpuins.rrm = (t_vaddrcc)(&vcpu.cx); break;
		case 2:	if(rmbit == 8) vcpuins.rrm = (t_vaddrcc)(&vcpu.dl); else vcpuins.rrm = (t_vaddrcc)(&vcpu.dx); break;
		case 3:	if(rmbit == 8) vcpuins.rrm = (t_vaddrcc)(&vcpu.bl); else vcpuins.rrm = (t_vaddrcc)(&vcpu.bx); break;
		case 4:	if(rmbit == 8) vcpuins.rrm = (t_vaddrcc)(&vcpu.ah); else vcpuins.rrm = (t_vaddrcc)(&vcpu.sp); break;
		case 5:	if(rmbit == 8) vcpuins.rrm = (t_vaddrcc)(&vcpu.ch); else vcpuins.rrm = (t_vaddrcc)(&vcpu.bp); break;
		case 6:	if(rmbit == 8) vcpuins.rrm = (t_vaddrcc)(&vcpu.dh); else vcpuins.rrm = (t_vaddrcc)(&vcpu.si); break;
		case 7:	if(rmbit == 8) vcpuins.rrm = (t_vaddrcc)(&vcpu.bh); else vcpuins.rrm = (t_vaddrcc)(&vcpu.di); break;
		default:CaseError("GetModRegRM::MOD3::ModRM_RM");break;}
		break;
	default:CaseError("GetModRegRM::ModRM_MOD");break;}
	switch(regbit) {
	case 0:		vcpuins.rr = _GetModRM_REG(modrm);break;
	case 4:
		switch(_GetModRM_REG(modrm)) {
		case 0:	vcpuins.rr = (t_vaddrcc)(&vcpu.es.selector);	break;
		case 1:	vcpuins.rr = (t_vaddrcc)(&vcpu.cs.selector);	break;
		case 2:	vcpuins.rr = (t_vaddrcc)(&vcpu.ss.selector);	break;
		case 3:	vcpuins.rr = (t_vaddrcc)(&vcpu.ds.selector);	break;
		case 4:	vcpuins.rr = (t_vaddrcc)(&vcpu.fs.selector);	break;
		case 5:	vcpuins.rr = (t_vaddrcc)(&vcpu.gs.selector);	break;
		default:CaseError("GetModRegRM::regbit4::ModRM_REG");break;}
		break;
	case 8:
		switch(_GetModRM_REG(modrm)) {
		case 0:	vcpuins.rr = (t_vaddrcc)(&vcpu.al);	break;
		case 1:	vcpuins.rr = (t_vaddrcc)(&vcpu.cl);	break;
		case 2:	vcpuins.rr = (t_vaddrcc)(&vcpu.dl);	break;
		case 3:	vcpuins.rr = (t_vaddrcc)(&vcpu.bl);	break;
		case 4:	vcpuins.rr = (t_vaddrcc)(&vcpu.ah);	break;
		case 5:	vcpuins.rr = (t_vaddrcc)(&vcpu.ch);	break;
		case 6:	vcpuins.rr = (t_vaddrcc)(&vcpu.dh);	break;
		case 7:	vcpuins.rr = (t_vaddrcc)(&vcpu.bh);	break;
		default:CaseError("GetModRegRM::regbit8::ModRM_REG");break;}
		break;
	case 16:
		switch(_GetModRM_REG(modrm)) {
		case 0: vcpuins.rr = (t_vaddrcc)(&vcpu.ax);	break;
		case 1:	vcpuins.rr = (t_vaddrcc)(&vcpu.cx);	break;
		case 2:	vcpuins.rr = (t_vaddrcc)(&vcpu.dx);	break;
		case 3:	vcpuins.rr = (t_vaddrcc)(&vcpu.bx);	break;
		case 4:	vcpuins.rr = (t_vaddrcc)(&vcpu.sp);	break;
		case 5:	vcpuins.rr = (t_vaddrcc)(&vcpu.bp);	break;
		case 6:	vcpuins.rr = (t_vaddrcc)(&vcpu.si);	break;
		case 7: vcpuins.rr = (t_vaddrcc)(&vcpu.di);	break;
		default:CaseError("GetModRegRM::regbit16::ModRM_REG");break;}
		break;
	default:CaseError("GetModRegRM::regbit");break;}
}
static void GetModRegRMEA()
{
	t_nsbit8 disp8 = 0x00;
	t_nubit16 disp16 = 0x0000;
	t_nubit8 modrm = vramVarByte(vcpu.cs.selector,vcpu.eip++);
	vcpuins.rrm = vcpuins.rr = (t_vaddrcc)NULL;
	switch(_GetModRM_MOD(modrm)) {
	case 0:
		switch(_GetModRM_RM(modrm)) {
		case 0:	vcpuins.rrm = vcpu.bx+vcpu.si;break;
		case 1:	vcpuins.rrm = vcpu.bx+vcpu.di;break;
		case 2:	vcpuins.rrm = vcpu.bp+vcpu.si;break;
		case 3:	vcpuins.rrm = vcpu.bp+vcpu.di;break;
		case 4:	vcpuins.rrm = vcpu.si;break;
		case 5:	vcpuins.rrm = vcpu.di;break;
		case 6:	vcpuins.rrm = vramVarWord(vcpu.cs.selector,vcpu.eip);vcpu.eip += 2;break;
		case 7:	vcpuins.rrm = vcpu.bx;break;
		default:CaseError("GetModRegRMEA::MOD0::ModRM_RM");break;}
		break;
	case 1:
		disp8 = (t_nsbit8)vramVarByte(vcpu.cs.selector,vcpu.eip);vcpu.eip += 1;
		switch(_GetModRM_RM(modrm)) {
		case 0:	vcpuins.rrm = vcpu.bx+vcpu.si+disp8;break;
		case 1:	vcpuins.rrm = vcpu.bx+vcpu.di+disp8;break;
		case 2:	vcpuins.rrm = vcpu.bp+vcpu.si+disp8;break;
		case 3:	vcpuins.rrm = vcpu.bp+vcpu.di+disp8;break;
		case 4:	vcpuins.rrm = vcpu.si+disp8;break;
		case 5:	vcpuins.rrm = vcpu.di+disp8;break;
		case 6:	vcpuins.rrm = vcpu.bp+disp8;break;
		case 7:	vcpuins.rrm = vcpu.bx+disp8;break;
		default:CaseError("GetModRegRMEA::MOD1::ModRM_RM");break;}
		vcpuins.rrm %= 0x10000;
		break;
	case 2:
		disp16 = vramVarWord(vcpu.cs.selector,vcpu.eip);vcpu.eip += 2;
		switch(_GetModRM_RM(modrm)) {
		case 0:	vcpuins.rrm = vcpu.bx+vcpu.si+disp16;break;
		case 1:	vcpuins.rrm = vcpu.bx+vcpu.di+disp16;break;
		case 2:	vcpuins.rrm = vcpu.bp+vcpu.si+disp16;break;
		case 3:	vcpuins.rrm = vcpu.bp+vcpu.di+disp16;break;
		case 4:	vcpuins.rrm = vcpu.si+disp16;break;
		case 5:	vcpuins.rrm = vcpu.di+disp16;break;
		case 6:
			vcpuins.rrm = vcpu.bp+disp16;
			break;
		case 7:	vcpuins.rrm = vcpu.bx+disp16;break;
		default:CaseError("GetModRegRMEA::MOD2::ModRM_RM");break;}
		vcpuins.rrm %= 0x10000;
		break;
	default:CaseError("GetModRegRMEA::ModRM_MOD");break;}
	switch(_GetModRM_REG(modrm)) {
	case 0: vcpuins.rr = (t_vaddrcc)(&vcpu.ax);	break;
	case 1:	vcpuins.rr = (t_vaddrcc)(&vcpu.cx);	break;
	case 2:	vcpuins.rr = (t_vaddrcc)(&vcpu.dx);	break;
	case 3:	vcpuins.rr = (t_vaddrcc)(&vcpu.bx);	break;
	case 4:	vcpuins.rr = (t_vaddrcc)(&vcpu.sp);	break;
	case 5:	vcpuins.rr = (t_vaddrcc)(&vcpu.bp);	break;
	case 6:	vcpuins.rr = (t_vaddrcc)(&vcpu.si);	break;
	case 7: vcpuins.rr = (t_vaddrcc)(&vcpu.di);	break;
	default:CaseError("GetModRegRMEA::ModRM_REG");break;}
}

#define done static void
#define todo static void
#define tots static void
#define bugfix(n) if(1)
#define i386(n)  if (1)

static void INT(t_nubit8 intid);

done _a_add(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	_cb("ADD");
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = ADD8;
		vcpuins.opr1 = GetMax8(d_nubit8(rdest));
		vcpuins.opr2 = GetMax8(d_nubit8(rsrc));
		vcpuins.result = GetMax8(vcpuins.opr1 + vcpuins.opr2);
		d_nubit8(rdest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = ADD16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nsbit8(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 + vcpuins.opr2);
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = ADD16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nubit16(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 + vcpuins.opr2);
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		break;
	case 20:
		vcpuins.bit = 32;
		vcpuins.type = ADD32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nsbit8(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 + vcpuins.opr2);
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		break;
	case 32:
		vcpuins.bit = 32;
		vcpuins.type = ADD32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nubit32(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 + vcpuins.opr2);
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		break;
	default:
		_bb("bit");
		_chk(_SetExcept_CE(0));
		_be;
		break;
	}
	_a_SetFlags(ADD_FLAG);
	_ce;
}
done _a_adc(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	_cb("ADC");
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = ADC8;
		vcpuins.opr1 = GetMax8(d_nubit8(rdest));
		vcpuins.opr2 = GetMax8(d_nubit8(rsrc));
		vcpuins.result = GetMax8(vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF);
		d_nubit8(rdest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = ADC16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nsbit8(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF);
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = ADC16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nubit16(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF);
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		break;
	case 20:
		vcpuins.bit = 32;
		vcpuins.type = ADC32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nsbit8(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF);
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		break;
	case 32:
		vcpuins.bit = 32;
		vcpuins.type = ADC32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nubit32(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF);
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		break;
	default:
		_bb("bit");
		_chk(_SetExcept_CE(0));
		_be;
		break;}
	_a_SetFlags(ADC_FLAG);
	_ce;
}
done _a_and(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit)
{
	_cb("AND");
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		//vcpuins.type = AND8;
		vcpuins.opr1 = GetMax8(d_nubit8(rdest));
		vcpuins.opr2 = GetMax8(d_nubit8(rsrc));
		vcpuins.result = GetMax8(vcpuins.opr1 & vcpuins.opr2);
		d_nubit8(rdest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		//vcpuins.type = AND16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nsbit8(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 & vcpuins.opr2);
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		//vcpuins.type = AND16;
		vcpuins.opr1 = GetMax16(d_nubit16(rdest));
		vcpuins.opr2 = GetMax16(d_nubit16(rsrc));
		vcpuins.result = GetMax16(vcpuins.opr1 & vcpuins.opr2);
		d_nubit16(rdest) = (t_nubit16)vcpuins.result;
		break;
	case 20:
		vcpuins.bit = 32;
		//vcpuins.type = AND32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nsbit8(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 & vcpuins.opr2);
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		break;
	case 32:
		vcpuins.bit = 32;
		//vcpuins.type = AND32;
		vcpuins.opr1 = GetMax32(d_nubit32(rdest));
		vcpuins.opr2 = GetMax32(d_nubit32(rsrc));
		vcpuins.result = GetMax32(vcpuins.opr1 & vcpuins.opr2);
		d_nubit32(rdest) = (t_nubit32)vcpuins.result;
		break;
	default:
		_bb("bit");
		_chk(_SetExcept_CE(0));
		_be;
		break;}
	_ClrOF;
	_ClrEFLAGS_CF;
	_ClrEFLAGS_AF;
	_a_SetFlags(AND_FLAG);
	_ce;
}
static void OR(void *dest, void *src, t_nubit8 bit)
{
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		//vcpuins.type = OR8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nubit8(src) & 0xff;
		vcpuins.result = (vcpuins.opr1|vcpuins.opr2) & 0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		//vcpuins.type = OR16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nsbit8(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1|vcpuins.opr2) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		//vcpuins.type = OR16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nubit16(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1|vcpuins.opr2) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("OR::bit");break;}
	_ClrOF;
	_ClrEFLAGS_CF;
	_ClrEFLAGS_AF;
	_a_SetFlags(OR_FLAG);
}
static void SBB(void *dest, void *src, t_nubit8 bit)
{
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = SBB8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nubit8(src) & 0xff;
		vcpuins.result = (vcpuins.opr1-(vcpuins.opr2+_GetEFLAGS_CF)) & 0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = SBB16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nsbit8(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1-(vcpuins.opr2+_GetEFLAGS_CF)) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = SBB16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nubit16(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1-(vcpuins.opr2+_GetEFLAGS_CF)) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("SBB::bit");break;}
	_a_SetFlags(SBB_FLAG);
}
static void SUB(void *dest, void *src, t_nubit8 bit)
{
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = SUB8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nubit8(src) & 0xff;
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = SUB16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nsbit8(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = SUB16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nubit16(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("SUB::bit");break;}
	_a_SetFlags(SUB_FLAG);
}
static void XOR(void *dest, void *src, t_nubit8 bit)
{
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		//vcpuins.type = XOR8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nubit8(src) & 0xff;
		vcpuins.result = (vcpuins.opr1^vcpuins.opr2)&0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		//vcpuins.type = XOR16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nsbit8(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1^vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		//vcpuins.type = XOR16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nubit16(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1^vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("XOR::bit");break;}
	_ClrOF;
	_ClrEFLAGS_CF;
	_ClrEFLAGS_AF;
	_a_SetFlags(XOR_FLAG);
}
static void CMP(void *dest, void *src, t_nubit8 bit)
{
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = CMP8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nsbit8(src) & 0xff;
		bugfix(7) vcpuins.result = ((t_nubit8)vcpuins.opr1-(t_nsbit8)vcpuins.opr2)&0xff;
		else vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nsbit8(src) & 0xffff;
		vcpuins.result = ((t_nubit16)vcpuins.opr1-(t_nsbit8)vcpuins.opr2)&0xffff;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nsbit16(src) & 0xffff;
		bugfix(7) vcpuins.result = ((t_nubit16)vcpuins.opr1-(t_nsbit16)vcpuins.opr2)&0xffff;
		else vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;
		break;
	default:CaseError("CMP::bit");break;}
	_a_SetFlags(CMP_FLAG);
}
static void PUSH(void *src, t_nubit8 bit)
{
	t_nubit16 data16;
	t_nubit32 data32;
	switch(bit) {
	case 16:
		data16 = d_nubit16(src);
		vcpuins.bit = 16;
		vcpu.sp -= 2;
		vramVarWord(vcpu.ss.selector,vcpu.sp) = data16;
		break;
	case 32:
		data32 = d_nubit32(src);
		vcpuins.bit = 32;
		vcpu.sp -= 4;
		vramVarDWord(vcpu.ss.selector,vcpu.sp) = data32;
		break;
	default:CaseError("PUSH::bit");break;}
	_ce;
}
static void POP(void *dest, t_nubit8 bit)
{
	switch(bit) {
	case 16:
		vcpuins.bit = 16;
		d_nubit16(dest) = vramVarWord(vcpu.ss.selector,vcpu.sp);
		vcpu.sp += 2;
		break;
	default:CaseError("POP::bit");break;}
}
static void INC(void *dest, t_nubit8 bit)
{
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = ADD8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = 0x01;
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2) & 0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = ADD16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = 0x0001;
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("INC::bit");break;}
	_a_SetFlags(INC_FLAG);
}
static void DEC(void *dest, t_nubit8 bit)
{
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = SUB8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = 0x01;
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = SUB16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = 0x0001;
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("DEC::bit");break;}
	_a_SetFlags(DEC_FLAG);
}
static void JCC(void *src, t_bool flagj,t_nubit8 bit)
{
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		if(flagj)
			bugfix(5) vcpu.eip += d_nsbit8(src);
			else vcpu.eip += d_nubit8(src);
		break;
	default:CaseError("JCC::bit");break;}
}
static void TEST(void *dest, void *src, t_nubit8 bit)
{
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		//vcpuins.type = TEST8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nubit8(src) & 0xff;
		vcpuins.result = (vcpuins.opr1&vcpuins.opr2)&0xff;
		break;
	case 16:
		vcpuins.bit = 16;
		//vcpuins.type = TEST16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nubit16(src) & 0xffff;
		vcpuins.result = (vcpuins.opr1&vcpuins.opr2)&0xffff;
		break;
	default:CaseError("TEST::bit");break;}
	_ClrOF;
	_ClrEFLAGS_CF;
	_ClrEFLAGS_AF;
	_a_SetFlags(TEST_FLAG);
}
static void XCHG(void *dest, void *src, t_nubit8 bit)
{
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = d_nubit8(src) & 0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.opr2;
		d_nubit8(src) = (t_nubit8)vcpuins.opr1;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = d_nubit16(src) & 0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.opr2;
		d_nubit16(src) = (t_nubit16)vcpuins.opr1;
		break;
	default:CaseError("XCHG::bit");break;}
}
static void MOV(void *dest, void *src, t_nubit8 bit)
{
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		d_nubit8(dest) = d_nubit8(src);
		break;
	case 16:
		vcpuins.bit = 16;
		d_nubit16(dest) = d_nubit16(src);
		break;
	default:CaseError("MOV::bit");break;}
}
static void ROL(void *dest, void *src, t_nubit8 bit)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(bit) {
	case 8:
		tempcount = (count & 0x1f) % 8;
		vcpuins.bit = 8;
		while(tempcount) {
			tempCF = GetMSB(d_nubit8(dest), 8);
			d_nubit8(dest) = (d_nubit8(dest)<<1)+(t_nubit8)tempCF;
			tempcount--;
		}
		MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetLSB(d_nubit8(dest), 8));
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^_GetEFLAGS_CF);
		break;
	case 16:
		tempcount = (count & 0x1f) % 16;
		vcpuins.bit = 16;
		while(tempcount) {
			tempCF = GetMSB(d_nubit16(dest), 16);
			d_nubit16(dest) = (d_nubit16(dest)<<1)+(t_nubit16)tempCF;
			tempcount--;
		}
		MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetLSB(d_nubit16(dest), 16));
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^_GetEFLAGS_CF);
		break;
	default:CaseError("ROL::bit");break;}
}
static void ROR(void *dest, void *src, t_nubit8 bit)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(bit) {
	case 8:
		tempcount = (count & 0x1f) % 8;
		vcpuins.bit = 8;
		while(tempcount) {
			tempCF = GetLSB(d_nubit8(dest), 8);
			d_nubit8(dest) >>= 1;
			if(tempCF) d_nubit8(dest) |= 0x80;
			tempcount--;
		}
		MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetMSB(d_nubit8(dest), 8));
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^(!!(d_nubit8(dest)&0x40)));
		break;
	case 16:
		tempcount = (count & 0x1f) % 16;
		vcpuins.bit = 16;
		while(tempcount) {
			tempCF = GetLSB(d_nubit16(dest), 16);
			d_nubit16(dest) >>= 1;
			if(tempCF) d_nubit16(dest) |= 0x8000;
			tempcount--;
		}
		MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetMSB(d_nubit16(dest), 16));
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^(!!(d_nubit16(dest)&0x4000)));
		break;
	default:CaseError("ROR::bit");break;}
}
static void RCL(void *dest, void *src, t_nubit8 bit)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(bit) {
	case 8:
		tempcount = (count & 0x1f) % 9;
		vcpuins.bit = 8;
		while(tempcount) {
			tempCF = GetMSB(d_nubit8(dest), 8);
			d_nubit8(dest) = (d_nubit8(dest)<<1)+_GetEFLAGS_CF;
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,tempCF);
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^_GetEFLAGS_CF);
		break;
	case 16:
		tempcount = (count & 0x1f) % 17;
		vcpuins.bit = 16;
		while(tempcount) {
			tempCF = GetMSB(d_nubit16(dest), 16);
			d_nubit16(dest) = (d_nubit16(dest)<<1)+_GetEFLAGS_CF;
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,tempCF);
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^_GetEFLAGS_CF);
		break;
	default:CaseError("RCL::bit");break;}
}
static void RCR(void *dest, void *src, t_nubit8 bit)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(bit) {
	case 8:
		tempcount = (count & 0x1f) % 9;
		vcpuins.bit = 8;
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^_GetEFLAGS_CF);
		while(tempcount) {
			tempCF = GetLSB(d_nubit8(dest), 8);
			d_nubit8(dest) >>= 1;
			if(_GetEFLAGS_CF) d_nubit8(dest) |= 0x80;
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,tempCF);
			tempcount--;
		}
		break;
	case 16:
		tempcount = (count & 0x1f) % 17;
		vcpuins.bit = 16;
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^_GetEFLAGS_CF);
		while(tempcount) {
			tempCF = GetLSB(d_nubit16(dest), 16);
			d_nubit16(dest) >>= 1;
			if(_GetEFLAGS_CF) d_nubit16(dest) |= 0x8000;
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,tempCF);
			tempcount--;
		}
		break;
	default:CaseError("RCR::bit");break;}
}
static void SHL(void *dest, void *src, t_nubit8 bit)
{
	t_nubit8 count,tempcount;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetMSB(d_nubit8(dest), 8));
			d_nubit8(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^_GetEFLAGS_CF);
		if(count != 0) {
			vcpuins.result = d_nubit8(dest);
			_a_SetFlags(SHL_FLAG);
		}
		break;
	case 16:
		vcpuins.bit = 16;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetMSB(d_nubit16(dest), 16));
			d_nubit16(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^_GetEFLAGS_CF);
		if(count != 0) {
			vcpuins.result = d_nubit16(dest);
			_a_SetFlags(SHL_FLAG);
		}
		break;
	default:CaseError("SHL::bit");break;}
	if (count) _ClrEFLAGS_AF;
}
static void SHR(void *dest, void *src, t_nubit8 bit)
{
	t_nubit8 count,tempcount,temrdest8;
	t_nubit16 temrdest16;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		tempcount = count&0x1f;
		temrdest8 = d_nubit8(dest);
		while(tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetLSB(d_nubit8(dest), 8));
			d_nubit8(dest) >>= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,!!(temrdest8&0x80));
		if(count != 0) {
			vcpuins.result = d_nubit8(dest);
			_a_SetFlags(SHR_FLAG);
		}
		break;
	case 16:
		vcpuins.bit = 16;
		tempcount = count&0x1f;
		temrdest16 = d_nubit16(dest);
		while(tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetLSB(d_nubit16(dest), 16));
			d_nubit16(dest) >>= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,!!(temrdest16&0x8000));
		if(count != 0) {
			vcpuins.result = d_nubit16(dest);
			_a_SetFlags(SHR_FLAG);
		}
		break;
	default:CaseError("SHR::bit");break;}
	if (count) _ClrEFLAGS_AF;
}
static void SAL(void *dest, void *src, t_nubit8 bit)
{
	t_nubit8 count,tempcount;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetMSB(d_nubit8(dest), 8));
			d_nubit8(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^_GetEFLAGS_CF);
		if(count != 0) {
			vcpuins.result = d_nubit8(dest);
			_a_SetFlags(SAL_FLAG);
		}
		break;
	case 16:
		vcpuins.bit = 16;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetMSB(d_nubit16(dest), 16));
			d_nubit16(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^_GetEFLAGS_CF);
		if(count != 0) {
			vcpuins.result = d_nubit16(dest);
			_a_SetFlags(SAL_FLAG);
		}
		break;
	default:CaseError("SAL::bit");break;}
	if (count) _ClrEFLAGS_AF;
}
static void SAR(void *dest, void *src, t_nubit8 bit)
{
	t_nubit8 count,tempcount,temrdest8;
	t_nubit16 temrdest16;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		tempcount = count&0x1f;
		temrdest8 = d_nubit8(dest);
		while(tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetLSB(d_nubit8(dest), 8));
			d_nsbit8(dest) >>= 1;
			//d_nubit8(dest) |= temrdest8&0x80;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,0);
		if(count != 0) {
			vcpuins.result = d_nsbit8(dest);
			_a_SetFlags(SAR_FLAG);
		}
		break;
	case 16:
		vcpuins.bit = 16;
		tempcount = count&0x1f;
		temrdest16 = d_nubit16(dest);
		while(tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetLSB(d_nubit16(dest), 16));
			d_nsbit16(dest) >>= 1;
			//d_nubit16(dest) |= temrdest16&0x8000;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,0);
		if(count != 0) {
			vcpuins.result = d_nsbit16(dest);
			_a_SetFlags(SAR_FLAG);
		}
		break;
	default:CaseError("SAR::bit");break;}
	if (count) _ClrEFLAGS_AF;
}
static void STRDIR(t_nubit8 bit, t_bool flagsi, t_bool flagdi)
{
	bugfix(10) {
		/* add parameters flagsi, flagdi */
	} else ;
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		if(_GetDF) {
			if (flagdi) vcpu.di--;
			if (flagsi) vcpu.si--;
		} else {
			if (flagdi) vcpu.di++;
			if (flagsi) vcpu.si++;
		}
		break;
	case 16:
		vcpuins.bit = 16;
		if(_GetDF) {
			if (flagdi) vcpu.di -= 2;
			if (flagsi) vcpu.si -= 2;
		} else {
			if (flagdi) vcpu.di += 2;
			if (flagsi) vcpu.si += 2;
		}
		break;
	default:CaseError("STRDIR::bit");break;}
}
static void MOVS(t_nubit8 bit)
{
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		vramVarByte(vcpu.es.selector,vcpu.di) = vramVarByte(vcpuins.roverds->selector,vcpu.si);
		STRDIR(8,1,1);
		// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOVSB\n");
		break;
	case 16:
		vcpuins.bit = 16;
		vramVarWord(vcpu.es.selector,vcpu.di) = vramVarWord(vcpuins.roverds->selector,vcpu.si);
		STRDIR(16,1,1);
		// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOVSW\n");
		break;
	default:CaseError("MOVS::bit");break;}
	//qdcgaCheckVideoRam(vramGetRealAddress(vcpu.es.selector, vcpu.di));
}
static void CMPS(t_nubit8 bit)
{
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = CMP8;
		vcpuins.opr1 = vramVarByte(vcpuins.roverds->selector,vcpu.si);
		vcpuins.opr2 = vramVarByte(vcpu.es.selector,vcpu.di);
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;
		STRDIR(8,1,1);
		_a_SetFlags(CMP_FLAG);
		// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  CMPSB\n");
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		vcpuins.opr1 = vramVarWord(vcpuins.roverds->selector,vcpu.si);
		vcpuins.opr2 = vramVarWord(vcpu.es.selector,vcpu.di);
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;
		STRDIR(16,1,1);
		_a_SetFlags(CMP_FLAG);
		// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  CMPSW\n");
		break;
	default:CaseError("CMPS::bit");break;}
}
static void STOS(t_nubit8 bit)
{
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		vramVarByte(vcpu.es.selector,vcpu.di) = vcpu.al;
		STRDIR(8,0,1);
		/*if (eCPU.di+t<0xc0000 && eCPU.di+t>=0xa0000)
		WriteVideoRam(eCPU.di+t-0xa0000);*/
		// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  STOSB\n");
		break;
	case 16:
		vcpuins.bit = 16;
		vramVarWord(vcpu.es.selector,vcpu.di) = vcpu.ax;
		STRDIR(16,0,1);
		/*if (eCPU.di+((t2=eCPU.es,t2<<4))<0xc0000 && eCPU.di+((t2=eCPU.es,t2<<4))>=0xa0000)
		{
			for (i=0;i<tmpOpdSize;i++)
			{
				WriteVideoRam(eCPU.di+((t2=eCPU.es,t2<<4))-0xa0000+i);
			}
		}*/
		// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  STOSW\n");
		break;
	default:CaseError("STOS::bit");break;}
}
static void LODS(t_nubit8 bit)
{
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		vcpu.al = vramVarByte(vcpuins.roverds->selector,vcpu.si);
		STRDIR(8,1,0);
		// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  LODSB\n");
		break;
	case 16:
		vcpuins.bit = 16;
		vcpu.ax = vramVarWord(vcpuins.roverds->selector,vcpu.si);
		STRDIR(16,1,0);
		// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  LODSW\n");
		break;
	default:CaseError("LODS::bit");break;}
}
static void SCAS(t_nubit8 bit)
{
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = CMP8;
		vcpuins.opr1 = vcpu.al;
		vcpuins.opr2 = vramVarByte(vcpu.es.selector,vcpu.di);
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;
		STRDIR(8,0,1);
		_a_SetFlags(CMP_FLAG);
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		vcpuins.opr1 = vcpu.ax;
		vcpuins.opr2 = vramVarWord(vcpu.es.selector,vcpu.di);
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;
		STRDIR(16,0,1);
		_a_SetFlags(CMP_FLAG);
		break;
	default:CaseError("SCAS::bit");break;}
}
static void NOT(void *dest, t_nubit8 bit)
{
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		d_nubit8(dest) = ~d_nubit8(dest);
		break;
	case 16:
		vcpuins.bit = 16;
		d_nubit16(dest) = ~d_nubit16(dest);
		break;
	default:CaseError("NOT::bit");break;}
}
static void NEG(void *dest, t_nubit8 bit)
{
	t_nubitcc zero = 0;
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		SUB((void *)&zero,(void *)dest,8);
		d_nubit8(dest) = (t_nubit8)zero;
		break;
	case 16:
		vcpuins.bit = 16;
		SUB((void *)&zero,(void *)dest,16);
		d_nubit16(dest) = (t_nubit16)zero;
		break;
	default:CaseError("NEG::bit");break;}
}
static void MUL(void *src, t_nubit8 bit)
{
	t_nubit32 tempresult;
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		vcpu.ax = vcpu.al * d_nubit8(src);
		MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,!!vcpu.ah);
		MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,!!vcpu.ah);
		break;
	case 16:
		vcpuins.bit = 16;
		tempresult = vcpu.ax * d_nubit16(src);
		vcpu.dx = (tempresult>>16)&0xffff;
		vcpu.ax = tempresult&0xffff;
		MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,!!vcpu.dx);
		MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,!!vcpu.dx);
		break;
	default:CaseError("MUL::bit");break;}
}
static void IMUL(void *src, t_nubit8 bit)
{
	t_nsbit32 tempresult;
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		vcpu.ax = (t_nsbit8)vcpu.al * d_nsbit8(src);
		if(vcpu.ax == vcpu.al) {
			_ClrOF;
			_ClrEFLAGS_CF;
		} else {
			_SetEFLAGS_OF;
			_SetEFLAGS_CF;
		}
		break;
	case 16:
		vcpuins.bit = 16;
		tempresult = (t_nsbit16)vcpu.ax * d_nsbit16(src);
		vcpu.dx = (t_nubit16)((tempresult>>16)&0xffff);
		vcpu.ax = (t_nubit16)(tempresult&0xffff);
		if(tempresult == (t_nsbit32)vcpu.ax) {
			_ClrOF;
			_ClrEFLAGS_CF;
		} else {
			_SetEFLAGS_OF;
			_SetEFLAGS_CF;
		}
		break;
	default:CaseError("IMUL::bit");break;}
}
static void DIV(void *src, t_nubit8 bit)
{
	t_nubit16 tempAX = vcpu.ax;
	t_nubit32 tempDXAX = (((t_nubit32)vcpu.dx)<<16)+vcpu.ax;
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		if(d_nubit8(src) == 0) {
			INT(0x00);
		} else {
			vcpu.al = (t_nubit8)(tempAX / d_nubit8(src));
			vcpu.ah = (t_nubit8)(tempAX % d_nubit8(src));
		}
		break;
	case 16:
		vcpuins.bit = 16;
		if(d_nubit16(src) == 0) {
			INT(0x00);
		} else {
			vcpu.ax = (t_nubit16)(tempDXAX / d_nubit16(src));
			vcpu.dx = (t_nubit16)(tempDXAX % d_nubit16(src));
		}
		break;
	default:CaseError("DIV::bit");break;}
}
static void IDIV(void *src, t_nubit8 bit)
{
	t_nsbit16 tempAX = vcpu.ax;
	t_nsbit32 tempDXAX = (((t_nubit32)vcpu.dx)<<16)+vcpu.ax;
	switch(bit) {
	case 8:
		vcpuins.bit = 8;
		if(d_nubit8(src) == 0) {
			INT(0x00);
		} else {
			vcpu.al = (t_nubit8)(tempAX / d_nsbit8(src));
			vcpu.ah = (t_nubit8)(tempAX % d_nsbit8(src));
		}
		break;
	case 16:
		vcpuins.bit = 16;
		if(d_nubit16(src) == 0) {
			INT(0x00);
		} else {
			vcpu.ax = (t_nubit16)(tempDXAX / d_nsbit16(src));
			vcpu.dx = (t_nubit16)(tempDXAX % d_nsbit16(src));
		}
		break;
	default:CaseError("IDIV::bit");break;}
}
static void INT(t_nubit8 intid)
{
	PUSH((void *)&vcpu.eflags,16);
	_ClrEFLAGS_IF;
	_ClrEFLAGS_TF;
	PUSH((void *)&vcpu.cs.selector,16);
	PUSH((void *)&vcpu.eip,16);
	vcpu.eip = vramVarWord(0x0000,intid*4+0);
	vcpu.cs.selector = vramVarWord(0x0000,intid*4+2);
}

#define _adv _d_skip(1)

done UndefinedOpcode()
{
	_cb("UndefinedOpcode");
	if (!_GetCR0_PE) {
		vapiPrint("The NXVM CPU has encountered an illegal instruction.\n");
		vapiPrint("CS:%04X IP:%04X OP:%02X %02X %02X %02X\n",
			vcpu.cs.selector, vcpu.eip, _m_read_cs(vcpu.eip+0, 1),
			_m_read_cs(vcpu.eip+1, 1),  _m_read_cs(vcpu.eip+2, 1),
			_m_read_cs(vcpu.eip+3, 1),  _m_read_cs(vcpu.eip+4, 1));
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
		_chk(_d_modrm(1, 1, 0x01));
		_chk(_a_add(vcpuins.rrm, vcpuins.rr, 8));
	} else {
		vcpu.ip++;
		GetModRegRM(8, 8);
		_a_add(vcpuins.rrm, vcpuins.rr, 8);
	}
	_ce;
}
done ADD_RM16_R16()
{
	_cb("ADD_RM16_R16");
	i386(0x01) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0x01));
		_chk(_a_add(vcpuins.rrm, vcpuins.rr, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		GetModRegRM(16, 16);
		_a_add(vcpuins.rrm, vcpuins.rr, 16);
	}
	_ce;
}
done ADD_R8_RM8()
{
	_cb("ADD_R8_RM8");
	i386(0x02) {
		_adv;
		_chk(_d_modrm(1, 1, 0x00));
		_chk(_a_add(vcpuins.rr, vcpuins.rrm, 8));
	} else {
		vcpu.ip++;
		GetModRegRM(8, 8);
		_a_add(vcpuins.rr, vcpuins.rrm, 8);
	}
	_ce;
}
done ADD_R16_RM16()
{
	_cb("ADD_R16_RM16");
	i386(0x03) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0x00));
		_chk(_a_add(vcpuins.rr, vcpuins.rrm, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		GetModRegRM(16, 16);
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
		GetImm(8);
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
		case 2:
			_chk(_d_imm(2));
			_chk(_a_add((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16));
			break;
		case 4:
			_chk(_d_imm(4));
			_chk(_a_adc((t_vaddrcc)&vcpu.eax, vcpuins.rimm, 32));
			break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		GetImm(16);
		_a_add((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16);
	}
	_ce;
}
void PUSH_ES()
{
	vcpu.eip++;
	PUSH((void *)&vcpu.es.selector,16);
}
void POP_ES()
{
	vcpu.eip++;
	POP((void *)&vcpu.es.selector,16);
}
void OR_RM8_R8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	OR((void *)vcpuins.rrm,(void *)vcpuins.rr,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  OR_RM8_R8\n");
}
void OR_RM16_R16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	OR((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  OR_RM16_R16\n");
}
void OR_R8_RM8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	OR((void *)vcpuins.rr,(void *)vcpuins.rrm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  OR_R8_RM8\n");
}
void OR_R16_RM16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	OR((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  OR_R16_RM16\n");
}
void OR_AL_I8()
{
	vcpu.eip++;
	GetImm(8);
	OR((void *)&vcpu.al,(void *)vcpuins.rimm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  OR_AL_I8\n");
}
void OR_AX_I16()
{
	vcpu.eip++;
	GetImm(16);
	OR((void *)&vcpu.ax,(void *)vcpuins.rimm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  OR_AX_I16\n");
}
void PUSH_CS()
{
	vcpu.eip++;
	PUSH((void *)&vcpu.cs.selector,16);
}
void POP_CS()
{
	vcpu.ip++;
	POP((void *)&vcpu.cs.selector,16);
}
done INS_0F()
{
	t_nubit8 opcode = 0x00;
	_cb("INS_0F");
	i386(0x0f) {
		_adv;
		_chk(opcode = (t_nubit8)_m_read_cs(vcpu.eip, 1));
		_chk(ExecFun(vcpuins.table_0f[opcode]));
	} else
		POP_CS();
	_ce;
}
done ADC_RM8_R8()
{
	_cb("ADC_RM8_R8");
	i386(0x10) {
		_adv;
		_chk(_d_modrm(1, 1, 0x01));
		_chk(_a_adc(vcpuins.rrm, vcpuins.rr, 8));
	} else {
		vcpu.ip++;
		GetModRegRM(8, 8);
		_a_adc(vcpuins.rrm, vcpuins.rr, 8); 
	}
	_ce;
}
done ADC_RM16_R16()
{
	_cb("ADC_RM16_R16");
	i386(0x11) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0x01));
		_chk(_a_adc(vcpuins.rrm, vcpuins.rr, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		GetModRegRM(16, 16);
		_a_adc(vcpuins.rrm, vcpuins.rr, 16);
	}
	_ce;
}
done ADC_R8_RM8()
{
	_cb("ADC_R8_RM8");
	i386(0x12) {
		_adv;
		_chk(_d_modrm(1, 1, 0x00));
		_chk(_a_adc(vcpuins.rr, vcpuins.rrm, 8));
	} else {
		vcpu.ip++;
		GetModRegRM(8, 8);
		_a_adc(vcpuins.rr, vcpuins.rrm, 8);
	}
	_ce;
}
done ADC_R16_RM16()
{
	_cb("ADC_R16_RM16");
	i386(0x13) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0x00));
		_chk(_a_adc(vcpuins.rr, vcpuins.rrm, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		GetModRegRM(16, 16);
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
		GetImm(8);
		_a_adc((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8);
	}
	_ce;
}
done ADC_AX_I16()
{
	_cb("ADC_AX_I16");
	i386(0x15) {
		_adv;
		switch (_GetOperandSize) {
		case 2:
			_chk(_d_imm(2));
			_chk(_a_adc((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16));
			break;
		case 4:
			_chk(_d_imm(4));
			_chk(_a_adc((t_vaddrcc)&vcpu.eax, vcpuins.rimm, 32));
			break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		GetImm(16);
		_a_adc((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16);
	}
	_ce;
}
void PUSH_SS()
{
	vcpu.eip++;
	PUSH((void *)&vcpu.ss.selector,16);
}
void POP_SS()
{
	vcpu.eip++;
	POP((void *)&vcpu.ss.selector,16);
}
void SBB_RM8_R8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	SBB((void *)vcpuins.rrm,(void *)vcpuins.rr,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SBB_RM8_R8\n");
}
void SBB_RM16_R16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	SBB((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SBB_RM16_R16\n");
}
void SBB_R8_RM8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	SBB((void *)vcpuins.rr,(void *)vcpuins.rrm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SBB_R8_RM8\n");
}
void SBB_R16_RM16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	SBB((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SBB_R16_RM16\n");
}
void SBB_AL_I8()
{
	vcpu.eip++;
	GetImm(8);
	SBB((void *)&vcpu.al,(void *)vcpuins.rimm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SBB_AL_I8\n");
}
void SBB_AX_I16()
{
	vcpu.eip++;
	GetImm(16);
	SBB((void *)&vcpu.ax,(void *)vcpuins.rimm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SBB_AX_I16\n");
}
void PUSH_DS()
{
	vcpu.eip++;
	PUSH((void *)&vcpu.ds.selector,16);
}
void POP_DS()
{
	vcpu.eip++;
	POP((void *)&vcpu.ds.selector,16);
}
done AND_RM8_R8()
{
	_cb("AND_RM8_R8");
	i386(0x20) {
		_adv;
		_chk(_d_modrm(1, 1, 0x01));
		_chk(_a_and(vcpuins.rrm, vcpuins.rr, 8));
	} else {
		vcpu.ip++;
		GetModRegRM(8,8);
		_a_and(vcpuins.rrm, vcpuins.rr, 8);
	}
	_ce;
}
done AND_RM16_R16()
{
	_cb("AND_RM16_R16");
	i386(0x21) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0x01));
		_chk(_a_and(vcpuins.rrm, vcpuins.rr, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		GetModRegRM(16, 16);
		_a_and(vcpuins.rrm, vcpuins.rr, 16);
	}
	_ce;
}
done AND_R8_RM8()
{
	_cb("AND_R8_RM8");
	i386(0x22) {
		_adv;
		_chk(_d_modrm(1, 1, 0x00));
		_chk(_a_and(vcpuins.rr, vcpuins.rrm, 8));
	} else {
		vcpu.ip++;
		GetModRegRM(8, 8);
		_a_and(vcpuins.rr, vcpuins.rrm, 8);
	}
	_ce;
}
done AND_R16_RM16()
{
	_cb("AND_R16_RM16");
	i386(0x23) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0x00));
		_chk(_a_and(vcpuins.rr, vcpuins.rrm, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		GetModRegRM(16, 16);
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
		GetImm(8);
		_a_and((t_vaddrcc)&vcpu.al, vcpuins.rimm, 8);
	}
	_ce;
}
done AND_AX_I16()
{
	_cb("AND_AX_I16");
	i386(0x25) {
		_adv;
		switch (_GetOperandSize) {
		case 2:
			_chk(_d_imm(2));
			_chk(_a_and((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16));
			break;
		case 4:
			_chk(_d_imm(4));
			_chk(_a_and((t_vaddrcc)&vcpu.eax, vcpuins.rimm, 32));
			break;
		default:
			CaseError("ADC_AX_I16::OperandSize");return;
		}
	} else {
		vcpu.ip++;
		GetImm(16);
		_a_and((t_vaddrcc)&vcpu.ax, vcpuins.rimm, 16);
	}
	_ce;
}
void ES()
{
	vcpu.eip++;
	vcpuins.roverds = &vcpu.es;
	vcpuins.roverss = &vcpu.es;
}
void DAA()
{
	t_nubit8 oldAL = vcpu.al;
	t_nubit8 newAL = vcpu.al + 0x06;
	vcpu.eip++;
	if(((vcpu.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
		vcpu.al = newAL;
		MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,_GetEFLAGS_CF || ((newAL < oldAL) || (newAL < 0x06)));
		bugfix(19) _SetEFLAGS_AF;
		else ;
	} else _ClrEFLAGS_AF;
	if(((vcpu.al & 0xf0) > 0x90) || _GetEFLAGS_CF) {
		vcpu.al += 0x60;
		_SetEFLAGS_CF;
	} else _ClrEFLAGS_CF;
	vcpuins.bit = 8;
	vcpuins.result = (t_nubitcc)vcpu.al;
	_a_SetFlags(DAA_FLAG);
}
void SUB_RM8_R8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	SUB((void *)vcpuins.rrm,(void *)vcpuins.rr,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SUB_RM8_R8\n");
}
void SUB_RM16_R16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	SUB((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SUB_RM16_R16\n");
}
void SUB_R8_RM8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	SUB((void *)vcpuins.rr,(void *)vcpuins.rrm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SUB_R8_RM8\n");
}
void SUB_R16_RM16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	SUB((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SUB_R16_RM16\n");
}
void SUB_AL_I8()
{
	vcpu.eip++;
	GetImm(8);
	SUB((void *)&vcpu.al,(void *)vcpuins.rimm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SUB_AL_I8\n");
}
void SUB_AX_I16()
{
	vcpu.eip++;
	GetImm(16);
	SUB((void *)&vcpu.ax,(void *)vcpuins.rimm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SUB_AX_I16\n");
}
void CS()
{
	vcpu.eip++;
	vcpuins.roverds = &vcpu.cs;
	vcpuins.roverss = &vcpu.cs;
}
void DAS()
{
	t_nubit8 oldAL = vcpu.al;
	vcpu.eip++;
	if(((vcpu.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
		vcpu.al -= 0x06;
		MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,_GetEFLAGS_CF || (oldAL < 0x06));
		_SetEFLAGS_AF;
	} else _ClrEFLAGS_AF;
	if((vcpu.al > 0x9f) || _GetEFLAGS_CF) {
		vcpu.al -= 0x60;
		_SetEFLAGS_CF;
	} else _ClrEFLAGS_CF;
	vcpuins.bit = 8;
	vcpuins.result = (t_nubitcc)vcpu.al;
	_a_SetFlags(DAS_FLAG);
}
void XOR_RM8_R8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	XOR((void *)vcpuins.rrm,(void *)vcpuins.rr,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XOR_RM8_R8\n");
}
void XOR_RM16_R16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	XOR((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XOR_RM16_R16\n");
}
void XOR_R8_RM8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	XOR((void *)vcpuins.rr,(void *)vcpuins.rrm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XOR_R8_RM8\n");
}
void XOR_R16_RM16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	XOR((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XOR_R16_RM16\n");
}
void XOR_AL_I8()
{
	vcpu.eip++;
	GetImm(8);
	XOR((void *)&vcpu.al,(void *)vcpuins.rimm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XOR_AL_I8\n");
}
void XOR_AX_I16()
{
	vcpu.eip++;
	GetImm(16);
	XOR((void *)&vcpu.ax,(void *)vcpuins.rimm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XOR_AX_I16\n");
}
void SS()
{
	vcpu.eip++;
	vcpuins.roverds = &vcpu.ss;
	vcpuins.roverss = &vcpu.ss;
}
done AAA()
{
	_cb("AAA");
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
	_ce;
}
void CMP_RM8_R8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	CMP((void *)vcpuins.rrm,(void *)vcpuins.rr,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  CMP_RM8_R8\n");
}
void CMP_RM16_R16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	CMP((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  CMP_RM16_R16\n");
}
void CMP_R8_RM8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	CMP((void *)vcpuins.rr,(void *)vcpuins.rrm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  CMP_R8_RM8\n");
}
void CMP_R16_RM16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	CMP((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  CMP_R16_RM16\n");
}
void CMP_AL_I8()
{
	vcpu.eip++;
	GetImm(8);
	CMP((void *)&vcpu.al,(void *)vcpuins.rimm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  CMP_AL_I8\n");
}
void CMP_AX_I16()
{
	vcpu.eip++;
	GetImm(16);
	CMP((void *)&vcpu.ax,(void *)vcpuins.rimm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  CMP_AX_I16\n");
}
void DS()
{
	vcpu.eip++;
	vcpuins.roverds = &vcpu.ds;
	vcpuins.roverss = &vcpu.ds;
}
done AAS()
{
	_cb("AAS");
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
	_ce;
}
void INC_AX()
{
	vcpu.eip++;
	INC((void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  INC_AX\n");
}
void INC_CX()
{
	vcpu.eip++;
	INC((void *)&vcpu.cx,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  INC_CX\n");
}
void INC_DX()
{
	vcpu.eip++;
	INC((void *)&vcpu.dx,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  INC_DX\n");
}
void INC_BX()
{
	vcpu.eip++;
	INC((void *)&vcpu.bx,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  INC_BX\n");
}
void INC_SP()
{
	vcpu.eip++;
	INC((void *)&vcpu.sp,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  INC_SP\n");
}
void INC_BP()
{
	vcpu.eip++;
	INC((void *)&vcpu.bp,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  INC_BP\n");
}
void INC_SI()
{
	vcpu.eip++;
	INC((void *)&vcpu.si,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  INC_SI\n");
}
void INC_DI()
{
	vcpu.eip++;
	INC((void *)&vcpu.di,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  INC_DI\n");
}
void DEC_AX()
{
	vcpu.eip++;
	DEC((void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  DEC_AX\n");
}
void DEC_CX()
{
	vcpu.eip++;
	DEC((void *)&vcpu.cx,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  DEC_CX\n");
}
void DEC_DX()
{
	vcpu.eip++;
	DEC((void *)&vcpu.dx,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  DEC_DX\n");
}
void DEC_BX()
{
	vcpu.eip++;
	DEC((void *)&vcpu.bx,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  DEC_BX\n");
}
void DEC_SP()
{
	vcpu.eip++;
	DEC((void *)&vcpu.sp,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  DEC_SP\n");
}
void DEC_BP()
{
	vcpu.eip++;
	DEC((void *)&vcpu.bp,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  DEC_BP\n");
}
void DEC_SI()
{
	vcpu.eip++;
	DEC((void *)&vcpu.si,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  DEC_SI\n");
}
void DEC_DI()
{
	vcpu.eip++;
	DEC((void *)&vcpu.di,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  DEC_DI\n");
}
void PUSH_AX()
{
	vcpu.eip++;
	PUSH((void *)&vcpu.ax,16);
}
void PUSH_CX()
{
	vcpu.eip++;
	PUSH((void *)&vcpu.cx,16);
}
void PUSH_DX()
{
	vcpu.eip++;
	PUSH((void *)&vcpu.dx,16);
}
void PUSH_BX()
{
	vcpu.eip++;
	PUSH((void *)&vcpu.bx,16);
}
void PUSH_SP()
{
	vcpu.eip++;
	PUSH((void *)&vcpu.sp,16);
}
void PUSH_BP()
{
	vcpu.eip++;
	PUSH((void *)&vcpu.bp,16);
}
void PUSH_SI()
{
	vcpu.eip++;
	PUSH((void *)&vcpu.si,16);
}
void PUSH_DI()
{
	vcpu.eip++;
	PUSH((void *)&vcpu.di,16);
}
void POP_AX()
{
	vcpu.eip++;
	POP((void *)&vcpu.ax,16);
}
void POP_CX()
{
	vcpu.eip++;
	POP((void *)&vcpu.cx,16);
}
void POP_DX()
{
	vcpu.eip++;
	POP((void *)&vcpu.dx,16);
}
void POP_BX()
{
	vcpu.eip++;
	POP((void *)&vcpu.bx,16);
}
void POP_SP()
{
	vcpu.eip++;
	POP((void *)&vcpu.sp,16);
}
void POP_BP()
{
	vcpu.eip++;
	POP((void *)&vcpu.bp,16);
}
void POP_SI()
{
	vcpu.eip++;
	POP((void *)&vcpu.si,16);
}
void POP_DI()
{
	vcpu.eip++;
	POP((void *)&vcpu.di,16);
}
done BOUND_R16_M16_16()
{
	t_nsbit16 a16,l16,u16;
	t_nsbit32 a32,l32,u32;
	_cb("BOUND_R16_M16_16");
	i386(0x62) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize * 2, 0x00));
		if (!vcpuins.flagmem)
			_chk(_SetExcept_UD(0));
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
done ARPL_RM16_R16()
{
	t_nubit8 drpl, srpl;
	_cb("ARPL_RM16_R16");
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
			_chk(_SetExcept_UD(0));
	} else
		UndefinedOpcode();
	_ce;
}
void FS()
{
	vcpu.eip++;
	vcpuins.roverds = &vcpu.fs;
	vcpuins.roverss = &vcpu.fs;
}
void GS()
{
	vcpu.eip++;
	vcpuins.roverds = &vcpu.gs;
	vcpuins.roverss = &vcpu.gs;
}
void OprSize()
{
	vcpu.eip++;
	vcpuins.prefix_oprsize = 0x01;
}
void AddrSize()
{
	vcpu.eip++;
	vcpuins.prefix_addrsize = 0x01;
}
void JO()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.rimm, _GetOF, 8);
}
void JNO()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.rimm, !_GetOF, 8);
}
void JC()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.rimm, _GetEFLAGS_CF, 8);
}
void JNC()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.rimm, !_GetEFLAGS_CF, 8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  JNC\n");
}
void JZ()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.rimm, _GetZF, 8);
}
void JNZ()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.rimm, !_GetZF, 8);
}
void JBE()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.rimm, (_GetEFLAGS_CF ||
		_GetZF), 8);
}
void JA()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.rimm, (!_GetEFLAGS_CF &&
		!_GetZF), 8);
}
void JS()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.rimm, _GetSF, 8);
}
void JNS()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.rimm, !_GetSF, 8);
}
void JP()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.rimm, _GetPF, 8);
}
void JNP()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.rimm, !_GetPF, 8);
}
void JL()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.rimm, (_GetSF !=
		_GetOF), 8);
}
void JNL()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.rimm, (_GetSF ==
		_GetOF), 8);
}
void JLE()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.rimm, (_GetZF ||
		(_GetSF !=
		_GetOF)), 8);
}
void JG()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.rimm, (!_GetZF &&
		(_GetSF ==
		_GetOF)), 8);
}
void INS_80()
{
	vcpu.ip++;
	GetModRegRM(0, 8);
	GetImm(8);
	switch (vcpuins.rr) {
	case 0: _a_add(vcpuins.rrm, vcpuins.rimm, 8); break;
	case 1: OR ((void *)vcpuins.rrm, (void *)vcpuins.rimm, 8); break;
	case 2: _a_adc(vcpuins.rrm, vcpuins.rimm, 8); break;
	case 3: SBB((void *)vcpuins.rrm, (void *)vcpuins.rimm, 8); break;
	case 4: _a_and(vcpuins.rrm, vcpuins.rimm, 8); break;
	case 5: SUB((void *)vcpuins.rrm, (void *)vcpuins.rimm, 8); break;
	case 6: XOR((void *)vcpuins.rrm, (void *)vcpuins.rimm, 8); break;
	case 7: CMP((void *)vcpuins.rrm, (void *)vcpuins.rimm, 8); break;
	default:CaseError("INS_80::vcpuins.rr");break;}
}
void INS_81()
{
	vcpu.ip++;
	GetModRegRM(0, 16);
	GetImm(16);
	switch(vcpuins.rr) {
	case 0: _a_add(vcpuins.rrm, vcpuins.rimm, 16); break;
	case 1: OR ((void *)vcpuins.rrm, (void *)vcpuins.rimm, 16); break;
	case 2: _a_adc(vcpuins.rrm, vcpuins.rimm, 16); break;
	case 3: SBB((void *)vcpuins.rrm, (void *)vcpuins.rimm, 16); break;
	case 4: _a_and(vcpuins.rrm, vcpuins.rimm, 16); break;
	case 5: SUB((void *)vcpuins.rrm, (void *)vcpuins.rimm, 16); break;
	case 6: XOR((void *)vcpuins.rrm, (void *)vcpuins.rimm, 16); break;
	case 7: CMP((void *)vcpuins.rrm, (void *)vcpuins.rimm, 16); break;
	default:CaseError("INS_81::vcpuins.rr");break;}
}
void INS_83()
{
	t_nubit8 bit = (_GetOperandSize + 8) >> 1;
	vcpu.ip++;
	GetModRegRM(0, 16);
	GetImm(8);
	switch(vcpuins.rr) {
	case 0: _a_add(vcpuins.rrm, vcpuins.rimm, 12); break;
	case 1: OR ((void *)vcpuins.rrm, (void *)vcpuins.rimm, 12); break;
	case 2: _a_adc(vcpuins.rrm, vcpuins.rimm, 12); break;
	case 3: SBB((void *)vcpuins.rrm, (void *)vcpuins.rimm, 12); break;
	case 4: _a_and(vcpuins.rrm, vcpuins.rimm, 12); break;
	case 5: SUB((void *)vcpuins.rrm, (void *)vcpuins.rimm, 12); break;
	case 6: XOR((void *)vcpuins.rrm, (void *)vcpuins.rimm, 12); break;
	case 7: CMP((void *)vcpuins.rrm, (void *)vcpuins.rimm, 12); break;
	default:CaseError("INS_83::vcpuins.rr");break;}
}
void TEST_RM8_R8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	TEST((void *)vcpuins.rrm,(void *)vcpuins.rr,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  TEST_RM8_R8\n");
}
void TEST_RM16_R16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	TEST((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  TEST_RM16_R16\n");
}
void XCHG_R8_RM8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	XCHG((void *)vcpuins.rr,(void *)vcpuins.rrm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XCHG_R8_RM8\n");
}
void XCHG_R16_RM16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	XCHG((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XCHG_R16_RM16\n");
}
void MOV_RM8_R8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	MOV((void *)vcpuins.rrm,(void *)vcpuins.rr,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOV_RM8_R8\n");
}
void MOV_RM16_R16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOV_RM16_R16\n");
}
void MOV_R8_RM8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	MOV((void *)vcpuins.rr,(void *)vcpuins.rrm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOV_R8_RM8\n");
}
void MOV_R16_RM16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOV_R16_RM16\n");
}
void MOV_RM16_SEG()
{
	vcpu.eip++;
	GetModRegRM(4,16);
	MOV((void *)vcpuins.rrm,(void *)vcpuins.rr,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOV_RM16_SEG\n");
}
void LEA_R16_M16()
{
	vcpu.eip++;
	GetModRegRMEA();
	d_nubit16(vcpuins.rr) = vcpuins.rrm & 0xffff;
}
void MOV_SEG_RM16()
{
	vcpu.eip++;
	GetModRegRM(4,16);
	MOV((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOV_SEG_RM16\n");
}
void POP_RM16()
{
	vcpu.eip++;
	GetModRegRM(0,16);
	switch(vcpuins.rr) {
	case 0:
		bugfix(14) POP((void *)vcpuins.rrm,16);
		else POP((void *)vcpuins.rrm,16);
		break;
	default:CaseError("POP_RM16::vcpuins.rr");break;}
}
void NOP()
{
	vcpu.eip++;
}
void XCHG_CX_AX()
{
	vcpu.eip++;
	XCHG((void *)&vcpu.cx,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XCHG_CX_AX\n");
}
void XCHG_DX_AX()
{
	vcpu.eip++;
	XCHG((void *)&vcpu.dx,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XCHG_DX_AX\n");
}
void XCHG_BX_AX()
{
	vcpu.eip++;
	XCHG((void *)&vcpu.bx,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XCHG_BX_AX\n");
}
void XCHG_SP_AX()
{
	vcpu.eip++;
	XCHG((void *)&vcpu.sp,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XCHG_SP_AX\n");
}
void XCHG_BP_AX()
{
	vcpu.eip++;
	XCHG((void *)&vcpu.bp,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XCHG_BP_AX\n");
}
void XCHG_SI_AX()
{
	vcpu.eip++;
	XCHG((void *)&vcpu.si,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XCHG_SI_AX\n");
}
void XCHG_DI_AX()
{
	vcpu.eip++;
	XCHG((void *)&vcpu.di,(void *)&vcpu.ax,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XCHG_DI_AX\n");
}
void CBW()
{
	vcpu.eip++;
	vcpu.ax = (t_nsbit8)vcpu.al;
}
void CWD()
{
	vcpu.eip++;
	if (vcpu.ax & 0x8000) vcpu.dx = 0xffff;
	else vcpu.dx = 0x0000;
}
void CALL_PTR16_16()
{
	t_nubit16 newcs;
	t_nubit32 neweip;/*
	i386(0x9a) {
		vcpu.eip++;
		switch (_GetOperandSize) {
		case 2:
			_d_imm(2);
			neweip = d_nubit16(vcpuins.rimm);
			_d_imm(2);
			newcs = d_nubit16(vcpuins.rimm);
			_Call_Far(newcs, neweip, 16);
			break;
		case 4:
			_d_imm(4);
			neweip = d_nubit32(vcpuins.rimm);
			_d_imm(2);
			newcs = d_nubit16(vcpuins.rimm);
			_Call_Far(newcs, neweip, 32);
			break;
		default:
			CaseError("CALL_PTR16_16::OperandSize");
			return;
		}
	} else */{
		vcpu.eip++;
		GetImm(16);
		neweip = d_nubit16(vcpuins.rimm);
		GetImm(16);
		newcs = d_nubit16(vcpuins.rimm);
		PUSH((void *)&vcpu.cs.selector,16);
		PUSH((void *)&vcpu.eip,16);
		vcpu.ip = GetMax16(neweip);
		vcpu.cs.selector = newcs;
	}
}
todo WAIT()
{
	/* not implemented */
	_adv;
}
void PUSHF()
{
	vcpu.eip++;
	PUSH((void *)&vcpu.eflags,16);
}
void POPF()
{
	vcpu.eip++;
	POP((void *)&vcpu.eflags,16);
}
void SAHF()
{
	vcpu.eip++;
	d_nubit8(&vcpu.eflags) = vcpu.ah;
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SAHF\n");
}
void LAHF()
{
	vcpu.eip++;
//	vapiPrint("1:LAHF:%4X\n",vcpu.eflags);
	vcpu.ah = d_nubit8(&vcpu.eflags);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  LAHF\n");
}
void MOV_AL_M8()
{
	vcpu.eip++;
	GetMem();
	MOV((void *)&vcpu.al,(void *)vcpuins.rrm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOV_AL_M8\n");
}
void MOV_AX_M16()
{
	vcpu.eip++;
	GetMem();
	MOV((void *)&vcpu.ax,(void *)vcpuins.rrm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOV_AX_M16\n");
}
void MOV_M8_AL()
{
	vcpu.eip++;
	GetMem();
	MOV((void *)vcpuins.rrm,(void *)&vcpu.al,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOV_M8_AL\n");
}
void MOV_M16_AX()
{
	vcpu.eip++;
	GetMem();
	MOV((void *)vcpuins.rrm,(void *)&vcpu.ax,16);
}
void MOVSB()
{
	vcpu.eip++;
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) MOVS(8);
	else {
		if (vcpu.cx) {
			MOVS(8);
			vcpu.cx--;
		}
		if (vcpu.cx) vcpuins.flaginsloop = 0x01;
	}
}
void MOVSW()
{
	vcpu.eip++;
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) MOVS(16);
	else {
		if (vcpu.cx) {
			MOVS(16);
			vcpu.cx--;
		}
		if (vcpu.cx) vcpuins.flaginsloop = 0x01;
	}
}
void CMPSB()
{
	vcpu.eip++;
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) CMPS(8);
	else {
		if (vcpu.cx) {
			CMPS(8);
			vcpu.cx--;
		}
		if (vcpu.cx && !(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetZF) && !(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetZF))
			 vcpuins.flaginsloop = 0x01;
	}
}
void CMPSW()
{
	vcpu.eip++;
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) CMPS(16);
	else {
		if (vcpu.cx) {
			CMPS(16);
			vcpu.cx--;
		}
		if (vcpu.cx && !(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetZF) && !(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetZF))
			 vcpuins.flaginsloop = 0x01;
	}
}
void TEST_AL_I8()
{
	vcpu.eip++;
	GetImm(8);
	TEST((void *)&vcpu.al,(void *)vcpuins.rimm,8);
}
void TEST_AX_I16()
{
	vcpu.eip++;
	GetImm(16);
	TEST((void *)&vcpu.ax,(void *)vcpuins.rimm,16);
}
void STOSB()
{
	vcpu.eip++;
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) STOS(8);
	else {
		if (vcpu.cx) {
			STOS(8);
			vcpu.cx--;
		}
		if (vcpu.cx) vcpuins.flaginsloop = 0x01;
	}
}
void STOSW()
{
	vcpu.eip++;
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) STOS(16);
	else {
		if (vcpu.cx) {
			STOS(16);
			vcpu.cx--;
		}
		if (vcpu.cx) vcpuins.flaginsloop = 0x01;
	}
}
void LODSB()
{
	vcpu.eip++;
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) LODS(8);
	else {
		if (vcpu.cx) {
			LODS(8);
			vcpu.cx--;
		}
		if (vcpu.cx) vcpuins.flaginsloop = 0x01;
	}
}
void LODSW()
{
	vcpu.eip++;
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) LODS(16);
	else {
		if (vcpu.cx) {
			LODS(16);
			vcpu.cx--;
		}
		if (vcpu.cx) vcpuins.flaginsloop = 0x01;
	}
}
void SCASB()
{
	vcpu.eip++;
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) SCAS(8);
	else {
		if (vcpu.cx) {
			SCAS(8);
			vcpu.cx--;
		}
		if (vcpu.cx && !(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetZF) && !(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetZF))
			 vcpuins.flaginsloop = 0x01;
	}
}
void SCASW()
{
	vcpu.eip++;
	if(vcpuins.prefix_rep == PREFIX_REP_NONE) SCAS(16);
	else {
		if (vcpu.cx) {
			SCAS(16);
			vcpu.cx--;
		}
		if (vcpu.cx && !(vcpuins.prefix_rep == PREFIX_REP_REPZ && !_GetZF) && !(vcpuins.prefix_rep == PREFIX_REP_REPZNZ && _GetZF))
			 vcpuins.flaginsloop = 0x01;
	}
}
void MOV_AL_I8()
{
	vcpu.eip++;
	GetImm(8);
	MOV((void *)&vcpu.al,(void *)vcpuins.rimm,8);
}
void MOV_CL_I8()
{
	vcpu.eip++;
	GetImm(8);
	MOV((void *)&vcpu.cl,(void *)vcpuins.rimm,8);
}
void MOV_DL_I8()
{
	vcpu.eip++;
	GetImm(8);
	MOV((void *)&vcpu.dl,(void *)vcpuins.rimm,8);
}
void MOV_BL_I8()
{
	vcpu.eip++;
	GetImm(8);
	MOV((void *)&vcpu.bl,(void *)vcpuins.rimm,8);
}
void MOV_AH_I8()
{
	vcpu.eip++;
	GetImm(8);
	MOV((void *)&vcpu.ah,(void *)vcpuins.rimm,8);
}
void MOV_CH_I8()
{
	vcpu.eip++;
	GetImm(8);
	MOV((void *)&vcpu.ch,(void *)vcpuins.rimm,8);
}
void MOV_DH_I8()
{
	vcpu.eip++;
	GetImm(8);
	MOV((void *)&vcpu.dh,(void *)vcpuins.rimm,8);
}
void MOV_BH_I8()
{
	vcpu.eip++;
	GetImm(8);
	MOV((void *)&vcpu.bh,(void *)vcpuins.rimm,8);
}
void MOV_AX_I16()
{
	vcpu.eip++;
	GetImm(16);
	MOV((void *)&vcpu.ax,(void *)vcpuins.rimm,16);
}
void MOV_CX_I16()
{
	vcpu.eip++;
	GetImm(16);
	MOV((void *)&vcpu.cx,(void *)vcpuins.rimm,16);
}
void MOV_DX_I16()
{
	vcpu.eip++;
	GetImm(16);
	MOV((void *)&vcpu.dx,(void *)vcpuins.rimm,16);
}
void MOV_BX_I16()
{
	vcpu.eip++;
	GetImm(16);
	MOV((void *)&vcpu.bx,(void *)vcpuins.rimm,16);
}
void MOV_SP_I16()
{
	vcpu.eip++;
	GetImm(16);
	MOV((void *)&vcpu.sp,(void *)vcpuins.rimm,16);
}
void MOV_BP_I16()
{
	vcpu.eip++;
	GetImm(16);
	MOV((void *)&vcpu.bp,(void *)vcpuins.rimm,16);
}
void MOV_SI_I16()
{
	vcpu.eip++;
	GetImm(16);
	MOV((void *)&vcpu.si,(void *)vcpuins.rimm,16);
}
void MOV_DI_I16()
{
	vcpu.eip++;
	GetImm(16);
	MOV((void *)&vcpu.di,(void *)vcpuins.rimm,16);
}
void INS_C0()
{
	vcpu.eip++;
	GetModRegRM(0,8);
	GetImm(8);
	switch(vcpuins.rr) {
	case 0:	ROL((void *)vcpuins.rrm,(void *)vcpuins.rimm,8);break;
	case 1:	ROR((void *)vcpuins.rrm,(void *)vcpuins.rimm,8);break;
	case 2:	RCL((void *)vcpuins.rrm,(void *)vcpuins.rimm,8);break;
	case 3:	RCR((void *)vcpuins.rrm,(void *)vcpuins.rimm,8);break;
	case 4:	SHL((void *)vcpuins.rrm,(void *)vcpuins.rimm,8);break;
	case 5:	SHR((void *)vcpuins.rrm,(void *)vcpuins.rimm,8);break;
	case 6:	SAL((void *)vcpuins.rrm,(void *)vcpuins.rimm,8);break;
	case 7:	SAR((void *)vcpuins.rrm,(void *)vcpuins.rimm,8);break;
	default:CaseError("INS_C0::vcpuins.rr");break;}
}
void INS_C1()
{
	vcpu.eip++;
	GetModRegRM(0,16);
	GetImm(8);
	switch(vcpuins.rr) {
	case 0:	ROL((void *)vcpuins.rrm,(void *)vcpuins.rimm,16);break;
	case 1:	ROR((void *)vcpuins.rrm,(void *)vcpuins.rimm,16);break;
	case 2:	RCL((void *)vcpuins.rrm,(void *)vcpuins.rimm,16);break;
	case 3:	RCR((void *)vcpuins.rrm,(void *)vcpuins.rimm,16);break;
	case 4:	SHL((void *)vcpuins.rrm,(void *)vcpuins.rimm,16);break;
	case 5:	SHR((void *)vcpuins.rrm,(void *)vcpuins.rimm,16);break;
	case 6:	SAL((void *)vcpuins.rrm,(void *)vcpuins.rimm,16);break;
	case 7:	SAR((void *)vcpuins.rrm,(void *)vcpuins.rimm,16);break;
	default:CaseError("INS_C1::vcpuins.rr");break;}
}
void RET_I16()
{
	t_nubit16 addsp;
	vcpu.eip++;
	bugfix(15) {
		GetImm(16);
		addsp = d_nubit16(vcpuins.rimm);
	} else {
		GetImm(8);
		addsp = d_nubit8(vcpuins.rimm);
	}
	POP((void *)&vcpu.eip,16);
	vcpu.sp += addsp;
}
void RET()
{
	vcpu.eip++;
	POP((void *)&vcpu.eip,16);
}
void LES_R16_M16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
	MOV((void *)&vcpu.es.selector,(void *)(vcpuins.rrm+2),16);
}
void LDS_R16_M16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.rr,(void *)vcpuins.rrm,16);
	MOV((void *)&vcpu.ds.selector,(void *)(vcpuins.rrm+2),16);
}
void MOV_M8_I8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	GetImm(8);
	MOV((void *)vcpuins.rrm,(void *)vcpuins.rimm,8);
}
void MOV_M16_I16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	GetImm(16);
	MOV((void *)vcpuins.rrm,(void *)vcpuins.rimm,16);
}
void RETF_I16()
{
	t_nubit16 addsp;
	vcpu.eip++;
	GetImm(16);
	addsp = d_nubit16(vcpuins.rimm);
	POP((void *)&vcpu.eip,16);
	POP((void *)&vcpu.cs.selector,16);
	vcpu.sp += addsp;
}
void RETF()
{
	POP((void *)&vcpu.eip,16);
	POP((void *)&vcpu.cs.selector,16);
}
void INT3()
{
	vcpu.eip++;
	INT(0x03);
}
void INT_I8()
{
	vcpu.eip++;
	GetImm(8);
	INT(d_nubit8(vcpuins.rimm));
}
void INTO()
{
	vcpu.eip++;
	if(_GetOF) INT(0x04);
}
void IRET()
{
	vcpu.eip++;
	POP((void *)&vcpu.eip,16);
	POP((void *)&vcpu.cs.selector,16);
	POP((void *)&vcpu.eflags,16);
}
void INS_D0()
{
	vcpu.eip++;
	GetModRegRM(0,8);
	switch(vcpuins.rr) {
	case 0:	ROL((void *)vcpuins.rrm,NULL,8);break;
	case 1:	ROR((void *)vcpuins.rrm,NULL,8);break;
	case 2:	RCL((void *)vcpuins.rrm,NULL,8);break;
	case 3:	RCR((void *)vcpuins.rrm,NULL,8);break;
	case 4:	SHL((void *)vcpuins.rrm,NULL,8);break;
	case 5:	SHR((void *)vcpuins.rrm,NULL,8);break;
	case 6:	SAL((void *)vcpuins.rrm,NULL,8);break;
	case 7:	SAR((void *)vcpuins.rrm,NULL,8);break;
	default:CaseError("INS_D0::vcpuins.rr");break;}
}
void INS_D1()
{
	vcpu.eip++;
	GetModRegRM(0,16);
	switch(vcpuins.rr) {
	case 0:	ROL((void *)vcpuins.rrm,NULL,16);break;
	case 1:	ROR((void *)vcpuins.rrm,NULL,16);break;
	case 2:	RCL((void *)vcpuins.rrm,NULL,16);break;
	case 3:	RCR((void *)vcpuins.rrm,NULL,16);break;
	case 4:	SHL((void *)vcpuins.rrm,NULL,16);break;
	case 5:	SHR((void *)vcpuins.rrm,NULL,16);break;
	case 6:	SAL((void *)vcpuins.rrm,NULL,16);break;
	case 7:	SAR((void *)vcpuins.rrm,NULL,16);break;
	default:CaseError("INS_D1::vcpuins.rr");break;}
}
void INS_D2()
{
	vcpu.eip++;
	GetModRegRM(0,8);
	switch(vcpuins.rr) {
	case 0:	ROL((void *)vcpuins.rrm,(void *)&vcpu.cl,8);break;
	case 1:	ROR((void *)vcpuins.rrm,(void *)&vcpu.cl,8);break;
	case 2:	RCL((void *)vcpuins.rrm,(void *)&vcpu.cl,8);break;
	case 3:	RCR((void *)vcpuins.rrm,(void *)&vcpu.cl,8);break;
	case 4:	SHL((void *)vcpuins.rrm,(void *)&vcpu.cl,8);break;
	case 5:	SHR((void *)vcpuins.rrm,(void *)&vcpu.cl,8);break;
	case 6:	SAL((void *)vcpuins.rrm,(void *)&vcpu.cl,8);break;
	case 7:	SAR((void *)vcpuins.rrm,(void *)&vcpu.cl,8);break;
	default:CaseError("INS_D2::vcpuins.rr");break;}
}
void INS_D3()
{
	vcpu.eip++;
	GetModRegRM(0,16);
	switch(vcpuins.rr) {
	case 0:	ROL((void *)vcpuins.rrm,(void *)&vcpu.cl,16);break;
	case 1:	ROR((void *)vcpuins.rrm,(void *)&vcpu.cl,16);break;
	case 2:	RCL((void *)vcpuins.rrm,(void *)&vcpu.cl,16);break;
	case 3:	RCR((void *)vcpuins.rrm,(void *)&vcpu.cl,16);break;
	case 4:	SHL((void *)vcpuins.rrm,(void *)&vcpu.cl,16);break;
	case 5:	SHR((void *)vcpuins.rrm,(void *)&vcpu.cl,16);break;
	case 6:	SAL((void *)vcpuins.rrm,(void *)&vcpu.cl,16);break;
	case 7:	SAR((void *)vcpuins.rrm,(void *)&vcpu.cl,16);break;
	default:CaseError("INS_D3::vcpuins.rr");break;}
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
		GetImm(8);
	}
	base = d_nubit8(vcpuins.rimm);
	vcpu.ah = vcpu.al / base;
	vcpu.al = vcpu.al % base;
	vcpuins.bit = 0x08;
	vcpuins.result = GetMax8(vcpu.al);
	_chk(_a_SetFlags(AAM_FLAG));
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
		GetImm(8);
	}
	base = d_nubit8(vcpuins.rimm);
	vcpu.al = GetMax8(vcpu.al + (vcpu.ah * base));
	vcpu.ah = 0x00;
	vcpuins.bit = 0x08;
	vcpuins.result = GetMax8(vcpu.al);
	_chk(_a_SetFlags(AAD_FLAG));
	_ce;
}
void XLAT()
{
	vcpu.eip++;
	vcpu.al = vramVarByte(vcpuins.roverds->selector,vcpu.bx+vcpu.al);
}
void LOOPNZ()
{
	t_nsbit8 rel8;
	vcpu.eip++;
	GetImm(8);
	bugfix(12) rel8 = d_nsbit8(vcpuins.rimm);
	else rel8 = d_nubit8(vcpuins.rimm);
	vcpu.cx--;
	if(vcpu.cx && !_GetZF) vcpu.eip += rel8;
}
void LOOPZ()
{
	t_nsbit8 rel8;
	vcpu.eip++;
	GetImm(8);
	bugfix(12) rel8 = d_nsbit8(vcpuins.rimm);
	else rel8 = d_nubit8(vcpuins.rimm);
	vcpu.cx--;
	if(vcpu.cx && _GetZF) vcpu.eip += rel8;
}
void LOOP()
{
	t_nsbit8 rel8;
	vcpu.eip++;
	GetImm(8);
	bugfix(12) rel8 = d_nsbit8(vcpuins.rimm);
	else rel8 = d_nubit8(vcpuins.rimm);
	vcpu.cx--;
	if(vcpu.cx) vcpu.eip += rel8;
}
void JCXZ_REL8()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void*)vcpuins.rimm,!vcpu.cx,8);
}
void IN_AL_I8()
{
	vcpu.eip++;
	GetImm(8);
//	vapiPrint("IN: %02X\n",d_nubit8(vcpuins.rimm));
	ExecFun(vport.in[d_nubit8(vcpuins.rimm)]);
	vcpu.al = vport.iobyte;
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  IN_AL_I8\n");
}
void IN_AX_I8()
{
	vcpu.eip++;
	GetImm(8);
//	vapiPrint("IN: %02X\n",d_nubit8(vcpuins.rimm));
	ExecFun(vport.in[d_nubit8(vcpuins.rimm)]);
	vcpu.ax = vport.ioword;
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  IN_AX_I8\n");
}
void OUT_I8_AL()
{
	vcpu.eip++;
	GetImm(8);
//	vapiPrint("OUT: %02X\n",d_nubit8(vcpuins.rimm));
	vport.iobyte = vcpu.al;
	ExecFun(vport.out[d_nubit8(vcpuins.rimm)]);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  OUT_I8_AL\n");
}
void OUT_I8_AX()
{
	vcpu.eip++;
	GetImm(8);
//	vapiPrint("OUT: %02X\n",d_nubit8(vcpuins.rimm));
	vport.ioword = vcpu.ax;
	ExecFun(vport.out[d_nubit8(vcpuins.rimm)]);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  OUT_I8_AX\n");
}
void CALL_REL16()
{
	t_nsbit16 rel16;
	vcpu.ip++;
	GetImm(16);
	rel16 = d_nsbit16(vcpuins.rimm);
	PUSH((void *)&vcpu.ip,16);
	vcpu.ip += rel16;
}
void JMP_REL16()
{
	t_nsbit16 rel16;
	vcpu.eip++;
	GetImm(16);
	rel16 = d_nsbit16(vcpuins.rimm);
	vcpu.eip += rel16;
	vcpu.eip &= 0x0000ffff;
}
void JMP_PTR16_16()
{
	t_nubit16 newip,newcs;
	vcpu.eip++;
	GetImm(16);
	newip = d_nubit16(vcpuins.rimm);
	GetImm(16);
	newcs = d_nubit16(vcpuins.rimm);
	vcpu.eip = newip;
	vcpu.cs.selector = newcs;
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  JMP_PTR16_16\n");
}
void JMP_REL8()
{
	t_nsbit8 rel8;
	vcpu.eip++;
	GetImm(8);
	rel8 = d_nsbit8(vcpuins.rimm);
	bugfix(9) vcpu.eip += rel8;
	else vcpu.eip += d_nubit8(vcpuins.rimm);
}
void IN_AL_DX()
{
	vcpu.eip++;
//	vapiPrint("IN: %04X\n",vcpu.dx);
	ExecFun(vport.in[vcpu.dx]);
	vcpu.al = vport.iobyte;
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  IN_AL_DX\n");
}
void IN_AX_DX()
{
	vcpu.eip++;
//	vapiPrint("IN: %04X\n",vcpu.dx);
	ExecFun(vport.in[vcpu.dx]);
	vcpu.ax = vport.ioword;
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  IN_AX_DX\n");
}
void OUT_DX_AL()
{
	vcpu.eip++;
	vport.iobyte = vcpu.al;
//	vapiPrint("OUT: %04X\n",vcpu.dx);
	ExecFun(vport.out[vcpu.dx]);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  OUT_DX_AL\n");
}
void OUT_DX_AX()
{
	vcpu.eip++;
//	vapiPrint("OUT: %04X\n",vcpu.dx);
	vport.ioword = vcpu.ax;
	ExecFun(vport.out[vcpu.dx]);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  OUT_DX_AX\n");
}
todo LOCK()
{
	/* Not Implemented */
	_adv;
}
void REPNZ()
{
	// CMPS,SCAS
	vcpu.eip++;
	vcpuins.prefix_rep = PREFIX_REP_REPZNZ;
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  REPNZ\n");
}
void REP()
{	// MOVS,LODS,STOS,CMPS,SCAS
	vcpu.eip++;
	vcpuins.prefix_rep = PREFIX_REP_REPZ;
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  REP\n");
}
todo HLT()
{
	/* Not Implemented */
	_adv;
}
void CMC()
{
	vcpu.eip++;
	vcpu.eflags ^= VCPU_EFLAGS_CF;
}
void INS_F6()
{
	vcpu.eip++;
	GetModRegRM(0,8);
	switch(vcpuins.rr) {
	case 0:	GetImm(8);
			TEST((void *)vcpuins.rrm,(void *)vcpuins.rimm,8);
			break;
	case 2:	NOT ((void *)vcpuins.rrm,8);	break;
	case 3:	NEG ((void *)vcpuins.rrm,8);	break;
	case 4:	MUL ((void *)vcpuins.rrm,8);	break;
	case 5:	IMUL((void *)vcpuins.rrm,8);	break;
	case 6:	DIV ((void *)vcpuins.rrm,8);	break;
	case 7:	IDIV((void *)vcpuins.rrm,8);	break;
	default:CaseError("INS_F6::vcpuins.rr");break;}
}
void INS_F7()
{
	vcpu.eip++;
	GetModRegRM(0,16);
	switch(vcpuins.rr) {
	case 0:	GetImm(16);
			TEST((void *)vcpuins.rrm,(void *)vcpuins.rimm,16);
			break;
	case 2:	NOT ((void *)vcpuins.rrm,16);	break;
	case 3:	NEG ((void *)vcpuins.rrm,16);	break;
	case 4:	MUL ((void *)vcpuins.rrm,16);	break;
	case 5:	IMUL((void *)vcpuins.rrm,16);	break;
	case 6:	DIV ((void *)vcpuins.rrm,16);	break;
	case 7:	IDIV((void *)vcpuins.rrm,16);	break;
	default:CaseError("INS_F7::vcpuins.rr");break;}
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  INS_F7\n");
}
void CLC()
{
	vcpu.eip++;
	_ClrEFLAGS_CF;
}
void STC()
{
	vcpu.eip++;
	_SetEFLAGS_CF;
}
void CLI()
{
	vcpu.eip++;
	_ClrEFLAGS_IF;
}
void STI()
{
	vcpu.eip++;
	_SetIF;
}
void CLD()
{
	vcpu.eip++;
	_ClrEFLAGS_DF;
}
void STD()
{
	vcpu.eip++;
	_SetEFLAGS_DF;
}
void INS_FE()
{
	vcpu.eip++;
	GetModRegRM(0,8);
	switch(vcpuins.rr) {
	case 0:	INC((void *)vcpuins.rrm,8);	break;
	case 1:	DEC((void *)vcpuins.rrm,8);	break;
	default:CaseError("INS_FE::vcpuins.rr");break;}
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  INS_FE\n");
}
void INS_FF()
{
	t_nubit8 modrm = 0x00;
	t_nubit16 newcs = 0x0000;
	t_nubit32 neweip = 0x00000000;
	vcpu.ip++;
	GetModRegRM(0,16);
	switch(vcpuins.rr) {
	case 0:	INC((void *)vcpuins.rrm,16);	break;
	case 1:	DEC((void *)vcpuins.rrm,16);	break;
	case 2:	/* CALL_RM16 */
		PUSH((void *)&vcpu.eip,16);
		vcpu.eip = d_nubit16(vcpuins.rrm);
		break;
	case 3:	/* CALL_M16_16 */
		PUSH((void *)&vcpu.cs.selector,16);
		PUSH((void *)&vcpu.eip,16);
		vcpu.eip = d_nubit16(vcpuins.rrm);
		vcpu.cs.selector = d_nubit16(vcpuins.rrm+2);
		break;
	case 4:	/* JMP_RM16 */
		vcpu.eip = d_nubit16(vcpuins.rrm);
		break;
	case 5:	/* JMP_M16_16 */
		vcpu.eip = d_nubit16(vcpuins.rrm);
		vcpu.cs.selector = d_nubit16(vcpuins.rrm+2);
		break;
	case 6:	/* PUSH_RM16 */
		PUSH((void *)vcpuins.rrm,16);
		break;
	case 7:
		UndefinedOpcode();
		return;
	default:CaseError("INS_FF::vcpuins.rr");return;}
}

tots _a_bscc(t_vaddrcc rdest, t_vaddrcc rsrc, t_nubit8 bit, t_bool forward)
{
	t_nubitcc src = 0;
	t_nubitcc temp = 0;
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
			while(!GetBit(src, (1 << temp))) {
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
			while(!GetBit(src, (1 << temp))) {
				if (forward) temp++;
				else temp--;
			}
			d_nubit32(rdest) = (t_nubit32)temp;
		}
		break;
	default:_chk(_SetExcept_CE(0));break;}
	_ce;
}
tots BSF_R16_RM16()
{
	_cb("BSF_R16_RM16");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
	_chk(_a_bscc(vcpuins.rr, vcpuins.rrm, _GetOperandSize * 8, 1));
	_ce;
}
tots BSR_R16_RM16()
{
	_cb("BSR_R16_RM16");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize, 0));
	_chk(_a_bscc(vcpuins.rr, vcpuins.rrm, _GetOperandSize * 8, 0));
	_ce;
}

static t_bool IsPrefix(t_nubit8 opcode)
{
	switch(opcode) {
	case 0xf0: case 0xf2: case 0xf3:
	case 0x2e: case 0x36: case 0x3e: case 0x26:
	case 0x64: case 0x65: case 0x66: case 0x67:
				return 0x01;break;
	default:	return 0x00;break;
	}
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
	vcpuins.flaginsloop = 0x00;
	vcpuins.except = 0x00;
	vcpuins.excode = 0x00;
	vcpuins.flagmem = 0x00;
	vcpuins.roverds = &vcpu.ds;
	vcpuins.roverss = &vcpu.ss;
/* temp stmts for real mode */
	vcpu.cs.base = vcpu.cs.selector << 4;vcpu.cs.limit = vcpu.cs.base + 0xffff;
	vcpu.ss.base = vcpu.ss.selector << 4;vcpu.ss.limit = vcpu.ss.base + 0xffff;
	vcpu.ds.base = vcpu.ds.selector << 4;vcpu.ds.limit = vcpu.ds.base + 0xffff;
	vcpu.es.base = vcpu.es.selector << 4;vcpu.es.limit = vcpu.es.base + 0xffff;
	vcpu.fs.base = vcpu.fs.selector << 4;vcpu.fs.limit = vcpu.fs.base + 0xffff;
	vcpu.gs.base = vcpu.gs.selector << 4;vcpu.gs.limit = vcpu.gs.base + 0xffff;
}
static void ExecInsFinal()
{
	if (vcpuins.flaginsloop) {
		vcpu.cs = vcpuins.oldcs;
		vcpu.eip = vcpuins.oldeip;
	}
	if (vcpuins.except || trace.cid) {
		_Trace_Release(1);
		vcpu.cs = vcpuins.oldcs;
		vcpu.eip = vcpuins.oldeip;
		vcpu.ss = vcpuins.oldss;
		vcpu.esp = vcpuins.oldesp;
		vapiCallBackMachineStop();
	}
}
static void ExecIns()
{
	t_nubit8 opcode;
	ExecInsInit();
	do {
		_cb("ExecIns");
		_chb(opcode = (t_nubit8)_m_read_cs(vcpu.eip, 1));
		_chb(ExecFun(vcpuins.table[opcode]));
		_ce;
	} while (IsPrefix(opcode));
	ExecInsFinal();
}
static void ExecInt()
{
	/* hardware interrupt handeler */
	if(vcpu.flagnmi) INT(0x02);
	vcpu.flagnmi = 0x00;

	if(_GetIF && vpicScanINTR())
		INT(vpicGetINTR());

	if(_GetTF) INT(0x01);
}

done QDX()
{
	t_nubit32 a, b;
	_cb("QDX");
	_adv;
	_chk(_d_imm(1));
	switch (d_nubit8(vcpuins.rimm)) {
	case 0x00:
	case 0xff:
		vapiPrint("\nNXVM CPU STOP at CS:%04X IP:%08X INS:QDX IMM:%02X\n",
			vcpu.cs.selector,vcpu.eip,d_nubit8(vcpuins.rimm));
		vapiPrint("This happens because of the special instruction.\n");
		vapiCallBackMachineStop();
		break;
	case 0x01:
	case 0xfe:
		vapiPrint("\nNXVM CPU RESET at CS:%04X IP:%08X INS:QDX IMM:%02X\n",
			vcpu.cs.selector,vcpu.eip,d_nubit8(vcpuins.rimm));
		vapiPrint("This happens because of the special instruction.\n");
		vapiCallBackMachineReset();
		break;
	case 0x02:
		vapiPrint("\nNXVM CPU PRINT16 at CS:%04X IP:%08X INS:QDX IMM:%02X\n",
			vcpu.cs.selector,vcpu.eip,d_nubit8(vcpuins.rimm));
		vapiCallBackDebugPrintRegs(16);
		break;
	case 0x03:
		vapiPrint("\nNXVM CPU PRINT32 at CS:%04X IP:%08X INS:QDX IMM:%02X\n",
			vcpu.cs.selector,vcpu.eip,d_nubit8(vcpuins.rimm));
		vapiCallBackDebugPrintRegs(32);
		break;
	case 0x04:
		/* code test */
		break;
	default:
		qdbiosExecInt(d_nubit8(vcpuins.rimm));
		MakeBit(vramVarWord(_ss,_sp + 4), VCPU_EFLAGS_ZF, _GetZF);
		MakeBit(vramVarWord(_ss,_sp + 4), VCPU_EFLAGS_CF, _GetEFLAGS_CF);
		break;
	}
	_ce;
}

void vcpuinsInit()
{
	_Trace_Init();
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
	vcpuins.table[0x0f] = (t_faddrcc)INS_0F;
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
	vcpuins.table[0x26] = (t_faddrcc)ES;
	vcpuins.table[0x27] = (t_faddrcc)DAA;
	vcpuins.table[0x28] = (t_faddrcc)SUB_RM8_R8;
	vcpuins.table[0x29] = (t_faddrcc)SUB_RM16_R16;
	vcpuins.table[0x2a] = (t_faddrcc)SUB_R8_RM8;
	vcpuins.table[0x2b] = (t_faddrcc)SUB_R16_RM16;
	vcpuins.table[0x2c] = (t_faddrcc)SUB_AL_I8;
	vcpuins.table[0x2d] = (t_faddrcc)SUB_AX_I16;
	vcpuins.table[0x2e] = (t_faddrcc)CS;
	vcpuins.table[0x2f] = (t_faddrcc)DAS;
	vcpuins.table[0x30] = (t_faddrcc)XOR_RM8_R8;
	vcpuins.table[0x31] = (t_faddrcc)XOR_RM16_R16;
	vcpuins.table[0x32] = (t_faddrcc)XOR_R8_RM8;
	vcpuins.table[0x33] = (t_faddrcc)XOR_R16_RM16;
	vcpuins.table[0x34] = (t_faddrcc)XOR_AL_I8;
	vcpuins.table[0x35] = (t_faddrcc)XOR_AX_I16;
	vcpuins.table[0x36] = (t_faddrcc)SS;
	vcpuins.table[0x37] = (t_faddrcc)AAA;
	vcpuins.table[0x38] = (t_faddrcc)CMP_RM8_R8;
	vcpuins.table[0x39] = (t_faddrcc)CMP_RM16_R16;
	vcpuins.table[0x3a] = (t_faddrcc)CMP_R8_RM8;
	vcpuins.table[0x3b] = (t_faddrcc)CMP_R16_RM16;
	vcpuins.table[0x3c] = (t_faddrcc)CMP_AL_I8;
	vcpuins.table[0x3d] = (t_faddrcc)CMP_AX_I16;
	vcpuins.table[0x3e] = (t_faddrcc)DS;
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
	vcpuins.table[0x60] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0x61] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0x62] = (t_faddrcc)BOUND_R16_M16_16;
	vcpuins.table[0x63] = (t_faddrcc)ARPL_RM16_R16;
	vcpuins.table[0x64] = (t_faddrcc)FS;
	vcpuins.table[0x65] = (t_faddrcc)GS;
	vcpuins.table[0x66] = (t_faddrcc)OprSize;
	vcpuins.table[0x67] = (t_faddrcc)AddrSize;
	vcpuins.table[0x68] = (t_faddrcc)UndefinedOpcode;
	//vcpuins.table[0x68] = (t_faddrcc)PUSH_I16;
	vcpuins.table[0x69] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0x6a] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0x6b] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0x6c] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0x6d] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0x6e] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0x6f] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0x70] = (t_faddrcc)JO;
	vcpuins.table[0x71] = (t_faddrcc)JNO;
	vcpuins.table[0x72] = (t_faddrcc)JC;
	vcpuins.table[0x73] = (t_faddrcc)JNC;
	vcpuins.table[0x74] = (t_faddrcc)JZ;
	vcpuins.table[0x75] = (t_faddrcc)JNZ;
	vcpuins.table[0x76] = (t_faddrcc)JBE;
	vcpuins.table[0x77] = (t_faddrcc)JA;
	vcpuins.table[0x78] = (t_faddrcc)JS;
	vcpuins.table[0x79] = (t_faddrcc)JNS;
	vcpuins.table[0x7a] = (t_faddrcc)JP;
	vcpuins.table[0x7b] = (t_faddrcc)JNP;
	vcpuins.table[0x7c] = (t_faddrcc)JL;
	vcpuins.table[0x7d] = (t_faddrcc)JNL;
	vcpuins.table[0x7e] = (t_faddrcc)JLE;
	vcpuins.table[0x7f] = (t_faddrcc)JG;
	vcpuins.table[0x80] = (t_faddrcc)INS_80;
	vcpuins.table[0x81] = (t_faddrcc)INS_81;
	vcpuins.table[0x82] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0x83] = (t_faddrcc)INS_83;
	vcpuins.table[0x84] = (t_faddrcc)TEST_RM8_R8;
	vcpuins.table[0x85] = (t_faddrcc)TEST_RM16_R16;
	vcpuins.table[0x86] = (t_faddrcc)XCHG_R8_RM8;
	vcpuins.table[0x87] = (t_faddrcc)XCHG_R16_RM16;
	vcpuins.table[0x88] = (t_faddrcc)MOV_RM8_R8;
	vcpuins.table[0x89] = (t_faddrcc)MOV_RM16_R16;
	vcpuins.table[0x8a] = (t_faddrcc)MOV_R8_RM8;
	vcpuins.table[0x8b] = (t_faddrcc)MOV_R16_RM16;
	vcpuins.table[0x8c] = (t_faddrcc)MOV_RM16_SEG;
	vcpuins.table[0x8d] = (t_faddrcc)LEA_R16_M16;
	vcpuins.table[0x8e] = (t_faddrcc)MOV_SEG_RM16;
	vcpuins.table[0x8f] = (t_faddrcc)POP_RM16;
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
	vcpuins.table[0xa0] = (t_faddrcc)MOV_AL_M8;
	vcpuins.table[0xa1] = (t_faddrcc)MOV_AX_M16;
	vcpuins.table[0xa2] = (t_faddrcc)MOV_M8_AL;
	vcpuins.table[0xa3] = (t_faddrcc)MOV_M16_AX;
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
	vcpuins.table[0xc0] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0xc1] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0xc2] = (t_faddrcc)RET_I16;
	vcpuins.table[0xc3] = (t_faddrcc)RET;
	vcpuins.table[0xc4] = (t_faddrcc)LES_R16_M16;
	vcpuins.table[0xc5] = (t_faddrcc)LDS_R16_M16;
	vcpuins.table[0xc6] = (t_faddrcc)MOV_M8_I8;
	vcpuins.table[0xc7] = (t_faddrcc)MOV_M16_I16;
	vcpuins.table[0xc8] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table[0xc9] = (t_faddrcc)UndefinedOpcode;
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
	vcpuins.table[0xf0] = (t_faddrcc)LOCK;
	vcpuins.table[0xf1] = (t_faddrcc)QDX;
	vcpuins.table[0xf2] = (t_faddrcc)REPNZ;
	vcpuins.table[0xf3] = (t_faddrcc)REP;
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
	vcpuins.table_0f[0x00] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x01] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x02] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x03] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x04] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x05] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x06] = (t_faddrcc)UndefinedOpcode;
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
	vcpuins.table_0f[0x20] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x21] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x22] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x23] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x24] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x25] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x26] = (t_faddrcc)UndefinedOpcode;
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
	vcpuins.table_0f[0x80] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x81] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x82] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x83] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x84] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x85] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x86] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x87] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x88] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x89] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x8a] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x8b] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x8c] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x8d] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x8e] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x8f] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x90] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x91] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x92] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x93] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x94] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x95] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x96] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x97] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x98] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x99] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x9a] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x9b] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x9c] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x9d] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x9e] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0x9f] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xa0] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xa1] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xa2] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xa3] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xa4] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xa5] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xa6] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xa7] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xa8] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xa9] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xaa] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xab] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xac] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xad] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xae] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xaf] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb0] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb1] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb2] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb3] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb4] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb5] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb6] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb7] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb8] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb9] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xba] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xbb] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xbc] = (t_faddrcc)BSF_R16_RM16;
	vcpuins.table_0f[0xbd] = (t_faddrcc)BSR_R16_RM16;
	vcpuins.table_0f[0xbe] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xbf] = (t_faddrcc)UndefinedOpcode;
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
void vcpuinsReset()
{
	vcpuins.rrm = vcpuins.rr = vcpuins.rimm = (t_vaddrcc)NULL;
	vcpuins.opr1 = vcpuins.opr2 = vcpuins.result = vcpuins.bit = 0;
}
void vcpuinsRefresh()
{
	ExecIns();
	ExecInt();
}
void vcpuinsFinal()
{
	_Trace_Release(0);
}
