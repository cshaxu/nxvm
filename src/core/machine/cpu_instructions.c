/* This file is a part of NXVM project. */

/* DEBUGGING OPTIONS ******************************************************* */
#define i386(n) if (1)
/* ************************************************************************* */

#include "type.h"
#include "core/machine/port.h"
#include "core/machine/memory.h"
#include "core/machine/pic.h"

#include "core/machine/cpu_instructions.h"

#define cpu_state (*context->cpu)
#define instruction_state (*context->instructions)
#define ExecCpuInstruction(handler) ((handler) ? ((handler)(context), 0) : 0)

#define UTILS_TRACE_VAR    trace
#define UTILS_TRACE_ERROR  instruction_state.data.except
#define UTILS_TRACE_SETERR (_SetExcept_CE(0xffffffff))

static t_utils_trace UTILS_TRACE_VAR;

/* indicates functions not implemented */
#define _______todo static void
/* prints untested code path */
#define _new_code_path_ do { \
PRINTF("NEW CODE PATH\n");utilsTracePrint(&(UTILS_TRACE_VAR));} while (0)

/* stack pointer size */
#define _GetStackSize   (cpu_state.data.ss.seg.data.big ? 4 : 2)
/* operand size */
#define _GetOperandSize ((cpu_state.data.cs.seg.exec.defsize ^ instruction_state.data.prefix_oprsize) ? 4 : 2)
/* address size of the source operand */
#define _GetAddressSize ((cpu_state.data.cs.seg.exec.defsize ^ instruction_state.data.prefix_addrsize) ? 4 : 2)
/* if opcode indicates a prefix */
#define _SetExcept_DE(n) (SetBit(instruction_state.data.except, VCPUINS_EXCEPT_DE), instruction_state.data.excode = (n), PRINTF("#DE(%x) - divide error\n",    instruction_state.data.excode))
#define _SetExcept_PF(n) (SetBit(instruction_state.data.except, VCPUINS_EXCEPT_PF), instruction_state.data.excode = (n), PRINTF("#PF(%x) - page fault\n",      instruction_state.data.excode))
#define _SetExcept_GP(n) (SetBit(instruction_state.data.except, VCPUINS_EXCEPT_GP), instruction_state.data.excode = (n), PRINTF("#GP(%x) - general protect\n", instruction_state.data.excode))
#define _SetExcept_SS(n) (SetBit(instruction_state.data.except, VCPUINS_EXCEPT_SS), instruction_state.data.excode = (n), PRINTF("#SS(%x) - stack segment\n",   instruction_state.data.excode))
#define _SetExcept_UD(n) (SetBit(instruction_state.data.except, VCPUINS_EXCEPT_UD), instruction_state.data.excode = (n), PRINTF("#UD(%x) - undefined\n",       instruction_state.data.excode))
#define _SetExcept_NP(n) (SetBit(instruction_state.data.except, VCPUINS_EXCEPT_NP), instruction_state.data.excode = (n), PRINTF("#NP(%x) - not present\n",     instruction_state.data.excode))
#define _SetExcept_BR(n) (SetBit(instruction_state.data.except, VCPUINS_EXCEPT_BR), instruction_state.data.excode = (n), PRINTF("#BR(%x) - boundary\n",        instruction_state.data.excode))
#define _SetExcept_TS(n) (SetBit(instruction_state.data.except, VCPUINS_EXCEPT_TS), instruction_state.data.excode = (n), PRINTF("#TS(%x) - task state\n",      instruction_state.data.excode))
#define _SetExcept_NM(n) (SetBit(instruction_state.data.except, VCPUINS_EXCEPT_NM), instruction_state.data.excode = (n), PRINTF("#NM(%x) - divide error\n",    instruction_state.data.excode))
#define _SetExcept_CE(n) (SetBit(instruction_state.data.except, VCPUINS_EXCEPT_CE), instruction_state.data.excode = (n), PRINTF("#CE(%x) - internal error\n",  instruction_state.data.excode))

/* memory management unit */
/* kernel memory accessing */
/* read content from reference */
static void _kma_read_ref(core_machine_cpu_execution_context *context, t_vaddrcc ref, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_kma_read_ref");
    MEMCPY((void *) rdata, (void *) ref, byte);
    _ce;
}
/* write content to reference */
static void _kma_write_ref(core_machine_cpu_execution_context *context, t_vaddrcc ref, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_kma_write_ref");
    MEMCPY((void *) ref, (void *) rdata, byte);
    _ce;
}
/* read content from physical */
static void _kma_read_physical(core_machine_cpu_execution_context *context, t_nubit32 physical, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_kma_read_physical");
    _chr(core_machine_memory_read_physical(context->memory, physical, rdata,
        byte));
    _ce;
}
/* write content to physical */
static void _kma_write_physical(core_machine_cpu_execution_context *context, t_nubit32 physical, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_kma_write_physical");
    _chr(core_machine_memory_write_physical(context->memory, physical, rdata,
        byte));
    _ce;
}
/* translate linear to physical - paging mechanism*/
static t_nubit32 _kma_physical_linear(core_machine_cpu_execution_context *context, t_nubit32 linear, t_nubit8 byte, t_bool write, t_nubit8 vpl) {
    t_nubit32 ppde, ppte; /* page table entries */
    t_nubit32 cpde, cpte;
    _cb("_t_kma_physical_linear");
    if (_GetLinear_Offset(linear) > GetMax32(_GetPageSize - byte)) _impossible_rz_;
    if (!_IsPaging) {
        _ce;
        return linear;
    }
    ppde = _GetCR3_Base + _GetLinear_Dir(linear) * 4;
    _chrz(_kma_read_physical(context, ppde, GetRef(cpde), 4));
    if (!_IsPageEntryPresent(cpde)) {
        _bb("!PageDirEntryPresent");
        cpu_state.data.cr2 = linear;
        _chrz(_SetExcept_PF(_MakePageFaultErrorCode(0, write, (vpl == 3))));
        _be;
    }
    if (vpl == 0x03) {
        _bb("vpl(3)");
        if (!_GetPageEntry_US(cpde)) {
            _bb("PageDirEntry_US(0)");
            cpu_state.data.cr2 = linear;
            _chrz(_SetExcept_PF(_MakePageFaultErrorCode(1, write, 1)));
            _be;
        }
        if (write && !_IsPageEntryWritable(cpde)) {
            _bb("write,!PageDirEntryWritable");
            cpu_state.data.cr2 = linear;
            _chrz(_SetExcept_PF(_MakePageFaultErrorCode(1, 1, 1)));
            _be;
        }
        _be;
    }
    _SetPageEntry_A(cpde);
    _chrz(_kma_write_physical(context, ppde, GetRef(cpde), 4));
    ppte = _GetPageEntry_Base(cpde) + _GetLinear_Page(linear) * 4;
    _chrz(_kma_read_physical(context, ppte, GetRef(cpte), 4));
    if (!_IsPageEntryPresent(cpte)) {
        _bb("!PageTabEntryPresent");
        cpu_state.data.cr2 = linear;
        _chrz(_SetExcept_PF(_MakePageFaultErrorCode(0, write, (vpl == 3))));
        _be;
    }
    if (vpl == 0x03) {
        _bb("vpl(3)");
        if (!_GetPageEntry_US(cpte)) {
            _bb("PageTabEntry_US(0)");
            cpu_state.data.cr2 = linear;
            _chrz(_SetExcept_PF(_MakePageFaultErrorCode(1, write, 1)));
            _be;
        }
        if (write && !_IsPageEntryWritable(cpte)) {
            _bb("write,!PageTabEntryWritable");
            cpu_state.data.cr2 = linear;
            _chrz(_SetExcept_PF(_MakePageFaultErrorCode(1, 1, 1)));
            _be;
        }
        _be;
    }
    _SetPageEntry_A(cpte);
    if (write) _SetPageEntry_D(cpte);
    _chrz(_kma_write_physical(context, ppte, GetRef(cpte), 4));
    _ce;
    return (_GetPageEntry_Base(cpte) + _GetLinear_Offset(linear));
}
/* translate logical to linear - segmentation mechanism */
static t_nubit32 _kma_linear_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force) {
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
static void _kma_read_linear(core_machine_cpu_execution_context *context, t_nubit32 linear, t_vaddrcc rdata, t_nubit8 byte, t_nubit8 vpl, t_bool force) {
    t_nubit32 phy1, phy2;
    t_nubit8  byte1, byte2;
    _cb("_kma_read_logical");
    if (_GetLinear_Offset(linear) > GetMax32(_GetPageSize - byte)) {
        _bb("Linear_Offset(>PageSize)");
        byte1 = _GetPageSize - _GetLinear_Offset(linear);
        byte2 = byte - byte1;
        _chr(phy1 = _kma_physical_linear(context, linear        , byte1, 0, vpl));
        _chr(phy2 = _kma_physical_linear(context, linear + byte1, byte2, 0, vpl));
        _chr(_kma_read_physical(context, phy1, rdata        , byte1));
        _chr(_kma_read_physical(context, phy2, rdata + byte1, byte2));
        _be;
    } else {
        _bb("Linear_Offset(<=PageSize)");
        byte1 = byte;
        _chr(phy1 = _kma_physical_linear(context, linear, byte1, 0, vpl));
        _chr(_kma_read_physical(context, phy1, rdata, byte1));
        _be;
    }
    _ce;
}
/* write content to logical */
static void _kma_write_linear(core_machine_cpu_execution_context *context, t_nubit32 linear, t_vaddrcc rdata, t_nubit8 byte, t_nubit8 vpl, t_bool force) {
    t_nubit32 phy1, phy2;
    t_nubit8  byte1, byte2;
    _cb("_kma_write_linear");
    if (_GetLinear_Offset(linear) > GetMax32(_GetPageSize - byte)) {
        _bb("Linear_Offset(>PageSize)");
        byte1 = _GetPageSize - _GetLinear_Offset(linear);
        byte2 = byte - byte1;
        _chr(phy1 = _kma_physical_linear(context, linear, byte1, 1, vpl));
        _chr(phy2 = _kma_physical_linear(context, linear + byte1, byte2, 1, vpl));
        _chr(_kma_write_physical(context, phy1, rdata, byte1));
        _chr(_kma_write_physical(context, phy2, rdata + byte1, byte2));
        _be;
    } else {
        _bb("Linear_Offset(<=PageSize)");
        byte1 = byte;
        _chr(phy1 = _kma_physical_linear(context, linear, byte1, 1, vpl));
        _chr(_kma_write_physical(context, phy1, rdata, byte1));
        _be;
    }
    _ce;
}
/* read content from logical */
static void _kma_read_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte, t_nubit8 vpl, t_bool force) {
    /* t_nubitcc i; */
    t_nubit32 linear;
    _cb("_kma_read_logical");
    _chr(linear = _kma_linear_logical(context, rsreg, offset, byte, 0, vpl, force));
    _chr(_kma_read_linear(context, linear, rdata, byte, vpl, force));
    if (!force) {
        _bb("!force");
        instruction_state.data.mem[instruction_state.data.msize].flagWrite = False;
        instruction_state.data.mem[instruction_state.data.msize].data = 0;
        MEMCPY((void *) GetRef(instruction_state.data.mem[instruction_state.data.msize].data), (void *) rdata, byte);
        instruction_state.data.mem[instruction_state.data.msize].byte = byte;
        instruction_state.data.mem[instruction_state.data.msize].linear = linear;
        if (instruction_state.data.flagWR) {
            if (instruction_state.data.wrLinear >= instruction_state.data.mem[instruction_state.data.msize].linear &&
                    instruction_state.data.wrLinear < instruction_state.data.mem[instruction_state.data.msize].linear + byte) {
                PRINTF("Watch point caught at L%08x: READ %01x BYTES OF DATA=%08x FROM L%08x\n", instruction_state.data.linear,
                       instruction_state.data.mem[instruction_state.data.msize].byte,
                       instruction_state.data.mem[instruction_state.data.msize].data,
                       instruction_state.data.mem[instruction_state.data.msize].linear);
            }
        }
        /* for (i = 0;i < instruction_state.data.msize;++i) {
            if (instruction_state.data.mem[i].flagWrite == instruction_state.data.mem[instruction_state.data.msize].flagWrite &&
                instruction_state.data.mem[i].linear == instruction_state.data.mem[instruction_state.data.msize].linear) {
                _bb("mem(same)");
                _impossible_rz_;
                _ce;
            }
        } */
        instruction_state.data.msize++;
        if (instruction_state.data.msize == 0x20) _impossible_r_;
        _be;
    }
    _ce;
}
/* write content to logical */
static void _kma_write_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte, t_nubit8 vpl, t_bool force) {
    /* t_nubitcc i; */
    t_nubit32 linear;
    _cb("_kma_write_logical");
    _chr(linear = _kma_linear_logical(context, rsreg, offset, byte, 1, vpl, force));
    _chr(_kma_write_linear(context, linear, rdata, byte, vpl, force));
    if (!force) {
        _bb("!force");
        instruction_state.data.mem[instruction_state.data.msize].flagWrite = True;
        instruction_state.data.mem[instruction_state.data.msize].data = 0;
        MEMCPY((void *) GetRef(instruction_state.data.mem[instruction_state.data.msize].data), (void *) rdata, byte);
        instruction_state.data.mem[instruction_state.data.msize].byte = byte;
        instruction_state.data.mem[instruction_state.data.msize].linear = linear;
        if (instruction_state.data.flagWW) {
            if (instruction_state.data.wwLinear >= instruction_state.data.mem[instruction_state.data.msize].linear &&
                    instruction_state.data.wwLinear < instruction_state.data.mem[instruction_state.data.msize].linear + byte) {
                PRINTF("Watch point caught at L%08x: WRITE %01x BYTES OF DATA=%08x TO L%08x\n", instruction_state.data.linear,
                       instruction_state.data.mem[instruction_state.data.msize].byte,
                       instruction_state.data.mem[instruction_state.data.msize].data,
                       instruction_state.data.mem[instruction_state.data.msize].linear);
            }
        }
        /* for (i = 0;i < instruction_state.data.msize;++i) {
            if (instruction_state.data.mem[i].flagWrite == instruction_state.data.mem[instruction_state.data.msize].flagWrite &&
                instruction_state.data.mem[i].linear == instruction_state.data.mem[instruction_state.data.msize].linear) {
                _bb("mem(same)");
                _impossible_r_;
                _ce;
            }
        } */
        instruction_state.data.msize++;
        if (instruction_state.data.msize == 0x20) _impossible_r_;
        _be;
    }
    _ce;
}
/* test logical accessing */
static void _kma_test_linear(core_machine_cpu_execution_context *context, t_nubit32 linear, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force) {
    t_nubit32 phy1, phy2;
    t_nubit8  byte1, byte2;
    _cb("_kma_test_linear");
    if (_GetLinear_Offset(linear) > GetMax32(_GetPageSize - byte)) {
        _bb("Linear_Offset(>PageSize)");
        byte1 = _GetPageSize - _GetLinear_Offset(linear);
        byte2 = byte - byte1;
        _chr(phy1 = _kma_physical_linear(context, linear        , byte1, write, vpl));
        _chr(phy2 = _kma_physical_linear(context, linear + byte1, byte2, write, vpl));
        _be;
    } else {
        _bb("Linear_Offset(<=PageSize)");
        byte1 = byte;
        _chr(phy1 = _kma_physical_linear(context, linear, byte1, write, vpl));
        _be;
    }
    _ce;
}
static void _kma_test_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force) {
    t_nubit32 linear;
    _cb("_kma_test_logical");
    _chr(linear = _kma_linear_logical(context, rsreg, offset, byte, write, vpl, force));
    _ce;
}
static void _kma_test_access(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write, t_nubit8 vpl, t_bool force) {
    t_nubit32 linear;
    _cb("_kma_test_access");
    _chr(linear = _kma_linear_logical(context, rsreg, offset, byte, write, vpl, force));
    _chr(_kma_test_linear(context, linear, byte, write, vpl, force));
    _ce;
}

/* general memory accessing */
static void _m_read_ref(core_machine_cpu_execution_context *context, t_vaddrcc ref, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_m_read_ref");
    /* _m_write_ref allows in-module reads only */
    if (ref < (t_vaddrcc)(&cpu_state) && ref >= (t_vaddrcc)(&cpu_state) + sizeof(t_cpu) &&
            ref < (t_vaddrcc)(&instruction_state) && ref >= (t_vaddrcc)(&instruction_state) + sizeof(t_cpuins)) {
        _impossible_r_;
    }
    _chr(_kma_read_ref(context, ref, rdata, byte));
    _ce;
}
static void _m_write_ref(core_machine_cpu_execution_context *context, t_vaddrcc ref, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_m_write_ref");
    /* _m_write_ref allows in-module writes only */
    if (ref < (t_vaddrcc)(&cpu_state) && ref >= (t_vaddrcc)(&cpu_state) + sizeof(t_cpu) &&
            ref < (t_vaddrcc)(&instruction_state) && ref >= (t_vaddrcc)(&instruction_state) + sizeof(t_cpuins)) {
        _impossible_r_;
    }
    _chr(_kma_write_ref(context, ref, rdata, byte));
    _ce;
}
static void _m_read_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_m_read_logical");
    _chr(_kma_read_logical(context, rsreg, offset, rdata, byte, _GetCPL, 0));
    _ce;
}
static void _m_write_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_m_write_logical");
    _chr(_kma_write_logical(context, rsreg, offset, rdata, byte, _GetCPL, 0));
    _ce;
}
static void _m_test_access(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write) {
    t_vaddrcc ref = 0;
    _cb("_m_test_access");
    _chr(_kma_test_access(context, rsreg, offset, byte, write, _GetCPL, 0));
    _ce;
}
static void _m_test_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, t_nubit32 offset, t_nubit8 byte, t_bool write) {
    t_vaddrcc ref = 0;
    _cb("_m_test_logical");
    _chr(_kma_test_logical(context, rsreg, offset, byte, write, _GetCPL, 0));
    _ce;
}

static void _m_read_rm(core_machine_cpu_execution_context *context, t_nubit8 byte) {
    _cb("_m_read_rm");
    instruction_state.data.crm = 0;
    if (instruction_state.data.flagMem)
        _chr(_m_read_logical(context, instruction_state.data.mrm.rsreg, instruction_state.data.mrm.offset, GetRef(instruction_state.data.crm), byte));
    else
        _chr(_m_read_ref(context, instruction_state.data.rrm, GetRef(instruction_state.data.crm), byte));
    _ce;
}
static void _m_write_rm(core_machine_cpu_execution_context *context, t_nubit8 byte) {
    _cb("_m_write_rm");
    if (instruction_state.data.flagMem)
        _chr(_m_write_logical(context, instruction_state.data.mrm.rsreg, instruction_state.data.mrm.offset, GetRef(instruction_state.data.crm), byte));
    else
        _chr(_m_write_ref(context, instruction_state.data.rrm, GetRef(instruction_state.data.crm), byte));
    _ce;
}

/* segment accessing unit: _s_ */
/* kernel segment accessing */
static void _ksa_read_idt(core_machine_cpu_execution_context *context, t_nubit8 intid, t_vaddrcc rdata) {
    _cb("_s_read_idt");
    if (!_GetCR0_PE) {
        _bb("CR0_PE(0)");
        if (GetMax16(intid * 4 + 3) > GetMax16(cpu_state.data.idtr.limit)) _impossible_r_;
        _chr(_kma_read_logical(context, &cpu_state.data.idtr, (intid * 4), rdata, 4, 0x00, 0));
        _be;
    } else {
        _bb("CR0_PE(1)");
        if (GetMax16(intid * 8 + 7) > GetMax16(cpu_state.data.idtr.limit)) _impossible_r_;
        _chr(_kma_read_logical(context, &cpu_state.data.idtr, (intid * 8), rdata, 8, 0x00, 0));
        _be;
    }
    _ce;
}
static void _ksa_read_ldt(core_machine_cpu_execution_context *context, t_nubit16 selector, t_vaddrcc rdata) {
    _cb("_ksa_read_ldt");
    if (!_GetCR0_PE) _impossible_r_;
    if (!_GetSelector_TI(selector)) _impossible_r_;
    if (GetMax16(_GetSelector_Offset(selector) + 7) > GetMax16(cpu_state.data.ldtr.limit)) {
        _bb("Selector_Offset(>ldtr.limit)");
        _chr(_SetExcept_GP(selector));
        _be;
    }
    _chr(_kma_read_logical(context, &cpu_state.data.ldtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
    _ce;
}
static void _ksa_read_gdt(core_machine_cpu_execution_context *context, t_nubit16 selector, t_vaddrcc rdata) {
    _cb("_ksa_read_gdt");
    if (!_GetCR0_PE) _impossible_r_;
    if (_GetSelector_TI(selector)) _impossible_r_;
    if (GetMax16(_GetSelector_Offset(selector) + 7) > GetMax16(cpu_state.data.gdtr.limit)) {
        _bb("Selector_Offset(>gdtr.limit)");
        _chr(_SetExcept_GP(selector));
        _be;
    }
    _chr(_kma_read_logical(context, &cpu_state.data.gdtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
    _ce;
}
static void _ksa_read_xdt(core_machine_cpu_execution_context *context, t_nubit16 selector, t_vaddrcc rdata) {
    _cb("_ksa_read_xdt");
    if (!_GetCR0_PE) _impossible_r_;
    if (_GetSelector_TI(selector)) {
        _bb("Selector_TI");
        _chr(_ksa_read_ldt(context, selector, rdata));
        _be;
    } else {
        _bb("!Selector_TI");
        _chr(_ksa_read_gdt(context, selector, rdata));
        _be;
    }
    _ce;
}
static void _ksa_write_ldt(core_machine_cpu_execution_context *context, t_nubit16 selector, t_vaddrcc rdata) {
    _cb("_ksa_write_ldt");
    if (!_GetCR0_PE) _impossible_r_;
    if (!_GetSelector_TI(selector)) _impossible_r_;
    if (GetMax16(_GetSelector_Offset(selector) + 7) > GetMax16(cpu_state.data.ldtr.limit))
        _impossible_r_;
    _chr(_kma_write_logical(context, &cpu_state.data.ldtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
    _ce;
}
static void _ksa_write_gdt(core_machine_cpu_execution_context *context, t_nubit16 selector, t_vaddrcc rdata) {
    _cb("_ksa_write_gdt");
    if (!_GetCR0_PE) _impossible_r_;
    if (_GetSelector_TI(selector)) _impossible_r_;
    if (GetMax16(_GetSelector_Offset(selector) + 7) > GetMax16(cpu_state.data.gdtr.limit))
        _impossible_r_;
    _chr(_kma_write_logical(context, &cpu_state.data.gdtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
    _ce;
}
static void _ksa_write_xdt(core_machine_cpu_execution_context *context, t_nubit16 selector, t_vaddrcc rdata) {
    _cb("_ksa_write_xdt");
    if (!_GetCR0_PE) _impossible_r_;
    if (_GetSelector_TI(selector)) {
        _bb("Selector_TI");
        _chr(_ksa_write_ldt(context, selector, rdata));
        _be;
    } else {
        _bb("!Selector_TI");
        _chr(_ksa_write_gdt(context, selector, rdata));
        _be;
    }
    _ce;
}
static void _ksa_load_sreg(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, t_nubit16 selector) {
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
            _chr(_ksa_read_xdt(context, selector, GetRef(descriptor)));
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
            _chr(_ksa_write_xdt(context, selector, GetRef(descriptor)));
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
                _chr(_ksa_read_xdt(context, selector, GetRef(descriptor)));
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
                _chr(_ksa_write_xdt(context, selector, GetRef(descriptor)));
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
            _chr(_ksa_read_xdt(context, selector, GetRef(descriptor)));
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
            _chr(_ksa_write_xdt(context, selector, GetRef(descriptor)));
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
        _chr(_ksa_read_xdt(context, selector, GetRef(descriptor)));
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
        _chr(_ksa_write_xdt(context, selector, GetRef(descriptor)));
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
            _chr(_ksa_read_xdt(context, selector, GetRef(descriptor)));
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
static t_bool _s_check_selector(core_machine_cpu_execution_context *context, t_nubit16 selector) {
    /* 0 = succ, 1 = fail */
    _cb("_s_check_selector");
    if (_IsSelectorNull(selector)) {
        _ce;
        return True;
    }
    if (GetMax32(_GetSelector_Offset(selector) + 7) >
            (_GetSelector_TI(selector) ? GetMax32(cpu_state.data.ldtr.limit) :
             GetMax32(cpu_state.data.gdtr.limit))) {
        _ce;
        return True;
    }
    _ce;
    return False;
}
static void _s_read_idt(core_machine_cpu_execution_context *context, t_nubit8 intid, t_vaddrcc rdata) {
    _cb("_s_read_idt");
    _chr(_ksa_read_idt(context, intid, rdata));
    _ce;
}
static void _s_read_xdt(core_machine_cpu_execution_context *context, t_nubit16 selector, t_vaddrcc rdata) {
    _cb("_s_read_xdt");
    if (!_GetCR0_PE) _impossible_r_;
    _chr(_ksa_read_xdt(context, selector, rdata));
    _ce;
}
static void _s_read_tss(core_machine_cpu_execution_context *context, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_read_tss");
    _chr(_kma_read_logical(context, &cpu_state.data.tr, offset, rdata, byte, 0, 1));
    _ce;
}
static void _s_read_es(core_machine_cpu_execution_context *context, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_read_es");
    _chr(_m_read_logical(context, &cpu_state.data.es, offset, rdata, byte));
    _ce;
}
static void _s_read_cs(core_machine_cpu_execution_context *context, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_read_cs");
    _chr(_kma_read_logical(context, &cpu_state.data.cs, offset, rdata, byte, 0, 1));
    _ce;
}
static void _s_read_ss(core_machine_cpu_execution_context *context, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_read_ss");
    _chr(_m_read_logical(context, &cpu_state.data.ss, offset, rdata, byte));
    _ce;
}
static void _s_read_ds(core_machine_cpu_execution_context *context, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_read_ds");
    _impossible_r_;
    _ce;
}
static void _s_read_fs(core_machine_cpu_execution_context *context, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_read_fs");
    _impossible_r_;
    _ce;
}
static void _s_read_gs(core_machine_cpu_execution_context *context, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_read_gs");
    _impossible_r_;
    _ce;
}
static void _s_write_idt(core_machine_cpu_execution_context *context, t_nubit8 intid, t_vaddrcc rdata) {
    _cb("_s_write_idt");
    _impossible_r_;
    _ce;
}
static void _s_write_xdt(core_machine_cpu_execution_context *context, t_nubit16 selector, t_vaddrcc rdata) {
    _cb("_s_write_xdt");
    if (!_GetCR0_PE) _impossible_r_;
    _chr(_ksa_write_xdt(context, selector, rdata));
    _ce;
}
static void _s_write_tss(core_machine_cpu_execution_context *context, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_write_tss");
    _impossible_r_;
    _ce;
}
static void _s_write_es(core_machine_cpu_execution_context *context, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_write_es");
    _chr(_m_write_logical(context, &cpu_state.data.es, offset, rdata, byte));
    _ce;
}
static void _s_write_cs(core_machine_cpu_execution_context *context, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_write_cs");
    _impossible_r_;
    _ce;
}
static void _s_write_ss(core_machine_cpu_execution_context *context, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_write_ss");
    _chr(_m_write_logical(context, &cpu_state.data.ss, offset, rdata, byte));
    _ce;
}
static void _s_write_ds(core_machine_cpu_execution_context *context, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_write_ds");
    _impossible_r_;
    _ce;
}
static void _s_write_fs(core_machine_cpu_execution_context *context, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_write_fs");
    _impossible_r_;
    _ce;
}
static void _s_write_gs(core_machine_cpu_execution_context *context, t_nubit32 offset, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_s_write_gs");
    _impossible_r_;
    _ce;
}
static void _s_test_tss(core_machine_cpu_execution_context *context, t_nubit32 offset, t_nubit8 byte) {
    _cb("_s_test_cs");
    _chr(_kma_test_logical(context, &cpu_state.data.tr, offset, byte, 0, 0x00, 1));
    _ce;
}
static void _s_test_cs(core_machine_cpu_execution_context *context, t_nubit32 offset, t_nubit8 byte) {
    _cb("_s_test_cs");
    _chr(_kma_test_logical(context, &cpu_state.data.cs, offset, byte, 0, 0x00, 1));
    _ce;
}
static void _s_test_ss_push(core_machine_cpu_execution_context *context, t_nubit8 byte) {
    t_nubit32 cesp = 0x00000000;
    _cb("_s_test_ss_push");
    switch (_GetStackSize) {
    case 2:
        _bb("StackSize(2)");
        if (cpu_state.data.sp && cpu_state.data.sp < byte)
            _chr(_SetExcept_SS(0));
        _chr(_m_test_access(context, &cpu_state.data.ss, GetMax16(cpu_state.data.sp - byte), byte, 1));
        _be;
        break;
    case 4:
        _bb("StackSize(4)");
        if (cpu_state.data.esp && cpu_state.data.esp < byte)
            _chr(_SetExcept_SS(0));
        _chr(_m_test_access(context, &cpu_state.data.ss, GetMax32(cpu_state.data.esp - byte), byte, 1));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void _s_test_ss_pop(core_machine_cpu_execution_context *context, t_nubit8 byte) {
    t_nubit32 cesp = 0x00000000;
    _cb("_s_test_ss_pop");
    switch (_GetStackSize) {
    case 2:
        _bb("StackSize(2)");
        _chr(_m_test_access(context, &cpu_state.data.ss, cpu_state.data.sp, byte, 0));
        _be;
        break;
    case 4:
        _bb("StackSize(4)");
        _chr(_m_test_access(context, &cpu_state.data.ss, cpu_state.data.esp, byte, 0));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void _s_load_sreg(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, t_nubit16 selector) {
    _cb("_s_load_sreg");
    _chr(_ksa_load_sreg(context, rsreg, selector));
    _ce;
}
static void _s_load_gdtr(core_machine_cpu_execution_context *context, t_nubit32 base, t_nubit16 limit, t_nubit8 byte) {
    _cb("_s_load_gdtr");
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    cpu_state.data.gdtr.limit = limit;
    switch (byte) {
    case 2:
        cpu_state.data.gdtr.base = GetMax24(base);
        break;
    case 4:
        cpu_state.data.gdtr.base = GetMax32(base);
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    _ce;
}
static void _s_load_idtr(core_machine_cpu_execution_context *context, t_nubit32 base, t_nubit16 limit, t_nubit8 byte) {
    _cb("_s_load_idtr");
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    cpu_state.data.idtr.limit = limit;
    switch (byte) {
    case 2:
        cpu_state.data.idtr.base = GetMax24(base);
        break;
    case 4:
        cpu_state.data.idtr.base = GetMax32(base);
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    _ce;
}
static void _s_load_ldtr(core_machine_cpu_execution_context *context, t_nubit16 selector) {
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
    _chr(_s_load_sreg(context, &cpu_state.data.ldtr, selector));
    _ce;
}
static void _s_load_tr(core_machine_cpu_execution_context *context, t_nubit16 selector) {
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
    _chr(_s_load_sreg(context, &cpu_state.data.tr, selector));
    _ce;
}
static void _s_load_cr0_msw(core_machine_cpu_execution_context *context, t_nubit16 msw) {
    _cb("_s_load_cr0_msw");
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    if (!_GetCR0_PE) {
        _bb("CR0_PE(0)");
        cpu_state.data.cr0 = (cpu_state.data.cr0 & 0xfffffff0) | (msw & 0x000f);
        _be;
    } else {
        _bb("CR0_PE(1)");
        cpu_state.data.cr0 = (cpu_state.data.cr0 & 0xfffffff0) | (msw & 0x000e) | 0x01;
        _be;
    }
    _ce;
}
static void _s_load_cs(core_machine_cpu_execution_context *context, t_nubit16 newcs) {
    _cb("_s_load_cs");
    _chr(_s_load_sreg(context, &cpu_state.data.cs, newcs));
    _ce;
}
static void _s_load_ss(core_machine_cpu_execution_context *context, t_nubit16 newss) {
    _cb("_s_load_ss");
    _chr(_s_load_sreg(context, &cpu_state.data.ss, newss));
    _ce;
}
static void _s_load_ds(core_machine_cpu_execution_context *context, t_nubit16 newds) {
    _cb("_s_load_ds");
    _chr(_s_load_sreg(context, &cpu_state.data.ds, newds));
    _ce;
}
static void _s_load_es(core_machine_cpu_execution_context *context, t_nubit16 newes) {
    _cb("_s_load_es");
    _chr(_s_load_sreg(context, &cpu_state.data.es, newes));
    _ce;
}
static void _s_load_fs(core_machine_cpu_execution_context *context, t_nubit16 newfs) {
    _cb("_s_load_fs");
    _chr(_s_load_sreg(context, &cpu_state.data.fs, newfs));
    _ce;
}
static void _s_load_gs(core_machine_cpu_execution_context *context, t_nubit16 newgs) {
    _cb("_s_load_gs");
    _chr(_s_load_sreg(context, &cpu_state.data.gs, newgs));
    _ce;
}
static void _s_test_eip(core_machine_cpu_execution_context *context) {
    _cb("_s_test_eip");
    _chr(_s_test_cs(context, cpu_state.data.eip, 0x01));
    _ce;
}
static void _s_test_esp(core_machine_cpu_execution_context *context) {
    t_nubit32 cesp;
    _cb("_s_test_esp");
    switch (_GetStackSize) {
    case 2:
        cesp = GetMax16(cpu_state.data.esp);
        break;
    case 4:
        cesp = GetMax32(cpu_state.data.esp);
        break;
    default:
        _impossible_r_;
        break;
    }
    _chr(_m_test_logical(context, &cpu_state.data.ss, cesp, 0x00, 0));
    _ce;
}

/* portid accessing unit */
/* kernel portid accessing */
_______todo _kpa_test_iomap(core_machine_cpu_execution_context *context, t_nubit16 portid, t_nubit8 byte) {
    /* TODO: iomap tester not implemented */
    _cb("_kpa_test_iomap");
    _ce;
}
static void _kpa_test_mode(core_machine_cpu_execution_context *context, t_nubit16 portid, t_nubit8 byte) {
    _cb("_p_test");
    if (_GetCR0_PE && (_GetCPL > (t_nubit8)_GetEFLAGS_IOPL || _GetEFLAGS_VM)) {
        _bb("CR0_PE(1),(CPL>IOPL/EFLAGS_VM(1))");
        _chr(_kpa_test_iomap(context, portid, byte));
        _be;
    }
    _ce;
}
/* regular portid accessing */
static void _p_input(core_machine_cpu_execution_context *context, t_nubit16 portid, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_p_input");
    _chr(_kpa_test_mode(context, portid, byte));
    core_machine_port_execute_read(context->port, portid);
    switch (byte) {
    case 1:
        _bb("byte(1)");
        _chr(_m_write_ref(context, rdata,
            GetRef(context->port->data.ioByte), 1));
        _be;
        break;
    case 2:
        _bb("byte(2)");
        _chr(_m_write_ref(context, rdata,
            GetRef(context->port->data.ioWord), 2));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_m_write_ref(context, rdata,
            GetRef(context->port->data.ioDWord), 4));
        _be;
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    instruction_state.data.flagIgnore = True;
    _ce;
}
static void _p_output(core_machine_cpu_execution_context *context, t_nubit16 portid, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_p_output");
    _chr(_kpa_test_mode(context, portid, byte));
    switch (byte) {
    case 1:
        _bb("byte(1)");
        _chr(_m_read_ref(context, rdata,
            GetRef(context->port->data.ioByte), 1));
        _be;
        break;
    case 2:
        _bb("byte(2)");
        _chr(_m_read_ref(context, rdata,
            GetRef(context->port->data.ioWord), 2));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_m_read_ref(context, rdata,
            GetRef(context->port->data.ioDWord), 4));
        _be;
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    core_machine_port_execute_write(context->port, portid);
    instruction_state.data.flagIgnore = True;
    _ce;
}

/* decoding unit */
/* kernel decoding function */
static t_bool _kdf_check_prefix(core_machine_cpu_execution_context *context, t_nubit8 opcode) {
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

static void _kdf_skip(core_machine_cpu_execution_context *context, t_nubit8 byte) {
    _cb("_kdf_skip");
    _chr(cpu_state.data.eip += byte);
    _ce;
}
static void _kdf_code(core_machine_cpu_execution_context *context, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_kdf_code");
    _chr(_s_read_cs(context, cpu_state.data.eip, rdata, byte));
    _chr(_kdf_skip(context, byte));
    _ce;
}
static void _kdf_modrm(core_machine_cpu_execution_context *context, t_nubit8 regbyte, t_nubit8 rmbyte) {
    t_nsbit8 disp8;
    t_nubit16 disp16;
    t_nubit32 disp32;
    t_nubit32 sibindex;
    t_nubit8 modrm, sib;
    _cb("_kdf_modrm");
    _chr(_kdf_code(context, GetRef(modrm), 1));
    instruction_state.data.flagMem = True;
    instruction_state.data.mrm.rsreg = NULL;
    instruction_state.data.mrm.offset = 0;
    instruction_state.data.cr = instruction_state.data.crm = 0;
    instruction_state.data.rrm = instruction_state.data.rr = (t_vaddrcc)NULL;
    switch (_GetAddressSize) {
    case 2:
        _bb("AddressSize(2)");
        switch (_GetModRM_MOD(modrm)) {
        case 0:
            _bb("ModRM_MOD(0)");
            switch (_GetModRM_RM(modrm)) {
            case 0:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.bx + cpu_state.data.si);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 1:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.bx + cpu_state.data.di);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 2:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.bp + cpu_state.data.si);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 3:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.bp + cpu_state.data.di);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 4:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.si);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 5:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.di);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 6:
                _bb("ModRM_RM(6)");
                _chr(_kdf_code(context, GetRef(disp16), 2));
                instruction_state.data.mrm.offset = GetMax16(disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                _be;
                break;
            case 7:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.bx);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
            break;
        case 1:
            _bb("ModRM_MOD(1)");
            _chr(_kdf_code(context, GetRef(disp8), 1));
            switch (_GetModRM_RM(modrm)) {
            case 0:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.bx + cpu_state.data.si + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 1:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.bx + cpu_state.data.di + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 2:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.bp + cpu_state.data.si + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 3:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.bp + cpu_state.data.di + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 4:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.si + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 5:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.di + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 6:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.bp + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 7:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.bx + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
            break;
        case 2:
            _bb("ModRM_MOD(2)");
            _chr(_kdf_code(context, GetRef(disp16), 2));
            switch (_GetModRM_RM(modrm)) {
            case 0:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.bx + cpu_state.data.si + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 1:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.bx + cpu_state.data.di + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 2:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.bp + cpu_state.data.si + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 3:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.bp + cpu_state.data.di + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 4:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.si + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 5:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.di + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 6:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.bp + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 7:
                instruction_state.data.mrm.offset = GetMax16(cpu_state.data.bx + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
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
            _chr(_kdf_code(context, GetRef(sib), 1));
            switch (_GetSIB_Index(sib)) {
            case 0:
                sibindex = cpu_state.data.eax;
                break;
            case 1:
                sibindex = cpu_state.data.ecx;
                break;
            case 2:
                sibindex = cpu_state.data.edx;
                break;
            case 3:
                sibindex = cpu_state.data.ebx;
                break;
            case 4:
                sibindex = 0x00000000;
                break;
            case 5:
                sibindex = cpu_state.data.ebp;
                break;
            case 6:
                sibindex = cpu_state.data.esi;
                break;
            case 7:
                sibindex = cpu_state.data.edi;
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
                instruction_state.data.mrm.offset = cpu_state.data.eax;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 1:
                instruction_state.data.mrm.offset = cpu_state.data.ecx;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 2:
                instruction_state.data.mrm.offset = cpu_state.data.edx;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 3:
                instruction_state.data.mrm.offset = cpu_state.data.ebx;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 4:
                _bb("ModRM_RM(4)");
                switch (_GetSIB_Base(sib)) {
                case 0:
                    instruction_state.data.mrm.offset = cpu_state.data.eax + sibindex;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    break;
                case 1:
                    instruction_state.data.mrm.offset = cpu_state.data.ecx + sibindex;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    break;
                case 2:
                    instruction_state.data.mrm.offset = cpu_state.data.edx + sibindex;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    break;
                case 3:
                    instruction_state.data.mrm.offset = cpu_state.data.ebx + sibindex;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    break;
                case 4:
                    instruction_state.data.mrm.offset = cpu_state.data.esp + sibindex;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                    break;
                case 5:
                    _bb("SIB_Base(5)");
                    _chr(_kdf_code(context, GetRef(disp32), 4));
                    instruction_state.data.mrm.offset = disp32 + sibindex;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    _be;
                    break;
                case 6:
                    instruction_state.data.mrm.offset = cpu_state.data.esi + sibindex;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    break;
                case 7:
                    instruction_state.data.mrm.offset = cpu_state.data.edi + sibindex;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    break;
                default:
                    _impossible_r_;
                    break;
                }
                _be;
                break;
            case 5:
                _bb("ModRM_RM(5)");
                _chr(_kdf_code(context, GetRef(disp32), 4));
                instruction_state.data.mrm.offset = disp32;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                _be;
                break;
            case 6:
                instruction_state.data.mrm.offset = cpu_state.data.esi;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 7:
                instruction_state.data.mrm.offset = cpu_state.data.edi;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
            break;
        case 1:
            _bb("ModRM_MOD(1)");
            _chr(_kdf_code(context, GetRef(disp8), 1));
            switch (_GetModRM_RM(modrm)) {
            case 0:
                instruction_state.data.mrm.offset = cpu_state.data.eax + disp8;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 1:
                instruction_state.data.mrm.offset = cpu_state.data.ecx + disp8;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 2:
                instruction_state.data.mrm.offset = cpu_state.data.edx + disp8;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 3:
                instruction_state.data.mrm.offset = cpu_state.data.ebx + disp8;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 4:
                _bb("ModRM_RM(4)");
                switch (_GetSIB_Base(sib)) {
                case 0:
                    instruction_state.data.mrm.offset = cpu_state.data.eax + sibindex + disp8;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    break;
                case 1:
                    instruction_state.data.mrm.offset = cpu_state.data.ecx + sibindex + disp8;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    break;
                case 2:
                    instruction_state.data.mrm.offset = cpu_state.data.edx + sibindex + disp8;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    break;
                case 3:
                    instruction_state.data.mrm.offset = cpu_state.data.ebx + sibindex + disp8;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    break;
                case 4:
                    instruction_state.data.mrm.offset = cpu_state.data.esp + sibindex + disp8;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                    break;
                case 5:
                    instruction_state.data.mrm.offset = cpu_state.data.ebp + sibindex + disp8;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                    break;
                case 6:
                    instruction_state.data.mrm.offset = cpu_state.data.esi + sibindex + disp8;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    break;
                case 7:
                    instruction_state.data.mrm.offset = cpu_state.data.edi + sibindex + disp8;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    break;
                default:
                    _impossible_r_;
                    break;
                }
                _be;
                break;
            case 5:
                instruction_state.data.mrm.offset = cpu_state.data.ebp + disp8;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 6:
                instruction_state.data.mrm.offset = cpu_state.data.esi + disp8;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 7:
                instruction_state.data.mrm.offset = cpu_state.data.edi + disp8;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            default:
                _impossible_r_;
                break;
            }
            _be;
            break;
        case 2:
            _bb("ModRM_MOD(2)");
            _chr(_kdf_code(context, GetRef(disp32), 4));
            switch (_GetModRM_RM(modrm)) {
            case 0:
                instruction_state.data.mrm.offset = cpu_state.data.eax + disp32;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 1:
                instruction_state.data.mrm.offset = cpu_state.data.ecx + disp32;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 2:
                instruction_state.data.mrm.offset = cpu_state.data.edx + disp32;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 3:
                instruction_state.data.mrm.offset = cpu_state.data.ebx + disp32;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 4:
                _bb("ModRM_RM(4)");
                switch (_GetSIB_Base(sib)) {
                case 0:
                    instruction_state.data.mrm.offset = cpu_state.data.eax + sibindex + disp32;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    break;
                case 1:
                    instruction_state.data.mrm.offset = cpu_state.data.ecx + sibindex + disp32;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    break;
                case 2:
                    instruction_state.data.mrm.offset = cpu_state.data.edx + sibindex + disp32;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    break;
                case 3:
                    instruction_state.data.mrm.offset = cpu_state.data.ebx + sibindex + disp32;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    break;
                case 4:
                    instruction_state.data.mrm.offset = cpu_state.data.esp + sibindex + disp32;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                    break;
                case 5:
                    instruction_state.data.mrm.offset = cpu_state.data.ebp + sibindex + disp32;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                    break;
                case 6:
                    instruction_state.data.mrm.offset = cpu_state.data.esi + sibindex + disp32;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    break;
                case 7:
                    instruction_state.data.mrm.offset = cpu_state.data.edi + sibindex + disp32;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    break;
                default:
                    _impossible_r_;
                    break;
                }
                _be;
                break;
            case 5:
                instruction_state.data.mrm.offset = cpu_state.data.ebp + disp32;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 6:
                instruction_state.data.mrm.offset = cpu_state.data.esi + disp32;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 7:
                instruction_state.data.mrm.offset = cpu_state.data.edi + disp32;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
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
        instruction_state.data.flagMem = False;
        switch (rmbyte) {
        case 1:
            switch (_GetModRM_RM(modrm)) {
            case 0:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.al);
                break;
            case 1:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.cl);
                break;
            case 2:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.dl);
                break;
            case 3:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.bl);
                break;
            case 4:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.ah);
                break;
            case 5:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.ch);
                break;
            case 6:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.dh);
                break;
            case 7:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.bh);
                break;
            default:
                _impossible_r_;
                break;
            }
            break;
        case 2:
            switch (_GetModRM_RM(modrm)) {
            case 0:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.ax);
                break;
            case 1:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.cx);
                break;
            case 2:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.dx);
                break;
            case 3:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.bx);
                break;
            case 4:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.sp);
                break;
            case 5:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.bp);
                break;
            case 6:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.si);
                break;
            case 7:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.di);
                break;
            default:
                _impossible_r_;
                break;
            }
            break;
        case 4:
            switch (_GetModRM_RM(modrm)) {
            case 0:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.eax);
                break;
            case 1:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.ecx);
                break;
            case 2:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.edx);
                break;
            case 3:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.ebx);
                break;
            case 4:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.esp);
                break;
            case 5:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.ebp);
                break;
            case 6:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.esi);
                break;
            case 7:
                instruction_state.data.rrm = (t_vaddrcc)(&cpu_state.data.edi);
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
        _chr(_m_read_ref(context, instruction_state.data.rrm, GetRef(instruction_state.data.crm), rmbyte));
        _be;
    }
    if (!regbyte) {
        /* reg is operation or segment */
        instruction_state.data.cr = _GetModRM_REG(modrm);
    } else {
        switch (regbyte) {
        case 1:
            switch (_GetModRM_REG(modrm)) {
            case 0:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.al);
                break;
            case 1:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.cl);
                break;
            case 2:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.dl);
                break;
            case 3:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.bl);
                break;
            case 4:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.ah);
                break;
            case 5:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.ch);
                break;
            case 6:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.dh);
                break;
            case 7:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.bh);
                break;
            default:
                _impossible_r_;
                break;
            }
            break;
        case 2:
            switch (_GetModRM_REG(modrm)) {
            case 0:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.ax);
                break;
            case 1:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.cx);
                break;
            case 2:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.dx);
                break;
            case 3:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.bx);
                break;
            case 4:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.sp);
                break;
            case 5:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.bp);
                break;
            case 6:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.si);
                break;
            case 7:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.di);
                break;
            default:
                _impossible_r_;
                break;
            }
            break;
        case 4:
            switch (_GetModRM_REG(modrm)) {
            case 0:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.eax);
                break;
            case 1:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.ecx);
                break;
            case 2:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.edx);
                break;
            case 3:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.ebx);
                break;
            case 4:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.esp);
                break;
            case 5:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.ebp);
                break;
            case 6:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.esi);
                break;
            case 7:
                instruction_state.data.rr = (t_vaddrcc)(&cpu_state.data.edi);
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
        _chr(_m_read_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.cr), regbyte));
    }
    _ce;
}
static void _d_skip(core_machine_cpu_execution_context *context, t_nubit8 byte) {
    _cb("_d_skip");
    _chr(_kdf_skip(context, byte));
    _ce;
}
static void _d_code(core_machine_cpu_execution_context *context, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_d_code");
    _chr(_kdf_code(context, rdata, byte));
    _ce;
}
static void _d_imm(core_machine_cpu_execution_context *context, t_nubit8 byte) {
    _cb("_d_imm");
    instruction_state.data.cimm = 0;
    _chr(_d_code(context, GetRef(instruction_state.data.cimm), byte));
    _ce;
}
static void _d_moffs(core_machine_cpu_execution_context *context, t_nubit8 byte) {
    _cb("_d_moffs");
    instruction_state.data.flagMem = True;
    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
    instruction_state.data.mrm.offset = 0;
    switch (_GetAddressSize) {
    case 2:
        _bb("AddressSize(2)");
        _chr(_d_code(context, GetRef(instruction_state.data.mrm.offset), 2));
        _be;
        break;
    case 4:
        _bb("AddressSize(4)");
        _chr(_d_code(context, GetRef(instruction_state.data.mrm.offset), 4));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void _d_modrm_sreg(core_machine_cpu_execution_context *context, t_nubit8 rmbyte) {
    _cb("_d_modrm_sreg");
    _chr(_kdf_modrm(context, 0, rmbyte));
    instruction_state.data.rmovsreg = NULL;
    switch (instruction_state.data.cr) {
    case 0:
        instruction_state.data.rmovsreg = &cpu_state.data.es;
        break;
    case 1:
        instruction_state.data.rmovsreg = &cpu_state.data.cs;
        break;
    case 2:
        instruction_state.data.rmovsreg = &cpu_state.data.ss;
        break;
    case 3:
        instruction_state.data.rmovsreg = &cpu_state.data.ds;
        break;
    case 4:
        instruction_state.data.rmovsreg = &cpu_state.data.fs;
        break;
    case 5:
        instruction_state.data.rmovsreg = &cpu_state.data.gs;
        break;
    default:
        _bb("instruction_state.data.cr");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    }
    _ce;
}
static void _d_modrm_ea(core_machine_cpu_execution_context *context, t_nubit8 regbyte, t_nubit8 rmbyte) {
    _cb("_d_modrm_ea");
    _chr(_kdf_modrm(context, regbyte, rmbyte));
    if (!instruction_state.data.flagMem) {
        _bb("flagMem(0)");
        _chr(_SetExcept_UD(0));
        _be;
    }
    _ce;
}
static void _d_modrm(core_machine_cpu_execution_context *context, t_nubit8 regbyte, t_nubit8 rmbyte) {
    _cb("_d_modrm");
    _chr(_kdf_modrm(context, regbyte, rmbyte));
    if (!instruction_state.data.flagMem && instruction_state.data.flagLock) {
        _bb("flagMem(0),flagLock(1)");
        _chr(_SetExcept_UD(0));
        _be;
    }
    _ce;
}

/* execution control unit: _e_ */
/* kernel execution control */
static void _kec_push(core_machine_cpu_execution_context *context, t_vaddrcc rdata, t_nubit8 byte) {
    t_nubit32 cesp;
    _cb("_kec_push");
    _chr(_s_test_ss_push(context, byte));
    switch (_GetStackSize) {
    case 2:
        cesp = GetMax16(cpu_state.data.sp - byte);
        break;
    case 4:
        cesp = GetMax32(cpu_state.data.esp - byte);
        break;
    default:
        _impossible_r_;
        break;
    }
    _chr(_s_write_ss(context, cesp, rdata, byte));
    switch (_GetStackSize) {
    case 2:
        cpu_state.data.sp -= byte;
        break;
    case 4:
        cpu_state.data.esp -= byte;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void _kec_pop(core_machine_cpu_execution_context *context, t_vaddrcc rdata, t_nubit8 byte) {
    t_nubit32 cesp;
    _cb("_kec_pop");
    switch (_GetStackSize) {
    case 2:
        cesp = cpu_state.data.sp;
        break;
    case 4:
        cesp = cpu_state.data.esp;
        break;
    default:
        _impossible_r_;
        break;
    }
    _chr(_s_read_ss(context, cesp, rdata, byte));
    if (rdata != GetRef(cpu_state.data.esp)) {
        switch (_GetStackSize) {
        case 2:
            cpu_state.data.sp += byte;
            break;
        case 4:
            cpu_state.data.esp += byte;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    _ce;
}
static void _kec_call_far(core_machine_cpu_execution_context *context, t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    t_nubit32 oldcs = cpu_state.data.cs.selector;
    t_cpu_data_sreg ccs = cpu_state.data.cs;
    _cb("_kec_call_far");
    switch (byte) {
    case 2:
        _bb("byte(2)");
        _chr(_s_test_ss_push(context, 4));
        neweip = GetMax16(neweip);
        _chr(_ksa_load_sreg(context, &ccs, newcs));
        _chr(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
        _chr(_kec_push(context, GetRef(oldcs), 2));
        _chr(_kec_push(context, GetRef(cpu_state.data.ip), 2));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_s_test_ss_push(context, 8));
        neweip = GetMax32(neweip);
        _chr(_ksa_load_sreg(context, &ccs, newcs));
        _chr(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
        _chr(_kec_push(context, GetRef(oldcs), 4));
        _chr(_kec_push(context, GetRef(cpu_state.data.eip), 4));
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    cpu_state.data.cs = ccs;
    cpu_state.data.eip = neweip;
    _ce;
}
static void _kec_call_near(core_machine_cpu_execution_context *context, t_nubit32 neweip, t_nubit8 byte) {
    _cb("_kec_call_near");
    switch (byte) {
    case 2:
        _bb("byte(2)");
        neweip = GetMax16(neweip);
        _chr(_s_test_cs(context, neweip, 0x01));
        _chr(_kec_push(context, GetRef(cpu_state.data.ip), 2));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        neweip = GetMax32(neweip);
        _chr(_s_test_cs(context, neweip, 0x01));
        _chr(_kec_push(context, GetRef(cpu_state.data.eip), 4));
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    cpu_state.data.eip = neweip;
    _ce;
}
_______todo _kec_task_switch(t_nubit16 newtss);
static void _kec_jmp_far(core_machine_cpu_execution_context *context, t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    t_cpu_data_sreg ccs = cpu_state.data.cs;
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
    _chr(_ksa_load_sreg(context, &ccs, newcs));
    _chr(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
    cpu_state.data.cs = ccs;
    cpu_state.data.eip = neweip;
    _ce;
}
static void _kec_jmp_near(core_machine_cpu_execution_context *context, t_nubit32 neweip, t_nubit8 byte) {
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
    _chr(_s_test_cs(context, neweip, 0x01));
    cpu_state.data.eip = neweip;
    _ce;
}
static void _kec_ret_far(core_machine_cpu_execution_context *context, t_nubit32 newcs, t_nubit32 neweip, t_nubit16 parambyte, t_nubit16 byte) {
    t_cpu_data_sreg ccs = cpu_state.data.cs;
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
    _chr(_ksa_load_sreg(context, &ccs, newcs));
    _chr(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
    cpu_state.data.cs = ccs;
    cpu_state.data.eip = neweip;
    switch (_GetStackSize) {
    case 2:
        cpu_state.data.sp += parambyte;
        break;
    case 4:
        cpu_state.data.esp += parambyte;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void _kec_ret_near(core_machine_cpu_execution_context *context, t_nubit16 parambyte, t_nubit8 byte) {
    t_nubit32 neweip = 0;
    _cb("_kec_ret_near");
    switch (byte) {
    case 2:
        _bb("byte(2)");
        _chr(_kec_pop(context, GetRef(neweip), 2));
        _chr(_s_test_cs(context, neweip, 0x01));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_kec_pop(context, GetRef(neweip), 4));
        _chr(_s_test_cs(context, neweip, 0x01));
        _be;
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    cpu_state.data.eip = neweip;
    switch (_GetStackSize) {
    case 2:
        cpu_state.data.sp += parambyte;
        break;
    case 4:
        cpu_state.data.esp += parambyte;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
/* sub execution routine */
static void _ser_call_far_real(core_machine_cpu_execution_context *context, t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    _cb("_ser_call_far_real");
    if (_IsProtected) _impossible_r_;
    _chr(_kec_call_far(context, newcs, neweip, byte));
    _ce;
}
_______todo _ser_call_far_cs_conf(core_machine_cpu_execution_context *context, t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    t_nubit64 descriptor;
    _cb("_ser_call_far_cs_conf");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(context, newcs, GetRef(descriptor)));
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
    _chr(_kec_call_far(context, newcs, neweip, byte));
    _ce;
}
_______todo _ser_call_far_cs_nonc(core_machine_cpu_execution_context *context, t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    t_nubit64 descriptor;
    _cb("_ser_call_far_cs_nonc");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(context, newcs, GetRef(descriptor)));
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
    _chr(_kec_call_far(context, newcs, neweip, byte));
    _ce;
}
_______todo _ser_call_far_call_gate(core_machine_cpu_execution_context *context, t_nubit16 newcs) {
    t_nubit64 descriptor;
    _cb("_ser_call_far_call_gate");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(context, newcs, GetRef(descriptor)));
    if (!_IsDescCallGate(descriptor)) _impossible_r_;
    _chr(_SetExcept_CE(0));
    _ce;
}
_______todo _ser_call_far_task_gate(core_machine_cpu_execution_context *context, t_nubit16 newcs) {
    t_nubit64 descriptor;
    _cb("_ser_call_far_task_gate");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(context, newcs, GetRef(descriptor)));
    if (!_IsDescTaskGate(descriptor)) _impossible_r_;
    _chr(_SetExcept_CE(0));
    _ce;
}
_______todo _ser_call_far_tss(core_machine_cpu_execution_context *context, t_nubit16 newcs) {
    t_nubit64 descriptor;
    _cb("_ser_call_far_tss");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(context, newcs, GetRef(descriptor)));
    if (!_IsDescTSS(descriptor)) _impossible_r_;
    _chr(_SetExcept_CE(0));
    _ce;
}
static void _ser_int_real(core_machine_cpu_execution_context *context, t_nubit8 intid, t_nubit8 byte) {
    t_nubit16 cip;
    t_nubit32 vector;
    t_nubit32 oldcs = cpu_state.data.cs.selector;
    _cb("_ser_int_real");
    if (_IsProtected) _impossible_r_;
    if (GetMax16(intid * 4 + 3) > GetMax16(cpu_state.data.idtr.limit)) {
        _bb("intid(>idtr.limit)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    switch (byte) {
    case 2:
        _bb("byte(2)");
        _chr(_s_test_ss_push(context, 6));
        _chr(_kec_push(context, GetRef(cpu_state.data.flags), 2));
        _ClrEFLAGS_IF;
        _ClrEFLAGS_TF;
        _chr(_kec_push(context, GetRef(oldcs), 2));
        _chr(_kec_push(context, GetRef(cpu_state.data.ip), 2));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_s_test_ss_push(context, 12));
        _chr(_kec_push(context, GetRef(cpu_state.data.eflags), 4));
        _ClrEFLAGS_IF;
        _ClrEFLAGS_TF;
        _chr(_kec_push(context, GetRef(oldcs), 4));
        _chr(_kec_push(context, GetRef(cpu_state.data.eip), 4));
        _be;
        break;
    default:
        _bb("byte");
        _chr(_SetExcept_CE(byte));
        _be;
        break;
    }
    _chr(_s_read_idt(context, intid, GetRef(vector)));
    cip = GetMax16(vector);
    _chr(_s_test_cs(context, cip, 0x01));
    cpu_state.data.eip = cip;
    _chr(_s_load_cs(context, GetMax16(vector >> 16)));
    _ce;
}
_______todo _ser_int_protected(core_machine_cpu_execution_context *context, t_nubit8 intid, t_nubit8 byte, t_bool flagext) {
    t_nubit16 oldss;
    t_nubit32 oldeflags, oldesp, newesp = Zero32;
    t_nubit32 xs_sel;
    t_nubit16 newcs, newss = Zero16;
    t_nubit64 cs_desc, ss_desc, gate_desc;
    _cb("_ser_int_protected");
    if (!_GetCR0_PE) _impossible_r_;
    if (GetMax16(intid * 8 + 7) > GetMax16(cpu_state.data.idtr.limit)) {
        _bb("intid(>idtr.limit)");
        _chr(_SetExcept_GP(intid * 8 + 2 + !!flagext));
        _be;
    }
    _chr(_s_read_idt(context, intid, GetRef(gate_desc)));
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
            if (GetMax16(_GetSelector_Offset(newcs) + 7) > GetMax16(cpu_state.data.ldtr.limit)) {
                _bb("Selector_Offset(>ldtr.limit)");
                _chr(_SetExcept_GP(newcs & 0xfffc + !!flagext));
                _be;
            }
            _be;
        } else {
            _bb("Selector_TI(0)");
            if (GetMax16(_GetSelector_Offset(newcs) + 7) > GetMax16(cpu_state.data.gdtr.limit)) {
                _bb("Selector_Offset(>gdtr.limit)");
                _chr(_SetExcept_GP(newcs & 0xfffc + !!flagext));
                _be;
            }
            _be;
        }
        _chr(_s_read_xdt(context, newcs, GetRef(cs_desc)));
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
                switch (cpu_state.data.tr.sys.type) {
                case VCPU_DESC_SYS_TYPE_TSS_32_AVL:
                case VCPU_DESC_SYS_TYPE_TSS_32_BUSY:
                    _bb("tr.type(32)");
                    _chr(_s_test_tss(context, 4, 8));
                    _chr(_s_read_tss(context, 4, GetRef(newesp), 4));
                    _chr(_s_read_tss(context, 8, GetRef(newss), 2));
                    _be;
                    break;
                case VCPU_DESC_SYS_TYPE_TSS_16_AVL:
                case VCPU_DESC_SYS_TYPE_TSS_16_BUSY:
                    _bb("tr.type(16)");
                    _chr(_s_test_tss(context, 2, 4));
                    _chr(_s_read_tss(context, 2, GetRef(newesp), 2));
                    _chr(_s_read_tss(context, 4, GetRef(newss), 2));
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
                _chr(_s_read_xdt(context, newss, GetRef(ss_desc)));
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
                /*_chr(_ksa_load_sreg(context, &css, &ss_xdesc));
                if (_IsDescSys32(gate_desc)) {
                    _bb("DescSys32(gate)");
                    _chr(_kma_test_logical(context, &css, newesp, 36, 1, 0x00, 0));
                    _be;
                } else {
                    _bb("!DescSys32(gate)");
                    _chr(_kma_test_logical(context, &css, newesp, 18, 1, 0x00, 0));
                    _be;
                }*/
                oldeflags = cpu_state.data.eflags;
                _ClrEFLAGS_VM;
                _MakeCPL(0x00);
                _ClrEFLAGS_TF;
                _ClrEFLAGS_NT;
                _ClrEFLAGS_RF;
                if (_IsDescIntGate(gate_desc))
                    _ClrEFLAGS_IF;
                oldss = cpu_state.data.ss.selector;
                oldesp = cpu_state.data.esp;
                _chr(_ksa_load_sreg(context, &cpu_state.data.ss, newss));
                cpu_state.data.esp = newesp;
                if (_IsDescSys32(gate_desc)) {
                    _bb("DescSys32(gate)");
                    xs_sel = cpu_state.data.gs.selector;
                    _chr(_kec_push(context, GetRef(xs_sel), 4));
                    xs_sel = cpu_state.data.fs.selector;
                    _chr(_kec_push(context, GetRef(xs_sel), 4));
                    xs_sel = cpu_state.data.ds.selector;
                    _chr(_kec_push(context, GetRef(xs_sel), 4));
                    xs_sel = cpu_state.data.es.selector;
                    _chr(_kec_push(context, GetRef(xs_sel), 4));
                    xs_sel = oldss;
                    _chr(_kec_push(context, GetRef(xs_sel), 4));
                    _chr(_kec_push(context, GetRef(oldesp), 4));
                    _chr(_kec_push(context, GetRef(oldeflags), 4));
                    xs_sel = cpu_state.data.cs.selector;
                    _chr(_kec_push(context, GetRef(xs_sel), 4));
                    _chr(_kec_push(context, GetRef(cpu_state.data.eip), 4));
                    _chr(_s_load_gs(context, 0x0000));
                    _chr(_s_load_fs(context, 0x0000));
                    _chr(_s_load_ds(context, 0x0000));
                    _chr(_s_load_es(context, 0x0000));
                    _chr(_ksa_load_sreg(context, &cpu_state.data.cs, newcs));
                    switch (byte) {
                    case 2:
                        cpu_state.data.eip = GetMax16(_GetDescGate_Offset(gate_desc));
                        break;
                    case 4:
                        cpu_state.data.eip = GetMax32(_GetDescGate_Offset(gate_desc));
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
static void _ser_ret_far_real(core_machine_cpu_execution_context *context, t_nubit16 newcs, t_nubit32 neweip, t_nubit16 parambyte, t_nubit16 byte) {
    _cb("_ser_ret_far_real");
    if (_IsProtected) _impossible_r_;
    _chr(_kec_ret_far(context, newcs, neweip, parambyte, byte));
    _ce;
}
static void _ser_ret_far_same(core_machine_cpu_execution_context *context, t_nubit16 newcs, t_nubit32 neweip, t_nubit16 parambyte, t_nubit16 byte) {
    _cb("_ser_ret_far_same");
    if (!_IsProtected) _impossible_r_;
    _chr(_kec_ret_far(context, newcs, neweip, parambyte, byte));
    _ce;
}
_______todo _ser_ret_far_outer(core_machine_cpu_execution_context *context, t_nubit16 newcs, t_nubit32 neweip, t_nubit16 parambyte, t_nubit16 byte) {
    _cb("_ser_ret_far_outer");
    if (!_IsProtected) _impossible_r_;
    _chr(_SetExcept_CE(0));
    _ce;
}
static void _ser_jmp_far_real(core_machine_cpu_execution_context *context, t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    _cb("_ser_jmp_far_real");
    if (_IsProtected) _impossible_r_;
    _chr(_kec_jmp_far(context, newcs, neweip, byte));
    _ce;
}
static void _ser_jmp_far_cs_conf(core_machine_cpu_execution_context *context, t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    t_nubit64 descriptor;
    _cb("_ser_jmp_far_cs_conf");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(context, newcs, GetRef(descriptor)));
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
    _chr(_kec_jmp_far(context, newcs, neweip, byte));
    _ce;
}
static void _ser_jmp_far_cs_nonc(core_machine_cpu_execution_context *context, t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    t_nubit64 descriptor;
    _cb("_ser_jmp_far_cs_nonc");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(context, newcs, GetRef(descriptor)));
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
    _chr(_kec_jmp_far(context, newcs, neweip, byte));
    _ce;
}
_______todo _ser_jmp_far_call_gate(core_machine_cpu_execution_context *context, t_nubit16 newcs) {
    t_nubit64 descriptor;
    _cb("_ser_jmp_far_call_gate");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(context, newcs, GetRef(descriptor)));
    if (!_IsDescCallGate(descriptor)) _impossible_r_;
    _chr(_SetExcept_CE(0));
    _ce;
}
_______todo _ser_jmp_far_task_gate(core_machine_cpu_execution_context *context, t_nubit16 newcs) {
    t_nubit64 descriptor;
    _cb("_ser_jmp_far_task_gate");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(context, newcs, GetRef(descriptor)));
    if (!_IsDescTaskGate(descriptor)) _impossible_r_;
    _chr(_SetExcept_CE(0));
    _ce;
}
_______todo _ser_jmp_far_tss(core_machine_cpu_execution_context *context, t_nubit16 newcs) {
    t_nubit64 descriptor;
    _cb("_ser_jmp_far_tss");
    if (!_IsProtected) _impossible_r_;
    _chr(_s_read_xdt(context, newcs, GetRef(descriptor)));
    if (!_IsDescTSS(descriptor)) _impossible_r_;
    _chr(_SetExcept_CE(0));
    _ce;
}
/* regular execute control */
static void _e_push(core_machine_cpu_execution_context *context, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_e_push");
    _chr(_kec_push(context, rdata, byte));
    _ce;
}
static void _e_pop(core_machine_cpu_execution_context *context, t_vaddrcc rdata, t_nubit8 byte) {
    _cb("_e_pop");
    _chr(_kec_pop(context, rdata, byte));
    _ce;
}
static void _e_call_far(core_machine_cpu_execution_context *context, t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    t_nubit64 descriptor;
    _cb("_e_call_far");
    if (!_IsProtected) {
        _bb("!Protected");
        _chr(_ser_call_far_real(context, newcs, neweip, byte));
        _be;
    } else {
        _bb("Protected");
        if (_IsSelectorNull(newcs)) {
            _bb("newcs(null)");
            _chr(_SetExcept_GP(0));
            _be;
        }
        _chr(_s_read_xdt(context, newcs, GetRef(descriptor)));
        if (_IsDescCodeConform(descriptor))
            _chr(_ser_call_far_cs_conf(context, newcs, neweip, byte));
        else if (_IsDescCodeNonConform(descriptor))
            _chr(_ser_call_far_cs_nonc(context, newcs, neweip, byte));
        else if (_IsDescCallGate(descriptor))
            _chr(_ser_call_far_call_gate(context, newcs));
        else if (_IsDescTaskGate(descriptor))
            _chr(_ser_call_far_task_gate(context, newcs));
        else if (_IsDescTSS(descriptor))
            _chr(_ser_call_far_tss(context, newcs));
        else {
            _bb("desc(invalid)");
            _chr(_SetExcept_GP(newcs));
            _be;
        }
        _be;
    }
    _ce;
}
static void _e_call_near(core_machine_cpu_execution_context *context, t_nubit32 neweip, t_nubit8 byte) {
    _cb("_e_call_near");
    _chr(_kec_call_near(context, neweip, byte));
    _ce;
}
_______todo _e_int3(core_machine_cpu_execution_context *context, t_nubit8 byte) {
    _cb("_e_int3");
    if (!_GetCR0_PE) {
        _bb("Real");
        _chr(_ser_int_real(context, 0x03, byte));
        _be;
    } else {
        _bb("!Real");
        _chr(_ser_int_protected(context, 0x03, byte, 0));
        _be;
    }
    _ce;
}
_______todo _e_into(core_machine_cpu_execution_context *context, t_nubit8 byte) {
    _cb("_e_into");
    if (_GetEFLAGS_OF) {
        _bb("EFLAGS_OF(1)");
        if (!_GetCR0_PE) {
            _bb("Real");
            _chr(_ser_int_real(context, 0x04, byte));
            _be;
        } else {
            _bb("!Real");
            _chr(_ser_int_protected(context, 0x04, byte, 0));
            _be;
        }
        _be;
    }
    _ce;
}
_______todo _e_int_n(core_machine_cpu_execution_context *context, t_nubit8 intid, t_nubit8 byte) {
    _cb("_e_int_n");
    if (!_GetCR0_PE) {
        _bb("Real");
        _chr(_ser_int_real(context, intid, byte));
        _be;
    } else {
        _bb("!Real");
        if (_GetEFLAGS_VM && _GetEFLAGS_IOPL < 3) {
            _bb("EFLAGAS_VM(1),IOPL(<3)");
            _chr(_SetExcept_GP(0));
            _be;
        } else {
            _bb("EFLAGS_VM(0)/IOPL(3)");
            _chr(_ser_int_protected(context, intid, byte, 0));
            _be;
        }
        _be;
    }
    _ce;
}
_______todo _e_intr_n(core_machine_cpu_execution_context *context, t_nubit8 intid, t_nubit8 byte) {
    _cb("_e_intr_n");
    if (!_GetCR0_PE) {
        _bb("Real");
        _chr(_ser_int_real(context, intid, byte));
        _be;
    } else {
        _bb("!Real");
        _chr(_ser_int_protected(context, intid, byte, 1));
        _be;
    }
    _ce;
}
_______todo _e_except_n(core_machine_cpu_execution_context *context, t_nubit8 exid, t_nubit8 byte) {
    _cb("_e_except_n");
    instruction_state.data.except &= ~(1 << exid);
    if (!_GetCR0_PE) {
        _bb("Real");
        _chr(_ser_int_real(context, exid, byte));
        _be;
    } else {
        _bb("!Real");
        _chr(_SetExcept_UD(0));
        _be;
    }
    _ce;
}
_______todo _e_iret(core_machine_cpu_execution_context *context, t_nubit8 byte) {
    t_nubit16 newcs, newss, newds, newes, newfs, newgs;
    t_nubit32 neweip = Zero32, newesp, neweflags = Zero32;
    t_nubit32 xs_sel;
    t_nubit32 mask = VCPU_EFLAGS_RESERVED;
    t_cpu_data_sreg ccs = cpu_state.data.cs;
    _cb("_e_iret");
    if (!_GetCR0_PE) {
        _bb("Real");
        switch (byte) {
        case 2:
            _bb("byte(2)");
            _chr(_s_test_ss_pop(context, 6));
            _chr(_kec_pop(context, GetRef(neweip), 2));
            _chr(_kec_pop(context, GetRef(xs_sel), 2));
            newcs = GetMax16(xs_sel);
            _chr(_kec_pop(context, GetRef(neweflags), 2));
            mask |= 0xffff0000;
            _be;
            break;
        case 4:
            _bb("byte(4)");
            _chr(_s_test_ss_pop(context, 12));
            _chr(_kec_pop(context, GetRef(neweip), 4));
            _chr(_kec_pop(context, GetRef(xs_sel), 4));
            newcs = GetMax16(xs_sel);
            _chr(_kec_pop(context, GetRef(neweflags), 4));
            /* cpu_state.data.eflags = (neweflags & 0x00257fd5) | (cpu_state.data.eflags & 0x001a0000); */
            _be;
            break;
        default:
            _bb("byte");
            _chr(_SetExcept_CE(byte));
            _be;
            break;
        }
        _chr(_ksa_load_sreg(context, &ccs, newcs));
        _chr(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
        cpu_state.data.cs = ccs;
        cpu_state.data.eip = neweip;
        cpu_state.data.eflags = (neweflags & ~mask) | (cpu_state.data.eflags & mask);
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
                    _chr(_s_test_ss_pop(context, 6));
                    _chr(_kec_pop(context, GetRef(neweip), 2));
                    _chr(_kec_pop(context, GetRef(xs_sel), 2));
                    newcs = GetMax16(xs_sel);
                    _chr(_kec_pop(context, GetRef(neweflags), 2));
                    mask |= (0xffff0000 | VCPU_EFLAGS_IOPL);
                    _be;
                    break;
                case 4:
                    _bb("byte(4)");
                    _chr(_s_test_ss_pop(context, 12));
                    _chr(_kec_pop(context, GetRef(neweip), 4));
                    _chr(_kec_pop(context, GetRef(xs_sel), 4));
                    newcs = GetMax16(xs_sel);
                    _chr(_kec_pop(context, GetRef(neweflags), 4));
                    mask |= (VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL);
                    _be;
                    break;
                default:
                    _bb("byte");
                    _chr(_SetExcept_CE(byte));
                    _be;
                    break;
                }
                _chr(_ksa_load_sreg(context, &ccs, newcs));
                _chr(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
                cpu_state.data.cs = ccs;
                cpu_state.data.eip = neweip;
                cpu_state.data.eflags = (neweflags & ~mask) | (cpu_state.data.eflags & mask);
                _be;
            } else {
                _bb("EFLAGS_IOPL(!3)");
                /* trap to virtual-8086 monitor */
                instruction_state.data.oldcpu = cpu_state;
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
                _chr(_s_test_ss_pop(context, 6));
                _chr(_kec_pop(context, GetRef(neweip), 2));
                _chr(_kec_pop(context, GetRef(xs_sel), 2));
                newcs = GetMax16(xs_sel);
                _chr(_kec_pop(context, GetRef(neweflags), 2));
                _be;
                break;
            case 4:
                _bb("byte(4)");
                _chr(_s_test_ss_pop(context, 12));
                _chr(_kec_pop(context, GetRef(neweip), 4));
                _chr(_kec_pop(context, GetRef(xs_sel), 4));
                newcs = GetMax16(xs_sel);
                _chr(_kec_pop(context, GetRef(neweflags), 4));
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
                _chr(_s_test_ss_pop(context, 24));
                cpu_state.data.eflags = (neweflags & ~mask) | (cpu_state.data.eflags & mask);
                _chr(_ksa_load_sreg(context, &ccs, newcs));
                _chr(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
                cpu_state.data.cs = ccs;
                cpu_state.data.eip = neweip;
                _chr(_kec_pop(context, GetRef(newesp), 4));
                _chr(_kec_pop(context, GetRef(xs_sel), 4));
                newss = GetMax16(xs_sel);
                _chr(_kec_pop(context, GetRef(xs_sel), 4));
                newes = GetMax16(xs_sel);
                _chr(_kec_pop(context, GetRef(xs_sel), 4));
                newds = GetMax16(xs_sel);
                _chr(_kec_pop(context, GetRef(xs_sel), 4));
                newfs = GetMax16(xs_sel);
                _chr(_kec_pop(context, GetRef(xs_sel), 4));
                newgs = GetMax16(xs_sel);
                _chr(_s_load_ss(context, newss));
                cpu_state.data.esp = newesp;
                _chr(_s_load_es(context, newes));
                _chr(_s_load_ds(context, newds));
                _chr(_s_load_fs(context, newfs));
                _chr(_s_load_gs(context, newgs));
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
static void _e_jcc(core_machine_cpu_execution_context *context, t_nubit32 csrc, t_nubit8 byte, t_bool condition) {
    t_nubit32 neweip = cpu_state.data.eip;
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
        _chr(_kec_jmp_near(context, neweip, _GetOperandSize));
        _be;
    }
    _ce;
}
static void _e_jmp_far(core_machine_cpu_execution_context *context, t_nubit16 newcs, t_nubit32 neweip, t_nubit8 byte) {
    t_nubit64 descriptor;
    _cb("_e_jmp_far");
    if (!_IsProtected) {
        _bb("Protected(0)");
        _chr(_ser_jmp_far_real(context, newcs, neweip, byte));
        _be;
    } else {
        _bb("Protected(1)");
        if (_IsSelectorNull(newcs)) {
            _bb("selector(null)");
            _chr(_SetExcept_GP(0));
            _be;
        }
        _chr(_s_read_xdt(context, newcs, GetRef(descriptor)));
        if (_IsDescCodeConform(descriptor))
            _chr(_ser_jmp_far_cs_conf(context, newcs, neweip, byte));
        else if (_IsDescCodeNonConform(descriptor))
            _chr(_ser_jmp_far_cs_nonc(context, newcs, neweip, byte));
        else if (_IsDescCallGate(descriptor))
            _chr(_ser_jmp_far_call_gate(context, newcs));
        else if (_IsDescTaskGate(descriptor))
            _chr(_ser_jmp_far_task_gate(context, newcs));
        else if (_IsDescTSS(descriptor))
            _chr(_ser_jmp_far_tss(context, newcs));
        else {
            _bb("newcs(invalid)");
            _chr(_SetExcept_GP(newcs));
            _be;
        }
        _be;
    }
    instruction_state.data.opr1 = cpu_state.data.cs.selector;
    instruction_state.data.opr2 = cpu_state.data.eip;
    _ce;
}
static void _e_jmp_near(core_machine_cpu_execution_context *context, t_nubit32 neweip, t_nubit8 byte) {
    _cb("_e_jmp_near");
    _chr(_kec_jmp_near(context, neweip, byte));
    _ce;
}
static void _e_load_far(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, t_vaddrcc rdest, t_nubit16 selector, t_nubit32 offset, t_nubit8 byte) {
    _cb("_e_load_far");
    _chr(_s_load_sreg(context, rsreg, selector));
    switch (byte) {
    case 2:
        _bb("byte(2)");
        _chr(_m_write_ref(context, rdest, GetRef(offset), 2));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_m_write_ref(context, rdest, GetRef(offset), 4));
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
static void _e_loopcc(core_machine_cpu_execution_context *context, t_nsbit8 csrc, t_bool condition) {
    t_nubit32 cecx;
    t_nubit32 neweip = cpu_state.data.eip;
    _cb("_e_loopcc");
    switch (_GetAddressSize) {
    case 2:
        cpu_state.data.cx--;
        cecx = GetMax16(cpu_state.data.ecx);
        break;
    case 4:
        cpu_state.data.ecx--;
        cecx = GetMax32(cpu_state.data.ecx);
        break;
    default:
        _impossible_r_;
        break;
    }
    if (cecx && condition) {
        _bb("cecx(!0),condition(1)");
        neweip += csrc;
        _chr(_kec_jmp_near(context, neweip, _GetOperandSize));
        _be;
    }
    _ce;
}
static void _e_ret_near(core_machine_cpu_execution_context *context, t_nubit16 parambyte, t_nubit8 byte) {
    _cb("_e_ret_near");
    _chr(_kec_ret_near(context, parambyte, byte));
    _ce;
}
static void _e_ret_far(core_machine_cpu_execution_context *context, t_nubit16 parambyte, t_nubit16 byte) {
    t_nubit16 newcs;
    t_nubit32 xs_sel;
    t_nubit32 neweip = 0;
    t_nubit64 descriptor;
    _cb("_e_ret_far");
    switch (byte) {
    case 2:
        _bb("byte(2)");
        _chr(_s_test_ss_pop(context, 4));
        _chr(_kec_pop(context, GetRef(neweip), 2));
        _chr(_kec_pop(context, GetRef(xs_sel), 2));
        newcs = GetMax16(xs_sel);
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_s_test_ss_pop(context, 8));
        _chr(_kec_pop(context, GetRef(neweip), 4));
        _chr(_kec_pop(context, GetRef(xs_sel), 4));
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
        _chr(_ser_ret_far_real(context, newcs, neweip, parambyte, byte));
        _be;
    } else {
        _bb("Protected");
        if (_IsSelectorNull(newcs)) {
            _bb("selector(null)");
            _chr(_SetExcept_GP(0));
            _be;
        }
        _chr(_s_read_xdt(context, newcs, GetRef(descriptor)));
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
            _chr(_ser_ret_far_outer(context, newcs, neweip, parambyte, byte));
            _be;
        } else {
            _bb("RPL(<=CPL)");
            _chr(_ser_ret_far_same(context, newcs, neweip, parambyte, byte));
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

static void _kaf_calc_CF(core_machine_cpu_execution_context *context) {
    _cb("_kaf_calc_CF");
    switch (instruction_state.data.type) {
    case ADC8:
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, (
                    (_GetEFLAGS_CF && instruction_state.data.opr2 == Max8) ?
                    1 : ((instruction_state.data.result < instruction_state.data.opr1) || (instruction_state.data.result < instruction_state.data.opr2))));
        break;
    case ADC16:
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, (
                    (_GetEFLAGS_CF && instruction_state.data.opr2 == Max16) ?
                    1 : ((instruction_state.data.result < instruction_state.data.opr1) || (instruction_state.data.result < instruction_state.data.opr2))));
        break;
    case ADC32:
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, (
                    (_GetEFLAGS_CF && instruction_state.data.opr2 == Max32) ?
                    1 : ((instruction_state.data.result < instruction_state.data.opr1) || (instruction_state.data.result < instruction_state.data.opr2))));
        break;
    case ADD8:
    case ADD16:
    case ADD32:
        MakeBit(cpu_state.data.eflags,VCPU_EFLAGS_CF,(instruction_state.data.result < instruction_state.data.opr1) || (instruction_state.data.result < instruction_state.data.opr2));
        break;
    case SBB8:
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, (instruction_state.data.opr1 < instruction_state.data.result) ||
                (_GetEFLAGS_CF && (instruction_state.data.opr2 == Max8)));
        break;
    case SBB16:
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, (instruction_state.data.opr1 < instruction_state.data.result) ||
                (_GetEFLAGS_CF && (instruction_state.data.opr2 == Max16)));
        break;
    case SBB32:
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, (instruction_state.data.opr1 < instruction_state.data.result) ||
                (_GetEFLAGS_CF && (instruction_state.data.opr2 == Max32)));
        break;
    case SUB8:
    case SUB16:
    case SUB32:
    case CMP8:
    case CMP16:
    case CMP32:
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, instruction_state.data.opr1 < instruction_state.data.opr2);
        break;
    default:
        _bb("type");
        _chr(_SetExcept_CE(instruction_state.data.type));
        _be;
        break;
    }
    _ce;
}
static void _kaf_calc_OF(core_machine_cpu_execution_context *context) {
    _cb("_kaf_calc_OF");
    switch (instruction_state.data.type) {
    case ADC8:
    case ADD8:
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                (GetMSB8(instruction_state.data.opr1) == GetMSB8(instruction_state.data.opr2)) && (GetMSB8(instruction_state.data.opr1) != GetMSB8(instruction_state.data.result)));
        break;
    case ADC16:
    case ADD16:
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                (GetMSB16(instruction_state.data.opr1) == GetMSB16(instruction_state.data.opr2)) && (GetMSB16(instruction_state.data.opr1) != GetMSB16(instruction_state.data.result)));
        break;
    case ADC32:
    case ADD32:
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                (GetMSB32(instruction_state.data.opr1) == GetMSB32(instruction_state.data.opr2)) && (GetMSB32(instruction_state.data.opr1) != GetMSB32(instruction_state.data.result)));
        break;
    case SBB8:
    case SUB8:
    case CMP8:
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                (GetMSB8(instruction_state.data.opr1) != GetMSB8(instruction_state.data.opr2)) && (GetMSB8(instruction_state.data.opr2) == GetMSB8(instruction_state.data.result)));
        break;
    case SBB16:
    case SUB16:
    case CMP16:
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                (GetMSB16(instruction_state.data.opr1) != GetMSB16(instruction_state.data.opr2)) && (GetMSB16(instruction_state.data.opr2) == GetMSB16(instruction_state.data.result)));
        break;
    case SBB32:
    case SUB32:
    case CMP32:
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                (GetMSB32(instruction_state.data.opr1) != GetMSB32(instruction_state.data.opr2)) && (GetMSB32(instruction_state.data.opr2) == GetMSB32(instruction_state.data.result)));
        break;
    default:
        _bb("type");
        _chr(_SetExcept_CE(instruction_state.data.type));
        _be;
        break;
    }
    _ce;
}
static void _kaf_calc_AF(core_machine_cpu_execution_context *context) {
    _cb("_kaf_calc_AF");
    MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_AF, ((instruction_state.data.opr1 ^ instruction_state.data.opr2) ^ instruction_state.data.result) & 0x10);
    _ce;
}
static void _kaf_calc_PF(core_machine_cpu_execution_context *context) {
    t_nubit8 res8 = GetMax8(instruction_state.data.result);
    t_bool even = 1;
    _cb("_kaf_calc_PF");
    while (res8) {
        even = 1 - even;
        res8 &= res8-1;
    }
    MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_PF, even);
    _ce;
}
static void _kaf_calc_ZF(core_machine_cpu_execution_context *context) {
    _cb("_kaf_calc_ZF");
    MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_ZF, !instruction_state.data.result);
    _ce;
}
static void _kaf_calc_SF(core_machine_cpu_execution_context *context) {
    _cb("_kaf_calc_SF");
    switch (instruction_state.data.bit) {
    case 8:
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_SF, GetMSB8(instruction_state.data.result));
        break;
    case 16:
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_SF, GetMSB16(instruction_state.data.result));
        break;
    case 32:
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_SF, GetMSB32(instruction_state.data.result));
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(instruction_state.data.bit));
        _be;
        break;
    }
    _ce;
}
static void _kaf_set_flags(core_machine_cpu_execution_context *context, t_nubit16 flags) {
    _cb("_kaf_set_flags");
    if (flags & VCPU_EFLAGS_CF) _chr(_kaf_calc_CF(context));
    if (flags & VCPU_EFLAGS_PF) _chr(_kaf_calc_PF(context));
    if (flags & VCPU_EFLAGS_AF) _chr(_kaf_calc_AF(context));
    if (flags & VCPU_EFLAGS_ZF) _chr(_kaf_calc_ZF(context));
    if (flags & VCPU_EFLAGS_SF) _chr(_kaf_calc_SF(context));
    if (flags & VCPU_EFLAGS_OF) _chr(_kaf_calc_OF(context));
    _ce;
}
static void _kas_move_index(core_machine_cpu_execution_context *context, t_nubit8 byte, t_bool flagsi, t_bool flagdi) {
    _cb("_kas_move_index");
    switch (_GetAddressSize) {
    case 2:
        if (_GetEFLAGS_DF) {
            if (flagdi) cpu_state.data.di -= byte;
            if (flagsi) cpu_state.data.si -= byte;
        } else {
            if (flagdi) cpu_state.data.di += byte;
            if (flagsi) cpu_state.data.si += byte;
        }
        break;
    case 4:
        if (_GetEFLAGS_DF) {
            if (flagdi) cpu_state.data.edi -= byte;
            if (flagsi) cpu_state.data.esi -= byte;
        } else {
            if (flagdi) cpu_state.data.edi += byte;
            if (flagsi) cpu_state.data.esi += byte;
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
        instruction_state.data.bit = 8; \
        instruction_state.data.type = (type8); \
        instruction_state.data.opr1 = GetMax8(cdest); \
        instruction_state.data.result = GetMax8(expr8); \
        break; \
    case 16: \
        instruction_state.data.bit = 16; \
        instruction_state.data.type = (type16); \
        instruction_state.data.opr1 = GetMax16(cdest); \
        instruction_state.data.result = GetMax16(expr16); \
        break; \
    case 32: \
        instruction_state.data.bit = 32; \
        instruction_state.data.type = (type32); \
        instruction_state.data.opr1 = GetMax32(cdest); \
        instruction_state.data.result = GetMax32(expr32); \
        break; \
    default: _bb("bit"); \
        _chr(_SetExcept_CE(bit)); \
        _be;break; \
    } \
    _chr(_kaf_set_flags(context, funflag)); \
} while (0)
#define _kac_arith2(funflag, type8, expr8, type12, expr12, type16, expr16, \
    type20, expr20, type32, expr32) \
do { \
    switch (bit) { \
    case 8: \
        instruction_state.data.bit = 8; \
        instruction_state.data.type = (type8); \
        instruction_state.data.opr1 = GetMax8(cdest); \
        instruction_state.data.opr2 = GetMax8(csrc); \
        instruction_state.data.result = GetMax8(expr8); \
        break; \
    case 12: \
        instruction_state.data.bit = 16; \
        instruction_state.data.type = (type12);\
        instruction_state.data.opr1 = GetMax16(cdest); \
        instruction_state.data.opr2 = GetMax16((t_nsbit8)csrc); \
        instruction_state.data.result = GetMax16(expr12); \
        break; \
    case 16: \
        instruction_state.data.bit = 16; \
        instruction_state.data.type = (type16); \
        instruction_state.data.opr1 = GetMax16(cdest); \
        instruction_state.data.opr2 = GetMax16(csrc); \
        instruction_state.data.result = GetMax16(expr16); \
        break; \
    case 20: \
        instruction_state.data.bit = 32; \
        instruction_state.data.type = (type20); \
        instruction_state.data.opr1 = GetMax32(cdest); \
        instruction_state.data.opr2 = GetMax32((t_nsbit8)csrc); \
        instruction_state.data.result = GetMax32(expr20); \
        break; \
    case 32: \
        instruction_state.data.bit = 32; \
        instruction_state.data.type = (type32); \
        instruction_state.data.opr1 = GetMax32(cdest); \
        instruction_state.data.opr2 = GetMax32(csrc); \
        instruction_state.data.result = GetMax32(expr32); \
        break; \
    default: _bb("bit"); \
        _chr(_SetExcept_CE(bit)); \
        _be;break; \
    } \
    _chr(_kaf_set_flags(context, funflag)); \
} while (0)
static void _a_add(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    _cb("_a_add");
    _kac_arith2(ADD_FLAG,
                ADD8,  (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD16, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD16, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD32, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD32, (instruction_state.data.opr1 + instruction_state.data.opr2));
    _ce;
}
static void _a_adc(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    _cb("_a_adc");
    _kac_arith2(ADC_FLAG,
                ADC8,  (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF),
                ADC16, (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF),
                ADC16, (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF),
                ADC32, (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF),
                ADC32, (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF));
    _ce;
}
static void _a_and(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    _cb("_a_and");
    _kac_arith2(AND_FLAG,
                AND8,  (instruction_state.data.opr1 & instruction_state.data.opr2),
                AND16, (instruction_state.data.opr1 & instruction_state.data.opr2),
                AND16, (instruction_state.data.opr1 & instruction_state.data.opr2),
                AND32, (instruction_state.data.opr1 & instruction_state.data.opr2),
                AND32, (instruction_state.data.opr1 & instruction_state.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    instruction_state.data.udf |= VCPU_EFLAGS_AF;
    _ce;
}
static void _a_or(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    _cb("_a_or");
    _kac_arith2(OR_FLAG,
                OR8,  (instruction_state.data.opr1 | instruction_state.data.opr2),
                OR16, (instruction_state.data.opr1 | instruction_state.data.opr2),
                OR16, (instruction_state.data.opr1 | instruction_state.data.opr2),
                OR32, (instruction_state.data.opr1 | instruction_state.data.opr2),
                OR32, (instruction_state.data.opr1 | instruction_state.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    instruction_state.data.udf |= VCPU_EFLAGS_AF;
    _ce;
}
static void _a_sbb(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    _cb("_a_sbb");
    _kac_arith2(SBB_FLAG,
                SBB8,  (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)),
                SBB16, (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)),
                SBB16, (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)),
                SBB32, (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)),
                SBB32, (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)));
    _ce;
}
static void _a_sub(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    _cb("_a_sub");
    _kac_arith2(SUB_FLAG,
                SUB8,  (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB16, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB16, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB32, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB32, (instruction_state.data.opr1 - instruction_state.data.opr2));
    _ce;
}
static void _a_xor(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    _cb("_a_xor");
    _kac_arith2(XOR_FLAG,
                XOR8,  (instruction_state.data.opr1 ^ instruction_state.data.opr2),
                XOR16, (instruction_state.data.opr1 ^ instruction_state.data.opr2),
                XOR16, (instruction_state.data.opr1 ^ instruction_state.data.opr2),
                XOR32, (instruction_state.data.opr1 ^ instruction_state.data.opr2),
                XOR32, (instruction_state.data.opr1 ^ instruction_state.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    instruction_state.data.udf |= VCPU_EFLAGS_AF;
    _ce;
}
static void _a_cmp(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    _cb("_a_cmp");
    _kac_arith2(CMP_FLAG,
                CMP8,  ((t_nubit8)instruction_state.data.opr1 - (t_nsbit8)instruction_state.data.opr2),
                CMP16, ((t_nubit16)instruction_state.data.opr1 - (t_nsbit8)instruction_state.data.opr2),
                CMP16, ((t_nubit16)instruction_state.data.opr1 - (t_nsbit16)instruction_state.data.opr2),
                CMP32, ((t_nubit32)instruction_state.data.opr1 - (t_nsbit8)instruction_state.data.opr2),
                CMP32, ((t_nubit32)instruction_state.data.opr1 - (t_nsbit32)instruction_state.data.opr2));
    _ce;
}
static void _a_test(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    _cb("_a_test");
    _kac_arith2(TEST_FLAG,
                TEST8,  (instruction_state.data.opr1 & instruction_state.data.opr2),
                TEST16, (instruction_state.data.opr1 & instruction_state.data.opr2),
                TEST16, (instruction_state.data.opr1 & instruction_state.data.opr2),
                TEST32, (instruction_state.data.opr1 & instruction_state.data.opr2),
                TEST32, (instruction_state.data.opr1 & instruction_state.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    instruction_state.data.udf |= VCPU_EFLAGS_AF;
    _ce;
}
static void _a_inc(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit8 bit) {
    _cb("_a_inc");
    instruction_state.data.opr2 = 1;
    _kac_arith1(INC_FLAG,
                ADD8,  (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD16, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD32, (instruction_state.data.opr1 + instruction_state.data.opr2));
    _ce;
}
static void _a_dec(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit8 bit) {
    _cb("_a_dec");
    instruction_state.data.opr2 = 1;
    _kac_arith1(DEC_FLAG,
                SUB8,  (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB16, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB32, (instruction_state.data.opr1 - instruction_state.data.opr2));
    _ce;
}
static void _a_not(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit8 bit) {
    _cb("_a_not");
    instruction_state.data.opr2 = 0;
    _kac_arith1(NOT_FLAG,
                ARITHTYPE_NULL, (~instruction_state.data.opr1),
                ARITHTYPE_NULL, (~instruction_state.data.opr1),
                ARITHTYPE_NULL, (~instruction_state.data.opr1));
    _ce;
}
static void _a_neg(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit8 bit) {
    _cb("_a_neg");
    instruction_state.data.opr2 = cdest;
    cdest = 0;
    _kac_arith1(NEG_FLAG,
                SUB8,  (GetMax8(instruction_state.data.opr1) - GetMax8(instruction_state.data.opr2)),
                SUB16, (GetMax16(instruction_state.data.opr1) - GetMax16(instruction_state.data.opr2)),
                SUB32, (GetMax32(instruction_state.data.opr1) - GetMax32(instruction_state.data.opr2)));
    MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!instruction_state.data.opr2);
    _ce;
}
static void _a_mul(core_machine_cpu_execution_context *context, t_nubit64 csrc, t_nubit8 bit) {
    t_nubit64 cdest;
    _cb("_a_mul");
    switch (bit) {
    case 8:
        _bb("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = cpu_state.data.al;
        instruction_state.data.opr2 = GetMax8(csrc);
        cdest = GetMax16(cpu_state.data.al * instruction_state.data.opr2);
        cpu_state.data.ax = GetMax16(cdest);
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF, !!cpu_state.data.ah);
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!cpu_state.data.ah);
        instruction_state.data.result = GetMax16(cdest);
        _be;
        break;
    case 16:
        _bb("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = cpu_state.data.ax;
        instruction_state.data.opr2 = GetMax16(csrc);
        cdest = GetMax32(cpu_state.data.ax * instruction_state.data.opr2);
        cpu_state.data.dx = GetMax16(cdest >> 16);
        cpu_state.data.ax = GetMax16(cdest);
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF, !!cpu_state.data.dx);
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!cpu_state.data.dx);
        instruction_state.data.result = GetMax32(cdest);
        _be;
        break;
    case 32:
        _bb("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = cpu_state.data.eax;
        instruction_state.data.opr2 = GetMax32(csrc);
        cdest = GetMax64(cpu_state.data.eax * instruction_state.data.opr2);
        cpu_state.data.edx = GetMax32(cdest >> 32);
        cpu_state.data.eax = GetMax32(cdest);
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF, !!cpu_state.data.edx);
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!cpu_state.data.edx);
        instruction_state.data.result = GetMax64(cdest);
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                         VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    _ce;
}
static void _a_imul(core_machine_cpu_execution_context *context, t_nubit64 csrc, t_nubit8 bit) {
    t_nsbit64 cdest;
    _cb("_a_imul");
    switch (bit) {
    case 8:
        _bb("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = cpu_state.data.al;
        instruction_state.data.opr2 = GetMax8((t_nsbit8)csrc);
        cdest = GetMax16((t_nsbit8)cpu_state.data.al * (t_nsbit8)instruction_state.data.opr2);
        cpu_state.data.ax = GetMax16(cdest);
        if (GetMax16(cdest) == (t_nsbit16)((t_nsbit8)cpu_state.data.al)) {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        } else {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        instruction_state.data.result = GetMax16(cdest);
        _be;
        break;
    case 16:
        _bb("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = cpu_state.data.ax;
        instruction_state.data.opr2 = GetMax16((t_nsbit16)csrc);
        cdest = GetMax32((t_nsbit16)cpu_state.data.ax * (t_nsbit16)instruction_state.data.opr2);
        cpu_state.data.ax = GetMax16(cdest);
        cpu_state.data.dx = GetMax16(cdest >> 16);
        if (GetMax32(cdest) == (t_nsbit32)((t_nsbit16)cpu_state.data.ax)) {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        } else {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        instruction_state.data.result = GetMax32(cdest);
        _be;
        break;
    case 32:
        _bb("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = cpu_state.data.eax;
        instruction_state.data.opr2 = GetMax32((t_nsbit32)csrc);
        cdest = GetMax64((t_nsbit32)cpu_state.data.eax * (t_nsbit32)instruction_state.data.opr2);
        cpu_state.data.eax = GetMax32(cdest);
        cpu_state.data.edx = GetMax32(cdest >> 32);
        if (GetMax64(cdest) == (t_nsbit64)((t_nsbit32)cpu_state.data.eax)) {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        } else {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        instruction_state.data.result = GetMax64(cdest);
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                         VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    _ce;
}
static void _a_div(core_machine_cpu_execution_context *context, t_nubit64 csrc, t_nubit8 bit) {
    t_nubit64 temp = 0x0000000000000000;
    _cb("_a_div");
    switch (bit) {
    case 8:
        _bb("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = GetMax16(cpu_state.data.ax);
        instruction_state.data.opr2 = GetMax8(csrc);
        if (!instruction_state.data.opr2) {
            _bb("opr2(0)");
            _chr(_SetExcept_DE(0));
            _be;
        } else {
            _bb("opr2(!0)");
            temp = GetMax16((t_nubit16)instruction_state.data.opr1 / (t_nubit8)instruction_state.data.opr2);
            if (temp > Max8) {
                _bb("temp(>0xff)");
                _chr(_SetExcept_DE(0));
                _be;
            } else {
                cpu_state.data.al = GetMax8(temp);
                cpu_state.data.ah = GetMax8((t_nubit16)instruction_state.data.opr1 % (t_nubit8)instruction_state.data.opr2);
            }
            _be;
        }
        instruction_state.data.result = cpu_state.data.ax;
        _be;
        break;
    case 16:
        _bb("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = GetMax32((cpu_state.data.dx << 16) | cpu_state.data.ax);
        instruction_state.data.opr2 = GetMax16(csrc);
        if (!instruction_state.data.opr2) {
            _bb("opr2(0)");
            _chr(_SetExcept_DE(0));
            _be;
        } else {
            _bb("opr2(!0)");
            temp = GetMax32((t_nubit32)instruction_state.data.opr1 / (t_nubit16)instruction_state.data.opr2);
            if (temp > Max16) {
                _bb("temp(>0xffff)");
                _chr(_SetExcept_DE(0));
                _be;
            } else {
                cpu_state.data.ax = GetMax16(temp);
                cpu_state.data.dx = GetMax16((t_nubit32)instruction_state.data.opr1 % (t_nubit16)instruction_state.data.opr2);
            }
            _be;
        }
        instruction_state.data.result = (cpu_state.data.dx << 16) | cpu_state.data.ax;
        _be;
        break;
    case 32:
        _bb("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = GetMax64(((t_nubit64)cpu_state.data.edx << 32) | cpu_state.data.eax);
        instruction_state.data.opr2 = GetMax32(csrc);
        if (!instruction_state.data.opr2) {
            _bb("opr2(0)");
            _chr(_SetExcept_DE(0));
            _be;
        } else {
            _bb("opr2(!0)");
            temp = GetMax64((t_nubit64)instruction_state.data.opr1 / (t_nubit32)instruction_state.data.opr2);
            if (temp > Max32) {
                _bb("temp(>0xffffffff)");
                _chr(_SetExcept_DE(0));
                _be;
            } else {
                cpu_state.data.eax = GetMax32(temp);
                cpu_state.data.edx = GetMax32((t_nubit64)instruction_state.data.opr1 % (t_nubit32)instruction_state.data.opr2);
            }
            _be;
        }
        instruction_state.data.result = ((t_nubit64)cpu_state.data.edx << 32) | cpu_state.data.eax;
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF |
                         VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_OF);
    _ce;
}
static void _a_idiv(core_machine_cpu_execution_context *context, t_nubit64 csrc, t_nubit8 bit) {
    t_nsbit64 temp;
    _cb("_a_idiv");
    switch (bit) {
    case 8:
        _bb("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = GetMax16((t_nsbit16)cpu_state.data.ax);
        instruction_state.data.opr2 = GetMax8((t_nsbit8)csrc);
        if (!instruction_state.data.opr2) {
            _bb("opr2(0)");
            _chr(_SetExcept_DE(0));
            _be;
        } else {
            _bb("opr2(!0)");
            temp = GetMax16((t_nsbit16)instruction_state.data.opr1 / (t_nsbit8)instruction_state.data.opr2);
            if ((temp > 0 && GetMax8(temp) > 0x7f) ||
                    (temp < 0 && GetMax8(temp) < 0x80)) {
                _bb("temp(>0x7f/<0x80)");
                _chr(_SetExcept_DE(0));
                _be;
            } else {
                cpu_state.data.al = GetMax8(temp);
                cpu_state.data.ah = GetMax8((t_nsbit16)instruction_state.data.opr1 % (t_nsbit8)instruction_state.data.opr2);
            }
            _be;
        }
        instruction_state.data.result = cpu_state.data.ax;
        _be;
        break;
    case 16:
        _bb("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = GetMax32((t_nsbit32)((cpu_state.data.dx << 16) | cpu_state.data.ax));
        instruction_state.data.opr2 = GetMax16((t_nsbit16)csrc);
        if (!instruction_state.data.opr2) {
            _bb("opr2(0)");
            _chr(_SetExcept_DE(0));
            _be;
        } else {
            _bb("opr2(!0)");
            temp = GetMax32((t_nsbit32)instruction_state.data.opr1 / (t_nsbit16)instruction_state.data.opr2);
            if ((temp > 0 && GetMax16(temp) > 0x7fff) ||
                    (temp < 0 && GetMax16(temp) < 0x8000)) {
                _bb("temp(>0x7fff/<0x8000)");
                _chr(_SetExcept_DE(0));
                _be;
            } else {
                cpu_state.data.ax = GetMax16(temp);
                cpu_state.data.dx = GetMax16((t_nsbit32)instruction_state.data.opr1 % (t_nsbit16)instruction_state.data.opr2);
            }
            _be;
        }
        instruction_state.data.result = ((t_nubit32)cpu_state.data.dx << 16) | cpu_state.data.ax;
        _be;
        break;
    case 32:
        _bb("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = GetMax64((t_nsbit64)(((t_nubit64)cpu_state.data.edx << 32) | cpu_state.data.eax));
        instruction_state.data.opr2 = GetMax32((t_nsbit32)csrc);
        if (!instruction_state.data.opr2) {
            _bb("opr2(0)");
            _chr(_SetExcept_DE(0));
            _be;
        } else {
            _bb("opr2(!0)");
            temp = GetMax64((t_nsbit64)instruction_state.data.opr1 / (t_nsbit32)instruction_state.data.opr2);
            if ((temp > 0 && GetMax32(temp) > 0x7fffffff) ||
                    (temp < 0 && GetMax32(temp) < 0x80000000)) {
                _bb("temp(>0x7fffffff/<0x80000000)");
                _chr(_SetExcept_DE(0));
                _be;
            } else {
                cpu_state.data.eax = GetMax32(temp);
                cpu_state.data.edx = GetMax32((t_nsbit64)instruction_state.data.opr1 % (t_nsbit32)instruction_state.data.opr2);
            }
            _be;
        }
        instruction_state.data.result = ((t_nubit64)cpu_state.data.edx << 32) | cpu_state.data.eax;
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF |
                         VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_OF);
    _ce;
}

static void _a_imul3(core_machine_cpu_execution_context *context, t_nubit64 csrc1, t_nubit64 csrc2, t_nubit8 bit) {
    t_nsbit64 cdest;
    _cb("_a_imul3");
    switch (bit) {
    case 12:
        _bb("bit(16+8)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = (t_nsbit16)csrc1;
        instruction_state.data.opr2 = (t_nsbit8)csrc2;
        cdest = GetMax32((t_nsbit16)instruction_state.data.opr1 * (t_nsbit8)instruction_state.data.opr2);
        instruction_state.data.result = GetMax16(cdest);
        if (GetMax32(cdest) != GetMax32((t_nsbit16)instruction_state.data.result)) {
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
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = (t_nsbit16)csrc1;
        instruction_state.data.opr2 = (t_nsbit16)csrc2;
        cdest = GetMax32((t_nsbit16)instruction_state.data.opr1 * (t_nsbit16)instruction_state.data.opr2);
        instruction_state.data.result = GetMax16(cdest);
        if (GetMax32(cdest) != GetMax32((t_nsbit16)instruction_state.data.result)) {
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
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = (t_nsbit32)csrc1;
        instruction_state.data.opr2 = (t_nsbit8)csrc2;
        cdest = GetMax64((t_nsbit32)instruction_state.data.opr1 * (t_nsbit8)instruction_state.data.opr2);
        instruction_state.data.result = GetMax32(cdest);
        if (GetMax64(cdest) != GetMax64((t_nsbit32)instruction_state.data.result)) {
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
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = (t_nsbit32)csrc1;
        instruction_state.data.opr2 = (t_nsbit32)csrc2;
        cdest = GetMax64((t_nsbit32)instruction_state.data.opr1 * (t_nsbit32)instruction_state.data.opr2);
        instruction_state.data.result = GetMax32(cdest);
        if (GetMax64(cdest) != GetMax64((t_nsbit32)instruction_state.data.result)) {
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
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                         VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    _ce;
}

static void _a_rol(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit) {
    t_nubit8 count;
    t_nubit32 flagcf;
    _cb("_a_rol");
    switch (bit) {
    case 8:
        _bb("bit(8)");
        count = csrc & 0x07;
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = GetMax8(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            flagcf = !!GetMSB8(instruction_state.data.result);
            instruction_state.data.result = GetMax8(instruction_state.data.result << 1) | flagcf;
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!GetLSB8(instruction_state.data.result));
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB8(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        _be;
        break;
    case 16:
        _bb("bit(16)");
        count = csrc & 0x0f;
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = GetMax16(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            flagcf = !!GetMSB16(instruction_state.data.result);
            instruction_state.data.result = GetMax16(instruction_state.data.result << 1) | flagcf;
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!GetLSB16(instruction_state.data.result));
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB16(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        _be;
        break;
    case 32:
        _bb("bit(32)");
        count = csrc & 0x1f;
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = GetMax32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            flagcf = !!GetMSB32(instruction_state.data.result);
            instruction_state.data.result = GetMax32(instruction_state.data.result << 1) | flagcf;
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!GetLSB32(instruction_state.data.result));
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB32(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
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
static void _a_ror(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit) {
    t_nubit8 count;
    t_nubit32 flagcf;
    _cb("_a_ror");
    switch (bit) {
    case 8:
        _bb("bit(8)");
        count = csrc & 0x07;
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = GetMax8(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            flagcf = GetLSB8(instruction_state.data.result) ? MSB8 : 0;
            instruction_state.data.result = GetMax8(instruction_state.data.result >> 1) | flagcf;
            count--;
        }
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!GetMSB8(instruction_state.data.result));
        if (instruction_state.data.opr2 == 1)
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB8(instruction_state.data.result)) ^ (!!GetMSB7(instruction_state.data.result))));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        _be;
        break;
    case 16:
        _bb("bit(16)");
        count = csrc & 0x0f;
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = GetMax16(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            flagcf = GetLSB16(instruction_state.data.result) ? MSB16 : 0;
            instruction_state.data.result = GetMax16(instruction_state.data.result >> 1) | flagcf;
            count--;
        }
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!GetMSB16(instruction_state.data.result));
        if (instruction_state.data.opr2 == 1)
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB16(instruction_state.data.result)) ^ (!!GetMSB15(instruction_state.data.result))));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        _be;
        break;
    case 32:
        _bb("bit(32)");
        count = csrc & 0x1f;
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = GetMax32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            flagcf = GetLSB32(instruction_state.data.result) ? MSB32 : 0;
            instruction_state.data.result = GetMax32(instruction_state.data.result >> 1) | flagcf;
            count--;
        }
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!GetMSB32(instruction_state.data.result));
        if (instruction_state.data.opr2 == 1)
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB32(instruction_state.data.result)) ^ (!!GetMSB31(instruction_state.data.result))));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
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
static void _a_rcl(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit) {
    t_nubit8 count;
    t_nubit32 flagcf;
    _cb("_a_rcl");
    switch (bit) {
    case 8:
        _bb("bit(8)");
        count = (csrc & 0x1f) % 9;
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = GetMax8(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            flagcf = !!GetMSB8(instruction_state.data.result);
            instruction_state.data.result = GetMax8(instruction_state.data.result << 1) | _GetEFLAGS_CF;
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, flagcf);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB8(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        _be;
        break;
    case 16:
        _bb("bit(16)");
        count = (csrc & 0x1f) % 17;
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = GetMax16(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            flagcf = !!GetMSB16(instruction_state.data.result);
            instruction_state.data.result = GetMax16(instruction_state.data.result << 1) | _GetEFLAGS_CF;
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, flagcf);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB16(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        _be;
        break;
    case 32:
        _bb("bit(32)");
        count = (csrc & 0x1f);
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = GetMax32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            flagcf = !!GetMSB32(instruction_state.data.result);
            instruction_state.data.result = GetMax32(instruction_state.data.result << 1) | _GetEFLAGS_CF;
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, flagcf);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB32(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
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
static void _a_rcr(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit) {
    t_nubit8 count;
    t_nubit32 flagcf;
    _cb("_a_rcr");
    switch (bit) {
    case 8:
        _bb("bit(8)");
        count = (csrc & 0x1f) % 9;
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = GetMax8(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (instruction_state.data.opr2 == 1)
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB8(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        while (count) {
            flagcf = GetLSB8(instruction_state.data.result);
            instruction_state.data.result = GetMax8(instruction_state.data.result >> 1) | (_GetEFLAGS_CF ? MSB8 : 0);
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!flagcf);
            count--;
        }
        _be;
        break;
    case 16:
        _bb("bit(16)");
        count = (csrc & 0x1f) % 17;
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = GetMax16(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (instruction_state.data.opr2 == 1)
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB16(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        while (count) {
            flagcf = GetLSB16(instruction_state.data.result);
            instruction_state.data.result = GetMax16(instruction_state.data.result >> 1) | (_GetEFLAGS_CF ? MSB16 : 0);
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!flagcf);
            count--;
        }
        _be;
        break;
    case 32:
        _bb("bit(32)");
        count = (csrc & 0x1f);
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = GetMax32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (instruction_state.data.opr2 == 1)
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB32(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        while (count) {
            flagcf = GetLSB32(instruction_state.data.result);
            instruction_state.data.result = GetMax32(instruction_state.data.result >> 1) | (_GetEFLAGS_CF ? MSB32 : 0);
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!flagcf);
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
static void _a_shl(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit) {
    t_nubit8 count;
    _cb("_a_shl");
    count = csrc & 0x1f;
    instruction_state.data.opr2 = count;
    if (count >= bit) instruction_state.data.udf |= VCPU_EFLAGS_CF;
    switch (bit) {
    case 8:
        _bb("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = GetMax8(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!GetMSB8(instruction_state.data.result));
            instruction_state.data.result = GetMax8(instruction_state.data.result << 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB8(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0) {
            _bb("opr2(!0)");
            _chr(_kaf_set_flags(context, SHL_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            _be;
        }
        _be;
        break;
    case 16:
        _bb("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = GetMax16(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            MakeBit(cpu_state.data.eflags,VCPU_EFLAGS_CF, !!GetMSB16(instruction_state.data.result));
            instruction_state.data.result = GetMax16(instruction_state.data.result << 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB16(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0) {
            _bb("opr2(!0)");
            _chr(_kaf_set_flags(context, SHL_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            _be;
        }
        _be;
        break;
    case 32:
        _bb("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = GetMax32(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            MakeBit(cpu_state.data.eflags,VCPU_EFLAGS_CF, !!GetMSB32(instruction_state.data.result));
            instruction_state.data.result = GetMax32(instruction_state.data.result << 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!GetMSB32(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0) {
            _bb("opr2(!0)");
            _chr(_kaf_set_flags(context, SHL_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
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
static void _a_shr(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit) {
    t_nubit8 count;
    _cb("_a_shr");
    count = csrc & 0x1f;
    instruction_state.data.opr2 = count;
    if (count >= bit) instruction_state.data.udf |= VCPU_EFLAGS_CF;
    switch (bit) {
    case 8:
        _bb("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = GetMax8(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!GetLSB8(instruction_state.data.result)));
            instruction_state.data.result = GetMax8(instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF, (!!GetMSB8(instruction_state.data.opr1)));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0) {
            _bb("opr2(!0)");
            _chr(_kaf_set_flags(context, SHR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            _be;
        }
        _be;
        break;
    case 16:
        _bb("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = GetMax16(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            MakeBit(cpu_state.data.eflags,VCPU_EFLAGS_CF, (!!GetLSB16(instruction_state.data.result)));
            instruction_state.data.result = GetMax16(instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF, (!!GetMSB16(instruction_state.data.opr1)));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0) {
            _bb("opr2(!0)");
            _chr(_kaf_set_flags(context, SHR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            _be;
        }
        _be;
        break;
    case 32:
        _bb("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = GetMax32(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            MakeBit(cpu_state.data.eflags,VCPU_EFLAGS_CF, (!!GetLSB32(instruction_state.data.result)));
            instruction_state.data.result = GetMax32(instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF, (!!GetMSB32(instruction_state.data.opr1)));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0) {
            _bb("opr2(!0)");
            _chr(_kaf_set_flags(context, SHR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
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
static void _a_sar(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit8 csrc, t_nubit8 bit) {
    t_nubit8 count;
    t_bool tempcf;
    _cb("_a_shr");
    count = csrc & 0x1f;
    instruction_state.data.opr2 = count;
    switch (bit) {
    case 8:
        _bb("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = GetMax8(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!GetLSB8(instruction_state.data.result)));
            tempcf = GetMSB8(instruction_state.data.result);
            instruction_state.data.result = GetMax8((t_nsbit8)instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1) _ClrEFLAGS_OF;
        else instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0) {
            _bb("opr2(!0)");
            _chr(_kaf_set_flags(context, SAR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            _be;
        }
        _be;
        break;
    case 16:
        _bb("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = GetMax16(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!GetLSB16(instruction_state.data.result)));
            instruction_state.data.result = GetMax16((t_nsbit16)instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1) _ClrEFLAGS_OF;
        else instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0) {
            _bb("opr2(!0)");
            _chr(_kaf_set_flags(context, SAR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            _be;
        }
        _be;
        break;
    case 32:
        _bb("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = GetMax32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!GetLSB32(instruction_state.data.result)));
            instruction_state.data.result = GetMax32((t_nsbit32)instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1) _ClrEFLAGS_OF;
        else instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0) {
            _bb("opr2(!0)");
            _chr(_kaf_set_flags(context, SAR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
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

static void _p_ins(core_machine_cpu_execution_context *context, t_nubit8 byte) {
    t_nubit32 cedi, data = 0;
    _cb("_p_ins");
    switch (_GetAddressSize) {
    case 2:
        cedi = cpu_state.data.di;
        break;
    case 4:
        cedi = cpu_state.data.edi;
        break;
    default:
        _impossible_r_;
        break;
    }
    switch (byte) {
    case 1:
        _bb("byte(1)");
        _chr(_m_test_access(context, &cpu_state.data.es, cedi, 1, 1));
        _chr(_p_input(context, cpu_state.data.dx, GetRef(data), 1));
        _chr(_s_write_es(context, cedi, GetRef(data), 1));
        _chr(_kas_move_index(context, 1, 0, 1));
        _be;
        break;
    case 2:
        _bb("byte(2)");
        _chr(_m_test_access(context, &cpu_state.data.es, cedi, 2, 1));
        _chr(_p_input(context, cpu_state.data.dx, GetRef(data), 2));
        _chr(_s_write_es(context, cedi, GetRef(data), 2));
        _chr(_kas_move_index(context, 2, 0, 1));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_m_test_access(context, &cpu_state.data.es, cedi, 4, 1));
        _chr(_p_input(context, cpu_state.data.dx, GetRef(data), 4));
        _chr(_s_write_es(context, cedi, GetRef(data), 4));
        _chr(_kas_move_index(context, 4, 0, 1));
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
static void _p_outs(core_machine_cpu_execution_context *context, t_nubit8 byte) {
    t_nubit32 cesi, data = 0;
    _cb("_p_outs");
    switch (_GetAddressSize) {
    case 2:
        cesi = cpu_state.data.si;
        break;
    case 4:
        cesi = cpu_state.data.esi;
        break;
    default:
        _impossible_r_;
        break;
    }
    switch (byte) {
    case 1:
        _bb("byte(1)");
        _chr(_m_read_logical(context, instruction_state.data.roverds, cesi, GetRef(data), 1));
        _chr(_p_output(context, cpu_state.data.dx, GetRef(data), 1));
        _chr(_kas_move_index(context, 1, 1, 0));
        _be;
        break;
    case 2:
        _bb("byte(2)");
        _chr(_m_read_logical(context, instruction_state.data.roverds, cesi, GetRef(data), 2));
        _chr(_p_output(context, cpu_state.data.dx, GetRef(data), 2));
        _chr(_kas_move_index(context, 2, 1, 0));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_m_read_logical(context, instruction_state.data.roverds, cesi, GetRef(data), 4));
        _chr(_p_output(context, cpu_state.data.dx, GetRef(data), 4));
        _chr(_kas_move_index(context, 4, 1, 0));
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
static void _m_movs(core_machine_cpu_execution_context *context, t_nubit8 byte) {
    t_nubit32 data = 0;
    t_nubit32 cesi, cedi;
    _cb("_m_movs");
    switch (_GetAddressSize) {
    case 2:
        cesi = cpu_state.data.si;
        cedi = cpu_state.data.di;
        break;
    case 4:
        cesi = cpu_state.data.esi;
        cedi = cpu_state.data.edi;
        break;
    default:
        _impossible_r_;
        break;
    }
    switch (byte) {
    case 1:
        _bb("byte(1)");
        _chr(_m_read_logical(context, instruction_state.data.roverds, cesi, GetRef(data), 1));
        _chr(_s_write_es(context, cedi, GetRef(data), 1));
        _chr(_kas_move_index(context, 1, 1, 1));
        _be;
        break;
    case 2:
        _bb("byte(2)");
        _chr(_m_read_logical(context, instruction_state.data.roverds, cesi, GetRef(data), 2));
        _chr(_s_write_es(context, cedi, GetRef(data), 2));
        _chr(_kas_move_index(context, 2, 1, 1));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_m_read_logical(context, instruction_state.data.roverds, cesi, GetRef(data), 4));
        _chr(_s_write_es(context, cedi, GetRef(data), 4));
        _chr(_kas_move_index(context, 4, 1, 1));
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
static void _m_stos(core_machine_cpu_execution_context *context, t_nubit8 byte) {
    t_nubit32 cedi;
    _cb("_m_stos");
    switch (_GetAddressSize) {
    case 2:
        cedi = cpu_state.data.di;
        break;
    case 4:
        cedi = cpu_state.data.edi;
        break;
    default:
        _impossible_r_;
        break;
    }
    switch (byte) {
    case 1:
        _bb("byte(1)");
        _chr(_s_write_es(context, cedi, GetRef(cpu_state.data.al), 1));
        _chr(_kas_move_index(context, 1, 0, 1));
        _be;
        break;
    case 2:
        _bb("byte(2)");
        _chr(_s_write_es(context, cedi, GetRef(cpu_state.data.ax), 2));
        _chr(_kas_move_index(context, 2, 0, 1));
        _be;
        break;
    case 4:
        _bb("byte(4)");
        _chr(_s_write_es(context, cedi, GetRef(cpu_state.data.eax), 4));
        _chr(_kas_move_index(context, 4, 0, 1));
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
static void _m_lods(core_machine_cpu_execution_context *context, t_nubit8 byte) {
    t_nubit32 cesi = 0x00000000;
    _cb("_m_lods");
    switch (_GetAddressSize) {
    case 2:
        cesi = cpu_state.data.si;
        break;
    case 4:
        cesi = cpu_state.data.esi;
        break;
    default:
        _impossible_r_;
        break;
    }
    switch (byte) {
    case 1:
        _bb("byte(1)");
        instruction_state.data.bit = 8;
        _chr(_m_read_logical(context, instruction_state.data.roverds, cesi, GetRef(cpu_state.data.al), 1));
        _chr(_kas_move_index(context, 1, 1, 0));
        instruction_state.data.result = cpu_state.data.al;
        _be;
        break;
    case 2:
        _bb("byte(2)");
        instruction_state.data.bit = 16;
        _chr(_m_read_logical(context, instruction_state.data.roverds, cesi, GetRef(cpu_state.data.ax), 2));
        _chr(_kas_move_index(context, 2, 1, 0));
        instruction_state.data.result = cpu_state.data.ax;
        _be;
        break;
    case 4:
        _bb("byte(4)");
        instruction_state.data.bit = 32;
        _chr(_m_read_logical(context, instruction_state.data.roverds, cesi, GetRef(cpu_state.data.eax), 4));
        _chr(_kas_move_index(context, 4, 1, 0));
        instruction_state.data.result = cpu_state.data.eax;
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
static void _a_cmps(core_machine_cpu_execution_context *context, t_nubit8 bit) {
    t_nubit32 cesi, cedi;
    _cb("_a_cmps");
    instruction_state.data.opr1 = 0;
    instruction_state.data.opr2 = 0;
    switch (_GetAddressSize) {
    case 2:
        cesi = cpu_state.data.si;
        cedi = cpu_state.data.di;
        break;
    case 4:
        cesi = cpu_state.data.esi;
        cedi = cpu_state.data.edi;
        break;
    default:
        _impossible_r_;
        break;
    }
    switch (bit) {
    case 8:
        _bb("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.type = CMP8;
        _chr(_m_read_logical(context, instruction_state.data.roverds, cesi, GetRef(instruction_state.data.opr1), 1));
        _chr(_s_read_es(context, cedi, GetRef(instruction_state.data.opr2), 1));
        instruction_state.data.result = GetMax8(instruction_state.data.opr1 - instruction_state.data.opr2);
        _chr(_kas_move_index(context, 1, 1, 1));
        _be;
        break;
    case 16:
        _bb("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.type = CMP16;
        _chr(_m_read_logical(context, instruction_state.data.roverds, cesi, GetRef(instruction_state.data.opr1), 2));
        _chr(_s_read_es(context, cedi, GetRef(instruction_state.data.opr2), 2));
        instruction_state.data.result = GetMax16(instruction_state.data.opr1 - instruction_state.data.opr2);
        _chr(_kas_move_index(context, 2, 1, 1));
        _be;
        break;
    case 32:
        _bb("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.type = CMP32;
        _chr(_m_read_logical(context, instruction_state.data.roverds, cesi, GetRef(instruction_state.data.opr1), 4));
        _chr(_s_read_es(context, cedi, GetRef(instruction_state.data.opr2), 4));
        instruction_state.data.result = GetMax32(instruction_state.data.opr1 - instruction_state.data.opr2);
        _chr(_kas_move_index(context, 4, 1, 1));
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    _chr(_kaf_set_flags(context, CMP_FLAG));
    _ce;
}
static void _a_scas(core_machine_cpu_execution_context *context, t_nubit8 bit) {
    t_nubit32 cedi;
    _cb("_a_scas");
    instruction_state.data.opr1 = 0;
    instruction_state.data.opr2 = 0;
    switch (_GetAddressSize) {
    case 2:
        cedi = cpu_state.data.di;
        break;
    case 4:
        cedi = cpu_state.data.edi;
        break;
    default:
        _impossible_r_;
        break;
    }
    switch (bit) {
    case 8:
        _bb("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.type = CMP8;
        _chr(instruction_state.data.opr1 = cpu_state.data.al);
        _chr(_s_read_es(context, cedi, GetRef(instruction_state.data.opr2), 1));
        instruction_state.data.result = GetMax8(instruction_state.data.opr1 - instruction_state.data.opr2);
        _chr(_kas_move_index(context, 1, 0, 1));
        _be;
        break;
    case 16:
        _bb("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.type = CMP16;
        _chr(instruction_state.data.opr1 = cpu_state.data.ax);
        _chr(_s_read_es(context, cedi, GetRef(instruction_state.data.opr2), 2));
        instruction_state.data.result = GetMax16(instruction_state.data.opr1 - instruction_state.data.opr2);
        _chr(_kas_move_index(context, 2, 0, 1));
        _be;
        break;
    case 32:
        _bb("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.type = CMP32;
        _chr(instruction_state.data.opr1 = cpu_state.data.eax);
        _chr(_s_read_es(context, cedi, GetRef(instruction_state.data.opr2), 4));
        instruction_state.data.result = GetMax32(instruction_state.data.opr1 - instruction_state.data.opr2);
        _chr(_kas_move_index(context, 4, 0, 1));
        _be;
        break;
    default:
        _bb("bit");
        _chr(_SetExcept_CE(bit));
        _be;
        break;
    }
    _chr(_kaf_set_flags(context, CMP_FLAG));
    _ce;
}
#define _adv _chr(_d_skip(context, 1))
static void UndefinedOpcode(core_machine_cpu_execution_context *context) {
    _cb("UndefinedOpcode");
    cpu_state = instruction_state.data.oldcpu;
    if (!_GetCR0_PE) {
        PRINTF("The NXVM CPU has encountered an illegal instruction at L%08X.\n", cpu_state.data.cs.base + cpu_state.data.eip);
        core_machine_cpu_execution_request_stop(context);
    }
    _chr(_SetExcept_UD(0));
    _ce;
}
static void ADD_RM8_R8(core_machine_cpu_execution_context *context) {
    _cb("ADD_RM8_R8");
    i386(0x00) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_a_add(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    _chr(_m_write_rm(context, 1));
    _ce;
}
static void ADD_RM32_R32(core_machine_cpu_execution_context *context) {
    _cb("ADD_RM32_R32");
    i386(0x01) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_a_add(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 2, 2));
        _chr(_m_read_rm(context, 2));
        _chr(_a_add(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 2));
    }
    _ce;
}
static void ADD_R8_RM8(core_machine_cpu_execution_context *context) {
    _cb("ADD_R8_RM8");
    i386(0x02) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_a_add(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), 1));
    _ce;
}
static void ADD_R32_RM32(core_machine_cpu_execution_context *context) {
    _cb("ADD_R32_RM32");
    i386(0x03) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_a_add(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 2, 2));
        _chr(_m_read_rm(context, 2));
        _chr(_a_add(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), 2));
    }
    _ce;
}
static void ADD_AL_I8(core_machine_cpu_execution_context *context) {
    _cb("ADD_AL_I8");
    i386(0x04) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_a_add(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = GetMax8(instruction_state.data.result);
    _ce;
}
static void ADD_EAX_I32(core_machine_cpu_execution_context *context) {
    _cb("ADD_EAX_I32");
    i386(0x05) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(context, 2));
            _chr(_a_add(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_d_imm(context, 4));
            _chr(_a_add(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        _chr(_a_add(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void PUSH_ES(core_machine_cpu_execution_context *context) {
    t_nubit32 xs_sel;
    _cb("PUSH_ES");
    i386(0x06) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    xs_sel = cpu_state.data.es.selector;
    _chr(_e_push(context, GetRef(xs_sel), _GetOperandSize));
    _ce;
}
static void POP_ES(core_machine_cpu_execution_context *context) {
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    t_nubit32 xs_sel;
    _cb("POP_ES");
    i386(0x07) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_e_pop(context, GetRef(xs_sel), _GetOperandSize));
    _chr(_s_load_es(context, GetMax16(xs_sel)));
    _ce;
}
static void OR_RM8_R8(core_machine_cpu_execution_context *context) {
    _cb("OR_RM8_R8");
    i386(0x08) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_a_or(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    _chr(_m_write_rm(context, 1));
    _ce;
}
static void OR_RM32_R32(core_machine_cpu_execution_context *context) {
    _cb("OR_RM32_R32");
    i386(0x09) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_a_or(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 2, 2));
        _chr(_m_read_rm(context, 2));
        _chr(_a_or(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 2));
    }
    _ce;
}
static void OR_R8_RM8(core_machine_cpu_execution_context *context) {
    _cb("OR_R8_RM8");
    i386(0x0a) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_a_or(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), 1));
    _ce;
}
static void OR_R32_RM32(core_machine_cpu_execution_context *context) {
    _cb("OR_R32_RM32");
    i386(0x0b) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_a_or(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 2, 2));
        _chr(_m_read_rm(context, 2));
        _chr(_a_or(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), 2));
    }
    _ce;
}
static void OR_AL_I8(core_machine_cpu_execution_context *context) {
    _cb("OR_AL_I8");
    i386(0x0c) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_a_or(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = GetMax8(instruction_state.data.result);
    _ce;
}
static void OR_EAX_I32(core_machine_cpu_execution_context *context) {
    _cb("OR_EAX_I32");
    i386(0x0d) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(context, 2));
            _chr(_a_or(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_d_imm(context, 4));
            _chr(_a_or(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        _chr(_a_or(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void PUSH_CS(core_machine_cpu_execution_context *context) {
    t_nubit32 xs_sel;
    _cb("PUSH_CS");
    i386(0x0e) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    xs_sel = cpu_state.data.cs.selector;
    _chr(_e_push(context, GetRef(xs_sel), _GetOperandSize));
    _ce;
}
static void POP_CS(core_machine_cpu_execution_context *context) {
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    t_nubit32 xs_sel;
    _cb("POP_CS");
    i386(0x0f) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_e_pop(context, GetRef(xs_sel), _GetOperandSize));
    _chr(_s_load_cs(context, GetMax16(xs_sel)));
    _ce;
}
static void INS_0F(core_machine_cpu_execution_context *context) {
    t_nubit8 opcode = 0x00;
    _cb("INS_0F");
    i386(0x0f) {
        _adv;
        _chr(_s_read_cs(context, cpu_state.data.eip, GetRef(opcode), 1));
        _chr(ExecCpuInstruction(instruction_state.connect.insTable_0f[opcode]));
    }
    else
        POP_CS(context);
    _ce;
}
static void ADC_RM8_R8(core_machine_cpu_execution_context *context) {
    _cb("ADC_RM8_R8");
    i386(0x10) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_a_adc(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    _chr(_m_write_rm(context, 1));
    _ce;
}
static void ADC_RM32_R32(core_machine_cpu_execution_context *context) {
    _cb("ADC_RM32_R32");
    i386(0x11) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_a_adc(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 2, 2));
        _chr(_m_read_rm(context, 2));
        _chr(_a_adc(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 2));
    }
    _ce;
}
static void ADC_R8_RM8(core_machine_cpu_execution_context *context) {
    _cb("ADC_R8_RM8");
    i386(0x12) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_a_adc(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), 1));
    _ce;
}
static void ADC_R32_RM32(core_machine_cpu_execution_context *context) {
    _cb("ADC_R32_RM32");
    i386(0x13) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_a_adc(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 2, 2));
        _chr(_m_read_rm(context, 2));
        _chr(_a_adc(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), 2));
    }
    _ce;
}
static void ADC_AL_I8(core_machine_cpu_execution_context *context) {
    _cb("ADC_AL_I8");
    i386(0x14) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_a_adc(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = GetMax8(instruction_state.data.result);
    _ce;
}
static void ADC_EAX_I32(core_machine_cpu_execution_context *context) {
    _cb("ADC_EAX_I32");
    i386(0x15) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(context, 2));
            _chr(_a_adc(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_d_imm(context, 4));
            _chr(_a_adc(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        _chr(_a_adc(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void PUSH_SS(core_machine_cpu_execution_context *context) {
    t_nubit32 xs_sel;
    _cb("PUSH_SS");
    i386(0x16) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    xs_sel = cpu_state.data.ss.selector;
    _chr(_e_push(context, GetRef(xs_sel), _GetOperandSize));
    _ce;
}
static void POP_SS(core_machine_cpu_execution_context *context) {
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    t_nubit32 xs_sel;
    _cb("POP_SS");
    i386(0x17) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_e_pop(context, GetRef(xs_sel), _GetOperandSize));
    _chr(_s_load_ss(context, GetMax16(xs_sel)));
    _ce;
}
static void SBB_RM8_R8(core_machine_cpu_execution_context *context) {
    _cb("SBB_RM8_R8");
    i386(0x18) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    _chr(_m_write_rm(context, 1));
    _ce;
}
static void SBB_RM32_R32(core_machine_cpu_execution_context *context) {
    _cb("SBB_RM32_R32");
    i386(0x19) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 2, 2));
        _chr(_m_read_rm(context, 2));
        _chr(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 2));
    }
    _ce;
}
static void SBB_R8_RM8(core_machine_cpu_execution_context *context) {
    _cb("SBB_R8_RM8");
    i386(0x1a) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_a_sbb(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), 1));
    _ce;
}
static void SBB_R32_RM32(core_machine_cpu_execution_context *context) {
    _cb("SBB_R32_RM32");
    i386(0x1b) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_a_sbb(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 2, 2));
        _chr(_m_read_rm(context, 2));
        _chr(_a_sbb(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), 2));
    }
    _ce;
}
static void SBB_AL_I8(core_machine_cpu_execution_context *context) {
    _cb("SBB_AL_I8");
    i386(0x1c) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_a_sbb(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = GetMax8(instruction_state.data.result);
    _ce;
}
static void SBB_EAX_I32(core_machine_cpu_execution_context *context) {
    _cb("SBB_EAX_I32");
    i386(0x1d) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(context, 2));
            _chr(_a_sbb(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_d_imm(context, 4));
            _chr(_a_sbb(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        _chr(_a_sbb(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void PUSH_DS(core_machine_cpu_execution_context *context) {
    t_nubit32 xs_sel;
    _cb("PUSH_DS");
    i386(0x1e) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    xs_sel = cpu_state.data.ds.selector;
    _chr(_e_push(context, GetRef(xs_sel), _GetOperandSize));
    _ce;
}
static void POP_DS(core_machine_cpu_execution_context *context) {
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    t_nubit32 xs_sel;
    _cb("POP_DS");
    i386(0x1f) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_e_pop(context, GetRef(xs_sel), _GetOperandSize));
    _chr(_s_load_ds(context, GetMax16(xs_sel)));
    _ce;
}
static void AND_RM8_R8(core_machine_cpu_execution_context *context) {
    _cb("AND_RM8_R8");
    i386(0x20) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_a_and(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    _chr(_m_write_rm(context, 1));
    _ce;
}
static void AND_RM32_R32(core_machine_cpu_execution_context *context) {
    _cb("AND_RM32_R32");
    i386(0x21) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_a_and(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 2, 2));
        _chr(_m_read_rm(context, 2));
        _chr(_a_and(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 2));
    }
    _ce;
}
static void AND_R8_RM8(core_machine_cpu_execution_context *context) {
    _cb("AND_R8_RM8");
    i386(0x22) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_a_and(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), 1));
    _ce;
}
static void AND_R32_RM32(core_machine_cpu_execution_context *context) {
    _cb("AND_R32_RM32");
    i386(0x23) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_a_and(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 2, 2));
        _chr(_m_read_rm(context, 2));
        _chr(_a_and(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), 2));
    }
    _ce;
}
static void AND_AL_I8(core_machine_cpu_execution_context *context) {
    _cb("AND_AL_I8");
    i386(0x24) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_a_and(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = GetMax8(instruction_state.data.result);
    _ce;
}
static void AND_EAX_I32(core_machine_cpu_execution_context *context) {
    _cb("AND_EAX_I32");
    i386(0x25) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(context, 2));
            _chr(_a_and(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_d_imm(context, 4));
            _chr(_a_and(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        _chr(_a_and(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void PREFIX_ES(core_machine_cpu_execution_context *context) {
    _cb("PREFIX_ES");
    i386(0x26) {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.es;
        instruction_state.data.roverss = &cpu_state.data.es;
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.roverds = &cpu_state.data.es;
        instruction_state.data.roverss = &cpu_state.data.es;
    }
    _ce;
}
static void DAA(core_machine_cpu_execution_context *context) {
    t_nubit8 oldAL = cpu_state.data.al;
    _cb("DAA");
    i386(0x27)
    _adv;
    else
        cpu_state.data.ip++;
    if (((cpu_state.data.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
        cpu_state.data.al += 0x06;
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                _GetEFLAGS_CF || ((cpu_state.data.al < oldAL) || (cpu_state.data.al < 0x06)));
        _SetEFLAGS_AF;
    } else _ClrEFLAGS_AF;
    if (((cpu_state.data.al & 0xf0) > 0x90) || _GetEFLAGS_CF) {
        cpu_state.data.al += 0x60;
        _SetEFLAGS_CF;
    } else _ClrEFLAGS_CF;
    instruction_state.data.bit = 8;
    instruction_state.data.result = (t_nubit32)cpu_state.data.al;
    _chr(_kaf_set_flags(context, DAA_FLAG));
    instruction_state.data.udf |= VCPU_EFLAGS_OF;
    _ce;
}
static void SUB_RM8_R8(core_machine_cpu_execution_context *context) {
    _cb("SUB_RM8_R8");
    i386(0x28) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_a_sub(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    _chr(_m_write_rm(context, 1));
    _ce;
}
static void SUB_RM32_R32(core_machine_cpu_execution_context *context) {
    _cb("SUB_RM32_R32");
    i386(0x29) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_a_sub(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 2, 2));
        _chr(_m_read_rm(context, 2));
        _chr(_a_sub(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 2));
    }
    _ce;
}
static void SUB_R8_RM8(core_machine_cpu_execution_context *context) {
    _cb("SUB_R8_RM8");
    i386(0x2a) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_a_sub(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), 1));
    _ce;
}
static void SUB_R32_RM32(core_machine_cpu_execution_context *context) {
    _cb("SUB_R32_RM32");
    i386(0x2b) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_a_sub(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 2, 2));
        _chr(_m_read_rm(context, 2));
        _chr(_a_sub(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), 2));
    }
    _ce;
}
static void SUB_AL_I8(core_machine_cpu_execution_context *context) {
    _cb("SUB_AL_I8");
    i386(0x2c) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_a_sub(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = GetMax8(instruction_state.data.result);
    _ce;
}
static void SUB_EAX_I32(core_machine_cpu_execution_context *context) {
    _cb("SUB_EAX_I32");
    i386(0x2d) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(context, 2));
            _chr(_a_sub(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_d_imm(context, 4));
            _chr(_a_sub(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        _chr(_a_sub(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void PREFIX_CS(core_machine_cpu_execution_context *context) {
    _cb("PREFIX_CS");
    i386(0x2e) {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.cs;
        instruction_state.data.roverss = &cpu_state.data.cs;
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.roverds = &cpu_state.data.cs;
        instruction_state.data.roverss = &cpu_state.data.cs;
    }
    _ce;
}
static void DAS(core_machine_cpu_execution_context *context) {
    t_nubit8 oldAL = cpu_state.data.al;
    _cb("DAS");
    i386(0x2f)
    _adv;
    else
        cpu_state.data.ip++;
    if (((cpu_state.data.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
        cpu_state.data.al -= 0x06;
        MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                _GetEFLAGS_CF || (oldAL < 0x06));
        _SetEFLAGS_AF;
    } else _ClrEFLAGS_AF;
    if ((cpu_state.data.al > 0x9f) || _GetEFLAGS_CF) {
        cpu_state.data.al -= 0x60;
        _SetEFLAGS_CF;
    } else _ClrEFLAGS_CF;
    instruction_state.data.bit = 8;
    instruction_state.data.result = (t_nubit32)cpu_state.data.al;
    _chr(_kaf_set_flags(context, DAS_FLAG));
    instruction_state.data.udf |= VCPU_EFLAGS_OF;
    _ce;
}
static void XOR_RM8_R8(core_machine_cpu_execution_context *context) {
    _cb("XOR_RM8_R8");
    i386(0x30) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_a_xor(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    _chr(_m_write_rm(context, 1));
    _ce;
}
static void XOR_RM32_R32(core_machine_cpu_execution_context *context) {
    _cb("XOR_RM32_R32");
    i386(0x31) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_a_xor(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 2, 2));
        _chr(_m_read_rm(context, 2));
        _chr(_a_xor(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 2));
    }
    _ce;
}
static void XOR_R8_RM8(core_machine_cpu_execution_context *context) {
    _cb("XOR_R8_RM8");
    i386(0x32) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_a_xor(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), 1));
    _ce;
}
static void XOR_R32_RM32(core_machine_cpu_execution_context *context) {
    _cb("XOR_R32_RM32");
    i386(0x33) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_a_xor(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 2, 2));
        _chr(_m_read_rm(context, 2));
        _chr(_a_xor(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), 2));
    }
    _ce;
}
static void XOR_AL_I8(core_machine_cpu_execution_context *context) {
    _cb("XOR_AL_I8");
    i386(0x34) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_a_xor(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = GetMax8(instruction_state.data.result);
    _ce;
}
static void XOR_EAX_I32(core_machine_cpu_execution_context *context) {
    _cb("XOR_EAX_I32");
    i386(0x35) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperxorSize(2)");
            _chr(_d_imm(context, 2));
            _chr(_a_xor(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperxorSize(4)");
            _chr(_d_imm(context, 4));
            _chr(_a_xor(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        _chr(_a_xor(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void PREFIX_SS(core_machine_cpu_execution_context *context) {
    _cb("PREFIX_SS");
    i386(0x36) {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.ss;
        instruction_state.data.roverss = &cpu_state.data.ss;
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.roverds = &cpu_state.data.ss;
        instruction_state.data.roverss = &cpu_state.data.ss;
    }
    _ce;
}
static void AAA(core_machine_cpu_execution_context *context) {
    _cb("AAA");
    i386(0x37) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    if (((cpu_state.data.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
        cpu_state.data.al += 0x06;
        cpu_state.data.ah += 0x01;
        _SetEFLAGS_AF;
        _SetEFLAGS_CF;
    } else {
        _ClrEFLAGS_AF;
        _ClrEFLAGS_CF;
    }
    cpu_state.data.al &= 0x0f;
    instruction_state.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF);
    _ce;
}
static void CMP_RM8_R8(core_machine_cpu_execution_context *context) {
    _cb("CMP_RM8_R8");
    i386(0x38) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    _ce;
}
static void CMP_RM32_R32(core_machine_cpu_execution_context *context) {
    _cb("CMP_RM32_R32");
    i386(0x39) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 2, 2));
        _chr(_m_read_rm(context, 2));
        _chr(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cr, 16));
    }
    _ce;
}
static void CMP_R8_RM8(core_machine_cpu_execution_context *context) {
    _cb("CMP_R8_RM8");
    i386(0x3a) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_a_cmp(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    _ce;
}
static void CMP_R32_RM32(core_machine_cpu_execution_context *context) {
    _cb("CMP_R32_RM32");
    i386(0x3b) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_a_cmp(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 2, 2));
        _chr(_m_read_rm(context, 2));
        _chr(_a_cmp(context, instruction_state.data.cr, instruction_state.data.crm, 16));
    }
    _ce;
}
static void CMP_AL_I8(core_machine_cpu_execution_context *context) {
    _cb("CMP_AL_I8");
    i386(0x3c) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_a_cmp(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    _ce;
}
static void CMP_EAX_I32(core_machine_cpu_execution_context *context) {
    _cb("CMP_EAX_I32");
    i386(0x3d) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(context, 2));
            _chr(_a_cmp(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_d_imm(context, 4));
            _chr(_a_cmp(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        _chr(_a_cmp(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
    }
    _ce;
}
static void PREFIX_DS(core_machine_cpu_execution_context *context) {
    _cb("PREFIX_DS");
    i386(0x3e) {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.ds;
        instruction_state.data.roverss = &cpu_state.data.ds;
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.roverds = &cpu_state.data.ds;
        instruction_state.data.roverss = &cpu_state.data.ds;
    }
    _ce;
}
static void AAS(core_machine_cpu_execution_context *context) {
    _cb("AAS");
    i386(0x3f) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    if (((cpu_state.data.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
        cpu_state.data.al -= 0x06;
        cpu_state.data.ah -= 0x01;
        _SetEFLAGS_AF;
        _SetEFLAGS_CF;
    } else {
        _ClrEFLAGS_CF;
        _ClrEFLAGS_AF;
    }
    cpu_state.data.al &= 0x0f;
    instruction_state.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF);
    _ce;
}
static void INC_EAX(core_machine_cpu_execution_context *context) {
    _cb("INC_EAX");
    i386(0x40) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_inc(context, cpu_state.data.ax, 16));
            cpu_state.data.ax = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_inc(context, cpu_state.data.eax, 32));
            cpu_state.data.eax = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_a_inc(context, cpu_state.data.ax, 16));
        cpu_state.data.ax = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void INC_ECX(core_machine_cpu_execution_context *context) {
    _cb("INC_ECX");
    i386(0x41) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_inc(context, cpu_state.data.cx, 16));
            cpu_state.data.cx = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_inc(context, cpu_state.data.ecx, 32));
            cpu_state.data.ecx = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_a_inc(context, cpu_state.data.cx, 16));
        cpu_state.data.cx = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void INC_EDX(core_machine_cpu_execution_context *context) {
    _cb("INC_EDX");
    i386(0x42) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_inc(context, cpu_state.data.dx, 16));
            cpu_state.data.dx = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_inc(context, cpu_state.data.edx, 32));
            cpu_state.data.edx = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_a_inc(context, cpu_state.data.dx, 16));
        cpu_state.data.dx = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void INC_EBX(core_machine_cpu_execution_context *context) {
    _cb("INC_EBX");
    i386(0x43) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_inc(context, cpu_state.data.bx, 16));
            cpu_state.data.bx = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_inc(context, cpu_state.data.ebx, 32));
            cpu_state.data.ebx = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_a_inc(context, cpu_state.data.bx, 16));
        cpu_state.data.bx = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void INC_ESP(core_machine_cpu_execution_context *context) {
    _cb("INC_ESP");
    i386(0x44) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_inc(context, cpu_state.data.sp, 16));
            cpu_state.data.sp = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_inc(context, cpu_state.data.esp, 32));
            cpu_state.data.esp = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_a_inc(context, cpu_state.data.sp, 16));
        cpu_state.data.sp = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void INC_EBP(core_machine_cpu_execution_context *context) {
    _cb("INC_EBP");
    i386(0x45) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_inc(context, cpu_state.data.bp, 16));
            cpu_state.data.bp = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_inc(context, cpu_state.data.ebp, 32));
            cpu_state.data.ebp = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_a_inc(context, cpu_state.data.bp, 16));
        cpu_state.data.bp = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void INC_ESI(core_machine_cpu_execution_context *context) {
    _cb("INC_ESI");
    i386(0x46) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_inc(context, cpu_state.data.si, 16));
            cpu_state.data.si = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_inc(context, cpu_state.data.esi, 32));
            cpu_state.data.esi = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_a_inc(context, cpu_state.data.si, 16));
        cpu_state.data.si = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void INC_EDI(core_machine_cpu_execution_context *context) {
    _cb("INC_EDI");
    i386(0x47) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_inc(context, cpu_state.data.di, 16));
            cpu_state.data.di = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_inc(context, cpu_state.data.edi, 32));
            cpu_state.data.edi = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_a_inc(context, cpu_state.data.di, 16));
        cpu_state.data.di = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void DEC_EAX(core_machine_cpu_execution_context *context) {
    _cb("DEC_EAX");
    i386(0x48) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_dec(context, cpu_state.data.ax, 16));
            cpu_state.data.ax = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_dec(context, cpu_state.data.eax, 32));
            cpu_state.data.eax = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_a_dec(context, cpu_state.data.ax, 16));
        cpu_state.data.ax = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void DEC_ECX(core_machine_cpu_execution_context *context) {
    _cb("DEC_ECX");
    i386(0x49) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_dec(context, cpu_state.data.cx, 16));
            cpu_state.data.cx = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_dec(context, cpu_state.data.ecx, 32));
            cpu_state.data.ecx = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_a_dec(context, cpu_state.data.cx, 16));
        cpu_state.data.cx = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void DEC_EDX(core_machine_cpu_execution_context *context) {
    _cb("DEC_EDX");
    i386(0x4a) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_dec(context, cpu_state.data.dx, 16));
            cpu_state.data.dx = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_dec(context, cpu_state.data.edx, 32));
            cpu_state.data.edx = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_a_dec(context, cpu_state.data.dx, 16));
        cpu_state.data.dx = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void DEC_EBX(core_machine_cpu_execution_context *context) {
    _cb("DEC_EBX");
    i386(0x4b) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_dec(context, cpu_state.data.bx, 16));
            cpu_state.data.bx = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_dec(context, cpu_state.data.ebx, 32));
            cpu_state.data.ebx = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_a_dec(context, cpu_state.data.bx, 16));
        cpu_state.data.bx = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void DEC_ESP(core_machine_cpu_execution_context *context) {
    _cb("DEC_ESP");
    i386(0x4c) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_dec(context, cpu_state.data.sp, 16));
            cpu_state.data.sp = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_dec(context, cpu_state.data.esp, 32));
            cpu_state.data.esp = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_a_dec(context, cpu_state.data.sp, 16));
        cpu_state.data.sp = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void DEC_EBP(core_machine_cpu_execution_context *context) {
    _cb("DEC_EBP");
    i386(0x4d) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_dec(context, cpu_state.data.bp, 16));
            cpu_state.data.bp = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_dec(context, cpu_state.data.ebp, 32));
            cpu_state.data.ebp = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_a_dec(context, cpu_state.data.bp, 16));
        cpu_state.data.bp = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void DEC_ESI(core_machine_cpu_execution_context *context) {
    _cb("DEC_ESI");
    i386(0x4e) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_dec(context, cpu_state.data.si, 16));
            cpu_state.data.si = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_dec(context, cpu_state.data.esi, 32));
            cpu_state.data.esi = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_a_dec(context, cpu_state.data.si, 16));
        cpu_state.data.si = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void DEC_EDI(core_machine_cpu_execution_context *context) {
    _cb("DEC_EDI");
    i386(0x4f) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_a_dec(context, cpu_state.data.di, 16));
            cpu_state.data.di = GetMax16(instruction_state.data.result);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_a_dec(context, cpu_state.data.edi, 32));
            cpu_state.data.edi = GetMax32(instruction_state.data.result);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_a_dec(context, cpu_state.data.di, 16));
        cpu_state.data.di = GetMax16(instruction_state.data.result);
    }
    _ce;
}
static void PUSH_EAX(core_machine_cpu_execution_context *context) {
    _cb("PUSH_EAX");
    i386(0x50) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_push(context, GetRef(cpu_state.data.ax), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_push(context, GetRef(cpu_state.data.eax), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_push(context, GetRef(cpu_state.data.ax), 2));
    }
    _ce;
}
static void PUSH_ECX(core_machine_cpu_execution_context *context) {
    _cb("PUSH_ECX");
    i386(0x51) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_push(context, GetRef(cpu_state.data.cx), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_push(context, GetRef(cpu_state.data.ecx), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_push(context, GetRef(cpu_state.data.cx), 2));
    }
    _ce;
}
static void PUSH_EDX(core_machine_cpu_execution_context *context) {
    _cb("PUSH_EDX");
    i386(0x52) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_push(context, GetRef(cpu_state.data.dx), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_push(context, GetRef(cpu_state.data.edx), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_push(context, GetRef(cpu_state.data.dx), 2));
    }
    _ce;
}
static void PUSH_EBX(core_machine_cpu_execution_context *context) {
    _cb("PUSH_EBX");
    i386(0x53) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_push(context, GetRef(cpu_state.data.bx), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_push(context, GetRef(cpu_state.data.ebx), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_push(context, GetRef(cpu_state.data.bx), 2));
    }
    _ce;
}
static void PUSH_ESP(core_machine_cpu_execution_context *context) {
    _cb("PUSH_ESP");
    i386(0x54) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_push(context, GetRef(cpu_state.data.sp), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_push(context, GetRef(cpu_state.data.esp), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_push(context, GetRef(cpu_state.data.sp), 2));
    }
    _ce;
}
static void PUSH_EBP(core_machine_cpu_execution_context *context) {
    _cb("PUSH_EBP");
    i386(0x55) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_push(context, GetRef(cpu_state.data.bp), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_push(context, GetRef(cpu_state.data.ebp), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_push(context, GetRef(cpu_state.data.bp), 2));
    }
    _ce;
}
static void PUSH_ESI(core_machine_cpu_execution_context *context) {
    _cb("PUSH_ESI");
    i386(0x56) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_push(context, GetRef(cpu_state.data.si), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_push(context, GetRef(cpu_state.data.esi), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_push(context, GetRef(cpu_state.data.si), 2));
    }
    _ce;
}
static void PUSH_EDI(core_machine_cpu_execution_context *context) {
    _cb("PUSH_EDI");
    i386(0x57) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_push(context, GetRef(cpu_state.data.di), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_push(context, GetRef(cpu_state.data.edi), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_push(context, GetRef(cpu_state.data.di), 2));
    }
    _ce;
}
static void POP_EAX(core_machine_cpu_execution_context *context) {
    _cb("POP_EAX");
    i386(0x58) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(context, GetRef(cpu_state.data.ax), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(context, GetRef(cpu_state.data.eax), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_pop(context, GetRef(cpu_state.data.ax), 2));
    }
    _ce;
}
static void POP_ECX(core_machine_cpu_execution_context *context) {
    _cb("POP_ECX");
    i386(0x59) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(context, GetRef(cpu_state.data.cx), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(context, GetRef(cpu_state.data.ecx), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_pop(context, GetRef(cpu_state.data.cx), 2));
    }
    _ce;
}
static void POP_EDX(core_machine_cpu_execution_context *context) {
    _cb("POP_EDX");
    i386(0x5a) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(context, GetRef(cpu_state.data.dx), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(context, GetRef(cpu_state.data.edx), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_pop(context, GetRef(cpu_state.data.dx), 2));
    }
    _ce;
}
static void POP_EBX(core_machine_cpu_execution_context *context) {
    _cb("POP_EBX");
    i386(0x5b) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(context, GetRef(cpu_state.data.bx), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(context, GetRef(cpu_state.data.ebx), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_pop(context, GetRef(cpu_state.data.bx), 2));
    }
    _ce;
}
static void POP_ESP(core_machine_cpu_execution_context *context) {
    _cb("POP_ESP");
    i386(0x5c) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(context, GetRef(cpu_state.data.sp), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(context, GetRef(cpu_state.data.esp), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_pop(context, GetRef(cpu_state.data.sp), 2));
    }
    _ce;
}
static void POP_EBP(core_machine_cpu_execution_context *context) {
    _cb("POP_EBP");
    i386(0x5d) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(context, GetRef(cpu_state.data.bp), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(context, GetRef(cpu_state.data.ebp), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_pop(context, GetRef(cpu_state.data.bp), 2));
    }
    _ce;
}
static void POP_ESI(core_machine_cpu_execution_context *context) {
    _cb("POP_ESI");
    i386(0x5e) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(context, GetRef(cpu_state.data.si), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(context, GetRef(cpu_state.data.esi), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_pop(context, GetRef(cpu_state.data.si), 2));
    }
    _ce;
}
static void POP_EDI(core_machine_cpu_execution_context *context) {
    _cb("POP_EDI");
    i386(0x5f) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(context, GetRef(cpu_state.data.di), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(context, GetRef(cpu_state.data.edi), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_pop(context, GetRef(cpu_state.data.di), 2));
    }
    _ce;
}
static void PUSHA(core_machine_cpu_execution_context *context) {
    t_nubit32 cesp;
    _cb("PUSHA");
    i386(0x60) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            cesp = cpu_state.data.sp;
            _chr(_e_push(context, GetRef(cpu_state.data.ax), 2));
            _chr(_e_push(context, GetRef(cpu_state.data.cx), 2));
            _chr(_e_push(context, GetRef(cpu_state.data.dx), 2));
            _chr(_e_push(context, GetRef(cpu_state.data.bx), 2));
            _chr(_e_push(context, GetRef(cesp),    2));
            _chr(_e_push(context, GetRef(cpu_state.data.bp), 2));
            _chr(_e_push(context, GetRef(cpu_state.data.si), 2));
            _chr(_e_push(context, GetRef(cpu_state.data.di), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            cesp = cpu_state.data.esp;
            _chr(_e_push(context, GetRef(cpu_state.data.eax), 4));
            _chr(_e_push(context, GetRef(cpu_state.data.ecx), 4));
            _chr(_e_push(context, GetRef(cpu_state.data.edx), 4));
            _chr(_e_push(context, GetRef(cpu_state.data.ebx), 4));
            _chr(_e_push(context, GetRef(cesp),     4));
            _chr(_e_push(context, GetRef(cpu_state.data.ebp), 4));
            _chr(_e_push(context, GetRef(cpu_state.data.esi), 4));
            _chr(_e_push(context, GetRef(cpu_state.data.edi), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    _ce;
}
static void POPA(core_machine_cpu_execution_context *context) {
    t_nubit32 cesp;
    _cb("POPA");
    i386(0x61) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(context, GetRef(cpu_state.data.di), 2));
            _chr(_e_pop(context, GetRef(cpu_state.data.si), 2));
            _chr(_e_pop(context, GetRef(cpu_state.data.bp), 2));
            _chr(_e_pop(context, GetRef(cesp),    2));
            _chr(_e_pop(context, GetRef(cpu_state.data.bx), 2));
            _chr(_e_pop(context, GetRef(cpu_state.data.dx), 2));
            _chr(_e_pop(context, GetRef(cpu_state.data.cx), 2));
            _chr(_e_pop(context, GetRef(cpu_state.data.ax), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(context, GetRef(cpu_state.data.edi), 4));
            _chr(_e_pop(context, GetRef(cpu_state.data.esi), 4));
            _chr(_e_pop(context, GetRef(cpu_state.data.ebp), 4));
            _chr(_e_pop(context, GetRef(cesp),     4));
            _chr(_e_pop(context, GetRef(cpu_state.data.ebx), 4));
            _chr(_e_pop(context, GetRef(cpu_state.data.edx), 4));
            _chr(_e_pop(context, GetRef(cpu_state.data.ecx), 4));
            _chr(_e_pop(context, GetRef(cpu_state.data.eax), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    _ce;
}
static void BOUND_R16_M16_16(core_machine_cpu_execution_context *context) {
    t_nsbit16 a16,l16,u16;
    t_nsbit32 a32,l32,u32;
    _cb("BOUND_R16_M16_16");
    i386(0x62) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize * 2));
        if (!instruction_state.data.flagMem) {
            _bb("flagMem(0)");
            _chr(UndefinedOpcode(context));
            _be;
        }
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            a16 = (t_nsbit16)instruction_state.data.cr;
            _chr(_m_read_rm(context, 2));
            l16 = (t_nsbit16)instruction_state.data.crm;
            instruction_state.data.mrm.offset += 2;
            _chr(_m_read_rm(context, 2));
            u16 = (t_nsbit16)instruction_state.data.crm;
            if (a16 < l16 || a16 > u16)
                _chr(_SetExcept_BR(0));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            a32 = (t_nsbit32)instruction_state.data.cr;
            _chr(_m_read_rm(context, 4));
            l32 = (t_nsbit32)instruction_state.data.crm;
            instruction_state.data.mrm.offset += 2;
            _chr(_m_read_rm(context, 4));
            u32 = (t_nsbit32)instruction_state.data.crm;
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
        UndefinedOpcode(context);
    _ce;
}
static void ARPL_RM16_R16(core_machine_cpu_execution_context *context) {
    _cb("ARPL_RM16_R16");
    i386(0x63) {
        if (_IsProtected) {
            _adv;
            _chr(_d_modrm(context, 2, 2));
            _chr(_m_read_rm(context, 2));
            instruction_state.data.cr = GetMax16(instruction_state.data.cr);
            if (_GetSelector_RPL(instruction_state.data.crm) < _GetSelector_RPL(instruction_state.data.cr)) {
                _SetEFLAGS_ZF;
                instruction_state.data.result = GetMax16((instruction_state.data.crm & ~VCPU_SELECTOR_RPL) |
                                               (instruction_state.data.cr & VCPU_SELECTOR_RPL));
                instruction_state.data.crm = instruction_state.data.result;
                _chr(_m_write_rm(context, 2));
            } else
                _ClrEFLAGS_ZF;
        } else
            _chr(UndefinedOpcode(context));
    }
    else
        UndefinedOpcode(context);
    _ce;
}
static void PREFIX_FS(core_machine_cpu_execution_context *context) {
    _cb("PREFIX_FS");
    i386(0x64) {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.fs;
        instruction_state.data.roverss = &cpu_state.data.fs;
    }
    else
        UndefinedOpcode(context);
    _ce;
}
static void PREFIX_GS(core_machine_cpu_execution_context *context) {
    _cb("PREFIX_GS");
    i386(0x65) {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.gs;
        instruction_state.data.roverss = &cpu_state.data.gs;
    }
    else
        UndefinedOpcode(context);
    _ce;
}
static void PREFIX_OprSize(core_machine_cpu_execution_context *context) {
    _cb("PREFIX_OprSize");
    i386(0x66) {
        _adv;
        instruction_state.data.prefix_oprsize = True;
    }
    else
        UndefinedOpcode(context);
    _ce;
}
static void PREFIX_AddrSize(core_machine_cpu_execution_context *context) {
    _cb("PREFIX_AddrSize");
    i386(0x67) {
        _adv;
        instruction_state.data.prefix_addrsize = True;
    }
    else
        UndefinedOpcode(context);
    _ce;
}
static void PUSH_I32(core_machine_cpu_execution_context *context) {
    _cb("PUSH_I32");
    i386(0x68) {
        _adv;
        _chr(_d_imm(context, _GetOperandSize));
        _chr(_e_push(context, GetRef(instruction_state.data.cimm), _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    _ce;
}
static void IMUL_R32_RM32_I32(core_machine_cpu_execution_context *context) {
    _cb("IMUL_R32_RM32_I32");
    i386(0x69) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_d_imm(context, _GetOperandSize));
        _chr(_a_imul3(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    _ce;
}
static void PUSH_I8(core_machine_cpu_execution_context *context) {
    _cb("PUSH_I8");
    i386(0x6a) {
        _adv;
        _chr(_d_imm(context, 1));
        _chr(_e_push(context, GetRef(instruction_state.data.cimm), _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    _ce;
}
static void IMUL_R32_RM32_I8(core_machine_cpu_execution_context *context) {
    _cb("IMUL_R32_RM32_I8");
    i386(0x6b) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_d_imm(context, 1));
        _chr(_a_imul3(context, instruction_state.data.crm, instruction_state.data.cimm, ((_GetOperandSize * 8 + 8) >> 1)));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    _ce;
}
static void INSB(core_machine_cpu_execution_context *context) {
    _cb("INSB");
    i386(0x6c) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_p_ins(context, 1));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (cpu_state.data.cx) {
                    _bb("cx(!0)");
                    _chr(_p_ins(context, 1));
                    cpu_state.data.cx--;
                    _be;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_p_ins(context, 1));
                    cpu_state.data.ecx--;
                    _be;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = True;
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
        UndefinedOpcode(context);
    _ce;
}
static void INSW(core_machine_cpu_execution_context *context) {
    _cb("INSW");
    i386(0x6d) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_p_ins(context, _GetOperandSize));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (cpu_state.data.cx) {
                    _bb("cx(!0)");
                    _chr(_p_ins(context, _GetOperandSize));
                    cpu_state.data.cx--;
                    _be;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_p_ins(context, _GetOperandSize));
                    cpu_state.data.ecx--;
                    _be;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = True;
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
        UndefinedOpcode(context);
    _ce;
}
static void OUTSB(core_machine_cpu_execution_context *context) {
    _cb("OUTSB");
    i386(0x6e) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_p_ins(context, 1));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (cpu_state.data.cx) {
                    _bb("cx(!0)");
                    _chr(_p_ins(context, 1));
                    cpu_state.data.cx--;
                    _be;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_p_ins(context, 1));
                    cpu_state.data.ecx--;
                    _be;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = True;
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
        UndefinedOpcode(context);
    _ce;
}
static void OUTSW(core_machine_cpu_execution_context *context) {
    _cb("OUTSW");
    i386(0x6f) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_p_outs(context, _GetOperandSize));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (cpu_state.data.cx) {
                    _bb("cx(!0)");
                    _chr(_p_outs(context, _GetOperandSize));
                    cpu_state.data.cx--;
                    _be;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_p_outs(context, _GetOperandSize));
                    cpu_state.data.ecx--;
                    _be;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = True;
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
        UndefinedOpcode(context);
    _ce;
}
static void JO_REL8(core_machine_cpu_execution_context *context) {
    _cb("JO_REL8");
    i386(0x70) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), 1, _GetEFLAGS_OF));
    _ce;
}
static void JNO_REL8(core_machine_cpu_execution_context *context) {
    _cb("JNO_REL8");
    i386(0x71) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), 1, !_GetEFLAGS_OF));
    _ce;
}
static void JC_REL8(core_machine_cpu_execution_context *context) {
    _cb("JC_REL8");
    i386(0x72) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), 1, _GetEFLAGS_CF));
    _ce;
}
static void JNC_REL8(core_machine_cpu_execution_context *context) {
    _cb("JNC_REL8");
    i386(0x73) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), 1, !_GetEFLAGS_CF));
    _ce;
}
static void JZ_REL8(core_machine_cpu_execution_context *context) {
    _cb("JZ_REL8");
    i386(0x74) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), 1, _GetEFLAGS_ZF));
    _ce;
}
static void JNZ_REL8(core_machine_cpu_execution_context *context) {
    _cb("JNZ_REL8");
    i386(0x75) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), 1, !_GetEFLAGS_ZF));
    _ce;
}
static void JNA_REL8(core_machine_cpu_execution_context *context) {
    _cb("JNA_REL8");
    i386(0x76) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), 1,
                (_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    _ce;
}
static void JA_REL8(core_machine_cpu_execution_context *context) {
    _cb("JA_REL8");
    i386(0x77) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), 1,
                !(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    _ce;
}
static void JS_REL8(core_machine_cpu_execution_context *context) {
    _cb("JS_REL8");
    i386(0x78) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), 1, _GetEFLAGS_SF));
    _ce;
}
static void JNS_REL8(core_machine_cpu_execution_context *context) {
    _cb("JNS_REL8");
    i386(0x79) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), 1, !_GetEFLAGS_SF));
    _ce;
}
static void JP_REL8(core_machine_cpu_execution_context *context) {
    _cb("JP_REL8");
    _new_code_path_;
    i386(0x7a) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), 1, _GetEFLAGS_PF));
    _ce;
}
static void JNP_REL8(core_machine_cpu_execution_context *context) {
    _cb("JNP_REL8");
    _new_code_path_;
    i386(0x7b) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), 1, !_GetEFLAGS_PF));
    _ce;
}
static void JL_REL8(core_machine_cpu_execution_context *context) {
    _cb("JL_REL8");
    i386(0x7c) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), 1, (_GetEFLAGS_SF != _GetEFLAGS_OF)));
    _ce;
}
static void JNL_REL8(core_machine_cpu_execution_context *context) {
    _cb("JNL_REL8");
    i386(0x7d) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), 1, (_GetEFLAGS_SF == _GetEFLAGS_OF)));
    _ce;
}
static void JNG_REL8(core_machine_cpu_execution_context *context) {
    _cb("JNG_REL8");
    i386(0x7e) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), 1,
                (_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF))));
    _ce;
}
static void JG_REL8(core_machine_cpu_execution_context *context) {
    _cb("JG_REL8");
    i386(0x7f) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), 1,
                (!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF))));
    _ce;
}
static void INS_80(core_machine_cpu_execution_context *context) {
    _cb("INS_80");
    i386 (0x80) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 0, 1));
    _chr(_d_imm(context, 1));
    _chr(_m_read_rm(context, 1));
    switch (instruction_state.data.cr) {
    case 0: /* ADD_RM8_I8 */
        _bb("ADD_RM8_I8");
        _chr(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 1));
        _be;
        break;
    case 1: /* OR_RM8_I8 */
        _bb("OR_RM8_I8");
        _chr(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 1));
        _be;
        break;
    case 2: /* ADC_RM8_I8 */
        _bb("ADC_RM8_I8");
        _chr(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 1));
        _be;
        break;
    case 3: /* SBB_RM8_I8 */
        _bb("SBB_RM8_I8");
        _new_code_path_;
        _chr(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 1));
        _be;
        break;
    case 4: /* AND_RM8_I8 */
        _bb("AND_RM8_I8");
        _chr(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 1));
        _be;
        break;
    case 5: /* SUB_RM8_I8 */
        _bb("SUB_RM8_I8");
        _chr(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 1));
        _be;
        break;
    case 6: /* XOR_RM8_I8 */
        _bb("XOR_RM8_I8");
        _chr(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 1));
        _be;
        break;
    case 7: /* CMP_RM8_I8 */
        _bb("CMP_RM8_I8");
        _chr(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void INS_81(core_machine_cpu_execution_context *context) {
    _cb("INS_81");
    i386(0x81) {
        _adv;
        _chr(_d_modrm(context, 0, _GetOperandSize));
        _chr(_d_imm(context, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        switch (instruction_state.data.cr) {
        case 0: /* ADD_RM32_I32 */
            _bb("ADD_RM32_I32");
            _chr(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 1: /* OR_RM32_I32 */
            _bb("OR_RM32_I32");
            _chr(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 2: /* ADC_RM32_I32 */
            _bb("ADC_RM32_I32");
            _chr(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 3: /* SBB_RM32_I32 */
            _bb("SBB_RM32_I32");
            _new_code_path_;
            _chr(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 4: /* AND_RM32_I32 */
            _bb("AND_RM32_I32");
            _chr(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 5: /* SUB_RM32_I32 */
            _bb("SUB_RM32_I32");
            _chr(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 6: /* XOR_RM32_I32 */
            _bb("XOR_RM32_I32");
            _chr(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 7: /* CMP_RM32_I32 */
            _bb("CMP_RM32_I32");
            _chr(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 0, 2));
        _chr(_d_imm(context, 2));
        _chr(_m_read_rm(context, 2));
        switch (instruction_state.data.cr) {
        case 0: /* ADD_RM16_I16 */
            _bb("ADD_RM16_I16");
            _chr(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 1: /* OR_RM16_I16 */
            _bb("OR_RM16_I16");
            _chr(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 2: /* ADC_RM16_I16 */
            _bb("ADC_RM16_I16");
            _chr(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 3: /* SBB_RM16_I16 */
            _bb("SBB_RM16_I16");
            _new_code_path_;
            _chr(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 4: /* AND_RM16_I16 */
            _bb("AND_RM16_I16");
            _chr(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 5: /* SUB_RM16_I16 */
            _bb("SUB_RM16_I16");
            _chr(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 6: /* XOR_RM16_I16 */
            _bb("XOR_RM16_I16");
            _chr(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 7: /* CMP_RM16_I16 */
            _bb("CMP_RM16_I16");
            _chr(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    _ce;
}
static void INS_83(core_machine_cpu_execution_context *context) {
    t_nubit8 bit;
    _cb("INS_83");
    i386(0x83) {
        _adv;
        _chr(_d_modrm(context, 0, _GetOperandSize));
        _chr(_d_imm(context, 1));
        _chr(_m_read_rm(context, _GetOperandSize));
        bit = (_GetOperandSize * 8 + 8) >> 1;
        switch (instruction_state.data.cr) {
        case 0: /* ADD_RM32_I8 */
            _bb("ADD_RM32_I8");
            _chr(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 1: /* OR_RM32_I8 */
            _bb("OR_RM32_I8");
            _chr(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 2: /* ADC_RM32_I8 */
            _bb("ADC_RM32_I8");
            _chr(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 3: /* SBB_RM32_I8 */
            _bb("SBB_RM32_I8");
            _chr(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 4: /* AND_RM32_I8 */
            _bb("AND_RM32_I8");
            _chr(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 5: /* SUB_RM32_I8 */
            _bb("SUB_RM32_I8");
            _chr(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 6: /* XOR_RM32_I8 */
            _bb("XOR_RM32_I8");
            _chr(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 7: /* CMP_RM32_I8 */
            _bb("CMP_RM32_I8");
            _chr(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 0, 2));
        _chr(_d_imm(context, 1));
        _chr(_m_read_rm(context, 2));
        bit = 12;
        switch (instruction_state.data.cr) {
        case 0: /* ADD_RM16_I8 */
            _bb("ADD_RM16_I8");
            _chr(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 1: /* OR_RM16_I8 */
            _bb("OR_RM16_I8");
            _chr(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 2: /* ADC_RM16_I8 */
            _bb("ADC_RM16_I8");
            _chr(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 3: /* SBB_RM16_I8 */
            _bb("SBB_RM16_I8");
            _chr(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 4: /* AND_RM16_I8 */
            _bb("AND_RM16_I8");
            _chr(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 5: /* SUB_RM16_I8 */
            _bb("SUB_RM16_I8");
            _chr(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 6: /* XOR_RM16_I8 */
            _bb("XOR_RM16_I8");
            _chr(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 7: /* CMP_RM16_I8 */
            _bb("CMP_RM16_I8");
            _chr(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    _ce;
}
static void TEST_RM8_R8(core_machine_cpu_execution_context *context) {
    _cb("TEST_RM8_R8");
    i386(0x84) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_a_test(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    _ce;
}
static void TEST_RM32_R32(core_machine_cpu_execution_context *context) {
    _cb("TEST_RM32_R32");
    i386(0x85) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_a_test(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
    }
    else {
        cpu_state.data.ip++;
        _d_modrm(context, 2, 2);
        _chr(_m_read_rm(context, 2));
        _a_test(context, instruction_state.data.crm, instruction_state.data.cr, 16);
    }
    _ce;
}
static void XCHG_RM8_R8(core_machine_cpu_execution_context *context) {
    _cb("XCHG_RM8_R8");
    i386(0x86) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.crm), 1));
    instruction_state.data.crm = instruction_state.data.cr;
    _chr(_m_write_rm(context, 1));
    _ce;
}
static void XCHG_RM32_R32(core_machine_cpu_execution_context *context) {
    _cb("XCHG_RM32_R32");
    i386(0x87) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.crm), _GetOperandSize));
        instruction_state.data.crm = instruction_state.data.cr;
        _chr(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _d_modrm(context, 2, 2);
        _chr(_m_read_rm(context, 2));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.crm), 2));
        instruction_state.data.crm = instruction_state.data.cr;
        _chr(_m_write_rm(context, 2));
    }
    _ce;
}
static void MOV_RM8_R8(core_machine_cpu_execution_context *context) {
    _cb("MOV_RM8_R8");
    i386(0x88) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    instruction_state.data.crm = instruction_state.data.cr;
    _chr(_m_write_rm(context, 1));
    _ce;
}
static void MOV_RM32_R32(core_machine_cpu_execution_context *context) {
    _cb("MOV_RM32_R32");
    i386(0x89) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        instruction_state.data.crm = instruction_state.data.cr;
        _chr(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _d_modrm(context, 2, 2);
        instruction_state.data.crm = instruction_state.data.cr;
        _chr(_m_write_rm(context, 2));
    }
    _ce;
}
static void MOV_R8_RM8(core_machine_cpu_execution_context *context) {
    _cb("MOV_R8_RM8");
    i386(0x8a) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 1, 1));
    _chr(_m_read_rm(context, 1));
    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.crm), 1));
    _ce;
}
static void MOV_R32_RM32(core_machine_cpu_execution_context *context) {
    _cb("MOV_R32_RM32");
    i386(0x8b) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.crm), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _d_modrm(context, 2, 2);
        _chr(_m_read_rm(context, 2));
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.crm), 2));
    }
    _ce;
}
static void MOV_RM16_SREG(core_machine_cpu_execution_context *context) {
    _cb("MOV_RM16_SREG");
    i386(0x8c) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm_sreg(context, 2));
    instruction_state.data.crm = instruction_state.data.rmovsreg->selector;
    _chr(_m_write_rm(context, 2));
    _ce;
}
static void LEA_R32_M32(core_machine_cpu_execution_context *context) {
    _cb("LEA_R32_M32");
    i386(0x8d) {
        _adv;
        _chr(_d_modrm_ea(context, _GetOperandSize, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.mrm.offset), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.mrm.offset), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _d_modrm_ea(context, 2, 2);
        _m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.mrm.offset), 2);
    }
    _ce;
}
static void MOV_SREG_RM16(core_machine_cpu_execution_context *context) {
    _cb("MOV_SREG_RM16");
    i386(0x8e) {
        _adv;
        _chr(_d_modrm_sreg(context, 2));
        if (instruction_state.data.rmovsreg->sregtype == SREG_CODE) {
            _bb("sregtype(SREG_CODE)");
            _chr(_SetExcept_UD(0));
            _be;
        }
        _chr(_m_read_rm(context, 2));
        _chr(_s_load_sreg(context, instruction_state.data.rmovsreg, GetMax16(instruction_state.data.crm)));
        if (instruction_state.data.rmovsreg->sregtype == SREG_STACK)
            instruction_state.data.flagMaskInt = True;
    }
    else {
        cpu_state.data.ip++;
        _d_modrm_sreg(context, 2);
        _chr(_m_read_rm(context, 2));
        _chr(_s_load_sreg(context, instruction_state.data.rmovsreg, GetMax16(instruction_state.data.crm)));
    }
    _ce;
}
static void INS_8F(core_machine_cpu_execution_context *context) {
    _cb("INS_8F");
    i386(0x8f) {
        _adv;
        _chr(_d_modrm(context, 0, _GetOperandSize));
        switch (instruction_state.data.cr) {
        case 0: /* POP_RM32 */
            _bb("POP_RM32");
            _chr(_e_pop(context, GetRef(instruction_state.data.crm), _GetOperandSize));
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 1:
            _bb("instruction_state.data.cr(1)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 2:
            _bb("instruction_state.data.cr(2)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 3:
            _bb("instruction_state.data.cr(3)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 4:
            _bb("instruction_state.data.cr(4)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 5:
            _bb("instruction_state.data.cr(5)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 6:
            _bb("instruction_state.data.cr(6)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 7:
            _bb("instruction_state.data.cr(7)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 0, 2));
        switch (instruction_state.data.cr) {
        case 0: /* POP_RM16 */
            _bb("POP_RM16");
            _chr(_e_pop(context, GetRef(instruction_state.data.crm), 2));
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 1:
            _bb("instruction_state.data.cr(1)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 2:
            _bb("instruction_state.data.cr(2)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 3:
            _bb("instruction_state.data.cr(3)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 4:
            _bb("instruction_state.data.cr(4)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 5:
            _bb("instruction_state.data.cr(5)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 6:
            _bb("instruction_state.data.cr(6)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 7:
            _bb("instruction_state.data.cr(7)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    _ce;
}
static void NOP(core_machine_cpu_execution_context *context) {
    _cb("NOP");
    i386(0x90) {
        _adv;
    }
    else
        cpu_state.data.ip++;
    _ce;
}
static void XCHG_ECX_EAX(core_machine_cpu_execution_context *context) {
    _cb("XCHG_ECX_EAX");
    i386(0x91) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.cx;
            cpu_state.data.cx = GetMax16(instruction_state.data.cr);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.ecx;
            cpu_state.data.ecx = GetMax32(instruction_state.data.cr);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.cx;
        cpu_state.data.cx = GetMax16(instruction_state.data.cr);
    }
    _ce;
}
static void XCHG_EDX_EAX(core_machine_cpu_execution_context *context) {
    _cb("XCHG_EDX_EAX");
    i386(0x92) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.dx;
            cpu_state.data.dx = GetMax16(instruction_state.data.cr);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.edx;
            cpu_state.data.edx = GetMax32(instruction_state.data.cr);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.dx;
        cpu_state.data.dx = GetMax16(instruction_state.data.cr);
    }
    _ce;
}
static void XCHG_EBX_EAX(core_machine_cpu_execution_context *context) {
    _cb("XCHG_EBX_EAX");
    i386(0x93) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.bx;
            cpu_state.data.bx = GetMax16(instruction_state.data.cr);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.ebx;
            cpu_state.data.ebx = GetMax32(instruction_state.data.cr);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.bx;
        cpu_state.data.bx = GetMax16(instruction_state.data.cr);
    }
    _ce;
}
static void XCHG_ESP_EAX(core_machine_cpu_execution_context *context) {
    _cb("XCHG_ESP_EAX");
    _new_code_path_;
    i386(0x94) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.sp;
            cpu_state.data.sp = GetMax16(instruction_state.data.cr);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.esp;
            cpu_state.data.esp = GetMax32(instruction_state.data.cr);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.sp;
        cpu_state.data.sp = GetMax16(instruction_state.data.cr);
    }
    _ce;
}
static void XCHG_EBP_EAX(core_machine_cpu_execution_context *context) {
    _cb("XCHG_EBP_EAX");
    i386(0x95) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.bp;
            cpu_state.data.bp = GetMax16(instruction_state.data.cr);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.ebp;
            cpu_state.data.ebp = GetMax32(instruction_state.data.cr);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.bp;
        cpu_state.data.bp = GetMax16(instruction_state.data.cr);
    }
    _ce;
}
static void XCHG_ESI_EAX(core_machine_cpu_execution_context *context) {
    _cb("XCHG_ESI_EAX");
    i386(0x96) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.si;
            cpu_state.data.si = GetMax16(instruction_state.data.cr);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.esi;
            cpu_state.data.esi = GetMax32(instruction_state.data.cr);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.si;
        cpu_state.data.si = GetMax16(instruction_state.data.cr);
    }
    _ce;
}
static void XCHG_EDI_EAX(core_machine_cpu_execution_context *context) {
    _cb("XCHG_EDI_EAX");
    i386(0x97) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.di;
            cpu_state.data.di = GetMax16(instruction_state.data.cr);
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.edi;
            cpu_state.data.edi = GetMax32(instruction_state.data.cr);
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.di;
        cpu_state.data.di = GetMax16(instruction_state.data.cr);
    }
    _ce;
}
static void CBW(core_machine_cpu_execution_context *context) {
    _cb("CBW");
    i386(0x98) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.ax = (t_nsbit8)cpu_state.data.al;
            break;
        case 4:
            cpu_state.data.eax = (t_nsbit16)cpu_state.data.ax;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        cpu_state.data.ax = (t_nsbit8)cpu_state.data.al;
    }
    _ce;
}
static void CWD(core_machine_cpu_execution_context *context) {
    _cb("CWD");
    i386(0x99) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.dx = GetMSB16(cpu_state.data.ax) ? 0xffff : 0x0000;
            break;
        case 4:
            cpu_state.data.edx = GetMSB32(cpu_state.data.eax) ? 0xffffffff : 0x00000000;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        if (cpu_state.data.ax & 0x8000) cpu_state.data.dx = 0xffff;
        else cpu_state.data.dx = 0x0000;
    }
    _ce;
}
static void CALL_PTR16_32(core_machine_cpu_execution_context *context) {
    t_nubit16 newcs;
    t_nubit32 neweip;
    _cb("CALL_PTR16_32");
    i386(0x9a) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(context, 4));
            neweip = GetMax16(instruction_state.data.cimm);
            newcs = GetMax16(instruction_state.data.cimm >> 16);
            _chr(_e_call_far(context, newcs, neweip, 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_d_imm(context, 8));
            neweip = GetMax32(instruction_state.data.cimm);
            newcs = GetMax16(instruction_state.data.cimm >> 32);
            _chr(_e_call_far(context, newcs, neweip, 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _d_imm(context, 4);
        neweip = GetMax16(instruction_state.data.cimm);
        newcs = GetMax16(instruction_state.data.cimm >> 16);
        _e_call_far(context, newcs, neweip, 2);
    }
    _ce;
}
_______todo WAIT(core_machine_cpu_execution_context *context) {
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
        cpu_state.data.ip++;
    }
    _ce;
}
static void PUSHF(core_machine_cpu_execution_context *context) {
    t_nubit32 ceflags;
    _cb("PUSHF");
    i386(0x9c) {
        _adv;
        if (!_GetCR0_PE || (_GetCR0_PE && (!_GetEFLAGS_VM || (_GetEFLAGS_VM && (_GetEFLAGS_IOPL == 3))))) {
            _bb("Real/Protected/(V86,IOPL(3))");
            switch (_GetOperandSize) {
            case 2:
                _bb("OperandSize(2)");
                ceflags = cpu_state.data.flags;
                _chr(_e_push(context, GetRef(ceflags), 2));
                _be;
                break;
            case 4:
                _bb("OperandSize(4)");
                ceflags = cpu_state.data.eflags & ~(VCPU_EFLAGS_VM | VCPU_EFLAGS_RF);
                _chr(_e_push(context, GetRef(ceflags), 4));
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
        cpu_state.data.ip++;
        _e_push(context, GetRef(cpu_state.data.flags), 2);
    }
    _ce;
}
static void POPF(core_machine_cpu_execution_context *context) {
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
                    _chr(_e_pop(context, GetRef(ceflags), 2));
                    mask |= 0xffff0000;
                    _be;
                    break;
                case 4:
                    _bb("OperandSize(4)");
                    _chr(_e_pop(context, GetRef(ceflags), 4));
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
                    _chr(_e_pop(context, GetRef(ceflags), 2));
                    mask |= (0xffff0000 | VCPU_EFLAGS_IOPL);
                    _be;
                    break;
                case 4:
                    _bb("OperandSize(4)");
                    _chr(_e_pop(context, GetRef(ceflags), 4));
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
            if (instruction_state.data.prefix_oprsize) {
                _bb("prefix_oprsize(1)");
                _chr(_SetExcept_GP(0));
                _be;
            }
            if (_GetEFLAGS_IOPL == 0x03) {
                _bb("EFLAGS_IOPL(3)");
                switch (_GetOperandSize) {
                case 2:
                    _bb("OperandSize(2)");
                    _chr(_e_pop(context, GetRef(ceflags), 2));
                    mask |= (0xffff0000 | VCPU_EFLAGS_IOPL);
                    _be;
                    break;
                case 4:
                    _bb("OperandSize(4)");
                    _chr(_e_pop(context, GetRef(ceflags), 4));
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
        cpu_state.data.eflags = (ceflags & ~mask) | (cpu_state.data.eflags & mask);
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_pop(context, GetRef(cpu_state.data.flags), 2));
    }
    _ce;
}
static void SAHF(core_machine_cpu_execution_context *context) {
    t_nubit32 mask = (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                      VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_CF);
    _cb("SAHF");
    i386(0x9e) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    cpu_state.data.eflags = (cpu_state.data.ah & mask) | (cpu_state.data.eflags & ~mask);
    _ce;
}
static void LAHF(core_machine_cpu_execution_context *context) {
    _cb("LAHF");
    i386(0x9f) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    cpu_state.data.ah = GetMax8(cpu_state.data.flags);
    _ce;
}
static void MOV_AL_MOFFS8(core_machine_cpu_execution_context *context) {
    _cb("MOV_AL_MOFFS8");
    i386(0xa0) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_moffs(context, 1));
    _chr(_m_read_rm(context, 1));
    cpu_state.data.al = GetMax8(instruction_state.data.crm);
    _ce;
}
static void MOV_EAX_MOFFS32(core_machine_cpu_execution_context *context) {
    _cb("MOV_EAX_MOFFS32");
    i386(0xa1) {
        _adv;
        _chr(_d_moffs(context, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.ax = GetMax16(instruction_state.data.crm);
            break;
        case 4:
            cpu_state.data.eax = GetMax32(instruction_state.data.crm);
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_moffs(context, 2));
        _chr(_m_read_rm(context, 2));
        cpu_state.data.ax = GetMax16(instruction_state.data.crm);
    }
    _ce;
}
static void MOV_MOFFS8_AL(core_machine_cpu_execution_context *context) {
    _cb("MOV_MOFFS8_AL");
    i386(0xa2) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_moffs(context, 1));
    instruction_state.data.result = cpu_state.data.al;
    instruction_state.data.crm = instruction_state.data.result;
    _chr(_m_write_rm(context, 1));
    _ce;
}
static void MOV_MOFFS32_EAX(core_machine_cpu_execution_context *context) {
    _cb("MOV_MOFFS32_EAX");
    i386(0xa3) {
        _adv;
        _chr(_d_moffs(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            instruction_state.data.result = cpu_state.data.ax;
            break;
        case 4:
            instruction_state.data.result = cpu_state.data.eax;
            break;
        default:
            _impossible_r_;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_moffs(context, 2));
        instruction_state.data.crm = cpu_state.data.ax;
        _chr(_m_write_rm(context, 2));
    }
    _ce;
}
static void MOVSB(core_machine_cpu_execution_context *context) {
    _cb("MOVSB");
    i386(0xa4) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_m_movs(context, 1));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (cpu_state.data.cx) {
                    _bb("cx(!0)");
                    _chr(_m_movs(context, 1));
                    cpu_state.data.cx--;
                    _be;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_m_movs(context, 1));
                    cpu_state.data.ecx--;
                    _be;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = True;
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
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) _m_movs(context, 1);
        else {
            if (cpu_state.data.cx) {
                _m_movs(context, 1);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx) instruction_state.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void MOVSW(core_machine_cpu_execution_context *context) {
    _cb("MOVSW");
    i386(0xa5) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_m_movs(context, _GetOperandSize));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (cpu_state.data.cx) {
                    _bb("cx(!0)");
                    _chr(_m_movs(context, _GetOperandSize));
                    cpu_state.data.cx--;
                    _be;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_m_movs(context, _GetOperandSize));
                    cpu_state.data.ecx--;
                    _be;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = True;
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
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) _m_movs(context, 2);
        else {
            if (cpu_state.data.cx) {
                _m_movs(context, 2);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx) instruction_state.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void CMPSB(core_machine_cpu_execution_context *context) {
    _cb("CMPSB");
    i386(0xa6) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_a_cmps(context, 8));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (cpu_state.data.cx) {
                    _bb("cx(!0)");
                    _chr(_a_cmps(context, 8));
                    cpu_state.data.cx--;
                    _be;
                }
                if (cpu_state.data.cx &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_a_cmps(context, 8));
                    cpu_state.data.ecx--;
                    _be;
                }
                if (cpu_state.data.ecx &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = True;
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
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) _a_cmps(context, 8);
        else {
            if (cpu_state.data.cx) {
                _a_cmps(context, 8);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                instruction_state.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void CMPSW(core_machine_cpu_execution_context *context) {
    _cb("CMPSW");
    i386(0xa7) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_a_cmps(context, _GetOperandSize * 8));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (cpu_state.data.cx) {
                    _bb("cx(!0)");
                    _chr(_a_cmps(context, _GetOperandSize * 8));
                    cpu_state.data.cx--;
                    _be;
                }
                if (cpu_state.data.cx &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_a_cmps(context, _GetOperandSize * 8));
                    cpu_state.data.ecx--;
                    _be;
                }
                if (cpu_state.data.ecx &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = True;
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
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) _a_cmps(context, 16);
        else {
            if (cpu_state.data.cx) {
                _a_cmps(context, 16);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                instruction_state.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void TEST_AL_I8(core_machine_cpu_execution_context *context) {
    _cb("TEST_AL_I8");
    i386(0xa8) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_a_test(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    _ce;
}
static void TEST_EAX_I32(core_machine_cpu_execution_context *context) {
    _cb("TEST_EAX_I32");
    i386(0xa9) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(context, 2));
            _chr(_a_test(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            _chr(_d_imm(context, 4));
            _chr(_a_test(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        _chr(_a_test(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
    }
    _ce;
}
static void STOSB(core_machine_cpu_execution_context *context) {
    _cb("STOSB");
    i386(0xaa) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_m_stos(context, 1));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (cpu_state.data.cx) {
                    _bb("cx(!0)");
                    _chr(_m_stos(context, 1));
                    cpu_state.data.cx--;
                    _be;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_m_stos(context, 1));
                    cpu_state.data.ecx--;
                    _be;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = True;
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
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) _m_stos(context, 1);
        else {
            if (cpu_state.data.cx) {
                _m_stos(context, 1);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx) instruction_state.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void STOSW(core_machine_cpu_execution_context *context) {
    _cb("STOSW");
    i386(0xab) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_m_stos(context, _GetOperandSize));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (cpu_state.data.cx) {
                    _bb("cx(!0)");
                    _chr(_m_stos(context, _GetOperandSize));
                    cpu_state.data.cx--;
                    _be;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_m_stos(context, _GetOperandSize));
                    cpu_state.data.ecx--;
                    _be;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = True;
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
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) _m_stos(context, 2);
        else {
            if (cpu_state.data.cx) {
                _m_stos(context, 2);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx) instruction_state.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void LODSB(core_machine_cpu_execution_context *context) {
    _cb("LODSB");
    i386(0xac) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_m_lods(context, 1));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (cpu_state.data.cx) {
                    _bb("cx(!0)");
                    _chr(_m_lods(context, 1));
                    cpu_state.data.cx--;
                    _be;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_m_lods(context, 1));
                    cpu_state.data.ecx--;
                    _be;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = True;
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
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) _m_lods(context, 1);
        else {
            if (cpu_state.data.cx) {
                _m_lods(context, 1);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx) instruction_state.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void LODSW(core_machine_cpu_execution_context *context) {
    _cb("LODSW");
    i386(0xad) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_m_lods(context, _GetOperandSize));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (cpu_state.data.cx) {
                    _bb("cx(!0)");
                    _chr(_m_lods(context, _GetOperandSize));
                    cpu_state.data.cx--;
                    _be;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_m_lods(context, _GetOperandSize));
                    cpu_state.data.ecx--;
                    _be;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = True;
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
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) _m_lods(context, 2);
        else {
            if (cpu_state.data.cx) {
                _m_lods(context, 2);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx) instruction_state.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void SCASB(core_machine_cpu_execution_context *context) {
    _cb("SCASB");
    i386(0xae) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_a_scas(context, 8));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (cpu_state.data.cx) {
                    _bb("cx(!0)");
                    _chr(_a_scas(context, 8));
                    cpu_state.data.cx--;
                    _be;
                }
                if (cpu_state.data.cx &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_a_scas(context, 8));
                    cpu_state.data.ecx--;
                    _be;
                }
                if (cpu_state.data.ecx &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = True;
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
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) _a_scas(context, 8);
        else {
            if (cpu_state.data.cx) {
                _a_scas(context, 8);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                instruction_state.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void SCASW(core_machine_cpu_execution_context *context) {
    _cb("SCASW");
    i386(0xaf) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            _bb("prefix_rep(PREFIX_REP_NONE)");
            _chr(_a_scas(context, _GetOperandSize * 8));
            _be;
        } else {
            _bb("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                _bb("AddressSize(2)");
                if (cpu_state.data.cx) {
                    _bb("cx(!0)");
                    _chr(_a_scas(context, _GetOperandSize * 8));
                    cpu_state.data.cx--;
                    _be;
                }
                if (cpu_state.data.cx &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = True;
                _be;
                break;
            case 4:
                _bb("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    _bb("ecx(!0)");
                    _chr(_a_scas(context, _GetOperandSize * 8));
                    cpu_state.data.ecx--;
                    _be;
                }
                if (cpu_state.data.ecx &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = True;
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
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) _a_scas(context, 16);
        else {
            if (cpu_state.data.cx) {
                _a_scas(context, 16);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                instruction_state.data.flagInsLoop = True;
        }
    }
    _ce;
}
static void MOV_AL_I8(core_machine_cpu_execution_context *context) {
    _cb("MOV_AL_I8");
    i386(0xb0) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    cpu_state.data.al = GetMax8(instruction_state.data.cimm);
    _ce;
}
static void MOV_CL_I8(core_machine_cpu_execution_context *context) {
    _cb("MOV_CL_I8");
    i386(0xb1) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    cpu_state.data.cl = GetMax8(instruction_state.data.cimm);
    _ce;
}
static void MOV_DL_I8(core_machine_cpu_execution_context *context) {
    _cb("MOV_DL_I8");
    i386(0xb2) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    cpu_state.data.dl = GetMax8(instruction_state.data.cimm);
    _ce;
}
static void MOV_BL_I8(core_machine_cpu_execution_context *context) {
    _cb("MOV_BL_I8");
    i386(0xb3) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    cpu_state.data.bl = GetMax8(instruction_state.data.cimm);
    _ce;
}
static void MOV_AH_I8(core_machine_cpu_execution_context *context) {
    _cb("MOV_AH_I8");
    i386(0xb4) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    cpu_state.data.ah = GetMax8(instruction_state.data.cimm);
    _ce;
}
static void MOV_CH_I8(core_machine_cpu_execution_context *context) {
    _cb("MOV_CH_I8");
    i386(0xb5) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    cpu_state.data.ch = GetMax8(instruction_state.data.cimm);
    _ce;
}
static void MOV_DH_I8(core_machine_cpu_execution_context *context) {
    _cb("MOV_DH_I8");
    i386(0xb6) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    cpu_state.data.dh = GetMax8(instruction_state.data.cimm);
    _ce;
}
static void MOV_BH_I8(core_machine_cpu_execution_context *context) {
    _cb("MOV_BH_I8");
    i386(0xb7) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    cpu_state.data.bh = GetMax8(instruction_state.data.cimm);
    _ce;
}
static void MOV_EAX_I32(core_machine_cpu_execution_context *context) {
    _cb("MOV_EAX_I32");
    i386(0xb8) {
        _adv;
        _chr(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.ax = GetMax16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.eax = GetMax32(instruction_state.data.cimm);
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        cpu_state.data.ax = GetMax16(instruction_state.data.cimm);
    }
    _ce;
}
static void MOV_ECX_I32(core_machine_cpu_execution_context *context) {
    _cb("MOV_ECX_I32");
    i386(0xb9) {
        _adv;
        _chr(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.cx = GetMax16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.ecx = GetMax32(instruction_state.data.cimm);
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        cpu_state.data.cx = GetMax16(instruction_state.data.cimm);
    }
    _ce;
}
static void MOV_EDX_I32(core_machine_cpu_execution_context *context) {
    _cb("MOV_EDX_I32");
    i386(0xba) {
        _adv;
        _chr(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.dx = GetMax16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.edx = GetMax32(instruction_state.data.cimm);
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        cpu_state.data.dx = GetMax16(instruction_state.data.cimm);
    }
    _ce;
}
static void MOV_EBX_I32(core_machine_cpu_execution_context *context) {
    _cb("MOV_EBX_I32");
    i386(0xbb) {
        _adv;
        _chr(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.bx = GetMax16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.ebx = GetMax32(instruction_state.data.cimm);
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        cpu_state.data.bx = GetMax16(instruction_state.data.cimm);
    }
    _ce;
}
static void MOV_ESP_I32(core_machine_cpu_execution_context *context) {
    _cb("MOV_ESP_I32");
    i386(0xbc) {
        _adv;
        _chr(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.sp = GetMax16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.esp = GetMax32(instruction_state.data.cimm);
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        cpu_state.data.sp = GetMax16(instruction_state.data.cimm);
    }
    _ce;
}
static void MOV_EBP_I32(core_machine_cpu_execution_context *context) {
    _cb("MOV_EBP_I32");
    i386(0xbd) {
        _adv;
        _chr(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.bp = GetMax16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.ebp = GetMax32(instruction_state.data.cimm);
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        cpu_state.data.bp = GetMax16(instruction_state.data.cimm);
    }
    _ce;
}
static void MOV_ESI_I32(core_machine_cpu_execution_context *context) {
    _cb("MOV_ESI_I32");
    i386(0xbe) {
        _adv;
        _chr(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.si = GetMax16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.esi = GetMax32(instruction_state.data.cimm);
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        cpu_state.data.si = GetMax16(instruction_state.data.cimm);
    }
    _ce;
}
static void MOV_EDI_I32(core_machine_cpu_execution_context *context) {
    _cb("MOV_EDI_I32");
    i386(0xbf) {
        _adv;
        _chr(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.di = GetMax16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.edi = GetMax32(instruction_state.data.cimm);
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        cpu_state.data.di = GetMax16(instruction_state.data.cimm);
    }
    _ce;
}
static void INS_C0(core_machine_cpu_execution_context *context) {
    _cb("INS_C0");
    _new_code_path_;
    i386(0xc0) {
        _adv;
        _chr(_d_modrm(context, 0, 1));
        _chr(_m_read_rm(context, 1));
        _chr(_d_imm(context, 1));
        switch (instruction_state.data.cr) {
        case 0: /* ROL_RM8_I8 */
            _bb("ROL_RM8_I8");
            _chr(_a_rol(context, GetMax8(instruction_state.data.crm), GetMax8(instruction_state.data.cimm), 8));
            _be;
            break;
        case 1: /* ROR_RM8_I8 */
            _bb("ROR_RM8_I8");
            _chr(_a_ror(context, GetMax8(instruction_state.data.crm), GetMax8(instruction_state.data.cimm), 8));
            _be;
            break;
        case 2: /* RCL_RM8_I8 */
            _bb("RCL_RM8_I8");
            _chr(_a_rcl(context, GetMax8(instruction_state.data.crm), GetMax8(instruction_state.data.cimm), 8));
            _be;
            break;
        case 3: /* RCR_RM8_I8 */
            _bb("RCR_RM8_I8");
            _chr(_a_rcr(context, GetMax8(instruction_state.data.crm), GetMax8(instruction_state.data.cimm), 8));
            _be;
            break;
        case 4: /* SHL_RM8_I8 */
            _bb("SHL_RM8_I8");
            _chr(_a_shl(context, GetMax8(instruction_state.data.crm), GetMax8(instruction_state.data.cimm), 8));
            _be;
            break;
        case 5: /* SHR_RM8_I8 */
            _bb("SHR_RM8_I8");
            _chr(_a_shr(context, GetMax8(instruction_state.data.crm), GetMax8(instruction_state.data.cimm), 8));
            _be;
            break;
        case 6: /* UndefinedOpcode */
            _bb("instruction_state.data.cr(6)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 7: /* SAR_RM8_I8 */
            _bb("SAR_RM8_I8");
            _chr(_a_sar(context, GetMax8(instruction_state.data.crm), GetMax8(instruction_state.data.cimm), 8));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 1));
    }
    else
        UndefinedOpcode(context);
    _ce;
}
static void INS_C1(core_machine_cpu_execution_context *context) {
    _cb("INS_C1");
    i386(0xc1) {
        _adv;
        _chr(_d_modrm(context, 0, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        _chr(_d_imm(context, 1));
        switch (instruction_state.data.cr) {
        case 0: /* ROL_RM32_I8 */
            _bb("ROL_RM32_I8");
            _chr(_a_rol(context, GetMax32(instruction_state.data.crm), GetMax8(instruction_state.data.cimm), _GetOperandSize * 8));
            _be;
            break;
        case 1: /* ROR_RM32_I8 */
            _bb("ROR_RM32_I8");
            _chr(_a_ror(context, GetMax32(instruction_state.data.crm), GetMax8(instruction_state.data.cimm), _GetOperandSize * 8));
            _be;
            break;
        case 2: /* RCL_RM32_I8 */
            _bb("RCL_RM32_I8");
            _chr(_a_rcl(context, GetMax32(instruction_state.data.crm), GetMax8(instruction_state.data.cimm), _GetOperandSize * 8));
            _be;
            break;
        case 3: /* RCR_RM32_I8 */
            _bb("RCR_RM32_I8");
            _chr(_a_rcr(context, GetMax32(instruction_state.data.crm), GetMax8(instruction_state.data.cimm), _GetOperandSize * 8));
            _be;
            break;
        case 4: /* SHL_RM32_I8 */
            _bb("SHL_RM32_I8");
            _chr(_a_shl(context, GetMax32(instruction_state.data.crm), GetMax8(instruction_state.data.cimm), _GetOperandSize * 8));
            _be;
            break;
        case 5: /* SHR_RM32_I8 */
            _bb("SHR_RM32_I8");
            _chr(_a_shr(context, GetMax32(instruction_state.data.crm), GetMax8(instruction_state.data.cimm), _GetOperandSize * 8));
            _be;
            break;
        case 6: /* UndefinedOpcode */
            _bb("instruction_state.data.cr(6)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 7: /* SAR_RM32_I8 */
            _bb("SAR_RM32_I8");
            _chr(_a_sar(context, GetMax32(instruction_state.data.crm), GetMax8(instruction_state.data.cimm), _GetOperandSize * 8));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    _ce;
}
static void RET_I16(core_machine_cpu_execution_context *context) {
    _cb("RET_I16");
    i386(0xc2) {
        _adv;
        _chr(_d_imm(context, 2));
        _chr(_e_ret_near(context, GetMax16(instruction_state.data.cimm), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _d_imm(context, 2);
        _e_ret_near(context, GetMax16(instruction_state.data.cimm), 2);
    }
    _ce;
}
static void RET(core_machine_cpu_execution_context *context) {
    _cb("RET");
    i386(0xc3) {
        _adv;
        _chr(_e_ret_near(context, 0, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_ret_near(context, 0, 2));
    }
    _ce;
}
static void LES_R32_M16_32(core_machine_cpu_execution_context *context) {
    t_nubit16 selector;
    t_nubit32 offset;
    _cb("LES_R32_M16_32");
    i386(0xc4) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize + 2));
        if (!instruction_state.data.flagMem) {
            _bb("flagMem(0)");
            _chr(UndefinedOpcode(context));
            _be;
        }
        _chr(_m_read_rm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            offset = GetMax16(instruction_state.data.crm);
            break;
        case 4:
            offset = GetMax32(instruction_state.data.crm);
            break;
        default:
            _impossible_r_;
            break;
        }
        instruction_state.data.mrm.offset += _GetOperandSize;
        _chr(_m_read_rm(context, 2));
        selector = GetMax16(instruction_state.data.crm);
        _chr(_e_load_far(context, &cpu_state.data.es, instruction_state.data.rr, selector, offset, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _d_modrm(context, 2, 4);
        _chr(_m_read_rm(context, 2));
        offset = GetMax16(instruction_state.data.crm);
        instruction_state.data.mrm.offset += 2;
        _chr(_m_read_rm(context, 2));
        selector = GetMax16(instruction_state.data.crm);
        _chr(_e_load_far(context, &cpu_state.data.es, instruction_state.data.rr, selector, offset, 2));
    }
    _ce;
}
static void LDS_R32_M16_32(core_machine_cpu_execution_context *context) {
    t_nubit16 selector;
    t_nubit32 offset;
    _cb("LDS_R32_M16_32");
    i386(0xc5) {
        _adv;
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize + 2));
        if (!instruction_state.data.flagMem) {
            _bb("flagMem(0)");
            _chr(UndefinedOpcode(context));
            _be;
        }
        _chr(_m_read_rm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            offset = GetMax16(instruction_state.data.crm);
            break;
        case 4:
            offset = GetMax32(instruction_state.data.crm);
            break;
        default:
            _impossible_r_;
            break;
        }
        instruction_state.data.mrm.offset += _GetOperandSize;
        _chr(_m_read_rm(context, 2));
        selector = GetMax16(instruction_state.data.crm);
        _chr(_e_load_far(context, &cpu_state.data.ds, instruction_state.data.rr, selector, offset, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _d_modrm(context, 2, 4);
        _chr(_m_read_rm(context, 2));
        offset = GetMax16(instruction_state.data.crm);
        instruction_state.data.mrm.offset += 2;
        _chr(_m_read_rm(context, 2));
        selector = GetMax16(instruction_state.data.crm);
        _chr(_e_load_far(context, &cpu_state.data.ds, instruction_state.data.rr, selector, offset, 2));
    }
    _ce;
}
static void INS_C6(core_machine_cpu_execution_context *context) {
    _cb("INS_C6");
    i386(0xc6) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 0, 1));
    switch (instruction_state.data.cr) {
    case 0: /* MOV_RM8_I8 */
        _bb("MOV_RM8_I8");
        _chr(_d_imm(context, 1));
        instruction_state.data.crm = instruction_state.data.cimm;
        _chr(_m_write_rm(context, 1));
        _be;
        break;
    case 1:
        _bb("instruction_state.data.cr(1)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 2:
        _bb("instruction_state.data.cr(2)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 3:
        _bb("instruction_state.data.cr(3)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 4:
        _bb("instruction_state.data.cr(4)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 5:
        _bb("instruction_state.data.cr(5)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 6:
        _bb("instruction_state.data.cr(6)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 7:
        _bb("instruction_state.data.cr(7)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void INS_C7(core_machine_cpu_execution_context *context) {
    _cb("INS_C7");
    i386(0xc7) {
        _adv;
        _chr(_d_modrm(context, 0, _GetOperandSize));
        switch (instruction_state.data.cr) {
        case 0: /* MOV_RM32_I32 */
            _bb("MOV_RM32_I32");
            _chr(_d_imm(context, _GetOperandSize));
            instruction_state.data.crm = instruction_state.data.cimm;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 1:
            _bb("instruction_state.data.cr(1)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 2:
            _bb("instruction_state.data.cr(2)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 3:
            _bb("instruction_state.data.cr(3)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 4:
            _bb("instruction_state.data.cr(4)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 5:
            _bb("instruction_state.data.cr(5)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 6:
            _bb("instruction_state.data.cr(6)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 7:
            _bb("instruction_state.data.cr(7)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 0, 2));
        switch (instruction_state.data.cr) {
        case 0: /* MOV_RM16_I16 */
            _bb("MOV_RM16_I16");
            _chr(_d_imm(context, 2));
            instruction_state.data.crm = instruction_state.data.cimm;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 1:
            _bb("instruction_state.data.cr(1)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 2:
            _bb("instruction_state.data.cr(2)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 3:
            _bb("instruction_state.data.cr(3)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 4:
            _bb("instruction_state.data.cr(4)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 5:
            _bb("instruction_state.data.cr(5)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 6:
            _bb("instruction_state.data.cr(6)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 7:
            _bb("instruction_state.data.cr(7)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    _ce;
}
static void ENTER(core_machine_cpu_execution_context *context) {
    t_nubit32 data = 0;
    t_nubit32 temp = 0;
    t_nubitcc i = 0;
    t_nubit16 size = Zero16;
    t_nubit8 level = Zero8;
    _cb("ENTER");
    i386(0xc8) {
        _adv;
        _chr(_d_imm(context, 2));
        size = (t_nubit16) instruction_state.data.cimm;
        _chr(_d_imm(context, 1));
        level = (t_nubit8) instruction_state.data.cimm;
        level %= 32;
        switch (_GetStackSize) {
        case 2:
            _bb("StackSize(2)");
            _chr(_e_push(context, GetRef(cpu_state.data.bp), 2));
            temp = cpu_state.data.sp;
            _be;
            break;
        case 4:
            _bb("StackSize(4)");
            _chr(_e_push(context, GetRef(cpu_state.data.ebp), 4));
            temp = cpu_state.data.esp;
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
                        cpu_state.data.bp -= 2;
                        _chr(_s_read_ss(context, cpu_state.data.bp, GetRef(data), 2));
                        _chr(_e_push(context, GetRef(data), 2));
                        _be;
                        break;
                    case 4:
                        _bb("StackSize(4)");
                        cpu_state.data.ebp -= 2;
                        _chr(_s_read_ss(context, cpu_state.data.ebp, GetRef(data), 2));
                        _chr(_e_push(context, GetRef(data), 2));
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
                        cpu_state.data.bp -= 4;
                        _chr(_s_read_ss(context, cpu_state.data.bp, GetRef(data), 4));
                        _chr(_e_push(context, GetRef(data), 4));
                        _be;
                        break;
                    case 4:
                        _bb("StackSize(4)");
                        cpu_state.data.ebp -= 4;
                        _chr(_s_read_ss(context, cpu_state.data.ebp, GetRef(data), 4));
                        _chr(_e_push(context, GetRef(data), 4));
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
                _chr(_e_push(context, GetRef(temp), 2));
                _be;
                break;
            case 4:
                _bb("OperandSize(4)");
                _chr(_e_push(context, GetRef(temp), 4));
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
            cpu_state.data.bp = GetMax16(temp);
            cpu_state.data.sp = cpu_state.data.bp - size;
            _be;
            break;
        case 4:
            _bb("StackSize(4)");
            cpu_state.data.ebp = GetMax32(temp);
            cpu_state.data.esp = cpu_state.data.ebp - size;
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    _ce;
}
static void LEAVE(core_machine_cpu_execution_context *context) {
    _cb("LEAVE");
    i386(0xc9) {
        _adv;
        if (!_IsProtected && cpu_state.data.ebp > 0x0000ffff) {
            _bb("Protected(0),ebp(>0000ffff)");
            _chr(_SetExcept_GP(0));
            _be;
        }
        _chr(_m_test_logical(context, &cpu_state.data.ss, cpu_state.data.ebp, _GetOperandSize, 1));
        switch (_GetStackSize) {
        case 2:
            cpu_state.data.sp = cpu_state.data.bp;
            break;
        case 4:
            cpu_state.data.esp = cpu_state.data.ebp;
            break;
        default:
            _impossible_r_;
            break;
        }
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_e_pop(context, GetRef(cpu_state.data.bp), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_e_pop(context, GetRef(cpu_state.data.ebp), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    _ce;
}
static void RETF_I16(core_machine_cpu_execution_context *context) {
    _cb("RETF_I16");
    i386(0xca) {
        _adv;
        _chr(_d_imm(context, 2));
        _chr(_e_ret_far(context, GetMax16(instruction_state.data.cimm), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _d_imm(context, 2);
        _e_ret_far(context, GetMax16(instruction_state.data.cimm), 2);
    }
    _ce;
}
static void RETF(core_machine_cpu_execution_context *context) {
    _cb("RETF");
    i386(0xcb) {
        _adv;
        _chr(_e_ret_far(context, 0, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_ret_far(context, 0, 2));
    }
    _ce;
}
static void INT3(core_machine_cpu_execution_context *context) {
    _cb("INT3");
    _new_code_path_;
    i386(0xcc) {
        _adv;
        _chr(_e_int3(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _e_int_n(context, 0x03, _GetOperandSize);
    }
    _ce;
}
static void INT_I8(core_machine_cpu_execution_context *context) {
    _cb("INT_I8");
    i386(0xcc) {
        _adv;
        _chr(_d_imm(context, 1));
        _chr(_e_int_n(context, (t_nubit8)instruction_state.data.cimm, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _d_imm(context, 1);
        _e_int_n(context, (t_nubit8)instruction_state.data.cimm, 2);
    }
    _ce;
}
static void INTO(core_machine_cpu_execution_context *context) {
    _cb("INTO");
    _new_code_path_;
    i386(0xce) {
        _adv;
        _chr(_e_into(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        if (_GetEFLAGS_OF) _e_int_n(context, 0x04, _GetOperandSize);
    }
    _ce;
}
static void IRET(core_machine_cpu_execution_context *context) {
    _cb("IRET");
    i386(0xcf) {
        _adv;
        _chr(_e_iret(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_e_iret(context, 2));
    }
    _ce;
}
static void INS_D0(core_machine_cpu_execution_context *context) {
    _cb("INS_D0");
    i386(0xd0) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_read_rm(context, 1));
    switch (instruction_state.data.cr) {
    case 0: /* ROL_RM8 */
        _bb("ROL_RM8");
        _chr(_a_rol(context, instruction_state.data.crm, 1, 8));
        _be;
        break;
    case 1: /* ROR_RM8 */
        _bb("ROR_RM8");
        _chr(_a_ror(context, instruction_state.data.crm, 1, 8));
        _be;
        break;
    case 2: /* RCL_RM8 */
        _bb("RCL_RM8");
        _chr(_a_rcl(context, instruction_state.data.crm, 1, 8));
        _be;
        break;
    case 3: /* RCR_RM8 */
        _bb("RCR_RM8");
        _chr(_a_rcr(context, instruction_state.data.crm, 1, 8));
        _be;
        break;
    case 4: /* SHL_RM8 */
        _bb("SHL_RM8");
        _chr(_a_shl(context, instruction_state.data.crm, 1, 8));
        _be;
        break;
    case 5: /* SHR_RM8 */
        _bb("SHR_RM8");
        _chr(_a_shr(context, instruction_state.data.crm, 1, 8));
        _be;
        break;
    case 6: /* UndefinedOpcode */
        _bb("instruction_state.data.cr(6)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 7: /* SAR_RM8 */
        _bb("SAR_RM8");
        _chr(_a_sar(context, instruction_state.data.crm, 1, 8));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    instruction_state.data.crm = instruction_state.data.result;
    _chr(_m_write_rm(context, 1));
    _ce;
}
static void INS_D1(core_machine_cpu_execution_context *context) {
    _cb("INS_D1");
    i386(0xd1) {
        _adv;
        _chr(_d_modrm(context, 0, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        switch (instruction_state.data.cr) {
        case 0: /* ROL_RM32 */
            _bb("ROL_RM32");
            _chr(_a_rol(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            _be;
            break;
        case 1: /* ROR_RM32 */
            _bb("ROR_RM32");
            _chr(_a_ror(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            _be;
            break;
        case 2: /* RCL_RM32 */
            _bb("RCL_RM32");
            _chr(_a_rcl(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            _be;
            break;
        case 3: /* RCR_RM32 */
            _bb("RCR_RM32");
            _chr(_a_rcr(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            _be;
            break;
        case 4: /* SHL_RM32 */
            _bb("SHL_RM32");
            _chr(_a_shl(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            _be;
            break;
        case 5: /* SHR_RM32 */
            _bb("SHR_RM32");
            _chr(_a_shr(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            _be;
            break;
        case 6: /* UndefinedOpcode */
            _bb("instruction_state.data.cr(6)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 7: /* SAR_RM32 */
            _bb("SAR_RM32");
            _chr(_a_sar(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 0, 2));
        _chr(_m_read_rm(context, 2));
        switch (instruction_state.data.cr) {
        case 0: /* ROL_RM16 */
            _bb("ROL_RM16");
            _chr(_a_rol(context, instruction_state.data.crm, 1, 16));
            _be;
            break;
        case 1: /* ROR_RM16 */
            _bb("ROR_RM16");
            _chr(_a_ror(context, instruction_state.data.crm, 1, 16));
            _be;
            break;
        case 2: /* RCL_RM16 */
            _bb("RCL_RM16");
            _chr(_a_rcl(context, instruction_state.data.crm, 1, 16));
            _be;
            break;
        case 3: /* RCR_RM16 */
            _bb("RCR_RM16");
            _chr(_a_rcr(context, instruction_state.data.crm, 1, 16));
            _be;
            break;
        case 4: /* SHL_RM16 */
            _bb("SHL_RM16");
            _chr(_a_shl(context, instruction_state.data.crm, 1, 16));
            _be;
            break;
        case 5: /* SHR_RM16 */
            _bb("SHR_RM16");
            _chr(_a_shr(context, instruction_state.data.crm, 1, 16));
            _be;
            break;
        case 6: /* UndefinedOpcode */
            _bb("instruction_state.data.cr(6)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 7: /* SAR_RM16 */
            _bb("SAR_RM16");
            _chr(_a_sar(context, instruction_state.data.crm, 1, 16));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 2));
    }
    _ce;
}
static void INS_D2(core_machine_cpu_execution_context *context) {
    _cb("INS_D2");
    i386(0xd2) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_read_rm(context, 1));
    switch (instruction_state.data.cr) {
    case 0: /* ROL_RM8_CL */
        _bb("ROL_RM8_CL");
        _chr(_a_rol(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        _be;
        break;
    case 1: /* ROR_RM8_CL */
        _bb("ROR_RM8_CL");
        _chr(_a_ror(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        _be;
        break;
    case 2: /* RCL_RM8_CL */
        _bb("RCL_RM8_CL");
        _chr(_a_rcl(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        _be;
        break;
    case 3: /* RCR_RM8_CL */
        _bb("RCR_RM8_CL");
        _chr(_a_rcr(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        _be;
        break;
    case 4: /* SHL_RM8_CL */
        _bb("SHL_RM8_CL");
        _chr(_a_shl(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        _be;
        break;
    case 5: /* SHR_RM8_CL */
        _bb("SHR_RM8_CL");
        _chr(_a_shr(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        _be;
        break;
    case 6: /* UndefinedOpcode */
        _bb("instruction_state.data.cr(6)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 7: /* SAR_RM8_CL */
        _bb("SAR_RM8_CL");
        _chr(_a_sar(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    instruction_state.data.crm = instruction_state.data.result;
    _chr(_m_write_rm(context, 1));
    _ce;
}
static void INS_D3(core_machine_cpu_execution_context *context) {
    _cb("INS_D3");
    i386(0xd3) {
        _adv;
        _chr(_d_modrm(context, 0, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        switch (instruction_state.data.cr) {
        case 0: /* ROL_RM32_CL */
            _bb("ROL_RM32_CL");
            _chr(_a_rol(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            _be;
            break;
        case 1: /* ROR_RM32_CL */
            _bb("ROR_RM32_CL");
            _chr(_a_ror(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            _be;
            break;
        case 2: /* RCL_RM32_CL */
            _bb("RCL_RM32_CL");
            _chr(_a_rcl(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            _be;
            break;
        case 3: /* RCR_RM32_CL */
            _bb("RCR_RM32_CL");
            _chr(_a_rcr(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            _be;
            break;
        case 4: /* SHL_RM32_CL */
            _bb("SHL_RM32_CL");
            _chr(_a_shl(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            _be;
            break;
        case 5: /* SHR_RM32_CL */
            _bb("SHR_RM32_CL");
            _chr(_a_shr(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            _be;
            break;
        case 6: /* UndefinedOpcode */
            _bb("instruction_state.data.cr(6)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 7: /* SAR_RM32_CL */
            _bb("SAR_RM32_CL");
            _chr(_a_sar(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 0, 2));
        _chr(_m_read_rm(context, 2));
        switch (instruction_state.data.cr) {
        case 0: /* ROL_RM16_CL */
            _bb("ROL_RM16_CL");
            _chr(_a_rol(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            _be;
            break;
        case 1: /* ROR_RM16_CL */
            _bb("ROR_RM16_CL");
            _chr(_a_ror(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            _be;
            break;
        case 2: /* RCL_RM16_CL */
            _bb("RCL_RM16_CL");
            _chr(_a_rcl(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            _be;
            break;
        case 3: /* RCR_RM16_CL */
            _bb("RCR_RM16_CL");
            _chr(_a_rcr(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            _be;
            break;
        case 4: /* SHL_RM16_CL */
            _bb("SHL_RM16_CL");
            _chr(_a_shl(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            _be;
            break;
        case 5: /* SHR_RM16_CL */
            _bb("SHR_RM16_CL");
            _chr(_a_shr(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            _be;
            break;
        case 6: /* UndefinedOpcode */
            _bb("instruction_state.data.cr(6)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 7: /* SAR_RM16_CL */
            _bb("SAR_RM16_CL");
            _chr(_a_sar(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 2));
    }
    _ce;
}
static void AAM(core_machine_cpu_execution_context *context) {
    t_nubit8 base;
    _cb("AAM");
    i386(0xd4) {
        _adv;
        _chr(_d_imm(context, 1));
    }
    else {
        cpu_state.data.ip++;
        _d_imm(context, 1);
    }
    base = GetMax8(instruction_state.data.cimm);
    cpu_state.data.ah = cpu_state.data.al / base;
    cpu_state.data.al = cpu_state.data.al % base;
    instruction_state.data.bit = 0x08;
    instruction_state.data.result = GetMax8(cpu_state.data.al);
    _chr(_kaf_set_flags(context, AAM_FLAG));
    instruction_state.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF);
    _ce;
}
static void AAD(core_machine_cpu_execution_context *context) {
    t_nubit8 base;
    _cb("AAD");
    i386(0xd5) {
        _adv;
        _chr(_d_imm(context, 1));
    }
    else {
        cpu_state.data.ip++;
        _d_imm(context, 1);
    }
    base = GetMax8(instruction_state.data.cimm);
    cpu_state.data.al = GetMax8(cpu_state.data.al + (cpu_state.data.ah * base));
    cpu_state.data.ah = 0x00;
    instruction_state.data.bit = 0x08;
    instruction_state.data.result = GetMax8(cpu_state.data.al);
    _chr(_kaf_set_flags(context, AAD_FLAG));
    instruction_state.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF);
    _ce;
}
static void XLAT(core_machine_cpu_execution_context *context) {
    _cb("XLAT");
    i386(0xd7) {
        _adv;
        switch (_GetAddressSize) {
        case 2:
            _bb("AddressSize(2)");
            _chr(_m_read_logical(context, instruction_state.data.roverds, (cpu_state.data.bx + cpu_state.data.al), GetRef(cpu_state.data.al), 1));
            _be;
            break;
        case 4:
            _bb("AddressSize(4)");
            _chr(_m_read_logical(context, instruction_state.data.roverds, (cpu_state.data.ebx + cpu_state.data.al), GetRef(cpu_state.data.al), 1));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_m_read_logical(context, instruction_state.data.roverds, (cpu_state.data.bx + cpu_state.data.al), GetRef(cpu_state.data.al), 1));
    }
    _ce;
}
static void LOOPNZ_REL8(core_machine_cpu_execution_context *context) {
    _cb("LOOPNZ_REL8");
    i386(0xe0) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_loopcc(context, (t_nsbit8)instruction_state.data.cimm, !_GetEFLAGS_ZF));
    _ce;
}
static void LOOPZ_REL8(core_machine_cpu_execution_context *context) {
    _cb("LOOPZ_REL8");
    i386(0xe1) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_loopcc(context, (t_nsbit8)instruction_state.data.cimm, _GetEFLAGS_ZF));
    _ce;
}
static void LOOP_REL8(core_machine_cpu_execution_context *context) {
    _cb("LOOP_REL8");
    i386(0xe2) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_loopcc(context, (t_nsbit8)instruction_state.data.cimm, 1));
    _ce;
}
static void JCXZ_REL8(core_machine_cpu_execution_context *context) {
    t_nubit32 cecx = 0x00000000;
    _cb("JCXZ_REL8");
    i386(0xe3) {
        _adv;
        _chr(_d_imm(context, 1));
        switch (_GetAddressSize) {
        case 2:
            cecx = cpu_state.data.cx;
            break;
        case 4:
            cecx = cpu_state.data.ecx;
            break;
        default:
            _impossible_r_;
            break;
        }
        _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), 1, !cecx));
    }
    else {
        cpu_state.data.ip++;
        _d_imm(context, 1);
        _e_jcc(context, GetMax32(instruction_state.data.cimm), 1, !cpu_state.data.cx);
    }
    _ce;
}
static void IN_AL_I8(core_machine_cpu_execution_context *context) {
    _cb("IN_AL_I8");
    i386(0xe4) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_p_input(context, GetMax8(instruction_state.data.cimm), GetRef(cpu_state.data.al), 1));
    _ce;
}
static void IN_EAX_I8(core_machine_cpu_execution_context *context) {
    _cb("IN_EAX_I8");
    _new_code_path_;
    i386(0xe5) {
        _adv;
        _chr(_d_imm(context, 1));
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_p_input(context, GetMax8(instruction_state.data.cimm), GetRef(cpu_state.data.ax), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_p_input(context, GetMax8(instruction_state.data.cimm), GetRef(cpu_state.data.eax), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 1));
        _chr(_p_input(context, GetMax8(instruction_state.data.cimm), GetRef(cpu_state.data.ax), 2));
    }
    _ce;
}
static void OUT_I8_AL(core_machine_cpu_execution_context *context) {
    _cb("OUT_I8_AL");
    i386(0xe6) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_p_output(context, GetMax8(instruction_state.data.cimm), GetRef(cpu_state.data.al), 1));
    _ce;

}
static void OUT_I8_EAX(core_machine_cpu_execution_context *context) {
    _cb("OUT_I8_EAX");
    _new_code_path_;
    i386(0xe7) {
        _adv;
        _chr(_d_imm(context, 1));
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_p_output(context, GetMax8(instruction_state.data.cimm), GetRef(cpu_state.data.ax), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_p_output(context, GetMax8(instruction_state.data.cimm), GetRef(cpu_state.data.eax), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _d_imm(context, 1);
        _p_output(context, GetMax8(instruction_state.data.cimm), GetRef(cpu_state.data.ax), 2);
    }
    _ce;
}
static void CALL_REL32(core_machine_cpu_execution_context *context) {
    _cb("CALL_REL32");
    i386(0xe8) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(context, 2));
            _chr(_e_call_near(context, GetMax16(cpu_state.data.ip + (t_nsbit16)instruction_state.data.cimm), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            _chr(_d_imm(context, 4));
            _chr(_e_call_near(context, GetMax32(cpu_state.data.eip + (t_nsbit32)instruction_state.data.cimm), 4));
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
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        _chr(_e_call_near(context, GetMax16(cpu_state.data.ip + (t_nsbit16)instruction_state.data.cimm), 2));
    }
    _ce;
}
static void JMP_REL32(core_machine_cpu_execution_context *context) {
    _cb("JMP_REL32");
    i386(0xe9) {
        _adv;
        _chr(_d_imm(context, _GetOperandSize));
        _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), _GetOperandSize, 1));
    }
    else {
        cpu_state.data.ip++;
        _d_imm(context, 2);
        _e_jcc(context, GetMax32(instruction_state.data.cimm), 2, 1);
    }
    _ce;
}
static void JMP_PTR16_32(core_machine_cpu_execution_context *context) {
    t_nubit16 newcs = 0x0000;
    t_nubit32 neweip = 0x00000000;
    _cb("JMP_PTR16_32");
    i386(0xea) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_d_imm(context, 2));
            neweip = (t_nubit16)instruction_state.data.cimm;
            _chr(_d_imm(context, 2));
            newcs = (t_nubit16)instruction_state.data.cimm;
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _new_code_path_;
            _chr(_d_imm(context, 4));
            neweip = (t_nubit32)instruction_state.data.cimm;
            _chr(_d_imm(context, 2));
            newcs = (t_nubit16)instruction_state.data.cimm;
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
        _chr(_e_jmp_far(context, newcs, neweip, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_imm(context, 2));
        neweip = (t_nubit16)instruction_state.data.cimm;
        _chr(_d_imm(context, 2));
        newcs = (t_nubit16)instruction_state.data.cimm;
        _chr(_e_jmp_far(context, newcs, neweip, 2));
    }
    _ce;
}
static void JMP_REL8(core_machine_cpu_execution_context *context) {
    _cb("JMP_REL8");
    i386(0xeb) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_imm(context, 1));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), 1, 1));
    _ce;
}
static void IN_AL_DX(core_machine_cpu_execution_context *context) {
    _cb("IN_AL_DX");
    i386(0xec) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_p_input(context, cpu_state.data.dx, GetRef(cpu_state.data.al), 1));
    _ce;
}
static void IN_EAX_DX(core_machine_cpu_execution_context *context) {
    _cb("IN_EAX_DX");
    i386(0xed) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_p_input(context, cpu_state.data.dx, GetRef(cpu_state.data.ax), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_p_input(context, cpu_state.data.dx, GetRef(cpu_state.data.eax), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_p_input(context, cpu_state.data.dx, GetRef(cpu_state.data.ax), 2));
    }
    _ce;
}
static void OUT_DX_AL(core_machine_cpu_execution_context *context) {
    _cb("OUT_DX_AL");
    i386(0xee) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_p_output(context, cpu_state.data.dx, GetRef(cpu_state.data.al), 1));
    _ce;
}
static void OUT_DX_EAX(core_machine_cpu_execution_context *context) {
    _cb("OUT_DX_EAX");
    _new_code_path_;
    i386(0xef) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            _bb("OperandSize(2)");
            _chr(_p_output(context, cpu_state.data.dx, GetRef(cpu_state.data.ax), 2));
            _be;
            break;
        case 4:
            _bb("OperandSize(4)");
            _chr(_p_output(context, cpu_state.data.dx, GetRef(cpu_state.data.eax), 4));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _p_output(context, cpu_state.data.dx, GetRef(cpu_state.data.ax), 2);
    }
    _ce;
}
static void PREFIX_LOCK(core_machine_cpu_execution_context *context) {
    t_nubit8 opcode = 0x00;
    t_nubit8 modrm = 0x00;
    t_nubit8 opcode_0f = 0x00;
    t_nubit32 ceip = cpu_state.data.eip;
    _cb("PREFIX_LOCK");
    i386(0xf0) {
        _adv;
        do {
            _chr(_s_read_cs(context, ceip, GetRef(opcode), 1));
            ceip++;
        } while (_kdf_check_prefix(context, opcode));
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
            instruction_state.data.flagLock = True;
            break;
        case 0x80:
        case 0x81:
        case 0x83:
            _bb("opcode(0x80/0x81/0x83)");
            _chr(_s_read_cs(context, ceip, GetRef(modrm), 1));
            if (_GetModRM_REG(modrm) != 7)
                instruction_state.data.flagLock = True;
            else {
                _bb("ModRM_REG(7)");
                _chr(_SetExcept_UD(0));
                _be;
            }
            _be;
            break;
        case 0x0f:
            _bb("opcode(0f)");
            _chr(_s_read_cs(context, ceip, GetRef(opcode_0f), 1));
            switch (opcode_0f) {
            case 0xa3: /* BT */
            case 0xab: /* BTS */
            case 0xb3: /* BTR */
            case 0xbb: /* BTC */
            case 0xba:
                instruction_state.data.flagLock = True;
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
static void PREFIX_REPNZ(core_machine_cpu_execution_context *context) {
    _cb("PREFIX_REPNZ");
    i386(0xf2) {
        _adv;
        instruction_state.data.prefix_rep = PREFIX_REP_REPZNZ;
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.prefix_rep = PREFIX_REP_REPZNZ;
    }
    _ce;
}
static void PREFIX_REPZ(core_machine_cpu_execution_context *context) {
    _cb("PREFIX_REPZ");
    i386(0xf3) {
        _adv;
        instruction_state.data.prefix_rep = PREFIX_REP_REPZ;
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.prefix_rep = PREFIX_REP_REPZ;
    }
    _ce;
}
static void HLT(core_machine_cpu_execution_context *context) {
    _cb("HLT");
    _new_code_path_;
    if (_GetCR0_PE && _GetCPL) {
        _bb("CR0_PE(1),CPL(!0)");
        _SetExcept_GP(0);
        _be;
    }
    _adv;
    cpu_state.data.flagHalt = True;
    _ce;
}
static void CMC(core_machine_cpu_execution_context *context) {
    _cb("CMC");
    i386(0xf5) {
        _adv;
        cpu_state.data.eflags ^= VCPU_EFLAGS_CF;
    }
    else {
        cpu_state.data.ip++;
        cpu_state.data.eflags ^= VCPU_EFLAGS_CF;
    }
    _ce;
}
static void INS_F6(core_machine_cpu_execution_context *context) {
    _cb("INS_F6");
    i386(0xf6) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_read_rm(context, 1));
    switch (instruction_state.data.cr) {
    case 0: /* TEST_RM8_I8 */
        _bb("TEST_RM8_I8");
        _chr(_d_imm(context, 1));
        _chr(_a_test(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        _be;
        break;
    case 1: /* UndefinedOpcode */
        _bb("ModRM_REG(1)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 2: /* NOT_RM8 */
        _bb("NOT_RM8");
        _chr(_a_not(context, instruction_state.data.crm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 1));
        _be;
        break;
    case 3: /* NEG_RM8 */
        _bb("NEG_RM8");
        _chr(_a_neg(context, instruction_state.data.crm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 1));
        _be;
        break;
    case 4: /* MUL_RM8 */
        _bb("MUL_RM8");
        _chr(_a_mul(context, instruction_state.data.crm, 8));
        _be;
        break;
    case 5: /* IMUL_RM8 */
        _bb("IMUL_RM8");
        _chr(_a_imul(context, instruction_state.data.crm, 8));
        _be;
        break;
    case 6: /* DIV_RM8 */
        _bb("DIV_RM8");
        _chr(_a_div(context, instruction_state.data.crm, 8));
        _be;
        break;
    case 7: /* IDIV_RM8 */
        _bb("IDIV_RM8");
        _new_code_path_;
        _chr(_a_idiv(context, instruction_state.data.crm, 8));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void INS_F7(core_machine_cpu_execution_context *context) {
    _cb("INS_F7");
    i386(0xf7) {
        _adv;
        _chr(_d_modrm(context, 0, _GetOperandSize));
        _chr(_m_read_rm(context, _GetOperandSize));
        switch (instruction_state.data.cr) {
        case 0: /* TEST_RM32_I32 */
            _bb("TEST_RM32_I32");
            _chr(_d_imm(context, _GetOperandSize));
            _chr(_a_test(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            _be;
            break;
        case 1: /* UndefinedOpcode */
            _bb("ModRM_REG(1)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 2: /* NOT_RM32 */
            _bb("NOT_RM32");
            _chr(_a_not(context, instruction_state.data.crm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 3: /* NEG_RM32 */
            _bb("NEG_RM32");
            _chr(_a_neg(context, instruction_state.data.crm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 4: /* MUL_RM32 */
            _bb("MUL_RM32");
            _chr(_a_mul(context, instruction_state.data.crm, _GetOperandSize * 8));
            _be;
            break;
        case 5: /* IMUL_RM32 */
            _bb("IMUL_RM32");
            _chr(_a_imul(context, instruction_state.data.crm, _GetOperandSize * 8));
            _be;
            break;
        case 6: /* DIV_RM32 */
            _bb("DIV_RM32");
            _chr(_a_div(context, instruction_state.data.crm, _GetOperandSize * 8));
            _be;
            break;
        case 7: /* IDIV_RM32 */
            _bb("IDIV_RM32");
            _chr(_a_idiv(context, instruction_state.data.crm, _GetOperandSize * 8));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_d_modrm(context, 0, 2));
        _chr(_m_read_rm(context, 2));
        switch (instruction_state.data.cr) {
        case 0: /* TEST_RM16_I16 */
            _bb("TEST_RM16_I16");
            _chr(_d_imm(context, 2));
            _chr(_a_test(context, instruction_state.data.crm, instruction_state.data.cimm, 16));
            _be;
            break;
        case 1: /* UndefinedOpcode */
            _bb("ModRM_REG(1)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 2: /* NOT_RM16 */
            _bb("NOT_RM16");
            _chr(_a_not(context, instruction_state.data.crm, 16));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 3: /* NEG_RM16 */
            _bb("NEG_RM16");
            _chr(_a_neg(context, instruction_state.data.crm, 16));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 4: /* MUL_RM16 */
            _bb("MUL_RM16");
            _chr(_a_mul(context, instruction_state.data.crm, 16));
            _be;
            break;
        case 5: /* IMUL_RM16 */
            _bb("IMUL_RM16");
            _chr(_a_imul(context, instruction_state.data.crm, 16));
            _be;
            break;
        case 6: /* DIV_RM16 */
            _bb("DIV_RM16");
            _chr(_a_div(context, instruction_state.data.crm, 16));
            _be;
            break;
        case 7: /* IDIV_RM16 */
            _bb("IDIV_RM16");
            _chr(_a_idiv(context, instruction_state.data.crm, 16));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    _ce;
}
static void CLC(core_machine_cpu_execution_context *context) {
    _cb("CLC");
    i386(0xf8) {
        _adv;
        _ClrEFLAGS_CF;
    }
    else {
        cpu_state.data.ip++;
        _ClrEFLAGS_CF;
    }
    _ce;
}
static void STC(core_machine_cpu_execution_context *context) {
    _cb("STC");
    i386(0xf9) {
        _adv;
        _SetEFLAGS_CF;
    }
    else {
        cpu_state.data.ip++;
        _SetEFLAGS_CF;
    }
    _ce;
}
static void CLI(core_machine_cpu_execution_context *context) {
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
        cpu_state.data.ip++;
        _ClrEFLAGS_IF;
    }
    _ce;
}
static void STI(core_machine_cpu_execution_context *context) {
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
        instruction_state.data.flagMaskInt = True;
    }
    else {
        cpu_state.data.ip++;
        _SetEFLAGS_IF;
    }
    _ce;
}
static void CLD(core_machine_cpu_execution_context *context) {
    _cb("CLD");
    i386(0xfc) {
        _adv;
        _ClrEFLAGS_DF;
    }
    else {
        cpu_state.data.ip++;
        _ClrEFLAGS_DF;
    }
    _ce;
}
static void STD(core_machine_cpu_execution_context *context) {
    _cb("CLD");
    i386(0xfd) {
        _adv;
        _SetEFLAGS_DF;
    }
    else {
        cpu_state.data.ip++;
        _SetEFLAGS_DF;
    }
    _ce;
}
static void INS_FE(core_machine_cpu_execution_context *context) {
    _cb("INS_FE");
    i386(0xfe) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_read_rm(context, 1));
    switch (instruction_state.data.cr) {
    case 0: /* INC_RM8 */
        _bb("INC_RM8");
        _chr(_a_inc(context, instruction_state.data.crm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 1));
        _be;
        break;
    case 1: /* DEC_RM8 */
        _bb("DEC_RM8");
        _chr(_a_dec(context, instruction_state.data.crm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, 1));
        _be;
        break;
    case 2:
        _bb("instruction_state.data.cr(2)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 3:
        _bb("instruction_state.data.cr(3)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 4:
        _bb("instruction_state.data.cr(4)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 5:
        _bb("instruction_state.data.cr(5)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 6:
        _bb("instruction_state.data.cr(6)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 7:
        _bb("instruction_state.data.cr(7)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void INS_FF(core_machine_cpu_execution_context *context) {
    t_nubit8 modrm;
    t_nubit16 newcs;
    t_nubit32 neweip;
    _cb("INS_FF");
    i386(0xff) {
        _adv;
        _chr(_s_read_cs(context, cpu_state.data.eip, GetRef(modrm), 1));
        switch (_GetModRM_REG(modrm)) {
        case 0: /* INC_RM32 */
            _bb("INC_RM32");
            _chr(_d_modrm(context, 0, _GetOperandSize));
            _chr(_m_read_rm(context, _GetOperandSize));
            _chr(_a_inc(context, instruction_state.data.crm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 1: /* DEC_RM32 */
            _bb("DEC_RM32");
            _chr(_d_modrm(context, 0, _GetOperandSize));
            _chr(_m_read_rm(context, _GetOperandSize));
            _chr(_a_dec(context, instruction_state.data.crm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, _GetOperandSize));
            _be;
            break;
        case 2: /* CALL_RM32 */
            _bb("CALL_RM32");
            _chr(_d_modrm(context, 0, _GetOperandSize));
            _chr(_m_read_rm(context, _GetOperandSize));
            neweip = GetMax32(instruction_state.data.crm);
            _chr(_e_call_near(context, neweip, _GetOperandSize));
            _be;
            break;
        case 3: /* CALL_M16_32 */
            _bb("CALL_M16_32");
            _chr(_d_modrm(context, 0, _GetOperandSize + 2));
            if (!instruction_state.data.flagMem) {
                _bb("flagMem(0)");
                _chr(UndefinedOpcode(context));
                _be;
            }
            _chr(_m_read_rm(context, _GetOperandSize));
            neweip = (t_nubit32)instruction_state.data.crm;
            instruction_state.data.mrm.offset += _GetOperandSize;
            _chr(_m_read_rm(context, 2));
            newcs = GetMax16(instruction_state.data.crm);
            _chr(_e_call_far(context, newcs, neweip, _GetOperandSize));
            _be;
            break;
        case 4: /* JMP_RM32 */
            _bb("JMP_RM32");
            _chr(_d_modrm(context, 0, _GetOperandSize));
            _chr(_m_read_rm(context, _GetOperandSize));
            neweip = GetMax32(instruction_state.data.crm);
            _chr(_e_jmp_near(context, neweip, _GetOperandSize));
            _be;
            break;
        case 5: /* JMP_M16_32 */
            _bb("JMP_M16_32");
            _chr(_d_modrm(context, 0, _GetOperandSize + 2));
            if (!instruction_state.data.flagMem) {
                _bb("flagMem(0)");
                _chr(UndefinedOpcode(context));
                _be;
            }
            _chr(_m_read_rm(context, _GetOperandSize));
            neweip = (t_nubit32)instruction_state.data.crm;
            instruction_state.data.mrm.offset += _GetOperandSize;
            _chr(_m_read_rm(context, 2));
            newcs = GetMax16(instruction_state.data.crm);
            _chr(_e_jmp_far(context, newcs, neweip, _GetOperandSize));
            _be;
            break;
        case 6: /* PUSH_RM32 */
            _bb("PUSH_RM32");
            _chr(_d_modrm(context, 0, _GetOperandSize));
            _chr(_m_read_rm(context, _GetOperandSize));
            _chr(_e_push(context, GetRef(instruction_state.data.crm), _GetOperandSize));
            _be;
            break;
        case 7: /* UndefinedOpcode */
            _bb("ModRM_REG(7)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _chr(_s_read_cs(context, cpu_state.data.eip, GetRef(modrm), 1));
        switch (_GetModRM_REG(modrm)) {
        case 0: /* INC_RM16 */
            _bb("INC_RM16");
            _chr(_d_modrm(context, 0, 2));
            _chr(_m_read_rm(context, 2));
            _chr(_a_inc(context, instruction_state.data.crm, 16));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 1: /* DEC_RM16 */
            _bb("DEC_RM16");
            _chr(_d_modrm(context, 0, 2));
            _chr(_m_read_rm(context, 2));
            _chr(_a_dec(context, instruction_state.data.crm, 16));
            instruction_state.data.crm = instruction_state.data.result;
            _chr(_m_write_rm(context, 2));
            _be;
            break;
        case 2: /* CALL_RM16 */
            _bb("CALL_RM16");
            _chr(_d_modrm(context, 0, 2));
            _chr(_m_read_rm(context, 2));
            neweip = GetMax16(instruction_state.data.crm);
            _chr(_e_call_near(context, neweip, 2));
            _be;
            break;
        case 3: /* CALL_M16_16 */
            _bb("CALL_M16_16");
            _chr(_d_modrm(context, 0, 4));
            if (!instruction_state.data.flagMem) {
                _bb("flagMem(0)");
                _chr(UndefinedOpcode(context));
                _be;
            }
            _chr(_m_read_rm(context, 2));
            neweip = (t_nubit16)instruction_state.data.crm;
            instruction_state.data.mrm.offset += 2;
            _chr(_m_read_rm(context, 2));
            newcs = GetMax16(instruction_state.data.crm);
            _chr(_e_call_far(context, newcs, neweip, 2));
            _be;
            break;
        case 4: /* JMP_RM16 */
            _bb("JMP_RM16");
            _chr(_d_modrm(context, 0, 2));
            _chr(_m_read_rm(context, 2));
            neweip = GetMax16(instruction_state.data.crm);
            _chr(_e_jmp_near(context, neweip, 2));
            _be;
            break;
        case 5: /* JMP_M16_16 */
            _bb("JMP_M16_16");
            _chr(_d_modrm(context, 0, 4));
            if (!instruction_state.data.flagMem) {
                _bb("flagMem(0)");
                _chr(UndefinedOpcode(context));
                _be;
            }
            _chr(_m_read_rm(context, 2));
            neweip = (t_nubit16)instruction_state.data.crm;
            instruction_state.data.mrm.offset += 2;
            _chr(_m_read_rm(context, 2));
            newcs = GetMax16(instruction_state.data.crm);
            _chr(_e_jmp_far(context, newcs, neweip, 2));
            _be;
            break;
        case 6: /* PUSH_RM16 */
            _bb("PUSH_RM16");
            _chr(_d_modrm(context, 0, 2));
            _chr(_m_read_rm(context, 2));
            _chr(_e_push(context, GetRef(instruction_state.data.crm), 2));
            _be;
            break;
        case 7: /* UndefinedOpcode */
            _bb("ModRM_REG(7)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
    }
    _ce;
}

static void _d_modrm_creg(core_machine_cpu_execution_context *context) {
    _cb("_d_modrm_creg");
    _chr(_kdf_modrm(context, 0, 4));
    if (instruction_state.data.flagMem) {
        _bb("flagMem(1)");
        _chr(_SetExcept_UD(0));
        _be;
    }
    switch (instruction_state.data.cr) {
    case 0:
        instruction_state.data.rr = (t_vaddrcc)&cpu_state.data.cr0;
        instruction_state.data.cr = cpu_state.data.cr0;
        break;
    case 1:
        _bb("instruction_state.data.cr(1)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 2:
        instruction_state.data.rr = (t_vaddrcc)&cpu_state.data.cr2;
        instruction_state.data.cr = cpu_state.data.cr2;
        break;
    case 3:
        instruction_state.data.rr = (t_vaddrcc)&cpu_state.data.cr3;
        instruction_state.data.cr = cpu_state.data.cr3;
        break;
    case 4:
        _bb("instruction_state.data.cr(4)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 5:
        _bb("instruction_state.data.cr(5)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 6:
        _bb("instruction_state.data.cr(6)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 7:
        _bb("instruction_state.data.cr(7)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void _d_modrm_dreg(core_machine_cpu_execution_context *context) {
    _cb("_d_modrm_dreg");
    _chr(_kdf_modrm(context, 0, 4));
    if (instruction_state.data.flagMem) {
        _bb("flagMem(1)");
        _chr(_SetExcept_UD(0));
        _be;
    }
    switch (instruction_state.data.cr) {
    case 0:
        instruction_state.data.rr = (t_vaddrcc)&cpu_state.data.dr0;
        instruction_state.data.cr = cpu_state.data.dr0;
        break;
    case 1:
        instruction_state.data.rr = (t_vaddrcc)&cpu_state.data.dr1;
        instruction_state.data.cr = cpu_state.data.dr1;
        break;
    case 2:
        instruction_state.data.rr = (t_vaddrcc)&cpu_state.data.dr2;
        instruction_state.data.cr = cpu_state.data.dr2;
        break;
    case 3:
        instruction_state.data.rr = (t_vaddrcc)&cpu_state.data.dr3;
        instruction_state.data.cr = cpu_state.data.dr3;
        break;
    case 4:
        _bb("instruction_state.data.cr(4)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 5:
        _bb("instruction_state.data.cr(5)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 6:
        instruction_state.data.rr = (t_vaddrcc)&cpu_state.data.dr6;
        instruction_state.data.cr = cpu_state.data.dr6;
        break;
    case 7:
        instruction_state.data.rr = (t_vaddrcc)&cpu_state.data.dr7;
        instruction_state.data.cr = cpu_state.data.dr7;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void _d_modrm_treg(core_machine_cpu_execution_context *context) {
    _cb("_d_modrm_treg");
    _chr(_kdf_modrm(context, 0, 4));
    if (instruction_state.data.flagMem) {
        _bb("flagMem(1)");
        _chr(_SetExcept_UD(0));
        _be;
    }
    switch (instruction_state.data.cr) {
    case 0:
        _bb("instruction_state.data.cr(0)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 1:
        _bb("instruction_state.data.cr(1)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 2:
        _bb("instruction_state.data.cr(2)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 3:
        _bb("instruction_state.data.cr(3)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 4:
        _bb("instruction_state.data.cr(4)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 5:
        _bb("instruction_state.data.cr(5)");
        _chr(_SetExcept_UD(0));
        _be;
        break;
    case 6:
        instruction_state.data.rr = (t_vaddrcc)&cpu_state.data.tr6;
        instruction_state.data.cr = cpu_state.data.tr6;
        break;
    case 7:
        instruction_state.data.rr = (t_vaddrcc)&cpu_state.data.tr7;
        instruction_state.data.cr = cpu_state.data.tr7;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void _d_bit_rmimm(core_machine_cpu_execution_context *context, t_nubit8 regbyte, t_nubit8 rmbyte, t_bool write) {
    /* xrm = actual destination, cimm = (1 << bitoffset) */
    t_nsbit16 bitoff16 = 0;
    t_nsbit32 bitoff32 = 0;
    t_nubit32 bitoperand = 0;
    _cb("_d_bit_rmimm");
    _chr(_kdf_modrm(context, regbyte, rmbyte));
    if (!regbyte) _chr(_d_imm(context, 1));
    switch (rmbyte) {
    case 2:
        _bb("rmbyte(2)");
        if (instruction_state.data.flagMem && regbyte) {
            /* valid for btcc_m16_r16 */
            _bb("flagMem(1),regbyte(1)");
            bitoff16 = (t_nsbit16)instruction_state.data.cr;
            if (bitoff16 >= 0)
                instruction_state.data.mrm.offset += 2 * (bitoff16 / 16);
            else
                instruction_state.data.mrm.offset += 2 * ((bitoff16 - 15) / 16);
            bitoperand = ((t_nubit16)bitoff16) % 16;
            _be;
        } else if (regbyte) {
            bitoperand = instruction_state.data.cr % 16;
        } else {
            bitoperand = (GetMax16(instruction_state.data.cimm) % 16);
        }
        _chr(instruction_state.data.cimm = GetMax16((1 << bitoperand)));
        _be;
        break;
    case 4:
        _bb("rmbyte(4)");
        if (instruction_state.data.flagMem && regbyte) {
            _bb("flagMem(1),regbyte(1)");
            bitoff32 = (t_nsbit32)instruction_state.data.cr;
            if (bitoff32 >= 0)
                instruction_state.data.mrm.offset += 4 * (bitoff32 / 32);
            else
                instruction_state.data.mrm.offset += 4 * ((bitoff32 - 31) / 32);
            bitoperand = ((t_nubit32)bitoff32) % 32;
            _be;
        } else if (regbyte) {
            bitoperand = instruction_state.data.cr % 32;
        } else {
            bitoperand = (GetMax32(instruction_state.data.cimm) % 32);
        }
        _chr(instruction_state.data.cimm = GetMax32((1 << bitoperand)));
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
    case 16: instruction_state.data.opr1 = GetMax16(cdest);break; \
    case 32: instruction_state.data.opr1 = GetMax32(cdest);break; \
    default: _bb("bit"); \
        _chr(_SetExcept_CE(bit)); \
        _be;break; \
    } \
    instruction_state.data.opr2 = bitoperand; \
    instruction_state.data.result = instruction_state.data.opr1; \
    MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF, GetBit(instruction_state.data.opr1, instruction_state.data.opr2)); \
} while (0)

static void _a_bscc(core_machine_cpu_execution_context *context, t_nubit64 csrc, t_nubit8 bit, t_bool forward) {
    t_nubit32 temp;
    _cb("_a_bscc");
    if (forward) temp = 0;
    else temp = bit - 1;
    switch (bit) {
    case 16:
        _bb("bit(16)");
        instruction_state.data.opr1 = GetMax16(csrc);
        if (!instruction_state.data.opr1)
            _SetEFLAGS_ZF;
        else {
            _ClrEFLAGS_ZF;
            while (!GetBit(instruction_state.data.opr1, GetMax64(1 << temp))) {
                if (forward) temp++;
                else temp--;
            }
            instruction_state.data.result = GetMax16(temp);
        }
        _be;
        break;
    case 32:
        _bb("bit(32)");
        instruction_state.data.opr1 = GetMax32(csrc);
        if (!instruction_state.data.opr1)
            _SetEFLAGS_ZF;
        else {
            _ClrEFLAGS_ZF;
            while (!GetBit(instruction_state.data.opr1, GetMax64(1 << temp))) {
                if (forward) temp++;
                else temp--;
            }
            instruction_state.data.result = GetMax32(temp);
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
static void _a_bt(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit32 bitoperand, t_nubit8 bit) {
    _cb("_a_bt");
    _kac_btcc;
    _ce;
}
static void _a_btc(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit32 bitoperand, t_nubit8 bit) {
    _cb("_a_btc");
    _kac_btcc;
    MakeBit(instruction_state.data.result, instruction_state.data.opr2, !_GetEFLAGS_CF);
    _ce;
}
static void _a_btr(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit32 bitoperand, t_nubit8 bit) {
    _cb("_a_btr");
    _kac_btcc;
    ClrBit(instruction_state.data.result, instruction_state.data.opr2);
    _ce;
}
static void _a_bts(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit32 bitoperand, t_nubit8 bit) {
    _cb("_a_bts");
    _kac_btcc;
    SetBit(instruction_state.data.result, instruction_state.data.opr2);
    _ce;
}

static void _a_imul2(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit64 csrc, t_nubit8 bit) {
    t_nubit64 temp;
    _cb("_a_imul2");
    switch (bit) {
    case 16:
        _bb("bit(16+16)");
        _new_code_path_;
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = GetMax16((t_nsbit16)cdest);
        instruction_state.data.opr2 = GetMax16((t_nsbit16)csrc);
        temp = GetMax32((t_nsbit16)instruction_state.data.opr1 * (t_nsbit16)instruction_state.data.opr2);
        instruction_state.data.result = GetMax16(temp);
        if (GetMax32(temp) != GetMax32((t_nsbit16)instruction_state.data.result)) {
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
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = GetMax32((t_nsbit32)cdest);
        instruction_state.data.opr2 = GetMax32((t_nsbit32)csrc);
        temp = GetMax64((t_nsbit32)instruction_state.data.opr1 * (t_nsbit32)instruction_state.data.opr2);
        instruction_state.data.result = GetMax32(temp);
        if (GetMax64(temp) != GetMax64((t_nsbit32)instruction_state.data.result)) {
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
static void _a_shld(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit64 csrc, t_nubit8 count, t_nubit8 bit) {
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
        instruction_state.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF |
                             VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF);
    } else {
        switch (bit) {
        case 16:
            _bb("bit(16)");
            instruction_state.data.bit = 16;
            instruction_state.data.opr1 = GetMax16(cdest);
            instruction_state.data.opr2 = GetMax16(csrc);
            instruction_state.data.result = instruction_state.data.opr1;
            flagcf = !!GetMSB16(instruction_state.data.result);
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                    GetBit(instruction_state.data.result, GetMax64(1 << (bit - count))));
            for (i = (t_nsbit32)(bit - 1); i >= (t_nsbit32)count; --i) {
                flagbit = GetBit(instruction_state.data.opr1, GetMax64(1 << (i - count)));
                MakeBit(instruction_state.data.result, GetMax64(1 << i), flagbit);
            }
            for (i = (t_nsbit32)(count - 1); i >= 0; --i) {
                flagbit = GetBit(instruction_state.data.opr2, GetMax64(1 << (i - count + bit)));
                MakeBit(instruction_state.data.result, GetMax64(1 << i), flagbit);
            }
            if (count == 1)
                MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                        ((!!GetMSB16(instruction_state.data.result)) ^ flagcf));
            else
                instruction_state.data.udf |= VCPU_EFLAGS_OF;
            _be;
            break;
        case 32:
            _bb("bit(32)");
            instruction_state.data.bit = 32;
            instruction_state.data.opr1 = GetMax32(cdest);
            instruction_state.data.opr2 = GetMax32(csrc);
            instruction_state.data.result = instruction_state.data.opr1;
            flagcf = !!GetMSB32(instruction_state.data.result);
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                    GetBit(instruction_state.data.result, GetMax64(1 << (bit - count))));
            for (i = (t_nsbit32)(bit - 1); i >= (t_nsbit32)count; --i) {
                flagbit = GetBit(instruction_state.data.opr1, GetMax64(1 << (i - count)));
                MakeBit(instruction_state.data.result, GetMax64(1 << i), flagbit);
            }
            for (i = (t_nsbit32)(count - 1); i >= 0; --i) {
                flagbit = GetBit(instruction_state.data.opr2, GetMax64(1 << (i - count + bit)));
                MakeBit(instruction_state.data.result, GetMax64(1 << i), flagbit);
            }
            if (count == 1)
                MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                        ((!!GetMSB32(instruction_state.data.result)) ^ flagcf));
            else
                instruction_state.data.udf |= VCPU_EFLAGS_OF;
            _be;
            break;
        default:
            _bb("bit");
            _chr(_SetExcept_CE(bit));
            _be;
            break;
        }
        _chr(_kaf_set_flags(context, SHLD_FLAG));
        instruction_state.data.udf |= VCPU_EFLAGS_AF;
    }
    _ce;
}
static void _a_shrd(core_machine_cpu_execution_context *context, t_nubit64 cdest, t_nubit64 csrc, t_nubit8 count, t_nubit8 bit) {
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
        instruction_state.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF |
                             VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF);
    } else {
        switch (bit) {
        case 16:
            _bb("bit(16)");
            instruction_state.data.bit = 16;
            instruction_state.data.opr1 = GetMax16(cdest);
            instruction_state.data.opr2 = GetMax16(csrc);
            instruction_state.data.result = instruction_state.data.opr1;
            flagcf = !!GetMSB16(instruction_state.data.result);
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                    GetBit(instruction_state.data.result, GetMax64(1 << (count - 1))));
            for (i = 0; i <= (t_nsbit32)(bit - count - 1); ++i) {
                flagbit = GetBit(instruction_state.data.opr1, GetMax64(1 << (i + count)));
                MakeBit(instruction_state.data.result, GetMax64(1 << i), flagbit);
            }
            for (i = (t_nsbit32)(bit - count); i <= (t_nsbit32)(bit - 1); ++i) {
                flagbit = GetBit(instruction_state.data.opr2, GetMax64(1 << (i + count - bit)));
                MakeBit(instruction_state.data.result, GetMax64(1 << i), flagbit);
            }
            if (count == 1)
                MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                        ((!!GetMSB16(instruction_state.data.result)) ^ flagcf));
            else
                instruction_state.data.udf |= VCPU_EFLAGS_OF;
            _be;
            break;
        case 32:
            _bb("bit(32)");
            instruction_state.data.bit = 32;
            instruction_state.data.opr1 = GetMax32(cdest);
            instruction_state.data.opr2 = GetMax32(csrc);
            instruction_state.data.result = instruction_state.data.opr1;
            flagcf = !!GetMSB32(instruction_state.data.result);
            MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                    GetBit(instruction_state.data.result, GetMax64(1 << (count - 1))));
            for (i = 0; i <= (t_nsbit32)(bit - count - 1); ++i) {
                flagbit = GetBit(instruction_state.data.opr1, GetMax64(1 << (i + count)));
                MakeBit(instruction_state.data.result, GetMax64(1 << i), flagbit);
            }
            for (i = (t_nsbit32)(bit - count); i <= (t_nsbit32)(bit - 1); ++i) {
                flagbit = GetBit(instruction_state.data.opr2, GetMax64(1 << (i + count - bit)));
                MakeBit(instruction_state.data.result, GetMax64(1 << i), flagbit);
            }
            if (count == 1)
                MakeBit(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                        ((!!GetMSB32(instruction_state.data.result)) ^ flagcf));
            else
                instruction_state.data.udf |= VCPU_EFLAGS_OF;
            _be;
            break;
        default:
            _bb("bit");
            _chr(_SetExcept_CE(bit));
            _be;
            break;
        }
        _chr(_kaf_set_flags(context, SHRD_FLAG));
        instruction_state.data.udf |= VCPU_EFLAGS_AF;
    }
    _ce;
}

static void _m_setcc_rm(core_machine_cpu_execution_context *context, t_bool condition) {
    _cb("_m_setcc_rm");
    instruction_state.data.result = GetMax8(!!condition);
    instruction_state.data.crm = instruction_state.data.result;
    _chr(_m_write_rm(context, 1));
    _ce;
}

static void INS_0F_00(core_machine_cpu_execution_context *context) {
    t_nubit8 modrm;
    t_nubit64 descriptor;
    _cb("INS_0F_00");
    _adv;
    if (_IsProtected) {
        _bb("Protected");
        _chr(_s_read_cs(context, cpu_state.data.eip, GetRef(modrm), 1));
        switch (_GetModRM_REG(modrm)) {
        case 0: /* SLDT_RM16 */
            _bb("SLDT_RM16");
            _chr(_d_modrm(context, 0, ((_GetModRM_MOD(modrm) != 3) ? 2 : _GetOperandSize)));
            instruction_state.data.crm = GetMax16(cpu_state.data.ldtr.selector);
            _chr(_m_write_rm(context, instruction_state.data.flagMem ? 2 : _GetOperandSize));
            _be;
            break;
        case 1: /* STR_RM16 */
            _bb("STR_RM16");
            _chr(_d_modrm(context, 0, ((_GetModRM_MOD(modrm) != 3) ? 2 : _GetOperandSize)));
            instruction_state.data.crm = GetMax16(cpu_state.data.tr.selector);
            _chr(_m_write_rm(context, instruction_state.data.flagMem ? 2 : _GetOperandSize));
            _be;
            break;
        case 2: /* LLDT_RM16 */
            _bb("LLDT_RM16");
            _chr(_d_modrm(context, 0, 2));
            _chr(_m_read_rm(context, 2));
            _chr(_s_load_ldtr(context, GetMax16(instruction_state.data.crm)));
            _be;
            break;
        case 3: /* LTR_RM16 */
            _bb("LTR_RM16");
            _chr(_d_modrm(context, 0, 2));
            _chr(_m_read_rm(context, 2));
            _chr(_s_load_tr(context, GetMax16(instruction_state.data.crm)));
            _be;
            break;
        case 4: /* VERR_RM16 */
            _bb("VERR_RM16");
            _chr(_d_modrm(context, 0, 2));
            _chr(_m_read_rm(context, 2));
            if (_s_check_selector(context, GetMax16(instruction_state.data.crm))) {
                _ClrEFLAGS_ZF;
            } else {
                _bb("selector(valid)");
                _chr(_s_read_xdt(context, GetMax16(instruction_state.data.crm), GetRef(descriptor)));
                if (_IsDescSys(descriptor) ||
                        (!_IsDescCodeConform(descriptor) &&
                         (_GetCPL > _GetDesc_DPL(descriptor) ||
                          _GetSelector_RPL(GetMax16(instruction_state.data.crm)) > _GetDesc_DPL(descriptor)))) {
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
            _chr(_d_modrm(context, 0, 2));
            _chr(_m_read_rm(context, 2));
            if (_s_check_selector(context, GetMax16(instruction_state.data.crm))) {
                _ClrEFLAGS_ZF;
            } else {
                _bb("selector(valid)");
                _chr(_s_read_xdt(context, GetMax16(instruction_state.data.crm), GetRef(descriptor)));
                if (_IsDescSys(descriptor) ||
                        (!_IsDescCodeConform(descriptor) &&
                         (_GetCPL > _GetDesc_DPL(descriptor) ||
                          _GetSelector_RPL(GetMax16(instruction_state.data.crm)) > _GetDesc_DPL(descriptor)))) {
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
            _chr(UndefinedOpcode(context));
            _be;
            break;
        case 7:
            _bb("ModRM_REG(7)");
            _chr(UndefinedOpcode(context));
            _be;
            break;
        default:
            _impossible_r_;
            break;
        }
        _be;
    } else {
        _bb("!Protected");
        _chr(UndefinedOpcode(context));
        _be;
    }
    _ce;
}
static void INS_0F_01(core_machine_cpu_execution_context *context) {
    t_nubit8 modrm;
    t_nubit16 limit;
    t_nubit32 base;
    _cb("INS_0F_01");
    _adv;
    _chr(_s_read_cs(context, cpu_state.data.eip, GetRef(modrm), 1));
    switch (_GetModRM_REG(modrm)) {
    case 0: /* SGDT_M32_16 */
        _bb("SGDT_M32_16");
        _chr(_d_modrm(context, 0, 6));
        if (!instruction_state.data.flagMem) {
            _bb("flagMem(0)");
            _chr(UndefinedOpcode(context));
            _be;
        }
        instruction_state.data.crm = cpu_state.data.gdtr.limit;
        _chr(_m_write_rm(context, 2));
        instruction_state.data.mrm.offset += 2;
        switch (_GetOperandSize) {
        case 2:
            instruction_state.data.crm = GetMax24(cpu_state.data.gdtr.base);
            break;
        case 4:
            instruction_state.data.crm = GetMax32(cpu_state.data.gdtr.base);
            break;
        default:
            _impossible_r_;
            break;
        }
        _chr(_m_write_rm(context, 4));
        _be;
        break;
    case 1: /* SIDT_M32_16 */
        _bb("SIDT_M32_16");
        _chr(_d_modrm(context, 0, 6));
        if (!instruction_state.data.flagMem) {
            _bb("flagMem(0)");
            _chr(UndefinedOpcode(context));
            _be;
        }
        instruction_state.data.crm = cpu_state.data.idtr.limit;
        _chr(_m_write_rm(context, 2));
        instruction_state.data.mrm.offset += 2;
        switch (_GetOperandSize) {
        case 2:
            instruction_state.data.crm = GetMax24(cpu_state.data.idtr.base);
            break;
        case 4:
            instruction_state.data.crm = GetMax32(cpu_state.data.idtr.base);
            break;
        default:
            _impossible_r_;
            break;
        }
        _chr(_m_write_rm(context, 4));
        _be;
        break;
    case 2: /* LGDT_M32_16 */
        _bb("LGDT_M32_16");
        _chr(_d_modrm(context, 0, 6));
        if (!instruction_state.data.flagMem) {
            _bb("flagMem(0)");
            _chr(UndefinedOpcode(context));
            _be;
        }
        _chr(_m_read_rm(context, 2));
        limit = GetMax16(instruction_state.data.crm);
        instruction_state.data.mrm.offset += 2;
        _chr(_m_read_rm(context, 4));
        switch (_GetOperandSize) {
        case 2:
            base = GetMax24(instruction_state.data.crm);
            break;
        case 4:
            base = GetMax32(instruction_state.data.crm);
            break;
        default:
            _impossible_r_;
            break;
        }
        /* PRINTF("LGDT_M32_16: executed at L%08X, read base=%08X, limit=%04X\n",
                instruction_state.data.linear, base, limit); */
        _chr(_s_load_gdtr(context, base, limit, _GetOperandSize));
        _be;
        break;
    case 3: /* LIDT_M32_16 */
        _bb("LIDT_M32_16");
        _chr(_d_modrm(context, 0, 6));
        if (!instruction_state.data.flagMem) {
            _bb("flagMem(0)");
            _chr(UndefinedOpcode(context));
            _be;
        }
        _chr(_m_read_rm(context, 2));
        limit = GetMax16(instruction_state.data.crm);
        instruction_state.data.mrm.offset += 2;
        _chr(_m_read_rm(context, 4));
        switch (_GetOperandSize) {
        case 2:
            base = GetMax24(instruction_state.data.crm);
            break;
        case 4:
            base = GetMax32(instruction_state.data.crm);
            break;
        default:
            _impossible_r_;
            break;
        }
        /* PRINTF("LIDT_M32_16: executed at L%08X, read base=%08X, limit=%04X\n",
                 instruction_state.data.linear, base, limit); */
        _chr(_s_load_idtr(context, base, limit, _GetOperandSize));
        _be;
        break;
    case 4: /* SMSW_RM16 */
        _bb("SMSW_RM16");
        _chr(_d_modrm(context, 0, ((_GetModRM_MOD(modrm) == 3) ? _GetOperandSize : 2)));
        instruction_state.data.crm = GetMax16(cpu_state.data.cr0);
        if (_GetOperandSize == 4 && !instruction_state.data.flagMem)
            _chr(_m_write_rm(context, 4));
        else
            _chr(_m_write_rm(context, 2));
        _be;
        break;
    case 5:
        _bb("ModRM_REG(5)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 6: /* LMSW_RM16 */
        _bb("LMSW_RM16");
        _chr(_d_modrm(context, 0, 2));
        _chr(_m_read_rm(context, 2));
        _chr(_s_load_cr0_msw(context, GetMax16(instruction_state.data.crm)));
        _be;
        break;
    case 7:
        _bb("ModRM_REG(7)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void LAR_R32_RM32(core_machine_cpu_execution_context *context) {
    t_nubit16 selector;
    t_nubit64 descriptor;
    _cb("LAR_R32_RM32");
    _adv;
    if (_IsProtected) {
        _bb("Protected(1)");
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, 2));
        selector = GetMax16(instruction_state.data.crm);
        if (_s_check_selector(context, selector)) {
            _ClrEFLAGS_ZF;
        } else {
            _bb("selector(valid)");
            _chr(_s_read_xdt(context, selector, GetRef(descriptor)));
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
                    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(descriptor), 2));
                    _be;
                    break;
                case 4:
                    _bb("OperandSize(4)");
                    descriptor = (GetMax32(descriptor >> 32) & 0x00ffff00);
                    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(descriptor), 4));
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
        _chr(UndefinedOpcode(context));
        _be;
    }
    _ce;
}
static void LSL_R32_RM32(core_machine_cpu_execution_context *context) {
    t_nubit16 selector;
    t_nubit32 limit;
    t_nubit64 descriptor;
    _cb("LSL_R32_RM32");
    _adv;
    if (_IsProtected) {
        _bb("Protected(1)");
        _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        _chr(_m_read_rm(context, 2));
        selector = GetMax16(instruction_state.data.crm);
        if (_s_check_selector(context, selector)) {
            _ClrEFLAGS_ZF;
        } else {
            _bb("selector(valid)");
            _chr(_s_read_xdt(context, selector, GetRef(descriptor)));
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
                    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(limit), 2));
                    _be;
                    break;
                case 4:
                    _bb("OperandSize(4)");
                    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(limit), 4));
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
        _chr(UndefinedOpcode(context));
        _be;
    }
    _ce;
}
static void CLTS(core_machine_cpu_execution_context *context) {
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
_______todo WBINVD(core_machine_cpu_execution_context *context) {
    _cb("WBINVD");
    UndefinedOpcode(context);
    instruction_state.data.flagIgnore = True;
    _ce;
}
static void MOV_R32_CR(core_machine_cpu_execution_context *context) {
    _cb("MOV_R32_CR");
    _adv;
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    _chr(_d_modrm_creg(context));
    _chr(_m_write_ref(context, instruction_state.data.rrm, GetRef(instruction_state.data.cr), 4));
    _ce;
}
static void MOV_R32_DR(core_machine_cpu_execution_context *context) {
    _cb("MOV_R32_DR");
    _new_code_path_;
    _adv;
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    _chr(_d_modrm_dreg(context));
    _chr(_m_write_ref(context, instruction_state.data.rrm, GetRef(instruction_state.data.cr), 4));
    _ce;
}
static void MOV_CR_R32(core_machine_cpu_execution_context *context) {
    _cb("MOV_CR_R32");
    _adv;
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    _chr(_d_modrm_creg(context));
    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.crm), 4));
    /* if (instruction_state.data.rr == (t_vaddrcc)&cpu_state.data.cr0) {
        PRINTF("MOV_CR_R32: executed at L%08X, CR0=%08X\n", instruction_state.data.linear, cpu_state.data.cr0);
    }
    if (instruction_state.data.rr == (t_vaddrcc)&cpu_state.data.cr2) {
        PRINTF("MOV_CR_R32: executed at L%08X, CR2=%08X\n", instruction_state.data.linear, cpu_state.data.cr2);
    }
    if (instruction_state.data.rr == (t_vaddrcc)&cpu_state.data.cr3) {
        PRINTF("MOV_CR_R32: executed at L%08X, CR3=%08X\n", instruction_state.data.linear, cpu_state.data.cr3);
    } */
    _ce;
}
static void MOV_DR_R32(core_machine_cpu_execution_context *context) {
    _cb("MOV_DR_R32");
    _new_code_path_;
    _adv;
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    _chr(_d_modrm_dreg(context));
    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.crm), 4));
    _ce;
}
static void MOV_R32_TR(core_machine_cpu_execution_context *context) {
    _cb("MOV_R32_TR");
    _new_code_path_;
    _adv;
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    _chr(_d_modrm_treg(context));
    _chr(_m_write_ref(context, instruction_state.data.rrm, GetRef(instruction_state.data.cr), 4));
    _ce;
}
static void MOV_TR_R32(core_machine_cpu_execution_context *context) {
    _cb("MOV_TR_R32");
    _new_code_path_;
    _adv;
    if (_GetCPL) {
        _bb("CPL(!0)");
        _chr(_SetExcept_GP(0));
        _be;
    }
    _chr(_d_modrm_treg(context));
    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.crm), 4));
    _ce;
}
_______todo WRMSR(core_machine_cpu_execution_context *context) {
    _cb("WRMSR");
    UndefinedOpcode(context);
    instruction_state.data.flagIgnore = True;
    _ce;
}
_______todo RDMSR(core_machine_cpu_execution_context *context) {
    _cb("RDMSR");
    UndefinedOpcode(context);
    instruction_state.data.flagIgnore = True;
    _ce;
}
static void JO_REL32(core_machine_cpu_execution_context *context) {
    _cb("JO_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(context, _GetOperandSize));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_OF));
    _ce;
}
static void JNO_REL32(core_machine_cpu_execution_context *context) {
    _cb("JNO_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(context, _GetOperandSize));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_OF));
    _ce;
}
static void JC_REL32(core_machine_cpu_execution_context *context) {
    _cb("JC_REL32");
    _adv;
    _chr(_d_imm(context, _GetOperandSize));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_CF));
    _ce;
}
static void JNC_REL32(core_machine_cpu_execution_context *context) {
    _cb("JNC_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(context, _GetOperandSize));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_CF));
    _ce;
}
static void JZ_REL32(core_machine_cpu_execution_context *context) {
    _cb("JZ_REL32");
    _adv;
    _chr(_d_imm(context, _GetOperandSize));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_ZF));
    _ce;
}
static void JNZ_REL32(core_machine_cpu_execution_context *context) {
    _cb("JNZ_REL32");
    _adv;
    _chr(_d_imm(context, _GetOperandSize));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_ZF));
    _ce;
}
static void JNA_REL32(core_machine_cpu_execution_context *context) {
    _cb("JNA_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(context, _GetOperandSize));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), _GetOperandSize,
                (_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    _ce;
}
static void JA_REL32(core_machine_cpu_execution_context *context) {
    _cb("JA_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(context, _GetOperandSize));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), _GetOperandSize,
                !(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    _ce;
}
static void JS_REL32(core_machine_cpu_execution_context *context) {
    _cb("JS_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(context, _GetOperandSize));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_SF));
    _ce;
}
static void JNS_REL32(core_machine_cpu_execution_context *context) {
    _cb("JNS_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(context, _GetOperandSize));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_SF));
    _ce;
}
static void JP_REL32(core_machine_cpu_execution_context *context) {
    _cb("JP_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(context, _GetOperandSize));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_PF));
    _ce;
}
static void JNP_REL32(core_machine_cpu_execution_context *context) {
    _cb("JNP_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(context, _GetOperandSize));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_PF));
    _ce;
}
static void JL_REL32(core_machine_cpu_execution_context *context) {
    _cb("JL_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(context, _GetOperandSize));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), _GetOperandSize, (_GetEFLAGS_SF != _GetEFLAGS_OF)));
    _ce;
}
static void JNL_REL32(core_machine_cpu_execution_context *context) {
    _cb("JNL_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(context, _GetOperandSize));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), _GetOperandSize, (_GetEFLAGS_SF == _GetEFLAGS_OF)));
    _ce;
}
static void JNG_REL32(core_machine_cpu_execution_context *context) {
    _cb("JNG_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(context, _GetOperandSize));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), _GetOperandSize,
                (_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF))));
    _ce;
}
static void JG_REL32(core_machine_cpu_execution_context *context) {
    _cb("JG_REL32");
    _new_code_path_;
    _adv;
    _chr(_d_imm(context, _GetOperandSize));
    _chr(_e_jcc(context, GetMax32(instruction_state.data.cimm), _GetOperandSize,
                (!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF))));
    _ce;
}
static void SETO_RM8(core_machine_cpu_execution_context *context) {
    _cb("SETO_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_setcc_rm(context, _GetEFLAGS_OF));
    _ce;
}
static void SETNO_RM8(core_machine_cpu_execution_context *context) {
    _cb("SETO_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_setcc_rm(context, !_GetEFLAGS_OF));
    _ce;
}
static void SETC_RM8(core_machine_cpu_execution_context *context) {
    _cb("SETC_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_setcc_rm(context, _GetEFLAGS_CF));
    _ce;
}
static void SETNC_RM8(core_machine_cpu_execution_context *context) {
    _cb("SETNC_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_setcc_rm(context, !_GetEFLAGS_CF));
    _ce;
}
static void SETZ_RM8(core_machine_cpu_execution_context *context) {
    _cb("SETZ_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_setcc_rm(context, _GetEFLAGS_ZF));
    _ce;
}
static void SETNZ_RM8(core_machine_cpu_execution_context *context) {
    _cb("SETNZ_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_setcc_rm(context, !_GetEFLAGS_ZF));
    _ce;
}
static void SETNA_RM8(core_machine_cpu_execution_context *context) {
    _cb("SETNA_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_setcc_rm(context, _GetEFLAGS_CF || _GetEFLAGS_ZF));
    _ce;
}
static void SETA_RM8(core_machine_cpu_execution_context *context) {
    _cb("SETA_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_setcc_rm(context, !(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    _ce;
}
static void SETS_RM8(core_machine_cpu_execution_context *context) {
    _cb("SETS_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_setcc_rm(context, _GetEFLAGS_SF));
    _ce;
}
static void SETNS_RM8(core_machine_cpu_execution_context *context) {
    _cb("SETNS_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_setcc_rm(context, !_GetEFLAGS_SF));
    _ce;
}
static void SETP_RM8(core_machine_cpu_execution_context *context) {
    _cb("SETP_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_setcc_rm(context, _GetEFLAGS_PF));
    _ce;
}
static void SETNP_RM8(core_machine_cpu_execution_context *context) {
    _cb("SETNP_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_setcc_rm(context, !_GetEFLAGS_PF));
    _ce;
}
static void SETL_RM8(core_machine_cpu_execution_context *context) {
    _cb("SETL_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_setcc_rm(context, _GetEFLAGS_SF != _GetEFLAGS_OF));
    _ce;
}
static void SETNL_RM8(core_machine_cpu_execution_context *context) {
    _cb("SETNL_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_setcc_rm(context, _GetEFLAGS_SF == _GetEFLAGS_OF));
    _ce;
}
static void SETNG_RM8(core_machine_cpu_execution_context *context) {
    _cb("SETNG_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_setcc_rm(context, _GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF)));
    _ce;
}
static void SETG_RM8(core_machine_cpu_execution_context *context) {
    _cb("SETG_RM8");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, 0, 1));
    _chr(_m_setcc_rm(context, !_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF)));
    _ce;
}
static void PUSH_FS(core_machine_cpu_execution_context *context) {
    t_nubit32 xs_sel;
    _cb("PUSH_FS");
    _adv;
    xs_sel = cpu_state.data.fs.selector;
    _chr(_e_push(context, GetRef(xs_sel), _GetOperandSize));
    _ce;
}
static void POP_FS(core_machine_cpu_execution_context *context) {
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    t_nubit32 xs_sel;
    _cb("POP_FS");
    _adv;
    _chr(_e_pop(context, GetRef(xs_sel), _GetOperandSize));
    _chr(_s_load_fs(context, GetMax16(xs_sel)));
    _ce;
}
_______todo CPUID(core_machine_cpu_execution_context *context) {
    _cb("CPUID");
    UndefinedOpcode(context);
    instruction_state.data.flagIgnore = True;
    _ce;
}
static void BT_RM32_R32(core_machine_cpu_execution_context *context) {
    _cb("BT_RM32_R32");
    _adv;
    _chr(_d_bit_rmimm(context, _GetOperandSize, _GetOperandSize, 0));
    _chr(_m_read_rm(context, _GetOperandSize));
    _chr(_a_bt(context, (t_nubit32)instruction_state.data.crm, (t_nubit32)instruction_state.data.cimm, _GetOperandSize * 8));
    _ce;
}
static void SHLD_RM32_R32_I8(core_machine_cpu_execution_context *context) {
    _cb("SHLD_RM32_R32_I8");
    _adv;
    _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    _chr(_m_read_rm(context, _GetOperandSize));
    _chr(_d_imm(context, 1));
    _chr(_a_shld(context, instruction_state.data.crm, instruction_state.data.cr, GetMax8(instruction_state.data.cimm), _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    _chr(_m_write_rm(context, _GetOperandSize));
    _ce;
}
static void SHLD_RM32_R32_CL(core_machine_cpu_execution_context *context) {
    _cb("SHLD_RM32_R32_CL");
    _adv;
    _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    _chr(_m_read_rm(context, _GetOperandSize));
    _chr(_a_shld(context, instruction_state.data.crm, instruction_state.data.cr, cpu_state.data.cl, _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    _chr(_m_write_rm(context, _GetOperandSize));
    _ce;
}
static void PUSH_GS(core_machine_cpu_execution_context *context) {
    t_nubit32 xs_sel;
    _cb("PUSH_GS");
    _adv;
    xs_sel = cpu_state.data.gs.selector;
    _chr(_e_push(context, GetRef(xs_sel), _GetOperandSize));
    _ce;
}
static void POP_GS(core_machine_cpu_execution_context *context) {
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    t_nubit32 xs_sel;
    _cb("POP_GS");
    _adv;
    _chr(_e_pop(context, GetRef(xs_sel), _GetOperandSize));
    _chr(_s_load_gs(context, GetMax16(xs_sel)));
    _ce;
}
_______todo RSM(core_machine_cpu_execution_context *context) {
    _cb("RSM");
    UndefinedOpcode(context);
    instruction_state.data.flagIgnore = True;
    _ce;
}
static void BTS_RM32_R32(core_machine_cpu_execution_context *context) {
    _cb("BTS_RM32_R32");
    _adv;
    _chr(_d_bit_rmimm(context, _GetOperandSize, _GetOperandSize, 1));
    _chr(_m_read_rm(context, _GetOperandSize));
    _chr(_a_bts(context, (t_nubit32)instruction_state.data.crm, (t_nubit32)instruction_state.data.cimm, _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    _chr(_m_write_rm(context, _GetOperandSize));
    _ce;
}
static void SHRD_RM32_R32_I8(core_machine_cpu_execution_context *context) {
    _cb("SHRD_RM32_R32_I8");
    _adv;
    _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    _chr(_m_read_rm(context, _GetOperandSize));
    _chr(_d_imm(context, 1));
    _chr(_a_shrd(context, instruction_state.data.crm, instruction_state.data.cr, GetMax8(instruction_state.data.cimm), _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    _chr(_m_write_rm(context, _GetOperandSize));
    _ce;
}
static void SHRD_RM32_R32_CL(core_machine_cpu_execution_context *context) {
    _cb("SHRD_RM32_R32_CL");
    _adv;
    _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    _chr(_m_read_rm(context, _GetOperandSize));
    _chr(_a_shrd(context, instruction_state.data.crm, instruction_state.data.cr, cpu_state.data.cl, _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    _chr(_m_write_rm(context, _GetOperandSize));
    _ce;
}
static void IMUL_R32_RM32(core_machine_cpu_execution_context *context) {
    _cb("IMUL_R32_RM32");
    _adv;
    _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    _chr(_m_read_rm(context, _GetOperandSize));
    _chr(_a_imul2(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), _GetOperandSize));
    _ce;
}
static void LSS_R32_M16_32(core_machine_cpu_execution_context *context) {
    t_nubit16 selector;
    t_nubit32 offset;
    _cb("LSS_R32_M16_32");
    _adv;
    _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize + 2));
    if (!instruction_state.data.flagMem) {
        _bb("flagMem(0)");
        _chr(UndefinedOpcode(context));
        _be;
    }
    _chr(_m_read_rm(context, _GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        offset = GetMax16(instruction_state.data.crm);
        break;
    case 4:
        offset = GetMax32(instruction_state.data.crm);
        break;
    default:
        _impossible_r_;
        break;
    }
    instruction_state.data.mrm.offset += _GetOperandSize;
    _chr(_m_read_rm(context, 2));
    selector = GetMax16(instruction_state.data.crm);
    _chr(_e_load_far(context, &cpu_state.data.ss, instruction_state.data.rr, selector, offset, _GetOperandSize));
    _ce;
}
static void BTR_RM32_R32(core_machine_cpu_execution_context *context) {
    _cb("BTR_RM32_R32");
    _adv;
    _chr(_d_bit_rmimm(context, _GetOperandSize, _GetOperandSize, 1));
    _chr(_m_read_rm(context, _GetOperandSize));
    _chr(_a_btr(context, (t_nubit32)instruction_state.data.crm, (t_nubit32)instruction_state.data.cimm, _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    _chr(_m_write_rm(context, _GetOperandSize));
    _ce;
}
static void LFS_R32_M16_32(core_machine_cpu_execution_context *context) {
    t_nubit16 selector;
    t_nubit32 offset;
    _cb("LFS_R32_M16_32");
    _adv;
    _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize + 2));
    if (!instruction_state.data.flagMem) {
        _bb("flagMem(0)");
        _chr(UndefinedOpcode(context));
        _be;
    }
    _chr(_m_read_rm(context, _GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        offset = GetMax16(instruction_state.data.crm);
        break;
    case 4:
        offset = GetMax32(instruction_state.data.crm);
        break;
    default:
        _impossible_r_;
        break;
    }
    instruction_state.data.mrm.offset += _GetOperandSize;
    _chr(_m_read_rm(context, 2));
    selector = GetMax16(instruction_state.data.crm);
    _chr(_e_load_far(context, &cpu_state.data.fs, instruction_state.data.rr, selector, offset, _GetOperandSize));
    _ce;
}
static void LGS_R32_M16_32(core_machine_cpu_execution_context *context) {
    t_nubit16 selector;
    t_nubit32 offset;
    _cb("LGS_R32_M16_32");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize + 2));
    if (!instruction_state.data.flagMem) {
        _bb("flagMem(0)");
        _chr(UndefinedOpcode(context));
        _be;
    }
    _chr(_m_read_rm(context, _GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        offset = GetMax16(instruction_state.data.crm);
        break;
    case 4:
        offset = GetMax32(instruction_state.data.crm);
        break;
    default:
        _impossible_r_;
        break;
    }
    instruction_state.data.mrm.offset += _GetOperandSize;
    _chr(_m_read_rm(context, 2));
    selector = GetMax16(instruction_state.data.crm);
    _chr(_e_load_far(context, &cpu_state.data.gs, instruction_state.data.rr, selector, offset, _GetOperandSize));
    _ce;
}
static void MOVZX_R32_RM8(core_machine_cpu_execution_context *context) {
    _cb("MOVZX_R32_RM8");
    _adv;
    _chr(_d_modrm(context, _GetOperandSize, 1));
    _chr(_m_read_rm(context, 1));
    instruction_state.data.crm = (t_nubit8)instruction_state.data.crm;
    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.crm), _GetOperandSize));
    _ce;
}
static void MOVZX_R32_RM16(core_machine_cpu_execution_context *context) {
    _cb("MOVZX_R32_RM16");
    _adv;
    _chr(_d_modrm(context, 4, 2));
    _chr(_m_read_rm(context, 2));
    instruction_state.data.crm = (t_nubit16)instruction_state.data.crm;
    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.crm), 4));
    _ce;
}
static void INS_0F_BA(core_machine_cpu_execution_context *context) {
    t_bool write = 0;
    t_nubit8 modrm = 0x00;
    _cb("INS_0F_BA");
    _new_code_path_;
    _adv;
    _chr(_s_read_cs(context, cpu_state.data.eip, GetRef(modrm), 1));
    if (_GetModRM_REG(modrm) == 4) write = 0;
    else write = 1;
    _chr(_d_bit_rmimm(context, 0, _GetOperandSize, write));
    _chr(_m_read_rm(context, _GetOperandSize));
    switch (instruction_state.data.cr) {
    case 0:
        _bb("instruction_state.data.cr(0)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 1:
        _bb("instruction_state.data.cr(1)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 2:
        _bb("instruction_state.data.cr(2)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 3:
        _bb("instruction_state.data.cr(3)");
        _chr(UndefinedOpcode(context));
        _be;
        break;
    case 4: /* BT_RM32_I8 */
        _bb("BT_RM32_I8");
        _chr(_a_bt(context, (t_nubit32)instruction_state.data.crm, (t_nubit32)instruction_state.data.cimm, _GetOperandSize * 8));
        _be;
        break;
    case 5: /* BTS_RM32_I8 */
        _bb("BTS_RM32_I8");
        _chr(_a_bts(context, (t_nubit32)instruction_state.data.crm, (t_nubit32)instruction_state.data.cimm, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, _GetOperandSize));
        _be;
        break;
    case 6: /* BTR_RM32_I8 */
        _bb("BTR_RM32_I8");
        _chr(_a_btr(context, (t_nubit32)instruction_state.data.crm, (t_nubit32)instruction_state.data.cimm, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, _GetOperandSize));
        _be;
        break;
    case 7: /* BTC_RM32_I8 */
        _bb("BTC_RM32_I8");
        _chr(_a_btc(context, (t_nubit32)instruction_state.data.crm, (t_nubit32)instruction_state.data.cimm, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        _chr(_m_write_rm(context, _GetOperandSize));
        _be;
        break;
    default:
        _impossible_r_;
        break;
    }
    _ce;
}
static void BTC_RM32_R32(core_machine_cpu_execution_context *context) {
    _cb("BTC_RM32_R32");
    _new_code_path_;
    _adv;
    _chr(_d_bit_rmimm(context, _GetOperandSize, _GetOperandSize, 1));
    _chr(_m_read_rm(context, _GetOperandSize));
    _chr(_a_btc(context, (t_nubit32)instruction_state.data.crm, (t_nubit32)instruction_state.data.cimm, _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    _chr(_m_write_rm(context, _GetOperandSize));
    _ce;
}
static void BSF_R32_RM32(core_machine_cpu_execution_context *context) {
    _cb("BSF_R32_RM32");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    _chr(_m_read_rm(context, _GetOperandSize));
    _chr(_a_bscc(context, instruction_state.data.crm, _GetOperandSize * 8, 1));
    if (!_GetEFLAGS_ZF) {
        _bb("EFLAGS_ZF(0)");
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), _GetOperandSize));
        _be;
    }
    _ce;
}
static void BSR_R32_RM32(core_machine_cpu_execution_context *context) {
    _cb("BSR_R32_RM32");
    _new_code_path_;
    _adv;
    _chr(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    _chr(_m_read_rm(context, _GetOperandSize));
    _chr(_a_bscc(context, instruction_state.data.crm, _GetOperandSize * 8, 0));
    if (!_GetEFLAGS_ZF) {
        _bb("EFLAGS_ZF(0)");
        _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.result), _GetOperandSize));
        _be;
    }
    _ce;
}
static void MOVSX_R32_RM8(core_machine_cpu_execution_context *context) {
    _cb("MOVSX_R32_RM8");
    _adv;
    _chr(_d_modrm(context, _GetOperandSize, 1));
    _chr(_m_read_rm(context, 1));
    instruction_state.data.crm = (t_nsbit8)instruction_state.data.crm;
    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.crm), _GetOperandSize));
    _ce;
}
static void MOVSX_R32_RM16(core_machine_cpu_execution_context *context) {
    _cb("MOVSX_R32_RM16");
    _adv;
    _chr(_d_modrm(context, 4, 2));
    _chr(_m_read_rm(context, 2));
    instruction_state.data.crm = (t_nsbit16)instruction_state.data.crm;
    _chr(_m_write_ref(context, instruction_state.data.rr, GetRef(instruction_state.data.crm), 4));
    _ce;
}

static void ExecInit(core_machine_cpu_execution_context *context) {
    instruction_state.data.flagIgnore = False;
    instruction_state.data.msize = 0;
    instruction_state.data.reccs = cpu_state.data.cs.selector;
    instruction_state.data.receip = cpu_state.data.eip;
    instruction_state.data.linear = cpu_state.data.cs.base + cpu_state.data.eip;
    if (core_machine_cpu_execution_read_linear(context,
        instruction_state.data.linear,
        (t_vaddrcc) instruction_state.data.opcodes, 15)) {
        instruction_state.data.oplen = 0;
    } else {
        instruction_state.data.oplen = 15;
    }

    instruction_state.data.flagLock = False;
    instruction_state.data.oldcpu = cpu_state;
    instruction_state.data.roverds = &cpu_state.data.ds;
    instruction_state.data.roverss = &cpu_state.data.ss;
    instruction_state.data.prefix_rep = PREFIX_REP_NONE;
    instruction_state.data.prefix_oprsize = False;
    instruction_state.data.prefix_addrsize = False;
    instruction_state.data.flagMem = False;
    instruction_state.data.flagInsLoop = False;
    instruction_state.data.flagMaskInt = False;
    instruction_state.data.bit = 0;
    instruction_state.data.opr1 = 0;
    instruction_state.data.opr2 = 0;
    instruction_state.data.result = 0;
    instruction_state.data.udf = Zero32;
    instruction_state.data.mrm.rsreg = NULL;
    instruction_state.data.mrm.offset = Zero32;
    instruction_state.data.except = Zero32;
    instruction_state.data.excode = Zero32;
#if VCPUINS_TRACE == 1
    utilsTraceInit(&trace);
#endif
}
static void ExecFinal(core_machine_cpu_execution_context *context) {
    if (instruction_state.data.flagInsLoop) {
        cpu_state.data.cs = instruction_state.data.oldcpu.data.cs;
        cpu_state.data.eip = instruction_state.data.oldcpu.data.eip;
    }
#if VCPUINS_TRACE == 1
    if (trace.callCount && !instruction_state.data.except) _SetExcept_CE(trace.cid);
    utilsTraceFinal(&trace);
#endif
    if (instruction_state.data.except) {
        cpu_state = instruction_state.data.oldcpu;
        if (GetBit(instruction_state.data.except, VCPUINS_EXCEPT_GP)) {
            ExecInit(context);
            ClrBit(instruction_state.data.except, VCPUINS_EXCEPT_GP);
            _e_except_n(context, 0x0d, _GetOperandSize);
        }
        core_machine_cpu_execution_request_stop(context);
    }
}
static void ExecIns(core_machine_cpu_execution_context *context) {
    t_nubit8 opcode = 0;
    ExecInit(context);
    do {
        _cb("ExecIns");
        _chb(_s_read_cs(context, cpu_state.data.eip, GetRef(opcode), 1));
        _chb(ExecCpuInstruction(instruction_state.connect.insTable[opcode]));
        _chb(_s_test_eip(context));
        _chb(_s_test_esp(context));
        _ce;
    } while (_kdf_check_prefix(context, opcode));
    if (instruction_state.data.flagWE && instruction_state.data.weLinear == instruction_state.data.linear) {
        PRINTF("Watch point caught at L%08x: EXECUTED\n", instruction_state.data.linear);
        /* printCpuReg(); */
        core_machine_cpu_execution_request_stop(context);
    }
    ExecFinal(context);
}
static void ExecInt(core_machine_cpu_execution_context *context) {
    t_nubit8 intr = 0x00;
    /* hardware interrupt handler */
    if (instruction_state.data.flagMaskInt)
        return;
    if (!cpu_state.data.flagMaskNMI && cpu_state.data.flagNMI) {
        cpu_state.data.flagHalt = False;
        cpu_state.data.flagNMI = False;
        ExecInit(context);
        _e_intr_n(context, 0x02, _GetOperandSize);
        ExecFinal(context);
    }
    if (_GetEFLAGS_IF && core_machine_pic_scan_interrupt(
            context->pic_master, context->pic_slave)) {
        cpu_state.data.flagHalt = False;
        intr = core_machine_pic_get_interrupt(context->pic_master,
            context->pic_slave);
        ExecInit(context);
        _e_intr_n(context, intr, _GetOperandSize);
        ExecFinal(context);
        instruction_state.data.flagIgnore = True;
    }
    if (_GetEFLAGS_TF) {
        cpu_state.data.flagHalt = False;
        ExecInit(context);
        _e_intr_n(context, 0x01, _GetOperandSize);
        ExecFinal(context);
    }
}

/* external interface */
t_bool core_machine_cpu_execution_load_segment(
    core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg,
    t_nubit16 selector) {
    t_bool fail;
    t_nubit32 oldexcept = instruction_state.data.except;
    instruction_state.data.except = 0;
    _ksa_load_sreg(context, rsreg, selector);
    fail = !!instruction_state.data.except;
    instruction_state.data.except = oldexcept;
    return fail;
}
t_bool core_machine_cpu_execution_read_linear(
    core_machine_cpu_execution_context *context, t_nubit32 linear,
    t_vaddrcc rdata, t_nubit8 byte) {
    t_bool fail;
    t_nubit32 oldexcept = instruction_state.data.except;
    instruction_state.data.except = 0;
    _kma_read_linear(context, linear, rdata, byte, 0x00, 1);
    fail = !!instruction_state.data.except;
    instruction_state.data.except = oldexcept;
    return fail;
}
t_bool core_machine_cpu_execution_write_linear(
    core_machine_cpu_execution_context *context, t_nubit32 linear,
    t_vaddrcc rdata, t_nubit8 byte) {
    t_bool fail;
    t_nubit32 oldexcept = instruction_state.data.except;
    instruction_state.data.except = 0;
    _kma_write_linear(context, linear, rdata, byte, 0x00, 1);
    fail = !!instruction_state.data.except;
    instruction_state.data.except = oldexcept;
    return fail;
}

void core_machine_cpu_execution_initialize(
    core_machine_cpu_execution_context *context) {
    instruction_state.connect.insTable[0x00] = (core_machine_cpu_instruction_handler) ADD_RM8_R8;
    instruction_state.connect.insTable[0x01] = (core_machine_cpu_instruction_handler) ADD_RM32_R32;
    instruction_state.connect.insTable[0x02] = (core_machine_cpu_instruction_handler) ADD_R8_RM8;
    instruction_state.connect.insTable[0x03] = (core_machine_cpu_instruction_handler) ADD_R32_RM32;
    instruction_state.connect.insTable[0x04] = (core_machine_cpu_instruction_handler) ADD_AL_I8;
    instruction_state.connect.insTable[0x05] = (core_machine_cpu_instruction_handler) ADD_EAX_I32;
    instruction_state.connect.insTable[0x06] = (core_machine_cpu_instruction_handler) PUSH_ES;
    instruction_state.connect.insTable[0x07] = (core_machine_cpu_instruction_handler) POP_ES;
    instruction_state.connect.insTable[0x08] = (core_machine_cpu_instruction_handler) OR_RM8_R8;
    instruction_state.connect.insTable[0x09] = (core_machine_cpu_instruction_handler) OR_RM32_R32;
    instruction_state.connect.insTable[0x0a] = (core_machine_cpu_instruction_handler) OR_R8_RM8;
    instruction_state.connect.insTable[0x0b] = (core_machine_cpu_instruction_handler) OR_R32_RM32;
    instruction_state.connect.insTable[0x0c] = (core_machine_cpu_instruction_handler) OR_AL_I8;
    instruction_state.connect.insTable[0x0d] = (core_machine_cpu_instruction_handler) OR_EAX_I32;
    instruction_state.connect.insTable[0x0e] = (core_machine_cpu_instruction_handler) PUSH_CS;
    instruction_state.connect.insTable[0x0f] = (core_machine_cpu_instruction_handler) INS_0F;
    instruction_state.connect.insTable[0x10] = (core_machine_cpu_instruction_handler) ADC_RM8_R8;
    instruction_state.connect.insTable[0x11] = (core_machine_cpu_instruction_handler) ADC_RM32_R32;
    instruction_state.connect.insTable[0x12] = (core_machine_cpu_instruction_handler) ADC_R8_RM8;
    instruction_state.connect.insTable[0x13] = (core_machine_cpu_instruction_handler) ADC_R32_RM32;
    instruction_state.connect.insTable[0x14] = (core_machine_cpu_instruction_handler) ADC_AL_I8;
    instruction_state.connect.insTable[0x15] = (core_machine_cpu_instruction_handler) ADC_EAX_I32;
    instruction_state.connect.insTable[0x16] = (core_machine_cpu_instruction_handler) PUSH_SS;
    instruction_state.connect.insTable[0x17] = (core_machine_cpu_instruction_handler) POP_SS;
    instruction_state.connect.insTable[0x18] = (core_machine_cpu_instruction_handler) SBB_RM8_R8;
    instruction_state.connect.insTable[0x19] = (core_machine_cpu_instruction_handler) SBB_RM32_R32;
    instruction_state.connect.insTable[0x1a] = (core_machine_cpu_instruction_handler) SBB_R8_RM8;
    instruction_state.connect.insTable[0x1b] = (core_machine_cpu_instruction_handler) SBB_R32_RM32;
    instruction_state.connect.insTable[0x1c] = (core_machine_cpu_instruction_handler) SBB_AL_I8;
    instruction_state.connect.insTable[0x1d] = (core_machine_cpu_instruction_handler) SBB_EAX_I32;
    instruction_state.connect.insTable[0x1e] = (core_machine_cpu_instruction_handler) PUSH_DS;
    instruction_state.connect.insTable[0x1f] = (core_machine_cpu_instruction_handler) POP_DS;
    instruction_state.connect.insTable[0x20] = (core_machine_cpu_instruction_handler) AND_RM8_R8;
    instruction_state.connect.insTable[0x21] = (core_machine_cpu_instruction_handler) AND_RM32_R32;
    instruction_state.connect.insTable[0x22] = (core_machine_cpu_instruction_handler) AND_R8_RM8;
    instruction_state.connect.insTable[0x23] = (core_machine_cpu_instruction_handler) AND_R32_RM32;
    instruction_state.connect.insTable[0x24] = (core_machine_cpu_instruction_handler) AND_AL_I8;
    instruction_state.connect.insTable[0x25] = (core_machine_cpu_instruction_handler) AND_EAX_I32;
    instruction_state.connect.insTable[0x26] = (core_machine_cpu_instruction_handler) PREFIX_ES;
    instruction_state.connect.insTable[0x27] = (core_machine_cpu_instruction_handler) DAA;
    instruction_state.connect.insTable[0x28] = (core_machine_cpu_instruction_handler) SUB_RM8_R8;
    instruction_state.connect.insTable[0x29] = (core_machine_cpu_instruction_handler) SUB_RM32_R32;
    instruction_state.connect.insTable[0x2a] = (core_machine_cpu_instruction_handler) SUB_R8_RM8;
    instruction_state.connect.insTable[0x2b] = (core_machine_cpu_instruction_handler) SUB_R32_RM32;
    instruction_state.connect.insTable[0x2c] = (core_machine_cpu_instruction_handler) SUB_AL_I8;
    instruction_state.connect.insTable[0x2d] = (core_machine_cpu_instruction_handler) SUB_EAX_I32;
    instruction_state.connect.insTable[0x2e] = (core_machine_cpu_instruction_handler) PREFIX_CS;
    instruction_state.connect.insTable[0x2f] = (core_machine_cpu_instruction_handler) DAS;
    instruction_state.connect.insTable[0x30] = (core_machine_cpu_instruction_handler) XOR_RM8_R8;
    instruction_state.connect.insTable[0x31] = (core_machine_cpu_instruction_handler) XOR_RM32_R32;
    instruction_state.connect.insTable[0x32] = (core_machine_cpu_instruction_handler) XOR_R8_RM8;
    instruction_state.connect.insTable[0x33] = (core_machine_cpu_instruction_handler) XOR_R32_RM32;
    instruction_state.connect.insTable[0x34] = (core_machine_cpu_instruction_handler) XOR_AL_I8;
    instruction_state.connect.insTable[0x35] = (core_machine_cpu_instruction_handler) XOR_EAX_I32;
    instruction_state.connect.insTable[0x36] = (core_machine_cpu_instruction_handler) PREFIX_SS;
    instruction_state.connect.insTable[0x37] = (core_machine_cpu_instruction_handler) AAA;
    instruction_state.connect.insTable[0x38] = (core_machine_cpu_instruction_handler) CMP_RM8_R8;
    instruction_state.connect.insTable[0x39] = (core_machine_cpu_instruction_handler) CMP_RM32_R32;
    instruction_state.connect.insTable[0x3a] = (core_machine_cpu_instruction_handler) CMP_R8_RM8;
    instruction_state.connect.insTable[0x3b] = (core_machine_cpu_instruction_handler) CMP_R32_RM32;
    instruction_state.connect.insTable[0x3c] = (core_machine_cpu_instruction_handler) CMP_AL_I8;
    instruction_state.connect.insTable[0x3d] = (core_machine_cpu_instruction_handler) CMP_EAX_I32;
    instruction_state.connect.insTable[0x3e] = (core_machine_cpu_instruction_handler) PREFIX_DS;
    instruction_state.connect.insTable[0x3f] = (core_machine_cpu_instruction_handler) AAS;
    instruction_state.connect.insTable[0x40] = (core_machine_cpu_instruction_handler) INC_EAX;
    instruction_state.connect.insTable[0x41] = (core_machine_cpu_instruction_handler) INC_ECX;
    instruction_state.connect.insTable[0x42] = (core_machine_cpu_instruction_handler) INC_EDX;
    instruction_state.connect.insTable[0x43] = (core_machine_cpu_instruction_handler) INC_EBX;
    instruction_state.connect.insTable[0x44] = (core_machine_cpu_instruction_handler) INC_ESP;
    instruction_state.connect.insTable[0x45] = (core_machine_cpu_instruction_handler) INC_EBP;
    instruction_state.connect.insTable[0x46] = (core_machine_cpu_instruction_handler) INC_ESI;
    instruction_state.connect.insTable[0x47] = (core_machine_cpu_instruction_handler) INC_EDI;
    instruction_state.connect.insTable[0x48] = (core_machine_cpu_instruction_handler) DEC_EAX;
    instruction_state.connect.insTable[0x49] = (core_machine_cpu_instruction_handler) DEC_ECX;
    instruction_state.connect.insTable[0x4a] = (core_machine_cpu_instruction_handler) DEC_EDX;
    instruction_state.connect.insTable[0x4b] = (core_machine_cpu_instruction_handler) DEC_EBX;
    instruction_state.connect.insTable[0x4c] = (core_machine_cpu_instruction_handler) DEC_ESP;
    instruction_state.connect.insTable[0x4d] = (core_machine_cpu_instruction_handler) DEC_EBP;
    instruction_state.connect.insTable[0x4e] = (core_machine_cpu_instruction_handler) DEC_ESI;
    instruction_state.connect.insTable[0x4f] = (core_machine_cpu_instruction_handler) DEC_EDI;
    instruction_state.connect.insTable[0x50] = (core_machine_cpu_instruction_handler) PUSH_EAX;
    instruction_state.connect.insTable[0x51] = (core_machine_cpu_instruction_handler) PUSH_ECX;
    instruction_state.connect.insTable[0x52] = (core_machine_cpu_instruction_handler) PUSH_EDX;
    instruction_state.connect.insTable[0x53] = (core_machine_cpu_instruction_handler) PUSH_EBX;
    instruction_state.connect.insTable[0x54] = (core_machine_cpu_instruction_handler) PUSH_ESP;
    instruction_state.connect.insTable[0x55] = (core_machine_cpu_instruction_handler) PUSH_EBP;
    instruction_state.connect.insTable[0x56] = (core_machine_cpu_instruction_handler) PUSH_ESI;
    instruction_state.connect.insTable[0x57] = (core_machine_cpu_instruction_handler) PUSH_EDI;
    instruction_state.connect.insTable[0x58] = (core_machine_cpu_instruction_handler) POP_EAX;
    instruction_state.connect.insTable[0x59] = (core_machine_cpu_instruction_handler) POP_ECX;
    instruction_state.connect.insTable[0x5a] = (core_machine_cpu_instruction_handler) POP_EDX;
    instruction_state.connect.insTable[0x5b] = (core_machine_cpu_instruction_handler) POP_EBX;
    instruction_state.connect.insTable[0x5c] = (core_machine_cpu_instruction_handler) POP_ESP;
    instruction_state.connect.insTable[0x5d] = (core_machine_cpu_instruction_handler) POP_EBP;
    instruction_state.connect.insTable[0x5e] = (core_machine_cpu_instruction_handler) POP_ESI;
    instruction_state.connect.insTable[0x5f] = (core_machine_cpu_instruction_handler) POP_EDI;
    instruction_state.connect.insTable[0x60] = (core_machine_cpu_instruction_handler) PUSHA;
    instruction_state.connect.insTable[0x61] = (core_machine_cpu_instruction_handler) POPA;
    instruction_state.connect.insTable[0x62] = (core_machine_cpu_instruction_handler) BOUND_R16_M16_16;
    instruction_state.connect.insTable[0x63] = (core_machine_cpu_instruction_handler) ARPL_RM16_R16;
    instruction_state.connect.insTable[0x64] = (core_machine_cpu_instruction_handler) PREFIX_FS;
    instruction_state.connect.insTable[0x65] = (core_machine_cpu_instruction_handler) PREFIX_GS;
    instruction_state.connect.insTable[0x66] = (core_machine_cpu_instruction_handler) PREFIX_OprSize;
    instruction_state.connect.insTable[0x67] = (core_machine_cpu_instruction_handler) PREFIX_AddrSize;
    instruction_state.connect.insTable[0x68] = (core_machine_cpu_instruction_handler) PUSH_I32;
    instruction_state.connect.insTable[0x69] = (core_machine_cpu_instruction_handler) IMUL_R32_RM32_I32;
    instruction_state.connect.insTable[0x6a] = (core_machine_cpu_instruction_handler) PUSH_I8;
    instruction_state.connect.insTable[0x6b] = (core_machine_cpu_instruction_handler) IMUL_R32_RM32_I8;
    instruction_state.connect.insTable[0x6c] = (core_machine_cpu_instruction_handler) INSB;
    instruction_state.connect.insTable[0x6d] = (core_machine_cpu_instruction_handler) INSW;
    instruction_state.connect.insTable[0x6e] = (core_machine_cpu_instruction_handler) OUTSB;
    instruction_state.connect.insTable[0x6f] = (core_machine_cpu_instruction_handler) OUTSW;
    instruction_state.connect.insTable[0x70] = (core_machine_cpu_instruction_handler) JO_REL8;
    instruction_state.connect.insTable[0x71] = (core_machine_cpu_instruction_handler) JNO_REL8;
    instruction_state.connect.insTable[0x72] = (core_machine_cpu_instruction_handler) JC_REL8;
    instruction_state.connect.insTable[0x73] = (core_machine_cpu_instruction_handler) JNC_REL8;
    instruction_state.connect.insTable[0x74] = (core_machine_cpu_instruction_handler) JZ_REL8;
    instruction_state.connect.insTable[0x75] = (core_machine_cpu_instruction_handler) JNZ_REL8;
    instruction_state.connect.insTable[0x76] = (core_machine_cpu_instruction_handler) JNA_REL8;
    instruction_state.connect.insTable[0x77] = (core_machine_cpu_instruction_handler) JA_REL8;
    instruction_state.connect.insTable[0x78] = (core_machine_cpu_instruction_handler) JS_REL8;
    instruction_state.connect.insTable[0x79] = (core_machine_cpu_instruction_handler) JNS_REL8;
    instruction_state.connect.insTable[0x7a] = (core_machine_cpu_instruction_handler) JP_REL8;
    instruction_state.connect.insTable[0x7b] = (core_machine_cpu_instruction_handler) JNP_REL8;
    instruction_state.connect.insTable[0x7c] = (core_machine_cpu_instruction_handler) JL_REL8;
    instruction_state.connect.insTable[0x7d] = (core_machine_cpu_instruction_handler) JNL_REL8;
    instruction_state.connect.insTable[0x7e] = (core_machine_cpu_instruction_handler) JNG_REL8;
    instruction_state.connect.insTable[0x7f] = (core_machine_cpu_instruction_handler) JG_REL8;
    instruction_state.connect.insTable[0x80] = (core_machine_cpu_instruction_handler) INS_80;
    instruction_state.connect.insTable[0x81] = (core_machine_cpu_instruction_handler) INS_81;
    instruction_state.connect.insTable[0x82] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable[0x83] = (core_machine_cpu_instruction_handler) INS_83;
    instruction_state.connect.insTable[0x84] = (core_machine_cpu_instruction_handler) TEST_RM8_R8;
    instruction_state.connect.insTable[0x85] = (core_machine_cpu_instruction_handler) TEST_RM32_R32;
    instruction_state.connect.insTable[0x86] = (core_machine_cpu_instruction_handler) XCHG_RM8_R8;
    instruction_state.connect.insTable[0x87] = (core_machine_cpu_instruction_handler) XCHG_RM32_R32;
    instruction_state.connect.insTable[0x88] = (core_machine_cpu_instruction_handler) MOV_RM8_R8;
    instruction_state.connect.insTable[0x89] = (core_machine_cpu_instruction_handler) MOV_RM32_R32;
    instruction_state.connect.insTable[0x8a] = (core_machine_cpu_instruction_handler) MOV_R8_RM8;
    instruction_state.connect.insTable[0x8b] = (core_machine_cpu_instruction_handler) MOV_R32_RM32;
    instruction_state.connect.insTable[0x8c] = (core_machine_cpu_instruction_handler) MOV_RM16_SREG;
    instruction_state.connect.insTable[0x8d] = (core_machine_cpu_instruction_handler) LEA_R32_M32;
    instruction_state.connect.insTable[0x8e] = (core_machine_cpu_instruction_handler) MOV_SREG_RM16;
    instruction_state.connect.insTable[0x8f] = (core_machine_cpu_instruction_handler) INS_8F;
    instruction_state.connect.insTable[0x90] = (core_machine_cpu_instruction_handler) NOP;
    instruction_state.connect.insTable[0x91] = (core_machine_cpu_instruction_handler) XCHG_ECX_EAX;
    instruction_state.connect.insTable[0x92] = (core_machine_cpu_instruction_handler) XCHG_EDX_EAX;
    instruction_state.connect.insTable[0x93] = (core_machine_cpu_instruction_handler) XCHG_EBX_EAX;
    instruction_state.connect.insTable[0x94] = (core_machine_cpu_instruction_handler) XCHG_ESP_EAX;
    instruction_state.connect.insTable[0x95] = (core_machine_cpu_instruction_handler) XCHG_EBP_EAX;
    instruction_state.connect.insTable[0x96] = (core_machine_cpu_instruction_handler) XCHG_ESI_EAX;
    instruction_state.connect.insTable[0x97] = (core_machine_cpu_instruction_handler) XCHG_EDI_EAX;
    instruction_state.connect.insTable[0x98] = (core_machine_cpu_instruction_handler) CBW;
    instruction_state.connect.insTable[0x99] = (core_machine_cpu_instruction_handler) CWD;
    instruction_state.connect.insTable[0x9a] = (core_machine_cpu_instruction_handler) CALL_PTR16_32;
    instruction_state.connect.insTable[0x9b] = (core_machine_cpu_instruction_handler) WAIT;
    instruction_state.connect.insTable[0x9c] = (core_machine_cpu_instruction_handler) PUSHF;
    instruction_state.connect.insTable[0x9d] = (core_machine_cpu_instruction_handler) POPF;
    instruction_state.connect.insTable[0x9e] = (core_machine_cpu_instruction_handler) SAHF;
    instruction_state.connect.insTable[0x9f] = (core_machine_cpu_instruction_handler) LAHF;
    instruction_state.connect.insTable[0xa0] = (core_machine_cpu_instruction_handler) MOV_AL_MOFFS8;
    instruction_state.connect.insTable[0xa1] = (core_machine_cpu_instruction_handler) MOV_EAX_MOFFS32;
    instruction_state.connect.insTable[0xa2] = (core_machine_cpu_instruction_handler) MOV_MOFFS8_AL;
    instruction_state.connect.insTable[0xa3] = (core_machine_cpu_instruction_handler) MOV_MOFFS32_EAX;
    instruction_state.connect.insTable[0xa4] = (core_machine_cpu_instruction_handler) MOVSB;
    instruction_state.connect.insTable[0xa5] = (core_machine_cpu_instruction_handler) MOVSW;
    instruction_state.connect.insTable[0xa6] = (core_machine_cpu_instruction_handler) CMPSB;
    instruction_state.connect.insTable[0xa7] = (core_machine_cpu_instruction_handler) CMPSW;
    instruction_state.connect.insTable[0xa8] = (core_machine_cpu_instruction_handler) TEST_AL_I8;
    instruction_state.connect.insTable[0xa9] = (core_machine_cpu_instruction_handler) TEST_EAX_I32;
    instruction_state.connect.insTable[0xaa] = (core_machine_cpu_instruction_handler) STOSB;
    instruction_state.connect.insTable[0xab] = (core_machine_cpu_instruction_handler) STOSW;
    instruction_state.connect.insTable[0xac] = (core_machine_cpu_instruction_handler) LODSB;
    instruction_state.connect.insTable[0xad] = (core_machine_cpu_instruction_handler) LODSW;
    instruction_state.connect.insTable[0xae] = (core_machine_cpu_instruction_handler) SCASB;
    instruction_state.connect.insTable[0xaf] = (core_machine_cpu_instruction_handler) SCASW;
    instruction_state.connect.insTable[0xb0] = (core_machine_cpu_instruction_handler) MOV_AL_I8;
    instruction_state.connect.insTable[0xb1] = (core_machine_cpu_instruction_handler) MOV_CL_I8;
    instruction_state.connect.insTable[0xb2] = (core_machine_cpu_instruction_handler) MOV_DL_I8;
    instruction_state.connect.insTable[0xb3] = (core_machine_cpu_instruction_handler) MOV_BL_I8;
    instruction_state.connect.insTable[0xb4] = (core_machine_cpu_instruction_handler) MOV_AH_I8;
    instruction_state.connect.insTable[0xb5] = (core_machine_cpu_instruction_handler) MOV_CH_I8;
    instruction_state.connect.insTable[0xb6] = (core_machine_cpu_instruction_handler) MOV_DH_I8;
    instruction_state.connect.insTable[0xb7] = (core_machine_cpu_instruction_handler) MOV_BH_I8;
    instruction_state.connect.insTable[0xb8] = (core_machine_cpu_instruction_handler) MOV_EAX_I32;
    instruction_state.connect.insTable[0xb9] = (core_machine_cpu_instruction_handler) MOV_ECX_I32;
    instruction_state.connect.insTable[0xba] = (core_machine_cpu_instruction_handler) MOV_EDX_I32;
    instruction_state.connect.insTable[0xbb] = (core_machine_cpu_instruction_handler) MOV_EBX_I32;
    instruction_state.connect.insTable[0xbc] = (core_machine_cpu_instruction_handler) MOV_ESP_I32;
    instruction_state.connect.insTable[0xbd] = (core_machine_cpu_instruction_handler) MOV_EBP_I32;
    instruction_state.connect.insTable[0xbe] = (core_machine_cpu_instruction_handler) MOV_ESI_I32;
    instruction_state.connect.insTable[0xbf] = (core_machine_cpu_instruction_handler) MOV_EDI_I32;
    instruction_state.connect.insTable[0xc0] = (core_machine_cpu_instruction_handler) INS_C0;
    instruction_state.connect.insTable[0xc1] = (core_machine_cpu_instruction_handler) INS_C1;
    instruction_state.connect.insTable[0xc2] = (core_machine_cpu_instruction_handler) RET_I16;
    instruction_state.connect.insTable[0xc3] = (core_machine_cpu_instruction_handler) RET;
    instruction_state.connect.insTable[0xc4] = (core_machine_cpu_instruction_handler) LES_R32_M16_32;
    instruction_state.connect.insTable[0xc5] = (core_machine_cpu_instruction_handler) LDS_R32_M16_32;
    instruction_state.connect.insTable[0xc6] = (core_machine_cpu_instruction_handler) INS_C6;
    instruction_state.connect.insTable[0xc7] = (core_machine_cpu_instruction_handler) INS_C7;
    instruction_state.connect.insTable[0xc8] = (core_machine_cpu_instruction_handler) ENTER;
    instruction_state.connect.insTable[0xc9] = (core_machine_cpu_instruction_handler) LEAVE;
    instruction_state.connect.insTable[0xca] = (core_machine_cpu_instruction_handler) RETF_I16;
    instruction_state.connect.insTable[0xcb] = (core_machine_cpu_instruction_handler) RETF;
    instruction_state.connect.insTable[0xcc] = (core_machine_cpu_instruction_handler) INT3;
    instruction_state.connect.insTable[0xcd] = (core_machine_cpu_instruction_handler) INT_I8;
    instruction_state.connect.insTable[0xce] = (core_machine_cpu_instruction_handler) INTO;
    instruction_state.connect.insTable[0xcf] = (core_machine_cpu_instruction_handler) IRET;
    instruction_state.connect.insTable[0xd0] = (core_machine_cpu_instruction_handler) INS_D0;
    instruction_state.connect.insTable[0xd1] = (core_machine_cpu_instruction_handler) INS_D1;
    instruction_state.connect.insTable[0xd2] = (core_machine_cpu_instruction_handler) INS_D2;
    instruction_state.connect.insTable[0xd3] = (core_machine_cpu_instruction_handler) INS_D3;
    instruction_state.connect.insTable[0xd4] = (core_machine_cpu_instruction_handler) AAM;
    instruction_state.connect.insTable[0xd5] = (core_machine_cpu_instruction_handler) AAD;
    instruction_state.connect.insTable[0xd6] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable[0xd7] = (core_machine_cpu_instruction_handler) XLAT;
    instruction_state.connect.insTable[0xd8] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable[0xd9] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    /* instruction_state.connect.insTable[0xd9] = (core_machine_cpu_instruction_handler) INS_D9; */
    instruction_state.connect.insTable[0xda] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable[0xdb] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    /* instruction_state.connect.insTable[0xdb] = (core_machine_cpu_instruction_handler) INS_DB; */
    instruction_state.connect.insTable[0xdc] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable[0xdd] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable[0xde] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable[0xdf] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable[0xe0] = (core_machine_cpu_instruction_handler) LOOPNZ_REL8;
    instruction_state.connect.insTable[0xe1] = (core_machine_cpu_instruction_handler) LOOPZ_REL8;
    instruction_state.connect.insTable[0xe2] = (core_machine_cpu_instruction_handler) LOOP_REL8;
    instruction_state.connect.insTable[0xe3] = (core_machine_cpu_instruction_handler) JCXZ_REL8;
    instruction_state.connect.insTable[0xe4] = (core_machine_cpu_instruction_handler) IN_AL_I8;
    instruction_state.connect.insTable[0xe5] = (core_machine_cpu_instruction_handler) IN_EAX_I8;
    instruction_state.connect.insTable[0xe6] = (core_machine_cpu_instruction_handler) OUT_I8_AL;
    instruction_state.connect.insTable[0xe7] = (core_machine_cpu_instruction_handler) OUT_I8_EAX;
    instruction_state.connect.insTable[0xe8] = (core_machine_cpu_instruction_handler) CALL_REL32;
    instruction_state.connect.insTable[0xe9] = (core_machine_cpu_instruction_handler) JMP_REL32;
    instruction_state.connect.insTable[0xea] = (core_machine_cpu_instruction_handler) JMP_PTR16_32;
    instruction_state.connect.insTable[0xeb] = (core_machine_cpu_instruction_handler) JMP_REL8;
    instruction_state.connect.insTable[0xec] = (core_machine_cpu_instruction_handler) IN_AL_DX;
    instruction_state.connect.insTable[0xed] = (core_machine_cpu_instruction_handler) IN_EAX_DX;
    instruction_state.connect.insTable[0xee] = (core_machine_cpu_instruction_handler) OUT_DX_AL;
    instruction_state.connect.insTable[0xef] = (core_machine_cpu_instruction_handler) OUT_DX_EAX;
    instruction_state.connect.insTable[0xf0] = (core_machine_cpu_instruction_handler) PREFIX_LOCK;
    instruction_state.connect.insTable[0xf1] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable[0xf2] = (core_machine_cpu_instruction_handler) PREFIX_REPNZ;
    instruction_state.connect.insTable[0xf3] = (core_machine_cpu_instruction_handler) PREFIX_REPZ;
    instruction_state.connect.insTable[0xf4] = (core_machine_cpu_instruction_handler) HLT;
    instruction_state.connect.insTable[0xf5] = (core_machine_cpu_instruction_handler) CMC;
    instruction_state.connect.insTable[0xf6] = (core_machine_cpu_instruction_handler) INS_F6;
    instruction_state.connect.insTable[0xf7] = (core_machine_cpu_instruction_handler) INS_F7;
    instruction_state.connect.insTable[0xf8] = (core_machine_cpu_instruction_handler) CLC;
    instruction_state.connect.insTable[0xf9] = (core_machine_cpu_instruction_handler) STC;
    instruction_state.connect.insTable[0xfa] = (core_machine_cpu_instruction_handler) CLI;
    instruction_state.connect.insTable[0xfb] = (core_machine_cpu_instruction_handler) STI;
    instruction_state.connect.insTable[0xfc] = (core_machine_cpu_instruction_handler) CLD;
    instruction_state.connect.insTable[0xfd] = (core_machine_cpu_instruction_handler) STD;
    instruction_state.connect.insTable[0xfe] = (core_machine_cpu_instruction_handler) INS_FE;
    instruction_state.connect.insTable[0xff] = (core_machine_cpu_instruction_handler) INS_FF;
    instruction_state.connect.insTable_0f[0x00] = (core_machine_cpu_instruction_handler) INS_0F_00;
    instruction_state.connect.insTable_0f[0x01] = (core_machine_cpu_instruction_handler) INS_0F_01;
    instruction_state.connect.insTable_0f[0x02] = (core_machine_cpu_instruction_handler) LAR_R32_RM32;
    instruction_state.connect.insTable_0f[0x03] = (core_machine_cpu_instruction_handler) LSL_R32_RM32;
    instruction_state.connect.insTable_0f[0x04] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x05] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x06] = (core_machine_cpu_instruction_handler) CLTS;
    instruction_state.connect.insTable_0f[0x07] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x08] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x09] = (core_machine_cpu_instruction_handler) WBINVD;
    instruction_state.connect.insTable_0f[0x0a] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x0b] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x0c] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x0d] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x0e] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x0f] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x10] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x11] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x12] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x13] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x14] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x15] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x16] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x17] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x18] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x19] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x1a] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x1b] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x1c] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x1d] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x1e] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x1f] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x20] = (core_machine_cpu_instruction_handler) MOV_R32_CR;
    instruction_state.connect.insTable_0f[0x21] = (core_machine_cpu_instruction_handler) MOV_R32_DR;
    instruction_state.connect.insTable_0f[0x22] = (core_machine_cpu_instruction_handler) MOV_CR_R32;
    instruction_state.connect.insTable_0f[0x23] = (core_machine_cpu_instruction_handler) MOV_DR_R32;
    instruction_state.connect.insTable_0f[0x24] = (core_machine_cpu_instruction_handler) MOV_R32_TR;
    instruction_state.connect.insTable_0f[0x25] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x26] = (core_machine_cpu_instruction_handler) MOV_TR_R32;
    instruction_state.connect.insTable_0f[0x27] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x28] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x29] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x2a] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x2b] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x2c] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x2d] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x2e] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x2f] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x30] = (core_machine_cpu_instruction_handler) WRMSR;
    instruction_state.connect.insTable_0f[0x31] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x32] = (core_machine_cpu_instruction_handler) RDMSR;
    instruction_state.connect.insTable_0f[0x33] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x34] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x35] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x36] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x37] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x38] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x39] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x3a] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x3b] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x3c] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x3d] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x3e] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x3f] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x40] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x41] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x42] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x43] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x44] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x45] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x46] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x47] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x48] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x49] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x4a] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x4b] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x4c] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x4d] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x4e] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x4f] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x50] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x51] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x52] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x53] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x54] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x55] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x56] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x57] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x58] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x59] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x5a] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x5b] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x5c] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x5d] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x5e] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x5f] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x60] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x61] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x62] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x63] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x64] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x65] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x66] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x67] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x68] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x69] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x6a] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x6b] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x6c] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x6d] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x6e] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x6f] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x70] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x71] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x72] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x73] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x74] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x75] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x76] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x77] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x78] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x79] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x7a] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x7b] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x7c] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x7d] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x7e] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x7f] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x80] = (core_machine_cpu_instruction_handler) JO_REL32;
    instruction_state.connect.insTable_0f[0x81] = (core_machine_cpu_instruction_handler) JNO_REL32;
    instruction_state.connect.insTable_0f[0x82] = (core_machine_cpu_instruction_handler) JC_REL32;
    instruction_state.connect.insTable_0f[0x83] = (core_machine_cpu_instruction_handler) JNC_REL32;
    instruction_state.connect.insTable_0f[0x84] = (core_machine_cpu_instruction_handler) JZ_REL32;
    instruction_state.connect.insTable_0f[0x85] = (core_machine_cpu_instruction_handler) JNZ_REL32;
    instruction_state.connect.insTable_0f[0x86] = (core_machine_cpu_instruction_handler) JNA_REL32;
    instruction_state.connect.insTable_0f[0x87] = (core_machine_cpu_instruction_handler) JA_REL32;
    instruction_state.connect.insTable_0f[0x88] = (core_machine_cpu_instruction_handler) JS_REL32;
    instruction_state.connect.insTable_0f[0x89] = (core_machine_cpu_instruction_handler) JNS_REL32;
    instruction_state.connect.insTable_0f[0x8a] = (core_machine_cpu_instruction_handler) JP_REL32;
    instruction_state.connect.insTable_0f[0x8b] = (core_machine_cpu_instruction_handler) JNP_REL32;
    instruction_state.connect.insTable_0f[0x8c] = (core_machine_cpu_instruction_handler) JL_REL32;
    instruction_state.connect.insTable_0f[0x8d] = (core_machine_cpu_instruction_handler) JNL_REL32;
    instruction_state.connect.insTable_0f[0x8e] = (core_machine_cpu_instruction_handler) JNG_REL32;
    instruction_state.connect.insTable_0f[0x8f] = (core_machine_cpu_instruction_handler) JG_REL32;
    instruction_state.connect.insTable_0f[0x90] = (core_machine_cpu_instruction_handler) SETO_RM8;
    instruction_state.connect.insTable_0f[0x91] = (core_machine_cpu_instruction_handler) SETNO_RM8;
    instruction_state.connect.insTable_0f[0x92] = (core_machine_cpu_instruction_handler) SETC_RM8;
    instruction_state.connect.insTable_0f[0x93] = (core_machine_cpu_instruction_handler) SETNC_RM8;
    instruction_state.connect.insTable_0f[0x94] = (core_machine_cpu_instruction_handler) SETZ_RM8;
    instruction_state.connect.insTable_0f[0x95] = (core_machine_cpu_instruction_handler) SETNZ_RM8;
    instruction_state.connect.insTable_0f[0x96] = (core_machine_cpu_instruction_handler) SETNA_RM8;
    instruction_state.connect.insTable_0f[0x97] = (core_machine_cpu_instruction_handler) SETA_RM8;
    instruction_state.connect.insTable_0f[0x98] = (core_machine_cpu_instruction_handler) SETS_RM8;
    instruction_state.connect.insTable_0f[0x99] = (core_machine_cpu_instruction_handler) SETNS_RM8;
    instruction_state.connect.insTable_0f[0x9a] = (core_machine_cpu_instruction_handler) SETP_RM8;
    instruction_state.connect.insTable_0f[0x9b] = (core_machine_cpu_instruction_handler) SETNP_RM8;
    instruction_state.connect.insTable_0f[0x9c] = (core_machine_cpu_instruction_handler) SETL_RM8;
    instruction_state.connect.insTable_0f[0x9d] = (core_machine_cpu_instruction_handler) SETNL_RM8;
    instruction_state.connect.insTable_0f[0x9e] = (core_machine_cpu_instruction_handler) SETNG_RM8;
    instruction_state.connect.insTable_0f[0x9f] = (core_machine_cpu_instruction_handler) SETG_RM8;
    instruction_state.connect.insTable_0f[0xa0] = (core_machine_cpu_instruction_handler) PUSH_FS;
    instruction_state.connect.insTable_0f[0xa1] = (core_machine_cpu_instruction_handler) POP_FS;
    instruction_state.connect.insTable_0f[0xa2] = (core_machine_cpu_instruction_handler) CPUID;
    instruction_state.connect.insTable_0f[0xa3] = (core_machine_cpu_instruction_handler) BT_RM32_R32;
    instruction_state.connect.insTable_0f[0xa4] = (core_machine_cpu_instruction_handler) SHLD_RM32_R32_I8;
    instruction_state.connect.insTable_0f[0xa5] = (core_machine_cpu_instruction_handler) SHLD_RM32_R32_CL;
    instruction_state.connect.insTable_0f[0xa6] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xa7] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xa8] = (core_machine_cpu_instruction_handler) PUSH_GS;
    instruction_state.connect.insTable_0f[0xa9] = (core_machine_cpu_instruction_handler) POP_GS;
    instruction_state.connect.insTable_0f[0xaa] = (core_machine_cpu_instruction_handler) RSM;
    instruction_state.connect.insTable_0f[0xab] = (core_machine_cpu_instruction_handler) BTS_RM32_R32;
    instruction_state.connect.insTable_0f[0xac] = (core_machine_cpu_instruction_handler) SHRD_RM32_R32_I8;
    instruction_state.connect.insTable_0f[0xad] = (core_machine_cpu_instruction_handler) SHRD_RM32_R32_CL;
    instruction_state.connect.insTable_0f[0xae] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xaf] = (core_machine_cpu_instruction_handler) IMUL_R32_RM32;
    instruction_state.connect.insTable_0f[0xb0] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xb1] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xb2] = (core_machine_cpu_instruction_handler) LSS_R32_M16_32;
    instruction_state.connect.insTable_0f[0xb3] = (core_machine_cpu_instruction_handler) BTR_RM32_R32;
    instruction_state.connect.insTable_0f[0xb4] = (core_machine_cpu_instruction_handler) LFS_R32_M16_32;
    instruction_state.connect.insTable_0f[0xb5] = (core_machine_cpu_instruction_handler) LGS_R32_M16_32;
    instruction_state.connect.insTable_0f[0xb6] = (core_machine_cpu_instruction_handler) MOVZX_R32_RM8;
    instruction_state.connect.insTable_0f[0xb7] = (core_machine_cpu_instruction_handler) MOVZX_R32_RM16;
    instruction_state.connect.insTable_0f[0xb8] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xb9] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xba] = (core_machine_cpu_instruction_handler) INS_0F_BA;
    instruction_state.connect.insTable_0f[0xbb] = (core_machine_cpu_instruction_handler) BTC_RM32_R32;
    instruction_state.connect.insTable_0f[0xbc] = (core_machine_cpu_instruction_handler) BSF_R32_RM32;
    instruction_state.connect.insTable_0f[0xbd] = (core_machine_cpu_instruction_handler) BSR_R32_RM32;
    instruction_state.connect.insTable_0f[0xbe] = (core_machine_cpu_instruction_handler) MOVSX_R32_RM8;
    instruction_state.connect.insTable_0f[0xbf] = (core_machine_cpu_instruction_handler) MOVSX_R32_RM16;
    instruction_state.connect.insTable_0f[0xc0] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc1] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc2] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc3] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc4] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc5] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc6] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc7] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc8] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc9] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xca] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xcb] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xcc] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xcd] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xce] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xcf] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd0] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd1] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd2] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd3] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd4] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd5] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd6] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd7] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd8] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd9] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xda] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xdb] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xdc] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xdd] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xde] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xdf] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe0] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe1] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe2] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe3] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe4] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe5] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe6] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe7] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe8] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe9] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xea] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xeb] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xec] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xed] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xee] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xef] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf0] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf1] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf2] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf3] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf4] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf5] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf6] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf7] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf8] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf9] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xfa] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xfb] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xfc] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xfd] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xfe] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xff] = (core_machine_cpu_instruction_handler) UndefinedOpcode;
}
void core_machine_cpu_execution_reset(
    core_machine_cpu_execution_context *context) {
    MEMSET((void *)(&instruction_state.data), Zero8, sizeof(t_cpuins_data));
}
void core_machine_cpu_execution_refresh(
    core_machine_cpu_execution_context *context) {
    if (!cpu_state.data.flagHalt) {
        ExecIns(context);
    }
    ExecInt(context);
}
void core_machine_cpu_execution_finalize(
    core_machine_cpu_execution_context *context) { (void)context; }
