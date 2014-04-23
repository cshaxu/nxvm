/* This file is a part of NXVM project. */

#include "stdio.h"
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

#define _CheckExcept(s) if (vcpuins.except) {vapiPrint("%s\n",(s));return;} else
#define _CheckExceptRet(s) if (vcpuins.except) {vapiPrint("%s\n",(s));return 0;} else
#define i386(n)  if (1)

#define _GetModRM_MOD(modrm) (((modrm) & 0xc0) >> 6)
#define _GetModRM_REG(modrm) (((modrm) & 0x38) >> 3)
#define _GetModRM_RM(modrm)  (((modrm) & 0x07) >> 0)
#define _GetSIB_SS(sib)    (((sib) & 0xc0) >> 6)
#define _GetSIB_Index(sib) (((sib) & 0x38) >> 3)
#define _GetSIB_Base(sib)  (((sib) & 0x07) >> 0)

#define ADD_FLAG  (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                   VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF)
#define	 OR_FLAG  (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
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

t_cpuins vcpuins;

#define _GetOperandSize ((vcpu.cs.db ^ vcpuins.prefix_oprsize) ? 32 : 16)
#define _GetAddressSize ((vcpu.cs.db ^ vcpuins.prefix_addrsize) ? 32 : 16)

#define bugfix if

#define NOTIMP void
#define ASMCMP void
#define EXCEPT void

static void CaseError(const char *str)
{
	vapiPrint("The NXVM CPU has encountered an internal case error: \n%s\n",str);
	vapiCallBackMachineStop();
}

#define _SetExcept_PF(n) (vapiPrint("#PF(%x)\n",n), SetBit(vcpuins.except, VCPUINS_EXCEPT_PF), vcpuins.excode = (n))
#define _SetExcept_GP(n) (vapiPrint("#GP(%x)\n",n), SetBit(vcpuins.except, VCPUINS_EXCEPT_GP), vcpuins.excode = (n))
#define _SetExcept_SS(n) (vapiPrint("#SS(%x)\n",n), SetBit(vcpuins.except, VCPUINS_EXCEPT_SS), vcpuins.excode = (n))
#define _SetExcept_UD(n) (vapiPrint("#UD(%x)\n",n), SetBit(vcpuins.except, VCPUINS_EXCEPT_UD), vcpuins.excode = (n))
#define _SetExcept_NP(n) (vapiPrint("#NP(%x)\n",n), SetBit(vcpuins.except, VCPUINS_EXCEPT_NP), vcpuins.excode = (n))
#define _SetExcept_BR(n) (vapiPrint("#BR(%x)\n",n), SetBit(vcpuins.except, VCPUINS_EXCEPT_BR), vcpuins.excode = (n))

/* paging mechanism */
static t_nubit32 _GetPhysicalFromLinear(t_nubit32 linear, t_nubit8 cpl, t_bool write)
{
	/* possible exception: pf */
	t_nubit32 physical = linear;
	t_vaddrcc ppdtitem, ppetitem; /* page table entries */
	_CheckExceptRet("_GetPhysicalFromLinear");
	if (_GetCR0_PE && _GetCR0_PG) {
		ppdtitem = vramAddr(_GetCR3_BASE + _GetLinear_DIR(linear) * 4);
		if (!_GetPageEntry_P(d_nubit32(ppdtitem))) {
			_SetExcept_PF(_MakePageFaultErrorCode(0, write, (cpl == 3)));
			_CheckExceptRet("_GetPhysicalFromLinear::CR0_PE(1)::CR0_PG(1)::ppdtitem::PageEntry_P(0)");
			return physical;
		}
		if (cpl == 0x03) {
			if (!_GetPageEntry_US(d_nubit32(ppdtitem))) {
				_SetExcept_PF(_MakePageFaultErrorCode(1, write, 1));
				_CheckExceptRet("_GetPhysicalFromLinear::CR0_PE(1)::CR0_PG(1)::cpl(3)::ppdtitem::PageEntry_US(0)");
				return physical;
			}
			if (!_GetPageEntry_RW(d_nubit32(ppdtitem)) && write) {
				_SetExcept_PF(_MakePageFaultErrorCode(1, 1, 1));
				_CheckExceptRet("_GetPhysicalFromLinear::CR0_PE(1)::CR0_PG(1)::cpl(3)::ppdtitem::PageEntry_RW(0)::write(1)");
				return physical;
			}
		}
		_SetPageEntry_A(d_nubit32(ppdtitem));
		ppetitem = vramAddr(_GetPageEntry_BASE(d_nubit32(ppdtitem)) + _GetLinear_PAGE(linear) * 4);
		if (!_GetPageEntry_P(d_nubit32(ppetitem))) {
			_SetExcept_PF(_MakePageFaultErrorCode(0, write, (cpl == 3)));
			_CheckExceptRet("_GetPhysicalFromLinear::CR0_PE(1)::CR0_PG(1)::ppetitem::PageEntry_P(0)");
			return physical;
		}
		if (cpl == 0x03) {
			if (!_GetPageEntry_US(d_nubit32(ppetitem))) {
				_SetExcept_PF(_MakePageFaultErrorCode(1, write, 1));
				_CheckExceptRet("_GetPhysicalFromLinear::CR0_PE(1)::CR0_PG(1)::cpl(3)::ppetitem::PageEntry_US(0)");
				return physical;
			}
			if (!_GetPageEntry_RW(d_nubit32(ppetitem)) && write) {
				_SetExcept_PF(_MakePageFaultErrorCode(1, 1, 1));
				_CheckExceptRet("_GetPhysicalFromLinear::CR0_PE(1)::CR0_PG(1)::cpl(3)::ppetitem::PageEntry_RW(0)::write(1)");
				return physical;
			}
		}
		_SetPageEntry_A(d_nubit32(ppetitem));
		if (write) _SetPageEntry_D(d_nubit32(ppetitem));
		physical = _GetPageEntry_BASE(d_nubit32(ppetitem)) + _GetLinear_OFFSET(linear);
	}
	return physical;
}

/* read descriptor table */
static t_nubit64 _GetDescriptorFromGDT(t_nubit16 selector)
{
	/* possible exception: gp/limit; pf/lost */
	t_nubit64 descriptor = 0x0000000000000000;
	t_nubit32 linear = _GetGDTR_BASE + _GetSelector_OFFSET(selector);
	t_nubit32 physical = 0x00000000;
	_CheckExceptRet("_GetDescriptorFromGDT");
	if (_GetSelector_OFFSET(selector) + 7 > _GetGDTR_LIMIT) {
		_SetExcept_GP(selector);
		_CheckExceptRet("_GetDescriptorFromLDT::gdtr.limit");
		return descriptor;
	}
	physical = _GetPhysicalFromLinear(linear, 0x00, 0x01);
	_CheckExceptRet("_GetDescriptorFromGDT::GetPhysicalFromLinear");
	if (_GetSegDesc_S(descriptor)) _SetSegDesc_TYPE_A(vramQWord(physical));
	descriptor = vramQWord(physical);
	return descriptor;
}
static t_nubit64 _GetDescriptorFromLDT(t_nubit16 selector)
{
	/* possible exception: gp/limit; pf/lost */
	t_nubit64 descriptor = 0x0000000000000000;
	t_nubit32 linear = vcpu.ldtr.base + _GetSelector_OFFSET(selector);
	t_nubit32 physical = 0x00000000;
	_CheckExceptRet("_GetDescriptorFromLDT");
	if (linear + 7 > vcpu.ldtr.limit) {
		_SetExcept_GP(selector);
		_CheckExceptRet("_GetDescriptorFromLDT::ldtr.limit");
		return descriptor;
	}
	physical = _GetPhysicalFromLinear(linear, 0x00, 0x01);
	_CheckExceptRet("_GetDescriptorFromLDT::GetPhysicalFromLinear");
	if (_GetSegDesc_S(descriptor)) _SetSegDesc_TYPE_A(vramQWord(physical));
	descriptor = vramQWord(physical);
	return descriptor;
}
static t_nubit64 _GetDescriptor(t_nubit16 selector)
{
	/* possible exception: gp/limit; pf/lost */
	t_nubit64 descriptor = 0x0000000000000000;
	_CheckExceptRet("_GetDescriptor");
	if (_GetCR0_PE && !_GetEFLAGS_VM) {
		if (_GetSelector_TI(selector)) {
			descriptor = _GetDescriptorFromLDT(selector);
			_CheckExceptRet("_GetDescriptor::PROTECTED::Selector_TI(1)::_GetDescriptorFromLDT");
		} else {
			if (_GetSelector_INDEX(selector)) {
				descriptor = _GetDescriptorFromGDT(selector);
				_CheckExceptRet("_GetDescriptor::PROTECTED::Selector_TI(0)::_GetDescriptorFromGDT");
			} else {
				_SetExcept_GP(0);
				_CheckExceptRet("GetDescriptor::PROTECTED::selector(null)");
			}
		}
	} else
		CaseError("_GetDescriptor::CR0_PE(0)/EFLAGS_VM(1)");
	return descriptor;
}

/* segmentation machanism */
static t_nubit32 _GetLinearFromLogical(t_cpu_sreg *psreg, t_nubit32 offset, t_nubit8 bit, t_bool write, t_bool force)
{
	/* gp, ss, pf */
	t_nubit32 linear = psreg->base + offset;
	t_nubit32 upper = (!psreg->cd && psreg->ce) ? (psreg->db ? 0xffffffff : 0x0000ffff) : psreg->limit;
	t_nubit32 lower = (!psreg->cd && psreg->ce) ? (psreg->limit + 1) : 0x00000000;
	_CheckExceptRet("_GetLinearFromLogical");
	if (_GetCR0_PE && !_GetEFLAGS_VM) {
		switch (psreg->sregtype) {
		case SREG_CODE:
			if (_IsSelectorNull(psreg->selector) || !psreg->cd) {
				CaseError("_GetLinearFromLogical::PROTECTED::sregtype(SREG_CODE)");
				return 0x00000000;
			}
			if (!force && (write || !psreg->rw)) {
				_SetExcept_GP(0);
				_CheckExceptRet("_GetLinearFromLogical::PROTECTED::sregtype(SREG_CODE)");
			}
			break;
		case SREG_DATA:
			if (_IsSelectorNull(psreg->selector)) {
				_SetExcept_GP(0);
				_CheckExceptRet("_GetLinearFromLogical::PROTECTED::sregtype(SREG_DATA)::selector(null)");
			}
			if (psreg->cd && !psreg->rw) { // cd=1,rw=0
				CaseError("_GetLinearFromLogical::PROTECTED::sregtype(SREG_DATA)::cd(1)::rw(0)");
				return 0x00000000;
			}
			if (!force && write && (psreg->cd || !psreg->rw)) {
				_SetExcept_GP(0);
				_CheckExceptRet("_GetLinearFromLogical::PROTECTED::sregtype(SREG_DATA)::write(1)");
			}
			break;
		case SREG_STACK:
			if (_IsSelectorNull(psreg->selector) || psreg->cd || !psreg->rw) {
				CaseError("_GetLinearFromLogical::PROTECTED::sregtype(SREG_STACK)");
				return 0x00000000;
			}
			break;
		default:
			CaseError("_GetLinearFromLogical::PROTECTED::sregtype");
			return 0x00000000;
		}
	}
	switch (bit) {
	case 8:
		if (linear < lower || linear > upper) {
			if (psreg->sregtype == SREG_STACK)
				_SetExcept_SS(0);
			else
				_SetExcept_GP(0);
			_CheckExceptRet("_GetLinearFromLogical::bit(8)");
		}
		break;
	case 16:
		if (linear < lower || linear >= upper) {
			if (psreg->sregtype == SREG_STACK)
				_SetExcept_SS(0);
			else
				_SetExcept_GP(0);
			_CheckExceptRet("_GetLinearFromLogical::bit(16)");
		}
		break;
	case 32:
		if (linear < lower || linear >= upper - 2) {
			if (psreg->sregtype == SREG_STACK)
				_SetExcept_SS(0);
			else
				_SetExcept_GP(0);
			_CheckExceptRet("_GetLinearFromLogical::bit(32)");
		}
		break;
	case 48:
		if (linear < lower || linear >= upper - 4) {
			if (psreg->sregtype == SREG_STACK)
				_SetExcept_SS(0);
			else
				_SetExcept_GP(0);
			_CheckExceptRet("_GetLinearFromLogical::bit(48)");
		}
		break;
	case 64:
		if (linear < lower || linear >= upper - 6) {
			if (psreg->sregtype == SREG_STACK)
				_SetExcept_SS(0);
			else
				_SetExcept_GP(0);
			_CheckExceptRet("_GetLinearFromLogical::bit(64)");
		}
		break;
	default:
		CaseError("_GetLinearFromLogical::bit");
		return 0x00000000;
	}
	return linear;
}
#define _GetPhysicalFromLogical(psreg, offset, bit, write, force) \
	_GetPhysicalFromLinear(_GetLinearFromLogical((psreg), (offset), (bit), (write), (force)), ((force) ? 0 : _GetCPL), (write))
#define _GetPhysicalOfSegment _GetPhysicalFromLogical
#define _CheckSegment(psreg, offset, bit, write) _GetPhysicalOfSegment((psreg), (offset), (bit), (write), 0x00);
static void _LoadSegment(t_cpu_sreg *psreg, t_nubit16 selector)
{
	/* TODO: need to be verified when constructing segment loaders */
	t_nubit64 descriptor = 0x0000000000000000;
	/* this does not check for priority level */
	if (_GetCR0_PE && !_GetEFLAGS_VM) {
		if (!_IsSelectorNull(selector)) {
			descriptor = _GetDescriptor(selector);
			_CheckExcept("_LoadSegment::PROTECTED::selector(!null)::_GetDescriptor");
			switch (psreg->sregtype) {
			case SREG_CODE:
				if (!_GetSegDesc_S(descriptor) || !_GetSegDesc_TYPE_CD(descriptor)) {
					_SetExcept_GP(selector);
					_CheckExcept("_LoadSegment::PROTECTED::selector(!null)::sregtype(SREG_CODE)::SegDesc_TYPE_CD(0)/SegDesc_S(0)/SegDesc_P(0)");
				}
				if (!_GetSegDesc_P(descriptor)) {
					_SetExcept_NP(selector);
					_CheckExcept("_LoadSegment::PROTECTED::selector(!null)::sregtype(SREG_CODE)::SegDesc_P(0)");
				}
				psreg->a     = _GetSegDesc_TYPE_A(descriptor);
				psreg->base  = _GetSegDesc_BASE(descriptor);
				psreg->cd    = _GetSegDesc_TYPE_CD(descriptor);
				psreg->ce    = _GetSegDesc_TYPE_C(descriptor);
				psreg->db    = _GetSegDesc_DB(descriptor);
				psreg->dpl   = psreg->ce ? psreg->dpl : _GetSegDesc_DPL(descriptor);
				psreg->g     = _GetSegDesc_G(descriptor);
				psreg->limit = psreg->base + (psreg->g ? ((_GetSegDesc_LIMIT(descriptor) << 12) | 0x0fff) : _GetSegDesc_LIMIT(descriptor));
				psreg->p     = _GetSegDesc_P(descriptor);
				psreg->rw    = _GetSegDesc_TYPE_R(descriptor);
				psreg->s     = _GetSegDesc_S(descriptor);
				psreg->selector = (selector & ~VCPU_SELECTOR_RPL) | psreg->dpl;
				break;
			case SREG_STACK:
				if (_GetCPL != _GetSelector_RPL(selector) || _GetCPL != _GetSegDesc_DPL(descriptor)) {
					_SetExcept_GP(selector);
					_CheckExcept("_LoadSegment::PROTECTED::selector(!null)::sregtype(SREG_STACK)::?PL");
				}
				if (!_GetSegDesc_S(descriptor) || _GetSegDesc_TYPE_CD(descriptor) || !_GetSegDesc_TYPE_W(descriptor)) {
					_SetExcept_GP(selector);
					_CheckExcept("_LoadSegment::PROTECTED::selector(!null)::sregtype(SREG_STACK)::SegDesc_S(0)/SegDesc_TYPE_CD(1)/SegDesc_TYPE_W(0)");
				}
				if (!_GetSegDesc_P(descriptor)) {
					_SetExcept_SS(selector);
					_CheckExcept("_LoadSegment::PROTECTED::selector(!null)::sregtype(SREG_STACK)::SegDesc_P(0)");
				}
				psreg->a     = _GetSegDesc_TYPE_A(descriptor);
				psreg->base  = _GetSegDesc_BASE(descriptor);
				psreg->cd    = _GetSegDesc_TYPE_CD(descriptor);
				psreg->ce    = _GetSegDesc_TYPE_E(descriptor);
				psreg->db    = _GetSegDesc_DB(descriptor);
				psreg->dpl   = _GetSegDesc_DPL(descriptor);
				psreg->g     = _GetSegDesc_G(descriptor);
				psreg->limit = psreg->base + (psreg->g ? ((_GetSegDesc_LIMIT(descriptor) << 12) | 0x0fff) : _GetSegDesc_LIMIT(descriptor));
				psreg->p     = _GetSegDesc_P(descriptor);
				psreg->rw    = _GetSegDesc_TYPE_W(descriptor);
				psreg->s     = _GetSegDesc_S(descriptor);
				psreg->selector = selector;
				break;
			case SREG_DATA:
				if (!_GetSegDesc_TYPE_CD(descriptor) || !_GetSegDesc_TYPE_C(descriptor)) {
					if (_GetSelector_RPL(selector) > _GetSegDesc_DPL(descriptor) && _GetCPL > _GetSegDesc_DPL(descriptor)) {
					_SetExcept_GP(selector);
					_CheckExcept("_LoadSegment::PROTECTED::selector(!null)::sregtype(SREG_DATA)::?PL");
					}
				}
				if (!_GetSegDesc_S(descriptor) || (_GetSegDesc_TYPE_CD(descriptor) && !_GetSegDesc_TYPE_R(descriptor))) {
					_SetExcept_GP(selector);
					_CheckExcept("_LoadSegment::PROTECTED::selector(!null)::sregtype(SREG_DATA)::...");
				}
				if (!_GetSegDesc_P(descriptor)) {
					_SetExcept_NP(selector);
					_CheckExcept("_LoadSegment::PROTECTED::selector(!null)::sregtype(SREG_DATA)::SegDesc_P(0)");
				}
				psreg->a     = _GetSegDesc_TYPE_A(descriptor);
				psreg->base  = _GetSegDesc_BASE(descriptor);
				psreg->cd    = _GetSegDesc_TYPE_CD(descriptor);
				psreg->ce    = psreg->cd ? _GetSegDesc_TYPE_C(descriptor) : _GetSegDesc_TYPE_E(descriptor);
				psreg->db    = _GetSegDesc_DB(descriptor);
				psreg->dpl   = _GetSegDesc_DPL(descriptor);
				psreg->g     = _GetSegDesc_G(descriptor);
				psreg->limit = psreg->base + (psreg->g ? ((_GetSegDesc_LIMIT(descriptor) << 12) | 0x0fff) : _GetSegDesc_LIMIT(descriptor));
				psreg->p     = _GetSegDesc_P(descriptor);
				psreg->rw    = psreg->cd ? _GetSegDesc_TYPE_R(descriptor) : _GetSegDesc_TYPE_W(descriptor);
				psreg->s     = _GetSegDesc_S(descriptor);
				psreg->selector = selector;
				break;
			default:
				break;
			}
		} else {
			switch (psreg->sregtype) {
			case SREG_DATA:
				psreg->selector = selector;
				break;
			case SREG_CODE:
			case SREG_STACK:
				_SetExcept_GP(0);
				_CheckExcept("_LoadSegment::PROTECTED::selector(null)::sregtype(SREG_CODE/SREG_STACK)");
				break;
			default:
				CaseError("_LoadSegment::PROTECTED::selector(null)::sregtype");
				return;
			}
		}
	} else {
		psreg->selector = selector;
		psreg->limit -= psreg->base;
		psreg->base = (selector << 4);
		psreg->limit += psreg->base;
	}
}

/* fetch instructions */
static t_cpu_sreg *_GetPtrOverrideDS()
{
	t_cpu_sreg *pds = NULL;
	switch (vcpuins.prefix_sreg) {
	case PREFIX_SREG_NONE: pds = &vcpu.ds;break;
	case PREFIX_SREG_CS: pds = &vcpu.cs;break;
	case PREFIX_SREG_SS: pds = &vcpu.ss;break;
	case PREFIX_SREG_DS: pds = &vcpu.ds;break;
	case PREFIX_SREG_ES: pds = &vcpu.es;break;
	case PREFIX_SREG_FS: pds = &vcpu.fs;break;
	case PREFIX_SREG_GS: pds = &vcpu.gs;break;
	default: CaseError("_GetPtrOverrideDS::prefix_sreg");break;
	}
	return pds;
}
static t_cpu_sreg *_GetPtrOverrideSS()
{
	t_cpu_sreg *pss = NULL;
	switch (vcpuins.prefix_sreg) {
	case PREFIX_SREG_NONE: pss = &vcpu.ss;break;
	case PREFIX_SREG_CS: pss = &vcpu.cs;break;
	case PREFIX_SREG_SS: pss = &vcpu.ss;break;
	case PREFIX_SREG_DS: pss = &vcpu.ds;break;
	case PREFIX_SREG_ES: pss = &vcpu.es;break;
	case PREFIX_SREG_FS: pss = &vcpu.fs;break;
	case PREFIX_SREG_GS: pss = &vcpu.gs;break;
	default: CaseError("_GetPtrOverrideSS::prefix_sreg");break;
	}
	return pss;
}
static t_nubit32 _GetCode(t_nubit32 offset, t_nubit8 bit)
{
	/* gp, pf */
	t_nubit32 physical = _GetPhysicalOfSegment(&vcpu.cs, offset, bit, 0x00, 0x01);
	_CheckExceptRet("_GetCode::_GetPhysicalOfSegment");
	switch (bit) {
	case 8:  return vramByte(physical);
	case 16: return vramWord(physical);
	case 32: return vramDWord(physical);
	case 48: return GetMax48(vramQWord(physical));
	case 64: return vramQWord(physical);
	default: CaseError("_GetCode::bit");return 0x00000000;
	}
	return 0x00000000;
}
static void _GetImm(t_nubit8 immbit)
{
	/* gp, pf */
	t_nubit32 physical = _GetPhysicalOfSegment(&vcpu.cs, vcpu.eip, immbit, 0x00, 0x01);
	_CheckExcept("_GetImm::_GetPhysicalOfSegment");
	vcpuins.pimm = vramAddr(physical);
	switch (immbit) {
	case 8:  vcpu.eip += 1;break;
	case 16: vcpu.eip += 2;break;
	case 32: vcpu.eip += 4;break;
	case 48: vcpu.eip += 6;break;
	default: CaseError("_GetImm::immbit");break;
	}
}
static void _GetModRegRM(t_nubit8 regbit, t_nubit8 rmbit, t_bool write)
{
	/* gp, pf, ss */
	t_nsbit8 disp8 = 0x00;
	t_nubit16 disp16 = 0x0000;
	t_nubit32 disp32 = 0x00000000;
	t_nubit32 physical = 0x00000000, sibindex = 0x00000000;
	t_cpu_sreg *pds = _GetPtrOverrideDS(), *pss = _GetPtrOverrideSS();
	t_nubit8 modrm = (t_nubit8)_GetCode(vcpu.eip++, 8), sib = 0x00;
	_CheckExcept("_GetModRegRM::_GetCode");
	vcpuins.prm = vcpuins.pr = (t_vaddrcc)NULL;
	vcpuins.lrm = 0x00000000;
	vcpuins.flagmem = 0x01;
	if (_GetAddressSize == 16) {
		switch (_GetModRM_MOD(modrm)) {
		case 0:
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.lrm = _GetLinearFromLogical(pds, GetMax16(vcpu.bx+vcpu.si), rmbit, write, 0x00); break;
			case 1: vcpuins.lrm = _GetLinearFromLogical(pds, GetMax16(vcpu.bx+vcpu.di), rmbit, write, 0x00); break;
			case 2: vcpuins.lrm = _GetLinearFromLogical(pss, GetMax16(vcpu.bp+vcpu.si), rmbit, write, 0x00); break;
			case 3: vcpuins.lrm = _GetLinearFromLogical(pss, GetMax16(vcpu.bp+vcpu.di), rmbit, write, 0x00); break;
			case 4: vcpuins.lrm = _GetLinearFromLogical(pds, GetMax16(vcpu.si), rmbit, write, 0x00); break;
			case 5: vcpuins.lrm = _GetLinearFromLogical(pds, GetMax16(vcpu.di), rmbit, write, 0x00); break;
			case 6:
				disp16 = (t_nubit16)_GetCode(vcpu.eip, 16);
				_CheckExcept("_GetModRegRM::AddressSize(16)::ModRM_MOD(0)::ModRM_RM(6)::_GetCode");
				vcpu.eip += 2;
				vcpuins.lrm = _GetLinearFromLogical(pds, GetMax16(disp16),  rmbit, write, 0x00); break;
			case 7: vcpuins.lrm = _GetLinearFromLogical(pds, GetMax16(vcpu.bx), rmbit, write, 0x00); break;
			default:CaseError("_GetModRegRM::AddressSize(16)::ModRM_MOD(0)::ModRM_RM");return;
			}
			_CheckExcept("_GetModRegRM::AddressSize(16)::ModRM_MOD(0)::_GetLinearFromLogical");
			physical = _GetPhysicalFromLinear(vcpuins.lrm, _GetCPL, write);
			_CheckExcept("_GetModRegRM::AddressSize(16)::ModRM_MOD(0)::_GetPhysicalFromLinear");
			vcpuins.prm = vramAddr(physical);
			break;
		case 1:
			disp8 = (t_nsbit8)_GetCode(vcpu.eip, 8);
			_CheckExcept("_GetModRegRM::AddressSize(16)::ModRM_MOD(1)::_GetCode");
			vcpu.eip += 1;
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.lrm = _GetLinearFromLogical(pds, GetMax16(vcpu.bx+vcpu.si+disp8), rmbit, write, 0x00); break;
			case 1: vcpuins.lrm = _GetLinearFromLogical(pds, GetMax16(vcpu.bx+vcpu.di+disp8), rmbit, write, 0x00); break;
			case 2: vcpuins.lrm = _GetLinearFromLogical(pss, GetMax16(vcpu.bp+vcpu.si+disp8), rmbit, write, 0x00); break;
			case 3: vcpuins.lrm = _GetLinearFromLogical(pss, GetMax16(vcpu.bp+vcpu.di+disp8), rmbit, write, 0x00); break;
			case 4: vcpuins.lrm = _GetLinearFromLogical(pds, GetMax16(vcpu.si+disp8), rmbit, write, 0x00); break;
			case 5: vcpuins.lrm = _GetLinearFromLogical(pds, GetMax16(vcpu.di+disp8), rmbit, write, 0x00); break;
			case 6: vcpuins.lrm = _GetLinearFromLogical(pss, GetMax16(vcpu.bp+disp8), rmbit, write, 0x00); break;
			case 7: vcpuins.lrm = _GetLinearFromLogical(pds, GetMax16(vcpu.bx+disp8), rmbit, write, 0x00); break;
			default:CaseError("_GetModRegRM::AddressSize(16)::ModRM_MOD(1)::ModRM_RM");return;
			}
			_CheckExcept("_GetModRegRM::AddressSize(16)::ModRM_MOD(1)::_GetLinearFromLogical");
			physical = _GetPhysicalFromLinear(vcpuins.lrm, _GetCPL, write);
			_CheckExcept("_GetModRegRM::AddressSize(16)::ModRM_MOD(1)::_GetPhysicalFromLinear");
			vcpuins.prm = vramAddr(physical);
			break;
		case 2:
			disp16 = (t_nubit16)_GetCode(vcpu.eip, 16);
			_CheckExcept("_GetModRegRM::AddressSize(16)::ModRM_MOD(2)::_GetCode");
			vcpu.eip += 2;
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.lrm = _GetLinearFromLogical(pds, GetMax16(vcpu.bx+vcpu.si+disp16), rmbit, write, 0x00); break;
			case 1: vcpuins.lrm = _GetLinearFromLogical(pds, GetMax16(vcpu.bx+vcpu.di+disp16), rmbit, write, 0x00); break;
			case 2: vcpuins.lrm = _GetLinearFromLogical(pss, GetMax16(vcpu.bp+vcpu.si+disp16), rmbit, write, 0x00); break;
			case 3: vcpuins.lrm = _GetLinearFromLogical(pss, GetMax16(vcpu.bp+vcpu.di+disp16), rmbit, write, 0x00); break;
			case 4: vcpuins.lrm = _GetLinearFromLogical(pds, GetMax16(vcpu.si+disp16), rmbit, write, 0x00); break;
			case 5: vcpuins.lrm = _GetLinearFromLogical(pds, GetMax16(vcpu.di+disp16), rmbit, write, 0x00); break;
			case 6: vcpuins.lrm = _GetLinearFromLogical(pss, GetMax16(vcpu.bp+disp16), rmbit, write, 0x00); break;
			case 7: vcpuins.lrm = _GetLinearFromLogical(pds, GetMax16(vcpu.bx+disp16), rmbit, write, 0x00); break;
			default:CaseError("_GetModRegRM::AddressSize(16)::ModRM_MOD(2)::ModRM_RM");return;
			}
			_CheckExcept("_GetModRegRM::AddressSize(16)::ModRM_MOD(2)::_GetLinearFromLogical");
			physical = _GetPhysicalFromLinear(vcpuins.lrm, _GetCPL, write);
			_CheckExcept("_GetModRegRM::AddressSize(16)::ModRM_MOD(2)::_GetPhysicalFromLinear");
			vcpuins.prm = vramAddr(physical);
			break;
		case 3:
			break;
		default:
			CaseError("_GetModRegRM::AddressSize(16)::ModRM_MOD");
			return;
		}
	} else {
		if (_GetModRM_MOD(modrm) != 3 && _GetModRM_RM(modrm) == 4) {
			sib = (t_nubit8)_GetCode(vcpu.eip, 8);
			_CheckExcept("_GetModRegRM::AddressSize(32)::ModRM_MOD(!3)::ModRM_RM(4)::_GetCode");
			vcpu.eip += 1;
			switch (_GetSIB_Index(sib)) {
			case 0: sibindex = vcpu.eax; break;
			case 1: sibindex = vcpu.ecx; break;
			case 2: sibindex = vcpu.edx; break;
			case 3: sibindex = vcpu.ebx; break;
			case 4: sibindex = 0x00000000; break;
			case 5: sibindex = vcpu.ebp; break;
			case 6: sibindex = vcpu.esi; break;
			case 7: sibindex = vcpu.edi; break;
			default:CaseError("_GetModRegRM::AddressSize(32)::ModRM_MOD(!3)::ModRM_RM(4)::SIB_Index");return;
			}
			sibindex <<= _GetSIB_SS(sib);
		}
		switch (_GetModRM_MOD(modrm)) {
		case 0:
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.eax, rmbit, write, 0x00); break;
			case 1: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.ecx, rmbit, write, 0x00); break;
			case 2: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.edx, rmbit, write, 0x00); break;
			case 3: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.ebx, rmbit, write, 0x00); break;
			case 4:
				switch (_GetSIB_Base(sib)) {
				case 0: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.eax + sibindex, rmbit, write, 0x00); break;
				case 1: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.ecx + sibindex, rmbit, write, 0x00); break;
				case 2: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.edx + sibindex, rmbit, write, 0x00); break;
				case 3: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.ebx + sibindex, rmbit, write, 0x00); break;
				case 4: vcpuins.lrm = _GetLinearFromLogical(pss, vcpu.esp + sibindex, rmbit, write, 0x00); break;
				case 5:
					disp32 = (t_nubit32)_GetCode(vcpu.eip, 32);
					_CheckExcept("_GetModRegRM::AddressSize(32)::ModRM_MOD(0)::ModRM::RM(4)::SIB_Base(5)::_GetCode");
					vcpu.eip += 4;
					vcpuins.lrm = _GetLinearFromLogical(pds, disp32 + sibindex, rmbit, write, 0x00);
					break;
				case 6: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.esi + sibindex, rmbit, write, 0x00); break;
				case 7: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.edi + sibindex, rmbit, write, 0x00); break;
				default:CaseError("_GetModRegRM::AddressSize(32)::ModRM_MOD(0)::ModRM_RM(4)::SIB_Base");return;
				}
				break;
			case 5:
				disp32 = (t_nubit32)_GetCode(vcpu.eip, 32);
				_CheckExcept("_GetModRegRM::AddressSize(32)::ModRM_MOD(0)::ModRM::RM(5)::_GetCode");
				vcpu.eip += 4;
				vcpuins.lrm = _GetLinearFromLogical(pds, disp32,   rmbit, write, 0x00);
				break;
			case 6: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.esi, rmbit, write, 0x00); break;
			case 7: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.edi, rmbit, write, 0x00); break;
			default:CaseError("_GetModRegRM::AddressSize(32)::ModRM_MOD(0)::ModRM_RM");return;
			}
			_CheckExcept("_GetModRegRM::AddressSize(32)::ModRM_MOD(0)::_GetLinearFromLogical");
			physical = _GetPhysicalFromLinear(vcpuins.lrm, _GetCPL, write);
			_CheckExcept("_GetModRegRM::AddressSize(32)::ModRM_MOD(0)::_GetPhysicalFromLinear");
			vcpuins.prm = vramAddr(physical);
			break;
		case 1:
			disp8 = (t_nsbit8)_GetCode(vcpu.eip, 8);
			_CheckExcept("_GetModRegRM::AddressSize(32)::ModRM_MOD(1)::_GetCode");
			vcpu.eip += 1;
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.eax + disp8, rmbit, write, 0x00); break;
			case 1: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.ecx + disp8, rmbit, write, 0x00); break;
			case 2: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.edx + disp8, rmbit, write, 0x00); break;
			case 3: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.ebx + disp8, rmbit, write, 0x00); break;
			case 4:
				switch (_GetSIB_Base(sib)) {
				case 0: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.eax + sibindex + disp8, rmbit, write, 0x00); break;
				case 1: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.ecx + sibindex + disp8, rmbit, write, 0x00); break;
				case 2: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.edx + sibindex + disp8, rmbit, write, 0x00); break;
				case 3: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.ebx + sibindex + disp8, rmbit, write, 0x00); break;
				case 4: vcpuins.lrm = _GetLinearFromLogical(pss, vcpu.esp + sibindex + disp8, rmbit, write, 0x00); break;
				case 5: vcpuins.lrm = _GetLinearFromLogical(pss, vcpu.ebp + sibindex + disp8, rmbit, write, 0x00); break;
				case 6: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.esi + sibindex + disp8, rmbit, write, 0x00); break;
				case 7: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.edi + sibindex + disp8, rmbit, write, 0x00); break;
				default:CaseError("_GetModRegRM::AddressSize(32)::ModRM_MOD(1)::ModRM_RM(4)::SIB_Base");return;
				}
				break;
			case 5: vcpuins.lrm = _GetLinearFromLogical(pss, vcpu.ebp + disp8, rmbit, write, 0x00); break;
			case 6: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.esi + disp8, rmbit, write, 0x00); break;
			case 7: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.edi + disp8, rmbit, write, 0x00); break;
			default:CaseError("_GetModRegRM::AddressSize(32)::ModRM_MOD(1)::ModRM_RM");return;
			}
			_CheckExcept("_GetModRegRM::AddressSize(32)::ModRM_MOD(1)::_GetLinearFromLogical");

			physical = _GetPhysicalFromLinear(vcpuins.lrm, _GetCPL, write);
			_CheckExcept("_GetModRegRM::AddressSize(32)::ModRM_MOD(1)::_GetPhysicalFromLinear");
			vcpuins.prm = vramAddr(physical);
			break;
		case 2:
			disp32 = (t_nubit32)_GetCode(vcpu.eip, 32);
			_CheckExcept("_GetModRegRM::AddressSize(32)::ModRM_MOD(2)::_GetCode");
			vcpu.eip += 4;
			switch (_GetModRM_RM(modrm)) {
			case 0: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.eax + disp32, rmbit, write, 0x00); break;
			case 1: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.ecx + disp32, rmbit, write, 0x00); break;
			case 2: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.edx + disp32, rmbit, write, 0x00); break;
			case 3: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.ebx + disp32, rmbit, write, 0x00); break;
			case 4:
				switch (_GetSIB_Base(sib)) {
				case 0: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.eax + sibindex + disp32, rmbit, write, 0x00); break;
				case 1: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.ecx + sibindex + disp32, rmbit, write, 0x00); break;
				case 2: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.edx + sibindex + disp32, rmbit, write, 0x00); break;
				case 3: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.ebx + sibindex + disp32, rmbit, write, 0x00); break;
				case 4: vcpuins.lrm = _GetLinearFromLogical(pss, vcpu.esp + sibindex + disp32, rmbit, write, 0x00); break;
				case 5: vcpuins.lrm = _GetLinearFromLogical(pss, vcpu.ebp + sibindex + disp32, rmbit, write, 0x00); break;
				case 6: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.esi + sibindex + disp32, rmbit, write, 0x00); break;
				case 7: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.edi + sibindex + disp32, rmbit, write, 0x00); break;
				default:CaseError("_GetModRegRM::AddressSize(32)::ModRM_MOD(2)::ModRM_RM(4)::SIB_Base");return;
				}
				break;
			case 5: vcpuins.lrm = _GetLinearFromLogical(pss, vcpu.ebp + disp32, rmbit, write, 0x00); break;
			case 6: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.esi + disp32, rmbit, write, 0x00); break;
			case 7: vcpuins.lrm = _GetLinearFromLogical(pds, vcpu.edi + disp32, rmbit, write, 0x00); break;
			default:CaseError("_GetModRegRM::AddressSize(32)::ModRM_MOD(2)::ModRM_RM");return;
			}
			_CheckExcept("_GetModRegRM::AddressSize(32)::ModRM_MOD(2)::_GetLinearFromLogical");
			physical = _GetPhysicalFromLinear(vcpuins.lrm, _GetCPL, write);
			_CheckExcept("_GetModRegRM::AddressSize(32)::ModRM_MOD(2)::_GetPhysicalFromLinear");
			vcpuins.prm = vramAddr(physical);
			break;
		case 3:
			break;
		default:
			CaseError("_GetModRegRM::AddressSize(32)::ModRM_MOD");
			break;
		}
	}
	if (_GetModRM_MOD(modrm) == 3) {
		vcpuins.flagmem = 0x00;
		switch (rmbit) {
		case 8:
			switch(_GetModRM_RM(modrm)) {
			case 0: vcpuins.prm = (t_vaddrcc)(&vcpu.al);break;
			case 1: vcpuins.prm = (t_vaddrcc)(&vcpu.cl);break;
			case 2: vcpuins.prm = (t_vaddrcc)(&vcpu.dl);break;
			case 3: vcpuins.prm = (t_vaddrcc)(&vcpu.bl);break;
			case 4: vcpuins.prm = (t_vaddrcc)(&vcpu.ah);break;
			case 5: vcpuins.prm = (t_vaddrcc)(&vcpu.ch);break;
			case 6: vcpuins.prm = (t_vaddrcc)(&vcpu.dh);break;
			case 7: vcpuins.prm = (t_vaddrcc)(&vcpu.bh);break;
			default:CaseError("_GetModRegRM::ModRM_MOD(3)::rmbit(8)::ModRM_RM");return;}
			break;
		case 16:
			switch(_GetModRM_RM(modrm)) {
			case 0: vcpuins.prm = (t_vaddrcc)(&vcpu.ax);break;
			case 1: vcpuins.prm = (t_vaddrcc)(&vcpu.cx);break;
			case 2: vcpuins.prm = (t_vaddrcc)(&vcpu.dx);break;
			case 3: vcpuins.prm = (t_vaddrcc)(&vcpu.bx);break;
			case 4: vcpuins.prm = (t_vaddrcc)(&vcpu.sp);break;
			case 5: vcpuins.prm = (t_vaddrcc)(&vcpu.bp);break;
			case 6: vcpuins.prm = (t_vaddrcc)(&vcpu.si);break;
			case 7: vcpuins.prm = (t_vaddrcc)(&vcpu.di);break;
			default:CaseError("_GetModRegRM::ModRM_MOD(3)::rmbit(16)::ModRM_RM");return;}
			break;
		case 32:
			switch(_GetModRM_RM(modrm)) {
			case 0: vcpuins.prm = (t_vaddrcc)(&vcpu.eax);break;
			case 1: vcpuins.prm = (t_vaddrcc)(&vcpu.ecx);break;
			case 2: vcpuins.prm = (t_vaddrcc)(&vcpu.edx);break;
			case 3: vcpuins.prm = (t_vaddrcc)(&vcpu.ebx);break;
			case 4: vcpuins.prm = (t_vaddrcc)(&vcpu.esp);break;
			case 5: vcpuins.prm = (t_vaddrcc)(&vcpu.ebp);break;
			case 6: vcpuins.prm = (t_vaddrcc)(&vcpu.esi);break;
			case 7: vcpuins.prm = (t_vaddrcc)(&vcpu.edi);break;
			default:CaseError("_GetModRegRM::ModRM_MOD(3)::rmbit(32)::ModRM_RM");return;}
			break;
		case 48:
			CaseError("_GetModRegRM::ModRM_MOD(3)::rmbit(48)");
			return;
		case 64:
			CaseError("_GetModRegRM::ModRM_MOD(3)::rmbit(64)");
			return;
		default:
			CaseError("GetModRegRM::ModRM_MOD(3)::rmbit");
			return;
		}
	}
	switch (regbit) {
	case 0:
		/* reg is operation or segment */
		vcpuins.pr = _GetModRM_REG(modrm);
		break;
	case 8:
		switch(_GetModRM_REG(modrm)) {
		case 0: vcpuins.pr = (t_vaddrcc)(&vcpu.al);break;
		case 1: vcpuins.pr = (t_vaddrcc)(&vcpu.cl);break;
		case 2: vcpuins.pr = (t_vaddrcc)(&vcpu.dl);break;
		case 3: vcpuins.pr = (t_vaddrcc)(&vcpu.bl);break;
		case 4: vcpuins.pr = (t_vaddrcc)(&vcpu.ah);break;
		case 5: vcpuins.pr = (t_vaddrcc)(&vcpu.ch);break;
		case 6: vcpuins.pr = (t_vaddrcc)(&vcpu.dh);break;
		case 7: vcpuins.pr = (t_vaddrcc)(&vcpu.bh);break;
		default:CaseError("_GetModRegRM::regbit(8)::ModRM_REG");return;}
		break;
	case 16:
		switch(_GetModRM_REG(modrm)) {
		case 0: vcpuins.pr = (t_vaddrcc)(&vcpu.ax);break;
		case 1: vcpuins.pr = (t_vaddrcc)(&vcpu.cx);break;
		case 2: vcpuins.pr = (t_vaddrcc)(&vcpu.dx);break;
		case 3: vcpuins.pr = (t_vaddrcc)(&vcpu.bx);break;
		case 4: vcpuins.pr = (t_vaddrcc)(&vcpu.sp);break;
		case 5: vcpuins.pr = (t_vaddrcc)(&vcpu.bp);break;
		case 6: vcpuins.pr = (t_vaddrcc)(&vcpu.si);break;
		case 7: vcpuins.pr = (t_vaddrcc)(&vcpu.di);break;
		default:CaseError("_GetModRegRM::regbit(16)::ModRM_REG");return;}
		break;
	case 32:
		switch(_GetModRM_REG(modrm)) {
		case 0: vcpuins.pr = (t_vaddrcc)(&vcpu.eax);break;
		case 1: vcpuins.pr = (t_vaddrcc)(&vcpu.ecx);break;
		case 2: vcpuins.pr = (t_vaddrcc)(&vcpu.edx);break;
		case 3: vcpuins.pr = (t_vaddrcc)(&vcpu.ebx);break;
		case 4: vcpuins.pr = (t_vaddrcc)(&vcpu.esp);break;
		case 5: vcpuins.pr = (t_vaddrcc)(&vcpu.ebp);break;
		case 6: vcpuins.pr = (t_vaddrcc)(&vcpu.esi);break;
		case 7: vcpuins.pr = (t_vaddrcc)(&vcpu.edi);break;
		default:CaseError("_GetModRegRM::regbit(32)::ModRM_REG");return;}
		break;
	default:
		CaseError("GetModRegRM::regbit");
		return;
	}
}

static t_cpu acpu;
static t_nubit8 ub1,ub2,ub3;
static t_nsbit8 sb1,sb2,sb3;
static t_nubit16 uw1,uw2,uw3;
static t_nsbit16 sw1,sw2,sw3;

#define async     if(0)
#define aexec     __asm
#define acheck(f) if(0)
#define async8    if(0)
#define async12   if(0)
#define async16   if(0)
#define asreg8    if(0)
#define asreg12   if(0)
#define asreg16   if(0)
#define acheck8(f)  if(0)
#define acheck12(f) if(0)
#define acheck16(f) if(0)
#define aexec8    if(0)
#define aexec12   if(0)
#define aexec16   if(0)
#define asyncall
#define aexecall
#define asregall
#define acheckall(f)
#define aexec_csf   if(0)
#define aexec_idc8  if(0)
#define aexec_idc16 if(0)
#define aexec_srd8  if(0)
#define aexec_srd16 if(0)
#define aexec_cax8  if(0)
#define aexec_cax16 if(0)
#define aexec_nxx8  if(0)
#define aexec_nxx16 if(0)
#define aexec_mdx8  if(0)
#define aexec_mdx16 if(0)

static void CalcCF()
{
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
		bugfix(20)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (vcpuins.opr1 < vcpuins.result) ||
				(_GetEFLAGS_CF && (vcpuins.opr2 == 0xff)));
		else
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (vcpuins.opr1 < vcpuins.result) ||
				(vcpuins.opr2 == 0xff));
		break;
	case SBB16:
		bugfix(20)
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (vcpuins.opr1 < vcpuins.result) ||
				(_GetEFLAGS_CF && (vcpuins.opr2 == 0xffff)));
		else
			MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, (vcpuins.opr1 < vcpuins.result) ||
				(vcpuins.opr2 == 0xffff));
		break;
	case SUB8:
	case SUB16:
	case CMP8:
	case CMP16:
		MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,vcpuins.opr1 < vcpuins.opr2);
		break;
	default:CaseError("CalcCF::vcpuins.type");break;}
}
static void CalcOF()
{
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
	default:CaseError("CalcOF::vcpuins.type");break;}
}
static void CalcAF()
{
	MakeBit(vcpu.eflags, VCPU_EFLAGS_AF, ((vcpuins.opr1^vcpuins.opr2)^vcpuins.result) & 0x10);
}
static void CalcPF()
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
static void CalcZF()
{
	MakeBit(vcpu.eflags, VCPU_EFLAGS_ZF, !vcpuins.result);
}
static void CalcSF()
{
	switch(vcpuins.bit) {
	case 8: MakeBit(vcpu.eflags, VCPU_EFLAGS_SF, GetMSB8(vcpuins.result));break;
	case 16:MakeBit(vcpu.eflags, VCPU_EFLAGS_SF, GetMSB16(vcpuins.result));break;
	case 32:MakeBit(vcpu.eflags, VCPU_EFLAGS_SF, GetMSB32(vcpuins.result));break;
	default:CaseError("CalcSF::vcpuins.bit");break;}
}

static void SetFlags(t_nubit16 flags)
{
	if(flags & VCPU_EFLAGS_CF) CalcCF();
	if(flags & VCPU_EFLAGS_PF) CalcPF();
	if(flags & VCPU_EFLAGS_AF) CalcAF();
	if(flags & VCPU_EFLAGS_ZF) CalcZF();
	if(flags & VCPU_EFLAGS_SF) CalcSF();
	if(flags & VCPU_EFLAGS_OF) CalcOF();
}

static void GetMem()
{
	/* returns vcpuins.rm */
	t_cpu_sreg *overds = _GetPtrOverrideDS();
	vcpuins.rm = vramGetRealAddress(overds->selector,vramVarWord(vcpu.cs.selector,vcpu.eip));
	vcpu.eip += 2;
}
static void GetImm(t_nubitcc immbit)
{
	// returns vcpuins.imm
	vcpuins.imm = vramGetRealAddress(vcpu.cs.selector,vcpu.eip);
	switch(immbit) {
	case 8:  vcpu.eip += 1;break;
	case 16: vcpu.eip += 2;break;
	case 32: vcpu.eip += 4;break;
	default: CaseError("GetImm::immbit");break;}
}
static void GetModRegRM(t_nubitcc regbit,t_nubitcc rmbit)
{
	// returns vcpuins.rm and vcpuins.r
	t_nsbit8 disp8 = 0x00;
	t_nubit16 disp16 = 0x0000;
	t_nubit32 disp32 = 0x00000000;
	t_nubit8 modrm = 0x00;
	t_cpu_sreg *overds = _GetPtrOverrideDS();
	t_cpu_sreg *overss = _GetPtrOverrideSS();
	modrm = (t_nubit8)_GetCode(vcpu.eip++, 8);
	vcpuins.rm = vcpuins.r = (t_vaddrcc)NULL;
	switch(_GetModRM_MOD(modrm)) {
	case 0:
		switch(_GetModRM_RM(modrm)) {
		case 0:	vcpuins.rm = vramGetRealAddress(overds->selector,vcpu.bx+vcpu.si);break;
		case 1:	vcpuins.rm = vramGetRealAddress(overds->selector,vcpu.bx+vcpu.di);break;
		case 2:	vcpuins.rm = vramGetRealAddress(overss->selector,vcpu.bp+vcpu.si);break;
		case 3:	vcpuins.rm = vramGetRealAddress(overss->selector,vcpu.bp+vcpu.di);break;
		case 4:	vcpuins.rm = vramGetRealAddress(overds->selector,vcpu.si);break;
		case 5:	vcpuins.rm = vramGetRealAddress(overds->selector,vcpu.di);break;
		case 6: disp16 = vramVarWord(vcpu.cs.selector,vcpu.eip);vcpu.eip += 2;
			    vcpuins.rm = vramGetRealAddress(overds->selector,disp16); break;
		case 7:	vcpuins.rm = vramGetRealAddress(overds->selector,vcpu.bx);break;
		default:CaseError("GetModRegRM::MOD0::ModRM_RM");break;}
		break;
	case 1:
		bugfix(23) {disp8 = (t_nsbit8)vramVarByte(vcpu.cs.selector,vcpu.eip);vcpu.eip += 1;}
		switch(_GetModRM_RM(modrm)) {
		case 0:	vcpuins.rm = vramGetRealAddress(overds->selector,vcpu.bx+vcpu.si+disp8);break;
		case 1:	vcpuins.rm = vramGetRealAddress(overds->selector,vcpu.bx+vcpu.di+disp8);break;
		case 2:	vcpuins.rm = vramGetRealAddress(overss->selector,vcpu.bp+vcpu.si+disp8);break;
		case 3:	vcpuins.rm = vramGetRealAddress(overss->selector,vcpu.bp+vcpu.di+disp8);break;
		case 4:	vcpuins.rm = vramGetRealAddress(overds->selector,vcpu.si+disp8);break;
		case 5:	vcpuins.rm = vramGetRealAddress(overds->selector,vcpu.di+disp8);break;
		case 6:	vcpuins.rm = vramGetRealAddress(overss->selector,vcpu.bp+disp8);break;
		case 7:	vcpuins.rm = vramGetRealAddress(overds->selector,vcpu.bx+disp8);break;
		default:CaseError("GetModRegRM::MOD1::ModRM_RM");break;}
		bugfix(23) ;
		else {
			bugfix(3) vcpuins.rm += (t_nsbit8)vramVarByte(vcpu.cs.selector,vcpu.eip);
			else vcpuins.rm += vramVarByte(vcpu.cs.selector,vcpu.eip);
			vcpu.eip += 1;
		}
		break;
	case 2:
		bugfix(23) {disp16 = vramVarWord(vcpu.cs.selector,vcpu.eip);vcpu.eip += 2;}
		switch(_GetModRM_RM(modrm)) {
		case 0:	vcpuins.rm = vramGetRealAddress(overds->selector,vcpu.bx+vcpu.si+disp16);break;
		case 1:	vcpuins.rm = vramGetRealAddress(overds->selector,vcpu.bx+vcpu.di+disp16);break;
		case 2:	vcpuins.rm = vramGetRealAddress(overss->selector,vcpu.bp+vcpu.si+disp16);break;
		case 3:	vcpuins.rm = vramGetRealAddress(overss->selector,vcpu.bp+vcpu.di+disp16);break;
		case 4:	vcpuins.rm = vramGetRealAddress(overds->selector,vcpu.si+disp16);break;
		case 5:	vcpuins.rm = vramGetRealAddress(overds->selector,vcpu.di+disp16);break;
		case 6:	vcpuins.rm = vramGetRealAddress(overss->selector,vcpu.bp+disp16);break;
		case 7:	vcpuins.rm = vramGetRealAddress(overds->selector,vcpu.bx+disp16);break;
		default:CaseError("GetModRegRM::MOD2::ModRM_RM");break;}
		bugfix(23) ;
		else {vcpuins.rm += vramVarWord(vcpu.cs.selector,vcpu.eip);vcpu.eip += 2;}
		break;
	case 3:
		switch(_GetModRM_RM(modrm)) {
		case 0:	if(rmbit == 8) vcpuins.rm = (t_vaddrcc)(&vcpu.al); else vcpuins.rm = (t_vaddrcc)(&vcpu.ax); break;
		case 1:	if(rmbit == 8) vcpuins.rm = (t_vaddrcc)(&vcpu.cl); else vcpuins.rm = (t_vaddrcc)(&vcpu.cx); break;
		case 2:	if(rmbit == 8) vcpuins.rm = (t_vaddrcc)(&vcpu.dl); else vcpuins.rm = (t_vaddrcc)(&vcpu.dx); break;
		case 3:	if(rmbit == 8) vcpuins.rm = (t_vaddrcc)(&vcpu.bl); else vcpuins.rm = (t_vaddrcc)(&vcpu.bx); break;
		case 4:	if(rmbit == 8) vcpuins.rm = (t_vaddrcc)(&vcpu.ah); else vcpuins.rm = (t_vaddrcc)(&vcpu.sp); break;
		case 5:	if(rmbit == 8) vcpuins.rm = (t_vaddrcc)(&vcpu.ch); else vcpuins.rm = (t_vaddrcc)(&vcpu.bp); break;
		case 6:	if(rmbit == 8) vcpuins.rm = (t_vaddrcc)(&vcpu.dh); else vcpuins.rm = (t_vaddrcc)(&vcpu.si); break;
		case 7:	if(rmbit == 8) vcpuins.rm = (t_vaddrcc)(&vcpu.bh); else vcpuins.rm = (t_vaddrcc)(&vcpu.di); break;
		default:CaseError("GetModRegRM::MOD3::ModRM_RM");break;}
		break;
	default:CaseError("GetModRegRM::ModRM_MOD");break;}
	switch(regbit) {
	case 0:		vcpuins.r = _GetModRM_REG(modrm);					break;
	case 4:
		switch(_GetModRM_REG(modrm)) {
		case 0:	vcpuins.r = (t_vaddrcc)(&vcpu.es.selector);	break;
		case 1:	vcpuins.r = (t_vaddrcc)(&vcpu.cs.selector);	break;
		case 2:	vcpuins.r = (t_vaddrcc)(&vcpu.ss.selector);	break;
		case 3:	vcpuins.r = (t_vaddrcc)(&vcpu.ds.selector);	break;
		case 4:	vcpuins.r = (t_vaddrcc)(&vcpu.fs.selector);	break;
		case 5:	vcpuins.r = (t_vaddrcc)(&vcpu.gs.selector);	break;
		default:CaseError("GetModRegRM::regbit4::ModRM_REG");break;}
		break;
	case 8:
		switch(_GetModRM_REG(modrm)) {
		case 0:	vcpuins.r = (t_vaddrcc)(&vcpu.al);	break;
		case 1:	vcpuins.r = (t_vaddrcc)(&vcpu.cl);	break;
		case 2:	vcpuins.r = (t_vaddrcc)(&vcpu.dl);	break;
		case 3:	vcpuins.r = (t_vaddrcc)(&vcpu.bl);	break;
		case 4:	vcpuins.r = (t_vaddrcc)(&vcpu.ah);	break;
		case 5:	vcpuins.r = (t_vaddrcc)(&vcpu.ch);	break;
		case 6:	vcpuins.r = (t_vaddrcc)(&vcpu.dh);	break;
		case 7:	vcpuins.r = (t_vaddrcc)(&vcpu.bh);	break;
		default:CaseError("GetModRegRM::regbit8::ModRM_REG");break;}
		break;
	case 16:
		switch(_GetModRM_REG(modrm)) {
		case 0: vcpuins.r = (t_vaddrcc)(&vcpu.ax);	break;
		case 1:	vcpuins.r = (t_vaddrcc)(&vcpu.cx);	break;
		case 2:	vcpuins.r = (t_vaddrcc)(&vcpu.dx);	break;
		case 3:	vcpuins.r = (t_vaddrcc)(&vcpu.bx);	break;
		case 4:	vcpuins.r = (t_vaddrcc)(&vcpu.sp);	break;
		case 5:	vcpuins.r = (t_vaddrcc)(&vcpu.bp);	break;
		case 6:	vcpuins.r = (t_vaddrcc)(&vcpu.si);	break;
		case 7: vcpuins.r = (t_vaddrcc)(&vcpu.di);	break;
		default:CaseError("GetModRegRM::regbit16::ModRM_REG");break;}
		break;
	default:CaseError("GetModRegRM::regbit");break;}
}
static void GetModRegRMEA()
{
	t_nsbit8 disp8 = 0x00;
	t_nubit16 disp16 = 0x0000;
	t_nubit8 modrm = vramVarByte(vcpu.cs.selector,vcpu.eip++);
	t_cpu_sreg *overds = _GetPtrOverrideDS();
	t_cpu_sreg *overss = _GetPtrOverrideSS();
	vcpuins.rm = vcpuins.r = (t_vaddrcc)NULL;
	switch(_GetModRM_MOD(modrm)) {
	case 0:
		switch(_GetModRM_RM(modrm)) {
		case 0:	vcpuins.rm = vcpu.bx+vcpu.si;break;
		case 1:	vcpuins.rm = vcpu.bx+vcpu.di;break;
		case 2:	vcpuins.rm = vcpu.bp+vcpu.si;break;
		case 3:	vcpuins.rm = vcpu.bp+vcpu.di;break;
		case 4:	vcpuins.rm = vcpu.si;break;
		case 5:	vcpuins.rm = vcpu.di;break;
		case 6:	vcpuins.rm = vramVarWord(vcpu.cs.selector,vcpu.eip);vcpu.eip += 2;break;
		case 7:	vcpuins.rm = vcpu.bx;break;
		default:CaseError("GetModRegRMEA::MOD0::ModRM_RM");break;}
		break;
	case 1:
		bugfix(23) {disp8 = (t_nsbit8)vramVarByte(vcpu.cs.selector,vcpu.eip);vcpu.eip += 1;}
		switch(_GetModRM_RM(modrm)) {
		case 0:	vcpuins.rm = vcpu.bx+vcpu.si+disp8;break;
		case 1:	vcpuins.rm = vcpu.bx+vcpu.di+disp8;break;
		case 2:	vcpuins.rm = vcpu.bp+vcpu.si+disp8;break;
		case 3:	vcpuins.rm = vcpu.bp+vcpu.di+disp8;break;
		case 4:	vcpuins.rm = vcpu.si+disp8;break;
		case 5:	vcpuins.rm = vcpu.di+disp8;break;
		case 6:	vcpuins.rm = vcpu.bp+disp8;break;
		case 7:	vcpuins.rm = vcpu.bx+disp8;break;
		default:CaseError("GetModRegRMEA::MOD1::ModRM_RM");break;}
		bugfix(23) {vcpuins.rm %= 0x10000;}
		else {
			bugfix(3) vcpuins.rm += (t_nsbit8)vramVarByte(vcpu.cs.selector,vcpu.eip);
			else vcpuins.rm += vramVarByte(vcpu.cs.selector,vcpu.eip);
			vcpu.eip += 1;
		}
		break;
	case 2:
		bugfix(23) {disp16 = vramVarWord(vcpu.cs.selector,vcpu.eip);vcpu.eip += 2;}
		switch(_GetModRM_RM(modrm)) {
		case 0:	vcpuins.rm = vcpu.bx+vcpu.si+disp16;break;
		case 1:	vcpuins.rm = vcpu.bx+vcpu.di+disp16;break;
		case 2:	vcpuins.rm = vcpu.bp+vcpu.si+disp16;break;
		case 3:	vcpuins.rm = vcpu.bp+vcpu.di+disp16;break;
		case 4:	vcpuins.rm = vcpu.si+disp16;break;
		case 5:	vcpuins.rm = vcpu.di+disp16;break;
		case 6:
			bugfix(14) vcpuins.rm = vcpu.bp+disp16;
			else vcpuins.rm = vramGetRealAddress(overss->selector,vcpu.bp);
			break;
		case 7:	vcpuins.rm = vcpu.bx+disp16;break;
		default:CaseError("GetModRegRMEA::MOD2::ModRM_RM");break;}
		bugfix(23) {vcpuins.rm %= 0x10000;}
		else {vcpuins.rm += vramVarWord(vcpu.cs.selector,vcpu.eip);vcpu.eip += 2;}
		break;
	default:CaseError("GetModRegRMEA::ModRM_MOD");break;}
	switch(_GetModRM_REG(modrm)) {
	case 0: vcpuins.r = (t_vaddrcc)(&vcpu.ax);	break;
	case 1:	vcpuins.r = (t_vaddrcc)(&vcpu.cx);	break;
	case 2:	vcpuins.r = (t_vaddrcc)(&vcpu.dx);	break;
	case 3:	vcpuins.r = (t_vaddrcc)(&vcpu.bx);	break;
	case 4:	vcpuins.r = (t_vaddrcc)(&vcpu.sp);	break;
	case 5:	vcpuins.r = (t_vaddrcc)(&vcpu.bp);	break;
	case 6:	vcpuins.r = (t_vaddrcc)(&vcpu.si);	break;
	case 7: vcpuins.r = (t_vaddrcc)(&vcpu.di);	break;
	default:CaseError("GetModRegRMEA::ModRM_REG");break;}
}

#define DONE static void
#define TODO static void

static void INT(t_nubit8 intid);
DONE _Push(t_vaddrcc psrc, t_nubit8 bit)
{
	t_nubit16 data16;
	t_nubit32 data32;
	t_nubit32 physical = 0x00000000;
	switch (bit) {
	case 16:
		if (vcpu.esp < 0x0000002) {
			_SetExcept_SS(0);
			_CheckExcept("_Push::bit(16)::esp(<2)");
			return;
		}
		data16 = d_nubit16(psrc);
		physical = _GetPhysicalOfSegment(&vcpu.ss, vcpu.esp - 2, 16, 0x01, 0x00);
		_CheckExcept("_Push::bit(16)::_GetPhysicalOfSegment");
		vcpu.esp -= 2;
		vramWord(physical) = data16;
		break;
	case 32:
		if (vcpu.esp < 0x0000004) {
			_SetExcept_SS(0);
			_CheckExcept("_Push::bit(32)::esp(<4)");
			return;
		}
		data32 = d_nubit32(psrc);
		physical = _GetPhysicalOfSegment(&vcpu.ss, vcpu.esp - 4, 32, 0x01, 0x00);
		_CheckExcept("_Push::bit(32)::_GetPhysicalOfSegment");
		vcpu.esp -= 4;
		vramDWord(physical) = data32;
		break;
	default:CaseError("_Push::bit");break;}
}
DONE _Call_Near(t_nubit32 neweip, t_nubit8 bit)
{
	switch (bit) {
	case 16:
		_Push((t_vaddrcc)&vcpu.ip, 16);
		_CheckExcept("_Call_Near::bit(16)::_Push");
		vcpu.eip = GetMax16(neweip);
		break;
	case 32:
		_Push((t_vaddrcc)&vcpu.eip, 32);
		_CheckExcept("_Call_Near::bit(32)::_Push");
		vcpu.eip = GetMax32(neweip);
		break;
	default:
		CaseError("_Call_Near::bit");
		return;
	}
}
DONE _Call_Far(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 bit)
{
	t_nubit32 oldcs = vcpu.cs.selector;
	t_nubit64 csdesc = 0x0000000000000000, gatedesc = 0x0000000000000000;
	t_nubit16 cssel = newcs, gatesel = newcs;
	if (_GetCR0_PE && !_GetEFLAGS_VM) {
		csdesc = gatedesc = _GetDescriptor(newcs);
		_CheckExcept("_Call_Far::PROTECTED::cssel(null)");
		if (_GetSegDesc_S(csdesc)) {
			/* conforming or nonconforming code segment */
			if (!_GetSegDesc_TYPE_CD(csdesc)) {
				_SetExcept_GP(cssel);
				_CheckExcept("_Call_Far::PROTECTED::DATA_SEGMENT");
			}
			if (_GetSegDesc_TYPE_C(csdesc)) {
				/* conforming code segment */
				if (_GetSegDesc_DPL(csdesc) > _GetCPL) {
					_SetExcept_GP(cssel);
					_CheckExcept("_Call_Far::PROTECTED::Descriptor_S(1)::Descriptor_TYPE_C(1)::?PL");
				}
			} else {
				/* nonconforming code segment */
				if (_GetSelector_RPL(cssel) > _GetCPL || _GetSegDesc_DPL(csdesc) != _GetCPL) {
					_SetExcept_GP(cssel);
					_CheckExcept("_Call_Far::PROTECTED::Descriptor_S(1)::Descriptor_TYPE_C(0)::?PL");
				}
			}
			if (!_GetSegDesc_P(csdesc)) {
				_SetExcept_NP(cssel);
				_CheckExcept("_Call_Far::PROTECTED::Descriptor_S(1)::Descriptor_P(0)");
			}
		} else {
			/* call gate, task gate or task state segment */
			switch (_GetGateDesc_TYPE(gatedesc)) {
			case 0x01: /* 16 tss available*/
			case 0x03: /* 16 tss busy*/
			case 0x09: /* 32 tss available*/
			case 0x0b: /* 32 tss busy*/
				break;
			case 0x04: /* 16 call gate */
			case 0x0c: /* 32 call gate */
				if (_GetGateDesc_DPL(gatedesc) < _GetCPL || _GetGateDesc_DPL(gatedesc) < _GetSelector_RPL(gatesel)) {
					_SetExcept_GP(gatesel);
					_CheckExcept("_Call_Far::PROTECTED::CALL_GATE::gatedesc(?PL)");
				}
				if (!_GetGateDesc_P(gatedesc)) {
					_SetExcept_NP(gatesel);
					_CheckExcept("_Call_Far::PROTECTED::CALL_GATE::gatedesc(!P)");
				}
				cssel = _GetGateDesc_SELECTOR(gatedesc);
				if (_IsSelectorNull(cssel)) {
					_SetExcept_GP(0);
					_CheckExcept("_Call_Far::PROTECTED::CALL_GATE::cssel(null)");
				}
				csdesc = _GetDescriptor(cssel);
				_CheckExcept("_Call_Far::PROTECTED::CALL_GATE::_GetDescriptor");
				if (!_GetSegDesc_S(csdesc) || !_GetSegDesc_TYPE_CD(csdesc) || _GetSegDesc_DPL(csdesc) > _GetCPL) {
					_SetExcept_GP(cssel);
					_CheckExcept("_Call_Far::PROTECTED::CALL_GATE::csdesc(!S/!CD/?PL)");
				}
				if (!_GetSegDesc_P(csdesc)) {
					_SetExcept_NP(cssel);
					_CheckExcept("_Call_Far::PROTECTED::CALL_GATE::csdesc(!P)");
				}
				if (!_GetSegDesc_TYPE_C(csdesc) && _GetSegDesc_DPL(csdesc) < _GetCPL) {
					/* more-privilege */
					//////// TODO /////////////
				} else {
					/* same privilege */
				}
				break;
			case 0x05: /* task gate */
				break;
			case 0x02: /* ldt */
			case 0x06: /* 16 interrupt gate */
			case 0x0e: /* 32 interrupt gate */
			case 0x07: /* 16 trap gate */
			case 0x0f: /* 32 trap gate */
			case 0x00: /* reserved */
			case 0x08: /* reserved */
			case 0x0a: /* reserved */
			case 0x0d: /* reserved */
			default:
				_SetExcept_GP(newcs);
				_CheckExcept("_Call_Far::PROTECTED::Descriptor_S(0)::Descriptor_TYPE");
				break;
			}
		}
	}
	switch (bit) {
	case 16:
		_CheckSegment(&vcpu.ss, vcpu.esp - 4, 32, 0x01);
		_CheckExcept("_Call_Far::bit(16)::_CheckSegment(ss)");
		_CheckSegment(&vcpu.cs, vcpu.eip, 8, 0x00);
		_CheckExcept("_Call_Far::bit(16)::_CheckSegment(cs/old)");
		_Push((t_vaddrcc)&oldcs, 16);
		_CheckExcept("_Call_Far::bit(16)::_Push(cs)");
		_Push((t_vaddrcc)&vcpu.ip, 16);
		_CheckExcept("_Call_Far::bit(16)::_Push(ip)");
		_LoadSegment(&vcpu.cs, newcs);
		_CheckExcept("_Call_Far::bit(16)::_LoadSegment(cs)");
		vcpu.eip = GetMax16(neweip);
		_CheckSegment(&vcpu.cs, vcpu.eip, 8, 0x00);
		_CheckExcept("_Call_Far::bit(16)::_CheckSegment(cs/new)");
		break;
	case 32:
		_CheckSegment(&vcpu.ss, vcpu.esp - 6, 48, 0x01);
		_CheckExcept("_Call_Far::bit(32)::_CheckSegment(ss)");
		_CheckSegment(&vcpu.cs, vcpu.eip, 8, 0x00);
		_CheckExcept("_Call_Far::bit(32)::_CheckSegment(cs/old)");
		_Push((t_vaddrcc)&oldcs, 32);
		_CheckExcept("_Call_Far::bit(32)::_Push(cs)");
		_Push((t_vaddrcc)&vcpu.eip, 32);
		_CheckExcept("_Call_Far::bit(32)::_Push(eip)");
		_LoadSegment(&vcpu.cs, newcs);
		_CheckExcept("_Call_Far::bit(32)::_LoadSegment(cs)");
		vcpu.eip = GetMax32(neweip);
		_CheckSegment(&vcpu.cs, vcpu.eip, 8, 0x00);
		_CheckExcept("_Call_Far::bit(32)::_CheckSegment(cs/new)");
		break;
	default:
		CaseError("_Call_Far::bit");
		return;
	}
}

DONE ADD(t_vaddrcc pdest, t_vaddrcc psrc, t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = ADD8;
		vcpuins.opr1 = GetMax8(d_nubit8(pdest));
		vcpuins.opr2 = GetMax8(d_nubit8(psrc));
		vcpuins.result = GetMax8(vcpuins.opr1 + vcpuins.opr2);
		d_nubit8(pdest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = ADD16;
		vcpuins.opr1 = GetMax16(d_nubit16(pdest));
		vcpuins.opr2 = GetMax16(d_nsbit8(psrc));
		vcpuins.result = GetMax16(vcpuins.opr1 + vcpuins.opr2);
		d_nubit16(pdest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = ADD16;
		vcpuins.opr1 = GetMax16(d_nubit16(pdest));
		vcpuins.opr2 = GetMax16(d_nubit16(psrc));
		vcpuins.result = GetMax16(vcpuins.opr1 + vcpuins.opr2);
		d_nubit16(pdest) = (t_nubit16)vcpuins.result;
		break;
	case 20:
		vcpuins.bit = 32;
		vcpuins.type = ADD32;
		vcpuins.opr1 = GetMax32(d_nubit32(pdest));
		vcpuins.opr2 = GetMax32(d_nsbit8(psrc));
		vcpuins.result = GetMax32(vcpuins.opr1 + vcpuins.opr2);
		d_nubit32(pdest) = (t_nubit32)vcpuins.result;
		break;
	case 32:
		vcpuins.bit = 32;
		vcpuins.type = ADD32;
		vcpuins.opr1 = GetMax32(d_nubit32(pdest));
		vcpuins.opr2 = GetMax32(d_nubit32(psrc));
		vcpuins.result = GetMax32(vcpuins.opr1 + vcpuins.opr2);
		d_nubit32(pdest) = (t_nubit32)vcpuins.result;
		break;
	default:CaseError("ADD::len");break;}
	SetFlags(ADD_FLAG);
}
static ASMCMP OR(void *dest, void *src, t_nubit8 len)
{
	asyncall
	switch(len) {
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
	default:CaseError("OR::len");break;}
	_ClrOF;
	_ClrEFLAGS_CF;
	_ClrEFLAGS_AF;
	SetFlags(OR_FLAG);
#define op or
	aexecall
#undef op
	asregall
	acheckall(AFLAGS1)
}
DONE ADC(t_vaddrcc pdest, t_vaddrcc psrc, t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = ADC8;
		vcpuins.opr1 = GetMax8(d_nubit8(pdest));
		vcpuins.opr2 = GetMax8(d_nubit8(psrc));
		vcpuins.result = GetMax8(vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF);
		d_nubit8(pdest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		vcpuins.type = ADC16;
		vcpuins.opr1 = GetMax16(d_nubit16(pdest));
		vcpuins.opr2 = GetMax16(d_nsbit8(psrc));
		vcpuins.result = GetMax16(vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF);
		d_nubit16(pdest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = ADC16;
		vcpuins.opr1 = GetMax16(d_nubit16(pdest));
		vcpuins.opr2 = GetMax16(d_nubit16(psrc));
		vcpuins.result = GetMax16(vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF);
		d_nubit16(pdest) = (t_nubit16)vcpuins.result;
		break;
	case 20:
		vcpuins.bit = 32;
		vcpuins.type = ADC32;
		vcpuins.opr1 = GetMax32(d_nubit32(pdest));
		vcpuins.opr2 = GetMax32(d_nsbit8(psrc));
		vcpuins.result = GetMax32(vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF);
		d_nubit32(pdest) = (t_nubit32)vcpuins.result;
		break;
	case 32:
		vcpuins.bit = 32;
		vcpuins.type = ADC32;
		vcpuins.opr1 = GetMax32(d_nubit32(pdest));
		vcpuins.opr2 = GetMax32(d_nubit32(psrc));
		vcpuins.result = GetMax32(vcpuins.opr1 + vcpuins.opr2 + _GetEFLAGS_CF);
		d_nubit32(pdest) = (t_nubit32)vcpuins.result;
		break;
	default:CaseError("ADC::len");break;}
	SetFlags(ADC_FLAG);
}
static ASMCMP SBB(void *dest, void *src, t_nubit8 len)
{
	asyncall
	switch(len) {
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
	default:CaseError("SBB::len");break;}
	SetFlags(SBB_FLAG);
#define op sbb
	aexecall
#undef op
	asregall
	acheckall(AFLAGS1)
}
DONE AND(t_vaddrcc pdest, t_vaddrcc psrc, t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		//vcpuins.type = AND8;
		vcpuins.opr1 = GetMax8(d_nubit8(pdest));
		vcpuins.opr2 = GetMax8(d_nubit8(psrc));
		vcpuins.result = GetMax8(vcpuins.opr1 & vcpuins.opr2);
		d_nubit8(pdest) = (t_nubit8)vcpuins.result;
		break;
	case 12:
		vcpuins.bit = 16;
		//vcpuins.type = AND16;
		vcpuins.opr1 = GetMax16(d_nubit16(pdest));
		vcpuins.opr2 = GetMax16(d_nsbit8(psrc));
		vcpuins.result = GetMax16(vcpuins.opr1 & vcpuins.opr2);
		d_nubit16(pdest) = (t_nubit16)vcpuins.result;
		break;
	case 16:
		vcpuins.bit = 16;
		//vcpuins.type = AND16;
		vcpuins.opr1 = GetMax16(d_nubit16(pdest));
		vcpuins.opr2 = GetMax16(d_nubit16(psrc));
		vcpuins.result = GetMax16(vcpuins.opr1 & vcpuins.opr2);
		d_nubit16(pdest) = (t_nubit16)vcpuins.result;
		break;
	case 20:
		vcpuins.bit = 32;
		//vcpuins.type = AND32;
		vcpuins.opr1 = GetMax32(d_nubit32(pdest));
		vcpuins.opr2 = GetMax32(d_nsbit8(psrc));
		vcpuins.result = GetMax32(vcpuins.opr1 & vcpuins.opr2);
		d_nubit32(pdest) = (t_nubit32)vcpuins.result;
		break;
	case 32:
		vcpuins.bit = 32;
		//vcpuins.type = AND32;
		vcpuins.opr1 = GetMax32(d_nubit32(pdest));
		vcpuins.opr2 = GetMax32(d_nubit32(psrc));
		vcpuins.result = GetMax32(vcpuins.opr1 & vcpuins.opr2);
		d_nubit32(pdest) = (t_nubit32)vcpuins.result;
		break;
	default:CaseError("AND::len");break;}
	_ClrOF;
	_ClrEFLAGS_CF;
	_ClrEFLAGS_AF;
	SetFlags(AND_FLAG);
}
static void SUB(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
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
	default:CaseError("SUB::len");break;}
	SetFlags(SUB_FLAG);
}
static ASMCMP XOR(void *dest, void *src, t_nubit8 len)
{
	asyncall
	switch(len) {
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
	default:CaseError("XOR::len");break;}
	_ClrOF;
	_ClrEFLAGS_CF;
	_ClrEFLAGS_AF;
	SetFlags(XOR_FLAG);
#define op xor
	aexecall
#undef op
	asregall
	acheckall(AFLAGS1)
}
static ASMCMP CMP(void *dest, void *src, t_nubit8 len)
{
	asyncall
	switch(len) {
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
	default:CaseError("CMP::len");break;}
	SetFlags(CMP_FLAG);
#define op cmp
	aexecall
#undef op
	asregall
	acheckall(AFLAGS1)
}
static void PUSH(void *src, t_nubit8 len)
{
	t_nubit16 data16;
	t_nubit32 data32;
	switch(len) {
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
	default:CaseError("PUSH::len");break;}
}
static void POP(void *dest, t_nubit8 len)
{
	switch(len) {
	case 16:
		vcpuins.bit = 16;
		d_nubit16(dest) = vramVarWord(vcpu.ss.selector,vcpu.sp);
		vcpu.sp += 2;
		break;
	default:CaseError("POP::len");break;}
}
static ASMCMP INC(void *dest, t_nubit8 len)
{
	switch(len) {
	case 8:
		async ub1 = d_nubit8(dest);
#define op inc
		aexec_idc8;
#undef op
		vcpuins.bit = 8;
		vcpuins.type = ADD8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = 0x01;
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2) & 0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 16:
		async uw1 = d_nubit16(dest);
#define op inc
		aexec_idc16;
#undef op
		vcpuins.bit = 16;
		vcpuins.type = ADD16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = 0x0001;
		vcpuins.result = (vcpuins.opr1+vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("INC::len");break;}
	SetFlags(INC_FLAG);
	asregall
	acheckall(AFLAGS1)
}
static ASMCMP DEC(void *dest, t_nubit8 len)
{
	switch(len) {
	case 8:
		async ub1 = d_nubit8(dest);
#define op dec
		aexec_idc8;
#undef op
		vcpuins.bit = 8;
		vcpuins.type = SUB8;
		vcpuins.opr1 = d_nubit8(dest) & 0xff;
		vcpuins.opr2 = 0x01;
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;
		d_nubit8(dest) = (t_nubit8)vcpuins.result;
		break;
	case 16:
		async uw1 = d_nubit16(dest);
#define op dec
		aexec_idc16;
#undef op
		vcpuins.bit = 16;
		vcpuins.type = SUB16;
		vcpuins.opr1 = d_nubit16(dest) & 0xffff;
		vcpuins.opr2 = 0x0001;
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;
		d_nubit16(dest) = (t_nubit16)vcpuins.result;
		break;
	default:CaseError("DEC::len");break;}
	SetFlags(DEC_FLAG);
	asregall
	acheckall(AFLAGS1)
}
static void JCC(void *src, t_bool flagj,t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		if(flagj)
			bugfix(5) vcpu.eip += d_nsbit8(src);
			else vcpu.eip += d_nubit8(src);
		break;
	default:CaseError("JCC::len");break;}
}
static ASMCMP TEST(void *dest, void *src, t_nubit8 len)
{
	asyncall
	switch(len) {
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
	default:CaseError("TEST::len");break;}
	_ClrOF;
	_ClrEFLAGS_CF;
	_ClrEFLAGS_AF;
	SetFlags(TEST_FLAG);
#define op test
	aexecall
#undef op
	acheckall(AFLAGS1)
}
static void XCHG(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
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
	default:CaseError("XCHG::len");break;}
}
static void MOV(void *dest, void *src, t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		d_nubit8(dest) = d_nubit8(src);
		break;
	case 16:
		vcpuins.bit = 16;
		d_nubit16(dest) = d_nubit16(src);
		break;
	default:CaseError("MOV::len");break;}
}
static ASMCMP ROL(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 8;
		vcpuins.bit = 8;
		while(tempcount) {
			tempCF = GetMSB(d_nubit8(dest), 8);
			d_nubit8(dest) = (d_nubit8(dest)<<1)+(t_nubit8)tempCF;
			tempcount--;
		}
		MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetLSB(d_nubit8(dest), 8));
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^_GetEFLAGS_CF);
#define op rol
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 16;
		vcpuins.bit = 16;
		while(tempcount) {
			tempCF = GetMSB(d_nubit16(dest), 16);
			d_nubit16(dest) = (d_nubit16(dest)<<1)+(t_nubit16)tempCF;
			tempcount--;
		}
		MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetLSB(d_nubit16(dest), 16));
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^_GetEFLAGS_CF);
#define op rol
		aexec_srd16;
#undef op
		break;
	default:CaseError("ROL::len");break;}
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static ASMCMP ROR(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
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
#define op ror
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
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
#define op ror
		aexec_srd16;
#undef op
		break;
	default:CaseError("ROR::len");break;}
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static ASMCMP RCL(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 9;
		vcpuins.bit = 8;
		while(tempcount) {
			tempCF = GetMSB(d_nubit8(dest), 8);
			d_nubit8(dest) = (d_nubit8(dest)<<1)+_GetEFLAGS_CF;
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,tempCF);
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^_GetEFLAGS_CF);
#define op rcl
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		tempcount = (count & 0x1f) % 17;
		vcpuins.bit = 16;
		while(tempcount) {
			tempCF = GetMSB(d_nubit16(dest), 16);
			d_nubit16(dest) = (d_nubit16(dest)<<1)+_GetEFLAGS_CF;
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,tempCF);
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^_GetEFLAGS_CF);
#define op rcl
		aexec_srd16;
#undef op
		break;
	default:CaseError("RCL::len");break;}
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static ASMCMP RCR(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	t_bool tempCF;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
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
#define op rcr
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
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
#define op rcr
		aexec_srd16;
#undef op
		break;
	default:CaseError("RCR::len");break;}
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static ASMCMP SHL(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		vcpuins.bit = 8;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetMSB(d_nubit8(dest), 8));
			d_nubit8(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^_GetEFLAGS_CF);
		bugfix(8) {
			if(count != 0) {
				vcpuins.result = d_nubit8(dest);
				SetFlags(SHL_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				vcpuins.result = d_nubit8(dest);
				SetFlags(SHL_FLAG);
			}
		}
#define op shl
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		vcpuins.bit = 16;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetMSB(d_nubit16(dest), 16));
			d_nubit16(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^_GetEFLAGS_CF);
		bugfix(8) {
			if(count != 0) {
				vcpuins.result = d_nubit16(dest);
				SetFlags(SHL_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				vcpuins.result = d_nubit16(dest);
				SetFlags(SHL_FLAG);
			}
		}
#define op shl
		aexec_srd16;
#undef op
		break;
	default:CaseError("SHL::len");break;}
	if (count) _ClrEFLAGS_AF;
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static ASMCMP SHR(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount,tempdest8;
	t_nubit16 tempdest16;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		vcpuins.bit = 8;
		tempcount = count&0x1f;
		tempdest8 = d_nubit8(dest);
		while(tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetLSB(d_nubit8(dest), 8));
			d_nubit8(dest) >>= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,!!(tempdest8&0x80));
		bugfix(8) {
			if(count != 0) {
				vcpuins.result = d_nubit8(dest);
				SetFlags(SHR_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				vcpuins.result = d_nubit8(dest);
				SetFlags(SHR_FLAG);
			}
		}
#define op shr
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		vcpuins.bit = 16;
		tempcount = count&0x1f;
		tempdest16 = d_nubit16(dest);
		while(tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetLSB(d_nubit16(dest), 16));
			d_nubit16(dest) >>= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,!!(tempdest16&0x8000));
		bugfix(8) {
			if(count != 0) {
				vcpuins.result = d_nubit16(dest);
				SetFlags(SHR_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				vcpuins.result = d_nubit16(dest);
				SetFlags(SHR_FLAG);
			}
		}
#define op shr
		aexec_srd16;
#undef op
		break;
	default:CaseError("SHR::len");break;}
	if (count) _ClrEFLAGS_AF;
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static ASMCMP SAL(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		vcpuins.bit = 8;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetMSB(d_nubit8(dest), 8));
			d_nubit8(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit8(dest), 8)^_GetEFLAGS_CF);
		bugfix(8) {
			if(count != 0) {
				vcpuins.result = d_nubit8(dest);
				SetFlags(SAL_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				vcpuins.result = d_nubit8(dest);
				SetFlags(SAL_FLAG);
			}
		}
#define op sal
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		vcpuins.bit = 16;
		tempcount = count&0x1f;
		while(tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetMSB(d_nubit16(dest), 16));
			d_nubit16(dest) <<= 1;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,GetMSB(d_nubit16(dest), 16)^_GetEFLAGS_CF);
		bugfix(8) {
			if(count != 0) {
				vcpuins.result = d_nubit16(dest);
				SetFlags(SAL_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				vcpuins.result = d_nubit16(dest);
				SetFlags(SAL_FLAG);
			}
		}
#define op sal
		aexec_srd16;
#undef op
		break;
	default:CaseError("SAL::len");break;}
	if (count) _ClrEFLAGS_AF;
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static ASMCMP SAR(void *dest, void *src, t_nubit8 len)
{
	t_nubit8 count,tempcount,tempdest8;
	t_nubit16 tempdest16;
	if(src) count = d_nubit8(src);
	else count = 1;
	switch(len) {
	case 8:
		async {
			ub1 = d_nubit8(dest);
			ub2 = count;
		}
		vcpuins.bit = 8;
		tempcount = count&0x1f;
		tempdest8 = d_nubit8(dest);
		while(tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetLSB(d_nubit8(dest), 8));
			d_nsbit8(dest) >>= 1;
			//d_nubit8(dest) |= tempdest8&0x80;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,0);
		bugfix(8) {
			if(count != 0) {
				vcpuins.result = d_nsbit8(dest);
				SetFlags(SAR_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				vcpuins.result = d_nsbit8(dest);
				SetFlags(SAR_FLAG);
			}
		}
#define op sar
		aexec_srd8;
#undef op
		break;
	case 16:
		async {
			uw1 = d_nubit16(dest);
			ub2 = count;
		}
		vcpuins.bit = 16;
		tempcount = count&0x1f;
		tempdest16 = d_nubit16(dest);
		while(tempcount) {
			MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,GetLSB(d_nubit16(dest), 16));
			d_nsbit16(dest) >>= 1;
			//d_nubit16(dest) |= tempdest16&0x8000;
			tempcount--;
		}
		if(count == 1) MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,0);
		bugfix(8) {
			if(count != 0) {
				vcpuins.result = d_nsbit16(dest);
				SetFlags(SAR_FLAG);
			}
		} else {
			if(count != 1 && count != 0) {
				vcpuins.result = d_nsbit16(dest);
				SetFlags(SAR_FLAG);
			}
		}
#define op sar
		aexec_srd16;
#undef op
		break;
	default:CaseError("SAR::len");break;}
	if (count) _ClrEFLAGS_AF;
	asregall
	acheckall((count == 1) ? AFLAGS1 : AFLAGS2)
}
static void STRDIR(t_nubit8 len, t_bool flagsi, t_bool flagdi)
{
	bugfix(10) {
		/* add parameters flagsi, flagdi */
	} else ;
	switch(len) {
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
	default:CaseError("STRDIR::len");break;}
}
static void MOVS(t_nubit8 len)
{
	t_cpu_sreg *overds = _GetPtrOverrideDS();
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vramVarByte(vcpu.es.selector,vcpu.di) = vramVarByte(overds->selector,vcpu.si);
		STRDIR(8,1,1);
		// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOVSB\n");
		break;
	case 16:
		vcpuins.bit = 16;
		vramVarWord(vcpu.es.selector,vcpu.di) = vramVarWord(overds->selector,vcpu.si);
		STRDIR(16,1,1);
		// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOVSW\n");
		break;
	default:CaseError("MOVS::len");break;}
	//qdcgaCheckVideoRam(vramGetRealAddress(vcpu.es.selector, vcpu.di));
}
static void CMPS(t_nubit8 len)
{
	t_cpu_sreg *overds = _GetPtrOverrideDS();
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = CMP8;
		vcpuins.opr1 = vramVarByte(overds->selector,vcpu.si);
		vcpuins.opr2 = vramVarByte(vcpu.es.selector,vcpu.di);
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;
		STRDIR(8,1,1);
		SetFlags(CMP_FLAG);
		// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  CMPSB\n");
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		vcpuins.opr1 = vramVarWord(overds->selector,vcpu.si);
		vcpuins.opr2 = vramVarWord(vcpu.es.selector,vcpu.di);
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;
		STRDIR(16,1,1);
		SetFlags(CMP_FLAG);
		// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  CMPSW\n");
		break;
	default:CaseError("CMPS::len");break;}
}
static void STOS(t_nubit8 len)
{
	switch(len) {
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
	default:CaseError("STOS::len");break;}
}
static void LODS(t_nubit8 len)
{
	t_cpu_sreg *overds = _GetPtrOverrideDS();
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpu.al = vramVarByte(overds->selector,vcpu.si);
		STRDIR(8,1,0);
		// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  LODSB\n");
		break;
	case 16:
		vcpuins.bit = 16;
		vcpu.ax = vramVarWord(overds->selector,vcpu.si);
		STRDIR(16,1,0);
		// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  LODSW\n");
		break;
	default:CaseError("LODS::len");break;}
}
static void SCAS(t_nubit8 len)
{
	switch(len) {
	case 8:
		vcpuins.bit = 8;
		vcpuins.type = CMP8;
		vcpuins.opr1 = vcpu.al;
		vcpuins.opr2 = vramVarByte(vcpu.es.selector,vcpu.di);
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xff;
		STRDIR(8,0,1);
		SetFlags(CMP_FLAG);
		break;
	case 16:
		vcpuins.bit = 16;
		vcpuins.type = CMP16;
		vcpuins.opr1 = vcpu.ax;
		vcpuins.opr2 = vramVarWord(vcpu.es.selector,vcpu.di);
		vcpuins.result = (vcpuins.opr1-vcpuins.opr2)&0xffff;
		STRDIR(16,0,1);
		SetFlags(CMP_FLAG);
		break;
	default:CaseError("SCAS::len");break;}
}
static ASMCMP NOT(void *dest, t_nubit8 len)
{
	switch(len) {
	case 8:
		async ub1 = d_nubit8(dest);
		vcpuins.bit = 8;
		d_nubit8(dest) = ~d_nubit8(dest);
#define op not
		aexec_nxx8;
#undef op
		break;
	case 16:
		async uw1 = d_nubit16(dest);
		vcpuins.bit = 16;
		d_nubit16(dest) = ~d_nubit16(dest);
#define op not
		aexec_nxx16;
#undef op
		break;
	default:CaseError("NOT::len");break;}
	asregall
	acheckall(AFLAGS1)
}
static ASMCMP NEG(void *dest, t_nubit8 len)
{
	t_nubitcc zero = 0;
	switch(len) {
	case 8:	
		async ub1 = d_nubit8(dest);
		vcpuins.bit = 8;
		SUB((void *)&zero,(void *)dest,8);
		d_nubit8(dest) = (t_nubit8)zero;
#define op neg
		aexec_nxx8;
#undef op
		break;
	case 16:
		async uw1 = d_nubit16(dest);
		vcpuins.bit = 16;
		SUB((void *)&zero,(void *)dest,16);
		d_nubit16(dest) = (t_nubit16)zero;
#define op neg
		aexec_nxx16;
#undef op
		break;
	default:CaseError("NEG::len");break;}
	asregall
	acheckall(AFLAGS1)
}
static ASMCMP MUL(void *src, t_nubit8 len)
{
	t_nubit32 tempresult;
	switch(len) {
	case 8:
		async ub2 = d_nubit8(src);
		vcpuins.bit = 8;
		vcpu.ax = vcpu.al * d_nubit8(src);
		MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,!!vcpu.ah);
		MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,!!vcpu.ah);
#define op mul
		aexec_mdx8;
#undef op
		break;
	case 16:
		async uw2 = d_nubit16(src);
		vcpuins.bit = 16;
		tempresult = vcpu.ax * d_nubit16(src);
		vcpu.dx = (tempresult>>16)&0xffff;
		vcpu.ax = tempresult&0xffff;
		MakeBit(vcpu.eflags,VCPU_EFLAGS_OF,!!vcpu.dx);
		MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,!!vcpu.dx);
#define op mul
		aexec_mdx16;
#undef op
		break;
	default:CaseError("MUL::len");break;}
	acheck(VCPU_EFLAGS_OF | VCPU_EFLAGS_CF) vapiPrintIns(vcpu.cs.selector,vcpu.eip,"+MUL");
}
static ASMCMP IMUL(void *src, t_nubit8 len)
{
	t_nsbit32 tempresult;
	switch(len) {
	case 8:
		async ub2 = d_nubit8(src);
		vcpuins.bit = 8;
		vcpu.ax = (t_nsbit8)vcpu.al * d_nsbit8(src);
		if(vcpu.ax == vcpu.al) {
			_ClrOF;
			_ClrEFLAGS_CF;
		} else {
			_SetOF;
			_SetEFLAGS_CF;
		}
#define op imul
		aexec_mdx8;
#undef op
		acheck(VCPU_EFLAGS_OF | VCPU_EFLAGS_CF) vapiPrintIns(vcpu.cs.selector,vcpu.eip,"+IMUL");
		break;
	case 16:
		async uw2 = d_nubit16(src);
		vcpuins.bit = 16;
		tempresult = (t_nsbit16)vcpu.ax * d_nsbit16(src);
		vcpu.dx = (t_nubit16)((tempresult>>16)&0xffff);
		vcpu.ax = (t_nubit16)(tempresult&0xffff);
		if(tempresult == (t_nsbit32)vcpu.ax) {
			_ClrOF;
			_ClrEFLAGS_CF;
		} else {
			_SetOF;
			_SetEFLAGS_CF;
		}
#define op imul
		aexec_mdx16;
#undef op
		acheck(VCPU_EFLAGS_OF | VCPU_EFLAGS_CF) vapiPrintIns(vcpu.cs.selector,vcpu.eip,"+IMUL");
		break;
	default:CaseError("IMUL::len");break;}
}
static ASMCMP DIV(void *src, t_nubit8 len)
{
	t_nubit16 tempAX = vcpu.ax;
	t_nubit32 tempDXAX = (((t_nubit32)vcpu.dx)<<16)+vcpu.ax;
	switch(len) {
	case 8:
		async ub2 = d_nubit8(src);
		vcpuins.bit = 8;
		if(d_nubit8(src) == 0) {
			INT(0x00);
		} else {
			vcpu.al = (t_nubit8)(tempAX / d_nubit8(src));
			vcpu.ah = (t_nubit8)(tempAX % d_nubit8(src));
		}
#define op div
		aexec_mdx8;
#undef op
		acheck(VCPU_EFLAGS_DF | VCPU_EFLAGS_TF) vapiPrintIns(vcpu.cs.selector,vcpu.eip,"+DIV");
		break;
	case 16:
		async uw2 = d_nubit16(src);
		vcpuins.bit = 16;
		if(d_nubit16(src) == 0) {
			INT(0x00);
		} else {
			vcpu.ax = (t_nubit16)(tempDXAX / d_nubit16(src));
			vcpu.dx = (t_nubit16)(tempDXAX % d_nubit16(src));
		}
#define op div
		aexec_mdx16;
#undef op
		acheck(VCPU_EFLAGS_DF | VCPU_EFLAGS_TF) vapiPrintIns(vcpu.cs.selector,vcpu.eip,"+DIV");
		break;
	default:CaseError("DIV::len");break;}
}
static void IDIV(void *src, t_nubit8 len)
{
	t_nsbit16 tempAX = vcpu.ax;
	t_nsbit32 tempDXAX = (((t_nubit32)vcpu.dx)<<16)+vcpu.ax;
	switch(len) {
	case 8:
		async ub2 = d_nubit8(src);
		vcpuins.bit = 8;
		if(d_nubit8(src) == 0) {
			INT(0x00);
		} else {
			vcpu.al = (t_nubit8)(tempAX / d_nsbit8(src));
			vcpu.ah = (t_nubit8)(tempAX % d_nsbit8(src));
		}
#define op idiv
		aexec_mdx8;
#undef op
		acheck(VCPU_EFLAGS_DF | VCPU_EFLAGS_TF) vapiPrintIns(vcpu.cs.selector,vcpu.eip,"+IDIV");
		break;
	case 16:
		async uw2 = d_nubit16(src);
		vcpuins.bit = 16;
		if(d_nubit16(src) == 0) {
			INT(0x00);
		} else {
			vcpu.ax = (t_nubit16)(tempDXAX / d_nsbit16(src));
			vcpu.dx = (t_nubit16)(tempDXAX % d_nsbit16(src));
		}
#define op idiv
		aexec_mdx16;
#undef op
		acheck(VCPU_EFLAGS_DF | VCPU_EFLAGS_TF) vapiPrintIns(vcpu.cs.selector,vcpu.eip,"+IDIV");
		break;
	default:CaseError("IDIV::len");break;}
}
static void INT(t_nubit8 intid)
{
	PUSH((void *)&vcpu.eflags,16);
	_ClrIF;
	_ClrTF;
	PUSH((void *)&vcpu.cs.selector,16);
	PUSH((void *)&vcpu.eip,16);
	vcpu.eip = vramVarWord(0x0000,intid*4+0);
	vcpu.cs.selector = vramVarWord(0x0000,intid*4+2);
}

DONE UndefinedOpcode()
{
	if (!_GetCR0_PE) {
		vapiPrint("The NXVM CPU has encountered an illegal instruction.\n");
		vapiPrint("CS:%04X IP:%04X OP:%02X %02X %02X %02X\n",
			vcpu.cs.selector, vcpu.eip, _GetCode(vcpu.eip+0, 8),
			_GetCode(vcpu.eip+1, 8), _GetCode(vcpu.eip+2, 8),
			_GetCode(vcpu.eip+3, 8), _GetCode(vcpu.eip+4, 8));
		vapiCallBackMachineStop();
	}
	_SetExcept_UD(0);
	_CheckExcept("UndefinedOpcode");
}
DONE ADD_RM8_R8()
{
	i386(0x00) {
		vcpu.eip++;
		_GetModRegRM(8, 8, 0x01);
		_CheckExcept("ADD_RM8_R8::GetModRegRM");

		ADD(vcpuins.prm, vcpuins.pr, 8);
	} else {
		vcpu.ip++;
		GetModRegRM(8, 8);
		ADD(vcpuins.rm, vcpuins.r, 8);
	}
}
DONE ADD_RM16_R16()
{
	i386(0x01) {
		vcpu.eip++;
		_GetModRegRM(_GetOperandSize, _GetOperandSize, 0x01);
		_CheckExcept("ADD_RM16_R16::GetModRegRM");

		ADD(vcpuins.prm, vcpuins.pr, _GetOperandSize);
	} else {
		vcpu.ip++;
		GetModRegRM(16, 16);
		ADD(vcpuins.rm, vcpuins.r, 16);
	}
}
DONE ADD_R8_RM8()
{
	i386(0x02) {
		vcpu.eip++;
		_GetModRegRM(8, 8, 0x00);
		_CheckExcept("ADD_R8_RM8::GetModRegRM");

		ADD(vcpuins.pr, vcpuins.prm, 8);
	} else {
		vcpu.ip++;
		GetModRegRM(8, 8);
		ADD(vcpuins.r, vcpuins.rm, 8);
	}
}
DONE ADD_R16_RM16()
{
	i386(0x03) {
		vcpu.eip++;
		_GetModRegRM(_GetOperandSize, _GetOperandSize, 0x00);
		_CheckExcept("ADD_R16_RM16::GetModRegRM");

		ADD(vcpuins.pr, vcpuins.prm, _GetOperandSize);
	} else {
		vcpu.ip++;
		GetModRegRM(16, 16);
		ADD(vcpuins.r, vcpuins.rm, 16);
	}
}
DONE ADD_AL_I8()
{
	i386(0x04) {
		vcpu.eip++;
		_GetImm(8);
		_CheckExcept("ADD_AL_I8::GetImm(8)");

		ADD((t_vaddrcc)&vcpu.al, vcpuins.pimm, 8);
	} else {
		vcpu.ip++;
		GetImm(8);
		ADD((t_vaddrcc)&vcpu.al, vcpuins.imm, 8);
	}
}
DONE ADD_AX_I16()
{
	i386(0x05) {
		vcpu.eip++;
		switch (_GetOperandSize) {
		case 16:
			_GetImm(16);
			_CheckExcept("ADD_AX_I16::GetImm(16)");

			ADD((t_vaddrcc)&vcpu.ax, vcpuins.pimm, 16);
			break;
		case 32:
			_GetImm(32);
			_CheckExcept("ADD_AX_I16::GetImm(32)");

			ADC((t_vaddrcc)&vcpu.eax, vcpuins.pimm, 32);
			break;
		default:
			CaseError("ADC_AX_I16::OperandSize");return;
		}
	} else {
		vcpu.ip++;
		GetImm(16);
		ADD((t_vaddrcc)&vcpu.ax, vcpuins.imm, 16);
	}
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
	OR((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  OR_RM8_R8\n");
}
void OR_RM16_R16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	OR((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  OR_RM16_R16\n");
}
void OR_R8_RM8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	OR((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  OR_R8_RM8\n");
}
void OR_R16_RM16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	OR((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  OR_R16_RM16\n");
}
void OR_AL_I8()
{
	vcpu.eip++;
	GetImm(8);
	OR((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  OR_AL_I8\n");
}
void OR_AX_I16()
{
	vcpu.eip++;
	GetImm(16);
	OR((void *)&vcpu.ax,(void *)vcpuins.imm,16);
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
DONE INS_0F()
{
	t_nubit8 opcode;
	i386(0x0f) {
		vcpu.eip++;
		opcode = (t_nubit8)_GetCode(vcpu.eip, 8);
		_CheckExcept("INS_0F::_GetCode");

		ExecFun(vcpuins.table_0f[opcode]);
	} else
		POP_CS();
}
DONE ADC_RM8_R8()
{
	i386(0x10) {
		vcpu.eip++;
		_GetModRegRM(8, 8, 0x01);
		_CheckExcept("ADC_RM8_R8::GetModRegRM");

		ADC(vcpuins.prm, vcpuins.pr, 8);
	} else {
		vcpu.ip++;
		GetModRegRM(8, 8);
		ADC(vcpuins.rm, vcpuins.r, 8); 
	}
}
DONE ADC_RM16_R16()
{
	i386(0x11) {
		vcpu.eip++;
		_GetModRegRM(_GetOperandSize, _GetOperandSize, 0x01);
		_CheckExcept("ADC_RM16_R16::GetModRegRM");

		ADC(vcpuins.prm, vcpuins.pr, _GetOperandSize);
	} else {
		vcpu.ip++;
		GetModRegRM(16, 16);
		ADC(vcpuins.rm, vcpuins.r, 16);
	}
}
DONE ADC_R8_RM8()
{
	i386(0x12) {
		vcpu.eip++;
		_GetModRegRM(8, 8, 0x00);
		_CheckExcept("ADC_R8_RM8::GetModRegRM");

		ADC(vcpuins.pr, vcpuins.prm, 8);
	} else {
		vcpu.ip++;
		GetModRegRM(8, 8);
		ADC(vcpuins.r, vcpuins.rm, 8);
	}
}
DONE ADC_R16_RM16()
{
	i386(0x13) {
		vcpu.eip++;
		_GetModRegRM(_GetOperandSize, _GetOperandSize, 0x00);
		_CheckExcept("ADC_R16_RM16::GetModRegRM");

		ADC(vcpuins.pr, vcpuins.prm, _GetOperandSize);
	} else {
		vcpu.ip++;
		GetModRegRM(16, 16);
		ADC(vcpuins.r, vcpuins.rm, 16);
	}
}
DONE ADC_AL_I8()
{
	i386(0x14) {
		vcpu.eip++;
		_GetImm(8);
		_CheckExcept("ADC_AL_I8::GetImm(8)");

		ADC((t_vaddrcc)&vcpu.al, vcpuins.pimm, 8);
	} else {
		vcpu.ip++;
		GetImm(8);
		ADC((t_vaddrcc)&vcpu.al, vcpuins.imm, 8);
	}
}
DONE ADC_AX_I16()
{
	i386(0x15) {
		vcpu.eip++;
		switch (_GetOperandSize) {
		case 16:
			_GetImm(16);
			_CheckExcept("ADC_AX_I16::GetImm(16)");

			ADC((t_vaddrcc)&vcpu.ax, vcpuins.pimm, 16);
			break;
		case 32:
			_GetImm(32);
			_CheckExcept("ADC_AX_I16::GetImm(32)");

			ADC((t_vaddrcc)&vcpu.eax, vcpuins.pimm, 32);
			break;
		default:
			CaseError("ADC_AX_I16::OperandSize");return;
		}
	} else {
		vcpu.ip++;
		GetImm(16);
		ADC((t_vaddrcc)&vcpu.ax, vcpuins.imm, 16);
	}
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
	SBB((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SBB_RM8_R8\n");
}
void SBB_RM16_R16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	SBB((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SBB_RM16_R16\n");
}
void SBB_R8_RM8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	SBB((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SBB_R8_RM8\n");
}
void SBB_R16_RM16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	SBB((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SBB_R16_RM16\n");
}
void SBB_AL_I8()
{
	vcpu.eip++;
	GetImm(8);
	SBB((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SBB_AL_I8\n");
}
void SBB_AX_I16()
{
	vcpu.eip++;
	GetImm(16);
	SBB((void *)&vcpu.ax,(void *)vcpuins.imm,16);
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
DONE AND_RM8_R8()
{
	i386(0x20) {
		vcpu.eip++;
		_GetModRegRM(8, 8, 0x01);
		_CheckExcept("AND_RM8_R8::GetModRegRM");

		AND(vcpuins.prm, vcpuins.pr, 8);
	} else {
		vcpu.ip++;
		GetModRegRM(8,8);
		AND(vcpuins.rm, vcpuins.r, 8);
	}
}
DONE AND_RM16_R16()
{
	i386(0x21) {
		vcpu.eip++;
		_GetModRegRM(_GetOperandSize, _GetOperandSize, 0x01);
		_CheckExcept("AND_RM16_R16::GetModRegRM");

		AND(vcpuins.prm, vcpuins.pr, _GetOperandSize);
	} else {
		vcpu.ip++;
		GetModRegRM(16, 16);
		AND(vcpuins.rm, vcpuins.r, 16);
	}
}
DONE AND_R8_RM8()
{
	i386(0x22) {
		vcpu.eip++;
		_GetModRegRM(8, 8, 0x00);
		_CheckExcept("AND_R8_RM8::GetModRegRM");

		AND(vcpuins.pr, vcpuins.prm, 8);
	} else {
		vcpu.ip++;
		GetModRegRM(8, 8);
		AND(vcpuins.r, vcpuins.rm, 8);
	}
}
DONE AND_R16_RM16()
{
	i386(0x23) {
		vcpu.eip++;
		_GetModRegRM(_GetOperandSize, _GetOperandSize, 0x00);
		_CheckExcept("AND_R16_RM16::GetModRegRM");

		AND(vcpuins.pr, vcpuins.prm, _GetOperandSize);
	} else {
		vcpu.ip++;
		GetModRegRM(16, 16);
		AND(vcpuins.r, vcpuins.rm, 16);
	}
}
DONE AND_AL_I8()
{
	i386(0x24) {
		vcpu.eip++;
		_GetImm(8);
		_CheckExcept("AND_AL_I8::GetImm(8)");

		AND((t_vaddrcc)&vcpu.al, vcpuins.pimm, 8);
	} else {
		vcpu.ip++;
		GetImm(8);
		AND((t_vaddrcc)&vcpu.al, vcpuins.imm, 8);
	}
}
DONE AND_AX_I16()
{
	i386(0x25) {
		vcpu.eip++;
		switch (_GetOperandSize) {
		case 16:
			_GetImm(16);
			_CheckExcept("AND_AX_I16::GetImm(16)");

			AND((t_vaddrcc)&vcpu.ax, vcpuins.pimm, 16);
			break;
		case 32:
			_GetImm(32);
			_CheckExcept("AND_AX_I16::GetImm(32)");

			AND((t_vaddrcc)&vcpu.eax, vcpuins.pimm, 32);
			break;
		default:
			CaseError("ADC_AX_I16::OperandSize");return;
		}
	} else {
		vcpu.ip++;
		GetImm(16);
		AND((t_vaddrcc)&vcpu.ax, vcpuins.imm, 16);
	}
}
void ES()
{
	vcpu.eip++;
	vcpuins.prefix_sreg = PREFIX_SREG_ES;
}
ASMCMP DAA()
{
	t_nubit8 oldAL = vcpu.al;
	t_nubit8 newAL = vcpu.al + 0x06;
	vcpu.eip++;
	async;
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
	bugfix(18) {
		vcpuins.bit = 8;
		vcpuins.result = (t_nubitcc)vcpu.al;
		SetFlags(DAA_FLAG);
	} else ;
#define op daa
	aexec_cax8;
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs.selector,vcpu.eip-1,"DAA");
}
void SUB_RM8_R8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	SUB((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SUB_RM8_R8\n");
}
void SUB_RM16_R16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	SUB((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SUB_RM16_R16\n");
}
void SUB_R8_RM8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	SUB((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SUB_R8_RM8\n");
}
void SUB_R16_RM16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	SUB((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SUB_R16_RM16\n");
}
void SUB_AL_I8()
{
	vcpu.eip++;
	GetImm(8);
	SUB((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SUB_AL_I8\n");
}
void SUB_AX_I16()
{
	vcpu.eip++;
	GetImm(16);
	SUB((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  SUB_AX_I16\n");
}
void CS()
{
	vcpu.eip++;
	vcpuins.prefix_sreg = PREFIX_SREG_CS;
}
ASMCMP DAS()
{
	t_nubit8 oldAL = vcpu.al;
	vcpu.eip++;
	async;
	if(((vcpu.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
		vcpu.al -= 0x06;
		MakeBit(vcpu.eflags,VCPU_EFLAGS_CF,_GetEFLAGS_CF || (oldAL < 0x06));
		_SetEFLAGS_AF;
	} else _ClrEFLAGS_AF;
	if((vcpu.al > 0x9f) || _GetEFLAGS_CF) {
		vcpu.al -= 0x60;
		_SetEFLAGS_CF;
	} else _ClrEFLAGS_CF;
	bugfix(18) {
		vcpuins.bit = 8;
		vcpuins.result = (t_nubitcc)vcpu.al;
		SetFlags(DAS_FLAG);
	} else ;
#define op das
	aexec_cax8;
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs.selector,vcpu.eip-1,"DAS");
}
void XOR_RM8_R8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	XOR((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XOR_RM8_R8\n");
}
void XOR_RM16_R16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	XOR((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XOR_RM16_R16\n");
}
void XOR_R8_RM8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	XOR((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XOR_R8_RM8\n");
}
void XOR_R16_RM16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	XOR((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XOR_R16_RM16\n");
}
void XOR_AL_I8()
{
	vcpu.eip++;
	GetImm(8);
	XOR((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XOR_AL_I8\n");
}
void XOR_AX_I16()
{
	vcpu.eip++;
	GetImm(16);
	XOR((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XOR_AX_I16\n");
}
void SS()
{
	vcpu.eip++;
	vcpuins.prefix_sreg = PREFIX_SREG_SS;
}
DONE AAA()
{
	i386(0x37) {
		vcpu.eip++;
	} else {
		vcpu.ip++;
	}
	if(((vcpu.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
		vcpu.al += 0x06;
		vcpu.al &= 0x0f;
		vcpu.ah += 0x01;
		_SetEFLAGS_AF;
		_SetEFLAGS_CF;
	} else {
		_ClrEFLAGS_AF;
		_ClrEFLAGS_CF;
	}
}
void CMP_RM8_R8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	CMP((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  CMP_RM8_R8\n");
}
void CMP_RM16_R16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	CMP((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  CMP_RM16_R16\n");
}
void CMP_R8_RM8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	CMP((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  CMP_R8_RM8\n");
}
void CMP_R16_RM16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	CMP((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  CMP_R16_RM16\n");
}
void CMP_AL_I8()
{
	vcpu.eip++;
	GetImm(8);
	CMP((void *)&vcpu.al,(void *)vcpuins.imm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  CMP_AL_I8\n");
}
void CMP_AX_I16()
{
	vcpu.eip++;
	GetImm(16);
	CMP((void *)&vcpu.ax,(void *)vcpuins.imm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  CMP_AX_I16\n");
}
void DS()
{
	vcpu.eip++;
	vcpuins.prefix_sreg = PREFIX_SREG_DS;
}
DONE AAS()
{
	i386(0x3f) {
		vcpu.eip++;
	} else {
		vcpu.ip++;
	}
	if(((vcpu.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
		vcpu.al -= 0x06;
		vcpu.al &= 0x0f;
		vcpu.ah -= 0x01;
		_SetEFLAGS_AF;
		_SetEFLAGS_CF;
	} else {
		_ClrEFLAGS_CF;
		_ClrEFLAGS_AF;
	}
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
DONE BOUND_R16_M16_16()
{
	t_nsbit16 a16,l16,u16;
	t_nsbit32 a32,l32,u32;
	i386(0x62) {
		vcpu.eip++;
		_GetModRegRM(_GetOperandSize, _GetOperandSize * 2, 0x00);
		_CheckExcept("BOUND_R16_M16_16::_GetModRegRM");

		if (!vcpuins.flagmem) {
			_SetExcept_UD(0);
			_CheckExcept("BOUND_R16_M16_16::_GetModRegRM::flagmem(0)");
			return;
		}
		switch (_GetOperandSize) {
		case 16:
			a16 = d_nsbit16(vcpuins.pr);
			l16 = d_nsbit16(vcpuins.prm);
			u16 = d_nsbit16(vcpuins.prm + 2);
			if (a16 < l16 || a16 > u16) {
				_SetExcept_BR(0);
				_CheckExcept("BOUND_R16_M16_16::OperandSize(16)");
				return;
			}
			break;
		case 32:
			_CheckExcept("BOUND_R16_M16_16::OperandSize(32)::_GetModRegRM");

			a32 = d_nsbit32(vcpuins.pr);
			l32 = d_nsbit32(vcpuins.prm);
			u32 = d_nsbit32(vcpuins.prm + 4);
			if (a32 < l32 || a32 > u32) {
				_SetExcept_BR(0);
				_CheckExcept("BOUND_R16_M16_16::OperandSize(32)");
				return;
			}
			break;
		default:
			CaseError("BOUND_R16_M16_16::OperandSize");
			break;
		}
	} else
		UndefinedOpcode();
}
DONE ARPL_RM16_R16()
{
	t_nubit8 drpl, srpl;
	i386(0x63) {
		if (_GetCR0_PE && !_GetEFLAGS_VM) {
			vcpu.eip++;
			_GetModRegRM(16, 16, 0x01);
			_CheckExcept("ARPL_RM16_R16::GetModRegRM");

			drpl = _GetSelector_RPL(d_nubit16(vcpuins.prm));
			srpl = _GetSelector_RPL(d_nubit16(vcpuins.pr));
			if (drpl < srpl) {
				_SetEFLAGS_ZF;
				d_nubit16(vcpuins.prm) = (d_nubit16(vcpuins.prm) & ~VCPU_SELECTOR_RPL) | srpl;
			} else
				_ClrEFLAGS_ZF;
		} else
			_SetExcept_UD(0);
	} else
		UndefinedOpcode();
}
void FS()
{
	vcpu.eip++;
	vcpuins.prefix_sreg = PREFIX_SREG_FS;
}
void GS()
{
	vcpu.eip++;
	vcpuins.prefix_sreg = PREFIX_SREG_GS;
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
	JCC((void *)vcpuins.imm, _GetOF, 8);
}
void JNO()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, !_GetOF, 8);
}
void JC()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, _GetEFLAGS_CF, 8);
}
void JNC()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, !_GetEFLAGS_CF, 8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  JNC\n");
}
void JZ()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, _GetZF, 8);
}
void JNZ()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, !_GetZF, 8);
}
void JBE()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, (_GetEFLAGS_CF ||
		_GetZF), 8);
}
void JA()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, (!_GetEFLAGS_CF &&
		!_GetZF), 8);
}
void JS()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, _GetSF, 8);
}
void JNS()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, !_GetSF, 8);
}
void JP()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, _GetPF, 8);
}
void JNP()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, !_GetPF, 8);
}
void JL()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, (_GetSF !=
		_GetOF), 8);
}
void JNL()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, (_GetSF ==
		_GetOF), 8);
}
void JLE()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, (_GetZF ||
		(_GetSF !=
		_GetOF)), 8);
}
void JG()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void *)vcpuins.imm, (!_GetZF &&
		(_GetSF ==
		_GetOF)), 8);
}
DONE INS_80()
{
	i386(0x80) {
		vcpu.eip++;
		_GetModRegRM(0, 8, 0x01);
		_CheckExcept("INS_80::GetModRegRM");

		_GetImm(8);
		_CheckExcept("INS_80::GetImm(8)");

		switch (vcpuins.pr) {
		case 0: ADD(vcpuins.prm, vcpuins.pimm, 8); break;
		case 1: OR ((void *)vcpuins.prm, (void *)vcpuins.pimm, 8); break;
		case 2: ADC(vcpuins.prm, vcpuins.pimm, 8); break;
		case 3: SBB((void *)vcpuins.prm, (void *)vcpuins.pimm, 8); break;
		case 4: AND(vcpuins.prm, vcpuins.pimm, 8); break;
		case 5: SUB((void *)vcpuins.prm, (void *)vcpuins.pimm, 8); break;
		case 6: XOR((void *)vcpuins.prm, (void *)vcpuins.pimm, 8); break;
		case 7: CMP((void *)vcpuins.prm, (void *)vcpuins.pimm, 8); break;
		default:CaseError("INS_80::vcpuins.pr");break;}
	} else {
		vcpu.ip++;
		GetModRegRM(0, 8);
		GetImm(8);
		switch (vcpuins.r) {
		case 0: ADD(vcpuins.rm, vcpuins.imm, 8); break;
		case 1: OR ((void *)vcpuins.rm, (void *)vcpuins.imm, 8); break;
		case 2: ADC(vcpuins.rm, vcpuins.imm, 8); break;
		case 3: SBB((void *)vcpuins.rm, (void *)vcpuins.imm, 8); break;
		case 4: AND(vcpuins.rm, vcpuins.imm, 8); break;
		case 5: SUB((void *)vcpuins.rm, (void *)vcpuins.imm, 8); break;
		case 6: XOR((void *)vcpuins.rm, (void *)vcpuins.imm, 8); break;
		case 7: CMP((void *)vcpuins.rm, (void *)vcpuins.imm, 8); break;
		default:CaseError("INS_80::vcpuins.r");break;}
	}
}
DONE INS_81()
{
	i386(0x81) {
		vcpu.eip++;
		_GetModRegRM(0, _GetOperandSize, 0x01);
		_CheckExcept("INS_81::GetModRegRM");

		_GetImm(_GetOperandSize);
		_CheckExcept("INS_81::GetImm");

		switch(vcpuins.pr) {
		case 0: ADD(vcpuins.prm, vcpuins.pimm, _GetOperandSize); break;
		case 1: OR ((void *)vcpuins.prm, (void *)vcpuins.pimm, _GetOperandSize); break;
		case 2: ADC(vcpuins.prm, vcpuins.pimm, _GetOperandSize); break;
		case 3: SBB((void *)vcpuins.prm, (void *)vcpuins.pimm, _GetOperandSize); break;
		case 4: AND(vcpuins.prm, vcpuins.pimm, _GetOperandSize); break;
		case 5: SUB((void *)vcpuins.prm, (void *)vcpuins.pimm, _GetOperandSize); break;
		case 6: XOR((void *)vcpuins.prm, (void *)vcpuins.pimm, _GetOperandSize); break;
		case 7: CMP((void *)vcpuins.prm, (void *)vcpuins.pimm, _GetOperandSize); break;
		default:CaseError("INS_81::vcpuins.pr");break;}
	} else {
		vcpu.ip++;
		GetModRegRM(0, 16);
		GetImm(16);
		switch(vcpuins.r) {
		case 0: ADD(vcpuins.rm, vcpuins.imm, 16); break;
		case 1: OR ((void *)vcpuins.rm, (void *)vcpuins.imm, 16); break;
		case 2: ADC(vcpuins.rm, vcpuins.imm, 16); break;
		case 3: SBB((void *)vcpuins.rm, (void *)vcpuins.imm, 16); break;
		case 4: AND(vcpuins.rm, vcpuins.imm, 16); break;
		case 5: SUB((void *)vcpuins.rm, (void *)vcpuins.imm, 16); break;
		case 6: XOR((void *)vcpuins.rm, (void *)vcpuins.imm, 16); break;
		case 7: CMP((void *)vcpuins.rm, (void *)vcpuins.imm, 16); break;
		default:CaseError("INS_81::vcpuins.r");break;}
	}
}
DONE INS_83()
{
	t_nubit8 len = (_GetOperandSize + 8) >> 1;
	i386(0x83) {
		vcpu.eip++;
		_GetModRegRM(0, _GetOperandSize, 0x01);
		_CheckExcept("INS_83::GetModRegRM");

		_GetImm(8);
		_CheckExcept("INS_83::GetImm(8)");

		switch(vcpuins.pr) {
		case 0: ADD(vcpuins.prm, vcpuins.pimm, len); break;
		case 1: OR ((void *)vcpuins.prm, (void *)vcpuins.pimm, len); break;
		case 2: ADC(vcpuins.prm, vcpuins.pimm, len); break;
		case 3: SBB((void *)vcpuins.prm, (void *)vcpuins.pimm, len); break;
		case 4: AND(vcpuins.prm, vcpuins.pimm, len); break;
		case 5: SUB((void *)vcpuins.prm, (void *)vcpuins.pimm, len); break;
		case 6: XOR((void *)vcpuins.prm, (void *)vcpuins.pimm, len); break;
		case 7: CMP((void *)vcpuins.prm, (void *)vcpuins.pimm, len); break;
		default:CaseError("INS_83::vcpuins.pr");break;}
	} else {
		vcpu.ip++;
		GetModRegRM(0, 16);
		GetImm(8);
		switch(vcpuins.r) {
		case 0: ADD(vcpuins.rm, vcpuins.imm, 12); break;
		case 1: OR ((void *)vcpuins.rm, (void *)vcpuins.imm, 12); break;
		case 2: ADC(vcpuins.rm, vcpuins.imm, 12); break;
		case 3: SBB((void *)vcpuins.rm, (void *)vcpuins.imm, 12); break;
		case 4: AND(vcpuins.rm, vcpuins.imm, 12); break;
		case 5: SUB((void *)vcpuins.rm, (void *)vcpuins.imm, 12); break;
		case 6: XOR((void *)vcpuins.rm, (void *)vcpuins.imm, 12); break;
		case 7: CMP((void *)vcpuins.rm, (void *)vcpuins.imm, 12); break;
		default:CaseError("INS_83::vcpuins.r");break;}
	}
}
void TEST_RM8_R8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	TEST((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  TEST_RM8_R8\n");
}
void TEST_RM16_R16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	TEST((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  TEST_RM16_R16\n");
}
void XCHG_R8_RM8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	XCHG((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XCHG_R8_RM8\n");
}
void XCHG_R16_RM16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	XCHG((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  XCHG_R16_RM16\n");
}
void MOV_RM8_R8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	MOV((void *)vcpuins.rm,(void *)vcpuins.r,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOV_RM8_R8\n");
}
void MOV_RM16_R16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOV_RM16_R16\n");
}
void MOV_R8_RM8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	MOV((void *)vcpuins.r,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOV_R8_RM8\n");
}
void MOV_R16_RM16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOV_R16_RM16\n");
}
void MOV_RM16_SEG()
{
	vcpu.eip++;
	GetModRegRM(4,16);
	MOV((void *)vcpuins.rm,(void *)vcpuins.r,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOV_RM16_SEG\n");
}
void LEA_R16_M16()
{
	vcpu.eip++;
	GetModRegRMEA();
	d_nubit16(vcpuins.r) = vcpuins.rm & 0xffff;
}
void MOV_SEG_RM16()
{
	vcpu.eip++;
	GetModRegRM(4,16);
	MOV((void *)vcpuins.r,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOV_SEG_RM16\n");
}
void POP_RM16()
{
	vcpu.eip++;
	GetModRegRM(0,16);
	switch(vcpuins.r) {
	case 0:
		bugfix(14) POP((void *)vcpuins.rm,16);
		else POP((void *)vcpuins.rm,16);
		break;
	default:CaseError("POP_RM16::vcpuins.r");break;}
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
ASMCMP CBW()
{
	vcpu.eip++;
	async;
	vcpu.ax = (t_nsbit8)vcpu.al;
#define op cbw
	aexec_cax16;
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs.selector,vcpu.eip-1,"CBW");
}
ASMCMP CWD()
{
	vcpu.eip++;
	async;
	if (vcpu.ax & 0x8000) vcpu.dx = 0xffff;
	else vcpu.dx = 0x0000;
#define op cwd
	aexec_cax16;
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs.selector,vcpu.eip-1,"CWD");
}
void CALL_PTR16_16()
{
	t_nubit16 newcs;
	t_nubit32 neweip;
	i386(0x9a) {
		vcpu.eip++;
		switch (_GetOperandSize) {
		case 16:
			_GetImm(16);
			_CheckExcept("CALL_PTR16_16::OperandSize(16)::_GetImm(ip)");
			neweip = d_nubit16(vcpuins.pimm);
			_GetImm(16);
			_CheckExcept("CALL_PTR16_16::OperandSize(16)::_GetImm(cs)");
			newcs = d_nubit16(vcpuins.pimm);
			_Call_Far(newcs, neweip, 16);
			_CheckExcept("CALL_PTR16_16::OperandSize(16)::_Call_Far");
			break;
		case 32:
			_GetImm(32);
			_CheckExcept("CALL_PTR16_16::OperandSize(32)::_GetImm(eip)");
			neweip = d_nubit32(vcpuins.pimm);
			_GetImm(16);
			_CheckExcept("CALL_PTR16_16::OperandSize(32)::_GetImm(cs)");
			newcs = d_nubit16(vcpuins.pimm);
			_Call_Far(newcs, neweip, 32);
			_CheckExcept("CALL_PTR16_16::OperandSize(32)::_Call_Far");
			break;
		default:
			CaseError("CALL_PTR16_16::OperandSize");
			return;
		}
	} else {
		vcpu.eip++;
		GetImm(16);
		neweip = d_nubit16(vcpuins.imm);
		GetImm(16);
		newcs = d_nubit16(vcpuins.imm);
		PUSH((void *)&vcpu.cs.selector,16);
		PUSH((void *)&vcpu.eip,16);
		vcpu.ip = GetMax16(neweip);
		vcpu.cs.selector = newcs;
	}
}
NOTIMP WAIT()
{
	vcpu.eip++;
	/* not implemented */
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
	MOV((void *)&vcpu.al,(void *)vcpuins.rm,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOV_AL_M8\n");
}
void MOV_AX_M16()
{
	vcpu.eip++;
	GetMem();
	MOV((void *)&vcpu.ax,(void *)vcpuins.rm,16);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOV_AX_M16\n");
}
void MOV_M8_AL()
{
	vcpu.eip++;
	GetMem();
	MOV((void *)vcpuins.rm,(void *)&vcpu.al,8);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  MOV_M8_AL\n");
}
void MOV_M16_AX()
{
	vcpu.eip++;
	GetMem();
	MOV((void *)vcpuins.rm,(void *)&vcpu.ax,16);
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
	TEST((void *)&vcpu.al,(void *)vcpuins.imm,8);
}
void TEST_AX_I16()
{
	vcpu.eip++;
	GetImm(16);
	TEST((void *)&vcpu.ax,(void *)vcpuins.imm,16);
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
	MOV((void *)&vcpu.al,(void *)vcpuins.imm,8);
}
void MOV_CL_I8()
{
	vcpu.eip++;
	GetImm(8);
	MOV((void *)&vcpu.cl,(void *)vcpuins.imm,8);
}
void MOV_DL_I8()
{
	vcpu.eip++;
	GetImm(8);
	MOV((void *)&vcpu.dl,(void *)vcpuins.imm,8);
}
void MOV_BL_I8()
{
	vcpu.eip++;
	GetImm(8);
	MOV((void *)&vcpu.bl,(void *)vcpuins.imm,8);
}
void MOV_AH_I8()
{
	vcpu.eip++;
	GetImm(8);
	MOV((void *)&vcpu.ah,(void *)vcpuins.imm,8);
}
void MOV_CH_I8()
{
	vcpu.eip++;
	GetImm(8);
	MOV((void *)&vcpu.ch,(void *)vcpuins.imm,8);
}
void MOV_DH_I8()
{
	vcpu.eip++;
	GetImm(8);
	MOV((void *)&vcpu.dh,(void *)vcpuins.imm,8);
}
void MOV_BH_I8()
{
	vcpu.eip++;
	GetImm(8);
	MOV((void *)&vcpu.bh,(void *)vcpuins.imm,8);
}
void MOV_AX_I16()
{
	vcpu.eip++;
	GetImm(16);
	MOV((void *)&vcpu.ax,(void *)vcpuins.imm,16);
}
void MOV_CX_I16()
{
	vcpu.eip++;
	GetImm(16);
	MOV((void *)&vcpu.cx,(void *)vcpuins.imm,16);
}
void MOV_DX_I16()
{
	vcpu.eip++;
	GetImm(16);
	MOV((void *)&vcpu.dx,(void *)vcpuins.imm,16);
}
void MOV_BX_I16()
{
	vcpu.eip++;
	GetImm(16);
	MOV((void *)&vcpu.bx,(void *)vcpuins.imm,16);
}
void MOV_SP_I16()
{
	vcpu.eip++;
	GetImm(16);
	MOV((void *)&vcpu.sp,(void *)vcpuins.imm,16);
}
void MOV_BP_I16()
{
	vcpu.eip++;
	GetImm(16);
	MOV((void *)&vcpu.bp,(void *)vcpuins.imm,16);
}
void MOV_SI_I16()
{
	vcpu.eip++;
	GetImm(16);
	MOV((void *)&vcpu.si,(void *)vcpuins.imm,16);
}
void MOV_DI_I16()
{
	vcpu.eip++;
	GetImm(16);
	MOV((void *)&vcpu.di,(void *)vcpuins.imm,16);
}
void INS_C0()
{
	vcpu.eip++;
	GetModRegRM(0,8);
	GetImm(8);
	switch(vcpuins.r) {
	case 0:	ROL((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 1:	ROR((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 2:	RCL((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 3:	RCR((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 4:	SHL((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 5:	SHR((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 6:	SAL((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	case 7:	SAR((void *)vcpuins.rm,(void *)vcpuins.imm,8);break;
	default:CaseError("INS_C0::vcpuins.r");break;}
}
void INS_C1()
{
	vcpu.eip++;
	GetModRegRM(0,16);
	GetImm(8);
	switch(vcpuins.r) {
	case 0:	ROL((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 1:	ROR((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 2:	RCL((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 3:	RCR((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 4:	SHL((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 5:	SHR((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 6:	SAL((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	case 7:	SAR((void *)vcpuins.rm,(void *)vcpuins.imm,16);break;
	default:CaseError("INS_C1::vcpuins.r");break;}
}
void RET_I16()
{
	t_nubit16 addsp;
	vcpu.eip++;
	bugfix(15) {
		GetImm(16);
		addsp = d_nubit16(vcpuins.imm);
	} else {
		GetImm(8);
		addsp = d_nubit8(vcpuins.imm);
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
	MOV((void *)vcpuins.r,(void *)vcpuins.rm,16);
	MOV((void *)&vcpu.es.selector,(void *)(vcpuins.rm+2),16);
}
void LDS_R16_M16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	MOV((void *)vcpuins.r,(void *)vcpuins.rm,16);
	MOV((void *)&vcpu.ds.selector,(void *)(vcpuins.rm+2),16);
}
void MOV_M8_I8()
{
	vcpu.eip++;
	GetModRegRM(8,8);
	GetImm(8);
	MOV((void *)vcpuins.rm,(void *)vcpuins.imm,8);
}
void MOV_M16_I16()
{
	vcpu.eip++;
	GetModRegRM(16,16);
	GetImm(16);
	MOV((void *)vcpuins.rm,(void *)vcpuins.imm,16);
}
void RETF_I16()
{
	t_nubit16 addsp;
	vcpu.eip++;
	GetImm(16);
	addsp = d_nubit16(vcpuins.imm);
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
	async;
	vcpu.eip++;
	INT(0x03);
}
void INT_I8()
{
	async;
	vcpu.eip++;
	GetImm(8);
	INT(d_nubit8(vcpuins.imm));
}
void INTO()
{
	async;
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
	switch(vcpuins.r) {
	case 0:	ROL((void *)vcpuins.rm,NULL,8);break;
	case 1:	ROR((void *)vcpuins.rm,NULL,8);break;
	case 2:	RCL((void *)vcpuins.rm,NULL,8);break;
	case 3:	RCR((void *)vcpuins.rm,NULL,8);break;
	case 4:	SHL((void *)vcpuins.rm,NULL,8);break;
	case 5:	SHR((void *)vcpuins.rm,NULL,8);break;
	case 6:	SAL((void *)vcpuins.rm,NULL,8);break;
	case 7:	SAR((void *)vcpuins.rm,NULL,8);break;
	default:CaseError("INS_D0::vcpuins.r");break;}
}
void INS_D1()
{
	vcpu.eip++;
	GetModRegRM(0,16);
	switch(vcpuins.r) {
	case 0:	ROL((void *)vcpuins.rm,NULL,16);break;
	case 1:	ROR((void *)vcpuins.rm,NULL,16);break;
	case 2:	RCL((void *)vcpuins.rm,NULL,16);break;
	case 3:	RCR((void *)vcpuins.rm,NULL,16);break;
	case 4:	SHL((void *)vcpuins.rm,NULL,16);break;
	case 5:	SHR((void *)vcpuins.rm,NULL,16);break;
	case 6:	SAL((void *)vcpuins.rm,NULL,16);break;
	case 7:	SAR((void *)vcpuins.rm,NULL,16);break;
	default:CaseError("INS_D1::vcpuins.r");break;}
}
void INS_D2()
{
	vcpu.eip++;
	GetModRegRM(0,8);
	switch(vcpuins.r) {
	case 0:	ROL((void *)vcpuins.rm,(void *)&vcpu.cl,8);break;
	case 1:	ROR((void *)vcpuins.rm,(void *)&vcpu.cl,8);break;
	case 2:	RCL((void *)vcpuins.rm,(void *)&vcpu.cl,8);break;
	case 3:	RCR((void *)vcpuins.rm,(void *)&vcpu.cl,8);break;
	case 4:	SHL((void *)vcpuins.rm,(void *)&vcpu.cl,8);break;
	case 5:	SHR((void *)vcpuins.rm,(void *)&vcpu.cl,8);break;
	case 6:	SAL((void *)vcpuins.rm,(void *)&vcpu.cl,8);break;
	case 7:	SAR((void *)vcpuins.rm,(void *)&vcpu.cl,8);break;
	default:CaseError("INS_D2::vcpuins.r");break;}
}
void INS_D3()
{
	vcpu.eip++;
	GetModRegRM(0,16);
	switch(vcpuins.r) {
	case 0:	ROL((void *)vcpuins.rm,(void *)&vcpu.cl,16);break;
	case 1:	ROR((void *)vcpuins.rm,(void *)&vcpu.cl,16);break;
	case 2:	RCL((void *)vcpuins.rm,(void *)&vcpu.cl,16);break;
	case 3:	RCR((void *)vcpuins.rm,(void *)&vcpu.cl,16);break;
	case 4:	SHL((void *)vcpuins.rm,(void *)&vcpu.cl,16);break;
	case 5:	SHR((void *)vcpuins.rm,(void *)&vcpu.cl,16);break;
	case 6:	SAL((void *)vcpuins.rm,(void *)&vcpu.cl,16);break;
	case 7:	SAR((void *)vcpuins.rm,(void *)&vcpu.cl,16);break;
	default:CaseError("INS_D3::vcpuins.r");break;}
}
DONE AAM()
{
	/* 0xd4 0x0a */
	t_nubit8 base;
	i386(0xd4) {
		vcpu.eip++;
		_GetImm(8);
		_CheckExcept("AAM::GetImm(8)");

		base = d_nubit8(vcpuins.pimm);
	} else {
		vcpu.ip++;
		GetImm(8);
		base = d_nubit8(vcpuins.imm);
	}
	if (base != 0x0a) {
		_SetExcept_UD(0);
		return;
	}
	vcpu.ah = vcpu.al / base;
	vcpu.al = vcpu.al % base;
	vcpuins.bit = 0x08;
	vcpuins.result = vcpu.al & 0xff;
	SetFlags(AAM_FLAG);
}
DONE AAD()
{
	// 0xd5 0x0a
	t_nubit8 base;
	i386(0xd5) {
		vcpu.eip++;
		_GetImm(8);
		_CheckExcept("AAD::GetImm(8)");

		base = d_nubit8(vcpuins.pimm);
		if (base != 0x0a) {
			_SetExcept_UD(0);
			return;
		}
	} else {
		vcpu.ip++;
		GetImm(8);
		base = d_nubit8(vcpuins.imm);
	}
	vcpu.al = (vcpu.al + (vcpu.ah * base)) & 0xff;
	vcpu.ah = 0x00;
	vcpuins.bit = 0x08;
	vcpuins.result = vcpu.al & 0xff;
	SetFlags(AAD_FLAG);
}
void XLAT()
{
	t_cpu_sreg *overds = _GetPtrOverrideDS();
	vcpu.eip++;
	vcpu.al = vramVarByte(overds->selector,vcpu.bx+vcpu.al);
}
/*
void INS_D9()
{// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  INS_D9\n");
}
void INS_DB()
{// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  INS_DB\n");
}
*/
void LOOPNZ()
{
	t_nsbit8 rel8;
	vcpu.eip++;
	GetImm(8);
	bugfix(12) rel8 = d_nsbit8(vcpuins.imm);
	else rel8 = d_nubit8(vcpuins.imm);
	vcpu.cx--;
	if(vcpu.cx && !_GetZF) vcpu.eip += rel8;
}
void LOOPZ()
{
	t_nsbit8 rel8;
	vcpu.eip++;
	GetImm(8);
	bugfix(12) rel8 = d_nsbit8(vcpuins.imm);
	else rel8 = d_nubit8(vcpuins.imm);
	vcpu.cx--;
	if(vcpu.cx && _GetZF) vcpu.eip += rel8;
}
void LOOP()
{
	t_nsbit8 rel8;
	vcpu.eip++;
	GetImm(8);
	bugfix(12) rel8 = d_nsbit8(vcpuins.imm);
	else rel8 = d_nubit8(vcpuins.imm);
	vcpu.cx--;
	if(vcpu.cx) vcpu.eip += rel8;
}
void JCXZ_REL8()
{
	vcpu.eip++;
	GetImm(8);
	JCC((void*)vcpuins.imm,!vcpu.cx,8);
}
void IN_AL_I8()
{
	vcpu.eip++;
	GetImm(8);
//	vapiPrint("IN: %02X\n",d_nubit8(vcpuins.imm));
	ExecFun(vport.in[d_nubit8(vcpuins.imm)]);
	vcpu.al = vport.iobyte;
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  IN_AL_I8\n");
}
void IN_AX_I8()
{
	vcpu.eip++;
	GetImm(8);
//	vapiPrint("IN: %02X\n",d_nubit8(vcpuins.imm));
	ExecFun(vport.in[d_nubit8(vcpuins.imm)]);
	vcpu.ax = vport.ioword;
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  IN_AX_I8\n");
}
void OUT_I8_AL()
{
	vcpu.eip++;
	GetImm(8);
//	vapiPrint("OUT: %02X\n",d_nubit8(vcpuins.imm));
	vport.iobyte = vcpu.al;
	ExecFun(vport.out[d_nubit8(vcpuins.imm)]);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  OUT_I8_AL\n");
}
void OUT_I8_AX()
{
	vcpu.eip++;
	GetImm(8);
//	vapiPrint("OUT: %02X\n",d_nubit8(vcpuins.imm));
	vport.ioword = vcpu.ax;
	ExecFun(vport.out[d_nubit8(vcpuins.imm)]);
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  OUT_I8_AX\n");
}
DONE CALL_REL16()
{
	t_nsbit16 rel16;
	t_nsbit32 rel32;
	i386(0xe8) {
		vcpu.eip++;
		switch (_GetOperandSize) {
		case 16:
			_GetImm(16);
			_CheckExcept("CALL_REL16::OperandSize(16)::_GetImm");
			rel16 = d_nsbit16(vcpuins.pimm);
			_Call_Near(GetMax16(vcpu.eip + rel16), 16);
			_CheckExcept("CALL_REL16::OperandSize(16)::_Call_Near");
			break;
		case 32:
			_GetImm(32);
			_CheckExcept("CALL_REL16::OperandSize(32)::_GetImm");
			rel32 = d_nsbit32(vcpuins.pimm);
			_Call_Near(GetMax32(vcpu.eip + rel32), 32);
			_CheckExcept("CALL_REL16::OperandSize(32)::_Call_Near");
			break;
		default:
			CaseError("CALL_REL16::OperandSize");
			return;
		}
	} else {
		vcpu.ip++;
		GetImm(16);
		rel16 = d_nsbit16(vcpuins.imm);
		PUSH((void *)&vcpu.ip,16);
		vcpu.ip += rel16;
		vcpu.ip &= 0x0000ffff;
	}
}
void JMP_REL16()
{
	t_nsbit16 rel16;
	vcpu.eip++;
	GetImm(16);
	rel16 = d_nsbit16(vcpuins.imm);
	vcpu.eip += rel16;
	vcpu.eip &= 0x0000ffff;
}
void JMP_PTR16_16()
{
	t_nubit16 newip,newcs;
	vcpu.eip++;
	GetImm(16);
	newip = d_nubit16(vcpuins.imm);
	GetImm(16);
	newcs = d_nubit16(vcpuins.imm);
	vcpu.eip = newip;
	vcpu.cs.selector = newcs;
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  JMP_PTR16_16\n");
}
void JMP_REL8()
{
	t_nsbit8 rel8;
	vcpu.eip++;
	GetImm(8);
	rel8 = d_nsbit8(vcpuins.imm);
	bugfix(9) vcpu.eip += rel8;
	else vcpu.eip += d_nubit8(vcpuins.imm);
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
NOTIMP LOCK()
{
	vcpu.eip++;
	/* Not Implemented */
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
NOTIMP HLT()
{
	vcpu.eip++;
	/* Not Implemented */
}
ASMCMP CMC()
{
	vcpu.eip++;
	async;
	vcpu.eflags ^= VCPU_EFLAGS_CF;
#define op cmc
	aexec_csf
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs.selector,vcpu.eip-1,"CMC");
}
void INS_F6()
{
	vcpu.eip++;
	GetModRegRM(0,8);
	switch(vcpuins.r) {
	case 0:	GetImm(8);
			TEST((void *)vcpuins.rm,(void *)vcpuins.imm,8);
			break;
	case 2:	NOT ((void *)vcpuins.rm,8);	break;
	case 3:	NEG ((void *)vcpuins.rm,8);	break;
	case 4:	MUL ((void *)vcpuins.rm,8);	break;
	case 5:	IMUL((void *)vcpuins.rm,8);	break;
	case 6:	DIV ((void *)vcpuins.rm,8);	break;
	case 7:	IDIV((void *)vcpuins.rm,8);	break;
	default:CaseError("INS_F6::vcpuins.r");break;}
}
void INS_F7()
{
	vcpu.eip++;
	GetModRegRM(0,16);
	switch(vcpuins.r) {
	case 0:	GetImm(16);
			TEST((void *)vcpuins.rm,(void *)vcpuins.imm,16);
			break;
	case 2:	NOT ((void *)vcpuins.rm,16);	break;
	case 3:	NEG ((void *)vcpuins.rm,16);	break;
	case 4:	MUL ((void *)vcpuins.rm,16);	break;
	case 5:	IMUL((void *)vcpuins.rm,16);	break;
	case 6:	DIV ((void *)vcpuins.rm,16);	break;
	case 7:	IDIV((void *)vcpuins.rm,16);	break;
	default:CaseError("INS_F7::vcpuins.r");break;}
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  INS_F7\n");
}
ASMCMP CLC()
{
	vcpu.eip++;
	async;
	_ClrEFLAGS_CF;
#define op clc
	aexec_csf
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs.selector,vcpu.eip-1,"CLC");
}
ASMCMP STC()
{
	vcpu.eip++;
	async;
	_SetEFLAGS_CF;
#define op stc
	aexec_csf
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs.selector,vcpu.eip-1,"STC");
}
void CLI()
{
	vcpu.eip++;
	_ClrIF;
}
void STI()
{
	vcpu.eip++;
	_SetIF;
}
ASMCMP CLD()
{
	vcpu.eip++;
	async;
	_ClrDF;
#define op cld
	aexec_csf
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs.selector,vcpu.eip-1,"CLD");
}
ASMCMP STD()
{
	vcpu.eip++;
	async;
	_SetDF;
#define op STD
	aexec_csf
#undef op
	acheck(AFLAGS1) vapiPrintIns(vcpu.cs.selector,vcpu.eip-1,"STD");
}
void INS_FE()
{
	vcpu.eip++;
	GetModRegRM(0,8);
	switch(vcpuins.r) {
	case 0:	INC((void *)vcpuins.rm,8);	break;
	case 1:	DEC((void *)vcpuins.rm,8);	break;
	default:CaseError("INS_FE::vcpuins.r");break;}
	// _vapiPrintAddr(vcpu.cs.selector,vcpu.eip);vapiPrint("  INS_FE\n");
}
void INS_FF()
{
	t_nubit8 modrm = 0x00;
	t_nubit16 newcs = 0x0000;
	t_nubit32 neweip = 0x00000000;
	i386(0xff) {
		vcpu.eip++;
		modrm = (t_nubit8)_GetCode(vcpu.eip, 8);
		_CheckExcept("INS_FF::_GetCode");
		switch (_GetModRM_REG(modrm)) {
		case 2: _GetModRegRM(0, _GetOperandSize, 0x00);break;
		case 3: _GetModRegRM(0, _GetOperandSize + 16, 0x00);break;
		default:_GetModRegRM(0, _GetOperandSize, 0x01);break;
		}
		switch(vcpuins.pr) {
		case 0: INC((void *)vcpuins.prm,16);break;
		case 1: DEC((void *)vcpuins.prm,16);break;
		case 2: /* CALL_RM16 */
			switch (_GetOperandSize) {
			case 16:
				_Call_Near(d_nubit16(vcpuins.prm), 16);
				_CheckExcept("INS_FF::vcpuins.pr(2)::OperandSize(16)::_Call_Near");
				break;
			case 32:
				_Call_Near(d_nubit32(vcpuins.prm), 32);
				_CheckExcept("INS_FF::vcpuins.pr(2)::OperandSize(32)::_Call_Near");
				break;
			default:
				CaseError("INS_FF::vcpuins.pr(2)::OperandSize");
				return;
			}
			break;
		case 3: /* CALL_M16_16 */
			switch (_GetOperandSize) {
			case 16:
				neweip = d_nubit16(vcpuins.prm);
				newcs = d_nubit16(vcpuins.prm + 2);
				_Call_Far(newcs, neweip, 16);
				_CheckExcept("INS_FF::vcpuins.pr(3)::OperandSize(16)::_Call_Far");
				break;
			case 32:
				neweip = d_nubit32(vcpuins.prm);
				newcs = d_nubit16(vcpuins.prm + 4);
				_Call_Far(newcs, neweip, 32);
				_CheckExcept("INS_FF::vcpuins.pr(3)::OperandSize(32)::_Call_Far");
				break;
			default: CaseError("INS_FF::vcpuins.pr(3)::OperandSize");break;
			}
			break;
		case 4: /* JMP_RM16 */
			vcpu.eip = d_nubit16(vcpuins.prm);
			break;
		case 5: /* JMP_M16_16 */
			vcpu.eip = d_nubit16(vcpuins.prm);
			vcpu.cs.selector = d_nubit16(vcpuins.prm+2);
			break;
		case 6: /* PUSH_RM16 */
			PUSH((void *)vcpuins.prm,16);
			break;
		case 7:
			UndefinedOpcode();
			return;
		default:
			CaseError("INS_FF::vcpuins.r");
			return;
		}
	} else {
		vcpu.ip++;
		GetModRegRM(0,16);
		switch(vcpuins.r) {
		case 0:	INC((void *)vcpuins.rm,16);	break;
		case 1:	DEC((void *)vcpuins.rm,16);	break;
		case 2:	/* CALL_RM16 */
			PUSH((void *)&vcpu.eip,16);
			vcpu.eip = d_nubit16(vcpuins.rm);
			break;
		case 3:	/* CALL_M16_16 */
			PUSH((void *)&vcpu.cs.selector,16);
			PUSH((void *)&vcpu.eip,16);
			vcpu.eip = d_nubit16(vcpuins.rm);
			bugfix(11) vcpu.cs.selector = d_nubit16(vcpuins.rm+2);
			else vcpu.cs.selector = d_nubit16(vcpuins.rm+1);
			break;
		case 4:	/* JMP_RM16 */
			vcpu.eip = d_nubit16(vcpuins.rm);
			break;
		case 5:	/* JMP_M16_16 */
			vcpu.eip = d_nubit16(vcpuins.rm);
			bugfix(11) vcpu.cs.selector = d_nubit16(vcpuins.rm+2);
			else vcpu.cs.selector = d_nubit16(vcpuins.rm+1);
			break;
		case 6:	/* PUSH_RM16 */
			PUSH((void *)vcpuins.rm,16);
			break;
		case 7:
			UndefinedOpcode();
			return;
		default:CaseError("INS_FF::vcpuins.r");return;}
	}
}

DONE _GetInfoForBTcc(t_nubit8 regbit, t_nubit8 rmbit, t_bool write)
{
	/* prm = actual destination, pimm = (1 << bitoffset) */
	t_nsbitcc bitoffset = 0;
	_GetModRegRM(regbit, rmbit, write);
	_CheckExcept("_GetInfoForBTcc::_GetModRegRM");
	if (!regbit) {
		_GetImm(8);
		_CheckExcept("_GetInfoForBTcc::regbit(0)::_GetImm");
	} else
		vcpuins.pimm = vcpuins.pr;
	switch (rmbit) {
	case 16:
		if (vcpuins.flagmem && regbit) {
			bitoffset = d_nsbit16(vcpuins.pimm);
			if (bitoffset >= 0)
				vcpuins.lrm += 2 * (bitoffset / 16);
			else
				vcpuins.lrm += 2 * ((bitoffset - 15) / 16);
			bitoffset = ((t_nubit16)bitoffset) % 16;
		} else
			bitoffset = (d_nubit16(vcpuins.pimm) % 16);
		vcpuins.pimm = (1 << bitoffset);
		vcpuins.prm = vramAddr(_GetPhysicalFromLinear(vcpuins.lrm, _GetCPL, 0x00));
		_CheckExcept("_GetInfoForBTcc::rmbit(16)::_GetPhysicalFromLinear");
		break;
	case 32:
		if (vcpuins.flagmem && regbit) {
			bitoffset = d_nsbit32(vcpuins.pimm);
			if (bitoffset >= 0)
				vcpuins.lrm += 4 * (bitoffset / 32);
			else
				vcpuins.lrm += 4 * ((bitoffset - 31) / 32);
			bitoffset = ((t_nubit32)bitoffset) % 32;
		} else
			bitoffset = (d_nubit32(vcpuins.pimm) % 32);
		vcpuins.pimm = (1 << bitoffset);
		vcpuins.prm = vramAddr(_GetPhysicalFromLinear(vcpuins.lrm, _GetCPL, 0x00));
		_CheckExcept("_GetInfoForBTcc::rmbit(32)::_GetPhysicalFromLinear");
		break;
	default:
		CaseError("_GetInfoForBTcc::rmbit");
		return;
	}
}

DONE BT(t_vaddrcc dest, t_nubitcc bitoperand, t_nubit8 bit)
{
	switch (bit) {
	case 16:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, GetBit(d_nubit16(dest), bitoperand));
		break;
	case 32:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, GetBit(d_nubit32(dest), bitoperand));
		break;
	default:
		CaseError("BT::bit");
		break;
	}
}
DONE BTC(t_vaddrcc dest, t_nubitcc bitoperand, t_nubit8 bit)
{
	switch (bit) {
	case 16:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, GetBit(d_nubit16(dest), bitoperand));
		MakeBit(d_nubit16(dest), bitoperand, !_GetEFLAGS_CF);
		break;
	case 32:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, GetBit(d_nubit32(dest), bitoperand));
		MakeBit(d_nubit32(dest), bitoperand, !_GetEFLAGS_CF);
		break;
	default:
		CaseError("BT::bit");
		break;
	}
}
DONE BTR(t_vaddrcc dest, t_nubitcc bitoperand, t_nubit8 bit)
{
	switch (bit) {
	case 16:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, GetBit(d_nubit16(dest), bitoperand));
		ClrBit(d_nubit16(dest), bitoperand);
		break;
	case 32:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, GetBit(d_nubit32(dest), bitoperand));
		ClrBit(d_nubit32(dest), bitoperand);
		break;
	default:
		CaseError("BT::bit");
		break;
	}
}
DONE BTS(t_vaddrcc dest, t_nubitcc bitoperand, t_nubit8 bit)
{
	switch (bit) {
	case 16:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, GetBit(d_nubit16(dest), bitoperand));
		SetBit(d_nubit16(dest), bitoperand);
		break;
	case 32:
		MakeBit(vcpu.eflags, VCPU_EFLAGS_CF, GetBit(d_nubit32(dest), bitoperand));
		SetBit(d_nubit32(dest), bitoperand);
		break;
	default:
		CaseError("BT::bit");
		break;
	}
}

DONE BT_RM16_R16()
{
	t_nsbitcc bitoffset = 0;
	t_vaddrcc dest = (t_vaddrcc)NULL;
	i386(0x0fa3) {
		vcpu.eip++;
		_GetInfoForBTcc(_GetOperandSize, _GetOperandSize, 0x00);
		_CheckExcept("BT_RM16_R16::_GetInfoForBTcc");
		BT(vcpuins.prm, vcpuins.pimm, _GetOperandSize);
	} else
		UndefinedOpcode();
}
DONE BTS_RM16_R16()
{
	i386(0x0fab) {
		vcpu.eip++;
		_GetInfoForBTcc(_GetOperandSize, _GetOperandSize, 0x01);
		_CheckExcept("BTS_RM16_R16::_GetInfoForBTcc");
		BTS(vcpuins.prm, vcpuins.pimm, _GetOperandSize);
	} else
		UndefinedOpcode();
}
DONE BTR_RM16_R16()
{
	i386(0x0fb3) {
		vcpu.eip++;
		_GetInfoForBTcc(_GetOperandSize, _GetOperandSize, 0x01);
		_CheckExcept("BTR_RM16_R16::_GetInfoForBTcc");
		BTS(vcpuins.prm, vcpuins.pimm, _GetOperandSize);
	} else
		UndefinedOpcode();
}
TODO INS_0F_BA()
{
	t_nubit8 modrm = 0x00;
	i386(0x0fba) {
		vcpu.eip++;
		modrm = (t_nubit8)_GetCode(vcpu.eip, 8);
		_CheckExcept("INS_0F_BA::_GetCode");
		if (_GetModRM_REG(modrm) == 4)
			_GetInfoForBTcc(0, _GetOperandSize, 0x00);
		else
			_GetInfoForBTcc(0, _GetOperandSize, 0x01);
		_CheckExcept("INS_0F_BA::_GetInfoForBTcc");
		switch (vcpuins.pr) {
		case 0: UndefinedOpcode();break;
		case 1: UndefinedOpcode();break;
		case 2: UndefinedOpcode();break;
		case 3: UndefinedOpcode();break;
		case 4: /* BT_RM16_I8 */
			BT(vcpuins.prm, vcpuins.pimm, _GetOperandSize);
			break;
		case 5: /* BTS_RM16_I8 */
			BTS(vcpuins.prm, vcpuins.pimm, _GetOperandSize);
			break;
		case 6: /* BTR_RM16_I8 */
			BTR(vcpuins.prm, vcpuins.pimm, _GetOperandSize);
			break;
		case 7: /* BTC_RM16_I8 */
			BTC(vcpuins.prm, vcpuins.pimm, _GetOperandSize);
			break;
		default: CaseError("INS_0F_BA::pr");return;
		}
	} else
		UndefinedOpcode();
}
DONE BTC_RM16_R16()
{
	i386(0x0fbb) {
		vcpu.eip++;
		_GetInfoForBTcc(_GetOperandSize, _GetOperandSize, 0x01);
		_CheckExcept("BTC_RM16_R16::_GetInfoForBTcc");
		BTC(vcpuins.prm, vcpuins.pimm, _GetOperandSize);
	} else
		UndefinedOpcode();
}
DONE BSF_R16_RM16()
{
	t_nubit16 src16 = 0x0000;
	t_nubit32 src32 = 0x00000000;
	t_nubitcc temp = 0;
	i386(0x0fbc) {
		vcpu.eip++;
		switch (_GetOperandSize) {
		case 16:
			_GetModRegRM(16, 16, 0x00);
			_CheckExcept("BSF_R16_RM16::OperandSize(16)::_GetModRegRM");
			src16 = d_nubit16(vcpuins.prm);
			if (!src16)
				_SetEFLAGS_ZF;
			else {
				_ClrEFLAGS_ZF;
				while(!GetBit(src16, (1 << temp))) temp++;
				d_nubit16(vcpuins.pr) = (t_nubit16)temp;
			}
			break;
		case 32:
			_GetModRegRM(32, 32, 0x00);
			_CheckExcept("BSF_R16_RM16::OperandSize(32)::_GetModRegRM");
			src32 = d_nubit32(vcpuins.prm);
			if (!src32)
				_SetEFLAGS_ZF;
			else {
				_ClrEFLAGS_ZF;
				while(!GetBit(src32, (1 << temp))) temp++;
				d_nubit32(vcpuins.pr) = (t_nubit32)temp;
			}
			break;
		default:
			CaseError("BSF_R16_RM16::OperandSize");
			return;
		}
	} else
		UndefinedOpcode();
}
DONE BSR_R16_RM16()
{
	t_nubit16 src16 = 0x0000;
	t_nubit32 src32 = 0x00000000;
	t_nubitcc temp = 0;
	i386(0x0fbd) {
		vcpu.eip++;
		switch (_GetOperandSize) {
		case 16:
			_GetModRegRM(16, 16, 0x00);
			_CheckExcept("BSR_R16_RM16::OperandSize(16)::_GetModRegRM");
			src16 = d_nubit16(vcpuins.prm);
			if (!src16)
				_SetEFLAGS_ZF;
			else {
				_ClrEFLAGS_ZF;
				temp = 15;
				while(!GetBit(src16, (1 << temp))) temp--;
				d_nubit16(vcpuins.pr) = (t_nubit16)temp;
			}
			break;
		case 32:
			_GetModRegRM(32, 32, 0x00);
			_CheckExcept("BSR_R16_RM16::OperandSize(32)::_GetModRegRM");
			src32 = d_nubit32(vcpuins.prm);
			if (!src32)
				_SetEFLAGS_ZF;
			else {
				_ClrEFLAGS_ZF;
				temp = 31;
				while(!GetBit(src32, (1 << temp))) temp--;
				d_nubit32(vcpuins.pr) = (t_nubit32)temp;
			}
			break;
		default:
			CaseError("BSR_R16_RM16::OperandSize");
			return;
		}
	} else
		UndefinedOpcode();
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
	vcpuins.prefix_sreg = PREFIX_SREG_NONE;
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
	if (vcpuins.flaginsloop || vcpuins.except) {
		vcpu.cs = vcpuins.oldcs;
		vcpu.ss = vcpuins.oldss;
		vcpu.eip = vcpuins.oldeip;
		vcpu.esp = vcpuins.oldesp;
	}
	if (vcpuins.except) {
		vapiCallBackMachineStop();
	}
}
static void ExecIns()
{
	t_nubit8 opcode;
	ExecInsInit();
//	if (vcpu.eip > 0xffff) vapiPrint("here!\n");
	do {
		opcode = (t_nubit8)_GetCode(vcpu.eip, 8);
		_CheckExcept("ExecIns::_GetCode");
		if (vcpuins.except) break;
		ExecFun(vcpuins.table[opcode]);
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

DONE QDX()
{
	vcpu.eip++;
	_GetImm(8);
	_CheckExcept("QDX");
	switch (d_nubit8(vcpuins.pimm)) {
	case 0x00:
	case 0xff:
		vapiPrint("\nNXVM CPU STOP at CS:%04X IP:%08X INS:QDX IMM:%02X\n",
			vcpu.cs.selector,vcpu.eip,d_nubit8(vcpuins.pimm));
		vapiPrint("This happens because of the special instruction.\n");
		vapiCallBackMachineStop();
		break;
	case 0x01:
	case 0xfe:
		vapiPrint("\nNXVM CPU RESET at CS:%04X IP:%08X INS:QDX IMM:%02X\n",
			vcpu.cs.selector,vcpu.eip,d_nubit8(vcpuins.pimm));
		vapiPrint("This happens because of the special instruction.\n");
		vapiCallBackMachineReset();
		break;
	case 0x02:
		vapiPrint("\nNXVM CPU PRINT16 at CS:%04X IP:%08X INS:QDX IMM:%02X\n",
			vcpu.cs.selector,vcpu.eip,d_nubit8(vcpuins.pimm));
		vapiCallBackDebugPrintRegs(16);
		break;
	case 0x03:
		vapiPrint("\nNXVM CPU PRINT32 at CS:%04X IP:%08X INS:QDX IMM:%02X\n",
			vcpu.cs.selector,vcpu.eip,d_nubit8(vcpuins.pimm));
		vapiCallBackDebugPrintRegs(32);
		break;
	default:
		qdbiosExecInt(d_nubit8(vcpuins.pimm));
		MakeBit(vramVarWord(_ss,_sp + 4), VCPU_EFLAGS_ZF, _GetZF);
		MakeBit(vramVarWord(_ss,_sp + 4), VCPU_EFLAGS_CF, _GetEFLAGS_CF);
		break;
	}
}

void vcpuinsInit()
{
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
	vcpuins.table_0f[0xa3] = (t_faddrcc)BT_RM16_R16;
	vcpuins.table_0f[0xa4] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xa5] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xa6] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xa7] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xa8] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xa9] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xaa] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xab] = (t_faddrcc)BTS_RM16_R16;
	vcpuins.table_0f[0xac] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xad] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xae] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xaf] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb0] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb1] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb2] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb3] = (t_faddrcc)BTR_RM16_R16;
	vcpuins.table_0f[0xb4] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb5] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb6] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb7] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb8] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xb9] = (t_faddrcc)UndefinedOpcode;
	vcpuins.table_0f[0xba] = (t_faddrcc)INS_0F_BA;
	vcpuins.table_0f[0xbb] = (t_faddrcc)BTC_RM16_R16;
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
	vcpuins.rm = vcpuins.r = vcpuins.imm = (t_vaddrcc)NULL;
	vcpuins.opr1 = vcpuins.opr2 = vcpuins.result = vcpuins.bit = 0;
}
void vcpuinsRefresh()
{
	ExecIns();
	ExecInt();
}
void vcpuinsFinal() {}
