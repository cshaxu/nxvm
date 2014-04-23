/* This file is a part of NXVM project. */

#include "stdlib.h"
#include "memory.h"

#include "vglobal.h"
#ifndef VGLOBAL_BOCHS
#include "vapi.h"
#include "vport.h"
#include "vram.h"
#include "vpic.h"
#include "bios/qdbios.h"
#else
#include "vcpuapi.h"
#define vapiPrint vcpuapiPrint
#endif
#include "vcpuins.h"

/* DEBUGGING OPTIONS ******************************************************* */
#define i386(n) if (1)
#define VCPUINS_TRACE 0
/* ************************************************************************* */

#define _______todo static void /* need to implement */

static t_bool flagignore;

#if VCPUINS_TRACE == 1

static t_api_trace_call trace;

#define _cb(s) vapiTraceCallBegin(&trace, s)
#define _ce    vapiTraceCallEnd(&trace)
#define _bb(s) vapiTraceBlockBegin(&trace, s)
#define _be    vapiTraceBlockEnd(&trace)
#define _chb(n) if (1) {(n);if (vcpuins.except) {trace.flagerror = 1;vapiTraceFinal(&trace);break;   }} else
#define _chk(n) do     {(n);if (vcpuins.except) {trace.flagerror = 1;vapiTraceFinal(&trace);return;  }} while (0)
#define _chr(n) do     {(n);if (vcpuins.except) {trace.flagerror = 1;vapiTraceFinal(&trace);return 0;}} while (0)
#else
#define _cb(s)
#define _ce
#define _be
#define _bb(s)
#define _chb(n) if (1) {(n);if (vcpuins.except) break;   } else
#define _chk(n) do     {(n);if (vcpuins.except) return;  } while (0)
#define _chr(n) do     {(n);if (vcpuins.except) return 0;} while (0)
#endif

#define _newins_ //if (1) {vapiPrint("NEWINS: ");vapiTracePrintCall();} else
#define _comment vapiPrint

t_cpurec vcpurec;
t_cpuins vcpuins;

/* instruction dispatch */
static t_faddrcc table[0x100], table_0f[0x100];

/* stack pointer size */
#define _GetStackSize   (vcpu.ss.seg.data.big ? 4 : 2)
/* operand size */
#define _GetOperandSize ((vcpu.cs.seg.exec.defsize ^ vcpuins.prefix_oprsize) ? 4 : 2)
/* address size of the source operand */
#define _GetAddressSize ((vcpu.cs.seg.exec.defsize ^ vcpuins.prefix_addrsize) ? 4 : 2)
/* if opcode indicates a prefix */
#define _SetExcept_DE(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_DE), vcpuins.excode = (n), vapiPrint("#DE(%x) - divide error\n",    (n)))
#define _SetExcept_PF(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_PF), vcpuins.excode = (n), vapiPrint("#PF(%x) - page fault\n",      (n)))
#define _SetExcept_GP(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_GP), vcpuins.excode = (n), vapiPrint("#GP(%x) - general protect\n", (n)))
#define _SetExcept_SS(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_SS), vcpuins.excode = (n), vapiPrint("#SS(%x) - stack segment\n",   (n)))
#define _SetExcept_UD(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_UD), vcpuins.excode = (n), vapiPrint("#UD(%x) - undefined\n",       (n)))
#define _SetExcept_NP(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_NP), vcpuins.excode = (n), vapiPrint("#NP(%x) - not present\n",     (n)))
#define _SetExcept_BR(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_BR), vcpuins.excode = (n), vapiPrint("#BR(%x) - boundary\n",        (n)))
#define _SetExcept_TS(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_TS), vcpuins.excode = (n), vapiPrint("#TS(%x) - task state\n",      (n)))
#define _SetExcept_NM(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_NM), vcpuins.excode = (n), vapiPrint("#NM(%x) - divide error\n",    (n)))

#define _SetExcept_CE(n) (SetBit(vcpuins.except, VCPUINS_EXCEPT_CE), vcpuins.excode = (n), vapiPrint("#CE(%x) - internal case error\n", (n)), vapiCallBackMachineStop())
#define _impossible_     _chk(_SetExcept_CE(0xffffffff))
#define _impossible_r_   _chr(_SetExcept_CE(0xffffffff))

/* memory management unit */
/* kernel memory accessing */
/* read content from reference */
static void _kma_read_ref(t_vaddrcc ref, t_vaddrcc rdata, t_nubit8 byte)
{
	t_nubit8 i;
	_cb("_kma_read_ref");
	for (i = 0;i < byte;++i)
		d_nubit8(rdata + i) = d_nubit8(ref + i);
	_ce;
}
/* write content to reference */
static void _kma_write_ref(t_vaddrcc ref, t_vaddrcc rdata, t_nubit8 byte)
{
	t_nubit8 i;
	_cb("_kma_write_ref");
	for (i = 0;i < byte;++i)
		d_nubit8(ref + i) = d_nubit8(rdata + i);
	_ce;
}
/* physical address mapping */
static t_nubit32 _kma_map_physical(t_nubit32 physical)
{
	_cb("_kma_map_physical");
#ifndef VGLOBAL_BOCHS
	if (physical > vramSize) {
		if (physical >= 0xfffe0000)
			physical &= 0x001fffff;
		else
			_impossible_r_;
	}
#endif
	_ce;
	return physical;
}
/* read content from physical */
static void _kma_read_physical(t_nubit32 physical, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_kma_read_physical");
	_chk(physical = _kma_map_physical(physical));
#ifndef VGLOBAL_BOCHS
	_chk(_kma_read_ref(vramAddr(physical), rdata, byte));
#else
	_chk(vcpuapiReadPhysical(physical, rdata, byte));
#endif
	_ce;
}
/* write content to physical */
static void _kma_write_physical(t_nubit32 physical, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_kma_write_physical");
	_chk(physical = _kma_map_physical(physical));
#ifndef VGLOBAL_BOCHS
	_chk(_kma_write_ref(vramAddr(physical), rdata, byte));
#else
	_chk(vcpuapiWritePhysical(physical, rdata, byte));
#endif
	_ce;
}
/* translate linear to physical - paging mechanism*/
static t_nubit32 _kma_physical_linear(t_nubit32 linear, t_nubit8 byte, t_bool write, t_nubit8 vpl)
{
	t_nubit32 ppde, ppte; /* page table entries */
	t_nubit32 cpde, cpte;
	_cb("_t_kma_physical_linear");
	if (_GetLinear_Offset(linear) > GetMax32(_GetPageSize - byte)) _impossible_r_;
	if (!_IsPaging) {
		_ce;
		return linear;
	}
	ppde = _GetCR3_Base + _GetLinear_Dir(linear) * 4;
	_chr(_kma_read_physical(ppde, GetRef(cpde), 4));
	if (!_IsPageEntryPresent(cpde)) {
		_bb("!PageDirEntryPresent");
		vcpu.cr2 = linear;
		_chr(_SetExcept_PF(_MakePageFaultErrorCode(0, write, (vpl == 3))));
		_be;
	}
	if (vpl == 0x03) {
		_bb("vpl(3)");
		if (!_GetPageEntry_US(cpde)) {
			_bb("PageDirEntry_US(0)");
			vcpu.cr2 = linear;
			_chr(_SetExcept_PF(_MakePageFaultErrorCode(1, write, 1)));
			_be;
		}
		if (write && !_IsPageEntryWritable(cpde)) {
			_bb("write,!PageDirEntryWritable");
			vcpu.cr2 = linear;
			_chr(_SetExcept_PF(_MakePageFaultErrorCode(1, 1, 1)));
			_be;
		}
		_be;
	}
	_SetPageEntry_A(cpde);
	_chr(_kma_write_physical(ppde, GetRef(cpde), 4));
	ppte = _GetPageEntry_Base(cpde) + _GetLinear_Page(linear) * 4;
	_chr(_kma_read_physical(ppte, GetRef(cpte), 4));
	if (!_IsPageEntryPresent(cpte)) {
		_bb("!PageTabEntryPresent");
		vcpu.cr2 = linear;
		_chr(_SetExcept_PF(_MakePageFaultErrorCode(0, write, (vpl == 3))));
		_be;
	}
	if (vpl == 0x03) {
		_bb("vpl(3)");
		if (!_GetPageEntry_US(cpte)) {
			_bb("PageTabEntry_US(0)");
			vcpu.cr2 = linear;
			_chr(_SetExcept_PF(_MakePageFaultErrorCode(1, write, 1)));
			_be;
		}
		if (write && !_IsPageEntryWritable(cpte)) {
			_bb("write,!PageTabEntryWritable");
			vcpu.cr2 = linear;
			_chr(_SetExcept_PF(_MakePageFaultErrorCode(1, 1, 1)));
			_be;
		}
		_be;
	}
	_SetPageEntry_A(cpte);
	if (write) _SetPageEntry_D(cpte);
	_chr(_kma_write_physical(ppte, GetRef(cpte), 4));
	_ce;
	return (_GetPageEntry_Base(cpte) + _GetLinear_Offset(linear));
}
/* translate logical to linear - segmentation mechanism */
static t_nubit32 _kma_linear_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force)
{
	t_nubit32 linear;
	t_nubit32 upper, lower;
	_cb("_kma_linear_logical");
	switch (rsreg->sregtype) {
	case SREG_CODE:
		_bb("sregtype(SREG_CODE)");
		if (!rsreg->flagvalid) _impossible_r_;
		if (_IsProtected) {
			_bb("Protected");
			if (!force) {
				_bb("force");
				if (write) {
					_bb("write");
					_chr(_SetExcept_GP(0));
					_be;
				}
				if (!rsreg->seg.exec.readable) {
					_bb("!readable");
					_chr(_SetExcept_GP(0));
					_be;
				}
				_be;
			}
			_be;
		}
		lower = 0x00000000;
		upper = rsreg->limit;
		_be;break;
	case SREG_STACK:
		_bb("sregtype(SREG_STACK)");
		if (!rsreg->flagvalid) _impossible_r_;
		if (_IsProtected) {
			_bb("Protected");
			if (rsreg->seg.executable || !rsreg->seg.data.writable) _impossible_r_;
			_be;
		}
		if (rsreg->seg.data.expdown) {
			lower = rsreg->limit + 1;
			upper = rsreg->seg.data.big ? 0xffffffff : 0x0000ffff;
		} else {
			lower = 0x00000000;
			upper = rsreg->limit;
		}
		_be;break;
	case SREG_DATA:
		_bb("sregtype(SREG_DATA)");
		if (!rsreg->flagvalid) {
			_bb("selector(null)");
			_chr(_SetExcept_GP(0));
			_be;
		}
		if (_IsProtected) {
			_bb("Protected");
			if (_IsSelectorNull(rsreg->selector)) {
				_bb("selector(null)");
				_chr(_SetExcept_GP(0));
				_be;
			}
			if (rsreg->seg.executable && !rsreg->seg.exec.readable) _impossible_r_;
			if (!force) {
				_bb("force");
				if (write) {
					_bb("write");
					if (rsreg->seg.executable) {
						_bb("executable");
						_chr(_SetExcept_GP(0));
						_be;
					} else {
						_bb("!executable");
						if (!rsreg->seg.data.writable) {
							_bb("!writable");
							_chr(_SetExcept_GP(0));
							_be;
						}
						_be;
					}
					_be;
				}
				_be;
			}
			_be;
		}
		if (rsreg->seg.data.expdown) {
			lower = rsreg->limit + 1;
			upper = rsreg->seg.data.big ? 0xffffffff : 0x0000ffff;
		} else {
			lower = 0x00000000;
			upper = rsreg->limit;
		}
		_be;break;
	case SREG_GDTR:
		_bb("sregtype(SREG_GDTR)");
		if (!_GetCR0_PE) _impossible_r_;
		lower = 0x00000000;
		upper = rsreg->limit;
		_be;break;
	case SREG_IDTR:
		_bb("sregtype(SREG_IDTR)");
		lower = 0x00000000;
		upper = rsreg->limit;
		_be;break;
	case SREG_LDTR:
		_bb("sregtype(SREG_LDTR)");
		if (!_GetCR0_PE) _impossible_r_;
		if (_GetSelector_TI(rsreg->selector)) _impossible_r_;
		if (!rsreg->flagvalid || _IsSelectorNull(rsreg->selector)) {
			_bb("selector(null)");
			_chr(_SetExcept_GP(0));
			_be;
		}
		lower = 0x00000000;
		upper = rsreg->limit;
		_be;break;
	case SREG_TR:
		_bb("sregtype(SREG_TR)");
		if (!_GetCR0_PE) _impossible_r_;
		if (!rsreg->flagvalid || _IsSelectorNull(rsreg->selector)) _impossible_r_;
		if (_GetSelector_TI(rsreg->selector)) _impossible_r_;
		lower = 0x00000000;
		upper = rsreg->limit;
		_be;break;
	default:_impossible_r_;}
	linear = rsreg->base + offset;
	if (offset < lower || offset > upper - (byte - 1)) {
		_bb("offset(<lower/>upper)");
		flagignore = 1;
		switch (rsreg->sregtype) {
		case SREG_STACK:
			_bb("sregtype(SREG_STACK)");
			_chr(_SetExcept_SS(0));
			_be;break;
		case SREG_TR:
			_bb("sregtype(SREG_TR)");
			_chr(_SetExcept_TS(rsreg->selector));
			_be;break;
		default:
			_bb("sregtype(default)");
			_chr(_SetExcept_GP(0));
			_be;break;
		}
		_be;
	}
	_ce;
	return linear;
}
/* read content from logical */
static void _kma_read_linear(t_nubit32 linear, t_vaddrcc rdata, t_nubit8 byte, t_nubit8 vpl, t_bool force)
{
	t_nubit32 phy1, phy2;
	t_nubit8  byte1, byte2;
	_cb("_kma_read_logical");
	if (_GetLinear_Offset(linear) > GetMax32(_GetPageSize - byte)) {
		_bb("Linear_Offset(>PageSize)");
		byte1 = _GetPageSize - _GetLinear_Offset(linear);
		byte2 = byte - byte1;
		_chk(phy1 = _kma_physical_linear(linear        , byte1, 0, vpl));
		_chk(phy2 = _kma_physical_linear(linear + byte1, byte2, 0, vpl));
		_chk(_kma_read_physical(phy1, rdata        , byte1));
		_chk(_kma_read_physical(phy2, rdata + byte1, byte2));
		_be;
	} else {
		_bb("Linear_Offset(<=PageSize)");
		byte1 = byte;
		_chk(phy1 = _kma_physical_linear(linear, byte1, 0, vpl));
		_chk(_kma_read_physical(phy1, rdata, byte1));
		_be;
	}
	_ce;
}
/* write content to logical */
static void _kma_write_linear(t_nubit32 linear, t_vaddrcc rdata, t_nubit8 byte, t_nubit8 vpl, t_bool force)
{
	t_nubit32 phy1, phy2;
	t_nubit8  byte1, byte2;
	_cb("_kma_write_linear");
	if (_GetLinear_Offset(linear) > GetMax32(_GetPageSize - byte)) {
		_bb("Linear_Offset(>PageSize)");
		byte1 = _GetPageSize - _GetLinear_Offset(linear);
		byte2 = byte - byte1;
		_chk(phy1 = _kma_physical_linear(linear        , byte1, 1, vpl));
		_chk(phy2 = _kma_physical_linear(linear + byte1, byte2, 1, vpl));
		_chk(_kma_write_physical(phy1, rdata        , byte1));
		_chk(_kma_write_physical(phy2, rdata + byte1, byte2));
		_be;
	} else {
		_bb("Linear_Offset(<=PageSize)");
		byte1 = byte;
		_chk(phy1 = _kma_physical_linear(linear, byte1, 1, vpl));
		_chk(_kma_write_physical(phy1, rdata, byte1));
		_be;
	}
	_ce;
}
/* read content from logical */
static void _kma_read_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte, t_nubit8 vpl, t_bool force)
{
//	t_nubit8 i;
	t_nubit32 linear;
	_cb("_kma_read_logical");
	_chk(linear = _kma_linear_logical(rsreg, offset, byte, 0, vpl, force));
	_chk(_kma_read_linear(linear, rdata, byte, vpl, force));
	if (!force) {
		_bb("!force");
		vcpurec.mem[vcpurec.msize].flagwrite = 0;
		vcpurec.mem[vcpurec.msize].data = 0;
		_chk(_kma_read_ref(rdata, GetRef(vcpurec.mem[vcpurec.msize].data), byte));
		vcpurec.mem[vcpurec.msize].byte = byte;
		vcpurec.mem[vcpurec.msize].linear = linear;
		if (vcpuins.flagwr) {
			if (vcpuins.wrlin >= vcpurec.mem[vcpurec.msize].linear &&
				vcpuins.wrlin < vcpurec.mem[vcpurec.msize].linear + byte) {
				vapiPrint("Watch point caught at L%08x: READ %01x BYTES OF DATA=%08x FROM L%08x\n", vcpurec.linear,
					vcpurec.mem[vcpurec.msize].byte,
					vcpurec.mem[vcpurec.msize].data,
					vcpurec.mem[vcpurec.msize].linear);
			}
		}
		/*for (i = 0;i < vcpurec.msize;++i) {
			if (vcpurec.mem[i].flagwrite == vcpurec.mem[vcpurec.msize].flagwrite &&
				vcpurec.mem[i].linear == vcpurec.mem[vcpurec.msize].linear) {
				_bb("mem(same)");
				_impossible_r_;
				_ce;
			}
		}*/
		vcpurec.msize++;
		if (vcpurec.msize == 0x20) _impossible_;
		_be;
	}
	_ce;
}
/* write content to logical */
static void _kma_write_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte, t_nubit8 vpl, t_bool force)
{
//	t_nubit8 i;
	t_nubit32 linear;
	_cb("_kma_write_logical");
	_chk(linear = _kma_linear_logical(rsreg, offset, byte, 1, vpl, force));
	_chk(_kma_write_linear(linear, rdata, byte, vpl, force));
	if (!force) {
		_bb("!force");
		vcpurec.mem[vcpurec.msize].flagwrite = 1;
		vcpurec.mem[vcpurec.msize].data = 0;
		_chk(_kma_read_ref(rdata, GetRef(vcpurec.mem[vcpurec.msize].data), byte));
		vcpurec.mem[vcpurec.msize].byte = byte;
		vcpurec.mem[vcpurec.msize].linear = linear;
		if (vcpuins.flagww) {
			if (vcpuins.wwlin >= vcpurec.mem[vcpurec.msize].linear &&
				vcpuins.wwlin < vcpurec.mem[vcpurec.msize].linear + byte) {
				vapiPrint("Watch point caught at L%08x: WRITE %01x BYTES OF DATA=%08x TO L%08x\n", vcpurec.linear,
					vcpurec.mem[vcpurec.msize].byte,
					vcpurec.mem[vcpurec.msize].data,
					vcpurec.mem[vcpurec.msize].linear);
			}
		}
		/*for (i = 0;i < vcpurec.msize;++i) {
			if (vcpurec.mem[i].flagwrite == vcpurec.mem[vcpurec.msize].flagwrite &&
				vcpurec.mem[i].linear == vcpurec.mem[vcpurec.msize].linear) {
				_bb("mem(same)");
				_impossible_;
				_ce;
			}
		}*/
		vcpurec.msize++;
		if (vcpurec.msize == 0x20) _impossible_;
		_be;
	}
	_ce;
}
/* test logical accessing */
static void _kma_test_linear(t_nubit32 linear, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force)
{
	t_nubit32 phy1, phy2;
	t_nubit8  byte1, byte2;
	_cb("_kma_test_linear");
	if (_GetLinear_Offset(linear) > GetMax32(_GetPageSize - byte)) {
		_bb("Linear_Offset(>PageSize)");
		byte1 = _GetPageSize - _GetLinear_Offset(linear);
		byte2 = byte - byte1;
		_chk(phy1 = _kma_physical_linear(linear        , byte1, write, vpl));
		_chk(phy2 = _kma_physical_linear(linear + byte1, byte2, write, vpl));
		_be;
	} else {
		_bb("Linear_Offset(<=PageSize)");
		byte1 = byte;
		_chk(phy1 = _kma_physical_linear(linear, byte1, write, vpl));
		_be;
	}
	_ce;
}
static void _kma_test_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force)
{
	t_nubit32 linear;
	_cb("_kma_test_logical");
	_chk(linear = _kma_linear_logical(rsreg, offset, byte, write, vpl, force));
	_ce;
}
static void _kma_test_access(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force)
{
	t_nubit32 linear;
	_cb("_kma_test_access");
	_chk(linear = _kma_linear_logical(rsreg, offset, byte, write, vpl, force));
	_chk(_kma_test_linear(linear, byte, write, vpl, force));
	_ce;
}

/* general memory accessing */
static void _m_read_ref(t_vaddrcc ref, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_m_read_ref");
	if (vramIsAddrInMem(ref)) _impossible_;
	_chk(_kma_read_ref(ref, rdata, byte));
	_ce;
}
static void _m_write_ref(t_vaddrcc ref, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_m_write_ref");
	if (vramIsAddrInMem(ref)) _impossible_;
	_chk(_kma_write_ref(ref, rdata, byte));
	_ce;
}
static void _m_read_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_m_read_logical");
	_chk(_kma_read_logical(rsreg, offset, rdata, byte, _GetCPL, 0));
	_ce;
}
static void _m_write_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_m_write_logical");
	_chk(_kma_write_logical(rsreg, offset, rdata, byte, _GetCPL, 0));
	_ce;
}
static void _m_test_access(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write)
{
	t_vaddrcc ref = 0;
	_cb("_m_test_access");
	_chk(_kma_test_access(rsreg, offset, byte, write, _GetCPL, 0));
	_ce;
}
static void _m_test_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write)
{
	t_vaddrcc ref = 0;
	_cb("_m_test_logical");
	_chk(_kma_test_logical(rsreg, offset, byte, write, _GetCPL, 0));
	_ce;
}

static void _m_read_rm(t_nubit8 byte)
{
	_cb("_m_read_rm");
	if (vcpuins.flagmem)
		_chk(_m_read_logical(vcpuins.mrm.rsreg, vcpuins.mrm.offset, GetRef(vcpuins.crm), byte));
	else
		_chk(_m_read_ref(vcpuins.rrm, GetRef(vcpuins.crm), byte));
	_ce;
}
static void _m_write_rm(t_nubit8 byte)
{
	_cb("_m_write_rm");
	if (vcpuins.flagmem)
		_chk(_m_write_logical(vcpuins.mrm.rsreg, vcpuins.mrm.offset, GetRef(vcpuins.crm), byte));
	else
		_chk(_m_write_ref(vcpuins.rrm, GetRef(vcpuins.crm), byte));
	_ce;
}

/* segment accessing unit: _s_ */
/* kernel segment accessing */
static void _ksa_read_idt(t_nubit8 intid, t_vaddrcc rdata)
{
	_cb("_s_read_idt");
	if (!_GetCR0_PE) {
		_bb("CR0_PE(0)");
		if (GetMax16(intid * 4 + 3) > GetMax16(vcpu.idtr.limit)) _impossible_;
		_chk(_kma_read_logical(&vcpu.idtr, (intid * 4), rdata, 4, 0x00, 0));
		_be;
	} else {
		_bb("CR0_PE(1)");
		if (GetMax16(intid * 8 + 7) > GetMax16(vcpu.idtr.limit)) _impossible_;
		_chk(_kma_read_logical(&vcpu.idtr, (intid * 8), rdata, 8, 0x00, 0));
		_be;
	}
	_ce;
}
static void _ksa_read_ldt(t_nubit16 selector, t_vaddrcc rdata)
{
	_cb("_ksa_read_ldt");
	if (!_GetCR0_PE) _impossible_;
	if (!_GetSelector_TI(selector)) _impossible_;
	if (GetMax16(_GetSelector_Offset(selector) + 7) > GetMax16(vcpu.ldtr.limit)) {
		_bb("Selector_Offset(>ldtr.limit)");
		_chk(_SetExcept_GP(selector));
		_be;
	}
	_chk(_kma_read_logical(&vcpu.ldtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
	_ce;
}
static void _ksa_read_gdt(t_nubit16 selector, t_vaddrcc rdata)
{
	_cb("_ksa_read_gdt");
	if (!_GetCR0_PE) _impossible_;
	if (_GetSelector_TI(selector)) _impossible_;
	if (GetMax16(_GetSelector_Offset(selector) + 7) > GetMax16(vcpu.gdtr.limit)) {
		_bb("Selector_Offset(>gdtr.limit)");
		_chk(_SetExcept_GP(selector));
		_be;
	}
	_chk(_kma_read_logical(&vcpu.gdtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
	_ce;
}
static void _ksa_read_xdt(t_nubit16 selector, t_vaddrcc rdata)
{
	_cb("_ksa_read_xdt");
	if (!_GetCR0_PE) _impossible_;
	if (_GetSelector_TI(selector)) {
		_bb("Selector_TI");
		_chk(_ksa_read_ldt(selector, rdata));
		_be;
	} else {
		_bb("!Selector_TI");
		_chk(_ksa_read_gdt(selector, rdata));
		_be;
	}
	_ce;
}
static void _ksa_write_ldt(t_nubit16 selector, t_vaddrcc rdata)
{
	_cb("_ksa_write_ldt");
	if (!_GetCR0_PE) _impossible_;
	if (!_GetSelector_TI(selector)) _impossible_;
	if (GetMax16(_GetSelector_Offset(selector) + 7) > GetMax16(vcpu.ldtr.limit))
		_impossible_;
	_chk(_kma_write_logical(&vcpu.ldtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
	_ce;
}
static void _ksa_write_gdt(t_nubit16 selector, t_vaddrcc rdata)
{
	_cb("_ksa_write_gdt");
	if (!_GetCR0_PE) _impossible_;
	if (_GetSelector_TI(selector)) _impossible_;
	if (GetMax16(_GetSelector_Offset(selector) + 7) > GetMax16(vcpu.gdtr.limit))
		_impossible_;
	_chk(_kma_write_logical(&vcpu.gdtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
	_ce;
}
static void _ksa_write_xdt(t_nubit16 selector, t_vaddrcc rdata)
{
	_cb("_ksa_write_xdt");
	if (!_GetCR0_PE) _impossible_;
	if (_GetSelector_TI(selector)) {
		_bb("Selector_TI");
		_chk(_ksa_write_ldt(selector, rdata));
		_be;
	} else {
		_bb("!Selector_TI");
		_chk(_ksa_write_gdt(selector, rdata));
		_be;
	}
	_ce;
}
static void _ksa_load_sreg(t_cpu_sreg *rsreg, t_nubit16 selector)
{
	t_nubit64 descriptor;
	_cb("_ksa_load_sreg");
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
			_chk(_ksa_read_xdt(selector, GetRef(descriptor)));
			if (!_IsDescCode(descriptor)) {
				_bb("!DescCode");
				_chk(_SetExcept_GP(selector));
				_be;
			}
			if (!_IsDescPresent(descriptor)) {
				_bb("!DescPresent");
				_chk(_SetExcept_NP(selector));
				_be;
			}
			_SetDescUserAccessed(descriptor);
			_chk(_ksa_write_xdt(selector, GetRef(descriptor)));
			rsreg->flagvalid = 1;
			rsreg->base = (t_nubit32)_GetDescSeg_Base(descriptor);
			if (_IsDescCodeNonConform(descriptor))
				rsreg->dpl = (t_nubit4)_GetDesc_DPL(descriptor);
			rsreg->limit = (t_nubit32)((_IsDescSegGranularLarge(descriptor) ?
				((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
			rsreg->seg.accessed = (t_bool)_IsDescUserAccessed(descriptor);
			rsreg->seg.executable = (t_bool)_IsDescUserExecutable(descriptor);
			rsreg->seg.exec.defsize = (t_bool)_IsDescCode32(descriptor);
			rsreg->seg.exec.conform = (t_bool)_IsDescCodeConform(descriptor);
			rsreg->seg.exec.readable = (t_bool)_IsDescCodeReadable(descriptor);
			rsreg->selector = (selector & ~VCPU_SELECTOR_RPL) | _GetCPL;
			_be;
		} else {
			_bb("!Protected");
			rsreg->flagvalid = 1;
			rsreg->base = (selector << 4);
			rsreg->selector = selector;
			if (_GetCR0_PE && _GetEFLAGS_VM) {
				rsreg->dpl = 0x03;
				rsreg->limit = 0x0000ffff;
			}
			_be;
		}
		_be;break;
	case SREG_DATA:
		_bb("sregtype(SREG_DATA)");
		if (_IsProtected) {
			_bb("Protected");
			if (_IsSelectorNull(selector)) {
				_bb("selector(null)");
				rsreg->flagvalid = 0;
				rsreg->selector = selector;
				_be;
			} else {
				_bb("selector(!null)");
				_chk(_ksa_read_xdt(selector, GetRef(descriptor)));
				if (!_IsDescData(descriptor) && !_IsDescCodeReadable(descriptor)) {
					_bb("!DescData,!DescCodeReadable");
					_chk(_SetExcept_GP(selector));
					_be;
				}
				if (_IsDescData(descriptor) || _IsDescCodeNonConform(descriptor)) {
					_bb("DescData/DescCodeNonConform)");
					if (_GetSelector_RPL(selector) > _GetDesc_DPL(descriptor) ||
						_GetCPL > _GetDesc_DPL(descriptor)) {
						_bb("PL(fail)");
						_chk(_SetExcept_GP(selector));
						_be;
					}
					_be;
				}
				if (!_IsDescPresent(descriptor)) {
					_bb("!DescPresent");
					_chk(_SetExcept_NP(selector));
					_be;
				}
				_SetDescUserAccessed(descriptor);
				_chk(_ksa_write_xdt(selector, GetRef(descriptor)));
				rsreg->flagvalid = 1;
				rsreg->selector = selector;
				rsreg->base = (t_nubit32)_GetDescSeg_Base(descriptor);
				rsreg->dpl = (t_nubit4)_GetDesc_DPL(descriptor);
				rsreg->limit = (t_nubit32)((_IsDescSegGranularLarge(descriptor) ?
					((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
				rsreg->seg.accessed = (t_bool)_IsDescUserAccessed(descriptor);
				rsreg->seg.executable = (t_bool)_IsDescUserExecutable(descriptor);
				if (rsreg->seg.executable) {
					rsreg->seg.exec.defsize = (t_bool)_IsDescCode32(descriptor);
					rsreg->seg.exec.conform = (t_bool)_IsDescCodeConform(descriptor);
					rsreg->seg.exec.readable = (t_bool)_IsDescCodeReadable(descriptor);
				} else {
					rsreg->seg.data.big = (t_bool)_IsDescDataBig(descriptor);
					rsreg->seg.data.expdown = (t_bool)_IsDescDataExpDown(descriptor);
					rsreg->seg.data.writable = (t_bool)_IsDescDataWritable(descriptor);
				}
				_be;
			}
			_be;
		} else {
			_bb("!Protected");
			rsreg->flagvalid = 1;
			rsreg->selector = selector;
			rsreg->base = (selector << 4);
			if (_GetCR0_PE && _GetEFLAGS_VM) {
				rsreg->dpl = 0x03;
				rsreg->limit = 0x0000ffff;
			}
			_be;
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
			_chk(_ksa_read_xdt(selector, GetRef(descriptor)));
			if (!_IsDescDataWritable(descriptor)) {
				_bb("!DescDataWritable");
				_chk(_SetExcept_GP(selector));
				_be;
			}
			if (_GetDesc_DPL(descriptor) != _GetCPL) {
				_bb("Desc_DPL(!CPL)");
				_chk(_SetExcept_GP(selector));
				_be;
			}
			if (!_IsDescPresent(descriptor)) {
				_bb("!DescPresent");
				_chk(_SetExcept_SS(selector));
				_be;
			}
			_SetDescUserAccessed(descriptor);
			_chk(_ksa_write_xdt(selector, GetRef(descriptor)));
			rsreg->flagvalid = 1;
			rsreg->selector = selector;
			rsreg->base = (t_nubit32)_GetDescSeg_Base(descriptor);
			rsreg->dpl = (t_nubit4)_GetDesc_DPL(descriptor);
			rsreg->limit = (t_nubit32)((_IsDescSegGranularLarge(descriptor) ?
				((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
			rsreg->seg.accessed = (t_bool)_IsDescUserAccessed(descriptor);
			rsreg->seg.executable = (t_bool)_IsDescUserExecutable(descriptor);
			rsreg->seg.data.big = (t_bool)_IsDescDataBig(descriptor);
			rsreg->seg.data.expdown = (t_bool)_IsDescDataExpDown(descriptor);
			rsreg->seg.data.writable = (t_bool)_IsDescDataWritable(descriptor);
			_be;
		} else {
			_bb("!Protected");
			rsreg->flagvalid = 1;
			rsreg->selector = selector;
			rsreg->base = (selector << 4);
			if (_GetCR0_PE && _GetEFLAGS_VM) {
				rsreg->dpl = 0x03;
				rsreg->limit = 0x0000ffff;
			}
			_be;
		}
		_be;break;
	case SREG_TR:
		_bb("sregtype(SREG_TR)");
		if (!_IsProtected) _impossible_;
		if (_GetCPL) _impossible_;
		if (_GetSelector_TI(selector)) _impossible_;
		if (_IsSelectorNull(selector)) {
			_bb("selector(null)");
			_chk(_SetExcept_GP(0));
			_be;
		}
		_chk(_ksa_read_xdt(selector, GetRef(descriptor)));
		if (!_IsDescTSSAvl(descriptor)) {
			_bb("!DescTssAvl");
			_chk(_SetExcept_GP(selector));
			_be;
		}
		if (!_IsDescPresent(descriptor)) {
			_bb("!DescPresent");
			_chk(_SetExcept_NP(selector));
			_be;
		}
		_SetDescTSSBusy(descriptor);
		_chk(_ksa_write_xdt(selector, GetRef(descriptor)));
		rsreg->flagvalid = 1;
		rsreg->selector = selector;
		rsreg->base = (t_nubit32)_GetDescSeg_Base(descriptor);
		rsreg->dpl = (t_nubit4)_GetDesc_DPL(descriptor);
		rsreg->limit = (t_nubit32)((_IsDescSegGranularLarge(descriptor) ?
				(_GetDescSeg_Limit(descriptor) << 12 | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
		rsreg->sys.type = (t_nubit4)_GetDesc_Type(descriptor);
		_be;break;
	case SREG_LDTR:
		_bb("sregtype(SREG_LDTR)");
		if (!_IsProtected) _impossible_;
		if (_GetCPL) _impossible_;
		if (_GetSelector_TI(selector)) _impossible_;
		if (_IsSelectorNull(selector)) {
			_bb("selector(null)");
			rsreg->flagvalid = 0;
			rsreg->selector = selector;
			_be;
		} else {
			_bb("selector(!null)");
			_chk(_ksa_read_xdt(selector, GetRef(descriptor)));
			if (!_IsDescLDT(descriptor)) {
				_bb("descriptor(!LDT)");
				_chk(_SetExcept_GP(selector));
				_be;
			}
			if (!_IsDescPresent(descriptor)) {
				_bb("descriptor(!P)");
				_chk(_SetExcept_NP(selector));
				_be;
			}
			rsreg->flagvalid = 1;
			rsreg->selector = selector;
			rsreg->base = (t_nubit32)_GetDescSeg_Base(descriptor);
			rsreg->dpl = (t_nubit4)_GetDesc_DPL(descriptor);
			rsreg->limit = (t_nubit32)((_IsDescSegGranularLarge(descriptor) ?
					(_GetDescSeg_Limit(descriptor) << 12 | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
			rsreg->sys.type = (t_nubit4)_GetDesc_Type(descriptor);
			_be;
		}
		_be;break;
	case SREG_GDTR:
	case SREG_IDTR:
	default:_impossible_;break;}
	_ce;
}

/* regular segment accessing */
static t_bool _s_check_selector(t_nubit16 selector)
{
	/* 0 = succ, 1 = fail */
	_cb("_s_check_selector");
	if (_IsSelectorNull(selector)) {
		_ce;
		return 1;
	}
	if (GetMax32(_GetSelector_Offset(selector) + 7) >
		(_GetSelector_TI(selector) ? GetMax32(vcpu.ldtr.limit) :
		GetMax32(vcpu.gdtr.limit))) {
		_ce;
		return 1;
	}
	_ce;
	return 0;
}
static void _s_read_idt(t_nubit8 intid, t_vaddrcc rdata)
{
	_cb("_s_read_idt");
	_chk(_ksa_read_idt(intid, rdata));
	_ce;
}
static void _s_read_xdt(t_nubit16 selector, t_vaddrcc rdata)
{
	_cb("_s_read_xdt");
	if (!_GetCR0_PE) _impossible_;
	_chk(_ksa_read_xdt(selector, rdata));
	_ce;
}
static void _s_read_tss(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_s_read_tss");
	_chk(_kma_read_logical(&vcpu.tr, offset, rdata, byte, 0, 1));
	_ce;
}
static void _s_read_es(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_s_read_es");
	_chk(_m_read_logical(&vcpu.es, offset, rdata, byte));
	_ce;
}
static void _s_read_cs(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_s_read_cs");
	_chk(_kma_read_logical(&vcpu.cs, offset, rdata, byte, 0, 1));
	_ce;
}
static void _s_read_ss(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_s_read_ss");
	_chk(_m_read_logical(&vcpu.ss, offset, rdata, byte));
	_ce;
}
static void _s_read_ds(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_s_read_ds");
	_impossible_;
	_ce;
}
static void _s_read_fs(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_s_read_fs");
	_impossible_;
	_ce;
}
static void _s_read_gs(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_s_read_gs");
	_impossible_;
	_ce;
}
static void _s_write_idt(t_nubit8 intid, t_vaddrcc rdata)
{
	_cb("_s_write_idt");
	_impossible_;
	_ce;
}
static void _s_write_xdt(t_nubit16 selector, t_vaddrcc rdata)
{
	_cb("_s_write_xdt");
	if (!_GetCR0_PE) _impossible_;
	_chk(_ksa_write_xdt(selector, rdata));
	_ce;
}
static void _s_write_tss(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_s_write_tss");
	_impossible_;
	_ce;
}
static void _s_write_es(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_s_write_es");
	_chk(_m_write_logical(&vcpu.es, offset, rdata, byte));
	_ce;
}
static void _s_write_cs(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_s_write_cs");
	_impossible_;
	_ce;
}
static void _s_write_ss(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_s_write_ss");
	_chk(_m_write_logical(&vcpu.ss, offset, rdata, byte));
	_ce;
}
static void _s_write_ds(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_s_write_ds");
	_impossible_;
	_ce;
}
static void _s_write_fs(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_s_write_fs");
	_impossible_;
	_ce;
}
static void _s_write_gs(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_s_write_gs");
	_impossible_;
	_ce;
}
static void _s_test_tss(t_nubit32 offset, t_nubit8 byte)
{
	_cb("_s_test_cs");
	_chk(_kma_test_logical(&vcpu.tr, offset, byte, 0, 0x00, 1));
	_ce;
}
static void _s_test_cs(t_nubit32 offset, t_nubit8 byte)
{
	_cb("_s_test_cs");
	_chk(_kma_test_logical(&vcpu.cs, offset, byte, 0, 0x00, 1));
	_ce;
}
static void _s_test_ss_push(t_nubit8 byte)
{
	t_nubit32 cesp = 0x00000000;
	_cb("_s_test_ss_push");
	switch (_GetStackSize) {
	case 2: _bb("StackSize(2)");
		if (vcpu.sp && vcpu.sp < byte)
			_chk(_SetExcept_SS(0));
		_chk(_m_test_access(&vcpu.ss, GetMax16(vcpu.sp - byte), byte, 1));
		_be;break;
	case 4: _bb("StackSize(4)");
		if (vcpu.esp && vcpu.esp < byte)
			_chk(_SetExcept_SS(0));
		_chk(_m_test_access(&vcpu.ss, GetMax32(vcpu.esp - byte), byte, 1));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void _s_test_ss_pop(t_nubit8 byte)
{
	t_nubit32 cesp = 0x00000000;
	_cb("_s_test_ss_pop");
	switch (_GetStackSize) {
	case 2: _bb("StackSize(2)");
		_chk(_m_test_access(&vcpu.ss, vcpu.sp, byte, 0));
		_be;break;
	case 4: _bb("StackSize(4)");
		_chk(_m_test_access(&vcpu.ss, vcpu.esp, byte, 0));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void _s_load_sreg(t_cpu_sreg *rsreg, t_nubit16 selector)
{
	_cb("_s_load_sreg");
	_chk(_ksa_load_sreg(rsreg, selector));
	_ce;
}
static void _s_load_gdtr(t_nubit32 base, t_nubit16 limit, t_nubit8 byte)
{
	_cb("_s_load_gdtr");
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	vcpu.gdtr.limit = limit;
	switch (byte) {
	case 2: vcpu.gdtr.base = GetMax24(base);break;
	case 4: vcpu.gdtr.base = GetMax32(base);break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;}
	_ce;
}
static void _s_load_idtr(t_nubit32 base, t_nubit16 limit, t_nubit8 byte)
{
	_cb("_s_load_idtr");
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	vcpu.idtr.limit = limit;
	switch (byte) {
	case 2: vcpu.idtr.base = GetMax24(base);break;
	case 4: vcpu.idtr.base = GetMax32(base);break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;}
	_ce;
}
static void _s_load_ldtr(t_nubit16 selector)
{
	_cb("_s_load_ldtr");
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	if (_GetSelector_TI(selector)) {
		_bb("Selector_TI(1)");
		_chk(_SetExcept_GP(selector));
		_be;
	}
	_chk(_s_load_sreg(&vcpu.ldtr, selector));
	_ce;
}
static void _s_load_tr(t_nubit16 selector)
{
	_cb("_s_load_tr");
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	if (_GetSelector_TI(selector)) {
		_bb("Selector_TI(1)");
		_chk(_SetExcept_GP(selector));
		_be;
	}
	_chk(_s_load_sreg(&vcpu.tr, selector));
	_ce;
}
static void _s_load_cr0_msw(t_nubit16 msw)
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
static void _s_load_cs(t_nubit16 newcs)
{
	_cb("_s_load_cs");
	_chk(_s_load_sreg(&vcpu.cs, newcs));
	_ce;
}
static void _s_load_ss(t_nubit16 newss)
{
	_cb("_s_load_ss");
	_chk(_s_load_sreg(&vcpu.ss, newss));
	_ce;
}
static void _s_load_ds(t_nubit16 newds)
{
	_cb("_s_load_ds");
	_chk(_s_load_sreg(&vcpu.ds, newds));
	_ce;
}
static void _s_load_es(t_nubit16 newes)
{
	_cb("_s_load_es");
	_chk(_s_load_sreg(&vcpu.es, newes));
	_ce;
}
static void _s_load_fs(t_nubit16 newfs)
{
	_cb("_s_load_fs");
	_chk(_s_load_sreg(&vcpu.fs, newfs));
	_ce;
}
static void _s_load_gs(t_nubit16 newgs)
{
	_cb("_s_load_gs");
	_chk(_s_load_sreg(&vcpu.gs, newgs));
	_ce;
}
static void _s_test_eip()
{
	_cb("_s_test_eip");
	_chk(_s_test_cs(vcpu.eip, 0x01));
	_ce;
}
static void _s_test_esp()
{
	t_nubit32 cesp;
	_cb("_s_test_esp");
	switch (_GetStackSize) {
	case 2: cesp = GetMax16(vcpu.esp);break;
	case 4: cesp = GetMax32(vcpu.esp);break;
	default:_impossible_;break;}
	_chk(_m_test_logical(&vcpu.ss, cesp, 0x02, 0));
	_ce;
}

/* portid accessing unit */
/* kernel portid accessing */
_______todo _kpa_test_iomap(t_nubit16 portid, t_nubit8 byte)
{
	/* TODO: iomap tester not implemented */
	_cb("_kpa_test_iomap");
	_ce;
}
static void _kpa_test_mode(t_nubit16 portid, t_nubit8 byte)
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
static void _p_input(t_nubit16 portid, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_p_input");
#ifndef VGLOBAL_BOCHS
	_chk(_kpa_test_mode(portid, byte));
	ExecFun(vport.in[portid]);
	switch (byte) {
	case 1: _bb("byte(1)");
		_chk(_m_write_ref(rdata, GetRef(vport.iobyte), 1));
		_be;break;
	case 2: _bb("byte(2)");
		_chk(_m_write_ref(rdata, GetRef(vport.ioword), 2));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_m_write_ref(rdata, GetRef(vport.iodword), 4));
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
#else
	vcpu.flagignore = 1;
#endif
	_ce;
}
static void _p_output(t_nubit16 portid, t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_p_output");
#ifndef VGLOBAL_BOCHS
	_chk(_kpa_test_mode(portid, byte));
	switch (byte) {
	case 1: _bb("byte(1)");
		_chk(_m_read_ref(rdata, GetRef(vport.iobyte), 1));
		_be;break;
	case 2: _bb("byte(2)");
		_chk(_m_read_ref(rdata, GetRef(vport.ioword), 2));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_m_read_ref(rdata, GetRef(vport.iodword), 4));
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	ExecFun(vport.out[portid]);
#else
	vcpu.flagignore = 1;
#endif
	_ce;
}

/* decoding unit */
/* kernel decoding function */
static t_bool _kdf_check_prefix(t_nubit8 opcode)
{
	switch (opcode) {
	case 0xf0:
	case 0xf2:
	case 0xf3:
	case 0x2e:
	case 0x36:
	case 0x3e:
	case 0x26:
		return 1;
		break;
	case 0x64:
	case 0x65:
	case 0x66:
	case 0x67:
		i386(opcode) return 1;
		else return 0;
		break;
	default:
		return 0;
		break;
	}
	return 0x00;
}

static void _kdf_skip(t_nubit8 byte)
{
	_cb("_kdf_skip");
	_chk(vcpu.eip += byte);
	_ce;
}
static void _kdf_code(t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_kdf_code");
	_chk(_s_read_cs(vcpu.eip, rdata, byte));
	_chk(_kdf_skip(byte));
	_ce;
}
static void _kdf_modrm(t_nubit8 regbyte, t_nubit8 rmbyte)
{
	t_nsbit8 disp8;
	t_nubit16 disp16;
	t_nubit32 disp32;
	t_nubit32 sibindex;
	t_nubit8 modrm, sib;
	_cb("_kdf_modrm");
	_chk(_kdf_code(GetRef(modrm), 1));
	vcpuins.flagmem = 1;
	vcpuins.mrm.rsreg = NULL;
	vcpuins.mrm.offset = 0;
	vcpuins.cr = vcpuins.crm = 0;
	vcpuins.rrm = vcpuins.rr = (t_vaddrcc)NULL;
	switch (_GetAddressSize) {
	case 2: _bb("AddressSize(2)");
		switch (_GetModRM_MOD(modrm)) {
		case 0: _bb("ModRM_MOD(0)");
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.mrm.offset = GetMax16(vcpu.bx + vcpu.si); vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 1: vcpuins.mrm.offset = GetMax16(vcpu.bx + vcpu.di); vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 2: vcpuins.mrm.offset = GetMax16(vcpu.bp + vcpu.si); vcpuins.mrm.rsreg = vcpuins.roverss;break;
			case 3: vcpuins.mrm.offset = GetMax16(vcpu.bp + vcpu.di); vcpuins.mrm.rsreg = vcpuins.roverss;break;
			case 4: vcpuins.mrm.offset = GetMax16(vcpu.si); vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 5: vcpuins.mrm.offset = GetMax16(vcpu.di); vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 6: _bb("ModRM_RM(6)");
				_chk(_kdf_code(GetRef(disp16), 2));
				vcpuins.mrm.offset = GetMax16(disp16); vcpuins.mrm.rsreg = vcpuins.roverds;
				_be;break;
			case 7: vcpuins.mrm.offset = GetMax16(vcpu.bx); vcpuins.mrm.rsreg = vcpuins.roverds;break;
			default:_impossible_;break;}
			_be;break;
		case 1: _bb("ModRM_MOD(1)");
			_chk(_kdf_code(GetRef(disp8), 1));
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.mrm.offset = GetMax16(vcpu.bx + vcpu.si + disp8); vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 1: vcpuins.mrm.offset = GetMax16(vcpu.bx + vcpu.di + disp8); vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 2: vcpuins.mrm.offset = GetMax16(vcpu.bp + vcpu.si + disp8); vcpuins.mrm.rsreg = vcpuins.roverss;break;
			case 3: vcpuins.mrm.offset = GetMax16(vcpu.bp + vcpu.di + disp8); vcpuins.mrm.rsreg = vcpuins.roverss;break;
			case 4: vcpuins.mrm.offset = GetMax16(vcpu.si + disp8); vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 5: vcpuins.mrm.offset = GetMax16(vcpu.di + disp8); vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 6: vcpuins.mrm.offset = GetMax16(vcpu.bp + disp8); vcpuins.mrm.rsreg = vcpuins.roverss;break;
			case 7: vcpuins.mrm.offset = GetMax16(vcpu.bx + disp8); vcpuins.mrm.rsreg = vcpuins.roverds;break;
			default:_impossible_;break;}
			_be;break;
		case 2: _bb("ModRM_MOD(2)");
			_chk(_kdf_code(GetRef(disp16), 2));
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.mrm.offset = GetMax16(vcpu.bx + vcpu.si + disp16); vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 1: vcpuins.mrm.offset = GetMax16(vcpu.bx + vcpu.di + disp16); vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 2: vcpuins.mrm.offset = GetMax16(vcpu.bp + vcpu.si + disp16); vcpuins.mrm.rsreg = vcpuins.roverss;break;
			case 3: vcpuins.mrm.offset = GetMax16(vcpu.bp + vcpu.di + disp16); vcpuins.mrm.rsreg = vcpuins.roverss;break;
			case 4: vcpuins.mrm.offset = GetMax16(vcpu.si + disp16); vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 5: vcpuins.mrm.offset = GetMax16(vcpu.di + disp16); vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 6: vcpuins.mrm.offset = GetMax16(vcpu.bp + disp16); vcpuins.mrm.rsreg = vcpuins.roverss;break;
			case 7: vcpuins.mrm.offset = GetMax16(vcpu.bx + disp16); vcpuins.mrm.rsreg = vcpuins.roverds;break;
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
		if (_GetModRM_MOD(modrm) != 3 && _GetModRM_RM(modrm) == 4) {
			_bb("ModRM_MOD(!3),ModRM_RM(4)");
			_chk(_kdf_code(GetRef(sib), 1));
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
			case 0: vcpuins.mrm.offset = vcpu.eax; vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 1: vcpuins.mrm.offset = vcpu.ecx; vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 2: vcpuins.mrm.offset = vcpu.edx; vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 3: vcpuins.mrm.offset = vcpu.ebx; vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 4:
				_bb("ModRM_RM(4)");
				switch (_GetSIB_Base(sib)) {
				case 0: vcpuins.mrm.offset = vcpu.eax + sibindex; vcpuins.mrm.rsreg = vcpuins.roverds;break;
				case 1: vcpuins.mrm.offset = vcpu.ecx + sibindex; vcpuins.mrm.rsreg = vcpuins.roverds;break;
				case 2: vcpuins.mrm.offset = vcpu.edx + sibindex; vcpuins.mrm.rsreg = vcpuins.roverds;break;
				case 3: vcpuins.mrm.offset = vcpu.ebx + sibindex; vcpuins.mrm.rsreg = vcpuins.roverds;break;
				case 4: vcpuins.mrm.offset = vcpu.esp + sibindex; vcpuins.mrm.rsreg = vcpuins.roverss;break;
				case 5: _bb("SIB_Base(5)");
					_chk(_kdf_code(GetRef(disp32), 4));
					vcpuins.mrm.offset = disp32 + sibindex; vcpuins.mrm.rsreg = vcpuins.roverds; 
					_be;break;
				case 6: vcpuins.mrm.offset = vcpu.esi + sibindex; vcpuins.mrm.rsreg = vcpuins.roverds;break;
				case 7: vcpuins.mrm.offset = vcpu.edi + sibindex; vcpuins.mrm.rsreg = vcpuins.roverds;break;
				default:_impossible_;break;}
				_be;break;
			case 5: _bb("ModRM_RM(5)");
				_chk(_kdf_code(GetRef(disp32), 4));
				vcpuins.mrm.offset = disp32; vcpuins.mrm.rsreg = vcpuins.roverds;
				_be;break;
			case 6: vcpuins.mrm.offset = vcpu.esi; vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 7: vcpuins.mrm.offset = vcpu.edi; vcpuins.mrm.rsreg = vcpuins.roverds;break;
			default:_impossible_;break;}
			_be;break;
		case 1: _bb("ModRM_MOD(1)");
			_chk(_kdf_code(GetRef(disp8), 1));
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.mrm.offset = vcpu.eax + disp8; vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 1: vcpuins.mrm.offset = vcpu.ecx + disp8; vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 2: vcpuins.mrm.offset = vcpu.edx + disp8; vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 3: vcpuins.mrm.offset = vcpu.ebx + disp8; vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 4: _bb("ModRM_RM(4)");
				switch (_GetSIB_Base(sib)) {
				case 0: vcpuins.mrm.offset = vcpu.eax + sibindex + disp8; vcpuins.mrm.rsreg = vcpuins.roverds;break;
				case 1: vcpuins.mrm.offset = vcpu.ecx + sibindex + disp8; vcpuins.mrm.rsreg = vcpuins.roverds;break;
				case 2: vcpuins.mrm.offset = vcpu.edx + sibindex + disp8; vcpuins.mrm.rsreg = vcpuins.roverds;break;
				case 3: vcpuins.mrm.offset = vcpu.ebx + sibindex + disp8; vcpuins.mrm.rsreg = vcpuins.roverds;break;
				case 4: vcpuins.mrm.offset = vcpu.esp + sibindex + disp8; vcpuins.mrm.rsreg = vcpuins.roverss;break;
				case 5: vcpuins.mrm.offset = vcpu.ebp + sibindex + disp8; vcpuins.mrm.rsreg = vcpuins.roverss;break;
				case 6: vcpuins.mrm.offset = vcpu.esi + sibindex + disp8; vcpuins.mrm.rsreg = vcpuins.roverds;break;
				case 7: vcpuins.mrm.offset = vcpu.edi + sibindex + disp8; vcpuins.mrm.rsreg = vcpuins.roverds;break;
				default:_impossible_;break;}
				_be;break;
			case 5: vcpuins.mrm.offset = vcpu.ebp + disp8; vcpuins.mrm.rsreg = vcpuins.roverss;break;
			case 6: vcpuins.mrm.offset = vcpu.esi + disp8; vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 7: vcpuins.mrm.offset = vcpu.edi + disp8; vcpuins.mrm.rsreg = vcpuins.roverds;break;
			default:_impossible_;break;}
			_be;break;
		case 2: _bb("ModRM_MOD(2)");
			_chk(_kdf_code(GetRef(disp32), 4));
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.mrm.offset = vcpu.eax + disp32; vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 1: vcpuins.mrm.offset = vcpu.ecx + disp32; vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 2: vcpuins.mrm.offset = vcpu.edx + disp32; vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 3: vcpuins.mrm.offset = vcpu.ebx + disp32; vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 4: _bb("ModRM_RM(4)");
				switch (_GetSIB_Base(sib)) {
				case 0: vcpuins.mrm.offset = vcpu.eax + sibindex + disp32; vcpuins.mrm.rsreg = vcpuins.roverds;break;
				case 1: vcpuins.mrm.offset = vcpu.ecx + sibindex + disp32; vcpuins.mrm.rsreg = vcpuins.roverds;break;
				case 2: vcpuins.mrm.offset = vcpu.edx + sibindex + disp32; vcpuins.mrm.rsreg = vcpuins.roverds;break;
				case 3: vcpuins.mrm.offset = vcpu.ebx + sibindex + disp32; vcpuins.mrm.rsreg = vcpuins.roverds;break;
				case 4: vcpuins.mrm.offset = vcpu.esp + sibindex + disp32; vcpuins.mrm.rsreg = vcpuins.roverss;break;
				case 5: vcpuins.mrm.offset = vcpu.ebp + sibindex + disp32; vcpuins.mrm.rsreg = vcpuins.roverss;break;
				case 6: vcpuins.mrm.offset = vcpu.esi + sibindex + disp32; vcpuins.mrm.rsreg = vcpuins.roverds;break;
				case 7: vcpuins.mrm.offset = vcpu.edi + sibindex + disp32; vcpuins.mrm.rsreg = vcpuins.roverds;break;
				default:_impossible_;break;}
				_be;break;
			case 5: vcpuins.mrm.offset = vcpu.ebp + disp32; vcpuins.mrm.rsreg = vcpuins.roverss;break;
			case 6: vcpuins.mrm.offset = vcpu.esi + disp32; vcpuins.mrm.rsreg = vcpuins.roverds;break;
			case 7: vcpuins.mrm.offset = vcpu.edi + disp32; vcpuins.mrm.rsreg = vcpuins.roverds;break;
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
		_chk(_m_read_ref(vcpuins.rrm, GetRef(vcpuins.crm), rmbyte));
		_be;
	}
	if (!regbyte) {
		/* reg is operation or segment */
		vcpuins.cr = _GetModRM_REG(modrm);
	} else {
		switch (regbyte) {
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
			break;
		default: _bb("regbyte");
			_chk(_SetExcept_CE(regbyte));
			_be;break;
		}
		_chk(_m_read_ref(vcpuins.rr, GetRef(vcpuins.cr), regbyte));
	}
	_ce;
}
static void _d_skip(t_nubit8 byte)
{
	_cb("_d_skip");
	_chk(_kdf_skip(byte));
	_ce;
}
static void _d_code(t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_d_code");
	_chk(_kdf_code(rdata, byte));
	_ce;
}
static void _d_imm(t_nubit8 byte)
{
	_cb("_d_imm");
	vcpuins.cimm = 0;
	_chk(_d_code(GetRef(vcpuins.cimm), byte));
	_ce;
}
static void _d_moffs(t_nubit8 byte)
{
	_cb("_d_moffs");
	vcpuins.flagmem = 1;
	vcpuins.mrm.rsreg = vcpuins.roverds;
	vcpuins.mrm.offset = 0;
	switch (_GetAddressSize) {
	case 2: _bb("AddressSize(2)");
		_chk(_d_code(GetRef(vcpuins.mrm.offset), 2));
		_be;break;
	case 4: _bb("AddressSize(4)");
		_chk(_d_code(GetRef(vcpuins.mrm.offset), 4));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void _d_modrm_sreg(t_nubit8 rmbyte)
{
	_cb("_d_modrm_sreg");
	_chk(_kdf_modrm(0, rmbyte));
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
static void _d_modrm_ea(t_nubit8 regbyte, t_nubit8 rmbyte)
{
	_cb("_d_modrm_ea");
	_chk(_kdf_modrm(regbyte, rmbyte));
	if (!vcpuins.flagmem) {
		_bb("flagmem(0)");
		_chk(_SetExcept_UD(0));
		_be;
	}
	_ce;
}
static void _d_modrm(t_nubit8 regbyte, t_nubit8 rmbyte)
{
	_cb("_d_modrm");
	_chk(_kdf_modrm(regbyte, rmbyte));
	if (!vcpuins.flagmem && vcpu.flaglock) {
		_bb("flagmem(0),flaglock(1)");
		_chk(_SetExcept_UD(0));
		_be;
	}
	_ce;
}

/* execution control unit: _e_ */
/* kernel execution control */
static void _kec_push(t_vaddrcc rdata, t_nubit8 byte)
{
	_cb("_kec_push");
	_chk(_s_test_ss_push(byte));
	switch (_GetStackSize) {
	case 2: _bb("StackSize(2)");
		vcpu.sp -= byte;
		_chk(_s_write_ss(vcpu.sp, rdata, byte));
		_be;break;
	case 4: _bb("StackSize(4)");
		vcpu.esp -= byte;
		_chk(_s_write_ss(vcpu.esp, rdata, byte));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static t_nubit64 _kec_pop(t_nubit8 byte)
{
	t_nubit64 result;
	_cb("_kec_pop");
	switch (_GetStackSize) {
	case 2: _bb("StackSize(2)");
		_chr(_s_read_ss(vcpu.sp, GetRef(result), byte));
		vcpu.sp += byte;
		_be;break;
	case 4: _bb("StackSize(4)");
		_chr(_s_read_ss(vcpu.esp, GetRef(result), byte));
		vcpu.esp += byte;
		_be;break;
	default:_impossible_r_;break;}
	_ce;
	return result;
}
static void _kec_call_far(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte)
{
	t_nubit32 oldcs = vcpu.cs.selector;
	t_cpu_sreg ccs = vcpu.cs;
	_cb("_kec_call_far");
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_s_test_ss_push(4));
		neweip = GetMax16(neweip);
		_chk(_ksa_load_sreg(&ccs, newcs));
		_chk(_kma_test_logical(&ccs, neweip, 0x01, 0, 0x00, 1));
		_chk(_kec_push(GetRef(oldcs), 2));
		_chk(_kec_push(GetRef(vcpu.ip), 2));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_s_test_ss_push(8));
		neweip = GetMax32(neweip);
		_chk(_ksa_load_sreg(&ccs, newcs));
		_chk(_kma_test_logical(&ccs, neweip, 0x01, 0, 0x00, 1));
		_chk(_kec_push(GetRef(oldcs), 4));
		_chk(_kec_push(GetRef(vcpu.eip), 4));
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	vcpu.cs = ccs;
	vcpu.eip = neweip;
	_ce;
}
static void _kec_call_near(t_nubit32 neweip, t_nubit8 byte)
{
	_cb("_kec_call_near");
	switch (byte) {
	case 2: _bb("byte(2)");
		neweip = GetMax16(neweip);
		_chk(_s_test_cs(neweip, 0x01));
		_chk(_kec_push(GetRef(vcpu.ip), 2));
		_be;break;
	case 4: _bb("byte(4)");
		neweip = GetMax32(neweip);
		_chk(_s_test_cs(neweip, 0x01));
		_chk(_kec_push(GetRef(vcpu.eip), 4));
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	vcpu.eip = neweip;
	_ce;
}
_______todo _kec_task_switch(t_nubit16 newtss);
static void _kec_jmp_far(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte)
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
	_chk(_ksa_load_sreg(&ccs, newcs));
	_chk(_kma_test_logical(&ccs, neweip, 0x01, 0, 0x00, 1));
	vcpu.cs = ccs;
	vcpu.eip = neweip;
	_ce;
}
static void _kec_jmp_near(t_nubit32 neweip, t_nubit8 byte)
{
	_cb("_kec_jmp_near");
	switch (byte) {
	case 2: neweip = GetMax16(neweip);break;
	case 4: neweip = GetMax32(neweip);break;
	default: _bb("bit");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_chk(_s_test_cs(neweip, 0x01));
	vcpu.eip = neweip;
	_ce;
}
static void _kec_ret_far(t_nubit32 newcs, t_nubit32 neweip, t_nubit16 parambyte, t_nubit16 byte)
{
	t_cpu_sreg ccs = vcpu.cs;
	_cb("_kec_ret_far");
	switch (byte) {
	case 2: neweip = GetMax16(neweip);break;
	case 4: neweip = GetMax32(neweip);break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_chk(_ksa_load_sreg(&ccs, newcs));
	_chk(_kma_test_logical(&ccs, neweip, 0x01, 0, 0x00, 1));
	vcpu.cs = ccs;
	vcpu.eip = neweip;
	switch (_GetStackSize) {
	case 2: vcpu.sp += parambyte;break;
	case 4: vcpu.esp += parambyte;break;
	default:_impossible_;break;}
	_ce;
}
static void _kec_ret_near(t_nubit16 parambyte, t_nubit8 byte)
{
	t_nubit32 neweip;
	_cb("_kec_ret_near");
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(neweip = GetMax16(_kec_pop(2)));
		_chk(_s_test_cs(GetMax16(neweip), 0x01));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(neweip = GetMax32(_kec_pop(4)));
		_chk(_s_test_cs(GetMax32(neweip), 0x01));
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	vcpu.eip = neweip;
	switch (_GetStackSize) {
	case 2: vcpu.sp += parambyte;break;
	case 4: vcpu.esp += parambyte;break;
	default:_impossible_;break;}
	_ce;
}
/* sub execution routine */
static void _ser_call_far_real(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte)
{
	_cb("_ser_call_far_real");
	if (_IsProtected) _impossible_;
	_chk(_kec_call_far(newcs, neweip, byte));
	_ce;
}
_______todo _ser_call_far_cs_conf(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte)
{
	t_nubit64 descriptor;
	_cb("_ser_call_far_cs_conf");
	if (!_IsProtected) _impossible_;
	_chk(_s_read_xdt(newcs, GetRef(descriptor)));
	if (!_IsDescCodeConform(descriptor)) _impossible_;
	if (_GetDesc_DPL(descriptor) > _GetCPL) {
		_bb("DPL(>CPL)");
		_chk(_SetExcept_GP(newcs));
		_be;
	}
	if (!_IsDescPresent(descriptor)) {
		_bb("!DescPresent");
		_chk(_SetExcept_NP(newcs));
		_be;
	}
	_chk(_kec_call_far(newcs, neweip, byte));
	_ce;
}
_______todo _ser_call_far_cs_nonc(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte)
{
	t_nubit64 descriptor;
	_cb("_ser_call_far_cs_nonc");
	if (!_IsProtected) _impossible_;
	_chk(_s_read_xdt(newcs, GetRef(descriptor)));
	if (!_IsDescCodeNonConform(descriptor)) _impossible_;
	if (_GetDesc_DPL(descriptor) != _GetCPL ||
		_GetSelector_RPL(newcs) > _GetCPL) {
		_bb("DPL(!CPL)/RPL(>CPL)");
		_chk(_SetExcept_GP(newcs));
		_be;
	}
	if (!_IsDescPresent(descriptor)) {
		_bb("!DescPresent");
		_chk(_SetExcept_NP(newcs));
		_be;
	}
	_chk(_kec_call_far(newcs, neweip, byte));
	_ce;
}
_______todo _ser_call_far_call_gate(t_nubit16 newcs)
{
	t_nubit64 descriptor;
	_cb("_ser_call_far_call_gate");
	if (!_IsProtected) _impossible_;
	_chk(_s_read_xdt(newcs, GetRef(descriptor)));
	if (!_IsDescCallGate(descriptor)) _impossible_;
	_chk(_SetExcept_CE(0));
	_ce;
}
_______todo _ser_call_far_task_gate(t_nubit16 newcs)
{
	t_nubit64 descriptor;
	_cb("_ser_call_far_task_gate");
	if (!_IsProtected) _impossible_;
	_chk(_s_read_xdt(newcs, GetRef(descriptor)));
	if (!_IsDescTaskGate(descriptor)) _impossible_;
	_chk(_SetExcept_CE(0));
	_ce;
}
_______todo _ser_call_far_tss(t_nubit16 newcs)
{
	t_nubit64 descriptor;
	_cb("_ser_call_far_tss");
	if (!_IsProtected) _impossible_;
	_chk(_s_read_xdt(newcs, GetRef(descriptor)));
	if (!_IsDescTSS(descriptor)) _impossible_;
	_chk(_SetExcept_CE(0));
	_ce;
}
static void _ser_int_real(t_nubit8 intid, t_nubit8 byte)
{
	t_nubit16 cip;
	t_nubit32 vector;
	t_nubit32 oldcs = vcpu.cs.selector;
	_cb("_ser_int_real");
	if (_IsProtected) _impossible_;
	if (GetMax16(intid * 4 + 3) > GetMax16(vcpu.idtr.limit)) {
		_bb("intid(>idtr.limit)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_s_test_ss_push(6));
		_chk(_kec_push(GetRef(vcpu.flags), 2));
		_ClrEFLAGS_IF;
		_ClrEFLAGS_TF;
		_chk(_kec_push(GetRef(oldcs), 2));
		_chk(_kec_push(GetRef(vcpu.ip), 2));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_s_test_ss_push(12));
		_chk(_kec_push(GetRef(vcpu.eflags), 4));
		_ClrEFLAGS_IF;
		_ClrEFLAGS_TF;
		_chk(_kec_push(GetRef(oldcs), 4));
		_chk(_kec_push(GetRef(vcpu.eip), 4));
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_chk(_s_read_idt(intid, GetRef(vector)));
	cip = GetMax16(vector);
	_chk(_s_test_cs(cip, 0x01));
	vcpu.eip = cip;
	_chk(_s_load_cs(GetMax16(vector >> 16)));
	_ce;
}
_______todo _ser_int_protected(t_nubit8 intid, t_nubit8 byte, t_bool flagext)
{
	t_nubit16 oldss;
	t_nubit32 oldeflags, oldesp, newesp = 0;
	t_nubit32 xs_sel;
	t_nubit16 newcs, newss = 0;
	t_nubit64 cs_desc, ss_desc, gate_desc;
	_cb("_ser_int_protected");
	if (!_GetCR0_PE) _impossible_;
	if (GetMax16(intid * 8 + 7) > GetMax16(vcpu.idtr.limit)) {
		_bb("intid(>idtr.limit)");
		_chk(_SetExcept_GP(intid * 8 + 2 + !!flagext));
		_be;
	}
	_chk(_s_read_idt(intid, GetRef(gate_desc)));
	switch (_GetDesc_Type(gate_desc)) {
	case VCPU_DESC_SYS_TYPE_TASKGATE:
	case VCPU_DESC_SYS_TYPE_INTGATE_16:
	case VCPU_DESC_SYS_TYPE_INTGATE_32:
	case VCPU_DESC_SYS_TYPE_TRAPGATE_16:
	case VCPU_DESC_SYS_TYPE_TRAPGATE_32:
		break;
	default: _bb("Desc_Type(!TaskGate/!IntGate/!TrapGate)");
		_chk(_SetExcept_GP(intid * 8 + 2 + !!flagext));
		_be;break;
	}
	if (!flagext) {
		_bb("!flagext");
		if (_GetDesc_DPL(gate_desc) < _GetCPL) {
			_bb("DPL(<CPL)");
			_chk(_SetExcept_GP(intid * 8 + 2));
			_be;
		}
		_be;
	}
	if (!_IsDescPresent(gate_desc)) {
		_bb("!DescPresent");
		_chk(_SetExcept_NP(intid * 8 + 2 + !!flagext));
		_be;
	}
	if (_GetDesc_Type(gate_desc) == VCPU_DESC_SYS_TYPE_TASKGATE) {
		_bb("Desc_Type(TaskGate)");
		_chk(_SetExcept_CE(0));
		_be;
	} else {
		_bb("Desc_Type(!TaskGate)");
		newcs = GetMax16(_GetDescGate_Selector(gate_desc));
		if (_IsSelectorNull(newcs)) {
			_bb("newcs(null)");
			_chk(_SetExcept_GP(!!flagext));
			_be;
		}
		if (_GetSelector_TI(newcs)) {
			_bb("Selector_TI(1)");
			if (GetMax16(_GetSelector_Offset(newcs) + 7) > GetMax16(vcpu.ldtr.limit)) {
				_bb("Selector_Offset(>ldtr.limit)");
				_chk(_SetExcept_GP(newcs & 0xfffc + !!flagext));
				_be;
			}
			_be;
		} else {
			_bb("Selector_TI(0)");
			if (GetMax16(_GetSelector_Offset(newcs) + 7) > GetMax16(vcpu.gdtr.limit)) {
				_bb("Selector_Offset(>gdtr.limit)");
				_chk(_SetExcept_GP(newcs & 0xfffc + !!flagext));
				_be;
			}
			_be;
		}
		_chk(_s_read_xdt(newcs, GetRef(cs_desc)));
		if (!_IsDescCode(cs_desc) ||
			_GetDesc_DPL(cs_desc) > _GetCPL) {
			_bb("!DescCode/DPL(>CPL)");
			_chk(_SetExcept_GP(newcs & 0xfffc + !!flagext));
			_be;
		}
		if (!_IsDescPresent(cs_desc)) {
			_bb("!DescPresent");
			_chk(_SetExcept_NP(newcs & 0xfffc + !!flagext));
			_be;
		}
		if (_IsDescCodeNonConform(cs_desc) &&
			_GetDesc_DPL(cs_desc) < _GetCPL) {
			_bb("DescCodeNonConform,DPL(<CPL)");
			if (!_GetEFLAGS_VM) {
				_bb("EFLAS_VM(0)");
				/* INTER-PRIVILEGE-LEVEL-INTERRUPT */
				_chk(_SetExcept_CE(0));
				_be;
			} else {
				_bb("EFLAS_VM(1)");
				if (_GetDesc_DPL(cs_desc) != 0) {
					_bb("DPL(!0)");
					_chk(_SetExcept_GP(newcs));
					_be;
				}
				/* INTERRUPT-FROM-VIRTUAL-8086-MODE */
				switch (vcpu.tr.sys.type) {
				case VCPU_DESC_SYS_TYPE_TSS_32_AVL:
				case VCPU_DESC_SYS_TYPE_TSS_32_BUSY:
					_bb("tr.type(32)");
					_chk(_s_test_tss(4, 8));
					_chk(_s_read_tss(4, GetRef(newesp), 4));
					_chk(_s_read_tss(8, GetRef(newss), 2));
					_be;break;
				case VCPU_DESC_SYS_TYPE_TSS_16_AVL:
				case VCPU_DESC_SYS_TYPE_TSS_16_BUSY:
					_bb("tr.type(16)");
					_chk(_s_test_tss(2, 4));
					_chk(_s_read_tss(2, GetRef(newesp), 2));
					_chk(_s_read_tss(4, GetRef(newss), 2));
					_be;break;
				default:_impossible_;break;}
				if (_IsSelectorNull(newss)) {
					_bb("newss(null)");
					_chk(_SetExcept_TS(!!flagext));
					_be;
				}
				if (_GetSelector_RPL(newss) != _GetDesc_DPL(cs_desc)) {
					_bb("RPL(newss)!=DPL(newcs)");
					_chk(_SetExcept_TS(newss & 0xfffc + !!flagext));
					_be;
				}
				_chk(_s_read_xdt(newss, GetRef(ss_desc)));
				if (_GetDesc_DPL(ss_desc) !=
					_GetDesc_DPL(cs_desc)) {
					_bb("DPL(newss)!=DPL(newcs)");
					_chk(_SetExcept_TS(newss & 0xfffc + !!flagext));
					_be;
				}
				if (!_IsDescPresent(ss_desc)) {
					_bb("!DescPresent(newss)");
					_chk(_SetExcept_SS(newss & 0xfffc + !!flagext));
					_be;
				}
				/*_chk(_ksa_load_sreg(&css, &ss_xdesc));
				if (_IsDescSys32(gate_desc)) {
					_bb("DescSys32(gate)");
					_chk(_kma_test_logical(&css, newesp, 36, 1, 0x00, 0));
					_be;
				} else {
					_bb("!DescSys32(gate)");
					_chk(_kma_test_logical(&css, newesp, 18, 1, 0x00, 0));
					_be;
				}*/
				oldeflags = vcpu.eflags;
				_ClrEFLAGS_VM;_MakeCPL(0x00);
				_ClrEFLAGS_TF;
				_ClrEFLAGS_NT;
				_ClrEFLAGS_RF;
				if (_IsDescIntGate(gate_desc))
					_ClrEFLAGS_IF;
				oldss = vcpu.ss.selector;
				oldesp = vcpu.esp;
				_chk(_ksa_load_sreg(&vcpu.ss, newss));
				vcpu.esp = newesp;
				if (_IsDescSys32(gate_desc)) {
					_bb("DescSys32(gate)");
					xs_sel = vcpu.gs.selector;_chk(_kec_push(GetRef(xs_sel), 4));
					xs_sel = vcpu.fs.selector;_chk(_kec_push(GetRef(xs_sel), 4));
					xs_sel = vcpu.ds.selector;_chk(_kec_push(GetRef(xs_sel), 4));
					xs_sel = vcpu.es.selector;_chk(_kec_push(GetRef(xs_sel), 4));
					xs_sel = oldss;_chk(_kec_push(GetRef(xs_sel), 4));
					_chk(_kec_push(GetRef(oldesp), 4));
					_chk(_kec_push(GetRef(oldeflags), 4));
					xs_sel = vcpu.cs.selector;_chk(_kec_push(GetRef(xs_sel), 4));
					_chk(_kec_push(GetRef(vcpu.eip), 4));
					_chk(_s_load_gs(0x0000));
					_chk(_s_load_fs(0x0000));
					_chk(_s_load_ds(0x0000));
					_chk(_s_load_es(0x0000));
					_chk(_ksa_load_sreg(&vcpu.cs, newcs));
					switch (byte) {
					case 2: vcpu.eip = GetMax16(_GetDescGate_Offset(gate_desc));break;
					case 4: vcpu.eip = GetMax32(_GetDescGate_Offset(gate_desc));break;
					default:_bb("byte");
						_chk(_SetExcept_CE(byte));
						_be;break;
					}
					_be;
				} else {
					_bb("!DescSys32(gate)");
					_chk(_SetExcept_CE(0));
					_be;
				}
				_be;
			}
			_be;
		} else {
			_bb("DescCodeConform/DPL(>=CPL)");
			_chk(_SetExcept_CE(0));
			_be;
		}
		_be;
	}
	_ce;
}
static void _ser_ret_far_real(t_nubit16 newcs, t_nubit32 neweip, t_nubit16 parambyte, t_nubit16 byte)
{
	_cb("_ser_ret_far_real");
	if (_IsProtected) _impossible_;
	_chk(_kec_ret_far(newcs, neweip, parambyte, byte));
	_ce;
}
static void _ser_ret_far_same(t_nubit16 newcs, t_nubit32 neweip, t_nubit16 parambyte, t_nubit16 byte)
{
	_cb("_ser_ret_far_same");
	if (!_IsProtected) _impossible_;
	_chk(_kec_ret_far(newcs, neweip, parambyte, byte));
	_ce;
}
_______todo _ser_ret_far_outer(t_nubit16 newcs, t_nubit32 neweip, t_nubit16 parambyte, t_nubit16 byte)
{
	_cb("_ser_ret_far_outer");
	if (!_IsProtected) _impossible_;
	_chk(_SetExcept_CE(0));
	_ce;
}
static void _ser_jmp_far_real(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte)
{
	_cb("_ser_jmp_far_real");
	if (_IsProtected) _impossible_;
	_chk(_kec_jmp_far(newcs, neweip, byte));
	_ce;
}
static void _ser_jmp_far_cs_conf(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte)
{
	t_nubit64 descriptor;
	_cb("_ser_jmp_far_cs_conf");
	if (!_IsProtected) _impossible_;
	_chk(_s_read_xdt(newcs, GetRef(descriptor)));
	if (!_IsDescCodeConform(descriptor)) _impossible_;
	if (_GetDesc_DPL(descriptor) > _GetCPL) {
		_bb("DPL(>CPL)");
		_chk(_SetExcept_GP(newcs));
		_be;
	}
	if (!_IsDescPresent(descriptor)) {
		_bb("!DescPresent");
		_chk(_SetExcept_NP(newcs));
		_be;
	}
	_chk(_kec_jmp_far(newcs, neweip, byte));
	_ce;
}
static void _ser_jmp_far_cs_nonc(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte)
{
	t_nubit64 descriptor;
	_cb("_ser_jmp_far_cs_nonc");
	if (!_IsProtected) _impossible_;
	_chk(_s_read_xdt(newcs, GetRef(descriptor)));
	if (!_IsDescCodeNonConform(descriptor)) _impossible_;
	if (_GetDesc_DPL(descriptor) != _GetCPL ||
		_GetSelector_RPL(newcs) > _GetCPL) {
		_bb("DPL(!CPL)/RPL(>CPL)");
		_chk(_SetExcept_GP(newcs));
		_be;
	}
	if (!_IsDescPresent(descriptor)) {
		_bb("!DescPresent");
		_chk(_SetExcept_NP(newcs));
		_be;
	}
	_chk(_kec_jmp_far(newcs, neweip, byte));
	_ce;
}
_______todo _ser_jmp_far_call_gate(t_nubit16 newcs)
{
	t_nubit64 descriptor;
	_cb("_ser_jmp_far_call_gate");
	if (!_IsProtected) _impossible_;
	_chk(_s_read_xdt(newcs, GetRef(descriptor)));
	if (!_IsDescCallGate(descriptor)) _impossible_;
	_chk(_SetExcept_CE(0));
	_ce;
}
_______todo _ser_jmp_far_task_gate(t_nubit16 newcs)
{
	t_nubit64 descriptor;
	_cb("_ser_jmp_far_task_gate");
	if (!_IsProtected) _impossible_;
	_chk(_s_read_xdt(newcs, GetRef(descriptor)));
	if (!_IsDescTaskGate(descriptor)) _impossible_;
	_chk(_SetExcept_CE(0));
	_ce;
}
_______todo _ser_jmp_far_tss(t_nubit16 newcs)
{
	t_nubit64 descriptor;
	_cb("_ser_jmp_far_tss");
	if (!_IsProtected) _impossible_;
	_chk(_s_read_xdt(newcs, GetRef(descriptor)));
	if (!_IsDescTSS(descriptor)) _impossible_;
	_chk(_SetExcept_CE(0));
	_ce;
}
/* regular execute control */
static void _e_push(t_nubit64 data, t_nubit8 byte)
{
	_cb("_e_push");
	_chk(_kec_push(GetRef(data), byte));
	_ce;
}
static t_nubit64 _e_pop(t_nubit8 byte)
{
	t_nubit64 result;
	_cb("_e_pop");
	_chr(result = _kec_pop(byte));
	_ce;
	return result;
}
static void _e_call_far(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte)
{
	t_nubit64 descriptor;
	_cb("_e_call_far");
	if (!_IsProtected) {
		_bb("!Protected");
		_chk(_ser_call_far_real(newcs, neweip, byte));
		_be;
	} else {
		_bb("Protected");
		if (_IsSelectorNull(newcs)) {
			_bb("newcs(null)");
			_chk(_SetExcept_GP(0));
			_be;
		}
		_chk(_s_read_xdt(newcs, GetRef(descriptor)));
		if (_IsDescCodeConform(descriptor))
			_chk(_ser_call_far_cs_conf(newcs, neweip, byte));
		else if (_IsDescCodeNonConform(descriptor))
			_chk(_ser_call_far_cs_nonc(newcs, neweip, byte));
		else if (_IsDescCallGate(descriptor))
			_chk(_ser_call_far_call_gate(newcs));
		else if (_IsDescTaskGate(descriptor))
			_chk(_ser_call_far_task_gate(newcs));
		else if (_IsDescTSS(descriptor))
			_chk(_ser_call_far_tss(newcs));
		else {
			_bb("desc(invalid)");
			_chk(_SetExcept_GP(newcs));
			_be;
		}
		_be;
	}
	_ce;
}
static void _e_call_near(t_nubit32 neweip, t_nubit8 byte)
{
	_cb("_e_call_near");
	_chk(_kec_call_near(neweip, byte));
	_ce;
}
_______todo _e_int3(t_nubit8 byte)
{
	_cb("_e_int3");
	if (!_GetCR0_PE) {
		_bb("Real");
		_chk(_ser_int_real(0x03, byte));
		_be;
	} else {
		_bb("!Real");
		_chk(_ser_int_protected(0x03, byte, 0));
		_be;
	}
	_ce;
}
_______todo _e_into(t_nubit8 byte)
{
	_cb("_e_into");
	if (_GetEFLAGS_OF) {
		_bb("EFLAGS_OF(1)");
		if (!_GetCR0_PE) {
			_bb("Real");
			_chk(_ser_int_real(0x04, byte));
			_be;
		} else {
			_bb("!Real");
			_chk(_ser_int_protected(0x04, byte, 0));
			_be;
		}
		_be;
	}
	_ce;
}
_______todo _e_int_n(t_nubit8 intid, t_nubit8 byte)
{
	_cb("_e_int_n");
	if (!_GetCR0_PE) {
		_bb("Real");
		_chk(_ser_int_real(intid, byte));
		_be;
	} else {
		_bb("!Real");
		if (_GetEFLAGS_VM && _GetEFLAGS_IOPL < 3) {
			_bb("EFLAGAS_VM(1),IOPL(<3)");
			_chk(_SetExcept_GP(0));
			_be;
		} else {
			_bb("EFLAGS_VM(0)/IOPL(3)");
			_chk(_ser_int_protected(intid, byte, 0));
			_be;
		}
		_be;
	}
	_ce;
}
_______todo _e_intr_n(t_nubit8 intid, t_nubit8 byte)
{
	_cb("_e_intr_n");
	if (!_GetCR0_PE) {
		_bb("Real");
		_chk(_ser_int_real(intid, byte));
		_be;
	} else {
		_bb("!Real");
		_chk(_ser_int_protected(intid, byte, 1));
		_be;
	}
	_ce;
}
_______todo _e_except_n(t_nubit8 exid, t_nubit8 byte)
{
	_cb("_e_except_n");
	vcpuins.except &= ~(1 << exid);
	if (!_GetCR0_PE) {
		_bb("Real");
		_chk(_ser_int_real(exid, byte));
		_be;
	} else {
		_bb("!Real");
		_chk(_SetExcept_UD(0));
		_be;
	}
	_ce;
}
_______todo _e_iret(t_nubit8 byte)
{
	t_nubit16 newcs, newss, newds, newes, newfs, newgs;
	t_nubit32 neweip, newesp;
	t_nubit32 neweflags;
	t_nubit32 mask = VCPU_EFLAGS_RESERVED;
	t_cpu_sreg ccs = vcpu.cs;
	_cb("_e_iret");
	if (!_GetCR0_PE) {
		_bb("Real");
		switch (byte) {
		case 2: _bb("byte(2)");
			_chk(_s_test_ss_pop(6));
			_chk(neweip    = GetMax16(_kec_pop(2)));
			_chk(newcs     = GetMax16(_kec_pop(2)));
			_chk(neweflags = GetMax16(_kec_pop(2)));
			mask |= 0xffff0000;
			_be;break;
		case 4: _bb("byte(4)");
			_chk(_s_test_ss_pop(12));
			_chk(neweip    = GetMax32(_kec_pop(4)));
			_chk(newcs     = GetMax16(_kec_pop(4)));
			_chk(neweflags = GetMax32(_kec_pop(4)));
			//vcpu.eflags = (neweflags & 0x00257fd5) | (vcpu.eflags & 0x001a0000);
			_be;break;
		default: _bb("byte");
			_chk(_SetExcept_CE(byte));
			_be;break;
		}
		_chk(_ksa_load_sreg(&ccs, newcs));
		_chk(_kma_test_logical(&ccs, neweip, 0x01, 0, 0x00, 1));
		vcpu.cs = ccs;
		vcpu.eip = neweip;
		vcpu.eflags = (neweflags & ~mask) | (vcpu.eflags & mask);
		_be;
	} else {
		_bb("!Real");
		if (_GetEFLAGS_VM) {
			_bb("V86");
			/* RETURN-FROM-VIRTUAL-8086-MODE */
			if (_GetEFLAGS_IOPL == 3) {
				_bb("EFLAGS_IOPL(3)");
				switch (byte) {
				case 2: _bb("byte(2)");
					_chk(_s_test_ss_pop(6));
					_chk(neweip    = GetMax16(_kec_pop(2)));
					_chk(newcs     = GetMax16(_kec_pop(2)));
					_chk(neweflags = GetMax16(_kec_pop(2)));
					mask |= (0xffff0000 | VCPU_EFLAGS_IOPL);
					_be;break;
				case 4: _bb("byte(4)");
					_chk(_s_test_ss_pop(12));
					_chk(neweip    = GetMax32(_kec_pop(4)));
					_chk(newcs     = GetMax16(_kec_pop(4)));
					_chk(neweflags = GetMax32(_kec_pop(4)));
					mask |= (VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL);
					_be;break;
				default: _bb("byte");
					_chk(_SetExcept_CE(byte));
					_be;break;
				}
				_chk(_ksa_load_sreg(&ccs, newcs));
				_chk(_kma_test_logical(&ccs, neweip, 0x01, 0, 0x00, 1));
				vcpu.cs = ccs;
				vcpu.eip = neweip;
				vcpu.eflags = (neweflags & ~mask) | (vcpu.eflags & mask);
				_be;
			} else {
				_bb("EFLAGS_IOPL(!3)");
				/* trap to virtual-8086 monitor */
				vcpuins.oldcpu = vcpu;
				_chk(_SetExcept_GP(0));
				_be;
			}
			_be;
		} else if (_GetEFLAGS_NT) {
			_bb("Nested");
			_chk(_SetExcept_CE(0));
			_be;
		} else {
			_bb("Protected,!Nested");
			switch (byte) {
			case 2: _bb("byte(2)");
				_chk(_s_test_ss_pop(6));
				_chk(neweip    = GetMax16(_kec_pop(2)));
				_chk(newcs     = GetMax16(_kec_pop(2)));
				_chk(neweflags = GetMax16(_kec_pop(2)));
				_be;break;
			case 4: _bb("byte(4)");
				_chk(_s_test_ss_pop(12));
				_chk(neweip    = GetMax32(_kec_pop(4)));
				_chk(newcs     = GetMax16(_kec_pop(4)));
				_chk(neweflags = GetMax32(_kec_pop(4)));
				_be;break;
			default: _bb("byte");
				_chk(_SetExcept_CE(byte));
				_be;break;
			}
			if (GetBit(neweflags, VCPU_EFLAGS_VM) && !_GetCPL) {
				_bb("neweflags(VM),CPL(0)");
				/* return to v86 */
				_chk(_s_test_ss_pop(24));
				vcpu.eflags = (neweflags & ~mask) | (vcpu.eflags & mask);
				_chk(_ksa_load_sreg(&ccs, newcs));
				_chk(_kma_test_logical(&ccs, neweip, 0x01, 0, 0x00, 1));
				vcpu.cs = ccs;
				vcpu.eip = neweip;
				_chk(newesp = GetMax32(_kec_pop(4)));
				_chk(newss = GetMax16(_kec_pop(4)));
				_chk(newes = GetMax16(_kec_pop(4)));
				_chk(newds = GetMax16(_kec_pop(4)));
				_chk(newfs = GetMax16(_kec_pop(4)));
				_chk(newgs = GetMax16(_kec_pop(4)));
				_chk(_s_load_ss(newss));
				vcpu.esp = newesp;
				_chk(_s_load_es(newes));
				_chk(_s_load_ds(newds));
				_chk(_s_load_fs(newfs));
				_chk(_s_load_gs(newgs));
				_MakeCPL(0x03);
				_be;
			} else {
				_bb("neweflags(!VM)/CPL(!0)");
				/* return to proctected */
				_chk(_SetExcept_CE(0));
				_be;
			}
			_be;
		}
		_be;
	}
	_ce;
}
static void _e_jcc(t_nubit32 csrc, t_nubit8 byte, t_bool condition)
{
	t_nubit32 neweip = vcpu.eip;
	_cb("_e_jcc");
	if (condition) {
		_bb("condition(1)");
		switch (byte) {
		case 1: neweip += (t_nsbit8)csrc;break;
		case 2: neweip += (t_nsbit16)csrc;break;
		case 4: neweip += (t_nsbit32)csrc;break;
		default: _bb("byte");
			_chk(_SetExcept_CE(byte));
			_be;break;
		}
		_chk(_kec_jmp_near(neweip, _GetOperandSize));
		_be;
	}
	_ce;
}
static void _e_jmp_far(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte)
{
	t_nubit64 descriptor;
	_cb("_e_jmp_far");
	if (!_IsProtected) {
		_bb("Protected(0)");
		_chk(_ser_jmp_far_real(newcs, neweip, byte));
		_be;
	} else {
		_bb("Protected(1)");
		if (_IsSelectorNull(newcs)) {
			_bb("selector(null)");
			_chk(_SetExcept_GP(0));
			_be;
		}
		_chk(_s_read_xdt(newcs, GetRef(descriptor)));
		if (_IsDescCodeConform(descriptor))
			_chk(_ser_jmp_far_cs_conf(newcs, neweip, byte));
		else if (_IsDescCodeNonConform(descriptor))
			_chk(_ser_jmp_far_cs_nonc(newcs, neweip, byte));
		else if (_IsDescCallGate(descriptor))
			_chk(_ser_jmp_far_call_gate(newcs));
		else if (_IsDescTaskGate(descriptor))
			_chk(_ser_jmp_far_task_gate(newcs));
		else if (_IsDescTSS(descriptor))
			_chk(_ser_jmp_far_tss(newcs));
		else {
			_bb("newcs(invalid)");
			_chk(_SetExcept_GP(newcs));
			_be;
		}
		_be;
	}
	vcpuins.opr1 = vcpu.cs.selector;
	vcpuins.opr2 = vcpu.eip;
	_ce;
}
static void _e_jmp_near(t_nubit32 neweip, t_nubit8 byte)
{
	_cb("_e_jmp_near");
	_chk(_kec_jmp_near(neweip, byte));
	_ce;
}
static void _e_load_far(t_cpu_sreg *rsreg, t_vaddrcc rdest, t_nubit16 selector, t_nubit32 offset, t_nubit8 byte)
{
	_cb("_e_load_far");
	_chk(_s_load_sreg(rsreg, selector));
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_m_write_ref(rdest, GetRef(offset), 2));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_m_write_ref(rdest, GetRef(offset), 4));
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_ce;
}
static void _e_loopcc(t_nsbit8 csrc, t_bool condition)
{
	t_nubit32 cecx;
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
		neweip += csrc;
		_chk(_kec_jmp_near(neweip, _GetOperandSize));
		_be;
	}
	_ce;
}
static void _e_ret_near(t_nubit16 parambyte, t_nubit8 byte)
{
	_cb("_e_ret_near");
	_chk(_kec_ret_near(parambyte, byte));
	_ce;
}
static void _e_ret_far(t_nubit16 parambyte, t_nubit16 byte)
{
	t_nubit16 newcs;
	t_nubit32 neweip;
	t_nubit64 descriptor;
	_cb("_e_ret_far");
	switch (byte) {
	case 2: _bb("byte(2)");
		_chk(_s_test_ss_pop(4));
		_chk(neweip = GetMax16(_kec_pop(2)));
		_chk(newcs = GetMax16(_kec_pop(2)));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_s_test_ss_pop(8));
		_chk(neweip = GetMax32(_kec_pop(4)));
		_chk(newcs = GetMax16(_kec_pop(4)));
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	if (!_IsProtected) {
		_bb("!Protected");
		_chk(_ser_ret_far_real(newcs, neweip, parambyte, byte));
		_be;
	} else {
		_bb("Protected");
		if (_IsSelectorNull(newcs)) {
			_bb("selector(null)");
			_chk(_SetExcept_GP(0));
			_be;
		}
		_chk(_s_read_xdt(newcs, GetRef(descriptor)));
		if (!_IsDescCode(descriptor)) {
			_bb("!DescCode");
			_chk(_SetExcept_GP(newcs));
			_be;
		}
		if (_GetSelector_RPL(newcs) < _GetCPL) {
			_bb("RPL(<CPL)");
			_chk(_SetExcept_GP(newcs));
			_be;
		}
		if (_IsDescCodeConform(descriptor)) {
			_bb("DescCodeConform");
			if (_GetDesc_DPL(descriptor) > _GetSelector_RPL(newcs)) {
				_bb("DPL(>RPL)");
				_chk(_SetExcept_GP(newcs));
				_be;
			}
			_be;
		}
		if (!_IsDescPresent(descriptor)) {
			_bb("!DescPresent");
			_chk(_SetExcept_NP(newcs));
			_be;
		}
		if (_GetSelector_RPL(newcs) > _GetCPL) {
			_bb("RPL(>CPL)");
			_chk(_ser_ret_far_outer(newcs, neweip, parambyte, byte));
			_be;
		} else {
			_bb("RPL(<=CPL)");
			_chk(_ser_ret_far_same(newcs, neweip, parambyte, byte));
			_be;
		}
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
#define NEG_FLAG  (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_PF)
#define NOT_FLAG  (0)
#define TEST_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SHL_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SHR_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SAL_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SAR_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define AAM_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define AAD_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define DAA_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define DAS_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)

static void _kaf_calc_CF()
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
static void _kaf_calc_OF()
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
static void _kaf_calc_AF()
{
	_cb("_kaf_calc_AF");
	MakeBit(vcpu.eflags, VCPU_EFLAGS_AF, ((vcpuins.opr1 ^ vcpuins.opr2) ^ vcpuins.result) & 0x10);
	_ce;
}
static void _kaf_calc_PF()
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
static void _kaf_calc_ZF()
{
	_cb("_kaf_calc_ZF");
	MakeBit(vcpu.eflags, VCPU_EFLAGS_ZF, !vcpuins.result);
	_ce;
}
static void _kaf_calc_SF()
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
static void _kaf_set_flags(t_nubit16 flags)
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
static void _kas_move_index(t_nubit8 byte, t_bool flagsi, t_bool flagdi)
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

#define _kac_arith1(funflag, type8, expr8, type16, expr16, type32, expr32) \
do { \
	switch (bit) { \
	case 8: \
		vcpuins.bit = 8; \
		vcpuins.type = (type8); \
		vcpuins.opr1 = GetMax8(cdest); \
		vcpuins.result = GetMax8(expr8); \
		break; \
	case 16: \
		vcpuins.bit = 16; \
		vcpuins.type = (type16); \
		vcpuins.opr1 = GetMax16(cdest); \
		vcpuins.result = GetMax16(expr16); \
		break; \
	case 32: \
		vcpuins.bit = 32; \
		vcpuins.type = (type32); \
		vcpuins.opr1 = GetMax32(cdest); \
		vcpuins.result = GetMax32(expr32); \
		break; \
	default: _bb("bit"); \
		_chk(_SetExcept_CE(bit)); \
		_be;break; \
	} \
	_chk(_kaf_set_flags(funflag)); \
} while (0)
#define _kac_arith2(funflag, type8, expr8, type12, expr12, type16, expr16, \
	type20, expr20, type32, expr32) \
do { \
	switch (bit) { \
	case 8: \
		vcpuins.bit = 8; \
		vcpuins.type = (type8); \
		vcpuins.opr1 = GetMax8(cdest); \
		vcpuins.opr2 = GetMax8(csrc); \
		vcpuins.result = GetMax8(expr8); \
		break; \
	case 12: \
		vcpuins.bit = 16; \
		vcpuins.type = (type12);\
		vcpuins.opr1 = GetMax16(cdest); \
		vcpuins.opr2 = GetMax16((t_nsbit8)csrc); \
		vcpuins.result = GetMax16(expr12); \
		break; \
	case 16: \
		vcpuins.bit = 16; \
		vcpuins.type = (type16); \
		vcpuins.opr1 = GetMax16(cdest); \
		vcpuins.opr2 = GetMax16(csrc); \
		vcpuins.result = GetMax16(expr16); \
		break; \
	case 20: \
		vcpuins.bit = 32; \
		vcpuins.type = (type20); \
		vcpuins.opr1 = GetMax32(cdest); \
		vcpuins.opr2 = GetMax32((t_nsbit8)csrc); \
		vcpuins.result = GetMax32(expr20); \
		break; \
	case 32: \
		vcpuins.bit = 32; \
		vcpuins.type = (type32); \
		vcpuins.opr1 = GetMax32(cdest); \
		vcpuins.opr2 = GetMax32(csrc); \
		vcpuins.result = GetMax32(expr32); \
		break; \
	default: _bb("bit"); \
		_chk(_SetExcept_CE(bit)); \
		_be;break; \
	} \
	_chk(_kaf_set_flags(funflag)); \
} while (0)
static void _a_add(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit)
{
	_cb("_a_add");
	_kac_arith2(ADD_FLAG,
		ADD8,  (vcpuins.opr1 + vcpuins.opr2),
		ADD16, (vcpuins.opr1 + vcpuins.opr2),
		ADD16, (vcpuins.opr1 + vcpuins.opr2),
		ADD32, (vcpuins.opr1 + vcpuins.opr2),
		ADD32, (vcpuins.opr1 + vcpuins.opr2));
	_ce;
}
static void _a_adc(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit)
{
	_cb("_a_adc");
	_kac_arith2(ADC_FLAG,
		ADC8,  (vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF),
		ADC16, (vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF),
		ADC16, (vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF),
		ADC32, (vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF),
		ADC32, (vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF));
	_ce;
}
static void _a_and(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit)
{
	_cb("_a_and");
	_kac_arith2(AND_FLAG,
		AND8,  (vcpuins.opr1 & vcpuins.opr2),
		AND16, (vcpuins.opr1 & vcpuins.opr2),
		AND16, (vcpuins.opr1 & vcpuins.opr2),
		AND32, (vcpuins.opr1 & vcpuins.opr2),
		AND32, (vcpuins.opr1 & vcpuins.opr2));
	_ClrEFLAGS_OF;
	_ClrEFLAGS_CF;
	vcpuins.udf |= VCPU_EFLAGS_AF;
	_ce;
}
static void _a_or(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit)
{
	_cb("_a_or");
	_kac_arith2(OR_FLAG,
		OR8,  (vcpuins.opr1 | vcpuins.opr2),
		OR16, (vcpuins.opr1 | vcpuins.opr2),
		OR16, (vcpuins.opr1 | vcpuins.opr2),
		OR32, (vcpuins.opr1 | vcpuins.opr2),
		OR32, (vcpuins.opr1 | vcpuins.opr2));
	_ClrEFLAGS_OF;
	_ClrEFLAGS_CF;
	vcpuins.udf |= VCPU_EFLAGS_AF;
	_ce;
}
static void _a_sbb(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit)
{
	_cb("_a_sbb");
	_kac_arith2(SBB_FLAG,
		SBB8,  (vcpuins.opr1 - (vcpuins.opr2 + _GetEFLAGS_CF)),
		SBB16, (vcpuins.opr1 - (vcpuins.opr2 + _GetEFLAGS_CF)),
		SBB16, (vcpuins.opr1 - (vcpuins.opr2 + _GetEFLAGS_CF)),
		SBB32, (vcpuins.opr1 - (vcpuins.opr2 + _GetEFLAGS_CF)),
		SBB32, (vcpuins.opr1 - (vcpuins.opr2 + _GetEFLAGS_CF)));
	_ce;
}
static void _a_sub(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit)
{
	_cb("_a_sub");
	_kac_arith2(SUB_FLAG,
		SUB8,  (vcpuins.opr1 - vcpuins.opr2),
		SUB16, (vcpuins.opr1 - vcpuins.opr2),
		SUB16, (vcpuins.opr1 - vcpuins.opr2),
		SUB32, (vcpuins.opr1 - vcpuins.opr2),
		SUB32, (vcpuins.opr1 - vcpuins.opr2));
	_ce;
}
static void _a_xor(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit)
{
	_cb("_a_xor");
	_kac_arith2(XOR_FLAG,
		XOR8,  (vcpuins.opr1 ^ vcpuins.opr2),
		XOR16, (vcpuins.opr1 ^ vcpuins.opr2),
		XOR16, (vcpuins.opr1 ^ vcpuins.opr2),
		XOR32, (vcpuins.opr1 ^ vcpuins.opr2),
		XOR32, (vcpuins.opr1 ^ vcpuins.opr2));
	_ClrEFLAGS_OF;
	_ClrEFLAGS_CF;
	vcpuins.udf |= VCPU_EFLAGS_AF;
	_ce;
}
static void _a_cmp(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit)
{
	_cb("_a_cmp");
	_kac_arith2(CMP_FLAG,
		CMP8,  ((t_nubit8)vcpuins.opr1 - (t_nsbit8)vcpuins.opr2),
		CMP16, ((t_nubit16)vcpuins.opr1 - (t_nsbit8)vcpuins.opr2),
		CMP16, ((t_nubit16)vcpuins.opr1 - (t_nsbit16)vcpuins.opr2),
		CMP32, ((t_nubit32)vcpuins.opr1 - (t_nsbit8)vcpuins.opr2),
		CMP32, ((t_nubit32)vcpuins.opr1 - (t_nsbit32)vcpuins.opr2));
	_ce;
}
static void _a_test(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit)
{
	_cb("_a_test");
	_kac_arith2(TEST_FLAG,
		TEST8,  (vcpuins.opr1 & vcpuins.opr2),
		TEST16, (vcpuins.opr1 & vcpuins.opr2),
		TEST16, (vcpuins.opr1 & vcpuins.opr2),
		TEST32, (vcpuins.opr1 & vcpuins.opr2),
		TEST32, (vcpuins.opr1 & vcpuins.opr2));
	_ClrEFLAGS_OF;
	_ClrEFLAGS_CF;
	vcpuins.udf |= VCPU_EFLAGS_AF;
	_ce;
}
static void _a_inc(t_nubit64 cdest, t_nubit8 bit)
{
	_cb("_a_inc");
	vcpuins.opr2 = 1;
	_kac_arith1(INC_FLAG,
		ADD8,  (vcpuins.opr1 + vcpuins.opr2),
		ADD16, (vcpuins.opr1 + vcpuins.opr2),
		ADD32, (vcpuins.opr1 + vcpuins.opr2));
	_ce;
}
static void _a_dec(t_nubit64 cdest, t_nubit8 bit)
{
	_cb("_a_dec");
	vcpuins.opr2 = 1;
	_kac_arith1(DEC_FLAG,
		SUB8,  (vcpuins.opr1 - vcpuins.opr2),
		SUB16, (vcpuins.opr1 - vcpuins.opr2),
		SUB32, (vcpuins.opr1 - vcpuins.opr2));
	_ce;
}
static void _a_not(t_nubit64 cdest, t_nubit8 bit)
{
	_cb("_a_not");
	vcpuins.opr2 = 0;
	_kac_arith1(NOT_FLAG,
		ARITHTYPE_NULL, (~vcpuins.opr1),
		ARITHTYPE_NULL, (~vcpuins.opr1),
		ARITHTYPE_NULL, (~vcpuins.opr1));
	_ce;
}
static void _a_neg(t_nubit64 cdest, t_nubit8 bit)
{
	_cb("_a_neg");
	vcpuins.opr2 = cdest;
	cdest = 0;
	_kac_arith1(NEG_FLAG,
		SUB8,  (GetMax8(vcpuins.opr1) - GetMax8(vcpuins.opr2)),
		SUB16, (GetMax16(vcpuins.opr1) - GetMax16(vcpuins.opr2)),
		SUB32, (GetMax32(vcpuins.opr1) - GetMax32(vcpuins.opr2)));
	MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!vcpuins.opr2);
	_ce;
}
static void _a_mul(t_nubit64 csrc, t_nubit8 bit)
{
	t_nubit64 cdest;
	_cb("_a_mul");
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		vcpuins.opr1 = vcpu.al;
		vcpuins.opr2 = GetMax8(csrc);
		cdest = GetMax16(vcpu.al * vcpuins.opr2);
		vcpu.ax = GetMax16(cdest);
		MakeBit(vcpu.eflags, VCPU_EFLAGS_OF, !!vcpu.ah);
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!vcpu.ah);
		vcpuins.result = GetMax16(cdest);
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		vcpuins.opr1 = vcpu.ax;
		vcpuins.opr2 = GetMax16(csrc);
		cdest = GetMax32(vcpu.ax * vcpuins.opr2);
		vcpu.dx = GetMax16(cdest >> 16);
		vcpu.ax = GetMax16(cdest);
		MakeBit(vcpu.eflags, VCPU_EFLAGS_OF, !!vcpu.dx);
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!vcpu.dx);
		vcpuins.result = GetMax32(cdest);
		_be;break;
	case 32: _bb("bit(32)");
		vcpuins.bit = 32;
		vcpuins.opr1 = vcpu.eax;
		vcpuins.opr2 = GetMax32(csrc);
		cdest = GetMax64(vcpu.eax * vcpuins.opr2);
		vcpu.edx = GetMax32(cdest >> 32);
		vcpu.eax = GetMax32(cdest);
		MakeBit(vcpu.eflags, VCPU_EFLAGS_OF, !!vcpu.edx);
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!vcpu.edx);
		vcpuins.result = GetMax64(cdest);
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	vcpuins.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
		VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
	_ce;
}
static void _a_imul(t_nubit64 csrc, t_nubit8 bit)
{
	t_nsbit64 cdest;
	_cb("_a_imul");
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		vcpuins.opr1 = vcpu.al;
		vcpuins.opr2 = GetMax8((t_nsbit8)csrc);
		cdest = GetMax16((t_nsbit8)vcpu.al * (t_nsbit8)vcpuins.opr2);
		vcpu.ax = GetMax16(cdest);
		if (GetMax16(cdest) == (t_nsbit16)((t_nsbit8)vcpu.al)) {
			_ClrEFLAGS_CF;
			_ClrEFLAGS_OF;
		} else {
			_SetEFLAGS_CF;
			_SetEFLAGS_OF;
		}
		vcpuins.result = GetMax16(cdest);
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		vcpuins.opr1 = vcpu.ax;
		vcpuins.opr2 = GetMax16((t_nsbit16)csrc);
		cdest = GetMax32((t_nsbit16)vcpu.ax * (t_nsbit16)vcpuins.opr2);
		vcpu.ax = GetMax16(cdest);
		vcpu.dx = GetMax16(cdest >> 16);
		if (GetMax32(cdest) == (t_nsbit32)((t_nsbit16)vcpu.ax)) {
			_ClrEFLAGS_CF;
			_ClrEFLAGS_OF;
		} else {
			_SetEFLAGS_CF;
			_SetEFLAGS_OF;
		}
		vcpuins.result = GetMax32(cdest);
		_be;break;
	case 32: _bb("bit(32)");
		vcpuins.bit = 32;
		vcpuins.opr1 = vcpu.eax;
		vcpuins.opr2 = GetMax32((t_nsbit32)csrc);
		cdest = GetMax64((t_nsbit32)vcpu.eax * (t_nsbit32)vcpuins.opr2);
		vcpu.eax = GetMax32(cdest);
		vcpu.edx = GetMax32(cdest >> 32);
		if (GetMax64(cdest) == (t_nsbit64)((t_nsbit32)vcpu.eax)) {
			_ClrEFLAGS_CF;
			_ClrEFLAGS_OF;
		} else {
			_SetEFLAGS_CF;
			_SetEFLAGS_OF;
		}
		vcpuins.result = GetMax64(cdest);
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	vcpuins.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
		VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
	_ce;
}
static void _a_div(t_nubit64 csrc, t_nubit8 bit)
{
	t_nubit64 temp = 0x0000000000000000;
	_cb("_a_div");
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		vcpuins.opr1 = GetMax16(vcpu.ax);
		vcpuins.opr2 = GetMax8(csrc);
		if(!vcpuins.opr2) {
			_bb("opr2(0)");
			_chk(_SetExcept_DE(0));
			_be;
		} else {
			_bb("opr2(!0)");
			temp = GetMax16((t_nubit16)vcpuins.opr1 / (t_nubit8)vcpuins.opr2);
			if (temp > Max8) {
				_bb("temp(>0xff)");
				_chk(_SetExcept_DE(0));
				_be;
			} else {
				vcpu.al = GetMax8(temp);
				vcpu.ah = GetMax8((t_nubit16)vcpuins.opr1 % (t_nubit8)vcpuins.opr2);
			}
			_be;
		}
		vcpuins.result = vcpu.ax;
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		vcpuins.opr1 = GetMax32((vcpu.dx << 16) | vcpu.ax);
		vcpuins.opr2 = GetMax16(csrc);
		if(!vcpuins.opr2) {
			_bb("opr2(0)");
			_chk(_SetExcept_DE(0));
			_be;
		} else {
			_bb("opr2(!0)");
			temp = GetMax32((t_nubit32)vcpuins.opr1 / (t_nubit16)vcpuins.opr2);
			if (temp > Max16) {
				_bb("temp(>0xffff)");
				_chk(_SetExcept_DE(0));
				_be;
			} else {
				vcpu.ax = GetMax16(temp);
				vcpu.dx = GetMax16((t_nubit32)vcpuins.opr1 % (t_nubit16)vcpuins.opr2);
			}
			_be;
		}
		vcpuins.result = (vcpu.dx << 16) | vcpu.ax;
		_be;break;
	case 32: _bb("bit(32)");
		vcpuins.bit = 32;
		vcpuins.opr1 = GetMax64(((t_nubit64)vcpu.edx << 32) | vcpu.eax);
		vcpuins.opr2 = GetMax32(csrc);
		if(!vcpuins.opr2) {
			_bb("opr2(0)");
			_chk(_SetExcept_DE(0));
			_be;
		} else {
			_bb("opr2(!0)");
			temp = GetMax64((t_nubit64)vcpuins.opr1 / (t_nubit32)vcpuins.opr2);
			if (temp > Max32) {
				_bb("temp(>0xffffffff)");
				_chk(_SetExcept_DE(0));
				_be;
			} else {
				vcpu.eax = GetMax32(temp);
				vcpu.edx = GetMax32((t_nubit64)vcpuins.opr1 % (t_nubit32)vcpuins.opr2);
			}
			_be;
		}
		vcpuins.result = ((t_nubit64)vcpu.edx << 32) | vcpu.eax;
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	vcpuins.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF |
		VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_OF);
	_ce;
}
static void _a_idiv(t_nubit64 csrc, t_nubit8 bit)
{
	t_nsbit64 temp;
	_cb("_a_idiv");
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		vcpuins.opr1 = GetMax16((t_nsbit16)vcpu.ax);
		vcpuins.opr2 = GetMax8((t_nsbit8)csrc);
		if(!vcpuins.opr2) {
			_bb("opr2(0)");
			_chk(_SetExcept_DE(0));
			_be;
		} else {
			_bb("opr2(!0)");
			temp = GetMax16((t_nsbit16)vcpuins.opr1 / (t_nsbit8)vcpuins.opr2);
			if ((temp > 0 && GetMax8(temp) > 0x7f) ||
				(temp < 0 && GetMax8(temp) < 0x80)) {
				_bb("temp(>0x7f/<0x80)");
				_chk(_SetExcept_DE(0));
				_be;
			} else {
				vcpu.al = GetMax8(temp);
				vcpu.ah = GetMax8((t_nsbit16)vcpuins.opr1 % (t_nsbit8)vcpuins.opr2);
			}
			_be;
		}
		vcpuins.result = vcpu.ax;
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		vcpuins.opr1 = GetMax32((t_nsbit32)((vcpu.dx << 16) | vcpu.ax));
		vcpuins.opr2 = GetMax16((t_nsbit16)csrc);
		if(!vcpuins.opr2) {
			_bb("opr2(0)");
			_chk(_SetExcept_DE(0));
			_be;
		} else {
			_bb("opr2(!0)");
			temp = GetMax32((t_nsbit32)vcpuins.opr1 / (t_nsbit16)vcpuins.opr2);
			if ((temp > 0 && GetMax16(temp) > 0x7fff) ||
				(temp < 0 && GetMax16(temp) < 0x8000)) {
				_bb("temp(>0x7fff/<0x8000)");
				_chk(_SetExcept_DE(0));
				_be;
			} else {
				vcpu.ax = GetMax16(temp);
				vcpu.dx = GetMax16((t_nsbit32)vcpuins.opr1 % (t_nsbit16)vcpuins.opr2);
			}
			_be;
		}
		vcpuins.result = ((t_nubit32)vcpu.dx << 16) | vcpu.ax;
		_be;break;
	case 32: _bb("bit(32)");
		vcpuins.bit = 32;
		vcpuins.opr1 = GetMax64((t_nsbit64)(((t_nubit64)vcpu.edx << 32) | vcpu.eax));
		vcpuins.opr2 = GetMax32((t_nsbit32)csrc);
		if(!vcpuins.opr2) {
			_bb("opr2(0)");
			_chk(_SetExcept_DE(0));
			_be;
		} else {
			_bb("opr2(!0)");
			temp = GetMax64((t_nsbit64)vcpuins.opr1 / (t_nsbit32)vcpuins.opr2);
			if ((temp > 0 && GetMax32(temp) > 0x7fffffff) ||
				(temp < 0 && GetMax32(temp) < 0x80000000)) {
				_bb("temp(>0x7fffffff/<0x80000000)");
				_chk(_SetExcept_DE(0));
				_be;
			} else {
				vcpu.eax = GetMax32(temp);
				vcpu.edx = GetMax32((t_nsbit64)vcpuins.opr1 % (t_nsbit32)vcpuins.opr2);
			}
			_be;
		}
		vcpuins.result = ((t_nubit64)vcpu.edx << 32) | vcpu.eax;
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	vcpuins.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF |
		VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_OF);
	_ce;
}

static void _a_imul3(t_nubit64 csrc1, t_nubit64 csrc2, t_nubit8 bit)
{
	t_nsbit64 cdest;
	_cb("_a_imul3");
	switch (bit) {
	case 12: _bb("bit(16+8)");
		vcpuins.bit = 16;
		vcpuins.opr1 = (t_nsbit16)csrc1;
		vcpuins.opr2 = (t_nsbit8)csrc2;
		cdest = GetMax32((t_nsbit16)vcpuins.opr1 * (t_nsbit8)vcpuins.opr2);
		vcpuins.result = GetMax16(cdest);
		if (GetMax32(cdest) != GetMax32((t_nsbit16)vcpuins.result)) {
			_SetEFLAGS_CF;
			_SetEFLAGS_OF;
		} else {
			_ClrEFLAGS_CF;
			_ClrEFLAGS_OF;
		}
		_be;break;
	case 16: _bb("bit(16+16)");
		vcpuins.bit = 16;
		vcpuins.opr1 = (t_nsbit16)csrc1;
		vcpuins.opr2 = (t_nsbit16)csrc2;
		cdest = GetMax32((t_nsbit16)vcpuins.opr1 * (t_nsbit16)vcpuins.opr2);
		vcpuins.result = GetMax16(cdest);
		if (GetMax32(cdest) != GetMax32((t_nsbit16)vcpuins.result)) {
			_SetEFLAGS_CF;
			_SetEFLAGS_OF;
		} else {
			_ClrEFLAGS_CF;
			_ClrEFLAGS_OF;
		}
		_be;break;
	case 20: _bb("bit(32+8)");
		vcpuins.bit = 32;
		vcpuins.opr1 = (t_nsbit32)csrc1;
		vcpuins.opr2 = (t_nsbit8)csrc2;
		cdest = GetMax64((t_nsbit32)vcpuins.opr1 * (t_nsbit8)vcpuins.opr2);
		vcpuins.result = GetMax32(cdest);
		if (GetMax64(cdest) != GetMax64((t_nsbit32)vcpuins.result)) {
			_SetEFLAGS_CF;
			_SetEFLAGS_OF;
		} else {
			_ClrEFLAGS_CF;
			_ClrEFLAGS_OF;
		}
		_be;break;
	case 32: _bb("bit(32+32");
		vcpuins.bit = 32;
		vcpuins.opr1 = (t_nsbit32)csrc1;
		vcpuins.opr2 = (t_nsbit32)csrc2;
		cdest = GetMax64((t_nsbit32)vcpuins.opr1 * (t_nsbit32)vcpuins.opr2);
		vcpuins.result = GetMax32(cdest);
		if (GetMax64(cdest) != GetMax64((t_nsbit32)vcpuins.result)) {
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

static void _a_rol(t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit)
{
	t_nubit8 count;
	t_nubit32 flagcf;
	_cb("_a_rol");
	switch (bit) {
	case 8: _bb("bit(8)");
		count = csrc & 0x07;
		vcpuins.bit = 8;
		vcpuins.opr1 = GetMax8(cdest);
		vcpuins.opr2 = count;
		vcpuins.result = vcpuins.opr1;
		while (count) {
			flagcf = !!GetMSB8(vcpuins.result);
			vcpuins.result = GetMax8(vcpuins.result << 1) | flagcf;
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!GetLSB8(vcpuins.result));
			count--;
		}
		if(vcpuins.opr2 == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB8(vcpuins.result)) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		_be;break;
	case 16: _bb("bit(16)");
		count = csrc & 0x0f;
		vcpuins.bit = 16;
		vcpuins.opr1 = GetMax16(cdest);
		vcpuins.opr2 = count;
		vcpuins.result = vcpuins.opr1;
		while (count) {
			flagcf = !!GetMSB16(vcpuins.result);
			vcpuins.result = GetMax16(vcpuins.result << 1) | flagcf;
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!GetLSB16(vcpuins.result));
			count--;
		}
		if(vcpuins.opr2 == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB16(vcpuins.result)) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		_be;break;
	case 32: _bb("bit(32)");
		count = csrc & 0x1f;
		vcpuins.bit = 32;
		vcpuins.opr1 = GetMax32(cdest);
		vcpuins.opr2 = count;
		vcpuins.result = vcpuins.opr1;
		while (count) {
			flagcf = !!GetMSB32(vcpuins.result);
			vcpuins.result = GetMax32(vcpuins.result << 1) | flagcf;
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!GetLSB32(vcpuins.result));
			count--;
		}
		if(vcpuins.opr2 == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB32(vcpuins.result)) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
static void _a_ror(t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit)
{
	t_nubit8 count;
	t_nubit32 flagcf;
	_cb("_a_ror");
	switch (bit) {
	case 8: _bb("bit(8)");
		count = csrc & 0x07;
		vcpuins.bit = 8;
		vcpuins.opr1 = GetMax8(cdest);
		vcpuins.opr2 = count;
		vcpuins.result = vcpuins.opr1;
		while (count) {
			flagcf = GetLSB8(vcpuins.result) ? MSB8 : 0;
			vcpuins.result = GetMax8(vcpuins.result >> 1) | flagcf;
			count--;
		}
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!GetMSB8(vcpuins.result));
		if(vcpuins.opr2 == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
			((!!GetMSB8(vcpuins.result)) ^ (!!GetMSB7(vcpuins.result))));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		_be;break;
	case 16: _bb("bit(16)");
		count = csrc & 0x0f;
		vcpuins.bit = 16;
		vcpuins.opr1 = GetMax16(cdest);
		vcpuins.opr2 = count;
		vcpuins.result = vcpuins.opr1;
		while (count) {
			flagcf = GetLSB16(vcpuins.result) ? MSB16 : 0;
			vcpuins.result = GetMax16(vcpuins.result >> 1) | flagcf;
			count--;
		}
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!GetMSB16(vcpuins.result));
		if(vcpuins.opr2 == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
			((!!GetMSB16(vcpuins.result)) ^ (!!GetMSB15(vcpuins.result))));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		_be;break;
	case 32: _bb("bit(32)");
		count = csrc & 0x1f;
		vcpuins.bit = 32;
		vcpuins.opr1 = GetMax32(cdest);
		vcpuins.opr2 = count;
		vcpuins.result = vcpuins.opr1;
		while (count) {
			flagcf = GetLSB32(vcpuins.result) ? MSB32 : 0;
			vcpuins.result = GetMax32(vcpuins.result >> 1) | flagcf;
			count--;
		}
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!GetMSB32(vcpuins.result));
		if(vcpuins.opr2 == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
			((!!GetMSB32(vcpuins.result)) ^ (!!GetMSB31(vcpuins.result))));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
static void _a_rcl(t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit)
{
	t_nubit8 count;
	t_nubit32 flagcf;
	_cb("_a_rcl");
	switch (bit) {
	case 8: _bb("bit(8)");
		count = (csrc & 0x1f) % 9;
		vcpuins.bit = 8;
		vcpuins.opr1 = GetMax8(cdest);
		vcpuins.opr2 = count;
		vcpuins.result = vcpuins.opr1;
		while (count) {
			flagcf = !!GetMSB8(vcpuins.result);
			vcpuins.result = GetMax8(vcpuins.result << 1) | _GetEFLAGS_CF;
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, flagcf);
			count--;
		}
		if(vcpuins.opr2 == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB8(vcpuins.result)) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		_be;break;
	case 16: _bb("bit(16)");
		count = (csrc & 0x1f) % 17;
		vcpuins.bit = 16;
		vcpuins.opr1 = GetMax16(cdest);
		vcpuins.opr2 = count;
		vcpuins.result = vcpuins.opr1;
		while (count) {
			flagcf = !!GetMSB16(vcpuins.result);
			vcpuins.result = GetMax16(vcpuins.result << 1) | _GetEFLAGS_CF;
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, flagcf);
			count--;
		}
		if(vcpuins.opr2 == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB16(vcpuins.result)) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		_be;break;
	case 32: _bb("bit(32)");
		count = (csrc & 0x1f);
		vcpuins.bit = 32;
		vcpuins.opr1 = GetMax32(cdest);
		vcpuins.opr2 = count;
		vcpuins.result = vcpuins.opr1;
		while (count) {
			flagcf = !!GetMSB32(vcpuins.result);
			vcpuins.result = GetMax32(vcpuins.result << 1) | _GetEFLAGS_CF;
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, flagcf);
			count--;
		}
		if(vcpuins.opr2 == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB32(vcpuins.result)) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
static void _a_rcr(t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit)
{
	t_nubit8 count;
	t_nubit32 flagcf;
	_cb("_a_rcr");
	switch (bit) {
	case 8: _bb("bit(8)");
		count = (csrc & 0x1f) % 9;
		vcpuins.bit = 8;
		vcpuins.opr1 = GetMax8(cdest);
		vcpuins.opr2 = count;
		vcpuins.result = vcpuins.opr1;
		if(vcpuins.opr2 == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB8(vcpuins.result)) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		while (count) {
			flagcf = GetLSB8(vcpuins.result);
			vcpuins.result = GetMax8(vcpuins.result >> 1) | (_GetEFLAGS_CF ? MSB8 : 0);
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!flagcf);
			count--;
		}
		_be;break;
	case 16: _bb("bit(16)");
		count = (csrc & 0x1f) % 17;
		vcpuins.bit = 16;
		vcpuins.opr1 = GetMax16(cdest);
		vcpuins.opr2 = count;
		vcpuins.result = vcpuins.opr1;
		if(vcpuins.opr2 == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB16(vcpuins.result)) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		while (count) {
			flagcf = GetLSB16(vcpuins.result);
			vcpuins.result = GetMax16(vcpuins.result >> 1) | (_GetEFLAGS_CF ? MSB16 : 0);
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!flagcf);
			count--;
		}
		_be;break;
	case 32: _bb("bit(32)");
		count = (csrc & 0x1f);
		vcpuins.bit = 32;
		vcpuins.opr1 = GetMax32(cdest);
		vcpuins.opr2 = count;
		vcpuins.result = vcpuins.opr1;
		if(vcpuins.opr2 == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB32(vcpuins.result)) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		while (count) {
			flagcf = GetLSB32(vcpuins.result);
			vcpuins.result = GetMax32(vcpuins.result >> 1) | (_GetEFLAGS_CF ? MSB32 : 0);
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!flagcf);
			count--;
		}
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
static void _a_shl(t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit)
{
	t_nubit8 count;
	_cb("_a_shl");
	count = csrc & 0x1f;
	vcpuins.opr2 = count;
	if (count >= bit) vcpuins.udf |= VCPU_EFLAGS_CF;
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		vcpuins.opr1 = GetMax8(cdest);
		vcpuins.result = vcpuins.opr1;
		while (count) {
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, !!GetMSB8(vcpuins.result));
			vcpuins.result = GetMax8(vcpuins.result << 1);
			count--;
		}
		if(vcpuins.opr2 == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB8(vcpuins.result)) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		if(vcpuins.opr2 != 0) {
			_bb("opr2(!0)");
			_chk(_kaf_set_flags(SHL_FLAG));
			vcpuins.udf |= VCPU_EFLAGS_AF;
			_be;
		}
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		vcpuins.opr1 = GetMax16(cdest);
		vcpuins.result = vcpuins.opr1;
		while (count) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF, !!GetMSB16(vcpuins.result));
			vcpuins.result = GetMax16(vcpuins.result << 1);
			count--;
		}
		if(vcpuins.opr2 == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB16(vcpuins.result)) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		if(vcpuins.opr2 != 0) {
			_bb("opr2(!0)");
			_chk(_kaf_set_flags(SHL_FLAG));
			vcpuins.udf |= VCPU_EFLAGS_AF;
			_be;
		}
		_be;break;
	case 32: _bb("bit(32)");
		vcpuins.bit = 32;
		vcpuins.opr1 = GetMax32(cdest);
		vcpuins.result = vcpuins.opr1;
		while (count) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF, !!GetMSB32(vcpuins.result));
			vcpuins.result = GetMax32(vcpuins.result << 1);
			count--;
		}
		if(vcpuins.opr2 == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
				((!!GetMSB32(vcpuins.result)) ^ _GetEFLAGS_CF));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		if(vcpuins.opr2 != 0) {
			_bb("opr2(!0)");
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
static void _a_shr(t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit)
{
	t_nubit8 count;
	_cb("_a_shr");
	count = csrc & 0x1f;
	vcpuins.opr2 = count;
	if (count >= bit) vcpuins.udf |= VCPU_EFLAGS_CF;
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		vcpuins.opr1 = GetMax8(cdest);
		vcpuins.result = vcpuins.opr1;
		while (count) {
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (!!GetLSB8(vcpuins.result)));
			vcpuins.result = GetMax8(vcpuins.result >> 1);
			count--;
		}
		if(vcpuins.opr2 == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF, (!!GetMSB8(vcpuins.opr1)));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		if(vcpuins.opr2 != 0) {
			_bb("opr2(!0)");
			_chk(_kaf_set_flags(SHR_FLAG));
			vcpuins.udf |= VCPU_EFLAGS_AF;
			_be;
		}
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		vcpuins.opr1 = GetMax16(cdest);
		vcpuins.result = vcpuins.opr1;
		while (count) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF, (!!GetLSB16(vcpuins.result)));
			vcpuins.result = GetMax16(vcpuins.result >> 1);
			count--;
		}
		if(vcpuins.opr2 == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF, (!!GetMSB16(vcpuins.opr1)));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		if(vcpuins.opr2 != 0) {
			_bb("opr2(!0)");
			_chk(_kaf_set_flags(SHR_FLAG));
			vcpuins.udf |= VCPU_EFLAGS_AF;
			_be;
		}
		_be;break;
	case 32: _bb("bit(32)");
		vcpuins.bit = 32;
		vcpuins.opr1 = GetMax32(cdest);
		vcpuins.result = vcpuins.opr1;
		while (count) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF, (!!GetLSB32(vcpuins.result)));
			vcpuins.result = GetMax32(vcpuins.result >> 1);
			count--;
		}
		if(vcpuins.opr2 == 1)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_OF, (!!GetMSB32(vcpuins.opr1)));
		else
			vcpuins.udf |= VCPU_EFLAGS_OF;
		if(vcpuins.opr2 != 0) {
			_bb("opr2(!0)");
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
static void _a_sar(t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit)
{
	t_nubit8 count;
	t_bool tempcf;
	_cb("_a_shr");
	count = csrc & 0x1f;
	vcpuins.opr2 = count;
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		vcpuins.opr1 = GetMax8(cdest);
		vcpuins.opr2 = count;
		vcpuins.result = vcpuins.opr1;
		while (count) {
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (!!GetLSB8(vcpuins.result)));
			tempcf = GetMSB8(vcpuins.result);
			vcpuins.result = GetMax8((t_nsbit8)vcpuins.result >> 1);
			count--;
		}
		if(vcpuins.opr2 == 1) _ClrEFLAGS_OF;
		else vcpuins.udf |= VCPU_EFLAGS_OF;
		if(vcpuins.opr2 != 0) {
			_bb("opr2(!0)");
			_chk(_kaf_set_flags(SAR_FLAG));
			vcpuins.udf |= VCPU_EFLAGS_AF;
			_be;
		}
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		vcpuins.opr1 = GetMax16(cdest);
		vcpuins.opr2 = count;
		vcpuins.result = vcpuins.opr1;
		while (count) {
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (!!GetLSB16(vcpuins.result)));
			vcpuins.result = GetMax16((t_nsbit16)vcpuins.result >> 1);
			count--;
		}
		if(vcpuins.opr2 == 1) _ClrEFLAGS_OF;
		else vcpuins.udf |= VCPU_EFLAGS_OF;
		if(vcpuins.opr2 != 0) {
			_bb("opr2(!0)");
			_chk(_kaf_set_flags(SAR_FLAG));
			vcpuins.udf |= VCPU_EFLAGS_AF;
			_be;
		}
		_be;break;
	case 32: _bb("bit(32)");
		vcpuins.bit = 32;
		vcpuins.opr1 = GetMax32(cdest);
		vcpuins.opr2 = count;
		vcpuins.result = vcpuins.opr1;
		while (count) {
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (!!GetLSB32(vcpuins.result)));
			vcpuins.result = GetMax32((t_nsbit32)vcpuins.result >> 1);
			count--;
		}
		if(vcpuins.opr2 == 1) _ClrEFLAGS_OF;
		else vcpuins.udf |= VCPU_EFLAGS_OF;
		if(vcpuins.opr2 != 0) {
			_bb("opr2(!0)");
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

static void _p_ins(t_nubit8 byte)
{
	t_nubit32 cedi, data = 0;
	_cb("_p_ins");
	switch (_GetAddressSize) {
	case 2: cedi = vcpu.di;break;
	case 4: cedi = vcpu.edi;break;
	default:_impossible_;break;}
	switch (byte) {
	case 1: _bb("byte(1)");
		_chk(_m_test_access(&vcpu.es, cedi, 1, 1));
		_chk(_p_input(vcpu.dx, GetRef(data), 1));
		_chk(_s_write_es(cedi, GetRef(data), 1));
		_chk(_kas_move_index(1, 0, 1));
		_be;break;
	case 2: _bb("byte(2)");
		_chk(_m_test_access(&vcpu.es, cedi, 2, 1));
		_chk(_p_input(vcpu.dx, GetRef(data), 2));
		_chk(_s_write_es(cedi, GetRef(data), 2));
		_chk(_kas_move_index(2, 0, 1));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_m_test_access(&vcpu.es, cedi, 4, 1));
		_chk(_p_input(vcpu.dx, GetRef(data), 4));
		_chk(_s_write_es(cedi, GetRef(data), 4));
		_chk(_kas_move_index(4, 0, 1));
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_ce;
}
static void _p_outs(t_nubit8 byte)
{
	t_nubit32 cesi, data = 0;
	_cb("_p_outs");
	switch (_GetAddressSize) {
	case 2: cesi = vcpu.si;break;
	case 4: cesi = vcpu.esi;break;
	default:_impossible_;break;}
	switch (byte) {
	case 1: _bb("byte(1)");
		_chk(_m_read_logical(vcpuins.roverds, cesi, GetRef(data), 1));
		_chk(_p_output(vcpu.dx, GetRef(data), 1));
		_chk(_kas_move_index(1, 1, 0));
		_be;break;
	case 2: _bb("byte(2)");
		_chk(_m_read_logical(vcpuins.roverds, cesi, GetRef(data), 2));
		_chk(_p_output(vcpu.dx, GetRef(data), 2));
		_chk(_kas_move_index(2, 1, 0));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_m_read_logical(vcpuins.roverds, cesi, GetRef(data), 4));
		_chk(_p_output(vcpu.dx, GetRef(data), 4));
		_chk(_kas_move_index(4, 1, 0));
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_ce;
}
static void _m_movs(t_nubit8 byte)
{
	t_nubit32 data = 0;
	t_nubit32 cesi, cedi;
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
		_chk(_m_read_logical(vcpuins.roverds, cesi, GetRef(data), 1));
		_chk(_s_write_es(cedi, GetRef(data), 1));
		_chk(_kas_move_index(1, 1, 1));
		_be;break;
	case 2: _bb("byte(2)");
		_chk(_m_read_logical(vcpuins.roverds, cesi, GetRef(data), 2));
		_chk(_s_write_es(cedi, GetRef(data), 2));
		_chk(_kas_move_index(2, 1, 1));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_m_read_logical(vcpuins.roverds, cesi, GetRef(data), 4));
		_chk(_s_write_es(cedi, GetRef(data), 4));
		_chk(_kas_move_index(4, 1, 1));
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_ce;
}
static void _m_stos(t_nubit8 byte)
{
	t_nubit32 cedi;
	_cb("_m_stos");
	switch (_GetAddressSize) {
	case 2: cedi = vcpu.di;break;
	case 4: cedi = vcpu.edi;break;
	default:_impossible_;break;}
	switch (byte) {
	case 1: _bb("byte(1)");
		_chk(_s_write_es(cedi, GetRef(vcpu.al), 1));
		_chk(_kas_move_index(1, 0, 1));
		_be;break;
	case 2: _bb("byte(2)");
		_chk(_s_write_es(cedi, GetRef(vcpu.ax), 2));
		_chk(_kas_move_index(2, 0, 1));
		_be;break;
	case 4: _bb("byte(4)");
		_chk(_s_write_es(cedi, GetRef(vcpu.eax), 4));
		_chk(_kas_move_index(4, 0, 1));
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_ce;
}
static void _m_lods(t_nubit8 byte)
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
		_chk(_m_read_logical(vcpuins.roverds, cesi, GetRef(vcpu.al), 1));
		_chk(_kas_move_index(1, 1, 0));
		vcpuins.result = vcpu.al;
		_be;break;
	case 2: _bb("byte(2)");
		vcpuins.bit = 16;
		_chk(_m_read_logical(vcpuins.roverds, cesi, GetRef(vcpu.ax), 2));
		_chk(_kas_move_index(2, 1, 0));
		vcpuins.result = vcpu.ax;
		_be;break;
	case 4: _bb("byte(4)");
		vcpuins.bit = 32;
		_chk(_m_read_logical(vcpuins.roverds, cesi, GetRef(vcpu.eax), 4));
		_chk(_kas_move_index(4, 1, 0));
		vcpuins.result = vcpu.eax;
		_be;break;
	default: _bb("byte");
		_chk(_SetExcept_CE(byte));
		_be;break;
	}
	_ce;
}
static void _a_cmps(t_nubit8 bit)
{
	t_nubit32 cesi, cedi;
	_cb("_a_cmps");
	vcpuins.opr1 = 0;
	vcpuins.opr2 = 0;
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
		_chk(_m_read_logical(vcpuins.roverds, cesi, GetRef(vcpuins.opr1), 1));
		_chk(_s_read_es(cedi, GetRef(vcpuins.opr2), 1));
		vcpuins.result = GetMax8(vcpuins.opr1 - vcpuins.opr2);
		_chk(_kas_move_index(1, 1, 1));
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		_chk(_m_read_logical(vcpuins.roverds, cesi, GetRef(vcpuins.opr1), 2));
		_chk(_s_read_es(cedi, GetRef(vcpuins.opr2), 2));
		vcpuins.result = GetMax16(vcpuins.opr1 - vcpuins.opr2);
		_chk(_kas_move_index(2, 1, 1));
		_be;break;
	case 32: _bb("bit(32)");
		vcpuins.bit = 32;
		vcpuins.type = CMP32;
		_chk(_m_read_logical(vcpuins.roverds, cesi, GetRef(vcpuins.opr1), 4));
		_chk(_s_read_es(cedi, GetRef(vcpuins.opr2), 4));
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
static void _a_scas(t_nubit8 bit)
{
	t_nubit32 cedi;
	_cb("_a_scas");
	vcpuins.opr1 = 0;
	vcpuins.opr2 = 0;
	switch (_GetAddressSize) {
	case 2: cedi = vcpu.di;break;
	case 4: cedi = vcpu.edi;break;
	default:_impossible_;break;}
	switch (bit) {
	case 8: _bb("bit(8)");
		vcpuins.bit = 8;
		vcpuins.type = CMP8;
		_chk(vcpuins.opr1 = vcpu.al);
		_chk(_s_read_es(cedi, GetRef(vcpuins.opr2), 1));
		vcpuins.result = GetMax8(vcpuins.opr1 - vcpuins.opr2);
		_chk(_kas_move_index(1, 0, 1));
		_be;break;
	case 16: _bb("bit(16)");
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		_chk(vcpuins.opr1 = vcpu.ax);
		_chk(_s_read_es(cedi, GetRef(vcpuins.opr2), 2));
		vcpuins.result = GetMax16(vcpuins.opr1 - vcpuins.opr2);
		_chk(_kas_move_index(2, 0, 1));
		_be;break;
	case 32: _bb("bit(32)");
		vcpuins.bit = 32;
		vcpuins.type = CMP32;
		_chk(vcpuins.opr1 = vcpu.eax);
		_chk(_s_read_es(cedi, GetRef(vcpuins.opr2), 4));
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
static void UndefinedOpcode()
{
	_cb("UndefinedOpcode");
	vcpu = vcpuins.oldcpu;
	if (!_GetCR0_PE) {
		vapiPrint("The NXVM CPU has encountered an illegal instruction at L%08X.\n", vcpu.cs.base + vcpu.eip);
		vapiCallBackMachineStop();
	}
	_chk(_SetExcept_UD(0));
	_ce;
}
static void ADD_RM8_R8()
{
	_cb("ADD_RM8_R8");
	i386(0x00) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_a_add(vcpuins.crm, vcpuins.cr, 8));
	vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
	_ce;
}
static void ADD_RM32_R32()
{
	_cb("ADD_RM32_R32");
	i386(0x01) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_a_add(vcpuins.crm, vcpuins.cr, _GetOperandSize * 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(2, 2));
		_chk(_m_read_rm(2));
		_chk(_a_add(vcpuins.crm, vcpuins.cr, 16));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
	}
	_ce;
}
static void ADD_R8_RM8()
{
	_cb("ADD_R8_RM8");
	i386(0x02) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_a_add(vcpuins.cr, vcpuins.crm, 8));
	_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), 1));
	_ce;
}
static void ADD_R32_RM32()
{
	_cb("ADD_R32_RM32");
	i386(0x03) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_a_add(vcpuins.cr, vcpuins.crm, _GetOperandSize * 8));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), _GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(2, 2));
		_chk(_m_read_rm(2));
		_chk(_a_add(vcpuins.cr, vcpuins.crm, 16));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), 2));
	}
	_ce;
}
static void ADD_AL_I8()
{
	_cb("ADD_AL_I8");
	i386(0x04) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_a_add(vcpu.al, vcpuins.cimm, 8));
	vcpu.al = GetMax8(vcpuins.result);
	_ce;
}
static void ADD_EAX_I32()
{
	_cb("ADD_EAX_I32");
	i386(0x05) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_a_add(vcpu.ax, vcpuins.cimm, 16));
			vcpu.ax = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_imm(4));
			_chk(_a_add(vcpu.eax, vcpuins.cimm, 32));
			vcpu.eax = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		_chk(_a_add(vcpu.ax, vcpuins.cimm, 16));
		vcpu.ax = GetMax16(vcpuins.result);
	}
	_ce;
}
static void PUSH_ES()
{
	_cb("PUSH_ES");
	i386(0x06) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_e_push(vcpu.es.selector, _GetOperandSize));
	_ce;
}
static void POP_ES()
{
	/* note: not sure if operand size is 32,
		push/pop selector only or with higher 16 bit */
	t_nubit16 sel;
	_cb("POP_ES");
	i386(0x07) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(sel = GetMax16(_e_pop(_GetOperandSize)));
	_chk(_s_load_es(sel));
	_ce;
}
static void OR_RM8_R8()
{
	_cb("OR_RM8_R8");
	i386(0x08) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_a_or(vcpuins.crm, vcpuins.cr, 8));
	vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
	_ce;
}
static void OR_RM32_R32()
{
	_cb("OR_RM32_R32");
	i386(0x09) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_a_or(vcpuins.crm, vcpuins.cr, _GetOperandSize * 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(2, 2));
		_chk(_m_read_rm(2));
		_chk(_a_or(vcpuins.crm, vcpuins.cr, 16));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
	}
	_ce;
}
static void OR_R8_RM8()
{
	_cb("OR_R8_RM8");
	i386(0x0a) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_a_or(vcpuins.cr, vcpuins.crm, 8));
	_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), 1));
	_ce;
}
static void OR_R32_RM32()
{
	_cb("OR_R32_RM32");
	i386(0x0b) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_a_or(vcpuins.cr, vcpuins.crm, _GetOperandSize * 8));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), _GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(2, 2));
		_chk(_m_read_rm(2));
		_chk(_a_or(vcpuins.cr, vcpuins.crm, 16));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), 2));
	}
	_ce;
}
static void OR_AL_I8()
{
	_cb("OR_AL_I8");
	i386(0x0c) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_a_or(vcpu.al, vcpuins.cimm, 8));
	vcpu.al = GetMax8(vcpuins.result);
	_ce;
}
static void OR_EAX_I32()
{
	_cb("OR_EAX_I32");
	i386(0x0d) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_a_or(vcpu.ax, vcpuins.cimm, 16));
			vcpu.ax = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_imm(4));
			_chk(_a_or(vcpu.eax, vcpuins.cimm, 32));
			vcpu.eax = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		_chk(_a_or(vcpu.ax, vcpuins.cimm, 16));
		vcpu.ax = GetMax16(vcpuins.result);
	}
	_ce;
}
static void PUSH_CS()
{
	_cb("PUSH_CS");
	i386(0x0e) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_e_push(vcpu.cs.selector, _GetOperandSize));
	_ce;
}
static void POP_CS()
{
	t_nubit16 sel;
	_cb("POP_CS");
	i386(0x0f) {
		_bb("i386");
		_chk(_SetExcept_CE(0x0f));
		_be;
	} else {
		vcpu.ip++;
		_chk(sel = GetMax16(_e_pop(_GetOperandSize)));
		_chk(_s_load_cs(sel));
	}
	_ce;
}
static void INS_0F()
{
	t_nubit8 opcode = 0x00;
	_cb("INS_0F");
	i386(0x0f) {
		_adv;
		_chk(_s_read_cs(vcpu.eip, GetRef(opcode), 1));
		_chk(ExecFun(table_0f[opcode]));
	} else
		POP_CS();
	_ce;
}
static void ADC_RM8_R8()
{
	_cb("ADC_RM8_R8");
	i386(0x10) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_a_adc(vcpuins.crm, vcpuins.cr, 8));
	vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
	_ce;
}
static void ADC_RM32_R32()
{
	_cb("ADC_RM32_R32");
	i386(0x11) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_a_adc(vcpuins.crm, vcpuins.cr, _GetOperandSize * 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(2, 2));
		_chk(_m_read_rm(2));
		_chk(_a_adc(vcpuins.crm, vcpuins.cr, 16));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
	}
	_ce;
}
static void ADC_R8_RM8()
{
	_cb("ADC_R8_RM8");
	i386(0x12) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_a_adc(vcpuins.cr, vcpuins.crm, 8));
	_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), 1));
	_ce;
}
static void ADC_R32_RM32()
{
	_cb("ADC_R32_RM32");
	i386(0x13) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_a_adc(vcpuins.cr, vcpuins.crm, _GetOperandSize * 8));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), _GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(2, 2));
		_chk(_m_read_rm(2));
		_chk(_a_adc(vcpuins.cr, vcpuins.crm, 16));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), 2));
	}
	_ce;
}
static void ADC_AL_I8()
{
	_cb("ADC_AL_I8");
	i386(0x14) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_a_adc(vcpu.al, vcpuins.cimm, 8));
	vcpu.al = GetMax8(vcpuins.result);
	_ce;
}
static void ADC_EAX_I32()
{
	_cb("ADC_EAX_I32");
	i386(0x15) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_a_adc(vcpu.ax, vcpuins.cimm, 16));
			vcpu.ax = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_imm(4));
			_chk(_a_adc(vcpu.eax, vcpuins.cimm, 32));
			vcpu.eax = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		_chk(_a_adc(vcpu.ax, vcpuins.cimm, 16));
		vcpu.ax = GetMax16(vcpuins.result);
	}
	_ce;
}
static void PUSH_SS()
{
	_cb("PUSH_SS");
	i386(0x16) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_e_push(vcpu.ss.selector, _GetOperandSize));
	_ce;
}
static void POP_SS()
{
	t_nubit16 sel;
	_cb("POP_SS");
	i386(0x17) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(sel = GetMax16(_e_pop(_GetOperandSize)));
	_chk(_s_load_ss(sel));
	_ce;
}
static void SBB_RM8_R8()
{
	_cb("SBB_RM8_R8");
	i386(0x18) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_a_sbb(vcpuins.crm, vcpuins.cr, 8));
	vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
	_ce;
}
static void SBB_RM32_R32()
{
	_cb("SBB_RM32_R32");
	i386(0x19) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_a_sbb(vcpuins.crm, vcpuins.cr, _GetOperandSize * 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(2, 2));
		_chk(_m_read_rm(2));
		_chk(_a_sbb(vcpuins.crm, vcpuins.cr, 16));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
	}
	_ce;
}
static void SBB_R8_RM8()
{
	_cb("SBB_R8_RM8");
	i386(0x1a) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_a_sbb(vcpuins.cr, vcpuins.crm, 8));
	_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), 1));
	_ce;
}
static void SBB_R32_RM32()
{
	_cb("SBB_R32_RM32");
	i386(0x1b) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_a_sbb(vcpuins.cr, vcpuins.crm, _GetOperandSize * 8));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), _GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(2, 2));
		_chk(_m_read_rm(2));
		_chk(_a_sbb(vcpuins.cr, vcpuins.crm, 16));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), 2));
	}
	_ce;
}
static void SBB_AL_I8()
{
	_cb("SBB_AL_I8");
	i386(0x1c) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_a_sbb(vcpu.al, vcpuins.cimm, 8));
	vcpu.al = GetMax8(vcpuins.result);
	_ce;
}
static void SBB_EAX_I32()
{
	_cb("SBB_EAX_I32");
	i386(0x1d) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_a_sbb(vcpu.ax, vcpuins.cimm, 16));
			vcpu.ax = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_imm(4));
			_chk(_a_sbb(vcpu.eax, vcpuins.cimm, 32));
			vcpu.eax = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		_chk(_a_sbb(vcpu.ax, vcpuins.cimm, 16));
		vcpu.ax = GetMax16(vcpuins.result);
	}
	_ce;
}
static void PUSH_DS()
{
	_cb("PUSH_DS");
	i386(0x1e) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_e_push(vcpu.ds.selector, _GetOperandSize));
	_ce;
}
static void POP_DS()
{
	t_nubit16 sel;
	_cb("POP_DS");
	i386(0x1f) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(sel = GetMax16(_e_pop(_GetOperandSize)));
	_chk(_s_load_ds(sel));
	_ce;
}
static void AND_RM8_R8()
{
	_cb("AND_RM8_R8");
	i386(0x20) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_a_and(vcpuins.crm, vcpuins.cr, 8));
	vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
	_ce;
}
static void AND_RM32_R32()
{
	_cb("AND_RM32_R32");
	i386(0x21) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_a_and(vcpuins.crm, vcpuins.cr, _GetOperandSize * 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(2, 2));
		_chk(_m_read_rm(2));
		_chk(_a_and(vcpuins.crm, vcpuins.cr, 16));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
	}
	_ce;
}
static void AND_R8_RM8()
{
	_cb("AND_R8_RM8");
	i386(0x22) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_a_and(vcpuins.cr, vcpuins.crm, 8));
	_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), 1));
	_ce;
}
static void AND_R32_RM32()
{
	_cb("AND_R32_RM32");
	i386(0x23) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_a_and(vcpuins.cr, vcpuins.crm, _GetOperandSize * 8));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), _GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(2, 2));
		_chk(_m_read_rm(2));
		_chk(_a_and(vcpuins.cr, vcpuins.crm, 16));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), 2));
	}
	_ce;
}
static void AND_AL_I8()
{
	_cb("AND_AL_I8");
	i386(0x24) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_a_and(vcpu.al, vcpuins.cimm, 8));
	vcpu.al = GetMax8(vcpuins.result);
	_ce;
}
static void AND_EAX_I32()
{
	_cb("AND_EAX_I32");
	i386(0x25) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_a_and(vcpu.ax, vcpuins.cimm, 16));
			vcpu.ax = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_imm(4));
			_chk(_a_and(vcpu.eax, vcpuins.cimm, 32));
			vcpu.eax = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		_chk(_a_and(vcpu.ax, vcpuins.cimm, 16));
		vcpu.ax = GetMax16(vcpuins.result);
	}
	_ce;
}
static void PREFIX_ES()
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
static void DAA()
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
static void SUB_RM8_R8()
{
	_cb("SUB_RM8_R8");
	i386(0x28) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_a_sub(vcpuins.crm, vcpuins.cr, 8));
	vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
	_ce;
}
static void SUB_RM32_R32()
{
	_cb("SUB_RM32_R32");
	i386(0x29) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_a_sub(vcpuins.crm, vcpuins.cr, _GetOperandSize * 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(2, 2));
		_chk(_m_read_rm(2));
		_chk(_a_sub(vcpuins.crm, vcpuins.cr, 16));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
	}
	_ce;
}
static void SUB_R8_RM8()
{
	_cb("SUB_R8_RM8");
	i386(0x2a) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_a_sub(vcpuins.cr, vcpuins.crm, 8));
	_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), 1));
	_ce;
}
static void SUB_R32_RM32()
{
	_cb("SUB_R32_RM32");
	i386(0x2b) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_a_sub(vcpuins.cr, vcpuins.crm, _GetOperandSize * 8));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), _GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(2, 2));
		_chk(_m_read_rm(2));
		_chk(_a_sub(vcpuins.cr, vcpuins.crm, 16));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), 2));
	}
	_ce;
}
static void SUB_AL_I8()
{
	_cb("SUB_AL_I8");
	i386(0x2c) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_a_sub(vcpu.al, vcpuins.cimm, 8));
	vcpu.al = GetMax8(vcpuins.result);
	_ce;
}
static void SUB_EAX_I32()
{
	_cb("SUB_EAX_I32");
	i386(0x2d) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_a_sub(vcpu.ax, vcpuins.cimm, 16));
			vcpu.ax = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_imm(4));
			_chk(_a_sub(vcpu.eax, vcpuins.cimm, 32));
			vcpu.eax = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		_chk(_a_sub(vcpu.ax, vcpuins.cimm, 16));
		vcpu.ax = GetMax16(vcpuins.result);
	}
	_ce;
}
static void PREFIX_CS()
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
static void DAS()
{
	t_nubit8 oldAL = vcpu.al;
	_cb("DAS");
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
static void XOR_RM8_R8()
{
	_cb("XOR_RM8_R8");
	i386(0x30) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_a_xor(vcpuins.crm, vcpuins.cr, 8));
	vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
	_ce;
}
static void XOR_RM32_R32()
{
	_cb("XOR_RM32_R32");
	i386(0x31) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_a_xor(vcpuins.crm, vcpuins.cr, _GetOperandSize * 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(2, 2));
		_chk(_m_read_rm(2));
		_chk(_a_xor(vcpuins.crm, vcpuins.cr, 16));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
	}
	_ce;
}
static void XOR_R8_RM8()
{
	_cb("XOR_R8_RM8");
	i386(0x32) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_a_xor(vcpuins.cr, vcpuins.crm, 8));
	_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), 1));
	_ce;
}
static void XOR_R32_RM32()
{
	_cb("XOR_R32_RM32");
	i386(0x33) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_a_xor(vcpuins.cr, vcpuins.crm, _GetOperandSize * 8));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), _GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(2, 2));
		_chk(_m_read_rm(2));
		_chk(_a_xor(vcpuins.cr, vcpuins.crm, 16));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), 2));
	}
	_ce;
}
static void XOR_AL_I8()
{
	_cb("XOR_AL_I8");
	i386(0x34) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_a_xor(vcpu.al, vcpuins.cimm, 8));
	vcpu.al = GetMax8(vcpuins.result);
	_ce;
}
static void XOR_EAX_I32()
{
	_cb("XOR_EAX_I32");
	i386(0x35) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperxorSize(2)");
			_chk(_d_imm(2));
			_chk(_a_xor(vcpu.ax, vcpuins.cimm, 16));
			vcpu.ax = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperxorSize(4)");
			_chk(_d_imm(4));
			_chk(_a_xor(vcpu.eax, vcpuins.cimm, 32));
			vcpu.eax = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		_chk(_a_xor(vcpu.ax, vcpuins.cimm, 16));
		vcpu.ax = GetMax16(vcpuins.result);
	}
	_ce;
}
static void PREFIX_SS()
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
static void AAA()
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
	vcpuins.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF);
	_ce;
}
static void CMP_RM8_R8()
{
	_cb("CMP_RM8_R8");
	i386(0x38) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_a_cmp(vcpuins.crm, vcpuins.cr, 8));
	_ce;
}
static void CMP_RM32_R32()
{
	_cb("CMP_RM32_R32");
	i386(0x39) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_a_cmp(vcpuins.crm, vcpuins.cr, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(2, 2));
		_chk(_m_read_rm(2));
		_chk(_a_cmp(vcpuins.crm, vcpuins.cr, 16));
	}
	_ce;
}
static void CMP_R8_RM8()
{
	_cb("CMP_R8_RM8");
	i386(0x3a) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_a_cmp(vcpuins.cr, vcpuins.crm, 8));
	_ce;
}
static void CMP_R32_RM32()
{
	_cb("CMP_R32_RM32");
	i386(0x3b) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_a_cmp(vcpuins.cr, vcpuins.crm, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(2, 2));
		_chk(_m_read_rm(2));
		_chk(_a_cmp(vcpuins.cr, vcpuins.crm, 16));
	}
	_ce;
}
static void CMP_AL_I8()
{
	_cb("CMP_AL_I8");
	i386(0x3c) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_a_cmp(vcpu.al, vcpuins.cimm, 8));
	_ce;
}
static void CMP_EAX_I32()
{
	_cb("CMP_EAX_I32");
	i386(0x3d) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_a_cmp(vcpu.ax, vcpuins.cimm, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_imm(4));
			_chk(_a_cmp(vcpu.eax, vcpuins.cimm, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		_chk(_a_cmp(vcpu.ax, vcpuins.cimm, 16));
	}
	_ce;
}
static void PREFIX_DS()
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
static void AAS()
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
	vcpuins.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF);
	_ce;
}
static void INC_EAX()
{
	_cb("INC_EAX");
	i386(0x40) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_inc(vcpu.ax, 16));
			vcpu.ax = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_a_inc(vcpu.eax, 32));
			vcpu.eax = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_a_inc(vcpu.ax, 16));
		vcpu.ax = GetMax16(vcpuins.result);
	}
	_ce;
}
static void INC_ECX()
{
	_cb("INC_ECX");
	i386(0x41) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_inc(vcpu.cx, 16));
			vcpu.cx = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_a_inc(vcpu.ecx, 32));
			vcpu.ecx = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_a_inc(vcpu.cx, 16));
		vcpu.cx = GetMax16(vcpuins.result);
	}
	_ce;
}
static void INC_EDX()
{
	_cb("INC_EDX");
	i386(0x42) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_inc(vcpu.dx, 16));
			vcpu.dx = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_a_inc(vcpu.edx, 32));
			vcpu.edx = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_a_inc(vcpu.dx, 16));
		vcpu.dx = GetMax16(vcpuins.result);
	}
	_ce;
}
static void INC_EBX()
{
	_cb("INC_EBX");
	i386(0x43) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_inc(vcpu.bx, 16));
			vcpu.bx = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_a_inc(vcpu.ebx, 32));
			vcpu.ebx = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_a_inc(vcpu.bx, 16));
		vcpu.bx = GetMax16(vcpuins.result);
	}
	_ce;
}
static void INC_ESP()
{
	_cb("INC_ESP");
	i386(0x44) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_inc(vcpu.sp, 16));
			vcpu.sp = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_a_inc(vcpu.esp, 32));
			vcpu.esp = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_a_inc(vcpu.sp, 16));
		vcpu.sp = GetMax16(vcpuins.result);
	}
	_ce;
}
static void INC_EBP()
{
	_cb("INC_EBP");
	i386(0x45) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_inc(vcpu.bp, 16));
			vcpu.bp = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_a_inc(vcpu.ebp, 32));
			vcpu.ebp = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_a_inc(vcpu.bp, 16));
		vcpu.bp = GetMax16(vcpuins.result);
	}
	_ce;
}
static void INC_ESI()
{
	_cb("INC_ESI");
	i386(0x46) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_inc(vcpu.si, 16));
			vcpu.si = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_a_inc(vcpu.esi, 32));
			vcpu.esi = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_a_inc(vcpu.si, 16));
		vcpu.si = GetMax16(vcpuins.result);
	}
	_ce;
}
static void INC_EDI()
{
	_cb("INC_EDI");
	i386(0x47) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_inc(vcpu.di, 16));
			vcpu.di = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_a_inc(vcpu.edi, 32));
			vcpu.edi = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_a_inc(vcpu.di, 16));
		vcpu.di = GetMax16(vcpuins.result);
	}
	_ce;
}
static void DEC_EAX()
{
	_cb("DEC_EAX");
	i386(0x48) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_dec(vcpu.ax, 16));
			vcpu.ax = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_a_dec(vcpu.eax, 32));
			vcpu.eax = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_a_dec(vcpu.ax, 16));
		vcpu.ax = GetMax16(vcpuins.result);
	}
	_ce;
}
static void DEC_ECX()
{
	_cb("DEC_ECX");
	i386(0x49) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_dec(vcpu.cx, 16));
			vcpu.cx = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_a_dec(vcpu.ecx, 32));
			vcpu.ecx = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_a_dec(vcpu.cx, 16));
		vcpu.cx = GetMax16(vcpuins.result);
	}
	_ce;
}
static void DEC_EDX()
{
	_cb("DEC_EDX");
	i386(0x4a) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_dec(vcpu.dx, 16));
			vcpu.dx = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_a_dec(vcpu.edx, 32));
			vcpu.edx = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_a_dec(vcpu.dx, 16));
		vcpu.dx = GetMax16(vcpuins.result);
	}
	_ce;
}
static void DEC_EBX()
{
	_cb("DEC_EBX");
	i386(0x4b) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_dec(vcpu.bx, 16));
			vcpu.bx = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_a_dec(vcpu.ebx, 32));
			vcpu.ebx = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_a_dec(vcpu.bx, 16));
		vcpu.bx = GetMax16(vcpuins.result);
	}
	_ce;
}
static void DEC_ESP()
{
	_cb("DEC_ESP");
	i386(0x4c) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_dec(vcpu.sp, 16));
			vcpu.sp = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_a_dec(vcpu.esp, 32));
			vcpu.esp = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_a_dec(vcpu.sp, 16));
		vcpu.sp = GetMax16(vcpuins.result);
	}
	_ce;
}
static void DEC_EBP()
{
	_cb("DEC_EBP");
	i386(0x4d) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_dec(vcpu.bp, 16));
			vcpu.bp = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_a_dec(vcpu.ebp, 32));
			vcpu.ebp = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_a_dec(vcpu.bp, 16));
		vcpu.bp = GetMax16(vcpuins.result);
	}
	_ce;
}
static void DEC_ESI()
{
	_cb("DEC_ESI");
	i386(0x4e) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_dec(vcpu.si, 16));
			vcpu.si = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_a_dec(vcpu.esi, 32));
			vcpu.esi = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_a_dec(vcpu.si, 16));
		vcpu.si = GetMax16(vcpuins.result);
	}
	_ce;
}
static void DEC_EDI()
{
	_cb("DEC_EDI");
	i386(0x4f) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_a_dec(vcpu.di, 16));
			vcpu.di = GetMax16(vcpuins.result);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_a_dec(vcpu.edi, 32));
			vcpu.edi = GetMax32(vcpuins.result);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_a_dec(vcpu.di, 16));
		vcpu.di = GetMax16(vcpuins.result);
	}
	_ce;
}
static void PUSH_EAX()
{
	_cb("PUSH_EAX");
	i386(0x50) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_push(vcpu.ax, 2));break;
		case 4: _chk(_e_push(vcpu.eax, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_push(vcpu.ax, 2);
	}
	_ce;
}
static void PUSH_ECX()
{
	_cb("PUSH_ECX");
	i386(0x51) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_push(vcpu.cx, 2));break;
		case 4: _chk(_e_push(vcpu.ecx, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_push(vcpu.cx, 2);
	}
	_ce;
}
static void PUSH_EDX()
{
	_cb("PUSH_EDX");
	i386(0x52) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_push(vcpu.dx, 2));break;
		case 4: _chk(_e_push(vcpu.edx, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_push(vcpu.dx, 2);
	}
	_ce;
}
static void PUSH_EBX()
{
	_cb("PUSH_EBX");
	i386(0x53) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_push(vcpu.bx, 2));break;
		case 4: _chk(_e_push(vcpu.ebx, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_push(vcpu.bx, 2);
	}
	_ce;
}
static void PUSH_ESP()
{
	_cb("PUSH_ESP");
	i386(0x54) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_push(vcpu.sp, 2));break;
		case 4: _chk(_e_push(vcpu.esp, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_push(vcpu.sp, 2);
	}
	_ce;
}
static void PUSH_EBP()
{
	_cb("PUSH_EBP");
	i386(0x55) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_push(vcpu.bp, 2));break;
		case 4: _chk(_e_push(vcpu.ebp, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_push(vcpu.bp, 2);
	}
	_ce;
}
static void PUSH_ESI()
{
	_cb("PUSH_ESI");
	i386(0x56) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_push(vcpu.si, 2));break;
		case 4: _chk(_e_push(vcpu.esi, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_push(vcpu.si, 2);
	}
	_ce;
}
static void PUSH_EDI()
{
	_cb("PUSH_EDI");
	i386(0x57) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _chk(_e_push(vcpu.di, 2));break;
		case 4: _chk(_e_push(vcpu.edi, 4));break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_e_push(vcpu.di, 2);
	}
	_ce;
}
static void POP_EAX()
{
	_cb("POP_EAX");
	i386(0x58) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(vcpu.ax = GetMax16(_e_pop(2)));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(vcpu.eax = GetMax32(_e_pop(4)));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(vcpu.ax = GetMax16(_e_pop(2)));
	}
	_ce;
}
static void POP_ECX()
{
	_cb("POP_ECX");
	i386(0x59) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(vcpu.cx = GetMax16(_e_pop(2)));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(vcpu.ecx = GetMax32(_e_pop(4)));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(vcpu.cx = GetMax16(_e_pop(2)));
	}
	_ce;
}
static void POP_EDX()
{
	_cb("POP_EDX");
	i386(0x5a) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(vcpu.dx = GetMax16(_e_pop(2)));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(vcpu.edx = GetMax32(_e_pop(4)));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(vcpu.dx = GetMax16(_e_pop(2)));
	}
	_ce;
}
static void POP_EBX()
{
	_cb("POP_EBX");
	i386(0x5b) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(vcpu.bx = GetMax16(_e_pop(2)));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(vcpu.ebx = GetMax32(_e_pop(4)));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(vcpu.bx = GetMax16(_e_pop(2)));
	}
	_ce;
}
static void POP_ESP()
{
	_cb("POP_ESP");
	i386(0x5c) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(vcpu.sp = GetMax16(_e_pop(2)));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(vcpu.esp = GetMax32(_e_pop(4)));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(vcpu.sp = GetMax16(_e_pop(2)));
	}
	_ce;
}
static void POP_EBP()
{
	_cb("POP_EBP");
	i386(0x5d) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(vcpu.bp = GetMax16(_e_pop(2)));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(vcpu.ebp = GetMax32(_e_pop(4)));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(vcpu.bp = GetMax16(_e_pop(2)));
	}
	_ce;
}
static void POP_ESI()
{
	_cb("POP_ESI");
	i386(0x5e) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(vcpu.si = GetMax16(_e_pop(2)));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(vcpu.esi = GetMax32(_e_pop(4)));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(vcpu.si = GetMax16(_e_pop(2)));
	}
	_ce;
}
static void POP_EDI()
{
	_cb("POP_EDI");
	i386(0x5f) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(vcpu.di = GetMax16(_e_pop(2)));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(vcpu.edi = GetMax32(_e_pop(4)));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(vcpu.di = GetMax16(_e_pop(2)));
	}
	_ce;
}
static void PUSHA()
{
	t_nubit32 cesp;
	_cb("PUSHA");
	i386(0x60) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			cesp = vcpu.sp;
			_chk(_e_push(vcpu.ax, 2));
			_chk(_e_push(vcpu.cx, 2));
			_chk(_e_push(vcpu.dx, 2));
			_chk(_e_push(vcpu.bx, 2));
			_chk(_e_push(cesp,    2));
			_chk(_e_push(vcpu.bp, 2));
			_chk(_e_push(vcpu.si, 2));
			_chk(_e_push(vcpu.di, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			cesp = vcpu.esp;
			_chk(_e_push(vcpu.eax, 4));
			_chk(_e_push(vcpu.ecx, 4));
			_chk(_e_push(vcpu.edx, 4));
			_chk(_e_push(vcpu.ebx, 4));
			_chk(_e_push(cesp,     4));
			_chk(_e_push(vcpu.ebp, 4));
			_chk(_e_push(vcpu.esi, 4));
			_chk(_e_push(vcpu.edi, 4));
			_be;break;
		default:_impossible_;break;}
	} else
		UndefinedOpcode();
	_ce;
}
static void POPA()
{
	t_nubit32 cesp;
	_cb("POPA");
	i386(0x61) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(vcpu.di = GetMax16(_e_pop(2)));
			_chk(vcpu.si = GetMax16(_e_pop(2)));
			_chk(vcpu.bp = GetMax16(_e_pop(2)));
			_chk(cesp    = GetMax16(_e_pop(2)));
			_chk(vcpu.bx = GetMax16(_e_pop(2)));
			_chk(vcpu.dx = GetMax16(_e_pop(2)));
			_chk(vcpu.cx = GetMax16(_e_pop(2)));
			_chk(vcpu.ax = GetMax16(_e_pop(2)));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(vcpu.edi = GetMax32(_e_pop(4)));
			_chk(vcpu.esi = GetMax32(_e_pop(4)));
			_chk(vcpu.ebp = GetMax32(_e_pop(4)));
			_chk(cesp     = GetMax32(_e_pop(4)));
			_chk(vcpu.ebx = GetMax32(_e_pop(4)));
			_chk(vcpu.edx = GetMax32(_e_pop(4)));
			_chk(vcpu.ecx = GetMax32(_e_pop(4)));
			_chk(vcpu.eax = GetMax32(_e_pop(4)));
			_be;break;
		default:_impossible_;break;}
	} else
		UndefinedOpcode();
	_ce;
}
static void BOUND_R16_M16_16()
{
	t_nsbit16 a16,l16,u16;
	t_nsbit32 a32,l32,u32;
	_cb("BOUND_R16_M16_16");
	i386(0x62) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize * 2));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			a16 = (t_nsbit16)vcpuins.cr;
			_chk(_m_read_rm(2));
			l16 = (t_nsbit16)vcpuins.crm;
			vcpuins.mrm.offset += 2;
			_chk(_m_read_rm(2));
			u16 = (t_nsbit16)vcpuins.crm;
			if (a16 < l16 || a16 > u16)
				_chk(_SetExcept_BR(0));
			_be;break;
		case 4: _bb("OperandSize(4)");
			a32 = (t_nsbit32)vcpuins.cr;
			_chk(_m_read_rm(4));
			l32 = (t_nsbit32)vcpuins.crm;
			vcpuins.mrm.offset += 2;
			_chk(_m_read_rm(4));
			u32 = (t_nsbit32)vcpuins.crm;
			if (a32 < l32 || a32 > u32)
				_chk(_SetExcept_BR(0));
			_be;break;
		default:_impossible_;break;}
	} else
		UndefinedOpcode();
	_ce;
}
static void ARPL_RM16_R16()
{
	_cb("ARPL_RM16_R16");
	i386(0x63) {
		if (_IsProtected) {
			_adv;
			_chk(_d_modrm(2, 2));
			_chk(_m_read_rm(2));
			vcpuins.cr = GetMax16(vcpuins.cr);
			if (_GetSelector_RPL(vcpuins.crm) < _GetSelector_RPL(vcpuins.cr)) {
				_SetEFLAGS_ZF;
				vcpuins.result = GetMax16((vcpuins.crm & ~VCPU_SELECTOR_RPL) |
					(vcpuins.cr & VCPU_SELECTOR_RPL));
				vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			} else
				_ClrEFLAGS_ZF;
		} else
			_chk(UndefinedOpcode());
	} else
		UndefinedOpcode();
	_ce;
}
static void PREFIX_FS()
{
	_cb("PREFIX_FS");
	i386(0x64) {
		_adv;
		vcpuins.roverds = &vcpu.fs;
		vcpuins.roverss = &vcpu.fs;
	} else
		UndefinedOpcode();
	_ce;
}
static void PREFIX_GS()
{
	_cb("PREFIX_GS");
	i386(0x65) {
		_adv;
		vcpuins.roverds = &vcpu.gs;
		vcpuins.roverss = &vcpu.gs;
	} else
		UndefinedOpcode();
	_ce;
}
static void PREFIX_OprSize()
{
	_cb("PREFIX_OprSize");
	i386(0x66) {
		_adv;
		vcpuins.prefix_oprsize = 0x01;
	} else
		UndefinedOpcode();
	_ce;
}
static void PREFIX_AddrSize()
{
	_cb("PREFIX_AddrSize");
	i386(0x67) {
		_adv;
		vcpuins.prefix_addrsize = 0x01;
	} else
		UndefinedOpcode();
	_ce;
}
static void PUSH_I32()
{
	_cb("PUSH_I32");
	i386(0x68) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		_chk(_e_push(vcpuins.cimm, _GetOperandSize));
	} else
		UndefinedOpcode();
	_ce;
}
static void IMUL_R32_RM32_I32()
{
	_cb("IMUL_R32_RM32_I32");
	i386(0x69) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_d_imm(_GetOperandSize));
		_chk(_a_imul3(vcpuins.crm, vcpuins.cimm, _GetOperandSize * 8));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), _GetOperandSize));
	} else
		UndefinedOpcode();
	_ce;
}
static void PUSH_I8()
{
	_cb("PUSH_I8");
	i386(0x6a) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_push(GetMax8(vcpuins.cimm), _GetOperandSize));
	} else
		UndefinedOpcode();
	_ce;
}
static void IMUL_R32_RM32_I8()
{
	_cb("IMUL_R32_RM32_I8");
	i386(0x6b) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_d_imm(1));
		_chk(_a_imul3(vcpuins.crm, vcpuins.cimm, ((_GetOperandSize * 8 + 8) >> 1)));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), _GetOperandSize));
	} else
		UndefinedOpcode();
	_ce;
}
static void INSB()
{
	_cb("INSB");
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
static void INSW()
{
	_cb("INSW");
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
static void OUTSB()
{
	_cb("OUTSB");
	i386(0x6e) {
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
static void OUTSW()
{
	_cb("OUTSW");
	i386(0x6f) {
		_adv;
		if(vcpuins.prefix_rep == PREFIX_REP_NONE) {
			_bb("prefix_rep(PREFIX_REP_NONE)");
			_chk(_p_outs(_GetOperandSize));
			_be;
		} else {
			_bb("prefix_rep(!PREFIX_REP_NONE)");
			switch (_GetAddressSize) {
			case 2: _bb("AddressSize(2)");
				if (vcpu.cx) {
					_bb("cx(!0)");
					_chk(_p_outs(_GetOperandSize));
					vcpu.cx--;
					_be;
				}
				if (vcpu.cx) vcpuins.flaginsloop = 1;
				_be;break;
			case 4: _bb("AddressSize(4)");
				if (vcpu.ecx) {
					_bb("ecx(!0)");
					_chk(_p_outs(_GetOperandSize));
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
static void JO_REL8()
{
	_cb("JO_REL8");
	i386(0x70) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), 1, _GetEFLAGS_OF));
	_ce;
}
static void JNO_REL8()
{
	_cb("JNO_REL8");
	i386(0x71) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), 1, !_GetEFLAGS_OF));
	_ce;
}
static void JC_REL8()
{
	_cb("JC_REL8");
	i386(0x72) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), 1, _GetEFLAGS_CF));
	_ce;
}
static void JNC_REL8()
{
	_cb("JNC_REL8");
	i386(0x73) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), 1, !_GetEFLAGS_CF));
	_ce;
}
static void JZ_REL8()
{
	_cb("JZ_REL8");
	i386(0x74) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), 1, _GetEFLAGS_ZF));
	_ce;
}
static void JNZ_REL8()
{
	_cb("JNZ_REL8");
	i386(0x75) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), 1, !_GetEFLAGS_ZF));
	_ce;
}
static void JNA_REL8()
{
	_cb("JNA_REL8");
	i386(0x76) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), 1,
		(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
	_ce;
}
static void JA_REL8()
{
	_cb("JA_REL8");
	i386(0x77) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), 1,
		!(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
	_ce;
}
static void JS_REL8()
{
	_cb("JS_REL8");
	i386(0x78) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), 1, _GetEFLAGS_SF));
	_ce;
}
static void JNS_REL8()
{
	_cb("JNS_REL8");
	i386(0x79) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), 1, !_GetEFLAGS_SF));
	_ce;
}
static void JP_REL8()
{
	_cb("JP_REL8");
	_newins_;
	i386(0x7a) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), 1, _GetEFLAGS_PF));
	_ce;
}
static void JNP_REL8()
{
	_cb("JNP_REL8");
	_newins_;
	i386(0x7b) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), 1, !_GetEFLAGS_PF));
	_ce;
}
static void JL_REL8()
{
	_cb("JL_REL8");
	i386(0x7c) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), 1, (_GetEFLAGS_SF != _GetEFLAGS_OF)));
	_ce;
}
static void JNL_REL8()
{
	_cb("JNL_REL8");
	i386(0x7d) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), 1, (_GetEFLAGS_SF == _GetEFLAGS_OF)));
	_ce;
}
static void JNG_REL8()
{
	_cb("JNG_REL8");
	i386(0x7e) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), 1,
		 (_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF))));
	_ce;
}
static void JG_REL8()
{
	_cb("JG_REL8");
	i386(0x7f) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), 1,
		(!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF))));
	_ce;
}
static void INS_80()
{
	_cb("INS_80");
	i386 (0x80) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(0, 1));
	_chk(_d_imm(1));
	_chk(_m_read_rm(1));
	switch (vcpuins.cr) {
	case 0: /* ADD_RM8_I8 */
		_bb("ADD_RM8_I8");
		_chk(_a_add(vcpuins.crm, vcpuins.cimm, 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
		_be;break;
	case 1: /* OR_RM8_I8 */
		_bb("OR_RM8_I8");
		_chk(_a_or(vcpuins.crm, vcpuins.cimm, 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
		_be;break;
	case 2: /* ADC_RM8_I8 */
		_bb("ADC_RM8_I8");
		_chk(_a_adc(vcpuins.crm, vcpuins.cimm, 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
		_be;break;
	case 3: /* SBB_RM8_I8 */
		_bb("SBB_RM8_I8");
		_newins_;
		_chk(_a_sbb(vcpuins.crm, vcpuins.cimm, 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
		_be;break;
	case 4: /* AND_RM8_I8 */
		_bb("AND_RM8_I8");
		_chk(_a_and(vcpuins.crm, vcpuins.cimm, 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
		_be;break;
	case 5: /* SUB_RM8_I8 */
		_bb("SUB_RM8_I8");
		_chk(_a_sub(vcpuins.crm, vcpuins.cimm, 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
		_be;break;
	case 6: /* XOR_RM8_I8 */
		_bb("XOR_RM8_I8");
		_chk(_a_xor(vcpuins.crm, vcpuins.cimm, 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
		_be;break;
	case 7: /* CMP_RM8_I8 */
		_bb("CMP_RM8_I8");
		_chk(_a_cmp(vcpuins.crm, vcpuins.cimm, 8));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INS_81()
{
	_cb("INS_81");
	i386(0x81) {
		_adv;
		_chk(_d_modrm(0, _GetOperandSize));
		_chk(_d_imm(_GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		switch (vcpuins.cr) {
		case 0: /* ADD_RM32_I32 */
			_bb("ADD_RM32_I32");
			_chk(_a_add(vcpuins.crm, vcpuins.cimm, _GetOperandSize * 8));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 1: /* OR_RM32_I32 */
			_bb("OR_RM32_I32");
			_chk(_a_or(vcpuins.crm, vcpuins.cimm, _GetOperandSize * 8));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 2: /* ADC_RM32_I32 */
			_bb("ADC_RM32_I32");
			_chk(_a_adc(vcpuins.crm, vcpuins.cimm, _GetOperandSize * 8));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 3: /* SBB_RM32_I32 */
			_bb("SBB_RM32_I32");
			_newins_;
			_chk(_a_sbb(vcpuins.crm, vcpuins.cimm, _GetOperandSize * 8));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 4: /* AND_RM32_I32 */
			_bb("AND_RM32_I32");
			_chk(_a_and(vcpuins.crm, vcpuins.cimm, _GetOperandSize * 8));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 5: /* SUB_RM32_I32 */
			_bb("SUB_RM32_I32");
			_chk(_a_sub(vcpuins.crm, vcpuins.cimm, _GetOperandSize * 8));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 6: /* XOR_RM32_I32 */
			_bb("XOR_RM32_I32");
			_chk(_a_xor(vcpuins.crm, vcpuins.cimm, _GetOperandSize * 8));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 7: /* CMP_RM32_I32 */
			_bb("CMP_RM32_I32");
			_chk(_a_cmp(vcpuins.crm, vcpuins.cimm, _GetOperandSize * 8));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_modrm(0, 2));
		_chk(_d_imm(2));
		_chk(_m_read_rm(2));
		switch (vcpuins.cr) {
		case 0: /* ADD_RM16_I16 */
			_bb("ADD_RM16_I16");
			_chk(_a_add(vcpuins.crm, vcpuins.cimm, 2 * 8));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			_be;break;
		case 1: /* OR_RM16_I16 */
			_bb("OR_RM16_I16");
			_chk(_a_or(vcpuins.crm, vcpuins.cimm, 2 * 8));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			_be;break;
		case 2: /* ADC_RM16_I16 */
			_bb("ADC_RM16_I16");
			_chk(_a_adc(vcpuins.crm, vcpuins.cimm, 2 * 8));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			_be;break;
		case 3: /* SBB_RM16_I16 */
			_bb("SBB_RM16_I16");
			_newins_;
			_chk(_a_sbb(vcpuins.crm, vcpuins.cimm, 2 * 8));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			_be;break;
		case 4: /* AND_RM16_I16 */
			_bb("AND_RM16_I16");
			_chk(_a_and(vcpuins.crm, vcpuins.cimm, 2 * 8));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			_be;break;
		case 5: /* SUB_RM16_I16 */
			_bb("SUB_RM16_I16");
			_chk(_a_sub(vcpuins.crm, vcpuins.cimm, 2 * 8));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			_be;break;
		case 6: /* XOR_RM16_I16 */
			_bb("XOR_RM16_I16");
			_chk(_a_xor(vcpuins.crm, vcpuins.cimm, 2 * 8));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			_be;break;
		case 7: /* CMP_RM16_I16 */
			_bb("CMP_RM16_I16");
			_chk(_a_cmp(vcpuins.crm, vcpuins.cimm, 2 * 8));
			_be;break;
		default:_impossible_;break;}
	}
	_ce;
}
static void INS_83()
{
	t_nubit8 bit;
	_cb("INS_83");
	i386(0x83) {
		_adv;
		_chk(_d_modrm(0, _GetOperandSize));
		_chk(_d_imm(1));
		_chk(_m_read_rm(_GetOperandSize));
		bit = (_GetOperandSize * 8 + 8) >> 1;
		switch (vcpuins.cr) {
		case 0: /* ADD_RM32_I8 */
			_bb("ADD_RM32_I8");
			_chk(_a_add(vcpuins.crm, vcpuins.cimm, bit));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 1: /* OR_RM32_I8 */
			_bb("OR_RM32_I8");
			_chk(_a_or(vcpuins.crm, vcpuins.cimm, bit));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 2: /* ADC_RM32_I8 */
			_bb("ADC_RM32_I8");
			_chk(_a_adc(vcpuins.crm, vcpuins.cimm, bit));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 3: /* SBB_RM32_I8 */
			_bb("SBB_RM32_I8");
			_chk(_a_sbb(vcpuins.crm, vcpuins.cimm, bit));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 4: /* AND_RM32_I8 */
			_bb("AND_RM32_I8");
			_chk(_a_and(vcpuins.crm, vcpuins.cimm, bit));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 5: /* SUB_RM32_I8 */
			_bb("SUB_RM32_I8");
			_chk(_a_sub(vcpuins.crm, vcpuins.cimm, bit));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 6: /* XOR_RM32_I8 */
			_bb("XOR_RM32_I8");
			_chk(_a_xor(vcpuins.crm, vcpuins.cimm, bit));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 7: /* CMP_RM32_I8 */
			_bb("CMP_RM32_I8");
			_chk(_a_cmp(vcpuins.crm, vcpuins.cimm, bit));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_modrm(0, 2));
		_chk(_d_imm(1));
		_chk(_m_read_rm(2));
		bit = 12;
		switch (vcpuins.cr) {
		case 0: /* ADD_RM16_I8 */
			_bb("ADD_RM16_I8");
			_chk(_a_add(vcpuins.crm, vcpuins.cimm, bit));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			_be;break;
		case 1: /* OR_RM16_I8 */
			_bb("OR_RM16_I8");
			_chk(_a_or(vcpuins.crm, vcpuins.cimm, bit));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			_be;break;
		case 2: /* ADC_RM16_I8 */
			_bb("ADC_RM16_I8");
			_chk(_a_adc(vcpuins.crm, vcpuins.cimm, bit));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			_be;break;
		case 3: /* SBB_RM16_I8 */
			_bb("SBB_RM16_I8");
			_chk(_a_sbb(vcpuins.crm, vcpuins.cimm, bit));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			_be;break;
		case 4: /* AND_RM16_I8 */
			_bb("AND_RM16_I8");
			_chk(_a_and(vcpuins.crm, vcpuins.cimm, bit));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			_be;break;
		case 5: /* SUB_RM16_I8 */
			_bb("SUB_RM16_I8");
			_chk(_a_sub(vcpuins.crm, vcpuins.cimm, bit));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			_be;break;
		case 6: /* XOR_RM16_I8 */
			_bb("XOR_RM16_I8");
			_chk(_a_xor(vcpuins.crm, vcpuins.cimm, bit));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			_be;break;
		case 7: /* CMP_RM16_I8 */
			_bb("CMP_RM16_I8");
			_chk(_a_cmp(vcpuins.crm, vcpuins.cimm, bit));
			_be;break;
		default:_impossible_;break;}
	}
	_ce;
}
static void TEST_RM8_R8()
{
	_cb("TEST_RM8_R8");
	i386(0x84) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_a_test(vcpuins.crm, vcpuins.cr, 8));
	_ce;
}
static void TEST_RM32_R32()
{
	_cb("TEST_RM32_R32");
	i386(0x85) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_a_test(vcpuins.crm, vcpuins.cr, _GetOperandSize * 8));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2);
		_chk(_m_read_rm(2));
		_a_test(vcpuins.crm, vcpuins.cr, 16);
	}
	_ce;
}
static void XCHG_RM8_R8()
{
	_cb("XCHG_RM8_R8");
	i386(0x86) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.crm), 1));
	vcpuins.crm = vcpuins.cr;_chk(_m_write_rm(1));
	_ce;
}
static void XCHG_RM32_R32()
{
	_cb("XCHG_RM32_R32");
	i386(0x87) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.crm), _GetOperandSize));
		vcpuins.crm = vcpuins.cr;_chk(_m_write_rm(_GetOperandSize));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2);
		_chk(_m_read_rm(2));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.crm), 2));
		vcpuins.crm = vcpuins.cr;_chk(_m_write_rm(2));
	}
	_ce;
}
static void MOV_RM8_R8()
{
	_cb("MOV_RM8_R8");
	i386(0x88) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	vcpuins.crm = vcpuins.cr;_chk(_m_write_rm(1));
	_ce;
}
static void MOV_RM32_R32()
{
	_cb("MOV_RM32_R32");
	i386(0x89) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		vcpuins.crm = vcpuins.cr;_chk(_m_write_rm(_GetOperandSize));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2);
		vcpuins.crm = vcpuins.cr;_chk(_m_write_rm(2));
	}
	_ce;
}
static void MOV_R8_RM8()
{
	_cb("MOV_R8_RM8");
	i386(0x8a) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(1, 1));
	_chk(_m_read_rm(1));
	_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.crm), 1));
	_ce;
}
static void MOV_R32_RM32()
{
	_cb("MOV_R32_RM32");
	i386(0x8b) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.crm), _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_modrm(2, 2);
		_chk(_m_read_rm(2));
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.crm), 2));
	}
	_ce;
}
static void MOV_RM16_SREG()
{
	_cb("MOV_RM16_SREG");
	i386(0x8c) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm_sreg(2));
	vcpuins.crm = vcpuins.rmovsreg->selector;
	_chk(_m_write_rm(2));
	_ce;
}
static void LEA_R32_M32()
{
	_cb("LEA_R32_M32");
	i386(0x8d) {
		_adv;
		_chk(_d_modrm_ea(_GetOperandSize, _GetOperandSize));
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.mrm.offset), 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.mrm.offset), 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_modrm_ea(2, 2);
		_m_write_ref(vcpuins.rr, GetRef(vcpuins.mrm.offset), 2);
	}
	_ce;
}
static void MOV_SREG_RM16()
{
	_cb("MOV_SREG_RM16");
	i386(0x8e) {
		_adv;
		_chk(_d_modrm_sreg(2));
		if (vcpuins.rmovsreg->sregtype == SREG_CODE) {
			_bb("sregtype(SREG_CODE)");
			_chk(_SetExcept_UD(0));
			_be;
		}
		_chk(_m_read_rm(2));
		_chk(_s_load_sreg(vcpuins.rmovsreg, GetMax16(vcpuins.crm)));
		if (vcpuins.rmovsreg->sregtype == SREG_STACK)
			vcpuins.flagmaskint = 1;
	} else {
		vcpu.ip++;
		_d_modrm_sreg(2);
		_chk(_m_read_rm(2));
		_chk(_s_load_sreg(vcpuins.rmovsreg, GetMax16(vcpuins.crm)));
	}
	_ce;
}
static void INS_8F()
{
	_cb("INS_8F");
	i386(0x8f) {
		_adv;
		_chk(_d_modrm(0, _GetOperandSize));
		switch (vcpuins.cr) {
		case 0: /* POP_RM32 */
			_bb("POP_RM32");
			_chk(vcpuins.crm = _e_pop(_GetOperandSize));
			_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 1: _bb("vcpuins.cr(1)");_chk(UndefinedOpcode());_be;break;
		case 2: _bb("vcpuins.cr(2)");_chk(UndefinedOpcode());_be;break;
		case 3: _bb("vcpuins.cr(3)");_chk(UndefinedOpcode());_be;break;
		case 4: _bb("vcpuins.cr(4)");_chk(UndefinedOpcode());_be;break;
		case 5: _bb("vcpuins.cr(5)");_chk(UndefinedOpcode());_be;break;
		case 6: _bb("vcpuins.cr(6)");_chk(UndefinedOpcode());_be;break;
		case 7: _bb("vcpuins.cr(7)");_chk(UndefinedOpcode());_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_modrm(0, 2));
		switch (vcpuins.cr) {
		case 0: /* POP_RM16 */
			_bb("POP_RM16");
			_chk(vcpuins.crm = GetMax16(_e_pop(2)));
			_chk(_m_write_rm(2));
			_be;break;
		case 1: _bb("vcpuins.cr(1)");_chk(UndefinedOpcode());_be;break;
		case 2: _bb("vcpuins.cr(2)");_chk(UndefinedOpcode());_be;break;
		case 3: _bb("vcpuins.cr(3)");_chk(UndefinedOpcode());_be;break;
		case 4: _bb("vcpuins.cr(4)");_chk(UndefinedOpcode());_be;break;
		case 5: _bb("vcpuins.cr(5)");_chk(UndefinedOpcode());_be;break;
		case 6: _bb("vcpuins.cr(6)");_chk(UndefinedOpcode());_be;break;
		case 7: _bb("vcpuins.cr(7)");_chk(UndefinedOpcode());_be;break;
		default:_impossible_;break;}
	}
	_ce;
}
static void NOP()
{
	_cb("NOP");
	i386(0x90) {
		_adv;
	} else
		vcpu.ip++;
	_ce;
}
static void XCHG_ECX_EAX()
{
	_cb("XCHG_ECX_EAX");
	i386(0x91) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			vcpuins.cr = vcpu.ax;
			vcpu.ax = vcpu.cx;
			vcpu.cx = GetMax16(vcpuins.cr);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			vcpuins.cr = vcpu.eax;
			vcpu.eax = vcpu.ecx;
			vcpu.ecx = GetMax32(vcpuins.cr);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		vcpuins.cr = vcpu.ax;
		vcpu.ax = vcpu.cx;
		vcpu.cx = GetMax16(vcpuins.cr);
	}
	_ce;
}
static void XCHG_EDX_EAX()
{
	_cb("XCHG_EDX_EAX");
	i386(0x92) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			vcpuins.cr = vcpu.ax;
			vcpu.ax = vcpu.dx;
			vcpu.dx = GetMax16(vcpuins.cr);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			vcpuins.cr = vcpu.eax;
			vcpu.eax = vcpu.edx;
			vcpu.edx = GetMax32(vcpuins.cr);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		vcpuins.cr = vcpu.ax;
		vcpu.ax = vcpu.dx;
		vcpu.dx = GetMax16(vcpuins.cr);
	}
	_ce;
}
static void XCHG_EBX_EAX()
{
	_cb("XCHG_EBX_EAX");
	i386(0x93) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			vcpuins.cr = vcpu.ax;
			vcpu.ax = vcpu.bx;
			vcpu.bx = GetMax16(vcpuins.cr);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			vcpuins.cr = vcpu.eax;
			vcpu.eax = vcpu.ebx;
			vcpu.ebx = GetMax32(vcpuins.cr);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		vcpuins.cr = vcpu.ax;
		vcpu.ax = vcpu.bx;
		vcpu.bx = GetMax16(vcpuins.cr);
	}
	_ce;
}
static void XCHG_ESP_EAX()
{
	_cb("XCHG_ESP_EAX");
	_newins_;
	i386(0x94) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			vcpuins.cr = vcpu.ax;
			vcpu.ax = vcpu.sp;
			vcpu.sp = GetMax16(vcpuins.cr);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			vcpuins.cr = vcpu.eax;
			vcpu.eax = vcpu.esp;
			vcpu.esp = GetMax32(vcpuins.cr);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		vcpuins.cr = vcpu.ax;
		vcpu.ax = vcpu.sp;
		vcpu.sp = GetMax16(vcpuins.cr);
	}
	_ce;
}
static void XCHG_EBP_EAX()
{
	_cb("XCHG_EBP_EAX");
	i386(0x95) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			vcpuins.cr = vcpu.ax;
			vcpu.ax = vcpu.bp;
			vcpu.bp = GetMax16(vcpuins.cr);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			vcpuins.cr = vcpu.eax;
			vcpu.eax = vcpu.ebp;
			vcpu.ebp = GetMax32(vcpuins.cr);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		vcpuins.cr = vcpu.ax;
		vcpu.ax = vcpu.bp;
		vcpu.bp = GetMax16(vcpuins.cr);
	}
	_ce;
}
static void XCHG_ESI_EAX()
{
	_cb("XCHG_ESI_EAX");
	i386(0x96) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			vcpuins.cr = vcpu.ax;
			vcpu.ax = vcpu.si;
			vcpu.si = GetMax16(vcpuins.cr);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			vcpuins.cr = vcpu.eax;
			vcpu.eax = vcpu.esi;
			vcpu.esi = GetMax32(vcpuins.cr);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		vcpuins.cr = vcpu.ax;
		vcpu.ax = vcpu.si;
		vcpu.si = GetMax16(vcpuins.cr);
	}
	_ce;
}
static void XCHG_EDI_EAX()
{
	_cb("XCHG_EDI_EAX");
	i386(0x97) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			vcpuins.cr = vcpu.ax;
			vcpu.ax = vcpu.di;
			vcpu.di = GetMax16(vcpuins.cr);
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			vcpuins.cr = vcpu.eax;
			vcpu.eax = vcpu.edi;
			vcpu.edi = GetMax32(vcpuins.cr);
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		vcpuins.cr = vcpu.ax;
		vcpu.ax = vcpu.di;
		vcpu.di = GetMax16(vcpuins.cr);
	}
	_ce;
}
static void CBW()
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
static void CWD()
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
static void CALL_PTR16_32()
{
	t_nubit16 newcs;
	t_nubit32 neweip;
	_cb("CALL_PTR16_32");
	i386(0x9a) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(4));
			neweip = GetMax16(vcpuins.cimm);
			newcs = GetMax16(vcpuins.cimm >> 16);
			_chk(_e_call_far(newcs, neweip, 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_d_imm(8));
			neweip = GetMax32(vcpuins.cimm);
			newcs = GetMax16(vcpuins.cimm >> 32);
			_chk(_e_call_far(newcs, neweip, 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(4);
		neweip = GetMax16(vcpuins.cimm);
		newcs = GetMax16(vcpuins.cimm >> 16);
		_e_call_far(newcs, neweip, 2);
	}
	_ce;
}
_______todo WAIT()
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
static void PUSHF()
{
	t_nubit32 ceflags;
	_cb("PUSHF");
	i386(0x9c) {
		_adv;
		if (!_GetCR0_PE || (_GetCR0_PE && (!_GetEFLAGS_VM || (_GetEFLAGS_VM && (_GetEFLAGS_IOPL == 3))))) {
			_bb("Real/Protected/(V86,IOPL(3))");
			switch (_GetOperandSize) {
			case 2: _bb("OperandSize(2)");
				ceflags = vcpu.flags;
				_chk(_e_push(ceflags, 2));
				_be;break;
			case 4: _bb("OperandSize(4)");
				_newins_;
				ceflags = vcpu.eflags & ~(VCPU_EFLAGS_VM | VCPU_EFLAGS_RF);
				_chk(_e_push(ceflags, 4));
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
		_e_push(vcpu.eflags, 2);
	}
	_ce;
}
static void POPF()
{
	t_nubit32 mask = VCPU_EFLAGS_RESERVED;
	t_nubit32 ceflags;
	_cb("POPF");
	i386(0x9d) {
		_adv;
		if (!_GetCR0_PE || !_GetEFLAGS_VM) {
			_bb("!V86");
			if (!_GetCPL) {
				_bb("CPL(0)");
				switch (_GetOperandSize) {
				case 2: _bb("OperandSize(2)");
					_chk(ceflags = GetMax16(_e_pop(2)));
					mask |= 0xffff0000;
					_be;break;
				case 4: _bb("OperandSize(4)");
					_chk(ceflags = GetMax32(_e_pop(4)));
					mask |= VCPU_EFLAGS_VM;
					_be;break;
				default:_impossible_;break;}
				_be;
			} else {
				_bb("CPL(!0)");
				_newins_;
				switch (_GetOperandSize) {
				case 2: _bb("OperandSize(2)");
					_chk(ceflags = GetMax16(_e_pop(2)));
					mask |= (0xffff0000 | VCPU_EFLAGS_IOPL);
					_be;break;
				case 4: _bb("OperandSize(4)");
					_chk(ceflags = GetMax32(_e_pop(4)));
					mask |= (VCPU_EFLAGS_VM | VCPU_EFLAGS_RF | VCPU_EFLAGS_IOPL);
					_be;break;
				default:_impossible_;break;}
				_be;
			}
			_be;
		} else {
			_bb("V86");
			if (vcpuins.prefix_oprsize) {
				_bb("prefix_oprsize(1)");
				_chk(_SetExcept_GP(0));
				_be;
			}
			if (_GetEFLAGS_IOPL == 0x03) {
				_bb("EFLAGS_IOPL(3)");
				switch (_GetOperandSize) {
				case 2: _bb("OperandSize(2)");
					_chk(ceflags = GetMax16(_e_pop(2)));
					mask |= (0xffff0000 | VCPU_EFLAGS_IOPL);
					_be;break;
				case 4: _bb("OperandSize(4)");
					_chk(ceflags = GetMax32(_e_pop(4)));
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
		_chk(vcpu.flags = GetMax16(_e_pop(2)));
	}
	_ce;
}
static void SAHF()
{
	t_nubit32 mask = (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
		VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_CF);
	_cb("SAHF");
	i386(0x9e) {
		_adv;
	} else {
		vcpu.ip++;
	}
	vcpu.eflags = (vcpu.ah & mask) | (vcpu.eflags & ~mask);
	_ce;
}
static void LAHF()
{
	_cb("LAHF");
	i386(0x9f) {
		_adv;
	} else {
		vcpu.ip++;
	}
	vcpu.ah = GetMax8(vcpu.flags);
	_ce;
}
static void MOV_AL_MOFFS8()
{
	_cb("MOV_AL_MOFFS8");
	i386(0xa0) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_moffs(1));
	_chk(_m_read_rm(1));
	vcpu.al = GetMax8(vcpuins.crm);
	_ce;
}
static void MOV_EAX_MOFFS32()
{
	_cb("MOV_EAX_MOFFS32");
	i386(0xa1) {
		_adv;
		_chk(_d_moffs(_GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpu.ax = GetMax16(vcpuins.crm);break;
		case 4: vcpu.eax = GetMax32(vcpuins.crm);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_moffs(2));
		_chk(_m_read_rm(2));
		vcpu.ax = GetMax16(vcpuins.crm);
	}
	_ce;
}
static void MOV_MOFFS8_AL()
{
	_cb("MOV_MOFFS8_AL");
	i386(0xa2) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_moffs(1));
	vcpuins.result = vcpu.al;
	vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
	_ce;
}
static void MOV_MOFFS32_EAX()
{
	_cb("MOV_MOFFS32_EAX");
	i386(0xa3) {
		_adv;
		_chk(_d_moffs(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpuins.result = vcpu.ax;break;
		case 4: vcpuins.result = vcpu.eax;break;
		default:_impossible_;break;}
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_moffs(2));
		vcpuins.crm = vcpu.ax;
		_chk(_m_write_rm(2));
	}
	_ce;
}
static void MOVSB()
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
			if (vcpu.cx) vcpuins.flaginsloop = 1;
		}
	}
	_ce;
}
static void MOVSW()
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
			if (vcpu.cx) vcpuins.flaginsloop = 1;
		}
	}
	_ce;
}
static void CMPSB()
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
				 vcpuins.flaginsloop = 1;
		}
	}
	_ce;
}
static void CMPSW()
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
				 vcpuins.flaginsloop = 1;
		}
	}
	_ce;
}
static void TEST_AL_I8()
{
	_cb("TEST_AL_I8");
	i386(0xa8) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_a_test(vcpu.al, vcpuins.cimm, 8));
	_ce;
}
static void TEST_EAX_I32()
{
	_cb("TEST_EAX_I32");
	i386(0xa9) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_a_test(vcpu.ax, vcpuins.cimm, 16));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_d_imm(4));
			_chk(_a_test(vcpu.eax, vcpuins.cimm, 32));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		_chk(_a_test(vcpu.ax, vcpuins.cimm, 16));
	}
	_ce;
}
static void STOSB()
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
			if (vcpu.cx) vcpuins.flaginsloop = 1;
		}
	}
	_ce;
}
static void STOSW()
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
			if (vcpu.cx) vcpuins.flaginsloop = 1;
		}
	}
	_ce;
}
static void LODSB()
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
			if (vcpu.cx) vcpuins.flaginsloop = 1;
		}
	}
	_ce;
}
static void LODSW()
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
			if (vcpu.cx) vcpuins.flaginsloop = 1;
		}
	}
	_ce;
}
static void SCASB()
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
				 vcpuins.flaginsloop = 1;
		}
	}
	_ce;
}
static void SCASW()
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
				 vcpuins.flaginsloop = 1;
		}
	}
	_ce;
}
static void MOV_AL_I8()
{
	_cb("MOV_AL_I8");
	i386(0xb0) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	vcpu.al = GetMax8(vcpuins.cimm);
	_ce;
}
static void MOV_CL_I8()
{
	_cb("MOV_CL_I8");
	i386(0xb1) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	vcpu.cl = GetMax8(vcpuins.cimm);
	_ce;
}
static void MOV_DL_I8()
{
	_cb("MOV_DL_I8");
	i386(0xb2) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	vcpu.dl = GetMax8(vcpuins.cimm);
	_ce;
}
static void MOV_BL_I8()
{
	_cb("MOV_BL_I8");
	i386(0xb3) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	vcpu.bl = GetMax8(vcpuins.cimm);
	_ce;
}
static void MOV_AH_I8()
{
	_cb("MOV_AH_I8");
	i386(0xb4) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	vcpu.ah = GetMax8(vcpuins.cimm);
	_ce;
}
static void MOV_CH_I8()
{
	_cb("MOV_CH_I8");
	i386(0xb5) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	vcpu.ch = GetMax8(vcpuins.cimm);
	_ce;
}
static void MOV_DH_I8()
{
	_cb("MOV_DH_I8");
	i386(0xb6) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	vcpu.dh = GetMax8(vcpuins.cimm);
	_ce;
}
static void MOV_BH_I8()
{
	_cb("MOV_BH_I8");
	i386(0xb7) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	vcpu.bh = GetMax8(vcpuins.cimm);
	_ce;
}
static void MOV_EAX_I32()
{
	_cb("MOV_EAX_I32");
	i386(0xb8) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpu.ax = GetMax16(vcpuins.cimm);break;
		case 4: vcpu.eax = GetMax32(vcpuins.cimm);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		vcpu.ax = GetMax16(vcpuins.cimm);
	}
	_ce;
}
static void MOV_ECX_I32()
{
	_cb("MOV_ECX_I32");
	i386(0xb9) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpu.cx = GetMax16(vcpuins.cimm);break;
		case 4: vcpu.ecx = GetMax32(vcpuins.cimm);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		vcpu.cx = GetMax16(vcpuins.cimm);
	}
	_ce;
}
static void MOV_EDX_I32()
{
	_cb("MOV_EDX_I32");
	i386(0xba) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpu.dx = GetMax16(vcpuins.cimm);break;
		case 4: vcpu.edx = GetMax32(vcpuins.cimm);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		vcpu.dx = GetMax16(vcpuins.cimm);
	}
	_ce;
}
static void MOV_EBX_I32()
{
	_cb("MOV_EBX_I32");
	i386(0xbb) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpu.bx = GetMax16(vcpuins.cimm);break;
		case 4: vcpu.ebx = GetMax32(vcpuins.cimm);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		vcpu.bx = GetMax16(vcpuins.cimm);
	}
	_ce;
}
static void MOV_ESP_I32()
{
	_cb("MOV_ESP_I32");
	i386(0xbc) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpu.sp = GetMax16(vcpuins.cimm);break;
		case 4: vcpu.esp = GetMax32(vcpuins.cimm);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		vcpu.sp = GetMax16(vcpuins.cimm);
	}
	_ce;
}
static void MOV_EBP_I32()
{
	_cb("MOV_EBP_I32");
	i386(0xbd) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpu.bp = GetMax16(vcpuins.cimm);break;
		case 4: vcpu.ebp = GetMax32(vcpuins.cimm);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		vcpu.bp = GetMax16(vcpuins.cimm);
	}
	_ce;
}
static void MOV_ESI_I32()
{
	_cb("MOV_ESI_I32");
	i386(0xbe) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpu.si = GetMax16(vcpuins.cimm);break;
		case 4: vcpu.esi = GetMax32(vcpuins.cimm);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		vcpu.si = GetMax16(vcpuins.cimm);
	}
	_ce;
}
static void MOV_EDI_I32()
{
	_cb("MOV_EDI_I32");
	i386(0xbf) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: vcpu.di = GetMax16(vcpuins.cimm);break;
		case 4: vcpu.edi = GetMax32(vcpuins.cimm);break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		vcpu.di = GetMax16(vcpuins.cimm);
	}
	_ce;
}
static void INS_C0()
{
	_cb("INS_C0");
	_newins_;
	i386(0xc0) {
		_adv;
		_chk(_d_modrm(0, 1));
		_chk(_m_read_rm(1));
		_chk(_d_imm(1));
		switch (vcpuins.cr) {
		case 0: /* ROL_RM8_I8 */
			_bb("ROL_RM8_I8");
			_chk(_a_rol(GetMax8(vcpuins.crm), GetMax8(vcpuins.cimm), 8));
			_be;break;
		case 1: /* ROR_RM8_I8 */
			_bb("ROR_RM8_I8");
			_chk(_a_ror(GetMax8(vcpuins.crm), GetMax8(vcpuins.cimm), 8));
			_be;break;
		case 2: /* RCL_RM8_I8 */
			_bb("RCL_RM8_I8");
			_chk(_a_rcl(GetMax8(vcpuins.crm), GetMax8(vcpuins.cimm), 8));
			_be;break;
		case 3: /* RCR_RM8_I8 */
			_bb("RCR_RM8_I8");
			_chk(_a_rcr(GetMax8(vcpuins.crm), GetMax8(vcpuins.cimm), 8));
			_be;break;
		case 4: /* SHL_RM8_I8 */
			_bb("SHL_RM8_I8");
			_chk(_a_shl(GetMax8(vcpuins.crm), GetMax8(vcpuins.cimm), 8));
			_be;break;
		case 5: /* SHR_RM8_I8 */
			_bb("SHR_RM8_I8");
			_chk(_a_shr(GetMax8(vcpuins.crm), GetMax8(vcpuins.cimm), 8));
			_be;break;
		case 6: /* UndefinedOpcode */
			_bb("vcpuins.cr(6)");
			_chk(UndefinedOpcode());
			_be;break;
		case 7: /* SAR_RM8_I8 */
			_bb("SAR_RM8_I8");
			_chk(_a_sar(GetMax8(vcpuins.crm), GetMax8(vcpuins.cimm), 8));
			_be;break;
		default:_impossible_;break;}
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
	} else
		UndefinedOpcode();
	_ce;
}
static void INS_C1()
{
	_cb("INS_C1");
	_newins_;
	i386(0xc1) {
		_adv;
		_chk(_d_modrm(0, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		_chk(_d_imm(1));
		switch (vcpuins.cr) {
		case 0: /* ROL_RM32_I8 */
			_bb("ROL_RM32_I8");
			_chk(_a_rol(GetMax32(vcpuins.crm), GetMax8(vcpuins.cimm), _GetOperandSize * 8));
			_be;break;
		case 1: /* ROR_RM32_I8 */
			_bb("ROR_RM32_I8");
			_chk(_a_ror(GetMax32(vcpuins.crm), GetMax8(vcpuins.cimm), _GetOperandSize * 8));
			_be;break;
		case 2: /* RCL_RM32_I8 */
			_bb("RCL_RM32_I8");
			_chk(_a_rcl(GetMax32(vcpuins.crm), GetMax8(vcpuins.cimm), _GetOperandSize * 8));
			_be;break;
		case 3: /* RCR_RM32_I8 */
			_bb("RCR_RM32_I8");
			_chk(_a_rcr(GetMax32(vcpuins.crm), GetMax8(vcpuins.cimm), _GetOperandSize * 8));
			_be;break;
		case 4: /* SHL_RM32_I8 */
			_bb("SHL_RM32_I8");
			_chk(_a_shl(GetMax32(vcpuins.crm), GetMax8(vcpuins.cimm), _GetOperandSize * 8));
			_be;break;
		case 5: /* SHR_RM32_I8 */
			_bb("SHR_RM32_I8");
			_chk(_a_shr(GetMax32(vcpuins.crm), GetMax8(vcpuins.cimm), _GetOperandSize * 8));
			_be;break;
		case 6: /* UndefinedOpcode */
			_bb("vcpuins.cr(6)");
			_chk(UndefinedOpcode());
			_be;break;
		case 7: /* SAR_RM32_I8 */
			_bb("SAR_RM32_I8");
			_chk(_a_sar(GetMax32(vcpuins.crm), GetMax8(vcpuins.cimm), _GetOperandSize * 8));
			_be;break;
		default:_impossible_;break;}
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
	} else 
		UndefinedOpcode();
	_ce;
}
static void RET_I16()
{
	_cb("RET_I16");
	i386(0xc2) {
		_adv;
		_chk(_d_imm(2));
		_chk(_e_ret_near(GetMax16(vcpuins.cimm), _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_imm(2);
		_e_ret_near(GetMax16(vcpuins.cimm), 2);
	}
	_ce;
}
static void RET()
{
	_cb("RET");
	i386(0xc3) {
		_adv;
		_chk(_e_ret_near(0, _GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_e_ret_near(0, 2));
	}
	_ce;
}
static void LES_R32_M16_32()
{
	t_nubit16 selector;
	t_nubit32 offset;
	_cb("LES_R32_M16_32");
	i386(0xc4) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		_chk(_m_read_rm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: offset = GetMax16(vcpuins.crm);break;
		case 4: offset = GetMax32(vcpuins.crm);break;
		default:_impossible_;break;}
		vcpuins.mrm.offset += _GetOperandSize;
		_chk(_m_read_rm(2));
		selector = GetMax16(vcpuins.crm);
		_chk(_e_load_far(&vcpu.es, vcpuins.rr, selector, offset, _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_modrm(2, 4);
		_chk(_m_read_rm(2));
		offset = GetMax16(vcpuins.crm);
		vcpuins.mrm.offset += 2;
		_chk(_m_read_rm(2));
		selector = GetMax16(vcpuins.crm);
		_chk(_e_load_far(&vcpu.es, vcpuins.rr, selector, offset, 2));
	}
	_ce;
}
static void LDS_R32_M16_32()
{
	t_nubit16 selector;
	t_nubit32 offset;
	_cb("LDS_R32_M16_32");
	i386(0xc5) {
		_adv;
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		_chk(_m_read_rm(_GetOperandSize));
		switch (_GetOperandSize) {
		case 2: offset = GetMax16(vcpuins.crm);break;
		case 4: offset = GetMax32(vcpuins.crm);break;
		default:_impossible_;break;}
		vcpuins.mrm.offset += _GetOperandSize;
		_chk(_m_read_rm(2));
		selector = GetMax16(vcpuins.crm);
		_chk(_e_load_far(&vcpu.ds, vcpuins.rr, selector, offset, _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_modrm(2, 4);
		_chk(_m_read_rm(2));
		offset = GetMax16(vcpuins.crm);
		vcpuins.mrm.offset += 2;
		_chk(_m_read_rm(2));
		selector = GetMax16(vcpuins.crm);
		_chk(_e_load_far(&vcpu.ds, vcpuins.rr, selector, offset, 2));
	}
	_ce;
}
static void INS_C6()
{
	_cb("INS_C6");
	i386(0xc6) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(0, 1));
	switch (vcpuins.cr) {
	case 0: /* MOV_RM8_I8 */
		_bb("MOV_RM8_I8");
		_chk(_d_imm(1));
		vcpuins.crm = vcpuins.cimm;_chk(_m_write_rm(1));
		_be;break;
	case 1: _bb("vcpuins.cr(1)");_chk(UndefinedOpcode());_be;break;
	case 2: _bb("vcpuins.cr(2)");_chk(UndefinedOpcode());_be;break;
	case 3: _bb("vcpuins.cr(3)");_chk(UndefinedOpcode());_be;break;
	case 4: _bb("vcpuins.cr(4)");_chk(UndefinedOpcode());_be;break;
	case 5: _bb("vcpuins.cr(5)");_chk(UndefinedOpcode());_be;break;
	case 6: _bb("vcpuins.cr(6)");_chk(UndefinedOpcode());_be;break;
	case 7: _bb("vcpuins.cr(7)");_chk(UndefinedOpcode());_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INS_C7()
{
	_cb("INS_C7");
	i386(0xc7) {
		_adv;
		_chk(_d_modrm(0, _GetOperandSize));
		switch (vcpuins.cr) {
		case 0: /* MOV_RM32_I32 */
			_bb("MOV_RM32_I32");
			_chk(_d_imm(_GetOperandSize));
			vcpuins.crm = vcpuins.cimm;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 1: _bb("vcpuins.cr(1)");_chk(UndefinedOpcode());_be;break;
		case 2: _bb("vcpuins.cr(2)");_chk(UndefinedOpcode());_be;break;
		case 3: _bb("vcpuins.cr(3)");_chk(UndefinedOpcode());_be;break;
		case 4: _bb("vcpuins.cr(4)");_chk(UndefinedOpcode());_be;break;
		case 5: _bb("vcpuins.cr(5)");_chk(UndefinedOpcode());_be;break;
		case 6: _bb("vcpuins.cr(6)");_chk(UndefinedOpcode());_be;break;
		case 7: _bb("vcpuins.cr(7)");_chk(UndefinedOpcode());_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_modrm(0, 2));
		switch (vcpuins.cr) {
		case 0: /* MOV_RM16_I16 */
			_bb("MOV_RM16_I16");
			_chk(_d_imm(2));
			vcpuins.crm = vcpuins.cimm;_chk(_m_write_rm(2));
			_be;break;
		case 1: _bb("vcpuins.cr(1)");_chk(UndefinedOpcode());_be;break;
		case 2: _bb("vcpuins.cr(2)");_chk(UndefinedOpcode());_be;break;
		case 3: _bb("vcpuins.cr(3)");_chk(UndefinedOpcode());_be;break;
		case 4: _bb("vcpuins.cr(4)");_chk(UndefinedOpcode());_be;break;
		case 5: _bb("vcpuins.cr(5)");_chk(UndefinedOpcode());_be;break;
		case 6: _bb("vcpuins.cr(6)");_chk(UndefinedOpcode());_be;break;
		case 7: _bb("vcpuins.cr(7)");_chk(UndefinedOpcode());_be;break;
		default:_impossible_;break;}
	}
	_ce;
}
static void ENTER()
{
	t_nubit32 data = 0;
	t_nubit32 temp = 0x00000000;
	t_nubit32 i = 0x00000000;
	t_nubit16 size = 0x0000;
	t_nubit8 level = 0x00;
	_cb("ENTER");
	i386(0xc8) {
		_adv;
		_chk(_d_imm(2));
		size = (t_nubit16)vcpuins.cimm;
		_chk(_d_imm(1));
		level = (t_nubit8)vcpuins.cimm;
		level %= 32;
		switch (_GetStackSize) {
		case 2: _bb("StackSize(2)");
			_chk(_e_push(vcpu.bp, 2));
			temp = vcpu.sp;
			_be;break;
		case 4: _bb("StackSize(4)");
			_chk(_e_push(vcpu.ebp, 4));
			temp = vcpu.esp;
			_be;break;
		default:_impossible_;break;}
		if (level) {
			_bb("level(!0)");
			for (i = 0;i < level;++i) {
				_bb("for");
				switch (_GetOperandSize) {
				case 2: _bb("OperandSize(2)");
					switch (_GetStackSize) {
					case 2: _bb("StackSize(2)");
						vcpu.bp -= 2;
						_chk(_s_read_ss(vcpu.bp, GetRef(data), 2));
						_chk(_e_push(data, 2));
						_be;break;
					case 4: _bb("StackSize(4)");
						vcpu.ebp -= 2;
						_chk(_s_read_ss(vcpu.ebp, GetRef(data), 2));
						_chk(_e_push(data, 2));
						_be;break;
					default:_impossible_;break;}
					_be;break;
				case 4: _bb("OperandSize(4)");
					switch (_GetStackSize) {
					case 2: _bb("StackSize(2)");
						vcpu.bp -= 4;
						_chk(_s_read_ss(vcpu.bp, GetRef(data), 4));
						_chk(_e_push(data, 4));
						_be;break;
					case 4: _bb("StackSize(4)");
						vcpu.ebp -= 4;
						_chk(_s_read_ss(vcpu.ebp, GetRef(data), 4));
						_chk(_e_push(data, 4));
						_be;break;
					default:_impossible_;break;}
					_be;break;
				default:_impossible_;break;}
				_be;
			}
			switch (_GetOperandSize) {
			case 2: _bb("OperandSize(2)");
				_chk(_e_push(temp, 2));
				_be;break;
			case 4: _bb("OperandSize(4)");
				_chk(_e_push(temp, 4));
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
static void LEAVE()
{
	_cb("LEAVE");
	i386(0xc9) {
		_adv;
		if (!_IsProtected && vcpu.ebp > 0x0000ffff) {
			_bb("Protected(0),ebp(>0000ffff)");
			_chk(_SetExcept_GP(0));
			_be;
		}
		_chk(_m_test_logical(&vcpu.ss, vcpu.ebp, _GetOperandSize, 1));
		switch (_GetStackSize) {
		case 2: vcpu.sp = vcpu.bp;break;
		case 4: vcpu.esp = vcpu.ebp;break;
		default:_impossible_;break;}
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(vcpu.bp = GetMax16(_e_pop(2)));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(vcpu.ebp = GetMax32(_e_pop(4)));
			_be;break;
		default:_impossible_;break;}
	} else
		UndefinedOpcode();
	_ce;
}
static void RETF_I16()
{
	_cb("RETF_I16");
	i386(0xca) {
		_adv;
		_chk(_d_imm(2));
		_chk(_e_ret_far(GetMax16(vcpuins.cimm), _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_imm(2);
		_e_ret_far(GetMax16(vcpuins.cimm), 2);
	}
	_ce;
}
static void RETF()
{
	_cb("RETF");
	i386(0xcb) {
		_adv;
		_chk(_e_ret_far(0, _GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_e_ret_far(0, 2));
	}
	_ce;
}
static void INT3()
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
static void INT_I8()
{
	_cb("INT_I8");
	i386(0xcc) {
		_adv;
		_chk(_d_imm(1));
		_chk(_e_int_n((t_nubit8)vcpuins.cimm, _GetOperandSize));
	} else {
		vcpu.ip++;
		_d_imm(1);
		_e_int_n((t_nubit8)vcpuins.cimm, 2);
	}
	_ce;
}
static void INTO()
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
static void IRET()
{
	_cb("IRET");
	i386(0xcf) {
		_adv;
		_chk(_e_iret(_GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_e_iret(2));
	}
	_ce;
}
static void INS_D0()
{
	_cb("INS_D0");
	i386(0xd0) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(0, 1));
	_chk(_m_read_rm(1));
	switch (vcpuins.cr) {
	case 0: /* ROL_RM8 */
		_bb("ROL_RM8");
		_chk(_a_rol(vcpuins.crm, 1, 8));
		_be;break;
	case 1: /* ROR_RM8 */
		_bb("ROR_RM8");
		_chk(_a_ror(vcpuins.crm, 1, 8));
		_be;break;
	case 2: /* RCL_RM8 */
		_bb("RCL_RM8");
		_chk(_a_rcl(vcpuins.crm, 1, 8));
		_be;break;
	case 3: /* RCR_RM8 */
		_bb("RCR_RM8");
		_chk(_a_rcr(vcpuins.crm, 1, 8));
		_be;break;
	case 4: /* SHL_RM8 */
		_bb("SHL_RM8");
		_chk(_a_shl(vcpuins.crm, 1, 8));
		_be;break;
	case 5: /* SHR_RM8 */
		_bb("SHR_RM8");
		_chk(_a_shr(vcpuins.crm, 1, 8));
		_be;break;
	case 6: /* UndefinedOpcode */
		_bb("vcpuins.cr(6)");
		_chk(UndefinedOpcode());
		_be;break;
	case 7: /* SAR_RM8 */
		_bb("SAR_RM8");
		_chk(_a_sar(vcpuins.crm, 1, 8));
		_be;break;
	default:_impossible_;break;}
	vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
	_ce;
}
static void INS_D1()
{
	_cb("INS_D1");
	i386(0xd1) {
		_adv;
		_chk(_d_modrm(0, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		switch (vcpuins.cr) {
		case 0: /* ROL_RM32 */
			_bb("ROL_RM32");
			_chk(_a_rol(vcpuins.crm, 1, _GetOperandSize * 8));
			_be;break;
		case 1: /* ROR_RM32 */
			_bb("ROR_RM32");
			_chk(_a_ror(vcpuins.crm, 1, _GetOperandSize * 8));
			_be;break;
		case 2: /* RCL_RM32 */
			_bb("RCL_RM32");
			_chk(_a_rcl(vcpuins.crm, 1, _GetOperandSize * 8));
			_be;break;
		case 3: /* RCR_RM32 */
			_bb("RCR_RM32");
			_chk(_a_rcr(vcpuins.crm, 1, _GetOperandSize * 8));
			_be;break;
		case 4: /* SHL_RM32 */
			_bb("SHL_RM32");
			_chk(_a_shl(vcpuins.crm, 1, _GetOperandSize * 8));
			_be;break;
		case 5: /* SHR_RM32 */
			_bb("SHR_RM32");
			_chk(_a_shr(vcpuins.crm, 1, _GetOperandSize * 8));
			_be;break;
		case 6: /* UndefinedOpcode */
			_bb("vcpuins.cr(6)");
			_chk(UndefinedOpcode());
			_be;break;
		case 7: /* SAR_RM32 */
			_bb("SAR_RM32");
			_chk(_a_sar(vcpuins.crm, 1, _GetOperandSize * 8));
			_be;break;
		default:_impossible_;break;}
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(0, 2));
		_chk(_m_read_rm(2));
		switch (vcpuins.cr) {
		case 0: /* ROL_RM16 */
			_bb("ROL_RM16");
			_chk(_a_rol(vcpuins.crm, 1, 16));
			_be;break;
		case 1: /* ROR_RM16 */
			_bb("ROR_RM16");
			_chk(_a_ror(vcpuins.crm, 1, 16));
			_be;break;
		case 2: /* RCL_RM16 */
			_bb("RCL_RM16");
			_chk(_a_rcl(vcpuins.crm, 1, 16));
			_be;break;
		case 3: /* RCR_RM16 */
			_bb("RCR_RM16");
			_chk(_a_rcr(vcpuins.crm, 1, 16));
			_be;break;
		case 4: /* SHL_RM16 */
			_bb("SHL_RM16");
			_chk(_a_shl(vcpuins.crm, 1, 16));
			_be;break;
		case 5: /* SHR_RM16 */
			_bb("SHR_RM16");
			_chk(_a_shr(vcpuins.crm, 1, 16));
			_be;break;
		case 6: /* UndefinedOpcode */
			_bb("vcpuins.cr(6)");
			_chk(UndefinedOpcode());
			_be;break;
		case 7: /* SAR_RM16 */
			_bb("SAR_RM16");
			_chk(_a_sar(vcpuins.crm, 1, 16));
			_be;break;
		default:_impossible_;break;}
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
	}
	_ce;
}
static void INS_D2()
{
	_cb("INS_D2");
	i386(0xd2) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(0, 1));
	_chk(_m_read_rm(1));
	switch (vcpuins.cr) {
	case 0: /* ROL_RM8_CL */
		_bb("ROL_RM8_CL");
		_chk(_a_rol(vcpuins.crm, vcpu.cl, 8));
		_be;break;
	case 1: /* ROR_RM8_CL */
		_bb("ROR_RM8_CL");
		_chk(_a_ror(vcpuins.crm, vcpu.cl, 8));
		_be;break;
	case 2: /* RCL_RM8_CL */
		_bb("RCL_RM8_CL");
		_chk(_a_rcl(vcpuins.crm, vcpu.cl, 8));
		_be;break;
	case 3: /* RCR_RM8_CL */
		_bb("RCR_RM8_CL");
		_chk(_a_rcr(vcpuins.crm, vcpu.cl, 8));
		_be;break;
	case 4: /* SHL_RM8_CL */
		_bb("SHL_RM8_CL");
		_chk(_a_shl(vcpuins.crm, vcpu.cl, 8));
		_be;break;
	case 5: /* SHR_RM8_CL */
		_bb("SHR_RM8_CL");
		_chk(_a_shr(vcpuins.crm, vcpu.cl, 8));
		_be;break;
	case 6: /* UndefinedOpcode */
		_bb("vcpuins.cr(6)");
		_chk(UndefinedOpcode());
		_be;break;
	case 7: /* SAR_RM8_CL */
		_bb("SAR_RM8_CL");
		_chk(_a_sar(vcpuins.crm, vcpu.cl, 8));
		_be;break;
	default:_impossible_;break;}
	vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
	_ce;
}
static void INS_D3()
{
	_cb("INS_D3");
	i386(0xd3) {
		_adv;
		_chk(_d_modrm(0, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		switch (vcpuins.cr) {
		case 0: /* ROL_RM32_CL */
			_bb("ROL_RM32_CL");
			_chk(_a_rol(vcpuins.crm, vcpu.cl, _GetOperandSize * 8));
			_be;break;
		case 1: /* ROR_RM32_CL */
			_bb("ROR_RM32_CL");
			_chk(_a_ror(vcpuins.crm, vcpu.cl, _GetOperandSize * 8));
			_be;break;
		case 2: /* RCL_RM32_CL */
			_bb("RCL_RM32_CL");
			_chk(_a_rcl(vcpuins.crm, vcpu.cl, _GetOperandSize * 8));
			_be;break;
		case 3: /* RCR_RM32_CL */
			_bb("RCR_RM32_CL");
			_chk(_a_rcr(vcpuins.crm, vcpu.cl, _GetOperandSize * 8));
			_be;break;
		case 4: /* SHL_RM32_CL */
			_bb("SHL_RM32_CL");
			_chk(_a_shl(vcpuins.crm, vcpu.cl, _GetOperandSize * 8));
			_be;break;
		case 5: /* SHR_RM32_CL */
			_bb("SHR_RM32_CL");
			_chk(_a_shr(vcpuins.crm, vcpu.cl, _GetOperandSize * 8));
			_be;break;
		case 6: /* UndefinedOpcode */
			_bb("vcpuins.cr(6)");
			_chk(UndefinedOpcode());
			_be;break;
		case 7: /* SAR_RM32_CL */
			_bb("SAR_RM32_CL");
			_chk(_a_sar(vcpuins.crm, vcpu.cl, _GetOperandSize * 8));
			_be;break;
		default:_impossible_;break;}
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
	} else {
		vcpu.ip++;
		_chk(_d_modrm(0, 2));
		_chk(_m_read_rm(2));
		switch (vcpuins.cr) {
		case 0: /* ROL_RM16_CL */
			_bb("ROL_RM16_CL");
			_chk(_a_rol(vcpuins.crm, vcpu.cl, 16));
			_be;break;
		case 1: /* ROR_RM16_CL */
			_bb("ROR_RM16_CL");
			_chk(_a_ror(vcpuins.crm, vcpu.cl, 16));
			_be;break;
		case 2: /* RCL_RM16_CL */
			_bb("RCL_RM16_CL");
			_chk(_a_rcl(vcpuins.crm, vcpu.cl, 16));
			_be;break;
		case 3: /* RCR_RM16_CL */
			_bb("RCR_RM16_CL");
			_chk(_a_rcr(vcpuins.crm, vcpu.cl, 16));
			_be;break;
		case 4: /* SHL_RM16_CL */
			_bb("SHL_RM16_CL");
			_chk(_a_shl(vcpuins.crm, vcpu.cl, 16));
			_be;break;
		case 5: /* SHR_RM16_CL */
			_bb("SHR_RM16_CL");
			_chk(_a_shr(vcpuins.crm, vcpu.cl, 16));
			_be;break;
		case 6: /* UndefinedOpcode */
			_bb("vcpuins.cr(6)");
			_chk(UndefinedOpcode());
			_be;break;
		case 7: /* SAR_RM16_CL */
			_bb("SAR_RM16_CL");
			_chk(_a_sar(vcpuins.crm, vcpu.cl, 16));
			_be;break;
		default:_impossible_;break;}
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
	}
	_ce;
}
static void AAM()
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
	base = GetMax8(vcpuins.cimm);
	vcpu.ah = vcpu.al / base;
	vcpu.al = vcpu.al % base;
	vcpuins.bit = 0x08;
	vcpuins.result = GetMax8(vcpu.al);
	_chk(_kaf_set_flags(AAM_FLAG));
	vcpuins.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF);
	_ce;
}
static void AAD()
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
	base = GetMax8(vcpuins.cimm);
	vcpu.al = GetMax8(vcpu.al + (vcpu.ah * base));
	vcpu.ah = 0x00;
	vcpuins.bit = 0x08;
	vcpuins.result = GetMax8(vcpu.al);
	_chk(_kaf_set_flags(AAD_FLAG));
	vcpuins.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF);
	_ce;
}
static void XLAT()
{
	_cb("XLAT");
	i386(0xd7) {
		_adv;
		switch (_GetAddressSize) {
		case 2: _bb("AddressSize(2)");
			_chk(_m_read_logical(vcpuins.roverds, (vcpu.bx + vcpu.al), GetRef(vcpu.al), 1));
			_be;break;
		case 4: _bb("AddressSize(4)");
			_chk(_m_read_logical(vcpuins.roverds, (vcpu.ebx + vcpu.al), GetRef(vcpu.al), 1));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_m_read_logical(vcpuins.roverds, (vcpu.bx + vcpu.al), GetRef(vcpu.al), 1));
	}
	_ce;
}
static void LOOPNZ_REL8()
{
	_cb("LOOPNZ_REL8");
	i386(0xe0) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_loopcc((t_nsbit8)vcpuins.cimm, !_GetEFLAGS_ZF));
	_ce;
}
static void LOOPZ_REL8()
{
	_cb("LOOPZ_REL8");
	i386(0xe1) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_loopcc((t_nsbit8)vcpuins.cimm, _GetEFLAGS_ZF));
	_ce;
}
static void LOOP_REL8()
{
	_cb("LOOP_REL8");
	i386(0xe2) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_loopcc((t_nsbit8)vcpuins.cimm, 1));
	_ce;
}
static void JCXZ_REL8()
{
	t_nubit32 cecx = 0x00000000;
	_cb("JCXZ_REL8");
	i386(0xe3) {
		_adv;
		_chk(_d_imm(1));
		switch (_GetAddressSize) {
		case 2: cecx = vcpu.cx;break;
		case 4: cecx = vcpu.ecx;break;
		default:_impossible_;break;}
		_chk(_e_jcc(GetMax32(vcpuins.cimm), 1, !cecx));
	} else {
		vcpu.ip++;
		_d_imm(1);
		_e_jcc(GetMax32(vcpuins.cimm), 1, !vcpu.cx);
	}
	_ce;
}
static void IN_AL_I8()
{
	_cb("IN_AL_I8");
	i386(0xe4) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_p_input(GetMax8(vcpuins.cimm), GetRef(vcpu.al), 1));
	_ce;
}
static void IN_EAX_I8()
{
	_cb("IN_EAX_I8");
	_newins_;
	i386(0xe5) {
		_adv;
		_chk(_d_imm(1));
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_p_input(GetMax8(vcpuins.cimm), GetRef(vcpu.ax), 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_p_input(GetMax8(vcpuins.cimm), GetRef(vcpu.eax), 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_imm(1));
		_chk(_p_input(GetMax8(vcpuins.cimm), GetRef(vcpu.ax), 2));
	}
	_ce;
}
static void OUT_I8_AL()
{
	_cb("OUT_I8_AL");
	i386(0xe6) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_p_output(GetMax8(vcpuins.cimm), GetRef(vcpu.al), 1));
	_ce;

}
static void OUT_I8_EAX()
{
	_cb("OUT_I8_EAX");
	_newins_;
	i386(0xe7) {
		_adv;
		_chk(_d_imm(1));
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_p_output(GetMax8(vcpuins.cimm), GetRef(vcpu.ax), 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_p_output(GetMax8(vcpuins.cimm), GetRef(vcpu.eax), 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_d_imm(1);
		_p_output(GetMax8(vcpuins.cimm), GetRef(vcpu.ax), 2);
	}
	_ce;
}
static void CALL_REL32()
{
	_cb("CALL_REL32");
	i386(0xe8) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_d_imm(2));
			_chk(_e_call_near(GetMax16(vcpu.ip + (t_nsbit16)vcpuins.cimm), 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_newins_;
			_chk(_d_imm(4));
			_chk(_e_call_near(GetMax32(vcpu.eip + (t_nsbit32)vcpuins.cimm), 4));
			_be;break;
		default: _bb("OperandSize");
			_chk(_SetExcept_CE(_GetOperandSize));
			_be;break;
		}
	} else {
		vcpu.ip++;
		_chk(_d_imm(2));
		_chk(_e_call_near(GetMax16(vcpu.ip + (t_nsbit16)vcpuins.cimm), 2));
	}
	_ce;
}
static void JMP_REL32()
{
	_cb("JMP_REL32");
	i386(0xe9) {
		_adv;
		_chk(_d_imm(_GetOperandSize));
		_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, 1));
	} else {
		vcpu.ip++;
		_d_imm(2);
		_e_jcc(GetMax32(vcpuins.cimm), 2, 1);
	}
	_ce;
}
static void JMP_PTR16_32()
{
	t_nubit16 newcs = 0x0000;
	t_nubit32 neweip = 0x00000000;
	_cb("JMP_PTR16_32");
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
		_chk(_d_imm(2));
		neweip = (t_nubit16)vcpuins.cimm;
		_chk(_d_imm(2));
		newcs = (t_nubit16)vcpuins.cimm;
		_chk(_e_jmp_far(newcs, neweip, 2));
	}
	_ce;
}
static void JMP_REL8()
{
	_cb("JMP_REL8");
	i386(0xeb) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_imm(1));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), 1, 1));
	_ce;
}
static void IN_AL_DX()
{
	_cb("IN_AL_DX");
	i386(0xec) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_p_input(vcpu.dx, GetRef(vcpu.al), 1));
	_ce;
}
static void IN_EAX_DX()
{
	_cb("IN_EAX_DX");
	i386(0xed) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_p_input(vcpu.dx, GetRef(vcpu.ax), 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_p_input(vcpu.dx, GetRef(vcpu.eax), 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_p_input(vcpu.dx, GetRef(vcpu.ax), 2));
	}
	_ce;
}
static void OUT_DX_AL()
{
	_cb("OUT_DX_AL");
	i386(0xee) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_p_output(vcpu.dx, GetRef(vcpu.al), 1));
	_ce;
}
static void OUT_DX_EAX()
{
	_cb("OUT_DX_EAX");
	_newins_;
	i386(0xef) {
		_adv;
		switch (_GetOperandSize) {
		case 2: _bb("OperandSize(2)");
			_chk(_p_output(vcpu.dx, GetRef(vcpu.ax), 2));
			_be;break;
		case 4: _bb("OperandSize(4)");
			_chk(_p_output(vcpu.dx, GetRef(vcpu.eax), 4));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_p_output(vcpu.dx, GetRef(vcpu.ax), 2);
	}
	_ce;
}
static void PREFIX_LOCK()
{
	t_nubit8 opcode = 0x00;
	t_nubit8 modrm = 0x00;
	t_nubit8 opcode_0f = 0x00;
	t_nubit32 ceip = vcpu.eip;
	_cb("PREFIX_LOCK");
	i386(0xf0) {
		_adv;
		do {
			_chk(_s_read_cs(ceip, GetRef(opcode), 1));
			ceip++;
		} while (_kdf_check_prefix(opcode));
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
			_chk(_s_read_cs(ceip, GetRef(modrm), 1));
			if (_GetModRM_REG(modrm) != 7)
				vcpu.flaglock = 1;
			else {
				_bb("ModRM_REG(7)");
				_chk(_SetExcept_UD(0));
				_be;
			}
			_be;break;
		case 0x0f: _bb("opcode(0f)");
			_chk(_s_read_cs(ceip, GetRef(opcode_0f), 1));
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
static void PREFIX_REPNZ()
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
static void PREFIX_REPZ()
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
static void HLT()
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
static void CMC()
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
static void INS_F6()
{
	_cb("INS_F6");
	i386(0xf6) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(0, 1));
	_chk(_m_read_rm(1));
	switch (vcpuins.cr) {
	case 0: /* TEST_RM8_I8 */
		_bb("TEST_RM8_I8");
		_chk(_d_imm(1));
		_chk(_a_test(vcpuins.crm, vcpuins.cimm, 8));
		_be;break;
	case 1: /* UndefinedOpcode */
		_bb("ModRM_REG(1)");
		_chk(UndefinedOpcode());
		_be;break;
	case 2: /* NOT_RM8 */
		_bb("NOT_RM8");
		_chk(_a_not(vcpuins.crm, 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
		_be;break;
	case 3: /* NEG_RM8 */
		_bb("NEG_RM8");
		_chk(_a_neg(vcpuins.crm, 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
		_be;break;
	case 4: /* MUL_RM8 */
		_bb("MUL_RM8");
		_chk(_a_mul(vcpuins.crm, 8));
		_be;break;
	case 5: /* IMUL_RM8 */
		_bb("IMUL_RM8");
		_chk(_a_imul(vcpuins.crm, 8));
		_be;break;
	case 6: /* DIV_RM8 */
		_bb("DIV_RM8");
		_chk(_a_div(vcpuins.crm, 8));
		_be;break;
	case 7: /* IDIV_RM8 */
		_bb("IDIV_RM8");
		_newins_;
		_chk(_a_idiv(vcpuins.crm, 8));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INS_F7()
{
	_cb("INS_F7");
	i386(0xf7) {
		_adv;
		_chk(_d_modrm(0, _GetOperandSize));
		_chk(_m_read_rm(_GetOperandSize));
		switch (vcpuins.cr) {
		case 0: /* TEST_RM32_I32 */
			_bb("TEST_RM32_I32");
			_chk(_d_imm(_GetOperandSize));
			_chk(_a_test(vcpuins.crm, vcpuins.cimm, _GetOperandSize * 8));
			_be;break;
		case 1: /* UndefinedOpcode */
			_bb("ModRM_REG(1)");
			_chk(UndefinedOpcode());
			_be;break;
		case 2: /* NOT_RM32 */
			_bb("NOT_RM32");
			_chk(_a_not(vcpuins.crm, _GetOperandSize * 8));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 3: /* NEG_RM32 */
			_bb("NEG_RM32");
			_chk(_a_neg(vcpuins.crm, _GetOperandSize * 8));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 4: /* MUL_RM32 */
			_bb("MUL_RM32");
			_chk(_a_mul(vcpuins.crm, _GetOperandSize * 8));
			_be;break;
		case 5: /* IMUL_RM32 */
			_bb("IMUL_RM32");
			_chk(_a_imul(vcpuins.crm, _GetOperandSize * 8));
			_be;break;
		case 6: /* DIV_RM32 */
			_bb("DIV_RM32");
			_chk(_a_div(vcpuins.crm, _GetOperandSize * 8));
			_be;break;
		case 7: /* IDIV_RM32 */
			_bb("IDIV_RM32");
			_chk(_a_idiv(vcpuins.crm, _GetOperandSize * 8));
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_d_modrm(0, 2));
		_chk(_m_read_rm(2));
		switch (vcpuins.cr) {
		case 0: /* TEST_RM16_I16 */
			_bb("TEST_RM16_I16");
			_chk(_d_imm(2));
			_chk(_a_test(vcpuins.crm, vcpuins.cimm, 16));
			_be;break;
		case 1: /* UndefinedOpcode */
			_bb("ModRM_REG(1)");
			_chk(UndefinedOpcode());
			_be;break;
		case 2: /* NOT_RM16 */
			_bb("NOT_RM16");
			_chk(_a_not(vcpuins.crm, 16));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			_be;break;
		case 3: /* NEG_RM16 */
			_bb("NEG_RM16");
			_chk(_a_neg(vcpuins.crm, 16));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			_be;break;
		case 4: /* MUL_RM16 */
			_bb("MUL_RM16");
			_chk(_a_mul(vcpuins.crm, 16));
			_be;break;
		case 5: /* IMUL_RM16 */
			_bb("IMUL_RM16");
			_chk(_a_imul(vcpuins.crm, 16));
			_be;break;
		case 6: /* DIV_RM16 */
			_bb("DIV_RM16");
			_chk(_a_div(vcpuins.crm, 16));
			_be;break;
		case 7: /* IDIV_RM16 */
			_bb("IDIV_RM16");
			_chk(_a_idiv(vcpuins.crm, 16));
			_be;break;
		default:_impossible_;break;}
	}
	_ce;
}
static void CLC()
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
static void STC()
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
static void CLI()
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
static void STI()
{
	_cb("STI");
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
static void CLD()
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
static void STD()
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
static void INS_FE()
{
	_cb("INS_FE");
	i386(0xfe) {
		_adv;
	} else {
		vcpu.ip++;
	}
	_chk(_d_modrm(0, 1));
	_chk(_m_read_rm(1));
	switch (vcpuins.cr) {
	case 0: /* INC_RM8 */
		_bb("INC_RM8");
		_chk(_a_inc(vcpuins.crm, 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
		_be;break;
	case 1: /* DEC_RM8 */
		_bb("DEC_RM8");
		_chk(_a_dec(vcpuins.crm, 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
		_be;break;
	case 2: _bb("vcpuins.cr(2)");_chk(UndefinedOpcode());_be;break;
	case 3: _bb("vcpuins.cr(3)");_chk(UndefinedOpcode());_be;break;
	case 4: _bb("vcpuins.cr(4)");_chk(UndefinedOpcode());_be;break;
	case 5: _bb("vcpuins.cr(5)");_chk(UndefinedOpcode());_be;break;
	case 6: _bb("vcpuins.cr(6)");_chk(UndefinedOpcode());_be;break;
	case 7: _bb("vcpuins.cr(7)");_chk(UndefinedOpcode());_be;break;
	default:_impossible_;break;}
	_ce;
}
static void INS_FF()
{
	t_nubit8 modrm;
	t_nubit16 newcs;
	t_nubit32 neweip;
	_cb("INS_FF");
	i386(0xff) {
		_adv;
		_chk(_s_read_cs(vcpu.eip, GetRef(modrm), 1));
		switch (_GetModRM_REG(modrm)) {
		case 0: /* INC_RM32 */
			_bb("INC_RM32");
			_chk(_d_modrm(0, _GetOperandSize));
			_chk(_m_read_rm(_GetOperandSize));
			_chk(_a_inc(vcpuins.crm, _GetOperandSize * 8));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 1: /* DEC_RM32 */
			_bb("DEC_RM32");
			_chk(_d_modrm(0, _GetOperandSize));
			_chk(_m_read_rm(_GetOperandSize));
			_chk(_a_dec(vcpuins.crm, _GetOperandSize * 8));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
			_be;break;
		case 2: /* CALL_RM32 */
			_bb("CALL_RM32");
			_chk(_d_modrm(0, _GetOperandSize));
			_chk(_m_read_rm(_GetOperandSize));
			neweip = GetMax32(vcpuins.crm);
			_chk(_e_call_near(neweip, _GetOperandSize));
			_be;break;
		case 3: /* CALL_M16_32 */
			_bb("CALL_M16_32");
			_chk(_d_modrm(0, _GetOperandSize + 2));
			if (!vcpuins.flagmem) {
				_bb("flagmem(0)");
				_chk(UndefinedOpcode());
				_be;
			}
			_chk(_m_read_rm(_GetOperandSize));
			neweip = (t_nubit32)vcpuins.crm;
			vcpuins.mrm.offset += _GetOperandSize;
			_chk(_m_read_rm(2));
			newcs = GetMax16(vcpuins.crm);
			_chk(_e_call_far(newcs, neweip, _GetOperandSize));
			_be;break;
		case 4: /* JMP_RM32 */
			_bb("JMP_RM32");
			_chk(_d_modrm(0, _GetOperandSize));
			_chk(_m_read_rm(_GetOperandSize));
			neweip = GetMax32(vcpuins.crm);
			_chk(_e_jmp_near(neweip, _GetOperandSize));
			_be;break;
		case 5: /* JMP_M16_32 */
			_bb("JMP_M16_32");
			_chk(_d_modrm(0, _GetOperandSize + 2));
			if (!vcpuins.flagmem) {
				_bb("flagmem(0)");
				_chk(UndefinedOpcode());
				_be;
			}
			_chk(_m_read_rm(_GetOperandSize));
			neweip = (t_nubit32)vcpuins.crm;
			vcpuins.mrm.offset += _GetOperandSize;
			_chk(_m_read_rm(2));
			newcs = GetMax16(vcpuins.crm);
			_chk(_e_jmp_far(newcs, neweip, _GetOperandSize));
			_be;break;
		case 6: /* PUSH_RM32 */
			_bb("PUSH_RM32");
			_chk(_d_modrm(0, _GetOperandSize));
			_chk(_m_read_rm(_GetOperandSize));
			_chk(_e_push(vcpuins.crm, _GetOperandSize));
			_be;break;
		case 7: /* UndefinedOpcode */
			_bb("ModRM_REG(7)");
			_chk(UndefinedOpcode());
			_be;break;
		default:_impossible_;break;}
	} else {
		vcpu.ip++;
		_chk(_s_read_cs(vcpu.eip, GetRef(modrm), 1));
		switch (_GetModRM_REG(modrm)) {
		case 0: /* INC_RM16 */
			_bb("INC_RM16");
			_chk(_d_modrm(0, 2));
			_chk(_m_read_rm(2));
			_chk(_a_inc(vcpuins.crm, 16));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			_be;break;
		case 1: /* DEC_RM16 */
			_bb("DEC_RM16");
			_chk(_d_modrm(0, 2));
			_chk(_m_read_rm(2));
			_chk(_a_dec(vcpuins.crm, 16));
			vcpuins.crm = vcpuins.result;_chk(_m_write_rm(2));
			_be;break;
		case 2: /* CALL_RM16 */
			_bb("CALL_RM16");
			_chk(_d_modrm(0, 2));
			_chk(_m_read_rm(2));
			neweip = GetMax16(vcpuins.crm);
			_chk(_e_call_near(neweip, 2));
			_be;break;
		case 3: /* CALL_M16_16 */
			_bb("CALL_M16_16");
			_chk(_d_modrm(0, 4));
			if (!vcpuins.flagmem) {
				_bb("flagmem(0)");
				_chk(UndefinedOpcode());
				_be;
			}
			_chk(_m_read_rm(2));
			neweip = (t_nubit16)vcpuins.crm;
			vcpuins.mrm.offset += 2;
			_chk(_m_read_rm(2));
			newcs = GetMax16(vcpuins.crm);
			_chk(_e_call_far(newcs, neweip, 2));
			_be;break;
		case 4: /* JMP_RM16 */
			_bb("JMP_RM16");
			_chk(_d_modrm(0, 2));
			_chk(_m_read_rm(2));
			neweip = GetMax16(vcpuins.crm);
			_chk(_e_jmp_near(neweip, 2));
			_be;break;
		case 5: /* JMP_M16_16 */
			_bb("JMP_M16_16");
			_chk(_d_modrm(0, 4));
			if (!vcpuins.flagmem) {
				_bb("flagmem(0)");
				_chk(UndefinedOpcode());
				_be;
			}
			_chk(_m_read_rm(2));
			neweip = (t_nubit16)vcpuins.crm;
			vcpuins.mrm.offset += 2;
			_chk(_m_read_rm(2));
			newcs = GetMax16(vcpuins.crm);
			_chk(_e_jmp_far(newcs, neweip, 2));
			_be;break;
		case 6: /* PUSH_RM16 */
			_bb("PUSH_RM16");
			_chk(_d_modrm(0, 2));
			_chk(_m_read_rm(2));
			_chk(_e_push(vcpuins.crm, 2));
			_be;break;
		case 7: /* UndefinedOpcode */
			_bb("ModRM_REG(7)");
			_chk(UndefinedOpcode());
			_be;break;
		default:_impossible_;break;}
	}
	_ce;
}

static void _d_modrm_creg()
{
	_cb("_d_modrm_creg");
	_chk(_kdf_modrm(0, 4));
	if (vcpuins.flagmem) {
		_bb("flagmem(1)");
		_chk(_SetExcept_UD(0));
		_be;
	}
	switch (vcpuins.cr) {
	case 0: vcpuins.rr = (t_vaddrcc)&vcpu.cr0;vcpuins.cr = vcpu.cr0;break;
	case 1: _bb("vcpuins.cr(1)");_chk(_SetExcept_UD(0));_be;break;
	case 2: vcpuins.rr = (t_vaddrcc)&vcpu.cr2;vcpuins.cr = vcpu.cr2;break;
	case 3: vcpuins.rr = (t_vaddrcc)&vcpu.cr3;vcpuins.cr = vcpu.cr3;break;
	case 4: _bb("vcpuins.cr(4)");_chk(_SetExcept_UD(0));_be;break;
	case 5: _bb("vcpuins.cr(5)");_chk(_SetExcept_UD(0));_be;break;
	case 6: _bb("vcpuins.cr(6)");_chk(_SetExcept_UD(0));_be;break;
	case 7: _bb("vcpuins.cr(7)");_chk(_SetExcept_UD(0));_be;break;
	default:_impossible_;break;}
	_ce;
}
static void _d_modrm_dreg()
{
	_cb("_d_modrm_dreg");
	_chk(_kdf_modrm(0, 4));
	if (vcpuins.flagmem) {
		_bb("flagmem(1)");
		_chk(_SetExcept_UD(0));
		_be;
	}
	switch (vcpuins.cr) {
	case 0: vcpuins.rr = (t_vaddrcc)&vcpu.dr0;vcpuins.cr = vcpu.dr0;break;
	case 1: vcpuins.rr = (t_vaddrcc)&vcpu.dr1;vcpuins.cr = vcpu.dr1;break;
	case 2: vcpuins.rr = (t_vaddrcc)&vcpu.dr2;vcpuins.cr = vcpu.dr2;break;
	case 3: vcpuins.rr = (t_vaddrcc)&vcpu.dr3;vcpuins.cr = vcpu.dr3;break;
	case 4: _bb("vcpuins.cr(4)");_chk(_SetExcept_UD(0));_be;break;
	case 5: _bb("vcpuins.cr(5)");_chk(_SetExcept_UD(0));_be;break;
	case 6: vcpuins.rr = (t_vaddrcc)&vcpu.dr6;vcpuins.cr = vcpu.dr6;break;
	case 7: vcpuins.rr = (t_vaddrcc)&vcpu.dr7;vcpuins.cr = vcpu.dr7;break;
	default:_impossible_;break;}
	_ce;
}
static void _d_modrm_treg()
{
	_cb("_d_modrm_treg");
	_chk(_kdf_modrm(0, 4));
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
	case 6: vcpuins.rr = (t_vaddrcc)&vcpu.tr6;vcpuins.cr = vcpu.tr6;break;
	case 7: vcpuins.rr = (t_vaddrcc)&vcpu.tr7;vcpuins.cr = vcpu.tr7;break;
	default:_impossible_;break;}
	_ce;
}
static void _d_bit_rmimm(t_nubit8 regbyte, t_nubit8 rmbyte, t_bool write)
{
	/* xrm = actual destination, cimm = (1 << bitoffset) */
	t_nsbit16 bitoff16 = 0;
	t_nsbit32 bitoff32 = 0;
	t_nubit32 bitoperand = 0;
	_cb("_d_bit_rmimm");
	_chk(_kdf_modrm(regbyte, rmbyte));
	if (!regbyte) _chk(_d_imm(1));
	switch (rmbyte) {
	case 2: _bb("rmbyte(2)");
		if (vcpuins.flagmem && regbyte) {
			/* valid for btcc_m16_r16 */
			_bb("flagmem(1),regbyte(1)");
			bitoff16 = (t_nsbit16)vcpuins.cr;
			if (bitoff16 >= 0)
				vcpuins.mrm.offset += 2 * (bitoff16 / 16);
			else
				vcpuins.mrm.offset += 2 * ((bitoff16 - 15) / 16);
			bitoperand = ((t_nubit16)bitoff16) % 16;
			_be;
		} else if (regbyte) {
			bitoperand = vcpuins.cr % 16;
		} else {
			bitoperand = (GetMax16(vcpuins.cimm) % 16);
		}
		_chk(vcpuins.cimm = GetMax16((1 << bitoperand)));
		_be;break;
	case 4: _bb("rmbyte(4)");
		if (vcpuins.flagmem && regbyte) {
			_bb("flagmem(1),regbyte(1)");
			bitoff32 = (t_nsbit32)vcpuins.cr;
			if (bitoff32 >= 0)
				vcpuins.mrm.offset += 4 * (bitoff32 / 32);
			else
				vcpuins.mrm.offset += 4 * ((bitoff32 - 31) / 32);
			bitoperand = ((t_nubit32)bitoff32) % 32;
			_be;
		} else if (regbyte) {
			bitoperand = vcpuins.cr % 32;
		} else {
			bitoperand = (GetMax32(vcpuins.cimm) % 32);
		}
		_chk(vcpuins.cimm = GetMax32((1 << bitoperand)));
		_be;break;
	default: _bb("rmbyte");
		_chk(_SetExcept_CE(rmbyte));
		_be;break;
	}
	_ce;
}

#define SHLD_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SHRD_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)

#define _kac_btcc \
do { \
	switch (bit) { \
	case 16: vcpuins.opr1 = GetMax16(cdest);break; \
	case 32: vcpuins.opr1 = GetMax32(cdest);break; \
	default: _bb("bit"); \
		_chk(_SetExcept_CE(bit)); \
		_be;break; \
	} \
	vcpuins.opr2 = bitoperand; \
	vcpuins.result = vcpuins.opr1; \
	MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, GetBit(vcpuins.opr1, vcpuins.opr2)); \
} while (0)

static void _a_bscc(t_nubit64 csrc, t_nubit8 bit, t_bool forward)
{
	t_nubit32 temp;
	_cb("_a_bscc");
	if (forward) temp = 0;
	else temp = bit - 1;
	switch (bit) {
	case 16: _bb("bit(16)");
		vcpuins.opr1 = GetMax16(csrc);
		if (!vcpuins.opr1)
			_SetEFLAGS_ZF;
		else {
			_ClrEFLAGS_ZF;
			while (!GetBit(vcpuins.opr1, GetMax64(1 << temp))) {
				if (forward) temp++;
				else temp--;
			}
			vcpuins.result = GetMax16(temp);
		}
		_be;break;
	case 32: _bb("bit(32)");
		vcpuins.opr1 = GetMax32(csrc);
		if (!vcpuins.opr1)
			_SetEFLAGS_ZF;
		else {
			_ClrEFLAGS_ZF;
			while (!GetBit(vcpuins.opr1, GetMax64(1 << temp))) {
				if (forward) temp++;
				else temp--;
			}
			vcpuins.result = GetMax32(temp);
		}
		_be;break;
	default: _bb("bit");
		_chk(_SetExcept_CE(bit));
		_be;break;
	}
	_ce;
}
static void _a_bt(t_nubit64 cdest, t_nubit32 bitoperand, t_nubit8 bit)
{
	_cb("_a_bt");
	_kac_btcc;
	_ce;
}
static void _a_btc(t_nubit64 cdest, t_nubit32 bitoperand, t_nubit8 bit)
{
	_cb("_a_btc");
	_kac_btcc;
	MakeBit(vcpuins.result, vcpuins.opr2, !_GetEFLAGS_CF);
	_ce;
}
static void _a_btr(t_nubit64 cdest, t_nubit32 bitoperand, t_nubit8 bit)
{
	_cb("_a_btr");
	_kac_btcc;
	ClrBit(vcpuins.result, vcpuins.opr2);
	_ce;
}
static void _a_bts(t_nubit64 cdest, t_nubit32 bitoperand, t_nubit8 bit)
{
	_cb("_a_bts");
	_kac_btcc;
	SetBit(vcpuins.result, vcpuins.opr2);
	_ce;
}

static void _a_imul2(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit)
{
	t_nubit64 temp;
	_cb("_a_imul2");
	switch (bit) {
	case 16: _bb("bit(16+16)");
		_newins_;
		vcpuins.bit = 16;
		vcpuins.opr1 = GetMax16((t_nsbit16)cdest);
		vcpuins.opr2 = GetMax16((t_nsbit16)csrc);
		temp = GetMax32((t_nsbit16)vcpuins.opr1 * (t_nsbit16)vcpuins.opr2);
		vcpuins.result = GetMax16(temp);
		if (GetMax32(temp) != GetMax32((t_nsbit16)vcpuins.result)) {
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
		vcpuins.opr1 = GetMax32((t_nsbit32)cdest);
		vcpuins.opr2 = GetMax32((t_nsbit32)csrc);
		temp = GetMax64((t_nsbit32)vcpuins.opr1 * (t_nsbit32)vcpuins.opr2);
		vcpuins.result = GetMax32(temp);
		if (GetMax64(temp) != GetMax64((t_nsbit32)vcpuins.result)) {
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
static void _a_shld(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 count, t_nubit8 bit)
{
	t_bool flagcf;
	t_bool flagbit;
	t_nsbit32 i;
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
			vcpuins.opr1 = GetMax16(cdest);
			vcpuins.opr2 = GetMax16(csrc);
			vcpuins.result = vcpuins.opr1;
			flagcf = !!GetMSB16(vcpuins.result);
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF,
				GetBit(vcpuins.result, GetMax64(1 << (bit - count))));
			for (i = (t_nsbit32)(bit - 1);i >= (t_nsbit32)count;--i) {
				flagbit = GetBit(vcpuins.opr1, GetMax64(1 << (i - count)));
				MakeBit(vcpuins.result, GetMax64(1 << i), flagbit);
			}
			for (i = (t_nsbit32)(count - 1);i >= 0;--i) {
				flagbit = GetBit(vcpuins.opr2, GetMax64(1 << (i - count + bit)));
				MakeBit(vcpuins.result, GetMax64(1 << i), flagbit);
			}
			if (count == 1)
				MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
					((!!GetMSB16(vcpuins.result)) ^ flagcf));
			else
				vcpuins.udf |= VCPU_EFLAGS_OF;
			_be;break;
		case 32: _bb("bit(32)");
			vcpuins.bit = 32;
			vcpuins.opr1 = GetMax32(cdest);
			vcpuins.opr2 = GetMax32(csrc);
			vcpuins.result = vcpuins.opr1;
			flagcf = !!GetMSB32(vcpuins.result);
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF,
				GetBit(vcpuins.result, GetMax64(1 << (bit - count))));
			for (i = (t_nsbit32)(bit - 1);i >= (t_nsbit32)count;--i) {
				flagbit = GetBit(vcpuins.opr1, GetMax64(1 << (i - count)));
				MakeBit(vcpuins.result, GetMax64(1 << i), flagbit);
			}
			for (i = (t_nsbit32)(count - 1);i >= 0;--i) {
				flagbit = GetBit(vcpuins.opr2, GetMax64(1 << (i - count + bit)));
				MakeBit(vcpuins.result, GetMax64(1 << i), flagbit);
			}
			if (count == 1)
				MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
					((!!GetMSB32(vcpuins.result)) ^ flagcf));
			else
				vcpuins.udf |= VCPU_EFLAGS_OF;
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
static void _a_shrd(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 count, t_nubit8 bit)
{
	t_bool flagcf;
	t_bool flagbit;
	t_nsbit32 i;
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
			vcpuins.opr1 = GetMax16(cdest);
			vcpuins.opr2 = GetMax16(csrc);
			vcpuins.result = vcpuins.opr1;
			flagcf = !!GetMSB16(vcpuins.result);
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF,
				GetBit(vcpuins.result, GetMax64(1 << (count - 1))));
			for (i = (t_nsbit32)0;i <= (t_nsbit32)(bit - count - 1);++i) {
				flagbit = GetBit(vcpuins.opr1, GetMax64(1 << (i + count)));
				MakeBit(vcpuins.result, GetMax64(1 << i), flagbit);
			}
			for (i = (t_nsbit32)(bit - count);i <= (t_nsbit32)(bit - 1);++i) {
				flagbit = GetBit(vcpuins.opr2, GetMax64(1 << (i + count - bit)));
				MakeBit(vcpuins.result, GetMax64(1 << i), flagbit);
			}
			if (count == 1)
				MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
					((!!GetMSB16(vcpuins.result)) ^ flagcf));
			else
				vcpuins.udf |= VCPU_EFLAGS_OF;
			_be;break;
		case 32: _bb("bit(32)");
			vcpuins.bit = 32;
			vcpuins.opr1 = GetMax32(cdest);
			vcpuins.opr2 = GetMax32(csrc);
			vcpuins.result = vcpuins.opr1;
			flagcf = !!GetMSB32(vcpuins.result);
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF,
				GetBit(vcpuins.result, GetMax64(1 << (count - 1))));
			for (i = (t_nsbit32)0;i <= (t_nsbit32)(bit - count - 1);++i) {
				flagbit = GetBit(vcpuins.opr1, GetMax64(1 << (i + count)));
				MakeBit(vcpuins.result, GetMax64(1 << i), flagbit);
			}
			for (i = (t_nsbit32)(bit - count);i <= (t_nsbit32)(bit - 1);++i) {
				flagbit = GetBit(vcpuins.opr2, GetMax64(1 << (i + count - bit)));
				MakeBit(vcpuins.result, GetMax64(1 << i), flagbit);
			}
			if (count == 1)
				MakeBit(vcpu.eflags, VCPU_EFLAGS_OF,
					((!!GetMSB32(vcpuins.result)) ^ flagcf));
			else
				vcpuins.udf |= VCPU_EFLAGS_OF;
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

static void _m_setcc_rm(t_bool condition)
{
	_cb("_m_setcc_rm");
	vcpuins.result = GetMax8(!!condition);
	vcpuins.crm = vcpuins.result;_chk(_m_write_rm(1));
	_ce;
}

static void INS_0F_00()
{
	t_nubit8 modrm;
	t_nubit64 descriptor;
	_cb("INS_0F_00");
	_adv;
	if (_IsProtected) {
		_bb("Protected");
		_chk(_s_read_cs(vcpu.eip, GetRef(modrm), 1));
		switch (_GetModRM_REG(modrm)) {
		case 0: /* SLDT_RM16 */
			_bb("SLDT_RM16");
			_chk(_d_modrm(0, ((_GetModRM_MOD(modrm) != 3) ? 2 : _GetOperandSize)));
			vcpuins.crm = GetMax16(vcpu.ldtr.selector);
			_chk(_m_write_rm(vcpuins.flagmem ? 2 : _GetOperandSize));
			_be;break;
		case 1: /* STR_RM16 */
			_bb("STR_RM16");
			_chk(_d_modrm(0, ((_GetModRM_MOD(modrm) != 3) ? 2 : _GetOperandSize)));
			vcpuins.crm = GetMax16(vcpu.tr.selector);
			_chk(_m_write_rm(vcpuins.flagmem ? 2 : _GetOperandSize));
			_be;break;
		case 2: /* LLDT_RM16 */
			_bb("LLDT_RM16");
			_chk(_d_modrm(0, 2));
			_chk(_m_read_rm(2));
			_chk(_s_load_ldtr(GetMax16(vcpuins.crm)));
			_be;break;
		case 3: /* LTR_RM16 */
			_bb("LTR_RM16");
			_chk(_d_modrm(0, 2));
			_chk(_m_read_rm(2));
			_chk(_s_load_tr(GetMax16(vcpuins.crm)));
			_be;break;
		case 4: /* VERR_RM16 */
			_bb("VERR_RM16");
			_chk(_d_modrm(0, 2));
			_chk(_m_read_rm(2));
			if (_s_check_selector(GetMax16(vcpuins.crm))) {
				_ClrEFLAGS_ZF;
			} else {
				_bb("selector(valid)");
				_chk(_s_read_xdt(GetMax16(vcpuins.crm), GetRef(descriptor)));
				if (_IsDescSys(descriptor) ||
						(!_IsDescCodeConform(descriptor) &&
							(_GetCPL > _GetDesc_DPL(descriptor) ||
							_GetSelector_RPL(GetMax16(vcpuins.crm)) > _GetDesc_DPL(descriptor)))) {
					_ClrEFLAGS_ZF;
				} else {
					if (_IsDescData(descriptor) || _IsDescCodeReadable(descriptor)) {
						_SetEFLAGS_ZF;
					} else {
						_ClrEFLAGS_ZF;
					}
				}
				_be;
			}
			_be;break;
		case 5: /* VERW_RM16 */
			_bb("VERW_RM16");
			_chk(_d_modrm(0, 2));
			_chk(_m_read_rm(2));
			if (_s_check_selector(GetMax16(vcpuins.crm))) {
				_ClrEFLAGS_ZF;
			} else {
				_bb("selector(valid)");
				_chk(_s_read_xdt(GetMax16(vcpuins.crm), GetRef(descriptor)));
				if (_IsDescSys(descriptor) ||
						(!_IsDescCodeConform(descriptor) &&
							(_GetCPL > _GetDesc_DPL(descriptor) ||
							_GetSelector_RPL(GetMax16(vcpuins.crm)) > _GetDesc_DPL(descriptor)))) {
					_ClrEFLAGS_ZF;
				} else {
					if (_IsDescDataWritable(descriptor)) {
						_SetEFLAGS_ZF;
					} else {
						_ClrEFLAGS_ZF;
					}
				}
				_be;
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
static void INS_0F_01()
{
	t_nubit8 modrm;
	t_nubit16 limit;
	t_nubit32 base;
	_cb("INS_0F_01");
	_adv;
	_chk(_s_read_cs(vcpu.eip, GetRef(modrm), 1));
	switch (_GetModRM_REG(modrm)) {
	case 0: /* SGDT_M32_16 */
		_bb("SGDT_M32_16");
		_chk(_d_modrm(0, 6));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		vcpuins.crm = vcpu.gdtr.limit;
		_chk(_m_write_rm(2));
		vcpuins.mrm.offset += 2;
		switch (_GetOperandSize) {
		case 2: vcpuins.crm = GetMax24(vcpu.gdtr.base);break;
		case 4: vcpuins.crm = GetMax32(vcpu.gdtr.base);break;
		default:_impossible_;break;}
		_chk(_m_write_rm(4));
		_be;break;
	case 1: /* SIDT_M32_16 */
		_bb("SIDT_M32_16");
		_chk(_d_modrm(0, 6));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		vcpuins.crm = vcpu.idtr.limit;
		_chk(_m_write_rm(2));
		vcpuins.mrm.offset += 2;
		switch (_GetOperandSize) {
		case 2: vcpuins.crm = GetMax24(vcpu.idtr.base);break;
		case 4: vcpuins.crm = GetMax32(vcpu.idtr.base);break;
		default:_impossible_;break;}
		_chk(_m_write_rm(4));
		_be;break;
	case 2: /* LGDT_M32_16 */
		_bb("LGDT_M32_16");
		_chk(_d_modrm(0, 6));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		_chk(_m_read_rm(2));
		limit = GetMax16(vcpuins.crm);
		vcpuins.mrm.offset += 2;
		_chk(_m_read_rm(4));
		switch (_GetOperandSize) {
		case 2: base = GetMax24(vcpuins.crm);break;
		case 4: base = GetMax32(vcpuins.crm);break;
		default:_impossible_;break;}
		_comment("LGDT_M32_16: executed at L%08X, read base=%08X, limit=%04X\n",
			vcpurec.linear, base, limit);
		_chk(_s_load_gdtr(base, limit, _GetOperandSize));
		_be;break;
	case 3: /* LIDT_M32_16 */
		_bb("LIDT_M32_16");
		_chk(_d_modrm(0, 6));
		if (!vcpuins.flagmem) {
			_bb("flagmem(0)");
			_chk(UndefinedOpcode());
			_be;
		}
		_chk(_m_read_rm(2));
		limit = GetMax16(vcpuins.crm);
		vcpuins.mrm.offset += 2;
		_chk(_m_read_rm(4));
		switch (_GetOperandSize) {
		case 2: base = GetMax24(vcpuins.crm);break;
		case 4: base = GetMax32(vcpuins.crm);break;
		default:_impossible_;break;}
		_comment("LIDT_M32_16: executed at L%08X, read base=%08X, limit=%04X\n",
			vcpurec.linear, base, limit);
		_chk(_s_load_idtr(base, limit, _GetOperandSize));
		_be;break;
	case 4: /* SMSW_RM16 */
		_bb("SMSW_RM16");
		_chk(_d_modrm(0, ((_GetModRM_MOD(modrm) == 3) ? _GetOperandSize : 2)));
		vcpuins.crm = GetMax16(vcpu.cr0);
		if (_GetOperandSize == 4 && !vcpuins.flagmem)
			_chk(_m_write_rm(4));
		else
			_chk(_m_write_rm(2));
		_be;break;
	case 5: _bb("ModRM_REG(5)");_chk(UndefinedOpcode());_be;break;
	case 6: /* LMSW_RM16 */
		_bb("LMSW_RM16");
		_chk(_d_modrm(0, 2));
		_chk(_m_read_rm(2));
		_chk(_s_load_cr0_msw(GetMax16(vcpuins.crm)));
		_be;break;
	case 7: _bb("ModRM_REG(7)");_chk(UndefinedOpcode());_be;break;
	default:_impossible_;break;}
	_ce;
}
static void LAR_R32_RM32()
{
	t_nubit16 selector;
	t_nubit64 descriptor;
	_cb("LAR_R32_RM32");
	_adv;
	if (_IsProtected) {
		_bb("Protected(1)");
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(2));
		selector = GetMax16(vcpuins.crm);
		if (_s_check_selector(selector)) {
			_ClrEFLAGS_ZF;
		} else {
			_bb("selector(valid)");
			_chk(_s_read_xdt(selector, GetRef(descriptor)));
			if (_IsDescUser(descriptor)) {
				if (_IsDescCodeConform(descriptor))
					_SetEFLAGS_ZF;
				else {
					if (_GetCPL > _GetDesc_DPL(descriptor) || _GetSelector_RPL(selector) > _GetDesc_DPL(descriptor))
						_ClrEFLAGS_ZF;
					else
						_SetEFLAGS_ZF;
				}
			} else {
				switch (_GetDesc_Type(descriptor)) {
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
			if (_GetEFLAGS_ZF) {
				_bb("EFLAGS_ZF");
				switch (_GetOperandSize) {
				case 2: _bb("OperandSize(2)");
					descriptor = (GetMax16(descriptor >> 32) & 0xff00);
					_chk(_m_write_ref(vcpuins.rr, GetRef(descriptor), 2));
					_be;break;
				case 4: _bb("OperandSize(4)");
					descriptor = (GetMax32(descriptor >> 32) & 0x00ffff00);
					_chk(_m_write_ref(vcpuins.rr, GetRef(descriptor), 4));
					_be;break;
				default:_impossible_;break;}
				_be;
			}
			_be;
		}
		_be;
	} else {
		_bb("Protected(0)");
		_chk(UndefinedOpcode());
		_be;
	}
	_ce;
}
static void LSL_R32_RM32()
{
	t_nubit16 selector;
	t_nubit32 limit;
	t_nubit64 descriptor;
	_cb("LSL_R32_RM32");
	_adv;
	if (_IsProtected) {
		_bb("Protected(1)");
		_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
		_chk(_m_read_rm(2));
		selector = GetMax16(vcpuins.crm);
		if (_s_check_selector(selector)) {
			_ClrEFLAGS_ZF;
		} else {
			_bb("selector(valid)");
			_chk(_s_read_xdt(selector, GetRef(descriptor)));
			if (_IsDescUser(descriptor)) {
				if (_IsDescCodeConform(descriptor))
					_SetEFLAGS_ZF;
				else {
					if (_GetCPL > _GetDesc_DPL(descriptor) || _GetSelector_RPL(selector) > _GetDesc_DPL(descriptor))
						_ClrEFLAGS_ZF;
					else
						_SetEFLAGS_ZF;
				}
			} else {
				switch (_GetDesc_Type(descriptor)) {
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
			if (_GetEFLAGS_ZF) {
				_bb("EFLAGS_ZF");
				limit = _IsDescSegGranularLarge(descriptor) ?
					((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) : _GetDescSeg_Limit(descriptor);
				switch (_GetOperandSize) {
				case 2: _bb("OperandSize(2)");
					_chk(_m_write_ref(vcpuins.rr, GetRef(limit), 2));
					_be;break;
				case 4: _bb("OperandSize(4)");
					_chk(_m_write_ref(vcpuins.rr, GetRef(limit), 4));
					_be;break;
				default:_impossible_;break;}
				_be;
			}
			_be;
		}
		_be;
	} else {
		_bb("Protected(0)");
		_chk(UndefinedOpcode());
		_be;
	}
	_ce;
}
static void CLTS()
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
_______todo WBINVD()
{
	_cb("WBINVD");
#ifndef VGLOBAL_BOCHS
	UndefinedOpcode();
#else
	_adv;
	vcpu.flagignore = 1;
#endif
	_ce;
}
static void MOV_R32_CR()
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
	_chk(_m_write_ref(vcpuins.rrm, GetRef(vcpuins.cr), 4));
	_ce;
}
static void MOV_R32_DR()
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
	_chk(_m_write_ref(vcpuins.rrm, GetRef(vcpuins.cr), 4));
	_ce;
}
static void MOV_CR_R32()
{
	_cb("MOV_CR_R32");
	_adv;
	if (_GetCPL) {
		_bb("CPL(!0)");
		_chk(_SetExcept_GP(0));
		_be;
	}
	_chk(_d_modrm_creg());
	_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.crm), 4));
	if (vcpuins.rr == (t_vaddrcc)&vcpu.cr0) _comment("MOV_CR_R32: executed at L%08X, CR0=%08X\n", vcpurec.linear, vcpu.cr0);
	if (vcpuins.rr == (t_vaddrcc)&vcpu.cr2) _comment("MOV_CR_R32: executed at L%08X, CR2=%08X\n", vcpurec.linear, vcpu.cr2);
	if (vcpuins.rr == (t_vaddrcc)&vcpu.cr3) _comment("MOV_CR_R32: executed at L%08X, CR3=%08X\n", vcpurec.linear, vcpu.cr3);
	_ce;
}
static void MOV_DR_R32()
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
	_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.crm), 4));
	_ce;
}
static void MOV_R32_TR()
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
	_chk(_m_write_ref(vcpuins.rrm, GetRef(vcpuins.cr), 4));
	_ce;
}
static void MOV_TR_R32()
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
	_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.crm), 4));
	_ce;
}
_______todo WRMSR()
{
	_cb("WRMSR");
#ifndef VGLOBAL_BOCHS
	UndefinedOpcode();
#else
	_adv;
	vcpu.flagignore = 1;
#endif
	_ce;
}
_______todo RDMSR()
{
	_cb("RDMSR");
#ifndef VGLOBAL_BOCHS
	UndefinedOpcode();
#else
	_adv;
	vcpu.flagignore = 1;
#endif
	_ce;
}
static void JO_REL32()
{
	_cb("JO_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, _GetEFLAGS_OF));
	_ce;
}
static void JNO_REL32()
{
	_cb("JNO_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, !_GetEFLAGS_OF));
	_ce;
}
static void JC_REL32()
{
	_cb("JC_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, _GetEFLAGS_CF));
	_ce;
}
static void JNC_REL32()
{
	_cb("JNC_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, !_GetEFLAGS_CF));
	_ce;
}
static void JZ_REL32()
{
	_cb("JZ_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, _GetEFLAGS_ZF));
	_ce;
}
static void JNZ_REL32()
{
	_cb("JNZ_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, !_GetEFLAGS_ZF));
	_ce;
}
static void JNA_REL32()
{
	_cb("JNA_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize,
		(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
	_ce;
}
static void JA_REL32()
{
	_cb("JA_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize,
		!(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
	_ce;
}
static void JS_REL32()
{
	_cb("JS_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, _GetEFLAGS_SF));
	_ce;
}
static void JNS_REL32()
{
	_cb("JNS_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, !_GetEFLAGS_SF));
	_ce;
}
static void JP_REL32()
{
	_cb("JP_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, _GetEFLAGS_PF));
	_ce;
}
static void JNP_REL32()
{
	_cb("JNP_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, !_GetEFLAGS_PF));
	_ce;
}
static void JL_REL32()
{
	_cb("JL_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, (_GetEFLAGS_SF != _GetEFLAGS_OF)));
	_ce;
}
static void JNL_REL32()
{
	_cb("JNL_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize, (_GetEFLAGS_SF == _GetEFLAGS_OF)));
	_ce;
}
static void JNG_REL32()
{
	_cb("JNG_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize,
			(_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF))));
	_ce;
}
static void JG_REL32()
{
	_cb("JG_REL32");
	_newins_;
	_adv;
	_chk(_d_imm(_GetOperandSize));
	_chk(_e_jcc(GetMax32(vcpuins.cimm), _GetOperandSize,
		(!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF))));
	_ce;
}
static void SETO_RM8()
{
	_cb("SETO_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(_GetEFLAGS_OF));
	_ce;
}
static void SETNO_RM8()
{
	_cb("SETO_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(!_GetEFLAGS_OF));
	_ce;
}
static void SETC_RM8()
{
	_cb("SETC_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(_GetEFLAGS_CF));
	_ce;
}
static void SETNC_RM8()
{
	_cb("SETNC_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(!_GetEFLAGS_CF));
	_ce;
}
static void SETZ_RM8()
{
	_cb("SETZ_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(_GetEFLAGS_ZF));
	_ce;
}
static void SETNZ_RM8()
{
	_cb("SETNZ_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(!_GetEFLAGS_ZF));
	_ce;
}
static void SETNA_RM8()
{
	_cb("SETNA_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(_GetEFLAGS_CF || _GetEFLAGS_ZF));
	_ce;
}
static void SETA_RM8()
{
	_cb("SETA_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(!(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
	_ce;
}
static void SETS_RM8()
{
	_cb("SETS_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(_GetEFLAGS_SF));
	_ce;
}
static void SETNS_RM8()
{
	_cb("SETNS_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(!_GetEFLAGS_SF));
	_ce;
}
static void SETP_RM8()
{
	_cb("SETP_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(_GetEFLAGS_PF));
	_ce;
}
static void SETNP_RM8()
{
	_cb("SETNP_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(!_GetEFLAGS_PF));
	_ce;
}
static void SETL_RM8()
{
	_cb("SETL_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(_GetEFLAGS_SF != _GetEFLAGS_OF));
	_ce;
}
static void SETNL_RM8()
{
	_cb("SETNL_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(_GetEFLAGS_SF == _GetEFLAGS_OF));
	_ce;
}
static void SETNG_RM8()
{
	_cb("SETNG_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF)));
	_ce;
}
static void SETG_RM8()
{
	_cb("SETG_RM8");
	_newins_;
	_adv;
	_chk(_d_modrm(0, 1));
	_chk(_m_setcc_rm(!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF)));
	_ce;
}
static void PUSH_FS()
{
	_cb("PUSH_FS");
	_newins_;
	_adv;
	_chk(_e_push(vcpu.fs.selector, _GetOperandSize));
	_ce;
}
static void POP_FS()
{
	t_nubit16 sel;
	_cb("POP_FS");
	_newins_;
	_adv;
	_chk(sel = GetMax16(_e_pop(_GetOperandSize)));
	_chk(_s_load_fs(sel));
	_ce;
}
_______todo CPUID()
{
	_cb("CPUID");
#ifndef VGLOBAL_BOCHS
	UndefinedOpcode();
#else
	_adv;
	vcpu.flagignore = 1;
#endif
	_ce;
}
static void BT_RM32_R32()
{
	_cb("BT_RM32_R32");
	_adv;
	_chk(_d_bit_rmimm(_GetOperandSize, _GetOperandSize, 0));
	_chk(_m_read_rm(_GetOperandSize));
	_chk(_a_bt((t_nubit32)vcpuins.crm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
	_ce;
}
static void SHLD_RM32_R32_I8()
{
	_cb("SHLD_RM32_R32_I8");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(_m_read_rm(_GetOperandSize));
	_chk(_d_imm(1));
	_chk(_a_shld(vcpuins.crm, vcpuins.cr, GetMax8(vcpuins.cimm), _GetOperandSize * 8));
	vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
	_ce;
}
static void SHLD_RM32_R32_CL()
{
	_cb("SHLD_RM32_R32_CL");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(_m_read_rm(_GetOperandSize));
	_chk(_a_shld(vcpuins.crm, vcpuins.cr, vcpu.cl, _GetOperandSize * 8));
	vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
	_ce;
}
static void PUSH_GS()
{
	_cb("PUSH_GS");
	_newins_;
	_adv;
	_chk(_e_push(vcpu.gs.selector, _GetOperandSize));
	_ce;
}
static void POP_GS()
{
	t_nubit16 sel;
	_cb("POP_GS");
	_newins_;
	_adv;
	_chk(sel = GetMax16(_e_pop(_GetOperandSize)));
	_chk(_s_load_gs(sel));
	_ce;
}
_______todo RSM()
{
	_cb("RSM");
#ifndef VGLOBAL_BOCHS
	UndefinedOpcode();
#else
	_adv;
	vcpu.flagignore = 1;
#endif
	_ce;
}
static void BTS_RM32_R32()
{
	_cb("BTS_RM32_R32");
	_adv;
	_chk(_d_bit_rmimm(_GetOperandSize, _GetOperandSize, 1));
	_chk(_m_read_rm(_GetOperandSize));
	_chk(_a_bts((t_nubit32)vcpuins.crm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
	vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
	_ce;
}
static void SHRD_RM32_R32_I8()
{
	_cb("SHRD_RM32_R32_I8");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(_m_read_rm(_GetOperandSize));
	_chk(_d_imm(1));
	_chk(_a_shrd(vcpuins.crm, vcpuins.cr, GetMax8(vcpuins.cimm), _GetOperandSize * 8));
	vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
	_ce;
}
static void SHRD_RM32_R32_CL()
{
	_cb("SHRD_RM32_R32_CL");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(_m_read_rm(_GetOperandSize));
	_chk(_a_shrd(vcpuins.crm, vcpuins.cr, vcpu.cl, _GetOperandSize * 8));
	vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
	_ce;
}
static void IMUL_R32_RM32()
{
	_cb("IMUL_R32_RM32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(_m_read_rm(_GetOperandSize));
	_chk(_a_imul2(vcpuins.cr, vcpuins.crm, _GetOperandSize * 8));
	_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), _GetOperandSize));
	_ce;
}
static void LSS_R32_M16_32()
{
	t_nubit16 selector;
	t_nubit32 offset;
	_cb("LSS_R32_M16_32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
	if (!vcpuins.flagmem) {
		_bb("flagmem(0)");
		_chk(UndefinedOpcode());
		_be;
	}
	_chk(_m_read_rm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: offset = GetMax16(vcpuins.crm);break;
	case 4: offset = GetMax32(vcpuins.crm);break;
	default:_impossible_;break;}
	vcpuins.mrm.offset += _GetOperandSize;
	_chk(_m_read_rm(2));
	selector = GetMax16(vcpuins.crm);
	_chk(_e_load_far(&vcpu.ss, vcpuins.rr, selector, offset, _GetOperandSize));
	_ce;
}
static void BTR_RM32_R32()
{
	_cb("BTR_RM32_R32");
	_adv;
	_chk(_d_bit_rmimm(_GetOperandSize, _GetOperandSize, 1));
	_chk(_m_read_rm(_GetOperandSize));
	_chk(_a_btr((t_nubit32)vcpuins.crm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
	vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
	_ce;
}
static void LFS_R32_M16_32()
{
	t_nubit16 selector;
	t_nubit32 offset;
	_cb("LFS_R32_M16_32");
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
	if (!vcpuins.flagmem) {
		_bb("flagmem(0)");
		_chk(UndefinedOpcode());
		_be;
	}
	_chk(_m_read_rm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: offset = GetMax16(vcpuins.crm);break;
	case 4: offset = GetMax32(vcpuins.crm);break;
	default:_impossible_;break;}
	vcpuins.mrm.offset += _GetOperandSize;
	_chk(_m_read_rm(2));
	selector = GetMax16(vcpuins.crm);
	_chk(_e_load_far(&vcpu.fs, vcpuins.rr, selector, offset, _GetOperandSize));
	_ce;
}
static void LGS_R32_M16_32()
{
	t_nubit16 selector;
	t_nubit32 offset;
	_cb("LGS_R32_M16_32");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
	if (!vcpuins.flagmem) {
		_bb("flagmem(0)");
		_chk(UndefinedOpcode());
		_be;
	}
	_chk(_m_read_rm(_GetOperandSize));
	switch (_GetOperandSize) {
	case 2: offset = GetMax16(vcpuins.crm);break;
	case 4: offset = GetMax32(vcpuins.crm);break;
	default:_impossible_;break;}
	vcpuins.mrm.offset += _GetOperandSize;
	_chk(_m_read_rm(2));
	selector = GetMax16(vcpuins.crm);
	_chk(_e_load_far(&vcpu.gs, vcpuins.rr, selector, offset, _GetOperandSize));
	_ce;
}
static void MOVZX_R32_RM8()
{
	_cb("MOVZX_R32_RM8");
	_adv;
	_chk(_d_modrm(_GetOperandSize, 1));
	_chk(_m_read_rm(1));
	vcpuins.crm = (t_nubit8)vcpuins.crm;
	_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.crm), _GetOperandSize));
	_ce;
}
static void MOVZX_R32_RM16()
{
	_cb("MOVZX_R32_RM16");
	_adv;
	_chk(_d_modrm(4, 2));
	_chk(_m_read_rm(2));
	vcpuins.crm = (t_nubit16)vcpuins.crm;
	_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.crm), 4));
	_ce;
}
static void INS_0F_BA()
{
	t_bool write = 0;
	t_nubit8 modrm = 0x00;
	_cb("INS_0F_BA");
	_newins_;
	_adv;
	_chk(_s_read_cs(vcpu.eip, GetRef(modrm), 1));
	if (_GetModRM_REG(modrm) == 4) write = 0;
	else write = 1;
	_chk(_d_bit_rmimm(0, _GetOperandSize, write));
	_chk(_m_read_rm(_GetOperandSize));
	switch (vcpuins.cr) {
	case 0: _bb("vcpuins.cr(0)");_chk(UndefinedOpcode());_be;break;
	case 1: _bb("vcpuins.cr(1)");_chk(UndefinedOpcode());_be;break;
	case 2: _bb("vcpuins.cr(2)");_chk(UndefinedOpcode());_be;break;
	case 3: _bb("vcpuins.cr(3)");_chk(UndefinedOpcode());_be;break;
	case 4: /* BT_RM32_I8 */
		_bb("BT_RM32_I8");
		_chk(_a_bt((t_nubit32)vcpuins.crm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
		_be;break;
	case 5: /* BTS_RM32_I8 */
		_bb("BTS_RM32_I8");
		_chk(_a_bts((t_nubit32)vcpuins.crm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
		_be;break;
	case 6: /* BTR_RM32_I8 */
		_bb("BTR_RM32_I8");
		_chk(_a_btr((t_nubit32)vcpuins.crm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
		_be;break;
	case 7: /* BTC_RM32_I8 */
		_bb("BTC_RM32_I8");
		_chk(_a_btc((t_nubit32)vcpuins.crm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
		vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
		_be;break;
	default:_impossible_;break;}
	_ce;
}
static void BTC_RM32_R32()
{
	_cb("BTC_RM32_R32");
	_newins_;
	_adv;
	_chk(_d_bit_rmimm(_GetOperandSize, _GetOperandSize, 1));
	_chk(_m_read_rm(_GetOperandSize));
	_chk(_a_btc((t_nubit32)vcpuins.crm, (t_nubit32)vcpuins.cimm, _GetOperandSize * 8));
	vcpuins.crm = vcpuins.result;_chk(_m_write_rm(_GetOperandSize));
	_ce;
}
static void BSF_R32_RM32()
{
	_cb("BSF_R32_RM32");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(_m_read_rm(_GetOperandSize));
	_chk(_a_bscc(vcpuins.crm, _GetOperandSize * 8, 1));
	if (!_GetEFLAGS_ZF) {
		_bb("EFLAGS_ZF(0)");
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), _GetOperandSize));
		_be;
	}
	_ce;
}
static void BSR_R32_RM32()
{
	_cb("BSR_R32_RM32");
	_newins_;
	_adv;
	_chk(_d_modrm(_GetOperandSize, _GetOperandSize));
	_chk(_m_read_rm(_GetOperandSize));
	_chk(_a_bscc(vcpuins.crm, _GetOperandSize * 8, 0));
	if (!_GetEFLAGS_ZF) {
		_bb("EFLAGS_ZF(0)");
		_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.result), _GetOperandSize));
		_be;
	}
	_ce;
}
static void MOVSX_R32_RM8()
{
	_cb("MOVSX_R32_RM8");
	_adv;
	_chk(_d_modrm(_GetOperandSize, 1));
	_chk(_m_read_rm(1));
	vcpuins.crm = (t_nsbit8)vcpuins.crm;
	_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.crm), _GetOperandSize));
	_ce;
}
static void MOVSX_R32_RM16()
{
	_cb("MOVSX_R32_RM16");
	_adv;
	_chk(_d_modrm(4, 2));
	_chk(_m_read_rm(2));
	vcpuins.crm = (t_nsbit16)vcpuins.crm;
	_chk(_m_write_ref(vcpuins.rr, GetRef(vcpuins.crm), 4));
	_ce;
}


static void RecInit()
{
	vcpurec.msize = 0;
	vcpurec.rcpu = vcpu;
	vcpurec.linear = vcpu.cs.base + vcpu.eip;
	if (vcpuinsReadLinear(vcpurec.linear, (t_vaddrcc)vcpurec.opcodes, 15))
		vcpurec.oplen = 0;
	else
		vcpurec.oplen = 15;
}
static void RecFinal()
{}
static void ExecInit()
{
	vcpu.flagignore = 0;
	vcpu.flaglock = 0;
	vcpuins.oldcpu = vcpu;
	vcpuins.roverds = &vcpu.ds;
	vcpuins.roverss = &vcpu.ss;
	vcpuins.prefix_rep = PREFIX_REP_NONE;
	vcpuins.prefix_oprsize = 0x00;
	vcpuins.prefix_addrsize = 0x00;
	vcpuins.flagmem = 0;
	vcpuins.flaginsloop = 0;
	vcpuins.flagmaskint = 0;
	vcpuins.bit = 0;
	vcpuins.opr1 = 0;
	vcpuins.opr2 = 0;
	vcpuins.result = 0;
	vcpuins.udf = 0x00000000;
	vcpuins.mrm.rsreg = NULL;
	vcpuins.mrm.offset = 0x00000000;
	vcpuins.except = 0x00000000;
	vcpuins.excode = 0x00000000;
#if DASM_TRACE == 1
	vapiTraceInit(&trace);
#endif
}
static void ExecFinal()
{
	if (vcpuins.flaginsloop) {
		vcpu.cs = vcpuins.oldcpu.cs;
		vcpu.eip = vcpuins.oldcpu.eip;
	}
#if DASM_TRACE == 1
	if (trace.cid && !vcpuins.except) _SetExcept_CE(trace.cid); 
	vapiTraceFinal(&trace);
#endif
	if (vcpuins.except) {
		vcpu = vcpuins.oldcpu;
		if (GetBit(vcpuins.except, VCPUINS_EXCEPT_GP)) {
			ExecInit();
			ClrBit(vcpuins.except, VCPUINS_EXCEPT_GP);
			_e_except_n(0x0d, _GetOperandSize);
		}
		//if (!flagignore)
			vapiCallBackMachineStop();
	}
}
static void ExecIns()
{
	t_nubit8 opcode = 0;
	t_nubit32 linear;
	RecInit();
	ExecInit();
	linear = vcpu.cs.base + vcpu.eip;
	if (vcpuins.flagwe && vcpuins.welin == linear) {
		vapiPrint("Watch point caught at L%08x: EXECUTED\n", linear);
		vapiCallBackDebugPrintRegs(1);
	}
	do {
		_cb("ExecIns");
		_chb(_s_read_cs(vcpu.eip, GetRef(opcode), 1));
		_chb(ExecFun(table[opcode]));
		_chb(_s_test_eip());
		_chb(_s_test_esp());
		_ce;
	} while (_kdf_check_prefix(opcode));
	ExecFinal();
	RecFinal();
} 
static void ExecInt()
{
#ifndef VGLOBAL_BOCHS
	t_nubit8 intr = 0x00;
	/* hardware interrupt handeler */
	if (vcpuins.flagmaskint)
		return;
	if(!vcpu.flagmasknmi && vcpu.flagnmi) {
		vcpu.flaghalt = 0;
		vcpu.flagnmi = 0;
		ExecInit();
		_e_intr_n(0x02, _GetOperandSize);
		ExecFinal();
	}
	if (_GetEFLAGS_IF && vpicScanINTR()) {
		vcpu.flaghalt = 0;
		intr = vpicGetINTR();
		/*printf("vint = %x\n", intr);*/
		ExecInit();
		_e_intr_n(intr, _GetOperandSize);
		ExecFinal();
	}
	if (_GetEFLAGS_TF) {
		vcpu.flaghalt = 0;
		ExecInit();
		_e_intr_n(0x01, _GetOperandSize);
		ExecFinal();
	}
#endif
}

static void QDX()
{
#ifndef VGLOBAL_BOCHS
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
	case 0x02:
		vcpurec.svcextl++;
		break;
	case 0x03:
		vcpurec.svcextl--;
		break;
	default: /* QDINT */
		_bb("QDINT");
		qdbiosExecInt(GetMax8(vcpuins.cimm));
		if (vcpuins.cimm < 0x80) {
			_chk(_s_read_ss(vcpu.sp + 4, GetRef(eflags), 4));
			MakeBit(eflags, VCPU_EFLAGS_ZF, _GetEFLAGS_ZF);
			MakeBit(eflags, VCPU_EFLAGS_CF, _GetEFLAGS_CF);
			_chk(_s_write_ss(vcpu.sp + 4, GetRef(eflags), 4));
		}
		_be;break;
	}
	_ce;
#endif
}

/* external interface */
t_bool vcpuinsLoadSreg(t_cpu_sreg *rsreg, t_nubit16 selector)
{
	t_bool fail;
	t_nubit32 oldexcept = vcpuins.except;
	vcpuins.except = 0;
	_ksa_load_sreg(rsreg, selector);
	fail = !!vcpuins.except;
	vcpuins.except = oldexcept;
	return fail;
}
t_bool vcpuinsReadLinear(t_nubit32 linear, t_vaddrcc rdata, t_nubit8 byte)
{
	t_bool fail;
	t_nubit32 oldexcept = vcpuins.except;
	vcpuins.except = 0;
	_kma_read_linear(linear, rdata, byte, 0x00, 1);
	fail = !!vcpuins.except;
	vcpuins.except = oldexcept;
	return fail;
}
t_bool vcpuinsWriteLinear(t_nubit32 linear, t_vaddrcc rdata, t_nubit8 byte)
{
	t_bool fail;
	t_nubit32 oldexcept = vcpuins.except;
	vcpuins.except = 0;
	_kma_write_linear(linear, rdata, byte, 0x00, 1);
	fail = !!vcpuins.except;
	vcpuins.except = oldexcept;
	return fail;
}

void vcpuinsInit()
{
#if VCPUINS_TRACE == 1
	vapiTraceInit(&trace);
#endif
	memset(&vcpuins, 0x00, sizeof(t_cpuins));
	memset(&vcpurec, 0x00, sizeof(t_cpurec));
	table[0x00] = (t_faddrcc)ADD_RM8_R8;
	table[0x01] = (t_faddrcc)ADD_RM32_R32;
	table[0x02] = (t_faddrcc)ADD_R8_RM8;
	table[0x03] = (t_faddrcc)ADD_R32_RM32;
	table[0x04] = (t_faddrcc)ADD_AL_I8;
	table[0x05] = (t_faddrcc)ADD_EAX_I32;
	table[0x06] = (t_faddrcc)PUSH_ES;
	table[0x07] = (t_faddrcc)POP_ES;
	table[0x08] = (t_faddrcc)OR_RM8_R8;
	table[0x09] = (t_faddrcc)OR_RM32_R32;
	table[0x0a] = (t_faddrcc)OR_R8_RM8;
	table[0x0b] = (t_faddrcc)OR_R32_RM32;
	table[0x0c] = (t_faddrcc)OR_AL_I8;
	table[0x0d] = (t_faddrcc)OR_EAX_I32;
	table[0x0e] = (t_faddrcc)PUSH_CS;
	table[0x0f] = (t_faddrcc)INS_0F;//
	table[0x10] = (t_faddrcc)ADC_RM8_R8;
	table[0x11] = (t_faddrcc)ADC_RM32_R32;
	table[0x12] = (t_faddrcc)ADC_R8_RM8;
	table[0x13] = (t_faddrcc)ADC_R32_RM32;
	table[0x14] = (t_faddrcc)ADC_AL_I8;
	table[0x15] = (t_faddrcc)ADC_EAX_I32;
	table[0x16] = (t_faddrcc)PUSH_SS;
	table[0x17] = (t_faddrcc)POP_SS;
	table[0x18] = (t_faddrcc)SBB_RM8_R8;
	table[0x19] = (t_faddrcc)SBB_RM32_R32;
	table[0x1a] = (t_faddrcc)SBB_R8_RM8;
	table[0x1b] = (t_faddrcc)SBB_R32_RM32;
	table[0x1c] = (t_faddrcc)SBB_AL_I8;
	table[0x1d] = (t_faddrcc)SBB_EAX_I32;
	table[0x1e] = (t_faddrcc)PUSH_DS;
	table[0x1f] = (t_faddrcc)POP_DS;
	table[0x20] = (t_faddrcc)AND_RM8_R8;
	table[0x21] = (t_faddrcc)AND_RM32_R32;
	table[0x22] = (t_faddrcc)AND_R8_RM8;
	table[0x23] = (t_faddrcc)AND_R32_RM32;
	table[0x24] = (t_faddrcc)AND_AL_I8;
	table[0x25] = (t_faddrcc)AND_EAX_I32;
	table[0x26] = (t_faddrcc)PREFIX_ES;
	table[0x27] = (t_faddrcc)DAA;
	table[0x28] = (t_faddrcc)SUB_RM8_R8;
	table[0x29] = (t_faddrcc)SUB_RM32_R32;
	table[0x2a] = (t_faddrcc)SUB_R8_RM8;
	table[0x2b] = (t_faddrcc)SUB_R32_RM32;
	table[0x2c] = (t_faddrcc)SUB_AL_I8;
	table[0x2d] = (t_faddrcc)SUB_EAX_I32;
	table[0x2e] = (t_faddrcc)PREFIX_CS;
	table[0x2f] = (t_faddrcc)DAS;
	table[0x30] = (t_faddrcc)XOR_RM8_R8;
	table[0x31] = (t_faddrcc)XOR_RM32_R32;
	table[0x32] = (t_faddrcc)XOR_R8_RM8;
	table[0x33] = (t_faddrcc)XOR_R32_RM32;
	table[0x34] = (t_faddrcc)XOR_AL_I8;
	table[0x35] = (t_faddrcc)XOR_EAX_I32;
	table[0x36] = (t_faddrcc)PREFIX_SS;
	table[0x37] = (t_faddrcc)AAA;
	table[0x38] = (t_faddrcc)CMP_RM8_R8;
	table[0x39] = (t_faddrcc)CMP_RM32_R32;
	table[0x3a] = (t_faddrcc)CMP_R8_RM8;
	table[0x3b] = (t_faddrcc)CMP_R32_RM32;
	table[0x3c] = (t_faddrcc)CMP_AL_I8;
	table[0x3d] = (t_faddrcc)CMP_EAX_I32;
	table[0x3e] = (t_faddrcc)PREFIX_DS;
	table[0x3f] = (t_faddrcc)AAS;
	table[0x40] = (t_faddrcc)INC_EAX;
	table[0x41] = (t_faddrcc)INC_ECX;
	table[0x42] = (t_faddrcc)INC_EDX;
	table[0x43] = (t_faddrcc)INC_EBX;
	table[0x44] = (t_faddrcc)INC_ESP;
	table[0x45] = (t_faddrcc)INC_EBP;
	table[0x46] = (t_faddrcc)INC_ESI;
	table[0x47] = (t_faddrcc)INC_EDI;
	table[0x48] = (t_faddrcc)DEC_EAX;
	table[0x49] = (t_faddrcc)DEC_ECX;
	table[0x4a] = (t_faddrcc)DEC_EDX;
	table[0x4b] = (t_faddrcc)DEC_EBX;
	table[0x4c] = (t_faddrcc)DEC_ESP;
	table[0x4d] = (t_faddrcc)DEC_EBP;
	table[0x4e] = (t_faddrcc)DEC_ESI;
	table[0x4f] = (t_faddrcc)DEC_EDI;
	table[0x50] = (t_faddrcc)PUSH_EAX;
	table[0x51] = (t_faddrcc)PUSH_ECX;
	table[0x52] = (t_faddrcc)PUSH_EDX;
	table[0x53] = (t_faddrcc)PUSH_EBX;
	table[0x54] = (t_faddrcc)PUSH_ESP;
	table[0x55] = (t_faddrcc)PUSH_EBP;
	table[0x56] = (t_faddrcc)PUSH_ESI;
	table[0x57] = (t_faddrcc)PUSH_EDI;
	table[0x58] = (t_faddrcc)POP_EAX;
	table[0x59] = (t_faddrcc)POP_ECX;
	table[0x5a] = (t_faddrcc)POP_EDX;
	table[0x5b] = (t_faddrcc)POP_EBX;
	table[0x5c] = (t_faddrcc)POP_ESP;
	table[0x5d] = (t_faddrcc)POP_EBP;
	table[0x5e] = (t_faddrcc)POP_ESI;
	table[0x5f] = (t_faddrcc)POP_EDI;
	table[0x60] = (t_faddrcc)PUSHA;//
	table[0x61] = (t_faddrcc)POPA;//
	table[0x62] = (t_faddrcc)BOUND_R16_M16_16;//
	table[0x63] = (t_faddrcc)ARPL_RM16_R16;//
	table[0x64] = (t_faddrcc)PREFIX_FS;//
	table[0x65] = (t_faddrcc)PREFIX_GS;//
	table[0x66] = (t_faddrcc)PREFIX_OprSize;//
	table[0x67] = (t_faddrcc)PREFIX_AddrSize;//
	table[0x68] = (t_faddrcc)PUSH_I32;//
	table[0x69] = (t_faddrcc)IMUL_R32_RM32_I32;//
	table[0x6a] = (t_faddrcc)PUSH_I8;//
	table[0x6b] = (t_faddrcc)IMUL_R32_RM32_I8;//
	table[0x6c] = (t_faddrcc)INSB;//
	table[0x6d] = (t_faddrcc)INSW;//
	table[0x6e] = (t_faddrcc)OUTSB;//
	table[0x6f] = (t_faddrcc)OUTSW;//
	table[0x70] = (t_faddrcc)JO_REL8;
	table[0x71] = (t_faddrcc)JNO_REL8;
	table[0x72] = (t_faddrcc)JC_REL8;
	table[0x73] = (t_faddrcc)JNC_REL8;
	table[0x74] = (t_faddrcc)JZ_REL8;
	table[0x75] = (t_faddrcc)JNZ_REL8;
	table[0x76] = (t_faddrcc)JNA_REL8;
	table[0x77] = (t_faddrcc)JA_REL8;
	table[0x78] = (t_faddrcc)JS_REL8;
	table[0x79] = (t_faddrcc)JNS_REL8;
	table[0x7a] = (t_faddrcc)JP_REL8;
	table[0x7b] = (t_faddrcc)JNP_REL8;
	table[0x7c] = (t_faddrcc)JL_REL8;
	table[0x7d] = (t_faddrcc)JNL_REL8;
	table[0x7e] = (t_faddrcc)JNG_REL8;
	table[0x7f] = (t_faddrcc)JG_REL8;
	table[0x80] = (t_faddrcc)INS_80;
	table[0x81] = (t_faddrcc)INS_81;
	table[0x82] = (t_faddrcc)UndefinedOpcode;
	table[0x83] = (t_faddrcc)INS_83;
	table[0x84] = (t_faddrcc)TEST_RM8_R8;
	table[0x85] = (t_faddrcc)TEST_RM32_R32;
	table[0x86] = (t_faddrcc)XCHG_RM8_R8;
	table[0x87] = (t_faddrcc)XCHG_RM32_R32;
	table[0x88] = (t_faddrcc)MOV_RM8_R8;
	table[0x89] = (t_faddrcc)MOV_RM32_R32;
	table[0x8a] = (t_faddrcc)MOV_R8_RM8;
	table[0x8b] = (t_faddrcc)MOV_R32_RM32;
	table[0x8c] = (t_faddrcc)MOV_RM16_SREG;
	table[0x8d] = (t_faddrcc)LEA_R32_M32;
	table[0x8e] = (t_faddrcc)MOV_SREG_RM16;
	table[0x8f] = (t_faddrcc)INS_8F;
	table[0x90] = (t_faddrcc)NOP;
	table[0x91] = (t_faddrcc)XCHG_ECX_EAX;
	table[0x92] = (t_faddrcc)XCHG_EDX_EAX;
	table[0x93] = (t_faddrcc)XCHG_EBX_EAX;
	table[0x94] = (t_faddrcc)XCHG_ESP_EAX;
	table[0x95] = (t_faddrcc)XCHG_EBP_EAX;
	table[0x96] = (t_faddrcc)XCHG_ESI_EAX;
	table[0x97] = (t_faddrcc)XCHG_EDI_EAX;
	table[0x98] = (t_faddrcc)CBW;
	table[0x99] = (t_faddrcc)CWD;
	table[0x9a] = (t_faddrcc)CALL_PTR16_32;
	table[0x9b] = (t_faddrcc)WAIT;
	table[0x9c] = (t_faddrcc)PUSHF;
	table[0x9d] = (t_faddrcc)POPF;
	table[0x9e] = (t_faddrcc)SAHF;
	table[0x9f] = (t_faddrcc)LAHF;
	table[0xa0] = (t_faddrcc)MOV_AL_MOFFS8;
	table[0xa1] = (t_faddrcc)MOV_EAX_MOFFS32;
	table[0xa2] = (t_faddrcc)MOV_MOFFS8_AL;
	table[0xa3] = (t_faddrcc)MOV_MOFFS32_EAX;
	table[0xa4] = (t_faddrcc)MOVSB;
	table[0xa5] = (t_faddrcc)MOVSW;
	table[0xa6] = (t_faddrcc)CMPSB;
	table[0xa7] = (t_faddrcc)CMPSW;
	table[0xa8] = (t_faddrcc)TEST_AL_I8;
	table[0xa9] = (t_faddrcc)TEST_EAX_I32;
	table[0xaa] = (t_faddrcc)STOSB;
	table[0xab] = (t_faddrcc)STOSW;
	table[0xac] = (t_faddrcc)LODSB;
	table[0xad] = (t_faddrcc)LODSW;
	table[0xae] = (t_faddrcc)SCASB;
	table[0xaf] = (t_faddrcc)SCASW;
	table[0xb0] = (t_faddrcc)MOV_AL_I8;
	table[0xb1] = (t_faddrcc)MOV_CL_I8;
	table[0xb2] = (t_faddrcc)MOV_DL_I8;
	table[0xb3] = (t_faddrcc)MOV_BL_I8;
	table[0xb4] = (t_faddrcc)MOV_AH_I8;
	table[0xb5] = (t_faddrcc)MOV_CH_I8;
	table[0xb6] = (t_faddrcc)MOV_DH_I8;
	table[0xb7] = (t_faddrcc)MOV_BH_I8;
	table[0xb8] = (t_faddrcc)MOV_EAX_I32;
	table[0xb9] = (t_faddrcc)MOV_ECX_I32;
	table[0xba] = (t_faddrcc)MOV_EDX_I32;
	table[0xbb] = (t_faddrcc)MOV_EBX_I32;
	table[0xbc] = (t_faddrcc)MOV_ESP_I32;
	table[0xbd] = (t_faddrcc)MOV_EBP_I32;
	table[0xbe] = (t_faddrcc)MOV_ESI_I32;
	table[0xbf] = (t_faddrcc)MOV_EDI_I32;
	table[0xc0] = (t_faddrcc)INS_C0;//
	table[0xc1] = (t_faddrcc)INS_C1;//
	table[0xc2] = (t_faddrcc)RET_I16;
	table[0xc3] = (t_faddrcc)RET;
	table[0xc4] = (t_faddrcc)LES_R32_M16_32;
	table[0xc5] = (t_faddrcc)LDS_R32_M16_32;
	table[0xc6] = (t_faddrcc)INS_C6;
	table[0xc7] = (t_faddrcc)INS_C7;
	table[0xc8] = (t_faddrcc)ENTER;//
	table[0xc9] = (t_faddrcc)LEAVE;//
	table[0xca] = (t_faddrcc)RETF_I16;
	table[0xcb] = (t_faddrcc)RETF;
	table[0xcc] = (t_faddrcc)INT3;
	table[0xcd] = (t_faddrcc)INT_I8;
	table[0xce] = (t_faddrcc)INTO;
	table[0xcf] = (t_faddrcc)IRET;
	table[0xd0] = (t_faddrcc)INS_D0;
	table[0xd1] = (t_faddrcc)INS_D1;
	table[0xd2] = (t_faddrcc)INS_D2;
	table[0xd3] = (t_faddrcc)INS_D3;
	table[0xd4] = (t_faddrcc)AAM;
	table[0xd5] = (t_faddrcc)AAD;
	table[0xd6] = (t_faddrcc)UndefinedOpcode;
	table[0xd7] = (t_faddrcc)XLAT;
	table[0xd8] = (t_faddrcc)UndefinedOpcode;
	table[0xd9] = (t_faddrcc)UndefinedOpcode;
	//table[0xd9] = (t_faddrcc)INS_D9;
	table[0xda] = (t_faddrcc)UndefinedOpcode;
	table[0xdb] = (t_faddrcc)UndefinedOpcode;
	//table[0xdb] = (t_faddrcc)INS_DB;
	table[0xdc] = (t_faddrcc)UndefinedOpcode;
	table[0xdd] = (t_faddrcc)UndefinedOpcode;
	table[0xde] = (t_faddrcc)UndefinedOpcode;
	table[0xdf] = (t_faddrcc)UndefinedOpcode;
	table[0xe0] = (t_faddrcc)LOOPNZ_REL8;
	table[0xe1] = (t_faddrcc)LOOPZ_REL8;
	table[0xe2] = (t_faddrcc)LOOP_REL8;
	table[0xe3] = (t_faddrcc)JCXZ_REL8;
	table[0xe4] = (t_faddrcc)IN_AL_I8;
	table[0xe5] = (t_faddrcc)IN_EAX_I8;
	table[0xe6] = (t_faddrcc)OUT_I8_AL;
	table[0xe7] = (t_faddrcc)OUT_I8_EAX;
	table[0xe8] = (t_faddrcc)CALL_REL32;
	table[0xe9] = (t_faddrcc)JMP_REL32;
	table[0xea] = (t_faddrcc)JMP_PTR16_32;
	table[0xeb] = (t_faddrcc)JMP_REL8;
	table[0xec] = (t_faddrcc)IN_AL_DX;
	table[0xed] = (t_faddrcc)IN_EAX_DX;
	table[0xee] = (t_faddrcc)OUT_DX_AL;
	table[0xef] = (t_faddrcc)OUT_DX_EAX;
	table[0xf0] = (t_faddrcc)PREFIX_LOCK;
	table[0xf1] = (t_faddrcc)QDX;
	table[0xf2] = (t_faddrcc)PREFIX_REPNZ;
	table[0xf3] = (t_faddrcc)PREFIX_REPZ;
	table[0xf4] = (t_faddrcc)HLT;
	table[0xf5] = (t_faddrcc)CMC;
	table[0xf6] = (t_faddrcc)INS_F6;
	table[0xf7] = (t_faddrcc)INS_F7;
	table[0xf8] = (t_faddrcc)CLC;
	table[0xf9] = (t_faddrcc)STC;
	table[0xfa] = (t_faddrcc)CLI;
	table[0xfb] = (t_faddrcc)STI;
	table[0xfc] = (t_faddrcc)CLD;
	table[0xfd] = (t_faddrcc)STD;
	table[0xfe] = (t_faddrcc)INS_FE;
	table[0xff] = (t_faddrcc)INS_FF;
	table_0f[0x00] = (t_faddrcc)INS_0F_00;
	table_0f[0x01] = (t_faddrcc)INS_0F_01;
	table_0f[0x02] = (t_faddrcc)LAR_R32_RM32;
	table_0f[0x03] = (t_faddrcc)LSL_R32_RM32;
	table_0f[0x04] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x05] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x06] = (t_faddrcc)CLTS;
	table_0f[0x07] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x08] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x09] = (t_faddrcc)WBINVD;
	table_0f[0x0a] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x0b] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x0c] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x0d] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x0e] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x0f] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x10] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x11] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x12] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x13] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x14] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x15] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x16] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x17] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x18] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x19] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x1a] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x1b] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x1c] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x1d] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x1e] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x1f] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x20] = (t_faddrcc)MOV_R32_CR;
	table_0f[0x21] = (t_faddrcc)MOV_R32_DR;
	table_0f[0x22] = (t_faddrcc)MOV_CR_R32;
	table_0f[0x23] = (t_faddrcc)MOV_DR_R32;
	table_0f[0x24] = (t_faddrcc)MOV_R32_TR;
	table_0f[0x25] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x26] = (t_faddrcc)MOV_TR_R32;
	table_0f[0x27] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x28] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x29] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x2a] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x2b] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x2c] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x2d] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x2e] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x2f] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x30] = (t_faddrcc)WRMSR;
	table_0f[0x31] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x32] = (t_faddrcc)RDMSR;
	table_0f[0x33] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x34] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x35] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x36] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x37] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x38] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x39] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x3a] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x3b] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x3c] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x3d] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x3e] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x3f] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x40] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x41] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x42] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x43] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x44] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x45] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x46] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x47] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x48] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x49] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x4a] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x4b] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x4c] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x4d] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x4e] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x4f] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x50] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x51] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x52] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x53] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x54] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x55] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x56] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x57] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x58] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x59] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x5a] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x5b] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x5c] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x5d] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x5e] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x5f] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x60] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x61] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x62] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x63] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x64] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x65] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x66] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x67] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x68] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x69] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x6a] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x6b] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x6c] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x6d] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x6e] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x6f] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x70] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x71] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x72] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x73] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x74] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x75] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x76] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x77] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x78] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x79] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x7a] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x7b] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x7c] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x7d] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x7e] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x7f] = (t_faddrcc)UndefinedOpcode;
	table_0f[0x80] = (t_faddrcc)JO_REL32;
	table_0f[0x81] = (t_faddrcc)JNO_REL32;
	table_0f[0x82] = (t_faddrcc)JC_REL32;
	table_0f[0x83] = (t_faddrcc)JNC_REL32;
	table_0f[0x84] = (t_faddrcc)JZ_REL32;
	table_0f[0x85] = (t_faddrcc)JNZ_REL32;
	table_0f[0x86] = (t_faddrcc)JNA_REL32;
	table_0f[0x87] = (t_faddrcc)JA_REL32;
	table_0f[0x88] = (t_faddrcc)JS_REL32;
	table_0f[0x89] = (t_faddrcc)JNS_REL32;
	table_0f[0x8a] = (t_faddrcc)JP_REL32;
	table_0f[0x8b] = (t_faddrcc)JNP_REL32;
	table_0f[0x8c] = (t_faddrcc)JL_REL32;
	table_0f[0x8d] = (t_faddrcc)JNL_REL32;
	table_0f[0x8e] = (t_faddrcc)JNG_REL32;
	table_0f[0x8f] = (t_faddrcc)JG_REL32;
	table_0f[0x90] = (t_faddrcc)SETO_RM8;
	table_0f[0x91] = (t_faddrcc)SETNO_RM8;
	table_0f[0x92] = (t_faddrcc)SETC_RM8;
	table_0f[0x93] = (t_faddrcc)SETNC_RM8;
	table_0f[0x94] = (t_faddrcc)SETZ_RM8;
	table_0f[0x95] = (t_faddrcc)SETNZ_RM8;
	table_0f[0x96] = (t_faddrcc)SETNA_RM8;
	table_0f[0x97] = (t_faddrcc)SETA_RM8;
	table_0f[0x98] = (t_faddrcc)SETS_RM8;
	table_0f[0x99] = (t_faddrcc)SETNS_RM8;
	table_0f[0x9a] = (t_faddrcc)SETP_RM8;
	table_0f[0x9b] = (t_faddrcc)SETNP_RM8;
	table_0f[0x9c] = (t_faddrcc)SETL_RM8;
	table_0f[0x9d] = (t_faddrcc)SETNL_RM8;
	table_0f[0x9e] = (t_faddrcc)SETNG_RM8;
	table_0f[0x9f] = (t_faddrcc)SETG_RM8;
	table_0f[0xa0] = (t_faddrcc)PUSH_FS;
	table_0f[0xa1] = (t_faddrcc)POP_FS;
	table_0f[0xa2] = (t_faddrcc)CPUID;
	table_0f[0xa3] = (t_faddrcc)BT_RM32_R32;
	table_0f[0xa4] = (t_faddrcc)SHLD_RM32_R32_I8;
	table_0f[0xa5] = (t_faddrcc)SHLD_RM32_R32_CL;
	table_0f[0xa6] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xa7] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xa8] = (t_faddrcc)PUSH_GS;
	table_0f[0xa9] = (t_faddrcc)POP_GS;
	table_0f[0xaa] = (t_faddrcc)RSM;
	table_0f[0xab] = (t_faddrcc)BTS_RM32_R32;
	table_0f[0xac] = (t_faddrcc)SHRD_RM32_R32_I8;
	table_0f[0xad] = (t_faddrcc)SHRD_RM32_R32_CL;
	table_0f[0xae] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xaf] = (t_faddrcc)IMUL_R32_RM32;
	table_0f[0xb0] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xb1] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xb2] = (t_faddrcc)LSS_R32_M16_32;
	table_0f[0xb3] = (t_faddrcc)BTR_RM32_R32;
	table_0f[0xb4] = (t_faddrcc)LFS_R32_M16_32;
	table_0f[0xb5] = (t_faddrcc)LGS_R32_M16_32;
	table_0f[0xb6] = (t_faddrcc)MOVZX_R32_RM8;
	table_0f[0xb7] = (t_faddrcc)MOVZX_R32_RM16;
	table_0f[0xb8] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xb9] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xba] = (t_faddrcc)INS_0F_BA;
	table_0f[0xbb] = (t_faddrcc)BTC_RM32_R32;
	table_0f[0xbc] = (t_faddrcc)BSF_R32_RM32;
	table_0f[0xbd] = (t_faddrcc)BSR_R32_RM32;
	table_0f[0xbe] = (t_faddrcc)MOVSX_R32_RM8;
	table_0f[0xbf] = (t_faddrcc)MOVSX_R32_RM16;
	table_0f[0xc0] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xc1] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xc2] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xc3] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xc4] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xc5] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xc6] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xc7] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xc8] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xc9] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xca] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xcb] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xcc] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xcd] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xce] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xcf] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xd0] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xd1] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xd2] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xd3] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xd4] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xd5] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xd6] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xd7] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xd8] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xd9] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xda] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xdb] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xdc] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xdd] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xde] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xdf] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xe0] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xe1] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xe2] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xe3] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xe4] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xe5] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xe6] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xe7] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xe8] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xe9] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xea] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xeb] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xec] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xed] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xee] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xef] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xf0] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xf1] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xf2] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xf3] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xf4] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xf5] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xf6] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xf7] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xf8] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xf9] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xfa] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xfb] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xfc] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xfd] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xfe] = (t_faddrcc)UndefinedOpcode;
	table_0f[0xff] = (t_faddrcc)UndefinedOpcode;
}
void vcpuinsReset()
{vcpurec.svcextl = 0;}
void vcpuinsRefresh()
{
#ifndef VGLOBAL_BOCHS
	if (!vcpu.flaghalt) {
#endif
		ExecIns();
#ifndef VGLOBAL_BOCHS
	} else {
		vapiSleep(1);
	}
#endif
	ExecInt();
}
void vcpuinsFinal(){}
