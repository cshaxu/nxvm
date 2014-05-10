/* This file is a part of NXVM project. */

/* DEBUGGING OPTIONS ******************************************************* */
#define i386(n) if (1)
/* ************************************************************************* */

#include "../utils.h"
#include "device.h"

#include "vport.h"
#include "vram.h"
#include "vpic.h"

#include "vcpuins.h"

t_cpuins vcpuins;

#define UTILS_TRACE_VAR    trace
#define UTILS_TRACE_ERROR  vcpuins.data.except
#define UTILS_TRACE_SETERR (_SetExcept_CE(0xffffffff))

static t_utils_trace UTILS_TRACE_VAR;

/* indicates functions not implemented */
#define _______todo static void
/* prints untested code path */
#define _new_code_path_ do { \
PRINTF("NEW CODE PATH\n");utilsTracePrint(&(UTILS_TRACE_VAR));} while (0)

/* stack pointer size */
#define _GetStackSize   (vcpu.data.ss.seg.data.big ? 4 : 2)
/* operand size */
#define _GetOperandSize ((vcpu.data.cs.seg.exec.defsize ^ vcpuins.data.prefix_oprsize) ? 4 : 2)
/* address size of the source operand */
#define _GetAddressSize ((vcpu.data.cs.seg.exec.defsize ^ vcpuins.data.prefix_addrsize) ? 4 : 2)
/* if opcode indicates a prefix */
#define _SetExcept_DE(n) (SetBit(vcpuins.data.except, VCPUINS_EXCEPT_DE), vcpuins.data.excode = (n), PRINTF("#DE(%x) - divide error\n",    vcpuins.data.excode))
#define _SetExcept_PF(n) (SetBit(vcpuins.data.except, VCPUINS_EXCEPT_PF), vcpuins.data.excode = (n), PRINTF("#PF(%x) - page fault\n",      vcpuins.data.excode))
#define _SetExcept_GP(n) (SetBit(vcpuins.data.except, VCPUINS_EXCEPT_GP), vcpuins.data.excode = (n), PRINTF("#GP(%x) - general protect\n", vcpuins.data.excode))
#define _SetExcept_SS(n) (SetBit(vcpuins.data.except, VCPUINS_EXCEPT_SS), vcpuins.data.excode = (n), PRINTF("#SS(%x) - stack segment\n",   vcpuins.data.excode))
#define _SetExcept_UD(n) (SetBit(vcpuins.data.except, VCPUINS_EXCEPT_UD), vcpuins.data.excode = (n), PRINTF("#UD(%x) - undefined\n",       vcpuins.data.excode))
#define _SetExcept_NP(n) (SetBit(vcpuins.data.except, VCPUINS_EXCEPT_NP), vcpuins.data.excode = (n), PRINTF("#NP(%x) - not present\n",     vcpuins.data.excode))
#define _SetExcept_BR(n) (SetBit(vcpuins.data.except, VCPUINS_EXCEPT_BR), vcpuins.data.excode = (n), PRINTF("#BR(%x) - boundary\n",        vcpuins.data.excode))
#define _SetExcept_TS(n) (SetBit(vcpuins.data.except, VCPUINS_EXCEPT_TS), vcpuins.data.excode = (n), PRINTF("#TS(%x) - task state\n",      vcpuins.data.excode))
#define _SetExcept_NM(n) (SetBit(vcpuins.data.except, VCPUINS_EXCEPT_NM), vcpuins.data.excode = (n), PRINTF("#NM(%x) - divide error\n",    vcpuins.data.excode))
#define _SetExcept_CE(n) (SetBit(vcpuins.data.except, VCPUINS_EXCEPT_CE), vcpuins.data.excode = (n), PRINTF("#CE(%x) - internal error\n",  vcpuins.data.excode))

/* memory management unit */
/* kernel memory accessing */
/* read content from reference */
static void _kma_read_ref(t_vaddrcc ref, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_kma_read_ref");
    MEMCPY((void *) rdata, (void *) ref, byte);
    _ce;
}
/* write content to reference */
static void _kma_write_ref(t_vaddrcc ref, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_kma_write_ref");
    MEMCPY((void *) ref, (void *) rdata, byte);
    _ce;
}
/* read content from physical */
static void _kma_read_physical(t_nubit32 physical, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_kma_read_physical");
    _chr(vramReadPhysical(physical, rdata, byte));
    _ce;
}
/* write content to physical */
static void _kma_write_physical(t_nubit32 physical, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_kma_write_physical");
    _chr(vramWritePhysical(physical, rdata, byte));
    _ce;
}
/* translate linear to physical - paging mechanism*/
static t_nubit32 _kma_physical_linear(t_nubit32 linear, t_nubit8 byte, t_bool write, t_nubit8 vpl) {
    t_nubit32 ppde, ppte; /* page table entries */
    t_nubit32 cpde, cpte;
    _cb("_t_kma_physical_linear");
    if (_GetLinear_Offset(linear) > GetMax32(_GetPageSize - byte)) _impossible_rz_;
    if (!_IsPaging) {
        _ce;
        return linear;
    }
    ppde = _GetCR3_Base + _GetLinear_Dir(linear) * 4;
    _chrz(_kma_read_physical(ppde, GetRef(cpde), 4));
    if (!_IsPageEntryPresent(cpde)) {
        _bb("!PageDirEntryPresent");
        vcpu.data.cr2 = linear;
        _chrz(_SetExcept_PF(_MakePageFaultErrorCode(0, write, (vpl == 3))));
        _be;
    }
    if (vpl == 0x03) {
        _bb("vpl(3)");
        if (!_GetPageEntry_US(cpde)) {
            _bb("PageDirEntry_US(0)");
            vcpu.data.cr2 = linear;
            _chrz(_SetExcept_PF(_MakePageFaultErrorCode(1, write, 1)));
            _be;
        }
        if (write && !_IsPageEntryWritable(cpde)) {
            _bb("write,!PageDirEntryWritable");
            vcpu.data.cr2 = linear;
            _chrz(_SetExcept_PF(_MakePageFaultErrorCode(1, 1, 1)));
            _be;
        }
        _be;
    }
    _SetPageEntry_A(cpde);
    _chrz(_kma_write_physical(ppde, GetRef(cpde), 4));
    ppte = _GetPageEntry_Base(cpde) + _GetLinear_Page(linear) * 4;
    _chrz(_kma_read_physical(ppte, GetRef(cpte), 4));
    if (!_IsPageEntryPresent(cpte)) {
        _bb("!PageTabEntryPresent");
        vcpu.data.cr2 = linear;
        _chrz(_SetExcept_PF(_MakePageFaultErrorCode(0, write, (vpl == 3))));
        _be;
    }
    if (vpl == 0x03) {
        _bb("vpl(3)");
        if (!_GetPageEntry_US(cpte)) {
            _bb("PageTabEntry_US(0)");
            vcpu.data.cr2 = linear;
            _chrz(_SetExcept_PF(_MakePageFaultErrorCode(1, write, 1)));
            _be;
        }
        if (write && !_IsPageEntryWritable(cpte)) {
            _bb("write,!PageTabEntryWritable");
            vcpu.data.cr2 = linear;
            _chrz(_SetExcept_PF(_MakePageFaultErrorCode(1, 1, 1)));
            _be;
        }
        _be;
    }
    _SetPageEntry_A(cpte);
    if (write) _SetPageEntry_D(cpte);
    _chrz(_kma_write_physical(ppte, GetRef(cpte), 4));
    _ce;
    return (_GetPageEntry_Base(cpte) + _GetLinear_Offset(linear));
}
/* translate logical to linear - segmentation mechanism */
static t_nubit32 _kma_linear_logical(t_cpu_data_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force) {
    t_nubit32 linear;
    t_nubit32 upper, lower;
    _cb("_kma_linear_logical");
    switch (rsreg->sregtype) {
    case SREG_CODE:
        _bb("sregtype(SREG_CODE)");
        if (!rsreg->flagValid) _impossible_rz_;
        if (_IsProtected) {
            _bb("Protected");
            if (!force) {
                _bb("force");
                if (write) {
                    _bb("write");
                    _chrz(_SetExcept_GP(0));
                    _be;
                }
                if (!rsreg->seg.exec.readable) {
                    _bb("!readable");
                    _chrz(_SetExcept_GP(0));
                    _be;
                }
                _be;
            }
            _be;
        }
        lower = 0x00000000;
        upper = rsreg->limit;
        _be;
        break;
    case SREG_STACK:
        _bb("sregtype(SREG_STACK)");
        if (!rsreg->flagValid) _impossible_rz_;
        if (_IsProtected) {
            _bb("Protected");
            if (rsreg->seg.executable || !rsreg->seg.data.writable) _impossible_rz_;
            _be;
        }
        if (rsreg->seg.data.expdown) {
            lower = rsreg->limit + 1;
            upper = rsreg->seg.data.big ? 0xffffffff : 0x0000ffff;
        } else {
            lower = 0x00000000;
            upper = rsreg->limit;
        }
        _be;
        break;
    case SREG_DATA:
        _bb("sregtype(SREG_DATA)");
        if (!rsreg->flagValid) {
            _bb("selector(null)");
            _chrz(_SetExcept_GP(0));
            _be;
        }
        if (_IsProtected) {
            _bb("Protected");
            if (_IsSelectorNull(rsreg->selector)) {
                _bb("selector(null)");
                _chrz(_SetExcept_GP(0));
                _be;
            }
            if (rsreg->seg.executable && !rsreg->seg.exec.readable) _impossible_rz_;
            if (!force) {
                _bb("force");
                if (write) {
                    _bb("write");
                    if (rsreg->seg.executable) {
                        _bb("executable");
                        _chrz(_SetExcept_GP(0));
                        _be;
                    } else {
                        _bb("!executable");
                        if (!rsreg->seg.data.writable) {
                            _bb("!writable");
                            _chrz(_SetExcept_GP(0));
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
        _be;
        break;
    case SREG_GDTR:
        _bb("sregtype(SREG_GDTR)");
        if (!_GetCR0_PE) _impossible_rz_;
        lower = 0x00000000;
        upper = rsreg->limit;
        _be;
        break;
    case SREG_IDTR:
        _bb("sregtype(SREG_IDTR)");
        lower = 0x00000000;
        upper = rsreg->limit;
        _be;
        break;
    case SREG_LDTR:
        _bb("sregtype(SREG_LDTR)");
        if (!_GetCR0_PE) _impossible_rz_;
        if (_GetSelector_TI(rsreg->selector)) _impossible_rz_;
        if (!rsreg->flagValid || _IsSelectorNull(rsreg->selector)) {
            _bb("selector(null)");
            _chrz(_SetExcept_GP(0));
            _be;
        }
        lower = 0x00000000;
        upper = rsreg->limit;
        _be;
        break;
    case SREG_TR:
        _bb("sregtype(SREG_TR)");
        if (!_GetCR0_PE) _impossible_rz_;
        if (!rsreg->flagValid || _IsSelectorNull(rsreg->selector)) _impossible_rz_;
        if (_GetSelector_TI(rsreg->selector)) _impossible_rz_;
        lower = 0x00000000;
        upper = rsreg->limit;
        _be;
        break;
    default:
        _impossible_rz_;
    }
    linear = rsreg->base + offset;
    if (offset < lower || offset > upper - (byte - 1)) {
        _bb("offset(<lower/>upper)");
        switch (rsreg->sregtype) {
        case SREG_STACK:
            _bb("sregtype(SREG_STACK)");
            _chrz(_SetExcept_SS(0));
            _be;
            break;
        case SREG_TR:
            _bb("sregtype(SREG_TR)");
            _chrz(_SetExcept_TS(rsreg->selector));
            _be;
            break;
        default:
            _bb("sregtype(default)");
            _chrz(_SetExcept_GP(0));
            _be;
            break;
        }
        _be;
    }
    _ce;
    return linear;
}
/* read content from logical */
static void _kma_read_linear(t_nubit32 linear, t_vaddrcc rdata, t_nubit8 byte, t_nubit8 vpl, t_bool force) {
    t_nubit32 phy1, phy2;
    t_nubit8  byte1, byte2;
    _cb("_kma_read_logical");
    if (_GetLinear_Offset(linear) > GetMax32(_GetPageSize - byte)) {
        _bb("Linear_Offset(>PageSize)");
        byte1 = _GetPageSize - _GetLinear_Offset(linear);
        byte2 = byte - byte1;
        _chr(phy1 = _kma_physical_linear(linear        , byte1, 0, vpl));
        _chr(phy2 = _kma_physical_linear(linear + byte1, byte2, 0, vpl));
        _chr(_kma_read_physical(phy1, rdata        , byte1));
        _chr(_kma_read_physical(phy2, rdata + byte1, byte2));
        _be;
    } else {
        _bb("Linear_Offset(<=PageSize)");
        byte1 = byte;
        _chr(phy1 = _kma_physical_linear(linear, byte1, 0, vpl));
        _chr(_kma_read_physical(phy1, rdata, byte1));
        _be;
    }
    _ce;
}
/* write content to logical */
static void _kma_write_linear(t_nubit32 linear, t_vaddrcc rdata, t_nubit8 byte, t_nubit8 vpl, t_bool force) {
    t_nubit32 phy1, phy2;
    t_nubit8  byte1, byte2;
    _cb("_kma_write_linear");
    if (_GetLinear_Offset(linear) > GetMax32(_GetPageSize - byte)) {
        _bb("Linear_Offset(>PageSize)");
        byte1 = _GetPageSize - _GetLinear_Offset(linear);
        byte2 = byte - byte1;
        _chr(phy1 = _kma_physical_linear(linear, byte1, 1, vpl));
        _chr(phy2 = _kma_physical_linear(linear + byte1, byte2, 1, vpl));
        _chr(_kma_write_physical(phy1, rdata, byte1));
        _chr(_kma_write_physical(phy2, rdata + byte1, byte2));
        _be;
    } else {
        _bb("Linear_Offset(<=PageSize)");
        byte1 = byte;
        _chr(phy1 = _kma_physical_linear(linear, byte1, 1, vpl));
        _chr(_kma_write_physical(phy1, rdata, byte1));
        _be;
    }
    _ce;
}
/* read content from logical */
static void _kma_read_logical(t_cpu_data_sreg *rsreg, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte, t_nubit8 vpl, t_bool force) {
    /* t_nubitcc i; */
    t_nubit32 linear;
    _cb("_kma_read_logical");
    _chr(linear = _kma_linear_logical(rsreg, offset, byte, 0, vpl, force));
    _chr(_kma_read_linear(linear, rdata, byte, vpl, force));
    if (!force) {
        _bb("!force");
        vcpuins.data.mem[vcpuins.data.msize].flagWrite = False;
        vcpuins.data.mem[vcpuins.data.msize].data = 0;
        MEMCPY((void *) GetRef(vcpuins.data.mem[vcpuins.data.msize].data), (void *) rdata, byte);
        vcpuins.data.mem[vcpuins.data.msize].byte = byte;
        vcpuins.data.mem[vcpuins.data.msize].linear = linear;
        if (vcpuins.data.flagWR) {
            if (vcpuins.data.wrLinear >= vcpuins.data.mem[vcpuins.data.msize].linear &&
                    vcpuins.data.wrLinear < vcpuins.data.mem[vcpuins.data.msize].linear + byte) {
                PRINTF("Watch point caught at L%08x: READ %01x BYTES OF DATA=%08x FROM L%08x\n", vcpuins.data.linear,
                       vcpuins.data.mem[vcpuins.data.msize].byte,
                       vcpuins.data.mem[vcpuins.data.msize].data,
                       vcpuins.data.mem[vcpuins.data.msize].linear);
            }
        }
        /* for (i = 0;i < vcpuins.data.msize;++i) {
            if (vcpuins.data.mem[i].flagWrite == vcpuins.data.mem[vcpuins.data.msize].flagWrite &&
                vcpuins.data.mem[i].linear == vcpuins.data.mem[vcpuins.data.msize].linear) {
                _bb("mem(same)");
                _impossible_rz_;
                _ce;
            }
        } */
        vcpuins.data.msize++;
        if (vcpuins.data.msize == 0x20) _impossible_r_;
        _be;
    }
    _ce;
}
/* write content to logical */
static void _kma_write_logical(t_cpu_data_sreg *rsreg, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte, t_nubit8 vpl, t_bool force) {
    /* t_nubitcc i; */
    t_nubit32 linear;
    _cb("_kma_write_logical");
    _chr(linear = _kma_linear_logical(rsreg, offset, byte, 1, vpl, force));
    _chr(_kma_write_linear(linear, rdata, byte, vpl, force));
    if (!force) {
        _bb("!force");
        vcpuins.data.mem[vcpuins.data.msize].flagWrite = True;
        vcpuins.data.mem[vcpuins.data.msize].data = 0;
        MEMCPY((void *) GetRef(vcpuins.data.mem[vcpuins.data.msize].data), (void *) rdata, byte);
        vcpuins.data.mem[vcpuins.data.msize].byte = byte;
        vcpuins.data.mem[vcpuins.data.msize].linear = linear;
        if (vcpuins.data.flagWW) {
            if (vcpuins.data.wwLinear >= vcpuins.data.mem[vcpuins.data.msize].linear &&
                    vcpuins.data.wwLinear < vcpuins.data.mem[vcpuins.data.msize].linear + byte) {
                PRINTF("Watch point caught at L%08x: WRITE %01x BYTES OF DATA=%08x TO L%08x\n", vcpuins.data.linear,
                       vcpuins.data.mem[vcpuins.data.msize].byte,
                       vcpuins.data.mem[vcpuins.data.msize].data,
                       vcpuins.data.mem[vcpuins.data.msize].linear);
            }
        }
        /* for (i = 0;i < vcpuins.data.msize;++i) {
            if (vcpuins.data.mem[i].flagWrite == vcpuins.data.mem[vcpuins.data.msize].flagWrite &&
                vcpuins.data.mem[i].linear == vcpuins.data.mem[vcpuins.data.msize].linear) {
                _bb("mem(same)");
                _impossible_r_;
                _ce;
            }
        } */
        vcpuins.data.msize++;
        if (vcpuins.data.msize == 0x20) _impossible_r_;
        _be;
    }
    _ce;
}
/* test logical accessing */
static void _kma_test_linear(t_nubit32 linear, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force) {
    t_nubit32 phy1, phy2;
    t_nubit8  byte1, byte2;
    _cb("_kma_test_linear");
    if (_GetLinear_Offset(linear) > GetMax32(_GetPageSize - byte)) {
        _bb("Linear_Offset(>PageSize)");
        byte1 = _GetPageSize - _GetLinear_Offset(linear);
        byte2 = byte - byte1;
        _chr(phy1 = _kma_physical_linear(linear        , byte1, write, vpl));
        _chr(phy2 = _kma_physical_linear(linear + byte1, byte2, write, vpl));
        _be;
    } else {
        _bb("Linear_Offset(<=PageSize)");
        byte1 = byte;
        _chr(phy1 = _kma_physical_linear(linear, byte1, write, vpl));
        _be;
    }
    _ce;
}
static void _kma_test_logical(t_cpu_data_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force) {
    t_nubit32 linear;
    _cb("_kma_test_logical");
    _chr(linear = _kma_linear_logical(rsreg, offset, byte, write, vpl, force));
    _ce;
}
static void _kma_test_access(t_cpu_data_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force) {
    t_nubit32 linear;
    _cb("_kma_test_access");
    _chr(linear = _kma_linear_logical(rsreg, offset, byte, write, vpl, force));
    _chr(_kma_test_linear(linear, byte, write, vpl, force));
    _ce;
}

/* general memory accessing */
static void _m_read_ref(t_vaddrcc ref, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_m_read_ref");
    /* _m_write_ref allows in-module reads only */
    if (ref < (t_vaddrcc)(&vcpu) && ref >= (t_vaddrcc)(&vcpu) + sizeof(t_cpu) &&
            ref < (t_vaddrcc)(&vcpuins) && ref >= (t_vaddrcc)(&vcpuins) + sizeof(t_cpuins)) {
        _impossible_r_;
    }
    _chr(_kma_read_ref(ref, rdata, byte));
    _ce;
}
static void _m_write_ref(t_vaddrcc ref, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_m_write_ref");
    /* _m_write_ref allows in-module writes only */
    if (ref < (t_vaddrcc)(&vcpu) && ref >= (t_vaddrcc)(&vcpu) + sizeof(t_cpu) &&
            ref < (t_vaddrcc)(&vcpuins) && ref >= (t_vaddrcc)(&vcpuins) + sizeof(t_cpuins)) {
        _impossible_r_;
    }
    _chr(_kma_write_ref(ref, rdata, byte));
    _ce;
}
static void _m_read_logical(t_cpu_data_sreg *rsreg, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_m_read_logical");
    _chr(_kma_read_logical(rsreg, offset, rdata, byte, _GetCPL, 0));
    _ce;
}
static void _m_write_logical(t_cpu_data_sreg *rsreg, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_m_write_logical");
    _chr(_kma_write_logical(rsreg, offset, rdata, byte, _GetCPL, 0));
    _ce;
}
static void _m_test_access(t_cpu_data_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write) {
    t_vaddrcc ref = 0;
    _cb("_m_test_access");
    _chr(_kma_test_access(rsreg, offset, byte, write, _GetCPL, 0));
    _ce;
}
static void _m_test_logical(t_cpu_data_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write) {
    t_vaddrcc ref = 0;
    _cb("_m_test_logical");
    _chr(_kma_test_logical(rsreg, offset, byte, write, _GetCPL, 0));
    _ce;
}

static void _m_read_rm(t_nubit8 byte) {
    _cb("_m_read_rm");
    vcpuins.data.crm = 0;
    if (vcpuins.data.flagMem)
        _chr(_m_read_logical(vcpuins.data.mrm.rsreg, vcpuins.data.mrm.offset, GetRef(vcpuins.data.crm), byte));
    else
        _chr(_m_read_ref(vcpuins.data.rrm, GetRef(vcpuins.data.crm), byte));
    _ce;
}
static void _m_write_rm(t_nubit8 byte) {
    _cb("_m_write_rm");
    if (vcpuins.data.flagMem)
        _chr(_m_write_logical(vcpuins.data.mrm.rsreg, vcpuins.data.mrm.offset, GetRef(vcpuins.data.crm), byte));
    else
        _chr(_m_write_ref(vcpuins.data.rrm, GetRef(vcpuins.data.crm), byte));
    _ce;
}

/* segment accessing unit: _s_ */
/* kernel segment accessing */
static void _ksa_read_idt(t_nubit8 intid, t_vaddrcc rdata) {
    _cb("_s_read_idt");
    if (!_GetCR0_PE) {
        _bb("CR0_PE(0)");
        if (GetMax16(intid * 4 + 3) > GetMax16(vcpu.data.idtr.limit)) _impossible_r_;
        _chr(_kma_read_logical(&vcpu.data.idtr, (intid * 4), rdata, 4, 0x00, 0));
        _be;
    } else {
        _bb("CR0_PE(1)");
        if (GetMax16(intid * 8 + 7) > GetMax16(vcpu.data.idtr.limit)) _impossible_r_;
        _chr(_kma_read_logical(&vcpu.data.idtr, (intid * 8), rdata, 8, 0x00, 0));
        _be;
    }
    _ce;
}
static void _ksa_read_ldt(t_nubit16 selector, t_vaddrcc rdata) {
    _cb("_ksa_read_ldt");
    if (!_GetCR0_PE) _impossible_r_;
    if (!_GetSelector_TI(selector)) _impossible_r_;
    if (GetMax16(_GetSelector_Offset(selector) + 7) > GetMax16(vcpu.data.ldtr.limit)) {
        _bb("Selector_Offset(>ldtr.limit)");
        _chr(_SetExcept_GP(selector));
        _be;
    }
    _chr(_kma_read_logical(&vcpu.data.ldtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
    _ce;
}
static void _ksa_read_gdt(t_nubit16 selector, t_vaddrcc rdata) {
    _cb("_ksa_read_gdt");
    if (!_GetCR0_PE) _impossible_r_;
    if (_GetSelector_TI(selector)) _impossible_r_;
    if (GetMax16(_GetSelector_Offset(selector) + 7) > GetMax16(vcpu.data.gdtr.limit)) {
        _bb("Selector_Offset(>gdtr.limit)");
        _chr(_SetExcept_GP(selector));
        _be;
    }
    _chr(_kma_read_logical(&vcpu.data.gdtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
    _ce;
}
static void _ksa_read_xdt(t_nubit16 selector, t_vaddrcc rdata) {
    _cb("_ksa_read_xdt");
    if (!_GetCR0_PE) _impossible_r_;
    if (_GetSelector_TI(selector)) {
        _bb("Selector_TI");
        _chr(_ksa_read_ldt(selector, rdata));
        _be;
    } else {
        _bb("!Selector_TI");
        _chr(_ksa_read_gdt(selector, rdata));
        _be;
    }
    _ce;
}
static void _ksa_write_ldt(t_nubit16 selector, t_vaddrcc rdata) {
    _cb("_ksa_write_ldt");
    if (!_GetCR0_PE) _impossible_r_;
    if (!_GetSelector_TI(selector)) _impossible_r_;
    if (GetMax16(_GetSelector_Offset(selector) + 7) > GetMax16(vcpu.data.ldtr.limit))
        _impossible_r_;
    _chr(_kma_write_logical(&vcpu.data.ldtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
    _ce;
}
static void _ksa_write_gdt(t_nubit16 selector, t_vaddrcc rdata) {
    _cb("_ksa_write_gdt");
    if (!_GetCR0_PE) _impossible_r_;
    if (_GetSelector_TI(selector)) _impossible_r_;
    if (GetMax16(_GetSelector_Offset(selector) + 7) > GetMax16(vcpu.data.gdtr.limit))
        _impossible_r_;
    _chr(_kma_write_logical(&vcpu.data.gdtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
    _ce;
}
static void _ksa_write_xdt(t_nubit16 selector, t_vaddrcc rdata) {
    _cb("_ksa_write_xdt");
    if (!_GetCR0_PE) _impossible_r_;
    if (_GetSelector_TI(selector)) {
        _bb("Selector_TI");
        _chr(_ksa_write_ldt(selector, rdata));
        _be;
    } else {
        _bb("!Selector_TI");
        _chr(_ksa_write_gdt(selector, rdata));
        _be;
    }
    _ce;
}
static void _ksa_load_sreg(t_cpu_data_sreg *rsreg, t_nubit16 selector) {
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
                _chr(_SetExcept_GP(0));
                _be;
            }
            _chr(_ksa_read_xdt(selector, GetRef(descriptor)));
            if (!_IsDescCode(descriptor)) {
                _bb("!DescCode");
                _chr(_SetExcept_GP(selector));
                _be;
            }
            if (!_IsDescPresent(descriptor)) {
                _bb("!DescPresent");
                _chr(_SetExcept_NP(selector));
                _be;
            }
            _SetDescUserAccessed(descriptor);
            _chr(_ksa_write_xdt(selector, GetRef(descriptor)));
            rsreg->flagValid = True;
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
            rsreg->flagValid = True;
            rsreg->base = (selector << 4);
            rsreg->selector = selector;
            if (_GetCR0_PE && _GetEFLAGS_VM) {
                rsreg->dpl = 0x03;
                rsreg->limit = 0x0000ffff;
            }
            _be;
        }
        _be;
        break;
    case SREG_DATA:
        _bb("sregtype(SREG_DATA)");
        if (_IsProtected) {
            _bb("Protected");
            if (_IsSelectorNull(selector)) {
                _bb("selector(null)");
                rsreg->flagValid = False;
                rsreg->selector = selector;
                _be;
            } else {
                _bb("selector(!null)");
                _chr(_ksa_read_xdt(selector, GetRef(descriptor)));
                if (!_IsDescData(descriptor) && !_IsDescCodeReadable(descriptor)) {
                    _bb("!DescData,!DescCodeReadable");
                    _chr(_SetExcept_GP(selector));
                    _be;
                }
                if (_IsDescData(descriptor) || _IsDescCodeNonConform(descriptor)) {
                    _bb("DescData/DescCodeNonConform)");
                    if (_GetSelector_RPL(selector) > _GetDesc_DPL(descriptor) ||
                            _GetCPL > _GetDesc_DPL(descriptor)) {
                        _bb("PL(fail)");
                        _chr(_SetExcept_GP(selector));
                        _be;
                    }
                    _be;
                }
                if (!_IsDescPresent(descriptor)) {
                    _bb("!DescPresent");
                    _chr(_SetExcept_NP(selector));
                    _be;
                }
                _SetDescUserAccessed(descriptor);
                _chr(_ksa_write_xdt(selector, GetRef(descriptor)));
                rsreg->flagValid = True;
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
            rsreg->flagValid = True;
            rsreg->selector = selector;
            rsreg->base = (selector << 4);
            if (_GetCR0_PE && _GetEFLAGS_VM) {
                rsreg->dpl = 0x03;
                rsreg->limit = 0x0000ffff;
            }
            _be;
        }
        _be;
        break;
    case SREG_STACK:
        _bb("sregtype(SREG_STACK)");
        if (_IsProtected) {
            _bb("Protected(1)");
            if (_IsSelectorNull(selector)) {
                _bb("selector(null)");
                _chr(_SetExcept_GP(0));
                _be;
            }
            if (_GetSelector_RPL(selector) != _GetCPL) {
                _bb("Selector_RPL(!CPL)");
                _chr(_SetExcept_GP(selector));
                _be;
            }
            _chr(_ksa_read_xdt(selector, GetRef(descriptor)));
            if (!_IsDescDataWritable(descriptor)) {
                _bb("!DescDataWritable");
                _chr(_SetExcept_GP(selector));
                _be;
            }
            if (_GetDesc_DPL(descriptor) != _GetCPL) {
                _bb("Desc_DPL(!CPL)");
                _chr(_SetExcept_GP(selector));
                _be;
            }
            if (!_IsDescPresent(descriptor)) {
                _bb("!DescPresent");
                _chr(_SetExcept_SS(selector));
                _be;
            }
            _SetDescUserAccessed(descriptor);
            _chr(_ksa_write_xdt(selector, GetRef(descriptor)));
            rsreg->flagValid = True;
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
            rsreg->flagValid = True;
            rsreg->selector = selector;
            rsreg->base = (selector << 4);
            if (_GetCR0_PE && _GetEFLAGS_VM) {
                rsreg->dpl = 0x03;
                rsreg->limit = 0x0000ffff;
            }
            _be;
        }
        _be;
        break;
    case SREG_TR:
        _bb("sregtype(SREG_TR)");
        if (!_IsProtected) _impossible_r_;
        if (_GetCPL) _impossible_r_;
        if (_GetSelector_TI(selector)) _impossible_r_;
        if (_IsSelectorNull(selector)) {
            _bb("selector(null)");
            _chr(_SetExcept_GP(0));
            _be;
        }
        _chr(_ksa_read_xdt(selector, GetRef(descriptor)));
        if (!_IsDescTSSAvl(descriptor)) {
            _bb("!DescTssAvl");
            _chr(_SetExcept_GP(selector));
            _be;
        }
        if (!_IsDescPresent(descriptor)) {
            _bb("!DescPresent");
            _chr(_SetExcept_NP(selector));
            _be;
        }
        _SetDescTSSBusy(descriptor);
        _chr(_ksa_write_xdt(selector, GetRef(descriptor)));
        rsreg->flagValid = True;
        rsreg->selector = selector;
        rsreg->base = (t_nubit32)_GetDescSeg_Base(descriptor);
        rsreg->dpl = (t_nubit4)_GetDesc_DPL(descriptor);
        rsreg->limit = (t_nubit32)((_IsDescSegGranularLarge(descriptor) ?
                                    (_GetDescSeg_Limit(descriptor) << 12 | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
        rsreg->sys.type = (t_nubit4)_GetDesc_Type(descriptor);
        _be;
        break;
    case SREG_LDTR:
        _bb("sregtype(SREG_LDTR)");
        if (!_IsProtected) _impossible_r_;
        if (_GetCPL) _impossible_r_;
        if (_GetSelector_TI(selector)) _impossible_r_;
        if (_IsSelectorNull(selector)) {
            _bb("selector(null)");
            rsreg->flagValid = False;
            rsreg->selector = selector;
            _be;
        } else {
            _bb("selector(!null)");
            _chr(_ksa_read_xdt(selector, GetRef(descriptor)));
            if (!_IsDescLDT(descriptor)) {
                _bb("descriptor(!LDT)");
                _chr(_SetExcept_GP(selector));
                _be;
            }
            if (!_IsDescPresent(descriptor)) {
                _bb("descriptor(!P)");
                _chr(_SetExcept_NP(selector));
                _be;
            }
            rsreg->flagValid = True;
            rsreg->selector = selector;
            rsreg->base = (t_nubit32)_GetDescSeg_Base(descriptor);
            rsreg->dpl = (t_nubit4)_GetDesc_DPL(descriptor);
            rsreg->limit = (t_nubit32)((_IsDescSegGranularLarge(descriptor) ?
                                        (_GetDescSeg_Limit(descriptor) << 12 | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
            rsreg->sys.type = (t_nubit4)_GetDesc_Type(descriptor);
            _be;
        }
        _be;
        break;
    case SREG_GDTR:
    case SREG_IDTR:
    default:
        _impossible_r_;
        break;
    }
    _ce;
}

/* regular segment accessing */
static t_bool _s_check_selector(t_nubit16 selector) {
    /* 0 = succ, 1 = fail */
    _cb("_s_check_selector");
    if (_IsSelectorNull(selector)) {
        _ce;
        return True;
    }
    if (GetMax32(_GetSelector_Offset(selector) + 7) >
            (_GetSelector_TI(selector) ? GetMax32(vcpu.data.ldtr.limit) :
             GetMax32(vcpu.data.gdtr.limit))) {
        _ce;
        return True;
    }
    _ce;
    return False;
}
static void _s_read_idt(t_nubit8 intid, t_vaddrcc rdata) {
    _cb("_s_read_idt");
    _chr(_ksa_read_idt(intid, rdata));
    _ce;
}
static void _s_read_xdt(t_nubit16 selector, t_vaddrcc rdata) {
    _cb("_s_read_xdt");
    if (!_GetCR0_PE) _impossible_r_;
    _chr(_ksa_read_xdt(selector, rdata));
    _ce;
}
static void _s_read_tss(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_read_tss");
    _chr(_kma_read_logical(&vcpu.data.tr, offset, rdata, byte, 0, 1));
    _ce;
}
static void _s_read_es(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_read_es");
    _chr(_m_read_logical(&vcpu.data.es, offset, rdata, byte));
    _ce;
}
static void _s_read_cs(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_read_cs");
    _chr(_kma_read_logical(&vcpu.data.cs, offset, rdata, byte, 0, 1));
    _ce;
}
static void _s_read_ss(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_read_ss");
    _chr(_m_read_logical(&vcpu.data.ss, offset, rdata, byte));
    _ce;
}
static void _s_read_ds(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_read_ds");
    _impossible_r_;
    _ce;
}
static void _s_read_fs(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_read_fs");
    _impossible_r_;
    _ce;
}
static void _s_read_gs(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_read_gs");
    _impossible_r_;
    _ce;
}
static void _s_write_idt(t_nubit8 intid, t_vaddrcc rdata) {
    _cb("_s_write_idt");
    _impossible_r_;
    _ce;
}
static void _s_write_xdt(t_nubit16 selector, t_vaddrcc rdata) {
    _cb("_s_write_xdt");
    if (!_GetCR0_PE) _impossible_r_;
    _chr(_ksa_write_xdt(selector, rdata));
    _ce;
}
static void _s_write_tss(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_write_tss");
    _impossible_r_;
    _ce;
}
static void _s_write_es(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_write_es");
    _chr(_m_write_logical(&vcpu.data.es, offset, rdata, byte));
    _ce;
}
static void _s_write_cs(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_write_cs");
    _impossible_r_;
    _ce;
}
static void _s_write_ss(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_write_ss");
    _chr(_m_write_logical(&vcpu.data.ss, offset, rdata, byte));
    _ce;
}
static void _s_write_ds(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_write_ds");
    _impossible_r_;
    _ce;
}
static void _s_write_fs(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_write_fs");
    _impossible_r_;
    _ce;
}
static void _s_write_gs(t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_write_gs");
    _impossible_r_;
    _ce;
}
static void _s_test_tss(t_nubit32 offset, t_nubit8 byte) {
    _cb("_s_test_cs");
    _chr(_kma_test_logical(&vcpu.data.tr, offset, byte, 0, 0x00, 1));
    _ce;
}
static void _s_test_cs(t_nubit32 offset, t_nubit8 byte) {
    _cb("_s_test_cs");
    _chr(_kma_test_logical(&vcpu.data.cs, offset, byte, 0, 0x00, 1));
    _ce;
}
static void _s_test_ss_push(t_nubit8 byte) {
    t_nubit32 cesp = 0x00000000;
    _cb("_s_test_ss_push");
    switch (_GetStackSize) {
    case 2:
        _bb("StackSize(2)");
        if (vcpu.data.sp && vcpu.data.sp < byte)
            _chr(_SetExcept_SS(0));
        _chr(_m_test_access(&vcpu.data.ss, GetMax16(vcpu.data.sp - byte), byte, 1));
        _be;
        break;
    case 4:
        _bb("StackSize(4)");
        if (vcpu.data.esp && vcpu.data.esp < byte)
            _chr(_SetExcept_SS(0));
        _chr(_m_test_access(&vcpu.data.ss, GetMax32(vcpu.data.esp - byte), byte, 1));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void _s_test_ss_pop(t_nubit8 byte) {
    t_nubit32 cesp = 0x00000000;
    _cb("_s_test_ss_pop");
    switch (_GetStackSize) {
    case 2:
        _bb("StackSize(2)");
        _chr(_m_test_access(&vcpu.data.ss, vcpu.data.sp, byte, 0));
        _be;
        break;
    case 4:
        _bb("StackSize(4)");
        _chr(_m_test_access(&vcpu.data.ss, vcpu.data.esp, byte, 0));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void _s_load_sreg(t_cpu_data_sreg *rsreg, t_nubit16 selector) {
    _cb("_s_load_sreg");
    _chr(_ksa_load_sreg(rsreg, selector));
    _ce;
}
static void _s_load_gdtr(t_nubit32 base, t_nubit16 limit, t_nubit8 byte) {
    _cb("_s_load_gdtr");
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    vcpu.data.gdtr.limit = limit;
    switch (byte) {
    case 2:
        vcpu.data.gdtr.base = GetMax24(base);
        break;
    case 4:
        vcpu.data.gdtr.base = GetMax32(base);
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    _ce;
}
static void _s_load_idtr(t_nubit32 base, t_nubit16 limit, t_nubit8 byte) {
    _cb("_s_load_idtr");
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    vcpu.data.idtr.limit = limit;
    switch (byte) {
    case 2:
        vcpu.data.idtr.base = GetMax24(base);
        break;
    case 4:
        vcpu.data.idtr.base = GetMax32(base);
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    _ce;
}
static void _s_load_ldtr(t_nubit16 selector) {
    _cb("_s_load_ldtr");
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    if (_GetSelector_TI(selector)) {
        _bb("Selector_TI(1)");
        _chr(_SetExcept_GP(selector));
        _be;
    }
    _chr(_s_load_sreg(&vcpu.data.ldtr, selector));
    _ce;
}
static void _s_load_tr(t_nubit16 selector) {
    _cb("_s_load_tr");
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    if (_GetSelector_TI(selector)) {
        _bb("Selector_TI(1)");
        _chr(_SetExcept_GP(selector));
        _be;
    }
    _chr(_s_load_sreg(&vcpu.data.tr, selector));
    _ce;
}
static void _s_load_cr0_msw(t_nubit16 msw) {
    _cb("_s_load_cr0_msw");
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    if (!_GetCR0_PE) {
        _bb("CR0_PE(0)");
        vcpu.data.cr0 = (vcpu.data.cr0 & 0xfffffff0) | (msw & 0x000f);
        _be;
    } else {
        _bb("CR0_PE(1)");
        vcpu.data.cr0 = (vcpu.data.cr0 & 0xfffffff0) | (msw & 0x000e) | 0x01;
        _be;
    }
    _ce;
}
static void _s_load_cs(t_nubit16 newcs) {
    _cb("_s_load_cs");
    _chr(_s_load_sreg(&vcpu.data.cs, newcs));
    _ce;
}
static void _s_load_ss(t_nubit16 newss) {
    _cb("_s_load_ss");
    _chr(_s_load_sreg(&vcpu.data.ss, newss));
    _ce;
}
static void _s_load_ds(t_nubit16 newds) {
    _cb("_s_load_ds");
    _chr(_s_load_sreg(&vcpu.data.ds, newds));
    _ce;
}
static void _s_load_es(t_nubit16 newes) {
    _cb("_s_load_es");
    _chr(_s_load_sreg(&vcpu.data.es, newes));
    _ce;
}
static void _s_load_fs(t_nubit16 newfs) {
    _cb("_s_load_fs");
    _chr(_s_load_sreg(&vcpu.data.fs, newfs));
    _ce;
}
static void _s_load_gs(t_nubit16 newgs) {
    _cb("_s_load_gs");
    _chr(_s_load_sreg(&vcpu.data.gs, newgs));
    _ce;
}
static void _s_test_eip() {
    _cb("_s_test_eip");
    _chr(_s_test_cs(vcpu.data.eip, 0x01));
    _ce;
}
static void _s_test_esp() {
    t_nubit32 cesp;
    _cb("_s_test_esp");
    switch (_GetStackSize) {
    case 2:
        cesp = GetMax16(vcpu.data.esp);
        break;
    case 4:
        cesp = GetMax32(vcpu.data.esp);
        break;
    default:
        _impossible_r_;
        break;
    }
    _chr(_m_test_logical(&vcpu.data.ss, cesp, 0x00, 0));
    _ce;
}

/* portid accessing unit */
/* kernel portid accessing */
_______todo _kpa_test_iomap(t_nubit16 portid, t_nubit8 byte) {
    /* TODO: iomap tester not implemented */
    _cb("_kpa_test_iomap");
    _ce;
}
static void _kpa_test_mode(t_nubit16 portid, t_nubit8 byte) {
    _cb("_p_test");
    if (_GetCR0_PE && (_GetCPL > (t_nubit8)_GetEFLAGS_IOPL || _GetEFLAGS_VM)) {
        _bb("CR0_PE(1),(CPL>IOPL/EFLAGS_VM(1))");
        _chr(_kpa_test_iomap(portid, byte));
        _be;
    }
    _ce;
}
/* regular portid accessing */
static void _p_input(t_nubit16 portid, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_p_input");
    _chr(_kpa_test_mode(portid, byte));
    vportExecRead(portid);
    switch (byte) {
    case 1:
        _bb("byte(1)");
        _chr(_m_write_ref(rdata, GetRef(vport.data.ioByte), 1));
        _be;
        break;
    case 2:
        _bb("byte(2)");
        _chr(_m_write_ref(rdata, GetRef(vport.data.ioWord), 2));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_m_write_ref(rdata, GetRef(vport.data.ioDWord), 4));
        _be;
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    vcpuins.data.flagIgnore = True;
    _ce;
}
static void _p_output(t_nubit16 portid, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_p_output");
    _chr(_kpa_test_mode(portid, byte));
    switch (byte) {
    case 1:
        _bb("byte(1)");
        _chr(_m_read_ref(rdata, GetRef(vport.data.ioByte), 1));
        _be;
        break;
    case 2:
        _bb("byte(2)");
        _chr(_m_read_ref(rdata, GetRef(vport.data.ioWord), 2));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_m_read_ref(rdata, GetRef(vport.data.ioDWord), 4));
        _be;
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    vportExecWrite(portid);
    vcpuins.data.flagIgnore = True;
    _ce;
}

/* decoding unit */
/* kernel decoding function */
static t_bool _kdf_check_prefix(t_nubit8 opcode) {
    switch (opcode) {
    case 0xf0:
    case 0xf2:
    case 0xf3:
    case 0x2e:
    case 0x36:
    case 0x3e:
    case 0x26:
        return True;
        break;
    case 0x64:
    case 0x65:
    case 0x66:
    case 0x67:
        i386(opcode) return True;
        else return False;
        break;
    default:
        return False;
        break;
    }
    return False;
}

static void _kdf_skip(t_nubit8 byte) {
    _cb("_kdf_skip");
    _chr(vcpu.data.eip += byte);
    _ce;
}
static void _kdf_code(t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_kdf_code");
    _chr(_s_read_cs(vcpu.data.eip, rdata, byte));
    _chr(_kdf_skip(byte));
    _ce;
}
static void _kdf_modrm(t_nubit8 regbyte, t_nubit8 rmbyte) {
    t_nsbit8 disp8;
    t_nubit16 disp16;
    t_nubit32 disp32;
    t_nubit32 sibindex;
    t_nubit8 modrm, sib;
    _cb("_kdf_modrm");
    _chr(_kdf_code(GetRef(modrm), 1));
    vcpuins.data.flagMem = True;
    vcpuins.data.mrm.rsreg = NULL;
    vcpuins.data.mrm.offset = 0;
    vcpuins.data.cr = vcpuins.data.crm = 0;
    vcpuins.data.rrm = vcpuins.data.rr = (t_vaddrcc)NULL;
    switch (_GetAddressSize) {
    case 2:
        _bb("AddressSize(2)");
        switch (_GetModRM_MOD(modrm)) {
        case 0:
            _bb("ModRM_MOD(0)");
            switch (_GetModRM_RM(modrm)) {
            case 0:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.bx + vcpu.data.si);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 1:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.bx + vcpu.data.di);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 2:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.bp + vcpu.data.si);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverss;
                break;
            case 3:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.bp + vcpu.data.di);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverss;
                break;
            case 4:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.si);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 5:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.di);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 6:
                _bb("ModRM_RM(6)");
                _chr(_kdf_code(GetRef(disp16), 2));
                vcpuins.data.mrm.offset = GetMax16(disp16);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                _be;
                break;
            case 7:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.bx);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
            break;
        case 1:
            _bb("ModRM_MOD(1)");
            _chr(_kdf_code(GetRef(disp8), 1));
            switch (_GetModRM_RM(modrm)) {
            case 0:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.bx + vcpu.data.si + disp8);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 1:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.bx + vcpu.data.di + disp8);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 2:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.bp + vcpu.data.si + disp8);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverss;
                break;
            case 3:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.bp + vcpu.data.di + disp8);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverss;
                break;
            case 4:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.si + disp8);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 5:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.di + disp8);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 6:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.bp + disp8);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverss;
                break;
            case 7:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.bx + disp8);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
            break;
        case 2:
            _bb("ModRM_MOD(2)");
            _chr(_kdf_code(GetRef(disp16), 2));
            switch (_GetModRM_RM(modrm)) {
            case 0:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.bx + vcpu.data.si + disp16);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 1:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.bx + vcpu.data.di + disp16);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 2:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.bp + vcpu.data.si + disp16);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverss;
                break;
            case 3:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.bp + vcpu.data.di + disp16);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverss;
                break;
            case 4:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.si + disp16);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 5:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.di + disp16);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 6:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.bp + disp16);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverss;
                break;
            case 7:
                vcpuins.data.mrm.offset = GetMax16(vcpu.data.bx + disp16);
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
            break;
        case 3:
            break;
        default:
            _impossible_r_;
            break;
        }
        _be;
        break;
    case 4:
        _bb("AddressSize(4)");
        if (_GetModRM_MOD(modrm) != 3 && _GetModRM_RM(modrm) == 4) {
            _bb("ModRM_MOD(!3),ModRM_RM(4)");
            _chr(_kdf_code(GetRef(sib), 1));
            switch (_GetSIB_Index(sib)) {
            case 0:
                sibindex = vcpu.data.eax;
                break;
            case 1:
                sibindex = vcpu.data.ecx;
                break;
            case 2:
                sibindex = vcpu.data.edx;
                break;
            case 3:
                sibindex = vcpu.data.ebx;
                break;
            case 4:
                sibindex = 0x00000000;
                break;
            case 5:
                sibindex = vcpu.data.ebp;
                break;
            case 6:
                sibindex = vcpu.data.esi;
                break;
            case 7:
                sibindex = vcpu.data.edi;
                break;
            default:
                _impossible_r_;
                break;
            }
            sibindex <<= _GetSIB_SS(sib);
            _be;
        }
        switch (_GetModRM_MOD(modrm)) {
        case 0:
            _bb("ModRM_MOD(0)");
            switch (_GetModRM_RM(modrm)) {
            case 0:
                vcpuins.data.mrm.offset = vcpu.data.eax;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 1:
                vcpuins.data.mrm.offset = vcpu.data.ecx;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 2:
                vcpuins.data.mrm.offset = vcpu.data.edx;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 3:
                vcpuins.data.mrm.offset = vcpu.data.ebx;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 4:
                _bb("ModRM_RM(4)");
                switch (_GetSIB_Base(sib)) {
                case 0:
                    vcpuins.data.mrm.offset = vcpu.data.eax + sibindex;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    break;
                case 1:
                    vcpuins.data.mrm.offset = vcpu.data.ecx + sibindex;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    break;
                case 2:
                    vcpuins.data.mrm.offset = vcpu.data.edx + sibindex;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    break;
                case 3:
                    vcpuins.data.mrm.offset = vcpu.data.ebx + sibindex;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    break;
                case 4:
                    vcpuins.data.mrm.offset = vcpu.data.esp + sibindex;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverss;
                    break;
                case 5:
                    _bb("SIB_Base(5)");
                    _chr(_kdf_code(GetRef(disp32), 4));
                    vcpuins.data.mrm.offset = disp32 + sibindex;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    _be;
                    break;
                case 6:
                    vcpuins.data.mrm.offset = vcpu.data.esi + sibindex;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    break;
                case 7:
                    vcpuins.data.mrm.offset = vcpu.data.edi + sibindex;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    break;
                default:
                    _impossible_r_;
                    break;
                }
                _be;
                break;
            case 5:
                _bb("ModRM_RM(5)");
                _chr(_kdf_code(GetRef(disp32), 4));
                vcpuins.data.mrm.offset = disp32;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                _be;
                break;
            case 6:
                vcpuins.data.mrm.offset = vcpu.data.esi;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 7:
                vcpuins.data.mrm.offset = vcpu.data.edi;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
            break;
        case 1:
            _bb("ModRM_MOD(1)");
            _chr(_kdf_code(GetRef(disp8), 1));
            switch (_GetModRM_RM(modrm)) {
            case 0:
                vcpuins.data.mrm.offset = vcpu.data.eax + disp8;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 1:
                vcpuins.data.mrm.offset = vcpu.data.ecx + disp8;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 2:
                vcpuins.data.mrm.offset = vcpu.data.edx + disp8;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 3:
                vcpuins.data.mrm.offset = vcpu.data.ebx + disp8;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 4:
                _bb("ModRM_RM(4)");
                switch (_GetSIB_Base(sib)) {
                case 0:
                    vcpuins.data.mrm.offset = vcpu.data.eax + sibindex + disp8;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    break;
                case 1:
                    vcpuins.data.mrm.offset = vcpu.data.ecx + sibindex + disp8;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    break;
                case 2:
                    vcpuins.data.mrm.offset = vcpu.data.edx + sibindex + disp8;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    break;
                case 3:
                    vcpuins.data.mrm.offset = vcpu.data.ebx + sibindex + disp8;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    break;
                case 4:
                    vcpuins.data.mrm.offset = vcpu.data.esp + sibindex + disp8;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverss;
                    break;
                case 5:
                    vcpuins.data.mrm.offset = vcpu.data.ebp + sibindex + disp8;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverss;
                    break;
                case 6:
                    vcpuins.data.mrm.offset = vcpu.data.esi + sibindex + disp8;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    break;
                case 7:
                    vcpuins.data.mrm.offset = vcpu.data.edi + sibindex + disp8;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    break;
                default:
                    _impossible_r_;
                    break;
                }
                _be;
                break;
            case 5:
                vcpuins.data.mrm.offset = vcpu.data.ebp + disp8;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverss;
                break;
            case 6:
                vcpuins.data.mrm.offset = vcpu.data.esi + disp8;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 7:
                vcpuins.data.mrm.offset = vcpu.data.edi + disp8;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
            break;
        case 2:
            _bb("ModRM_MOD(2)");
            _chr(_kdf_code(GetRef(disp32), 4));
            switch (_GetModRM_RM(modrm)) {
            case 0:
                vcpuins.data.mrm.offset = vcpu.data.eax + disp32;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 1:
                vcpuins.data.mrm.offset = vcpu.data.ecx + disp32;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 2:
                vcpuins.data.mrm.offset = vcpu.data.edx + disp32;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 3:
                vcpuins.data.mrm.offset = vcpu.data.ebx + disp32;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 4:
                _bb("ModRM_RM(4)");
                switch (_GetSIB_Base(sib)) {
                case 0:
                    vcpuins.data.mrm.offset = vcpu.data.eax + sibindex + disp32;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    break;
                case 1:
                    vcpuins.data.mrm.offset = vcpu.data.ecx + sibindex + disp32;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    break;
                case 2:
                    vcpuins.data.mrm.offset = vcpu.data.edx + sibindex + disp32;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    break;
                case 3:
                    vcpuins.data.mrm.offset = vcpu.data.ebx + sibindex + disp32;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    break;
                case 4:
                    vcpuins.data.mrm.offset = vcpu.data.esp + sibindex + disp32;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverss;
                    break;
                case 5:
                    vcpuins.data.mrm.offset = vcpu.data.ebp + sibindex + disp32;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverss;
                    break;
                case 6:
                    vcpuins.data.mrm.offset = vcpu.data.esi + sibindex + disp32;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    break;
                case 7:
                    vcpuins.data.mrm.offset = vcpu.data.edi + sibindex + disp32;
                    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                    break;
                default:
                    _impossible_r_;
                    break;
                }
                _be;
                break;
            case 5:
                vcpuins.data.mrm.offset = vcpu.data.ebp + disp32;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverss;
                break;
            case 6:
                vcpuins.data.mrm.offset = vcpu.data.esi + disp32;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            case 7:
                vcpuins.data.mrm.offset = vcpu.data.edi + disp32;
                vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
            break;
        case 3:
            break;
        default:
            _impossible_r_;
            break;
        }
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    if (_GetModRM_MOD(modrm) == 3) {
        _bb("ModRM_MOD(3)");
        vcpuins.data.flagMem = False;
        switch (rmbyte) {
        case 1:
            switch (_GetModRM_RM(modrm)) {
            case 0:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.al);
                break;
            case 1:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.cl);
                break;
            case 2:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.dl);
                break;
            case 3:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.bl);
                break;
            case 4:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.ah);
                break;
            case 5:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.ch);
                break;
            case 6:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.dh);
                break;
            case 7:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.bh);
                break;
            default:
                _impossible_r_;
                break;
            }
            break;
        case 2:
            switch (_GetModRM_RM(modrm)) {
            case 0:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.ax);
                break;
            case 1:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.cx);
                break;
            case 2:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.dx);
                break;
            case 3:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.bx);
                break;
            case 4:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.sp);
                break;
            case 5:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.bp);
                break;
            case 6:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.si);
                break;
            case 7:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.di);
                break;
            default:
                _impossible_r_;
                break;
            }
            break;
        case 4:
            switch (_GetModRM_RM(modrm)) {
            case 0:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.eax);
                break;
            case 1:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.ecx);
                break;
            case 2:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.edx);
                break;
            case 3:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.ebx);
                break;
            case 4:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.esp);
                break;
            case 5:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.ebp);
                break;
            case 6:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.esi);
                break;
            case 7:
                vcpuins.data.rrm = (t_vaddrcc)(&vcpu.data.edi);
                break;
            default:
                _impossible_r_;
                break;
            }
            break;
        default:
            _bb("rmbyte");
            _chr(_SetExcept_CE(rmbyte));
            _be;
            break;
        }
        _chr(_m_read_ref(vcpuins.data.rrm, GetRef(vcpuins.data.crm), rmbyte));
        _be;
    }
    if (!regbyte) {
        /* reg is operation or segment */
        vcpuins.data.cr = _GetModRM_REG(modrm);
    } else {
        switch (regbyte) {
        case 1:
            switch (_GetModRM_REG(modrm)) {
            case 0:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.al);
                break;
            case 1:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.cl);
                break;
            case 2:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.dl);
                break;
            case 3:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.bl);
                break;
            case 4:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.ah);
                break;
            case 5:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.ch);
                break;
            case 6:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.dh);
                break;
            case 7:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.bh);
                break;
            default:
                _impossible_r_;
                break;
            }
            break;
        case 2:
            switch (_GetModRM_REG(modrm)) {
            case 0:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.ax);
                break;
            case 1:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.cx);
                break;
            case 2:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.dx);
                break;
            case 3:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.bx);
                break;
            case 4:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.sp);
                break;
            case 5:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.bp);
                break;
            case 6:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.si);
                break;
            case 7:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.di);
                break;
            default:
                _impossible_r_;
                break;
            }
            break;
        case 4:
            switch (_GetModRM_REG(modrm)) {
            case 0:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.eax);
                break;
            case 1:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.ecx);
                break;
            case 2:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.edx);
                break;
            case 3:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.ebx);
                break;
            case 4:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.esp);
                break;
            case 5:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.ebp);
                break;
            case 6:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.esi);
                break;
            case 7:
                vcpuins.data.rr = (t_vaddrcc)(&vcpu.data.edi);
                break;
            default:
                _impossible_r_;
                break;
            }
            break;
        default:
            _bb("regbyte");
            _chr(_SetExcept_CE(regbyte));
            _be;
            break;
        }
        _chr(_m_read_ref(vcpuins.data.rr, GetRef(vcpuins.data.cr), regbyte));
    }
    _ce;
}
static void _d_skip(t_nubit8 byte) {
    _cb("_d_skip");
    _chr(_kdf_skip(byte));
    _ce;
}
static void _d_code(t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_d_code");
    _chr(_kdf_code(rdata, byte));
    _ce;
}
static void _d_imm(t_nubit8 byte) {
    _cb("_d_imm");
    vcpuins.data.cimm = 0;
    _chr(_d_code(GetRef(vcpuins.data.cimm), byte));
    _ce;
}
static void _d_moffs(t_nubit8 byte) {
    _cb("_d_moffs");
    vcpuins.data.flagMem = True;
    vcpuins.data.mrm.rsreg = vcpuins.data.roverds;
    vcpuins.data.mrm.offset = 0;
    switch (_GetAddressSize) {
    case 2:
        _bb("AddressSize(2)");
        _chr(_d_code(GetRef(vcpuins.data.mrm.offset), 2));
        _be;
        break;
    case 4:
        _bb("AddressSize(4)");
        _chr(_d_code(GetRef(vcpuins.data.mrm.offset), 4));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void _d_modrm_sreg(t_nubit8 rmbyte) {
    _cb("_d_modrm_sreg");
    _chr(_kdf_modrm(0, rmbyte));
    vcpuins.data.rmovsreg = NULL;
    switch (vcpuins.data.cr) {
    case 0:
        vcpuins.data.rmovsreg = &vcpu.data.es;
        break;
    case 1:
        vcpuins.data.rmovsreg = &vcpu.data.cs;
        break;
    case 2:
        vcpuins.data.rmovsreg = &vcpu.data.ss;
        break;
    case 3:
        vcpuins.data.rmovsreg = &vcpu.data.ds;
        break;
    case 4:
        vcpuins.data.rmovsreg = &vcpu.data.fs;
        break;
    case 5:
        vcpuins.data.rmovsreg = &vcpu.data.gs;
        break;
    default:
        _bb("vcpuins.data.cr");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    }
    _ce;
}
static void _d_modrm_ea(t_nubit8 regbyte, t_nubit8 rmbyte) {
    _cb("_d_modrm_ea");
    _chr(_kdf_modrm(regbyte, rmbyte));
    if (!vcpuins.data.flagMem) {
        _bb("flagMem(0)");
        _chr(_SetExcept_UD(0));
        _be;
    }
    _ce;
}
static void _d_modrm(t_nubit8 regbyte, t_nubit8 rmbyte) {
    _cb("_d_modrm");
    _chr(_kdf_modrm(regbyte, rmbyte));
    if (!vcpuins.data.flagMem && vcpuins.data.flagLock) {
        _bb("flagMem(0),flagLock(1)");
        _chr(_SetExcept_UD(0));
        _be;
    }
    _ce;
}

/* execution control unit: _e_ */
/* kernel execution control */
static void _kec_push(t_vaddrcc rdata, t_nubit8 byte) {
    t_nubit32 cesp;
    _cb("_kec_push");
    _chr(_s_test_ss_push(byte));
    switch (_GetStackSize) {
    case 2:
        cesp = GetMax16(vcpu.data.sp - byte);
        break;
    case 4:
        cesp = GetMax32(vcpu.data.esp - byte);
        break;
    default:
        _impossible_r_;
        break;
    }
    _chr(_s_write_ss(cesp, rdata, byte));
    switch (_GetStackSize) {
    case 2:
        vcpu.data.sp -= byte;
        break;
    case 4:
        vcpu.data.esp -= byte;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void _kec_pop(t_vaddrcc rdata, t_nubit8 byte) {
    t_nubit32 cesp;
    _cb("_kec_pop");
    switch (_GetStackSize) {
    case 2:
        cesp = vcpu.data.sp;
        break;
    case 4:
        cesp = vcpu.data.esp;
        break;
    default:
        _impossible_r_;
        break;
    }
    _chr(_s_read_ss(cesp, rdata, byte));
    if (rdata != GetRef(vcpu.data.esp)) {
        switch (_GetStackSize) {
        case 2:
            vcpu.data.sp += byte;
            break;
        case 4:
            vcpu.data.esp += byte;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    _ce;
}
static void _kec_call_far(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    t_nubit32 oldcs = vcpu.data.cs.selector;
    t_cpu_data_sreg ccs = vcpu.data.cs;
    _cb("_kec_call_far");
    switch (byte) {
    case 2:
        _bb("byte(2)");
        _chr(_s_test_ss_push(4));
        neweip = GetMax16(neweip);
        _chr(_ksa_load_sreg(&ccs, newcs));
        _chr(_kma_test_logical(&ccs, neweip, 0x01, 0, 0x00, 1));
        _chr(_kec_push(GetRef(oldcs), 2));
        _chr(_kec_push(GetRef(vcpu.data.ip), 2));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_s_test_ss_push(8));
        neweip = GetMax32(neweip);
        _chr(_ksa_load_sreg(&ccs, newcs));
        _chr(_kma_test_logical(&ccs, neweip, 0x01, 0, 0x00, 1));
        _chr(_kec_push(GetRef(oldcs), 4));
        _chr(_kec_push(GetRef(vcpu.data.eip), 4));
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    vcpu.data.cs = ccs;
    vcpu.data.eip = neweip;
    _ce;
}
static void _kec_call_near(t_nubit32 neweip, t_nubit8 byte) {
    _cb("_kec_call_near");
    switch (byte) {
    case 2:
        _bb("byte(2)");
        neweip = GetMax16(neweip);
        _chr(_s_test_cs(neweip, 0x01));
        _chr(_kec_push(GetRef(vcpu.data.ip), 2));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        neweip = GetMax32(neweip);
        _chr(_s_test_cs(neweip, 0x01));
        _chr(_kec_push(GetRef(vcpu.data.eip), 4));
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    vcpu.data.eip = neweip;
    _ce;
}
_______todo _kec_task_switch(t_nubit16 newtss);
static void _kec_jmp_far(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    t_cpu_data_sreg ccs = vcpu.data.cs;
    _cb("_kec_jmp_far");
    switch (byte) {
    case 2:
        neweip = GetMax16(neweip);
        break;
    case 4:
        neweip = GetMax32(neweip);
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    _chr(_ksa_load_sreg(&ccs, newcs));
    _chr(_kma_test_logical(&ccs, neweip, 0x01, 0, 0x00, 1));
    vcpu.data.cs = ccs;
    vcpu.data.eip = neweip;
    _ce;
}
static void _kec_jmp_near(t_nubit32 neweip, t_nubit8 byte) {
    _cb("_kec_jmp_near");
    switch (byte) {
    case 2:
        neweip = GetMax16(neweip);
        break;
    case 4:
        neweip = GetMax32(neweip);
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    _chr(_s_test_cs(neweip, 0x01));
    vcpu.data.eip = neweip;
    _ce;
}
static void _kec_ret_far(t_nubit32 newcs, t_nubit32 neweip, t_nubit16 parambyte, t_nubit16 byte) {
    t_cpu_data_sreg ccs = vcpu.data.cs;
    _cb("_kec_ret_far");
    switch (byte) {
    case 2:
        neweip = GetMax16(neweip);
        break;
    case 4:
        neweip = GetMax32(neweip);
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    _chr(_ksa_load_sreg(&ccs, newcs));
    _chr(_kma_test_logical(&ccs, neweip, 0x01, 0, 0x00, 1));
    vcpu.data.cs = ccs;
    vcpu.data.eip = neweip;
    switch (_GetStackSize) {
    case 2:
        vcpu.data.sp += parambyte;
        break;
    case 4:
        vcpu.data.esp += parambyte;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void _kec_ret_near(t_nubit16 parambyte, t_nubit8 byte) {
    t_nubit32 neweip = 0;
    _cb("_kec_ret_near");
    switch (byte) {
    case 2:
        _bb("byte(2)");
        _chr(_kec_pop(GetRef(neweip), 2));
        _chr(_s_test_cs(neweip, 0x01));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_kec_pop(GetRef(neweip), 4));
        _chr(_s_test_cs(neweip, 0x01));
        _be;
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    vcpu.data.eip = neweip;
    switch (_GetStackSize) {
    case 2:
        vcpu.data.sp += parambyte;
        break;
    case 4:
        vcpu.data.esp += parambyte;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
/* sub execution routine */
static void _ser_call_far_real(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    _cb("_ser_call_far_real");
    if (_IsProtected) _impossible_r_;
    _chr(_kec_call_far(newcs, neweip, byte));
    _ce;
}
_______todo _ser_call_far_cs_conf(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    t_nubit64 descriptor;
    _cb("_ser_call_far_cs_conf");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(newcs, GetRef(descriptor)));
    if (!_IsDescCodeConform(descriptor)) _impossible_r_;
    if (_GetDesc_DPL(descriptor) > _GetCPL) {
        _bb("DPL(>CPL)");
        _chr(_SetExcept_GP(newcs));
        _be;
    }
    if (!_IsDescPresent(descriptor)) {
        _bb("!DescPresent");
        _chr(_SetExcept_NP(newcs));
        _be;
    }
    _chr(_kec_call_far(newcs, neweip, byte));
    _ce;
}
_______todo _ser_call_far_cs_nonc(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    t_nubit64 descriptor;
    _cb("_ser_call_far_cs_nonc");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(newcs, GetRef(descriptor)));
    if (!_IsDescCodeNonConform(descriptor)) _impossible_r_;
    if (_GetDesc_DPL(descriptor) != _GetCPL ||
            _GetSelector_RPL(newcs) > _GetCPL) {
        _bb("DPL(!CPL)/RPL(>CPL)");
        _chr(_SetExcept_GP(newcs));
        _be;
    }
    if (!_IsDescPresent(descriptor)) {
        _bb("!DescPresent");
        _chr(_SetExcept_NP(newcs));
        _be;
    }
    _chr(_kec_call_far(newcs, neweip, byte));
    _ce;
}
_______todo _ser_call_far_call_gate(t_nubit16 newcs) {
    t_nubit64 descriptor;
    _cb("_ser_call_far_call_gate");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(newcs, GetRef(descriptor)));
    if (!_IsDescCallGate(descriptor)) _impossible_r_;
    _chr(_SetExcept_CE(0));
    _ce;
}
_______todo _ser_call_far_task_gate(t_nubit16 newcs) {
    t_nubit64 descriptor;
    _cb("_ser_call_far_task_gate");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(newcs, GetRef(descriptor)));
    if (!_IsDescTaskGate(descriptor)) _impossible_r_;
    _chr(_SetExcept_CE(0));
    _ce;
}
_______todo _ser_call_far_tss(t_nubit16 newcs) {
    t_nubit64 descriptor;
    _cb("_ser_call_far_tss");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(newcs, GetRef(descriptor)));
    if (!_IsDescTSS(descriptor)) _impossible_r_;
    _chr(_SetExcept_CE(0));
    _ce;
}
static void _ser_int_real(t_nubit8 intid, t_nubit8 byte) {
    t_nubit16 cip;
    t_nubit32 vector;
    t_nubit32 oldcs = vcpu.data.cs.selector;
    _cb("_ser_int_real");
    if (_IsProtected) _impossible_r_;
    if (GetMax16(intid * 4 + 3) > GetMax16(vcpu.data.idtr.limit)) {
        _bb("intid(>idtr.limit)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    switch (byte) {
    case 2:
        _bb("byte(2)");
        _chr(_s_test_ss_push(6));
        _chr(_kec_push(GetRef(vcpu.data.flags), 2));
        _ClrEFLAGS_IF;
        _ClrEFLAGS_TF;
        _chr(_kec_push(GetRef(oldcs), 2));
        _chr(_kec_push(GetRef(vcpu.data.ip), 2));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_s_test_ss_push(12));
        _chr(_kec_push(GetRef(vcpu.data.eflags), 4));
        _ClrEFLAGS_IF;
        _ClrEFLAGS_TF;
        _chr(_kec_push(GetRef(oldcs), 4));
        _chr(_kec_push(GetRef(vcpu.data.eip), 4));
        _be;
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    _chr(_s_read_idt(intid, GetRef(vector)));
    cip = GetMax16(vector);
    _chr(_s_test_cs(cip, 0x01));
    vcpu.data.eip = cip;
    _chr(_s_load_cs(GetMax16(vector >> 16)));
    _ce;
}
_______todo _ser_int_protected(t_nubit8 intid, t_nubit8 byte, t_bool flagext) {
    t_nubit16 oldss;
    t_nubit32 oldeflags, oldesp, newesp = Zero32;
    t_nubit32 xs_sel;
    t_nubit16 newcs, newss = Zero16;
    t_nubit64 cs_desc, ss_desc, gate_desc;
    _cb("_ser_int_protected");
    if (!_GetCR0_PE) _impossible_r_;
    if (GetMax16(intid * 8 + 7) > GetMax16(vcpu.data.idtr.limit)) {
        _bb("intid(>idtr.limit)");
        _chr(_SetExcept_GP(intid * 8 + 2 + !!flagext));
        _be;
    }
    _chr(_s_read_idt(intid, GetRef(gate_desc)));
    switch (_GetDesc_Type(gate_desc)) {
    case VCPU_DESC_SYS_TYPE_TASKGATE:
    case VCPU_DESC_SYS_TYPE_INTGATE_16:
    case VCPU_DESC_SYS_TYPE_INTGATE_32:
    case VCPU_DESC_SYS_TYPE_TRAPGATE_16:
    case VCPU_DESC_SYS_TYPE_TRAPGATE_32:
        break;
    default:
        _bb("Desc_Type(!TaskGate/!IntGate/!TrapGate)");
        _chr(_SetExcept_GP(intid * 8 + 2 + !!flagext));
        _be;
        break;
    }
    if (!flagext) {
        _bb("!flagext");
        if (_GetDesc_DPL(gate_desc) < _GetCPL) {
            _bb("DPL(<CPL)");
            _chr(_SetExcept_GP(intid * 8 + 2));
            _be;
        }
        _be;
    }
    if (!_IsDescPresent(gate_desc)) {
        _bb("!DescPresent");
        _chr(_SetExcept_NP(intid * 8 + 2 + !!flagext));
        _be;
    }
    if (_GetDesc_Type(gate_desc) == VCPU_DESC_SYS_TYPE_TASKGATE) {
        _bb("Desc_Type(TaskGate)");
        _chr(_SetExcept_CE(0));
        _be;
    } else {
        _bb("Desc_Type(!TaskGate)");
        newcs = GetMax16(_GetDescGate_Selector(gate_desc));
        if (_IsSelectorNull(newcs)) {
            _bb("newcs(null)");
            _chr(_SetExcept_GP(!!flagext));
            _be;
        }
        if (_GetSelector_TI(newcs)) {
            _bb("Selector_TI(1)");
            if (GetMax16(_GetSelector_Offset(newcs) + 7) > GetMax16(vcpu.data.ldtr.limit)) {
                _bb("Selector_Offset(>ldtr.limit)");
                _chr(_SetExcept_GP(newcs & 0xfffc + !!flagext));
                _be;
            }
            _be;
        } else {
            _bb("Selector_TI(0)");
            if (GetMax16(_GetSelector_Offset(newcs) + 7) > GetMax16(vcpu.data.gdtr.limit)) {
                _bb("Selector_Offset(>gdtr.limit)");
                _chr(_SetExcept_GP(newcs & 0xfffc + !!flagext));
                _be;
            }
            _be;
        }
        _chr(_s_read_xdt(newcs, GetRef(cs_desc)));
        if (!_IsDescCode(cs_desc) ||
                _GetDesc_DPL(cs_desc) > _GetCPL) {
            _bb("!DescCode/DPL(>CPL)");
            _chr(_SetExcept_GP(newcs & 0xfffc + !!flagext));
            _be;
        }
        if (!_IsDescPresent(cs_desc)) {
            _bb("!DescPresent");
            _chr(_SetExcept_NP(newcs & 0xfffc + !!flagext));
            _be;
        }
        if (_IsDescCodeNonConform(cs_desc) &&
                _GetDesc_DPL(cs_desc) < _GetCPL) {
            _bb("DescCodeNonConform,DPL(<CPL)");
            if (!_GetEFLAGS_VM) {
                _bb("EFLAS_VM(0)");
                /* INTER-PRIVILEGE-LEVEL-INTERRUPT */
                _chr(_SetExcept_CE(0));
                _be;
            } else {
                _bb("EFLAS_VM(1)");
                if (_GetDesc_DPL(cs_desc) != 0) {
                    _bb("DPL(!0)");
                    _chr(_SetExcept_GP(newcs));
                    _be;
                }
                /* INTERRUPT-FROM-VIRTUAL-8086-MODE */
                switch (vcpu.data.tr.sys.type) {
                case VCPU_DESC_SYS_TYPE_TSS_32_AVL:
                case VCPU_DESC_SYS_TYPE_TSS_32_BUSY:
                    _bb("tr.type(32)");
                    _chr(_s_test_tss(4, 8));
                    _chr(_s_read_tss(4, GetRef(newesp), 4));
                    _chr(_s_read_tss(8, GetRef(newss), 2));
                    _be;
                    break;
                case VCPU_DESC_SYS_TYPE_TSS_16_AVL:
                case VCPU_DESC_SYS_TYPE_TSS_16_BUSY:
                    _bb("tr.type(16)");
                    _chr(_s_test_tss(2, 4));
                    _chr(_s_read_tss(2, GetRef(newesp), 2));
                    _chr(_s_read_tss(4, GetRef(newss), 2));
                    _be;
                    break;
                default:
                    _impossible_r_;
                    break;
                }
                if (_IsSelectorNull(newss)) {
                    _bb("newss(null)");
                    _chr(_SetExcept_TS(!!flagext));
                    _be;
                }
                if (_GetSelector_RPL(newss) != _GetDesc_DPL(cs_desc)) {
                    _bb("RPL(newss)!=DPL(newcs)");
                    _chr(_SetExcept_TS(newss & 0xfffc + !!flagext));
                    _be;
                }
                _chr(_s_read_xdt(newss, GetRef(ss_desc)));
                if (_GetDesc_DPL(ss_desc) !=
                        _GetDesc_DPL(cs_desc)) {
                    _bb("DPL(newss)!=DPL(newcs)");
                    _chr(_SetExcept_TS(newss & 0xfffc + !!flagext));
                    _be;
                }
                if (!_IsDescPresent(ss_desc)) {
                    _bb("!DescPresent(newss)");
                    _chr(_SetExcept_SS(newss & 0xfffc + !!flagext));
                    _be;
                }
                /*_chr(_ksa_load_sreg(&css, &ss_xdesc));
                if (_IsDescSys32(gate_desc)) {
                    _bb("DescSys32(gate)");
                    _chr(_kma_test_logical(&css, newesp, 36, 1, 0x00, 0));
                    _be;
                } else {
                    _bb("!DescSys32(gate)");
                    _chr(_kma_test_logical(&css, newesp, 18, 1, 0x00, 0));
                    _be;
                }*/
                oldeflags = vcpu.data.eflags;
                _ClrEFLAGS_VM;
                _MakeCPL(0x00);
                _ClrEFLAGS_TF;
                _ClrEFLAGS_NT;
                _ClrEFLAGS_RF;
                if (_IsDescIntGate(gate_desc))
                    _ClrEFLAGS_IF;
                oldss = vcpu.data.ss.selector;
                oldesp = vcpu.data.esp;
                _chr(_ksa_load_sreg(&vcpu.data.ss, newss));
                vcpu.data.esp = newesp;
                if (_IsDescSys32(gate_desc)) {
                    _bb("DescSys32(gate)");
                    xs_sel = vcpu.data.gs.selector;
                    _chr(_kec_push(GetRef(xs_sel), 4));
                    xs_sel = vcpu.data.fs.selector;
                    _chr(_kec_push(GetRef(xs_sel), 4));
                    xs_sel = vcpu.data.ds.selector;
                    _chr(_kec_push(GetRef(xs_sel), 4));
                    xs_sel = vcpu.data.es.selector;
                    _chr(_kec_push(GetRef(xs_sel), 4));
                    xs_sel = oldss;
                    _chr(_kec_push(GetRef(xs_sel), 4));
                    _chr(_kec_push(GetRef(oldesp), 4));
                    _chr(_kec_push(GetRef(oldeflags), 4));
                    xs_sel = vcpu.data.cs.selector;
                    _chr(_kec_push(GetRef(xs_sel), 4));
                    _chr(_kec_push(GetRef(vcpu.data.eip), 4));
                    _chr(_s_load_gs(0x0000));
                    _chr(_s_load_fs(0x0000));
                    _chr(_s_load_ds(0x0000));
                    _chr(_s_load_es(0x0000));
                    _chr(_ksa_load_sreg(&vcpu.data.cs, newcs));
                    switch (byte) {
                    case 2:
                        vcpu.data.eip = GetMax16(_GetDescGate_Offset(gate_desc));
                        break;
                    case 4:
                        vcpu.data.eip = GetMax32(_GetDescGate_Offset(gate_desc));
                        break;
                    default:
                        _bb("byte");
                        _chr(_SetExcept_CE(byte));
                        _be;
                        break;
                    }
                    _be;
                } else {
                    _bb("!DescSys32(gate)");
                    _chr(_SetExcept_CE(0));
                    _be;
                }
                _be;
            }
            _be;
        } else {
            _bb("DescCodeConform/DPL(>=CPL)");
            _chr(_SetExcept_CE(0));
            _be;
        }
        _be;
    }
    _ce;
}
static void _ser_ret_far_real(t_nubit16 newcs, t_nubit32 neweip, t_nubit16 parambyte, t_nubit16 byte) {
    _cb("_ser_ret_far_real");
    if (_IsProtected) _impossible_r_;
    _chr(_kec_ret_far(newcs, neweip, parambyte, byte));
    _ce;
}
static void _ser_ret_far_same(t_nubit16 newcs, t_nubit32 neweip, t_nubit16 parambyte, t_nubit16 byte) {
    _cb("_ser_ret_far_same");
    if (!_IsProtected) _impossible_r_;
    _chr(_kec_ret_far(newcs, neweip, parambyte, byte));
    _ce;
}
_______todo _ser_ret_far_outer(t_nubit16 newcs, t_nubit32 neweip, t_nubit16 parambyte, t_nubit16 byte) {
    _cb("_ser_ret_far_outer");
    if (!_IsProtected) _impossible_r_;
    _chr(_SetExcept_CE(0));
    _ce;
}
static void _ser_jmp_far_real(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    _cb("_ser_jmp_far_real");
    if (_IsProtected) _impossible_r_;
    _chr(_kec_jmp_far(newcs, neweip, byte));
    _ce;
}
static void _ser_jmp_far_cs_conf(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    t_nubit64 descriptor;
    _cb("_ser_jmp_far_cs_conf");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(newcs, GetRef(descriptor)));
    if (!_IsDescCodeConform(descriptor)) _impossible_r_;
    if (_GetDesc_DPL(descriptor) > _GetCPL) {
        _bb("DPL(>CPL)");
        _chr(_SetExcept_GP(newcs));
        _be;
    }
    if (!_IsDescPresent(descriptor)) {
        _bb("!DescPresent");
        _chr(_SetExcept_NP(newcs));
        _be;
    }
    _chr(_kec_jmp_far(newcs, neweip, byte));
    _ce;
}
static void _ser_jmp_far_cs_nonc(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    t_nubit64 descriptor;
    _cb("_ser_jmp_far_cs_nonc");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(newcs, GetRef(descriptor)));
    if (!_IsDescCodeNonConform(descriptor)) _impossible_r_;
    if (_GetDesc_DPL(descriptor) != _GetCPL ||
            _GetSelector_RPL(newcs) > _GetCPL) {
        _bb("DPL(!CPL)/RPL(>CPL)");
        _chr(_SetExcept_GP(newcs));
        _be;
    }
    if (!_IsDescPresent(descriptor)) {
        _bb("!DescPresent");
        _chr(_SetExcept_NP(newcs));
        _be;
    }
    _chr(_kec_jmp_far(newcs, neweip, byte));
    _ce;
}
_______todo _ser_jmp_far_call_gate(t_nubit16 newcs) {
    t_nubit64 descriptor;
    _cb("_ser_jmp_far_call_gate");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(newcs, GetRef(descriptor)));
    if (!_IsDescCallGate(descriptor)) _impossible_r_;
    _chr(_SetExcept_CE(0));
    _ce;
}
_______todo _ser_jmp_far_task_gate(t_nubit16 newcs) {
    t_nubit64 descriptor;
    _cb("_ser_jmp_far_task_gate");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(newcs, GetRef(descriptor)));
    if (!_IsDescTaskGate(descriptor)) _impossible_r_;
    _chr(_SetExcept_CE(0));
    _ce;
}
_______todo _ser_jmp_far_tss(t_nubit16 newcs) {
    t_nubit64 descriptor;
    _cb("_ser_jmp_far_tss");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(newcs, GetRef(descriptor)));
    if (!_IsDescTSS(descriptor)) _impossible_r_;
    _chr(_SetExcept_CE(0));
    _ce;
}
/* regular execute control */
static void _e_push(t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_e_push");
    _chr(_kec_push(rdata, byte));
    _ce;
}
static void _e_pop(t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_e_pop");
    _chr(_kec_pop(rdata, byte));
    _ce;
}
static void _e_call_far(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    t_nubit64 descriptor;
    _cb("_e_call_far");
    if (!_IsProtected) {
        _bb("!Protected");
        _chr(_ser_call_far_real(newcs, neweip, byte));
        _be;
    } else {
        _bb("Protected");
        if (_IsSelectorNull(newcs)) {
            _bb("newcs(null)");
            _chr(_SetExcept_GP(0));
            _be;
        }
        _chr(_s_read_xdt(newcs, GetRef(descriptor)));
        if (_IsDescCodeConform(descriptor))
            _chr(_ser_call_far_cs_conf(newcs, neweip, byte));
        else if (_IsDescCodeNonConform(descriptor))
            _chr(_ser_call_far_cs_nonc(newcs, neweip, byte));
        else if (_IsDescCallGate(descriptor))
            _chr(_ser_call_far_call_gate(newcs));
        else if (_IsDescTaskGate(descriptor))
            _chr(_ser_call_far_task_gate(newcs));
        else if (_IsDescTSS(descriptor))
            _chr(_ser_call_far_tss(newcs));
        else {
            _bb("desc(invalid)");
            _chr(_SetExcept_GP(newcs));
            _be;
        }
        _be;
    }
    _ce;
}
static void _e_call_near(t_nubit32 neweip, t_nubit8 byte) {
    _cb("_e_call_near");
    _chr(_kec_call_near(neweip, byte));
    _ce;
}
_______todo _e_int3(t_nubit8 byte) {
    _cb("_e_int3");
    if (!_GetCR0_PE) {
        _bb("Real");
        _chr(_ser_int_real(0x03, byte));
        _be;
    } else {
        _bb("!Real");
        _chr(_ser_int_protected(0x03, byte, 0));
        _be;
    }
    _ce;
}
_______todo _e_into(t_nubit8 byte) {
    _cb("_e_into");
    if (_GetEFLAGS_OF) {
        _bb("EFLAGS_OF(1)");
        if (!_GetCR0_PE) {
            _bb("Real");
            _chr(_ser_int_real(0x04, byte));
            _be;
        } else {
            _bb("!Real");
            _chr(_ser_int_protected(0x04, byte, 0));
            _be;
        }
        _be;
    }
    _ce;
}
_______todo _e_int_n(t_nubit8 intid, t_nubit8 byte) {
    _cb("_e_int_n");
    if (!_GetCR0_PE) {
        _bb("Real");
        _chr(_ser_int_real(intid, byte));
        _be;
    } else {
        _bb("!Real");
        if (_GetEFLAGS_VM && _GetEFLAGS_IOPL < 3) {
            _bb("EFLAGAS_VM(1),IOPL(<3)");
            _chr(_SetExcept_GP(0));
            _be;
        } else {
            _bb("EFLAGS_VM(0)/IOPL(3)");
            _chr(_ser_int_protected(intid, byte, 0));
            _be;
        }
        _be;
    }
    _ce;
}
_______todo _e_intr_n(t_nubit8 intid, t_nubit8 byte) {
    _cb("_e_intr_n");
    if (!_GetCR0_PE) {
        _bb("Real");
        _chr(_ser_int_real(intid, byte));
        _be;
    } else {
        _bb("!Real");
        _chr(_ser_int_protected(intid, byte, 1));
        _be;
    }
    _ce;
}
_______todo _e_except_n(t_nubit8 exid, t_nubit8 byte) {
    _cb("_e_except_n");
    vcpuins.data.except &= ~(1 << exid);
    if (!_GetCR0_PE) {
        _bb("Real");
        _chr(_ser_int_real(exid, byte));
        _be;
    } else {
        _bb("!Real");
        _chr(_SetExcept_UD(0));
        _be;
    }
    _ce;
}
_______todo _e_iret(t_nubit8 byte) {
    t_nubit16 newcs, newss, newds, newes, newfs, newgs;
    t_nubit32 neweip = Zero32, newesp, neweflags = Zero32;
    t_nubit32 xs_sel;
    t_nubit32 mask = VCPU_EFLAGS_RESERVED;
    t_cpu_data_sreg ccs = vcpu.data.cs;
    _cb("_e_iret");
    if (!_GetCR0_PE) {
        _bb("Real");
        switch (byte) {
        case 2:
            _bb("byte(2)");
            _chr(_s_test_ss_pop(6));
            _chr(_kec_pop(GetRef(neweip), 2));
            _chr(_kec_pop(GetRef(xs_sel), 2));
            newcs = GetMax16(xs_sel);
            _chr(_kec_pop(GetRef(neweflags), 2));
            mask |= 0xffff0000;
            _be;
            break;
        case 4:
            _bb("byte(4)");
            _chr(_s_test_ss_pop(12));
            _chr(_kec_pop(GetRef(neweip), 4));
            _chr(_kec_pop(GetRef(xs_sel), 4));
            newcs = GetMax16(xs_sel);
            _chr(_kec_pop(GetRef(neweflags), 4));
            /* vcpu.data.eflags = (neweflags & 0x00257fd5) | (vcpu.data.eflags & 0x001a0000); */
            _be;
            break;
        default:
            _bb("byte");
            _chr(_SetExcept_CE(byte));
            _be;
            break;
        }
        _chr(_ksa_load_sreg(&ccs, newcs));
        _chr(_kma_test_logical(&ccs, neweip, 0x01, 0, 0x00, 1));
        vcpu.data.cs = ccs;
        vcpu.data.eip = neweip;
        vcpu.data.eflags = (neweflags & ~mask) | (vcpu.data.eflags & mask);
        _be;
    } else {
        _bb("!Real");
        if (_GetEFLAGS_VM) {
            _bb("V86");
            /* RETURN-FROM-VIRTUAL-8086-MODE */
            if (_GetEFLAGS_IOPL == 3) {
                _bb("EFLAGS_IOPL(3)");
                switch (byte) {
                case 2:
                    _bb("byte(2)");
                    _chr(_s_test_ss_pop(6));
                    _chr(_kec_pop(GetRef(neweip), 2));
                    _chr(_kec_pop(GetRef(xs_sel), 2));
                    newcs = GetMax16(xs_sel);
                    _chr(_kec_pop(GetRef(neweflags), 2));
                    mask |= (0xffff0000 | VCPU_EFLAGS_IOPL);
                    _be;
                    break;
                case 4:
                    _bb("byte(4)");
                    _chr(_s_test_ss_pop(12));
                    _chr(_kec_pop(GetRef(neweip), 4));
                    _chr(_kec_pop(GetRef(xs_sel), 4));
                    newcs = GetMax16(xs_sel);
                    _chr(_kec_pop(GetRef(neweflags), 4));
                    mask |= (VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL);
                    _be;
                    break;
                default:
                    _bb("byte");
                    _chr(_SetExcept_CE(byte));
                    _be;
                    break;
                }
                _chr(_ksa_load_sreg(&ccs, newcs));
                _chr(_kma_test_logical(&ccs, neweip, 0x01, 0, 0x00, 1));
                vcpu.data.cs = ccs;
                vcpu.data.eip = neweip;
                vcpu.data.eflags = (neweflags & ~mask) | (vcpu.data.eflags & mask);
                _be;
            } else {
                _bb("EFLAGS_IOPL(!3)");
                /* trap to virtual-8086 monitor */
                vcpuins.data.oldcpu = vcpu;
                _chr(_SetExcept_GP(0));
                _be;
            }
            _be;
        } else if (_GetEFLAGS_NT) {
            _bb("Nested");
            _chr(_SetExcept_CE(0));
            _be;
        } else {
            _bb("Protected,!Nested");
            switch (byte) {
            case 2:
                _bb("byte(2)");
                _chr(_s_test_ss_pop(6));
                _chr(_kec_pop(GetRef(neweip), 2));
                _chr(_kec_pop(GetRef(xs_sel), 2));
                newcs = GetMax16(xs_sel);
                _chr(_kec_pop(GetRef(neweflags), 2));
                _be;
                break;
            case 4:
                _bb("byte(4)");
                _chr(_s_test_ss_pop(12));
                _chr(_kec_pop(GetRef(neweip), 4));
                _chr(_kec_pop(GetRef(xs_sel), 4));
                newcs = GetMax16(xs_sel);
                _chr(_kec_pop(GetRef(neweflags), 4));
                _be;
                break;
            default:
                _bb("byte");
                _chr(_SetExcept_CE(byte));
                _be;
                break;
            }
            if (GetBit(neweflags, VCPU_EFLAGS_VM) && !_GetCPL) {
                _bb("neweflags(VM),CPL(0)");
                /* return to v86 */
                _chr(_s_test_ss_pop(24));
                vcpu.data.eflags = (neweflags & ~mask) | (vcpu.data.eflags & mask);
                _chr(_ksa_load_sreg(&ccs, newcs));
                _chr(_kma_test_logical(&ccs, neweip, 0x01, 0, 0x00, 1));
                vcpu.data.cs = ccs;
                vcpu.data.eip = neweip;
                _chr(_kec_pop(GetRef(newesp), 4));
                _chr(_kec_pop(GetRef(xs_sel), 4));
                newss = GetMax16(xs_sel);
                _chr(_kec_pop(GetRef(xs_sel), 4));
                newes = GetMax16(xs_sel);
                _chr(_kec_pop(GetRef(xs_sel), 4));
                newds = GetMax16(xs_sel);
                _chr(_kec_pop(GetRef(xs_sel), 4));
                newfs = GetMax16(xs_sel);
                _chr(_kec_pop(GetRef(xs_sel), 4));
                newgs = GetMax16(xs_sel);
                _chr(_s_load_ss(newss));
                vcpu.data.esp = newesp;
                _chr(_s_load_es(newes));
                _chr(_s_load_ds(newds));
                _chr(_s_load_fs(newfs));
                _chr(_s_load_gs(newgs));
                _MakeCPL(0x03);
                _be;
            } else {
                _bb("neweflags(!VM)/CPL(!0)");
                /* return to proctected */
                _chr(_SetExcept_CE(0));
                _be;
            }
            _be;
        }
        _be;
    }
    _ce;
}
static void _e_jcc(t_nubit32 csrc, t_nubit8 byte, t_bool condition) {
    t_nubit32 neweip = vcpu.data.eip;
    _cb("_e_jcc");
    if (condition) {
        _bb("condition(1)");
        switch (byte) {
        case 1:
            neweip += (t_nsbit8)csrc;
            break;
        case 2:
            neweip += (t_nsbit16)csrc;
            break;
        case 4:
            neweip += (t_nsbit32)csrc;
            break;
        default:
            _bb("byte");
            _chr(_SetExcept_CE(byte));
            _be;
            break;
        }
        _chr(_kec_jmp_near(neweip, _GetOperandSize));
        _be;
    }
    _ce;
}
static void _e_jmp_far(t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    t_nubit64 descriptor;
    _cb("_e_jmp_far");
    if (!_IsProtected) {
        _bb("Protected(0)");
        _chr(_ser_jmp_far_real(newcs, neweip, byte));
        _be;
    } else {
        _bb("Protected(1)");
        if (_IsSelectorNull(newcs)) {
            _bb("selector(null)");
            _chr(_SetExcept_GP(0));
            _be;
        }
        _chr(_s_read_xdt(newcs, GetRef(descriptor)));
        if (_IsDescCodeConform(descriptor))
            _chr(_ser_jmp_far_cs_conf(newcs, neweip, byte));
        else if (_IsDescCodeNonConform(descriptor))
            _chr(_ser_jmp_far_cs_nonc(newcs, neweip, byte));
        else if (_IsDescCallGate(descriptor))
            _chr(_ser_jmp_far_call_gate(newcs));
        else if (_IsDescTaskGate(descriptor))
            _chr(_ser_jmp_far_task_gate(newcs));
        else if (_IsDescTSS(descriptor))
            _chr(_ser_jmp_far_tss(newcs));
        else {
            _bb("newcs(invalid)");
            _chr(_SetExcept_GP(newcs));
            _be;
        }
        _be;
    }
    vcpuins.data.opr1 = vcpu.data.cs.selector;
    vcpuins.data.opr2 = vcpu.data.eip;
    _ce;
}
static void _e_jmp_near(t_nubit32 neweip, t_nubit8 byte) {
    _cb("_e_jmp_near");
    _chr(_kec_jmp_near(neweip, byte));
    _ce;
}
static void _e_load_far(t_cpu_data_sreg *rsreg, t_vaddrcc rdest, t_nubit16 selector, t_nubit32 offset, t_nubit8 byte) {
    _cb("_e_load_far");
    _chr(_s_load_sreg(rsreg, selector));
    switch (byte) {
    case 2:
        _bb("byte(2)");
        _chr(_m_write_ref(rdest, GetRef(offset), 2));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_m_write_ref(rdest, GetRef(offset), 4));
        _be;
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    _ce;
}
static void _e_loopcc(t_nsbit8 csrc, t_bool condition) {
    t_nubit32 cecx;
    t_nubit32 neweip = vcpu.data.eip;
    _cb("_e_loopcc");
    switch (_GetAddressSize) {
    case 2:
        vcpu.data.cx--;
        cecx = GetMax16(vcpu.data.ecx);
        break;
    case 4:
        vcpu.data.ecx--;
        cecx = GetMax32(vcpu.data.ecx);
        break;
    default:
        _impossible_r_;
        break;
    }
    if (cecx && condition) {
        _bb("cecx(!0),condition(1)");
        neweip += csrc;
        _chr(_kec_jmp_near(neweip, _GetOperandSize));
        _be;
    }
    _ce;
}
static void _e_ret_near(t_nubit16 parambyte, t_nubit8 byte) {
    _cb("_e_ret_near");
    _chr(_kec_ret_near(parambyte, byte));
    _ce;
}
static void _e_ret_far(t_nubit16 parambyte, t_nubit16 byte) {
    t_nubit16 newcs;
    t_nubit32 xs_sel;
    t_nubit32 neweip = 0;
    t_nubit64 descriptor;
    _cb("_e_ret_far");
    switch (byte) {
    case 2:
        _bb("byte(2)");
        _chr(_s_test_ss_pop(4));
        _chr(_kec_pop(GetRef(neweip), 2));
        _chr(_kec_pop(GetRef(xs_sel), 2));
        newcs = GetMax16(xs_sel);
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_s_test_ss_pop(8));
        _chr(_kec_pop(GetRef(neweip), 4));
        _chr(_kec_pop(GetRef(xs_sel), 4));
        newcs = GetMax16(xs_sel);
        _be;
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    if (!_IsProtected) {
        _bb("!Protected");
        _chr(_ser_ret_far_real(newcs, neweip, parambyte, byte));
        _be;
    } else {
        _bb("Protected");
        if (_IsSelectorNull(newcs)) {
            _bb("selector(null)");
            _chr(_SetExcept_GP(0));
            _be;
        }
        _chr(_s_read_xdt(newcs, GetRef(descriptor)));
        if (!_IsDescCode(descriptor)) {
            _bb("!DescCode");
            _chr(_SetExcept_GP(newcs));
            _be;
        }
        if (_GetSelector_RPL(newcs) < _GetCPL) {
            _bb("RPL(<CPL)");
            _chr(_SetExcept_GP(newcs));
            _be;
        }
        if (_IsDescCodeConform(descriptor)) {
            _bb("DescCodeConform");
            if (_GetDesc_DPL(descriptor) > _GetSelector_RPL(newcs)) {
                _bb("DPL(>RPL)");
                _chr(_SetExcept_GP(newcs));
                _be;
            }
            _be;
        }
        if (!_IsDescPresent(descriptor)) {
            _bb("!DescPresent");
            _chr(_SetExcept_NP(newcs));
            _be;
        }
        if (_GetSelector_RPL(newcs) > _GetCPL) {
            _bb("RPL(>CPL)");
            _chr(_ser_ret_far_outer(newcs, neweip, parambyte, byte));
            _be;
        } else {
            _bb("RPL(<=CPL)");
            _chr(_ser_ret_far_same(newcs, neweip, parambyte, byte));
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

static void _kaf_calc_CF() {
    _cb("_kaf_calc_CF");
    switch (vcpuins.data.type) {
    case ADC8:
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, (
                    (_GetEFLAGS_CF && vcpuins.data.opr2 == Max8) ?
                    1 : ((vcpuins.data.result < vcpuins.data.opr1) || (vcpuins.data.result < vcpuins.data.opr2))));
        break;
    case ADC16:
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, (
                    (_GetEFLAGS_CF && vcpuins.data.opr2 == Max16) ?
                    1 : ((vcpuins.data.result < vcpuins.data.opr1) || (vcpuins.data.result < vcpuins.data.opr2))));
        break;
    case ADC32:
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, (
                    (_GetEFLAGS_CF && vcpuins.data.opr2 == Max32) ?
                    1 : ((vcpuins.data.result < vcpuins.data.opr1) || (vcpuins.data.result < vcpuins.data.opr2))));
        break;
    case ADD8:
    case ADD16:
    case ADD32:
        MakeBit(vcpu.data.eflags,VCPU_EFLAGS_CF,(vcpuins.data.result < vcpuins.data.opr1) || (vcpuins.data.result < vcpuins.data.opr2));
        break;
    case SBB8:
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, (vcpuins.data.opr1 < vcpuins.data.result) ||
                (_GetEFLAGS_CF && (vcpuins.data.opr2 == Max8)));
        break;
    case SBB16:
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, (vcpuins.data.opr1 < vcpuins.data.result) ||
                (_GetEFLAGS_CF && (vcpuins.data.opr2 == Max16)));
        break;
    case SBB32:
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, (vcpuins.data.opr1 < vcpuins.data.result) ||
                (_GetEFLAGS_CF && (vcpuins.data.opr2 == Max32)));
        break;
    case SUB8:
    case SUB16:
    case SUB32:
    case CMP8:
    case CMP16:
    case CMP32:
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, vcpuins.data.opr1 < vcpuins.data.opr2);
        break;
    default:
        _bb("type");
        _chr(_SetExcept_CE(vcpuins.data.type));
        _be;
        break;
    }
    _ce;
}
static void _kaf_calc_OF() {
    _cb("_kaf_calc_OF");
    switch (vcpuins.data.type) {
    case ADC8:
    case ADD8:
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                (GetMSB8(vcpuins.data.opr1) == GetMSB8(vcpuins.data.opr2)) && (GetMSB8(vcpuins.data.opr1) != GetMSB8(vcpuins.data.result)));
        break;
    case ADC16:
    case ADD16:
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                (GetMSB16(vcpuins.data.opr1) == GetMSB16(vcpuins.data.opr2)) && (GetMSB16(vcpuins.data.opr1) != GetMSB16(vcpuins.data.result)));
        break;
    case ADC32:
    case ADD32:
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                (GetMSB32(vcpuins.data.opr1) == GetMSB32(vcpuins.data.opr2)) && (GetMSB32(vcpuins.data.opr1) != GetMSB32(vcpuins.data.result)));
        break;
    case SBB8:
    case SUB8:
    case CMP8:
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                (GetMSB8(vcpuins.data.opr1) != GetMSB8(vcpuins.data.opr2)) && (GetMSB8(vcpuins.data.opr2) == GetMSB8(vcpuins.data.result)));
        break;
    case SBB16:
    case SUB16:
    case CMP16:
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                (GetMSB16(vcpuins.data.opr1) != GetMSB16(vcpuins.data.opr2)) && (GetMSB16(vcpuins.data.opr2) == GetMSB16(vcpuins.data.result)));
        break;
    case SBB32:
    case SUB32:
    case CMP32:
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                (GetMSB32(vcpuins.data.opr1) != GetMSB32(vcpuins.data.opr2)) && (GetMSB32(vcpuins.data.opr2) == GetMSB32(vcpuins.data.result)));
        break;
    default:
        _bb("type");
        _chr(_SetExcept_CE(vcpuins.data.type));
        _be;
        break;
    }
    _ce;
}
static void _kaf_calc_AF() {
    _cb("_kaf_calc_AF");
    MakeBit(vcpu.data.eflags, VCPU_EFLAGS_AF, ((vcpuins.data.opr1 ^ vcpuins.data.opr2) ^ vcpuins.data.result) & 0x10);
    _ce;
}
static void _kaf_calc_PF() {
    t_nubit8 res8 = GetMax8(vcpuins.data.result);
    t_bool even = 1;
    _cb("_kaf_calc_PF");
    while (res8) {
        even = 1 - even;
        res8 &= res8-1;
    }
    MakeBit(vcpu.data.eflags, VCPU_EFLAGS_PF, even);
    _ce;
}
static void _kaf_calc_ZF() {
    _cb("_kaf_calc_ZF");
    MakeBit(vcpu.data.eflags, VCPU_EFLAGS_ZF, !vcpuins.data.result);
    _ce;
}
static void _kaf_calc_SF() {
    _cb("_kaf_calc_SF");
    switch (vcpuins.data.bit) {
    case 8:
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_SF, GetMSB8(vcpuins.data.result));
        break;
    case 16:
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_SF, GetMSB16(vcpuins.data.result));
        break;
    case 32:
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_SF, GetMSB32(vcpuins.data.result));
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(vcpuins.data.bit));
        _be;
        break;
    }
    _ce;
}
static void _kaf_set_flags(t_nubit16 flags) {
    _cb("_kaf_set_flags");
    if (flags & VCPU_EFLAGS_CF) _chr(_kaf_calc_CF());
    if (flags & VCPU_EFLAGS_PF) _chr(_kaf_calc_PF());
    if (flags & VCPU_EFLAGS_AF) _chr(_kaf_calc_AF());
    if (flags & VCPU_EFLAGS_ZF) _chr(_kaf_calc_ZF());
    if (flags & VCPU_EFLAGS_SF) _chr(_kaf_calc_SF());
    if (flags & VCPU_EFLAGS_OF) _chr(_kaf_calc_OF());
    _ce;
}
static void _kas_move_index(t_nubit8 byte, t_bool flagsi, t_bool flagdi) {
    _cb("_kas_move_index");
    switch (_GetAddressSize) {
    case 2:
        if (_GetEFLAGS_DF) {
            if (flagdi) vcpu.data.di -= byte;
            if (flagsi) vcpu.data.si -= byte;
        } else {
            if (flagdi) vcpu.data.di += byte;
            if (flagsi) vcpu.data.si += byte;
        }
        break;
    case 4:
        if (_GetEFLAGS_DF) {
            if (flagdi) vcpu.data.edi -= byte;
            if (flagsi) vcpu.data.esi -= byte;
        } else {
            if (flagdi) vcpu.data.edi += byte;
            if (flagsi) vcpu.data.esi += byte;
        }
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}

#define _kac_arith1(funflag, type8, expr8, type16, expr16, type32, expr32) \
do { \
    switch (bit) { \
    case 8: \
        vcpuins.data.bit = 8; \
        vcpuins.data.type = (type8); \
        vcpuins.data.opr1 = GetMax8(cdest); \
        vcpuins.data.result = GetMax8(expr8); \
        break; \
    case 16: \
        vcpuins.data.bit = 16; \
        vcpuins.data.type = (type16); \
        vcpuins.data.opr1 = GetMax16(cdest); \
        vcpuins.data.result = GetMax16(expr16); \
        break; \
    case 32: \
        vcpuins.data.bit = 32; \
        vcpuins.data.type = (type32); \
        vcpuins.data.opr1 = GetMax32(cdest); \
        vcpuins.data.result = GetMax32(expr32); \
        break; \
    default: _bb("bit"); \
        _chr(_SetExcept_CE(bit)); \
        _be;break; \
    } \
    _chr(_kaf_set_flags(funflag)); \
} while (0)
#define _kac_arith2(funflag, type8, expr8, type12, expr12, type16, expr16, \
    type20, expr20, type32, expr32) \
do { \
    switch (bit) { \
    case 8: \
        vcpuins.data.bit = 8; \
        vcpuins.data.type = (type8); \
        vcpuins.data.opr1 = GetMax8(cdest); \
        vcpuins.data.opr2 = GetMax8(csrc); \
        vcpuins.data.result = GetMax8(expr8); \
        break; \
    case 12: \
        vcpuins.data.bit = 16; \
        vcpuins.data.type = (type12);\
        vcpuins.data.opr1 = GetMax16(cdest); \
        vcpuins.data.opr2 = GetMax16((t_nsbit8)csrc); \
        vcpuins.data.result = GetMax16(expr12); \
        break; \
    case 16: \
        vcpuins.data.bit = 16; \
        vcpuins.data.type = (type16); \
        vcpuins.data.opr1 = GetMax16(cdest); \
        vcpuins.data.opr2 = GetMax16(csrc); \
        vcpuins.data.result = GetMax16(expr16); \
        break; \
    case 20: \
        vcpuins.data.bit = 32; \
        vcpuins.data.type = (type20); \
        vcpuins.data.opr1 = GetMax32(cdest); \
        vcpuins.data.opr2 = GetMax32((t_nsbit8)csrc); \
        vcpuins.data.result = GetMax32(expr20); \
        break; \
    case 32: \
        vcpuins.data.bit = 32; \
        vcpuins.data.type = (type32); \
        vcpuins.data.opr1 = GetMax32(cdest); \
        vcpuins.data.opr2 = GetMax32(csrc); \
        vcpuins.data.result = GetMax32(expr32); \
        break; \
    default: _bb("bit"); \
        _chr(_SetExcept_CE(bit)); \
        _be;break; \
    } \
    _chr(_kaf_set_flags(funflag)); \
} while (0)
static void _a_add(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    _cb("_a_add");
    _kac_arith2(ADD_FLAG,
                ADD8,  (vcpuins.data.opr1 + vcpuins.data.opr2),
                ADD16, (vcpuins.data.opr1 + vcpuins.data.opr2),
                ADD16, (vcpuins.data.opr1 + vcpuins.data.opr2),
                ADD32, (vcpuins.data.opr1 + vcpuins.data.opr2),
                ADD32, (vcpuins.data.opr1 + vcpuins.data.opr2));
    _ce;
}
static void _a_adc(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    _cb("_a_adc");
    _kac_arith2(ADC_FLAG,
                ADC8,  (vcpuins.data.opr1 + vcpuins.data.opr2 + _GetEFLAGS_CF),
                ADC16, (vcpuins.data.opr1 + vcpuins.data.opr2 + _GetEFLAGS_CF),
                ADC16, (vcpuins.data.opr1 + vcpuins.data.opr2 + _GetEFLAGS_CF),
                ADC32, (vcpuins.data.opr1 + vcpuins.data.opr2 + _GetEFLAGS_CF),
                ADC32, (vcpuins.data.opr1 + vcpuins.data.opr2 + _GetEFLAGS_CF));
    _ce;
}
static void _a_and(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    _cb("_a_and");
    _kac_arith2(AND_FLAG,
                AND8,  (vcpuins.data.opr1 & vcpuins.data.opr2),
                AND16, (vcpuins.data.opr1 & vcpuins.data.opr2),
                AND16, (vcpuins.data.opr1 & vcpuins.data.opr2),
                AND32, (vcpuins.data.opr1 & vcpuins.data.opr2),
                AND32, (vcpuins.data.opr1 & vcpuins.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    vcpuins.data.udf |= VCPU_EFLAGS_AF;
    _ce;
}
static void _a_or(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    _cb("_a_or");
    _kac_arith2(OR_FLAG,
                OR8,  (vcpuins.data.opr1 | vcpuins.data.opr2),
                OR16, (vcpuins.data.opr1 | vcpuins.data.opr2),
                OR16, (vcpuins.data.opr1 | vcpuins.data.opr2),
                OR32, (vcpuins.data.opr1 | vcpuins.data.opr2),
                OR32, (vcpuins.data.opr1 | vcpuins.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    vcpuins.data.udf |= VCPU_EFLAGS_AF;
    _ce;
}
static void _a_sbb(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    _cb("_a_sbb");
    _kac_arith2(SBB_FLAG,
                SBB8,  (vcpuins.data.opr1 - (vcpuins.data.opr2 + _GetEFLAGS_CF)),
                SBB16, (vcpuins.data.opr1 - (vcpuins.data.opr2 + _GetEFLAGS_CF)),
                SBB16, (vcpuins.data.opr1 - (vcpuins.data.opr2 + _GetEFLAGS_CF)),
                SBB32, (vcpuins.data.opr1 - (vcpuins.data.opr2 + _GetEFLAGS_CF)),
                SBB32, (vcpuins.data.opr1 - (vcpuins.data.opr2 + _GetEFLAGS_CF)));
    _ce;
}
static void _a_sub(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    _cb("_a_sub");
    _kac_arith2(SUB_FLAG,
                SUB8,  (vcpuins.data.opr1 - vcpuins.data.opr2),
                SUB16, (vcpuins.data.opr1 - vcpuins.data.opr2),
                SUB16, (vcpuins.data.opr1 - vcpuins.data.opr2),
                SUB32, (vcpuins.data.opr1 - vcpuins.data.opr2),
                SUB32, (vcpuins.data.opr1 - vcpuins.data.opr2));
    _ce;
}
static void _a_xor(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    _cb("_a_xor");
    _kac_arith2(XOR_FLAG,
                XOR8,  (vcpuins.data.opr1 ^ vcpuins.data.opr2),
                XOR16, (vcpuins.data.opr1 ^ vcpuins.data.opr2),
                XOR16, (vcpuins.data.opr1 ^ vcpuins.data.opr2),
                XOR32, (vcpuins.data.opr1 ^ vcpuins.data.opr2),
                XOR32, (vcpuins.data.opr1 ^ vcpuins.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    vcpuins.data.udf |= VCPU_EFLAGS_AF;
    _ce;
}
static void _a_cmp(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    _cb("_a_cmp");
    _kac_arith2(CMP_FLAG,
                CMP8,  ((t_nubit8)vcpuins.data.opr1 - (t_nsbit8)vcpuins.data.opr2),
                CMP16, ((t_nubit16)vcpuins.data.opr1 - (t_nsbit8)vcpuins.data.opr2),
                CMP16, ((t_nubit16)vcpuins.data.opr1 - (t_nsbit16)vcpuins.data.opr2),
                CMP32, ((t_nubit32)vcpuins.data.opr1 - (t_nsbit8)vcpuins.data.opr2),
                CMP32, ((t_nubit32)vcpuins.data.opr1 - (t_nsbit32)vcpuins.data.opr2));
    _ce;
}
static void _a_test(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    _cb("_a_test");
    _kac_arith2(TEST_FLAG,
                TEST8,  (vcpuins.data.opr1 & vcpuins.data.opr2),
                TEST16, (vcpuins.data.opr1 & vcpuins.data.opr2),
                TEST16, (vcpuins.data.opr1 & vcpuins.data.opr2),
                TEST32, (vcpuins.data.opr1 & vcpuins.data.opr2),
                TEST32, (vcpuins.data.opr1 & vcpuins.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    vcpuins.data.udf |= VCPU_EFLAGS_AF;
    _ce;
}
static void _a_inc(t_nubit64 cdest, t_nubit8 bit) {
    _cb("_a_inc");
    vcpuins.data.opr2 = 1;
    _kac_arith1(INC_FLAG,
                ADD8,  (vcpuins.data.opr1 + vcpuins.data.opr2),
                ADD16, (vcpuins.data.opr1 + vcpuins.data.opr2),
                ADD32, (vcpuins.data.opr1 + vcpuins.data.opr2));
    _ce;
}
static void _a_dec(t_nubit64 cdest, t_nubit8 bit) {
    _cb("_a_dec");
    vcpuins.data.opr2 = 1;
    _kac_arith1(DEC_FLAG,
                SUB8,  (vcpuins.data.opr1 - vcpuins.data.opr2),
                SUB16, (vcpuins.data.opr1 - vcpuins.data.opr2),
                SUB32, (vcpuins.data.opr1 - vcpuins.data.opr2));
    _ce;
}
static void _a_not(t_nubit64 cdest, t_nubit8 bit) {
    _cb("_a_not");
    vcpuins.data.opr2 = 0;
    _kac_arith1(NOT_FLAG,
                ARITHTYPE_NULL, (~vcpuins.data.opr1),
                ARITHTYPE_NULL, (~vcpuins.data.opr1),
                ARITHTYPE_NULL, (~vcpuins.data.opr1));
    _ce;
}
static void _a_neg(t_nubit64 cdest, t_nubit8 bit) {
    _cb("_a_neg");
    vcpuins.data.opr2 = cdest;
    cdest = 0;
    _kac_arith1(NEG_FLAG,
                SUB8,  (GetMax8(vcpuins.data.opr1) - GetMax8(vcpuins.data.opr2)),
                SUB16, (GetMax16(vcpuins.data.opr1) - GetMax16(vcpuins.data.opr2)),
                SUB32, (GetMax32(vcpuins.data.opr1) - GetMax32(vcpuins.data.opr2)));
    MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, !!vcpuins.data.opr2);
    _ce;
}
static void _a_mul(t_nubit64 csrc, t_nubit8 bit) {
    t_nubit64 cdest;
    _cb("_a_mul");
    switch (bit) {
    case 8:
        _bb("bit(8)");
        vcpuins.data.bit = 8;
        vcpuins.data.opr1 = vcpu.data.al;
        vcpuins.data.opr2 = GetMax8(csrc);
        cdest = GetMax16(vcpu.data.al * vcpuins.data.opr2);
        vcpu.data.ax = GetMax16(cdest);
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF, !!vcpu.data.ah);
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, !!vcpu.data.ah);
        vcpuins.data.result = GetMax16(cdest);
        _be;
        break;
    case 16:
        _bb("bit(16)");
        vcpuins.data.bit = 16;
        vcpuins.data.opr1 = vcpu.data.ax;
        vcpuins.data.opr2 = GetMax16(csrc);
        cdest = GetMax32(vcpu.data.ax * vcpuins.data.opr2);
        vcpu.data.dx = GetMax16(cdest >> 16);
        vcpu.data.ax = GetMax16(cdest);
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF, !!vcpu.data.dx);
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, !!vcpu.data.dx);
        vcpuins.data.result = GetMax32(cdest);
        _be;
        break;
    case 32:
        _bb("bit(32)");
        vcpuins.data.bit = 32;
        vcpuins.data.opr1 = vcpu.data.eax;
        vcpuins.data.opr2 = GetMax32(csrc);
        cdest = GetMax64(vcpu.data.eax * vcpuins.data.opr2);
        vcpu.data.edx = GetMax32(cdest >> 32);
        vcpu.data.eax = GetMax32(cdest);
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF, !!vcpu.data.edx);
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, !!vcpu.data.edx);
        vcpuins.data.result = GetMax64(cdest);
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    vcpuins.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                         VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    _ce;
}
static void _a_imul(t_nubit64 csrc, t_nubit8 bit) {
    t_nsbit64 cdest;
    _cb("_a_imul");
    switch (bit) {
    case 8:
        _bb("bit(8)");
        vcpuins.data.bit = 8;
        vcpuins.data.opr1 = vcpu.data.al;
        vcpuins.data.opr2 = GetMax8((t_nsbit8)csrc);
        cdest = GetMax16((t_nsbit8)vcpu.data.al * (t_nsbit8)vcpuins.data.opr2);
        vcpu.data.ax = GetMax16(cdest);
        if (GetMax16(cdest) == (t_nsbit16)((t_nsbit8)vcpu.data.al)) {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        } else {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        vcpuins.data.result = GetMax16(cdest);
        _be;
        break;
    case 16:
        _bb("bit(16)");
        vcpuins.data.bit = 16;
        vcpuins.data.opr1 = vcpu.data.ax;
        vcpuins.data.opr2 = GetMax16((t_nsbit16)csrc);
        cdest = GetMax32((t_nsbit16)vcpu.data.ax * (t_nsbit16)vcpuins.data.opr2);
        vcpu.data.ax = GetMax16(cdest);
        vcpu.data.dx = GetMax16(cdest >> 16);
        if (GetMax32(cdest) == (t_nsbit32)((t_nsbit16)vcpu.data.ax)) {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        } else {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        vcpuins.data.result = GetMax32(cdest);
        _be;
        break;
    case 32:
        _bb("bit(32)");
        vcpuins.data.bit = 32;
        vcpuins.data.opr1 = vcpu.data.eax;
        vcpuins.data.opr2 = GetMax32((t_nsbit32)csrc);
        cdest = GetMax64((t_nsbit32)vcpu.data.eax * (t_nsbit32)vcpuins.data.opr2);
        vcpu.data.eax = GetMax32(cdest);
        vcpu.data.edx = GetMax32(cdest >> 32);
        if (GetMax64(cdest) == (t_nsbit64)((t_nsbit32)vcpu.data.eax)) {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        } else {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        vcpuins.data.result = GetMax64(cdest);
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    vcpuins.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                         VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    _ce;
}
static void _a_div(t_nubit64 csrc, t_nubit8 bit) {
    t_nubit64 temp = 0x0000000000000000;
    _cb("_a_div");
    switch (bit) {
    case 8:
        _bb("bit(8)");
        vcpuins.data.bit = 8;
        vcpuins.data.opr1 = GetMax16(vcpu.data.ax);
        vcpuins.data.opr2 = GetMax8(csrc);
        if (!vcpuins.data.opr2) {
            _bb("opr2(0)");
            _chr(_SetExcept_DE(0));
            _be;
        } else {
            _bb("opr2(!0)");
            temp = GetMax16((t_nubit16)vcpuins.data.opr1 / (t_nubit8)vcpuins.data.opr2);
            if (temp > Max8) {
                _bb("temp(>0xff)");
                _chr(_SetExcept_DE(0));
                _be;
            } else {
                vcpu.data.al = GetMax8(temp);
                vcpu.data.ah = GetMax8((t_nubit16)vcpuins.data.opr1 % (t_nubit8)vcpuins.data.opr2);
            }
            _be;
        }
        vcpuins.data.result = vcpu.data.ax;
        _be;
        break;
    case 16:
        _bb("bit(16)");
        vcpuins.data.bit = 16;
        vcpuins.data.opr1 = GetMax32((vcpu.data.dx << 16) | vcpu.data.ax);
        vcpuins.data.opr2 = GetMax16(csrc);
        if (!vcpuins.data.opr2) {
            _bb("opr2(0)");
            _chr(_SetExcept_DE(0));
            _be;
        } else {
            _bb("opr2(!0)");
            temp = GetMax32((t_nubit32)vcpuins.data.opr1 / (t_nubit16)vcpuins.data.opr2);
            if (temp > Max16) {
                _bb("temp(>0xffff)");
                _chr(_SetExcept_DE(0));
                _be;
            } else {
                vcpu.data.ax = GetMax16(temp);
                vcpu.data.dx = GetMax16((t_nubit32)vcpuins.data.opr1 % (t_nubit16)vcpuins.data.opr2);
            }
            _be;
        }
        vcpuins.data.result = (vcpu.data.dx << 16) | vcpu.data.ax;
        _be;
        break;
    case 32:
        _bb("bit(32)");
        vcpuins.data.bit = 32;
        vcpuins.data.opr1 = GetMax64(((t_nubit64)vcpu.data.edx << 32) | vcpu.data.eax);
        vcpuins.data.opr2 = GetMax32(csrc);
        if (!vcpuins.data.opr2) {
            _bb("opr2(0)");
            _chr(_SetExcept_DE(0));
            _be;
        } else {
            _bb("opr2(!0)");
            temp = GetMax64((t_nubit64)vcpuins.data.opr1 / (t_nubit32)vcpuins.data.opr2);
            if (temp > Max32) {
                _bb("temp(>0xffffffff)");
                _chr(_SetExcept_DE(0));
                _be;
            } else {
                vcpu.data.eax = GetMax32(temp);
                vcpu.data.edx = GetMax32((t_nubit64)vcpuins.data.opr1 % (t_nubit32)vcpuins.data.opr2);
            }
            _be;
        }
        vcpuins.data.result = ((t_nubit64)vcpu.data.edx << 32) | vcpu.data.eax;
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    vcpuins.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF |
                         VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_OF);
    _ce;
}
static void _a_idiv(t_nubit64 csrc, t_nubit8 bit) {
    t_nsbit64 temp;
    _cb("_a_idiv");
    switch (bit) {
    case 8:
        _bb("bit(8)");
        vcpuins.data.bit = 8;
        vcpuins.data.opr1 = GetMax16((t_nsbit16)vcpu.data.ax);
        vcpuins.data.opr2 = GetMax8((t_nsbit8)csrc);
        if (!vcpuins.data.opr2) {
            _bb("opr2(0)");
            _chr(_SetExcept_DE(0));
            _be;
        } else {
            _bb("opr2(!0)");
            temp = GetMax16((t_nsbit16)vcpuins.data.opr1 / (t_nsbit8)vcpuins.data.opr2);
            if ((temp > 0 && GetMax8(temp) > 0x7f) ||
                    (temp < 0 && GetMax8(temp) < 0x80)) {
                _bb("temp(>0x7f/<0x80)");
                _chr(_SetExcept_DE(0));
                _be;
            } else {
                vcpu.data.al = GetMax8(temp);
                vcpu.data.ah = GetMax8((t_nsbit16)vcpuins.data.opr1 % (t_nsbit8)vcpuins.data.opr2);
            }
            _be;
        }
        vcpuins.data.result = vcpu.data.ax;
        _be;
        break;
    case 16:
        _bb("bit(16)");
        vcpuins.data.bit = 16;
        vcpuins.data.opr1 = GetMax32((t_nsbit32)((vcpu.data.dx << 16) | vcpu.data.ax));
        vcpuins.data.opr2 = GetMax16((t_nsbit16)csrc);
        if (!vcpuins.data.opr2) {
            _bb("opr2(0)");
            _chr(_SetExcept_DE(0));
            _be;
        } else {
            _bb("opr2(!0)");
            temp = GetMax32((t_nsbit32)vcpuins.data.opr1 / (t_nsbit16)vcpuins.data.opr2);
            if ((temp > 0 && GetMax16(temp) > 0x7fff) ||
                    (temp < 0 && GetMax16(temp) < 0x8000)) {
                _bb("temp(>0x7fff/<0x8000)");
                _chr(_SetExcept_DE(0));
                _be;
            } else {
                vcpu.data.ax = GetMax16(temp);
                vcpu.data.dx = GetMax16((t_nsbit32)vcpuins.data.opr1 % (t_nsbit16)vcpuins.data.opr2);
            }
            _be;
        }
        vcpuins.data.result = ((t_nubit32)vcpu.data.dx << 16) | vcpu.data.ax;
        _be;
        break;
    case 32:
        _bb("bit(32)");
        vcpuins.data.bit = 32;
        vcpuins.data.opr1 = GetMax64((t_nsbit64)(((t_nubit64)vcpu.data.edx << 32) | vcpu.data.eax));
        vcpuins.data.opr2 = GetMax32((t_nsbit32)csrc);
        if (!vcpuins.data.opr2) {
            _bb("opr2(0)");
            _chr(_SetExcept_DE(0));
            _be;
        } else {
            _bb("opr2(!0)");
            temp = GetMax64((t_nsbit64)vcpuins.data.opr1 / (t_nsbit32)vcpuins.data.opr2);
            if ((temp > 0 && GetMax32(temp) > 0x7fffffff) ||
                    (temp < 0 && GetMax32(temp) < 0x80000000)) {
                _bb("temp(>0x7fffffff/<0x80000000)");
                _chr(_SetExcept_DE(0));
                _be;
            } else {
                vcpu.data.eax = GetMax32(temp);
                vcpu.data.edx = GetMax32((t_nsbit64)vcpuins.data.opr1 % (t_nsbit32)vcpuins.data.opr2);
            }
            _be;
        }
        vcpuins.data.result = ((t_nubit64)vcpu.data.edx << 32) | vcpu.data.eax;
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    vcpuins.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF |
                         VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_OF);
    _ce;
}

static void _a_imul3(t_nubit64 csrc1, t_nubit64 csrc2, t_nubit8 bit) {
    t_nsbit64 cdest;
    _cb("_a_imul3");
    switch (bit) {
    case 12:
        _bb("bit(16+8)");
        vcpuins.data.bit = 16;
        vcpuins.data.opr1 = (t_nsbit16)csrc1;
        vcpuins.data.opr2 = (t_nsbit8)csrc2;
        cdest = GetMax32((t_nsbit16)vcpuins.data.opr1 * (t_nsbit8)vcpuins.data.opr2);
        vcpuins.data.result = GetMax16(cdest);
        if (GetMax32(cdest) != GetMax32((t_nsbit16)vcpuins.data.result)) {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        } else {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        _be;
        break;
    case 16:
        _bb("bit(16+16)");
        vcpuins.data.bit = 16;
        vcpuins.data.opr1 = (t_nsbit16)csrc1;
        vcpuins.data.opr2 = (t_nsbit16)csrc2;
        cdest = GetMax32((t_nsbit16)vcpuins.data.opr1 * (t_nsbit16)vcpuins.data.opr2);
        vcpuins.data.result = GetMax16(cdest);
        if (GetMax32(cdest) != GetMax32((t_nsbit16)vcpuins.data.result)) {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        } else {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        _be;
        break;
    case 20:
        _bb("bit(32+8)");
        vcpuins.data.bit = 32;
        vcpuins.data.opr1 = (t_nsbit32)csrc1;
        vcpuins.data.opr2 = (t_nsbit8)csrc2;
        cdest = GetMax64((t_nsbit32)vcpuins.data.opr1 * (t_nsbit8)vcpuins.data.opr2);
        vcpuins.data.result = GetMax32(cdest);
        if (GetMax64(cdest) != GetMax64((t_nsbit32)vcpuins.data.result)) {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        } else {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        _be;
        break;
    case 32:
        _bb("bit(32+32");
        vcpuins.data.bit = 32;
        vcpuins.data.opr1 = (t_nsbit32)csrc1;
        vcpuins.data.opr2 = (t_nsbit32)csrc2;
        cdest = GetMax64((t_nsbit32)vcpuins.data.opr1 * (t_nsbit32)vcpuins.data.opr2);
        vcpuins.data.result = GetMax32(cdest);
        if (GetMax64(cdest) != GetMax64((t_nsbit32)vcpuins.data.result)) {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        } else {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    vcpuins.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                         VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    _ce;
}

static void _a_rol(t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit) {
    t_nubit8 count;
    t_nubit32 flagcf;
    _cb("_a_rol");
    switch (bit) {
    case 8:
        _bb("bit(8)");
        count = csrc & 0x07;
        vcpuins.data.bit = 8;
        vcpuins.data.opr1 = GetMax8(cdest);
        vcpuins.data.opr2 = count;
        vcpuins.data.result = vcpuins.data.opr1;
        while (count) {
            flagcf = !!GetMSB8(vcpuins.data.result);
            vcpuins.data.result = GetMax8(vcpuins.data.result << 1) | flagcf;
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, !!GetLSB8(vcpuins.data.result));
            count--;
        }
        if (vcpuins.data.opr2 == 1)
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB8(vcpuins.data.result)) ^ _GetEFLAGS_CF));
        else
            vcpuins.data.udf |= VCPU_EFLAGS_OF;
        _be;
        break;
    case 16:
        _bb("bit(16)");
        count = csrc & 0x0f;
        vcpuins.data.bit = 16;
        vcpuins.data.opr1 = GetMax16(cdest);
        vcpuins.data.opr2 = count;
        vcpuins.data.result = vcpuins.data.opr1;
        while (count) {
            flagcf = !!GetMSB16(vcpuins.data.result);
            vcpuins.data.result = GetMax16(vcpuins.data.result << 1) | flagcf;
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, !!GetLSB16(vcpuins.data.result));
            count--;
        }
        if (vcpuins.data.opr2 == 1)
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB16(vcpuins.data.result)) ^ _GetEFLAGS_CF));
        else
            vcpuins.data.udf |= VCPU_EFLAGS_OF;
        _be;
        break;
    case 32:
        _bb("bit(32)");
        count = csrc & 0x1f;
        vcpuins.data.bit = 32;
        vcpuins.data.opr1 = GetMax32(cdest);
        vcpuins.data.opr2 = count;
        vcpuins.data.result = vcpuins.data.opr1;
        while (count) {
            flagcf = !!GetMSB32(vcpuins.data.result);
            vcpuins.data.result = GetMax32(vcpuins.data.result << 1) | flagcf;
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, !!GetLSB32(vcpuins.data.result));
            count--;
        }
        if (vcpuins.data.opr2 == 1)
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB32(vcpuins.data.result)) ^ _GetEFLAGS_CF));
        else
            vcpuins.data.udf |= VCPU_EFLAGS_OF;
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    _ce;
}
static void _a_ror(t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit) {
    t_nubit8 count;
    t_nubit32 flagcf;
    _cb("_a_ror");
    switch (bit) {
    case 8:
        _bb("bit(8)");
        count = csrc & 0x07;
        vcpuins.data.bit = 8;
        vcpuins.data.opr1 = GetMax8(cdest);
        vcpuins.data.opr2 = count;
        vcpuins.data.result = vcpuins.data.opr1;
        while (count) {
            flagcf = GetLSB8(vcpuins.data.result) ? MSB8 : 0;
            vcpuins.data.result = GetMax8(vcpuins.data.result >> 1) | flagcf;
            count--;
        }
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, !!GetMSB8(vcpuins.data.result));
        if (vcpuins.data.opr2 == 1)
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB8(vcpuins.data.result)) ^ (!!GetMSB7(vcpuins.data.result))));
        else
            vcpuins.data.udf |= VCPU_EFLAGS_OF;
        _be;
        break;
    case 16:
        _bb("bit(16)");
        count = csrc & 0x0f;
        vcpuins.data.bit = 16;
        vcpuins.data.opr1 = GetMax16(cdest);
        vcpuins.data.opr2 = count;
        vcpuins.data.result = vcpuins.data.opr1;
        while (count) {
            flagcf = GetLSB16(vcpuins.data.result) ? MSB16 : 0;
            vcpuins.data.result = GetMax16(vcpuins.data.result >> 1) | flagcf;
            count--;
        }
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, !!GetMSB16(vcpuins.data.result));
        if (vcpuins.data.opr2 == 1)
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB16(vcpuins.data.result)) ^ (!!GetMSB15(vcpuins.data.result))));
        else
            vcpuins.data.udf |= VCPU_EFLAGS_OF;
        _be;
        break;
    case 32:
        _bb("bit(32)");
        count = csrc & 0x1f;
        vcpuins.data.bit = 32;
        vcpuins.data.opr1 = GetMax32(cdest);
        vcpuins.data.opr2 = count;
        vcpuins.data.result = vcpuins.data.opr1;
        while (count) {
            flagcf = GetLSB32(vcpuins.data.result) ? MSB32 : 0;
            vcpuins.data.result = GetMax32(vcpuins.data.result >> 1) | flagcf;
            count--;
        }
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, !!GetMSB32(vcpuins.data.result));
        if (vcpuins.data.opr2 == 1)
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB32(vcpuins.data.result)) ^ (!!GetMSB31(vcpuins.data.result))));
        else
            vcpuins.data.udf |= VCPU_EFLAGS_OF;
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    _ce;
}
static void _a_rcl(t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit) {
    t_nubit8 count;
    t_nubit32 flagcf;
    _cb("_a_rcl");
    switch (bit) {
    case 8:
        _bb("bit(8)");
        count = (csrc & 0x1f) % 9;
        vcpuins.data.bit = 8;
        vcpuins.data.opr1 = GetMax8(cdest);
        vcpuins.data.opr2 = count;
        vcpuins.data.result = vcpuins.data.opr1;
        while (count) {
            flagcf = !!GetMSB8(vcpuins.data.result);
            vcpuins.data.result = GetMax8(vcpuins.data.result << 1) | _GetEFLAGS_CF;
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, flagcf);
            count--;
        }
        if (vcpuins.data.opr2 == 1)
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB8(vcpuins.data.result)) ^ _GetEFLAGS_CF));
        else
            vcpuins.data.udf |= VCPU_EFLAGS_OF;
        _be;
        break;
    case 16:
        _bb("bit(16)");
        count = (csrc & 0x1f) % 17;
        vcpuins.data.bit = 16;
        vcpuins.data.opr1 = GetMax16(cdest);
        vcpuins.data.opr2 = count;
        vcpuins.data.result = vcpuins.data.opr1;
        while (count) {
            flagcf = !!GetMSB16(vcpuins.data.result);
            vcpuins.data.result = GetMax16(vcpuins.data.result << 1) | _GetEFLAGS_CF;
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, flagcf);
            count--;
        }
        if (vcpuins.data.opr2 == 1)
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB16(vcpuins.data.result)) ^ _GetEFLAGS_CF));
        else
            vcpuins.data.udf |= VCPU_EFLAGS_OF;
        _be;
        break;
    case 32:
        _bb("bit(32)");
        count = (csrc & 0x1f);
        vcpuins.data.bit = 32;
        vcpuins.data.opr1 = GetMax32(cdest);
        vcpuins.data.opr2 = count;
        vcpuins.data.result = vcpuins.data.opr1;
        while (count) {
            flagcf = !!GetMSB32(vcpuins.data.result);
            vcpuins.data.result = GetMax32(vcpuins.data.result << 1) | _GetEFLAGS_CF;
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, flagcf);
            count--;
        }
        if (vcpuins.data.opr2 == 1)
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB32(vcpuins.data.result)) ^ _GetEFLAGS_CF));
        else
            vcpuins.data.udf |= VCPU_EFLAGS_OF;
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    _ce;
}
static void _a_rcr(t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit) {
    t_nubit8 count;
    t_nubit32 flagcf;
    _cb("_a_rcr");
    switch (bit) {
    case 8:
        _bb("bit(8)");
        count = (csrc & 0x1f) % 9;
        vcpuins.data.bit = 8;
        vcpuins.data.opr1 = GetMax8(cdest);
        vcpuins.data.opr2 = count;
        vcpuins.data.result = vcpuins.data.opr1;
        if (vcpuins.data.opr2 == 1)
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB8(vcpuins.data.result)) ^ _GetEFLAGS_CF));
        else
            vcpuins.data.udf |= VCPU_EFLAGS_OF;
        while (count) {
            flagcf = GetLSB8(vcpuins.data.result);
            vcpuins.data.result = GetMax8(vcpuins.data.result >> 1) | (_GetEFLAGS_CF ? MSB8 : 0);
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, !!flagcf);
            count--;
        }
        _be;
        break;
    case 16:
        _bb("bit(16)");
        count = (csrc & 0x1f) % 17;
        vcpuins.data.bit = 16;
        vcpuins.data.opr1 = GetMax16(cdest);
        vcpuins.data.opr2 = count;
        vcpuins.data.result = vcpuins.data.opr1;
        if (vcpuins.data.opr2 == 1)
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB16(vcpuins.data.result)) ^ _GetEFLAGS_CF));
        else
            vcpuins.data.udf |= VCPU_EFLAGS_OF;
        while (count) {
            flagcf = GetLSB16(vcpuins.data.result);
            vcpuins.data.result = GetMax16(vcpuins.data.result >> 1) | (_GetEFLAGS_CF ? MSB16 : 0);
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, !!flagcf);
            count--;
        }
        _be;
        break;
    case 32:
        _bb("bit(32)");
        count = (csrc & 0x1f);
        vcpuins.data.bit = 32;
        vcpuins.data.opr1 = GetMax32(cdest);
        vcpuins.data.opr2 = count;
        vcpuins.data.result = vcpuins.data.opr1;
        if (vcpuins.data.opr2 == 1)
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB32(vcpuins.data.result)) ^ _GetEFLAGS_CF));
        else
            vcpuins.data.udf |= VCPU_EFLAGS_OF;
        while (count) {
            flagcf = GetLSB32(vcpuins.data.result);
            vcpuins.data.result = GetMax32(vcpuins.data.result >> 1) | (_GetEFLAGS_CF ? MSB32 : 0);
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, !!flagcf);
            count--;
        }
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    _ce;
}
static void _a_shl(t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit) {
    t_nubit8 count;
    _cb("_a_shl");
    count = csrc & 0x1f;
    vcpuins.data.opr2 = count;
    if (count >= bit) vcpuins.data.udf |= VCPU_EFLAGS_CF;
    switch (bit) {
    case 8:
        _bb("bit(8)");
        vcpuins.data.bit = 8;
        vcpuins.data.opr1 = GetMax8(cdest);
        vcpuins.data.result = vcpuins.data.opr1;
        while (count) {
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, !!GetMSB8(vcpuins.data.result));
            vcpuins.data.result = GetMax8(vcpuins.data.result << 1);
            count--;
        }
        if (vcpuins.data.opr2 == 1)
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB8(vcpuins.data.result)) ^ _GetEFLAGS_CF));
        else
            vcpuins.data.udf |= VCPU_EFLAGS_OF;
        if (vcpuins.data.opr2 != 0) {
            _bb("opr2(!0)");
            _chr(_kaf_set_flags(SHL_FLAG));
            vcpuins.data.udf |= VCPU_EFLAGS_AF;
            _be;
        }
        _be;
        break;
    case 16:
        _bb("bit(16)");
        vcpuins.data.bit = 16;
        vcpuins.data.opr1 = GetMax16(cdest);
        vcpuins.data.result = vcpuins.data.opr1;
        while (count) {
            MakeBit(vcpu.data.eflags,VCPU_EFLAGS_CF, !!GetMSB16(vcpuins.data.result));
            vcpuins.data.result = GetMax16(vcpuins.data.result << 1);
            count--;
        }
        if (vcpuins.data.opr2 == 1)
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB16(vcpuins.data.result)) ^ _GetEFLAGS_CF));
        else
            vcpuins.data.udf |= VCPU_EFLAGS_OF;
        if (vcpuins.data.opr2 != 0) {
            _bb("opr2(!0)");
            _chr(_kaf_set_flags(SHL_FLAG));
            vcpuins.data.udf |= VCPU_EFLAGS_AF;
            _be;
        }
        _be;
        break;
    case 32:
        _bb("bit(32)");
        vcpuins.data.bit = 32;
        vcpuins.data.opr1 = GetMax32(cdest);
        vcpuins.data.result = vcpuins.data.opr1;
        while (count) {
            MakeBit(vcpu.data.eflags,VCPU_EFLAGS_CF, !!GetMSB32(vcpuins.data.result));
            vcpuins.data.result = GetMax32(vcpuins.data.result << 1);
            count--;
        }
        if (vcpuins.data.opr2 == 1)
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB32(vcpuins.data.result)) ^ _GetEFLAGS_CF));
        else
            vcpuins.data.udf |= VCPU_EFLAGS_OF;
        if (vcpuins.data.opr2 != 0) {
            _bb("opr2(!0)");
            _chr(_kaf_set_flags(SHL_FLAG));
            vcpuins.data.udf |= VCPU_EFLAGS_AF;
            _be;
        }
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    _ce;
}
static void _a_shr(t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit) {
    t_nubit8 count;
    _cb("_a_shr");
    count = csrc & 0x1f;
    vcpuins.data.opr2 = count;
    if (count >= bit) vcpuins.data.udf |= VCPU_EFLAGS_CF;
    switch (bit) {
    case 8:
        _bb("bit(8)");
        vcpuins.data.bit = 8;
        vcpuins.data.opr1 = GetMax8(cdest);
        vcpuins.data.result = vcpuins.data.opr1;
        while (count) {
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, (!!GetLSB8(vcpuins.data.result)));
            vcpuins.data.result = GetMax8(vcpuins.data.result >> 1);
            count--;
        }
        if (vcpuins.data.opr2 == 1)
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF, (!!GetMSB8(vcpuins.data.opr1)));
        else
            vcpuins.data.udf |= VCPU_EFLAGS_OF;
        if (vcpuins.data.opr2 != 0) {
            _bb("opr2(!0)");
            _chr(_kaf_set_flags(SHR_FLAG));
            vcpuins.data.udf |= VCPU_EFLAGS_AF;
            _be;
        }
        _be;
        break;
    case 16:
        _bb("bit(16)");
        vcpuins.data.bit = 16;
        vcpuins.data.opr1 = GetMax16(cdest);
        vcpuins.data.result = vcpuins.data.opr1;
        while (count) {
            MakeBit(vcpu.data.eflags,VCPU_EFLAGS_CF, (!!GetLSB16(vcpuins.data.result)));
            vcpuins.data.result = GetMax16(vcpuins.data.result >> 1);
            count--;
        }
        if (vcpuins.data.opr2 == 1)
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF, (!!GetMSB16(vcpuins.data.opr1)));
        else
            vcpuins.data.udf |= VCPU_EFLAGS_OF;
        if (vcpuins.data.opr2 != 0) {
            _bb("opr2(!0)");
            _chr(_kaf_set_flags(SHR_FLAG));
            vcpuins.data.udf |= VCPU_EFLAGS_AF;
            _be;
        }
        _be;
        break;
    case 32:
        _bb("bit(32)");
        vcpuins.data.bit = 32;
        vcpuins.data.opr1 = GetMax32(cdest);
        vcpuins.data.result = vcpuins.data.opr1;
        while (count) {
            MakeBit(vcpu.data.eflags,VCPU_EFLAGS_CF, (!!GetLSB32(vcpuins.data.result)));
            vcpuins.data.result = GetMax32(vcpuins.data.result >> 1);
            count--;
        }
        if (vcpuins.data.opr2 == 1)
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF, (!!GetMSB32(vcpuins.data.opr1)));
        else
            vcpuins.data.udf |= VCPU_EFLAGS_OF;
        if (vcpuins.data.opr2 != 0) {
            _bb("opr2(!0)");
            _chr(_kaf_set_flags(SHR_FLAG));
            vcpuins.data.udf |= VCPU_EFLAGS_AF;
            _be;
        }
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    _ce;
}
static void _a_sar(t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit) {
    t_nubit8 count;
    t_bool tempcf;
    _cb("_a_shr");
    count = csrc & 0x1f;
    vcpuins.data.opr2 = count;
    switch (bit) {
    case 8:
        _bb("bit(8)");
        vcpuins.data.bit = 8;
        vcpuins.data.opr1 = GetMax8(cdest);
        vcpuins.data.opr2 = count;
        vcpuins.data.result = vcpuins.data.opr1;
        while (count) {
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, (!!GetLSB8(vcpuins.data.result)));
            tempcf = GetMSB8(vcpuins.data.result);
            vcpuins.data.result = GetMax8((t_nsbit8)vcpuins.data.result >> 1);
            count--;
        }
        if (vcpuins.data.opr2 == 1) _ClrEFLAGS_OF;
        else vcpuins.data.udf |= VCPU_EFLAGS_OF;
        if (vcpuins.data.opr2 != 0) {
            _bb("opr2(!0)");
            _chr(_kaf_set_flags(SAR_FLAG));
            vcpuins.data.udf |= VCPU_EFLAGS_AF;
            _be;
        }
        _be;
        break;
    case 16:
        _bb("bit(16)");
        vcpuins.data.bit = 16;
        vcpuins.data.opr1 = GetMax16(cdest);
        vcpuins.data.opr2 = count;
        vcpuins.data.result = vcpuins.data.opr1;
        while (count) {
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, (!!GetLSB16(vcpuins.data.result)));
            vcpuins.data.result = GetMax16((t_nsbit16)vcpuins.data.result >> 1);
            count--;
        }
        if (vcpuins.data.opr2 == 1) _ClrEFLAGS_OF;
        else vcpuins.data.udf |= VCPU_EFLAGS_OF;
        if (vcpuins.data.opr2 != 0) {
            _bb("opr2(!0)");
            _chr(_kaf_set_flags(SAR_FLAG));
            vcpuins.data.udf |= VCPU_EFLAGS_AF;
            _be;
        }
        _be;
        break;
    case 32:
        _bb("bit(32)");
        vcpuins.data.bit = 32;
        vcpuins.data.opr1 = GetMax32(cdest);
        vcpuins.data.opr2 = count;
        vcpuins.data.result = vcpuins.data.opr1;
        while (count) {
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, (!!GetLSB32(vcpuins.data.result)));
            vcpuins.data.result = GetMax32((t_nsbit32)vcpuins.data.result >> 1);
            count--;
        }
        if (vcpuins.data.opr2 == 1) _ClrEFLAGS_OF;
        else vcpuins.data.udf |= VCPU_EFLAGS_OF;
        if (vcpuins.data.opr2 != 0) {
            _bb("opr2(!0)");
            _chr(_kaf_set_flags(SAR_FLAG));
            vcpuins.data.udf |= VCPU_EFLAGS_AF;
            _be;
        }
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    _ce;
}

static void _p_ins(t_nubit8 byte) {
    t_nubit32 cedi, data = 0;
    _cb("_p_ins");
    switch (_GetAddressSize) {
    case 2:
        cedi = vcpu.data.di;
        break;
    case 4:
        cedi = vcpu.data.edi;
        break;
    default:
        _impossible_r_;
        break;
    }
    switch (byte) {
    case 1:
        _bb("byte(1)");
        _chr(_m_test_access(&vcpu.data.es, cedi, 1, 1));
        _chr(_p_input(vcpu.data.dx, GetRef(data), 1));
        _chr(_s_write_es(cedi, GetRef(data), 1));
        _chr(_kas_move_index(1, 0, 1));
        _be;
        break;
    case 2:
        _bb("byte(2)");
        _chr(_m_test_access(&vcpu.data.es, cedi, 2, 1));
        _chr(_p_input(vcpu.data.dx, GetRef(data), 2));
        _chr(_s_write_es(cedi, GetRef(data), 2));
        _chr(_kas_move_index(2, 0, 1));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_m_test_access(&vcpu.data.es, cedi, 4, 1));
        _chr(_p_input(vcpu.data.dx, GetRef(data), 4));
        _chr(_s_write_es(cedi, GetRef(data), 4));
        _chr(_kas_move_index(4, 0, 1));
        _be;
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    _ce;
}
static void _p_outs(t_nubit8 byte) {
    t_nubit32 cesi, data = 0;
    _cb("_p_outs");
    switch (_GetAddressSize) {
    case 2:
        cesi = vcpu.data.si;
        break;
    case 4:
        cesi = vcpu.data.esi;
        break;
    default:
        _impossible_r_;
        break;
    }
    switch (byte) {
    case 1:
        _bb("byte(1)");
        _chr(_m_read_logical(vcpuins.data.roverds, cesi, GetRef(data), 1));
        _chr(_p_output(vcpu.data.dx, GetRef(data), 1));
        _chr(_kas_move_index(1, 1, 0));
        _be;
        break;
    case 2:
        _bb("byte(2)");
        _chr(_m_read_logical(vcpuins.data.roverds, cesi, GetRef(data), 2));
        _chr(_p_output(vcpu.data.dx, GetRef(data), 2));
        _chr(_kas_move_index(2, 1, 0));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_m_read_logical(vcpuins.data.roverds, cesi, GetRef(data), 4));
        _chr(_p_output(vcpu.data.dx, GetRef(data), 4));
        _chr(_kas_move_index(4, 1, 0));
        _be;
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    _ce;
}
static void _m_movs(t_nubit8 byte) {
    t_nubit32 data = 0;
    t_nubit32 cesi, cedi;
    _cb("_m_movs");
    switch (_GetAddressSize) {
    case 2:
        cesi = vcpu.data.si;
        cedi = vcpu.data.di;
        break;
    case 4:
        cesi = vcpu.data.esi;
        cedi = vcpu.data.edi;
        break;
    default:
        _impossible_r_;
        break;
    }
    switch (byte) {
    case 1:
        _bb("byte(1)");
        _chr(_m_read_logical(vcpuins.data.roverds, cesi, GetRef(data), 1));
        _chr(_s_write_es(cedi, GetRef(data), 1));
        _chr(_kas_move_index(1, 1, 1));
        _be;
        break;
    case 2:
        _bb("byte(2)");
        _chr(_m_read_logical(vcpuins.data.roverds, cesi, GetRef(data), 2));
        _chr(_s_write_es(cedi, GetRef(data), 2));
        _chr(_kas_move_index(2, 1, 1));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_m_read_logical(vcpuins.data.roverds, cesi, GetRef(data), 4));
        _chr(_s_write_es(cedi, GetRef(data), 4));
        _chr(_kas_move_index(4, 1, 1));
        _be;
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    _ce;
}
static void _m_stos(t_nubit8 byte) {
    t_nubit32 cedi;
    _cb("_m_stos");
    switch (_GetAddressSize) {
    case 2:
        cedi = vcpu.data.di;
        break;
    case 4:
        cedi = vcpu.data.edi;
        break;
    default:
        _impossible_r_;
        break;
    }
    switch (byte) {
    case 1:
        _bb("byte(1)");
        _chr(_s_write_es(cedi, GetRef(vcpu.data.al), 1));
        _chr(_kas_move_index(1, 0, 1));
        _be;
        break;
    case 2:
        _bb("byte(2)");
        _chr(_s_write_es(cedi, GetRef(vcpu.data.ax), 2));
        _chr(_kas_move_index(2, 0, 1));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_s_write_es(cedi, GetRef(vcpu.data.eax), 4));
        _chr(_kas_move_index(4, 0, 1));
        _be;
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    _ce;
}
static void _m_lods(t_nubit8 byte) {
    t_nubit32 cesi = 0x00000000;
    _cb("_m_lods");
    switch (_GetAddressSize) {
    case 2:
        cesi = vcpu.data.si;
        break;
    case 4:
        cesi = vcpu.data.esi;
        break;
    default:
        _impossible_r_;
        break;
    }
    switch (byte) {
    case 1:
        _bb("byte(1)");
        vcpuins.data.bit = 8;
        _chr(_m_read_logical(vcpuins.data.roverds, cesi, GetRef(vcpu.data.al), 1));
        _chr(_kas_move_index(1, 1, 0));
        vcpuins.data.result = vcpu.data.al;
        _be;
        break;
    case 2:
        _bb("byte(2)");
        vcpuins.data.bit = 16;
        _chr(_m_read_logical(vcpuins.data.roverds, cesi, GetRef(vcpu.data.ax), 2));
        _chr(_kas_move_index(2, 1, 0));
        vcpuins.data.result = vcpu.data.ax;
        _be;
        break;
    case 4:
        _bb("byte(4)");
        vcpuins.data.bit = 32;
        _chr(_m_read_logical(vcpuins.data.roverds, cesi, GetRef(vcpu.data.eax), 4));
        _chr(_kas_move_index(4, 1, 0));
        vcpuins.data.result = vcpu.data.eax;
        _be;
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    _ce;
}
static void _a_cmps(t_nubit8 bit) {
    t_nubit32 cesi, cedi;
    _cb("_a_cmps");
    vcpuins.data.opr1 = 0;
    vcpuins.data.opr2 = 0;
    switch (_GetAddressSize) {
    case 2:
        cesi = vcpu.data.si;
        cedi = vcpu.data.di;
        break;
    case 4:
        cesi = vcpu.data.esi;
        cedi = vcpu.data.edi;
        break;
    default:
        _impossible_r_;
        break;
    }
    switch (bit) {
    case 8:
        _bb("bit(8)");
        vcpuins.data.bit = 8;
        vcpuins.data.type = CMP8;
        _chr(_m_read_logical(vcpuins.data.roverds, cesi, GetRef(vcpuins.data.opr1), 1));
        _chr(_s_read_es(cedi, GetRef(vcpuins.data.opr2), 1));
        vcpuins.data.result = GetMax8(vcpuins.data.opr1 - vcpuins.data.opr2);
        _chr(_kas_move_index(1, 1, 1));
        _be;
        break;
    case 16:
        _bb("bit(16)");
        vcpuins.data.bit = 16;
        vcpuins.data.type = CMP16;
        _chr(_m_read_logical(vcpuins.data.roverds, cesi, GetRef(vcpuins.data.opr1), 2));
        _chr(_s_read_es(cedi, GetRef(vcpuins.data.opr2), 2));
        vcpuins.data.result = GetMax16(vcpuins.data.opr1 - vcpuins.data.opr2);
        _chr(_kas_move_index(2, 1, 1));
        _be;
        break;
    case 32:
        _bb("bit(32)");
        vcpuins.data.bit = 32;
        vcpuins.data.type = CMP32;
        _chr(_m_read_logical(vcpuins.data.roverds, cesi, GetRef(vcpuins.data.opr1), 4));
        _chr(_s_read_es(cedi, GetRef(vcpuins.data.opr2), 4));
        vcpuins.data.result = GetMax32(vcpuins.data.opr1 - vcpuins.data.opr2);
        _chr(_kas_move_index(4, 1, 1));
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    _chr(_kaf_set_flags(CMP_FLAG));
    _ce;
}
static void _a_scas(t_nubit8 bit) {
    t_nubit32 cedi;
    _cb("_a_scas");
    vcpuins.data.opr1 = 0;
    vcpuins.data.opr2 = 0;
    switch (_GetAddressSize) {
    case 2:
        cedi = vcpu.data.di;
        break;
    case 4:
        cedi = vcpu.data.edi;
        break;
    default:
        _impossible_r_;
        break;
    }
    switch (bit) {
    case 8:
        _bb("bit(8)");
        vcpuins.data.bit = 8;
        vcpuins.data.type = CMP8;
        _chr(vcpuins.data.opr1 = vcpu.data.al);
        _chr(_s_read_es(cedi, GetRef(vcpuins.data.opr2), 1));
        vcpuins.data.result = GetMax8(vcpuins.data.opr1 - vcpuins.data.opr2);
        _chr(_kas_move_index(1, 0, 1));
        _be;
        break;
    case 16:
        _bb("bit(16)");
        vcpuins.data.bit = 16;
        vcpuins.data.type = CMP16;
        _chr(vcpuins.data.opr1 = vcpu.data.ax);
        _chr(_s_read_es(cedi, GetRef(vcpuins.data.opr2), 2));
        vcpuins.data.result = GetMax16(vcpuins.data.opr1 - vcpuins.data.opr2);
        _chr(_kas_move_index(2, 0, 1));
        _be;
        break;
    case 32:
        _bb("bit(32)");
        vcpuins.data.bit = 32;
        vcpuins.data.type = CMP32;
        _chr(vcpuins.data.opr1 = vcpu.data.eax);
        _chr(_s_read_es(cedi, GetRef(vcpuins.data.opr2), 4));
        vcpuins.data.result = GetMax32(vcpuins.data.opr1 - vcpuins.data.opr2);
        _chr(_kas_move_index(4, 0, 1));
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    _chr(_kaf_set_flags(CMP_FLAG));
    _ce;
}
#define _adv _chr(_d_skip(1))
static void UndefinedOpcode() {
    _cb("UndefinedOpcode");
    vcpu = vcpuins.data.oldcpu;
    if (!_GetCR0_PE) {
        PRINTF("The NXVM CPU has encountered an illegal instruction at L%08X.\n", vcpu.data.cs.base + vcpu.data.eip);
        deviceStop();
    }
    _chr(_SetExcept_UD(0));
    _ce;
}
static void ADD_RM8_R8() {
    _cb("ADD_RM8_R8");
    i386(0x00) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_a_add(vcpuins.data.crm, vcpuins.data.cr, 8));
    vcpuins.data.crm = vcpuins.data.result;
    _chr(_m_write_rm(1));
    _ce;
}
static void ADD_RM32_R32() {
    _cb("ADD_RM32_R32");
    i386(0x01) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_a_add(vcpuins.data.crm, vcpuins.data.cr, _GetOperandSize * 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(_GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(2, 2));
        _chr(_m_read_rm(2));
        _chr(_a_add(vcpuins.data.crm, vcpuins.data.cr, 16));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(2));
    }
    _ce;
}
static void ADD_R8_RM8() {
    _cb("ADD_R8_RM8");
    i386(0x02) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_a_add(vcpuins.data.cr, vcpuins.data.crm, 8));
    _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), 1));
    _ce;
}
static void ADD_R32_RM32() {
    _cb("ADD_R32_RM32");
    i386(0x03) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_a_add(vcpuins.data.cr, vcpuins.data.crm, _GetOperandSize * 8));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), _GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(2, 2));
        _chr(_m_read_rm(2));
        _chr(_a_add(vcpuins.data.cr, vcpuins.data.crm, 16));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), 2));
    }
    _ce;
}
static void ADD_AL_I8() {
    _cb("ADD_AL_I8");
    i386(0x04) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_a_add(vcpu.data.al, vcpuins.data.cimm, 8));
    vcpu.data.al = GetMax8(vcpuins.data.result);
    _ce;
}
static void ADD_EAX_I32() {
    _cb("ADD_EAX_I32");
    i386(0x05) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(2));
            _chr(_a_add(vcpu.data.ax, vcpuins.data.cimm, 16));
            vcpu.data.ax = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_d_imm(4));
            _chr(_a_add(vcpu.data.eax, vcpuins.data.cimm, 32));
            vcpu.data.eax = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        _chr(_a_add(vcpu.data.ax, vcpuins.data.cimm, 16));
        vcpu.data.ax = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void PUSH_ES() {
    t_nubit32 xs_sel;
    _cb("PUSH_ES");
    i386(0x06) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    xs_sel = vcpu.data.es.selector;
    _chr(_e_push(GetRef(xs_sel), _GetOperandSize));
    _ce;
}
static void POP_ES() {
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    t_nubit32 xs_sel;
    _cb("POP_ES");
    i386(0x07) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_e_pop(GetRef(xs_sel), _GetOperandSize));
    _chr(_s_load_es(GetMax16(xs_sel)));
    _ce;
}
static void OR_RM8_R8() {
    _cb("OR_RM8_R8");
    i386(0x08) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_a_or(vcpuins.data.crm, vcpuins.data.cr, 8));
    vcpuins.data.crm = vcpuins.data.result;
    _chr(_m_write_rm(1));
    _ce;
}
static void OR_RM32_R32() {
    _cb("OR_RM32_R32");
    i386(0x09) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_a_or(vcpuins.data.crm, vcpuins.data.cr, _GetOperandSize * 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(_GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(2, 2));
        _chr(_m_read_rm(2));
        _chr(_a_or(vcpuins.data.crm, vcpuins.data.cr, 16));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(2));
    }
    _ce;
}
static void OR_R8_RM8() {
    _cb("OR_R8_RM8");
    i386(0x0a) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_a_or(vcpuins.data.cr, vcpuins.data.crm, 8));
    _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), 1));
    _ce;
}
static void OR_R32_RM32() {
    _cb("OR_R32_RM32");
    i386(0x0b) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_a_or(vcpuins.data.cr, vcpuins.data.crm, _GetOperandSize * 8));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), _GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(2, 2));
        _chr(_m_read_rm(2));
        _chr(_a_or(vcpuins.data.cr, vcpuins.data.crm, 16));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), 2));
    }
    _ce;
}
static void OR_AL_I8() {
    _cb("OR_AL_I8");
    i386(0x0c) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_a_or(vcpu.data.al, vcpuins.data.cimm, 8));
    vcpu.data.al = GetMax8(vcpuins.data.result);
    _ce;
}
static void OR_EAX_I32() {
    _cb("OR_EAX_I32");
    i386(0x0d) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(2));
            _chr(_a_or(vcpu.data.ax, vcpuins.data.cimm, 16));
            vcpu.data.ax = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_d_imm(4));
            _chr(_a_or(vcpu.data.eax, vcpuins.data.cimm, 32));
            vcpu.data.eax = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        _chr(_a_or(vcpu.data.ax, vcpuins.data.cimm, 16));
        vcpu.data.ax = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void PUSH_CS() {
    t_nubit32 xs_sel;
    _cb("PUSH_CS");
    i386(0x0e) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    xs_sel = vcpu.data.cs.selector;
    _chr(_e_push(GetRef(xs_sel), _GetOperandSize));
    _ce;
}
static void POP_CS() {
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    t_nubit32 xs_sel;
    _cb("POP_CS");
    i386(0x0f) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_e_pop(GetRef(xs_sel), _GetOperandSize));
    _chr(_s_load_cs(GetMax16(xs_sel)));
    _ce;
}
static void INS_0F() {
    t_nubit8 opcode = 0x00;
    _cb("INS_0F");
    i386(0x0f) {
        _adv;
        _chr(_s_read_cs(vcpu.data.eip, GetRef(opcode), 1));
        _chr(ExecFun(vcpuins.connect.insTable_0f[opcode]));
    }
    else
        POP_CS();
    _ce;
}
static void ADC_RM8_R8() {
    _cb("ADC_RM8_R8");
    i386(0x10) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_a_adc(vcpuins.data.crm, vcpuins.data.cr, 8));
    vcpuins.data.crm = vcpuins.data.result;
    _chr(_m_write_rm(1));
    _ce;
}
static void ADC_RM32_R32() {
    _cb("ADC_RM32_R32");
    i386(0x11) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_a_adc(vcpuins.data.crm, vcpuins.data.cr, _GetOperandSize * 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(_GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(2, 2));
        _chr(_m_read_rm(2));
        _chr(_a_adc(vcpuins.data.crm, vcpuins.data.cr, 16));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(2));
    }
    _ce;
}
static void ADC_R8_RM8() {
    _cb("ADC_R8_RM8");
    i386(0x12) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_a_adc(vcpuins.data.cr, vcpuins.data.crm, 8));
    _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), 1));
    _ce;
}
static void ADC_R32_RM32() {
    _cb("ADC_R32_RM32");
    i386(0x13) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_a_adc(vcpuins.data.cr, vcpuins.data.crm, _GetOperandSize * 8));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), _GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(2, 2));
        _chr(_m_read_rm(2));
        _chr(_a_adc(vcpuins.data.cr, vcpuins.data.crm, 16));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), 2));
    }
    _ce;
}
static void ADC_AL_I8() {
    _cb("ADC_AL_I8");
    i386(0x14) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_a_adc(vcpu.data.al, vcpuins.data.cimm, 8));
    vcpu.data.al = GetMax8(vcpuins.data.result);
    _ce;
}
static void ADC_EAX_I32() {
    _cb("ADC_EAX_I32");
    i386(0x15) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(2));
            _chr(_a_adc(vcpu.data.ax, vcpuins.data.cimm, 16));
            vcpu.data.ax = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_d_imm(4));
            _chr(_a_adc(vcpu.data.eax, vcpuins.data.cimm, 32));
            vcpu.data.eax = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        _chr(_a_adc(vcpu.data.ax, vcpuins.data.cimm, 16));
        vcpu.data.ax = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void PUSH_SS() {
    t_nubit32 xs_sel;
    _cb("PUSH_SS");
    i386(0x16) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    xs_sel = vcpu.data.ss.selector;
    _chr(_e_push(GetRef(xs_sel), _GetOperandSize));
    _ce;
}
static void POP_SS() {
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    t_nubit32 xs_sel;
    _cb("POP_SS");
    i386(0x17) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_e_pop(GetRef(xs_sel), _GetOperandSize));
    _chr(_s_load_ss(GetMax16(xs_sel)));
    _ce;
}
static void SBB_RM8_R8() {
    _cb("SBB_RM8_R8");
    i386(0x18) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_a_sbb(vcpuins.data.crm, vcpuins.data.cr, 8));
    vcpuins.data.crm = vcpuins.data.result;
    _chr(_m_write_rm(1));
    _ce;
}
static void SBB_RM32_R32() {
    _cb("SBB_RM32_R32");
    i386(0x19) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_a_sbb(vcpuins.data.crm, vcpuins.data.cr, _GetOperandSize * 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(_GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(2, 2));
        _chr(_m_read_rm(2));
        _chr(_a_sbb(vcpuins.data.crm, vcpuins.data.cr, 16));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(2));
    }
    _ce;
}
static void SBB_R8_RM8() {
    _cb("SBB_R8_RM8");
    i386(0x1a) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_a_sbb(vcpuins.data.cr, vcpuins.data.crm, 8));
    _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), 1));
    _ce;
}
static void SBB_R32_RM32() {
    _cb("SBB_R32_RM32");
    i386(0x1b) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_a_sbb(vcpuins.data.cr, vcpuins.data.crm, _GetOperandSize * 8));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), _GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(2, 2));
        _chr(_m_read_rm(2));
        _chr(_a_sbb(vcpuins.data.cr, vcpuins.data.crm, 16));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), 2));
    }
    _ce;
}
static void SBB_AL_I8() {
    _cb("SBB_AL_I8");
    i386(0x1c) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_a_sbb(vcpu.data.al, vcpuins.data.cimm, 8));
    vcpu.data.al = GetMax8(vcpuins.data.result);
    _ce;
}
static void SBB_EAX_I32() {
    _cb("SBB_EAX_I32");
    i386(0x1d) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(2));
            _chr(_a_sbb(vcpu.data.ax, vcpuins.data.cimm, 16));
            vcpu.data.ax = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_d_imm(4));
            _chr(_a_sbb(vcpu.data.eax, vcpuins.data.cimm, 32));
            vcpu.data.eax = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        _chr(_a_sbb(vcpu.data.ax, vcpuins.data.cimm, 16));
        vcpu.data.ax = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void PUSH_DS() {
    t_nubit32 xs_sel;
    _cb("PUSH_DS");
    i386(0x1e) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    xs_sel = vcpu.data.ds.selector;
    _chr(_e_push(GetRef(xs_sel), _GetOperandSize));
    _ce;
}
static void POP_DS() {
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    t_nubit32 xs_sel;
    _cb("POP_DS");
    i386(0x1f) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_e_pop(GetRef(xs_sel), _GetOperandSize));
    _chr(_s_load_ds(GetMax16(xs_sel)));
    _ce;
}
static void AND_RM8_R8() {
    _cb("AND_RM8_R8");
    i386(0x20) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_a_and(vcpuins.data.crm, vcpuins.data.cr, 8));
    vcpuins.data.crm = vcpuins.data.result;
    _chr(_m_write_rm(1));
    _ce;
}
static void AND_RM32_R32() {
    _cb("AND_RM32_R32");
    i386(0x21) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_a_and(vcpuins.data.crm, vcpuins.data.cr, _GetOperandSize * 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(_GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(2, 2));
        _chr(_m_read_rm(2));
        _chr(_a_and(vcpuins.data.crm, vcpuins.data.cr, 16));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(2));
    }
    _ce;
}
static void AND_R8_RM8() {
    _cb("AND_R8_RM8");
    i386(0x22) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_a_and(vcpuins.data.cr, vcpuins.data.crm, 8));
    _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), 1));
    _ce;
}
static void AND_R32_RM32() {
    _cb("AND_R32_RM32");
    i386(0x23) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_a_and(vcpuins.data.cr, vcpuins.data.crm, _GetOperandSize * 8));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), _GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(2, 2));
        _chr(_m_read_rm(2));
        _chr(_a_and(vcpuins.data.cr, vcpuins.data.crm, 16));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), 2));
    }
    _ce;
}
static void AND_AL_I8() {
    _cb("AND_AL_I8");
    i386(0x24) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_a_and(vcpu.data.al, vcpuins.data.cimm, 8));
    vcpu.data.al = GetMax8(vcpuins.data.result);
    _ce;
}
static void AND_EAX_I32() {
    _cb("AND_EAX_I32");
    i386(0x25) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(2));
            _chr(_a_and(vcpu.data.ax, vcpuins.data.cimm, 16));
            vcpu.data.ax = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_d_imm(4));
            _chr(_a_and(vcpu.data.eax, vcpuins.data.cimm, 32));
            vcpu.data.eax = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        _chr(_a_and(vcpu.data.ax, vcpuins.data.cimm, 16));
        vcpu.data.ax = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void PREFIX_ES() {
    _cb("PREFIX_ES");
    i386(0x26) {
        _adv;
        vcpuins.data.roverds = &vcpu.data.es;
        vcpuins.data.roverss = &vcpu.data.es;
    }
    else {
        vcpu.data.ip++;
        vcpuins.data.roverds = &vcpu.data.es;
        vcpuins.data.roverss = &vcpu.data.es;
    }
    _ce;
}
static void DAA() {
    t_nubit8 oldAL = vcpu.data.al;
    _cb("DAA");
    i386(0x27)
    _adv;
    else
        vcpu.data.ip++;
    if (((vcpu.data.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
        vcpu.data.al += 0x06;
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF,
                _GetEFLAGS_CF || ((vcpu.data.al < oldAL) || (vcpu.data.al < 0x06)));
        _SetEFLAGS_AF;
    } else _ClrEFLAGS_AF;
    if (((vcpu.data.al & 0xf0) > 0x90) || _GetEFLAGS_CF) {
        vcpu.data.al += 0x60;
        _SetEFLAGS_CF;
    } else _ClrEFLAGS_CF;
    vcpuins.data.bit = 8;
    vcpuins.data.result = (t_nubit32)vcpu.data.al;
    _chr(_kaf_set_flags(DAA_FLAG));
    vcpuins.data.udf |= VCPU_EFLAGS_OF;
    _ce;
}
static void SUB_RM8_R8() {
    _cb("SUB_RM8_R8");
    i386(0x28) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_a_sub(vcpuins.data.crm, vcpuins.data.cr, 8));
    vcpuins.data.crm = vcpuins.data.result;
    _chr(_m_write_rm(1));
    _ce;
}
static void SUB_RM32_R32() {
    _cb("SUB_RM32_R32");
    i386(0x29) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_a_sub(vcpuins.data.crm, vcpuins.data.cr, _GetOperandSize * 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(_GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(2, 2));
        _chr(_m_read_rm(2));
        _chr(_a_sub(vcpuins.data.crm, vcpuins.data.cr, 16));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(2));
    }
    _ce;
}
static void SUB_R8_RM8() {
    _cb("SUB_R8_RM8");
    i386(0x2a) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_a_sub(vcpuins.data.cr, vcpuins.data.crm, 8));
    _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), 1));
    _ce;
}
static void SUB_R32_RM32() {
    _cb("SUB_R32_RM32");
    i386(0x2b) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_a_sub(vcpuins.data.cr, vcpuins.data.crm, _GetOperandSize * 8));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), _GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(2, 2));
        _chr(_m_read_rm(2));
        _chr(_a_sub(vcpuins.data.cr, vcpuins.data.crm, 16));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), 2));
    }
    _ce;
}
static void SUB_AL_I8() {
    _cb("SUB_AL_I8");
    i386(0x2c) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_a_sub(vcpu.data.al, vcpuins.data.cimm, 8));
    vcpu.data.al = GetMax8(vcpuins.data.result);
    _ce;
}
static void SUB_EAX_I32() {
    _cb("SUB_EAX_I32");
    i386(0x2d) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(2));
            _chr(_a_sub(vcpu.data.ax, vcpuins.data.cimm, 16));
            vcpu.data.ax = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_d_imm(4));
            _chr(_a_sub(vcpu.data.eax, vcpuins.data.cimm, 32));
            vcpu.data.eax = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        _chr(_a_sub(vcpu.data.ax, vcpuins.data.cimm, 16));
        vcpu.data.ax = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void PREFIX_CS() {
    _cb("PREFIX_CS");
    i386(0x2e) {
        _adv;
        vcpuins.data.roverds = &vcpu.data.cs;
        vcpuins.data.roverss = &vcpu.data.cs;
    }
    else {
        vcpu.data.ip++;
        vcpuins.data.roverds = &vcpu.data.cs;
        vcpuins.data.roverss = &vcpu.data.cs;
    }
    _ce;
}
static void DAS() {
    t_nubit8 oldAL = vcpu.data.al;
    _cb("DAS");
    i386(0x2f)
    _adv;
    else
        vcpu.data.ip++;
    if (((vcpu.data.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
        vcpu.data.al -= 0x06;
        MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF,
                _GetEFLAGS_CF || (oldAL < 0x06));
        _SetEFLAGS_AF;
    } else _ClrEFLAGS_AF;
    if ((vcpu.data.al > 0x9f) || _GetEFLAGS_CF) {
        vcpu.data.al -= 0x60;
        _SetEFLAGS_CF;
    } else _ClrEFLAGS_CF;
    vcpuins.data.bit = 8;
    vcpuins.data.result = (t_nubit32)vcpu.data.al;
    _chr(_kaf_set_flags(DAS_FLAG));
    vcpuins.data.udf |= VCPU_EFLAGS_OF;
    _ce;
}
static void XOR_RM8_R8() {
    _cb("XOR_RM8_R8");
    i386(0x30) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_a_xor(vcpuins.data.crm, vcpuins.data.cr, 8));
    vcpuins.data.crm = vcpuins.data.result;
    _chr(_m_write_rm(1));
    _ce;
}
static void XOR_RM32_R32() {
    _cb("XOR_RM32_R32");
    i386(0x31) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_a_xor(vcpuins.data.crm, vcpuins.data.cr, _GetOperandSize * 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(_GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(2, 2));
        _chr(_m_read_rm(2));
        _chr(_a_xor(vcpuins.data.crm, vcpuins.data.cr, 16));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(2));
    }
    _ce;
}
static void XOR_R8_RM8() {
    _cb("XOR_R8_RM8");
    i386(0x32) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_a_xor(vcpuins.data.cr, vcpuins.data.crm, 8));
    _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), 1));
    _ce;
}
static void XOR_R32_RM32() {
    _cb("XOR_R32_RM32");
    i386(0x33) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_a_xor(vcpuins.data.cr, vcpuins.data.crm, _GetOperandSize * 8));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), _GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(2, 2));
        _chr(_m_read_rm(2));
        _chr(_a_xor(vcpuins.data.cr, vcpuins.data.crm, 16));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), 2));
    }
    _ce;
}
static void XOR_AL_I8() {
    _cb("XOR_AL_I8");
    i386(0x34) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_a_xor(vcpu.data.al, vcpuins.data.cimm, 8));
    vcpu.data.al = GetMax8(vcpuins.data.result);
    _ce;
}
static void XOR_EAX_I32() {
    _cb("XOR_EAX_I32");
    i386(0x35) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperxorSize(2)");
            _chr(_d_imm(2));
            _chr(_a_xor(vcpu.data.ax, vcpuins.data.cimm, 16));
            vcpu.data.ax = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperxorSize(4)");
            _chr(_d_imm(4));
            _chr(_a_xor(vcpu.data.eax, vcpuins.data.cimm, 32));
            vcpu.data.eax = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        _chr(_a_xor(vcpu.data.ax, vcpuins.data.cimm, 16));
        vcpu.data.ax = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void PREFIX_SS() {
    _cb("PREFIX_SS");
    i386(0x36) {
        _adv;
        vcpuins.data.roverds = &vcpu.data.ss;
        vcpuins.data.roverss = &vcpu.data.ss;
    }
    else {
        vcpu.data.ip++;
        vcpuins.data.roverds = &vcpu.data.ss;
        vcpuins.data.roverss = &vcpu.data.ss;
    }
    _ce;
}
static void AAA() {
    _cb("AAA");
    i386(0x37) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    if (((vcpu.data.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
        vcpu.data.al += 0x06;
        vcpu.data.ah += 0x01;
        _SetEFLAGS_AF;
        _SetEFLAGS_CF;
    } else {
        _ClrEFLAGS_AF;
        _ClrEFLAGS_CF;
    }
    vcpu.data.al &= 0x0f;
    vcpuins.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF);
    _ce;
}
static void CMP_RM8_R8() {
    _cb("CMP_RM8_R8");
    i386(0x38) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_a_cmp(vcpuins.data.crm, vcpuins.data.cr, 8));
    _ce;
}
static void CMP_RM32_R32() {
    _cb("CMP_RM32_R32");
    i386(0x39) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_a_cmp(vcpuins.data.crm, vcpuins.data.cr, _GetOperandSize * 8));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(2, 2));
        _chr(_m_read_rm(2));
        _chr(_a_cmp(vcpuins.data.crm, vcpuins.data.cr, 16));
    }
    _ce;
}
static void CMP_R8_RM8() {
    _cb("CMP_R8_RM8");
    i386(0x3a) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_a_cmp(vcpuins.data.cr, vcpuins.data.crm, 8));
    _ce;
}
static void CMP_R32_RM32() {
    _cb("CMP_R32_RM32");
    i386(0x3b) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_a_cmp(vcpuins.data.cr, vcpuins.data.crm, _GetOperandSize * 8));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(2, 2));
        _chr(_m_read_rm(2));
        _chr(_a_cmp(vcpuins.data.cr, vcpuins.data.crm, 16));
    }
    _ce;
}
static void CMP_AL_I8() {
    _cb("CMP_AL_I8");
    i386(0x3c) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_a_cmp(vcpu.data.al, vcpuins.data.cimm, 8));
    _ce;
}
static void CMP_EAX_I32() {
    _cb("CMP_EAX_I32");
    i386(0x3d) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(2));
            _chr(_a_cmp(vcpu.data.ax, vcpuins.data.cimm, 16));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_d_imm(4));
            _chr(_a_cmp(vcpu.data.eax, vcpuins.data.cimm, 32));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        _chr(_a_cmp(vcpu.data.ax, vcpuins.data.cimm, 16));
    }
    _ce;
}
static void PREFIX_DS() {
    _cb("PREFIX_DS");
    i386(0x3e) {
        _adv;
        vcpuins.data.roverds = &vcpu.data.ds;
        vcpuins.data.roverss = &vcpu.data.ds;
    }
    else {
        vcpu.data.ip++;
        vcpuins.data.roverds = &vcpu.data.ds;
        vcpuins.data.roverss = &vcpu.data.ds;
    }
    _ce;
}
static void AAS() {
    _cb("AAS");
    i386(0x3f) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    if (((vcpu.data.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
        vcpu.data.al -= 0x06;
        vcpu.data.ah -= 0x01;
        _SetEFLAGS_AF;
        _SetEFLAGS_CF;
    } else {
        _ClrEFLAGS_CF;
        _ClrEFLAGS_AF;
    }
    vcpu.data.al &= 0x0f;
    vcpuins.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF);
    _ce;
}
static void INC_EAX() {
    _cb("INC_EAX");
    i386(0x40) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_inc(vcpu.data.ax, 16));
            vcpu.data.ax = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_inc(vcpu.data.eax, 32));
            vcpu.data.eax = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_a_inc(vcpu.data.ax, 16));
        vcpu.data.ax = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void INC_ECX() {
    _cb("INC_ECX");
    i386(0x41) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_inc(vcpu.data.cx, 16));
            vcpu.data.cx = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_inc(vcpu.data.ecx, 32));
            vcpu.data.ecx = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_a_inc(vcpu.data.cx, 16));
        vcpu.data.cx = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void INC_EDX() {
    _cb("INC_EDX");
    i386(0x42) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_inc(vcpu.data.dx, 16));
            vcpu.data.dx = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_inc(vcpu.data.edx, 32));
            vcpu.data.edx = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_a_inc(vcpu.data.dx, 16));
        vcpu.data.dx = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void INC_EBX() {
    _cb("INC_EBX");
    i386(0x43) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_inc(vcpu.data.bx, 16));
            vcpu.data.bx = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_inc(vcpu.data.ebx, 32));
            vcpu.data.ebx = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_a_inc(vcpu.data.bx, 16));
        vcpu.data.bx = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void INC_ESP() {
    _cb("INC_ESP");
    i386(0x44) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_inc(vcpu.data.sp, 16));
            vcpu.data.sp = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_inc(vcpu.data.esp, 32));
            vcpu.data.esp = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_a_inc(vcpu.data.sp, 16));
        vcpu.data.sp = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void INC_EBP() {
    _cb("INC_EBP");
    i386(0x45) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_inc(vcpu.data.bp, 16));
            vcpu.data.bp = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_inc(vcpu.data.ebp, 32));
            vcpu.data.ebp = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_a_inc(vcpu.data.bp, 16));
        vcpu.data.bp = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void INC_ESI() {
    _cb("INC_ESI");
    i386(0x46) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_inc(vcpu.data.si, 16));
            vcpu.data.si = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_inc(vcpu.data.esi, 32));
            vcpu.data.esi = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_a_inc(vcpu.data.si, 16));
        vcpu.data.si = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void INC_EDI() {
    _cb("INC_EDI");
    i386(0x47) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_inc(vcpu.data.di, 16));
            vcpu.data.di = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_inc(vcpu.data.edi, 32));
            vcpu.data.edi = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_a_inc(vcpu.data.di, 16));
        vcpu.data.di = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void DEC_EAX() {
    _cb("DEC_EAX");
    i386(0x48) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_dec(vcpu.data.ax, 16));
            vcpu.data.ax = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_dec(vcpu.data.eax, 32));
            vcpu.data.eax = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_a_dec(vcpu.data.ax, 16));
        vcpu.data.ax = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void DEC_ECX() {
    _cb("DEC_ECX");
    i386(0x49) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_dec(vcpu.data.cx, 16));
            vcpu.data.cx = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_dec(vcpu.data.ecx, 32));
            vcpu.data.ecx = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_a_dec(vcpu.data.cx, 16));
        vcpu.data.cx = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void DEC_EDX() {
    _cb("DEC_EDX");
    i386(0x4a) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_dec(vcpu.data.dx, 16));
            vcpu.data.dx = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_dec(vcpu.data.edx, 32));
            vcpu.data.edx = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_a_dec(vcpu.data.dx, 16));
        vcpu.data.dx = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void DEC_EBX() {
    _cb("DEC_EBX");
    i386(0x4b) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_dec(vcpu.data.bx, 16));
            vcpu.data.bx = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_dec(vcpu.data.ebx, 32));
            vcpu.data.ebx = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_a_dec(vcpu.data.bx, 16));
        vcpu.data.bx = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void DEC_ESP() {
    _cb("DEC_ESP");
    i386(0x4c) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_dec(vcpu.data.sp, 16));
            vcpu.data.sp = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_dec(vcpu.data.esp, 32));
            vcpu.data.esp = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_a_dec(vcpu.data.sp, 16));
        vcpu.data.sp = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void DEC_EBP() {
    _cb("DEC_EBP");
    i386(0x4d) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_dec(vcpu.data.bp, 16));
            vcpu.data.bp = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_dec(vcpu.data.ebp, 32));
            vcpu.data.ebp = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_a_dec(vcpu.data.bp, 16));
        vcpu.data.bp = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void DEC_ESI() {
    _cb("DEC_ESI");
    i386(0x4e) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_dec(vcpu.data.si, 16));
            vcpu.data.si = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_dec(vcpu.data.esi, 32));
            vcpu.data.esi = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_a_dec(vcpu.data.si, 16));
        vcpu.data.si = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void DEC_EDI() {
    _cb("DEC_EDI");
    i386(0x4f) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_dec(vcpu.data.di, 16));
            vcpu.data.di = GetMax16(vcpuins.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_dec(vcpu.data.edi, 32));
            vcpu.data.edi = GetMax32(vcpuins.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_a_dec(vcpu.data.di, 16));
        vcpu.data.di = GetMax16(vcpuins.data.result);
    }
    _ce;
}
static void PUSH_EAX() {
    _cb("PUSH_EAX");
    i386(0x50) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_push(GetRef(vcpu.data.ax), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_push(GetRef(vcpu.data.eax), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_e_push(GetRef(vcpu.data.ax), 2));
    }
    _ce;
}
static void PUSH_ECX() {
    _cb("PUSH_ECX");
    i386(0x51) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_push(GetRef(vcpu.data.cx), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_push(GetRef(vcpu.data.ecx), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_e_push(GetRef(vcpu.data.cx), 2));
    }
    _ce;
}
static void PUSH_EDX() {
    _cb("PUSH_EDX");
    i386(0x52) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_push(GetRef(vcpu.data.dx), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_push(GetRef(vcpu.data.edx), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_e_push(GetRef(vcpu.data.dx), 2));
    }
    _ce;
}
static void PUSH_EBX() {
    _cb("PUSH_EBX");
    i386(0x53) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_push(GetRef(vcpu.data.bx), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_push(GetRef(vcpu.data.ebx), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_e_push(GetRef(vcpu.data.bx), 2));
    }
    _ce;
}
static void PUSH_ESP() {
    _cb("PUSH_ESP");
    i386(0x54) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_push(GetRef(vcpu.data.sp), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_push(GetRef(vcpu.data.esp), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_e_push(GetRef(vcpu.data.sp), 2));
    }
    _ce;
}
static void PUSH_EBP() {
    _cb("PUSH_EBP");
    i386(0x55) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_push(GetRef(vcpu.data.bp), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_push(GetRef(vcpu.data.ebp), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_e_push(GetRef(vcpu.data.bp), 2));
    }
    _ce;
}
static void PUSH_ESI() {
    _cb("PUSH_ESI");
    i386(0x56) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_push(GetRef(vcpu.data.si), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_push(GetRef(vcpu.data.esi), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_e_push(GetRef(vcpu.data.si), 2));
    }
    _ce;
}
static void PUSH_EDI() {
    _cb("PUSH_EDI");
    i386(0x57) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_push(GetRef(vcpu.data.di), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_push(GetRef(vcpu.data.edi), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_e_push(GetRef(vcpu.data.di), 2));
    }
    _ce;
}
static void POP_EAX() {
    _cb("POP_EAX");
    i386(0x58) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(GetRef(vcpu.data.ax), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(GetRef(vcpu.data.eax), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_e_pop(GetRef(vcpu.data.ax), 2));
    }
    _ce;
}
static void POP_ECX() {
    _cb("POP_ECX");
    i386(0x59) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(GetRef(vcpu.data.cx), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(GetRef(vcpu.data.ecx), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_e_pop(GetRef(vcpu.data.cx), 2));
    }
    _ce;
}
static void POP_EDX() {
    _cb("POP_EDX");
    i386(0x5a) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(GetRef(vcpu.data.dx), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(GetRef(vcpu.data.edx), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_e_pop(GetRef(vcpu.data.dx), 2));
    }
    _ce;
}
static void POP_EBX() {
    _cb("POP_EBX");
    i386(0x5b) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(GetRef(vcpu.data.bx), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(GetRef(vcpu.data.ebx), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_e_pop(GetRef(vcpu.data.bx), 2));
    }
    _ce;
}
static void POP_ESP() {
    _cb("POP_ESP");
    i386(0x5c) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(GetRef(vcpu.data.sp), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(GetRef(vcpu.data.esp), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_e_pop(GetRef(vcpu.data.sp), 2));
    }
    _ce;
}
static void POP_EBP() {
    _cb("POP_EBP");
    i386(0x5d) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(GetRef(vcpu.data.bp), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(GetRef(vcpu.data.ebp), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_e_pop(GetRef(vcpu.data.bp), 2));
    }
    _ce;
}
static void POP_ESI() {
    _cb("POP_ESI");
    i386(0x5e) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(GetRef(vcpu.data.si), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(GetRef(vcpu.data.esi), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_e_pop(GetRef(vcpu.data.si), 2));
    }
    _ce;
}
static void POP_EDI() {
    _cb("POP_EDI");
    i386(0x5f) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(GetRef(vcpu.data.di), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(GetRef(vcpu.data.edi), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_e_pop(GetRef(vcpu.data.di), 2));
    }
    _ce;
}
static void PUSHA() {
    t_nubit32 cesp;
    _cb("PUSHA");
    i386(0x60) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            cesp = vcpu.data.sp;
            _chr(_e_push(GetRef(vcpu.data.ax), 2));
            _chr(_e_push(GetRef(vcpu.data.cx), 2));
            _chr(_e_push(GetRef(vcpu.data.dx), 2));
            _chr(_e_push(GetRef(vcpu.data.bx), 2));
            _chr(_e_push(GetRef(cesp),    2));
            _chr(_e_push(GetRef(vcpu.data.bp), 2));
            _chr(_e_push(GetRef(vcpu.data.si), 2));
            _chr(_e_push(GetRef(vcpu.data.di), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            cesp = vcpu.data.esp;
            _chr(_e_push(GetRef(vcpu.data.eax), 4));
            _chr(_e_push(GetRef(vcpu.data.ecx), 4));
            _chr(_e_push(GetRef(vcpu.data.edx), 4));
            _chr(_e_push(GetRef(vcpu.data.ebx), 4));
            _chr(_e_push(GetRef(cesp),     4));
            _chr(_e_push(GetRef(vcpu.data.ebp), 4));
            _chr(_e_push(GetRef(vcpu.data.esi), 4));
            _chr(_e_push(GetRef(vcpu.data.edi), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else
        UndefinedOpcode();
    _ce;
}
static void POPA() {
    t_nubit32 cesp;
    _cb("POPA");
    i386(0x61) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(GetRef(vcpu.data.di), 2));
            _chr(_e_pop(GetRef(vcpu.data.si), 2));
            _chr(_e_pop(GetRef(vcpu.data.bp), 2));
            _chr(_e_pop(GetRef(cesp),    2));
            _chr(_e_pop(GetRef(vcpu.data.bx), 2));
            _chr(_e_pop(GetRef(vcpu.data.dx), 2));
            _chr(_e_pop(GetRef(vcpu.data.cx), 2));
            _chr(_e_pop(GetRef(vcpu.data.ax), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(GetRef(vcpu.data.edi), 4));
            _chr(_e_pop(GetRef(vcpu.data.esi), 4));
            _chr(_e_pop(GetRef(vcpu.data.ebp), 4));
            _chr(_e_pop(GetRef(cesp),     4));
            _chr(_e_pop(GetRef(vcpu.data.ebx), 4));
            _chr(_e_pop(GetRef(vcpu.data.edx), 4));
            _chr(_e_pop(GetRef(vcpu.data.ecx), 4));
            _chr(_e_pop(GetRef(vcpu.data.eax), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else
        UndefinedOpcode();
    _ce;
}
static void BOUND_R16_M16_16() {
    t_nsbit16 a16,l16,u16;
    t_nsbit32 a32,l32,u32;
    _cb("BOUND_R16_M16_16");
    i386(0x62) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize * 2));
        if (!vcpuins.data.flagMem) {
            _bb("flagMem(0)");
            _chr(UndefinedOpcode());
            _be;
        }
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            a16 = (t_nsbit16)vcpuins.data.cr;
            _chr(_m_read_rm(2));
            l16 = (t_nsbit16)vcpuins.data.crm;
            vcpuins.data.mrm.offset += 2;
            _chr(_m_read_rm(2));
            u16 = (t_nsbit16)vcpuins.data.crm;
            if (a16 < l16 || a16 > u16)
                _chr(_SetExcept_BR(0));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            a32 = (t_nsbit32)vcpuins.data.cr;
            _chr(_m_read_rm(4));
            l32 = (t_nsbit32)vcpuins.data.crm;
            vcpuins.data.mrm.offset += 2;
            _chr(_m_read_rm(4));
            u32 = (t_nsbit32)vcpuins.data.crm;
            if (a32 < l32 || a32 > u32)
                _chr(_SetExcept_BR(0));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else
        UndefinedOpcode();
    _ce;
}
static void ARPL_RM16_R16() {
    _cb("ARPL_RM16_R16");
    i386(0x63) {
        if (_IsProtected) {
            _adv;
            _chr(_d_modrm(2, 2));
            _chr(_m_read_rm(2));
            vcpuins.data.cr = GetMax16(vcpuins.data.cr);
            if (_GetSelector_RPL(vcpuins.data.crm) < _GetSelector_RPL(vcpuins.data.cr)) {
                _SetEFLAGS_ZF;
                vcpuins.data.result = GetMax16((vcpuins.data.crm & ~VCPU_SELECTOR_RPL) |
                                               (vcpuins.data.cr & VCPU_SELECTOR_RPL));
                vcpuins.data.crm = vcpuins.data.result;
                _chr(_m_write_rm(2));
            } else
                _ClrEFLAGS_ZF;
        } else
            _chr(UndefinedOpcode());
    }
    else
        UndefinedOpcode();
    _ce;
}
static void PREFIX_FS() {
    _cb("PREFIX_FS");
    i386(0x64) {
        _adv;
        vcpuins.data.roverds = &vcpu.data.fs;
        vcpuins.data.roverss = &vcpu.data.fs;
    }
    else
        UndefinedOpcode();
    _ce;
}
static void PREFIX_GS() {
    _cb("PREFIX_GS");
    i386(0x65) {
        _adv;
        vcpuins.data.roverds = &vcpu.data.gs;
        vcpuins.data.roverss = &vcpu.data.gs;
    }
    else
        UndefinedOpcode();
    _ce;
}
static void PREFIX_OprSize() {
    _cb("PREFIX_OprSize");
    i386(0x66) {
        _adv;
        vcpuins.data.prefix_oprsize = True;
    }
    else
        UndefinedOpcode();
    _ce;
}
static void PREFIX_AddrSize() {
    _cb("PREFIX_AddrSize");
    i386(0x67) {
        _adv;
        vcpuins.data.prefix_addrsize = True;
    }
    else
        UndefinedOpcode();
    _ce;
}
static void PUSH_I32() {
    _cb("PUSH_I32");
    i386(0x68) {
        _adv;
        _chr(_d_imm(_GetOperandSize));
        _chr(_e_push(GetRef(vcpuins.data.cimm), _GetOperandSize));
    }
    else
        UndefinedOpcode();
    _ce;
}
static void IMUL_R32_RM32_I32() {
    _cb("IMUL_R32_RM32_I32");
    i386(0x69) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_d_imm(_GetOperandSize));
        _chr(_a_imul3(vcpuins.data.crm, vcpuins.data.cimm, _GetOperandSize * 8));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), _GetOperandSize));
    }
    else
        UndefinedOpcode();
    _ce;
}
static void PUSH_I8() {
    _cb("PUSH_I8");
    i386(0x6a) {
        _adv;
        _chr(_d_imm(1));
        _chr(_e_push(GetRef(vcpuins.data.cimm), _GetOperandSize));
    }
    else
        UndefinedOpcode();
    _ce;
}
static void IMUL_R32_RM32_I8() {
    _cb("IMUL_R32_RM32_I8");
    i386(0x6b) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_d_imm(1));
        _chr(_a_imul3(vcpuins.data.crm, vcpuins.data.cimm, ((_GetOperandSize * 8 + 8) >> 1)));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), _GetOperandSize));
    }
    else
        UndefinedOpcode();
    _ce;
}
static void INSB() {
    _cb("INSB");
    i386(0x6c) {
        _adv;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_p_ins(1));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (vcpu.data.cx) {
                    _bb("cx(!0)");
                    _chr(_p_ins(1));
                    vcpu.data.cx--;
                    _be;
                }
                if (vcpu.data.cx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (vcpu.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_p_ins(1));
                    vcpu.data.ecx--;
                    _be;
                }
                if (vcpu.data.ecx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
        }
    }
    else
        UndefinedOpcode();
    _ce;
}
static void INSW() {
    _cb("INSW");
    i386(0x6d) {
        _adv;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_p_ins(_GetOperandSize));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (vcpu.data.cx) {
                    _bb("cx(!0)");
                    _chr(_p_ins(_GetOperandSize));
                    vcpu.data.cx--;
                    _be;
                }
                if (vcpu.data.cx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (vcpu.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_p_ins(_GetOperandSize));
                    vcpu.data.ecx--;
                    _be;
                }
                if (vcpu.data.ecx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
        }
    }
    else
        UndefinedOpcode();
    _ce;
}
static void OUTSB() {
    _cb("OUTSB");
    i386(0x6e) {
        _adv;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_p_ins(1));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (vcpu.data.cx) {
                    _bb("cx(!0)");
                    _chr(_p_ins(1));
                    vcpu.data.cx--;
                    _be;
                }
                if (vcpu.data.cx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (vcpu.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_p_ins(1));
                    vcpu.data.ecx--;
                    _be;
                }
                if (vcpu.data.ecx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
        }
    }
    else
        UndefinedOpcode();
    _ce;
}
static void OUTSW() {
    _cb("OUTSW");
    i386(0x6f) {
        _adv;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_p_outs(_GetOperandSize));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (vcpu.data.cx) {
                    _bb("cx(!0)");
                    _chr(_p_outs(_GetOperandSize));
                    vcpu.data.cx--;
                    _be;
                }
                if (vcpu.data.cx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (vcpu.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_p_outs(_GetOperandSize));
                    vcpu.data.ecx--;
                    _be;
                }
                if (vcpu.data.ecx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
        }
    }
    else
        UndefinedOpcode();
    _ce;
}
static void JO_REL8() {
    _cb("JO_REL8");
    i386(0x70) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), 1, _GetEFLAGS_OF));
    _ce;
}
static void JNO_REL8() {
    _cb("JNO_REL8");
    i386(0x71) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), 1, !_GetEFLAGS_OF));
    _ce;
}
static void JC_REL8() {
    _cb("JC_REL8");
    i386(0x72) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), 1, _GetEFLAGS_CF));
    _ce;
}
static void JNC_REL8() {
    _cb("JNC_REL8");
    i386(0x73) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), 1, !_GetEFLAGS_CF));
    _ce;
}
static void JZ_REL8() {
    _cb("JZ_REL8");
    i386(0x74) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), 1, _GetEFLAGS_ZF));
    _ce;
}
static void JNZ_REL8() {
    _cb("JNZ_REL8");
    i386(0x75) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), 1, !_GetEFLAGS_ZF));
    _ce;
}
static void JNA_REL8() {
    _cb("JNA_REL8");
    i386(0x76) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), 1,
                (_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    _ce;
}
static void JA_REL8() {
    _cb("JA_REL8");
    i386(0x77) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), 1,
                !(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    _ce;
}
static void JS_REL8() {
    _cb("JS_REL8");
    i386(0x78) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), 1, _GetEFLAGS_SF));
    _ce;
}
static void JNS_REL8() {
    _cb("JNS_REL8");
    i386(0x79) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), 1, !_GetEFLAGS_SF));
    _ce;
}
static void JP_REL8() {
    _cb("JP_REL8");
    _new_code_path_;
    i386(0x7a) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), 1, _GetEFLAGS_PF));
    _ce;
}
static void JNP_REL8() {
    _cb("JNP_REL8");
    _new_code_path_;
    i386(0x7b) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), 1, !_GetEFLAGS_PF));
    _ce;
}
static void JL_REL8() {
    _cb("JL_REL8");
    i386(0x7c) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), 1, (_GetEFLAGS_SF != _GetEFLAGS_OF)));
    _ce;
}
static void JNL_REL8() {
    _cb("JNL_REL8");
    i386(0x7d) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), 1, (_GetEFLAGS_SF == _GetEFLAGS_OF)));
    _ce;
}
static void JNG_REL8() {
    _cb("JNG_REL8");
    i386(0x7e) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), 1,
                (_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF))));
    _ce;
}
static void JG_REL8() {
    _cb("JG_REL8");
    i386(0x7f) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), 1,
                (!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF))));
    _ce;
}
static void INS_80() {
    _cb("INS_80");
    i386 (0x80) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(0, 1));
    _chr(_d_imm(1));
    _chr(_m_read_rm(1));
    switch (vcpuins.data.cr) {
    case 0: /* ADD_RM8_I8 */
        _bb("ADD_RM8_I8");
        _chr(_a_add(vcpuins.data.crm, vcpuins.data.cimm, 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(1));
        _be;
        break;
    case 1: /* OR_RM8_I8 */
        _bb("OR_RM8_I8");
        _chr(_a_or(vcpuins.data.crm, vcpuins.data.cimm, 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(1));
        _be;
        break;
    case 2: /* ADC_RM8_I8 */
        _bb("ADC_RM8_I8");
        _chr(_a_adc(vcpuins.data.crm, vcpuins.data.cimm, 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(1));
        _be;
        break;
    case 3: /* SBB_RM8_I8 */
        _bb("SBB_RM8_I8");
        _new_code_path_;
        _chr(_a_sbb(vcpuins.data.crm, vcpuins.data.cimm, 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(1));
        _be;
        break;
    case 4: /* AND_RM8_I8 */
        _bb("AND_RM8_I8");
        _chr(_a_and(vcpuins.data.crm, vcpuins.data.cimm, 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(1));
        _be;
        break;
    case 5: /* SUB_RM8_I8 */
        _bb("SUB_RM8_I8");
        _chr(_a_sub(vcpuins.data.crm, vcpuins.data.cimm, 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(1));
        _be;
        break;
    case 6: /* XOR_RM8_I8 */
        _bb("XOR_RM8_I8");
        _chr(_a_xor(vcpuins.data.crm, vcpuins.data.cimm, 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(1));
        _be;
        break;
    case 7: /* CMP_RM8_I8 */
        _bb("CMP_RM8_I8");
        _chr(_a_cmp(vcpuins.data.crm, vcpuins.data.cimm, 8));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void INS_81() {
    _cb("INS_81");
    i386(0x81) {
        _adv;
        _chr(_d_modrm(0, _GetOperandSize));
        _chr(_d_imm(_GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        switch (vcpuins.data.cr) {
        case 0: /* ADD_RM32_I32 */
            _bb("ADD_RM32_I32");
            _chr(_a_add(vcpuins.data.crm, vcpuins.data.cimm, _GetOperandSize * 8));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 1: /* OR_RM32_I32 */
            _bb("OR_RM32_I32");
            _chr(_a_or(vcpuins.data.crm, vcpuins.data.cimm, _GetOperandSize * 8));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 2: /* ADC_RM32_I32 */
            _bb("ADC_RM32_I32");
            _chr(_a_adc(vcpuins.data.crm, vcpuins.data.cimm, _GetOperandSize * 8));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 3: /* SBB_RM32_I32 */
            _bb("SBB_RM32_I32");
            _new_code_path_;
            _chr(_a_sbb(vcpuins.data.crm, vcpuins.data.cimm, _GetOperandSize * 8));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 4: /* AND_RM32_I32 */
            _bb("AND_RM32_I32");
            _chr(_a_and(vcpuins.data.crm, vcpuins.data.cimm, _GetOperandSize * 8));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 5: /* SUB_RM32_I32 */
            _bb("SUB_RM32_I32");
            _chr(_a_sub(vcpuins.data.crm, vcpuins.data.cimm, _GetOperandSize * 8));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 6: /* XOR_RM32_I32 */
            _bb("XOR_RM32_I32");
            _chr(_a_xor(vcpuins.data.crm, vcpuins.data.cimm, _GetOperandSize * 8));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 7: /* CMP_RM32_I32 */
            _bb("CMP_RM32_I32");
            _chr(_a_cmp(vcpuins.data.crm, vcpuins.data.cimm, _GetOperandSize * 8));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(0, 2));
        _chr(_d_imm(2));
        _chr(_m_read_rm(2));
        switch (vcpuins.data.cr) {
        case 0: /* ADD_RM16_I16 */
            _bb("ADD_RM16_I16");
            _chr(_a_add(vcpuins.data.crm, vcpuins.data.cimm, 2 * 8));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 1: /* OR_RM16_I16 */
            _bb("OR_RM16_I16");
            _chr(_a_or(vcpuins.data.crm, vcpuins.data.cimm, 2 * 8));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 2: /* ADC_RM16_I16 */
            _bb("ADC_RM16_I16");
            _chr(_a_adc(vcpuins.data.crm, vcpuins.data.cimm, 2 * 8));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 3: /* SBB_RM16_I16 */
            _bb("SBB_RM16_I16");
            _new_code_path_;
            _chr(_a_sbb(vcpuins.data.crm, vcpuins.data.cimm, 2 * 8));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 4: /* AND_RM16_I16 */
            _bb("AND_RM16_I16");
            _chr(_a_and(vcpuins.data.crm, vcpuins.data.cimm, 2 * 8));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 5: /* SUB_RM16_I16 */
            _bb("SUB_RM16_I16");
            _chr(_a_sub(vcpuins.data.crm, vcpuins.data.cimm, 2 * 8));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 6: /* XOR_RM16_I16 */
            _bb("XOR_RM16_I16");
            _chr(_a_xor(vcpuins.data.crm, vcpuins.data.cimm, 2 * 8));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 7: /* CMP_RM16_I16 */
            _bb("CMP_RM16_I16");
            _chr(_a_cmp(vcpuins.data.crm, vcpuins.data.cimm, 2 * 8));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    _ce;
}
static void INS_83() {
    t_nubit8 bit;
    _cb("INS_83");
    i386(0x83) {
        _adv;
        _chr(_d_modrm(0, _GetOperandSize));
        _chr(_d_imm(1));
        _chr(_m_read_rm(_GetOperandSize));
        bit = (_GetOperandSize * 8 + 8) >> 1;
        switch (vcpuins.data.cr) {
        case 0: /* ADD_RM32_I8 */
            _bb("ADD_RM32_I8");
            _chr(_a_add(vcpuins.data.crm, vcpuins.data.cimm, bit));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 1: /* OR_RM32_I8 */
            _bb("OR_RM32_I8");
            _chr(_a_or(vcpuins.data.crm, vcpuins.data.cimm, bit));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 2: /* ADC_RM32_I8 */
            _bb("ADC_RM32_I8");
            _chr(_a_adc(vcpuins.data.crm, vcpuins.data.cimm, bit));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 3: /* SBB_RM32_I8 */
            _bb("SBB_RM32_I8");
            _chr(_a_sbb(vcpuins.data.crm, vcpuins.data.cimm, bit));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 4: /* AND_RM32_I8 */
            _bb("AND_RM32_I8");
            _chr(_a_and(vcpuins.data.crm, vcpuins.data.cimm, bit));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 5: /* SUB_RM32_I8 */
            _bb("SUB_RM32_I8");
            _chr(_a_sub(vcpuins.data.crm, vcpuins.data.cimm, bit));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 6: /* XOR_RM32_I8 */
            _bb("XOR_RM32_I8");
            _chr(_a_xor(vcpuins.data.crm, vcpuins.data.cimm, bit));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 7: /* CMP_RM32_I8 */
            _bb("CMP_RM32_I8");
            _chr(_a_cmp(vcpuins.data.crm, vcpuins.data.cimm, bit));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(0, 2));
        _chr(_d_imm(1));
        _chr(_m_read_rm(2));
        bit = 12;
        switch (vcpuins.data.cr) {
        case 0: /* ADD_RM16_I8 */
            _bb("ADD_RM16_I8");
            _chr(_a_add(vcpuins.data.crm, vcpuins.data.cimm, bit));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 1: /* OR_RM16_I8 */
            _bb("OR_RM16_I8");
            _chr(_a_or(vcpuins.data.crm, vcpuins.data.cimm, bit));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 2: /* ADC_RM16_I8 */
            _bb("ADC_RM16_I8");
            _chr(_a_adc(vcpuins.data.crm, vcpuins.data.cimm, bit));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 3: /* SBB_RM16_I8 */
            _bb("SBB_RM16_I8");
            _chr(_a_sbb(vcpuins.data.crm, vcpuins.data.cimm, bit));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 4: /* AND_RM16_I8 */
            _bb("AND_RM16_I8");
            _chr(_a_and(vcpuins.data.crm, vcpuins.data.cimm, bit));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 5: /* SUB_RM16_I8 */
            _bb("SUB_RM16_I8");
            _chr(_a_sub(vcpuins.data.crm, vcpuins.data.cimm, bit));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 6: /* XOR_RM16_I8 */
            _bb("XOR_RM16_I8");
            _chr(_a_xor(vcpuins.data.crm, vcpuins.data.cimm, bit));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 7: /* CMP_RM16_I8 */
            _bb("CMP_RM16_I8");
            _chr(_a_cmp(vcpuins.data.crm, vcpuins.data.cimm, bit));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    _ce;
}
static void TEST_RM8_R8() {
    _cb("TEST_RM8_R8");
    i386(0x84) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_a_test(vcpuins.data.crm, vcpuins.data.cr, 8));
    _ce;
}
static void TEST_RM32_R32() {
    _cb("TEST_RM32_R32");
    i386(0x85) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_a_test(vcpuins.data.crm, vcpuins.data.cr, _GetOperandSize * 8));
    }
    else {
        vcpu.data.ip++;
        _d_modrm(2, 2);
        _chr(_m_read_rm(2));
        _a_test(vcpuins.data.crm, vcpuins.data.cr, 16);
    }
    _ce;
}
static void XCHG_RM8_R8() {
    _cb("XCHG_RM8_R8");
    i386(0x86) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.crm), 1));
    vcpuins.data.crm = vcpuins.data.cr;
    _chr(_m_write_rm(1));
    _ce;
}
static void XCHG_RM32_R32() {
    _cb("XCHG_RM32_R32");
    i386(0x87) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.crm), _GetOperandSize));
        vcpuins.data.crm = vcpuins.data.cr;
        _chr(_m_write_rm(_GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _d_modrm(2, 2);
        _chr(_m_read_rm(2));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.crm), 2));
        vcpuins.data.crm = vcpuins.data.cr;
        _chr(_m_write_rm(2));
    }
    _ce;
}
static void MOV_RM8_R8() {
    _cb("MOV_RM8_R8");
    i386(0x88) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    vcpuins.data.crm = vcpuins.data.cr;
    _chr(_m_write_rm(1));
    _ce;
}
static void MOV_RM32_R32() {
    _cb("MOV_RM32_R32");
    i386(0x89) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        vcpuins.data.crm = vcpuins.data.cr;
        _chr(_m_write_rm(_GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _d_modrm(2, 2);
        vcpuins.data.crm = vcpuins.data.cr;
        _chr(_m_write_rm(2));
    }
    _ce;
}
static void MOV_R8_RM8() {
    _cb("MOV_R8_RM8");
    i386(0x8a) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(1, 1));
    _chr(_m_read_rm(1));
    _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.crm), 1));
    _ce;
}
static void MOV_R32_RM32() {
    _cb("MOV_R32_RM32");
    i386(0x8b) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.crm), _GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _d_modrm(2, 2);
        _chr(_m_read_rm(2));
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.crm), 2));
    }
    _ce;
}
static void MOV_RM16_SREG() {
    _cb("MOV_RM16_SREG");
    i386(0x8c) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm_sreg(2));
    vcpuins.data.crm = vcpuins.data.rmovsreg->selector;
    _chr(_m_write_rm(2));
    _ce;
}
static void LEA_R32_M32() {
    _cb("LEA_R32_M32");
    i386(0x8d) {
        _adv;
        _chr(_d_modrm_ea(_GetOperandSize, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.mrm.offset), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.mrm.offset), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _d_modrm_ea(2, 2);
        _m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.mrm.offset), 2);
    }
    _ce;
}
static void MOV_SREG_RM16() {
    _cb("MOV_SREG_RM16");
    i386(0x8e) {
        _adv;
        _chr(_d_modrm_sreg(2));
        if (vcpuins.data.rmovsreg->sregtype == SREG_CODE) {
            _bb("sregtype(SREG_CODE)");
            _chr(_SetExcept_UD(0));
            _be;
        }
        _chr(_m_read_rm(2));
        _chr(_s_load_sreg(vcpuins.data.rmovsreg, GetMax16(vcpuins.data.crm)));
        if (vcpuins.data.rmovsreg->sregtype == SREG_STACK)
            vcpuins.data.flagMaskInt = True;
    }
    else {
        vcpu.data.ip++;
        _d_modrm_sreg(2);
        _chr(_m_read_rm(2));
        _chr(_s_load_sreg(vcpuins.data.rmovsreg, GetMax16(vcpuins.data.crm)));
    }
    _ce;
}
static void INS_8F() {
    _cb("INS_8F");
    i386(0x8f) {
        _adv;
        _chr(_d_modrm(0, _GetOperandSize));
        switch (vcpuins.data.cr) {
        case 0: /* POP_RM32 */
            _bb("POP_RM32");
            _chr(_e_pop(GetRef(vcpuins.data.crm), _GetOperandSize));
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 1:
            _bb("vcpuins.data.cr(1)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 2:
            _bb("vcpuins.data.cr(2)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 3:
            _bb("vcpuins.data.cr(3)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 4:
            _bb("vcpuins.data.cr(4)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 5:
            _bb("vcpuins.data.cr(5)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 6:
            _bb("vcpuins.data.cr(6)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 7:
            _bb("vcpuins.data.cr(7)");
            _chr(UndefinedOpcode());
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(0, 2));
        switch (vcpuins.data.cr) {
        case 0: /* POP_RM16 */
            _bb("POP_RM16");
            _chr(_e_pop(GetRef(vcpuins.data.crm), 2));
            _chr(_m_write_rm(2));
            _be;
            break;
        case 1:
            _bb("vcpuins.data.cr(1)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 2:
            _bb("vcpuins.data.cr(2)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 3:
            _bb("vcpuins.data.cr(3)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 4:
            _bb("vcpuins.data.cr(4)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 5:
            _bb("vcpuins.data.cr(5)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 6:
            _bb("vcpuins.data.cr(6)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 7:
            _bb("vcpuins.data.cr(7)");
            _chr(UndefinedOpcode());
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    _ce;
}
static void NOP() {
    _cb("NOP");
    i386(0x90) {
        _adv;
    }
    else
        vcpu.data.ip++;
    _ce;
}
static void XCHG_ECX_EAX() {
    _cb("XCHG_ECX_EAX");
    i386(0x91) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            vcpuins.data.cr = vcpu.data.ax;
            vcpu.data.ax = vcpu.data.cx;
            vcpu.data.cx = GetMax16(vcpuins.data.cr);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            vcpuins.data.cr = vcpu.data.eax;
            vcpu.data.eax = vcpu.data.ecx;
            vcpu.data.ecx = GetMax32(vcpuins.data.cr);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        vcpuins.data.cr = vcpu.data.ax;
        vcpu.data.ax = vcpu.data.cx;
        vcpu.data.cx = GetMax16(vcpuins.data.cr);
    }
    _ce;
}
static void XCHG_EDX_EAX() {
    _cb("XCHG_EDX_EAX");
    i386(0x92) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            vcpuins.data.cr = vcpu.data.ax;
            vcpu.data.ax = vcpu.data.dx;
            vcpu.data.dx = GetMax16(vcpuins.data.cr);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            vcpuins.data.cr = vcpu.data.eax;
            vcpu.data.eax = vcpu.data.edx;
            vcpu.data.edx = GetMax32(vcpuins.data.cr);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        vcpuins.data.cr = vcpu.data.ax;
        vcpu.data.ax = vcpu.data.dx;
        vcpu.data.dx = GetMax16(vcpuins.data.cr);
    }
    _ce;
}
static void XCHG_EBX_EAX() {
    _cb("XCHG_EBX_EAX");
    i386(0x93) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            vcpuins.data.cr = vcpu.data.ax;
            vcpu.data.ax = vcpu.data.bx;
            vcpu.data.bx = GetMax16(vcpuins.data.cr);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            vcpuins.data.cr = vcpu.data.eax;
            vcpu.data.eax = vcpu.data.ebx;
            vcpu.data.ebx = GetMax32(vcpuins.data.cr);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        vcpuins.data.cr = vcpu.data.ax;
        vcpu.data.ax = vcpu.data.bx;
        vcpu.data.bx = GetMax16(vcpuins.data.cr);
    }
    _ce;
}
static void XCHG_ESP_EAX() {
    _cb("XCHG_ESP_EAX");
    _new_code_path_;
    i386(0x94) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            vcpuins.data.cr = vcpu.data.ax;
            vcpu.data.ax = vcpu.data.sp;
            vcpu.data.sp = GetMax16(vcpuins.data.cr);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            vcpuins.data.cr = vcpu.data.eax;
            vcpu.data.eax = vcpu.data.esp;
            vcpu.data.esp = GetMax32(vcpuins.data.cr);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        vcpuins.data.cr = vcpu.data.ax;
        vcpu.data.ax = vcpu.data.sp;
        vcpu.data.sp = GetMax16(vcpuins.data.cr);
    }
    _ce;
}
static void XCHG_EBP_EAX() {
    _cb("XCHG_EBP_EAX");
    i386(0x95) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            vcpuins.data.cr = vcpu.data.ax;
            vcpu.data.ax = vcpu.data.bp;
            vcpu.data.bp = GetMax16(vcpuins.data.cr);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            vcpuins.data.cr = vcpu.data.eax;
            vcpu.data.eax = vcpu.data.ebp;
            vcpu.data.ebp = GetMax32(vcpuins.data.cr);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        vcpuins.data.cr = vcpu.data.ax;
        vcpu.data.ax = vcpu.data.bp;
        vcpu.data.bp = GetMax16(vcpuins.data.cr);
    }
    _ce;
}
static void XCHG_ESI_EAX() {
    _cb("XCHG_ESI_EAX");
    i386(0x96) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            vcpuins.data.cr = vcpu.data.ax;
            vcpu.data.ax = vcpu.data.si;
            vcpu.data.si = GetMax16(vcpuins.data.cr);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            vcpuins.data.cr = vcpu.data.eax;
            vcpu.data.eax = vcpu.data.esi;
            vcpu.data.esi = GetMax32(vcpuins.data.cr);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        vcpuins.data.cr = vcpu.data.ax;
        vcpu.data.ax = vcpu.data.si;
        vcpu.data.si = GetMax16(vcpuins.data.cr);
    }
    _ce;
}
static void XCHG_EDI_EAX() {
    _cb("XCHG_EDI_EAX");
    i386(0x97) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            vcpuins.data.cr = vcpu.data.ax;
            vcpu.data.ax = vcpu.data.di;
            vcpu.data.di = GetMax16(vcpuins.data.cr);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            vcpuins.data.cr = vcpu.data.eax;
            vcpu.data.eax = vcpu.data.edi;
            vcpu.data.edi = GetMax32(vcpuins.data.cr);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        vcpuins.data.cr = vcpu.data.ax;
        vcpu.data.ax = vcpu.data.di;
        vcpu.data.di = GetMax16(vcpuins.data.cr);
    }
    _ce;
}
static void CBW() {
    _cb("CBW");
    i386(0x98) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            vcpu.data.ax = (t_nsbit8)vcpu.data.al;
            break;
        case 4:
            vcpu.data.eax = (t_nsbit16)vcpu.data.ax;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        vcpu.data.ax = (t_nsbit8)vcpu.data.al;
    }
    _ce;
}
static void CWD() {
    _cb("CWD");
    i386(0x99) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            vcpu.data.dx = GetMSB16(vcpu.data.ax) ? 0xffff : 0x0000;
            break;
        case 4:
            vcpu.data.edx = GetMSB32(vcpu.data.eax) ? 0xffffffff : 0x00000000;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        if (vcpu.data.ax & 0x8000) vcpu.data.dx = 0xffff;
        else vcpu.data.dx = 0x0000;
    }
    _ce;
}
static void CALL_PTR16_32() {
    t_nubit16 newcs;
    t_nubit32 neweip;
    _cb("CALL_PTR16_32");
    i386(0x9a) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(4));
            neweip = GetMax16(vcpuins.data.cimm);
            newcs = GetMax16(vcpuins.data.cimm >> 16);
            _chr(_e_call_far(newcs, neweip, 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_d_imm(8));
            neweip = GetMax32(vcpuins.data.cimm);
            newcs = GetMax16(vcpuins.data.cimm >> 32);
            _chr(_e_call_far(newcs, neweip, 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _d_imm(4);
        neweip = GetMax16(vcpuins.data.cimm);
        newcs = GetMax16(vcpuins.data.cimm >> 16);
        _e_call_far(newcs, neweip, 2);
    }
    _ce;
}
_______todo WAIT() {
    /* not implemented */
    _cb("WAIT");
    _new_code_path_;
    i386(0x9b) {
        _adv;
        if (_GetCR0_TS) {
            _bb("CR0_TS(1)");
            _chr(_SetExcept_NM(0));
            _be;
        }
    }
    else {
        vcpu.data.ip++;
    }
    _ce;
}
static void PUSHF() {
    t_nubit32 ceflags;
    _cb("PUSHF");
    i386(0x9c) {
        _adv;
        if (!_GetCR0_PE || (_GetCR0_PE && (!_GetEFLAGS_VM || (_GetEFLAGS_VM && (_GetEFLAGS_IOPL == 3))))) {
            _bb("Real/Protected/(V86,IOPL(3))");
            switch (_GetOperandSize) {
            case 2:
                _bb("OperandSize(2)");
                ceflags = vcpu.data.flags;
                _chr(_e_push(GetRef(ceflags), 2));
                _be;
                break;
            case 4:
                _bb("OperandSize(4)");
                ceflags = vcpu.data.eflags & ~(VCPU_EFLAGS_VM | VCPU_EFLAGS_RF);
                _chr(_e_push(GetRef(ceflags), 4));
                _be;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
        } else {
            _bb("EFLAGS_VM(1),EFLAGS_IOPL(!3)");
            _chr(_SetExcept_GP(0));
            _be;
        }
    }
    else {
        vcpu.data.ip++;
        _e_push(GetRef(vcpu.data.flags), 2);
    }
    _ce;
}
static void POPF() {
    t_nubit32 mask = VCPU_EFLAGS_RESERVED;
    t_nubit32 ceflags = Zero32;
    _cb("POPF");
    i386(0x9d) {
        _adv;
        if (!_GetCR0_PE || !_GetEFLAGS_VM) {
            _bb("!V86");
            if (!_GetCPL) {
                _bb("CPL(0)");
                switch (_GetOperandSize) {
                case 2:
                    _bb("OperandSize(2)");
                    _chr(_e_pop(GetRef(ceflags), 2));
                    mask |= 0xffff0000;
                    _be;
                    break;
                case 4:
                    _bb("OperandSize(4)");
                    _chr(_e_pop(GetRef(ceflags), 4));
                    mask |= VCPU_EFLAGS_VM;
                    _be;
                    break;
                default:
                    _impossible_r_;
                    break;
                }
                _be;
            } else {
                _bb("CPL(!0)");
                _new_code_path_;
                switch (_GetOperandSize) {
                case 2:
                    _bb("OperandSize(2)");
                    _chr(_e_pop(GetRef(ceflags), 2));
                    mask |= (0xffff0000 | VCPU_EFLAGS_IOPL);
                    _be;
                    break;
                case 4:
                    _bb("OperandSize(4)");
                    _chr(_e_pop(GetRef(ceflags), 4));
                    mask |= (VCPU_EFLAGS_VM | VCPU_EFLAGS_RF | VCPU_EFLAGS_IOPL);
                    _be;
                    break;
                default:
                    _impossible_r_;
                    break;
                }
                _be;
            }
            _be;
        } else {
            _bb("V86");
            if (vcpuins.data.prefix_oprsize) {
                _bb("prefix_oprsize(1)");
                _chr(_SetExcept_GP(0));
                _be;
            }
            if (_GetEFLAGS_IOPL == 0x03) {
                _bb("EFLAGS_IOPL(3)");
                switch (_GetOperandSize) {
                case 2:
                    _bb("OperandSize(2)");
                    _chr(_e_pop(GetRef(ceflags), 2));
                    mask |= (0xffff0000 | VCPU_EFLAGS_IOPL);
                    _be;
                    break;
                case 4:
                    _bb("OperandSize(4)");
                    _chr(_e_pop(GetRef(ceflags), 4));
                    mask |= (VCPU_EFLAGS_VM | VCPU_EFLAGS_RF | VCPU_EFLAGS_IOPL);
                    _be;
                    break;
                default:
                    _impossible_r_;
                    break;
                }
                _be;
            } else {
                _bb("EFLAGS_IOPL(!3)");
                _chr(_SetExcept_GP(0));
                _be;
            }
            _be;
        }
        vcpu.data.eflags = (ceflags & ~mask) | (vcpu.data.eflags & mask);
    }
    else {
        vcpu.data.ip++;
        _chr(_e_pop(GetRef(vcpu.data.flags), 2));
    }
    _ce;
}
static void SAHF() {
    t_nubit32 mask = (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                      VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_CF);
    _cb("SAHF");
    i386(0x9e) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    vcpu.data.eflags = (vcpu.data.ah & mask) | (vcpu.data.eflags & ~mask);
    _ce;
}
static void LAHF() {
    _cb("LAHF");
    i386(0x9f) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    vcpu.data.ah = GetMax8(vcpu.data.flags);
    _ce;
}
static void MOV_AL_MOFFS8() {
    _cb("MOV_AL_MOFFS8");
    i386(0xa0) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_moffs(1));
    _chr(_m_read_rm(1));
    vcpu.data.al = GetMax8(vcpuins.data.crm);
    _ce;
}
static void MOV_EAX_MOFFS32() {
    _cb("MOV_EAX_MOFFS32");
    i386(0xa1) {
        _adv;
        _chr(_d_moffs(_GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            vcpu.data.ax = GetMax16(vcpuins.data.crm);
            break;
        case 4:
            vcpu.data.eax = GetMax32(vcpuins.data.crm);
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_moffs(2));
        _chr(_m_read_rm(2));
        vcpu.data.ax = GetMax16(vcpuins.data.crm);
    }
    _ce;
}
static void MOV_MOFFS8_AL() {
    _cb("MOV_MOFFS8_AL");
    i386(0xa2) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_moffs(1));
    vcpuins.data.result = vcpu.data.al;
    vcpuins.data.crm = vcpuins.data.result;
    _chr(_m_write_rm(1));
    _ce;
}
static void MOV_MOFFS32_EAX() {
    _cb("MOV_MOFFS32_EAX");
    i386(0xa3) {
        _adv;
        _chr(_d_moffs(_GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            vcpuins.data.result = vcpu.data.ax;
            break;
        case 4:
            vcpuins.data.result = vcpu.data.eax;
            break;
        default:
            _impossible_r_;
            break;
        }
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(_GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_moffs(2));
        vcpuins.data.crm = vcpu.data.ax;
        _chr(_m_write_rm(2));
    }
    _ce;
}
static void MOVSB() {
    _cb("MOVSB");
    i386(0xa4) {
        _adv;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_m_movs(1));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (vcpu.data.cx) {
                    _bb("cx(!0)");
                    _chr(_m_movs(1));
                    vcpu.data.cx--;
                    _be;
                }
                if (vcpu.data.cx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (vcpu.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_m_movs(1));
                    vcpu.data.ecx--;
                    _be;
                }
                if (vcpu.data.ecx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
        }
    }
    else {
        vcpu.data.ip++;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) _m_movs(1);
        else {
            if (vcpu.data.cx) {
                _m_movs(1);
                vcpu.data.cx--;
            }
            if (vcpu.data.cx) vcpuins.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void MOVSW() {
    _cb("MOVSW");
    i386(0xa5) {
        _adv;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_m_movs(_GetOperandSize));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (vcpu.data.cx) {
                    _bb("cx(!0)");
                    _chr(_m_movs(_GetOperandSize));
                    vcpu.data.cx--;
                    _be;
                }
                if (vcpu.data.cx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (vcpu.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_m_movs(_GetOperandSize));
                    vcpu.data.ecx--;
                    _be;
                }
                if (vcpu.data.ecx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
        }
    }
    else {
        vcpu.data.ip++;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) _m_movs(2);
        else {
            if (vcpu.data.cx) {
                _m_movs(2);
                vcpu.data.cx--;
            }
            if (vcpu.data.cx) vcpuins.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void CMPSB() {
    _cb("CMPSB");
    i386(0xa6) {
        _adv;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_a_cmps(8));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (vcpu.data.cx) {
                    _bb("cx(!0)");
                    _chr(_a_cmps(8));
                    vcpu.data.cx--;
                    _be;
                }
                if (vcpu.data.cx &&
                        !(vcpuins.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(vcpuins.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    vcpuins.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (vcpu.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_a_cmps(8));
                    vcpu.data.ecx--;
                    _be;
                }
                if (vcpu.data.ecx &&
                        !(vcpuins.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(vcpuins.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    vcpuins.data.flagInsLoop = True;
                _be;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
        }
    }
    else {
        vcpu.data.ip++;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) _a_cmps(8);
        else {
            if (vcpu.data.cx) {
                _a_cmps(8);
                vcpu.data.cx--;
            }
            if (vcpu.data.cx &&
                    !(vcpuins.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(vcpuins.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                vcpuins.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void CMPSW() {
    _cb("CMPSW");
    i386(0xa7) {
        _adv;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_a_cmps(_GetOperandSize * 8));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (vcpu.data.cx) {
                    _bb("cx(!0)");
                    _chr(_a_cmps(_GetOperandSize * 8));
                    vcpu.data.cx--;
                    _be;
                }
                if (vcpu.data.cx &&
                        !(vcpuins.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(vcpuins.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    vcpuins.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (vcpu.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_a_cmps(_GetOperandSize * 8));
                    vcpu.data.ecx--;
                    _be;
                }
                if (vcpu.data.ecx &&
                        !(vcpuins.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(vcpuins.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    vcpuins.data.flagInsLoop = True;
                _be;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
        }
    }
    else {
        vcpu.data.ip++;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) _a_cmps(16);
        else {
            if (vcpu.data.cx) {
                _a_cmps(16);
                vcpu.data.cx--;
            }
            if (vcpu.data.cx &&
                    !(vcpuins.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(vcpuins.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                vcpuins.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void TEST_AL_I8() {
    _cb("TEST_AL_I8");
    i386(0xa8) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_a_test(vcpu.data.al, vcpuins.data.cimm, 8));
    _ce;
}
static void TEST_EAX_I32() {
    _cb("TEST_EAX_I32");
    i386(0xa9) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(2));
            _chr(_a_test(vcpu.data.ax, vcpuins.data.cimm, 16));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            _chr(_d_imm(4));
            _chr(_a_test(vcpu.data.eax, vcpuins.data.cimm, 32));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        _chr(_a_test(vcpu.data.ax, vcpuins.data.cimm, 16));
    }
    _ce;
}
static void STOSB() {
    _cb("STOSB");
    i386(0xaa) {
        _adv;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_m_stos(1));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (vcpu.data.cx) {
                    _bb("cx(!0)");
                    _chr(_m_stos(1));
                    vcpu.data.cx--;
                    _be;
                }
                if (vcpu.data.cx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (vcpu.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_m_stos(1));
                    vcpu.data.ecx--;
                    _be;
                }
                if (vcpu.data.ecx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
        }
    }
    else {
        vcpu.data.ip++;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) _m_stos(1);
        else {
            if (vcpu.data.cx) {
                _m_stos(1);
                vcpu.data.cx--;
            }
            if (vcpu.data.cx) vcpuins.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void STOSW() {
    _cb("STOSW");
    i386(0xab) {
        _adv;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_m_stos(_GetOperandSize));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (vcpu.data.cx) {
                    _bb("cx(!0)");
                    _chr(_m_stos(_GetOperandSize));
                    vcpu.data.cx--;
                    _be;
                }
                if (vcpu.data.cx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (vcpu.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_m_stos(_GetOperandSize));
                    vcpu.data.ecx--;
                    _be;
                }
                if (vcpu.data.ecx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
        }
    }
    else {
        vcpu.data.ip++;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) _m_stos(2);
        else {
            if (vcpu.data.cx) {
                _m_stos(2);
                vcpu.data.cx--;
            }
            if (vcpu.data.cx) vcpuins.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void LODSB() {
    _cb("LODSB");
    i386(0xac) {
        _adv;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_m_lods(1));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (vcpu.data.cx) {
                    _bb("cx(!0)");
                    _chr(_m_lods(1));
                    vcpu.data.cx--;
                    _be;
                }
                if (vcpu.data.cx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (vcpu.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_m_lods(1));
                    vcpu.data.ecx--;
                    _be;
                }
                if (vcpu.data.ecx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
        }
    }
    else {
        vcpu.data.ip++;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) _m_lods(1);
        else {
            if (vcpu.data.cx) {
                _m_lods(1);
                vcpu.data.cx--;
            }
            if (vcpu.data.cx) vcpuins.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void LODSW() {
    _cb("LODSW");
    i386(0xad) {
        _adv;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_m_lods(_GetOperandSize));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (vcpu.data.cx) {
                    _bb("cx(!0)");
                    _chr(_m_lods(_GetOperandSize));
                    vcpu.data.cx--;
                    _be;
                }
                if (vcpu.data.cx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (vcpu.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_m_lods(_GetOperandSize));
                    vcpu.data.ecx--;
                    _be;
                }
                if (vcpu.data.ecx) vcpuins.data.flagInsLoop = True;
                _be;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
        }
    }
    else {
        vcpu.data.ip++;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) _m_lods(2);
        else {
            if (vcpu.data.cx) {
                _m_lods(2);
                vcpu.data.cx--;
            }
            if (vcpu.data.cx) vcpuins.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void SCASB() {
    _cb("SCASB");
    i386(0xae) {
        _adv;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_a_scas(8));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (vcpu.data.cx) {
                    _bb("cx(!0)");
                    _chr(_a_scas(8));
                    vcpu.data.cx--;
                    _be;
                }
                if (vcpu.data.cx &&
                        !(vcpuins.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(vcpuins.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    vcpuins.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (vcpu.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_a_scas(8));
                    vcpu.data.ecx--;
                    _be;
                }
                if (vcpu.data.ecx &&
                        !(vcpuins.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(vcpuins.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    vcpuins.data.flagInsLoop = True;
                _be;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
        }
    }
    else {
        vcpu.data.ip++;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) _a_scas(8);
        else {
            if (vcpu.data.cx) {
                _a_scas(8);
                vcpu.data.cx--;
            }
            if (vcpu.data.cx &&
                    !(vcpuins.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(vcpuins.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                vcpuins.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void SCASW() {
    _cb("SCASW");
    i386(0xaf) {
        _adv;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_a_scas(_GetOperandSize * 8));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (vcpu.data.cx) {
                    _bb("cx(!0)");
                    _chr(_a_scas(_GetOperandSize * 8));
                    vcpu.data.cx--;
                    _be;
                }
                if (vcpu.data.cx &&
                        !(vcpuins.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(vcpuins.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    vcpuins.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (vcpu.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_a_scas(_GetOperandSize * 8));
                    vcpu.data.ecx--;
                    _be;
                }
                if (vcpu.data.ecx &&
                        !(vcpuins.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(vcpuins.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    vcpuins.data.flagInsLoop = True;
                _be;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
        }
    }
    else {
        vcpu.data.ip++;
        if (vcpuins.data.prefix_rep == PREFIX_REP_NONE) _a_scas(16);
        else {
            if (vcpu.data.cx) {
                _a_scas(16);
                vcpu.data.cx--;
            }
            if (vcpu.data.cx &&
                    !(vcpuins.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(vcpuins.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                vcpuins.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void MOV_AL_I8() {
    _cb("MOV_AL_I8");
    i386(0xb0) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    vcpu.data.al = GetMax8(vcpuins.data.cimm);
    _ce;
}
static void MOV_CL_I8() {
    _cb("MOV_CL_I8");
    i386(0xb1) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    vcpu.data.cl = GetMax8(vcpuins.data.cimm);
    _ce;
}
static void MOV_DL_I8() {
    _cb("MOV_DL_I8");
    i386(0xb2) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    vcpu.data.dl = GetMax8(vcpuins.data.cimm);
    _ce;
}
static void MOV_BL_I8() {
    _cb("MOV_BL_I8");
    i386(0xb3) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    vcpu.data.bl = GetMax8(vcpuins.data.cimm);
    _ce;
}
static void MOV_AH_I8() {
    _cb("MOV_AH_I8");
    i386(0xb4) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    vcpu.data.ah = GetMax8(vcpuins.data.cimm);
    _ce;
}
static void MOV_CH_I8() {
    _cb("MOV_CH_I8");
    i386(0xb5) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    vcpu.data.ch = GetMax8(vcpuins.data.cimm);
    _ce;
}
static void MOV_DH_I8() {
    _cb("MOV_DH_I8");
    i386(0xb6) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    vcpu.data.dh = GetMax8(vcpuins.data.cimm);
    _ce;
}
static void MOV_BH_I8() {
    _cb("MOV_BH_I8");
    i386(0xb7) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    vcpu.data.bh = GetMax8(vcpuins.data.cimm);
    _ce;
}
static void MOV_EAX_I32() {
    _cb("MOV_EAX_I32");
    i386(0xb8) {
        _adv;
        _chr(_d_imm(_GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            vcpu.data.ax = GetMax16(vcpuins.data.cimm);
            break;
        case 4:
            vcpu.data.eax = GetMax32(vcpuins.data.cimm);
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        vcpu.data.ax = GetMax16(vcpuins.data.cimm);
    }
    _ce;
}
static void MOV_ECX_I32() {
    _cb("MOV_ECX_I32");
    i386(0xb9) {
        _adv;
        _chr(_d_imm(_GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            vcpu.data.cx = GetMax16(vcpuins.data.cimm);
            break;
        case 4:
            vcpu.data.ecx = GetMax32(vcpuins.data.cimm);
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        vcpu.data.cx = GetMax16(vcpuins.data.cimm);
    }
    _ce;
}
static void MOV_EDX_I32() {
    _cb("MOV_EDX_I32");
    i386(0xba) {
        _adv;
        _chr(_d_imm(_GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            vcpu.data.dx = GetMax16(vcpuins.data.cimm);
            break;
        case 4:
            vcpu.data.edx = GetMax32(vcpuins.data.cimm);
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        vcpu.data.dx = GetMax16(vcpuins.data.cimm);
    }
    _ce;
}
static void MOV_EBX_I32() {
    _cb("MOV_EBX_I32");
    i386(0xbb) {
        _adv;
        _chr(_d_imm(_GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            vcpu.data.bx = GetMax16(vcpuins.data.cimm);
            break;
        case 4:
            vcpu.data.ebx = GetMax32(vcpuins.data.cimm);
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        vcpu.data.bx = GetMax16(vcpuins.data.cimm);
    }
    _ce;
}
static void MOV_ESP_I32() {
    _cb("MOV_ESP_I32");
    i386(0xbc) {
        _adv;
        _chr(_d_imm(_GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            vcpu.data.sp = GetMax16(vcpuins.data.cimm);
            break;
        case 4:
            vcpu.data.esp = GetMax32(vcpuins.data.cimm);
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        vcpu.data.sp = GetMax16(vcpuins.data.cimm);
    }
    _ce;
}
static void MOV_EBP_I32() {
    _cb("MOV_EBP_I32");
    i386(0xbd) {
        _adv;
        _chr(_d_imm(_GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            vcpu.data.bp = GetMax16(vcpuins.data.cimm);
            break;
        case 4:
            vcpu.data.ebp = GetMax32(vcpuins.data.cimm);
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        vcpu.data.bp = GetMax16(vcpuins.data.cimm);
    }
    _ce;
}
static void MOV_ESI_I32() {
    _cb("MOV_ESI_I32");
    i386(0xbe) {
        _adv;
        _chr(_d_imm(_GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            vcpu.data.si = GetMax16(vcpuins.data.cimm);
            break;
        case 4:
            vcpu.data.esi = GetMax32(vcpuins.data.cimm);
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        vcpu.data.si = GetMax16(vcpuins.data.cimm);
    }
    _ce;
}
static void MOV_EDI_I32() {
    _cb("MOV_EDI_I32");
    i386(0xbf) {
        _adv;
        _chr(_d_imm(_GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            vcpu.data.di = GetMax16(vcpuins.data.cimm);
            break;
        case 4:
            vcpu.data.edi = GetMax32(vcpuins.data.cimm);
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        vcpu.data.di = GetMax16(vcpuins.data.cimm);
    }
    _ce;
}
static void INS_C0() {
    _cb("INS_C0");
    _new_code_path_;
    i386(0xc0) {
        _adv;
        _chr(_d_modrm(0, 1));
        _chr(_m_read_rm(1));
        _chr(_d_imm(1));
        switch (vcpuins.data.cr) {
        case 0: /* ROL_RM8_I8 */
            _bb("ROL_RM8_I8");
            _chr(_a_rol(GetMax8(vcpuins.data.crm), GetMax8(vcpuins.data.cimm), 8));
            _be;
            break;
        case 1: /* ROR_RM8_I8 */
            _bb("ROR_RM8_I8");
            _chr(_a_ror(GetMax8(vcpuins.data.crm), GetMax8(vcpuins.data.cimm), 8));
            _be;
            break;
        case 2: /* RCL_RM8_I8 */
            _bb("RCL_RM8_I8");
            _chr(_a_rcl(GetMax8(vcpuins.data.crm), GetMax8(vcpuins.data.cimm), 8));
            _be;
            break;
        case 3: /* RCR_RM8_I8 */
            _bb("RCR_RM8_I8");
            _chr(_a_rcr(GetMax8(vcpuins.data.crm), GetMax8(vcpuins.data.cimm), 8));
            _be;
            break;
        case 4: /* SHL_RM8_I8 */
            _bb("SHL_RM8_I8");
            _chr(_a_shl(GetMax8(vcpuins.data.crm), GetMax8(vcpuins.data.cimm), 8));
            _be;
            break;
        case 5: /* SHR_RM8_I8 */
            _bb("SHR_RM8_I8");
            _chr(_a_shr(GetMax8(vcpuins.data.crm), GetMax8(vcpuins.data.cimm), 8));
            _be;
            break;
        case 6: /* UndefinedOpcode */
            _bb("vcpuins.data.cr(6)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 7: /* SAR_RM8_I8 */
            _bb("SAR_RM8_I8");
            _chr(_a_sar(GetMax8(vcpuins.data.crm), GetMax8(vcpuins.data.cimm), 8));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(1));
    }
    else
        UndefinedOpcode();
    _ce;
}
static void INS_C1() {
    _cb("INS_C1");
    i386(0xc1) {
        _adv;
        _chr(_d_modrm(0, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        _chr(_d_imm(1));
        switch (vcpuins.data.cr) {
        case 0: /* ROL_RM32_I8 */
            _bb("ROL_RM32_I8");
            _chr(_a_rol(GetMax32(vcpuins.data.crm), GetMax8(vcpuins.data.cimm), _GetOperandSize * 8));
            _be;
            break;
        case 1: /* ROR_RM32_I8 */
            _bb("ROR_RM32_I8");
            _chr(_a_ror(GetMax32(vcpuins.data.crm), GetMax8(vcpuins.data.cimm), _GetOperandSize * 8));
            _be;
            break;
        case 2: /* RCL_RM32_I8 */
            _bb("RCL_RM32_I8");
            _chr(_a_rcl(GetMax32(vcpuins.data.crm), GetMax8(vcpuins.data.cimm), _GetOperandSize * 8));
            _be;
            break;
        case 3: /* RCR_RM32_I8 */
            _bb("RCR_RM32_I8");
            _chr(_a_rcr(GetMax32(vcpuins.data.crm), GetMax8(vcpuins.data.cimm), _GetOperandSize * 8));
            _be;
            break;
        case 4: /* SHL_RM32_I8 */
            _bb("SHL_RM32_I8");
            _chr(_a_shl(GetMax32(vcpuins.data.crm), GetMax8(vcpuins.data.cimm), _GetOperandSize * 8));
            _be;
            break;
        case 5: /* SHR_RM32_I8 */
            _bb("SHR_RM32_I8");
            _chr(_a_shr(GetMax32(vcpuins.data.crm), GetMax8(vcpuins.data.cimm), _GetOperandSize * 8));
            _be;
            break;
        case 6: /* UndefinedOpcode */
            _bb("vcpuins.data.cr(6)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 7: /* SAR_RM32_I8 */
            _bb("SAR_RM32_I8");
            _chr(_a_sar(GetMax32(vcpuins.data.crm), GetMax8(vcpuins.data.cimm), _GetOperandSize * 8));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(_GetOperandSize));
    }
    else
        UndefinedOpcode();
    _ce;
}
static void RET_I16() {
    _cb("RET_I16");
    i386(0xc2) {
        _adv;
        _chr(_d_imm(2));
        _chr(_e_ret_near(GetMax16(vcpuins.data.cimm), _GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _d_imm(2);
        _e_ret_near(GetMax16(vcpuins.data.cimm), 2);
    }
    _ce;
}
static void RET() {
    _cb("RET");
    i386(0xc3) {
        _adv;
        _chr(_e_ret_near(0, _GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_e_ret_near(0, 2));
    }
    _ce;
}
static void LES_R32_M16_32() {
    t_nubit16 selector;
    t_nubit32 offset;
    _cb("LES_R32_M16_32");
    i386(0xc4) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
        if (!vcpuins.data.flagMem) {
            _bb("flagMem(0)");
            _chr(UndefinedOpcode());
            _be;
        }
        _chr(_m_read_rm(_GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            offset = GetMax16(vcpuins.data.crm);
            break;
        case 4:
            offset = GetMax32(vcpuins.data.crm);
            break;
        default:
            _impossible_r_;
            break;
        }
        vcpuins.data.mrm.offset += _GetOperandSize;
        _chr(_m_read_rm(2));
        selector = GetMax16(vcpuins.data.crm);
        _chr(_e_load_far(&vcpu.data.es, vcpuins.data.rr, selector, offset, _GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _d_modrm(2, 4);
        _chr(_m_read_rm(2));
        offset = GetMax16(vcpuins.data.crm);
        vcpuins.data.mrm.offset += 2;
        _chr(_m_read_rm(2));
        selector = GetMax16(vcpuins.data.crm);
        _chr(_e_load_far(&vcpu.data.es, vcpuins.data.rr, selector, offset, 2));
    }
    _ce;
}
static void LDS_R32_M16_32() {
    t_nubit16 selector;
    t_nubit32 offset;
    _cb("LDS_R32_M16_32");
    i386(0xc5) {
        _adv;
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
        if (!vcpuins.data.flagMem) {
            _bb("flagMem(0)");
            _chr(UndefinedOpcode());
            _be;
        }
        _chr(_m_read_rm(_GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            offset = GetMax16(vcpuins.data.crm);
            break;
        case 4:
            offset = GetMax32(vcpuins.data.crm);
            break;
        default:
            _impossible_r_;
            break;
        }
        vcpuins.data.mrm.offset += _GetOperandSize;
        _chr(_m_read_rm(2));
        selector = GetMax16(vcpuins.data.crm);
        _chr(_e_load_far(&vcpu.data.ds, vcpuins.data.rr, selector, offset, _GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _d_modrm(2, 4);
        _chr(_m_read_rm(2));
        offset = GetMax16(vcpuins.data.crm);
        vcpuins.data.mrm.offset += 2;
        _chr(_m_read_rm(2));
        selector = GetMax16(vcpuins.data.crm);
        _chr(_e_load_far(&vcpu.data.ds, vcpuins.data.rr, selector, offset, 2));
    }
    _ce;
}
static void INS_C6() {
    _cb("INS_C6");
    i386(0xc6) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(0, 1));
    switch (vcpuins.data.cr) {
    case 0: /* MOV_RM8_I8 */
        _bb("MOV_RM8_I8");
        _chr(_d_imm(1));
        vcpuins.data.crm = vcpuins.data.cimm;
        _chr(_m_write_rm(1));
        _be;
        break;
    case 1:
        _bb("vcpuins.data.cr(1)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 2:
        _bb("vcpuins.data.cr(2)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 3:
        _bb("vcpuins.data.cr(3)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 4:
        _bb("vcpuins.data.cr(4)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 5:
        _bb("vcpuins.data.cr(5)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 6:
        _bb("vcpuins.data.cr(6)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 7:
        _bb("vcpuins.data.cr(7)");
        _chr(UndefinedOpcode());
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void INS_C7() {
    _cb("INS_C7");
    i386(0xc7) {
        _adv;
        _chr(_d_modrm(0, _GetOperandSize));
        switch (vcpuins.data.cr) {
        case 0: /* MOV_RM32_I32 */
            _bb("MOV_RM32_I32");
            _chr(_d_imm(_GetOperandSize));
            vcpuins.data.crm = vcpuins.data.cimm;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 1:
            _bb("vcpuins.data.cr(1)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 2:
            _bb("vcpuins.data.cr(2)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 3:
            _bb("vcpuins.data.cr(3)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 4:
            _bb("vcpuins.data.cr(4)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 5:
            _bb("vcpuins.data.cr(5)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 6:
            _bb("vcpuins.data.cr(6)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 7:
            _bb("vcpuins.data.cr(7)");
            _chr(UndefinedOpcode());
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(0, 2));
        switch (vcpuins.data.cr) {
        case 0: /* MOV_RM16_I16 */
            _bb("MOV_RM16_I16");
            _chr(_d_imm(2));
            vcpuins.data.crm = vcpuins.data.cimm;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 1:
            _bb("vcpuins.data.cr(1)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 2:
            _bb("vcpuins.data.cr(2)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 3:
            _bb("vcpuins.data.cr(3)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 4:
            _bb("vcpuins.data.cr(4)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 5:
            _bb("vcpuins.data.cr(5)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 6:
            _bb("vcpuins.data.cr(6)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 7:
            _bb("vcpuins.data.cr(7)");
            _chr(UndefinedOpcode());
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    _ce;
}
static void ENTER() {
    t_nubit32 data = 0;
    t_nubit32 temp = 0;
    t_nubitcc i = 0;
    t_nubit16 size = Zero16;
    t_nubit8 level = Zero8;
    _cb("ENTER");
    i386(0xc8) {
        _adv;
        _chr(_d_imm(2));
        size = (t_nubit16) vcpuins.data.cimm;
        _chr(_d_imm(1));
        level = (t_nubit8) vcpuins.data.cimm;
        level %= 32;
        switch (_GetStackSize) {
        case 2:
            _bb("StackSize(2)");
            _chr(_e_push(GetRef(vcpu.data.bp), 2));
            temp = vcpu.data.sp;
            _be;
            break;
        case 4:
            _bb("StackSize(4)");
            _chr(_e_push(GetRef(vcpu.data.ebp), 4));
            temp = vcpu.data.esp;
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
        if (level) {
            _bb("level(!0)");
            for (i = 0; i < level; ++i) {
                _bb("for");
                switch (_GetOperandSize) {
                case 2:
                    _bb("OperandSize(2)");
                    switch (_GetStackSize) {
                    case 2:
                        _bb("StackSize(2)");
                        vcpu.data.bp -= 2;
                        _chr(_s_read_ss(vcpu.data.bp, GetRef(data), 2));
                        _chr(_e_push(GetRef(data), 2));
                        _be;
                        break;
                    case 4:
                        _bb("StackSize(4)");
                        vcpu.data.ebp -= 2;
                        _chr(_s_read_ss(vcpu.data.ebp, GetRef(data), 2));
                        _chr(_e_push(GetRef(data), 2));
                        _be;
                        break;
                    default:
                        _impossible_r_;
                        break;
                    }
                    _be;
                    break;
                case 4:
                    _bb("OperandSize(4)");
                    switch (_GetStackSize) {
                    case 2:
                        _bb("StackSize(2)");
                        vcpu.data.bp -= 4;
                        _chr(_s_read_ss(vcpu.data.bp, GetRef(data), 4));
                        _chr(_e_push(GetRef(data), 4));
                        _be;
                        break;
                    case 4:
                        _bb("StackSize(4)");
                        vcpu.data.ebp -= 4;
                        _chr(_s_read_ss(vcpu.data.ebp, GetRef(data), 4));
                        _chr(_e_push(GetRef(data), 4));
                        _be;
                        break;
                    default:
                        _impossible_r_;
                        break;
                    }
                    _be;
                    break;
                default:
                    _impossible_r_;
                    break;
                }
                _be;
            }
            switch (_GetOperandSize) {
            case 2:
                _bb("OperandSize(2)");
                _chr(_e_push(GetRef(temp), 2));
                _be;
                break;
            case 4:
                _bb("OperandSize(4)");
                _chr(_e_push(GetRef(temp), 4));
                _be;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
        }
        switch (_GetStackSize) {
        case 2:
            _bb("StackSize(2)");
            vcpu.data.bp = GetMax16(temp);
            vcpu.data.sp = vcpu.data.bp - size;
            _be;
            break;
        case 4:
            _bb("StackSize(4)");
            vcpu.data.ebp = GetMax32(temp);
            vcpu.data.esp = vcpu.data.ebp - size;
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else
        UndefinedOpcode();
    _ce;
}
static void LEAVE() {
    _cb("LEAVE");
    i386(0xc9) {
        _adv;
        if (!_IsProtected && vcpu.data.ebp > 0x0000ffff) {
            _bb("Protected(0),ebp(>0000ffff)");
            _chr(_SetExcept_GP(0));
            _be;
        }
        _chr(_m_test_logical(&vcpu.data.ss, vcpu.data.ebp, _GetOperandSize, 1));
        switch (_GetStackSize) {
        case 2:
            vcpu.data.sp = vcpu.data.bp;
            break;
        case 4:
            vcpu.data.esp = vcpu.data.ebp;
            break;
        default:
            _impossible_r_;
            break;
        }
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(GetRef(vcpu.data.bp), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(GetRef(vcpu.data.ebp), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else
        UndefinedOpcode();
    _ce;
}
static void RETF_I16() {
    _cb("RETF_I16");
    i386(0xca) {
        _adv;
        _chr(_d_imm(2));
        _chr(_e_ret_far(GetMax16(vcpuins.data.cimm), _GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _d_imm(2);
        _e_ret_far(GetMax16(vcpuins.data.cimm), 2);
    }
    _ce;
}
static void RETF() {
    _cb("RETF");
    i386(0xcb) {
        _adv;
        _chr(_e_ret_far(0, _GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_e_ret_far(0, 2));
    }
    _ce;
}
static void INT3() {
    _cb("INT3");
    _new_code_path_;
    i386(0xcc) {
        _adv;
        _chr(_e_int3(_GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _e_int_n(0x03, _GetOperandSize);
    }
    _ce;
}
static void INT_I8() {
    _cb("INT_I8");
    i386(0xcc) {
        _adv;
        _chr(_d_imm(1));
        _chr(_e_int_n((t_nubit8)vcpuins.data.cimm, _GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _d_imm(1);
        _e_int_n((t_nubit8)vcpuins.data.cimm, 2);
    }
    _ce;
}
static void INTO() {
    _cb("INTO");
    _new_code_path_;
    i386(0xce) {
        _adv;
        _chr(_e_into(_GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        if (_GetEFLAGS_OF) _e_int_n(0x04, _GetOperandSize);
    }
    _ce;
}
static void IRET() {
    _cb("IRET");
    i386(0xcf) {
        _adv;
        _chr(_e_iret(_GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_e_iret(2));
    }
    _ce;
}
static void INS_D0() {
    _cb("INS_D0");
    i386(0xd0) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(0, 1));
    _chr(_m_read_rm(1));
    switch (vcpuins.data.cr) {
    case 0: /* ROL_RM8 */
        _bb("ROL_RM8");
        _chr(_a_rol(vcpuins.data.crm, 1, 8));
        _be;
        break;
    case 1: /* ROR_RM8 */
        _bb("ROR_RM8");
        _chr(_a_ror(vcpuins.data.crm, 1, 8));
        _be;
        break;
    case 2: /* RCL_RM8 */
        _bb("RCL_RM8");
        _chr(_a_rcl(vcpuins.data.crm, 1, 8));
        _be;
        break;
    case 3: /* RCR_RM8 */
        _bb("RCR_RM8");
        _chr(_a_rcr(vcpuins.data.crm, 1, 8));
        _be;
        break;
    case 4: /* SHL_RM8 */
        _bb("SHL_RM8");
        _chr(_a_shl(vcpuins.data.crm, 1, 8));
        _be;
        break;
    case 5: /* SHR_RM8 */
        _bb("SHR_RM8");
        _chr(_a_shr(vcpuins.data.crm, 1, 8));
        _be;
        break;
    case 6: /* UndefinedOpcode */
        _bb("vcpuins.data.cr(6)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 7: /* SAR_RM8 */
        _bb("SAR_RM8");
        _chr(_a_sar(vcpuins.data.crm, 1, 8));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    vcpuins.data.crm = vcpuins.data.result;
    _chr(_m_write_rm(1));
    _ce;
}
static void INS_D1() {
    _cb("INS_D1");
    i386(0xd1) {
        _adv;
        _chr(_d_modrm(0, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        switch (vcpuins.data.cr) {
        case 0: /* ROL_RM32 */
            _bb("ROL_RM32");
            _chr(_a_rol(vcpuins.data.crm, 1, _GetOperandSize * 8));
            _be;
            break;
        case 1: /* ROR_RM32 */
            _bb("ROR_RM32");
            _chr(_a_ror(vcpuins.data.crm, 1, _GetOperandSize * 8));
            _be;
            break;
        case 2: /* RCL_RM32 */
            _bb("RCL_RM32");
            _chr(_a_rcl(vcpuins.data.crm, 1, _GetOperandSize * 8));
            _be;
            break;
        case 3: /* RCR_RM32 */
            _bb("RCR_RM32");
            _chr(_a_rcr(vcpuins.data.crm, 1, _GetOperandSize * 8));
            _be;
            break;
        case 4: /* SHL_RM32 */
            _bb("SHL_RM32");
            _chr(_a_shl(vcpuins.data.crm, 1, _GetOperandSize * 8));
            _be;
            break;
        case 5: /* SHR_RM32 */
            _bb("SHR_RM32");
            _chr(_a_shr(vcpuins.data.crm, 1, _GetOperandSize * 8));
            _be;
            break;
        case 6: /* UndefinedOpcode */
            _bb("vcpuins.data.cr(6)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 7: /* SAR_RM32 */
            _bb("SAR_RM32");
            _chr(_a_sar(vcpuins.data.crm, 1, _GetOperandSize * 8));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(_GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(0, 2));
        _chr(_m_read_rm(2));
        switch (vcpuins.data.cr) {
        case 0: /* ROL_RM16 */
            _bb("ROL_RM16");
            _chr(_a_rol(vcpuins.data.crm, 1, 16));
            _be;
            break;
        case 1: /* ROR_RM16 */
            _bb("ROR_RM16");
            _chr(_a_ror(vcpuins.data.crm, 1, 16));
            _be;
            break;
        case 2: /* RCL_RM16 */
            _bb("RCL_RM16");
            _chr(_a_rcl(vcpuins.data.crm, 1, 16));
            _be;
            break;
        case 3: /* RCR_RM16 */
            _bb("RCR_RM16");
            _chr(_a_rcr(vcpuins.data.crm, 1, 16));
            _be;
            break;
        case 4: /* SHL_RM16 */
            _bb("SHL_RM16");
            _chr(_a_shl(vcpuins.data.crm, 1, 16));
            _be;
            break;
        case 5: /* SHR_RM16 */
            _bb("SHR_RM16");
            _chr(_a_shr(vcpuins.data.crm, 1, 16));
            _be;
            break;
        case 6: /* UndefinedOpcode */
            _bb("vcpuins.data.cr(6)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 7: /* SAR_RM16 */
            _bb("SAR_RM16");
            _chr(_a_sar(vcpuins.data.crm, 1, 16));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(2));
    }
    _ce;
}
static void INS_D2() {
    _cb("INS_D2");
    i386(0xd2) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(0, 1));
    _chr(_m_read_rm(1));
    switch (vcpuins.data.cr) {
    case 0: /* ROL_RM8_CL */
        _bb("ROL_RM8_CL");
        _chr(_a_rol(vcpuins.data.crm, vcpu.data.cl, 8));
        _be;
        break;
    case 1: /* ROR_RM8_CL */
        _bb("ROR_RM8_CL");
        _chr(_a_ror(vcpuins.data.crm, vcpu.data.cl, 8));
        _be;
        break;
    case 2: /* RCL_RM8_CL */
        _bb("RCL_RM8_CL");
        _chr(_a_rcl(vcpuins.data.crm, vcpu.data.cl, 8));
        _be;
        break;
    case 3: /* RCR_RM8_CL */
        _bb("RCR_RM8_CL");
        _chr(_a_rcr(vcpuins.data.crm, vcpu.data.cl, 8));
        _be;
        break;
    case 4: /* SHL_RM8_CL */
        _bb("SHL_RM8_CL");
        _chr(_a_shl(vcpuins.data.crm, vcpu.data.cl, 8));
        _be;
        break;
    case 5: /* SHR_RM8_CL */
        _bb("SHR_RM8_CL");
        _chr(_a_shr(vcpuins.data.crm, vcpu.data.cl, 8));
        _be;
        break;
    case 6: /* UndefinedOpcode */
        _bb("vcpuins.data.cr(6)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 7: /* SAR_RM8_CL */
        _bb("SAR_RM8_CL");
        _chr(_a_sar(vcpuins.data.crm, vcpu.data.cl, 8));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    vcpuins.data.crm = vcpuins.data.result;
    _chr(_m_write_rm(1));
    _ce;
}
static void INS_D3() {
    _cb("INS_D3");
    i386(0xd3) {
        _adv;
        _chr(_d_modrm(0, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        switch (vcpuins.data.cr) {
        case 0: /* ROL_RM32_CL */
            _bb("ROL_RM32_CL");
            _chr(_a_rol(vcpuins.data.crm, vcpu.data.cl, _GetOperandSize * 8));
            _be;
            break;
        case 1: /* ROR_RM32_CL */
            _bb("ROR_RM32_CL");
            _chr(_a_ror(vcpuins.data.crm, vcpu.data.cl, _GetOperandSize * 8));
            _be;
            break;
        case 2: /* RCL_RM32_CL */
            _bb("RCL_RM32_CL");
            _chr(_a_rcl(vcpuins.data.crm, vcpu.data.cl, _GetOperandSize * 8));
            _be;
            break;
        case 3: /* RCR_RM32_CL */
            _bb("RCR_RM32_CL");
            _chr(_a_rcr(vcpuins.data.crm, vcpu.data.cl, _GetOperandSize * 8));
            _be;
            break;
        case 4: /* SHL_RM32_CL */
            _bb("SHL_RM32_CL");
            _chr(_a_shl(vcpuins.data.crm, vcpu.data.cl, _GetOperandSize * 8));
            _be;
            break;
        case 5: /* SHR_RM32_CL */
            _bb("SHR_RM32_CL");
            _chr(_a_shr(vcpuins.data.crm, vcpu.data.cl, _GetOperandSize * 8));
            _be;
            break;
        case 6: /* UndefinedOpcode */
            _bb("vcpuins.data.cr(6)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 7: /* SAR_RM32_CL */
            _bb("SAR_RM32_CL");
            _chr(_a_sar(vcpuins.data.crm, vcpu.data.cl, _GetOperandSize * 8));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(_GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(0, 2));
        _chr(_m_read_rm(2));
        switch (vcpuins.data.cr) {
        case 0: /* ROL_RM16_CL */
            _bb("ROL_RM16_CL");
            _chr(_a_rol(vcpuins.data.crm, vcpu.data.cl, 16));
            _be;
            break;
        case 1: /* ROR_RM16_CL */
            _bb("ROR_RM16_CL");
            _chr(_a_ror(vcpuins.data.crm, vcpu.data.cl, 16));
            _be;
            break;
        case 2: /* RCL_RM16_CL */
            _bb("RCL_RM16_CL");
            _chr(_a_rcl(vcpuins.data.crm, vcpu.data.cl, 16));
            _be;
            break;
        case 3: /* RCR_RM16_CL */
            _bb("RCR_RM16_CL");
            _chr(_a_rcr(vcpuins.data.crm, vcpu.data.cl, 16));
            _be;
            break;
        case 4: /* SHL_RM16_CL */
            _bb("SHL_RM16_CL");
            _chr(_a_shl(vcpuins.data.crm, vcpu.data.cl, 16));
            _be;
            break;
        case 5: /* SHR_RM16_CL */
            _bb("SHR_RM16_CL");
            _chr(_a_shr(vcpuins.data.crm, vcpu.data.cl, 16));
            _be;
            break;
        case 6: /* UndefinedOpcode */
            _bb("vcpuins.data.cr(6)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 7: /* SAR_RM16_CL */
            _bb("SAR_RM16_CL");
            _chr(_a_sar(vcpuins.data.crm, vcpu.data.cl, 16));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(2));
    }
    _ce;
}
static void AAM() {
    t_nubit8 base;
    _cb("AAM");
    i386(0xd4) {
        _adv;
        _chr(_d_imm(1));
    }
    else {
        vcpu.data.ip++;
        _d_imm(1);
    }
    base = GetMax8(vcpuins.data.cimm);
    vcpu.data.ah = vcpu.data.al / base;
    vcpu.data.al = vcpu.data.al % base;
    vcpuins.data.bit = 0x08;
    vcpuins.data.result = GetMax8(vcpu.data.al);
    _chr(_kaf_set_flags(AAM_FLAG));
    vcpuins.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF);
    _ce;
}
static void AAD() {
    t_nubit8 base;
    _cb("AAD");
    i386(0xd5) {
        _adv;
        _chr(_d_imm(1));
    }
    else {
        vcpu.data.ip++;
        _d_imm(1);
    }
    base = GetMax8(vcpuins.data.cimm);
    vcpu.data.al = GetMax8(vcpu.data.al + (vcpu.data.ah * base));
    vcpu.data.ah = 0x00;
    vcpuins.data.bit = 0x08;
    vcpuins.data.result = GetMax8(vcpu.data.al);
    _chr(_kaf_set_flags(AAD_FLAG));
    vcpuins.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF);
    _ce;
}
static void XLAT() {
    _cb("XLAT");
    i386(0xd7) {
        _adv;
        switch (_GetAddressSize) {
        case 2:
            _bb("AddressSize(2)");
            _chr(_m_read_logical(vcpuins.data.roverds, (vcpu.data.bx + vcpu.data.al), GetRef(vcpu.data.al), 1));
            _be;
            break;
        case 4:
            _bb("AddressSize(4)");
            _chr(_m_read_logical(vcpuins.data.roverds, (vcpu.data.ebx + vcpu.data.al), GetRef(vcpu.data.al), 1));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_m_read_logical(vcpuins.data.roverds, (vcpu.data.bx + vcpu.data.al), GetRef(vcpu.data.al), 1));
    }
    _ce;
}
static void LOOPNZ_REL8() {
    _cb("LOOPNZ_REL8");
    i386(0xe0) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_loopcc((t_nsbit8)vcpuins.data.cimm, !_GetEFLAGS_ZF));
    _ce;
}
static void LOOPZ_REL8() {
    _cb("LOOPZ_REL8");
    i386(0xe1) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_loopcc((t_nsbit8)vcpuins.data.cimm, _GetEFLAGS_ZF));
    _ce;
}
static void LOOP_REL8() {
    _cb("LOOP_REL8");
    i386(0xe2) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_loopcc((t_nsbit8)vcpuins.data.cimm, 1));
    _ce;
}
static void JCXZ_REL8() {
    t_nubit32 cecx = 0x00000000;
    _cb("JCXZ_REL8");
    i386(0xe3) {
        _adv;
        _chr(_d_imm(1));
        switch (_GetAddressSize) {
        case 2:
            cecx = vcpu.data.cx;
            break;
        case 4:
            cecx = vcpu.data.ecx;
            break;
        default:
            _impossible_r_;
            break;
        }
        _chr(_e_jcc(GetMax32(vcpuins.data.cimm), 1, !cecx));
    }
    else {
        vcpu.data.ip++;
        _d_imm(1);
        _e_jcc(GetMax32(vcpuins.data.cimm), 1, !vcpu.data.cx);
    }
    _ce;
}
static void IN_AL_I8() {
    _cb("IN_AL_I8");
    i386(0xe4) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_p_input(GetMax8(vcpuins.data.cimm), GetRef(vcpu.data.al), 1));
    _ce;
}
static void IN_EAX_I8() {
    _cb("IN_EAX_I8");
    _new_code_path_;
    i386(0xe5) {
        _adv;
        _chr(_d_imm(1));
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_p_input(GetMax8(vcpuins.data.cimm), GetRef(vcpu.data.ax), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_p_input(GetMax8(vcpuins.data.cimm), GetRef(vcpu.data.eax), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(1));
        _chr(_p_input(GetMax8(vcpuins.data.cimm), GetRef(vcpu.data.ax), 2));
    }
    _ce;
}
static void OUT_I8_AL() {
    _cb("OUT_I8_AL");
    i386(0xe6) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_p_output(GetMax8(vcpuins.data.cimm), GetRef(vcpu.data.al), 1));
    _ce;

}
static void OUT_I8_EAX() {
    _cb("OUT_I8_EAX");
    _new_code_path_;
    i386(0xe7) {
        _adv;
        _chr(_d_imm(1));
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_p_output(GetMax8(vcpuins.data.cimm), GetRef(vcpu.data.ax), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_p_output(GetMax8(vcpuins.data.cimm), GetRef(vcpu.data.eax), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _d_imm(1);
        _p_output(GetMax8(vcpuins.data.cimm), GetRef(vcpu.data.ax), 2);
    }
    _ce;
}
static void CALL_REL32() {
    _cb("CALL_REL32");
    i386(0xe8) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(2));
            _chr(_e_call_near(GetMax16(vcpu.data.ip + (t_nsbit16)vcpuins.data.cimm), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            _chr(_d_imm(4));
            _chr(_e_call_near(GetMax32(vcpu.data.eip + (t_nsbit32)vcpuins.data.cimm), 4));
            _be;
            break;
        default:
            _bb("OperandSize");
            _chr(_SetExcept_CE(_GetOperandSize));
            _be;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        _chr(_e_call_near(GetMax16(vcpu.data.ip + (t_nsbit16)vcpuins.data.cimm), 2));
    }
    _ce;
}
static void JMP_REL32() {
    _cb("JMP_REL32");
    i386(0xe9) {
        _adv;
        _chr(_d_imm(_GetOperandSize));
        _chr(_e_jcc(GetMax32(vcpuins.data.cimm), _GetOperandSize, 1));
    }
    else {
        vcpu.data.ip++;
        _d_imm(2);
        _e_jcc(GetMax32(vcpuins.data.cimm), 2, 1);
    }
    _ce;
}
static void JMP_PTR16_32() {
    t_nubit16 newcs = 0x0000;
    t_nubit32 neweip = 0x00000000;
    _cb("JMP_PTR16_32");
    i386(0xea) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(2));
            neweip = (t_nubit16)vcpuins.data.cimm;
            _chr(_d_imm(2));
            newcs = (t_nubit16)vcpuins.data.cimm;
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            _chr(_d_imm(4));
            neweip = (t_nubit32)vcpuins.data.cimm;
            _chr(_d_imm(2));
            newcs = (t_nubit16)vcpuins.data.cimm;
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
        _chr(_e_jmp_far(newcs, neweip, _GetOperandSize));
    }
    else {
        vcpu.data.ip++;
        _chr(_d_imm(2));
        neweip = (t_nubit16)vcpuins.data.cimm;
        _chr(_d_imm(2));
        newcs = (t_nubit16)vcpuins.data.cimm;
        _chr(_e_jmp_far(newcs, neweip, 2));
    }
    _ce;
}
static void JMP_REL8() {
    _cb("JMP_REL8");
    i386(0xeb) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_imm(1));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), 1, 1));
    _ce;
}
static void IN_AL_DX() {
    _cb("IN_AL_DX");
    i386(0xec) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_p_input(vcpu.data.dx, GetRef(vcpu.data.al), 1));
    _ce;
}
static void IN_EAX_DX() {
    _cb("IN_EAX_DX");
    i386(0xed) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_p_input(vcpu.data.dx, GetRef(vcpu.data.ax), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_p_input(vcpu.data.dx, GetRef(vcpu.data.eax), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_p_input(vcpu.data.dx, GetRef(vcpu.data.ax), 2));
    }
    _ce;
}
static void OUT_DX_AL() {
    _cb("OUT_DX_AL");
    i386(0xee) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_p_output(vcpu.data.dx, GetRef(vcpu.data.al), 1));
    _ce;
}
static void OUT_DX_EAX() {
    _cb("OUT_DX_EAX");
    _new_code_path_;
    i386(0xef) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_p_output(vcpu.data.dx, GetRef(vcpu.data.ax), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_p_output(vcpu.data.dx, GetRef(vcpu.data.eax), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _p_output(vcpu.data.dx, GetRef(vcpu.data.ax), 2);
    }
    _ce;
}
static void PREFIX_LOCK() {
    t_nubit8 opcode = 0x00;
    t_nubit8 modrm = 0x00;
    t_nubit8 opcode_0f = 0x00;
    t_nubit32 ceip = vcpu.data.eip;
    _cb("PREFIX_LOCK");
    i386(0xf0) {
        _adv;
        do {
            _chr(_s_read_cs(ceip, GetRef(opcode), 1));
            ceip++;
        } while (_kdf_check_prefix(opcode));
        switch (opcode) {
        case 0x00:
        case 0x01: /* ADD */
        case 0x08:
        case 0x09: /* OR */
        case 0x10:
        case 0x11: /* ADC */
        case 0x18:
        case 0x19: /* SBB */
        case 0x20:
        case 0x21: /* AND */
        case 0x28:
        case 0x29: /* SUB */
        case 0x30:
        case 0x31: /* XOR */
        case 0x86:
        case 0x87: /* XCHG */
        case 0xf6:
        case 0xf7: /* NOT, NEG */
        case 0xfe:
        case 0xff: /* DEC, INC */
            vcpuins.data.flagLock = True;
            break;
        case 0x80:
        case 0x81:
        case 0x83:
            _bb("opcode(0x80/0x81/0x83)");
            _chr(_s_read_cs(ceip, GetRef(modrm), 1));
            if (_GetModRM_REG(modrm) != 7)
                vcpuins.data.flagLock = True;
            else {
                _bb("ModRM_REG(7)");
                _chr(_SetExcept_UD(0));
                _be;
            }
            _be;
            break;
        case 0x0f:
            _bb("opcode(0f)");
            _chr(_s_read_cs(ceip, GetRef(opcode_0f), 1));
            switch (opcode_0f) {
            case 0xa3: /* BT */
            case 0xab: /* BTS */
            case 0xb3: /* BTR */
            case 0xbb: /* BTC */
            case 0xba:
                vcpuins.data.flagLock = True;
                break;
            default:
                _bb("opcode_0f");
                _chr(_SetExcept_UD(0));
                _be;
                break;
            }
            _be;
            break;
        default:
            _bb("opcode");
            _chr(_SetExcept_UD(0));
            _be;
            break;
        }
    }
    else _adv;
    _ce;
}
static void PREFIX_REPNZ() {
    _cb("PREFIX_REPNZ");
    i386(0xf2) {
        _adv;
        vcpuins.data.prefix_rep = PREFIX_REP_REPZNZ;
    }
    else {
        vcpu.data.ip++;
        vcpuins.data.prefix_rep = PREFIX_REP_REPZNZ;
    }
    _ce;
}
static void PREFIX_REPZ() {
    _cb("PREFIX_REPZ");
    i386(0xf3) {
        _adv;
        vcpuins.data.prefix_rep = PREFIX_REP_REPZ;
    }
    else {
        vcpu.data.ip++;
        vcpuins.data.prefix_rep = PREFIX_REP_REPZ;
    }
    _ce;
}
static void HLT() {
    _cb("HLT");
    _new_code_path_;
    if (_GetCR0_PE && _GetCPL) {
        _bb("CR0_PE(1),CPL(!0)");
        _SetExcept_GP(0);
        _be;
    }
    _adv;
    vcpu.data.flagHalt = True;
    _ce;
}
static void CMC() {
    _cb("CMC");
    i386(0xf5) {
        _adv;
        vcpu.data.eflags ^= VCPU_EFLAGS_CF;
    }
    else {
        vcpu.data.ip++;
        vcpu.data.eflags ^= VCPU_EFLAGS_CF;
    }
    _ce;
}
static void INS_F6() {
    _cb("INS_F6");
    i386(0xf6) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(0, 1));
    _chr(_m_read_rm(1));
    switch (vcpuins.data.cr) {
    case 0: /* TEST_RM8_I8 */
        _bb("TEST_RM8_I8");
        _chr(_d_imm(1));
        _chr(_a_test(vcpuins.data.crm, vcpuins.data.cimm, 8));
        _be;
        break;
    case 1: /* UndefinedOpcode */
        _bb("ModRM_REG(1)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 2: /* NOT_RM8 */
        _bb("NOT_RM8");
        _chr(_a_not(vcpuins.data.crm, 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(1));
        _be;
        break;
    case 3: /* NEG_RM8 */
        _bb("NEG_RM8");
        _chr(_a_neg(vcpuins.data.crm, 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(1));
        _be;
        break;
    case 4: /* MUL_RM8 */
        _bb("MUL_RM8");
        _chr(_a_mul(vcpuins.data.crm, 8));
        _be;
        break;
    case 5: /* IMUL_RM8 */
        _bb("IMUL_RM8");
        _chr(_a_imul(vcpuins.data.crm, 8));
        _be;
        break;
    case 6: /* DIV_RM8 */
        _bb("DIV_RM8");
        _chr(_a_div(vcpuins.data.crm, 8));
        _be;
        break;
    case 7: /* IDIV_RM8 */
        _bb("IDIV_RM8");
        _new_code_path_;
        _chr(_a_idiv(vcpuins.data.crm, 8));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void INS_F7() {
    _cb("INS_F7");
    i386(0xf7) {
        _adv;
        _chr(_d_modrm(0, _GetOperandSize));
        _chr(_m_read_rm(_GetOperandSize));
        switch (vcpuins.data.cr) {
        case 0: /* TEST_RM32_I32 */
            _bb("TEST_RM32_I32");
            _chr(_d_imm(_GetOperandSize));
            _chr(_a_test(vcpuins.data.crm, vcpuins.data.cimm, _GetOperandSize * 8));
            _be;
            break;
        case 1: /* UndefinedOpcode */
            _bb("ModRM_REG(1)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 2: /* NOT_RM32 */
            _bb("NOT_RM32");
            _chr(_a_not(vcpuins.data.crm, _GetOperandSize * 8));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 3: /* NEG_RM32 */
            _bb("NEG_RM32");
            _chr(_a_neg(vcpuins.data.crm, _GetOperandSize * 8));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 4: /* MUL_RM32 */
            _bb("MUL_RM32");
            _chr(_a_mul(vcpuins.data.crm, _GetOperandSize * 8));
            _be;
            break;
        case 5: /* IMUL_RM32 */
            _bb("IMUL_RM32");
            _chr(_a_imul(vcpuins.data.crm, _GetOperandSize * 8));
            _be;
            break;
        case 6: /* DIV_RM32 */
            _bb("DIV_RM32");
            _chr(_a_div(vcpuins.data.crm, _GetOperandSize * 8));
            _be;
            break;
        case 7: /* IDIV_RM32 */
            _bb("IDIV_RM32");
            _chr(_a_idiv(vcpuins.data.crm, _GetOperandSize * 8));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_d_modrm(0, 2));
        _chr(_m_read_rm(2));
        switch (vcpuins.data.cr) {
        case 0: /* TEST_RM16_I16 */
            _bb("TEST_RM16_I16");
            _chr(_d_imm(2));
            _chr(_a_test(vcpuins.data.crm, vcpuins.data.cimm, 16));
            _be;
            break;
        case 1: /* UndefinedOpcode */
            _bb("ModRM_REG(1)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 2: /* NOT_RM16 */
            _bb("NOT_RM16");
            _chr(_a_not(vcpuins.data.crm, 16));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 3: /* NEG_RM16 */
            _bb("NEG_RM16");
            _chr(_a_neg(vcpuins.data.crm, 16));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 4: /* MUL_RM16 */
            _bb("MUL_RM16");
            _chr(_a_mul(vcpuins.data.crm, 16));
            _be;
            break;
        case 5: /* IMUL_RM16 */
            _bb("IMUL_RM16");
            _chr(_a_imul(vcpuins.data.crm, 16));
            _be;
            break;
        case 6: /* DIV_RM16 */
            _bb("DIV_RM16");
            _chr(_a_div(vcpuins.data.crm, 16));
            _be;
            break;
        case 7: /* IDIV_RM16 */
            _bb("IDIV_RM16");
            _chr(_a_idiv(vcpuins.data.crm, 16));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    _ce;
}
static void CLC() {
    _cb("CLC");
    i386(0xf8) {
        _adv;
        _ClrEFLAGS_CF;
    }
    else {
        vcpu.data.ip++;
        _ClrEFLAGS_CF;
    }
    _ce;
}
static void STC() {
    _cb("STC");
    i386(0xf9) {
        _adv;
        _SetEFLAGS_CF;
    }
    else {
        vcpu.data.ip++;
        _SetEFLAGS_CF;
    }
    _ce;
}
static void CLI() {
    _cb("CLI");
    i386(0xfa) {
        _adv;
        if (!_GetCR0_PE)
            _ClrEFLAGS_IF;
        else {
            _bb("CR0_PE(1)");
            _new_code_path_;
            if (!_GetEFLAGS_VM) {
                _bb("EFLAGS_VM(0)");
                if (_GetCPL <= (t_nubit8)(_GetEFLAGS_IOPL))
                    _ClrEFLAGS_IF;
                else
                    _chr(_SetExcept_GP(0));
                _be;
            } else {
                _bb("EFLAGS_VM(1)");
                if (_GetEFLAGS_IOPL == 3)
                    _ClrEFLAGS_IF;
                else
                    _chr(_SetExcept_GP(0));
                _be;
            }
            _be;
        }
    }
    else {
        vcpu.data.ip++;
        _ClrEFLAGS_IF;
    }
    _ce;
}
static void STI() {
    _cb("STI");
    i386(0xfb) {
        _adv;
        if (!_GetCR0_PE)
            _SetEFLAGS_IF;
        else {
            _bb("CR0_PE(1)");
            _new_code_path_;
            if (!_GetEFLAGS_VM) {
                _bb("EFLAGS_VM(0)");
                if (_GetCPL <= (t_nubit8)(_GetEFLAGS_IOPL))
                    _SetEFLAGS_IF;
                else
                    _chr(_SetExcept_GP(0));
                _be;
            } else {
                _bb("EFLAGS_VM(1)");
                if (_GetEFLAGS_IOPL == 3)
                    _SetEFLAGS_IF;
                else
                    _chr(_SetExcept_GP(0));
                _be;
            }
            _be;
        }
        vcpuins.data.flagMaskInt = True;
    }
    else {
        vcpu.data.ip++;
        _SetEFLAGS_IF;
    }
    _ce;
}
static void CLD() {
    _cb("CLD");
    i386(0xfc) {
        _adv;
        _ClrEFLAGS_DF;
    }
    else {
        vcpu.data.ip++;
        _ClrEFLAGS_DF;
    }
    _ce;
}
static void STD() {
    _cb("CLD");
    i386(0xfd) {
        _adv;
        _SetEFLAGS_DF;
    }
    else {
        vcpu.data.ip++;
        _SetEFLAGS_DF;
    }
    _ce;
}
static void INS_FE() {
    _cb("INS_FE");
    i386(0xfe) {
        _adv;
    }
    else {
        vcpu.data.ip++;
    }
    _chr(_d_modrm(0, 1));
    _chr(_m_read_rm(1));
    switch (vcpuins.data.cr) {
    case 0: /* INC_RM8 */
        _bb("INC_RM8");
        _chr(_a_inc(vcpuins.data.crm, 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(1));
        _be;
        break;
    case 1: /* DEC_RM8 */
        _bb("DEC_RM8");
        _chr(_a_dec(vcpuins.data.crm, 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(1));
        _be;
        break;
    case 2:
        _bb("vcpuins.data.cr(2)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 3:
        _bb("vcpuins.data.cr(3)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 4:
        _bb("vcpuins.data.cr(4)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 5:
        _bb("vcpuins.data.cr(5)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 6:
        _bb("vcpuins.data.cr(6)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 7:
        _bb("vcpuins.data.cr(7)");
        _chr(UndefinedOpcode());
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void INS_FF() {
    t_nubit8 modrm;
    t_nubit16 newcs;
    t_nubit32 neweip;
    _cb("INS_FF");
    i386(0xff) {
        _adv;
        _chr(_s_read_cs(vcpu.data.eip, GetRef(modrm), 1));
        switch (_GetModRM_REG(modrm)) {
        case 0: /* INC_RM32 */
            _bb("INC_RM32");
            _chr(_d_modrm(0, _GetOperandSize));
            _chr(_m_read_rm(_GetOperandSize));
            _chr(_a_inc(vcpuins.data.crm, _GetOperandSize * 8));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 1: /* DEC_RM32 */
            _bb("DEC_RM32");
            _chr(_d_modrm(0, _GetOperandSize));
            _chr(_m_read_rm(_GetOperandSize));
            _chr(_a_dec(vcpuins.data.crm, _GetOperandSize * 8));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(_GetOperandSize));
            _be;
            break;
        case 2: /* CALL_RM32 */
            _bb("CALL_RM32");
            _chr(_d_modrm(0, _GetOperandSize));
            _chr(_m_read_rm(_GetOperandSize));
            neweip = GetMax32(vcpuins.data.crm);
            _chr(_e_call_near(neweip, _GetOperandSize));
            _be;
            break;
        case 3: /* CALL_M16_32 */
            _bb("CALL_M16_32");
            _chr(_d_modrm(0, _GetOperandSize + 2));
            if (!vcpuins.data.flagMem) {
                _bb("flagMem(0)");
                _chr(UndefinedOpcode());
                _be;
            }
            _chr(_m_read_rm(_GetOperandSize));
            neweip = (t_nubit32)vcpuins.data.crm;
            vcpuins.data.mrm.offset += _GetOperandSize;
            _chr(_m_read_rm(2));
            newcs = GetMax16(vcpuins.data.crm);
            _chr(_e_call_far(newcs, neweip, _GetOperandSize));
            _be;
            break;
        case 4: /* JMP_RM32 */
            _bb("JMP_RM32");
            _chr(_d_modrm(0, _GetOperandSize));
            _chr(_m_read_rm(_GetOperandSize));
            neweip = GetMax32(vcpuins.data.crm);
            _chr(_e_jmp_near(neweip, _GetOperandSize));
            _be;
            break;
        case 5: /* JMP_M16_32 */
            _bb("JMP_M16_32");
            _chr(_d_modrm(0, _GetOperandSize + 2));
            if (!vcpuins.data.flagMem) {
                _bb("flagMem(0)");
                _chr(UndefinedOpcode());
                _be;
            }
            _chr(_m_read_rm(_GetOperandSize));
            neweip = (t_nubit32)vcpuins.data.crm;
            vcpuins.data.mrm.offset += _GetOperandSize;
            _chr(_m_read_rm(2));
            newcs = GetMax16(vcpuins.data.crm);
            _chr(_e_jmp_far(newcs, neweip, _GetOperandSize));
            _be;
            break;
        case 6: /* PUSH_RM32 */
            _bb("PUSH_RM32");
            _chr(_d_modrm(0, _GetOperandSize));
            _chr(_m_read_rm(_GetOperandSize));
            _chr(_e_push(GetRef(vcpuins.data.crm), _GetOperandSize));
            _be;
            break;
        case 7: /* UndefinedOpcode */
            _bb("ModRM_REG(7)");
            _chr(UndefinedOpcode());
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        vcpu.data.ip++;
        _chr(_s_read_cs(vcpu.data.eip, GetRef(modrm), 1));
        switch (_GetModRM_REG(modrm)) {
        case 0: /* INC_RM16 */
            _bb("INC_RM16");
            _chr(_d_modrm(0, 2));
            _chr(_m_read_rm(2));
            _chr(_a_inc(vcpuins.data.crm, 16));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 1: /* DEC_RM16 */
            _bb("DEC_RM16");
            _chr(_d_modrm(0, 2));
            _chr(_m_read_rm(2));
            _chr(_a_dec(vcpuins.data.crm, 16));
            vcpuins.data.crm = vcpuins.data.result;
            _chr(_m_write_rm(2));
            _be;
            break;
        case 2: /* CALL_RM16 */
            _bb("CALL_RM16");
            _chr(_d_modrm(0, 2));
            _chr(_m_read_rm(2));
            neweip = GetMax16(vcpuins.data.crm);
            _chr(_e_call_near(neweip, 2));
            _be;
            break;
        case 3: /* CALL_M16_16 */
            _bb("CALL_M16_16");
            _chr(_d_modrm(0, 4));
            if (!vcpuins.data.flagMem) {
                _bb("flagMem(0)");
                _chr(UndefinedOpcode());
                _be;
            }
            _chr(_m_read_rm(2));
            neweip = (t_nubit16)vcpuins.data.crm;
            vcpuins.data.mrm.offset += 2;
            _chr(_m_read_rm(2));
            newcs = GetMax16(vcpuins.data.crm);
            _chr(_e_call_far(newcs, neweip, 2));
            _be;
            break;
        case 4: /* JMP_RM16 */
            _bb("JMP_RM16");
            _chr(_d_modrm(0, 2));
            _chr(_m_read_rm(2));
            neweip = GetMax16(vcpuins.data.crm);
            _chr(_e_jmp_near(neweip, 2));
            _be;
            break;
        case 5: /* JMP_M16_16 */
            _bb("JMP_M16_16");
            _chr(_d_modrm(0, 4));
            if (!vcpuins.data.flagMem) {
                _bb("flagMem(0)");
                _chr(UndefinedOpcode());
                _be;
            }
            _chr(_m_read_rm(2));
            neweip = (t_nubit16)vcpuins.data.crm;
            vcpuins.data.mrm.offset += 2;
            _chr(_m_read_rm(2));
            newcs = GetMax16(vcpuins.data.crm);
            _chr(_e_jmp_far(newcs, neweip, 2));
            _be;
            break;
        case 6: /* PUSH_RM16 */
            _bb("PUSH_RM16");
            _chr(_d_modrm(0, 2));
            _chr(_m_read_rm(2));
            _chr(_e_push(GetRef(vcpuins.data.crm), 2));
            _be;
            break;
        case 7: /* UndefinedOpcode */
            _bb("ModRM_REG(7)");
            _chr(UndefinedOpcode());
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    _ce;
}

static void _d_modrm_creg() {
    _cb("_d_modrm_creg");
    _chr(_kdf_modrm(0, 4));
    if (vcpuins.data.flagMem) {
        _bb("flagMem(1)");
        _chr(_SetExcept_UD(0));
        _be;
    }
    switch (vcpuins.data.cr) {
    case 0:
        vcpuins.data.rr = (t_vaddrcc)&vcpu.data.cr0;
        vcpuins.data.cr = vcpu.data.cr0;
        break;
    case 1:
        _bb("vcpuins.data.cr(1)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 2:
        vcpuins.data.rr = (t_vaddrcc)&vcpu.data.cr2;
        vcpuins.data.cr = vcpu.data.cr2;
        break;
    case 3:
        vcpuins.data.rr = (t_vaddrcc)&vcpu.data.cr3;
        vcpuins.data.cr = vcpu.data.cr3;
        break;
    case 4:
        _bb("vcpuins.data.cr(4)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 5:
        _bb("vcpuins.data.cr(5)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 6:
        _bb("vcpuins.data.cr(6)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 7:
        _bb("vcpuins.data.cr(7)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void _d_modrm_dreg() {
    _cb("_d_modrm_dreg");
    _chr(_kdf_modrm(0, 4));
    if (vcpuins.data.flagMem) {
        _bb("flagMem(1)");
        _chr(_SetExcept_UD(0));
        _be;
    }
    switch (vcpuins.data.cr) {
    case 0:
        vcpuins.data.rr = (t_vaddrcc)&vcpu.data.dr0;
        vcpuins.data.cr = vcpu.data.dr0;
        break;
    case 1:
        vcpuins.data.rr = (t_vaddrcc)&vcpu.data.dr1;
        vcpuins.data.cr = vcpu.data.dr1;
        break;
    case 2:
        vcpuins.data.rr = (t_vaddrcc)&vcpu.data.dr2;
        vcpuins.data.cr = vcpu.data.dr2;
        break;
    case 3:
        vcpuins.data.rr = (t_vaddrcc)&vcpu.data.dr3;
        vcpuins.data.cr = vcpu.data.dr3;
        break;
    case 4:
        _bb("vcpuins.data.cr(4)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 5:
        _bb("vcpuins.data.cr(5)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 6:
        vcpuins.data.rr = (t_vaddrcc)&vcpu.data.dr6;
        vcpuins.data.cr = vcpu.data.dr6;
        break;
    case 7:
        vcpuins.data.rr = (t_vaddrcc)&vcpu.data.dr7;
        vcpuins.data.cr = vcpu.data.dr7;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void _d_modrm_treg() {
    _cb("_d_modrm_treg");
    _chr(_kdf_modrm(0, 4));
    if (vcpuins.data.flagMem) {
        _bb("flagMem(1)");
        _chr(_SetExcept_UD(0));
        _be;
    }
    switch (vcpuins.data.cr) {
    case 0:
        _bb("vcpuins.data.cr(0)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 1:
        _bb("vcpuins.data.cr(1)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 2:
        _bb("vcpuins.data.cr(2)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 3:
        _bb("vcpuins.data.cr(3)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 4:
        _bb("vcpuins.data.cr(4)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 5:
        _bb("vcpuins.data.cr(5)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 6:
        vcpuins.data.rr = (t_vaddrcc)&vcpu.data.tr6;
        vcpuins.data.cr = vcpu.data.tr6;
        break;
    case 7:
        vcpuins.data.rr = (t_vaddrcc)&vcpu.data.tr7;
        vcpuins.data.cr = vcpu.data.tr7;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void _d_bit_rmimm(t_nubit8 regbyte, t_nubit8 rmbyte, t_bool write) {
    /* xrm = actual destination, cimm = (1 << bitoffset) */
    t_nsbit16 bitoff16 = 0;
    t_nsbit32 bitoff32 = 0;
    t_nubit32 bitoperand = 0;
    _cb("_d_bit_rmimm");
    _chr(_kdf_modrm(regbyte, rmbyte));
    if (!regbyte) _chr(_d_imm(1));
    switch (rmbyte) {
    case 2:
        _bb("rmbyte(2)");
        if (vcpuins.data.flagMem && regbyte) {
            /* valid for btcc_m16_r16 */
            _bb("flagMem(1),regbyte(1)");
            bitoff16 = (t_nsbit16)vcpuins.data.cr;
            if (bitoff16 >= 0)
                vcpuins.data.mrm.offset += 2 * (bitoff16 / 16);
            else
                vcpuins.data.mrm.offset += 2 * ((bitoff16 - 15) / 16);
            bitoperand = ((t_nubit16)bitoff16) % 16;
            _be;
        } else if (regbyte) {
            bitoperand = vcpuins.data.cr % 16;
        } else {
            bitoperand = (GetMax16(vcpuins.data.cimm) % 16);
        }
        _chr(vcpuins.data.cimm = GetMax16((1 << bitoperand)));
        _be;
        break;
    case 4:
        _bb("rmbyte(4)");
        if (vcpuins.data.flagMem && regbyte) {
            _bb("flagMem(1),regbyte(1)");
            bitoff32 = (t_nsbit32)vcpuins.data.cr;
            if (bitoff32 >= 0)
                vcpuins.data.mrm.offset += 4 * (bitoff32 / 32);
            else
                vcpuins.data.mrm.offset += 4 * ((bitoff32 - 31) / 32);
            bitoperand = ((t_nubit32)bitoff32) % 32;
            _be;
        } else if (regbyte) {
            bitoperand = vcpuins.data.cr % 32;
        } else {
            bitoperand = (GetMax32(vcpuins.data.cimm) % 32);
        }
        _chr(vcpuins.data.cimm = GetMax32((1 << bitoperand)));
        _be;
        break;
    default:
        _bb("rmbyte");
        _chr(_SetExcept_CE(rmbyte));
        _be;
        break;
    }
    _ce;
}

#define SHLD_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SHRD_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)

#define _kac_btcc \
do { \
    switch (bit) { \
    case 16: vcpuins.data.opr1 = GetMax16(cdest);break; \
    case 32: vcpuins.data.opr1 = GetMax32(cdest);break; \
    default: _bb("bit"); \
        _chr(_SetExcept_CE(bit)); \
        _be;break; \
    } \
    vcpuins.data.opr2 = bitoperand; \
    vcpuins.data.result = vcpuins.data.opr1; \
    MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF, GetBit(vcpuins.data.opr1, vcpuins.data.opr2)); \
} while (0)

static void _a_bscc(t_nubit64 csrc, t_nubit8 bit, t_bool forward) {
    t_nubit32 temp;
    _cb("_a_bscc");
    if (forward) temp = 0;
    else temp = bit - 1;
    switch (bit) {
    case 16:
        _bb("bit(16)");
        vcpuins.data.opr1 = GetMax16(csrc);
        if (!vcpuins.data.opr1)
            _SetEFLAGS_ZF;
        else {
            _ClrEFLAGS_ZF;
            while (!GetBit(vcpuins.data.opr1, GetMax64(1 << temp))) {
                if (forward) temp++;
                else temp--;
            }
            vcpuins.data.result = GetMax16(temp);
        }
        _be;
        break;
    case 32:
        _bb("bit(32)");
        vcpuins.data.opr1 = GetMax32(csrc);
        if (!vcpuins.data.opr1)
            _SetEFLAGS_ZF;
        else {
            _ClrEFLAGS_ZF;
            while (!GetBit(vcpuins.data.opr1, GetMax64(1 << temp))) {
                if (forward) temp++;
                else temp--;
            }
            vcpuins.data.result = GetMax32(temp);
        }
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    _ce;
}
static void _a_bt(t_nubit64 cdest, t_nubit32 bitoperand, t_nubit8 bit) {
    _cb("_a_bt");
    _kac_btcc;
    _ce;
}
static void _a_btc(t_nubit64 cdest, t_nubit32 bitoperand, t_nubit8 bit) {
    _cb("_a_btc");
    _kac_btcc;
    MakeBit(vcpuins.data.result, vcpuins.data.opr2, !_GetEFLAGS_CF);
    _ce;
}
static void _a_btr(t_nubit64 cdest, t_nubit32 bitoperand, t_nubit8 bit) {
    _cb("_a_btr");
    _kac_btcc;
    ClrBit(vcpuins.data.result, vcpuins.data.opr2);
    _ce;
}
static void _a_bts(t_nubit64 cdest, t_nubit32 bitoperand, t_nubit8 bit) {
    _cb("_a_bts");
    _kac_btcc;
    SetBit(vcpuins.data.result, vcpuins.data.opr2);
    _ce;
}

static void _a_imul2(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    t_nubit64 temp;
    _cb("_a_imul2");
    switch (bit) {
    case 16:
        _bb("bit(16+16)");
        _new_code_path_;
        vcpuins.data.bit = 16;
        vcpuins.data.opr1 = GetMax16((t_nsbit16)cdest);
        vcpuins.data.opr2 = GetMax16((t_nsbit16)csrc);
        temp = GetMax32((t_nsbit16)vcpuins.data.opr1 * (t_nsbit16)vcpuins.data.opr2);
        vcpuins.data.result = GetMax16(temp);
        if (GetMax32(temp) != GetMax32((t_nsbit16)vcpuins.data.result)) {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        } else {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        _be;
        break;
    case 32:
        _bb("bit(32+32");
        _new_code_path_;
        vcpuins.data.bit = 32;
        vcpuins.data.opr1 = GetMax32((t_nsbit32)cdest);
        vcpuins.data.opr2 = GetMax32((t_nsbit32)csrc);
        temp = GetMax64((t_nsbit32)vcpuins.data.opr1 * (t_nsbit32)vcpuins.data.opr2);
        vcpuins.data.result = GetMax32(temp);
        if (GetMax64(temp) != GetMax64((t_nsbit32)vcpuins.data.result)) {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        } else {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    _ce;
}
static void _a_shld(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 count, t_nubit8 bit) {
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
        vcpuins.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF |
                             VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF);
    } else {
        switch (bit) {
        case 16:
            _bb("bit(16)");
            vcpuins.data.bit = 16;
            vcpuins.data.opr1 = GetMax16(cdest);
            vcpuins.data.opr2 = GetMax16(csrc);
            vcpuins.data.result = vcpuins.data.opr1;
            flagcf = !!GetMSB16(vcpuins.data.result);
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF,
                    GetBit(vcpuins.data.result, GetMax64(1 << (bit - count))));
            for (i = (t_nsbit32)(bit - 1); i >= (t_nsbit32)count; --i) {
                flagbit = GetBit(vcpuins.data.opr1, GetMax64(1 << (i - count)));
                MakeBit(vcpuins.data.result, GetMax64(1 << i), flagbit);
            }
            for (i = (t_nsbit32)(count - 1); i >= 0; --i) {
                flagbit = GetBit(vcpuins.data.opr2, GetMax64(1 << (i - count + bit)));
                MakeBit(vcpuins.data.result, GetMax64(1 << i), flagbit);
            }
            if (count == 1)
                MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                        ((!!GetMSB16(vcpuins.data.result)) ^ flagcf));
            else
                vcpuins.data.udf |= VCPU_EFLAGS_OF;
            _be;
            break;
        case 32:
            _bb("bit(32)");
            vcpuins.data.bit = 32;
            vcpuins.data.opr1 = GetMax32(cdest);
            vcpuins.data.opr2 = GetMax32(csrc);
            vcpuins.data.result = vcpuins.data.opr1;
            flagcf = !!GetMSB32(vcpuins.data.result);
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF,
                    GetBit(vcpuins.data.result, GetMax64(1 << (bit - count))));
            for (i = (t_nsbit32)(bit - 1); i >= (t_nsbit32)count; --i) {
                flagbit = GetBit(vcpuins.data.opr1, GetMax64(1 << (i - count)));
                MakeBit(vcpuins.data.result, GetMax64(1 << i), flagbit);
            }
            for (i = (t_nsbit32)(count - 1); i >= 0; --i) {
                flagbit = GetBit(vcpuins.data.opr2, GetMax64(1 << (i - count + bit)));
                MakeBit(vcpuins.data.result, GetMax64(1 << i), flagbit);
            }
            if (count == 1)
                MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                        ((!!GetMSB32(vcpuins.data.result)) ^ flagcf));
            else
                vcpuins.data.udf |= VCPU_EFLAGS_OF;
            _be;
            break;
        default:
            _bb("bit");
            _chr(_SetExcept_CE(bit));
            _be;
            break;
        }
        _chr(_kaf_set_flags(SHLD_FLAG));
        vcpuins.data.udf |= VCPU_EFLAGS_AF;
    }
    _ce;
}
static void _a_shrd(t_nubit64 cdest, t_nubit64 csrc, t_nubit8 count, t_nubit8 bit) {
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
        vcpuins.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF |
                             VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF);
    } else {
        switch (bit) {
        case 16:
            _bb("bit(16)");
            vcpuins.data.bit = 16;
            vcpuins.data.opr1 = GetMax16(cdest);
            vcpuins.data.opr2 = GetMax16(csrc);
            vcpuins.data.result = vcpuins.data.opr1;
            flagcf = !!GetMSB16(vcpuins.data.result);
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF,
                    GetBit(vcpuins.data.result, GetMax64(1 << (count - 1))));
            for (i = 0; i <= (t_nsbit32)(bit - count - 1); ++i) {
                flagbit = GetBit(vcpuins.data.opr1, GetMax64(1 << (i + count)));
                MakeBit(vcpuins.data.result, GetMax64(1 << i), flagbit);
            }
            for (i = (t_nsbit32)(bit - count); i <= (t_nsbit32)(bit - 1); ++i) {
                flagbit = GetBit(vcpuins.data.opr2, GetMax64(1 << (i + count - bit)));
                MakeBit(vcpuins.data.result, GetMax64(1 << i), flagbit);
            }
            if (count == 1)
                MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                        ((!!GetMSB16(vcpuins.data.result)) ^ flagcf));
            else
                vcpuins.data.udf |= VCPU_EFLAGS_OF;
            _be;
            break;
        case 32:
            _bb("bit(32)");
            vcpuins.data.bit = 32;
            vcpuins.data.opr1 = GetMax32(cdest);
            vcpuins.data.opr2 = GetMax32(csrc);
            vcpuins.data.result = vcpuins.data.opr1;
            flagcf = !!GetMSB32(vcpuins.data.result);
            MakeBit(vcpu.data.eflags, VCPU_EFLAGS_CF,
                    GetBit(vcpuins.data.result, GetMax64(1 << (count - 1))));
            for (i = 0; i <= (t_nsbit32)(bit - count - 1); ++i) {
                flagbit = GetBit(vcpuins.data.opr1, GetMax64(1 << (i + count)));
                MakeBit(vcpuins.data.result, GetMax64(1 << i), flagbit);
            }
            for (i = (t_nsbit32)(bit - count); i <= (t_nsbit32)(bit - 1); ++i) {
                flagbit = GetBit(vcpuins.data.opr2, GetMax64(1 << (i + count - bit)));
                MakeBit(vcpuins.data.result, GetMax64(1 << i), flagbit);
            }
            if (count == 1)
                MakeBit(vcpu.data.eflags, VCPU_EFLAGS_OF,
                        ((!!GetMSB32(vcpuins.data.result)) ^ flagcf));
            else
                vcpuins.data.udf |= VCPU_EFLAGS_OF;
            _be;
            break;
        default:
            _bb("bit");
            _chr(_SetExcept_CE(bit));
            _be;
            break;
        }
        _chr(_kaf_set_flags(SHRD_FLAG));
        vcpuins.data.udf |= VCPU_EFLAGS_AF;
    }
    _ce;
}

static void _m_setcc_rm(t_bool condition) {
    _cb("_m_setcc_rm");
    vcpuins.data.result = GetMax8(!!condition);
    vcpuins.data.crm = vcpuins.data.result;
    _chr(_m_write_rm(1));
    _ce;
}

static void INS_0F_00() {
    t_nubit8 modrm;
    t_nubit64 descriptor;
    _cb("INS_0F_00");
    _adv;
    if (_IsProtected) {
        _bb("Protected");
        _chr(_s_read_cs(vcpu.data.eip, GetRef(modrm), 1));
        switch (_GetModRM_REG(modrm)) {
        case 0: /* SLDT_RM16 */
            _bb("SLDT_RM16");
            _chr(_d_modrm(0, ((_GetModRM_MOD(modrm) != 3) ? 2 : _GetOperandSize)));
            vcpuins.data.crm = GetMax16(vcpu.data.ldtr.selector);
            _chr(_m_write_rm(vcpuins.data.flagMem ? 2 : _GetOperandSize));
            _be;
            break;
        case 1: /* STR_RM16 */
            _bb("STR_RM16");
            _chr(_d_modrm(0, ((_GetModRM_MOD(modrm) != 3) ? 2 : _GetOperandSize)));
            vcpuins.data.crm = GetMax16(vcpu.data.tr.selector);
            _chr(_m_write_rm(vcpuins.data.flagMem ? 2 : _GetOperandSize));
            _be;
            break;
        case 2: /* LLDT_RM16 */
            _bb("LLDT_RM16");
            _chr(_d_modrm(0, 2));
            _chr(_m_read_rm(2));
            _chr(_s_load_ldtr(GetMax16(vcpuins.data.crm)));
            _be;
            break;
        case 3: /* LTR_RM16 */
            _bb("LTR_RM16");
            _chr(_d_modrm(0, 2));
            _chr(_m_read_rm(2));
            _chr(_s_load_tr(GetMax16(vcpuins.data.crm)));
            _be;
            break;
        case 4: /* VERR_RM16 */
            _bb("VERR_RM16");
            _chr(_d_modrm(0, 2));
            _chr(_m_read_rm(2));
            if (_s_check_selector(GetMax16(vcpuins.data.crm))) {
                _ClrEFLAGS_ZF;
            } else {
                _bb("selector(valid)");
                _chr(_s_read_xdt(GetMax16(vcpuins.data.crm), GetRef(descriptor)));
                if (_IsDescSys(descriptor) ||
                        (!_IsDescCodeConform(descriptor) &&
                         (_GetCPL > _GetDesc_DPL(descriptor) ||
                          _GetSelector_RPL(GetMax16(vcpuins.data.crm)) > _GetDesc_DPL(descriptor)))) {
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
            _be;
            break;
        case 5: /* VERW_RM16 */
            _bb("VERW_RM16");
            _chr(_d_modrm(0, 2));
            _chr(_m_read_rm(2));
            if (_s_check_selector(GetMax16(vcpuins.data.crm))) {
                _ClrEFLAGS_ZF;
            } else {
                _bb("selector(valid)");
                _chr(_s_read_xdt(GetMax16(vcpuins.data.crm), GetRef(descriptor)));
                if (_IsDescSys(descriptor) ||
                        (!_IsDescCodeConform(descriptor) &&
                         (_GetCPL > _GetDesc_DPL(descriptor) ||
                          _GetSelector_RPL(GetMax16(vcpuins.data.crm)) > _GetDesc_DPL(descriptor)))) {
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
            _be;
            break;
        case 6:
            _bb("ModRM_REG(6)");
            _chr(UndefinedOpcode());
            _be;
            break;
        case 7:
            _bb("ModRM_REG(7)");
            _chr(UndefinedOpcode());
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
        _be;
    } else {
        _bb("!Protected");
        _chr(UndefinedOpcode());
        _be;
    }
    _ce;
}
static void INS_0F_01() {
    t_nubit8 modrm;
    t_nubit16 limit;
    t_nubit32 base;
    _cb("INS_0F_01");
    _adv;
    _chr(_s_read_cs(vcpu.data.eip, GetRef(modrm), 1));
    switch (_GetModRM_REG(modrm)) {
    case 0: /* SGDT_M32_16 */
        _bb("SGDT_M32_16");
        _chr(_d_modrm(0, 6));
        if (!vcpuins.data.flagMem) {
            _bb("flagMem(0)");
            _chr(UndefinedOpcode());
            _be;
        }
        vcpuins.data.crm = vcpu.data.gdtr.limit;
        _chr(_m_write_rm(2));
        vcpuins.data.mrm.offset += 2;
        switch (_GetOperandSize) {
        case 2:
            vcpuins.data.crm = GetMax24(vcpu.data.gdtr.base);
            break;
        case 4:
            vcpuins.data.crm = GetMax32(vcpu.data.gdtr.base);
            break;
        default:
            _impossible_r_;
            break;
        }
        _chr(_m_write_rm(4));
        _be;
        break;
    case 1: /* SIDT_M32_16 */
        _bb("SIDT_M32_16");
        _chr(_d_modrm(0, 6));
        if (!vcpuins.data.flagMem) {
            _bb("flagMem(0)");
            _chr(UndefinedOpcode());
            _be;
        }
        vcpuins.data.crm = vcpu.data.idtr.limit;
        _chr(_m_write_rm(2));
        vcpuins.data.mrm.offset += 2;
        switch (_GetOperandSize) {
        case 2:
            vcpuins.data.crm = GetMax24(vcpu.data.idtr.base);
            break;
        case 4:
            vcpuins.data.crm = GetMax32(vcpu.data.idtr.base);
            break;
        default:
            _impossible_r_;
            break;
        }
        _chr(_m_write_rm(4));
        _be;
        break;
    case 2: /* LGDT_M32_16 */
        _bb("LGDT_M32_16");
        _chr(_d_modrm(0, 6));
        if (!vcpuins.data.flagMem) {
            _bb("flagMem(0)");
            _chr(UndefinedOpcode());
            _be;
        }
        _chr(_m_read_rm(2));
        limit = GetMax16(vcpuins.data.crm);
        vcpuins.data.mrm.offset += 2;
        _chr(_m_read_rm(4));
        switch (_GetOperandSize) {
        case 2:
            base = GetMax24(vcpuins.data.crm);
            break;
        case 4:
            base = GetMax32(vcpuins.data.crm);
            break;
        default:
            _impossible_r_;
            break;
        }
        /* PRINTF("LGDT_M32_16: executed at L%08X, read base=%08X, limit=%04X\n",
                vcpuins.data.linear, base, limit); */
        _chr(_s_load_gdtr(base, limit, _GetOperandSize));
        _be;
        break;
    case 3: /* LIDT_M32_16 */
        _bb("LIDT_M32_16");
        _chr(_d_modrm(0, 6));
        if (!vcpuins.data.flagMem) {
            _bb("flagMem(0)");
            _chr(UndefinedOpcode());
            _be;
        }
        _chr(_m_read_rm(2));
        limit = GetMax16(vcpuins.data.crm);
        vcpuins.data.mrm.offset += 2;
        _chr(_m_read_rm(4));
        switch (_GetOperandSize) {
        case 2:
            base = GetMax24(vcpuins.data.crm);
            break;
        case 4:
            base = GetMax32(vcpuins.data.crm);
            break;
        default:
            _impossible_r_;
            break;
        }
        /* PRINTF("LIDT_M32_16: executed at L%08X, read base=%08X, limit=%04X\n",
                 vcpuins.data.linear, base, limit); */
        _chr(_s_load_idtr(base, limit, _GetOperandSize));
        _be;
        break;
    case 4: /* SMSW_RM16 */
        _bb("SMSW_RM16");
        _chr(_d_modrm(0, ((_GetModRM_MOD(modrm) == 3) ? _GetOperandSize : 2)));
        vcpuins.data.crm = GetMax16(vcpu.data.cr0);
        if (_GetOperandSize == 4 && !vcpuins.data.flagMem)
            _chr(_m_write_rm(4));
        else
            _chr(_m_write_rm(2));
        _be;
        break;
    case 5:
        _bb("ModRM_REG(5)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 6: /* LMSW_RM16 */
        _bb("LMSW_RM16");
        _chr(_d_modrm(0, 2));
        _chr(_m_read_rm(2));
        _chr(_s_load_cr0_msw(GetMax16(vcpuins.data.crm)));
        _be;
        break;
    case 7:
        _bb("ModRM_REG(7)");
        _chr(UndefinedOpcode());
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void LAR_R32_RM32() {
    t_nubit16 selector;
    t_nubit64 descriptor;
    _cb("LAR_R32_RM32");
    _adv;
    if (_IsProtected) {
        _bb("Protected(1)");
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(2));
        selector = GetMax16(vcpuins.data.crm);
        if (_s_check_selector(selector)) {
            _ClrEFLAGS_ZF;
        } else {
            _bb("selector(valid)");
            _chr(_s_read_xdt(selector, GetRef(descriptor)));
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
                case 2:
                    _bb("OperandSize(2)");
                    descriptor = (GetMax16(descriptor >> 32) & 0xff00);
                    _chr(_m_write_ref(vcpuins.data.rr, GetRef(descriptor), 2));
                    _be;
                    break;
                case 4:
                    _bb("OperandSize(4)");
                    descriptor = (GetMax32(descriptor >> 32) & 0x00ffff00);
                    _chr(_m_write_ref(vcpuins.data.rr, GetRef(descriptor), 4));
                    _be;
                    break;
                default:
                    _impossible_r_;
                    break;
                }
                _be;
            }
            _be;
        }
        _be;
    } else {
        _bb("Protected(0)");
        _chr(UndefinedOpcode());
        _be;
    }
    _ce;
}
static void LSL_R32_RM32() {
    t_nubit16 selector;
    t_nubit32 limit;
    t_nubit64 descriptor;
    _cb("LSL_R32_RM32");
    _adv;
    if (_IsProtected) {
        _bb("Protected(1)");
        _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(2));
        selector = GetMax16(vcpuins.data.crm);
        if (_s_check_selector(selector)) {
            _ClrEFLAGS_ZF;
        } else {
            _bb("selector(valid)");
            _chr(_s_read_xdt(selector, GetRef(descriptor)));
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
                case 2:
                    _bb("OperandSize(2)");
                    _chr(_m_write_ref(vcpuins.data.rr, GetRef(limit), 2));
                    _be;
                    break;
                case 4:
                    _bb("OperandSize(4)");
                    _chr(_m_write_ref(vcpuins.data.rr, GetRef(limit), 4));
                    _be;
                    break;
                default:
                    _impossible_r_;
                    break;
                }
                _be;
            }
            _be;
        }
        _be;
    } else {
        _bb("Protected(0)");
        _chr(UndefinedOpcode());
        _be;
    }
    _ce;
}
static void CLTS() {
    _cb("CLTS");
    _new_code_path_;
    _adv;
    if (!_GetCR0_PE) {
        _ClrCR0_TS;
    } else {
        _bb("CR0_PE(1)");
        if (_GetCPL > 0)
            _chr(_SetExcept_GP(0));
        else
            _ClrCR0_TS;
        _be;
    }
    _ce;
}
_______todo WBINVD() {
    _cb("WBINVD");
    UndefinedOpcode();
    vcpuins.data.flagIgnore = True;
    _ce;
}
static void MOV_R32_CR() {
    _cb("MOV_R32_CR");
    _adv;
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    _chr(_d_modrm_creg());
    _chr(_m_write_ref(vcpuins.data.rrm, GetRef(vcpuins.data.cr), 4));
    _ce;
}
static void MOV_R32_DR() {
    _cb("MOV_R32_DR");
    _new_code_path_;
    _adv;
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    _chr(_d_modrm_dreg());
    _chr(_m_write_ref(vcpuins.data.rrm, GetRef(vcpuins.data.cr), 4));
    _ce;
}
static void MOV_CR_R32() {
    _cb("MOV_CR_R32");
    _adv;
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    _chr(_d_modrm_creg());
    _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.crm), 4));
    /* if (vcpuins.data.rr == (t_vaddrcc)&vcpu.data.cr0) {
        PRINTF("MOV_CR_R32: executed at L%08X, CR0=%08X\n", vcpuins.data.linear, vcpu.data.cr0);
    }
    if (vcpuins.data.rr == (t_vaddrcc)&vcpu.data.cr2) {
        PRINTF("MOV_CR_R32: executed at L%08X, CR2=%08X\n", vcpuins.data.linear, vcpu.data.cr2);
    }
    if (vcpuins.data.rr == (t_vaddrcc)&vcpu.data.cr3) {
        PRINTF("MOV_CR_R32: executed at L%08X, CR3=%08X\n", vcpuins.data.linear, vcpu.data.cr3);
    } */
    _ce;
}
static void MOV_DR_R32() {
    _cb("MOV_DR_R32");
    _new_code_path_;
    _adv;
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    _chr(_d_modrm_dreg());
    _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.crm), 4));
    _ce;
}
static void MOV_R32_TR() {
    _cb("MOV_R32_TR");
    _new_code_path_;
    _adv;
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    _chr(_d_modrm_treg());
    _chr(_m_write_ref(vcpuins.data.rrm, GetRef(vcpuins.data.cr), 4));
    _ce;
}
static void MOV_TR_R32() {
    _cb("MOV_TR_R32");
    _new_code_path_;
    _adv;
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    _chr(_d_modrm_treg());
    _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.crm), 4));
    _ce;
}
_______todo WRMSR() {
    _cb("WRMSR");
    UndefinedOpcode();
    vcpuins.data.flagIgnore = True;
    _ce;
}
_______todo RDMSR() {
    _cb("RDMSR");
    UndefinedOpcode();
    vcpuins.data.flagIgnore = True;
    _ce;
}
static void JO_REL32() {
    _cb("JO_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(_GetOperandSize));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), _GetOperandSize, _GetEFLAGS_OF));
    _ce;
}
static void JNO_REL32() {
    _cb("JNO_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(_GetOperandSize));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), _GetOperandSize, !_GetEFLAGS_OF));
    _ce;
}
static void JC_REL32() {
    _cb("JC_REL32");
    _adv;
    _chr(_d_imm(_GetOperandSize));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), _GetOperandSize, _GetEFLAGS_CF));
    _ce;
}
static void JNC_REL32() {
    _cb("JNC_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(_GetOperandSize));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), _GetOperandSize, !_GetEFLAGS_CF));
    _ce;
}
static void JZ_REL32() {
    _cb("JZ_REL32");
    _adv;
    _chr(_d_imm(_GetOperandSize));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), _GetOperandSize, _GetEFLAGS_ZF));
    _ce;
}
static void JNZ_REL32() {
    _cb("JNZ_REL32");
    _adv;
    _chr(_d_imm(_GetOperandSize));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), _GetOperandSize, !_GetEFLAGS_ZF));
    _ce;
}
static void JNA_REL32() {
    _cb("JNA_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(_GetOperandSize));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), _GetOperandSize,
                (_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    _ce;
}
static void JA_REL32() {
    _cb("JA_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(_GetOperandSize));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), _GetOperandSize,
                !(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    _ce;
}
static void JS_REL32() {
    _cb("JS_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(_GetOperandSize));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), _GetOperandSize, _GetEFLAGS_SF));
    _ce;
}
static void JNS_REL32() {
    _cb("JNS_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(_GetOperandSize));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), _GetOperandSize, !_GetEFLAGS_SF));
    _ce;
}
static void JP_REL32() {
    _cb("JP_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(_GetOperandSize));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), _GetOperandSize, _GetEFLAGS_PF));
    _ce;
}
static void JNP_REL32() {
    _cb("JNP_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(_GetOperandSize));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), _GetOperandSize, !_GetEFLAGS_PF));
    _ce;
}
static void JL_REL32() {
    _cb("JL_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(_GetOperandSize));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), _GetOperandSize, (_GetEFLAGS_SF != _GetEFLAGS_OF)));
    _ce;
}
static void JNL_REL32() {
    _cb("JNL_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(_GetOperandSize));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), _GetOperandSize, (_GetEFLAGS_SF == _GetEFLAGS_OF)));
    _ce;
}
static void JNG_REL32() {
    _cb("JNG_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(_GetOperandSize));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), _GetOperandSize,
                (_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF))));
    _ce;
}
static void JG_REL32() {
    _cb("JG_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(_GetOperandSize));
    _chr(_e_jcc(GetMax32(vcpuins.data.cimm), _GetOperandSize,
                (!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF))));
    _ce;
}
static void SETO_RM8() {
    _cb("SETO_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(0, 1));
    _chr(_m_setcc_rm(_GetEFLAGS_OF));
    _ce;
}
static void SETNO_RM8() {
    _cb("SETO_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(0, 1));
    _chr(_m_setcc_rm(!_GetEFLAGS_OF));
    _ce;
}
static void SETC_RM8() {
    _cb("SETC_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(0, 1));
    _chr(_m_setcc_rm(_GetEFLAGS_CF));
    _ce;
}
static void SETNC_RM8() {
    _cb("SETNC_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(0, 1));
    _chr(_m_setcc_rm(!_GetEFLAGS_CF));
    _ce;
}
static void SETZ_RM8() {
    _cb("SETZ_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(0, 1));
    _chr(_m_setcc_rm(_GetEFLAGS_ZF));
    _ce;
}
static void SETNZ_RM8() {
    _cb("SETNZ_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(0, 1));
    _chr(_m_setcc_rm(!_GetEFLAGS_ZF));
    _ce;
}
static void SETNA_RM8() {
    _cb("SETNA_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(0, 1));
    _chr(_m_setcc_rm(_GetEFLAGS_CF || _GetEFLAGS_ZF));
    _ce;
}
static void SETA_RM8() {
    _cb("SETA_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(0, 1));
    _chr(_m_setcc_rm(!(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    _ce;
}
static void SETS_RM8() {
    _cb("SETS_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(0, 1));
    _chr(_m_setcc_rm(_GetEFLAGS_SF));
    _ce;
}
static void SETNS_RM8() {
    _cb("SETNS_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(0, 1));
    _chr(_m_setcc_rm(!_GetEFLAGS_SF));
    _ce;
}
static void SETP_RM8() {
    _cb("SETP_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(0, 1));
    _chr(_m_setcc_rm(_GetEFLAGS_PF));
    _ce;
}
static void SETNP_RM8() {
    _cb("SETNP_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(0, 1));
    _chr(_m_setcc_rm(!_GetEFLAGS_PF));
    _ce;
}
static void SETL_RM8() {
    _cb("SETL_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(0, 1));
    _chr(_m_setcc_rm(_GetEFLAGS_SF != _GetEFLAGS_OF));
    _ce;
}
static void SETNL_RM8() {
    _cb("SETNL_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(0, 1));
    _chr(_m_setcc_rm(_GetEFLAGS_SF == _GetEFLAGS_OF));
    _ce;
}
static void SETNG_RM8() {
    _cb("SETNG_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(0, 1));
    _chr(_m_setcc_rm(_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF)));
    _ce;
}
static void SETG_RM8() {
    _cb("SETG_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(0, 1));
    _chr(_m_setcc_rm(!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF)));
    _ce;
}
static void PUSH_FS() {
    t_nubit32 xs_sel;
    _cb("PUSH_FS");
    _adv;
    xs_sel = vcpu.data.fs.selector;
    _chr(_e_push(GetRef(xs_sel), _GetOperandSize));
    _ce;
}
static void POP_FS() {
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    t_nubit32 xs_sel;
    _cb("POP_FS");
    _adv;
    _chr(_e_pop(GetRef(xs_sel), _GetOperandSize));
    _chr(_s_load_fs(GetMax16(xs_sel)));
    _ce;
}
_______todo CPUID() {
    _cb("CPUID");
    UndefinedOpcode();
    vcpuins.data.flagIgnore = True;
    _ce;
}
static void BT_RM32_R32() {
    _cb("BT_RM32_R32");
    _adv;
    _chr(_d_bit_rmimm(_GetOperandSize, _GetOperandSize, 0));
    _chr(_m_read_rm(_GetOperandSize));
    _chr(_a_bt((t_nubit32)vcpuins.data.crm, (t_nubit32)vcpuins.data.cimm, _GetOperandSize * 8));
    _ce;
}
static void SHLD_RM32_R32_I8() {
    _cb("SHLD_RM32_R32_I8");
    _adv;
    _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
    _chr(_m_read_rm(_GetOperandSize));
    _chr(_d_imm(1));
    _chr(_a_shld(vcpuins.data.crm, vcpuins.data.cr, GetMax8(vcpuins.data.cimm), _GetOperandSize * 8));
    vcpuins.data.crm = vcpuins.data.result;
    _chr(_m_write_rm(_GetOperandSize));
    _ce;
}
static void SHLD_RM32_R32_CL() {
    _cb("SHLD_RM32_R32_CL");
    _adv;
    _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
    _chr(_m_read_rm(_GetOperandSize));
    _chr(_a_shld(vcpuins.data.crm, vcpuins.data.cr, vcpu.data.cl, _GetOperandSize * 8));
    vcpuins.data.crm = vcpuins.data.result;
    _chr(_m_write_rm(_GetOperandSize));
    _ce;
}
static void PUSH_GS() {
    t_nubit32 xs_sel;
    _cb("PUSH_GS");
    _adv;
    xs_sel = vcpu.data.gs.selector;
    _chr(_e_push(GetRef(xs_sel), _GetOperandSize));
    _ce;
}
static void POP_GS() {
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    t_nubit32 xs_sel;
    _cb("POP_GS");
    _adv;
    _chr(_e_pop(GetRef(xs_sel), _GetOperandSize));
    _chr(_s_load_gs(GetMax16(xs_sel)));
    _ce;
}
_______todo RSM() {
    _cb("RSM");
    UndefinedOpcode();
    vcpuins.data.flagIgnore = True;
    _ce;
}
static void BTS_RM32_R32() {
    _cb("BTS_RM32_R32");
    _adv;
    _chr(_d_bit_rmimm(_GetOperandSize, _GetOperandSize, 1));
    _chr(_m_read_rm(_GetOperandSize));
    _chr(_a_bts((t_nubit32)vcpuins.data.crm, (t_nubit32)vcpuins.data.cimm, _GetOperandSize * 8));
    vcpuins.data.crm = vcpuins.data.result;
    _chr(_m_write_rm(_GetOperandSize));
    _ce;
}
static void SHRD_RM32_R32_I8() {
    _cb("SHRD_RM32_R32_I8");
    _adv;
    _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
    _chr(_m_read_rm(_GetOperandSize));
    _chr(_d_imm(1));
    _chr(_a_shrd(vcpuins.data.crm, vcpuins.data.cr, GetMax8(vcpuins.data.cimm), _GetOperandSize * 8));
    vcpuins.data.crm = vcpuins.data.result;
    _chr(_m_write_rm(_GetOperandSize));
    _ce;
}
static void SHRD_RM32_R32_CL() {
    _cb("SHRD_RM32_R32_CL");
    _adv;
    _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
    _chr(_m_read_rm(_GetOperandSize));
    _chr(_a_shrd(vcpuins.data.crm, vcpuins.data.cr, vcpu.data.cl, _GetOperandSize * 8));
    vcpuins.data.crm = vcpuins.data.result;
    _chr(_m_write_rm(_GetOperandSize));
    _ce;
}
static void IMUL_R32_RM32() {
    _cb("IMUL_R32_RM32");
    _adv;
    _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
    _chr(_m_read_rm(_GetOperandSize));
    _chr(_a_imul2(vcpuins.data.cr, vcpuins.data.crm, _GetOperandSize * 8));
    _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), _GetOperandSize));
    _ce;
}
static void LSS_R32_M16_32() {
    t_nubit16 selector;
    t_nubit32 offset;
    _cb("LSS_R32_M16_32");
    _adv;
    _chr(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
    if (!vcpuins.data.flagMem) {
        _bb("flagMem(0)");
        _chr(UndefinedOpcode());
        _be;
    }
    _chr(_m_read_rm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        offset = GetMax16(vcpuins.data.crm);
        break;
    case 4:
        offset = GetMax32(vcpuins.data.crm);
        break;
    default:
        _impossible_r_;
        break;
    }
    vcpuins.data.mrm.offset += _GetOperandSize;
    _chr(_m_read_rm(2));
    selector = GetMax16(vcpuins.data.crm);
    _chr(_e_load_far(&vcpu.data.ss, vcpuins.data.rr, selector, offset, _GetOperandSize));
    _ce;
}
static void BTR_RM32_R32() {
    _cb("BTR_RM32_R32");
    _adv;
    _chr(_d_bit_rmimm(_GetOperandSize, _GetOperandSize, 1));
    _chr(_m_read_rm(_GetOperandSize));
    _chr(_a_btr((t_nubit32)vcpuins.data.crm, (t_nubit32)vcpuins.data.cimm, _GetOperandSize * 8));
    vcpuins.data.crm = vcpuins.data.result;
    _chr(_m_write_rm(_GetOperandSize));
    _ce;
}
static void LFS_R32_M16_32() {
    t_nubit16 selector;
    t_nubit32 offset;
    _cb("LFS_R32_M16_32");
    _adv;
    _chr(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
    if (!vcpuins.data.flagMem) {
        _bb("flagMem(0)");
        _chr(UndefinedOpcode());
        _be;
    }
    _chr(_m_read_rm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        offset = GetMax16(vcpuins.data.crm);
        break;
    case 4:
        offset = GetMax32(vcpuins.data.crm);
        break;
    default:
        _impossible_r_;
        break;
    }
    vcpuins.data.mrm.offset += _GetOperandSize;
    _chr(_m_read_rm(2));
    selector = GetMax16(vcpuins.data.crm);
    _chr(_e_load_far(&vcpu.data.fs, vcpuins.data.rr, selector, offset, _GetOperandSize));
    _ce;
}
static void LGS_R32_M16_32() {
    t_nubit16 selector;
    t_nubit32 offset;
    _cb("LGS_R32_M16_32");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(_GetOperandSize, _GetOperandSize + 2));
    if (!vcpuins.data.flagMem) {
        _bb("flagMem(0)");
        _chr(UndefinedOpcode());
        _be;
    }
    _chr(_m_read_rm(_GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        offset = GetMax16(vcpuins.data.crm);
        break;
    case 4:
        offset = GetMax32(vcpuins.data.crm);
        break;
    default:
        _impossible_r_;
        break;
    }
    vcpuins.data.mrm.offset += _GetOperandSize;
    _chr(_m_read_rm(2));
    selector = GetMax16(vcpuins.data.crm);
    _chr(_e_load_far(&vcpu.data.gs, vcpuins.data.rr, selector, offset, _GetOperandSize));
    _ce;
}
static void MOVZX_R32_RM8() {
    _cb("MOVZX_R32_RM8");
    _adv;
    _chr(_d_modrm(_GetOperandSize, 1));
    _chr(_m_read_rm(1));
    vcpuins.data.crm = (t_nubit8)vcpuins.data.crm;
    _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.crm), _GetOperandSize));
    _ce;
}
static void MOVZX_R32_RM16() {
    _cb("MOVZX_R32_RM16");
    _adv;
    _chr(_d_modrm(4, 2));
    _chr(_m_read_rm(2));
    vcpuins.data.crm = (t_nubit16)vcpuins.data.crm;
    _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.crm), 4));
    _ce;
}
static void INS_0F_BA() {
    t_bool write = 0;
    t_nubit8 modrm = 0x00;
    _cb("INS_0F_BA");
    _new_code_path_;
    _adv;
    _chr(_s_read_cs(vcpu.data.eip, GetRef(modrm), 1));
    if (_GetModRM_REG(modrm) == 4) write = 0;
    else write = 1;
    _chr(_d_bit_rmimm(0, _GetOperandSize, write));
    _chr(_m_read_rm(_GetOperandSize));
    switch (vcpuins.data.cr) {
    case 0:
        _bb("vcpuins.data.cr(0)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 1:
        _bb("vcpuins.data.cr(1)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 2:
        _bb("vcpuins.data.cr(2)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 3:
        _bb("vcpuins.data.cr(3)");
        _chr(UndefinedOpcode());
        _be;
        break;
    case 4: /* BT_RM32_I8 */
        _bb("BT_RM32_I8");
        _chr(_a_bt((t_nubit32)vcpuins.data.crm, (t_nubit32)vcpuins.data.cimm, _GetOperandSize * 8));
        _be;
        break;
    case 5: /* BTS_RM32_I8 */
        _bb("BTS_RM32_I8");
        _chr(_a_bts((t_nubit32)vcpuins.data.crm, (t_nubit32)vcpuins.data.cimm, _GetOperandSize * 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(_GetOperandSize));
        _be;
        break;
    case 6: /* BTR_RM32_I8 */
        _bb("BTR_RM32_I8");
        _chr(_a_btr((t_nubit32)vcpuins.data.crm, (t_nubit32)vcpuins.data.cimm, _GetOperandSize * 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(_GetOperandSize));
        _be;
        break;
    case 7: /* BTC_RM32_I8 */
        _bb("BTC_RM32_I8");
        _chr(_a_btc((t_nubit32)vcpuins.data.crm, (t_nubit32)vcpuins.data.cimm, _GetOperandSize * 8));
        vcpuins.data.crm = vcpuins.data.result;
        _chr(_m_write_rm(_GetOperandSize));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void BTC_RM32_R32() {
    _cb("BTC_RM32_R32");
    _new_code_path_;
    _adv;
    _chr(_d_bit_rmimm(_GetOperandSize, _GetOperandSize, 1));
    _chr(_m_read_rm(_GetOperandSize));
    _chr(_a_btc((t_nubit32)vcpuins.data.crm, (t_nubit32)vcpuins.data.cimm, _GetOperandSize * 8));
    vcpuins.data.crm = vcpuins.data.result;
    _chr(_m_write_rm(_GetOperandSize));
    _ce;
}
static void BSF_R32_RM32() {
    _cb("BSF_R32_RM32");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
    _chr(_m_read_rm(_GetOperandSize));
    _chr(_a_bscc(vcpuins.data.crm, _GetOperandSize * 8, 1));
    if (!_GetEFLAGS_ZF) {
        _bb("EFLAGS_ZF(0)");
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), _GetOperandSize));
        _be;
    }
    _ce;
}
static void BSR_R32_RM32() {
    _cb("BSR_R32_RM32");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(_GetOperandSize, _GetOperandSize));
    _chr(_m_read_rm(_GetOperandSize));
    _chr(_a_bscc(vcpuins.data.crm, _GetOperandSize * 8, 0));
    if (!_GetEFLAGS_ZF) {
        _bb("EFLAGS_ZF(0)");
        _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.result), _GetOperandSize));
        _be;
    }
    _ce;
}
static void MOVSX_R32_RM8() {
    _cb("MOVSX_R32_RM8");
    _adv;
    _chr(_d_modrm(_GetOperandSize, 1));
    _chr(_m_read_rm(1));
    vcpuins.data.crm = (t_nsbit8)vcpuins.data.crm;
    _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.crm), _GetOperandSize));
    _ce;
}
static void MOVSX_R32_RM16() {
    _cb("MOVSX_R32_RM16");
    _adv;
    _chr(_d_modrm(4, 2));
    _chr(_m_read_rm(2));
    vcpuins.data.crm = (t_nsbit16)vcpuins.data.crm;
    _chr(_m_write_ref(vcpuins.data.rr, GetRef(vcpuins.data.crm), 4));
    _ce;
}

static void ExecInit() {
    vcpuins.data.flagIgnore = False;
    vcpuins.data.msize = 0;
    vcpuins.data.reccs = vcpu.data.cs.selector;
    vcpuins.data.receip = vcpu.data.eip;
    vcpuins.data.linear = vcpu.data.cs.base + vcpu.data.eip;
    if (vcpuinsReadLinear(vcpuins.data.linear, (t_vaddrcc) vcpuins.data.opcodes, 15)) {
        vcpuins.data.oplen = 0;
    } else {
        vcpuins.data.oplen = 15;
    }

    vcpuins.data.flagLock = False;
    vcpuins.data.oldcpu = vcpu;
    vcpuins.data.roverds = &vcpu.data.ds;
    vcpuins.data.roverss = &vcpu.data.ss;
    vcpuins.data.prefix_rep = PREFIX_REP_NONE;
    vcpuins.data.prefix_oprsize = False;
    vcpuins.data.prefix_addrsize = False;
    vcpuins.data.flagMem = False;
    vcpuins.data.flagInsLoop = False;
    vcpuins.data.flagMaskInt = False;
    vcpuins.data.bit = 0;
    vcpuins.data.opr1 = 0;
    vcpuins.data.opr2 = 0;
    vcpuins.data.result = 0;
    vcpuins.data.udf = Zero32;
    vcpuins.data.mrm.rsreg = NULL;
    vcpuins.data.mrm.offset = Zero32;
    vcpuins.data.except = Zero32;
    vcpuins.data.excode = Zero32;
#if VCPUINS_TRACE == 1
    utilsTraceInit(&trace);
#endif
}
static void ExecFinal() {
    if (vcpuins.data.flagInsLoop) {
        vcpu.data.cs = vcpuins.data.oldcpu.data.cs;
        vcpu.data.eip = vcpuins.data.oldcpu.data.eip;
    }
#if VCPUINS_TRACE == 1
    if (trace.callCount && !vcpuins.data.except) _SetExcept_CE(trace.cid);
    utilsTraceFinal(&trace);
#endif
    if (vcpuins.data.except) {
        vcpu = vcpuins.data.oldcpu;
        if (GetBit(vcpuins.data.except, VCPUINS_EXCEPT_GP)) {
            ExecInit();
            ClrBit(vcpuins.data.except, VCPUINS_EXCEPT_GP);
            _e_except_n(0x0d, _GetOperandSize);
        }
        deviceStop();
    }
}
static void ExecIns() {
    t_nubit8 opcode = 0;
    ExecInit();
    do {
        _cb("ExecIns");
        _chb(_s_read_cs(vcpu.data.eip, GetRef(opcode), 1));
        _chb(ExecFun(vcpuins.connect.insTable[opcode]));
        _chb(_s_test_eip());
        _chb(_s_test_esp());
        _ce;
    } while (_kdf_check_prefix(opcode));
    if (vcpuins.data.flagWE && vcpuins.data.weLinear == vcpuins.data.linear) {
        PRINTF("Watch point caught at L%08x: EXECUTED\n", vcpuins.data.linear);
        /* printCpuReg(); */
        deviceStop();
    }
    ExecFinal();
}
static void ExecInt() {
    t_nubit8 intr = 0x00;
    /* hardware interrupt handler */
    if (vcpuins.data.flagMaskInt)
        return;
    if (!vcpu.data.flagMaskNMI && vcpu.data.flagNMI) {
        vcpu.data.flagHalt = False;
        vcpu.data.flagNMI = False;
        ExecInit();
        _e_intr_n(0x02, _GetOperandSize);
        ExecFinal();
    }
    if (_GetEFLAGS_IF && vpicScanINTR()) {
        vcpu.data.flagHalt = False;
        intr = vpicGetINTR();
        ExecInit();
        _e_intr_n(intr, _GetOperandSize);
        ExecFinal();
        vcpuins.data.flagIgnore = True;
    }
    if (_GetEFLAGS_TF) {
        vcpu.data.flagHalt = False;
        ExecInit();
        _e_intr_n(0x01, _GetOperandSize);
        ExecFinal();
    }
}

/* external interface */
t_bool vcpuinsLoadSreg(t_cpu_data_sreg *rsreg, t_nubit16 selector) {
    t_bool fail;
    t_nubit32 oldexcept = vcpuins.data.except;
    vcpuins.data.except = 0;
    _ksa_load_sreg(rsreg, selector);
    fail = !!vcpuins.data.except;
    vcpuins.data.except = oldexcept;
    return fail;
}
t_bool vcpuinsReadLinear(t_nubit32 linear, t_vaddrcc rdata, t_nubit8 byte) {
    t_bool fail;
    t_nubit32 oldexcept = vcpuins.data.except;
    vcpuins.data.except = 0;
    _kma_read_linear(linear, rdata, byte, 0x00, 1);
    fail = !!vcpuins.data.except;
    vcpuins.data.except = oldexcept;
    return fail;
}
t_bool vcpuinsWriteLinear(t_nubit32 linear, t_vaddrcc rdata, t_nubit8 byte) {
    t_bool fail;
    t_nubit32 oldexcept = vcpuins.data.except;
    vcpuins.data.except = 0;
    _kma_write_linear(linear, rdata, byte, 0x00, 1);
    fail = !!vcpuins.data.except;
    vcpuins.data.except = oldexcept;
    return fail;
}

void vcpuinsInit() {
    vcpuins.connect.insTable[0x00] = (t_faddrcc) ADD_RM8_R8;
    vcpuins.connect.insTable[0x01] = (t_faddrcc) ADD_RM32_R32;
    vcpuins.connect.insTable[0x02] = (t_faddrcc) ADD_R8_RM8;
    vcpuins.connect.insTable[0x03] = (t_faddrcc) ADD_R32_RM32;
    vcpuins.connect.insTable[0x04] = (t_faddrcc) ADD_AL_I8;
    vcpuins.connect.insTable[0x05] = (t_faddrcc) ADD_EAX_I32;
    vcpuins.connect.insTable[0x06] = (t_faddrcc) PUSH_ES;
    vcpuins.connect.insTable[0x07] = (t_faddrcc) POP_ES;
    vcpuins.connect.insTable[0x08] = (t_faddrcc) OR_RM8_R8;
    vcpuins.connect.insTable[0x09] = (t_faddrcc) OR_RM32_R32;
    vcpuins.connect.insTable[0x0a] = (t_faddrcc) OR_R8_RM8;
    vcpuins.connect.insTable[0x0b] = (t_faddrcc) OR_R32_RM32;
    vcpuins.connect.insTable[0x0c] = (t_faddrcc) OR_AL_I8;
    vcpuins.connect.insTable[0x0d] = (t_faddrcc) OR_EAX_I32;
    vcpuins.connect.insTable[0x0e] = (t_faddrcc) PUSH_CS;
    vcpuins.connect.insTable[0x0f] = (t_faddrcc) INS_0F;
    vcpuins.connect.insTable[0x10] = (t_faddrcc) ADC_RM8_R8;
    vcpuins.connect.insTable[0x11] = (t_faddrcc) ADC_RM32_R32;
    vcpuins.connect.insTable[0x12] = (t_faddrcc) ADC_R8_RM8;
    vcpuins.connect.insTable[0x13] = (t_faddrcc) ADC_R32_RM32;
    vcpuins.connect.insTable[0x14] = (t_faddrcc) ADC_AL_I8;
    vcpuins.connect.insTable[0x15] = (t_faddrcc) ADC_EAX_I32;
    vcpuins.connect.insTable[0x16] = (t_faddrcc) PUSH_SS;
    vcpuins.connect.insTable[0x17] = (t_faddrcc) POP_SS;
    vcpuins.connect.insTable[0x18] = (t_faddrcc) SBB_RM8_R8;
    vcpuins.connect.insTable[0x19] = (t_faddrcc) SBB_RM32_R32;
    vcpuins.connect.insTable[0x1a] = (t_faddrcc) SBB_R8_RM8;
    vcpuins.connect.insTable[0x1b] = (t_faddrcc) SBB_R32_RM32;
    vcpuins.connect.insTable[0x1c] = (t_faddrcc) SBB_AL_I8;
    vcpuins.connect.insTable[0x1d] = (t_faddrcc) SBB_EAX_I32;
    vcpuins.connect.insTable[0x1e] = (t_faddrcc) PUSH_DS;
    vcpuins.connect.insTable[0x1f] = (t_faddrcc) POP_DS;
    vcpuins.connect.insTable[0x20] = (t_faddrcc) AND_RM8_R8;
    vcpuins.connect.insTable[0x21] = (t_faddrcc) AND_RM32_R32;
    vcpuins.connect.insTable[0x22] = (t_faddrcc) AND_R8_RM8;
    vcpuins.connect.insTable[0x23] = (t_faddrcc) AND_R32_RM32;
    vcpuins.connect.insTable[0x24] = (t_faddrcc) AND_AL_I8;
    vcpuins.connect.insTable[0x25] = (t_faddrcc) AND_EAX_I32;
    vcpuins.connect.insTable[0x26] = (t_faddrcc) PREFIX_ES;
    vcpuins.connect.insTable[0x27] = (t_faddrcc) DAA;
    vcpuins.connect.insTable[0x28] = (t_faddrcc) SUB_RM8_R8;
    vcpuins.connect.insTable[0x29] = (t_faddrcc) SUB_RM32_R32;
    vcpuins.connect.insTable[0x2a] = (t_faddrcc) SUB_R8_RM8;
    vcpuins.connect.insTable[0x2b] = (t_faddrcc) SUB_R32_RM32;
    vcpuins.connect.insTable[0x2c] = (t_faddrcc) SUB_AL_I8;
    vcpuins.connect.insTable[0x2d] = (t_faddrcc) SUB_EAX_I32;
    vcpuins.connect.insTable[0x2e] = (t_faddrcc) PREFIX_CS;
    vcpuins.connect.insTable[0x2f] = (t_faddrcc) DAS;
    vcpuins.connect.insTable[0x30] = (t_faddrcc) XOR_RM8_R8;
    vcpuins.connect.insTable[0x31] = (t_faddrcc) XOR_RM32_R32;
    vcpuins.connect.insTable[0x32] = (t_faddrcc) XOR_R8_RM8;
    vcpuins.connect.insTable[0x33] = (t_faddrcc) XOR_R32_RM32;
    vcpuins.connect.insTable[0x34] = (t_faddrcc) XOR_AL_I8;
    vcpuins.connect.insTable[0x35] = (t_faddrcc) XOR_EAX_I32;
    vcpuins.connect.insTable[0x36] = (t_faddrcc) PREFIX_SS;
    vcpuins.connect.insTable[0x37] = (t_faddrcc) AAA;
    vcpuins.connect.insTable[0x38] = (t_faddrcc) CMP_RM8_R8;
    vcpuins.connect.insTable[0x39] = (t_faddrcc) CMP_RM32_R32;
    vcpuins.connect.insTable[0x3a] = (t_faddrcc) CMP_R8_RM8;
    vcpuins.connect.insTable[0x3b] = (t_faddrcc) CMP_R32_RM32;
    vcpuins.connect.insTable[0x3c] = (t_faddrcc) CMP_AL_I8;
    vcpuins.connect.insTable[0x3d] = (t_faddrcc) CMP_EAX_I32;
    vcpuins.connect.insTable[0x3e] = (t_faddrcc) PREFIX_DS;
    vcpuins.connect.insTable[0x3f] = (t_faddrcc) AAS;
    vcpuins.connect.insTable[0x40] = (t_faddrcc) INC_EAX;
    vcpuins.connect.insTable[0x41] = (t_faddrcc) INC_ECX;
    vcpuins.connect.insTable[0x42] = (t_faddrcc) INC_EDX;
    vcpuins.connect.insTable[0x43] = (t_faddrcc) INC_EBX;
    vcpuins.connect.insTable[0x44] = (t_faddrcc) INC_ESP;
    vcpuins.connect.insTable[0x45] = (t_faddrcc) INC_EBP;
    vcpuins.connect.insTable[0x46] = (t_faddrcc) INC_ESI;
    vcpuins.connect.insTable[0x47] = (t_faddrcc) INC_EDI;
    vcpuins.connect.insTable[0x48] = (t_faddrcc) DEC_EAX;
    vcpuins.connect.insTable[0x49] = (t_faddrcc) DEC_ECX;
    vcpuins.connect.insTable[0x4a] = (t_faddrcc) DEC_EDX;
    vcpuins.connect.insTable[0x4b] = (t_faddrcc) DEC_EBX;
    vcpuins.connect.insTable[0x4c] = (t_faddrcc) DEC_ESP;
    vcpuins.connect.insTable[0x4d] = (t_faddrcc) DEC_EBP;
    vcpuins.connect.insTable[0x4e] = (t_faddrcc) DEC_ESI;
    vcpuins.connect.insTable[0x4f] = (t_faddrcc) DEC_EDI;
    vcpuins.connect.insTable[0x50] = (t_faddrcc) PUSH_EAX;
    vcpuins.connect.insTable[0x51] = (t_faddrcc) PUSH_ECX;
    vcpuins.connect.insTable[0x52] = (t_faddrcc) PUSH_EDX;
    vcpuins.connect.insTable[0x53] = (t_faddrcc) PUSH_EBX;
    vcpuins.connect.insTable[0x54] = (t_faddrcc) PUSH_ESP;
    vcpuins.connect.insTable[0x55] = (t_faddrcc) PUSH_EBP;
    vcpuins.connect.insTable[0x56] = (t_faddrcc) PUSH_ESI;
    vcpuins.connect.insTable[0x57] = (t_faddrcc) PUSH_EDI;
    vcpuins.connect.insTable[0x58] = (t_faddrcc) POP_EAX;
    vcpuins.connect.insTable[0x59] = (t_faddrcc) POP_ECX;
    vcpuins.connect.insTable[0x5a] = (t_faddrcc) POP_EDX;
    vcpuins.connect.insTable[0x5b] = (t_faddrcc) POP_EBX;
    vcpuins.connect.insTable[0x5c] = (t_faddrcc) POP_ESP;
    vcpuins.connect.insTable[0x5d] = (t_faddrcc) POP_EBP;
    vcpuins.connect.insTable[0x5e] = (t_faddrcc) POP_ESI;
    vcpuins.connect.insTable[0x5f] = (t_faddrcc) POP_EDI;
    vcpuins.connect.insTable[0x60] = (t_faddrcc) PUSHA;
    vcpuins.connect.insTable[0x61] = (t_faddrcc) POPA;
    vcpuins.connect.insTable[0x62] = (t_faddrcc) BOUND_R16_M16_16;
    vcpuins.connect.insTable[0x63] = (t_faddrcc) ARPL_RM16_R16;
    vcpuins.connect.insTable[0x64] = (t_faddrcc) PREFIX_FS;
    vcpuins.connect.insTable[0x65] = (t_faddrcc) PREFIX_GS;
    vcpuins.connect.insTable[0x66] = (t_faddrcc) PREFIX_OprSize;
    vcpuins.connect.insTable[0x67] = (t_faddrcc) PREFIX_AddrSize;
    vcpuins.connect.insTable[0x68] = (t_faddrcc) PUSH_I32;
    vcpuins.connect.insTable[0x69] = (t_faddrcc) IMUL_R32_RM32_I32;
    vcpuins.connect.insTable[0x6a] = (t_faddrcc) PUSH_I8;
    vcpuins.connect.insTable[0x6b] = (t_faddrcc) IMUL_R32_RM32_I8;
    vcpuins.connect.insTable[0x6c] = (t_faddrcc) INSB;
    vcpuins.connect.insTable[0x6d] = (t_faddrcc) INSW;
    vcpuins.connect.insTable[0x6e] = (t_faddrcc) OUTSB;
    vcpuins.connect.insTable[0x6f] = (t_faddrcc) OUTSW;
    vcpuins.connect.insTable[0x70] = (t_faddrcc) JO_REL8;
    vcpuins.connect.insTable[0x71] = (t_faddrcc) JNO_REL8;
    vcpuins.connect.insTable[0x72] = (t_faddrcc) JC_REL8;
    vcpuins.connect.insTable[0x73] = (t_faddrcc) JNC_REL8;
    vcpuins.connect.insTable[0x74] = (t_faddrcc) JZ_REL8;
    vcpuins.connect.insTable[0x75] = (t_faddrcc) JNZ_REL8;
    vcpuins.connect.insTable[0x76] = (t_faddrcc) JNA_REL8;
    vcpuins.connect.insTable[0x77] = (t_faddrcc) JA_REL8;
    vcpuins.connect.insTable[0x78] = (t_faddrcc) JS_REL8;
    vcpuins.connect.insTable[0x79] = (t_faddrcc) JNS_REL8;
    vcpuins.connect.insTable[0x7a] = (t_faddrcc) JP_REL8;
    vcpuins.connect.insTable[0x7b] = (t_faddrcc) JNP_REL8;
    vcpuins.connect.insTable[0x7c] = (t_faddrcc) JL_REL8;
    vcpuins.connect.insTable[0x7d] = (t_faddrcc) JNL_REL8;
    vcpuins.connect.insTable[0x7e] = (t_faddrcc) JNG_REL8;
    vcpuins.connect.insTable[0x7f] = (t_faddrcc) JG_REL8;
    vcpuins.connect.insTable[0x80] = (t_faddrcc) INS_80;
    vcpuins.connect.insTable[0x81] = (t_faddrcc) INS_81;
    vcpuins.connect.insTable[0x82] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable[0x83] = (t_faddrcc) INS_83;
    vcpuins.connect.insTable[0x84] = (t_faddrcc) TEST_RM8_R8;
    vcpuins.connect.insTable[0x85] = (t_faddrcc) TEST_RM32_R32;
    vcpuins.connect.insTable[0x86] = (t_faddrcc) XCHG_RM8_R8;
    vcpuins.connect.insTable[0x87] = (t_faddrcc) XCHG_RM32_R32;
    vcpuins.connect.insTable[0x88] = (t_faddrcc) MOV_RM8_R8;
    vcpuins.connect.insTable[0x89] = (t_faddrcc) MOV_RM32_R32;
    vcpuins.connect.insTable[0x8a] = (t_faddrcc) MOV_R8_RM8;
    vcpuins.connect.insTable[0x8b] = (t_faddrcc) MOV_R32_RM32;
    vcpuins.connect.insTable[0x8c] = (t_faddrcc) MOV_RM16_SREG;
    vcpuins.connect.insTable[0x8d] = (t_faddrcc) LEA_R32_M32;
    vcpuins.connect.insTable[0x8e] = (t_faddrcc) MOV_SREG_RM16;
    vcpuins.connect.insTable[0x8f] = (t_faddrcc) INS_8F;
    vcpuins.connect.insTable[0x90] = (t_faddrcc) NOP;
    vcpuins.connect.insTable[0x91] = (t_faddrcc) XCHG_ECX_EAX;
    vcpuins.connect.insTable[0x92] = (t_faddrcc) XCHG_EDX_EAX;
    vcpuins.connect.insTable[0x93] = (t_faddrcc) XCHG_EBX_EAX;
    vcpuins.connect.insTable[0x94] = (t_faddrcc) XCHG_ESP_EAX;
    vcpuins.connect.insTable[0x95] = (t_faddrcc) XCHG_EBP_EAX;
    vcpuins.connect.insTable[0x96] = (t_faddrcc) XCHG_ESI_EAX;
    vcpuins.connect.insTable[0x97] = (t_faddrcc) XCHG_EDI_EAX;
    vcpuins.connect.insTable[0x98] = (t_faddrcc) CBW;
    vcpuins.connect.insTable[0x99] = (t_faddrcc) CWD;
    vcpuins.connect.insTable[0x9a] = (t_faddrcc) CALL_PTR16_32;
    vcpuins.connect.insTable[0x9b] = (t_faddrcc) WAIT;
    vcpuins.connect.insTable[0x9c] = (t_faddrcc) PUSHF;
    vcpuins.connect.insTable[0x9d] = (t_faddrcc) POPF;
    vcpuins.connect.insTable[0x9e] = (t_faddrcc) SAHF;
    vcpuins.connect.insTable[0x9f] = (t_faddrcc) LAHF;
    vcpuins.connect.insTable[0xa0] = (t_faddrcc) MOV_AL_MOFFS8;
    vcpuins.connect.insTable[0xa1] = (t_faddrcc) MOV_EAX_MOFFS32;
    vcpuins.connect.insTable[0xa2] = (t_faddrcc) MOV_MOFFS8_AL;
    vcpuins.connect.insTable[0xa3] = (t_faddrcc) MOV_MOFFS32_EAX;
    vcpuins.connect.insTable[0xa4] = (t_faddrcc) MOVSB;
    vcpuins.connect.insTable[0xa5] = (t_faddrcc) MOVSW;
    vcpuins.connect.insTable[0xa6] = (t_faddrcc) CMPSB;
    vcpuins.connect.insTable[0xa7] = (t_faddrcc) CMPSW;
    vcpuins.connect.insTable[0xa8] = (t_faddrcc) TEST_AL_I8;
    vcpuins.connect.insTable[0xa9] = (t_faddrcc) TEST_EAX_I32;
    vcpuins.connect.insTable[0xaa] = (t_faddrcc) STOSB;
    vcpuins.connect.insTable[0xab] = (t_faddrcc) STOSW;
    vcpuins.connect.insTable[0xac] = (t_faddrcc) LODSB;
    vcpuins.connect.insTable[0xad] = (t_faddrcc) LODSW;
    vcpuins.connect.insTable[0xae] = (t_faddrcc) SCASB;
    vcpuins.connect.insTable[0xaf] = (t_faddrcc) SCASW;
    vcpuins.connect.insTable[0xb0] = (t_faddrcc) MOV_AL_I8;
    vcpuins.connect.insTable[0xb1] = (t_faddrcc) MOV_CL_I8;
    vcpuins.connect.insTable[0xb2] = (t_faddrcc) MOV_DL_I8;
    vcpuins.connect.insTable[0xb3] = (t_faddrcc) MOV_BL_I8;
    vcpuins.connect.insTable[0xb4] = (t_faddrcc) MOV_AH_I8;
    vcpuins.connect.insTable[0xb5] = (t_faddrcc) MOV_CH_I8;
    vcpuins.connect.insTable[0xb6] = (t_faddrcc) MOV_DH_I8;
    vcpuins.connect.insTable[0xb7] = (t_faddrcc) MOV_BH_I8;
    vcpuins.connect.insTable[0xb8] = (t_faddrcc) MOV_EAX_I32;
    vcpuins.connect.insTable[0xb9] = (t_faddrcc) MOV_ECX_I32;
    vcpuins.connect.insTable[0xba] = (t_faddrcc) MOV_EDX_I32;
    vcpuins.connect.insTable[0xbb] = (t_faddrcc) MOV_EBX_I32;
    vcpuins.connect.insTable[0xbc] = (t_faddrcc) MOV_ESP_I32;
    vcpuins.connect.insTable[0xbd] = (t_faddrcc) MOV_EBP_I32;
    vcpuins.connect.insTable[0xbe] = (t_faddrcc) MOV_ESI_I32;
    vcpuins.connect.insTable[0xbf] = (t_faddrcc) MOV_EDI_I32;
    vcpuins.connect.insTable[0xc0] = (t_faddrcc) INS_C0;
    vcpuins.connect.insTable[0xc1] = (t_faddrcc) INS_C1;
    vcpuins.connect.insTable[0xc2] = (t_faddrcc) RET_I16;
    vcpuins.connect.insTable[0xc3] = (t_faddrcc) RET;
    vcpuins.connect.insTable[0xc4] = (t_faddrcc) LES_R32_M16_32;
    vcpuins.connect.insTable[0xc5] = (t_faddrcc) LDS_R32_M16_32;
    vcpuins.connect.insTable[0xc6] = (t_faddrcc) INS_C6;
    vcpuins.connect.insTable[0xc7] = (t_faddrcc) INS_C7;
    vcpuins.connect.insTable[0xc8] = (t_faddrcc) ENTER;
    vcpuins.connect.insTable[0xc9] = (t_faddrcc) LEAVE;
    vcpuins.connect.insTable[0xca] = (t_faddrcc) RETF_I16;
    vcpuins.connect.insTable[0xcb] = (t_faddrcc) RETF;
    vcpuins.connect.insTable[0xcc] = (t_faddrcc) INT3;
    vcpuins.connect.insTable[0xcd] = (t_faddrcc) INT_I8;
    vcpuins.connect.insTable[0xce] = (t_faddrcc) INTO;
    vcpuins.connect.insTable[0xcf] = (t_faddrcc) IRET;
    vcpuins.connect.insTable[0xd0] = (t_faddrcc) INS_D0;
    vcpuins.connect.insTable[0xd1] = (t_faddrcc) INS_D1;
    vcpuins.connect.insTable[0xd2] = (t_faddrcc) INS_D2;
    vcpuins.connect.insTable[0xd3] = (t_faddrcc) INS_D3;
    vcpuins.connect.insTable[0xd4] = (t_faddrcc) AAM;
    vcpuins.connect.insTable[0xd5] = (t_faddrcc) AAD;
    vcpuins.connect.insTable[0xd6] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable[0xd7] = (t_faddrcc) XLAT;
    vcpuins.connect.insTable[0xd8] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable[0xd9] = (t_faddrcc) UndefinedOpcode;
    /* vcpuins.connect.insTable[0xd9] = (t_faddrcc) INS_D9; */
    vcpuins.connect.insTable[0xda] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable[0xdb] = (t_faddrcc) UndefinedOpcode;
    /* vcpuins.connect.insTable[0xdb] = (t_faddrcc) INS_DB; */
    vcpuins.connect.insTable[0xdc] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable[0xdd] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable[0xde] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable[0xdf] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable[0xe0] = (t_faddrcc) LOOPNZ_REL8;
    vcpuins.connect.insTable[0xe1] = (t_faddrcc) LOOPZ_REL8;
    vcpuins.connect.insTable[0xe2] = (t_faddrcc) LOOP_REL8;
    vcpuins.connect.insTable[0xe3] = (t_faddrcc) JCXZ_REL8;
    vcpuins.connect.insTable[0xe4] = (t_faddrcc) IN_AL_I8;
    vcpuins.connect.insTable[0xe5] = (t_faddrcc) IN_EAX_I8;
    vcpuins.connect.insTable[0xe6] = (t_faddrcc) OUT_I8_AL;
    vcpuins.connect.insTable[0xe7] = (t_faddrcc) OUT_I8_EAX;
    vcpuins.connect.insTable[0xe8] = (t_faddrcc) CALL_REL32;
    vcpuins.connect.insTable[0xe9] = (t_faddrcc) JMP_REL32;
    vcpuins.connect.insTable[0xea] = (t_faddrcc) JMP_PTR16_32;
    vcpuins.connect.insTable[0xeb] = (t_faddrcc) JMP_REL8;
    vcpuins.connect.insTable[0xec] = (t_faddrcc) IN_AL_DX;
    vcpuins.connect.insTable[0xed] = (t_faddrcc) IN_EAX_DX;
    vcpuins.connect.insTable[0xee] = (t_faddrcc) OUT_DX_AL;
    vcpuins.connect.insTable[0xef] = (t_faddrcc) OUT_DX_EAX;
    vcpuins.connect.insTable[0xf0] = (t_faddrcc) PREFIX_LOCK;
    vcpuins.connect.insTable[0xf1] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable[0xf2] = (t_faddrcc) PREFIX_REPNZ;
    vcpuins.connect.insTable[0xf3] = (t_faddrcc) PREFIX_REPZ;
    vcpuins.connect.insTable[0xf4] = (t_faddrcc) HLT;
    vcpuins.connect.insTable[0xf5] = (t_faddrcc) CMC;
    vcpuins.connect.insTable[0xf6] = (t_faddrcc) INS_F6;
    vcpuins.connect.insTable[0xf7] = (t_faddrcc) INS_F7;
    vcpuins.connect.insTable[0xf8] = (t_faddrcc) CLC;
    vcpuins.connect.insTable[0xf9] = (t_faddrcc) STC;
    vcpuins.connect.insTable[0xfa] = (t_faddrcc) CLI;
    vcpuins.connect.insTable[0xfb] = (t_faddrcc) STI;
    vcpuins.connect.insTable[0xfc] = (t_faddrcc) CLD;
    vcpuins.connect.insTable[0xfd] = (t_faddrcc) STD;
    vcpuins.connect.insTable[0xfe] = (t_faddrcc) INS_FE;
    vcpuins.connect.insTable[0xff] = (t_faddrcc) INS_FF;
    vcpuins.connect.insTable_0f[0x00] = (t_faddrcc) INS_0F_00;
    vcpuins.connect.insTable_0f[0x01] = (t_faddrcc) INS_0F_01;
    vcpuins.connect.insTable_0f[0x02] = (t_faddrcc) LAR_R32_RM32;
    vcpuins.connect.insTable_0f[0x03] = (t_faddrcc) LSL_R32_RM32;
    vcpuins.connect.insTable_0f[0x04] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x05] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x06] = (t_faddrcc) CLTS;
    vcpuins.connect.insTable_0f[0x07] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x08] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x09] = (t_faddrcc) WBINVD;
    vcpuins.connect.insTable_0f[0x0a] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x0b] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x0c] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x0d] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x0e] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x0f] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x10] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x11] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x12] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x13] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x14] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x15] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x16] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x17] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x18] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x19] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x1a] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x1b] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x1c] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x1d] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x1e] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x1f] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x20] = (t_faddrcc) MOV_R32_CR;
    vcpuins.connect.insTable_0f[0x21] = (t_faddrcc) MOV_R32_DR;
    vcpuins.connect.insTable_0f[0x22] = (t_faddrcc) MOV_CR_R32;
    vcpuins.connect.insTable_0f[0x23] = (t_faddrcc) MOV_DR_R32;
    vcpuins.connect.insTable_0f[0x24] = (t_faddrcc) MOV_R32_TR;
    vcpuins.connect.insTable_0f[0x25] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x26] = (t_faddrcc) MOV_TR_R32;
    vcpuins.connect.insTable_0f[0x27] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x28] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x29] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x2a] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x2b] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x2c] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x2d] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x2e] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x2f] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x30] = (t_faddrcc) WRMSR;
    vcpuins.connect.insTable_0f[0x31] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x32] = (t_faddrcc) RDMSR;
    vcpuins.connect.insTable_0f[0x33] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x34] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x35] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x36] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x37] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x38] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x39] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x3a] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x3b] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x3c] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x3d] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x3e] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x3f] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x40] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x41] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x42] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x43] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x44] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x45] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x46] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x47] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x48] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x49] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x4a] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x4b] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x4c] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x4d] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x4e] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x4f] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x50] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x51] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x52] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x53] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x54] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x55] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x56] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x57] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x58] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x59] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x5a] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x5b] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x5c] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x5d] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x5e] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x5f] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x60] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x61] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x62] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x63] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x64] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x65] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x66] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x67] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x68] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x69] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x6a] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x6b] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x6c] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x6d] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x6e] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x6f] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x70] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x71] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x72] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x73] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x74] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x75] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x76] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x77] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x78] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x79] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x7a] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x7b] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x7c] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x7d] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x7e] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x7f] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0x80] = (t_faddrcc) JO_REL32;
    vcpuins.connect.insTable_0f[0x81] = (t_faddrcc) JNO_REL32;
    vcpuins.connect.insTable_0f[0x82] = (t_faddrcc) JC_REL32;
    vcpuins.connect.insTable_0f[0x83] = (t_faddrcc) JNC_REL32;
    vcpuins.connect.insTable_0f[0x84] = (t_faddrcc) JZ_REL32;
    vcpuins.connect.insTable_0f[0x85] = (t_faddrcc) JNZ_REL32;
    vcpuins.connect.insTable_0f[0x86] = (t_faddrcc) JNA_REL32;
    vcpuins.connect.insTable_0f[0x87] = (t_faddrcc) JA_REL32;
    vcpuins.connect.insTable_0f[0x88] = (t_faddrcc) JS_REL32;
    vcpuins.connect.insTable_0f[0x89] = (t_faddrcc) JNS_REL32;
    vcpuins.connect.insTable_0f[0x8a] = (t_faddrcc) JP_REL32;
    vcpuins.connect.insTable_0f[0x8b] = (t_faddrcc) JNP_REL32;
    vcpuins.connect.insTable_0f[0x8c] = (t_faddrcc) JL_REL32;
    vcpuins.connect.insTable_0f[0x8d] = (t_faddrcc) JNL_REL32;
    vcpuins.connect.insTable_0f[0x8e] = (t_faddrcc) JNG_REL32;
    vcpuins.connect.insTable_0f[0x8f] = (t_faddrcc) JG_REL32;
    vcpuins.connect.insTable_0f[0x90] = (t_faddrcc) SETO_RM8;
    vcpuins.connect.insTable_0f[0x91] = (t_faddrcc) SETNO_RM8;
    vcpuins.connect.insTable_0f[0x92] = (t_faddrcc) SETC_RM8;
    vcpuins.connect.insTable_0f[0x93] = (t_faddrcc) SETNC_RM8;
    vcpuins.connect.insTable_0f[0x94] = (t_faddrcc) SETZ_RM8;
    vcpuins.connect.insTable_0f[0x95] = (t_faddrcc) SETNZ_RM8;
    vcpuins.connect.insTable_0f[0x96] = (t_faddrcc) SETNA_RM8;
    vcpuins.connect.insTable_0f[0x97] = (t_faddrcc) SETA_RM8;
    vcpuins.connect.insTable_0f[0x98] = (t_faddrcc) SETS_RM8;
    vcpuins.connect.insTable_0f[0x99] = (t_faddrcc) SETNS_RM8;
    vcpuins.connect.insTable_0f[0x9a] = (t_faddrcc) SETP_RM8;
    vcpuins.connect.insTable_0f[0x9b] = (t_faddrcc) SETNP_RM8;
    vcpuins.connect.insTable_0f[0x9c] = (t_faddrcc) SETL_RM8;
    vcpuins.connect.insTable_0f[0x9d] = (t_faddrcc) SETNL_RM8;
    vcpuins.connect.insTable_0f[0x9e] = (t_faddrcc) SETNG_RM8;
    vcpuins.connect.insTable_0f[0x9f] = (t_faddrcc) SETG_RM8;
    vcpuins.connect.insTable_0f[0xa0] = (t_faddrcc) PUSH_FS;
    vcpuins.connect.insTable_0f[0xa1] = (t_faddrcc) POP_FS;
    vcpuins.connect.insTable_0f[0xa2] = (t_faddrcc) CPUID;
    vcpuins.connect.insTable_0f[0xa3] = (t_faddrcc) BT_RM32_R32;
    vcpuins.connect.insTable_0f[0xa4] = (t_faddrcc) SHLD_RM32_R32_I8;
    vcpuins.connect.insTable_0f[0xa5] = (t_faddrcc) SHLD_RM32_R32_CL;
    vcpuins.connect.insTable_0f[0xa6] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xa7] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xa8] = (t_faddrcc) PUSH_GS;
    vcpuins.connect.insTable_0f[0xa9] = (t_faddrcc) POP_GS;
    vcpuins.connect.insTable_0f[0xaa] = (t_faddrcc) RSM;
    vcpuins.connect.insTable_0f[0xab] = (t_faddrcc) BTS_RM32_R32;
    vcpuins.connect.insTable_0f[0xac] = (t_faddrcc) SHRD_RM32_R32_I8;
    vcpuins.connect.insTable_0f[0xad] = (t_faddrcc) SHRD_RM32_R32_CL;
    vcpuins.connect.insTable_0f[0xae] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xaf] = (t_faddrcc) IMUL_R32_RM32;
    vcpuins.connect.insTable_0f[0xb0] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xb1] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xb2] = (t_faddrcc) LSS_R32_M16_32;
    vcpuins.connect.insTable_0f[0xb3] = (t_faddrcc) BTR_RM32_R32;
    vcpuins.connect.insTable_0f[0xb4] = (t_faddrcc) LFS_R32_M16_32;
    vcpuins.connect.insTable_0f[0xb5] = (t_faddrcc) LGS_R32_M16_32;
    vcpuins.connect.insTable_0f[0xb6] = (t_faddrcc) MOVZX_R32_RM8;
    vcpuins.connect.insTable_0f[0xb7] = (t_faddrcc) MOVZX_R32_RM16;
    vcpuins.connect.insTable_0f[0xb8] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xb9] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xba] = (t_faddrcc) INS_0F_BA;
    vcpuins.connect.insTable_0f[0xbb] = (t_faddrcc) BTC_RM32_R32;
    vcpuins.connect.insTable_0f[0xbc] = (t_faddrcc) BSF_R32_RM32;
    vcpuins.connect.insTable_0f[0xbd] = (t_faddrcc) BSR_R32_RM32;
    vcpuins.connect.insTable_0f[0xbe] = (t_faddrcc) MOVSX_R32_RM8;
    vcpuins.connect.insTable_0f[0xbf] = (t_faddrcc) MOVSX_R32_RM16;
    vcpuins.connect.insTable_0f[0xc0] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xc1] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xc2] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xc3] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xc4] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xc5] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xc6] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xc7] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xc8] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xc9] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xca] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xcb] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xcc] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xcd] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xce] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xcf] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xd0] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xd1] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xd2] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xd3] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xd4] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xd5] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xd6] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xd7] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xd8] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xd9] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xda] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xdb] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xdc] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xdd] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xde] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xdf] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xe0] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xe1] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xe2] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xe3] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xe4] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xe5] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xe6] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xe7] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xe8] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xe9] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xea] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xeb] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xec] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xed] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xee] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xef] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xf0] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xf1] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xf2] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xf3] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xf4] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xf5] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xf6] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xf7] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xf8] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xf9] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xfa] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xfb] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xfc] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xfd] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xfe] = (t_faddrcc) UndefinedOpcode;
    vcpuins.connect.insTable_0f[0xff] = (t_faddrcc) UndefinedOpcode;
}
void vcpuinsReset() {
    MEMSET((void *)(&vcpuins.data), Zero8, sizeof(t_cpuins_data));
}
void vcpuinsRefresh() {
    if (!vcpu.data.flagHalt) {
        ExecIns();
    } else {
        utilsSleep(1);
    }
    ExecInt();
}
void vcpuinsFinal() {}
