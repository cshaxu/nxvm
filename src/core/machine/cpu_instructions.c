/* This file is a part of NXVM project. */

/* DEBUGGING OPTIONS ******************************************************* */
/* T154 retains this compatibility shim; T155 replaces it with profile gates. */
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

#define NTVDM64_TYPE_TRACE_CONTEXT    (*context->trace)
#define NTVDM64_TYPE_TRACE_ERROR  instruction_state.data.except
#define NTVDM64_TYPE_TRACE_SET_ERROR (_SetExcept_CE(0xffffffff))

/* indicates functions not implemented */
#define _______todo static C_VOID
/* prints untested code path */
#define _new_code_path_ do { \
STD_PRINTF("NEW CODE PATH\n");if (context->trace != STD_NULL) ntvdm64_type_trace_print(context->trace);} while (0)

/* stack pointer size */
#define _GetStackSize   (cpu_state.data.ss.seg.data.big ? 4 : 2)
/* operand size */
#define _GetOperandSize ((cpu_state.data.cs.seg.exec.defsize ^ instruction_state.data.prefix_oprsize) ? 4 : 2)
/* address size of the source operand */
#define _GetAddressSize ((cpu_state.data.cs.seg.exec.defsize ^ instruction_state.data.prefix_addrsize) ? 4 : 2)
/* if opcode indicates a prefix */
#define _SetExcept_DE(n) (NTVDM64_TYPE_SET_BIT(instruction_state.data.except, VCPUINS_EXCEPT_DE), instruction_state.data.excode = (n), STD_PRINTF("#DE(%x) - divide error\n",    instruction_state.data.excode))
#define _SetExcept_PF(n) (NTVDM64_TYPE_SET_BIT(instruction_state.data.except, VCPUINS_EXCEPT_PF), instruction_state.data.excode = (n), STD_PRINTF("#PF(%x) - page fault\n",      instruction_state.data.excode))
#define _SetExcept_GP(n) (NTVDM64_TYPE_SET_BIT(instruction_state.data.except, VCPUINS_EXCEPT_GP), instruction_state.data.excode = (n), STD_PRINTF("#GP(%x) - general protect\n", instruction_state.data.excode))
#define _SetExcept_SS(n) (NTVDM64_TYPE_SET_BIT(instruction_state.data.except, VCPUINS_EXCEPT_SS), instruction_state.data.excode = (n), STD_PRINTF("#SS(%x) - stack segment\n",   instruction_state.data.excode))
#define _SetExcept_UD(n) (NTVDM64_TYPE_SET_BIT(instruction_state.data.except, VCPUINS_EXCEPT_UD), instruction_state.data.excode = (n), STD_PRINTF("#UD(%x) - undefined\n",       instruction_state.data.excode))
#define _SetExcept_NP(n) (NTVDM64_TYPE_SET_BIT(instruction_state.data.except, VCPUINS_EXCEPT_NP), instruction_state.data.excode = (n), STD_PRINTF("#NP(%x) - not present\n",     instruction_state.data.excode))
#define _SetExcept_BR(n) (NTVDM64_TYPE_SET_BIT(instruction_state.data.except, VCPUINS_EXCEPT_BR), instruction_state.data.excode = (n), STD_PRINTF("#BR(%x) - boundary\n",        instruction_state.data.excode))
#define _SetExcept_TS(n) (NTVDM64_TYPE_SET_BIT(instruction_state.data.except, VCPUINS_EXCEPT_TS), instruction_state.data.excode = (n), STD_PRINTF("#TS(%x) - task state\n",      instruction_state.data.excode))
#define _SetExcept_NM(n) (NTVDM64_TYPE_SET_BIT(instruction_state.data.except, VCPUINS_EXCEPT_NM), instruction_state.data.excode = (n), STD_PRINTF("#NM(%x) - divide error\n",    instruction_state.data.excode))
#define _SetExcept_CE(n) (NTVDM64_TYPE_SET_BIT(instruction_state.data.except, VCPUINS_EXCEPT_CE), instruction_state.data.excode = (n), STD_PRINTF("#CE(%x) - internal error\n",  instruction_state.data.excode))

/* memory management unit */
/* kernel memory accessing */
/* read content from reference */
static C_VOID _kma_read_ref(core_machine_cpu_execution_context *context, ntvdm64_type_virtual_address ref, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kma_read_ref");
    STD_MEMCPY((C_VOID *) rdata, (C_VOID *) ref, byte);
    NTVDM64_TYPE_TRACE_CALL_END;
}
/* write content to reference */
static C_VOID _kma_write_ref(core_machine_cpu_execution_context *context, ntvdm64_type_virtual_address ref, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kma_write_ref");
    STD_MEMCPY((C_VOID *) ref, (C_VOID *) rdata, byte);
    NTVDM64_TYPE_TRACE_CALL_END;
}
/* read content from physical */
static C_VOID _kma_read_physical(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 physical, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kma_read_physical");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(core_machine_memory_read_physical(context->memory, physical, rdata,
        byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
/* write content to physical */
static C_VOID _kma_write_physical(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 physical, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kma_write_physical");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(core_machine_memory_write_physical(context->memory, physical, rdata,
        byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
/* translate linear to physical - paging mechanism*/
static ntvdm64_type_unsigned_32 _kma_physical_linear(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 linear, ntvdm64_type_unsigned_8 byte, ntvdm64_type_bool write, ntvdm64_type_unsigned_8 vpl) {
    ntvdm64_type_unsigned_32 ppde, ppte; /* page table entries */
    ntvdm64_type_unsigned_32 cpde, cpte;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_t_kma_physical_linear");
    if (_GetLinear_Offset(linear) > NTVDM64_TYPE_MASK_UNSIGNED_32(_GetPageSize - byte)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
    if (!_IsPaging) {
        NTVDM64_TYPE_TRACE_CALL_END;
        return linear;
    }
    ppde = _GetCR3_Base + _GetLinear_Dir(linear) * 4;
    NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_kma_read_physical(context, ppde, NTVDM64_TYPE_REFERENCE_OF(cpde), 4));
    if (!_IsPageEntryPresent(cpde)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!PageDirEntryPresent");
        cpu_state.data.cr2 = linear;
        NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_PF(_MakePageFaultErrorCode(0, write, (vpl == 3))));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    if (vpl == 0x03) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("vpl(3)");
        if (!_GetPageEntry_US(cpde)) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("PageDirEntry_US(0)");
            cpu_state.data.cr2 = linear;
            NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_PF(_MakePageFaultErrorCode(1, write, 1)));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        if (write && !_IsPageEntryWritable(cpde)) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("write,!PageDirEntryWritable");
            cpu_state.data.cr2 = linear;
            NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_PF(_MakePageFaultErrorCode(1, 1, 1)));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    _SetPageEntry_A(cpde);
    NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_kma_write_physical(context, ppde, NTVDM64_TYPE_REFERENCE_OF(cpde), 4));
    ppte = _GetPageEntry_Base(cpde) + _GetLinear_Page(linear) * 4;
    NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_kma_read_physical(context, ppte, NTVDM64_TYPE_REFERENCE_OF(cpte), 4));
    if (!_IsPageEntryPresent(cpte)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!PageTabEntryPresent");
        cpu_state.data.cr2 = linear;
        NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_PF(_MakePageFaultErrorCode(0, write, (vpl == 3))));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    if (vpl == 0x03) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("vpl(3)");
        if (!_GetPageEntry_US(cpte)) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("PageTabEntry_US(0)");
            cpu_state.data.cr2 = linear;
            NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_PF(_MakePageFaultErrorCode(1, write, 1)));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        if (write && !_IsPageEntryWritable(cpte)) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("write,!PageTabEntryWritable");
            cpu_state.data.cr2 = linear;
            NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_PF(_MakePageFaultErrorCode(1, 1, 1)));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    _SetPageEntry_A(cpte);
    if (write) _SetPageEntry_D(cpte);
    NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_kma_write_physical(context, ppte, NTVDM64_TYPE_REFERENCE_OF(cpte), 4));
    NTVDM64_TYPE_TRACE_CALL_END;
    return (_GetPageEntry_Base(cpte) + _GetLinear_Offset(linear));
}
/* translate logical to linear - segmentation mechanism */
static ntvdm64_type_unsigned_32 _kma_linear_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, ntvdm64_type_unsigned_32 offset, ntvdm64_type_unsigned_8 byte, ntvdm64_type_bool write, ntvdm64_type_unsigned_8 vpl, ntvdm64_type_bool force) {
    ntvdm64_type_unsigned_32 linear;
    ntvdm64_type_unsigned_32 upper, lower;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kma_linear_logical");
    switch (rsreg->sregtype) {
    case SREG_CODE:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_CODE)");
        if (!rsreg->flagValid) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
        if (_IsProtected) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Protected");
            if (!force) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("force");
                if (write) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("write");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (!rsreg->seg.exec.readable) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!readable");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        lower = 0x00000000;
        upper = rsreg->limit;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case SREG_STACK:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_STACK)");
        if (!rsreg->flagValid) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
        if (_IsProtected) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Protected");
            if (rsreg->seg.executable || !rsreg->seg.data.writable) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        if (rsreg->seg.data.expdown) {
            lower = rsreg->limit + 1;
            upper = rsreg->seg.data.big ? 0xffffffff : 0x0000ffff;
        } else {
            lower = 0x00000000;
            upper = rsreg->limit;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case SREG_DATA:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_DATA)");
        if (!rsreg->flagValid) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("selector(null)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        if (_IsProtected) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Protected");
            if (_IsSelectorNull(rsreg->selector)) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("selector(null)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            if (rsreg->seg.executable && !rsreg->seg.exec.readable) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
            if (!force) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("force");
                if (write) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("write");
                    if (rsreg->seg.executable) {
                        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("executable");
                        NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
                        NTVDM64_TYPE_TRACE_BLOCK_END;
                    } else {
                        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!executable");
                        if (!rsreg->seg.data.writable) {
                            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!writable");
                            NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
                            NTVDM64_TYPE_TRACE_BLOCK_END;
                        }
                        NTVDM64_TYPE_TRACE_BLOCK_END;
                    }
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        if (rsreg->seg.data.expdown) {
            lower = rsreg->limit + 1;
            upper = rsreg->seg.data.big ? 0xffffffff : 0x0000ffff;
        } else {
            lower = 0x00000000;
            upper = rsreg->limit;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case SREG_GDTR:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_GDTR)");
        if (!_GetCR0_PE) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
        lower = 0x00000000;
        upper = rsreg->limit;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case SREG_IDTR:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_IDTR)");
        lower = 0x00000000;
        upper = rsreg->limit;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case SREG_LDTR:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_LDTR)");
        if (!_GetCR0_PE) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
        if (_GetSelector_TI(rsreg->selector)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
        if (!rsreg->flagValid || _IsSelectorNull(rsreg->selector)) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("selector(null)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        lower = 0x00000000;
        upper = rsreg->limit;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case SREG_TR:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_TR)");
        if (!_GetCR0_PE) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
        if (!rsreg->flagValid || _IsSelectorNull(rsreg->selector)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
        if (_GetSelector_TI(rsreg->selector)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
        lower = 0x00000000;
        upper = rsreg->limit;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
    }
    linear = rsreg->base + offset;
    if (offset < lower || offset > upper - (byte - 1)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("offset(<lower/>upper)");
        switch (rsreg->sregtype) {
        case SREG_STACK:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_STACK)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_SS(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case SREG_TR:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_TR)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_TS(rsreg->selector));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("sregtype(default)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
    return linear;
}
/* read content from logical */
static C_VOID _kma_read_linear(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 linear, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte, ntvdm64_type_unsigned_8 vpl, ntvdm64_type_bool force) {
    ntvdm64_type_unsigned_32 phy1, phy2;
    ntvdm64_type_unsigned_8  byte1, byte2;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kma_read_logical");
    if (_GetLinear_Offset(linear) > NTVDM64_TYPE_MASK_UNSIGNED_32(_GetPageSize - byte)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Linear_Offset(>PageSize)");
        byte1 = _GetPageSize - _GetLinear_Offset(linear);
        byte2 = byte - byte1;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(phy1 = _kma_physical_linear(context, linear        , byte1, 0, vpl));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(phy2 = _kma_physical_linear(context, linear + byte1, byte2, 0, vpl));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_read_physical(context, phy1, rdata        , byte1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_read_physical(context, phy2, rdata + byte1, byte2));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Linear_Offset(<=PageSize)");
        byte1 = byte;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(phy1 = _kma_physical_linear(context, linear, byte1, 0, vpl));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_read_physical(context, phy1, rdata, byte1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
/* write content to logical */
static C_VOID _kma_write_linear(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 linear, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte, ntvdm64_type_unsigned_8 vpl, ntvdm64_type_bool force) {
    ntvdm64_type_unsigned_32 phy1, phy2;
    ntvdm64_type_unsigned_8  byte1, byte2;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kma_write_linear");
    if (_GetLinear_Offset(linear) > NTVDM64_TYPE_MASK_UNSIGNED_32(_GetPageSize - byte)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Linear_Offset(>PageSize)");
        byte1 = _GetPageSize - _GetLinear_Offset(linear);
        byte2 = byte - byte1;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(phy1 = _kma_physical_linear(context, linear, byte1, 1, vpl));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(phy2 = _kma_physical_linear(context, linear + byte1, byte2, 1, vpl));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_write_physical(context, phy1, rdata, byte1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_write_physical(context, phy2, rdata + byte1, byte2));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Linear_Offset(<=PageSize)");
        byte1 = byte;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(phy1 = _kma_physical_linear(context, linear, byte1, 1, vpl));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_write_physical(context, phy1, rdata, byte1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
/* read content from logical */
static C_VOID _kma_read_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, ntvdm64_type_unsigned_32 offset, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte, ntvdm64_type_unsigned_8 vpl, ntvdm64_type_bool force) {
    /* ntvdm64_type_native_unsigned i; */
    ntvdm64_type_unsigned_32 linear;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kma_read_logical");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(linear = _kma_linear_logical(context, rsreg, offset, byte, 0, vpl, force));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_read_linear(context, linear, rdata, byte, vpl, force));
    if (!force) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!force");
        instruction_state.data.mem[instruction_state.data.msize].flagWrite = NTVDM64_TYPE_FALSE;
        instruction_state.data.mem[instruction_state.data.msize].data = 0;
        STD_MEMCPY((C_VOID *) NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.mem[instruction_state.data.msize].data), (C_VOID *) rdata, byte);
        instruction_state.data.mem[instruction_state.data.msize].byte = byte;
        instruction_state.data.mem[instruction_state.data.msize].linear = linear;
        if (instruction_state.data.flagWR) {
            if (instruction_state.data.wrLinear >= instruction_state.data.mem[instruction_state.data.msize].linear &&
                    instruction_state.data.wrLinear < instruction_state.data.mem[instruction_state.data.msize].linear + byte) {
                STD_PRINTF("Watch point caught at L%08x: READ %01x BYTES OF DATA=%08x FROM L%08x\n", instruction_state.data.linear,
                       instruction_state.data.mem[instruction_state.data.msize].byte,
                       instruction_state.data.mem[instruction_state.data.msize].data,
                       instruction_state.data.mem[instruction_state.data.msize].linear);
            }
        }
        /* for (i = 0;i < instruction_state.data.msize;++i) {
            if (instruction_state.data.mem[i].flagWrite == instruction_state.data.mem[instruction_state.data.msize].flagWrite &&
                instruction_state.data.mem[i].linear == instruction_state.data.mem[instruction_state.data.msize].linear) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("mem(same)");
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
                NTVDM64_TYPE_TRACE_CALL_END;
            }
        } */
        instruction_state.data.msize++;
        if (instruction_state.data.msize == 0x20) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
/* write content to logical */
static C_VOID _kma_write_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, ntvdm64_type_unsigned_32 offset, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte, ntvdm64_type_unsigned_8 vpl, ntvdm64_type_bool force) {
    /* ntvdm64_type_native_unsigned i; */
    ntvdm64_type_unsigned_32 linear;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kma_write_logical");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(linear = _kma_linear_logical(context, rsreg, offset, byte, 1, vpl, force));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_write_linear(context, linear, rdata, byte, vpl, force));
    if (!force) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!force");
        instruction_state.data.mem[instruction_state.data.msize].flagWrite = NTVDM64_TYPE_TRUE;
        instruction_state.data.mem[instruction_state.data.msize].data = 0;
        STD_MEMCPY((C_VOID *) NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.mem[instruction_state.data.msize].data), (C_VOID *) rdata, byte);
        instruction_state.data.mem[instruction_state.data.msize].byte = byte;
        instruction_state.data.mem[instruction_state.data.msize].linear = linear;
        if (instruction_state.data.flagWW) {
            if (instruction_state.data.wwLinear >= instruction_state.data.mem[instruction_state.data.msize].linear &&
                    instruction_state.data.wwLinear < instruction_state.data.mem[instruction_state.data.msize].linear + byte) {
                STD_PRINTF("Watch point caught at L%08x: WRITE %01x BYTES OF DATA=%08x TO L%08x\n", instruction_state.data.linear,
                       instruction_state.data.mem[instruction_state.data.msize].byte,
                       instruction_state.data.mem[instruction_state.data.msize].data,
                       instruction_state.data.mem[instruction_state.data.msize].linear);
            }
        }
        /* for (i = 0;i < instruction_state.data.msize;++i) {
            if (instruction_state.data.mem[i].flagWrite == instruction_state.data.mem[instruction_state.data.msize].flagWrite &&
                instruction_state.data.mem[i].linear == instruction_state.data.mem[instruction_state.data.msize].linear) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("mem(same)");
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                NTVDM64_TYPE_TRACE_CALL_END;
            }
        } */
        instruction_state.data.msize++;
        if (instruction_state.data.msize == 0x20) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
/* test logical accessing */
static C_VOID _kma_test_linear(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 linear, ntvdm64_type_unsigned_8 byte, ntvdm64_type_bool write, ntvdm64_type_unsigned_8 vpl, ntvdm64_type_bool force) {
    ntvdm64_type_unsigned_32 phy1, phy2;
    ntvdm64_type_unsigned_8  byte1, byte2;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kma_test_linear");
    if (_GetLinear_Offset(linear) > NTVDM64_TYPE_MASK_UNSIGNED_32(_GetPageSize - byte)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Linear_Offset(>PageSize)");
        byte1 = _GetPageSize - _GetLinear_Offset(linear);
        byte2 = byte - byte1;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(phy1 = _kma_physical_linear(context, linear        , byte1, write, vpl));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(phy2 = _kma_physical_linear(context, linear + byte1, byte2, write, vpl));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Linear_Offset(<=PageSize)");
        byte1 = byte;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(phy1 = _kma_physical_linear(context, linear, byte1, write, vpl));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _kma_test_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, ntvdm64_type_unsigned_32 offset, ntvdm64_type_unsigned_8 byte, ntvdm64_type_bool write, ntvdm64_type_unsigned_8 vpl, ntvdm64_type_bool force) {
    ntvdm64_type_unsigned_32 linear;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kma_test_logical");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(linear = _kma_linear_logical(context, rsreg, offset, byte, write, vpl, force));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _kma_test_access(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, ntvdm64_type_unsigned_32 offset, ntvdm64_type_unsigned_8 byte, ntvdm64_type_bool write, ntvdm64_type_unsigned_8 vpl, ntvdm64_type_bool force) {
    ntvdm64_type_unsigned_32 linear;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kma_test_access");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(linear = _kma_linear_logical(context, rsreg, offset, byte, write, vpl, force));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_test_linear(context, linear, byte, write, vpl, force));
    NTVDM64_TYPE_TRACE_CALL_END;
}

/* general memory accessing */
static C_VOID _m_read_ref(core_machine_cpu_execution_context *context, ntvdm64_type_virtual_address ref, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_m_read_ref");
    /* _m_write_ref allows in-module reads only */
    if (ref < (ntvdm64_type_virtual_address)(&cpu_state) && ref >= (ntvdm64_type_virtual_address)(&cpu_state) + sizeof(t_cpu) &&
            ref < (ntvdm64_type_virtual_address)(&instruction_state) && ref >= (ntvdm64_type_virtual_address)(&instruction_state) + sizeof(t_cpuins)) {
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_read_ref(context, ref, rdata, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _m_write_ref(core_machine_cpu_execution_context *context, ntvdm64_type_virtual_address ref, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_m_write_ref");
    /* _m_write_ref allows in-module writes only */
    if (ref < (ntvdm64_type_virtual_address)(&cpu_state) && ref >= (ntvdm64_type_virtual_address)(&cpu_state) + sizeof(t_cpu) &&
            ref < (ntvdm64_type_virtual_address)(&instruction_state) && ref >= (ntvdm64_type_virtual_address)(&instruction_state) + sizeof(t_cpuins)) {
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_write_ref(context, ref, rdata, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _m_read_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, ntvdm64_type_unsigned_32 offset, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_m_read_logical");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_read_logical(context, rsreg, offset, rdata, byte, _GetCPL, 0));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _m_write_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, ntvdm64_type_unsigned_32 offset, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_m_write_logical");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_write_logical(context, rsreg, offset, rdata, byte, _GetCPL, 0));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _m_test_access(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, ntvdm64_type_unsigned_32 offset, ntvdm64_type_unsigned_8 byte, ntvdm64_type_bool write) {
    ntvdm64_type_virtual_address ref = 0;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_m_test_access");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, rsreg, offset, byte, write, _GetCPL, 0));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _m_test_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, ntvdm64_type_unsigned_32 offset, ntvdm64_type_unsigned_8 byte, ntvdm64_type_bool write) {
    ntvdm64_type_virtual_address ref = 0;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_m_test_logical");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, rsreg, offset, byte, write, _GetCPL, 0));
    NTVDM64_TYPE_TRACE_CALL_END;
}

static C_VOID _m_read_rm(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_m_read_rm");
    instruction_state.data.crm = 0;
    if (instruction_state.data.flagMem)
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.mrm.rsreg, instruction_state.data.mrm.offset, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), byte));
    else
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_ref(context, instruction_state.data.rrm, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _m_write_rm(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_m_write_rm");
    if (instruction_state.data.flagMem)
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_logical(context, instruction_state.data.mrm.rsreg, instruction_state.data.mrm.offset, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), byte));
    else
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rrm, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}

/* segment accessing unit: _s_ */
/* kernel segment accessing */
static C_VOID _ksa_read_idt(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 intid, ntvdm64_type_virtual_address rdata) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_read_idt");
    if (!_GetCR0_PE) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CR0_PE(0)");
        if (NTVDM64_TYPE_MASK_UNSIGNED_16(intid * 4 + 3) > NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.idtr.limit)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_read_logical(context, &cpu_state.data.idtr, (intid * 4), rdata, 4, 0x00, 0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CR0_PE(1)");
        if (NTVDM64_TYPE_MASK_UNSIGNED_16(intid * 8 + 7) > NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.idtr.limit)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_read_logical(context, &cpu_state.data.idtr, (intid * 8), rdata, 8, 0x00, 0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _ksa_read_ldt(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 selector, ntvdm64_type_virtual_address rdata) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ksa_read_ldt");
    if (!_GetCR0_PE) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (!_GetSelector_TI(selector)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (NTVDM64_TYPE_MASK_UNSIGNED_16(_GetSelector_Offset(selector) + 7) > NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.ldtr.limit)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Selector_Offset(>ldtr.limit)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_read_logical(context, &cpu_state.data.ldtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _ksa_read_gdt(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 selector, ntvdm64_type_virtual_address rdata) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ksa_read_gdt");
    if (!_GetCR0_PE) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetSelector_TI(selector)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (NTVDM64_TYPE_MASK_UNSIGNED_16(_GetSelector_Offset(selector) + 7) > NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.gdtr.limit)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Selector_Offset(>gdtr.limit)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_read_logical(context, &cpu_state.data.gdtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _ksa_read_xdt(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 selector, ntvdm64_type_virtual_address rdata) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ksa_read_xdt");
    if (!_GetCR0_PE) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetSelector_TI(selector)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Selector_TI");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_read_ldt(context, selector, rdata));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!Selector_TI");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_read_gdt(context, selector, rdata));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _ksa_write_ldt(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 selector, ntvdm64_type_virtual_address rdata) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ksa_write_ldt");
    if (!_GetCR0_PE) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (!_GetSelector_TI(selector)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (NTVDM64_TYPE_MASK_UNSIGNED_16(_GetSelector_Offset(selector) + 7) > NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.ldtr.limit))
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_write_logical(context, &cpu_state.data.ldtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _ksa_write_gdt(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 selector, ntvdm64_type_virtual_address rdata) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ksa_write_gdt");
    if (!_GetCR0_PE) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetSelector_TI(selector)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (NTVDM64_TYPE_MASK_UNSIGNED_16(_GetSelector_Offset(selector) + 7) > NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.gdtr.limit))
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_write_logical(context, &cpu_state.data.gdtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _ksa_write_xdt(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 selector, ntvdm64_type_virtual_address rdata) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ksa_write_xdt");
    if (!_GetCR0_PE) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetSelector_TI(selector)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Selector_TI");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_write_ldt(context, selector, rdata));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!Selector_TI");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_write_gdt(context, selector, rdata));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _ksa_load_sreg(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, ntvdm64_type_unsigned_16 selector) {
    ntvdm64_type_unsigned_64 descriptor;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ksa_load_sreg");
    switch (rsreg->sregtype) {
    case SREG_CODE:
        /* note: privilege checking not performed */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_CODE)");
        if (_IsProtected) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Protected(1)");
            if (_IsSelectorNull(selector)) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("selector(null)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
            if (!_IsDescCode(descriptor)) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescCode");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            if (!_IsDescPresent(descriptor)) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(selector));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            _SetDescUserAccessed(descriptor);
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_write_xdt(context, selector, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
            rsreg->flagValid = NTVDM64_TYPE_TRUE;
            rsreg->base = (ntvdm64_type_unsigned_32)_GetDescSeg_Base(descriptor);
            if (_IsDescCodeNonConform(descriptor))
                rsreg->dpl = (ntvdm64_type_unsigned_4)_GetDesc_DPL(descriptor);
            rsreg->limit = (ntvdm64_type_unsigned_32)((_IsDescSegGranularLarge(descriptor) ?
                                        ((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
            rsreg->seg.accessed = (ntvdm64_type_bool)_IsDescUserAccessed(descriptor);
            rsreg->seg.executable = (ntvdm64_type_bool)_IsDescUserExecutable(descriptor);
            rsreg->seg.exec.defsize = (ntvdm64_type_bool)_IsDescCode32(descriptor);
            rsreg->seg.exec.conform = (ntvdm64_type_bool)_IsDescCodeConform(descriptor);
            rsreg->seg.exec.readable = (ntvdm64_type_bool)_IsDescCodeReadable(descriptor);
            rsreg->selector = (selector & ~VCPU_SELECTOR_RPL) | _GetCPL;
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!Protected");
            rsreg->flagValid = NTVDM64_TYPE_TRUE;
            rsreg->base = (selector << 4);
            rsreg->selector = selector;
            if (_GetCR0_PE && _GetEFLAGS_VM) {
                rsreg->dpl = 0x03;
                rsreg->limit = 0x0000ffff;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case SREG_DATA:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_DATA)");
        if (_IsProtected) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Protected");
            if (_IsSelectorNull(selector)) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("selector(null)");
                rsreg->flagValid = NTVDM64_TYPE_FALSE;
                rsreg->selector = selector;
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("selector(!null)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
                if (!_IsDescData(descriptor) && !_IsDescCodeReadable(descriptor)) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescData,!DescCodeReadable");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (_IsDescData(descriptor) || _IsDescCodeNonConform(descriptor)) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DescData/DescCodeNonConform)");
                    if (_GetSelector_RPL(selector) > _GetDesc_DPL(descriptor) ||
                            _GetCPL > _GetDesc_DPL(descriptor)) {
                        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("PL(fail)");
                        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
                        NTVDM64_TYPE_TRACE_BLOCK_END;
                    }
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (!_IsDescPresent(descriptor)) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(selector));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                _SetDescUserAccessed(descriptor);
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_write_xdt(context, selector, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
                rsreg->flagValid = NTVDM64_TYPE_TRUE;
                rsreg->selector = selector;
                rsreg->base = (ntvdm64_type_unsigned_32)_GetDescSeg_Base(descriptor);
                rsreg->dpl = (ntvdm64_type_unsigned_4)_GetDesc_DPL(descriptor);
                rsreg->limit = (ntvdm64_type_unsigned_32)((_IsDescSegGranularLarge(descriptor) ?
                                            ((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
                rsreg->seg.accessed = (ntvdm64_type_bool)_IsDescUserAccessed(descriptor);
                rsreg->seg.executable = (ntvdm64_type_bool)_IsDescUserExecutable(descriptor);
                if (rsreg->seg.executable) {
                    rsreg->seg.exec.defsize = (ntvdm64_type_bool)_IsDescCode32(descriptor);
                    rsreg->seg.exec.conform = (ntvdm64_type_bool)_IsDescCodeConform(descriptor);
                    rsreg->seg.exec.readable = (ntvdm64_type_bool)_IsDescCodeReadable(descriptor);
                } else {
                    rsreg->seg.data.big = (ntvdm64_type_bool)_IsDescDataBig(descriptor);
                    rsreg->seg.data.expdown = (ntvdm64_type_bool)_IsDescDataExpDown(descriptor);
                    rsreg->seg.data.writable = (ntvdm64_type_bool)_IsDescDataWritable(descriptor);
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!Protected");
            rsreg->flagValid = NTVDM64_TYPE_TRUE;
            rsreg->selector = selector;
            rsreg->base = (selector << 4);
            if (_GetCR0_PE && _GetEFLAGS_VM) {
                rsreg->dpl = 0x03;
                rsreg->limit = 0x0000ffff;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case SREG_STACK:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_STACK)");
        if (_IsProtected) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Protected(1)");
            if (_IsSelectorNull(selector)) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("selector(null)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            if (_GetSelector_RPL(selector) != _GetCPL) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Selector_RPL(!CPL)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
            if (!_IsDescDataWritable(descriptor)) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescDataWritable");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            if (_GetDesc_DPL(descriptor) != _GetCPL) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Desc_DPL(!CPL)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            if (!_IsDescPresent(descriptor)) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_SS(selector));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            _SetDescUserAccessed(descriptor);
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_write_xdt(context, selector, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
            rsreg->flagValid = NTVDM64_TYPE_TRUE;
            rsreg->selector = selector;
            rsreg->base = (ntvdm64_type_unsigned_32)_GetDescSeg_Base(descriptor);
            rsreg->dpl = (ntvdm64_type_unsigned_4)_GetDesc_DPL(descriptor);
            rsreg->limit = (ntvdm64_type_unsigned_32)((_IsDescSegGranularLarge(descriptor) ?
                                        ((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
            rsreg->seg.accessed = (ntvdm64_type_bool)_IsDescUserAccessed(descriptor);
            rsreg->seg.executable = (ntvdm64_type_bool)_IsDescUserExecutable(descriptor);
            rsreg->seg.data.big = (ntvdm64_type_bool)_IsDescDataBig(descriptor);
            rsreg->seg.data.expdown = (ntvdm64_type_bool)_IsDescDataExpDown(descriptor);
            rsreg->seg.data.writable = (ntvdm64_type_bool)_IsDescDataWritable(descriptor);
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!Protected");
            rsreg->flagValid = NTVDM64_TYPE_TRUE;
            rsreg->selector = selector;
            rsreg->base = (selector << 4);
            if (_GetCR0_PE && _GetEFLAGS_VM) {
                rsreg->dpl = 0x03;
                rsreg->limit = 0x0000ffff;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case SREG_TR:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_TR)");
        if (!_IsProtected) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        if (_GetCPL) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        if (_GetSelector_TI(selector)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        if (_IsSelectorNull(selector)) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("selector(null)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
        if (!_IsDescTSSAvl(descriptor)) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescTssAvl");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        if (!_IsDescPresent(descriptor)) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(selector));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        _SetDescTSSBusy(descriptor);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_write_xdt(context, selector, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
        rsreg->flagValid = NTVDM64_TYPE_TRUE;
        rsreg->selector = selector;
        rsreg->base = (ntvdm64_type_unsigned_32)_GetDescSeg_Base(descriptor);
        rsreg->dpl = (ntvdm64_type_unsigned_4)_GetDesc_DPL(descriptor);
        rsreg->limit = (ntvdm64_type_unsigned_32)((_IsDescSegGranularLarge(descriptor) ?
                                    (_GetDescSeg_Limit(descriptor) << 12 | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
        rsreg->sys.type = (ntvdm64_type_unsigned_4)_GetDesc_Type(descriptor);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case SREG_LDTR:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_LDTR)");
        if (!_IsProtected) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        if (_GetCPL) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        if (_GetSelector_TI(selector)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        if (_IsSelectorNull(selector)) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("selector(null)");
            rsreg->flagValid = NTVDM64_TYPE_FALSE;
            rsreg->selector = selector;
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("selector(!null)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
            if (!_IsDescLDT(descriptor)) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("descriptor(!LDT)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            if (!_IsDescPresent(descriptor)) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("descriptor(!P)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(selector));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            rsreg->flagValid = NTVDM64_TYPE_TRUE;
            rsreg->selector = selector;
            rsreg->base = (ntvdm64_type_unsigned_32)_GetDescSeg_Base(descriptor);
            rsreg->dpl = (ntvdm64_type_unsigned_4)_GetDesc_DPL(descriptor);
            rsreg->limit = (ntvdm64_type_unsigned_32)((_IsDescSegGranularLarge(descriptor) ?
                                        (_GetDescSeg_Limit(descriptor) << 12 | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
            rsreg->sys.type = (ntvdm64_type_unsigned_4)_GetDesc_Type(descriptor);
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case SREG_GDTR:
    case SREG_IDTR:
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}

/* regular segment accessing */
static ntvdm64_type_bool _s_check_selector(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 selector) {
    /* 0 = succ, 1 = fail */
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_check_selector");
    if (_IsSelectorNull(selector)) {
        NTVDM64_TYPE_TRACE_CALL_END;
        return NTVDM64_TYPE_TRUE;
    }
    if (NTVDM64_TYPE_MASK_UNSIGNED_32(_GetSelector_Offset(selector) + 7) >
            (_GetSelector_TI(selector) ? NTVDM64_TYPE_MASK_UNSIGNED_32(cpu_state.data.ldtr.limit) :
             NTVDM64_TYPE_MASK_UNSIGNED_32(cpu_state.data.gdtr.limit))) {
        NTVDM64_TYPE_TRACE_CALL_END;
        return NTVDM64_TYPE_TRUE;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
    return NTVDM64_TYPE_FALSE;
}
static C_VOID _s_read_idt(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 intid, ntvdm64_type_virtual_address rdata) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_read_idt");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_read_idt(context, intid, rdata));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_read_xdt(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 selector, ntvdm64_type_virtual_address rdata) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_read_xdt");
    if (!_GetCR0_PE) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector, rdata));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_read_tss(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 offset, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_read_tss");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_read_logical(context, &cpu_state.data.tr, offset, rdata, byte, 0, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_read_es(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 offset, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_read_es");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, &cpu_state.data.es, offset, rdata, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_read_cs(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 offset, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_read_cs");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_read_logical(context, &cpu_state.data.cs, offset, rdata, byte, 0, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_read_ss(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 offset, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_read_ss");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, &cpu_state.data.ss, offset, rdata, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_read_ds(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 offset, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_read_ds");
    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_read_fs(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 offset, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_read_fs");
    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_read_gs(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 offset, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_read_gs");
    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_idt(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 intid, ntvdm64_type_virtual_address rdata) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_write_idt");
    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_xdt(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 selector, ntvdm64_type_virtual_address rdata) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_write_xdt");
    if (!_GetCR0_PE) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_write_xdt(context, selector, rdata));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_tss(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 offset, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_write_tss");
    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_es(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 offset, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_write_es");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_logical(context, &cpu_state.data.es, offset, rdata, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_cs(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 offset, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_write_cs");
    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_ss(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 offset, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_write_ss");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_logical(context, &cpu_state.data.ss, offset, rdata, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_ds(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 offset, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_write_ds");
    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_fs(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 offset, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_write_fs");
    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_gs(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 offset, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_write_gs");
    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_test_tss(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 offset, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_test_cs");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &cpu_state.data.tr, offset, byte, 0, 0x00, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_test_cs(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 offset, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_test_cs");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &cpu_state.data.cs, offset, byte, 0, 0x00, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_test_ss_push(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_32 cesp = 0x00000000;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_test_ss_push");
    switch (_GetStackSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("StackSize(2)");
        if (cpu_state.data.sp && cpu_state.data.sp < byte)
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_SS(0));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.ss, NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.sp - byte), byte, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("StackSize(4)");
        if (cpu_state.data.esp && cpu_state.data.esp < byte)
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_SS(0));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.ss, NTVDM64_TYPE_MASK_UNSIGNED_32(cpu_state.data.esp - byte), byte, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_test_ss_pop(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_32 cesp = 0x00000000;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_test_ss_pop");
    switch (_GetStackSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("StackSize(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.ss, cpu_state.data.sp, byte, 0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("StackSize(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.ss, cpu_state.data.esp, byte, 0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_sreg(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, ntvdm64_type_unsigned_16 selector) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_load_sreg");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_load_sreg(context, rsreg, selector));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_gdtr(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 base, ntvdm64_type_unsigned_16 limit, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_load_gdtr");
    if (_GetCPL) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    cpu_state.data.gdtr.limit = limit;
    switch (byte) {
    case 2:
        cpu_state.data.gdtr.base = NTVDM64_TYPE_MASK_UNSIGNED_24(base);
        break;
    case 4:
        cpu_state.data.gdtr.base = NTVDM64_TYPE_MASK_UNSIGNED_32(base);
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_idtr(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 base, ntvdm64_type_unsigned_16 limit, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_load_idtr");
    if (_GetCPL) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    cpu_state.data.idtr.limit = limit;
    switch (byte) {
    case 2:
        cpu_state.data.idtr.base = NTVDM64_TYPE_MASK_UNSIGNED_24(base);
        break;
    case 4:
        cpu_state.data.idtr.base = NTVDM64_TYPE_MASK_UNSIGNED_32(base);
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_ldtr(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 selector) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_load_ldtr");
    if (_GetCPL) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    if (_GetSelector_TI(selector)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Selector_TI(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.ldtr, selector));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_tr(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 selector) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_load_tr");
    if (_GetCPL) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    if (_GetSelector_TI(selector)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Selector_TI(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.tr, selector));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_cr0_msw(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 msw) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_load_cr0_msw");
    if (_GetCPL) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    if (!_GetCR0_PE) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CR0_PE(0)");
        cpu_state.data.cr0 = (cpu_state.data.cr0 & 0xfffffff0) | (msw & 0x000f);
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CR0_PE(1)");
        cpu_state.data.cr0 = (cpu_state.data.cr0 & 0xfffffff0) | (msw & 0x000e) | 0x01;
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_cs(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_load_cs");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.cs, newcs));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_ss(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newss) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_load_ss");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.ss, newss));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_ds(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newds) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_load_ds");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.ds, newds));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_es(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newes) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_load_es");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.es, newes));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_fs(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newfs) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_load_fs");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.fs, newfs));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_gs(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newgs) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_load_gs");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.gs, newgs));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_test_eip(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_test_eip");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_cs(context, cpu_state.data.eip, 0x01));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _s_test_esp(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_32 cesp;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_s_test_esp");
    switch (_GetStackSize) {
    case 2:
        cesp = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.esp);
        break;
    case 4:
        cesp = NTVDM64_TYPE_MASK_UNSIGNED_32(cpu_state.data.esp);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_test_logical(context, &cpu_state.data.ss, cesp, 0x00, 0));
    NTVDM64_TYPE_TRACE_CALL_END;
}

/* portid accessing unit */
/* kernel portid accessing */
_______todo _kpa_test_iomap(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 portid, ntvdm64_type_unsigned_8 byte) {
    /* TODO(High): Add protected-mode I/O-map permission checks with owned probes. */
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kpa_test_iomap");
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _kpa_test_mode(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 portid, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_p_test");
    if (_GetCR0_PE && (_GetCPL > (ntvdm64_type_unsigned_8)_GetEFLAGS_IOPL || _GetEFLAGS_VM)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CR0_PE(1),(CPL>IOPL/EFLAGS_VM(1))");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kpa_test_iomap(context, portid, byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
/* regular portid accessing */
static C_VOID _p_input(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 portid, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_p_input");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kpa_test_mode(context, portid, byte));
    core_machine_port_execute_read(context->port, portid);
    switch (byte) {
    case 1:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, rdata,
            NTVDM64_TYPE_REFERENCE_OF(context->port->data.ioByte), 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, rdata,
            NTVDM64_TYPE_REFERENCE_OF(context->port->data.ioWord), 2));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, rdata,
            NTVDM64_TYPE_REFERENCE_OF(context->port->data.ioDWord), 4));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    instruction_state.data.flagIgnore = NTVDM64_TYPE_TRUE;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _p_output(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 portid, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_p_output");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kpa_test_mode(context, portid, byte));
    switch (byte) {
    case 1:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_ref(context, rdata,
            NTVDM64_TYPE_REFERENCE_OF(context->port->data.ioByte), 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_ref(context, rdata,
            NTVDM64_TYPE_REFERENCE_OF(context->port->data.ioWord), 2));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_ref(context, rdata,
            NTVDM64_TYPE_REFERENCE_OF(context->port->data.ioDWord), 4));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    core_machine_port_execute_write(context->port, portid);
    instruction_state.data.flagIgnore = NTVDM64_TYPE_TRUE;
    NTVDM64_TYPE_TRACE_CALL_END;
}

/* decoding unit */
/* kernel decoding function */
static ntvdm64_type_bool _kdf_check_prefix(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 opcode) {
    switch (opcode) {
    case 0xf0:
    case 0xf2:
    case 0xf3:
    case 0x2e:
    case 0x36:
    case 0x3e:
    case 0x26:
        return NTVDM64_TYPE_TRUE;
        break;
    case 0x64:
    case 0x65:
    case 0x66:
    case 0x67:
        i386(opcode) return NTVDM64_TYPE_TRUE;
        else return NTVDM64_TYPE_FALSE;
        break;
    default:
        return NTVDM64_TYPE_FALSE;
        break;
    }
    return NTVDM64_TYPE_FALSE;
}

static C_VOID _kdf_skip(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kdf_skip");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(cpu_state.data.eip += byte);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _kdf_code(core_machine_cpu_execution_context *context, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kdf_code");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, rdata, byte));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_skip(context, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _kdf_modrm(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 regbyte, ntvdm64_type_unsigned_8 rmbyte) {
    ntvdm64_type_signed_8 disp8;
    ntvdm64_type_unsigned_16 disp16;
    ntvdm64_type_unsigned_32 disp32;
    ntvdm64_type_unsigned_32 sibindex;
    ntvdm64_type_unsigned_8 modrm, sib;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kdf_modrm");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code(context, NTVDM64_TYPE_REFERENCE_OF(modrm), 1));
    instruction_state.data.flagMem = NTVDM64_TYPE_TRUE;
    instruction_state.data.mrm.rsreg = STD_NULL;
    instruction_state.data.mrm.offset = 0;
    instruction_state.data.cr = instruction_state.data.crm = 0;
    instruction_state.data.rrm = instruction_state.data.rr = (ntvdm64_type_virtual_address)STD_NULL;
    switch (_GetAddressSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
        switch (_GetModRM_MOD(modrm)) {
        case 0:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(0)");
            switch (_GetModRM_RM(modrm)) {
            case 0:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.bx + cpu_state.data.si);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 1:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.bx + cpu_state.data.di);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 2:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.bp + cpu_state.data.si);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 3:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.bp + cpu_state.data.di);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 4:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.si);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 5:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.di);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 6:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(6)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code(context, NTVDM64_TYPE_REFERENCE_OF(disp16), 2));
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 7:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.bx);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(1)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code(context, NTVDM64_TYPE_REFERENCE_OF(disp8), 1));
            switch (_GetModRM_RM(modrm)) {
            case 0:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.bx + cpu_state.data.si + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 1:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.bx + cpu_state.data.di + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 2:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.bp + cpu_state.data.si + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 3:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.bp + cpu_state.data.di + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 4:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.si + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 5:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.di + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 6:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.bp + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 7:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.bx + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code(context, NTVDM64_TYPE_REFERENCE_OF(disp16), 2));
            switch (_GetModRM_RM(modrm)) {
            case 0:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.bx + cpu_state.data.si + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 1:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.bx + cpu_state.data.di + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 2:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.bp + cpu_state.data.si + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 3:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.bp + cpu_state.data.di + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 4:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.si + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 5:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.di + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 6:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.bp + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 7:
                instruction_state.data.mrm.offset = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.bx + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3:
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
        if (_GetModRM_MOD(modrm) != 3 && _GetModRM_RM(modrm) == 4) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(!3),ModRM_RM(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code(context, NTVDM64_TYPE_REFERENCE_OF(sib), 1));
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
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            sibindex <<= _GetSIB_SS(sib);
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        switch (_GetModRM_MOD(modrm)) {
        case 0:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(0)");
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
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(4)");
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
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SIB_Base(5)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code(context, NTVDM64_TYPE_REFERENCE_OF(disp32), 4));
                    instruction_state.data.mrm.offset = disp32 + sibindex;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
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
                    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 5:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(5)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code(context, NTVDM64_TYPE_REFERENCE_OF(disp32), 4));
                instruction_state.data.mrm.offset = disp32;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                NTVDM64_TYPE_TRACE_BLOCK_END;
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
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(1)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code(context, NTVDM64_TYPE_REFERENCE_OF(disp8), 1));
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
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(4)");
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
                    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
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
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code(context, NTVDM64_TYPE_REFERENCE_OF(disp32), 4));
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
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(4)");
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
                    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
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
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3:
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    if (_GetModRM_MOD(modrm) == 3) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(3)");
        instruction_state.data.flagMem = NTVDM64_TYPE_FALSE;
        switch (rmbyte) {
        case 1:
            switch (_GetModRM_RM(modrm)) {
            case 0:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.al);
                break;
            case 1:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.cl);
                break;
            case 2:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.dl);
                break;
            case 3:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.bl);
                break;
            case 4:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.ah);
                break;
            case 5:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.ch);
                break;
            case 6:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.dh);
                break;
            case 7:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.bh);
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        case 2:
            switch (_GetModRM_RM(modrm)) {
            case 0:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.ax);
                break;
            case 1:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.cx);
                break;
            case 2:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.dx);
                break;
            case 3:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.bx);
                break;
            case 4:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.sp);
                break;
            case 5:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.bp);
                break;
            case 6:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.si);
                break;
            case 7:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.di);
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        case 4:
            switch (_GetModRM_RM(modrm)) {
            case 0:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.eax);
                break;
            case 1:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.ecx);
                break;
            case 2:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.edx);
                break;
            case 3:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.ebx);
                break;
            case 4:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.esp);
                break;
            case 5:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.ebp);
                break;
            case 6:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.esi);
                break;
            case 7:
                instruction_state.data.rrm = (ntvdm64_type_virtual_address)(&cpu_state.data.edi);
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        default:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("rmbyte");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(rmbyte));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_ref(context, instruction_state.data.rrm, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), rmbyte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    if (!regbyte) {
        /* reg is operation or segment */
        instruction_state.data.cr = _GetModRM_REG(modrm);
    } else {
        switch (regbyte) {
        case 1:
            switch (_GetModRM_REG(modrm)) {
            case 0:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.al);
                break;
            case 1:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.cl);
                break;
            case 2:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.dl);
                break;
            case 3:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.bl);
                break;
            case 4:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.ah);
                break;
            case 5:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.ch);
                break;
            case 6:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.dh);
                break;
            case 7:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.bh);
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        case 2:
            switch (_GetModRM_REG(modrm)) {
            case 0:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.ax);
                break;
            case 1:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.cx);
                break;
            case 2:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.dx);
                break;
            case 3:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.bx);
                break;
            case 4:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.sp);
                break;
            case 5:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.bp);
                break;
            case 6:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.si);
                break;
            case 7:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.di);
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        case 4:
            switch (_GetModRM_REG(modrm)) {
            case 0:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.eax);
                break;
            case 1:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.ecx);
                break;
            case 2:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.edx);
                break;
            case 3:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.ebx);
                break;
            case 4:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.esp);
                break;
            case 5:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.ebp);
                break;
            case 6:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.esi);
                break;
            case 7:
                instruction_state.data.rr = (ntvdm64_type_virtual_address)(&cpu_state.data.edi);
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        default:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("regbyte");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(regbyte));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.cr), regbyte));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _d_skip(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_skip");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_skip(context, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _d_code(core_machine_cpu_execution_context *context, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_code");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_code(context, rdata, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _d_imm(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_imm");
    instruction_state.data.cimm = 0;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_code(context, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.cimm), byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _d_moffs(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_moffs");
    instruction_state.data.flagMem = NTVDM64_TYPE_TRUE;
    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
    instruction_state.data.mrm.offset = 0;
    switch (_GetAddressSize) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_code(context, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.mrm.offset), 2));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_code(context, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.mrm.offset), 4));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _d_modrm_sreg(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 rmbyte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_modrm_sreg");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_modrm(context, 0, rmbyte));
    instruction_state.data.rmovsreg = STD_NULL;
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
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _d_modrm_ea(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 regbyte, ntvdm64_type_unsigned_8 rmbyte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_modrm_ea");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_modrm(context, regbyte, rmbyte));
    if (!instruction_state.data.flagMem) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _d_modrm(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 regbyte, ntvdm64_type_unsigned_8 rmbyte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_modrm");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_modrm(context, regbyte, rmbyte));
    if (!instruction_state.data.flagMem && instruction_state.data.flagLock) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(0),flagLock(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}

/* execution control unit: _e_ */
/* kernel execution control */
static C_VOID _kec_push(core_machine_cpu_execution_context *context, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_32 cesp;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kec_push");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_ss_push(context, byte));
    switch (_GetStackSize) {
    case 2:
        cesp = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.sp - byte);
        break;
    case 4:
        cesp = NTVDM64_TYPE_MASK_UNSIGNED_32(cpu_state.data.esp - byte);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_write_ss(context, cesp, rdata, byte));
    switch (_GetStackSize) {
    case 2:
        cpu_state.data.sp -= byte;
        break;
    case 4:
        cpu_state.data.esp -= byte;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _kec_pop(core_machine_cpu_execution_context *context, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_32 cesp;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kec_pop");
    switch (_GetStackSize) {
    case 2:
        cesp = cpu_state.data.sp;
        break;
    case 4:
        cesp = cpu_state.data.esp;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_ss(context, cesp, rdata, byte));
    if (rdata != NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.esp)) {
        switch (_GetStackSize) {
        case 2:
            cpu_state.data.sp += byte;
            break;
        case 4:
            cpu_state.data.esp += byte;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _kec_call_far(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs, ntvdm64_type_unsigned_32 neweip, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_32 oldcs = cpu_state.data.cs.selector;
    t_cpu_data_sreg ccs = cpu_state.data.cs;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kec_call_far");
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_ss_push(context, 4));
        neweip = NTVDM64_TYPE_MASK_UNSIGNED_16(neweip);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(oldcs), 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ip), 2));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_ss_push(context, 8));
        neweip = NTVDM64_TYPE_MASK_UNSIGNED_32(neweip);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(oldcs), 4));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.eip), 4));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    cpu_state.data.cs = ccs;
    cpu_state.data.eip = neweip;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _kec_call_near(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 neweip, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kec_call_near");
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        neweip = NTVDM64_TYPE_MASK_UNSIGNED_16(neweip);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_cs(context, neweip, 0x01));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ip), 2));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        neweip = NTVDM64_TYPE_MASK_UNSIGNED_32(neweip);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_cs(context, neweip, 0x01));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.eip), 4));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    cpu_state.data.eip = neweip;
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo _kec_task_switch(ntvdm64_type_unsigned_16 newtss);
static C_VOID _kec_jmp_far(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs, ntvdm64_type_unsigned_32 neweip, ntvdm64_type_unsigned_8 byte) {
    t_cpu_data_sreg ccs = cpu_state.data.cs;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kec_jmp_far");
    switch (byte) {
    case 2:
        neweip = NTVDM64_TYPE_MASK_UNSIGNED_16(neweip);
        break;
    case 4:
        neweip = NTVDM64_TYPE_MASK_UNSIGNED_32(neweip);
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
    cpu_state.data.cs = ccs;
    cpu_state.data.eip = neweip;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _kec_jmp_near(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 neweip, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kec_jmp_near");
    switch (byte) {
    case 2:
        neweip = NTVDM64_TYPE_MASK_UNSIGNED_16(neweip);
        break;
    case 4:
        neweip = NTVDM64_TYPE_MASK_UNSIGNED_32(neweip);
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_cs(context, neweip, 0x01));
    cpu_state.data.eip = neweip;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _kec_ret_far(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 newcs, ntvdm64_type_unsigned_32 neweip, ntvdm64_type_unsigned_16 parambyte, ntvdm64_type_unsigned_16 byte) {
    t_cpu_data_sreg ccs = cpu_state.data.cs;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kec_ret_far");
    switch (byte) {
    case 2:
        neweip = NTVDM64_TYPE_MASK_UNSIGNED_16(neweip);
        break;
    case 4:
        neweip = NTVDM64_TYPE_MASK_UNSIGNED_32(neweip);
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
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
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _kec_ret_near(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 parambyte, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_32 neweip = 0;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kec_ret_near");
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(neweip), 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_cs(context, neweip, 0x01));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(neweip), 4));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_cs(context, neweip, 0x01));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
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
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
/* sub execution routine */
static C_VOID _ser_call_far_real(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs, ntvdm64_type_unsigned_32 neweip, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ser_call_far_real");
    if (_IsProtected) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_call_far(context, newcs, neweip, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo _ser_call_far_cs_conf(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs, ntvdm64_type_unsigned_32 neweip, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_64 descriptor;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ser_call_far_cs_conf");
    if (!_IsProtected) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescCodeConform(descriptor)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetDesc_DPL(descriptor) > _GetCPL) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DPL(>CPL)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    if (!_IsDescPresent(descriptor)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_call_far(context, newcs, neweip, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo _ser_call_far_cs_nonc(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs, ntvdm64_type_unsigned_32 neweip, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_64 descriptor;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ser_call_far_cs_nonc");
    if (!_IsProtected) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescCodeNonConform(descriptor)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetDesc_DPL(descriptor) != _GetCPL ||
            _GetSelector_RPL(newcs) > _GetCPL) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DPL(!CPL)/RPL(>CPL)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    if (!_IsDescPresent(descriptor)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_call_far(context, newcs, neweip, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo _ser_call_far_call_gate(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs) {
    ntvdm64_type_unsigned_64 descriptor;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ser_call_far_call_gate");
    if (!_IsProtected) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescCallGate(descriptor)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(0));
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo _ser_call_far_task_gate(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs) {
    ntvdm64_type_unsigned_64 descriptor;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ser_call_far_task_gate");
    if (!_IsProtected) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescTaskGate(descriptor)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(0));
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo _ser_call_far_tss(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs) {
    ntvdm64_type_unsigned_64 descriptor;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ser_call_far_tss");
    if (!_IsProtected) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescTSS(descriptor)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(0));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _ser_int_real(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 intid, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_16 cip;
    ntvdm64_type_unsigned_32 vector;
    ntvdm64_type_unsigned_32 oldcs = cpu_state.data.cs.selector;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ser_int_real");
    if (_IsProtected) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (NTVDM64_TYPE_MASK_UNSIGNED_16(intid * 4 + 3) > NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.idtr.limit)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("intid(>idtr.limit)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_ss_push(context, 6));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.flags), 2));
        _ClrEFLAGS_IF;
        _ClrEFLAGS_TF;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(oldcs), 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ip), 2));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_ss_push(context, 12));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.eflags), 4));
        _ClrEFLAGS_IF;
        _ClrEFLAGS_TF;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(oldcs), 4));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.eip), 4));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_idt(context, intid, NTVDM64_TYPE_REFERENCE_OF(vector)));
    cip = NTVDM64_TYPE_MASK_UNSIGNED_16(vector);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_cs(context, cip, 0x01));
    cpu_state.data.eip = cip;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_cs(context, NTVDM64_TYPE_MASK_UNSIGNED_16(vector >> 16)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo _ser_int_protected(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 intid, ntvdm64_type_unsigned_8 byte, ntvdm64_type_bool flagext) {
    ntvdm64_type_unsigned_16 oldss;
    ntvdm64_type_unsigned_32 oldeflags, oldesp, newesp = NTVDM64_TYPE_ZERO_32;
    ntvdm64_type_unsigned_32 xs_sel;
    ntvdm64_type_unsigned_16 newcs, newss = NTVDM64_TYPE_ZERO_16;
    ntvdm64_type_unsigned_64 cs_desc, ss_desc, gate_desc;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ser_int_protected");
    if (!_GetCR0_PE) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (NTVDM64_TYPE_MASK_UNSIGNED_16(intid * 8 + 7) > NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.idtr.limit)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("intid(>idtr.limit)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(intid * 8 + 2 + !!flagext));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_idt(context, intid, NTVDM64_TYPE_REFERENCE_OF(gate_desc)));
    switch (_GetDesc_Type(gate_desc)) {
    case VCPU_DESC_SYS_TYPE_TASKGATE:
    case VCPU_DESC_SYS_TYPE_INTGATE_16:
    case VCPU_DESC_SYS_TYPE_INTGATE_32:
    case VCPU_DESC_SYS_TYPE_TRAPGATE_16:
    case VCPU_DESC_SYS_TYPE_TRAPGATE_32:
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Desc_Type(!TaskGate/!IntGate/!TrapGate)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(intid * 8 + 2 + !!flagext));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    if (!flagext) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!flagext");
        if (_GetDesc_DPL(gate_desc) < _GetCPL) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DPL(<CPL)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(intid * 8 + 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    if (!_IsDescPresent(gate_desc)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(intid * 8 + 2 + !!flagext));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    if (_GetDesc_Type(gate_desc) == VCPU_DESC_SYS_TYPE_TASKGATE) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Desc_Type(TaskGate)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Desc_Type(!TaskGate)");
        newcs = NTVDM64_TYPE_MASK_UNSIGNED_16(_GetDescGate_Selector(gate_desc));
        if (_IsSelectorNull(newcs)) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("newcs(null)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(!!flagext));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        if (_GetSelector_TI(newcs)) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Selector_TI(1)");
            if (NTVDM64_TYPE_MASK_UNSIGNED_16(_GetSelector_Offset(newcs) + 7) > NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.ldtr.limit)) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Selector_Offset(>ldtr.limit)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffc + !!flagext));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Selector_TI(0)");
            if (NTVDM64_TYPE_MASK_UNSIGNED_16(_GetSelector_Offset(newcs) + 7) > NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.gdtr.limit)) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Selector_Offset(>gdtr.limit)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffc + !!flagext));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, NTVDM64_TYPE_REFERENCE_OF(cs_desc)));
        if (!_IsDescCode(cs_desc) ||
                _GetDesc_DPL(cs_desc) > _GetCPL) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescCode/DPL(>CPL)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffc + !!flagext));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        if (!_IsDescPresent(cs_desc)) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffc + !!flagext));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        if (_IsDescCodeNonConform(cs_desc) &&
                _GetDesc_DPL(cs_desc) < _GetCPL) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DescCodeNonConform,DPL(<CPL)");
            if (!_GetEFLAGS_VM) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("EFLAS_VM(0)");
                /* INTER-PRIVILEGE-LEVEL-INTERRUPT */
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("EFLAS_VM(1)");
                if (_GetDesc_DPL(cs_desc) != 0) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DPL(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                /* INTERRUPT-FROM-VIRTUAL-8086-MODE */
                switch (cpu_state.data.tr.sys.type) {
                case VCPU_DESC_SYS_TYPE_TSS_32_AVL:
                case VCPU_DESC_SYS_TYPE_TSS_32_BUSY:
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("tr.type(32)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_tss(context, 4, 8));
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, 4, NTVDM64_TYPE_REFERENCE_OF(newesp), 4));
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, 8, NTVDM64_TYPE_REFERENCE_OF(newss), 2));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                    break;
                case VCPU_DESC_SYS_TYPE_TSS_16_AVL:
                case VCPU_DESC_SYS_TYPE_TSS_16_BUSY:
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("tr.type(16)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_tss(context, 2, 4));
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, 2, NTVDM64_TYPE_REFERENCE_OF(newesp), 2));
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, 4, NTVDM64_TYPE_REFERENCE_OF(newss), 2));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                    break;
                default:
                    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                if (_IsSelectorNull(newss)) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("newss(null)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(!!flagext));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (_GetSelector_RPL(newss) != _GetDesc_DPL(cs_desc)) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RPL(newss)!=DPL(newcs)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(newss & 0xfffc + !!flagext));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newss, NTVDM64_TYPE_REFERENCE_OF(ss_desc)));
                if (_GetDesc_DPL(ss_desc) !=
                        _GetDesc_DPL(cs_desc)) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DPL(newss)!=DPL(newcs)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(newss & 0xfffc + !!flagext));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (!_IsDescPresent(ss_desc)) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescPresent(newss)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_SS(newss & 0xfffc + !!flagext));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                /*NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &css, &ss_xdesc));
                if (_IsDescSys32(gate_desc)) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DescSys32(gate)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &css, newesp, 36, 1, 0x00, 0));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                } else {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescSys32(gate)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &css, newesp, 18, 1, 0x00, 0));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
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
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &cpu_state.data.ss, newss));
                cpu_state.data.esp = newesp;
                if (_IsDescSys32(gate_desc)) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DescSys32(gate)");
                    xs_sel = cpu_state.data.gs.selector;
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 4));
                    xs_sel = cpu_state.data.fs.selector;
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 4));
                    xs_sel = cpu_state.data.ds.selector;
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 4));
                    xs_sel = cpu_state.data.es.selector;
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 4));
                    xs_sel = oldss;
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 4));
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(oldesp), 4));
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(oldeflags), 4));
                    xs_sel = cpu_state.data.cs.selector;
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 4));
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.eip), 4));
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_gs(context, 0x0000));
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_fs(context, 0x0000));
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_ds(context, 0x0000));
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_es(context, 0x0000));
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &cpu_state.data.cs, newcs));
                    switch (byte) {
                    case 2:
                        cpu_state.data.eip = NTVDM64_TYPE_MASK_UNSIGNED_16(_GetDescGate_Offset(gate_desc));
                        break;
                    case 4:
                        cpu_state.data.eip = NTVDM64_TYPE_MASK_UNSIGNED_32(_GetDescGate_Offset(gate_desc));
                        break;
                    default:
                        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
                        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
                        NTVDM64_TYPE_TRACE_BLOCK_END;
                        break;
                    }
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                } else {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescSys32(gate)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(0));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DescCodeConform/DPL(>=CPL)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _ser_ret_far_real(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs, ntvdm64_type_unsigned_32 neweip, ntvdm64_type_unsigned_16 parambyte, ntvdm64_type_unsigned_16 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ser_ret_far_real");
    if (_IsProtected) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_ret_far(context, newcs, neweip, parambyte, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _ser_ret_far_same(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs, ntvdm64_type_unsigned_32 neweip, ntvdm64_type_unsigned_16 parambyte, ntvdm64_type_unsigned_16 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ser_ret_far_same");
    if (!_IsProtected) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_ret_far(context, newcs, neweip, parambyte, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo _ser_ret_far_outer(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs, ntvdm64_type_unsigned_32 neweip, ntvdm64_type_unsigned_16 parambyte, ntvdm64_type_unsigned_16 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ser_ret_far_outer");
    if (!_IsProtected) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(0));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _ser_jmp_far_real(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs, ntvdm64_type_unsigned_32 neweip, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ser_jmp_far_real");
    if (_IsProtected) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_jmp_far(context, newcs, neweip, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _ser_jmp_far_cs_conf(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs, ntvdm64_type_unsigned_32 neweip, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_64 descriptor;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ser_jmp_far_cs_conf");
    if (!_IsProtected) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescCodeConform(descriptor)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetDesc_DPL(descriptor) > _GetCPL) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DPL(>CPL)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    if (!_IsDescPresent(descriptor)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_jmp_far(context, newcs, neweip, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _ser_jmp_far_cs_nonc(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs, ntvdm64_type_unsigned_32 neweip, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_64 descriptor;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ser_jmp_far_cs_nonc");
    if (!_IsProtected) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescCodeNonConform(descriptor)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetDesc_DPL(descriptor) != _GetCPL ||
            _GetSelector_RPL(newcs) > _GetCPL) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DPL(!CPL)/RPL(>CPL)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    if (!_IsDescPresent(descriptor)) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_jmp_far(context, newcs, neweip, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo _ser_jmp_far_call_gate(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs) {
    ntvdm64_type_unsigned_64 descriptor;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ser_jmp_far_call_gate");
    if (!_IsProtected) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescCallGate(descriptor)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(0));
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo _ser_jmp_far_task_gate(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs) {
    ntvdm64_type_unsigned_64 descriptor;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ser_jmp_far_task_gate");
    if (!_IsProtected) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescTaskGate(descriptor)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(0));
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo _ser_jmp_far_tss(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs) {
    ntvdm64_type_unsigned_64 descriptor;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_ser_jmp_far_tss");
    if (!_IsProtected) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescTSS(descriptor)) NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(0));
    NTVDM64_TYPE_TRACE_CALL_END;
}
/* regular execute control */
static C_VOID _e_push(core_machine_cpu_execution_context *context, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_e_push");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_push(context, rdata, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _e_pop(core_machine_cpu_execution_context *context, ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_e_pop");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, rdata, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _e_call_far(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs, ntvdm64_type_unsigned_32 neweip, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_64 descriptor;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_e_call_far");
    if (!_IsProtected) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!Protected");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_call_far_real(context, newcs, neweip, byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Protected");
        if (_IsSelectorNull(newcs)) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("newcs(null)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
        if (_IsDescCodeConform(descriptor))
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_call_far_cs_conf(context, newcs, neweip, byte));
        else if (_IsDescCodeNonConform(descriptor))
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_call_far_cs_nonc(context, newcs, neweip, byte));
        else if (_IsDescCallGate(descriptor))
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_call_far_call_gate(context, newcs));
        else if (_IsDescTaskGate(descriptor))
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_call_far_task_gate(context, newcs));
        else if (_IsDescTSS(descriptor))
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_call_far_tss(context, newcs));
        else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("desc(invalid)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _e_call_near(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 neweip, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_e_call_near");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_call_near(context, neweip, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo _e_int3(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_e_int3");
    if (!_GetCR0_PE) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Real");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_int_real(context, 0x03, byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!Real");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_int_protected(context, 0x03, byte, 0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo _e_into(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_e_into");
    if (_GetEFLAGS_OF) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("EFLAGS_OF(1)");
        if (!_GetCR0_PE) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Real");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_int_real(context, 0x04, byte));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!Real");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_int_protected(context, 0x04, byte, 0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo _e_int_n(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 intid, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_e_int_n");
    if (!_GetCR0_PE) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Real");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_int_real(context, intid, byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!Real");
        if (_GetEFLAGS_VM && _GetEFLAGS_IOPL < 3) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("EFLAGAS_VM(1),IOPL(<3)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("EFLAGS_VM(0)/IOPL(3)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_int_protected(context, intid, byte, 0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo _e_intr_n(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 intid, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_e_intr_n");
    if (!_GetCR0_PE) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Real");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_int_real(context, intid, byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!Real");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_int_protected(context, intid, byte, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo _e_except_n(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 exid, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_e_except_n");
    instruction_state.data.except &= ~(1 << exid);
    if (!_GetCR0_PE) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Real");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_int_real(context, exid, byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!Real");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo _e_iret(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_16 newcs, newss, newds, newes, newfs, newgs;
    ntvdm64_type_unsigned_32 neweip = NTVDM64_TYPE_ZERO_32, newesp, neweflags = NTVDM64_TYPE_ZERO_32;
    ntvdm64_type_unsigned_32 xs_sel;
    ntvdm64_type_unsigned_32 mask = VCPU_EFLAGS_RESERVED;
    t_cpu_data_sreg ccs = cpu_state.data.cs;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_e_iret");
    if (!_GetCR0_PE) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Real");
        switch (byte) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 6));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(neweip), 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 2));
            newcs = NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel);
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(neweflags), 2));
            mask |= 0xffff0000;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 12));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(neweip), 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 4));
            newcs = NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel);
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(neweflags), 4));
            /* cpu_state.data.eflags = (neweflags & 0x00257fd5) | (cpu_state.data.eflags & 0x001a0000); */
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
        cpu_state.data.cs = ccs;
        cpu_state.data.eip = neweip;
        cpu_state.data.eflags = (neweflags & ~mask) | (cpu_state.data.eflags & mask);
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!Real");
        if (_GetEFLAGS_VM) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("V86");
            /* RETURN-FROM-VIRTUAL-8086-MODE */
            if (_GetEFLAGS_IOPL == 3) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("EFLAGS_IOPL(3)");
                switch (byte) {
                case 2:
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 6));
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(neweip), 2));
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 2));
                    newcs = NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel);
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(neweflags), 2));
                    mask |= (0xffff0000 | VCPU_EFLAGS_IOPL);
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                    break;
                case 4:
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 12));
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(neweip), 4));
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 4));
                    newcs = NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel);
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(neweflags), 4));
                    mask |= (VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL);
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                    break;
                default:
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                    break;
                }
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
                cpu_state.data.cs = ccs;
                cpu_state.data.eip = neweip;
                cpu_state.data.eflags = (neweflags & ~mask) | (cpu_state.data.eflags & mask);
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("EFLAGS_IOPL(!3)");
                /* trap to virtual-8086 monitor */
                instruction_state.data.oldcpu = cpu_state;
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else if (_GetEFLAGS_NT) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Nested");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Protected,!Nested");
            switch (byte) {
            case 2:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 6));
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(neweip), 2));
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 2));
                newcs = NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel);
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(neweflags), 2));
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 12));
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(neweip), 4));
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 4));
                newcs = NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel);
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(neweflags), 4));
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            default:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            }
            if (NTVDM64_TYPE_GET_BIT(neweflags, VCPU_EFLAGS_VM) && !_GetCPL) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("neweflags(VM),CPL(0)");
                /* return to v86 */
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 24));
                cpu_state.data.eflags = (neweflags & ~mask) | (cpu_state.data.eflags & mask);
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
                cpu_state.data.cs = ccs;
                cpu_state.data.eip = neweip;
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(newesp), 4));
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 4));
                newss = NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel);
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 4));
                newes = NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel);
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 4));
                newds = NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel);
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 4));
                newfs = NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel);
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 4));
                newgs = NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel);
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_ss(context, newss));
                cpu_state.data.esp = newesp;
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_es(context, newes));
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_ds(context, newds));
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_fs(context, newfs));
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_gs(context, newgs));
                _MakeCPL(0x03);
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("neweflags(!VM)/CPL(!0)");
                /* return to proctected */
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _e_jcc(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 csrc, ntvdm64_type_unsigned_8 byte, ntvdm64_type_bool condition) {
    ntvdm64_type_unsigned_32 neweip = cpu_state.data.eip;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_e_jcc");
    if (condition) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("condition(1)");
        switch (byte) {
        case 1:
            neweip += (ntvdm64_type_signed_8)csrc;
            break;
        case 2:
            neweip += (ntvdm64_type_signed_16)csrc;
            break;
        case 4:
            neweip += (ntvdm64_type_signed_32)csrc;
            break;
        default:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_jmp_near(context, neweip, _GetOperandSize));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _e_jmp_far(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 newcs, ntvdm64_type_unsigned_32 neweip, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_64 descriptor;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_e_jmp_far");
    if (!_IsProtected) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Protected(0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_jmp_far_real(context, newcs, neweip, byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Protected(1)");
        if (_IsSelectorNull(newcs)) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("selector(null)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
        if (_IsDescCodeConform(descriptor))
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_jmp_far_cs_conf(context, newcs, neweip, byte));
        else if (_IsDescCodeNonConform(descriptor))
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_jmp_far_cs_nonc(context, newcs, neweip, byte));
        else if (_IsDescCallGate(descriptor))
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_jmp_far_call_gate(context, newcs));
        else if (_IsDescTaskGate(descriptor))
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_jmp_far_task_gate(context, newcs));
        else if (_IsDescTSS(descriptor))
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_jmp_far_tss(context, newcs));
        else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("newcs(invalid)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    instruction_state.data.opr1 = cpu_state.data.cs.selector;
    instruction_state.data.opr2 = cpu_state.data.eip;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _e_jmp_near(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 neweip, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_e_jmp_near");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_jmp_near(context, neweip, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _e_load_far(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, ntvdm64_type_virtual_address rdest, ntvdm64_type_unsigned_16 selector, ntvdm64_type_unsigned_32 offset, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_e_load_far");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, rsreg, selector));
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, rdest, NTVDM64_TYPE_REFERENCE_OF(offset), 2));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, rdest, NTVDM64_TYPE_REFERENCE_OF(offset), 4));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _e_loopcc(core_machine_cpu_execution_context *context, ntvdm64_type_signed_8 csrc, ntvdm64_type_bool condition) {
    ntvdm64_type_unsigned_32 cecx;
    ntvdm64_type_unsigned_32 neweip = cpu_state.data.eip;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_e_loopcc");
    switch (_GetAddressSize) {
    case 2:
        cpu_state.data.cx--;
        cecx = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.ecx);
        break;
    case 4:
        cpu_state.data.ecx--;
        cecx = NTVDM64_TYPE_MASK_UNSIGNED_32(cpu_state.data.ecx);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    if (cecx && condition) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cecx(!0),condition(1)");
        neweip += csrc;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_jmp_near(context, neweip, _GetOperandSize));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _e_ret_near(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 parambyte, ntvdm64_type_unsigned_8 byte) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_e_ret_near");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_ret_near(context, parambyte, byte));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _e_ret_far(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 parambyte, ntvdm64_type_unsigned_16 byte) {
    ntvdm64_type_unsigned_16 newcs;
    ntvdm64_type_unsigned_32 xs_sel;
    ntvdm64_type_unsigned_32 neweip = 0;
    ntvdm64_type_unsigned_64 descriptor;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_e_ret_far");
    switch (byte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 4));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(neweip), 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 2));
        newcs = NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 8));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(neweip), 4));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kec_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), 4));
        newcs = NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    if (!_IsProtected) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!Protected");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_ret_far_real(context, newcs, neweip, parambyte, byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Protected");
        if (_IsSelectorNull(newcs)) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("selector(null)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
        if (!_IsDescCode(descriptor)) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescCode");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        if (_GetSelector_RPL(newcs) < _GetCPL) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RPL(<CPL)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        if (_IsDescCodeConform(descriptor)) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DescCodeConform");
            if (_GetDesc_DPL(descriptor) > _GetSelector_RPL(newcs)) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DPL(>RPL)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        if (!_IsDescPresent(descriptor)) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        if (_GetSelector_RPL(newcs) > _GetCPL) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RPL(>CPL)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_ret_far_outer(context, newcs, neweip, parambyte, byte));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RPL(<=CPL)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_ser_ret_far_same(context, newcs, neweip, parambyte, byte));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
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

static C_VOID _kaf_calc_CF(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kaf_calc_CF");
    switch (instruction_state.data.type) {
    case ADC8:
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (
                    (_GetEFLAGS_CF && instruction_state.data.opr2 == NTVDM64_TYPE_MAX_UNSIGNED_8) ?
                    1 : ((instruction_state.data.result < instruction_state.data.opr1) || (instruction_state.data.result < instruction_state.data.opr2))));
        break;
    case ADC16:
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (
                    (_GetEFLAGS_CF && instruction_state.data.opr2 == NTVDM64_TYPE_MAX_UNSIGNED_16) ?
                    1 : ((instruction_state.data.result < instruction_state.data.opr1) || (instruction_state.data.result < instruction_state.data.opr2))));
        break;
    case ADC32:
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (
                    (_GetEFLAGS_CF && instruction_state.data.opr2 == NTVDM64_TYPE_MAX_UNSIGNED_32) ?
                    1 : ((instruction_state.data.result < instruction_state.data.opr1) || (instruction_state.data.result < instruction_state.data.opr2))));
        break;
    case ADD8:
    case ADD16:
    case ADD32:
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags,VCPU_EFLAGS_CF,(instruction_state.data.result < instruction_state.data.opr1) || (instruction_state.data.result < instruction_state.data.opr2));
        break;
    case SBB8:
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (instruction_state.data.opr1 < instruction_state.data.result) ||
                (_GetEFLAGS_CF && (instruction_state.data.opr2 == NTVDM64_TYPE_MAX_UNSIGNED_8)));
        break;
    case SBB16:
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (instruction_state.data.opr1 < instruction_state.data.result) ||
                (_GetEFLAGS_CF && (instruction_state.data.opr2 == NTVDM64_TYPE_MAX_UNSIGNED_16)));
        break;
    case SBB32:
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (instruction_state.data.opr1 < instruction_state.data.result) ||
                (_GetEFLAGS_CF && (instruction_state.data.opr2 == NTVDM64_TYPE_MAX_UNSIGNED_32)));
        break;
    case SUB8:
    case SUB16:
    case SUB32:
    case CMP8:
    case CMP16:
    case CMP32:
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, instruction_state.data.opr1 < instruction_state.data.opr2);
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("type");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(instruction_state.data.type));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _kaf_calc_OF(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kaf_calc_OF");
    switch (instruction_state.data.type) {
    case ADC8:
    case ADD8:
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                (NTVDM64_TYPE_GET_MSB_8(instruction_state.data.opr1) == NTVDM64_TYPE_GET_MSB_8(instruction_state.data.opr2)) && (NTVDM64_TYPE_GET_MSB_8(instruction_state.data.opr1) != NTVDM64_TYPE_GET_MSB_8(instruction_state.data.result)));
        break;
    case ADC16:
    case ADD16:
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                (NTVDM64_TYPE_GET_MSB_16(instruction_state.data.opr1) == NTVDM64_TYPE_GET_MSB_16(instruction_state.data.opr2)) && (NTVDM64_TYPE_GET_MSB_16(instruction_state.data.opr1) != NTVDM64_TYPE_GET_MSB_16(instruction_state.data.result)));
        break;
    case ADC32:
    case ADD32:
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                (NTVDM64_TYPE_GET_MSB_32(instruction_state.data.opr1) == NTVDM64_TYPE_GET_MSB_32(instruction_state.data.opr2)) && (NTVDM64_TYPE_GET_MSB_32(instruction_state.data.opr1) != NTVDM64_TYPE_GET_MSB_32(instruction_state.data.result)));
        break;
    case SBB8:
    case SUB8:
    case CMP8:
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                (NTVDM64_TYPE_GET_MSB_8(instruction_state.data.opr1) != NTVDM64_TYPE_GET_MSB_8(instruction_state.data.opr2)) && (NTVDM64_TYPE_GET_MSB_8(instruction_state.data.opr2) == NTVDM64_TYPE_GET_MSB_8(instruction_state.data.result)));
        break;
    case SBB16:
    case SUB16:
    case CMP16:
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                (NTVDM64_TYPE_GET_MSB_16(instruction_state.data.opr1) != NTVDM64_TYPE_GET_MSB_16(instruction_state.data.opr2)) && (NTVDM64_TYPE_GET_MSB_16(instruction_state.data.opr2) == NTVDM64_TYPE_GET_MSB_16(instruction_state.data.result)));
        break;
    case SBB32:
    case SUB32:
    case CMP32:
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                (NTVDM64_TYPE_GET_MSB_32(instruction_state.data.opr1) != NTVDM64_TYPE_GET_MSB_32(instruction_state.data.opr2)) && (NTVDM64_TYPE_GET_MSB_32(instruction_state.data.opr2) == NTVDM64_TYPE_GET_MSB_32(instruction_state.data.result)));
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("type");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(instruction_state.data.type));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _kaf_calc_AF(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kaf_calc_AF");
    NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_AF, ((instruction_state.data.opr1 ^ instruction_state.data.opr2) ^ instruction_state.data.result) & 0x10);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _kaf_calc_PF(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_8 res8 = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.result);
    ntvdm64_type_bool even = 1;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kaf_calc_PF");
    while (res8) {
        even = 1 - even;
        res8 &= res8-1;
    }
    NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_PF, even);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _kaf_calc_ZF(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kaf_calc_ZF");
    NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_ZF, !instruction_state.data.result);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _kaf_calc_SF(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kaf_calc_SF");
    switch (instruction_state.data.bit) {
    case 8:
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_SF, NTVDM64_TYPE_GET_MSB_8(instruction_state.data.result));
        break;
    case 16:
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_SF, NTVDM64_TYPE_GET_MSB_16(instruction_state.data.result));
        break;
    case 32:
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_SF, NTVDM64_TYPE_GET_MSB_32(instruction_state.data.result));
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(instruction_state.data.bit));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _kaf_set_flags(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_16 flags) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kaf_set_flags");
    if (flags & VCPU_EFLAGS_CF) NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_calc_CF(context));
    if (flags & VCPU_EFLAGS_PF) NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_calc_PF(context));
    if (flags & VCPU_EFLAGS_AF) NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_calc_AF(context));
    if (flags & VCPU_EFLAGS_ZF) NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_calc_ZF(context));
    if (flags & VCPU_EFLAGS_SF) NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_calc_SF(context));
    if (flags & VCPU_EFLAGS_OF) NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_calc_OF(context));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _kas_move_index(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 byte, ntvdm64_type_bool flagsi, ntvdm64_type_bool flagdi) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_kas_move_index");
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
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}

#define _kac_arith1(funflag, type8, expr8, type16, expr16, type32, expr32) \
do { \
    switch (bit) { \
    case 8: \
        instruction_state.data.bit = 8; \
        instruction_state.data.type = (type8); \
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_8(cdest); \
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_8(expr8); \
        break; \
    case 16: \
        instruction_state.data.bit = 16; \
        instruction_state.data.type = (type16); \
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest); \
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16(expr16); \
        break; \
    case 32: \
        instruction_state.data.bit = 32; \
        instruction_state.data.type = (type32); \
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest); \
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_32(expr32); \
        break; \
    default: NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit"); \
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit)); \
        NTVDM64_TYPE_TRACE_BLOCK_END;break; \
    } \
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, funflag)); \
} while (0)
#define _kac_arith2(funflag, type8, expr8, type12, expr12, type16, expr16, \
    type20, expr20, type32, expr32) \
do { \
    switch (bit) { \
    case 8: \
        instruction_state.data.bit = 8; \
        instruction_state.data.type = (type8); \
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_8(cdest); \
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_8(csrc); \
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_8(expr8); \
        break; \
    case 12: \
        instruction_state.data.bit = 16; \
        instruction_state.data.type = (type12);\
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest); \
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_16((ntvdm64_type_signed_8)csrc); \
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16(expr12); \
        break; \
    case 16: \
        instruction_state.data.bit = 16; \
        instruction_state.data.type = (type16); \
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest); \
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_16(csrc); \
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16(expr16); \
        break; \
    case 20: \
        instruction_state.data.bit = 32; \
        instruction_state.data.type = (type20); \
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest); \
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_32((ntvdm64_type_signed_8)csrc); \
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_32(expr20); \
        break; \
    case 32: \
        instruction_state.data.bit = 32; \
        instruction_state.data.type = (type32); \
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest); \
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_32(csrc); \
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_32(expr32); \
        break; \
    default: NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit"); \
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit)); \
        NTVDM64_TYPE_TRACE_BLOCK_END;break; \
    } \
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, funflag)); \
} while (0)
static C_VOID _a_add(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_64 csrc, ntvdm64_type_unsigned_8 bit) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_add");
    _kac_arith2(ADD_FLAG,
                ADD8,  (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD16, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD16, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD32, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD32, (instruction_state.data.opr1 + instruction_state.data.opr2));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_adc(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_64 csrc, ntvdm64_type_unsigned_8 bit) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_adc");
    _kac_arith2(ADC_FLAG,
                ADC8,  (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF),
                ADC16, (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF),
                ADC16, (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF),
                ADC32, (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF),
                ADC32, (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_and(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_64 csrc, ntvdm64_type_unsigned_8 bit) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_and");
    _kac_arith2(AND_FLAG,
                AND8,  (instruction_state.data.opr1 & instruction_state.data.opr2),
                AND16, (instruction_state.data.opr1 & instruction_state.data.opr2),
                AND16, (instruction_state.data.opr1 & instruction_state.data.opr2),
                AND32, (instruction_state.data.opr1 & instruction_state.data.opr2),
                AND32, (instruction_state.data.opr1 & instruction_state.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    instruction_state.data.udf |= VCPU_EFLAGS_AF;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_or(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_64 csrc, ntvdm64_type_unsigned_8 bit) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_or");
    _kac_arith2(OR_FLAG,
                OR8,  (instruction_state.data.opr1 | instruction_state.data.opr2),
                OR16, (instruction_state.data.opr1 | instruction_state.data.opr2),
                OR16, (instruction_state.data.opr1 | instruction_state.data.opr2),
                OR32, (instruction_state.data.opr1 | instruction_state.data.opr2),
                OR32, (instruction_state.data.opr1 | instruction_state.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    instruction_state.data.udf |= VCPU_EFLAGS_AF;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_sbb(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_64 csrc, ntvdm64_type_unsigned_8 bit) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_sbb");
    _kac_arith2(SBB_FLAG,
                SBB8,  (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)),
                SBB16, (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)),
                SBB16, (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)),
                SBB32, (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)),
                SBB32, (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_sub(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_64 csrc, ntvdm64_type_unsigned_8 bit) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_sub");
    _kac_arith2(SUB_FLAG,
                SUB8,  (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB16, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB16, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB32, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB32, (instruction_state.data.opr1 - instruction_state.data.opr2));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_xor(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_64 csrc, ntvdm64_type_unsigned_8 bit) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_xor");
    _kac_arith2(XOR_FLAG,
                XOR8,  (instruction_state.data.opr1 ^ instruction_state.data.opr2),
                XOR16, (instruction_state.data.opr1 ^ instruction_state.data.opr2),
                XOR16, (instruction_state.data.opr1 ^ instruction_state.data.opr2),
                XOR32, (instruction_state.data.opr1 ^ instruction_state.data.opr2),
                XOR32, (instruction_state.data.opr1 ^ instruction_state.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    instruction_state.data.udf |= VCPU_EFLAGS_AF;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_cmp(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_64 csrc, ntvdm64_type_unsigned_8 bit) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_cmp");
    _kac_arith2(CMP_FLAG,
                CMP8,  ((ntvdm64_type_unsigned_8)instruction_state.data.opr1 - (ntvdm64_type_signed_8)instruction_state.data.opr2),
                CMP16, ((ntvdm64_type_unsigned_16)instruction_state.data.opr1 - (ntvdm64_type_signed_8)instruction_state.data.opr2),
                CMP16, ((ntvdm64_type_unsigned_16)instruction_state.data.opr1 - (ntvdm64_type_signed_16)instruction_state.data.opr2),
                CMP32, ((ntvdm64_type_unsigned_32)instruction_state.data.opr1 - (ntvdm64_type_signed_8)instruction_state.data.opr2),
                CMP32, ((ntvdm64_type_unsigned_32)instruction_state.data.opr1 - (ntvdm64_type_signed_32)instruction_state.data.opr2));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_test(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_64 csrc, ntvdm64_type_unsigned_8 bit) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_test");
    _kac_arith2(TEST_FLAG,
                TEST8,  (instruction_state.data.opr1 & instruction_state.data.opr2),
                TEST16, (instruction_state.data.opr1 & instruction_state.data.opr2),
                TEST16, (instruction_state.data.opr1 & instruction_state.data.opr2),
                TEST32, (instruction_state.data.opr1 & instruction_state.data.opr2),
                TEST32, (instruction_state.data.opr1 & instruction_state.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    instruction_state.data.udf |= VCPU_EFLAGS_AF;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_inc(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_8 bit) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_inc");
    instruction_state.data.opr2 = 1;
    _kac_arith1(INC_FLAG,
                ADD8,  (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD16, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD32, (instruction_state.data.opr1 + instruction_state.data.opr2));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_dec(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_8 bit) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_dec");
    instruction_state.data.opr2 = 1;
    _kac_arith1(DEC_FLAG,
                SUB8,  (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB16, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB32, (instruction_state.data.opr1 - instruction_state.data.opr2));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_not(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_8 bit) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_not");
    instruction_state.data.opr2 = 0;
    _kac_arith1(NOT_FLAG,
                ARITHTYPE_NULL, (~instruction_state.data.opr1),
                ARITHTYPE_NULL, (~instruction_state.data.opr1),
                ARITHTYPE_NULL, (~instruction_state.data.opr1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_neg(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_8 bit) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_neg");
    instruction_state.data.opr2 = cdest;
    cdest = 0;
    _kac_arith1(NEG_FLAG,
                SUB8,  (NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.opr1) - NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.opr2)),
                SUB16, (NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.opr1) - NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.opr2)),
                SUB32, (NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.opr1) - NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.opr2)));
    NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!instruction_state.data.opr2);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_mul(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 csrc, ntvdm64_type_unsigned_8 bit) {
    ntvdm64_type_unsigned_64 cdest;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_mul");
    switch (bit) {
    case 8:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = cpu_state.data.al;
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_8(csrc);
        cdest = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.al * instruction_state.data.opr2);
        cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest);
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF, !!cpu_state.data.ah);
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!cpu_state.data.ah);
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = cpu_state.data.ax;
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_16(csrc);
        cdest = NTVDM64_TYPE_MASK_UNSIGNED_32(cpu_state.data.ax * instruction_state.data.opr2);
        cpu_state.data.dx = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest >> 16);
        cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest);
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF, !!cpu_state.data.dx);
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!cpu_state.data.dx);
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = cpu_state.data.eax;
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_32(csrc);
        cdest = NTVDM64_TYPE_MASK_UNSIGNED_64(cpu_state.data.eax * instruction_state.data.opr2);
        cpu_state.data.edx = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest >> 32);
        cpu_state.data.eax = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest);
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF, !!cpu_state.data.edx);
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!cpu_state.data.edx);
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_64(cdest);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                         VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_imul(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 csrc, ntvdm64_type_unsigned_8 bit) {
    ntvdm64_type_signed_64 cdest;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_imul");
    switch (bit) {
    case 8:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = cpu_state.data.al;
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_8((ntvdm64_type_signed_8)csrc);
        cdest = NTVDM64_TYPE_MASK_UNSIGNED_16((ntvdm64_type_signed_8)cpu_state.data.al * (ntvdm64_type_signed_8)instruction_state.data.opr2);
        cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest);
        if (NTVDM64_TYPE_MASK_UNSIGNED_16(cdest) == (ntvdm64_type_signed_16)((ntvdm64_type_signed_8)cpu_state.data.al)) {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        } else {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = cpu_state.data.ax;
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_16((ntvdm64_type_signed_16)csrc);
        cdest = NTVDM64_TYPE_MASK_UNSIGNED_32((ntvdm64_type_signed_16)cpu_state.data.ax * (ntvdm64_type_signed_16)instruction_state.data.opr2);
        cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest);
        cpu_state.data.dx = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest >> 16);
        if (NTVDM64_TYPE_MASK_UNSIGNED_32(cdest) == (ntvdm64_type_signed_32)((ntvdm64_type_signed_16)cpu_state.data.ax)) {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        } else {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = cpu_state.data.eax;
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_32((ntvdm64_type_signed_32)csrc);
        cdest = NTVDM64_TYPE_MASK_UNSIGNED_64((ntvdm64_type_signed_32)cpu_state.data.eax * (ntvdm64_type_signed_32)instruction_state.data.opr2);
        cpu_state.data.eax = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest);
        cpu_state.data.edx = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest >> 32);
        if (NTVDM64_TYPE_MASK_UNSIGNED_64(cdest) == (ntvdm64_type_signed_64)((ntvdm64_type_signed_32)cpu_state.data.eax)) {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        } else {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_64(cdest);
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                         VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_div(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 csrc, ntvdm64_type_unsigned_8 bit) {
    ntvdm64_type_unsigned_64 temp = 0x0000000000000000;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_div");
    switch (bit) {
    case 8:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.ax);
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_8(csrc);
        if (!instruction_state.data.opr2) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            temp = NTVDM64_TYPE_MASK_UNSIGNED_16((ntvdm64_type_unsigned_16)instruction_state.data.opr1 / (ntvdm64_type_unsigned_8)instruction_state.data.opr2);
            if (temp > NTVDM64_TYPE_MAX_UNSIGNED_8) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("temp(>0xff)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                cpu_state.data.al = NTVDM64_TYPE_MASK_UNSIGNED_8(temp);
                cpu_state.data.ah = NTVDM64_TYPE_MASK_UNSIGNED_8((ntvdm64_type_unsigned_16)instruction_state.data.opr1 % (ntvdm64_type_unsigned_8)instruction_state.data.opr2);
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        instruction_state.data.result = cpu_state.data.ax;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_32((cpu_state.data.dx << 16) | cpu_state.data.ax);
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_16(csrc);
        if (!instruction_state.data.opr2) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            temp = NTVDM64_TYPE_MASK_UNSIGNED_32((ntvdm64_type_unsigned_32)instruction_state.data.opr1 / (ntvdm64_type_unsigned_16)instruction_state.data.opr2);
            if (temp > NTVDM64_TYPE_MAX_UNSIGNED_16) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("temp(>0xffff)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(temp);
                cpu_state.data.dx = NTVDM64_TYPE_MASK_UNSIGNED_16((ntvdm64_type_unsigned_32)instruction_state.data.opr1 % (ntvdm64_type_unsigned_16)instruction_state.data.opr2);
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        instruction_state.data.result = (cpu_state.data.dx << 16) | cpu_state.data.ax;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_64(((ntvdm64_type_unsigned_64)cpu_state.data.edx << 32) | cpu_state.data.eax);
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_32(csrc);
        if (!instruction_state.data.opr2) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            temp = NTVDM64_TYPE_MASK_UNSIGNED_64((ntvdm64_type_unsigned_64)instruction_state.data.opr1 / (ntvdm64_type_unsigned_32)instruction_state.data.opr2);
            if (temp > NTVDM64_TYPE_MAX_UNSIGNED_32) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("temp(>0xffffffff)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                cpu_state.data.eax = NTVDM64_TYPE_MASK_UNSIGNED_32(temp);
                cpu_state.data.edx = NTVDM64_TYPE_MASK_UNSIGNED_32((ntvdm64_type_unsigned_64)instruction_state.data.opr1 % (ntvdm64_type_unsigned_32)instruction_state.data.opr2);
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        instruction_state.data.result = ((ntvdm64_type_unsigned_64)cpu_state.data.edx << 32) | cpu_state.data.eax;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF |
                         VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_OF);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_idiv(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 csrc, ntvdm64_type_unsigned_8 bit) {
    ntvdm64_type_signed_64 temp;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_idiv");
    switch (bit) {
    case 8:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_16((ntvdm64_type_signed_16)cpu_state.data.ax);
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_8((ntvdm64_type_signed_8)csrc);
        if (!instruction_state.data.opr2) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            temp = NTVDM64_TYPE_MASK_UNSIGNED_16((ntvdm64_type_signed_16)instruction_state.data.opr1 / (ntvdm64_type_signed_8)instruction_state.data.opr2);
            if ((temp > 0 && NTVDM64_TYPE_MASK_UNSIGNED_8(temp) > 0x7f) ||
                    (temp < 0 && NTVDM64_TYPE_MASK_UNSIGNED_8(temp) < 0x80)) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("temp(>0x7f/<0x80)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                cpu_state.data.al = NTVDM64_TYPE_MASK_UNSIGNED_8(temp);
                cpu_state.data.ah = NTVDM64_TYPE_MASK_UNSIGNED_8((ntvdm64_type_signed_16)instruction_state.data.opr1 % (ntvdm64_type_signed_8)instruction_state.data.opr2);
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        instruction_state.data.result = cpu_state.data.ax;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_32((ntvdm64_type_signed_32)((cpu_state.data.dx << 16) | cpu_state.data.ax));
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_16((ntvdm64_type_signed_16)csrc);
        if (!instruction_state.data.opr2) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            temp = NTVDM64_TYPE_MASK_UNSIGNED_32((ntvdm64_type_signed_32)instruction_state.data.opr1 / (ntvdm64_type_signed_16)instruction_state.data.opr2);
            if ((temp > 0 && NTVDM64_TYPE_MASK_UNSIGNED_16(temp) > 0x7fff) ||
                    (temp < 0 && NTVDM64_TYPE_MASK_UNSIGNED_16(temp) < 0x8000)) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("temp(>0x7fff/<0x8000)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(temp);
                cpu_state.data.dx = NTVDM64_TYPE_MASK_UNSIGNED_16((ntvdm64_type_signed_32)instruction_state.data.opr1 % (ntvdm64_type_signed_16)instruction_state.data.opr2);
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        instruction_state.data.result = ((ntvdm64_type_unsigned_32)cpu_state.data.dx << 16) | cpu_state.data.ax;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_64((ntvdm64_type_signed_64)(((ntvdm64_type_unsigned_64)cpu_state.data.edx << 32) | cpu_state.data.eax));
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_32((ntvdm64_type_signed_32)csrc);
        if (!instruction_state.data.opr2) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            temp = NTVDM64_TYPE_MASK_UNSIGNED_64((ntvdm64_type_signed_64)instruction_state.data.opr1 / (ntvdm64_type_signed_32)instruction_state.data.opr2);
            if ((temp > 0 && NTVDM64_TYPE_MASK_UNSIGNED_32(temp) > 0x7fffffff) ||
                    (temp < 0 && NTVDM64_TYPE_MASK_UNSIGNED_32(temp) < 0x80000000)) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("temp(>0x7fffffff/<0x80000000)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                cpu_state.data.eax = NTVDM64_TYPE_MASK_UNSIGNED_32(temp);
                cpu_state.data.edx = NTVDM64_TYPE_MASK_UNSIGNED_32((ntvdm64_type_signed_64)instruction_state.data.opr1 % (ntvdm64_type_signed_32)instruction_state.data.opr2);
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        instruction_state.data.result = ((ntvdm64_type_unsigned_64)cpu_state.data.edx << 32) | cpu_state.data.eax;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF |
                         VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_OF);
    NTVDM64_TYPE_TRACE_CALL_END;
}

static C_VOID _a_imul3(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 csrc1, ntvdm64_type_unsigned_64 csrc2, ntvdm64_type_unsigned_8 bit) {
    ntvdm64_type_signed_64 cdest;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_imul3");
    switch (bit) {
    case 12:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16+8)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = (ntvdm64_type_signed_16)csrc1;
        instruction_state.data.opr2 = (ntvdm64_type_signed_8)csrc2;
        cdest = NTVDM64_TYPE_MASK_UNSIGNED_32((ntvdm64_type_signed_16)instruction_state.data.opr1 * (ntvdm64_type_signed_8)instruction_state.data.opr2);
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest);
        if (NTVDM64_TYPE_MASK_UNSIGNED_32(cdest) != NTVDM64_TYPE_MASK_UNSIGNED_32((ntvdm64_type_signed_16)instruction_state.data.result)) {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        } else {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16+16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = (ntvdm64_type_signed_16)csrc1;
        instruction_state.data.opr2 = (ntvdm64_type_signed_16)csrc2;
        cdest = NTVDM64_TYPE_MASK_UNSIGNED_32((ntvdm64_type_signed_16)instruction_state.data.opr1 * (ntvdm64_type_signed_16)instruction_state.data.opr2);
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest);
        if (NTVDM64_TYPE_MASK_UNSIGNED_32(cdest) != NTVDM64_TYPE_MASK_UNSIGNED_32((ntvdm64_type_signed_16)instruction_state.data.result)) {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        } else {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 20:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32+8)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = (ntvdm64_type_signed_32)csrc1;
        instruction_state.data.opr2 = (ntvdm64_type_signed_8)csrc2;
        cdest = NTVDM64_TYPE_MASK_UNSIGNED_64((ntvdm64_type_signed_32)instruction_state.data.opr1 * (ntvdm64_type_signed_8)instruction_state.data.opr2);
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest);
        if (NTVDM64_TYPE_MASK_UNSIGNED_64(cdest) != NTVDM64_TYPE_MASK_UNSIGNED_64((ntvdm64_type_signed_32)instruction_state.data.result)) {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        } else {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32+32");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = (ntvdm64_type_signed_32)csrc1;
        instruction_state.data.opr2 = (ntvdm64_type_signed_32)csrc2;
        cdest = NTVDM64_TYPE_MASK_UNSIGNED_64((ntvdm64_type_signed_32)instruction_state.data.opr1 * (ntvdm64_type_signed_32)instruction_state.data.opr2);
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest);
        if (NTVDM64_TYPE_MASK_UNSIGNED_64(cdest) != NTVDM64_TYPE_MASK_UNSIGNED_64((ntvdm64_type_signed_32)instruction_state.data.result)) {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        } else {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                         VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    NTVDM64_TYPE_TRACE_CALL_END;
}

static C_VOID _a_rol(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_8 csrc, ntvdm64_type_unsigned_8 bit) {
    ntvdm64_type_unsigned_8 count;
    ntvdm64_type_unsigned_32 flagcf;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_rol");
    switch (bit) {
    case 8:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        count = csrc & 0x07;
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_8(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            flagcf = !!NTVDM64_TYPE_GET_MSB_8(instruction_state.data.result);
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.result << 1) | flagcf;
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!NTVDM64_TYPE_GET_LSB_UNSIGNED_8(instruction_state.data.result));
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!NTVDM64_TYPE_GET_MSB_8(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        count = csrc & 0x0f;
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            flagcf = !!NTVDM64_TYPE_GET_MSB_16(instruction_state.data.result);
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result << 1) | flagcf;
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!NTVDM64_TYPE_GET_LSB_UNSIGNED_16(instruction_state.data.result));
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!NTVDM64_TYPE_GET_MSB_16(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        count = csrc & 0x1f;
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            flagcf = !!NTVDM64_TYPE_GET_MSB_32(instruction_state.data.result);
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result << 1) | flagcf;
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!NTVDM64_TYPE_GET_LSB_UNSIGNED_32(instruction_state.data.result));
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!NTVDM64_TYPE_GET_MSB_32(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_ror(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_8 csrc, ntvdm64_type_unsigned_8 bit) {
    ntvdm64_type_unsigned_8 count;
    ntvdm64_type_unsigned_32 flagcf;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_ror");
    switch (bit) {
    case 8:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        count = csrc & 0x07;
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_8(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            flagcf = NTVDM64_TYPE_GET_LSB_UNSIGNED_8(instruction_state.data.result) ? NTVDM64_TYPE_MSB_8 : 0;
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.result >> 1) | flagcf;
            count--;
        }
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!NTVDM64_TYPE_GET_MSB_8(instruction_state.data.result));
        if (instruction_state.data.opr2 == 1)
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!NTVDM64_TYPE_GET_MSB_8(instruction_state.data.result)) ^ (!!NTVDM64_TYPE_GET_MSB_7(instruction_state.data.result))));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        count = csrc & 0x0f;
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            flagcf = NTVDM64_TYPE_GET_LSB_UNSIGNED_16(instruction_state.data.result) ? NTVDM64_TYPE_MSB_16 : 0;
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result >> 1) | flagcf;
            count--;
        }
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!NTVDM64_TYPE_GET_MSB_16(instruction_state.data.result));
        if (instruction_state.data.opr2 == 1)
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!NTVDM64_TYPE_GET_MSB_16(instruction_state.data.result)) ^ (!!NTVDM64_TYPE_GET_MSB_15(instruction_state.data.result))));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        count = csrc & 0x1f;
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            flagcf = NTVDM64_TYPE_GET_LSB_UNSIGNED_32(instruction_state.data.result) ? NTVDM64_TYPE_MSB_32 : 0;
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result >> 1) | flagcf;
            count--;
        }
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!NTVDM64_TYPE_GET_MSB_32(instruction_state.data.result));
        if (instruction_state.data.opr2 == 1)
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!NTVDM64_TYPE_GET_MSB_32(instruction_state.data.result)) ^ (!!NTVDM64_TYPE_GET_MSB_31(instruction_state.data.result))));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_rcl(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_8 csrc, ntvdm64_type_unsigned_8 bit) {
    ntvdm64_type_unsigned_8 count;
    ntvdm64_type_unsigned_32 flagcf;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_rcl");
    switch (bit) {
    case 8:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        count = (csrc & 0x1f) % 9;
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_8(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            flagcf = !!NTVDM64_TYPE_GET_MSB_8(instruction_state.data.result);
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.result << 1) | _GetEFLAGS_CF;
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, flagcf);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!NTVDM64_TYPE_GET_MSB_8(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        count = (csrc & 0x1f) % 17;
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            flagcf = !!NTVDM64_TYPE_GET_MSB_16(instruction_state.data.result);
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result << 1) | _GetEFLAGS_CF;
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, flagcf);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!NTVDM64_TYPE_GET_MSB_16(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        count = (csrc & 0x1f);
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            flagcf = !!NTVDM64_TYPE_GET_MSB_32(instruction_state.data.result);
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result << 1) | _GetEFLAGS_CF;
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, flagcf);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!NTVDM64_TYPE_GET_MSB_32(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_rcr(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_8 csrc, ntvdm64_type_unsigned_8 bit) {
    ntvdm64_type_unsigned_8 count;
    ntvdm64_type_unsigned_32 flagcf;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_rcr");
    switch (bit) {
    case 8:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        count = (csrc & 0x1f) % 9;
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_8(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (instruction_state.data.opr2 == 1)
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!NTVDM64_TYPE_GET_MSB_8(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        while (count) {
            flagcf = NTVDM64_TYPE_GET_LSB_UNSIGNED_8(instruction_state.data.result);
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.result >> 1) | (_GetEFLAGS_CF ? NTVDM64_TYPE_MSB_8 : 0);
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!flagcf);
            count--;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        count = (csrc & 0x1f) % 17;
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (instruction_state.data.opr2 == 1)
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!NTVDM64_TYPE_GET_MSB_16(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        while (count) {
            flagcf = NTVDM64_TYPE_GET_LSB_UNSIGNED_16(instruction_state.data.result);
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result >> 1) | (_GetEFLAGS_CF ? NTVDM64_TYPE_MSB_16 : 0);
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!flagcf);
            count--;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        count = (csrc & 0x1f);
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (instruction_state.data.opr2 == 1)
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!NTVDM64_TYPE_GET_MSB_32(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        while (count) {
            flagcf = NTVDM64_TYPE_GET_LSB_UNSIGNED_32(instruction_state.data.result);
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result >> 1) | (_GetEFLAGS_CF ? NTVDM64_TYPE_MSB_32 : 0);
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!flagcf);
            count--;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_shl(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_8 csrc, ntvdm64_type_unsigned_8 bit) {
    ntvdm64_type_unsigned_8 count;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_shl");
    count = csrc & 0x1f;
    instruction_state.data.opr2 = count;
    if (count >= bit) instruction_state.data.udf |= VCPU_EFLAGS_CF;
    switch (bit) {
    case 8:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_8(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!NTVDM64_TYPE_GET_MSB_8(instruction_state.data.result));
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.result << 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!NTVDM64_TYPE_GET_MSB_8(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHL_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags,VCPU_EFLAGS_CF, !!NTVDM64_TYPE_GET_MSB_16(instruction_state.data.result));
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result << 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!NTVDM64_TYPE_GET_MSB_16(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHL_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags,VCPU_EFLAGS_CF, !!NTVDM64_TYPE_GET_MSB_32(instruction_state.data.result));
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result << 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                    ((!!NTVDM64_TYPE_GET_MSB_32(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHL_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_shr(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_8 csrc, ntvdm64_type_unsigned_8 bit) {
    ntvdm64_type_unsigned_8 count;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_shr");
    count = csrc & 0x1f;
    instruction_state.data.opr2 = count;
    if (count >= bit) instruction_state.data.udf |= VCPU_EFLAGS_CF;
    switch (bit) {
    case 8:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_8(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!NTVDM64_TYPE_GET_LSB_UNSIGNED_8(instruction_state.data.result)));
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF, (!!NTVDM64_TYPE_GET_MSB_8(instruction_state.data.opr1)));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags,VCPU_EFLAGS_CF, (!!NTVDM64_TYPE_GET_LSB_UNSIGNED_16(instruction_state.data.result)));
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF, (!!NTVDM64_TYPE_GET_MSB_16(instruction_state.data.opr1)));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags,VCPU_EFLAGS_CF, (!!NTVDM64_TYPE_GET_LSB_UNSIGNED_32(instruction_state.data.result)));
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF, (!!NTVDM64_TYPE_GET_MSB_32(instruction_state.data.opr1)));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_sar(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_8 csrc, ntvdm64_type_unsigned_8 bit) {
    ntvdm64_type_unsigned_8 count;
    ntvdm64_type_bool tempcf;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_shr");
    count = csrc & 0x1f;
    instruction_state.data.opr2 = count;
    switch (bit) {
    case 8:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_8(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!NTVDM64_TYPE_GET_LSB_UNSIGNED_8(instruction_state.data.result)));
            tempcf = NTVDM64_TYPE_GET_MSB_8(instruction_state.data.result);
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_8((ntvdm64_type_signed_8)instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1) _ClrEFLAGS_OF;
        else instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SAR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!NTVDM64_TYPE_GET_LSB_UNSIGNED_16(instruction_state.data.result)));
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16((ntvdm64_type_signed_16)instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1) _ClrEFLAGS_OF;
        else instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SAR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count) {
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!NTVDM64_TYPE_GET_LSB_UNSIGNED_32(instruction_state.data.result)));
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_32((ntvdm64_type_signed_32)instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1) _ClrEFLAGS_OF;
        else instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SAR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}

static C_VOID _p_ins(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_32 cedi, data = 0;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_p_ins");
    switch (_GetAddressSize) {
    case 2:
        cedi = cpu_state.data.di;
        break;
    case 4:
        cedi = cpu_state.data.edi;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (byte) {
    case 1:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.es, cedi, 1, 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, NTVDM64_TYPE_REFERENCE_OF(data), 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_write_es(context, cedi, NTVDM64_TYPE_REFERENCE_OF(data), 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 0, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.es, cedi, 2, 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, NTVDM64_TYPE_REFERENCE_OF(data), 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_write_es(context, cedi, NTVDM64_TYPE_REFERENCE_OF(data), 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 0, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.es, cedi, 4, 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, NTVDM64_TYPE_REFERENCE_OF(data), 4));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_write_es(context, cedi, NTVDM64_TYPE_REFERENCE_OF(data), 4));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 0, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _p_outs(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_32 cesi, data = 0;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_p_outs");
    switch (_GetAddressSize) {
    case 2:
        cesi = cpu_state.data.si;
        break;
    case 4:
        cesi = cpu_state.data.esi;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (byte) {
    case 1:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, NTVDM64_TYPE_REFERENCE_OF(data), 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx, NTVDM64_TYPE_REFERENCE_OF(data), 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 1, 0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, NTVDM64_TYPE_REFERENCE_OF(data), 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx, NTVDM64_TYPE_REFERENCE_OF(data), 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 1, 0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, NTVDM64_TYPE_REFERENCE_OF(data), 4));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx, NTVDM64_TYPE_REFERENCE_OF(data), 4));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 1, 0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _m_movs(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_32 data = 0;
    ntvdm64_type_unsigned_32 cesi, cedi;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_m_movs");
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
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (byte) {
    case 1:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, NTVDM64_TYPE_REFERENCE_OF(data), 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_write_es(context, cedi, NTVDM64_TYPE_REFERENCE_OF(data), 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 1, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, NTVDM64_TYPE_REFERENCE_OF(data), 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_write_es(context, cedi, NTVDM64_TYPE_REFERENCE_OF(data), 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 1, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, NTVDM64_TYPE_REFERENCE_OF(data), 4));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_write_es(context, cedi, NTVDM64_TYPE_REFERENCE_OF(data), 4));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 1, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _m_stos(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_32 cedi;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_m_stos");
    switch (_GetAddressSize) {
    case 2:
        cedi = cpu_state.data.di;
        break;
    case 4:
        cedi = cpu_state.data.edi;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (byte) {
    case 1:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_write_es(context, cedi, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.al), 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 0, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_write_es(context, cedi, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 0, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_write_es(context, cedi, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 0, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _m_lods(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_unsigned_32 cesi = 0x00000000;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_m_lods");
    switch (_GetAddressSize) {
    case 2:
        cesi = cpu_state.data.si;
        break;
    case 4:
        cesi = cpu_state.data.esi;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (byte) {
    case 1:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(1)");
        instruction_state.data.bit = 8;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.al), 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 1, 0));
        instruction_state.data.result = cpu_state.data.al;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        instruction_state.data.bit = 16;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 1, 0));
        instruction_state.data.result = cpu_state.data.ax;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        instruction_state.data.bit = 32;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 1, 0));
        instruction_state.data.result = cpu_state.data.eax;
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("byte");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_cmps(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 bit) {
    ntvdm64_type_unsigned_32 cesi, cedi;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_cmps");
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
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (bit) {
    case 8:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.type = CMP8;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.opr1), 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_es(context, cedi, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.opr2), 1));
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.opr1 - instruction_state.data.opr2);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 1, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.type = CMP16;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.opr1), 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_es(context, cedi, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.opr2), 2));
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.opr1 - instruction_state.data.opr2);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 1, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.type = CMP32;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.opr1), 4));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_es(context, cedi, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.opr2), 4));
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.opr1 - instruction_state.data.opr2);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 1, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, CMP_FLAG));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_scas(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 bit) {
    ntvdm64_type_unsigned_32 cedi;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_scas");
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
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (bit) {
    case 8:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.type = CMP8;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(instruction_state.data.opr1 = cpu_state.data.al);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_es(context, cedi, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.opr2), 1));
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.opr1 - instruction_state.data.opr2);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 0, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.type = CMP16;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(instruction_state.data.opr1 = cpu_state.data.ax);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_es(context, cedi, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.opr2), 2));
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.opr1 - instruction_state.data.opr2);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 0, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.type = CMP32;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(instruction_state.data.opr1 = cpu_state.data.eax);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_es(context, cedi, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.opr2), 4));
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.opr1 - instruction_state.data.opr2);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 0, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, CMP_FLAG));
    NTVDM64_TYPE_TRACE_CALL_END;
}
#define _adv NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_skip(context, 1))

core_machine_cpu_instruction_metadata core_machine_cpu_instruction_metadata_get(
    core_machine_cpu_instruction_space space, uint8_t opcode, uint8_t modrm)
{
    core_machine_cpu_instruction_metadata metadata = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_FPU_PROFILE_NONE, 1
    };

    switch (space) {
    case CORE_MACHINE_CPU_INSTRUCTION_PRIMARY:
        if (opcode >= 0xd8u && opcode <= 0xdfu) {
            metadata.minimum_fpu = CORE_MACHINE_FPU_PROFILE_8087;
        } else if ((opcode >= 0x60u && opcode <= 0x62u) || opcode == 0x68u ||
                   opcode == 0x69u || opcode == 0x6au || opcode == 0x6bu ||
                   (opcode >= 0x6cu && opcode <= 0x6fu) || opcode == 0xc0u ||
                   opcode == 0xc1u || opcode == 0xc8u || opcode == 0xc9u) {
            metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80186;
        } else if (opcode == 0x63u) {
            metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80286;
        } else if (opcode >= 0x64u && opcode <= 0x67u) {
            metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80386;
        } else if (opcode == 0x82u || opcode == 0xd6u || opcode == 0xf1u) {
            metadata.valid = 0;
        }
        break;
    case CORE_MACHINE_CPU_INSTRUCTION_0F:
        metadata.valid = 0;
        if (opcode == 0x00u) {
            metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80286;
            metadata.valid = ((modrm >> 3u) & 7u) <= 3u;
        } else if (opcode == 0x01u) {
            metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80286;
            metadata.valid = ((modrm >> 3u) & 7u) <= 5u;
        } else if (opcode == 0x02u || opcode == 0x03u) {
            metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80286;
            metadata.valid = 1;
        } else if (opcode == 0x06u ||
                   (opcode >= 0x20u && opcode <= 0x26u) ||
                   (opcode >= 0x80u && opcode <= 0x8fu) ||
                   (opcode >= 0x90u && opcode <= 0x9fu) ||
                   opcode == 0xa0u || opcode == 0xa1u || opcode == 0xa3u ||
                   opcode == 0xa4u || opcode == 0xa5u || opcode == 0xa8u ||
                   opcode == 0xa9u || opcode == 0xabu || opcode == 0xacu ||
                   opcode == 0xadu || opcode == 0xafu ||
                   (opcode >= 0xb2u && opcode <= 0xb7u) ||
                   (opcode >= 0xbbu && opcode <= 0xbfu)) {
            metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80386;
            metadata.valid = 1;
        } else if (opcode == 0xbau) {
            metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80386;
            metadata.valid = ((modrm >> 3u) & 7u) >= 4u;
        }
        break;
    case CORE_MACHINE_CPU_INSTRUCTION_FPU_ESCAPE:
        metadata.minimum_fpu = CORE_MACHINE_FPU_PROFILE_8087;
        metadata.valid = opcode >= 0xd8u && opcode <= 0xdfu;
        break;
    }
    return metadata;
}

static C_VOID UndefinedOpcode(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("UndefinedOpcode");
    cpu_state = instruction_state.data.oldcpu;
    if (!_GetCR0_PE) {
        STD_PRINTF("The NXVM CPU has encountered an illegal instruction at L%08X.\n", cpu_state.data.cs.base + cpu_state.data.eip);
        core_machine_cpu_execution_request_stop(context);
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID ADD_RM8_R8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD_RM8_R8");
    i386(0x00) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID ADD_RM32_R32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD_RM32_R32");
    i386(0x01) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID ADD_R8_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD_R8_RM8");
    i386(0x02) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID ADD_R32_RM32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD_R32_RM32");
    i386(0x03) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID ADD_AL_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD_AL_I8");
    i386(0x04) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_add(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.result);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID ADD_EAX_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADD_EAX_I32");
    i386(0x05) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_add(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_add(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_add(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_ES(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_32 xs_sel;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_ES");
    i386(0x06) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    xs_sel = cpu_state.data.es.selector;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID POP_ES(core_machine_cpu_execution_context *context) {
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    ntvdm64_type_unsigned_32 xs_sel;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_ES");
    i386(0x07) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_es(context, NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID OR_RM8_R8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR_RM8_R8");
    i386(0x08) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID OR_RM32_R32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR_RM32_R32");
    i386(0x09) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID OR_R8_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR_R8_RM8");
    i386(0x0a) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID OR_R32_RM32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR_R32_RM32");
    i386(0x0b) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID OR_AL_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR_AL_I8");
    i386(0x0c) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_or(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.result);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID OR_EAX_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OR_EAX_I32");
    i386(0x0d) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_or(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_or(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_or(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_CS(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_32 xs_sel;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_CS");
    i386(0x0e) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    xs_sel = cpu_state.data.cs.selector;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID POP_CS(core_machine_cpu_execution_context *context) {
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    ntvdm64_type_unsigned_32 xs_sel;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_CS");
    i386(0x0f) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_cs(context, NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_0F(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_8 opcode = 0x00;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_0F");
    i386(0x0f) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, NTVDM64_TYPE_REFERENCE_OF(opcode), 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(ExecCpuInstruction(instruction_state.connect.insTable_0f[opcode]));
    }
    else
        POP_CS(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID ADC_RM8_R8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC_RM8_R8");
    i386(0x10) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID ADC_RM32_R32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC_RM32_R32");
    i386(0x11) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID ADC_R8_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC_R8_RM8");
    i386(0x12) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID ADC_R32_RM32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC_R32_RM32");
    i386(0x13) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID ADC_AL_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC_AL_I8");
    i386(0x14) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_adc(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.result);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID ADC_EAX_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ADC_EAX_I32");
    i386(0x15) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_adc(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_adc(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_adc(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_SS(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_32 xs_sel;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_SS");
    i386(0x16) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    xs_sel = cpu_state.data.ss.selector;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID POP_SS(core_machine_cpu_execution_context *context) {
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    ntvdm64_type_unsigned_32 xs_sel;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_SS");
    i386(0x17) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_ss(context, NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SBB_RM8_R8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB_RM8_R8");
    i386(0x18) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SBB_RM32_R32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB_RM32_R32");
    i386(0x19) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SBB_R8_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB_R8_RM8");
    i386(0x1a) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SBB_R32_RM32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB_R32_RM32");
    i386(0x1b) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SBB_AL_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB_AL_I8");
    i386(0x1c) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sbb(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.result);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SBB_EAX_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SBB_EAX_I32");
    i386(0x1d) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sbb(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sbb(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sbb(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_DS(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_32 xs_sel;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_DS");
    i386(0x1e) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    xs_sel = cpu_state.data.ds.selector;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID POP_DS(core_machine_cpu_execution_context *context) {
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    ntvdm64_type_unsigned_32 xs_sel;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_DS");
    i386(0x1f) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_ds(context, NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID AND_RM8_R8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND_RM8_R8");
    i386(0x20) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID AND_RM32_R32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND_RM32_R32");
    i386(0x21) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID AND_R8_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND_R8_RM8");
    i386(0x22) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID AND_R32_RM32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND_R32_RM32");
    i386(0x23) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID AND_AL_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND_AL_I8");
    i386(0x24) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_and(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.result);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID AND_EAX_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AND_EAX_I32");
    i386(0x25) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_and(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_and(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_and(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_ES(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_ES");
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
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID DAA(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_8 oldAL = cpu_state.data.al;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DAA");
    i386(0x27)
    _adv;
    else
        cpu_state.data.ip++;
    if (((cpu_state.data.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
        cpu_state.data.al += 0x06;
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                _GetEFLAGS_CF || ((cpu_state.data.al < oldAL) || (cpu_state.data.al < 0x06)));
        _SetEFLAGS_AF;
    } else _ClrEFLAGS_AF;
    if (((cpu_state.data.al & 0xf0) > 0x90) || _GetEFLAGS_CF) {
        cpu_state.data.al += 0x60;
        _SetEFLAGS_CF;
    } else _ClrEFLAGS_CF;
    instruction_state.data.bit = 8;
    instruction_state.data.result = (ntvdm64_type_unsigned_32)cpu_state.data.al;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, DAA_FLAG));
    instruction_state.data.udf |= VCPU_EFLAGS_OF;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SUB_RM8_R8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB_RM8_R8");
    i386(0x28) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SUB_RM32_R32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB_RM32_R32");
    i386(0x29) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SUB_R8_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB_R8_RM8");
    i386(0x2a) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SUB_R32_RM32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB_R32_RM32");
    i386(0x2b) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SUB_AL_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB_AL_I8");
    i386(0x2c) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sub(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.result);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SUB_EAX_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SUB_EAX_I32");
    i386(0x2d) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sub(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sub(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sub(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_CS(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_CS");
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
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID DAS(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_8 oldAL = cpu_state.data.al;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DAS");
    i386(0x2f)
    _adv;
    else
        cpu_state.data.ip++;
    if (((cpu_state.data.al & 0x0f) > 0x09) || _GetEFLAGS_AF) {
        cpu_state.data.al -= 0x06;
        NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                _GetEFLAGS_CF || (oldAL < 0x06));
        _SetEFLAGS_AF;
    } else _ClrEFLAGS_AF;
    if ((cpu_state.data.al > 0x9f) || _GetEFLAGS_CF) {
        cpu_state.data.al -= 0x60;
        _SetEFLAGS_CF;
    } else _ClrEFLAGS_CF;
    instruction_state.data.bit = 8;
    instruction_state.data.result = (ntvdm64_type_unsigned_32)cpu_state.data.al;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, DAS_FLAG));
    instruction_state.data.udf |= VCPU_EFLAGS_OF;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID XOR_RM8_R8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR_RM8_R8");
    i386(0x30) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID XOR_RM32_R32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR_RM32_R32");
    i386(0x31) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID XOR_R8_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR_R8_RM8");
    i386(0x32) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID XOR_R32_RM32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR_R32_RM32");
    i386(0x33) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID XOR_AL_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR_AL_I8");
    i386(0x34) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_xor(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.result);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID XOR_EAX_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XOR_EAX_I32");
    i386(0x35) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperxorSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_xor(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperxorSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_xor(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_xor(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_SS(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_SS");
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
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID AAA(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AAA");
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
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID CMP_RM8_R8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP_RM8_R8");
    i386(0x38) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID CMP_RM32_R32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP_RM32_R32");
    i386(0x39) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cr, 16));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID CMP_R8_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP_R8_RM8");
    i386(0x3a) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID CMP_R32_RM32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP_R32_RM32");
    i386(0x3b) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.cr, instruction_state.data.crm, 16));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID CMP_AL_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP_AL_I8");
    i386(0x3c) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmp(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID CMP_EAX_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMP_EAX_I32");
    i386(0x3d) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmp(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmp(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmp(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_DS(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_DS");
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
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID AAS(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AAS");
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
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INC_EAX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_EAX");
    i386(0x40) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.ax, 16));
            cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.eax, 32));
            cpu_state.data.eax = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.ax, 16));
        cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INC_ECX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_ECX");
    i386(0x41) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.cx, 16));
            cpu_state.data.cx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.ecx, 32));
            cpu_state.data.ecx = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.cx, 16));
        cpu_state.data.cx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INC_EDX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_EDX");
    i386(0x42) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.dx, 16));
            cpu_state.data.dx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.edx, 32));
            cpu_state.data.edx = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.dx, 16));
        cpu_state.data.dx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INC_EBX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_EBX");
    i386(0x43) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.bx, 16));
            cpu_state.data.bx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.ebx, 32));
            cpu_state.data.ebx = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.bx, 16));
        cpu_state.data.bx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INC_ESP(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_ESP");
    i386(0x44) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.sp, 16));
            cpu_state.data.sp = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.esp, 32));
            cpu_state.data.esp = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.sp, 16));
        cpu_state.data.sp = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INC_EBP(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_EBP");
    i386(0x45) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.bp, 16));
            cpu_state.data.bp = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.ebp, 32));
            cpu_state.data.ebp = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.bp, 16));
        cpu_state.data.bp = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INC_ESI(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_ESI");
    i386(0x46) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.si, 16));
            cpu_state.data.si = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.esi, 32));
            cpu_state.data.esi = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.si, 16));
        cpu_state.data.si = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INC_EDI(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INC_EDI");
    i386(0x47) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.di, 16));
            cpu_state.data.di = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.edi, 32));
            cpu_state.data.edi = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.di, 16));
        cpu_state.data.di = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID DEC_EAX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_EAX");
    i386(0x48) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.ax, 16));
            cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.eax, 32));
            cpu_state.data.eax = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.ax, 16));
        cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID DEC_ECX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_ECX");
    i386(0x49) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.cx, 16));
            cpu_state.data.cx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.ecx, 32));
            cpu_state.data.ecx = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.cx, 16));
        cpu_state.data.cx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID DEC_EDX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_EDX");
    i386(0x4a) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.dx, 16));
            cpu_state.data.dx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.edx, 32));
            cpu_state.data.edx = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.dx, 16));
        cpu_state.data.dx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID DEC_EBX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_EBX");
    i386(0x4b) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.bx, 16));
            cpu_state.data.bx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.ebx, 32));
            cpu_state.data.ebx = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.bx, 16));
        cpu_state.data.bx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID DEC_ESP(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_ESP");
    i386(0x4c) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.sp, 16));
            cpu_state.data.sp = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.esp, 32));
            cpu_state.data.esp = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.sp, 16));
        cpu_state.data.sp = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID DEC_EBP(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_EBP");
    i386(0x4d) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.bp, 16));
            cpu_state.data.bp = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.ebp, 32));
            cpu_state.data.ebp = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.bp, 16));
        cpu_state.data.bp = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID DEC_ESI(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_ESI");
    i386(0x4e) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.si, 16));
            cpu_state.data.si = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.esi, 32));
            cpu_state.data.esi = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.si, 16));
        cpu_state.data.si = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID DEC_EDI(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("DEC_EDI");
    i386(0x4f) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.di, 16));
            cpu_state.data.di = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.edi, 32));
            cpu_state.data.edi = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.di, 16));
        cpu_state.data.di = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_EAX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_EAX");
    i386(0x50) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_ECX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_ECX");
    i386(0x51) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.cx), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ecx), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.cx), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_EDX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_EDX");
    i386(0x52) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.dx), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.edx), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.dx), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_EBX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_EBX");
    i386(0x53) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.bx), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ebx), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.bx), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_ESP(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_ESP");
    i386(0x54) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.sp), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.esp), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.sp), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_EBP(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_EBP");
    i386(0x55) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.bp), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ebp), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.bp), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_ESI(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_ESI");
    i386(0x56) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.si), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.esi), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.si), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_EDI(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_EDI");
    i386(0x57) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.di), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.edi), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.di), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID POP_EAX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_EAX");
    i386(0x58) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID POP_ECX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_ECX");
    i386(0x59) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.cx), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ecx), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.cx), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID POP_EDX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_EDX");
    i386(0x5a) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.dx), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.edx), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.dx), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID POP_EBX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_EBX");
    i386(0x5b) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.bx), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ebx), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.bx), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID POP_ESP(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_ESP");
    i386(0x5c) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.sp), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.esp), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.sp), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID POP_EBP(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_EBP");
    i386(0x5d) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.bp), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ebp), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.bp), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID POP_ESI(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_ESI");
    i386(0x5e) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.si), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.esi), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.si), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID POP_EDI(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_EDI");
    i386(0x5f) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.di), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.edi), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.di), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PUSHA(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_32 cesp;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSHA");
    i386(0x60) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            cesp = cpu_state.data.sp;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.cx), 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.dx), 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.bx), 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cesp),    2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.bp), 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.si), 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.di), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            cesp = cpu_state.data.esp;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ecx), 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.edx), 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ebx), 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cesp),     4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ebp), 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.esi), 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.edi), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID POPA(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_32 cesp;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POPA");
    i386(0x61) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.di), 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.si), 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.bp), 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cesp),    2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.bx), 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.dx), 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.cx), 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.edi), 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.esi), 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ebp), 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cesp),     4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ebx), 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.edx), 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ecx), 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID BOUND_R16_M16_16(core_machine_cpu_execution_context *context) {
    ntvdm64_type_signed_16 a16,l16,u16;
    ntvdm64_type_signed_32 a32,l32,u32;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BOUND_R16_M16_16");
    i386(0x62) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize * 2));
        if (!instruction_state.data.flagMem) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            a16 = (ntvdm64_type_signed_16)instruction_state.data.cr;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            l16 = (ntvdm64_type_signed_16)instruction_state.data.crm;
            instruction_state.data.mrm.offset += 2;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            u16 = (ntvdm64_type_signed_16)instruction_state.data.crm;
            if (a16 < l16 || a16 > u16)
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_BR(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            a32 = (ntvdm64_type_signed_32)instruction_state.data.cr;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 4));
            l32 = (ntvdm64_type_signed_32)instruction_state.data.crm;
            instruction_state.data.mrm.offset += 2;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 4));
            u32 = (ntvdm64_type_signed_32)instruction_state.data.crm;
            if (a32 < l32 || a32 > u32)
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_BR(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID ARPL_RM16_R16(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ARPL_RM16_R16");
    i386(0x63) {
        if (_IsProtected) {
            _adv;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            instruction_state.data.cr = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
            if (_GetSelector_RPL(instruction_state.data.crm) < _GetSelector_RPL(instruction_state.data.cr)) {
                _SetEFLAGS_ZF;
                instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16((instruction_state.data.crm & ~VCPU_SELECTOR_RPL) |
                                               (instruction_state.data.cr & VCPU_SELECTOR_RPL));
                instruction_state.data.crm = instruction_state.data.result;
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            } else
                _ClrEFLAGS_ZF;
        } else
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_FS(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_FS");
    i386(0x64) {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.fs;
        instruction_state.data.roverss = &cpu_state.data.fs;
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_GS(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_GS");
    i386(0x65) {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.gs;
        instruction_state.data.roverss = &cpu_state.data.gs;
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_OprSize(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_OprSize");
    i386(0x66) {
        _adv;
        instruction_state.data.prefix_oprsize = NTVDM64_TYPE_TRUE;
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_AddrSize(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_AddrSize");
    i386(0x67) {
        _adv;
        instruction_state.data.prefix_addrsize = NTVDM64_TYPE_TRUE;
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_I32");
    i386(0x68) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.cimm), _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID IMUL_R32_RM32_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IMUL_R32_RM32_I32");
    i386(0x69) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_imul3(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_I8");
    i386(0x6a) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.cimm), _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID IMUL_R32_RM32_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IMUL_R32_RM32_I8");
    i386(0x6b) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_imul3(context, instruction_state.data.crm, instruction_state.data.cimm, ((_GetOperandSize * 8 + 8) >> 1)));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INSB(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INSB");
    i386(0x6c) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_ins(context, 1));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_ins(context, 1));
                    cpu_state.data.cx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_ins(context, 1));
                    cpu_state.data.ecx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INSW(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INSW");
    i386(0x6d) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_ins(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_ins(context, _GetOperandSize));
                    cpu_state.data.cx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_ins(context, _GetOperandSize));
                    cpu_state.data.ecx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID OUTSB(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUTSB");
    i386(0x6e) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_ins(context, 1));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_ins(context, 1));
                    cpu_state.data.cx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_ins(context, 1));
                    cpu_state.data.ecx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID OUTSW(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUTSW");
    i386(0x6f) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_outs(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_outs(context, _GetOperandSize));
                    cpu_state.data.cx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_outs(context, _GetOperandSize));
                    cpu_state.data.ecx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JO_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JO_REL8");
    i386(0x70) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, _GetEFLAGS_OF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JNO_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNO_REL8");
    i386(0x71) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, !_GetEFLAGS_OF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JC_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JC_REL8");
    i386(0x72) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, _GetEFLAGS_CF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JNC_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNC_REL8");
    i386(0x73) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, !_GetEFLAGS_CF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JZ_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JZ_REL8");
    i386(0x74) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, _GetEFLAGS_ZF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JNZ_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNZ_REL8");
    i386(0x75) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, !_GetEFLAGS_ZF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JNA_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNA_REL8");
    i386(0x76) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1,
                (_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JA_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JA_REL8");
    i386(0x77) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1,
                !(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JS_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JS_REL8");
    i386(0x78) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, _GetEFLAGS_SF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JNS_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNS_REL8");
    i386(0x79) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, !_GetEFLAGS_SF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JP_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JP_REL8");
    _new_code_path_;
    i386(0x7a) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, _GetEFLAGS_PF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JNP_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNP_REL8");
    _new_code_path_;
    i386(0x7b) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, !_GetEFLAGS_PF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JL_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JL_REL8");
    i386(0x7c) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, (_GetEFLAGS_SF != _GetEFLAGS_OF)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JNL_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNL_REL8");
    i386(0x7d) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, (_GetEFLAGS_SF == _GetEFLAGS_OF)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JNG_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNG_REL8");
    i386(0x7e) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1,
                (_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF))));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JG_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JG_REL8");
    i386(0x7f) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1,
                (!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF))));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_80(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_80");
    i386 (0x80) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    switch (instruction_state.data.cr) {
    case 0: /* ADD_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ADD_RM8_I8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* OR_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OR_RM8_I8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* ADC_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ADC_RM8_I8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* SBB_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SBB_RM8_I8");
        _new_code_path_;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* AND_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AND_RM8_I8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SUB_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SUB_RM8_I8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* XOR_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("XOR_RM8_I8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* CMP_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CMP_RM8_I8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_81(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_81");
    i386(0x81) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        switch (instruction_state.data.cr) {
        case 0: /* ADD_RM32_I32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ADD_RM32_I32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* OR_RM32_I32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OR_RM32_I32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* ADC_RM32_I32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ADC_RM32_I32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* SBB_RM32_I32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SBB_RM32_I32");
            _new_code_path_;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* AND_RM32_I32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AND_RM32_I32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SUB_RM32_I32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SUB_RM32_I32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* XOR_RM32_I32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("XOR_RM32_I32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* CMP_RM32_I32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CMP_RM32_I32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        switch (instruction_state.data.cr) {
        case 0: /* ADD_RM16_I16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ADD_RM16_I16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* OR_RM16_I16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OR_RM16_I16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* ADC_RM16_I16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ADC_RM16_I16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* SBB_RM16_I16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SBB_RM16_I16");
            _new_code_path_;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* AND_RM16_I16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AND_RM16_I16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SUB_RM16_I16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SUB_RM16_I16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* XOR_RM16_I16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("XOR_RM16_I16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* CMP_RM16_I16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CMP_RM16_I16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_83(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_8 bit;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_83");
    i386(0x83) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        bit = (_GetOperandSize * 8 + 8) >> 1;
        switch (instruction_state.data.cr) {
        case 0: /* ADD_RM32_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ADD_RM32_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* OR_RM32_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OR_RM32_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* ADC_RM32_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ADC_RM32_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* SBB_RM32_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SBB_RM32_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* AND_RM32_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AND_RM32_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SUB_RM32_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SUB_RM32_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* XOR_RM32_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("XOR_RM32_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* CMP_RM32_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CMP_RM32_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        bit = 12;
        switch (instruction_state.data.cr) {
        case 0: /* ADD_RM16_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ADD_RM16_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* OR_RM16_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OR_RM16_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* ADC_RM16_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ADC_RM16_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* SBB_RM16_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SBB_RM16_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* AND_RM16_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AND_RM16_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SUB_RM16_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SUB_RM16_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* XOR_RM16_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("XOR_RM16_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* CMP_RM16_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CMP_RM16_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID TEST_RM8_R8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("TEST_RM8_R8");
    i386(0x84) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_test(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID TEST_RM32_R32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("TEST_RM32_R32");
    i386(0x85) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_test(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
    }
    else {
        cpu_state.data.ip++;
        _d_modrm(context, 2, 2);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        _a_test(context, instruction_state.data.crm, instruction_state.data.cr, 16);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_RM8_R8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_RM8_R8");
    i386(0x86) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), 1));
    instruction_state.data.crm = instruction_state.data.cr;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_RM32_R32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_RM32_R32");
    i386(0x87) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
        instruction_state.data.crm = instruction_state.data.cr;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _d_modrm(context, 2, 2);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), 2));
        instruction_state.data.crm = instruction_state.data.cr;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_RM8_R8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_RM8_R8");
    i386(0x88) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    instruction_state.data.crm = instruction_state.data.cr;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_RM32_R32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_RM32_R32");
    i386(0x89) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        instruction_state.data.crm = instruction_state.data.cr;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _d_modrm(context, 2, 2);
        instruction_state.data.crm = instruction_state.data.cr;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_R8_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_R8_RM8");
    i386(0x8a) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_R32_RM32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_R32_RM32");
    i386(0x8b) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _d_modrm(context, 2, 2);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_RM16_SREG(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_RM16_SREG");
    i386(0x8c) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm_sreg(context, 2));
    instruction_state.data.crm = instruction_state.data.rmovsreg->selector;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID LEA_R32_M32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LEA_R32_M32");
    i386(0x8d) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm_ea(context, _GetOperandSize, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.mrm.offset), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.mrm.offset), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _d_modrm_ea(context, 2, 2);
        _m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.mrm.offset), 2);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_SREG_RM16(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_SREG_RM16");
    i386(0x8e) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm_sreg(context, 2));
        if (instruction_state.data.rmovsreg->sregtype == SREG_CODE) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_CODE)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, instruction_state.data.rmovsreg, NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm)));
        if (instruction_state.data.rmovsreg->sregtype == SREG_STACK)
            instruction_state.data.flagMaskInt = NTVDM64_TYPE_TRUE;
    }
    else {
        cpu_state.data.ip++;
        _d_modrm_sreg(context, 2);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, instruction_state.data.rmovsreg, NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm)));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_8F(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_8F");
    i386(0x8f) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        switch (instruction_state.data.cr) {
        case 0: /* POP_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("POP_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        switch (instruction_state.data.cr) {
        case 0: /* POP_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("POP_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID NOP(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("NOP");
    i386(0x90) {
        _adv;
    }
    else
        cpu_state.data.ip++;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_ECX_EAX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_ECX_EAX");
    i386(0x91) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.cx;
            cpu_state.data.cx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.ecx;
            cpu_state.data.ecx = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cr);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.cx;
        cpu_state.data.cx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_EDX_EAX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_EDX_EAX");
    i386(0x92) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.dx;
            cpu_state.data.dx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.edx;
            cpu_state.data.edx = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cr);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.dx;
        cpu_state.data.dx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_EBX_EAX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_EBX_EAX");
    i386(0x93) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.bx;
            cpu_state.data.bx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.ebx;
            cpu_state.data.ebx = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cr);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.bx;
        cpu_state.data.bx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_ESP_EAX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_ESP_EAX");
    _new_code_path_;
    i386(0x94) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.sp;
            cpu_state.data.sp = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.esp;
            cpu_state.data.esp = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cr);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.sp;
        cpu_state.data.sp = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_EBP_EAX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_EBP_EAX");
    i386(0x95) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.bp;
            cpu_state.data.bp = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.ebp;
            cpu_state.data.ebp = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cr);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.bp;
        cpu_state.data.bp = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_ESI_EAX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_ESI_EAX");
    i386(0x96) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.si;
            cpu_state.data.si = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.esi;
            cpu_state.data.esi = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cr);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.si;
        cpu_state.data.si = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_EDI_EAX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XCHG_EDI_EAX");
    i386(0x97) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.di;
            cpu_state.data.di = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.edi;
            cpu_state.data.edi = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cr);
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.di;
        cpu_state.data.di = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID CBW(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CBW");
    i386(0x98) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.ax = (ntvdm64_type_signed_8)cpu_state.data.al;
            break;
        case 4:
            cpu_state.data.eax = (ntvdm64_type_signed_16)cpu_state.data.ax;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        cpu_state.data.ax = (ntvdm64_type_signed_8)cpu_state.data.al;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID CWD(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CWD");
    i386(0x99) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.dx = NTVDM64_TYPE_GET_MSB_16(cpu_state.data.ax) ? 0xffff : 0x0000;
            break;
        case 4:
            cpu_state.data.edx = NTVDM64_TYPE_GET_MSB_32(cpu_state.data.eax) ? 0xffffffff : 0x00000000;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        if (cpu_state.data.ax & 0x8000) cpu_state.data.dx = 0xffff;
        else cpu_state.data.dx = 0x0000;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID CALL_PTR16_32(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_16 newcs;
    ntvdm64_type_unsigned_32 neweip;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CALL_PTR16_32");
    i386(0x9a) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            neweip = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
            newcs = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm >> 16);
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_call_far(context, newcs, neweip, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 8));
            neweip = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm);
            newcs = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm >> 32);
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_call_far(context, newcs, neweip, 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _d_imm(context, 4);
        neweip = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
        newcs = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm >> 16);
        _e_call_far(context, newcs, neweip, 2);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo WAIT(core_machine_cpu_execution_context *context) {
    /* not implemented */
    NTVDM64_TYPE_TRACE_CALL_BEGIN("WAIT");
    _new_code_path_;
    i386(0x9b) {
        _adv;
        if (_GetCR0_TS) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CR0_TS(1)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_NM(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PUSHF(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_32 ceflags;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSHF");
    i386(0x9c) {
        _adv;
        if (!_GetCR0_PE || (_GetCR0_PE && (!_GetEFLAGS_VM || (_GetEFLAGS_VM && (_GetEFLAGS_IOPL == 3))))) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Real/Protected/(V86,IOPL(3))");
            switch (_GetOperandSize) {
            case 2:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
                ceflags = cpu_state.data.flags;
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(ceflags), 2));
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
                ceflags = cpu_state.data.eflags & ~(VCPU_EFLAGS_VM | VCPU_EFLAGS_RF);
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(ceflags), 4));
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("EFLAGS_VM(1),EFLAGS_IOPL(!3)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
    }
    else {
        cpu_state.data.ip++;
        _e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.flags), 2);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID POPF(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_32 mask = VCPU_EFLAGS_RESERVED;
    ntvdm64_type_unsigned_32 ceflags = NTVDM64_TYPE_ZERO_32;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POPF");
    i386(0x9d) {
        _adv;
        if (!_GetCR0_PE || !_GetEFLAGS_VM) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!V86");
            if (!_GetCPL) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CPL(0)");
                switch (_GetOperandSize) {
                case 2:
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(ceflags), 2));
                    mask |= 0xffff0000;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                    break;
                case 4:
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(ceflags), 4));
                    mask |= VCPU_EFLAGS_VM;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                    break;
                default:
                    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
                _new_code_path_;
                switch (_GetOperandSize) {
                case 2:
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(ceflags), 2));
                    mask |= (0xffff0000 | VCPU_EFLAGS_IOPL);
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                    break;
                case 4:
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(ceflags), 4));
                    mask |= (VCPU_EFLAGS_VM | VCPU_EFLAGS_RF | VCPU_EFLAGS_IOPL);
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                    break;
                default:
                    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("V86");
            if (instruction_state.data.prefix_oprsize) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_oprsize(1)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            if (_GetEFLAGS_IOPL == 0x03) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("EFLAGS_IOPL(3)");
                switch (_GetOperandSize) {
                case 2:
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(ceflags), 2));
                    mask |= (0xffff0000 | VCPU_EFLAGS_IOPL);
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                    break;
                case 4:
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(ceflags), 4));
                    mask |= (VCPU_EFLAGS_VM | VCPU_EFLAGS_RF | VCPU_EFLAGS_IOPL);
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                    break;
                default:
                    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("EFLAGS_IOPL(!3)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        cpu_state.data.eflags = (ceflags & ~mask) | (cpu_state.data.eflags & mask);
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.flags), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SAHF(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_32 mask = (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                      VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_CF);
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SAHF");
    i386(0x9e) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    cpu_state.data.eflags = (cpu_state.data.ah & mask) | (cpu_state.data.eflags & ~mask);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID LAHF(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LAHF");
    i386(0x9f) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    cpu_state.data.ah = NTVDM64_TYPE_MASK_UNSIGNED_8(cpu_state.data.flags);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_AL_MOFFS8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_AL_MOFFS8");
    i386(0xa0) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_moffs(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    cpu_state.data.al = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.crm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_EAX_MOFFS32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_EAX_MOFFS32");
    i386(0xa1) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_moffs(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
            break;
        case 4:
            cpu_state.data.eax = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_moffs(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_MOFFS8_AL(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_MOFFS8_AL");
    i386(0xa2) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_moffs(context, 1));
    instruction_state.data.result = cpu_state.data.al;
    instruction_state.data.crm = instruction_state.data.result;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_MOFFS32_EAX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_MOFFS32_EAX");
    i386(0xa3) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_moffs(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            instruction_state.data.result = cpu_state.data.ax;
            break;
        case 4:
            instruction_state.data.result = cpu_state.data.eax;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_moffs(context, 2));
        instruction_state.data.crm = cpu_state.data.ax;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOVSB(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVSB");
    i386(0xa4) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_movs(context, 1));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_movs(context, 1));
                    cpu_state.data.cx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_movs(context, 1));
                    cpu_state.data.ecx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
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
            if (cpu_state.data.cx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
        }
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOVSW(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVSW");
    i386(0xa5) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_movs(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_movs(context, _GetOperandSize));
                    cpu_state.data.cx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_movs(context, _GetOperandSize));
                    cpu_state.data.ecx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
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
            if (cpu_state.data.cx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
        }
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID CMPSB(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMPSB");
    i386(0xa6) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmps(context, 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmps(context, 8));
                    cpu_state.data.cx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmps(context, 8));
                    cpu_state.data.ecx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
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
                instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
        }
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID CMPSW(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMPSW");
    i386(0xa7) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmps(context, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmps(context, _GetOperandSize * 8));
                    cpu_state.data.cx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_cmps(context, _GetOperandSize * 8));
                    cpu_state.data.ecx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
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
                instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
        }
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID TEST_AL_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("TEST_AL_I8");
    i386(0xa8) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_test(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID TEST_EAX_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("TEST_EAX_I32");
    i386(0xa9) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_test(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_test(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_test(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID STOSB(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("STOSB");
    i386(0xaa) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_stos(context, 1));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_stos(context, 1));
                    cpu_state.data.cx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_stos(context, 1));
                    cpu_state.data.ecx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
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
            if (cpu_state.data.cx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
        }
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID STOSW(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("STOSW");
    i386(0xab) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_stos(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_stos(context, _GetOperandSize));
                    cpu_state.data.cx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_stos(context, _GetOperandSize));
                    cpu_state.data.ecx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
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
            if (cpu_state.data.cx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
        }
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID LODSB(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LODSB");
    i386(0xac) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_lods(context, 1));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_lods(context, 1));
                    cpu_state.data.cx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_lods(context, 1));
                    cpu_state.data.ecx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
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
            if (cpu_state.data.cx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
        }
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID LODSW(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LODSW");
    i386(0xad) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_lods(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_lods(context, _GetOperandSize));
                    cpu_state.data.cx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_lods(context, _GetOperandSize));
                    cpu_state.data.ecx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
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
            if (cpu_state.data.cx) instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
        }
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SCASB(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SCASB");
    i386(0xae) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_scas(context, 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_scas(context, 8));
                    cpu_state.data.cx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_scas(context, 8));
                    cpu_state.data.ecx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
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
                instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
        }
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SCASW(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SCASW");
    i386(0xaf) {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_scas(context, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize) {
            case 2:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_scas(context, _GetOperandSize * 8));
                    cpu_state.data.cx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx) {
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_scas(context, _GetOperandSize * 8));
                    cpu_state.data.ecx--;
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                        !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
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
                instruction_state.data.flagInsLoop = NTVDM64_TYPE_TRUE;
        }
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_AL_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_AL_I8");
    i386(0xb0) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    cpu_state.data.al = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_CL_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_CL_I8");
    i386(0xb1) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    cpu_state.data.cl = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_DL_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_DL_I8");
    i386(0xb2) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    cpu_state.data.dl = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_BL_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_BL_I8");
    i386(0xb3) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    cpu_state.data.bl = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_AH_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_AH_I8");
    i386(0xb4) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    cpu_state.data.ah = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_CH_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_CH_I8");
    i386(0xb5) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    cpu_state.data.ch = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_DH_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_DH_I8");
    i386(0xb6) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    cpu_state.data.dh = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_BH_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_BH_I8");
    i386(0xb7) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    cpu_state.data.bh = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_EAX_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_EAX_I32");
    i386(0xb8) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.eax = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        cpu_state.data.ax = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_ECX_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_ECX_I32");
    i386(0xb9) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.cx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.ecx = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        cpu_state.data.cx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_EDX_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_EDX_I32");
    i386(0xba) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.dx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.edx = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        cpu_state.data.dx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_EBX_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_EBX_I32");
    i386(0xbb) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.bx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.ebx = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        cpu_state.data.bx = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_ESP_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_ESP_I32");
    i386(0xbc) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.sp = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.esp = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        cpu_state.data.sp = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_EBP_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_EBP_I32");
    i386(0xbd) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.bp = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.ebp = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        cpu_state.data.bp = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_ESI_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_ESI_I32");
    i386(0xbe) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.si = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.esi = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        cpu_state.data.si = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_EDI_I32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_EDI_I32");
    i386(0xbf) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            cpu_state.data.di = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.edi = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        cpu_state.data.di = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_C0(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_C0");
    _new_code_path_;
    i386(0xc0) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        switch (instruction_state.data.cr) {
        case 0: /* ROL_RM8_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROL_RM8_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rol(context, NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.crm), NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* ROR_RM8_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROR_RM8_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_ror(context, NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.crm), NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* RCL_RM8_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCL_RM8_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rcl(context, NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.crm), NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* RCR_RM8_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCR_RM8_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rcr(context, NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.crm), NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* SHL_RM8_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHL_RM8_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shl(context, NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.crm), NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SHR_RM8_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHR_RM8_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shr(context, NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.crm), NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* UndefinedOpcode */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* SAR_RM8_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SAR_RM8_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sar(context, NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.crm), NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_C1(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_C1");
    i386(0xc1) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        switch (instruction_state.data.cr) {
        case 0: /* ROL_RM32_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROL_RM32_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rol(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.crm), NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* ROR_RM32_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROR_RM32_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_ror(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.crm), NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* RCL_RM32_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCL_RM32_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rcl(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.crm), NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* RCR_RM32_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCR_RM32_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rcr(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.crm), NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* SHL_RM32_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHL_RM32_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shl(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.crm), NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SHR_RM32_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHR_RM32_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shr(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.crm), NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* UndefinedOpcode */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* SAR_RM32_I8 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SAR_RM32_I8");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sar(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.crm), NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID RET_I16(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("RET_I16");
    i386(0xc2) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_ret_near(context, NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _d_imm(context, 2);
        _e_ret_near(context, NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm), 2);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID RET(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("RET");
    i386(0xc3) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_ret_near(context, 0, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_ret_near(context, 0, 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID LES_R32_M16_32(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_16 selector;
    ntvdm64_type_unsigned_32 offset;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LES_R32_M16_32");
    i386(0xc4) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize + 2));
        if (!instruction_state.data.flagMem) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            offset = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
            break;
        case 4:
            offset = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.mrm.offset += _GetOperandSize;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        selector = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.es, instruction_state.data.rr, selector, offset, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _d_modrm(context, 2, 4);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        offset = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        instruction_state.data.mrm.offset += 2;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        selector = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.es, instruction_state.data.rr, selector, offset, 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID LDS_R32_M16_32(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_16 selector;
    ntvdm64_type_unsigned_32 offset;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LDS_R32_M16_32");
    i386(0xc5) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize + 2));
        if (!instruction_state.data.flagMem) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        switch (_GetOperandSize) {
        case 2:
            offset = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
            break;
        case 4:
            offset = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.mrm.offset += _GetOperandSize;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        selector = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.ds, instruction_state.data.rr, selector, offset, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _d_modrm(context, 2, 4);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        offset = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        instruction_state.data.mrm.offset += 2;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        selector = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.ds, instruction_state.data.rr, selector, offset, 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_C6(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_C6");
    i386(0xc6) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    switch (instruction_state.data.cr) {
    case 0: /* MOV_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("MOV_RM8_I8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        instruction_state.data.crm = instruction_state.data.cimm;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_C7(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_C7");
    i386(0xc7) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        switch (instruction_state.data.cr) {
        case 0: /* MOV_RM32_I32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("MOV_RM32_I32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
            instruction_state.data.crm = instruction_state.data.cimm;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        switch (instruction_state.data.cr) {
        case 0: /* MOV_RM16_I16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("MOV_RM16_I16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            instruction_state.data.crm = instruction_state.data.cimm;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID ENTER(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_32 data = 0;
    ntvdm64_type_unsigned_32 temp = 0;
    ntvdm64_type_native_unsigned i = 0;
    ntvdm64_type_unsigned_16 size = NTVDM64_TYPE_ZERO_16;
    ntvdm64_type_unsigned_8 level = NTVDM64_TYPE_ZERO_8;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("ENTER");
    i386(0xc8) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        size = (ntvdm64_type_unsigned_16) instruction_state.data.cimm;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        level = (ntvdm64_type_unsigned_8) instruction_state.data.cimm;
        level %= 32;
        switch (_GetStackSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("StackSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.bp), 2));
            temp = cpu_state.data.sp;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("StackSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ebp), 4));
            temp = cpu_state.data.esp;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        if (level) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("level(!0)");
            for (i = 0; i < level; ++i) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("for");
                switch (_GetOperandSize) {
                case 2:
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
                    switch (_GetStackSize) {
                    case 2:
                        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("StackSize(2)");
                        cpu_state.data.bp -= 2;
                        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_ss(context, cpu_state.data.bp, NTVDM64_TYPE_REFERENCE_OF(data), 2));
                        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(data), 2));
                        NTVDM64_TYPE_TRACE_BLOCK_END;
                        break;
                    case 4:
                        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("StackSize(4)");
                        cpu_state.data.ebp -= 2;
                        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_ss(context, cpu_state.data.ebp, NTVDM64_TYPE_REFERENCE_OF(data), 2));
                        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(data), 2));
                        NTVDM64_TYPE_TRACE_BLOCK_END;
                        break;
                    default:
                        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                        break;
                    }
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                    break;
                case 4:
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
                    switch (_GetStackSize) {
                    case 2:
                        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("StackSize(2)");
                        cpu_state.data.bp -= 4;
                        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_ss(context, cpu_state.data.bp, NTVDM64_TYPE_REFERENCE_OF(data), 4));
                        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(data), 4));
                        NTVDM64_TYPE_TRACE_BLOCK_END;
                        break;
                    case 4:
                        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("StackSize(4)");
                        cpu_state.data.ebp -= 4;
                        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_ss(context, cpu_state.data.ebp, NTVDM64_TYPE_REFERENCE_OF(data), 4));
                        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(data), 4));
                        NTVDM64_TYPE_TRACE_BLOCK_END;
                        break;
                    default:
                        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                        break;
                    }
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                    break;
                default:
                    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            switch (_GetOperandSize) {
            case 2:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(temp), 2));
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(temp), 4));
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            default:
                NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        switch (_GetStackSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("StackSize(2)");
            cpu_state.data.bp = NTVDM64_TYPE_MASK_UNSIGNED_16(temp);
            cpu_state.data.sp = cpu_state.data.bp - size;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("StackSize(4)");
            cpu_state.data.ebp = NTVDM64_TYPE_MASK_UNSIGNED_32(temp);
            cpu_state.data.esp = cpu_state.data.ebp - size;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID LEAVE(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LEAVE");
    i386(0xc9) {
        _adv;
        if (!_IsProtected && cpu_state.data.ebp > 0x0000ffff) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Protected(0),ebp(>0000ffff)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_test_logical(context, &cpu_state.data.ss, cpu_state.data.ebp, _GetOperandSize, 1));
        switch (_GetStackSize) {
        case 2:
            cpu_state.data.sp = cpu_state.data.bp;
            break;
        case 4:
            cpu_state.data.esp = cpu_state.data.ebp;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.bp), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ebp), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID RETF_I16(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("RETF_I16");
    i386(0xca) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_ret_far(context, NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm), _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _d_imm(context, 2);
        _e_ret_far(context, NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm), 2);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID RETF(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("RETF");
    i386(0xcb) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_ret_far(context, 0, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_ret_far(context, 0, 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INT3(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INT3");
    _new_code_path_;
    i386(0xcc) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_int3(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _e_int_n(context, 0x03, _GetOperandSize);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INT_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INT_I8");
    i386(0xcc) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_int_n(context, (ntvdm64_type_unsigned_8)instruction_state.data.cimm, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        _d_imm(context, 1);
        _e_int_n(context, (ntvdm64_type_unsigned_8)instruction_state.data.cimm, 2);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INTO(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INTO");
    _new_code_path_;
    i386(0xce) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_into(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        if (_GetEFLAGS_OF) _e_int_n(context, 0x04, _GetOperandSize);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID IRET(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IRET");
    i386(0xcf) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_iret(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_iret(context, 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_D0(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_D0");
    i386(0xd0) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    switch (instruction_state.data.cr) {
    case 0: /* ROL_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROL_RM8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rol(context, instruction_state.data.crm, 1, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROR_RM8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_ror(context, instruction_state.data.crm, 1, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCL_RM8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rcl(context, instruction_state.data.crm, 1, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCR_RM8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rcr(context, instruction_state.data.crm, 1, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHL_RM8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shl(context, instruction_state.data.crm, 1, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHR_RM8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shr(context, instruction_state.data.crm, 1, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SAR_RM8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sar(context, instruction_state.data.crm, 1, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    instruction_state.data.crm = instruction_state.data.result;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_D1(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_D1");
    i386(0xd1) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        switch (instruction_state.data.cr) {
        case 0: /* ROL_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROL_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rol(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* ROR_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROR_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_ror(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* RCL_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCL_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rcl(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* RCR_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCR_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rcr(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* SHL_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHL_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shl(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SHR_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHR_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shr(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* UndefinedOpcode */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* SAR_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SAR_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sar(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        switch (instruction_state.data.cr) {
        case 0: /* ROL_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROL_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rol(context, instruction_state.data.crm, 1, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* ROR_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROR_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_ror(context, instruction_state.data.crm, 1, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* RCL_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCL_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rcl(context, instruction_state.data.crm, 1, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* RCR_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCR_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rcr(context, instruction_state.data.crm, 1, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* SHL_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHL_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shl(context, instruction_state.data.crm, 1, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SHR_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHR_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shr(context, instruction_state.data.crm, 1, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* UndefinedOpcode */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* SAR_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SAR_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sar(context, instruction_state.data.crm, 1, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_D2(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_D2");
    i386(0xd2) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    switch (instruction_state.data.cr) {
    case 0: /* ROL_RM8_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROL_RM8_CL");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rol(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM8_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROR_RM8_CL");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_ror(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM8_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCL_RM8_CL");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rcl(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM8_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCR_RM8_CL");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rcr(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM8_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHL_RM8_CL");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shl(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM8_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHR_RM8_CL");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shr(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM8_CL */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SAR_RM8_CL");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sar(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    instruction_state.data.crm = instruction_state.data.result;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_D3(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_D3");
    i386(0xd3) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        switch (instruction_state.data.cr) {
        case 0: /* ROL_RM32_CL */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROL_RM32_CL");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rol(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* ROR_RM32_CL */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROR_RM32_CL");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_ror(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* RCL_RM32_CL */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCL_RM32_CL");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rcl(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* RCR_RM32_CL */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCR_RM32_CL");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rcr(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* SHL_RM32_CL */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHL_RM32_CL");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shl(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SHR_RM32_CL */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHR_RM32_CL");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shr(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* UndefinedOpcode */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* SAR_RM32_CL */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SAR_RM32_CL");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sar(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        switch (instruction_state.data.cr) {
        case 0: /* ROL_RM16_CL */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROL_RM16_CL");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rol(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* ROR_RM16_CL */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ROR_RM16_CL");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_ror(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* RCL_RM16_CL */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCL_RM16_CL");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rcl(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* RCR_RM16_CL */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("RCR_RM16_CL");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_rcr(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* SHL_RM16_CL */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHL_RM16_CL");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shl(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SHR_RM16_CL */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SHR_RM16_CL");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shr(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* UndefinedOpcode */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* SAR_RM16_CL */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SAR_RM16_CL");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_sar(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID AAM(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_8 base;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AAM");
    i386(0xd4) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    }
    else {
        cpu_state.data.ip++;
        _d_imm(context, 1);
    }
    base = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    cpu_state.data.ah = cpu_state.data.al / base;
    cpu_state.data.al = cpu_state.data.al % base;
    instruction_state.data.bit = 0x08;
    instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_8(cpu_state.data.al);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, AAM_FLAG));
    instruction_state.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID AAD(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_8 base;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("AAD");
    i386(0xd5) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    }
    else {
        cpu_state.data.ip++;
        _d_imm(context, 1);
    }
    base = NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    cpu_state.data.al = NTVDM64_TYPE_MASK_UNSIGNED_8(cpu_state.data.al + (cpu_state.data.ah * base));
    cpu_state.data.ah = 0x00;
    instruction_state.data.bit = 0x08;
    instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_8(cpu_state.data.al);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, AAD_FLAG));
    instruction_state.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID XLAT(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("XLAT");
    i386(0xd7) {
        _adv;
        switch (_GetAddressSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, (cpu_state.data.bx + cpu_state.data.al), NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.al), 1));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, (cpu_state.data.ebx + cpu_state.data.al), NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.al), 1));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, (cpu_state.data.bx + cpu_state.data.al), NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.al), 1));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID LOOPNZ_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LOOPNZ_REL8");
    i386(0xe0) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_loopcc(context, (ntvdm64_type_signed_8)instruction_state.data.cimm, !_GetEFLAGS_ZF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID LOOPZ_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LOOPZ_REL8");
    i386(0xe1) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_loopcc(context, (ntvdm64_type_signed_8)instruction_state.data.cimm, _GetEFLAGS_ZF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID LOOP_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LOOP_REL8");
    i386(0xe2) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_loopcc(context, (ntvdm64_type_signed_8)instruction_state.data.cimm, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JCXZ_REL8(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_32 cecx = 0x00000000;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JCXZ_REL8");
    i386(0xe3) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        switch (_GetAddressSize) {
        case 2:
            cecx = cpu_state.data.cx;
            break;
        case 4:
            cecx = cpu_state.data.ecx;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, !cecx));
    }
    else {
        cpu_state.data.ip++;
        _d_imm(context, 1);
        _e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, !cpu_state.data.cx);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID IN_AL_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IN_AL_I8");
    i386(0xe4) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_input(context, NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.al), 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID IN_EAX_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IN_EAX_I8");
    _new_code_path_;
    i386(0xe5) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_input(context, NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_input(context, NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_input(context, NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID OUT_I8_AL(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUT_I8_AL");
    i386(0xe6) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_output(context, NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.al), 1));
    NTVDM64_TYPE_TRACE_CALL_END;

}
static C_VOID OUT_I8_EAX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUT_I8_EAX");
    _new_code_path_;
    i386(0xe7) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_output(context, NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_output(context, NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _d_imm(context, 1);
        _p_output(context, NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ax), 2);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID CALL_REL32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CALL_REL32");
    i386(0xe8) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_call_near(context, NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.ip + (ntvdm64_type_signed_16)instruction_state.data.cimm), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_call_near(context, NTVDM64_TYPE_MASK_UNSIGNED_32(cpu_state.data.eip + (ntvdm64_type_signed_32)instruction_state.data.cimm), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(_GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_call_near(context, NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.ip + (ntvdm64_type_signed_16)instruction_state.data.cimm), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JMP_REL32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JMP_REL32");
    i386(0xe9) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, 1));
    }
    else {
        cpu_state.data.ip++;
        _d_imm(context, 2);
        _e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 2, 1);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JMP_PTR16_32(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_16 newcs = 0x0000;
    ntvdm64_type_unsigned_32 neweip = 0x00000000;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JMP_PTR16_32");
    i386(0xea) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            neweip = (ntvdm64_type_unsigned_16)instruction_state.data.cimm;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            newcs = (ntvdm64_type_unsigned_16)instruction_state.data.cimm;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            neweip = (ntvdm64_type_unsigned_32)instruction_state.data.cimm;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            newcs = (ntvdm64_type_unsigned_16)instruction_state.data.cimm;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jmp_far(context, newcs, neweip, _GetOperandSize));
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        neweip = (ntvdm64_type_unsigned_16)instruction_state.data.cimm;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        newcs = (ntvdm64_type_unsigned_16)instruction_state.data.cimm;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jmp_far(context, newcs, neweip, 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JMP_REL8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JMP_REL8");
    i386(0xeb) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID IN_AL_DX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IN_AL_DX");
    i386(0xec) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.al), 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID IN_EAX_DX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IN_EAX_DX");
    i386(0xed) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID OUT_DX_AL(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUT_DX_AL");
    i386(0xee) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.al), 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID OUT_DX_EAX(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("OUT_DX_EAX");
    _new_code_path_;
    i386(0xef) {
        _adv;
        switch (_GetOperandSize) {
        case 2:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        _p_output(context, cpu_state.data.dx, NTVDM64_TYPE_REFERENCE_OF(cpu_state.data.ax), 2);
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_LOCK(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_8 opcode = 0x00;
    ntvdm64_type_unsigned_8 modrm = 0x00;
    ntvdm64_type_unsigned_8 opcode_0f = 0x00;
    ntvdm64_type_unsigned_32 ceip = cpu_state.data.eip;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_LOCK");
    i386(0xf0) {
        _adv;
        do {
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, ceip, NTVDM64_TYPE_REFERENCE_OF(opcode), 1));
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
            instruction_state.data.flagLock = NTVDM64_TYPE_TRUE;
            break;
        case 0x80:
        case 0x81:
        case 0x83:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opcode(0x80/0x81/0x83)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, ceip, NTVDM64_TYPE_REFERENCE_OF(modrm), 1));
            if (_GetModRM_REG(modrm) != 7)
                instruction_state.data.flagLock = NTVDM64_TYPE_TRUE;
            else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 0x0f:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opcode(0f)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, ceip, NTVDM64_TYPE_REFERENCE_OF(opcode_0f), 1));
            switch (opcode_0f) {
            case 0xa3: /* BT */
            case 0xab: /* BTS */
            case 0xb3: /* BTR */
            case 0xbb: /* BTC */
            case 0xba:
                instruction_state.data.flagLock = NTVDM64_TYPE_TRUE;
                break;
            default:
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opcode_0f");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
                break;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("opcode");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        }
    }
    else _adv;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_REPNZ(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_REPNZ");
    i386(0xf2) {
        _adv;
        instruction_state.data.prefix_rep = PREFIX_REP_REPZNZ;
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.prefix_rep = PREFIX_REP_REPZNZ;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_REPZ(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PREFIX_REPZ");
    i386(0xf3) {
        _adv;
        instruction_state.data.prefix_rep = PREFIX_REP_REPZ;
    }
    else {
        cpu_state.data.ip++;
        instruction_state.data.prefix_rep = PREFIX_REP_REPZ;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID HLT(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("HLT");
    _new_code_path_;
    if (_GetCR0_PE && _GetCPL) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CR0_PE(1),CPL(!0)");
        _SetExcept_GP(0);
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    _adv;
    cpu_state.data.flagHalt = NTVDM64_TYPE_TRUE;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID CMC(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CMC");
    i386(0xf5) {
        _adv;
        cpu_state.data.eflags ^= VCPU_EFLAGS_CF;
    }
    else {
        cpu_state.data.ip++;
        cpu_state.data.eflags ^= VCPU_EFLAGS_CF;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_F6(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_F6");
    i386(0xf6) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    switch (instruction_state.data.cr) {
    case 0: /* TEST_RM8_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("TEST_RM8_I8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_test(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* UndefinedOpcode */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* NOT_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("NOT_RM8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_not(context, instruction_state.data.crm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* NEG_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("NEG_RM8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_neg(context, instruction_state.data.crm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* MUL_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("MUL_RM8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_mul(context, instruction_state.data.crm, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* IMUL_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("IMUL_RM8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_imul(context, instruction_state.data.crm, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* DIV_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DIV_RM8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_div(context, instruction_state.data.crm, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* IDIV_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("IDIV_RM8");
        _new_code_path_;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_idiv(context, instruction_state.data.crm, 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_F7(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_F7");
    i386(0xf7) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        switch (instruction_state.data.cr) {
        case 0: /* TEST_RM32_I32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("TEST_RM32_I32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_test(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* UndefinedOpcode */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(1)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* NOT_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("NOT_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_not(context, instruction_state.data.crm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* NEG_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("NEG_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_neg(context, instruction_state.data.crm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* MUL_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("MUL_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_mul(context, instruction_state.data.crm, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* IMUL_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("IMUL_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_imul(context, instruction_state.data.crm, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* DIV_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DIV_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_div(context, instruction_state.data.crm, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* IDIV_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("IDIV_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_idiv(context, instruction_state.data.crm, _GetOperandSize * 8));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        switch (instruction_state.data.cr) {
        case 0: /* TEST_RM16_I16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("TEST_RM16_I16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_test(context, instruction_state.data.crm, instruction_state.data.cimm, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* UndefinedOpcode */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(1)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* NOT_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("NOT_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_not(context, instruction_state.data.crm, 16));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* NEG_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("NEG_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_neg(context, instruction_state.data.crm, 16));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* MUL_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("MUL_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_mul(context, instruction_state.data.crm, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* IMUL_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("IMUL_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_imul(context, instruction_state.data.crm, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* DIV_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DIV_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_div(context, instruction_state.data.crm, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* IDIV_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("IDIV_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_idiv(context, instruction_state.data.crm, 16));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID CLC(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CLC");
    i386(0xf8) {
        _adv;
        _ClrEFLAGS_CF;
    }
    else {
        cpu_state.data.ip++;
        _ClrEFLAGS_CF;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID STC(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("STC");
    i386(0xf9) {
        _adv;
        _SetEFLAGS_CF;
    }
    else {
        cpu_state.data.ip++;
        _SetEFLAGS_CF;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID CLI(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CLI");
    i386(0xfa) {
        _adv;
        if (!_GetCR0_PE)
            _ClrEFLAGS_IF;
        else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CR0_PE(1)");
            _new_code_path_;
            if (!_GetEFLAGS_VM) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("EFLAGS_VM(0)");
                if (_GetCPL <= (ntvdm64_type_unsigned_8)(_GetEFLAGS_IOPL))
                    _ClrEFLAGS_IF;
                else
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("EFLAGS_VM(1)");
                if (_GetEFLAGS_IOPL == 3)
                    _ClrEFLAGS_IF;
                else
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
    }
    else {
        cpu_state.data.ip++;
        _ClrEFLAGS_IF;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID STI(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("STI");
    i386(0xfb) {
        _adv;
        if (!_GetCR0_PE)
            _SetEFLAGS_IF;
        else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CR0_PE(1)");
            _new_code_path_;
            if (!_GetEFLAGS_VM) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("EFLAGS_VM(0)");
                if (_GetCPL <= (ntvdm64_type_unsigned_8)(_GetEFLAGS_IOPL))
                    _SetEFLAGS_IF;
                else
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("EFLAGS_VM(1)");
                if (_GetEFLAGS_IOPL == 3)
                    _SetEFLAGS_IF;
                else
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        instruction_state.data.flagMaskInt = NTVDM64_TYPE_TRUE;
    }
    else {
        cpu_state.data.ip++;
        _SetEFLAGS_IF;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID CLD(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CLD");
    i386(0xfc) {
        _adv;
        _ClrEFLAGS_DF;
    }
    else {
        cpu_state.data.ip++;
        _ClrEFLAGS_DF;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID STD(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CLD");
    i386(0xfd) {
        _adv;
        _SetEFLAGS_DF;
    }
    else {
        cpu_state.data.ip++;
        _SetEFLAGS_DF;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_FE(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_FE");
    i386(0xfe) {
        _adv;
    }
    else {
        cpu_state.data.ip++;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    switch (instruction_state.data.cr) {
    case 0: /* INC_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("INC_RM8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, instruction_state.data.crm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* DEC_RM8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DEC_RM8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, instruction_state.data.crm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_FF(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_8 modrm;
    ntvdm64_type_unsigned_16 newcs;
    ntvdm64_type_unsigned_32 neweip;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_FF");
    i386(0xff) {
        _adv;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, NTVDM64_TYPE_REFERENCE_OF(modrm), 1));
        switch (_GetModRM_REG(modrm)) {
        case 0: /* INC_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("INC_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, instruction_state.data.crm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* DEC_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DEC_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, instruction_state.data.crm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* CALL_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CALL_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
            neweip = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_call_near(context, neweip, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* CALL_M16_32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CALL_M16_32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize + 2));
            if (!instruction_state.data.flagMem) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
            neweip = (ntvdm64_type_unsigned_32)instruction_state.data.crm;
            instruction_state.data.mrm.offset += _GetOperandSize;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            newcs = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_call_far(context, newcs, neweip, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* JMP_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("JMP_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
            neweip = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jmp_near(context, neweip, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* JMP_M16_32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("JMP_M16_32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize + 2));
            if (!instruction_state.data.flagMem) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
            neweip = (ntvdm64_type_unsigned_32)instruction_state.data.crm;
            instruction_state.data.mrm.offset += _GetOperandSize;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            newcs = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jmp_far(context, newcs, neweip, _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* PUSH_RM32 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("PUSH_RM32");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* UndefinedOpcode */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else {
        cpu_state.data.ip++;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, NTVDM64_TYPE_REFERENCE_OF(modrm), 1));
        switch (_GetModRM_REG(modrm)) {
        case 0: /* INC_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("INC_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_inc(context, instruction_state.data.crm, 16));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* DEC_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("DEC_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_dec(context, instruction_state.data.crm, 16));
            instruction_state.data.crm = instruction_state.data.result;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* CALL_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CALL_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            neweip = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_call_near(context, neweip, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* CALL_M16_16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CALL_M16_16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 4));
            if (!instruction_state.data.flagMem) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            neweip = (ntvdm64_type_unsigned_16)instruction_state.data.crm;
            instruction_state.data.mrm.offset += 2;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            newcs = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_call_far(context, newcs, neweip, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* JMP_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("JMP_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            neweip = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jmp_near(context, neweip, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* JMP_M16_16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("JMP_M16_16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 4));
            if (!instruction_state.data.flagMem) {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            neweip = (ntvdm64_type_unsigned_16)instruction_state.data.crm;
            instruction_state.data.mrm.offset += 2;
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            newcs = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jmp_far(context, newcs, neweip, 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* PUSH_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("PUSH_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), 2));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* UndefinedOpcode */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}

static C_VOID _d_modrm_creg(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_modrm_creg");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_modrm(context, 0, 4));
    if (instruction_state.data.flagMem) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    switch (instruction_state.data.cr) {
    case 0:
        instruction_state.data.rr = (ntvdm64_type_virtual_address)&cpu_state.data.cr0;
        instruction_state.data.cr = cpu_state.data.cr0;
        break;
    case 1:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        instruction_state.data.rr = (ntvdm64_type_virtual_address)&cpu_state.data.cr2;
        instruction_state.data.cr = cpu_state.data.cr2;
        break;
    case 3:
        instruction_state.data.rr = (ntvdm64_type_virtual_address)&cpu_state.data.cr3;
        instruction_state.data.cr = cpu_state.data.cr3;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _d_modrm_dreg(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_modrm_dreg");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_modrm(context, 0, 4));
    if (instruction_state.data.flagMem) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    switch (instruction_state.data.cr) {
    case 0:
        instruction_state.data.rr = (ntvdm64_type_virtual_address)&cpu_state.data.dr0;
        instruction_state.data.cr = cpu_state.data.dr0;
        break;
    case 1:
        instruction_state.data.rr = (ntvdm64_type_virtual_address)&cpu_state.data.dr1;
        instruction_state.data.cr = cpu_state.data.dr1;
        break;
    case 2:
        instruction_state.data.rr = (ntvdm64_type_virtual_address)&cpu_state.data.dr2;
        instruction_state.data.cr = cpu_state.data.dr2;
        break;
    case 3:
        instruction_state.data.rr = (ntvdm64_type_virtual_address)&cpu_state.data.dr3;
        instruction_state.data.cr = cpu_state.data.dr3;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        instruction_state.data.rr = (ntvdm64_type_virtual_address)&cpu_state.data.dr6;
        instruction_state.data.cr = cpu_state.data.dr6;
        break;
    case 7:
        instruction_state.data.rr = (ntvdm64_type_virtual_address)&cpu_state.data.dr7;
        instruction_state.data.cr = cpu_state.data.dr7;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _d_modrm_treg(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_modrm_treg");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_modrm(context, 0, 4));
    if (instruction_state.data.flagMem) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    switch (instruction_state.data.cr) {
    case 0:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        instruction_state.data.rr = (ntvdm64_type_virtual_address)&cpu_state.data.tr6;
        instruction_state.data.cr = cpu_state.data.tr6;
        break;
    case 7:
        instruction_state.data.rr = (ntvdm64_type_virtual_address)&cpu_state.data.tr7;
        instruction_state.data.cr = cpu_state.data.tr7;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _d_bit_rmimm(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_8 regbyte, ntvdm64_type_unsigned_8 rmbyte, ntvdm64_type_bool write) {
    /* xrm = actual destination, cimm = (1 << bitoffset) */
    ntvdm64_type_signed_16 bitoff16 = 0;
    ntvdm64_type_signed_32 bitoff32 = 0;
    ntvdm64_type_unsigned_32 bitoperand = 0;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_d_bit_rmimm");
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_kdf_modrm(context, regbyte, rmbyte));
    if (!regbyte) NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    switch (rmbyte) {
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("rmbyte(2)");
        if (instruction_state.data.flagMem && regbyte) {
            /* valid for btcc_m16_r16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(1),regbyte(1)");
            bitoff16 = (ntvdm64_type_signed_16)instruction_state.data.cr;
            if (bitoff16 >= 0)
                instruction_state.data.mrm.offset += 2 * (bitoff16 / 16);
            else
                instruction_state.data.mrm.offset += 2 * ((bitoff16 - 15) / 16);
            bitoperand = ((ntvdm64_type_unsigned_16)bitoff16) % 16;
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else if (regbyte) {
            bitoperand = instruction_state.data.cr % 16;
        } else {
            bitoperand = (NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm) % 16);
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(instruction_state.data.cimm = NTVDM64_TYPE_MASK_UNSIGNED_16((1 << bitoperand)));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("rmbyte(4)");
        if (instruction_state.data.flagMem && regbyte) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(1),regbyte(1)");
            bitoff32 = (ntvdm64_type_signed_32)instruction_state.data.cr;
            if (bitoff32 >= 0)
                instruction_state.data.mrm.offset += 4 * (bitoff32 / 32);
            else
                instruction_state.data.mrm.offset += 4 * ((bitoff32 - 31) / 32);
            bitoperand = ((ntvdm64_type_unsigned_32)bitoff32) % 32;
            NTVDM64_TYPE_TRACE_BLOCK_END;
        } else if (regbyte) {
            bitoperand = instruction_state.data.cr % 32;
        } else {
            bitoperand = (NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm) % 32);
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(instruction_state.data.cimm = NTVDM64_TYPE_MASK_UNSIGNED_32((1 << bitoperand)));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("rmbyte");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(rmbyte));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}

#define SHLD_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SHRD_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)

#define _kac_btcc \
do { \
    switch (bit) { \
    case 16: instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest);break; \
    case 32: instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest);break; \
    default: NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit"); \
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit)); \
        NTVDM64_TYPE_TRACE_BLOCK_END;break; \
    } \
    instruction_state.data.opr2 = bitoperand; \
    instruction_state.data.result = instruction_state.data.opr1; \
    NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, NTVDM64_TYPE_GET_BIT(instruction_state.data.opr1, instruction_state.data.opr2)); \
} while (0)

static C_VOID _a_bscc(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 csrc, ntvdm64_type_unsigned_8 bit, ntvdm64_type_bool forward) {
    ntvdm64_type_unsigned_32 temp;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_bscc");
    if (forward) temp = 0;
    else temp = bit - 1;
    switch (bit) {
    case 16:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_16(csrc);
        if (!instruction_state.data.opr1)
            _SetEFLAGS_ZF;
        else {
            _ClrEFLAGS_ZF;
            while (!NTVDM64_TYPE_GET_BIT(instruction_state.data.opr1, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << temp))) {
                if (forward) temp++;
                else temp--;
            }
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16(temp);
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_32(csrc);
        if (!instruction_state.data.opr1)
            _SetEFLAGS_ZF;
        else {
            _ClrEFLAGS_ZF;
            while (!NTVDM64_TYPE_GET_BIT(instruction_state.data.opr1, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << temp))) {
                if (forward) temp++;
                else temp--;
            }
            instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_32(temp);
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_bt(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_32 bitoperand, ntvdm64_type_unsigned_8 bit) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_bt");
    _kac_btcc;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_btc(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_32 bitoperand, ntvdm64_type_unsigned_8 bit) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_btc");
    _kac_btcc;
    NTVDM64_TYPE_MAKE_BIT(instruction_state.data.result, instruction_state.data.opr2, !_GetEFLAGS_CF);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_btr(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_32 bitoperand, ntvdm64_type_unsigned_8 bit) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_btr");
    _kac_btcc;
    NTVDM64_TYPE_CLEAR_BIT(instruction_state.data.result, instruction_state.data.opr2);
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_bts(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_32 bitoperand, ntvdm64_type_unsigned_8 bit) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_bts");
    _kac_btcc;
    NTVDM64_TYPE_SET_BIT(instruction_state.data.result, instruction_state.data.opr2);
    NTVDM64_TYPE_TRACE_CALL_END;
}

static C_VOID _a_imul2(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_64 csrc, ntvdm64_type_unsigned_8 bit) {
    ntvdm64_type_unsigned_64 temp;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_imul2");
    switch (bit) {
    case 16:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16+16)");
        _new_code_path_;
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_16((ntvdm64_type_signed_16)cdest);
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_16((ntvdm64_type_signed_16)csrc);
        temp = NTVDM64_TYPE_MASK_UNSIGNED_32((ntvdm64_type_signed_16)instruction_state.data.opr1 * (ntvdm64_type_signed_16)instruction_state.data.opr2);
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_16(temp);
        if (NTVDM64_TYPE_MASK_UNSIGNED_32(temp) != NTVDM64_TYPE_MASK_UNSIGNED_32((ntvdm64_type_signed_16)instruction_state.data.result)) {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        } else {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32+32");
        _new_code_path_;
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_32((ntvdm64_type_signed_32)cdest);
        instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_32((ntvdm64_type_signed_32)csrc);
        temp = NTVDM64_TYPE_MASK_UNSIGNED_64((ntvdm64_type_signed_32)instruction_state.data.opr1 * (ntvdm64_type_signed_32)instruction_state.data.opr2);
        instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_32(temp);
        if (NTVDM64_TYPE_MASK_UNSIGNED_64(temp) != NTVDM64_TYPE_MASK_UNSIGNED_64((ntvdm64_type_signed_32)instruction_state.data.result)) {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        } else {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_shld(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_64 csrc, ntvdm64_type_unsigned_8 count, ntvdm64_type_unsigned_8 bit) {
    ntvdm64_type_bool flagcf;
    ntvdm64_type_bool flagbit;
    ntvdm64_type_signed_32 i;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_shld");
    count &= 0x1f;
    if (!count) {
        NTVDM64_TYPE_TRACE_CALL_END;
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
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16)");
            instruction_state.data.bit = 16;
            instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest);
            instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_16(csrc);
            instruction_state.data.result = instruction_state.data.opr1;
            flagcf = !!NTVDM64_TYPE_GET_MSB_16(instruction_state.data.result);
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                    NTVDM64_TYPE_GET_BIT(instruction_state.data.result, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << (bit - count))));
            for (i = (ntvdm64_type_signed_32)(bit - 1); i >= (ntvdm64_type_signed_32)count; --i) {
                flagbit = NTVDM64_TYPE_GET_BIT(instruction_state.data.opr1, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << (i - count)));
                NTVDM64_TYPE_MAKE_BIT(instruction_state.data.result, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << i), flagbit);
            }
            for (i = (ntvdm64_type_signed_32)(count - 1); i >= 0; --i) {
                flagbit = NTVDM64_TYPE_GET_BIT(instruction_state.data.opr2, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << (i - count + bit)));
                NTVDM64_TYPE_MAKE_BIT(instruction_state.data.result, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << i), flagbit);
            }
            if (count == 1)
                NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                        ((!!NTVDM64_TYPE_GET_MSB_16(instruction_state.data.result)) ^ flagcf));
            else
                instruction_state.data.udf |= VCPU_EFLAGS_OF;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 32:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32)");
            instruction_state.data.bit = 32;
            instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest);
            instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_32(csrc);
            instruction_state.data.result = instruction_state.data.opr1;
            flagcf = !!NTVDM64_TYPE_GET_MSB_32(instruction_state.data.result);
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                    NTVDM64_TYPE_GET_BIT(instruction_state.data.result, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << (bit - count))));
            for (i = (ntvdm64_type_signed_32)(bit - 1); i >= (ntvdm64_type_signed_32)count; --i) {
                flagbit = NTVDM64_TYPE_GET_BIT(instruction_state.data.opr1, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << (i - count)));
                NTVDM64_TYPE_MAKE_BIT(instruction_state.data.result, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << i), flagbit);
            }
            for (i = (ntvdm64_type_signed_32)(count - 1); i >= 0; --i) {
                flagbit = NTVDM64_TYPE_GET_BIT(instruction_state.data.opr2, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << (i - count + bit)));
                NTVDM64_TYPE_MAKE_BIT(instruction_state.data.result, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << i), flagbit);
            }
            if (count == 1)
                NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                        ((!!NTVDM64_TYPE_GET_MSB_32(instruction_state.data.result)) ^ flagcf));
            else
                instruction_state.data.udf |= VCPU_EFLAGS_OF;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHLD_FLAG));
        instruction_state.data.udf |= VCPU_EFLAGS_AF;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID _a_shrd(core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_64 cdest, ntvdm64_type_unsigned_64 csrc, ntvdm64_type_unsigned_8 count, ntvdm64_type_unsigned_8 bit) {
    ntvdm64_type_bool flagcf;
    ntvdm64_type_bool flagbit;
    ntvdm64_type_signed_32 i;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_a_shrd");
    count &= 0x1f;
    if (!count) {
        NTVDM64_TYPE_TRACE_CALL_END;
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
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(16)");
            instruction_state.data.bit = 16;
            instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_16(cdest);
            instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_16(csrc);
            instruction_state.data.result = instruction_state.data.opr1;
            flagcf = !!NTVDM64_TYPE_GET_MSB_16(instruction_state.data.result);
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                    NTVDM64_TYPE_GET_BIT(instruction_state.data.result, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << (count - 1))));
            for (i = 0; i <= (ntvdm64_type_signed_32)(bit - count - 1); ++i) {
                flagbit = NTVDM64_TYPE_GET_BIT(instruction_state.data.opr1, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << (i + count)));
                NTVDM64_TYPE_MAKE_BIT(instruction_state.data.result, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << i), flagbit);
            }
            for (i = (ntvdm64_type_signed_32)(bit - count); i <= (ntvdm64_type_signed_32)(bit - 1); ++i) {
                flagbit = NTVDM64_TYPE_GET_BIT(instruction_state.data.opr2, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << (i + count - bit)));
                NTVDM64_TYPE_MAKE_BIT(instruction_state.data.result, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << i), flagbit);
            }
            if (count == 1)
                NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                        ((!!NTVDM64_TYPE_GET_MSB_16(instruction_state.data.result)) ^ flagcf));
            else
                instruction_state.data.udf |= VCPU_EFLAGS_OF;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 32:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit(32)");
            instruction_state.data.bit = 32;
            instruction_state.data.opr1 = NTVDM64_TYPE_MASK_UNSIGNED_32(cdest);
            instruction_state.data.opr2 = NTVDM64_TYPE_MASK_UNSIGNED_32(csrc);
            instruction_state.data.result = instruction_state.data.opr1;
            flagcf = !!NTVDM64_TYPE_GET_MSB_32(instruction_state.data.result);
            NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                    NTVDM64_TYPE_GET_BIT(instruction_state.data.result, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << (count - 1))));
            for (i = 0; i <= (ntvdm64_type_signed_32)(bit - count - 1); ++i) {
                flagbit = NTVDM64_TYPE_GET_BIT(instruction_state.data.opr1, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << (i + count)));
                NTVDM64_TYPE_MAKE_BIT(instruction_state.data.result, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << i), flagbit);
            }
            for (i = (ntvdm64_type_signed_32)(bit - count); i <= (ntvdm64_type_signed_32)(bit - 1); ++i) {
                flagbit = NTVDM64_TYPE_GET_BIT(instruction_state.data.opr2, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << (i + count - bit)));
                NTVDM64_TYPE_MAKE_BIT(instruction_state.data.result, NTVDM64_TYPE_MASK_UNSIGNED_64(1 << i), flagbit);
            }
            if (count == 1)
                NTVDM64_TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                        ((!!NTVDM64_TYPE_GET_MSB_32(instruction_state.data.result)) ^ flagcf));
            else
                instruction_state.data.udf |= VCPU_EFLAGS_OF;
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("bit");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHRD_FLAG));
        instruction_state.data.udf |= VCPU_EFLAGS_AF;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}

static C_VOID _m_setcc_rm(core_machine_cpu_execution_context *context, ntvdm64_type_bool condition) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("_m_setcc_rm");
    instruction_state.data.result = NTVDM64_TYPE_MASK_UNSIGNED_8(!!condition);
    instruction_state.data.crm = instruction_state.data.result;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    NTVDM64_TYPE_TRACE_CALL_END;
}

static C_VOID INS_0F_00(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_8 modrm;
    ntvdm64_type_unsigned_64 descriptor;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_0F_00");
    _adv;
    if (_IsProtected) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Protected");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, NTVDM64_TYPE_REFERENCE_OF(modrm), 1));
        switch (_GetModRM_REG(modrm)) {
        case 0: /* SLDT_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SLDT_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, ((_GetModRM_MOD(modrm) != 3) ? 2 : _GetOperandSize)));
            instruction_state.data.crm = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.ldtr.selector);
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, instruction_state.data.flagMem ? 2 : _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* STR_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("STR_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, ((_GetModRM_MOD(modrm) != 3) ? 2 : _GetOperandSize)));
            instruction_state.data.crm = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.tr.selector);
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, instruction_state.data.flagMem ? 2 : _GetOperandSize));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* LLDT_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("LLDT_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_ldtr(context, NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm)));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* LTR_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("LTR_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_tr(context, NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm)));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* VERR_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("VERR_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            if (_s_check_selector(context, NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm))) {
                _ClrEFLAGS_ZF;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("selector(valid)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm), NTVDM64_TYPE_REFERENCE_OF(descriptor)));
                if (_IsDescSys(descriptor) ||
                        (!_IsDescCodeConform(descriptor) &&
                         (_GetCPL > _GetDesc_DPL(descriptor) ||
                          _GetSelector_RPL(NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm)) > _GetDesc_DPL(descriptor)))) {
                    _ClrEFLAGS_ZF;
                } else {
                    if (_IsDescData(descriptor) || _IsDescCodeReadable(descriptor)) {
                        _SetEFLAGS_ZF;
                    } else {
                        _ClrEFLAGS_ZF;
                    }
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* VERW_RM16 */
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("VERW_RM16");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            if (_s_check_selector(context, NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm))) {
                _ClrEFLAGS_ZF;
            } else {
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("selector(valid)");
                NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm), NTVDM64_TYPE_REFERENCE_OF(descriptor)));
                if (_IsDescSys(descriptor) ||
                        (!_IsDescCodeConform(descriptor) &&
                         (_GetCPL > _GetDesc_DPL(descriptor) ||
                          _GetSelector_RPL(NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm)) > _GetDesc_DPL(descriptor)))) {
                    _ClrEFLAGS_ZF;
                } else {
                    if (_IsDescDataWritable(descriptor)) {
                        _SetEFLAGS_ZF;
                    } else {
                        _ClrEFLAGS_ZF;
                    }
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 6:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(6)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        case 7:
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("!Protected");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_0F_01(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_8 modrm;
    ntvdm64_type_unsigned_16 limit;
    ntvdm64_type_unsigned_32 base;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_0F_01");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, NTVDM64_TYPE_REFERENCE_OF(modrm), 1));
    switch (_GetModRM_REG(modrm)) {
    case 0: /* SGDT_M32_16 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SGDT_M32_16");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 6));
        if (!instruction_state.data.flagMem) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        instruction_state.data.crm = cpu_state.data.gdtr.limit;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
        instruction_state.data.mrm.offset += 2;
        switch (_GetOperandSize) {
        case 2:
            instruction_state.data.crm = NTVDM64_TYPE_MASK_UNSIGNED_24(cpu_state.data.gdtr.base);
            break;
        case 4:
            instruction_state.data.crm = NTVDM64_TYPE_MASK_UNSIGNED_32(cpu_state.data.gdtr.base);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 4));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* SIDT_M32_16 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SIDT_M32_16");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 6));
        if (!instruction_state.data.flagMem) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        instruction_state.data.crm = cpu_state.data.idtr.limit;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
        instruction_state.data.mrm.offset += 2;
        switch (_GetOperandSize) {
        case 2:
            instruction_state.data.crm = NTVDM64_TYPE_MASK_UNSIGNED_24(cpu_state.data.idtr.base);
            break;
        case 4:
            instruction_state.data.crm = NTVDM64_TYPE_MASK_UNSIGNED_32(cpu_state.data.idtr.base);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 4));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* LGDT_M32_16 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("LGDT_M32_16");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 6));
        if (!instruction_state.data.flagMem) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        limit = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        instruction_state.data.mrm.offset += 2;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 4));
        switch (_GetOperandSize) {
        case 2:
            base = NTVDM64_TYPE_MASK_UNSIGNED_24(instruction_state.data.crm);
            break;
        case 4:
            base = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        /* STD_PRINTF("LGDT_M32_16: executed at L%08X, read base=%08X, limit=%04X\n",
                instruction_state.data.linear, base, limit); */
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_gdtr(context, base, limit, _GetOperandSize));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* LIDT_M32_16 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("LIDT_M32_16");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 6));
        if (!instruction_state.data.flagMem) {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        limit = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        instruction_state.data.mrm.offset += 2;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 4));
        switch (_GetOperandSize) {
        case 2:
            base = NTVDM64_TYPE_MASK_UNSIGNED_24(instruction_state.data.crm);
            break;
        case 4:
            base = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
            break;
        default:
            NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        /* STD_PRINTF("LIDT_M32_16: executed at L%08X, read base=%08X, limit=%04X\n",
                 instruction_state.data.linear, base, limit); */
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_idtr(context, base, limit, _GetOperandSize));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SMSW_RM16 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("SMSW_RM16");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, ((_GetModRM_MOD(modrm) == 3) ? _GetOperandSize : 2)));
        instruction_state.data.crm = NTVDM64_TYPE_MASK_UNSIGNED_16(cpu_state.data.cr0);
        if (_GetOperandSize == 4 && !instruction_state.data.flagMem)
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 4));
        else
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(5)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* LMSW_RM16 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("LMSW_RM16");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_cr0_msw(context, NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm)));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID LAR_R32_RM32(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_16 selector;
    ntvdm64_type_unsigned_64 descriptor;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LAR_R32_RM32");
    _adv;
    if (_IsProtected) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Protected(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        selector = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        if (_s_check_selector(context, selector)) {
            _ClrEFLAGS_ZF;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("selector(valid)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, selector, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
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
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("EFLAGS_ZF");
                switch (_GetOperandSize) {
                case 2:
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
                    descriptor = (NTVDM64_TYPE_MASK_UNSIGNED_16(descriptor >> 32) & 0xff00);
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(descriptor), 2));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                    break;
                case 4:
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
                    descriptor = (NTVDM64_TYPE_MASK_UNSIGNED_32(descriptor >> 32) & 0x00ffff00);
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(descriptor), 4));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                    break;
                default:
                    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Protected(0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID LSL_R32_RM32(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_16 selector;
    ntvdm64_type_unsigned_32 limit;
    ntvdm64_type_unsigned_64 descriptor;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LSL_R32_RM32");
    _adv;
    if (_IsProtected) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Protected(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        selector = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        if (_s_check_selector(context, selector)) {
            _ClrEFLAGS_ZF;
        } else {
            NTVDM64_TYPE_TRACE_BLOCK_BEGIN("selector(valid)");
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, selector, NTVDM64_TYPE_REFERENCE_OF(descriptor)));
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
                NTVDM64_TYPE_TRACE_BLOCK_BEGIN("EFLAGS_ZF");
                limit = _IsDescSegGranularLarge(descriptor) ?
                        ((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) : _GetDescSeg_Limit(descriptor);
                switch (_GetOperandSize) {
                case 2:
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(limit), 2));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                    break;
                case 4:
                    NTVDM64_TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
                    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(limit), 4));
                    NTVDM64_TYPE_TRACE_BLOCK_END;
                    break;
                default:
                    NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                NTVDM64_TYPE_TRACE_BLOCK_END;
            }
            NTVDM64_TYPE_TRACE_BLOCK_END;
        }
        NTVDM64_TYPE_TRACE_BLOCK_END;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("Protected(0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID CLTS(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CLTS");
    _new_code_path_;
    _adv;
    if (!_GetCR0_PE) {
        _ClrCR0_TS;
    } else {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CR0_PE(1)");
        if (_GetCPL > 0)
            NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        else
            _ClrCR0_TS;
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo WBINVD(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("WBINVD");
    UndefinedOpcode(context);
    instruction_state.data.flagIgnore = NTVDM64_TYPE_TRUE;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_R32_CR(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_R32_CR");
    _adv;
    if (_GetCPL) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm_creg(context));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rrm, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.cr), 4));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_R32_DR(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_R32_DR");
    _new_code_path_;
    _adv;
    if (_GetCPL) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm_dreg(context));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rrm, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.cr), 4));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_CR_R32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_CR_R32");
    _adv;
    if (_GetCPL) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm_creg(context));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), 4));
    /* if (instruction_state.data.rr == (ntvdm64_type_virtual_address)&cpu_state.data.cr0) {
        STD_PRINTF("MOV_CR_R32: executed at L%08X, CR0=%08X\n", instruction_state.data.linear, cpu_state.data.cr0);
    }
    if (instruction_state.data.rr == (ntvdm64_type_virtual_address)&cpu_state.data.cr2) {
        STD_PRINTF("MOV_CR_R32: executed at L%08X, CR2=%08X\n", instruction_state.data.linear, cpu_state.data.cr2);
    }
    if (instruction_state.data.rr == (ntvdm64_type_virtual_address)&cpu_state.data.cr3) {
        STD_PRINTF("MOV_CR_R32: executed at L%08X, CR3=%08X\n", instruction_state.data.linear, cpu_state.data.cr3);
    } */
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_DR_R32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_DR_R32");
    _new_code_path_;
    _adv;
    if (_GetCPL) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm_dreg(context));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), 4));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_R32_TR(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_R32_TR");
    _new_code_path_;
    _adv;
    if (_GetCPL) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm_treg(context));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rrm, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.cr), 4));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOV_TR_R32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOV_TR_R32");
    _new_code_path_;
    _adv;
    if (_GetCPL) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm_treg(context));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), 4));
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo WRMSR(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("WRMSR");
    UndefinedOpcode(context);
    instruction_state.data.flagIgnore = NTVDM64_TYPE_TRUE;
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo RDMSR(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("RDMSR");
    UndefinedOpcode(context);
    instruction_state.data.flagIgnore = NTVDM64_TYPE_TRUE;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JO_REL32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JO_REL32");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_OF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JNO_REL32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNO_REL32");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_OF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JC_REL32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JC_REL32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_CF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JNC_REL32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNC_REL32");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_CF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JZ_REL32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JZ_REL32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_ZF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JNZ_REL32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNZ_REL32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_ZF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JNA_REL32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNA_REL32");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize,
                (_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JA_REL32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JA_REL32");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize,
                !(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JS_REL32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JS_REL32");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_SF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JNS_REL32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNS_REL32");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_SF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JP_REL32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JP_REL32");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_PF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JNP_REL32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNP_REL32");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_PF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JL_REL32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JL_REL32");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, (_GetEFLAGS_SF != _GetEFLAGS_OF)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JNL_REL32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNL_REL32");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, (_GetEFLAGS_SF == _GetEFLAGS_OF)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JNG_REL32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JNG_REL32");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize,
                (_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF))));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID JG_REL32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("JG_REL32");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_jcc(context, NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize,
                (!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF))));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SETO_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETO_RM8");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_OF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SETNO_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETO_RM8");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, !_GetEFLAGS_OF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SETC_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETC_RM8");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_CF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SETNC_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETNC_RM8");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, !_GetEFLAGS_CF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SETZ_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETZ_RM8");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_ZF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SETNZ_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETNZ_RM8");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, !_GetEFLAGS_ZF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SETNA_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETNA_RM8");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_CF || _GetEFLAGS_ZF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SETA_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETA_RM8");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, !(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SETS_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETS_RM8");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_SF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SETNS_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETNS_RM8");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, !_GetEFLAGS_SF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SETP_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETP_RM8");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_PF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SETNP_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETNP_RM8");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, !_GetEFLAGS_PF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SETL_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETL_RM8");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_SF != _GetEFLAGS_OF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SETNL_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETNL_RM8");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_SF == _GetEFLAGS_OF));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SETNG_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETNG_RM8");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SETG_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SETG_RM8");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, !_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_FS(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_32 xs_sel;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_FS");
    _adv;
    xs_sel = cpu_state.data.fs.selector;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID POP_FS(core_machine_cpu_execution_context *context) {
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    ntvdm64_type_unsigned_32 xs_sel;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_FS");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_fs(context, NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo CPUID(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("CPUID");
    UndefinedOpcode(context);
    instruction_state.data.flagIgnore = NTVDM64_TYPE_TRUE;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID BT_RM32_R32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BT_RM32_R32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_bit_rmimm(context, _GetOperandSize, _GetOperandSize, 0));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_bt(context, (ntvdm64_type_unsigned_32)instruction_state.data.crm, (ntvdm64_type_unsigned_32)instruction_state.data.cimm, _GetOperandSize * 8));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SHLD_RM32_R32_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SHLD_RM32_R32_I8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shld(context, instruction_state.data.crm, instruction_state.data.cr, NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SHLD_RM32_R32_CL(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SHLD_RM32_R32_CL");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shld(context, instruction_state.data.crm, instruction_state.data.cr, cpu_state.data.cl, _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_GS(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_32 xs_sel;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("PUSH_GS");
    _adv;
    xs_sel = cpu_state.data.gs.selector;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_push(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID POP_GS(core_machine_cpu_execution_context *context) {
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    ntvdm64_type_unsigned_32 xs_sel;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("POP_GS");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_pop(context, NTVDM64_TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_load_gs(context, NTVDM64_TYPE_MASK_UNSIGNED_16(xs_sel)));
    NTVDM64_TYPE_TRACE_CALL_END;
}
_______todo RSM(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("RSM");
    UndefinedOpcode(context);
    instruction_state.data.flagIgnore = NTVDM64_TYPE_TRUE;
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID BTS_RM32_R32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BTS_RM32_R32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_bit_rmimm(context, _GetOperandSize, _GetOperandSize, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_bts(context, (ntvdm64_type_unsigned_32)instruction_state.data.crm, (ntvdm64_type_unsigned_32)instruction_state.data.cimm, _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SHRD_RM32_R32_I8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SHRD_RM32_R32_I8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shrd(context, instruction_state.data.crm, instruction_state.data.cr, NTVDM64_TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID SHRD_RM32_R32_CL(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("SHRD_RM32_R32_CL");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_shrd(context, instruction_state.data.crm, instruction_state.data.cr, cpu_state.data.cl, _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID IMUL_R32_RM32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("IMUL_R32_RM32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_imul2(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID LSS_R32_M16_32(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_16 selector;
    ntvdm64_type_unsigned_32 offset;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LSS_R32_M16_32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize + 2));
    if (!instruction_state.data.flagMem) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        offset = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        break;
    case 4:
        offset = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    instruction_state.data.mrm.offset += _GetOperandSize;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
    selector = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.ss, instruction_state.data.rr, selector, offset, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID BTR_RM32_R32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BTR_RM32_R32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_bit_rmimm(context, _GetOperandSize, _GetOperandSize, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_btr(context, (ntvdm64_type_unsigned_32)instruction_state.data.crm, (ntvdm64_type_unsigned_32)instruction_state.data.cimm, _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID LFS_R32_M16_32(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_16 selector;
    ntvdm64_type_unsigned_32 offset;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LFS_R32_M16_32");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize + 2));
    if (!instruction_state.data.flagMem) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        offset = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        break;
    case 4:
        offset = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    instruction_state.data.mrm.offset += _GetOperandSize;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
    selector = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.fs, instruction_state.data.rr, selector, offset, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID LGS_R32_M16_32(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_16 selector;
    ntvdm64_type_unsigned_32 offset;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("LGS_R32_M16_32");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize + 2));
    if (!instruction_state.data.flagMem) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    switch (_GetOperandSize) {
    case 2:
        offset = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        break;
    case 4:
        offset = NTVDM64_TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    instruction_state.data.mrm.offset += _GetOperandSize;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
    selector = NTVDM64_TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.gs, instruction_state.data.rr, selector, offset, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOVZX_R32_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVZX_R32_RM8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    instruction_state.data.crm = (ntvdm64_type_unsigned_8)instruction_state.data.crm;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOVZX_R32_RM16(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVZX_R32_RM16");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 4, 2));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
    instruction_state.data.crm = (ntvdm64_type_unsigned_16)instruction_state.data.crm;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), 4));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID INS_0F_BA(core_machine_cpu_execution_context *context) {
    ntvdm64_type_bool write = 0;
    ntvdm64_type_unsigned_8 modrm = 0x00;
    NTVDM64_TYPE_TRACE_CALL_BEGIN("INS_0F_BA");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, NTVDM64_TYPE_REFERENCE_OF(modrm), 1));
    if (_GetModRM_REG(modrm) == 4) write = 0;
    else write = 1;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_bit_rmimm(context, 0, _GetOperandSize, write));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    switch (instruction_state.data.cr) {
    case 0:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 1:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 3:
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* BT_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("BT_RM32_I8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_bt(context, (ntvdm64_type_unsigned_32)instruction_state.data.crm, (ntvdm64_type_unsigned_32)instruction_state.data.cimm, _GetOperandSize * 8));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* BTS_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("BTS_RM32_I8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_bts(context, (ntvdm64_type_unsigned_32)instruction_state.data.crm, (ntvdm64_type_unsigned_32)instruction_state.data.cimm, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* BTR_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("BTR_RM32_I8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_btr(context, (ntvdm64_type_unsigned_32)instruction_state.data.crm, (ntvdm64_type_unsigned_32)instruction_state.data.cimm, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* BTC_RM32_I8 */
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("BTC_RM32_I8");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_btc(context, (ntvdm64_type_unsigned_32)instruction_state.data.crm, (ntvdm64_type_unsigned_32)instruction_state.data.cimm, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
        NTVDM64_TYPE_TRACE_BLOCK_END;
        break;
    default:
        NTVDM64_TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID BTC_RM32_R32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BTC_RM32_R32");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_bit_rmimm(context, _GetOperandSize, _GetOperandSize, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_btc(context, (ntvdm64_type_unsigned_32)instruction_state.data.crm, (ntvdm64_type_unsigned_32)instruction_state.data.cimm, _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID BSF_R32_RM32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BSF_R32_RM32");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_bscc(context, instruction_state.data.crm, _GetOperandSize * 8, 1));
    if (!_GetEFLAGS_ZF) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("EFLAGS_ZF(0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID BSR_R32_RM32(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("BSR_R32_RM32");
    _new_code_path_;
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_a_bscc(context, instruction_state.data.crm, _GetOperandSize * 8, 0));
    if (!_GetEFLAGS_ZF) {
        NTVDM64_TYPE_TRACE_BLOCK_BEGIN("EFLAGS_ZF(0)");
        NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
        NTVDM64_TYPE_TRACE_BLOCK_END;
    }
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOVSX_R32_RM8(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVSX_R32_RM8");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, 1));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    instruction_state.data.crm = (ntvdm64_type_signed_8)instruction_state.data.crm;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
    NTVDM64_TYPE_TRACE_CALL_END;
}
static C_VOID MOVSX_R32_RM16(core_machine_cpu_execution_context *context) {
    NTVDM64_TYPE_TRACE_CALL_BEGIN("MOVSX_R32_RM16");
    _adv;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 4, 2));
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
    instruction_state.data.crm = (ntvdm64_type_signed_16)instruction_state.data.crm;
    NTVDM64_TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, NTVDM64_TYPE_REFERENCE_OF(instruction_state.data.crm), 4));
    NTVDM64_TYPE_TRACE_CALL_END;
}

static C_VOID ExecInit(core_machine_cpu_execution_context *context) {
    instruction_state.data.flagIgnore = NTVDM64_TYPE_FALSE;
    instruction_state.data.msize = 0;
    instruction_state.data.reccs = cpu_state.data.cs.selector;
    instruction_state.data.receip = cpu_state.data.eip;
    instruction_state.data.linear = cpu_state.data.cs.base + cpu_state.data.eip;
    if (core_machine_cpu_execution_read_linear(context,
        instruction_state.data.linear,
        (ntvdm64_type_virtual_address) instruction_state.data.opcodes, 15)) {
        instruction_state.data.oplen = 0;
    } else {
        instruction_state.data.oplen = 15;
    }

    instruction_state.data.flagLock = NTVDM64_TYPE_FALSE;
    instruction_state.data.oldcpu = cpu_state;
    instruction_state.data.roverds = &cpu_state.data.ds;
    instruction_state.data.roverss = &cpu_state.data.ss;
    instruction_state.data.prefix_rep = PREFIX_REP_NONE;
    instruction_state.data.prefix_oprsize = NTVDM64_TYPE_FALSE;
    instruction_state.data.prefix_addrsize = NTVDM64_TYPE_FALSE;
    instruction_state.data.flagMem = NTVDM64_TYPE_FALSE;
    instruction_state.data.flagInsLoop = NTVDM64_TYPE_FALSE;
    instruction_state.data.flagMaskInt = NTVDM64_TYPE_FALSE;
    instruction_state.data.bit = 0;
    instruction_state.data.opr1 = 0;
    instruction_state.data.opr2 = 0;
    instruction_state.data.result = 0;
    instruction_state.data.udf = NTVDM64_TYPE_ZERO_32;
    instruction_state.data.mrm.rsreg = STD_NULL;
    instruction_state.data.mrm.offset = NTVDM64_TYPE_ZERO_32;
    instruction_state.data.except = NTVDM64_TYPE_ZERO_32;
    instruction_state.data.excode = NTVDM64_TYPE_ZERO_32;
#if VCPUINS_TRACE == 1
    if (context->trace != STD_NULL) ntvdm64_type_trace_initialize(context->trace);
#endif
    if (context->diagnostic_provider != STD_NULL &&
        context->diagnostic_provider->record_instruction != STD_NULL) {
        context->diagnostic_provider->record_instruction(context->diagnostic_context,
            &cpu_state, &instruction_state);
    }
}
static C_VOID ExecFinal(core_machine_cpu_execution_context *context) {
    if (instruction_state.data.flagInsLoop) {
        cpu_state.data.cs = instruction_state.data.oldcpu.data.cs;
        cpu_state.data.eip = instruction_state.data.oldcpu.data.eip;
    }
#if VCPUINS_TRACE == 1
    if (context->trace != STD_NULL && context->trace->callCount &&
        !instruction_state.data.except)
        _SetExcept_CE(0);
    if (context->trace != STD_NULL) ntvdm64_type_trace_finalize(context->trace);
#endif
    if (instruction_state.data.except) {
        if (context->diagnostic_provider != STD_NULL &&
            context->diagnostic_provider->record_fault != STD_NULL) {
            context->diagnostic_provider->record_fault(context->diagnostic_context,
                &instruction_state.data.oldcpu, &instruction_state);
        }
        cpu_state = instruction_state.data.oldcpu;
        if (NTVDM64_TYPE_GET_BIT(instruction_state.data.except, VCPUINS_EXCEPT_GP)) {
            ExecInit(context);
            NTVDM64_TYPE_CLEAR_BIT(instruction_state.data.except, VCPUINS_EXCEPT_GP);
            _e_except_n(context, 0x0d, _GetOperandSize);
        }
        core_machine_cpu_execution_request_stop(context);
    }
}
static C_VOID ExecIns(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_8 opcode = 0;
    ExecInit(context);
    do {
        NTVDM64_TYPE_TRACE_CALL_BEGIN("ExecIns");
        NTVDM64_TYPE_TRACE_CHECK_BREAK(_s_read_cs(context, cpu_state.data.eip, NTVDM64_TYPE_REFERENCE_OF(opcode), 1));
        NTVDM64_TYPE_TRACE_CHECK_BREAK(ExecCpuInstruction(instruction_state.connect.insTable[opcode]));
        NTVDM64_TYPE_TRACE_CHECK_BREAK(_s_test_eip(context));
        NTVDM64_TYPE_TRACE_CHECK_BREAK(_s_test_esp(context));
        NTVDM64_TYPE_TRACE_CALL_END;
    } while (_kdf_check_prefix(context, opcode));
    if (instruction_state.data.flagWE && instruction_state.data.weLinear == instruction_state.data.linear) {
        STD_PRINTF("Watch point caught at L%08x: EXECUTED\n", instruction_state.data.linear);
        /* printCpuReg(); */
        core_machine_cpu_execution_request_stop(context);
    }
    ExecFinal(context);
}
static C_VOID ExecInt(core_machine_cpu_execution_context *context) {
    ntvdm64_type_unsigned_8 intr = 0x00;
    /* hardware interrupt handler */
    if (instruction_state.data.flagMaskInt)
        return;
    if (!cpu_state.data.flagMaskNMI && cpu_state.data.flagNMI) {
        cpu_state.data.flagHalt = NTVDM64_TYPE_FALSE;
        cpu_state.data.flagNMI = NTVDM64_TYPE_FALSE;
        ExecInit(context);
        _e_intr_n(context, 0x02, _GetOperandSize);
        ExecFinal(context);
    }
    if (_GetEFLAGS_IF && core_machine_pic_scan_interrupt(
            context->pic_master, context->pic_slave)) {
        cpu_state.data.flagHalt = NTVDM64_TYPE_FALSE;
        intr = core_machine_pic_get_interrupt(context->pic_master,
            context->pic_slave);
        ExecInit(context);
        _e_intr_n(context, intr, _GetOperandSize);
        ExecFinal(context);
        instruction_state.data.flagIgnore = NTVDM64_TYPE_TRUE;
    }
    if (_GetEFLAGS_TF) {
        cpu_state.data.flagHalt = NTVDM64_TYPE_FALSE;
        ExecInit(context);
        _e_intr_n(context, 0x01, _GetOperandSize);
        ExecFinal(context);
    }
}

/* external interface */
ntvdm64_type_bool core_machine_cpu_execution_load_segment(
    core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg,
    ntvdm64_type_unsigned_16 selector) {
    ntvdm64_type_bool fail;
    ntvdm64_type_unsigned_32 oldexcept = instruction_state.data.except;
    instruction_state.data.except = 0;
    _ksa_load_sreg(context, rsreg, selector);
    fail = !!instruction_state.data.except;
    instruction_state.data.except = oldexcept;
    return fail;
}
ntvdm64_type_bool core_machine_cpu_execution_read_linear(
    core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 linear,
    ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_bool fail;
    ntvdm64_type_unsigned_32 oldexcept = instruction_state.data.except;
    instruction_state.data.except = 0;
    _kma_read_linear(context, linear, rdata, byte, 0x00, 1);
    fail = !!instruction_state.data.except;
    instruction_state.data.except = oldexcept;
    return fail;
}
ntvdm64_type_bool core_machine_cpu_execution_write_linear(
    core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 linear,
    ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte) {
    ntvdm64_type_bool fail;
    ntvdm64_type_unsigned_32 oldexcept = instruction_state.data.except;
    instruction_state.data.except = 0;
    _kma_write_linear(context, linear, rdata, byte, 0x00, 1);
    fail = !!instruction_state.data.except;
    instruction_state.data.except = oldexcept;
    return fail;
}

C_VOID core_machine_cpu_execution_initialize(
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
C_VOID core_machine_cpu_execution_reset(
    core_machine_cpu_execution_context *context) {
    STD_MEMSET((C_VOID *)(&instruction_state.data), NTVDM64_TYPE_ZERO_8, sizeof(t_cpuins_data));
}
C_VOID core_machine_cpu_execution_refresh(
    core_machine_cpu_execution_context *context) {
    if (!cpu_state.data.flagHalt) {
        ExecIns(context);
    }
    ExecInt(context);
}
C_VOID core_machine_cpu_execution_finalize(
    core_machine_cpu_execution_context *context)
{
    if (context != STD_NULL) {
        STD_FREE(context->trace);
        context->trace = STD_NULL;
    }
}
