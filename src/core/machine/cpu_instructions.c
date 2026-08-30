#include "type.h"
#include "core/machine/port.h"
#include "core/machine/memory.h"
#include "core/machine/pic.h"

#include "core/machine/transaction.h"

#include "core/machine/cpu_instructions.h"
#include "core/machine/cpu.h"

#define cpu_state (*context->cpu)
#define instruction_state (*context->instructions)
#define ExecCpuInstruction(handler) ((handler) ? ((handler)(context), 0) : 0)

#define TYPE_TRACE_CONTEXT (*context->trace)
#define TYPE_TRACE_ERROR instruction_state.data.except
#define TYPE_TRACE_SET_ERROR (_SetExcept_CE(0xffffffff))

/* indicates functions not implemented */
#define _______todo static C_VOID
/* Records an untested code path only when an explicit trace is active. */
#define _new_code_path_                       \
    do                                        \
    {                                         \
        if (context->trace != STD_NULL)       \
            type_trace_print(context->trace); \
    } while (0)

/* stack pointer size */
#define _GetStackSize (cpu_state.data.ss.seg.data.big ? 4 : 2)
/* operand size */
#define _GetOperandSize ((cpu_state.data.cs.seg.exec.defsize ^ instruction_state.data.prefix_oprsize) ? 4 : 2)
/* address size of the source operand */
#define _GetAddressSize ((cpu_state.data.cs.seg.exec.defsize ^ instruction_state.data.prefix_addrsize) ? 4 : 2)
/* if opcode indicates a prefix */
static C_VOID core_machine_cpu_execution_raise_exception(
    core_machine_cpu_execution_context *context, type_unsigned_32 exception,
    type_unsigned_32 code)
{
    TYPE_SET_BIT(instruction_state.data.except, exception);
    instruction_state.data.excode = code;
}
#define _SetExcept_DE(n) core_machine_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_DE, (n))
#define _SetExcept_PF(n) core_machine_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_PF, (n))
#define _SetExcept_GP(n) core_machine_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_GP, (n))
#define _SetExcept_SS(n) core_machine_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_SS, (n))
#define _SetExcept_UD(n) core_machine_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_UD, (n))
#define _SetExcept_NP(n) core_machine_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_NP, (n))
#define _SetExcept_BR(n) core_machine_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_BR, (n))
#define _SetExcept_TS(n) core_machine_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_TS, (n))
#define _SetExcept_NM(n) core_machine_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_NM, (n))
#define _SetExcept_MF(n) core_machine_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_MF, (n))
#define _SetExcept_FPU_UNSUPPORTED(n) core_machine_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_FPU_UNSUPPORTED, (n))
#define _SetExcept_CE(n) core_machine_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_CE, (n))

#define VCPU_DR6_BS 0x00004000u
#define VCPU_DR6_BT 0x00008000u
#define VCPU_DR7_LOCAL_ENABLE_MASK 0x00000155u


static C_VOID UndefinedOpcode(core_machine_cpu_execution_context *context);

/* The FLAGS image and a FLAGS load are distinct architectural operations, but
 * share one profile-owned set of defined 16-bit fields. An undefined bit is
 * canonicalized to zero in Core; that is a deterministic implementation
 * value, not a claim about a processor's externally observable bit image. */
static type_unsigned_16 _e_real_flags_defined_mask(
    const core_machine_cpu_execution_context *context)
{
    if (context == STD_NULL ||
        context->cpu_profile < CORE_MACHINE_CPU_PROFILE_80286) {
        return 0x0fd5u;
    }
    return 0x7fd5u;
}

static type_unsigned_16 _e_real_flags_load_16(
    const core_machine_cpu_execution_context *context, type_unsigned_16 flags)
{
    return TYPE_MASK_UNSIGNED_16((flags &
        _e_real_flags_defined_mask(context)) | 0x02u);
}

static type_unsigned_16 _e_real_flags_image_16(
    const core_machine_cpu_execution_context *context, type_unsigned_16 flags)
{
    return TYPE_MASK_UNSIGNED_16((flags &
        _e_real_flags_defined_mask(context)) | 0x02u);
}

static type_unsigned_32 _e_eflags_load(
    const core_machine_cpu_execution_context *context, type_unsigned_32 flags)
{
    if (context == STD_NULL ||
        context->cpu_profile < CORE_MACHINE_CPU_PROFILE_80386) {
        return _e_real_flags_load_16(context, TYPE_MASK_UNSIGNED_16(flags));
    }
    return flags | 0x02u;
}

static type_status _e_try_firmware_software_interrupt(
    core_machine_cpu_execution_context *context, type_unsigned_8 intid,
    type_bool *out_handled)
{
    core_machine_firmware_interrupt_frame frame;
    core_machine_firmware_interrupt_result result;
    type_status status;

    if (out_handled == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_handled = TYPE_FALSE;
    if (context == STD_NULL || context->firmware_interrupt_provider == STD_NULL) {
        return TYPE_STATUS_OK;
    }
    frame.ax = cpu_state.data.ax;
    frame.bx = cpu_state.data.bx;
    frame.cx = cpu_state.data.cx;
    frame.dx = cpu_state.data.dx;
    frame.si = cpu_state.data.si;
    frame.di = cpu_state.data.di;
    frame.bp = cpu_state.data.bp;
    frame.ds = cpu_state.data.ds.selector;
    frame.es = cpu_state.data.es.selector;
    frame.flags = _e_real_flags_image_16(context, cpu_state.data.flags);
    result.ax = frame.ax;
    result.flags = frame.flags;
    status = context->firmware_interrupt_provider(
        context->firmware_interrupt_context, intid, &frame, &result, out_handled);
    if (status != TYPE_STATUS_OK || !*out_handled) return status;
    cpu_state.data.ax = result.ax;
    cpu_state.data.flags = _e_real_flags_load_16(context, result.flags);
    return TYPE_STATUS_OK;
}

/* memory management unit */
/* kernel memory accessing */
/* read content from reference */
static C_VOID _kma_read_ref(core_machine_cpu_execution_context *context, type_virtual_address ref, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_kma_read_ref");
    STD_MEMCPY((C_VOID *)rdata, (C_VOID *)ref, byte);
    TYPE_TRACE_CALL_END;
}
/* write content to reference */
static C_VOID _kma_write_ref(core_machine_cpu_execution_context *context, type_virtual_address ref, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_kma_write_ref");
    STD_MEMCPY((C_VOID *)ref, (C_VOID *)rdata, byte);
    TYPE_TRACE_CALL_END;
}
/* Observation only: this does not assert that a serial logical access already
 * overlaps a physical 80386 external bus cycle. */
static C_VOID _kma_publish_external_cycle(
    core_machine_cpu_execution_context *context,
    core_machine_cpu_external_cycle_phase phase, type_unsigned_32 physical,
    type_unsigned_8 byte, type_bool write,
    core_machine_cpu_memory_access_provenance provenance)
{
    if (context != STD_NULL && !context->preview_mode &&
        context->external_cycle_provider != STD_NULL) {
        context->external_cycle_provider(context->external_cycle_context, phase,
            CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, physical, byte, write,
            provenance);
    }
}
static C_VOID _p_publish_external_cycle(
    core_machine_cpu_execution_context *context,
    core_machine_cpu_external_cycle_phase phase, type_unsigned_16 portid,
    type_unsigned_8 byte, type_bool write)
{
    if (context != STD_NULL && !context->preview_mode &&
        context->external_cycle_provider != STD_NULL) {
        context->external_cycle_provider(context->external_cycle_context, phase,
            CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT, portid, byte, write,
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
    }
}

static C_INT _kma_is_reset_vector_fetch(
    const core_machine_cpu_execution_context *context,
    type_unsigned_32 physical, type_unsigned_8 bytes,
    core_machine_cpu_memory_access_provenance provenance)
{
    type_unsigned_32 reset_vector;

    if (context == STD_NULL || bytes == 0u ||
        (provenance != CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH &&
         provenance != CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH)) {
        return 0;
    }
    if (context->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286) {
        reset_vector = 0x00fffff0u;
    } else if (context->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) {
        reset_vector = 0xfffffff0u;
    } else {
        return 0;
    }
    return physical >= reset_vector &&
        physical <= UINT32_MAX - (type_unsigned_32)(bytes - 1u);
}

/* read content from physical */
static C_VOID _kma_read_physical(core_machine_cpu_execution_context *context, type_unsigned_32 physical, type_virtual_address rdata, type_unsigned_8 byte, core_machine_cpu_memory_access_provenance provenance)
{
    type_status memory_status;
    TYPE_TRACE_CALL_BEGIN("_kma_read_physical");
    /* The processor owns the width of its external physical-address bus.
     * Board A20 routing remains in the memory owner and is applied afterwards.
     * In particular, the 80286 reset fetch at FFFFF0h wraps at 16 MiB rather
     * than falling into a synthetic 25th address bit. */
    if (context->cpu_profile <= CORE_MACHINE_CPU_PROFILE_80186) {
        physical &= 0x000fffffu;
    } else if (context->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286) {
        physical &= 0x00ffffffu;
    }
    _kma_publish_external_cycle(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN,
        physical, byte, TYPE_FALSE, provenance);
    if (!context->preview_mode && context->transaction != STD_NULL && core_machine_transaction_begin(
            context->transaction, CORE_MACHINE_TRANSACTION_OWNER_CPU,
            CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ, physical, byte, provenance) !=
            TYPE_STATUS_OK) {
        _kma_publish_external_cycle(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
            physical, byte, TYPE_FALSE, provenance);
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(physical));
    }
    memory_status = _kma_is_reset_vector_fetch(context, physical, byte, provenance) ?
        core_machine_memory_read_reset_physical(context->memory, physical, rdata,
            byte) : core_machine_memory_read_physical(context->memory, physical,
            rdata, byte);
    if (memory_status == TYPE_STATUS_UNSUPPORTED) {
        memory_status = core_machine_memory_read_physical(context->memory, physical,
            rdata, byte);
    }
    if (memory_status != TYPE_STATUS_OK) {
        if (!context->preview_mode) core_machine_transaction_cancel(context->transaction);
        _kma_publish_external_cycle(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
            physical, byte, TYPE_FALSE, provenance);
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(physical));
    }
    if (!context->preview_mode) core_machine_transaction_commit(context->transaction);
    _kma_publish_external_cycle(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT,
        physical, byte, TYPE_FALSE, provenance);
    TYPE_TRACE_CALL_END;
}/* write content to physical */
static C_VOID _kma_write_physical(core_machine_cpu_execution_context *context, type_unsigned_32 physical, type_virtual_address rdata, type_unsigned_8 byte, core_machine_cpu_memory_access_provenance provenance)
{
    TYPE_TRACE_CALL_BEGIN("_kma_write_physical");
    if (context->cpu_profile <= CORE_MACHINE_CPU_PROFILE_80186) {
        physical &= 0x000fffffu;
    } else if (context->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286) {
        physical &= 0x00ffffffu;
    }
    _kma_publish_external_cycle(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN,
        physical, byte, TYPE_TRUE, provenance);
    if (context->transaction != STD_NULL && core_machine_transaction_begin(
            context->transaction, CORE_MACHINE_TRANSACTION_OWNER_CPU,
            CORE_MACHINE_TRANSACTION_CPU_MEMORY_WRITE, physical, byte, provenance) !=
            TYPE_STATUS_OK) {
        _kma_publish_external_cycle(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
            physical, byte, TYPE_TRUE, provenance);
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(physical));
    }
    if (core_machine_memory_write_physical(context->memory, physical, rdata,
            byte) != TYPE_STATUS_OK) {
        core_machine_transaction_cancel(context->transaction);
        _kma_publish_external_cycle(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
            physical, byte, TYPE_TRUE, provenance);
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(physical));
    }
    core_machine_transaction_commit(context->transaction);
    _kma_publish_external_cycle(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT,
        physical, byte, TYPE_TRUE, provenance);
    TYPE_TRACE_CALL_END;
}typedef struct t_kma_linear_translation {
    type_unsigned_32 physical;
    type_unsigned_32 ppde;
    type_unsigned_32 ppte;
    type_unsigned_32 cpde;
    type_unsigned_32 cpte;
    type_bool paging;
} t_kma_linear_translation;

/* Validate a linear page translation without publishing page-table side effects. */
static C_VOID _kma_prepare_physical_linear(core_machine_cpu_execution_context *context, type_unsigned_32 linear, type_unsigned_8 byte, type_bool write, type_unsigned_8 vpl, t_kma_linear_translation *translation)
{
    type_unsigned_32 ppde, ppte; /* page table entries */
    type_unsigned_32 cpde, cpte;
    TYPE_TRACE_CALL_BEGIN("_kma_prepare_physical_linear");
    if (_GetLinear_Offset(linear) > TYPE_MASK_UNSIGNED_32(_GetPageSize - byte))
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (!_IsPaging)
    {
        translation->physical = linear;
        translation->paging = TYPE_FALSE;
        TYPE_TRACE_CALL_END;
        return;
    }
    ppde = _GetCR3_Base + _GetLinear_Dir(linear) * 4;
    TYPE_TRACE_CHECK_RETURN(_kma_read_physical(context, ppde, TYPE_REFERENCE_OF(cpde), 4, CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ));
    if (!_IsPageEntryPresent(cpde))
    {
        TYPE_TRACE_BLOCK_BEGIN("!PageDirEntryPresent");
        cpu_state.data.cr2 = linear;
        TYPE_TRACE_CHECK_RETURN(_SetExcept_PF(_MakePageFaultErrorCode(0, write, (vpl == 3))));
        return;
        TYPE_TRACE_BLOCK_END;
    }
    if (vpl == 0x03)
    {
        TYPE_TRACE_BLOCK_BEGIN("vpl(3)");
        if (!_GetPageEntry_US(cpde))
        {
            TYPE_TRACE_BLOCK_BEGIN("PageDirEntry_US(0)");
            cpu_state.data.cr2 = linear;
            TYPE_TRACE_CHECK_RETURN(_SetExcept_PF(_MakePageFaultErrorCode(1, write, 1)));
            return;
            TYPE_TRACE_BLOCK_END;
        }
        if (write && !_IsPageEntryWritable(cpde))
        {
            TYPE_TRACE_BLOCK_BEGIN("write,!PageDirEntryWritable");
            cpu_state.data.cr2 = linear;
            TYPE_TRACE_CHECK_RETURN(_SetExcept_PF(_MakePageFaultErrorCode(1, 1, 1)));
            return;
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
    }
    ppte = _GetPageEntry_Base(cpde) + _GetLinear_Page(linear) * 4;
    TYPE_TRACE_CHECK_RETURN(_kma_read_physical(context, ppte, TYPE_REFERENCE_OF(cpte), 4, CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ));
    if (!_IsPageEntryPresent(cpte))
    {
        TYPE_TRACE_BLOCK_BEGIN("!PageTabEntryPresent");
        cpu_state.data.cr2 = linear;
        TYPE_TRACE_CHECK_RETURN(_SetExcept_PF(_MakePageFaultErrorCode(0, write, (vpl == 3))));
        return;
        TYPE_TRACE_BLOCK_END;
    }
    if (vpl == 0x03)
    {
        TYPE_TRACE_BLOCK_BEGIN("vpl(3)");
        if (!_GetPageEntry_US(cpte))
        {
            TYPE_TRACE_BLOCK_BEGIN("PageTabEntry_US(0)");
            cpu_state.data.cr2 = linear;
            TYPE_TRACE_CHECK_RETURN(_SetExcept_PF(_MakePageFaultErrorCode(1, write, 1)));
            return;
            TYPE_TRACE_BLOCK_END;
        }
        if (write && !_IsPageEntryWritable(cpte))
        {
            TYPE_TRACE_BLOCK_BEGIN("write,!PageTabEntryWritable");
            cpu_state.data.cr2 = linear;
            TYPE_TRACE_CHECK_RETURN(_SetExcept_PF(_MakePageFaultErrorCode(1, 1, 1)));
            return;
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
    }
    translation->physical = _GetPageEntry_Base(cpte) + _GetLinear_Offset(linear);
    translation->ppde = ppde;
    translation->ppte = ppte;
    translation->cpde = cpde;
    translation->cpte = cpte;
    translation->paging = TYPE_TRUE;
    TYPE_TRACE_CALL_END;
}

/* Publish A/D only after the complete checked access has been preflighted. */
static C_VOID _kma_commit_physical_linear(core_machine_cpu_execution_context *context, t_kma_linear_translation *translation, type_bool write)
{
    TYPE_TRACE_CALL_BEGIN("_kma_commit_physical_linear");
    if (context->preview_mode || !translation->paging)
    {
        TYPE_TRACE_CALL_END;
        return;
    }
    _SetPageEntry_A(translation->cpde);
    TYPE_TRACE_CHECK_RETURN(_kma_write_physical(context, translation->ppde,
        TYPE_REFERENCE_OF(translation->cpde), 4, CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE));
    _SetPageEntry_A(translation->cpte);
    if (write)
        _SetPageEntry_D(translation->cpte);
    TYPE_TRACE_CHECK_RETURN(_kma_write_physical(context, translation->ppte,
        TYPE_REFERENCE_OF(translation->cpte), 4, CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE));
    TYPE_TRACE_CALL_END;
}

/* Translate and publish a single checked linear page access. */
static type_unsigned_32 _kma_physical_linear(core_machine_cpu_execution_context *context, type_unsigned_32 linear, type_unsigned_8 byte, type_bool write, type_unsigned_8 vpl)
{
    t_kma_linear_translation translation;

    TYPE_TRACE_CALL_BEGIN("_kma_physical_linear");
    TYPE_TRACE_CHECK_RETURN_ZERO(_kma_prepare_physical_linear(context, linear,
        byte, write, vpl, &translation));
    TYPE_TRACE_CHECK_RETURN_ZERO(_kma_commit_physical_linear(context,
        &translation, write));
    TYPE_TRACE_CALL_END;
    return translation.physical;
}
/* translate logical to linear - segmentation mechanism */
static type_unsigned_32 _kma_linear_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, type_unsigned_32 offset, type_unsigned_8 byte, type_bool write, type_unsigned_8 vpl, type_bool force)
{
    type_unsigned_32 linear;
    type_unsigned_32 upper, lower;
    TYPE_TRACE_CALL_BEGIN("_kma_linear_logical");
    switch (rsreg->sregtype)
    {
    case SREG_CODE:
        TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_CODE)");
        if (!rsreg->flagValid)
            TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
        if (_IsProtected)
        {
            TYPE_TRACE_BLOCK_BEGIN("Protected");
            if (!force)
            {
                TYPE_TRACE_BLOCK_BEGIN("force");
                if (write)
                {
                    TYPE_TRACE_BLOCK_BEGIN("write");
                    TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
                    TYPE_TRACE_BLOCK_END;
                }
                if (!rsreg->seg.exec.readable)
                {
                    TYPE_TRACE_BLOCK_BEGIN("!readable");
                    TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
                    TYPE_TRACE_BLOCK_END;
                }
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_BLOCK_END;
        }
        lower = 0x00000000;
        upper = rsreg->limit;
        TYPE_TRACE_BLOCK_END;
        break;
    case SREG_STACK:
        TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_STACK)");
        if (!rsreg->flagValid)
            TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
        if (_IsProtected)
        {
            TYPE_TRACE_BLOCK_BEGIN("Protected");
            if (rsreg->seg.executable || !rsreg->seg.data.writable)
                TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
            TYPE_TRACE_BLOCK_END;
        }
        if (rsreg->seg.data.expdown)
        {
            lower = rsreg->limit + 1;
            upper = rsreg->seg.data.big ? 0xffffffff : 0x0000ffff;
        }
        else
        {
            lower = 0x00000000;
            upper = rsreg->limit;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case SREG_DATA:
        TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_DATA)");
        if (!rsreg->flagValid)
        {
            TYPE_TRACE_BLOCK_BEGIN("selector(null)");
            TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
            TYPE_TRACE_BLOCK_END;
        }
        if (_IsProtected)
        {
            TYPE_TRACE_BLOCK_BEGIN("Protected");
            if (_IsSelectorNull(rsreg->selector))
            {
                TYPE_TRACE_BLOCK_BEGIN("selector(null)");
                TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
                TYPE_TRACE_BLOCK_END;
            }
            if (rsreg->seg.executable && !rsreg->seg.exec.readable)
                TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
            if (!force)
            {
                TYPE_TRACE_BLOCK_BEGIN("force");
                if (write)
                {
                    TYPE_TRACE_BLOCK_BEGIN("write");
                    if (rsreg->seg.executable)
                    {
                        TYPE_TRACE_BLOCK_BEGIN("executable");
                        TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
                        TYPE_TRACE_BLOCK_END;
                    }
                    else
                    {
                        TYPE_TRACE_BLOCK_BEGIN("!executable");
                        if (!rsreg->seg.data.writable)
                        {
                            TYPE_TRACE_BLOCK_BEGIN("!writable");
                            TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
                            TYPE_TRACE_BLOCK_END;
                        }
                        TYPE_TRACE_BLOCK_END;
                    }
                    TYPE_TRACE_BLOCK_END;
                }
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_BLOCK_END;
        }
        if (rsreg->seg.data.expdown)
        {
            lower = rsreg->limit + 1;
            upper = rsreg->seg.data.big ? 0xffffffff : 0x0000ffff;
        }
        else
        {
            lower = 0x00000000;
            upper = rsreg->limit;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case SREG_GDTR:
        TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_GDTR)");
        if (!_GetCR0_PE)
            TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
        lower = 0x00000000;
        upper = rsreg->limit;
        TYPE_TRACE_BLOCK_END;
        break;
    case SREG_IDTR:
        TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_IDTR)");
        lower = 0x00000000;
        upper = rsreg->limit;
        TYPE_TRACE_BLOCK_END;
        break;
    case SREG_LDTR:
        TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_LDTR)");
        if (!_GetCR0_PE)
            TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
        if (_GetSelector_TI(rsreg->selector))
            TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
        if (!rsreg->flagValid || _IsSelectorNull(rsreg->selector))
        {
            TYPE_TRACE_BLOCK_BEGIN("selector(null)");
            TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
            TYPE_TRACE_BLOCK_END;
        }
        lower = 0x00000000;
        upper = rsreg->limit;
        TYPE_TRACE_BLOCK_END;
        break;
    case SREG_TR:
        TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_TR)");
        if (!_GetCR0_PE)
            TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
        if (!rsreg->flagValid || _IsSelectorNull(rsreg->selector))
            TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
        if (_GetSelector_TI(rsreg->selector))
            TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
        lower = 0x00000000;
        upper = rsreg->limit;
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
    }
    /* Real-address data accesses may use 32-bit offsets on an 80386. */
    if (!_IsProtected && rsreg->sregtype == SREG_DATA && byte != 0u) {
        lower = 0x00000000;
        upper = 0xffffffff;
    }
    linear = rsreg->base + offset;
    if (offset < lower || offset > upper || (byte != 0u &&
        (type_unsigned_32)(byte - 1u) > upper - offset))
    {
        TYPE_TRACE_BLOCK_BEGIN("offset(<lower/>upper)");
        switch (rsreg->sregtype)
        {
        case SREG_STACK:
            TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_STACK)");
            TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_SS(0));
            TYPE_TRACE_BLOCK_END;
            break;
        case SREG_TR:
            TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_TR)");
            TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_TS(rsreg->selector));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_BLOCK_BEGIN("sregtype(default)");
            TYPE_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
            TYPE_TRACE_BLOCK_END;
            break;
        }
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
    return linear;
}
/* read content from logical */
static C_VOID _kma_read_linear(core_machine_cpu_execution_context *context, type_unsigned_32 linear, type_virtual_address rdata, type_unsigned_8 byte, type_unsigned_8 vpl, type_bool force)
{
    type_unsigned_32 phy1, phy2;
    type_unsigned_8 byte1, byte2;
    TYPE_TRACE_CALL_BEGIN("_kma_read_logical");
    if (_GetLinear_Offset(linear) > TYPE_MASK_UNSIGNED_32(_GetPageSize - byte))
    {
        TYPE_TRACE_BLOCK_BEGIN("Linear_Offset(>PageSize)");
        byte1 = _GetPageSize - _GetLinear_Offset(linear);
        byte2 = byte - byte1;
        {
            t_kma_linear_translation translation1, translation2;

            TYPE_TRACE_CHECK_RETURN(_kma_prepare_physical_linear(context,
                linear, byte1, 0, vpl, &translation1));
            TYPE_TRACE_CHECK_RETURN(_kma_prepare_physical_linear(context,
                linear + byte1, byte2, 0, vpl, &translation2));
            TYPE_TRACE_CHECK_RETURN(_kma_commit_physical_linear(context,
                &translation1, 0));
            TYPE_TRACE_CHECK_RETURN(_kma_commit_physical_linear(context,
                &translation2, 0));
            phy1 = translation1.physical;
            phy2 = translation2.physical;
        }
        TYPE_TRACE_CHECK_RETURN(_kma_read_physical(context, phy1, rdata, byte1, context->memory_access_provenance));
        TYPE_TRACE_CHECK_RETURN(_kma_read_physical(context, phy2, rdata + byte1, byte2, context->memory_access_provenance));
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("Linear_Offset(<=PageSize)");
        byte1 = byte;
        TYPE_TRACE_CHECK_RETURN(phy1 = _kma_physical_linear(context, linear, byte1, 0, vpl));
        TYPE_TRACE_CHECK_RETURN(_kma_read_physical(context, phy1, rdata, byte1, context->memory_access_provenance));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
/* write content to logical */
static C_VOID _kma_write_linear(core_machine_cpu_execution_context *context, type_unsigned_32 linear, type_virtual_address rdata, type_unsigned_8 byte, type_unsigned_8 vpl, type_bool force)
{
    type_unsigned_32 phy1, phy2;
    type_unsigned_8 byte1, byte2;
    TYPE_TRACE_CALL_BEGIN("_kma_write_linear");
    if (_GetLinear_Offset(linear) > TYPE_MASK_UNSIGNED_32(_GetPageSize - byte))
    {
        TYPE_TRACE_BLOCK_BEGIN("Linear_Offset(>PageSize)");
        byte1 = _GetPageSize - _GetLinear_Offset(linear);
        byte2 = byte - byte1;
        {
            t_kma_linear_translation translation1, translation2;

            TYPE_TRACE_CHECK_RETURN(_kma_prepare_physical_linear(context,
                linear, byte1, 1, vpl, &translation1));
            TYPE_TRACE_CHECK_RETURN(_kma_prepare_physical_linear(context,
                linear + byte1, byte2, 1, vpl, &translation2));
            TYPE_TRACE_CHECK_RETURN(_kma_commit_physical_linear(context,
                &translation1, 1));
            TYPE_TRACE_CHECK_RETURN(_kma_commit_physical_linear(context,
                &translation2, 1));
            phy1 = translation1.physical;
            phy2 = translation2.physical;
        }
        TYPE_TRACE_CHECK_RETURN(_kma_write_physical(context, phy1, rdata, byte1, context->memory_access_provenance));
        TYPE_TRACE_CHECK_RETURN(_kma_write_physical(context, phy2, rdata + byte1, byte2, context->memory_access_provenance));
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("Linear_Offset(<=PageSize)");
        byte1 = byte;
        TYPE_TRACE_CHECK_RETURN(phy1 = _kma_physical_linear(context, linear, byte1, 1, vpl));
        TYPE_TRACE_CHECK_RETURN(_kma_write_physical(context, phy1, rdata, byte1, context->memory_access_provenance));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
/* read content from logical */
static C_VOID _kma_read_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte, type_unsigned_8 vpl, type_bool force)
{
    /* type_native_unsigned i; */
    type_unsigned_32 linear;
    TYPE_TRACE_CALL_BEGIN("_kma_read_logical");
    TYPE_TRACE_CHECK_RETURN(linear = _kma_linear_logical(context, rsreg, offset, byte, 0, vpl, force));
    TYPE_TRACE_CHECK_RETURN(_kma_read_linear(context, linear, rdata, byte, vpl, force));
    if (!force && instruction_state.data.msize <
        CORE_MACHINE_CPU_INSTRUCTION_MEMORY_ACCESS_CAPACITY)
    {
        TYPE_TRACE_BLOCK_BEGIN("!force");
        instruction_state.data.mem[instruction_state.data.msize].flagWrite = TYPE_FALSE;
        instruction_state.data.mem[instruction_state.data.msize].data = 0;
        STD_MEMCPY((C_VOID *)TYPE_REFERENCE_OF(instruction_state.data.mem[instruction_state.data.msize].data), (C_VOID *)rdata, byte);
        instruction_state.data.mem[instruction_state.data.msize].byte = byte;
        instruction_state.data.mem[instruction_state.data.msize].linear = linear;
        if (instruction_state.data.flagWR)
        {
            if (instruction_state.data.wrLinear >= instruction_state.data.mem[instruction_state.data.msize].linear &&
                instruction_state.data.wrLinear < instruction_state.data.mem[instruction_state.data.msize].linear + byte)
            {
                STD_PRINTF("Watch point caught at L%08x: READ %01x BYTES OF DATA=%08x FROM L%08x\n", instruction_state.data.linear,
                           instruction_state.data.mem[instruction_state.data.msize].byte,
                           instruction_state.data.mem[instruction_state.data.msize].data,
                           instruction_state.data.mem[instruction_state.data.msize].linear);
            }
        }
        /* for (i = 0;i < instruction_state.data.msize;++i) {
            if (instruction_state.data.mem[i].flagWrite == instruction_state.data.mem[instruction_state.data.msize].flagWrite &&
                instruction_state.data.mem[i].linear == instruction_state.data.mem[instruction_state.data.msize].linear) {
                TYPE_TRACE_BLOCK_BEGIN("mem(same)");
                TYPE_TRACE_IMPOSSIBLE_RETURN_ZERO;
                TYPE_TRACE_CALL_END;
            }
        } */
        instruction_state.data.msize++;
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
/* write content to logical */
static C_VOID _kma_write_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte, type_unsigned_8 vpl, type_bool force)
{
    /* type_native_unsigned i; */
    type_unsigned_32 linear;
    TYPE_TRACE_CALL_BEGIN("_kma_write_logical");
    TYPE_TRACE_CHECK_RETURN(linear = _kma_linear_logical(context, rsreg, offset, byte, 1, vpl, force));
    TYPE_TRACE_CHECK_RETURN(_kma_write_linear(context, linear, rdata, byte, vpl, force));
    if (!force && instruction_state.data.msize <
        CORE_MACHINE_CPU_INSTRUCTION_MEMORY_ACCESS_CAPACITY)
    {
        TYPE_TRACE_BLOCK_BEGIN("!force");
        instruction_state.data.mem[instruction_state.data.msize].flagWrite = TYPE_TRUE;
        instruction_state.data.mem[instruction_state.data.msize].data = 0;
        STD_MEMCPY((C_VOID *)TYPE_REFERENCE_OF(instruction_state.data.mem[instruction_state.data.msize].data), (C_VOID *)rdata, byte);
        instruction_state.data.mem[instruction_state.data.msize].byte = byte;
        instruction_state.data.mem[instruction_state.data.msize].linear = linear;
        if (instruction_state.data.flagWW)
        {
            if (instruction_state.data.wwLinear >= instruction_state.data.mem[instruction_state.data.msize].linear &&
                instruction_state.data.wwLinear < instruction_state.data.mem[instruction_state.data.msize].linear + byte)
            {
                STD_PRINTF("Watch point caught at L%08x: WRITE %01x BYTES OF DATA=%08x TO L%08x\n", instruction_state.data.linear,
                           instruction_state.data.mem[instruction_state.data.msize].byte,
                           instruction_state.data.mem[instruction_state.data.msize].data,
                           instruction_state.data.mem[instruction_state.data.msize].linear);
            }
        }
        /* for (i = 0;i < instruction_state.data.msize;++i) {
            if (instruction_state.data.mem[i].flagWrite == instruction_state.data.mem[instruction_state.data.msize].flagWrite &&
                instruction_state.data.mem[i].linear == instruction_state.data.mem[instruction_state.data.msize].linear) {
                TYPE_TRACE_BLOCK_BEGIN("mem(same)");
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                TYPE_TRACE_CALL_END;
            }
        } */
        instruction_state.data.msize++;
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
/* test logical accessing */
static C_VOID _kma_test_linear(core_machine_cpu_execution_context *context, type_unsigned_32 linear, type_unsigned_8 byte, type_bool write, type_unsigned_8 vpl, type_bool force)
{
    type_unsigned_32 phy1, phy2;
    type_unsigned_8 byte1, byte2;
    TYPE_TRACE_CALL_BEGIN("_kma_test_linear");
    if (_GetLinear_Offset(linear) > TYPE_MASK_UNSIGNED_32(_GetPageSize - byte))
    {
        TYPE_TRACE_BLOCK_BEGIN("Linear_Offset(>PageSize)");
        byte1 = _GetPageSize - _GetLinear_Offset(linear);
        byte2 = byte - byte1;
        {
            t_kma_linear_translation translation1, translation2;

            TYPE_TRACE_CHECK_RETURN(_kma_prepare_physical_linear(context,
                linear, byte1, write, vpl, &translation1));
            TYPE_TRACE_CHECK_RETURN(_kma_prepare_physical_linear(context,
                linear + byte1, byte2, write, vpl, &translation2));
            TYPE_TRACE_CHECK_RETURN(_kma_commit_physical_linear(context,
                &translation1, write));
            TYPE_TRACE_CHECK_RETURN(_kma_commit_physical_linear(context,
                &translation2, write));
            phy1 = translation1.physical;
            phy2 = translation2.physical;
        }
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("Linear_Offset(<=PageSize)");
        byte1 = byte;
        TYPE_TRACE_CHECK_RETURN(phy1 = _kma_physical_linear(context, linear, byte1, write, vpl));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _kma_test_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, type_unsigned_32 offset, type_unsigned_8 byte, type_bool write, type_unsigned_8 vpl, type_bool force)
{
    type_unsigned_32 linear;
    TYPE_TRACE_CALL_BEGIN("_kma_test_logical");
    TYPE_TRACE_CHECK_RETURN(linear = _kma_linear_logical(context, rsreg, offset, byte, write, vpl, force));
    TYPE_TRACE_CALL_END;
}
static C_VOID _kma_test_access(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, type_unsigned_32 offset, type_unsigned_8 byte, type_bool write, type_unsigned_8 vpl, type_bool force)
{
    type_unsigned_32 linear;
    TYPE_TRACE_CALL_BEGIN("_kma_test_access");
    TYPE_TRACE_CHECK_RETURN(linear = _kma_linear_logical(context, rsreg, offset, byte, write, vpl, force));
    TYPE_TRACE_CHECK_RETURN(_kma_test_linear(context, linear, byte, write, vpl, force));
    TYPE_TRACE_CALL_END;
}

/* general memory accessing */
static C_VOID _m_read_ref(core_machine_cpu_execution_context *context, type_virtual_address ref, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_m_read_ref");
    /* _m_write_ref allows in-module reads only */
    if (ref < (type_virtual_address)(&cpu_state) && ref >= (type_virtual_address)(&cpu_state) + sizeof(t_cpu) &&
        ref < (type_virtual_address)(&instruction_state) && ref >= (type_virtual_address)(&instruction_state) + sizeof(t_cpuins))
    {
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    }
    TYPE_TRACE_CHECK_RETURN(_kma_read_ref(context, ref, rdata, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _m_write_ref(core_machine_cpu_execution_context *context, type_virtual_address ref, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_m_write_ref");
    /* _m_write_ref allows in-module writes only */
    if (ref < (type_virtual_address)(&cpu_state) && ref >= (type_virtual_address)(&cpu_state) + sizeof(t_cpu) &&
        ref < (type_virtual_address)(&instruction_state) && ref >= (type_virtual_address)(&instruction_state) + sizeof(t_cpuins))
    {
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    }
    TYPE_TRACE_CHECK_RETURN(_kma_write_ref(context, ref, rdata, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _m_read_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_m_read_logical");
    TYPE_TRACE_CHECK_RETURN(_kma_read_logical(context, rsreg, offset, rdata, byte, _GetCPL, 0));
    TYPE_TRACE_CALL_END;
}
static C_VOID _m_write_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_m_write_logical");
    TYPE_TRACE_CHECK_RETURN(_kma_write_logical(context, rsreg, offset, rdata, byte, _GetCPL, 0));
    TYPE_TRACE_CALL_END;
}
static C_VOID _m_test_access(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, type_unsigned_32 offset, type_unsigned_8 byte, type_bool write)
{
    type_virtual_address ref = 0;
    TYPE_TRACE_CALL_BEGIN("_m_test_access");
    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, rsreg, offset, byte, write, _GetCPL, 0));
    TYPE_TRACE_CALL_END;
}
static C_VOID _m_test_logical(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, type_unsigned_32 offset, type_unsigned_8 byte, type_bool write)
{
    type_virtual_address ref = 0;
    TYPE_TRACE_CALL_BEGIN("_m_test_logical");
    TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, rsreg, offset, byte, write, _GetCPL, 0));
    TYPE_TRACE_CALL_END;
}

static C_VOID _m_read_rm(core_machine_cpu_execution_context *context, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_m_read_rm");
    instruction_state.data.crm = 0;
    if (instruction_state.data.flagMem)
        TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.mrm.rsreg, instruction_state.data.mrm.offset, TYPE_REFERENCE_OF(instruction_state.data.crm), byte));
    else
        TYPE_TRACE_CHECK_RETURN(_m_read_ref(context, instruction_state.data.rrm, TYPE_REFERENCE_OF(instruction_state.data.crm), byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _m_write_rm(core_machine_cpu_execution_context *context, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_m_write_rm");
    if (instruction_state.data.flagMem)
        TYPE_TRACE_CHECK_RETURN(_m_write_logical(context, instruction_state.data.mrm.rsreg, instruction_state.data.mrm.offset, TYPE_REFERENCE_OF(instruction_state.data.crm), byte));
    else
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rrm, TYPE_REFERENCE_OF(instruction_state.data.crm), byte));
    TYPE_TRACE_CALL_END;
}

/* segment accessing unit: _s_ */
/* kernel segment accessing */
static C_VOID _ksa_read_idt(core_machine_cpu_execution_context *context, type_unsigned_8 intid, type_virtual_address rdata)
{
    TYPE_TRACE_CALL_BEGIN("_s_read_idt");
    if (!_GetCR0_PE)
    {
        TYPE_TRACE_BLOCK_BEGIN("CR0_PE(0)");
        if (TYPE_MASK_UNSIGNED_16(intid * 4 + 3) > TYPE_MASK_UNSIGNED_16(cpu_state.data.idtr.limit))
            TYPE_TRACE_IMPOSSIBLE_RETURN;
        TYPE_TRACE_CHECK_RETURN(_kma_read_logical(context, &cpu_state.data.idtr, (intid * 4), rdata, 4, 0x00, 0));
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("CR0_PE(1)");
        if (TYPE_MASK_UNSIGNED_16(intid * 8 + 7) > TYPE_MASK_UNSIGNED_16(cpu_state.data.idtr.limit))
            TYPE_TRACE_IMPOSSIBLE_RETURN;
        TYPE_TRACE_CHECK_RETURN(_kma_read_logical(context, &cpu_state.data.idtr, (intid * 8), rdata, 8, 0x00, 0));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _ksa_read_ldt(core_machine_cpu_execution_context *context, type_unsigned_16 selector, type_virtual_address rdata)
{
    TYPE_TRACE_CALL_BEGIN("_ksa_read_ldt");
    if (!_GetCR0_PE)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (!_GetSelector_TI(selector))
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (TYPE_MASK_UNSIGNED_16(_GetSelector_Offset(selector) + 7) > TYPE_MASK_UNSIGNED_16(cpu_state.data.ldtr.limit))
    {
        TYPE_TRACE_BLOCK_BEGIN("Selector_Offset(>ldtr.limit)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CHECK_RETURN(_kma_read_logical(context, &cpu_state.data.ldtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID _ksa_read_gdt(core_machine_cpu_execution_context *context, type_unsigned_16 selector, type_virtual_address rdata)
{
    TYPE_TRACE_CALL_BEGIN("_ksa_read_gdt");
    if (!_GetCR0_PE)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetSelector_TI(selector))
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (TYPE_MASK_UNSIGNED_16(_GetSelector_Offset(selector) + 7) > TYPE_MASK_UNSIGNED_16(cpu_state.data.gdtr.limit))
    {
        TYPE_TRACE_BLOCK_BEGIN("Selector_Offset(>gdtr.limit)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CHECK_RETURN(_kma_read_logical(context, &cpu_state.data.gdtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID _ksa_read_xdt(core_machine_cpu_execution_context *context, type_unsigned_16 selector, type_virtual_address rdata)
{
    TYPE_TRACE_CALL_BEGIN("_ksa_read_xdt");
    if (!_GetCR0_PE)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetSelector_TI(selector))
    {
        TYPE_TRACE_BLOCK_BEGIN("Selector_TI");
        TYPE_TRACE_CHECK_RETURN(_ksa_read_ldt(context, selector, rdata));
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("!Selector_TI");
        TYPE_TRACE_CHECK_RETURN(_ksa_read_gdt(context, selector, rdata));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _ksa_write_ldt(core_machine_cpu_execution_context *context, type_unsigned_16 selector, type_virtual_address rdata)
{
    TYPE_TRACE_CALL_BEGIN("_ksa_write_ldt");
    if (!_GetCR0_PE)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (!_GetSelector_TI(selector))
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (TYPE_MASK_UNSIGNED_16(_GetSelector_Offset(selector) + 7) > TYPE_MASK_UNSIGNED_16(cpu_state.data.ldtr.limit))
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CHECK_RETURN(_kma_write_logical(context, &cpu_state.data.ldtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID _ksa_write_gdt(core_machine_cpu_execution_context *context, type_unsigned_16 selector, type_virtual_address rdata)
{
    TYPE_TRACE_CALL_BEGIN("_ksa_write_gdt");
    if (!_GetCR0_PE)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetSelector_TI(selector))
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (TYPE_MASK_UNSIGNED_16(_GetSelector_Offset(selector) + 7) > TYPE_MASK_UNSIGNED_16(cpu_state.data.gdtr.limit))
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CHECK_RETURN(_kma_write_logical(context, &cpu_state.data.gdtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID _ksa_write_xdt(core_machine_cpu_execution_context *context, type_unsigned_16 selector, type_virtual_address rdata)
{
    TYPE_TRACE_CALL_BEGIN("_ksa_write_xdt");
    if (!_GetCR0_PE)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetSelector_TI(selector))
    {
        TYPE_TRACE_BLOCK_BEGIN("Selector_TI");
        TYPE_TRACE_CHECK_RETURN(_ksa_write_ldt(context, selector, rdata));
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("!Selector_TI");
        TYPE_TRACE_CHECK_RETURN(_ksa_write_gdt(context, selector, rdata));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _ksa_test_write_xdt(core_machine_cpu_execution_context *context,
    type_unsigned_16 selector)
{
    TYPE_TRACE_CALL_BEGIN("_ksa_test_write_xdt");
    if (!_GetCR0_PE)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetSelector_TI(selector))
    {
        if (TYPE_MASK_UNSIGNED_16(_GetSelector_Offset(selector) + 7) >
            TYPE_MASK_UNSIGNED_16(cpu_state.data.ldtr.limit))
            TYPE_TRACE_IMPOSSIBLE_RETURN;
        TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &cpu_state.data.ldtr,
            _GetSelector_Offset(selector), 8, 1, 0x00, 1));
    }
    else
    {
        if (TYPE_MASK_UNSIGNED_16(_GetSelector_Offset(selector) + 7) >
            TYPE_MASK_UNSIGNED_16(cpu_state.data.gdtr.limit))
            TYPE_TRACE_IMPOSSIBLE_RETURN;
        TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &cpu_state.data.gdtr,
            _GetSelector_Offset(selector), 8, 1, 0x00, 1));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _ksa_prepare_code_sreg(core_machine_cpu_execution_context *context,
    type_unsigned_16 selector, type_unsigned_8 cpl, t_cpu_data_sreg *rsreg,
    type_unsigned_64 *rdescriptor)
{
    type_unsigned_64 descriptor;

    TYPE_TRACE_CALL_BEGIN("_ksa_prepare_code_sreg");
    TYPE_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector,
        TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescCode(descriptor))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector & 0xfffcu));
    if (!_IsDescPresent(descriptor))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(selector & 0xfffcu));
    _SetDescUserAccessed(descriptor);
    TYPE_TRACE_CHECK_RETURN(_ksa_test_write_xdt(context, selector));
    rsreg->flagValid = TYPE_TRUE;
    rsreg->selector = (selector & ~VCPU_SELECTOR_RPL) | cpl;
    rsreg->sregtype = SREG_CODE;
    rsreg->base = (type_unsigned_32)_GetDescSeg_Base(descriptor);
    rsreg->dpl = (type_unsigned_4)_GetDesc_DPL(descriptor);
    rsreg->limit = (type_unsigned_32)(_IsDescSegGranularLarge(descriptor) ?
        ((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) :
        _GetDescSeg_Limit(descriptor));
    rsreg->seg.accessed = (type_bool)_IsDescUserAccessed(descriptor);
    rsreg->seg.executable = (type_bool)_IsDescUserExecutable(descriptor);
    rsreg->seg.exec.defsize = (type_bool)_IsDescCode32(descriptor);
    rsreg->seg.exec.conform = (type_bool)_IsDescCodeConform(descriptor);
    rsreg->seg.exec.readable = (type_bool)_IsDescCodeReadable(descriptor);
    *rdescriptor = descriptor;
    TYPE_TRACE_CALL_END;
}
static C_VOID _ksa_prepare_stack_sreg(core_machine_cpu_execution_context *context,
    type_unsigned_16 selector, type_unsigned_8 cpl, t_cpu_data_sreg *rsreg,
    type_unsigned_64 *rdescriptor)
{
    type_unsigned_64 descriptor;

    TYPE_TRACE_CALL_BEGIN("_ksa_prepare_stack_sreg");
    TYPE_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector,
        TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescDataWritable(descriptor) || _GetSelector_RPL(selector) != cpl ||
        _GetDesc_DPL(descriptor) != cpl)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector & 0xfffcu));
    if (!_IsDescPresent(descriptor))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_SS(selector & 0xfffcu));
    _SetDescUserAccessed(descriptor);
    TYPE_TRACE_CHECK_RETURN(_ksa_test_write_xdt(context, selector));
    rsreg->flagValid = TYPE_TRUE;
    rsreg->selector = selector;
    rsreg->sregtype = SREG_STACK;
    rsreg->base = (type_unsigned_32)_GetDescSeg_Base(descriptor);
    rsreg->dpl = (type_unsigned_4)_GetDesc_DPL(descriptor);
    rsreg->limit = (type_unsigned_32)(_IsDescSegGranularLarge(descriptor) ?
        ((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) :
        _GetDescSeg_Limit(descriptor));
    rsreg->seg.accessed = (type_bool)_IsDescUserAccessed(descriptor);
    rsreg->seg.executable = TYPE_FALSE;
    rsreg->seg.data.big = (type_bool)_IsDescDataBig(descriptor);
    rsreg->seg.data.expdown = (type_bool)_IsDescDataExpDown(descriptor);
    rsreg->seg.data.writable = (type_bool)_IsDescDataWritable(descriptor);
    *rdescriptor = descriptor;
    TYPE_TRACE_CALL_END;
}
static C_VOID _ksa_load_sreg(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, type_unsigned_16 selector)
{
    type_unsigned_64 descriptor;
    TYPE_TRACE_CALL_BEGIN("_ksa_load_sreg");
    switch (rsreg->sregtype)
    {
    case SREG_CODE:
        /* note: privilege checking not performed */
        TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_CODE)");
        if (_IsProtected)
        {
            TYPE_TRACE_BLOCK_BEGIN("Protected(1)");
            if (_IsSelectorNull(selector))
            {
                TYPE_TRACE_BLOCK_BEGIN("selector(null)");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector, TYPE_REFERENCE_OF(descriptor)));
            if (!_IsDescCode(descriptor))
            {
                TYPE_TRACE_BLOCK_BEGIN("!DescCode");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
                TYPE_TRACE_BLOCK_END;
            }
            if (!_IsDescPresent(descriptor))
            {
                TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(selector));
                TYPE_TRACE_BLOCK_END;
            }
            _SetDescUserAccessed(descriptor);
            TYPE_TRACE_CHECK_RETURN(_ksa_write_xdt(context, selector, TYPE_REFERENCE_OF(descriptor)));
            rsreg->flagValid = TYPE_TRUE;
            rsreg->base = (type_unsigned_32)_GetDescSeg_Base(descriptor);
            if (_IsDescCodeNonConform(descriptor))
                rsreg->dpl = (type_unsigned_4)_GetDesc_DPL(descriptor);
            rsreg->limit = (type_unsigned_32)((_IsDescSegGranularLarge(descriptor) ? ((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
            rsreg->seg.accessed = (type_bool)_IsDescUserAccessed(descriptor);
            rsreg->seg.executable = (type_bool)_IsDescUserExecutable(descriptor);
            rsreg->seg.exec.defsize = (type_bool)_IsDescCode32(descriptor);
            rsreg->seg.exec.conform = (type_bool)_IsDescCodeConform(descriptor);
            rsreg->seg.exec.readable = (type_bool)_IsDescCodeReadable(descriptor);
            rsreg->selector = (selector & ~VCPU_SELECTOR_RPL) | _GetCPL;
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("!Protected");
            rsreg->flagValid = TYPE_TRUE;
            rsreg->base = (selector << 4);
            rsreg->selector = selector;
            if (_GetCR0_PE && _GetEFLAGS_VM)
            {
                rsreg->dpl = 0x03;
                rsreg->limit = 0x0000ffff;
            }
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case SREG_DATA:
        TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_DATA)");
        if (_IsProtected)
        {
            TYPE_TRACE_BLOCK_BEGIN("Protected");
            if (_IsSelectorNull(selector))
            {
                TYPE_TRACE_BLOCK_BEGIN("selector(null)");
                rsreg->flagValid = TYPE_FALSE;
                rsreg->selector = selector;
                TYPE_TRACE_BLOCK_END;
            }
            else
            {
                TYPE_TRACE_BLOCK_BEGIN("selector(!null)");
                TYPE_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector, TYPE_REFERENCE_OF(descriptor)));
                if (!_IsDescData(descriptor) && !_IsDescCodeReadable(descriptor))
                {
                    TYPE_TRACE_BLOCK_BEGIN("!DescData,!DescCodeReadable");
                    TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
                    TYPE_TRACE_BLOCK_END;
                }
                if (_IsDescData(descriptor) || _IsDescCodeNonConform(descriptor))
                {
                    TYPE_TRACE_BLOCK_BEGIN("DescData/DescCodeNonConform)");
                    if (_GetSelector_RPL(selector) > _GetDesc_DPL(descriptor) ||
                        _GetCPL > _GetDesc_DPL(descriptor))
                    {
                        TYPE_TRACE_BLOCK_BEGIN("PL(fail)");
                        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
                        TYPE_TRACE_BLOCK_END;
                    }
                    TYPE_TRACE_BLOCK_END;
                }
                if (!_IsDescPresent(descriptor))
                {
                    TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
                    TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(selector));
                    TYPE_TRACE_BLOCK_END;
                }
                _SetDescUserAccessed(descriptor);
                TYPE_TRACE_CHECK_RETURN(_ksa_write_xdt(context, selector, TYPE_REFERENCE_OF(descriptor)));
                rsreg->flagValid = TYPE_TRUE;
                rsreg->selector = selector;
                rsreg->base = (type_unsigned_32)_GetDescSeg_Base(descriptor);
                rsreg->dpl = (type_unsigned_4)_GetDesc_DPL(descriptor);
                rsreg->limit = (type_unsigned_32)((_IsDescSegGranularLarge(descriptor) ? ((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
                rsreg->seg.accessed = (type_bool)_IsDescUserAccessed(descriptor);
                rsreg->seg.executable = (type_bool)_IsDescUserExecutable(descriptor);
                if (rsreg->seg.executable)
                {
                    rsreg->seg.exec.defsize = (type_bool)_IsDescCode32(descriptor);
                    rsreg->seg.exec.conform = (type_bool)_IsDescCodeConform(descriptor);
                    rsreg->seg.exec.readable = (type_bool)_IsDescCodeReadable(descriptor);
                }
                else
                {
                    rsreg->seg.data.big = (type_bool)_IsDescDataBig(descriptor);
                    rsreg->seg.data.expdown = (type_bool)_IsDescDataExpDown(descriptor);
                    rsreg->seg.data.writable = (type_bool)_IsDescDataWritable(descriptor);
                }
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("!Protected");
            rsreg->flagValid = TYPE_TRUE;
            rsreg->selector = selector;
            rsreg->base = (selector << 4);
            if (_GetCR0_PE && _GetEFLAGS_VM)
            {
                rsreg->dpl = 0x03;
                rsreg->limit = 0x0000ffff;
            }
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case SREG_STACK:
        TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_STACK)");
        if (_IsProtected)
        {
            TYPE_TRACE_BLOCK_BEGIN("Protected(1)");
            if (_IsSelectorNull(selector))
            {
                TYPE_TRACE_BLOCK_BEGIN("selector(null)");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                TYPE_TRACE_BLOCK_END;
            }
            if (_GetSelector_RPL(selector) != _GetCPL)
            {
                TYPE_TRACE_BLOCK_BEGIN("Selector_RPL(!CPL)");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector, TYPE_REFERENCE_OF(descriptor)));
            if (!_IsDescDataWritable(descriptor))
            {
                TYPE_TRACE_BLOCK_BEGIN("!DescDataWritable");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
                TYPE_TRACE_BLOCK_END;
            }
            if (_GetDesc_DPL(descriptor) != _GetCPL)
            {
                TYPE_TRACE_BLOCK_BEGIN("Desc_DPL(!CPL)");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
                TYPE_TRACE_BLOCK_END;
            }
            if (!_IsDescPresent(descriptor))
            {
                TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_SS(selector));
                TYPE_TRACE_BLOCK_END;
            }
            _SetDescUserAccessed(descriptor);
            TYPE_TRACE_CHECK_RETURN(_ksa_write_xdt(context, selector, TYPE_REFERENCE_OF(descriptor)));
            rsreg->flagValid = TYPE_TRUE;
            rsreg->selector = selector;
            rsreg->base = (type_unsigned_32)_GetDescSeg_Base(descriptor);
            rsreg->dpl = (type_unsigned_4)_GetDesc_DPL(descriptor);
            rsreg->limit = (type_unsigned_32)((_IsDescSegGranularLarge(descriptor) ? ((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
            rsreg->seg.accessed = (type_bool)_IsDescUserAccessed(descriptor);
            rsreg->seg.executable = (type_bool)_IsDescUserExecutable(descriptor);
            rsreg->seg.data.big = (type_bool)_IsDescDataBig(descriptor);
            rsreg->seg.data.expdown = (type_bool)_IsDescDataExpDown(descriptor);
            rsreg->seg.data.writable = (type_bool)_IsDescDataWritable(descriptor);
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("!Protected");
            rsreg->flagValid = TYPE_TRUE;
            rsreg->selector = selector;
            rsreg->base = (selector << 4);
            if (_GetCR0_PE && _GetEFLAGS_VM)
            {
                rsreg->dpl = 0x03;
                rsreg->limit = 0x0000ffff;
            }
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case SREG_TR:
        TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_TR)");
        if (!_IsProtected)
            TYPE_TRACE_IMPOSSIBLE_RETURN;
        if (_GetCPL)
            TYPE_TRACE_IMPOSSIBLE_RETURN;
        if (_GetSelector_TI(selector))
            TYPE_TRACE_IMPOSSIBLE_RETURN;
        if (_IsSelectorNull(selector))
        {
            TYPE_TRACE_BLOCK_BEGIN("selector(null)");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector, TYPE_REFERENCE_OF(descriptor)));
        if (!_IsDescTSSAvl(descriptor))
        {
            TYPE_TRACE_BLOCK_BEGIN("!DescTssAvl");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
            TYPE_TRACE_BLOCK_END;
        }
        if (!_IsDescPresent(descriptor))
        {
            TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(selector));
            TYPE_TRACE_BLOCK_END;
        }
        _SetDescTSSBusy(descriptor);
        TYPE_TRACE_CHECK_RETURN(_ksa_write_xdt(context, selector, TYPE_REFERENCE_OF(descriptor)));
        rsreg->flagValid = TYPE_TRUE;
        rsreg->selector = selector;
        rsreg->base = (type_unsigned_32)_GetDescSeg_Base(descriptor);
        rsreg->dpl = (type_unsigned_4)_GetDesc_DPL(descriptor);
        rsreg->limit = (type_unsigned_32)((_IsDescSegGranularLarge(descriptor) ? (_GetDescSeg_Limit(descriptor) << 12 | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
        rsreg->sys.type = (type_unsigned_4)_GetDesc_Type(descriptor);
        TYPE_TRACE_BLOCK_END;
        break;
    case SREG_LDTR:
        TYPE_TRACE_BLOCK_BEGIN("sregtype(SREG_LDTR)");
        if (!_IsProtected)
            TYPE_TRACE_IMPOSSIBLE_RETURN;
        if (_GetCPL)
            TYPE_TRACE_IMPOSSIBLE_RETURN;
        if (_GetSelector_TI(selector))
            TYPE_TRACE_IMPOSSIBLE_RETURN;
        if (_IsSelectorNull(selector))
        {
            TYPE_TRACE_BLOCK_BEGIN("selector(null)");
            rsreg->flagValid = TYPE_FALSE;
            rsreg->selector = selector;
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("selector(!null)");
            TYPE_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector, TYPE_REFERENCE_OF(descriptor)));
            if (!_IsDescLDT(descriptor))
            {
                TYPE_TRACE_BLOCK_BEGIN("descriptor(!LDT)");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
                TYPE_TRACE_BLOCK_END;
            }
            if (!_IsDescPresent(descriptor))
            {
                TYPE_TRACE_BLOCK_BEGIN("descriptor(!P)");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(selector));
                TYPE_TRACE_BLOCK_END;
            }
            rsreg->flagValid = TYPE_TRUE;
            rsreg->selector = selector;
            rsreg->base = (type_unsigned_32)_GetDescSeg_Base(descriptor);
            rsreg->dpl = (type_unsigned_4)_GetDesc_DPL(descriptor);
            rsreg->limit = (type_unsigned_32)((_IsDescSegGranularLarge(descriptor) ? (_GetDescSeg_Limit(descriptor) << 12 | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
            rsreg->sys.type = (type_unsigned_4)_GetDesc_Type(descriptor);
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case SREG_GDTR:
    case SREG_IDTR:
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}

/* regular segment accessing */
static type_bool _s_check_selector(core_machine_cpu_execution_context *context, type_unsigned_16 selector)
{
    /* 0 = succ, 1 = fail */
    TYPE_TRACE_CALL_BEGIN("_s_check_selector");
    if (_IsSelectorNull(selector))
    {
        TYPE_TRACE_CALL_END;
        return TYPE_TRUE;
    }
    if (TYPE_MASK_UNSIGNED_32(_GetSelector_Offset(selector) + 7) >
        (_GetSelector_TI(selector) ? TYPE_MASK_UNSIGNED_32(cpu_state.data.ldtr.limit) : TYPE_MASK_UNSIGNED_32(cpu_state.data.gdtr.limit)))
    {
        TYPE_TRACE_CALL_END;
        return TYPE_TRUE;
    }
    TYPE_TRACE_CALL_END;
    return TYPE_FALSE;
}
static C_VOID _s_read_idt(core_machine_cpu_execution_context *context, type_unsigned_8 intid, type_virtual_address rdata)
{
    TYPE_TRACE_CALL_BEGIN("_s_read_idt");
    TYPE_TRACE_CHECK_RETURN(_ksa_read_idt(context, intid, rdata));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_read_xdt(core_machine_cpu_execution_context *context, type_unsigned_16 selector, type_virtual_address rdata)
{
    TYPE_TRACE_CALL_BEGIN("_s_read_xdt");
    if (!_GetCR0_PE)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector, rdata));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_read_tss(core_machine_cpu_execution_context *context, type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_s_read_tss");
    TYPE_TRACE_CHECK_RETURN(_kma_read_logical(context, &cpu_state.data.tr, offset, rdata, byte, 0, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_read_es(core_machine_cpu_execution_context *context, type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_s_read_es");
    TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, &cpu_state.data.es, offset, rdata, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_read_cs(core_machine_cpu_execution_context *context, type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte)
{
    core_machine_cpu_memory_access_provenance previous =
        context->memory_access_provenance;
    type_unsigned_32 linear = cpu_state.data.cs.base + offset;
    type_unsigned_32 prefetch_offset;
    TYPE_TRACE_CALL_BEGIN("_s_read_cs");
    if (context->prefetch_valid && linear >= context->prefetch_linear &&
        (prefetch_offset = linear - context->prefetch_linear) <=
            context->prefetch_count && byte <= context->prefetch_count -
            prefetch_offset) {
        STD_MEMCPY((C_VOID *)rdata, context->prefetch_bytes + prefetch_offset,
            byte);
    } else {
        context->memory_access_provenance = CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH;
        _kma_read_logical(context, &cpu_state.data.cs, offset, rdata, byte,
            _GetCPL, 1);
    }
    context->memory_access_provenance = previous;
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_read_ss(core_machine_cpu_execution_context *context, type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_s_read_ss");
    TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, &cpu_state.data.ss, offset, rdata, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_read_ds(core_machine_cpu_execution_context *context, type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_s_read_ds");
    TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_read_fs(core_machine_cpu_execution_context *context, type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_s_read_fs");
    TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_read_gs(core_machine_cpu_execution_context *context, type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_s_read_gs");
    TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_idt(core_machine_cpu_execution_context *context, type_unsigned_8 intid, type_virtual_address rdata)
{
    TYPE_TRACE_CALL_BEGIN("_s_write_idt");
    TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_xdt(core_machine_cpu_execution_context *context, type_unsigned_16 selector, type_virtual_address rdata)
{
    TYPE_TRACE_CALL_BEGIN("_s_write_xdt");
    if (!_GetCR0_PE)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CHECK_RETURN(_ksa_write_xdt(context, selector, rdata));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_tss(core_machine_cpu_execution_context *context, type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_s_write_tss");
    TYPE_TRACE_CHECK_RETURN(_kma_write_logical(context, &cpu_state.data.tr,
        offset, rdata, byte, 0, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_es(core_machine_cpu_execution_context *context, type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_s_write_es");
    TYPE_TRACE_CHECK_RETURN(_m_write_logical(context, &cpu_state.data.es, offset, rdata, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_cs(core_machine_cpu_execution_context *context, type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_s_write_cs");
    TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_ss(core_machine_cpu_execution_context *context, type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_s_write_ss");
    TYPE_TRACE_CHECK_RETURN(_m_write_logical(context, &cpu_state.data.ss, offset, rdata, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_ds(core_machine_cpu_execution_context *context, type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_s_write_ds");
    TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_fs(core_machine_cpu_execution_context *context, type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_s_write_fs");
    TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_gs(core_machine_cpu_execution_context *context, type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_s_write_gs");
    TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_test_tss(core_machine_cpu_execution_context *context, type_unsigned_32 offset, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_s_test_cs");
    TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &cpu_state.data.tr, offset, byte, 0, 0x00, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_test_cs(core_machine_cpu_execution_context *context, type_unsigned_32 offset, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_s_test_cs");
    TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &cpu_state.data.cs, offset, byte, 0, 0x00, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_test_ss_push(core_machine_cpu_execution_context *context, type_unsigned_8 byte)
{
    type_unsigned_32 cesp = 0x00000000;
    TYPE_TRACE_CALL_BEGIN("_s_test_ss_push");
    switch (_GetStackSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("StackSize(2)");
        if (cpu_state.data.sp && cpu_state.data.sp < byte)
            TYPE_TRACE_CHECK_RETURN(_SetExcept_SS(0));
        TYPE_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.ss, TYPE_MASK_UNSIGNED_16(cpu_state.data.sp - byte), byte, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("StackSize(4)");
        if (cpu_state.data.esp && cpu_state.data.esp < byte)
            TYPE_TRACE_CHECK_RETURN(_SetExcept_SS(0));
        TYPE_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.ss, TYPE_MASK_UNSIGNED_32(cpu_state.data.esp - byte), byte, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_test_stack_frame_16(core_machine_cpu_execution_context *context,
    t_cpu_data_sreg *stack, type_unsigned_16 sp, type_unsigned_8 words,
    type_unsigned_8 cpl)
{
    type_unsigned_8 index;

    TYPE_TRACE_CALL_BEGIN("_s_test_stack_frame_16");
    for (index = 0; index < words; ++index)
    {
        if (sp && sp < 2u)
            TYPE_TRACE_CHECK_RETURN(_SetExcept_SS(0));
        sp = TYPE_MASK_UNSIGNED_16(sp - 2u);
        TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, stack, sp, 2u, 1,
            cpl, 1));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_test_stack_frame_32(core_machine_cpu_execution_context *context,
    t_cpu_data_sreg *stack, type_unsigned_32 esp, type_unsigned_8 dwords,
    type_unsigned_8 cpl)
{
    type_unsigned_8 index;

    TYPE_TRACE_CALL_BEGIN("_s_test_stack_frame_32");
    for (index = 0u; index < dwords; ++index)
    {
        if (stack->seg.data.big) {
            if (esp && esp < 4u)
                TYPE_TRACE_CHECK_RETURN(_SetExcept_SS(0));
            esp -= 4u;
        }
        else {
            type_unsigned_16 sp = TYPE_MASK_UNSIGNED_16(esp);

            if (sp && sp < 4u)
                TYPE_TRACE_CHECK_RETURN(_SetExcept_SS(0));
            esp = TYPE_MASK_UNSIGNED_16(sp - 4u);
        }
        TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, stack, esp, 4u, 1,
            cpl, 1));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_test_ss_pop(core_machine_cpu_execution_context *context, type_unsigned_8 byte)
{
    type_unsigned_32 cesp = 0x00000000;
    TYPE_TRACE_CALL_BEGIN("_s_test_ss_pop");
    switch (_GetStackSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("StackSize(2)");
        TYPE_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.ss, cpu_state.data.sp, byte, 0));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("StackSize(4)");
        TYPE_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.ss, cpu_state.data.esp, byte, 0));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_peek_ss_pop(core_machine_cpu_execution_context *context,
    type_unsigned_32 offset, type_virtual_address rdata, type_unsigned_8 byte)
{
    type_unsigned_32 cesp;

    TYPE_TRACE_CALL_BEGIN("_s_peek_ss_pop");
    switch (_GetStackSize)
    {
    case 2:
        cesp = TYPE_MASK_UNSIGNED_16(cpu_state.data.sp + offset);
        break;
    case 4:
        cesp = cpu_state.data.esp + offset;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CHECK_RETURN(_s_read_ss(context, cesp, rdata, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_sreg(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, type_unsigned_16 selector)
{
    TYPE_TRACE_CALL_BEGIN("_s_load_sreg");
    TYPE_TRACE_CHECK_RETURN(_ksa_load_sreg(context, rsreg, selector));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_gdtr(core_machine_cpu_execution_context *context, type_unsigned_32 base, type_unsigned_16 limit, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_s_load_gdtr");
    if (_GetCPL)
    {
        TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        TYPE_TRACE_BLOCK_END;
    }
    cpu_state.data.gdtr.limit = limit;
    switch (byte)
    {
    case 2:
        cpu_state.data.gdtr.base = TYPE_MASK_UNSIGNED_24(base);
        break;
    case 4:
        cpu_state.data.gdtr.base = TYPE_MASK_UNSIGNED_32(base);
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("byte");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_idtr(core_machine_cpu_execution_context *context, type_unsigned_32 base, type_unsigned_16 limit, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_s_load_idtr");
    if (_GetCPL)
    {
        TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        TYPE_TRACE_BLOCK_END;
    }
    cpu_state.data.idtr.limit = limit;
    switch (byte)
    {
    case 2:
        cpu_state.data.idtr.base = TYPE_MASK_UNSIGNED_24(base);
        break;
    case 4:
        cpu_state.data.idtr.base = TYPE_MASK_UNSIGNED_32(base);
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("byte");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_ldtr(core_machine_cpu_execution_context *context, type_unsigned_16 selector)
{
    TYPE_TRACE_CALL_BEGIN("_s_load_ldtr");
    if (_GetCPL)
    {
        TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        TYPE_TRACE_BLOCK_END;
    }
    if (_GetSelector_TI(selector))
    {
        TYPE_TRACE_BLOCK_BEGIN("Selector_TI(1)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.ldtr, selector));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_tr(core_machine_cpu_execution_context *context, type_unsigned_16 selector)
{
    TYPE_TRACE_CALL_BEGIN("_s_load_tr");
    if (_GetCPL)
    {
        TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        TYPE_TRACE_BLOCK_END;
    }
    if (_GetSelector_TI(selector))
    {
        TYPE_TRACE_BLOCK_BEGIN("Selector_TI(1)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(selector));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.tr, selector));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_cr0_msw(core_machine_cpu_execution_context *context, type_unsigned_16 msw)
{
    TYPE_TRACE_CALL_BEGIN("_s_load_cr0_msw");
    if (_GetCPL)
    {
        TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        TYPE_TRACE_BLOCK_END;
    }
    if (!_GetCR0_PE)
    {
        TYPE_TRACE_BLOCK_BEGIN("CR0_PE(0)");
        cpu_state.data.cr0 = (cpu_state.data.cr0 & 0xfffffff0) | (msw & 0x000f);
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("CR0_PE(1)");
        cpu_state.data.cr0 = (cpu_state.data.cr0 & 0xfffffff0) | (msw & 0x000e) | 0x01;
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_cr0_80386(core_machine_cpu_execution_context *context,
    type_unsigned_32 value)
{
    const type_unsigned_32 mutable_mask = VCPU_CR0_PE | VCPU_CR0_MP |
        VCPU_CR0_EM | VCPU_CR0_TS | VCPU_CR0_ET | VCPU_CR0_PG;
    const type_unsigned_32 retained_mask = ~mutable_mask;

    TYPE_TRACE_CALL_BEGIN("_s_write_cr0_80386");
    if ((value & retained_mask) != (cpu_state.data.cr0 & retained_mask))
    {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
    }
    if (TYPE_GET_BIT(value, VCPU_CR0_PG) &&
        !TYPE_GET_BIT(value, VCPU_CR0_PE))
    {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
    }
    /* On 80386 a privileged MOV CR0 may clear PE. The following far jump
     * supplies the real-address CS cache transition. A PG change also
     * invalidates bytes fetched through the former translation context. */
    if ((cpu_state.data.cr0 ^ value) & VCPU_CR0_PG) {
        core_machine_cpu_execution_invalidate_prefetch(context);
    }
    cpu_state.data.cr0 = value;
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_write_cr3_80386(core_machine_cpu_execution_context *context,
    type_unsigned_32 value)
{
    TYPE_TRACE_CALL_BEGIN("_s_write_cr3_80386");
    if (value & ~VCPU_CR3_BASE)
    {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
    }
    /* A new directory changes the translation context of queued bytes. */
    core_machine_cpu_execution_invalidate_prefetch(context);
    cpu_state.data.cr3 = value;
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_cs(core_machine_cpu_execution_context *context, type_unsigned_16 newcs)
{
    TYPE_TRACE_CALL_BEGIN("_s_load_cs");
    TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.cs, newcs));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_ss(core_machine_cpu_execution_context *context, type_unsigned_16 newss)
{
    TYPE_TRACE_CALL_BEGIN("_s_load_ss");
    TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.ss, newss));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_ds(core_machine_cpu_execution_context *context, type_unsigned_16 newds)
{
    TYPE_TRACE_CALL_BEGIN("_s_load_ds");
    TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.ds, newds));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_es(core_machine_cpu_execution_context *context, type_unsigned_16 newes)
{
    TYPE_TRACE_CALL_BEGIN("_s_load_es");
    TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.es, newes));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_fs(core_machine_cpu_execution_context *context, type_unsigned_16 newfs)
{
    TYPE_TRACE_CALL_BEGIN("_s_load_fs");
    TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.fs, newfs));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_load_gs(core_machine_cpu_execution_context *context, type_unsigned_16 newgs)
{
    TYPE_TRACE_CALL_BEGIN("_s_load_gs");
    TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.gs, newgs));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_test_eip(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("_s_test_eip");
    TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &cpu_state.data.cs,
        cpu_state.data.eip, 0x01, 0, _GetCPL, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_test_esp(core_machine_cpu_execution_context *context)
{
    type_unsigned_32 cesp;
    TYPE_TRACE_CALL_BEGIN("_s_test_esp");
    switch (_GetStackSize)
    {
    case 2:
        cesp = TYPE_MASK_UNSIGNED_16(cpu_state.data.esp);
        break;
    case 4:
        cesp = TYPE_MASK_UNSIGNED_32(cpu_state.data.esp);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CHECK_RETURN(_m_test_logical(context, &cpu_state.data.ss, cesp, 0x00, 0));
    TYPE_TRACE_CALL_END;
}

/* portid accessing unit */
/* kernel portid accessing */
_______todo _kpa_test_iomap(core_machine_cpu_execution_context *context, type_unsigned_16 portid, type_unsigned_8 byte)
{
    type_unsigned_16 iomap_base;
    type_unsigned_32 bitmap_offset;
    type_unsigned_32 checked_port;
    type_unsigned_8 bitmap_byte;

    TYPE_TRACE_CALL_BEGIN("_kpa_test_iomap");
    if (byte != 1u && byte != 2u && byte != 4u) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
    }
    if (context->cpu_profile < CORE_MACHINE_CPU_PROFILE_80386 ||
        !cpu_state.data.tr.flagValid ||
        cpu_state.data.tr.sys.type != VCPU_DESC_SYS_TYPE_TSS_32_BUSY) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
    }
    if (TYPE_MASK_UNSIGNED_32(portid) + byte > VPORT_MAX_PORT_COUNT) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
    }
    if (cpu_state.data.tr.limit < 0x67u) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
    }
    TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, 0x66u,
        TYPE_REFERENCE_OF(iomap_base), 2u));
    for (checked_port = portid; checked_port < TYPE_MASK_UNSIGNED_32(portid) +
            byte; ++checked_port) {
        bitmap_offset = TYPE_MASK_UNSIGNED_32(iomap_base) +
            (checked_port >> 3u);
        if (bitmap_offset > cpu_state.data.tr.limit) {
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        }
        TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, bitmap_offset,
            TYPE_REFERENCE_OF(bitmap_byte), 1u));
        if (TYPE_GET_BIT(bitmap_byte, 1u << (checked_port & 7u))) {
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        }
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _kpa_test_mode(core_machine_cpu_execution_context *context, type_unsigned_16 portid, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_p_test");
    if (_GetCR0_PE && (_GetCPL > (type_unsigned_8)_GetEFLAGS_IOPL || _GetEFLAGS_VM))
    {
        TYPE_TRACE_BLOCK_BEGIN("CR0_PE(1),(CPL>IOPL/EFLAGS_VM(1))");
        TYPE_TRACE_CHECK_RETURN(_kpa_test_iomap(context, portid, byte));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
/* regular portid accessing */
static C_VOID _p_input(core_machine_cpu_execution_context *context, type_unsigned_16 portid, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_p_input");
    TYPE_TRACE_CHECK_RETURN(_kpa_test_mode(context, portid, byte));
    if (byte != 1u && byte != 2u && byte != 4u) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
    }
    _p_publish_external_cycle(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN,
        portid, byte, TYPE_FALSE);
    if (context->transaction != STD_NULL && core_machine_transaction_begin(
            context->transaction, CORE_MACHINE_TRANSACTION_OWNER_CPU,
            CORE_MACHINE_TRANSACTION_CPU_PORT_READ, portid, byte, 0u) !=
            TYPE_STATUS_OK) {
        _p_publish_external_cycle(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
            portid, byte, TYPE_FALSE);
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(portid));
    }
    if (core_machine_port_execute_read(context->port, portid) != TYPE_STATUS_OK) {
        core_machine_transaction_cancel(context->transaction);
        _p_publish_external_cycle(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
            portid, byte, TYPE_FALSE);
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(portid));
    }
    core_machine_transaction_set_value(context->transaction,
        byte == 1u ? context->port->data.ioByte :
        (byte == 2u ? context->port->data.ioWord : context->port->data.ioDWord));
    switch (byte)
    {
    case 1:
        TYPE_TRACE_BLOCK_BEGIN("byte(1)");
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, rdata,
                                             TYPE_REFERENCE_OF(context->port->data.ioByte), 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, rdata,
                                             TYPE_REFERENCE_OF(context->port->data.ioWord), 2));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, rdata,
                                             TYPE_REFERENCE_OF(context->port->data.ioDWord), 4));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("byte");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    instruction_state.data.flagIgnore = TYPE_TRUE;
    core_machine_transaction_commit(context->transaction);
    _p_publish_external_cycle(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT,
        portid, byte, TYPE_FALSE);
    TYPE_TRACE_CALL_END;
}
static C_VOID _p_output(core_machine_cpu_execution_context *context, type_unsigned_16 portid, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_p_output");
    TYPE_TRACE_CHECK_RETURN(_kpa_test_mode(context, portid, byte));
    switch (byte)
    {
    case 1:
        TYPE_TRACE_BLOCK_BEGIN("byte(1)");
        TYPE_TRACE_CHECK_RETURN(_m_read_ref(context, rdata,
                                            TYPE_REFERENCE_OF(context->port->data.ioByte), 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        TYPE_TRACE_CHECK_RETURN(_m_read_ref(context, rdata,
                                            TYPE_REFERENCE_OF(context->port->data.ioWord), 2));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        TYPE_TRACE_CHECK_RETURN(_m_read_ref(context, rdata,
                                            TYPE_REFERENCE_OF(context->port->data.ioDWord), 4));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("byte");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    _p_publish_external_cycle(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN,
        portid, byte, TYPE_TRUE);
    if (context->transaction != STD_NULL && core_machine_transaction_begin(
            context->transaction, CORE_MACHINE_TRANSACTION_OWNER_CPU,
            CORE_MACHINE_TRANSACTION_CPU_PORT_WRITE, portid,
            byte == 1u ? context->port->data.ioByte :
            (byte == 2u ? context->port->data.ioWord : context->port->data.ioDWord),
            byte) !=
            TYPE_STATUS_OK) {
        _p_publish_external_cycle(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
            portid, byte, TYPE_TRUE);
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(portid));
    }
    if (core_machine_port_execute_write(context->port, portid) != TYPE_STATUS_OK) {
        core_machine_transaction_cancel(context->transaction);
        _p_publish_external_cycle(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
            portid, byte, TYPE_TRUE);
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(portid));
    }
    instruction_state.data.flagIgnore = TYPE_TRUE;
    core_machine_transaction_commit(context->transaction);
    _p_publish_external_cycle(context, CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT,
        portid, byte, TYPE_TRUE);
    TYPE_TRACE_CALL_END;
}

/* decoding unit */
/* kernel decoding function */
static type_bool _kdf_check_prefix(core_machine_cpu_execution_context *context, type_unsigned_8 opcode)
{
    switch (opcode)
    {
    case 0xf0:
    case 0xf2:
    case 0xf3:
    case 0x2e:
    case 0x36:
    case 0x3e:
    case 0x26:
        return TYPE_TRUE;
        break;
    case 0x64:
    case 0x65:
    case 0x66:
    case 0x67:
        if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
            return TYPE_TRUE;
        else
            return TYPE_FALSE;
        break;
    default:
        return TYPE_FALSE;
        break;
    }
    return TYPE_FALSE;
}

static C_VOID _kdf_skip(core_machine_cpu_execution_context *context, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_kdf_skip");
    if (cpu_state.data.cs.seg.exec.defsize)
        TYPE_TRACE_CHECK_RETURN(cpu_state.data.eip += byte);
    else
        TYPE_TRACE_CHECK_RETURN(cpu_state.data.eip = TYPE_MASK_UNSIGNED_16(
            cpu_state.data.eip + byte));
    context->prefetch_expected_linear = cpu_state.data.cs.base + cpu_state.data.eip;
    context->prefetch_expected_valid = TYPE_TRUE;
    TYPE_TRACE_CALL_END;
}
static C_VOID _kdf_code(core_machine_cpu_execution_context *context, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_kdf_code");
    TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, rdata, byte));
    TYPE_TRACE_CHECK_RETURN(_kdf_skip(context, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _kdf_modrm_with_mod_quirk(core_machine_cpu_execution_context *context, type_unsigned_8 regbyte, type_unsigned_8 rmbyte, type_bool ignore_mod)
{
    type_signed_8 disp8;
    type_unsigned_16 disp16;
    type_unsigned_32 disp32;
    type_unsigned_32 sibindex;
    type_unsigned_8 modrm, sib;
    TYPE_TRACE_CALL_BEGIN("_kdf_modrm");
    TYPE_TRACE_CHECK_RETURN(_kdf_code(context, TYPE_REFERENCE_OF(modrm), 1));
    instruction_state.data.flagMem = TYPE_TRUE;
    instruction_state.data.mrm.rsreg = STD_NULL;
    instruction_state.data.mrm.offset = 0;
    instruction_state.data.cr = instruction_state.data.crm = 0;
    instruction_state.data.rrm = instruction_state.data.rr = (type_virtual_address)STD_NULL;
    /* The normal architecture requires MOD=11 for MOV CR. Selected original
     * 80386 silicon instead takes only the r/m register bits: no effective
     * address, SIB, displacement, or memory operation is decoded. */
    if (!ignore_mod || _GetModRM_MOD(modrm) == 3)
    {
        switch (_GetAddressSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
        switch (_GetModRM_MOD(modrm))
        {
        case 0:
            TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(0)");
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.bx + cpu_state.data.si);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 1:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.bx + cpu_state.data.di);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 2:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.bp + cpu_state.data.si);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 3:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.bp + cpu_state.data.di);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 4:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.si);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 5:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.di);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 6:
                TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(6)");
                TYPE_TRACE_CHECK_RETURN(_kdf_code(context, TYPE_REFERENCE_OF(disp16), 2));
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                TYPE_TRACE_BLOCK_END;
                break;
            case 7:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.bx);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
            break;
        case 1:
            TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(1)");
            TYPE_TRACE_CHECK_RETURN(_kdf_code(context, TYPE_REFERENCE_OF(disp8), 1));
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.bx + cpu_state.data.si + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 1:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.bx + cpu_state.data.di + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 2:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.bp + cpu_state.data.si + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 3:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.bp + cpu_state.data.di + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 4:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.si + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 5:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.di + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 6:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.bp + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 7:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.bx + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
            break;
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(2)");
            TYPE_TRACE_CHECK_RETURN(_kdf_code(context, TYPE_REFERENCE_OF(disp16), 2));
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.bx + cpu_state.data.si + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 1:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.bx + cpu_state.data.di + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 2:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.bp + cpu_state.data.si + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 3:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.bp + cpu_state.data.di + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 4:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.si + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 5:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.di + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 6:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.bp + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 7:
                instruction_state.data.mrm.offset = TYPE_MASK_UNSIGNED_16(cpu_state.data.bx + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
            break;
        case 3:
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
        if (_GetModRM_MOD(modrm) != 3 && _GetModRM_RM(modrm) == 4)
        {
            TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(!3),ModRM_RM(4)");
            TYPE_TRACE_CHECK_RETURN(_kdf_code(context, TYPE_REFERENCE_OF(sib), 1));
            switch (_GetSIB_Index(sib))
            {
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
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            sibindex <<= _GetSIB_SS(sib);
            TYPE_TRACE_BLOCK_END;
        }
        switch (_GetModRM_MOD(modrm))
        {
        case 0:
            TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(0)");
            switch (_GetModRM_RM(modrm))
            {
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
                TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(4)");
                switch (_GetSIB_Base(sib))
                {
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
                    TYPE_TRACE_BLOCK_BEGIN("SIB_Base(5)");
                    TYPE_TRACE_CHECK_RETURN(_kdf_code(context, TYPE_REFERENCE_OF(disp32), 4));
                    instruction_state.data.mrm.offset = disp32 + sibindex;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    TYPE_TRACE_BLOCK_END;
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
                    TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                TYPE_TRACE_BLOCK_END;
                break;
            case 5:
                TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(5)");
                TYPE_TRACE_CHECK_RETURN(_kdf_code(context, TYPE_REFERENCE_OF(disp32), 4));
                instruction_state.data.mrm.offset = disp32;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                TYPE_TRACE_BLOCK_END;
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
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
            break;
        case 1:
            TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(1)");
            TYPE_TRACE_CHECK_RETURN(_kdf_code(context, TYPE_REFERENCE_OF(disp8), 1));
            switch (_GetModRM_RM(modrm))
            {
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
                TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(4)");
                switch (_GetSIB_Base(sib))
                {
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
                    TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                TYPE_TRACE_BLOCK_END;
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
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
            break;
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(2)");
            TYPE_TRACE_CHECK_RETURN(_kdf_code(context, TYPE_REFERENCE_OF(disp32), 4));
            switch (_GetModRM_RM(modrm))
            {
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
                TYPE_TRACE_BLOCK_BEGIN("ModRM_RM(4)");
                switch (_GetSIB_Base(sib))
                {
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
                    TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                TYPE_TRACE_BLOCK_END;
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
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
            break;
        case 3:
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    }
    if (_GetModRM_MOD(modrm) == 3 || ignore_mod)
    {
        TYPE_TRACE_BLOCK_BEGIN("ModRM_MOD(3)");
        instruction_state.data.flagMem = TYPE_FALSE;
        switch (rmbyte)
        {
        case 1:
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.al);
                break;
            case 1:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.cl);
                break;
            case 2:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.dl);
                break;
            case 3:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.bl);
                break;
            case 4:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.ah);
                break;
            case 5:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.ch);
                break;
            case 6:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.dh);
                break;
            case 7:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.bh);
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        case 2:
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.ax);
                break;
            case 1:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.cx);
                break;
            case 2:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.dx);
                break;
            case 3:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.bx);
                break;
            case 4:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.sp);
                break;
            case 5:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.bp);
                break;
            case 6:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.si);
                break;
            case 7:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.di);
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        case 4:
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.eax);
                break;
            case 1:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.ecx);
                break;
            case 2:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.edx);
                break;
            case 3:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.ebx);
                break;
            case 4:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.esp);
                break;
            case 5:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.ebp);
                break;
            case 6:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.esi);
                break;
            case 7:
                instruction_state.data.rrm = (type_virtual_address)(&cpu_state.data.edi);
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        default:
            TYPE_TRACE_BLOCK_BEGIN("rmbyte");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(rmbyte));
            TYPE_TRACE_BLOCK_END;
            break;
        }
        TYPE_TRACE_CHECK_RETURN(_m_read_ref(context, instruction_state.data.rrm, TYPE_REFERENCE_OF(instruction_state.data.crm), rmbyte));
        TYPE_TRACE_BLOCK_END;
    }
    if (!regbyte)
    {
        /* reg is operation or segment */
        instruction_state.data.cr = _GetModRM_REG(modrm);
    }
    else
    {
        switch (regbyte)
        {
        case 1:
            switch (_GetModRM_REG(modrm))
            {
            case 0:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.al);
                break;
            case 1:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.cl);
                break;
            case 2:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.dl);
                break;
            case 3:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.bl);
                break;
            case 4:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.ah);
                break;
            case 5:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.ch);
                break;
            case 6:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.dh);
                break;
            case 7:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.bh);
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        case 2:
            switch (_GetModRM_REG(modrm))
            {
            case 0:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.ax);
                break;
            case 1:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.cx);
                break;
            case 2:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.dx);
                break;
            case 3:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.bx);
                break;
            case 4:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.sp);
                break;
            case 5:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.bp);
                break;
            case 6:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.si);
                break;
            case 7:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.di);
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        case 4:
            switch (_GetModRM_REG(modrm))
            {
            case 0:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.eax);
                break;
            case 1:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.ecx);
                break;
            case 2:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.edx);
                break;
            case 3:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.ebx);
                break;
            case 4:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.esp);
                break;
            case 5:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.ebp);
                break;
            case 6:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.esi);
                break;
            case 7:
                instruction_state.data.rr = (type_virtual_address)(&cpu_state.data.edi);
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        default:
            TYPE_TRACE_BLOCK_BEGIN("regbyte");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(regbyte));
            TYPE_TRACE_BLOCK_END;
            break;
        }
        TYPE_TRACE_CHECK_RETURN(_m_read_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.cr), regbyte));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _kdf_modrm(core_machine_cpu_execution_context *context,
    type_unsigned_8 regbyte, type_unsigned_8 rmbyte)
{
    _kdf_modrm_with_mod_quirk(context, regbyte, rmbyte, TYPE_FALSE);
}

static C_VOID _kdf_modrm_creg(core_machine_cpu_execution_context *context)
{
    _kdf_modrm_with_mod_quirk(context, 0, 4,
        context->cpu_80386_cr_mov_ignores_mod);
}
static C_VOID _d_skip(core_machine_cpu_execution_context *context, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_d_skip");
    TYPE_TRACE_CHECK_RETURN(_kdf_skip(context, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _d_code(core_machine_cpu_execution_context *context, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_d_code");
    TYPE_TRACE_CHECK_RETURN(_kdf_code(context, rdata, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _d_imm(core_machine_cpu_execution_context *context, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_d_imm");
    instruction_state.data.cimm = 0;
    TYPE_TRACE_CHECK_RETURN(_d_code(context, TYPE_REFERENCE_OF(instruction_state.data.cimm), byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _d_moffs(core_machine_cpu_execution_context *context, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_d_moffs");
    instruction_state.data.flagMem = TYPE_TRUE;
    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
    instruction_state.data.mrm.offset = 0;
    switch (_GetAddressSize)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
        TYPE_TRACE_CHECK_RETURN(_d_code(context, TYPE_REFERENCE_OF(instruction_state.data.mrm.offset), 2));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
        TYPE_TRACE_CHECK_RETURN(_d_code(context, TYPE_REFERENCE_OF(instruction_state.data.mrm.offset), 4));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _d_modrm_sreg(core_machine_cpu_execution_context *context, type_unsigned_8 rmbyte)
{
    TYPE_TRACE_CALL_BEGIN("_d_modrm_sreg");
    TYPE_TRACE_CHECK_RETURN(_kdf_modrm(context, 0, rmbyte));
    instruction_state.data.rmovsreg = STD_NULL;
    switch (instruction_state.data.cr)
    {
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
        if (context->cpu_profile < CORE_MACHINE_CPU_PROFILE_80386)
            TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        instruction_state.data.rmovsreg = &cpu_state.data.fs;
        break;
    case 5:
        if (context->cpu_profile < CORE_MACHINE_CPU_PROFILE_80386)
            TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        instruction_state.data.rmovsreg = &cpu_state.data.gs;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _d_modrm_ea(core_machine_cpu_execution_context *context, type_unsigned_8 regbyte, type_unsigned_8 rmbyte)
{
    TYPE_TRACE_CALL_BEGIN("_d_modrm_ea");
    TYPE_TRACE_CHECK_RETURN(_kdf_modrm(context, regbyte, rmbyte));
    if (!instruction_state.data.flagMem)
    {
        TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _d_modrm_table_memory(core_machine_cpu_execution_context *context,
    type_unsigned_8 modrm)
{
    TYPE_TRACE_CALL_BEGIN("_d_modrm_table_memory");
    /* Reject a register form before the six-byte memory operand is decoded. */
    if (_GetModRM_MOD(modrm) == 3)
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
    TYPE_TRACE_CHECK_RETURN(_d_modrm_ea(context, 0, 6));
    TYPE_TRACE_CALL_END;
}
/* Store one six-byte SGDT/SIDT pseudo-descriptor after a complete write check.
 * The 80286 manual calls byte six undefined, but physical 80286 software
 * observes FF there to distinguish a 286 from a 386.  Keep that observable
 * compatibility value at this sole table-register boundary. */
static C_VOID _m_write_table_pseudo_descriptor(
    core_machine_cpu_execution_context *context, type_unsigned_16 limit,
    type_unsigned_32 base)
{
    type_unsigned_8 image[6];
    TYPE_TRACE_CALL_BEGIN("_m_write_table_pseudo_descriptor");
    image[0] = TYPE_MASK_UNSIGNED_8(limit);
    image[1] = TYPE_MASK_UNSIGNED_8(limit >> 8u);
    image[2] = TYPE_MASK_UNSIGNED_8(base);
    image[3] = TYPE_MASK_UNSIGNED_8(base >> 8u);
    image[4] = TYPE_MASK_UNSIGNED_8(base >> 16u);
    image[5] = context->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ?
        0xffu : TYPE_MASK_UNSIGNED_8(base >> 24u);
    TYPE_TRACE_CHECK_RETURN(_m_test_access(context,
        instruction_state.data.mrm.rsreg, instruction_state.data.mrm.offset,
        6, TYPE_TRUE));
    TYPE_TRACE_CHECK_RETURN(_m_write_logical(context,
        instruction_state.data.mrm.rsreg, instruction_state.data.mrm.offset,
        (type_virtual_address)image, 6));
    TYPE_TRACE_CALL_END;
}
static C_VOID _d_modrm(core_machine_cpu_execution_context *context, type_unsigned_8 regbyte, type_unsigned_8 rmbyte)
{
    TYPE_TRACE_CALL_BEGIN("_d_modrm");
    TYPE_TRACE_CHECK_RETURN(_kdf_modrm(context, regbyte, rmbyte));
    if (!instruction_state.data.flagMem && instruction_state.data.flagLock)
    {
        TYPE_TRACE_BLOCK_BEGIN("flagMem(0),flagLock(1)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}

/* execution control unit: _e_ */
/* kernel execution control */
static C_VOID _kec_push(core_machine_cpu_execution_context *context, type_virtual_address rdata, type_unsigned_8 byte)
{
    type_unsigned_32 cesp;
    TYPE_TRACE_CALL_BEGIN("_kec_push");
    TYPE_TRACE_CHECK_RETURN(_s_test_ss_push(context, byte));
    switch (_GetStackSize)
    {
    case 2:
        cesp = TYPE_MASK_UNSIGNED_16(cpu_state.data.sp - byte);
        break;
    case 4:
        cesp = TYPE_MASK_UNSIGNED_32(cpu_state.data.esp - byte);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CHECK_RETURN(_s_write_ss(context, cesp, rdata, byte));
    switch (_GetStackSize)
    {
    case 2:
        cpu_state.data.sp -= byte;
        break;
    case 4:
        cpu_state.data.esp -= byte;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _kec_pop(core_machine_cpu_execution_context *context, type_virtual_address rdata, type_unsigned_8 byte)
{
    type_unsigned_32 cesp;
    TYPE_TRACE_CALL_BEGIN("_kec_pop");
    switch (_GetStackSize)
    {
    case 2:
        cesp = cpu_state.data.sp;
        break;
    case 4:
        cesp = cpu_state.data.esp;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CHECK_RETURN(_s_read_ss(context, cesp, rdata, byte));
    if (rdata != TYPE_REFERENCE_OF(cpu_state.data.esp))
    {
        switch (_GetStackSize)
        {
        case 2:
            cpu_state.data.sp += byte;
            break;
        case 4:
            cpu_state.data.esp += byte;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _kec_call_far(core_machine_cpu_execution_context *context, type_unsigned_16 newcs, type_unsigned_32 neweip, type_unsigned_8 byte)
{
    type_unsigned_32 oldcs = cpu_state.data.cs.selector;
    t_cpu_data_sreg ccs = cpu_state.data.cs;
    TYPE_TRACE_CALL_BEGIN("_kec_call_far");
    switch (byte)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        TYPE_TRACE_CHECK_RETURN(_s_test_ss_push(context, 4));
        neweip = TYPE_MASK_UNSIGNED_16(neweip);
        TYPE_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
        TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldcs), 2));
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(cpu_state.data.ip), 2));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        TYPE_TRACE_CHECK_RETURN(_s_test_ss_push(context, 8));
        neweip = TYPE_MASK_UNSIGNED_32(neweip);
        TYPE_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
        TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldcs), 4));
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(cpu_state.data.eip), 4));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    cpu_state.data.cs = ccs;
    cpu_state.data.eip = neweip;
    TYPE_TRACE_CALL_END;
}
static C_VOID _kec_call_near(core_machine_cpu_execution_context *context, type_unsigned_32 neweip, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_kec_call_near");
    switch (byte)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        neweip = TYPE_MASK_UNSIGNED_16(neweip);
        TYPE_TRACE_CHECK_RETURN(_s_test_cs(context, neweip, 0x01));
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(cpu_state.data.ip), 2));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        neweip = TYPE_MASK_UNSIGNED_32(neweip);
        TYPE_TRACE_CHECK_RETURN(_s_test_cs(context, neweip, 0x01));
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(cpu_state.data.eip), 4));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    cpu_state.data.eip = neweip;
    TYPE_TRACE_CALL_END;
}
_______todo _kec_task_switch(type_unsigned_16 newtss);
static C_VOID _kec_jmp_far(core_machine_cpu_execution_context *context, type_unsigned_16 newcs, type_unsigned_32 neweip, type_unsigned_8 byte)
{
    t_cpu_data_sreg ccs = cpu_state.data.cs;
    TYPE_TRACE_CALL_BEGIN("_kec_jmp_far");
    switch (byte)
    {
    case 2:
        neweip = TYPE_MASK_UNSIGNED_16(neweip);
        break;
    case 4:
        neweip = TYPE_MASK_UNSIGNED_32(neweip);
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("byte");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
    TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
    cpu_state.data.cs = ccs;
    cpu_state.data.eip = neweip;
    TYPE_TRACE_CALL_END;
}
static C_VOID _kec_jmp_near(core_machine_cpu_execution_context *context, type_unsigned_32 neweip, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_kec_jmp_near");
    switch (byte)
    {
    case 2:
        neweip = TYPE_MASK_UNSIGNED_16(neweip);
        break;
    case 4:
        neweip = TYPE_MASK_UNSIGNED_32(neweip);
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CHECK_RETURN(_s_test_cs(context, neweip, 0x01));
    cpu_state.data.eip = neweip;
    TYPE_TRACE_CALL_END;
}
static C_VOID _kec_ret_far(core_machine_cpu_execution_context *context, type_unsigned_32 newcs, type_unsigned_32 neweip, type_unsigned_16 parambyte, type_unsigned_16 byte)
{
    t_cpu_data_sreg ccs = cpu_state.data.cs;
    TYPE_TRACE_CALL_BEGIN("_kec_ret_far");
    switch (byte)
    {
    case 2:
        neweip = TYPE_MASK_UNSIGNED_16(neweip);
        break;
    case 4:
        neweip = TYPE_MASK_UNSIGNED_32(neweip);
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("byte");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
    TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
    cpu_state.data.cs = ccs;
    cpu_state.data.eip = neweip;
    switch (_GetStackSize)
    {
    case 2:
        cpu_state.data.sp += parambyte;
        break;
    case 4:
        cpu_state.data.esp += parambyte;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _kec_ret_near(core_machine_cpu_execution_context *context, type_unsigned_16 parambyte, type_unsigned_8 byte)
{
    type_unsigned_32 neweip = 0;
    TYPE_TRACE_CALL_BEGIN("_kec_ret_near");
    switch (byte)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(neweip), 2));
        TYPE_TRACE_CHECK_RETURN(_s_test_cs(context, neweip, 0x01));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(neweip), 4));
        TYPE_TRACE_CHECK_RETURN(_s_test_cs(context, neweip, 0x01));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("byte");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    cpu_state.data.eip = neweip;
    switch (_GetStackSize)
    {
    case 2:
        cpu_state.data.sp += parambyte;
        break;
    case 4:
        cpu_state.data.esp += parambyte;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
/* sub execution routine */
static C_VOID _ser_call_far_real(core_machine_cpu_execution_context *context, type_unsigned_16 newcs, type_unsigned_32 neweip, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_ser_call_far_real");
    if (_IsProtected)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CHECK_RETURN(_kec_call_far(context, newcs, neweip, byte));
    TYPE_TRACE_CALL_END;
}
_______todo _ser_call_far_cs_conf(core_machine_cpu_execution_context *context, type_unsigned_16 newcs, type_unsigned_32 neweip, type_unsigned_8 byte)
{
    type_unsigned_64 descriptor;
    TYPE_TRACE_CALL_BEGIN("_ser_call_far_cs_conf");
    if (!_IsProtected)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescCodeConform(descriptor))
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetDesc_DPL(descriptor) > _GetCPL)
    {
        TYPE_TRACE_BLOCK_BEGIN("DPL(>CPL)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs));
        TYPE_TRACE_BLOCK_END;
    }
    if (!_IsDescPresent(descriptor))
    {
        TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CHECK_RETURN(_kec_call_far(context, newcs, neweip, byte));
    TYPE_TRACE_CALL_END;
}
_______todo _ser_call_far_cs_nonc(core_machine_cpu_execution_context *context, type_unsigned_16 newcs, type_unsigned_32 neweip, type_unsigned_8 byte)
{
    type_unsigned_64 descriptor;
    TYPE_TRACE_CALL_BEGIN("_ser_call_far_cs_nonc");
    if (!_IsProtected)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescCodeNonConform(descriptor))
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetDesc_DPL(descriptor) != _GetCPL ||
        _GetSelector_RPL(newcs) > _GetCPL)
    {
        TYPE_TRACE_BLOCK_BEGIN("DPL(!CPL)/RPL(>CPL)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs));
        TYPE_TRACE_BLOCK_END;
    }
    if (!_IsDescPresent(descriptor))
    {
        TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CHECK_RETURN(_kec_call_far(context, newcs, neweip, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _ser_check_call_gate_stack_sreg(
    core_machine_cpu_execution_context *context, type_unsigned_16 selector,
    type_unsigned_8 target_cpl, type_virtual_address out_descriptor)
{
    type_unsigned_64 descriptor;

    TYPE_TRACE_CALL_BEGIN("_ser_check_call_gate_stack_sreg");
    if (_IsSelectorNull(selector) || _GetSelector_TI(selector) ||
        _GetSelector_RPL(selector) != target_cpl)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(selector & 0xfffcu));
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, selector,
        TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescDataWritable(descriptor) ||
        _GetDesc_DPL(descriptor) != target_cpl)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(selector & 0xfffcu));
    if (!_IsDescPresent(descriptor))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_SS(selector & 0xfffcu));
    STD_MEMCPY((C_VOID *)out_descriptor, (C_VOID *)TYPE_REFERENCE_OF(descriptor),
        sizeof(descriptor));
    TYPE_TRACE_CALL_END;
}
static C_VOID _ser_call_far_call_gate_32(core_machine_cpu_execution_context *context,
    type_unsigned_16 gate_selector)
{
    type_unsigned_64 gate_desc;
    type_unsigned_64 code_desc;
    type_unsigned_64 ss_desc;
    type_unsigned_16 newcs;
    type_unsigned_16 newss;
    type_unsigned_16 oldcs;
    type_unsigned_16 oldss;
    type_unsigned_32 newesp;
    type_unsigned_32 oldeip;
    type_unsigned_32 oldesp;
    type_unsigned_32 oldcs_frame;
    type_unsigned_32 oldss_frame;
    type_unsigned_32 parameters[31];
    type_unsigned_8 oldcpl;
    type_unsigned_8 target_cpl;
    type_unsigned_8 parameter_count;
    type_unsigned_8 index;
    t_cpu_data_sreg newcs_cache;
    t_cpu_data_sreg newss_cache;

    TYPE_TRACE_CALL_BEGIN("_ser_call_far_call_gate_32");
    if (!_IsProtected || _GetEFLAGS_VM)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, gate_selector,
        TYPE_REFERENCE_OF(gate_desc)));
    if (!_IsDescCallGate32(gate_desc))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(gate_selector & 0xfffcu));
    oldcpl = _GetCPL;
    if (_GetDesc_DPL(gate_desc) < oldcpl ||
        _GetDesc_DPL(gate_desc) < _GetSelector_RPL(gate_selector))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(gate_selector & 0xfffcu));
    if (!_IsDescPresent(gate_desc))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(gate_selector & 0xfffcu));
    newcs = TYPE_MASK_UNSIGNED_16(_GetDescGate_Selector(gate_desc));
    if (_IsSelectorNull(newcs) || _GetSelector_TI(newcs))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
        TYPE_REFERENCE_OF(code_desc)));
    if (!_IsDescCodeNonConform(code_desc) ||
        _GetDesc_DPL(code_desc) > oldcpl)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    target_cpl = (type_unsigned_8)_GetDesc_DPL(code_desc);
    if (!_IsDescPresent(code_desc))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
    oldcs = cpu_state.data.cs.selector;
    oldss = cpu_state.data.ss.selector;
    oldeip = cpu_state.data.eip;
    oldesp = cpu_state.data.esp;
    oldcs_frame = oldcs;
    oldss_frame = oldss;
    newcs_cache = cpu_state.data.cs;
    TYPE_TRACE_CHECK_RETURN(_ksa_prepare_code_sreg(context, newcs, target_cpl,
        &newcs_cache, &code_desc));
    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &newcs_cache,
        TYPE_MASK_UNSIGNED_32(_GetDescGate_Offset(gate_desc)), 1u, 0,
        target_cpl, 1));
    if (target_cpl < oldcpl) {
        if (!cpu_state.data.tr.flagValid || cpu_state.data.tr.sys.type !=
            VCPU_DESC_SYS_TYPE_TSS_32_BUSY)
            TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(0));
        TYPE_TRACE_CHECK_RETURN(_s_test_tss(context, 4u, 6u));
        TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, 4u,
            TYPE_REFERENCE_OF(newesp), 4u));
        TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, 8u,
            TYPE_REFERENCE_OF(newss), 2u));
        TYPE_TRACE_CHECK_RETURN(_ser_check_call_gate_stack_sreg(context, newss,
            target_cpl, TYPE_REFERENCE_OF(ss_desc)));
        parameter_count = (type_unsigned_8)_GetDescCall_Count(gate_desc);
        newss_cache = cpu_state.data.ss;
        TYPE_TRACE_CHECK_RETURN(_ksa_prepare_stack_sreg(context, newss,
            target_cpl, &newss_cache, &ss_desc));
        TYPE_TRACE_CHECK_RETURN(_s_test_stack_frame_32(context, &newss_cache,
            newesp, (type_unsigned_8)(4u + parameter_count), target_cpl));
        for (index = 0u; index < parameter_count; ++index)
            TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context,
                (type_unsigned_32)index * 4u,
                TYPE_REFERENCE_OF(parameters[index]), 4u));
        TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newss,
            TYPE_REFERENCE_OF(ss_desc)));
        TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
            TYPE_REFERENCE_OF(code_desc)));
        cpu_state.data.ss = newss_cache;
        if (newss_cache.seg.data.big)
            cpu_state.data.esp = newesp;
        else
            cpu_state.data.sp = TYPE_MASK_UNSIGNED_16(newesp);
        _MakeCPL(target_cpl);
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldss_frame),
            4u));
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldesp),
            4u));
        for (index = parameter_count; index > 0u; --index)
            TYPE_TRACE_CHECK_RETURN(_kec_push(context,
                TYPE_REFERENCE_OF(parameters[index - 1u]), 4u));
    } else {
        TYPE_TRACE_CHECK_RETURN(_s_test_ss_push(context, 8u));
        TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
            TYPE_REFERENCE_OF(code_desc)));
    }
    TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldcs_frame), 4u));
    TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldeip), 4u));
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.eip = TYPE_MASK_UNSIGNED_32(_GetDescGate_Offset(gate_desc));
    TYPE_TRACE_CALL_END;
}
static C_VOID _ser_call_far_call_gate(core_machine_cpu_execution_context *context,
    type_unsigned_16 gate_selector, type_unsigned_8 byte)
{
    type_unsigned_64 gate_desc;
    type_unsigned_64 code_desc;
    type_unsigned_64 ss_desc;
    type_unsigned_16 newcs;
    type_unsigned_16 newss;
    type_unsigned_16 newsp;
    type_unsigned_32 newesp = 0;
    type_unsigned_16 oldss;
    type_unsigned_16 oldsp;
    type_unsigned_16 oldcs;
    type_unsigned_16 oldip;
    type_unsigned_16 parameters[31];
    type_unsigned_8 oldcpl;
    type_unsigned_8 target_cpl;
    type_unsigned_8 parameter_count;
    type_unsigned_8 index;
    t_cpu_data_sreg newcs_cache;
    t_cpu_data_sreg newss_cache;

    TYPE_TRACE_CALL_BEGIN("_ser_call_far_call_gate");
    if (!_IsProtected)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (byte == 4u) {
        TYPE_TRACE_CHECK_RETURN(_ser_call_far_call_gate_32(context,
            gate_selector));
        TYPE_TRACE_CALL_END;
        return;
    }
    if (byte != 2u)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, gate_selector,
        TYPE_REFERENCE_OF(gate_desc)));
    if (!_IsDescCallGate16(gate_desc))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(gate_selector & 0xfffcu));
    oldcpl = _GetCPL;
    if (_GetDesc_DPL(gate_desc) < oldcpl ||
        _GetDesc_DPL(gate_desc) < _GetSelector_RPL(gate_selector)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(gate_selector & 0xfffcu));
    }
    if (!_IsDescPresent(gate_desc)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(gate_selector & 0xfffcu));
    }
    newcs = TYPE_MASK_UNSIGNED_16(_GetDescGate_Selector(gate_desc));
    if (_IsSelectorNull(newcs) || _GetSelector_TI(newcs)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    }
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
        TYPE_REFERENCE_OF(code_desc)));
    if (!_IsDescCodeNonConform(code_desc)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    }
    if (!_IsDescPresent(code_desc)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
    }
    target_cpl = (type_unsigned_8)_GetDesc_DPL(code_desc);
    if (target_cpl > oldcpl) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    }
    oldss = cpu_state.data.ss.selector;
    oldsp = cpu_state.data.sp;
    oldcs = cpu_state.data.cs.selector;
    oldip = cpu_state.data.ip;
    parameter_count = (type_unsigned_8)_GetDescCall_Count(gate_desc);
    newcs_cache = cpu_state.data.cs;
    TYPE_TRACE_CHECK_RETURN(_ksa_prepare_code_sreg(context, newcs, target_cpl,
        &newcs_cache, &code_desc));
    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &newcs_cache,
        TYPE_MASK_UNSIGNED_16(_GetDescGate_Offset(gate_desc)), 1u, 0,
        target_cpl, 1));
    if (target_cpl < oldcpl) {
        if (!cpu_state.data.tr.flagValid) {
            TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(0));
        }
        switch (cpu_state.data.tr.sys.type) {
        case VCPU_DESC_SYS_TYPE_TSS_16_BUSY:
            TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, 2u,
                TYPE_REFERENCE_OF(newsp), 2u));
            TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, 4u,
                TYPE_REFERENCE_OF(newss), 2u));
            break;
        case VCPU_DESC_SYS_TYPE_TSS_32_BUSY:
            TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, 4u,
                TYPE_REFERENCE_OF(newesp), 4u));
            TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, 8u,
                TYPE_REFERENCE_OF(newss), 2u));
            if (newesp > 0xffffu) {
                TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(0));
            }
            newsp = TYPE_MASK_UNSIGNED_16(newesp);
            break;
        default:
            TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(0));
            break;
        }
        TYPE_TRACE_CHECK_RETURN(_ser_check_call_gate_stack_sreg(context, newss,
            target_cpl, TYPE_REFERENCE_OF(ss_desc)));
        newss_cache = cpu_state.data.ss;
        TYPE_TRACE_CHECK_RETURN(_ksa_prepare_stack_sreg(context, newss,
            target_cpl, &newss_cache, &ss_desc));
        TYPE_TRACE_CHECK_RETURN(_s_test_stack_frame_16(context, &newss_cache,
            newsp, (type_unsigned_8)(4u + parameter_count), target_cpl));
        for (index = 0u; index < parameter_count; ++index)
            TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context,
                (type_unsigned_32)index * 2u,
                TYPE_REFERENCE_OF(parameters[index]), 2u));

        TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newss,
            TYPE_REFERENCE_OF(ss_desc)));
        TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
            TYPE_REFERENCE_OF(code_desc)));
        cpu_state.data.ss = newss_cache;
        cpu_state.data.sp = newsp;
        _MakeCPL(target_cpl);
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldss), 2u));
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldsp), 2u));
        for (index = parameter_count; index > 0u; --index)
            TYPE_TRACE_CHECK_RETURN(_kec_push(context,
                TYPE_REFERENCE_OF(parameters[index - 1u]), 2u));
    }
    else {
        TYPE_TRACE_CHECK_RETURN(_s_test_ss_push(context, 4u));
        TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
            TYPE_REFERENCE_OF(code_desc)));
    }
    TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldcs), 2u));
    TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldip), 2u));
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.ip = TYPE_MASK_UNSIGNED_16(_GetDescGate_Offset(gate_desc));
    TYPE_TRACE_CALL_END;
}
static C_VOID _ser_task_switch_tss(core_machine_cpu_execution_context *context,
    type_unsigned_16 newcs, type_bool nested);
static C_VOID _ser_task_return_tss(core_machine_cpu_execution_context *context);

static C_VOID _ser_task_gate_descriptor(
    core_machine_cpu_execution_context *context, type_unsigned_64 descriptor,
    type_unsigned_16 gate_error, type_bool check_privilege,
    type_bool nested)
{
    type_unsigned_16 target_selector;

    TYPE_TRACE_CALL_BEGIN("_ser_task_gate_descriptor");
    if (!_IsDescTaskGate(descriptor))
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (check_privilege && _GetDesc_DPL(descriptor) < _GetCPL)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(gate_error));
    if (!_IsDescPresent(descriptor))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(gate_error));
    target_selector = TYPE_MASK_UNSIGNED_16(_GetDescGate_Selector(descriptor));
    if (_IsSelectorNull(target_selector) || _GetSelector_TI(target_selector))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(target_selector & 0xfffcu));
    TYPE_TRACE_CHECK_RETURN(_ser_task_switch_tss(context, target_selector,
        nested));
    TYPE_TRACE_CALL_END;
}

static C_VOID _ser_task_gate_target(core_machine_cpu_execution_context *context,
    type_unsigned_16 gate_selector, type_bool nested)
{
    type_unsigned_64 descriptor;

    TYPE_TRACE_CALL_BEGIN("_ser_task_gate_target");
    if (!_IsProtected || _GetEFLAGS_VM)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, gate_selector,
        TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescTaskGate(descriptor))
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetDesc_DPL(descriptor) < _GetSelector_RPL(gate_selector))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(gate_selector & 0xfffcu));
    TYPE_TRACE_CHECK_RETURN(_ser_task_gate_descriptor(context, descriptor,
        gate_selector & 0xfffcu, TYPE_TRUE, nested));
    TYPE_TRACE_CALL_END;
}

static C_VOID _ser_call_far_task_gate(core_machine_cpu_execution_context *context,
    type_unsigned_16 newcs)
{
    TYPE_TRACE_CALL_BEGIN("_ser_call_far_task_gate");
    TYPE_TRACE_CHECK_RETURN(_ser_task_gate_target(context, newcs, TYPE_TRUE));
    TYPE_TRACE_CALL_END;
}

static C_VOID _ser_call_far_tss(core_machine_cpu_execution_context *context,
    type_unsigned_16 newcs)
{
    TYPE_TRACE_CALL_BEGIN("_ser_call_far_tss");
    TYPE_TRACE_CHECK_RETURN(_ser_task_switch_tss(context, newcs, TYPE_TRUE));
    TYPE_TRACE_CALL_END;
}
static C_VOID _ser_int_real(core_machine_cpu_execution_context *context, type_unsigned_8 intid, type_unsigned_8 byte)
{
    type_unsigned_16 cip;
    type_unsigned_16 oldflags;
    type_unsigned_32 vector;
    type_unsigned_32 oldcs = cpu_state.data.cs.selector;
    TYPE_TRACE_CALL_BEGIN("_ser_int_real");
    if (_IsProtected)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (TYPE_MASK_UNSIGNED_16(intid * 4 + 3) > TYPE_MASK_UNSIGNED_16(cpu_state.data.idtr.limit))
    {
        TYPE_TRACE_BLOCK_BEGIN("intid(>idtr.limit)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        TYPE_TRACE_BLOCK_END;
    }
    switch (byte)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        TYPE_TRACE_CHECK_RETURN(_s_test_ss_push(context, 6));
        oldflags = _e_real_flags_image_16(context, cpu_state.data.flags);
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldflags), 2));
        _ClrEFLAGS_IF;
        _ClrEFLAGS_TF;
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldcs), 2));
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(cpu_state.data.ip), 2));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        TYPE_TRACE_CHECK_RETURN(_s_test_ss_push(context, 12));
        {
            type_unsigned_32 frame_flags =
                _e_real_flags_image_16(context, cpu_state.data.flags);
            TYPE_TRACE_CHECK_RETURN(_kec_push(context,
                TYPE_REFERENCE_OF(frame_flags), 4));
        }
        _ClrEFLAGS_IF;
        _ClrEFLAGS_TF;
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldcs), 4));
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(cpu_state.data.eip), 4));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("byte");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CHECK_RETURN(_s_read_idt(context, intid, TYPE_REFERENCE_OF(vector)));
    cip = TYPE_MASK_UNSIGNED_16(vector);
    TYPE_TRACE_CHECK_RETURN(_s_test_cs(context, cip, 0x01));
    cpu_state.data.eip = cip;
    TYPE_TRACE_CHECK_RETURN(_s_load_cs(context, TYPE_MASK_UNSIGNED_16(vector >> 16)));
    TYPE_TRACE_CALL_END;
}
static type_unsigned_16 _ser_idt_error_code(type_unsigned_8 intid)
{
    /* All currently admitted IDT validation is a synchronous CPU event. */
    return TYPE_MASK_UNSIGNED_16(intid * 8u + 2u);
}
static C_VOID _ser_int_protected_16(core_machine_cpu_execution_context *context,
    type_unsigned_8 intid, type_bool software_origin, type_bool error_frame)
{
    type_unsigned_64 gate_desc;
    type_unsigned_64 code_desc;
    type_unsigned_64 ss_desc;
    type_unsigned_16 newcs;
    type_unsigned_16 newss;
    type_unsigned_16 newsp;
    type_unsigned_32 newesp;
    type_unsigned_16 oldss;
    type_unsigned_16 oldsp;
    type_unsigned_16 oldcs;
    type_unsigned_16 oldip;
    type_unsigned_16 oldflags;
    type_unsigned_16 error_code;
    type_unsigned_8 oldcpl;
    type_unsigned_8 target_cpl;
    type_unsigned_8 frame_words;
    type_bool interrupt_gate;
    t_cpu_data_sreg newcs_cache;
    t_cpu_data_sreg newss_cache;

    TYPE_TRACE_CALL_BEGIN("_ser_int_protected_16");
    if (!_IsProtected || TYPE_MASK_UNSIGNED_16(intid * 8u + 7u) >
        TYPE_MASK_UNSIGNED_16(cpu_state.data.idtr.limit)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(_ser_idt_error_code(intid)));
    }
    TYPE_TRACE_CHECK_RETURN(_s_read_idt(context, intid,
        TYPE_REFERENCE_OF(gate_desc)));
    interrupt_gate = _GetDesc_Type(gate_desc) == VCPU_DESC_SYS_TYPE_INTGATE_16;
    if (!interrupt_gate && _GetDesc_Type(gate_desc) !=
        VCPU_DESC_SYS_TYPE_TRAPGATE_16) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(_ser_idt_error_code(intid)));
    }
    if (!_IsDescPresent(gate_desc)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(_ser_idt_error_code(intid)));
    }
    oldcpl = _GetCPL;
    if (software_origin && _GetDesc_DPL(gate_desc) < oldcpl) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(_ser_idt_error_code(intid)));
    }
    newcs = TYPE_MASK_UNSIGNED_16(_GetDescGate_Selector(gate_desc));
    if (_IsSelectorNull(newcs) || _GetSelector_TI(newcs)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    }
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
        TYPE_REFERENCE_OF(code_desc)));
    if (!_IsDescCodeNonConform(code_desc))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    if (!_IsDescPresent(code_desc))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
    target_cpl = (type_unsigned_8)_GetDesc_DPL(code_desc);
    if (target_cpl > oldcpl) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    }

    oldss = cpu_state.data.ss.selector;
    oldsp = cpu_state.data.sp;
    oldcs = cpu_state.data.cs.selector;
    oldip = cpu_state.data.ip;
    oldflags = cpu_state.data.flags;
    error_code = TYPE_MASK_UNSIGNED_16(instruction_state.data.excode);
    newcs_cache = cpu_state.data.cs;
    TYPE_TRACE_CHECK_RETURN(_ksa_prepare_code_sreg(context, newcs, target_cpl,
        &newcs_cache, &code_desc));
    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &newcs_cache,
        TYPE_MASK_UNSIGNED_16(_GetDescGate_Offset(gate_desc)), 1u, 0,
        target_cpl, 1));
    frame_words = (type_unsigned_8)(3u + (error_frame ? 1u : 0u));
    if (target_cpl < oldcpl) {
        if (!cpu_state.data.tr.flagValid) {
            TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(0));
        }
        switch (cpu_state.data.tr.sys.type) {
        case VCPU_DESC_SYS_TYPE_TSS_16_BUSY:
            TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, 2u,
                TYPE_REFERENCE_OF(newsp), 2u));
            TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, 4u,
                TYPE_REFERENCE_OF(newss), 2u));
            break;
        case VCPU_DESC_SYS_TYPE_TSS_32_BUSY:
            TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, 4u,
                TYPE_REFERENCE_OF(newesp), 4u));
            TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, 8u,
                TYPE_REFERENCE_OF(newss), 2u));
            if (newesp > 0xffffu) {
                TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(0));
            }
            newsp = TYPE_MASK_UNSIGNED_16(newesp);
            break;
        default:
            TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(0));
            break;
        }
        if (_IsSelectorNull(newss) || _GetSelector_TI(newss) ||
            _GetSelector_RPL(newss) != target_cpl) {
            TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(newss & 0xfffcu));
        }
        newss_cache = cpu_state.data.ss;
        TYPE_TRACE_CHECK_RETURN(_ksa_prepare_stack_sreg(context, newss,
            target_cpl, &newss_cache, &ss_desc));
        TYPE_TRACE_CHECK_RETURN(_s_test_stack_frame_16(context, &newss_cache,
            newsp, (type_unsigned_8)(frame_words + 2u), target_cpl));

        /* All faulting reads and writes are preflighted before CPU mutation. */
        TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newss,
            TYPE_REFERENCE_OF(ss_desc)));
        TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
            TYPE_REFERENCE_OF(code_desc)));
        cpu_state.data.ss = newss_cache;
        cpu_state.data.sp = newsp;
        _MakeCPL(target_cpl);
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldss), 2u));
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldsp), 2u));
    }
    else {
        TYPE_TRACE_CHECK_RETURN(_s_test_ss_push(context,
            (type_unsigned_8)(frame_words * 2u)));
        TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
            TYPE_REFERENCE_OF(code_desc)));
    }
    TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldflags), 2u));
    TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldcs), 2u));
    TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldip), 2u));
    if (error_frame) {
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(error_code),
            2u));
    }
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.ip = TYPE_MASK_UNSIGNED_16(_GetDescGate_Offset(gate_desc));
    if (interrupt_gate) _ClrEFLAGS_IF;
    _ClrEFLAGS_TF;
    TYPE_TRACE_CALL_END;
}
static C_VOID _ser_int_protected_32_outer(core_machine_cpu_execution_context *context,
    type_unsigned_8 intid, type_unsigned_64 gate_desc, type_bool software_origin,
    type_bool error_frame)
{
    type_unsigned_64 code_desc;
    type_unsigned_64 ss_desc;
    type_unsigned_16 newcs;
    type_unsigned_16 newss;
    type_unsigned_16 oldcs;
    type_unsigned_16 oldss;
    type_unsigned_16 oldes;
    type_unsigned_16 oldds;
    type_unsigned_16 oldfs;
    type_unsigned_16 oldgs;
    type_unsigned_32 newesp;
    type_unsigned_32 oldeip;
    type_unsigned_32 oldesp;
    type_unsigned_32 oldeflags;
    type_unsigned_32 oldcs_frame;
    type_unsigned_32 oldss_frame;
    type_unsigned_32 oldes_frame;
    type_unsigned_32 oldds_frame;
    type_unsigned_32 oldfs_frame;
    type_unsigned_32 oldgs_frame;
    type_unsigned_32 error_code;
    type_unsigned_8 oldcpl;
    type_unsigned_8 target_cpl;
    type_unsigned_8 frame_dwords;
    type_bool interrupt_gate;
    type_bool vm86_origin;
    t_cpu_data_sreg newcs_cache;
    t_cpu_data_sreg newss_cache;

    TYPE_TRACE_CALL_BEGIN("_ser_int_protected_32_outer");
    interrupt_gate = _GetDesc_Type(gate_desc) == VCPU_DESC_SYS_TYPE_INTGATE_32;
    if (!interrupt_gate && _GetDesc_Type(gate_desc) !=
        VCPU_DESC_SYS_TYPE_TRAPGATE_32)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (!_IsDescPresent(gate_desc))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(_ser_idt_error_code(intid)));
    oldcpl = _GetCPL;
    vm86_origin = _GetEFLAGS_VM;
    if (vm86_origin) {
        oldcs = cpu_state.data.cs.selector;
        oldss = cpu_state.data.ss.selector;
        oldes = cpu_state.data.es.selector;
        oldds = cpu_state.data.ds.selector;
        oldfs = cpu_state.data.fs.selector;
        oldgs = cpu_state.data.gs.selector;
        oldeip = cpu_state.data.eip;
        oldesp = cpu_state.data.esp;
        oldeflags = cpu_state.data.eflags;
        oldcs_frame = oldcs;
        oldss_frame = oldss;
        oldes_frame = oldes;
        oldds_frame = oldds;
        oldfs_frame = oldfs;
        oldgs_frame = oldgs;
        /* Descriptor and target-stack validation use protected semantics. */
        TYPE_CLEAR_BIT(cpu_state.data.eflags, VCPU_EFLAGS_VM);
    }
    if (software_origin && _GetDesc_DPL(gate_desc) < oldcpl)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(_ser_idt_error_code(intid)));
    newcs = TYPE_MASK_UNSIGNED_16(_GetDescGate_Selector(gate_desc));
    if (_IsSelectorNull(newcs) || _GetSelector_TI(newcs))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
        TYPE_REFERENCE_OF(code_desc)));
    if (!_IsDescCodeNonConform(code_desc) || _GetDesc_DPL(code_desc) > oldcpl)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    target_cpl = (type_unsigned_8)_GetDesc_DPL(code_desc);
    if (target_cpl >= oldcpl)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (!_IsDescPresent(code_desc))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
    if (!cpu_state.data.tr.flagValid || cpu_state.data.tr.sys.type !=
        VCPU_DESC_SYS_TYPE_TSS_32_BUSY)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(0));
    TYPE_TRACE_CHECK_RETURN(_s_test_tss(context, 4u, 6u));
    TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, 4u, TYPE_REFERENCE_OF(newesp),
        4u));
    TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, 8u, TYPE_REFERENCE_OF(newss),
        2u));
    if (_IsSelectorNull(newss) || _GetSelector_TI(newss) ||
        _GetSelector_RPL(newss) != target_cpl)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(newss & 0xfffcu));

    if (!vm86_origin) {
        oldcs = cpu_state.data.cs.selector;
        oldss = cpu_state.data.ss.selector;
        oldeip = cpu_state.data.eip;
        oldesp = cpu_state.data.esp;
        oldeflags = cpu_state.data.eflags;
        oldcs_frame = oldcs;
        oldss_frame = oldss;
    }
    error_code = TYPE_MASK_UNSIGNED_32(instruction_state.data.excode);
    frame_dwords = (type_unsigned_8)(5u + (vm86_origin ? 4u : 0u) +
        (error_frame ? 1u : 0u));
    newcs_cache = cpu_state.data.cs;
    TYPE_TRACE_CHECK_RETURN(_ksa_prepare_code_sreg(context, newcs, target_cpl,
        &newcs_cache, &code_desc));
    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &newcs_cache,
        TYPE_MASK_UNSIGNED_32(_GetDescGate_Offset(gate_desc)), 1u, 0,
        target_cpl, 1));
    newss_cache = cpu_state.data.ss;
    TYPE_TRACE_CHECK_RETURN(_ksa_prepare_stack_sreg(context, newss, target_cpl,
        &newss_cache, &ss_desc));
    TYPE_TRACE_CHECK_RETURN(_s_test_stack_frame_32(context, &newss_cache,
        newesp, frame_dwords, target_cpl));

    TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newss,
        TYPE_REFERENCE_OF(ss_desc)));
    TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
        TYPE_REFERENCE_OF(code_desc)));
    cpu_state.data.ss = newss_cache;
    if (newss_cache.seg.data.big)
        cpu_state.data.esp = newesp;
    else
        cpu_state.data.sp = TYPE_MASK_UNSIGNED_16(newesp);
    _MakeCPL(target_cpl);
    if (vm86_origin) {
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldgs_frame), 4u));
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldfs_frame), 4u));
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldds_frame), 4u));
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldes_frame), 4u));
    }
    TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldss_frame), 4u));
    TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldesp), 4u));
    TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldeflags), 4u));
    TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldcs_frame), 4u));
    TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldeip), 4u));
    if (error_frame)
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(error_code),
            4u));
    if (vm86_origin) {
        STD_MEMSET(&cpu_state.data.es, 0, sizeof(cpu_state.data.es));
        STD_MEMSET(&cpu_state.data.ds, 0, sizeof(cpu_state.data.ds));
        STD_MEMSET(&cpu_state.data.fs, 0, sizeof(cpu_state.data.fs));
        STD_MEMSET(&cpu_state.data.gs, 0, sizeof(cpu_state.data.gs));
    }
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.eip = TYPE_MASK_UNSIGNED_32(_GetDescGate_Offset(gate_desc));
    if (interrupt_gate) _ClrEFLAGS_IF;
    _ClrEFLAGS_TF;
    TYPE_TRACE_CALL_END;
}
static C_VOID _ser_int_protected_32_same(core_machine_cpu_execution_context *context,
    type_unsigned_8 intid, type_unsigned_64 gate_desc, type_bool software_origin,
    type_bool error_frame)
{
    type_unsigned_64 code_desc;
    type_unsigned_16 newcs;
    type_unsigned_16 oldcs;
    type_unsigned_32 oldeip;
    type_unsigned_32 oldeflags;
    type_unsigned_32 oldcs_frame;
    type_unsigned_32 error_code;
    type_unsigned_8 oldcpl;
    type_bool interrupt_gate;
    t_cpu_data_sreg newcs_cache;

    TYPE_TRACE_CALL_BEGIN("_ser_int_protected_32_same");
    interrupt_gate = _GetDesc_Type(gate_desc) == VCPU_DESC_SYS_TYPE_INTGATE_32;
    if (!interrupt_gate && _GetDesc_Type(gate_desc) !=
        VCPU_DESC_SYS_TYPE_TRAPGATE_32)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (!_IsDescPresent(gate_desc))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(_ser_idt_error_code(intid)));
    oldcpl = _GetCPL;
    if (software_origin && _GetDesc_DPL(gate_desc) < oldcpl)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(_ser_idt_error_code(intid)));
    newcs = TYPE_MASK_UNSIGNED_16(_GetDescGate_Selector(gate_desc));
    if (_IsSelectorNull(newcs) || _GetSelector_TI(newcs))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
        TYPE_REFERENCE_OF(code_desc)));
    if (!_IsDescCodeNonConform(code_desc) || _GetDesc_DPL(code_desc) > oldcpl)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    if (_GetDesc_DPL(code_desc) < oldcpl) {
        TYPE_TRACE_CHECK_RETURN(_ser_int_protected_32_outer(context, intid,
            gate_desc, software_origin, error_frame));
        TYPE_TRACE_CALL_END;
        return;
    }
    if (!_IsDescPresent(code_desc))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));

    oldcs = cpu_state.data.cs.selector;
    oldeip = cpu_state.data.eip;
    oldeflags = cpu_state.data.eflags;
    oldcs_frame = oldcs;
    error_code = TYPE_MASK_UNSIGNED_32(instruction_state.data.excode);
    newcs_cache = cpu_state.data.cs;
    TYPE_TRACE_CHECK_RETURN(_ksa_prepare_code_sreg(context, newcs, oldcpl,
        &newcs_cache, &code_desc));
    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &newcs_cache,
        TYPE_MASK_UNSIGNED_32(_GetDescGate_Offset(gate_desc)), 1u, 0,
        oldcpl, 1));
    TYPE_TRACE_CHECK_RETURN(_s_test_ss_push(context, error_frame ? 16u : 12u));

    /* Descriptor access and each frame write are preflighted before state publish. */
    TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
        TYPE_REFERENCE_OF(code_desc)));
    TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldeflags), 4u));
    TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldcs_frame), 4u));
    TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(oldeip), 4u));
    if (error_frame)
        TYPE_TRACE_CHECK_RETURN(_kec_push(context, TYPE_REFERENCE_OF(error_code), 4u));
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.eip = TYPE_MASK_UNSIGNED_32(_GetDescGate_Offset(gate_desc));
    if (interrupt_gate) _ClrEFLAGS_IF;
    _ClrEFLAGS_TF;
    TYPE_TRACE_CALL_END;
}
static C_VOID _ser_int_protected(core_machine_cpu_execution_context *context,
    type_unsigned_8 intid, type_unsigned_8 byte, type_bool software_origin,
    type_bool flagext)
{
    type_unsigned_64 gate_desc;

    TYPE_TRACE_CALL_BEGIN("_ser_int_protected");
    if (!_GetCR0_PE)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (byte != 2u && byte != 4u)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
    if (TYPE_MASK_UNSIGNED_16(intid * 8u + 7u) >
        TYPE_MASK_UNSIGNED_16(cpu_state.data.idtr.limit))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(_ser_idt_error_code(intid)));
    TYPE_TRACE_CHECK_RETURN(_s_read_idt(context, intid,
        TYPE_REFERENCE_OF(gate_desc)));
    /* VM86 admission is limited to a 32-bit interrupt gate. */
    if (_GetEFLAGS_VM && _GetDesc_Type(gate_desc) !=
        VCPU_DESC_SYS_TYPE_INTGATE_32)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(_ser_idt_error_code(intid)));
    /* An unavailable target-stack facility is an architectural task fault. */
    if (_GetEFLAGS_VM && (!cpu_state.data.tr.flagValid ||
        cpu_state.data.tr.sys.type != VCPU_DESC_SYS_TYPE_TSS_32_BUSY))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(0));
    switch (_GetDesc_Type(gate_desc)) {
    case VCPU_DESC_SYS_TYPE_INTGATE_16:
    case VCPU_DESC_SYS_TYPE_TRAPGATE_16:
        TYPE_TRACE_CHECK_RETURN(_ser_int_protected_16(context, intid,
            software_origin, flagext));
        break;
    case VCPU_DESC_SYS_TYPE_INTGATE_32:
    case VCPU_DESC_SYS_TYPE_TRAPGATE_32:
        if (context->cpu_profile < CORE_MACHINE_CPU_PROFILE_80386)
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(_ser_idt_error_code(intid)));
        TYPE_TRACE_CHECK_RETURN(_ser_int_protected_32_same(context, intid,
            gate_desc, software_origin, flagext));
        break;
    case VCPU_DESC_SYS_TYPE_TASKGATE:
        TYPE_TRACE_CHECK_RETURN(_ser_task_gate_descriptor(context, gate_desc,
            _ser_idt_error_code(intid), software_origin, TYPE_TRUE));
        break;
    default:
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(_ser_idt_error_code(intid)));
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _ser_ret_far_real(core_machine_cpu_execution_context *context, type_unsigned_16 newcs, type_unsigned_32 neweip, type_unsigned_16 parambyte, type_unsigned_16 byte)
{
    TYPE_TRACE_CALL_BEGIN("_ser_ret_far_real");
    if (_IsProtected)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CHECK_RETURN(_kec_ret_far(context, newcs, neweip, parambyte, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _ser_ret_far_same(core_machine_cpu_execution_context *context, type_unsigned_16 newcs, type_unsigned_32 neweip, type_unsigned_16 parambyte, type_unsigned_16 byte)
{
    TYPE_TRACE_CALL_BEGIN("_ser_ret_far_same");
    if (!_IsProtected)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CHECK_RETURN(_kec_ret_far(context, newcs, neweip, parambyte, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _ser_ret_far_outer(core_machine_cpu_execution_context *context,
    type_unsigned_16 newcs, type_unsigned_32 neweip, type_unsigned_16 parambyte,
    type_unsigned_16 byte)
{
    type_unsigned_64 code_desc;
    type_unsigned_64 ss_desc;
    type_unsigned_16 newss;
    type_unsigned_32 newesp;
    type_unsigned_32 xs_sel;
    t_cpu_data_sreg newcs_cache;
    t_cpu_data_sreg newss_cache;
    type_unsigned_8 target_cpl;

    TYPE_TRACE_CALL_BEGIN("_ser_ret_far_outer");
    if (!_IsProtected)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    target_cpl = (type_unsigned_8)_GetSelector_RPL(newcs);
    switch (byte)
    {
    case 2:
        TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 4u));
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context,
            (type_unsigned_32)parambyte + 4u, TYPE_REFERENCE_OF(newesp), 2u));
        xs_sel = 0u;
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context,
            (type_unsigned_32)parambyte + 6u, TYPE_REFERENCE_OF(xs_sel), 2u));
        neweip = TYPE_MASK_UNSIGNED_16(neweip);
        newesp = TYPE_MASK_UNSIGNED_16(newesp);
        break;
    case 4:
        TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 8u));
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context,
            (type_unsigned_32)parambyte + 8u, TYPE_REFERENCE_OF(newesp), 4u));
        xs_sel = 0u;
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context,
            (type_unsigned_32)parambyte + 12u, TYPE_REFERENCE_OF(xs_sel), 4u));
        break;
    default:
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        break;
    }
    newss = TYPE_MASK_UNSIGNED_16(xs_sel);
    if (_IsSelectorNull(newss) || _GetSelector_TI(newss) ||
        _GetSelector_RPL(newss) != target_cpl) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newss & 0xfffcu));
    }
    newss_cache = cpu_state.data.ss;
    TYPE_TRACE_CHECK_RETURN(_ksa_prepare_stack_sreg(context, newss, target_cpl,
        &newss_cache, &ss_desc));
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
        TYPE_REFERENCE_OF(code_desc)));
    if (!_IsDescCodeNonConform(code_desc))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    if (!_IsDescPresent(code_desc))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
    newcs_cache = cpu_state.data.cs;
    TYPE_TRACE_CHECK_RETURN(_ksa_prepare_code_sreg(context, newcs, target_cpl,
        &newcs_cache, &code_desc));
    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &newcs_cache,
        neweip, 1u, 0, target_cpl, 1));
    TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newss,
        TYPE_REFERENCE_OF(ss_desc)));
    TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
        TYPE_REFERENCE_OF(code_desc)));
    cpu_state.data.ss = newss_cache;
    if (newss_cache.seg.data.big)
        cpu_state.data.esp = newesp + parambyte;
    else
        cpu_state.data.sp = TYPE_MASK_UNSIGNED_16(newesp + parambyte);
    _MakeCPL(target_cpl);
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.eip = neweip;
    TYPE_TRACE_CALL_END;
}
static C_VOID _ser_jmp_far_real(core_machine_cpu_execution_context *context, type_unsigned_16 newcs, type_unsigned_32 neweip, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_ser_jmp_far_real");
    if (_IsProtected)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CHECK_RETURN(_kec_jmp_far(context, newcs, neweip, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _ser_jmp_far_cs_conf(core_machine_cpu_execution_context *context, type_unsigned_16 newcs, type_unsigned_32 neweip, type_unsigned_8 byte)
{
    type_unsigned_64 descriptor;
    TYPE_TRACE_CALL_BEGIN("_ser_jmp_far_cs_conf");
    if (!_IsProtected)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescCodeConform(descriptor))
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetDesc_DPL(descriptor) > _GetCPL)
    {
        TYPE_TRACE_BLOCK_BEGIN("DPL(>CPL)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs));
        TYPE_TRACE_BLOCK_END;
    }
    if (!_IsDescPresent(descriptor))
    {
        TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CHECK_RETURN(_kec_jmp_far(context, newcs, neweip, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _ser_jmp_far_cs_nonc(core_machine_cpu_execution_context *context, type_unsigned_16 newcs, type_unsigned_32 neweip, type_unsigned_8 byte)
{
    type_unsigned_64 descriptor;
    TYPE_TRACE_CALL_BEGIN("_ser_jmp_far_cs_nonc");
    if (!_IsProtected)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescCodeNonConform(descriptor))
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetDesc_DPL(descriptor) != _GetCPL ||
        _GetSelector_RPL(newcs) > _GetCPL)
    {
        TYPE_TRACE_BLOCK_BEGIN("DPL(!CPL)/RPL(>CPL)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs));
        TYPE_TRACE_BLOCK_END;
    }
    if (!_IsDescPresent(descriptor))
    {
        TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CHECK_RETURN(_kec_jmp_far(context, newcs, neweip, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _ser_jmp_far_call_gate(core_machine_cpu_execution_context *context,
    type_unsigned_16 gate_selector)
{
    type_unsigned_64 gate_desc;
    type_unsigned_64 code_desc;
    type_unsigned_16 target_selector;
    type_unsigned_8 cpl;
    type_bool gate32;
    t_cpu_data_sreg newcs_cache;

    TYPE_TRACE_CALL_BEGIN("_ser_jmp_far_call_gate");
    if (!_IsProtected)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, gate_selector,
        TYPE_REFERENCE_OF(gate_desc)));
    /* The 80386 extends the same-privilege JMP gate route to both 16- and
     * 32-bit call gates.  It transfers directly to the target: no return
     * frame, stack switch, or parameter copy is performed. */
    gate32 = _IsDescCallGate32(gate_desc);
    if (!_IsDescCallGate16(gate_desc) && !gate32)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    cpl = _GetCPL;
    if (_GetDesc_DPL(gate_desc) < cpl ||
        _GetDesc_DPL(gate_desc) < _GetSelector_RPL(gate_selector)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(gate_selector & 0xfffcu));
    }
    if (!_IsDescPresent(gate_desc)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(gate_selector & 0xfffcu));
    }
    target_selector = TYPE_MASK_UNSIGNED_16(_GetDescGate_Selector(gate_desc));
    if (_IsSelectorNull(target_selector) || _GetSelector_TI(target_selector)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(target_selector & 0xfffcu));
    }
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, target_selector,
        TYPE_REFERENCE_OF(code_desc)));
    if (!_IsDescCodeNonConform(code_desc) || _GetDesc_DPL(code_desc) != cpl) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(target_selector & 0xfffcu));
    }
    if (!_IsDescPresent(code_desc)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(target_selector & 0xfffcu));
    }
    newcs_cache = cpu_state.data.cs;
    TYPE_TRACE_CHECK_RETURN(_ksa_prepare_code_sreg(context, target_selector,
        cpl, &newcs_cache, &code_desc));
    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &newcs_cache,
        gate32 ? TYPE_MASK_UNSIGNED_32(_GetDescGate_Offset(gate_desc)) :
        TYPE_MASK_UNSIGNED_16(_GetDescGate_Offset(gate_desc)), 1u, 0, cpl, 1));
    TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, target_selector,
        TYPE_REFERENCE_OF(code_desc)));
    cpu_state.data.cs = newcs_cache;
    if (gate32) cpu_state.data.eip = TYPE_MASK_UNSIGNED_32(
        _GetDescGate_Offset(gate_desc));
    else cpu_state.data.ip = TYPE_MASK_UNSIGNED_16(_GetDescGate_Offset(gate_desc));
    TYPE_TRACE_CALL_END;
}
static C_VOID _ser_jmp_far_task_gate(core_machine_cpu_execution_context *context,
    type_unsigned_16 newcs)
{
    TYPE_TRACE_CALL_BEGIN("_ser_jmp_far_task_gate");
    TYPE_TRACE_CHECK_RETURN(_ser_task_gate_target(context, newcs, TYPE_FALSE));
    TYPE_TRACE_CALL_END;
}
typedef struct task_switch_state_16 {
    type_unsigned_16 ip;
    type_unsigned_16 flags;
    type_unsigned_16 ax;
    type_unsigned_16 cx;
    type_unsigned_16 dx;
    type_unsigned_16 bx;
    type_unsigned_16 sp;
    type_unsigned_16 bp;
    type_unsigned_16 si;
    type_unsigned_16 di;
    type_unsigned_16 es;
    type_unsigned_16 cs;
    type_unsigned_16 ss;
    type_unsigned_16 ds;
    type_unsigned_16 ldtr;
} task_switch_state_16;

static C_VOID _s_task_cache_descriptor(t_cpu_data_sreg *cache,
    type_unsigned_16 selector, type_unsigned_64 descriptor,
    t_cpu_data_sreg_type sregtype)
{
    TYPE_TRACE_CALL_BEGIN("_s_task_cache_descriptor");
    STD_MEMSET(cache, 0, sizeof(*cache));
    cache->flagValid = TYPE_TRUE;
    cache->selector = selector;
    cache->sregtype = sregtype;
    cache->base = (type_unsigned_32)_GetDescSeg_Base(descriptor);
    cache->limit = (type_unsigned_32)(_IsDescSegGranularLarge(descriptor) ?
        (_GetDescSeg_Limit(descriptor) << 12u | 0x0fffu) :
        _GetDescSeg_Limit(descriptor));
    cache->dpl = (type_unsigned_4)_GetDesc_DPL(descriptor);
    if (sregtype == SREG_TR || sregtype == SREG_LDTR) {
        cache->sys.type = (type_unsigned_4)_GetDesc_Type(descriptor);
    } else if (_IsDescCode(descriptor)) {
        cache->seg.accessed = (type_bool)_IsDescUserAccessed(descriptor);
        cache->seg.executable = TYPE_TRUE;
        cache->seg.exec.defsize = (type_bool)_IsDescCode32(descriptor);
        cache->seg.exec.conform = (type_bool)_IsDescCodeConform(descriptor);
        cache->seg.exec.readable = (type_bool)_IsDescCodeReadable(descriptor);
    } else {
        cache->seg.accessed = (type_bool)_IsDescUserAccessed(descriptor);
        cache->seg.executable = TYPE_FALSE;
        cache->seg.data.big = (type_bool)_IsDescDataBig(descriptor);
        cache->seg.data.expdown = (type_bool)_IsDescDataExpDown(descriptor);
        cache->seg.data.writable = (type_bool)_IsDescDataWritable(descriptor);
    }
    TYPE_TRACE_CALL_END;
}

static C_VOID _s_task_prepare_ldtr(core_machine_cpu_execution_context *context,
    type_unsigned_16 selector, t_cpu_data_sreg *out_cache)
{
    type_unsigned_64 descriptor;

    TYPE_TRACE_CALL_BEGIN("_s_task_prepare_ldtr");
    STD_MEMSET(out_cache, 0, sizeof(*out_cache));
    out_cache->sregtype = SREG_LDTR;
    if (_IsSelectorNull(selector)) {
        out_cache->selector = selector;
        TYPE_TRACE_CALL_END;
        return;
    }
    if (_GetSelector_TI(selector))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(selector & 0xfffcu));
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, selector,
        TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescLDT(descriptor))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(selector & 0xfffcu));
    if (!_IsDescPresent(descriptor))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(selector & 0xfffcu));
    _s_task_cache_descriptor(out_cache, selector, descriptor, SREG_LDTR);
    TYPE_TRACE_CALL_END;
}

static C_VOID _s_task_read_selector(core_machine_cpu_execution_context *context,
    t_cpu_data_sreg *ldtr, type_unsigned_16 selector,
    type_virtual_address rdata)
{
    TYPE_TRACE_CALL_BEGIN("_s_task_read_selector");
    if (!_GetSelector_TI(selector)) {
        TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, selector, rdata));
        TYPE_TRACE_CALL_END;
        return;
    }
    if (!ldtr->flagValid || TYPE_MASK_UNSIGNED_16(_GetSelector_Offset(selector) +
        7u) > TYPE_MASK_UNSIGNED_16(ldtr->limit))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(selector & 0xfffcu));
    TYPE_TRACE_CHECK_RETURN(_kma_read_logical(context, ldtr,
        _GetSelector_Offset(selector), rdata, 8u, 0u, TYPE_TRUE));
    TYPE_TRACE_CALL_END;
}

static C_VOID _s_task_validate_data_selector(
    core_machine_cpu_execution_context *context, t_cpu_data_sreg *ldtr,
    type_unsigned_16 selector, t_cpu_data_sreg_type sregtype,
    t_cpu_data_sreg *out_cache)
{
    type_unsigned_64 descriptor;

    TYPE_TRACE_CALL_BEGIN("_s_task_validate_data_selector");
    if (_IsSelectorNull(selector)) {
        if (sregtype == SREG_STACK)
            TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(0));
        STD_MEMSET(out_cache, 0, sizeof(*out_cache));
        out_cache->selector = selector;
        out_cache->sregtype = sregtype;
        TYPE_TRACE_CALL_END;
        return;
    }
    if (_GetSelector_RPL(selector) != 0u) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(selector & 0xfffcu));
    }
    TYPE_TRACE_CHECK_RETURN(_s_task_read_selector(context, ldtr, selector,
        TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescDataWritable(descriptor) || _GetDesc_DPL(descriptor) != 0u) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(selector & 0xfffcu));
    }
    if (!_IsDescPresent(descriptor)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(selector & 0xfffcu));
    }
    _s_task_cache_descriptor(out_cache, selector, descriptor, sregtype);
    TYPE_TRACE_CALL_END;
}

static C_VOID _s_task_validate_code_selector(
    core_machine_cpu_execution_context *context, t_cpu_data_sreg *ldtr,
    type_unsigned_16 selector, type_unsigned_32 eip, t_cpu_data_sreg *out_cache)
{
    type_unsigned_64 descriptor;

    TYPE_TRACE_CALL_BEGIN("_s_task_validate_code_selector");
    if (_IsSelectorNull(selector) || _GetSelector_RPL(selector) != 0u) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(selector & 0xfffcu));
    }
    TYPE_TRACE_CHECK_RETURN(_s_task_read_selector(context, ldtr, selector,
        TYPE_REFERENCE_OF(descriptor)));
    if (!_IsDescCodeNonConform(descriptor) || _GetDesc_DPL(descriptor) != 0u) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(selector & 0xfffcu));
    }
    if (!_IsDescPresent(descriptor)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(selector & 0xfffcu));
    }
    _s_task_cache_descriptor(out_cache, selector, descriptor, SREG_CODE);
    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, out_cache, eip, 1u,
        TYPE_FALSE, 0u, TYPE_TRUE));
    TYPE_TRACE_CALL_END;
}

typedef struct task_switch_sreg_32 {
    type_unsigned_16 selector;
    type_unsigned_16 reserved;
} task_switch_sreg_32;

#define TASK_SWITCH_TSS32_CR3_OFFSET 0x1cu
#define TASK_SWITCH_TSS_BACKLINK_OFFSET 0x00u
#define TASK_SWITCH_TSS32_EIP_OFFSET 0x20u
#define TASK_SWITCH_TSS32_EFLAGS_OFFSET 0x24u
#define TASK_SWITCH_TSS32_EAX_OFFSET 0x28u
#define TASK_SWITCH_TSS32_ECX_OFFSET 0x2cu
#define TASK_SWITCH_TSS32_EDX_OFFSET 0x30u
#define TASK_SWITCH_TSS32_EBX_OFFSET 0x34u
#define TASK_SWITCH_TSS32_ESP_OFFSET 0x38u
#define TASK_SWITCH_TSS32_EBP_OFFSET 0x3cu
#define TASK_SWITCH_TSS32_ESI_OFFSET 0x40u
#define TASK_SWITCH_TSS32_EDI_OFFSET 0x44u
#define TASK_SWITCH_TSS32_ES_OFFSET 0x48u
#define TASK_SWITCH_TSS32_CS_OFFSET 0x4cu
#define TASK_SWITCH_TSS32_SS_OFFSET 0x50u
#define TASK_SWITCH_TSS32_DS_OFFSET 0x54u
#define TASK_SWITCH_TSS32_FS_OFFSET 0x58u
#define TASK_SWITCH_TSS32_GS_OFFSET 0x5cu
#define TASK_SWITCH_TSS32_LDTR_OFFSET 0x60u
#define TASK_SWITCH_TSS32_DEBUG_TRAP_OFFSET 0x64u
#define TASK_SWITCH_TSS32_STATE_BYTES 0x48u
#define TASK_SWITCH_TSS32_INCOMING_BYTES 0x4au

typedef struct task_switch_state_32 {
    type_unsigned_32 cr3;
    type_unsigned_32 eip;
    type_unsigned_32 eflags;
    type_unsigned_32 eax;
    type_unsigned_32 ecx;
    type_unsigned_32 edx;
    type_unsigned_32 ebx;
    type_unsigned_32 esp;
    type_unsigned_32 ebp;
    type_unsigned_32 esi;
    type_unsigned_32 edi;
    task_switch_sreg_32 es;
    task_switch_sreg_32 cs;
    task_switch_sreg_32 ss;
    task_switch_sreg_32 ds;
    task_switch_sreg_32 fs;
    task_switch_sreg_32 gs;
    task_switch_sreg_32 ldtr;
} task_switch_state_32;

_Static_assert(sizeof(task_switch_sreg_32) == 4u,
    "80386 TSS selector slots are four bytes");
_Static_assert(sizeof(task_switch_state_32) == TASK_SWITCH_TSS32_STATE_BYTES,
    "80386 TSS saved-state image spans 0x1c through 0x63");
_Static_assert(offsetof(task_switch_state_32, cr3) == 0u &&
    offsetof(task_switch_state_32, eip) == 4u &&
    offsetof(task_switch_state_32, eflags) == 8u &&
    offsetof(task_switch_state_32, eax) == 12u &&
    offsetof(task_switch_state_32, ecx) == 16u &&
    offsetof(task_switch_state_32, edx) == 20u &&
    offsetof(task_switch_state_32, ebx) == 24u &&
    offsetof(task_switch_state_32, esp) == 28u &&
    offsetof(task_switch_state_32, ebp) == 32u &&
    offsetof(task_switch_state_32, esi) == 36u &&
    offsetof(task_switch_state_32, edi) == 40u &&
    offsetof(task_switch_state_32, es) == 44u &&
    offsetof(task_switch_state_32, cs) == 48u &&
    offsetof(task_switch_state_32, ss) == 52u &&
    offsetof(task_switch_state_32, ds) == 56u &&
    offsetof(task_switch_state_32, fs) == 60u &&
    offsetof(task_switch_state_32, gs) == 64u &&
    offsetof(task_switch_state_32, ldtr) == 68u,
    "80386 TSS image fields retain their named offsets");

static C_VOID _e_except_n(core_machine_cpu_execution_context *context,
    type_unsigned_8 exid, type_unsigned_8 byte);

static C_VOID _s_task_write_state_32(core_machine_cpu_execution_context *context,
    const task_switch_state_32 *state)
{
    TYPE_TRACE_CALL_BEGIN("_s_task_write_state_32");
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_CR3_OFFSET,
        TYPE_REFERENCE_OF(state->cr3), 4u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_EIP_OFFSET,
        TYPE_REFERENCE_OF(state->eip), 4u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_EFLAGS_OFFSET,
        TYPE_REFERENCE_OF(state->eflags), 4u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_EAX_OFFSET,
        TYPE_REFERENCE_OF(state->eax), 4u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_ECX_OFFSET,
        TYPE_REFERENCE_OF(state->ecx), 4u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_EDX_OFFSET,
        TYPE_REFERENCE_OF(state->edx), 4u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_EBX_OFFSET,
        TYPE_REFERENCE_OF(state->ebx), 4u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_ESP_OFFSET,
        TYPE_REFERENCE_OF(state->esp), 4u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_EBP_OFFSET,
        TYPE_REFERENCE_OF(state->ebp), 4u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_ESI_OFFSET,
        TYPE_REFERENCE_OF(state->esi), 4u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_EDI_OFFSET,
        TYPE_REFERENCE_OF(state->edi), 4u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_ES_OFFSET,
        TYPE_REFERENCE_OF(state->es.selector), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_CS_OFFSET,
        TYPE_REFERENCE_OF(state->cs.selector), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_SS_OFFSET,
        TYPE_REFERENCE_OF(state->ss.selector), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_DS_OFFSET,
        TYPE_REFERENCE_OF(state->ds.selector), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_FS_OFFSET,
        TYPE_REFERENCE_OF(state->fs.selector), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_GS_OFFSET,
        TYPE_REFERENCE_OF(state->gs.selector), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_LDTR_OFFSET,
        TYPE_REFERENCE_OF(state->ldtr.selector), 2u));
    TYPE_TRACE_CALL_END;
}
static C_VOID _s_task_write_state_16(core_machine_cpu_execution_context *context,
    const task_switch_state_16 *state)
{
    TYPE_TRACE_CALL_BEGIN("_s_task_write_state_16");
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x0eu,
        TYPE_REFERENCE_OF(state->ip), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x10u,
        TYPE_REFERENCE_OF(state->flags), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x12u,
        TYPE_REFERENCE_OF(state->ax), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x14u,
        TYPE_REFERENCE_OF(state->cx), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x16u,
        TYPE_REFERENCE_OF(state->dx), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x18u,
        TYPE_REFERENCE_OF(state->bx), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x1au,
        TYPE_REFERENCE_OF(state->sp), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x1cu,
        TYPE_REFERENCE_OF(state->bp), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x1eu,
        TYPE_REFERENCE_OF(state->si), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x20u,
        TYPE_REFERENCE_OF(state->di), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x22u,
        TYPE_REFERENCE_OF(state->es), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x24u,
        TYPE_REFERENCE_OF(state->cs), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x26u,
        TYPE_REFERENCE_OF(state->ss), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x28u,
        TYPE_REFERENCE_OF(state->ds), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x2au,
        TYPE_REFERENCE_OF(state->ldtr), 2u));
    TYPE_TRACE_CALL_END;
}

/* A task switch may save a 286-format TSS and load a 386-format TSS, or the
 * reverse.  The processor selects each image from its own descriptor; it does
 * not require the outgoing and incoming TSS formats to match.  Keep the two
 * architectural layouts explicit, but stage their common transition before
 * the first descriptor, TSS, or CPU-state publication. */
static C_VOID _ser_task_transition_tss_plan(
    core_machine_cpu_execution_context *context, type_unsigned_16 newcs,
    type_bool nested, type_bool returning, type_bool old_is_32,
    type_bool new_is_32)
{
    type_unsigned_64 old_descriptor;
    type_unsigned_64 new_descriptor;
    t_cpu_data_sreg newtr;
    t_cpu_data_sreg newldtr;
    t_cpu_data_sreg newcs_cache;
    t_cpu_data_sreg newss_cache;
    t_cpu_data_sreg newds_cache;
    t_cpu_data_sreg newes_cache;
    t_cpu_data_sreg newfs_cache;
    t_cpu_data_sreg newgs_cache;
    task_switch_state_16 incoming16;
    task_switch_state_16 outgoing16;
    task_switch_state_32 incoming32;
    task_switch_state_32 outgoing32;
    type_unsigned_16 backlink;
    type_unsigned_16 debug_trap = 0u;

    TYPE_TRACE_CALL_BEGIN("_ser_task_transition_tss_plan");
    if (!_IsProtected || context->cpu_profile < CORE_MACHINE_CPU_PROFILE_80386 ||
        _GetCPL || _GetSelector_TI(newcs) || _GetSelector_RPL(newcs) != 0u)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    if (!cpu_state.data.tr.flagValid || _GetSelector_TI(cpu_state.data.tr.selector))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(0));
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, cpu_state.data.tr.selector,
        TYPE_REFERENCE_OF(old_descriptor)));
    if ((old_is_32 && !_IsDescTSS32Busy(old_descriptor)) ||
        (!old_is_32 && !_IsDescTSS16Busy(old_descriptor)) ||
        !_IsDescPresent(old_descriptor) || cpu_state.data.tr.limit <
        (old_is_32 ? 0x67u : 0x2bu))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(cpu_state.data.tr.selector & 0xfffcu));
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
        TYPE_REFERENCE_OF(new_descriptor)));
    if ((new_is_32 && ((returning && !_IsDescTSS32Busy(new_descriptor)) ||
        (!returning && (!_IsDescTSS32Avl(new_descriptor) ||
            _IsDescTSS32Busy(new_descriptor))))) ||
        (!new_is_32 && ((returning && !_IsDescTSS16Busy(new_descriptor)) ||
        (!returning && (!_IsDescTSS16Avl(new_descriptor) ||
            _IsDescTSS16Busy(new_descriptor))))) ||
        _GetDesc_DPL(new_descriptor) != 0u)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    if (!_IsDescPresent(new_descriptor))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
    _s_task_cache_descriptor(&newtr, newcs, new_descriptor, SREG_TR);
    if (newtr.limit < (new_is_32 ? 0x67u : 0x2bu))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(newcs & 0xfffcu));

    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &cpu_state.data.tr,
        old_is_32 ? TASK_SWITCH_TSS32_CR3_OFFSET : 0x0eu,
        old_is_32 ? TASK_SWITCH_TSS32_STATE_BYTES : sizeof(outgoing16),
        TYPE_TRUE, 0u, TYPE_TRUE));
    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &newtr,
        new_is_32 ? TASK_SWITCH_TSS32_CR3_OFFSET : 0x0eu,
        new_is_32 ? TASK_SWITCH_TSS32_INCOMING_BYTES : sizeof(incoming16),
        TYPE_FALSE, 0u, TYPE_TRUE));
    if (nested)
        TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &newtr,
            TASK_SWITCH_TSS_BACKLINK_OFFSET, 2u, TYPE_TRUE, 0u, TYPE_TRUE));
    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &cpu_state.data.gdtr,
        _GetSelector_Offset(cpu_state.data.tr.selector), 8u, TYPE_TRUE, 0u,
        TYPE_TRUE));
    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &cpu_state.data.gdtr,
        _GetSelector_Offset(newcs), 8u, TYPE_TRUE, 0u, TYPE_TRUE));
    if (new_is_32) {
        TYPE_TRACE_CHECK_RETURN(_kma_read_logical(context, &newtr,
            TASK_SWITCH_TSS32_CR3_OFFSET, TYPE_REFERENCE_OF(incoming32),
            sizeof(incoming32), 0u, TYPE_TRUE));
        TYPE_TRACE_CHECK_RETURN(_kma_read_logical(context, &newtr,
            TASK_SWITCH_TSS32_DEBUG_TRAP_OFFSET, TYPE_REFERENCE_OF(debug_trap),
            2u, 0u, TYPE_TRUE));
        if (incoming32.cr3 & ~VCPU_CR3_BASE)
            TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(newcs & 0xfffcu));
        TYPE_TRACE_CHECK_RETURN(_s_task_prepare_ldtr(context,
            incoming32.ldtr.selector, &newldtr));
        TYPE_TRACE_CHECK_RETURN(_s_task_validate_code_selector(context,
            &newldtr, incoming32.cs.selector, incoming32.eip, &newcs_cache));
        TYPE_TRACE_CHECK_RETURN(_s_task_validate_data_selector(context,
            &newldtr, incoming32.ss.selector, SREG_STACK, &newss_cache));
        TYPE_TRACE_CHECK_RETURN(_s_task_validate_data_selector(context,
            &newldtr, incoming32.ds.selector, SREG_DATA, &newds_cache));
        TYPE_TRACE_CHECK_RETURN(_s_task_validate_data_selector(context,
            &newldtr, incoming32.es.selector, SREG_DATA, &newes_cache));
        TYPE_TRACE_CHECK_RETURN(_s_task_validate_data_selector(context,
            &newldtr, incoming32.fs.selector, SREG_DATA, &newfs_cache));
        TYPE_TRACE_CHECK_RETURN(_s_task_validate_data_selector(context,
            &newldtr, incoming32.gs.selector, SREG_DATA, &newgs_cache));
        TYPE_TRACE_CHECK_RETURN(_m_test_logical(context, &newss_cache,
            incoming32.esp, 0u, TYPE_FALSE));
    } else {
        TYPE_TRACE_CHECK_RETURN(_kma_read_logical(context, &newtr, 0x0eu,
            TYPE_REFERENCE_OF(incoming16), sizeof(incoming16), 0u, TYPE_TRUE));
        TYPE_TRACE_CHECK_RETURN(_s_task_prepare_ldtr(context, incoming16.ldtr,
            &newldtr));
        TYPE_TRACE_CHECK_RETURN(_s_task_validate_code_selector(context,
            &newldtr, incoming16.cs, incoming16.ip, &newcs_cache));
        TYPE_TRACE_CHECK_RETURN(_s_task_validate_data_selector(context,
            &newldtr, incoming16.ss, SREG_STACK, &newss_cache));
        TYPE_TRACE_CHECK_RETURN(_s_task_validate_data_selector(context,
            &newldtr, incoming16.ds, SREG_DATA, &newds_cache));
        TYPE_TRACE_CHECK_RETURN(_s_task_validate_data_selector(context,
            &newldtr, incoming16.es, SREG_DATA, &newes_cache));
    }

    if (old_is_32) {
        STD_MEMSET(&outgoing32, 0, sizeof(outgoing32));
        outgoing32.cr3 = cpu_state.data.cr3;
        outgoing32.eip = cpu_state.data.eip;
        outgoing32.eflags = cpu_state.data.eflags;
        outgoing32.eax = cpu_state.data.eax;
        outgoing32.ecx = cpu_state.data.ecx;
        outgoing32.edx = cpu_state.data.edx;
        outgoing32.ebx = cpu_state.data.ebx;
        outgoing32.esp = cpu_state.data.esp;
        outgoing32.ebp = cpu_state.data.ebp;
        outgoing32.esi = cpu_state.data.esi;
        outgoing32.edi = cpu_state.data.edi;
        outgoing32.es.selector = cpu_state.data.es.selector;
        outgoing32.cs.selector = cpu_state.data.cs.selector;
        outgoing32.ss.selector = cpu_state.data.ss.selector;
        outgoing32.ds.selector = cpu_state.data.ds.selector;
        outgoing32.fs.selector = cpu_state.data.fs.selector;
        outgoing32.gs.selector = cpu_state.data.gs.selector;
        outgoing32.ldtr.selector = cpu_state.data.ldtr.selector;
    } else {
        outgoing16.ip = cpu_state.data.ip;
        outgoing16.flags = cpu_state.data.flags;
        outgoing16.ax = cpu_state.data.ax;
        outgoing16.cx = cpu_state.data.cx;
        outgoing16.dx = cpu_state.data.dx;
        outgoing16.bx = cpu_state.data.bx;
        outgoing16.sp = cpu_state.data.sp;
        outgoing16.bp = cpu_state.data.bp;
        outgoing16.si = cpu_state.data.si;
        outgoing16.di = cpu_state.data.di;
        outgoing16.es = cpu_state.data.es.selector;
        outgoing16.cs = cpu_state.data.cs.selector;
        outgoing16.ss = cpu_state.data.ss.selector;
        outgoing16.ds = cpu_state.data.ds.selector;
        outgoing16.ldtr = cpu_state.data.ldtr.selector;
    }
    backlink = cpu_state.data.tr.selector;
    if (!nested)
        _ClrDescTSSBusy(old_descriptor);
    if (!returning)
        _SetDescTSSBusy(new_descriptor);

    if (old_is_32) {
        TYPE_TRACE_CHECK_RETURN(_s_task_write_state_32(context, &outgoing32));
    } else
        TYPE_TRACE_CHECK_RETURN(_s_task_write_state_16(context, &outgoing16));
    if (nested)
        TYPE_TRACE_CHECK_RETURN(_kma_write_logical(context, &newtr,
            TASK_SWITCH_TSS_BACKLINK_OFFSET, TYPE_REFERENCE_OF(backlink), 2u,
            0u, TYPE_TRUE));
    TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, cpu_state.data.tr.selector,
        TYPE_REFERENCE_OF(old_descriptor)));
    TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newtr.selector,
        TYPE_REFERENCE_OF(new_descriptor)));
    if (new_is_32) {
        cpu_state.data.cr3 = incoming32.cr3;
        cpu_state.data.eip = incoming32.eip;
        cpu_state.data.eflags = _e_eflags_load(context, incoming32.eflags);
        if (nested) _SetEFLAGS_NT;
        cpu_state.data.eax = incoming32.eax;
        cpu_state.data.ecx = incoming32.ecx;
        cpu_state.data.edx = incoming32.edx;
        cpu_state.data.ebx = incoming32.ebx;
        cpu_state.data.esp = incoming32.esp;
        cpu_state.data.ebp = incoming32.ebp;
        cpu_state.data.esi = incoming32.esi;
        cpu_state.data.edi = incoming32.edi;
        cpu_state.data.es = newes_cache;
        cpu_state.data.cs = newcs_cache;
        cpu_state.data.ss = newss_cache;
        cpu_state.data.ds = newds_cache;
        cpu_state.data.fs = newfs_cache;
        cpu_state.data.gs = newgs_cache;
    } else {
        cpu_state.data.eax = 0xffff0000u | incoming16.ax;
        cpu_state.data.ecx = 0xffff0000u | incoming16.cx;
        cpu_state.data.edx = 0xffff0000u | incoming16.dx;
        cpu_state.data.ebx = 0xffff0000u | incoming16.bx;
        cpu_state.data.esp = 0xffff0000u | incoming16.sp;
        cpu_state.data.ebp = 0xffff0000u | incoming16.bp;
        cpu_state.data.esi = 0xffff0000u | incoming16.si;
        cpu_state.data.edi = 0xffff0000u | incoming16.di;
        cpu_state.data.eip = incoming16.ip;
        cpu_state.data.eflags = _e_eflags_load(context, incoming16.flags);
        if (nested) _SetEFLAGS_NT;
        cpu_state.data.es = newes_cache;
        cpu_state.data.cs = newcs_cache;
        cpu_state.data.ss = newss_cache;
        cpu_state.data.ds = newds_cache;
        STD_MEMSET(&cpu_state.data.fs, 0, sizeof(cpu_state.data.fs));
        cpu_state.data.fs.sregtype = SREG_DATA;
        STD_MEMSET(&cpu_state.data.gs, 0, sizeof(cpu_state.data.gs));
        cpu_state.data.gs.sregtype = SREG_DATA;
    }
    cpu_state.data.ldtr = newldtr;
    newtr.sys.type = new_is_32 ? VCPU_DESC_SYS_TYPE_TSS_32_BUSY :
        VCPU_DESC_SYS_TYPE_TSS_16_BUSY;
    cpu_state.data.tr = newtr;
    if (new_is_32) cpu_state.data.dr7 &= ~VCPU_DR7_LOCAL_ENABLE_MASK;
    _SetCR0_TS;
    if (new_is_32 && TYPE_GET_LSB(debug_trap)) {
        t_cpu trap_cpu = cpu_state;

        cpu_state.data.dr6 |= VCPU_DR6_BT;
        TYPE_TRACE_CHECK_RETURN(_e_except_n(context, 0x01u, _GetOperandSize));
        if (context->diagnostic_provider != STD_NULL &&
            context->diagnostic_provider->record_delivered_exception != STD_NULL) {
            t_cpuins diagnostic_instructions = instruction_state;

            /* The task-switch #DB is a trap at the incoming task's EIP.  It is
             * delivered outside ExecFinal(), so preserve that point for the
             * fault-origin diagnostic contract without changing the current
             * instruction's saved oldcpu. */
            diagnostic_instructions.data.oldcpu = trap_cpu;
            diagnostic_instructions.data.except = VCPUINS_EXCEPT_DB;
            diagnostic_instructions.data.excode = 0u;
            context->diagnostic_provider->record_delivered_exception(
                context->diagnostic_context, &trap_cpu, &diagnostic_instructions);
        }
    }
    TYPE_TRACE_CALL_END;
}

static C_VOID _ser_task_transition_tss(core_machine_cpu_execution_context *context,
    type_unsigned_16 newcs, type_bool nested, type_bool returning)
{
    type_unsigned_64 old_descriptor;
    type_unsigned_64 new_descriptor;
    t_cpu_data_sreg newtr;
    t_cpu_data_sreg newldtr;
    t_cpu_data_sreg newcs_cache;
    t_cpu_data_sreg newss_cache;
    t_cpu_data_sreg newds_cache;
    t_cpu_data_sreg newes_cache;
    task_switch_state_16 state;
    type_unsigned_64 cross_descriptor;
    type_bool old_is_32;
    type_bool new_is_32;

    TYPE_TRACE_CALL_BEGIN("_ser_task_transition_tss");
    old_is_32 = context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386 &&
        cpu_state.data.tr.flagValid && cpu_state.data.tr.sys.type ==
        VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386 &&
        cpu_state.data.tr.flagValid && !_GetSelector_TI(newcs) &&
        _GetSelector_RPL(newcs) == 0u) {
        TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
            TYPE_REFERENCE_OF(cross_descriptor)));
        new_is_32 = _IsDescTSS32Avl(cross_descriptor) ||
            _IsDescTSS32Busy(cross_descriptor);
        TYPE_TRACE_CHECK_RETURN(_ser_task_transition_tss_plan(context,
            newcs, nested, returning, old_is_32, new_is_32));
        TYPE_TRACE_CALL_END;
        return;
    }
    if (!_IsProtected)
        TYPE_TRACE_IMPOSSIBLE_RETURN;
    if (_GetCPL || _GetSelector_TI(newcs) || _GetSelector_RPL(newcs) != 0u) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    }
    if (!cpu_state.data.tr.flagValid || _GetSelector_TI(cpu_state.data.tr.selector)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(0));
    }
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, cpu_state.data.tr.selector,
        TYPE_REFERENCE_OF(old_descriptor)));
    if (!_IsDescTSS16Busy(old_descriptor) || !_IsDescPresent(old_descriptor)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(cpu_state.data.tr.selector & 0xfffcu));
    }
    if (cpu_state.data.tr.limit < 0x29u) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(cpu_state.data.tr.selector & 0xfffcu));
    }
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
        TYPE_REFERENCE_OF(new_descriptor)));
    if ((returning && !_IsDescTSS16Busy(new_descriptor)) ||
        (!returning && (!_IsDescTSS16Avl(new_descriptor) ||
            _IsDescTSS16Busy(new_descriptor))) ||
        _GetDesc_DPL(new_descriptor) != 0u) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    }
    if (!_IsDescPresent(new_descriptor)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
    }
    _s_task_cache_descriptor(&newtr, newcs, new_descriptor, SREG_TR);
    if (newtr.limit < 0x2bu) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(newcs & 0xfffcu));
    }
    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &cpu_state.data.tr,
        0x0eu, sizeof(state), TYPE_TRUE, 0u, TYPE_TRUE));
    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &newtr, 0x0eu, 0x1eu,
        TYPE_FALSE, 0u, TYPE_TRUE));
    if (nested)
        TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &newtr,
            TASK_SWITCH_TSS_BACKLINK_OFFSET, 2u, TYPE_TRUE, 0u, TYPE_TRUE));
    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &cpu_state.data.gdtr,
        _GetSelector_Offset(cpu_state.data.tr.selector), 8u, TYPE_TRUE, 0u,
        TYPE_TRUE));
    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &cpu_state.data.gdtr,
        _GetSelector_Offset(newcs), 8u, TYPE_TRUE, 0u, TYPE_TRUE));
    TYPE_TRACE_CHECK_RETURN(_kma_read_logical(context, &newtr, 0x0eu,
        TYPE_REFERENCE_OF(state), sizeof(state), 0u, TYPE_TRUE));
    TYPE_TRACE_CHECK_RETURN(_s_task_prepare_ldtr(context, state.ldtr,
        &newldtr));
    TYPE_TRACE_CHECK_RETURN(_s_task_validate_code_selector(context, &newldtr,
        state.cs, state.ip, &newcs_cache));
    TYPE_TRACE_CHECK_RETURN(_s_task_validate_data_selector(context, &newldtr,
        state.ss, SREG_STACK, &newss_cache));
    TYPE_TRACE_CHECK_RETURN(_s_task_validate_data_selector(context, &newldtr,
        state.ds, SREG_DATA, &newds_cache));
    TYPE_TRACE_CHECK_RETURN(_s_task_validate_data_selector(context, &newldtr,
        state.es, SREG_DATA, &newes_cache));

    if (!nested)
        _ClrDescTSSBusy(old_descriptor);
    if (!returning)
        _SetDescTSSBusy(new_descriptor);
    if (nested)
        TYPE_TRACE_CHECK_RETURN(_kma_write_logical(context, &newtr,
            TASK_SWITCH_TSS_BACKLINK_OFFSET,
            TYPE_REFERENCE_OF(cpu_state.data.tr.selector), 2u, 0u,
            TYPE_TRUE));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x0eu,
        TYPE_REFERENCE_OF(cpu_state.data.ip), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x10u,
        TYPE_REFERENCE_OF(cpu_state.data.flags), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x12u,
        TYPE_REFERENCE_OF(cpu_state.data.ax), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x14u,
        TYPE_REFERENCE_OF(cpu_state.data.cx), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x16u,
        TYPE_REFERENCE_OF(cpu_state.data.dx), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x18u,
        TYPE_REFERENCE_OF(cpu_state.data.bx), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x1au,
        TYPE_REFERENCE_OF(cpu_state.data.sp), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x1cu,
        TYPE_REFERENCE_OF(cpu_state.data.bp), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x1eu,
        TYPE_REFERENCE_OF(cpu_state.data.si), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x20u,
        TYPE_REFERENCE_OF(cpu_state.data.di), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x22u,
        TYPE_REFERENCE_OF(cpu_state.data.es.selector), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x24u,
        TYPE_REFERENCE_OF(cpu_state.data.cs.selector), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x26u,
        TYPE_REFERENCE_OF(cpu_state.data.ss.selector), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x28u,
        TYPE_REFERENCE_OF(cpu_state.data.ds.selector), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_tss(context, 0x2au,
        TYPE_REFERENCE_OF(cpu_state.data.ldtr.selector), 2u));
    TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, cpu_state.data.tr.selector,
        TYPE_REFERENCE_OF(old_descriptor)));
    TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
        TYPE_REFERENCE_OF(new_descriptor)));

    cpu_state.data.eax = context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386 ?
        0xffff0000u | state.ax : state.ax;
    cpu_state.data.ecx = context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386 ?
        0xffff0000u | state.cx : state.cx;
    cpu_state.data.edx = context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386 ?
        0xffff0000u | state.dx : state.dx;
    cpu_state.data.ebx = context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386 ?
        0xffff0000u | state.bx : state.bx;
    cpu_state.data.esp = context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386 ?
        0xffff0000u | state.sp : state.sp;
    cpu_state.data.ebp = context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386 ?
        0xffff0000u | state.bp : state.bp;
    cpu_state.data.esi = context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386 ?
        0xffff0000u | state.si : state.si;
    cpu_state.data.edi = context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386 ?
        0xffff0000u | state.di : state.di;
    cpu_state.data.eip = state.ip;
    cpu_state.data.eflags = _e_eflags_load(context, state.flags);
    if (nested)
        _SetEFLAGS_NT;
    cpu_state.data.es = newes_cache;
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.ss = newss_cache;
    cpu_state.data.ds = newds_cache;
    cpu_state.data.ldtr = newldtr;
    STD_MEMSET(&cpu_state.data.fs, 0, sizeof(cpu_state.data.fs));
    cpu_state.data.fs.sregtype = SREG_DATA;
    STD_MEMSET(&cpu_state.data.gs, 0, sizeof(cpu_state.data.gs));
    cpu_state.data.gs.sregtype = SREG_DATA;
    newtr.sys.type = VCPU_DESC_SYS_TYPE_TSS_16_BUSY;
    cpu_state.data.tr = newtr;
    _SetCR0_TS;
    TYPE_TRACE_CALL_END;
}

static C_VOID _ser_task_switch_tss(core_machine_cpu_execution_context *context,
    type_unsigned_16 newcs, type_bool nested)
{
    TYPE_TRACE_CALL_BEGIN("_ser_task_switch_tss");
    TYPE_TRACE_CHECK_RETURN(_ser_task_transition_tss(context, newcs, nested,
        TYPE_FALSE));
    TYPE_TRACE_CALL_END;
}

static C_VOID _ser_task_return_tss(core_machine_cpu_execution_context *context)
{
    type_unsigned_16 backlink;

    TYPE_TRACE_CALL_BEGIN("_ser_task_return_tss");
    if (!_IsProtected || !cpu_state.data.tr.flagValid ||
        _GetSelector_TI(cpu_state.data.tr.selector) ||
        (cpu_state.data.tr.sys.type != VCPU_DESC_SYS_TYPE_TSS_16_BUSY &&
            cpu_state.data.tr.sys.type != VCPU_DESC_SYS_TYPE_TSS_32_BUSY) ||
        cpu_state.data.tr.limit < 1u)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(0));
    TYPE_TRACE_CHECK_RETURN(_s_read_tss(context, TASK_SWITCH_TSS_BACKLINK_OFFSET,
        TYPE_REFERENCE_OF(backlink), 2u));
    if (_IsSelectorNull(backlink) || _GetSelector_TI(backlink))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_TS(backlink & 0xfffcu));
    TYPE_TRACE_CHECK_RETURN(_ser_task_transition_tss(context, backlink,
        TYPE_FALSE, TYPE_TRUE));
    TYPE_TRACE_CALL_END;
}
/* regular execute control */
static C_VOID _e_push(core_machine_cpu_execution_context *context, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_e_push");
    TYPE_TRACE_CHECK_RETURN(_kec_push(context, rdata, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _e_pop(core_machine_cpu_execution_context *context, type_virtual_address rdata, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_e_pop");
    TYPE_TRACE_CHECK_RETURN(_kec_pop(context, rdata, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _e_call_far(core_machine_cpu_execution_context *context, type_unsigned_16 newcs, type_unsigned_32 neweip, type_unsigned_8 byte)
{
    type_unsigned_64 descriptor;
    TYPE_TRACE_CALL_BEGIN("_e_call_far");
    if (!_IsProtected)
    {
        TYPE_TRACE_BLOCK_BEGIN("!Protected");
        TYPE_TRACE_CHECK_RETURN(_ser_call_far_real(context, newcs, neweip, byte));
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("Protected");
        if (_IsSelectorNull(newcs))
        {
            TYPE_TRACE_BLOCK_BEGIN("newcs(null)");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, TYPE_REFERENCE_OF(descriptor)));
        if (_IsDescCodeConform(descriptor))
            TYPE_TRACE_CHECK_RETURN(_ser_call_far_cs_conf(context, newcs, neweip, byte));
        else if (_IsDescCodeNonConform(descriptor))
            TYPE_TRACE_CHECK_RETURN(_ser_call_far_cs_nonc(context, newcs, neweip, byte));
        else if (_IsDescCallGate(descriptor))
            TYPE_TRACE_CHECK_RETURN(_ser_call_far_call_gate(context, newcs, byte));
        else if (_IsDescTaskGate(descriptor))
            TYPE_TRACE_CHECK_RETURN(_ser_call_far_task_gate(context, newcs));
        else if (_IsDescTSS(descriptor))
            TYPE_TRACE_CHECK_RETURN(_ser_call_far_tss(context, newcs));
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("desc(invalid)");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _e_call_near(core_machine_cpu_execution_context *context, type_unsigned_32 neweip, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_e_call_near");
    TYPE_TRACE_CHECK_RETURN(_kec_call_near(context, neweip, byte));
    TYPE_TRACE_CALL_END;
}
_______todo _e_int3(core_machine_cpu_execution_context *context, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_e_int3");
    if (!_GetCR0_PE)
    {
        TYPE_TRACE_BLOCK_BEGIN("Real");
        TYPE_TRACE_CHECK_RETURN(_ser_int_real(context, 0x03, byte));
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("!Real");
        TYPE_TRACE_CHECK_RETURN(_ser_int_protected(context, 0x03u, byte,
            TYPE_TRUE, TYPE_FALSE));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
_______todo _e_into(core_machine_cpu_execution_context *context, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_e_into");
    if (_GetEFLAGS_OF)
    {
        TYPE_TRACE_BLOCK_BEGIN("EFLAGS_OF(1)");
        if (!_GetCR0_PE)
        {
            TYPE_TRACE_BLOCK_BEGIN("Real");
            TYPE_TRACE_CHECK_RETURN(_ser_int_real(context, 0x04, byte));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("!Real");
            TYPE_TRACE_CHECK_RETURN(_ser_int_protected(context, 0x04u, byte,
                TYPE_TRUE, TYPE_FALSE));
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
_______todo _e_int_n(core_machine_cpu_execution_context *context, type_unsigned_8 intid, type_unsigned_8 byte)
{
    type_bool handled;

    TYPE_TRACE_CALL_BEGIN("_e_int_n");
    if (!_GetCR0_PE)
    {
        TYPE_TRACE_BLOCK_BEGIN("Real");
        TYPE_TRACE_CHECK_RETURN(_e_try_firmware_software_interrupt(context,
            intid, &handled));
        if (!handled) {
            TYPE_TRACE_CHECK_RETURN(_ser_int_real(context, intid, byte));
        }
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("!Real");
        if (_GetEFLAGS_VM && _GetEFLAGS_IOPL < 3)
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        else
            TYPE_TRACE_CHECK_RETURN(_ser_int_protected(context, intid, byte,
                TYPE_TRUE, TYPE_FALSE));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
_______todo _e_intr_n(core_machine_cpu_execution_context *context,
    type_unsigned_8 intid, type_unsigned_8 byte, type_bool external_origin)
{
    TYPE_TRACE_CALL_BEGIN("_e_intr_n");
    if (!_GetCR0_PE)
    {
        TYPE_TRACE_BLOCK_BEGIN("Real");
        TYPE_TRACE_CHECK_RETURN(_ser_int_real(context, intid, byte));
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("!Real");
        if (!external_origin)
            TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        else
            TYPE_TRACE_CHECK_RETURN(_ser_int_protected(context, intid, byte,
                TYPE_FALSE, TYPE_FALSE));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
static type_bool _e_exception_has_error_code(type_unsigned_8 exid)
{
    return exid == 0x08u || exid == 0x0au || exid == 0x0bu ||
        exid == 0x0cu || exid == 0x0du || exid == 0x0eu || exid == 0x11u;
}

_______todo _e_except_n(core_machine_cpu_execution_context *context, type_unsigned_8 exid, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_e_except_n");
    instruction_state.data.except &= ~(1 << exid);
    if (!_GetCR0_PE)
    {
        TYPE_TRACE_BLOCK_BEGIN("Real");
        TYPE_TRACE_CHECK_RETURN(_ser_int_real(context, exid, byte));
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("!Real");
        TYPE_TRACE_CHECK_RETURN(_ser_int_protected(context, exid, byte,
            TYPE_FALSE, _e_exception_has_error_code(exid)));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _ser_iret_protected_outer(core_machine_cpu_execution_context *context,
    type_unsigned_8 byte)
{
    type_unsigned_16 newcs;
    type_unsigned_16 newss;
    type_unsigned_32 neweip;
    type_unsigned_32 newesp;
    type_unsigned_32 newflags;
    type_unsigned_32 selector;
    type_unsigned_64 code_desc;
    type_unsigned_64 ss_desc;
    type_unsigned_8 oldcpl;
    type_unsigned_8 newcpl;
    type_unsigned_32 flags_mask;
    t_cpu_data_sreg newcs_cache;
    t_cpu_data_sreg newss_cache;

    TYPE_TRACE_CALL_BEGIN("_ser_iret_protected_outer");
    oldcpl = _GetCPL;
    /* Ordinary protected IRET never restores VM; old CPL controls IOPL/IF. */
    flags_mask = VCPU_EFLAGS_RESERVED | VCPU_EFLAGS_VM;
    if (oldcpl != 0u) {
        flags_mask |= VCPU_EFLAGS_IOPL;
        if (oldcpl > _GetEFLAGS_IOPL) flags_mask |= VCPU_EFLAGS_IF;
    }
    switch (byte)
    {
    case 2:
        TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 10u));
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 0u,
            TYPE_REFERENCE_OF(neweip), 2u));
        selector = 0u;
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 2u,
            TYPE_REFERENCE_OF(selector), 2u));
        newcs = TYPE_MASK_UNSIGNED_16(selector);
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 4u,
            TYPE_REFERENCE_OF(newflags), 2u));
        neweip = TYPE_MASK_UNSIGNED_16(neweip);
        newflags = TYPE_MASK_UNSIGNED_16(newflags);
        break;
    case 4:
        TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 20u));
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 0u,
            TYPE_REFERENCE_OF(neweip), 4u));
        selector = 0u;
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 4u,
            TYPE_REFERENCE_OF(selector), 4u));
        newcs = TYPE_MASK_UNSIGNED_16(selector);
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 8u,
            TYPE_REFERENCE_OF(newflags), 4u));
        break;
    default:
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        break;
    }
    if (_IsSelectorNull(newcs) || _GetSelector_TI(newcs)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    }
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
        TYPE_REFERENCE_OF(code_desc)));
    if (!_IsDescCodeNonConform(code_desc)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    }
    if (!_IsDescPresent(code_desc)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
    }
    newcpl = (type_unsigned_8)_GetSelector_RPL(newcs);
    if (newcpl <= oldcpl || newcpl != _GetDesc_DPL(code_desc)) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    }
    switch (byte)
    {
    case 2:
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 6u,
            TYPE_REFERENCE_OF(newesp), 2u));
        selector = 0u;
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 8u,
            TYPE_REFERENCE_OF(selector), 2u));
        newesp = TYPE_MASK_UNSIGNED_16(newesp);
        break;
    case 4:
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 12u,
            TYPE_REFERENCE_OF(newesp), 4u));
        selector = 0u;
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 16u,
            TYPE_REFERENCE_OF(selector), 4u));
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    newss = TYPE_MASK_UNSIGNED_16(selector);
    if (_IsSelectorNull(newss) || _GetSelector_TI(newss) ||
        _GetSelector_RPL(newss) != newcpl) {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newss & 0xfffcu));
    }
    newcs_cache = cpu_state.data.cs;
    TYPE_TRACE_CHECK_RETURN(_ksa_prepare_code_sreg(context, newcs, newcpl,
        &newcs_cache, &code_desc));
    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &newcs_cache, neweip,
        1u, 0, newcpl, 1));
    newss_cache = cpu_state.data.ss;
    TYPE_TRACE_CHECK_RETURN(_ksa_prepare_stack_sreg(context, newss, newcpl,
        &newss_cache, &ss_desc));
    TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
        TYPE_REFERENCE_OF(code_desc)));
    TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newss,
        TYPE_REFERENCE_OF(ss_desc)));
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.ss = newss_cache;
    if (newss_cache.seg.data.big)
        cpu_state.data.esp = newesp;
    else
        cpu_state.data.sp = TYPE_MASK_UNSIGNED_16(newesp);
    cpu_state.data.eip = neweip;
    cpu_state.data.eflags = _e_eflags_load(context,
        (cpu_state.data.eflags & flags_mask) | (newflags & ~flags_mask));
    TYPE_TRACE_CALL_END;
}
static C_VOID _ser_iret_protected_same(core_machine_cpu_execution_context *context,
    type_unsigned_8 byte)
{
    type_unsigned_16 newcs;
    type_unsigned_32 neweip;
    type_unsigned_32 neweflags;
    type_unsigned_32 selector;
    type_unsigned_32 mask = VCPU_EFLAGS_RESERVED;
    type_unsigned_64 code_desc;
    type_unsigned_8 cpl;
    t_cpu_data_sreg newcs_cache;

    TYPE_TRACE_CALL_BEGIN("_ser_iret_protected_same");
    cpl = _GetCPL;
    switch (byte)
    {
    case 2:
        TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 6u));
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 0u,
            TYPE_REFERENCE_OF(neweip), 2u));
        selector = 0u;
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 2u,
            TYPE_REFERENCE_OF(selector), 2u));
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 4u,
            TYPE_REFERENCE_OF(neweflags), 2u));
        neweip = TYPE_MASK_UNSIGNED_16(neweip);
        mask |= 0xffff0000u;
        break;
    case 4:
        TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 12u));
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 0u,
            TYPE_REFERENCE_OF(neweip), 4u));
        selector = 0u;
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 4u,
            TYPE_REFERENCE_OF(selector), 4u));
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 8u,
            TYPE_REFERENCE_OF(neweflags), 4u));
        break;
    default:
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        break;
    }
    newcs = TYPE_MASK_UNSIGNED_16(selector);
    if (_IsSelectorNull(newcs))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
    TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
        TYPE_REFERENCE_OF(code_desc)));
    if (!_IsDescCode(code_desc))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    if (_GetSelector_RPL(newcs) != cpl)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    if ((_IsDescCodeConform(code_desc) && _GetDesc_DPL(code_desc) > cpl) ||
        (_IsDescCodeNonConform(code_desc) && _GetDesc_DPL(code_desc) != cpl))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    if (!_IsDescPresent(code_desc))
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
    newcs_cache = cpu_state.data.cs;
    TYPE_TRACE_CHECK_RETURN(_ksa_prepare_code_sreg(context, newcs, cpl,
        &newcs_cache, &code_desc));
    TYPE_TRACE_CHECK_RETURN(_kma_test_access(context, &newcs_cache, neweip,
        1u, 0, cpl, 1));
    if (cpl) {
        mask |= VCPU_EFLAGS_IOPL | VCPU_EFLAGS_VM;
        if (cpl > _GetEFLAGS_IOPL) mask |= VCPU_EFLAGS_IF;
    }
    TYPE_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
        TYPE_REFERENCE_OF(code_desc)));
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.eip = neweip;
    cpu_state.data.eflags = _e_eflags_load(context,
        (neweflags & ~mask) | (cpu_state.data.eflags & mask));
    switch (_GetStackSize)
    {
    case 2:
        cpu_state.data.sp += byte * 3u;
        break;
    case 4:
        cpu_state.data.esp += byte * 3u;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
/* Build the architectural real-mode-style cache used after a CPL0 32-bit
 * IRET return to virtual-8086 mode.  The return-frame selectors are not
 * protected-mode selectors: they cannot require a descriptor-table lookup
 * and therefore this helper is deliberately infallible. */
static C_VOID _ser_iret_vm86_sreg(t_cpu_data_sreg *rsreg,
    type_unsigned_16 selector, type_unsigned_8 sregtype)
{
    rsreg->flagValid = TYPE_TRUE;
    rsreg->selector = selector;
    rsreg->base = (type_unsigned_32)selector << 4u;
    rsreg->limit = 0x0000ffffu;
    rsreg->dpl = 3u;
    rsreg->sregtype = sregtype;
    rsreg->seg.accessed = TYPE_FALSE;
    rsreg->seg.executable = sregtype == SREG_CODE;
    if (sregtype == SREG_CODE) {
        rsreg->seg.exec.defsize = TYPE_FALSE;
        rsreg->seg.exec.conform = TYPE_FALSE;
        rsreg->seg.exec.readable = TYPE_TRUE;
    } else {
        rsreg->seg.data.big = TYPE_FALSE;
        rsreg->seg.data.expdown = TYPE_FALSE;
        rsreg->seg.data.writable = TYPE_TRUE;
    }
}
static C_VOID _ser_iret_protected_to_vm86(
    core_machine_cpu_execution_context *context)
{
    type_unsigned_32 neweip;
    type_unsigned_32 newflags;
    type_unsigned_32 newesp;
    type_unsigned_32 selector;
    type_unsigned_16 newcs, newss, newes, newds, newfs, newgs;
    t_cpu_data_sreg newcs_cache = cpu_state.data.cs;
    t_cpu_data_sreg newss_cache = cpu_state.data.ss;
    t_cpu_data_sreg newes_cache = cpu_state.data.es;
    t_cpu_data_sreg newds_cache = cpu_state.data.ds;
    t_cpu_data_sreg newfs_cache = cpu_state.data.fs;
    t_cpu_data_sreg newgs_cache = cpu_state.data.gs;

    TYPE_TRACE_CALL_BEGIN("_ser_iret_protected_to_vm86");
    /* This is the full 80386 32-bit VM86 return frame.  Every read precedes
     * publication so a failed stack read leaves the CPL0 frame intact. */
    TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 36u));
    TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 0u,
        TYPE_REFERENCE_OF(neweip), 4u));
    TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 4u,
        TYPE_REFERENCE_OF(selector), 4u));
    newcs = TYPE_MASK_UNSIGNED_16(selector);
    TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 8u,
        TYPE_REFERENCE_OF(newflags), 4u));
    TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 12u,
        TYPE_REFERENCE_OF(newesp), 4u));
    TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 16u,
        TYPE_REFERENCE_OF(selector), 4u));
    newss = TYPE_MASK_UNSIGNED_16(selector);
    TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 20u,
        TYPE_REFERENCE_OF(selector), 4u));
    newes = TYPE_MASK_UNSIGNED_16(selector);
    TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 24u,
        TYPE_REFERENCE_OF(selector), 4u));
    newds = TYPE_MASK_UNSIGNED_16(selector);
    TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 28u,
        TYPE_REFERENCE_OF(selector), 4u));
    newfs = TYPE_MASK_UNSIGNED_16(selector);
    TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 32u,
        TYPE_REFERENCE_OF(selector), 4u));
    newgs = TYPE_MASK_UNSIGNED_16(selector);
    _ser_iret_vm86_sreg(&newcs_cache, newcs, SREG_CODE);
    _ser_iret_vm86_sreg(&newss_cache, newss, SREG_STACK);
    _ser_iret_vm86_sreg(&newes_cache, newes, SREG_DATA);
    _ser_iret_vm86_sreg(&newds_cache, newds, SREG_DATA);
    _ser_iret_vm86_sreg(&newfs_cache, newfs, SREG_DATA);
    _ser_iret_vm86_sreg(&newgs_cache, newgs, SREG_DATA);
    cpu_state.data.eflags = _e_eflags_load(context,
        (newflags & ~VCPU_EFLAGS_RESERVED) |
        (cpu_state.data.eflags & VCPU_EFLAGS_RESERVED));
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.ss = newss_cache;
    cpu_state.data.es = newes_cache;
    cpu_state.data.ds = newds_cache;
    cpu_state.data.fs = newfs_cache;
    cpu_state.data.gs = newgs_cache;
    cpu_state.data.eip = neweip;
    cpu_state.data.esp = newesp;
    _MakeCPL(3u);
    TYPE_TRACE_CALL_END;
}
_______todo _e_iret(core_machine_cpu_execution_context *context, type_unsigned_8 byte)
{
    type_unsigned_16 newcs, newss, newds, newes, newfs, newgs;
    type_unsigned_16 return_cs;
    type_unsigned_32 neweip = TYPE_ZERO_32, newesp, neweflags = TYPE_ZERO_32;
    type_unsigned_32 xs_sel;
    type_unsigned_32 mask = VCPU_EFLAGS_RESERVED;
    t_cpu_data_sreg ccs = cpu_state.data.cs;
    TYPE_TRACE_CALL_BEGIN("_e_iret");
    if (!_GetCR0_PE)
    {
        TYPE_TRACE_BLOCK_BEGIN("Real");
        switch (byte)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("byte(2)");
            TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 6));
            TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(neweip), 2));
            TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(xs_sel), 2));
            newcs = TYPE_MASK_UNSIGNED_16(xs_sel);
            TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(neweflags), 2));
            mask |= 0xffff0000;
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("byte(4)");
            TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 12));
            TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(neweip), 4));
            TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(xs_sel), 4));
            newcs = TYPE_MASK_UNSIGNED_16(xs_sel);
            TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(neweflags), 4));
            /* cpu_state.data.eflags = (neweflags & 0x00257fd5) | (cpu_state.data.eflags & 0x001a0000); */
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_BLOCK_BEGIN("byte");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
            TYPE_TRACE_BLOCK_END;
            break;
        }
        TYPE_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
        TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
        cpu_state.data.cs = ccs;
        cpu_state.data.eip = neweip;
        cpu_state.data.eflags = _e_eflags_load(context,
            (neweflags & ~mask) | (cpu_state.data.eflags & mask));
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("!Real");
        if (!_GetEFLAGS_VM && !_GetEFLAGS_NT) {
            type_unsigned_32 return_flags = 0u;

            switch (byte) {
            case 2:
                TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 6u));
                TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 4u,
                    TYPE_REFERENCE_OF(return_flags), 2u));
                break;
            case 4:
                TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 12u));
                TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 8u,
                    TYPE_REFERENCE_OF(return_flags), 4u));
                if (TYPE_GET_BIT(return_flags, VCPU_EFLAGS_VM) && !_GetCPL) {
                    TYPE_TRACE_CHECK_RETURN(_ser_iret_protected_to_vm86(context));
                    TYPE_TRACE_CALL_END;
                    return;
                }
                break;
            default:
                TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
                break;
            }
            if (!TYPE_GET_BIT(return_flags, VCPU_EFLAGS_VM)) {
                TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, byte,
                    TYPE_REFERENCE_OF(return_cs), 2u));
                if (_GetSelector_RPL(return_cs) <= _GetCPL) {
                    TYPE_TRACE_CHECK_RETURN(_ser_iret_protected_same(context,
                        byte));
                    TYPE_TRACE_CALL_END;
                    return;
                }
                TYPE_TRACE_CHECK_RETURN(_ser_iret_protected_outer(context, byte));
                TYPE_TRACE_CALL_END;
                return;
            }
        }
        if (_GetEFLAGS_VM)
        {
            TYPE_TRACE_BLOCK_BEGIN("V86");
            /* RETURN-FROM-VIRTUAL-8086-MODE */
            if (_GetEFLAGS_IOPL == 3)
            {
                TYPE_TRACE_BLOCK_BEGIN("EFLAGS_IOPL(3)");
                switch (byte)
                {
                case 2:
                    TYPE_TRACE_BLOCK_BEGIN("byte(2)");
                    TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 6));
                    TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(neweip), 2));
                    TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(xs_sel), 2));
                    newcs = TYPE_MASK_UNSIGNED_16(xs_sel);
                    TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(neweflags), 2));
                    mask |= (0xffff0000 | VCPU_EFLAGS_IOPL);
                    TYPE_TRACE_BLOCK_END;
                    break;
                case 4:
                    TYPE_TRACE_BLOCK_BEGIN("byte(4)");
                    TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 12));
                    TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(neweip), 4));
                    TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(xs_sel), 4));
                    newcs = TYPE_MASK_UNSIGNED_16(xs_sel);
                    TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(neweflags), 4));
                    mask |= (VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL);
                    TYPE_TRACE_BLOCK_END;
                    break;
                default:
                    TYPE_TRACE_BLOCK_BEGIN("byte");
                    TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
                    TYPE_TRACE_BLOCK_END;
                    break;
                }
                TYPE_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
                TYPE_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
                cpu_state.data.cs = ccs;
                cpu_state.data.eip = neweip;
                cpu_state.data.eflags = _e_eflags_load(context,
                    (neweflags & ~mask) | (cpu_state.data.eflags & mask));
                TYPE_TRACE_BLOCK_END;
            }
            else
            {
                TYPE_TRACE_BLOCK_BEGIN("EFLAGS_IOPL(!3)");
                /* trap to virtual-8086 monitor */
                instruction_state.data.oldcpu = cpu_state;
                TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_BLOCK_END;
        }
        else if (_GetEFLAGS_NT)
        {
            TYPE_TRACE_BLOCK_BEGIN("Nested");
            TYPE_TRACE_CHECK_RETURN(_ser_task_return_tss(context));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("Protected,!Nested");
            switch (byte)
            {
            case 2:
                TYPE_TRACE_BLOCK_BEGIN("byte(2)");
                TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 6));
                TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(neweip), 2));
                TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(xs_sel), 2));
                newcs = TYPE_MASK_UNSIGNED_16(xs_sel);
                TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(neweflags), 2));
                TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("byte(4)");
                TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 12));
                TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(neweip), 4));
                TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(xs_sel), 4));
                newcs = TYPE_MASK_UNSIGNED_16(xs_sel);
                TYPE_TRACE_CHECK_RETURN(_kec_pop(context, TYPE_REFERENCE_OF(neweflags), 4));
                TYPE_TRACE_BLOCK_END;
                break;
            default:
                TYPE_TRACE_BLOCK_BEGIN("byte");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
                TYPE_TRACE_BLOCK_END;
                break;
            }
            /* CPL0 32-bit VM86 return is handled before this legacy pop path
             * can publish its first frame field.  This retained path has no
             * other supported protected return contract. */
            TYPE_TRACE_BLOCK_BEGIN("unsupported protected return");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(0));
            TYPE_TRACE_BLOCK_END;
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _e_jcc(core_machine_cpu_execution_context *context, type_unsigned_32 csrc, type_unsigned_8 byte, type_bool condition)
{
    type_unsigned_32 neweip = cpu_state.data.eip;
    TYPE_TRACE_CALL_BEGIN("_e_jcc");
    if (condition)
    {
        TYPE_TRACE_BLOCK_BEGIN("condition(1)");
        switch (byte)
        {
        case 1:
            neweip += (type_signed_8)csrc;
            break;
        case 2:
            neweip += (type_signed_16)csrc;
            break;
        case 4:
            neweip += (type_signed_32)csrc;
            break;
        default:
            TYPE_TRACE_BLOCK_BEGIN("byte");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
            TYPE_TRACE_BLOCK_END;
            break;
        }
        TYPE_TRACE_CHECK_RETURN(_kec_jmp_near(context, neweip, _GetOperandSize));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _e_jmp_far(core_machine_cpu_execution_context *context, type_unsigned_16 newcs, type_unsigned_32 neweip, type_unsigned_8 byte)
{
    type_unsigned_64 descriptor;
    TYPE_TRACE_CALL_BEGIN("_e_jmp_far");
    if (!_IsProtected)
    {
        TYPE_TRACE_BLOCK_BEGIN("Protected(0)");
        TYPE_TRACE_CHECK_RETURN(_ser_jmp_far_real(context, newcs, neweip, byte));
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("Protected(1)");
        if (_IsSelectorNull(newcs))
        {
            TYPE_TRACE_BLOCK_BEGIN("selector(null)");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, TYPE_REFERENCE_OF(descriptor)));
        if (_IsDescCodeConform(descriptor))
            TYPE_TRACE_CHECK_RETURN(_ser_jmp_far_cs_conf(context, newcs, neweip, byte));
        else if (_IsDescCodeNonConform(descriptor))
            TYPE_TRACE_CHECK_RETURN(_ser_jmp_far_cs_nonc(context, newcs, neweip, byte));
        else if (_IsDescCallGate(descriptor))
            TYPE_TRACE_CHECK_RETURN(_ser_jmp_far_call_gate(context, newcs));
        else if (_IsDescTaskGate(descriptor))
            TYPE_TRACE_CHECK_RETURN(_ser_jmp_far_task_gate(context, newcs));
        else if (_IsDescTSS(descriptor))
            TYPE_TRACE_CHECK_RETURN(_ser_task_switch_tss(context, newcs,
                TYPE_FALSE));
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("newcs(invalid)");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs));
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
    }
    instruction_state.data.opr1 = cpu_state.data.cs.selector;
    instruction_state.data.opr2 = cpu_state.data.eip;
    TYPE_TRACE_CALL_END;
}
static C_VOID _e_jmp_near(core_machine_cpu_execution_context *context, type_unsigned_32 neweip, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_e_jmp_near");
    TYPE_TRACE_CHECK_RETURN(_kec_jmp_near(context, neweip, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _e_load_far(core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg, type_virtual_address rdest, type_unsigned_16 selector, type_unsigned_32 offset, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_e_load_far");
    TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, rsreg, selector));
    switch (byte)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, rdest, TYPE_REFERENCE_OF(offset), 2));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, rdest, TYPE_REFERENCE_OF(offset), 4));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("byte");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    if (rsreg->sregtype == SREG_STACK)
        instruction_state.data.flagMaskInt = TYPE_TRUE;
    TYPE_TRACE_CALL_END;
}
static C_VOID _e_pop_sreg(core_machine_cpu_execution_context *context,
    t_cpu_data_sreg *rsreg, type_unsigned_8 byte)
{
    type_unsigned_32 selector;
    type_unsigned_32 cesp;

    TYPE_TRACE_CALL_BEGIN("_e_pop_sreg");
    switch (_GetStackSize)
    {
    case 2:
        cesp = cpu_state.data.sp;
        break;
    case 4:
        cesp = cpu_state.data.esp;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CHECK_RETURN(_s_read_ss(context, cesp, TYPE_REFERENCE_OF(selector),
        byte));
    TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, rsreg,
        TYPE_MASK_UNSIGNED_16(selector)));
    switch (_GetStackSize)
    {
    case 2:
        cpu_state.data.sp += byte;
        break;
    case 4:
        cpu_state.data.esp += byte;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    if (rsreg->sregtype == SREG_STACK)
        instruction_state.data.flagMaskInt = TYPE_TRUE;
    TYPE_TRACE_CALL_END;
}
static C_VOID _e_loopcc(core_machine_cpu_execution_context *context, type_signed_8 csrc, type_bool condition)
{
    type_unsigned_32 cecx;
    type_unsigned_32 neweip = cpu_state.data.eip;
    TYPE_TRACE_CALL_BEGIN("_e_loopcc");
    switch (_GetAddressSize)
    {
    case 2:
        cecx = TYPE_MASK_UNSIGNED_16(cpu_state.data.cx - 1u);
        break;
    case 4:
        cecx = TYPE_MASK_UNSIGNED_32(cpu_state.data.ecx - 1u);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    if (cecx && condition)
    {
        TYPE_TRACE_BLOCK_BEGIN("cecx(!0),condition(1)");
        neweip += csrc;
        TYPE_TRACE_CHECK_RETURN(_kec_jmp_near(context, neweip, _GetOperandSize));
        TYPE_TRACE_BLOCK_END;
    }
    switch (_GetAddressSize)
    {
    case 2:
        cpu_state.data.cx = TYPE_MASK_UNSIGNED_16(cecx);
        break;
    case 4:
        cpu_state.data.ecx = cecx;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _e_ret_near(core_machine_cpu_execution_context *context, type_unsigned_16 parambyte, type_unsigned_8 byte)
{
    TYPE_TRACE_CALL_BEGIN("_e_ret_near");
    TYPE_TRACE_CHECK_RETURN(_kec_ret_near(context, parambyte, byte));
    TYPE_TRACE_CALL_END;
}
static C_VOID _e_ret_far(core_machine_cpu_execution_context *context, type_unsigned_16 parambyte, type_unsigned_16 byte)
{
    type_unsigned_16 newcs;
    type_unsigned_32 xs_sel;
    type_unsigned_32 neweip = 0;
    type_unsigned_64 descriptor;
    TYPE_TRACE_CALL_BEGIN("_e_ret_far");
    switch (byte)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 4));
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 0u, TYPE_REFERENCE_OF(neweip), 2));
        xs_sel = 0u;
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 2u, TYPE_REFERENCE_OF(xs_sel), 2));
        newcs = TYPE_MASK_UNSIGNED_16(xs_sel);
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        TYPE_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 8));
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 0u, TYPE_REFERENCE_OF(neweip), 4));
        xs_sel = 0u;
        TYPE_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 4u, TYPE_REFERENCE_OF(xs_sel), 4));
        newcs = TYPE_MASK_UNSIGNED_16(xs_sel);
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("byte");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    if (!_IsProtected)
    {
        TYPE_TRACE_BLOCK_BEGIN("!Protected");
        TYPE_TRACE_CHECK_RETURN(_ser_ret_far_real(context, newcs, neweip,
            TYPE_MASK_UNSIGNED_16(parambyte + byte * 2u), byte));
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("Protected");
        if (_IsSelectorNull(newcs))
        {
            TYPE_TRACE_BLOCK_BEGIN("selector(null)");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, TYPE_REFERENCE_OF(descriptor)));
        if (!_IsDescCode(descriptor))
        {
            TYPE_TRACE_BLOCK_BEGIN("!DescCode");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
            TYPE_TRACE_BLOCK_END;
        }
        if (_GetSelector_RPL(newcs) < _GetCPL)
        {
            TYPE_TRACE_BLOCK_BEGIN("RPL(<CPL)");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
            TYPE_TRACE_BLOCK_END;
        }
        if (_IsDescCodeConform(descriptor))
        {
            TYPE_TRACE_BLOCK_BEGIN("DescCodeConform");
            if (_GetDesc_DPL(descriptor) > _GetSelector_RPL(newcs))
            {
                TYPE_TRACE_BLOCK_BEGIN("DPL(>RPL)");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_BLOCK_END;
        }
        else if (_GetDesc_DPL(descriptor) != _GetSelector_RPL(newcs))
        {
            TYPE_TRACE_BLOCK_BEGIN("DescCodeNonConform/DPL(!RPL)");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
            TYPE_TRACE_BLOCK_END;
        }
        if (!_IsDescPresent(descriptor))
        {
            TYPE_TRACE_BLOCK_BEGIN("!DescPresent");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
            TYPE_TRACE_BLOCK_END;
        }
        if (_GetSelector_RPL(newcs) > _GetCPL)
        {
            TYPE_TRACE_BLOCK_BEGIN("RPL(>CPL)");
            TYPE_TRACE_CHECK_RETURN(_ser_ret_far_outer(context, newcs, neweip, parambyte, byte));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("RPL(<=CPL)");
            TYPE_TRACE_CHECK_RETURN(_ser_ret_far_same(context, newcs, neweip,
                TYPE_MASK_UNSIGNED_16(parambyte + byte * 2u), byte));
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}

/* arithmetic unit */
/* kernel arithmetic flags */
#define ADD_FLAG (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                  VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF)
#define OR_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define ADC_FLAG (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                  VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF)
#define SBB_FLAG (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                  VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF)
#define AND_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SUB_FLAG (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                  VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF)
#define XOR_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define CMP_FLAG (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                  VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF)
#define INC_FLAG (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                  VCPU_EFLAGS_AF | VCPU_EFLAGS_PF)
#define DEC_FLAG (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                  VCPU_EFLAGS_AF | VCPU_EFLAGS_PF)
#define NEG_FLAG (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | \
                  VCPU_EFLAGS_AF | VCPU_EFLAGS_PF)
#define NOT_FLAG (0)
#define TEST_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SHL_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SHR_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SAL_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SAR_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define AAM_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define AAD_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define DAA_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define DAS_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)

static C_VOID _kaf_calc_CF(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("_kaf_calc_CF");
    switch (instruction_state.data.type)
    {
    case ADC8:
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, ((_GetEFLAGS_CF && instruction_state.data.opr2 == TYPE_MAX_UNSIGNED_8) ? 1 : ((instruction_state.data.result < instruction_state.data.opr1) || (instruction_state.data.result < instruction_state.data.opr2))));
        break;
    case ADC16:
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, ((_GetEFLAGS_CF && instruction_state.data.opr2 == TYPE_MAX_UNSIGNED_16) ? 1 : ((instruction_state.data.result < instruction_state.data.opr1) || (instruction_state.data.result < instruction_state.data.opr2))));
        break;
    case ADC32:
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, ((_GetEFLAGS_CF && instruction_state.data.opr2 == TYPE_MAX_UNSIGNED_32) ? 1 : ((instruction_state.data.result < instruction_state.data.opr1) || (instruction_state.data.result < instruction_state.data.opr2))));
        break;
    case ADD8:
    case ADD16:
    case ADD32:
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (instruction_state.data.result < instruction_state.data.opr1) || (instruction_state.data.result < instruction_state.data.opr2));
        break;
    case SBB8:
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (instruction_state.data.opr1 < instruction_state.data.result) || (_GetEFLAGS_CF && (instruction_state.data.opr2 == TYPE_MAX_UNSIGNED_8)));
        break;
    case SBB16:
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (instruction_state.data.opr1 < instruction_state.data.result) || (_GetEFLAGS_CF && (instruction_state.data.opr2 == TYPE_MAX_UNSIGNED_16)));
        break;
    case SBB32:
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (instruction_state.data.opr1 < instruction_state.data.result) || (_GetEFLAGS_CF && (instruction_state.data.opr2 == TYPE_MAX_UNSIGNED_32)));
        break;
    case SUB8:
    case SUB16:
    case SUB32:
    case CMP8:
    case CMP16:
    case CMP32:
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, instruction_state.data.opr1 < instruction_state.data.opr2);
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("type");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(instruction_state.data.type));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _kaf_calc_OF(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("_kaf_calc_OF");
    switch (instruction_state.data.type)
    {
    case ADC8:
    case ADD8:
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                      (TYPE_GET_MSB_8(instruction_state.data.opr1) == TYPE_GET_MSB_8(instruction_state.data.opr2)) && (TYPE_GET_MSB_8(instruction_state.data.opr1) != TYPE_GET_MSB_8(instruction_state.data.result)));
        break;
    case ADC16:
    case ADD16:
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                      (TYPE_GET_MSB_16(instruction_state.data.opr1) == TYPE_GET_MSB_16(instruction_state.data.opr2)) && (TYPE_GET_MSB_16(instruction_state.data.opr1) != TYPE_GET_MSB_16(instruction_state.data.result)));
        break;
    case ADC32:
    case ADD32:
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                      (TYPE_GET_MSB_32(instruction_state.data.opr1) == TYPE_GET_MSB_32(instruction_state.data.opr2)) && (TYPE_GET_MSB_32(instruction_state.data.opr1) != TYPE_GET_MSB_32(instruction_state.data.result)));
        break;
    case SBB8:
    case SUB8:
    case CMP8:
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                      (TYPE_GET_MSB_8(instruction_state.data.opr1) != TYPE_GET_MSB_8(instruction_state.data.opr2)) && (TYPE_GET_MSB_8(instruction_state.data.opr2) == TYPE_GET_MSB_8(instruction_state.data.result)));
        break;
    case SBB16:
    case SUB16:
    case CMP16:
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                      (TYPE_GET_MSB_16(instruction_state.data.opr1) != TYPE_GET_MSB_16(instruction_state.data.opr2)) && (TYPE_GET_MSB_16(instruction_state.data.opr2) == TYPE_GET_MSB_16(instruction_state.data.result)));
        break;
    case SBB32:
    case SUB32:
    case CMP32:
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                      (TYPE_GET_MSB_32(instruction_state.data.opr1) != TYPE_GET_MSB_32(instruction_state.data.opr2)) && (TYPE_GET_MSB_32(instruction_state.data.opr2) == TYPE_GET_MSB_32(instruction_state.data.result)));
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("type");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(instruction_state.data.type));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _kaf_calc_AF(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("_kaf_calc_AF");
    TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_AF, ((instruction_state.data.opr1 ^ instruction_state.data.opr2) ^ instruction_state.data.result) & 0x10);
    TYPE_TRACE_CALL_END;
}
static C_VOID _kaf_calc_PF(core_machine_cpu_execution_context *context)
{
    type_unsigned_8 res8 = TYPE_MASK_UNSIGNED_8(instruction_state.data.result);
    type_bool even = 1;
    TYPE_TRACE_CALL_BEGIN("_kaf_calc_PF");
    while (res8)
    {
        even = 1 - even;
        res8 &= res8 - 1;
    }
    TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_PF, even);
    TYPE_TRACE_CALL_END;
}
static C_VOID _kaf_calc_ZF(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("_kaf_calc_ZF");
    TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_ZF, !instruction_state.data.result);
    TYPE_TRACE_CALL_END;
}
static C_VOID _kaf_calc_SF(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("_kaf_calc_SF");
    switch (instruction_state.data.bit)
    {
    case 8:
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_SF, TYPE_GET_MSB_8(instruction_state.data.result));
        break;
    case 16:
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_SF, TYPE_GET_MSB_16(instruction_state.data.result));
        break;
    case 32:
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_SF, TYPE_GET_MSB_32(instruction_state.data.result));
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(instruction_state.data.bit));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _kaf_set_flags(core_machine_cpu_execution_context *context, type_unsigned_16 flags)
{
    TYPE_TRACE_CALL_BEGIN("_kaf_set_flags");
    if (flags & VCPU_EFLAGS_CF)
        TYPE_TRACE_CHECK_RETURN(_kaf_calc_CF(context));
    if (flags & VCPU_EFLAGS_PF)
        TYPE_TRACE_CHECK_RETURN(_kaf_calc_PF(context));
    if (flags & VCPU_EFLAGS_AF)
        TYPE_TRACE_CHECK_RETURN(_kaf_calc_AF(context));
    if (flags & VCPU_EFLAGS_ZF)
        TYPE_TRACE_CHECK_RETURN(_kaf_calc_ZF(context));
    if (flags & VCPU_EFLAGS_SF)
        TYPE_TRACE_CHECK_RETURN(_kaf_calc_SF(context));
    if (flags & VCPU_EFLAGS_OF)
        TYPE_TRACE_CHECK_RETURN(_kaf_calc_OF(context));
    TYPE_TRACE_CALL_END;
}
static C_VOID _kas_move_index(core_machine_cpu_execution_context *context, type_unsigned_8 byte, type_bool flagsi, type_bool flagdi)
{
    TYPE_TRACE_CALL_BEGIN("_kas_move_index");
    switch (_GetAddressSize)
    {
    case 2:
        if (_GetEFLAGS_DF)
        {
            if (flagdi)
                cpu_state.data.di -= byte;
            if (flagsi)
                cpu_state.data.si -= byte;
        }
        else
        {
            if (flagdi)
                cpu_state.data.di += byte;
            if (flagsi)
                cpu_state.data.si += byte;
        }
        break;
    case 4:
        if (_GetEFLAGS_DF)
        {
            if (flagdi)
                cpu_state.data.edi -= byte;
            if (flagsi)
                cpu_state.data.esi -= byte;
        }
        else
        {
            if (flagdi)
                cpu_state.data.edi += byte;
            if (flagsi)
                cpu_state.data.esi += byte;
        }
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}

#define _kac_arith1(funflag, type8, expr8, type16, expr16, type32, expr32) \
    do                                                                     \
    {                                                                      \
        switch (bit)                                                       \
        {                                                                  \
        case 8:                                                            \
            instruction_state.data.bit = 8;                                \
            instruction_state.data.type = (type8);                         \
            instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_8(cdest);     \
            instruction_state.data.result = TYPE_MASK_UNSIGNED_8(expr8);   \
            break;                                                         \
        case 16:                                                           \
            instruction_state.data.bit = 16;                               \
            instruction_state.data.type = (type16);                        \
            instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_16(cdest);    \
            instruction_state.data.result = TYPE_MASK_UNSIGNED_16(expr16); \
            break;                                                         \
        case 32:                                                           \
            instruction_state.data.bit = 32;                               \
            instruction_state.data.type = (type32);                        \
            instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_32(cdest);    \
            instruction_state.data.result = TYPE_MASK_UNSIGNED_32(expr32); \
            break;                                                         \
        default:                                                           \
            TYPE_TRACE_BLOCK_BEGIN("bit");                                 \
            TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));                   \
            TYPE_TRACE_BLOCK_END;                                          \
            break;                                                         \
        }                                                                  \
        TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, funflag));         \
    } while (0)
#define _kac_arith2(funflag, type8, expr8, type12, expr12, type16, expr16,            \
                    type20, expr20, type32, expr32)                                   \
    do                                                                                \
    {                                                                                 \
        switch (bit)                                                                  \
        {                                                                             \
        case 8:                                                                       \
            instruction_state.data.bit = 8;                                           \
            instruction_state.data.type = (type8);                                    \
            instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_8(cdest);                \
            instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_8(csrc);                 \
            instruction_state.data.result = TYPE_MASK_UNSIGNED_8(expr8);              \
            break;                                                                    \
        case 12:                                                                      \
            instruction_state.data.bit = 16;                                          \
            instruction_state.data.type = (type12);                                   \
            instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_16(cdest);               \
            instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_16((type_signed_8)csrc); \
            instruction_state.data.result = TYPE_MASK_UNSIGNED_16(expr12);            \
            break;                                                                    \
        case 16:                                                                      \
            instruction_state.data.bit = 16;                                          \
            instruction_state.data.type = (type16);                                   \
            instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_16(cdest);               \
            instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_16(csrc);                \
            instruction_state.data.result = TYPE_MASK_UNSIGNED_16(expr16);            \
            break;                                                                    \
        case 20:                                                                      \
            instruction_state.data.bit = 32;                                          \
            instruction_state.data.type = (type20);                                   \
            instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_32(cdest);               \
            instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_32((type_signed_8)csrc); \
            instruction_state.data.result = TYPE_MASK_UNSIGNED_32(expr20);            \
            break;                                                                    \
        case 32:                                                                      \
            instruction_state.data.bit = 32;                                          \
            instruction_state.data.type = (type32);                                   \
            instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_32(cdest);               \
            instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_32(csrc);                \
            instruction_state.data.result = TYPE_MASK_UNSIGNED_32(expr32);            \
            break;                                                                    \
        default:                                                                      \
            TYPE_TRACE_BLOCK_BEGIN("bit");                                            \
            TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));                              \
            TYPE_TRACE_BLOCK_END;                                                     \
            break;                                                                    \
        }                                                                             \
        TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, funflag));                    \
    } while (0)
static C_VOID _a_add(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_64 csrc, type_unsigned_8 bit)
{
    TYPE_TRACE_CALL_BEGIN("_a_add");
    _kac_arith2(ADD_FLAG,
                ADD8, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD16, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD16, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD32, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD32, (instruction_state.data.opr1 + instruction_state.data.opr2));
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_adc(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_64 csrc, type_unsigned_8 bit)
{
    TYPE_TRACE_CALL_BEGIN("_a_adc");
    _kac_arith2(ADC_FLAG,
                ADC8, (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF),
                ADC16, (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF),
                ADC16, (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF),
                ADC32, (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF),
                ADC32, (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF));
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_and(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_64 csrc, type_unsigned_8 bit)
{
    TYPE_TRACE_CALL_BEGIN("_a_and");
    _kac_arith2(AND_FLAG,
                AND8, (instruction_state.data.opr1 & instruction_state.data.opr2),
                AND16, (instruction_state.data.opr1 & instruction_state.data.opr2),
                AND16, (instruction_state.data.opr1 & instruction_state.data.opr2),
                AND32, (instruction_state.data.opr1 & instruction_state.data.opr2),
                AND32, (instruction_state.data.opr1 & instruction_state.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    instruction_state.data.udf |= VCPU_EFLAGS_AF;
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_or(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_64 csrc, type_unsigned_8 bit)
{
    TYPE_TRACE_CALL_BEGIN("_a_or");
    _kac_arith2(OR_FLAG,
                OR8, (instruction_state.data.opr1 | instruction_state.data.opr2),
                OR16, (instruction_state.data.opr1 | instruction_state.data.opr2),
                OR16, (instruction_state.data.opr1 | instruction_state.data.opr2),
                OR32, (instruction_state.data.opr1 | instruction_state.data.opr2),
                OR32, (instruction_state.data.opr1 | instruction_state.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    instruction_state.data.udf |= VCPU_EFLAGS_AF;
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_sbb(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_64 csrc, type_unsigned_8 bit)
{
    TYPE_TRACE_CALL_BEGIN("_a_sbb");
    _kac_arith2(SBB_FLAG,
                SBB8, (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)),
                SBB16, (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)),
                SBB16, (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)),
                SBB32, (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)),
                SBB32, (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)));
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_sub(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_64 csrc, type_unsigned_8 bit)
{
    TYPE_TRACE_CALL_BEGIN("_a_sub");
    _kac_arith2(SUB_FLAG,
                SUB8, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB16, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB16, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB32, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB32, (instruction_state.data.opr1 - instruction_state.data.opr2));
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_xor(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_64 csrc, type_unsigned_8 bit)
{
    TYPE_TRACE_CALL_BEGIN("_a_xor");
    _kac_arith2(XOR_FLAG,
                XOR8, (instruction_state.data.opr1 ^ instruction_state.data.opr2),
                XOR16, (instruction_state.data.opr1 ^ instruction_state.data.opr2),
                XOR16, (instruction_state.data.opr1 ^ instruction_state.data.opr2),
                XOR32, (instruction_state.data.opr1 ^ instruction_state.data.opr2),
                XOR32, (instruction_state.data.opr1 ^ instruction_state.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    instruction_state.data.udf |= VCPU_EFLAGS_AF;
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_cmp(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_64 csrc, type_unsigned_8 bit)
{
    TYPE_TRACE_CALL_BEGIN("_a_cmp");
    _kac_arith2(CMP_FLAG,
                CMP8, ((type_unsigned_8)instruction_state.data.opr1 - (type_signed_8)instruction_state.data.opr2),
                CMP16, ((type_unsigned_16)instruction_state.data.opr1 - (type_signed_8)instruction_state.data.opr2),
                CMP16, ((type_unsigned_16)instruction_state.data.opr1 - (type_signed_16)instruction_state.data.opr2),
                CMP32, ((type_unsigned_32)instruction_state.data.opr1 - (type_signed_8)instruction_state.data.opr2),
                CMP32, ((type_unsigned_32)instruction_state.data.opr1 - (type_signed_32)instruction_state.data.opr2));
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_test(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_64 csrc, type_unsigned_8 bit)
{
    TYPE_TRACE_CALL_BEGIN("_a_test");
    _kac_arith2(TEST_FLAG,
                TEST8, (instruction_state.data.opr1 & instruction_state.data.opr2),
                TEST16, (instruction_state.data.opr1 & instruction_state.data.opr2),
                TEST16, (instruction_state.data.opr1 & instruction_state.data.opr2),
                TEST32, (instruction_state.data.opr1 & instruction_state.data.opr2),
                TEST32, (instruction_state.data.opr1 & instruction_state.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    instruction_state.data.udf |= VCPU_EFLAGS_AF;
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_inc(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_8 bit)
{
    TYPE_TRACE_CALL_BEGIN("_a_inc");
    instruction_state.data.opr2 = 1;
    _kac_arith1(INC_FLAG,
                ADD8, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD16, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD32, (instruction_state.data.opr1 + instruction_state.data.opr2));
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_dec(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_8 bit)
{
    TYPE_TRACE_CALL_BEGIN("_a_dec");
    instruction_state.data.opr2 = 1;
    _kac_arith1(DEC_FLAG,
                SUB8, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB16, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB32, (instruction_state.data.opr1 - instruction_state.data.opr2));
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_not(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_8 bit)
{
    TYPE_TRACE_CALL_BEGIN("_a_not");
    instruction_state.data.opr2 = 0;
    _kac_arith1(NOT_FLAG,
                ARITHTYPE_NULL, (~instruction_state.data.opr1),
                ARITHTYPE_NULL, (~instruction_state.data.opr1),
                ARITHTYPE_NULL, (~instruction_state.data.opr1));
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_neg(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_8 bit)
{
    TYPE_TRACE_CALL_BEGIN("_a_neg");
    instruction_state.data.opr2 = cdest;
    cdest = 0;
    _kac_arith1(NEG_FLAG,
                SUB8, (TYPE_MASK_UNSIGNED_8(instruction_state.data.opr1) - TYPE_MASK_UNSIGNED_8(instruction_state.data.opr2)),
                SUB16, (TYPE_MASK_UNSIGNED_16(instruction_state.data.opr1) - TYPE_MASK_UNSIGNED_16(instruction_state.data.opr2)),
                SUB32, (TYPE_MASK_UNSIGNED_32(instruction_state.data.opr1) - TYPE_MASK_UNSIGNED_32(instruction_state.data.opr2)));
    TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!instruction_state.data.opr2);
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_mul(core_machine_cpu_execution_context *context, type_unsigned_64 csrc, type_unsigned_8 bit)
{
    type_unsigned_64 cdest;
    TYPE_TRACE_CALL_BEGIN("_a_mul");
    switch (bit)
    {
    case 8:
        TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = cpu_state.data.al;
        instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_8(csrc);
        cdest = TYPE_MASK_UNSIGNED_16(cpu_state.data.al * instruction_state.data.opr2);
        cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(cdest);
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF, !!cpu_state.data.ah);
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!cpu_state.data.ah);
        instruction_state.data.result = TYPE_MASK_UNSIGNED_16(cdest);
        TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = cpu_state.data.ax;
        instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_16(csrc);
        cdest = TYPE_MASK_UNSIGNED_32(cpu_state.data.ax * instruction_state.data.opr2);
        cpu_state.data.dx = TYPE_MASK_UNSIGNED_16(cdest >> 16);
        cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(cdest);
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF, !!cpu_state.data.dx);
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!cpu_state.data.dx);
        instruction_state.data.result = TYPE_MASK_UNSIGNED_32(cdest);
        TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = cpu_state.data.eax;
        instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_32(csrc);
        cdest = TYPE_MASK_UNSIGNED_64(cpu_state.data.eax * instruction_state.data.opr2);
        cpu_state.data.edx = TYPE_MASK_UNSIGNED_32(cdest >> 32);
        cpu_state.data.eax = TYPE_MASK_UNSIGNED_32(cdest);
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF, !!cpu_state.data.edx);
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!cpu_state.data.edx);
        instruction_state.data.result = TYPE_MASK_UNSIGNED_64(cdest);
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                                   VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_imul(core_machine_cpu_execution_context *context, type_unsigned_64 csrc, type_unsigned_8 bit)
{
    type_signed_64 cdest;
    TYPE_TRACE_CALL_BEGIN("_a_imul");
    switch (bit)
    {
    case 8:
        TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = cpu_state.data.al;
        instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_8((type_signed_8)csrc);
        cdest = TYPE_MASK_UNSIGNED_16((type_signed_8)cpu_state.data.al * (type_signed_8)instruction_state.data.opr2);
        cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(cdest);
        if ((type_signed_16)cdest == (type_signed_16)(type_signed_8)cpu_state.data.al)
        {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        else
        {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        instruction_state.data.result = TYPE_MASK_UNSIGNED_16(cdest);
        TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = cpu_state.data.ax;
        instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_16((type_signed_16)csrc);
        cdest = TYPE_MASK_UNSIGNED_32((type_signed_16)cpu_state.data.ax * (type_signed_16)instruction_state.data.opr2);
        cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(cdest);
        cpu_state.data.dx = TYPE_MASK_UNSIGNED_16(cdest >> 16);
        if ((type_signed_32)cdest == (type_signed_32)(type_signed_16)cpu_state.data.ax)
        {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        else
        {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        instruction_state.data.result = TYPE_MASK_UNSIGNED_32(cdest);
        TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = cpu_state.data.eax;
        instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_32((type_signed_32)csrc);
        cdest = TYPE_MASK_UNSIGNED_64((type_signed_64)(type_signed_32)cpu_state.data.eax *
                                      (type_signed_64)(type_signed_32)instruction_state.data.opr2);
        cpu_state.data.eax = TYPE_MASK_UNSIGNED_32(cdest);
        cpu_state.data.edx = TYPE_MASK_UNSIGNED_32(cdest >> 32);
        if ((type_signed_64)cdest == (type_signed_64)(type_signed_32)cpu_state.data.eax)
        {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        else
        {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        instruction_state.data.result = TYPE_MASK_UNSIGNED_64(cdest);
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                                   VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_div(core_machine_cpu_execution_context *context, type_unsigned_64 csrc, type_unsigned_8 bit)
{
    type_unsigned_64 temp = 0x0000000000000000;
    TYPE_TRACE_CALL_BEGIN("_a_div");
    switch (bit)
    {
    case 8:
        TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_16(cpu_state.data.ax);
        instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_8(csrc);
        if (!instruction_state.data.opr2)
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(0)");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            temp = TYPE_MASK_UNSIGNED_16((type_unsigned_16)instruction_state.data.opr1 / (type_unsigned_8)instruction_state.data.opr2);
            if (temp > TYPE_MAX_UNSIGNED_8)
            {
                TYPE_TRACE_BLOCK_BEGIN("temp(>0xff)");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                TYPE_TRACE_BLOCK_END;
            }
            else
            {
                cpu_state.data.al = TYPE_MASK_UNSIGNED_8(temp);
                cpu_state.data.ah = TYPE_MASK_UNSIGNED_8((type_unsigned_16)instruction_state.data.opr1 % (type_unsigned_8)instruction_state.data.opr2);
            }
            TYPE_TRACE_BLOCK_END;
        }
        instruction_state.data.result = cpu_state.data.ax;
        TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_32((cpu_state.data.dx << 16) | cpu_state.data.ax);
        instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_16(csrc);
        if (!instruction_state.data.opr2)
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(0)");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            temp = TYPE_MASK_UNSIGNED_32((type_unsigned_32)instruction_state.data.opr1 / (type_unsigned_16)instruction_state.data.opr2);
            if (temp > TYPE_MAX_UNSIGNED_16)
            {
                TYPE_TRACE_BLOCK_BEGIN("temp(>0xffff)");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                TYPE_TRACE_BLOCK_END;
            }
            else
            {
                cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(temp);
                cpu_state.data.dx = TYPE_MASK_UNSIGNED_16((type_unsigned_32)instruction_state.data.opr1 % (type_unsigned_16)instruction_state.data.opr2);
            }
            TYPE_TRACE_BLOCK_END;
        }
        instruction_state.data.result = (cpu_state.data.dx << 16) | cpu_state.data.ax;
        TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_64(((type_unsigned_64)cpu_state.data.edx << 32) | cpu_state.data.eax);
        instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_32(csrc);
        if (!instruction_state.data.opr2)
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(0)");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            temp = TYPE_MASK_UNSIGNED_64((type_unsigned_64)instruction_state.data.opr1 / (type_unsigned_32)instruction_state.data.opr2);
            if (temp > TYPE_MAX_UNSIGNED_32)
            {
                TYPE_TRACE_BLOCK_BEGIN("temp(>0xffffffff)");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                TYPE_TRACE_BLOCK_END;
            }
            else
            {
                cpu_state.data.eax = TYPE_MASK_UNSIGNED_32(temp);
                cpu_state.data.edx = TYPE_MASK_UNSIGNED_32((type_unsigned_64)instruction_state.data.opr1 % (type_unsigned_32)instruction_state.data.opr2);
            }
            TYPE_TRACE_BLOCK_END;
        }
        instruction_state.data.result = ((type_unsigned_64)cpu_state.data.edx << 32) | cpu_state.data.eax;
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF |
                                   VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_OF);
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_idiv(core_machine_cpu_execution_context *context, type_unsigned_64 csrc, type_unsigned_8 bit)
{
    type_signed_64 temp;
    type_signed_64 dividend;
    type_signed_64 divisor;
    TYPE_TRACE_CALL_BEGIN("_a_idiv");
    switch (bit)
    {
    case 8:
        TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_16((type_signed_16)cpu_state.data.ax);
        instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_8((type_signed_8)csrc);
        if (!instruction_state.data.opr2)
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(0)");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            dividend = (type_signed_16)cpu_state.data.ax;
            divisor = (type_signed_8)csrc;
            temp = dividend / divisor;
            if (temp > 0x7f || temp < -0x80)
            {
                TYPE_TRACE_BLOCK_BEGIN("temp(>0x7f/<0x80)");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                TYPE_TRACE_BLOCK_END;
            }
            else
            {
                cpu_state.data.al = TYPE_MASK_UNSIGNED_8(temp);
                cpu_state.data.ah = TYPE_MASK_UNSIGNED_8(dividend % divisor);
            }
            TYPE_TRACE_BLOCK_END;
        }
        instruction_state.data.result = cpu_state.data.ax;
        TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_32((type_signed_32)((cpu_state.data.dx << 16) | cpu_state.data.ax));
        instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_16((type_signed_16)csrc);
        if (!instruction_state.data.opr2)
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(0)");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            dividend = (type_signed_32)(((type_unsigned_32)cpu_state.data.dx << 16) | cpu_state.data.ax);
            divisor = (type_signed_16)csrc;
            if (dividend == ((type_signed_32)-2147483647 - 1) && divisor == -1)
            {
                TYPE_TRACE_BLOCK_BEGIN("temp(overflow)");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                TYPE_TRACE_BLOCK_END;
            }
            else if ((temp = dividend / divisor) > 0x7fff || temp < -0x8000)
            {
                TYPE_TRACE_BLOCK_BEGIN("temp(>0x7fff/<0x8000)");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                TYPE_TRACE_BLOCK_END;
            }
            else
            {
                cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(temp);
                cpu_state.data.dx = TYPE_MASK_UNSIGNED_16(dividend % divisor);
            }
            TYPE_TRACE_BLOCK_END;
        }
        instruction_state.data.result = ((type_unsigned_32)cpu_state.data.dx << 16) | cpu_state.data.ax;
        TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_64((type_signed_64)(((type_unsigned_64)cpu_state.data.edx << 32) | cpu_state.data.eax));
        instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_32((type_signed_32)csrc);
        if (!instruction_state.data.opr2)
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(0)");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            dividend = (type_signed_64)(((type_unsigned_64)cpu_state.data.edx << 32) | cpu_state.data.eax);
            divisor = (type_signed_32)csrc;
            if (dividend == ((type_signed_64)-9223372036854775807LL - 1LL) && divisor == -1)
            {
                TYPE_TRACE_BLOCK_BEGIN("temp(overflow)");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                TYPE_TRACE_BLOCK_END;
            }
            else if ((temp = dividend / divisor) > 0x7fffffff || temp < -0x80000000LL)
            {
                TYPE_TRACE_BLOCK_BEGIN("temp(>0x7fffffff/<0x80000000)");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                TYPE_TRACE_BLOCK_END;
            }
            else
            {
                cpu_state.data.eax = TYPE_MASK_UNSIGNED_32(temp);
                cpu_state.data.edx = TYPE_MASK_UNSIGNED_32(dividend % divisor);
            }
            TYPE_TRACE_BLOCK_END;
        }
        instruction_state.data.result = ((type_unsigned_64)cpu_state.data.edx << 32) | cpu_state.data.eax;
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF |
                                   VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_OF);
    TYPE_TRACE_CALL_END;
}

static C_VOID _a_imul3(core_machine_cpu_execution_context *context, type_unsigned_64 csrc1, type_unsigned_64 csrc2, type_unsigned_8 bit)
{
    type_signed_64 cdest;
    TYPE_TRACE_CALL_BEGIN("_a_imul3");
    switch (bit)
    {
    case 12:
        TYPE_TRACE_BLOCK_BEGIN("bit(16+8)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = (type_signed_16)csrc1;
        instruction_state.data.opr2 = (type_signed_8)csrc2;
        cdest = TYPE_MASK_UNSIGNED_32((type_signed_16)instruction_state.data.opr1 * (type_signed_8)instruction_state.data.opr2);
        instruction_state.data.result = TYPE_MASK_UNSIGNED_16(cdest);
        if (TYPE_MASK_UNSIGNED_32(cdest) != TYPE_MASK_UNSIGNED_32((type_signed_16)instruction_state.data.result))
        {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        else
        {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        TYPE_TRACE_BLOCK_BEGIN("bit(16+16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = (type_signed_16)csrc1;
        instruction_state.data.opr2 = (type_signed_16)csrc2;
        cdest = TYPE_MASK_UNSIGNED_32((type_signed_16)instruction_state.data.opr1 * (type_signed_16)instruction_state.data.opr2);
        instruction_state.data.result = TYPE_MASK_UNSIGNED_16(cdest);
        if (TYPE_MASK_UNSIGNED_32(cdest) != TYPE_MASK_UNSIGNED_32((type_signed_16)instruction_state.data.result))
        {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        else
        {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 20:
        TYPE_TRACE_BLOCK_BEGIN("bit(32+8)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = (type_signed_32)csrc1;
        instruction_state.data.opr2 = (type_signed_8)csrc2;
        cdest = TYPE_MASK_UNSIGNED_64((type_signed_32)instruction_state.data.opr1 * (type_signed_8)instruction_state.data.opr2);
        instruction_state.data.result = TYPE_MASK_UNSIGNED_32(cdest);
        if (TYPE_MASK_UNSIGNED_64(cdest) != TYPE_MASK_UNSIGNED_64((type_signed_32)instruction_state.data.result))
        {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        else
        {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        TYPE_TRACE_BLOCK_BEGIN("bit(32+32");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = (type_signed_32)csrc1;
        instruction_state.data.opr2 = (type_signed_32)csrc2;
        cdest = TYPE_MASK_UNSIGNED_64((type_signed_64)(type_signed_32)instruction_state.data.opr1 *
            (type_signed_64)(type_signed_32)instruction_state.data.opr2);
        instruction_state.data.result = TYPE_MASK_UNSIGNED_32(cdest);
        if (TYPE_MASK_UNSIGNED_64(cdest) != TYPE_MASK_UNSIGNED_64((type_signed_32)instruction_state.data.result))
        {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        else
        {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                                   VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    TYPE_TRACE_CALL_END;
}

static type_unsigned_8 _a_shift_rotate_count(core_machine_cpu_execution_context *context,
    type_unsigned_8 csrc)
{
    return core_machine_cpu_profile_has_8086_semantics(context->cpu_profile) ? csrc :
        (type_unsigned_8)(csrc & 0x1fu);
}
static C_VOID _a_rol(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_8 csrc, type_unsigned_8 bit)
{
    type_unsigned_8 count;
    type_unsigned_32 flagcf;
    TYPE_TRACE_CALL_BEGIN("_a_rol");
    switch (bit)
    {
    case 8:
        TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        count = _a_shift_rotate_count(context, csrc);
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_8(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (count == 0)
            break;
        while (count)
        {
            flagcf = !!TYPE_GET_MSB_8(instruction_state.data.result);
            instruction_state.data.result = TYPE_MASK_UNSIGNED_8(instruction_state.data.result << 1) | flagcf;
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!TYPE_GET_LSB_UNSIGNED_8(instruction_state.data.result));
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!TYPE_GET_MSB_8(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        count = _a_shift_rotate_count(context, csrc);
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_16(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (count == 0)
            break;
        while (count)
        {
            flagcf = !!TYPE_GET_MSB_16(instruction_state.data.result);
            instruction_state.data.result = TYPE_MASK_UNSIGNED_16(instruction_state.data.result << 1) | flagcf;
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!TYPE_GET_LSB_UNSIGNED_16(instruction_state.data.result));
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!TYPE_GET_MSB_16(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        count = _a_shift_rotate_count(context, csrc);
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (count == 0)
            break;
        while (count)
        {
            flagcf = !!TYPE_GET_MSB_32(instruction_state.data.result);
            instruction_state.data.result = TYPE_MASK_UNSIGNED_32(instruction_state.data.result << 1) | flagcf;
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!TYPE_GET_LSB_UNSIGNED_32(instruction_state.data.result));
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!TYPE_GET_MSB_32(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_ror(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_8 csrc, type_unsigned_8 bit)
{
    type_unsigned_8 count;
    type_unsigned_32 flagcf;
    TYPE_TRACE_CALL_BEGIN("_a_ror");
    switch (bit)
    {
    case 8:
        TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        count = _a_shift_rotate_count(context, csrc);
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_8(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (count == 0)
            break;
        while (count)
        {
            flagcf = TYPE_GET_LSB_UNSIGNED_8(instruction_state.data.result) ? TYPE_MSB_8 : 0;
            instruction_state.data.result = TYPE_MASK_UNSIGNED_8(instruction_state.data.result >> 1) | flagcf;
            count--;
        }
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!TYPE_GET_MSB_8(instruction_state.data.result));
        if (instruction_state.data.opr2 == 1)
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!TYPE_GET_MSB_8(instruction_state.data.result)) ^ (!!TYPE_GET_MSB_7(instruction_state.data.result))));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        count = _a_shift_rotate_count(context, csrc);
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_16(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (count == 0)
            break;
        while (count)
        {
            flagcf = TYPE_GET_LSB_UNSIGNED_16(instruction_state.data.result) ? TYPE_MSB_16 : 0;
            instruction_state.data.result = TYPE_MASK_UNSIGNED_16(instruction_state.data.result >> 1) | flagcf;
            count--;
        }
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!TYPE_GET_MSB_16(instruction_state.data.result));
        if (instruction_state.data.opr2 == 1)
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!TYPE_GET_MSB_16(instruction_state.data.result)) ^ (!!TYPE_GET_MSB_15(instruction_state.data.result))));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        count = _a_shift_rotate_count(context, csrc);
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (count == 0)
            break;
        while (count)
        {
            flagcf = TYPE_GET_LSB_UNSIGNED_32(instruction_state.data.result) ? TYPE_MSB_32 : 0;
            instruction_state.data.result = TYPE_MASK_UNSIGNED_32(instruction_state.data.result >> 1) | flagcf;
            count--;
        }
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!TYPE_GET_MSB_32(instruction_state.data.result));
        if (instruction_state.data.opr2 == 1)
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!TYPE_GET_MSB_32(instruction_state.data.result)) ^ (!!TYPE_GET_MSB_31(instruction_state.data.result))));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_rcl(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_8 csrc, type_unsigned_8 bit)
{
    type_unsigned_8 count;
    type_unsigned_32 flagcf;
    TYPE_TRACE_CALL_BEGIN("_a_rcl");
    switch (bit)
    {
    case 8:
        TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        count = _a_shift_rotate_count(context, csrc);
        if (!core_machine_cpu_profile_has_8086_semantics(context->cpu_profile))
            count %= 9;
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_8(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (count == 0)
            break;
        while (count)
        {
            flagcf = !!TYPE_GET_MSB_8(instruction_state.data.result);
            instruction_state.data.result = TYPE_MASK_UNSIGNED_8(instruction_state.data.result << 1) | _GetEFLAGS_CF;
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, flagcf);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!TYPE_GET_MSB_8(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        count = _a_shift_rotate_count(context, csrc);
        if (!core_machine_cpu_profile_has_8086_semantics(context->cpu_profile))
            count %= 17;
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_16(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (count == 0)
            break;
        while (count)
        {
            flagcf = !!TYPE_GET_MSB_16(instruction_state.data.result);
            instruction_state.data.result = TYPE_MASK_UNSIGNED_16(instruction_state.data.result << 1) | _GetEFLAGS_CF;
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, flagcf);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!TYPE_GET_MSB_16(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        count = (csrc & 0x1f);
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (count == 0)
            break;
        while (count)
        {
            flagcf = !!TYPE_GET_MSB_32(instruction_state.data.result);
            instruction_state.data.result = TYPE_MASK_UNSIGNED_32(instruction_state.data.result << 1) | _GetEFLAGS_CF;
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, flagcf);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!TYPE_GET_MSB_32(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_rcr(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_8 csrc, type_unsigned_8 bit)
{
    type_unsigned_8 count;
    type_unsigned_32 flagcf;
    TYPE_TRACE_CALL_BEGIN("_a_rcr");
    switch (bit)
    {
    case 8:
        TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        count = _a_shift_rotate_count(context, csrc);
        if (!core_machine_cpu_profile_has_8086_semantics(context->cpu_profile))
            count %= 9;
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_8(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (count == 0)
            break;
        if (instruction_state.data.opr2 == 1)
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!TYPE_GET_MSB_8(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        while (count)
        {
            flagcf = TYPE_GET_LSB_UNSIGNED_8(instruction_state.data.result);
            instruction_state.data.result = TYPE_MASK_UNSIGNED_8(instruction_state.data.result >> 1) | (_GetEFLAGS_CF ? TYPE_MSB_8 : 0);
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!flagcf);
            count--;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        count = _a_shift_rotate_count(context, csrc);
        if (!core_machine_cpu_profile_has_8086_semantics(context->cpu_profile))
            count %= 17;
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_16(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (count == 0)
            break;
        if (instruction_state.data.opr2 == 1)
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!TYPE_GET_MSB_16(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        while (count)
        {
            flagcf = TYPE_GET_LSB_UNSIGNED_16(instruction_state.data.result);
            instruction_state.data.result = TYPE_MASK_UNSIGNED_16(instruction_state.data.result >> 1) | (_GetEFLAGS_CF ? TYPE_MSB_16 : 0);
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!flagcf);
            count--;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        count = (csrc & 0x1f);
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (count == 0)
            break;
        if (instruction_state.data.opr2 == 1)
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!TYPE_GET_MSB_32(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        while (count)
        {
            flagcf = TYPE_GET_LSB_UNSIGNED_32(instruction_state.data.result);
            instruction_state.data.result = TYPE_MASK_UNSIGNED_32(instruction_state.data.result >> 1) | (_GetEFLAGS_CF ? TYPE_MSB_32 : 0);
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!flagcf);
            count--;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_shl(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_8 csrc, type_unsigned_8 bit)
{
    type_unsigned_8 count;
    TYPE_TRACE_CALL_BEGIN("_a_shl");
    count = _a_shift_rotate_count(context, csrc);
    instruction_state.data.opr2 = count;
    if (count >= bit)
        instruction_state.data.udf |= VCPU_EFLAGS_CF;
    switch (bit)
    {
    case 8:
        TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_8(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count)
        {
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!TYPE_GET_MSB_8(instruction_state.data.result));
            instruction_state.data.result = TYPE_MASK_UNSIGNED_8(instruction_state.data.result << 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!TYPE_GET_MSB_8(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0)
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHL_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_16(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count)
        {
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!TYPE_GET_MSB_16(instruction_state.data.result));
            instruction_state.data.result = TYPE_MASK_UNSIGNED_16(instruction_state.data.result << 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!TYPE_GET_MSB_16(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0)
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHL_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_32(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count)
        {
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!TYPE_GET_MSB_32(instruction_state.data.result));
            instruction_state.data.result = TYPE_MASK_UNSIGNED_32(instruction_state.data.result << 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!TYPE_GET_MSB_32(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0)
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHL_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_shr(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_8 csrc, type_unsigned_8 bit)
{
    type_unsigned_8 count;
    TYPE_TRACE_CALL_BEGIN("_a_shr");
    count = _a_shift_rotate_count(context, csrc);
    instruction_state.data.opr2 = count;
    if (count >= bit)
        instruction_state.data.udf |= VCPU_EFLAGS_CF;
    switch (bit)
    {
    case 8:
        TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_8(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count)
        {
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!TYPE_GET_LSB_UNSIGNED_8(instruction_state.data.result)));
            instruction_state.data.result = TYPE_MASK_UNSIGNED_8(instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF, (!!TYPE_GET_MSB_8(instruction_state.data.opr1)));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0)
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_16(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count)
        {
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!TYPE_GET_LSB_UNSIGNED_16(instruction_state.data.result)));
            instruction_state.data.result = TYPE_MASK_UNSIGNED_16(instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF, (!!TYPE_GET_MSB_16(instruction_state.data.opr1)));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0)
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_32(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count)
        {
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!TYPE_GET_LSB_UNSIGNED_32(instruction_state.data.result)));
            instruction_state.data.result = TYPE_MASK_UNSIGNED_32(instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF, (!!TYPE_GET_MSB_32(instruction_state.data.opr1)));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0)
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_sar(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_8 csrc, type_unsigned_8 bit)
{
    type_unsigned_8 count;
    type_bool tempcf;
    TYPE_TRACE_CALL_BEGIN("_a_shr");
    count = _a_shift_rotate_count(context, csrc);
    instruction_state.data.opr2 = count;
    switch (bit)
    {
    case 8:
        TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_8(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count)
        {
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!TYPE_GET_LSB_UNSIGNED_8(instruction_state.data.result)));
            tempcf = TYPE_GET_MSB_8(instruction_state.data.result);
            instruction_state.data.result = TYPE_MASK_UNSIGNED_8((type_signed_8)instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            _ClrEFLAGS_OF;
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0)
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SAR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_16(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count)
        {
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!TYPE_GET_LSB_UNSIGNED_16(instruction_state.data.result)));
            instruction_state.data.result = TYPE_MASK_UNSIGNED_16((type_signed_16)instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            _ClrEFLAGS_OF;
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0)
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SAR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count)
        {
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!TYPE_GET_LSB_UNSIGNED_32(instruction_state.data.result)));
            instruction_state.data.result = TYPE_MASK_UNSIGNED_32((type_signed_32)instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            _ClrEFLAGS_OF;
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0)
        {
            TYPE_TRACE_BLOCK_BEGIN("opr2(!0)");
            TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SAR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}

static C_VOID _p_ins(core_machine_cpu_execution_context *context, type_unsigned_8 byte)
{
    type_unsigned_32 cedi, data = 0;
    TYPE_TRACE_CALL_BEGIN("_p_ins");
    switch (_GetAddressSize)
    {
    case 2:
        cedi = cpu_state.data.di;
        break;
    case 4:
        cedi = cpu_state.data.edi;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (byte)
    {
    case 1:
        TYPE_TRACE_BLOCK_BEGIN("byte(1)");
        TYPE_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.es, cedi, 1, 1));
        TYPE_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, TYPE_REFERENCE_OF(data), 1));
        TYPE_TRACE_CHECK_RETURN(_s_write_es(context, cedi, TYPE_REFERENCE_OF(data), 1));
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 0, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        TYPE_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.es, cedi, 2, 1));
        TYPE_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, TYPE_REFERENCE_OF(data), 2));
        TYPE_TRACE_CHECK_RETURN(_s_write_es(context, cedi, TYPE_REFERENCE_OF(data), 2));
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 0, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        TYPE_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.es, cedi, 4, 1));
        TYPE_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, TYPE_REFERENCE_OF(data), 4));
        TYPE_TRACE_CHECK_RETURN(_s_write_es(context, cedi, TYPE_REFERENCE_OF(data), 4));
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 0, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("byte");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _p_outs(core_machine_cpu_execution_context *context, type_unsigned_8 byte)
{
    type_unsigned_32 cesi, data = 0;
    TYPE_TRACE_CALL_BEGIN("_p_outs");
    switch (_GetAddressSize)
    {
    case 2:
        cesi = cpu_state.data.si;
        break;
    case 4:
        cesi = cpu_state.data.esi;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (byte)
    {
    case 1:
        TYPE_TRACE_BLOCK_BEGIN("byte(1)");
        TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, TYPE_REFERENCE_OF(data), 1));
        TYPE_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx, TYPE_REFERENCE_OF(data), 1));
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 1, 0));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, TYPE_REFERENCE_OF(data), 2));
        TYPE_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx, TYPE_REFERENCE_OF(data), 2));
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 1, 0));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, TYPE_REFERENCE_OF(data), 4));
        TYPE_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx, TYPE_REFERENCE_OF(data), 4));
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 1, 0));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("byte");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _m_movs(core_machine_cpu_execution_context *context, type_unsigned_8 byte)
{
    type_unsigned_32 data = 0;
    type_unsigned_32 cesi, cedi;
    TYPE_TRACE_CALL_BEGIN("_m_movs");
    switch (_GetAddressSize)
    {
    case 2:
        cesi = cpu_state.data.si;
        cedi = cpu_state.data.di;
        break;
    case 4:
        cesi = cpu_state.data.esi;
        cedi = cpu_state.data.edi;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (byte)
    {
    case 1:
        TYPE_TRACE_BLOCK_BEGIN("byte(1)");
        TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, TYPE_REFERENCE_OF(data), 1));
        TYPE_TRACE_CHECK_RETURN(_s_write_es(context, cedi, TYPE_REFERENCE_OF(data), 1));
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 1, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, TYPE_REFERENCE_OF(data), 2));
        TYPE_TRACE_CHECK_RETURN(_s_write_es(context, cedi, TYPE_REFERENCE_OF(data), 2));
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 1, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, TYPE_REFERENCE_OF(data), 4));
        TYPE_TRACE_CHECK_RETURN(_s_write_es(context, cedi, TYPE_REFERENCE_OF(data), 4));
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 1, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("byte");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _m_stos(core_machine_cpu_execution_context *context, type_unsigned_8 byte)
{
    type_unsigned_32 cedi;
    TYPE_TRACE_CALL_BEGIN("_m_stos");
    switch (_GetAddressSize)
    {
    case 2:
        cedi = cpu_state.data.di;
        break;
    case 4:
        cedi = cpu_state.data.edi;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (byte)
    {
    case 1:
        TYPE_TRACE_BLOCK_BEGIN("byte(1)");
        TYPE_TRACE_CHECK_RETURN(_s_write_es(context, cedi, TYPE_REFERENCE_OF(cpu_state.data.al), 1));
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 0, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        TYPE_TRACE_CHECK_RETURN(_s_write_es(context, cedi, TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 0, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        TYPE_TRACE_CHECK_RETURN(_s_write_es(context, cedi, TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 0, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("byte");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _m_lods(core_machine_cpu_execution_context *context, type_unsigned_8 byte)
{
    type_unsigned_32 cesi = 0x00000000;
    TYPE_TRACE_CALL_BEGIN("_m_lods");
    switch (_GetAddressSize)
    {
    case 2:
        cesi = cpu_state.data.si;
        break;
    case 4:
        cesi = cpu_state.data.esi;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (byte)
    {
    case 1:
        TYPE_TRACE_BLOCK_BEGIN("byte(1)");
        instruction_state.data.bit = 8;
        TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, TYPE_REFERENCE_OF(cpu_state.data.al), 1));
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 1, 0));
        instruction_state.data.result = cpu_state.data.al;
        TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("byte(2)");
        instruction_state.data.bit = 16;
        TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 1, 0));
        instruction_state.data.result = cpu_state.data.ax;
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("byte(4)");
        instruction_state.data.bit = 32;
        TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 1, 0));
        instruction_state.data.result = cpu_state.data.eax;
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("byte");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_cmps(core_machine_cpu_execution_context *context, type_unsigned_8 bit)
{
    type_unsigned_32 cesi, cedi;
    TYPE_TRACE_CALL_BEGIN("_a_cmps");
    instruction_state.data.opr1 = 0;
    instruction_state.data.opr2 = 0;
    switch (_GetAddressSize)
    {
    case 2:
        cesi = cpu_state.data.si;
        cedi = cpu_state.data.di;
        break;
    case 4:
        cesi = cpu_state.data.esi;
        cedi = cpu_state.data.edi;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (bit)
    {
    case 8:
        TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.type = CMP8;
        TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, TYPE_REFERENCE_OF(instruction_state.data.opr1), 1));
        TYPE_TRACE_CHECK_RETURN(_s_read_es(context, cedi, TYPE_REFERENCE_OF(instruction_state.data.opr2), 1));
        instruction_state.data.result = TYPE_MASK_UNSIGNED_8(instruction_state.data.opr1 - instruction_state.data.opr2);
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 1, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.type = CMP16;
        TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, TYPE_REFERENCE_OF(instruction_state.data.opr1), 2));
        TYPE_TRACE_CHECK_RETURN(_s_read_es(context, cedi, TYPE_REFERENCE_OF(instruction_state.data.opr2), 2));
        instruction_state.data.result = TYPE_MASK_UNSIGNED_16(instruction_state.data.opr1 - instruction_state.data.opr2);
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 1, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.type = CMP32;
        TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, TYPE_REFERENCE_OF(instruction_state.data.opr1), 4));
        TYPE_TRACE_CHECK_RETURN(_s_read_es(context, cedi, TYPE_REFERENCE_OF(instruction_state.data.opr2), 4));
        instruction_state.data.result = TYPE_MASK_UNSIGNED_32(instruction_state.data.opr1 - instruction_state.data.opr2);
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 1, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, CMP_FLAG));
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_scas(core_machine_cpu_execution_context *context, type_unsigned_8 bit)
{
    type_unsigned_32 cedi;
    TYPE_TRACE_CALL_BEGIN("_a_scas");
    instruction_state.data.opr1 = 0;
    instruction_state.data.opr2 = 0;
    switch (_GetAddressSize)
    {
    case 2:
        cedi = cpu_state.data.di;
        break;
    case 4:
        cedi = cpu_state.data.edi;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (bit)
    {
    case 8:
        TYPE_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.type = CMP8;
        TYPE_TRACE_CHECK_RETURN(instruction_state.data.opr1 = cpu_state.data.al);
        TYPE_TRACE_CHECK_RETURN(_s_read_es(context, cedi, TYPE_REFERENCE_OF(instruction_state.data.opr2), 1));
        instruction_state.data.result = TYPE_MASK_UNSIGNED_8(instruction_state.data.opr1 - instruction_state.data.opr2);
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 0, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 16:
        TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.type = CMP16;
        TYPE_TRACE_CHECK_RETURN(instruction_state.data.opr1 = cpu_state.data.ax);
        TYPE_TRACE_CHECK_RETURN(_s_read_es(context, cedi, TYPE_REFERENCE_OF(instruction_state.data.opr2), 2));
        instruction_state.data.result = TYPE_MASK_UNSIGNED_16(instruction_state.data.opr1 - instruction_state.data.opr2);
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 0, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.type = CMP32;
        TYPE_TRACE_CHECK_RETURN(instruction_state.data.opr1 = cpu_state.data.eax);
        TYPE_TRACE_CHECK_RETURN(_s_read_es(context, cedi, TYPE_REFERENCE_OF(instruction_state.data.opr2), 4));
        instruction_state.data.result = TYPE_MASK_UNSIGNED_32(instruction_state.data.opr1 - instruction_state.data.opr2);
        TYPE_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 0, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, CMP_FLAG));
    TYPE_TRACE_CALL_END;
}
#define _adv TYPE_TRACE_CHECK_RETURN(_d_skip(context, 1))

static type_bool core_machine_cpu_instruction_lexeme_is_prefix(
    type_unsigned_8 byte, core_machine_cpu_profile profile)
{
    switch (byte) {
    case 0x26u: case 0x2eu: case 0x36u: case 0x3eu:
    case 0xf0u: case 0xf2u: case 0xf3u:
        return TYPE_TRUE;
    case 0x64u: case 0x65u: case 0x66u: case 0x67u:
        return profile >= CORE_MACHINE_CPU_PROFILE_80386;
    default:
        return TYPE_FALSE;
    }
}

static type_bool core_machine_cpu_instruction_lexeme_has_modrm(
    type_unsigned_8 opcode, type_bool extended)
{
    if (extended) {
        return opcode <= 0x03u ||
            (opcode >= 0x20u && opcode <= 0x24u) || opcode == 0x26u ||
            (opcode >= 0x90u && opcode <= 0x9fu) || opcode == 0xa3u ||
            (opcode >= 0xa4u && opcode <= 0xa5u) || opcode == 0xabu ||
            (opcode >= 0xacu && opcode <= 0xadu) || opcode == 0xafu ||
            (opcode >= 0xb0u && opcode <= 0xbfu);
    }
    return (opcode <= 0x03u) || (opcode >= 0x08u && opcode <= 0x0bu) ||
        (opcode >= 0x10u && opcode <= 0x13u) ||
        (opcode >= 0x18u && opcode <= 0x1bu) ||
        (opcode >= 0x20u && opcode <= 0x23u) ||
        (opcode >= 0x28u && opcode <= 0x2bu) ||
        (opcode >= 0x30u && opcode <= 0x33u) ||
        (opcode >= 0x38u && opcode <= 0x3bu) || opcode == 0x62u ||
        opcode == 0x63u || opcode == 0x69u || opcode == 0x6bu ||
        (opcode >= 0x80u && opcode <= 0x8fu) || opcode == 0xc0u ||
        opcode == 0xc1u || (opcode >= 0xc4u && opcode <= 0xc7u) ||
        (opcode >= 0xd0u && opcode <= 0xd3u) ||
        (opcode >= 0xd8u && opcode <= 0xdfu) || opcode == 0xf6u ||
        opcode == 0xf7u || opcode == 0xfeu || opcode == 0xffu;
}

static type_bool core_machine_cpu_instruction_lexeme_modrm_form_valid(
    type_unsigned_8 opcode, type_bool extended, type_unsigned_8 modrm,
    type_bool cpu_80386_cr_mov_ignores_mod)
{
    type_unsigned_8 reg = (modrm >> 3u) & 7u;

    if (extended) {
        if ((opcode == 0x21u || opcode == 0x23u) && (reg == 4u || reg == 5u)) return TYPE_FALSE;
        if ((opcode == 0x24u || opcode == 0x26u) && reg < 6u) return TYPE_FALSE;
        if (opcode == 0x01u && (reg == 5u || reg == 7u)) return TYPE_FALSE;
        if (opcode >= 0x20u && opcode <= 0x24u || opcode == 0x26u) {
            if ((opcode == 0x20u || opcode == 0x22u) &&
                cpu_80386_cr_mov_ignores_mod) return TYPE_TRUE;
            return (modrm >> 6u) == 3u;
        }
        if ((opcode == 0x01u && reg <= 3u) || opcode == 0xb2u ||
            opcode == 0xb4u || opcode == 0xb5u)
            return (modrm >> 6u) != 3u;
        return TYPE_TRUE;
    }
    if (opcode == 0x62u || opcode == 0x8du || opcode == 0xc4u || opcode == 0xc5u)
        return (modrm >> 6u) != 3u;
    if (opcode == 0xffu && (reg == 3u || reg == 5u))
        return (modrm >> 6u) != 3u;
    if (opcode == 0x8fu || opcode == 0xc6u || opcode == 0xc7u)
        return reg == 0u;
    if (opcode == 0xf6u || opcode == 0xf7u)
        return reg != 1u;
    if (opcode == 0xfeu)
        return reg <= 1u;
    if (opcode == 0xffu)
        return reg <= 6u;
    return TYPE_TRUE;
}
static type_unsigned_8 core_machine_cpu_instruction_lexeme_immediate_bytes(
    type_unsigned_8 opcode, type_bool extended, type_unsigned_8 modrm,
    type_unsigned_8 operand_bytes, type_unsigned_8 address_bytes)
{
    if (extended) {
        if (opcode >= 0x80u && opcode <= 0x8fu) return operand_bytes;
        if (opcode == 0xa4u || opcode == 0xacu) return 1u;
        if (opcode == 0xbau) return 1u;
        return 0u;
    }
    if ((opcode & 0xc7u) == 0x04u || opcode == 0x6au || opcode == 0x6bu ||
        (opcode >= 0x70u && opcode <= 0x7fu) || opcode == 0x80u ||
        opcode == 0x82u || opcode == 0x83u || opcode == 0xa8u ||
        (opcode >= 0xb0u && opcode <= 0xb7u) || opcode == 0xc0u || opcode == 0xc6u ||
        opcode == 0xc1u || opcode == 0xcdu || opcode == 0xd4u ||
        opcode == 0xd5u || (opcode >= 0xe0u && opcode <= 0xe3u) ||
        (opcode >= 0xe4u && opcode <= 0xe7u) || opcode == 0xebu ||
        (opcode == 0xf6u && ((modrm >> 3u) & 7u) <= 1u)) return 1u;
    if ((opcode & 0xc7u) == 0x05u || opcode == 0x68u || opcode == 0x69u ||
        opcode == 0x81u || opcode == 0xa9u ||
        (opcode >= 0xb8u && opcode <= 0xbfu) || opcode == 0xc7u ||
        opcode == 0xe8u || opcode == 0xe9u ||
        (opcode == 0xf7u && ((modrm >> 3u) & 7u) <= 1u)) return operand_bytes;
    if (opcode >= 0xa0u && opcode <= 0xa3u) return address_bytes;
    if (opcode == 0x9au || opcode == 0xeau) return operand_bytes + 2u;
    if (opcode == 0xc2u || opcode == 0xcau) return 2u;
    if (opcode == 0xc8u) return 3u;
    return 0u;
}

static type_bool core_machine_cpu_instruction_lexeme_scan_with_options(
    const type_unsigned_8 *bytes, type_unsigned_8 available_bytes,
    core_machine_cpu_profile profile, type_bool code_32,
    type_bool cpu_80386_cr_mov_ignores_mod,
    core_machine_cpu_instruction_lexeme *out_lexeme)
{
    type_unsigned_8 index = 0u;
    type_unsigned_8 components = 0u;
    type_unsigned_8 opcode;
    type_unsigned_8 modrm = 0u;
    type_unsigned_8 operand_bytes;
    type_unsigned_8 address_bytes;
    type_unsigned_8 displacement = 0u;
    type_unsigned_8 immediate;
    type_bool extended = TYPE_FALSE;
    type_bool lock_prefix = TYPE_FALSE;
    type_bool operand_size_prefix = TYPE_FALSE;
    type_bool address_size_prefix = TYPE_FALSE;
    core_machine_cpu_instruction_metadata metadata;

    if (out_lexeme == STD_NULL) return TYPE_FALSE;
    *out_lexeme = (core_machine_cpu_instruction_lexeme) { 0u, 0u, TYPE_FALSE };
    if (bytes == STD_NULL || available_bytes == 0u ||
        profile < CORE_MACHINE_CPU_PROFILE_8086 ||
        profile > CORE_MACHINE_CPU_PROFILE_80386) return TYPE_FALSE;
    while (index < available_bytes && index < 15u &&
        core_machine_cpu_instruction_lexeme_is_prefix(bytes[index], profile)) {
        if (bytes[index] == 0xf0u) lock_prefix = TYPE_TRUE;
        ++index;
        ++components;
    }
    if (index < available_bytes && profile < CORE_MACHINE_CPU_PROFILE_80386 &&
        (bytes[index] == 0x64u || bytes[index] == 0x65u ||
         bytes[index] == 0x66u || bytes[index] == 0x67u)) return TYPE_FALSE;
    if (index >= available_bytes || index >= 15u) return TYPE_FALSE;
    opcode = bytes[index++];
    ++components;
    operand_bytes = code_32 ? 4u : 2u;
    address_bytes = code_32 ? 4u : 2u;
    if (profile >= CORE_MACHINE_CPU_PROFILE_80386) {
        type_unsigned_8 prefix_index;

        for (prefix_index = 0u; prefix_index < index - 1u; ++prefix_index) {
            if (bytes[prefix_index] == 0x66u) operand_size_prefix = TYPE_TRUE;
            if (bytes[prefix_index] == 0x67u) address_size_prefix = TYPE_TRUE;
        }
        if (operand_size_prefix)
            operand_bytes = code_32 ? 2u : 4u;
        if (address_size_prefix)
            address_bytes = code_32 ? 2u : 4u;
    }
    if (opcode == 0x0fu && !core_machine_cpu_profile_has_8086_semantics(profile)) {
        if (profile != CORE_MACHINE_CPU_PROFILE_80286 &&
            profile != CORE_MACHINE_CPU_PROFILE_80386) return TYPE_FALSE;
        if (index >= available_bytes || index >= 15u) return TYPE_FALSE;
        opcode = bytes[index++];
        ++components;
        extended = TYPE_TRUE;
    }
    if (core_machine_cpu_instruction_lexeme_has_modrm(opcode, extended)) {
        type_unsigned_8 mod;
        type_unsigned_8 rm;

        if (index >= available_bytes || index >= 15u) return TYPE_FALSE;
        modrm = bytes[index++];
        ++components;
        mod = modrm >> 6u;
        rm = modrm & 7u;
        if (mod != 3u && !(extended && cpu_80386_cr_mov_ignores_mod &&
            (opcode == 0x20u || opcode == 0x22u))) {
            if (address_bytes == 4u) {
                if (rm == 4u) {
                    type_unsigned_8 sib;

                    if (index >= available_bytes || index >= 15u) return TYPE_FALSE;
                    sib = bytes[index++];
                    ++components;
                    if (mod == 0u && (sib & 7u) == 5u) displacement = 4u;
                } else if (mod == 0u && rm == 5u) {
                    displacement = 4u;
                }
                if (mod == 1u) displacement = 1u;
                if (mod == 2u) displacement = 4u;
            } else {
                if (mod == 0u && rm == 6u) displacement = 2u;
                if (mod == 1u) displacement = 1u;
                if (mod == 2u) displacement = 2u;
            }
        }
    }
    if (!core_machine_cpu_instruction_lexeme_modrm_form_valid(opcode, extended, modrm,
        cpu_80386_cr_mov_ignores_mod))
        return TYPE_FALSE;
    metadata = core_machine_cpu_instruction_metadata_get(extended ?
        CORE_MACHINE_CPU_INSTRUCTION_0F : CORE_MACHINE_CPU_INSTRUCTION_PRIMARY,
        opcode, modrm);
    if (!metadata.valid || profile < metadata.minimum_cpu || lock_prefix) {
        return TYPE_FALSE;
    }
    immediate = core_machine_cpu_instruction_lexeme_immediate_bytes(opcode,
        extended, modrm, operand_bytes, address_bytes);
    if ((type_unsigned_16)index + displacement + immediate > available_bytes ||
        (type_unsigned_16)index + displacement + immediate > 15u) return TYPE_FALSE;
    if (displacement != 0u) {
        index = (type_unsigned_8)(index + displacement);
        ++components;
    }
    if (immediate != 0u) {
        index = (type_unsigned_8)(index + immediate);
        ++components;
    }
    out_lexeme->byte_count = index;
    out_lexeme->component_count = components;
    out_lexeme->available = TYPE_TRUE;
    return TYPE_TRUE;
}

core_machine_cpu_instruction_metadata core_machine_cpu_instruction_metadata_get(
    core_machine_cpu_instruction_space space, type_unsigned_8 opcode, type_unsigned_8 modrm)
{
    core_machine_cpu_instruction_metadata metadata = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_FPU_PROFILE_NONE, 1};

    switch (space)
    {
    case CORE_MACHINE_CPU_INSTRUCTION_PRIMARY:
        if (opcode >= 0xd8u && opcode <= 0xdfu)
        {
            metadata.minimum_fpu = CORE_MACHINE_FPU_PROFILE_8087;
        }
        else if ((opcode >= 0x60u && opcode <= 0x62u) || opcode == 0x68u ||
                 opcode == 0x69u || opcode == 0x6au || opcode == 0x6bu ||
                 (opcode >= 0x6cu && opcode <= 0x6fu) || opcode == 0xc0u ||
                 opcode == 0xc1u || opcode == 0xc8u || opcode == 0xc9u)
        {
            metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80186;
        }
        else if (opcode == 0x63u)
        {
            metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80286;
        }
        else if (opcode >= 0x64u && opcode <= 0x67u)
        {
            metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80386;
        }
        else if (opcode == 0xd6u || opcode == 0xf1u)
        {
            metadata.valid = 0;
        }
        else if (opcode == 0x8cu)
        {
            if (((modrm >> 3u) & 7u) >= 6u) metadata.valid = 0;
            else if (((modrm >> 3u) & 7u) >= 4u)
                metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80386;
        }
        else if (opcode == 0x8eu)
        {
            if (((modrm >> 3u) & 7u) == 1u ||
                ((modrm >> 3u) & 7u) >= 6u) metadata.valid = 0;
            else if (((modrm >> 3u) & 7u) >= 4u)
                metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80386;
        }
        else if (opcode >= 0xd0u && opcode <= 0xd3u &&
                 ((modrm >> 3u) & 7u) == 6u)
        {
            metadata.valid = 0;
        }
        break;
    case CORE_MACHINE_CPU_INSTRUCTION_0F:
        metadata.valid = 0;
        if (opcode == 0x00u)
        {
            metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80286;
            metadata.valid = ((modrm >> 3u) & 7u) <= 5u;
        }
        else if (opcode == 0x01u)
        {
            metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80286;
            metadata.valid = ((modrm >> 3u) & 7u) <= 6u;
        }
        else if (opcode == 0x02u || opcode == 0x03u)
        {
            metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80286;
            metadata.valid = 1;
        }
        else if (opcode == 0x06u)
        {
            metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80286;
            metadata.valid = 1;
        }
        else if ((opcode >= 0x20u && opcode <= 0x24u) || opcode == 0x26u ||
                 (opcode >= 0x80u && opcode <= 0x8fu) ||
                 (opcode >= 0x90u && opcode <= 0x9fu) ||
                 opcode == 0xa0u || opcode == 0xa1u || opcode == 0xa3u ||
                 opcode == 0xa4u || opcode == 0xa5u || opcode == 0xa8u ||
                 opcode == 0xa9u || opcode == 0xabu || opcode == 0xacu ||
                 opcode == 0xadu || opcode == 0xafu ||
                 (opcode >= 0xb2u && opcode <= 0xb7u) ||
                 (opcode >= 0xbbu && opcode <= 0xbfu))
        {
            metadata.minimum_cpu = CORE_MACHINE_CPU_PROFILE_80386;
            metadata.valid = 1;
        }
        else if (opcode == 0xbau)
        {
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

static C_INT core_machine_cpu_profile_allows_form(
    const core_machine_cpu_execution_context *context,
    core_machine_cpu_instruction_space space, type_unsigned_8 opcode, type_unsigned_8 modrm)
{
    core_machine_cpu_instruction_metadata metadata;

    if (context == STD_NULL)
        return 0;
    metadata = core_machine_cpu_instruction_metadata_get(space, opcode, modrm);
    return metadata.valid && context->cpu_profile >= metadata.minimum_cpu;
}

type_bool core_machine_cpu_instruction_lexeme_scan(
    const type_unsigned_8 *bytes, type_unsigned_8 available_bytes,
    core_machine_cpu_profile profile, type_bool code_32,
    core_machine_cpu_instruction_lexeme *out_lexeme)
{
    return core_machine_cpu_instruction_lexeme_scan_with_options(bytes,
        available_bytes, profile, code_32, TYPE_FALSE, out_lexeme);
}
static C_VOID UndefinedOpcode(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("UndefinedOpcode");
    cpu_state = instruction_state.data.oldcpu;
    TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
    TYPE_TRACE_CALL_END;
}
static C_VOID FPU_ESCAPE(core_machine_cpu_execution_context *context)
{
    core_machine_fpu_escape_action action;
    core_machine_cpu_instruction_metadata metadata;
    core_machine_fpu_operation_metadata fpu_metadata;
    type_unsigned_8 escape_opcode;
    type_unsigned_8 modrm;
    type_unsigned_32 fpu_m32;

    TYPE_TRACE_CALL_BEGIN("FPU_ESCAPE");
    TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip,
                                       TYPE_REFERENCE_OF(escape_opcode), 1));
    _adv;
    TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip,
                                       TYPE_REFERENCE_OF(modrm), 1));
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
    metadata = core_machine_cpu_instruction_metadata_get(
        CORE_MACHINE_CPU_INSTRUCTION_FPU_ESCAPE, escape_opcode, modrm);
    if (!metadata.valid)
    {
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
    }
    if (_GetCR0_EM || _GetCR0_TS)
    {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NM(0));
    }
    else
    {
        action = core_machine_fpu_escape_dispatch(context->fpu,
            context->cpu_profile, escape_opcode, modrm);
        if (action == CORE_MACHINE_FPU_ESCAPE_UNSUPPORTED)
        {
            TYPE_TRACE_CHECK_RETURN(_SetExcept_FPU_UNSUPPORTED(0));
        }
        else if (action != CORE_MACHINE_FPU_ESCAPE_CONSUME_NONE &&
            !context->preview_mode && context->transaction != STD_NULL &&
            core_machine_transaction_begin(context->transaction,
                CORE_MACHINE_TRANSACTION_OWNER_CPU,
                CORE_MACHINE_TRANSACTION_CPU_FPU_COMMAND, escape_opcode, modrm,
                (type_unsigned_32)context->fpu->profile) == TYPE_STATUS_OK)
        {
            core_machine_transaction_commit(context->transaction);
        }
        if (action == CORE_MACHINE_FPU_ESCAPE_EXECUTE_8087)
        {
            fpu_metadata = core_machine_fpu_operation_metadata_get(escape_opcode, modrm);
            switch (fpu_metadata.operation)
            {
            case CORE_MACHINE_FPU_OPERATION_FNINIT:
                core_machine_fpu_reset(context->fpu);
                break;
            case CORE_MACHINE_FPU_OPERATION_FLD_M32:
                TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 4));
                (C_VOID)core_machine_fpu_load_m32(context->fpu,
                    TYPE_MASK_UNSIGNED_32(instruction_state.data.crm));
                break;
            case CORE_MACHINE_FPU_OPERATION_FSTP_M32:
                if (core_machine_fpu_store_m32(context->fpu, &fpu_m32) ==
                    CORE_MACHINE_FPU_EXECUTE_COMPLETED) {
                    instruction_state.data.crm = fpu_m32;
                    TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 4));
                }
                break;
            case CORE_MACHINE_FPU_OPERATION_FLDCW_M16:
                TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
                core_machine_fpu_load_control_word(context->fpu,
                    TYPE_MASK_UNSIGNED_16(instruction_state.data.crm));
                break;
            case CORE_MACHINE_FPU_OPERATION_FADD_ST0_STI:
            case CORE_MACHINE_FPU_OPERATION_FMUL_ST0_STI:
            case CORE_MACHINE_FPU_OPERATION_FSUB_ST0_STI:
            case CORE_MACHINE_FPU_OPERATION_FDIV_ST0_STI:
                (C_VOID)core_machine_fpu_binary_st0_sti(context->fpu,
                    fpu_metadata.operation, (type_unsigned_8)(modrm & 7u));
                break;
            default: break;
            }
            /* Arithmetic coverage is deliberately partial.  An operation
             * outside that semantic subset remains a valid coprocessor
             * command handoff; it is not a CPU #UD or model-only fault. */
        }
        if (action != CORE_MACHINE_FPU_ESCAPE_CONSUME_NONE) {
            core_machine_fpu_begin_command(context->fpu, escape_opcode, modrm);
        }
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID ADD_RM8_R8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("ADD_RM8_R8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID ADD_RM32_R32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("ADD_RM32_R32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID ADD_R8_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("ADD_R8_RM8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID ADD_R32_RM32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("ADD_R32_RM32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID ADD_AL_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("ADD_AL_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_add(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = TYPE_MASK_UNSIGNED_8(instruction_state.data.result);
    TYPE_TRACE_CALL_END;
}
static C_VOID ADD_EAX_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("ADD_EAX_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            TYPE_TRACE_CHECK_RETURN(_a_add(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            TYPE_TRACE_CHECK_RETURN(_a_add(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_add(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_ES(core_machine_cpu_execution_context *context)
{
    type_unsigned_32 xs_sel;
    TYPE_TRACE_CALL_BEGIN("PUSH_ES");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    xs_sel = cpu_state.data.es.selector;
    TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_ES(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("POP_ES");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_e_pop_sreg(context, &cpu_state.data.es,
        _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID OR_RM8_R8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("OR_RM8_R8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID OR_RM32_R32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("OR_RM32_R32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID OR_R8_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("OR_R8_RM8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID OR_R32_RM32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("OR_R32_RM32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID OR_AL_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("OR_AL_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_or(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = TYPE_MASK_UNSIGNED_8(instruction_state.data.result);
    TYPE_TRACE_CALL_END;
}
static C_VOID OR_EAX_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("OR_EAX_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            TYPE_TRACE_CHECK_RETURN(_a_or(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            TYPE_TRACE_CHECK_RETURN(_a_or(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_or(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_CS(core_machine_cpu_execution_context *context)
{
    type_unsigned_32 xs_sel;
    TYPE_TRACE_CALL_BEGIN("PUSH_CS");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    xs_sel = cpu_state.data.cs.selector;
    TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_CS(core_machine_cpu_execution_context *context)
{
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    type_unsigned_32 xs_sel;
    TYPE_TRACE_CALL_BEGIN("POP_CS");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_s_load_cs(context, TYPE_MASK_UNSIGNED_16(xs_sel)));
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_0F(core_machine_cpu_execution_context *context)
{
    type_unsigned_8 opcode = 0x00;
    type_unsigned_8 modrm = 0x00;
    TYPE_TRACE_CALL_BEGIN("INS_0F");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80286)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, TYPE_REFERENCE_OF(opcode), 1));
        if (opcode == 0x00u || opcode == 0x01u || opcode == 0xbau)
        {
            TYPE_TRACE_CHECK_RETURN(_s_read_cs(context,
                                               cpu_state.data.eip + 1u, TYPE_REFERENCE_OF(modrm), 1));
        }
        if (!core_machine_cpu_profile_allows_form(context,
                                                  CORE_MACHINE_CPU_INSTRUCTION_0F, opcode, modrm))
        {
            UndefinedOpcode(context);
            TYPE_TRACE_CALL_END;
            return;
        }
        TYPE_TRACE_CHECK_RETURN(ExecCpuInstruction(instruction_state.connect.insTable_0f[opcode]));
    }
    else if (core_machine_cpu_profile_has_8086_semantics(context->cpu_profile))
        POP_CS(context);
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID ADC_RM8_R8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("ADC_RM8_R8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID ADC_RM32_R32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("ADC_RM32_R32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID ADC_R8_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("ADC_R8_RM8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID ADC_R32_RM32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("ADC_R32_RM32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID ADC_AL_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("ADC_AL_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_adc(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = TYPE_MASK_UNSIGNED_8(instruction_state.data.result);
    TYPE_TRACE_CALL_END;
}
static C_VOID ADC_EAX_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("ADC_EAX_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            TYPE_TRACE_CHECK_RETURN(_a_adc(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            TYPE_TRACE_CHECK_RETURN(_a_adc(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_adc(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_SS(core_machine_cpu_execution_context *context)
{
    type_unsigned_32 xs_sel;
    TYPE_TRACE_CALL_BEGIN("PUSH_SS");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    xs_sel = cpu_state.data.ss.selector;
    TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_SS(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("POP_SS");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_e_pop_sreg(context, &cpu_state.data.ss,
        _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID SBB_RM8_R8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SBB_RM8_R8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID SBB_RM32_R32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SBB_RM32_R32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID SBB_R8_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SBB_R8_RM8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID SBB_R32_RM32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SBB_R32_RM32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID SBB_AL_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SBB_AL_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_sbb(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = TYPE_MASK_UNSIGNED_8(instruction_state.data.result);
    TYPE_TRACE_CALL_END;
}
static C_VOID SBB_EAX_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SBB_EAX_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            TYPE_TRACE_CHECK_RETURN(_a_sbb(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            TYPE_TRACE_CHECK_RETURN(_a_sbb(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_sbb(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_DS(core_machine_cpu_execution_context *context)
{
    type_unsigned_32 xs_sel;
    TYPE_TRACE_CALL_BEGIN("PUSH_DS");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    xs_sel = cpu_state.data.ds.selector;
    TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_DS(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("POP_DS");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_e_pop_sreg(context, &cpu_state.data.ds,
        _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID AND_RM8_R8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("AND_RM8_R8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID AND_RM32_R32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("AND_RM32_R32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID AND_R8_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("AND_R8_RM8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID AND_R32_RM32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("AND_R32_RM32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID AND_AL_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("AND_AL_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_and(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = TYPE_MASK_UNSIGNED_8(instruction_state.data.result);
    TYPE_TRACE_CALL_END;
}
static C_VOID AND_EAX_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("AND_EAX_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            TYPE_TRACE_CHECK_RETURN(_a_and(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            TYPE_TRACE_CHECK_RETURN(_a_and(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_and(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_ES(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_ES");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.es;
        instruction_state.data.roverss = &cpu_state.data.es;
    }
    else
    {
        cpu_state.data.ip++;
        instruction_state.data.roverds = &cpu_state.data.es;
        instruction_state.data.roverss = &cpu_state.data.es;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID DAA(core_machine_cpu_execution_context *context)
{
    type_unsigned_8 oldAL = cpu_state.data.al;
    TYPE_TRACE_CALL_BEGIN("DAA");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
        _adv;
    else
        cpu_state.data.ip++;
    if (((cpu_state.data.al & 0x0f) > 0x09) || _GetEFLAGS_AF)
    {
        cpu_state.data.al += 0x06;
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                      _GetEFLAGS_CF || ((cpu_state.data.al < oldAL) || (cpu_state.data.al < 0x06)));
        _SetEFLAGS_AF;
    }
    else
        _ClrEFLAGS_AF;
    if (((cpu_state.data.al & 0xf0) > 0x90) || _GetEFLAGS_CF)
    {
        cpu_state.data.al += 0x60;
        _SetEFLAGS_CF;
    }
    else
        _ClrEFLAGS_CF;
    instruction_state.data.bit = 8;
    instruction_state.data.result = (type_unsigned_32)cpu_state.data.al;
    TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, DAA_FLAG));
    instruction_state.data.udf |= VCPU_EFLAGS_OF;
    TYPE_TRACE_CALL_END;
}
static C_VOID SUB_RM8_R8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SUB_RM8_R8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID SUB_RM32_R32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SUB_RM32_R32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID SUB_R8_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SUB_R8_RM8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID SUB_R32_RM32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SUB_R32_RM32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID SUB_AL_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SUB_AL_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_sub(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = TYPE_MASK_UNSIGNED_8(instruction_state.data.result);
    TYPE_TRACE_CALL_END;
}
static C_VOID SUB_EAX_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SUB_EAX_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            TYPE_TRACE_CHECK_RETURN(_a_sub(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            TYPE_TRACE_CHECK_RETURN(_a_sub(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_sub(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_CS(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_CS");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.cs;
        instruction_state.data.roverss = &cpu_state.data.cs;
    }
    else
    {
        cpu_state.data.ip++;
        instruction_state.data.roverds = &cpu_state.data.cs;
        instruction_state.data.roverss = &cpu_state.data.cs;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID DAS(core_machine_cpu_execution_context *context)
{
    type_unsigned_8 oldAL = cpu_state.data.al;
    TYPE_TRACE_CALL_BEGIN("DAS");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
        _adv;
    else
        cpu_state.data.ip++;
    if (((cpu_state.data.al & 0x0f) > 0x09) || _GetEFLAGS_AF)
    {
        cpu_state.data.al -= 0x06;
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                      _GetEFLAGS_CF || (oldAL < 0x06));
        _SetEFLAGS_AF;
    }
    else
        _ClrEFLAGS_AF;
    if ((cpu_state.data.al > 0x9f) || _GetEFLAGS_CF)
    {
        cpu_state.data.al -= 0x60;
        _SetEFLAGS_CF;
    }
    else
        _ClrEFLAGS_CF;
    instruction_state.data.bit = 8;
    instruction_state.data.result = (type_unsigned_32)cpu_state.data.al;
    TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, DAS_FLAG));
    instruction_state.data.udf |= VCPU_EFLAGS_OF;
    TYPE_TRACE_CALL_END;
}
static C_VOID XOR_RM8_R8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("XOR_RM8_R8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID XOR_RM32_R32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("XOR_RM32_R32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID XOR_R8_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("XOR_R8_RM8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID XOR_R32_RM32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("XOR_R32_RM32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID XOR_AL_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("XOR_AL_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_xor(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    cpu_state.data.al = TYPE_MASK_UNSIGNED_8(instruction_state.data.result);
    TYPE_TRACE_CALL_END;
}
static C_VOID XOR_EAX_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("XOR_EAX_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperxorSize(2)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            TYPE_TRACE_CHECK_RETURN(_a_xor(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperxorSize(4)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            TYPE_TRACE_CHECK_RETURN(_a_xor(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            cpu_state.data.eax = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_xor(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_SS(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_SS");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.ss;
        instruction_state.data.roverss = &cpu_state.data.ss;
    }
    else
    {
        cpu_state.data.ip++;
        instruction_state.data.roverds = &cpu_state.data.ss;
        instruction_state.data.roverss = &cpu_state.data.ss;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID AAA(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("AAA");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    if (((cpu_state.data.al & 0x0f) > 0x09) || _GetEFLAGS_AF)
    {
        cpu_state.data.al += 0x06;
        cpu_state.data.ah += 0x01;
        _SetEFLAGS_AF;
        _SetEFLAGS_CF;
    }
    else
    {
        _ClrEFLAGS_AF;
        _ClrEFLAGS_CF;
    }
    cpu_state.data.al &= 0x0f;
    instruction_state.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF);
    TYPE_TRACE_CALL_END;
}
static C_VOID CMP_RM8_R8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("CMP_RM8_R8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    TYPE_TRACE_CALL_END;
}
static C_VOID CMP_RM32_R32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("CMP_RM32_R32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cr, 16));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID CMP_R8_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("CMP_R8_RM8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    TYPE_TRACE_CALL_END;
}
static C_VOID CMP_R32_RM32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("CMP_R32_RM32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.cr, instruction_state.data.crm, 16));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID CMP_AL_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("CMP_AL_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_cmp(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    TYPE_TRACE_CALL_END;
}
static C_VOID CMP_EAX_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("CMP_EAX_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            TYPE_TRACE_CHECK_RETURN(_a_cmp(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            TYPE_TRACE_CHECK_RETURN(_a_cmp(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_cmp(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_DS(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_DS");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.ds;
        instruction_state.data.roverss = &cpu_state.data.ds;
    }
    else
    {
        cpu_state.data.ip++;
        instruction_state.data.roverds = &cpu_state.data.ds;
        instruction_state.data.roverss = &cpu_state.data.ds;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID AAS(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("AAS");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    if (((cpu_state.data.al & 0x0f) > 0x09) || _GetEFLAGS_AF)
    {
        cpu_state.data.al -= 0x06;
        cpu_state.data.ah -= 0x01;
        _SetEFLAGS_AF;
        _SetEFLAGS_CF;
    }
    else
    {
        _ClrEFLAGS_CF;
        _ClrEFLAGS_AF;
    }
    cpu_state.data.al &= 0x0f;
    instruction_state.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF);
    TYPE_TRACE_CALL_END;
}
static C_VOID INC_EAX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INC_EAX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.ax, 16));
            cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.eax, 32));
            cpu_state.data.eax = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.ax, 16));
        cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INC_ECX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INC_ECX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.cx, 16));
            cpu_state.data.cx = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.ecx, 32));
            cpu_state.data.ecx = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.cx, 16));
        cpu_state.data.cx = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INC_EDX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INC_EDX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.dx, 16));
            cpu_state.data.dx = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.edx, 32));
            cpu_state.data.edx = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.dx, 16));
        cpu_state.data.dx = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INC_EBX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INC_EBX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.bx, 16));
            cpu_state.data.bx = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.ebx, 32));
            cpu_state.data.ebx = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.bx, 16));
        cpu_state.data.bx = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INC_ESP(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INC_ESP");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.sp, 16));
            cpu_state.data.sp = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.esp, 32));
            cpu_state.data.esp = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.sp, 16));
        cpu_state.data.sp = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INC_EBP(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INC_EBP");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.bp, 16));
            cpu_state.data.bp = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.ebp, 32));
            cpu_state.data.ebp = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.bp, 16));
        cpu_state.data.bp = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INC_ESI(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INC_ESI");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.si, 16));
            cpu_state.data.si = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.esi, 32));
            cpu_state.data.esi = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.si, 16));
        cpu_state.data.si = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INC_EDI(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INC_EDI");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.di, 16));
            cpu_state.data.di = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.edi, 32));
            cpu_state.data.edi = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.di, 16));
        cpu_state.data.di = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID DEC_EAX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("DEC_EAX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.ax, 16));
            cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.eax, 32));
            cpu_state.data.eax = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.ax, 16));
        cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID DEC_ECX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("DEC_ECX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.cx, 16));
            cpu_state.data.cx = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.ecx, 32));
            cpu_state.data.ecx = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.cx, 16));
        cpu_state.data.cx = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID DEC_EDX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("DEC_EDX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.dx, 16));
            cpu_state.data.dx = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.edx, 32));
            cpu_state.data.edx = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.dx, 16));
        cpu_state.data.dx = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID DEC_EBX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("DEC_EBX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.bx, 16));
            cpu_state.data.bx = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.ebx, 32));
            cpu_state.data.ebx = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.bx, 16));
        cpu_state.data.bx = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID DEC_ESP(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("DEC_ESP");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.sp, 16));
            cpu_state.data.sp = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.esp, 32));
            cpu_state.data.esp = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.sp, 16));
        cpu_state.data.sp = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID DEC_EBP(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("DEC_EBP");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.bp, 16));
            cpu_state.data.bp = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.ebp, 32));
            cpu_state.data.ebp = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.bp, 16));
        cpu_state.data.bp = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID DEC_ESI(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("DEC_ESI");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.si, 16));
            cpu_state.data.si = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.esi, 32));
            cpu_state.data.esi = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.si, 16));
        cpu_state.data.si = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID DEC_EDI(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("DEC_EDI");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.di, 16));
            cpu_state.data.di = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.edi, 32));
            cpu_state.data.edi = TYPE_MASK_UNSIGNED_32(instruction_state.data.result);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.di, 16));
        cpu_state.data.di = TYPE_MASK_UNSIGNED_16(instruction_state.data.result);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_EAX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_EAX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_ECX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_ECX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.cx), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.ecx), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.cx), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_EDX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_EDX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.dx), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.edx), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.dx), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_EBX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_EBX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.bx), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.ebx), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.bx), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_ESP(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_ESP");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.sp), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.esp), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        /* 8086/8088 and 80186/80188 expose the decremented SP when SP is
         * the PUSH source.  The 80286 changes this one observable case to
         * push the pre-instruction value. */
        if (context->cpu_profile < CORE_MACHINE_CPU_PROFILE_80286)
        {
            type_unsigned_16 value = cpu_state.data.sp - 2;

            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(value), 2));
        }
        else
            TYPE_TRACE_CHECK_RETURN(_e_push(context,
                TYPE_REFERENCE_OF(cpu_state.data.sp), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_EBP(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_EBP");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.bp), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.ebp), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.bp), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_ESI(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_ESI");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.si), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.esi), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.si), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_EDI(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_EDI");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.di), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.edi), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.di), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_EAX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("POP_EAX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_ECX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("POP_ECX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.cx), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.ecx), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.cx), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_EDX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("POP_EDX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.dx), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.edx), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.dx), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_EBX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("POP_EBX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.bx), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.ebx), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.bx), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_ESP(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("POP_ESP");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.sp), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.esp), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.sp), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_EBP(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("POP_EBP");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.bp), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.ebp), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.bp), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_ESI(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("POP_ESI");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.si), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.esi), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.si), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_EDI(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("POP_EDI");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.di), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.edi), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.di), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSHA(core_machine_cpu_execution_context *context)
{
    type_unsigned_32 cesp;
    TYPE_TRACE_CALL_BEGIN("PUSHA");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            cesp = cpu_state.data.sp;
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.cx), 2));
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.dx), 2));
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.bx), 2));
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cesp), 2));
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.bp), 2));
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.si), 2));
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.di), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            cesp = cpu_state.data.esp;
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.ecx), 4));
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.edx), 4));
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.ebx), 4));
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cesp), 4));
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.ebp), 4));
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.esi), 4));
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.edi), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID POPA(core_machine_cpu_execution_context *context)
{
    type_unsigned_32 cesp;
    TYPE_TRACE_CALL_BEGIN("POPA");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.di), 2));
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.si), 2));
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.bp), 2));
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cesp), 2));
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.bx), 2));
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.dx), 2));
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.cx), 2));
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.edi), 4));
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.esi), 4));
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.ebp), 4));
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cesp), 4));
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.ebx), 4));
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.edx), 4));
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.ecx), 4));
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID BOUND_R16_M16_16(core_machine_cpu_execution_context *context)
{
    type_signed_16 a16, l16, u16;
    type_signed_32 a32, l32, u32;
    TYPE_TRACE_CALL_BEGIN("BOUND_R16_M16_16");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize * 2));
        if (!instruction_state.data.flagMem)
        {
            TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            a16 = (type_signed_16)instruction_state.data.cr;
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            l16 = (type_signed_16)instruction_state.data.crm;
            instruction_state.data.mrm.offset += 2;
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            u16 = (type_signed_16)instruction_state.data.crm;
            if (a16 < l16 || a16 > u16)
                TYPE_TRACE_CHECK_RETURN(_SetExcept_BR(0));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            a32 = (type_signed_32)instruction_state.data.cr;
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 4));
            l32 = (type_signed_32)instruction_state.data.crm;
            instruction_state.data.mrm.offset += 4;
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 4));
            u32 = (type_signed_32)instruction_state.data.crm;
            if (a32 < l32 || a32 > u32)
                TYPE_TRACE_CHECK_RETURN(_SetExcept_BR(0));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID ARPL_RM16_R16(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("ARPL_RM16_R16");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80286)
    {
        if (_IsProtected)
        {
            _adv;
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            instruction_state.data.cr = TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
            if (_GetSelector_RPL(instruction_state.data.crm) < _GetSelector_RPL(instruction_state.data.cr))
            {
                _SetEFLAGS_ZF;
                instruction_state.data.result = TYPE_MASK_UNSIGNED_16((instruction_state.data.crm & ~VCPU_SELECTOR_RPL) |
                                                                      (instruction_state.data.cr & VCPU_SELECTOR_RPL));
                instruction_state.data.crm = instruction_state.data.result;
                TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            }
            else
                _ClrEFLAGS_ZF;
        }
        else
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_FS(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_FS");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.fs;
        instruction_state.data.roverss = &cpu_state.data.fs;
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_GS(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_GS");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.gs;
        instruction_state.data.roverss = &cpu_state.data.gs;
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_OprSize(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_OprSize");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        instruction_state.data.prefix_oprsize = TYPE_TRUE;
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_AddrSize(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_AddrSize");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        instruction_state.data.prefix_addrsize = TYPE_TRUE;
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186)
    {
        _adv;
        if (instruction_state.data.flagLock)
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(instruction_state.data.cimm), _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID IMUL_R32_RM32_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("IMUL_R32_RM32_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_a_imul3(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PUSH_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186)
    {
        _adv;
        if (instruction_state.data.flagLock)
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        instruction_state.data.cimm = (type_signed_8)instruction_state.data.cimm;
        TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(instruction_state.data.cimm), _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID IMUL_R32_RM32_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("IMUL_R32_RM32_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        TYPE_TRACE_CHECK_RETURN(_a_imul3(context, instruction_state.data.crm, instruction_state.data.cimm, ((_GetOperandSize * 8 + 8) >> 1)));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID INSB(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INSB");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186)
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            TYPE_TRACE_CHECK_RETURN(_p_ins(context, 1));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_p_ins(context, 1));
                    cpu_state.data.cx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_p_ins(context, 1));
                    cpu_state.data.ecx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
        }
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID INSW(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INSW");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186)
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            TYPE_TRACE_CHECK_RETURN(_p_ins(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_p_ins(context, _GetOperandSize));
                    cpu_state.data.cx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_p_ins(context, _GetOperandSize));
                    cpu_state.data.ecx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
        }
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID OUTSB(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("OUTSB");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186)
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            TYPE_TRACE_CHECK_RETURN(_p_outs(context, 1));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_p_outs(context, 1));
                    cpu_state.data.cx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_p_outs(context, 1));
                    cpu_state.data.ecx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
        }
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID OUTSW(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("OUTSW");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186)
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            TYPE_TRACE_CHECK_RETURN(_p_outs(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_p_outs(context, _GetOperandSize));
                    cpu_state.data.cx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_p_outs(context, _GetOperandSize));
                    cpu_state.data.ecx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
        }
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID JO_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JO_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, _GetEFLAGS_OF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JNO_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JNO_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, !_GetEFLAGS_OF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JC_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JC_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, _GetEFLAGS_CF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JNC_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JNC_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, !_GetEFLAGS_CF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JZ_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JZ_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, _GetEFLAGS_ZF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JNZ_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JNZ_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, !_GetEFLAGS_ZF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JNA_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JNA_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1,
                                   (_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    TYPE_TRACE_CALL_END;
}
static C_VOID JA_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JA_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1,
                                   !(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    TYPE_TRACE_CALL_END;
}
static C_VOID JS_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JS_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, _GetEFLAGS_SF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JNS_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JNS_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, !_GetEFLAGS_SF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JP_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JP_REL8");
    _new_code_path_;
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, _GetEFLAGS_PF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JNP_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JNP_REL8");
    _new_code_path_;
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, !_GetEFLAGS_PF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JL_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JL_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, (_GetEFLAGS_SF != _GetEFLAGS_OF)));
    TYPE_TRACE_CALL_END;
}
static C_VOID JNL_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JNL_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, (_GetEFLAGS_SF == _GetEFLAGS_OF)));
    TYPE_TRACE_CALL_END;
}
static C_VOID JNG_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JNG_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1,
                                   (_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF))));
    TYPE_TRACE_CALL_END;
}
static C_VOID JG_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JG_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1,
                                   (!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF))));
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_80(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INS_80");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    switch (instruction_state.data.cr)
    {
    case 0: /* ADD_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("ADD_RM8_I8");
        TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* OR_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("OR_RM8_I8");
        TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* ADC_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("ADC_RM8_I8");
        TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* SBB_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("SBB_RM8_I8");
        _new_code_path_;
        TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* AND_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("AND_RM8_I8");
        TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SUB_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("SUB_RM8_I8");
        TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* XOR_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("XOR_RM8_I8");
        TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* CMP_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("CMP_RM8_I8");
        TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_81(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INS_81");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        switch (instruction_state.data.cr)
        {
        case 0: /* ADD_RM32_I32 */
            TYPE_TRACE_BLOCK_BEGIN("ADD_RM32_I32");
            TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* OR_RM32_I32 */
            TYPE_TRACE_BLOCK_BEGIN("OR_RM32_I32");
            TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* ADC_RM32_I32 */
            TYPE_TRACE_BLOCK_BEGIN("ADC_RM32_I32");
            TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* SBB_RM32_I32 */
            TYPE_TRACE_BLOCK_BEGIN("SBB_RM32_I32");
            _new_code_path_;
            TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* AND_RM32_I32 */
            TYPE_TRACE_BLOCK_BEGIN("AND_RM32_I32");
            TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SUB_RM32_I32 */
            TYPE_TRACE_BLOCK_BEGIN("SUB_RM32_I32");
            TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* XOR_RM32_I32 */
            TYPE_TRACE_BLOCK_BEGIN("XOR_RM32_I32");
            TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* CMP_RM32_I32 */
            TYPE_TRACE_BLOCK_BEGIN("CMP_RM32_I32");
            TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        switch (instruction_state.data.cr)
        {
        case 0: /* ADD_RM16_I16 */
            TYPE_TRACE_BLOCK_BEGIN("ADD_RM16_I16");
            TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* OR_RM16_I16 */
            TYPE_TRACE_BLOCK_BEGIN("OR_RM16_I16");
            TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* ADC_RM16_I16 */
            TYPE_TRACE_BLOCK_BEGIN("ADC_RM16_I16");
            TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* SBB_RM16_I16 */
            TYPE_TRACE_BLOCK_BEGIN("SBB_RM16_I16");
            _new_code_path_;
            TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* AND_RM16_I16 */
            TYPE_TRACE_BLOCK_BEGIN("AND_RM16_I16");
            TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SUB_RM16_I16 */
            TYPE_TRACE_BLOCK_BEGIN("SUB_RM16_I16");
            TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* XOR_RM16_I16 */
            TYPE_TRACE_BLOCK_BEGIN("XOR_RM16_I16");
            TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* CMP_RM16_I16 */
            TYPE_TRACE_BLOCK_BEGIN("CMP_RM16_I16");
            TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_83(core_machine_cpu_execution_context *context)
{
    type_unsigned_8 bit;
    TYPE_TRACE_CALL_BEGIN("INS_83");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        bit = (_GetOperandSize * 8 + 8) >> 1;
        switch (instruction_state.data.cr)
        {
        case 0: /* ADD_RM32_I8 */
            TYPE_TRACE_BLOCK_BEGIN("ADD_RM32_I8");
            TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* OR_RM32_I8 */
            TYPE_TRACE_BLOCK_BEGIN("OR_RM32_I8");
            TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* ADC_RM32_I8 */
            TYPE_TRACE_BLOCK_BEGIN("ADC_RM32_I8");
            TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* SBB_RM32_I8 */
            TYPE_TRACE_BLOCK_BEGIN("SBB_RM32_I8");
            TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* AND_RM32_I8 */
            TYPE_TRACE_BLOCK_BEGIN("AND_RM32_I8");
            TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SUB_RM32_I8 */
            TYPE_TRACE_BLOCK_BEGIN("SUB_RM32_I8");
            TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* XOR_RM32_I8 */
            TYPE_TRACE_BLOCK_BEGIN("XOR_RM32_I8");
            TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* CMP_RM32_I8 */
            TYPE_TRACE_BLOCK_BEGIN("CMP_RM32_I8");
            TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        bit = 12;
        switch (instruction_state.data.cr)
        {
        case 0: /* ADD_RM16_I8 */
            TYPE_TRACE_BLOCK_BEGIN("ADD_RM16_I8");
            TYPE_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* OR_RM16_I8 */
            TYPE_TRACE_BLOCK_BEGIN("OR_RM16_I8");
            TYPE_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* ADC_RM16_I8 */
            TYPE_TRACE_BLOCK_BEGIN("ADC_RM16_I8");
            TYPE_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* SBB_RM16_I8 */
            TYPE_TRACE_BLOCK_BEGIN("SBB_RM16_I8");
            TYPE_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* AND_RM16_I8 */
            TYPE_TRACE_BLOCK_BEGIN("AND_RM16_I8");
            TYPE_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SUB_RM16_I8 */
            TYPE_TRACE_BLOCK_BEGIN("SUB_RM16_I8");
            TYPE_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* XOR_RM16_I8 */
            TYPE_TRACE_BLOCK_BEGIN("XOR_RM16_I8");
            TYPE_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* CMP_RM16_I8 */
            TYPE_TRACE_BLOCK_BEGIN("CMP_RM16_I8");
            TYPE_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID TEST_RM8_R8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("TEST_RM8_R8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_test(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    TYPE_TRACE_CALL_END;
}
static C_VOID TEST_RM32_R32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("TEST_RM32_R32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_a_test(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
    }
    else
    {
        cpu_state.data.ip++;
        _d_modrm(context, 2, 2);
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        _a_test(context, instruction_state.data.crm, instruction_state.data.cr, 16);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_RM8_R8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("XCHG_RM8_R8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.crm), 1));
    instruction_state.data.crm = instruction_state.data.cr;
    TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_RM32_R32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("XCHG_RM32_R32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
        instruction_state.data.crm = instruction_state.data.cr;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        _d_modrm(context, 2, 2);
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.crm), 2));
        instruction_state.data.crm = instruction_state.data.cr;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_RM8_R8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_RM8_R8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    instruction_state.data.crm = instruction_state.data.cr;
    TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_RM32_R32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_RM32_R32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        instruction_state.data.crm = instruction_state.data.cr;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        _d_modrm(context, 2, 2);
        instruction_state.data.crm = instruction_state.data.cr;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_R8_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_R8_RM8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.crm), 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_R32_RM32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_R32_RM32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        _d_modrm(context, 2, 2);
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.crm), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_RM16_SREG(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_RM16_SREG");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm_sreg(context, 2));
    instruction_state.data.crm = instruction_state.data.rmovsreg->selector;
    TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    TYPE_TRACE_CALL_END;
}
static C_VOID LEA_R32_M32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("LEA_R32_M32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm_ea(context, _GetOperandSize, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.mrm.offset), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.mrm.offset), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        _d_modrm_ea(context, 2, 2);
        _m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.mrm.offset), 2);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_SREG_RM16(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_SREG_RM16");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm_sreg(context, 2));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm_sreg(context, 2));
    }
    if (instruction_state.data.rmovsreg->sregtype == SREG_CODE)
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
    TYPE_TRACE_CHECK_RETURN(_s_load_sreg(context, instruction_state.data.rmovsreg,
        TYPE_MASK_UNSIGNED_16(instruction_state.data.crm)));
    if (instruction_state.data.rmovsreg->sregtype == SREG_STACK)
        instruction_state.data.flagMaskInt = TYPE_TRUE;
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_8F(core_machine_cpu_execution_context *context)
{
    type_unsigned_8 modrm;
    type_unsigned_32 value;

    TYPE_TRACE_CALL_BEGIN("INS_8F");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip,
            TYPE_REFERENCE_OF(modrm), 1));
        if (instruction_state.data.flagLock)
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        switch (_GetModRM_REG(modrm))
        {
        case 0: /* POP_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("POP_RM32");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context,
                TYPE_REFERENCE_OF(value), _GetOperandSize));
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            instruction_state.data.crm = value;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 5:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 6:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.ip,
            TYPE_REFERENCE_OF(modrm), 1));
        if (instruction_state.data.flagLock)
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        switch (_GetModRM_REG(modrm))
        {
        case 0: /* POP_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("POP_RM16");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context,
                TYPE_REFERENCE_OF(value), 2));
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            instruction_state.data.crm = value;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 5:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 6:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID NOP(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("NOP");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
        cpu_state.data.ip++;
    TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_ECX_EAX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("XCHG_ECX_EAX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.cx;
            cpu_state.data.cx = TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.ecx;
            cpu_state.data.ecx = TYPE_MASK_UNSIGNED_32(instruction_state.data.cr);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.cx;
        cpu_state.data.cx = TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_EDX_EAX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("XCHG_EDX_EAX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.dx;
            cpu_state.data.dx = TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.edx;
            cpu_state.data.edx = TYPE_MASK_UNSIGNED_32(instruction_state.data.cr);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.dx;
        cpu_state.data.dx = TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_EBX_EAX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("XCHG_EBX_EAX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.bx;
            cpu_state.data.bx = TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.ebx;
            cpu_state.data.ebx = TYPE_MASK_UNSIGNED_32(instruction_state.data.cr);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.bx;
        cpu_state.data.bx = TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_ESP_EAX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("XCHG_ESP_EAX");
    _new_code_path_;
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.sp;
            cpu_state.data.sp = TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.esp;
            cpu_state.data.esp = TYPE_MASK_UNSIGNED_32(instruction_state.data.cr);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.sp;
        cpu_state.data.sp = TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_EBP_EAX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("XCHG_EBP_EAX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.bp;
            cpu_state.data.bp = TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.ebp;
            cpu_state.data.ebp = TYPE_MASK_UNSIGNED_32(instruction_state.data.cr);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.bp;
        cpu_state.data.bp = TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_ESI_EAX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("XCHG_ESI_EAX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.si;
            cpu_state.data.si = TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.esi;
            cpu_state.data.esi = TYPE_MASK_UNSIGNED_32(instruction_state.data.cr);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.si;
        cpu_state.data.si = TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID XCHG_EDI_EAX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("XCHG_EDI_EAX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            cpu_state.data.ax = cpu_state.data.di;
            cpu_state.data.di = TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            cpu_state.data.eax = cpu_state.data.edi;
            cpu_state.data.edi = TYPE_MASK_UNSIGNED_32(instruction_state.data.cr);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        instruction_state.data.cr = cpu_state.data.ax;
        cpu_state.data.ax = cpu_state.data.di;
        cpu_state.data.di = TYPE_MASK_UNSIGNED_16(instruction_state.data.cr);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID CBW(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("CBW");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            cpu_state.data.ax = (type_signed_8)cpu_state.data.al;
            break;
        case 4:
            cpu_state.data.eax = (type_signed_16)cpu_state.data.ax;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        cpu_state.data.ax = (type_signed_8)cpu_state.data.al;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID CWD(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("CWD");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            cpu_state.data.dx = TYPE_GET_MSB_16(cpu_state.data.ax) ? 0xffff : 0x0000;
            break;
        case 4:
            cpu_state.data.edx = TYPE_GET_MSB_32(cpu_state.data.eax) ? 0xffffffff : 0x00000000;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        if (cpu_state.data.ax & 0x8000)
            cpu_state.data.dx = 0xffff;
        else
            cpu_state.data.dx = 0x0000;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID CALL_PTR16_32(core_machine_cpu_execution_context *context)
{
    type_unsigned_16 newcs;
    type_unsigned_32 neweip;
    TYPE_TRACE_CALL_BEGIN("CALL_PTR16_32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            neweip = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
            newcs = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm >> 16);
            TYPE_TRACE_CHECK_RETURN(_e_call_far(context, newcs, neweip, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 6));
            neweip = TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm);
            newcs = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm >> 32);
            TYPE_TRACE_CHECK_RETURN(_e_call_far(context, newcs, neweip, 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        _d_imm(context, 4);
        neweip = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
        newcs = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm >> 16);
        _e_call_far(context, newcs, neweip, 2);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID WAIT(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("WAIT");
    _new_code_path_;
    _adv;
    if (_GetCR0_TS && _GetCR0_MP)
    {
        TYPE_TRACE_BLOCK_BEGIN("CR0_TS_AND_MP(1)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_NM(0));
        TYPE_TRACE_BLOCK_END;
    }
    else if (core_machine_fpu_wait_pending(context->fpu))
    {
        TYPE_TRACE_BLOCK_BEGIN("FPU_PENDING(1)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_MF(0));
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        (C_VOID)core_machine_fpu_complete_wait(context->fpu);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSHF(core_machine_cpu_execution_context *context)
{
    type_unsigned_32 ceflags;
    TYPE_TRACE_CALL_BEGIN("PUSHF");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        if (!_GetCR0_PE || (_GetCR0_PE && (!_GetEFLAGS_VM || (_GetEFLAGS_VM && (_GetEFLAGS_IOPL == 3)))))
        {
            TYPE_TRACE_BLOCK_BEGIN("Real/Protected/(V86,IOPL(3))");
            switch (_GetOperandSize)
            {
            case 2:
                TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
                ceflags = _e_real_flags_image_16(context, cpu_state.data.flags);
                TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(ceflags), 2));
                TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
                ceflags = (cpu_state.data.eflags & ~(VCPU_EFLAGS_VM | VCPU_EFLAGS_RF |
                    VCPU_EFLAGS_RESERVED)) | 0x02u;
                TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(ceflags), 4));
                TYPE_TRACE_BLOCK_END;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("EFLAGS_VM(1),EFLAGS_IOPL(!3)");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            TYPE_TRACE_BLOCK_END;
        }
    }
    else
    {
        cpu_state.data.ip++;
        ceflags = _e_real_flags_image_16(context, cpu_state.data.flags);
        _e_push(context, TYPE_REFERENCE_OF(ceflags), 2);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID POPF(core_machine_cpu_execution_context *context)
{
    type_unsigned_32 mask = VCPU_EFLAGS_RESERVED | VCPU_EFLAGS_RF;
    type_unsigned_32 ceflags = TYPE_ZERO_32;
    TYPE_TRACE_CALL_BEGIN("POPF");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        if (!_GetCR0_PE || !_GetEFLAGS_VM)
        {
            TYPE_TRACE_BLOCK_BEGIN("!V86");
            if (!_GetCPL)
            {
                TYPE_TRACE_BLOCK_BEGIN("CPL(0)");
                switch (_GetOperandSize)
                {
                case 2:
                    TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
                    TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(ceflags), 2));
                    mask |= 0xffff0000;
                    TYPE_TRACE_BLOCK_END;
                    break;
                case 4:
                    TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
                    TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(ceflags), 4));
                    mask |= VCPU_EFLAGS_VM;
                    TYPE_TRACE_BLOCK_END;
                    break;
                default:
                    TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                TYPE_TRACE_BLOCK_END;
            }
            else
            {
                TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
                _new_code_path_;
                switch (_GetOperandSize)
                {
                case 2:
                    TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
                    TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(ceflags), 2));
                    mask |= (0xffff0000 | VCPU_EFLAGS_IOPL);
                    TYPE_TRACE_BLOCK_END;
                    break;
                case 4:
                    TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
                    TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(ceflags), 4));
                    mask |= (VCPU_EFLAGS_VM | VCPU_EFLAGS_RF | VCPU_EFLAGS_IOPL);
                    TYPE_TRACE_BLOCK_END;
                    break;
                default:
                    TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                if (_GetCPL > _GetEFLAGS_IOPL)
                    mask |= VCPU_EFLAGS_IF;
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("V86");
            if (instruction_state.data.prefix_oprsize)
            {
                TYPE_TRACE_BLOCK_BEGIN("prefix_oprsize(1)");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                TYPE_TRACE_BLOCK_END;
            }
            if (_GetEFLAGS_IOPL == 0x03)
            {
                TYPE_TRACE_BLOCK_BEGIN("EFLAGS_IOPL(3)");
                switch (_GetOperandSize)
                {
                case 2:
                    TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
                    TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(ceflags), 2));
                    mask |= (0xffff0000 | VCPU_EFLAGS_IOPL);
                    TYPE_TRACE_BLOCK_END;
                    break;
                case 4:
                    TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
                    TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(ceflags), 4));
                    mask |= (VCPU_EFLAGS_VM | VCPU_EFLAGS_RF | VCPU_EFLAGS_IOPL);
                    TYPE_TRACE_BLOCK_END;
                    break;
                default:
                    TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                TYPE_TRACE_BLOCK_END;
            }
            else
            {
                TYPE_TRACE_BLOCK_BEGIN("EFLAGS_IOPL(!3)");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_BLOCK_END;
        }
        cpu_state.data.eflags = _e_eflags_load(context,
            (ceflags & ~mask) | (cpu_state.data.eflags & mask));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(ceflags), 2));
        cpu_state.data.eflags = _e_eflags_load(context, ceflags);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID SAHF(core_machine_cpu_execution_context *context)
{
    type_unsigned_32 mask = (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                             VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_CF);
    TYPE_TRACE_CALL_BEGIN("SAHF");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    cpu_state.data.eflags = (cpu_state.data.ah & mask) | (cpu_state.data.eflags & ~mask);
    TYPE_TRACE_CALL_END;
}
static C_VOID LAHF(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("LAHF");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    cpu_state.data.ah = TYPE_MASK_UNSIGNED_8(cpu_state.data.flags) | 0x02u;
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_AL_MOFFS8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_AL_MOFFS8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_moffs(context, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    cpu_state.data.al = TYPE_MASK_UNSIGNED_8(instruction_state.data.crm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_EAX_MOFFS32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_EAX_MOFFS32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_moffs(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
            break;
        case 4:
            cpu_state.data.eax = TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_moffs(context, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_MOFFS8_AL(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_MOFFS8_AL");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_moffs(context, 1));
    instruction_state.data.result = cpu_state.data.al;
    instruction_state.data.crm = instruction_state.data.result;
    TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_MOFFS32_EAX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_MOFFS32_EAX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_moffs(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            instruction_state.data.result = cpu_state.data.ax;
            break;
        case 4:
            instruction_state.data.result = cpu_state.data.eax;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_moffs(context, 2));
        instruction_state.data.crm = cpu_state.data.ax;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOVSB(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOVSB");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            TYPE_TRACE_CHECK_RETURN(_m_movs(context, 1));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_m_movs(context, 1));
                    cpu_state.data.cx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_m_movs(context, 1));
                    cpu_state.data.ecx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
        }
    }
    else
    {
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            _m_movs(context, 1);
        else
        {
            if (cpu_state.data.cx)
            {
                _m_movs(context, 1);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx)
                instruction_state.data.flagInsLoop = TYPE_TRUE;
        }
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOVSW(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOVSW");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            TYPE_TRACE_CHECK_RETURN(_m_movs(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_m_movs(context, _GetOperandSize));
                    cpu_state.data.cx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_m_movs(context, _GetOperandSize));
                    cpu_state.data.ecx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
        }
    }
    else
    {
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            _m_movs(context, 2);
        else
        {
            if (cpu_state.data.cx)
            {
                _m_movs(context, 2);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx)
                instruction_state.data.flagInsLoop = TYPE_TRUE;
        }
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID CMPSB(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("CMPSB");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            TYPE_TRACE_CHECK_RETURN(_a_cmps(context, 8));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_a_cmps(context, 8));
                    cpu_state.data.cx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_a_cmps(context, 8));
                    cpu_state.data.ecx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
        }
    }
    else
    {
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            _a_cmps(context, 8);
        else
        {
            if (cpu_state.data.cx)
            {
                _a_cmps(context, 8);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx &&
                !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                instruction_state.data.flagInsLoop = TYPE_TRUE;
        }
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID CMPSW(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("CMPSW");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            TYPE_TRACE_CHECK_RETURN(_a_cmps(context, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_a_cmps(context, _GetOperandSize * 8));
                    cpu_state.data.cx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_a_cmps(context, _GetOperandSize * 8));
                    cpu_state.data.ecx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
        }
    }
    else
    {
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            _a_cmps(context, 16);
        else
        {
            if (cpu_state.data.cx)
            {
                _a_cmps(context, 16);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx &&
                !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                instruction_state.data.flagInsLoop = TYPE_TRUE;
        }
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID TEST_AL_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("TEST_AL_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_test(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    TYPE_TRACE_CALL_END;
}
static C_VOID TEST_EAX_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("TEST_EAX_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            TYPE_TRACE_CHECK_RETURN(_a_test(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            TYPE_TRACE_CHECK_RETURN(_a_test(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_a_test(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID STOSB(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("STOSB");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            TYPE_TRACE_CHECK_RETURN(_m_stos(context, 1));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_m_stos(context, 1));
                    cpu_state.data.cx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_m_stos(context, 1));
                    cpu_state.data.ecx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
        }
    }
    else
    {
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            _m_stos(context, 1);
        else
        {
            if (cpu_state.data.cx)
            {
                _m_stos(context, 1);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx)
                instruction_state.data.flagInsLoop = TYPE_TRUE;
        }
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID STOSW(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("STOSW");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            TYPE_TRACE_CHECK_RETURN(_m_stos(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_m_stos(context, _GetOperandSize));
                    cpu_state.data.cx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_m_stos(context, _GetOperandSize));
                    cpu_state.data.ecx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
        }
    }
    else
    {
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            _m_stos(context, 2);
        else
        {
            if (cpu_state.data.cx)
            {
                _m_stos(context, 2);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx)
                instruction_state.data.flagInsLoop = TYPE_TRUE;
        }
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID LODSB(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("LODSB");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            TYPE_TRACE_CHECK_RETURN(_m_lods(context, 1));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_m_lods(context, 1));
                    cpu_state.data.cx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_m_lods(context, 1));
                    cpu_state.data.ecx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
        }
    }
    else
    {
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            _m_lods(context, 1);
        else
        {
            if (cpu_state.data.cx)
            {
                _m_lods(context, 1);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx)
                instruction_state.data.flagInsLoop = TYPE_TRUE;
        }
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID LODSW(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("LODSW");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            TYPE_TRACE_CHECK_RETURN(_m_lods(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_m_lods(context, _GetOperandSize));
                    cpu_state.data.cx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_m_lods(context, _GetOperandSize));
                    cpu_state.data.ecx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
        }
    }
    else
    {
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            _m_lods(context, 2);
        else
        {
            if (cpu_state.data.cx)
            {
                _m_lods(context, 2);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx)
                instruction_state.data.flagInsLoop = TYPE_TRUE;
        }
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID SCASB(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SCASB");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            TYPE_TRACE_CHECK_RETURN(_a_scas(context, 8));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_a_scas(context, 8));
                    cpu_state.data.cx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_a_scas(context, 8));
                    cpu_state.data.ecx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
        }
    }
    else
    {
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            _a_scas(context, 8);
        else
        {
            if (cpu_state.data.cx)
            {
                _a_scas(context, 8);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx &&
                !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                instruction_state.data.flagInsLoop = TYPE_TRUE;
        }
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID SCASW(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SCASW");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            TYPE_TRACE_CHECK_RETURN(_a_scas(context, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("cx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_a_scas(context, _GetOperandSize * 8));
                    cpu_state.data.cx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    TYPE_TRACE_BLOCK_BEGIN("ecx(!0)");
                    TYPE_TRACE_CHECK_RETURN(_a_scas(context, _GetOperandSize * 8));
                    cpu_state.data.ecx--;
                    TYPE_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = TYPE_TRUE;
                TYPE_TRACE_BLOCK_END;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
        }
    }
    else
    {
        cpu_state.data.ip++;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            _a_scas(context, 16);
        else
        {
            if (cpu_state.data.cx)
            {
                _a_scas(context, 16);
                cpu_state.data.cx--;
            }
            if (cpu_state.data.cx &&
                !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                instruction_state.data.flagInsLoop = TYPE_TRUE;
        }
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_AL_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_AL_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    cpu_state.data.al = TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_CL_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_CL_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    cpu_state.data.cl = TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_DL_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_DL_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    cpu_state.data.dl = TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_BL_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_BL_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    cpu_state.data.bl = TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_AH_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_AH_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    cpu_state.data.ah = TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_CH_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_CH_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    cpu_state.data.ch = TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_DH_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_DH_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    cpu_state.data.dh = TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_BH_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_BH_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    cpu_state.data.bh = TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_EAX_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_EAX_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.eax = TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm);
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        cpu_state.data.ax = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_ECX_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_ECX_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            cpu_state.data.cx = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.ecx = TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm);
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        cpu_state.data.cx = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_EDX_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_EDX_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            cpu_state.data.dx = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.edx = TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm);
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        cpu_state.data.dx = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_EBX_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_EBX_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            cpu_state.data.bx = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.ebx = TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm);
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        cpu_state.data.bx = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_ESP_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_ESP_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            cpu_state.data.sp = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.esp = TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm);
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        cpu_state.data.sp = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_EBP_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_EBP_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            cpu_state.data.bp = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.ebp = TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm);
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        cpu_state.data.bp = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_ESI_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_ESI_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            cpu_state.data.si = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.esi = TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm);
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        cpu_state.data.si = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_EDI_I32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_EDI_I32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            cpu_state.data.di = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
            break;
        case 4:
            cpu_state.data.edi = TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm);
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        cpu_state.data.di = TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_C0(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INS_C0");
    _new_code_path_;
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186)
    {
        if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
        {
            _adv;
        }
        else
        {
            cpu_state.data.ip++;
        }
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        switch (instruction_state.data.cr)
        {
        case 0: /* ROL_RM8_I8 */
            TYPE_TRACE_BLOCK_BEGIN("ROL_RM8_I8");
            TYPE_TRACE_CHECK_RETURN(_a_rol(context, TYPE_MASK_UNSIGNED_8(instruction_state.data.crm), TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* ROR_RM8_I8 */
            TYPE_TRACE_BLOCK_BEGIN("ROR_RM8_I8");
            TYPE_TRACE_CHECK_RETURN(_a_ror(context, TYPE_MASK_UNSIGNED_8(instruction_state.data.crm), TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* RCL_RM8_I8 */
            TYPE_TRACE_BLOCK_BEGIN("RCL_RM8_I8");
            TYPE_TRACE_CHECK_RETURN(_a_rcl(context, TYPE_MASK_UNSIGNED_8(instruction_state.data.crm), TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* RCR_RM8_I8 */
            TYPE_TRACE_BLOCK_BEGIN("RCR_RM8_I8");
            TYPE_TRACE_CHECK_RETURN(_a_rcr(context, TYPE_MASK_UNSIGNED_8(instruction_state.data.crm), TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* SHL_RM8_I8 */
            TYPE_TRACE_BLOCK_BEGIN("SHL_RM8_I8");
            TYPE_TRACE_CHECK_RETURN(_a_shl(context, TYPE_MASK_UNSIGNED_8(instruction_state.data.crm), TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SHR_RM8_I8 */
            TYPE_TRACE_BLOCK_BEGIN("SHR_RM8_I8");
            TYPE_TRACE_CHECK_RETURN(_a_shr(context, TYPE_MASK_UNSIGNED_8(instruction_state.data.crm), TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* UndefinedOpcode */
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* SAR_RM8_I8 */
            TYPE_TRACE_BLOCK_BEGIN("SAR_RM8_I8");
            TYPE_TRACE_CHECK_RETURN(_a_sar(context, TYPE_MASK_UNSIGNED_8(instruction_state.data.crm), TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), 8));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_C1(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INS_C1");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186)
    {
        if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
        {
            _adv;
        }
        else
        {
            cpu_state.data.ip++;
        }
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        switch (instruction_state.data.cr)
        {
        case 0: /* ROL_RM32_I8 */
            TYPE_TRACE_BLOCK_BEGIN("ROL_RM32_I8");
            TYPE_TRACE_CHECK_RETURN(_a_rol(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.crm), TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* ROR_RM32_I8 */
            TYPE_TRACE_BLOCK_BEGIN("ROR_RM32_I8");
            TYPE_TRACE_CHECK_RETURN(_a_ror(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.crm), TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* RCL_RM32_I8 */
            TYPE_TRACE_BLOCK_BEGIN("RCL_RM32_I8");
            TYPE_TRACE_CHECK_RETURN(_a_rcl(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.crm), TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* RCR_RM32_I8 */
            TYPE_TRACE_BLOCK_BEGIN("RCR_RM32_I8");
            TYPE_TRACE_CHECK_RETURN(_a_rcr(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.crm), TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* SHL_RM32_I8 */
            TYPE_TRACE_BLOCK_BEGIN("SHL_RM32_I8");
            TYPE_TRACE_CHECK_RETURN(_a_shl(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.crm), TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SHR_RM32_I8 */
            TYPE_TRACE_BLOCK_BEGIN("SHR_RM32_I8");
            TYPE_TRACE_CHECK_RETURN(_a_shr(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.crm), TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* UndefinedOpcode */
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* SAR_RM32_I8 */
            TYPE_TRACE_BLOCK_BEGIN("SAR_RM32_I8");
            TYPE_TRACE_CHECK_RETURN(_a_sar(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.crm), TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID RET_I16(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("RET_I16");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_e_ret_near(context, TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm), _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        _d_imm(context, 2);
        _e_ret_near(context, TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm), 2);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID RET(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("RET");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_e_ret_near(context, 0, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_ret_near(context, 0, 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID LES_R32_M16_32(core_machine_cpu_execution_context *context)
{
    type_unsigned_16 selector;
    type_unsigned_32 offset;
    TYPE_TRACE_CALL_BEGIN("LES_R32_M16_32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm_ea(context, _GetOperandSize,
            _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            offset = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
            break;
        case 4:
            offset = TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.mrm.offset += _GetOperandSize;
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        selector = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        TYPE_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.es, instruction_state.data.rr, selector, offset, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm_ea(context, 2, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        offset = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        instruction_state.data.mrm.offset += 2;
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        selector = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        TYPE_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.es, instruction_state.data.rr, selector, offset, 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID LDS_R32_M16_32(core_machine_cpu_execution_context *context)
{
    type_unsigned_16 selector;
    type_unsigned_32 offset;
    TYPE_TRACE_CALL_BEGIN("LDS_R32_M16_32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm_ea(context, _GetOperandSize,
            _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            offset = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
            break;
        case 4:
            offset = TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.mrm.offset += _GetOperandSize;
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        selector = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        TYPE_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.ds, instruction_state.data.rr, selector, offset, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm_ea(context, 2, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        offset = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        instruction_state.data.mrm.offset += 2;
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        selector = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        TYPE_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.ds, instruction_state.data.rr, selector, offset, 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_C6(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INS_C6");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    switch (instruction_state.data.cr)
    {
    case 0: /* MOV_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("MOV_RM8_I8");
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        instruction_state.data.crm = instruction_state.data.cimm;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 1:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 3:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_C7(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INS_C7");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        switch (instruction_state.data.cr)
        {
        case 0: /* MOV_RM32_I32 */
            TYPE_TRACE_BLOCK_BEGIN("MOV_RM32_I32");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
            instruction_state.data.crm = instruction_state.data.cimm;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 5:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 6:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        switch (instruction_state.data.cr)
        {
        case 0: /* MOV_RM16_I16 */
            TYPE_TRACE_BLOCK_BEGIN("MOV_RM16_I16");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            instruction_state.data.crm = instruction_state.data.cimm;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 5:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 6:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7:
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID ENTER(core_machine_cpu_execution_context *context)
{
    type_unsigned_32 data = 0;
    type_unsigned_32 temp = 0;
    type_native_unsigned i = 0;
    type_unsigned_16 size = TYPE_ZERO_16;
    type_unsigned_8 level = TYPE_ZERO_8;
    TYPE_TRACE_CALL_BEGIN("ENTER");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        size = (type_unsigned_16)instruction_state.data.cimm;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        level = (type_unsigned_8)instruction_state.data.cimm;
        /* The 80186 defines the byte as an unsigned lexical level through
         * 255.  The later 80286/80386 architecture limits it to 0--31. */
        if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80286)
            level %= 32;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.bp), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(cpu_state.data.ebp), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        switch (_GetStackSize)
        {
        case 2:
            temp = cpu_state.data.sp;
            break;
        case 4:
            temp = cpu_state.data.esp;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        if (level)
        {
            TYPE_TRACE_BLOCK_BEGIN("level(!0)");
            for (i = 1; i < level; ++i)
            {
                TYPE_TRACE_BLOCK_BEGIN("for");
                switch (_GetOperandSize)
                {
                case 2:
                    TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
                    switch (_GetStackSize)
                    {
                    case 2:
                        TYPE_TRACE_BLOCK_BEGIN("StackSize(2)");
                        cpu_state.data.bp -= 2;
                        TYPE_TRACE_CHECK_RETURN(_s_read_ss(context, cpu_state.data.bp, TYPE_REFERENCE_OF(data), 2));
                        TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(data), 2));
                        TYPE_TRACE_BLOCK_END;
                        break;
                    case 4:
                        TYPE_TRACE_BLOCK_BEGIN("StackSize(4)");
                        cpu_state.data.ebp -= 2;
                        TYPE_TRACE_CHECK_RETURN(_s_read_ss(context, cpu_state.data.ebp, TYPE_REFERENCE_OF(data), 2));
                        TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(data), 2));
                        TYPE_TRACE_BLOCK_END;
                        break;
                    default:
                        TYPE_TRACE_IMPOSSIBLE_RETURN;
                        break;
                    }
                    TYPE_TRACE_BLOCK_END;
                    break;
                case 4:
                    TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
                    switch (_GetStackSize)
                    {
                    case 2:
                        TYPE_TRACE_BLOCK_BEGIN("StackSize(2)");
                        cpu_state.data.bp -= 4;
                        TYPE_TRACE_CHECK_RETURN(_s_read_ss(context, cpu_state.data.bp, TYPE_REFERENCE_OF(data), 4));
                        TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(data), 4));
                        TYPE_TRACE_BLOCK_END;
                        break;
                    case 4:
                        TYPE_TRACE_BLOCK_BEGIN("StackSize(4)");
                        cpu_state.data.ebp -= 4;
                        TYPE_TRACE_CHECK_RETURN(_s_read_ss(context, cpu_state.data.ebp, TYPE_REFERENCE_OF(data), 4));
                        TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(data), 4));
                        TYPE_TRACE_BLOCK_END;
                        break;
                    default:
                        TYPE_TRACE_IMPOSSIBLE_RETURN;
                        break;
                    }
                    TYPE_TRACE_BLOCK_END;
                    break;
                default:
                    TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                TYPE_TRACE_BLOCK_END;
            }
            switch (_GetOperandSize)
            {
            case 2:
                TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
                TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(temp), 2));
                TYPE_TRACE_BLOCK_END;
                break;
            case 4:
                TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
                TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(temp), 4));
                TYPE_TRACE_BLOCK_END;
                break;
            default:
                TYPE_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            TYPE_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            cpu_state.data.bp = TYPE_MASK_UNSIGNED_16(temp);
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            cpu_state.data.ebp = TYPE_MASK_UNSIGNED_32(temp);
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        switch (_GetStackSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("StackSize(2)");
            cpu_state.data.sp -= size;
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("StackSize(4)");
            cpu_state.data.esp -= size;
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID LEAVE(core_machine_cpu_execution_context *context)
{
    type_unsigned_32 stack = TYPE_ZERO_32;
    TYPE_TRACE_CALL_BEGIN("LEAVE");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186)
    {
        _adv;
        switch (_GetStackSize)
        {
        case 2:
            stack = cpu_state.data.bp;
            break;
        case 4:
            stack = cpu_state.data.ebp;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        if (!_IsProtected && stack > 0x0000ffff)
        {
            TYPE_TRACE_BLOCK_BEGIN("Protected(0),ebp(>0000ffff)");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_CHECK_RETURN(_m_test_logical(context, &cpu_state.data.ss,
            stack, _GetOperandSize, 1));
        switch (_GetStackSize)
        {
        case 2:
            cpu_state.data.sp = cpu_state.data.bp;
            break;
        case 4:
            cpu_state.data.esp = cpu_state.data.ebp;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.bp), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_e_pop(context, TYPE_REFERENCE_OF(cpu_state.data.ebp), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    TYPE_TRACE_CALL_END;
}
static C_VOID RETF_I16(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("RETF_I16");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_e_ret_far(context, TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm), _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        _d_imm(context, 2);
        _e_ret_far(context, TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm), 2);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID RETF(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("RETF");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_e_ret_far(context, 0, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_ret_far(context, 0, 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INT3(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INT3");
    _new_code_path_;
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_e_int3(context, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        _e_int_n(context, 0x03, _GetOperandSize);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INT_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INT_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        TYPE_TRACE_CHECK_RETURN(_e_int_n(context,
            (type_unsigned_8)instruction_state.data.cimm, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        _d_imm(context, 1);
        _e_int_n(context, (type_unsigned_8)instruction_state.data.cimm, 2);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INTO(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INTO");
    _new_code_path_;
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_e_into(context, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        if (_GetEFLAGS_OF)
            _e_int_n(context, 0x04, _GetOperandSize);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID IRET(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("IRET");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_e_iret(context, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_e_iret(context, 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_D0(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INS_D0");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    switch (instruction_state.data.cr)
    {
    case 0: /* ROL_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("ROL_RM8");
        TYPE_TRACE_CHECK_RETURN(_a_rol(context, instruction_state.data.crm, 1, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("ROR_RM8");
        TYPE_TRACE_CHECK_RETURN(_a_ror(context, instruction_state.data.crm, 1, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("RCL_RM8");
        TYPE_TRACE_CHECK_RETURN(_a_rcl(context, instruction_state.data.crm, 1, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("RCR_RM8");
        TYPE_TRACE_CHECK_RETURN(_a_rcr(context, instruction_state.data.crm, 1, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("SHL_RM8");
        TYPE_TRACE_CHECK_RETURN(_a_shl(context, instruction_state.data.crm, 1, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("SHR_RM8");
        TYPE_TRACE_CHECK_RETURN(_a_shr(context, instruction_state.data.crm, 1, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("SAR_RM8");
        TYPE_TRACE_CHECK_RETURN(_a_sar(context, instruction_state.data.crm, 1, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    instruction_state.data.crm = instruction_state.data.result;
    TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_D1(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INS_D1");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        switch (instruction_state.data.cr)
        {
        case 0: /* ROL_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("ROL_RM32");
            TYPE_TRACE_CHECK_RETURN(_a_rol(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* ROR_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("ROR_RM32");
            TYPE_TRACE_CHECK_RETURN(_a_ror(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* RCL_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("RCL_RM32");
            TYPE_TRACE_CHECK_RETURN(_a_rcl(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* RCR_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("RCR_RM32");
            TYPE_TRACE_CHECK_RETURN(_a_rcr(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* SHL_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("SHL_RM32");
            TYPE_TRACE_CHECK_RETURN(_a_shl(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SHR_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("SHR_RM32");
            TYPE_TRACE_CHECK_RETURN(_a_shr(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* UndefinedOpcode */
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* SAR_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("SAR_RM32");
            TYPE_TRACE_CHECK_RETURN(_a_sar(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        switch (instruction_state.data.cr)
        {
        case 0: /* ROL_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("ROL_RM16");
            TYPE_TRACE_CHECK_RETURN(_a_rol(context, instruction_state.data.crm, 1, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* ROR_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("ROR_RM16");
            TYPE_TRACE_CHECK_RETURN(_a_ror(context, instruction_state.data.crm, 1, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* RCL_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("RCL_RM16");
            TYPE_TRACE_CHECK_RETURN(_a_rcl(context, instruction_state.data.crm, 1, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* RCR_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("RCR_RM16");
            TYPE_TRACE_CHECK_RETURN(_a_rcr(context, instruction_state.data.crm, 1, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* SHL_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("SHL_RM16");
            TYPE_TRACE_CHECK_RETURN(_a_shl(context, instruction_state.data.crm, 1, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SHR_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("SHR_RM16");
            TYPE_TRACE_CHECK_RETURN(_a_shr(context, instruction_state.data.crm, 1, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* UndefinedOpcode */
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* SAR_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("SAR_RM16");
            TYPE_TRACE_CHECK_RETURN(_a_sar(context, instruction_state.data.crm, 1, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_D2(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INS_D2");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    switch (instruction_state.data.cr)
    {
    case 0: /* ROL_RM8_CL */
        TYPE_TRACE_BLOCK_BEGIN("ROL_RM8_CL");
        TYPE_TRACE_CHECK_RETURN(_a_rol(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM8_CL */
        TYPE_TRACE_BLOCK_BEGIN("ROR_RM8_CL");
        TYPE_TRACE_CHECK_RETURN(_a_ror(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM8_CL */
        TYPE_TRACE_BLOCK_BEGIN("RCL_RM8_CL");
        TYPE_TRACE_CHECK_RETURN(_a_rcl(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM8_CL */
        TYPE_TRACE_BLOCK_BEGIN("RCR_RM8_CL");
        TYPE_TRACE_CHECK_RETURN(_a_rcr(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM8_CL */
        TYPE_TRACE_BLOCK_BEGIN("SHL_RM8_CL");
        TYPE_TRACE_CHECK_RETURN(_a_shl(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM8_CL */
        TYPE_TRACE_BLOCK_BEGIN("SHR_RM8_CL");
        TYPE_TRACE_CHECK_RETURN(_a_shr(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM8_CL */
        TYPE_TRACE_BLOCK_BEGIN("SAR_RM8_CL");
        TYPE_TRACE_CHECK_RETURN(_a_sar(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    instruction_state.data.crm = instruction_state.data.result;
    TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_D3(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INS_D3");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        switch (instruction_state.data.cr)
        {
        case 0: /* ROL_RM32_CL */
            TYPE_TRACE_BLOCK_BEGIN("ROL_RM32_CL");
            TYPE_TRACE_CHECK_RETURN(_a_rol(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* ROR_RM32_CL */
            TYPE_TRACE_BLOCK_BEGIN("ROR_RM32_CL");
            TYPE_TRACE_CHECK_RETURN(_a_ror(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* RCL_RM32_CL */
            TYPE_TRACE_BLOCK_BEGIN("RCL_RM32_CL");
            TYPE_TRACE_CHECK_RETURN(_a_rcl(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* RCR_RM32_CL */
            TYPE_TRACE_BLOCK_BEGIN("RCR_RM32_CL");
            TYPE_TRACE_CHECK_RETURN(_a_rcr(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* SHL_RM32_CL */
            TYPE_TRACE_BLOCK_BEGIN("SHL_RM32_CL");
            TYPE_TRACE_CHECK_RETURN(_a_shl(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SHR_RM32_CL */
            TYPE_TRACE_BLOCK_BEGIN("SHR_RM32_CL");
            TYPE_TRACE_CHECK_RETURN(_a_shr(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* UndefinedOpcode */
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* SAR_RM32_CL */
            TYPE_TRACE_BLOCK_BEGIN("SAR_RM32_CL");
            TYPE_TRACE_CHECK_RETURN(_a_sar(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        switch (instruction_state.data.cr)
        {
        case 0: /* ROL_RM16_CL */
            TYPE_TRACE_BLOCK_BEGIN("ROL_RM16_CL");
            TYPE_TRACE_CHECK_RETURN(_a_rol(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* ROR_RM16_CL */
            TYPE_TRACE_BLOCK_BEGIN("ROR_RM16_CL");
            TYPE_TRACE_CHECK_RETURN(_a_ror(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* RCL_RM16_CL */
            TYPE_TRACE_BLOCK_BEGIN("RCL_RM16_CL");
            TYPE_TRACE_CHECK_RETURN(_a_rcl(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* RCR_RM16_CL */
            TYPE_TRACE_BLOCK_BEGIN("RCR_RM16_CL");
            TYPE_TRACE_CHECK_RETURN(_a_rcr(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* SHL_RM16_CL */
            TYPE_TRACE_BLOCK_BEGIN("SHL_RM16_CL");
            TYPE_TRACE_CHECK_RETURN(_a_shl(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* SHR_RM16_CL */
            TYPE_TRACE_BLOCK_BEGIN("SHR_RM16_CL");
            TYPE_TRACE_CHECK_RETURN(_a_shr(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* UndefinedOpcode */
            TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* SAR_RM16_CL */
            TYPE_TRACE_BLOCK_BEGIN("SAR_RM16_CL");
            TYPE_TRACE_CHECK_RETURN(_a_sar(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID AAM(core_machine_cpu_execution_context *context)
{
    type_unsigned_8 base;
    TYPE_TRACE_CALL_BEGIN("AAM");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    }
    else
    {
        cpu_state.data.ip++;
        _d_imm(context, 1);
    }
    base = TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    if (base == 0)
    {
        TYPE_TRACE_CHECK_RETURN(_SetExcept_DE(0));
        return;
    }
    cpu_state.data.ah = cpu_state.data.al / base;
    cpu_state.data.al = cpu_state.data.al % base;
    instruction_state.data.bit = 0x08;
    instruction_state.data.result = TYPE_MASK_UNSIGNED_8(cpu_state.data.al);
    TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, AAM_FLAG));
    instruction_state.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF);
    TYPE_TRACE_CALL_END;
}
static C_VOID AAD(core_machine_cpu_execution_context *context)
{
    type_unsigned_8 base;
    TYPE_TRACE_CALL_BEGIN("AAD");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    }
    else
    {
        cpu_state.data.ip++;
        _d_imm(context, 1);
    }
    base = TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm);
    cpu_state.data.al = TYPE_MASK_UNSIGNED_8(cpu_state.data.al + (cpu_state.data.ah * base));
    cpu_state.data.ah = 0x00;
    instruction_state.data.bit = 0x08;
    instruction_state.data.result = TYPE_MASK_UNSIGNED_8(cpu_state.data.al);
    TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, AAD_FLAG));
    instruction_state.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF);
    TYPE_TRACE_CALL_END;
}
static C_VOID XLAT(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("XLAT");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetAddressSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("AddressSize(2)");
            TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, (cpu_state.data.bx + cpu_state.data.al), TYPE_REFERENCE_OF(cpu_state.data.al), 1));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("AddressSize(4)");
            TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, (cpu_state.data.ebx + cpu_state.data.al), TYPE_REFERENCE_OF(cpu_state.data.al), 1));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, (cpu_state.data.bx + cpu_state.data.al), TYPE_REFERENCE_OF(cpu_state.data.al), 1));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID LOOPNZ_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("LOOPNZ_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_loopcc(context, (type_signed_8)instruction_state.data.cimm, !_GetEFLAGS_ZF));
    TYPE_TRACE_CALL_END;
}
static C_VOID LOOPZ_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("LOOPZ_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_loopcc(context, (type_signed_8)instruction_state.data.cimm, _GetEFLAGS_ZF));
    TYPE_TRACE_CALL_END;
}
static C_VOID LOOP_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("LOOP_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_loopcc(context, (type_signed_8)instruction_state.data.cimm, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID JCXZ_REL8(core_machine_cpu_execution_context *context)
{
    type_unsigned_32 cecx = 0x00000000;
    TYPE_TRACE_CALL_BEGIN("JCXZ_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        switch (_GetAddressSize)
        {
        case 2:
            cecx = cpu_state.data.cx;
            break;
        case 4:
            cecx = cpu_state.data.ecx;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, !cecx));
    }
    else
    {
        cpu_state.data.ip++;
        _d_imm(context, 1);
        _e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, !cpu_state.data.cx);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID IN_AL_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("IN_AL_I8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_p_input(context, TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), TYPE_REFERENCE_OF(cpu_state.data.al), 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID IN_EAX_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("IN_EAX_I8");
    _new_code_path_;
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_p_input(context, TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_p_input(context, TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        TYPE_TRACE_CHECK_RETURN(_p_input(context, TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID OUT_I8_AL(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("OUT_I8_AL");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_p_output(context, TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), TYPE_REFERENCE_OF(cpu_state.data.al), 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID OUT_I8_EAX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("OUT_I8_EAX");
    _new_code_path_;
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_p_output(context, TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_p_output(context, TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        TYPE_TRACE_CHECK_RETURN(_p_output(context,
            TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm),
            TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID CALL_REL32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("CALL_REL32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            TYPE_TRACE_CHECK_RETURN(_e_call_near(context, TYPE_MASK_UNSIGNED_16(cpu_state.data.ip + (type_signed_16)instruction_state.data.cimm), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            TYPE_TRACE_CHECK_RETURN(_e_call_near(context, TYPE_MASK_UNSIGNED_32(cpu_state.data.eip + (type_signed_32)instruction_state.data.cimm), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(_GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_e_call_near(context, TYPE_MASK_UNSIGNED_16(cpu_state.data.ip + (type_signed_16)instruction_state.data.cimm), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JMP_REL32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JMP_REL32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, 1));
    }
    else
    {
        cpu_state.data.ip++;
        _d_imm(context, 2);
        _e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 2, 1);
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JMP_PTR16_32(core_machine_cpu_execution_context *context)
{
    type_unsigned_16 newcs = 0x0000;
    type_unsigned_32 neweip = 0x00000000;
    TYPE_TRACE_CALL_BEGIN("JMP_PTR16_32");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            neweip = (type_unsigned_16)instruction_state.data.cimm;
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            newcs = (type_unsigned_16)instruction_state.data.cimm;
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 4));
            neweip = (type_unsigned_32)instruction_state.data.cimm;
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            newcs = (type_unsigned_16)instruction_state.data.cimm;
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        TYPE_TRACE_CHECK_RETURN(_e_jmp_far(context, newcs, neweip, _GetOperandSize));
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        neweip = (type_unsigned_16)instruction_state.data.cimm;
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
        newcs = (type_unsigned_16)instruction_state.data.cimm;
        TYPE_TRACE_CHECK_RETURN(_e_jmp_far(context, newcs, neweip, 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID JMP_REL8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JMP_REL8");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), 1, 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID IN_AL_DX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("IN_AL_DX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, TYPE_REFERENCE_OF(cpu_state.data.al), 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID IN_EAX_DX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("IN_EAX_DX");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID OUT_DX_AL(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("OUT_DX_AL");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx, TYPE_REFERENCE_OF(cpu_state.data.al), 1));
    TYPE_TRACE_CALL_END;
}
static C_VOID OUT_DX_EAX(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("OUT_DX_EAX");
    _new_code_path_;
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
            TYPE_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx, TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4:
            TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
            TYPE_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx, TYPE_REFERENCE_OF(cpu_state.data.eax), 4));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx,
            TYPE_REFERENCE_OF(cpu_state.data.ax), 2));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_LOCK(core_machine_cpu_execution_context *context)
{
    type_unsigned_8 opcode = 0x00;
    type_unsigned_8 modrm = 0x00;
    type_unsigned_8 opcode_0f = 0x00;
    type_unsigned_32 ceip = cpu_state.data.eip;
    TYPE_TRACE_CALL_BEGIN("PREFIX_LOCK");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        do
        {
            TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, ceip, TYPE_REFERENCE_OF(opcode), 1));
            ceip++;
        } while (_kdf_check_prefix(context, opcode));
        switch (opcode)
        {
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
        case 0xfe:
            TYPE_TRACE_BLOCK_BEGIN("opcode(0xfe)");
            TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, ceip,
                TYPE_REFERENCE_OF(modrm), 1));
            if (_GetModRM_REG(modrm) <= 1u)
                instruction_state.data.flagLock = TYPE_TRUE;
            else
                TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
            TYPE_TRACE_BLOCK_END;
            break;
        case 0x80:
        case 0x81:
        case 0x83:
            TYPE_TRACE_BLOCK_BEGIN("opcode(0x80/0x81/0x83)");
            TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, ceip, TYPE_REFERENCE_OF(modrm), 1));
            if (_GetModRM_REG(modrm) != 7)
                instruction_state.data.flagLock = TYPE_TRUE;
            else
            {
                TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_BLOCK_END;
            break;
        case 0x0f:
            TYPE_TRACE_BLOCK_BEGIN("opcode(0f)");
            TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, ceip, TYPE_REFERENCE_OF(opcode_0f), 1));
            switch (opcode_0f)
            {
            case 0xab: /* BTS */
            case 0xb3: /* BTR */
            case 0xbb: /* BTC */
                instruction_state.data.flagLock = TYPE_TRUE;
                break;
            case 0xba:
                TYPE_TRACE_BLOCK_BEGIN("opcode_0f(0xba)");
                TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, ceip + 1u,
                    TYPE_REFERENCE_OF(modrm), 1));
                if (_GetModRM_REG(modrm) >= 5u)
                    instruction_state.data.flagLock = TYPE_TRUE;
                else
                    TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
                TYPE_TRACE_BLOCK_END;
                break;
            default:
                TYPE_TRACE_BLOCK_BEGIN("opcode_0f");
                TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
                TYPE_TRACE_BLOCK_END;
                break;
            }
            TYPE_TRACE_BLOCK_END;
            break;
        case 0xf6:
        case 0xf7:
            TYPE_TRACE_BLOCK_BEGIN("opcode(0xf6/0xf7)");
            TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, ceip,
                TYPE_REFERENCE_OF(modrm), 1));
            if (_GetModRM_REG(modrm) == 2u || _GetModRM_REG(modrm) == 3u)
                instruction_state.data.flagLock = TYPE_TRUE;
            else
                TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
            TYPE_TRACE_BLOCK_END;
            break;
        case 0xff:
            TYPE_TRACE_BLOCK_BEGIN("opcode(0xff)");
            TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, ceip,
                TYPE_REFERENCE_OF(modrm), 1));
            if (_GetModRM_REG(modrm) <= 1u)
                instruction_state.data.flagLock = TYPE_TRUE;
            else
                TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_BLOCK_BEGIN("opcode");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
            TYPE_TRACE_BLOCK_END;
            break;
        }
    }
    else
    {
        /* Before the 80386, LOCK is a bus prefix for the following valid
         * instruction.  The 80286 additionally treats it as IOPL-sensitive
         * in protected mode; it does not use the later 80386 opcode whitelist.
         */
        if (context->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 &&
            _IsProtected && _GetCPL > (type_unsigned_8)_GetEFLAGS_IOPL)
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        else
            _adv;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_REPNZ(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_REPNZ");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        instruction_state.data.prefix_rep = PREFIX_REP_REPZNZ;
    }
    else
    {
        cpu_state.data.ip++;
        instruction_state.data.prefix_rep = PREFIX_REP_REPZNZ;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PREFIX_REPZ(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("PREFIX_REPZ");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        instruction_state.data.prefix_rep = PREFIX_REP_REPZ;
    }
    else
    {
        cpu_state.data.ip++;
        instruction_state.data.prefix_rep = PREFIX_REP_REPZ;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID HLT(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("HLT");
    _new_code_path_;
    if (_GetCR0_PE && _GetCPL)
    {
        TYPE_TRACE_BLOCK_BEGIN("CR0_PE(1),CPL(!0)");
        _SetExcept_GP(0);
        TYPE_TRACE_BLOCK_END;
    }
    _adv;
    cpu_state.data.flagHalt = TYPE_TRUE;
    TYPE_TRACE_CALL_END;
}
static C_VOID CMC(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("CMC");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        cpu_state.data.eflags ^= VCPU_EFLAGS_CF;
    }
    else
    {
        cpu_state.data.ip++;
        cpu_state.data.eflags ^= VCPU_EFLAGS_CF;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_F6(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INS_F6");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    switch (instruction_state.data.cr)
    {
    case 0: /* TEST_RM8_I8 */
        TYPE_TRACE_BLOCK_BEGIN("TEST_RM8_I8");
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
        TYPE_TRACE_CHECK_RETURN(_a_test(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* UndefinedOpcode */
        TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(1)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* NOT_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("NOT_RM8");
        TYPE_TRACE_CHECK_RETURN(_a_not(context, instruction_state.data.crm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* NEG_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("NEG_RM8");
        TYPE_TRACE_CHECK_RETURN(_a_neg(context, instruction_state.data.crm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* MUL_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("MUL_RM8");
        TYPE_TRACE_CHECK_RETURN(_a_mul(context, instruction_state.data.crm, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* IMUL_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("IMUL_RM8");
        TYPE_TRACE_CHECK_RETURN(_a_imul(context, instruction_state.data.crm, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* DIV_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("DIV_RM8");
        TYPE_TRACE_CHECK_RETURN(_a_div(context, instruction_state.data.crm, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* IDIV_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("IDIV_RM8");
        _new_code_path_;
        TYPE_TRACE_CHECK_RETURN(_a_idiv(context, instruction_state.data.crm, 8));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_F7(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INS_F7");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        switch (instruction_state.data.cr)
        {
        case 0: /* TEST_RM32_I32 */
            TYPE_TRACE_BLOCK_BEGIN("TEST_RM32_I32");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
            TYPE_TRACE_CHECK_RETURN(_a_test(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* UndefinedOpcode */
            TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(1)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* NOT_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("NOT_RM32");
            TYPE_TRACE_CHECK_RETURN(_a_not(context, instruction_state.data.crm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* NEG_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("NEG_RM32");
            TYPE_TRACE_CHECK_RETURN(_a_neg(context, instruction_state.data.crm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* MUL_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("MUL_RM32");
            TYPE_TRACE_CHECK_RETURN(_a_mul(context, instruction_state.data.crm, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* IMUL_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("IMUL_RM32");
            TYPE_TRACE_CHECK_RETURN(_a_imul(context, instruction_state.data.crm, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* DIV_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("DIV_RM32");
            TYPE_TRACE_CHECK_RETURN(_a_div(context, instruction_state.data.crm, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* IDIV_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("IDIV_RM32");
            TYPE_TRACE_CHECK_RETURN(_a_idiv(context, instruction_state.data.crm, _GetOperandSize * 8));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        switch (instruction_state.data.cr)
        {
        case 0: /* TEST_RM16_I16 */
            TYPE_TRACE_BLOCK_BEGIN("TEST_RM16_I16");
            TYPE_TRACE_CHECK_RETURN(_d_imm(context, 2));
            TYPE_TRACE_CHECK_RETURN(_a_test(context, instruction_state.data.crm, instruction_state.data.cimm, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* UndefinedOpcode */
            TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(1)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* NOT_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("NOT_RM16");
            TYPE_TRACE_CHECK_RETURN(_a_not(context, instruction_state.data.crm, 16));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* NEG_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("NEG_RM16");
            TYPE_TRACE_CHECK_RETURN(_a_neg(context, instruction_state.data.crm, 16));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* MUL_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("MUL_RM16");
            TYPE_TRACE_CHECK_RETURN(_a_mul(context, instruction_state.data.crm, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* IMUL_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("IMUL_RM16");
            TYPE_TRACE_CHECK_RETURN(_a_imul(context, instruction_state.data.crm, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* DIV_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("DIV_RM16");
            TYPE_TRACE_CHECK_RETURN(_a_div(context, instruction_state.data.crm, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* IDIV_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("IDIV_RM16");
            TYPE_TRACE_CHECK_RETURN(_a_idiv(context, instruction_state.data.crm, 16));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID CLC(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("CLC");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        _ClrEFLAGS_CF;
    }
    else
    {
        cpu_state.data.ip++;
        _ClrEFLAGS_CF;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID STC(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("STC");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        _SetEFLAGS_CF;
    }
    else
    {
        cpu_state.data.ip++;
        _SetEFLAGS_CF;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID CLI(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("CLI");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        if (!_GetCR0_PE)
            _ClrEFLAGS_IF;
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("CR0_PE(1)");
            _new_code_path_;
            if (!_GetEFLAGS_VM)
            {
                TYPE_TRACE_BLOCK_BEGIN("EFLAGS_VM(0)");
                if (_GetCPL <= (type_unsigned_8)(_GetEFLAGS_IOPL))
                    _ClrEFLAGS_IF;
                else
                    TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                TYPE_TRACE_BLOCK_END;
            }
            else
            {
                TYPE_TRACE_BLOCK_BEGIN("EFLAGS_VM(1)");
                if (_GetEFLAGS_IOPL == 3)
                    _ClrEFLAGS_IF;
                else
                    TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_BLOCK_END;
        }
    }
    else
    {
        cpu_state.data.ip++;
        _ClrEFLAGS_IF;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID STI(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("STI");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        if (!_GetCR0_PE)
            _SetEFLAGS_IF;
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("CR0_PE(1)");
            _new_code_path_;
            if (!_GetEFLAGS_VM)
            {
                TYPE_TRACE_BLOCK_BEGIN("EFLAGS_VM(0)");
                if (_GetCPL <= (type_unsigned_8)(_GetEFLAGS_IOPL))
                    _SetEFLAGS_IF;
                else
                    TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                TYPE_TRACE_BLOCK_END;
            }
            else
            {
                TYPE_TRACE_BLOCK_BEGIN("EFLAGS_VM(1)");
                if (_GetEFLAGS_IOPL == 3)
                    _SetEFLAGS_IF;
                else
                    TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_BLOCK_END;
        }
    }
    else
    {
        cpu_state.data.ip++;
        _SetEFLAGS_IF;
    }
    instruction_state.data.flagMaskInt = TYPE_TRUE;
    TYPE_TRACE_CALL_END;
}
static C_VOID CLD(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("CLD");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        _ClrEFLAGS_DF;
    }
    else
    {
        cpu_state.data.ip++;
        _ClrEFLAGS_DF;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID STD(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("STD");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        _SetEFLAGS_DF;
    }
    else
    {
        cpu_state.data.ip++;
        _SetEFLAGS_DF;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_FE(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("INS_FE");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
    }
    else
    {
        cpu_state.data.ip++;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    switch (instruction_state.data.cr)
    {
    case 0: /* INC_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("INC_RM8");
        TYPE_TRACE_CHECK_RETURN(_a_inc(context, instruction_state.data.crm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* DEC_RM8 */
        TYPE_TRACE_BLOCK_BEGIN("DEC_RM8");
        TYPE_TRACE_CHECK_RETURN(_a_dec(context, instruction_state.data.crm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 3:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_FF(core_machine_cpu_execution_context *context)
{
    type_unsigned_8 modrm;
    type_unsigned_16 newcs;
    type_unsigned_32 neweip;
    TYPE_TRACE_CALL_BEGIN("INS_FF");
    if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386)
    {
        _adv;
        TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, TYPE_REFERENCE_OF(modrm), 1));
        switch (_GetModRM_REG(modrm))
        {
        case 0: /* INC_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("INC_RM32");
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
            TYPE_TRACE_CHECK_RETURN(_a_inc(context, instruction_state.data.crm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* DEC_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("DEC_RM32");
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
            TYPE_TRACE_CHECK_RETURN(_a_dec(context, instruction_state.data.crm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* CALL_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("CALL_RM32");
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
            neweip = TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
            TYPE_TRACE_CHECK_RETURN(_e_call_near(context, neweip, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* CALL_M16_32 */
            TYPE_TRACE_BLOCK_BEGIN("CALL_M16_32");
            if (instruction_state.data.flagLock)
                TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            if (!instruction_state.data.flagMem)
            {
                TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
                TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
            neweip = (type_unsigned_32)instruction_state.data.crm;
            instruction_state.data.mrm.offset += _GetOperandSize;
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            newcs = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
            TYPE_TRACE_CHECK_RETURN(_e_call_far(context, newcs, neweip, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* JMP_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("JMP_RM32");
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
            neweip = TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
            TYPE_TRACE_CHECK_RETURN(_e_jmp_near(context, neweip, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* JMP_M16_32 */
            TYPE_TRACE_BLOCK_BEGIN("JMP_M16_32");
            if (instruction_state.data.flagLock)
                TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            if (!instruction_state.data.flagMem)
            {
                TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
                TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
            neweip = (type_unsigned_32)instruction_state.data.crm;
            instruction_state.data.mrm.offset += _GetOperandSize;
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            newcs = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
            TYPE_TRACE_CHECK_RETURN(_e_jmp_far(context, newcs, neweip, _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* PUSH_RM32 */
            TYPE_TRACE_BLOCK_BEGIN("PUSH_RM32");
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            if (instruction_state.data.flagLock)
                TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* UndefinedOpcode */
            TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        cpu_state.data.ip++;
        TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, TYPE_REFERENCE_OF(modrm), 1));
        switch (_GetModRM_REG(modrm))
        {
        case 0: /* INC_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("INC_RM16");
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            TYPE_TRACE_CHECK_RETURN(_a_inc(context, instruction_state.data.crm, 16));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* DEC_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("DEC_RM16");
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            TYPE_TRACE_CHECK_RETURN(_a_dec(context, instruction_state.data.crm, 16));
            instruction_state.data.crm = instruction_state.data.result;
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* CALL_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("CALL_RM16");
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            neweip = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
            TYPE_TRACE_CHECK_RETURN(_e_call_near(context, neweip, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* CALL_M16_16 */
            TYPE_TRACE_BLOCK_BEGIN("CALL_M16_16");
            if (instruction_state.data.flagLock)
                TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            if (!instruction_state.data.flagMem)
            {
                TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
                TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            neweip = (type_unsigned_16)instruction_state.data.crm;
            instruction_state.data.mrm.offset += 2;
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            newcs = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
            TYPE_TRACE_CHECK_RETURN(_e_call_far(context, newcs, neweip, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* JMP_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("JMP_RM16");
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            neweip = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
            TYPE_TRACE_CHECK_RETURN(_e_jmp_near(context, neweip, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* JMP_M16_16 */
            TYPE_TRACE_BLOCK_BEGIN("JMP_M16_16");
            if (instruction_state.data.flagLock)
                TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            if (!instruction_state.data.flagMem)
            {
                TYPE_TRACE_BLOCK_BEGIN("flagMem(0)");
                TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            neweip = (type_unsigned_16)instruction_state.data.crm;
            instruction_state.data.mrm.offset += 2;
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            newcs = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
            TYPE_TRACE_CHECK_RETURN(_e_jmp_far(context, newcs, neweip, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 6: /* PUSH_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("PUSH_RM16");
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            if (instruction_state.data.flagLock)
                TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(instruction_state.data.crm), 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7: /* UndefinedOpcode */
            TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    TYPE_TRACE_CALL_END;
}

static C_VOID _d_modrm_creg(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("_d_modrm_creg");
    TYPE_TRACE_CHECK_RETURN(_kdf_modrm_creg(context));
    if (instruction_state.data.flagMem &&
        !context->cpu_80386_cr_mov_ignores_mod)
    {
        TYPE_TRACE_BLOCK_BEGIN("flagMem(1)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
    }
    switch (instruction_state.data.cr)
    {
    case 0:
        instruction_state.data.rr = (type_virtual_address)&cpu_state.data.cr0;
        instruction_state.data.cr = cpu_state.data.cr0;
        break;
    case 1:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        instruction_state.data.rr = (type_virtual_address)&cpu_state.data.cr2;
        instruction_state.data.cr = cpu_state.data.cr2;
        break;
    case 3:
        instruction_state.data.rr = (type_virtual_address)&cpu_state.data.cr3;
        instruction_state.data.cr = cpu_state.data.cr3;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _d_modrm_dreg(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("_d_modrm_dreg");
    TYPE_TRACE_CHECK_RETURN(_kdf_modrm(context, 0, 4));
    if (instruction_state.data.flagMem)
    {
        TYPE_TRACE_BLOCK_BEGIN("flagMem(1)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
    }
    switch (instruction_state.data.cr)
    {
    case 0:
        instruction_state.data.rr = (type_virtual_address)&cpu_state.data.dr0;
        instruction_state.data.cr = cpu_state.data.dr0;
        break;
    case 1:
        instruction_state.data.rr = (type_virtual_address)&cpu_state.data.dr1;
        instruction_state.data.cr = cpu_state.data.dr1;
        break;
    case 2:
        instruction_state.data.rr = (type_virtual_address)&cpu_state.data.dr2;
        instruction_state.data.cr = cpu_state.data.dr2;
        break;
    case 3:
        instruction_state.data.rr = (type_virtual_address)&cpu_state.data.dr3;
        instruction_state.data.cr = cpu_state.data.dr3;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        instruction_state.data.rr = (type_virtual_address)&cpu_state.data.dr6;
        instruction_state.data.cr = cpu_state.data.dr6;
        break;
    case 7:
        instruction_state.data.rr = (type_virtual_address)&cpu_state.data.dr7;
        instruction_state.data.cr = cpu_state.data.dr7;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _d_modrm_treg(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("_d_modrm_treg");
    TYPE_TRACE_CHECK_RETURN(_kdf_modrm(context, 0, 4));
    if (instruction_state.data.flagMem)
    {
        TYPE_TRACE_BLOCK_BEGIN("flagMem(1)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
    }
    switch (instruction_state.data.cr)
    {
    case 0:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(0)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
        break;
    case 1:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
        break;
    case 3:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6:
        instruction_state.data.rr = (type_virtual_address)&cpu_state.data.tr6;
        instruction_state.data.cr = cpu_state.data.tr6;
        break;
    case 7:
        instruction_state.data.rr = (type_virtual_address)&cpu_state.data.tr7;
        instruction_state.data.cr = cpu_state.data.tr7;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _d_bit_rmimm(core_machine_cpu_execution_context *context, type_unsigned_8 regbyte, type_unsigned_8 rmbyte, type_bool write)
{
    /* xrm = actual destination, cimm = (1 << bitoffset) */
    type_signed_16 bitoff16 = 0;
    type_signed_32 bitoff32 = 0;
    type_unsigned_32 bitoperand = 0;
    TYPE_TRACE_CALL_BEGIN("_d_bit_rmimm");
    TYPE_TRACE_CHECK_RETURN(_kdf_modrm(context, regbyte, rmbyte));
    if (!regbyte)
        TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    switch (rmbyte)
    {
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("rmbyte(2)");
        if (instruction_state.data.flagMem && regbyte)
        {
            /* valid for btcc_m16_r16 */
            TYPE_TRACE_BLOCK_BEGIN("flagMem(1),regbyte(1)");
            bitoff16 = (type_signed_16)instruction_state.data.cr;
            if (bitoff16 >= 0)
                instruction_state.data.mrm.offset += 2 * (bitoff16 / 16);
            else
                instruction_state.data.mrm.offset += 2 * ((bitoff16 - 15) / 16);
            bitoperand = ((type_unsigned_16)bitoff16) % 16;
            TYPE_TRACE_BLOCK_END;
        }
        else if (regbyte)
        {
            bitoperand = instruction_state.data.cr % 16;
        }
        else
        {
            if (instruction_state.data.flagMem)
                instruction_state.data.mrm.offset += 2 *
                    (TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm) / 16);
            bitoperand = (TYPE_MASK_UNSIGNED_16(instruction_state.data.cimm) % 16);
        }
        TYPE_TRACE_CHECK_RETURN(instruction_state.data.cimm = TYPE_MASK_UNSIGNED_16((1 << bitoperand)));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4:
        TYPE_TRACE_BLOCK_BEGIN("rmbyte(4)");
        if (instruction_state.data.flagMem && regbyte)
        {
            TYPE_TRACE_BLOCK_BEGIN("flagMem(1),regbyte(1)");
            bitoff32 = (type_signed_32)instruction_state.data.cr;
            if (bitoff32 >= 0)
                instruction_state.data.mrm.offset += 4 * (bitoff32 / 32);
            else
                instruction_state.data.mrm.offset += 4 * ((bitoff32 - 31) / 32);
            bitoperand = ((type_unsigned_32)bitoff32) % 32;
            TYPE_TRACE_BLOCK_END;
        }
        else if (regbyte)
        {
            bitoperand = instruction_state.data.cr % 32;
        }
        else
        {
            if (instruction_state.data.flagMem)
                instruction_state.data.mrm.offset += 4 *
                    (TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm) / 32);
            bitoperand = (TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm) % 32);
        }
        TYPE_TRACE_CHECK_RETURN(instruction_state.data.cimm = TYPE_MASK_UNSIGNED_32((1 << bitoperand)));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("rmbyte");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(rmbyte));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}

#define SHLD_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SHRD_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)

#define _kac_btcc                                                                                                                     \
    do                                                                                                                                \
    {                                                                                                                                 \
        switch (bit)                                                                                                                  \
        {                                                                                                                             \
        case 16:                                                                                                                      \
            instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_16(cdest);                                                               \
            break;                                                                                                                    \
        case 32:                                                                                                                      \
            instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_32(cdest);                                                               \
            break;                                                                                                                    \
        default:                                                                                                                      \
            TYPE_TRACE_BLOCK_BEGIN("bit");                                                                                            \
            TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));                                                                              \
            TYPE_TRACE_BLOCK_END;                                                                                                     \
            break;                                                                                                                    \
        }                                                                                                                             \
        instruction_state.data.opr2 = bitoperand;                                                                                     \
        instruction_state.data.result = instruction_state.data.opr1;                                                                  \
        TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF, TYPE_GET_BIT(instruction_state.data.opr1, instruction_state.data.opr2)); \
    } while (0)

static C_VOID _a_bscc(core_machine_cpu_execution_context *context, type_unsigned_64 csrc, type_unsigned_8 bit, type_bool forward)
{
    type_unsigned_32 temp;
    TYPE_TRACE_CALL_BEGIN("_a_bscc");
    if (forward)
        temp = 0;
    else
        temp = bit - 1;
    switch (bit)
    {
    case 16:
        TYPE_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_16(csrc);
        if (!instruction_state.data.opr1)
            _SetEFLAGS_ZF;
        else
        {
            _ClrEFLAGS_ZF;
            while (!TYPE_GET_BIT(instruction_state.data.opr1, TYPE_MASK_UNSIGNED_64(1 << temp)))
            {
                if (forward)
                    temp++;
                else
                    temp--;
            }
            instruction_state.data.result = TYPE_MASK_UNSIGNED_16(temp);
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        TYPE_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_32(csrc);
        if (!instruction_state.data.opr1)
            _SetEFLAGS_ZF;
        else
        {
            _ClrEFLAGS_ZF;
            while (!TYPE_GET_BIT(instruction_state.data.opr1, TYPE_MASK_UNSIGNED_64(1 << temp)))
            {
                if (forward)
                    temp++;
                else
                    temp--;
            }
            instruction_state.data.result = TYPE_MASK_UNSIGNED_32(temp);
        }
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_bt(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_32 bitoperand, type_unsigned_8 bit)
{
    TYPE_TRACE_CALL_BEGIN("_a_bt");
    _kac_btcc;
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_btc(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_32 bitoperand, type_unsigned_8 bit)
{
    TYPE_TRACE_CALL_BEGIN("_a_btc");
    _kac_btcc;
    TYPE_MAKE_BIT(instruction_state.data.result, instruction_state.data.opr2, !_GetEFLAGS_CF);
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_btr(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_32 bitoperand, type_unsigned_8 bit)
{
    TYPE_TRACE_CALL_BEGIN("_a_btr");
    _kac_btcc;
    TYPE_CLEAR_BIT(instruction_state.data.result, instruction_state.data.opr2);
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_bts(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_32 bitoperand, type_unsigned_8 bit)
{
    TYPE_TRACE_CALL_BEGIN("_a_bts");
    _kac_btcc;
    TYPE_SET_BIT(instruction_state.data.result, instruction_state.data.opr2);
    TYPE_TRACE_CALL_END;
}

static C_VOID _a_imul2(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_64 csrc, type_unsigned_8 bit)
{
    type_unsigned_64 temp;
    TYPE_TRACE_CALL_BEGIN("_a_imul2");
    switch (bit)
    {
    case 16:
        TYPE_TRACE_BLOCK_BEGIN("bit(16+16)");
        _new_code_path_;
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_16((type_signed_16)cdest);
        instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_16((type_signed_16)csrc);
        temp = TYPE_MASK_UNSIGNED_32((type_signed_16)instruction_state.data.opr1 * (type_signed_16)instruction_state.data.opr2);
        instruction_state.data.result = TYPE_MASK_UNSIGNED_16(temp);
        if (TYPE_MASK_UNSIGNED_32(temp) != TYPE_MASK_UNSIGNED_32((type_signed_16)instruction_state.data.result))
        {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        else
        {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    case 32:
        TYPE_TRACE_BLOCK_BEGIN("bit(32+32");
        _new_code_path_;
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_32((type_signed_32)cdest);
        instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_32((type_signed_32)csrc);
        temp = TYPE_MASK_UNSIGNED_64((type_signed_64)(type_signed_32)instruction_state.data.opr1 *
                                     (type_signed_64)(type_signed_32)instruction_state.data.opr2);
        instruction_state.data.result = TYPE_MASK_UNSIGNED_32(temp);
        if (TYPE_MASK_UNSIGNED_64(temp) != TYPE_MASK_UNSIGNED_64((type_signed_32)instruction_state.data.result))
        {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        else
        {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_BLOCK_BEGIN("bit");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        TYPE_TRACE_BLOCK_END;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_shld(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_64 csrc, type_unsigned_8 count, type_unsigned_8 bit)
{
    type_bool flagcf;
    type_bool flagbit;
    type_signed_32 i;
    TYPE_TRACE_CALL_BEGIN("_a_shld");
    count &= 0x1f;
    if (!count)
    {
        TYPE_TRACE_CALL_END;
        return;
    }
    if (count > bit)
    {
        /* bad parameters */
        /* dest is undefined */
        /* cf, of, sf, zf, af, pf are undefined */
        instruction_state.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF |
                                       VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF);
    }
    else
    {
        switch (bit)
        {
        case 16:
            TYPE_TRACE_BLOCK_BEGIN("bit(16)");
            instruction_state.data.bit = 16;
            instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_16(cdest);
            instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_16(csrc);
            instruction_state.data.result = instruction_state.data.opr1;
            flagcf = !!TYPE_GET_MSB_16(instruction_state.data.result);
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                          TYPE_GET_BIT(instruction_state.data.result, TYPE_MASK_UNSIGNED_64(1 << (bit - count))));
            for (i = (type_signed_32)(bit - 1); i >= (type_signed_32)count; --i)
            {
                flagbit = TYPE_GET_BIT(instruction_state.data.opr1, TYPE_MASK_UNSIGNED_64(1 << (i - count)));
                TYPE_MAKE_BIT(instruction_state.data.result, TYPE_MASK_UNSIGNED_64(1 << i), flagbit);
            }
            for (i = (type_signed_32)(count - 1); i >= 0; --i)
            {
                flagbit = TYPE_GET_BIT(instruction_state.data.opr2, TYPE_MASK_UNSIGNED_64(1 << (i - count + bit)));
                TYPE_MAKE_BIT(instruction_state.data.result, TYPE_MASK_UNSIGNED_64(1 << i), flagbit);
            }
            if (count == 1)
                TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                              ((!!TYPE_GET_MSB_16(instruction_state.data.result)) ^ flagcf));
            else
                instruction_state.data.udf |= VCPU_EFLAGS_OF;
            TYPE_TRACE_BLOCK_END;
            break;
        case 32:
            TYPE_TRACE_BLOCK_BEGIN("bit(32)");
            instruction_state.data.bit = 32;
            instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_32(cdest);
            instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_32(csrc);
            instruction_state.data.result = instruction_state.data.opr1;
            flagcf = !!TYPE_GET_MSB_32(instruction_state.data.result);
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                          TYPE_GET_BIT(instruction_state.data.result, TYPE_MASK_UNSIGNED_64(1 << (bit - count))));
            for (i = (type_signed_32)(bit - 1); i >= (type_signed_32)count; --i)
            {
                flagbit = TYPE_GET_BIT(instruction_state.data.opr1, TYPE_MASK_UNSIGNED_64(1 << (i - count)));
                TYPE_MAKE_BIT(instruction_state.data.result, TYPE_MASK_UNSIGNED_64(1 << i), flagbit);
            }
            for (i = (type_signed_32)(count - 1); i >= 0; --i)
            {
                flagbit = TYPE_GET_BIT(instruction_state.data.opr2, TYPE_MASK_UNSIGNED_64(1 << (i - count + bit)));
                TYPE_MAKE_BIT(instruction_state.data.result, TYPE_MASK_UNSIGNED_64(1 << i), flagbit);
            }
            if (count == 1)
                TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                              ((!!TYPE_GET_MSB_32(instruction_state.data.result)) ^ flagcf));
            else
                instruction_state.data.udf |= VCPU_EFLAGS_OF;
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_BLOCK_BEGIN("bit");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
            TYPE_TRACE_BLOCK_END;
            break;
        }
        TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHLD_FLAG));
        instruction_state.data.udf |= VCPU_EFLAGS_AF;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID _a_shrd(core_machine_cpu_execution_context *context, type_unsigned_64 cdest, type_unsigned_64 csrc, type_unsigned_8 count, type_unsigned_8 bit)
{
    type_bool flagcf;
    type_bool flagbit;
    type_signed_32 i;
    TYPE_TRACE_CALL_BEGIN("_a_shrd");
    count &= 0x1f;
    if (!count)
    {
        TYPE_TRACE_CALL_END;
        return;
    }
    if (count > bit)
    {
        /* bad parameters */
        /* dest is undefined */
        /* cf, of, sf, zf, af, pf are undefined */
        instruction_state.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF |
                                       VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF);
    }
    else
    {
        switch (bit)
        {
        case 16:
            TYPE_TRACE_BLOCK_BEGIN("bit(16)");
            instruction_state.data.bit = 16;
            instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_16(cdest);
            instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_16(csrc);
            instruction_state.data.result = instruction_state.data.opr1;
            flagcf = !!TYPE_GET_MSB_16(instruction_state.data.result);
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                          TYPE_GET_BIT(instruction_state.data.result, TYPE_MASK_UNSIGNED_64(1 << (count - 1))));
            for (i = 0; i <= (type_signed_32)(bit - count - 1); ++i)
            {
                flagbit = TYPE_GET_BIT(instruction_state.data.opr1, TYPE_MASK_UNSIGNED_64(1 << (i + count)));
                TYPE_MAKE_BIT(instruction_state.data.result, TYPE_MASK_UNSIGNED_64(1 << i), flagbit);
            }
            for (i = (type_signed_32)(bit - count); i <= (type_signed_32)(bit - 1); ++i)
            {
                flagbit = TYPE_GET_BIT(instruction_state.data.opr2, TYPE_MASK_UNSIGNED_64(1 << (i + count - bit)));
                TYPE_MAKE_BIT(instruction_state.data.result, TYPE_MASK_UNSIGNED_64(1 << i), flagbit);
            }
            if (count == 1)
                TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                              ((!!TYPE_GET_MSB_16(instruction_state.data.result)) ^ flagcf));
            else
                instruction_state.data.udf |= VCPU_EFLAGS_OF;
            TYPE_TRACE_BLOCK_END;
            break;
        case 32:
            TYPE_TRACE_BLOCK_BEGIN("bit(32)");
            instruction_state.data.bit = 32;
            instruction_state.data.opr1 = TYPE_MASK_UNSIGNED_32(cdest);
            instruction_state.data.opr2 = TYPE_MASK_UNSIGNED_32(csrc);
            instruction_state.data.result = instruction_state.data.opr1;
            flagcf = !!TYPE_GET_MSB_32(instruction_state.data.result);
            TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                          TYPE_GET_BIT(instruction_state.data.result, TYPE_MASK_UNSIGNED_64(1 << (count - 1))));
            for (i = 0; i <= (type_signed_32)(bit - count - 1); ++i)
            {
                flagbit = TYPE_GET_BIT(instruction_state.data.opr1, TYPE_MASK_UNSIGNED_64(1 << (i + count)));
                TYPE_MAKE_BIT(instruction_state.data.result, TYPE_MASK_UNSIGNED_64(1 << i), flagbit);
            }
            for (i = (type_signed_32)(bit - count); i <= (type_signed_32)(bit - 1); ++i)
            {
                flagbit = TYPE_GET_BIT(instruction_state.data.opr2, TYPE_MASK_UNSIGNED_64(1 << (i + count - bit)));
                TYPE_MAKE_BIT(instruction_state.data.result, TYPE_MASK_UNSIGNED_64(1 << i), flagbit);
            }
            if (count == 1)
                TYPE_MAKE_BIT(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                              ((!!TYPE_GET_MSB_32(instruction_state.data.result)) ^ flagcf));
            else
                instruction_state.data.udf |= VCPU_EFLAGS_OF;
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_BLOCK_BEGIN("bit");
            TYPE_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
            TYPE_TRACE_BLOCK_END;
            break;
        }
        TYPE_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHRD_FLAG));
        instruction_state.data.udf |= VCPU_EFLAGS_AF;
    }
    TYPE_TRACE_CALL_END;
}

static C_VOID _m_setcc_rm(core_machine_cpu_execution_context *context, type_bool condition)
{
    TYPE_TRACE_CALL_BEGIN("_m_setcc_rm");
    instruction_state.data.result = TYPE_MASK_UNSIGNED_8(!!condition);
    instruction_state.data.crm = instruction_state.data.result;
    TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    TYPE_TRACE_CALL_END;
}

static C_VOID INS_0F_00(core_machine_cpu_execution_context *context)
{
    type_unsigned_8 modrm;
    type_unsigned_64 descriptor;
    TYPE_TRACE_CALL_BEGIN("INS_0F_00");
    _adv;
    if (_IsProtected)
    {
        TYPE_TRACE_BLOCK_BEGIN("Protected");
        TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, TYPE_REFERENCE_OF(modrm), 1));
        switch (_GetModRM_REG(modrm))
        {
        case 0: /* SLDT_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("SLDT_RM16");
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            instruction_state.data.crm = TYPE_MASK_UNSIGNED_16(cpu_state.data.ldtr.selector);
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 1: /* STR_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("STR_RM16");
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            instruction_state.data.crm = TYPE_MASK_UNSIGNED_16(cpu_state.data.tr.selector);
            TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            TYPE_TRACE_BLOCK_END;
            break;
        case 2: /* LLDT_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("LLDT_RM16");
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            TYPE_TRACE_CHECK_RETURN(_s_load_ldtr(context, TYPE_MASK_UNSIGNED_16(instruction_state.data.crm)));
            TYPE_TRACE_BLOCK_END;
            break;
        case 3: /* LTR_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("LTR_RM16");
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            TYPE_TRACE_CHECK_RETURN(_s_load_tr(context, TYPE_MASK_UNSIGNED_16(instruction_state.data.crm)));
            TYPE_TRACE_BLOCK_END;
            break;
        case 4: /* VERR_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("VERR_RM16");
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            if (_s_check_selector(context, TYPE_MASK_UNSIGNED_16(instruction_state.data.crm)))
            {
                _ClrEFLAGS_ZF;
            }
            else
            {
                TYPE_TRACE_BLOCK_BEGIN("selector(valid)");
                TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, TYPE_MASK_UNSIGNED_16(instruction_state.data.crm), TYPE_REFERENCE_OF(descriptor)));
                if (_IsDescSys(descriptor) || !_IsDescPresent(descriptor) ||
                    (!_IsDescCodeConform(descriptor) &&
                     (_GetCPL > _GetDesc_DPL(descriptor) ||
                      _GetSelector_RPL(TYPE_MASK_UNSIGNED_16(instruction_state.data.crm)) > _GetDesc_DPL(descriptor))))
                {
                    _ClrEFLAGS_ZF;
                }
                else
                {
                    if (_IsDescData(descriptor) || _IsDescCodeReadable(descriptor))
                    {
                        _SetEFLAGS_ZF;
                    }
                    else
                    {
                        _ClrEFLAGS_ZF;
                    }
                }
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_BLOCK_END;
            break;
        case 5: /* VERW_RM16 */
            TYPE_TRACE_BLOCK_BEGIN("VERW_RM16");
            TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            if (_s_check_selector(context, TYPE_MASK_UNSIGNED_16(instruction_state.data.crm)))
            {
                _ClrEFLAGS_ZF;
            }
            else
            {
                TYPE_TRACE_BLOCK_BEGIN("selector(valid)");
                TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, TYPE_MASK_UNSIGNED_16(instruction_state.data.crm), TYPE_REFERENCE_OF(descriptor)));
                if (_IsDescSys(descriptor) || !_IsDescPresent(descriptor) ||
                    (!_IsDescCodeConform(descriptor) &&
                     (_GetCPL > _GetDesc_DPL(descriptor) ||
                      _GetSelector_RPL(TYPE_MASK_UNSIGNED_16(instruction_state.data.crm)) > _GetDesc_DPL(descriptor))))
                {
                    _ClrEFLAGS_ZF;
                }
                else
                {
                    if (_IsDescDataWritable(descriptor))
                    {
                        _SetEFLAGS_ZF;
                    }
                    else
                    {
                        _ClrEFLAGS_ZF;
                    }
                }
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_BLOCK_END;
            break;
        case 6:
            TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(6)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        case 7:
            TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
            TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            TYPE_TRACE_BLOCK_END;
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("!Protected");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_0F_01(core_machine_cpu_execution_context *context)
{
    type_unsigned_8 modrm;
    type_unsigned_16 limit;
    type_unsigned_32 base;
    TYPE_TRACE_CALL_BEGIN("INS_0F_01");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, TYPE_REFERENCE_OF(modrm), 1));
    switch (_GetModRM_REG(modrm))
    {
    case 0: /* SGDT_M32_16 */
        TYPE_TRACE_BLOCK_BEGIN("SGDT_M32_16");
        TYPE_TRACE_CHECK_RETURN(_d_modrm_table_memory(context, modrm));
        TYPE_TRACE_CHECK_RETURN(_m_write_table_pseudo_descriptor(context,
            cpu_state.data.gdtr.limit, _GetOperandSize == 2 ?
            TYPE_MASK_UNSIGNED_24(cpu_state.data.gdtr.base) :
            cpu_state.data.gdtr.base));
        TYPE_TRACE_BLOCK_END;
        break;
    case 1: /* SIDT_M32_16 */
        TYPE_TRACE_BLOCK_BEGIN("SIDT_M32_16");
        TYPE_TRACE_CHECK_RETURN(_d_modrm_table_memory(context, modrm));
        TYPE_TRACE_CHECK_RETURN(_m_write_table_pseudo_descriptor(context,
            cpu_state.data.idtr.limit, _GetOperandSize == 2 ?
            TYPE_MASK_UNSIGNED_24(cpu_state.data.idtr.base) :
            cpu_state.data.idtr.base));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2: /* LGDT_M32_16 */
        TYPE_TRACE_BLOCK_BEGIN("LGDT_M32_16");
        if (_IsProtected && (_GetEFLAGS_VM || _GetCPL))
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        TYPE_TRACE_CHECK_RETURN(_d_modrm_table_memory(context, modrm));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        limit = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        instruction_state.data.mrm.offset += 2;
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 4));
        switch (_GetOperandSize)
        {
        case 2:
            base = TYPE_MASK_UNSIGNED_24(instruction_state.data.crm);
            break;
        case 4:
            base = TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        /* STD_PRINTF("LGDT_M32_16: executed at L%08X, read base=%08X, limit=%04X\n",
                instruction_state.data.linear, base, limit); */
        TYPE_TRACE_CHECK_RETURN(_s_load_gdtr(context, base, limit, _GetOperandSize));
        TYPE_TRACE_BLOCK_END;
        break;
    case 3: /* LIDT_M32_16 */
        TYPE_TRACE_BLOCK_BEGIN("LIDT_M32_16");
        if (_IsProtected && (_GetEFLAGS_VM || _GetCPL))
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        TYPE_TRACE_CHECK_RETURN(_d_modrm_table_memory(context, modrm));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        limit = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        instruction_state.data.mrm.offset += 2;
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 4));
        switch (_GetOperandSize)
        {
        case 2:
            base = TYPE_MASK_UNSIGNED_24(instruction_state.data.crm);
            break;
        case 4:
            base = TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
            break;
        default:
            TYPE_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        /* STD_PRINTF("LIDT_M32_16: executed at L%08X, read base=%08X, limit=%04X\n",
                 instruction_state.data.linear, base, limit); */
        TYPE_TRACE_CHECK_RETURN(_s_load_idtr(context, base, limit, _GetOperandSize));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* SMSW_RM16 */
        TYPE_TRACE_BLOCK_BEGIN("SMSW_RM16");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        instruction_state.data.crm = TYPE_MASK_UNSIGNED_16(cpu_state.data.cr0);
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
        TYPE_TRACE_BLOCK_END;
        break;
    case 5:
        TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(5)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* LMSW_RM16 */
        TYPE_TRACE_BLOCK_BEGIN("LMSW_RM16");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        TYPE_TRACE_CHECK_RETURN(_s_load_cr0_msw(context, TYPE_MASK_UNSIGNED_16(instruction_state.data.crm)));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7:
        TYPE_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID LAR_R32_RM32(core_machine_cpu_execution_context *context)
{
    type_unsigned_16 selector;
    type_unsigned_64 descriptor;
    TYPE_TRACE_CALL_BEGIN("LAR_R32_RM32");
    _adv;
    if (_IsProtected)
    {
        TYPE_TRACE_BLOCK_BEGIN("Protected(1)");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        selector = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        if (_s_check_selector(context, selector))
        {
            _ClrEFLAGS_ZF;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("selector(valid)");
            TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, selector, TYPE_REFERENCE_OF(descriptor)));
            if (!_IsDescPresent(descriptor))
            {
                _ClrEFLAGS_ZF;
            }
            else if (_IsDescUser(descriptor))
            {
                if (_IsDescCodeConform(descriptor))
                    _SetEFLAGS_ZF;
                else
                {
                    if (_GetCPL > _GetDesc_DPL(descriptor) || _GetSelector_RPL(selector) > _GetDesc_DPL(descriptor))
                        _ClrEFLAGS_ZF;
                    else
                        _SetEFLAGS_ZF;
                }
            }
            else
            {
                switch (_GetDesc_Type(descriptor))
                {
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
                if (_GetEFLAGS_ZF && (_GetCPL > _GetDesc_DPL(descriptor) ||
                    _GetSelector_RPL(selector) > _GetDesc_DPL(descriptor)))
                    _ClrEFLAGS_ZF;
            }
            if (_GetEFLAGS_ZF)
            {
                TYPE_TRACE_BLOCK_BEGIN("EFLAGS_ZF");
                switch (_GetOperandSize)
                {
                case 2:
                    TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
                    descriptor = (TYPE_MASK_UNSIGNED_16(descriptor >> 32) & 0xff00);
                    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(descriptor), 2));
                    TYPE_TRACE_BLOCK_END;
                    break;
                case 4:
                    TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
                    descriptor = (TYPE_MASK_UNSIGNED_32(descriptor >> 32) & 0x00ffff00);
                    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(descriptor), 4));
                    TYPE_TRACE_BLOCK_END;
                    break;
                default:
                    TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("Protected(0)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID LSL_R32_RM32(core_machine_cpu_execution_context *context)
{
    type_unsigned_16 selector;
    type_unsigned_32 limit;
    type_unsigned_64 descriptor;
    TYPE_TRACE_CALL_BEGIN("LSL_R32_RM32");
    _adv;
    if (_IsProtected)
    {
        TYPE_TRACE_BLOCK_BEGIN("Protected(1)");
        TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        selector = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        if (_s_check_selector(context, selector))
        {
            _ClrEFLAGS_ZF;
        }
        else
        {
            TYPE_TRACE_BLOCK_BEGIN("selector(valid)");
            TYPE_TRACE_CHECK_RETURN(_s_read_xdt(context, selector, TYPE_REFERENCE_OF(descriptor)));
            if (!_IsDescPresent(descriptor))
            {
                _ClrEFLAGS_ZF;
            }
            else if (_IsDescUser(descriptor))
            {
                if (_IsDescCodeConform(descriptor))
                    _SetEFLAGS_ZF;
                else
                {
                    if (_GetCPL > _GetDesc_DPL(descriptor) || _GetSelector_RPL(selector) > _GetDesc_DPL(descriptor))
                        _ClrEFLAGS_ZF;
                    else
                        _SetEFLAGS_ZF;
                }
            }
            else
            {
                switch (_GetDesc_Type(descriptor))
                {
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
                if (_GetEFLAGS_ZF && (_GetCPL > _GetDesc_DPL(descriptor) ||
                    _GetSelector_RPL(selector) > _GetDesc_DPL(descriptor)))
                    _ClrEFLAGS_ZF;
            }
            if (_GetEFLAGS_ZF)
            {
                TYPE_TRACE_BLOCK_BEGIN("EFLAGS_ZF");
                limit = _IsDescSegGranularLarge(descriptor) ? ((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) : _GetDescSeg_Limit(descriptor);
                if (context->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) {
                    instruction_state.data.source_lsl_granularity_valid = TYPE_TRUE;
                    instruction_state.data.source_lsl_page_granular =
                        _IsDescSegGranularLarge(descriptor);
                }
                switch (_GetOperandSize)
                {
                case 2:
                    TYPE_TRACE_BLOCK_BEGIN("OperandSize(2)");
                    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(limit), 2));
                    TYPE_TRACE_BLOCK_END;
                    break;
                case 4:
                    TYPE_TRACE_BLOCK_BEGIN("OperandSize(4)");
                    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(limit), 4));
                    TYPE_TRACE_BLOCK_END;
                    break;
                default:
                    TYPE_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                TYPE_TRACE_BLOCK_END;
            }
            TYPE_TRACE_BLOCK_END;
        }
        TYPE_TRACE_BLOCK_END;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("Protected(0)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID CLTS(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("CLTS");
    _new_code_path_;
    _adv;
    if (!_GetCR0_PE)
    {
        _ClrCR0_TS;
    }
    else
    {
        TYPE_TRACE_BLOCK_BEGIN("CR0_PE(1)");
        if (_GetCPL > 0)
            TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        else
            _ClrCR0_TS;
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
_______todo WBINVD(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("WBINVD");
    UndefinedOpcode(context);
    instruction_state.data.flagIgnore = TYPE_TRUE;
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_R32_CR(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_R32_CR");
    _adv;
    if (_GetCPL)
    {
        TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm_creg(context));
    if (instruction_state.data.rr != (type_virtual_address)&cpu_state.data.cr0 &&
        instruction_state.data.rr != (type_virtual_address)&cpu_state.data.cr2 &&
        instruction_state.data.rr != (type_virtual_address)&cpu_state.data.cr3)
    {
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
    }
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rrm, TYPE_REFERENCE_OF(instruction_state.data.cr), 4));
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_R32_DR(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_R32_DR");
    _new_code_path_;
    _adv;
    if (_GetCPL)
    {
        TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm_dreg(context));
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rrm, TYPE_REFERENCE_OF(instruction_state.data.cr), 4));
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_CR_R32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_CR_R32");
    _adv;
    if (_GetCPL)
    {
        TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm_creg(context));
    if (instruction_state.data.rr == (type_virtual_address)&cpu_state.data.cr0)
    {
        TYPE_TRACE_CHECK_RETURN(_s_write_cr0_80386(context,
            instruction_state.data.crm));
    }
    else if (instruction_state.data.rr == (type_virtual_address)&cpu_state.data.cr2)
    {
        cpu_state.data.cr2 = instruction_state.data.crm;
    }
    else if (instruction_state.data.rr == (type_virtual_address)&cpu_state.data.cr3)
    {
        TYPE_TRACE_CHECK_RETURN(_s_write_cr3_80386(context,
            instruction_state.data.crm));
    }
    else
    {
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
    }
    /* if (instruction_state.data.rr == (type_virtual_address)&cpu_state.data.cr0) {
        STD_PRINTF("MOV_CR_R32: executed at L%08X, CR0=%08X\n", instruction_state.data.linear, cpu_state.data.cr0);
    }
    if (instruction_state.data.rr == (type_virtual_address)&cpu_state.data.cr2) {
        STD_PRINTF("MOV_CR_R32: executed at L%08X, CR2=%08X\n", instruction_state.data.linear, cpu_state.data.cr2);
    }
    if (instruction_state.data.rr == (type_virtual_address)&cpu_state.data.cr3) {
        STD_PRINTF("MOV_CR_R32: executed at L%08X, CR3=%08X\n", instruction_state.data.linear, cpu_state.data.cr3);
    } */
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_DR_R32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_DR_R32");
    _new_code_path_;
    _adv;
    if (_GetCPL)
    {
        TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm_dreg(context));
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.crm), 4));
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_R32_TR(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_R32_TR");
    _new_code_path_;
    _adv;
    if (_GetCPL)
    {
        TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm_treg(context));
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rrm, TYPE_REFERENCE_OF(instruction_state.data.cr), 4));
    TYPE_TRACE_CALL_END;
}
static C_VOID MOV_TR_R32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOV_TR_R32");
    _new_code_path_;
    _adv;
    if (_GetCPL)
    {
        TYPE_TRACE_BLOCK_BEGIN("CPL(!0)");
        TYPE_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CHECK_RETURN(_d_modrm_treg(context));
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.crm), 4));
    TYPE_TRACE_CALL_END;
}
_______todo WRMSR(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("WRMSR");
    UndefinedOpcode(context);
    instruction_state.data.flagIgnore = TYPE_TRUE;
    TYPE_TRACE_CALL_END;
}
_______todo RDMSR(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("RDMSR");
    UndefinedOpcode(context);
    instruction_state.data.flagIgnore = TYPE_TRUE;
    TYPE_TRACE_CALL_END;
}
static C_VOID JO_REL32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JO_REL32");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_OF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JNO_REL32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JNO_REL32");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_OF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JC_REL32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JC_REL32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_CF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JNC_REL32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JNC_REL32");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_CF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JZ_REL32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JZ_REL32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_ZF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JNZ_REL32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JNZ_REL32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_ZF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JNA_REL32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JNA_REL32");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize,
                                   (_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    TYPE_TRACE_CALL_END;
}
static C_VOID JA_REL32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JA_REL32");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize,
                                   !(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    TYPE_TRACE_CALL_END;
}
static C_VOID JS_REL32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JS_REL32");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_SF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JNS_REL32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JNS_REL32");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_SF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JP_REL32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JP_REL32");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_PF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JNP_REL32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JNP_REL32");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_PF));
    TYPE_TRACE_CALL_END;
}
static C_VOID JL_REL32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JL_REL32");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, (_GetEFLAGS_SF != _GetEFLAGS_OF)));
    TYPE_TRACE_CALL_END;
}
static C_VOID JNL_REL32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JNL_REL32");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize, (_GetEFLAGS_SF == _GetEFLAGS_OF)));
    TYPE_TRACE_CALL_END;
}
static C_VOID JNG_REL32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JNG_REL32");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize,
                                   (_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF))));
    TYPE_TRACE_CALL_END;
}
static C_VOID JG_REL32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("JG_REL32");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_e_jcc(context, TYPE_MASK_UNSIGNED_32(instruction_state.data.cimm), _GetOperandSize,
                                   (!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF))));
    TYPE_TRACE_CALL_END;
}
static C_VOID SETO_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SETO_RM8");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_OF));
    TYPE_TRACE_CALL_END;
}
static C_VOID SETNO_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SETO_RM8");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, !_GetEFLAGS_OF));
    TYPE_TRACE_CALL_END;
}
static C_VOID SETC_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SETC_RM8");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_CF));
    TYPE_TRACE_CALL_END;
}
static C_VOID SETNC_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SETNC_RM8");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, !_GetEFLAGS_CF));
    TYPE_TRACE_CALL_END;
}
static C_VOID SETZ_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SETZ_RM8");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_ZF));
    TYPE_TRACE_CALL_END;
}
static C_VOID SETNZ_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SETNZ_RM8");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, !_GetEFLAGS_ZF));
    TYPE_TRACE_CALL_END;
}
static C_VOID SETNA_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SETNA_RM8");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_CF || _GetEFLAGS_ZF));
    TYPE_TRACE_CALL_END;
}
static C_VOID SETA_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SETA_RM8");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, !(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    TYPE_TRACE_CALL_END;
}
static C_VOID SETS_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SETS_RM8");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_SF));
    TYPE_TRACE_CALL_END;
}
static C_VOID SETNS_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SETNS_RM8");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, !_GetEFLAGS_SF));
    TYPE_TRACE_CALL_END;
}
static C_VOID SETP_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SETP_RM8");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_PF));
    TYPE_TRACE_CALL_END;
}
static C_VOID SETNP_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SETNP_RM8");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, !_GetEFLAGS_PF));
    TYPE_TRACE_CALL_END;
}
static C_VOID SETL_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SETL_RM8");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_SF != _GetEFLAGS_OF));
    TYPE_TRACE_CALL_END;
}
static C_VOID SETNL_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SETNL_RM8");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_SF == _GetEFLAGS_OF));
    TYPE_TRACE_CALL_END;
}
static C_VOID SETNG_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SETNG_RM8");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF)));
    TYPE_TRACE_CALL_END;
}
static C_VOID SETG_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SETG_RM8");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    TYPE_TRACE_CHECK_RETURN(_m_setcc_rm(context, !_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF)));
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_FS(core_machine_cpu_execution_context *context)
{
    type_unsigned_32 xs_sel;
    TYPE_TRACE_CALL_BEGIN("PUSH_FS");
    _adv;
    xs_sel = cpu_state.data.fs.selector;
    TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_FS(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("POP_FS");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_e_pop_sreg(context, &cpu_state.data.fs,
        _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
_______todo CPUID(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("CPUID");
    UndefinedOpcode(context);
    instruction_state.data.flagIgnore = TYPE_TRUE;
    TYPE_TRACE_CALL_END;
}
static C_VOID BT_RM32_R32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("BT_RM32_R32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_bit_rmimm(context, _GetOperandSize, _GetOperandSize, 0));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_a_bt(context, (type_unsigned_32)instruction_state.data.crm, (type_unsigned_32)instruction_state.data.cimm, _GetOperandSize * 8));
    TYPE_TRACE_CALL_END;
}
static C_VOID SHLD_RM32_R32_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SHLD_RM32_R32_I8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_shld(context, instruction_state.data.crm, instruction_state.data.cr, TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), _GetOperandSize * 8));
    if (TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm) & 0x1f)
    {
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID SHLD_RM32_R32_CL(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SHLD_RM32_R32_CL");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_a_shld(context, instruction_state.data.crm, instruction_state.data.cr, cpu_state.data.cl, _GetOperandSize * 8));
    if (cpu_state.data.cl & 0x1f)
    {
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID PUSH_GS(core_machine_cpu_execution_context *context)
{
    type_unsigned_32 xs_sel;
    TYPE_TRACE_CALL_BEGIN("PUSH_GS");
    _adv;
    xs_sel = cpu_state.data.gs.selector;
    TYPE_TRACE_CHECK_RETURN(_e_push(context, TYPE_REFERENCE_OF(xs_sel), _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID POP_GS(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("POP_GS");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_e_pop_sreg(context, &cpu_state.data.gs,
        _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
_______todo RSM(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("RSM");
    UndefinedOpcode(context);
    instruction_state.data.flagIgnore = TYPE_TRUE;
    TYPE_TRACE_CALL_END;
}
static C_VOID BTS_RM32_R32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("BTS_RM32_R32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_bit_rmimm(context, _GetOperandSize, _GetOperandSize, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_a_bts(context, (type_unsigned_32)instruction_state.data.crm, (type_unsigned_32)instruction_state.data.cimm, _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID SHRD_RM32_R32_I8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SHRD_RM32_R32_I8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_d_imm(context, 1));
    TYPE_TRACE_CHECK_RETURN(_a_shrd(context, instruction_state.data.crm, instruction_state.data.cr, TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm), _GetOperandSize * 8));
    if (TYPE_MASK_UNSIGNED_8(instruction_state.data.cimm) & 0x1f)
    {
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID SHRD_RM32_R32_CL(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("SHRD_RM32_R32_CL");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_a_shrd(context, instruction_state.data.crm, instruction_state.data.cr, cpu_state.data.cl, _GetOperandSize * 8));
    if (cpu_state.data.cl & 0x1f)
    {
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID IMUL_R32_RM32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("IMUL_R32_RM32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_a_imul2(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID LSS_R32_M16_32(core_machine_cpu_execution_context *context)
{
    type_unsigned_16 selector;
    type_unsigned_32 offset;
    TYPE_TRACE_CALL_BEGIN("LSS_R32_M16_32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm_ea(context, _GetOperandSize,
        _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        offset = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        break;
    case 4:
        offset = TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    instruction_state.data.mrm.offset += _GetOperandSize;
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
    selector = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
    TYPE_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.ss, instruction_state.data.rr, selector, offset, _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID BTR_RM32_R32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("BTR_RM32_R32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_bit_rmimm(context, _GetOperandSize, _GetOperandSize, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_a_btr(context, (type_unsigned_32)instruction_state.data.crm, (type_unsigned_32)instruction_state.data.cimm, _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID LFS_R32_M16_32(core_machine_cpu_execution_context *context)
{
    type_unsigned_16 selector;
    type_unsigned_32 offset;
    TYPE_TRACE_CALL_BEGIN("LFS_R32_M16_32");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm_ea(context, _GetOperandSize,
        _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        offset = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        break;
    case 4:
        offset = TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    instruction_state.data.mrm.offset += _GetOperandSize;
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
    selector = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
    TYPE_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.fs, instruction_state.data.rr, selector, offset, _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID LGS_R32_M16_32(core_machine_cpu_execution_context *context)
{
    type_unsigned_16 selector;
    type_unsigned_32 offset;
    TYPE_TRACE_CALL_BEGIN("LGS_R32_M16_32");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm_ea(context, _GetOperandSize,
        _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    switch (_GetOperandSize)
    {
    case 2:
        offset = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
        break;
    case 4:
        offset = TYPE_MASK_UNSIGNED_32(instruction_state.data.crm);
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    instruction_state.data.mrm.offset += _GetOperandSize;
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
    selector = TYPE_MASK_UNSIGNED_16(instruction_state.data.crm);
    TYPE_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.gs, instruction_state.data.rr, selector, offset, _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID MOVZX_R32_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOVZX_R32_RM8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    instruction_state.data.crm = (type_unsigned_8)instruction_state.data.crm;
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID MOVZX_R32_RM16(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOVZX_R32_RM16");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, 2));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
    instruction_state.data.crm = (type_unsigned_16)instruction_state.data.crm;
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID INS_0F_BA(core_machine_cpu_execution_context *context)
{
    type_bool write = 0;
    type_unsigned_8 modrm = 0x00;
    TYPE_TRACE_CALL_BEGIN("INS_0F_BA");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, TYPE_REFERENCE_OF(modrm), 1));
    if (_GetModRM_REG(modrm) == 4)
        write = 0;
    else
        write = 1;
    TYPE_TRACE_CHECK_RETURN(_d_bit_rmimm(context, 0, _GetOperandSize, write));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    switch (instruction_state.data.cr)
    {
    case 0:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(0)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 1:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 2:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 3:
        TYPE_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
        TYPE_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        TYPE_TRACE_BLOCK_END;
        break;
    case 4: /* BT_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("BT_RM32_I8");
        TYPE_TRACE_CHECK_RETURN(_a_bt(context, (type_unsigned_32)instruction_state.data.crm, (type_unsigned_32)instruction_state.data.cimm, _GetOperandSize * 8));
        TYPE_TRACE_BLOCK_END;
        break;
    case 5: /* BTS_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("BTS_RM32_I8");
        TYPE_TRACE_CHECK_RETURN(_a_bts(context, (type_unsigned_32)instruction_state.data.crm, (type_unsigned_32)instruction_state.data.cimm, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
        TYPE_TRACE_BLOCK_END;
        break;
    case 6: /* BTR_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("BTR_RM32_I8");
        TYPE_TRACE_CHECK_RETURN(_a_btr(context, (type_unsigned_32)instruction_state.data.crm, (type_unsigned_32)instruction_state.data.cimm, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
        TYPE_TRACE_BLOCK_END;
        break;
    case 7: /* BTC_RM32_I8 */
        TYPE_TRACE_BLOCK_BEGIN("BTC_RM32_I8");
        TYPE_TRACE_CHECK_RETURN(_a_btc(context, (type_unsigned_32)instruction_state.data.crm, (type_unsigned_32)instruction_state.data.cimm, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
        TYPE_TRACE_BLOCK_END;
        break;
    default:
        TYPE_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID BTC_RM32_R32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("BTC_RM32_R32");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_bit_rmimm(context, _GetOperandSize, _GetOperandSize, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_a_btc(context, (type_unsigned_32)instruction_state.data.crm, (type_unsigned_32)instruction_state.data.cimm, _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    TYPE_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID BSF_R32_RM32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("BSF_R32_RM32");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_a_bscc(context, instruction_state.data.crm, _GetOperandSize * 8, 1));
    if (!_GetEFLAGS_ZF)
    {
        TYPE_TRACE_BLOCK_BEGIN("EFLAGS_ZF(0)");
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID BSR_R32_RM32(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("BSR_R32_RM32");
    _new_code_path_;
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    TYPE_TRACE_CHECK_RETURN(_a_bscc(context, instruction_state.data.crm, _GetOperandSize * 8, 0));
    if (!_GetEFLAGS_ZF)
    {
        TYPE_TRACE_BLOCK_BEGIN("EFLAGS_ZF(0)");
        TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
        TYPE_TRACE_BLOCK_END;
    }
    TYPE_TRACE_CALL_END;
}
static C_VOID MOVSX_R32_RM8(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOVSX_R32_RM8");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, 1));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    instruction_state.data.crm = (type_signed_8)instruction_state.data.crm;
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
    TYPE_TRACE_CALL_END;
}
static C_VOID MOVSX_R32_RM16(core_machine_cpu_execution_context *context)
{
    TYPE_TRACE_CALL_BEGIN("MOVSX_R32_RM16");
    _adv;
    TYPE_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, 2));
    TYPE_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
    instruction_state.data.crm = (type_signed_16)instruction_state.data.crm;
    TYPE_TRACE_CHECK_RETURN(_m_write_ref(context, instruction_state.data.rr, TYPE_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
    TYPE_TRACE_CALL_END;
}

static C_VOID ExecFinal(core_machine_cpu_execution_context *context);

static C_VOID ExecInit(core_machine_cpu_execution_context *context)
{
    instruction_state.data.flagIgnore = TYPE_FALSE;
    instruction_state.data.msize = 0;
    instruction_state.data.reccs = cpu_state.data.cs.selector;
    instruction_state.data.receip = cpu_state.data.eip;
    instruction_state.data.linear = cpu_state.data.cs.base + cpu_state.data.eip;
    instruction_state.data.oldcpu = cpu_state;
    instruction_state.data.except = TYPE_ZERO_32;
    instruction_state.data.excode = TYPE_ZERO_32;
    if (context->prefetch_expected_valid && instruction_state.data.linear !=
        context->prefetch_expected_linear) core_machine_cpu_execution_invalidate_prefetch(context);
    if (!context->prefetch_valid || instruction_state.data.linear <
        context->prefetch_linear || instruction_state.data.linear -
        context->prefetch_linear >= context->prefetch_count) {
        type_unsigned_8 prefetch_bytes = context->prefetch_capacity;

        core_machine_cpu_execution_invalidate_prefetch(context);
        if (cpu_state.data.eip <= cpu_state.data.cs.limit &&
            (type_unsigned_64)cpu_state.data.cs.limit - cpu_state.data.eip + 1u <
                prefetch_bytes) {
            prefetch_bytes = (type_unsigned_8)((type_unsigned_64)
                cpu_state.data.cs.limit - cpu_state.data.eip + 1u);
        }
        context->memory_access_provenance = CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH;
        _kma_read_logical(context, &cpu_state.data.cs, cpu_state.data.eip,
            (type_virtual_address)context->prefetch_bytes, prefetch_bytes,
            _GetCPL, 1);
        context->memory_access_provenance = CORE_MACHINE_CPU_MEMORY_ACCESS_DATA;
        if (!instruction_state.data.except) {
            context->prefetch_linear = instruction_state.data.linear;
            context->prefetch_count = prefetch_bytes;
            context->prefetch_valid = TYPE_TRUE;
        }
    }
    if (context->prefetch_valid && instruction_state.data.linear >=
        context->prefetch_linear && instruction_state.data.linear -
        context->prefetch_linear < context->prefetch_count) {
        type_unsigned_8 available = context->prefetch_count -
            (type_unsigned_8)(instruction_state.data.linear - context->prefetch_linear);
        STD_MEMSET(instruction_state.data.opcodes, 0, 15u);
        STD_MEMCPY(instruction_state.data.opcodes, context->prefetch_bytes +
            (instruction_state.data.linear - context->prefetch_linear), available);
    }
    if (instruction_state.data.except) {
        instruction_state.data.oplen = 0;
    }
    else
    {
        instruction_state.data.oplen = 15;
    }

    instruction_state.data.flagLock = TYPE_FALSE;
    instruction_state.data.roverds = &cpu_state.data.ds;
    instruction_state.data.roverss = &cpu_state.data.ss;
    instruction_state.data.prefix_rep = PREFIX_REP_NONE;
    instruction_state.data.prefix_oprsize = TYPE_FALSE;
    instruction_state.data.prefix_addrsize = TYPE_FALSE;
    instruction_state.data.flagMem = TYPE_FALSE;
    instruction_state.data.source_lsl_granularity_valid = TYPE_FALSE;
    instruction_state.data.source_lsl_page_granular = TYPE_FALSE;
    instruction_state.data.flagInsLoop = TYPE_FALSE;
    instruction_state.data.flagMaskInt = TYPE_FALSE;
    instruction_state.data.bit = 0;
    instruction_state.data.opr1 = 0;
    instruction_state.data.opr2 = 0;
    instruction_state.data.result = 0;
    instruction_state.data.udf = TYPE_ZERO_32;
    instruction_state.data.mrm.rsreg = STD_NULL;
    instruction_state.data.mrm.offset = TYPE_ZERO_32;
    context->debug_tf_before = _GetEFLAGS_TF;
    context->debug_rf_before = _GetEFLAGS_RF;
#if VCPUINS_TRACE == 1
    if (context->trace != STD_NULL)
        type_trace_initialize(context->trace);
#endif
    if (context->diagnostic_provider != STD_NULL &&
        context->diagnostic_provider->record_instruction != STD_NULL)
    {
        context->diagnostic_provider->record_instruction(context->diagnostic_context,
                                                         &cpu_state, &instruction_state);
    }
}

type_bool core_machine_cpu_execution_preview_lexeme(
    const core_machine_cpu_execution_context *context,
    core_machine_cpu_instruction_lexeme *out_lexeme)
{
    core_machine_cpu_execution_context preview;
    t_cpu preview_cpu;
    t_cpuins preview_instructions;

    if (out_lexeme == STD_NULL) return TYPE_FALSE;
    *out_lexeme = (core_machine_cpu_instruction_lexeme) { 0u, 0u, TYPE_FALSE };
    if (context == STD_NULL || context->cpu == STD_NULL ||
        context->instructions == STD_NULL || context->memory == STD_NULL) {
        return TYPE_FALSE;
    }
    preview_cpu = *context->cpu;
    preview_instructions = *context->instructions;
    preview = *context;
    preview.cpu = &preview_cpu;
    preview.instructions = &preview_instructions;
    preview.transaction = STD_NULL;
    preview.trace = STD_NULL;
    preview.diagnostic_provider = STD_NULL;
    preview.diagnostic_context = STD_NULL;
    preview.preview_mode = TYPE_TRUE;
    ExecInit(&preview);
    if (preview_instructions.data.except) return TYPE_FALSE;
    return core_machine_cpu_instruction_lexeme_scan_with_options(
        preview_instructions.data.opcodes,
        (type_unsigned_8)sizeof(preview_instructions.data.opcodes),
        preview.cpu_profile, preview_cpu.data.cs.seg.exec.defsize,
        preview.cpu_80386_cr_mov_ignores_mod, out_lexeme);
}

static type_unsigned_32 _debug_breakpoint_address(type_unsigned_8 index,
    const t_cpu *cpu)
{
    switch (index) {
    case 0u: return cpu->data.dr0;
    case 1u: return cpu->data.dr1;
    case 2u: return cpu->data.dr2;
    case 3u: return cpu->data.dr3;
    default: return TYPE_ZERO_32;
    }
}

static type_bool _debug_breakpoint_enabled(type_unsigned_8 index,
    const t_cpu *cpu)
{
    type_unsigned_32 enable_mask = ((type_unsigned_32)1u << (index * 2u)) |
        ((type_unsigned_32)1u << (index * 2u + 1u));

    return (cpu->data.dr7 & enable_mask) != TYPE_ZERO_32;
}

static type_unsigned_8 _debug_breakpoint_length(type_unsigned_8 length)
{
    switch (length) {
    case 0u: return 1u;
    case 1u: return 2u;
    case 3u: return 4u;
    default: return 0u;
    }
}

static type_unsigned_32 _debug_match_instruction_breakpoint(
    core_machine_cpu_execution_context *context)
{
    type_unsigned_32 enabled = TYPE_ZERO_32;
    type_unsigned_8 index;

    if (context->cpu_profile < CORE_MACHINE_CPU_PROFILE_80386 ||
        context->debug_rf_before) return TYPE_ZERO_32;
    for (index = 0u; index < 4u; ++index) {
        type_unsigned_32 control = (cpu_state.data.dr7 >> (16u + index * 4u)) &
            0x0fu;

        if ((control & 3u) != 0u || (control >> 2u) != 0u ||
            _debug_breakpoint_address(index, &cpu_state) !=
                instruction_state.data.linear) continue;
        if (_debug_breakpoint_enabled(index, &cpu_state)) {
            enabled |= (type_unsigned_32)1u << index;
        }
    }
    return enabled;
}

static type_unsigned_32 _debug_match_data_breakpoint(
    core_machine_cpu_execution_context *context)
{
    type_unsigned_32 enabled = TYPE_ZERO_32;
    type_unsigned_8 index;
    type_unsigned_16 access_index;

    if (context->cpu_profile < CORE_MACHINE_CPU_PROFILE_80386) return TYPE_ZERO_32;
    for (access_index = 0u; access_index < instruction_state.data.msize;
        ++access_index) {
        const t_cpuins_data_memory *access =
            &instruction_state.data.mem[access_index];

        for (index = 0u; index < 4u; ++index) {
            type_unsigned_32 control =
                (cpu_state.data.dr7 >> (16u + index * 4u)) & 0x0fu;
            type_unsigned_8 rw = control & 3u;
            type_unsigned_8 length = _debug_breakpoint_length(control >> 2u);
            type_unsigned_64 address;
            type_unsigned_64 first;
            type_unsigned_64 last;

            if (access->byte == 0u || rw == 0u || rw == 2u ||
                (rw == 1u && !access->flagWrite) ||
                length == 0u) continue;
            address = _debug_breakpoint_address(index, &cpu_state) &
                ~(type_unsigned_32)(length - 1u);
            first = access->linear;
            last = first + access->byte - 1u;
            if (last < address || first >= address + length) continue;
            if (_debug_breakpoint_enabled(index, &cpu_state)) {
                enabled |= (type_unsigned_32)1u << index;
            }
        }
    }
    return enabled;
}

static C_VOID _debug_schedule_trap(core_machine_cpu_execution_context *context,
    type_unsigned_32 cause)
{
    if (cause == TYPE_ZERO_32) return;
    cpu_state.data.dr6 |= cause;
    context->debug_trap_pending = TYPE_TRUE;
    context->debug_trap_cause |= cause;
}

static C_VOID _debug_complete_instruction(
    core_machine_cpu_execution_context *context, type_unsigned_8 opcode)
{
    type_unsigned_32 cause;

    if (instruction_state.data.except) return;
    cause = _debug_match_data_breakpoint(context);
    /* An interrupt gate can clear TF while completing the instruction.  A
     * single-step trap is pending only when tracing remained enabled at the
     * architectural completion boundary. */
    if (context->debug_tf_before && _GetEFLAGS_TF) cause |= VCPU_DR6_BS;
    _debug_schedule_trap(context, cause);
    if (context->debug_rf_before && opcode != 0xcfu) _ClrEFLAGS_RF;
}

static C_VOID _debug_deliver_trap(core_machine_cpu_execution_context *context)
{
    t_cpu trap_cpu;
    type_unsigned_32 cause;

    if (!context->debug_trap_pending) return;
    cause = context->debug_trap_cause;
    context->debug_trap_pending = TYPE_FALSE;
    context->debug_trap_cause = TYPE_ZERO_32;
    trap_cpu = cpu_state;
    ExecInit(context);
    _e_intr_n(context, 0x01u, _GetOperandSize, TYPE_TRUE);
    if (!instruction_state.data.except && context->diagnostic_provider !=
        STD_NULL && context->diagnostic_provider->record_delivered_exception !=
        STD_NULL) {
        instruction_state.data.except = VCPUINS_EXCEPT_DB;
        instruction_state.data.excode = cause;
        context->diagnostic_provider->record_delivered_exception(
            context->diagnostic_context, &trap_cpu, &instruction_state);
        instruction_state.data.except = TYPE_ZERO_32;
        instruction_state.data.excode = TYPE_ZERO_32;
    }
    ExecFinal(context);
}
static type_bool _e_is_contributory_exception(type_unsigned_32 exception)
{
    return exception == VCPUINS_EXCEPT_TS || exception == VCPUINS_EXCEPT_NP ||
        exception == VCPUINS_EXCEPT_SS || exception == VCPUINS_EXCEPT_GP;
}
/* Real-mode final delivery has one rollback and diagnostic boundary.  The
 * vector remains an architectural property of the producer; this helper owns
 * only the common fault-state restoration around IVT delivery. */
static type_bool _e_final_deliver_real_exception(
    core_machine_cpu_execution_context *context, const t_cpu *fault_cpu,
    type_unsigned_8 exception_vector)
{
    type_unsigned_32 original_except;
    type_unsigned_32 original_excode;

    TYPE_TRACE_CALL_BEGIN("_e_final_deliver_real_exception");
    original_except = instruction_state.data.except;
    original_excode = instruction_state.data.excode;
    cpu_state = *fault_cpu;
    _e_except_n(context, exception_vector, _GetOperandSize);
    if (instruction_state.data.except) {
        cpu_state = *fault_cpu;
        instruction_state.data.except = original_except;
        instruction_state.data.excode = original_excode;
        TYPE_TRACE_CALL_END;
        return TYPE_FALSE;
    }
    if (context->diagnostic_provider != STD_NULL &&
        context->diagnostic_provider->record_delivered_exception != STD_NULL) {
        instruction_state.data.except = original_except;
        instruction_state.data.excode = original_excode;
        context->diagnostic_provider->record_delivered_exception(
            context->diagnostic_context, fault_cpu, &instruction_state);
        instruction_state.data.except = 0u;
    }
    TYPE_TRACE_CALL_END;
    return TYPE_TRUE;
}

static C_VOID _e_mark_instruction_fault_delivered(
    core_machine_cpu_execution_context *context)
{
    if (context != STD_NULL && context->instruction_in_progress) {
        context->instruction_fault_delivered = TYPE_TRUE;
    }
}

static C_VOID ExecFinal(core_machine_cpu_execution_context *context)
{
    t_cpu fault_cpu;
    type_unsigned_32 original_except;
    type_unsigned_32 original_excode;
    type_unsigned_8 exception_vector;
    type_bool exception_deliverable;
    if (instruction_state.data.flagInsLoop)
    {
        cpu_state.data.cs = instruction_state.data.oldcpu.data.cs;
        cpu_state.data.eip = instruction_state.data.oldcpu.data.eip;
    }
#if VCPUINS_TRACE == 1
    if (context->trace != STD_NULL && context->trace->callCount &&
        !instruction_state.data.except)
        _SetExcept_CE(0);
    if (context->trace != STD_NULL)
        type_trace_finalize(context->trace);
#endif
    if (instruction_state.data.except)
    {
        fault_cpu = instruction_state.data.oldcpu;
        if (instruction_state.data.except == VCPUINS_EXCEPT_DB) {
            fault_cpu.data.dr6 = cpu_state.data.dr6;
            fault_cpu.data.eflags |= VCPU_EFLAGS_RF;
        }
        if (TYPE_GET_BIT(instruction_state.data.except, VCPUINS_EXCEPT_PF))
        {
            fault_cpu.data.cr2 = cpu_state.data.cr2;
        }
        exception_vector = 0u;
        exception_deliverable = TYPE_FALSE;
        if (instruction_state.data.except == VCPUINS_EXCEPT_DE) {
            exception_vector = 0x00u;
            exception_deliverable = TYPE_TRUE;
        }
        else if (instruction_state.data.except == VCPUINS_EXCEPT_DB) {
            exception_vector = 0x01u;
            exception_deliverable = TYPE_TRUE;
        }
        else if (instruction_state.data.except == VCPUINS_EXCEPT_GP) {
            exception_vector = 0x0du;
            exception_deliverable = TYPE_TRUE;
        }
        else if (instruction_state.data.except == VCPUINS_EXCEPT_UD) {
            exception_vector = 0x06u;
            exception_deliverable = TYPE_TRUE;
        }
        else if (instruction_state.data.except == VCPUINS_EXCEPT_NM) {
            exception_vector = 0x07u;
            exception_deliverable = TYPE_TRUE;
        }
        else if (instruction_state.data.except == VCPUINS_EXCEPT_BR) {
            exception_vector = 0x05u;
            exception_deliverable = TYPE_TRUE;
        }
        else if (instruction_state.data.except == VCPUINS_EXCEPT_NP) {
            exception_vector = 0x0bu;
            exception_deliverable = TYPE_TRUE;
        }
        else if (instruction_state.data.except == VCPUINS_EXCEPT_SS &&
            context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80286) {
            exception_vector = 0x0cu;
            exception_deliverable = TYPE_TRUE;
        }
        else if (instruction_state.data.except == VCPUINS_EXCEPT_TS &&
            context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80286) {
            exception_vector = 0x0au;
            exception_deliverable = TYPE_TRUE;
        }
        else if (instruction_state.data.except == VCPUINS_EXCEPT_PF) {
            exception_vector = 0x0eu;
            exception_deliverable = TYPE_TRUE;
        }
        else if (instruction_state.data.except == VCPUINS_EXCEPT_MF) {
            exception_vector = 0x10u;
            exception_deliverable = TYPE_TRUE;
        }
        if (TYPE_GET_BIT(fault_cpu.data.cr0, VCPU_CR0_PE) &&
            exception_deliverable) {
            original_except = instruction_state.data.except;
            original_excode = instruction_state.data.excode;
            cpu_state = fault_cpu;
            _e_except_n(context, exception_vector, _GetOperandSize);
            if (!instruction_state.data.except) {
                if (context->diagnostic_provider != STD_NULL &&
                    context->diagnostic_provider->record_delivered_exception !=
                        STD_NULL) {
                    instruction_state.data.except = original_except;
                    instruction_state.data.excode = original_excode;
                    context->diagnostic_provider->record_delivered_exception(
                        context->diagnostic_context, &fault_cpu,
                        &instruction_state);
                    instruction_state.data.except = 0u;
                }
                if (original_except == VCPUINS_EXCEPT_DB) _ClrEFLAGS_RF;
                _e_mark_instruction_fault_delivered(context);
                return;
            }
            cpu_state = fault_cpu;
            if (context->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386 &&
                _e_is_contributory_exception(original_except) &&
                _e_is_contributory_exception(instruction_state.data.except)) {
                instruction_state.data.except = VCPUINS_EXCEPT_DF;
                instruction_state.data.excode = 0u;
                _e_except_n(context, 0x08u, _GetOperandSize);
                if (!instruction_state.data.except) {
                    if (context->diagnostic_provider != STD_NULL &&
                        context->diagnostic_provider->record_delivered_exception !=
                            STD_NULL) {
                        instruction_state.data.except = VCPUINS_EXCEPT_DF;
                        instruction_state.data.excode = 0u;
                        context->diagnostic_provider->record_delivered_exception(
                            context->diagnostic_context, &fault_cpu,
                            &instruction_state);
                        instruction_state.data.except = 0u;
                    }
                    _e_mark_instruction_fault_delivered(context);
                    return;
                }
                /* The CPU enters shutdown. Platform reset policy consumes the
                 * event without changing CPU exception production. */
                core_machine_cpu_execution_request_shutdown(context);
                cpu_state = fault_cpu;
                instruction_state.data.except = VCPUINS_EXCEPT_DF;
                instruction_state.data.excode = 0u;
            }
            else {
                instruction_state.data.except = original_except;
                instruction_state.data.excode = original_excode;
            }
        }

        if (!TYPE_GET_BIT(fault_cpu.data.cr0, VCPU_CR0_PE) &&
            (instruction_state.data.except == VCPUINS_EXCEPT_DE ||
             instruction_state.data.except == VCPUINS_EXCEPT_DB ||
             instruction_state.data.except == VCPUINS_EXCEPT_PF ||
             instruction_state.data.except == VCPUINS_EXCEPT_MF ||
             instruction_state.data.except == VCPUINS_EXCEPT_UD) &&
            _e_final_deliver_real_exception(context, &fault_cpu,
                exception_vector)) {
            _e_mark_instruction_fault_delivered(context);
            return;
        }

        if (!TYPE_GET_BIT(fault_cpu.data.cr0, VCPU_CR0_PE) &&
            TYPE_GET_BIT(instruction_state.data.except, VCPUINS_EXCEPT_BR) &&
            _e_final_deliver_real_exception(context, &fault_cpu, 0x05u)) {
            _e_mark_instruction_fault_delivered(context);
            return;
        }

        if (!TYPE_GET_BIT(fault_cpu.data.cr0, VCPU_CR0_PE) &&
            TYPE_GET_BIT(instruction_state.data.except, VCPUINS_EXCEPT_NM) &&
            _e_final_deliver_real_exception(context, &fault_cpu, 0x07u)) {
            _e_mark_instruction_fault_delivered(context);
            return;
        }

        if (!TYPE_GET_BIT(fault_cpu.data.cr0, VCPU_CR0_PE) &&
            TYPE_GET_BIT(instruction_state.data.except, VCPUINS_EXCEPT_GP) &&
            _e_final_deliver_real_exception(context, &fault_cpu, 0x0du)) {
            _e_mark_instruction_fault_delivered(context);
            return;
        }

        if (context->diagnostic_provider != STD_NULL &&
            context->diagnostic_provider->record_fault != STD_NULL)
        {
            context->diagnostic_provider->record_fault(context->diagnostic_context,
                                                       &fault_cpu, &instruction_state);
        }
        cpu_state = fault_cpu;
        core_machine_cpu_execution_request_stop(context);
    }
}
static C_VOID ExecIns(core_machine_cpu_execution_context *context)
{
    type_unsigned_8 opcode = 0;
    type_unsigned_32 debug_cause;

    ExecInit(context);
    debug_cause = _debug_match_instruction_breakpoint(context);
    if (debug_cause != TYPE_ZERO_32) {
        cpu_state.data.dr6 |= debug_cause;
        instruction_state.data.except = VCPUINS_EXCEPT_DB;
        ExecFinal(context);
        return;
    }
    do
    {
        TYPE_TRACE_CALL_BEGIN("ExecIns");
        TYPE_TRACE_CHECK_BREAK(_s_read_cs(context, cpu_state.data.eip, TYPE_REFERENCE_OF(opcode), 1));
        if (!core_machine_cpu_profile_allows_form(context,
                                                  CORE_MACHINE_CPU_INSTRUCTION_PRIMARY, opcode, 0u))
        {
            UndefinedOpcode(context);
            break;
        }
        TYPE_TRACE_CHECK_BREAK(ExecCpuInstruction(instruction_state.connect.insTable[opcode]));
        TYPE_TRACE_CHECK_BREAK(_s_test_eip(context));
        TYPE_TRACE_CHECK_BREAK(_s_test_esp(context));
        TYPE_TRACE_CALL_END;
    } while (_kdf_check_prefix(context, opcode));
    _debug_complete_instruction(context, opcode);
    if (instruction_state.data.flagWE && instruction_state.data.weLinear == instruction_state.data.linear)
    {
        STD_PRINTF("Watch point caught at L%08x: EXECUTED\n", instruction_state.data.linear);
        /* printCpuReg(); */
        core_machine_cpu_execution_request_stop(context);
    }
    ExecFinal(context);
}
static C_VOID ExecInt(core_machine_cpu_execution_context *context)
{
    type_unsigned_8 intr = 0x00;
    /* hardware interrupt handler */
    if (!instruction_state.data.flagMaskInt && !cpu_state.data.flagMaskNMI &&
        cpu_state.data.flagNMI)
    {
        ExecInit(context);
        _e_intr_n(context, 0x02, _GetOperandSize, TYPE_TRUE);
        if (!instruction_state.data.except) {
            cpu_state.data.flagHalt = TYPE_FALSE;
            cpu_state.data.flagNMI = TYPE_FALSE;
        }
        ExecFinal(context);
    }
    _debug_deliver_trap(context);
    if (context->stop_requested) return;
    if (!instruction_state.data.flagMaskInt && _GetEFLAGS_IF &&
        core_machine_pic_scan_interrupt(
                             context->pic_master, context->pic_slave))
    {
        if (context->transaction != STD_NULL && core_machine_transaction_begin(
                context->transaction, CORE_MACHINE_TRANSACTION_OWNER_CPU,
                CORE_MACHINE_TRANSACTION_CPU_INTERRUPT_ACKNOWLEDGE, 0u, 0u,
                0u) != TYPE_STATUS_OK) return;
        /* 8259A first INTA: the PIC owns the IRR-to-ISR transition before the
         * CPU consumes the vector through its existing interrupt-entry path. */
        intr = core_machine_pic_get_interrupt(context->pic_master,
            context->pic_slave);
        if (context->transaction != STD_NULL) {
            core_machine_transaction_commit(context->transaction);
        }
        ExecInit(context);
        _e_intr_n(context, intr, _GetOperandSize, TYPE_TRUE);
        if (!instruction_state.data.except) {
            cpu_state.data.flagHalt = TYPE_FALSE;
            instruction_state.data.flagIgnore = TYPE_TRUE;
        }
        ExecFinal(context);
    }
}

/* external interface */
type_bool core_machine_cpu_execution_load_segment(
    core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg,
    type_unsigned_16 selector)
{
    type_bool fail;
    type_unsigned_32 oldexcept = instruction_state.data.except;
    instruction_state.data.except = 0;
    _ksa_load_sreg(context, rsreg, selector);
    fail = !!instruction_state.data.except;
    instruction_state.data.except = oldexcept;
    return fail;
}
type_bool core_machine_cpu_execution_read_linear(
    core_machine_cpu_execution_context *context, type_unsigned_32 linear,
    type_virtual_address rdata, type_unsigned_8 byte)
{
    type_bool fail;
    type_unsigned_32 oldexcept = instruction_state.data.except;
    instruction_state.data.except = 0;
    _kma_read_linear(context, linear, rdata, byte, 0x00, 1);
    fail = !!instruction_state.data.except;
    instruction_state.data.except = oldexcept;
    return fail;
}
type_bool core_machine_cpu_execution_write_linear(
    core_machine_cpu_execution_context *context, type_unsigned_32 linear,
    type_virtual_address rdata, type_unsigned_8 byte)
{
    type_bool fail;
    type_unsigned_32 oldexcept = instruction_state.data.except;
    instruction_state.data.except = 0;
    _kma_write_linear(context, linear, rdata, byte, 0x00, 1);
    fail = !!instruction_state.data.except;
    instruction_state.data.except = oldexcept;
    return fail;
}

C_VOID core_machine_cpu_execution_initialize(
    core_machine_cpu_execution_context *context)
{
    instruction_state.connect.insTable[0x00] = (core_machine_cpu_instruction_handler)ADD_RM8_R8;
    instruction_state.connect.insTable[0x01] = (core_machine_cpu_instruction_handler)ADD_RM32_R32;
    instruction_state.connect.insTable[0x02] = (core_machine_cpu_instruction_handler)ADD_R8_RM8;
    instruction_state.connect.insTable[0x03] = (core_machine_cpu_instruction_handler)ADD_R32_RM32;
    instruction_state.connect.insTable[0x04] = (core_machine_cpu_instruction_handler)ADD_AL_I8;
    instruction_state.connect.insTable[0x05] = (core_machine_cpu_instruction_handler)ADD_EAX_I32;
    instruction_state.connect.insTable[0x06] = (core_machine_cpu_instruction_handler)PUSH_ES;
    instruction_state.connect.insTable[0x07] = (core_machine_cpu_instruction_handler)POP_ES;
    instruction_state.connect.insTable[0x08] = (core_machine_cpu_instruction_handler)OR_RM8_R8;
    instruction_state.connect.insTable[0x09] = (core_machine_cpu_instruction_handler)OR_RM32_R32;
    instruction_state.connect.insTable[0x0a] = (core_machine_cpu_instruction_handler)OR_R8_RM8;
    instruction_state.connect.insTable[0x0b] = (core_machine_cpu_instruction_handler)OR_R32_RM32;
    instruction_state.connect.insTable[0x0c] = (core_machine_cpu_instruction_handler)OR_AL_I8;
    instruction_state.connect.insTable[0x0d] = (core_machine_cpu_instruction_handler)OR_EAX_I32;
    instruction_state.connect.insTable[0x0e] = (core_machine_cpu_instruction_handler)PUSH_CS;
    instruction_state.connect.insTable[0x0f] = (core_machine_cpu_instruction_handler)INS_0F;
    instruction_state.connect.insTable[0x10] = (core_machine_cpu_instruction_handler)ADC_RM8_R8;
    instruction_state.connect.insTable[0x11] = (core_machine_cpu_instruction_handler)ADC_RM32_R32;
    instruction_state.connect.insTable[0x12] = (core_machine_cpu_instruction_handler)ADC_R8_RM8;
    instruction_state.connect.insTable[0x13] = (core_machine_cpu_instruction_handler)ADC_R32_RM32;
    instruction_state.connect.insTable[0x14] = (core_machine_cpu_instruction_handler)ADC_AL_I8;
    instruction_state.connect.insTable[0x15] = (core_machine_cpu_instruction_handler)ADC_EAX_I32;
    instruction_state.connect.insTable[0x16] = (core_machine_cpu_instruction_handler)PUSH_SS;
    instruction_state.connect.insTable[0x17] = (core_machine_cpu_instruction_handler)POP_SS;
    instruction_state.connect.insTable[0x18] = (core_machine_cpu_instruction_handler)SBB_RM8_R8;
    instruction_state.connect.insTable[0x19] = (core_machine_cpu_instruction_handler)SBB_RM32_R32;
    instruction_state.connect.insTable[0x1a] = (core_machine_cpu_instruction_handler)SBB_R8_RM8;
    instruction_state.connect.insTable[0x1b] = (core_machine_cpu_instruction_handler)SBB_R32_RM32;
    instruction_state.connect.insTable[0x1c] = (core_machine_cpu_instruction_handler)SBB_AL_I8;
    instruction_state.connect.insTable[0x1d] = (core_machine_cpu_instruction_handler)SBB_EAX_I32;
    instruction_state.connect.insTable[0x1e] = (core_machine_cpu_instruction_handler)PUSH_DS;
    instruction_state.connect.insTable[0x1f] = (core_machine_cpu_instruction_handler)POP_DS;
    instruction_state.connect.insTable[0x20] = (core_machine_cpu_instruction_handler)AND_RM8_R8;
    instruction_state.connect.insTable[0x21] = (core_machine_cpu_instruction_handler)AND_RM32_R32;
    instruction_state.connect.insTable[0x22] = (core_machine_cpu_instruction_handler)AND_R8_RM8;
    instruction_state.connect.insTable[0x23] = (core_machine_cpu_instruction_handler)AND_R32_RM32;
    instruction_state.connect.insTable[0x24] = (core_machine_cpu_instruction_handler)AND_AL_I8;
    instruction_state.connect.insTable[0x25] = (core_machine_cpu_instruction_handler)AND_EAX_I32;
    instruction_state.connect.insTable[0x26] = (core_machine_cpu_instruction_handler)PREFIX_ES;
    instruction_state.connect.insTable[0x27] = (core_machine_cpu_instruction_handler)DAA;
    instruction_state.connect.insTable[0x28] = (core_machine_cpu_instruction_handler)SUB_RM8_R8;
    instruction_state.connect.insTable[0x29] = (core_machine_cpu_instruction_handler)SUB_RM32_R32;
    instruction_state.connect.insTable[0x2a] = (core_machine_cpu_instruction_handler)SUB_R8_RM8;
    instruction_state.connect.insTable[0x2b] = (core_machine_cpu_instruction_handler)SUB_R32_RM32;
    instruction_state.connect.insTable[0x2c] = (core_machine_cpu_instruction_handler)SUB_AL_I8;
    instruction_state.connect.insTable[0x2d] = (core_machine_cpu_instruction_handler)SUB_EAX_I32;
    instruction_state.connect.insTable[0x2e] = (core_machine_cpu_instruction_handler)PREFIX_CS;
    instruction_state.connect.insTable[0x2f] = (core_machine_cpu_instruction_handler)DAS;
    instruction_state.connect.insTable[0x30] = (core_machine_cpu_instruction_handler)XOR_RM8_R8;
    instruction_state.connect.insTable[0x31] = (core_machine_cpu_instruction_handler)XOR_RM32_R32;
    instruction_state.connect.insTable[0x32] = (core_machine_cpu_instruction_handler)XOR_R8_RM8;
    instruction_state.connect.insTable[0x33] = (core_machine_cpu_instruction_handler)XOR_R32_RM32;
    instruction_state.connect.insTable[0x34] = (core_machine_cpu_instruction_handler)XOR_AL_I8;
    instruction_state.connect.insTable[0x35] = (core_machine_cpu_instruction_handler)XOR_EAX_I32;
    instruction_state.connect.insTable[0x36] = (core_machine_cpu_instruction_handler)PREFIX_SS;
    instruction_state.connect.insTable[0x37] = (core_machine_cpu_instruction_handler)AAA;
    instruction_state.connect.insTable[0x38] = (core_machine_cpu_instruction_handler)CMP_RM8_R8;
    instruction_state.connect.insTable[0x39] = (core_machine_cpu_instruction_handler)CMP_RM32_R32;
    instruction_state.connect.insTable[0x3a] = (core_machine_cpu_instruction_handler)CMP_R8_RM8;
    instruction_state.connect.insTable[0x3b] = (core_machine_cpu_instruction_handler)CMP_R32_RM32;
    instruction_state.connect.insTable[0x3c] = (core_machine_cpu_instruction_handler)CMP_AL_I8;
    instruction_state.connect.insTable[0x3d] = (core_machine_cpu_instruction_handler)CMP_EAX_I32;
    instruction_state.connect.insTable[0x3e] = (core_machine_cpu_instruction_handler)PREFIX_DS;
    instruction_state.connect.insTable[0x3f] = (core_machine_cpu_instruction_handler)AAS;
    instruction_state.connect.insTable[0x40] = (core_machine_cpu_instruction_handler)INC_EAX;
    instruction_state.connect.insTable[0x41] = (core_machine_cpu_instruction_handler)INC_ECX;
    instruction_state.connect.insTable[0x42] = (core_machine_cpu_instruction_handler)INC_EDX;
    instruction_state.connect.insTable[0x43] = (core_machine_cpu_instruction_handler)INC_EBX;
    instruction_state.connect.insTable[0x44] = (core_machine_cpu_instruction_handler)INC_ESP;
    instruction_state.connect.insTable[0x45] = (core_machine_cpu_instruction_handler)INC_EBP;
    instruction_state.connect.insTable[0x46] = (core_machine_cpu_instruction_handler)INC_ESI;
    instruction_state.connect.insTable[0x47] = (core_machine_cpu_instruction_handler)INC_EDI;
    instruction_state.connect.insTable[0x48] = (core_machine_cpu_instruction_handler)DEC_EAX;
    instruction_state.connect.insTable[0x49] = (core_machine_cpu_instruction_handler)DEC_ECX;
    instruction_state.connect.insTable[0x4a] = (core_machine_cpu_instruction_handler)DEC_EDX;
    instruction_state.connect.insTable[0x4b] = (core_machine_cpu_instruction_handler)DEC_EBX;
    instruction_state.connect.insTable[0x4c] = (core_machine_cpu_instruction_handler)DEC_ESP;
    instruction_state.connect.insTable[0x4d] = (core_machine_cpu_instruction_handler)DEC_EBP;
    instruction_state.connect.insTable[0x4e] = (core_machine_cpu_instruction_handler)DEC_ESI;
    instruction_state.connect.insTable[0x4f] = (core_machine_cpu_instruction_handler)DEC_EDI;
    instruction_state.connect.insTable[0x50] = (core_machine_cpu_instruction_handler)PUSH_EAX;
    instruction_state.connect.insTable[0x51] = (core_machine_cpu_instruction_handler)PUSH_ECX;
    instruction_state.connect.insTable[0x52] = (core_machine_cpu_instruction_handler)PUSH_EDX;
    instruction_state.connect.insTable[0x53] = (core_machine_cpu_instruction_handler)PUSH_EBX;
    instruction_state.connect.insTable[0x54] = (core_machine_cpu_instruction_handler)PUSH_ESP;
    instruction_state.connect.insTable[0x55] = (core_machine_cpu_instruction_handler)PUSH_EBP;
    instruction_state.connect.insTable[0x56] = (core_machine_cpu_instruction_handler)PUSH_ESI;
    instruction_state.connect.insTable[0x57] = (core_machine_cpu_instruction_handler)PUSH_EDI;
    instruction_state.connect.insTable[0x58] = (core_machine_cpu_instruction_handler)POP_EAX;
    instruction_state.connect.insTable[0x59] = (core_machine_cpu_instruction_handler)POP_ECX;
    instruction_state.connect.insTable[0x5a] = (core_machine_cpu_instruction_handler)POP_EDX;
    instruction_state.connect.insTable[0x5b] = (core_machine_cpu_instruction_handler)POP_EBX;
    instruction_state.connect.insTable[0x5c] = (core_machine_cpu_instruction_handler)POP_ESP;
    instruction_state.connect.insTable[0x5d] = (core_machine_cpu_instruction_handler)POP_EBP;
    instruction_state.connect.insTable[0x5e] = (core_machine_cpu_instruction_handler)POP_ESI;
    instruction_state.connect.insTable[0x5f] = (core_machine_cpu_instruction_handler)POP_EDI;
    instruction_state.connect.insTable[0x60] = (core_machine_cpu_instruction_handler)PUSHA;
    instruction_state.connect.insTable[0x61] = (core_machine_cpu_instruction_handler)POPA;
    instruction_state.connect.insTable[0x62] = (core_machine_cpu_instruction_handler)BOUND_R16_M16_16;
    instruction_state.connect.insTable[0x63] = (core_machine_cpu_instruction_handler)ARPL_RM16_R16;
    instruction_state.connect.insTable[0x64] = (core_machine_cpu_instruction_handler)PREFIX_FS;
    instruction_state.connect.insTable[0x65] = (core_machine_cpu_instruction_handler)PREFIX_GS;
    instruction_state.connect.insTable[0x66] = (core_machine_cpu_instruction_handler)PREFIX_OprSize;
    instruction_state.connect.insTable[0x67] = (core_machine_cpu_instruction_handler)PREFIX_AddrSize;
    instruction_state.connect.insTable[0x68] = (core_machine_cpu_instruction_handler)PUSH_I32;
    instruction_state.connect.insTable[0x69] = (core_machine_cpu_instruction_handler)IMUL_R32_RM32_I32;
    instruction_state.connect.insTable[0x6a] = (core_machine_cpu_instruction_handler)PUSH_I8;
    instruction_state.connect.insTable[0x6b] = (core_machine_cpu_instruction_handler)IMUL_R32_RM32_I8;
    instruction_state.connect.insTable[0x6c] = (core_machine_cpu_instruction_handler)INSB;
    instruction_state.connect.insTable[0x6d] = (core_machine_cpu_instruction_handler)INSW;
    instruction_state.connect.insTable[0x6e] = (core_machine_cpu_instruction_handler)OUTSB;
    instruction_state.connect.insTable[0x6f] = (core_machine_cpu_instruction_handler)OUTSW;
    instruction_state.connect.insTable[0x70] = (core_machine_cpu_instruction_handler)JO_REL8;
    instruction_state.connect.insTable[0x71] = (core_machine_cpu_instruction_handler)JNO_REL8;
    instruction_state.connect.insTable[0x72] = (core_machine_cpu_instruction_handler)JC_REL8;
    instruction_state.connect.insTable[0x73] = (core_machine_cpu_instruction_handler)JNC_REL8;
    instruction_state.connect.insTable[0x74] = (core_machine_cpu_instruction_handler)JZ_REL8;
    instruction_state.connect.insTable[0x75] = (core_machine_cpu_instruction_handler)JNZ_REL8;
    instruction_state.connect.insTable[0x76] = (core_machine_cpu_instruction_handler)JNA_REL8;
    instruction_state.connect.insTable[0x77] = (core_machine_cpu_instruction_handler)JA_REL8;
    instruction_state.connect.insTable[0x78] = (core_machine_cpu_instruction_handler)JS_REL8;
    instruction_state.connect.insTable[0x79] = (core_machine_cpu_instruction_handler)JNS_REL8;
    instruction_state.connect.insTable[0x7a] = (core_machine_cpu_instruction_handler)JP_REL8;
    instruction_state.connect.insTable[0x7b] = (core_machine_cpu_instruction_handler)JNP_REL8;
    instruction_state.connect.insTable[0x7c] = (core_machine_cpu_instruction_handler)JL_REL8;
    instruction_state.connect.insTable[0x7d] = (core_machine_cpu_instruction_handler)JNL_REL8;
    instruction_state.connect.insTable[0x7e] = (core_machine_cpu_instruction_handler)JNG_REL8;
    instruction_state.connect.insTable[0x7f] = (core_machine_cpu_instruction_handler)JG_REL8;
    instruction_state.connect.insTable[0x80] = (core_machine_cpu_instruction_handler)INS_80;
    instruction_state.connect.insTable[0x81] = (core_machine_cpu_instruction_handler)INS_81;
    instruction_state.connect.insTable[0x82] = (core_machine_cpu_instruction_handler)INS_80;
    instruction_state.connect.insTable[0x83] = (core_machine_cpu_instruction_handler)INS_83;
    instruction_state.connect.insTable[0x84] = (core_machine_cpu_instruction_handler)TEST_RM8_R8;
    instruction_state.connect.insTable[0x85] = (core_machine_cpu_instruction_handler)TEST_RM32_R32;
    instruction_state.connect.insTable[0x86] = (core_machine_cpu_instruction_handler)XCHG_RM8_R8;
    instruction_state.connect.insTable[0x87] = (core_machine_cpu_instruction_handler)XCHG_RM32_R32;
    instruction_state.connect.insTable[0x88] = (core_machine_cpu_instruction_handler)MOV_RM8_R8;
    instruction_state.connect.insTable[0x89] = (core_machine_cpu_instruction_handler)MOV_RM32_R32;
    instruction_state.connect.insTable[0x8a] = (core_machine_cpu_instruction_handler)MOV_R8_RM8;
    instruction_state.connect.insTable[0x8b] = (core_machine_cpu_instruction_handler)MOV_R32_RM32;
    instruction_state.connect.insTable[0x8c] = (core_machine_cpu_instruction_handler)MOV_RM16_SREG;
    instruction_state.connect.insTable[0x8d] = (core_machine_cpu_instruction_handler)LEA_R32_M32;
    instruction_state.connect.insTable[0x8e] = (core_machine_cpu_instruction_handler)MOV_SREG_RM16;
    instruction_state.connect.insTable[0x8f] = (core_machine_cpu_instruction_handler)INS_8F;
    instruction_state.connect.insTable[0x90] = (core_machine_cpu_instruction_handler)NOP;
    instruction_state.connect.insTable[0x91] = (core_machine_cpu_instruction_handler)XCHG_ECX_EAX;
    instruction_state.connect.insTable[0x92] = (core_machine_cpu_instruction_handler)XCHG_EDX_EAX;
    instruction_state.connect.insTable[0x93] = (core_machine_cpu_instruction_handler)XCHG_EBX_EAX;
    instruction_state.connect.insTable[0x94] = (core_machine_cpu_instruction_handler)XCHG_ESP_EAX;
    instruction_state.connect.insTable[0x95] = (core_machine_cpu_instruction_handler)XCHG_EBP_EAX;
    instruction_state.connect.insTable[0x96] = (core_machine_cpu_instruction_handler)XCHG_ESI_EAX;
    instruction_state.connect.insTable[0x97] = (core_machine_cpu_instruction_handler)XCHG_EDI_EAX;
    instruction_state.connect.insTable[0x98] = (core_machine_cpu_instruction_handler)CBW;
    instruction_state.connect.insTable[0x99] = (core_machine_cpu_instruction_handler)CWD;
    instruction_state.connect.insTable[0x9a] = (core_machine_cpu_instruction_handler)CALL_PTR16_32;
    instruction_state.connect.insTable[0x9b] = (core_machine_cpu_instruction_handler)WAIT;
    instruction_state.connect.insTable[0x9c] = (core_machine_cpu_instruction_handler)PUSHF;
    instruction_state.connect.insTable[0x9d] = (core_machine_cpu_instruction_handler)POPF;
    instruction_state.connect.insTable[0x9e] = (core_machine_cpu_instruction_handler)SAHF;
    instruction_state.connect.insTable[0x9f] = (core_machine_cpu_instruction_handler)LAHF;
    instruction_state.connect.insTable[0xa0] = (core_machine_cpu_instruction_handler)MOV_AL_MOFFS8;
    instruction_state.connect.insTable[0xa1] = (core_machine_cpu_instruction_handler)MOV_EAX_MOFFS32;
    instruction_state.connect.insTable[0xa2] = (core_machine_cpu_instruction_handler)MOV_MOFFS8_AL;
    instruction_state.connect.insTable[0xa3] = (core_machine_cpu_instruction_handler)MOV_MOFFS32_EAX;
    instruction_state.connect.insTable[0xa4] = (core_machine_cpu_instruction_handler)MOVSB;
    instruction_state.connect.insTable[0xa5] = (core_machine_cpu_instruction_handler)MOVSW;
    instruction_state.connect.insTable[0xa6] = (core_machine_cpu_instruction_handler)CMPSB;
    instruction_state.connect.insTable[0xa7] = (core_machine_cpu_instruction_handler)CMPSW;
    instruction_state.connect.insTable[0xa8] = (core_machine_cpu_instruction_handler)TEST_AL_I8;
    instruction_state.connect.insTable[0xa9] = (core_machine_cpu_instruction_handler)TEST_EAX_I32;
    instruction_state.connect.insTable[0xaa] = (core_machine_cpu_instruction_handler)STOSB;
    instruction_state.connect.insTable[0xab] = (core_machine_cpu_instruction_handler)STOSW;
    instruction_state.connect.insTable[0xac] = (core_machine_cpu_instruction_handler)LODSB;
    instruction_state.connect.insTable[0xad] = (core_machine_cpu_instruction_handler)LODSW;
    instruction_state.connect.insTable[0xae] = (core_machine_cpu_instruction_handler)SCASB;
    instruction_state.connect.insTable[0xaf] = (core_machine_cpu_instruction_handler)SCASW;
    instruction_state.connect.insTable[0xb0] = (core_machine_cpu_instruction_handler)MOV_AL_I8;
    instruction_state.connect.insTable[0xb1] = (core_machine_cpu_instruction_handler)MOV_CL_I8;
    instruction_state.connect.insTable[0xb2] = (core_machine_cpu_instruction_handler)MOV_DL_I8;
    instruction_state.connect.insTable[0xb3] = (core_machine_cpu_instruction_handler)MOV_BL_I8;
    instruction_state.connect.insTable[0xb4] = (core_machine_cpu_instruction_handler)MOV_AH_I8;
    instruction_state.connect.insTable[0xb5] = (core_machine_cpu_instruction_handler)MOV_CH_I8;
    instruction_state.connect.insTable[0xb6] = (core_machine_cpu_instruction_handler)MOV_DH_I8;
    instruction_state.connect.insTable[0xb7] = (core_machine_cpu_instruction_handler)MOV_BH_I8;
    instruction_state.connect.insTable[0xb8] = (core_machine_cpu_instruction_handler)MOV_EAX_I32;
    instruction_state.connect.insTable[0xb9] = (core_machine_cpu_instruction_handler)MOV_ECX_I32;
    instruction_state.connect.insTable[0xba] = (core_machine_cpu_instruction_handler)MOV_EDX_I32;
    instruction_state.connect.insTable[0xbb] = (core_machine_cpu_instruction_handler)MOV_EBX_I32;
    instruction_state.connect.insTable[0xbc] = (core_machine_cpu_instruction_handler)MOV_ESP_I32;
    instruction_state.connect.insTable[0xbd] = (core_machine_cpu_instruction_handler)MOV_EBP_I32;
    instruction_state.connect.insTable[0xbe] = (core_machine_cpu_instruction_handler)MOV_ESI_I32;
    instruction_state.connect.insTable[0xbf] = (core_machine_cpu_instruction_handler)MOV_EDI_I32;
    instruction_state.connect.insTable[0xc0] = (core_machine_cpu_instruction_handler)INS_C0;
    instruction_state.connect.insTable[0xc1] = (core_machine_cpu_instruction_handler)INS_C1;
    instruction_state.connect.insTable[0xc2] = (core_machine_cpu_instruction_handler)RET_I16;
    instruction_state.connect.insTable[0xc3] = (core_machine_cpu_instruction_handler)RET;
    instruction_state.connect.insTable[0xc4] = (core_machine_cpu_instruction_handler)LES_R32_M16_32;
    instruction_state.connect.insTable[0xc5] = (core_machine_cpu_instruction_handler)LDS_R32_M16_32;
    instruction_state.connect.insTable[0xc6] = (core_machine_cpu_instruction_handler)INS_C6;
    instruction_state.connect.insTable[0xc7] = (core_machine_cpu_instruction_handler)INS_C7;
    instruction_state.connect.insTable[0xc8] = (core_machine_cpu_instruction_handler)ENTER;
    instruction_state.connect.insTable[0xc9] = (core_machine_cpu_instruction_handler)LEAVE;
    instruction_state.connect.insTable[0xca] = (core_machine_cpu_instruction_handler)RETF_I16;
    instruction_state.connect.insTable[0xcb] = (core_machine_cpu_instruction_handler)RETF;
    instruction_state.connect.insTable[0xcc] = (core_machine_cpu_instruction_handler)INT3;
    instruction_state.connect.insTable[0xcd] = (core_machine_cpu_instruction_handler)INT_I8;
    instruction_state.connect.insTable[0xce] = (core_machine_cpu_instruction_handler)INTO;
    instruction_state.connect.insTable[0xcf] = (core_machine_cpu_instruction_handler)IRET;
    instruction_state.connect.insTable[0xd0] = (core_machine_cpu_instruction_handler)INS_D0;
    instruction_state.connect.insTable[0xd1] = (core_machine_cpu_instruction_handler)INS_D1;
    instruction_state.connect.insTable[0xd2] = (core_machine_cpu_instruction_handler)INS_D2;
    instruction_state.connect.insTable[0xd3] = (core_machine_cpu_instruction_handler)INS_D3;
    instruction_state.connect.insTable[0xd4] = (core_machine_cpu_instruction_handler)AAM;
    instruction_state.connect.insTable[0xd5] = (core_machine_cpu_instruction_handler)AAD;
    instruction_state.connect.insTable[0xd6] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable[0xd7] = (core_machine_cpu_instruction_handler)XLAT;
    instruction_state.connect.insTable[0xd8] = (core_machine_cpu_instruction_handler)FPU_ESCAPE;
    instruction_state.connect.insTable[0xd9] = (core_machine_cpu_instruction_handler)FPU_ESCAPE;
    instruction_state.connect.insTable[0xda] = (core_machine_cpu_instruction_handler)FPU_ESCAPE;
    instruction_state.connect.insTable[0xdb] = (core_machine_cpu_instruction_handler)FPU_ESCAPE;
    instruction_state.connect.insTable[0xdc] = (core_machine_cpu_instruction_handler)FPU_ESCAPE;
    instruction_state.connect.insTable[0xdd] = (core_machine_cpu_instruction_handler)FPU_ESCAPE;
    instruction_state.connect.insTable[0xde] = (core_machine_cpu_instruction_handler)FPU_ESCAPE;
    instruction_state.connect.insTable[0xdf] = (core_machine_cpu_instruction_handler)FPU_ESCAPE;
    instruction_state.connect.insTable[0xe0] = (core_machine_cpu_instruction_handler)LOOPNZ_REL8;
    instruction_state.connect.insTable[0xe1] = (core_machine_cpu_instruction_handler)LOOPZ_REL8;
    instruction_state.connect.insTable[0xe2] = (core_machine_cpu_instruction_handler)LOOP_REL8;
    instruction_state.connect.insTable[0xe3] = (core_machine_cpu_instruction_handler)JCXZ_REL8;
    instruction_state.connect.insTable[0xe4] = (core_machine_cpu_instruction_handler)IN_AL_I8;
    instruction_state.connect.insTable[0xe5] = (core_machine_cpu_instruction_handler)IN_EAX_I8;
    instruction_state.connect.insTable[0xe6] = (core_machine_cpu_instruction_handler)OUT_I8_AL;
    instruction_state.connect.insTable[0xe7] = (core_machine_cpu_instruction_handler)OUT_I8_EAX;
    instruction_state.connect.insTable[0xe8] = (core_machine_cpu_instruction_handler)CALL_REL32;
    instruction_state.connect.insTable[0xe9] = (core_machine_cpu_instruction_handler)JMP_REL32;
    instruction_state.connect.insTable[0xea] = (core_machine_cpu_instruction_handler)JMP_PTR16_32;
    instruction_state.connect.insTable[0xeb] = (core_machine_cpu_instruction_handler)JMP_REL8;
    instruction_state.connect.insTable[0xec] = (core_machine_cpu_instruction_handler)IN_AL_DX;
    instruction_state.connect.insTable[0xed] = (core_machine_cpu_instruction_handler)IN_EAX_DX;
    instruction_state.connect.insTable[0xee] = (core_machine_cpu_instruction_handler)OUT_DX_AL;
    instruction_state.connect.insTable[0xef] = (core_machine_cpu_instruction_handler)OUT_DX_EAX;
    instruction_state.connect.insTable[0xf0] = (core_machine_cpu_instruction_handler)PREFIX_LOCK;
    instruction_state.connect.insTable[0xf1] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable[0xf2] = (core_machine_cpu_instruction_handler)PREFIX_REPNZ;
    instruction_state.connect.insTable[0xf3] = (core_machine_cpu_instruction_handler)PREFIX_REPZ;
    instruction_state.connect.insTable[0xf4] = (core_machine_cpu_instruction_handler)HLT;
    instruction_state.connect.insTable[0xf5] = (core_machine_cpu_instruction_handler)CMC;
    instruction_state.connect.insTable[0xf6] = (core_machine_cpu_instruction_handler)INS_F6;
    instruction_state.connect.insTable[0xf7] = (core_machine_cpu_instruction_handler)INS_F7;
    instruction_state.connect.insTable[0xf8] = (core_machine_cpu_instruction_handler)CLC;
    instruction_state.connect.insTable[0xf9] = (core_machine_cpu_instruction_handler)STC;
    instruction_state.connect.insTable[0xfa] = (core_machine_cpu_instruction_handler)CLI;
    instruction_state.connect.insTable[0xfb] = (core_machine_cpu_instruction_handler)STI;
    instruction_state.connect.insTable[0xfc] = (core_machine_cpu_instruction_handler)CLD;
    instruction_state.connect.insTable[0xfd] = (core_machine_cpu_instruction_handler)STD;
    instruction_state.connect.insTable[0xfe] = (core_machine_cpu_instruction_handler)INS_FE;
    instruction_state.connect.insTable[0xff] = (core_machine_cpu_instruction_handler)INS_FF;
    instruction_state.connect.insTable_0f[0x00] = (core_machine_cpu_instruction_handler)INS_0F_00;
    instruction_state.connect.insTable_0f[0x01] = (core_machine_cpu_instruction_handler)INS_0F_01;
    instruction_state.connect.insTable_0f[0x02] = (core_machine_cpu_instruction_handler)LAR_R32_RM32;
    instruction_state.connect.insTable_0f[0x03] = (core_machine_cpu_instruction_handler)LSL_R32_RM32;
    instruction_state.connect.insTable_0f[0x04] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x05] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x06] = (core_machine_cpu_instruction_handler)CLTS;
    instruction_state.connect.insTable_0f[0x07] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x08] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x09] = (core_machine_cpu_instruction_handler)WBINVD;
    instruction_state.connect.insTable_0f[0x0a] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x0b] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x0c] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x0d] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x0e] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x0f] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x10] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x11] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x12] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x13] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x14] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x15] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x16] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x17] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x18] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x19] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x1a] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x1b] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x1c] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x1d] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x1e] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x1f] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x20] = (core_machine_cpu_instruction_handler)MOV_R32_CR;
    instruction_state.connect.insTable_0f[0x21] = (core_machine_cpu_instruction_handler)MOV_R32_DR;
    instruction_state.connect.insTable_0f[0x22] = (core_machine_cpu_instruction_handler)MOV_CR_R32;
    instruction_state.connect.insTable_0f[0x23] = (core_machine_cpu_instruction_handler)MOV_DR_R32;
    instruction_state.connect.insTable_0f[0x24] = (core_machine_cpu_instruction_handler)MOV_R32_TR;
    instruction_state.connect.insTable_0f[0x25] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x26] = (core_machine_cpu_instruction_handler)MOV_TR_R32;
    instruction_state.connect.insTable_0f[0x27] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x28] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x29] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x2a] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x2b] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x2c] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x2d] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x2e] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x2f] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x30] = (core_machine_cpu_instruction_handler)WRMSR;
    instruction_state.connect.insTable_0f[0x31] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x32] = (core_machine_cpu_instruction_handler)RDMSR;
    instruction_state.connect.insTable_0f[0x33] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x34] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x35] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x36] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x37] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x38] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x39] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x3a] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x3b] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x3c] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x3d] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x3e] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x3f] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x40] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x41] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x42] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x43] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x44] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x45] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x46] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x47] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x48] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x49] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x4a] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x4b] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x4c] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x4d] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x4e] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x4f] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x50] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x51] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x52] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x53] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x54] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x55] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x56] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x57] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x58] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x59] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x5a] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x5b] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x5c] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x5d] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x5e] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x5f] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x60] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x61] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x62] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x63] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x64] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x65] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x66] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x67] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x68] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x69] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x6a] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x6b] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x6c] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x6d] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x6e] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x6f] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x70] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x71] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x72] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x73] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x74] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x75] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x76] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x77] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x78] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x79] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x7a] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x7b] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x7c] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x7d] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x7e] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x7f] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x80] = (core_machine_cpu_instruction_handler)JO_REL32;
    instruction_state.connect.insTable_0f[0x81] = (core_machine_cpu_instruction_handler)JNO_REL32;
    instruction_state.connect.insTable_0f[0x82] = (core_machine_cpu_instruction_handler)JC_REL32;
    instruction_state.connect.insTable_0f[0x83] = (core_machine_cpu_instruction_handler)JNC_REL32;
    instruction_state.connect.insTable_0f[0x84] = (core_machine_cpu_instruction_handler)JZ_REL32;
    instruction_state.connect.insTable_0f[0x85] = (core_machine_cpu_instruction_handler)JNZ_REL32;
    instruction_state.connect.insTable_0f[0x86] = (core_machine_cpu_instruction_handler)JNA_REL32;
    instruction_state.connect.insTable_0f[0x87] = (core_machine_cpu_instruction_handler)JA_REL32;
    instruction_state.connect.insTable_0f[0x88] = (core_machine_cpu_instruction_handler)JS_REL32;
    instruction_state.connect.insTable_0f[0x89] = (core_machine_cpu_instruction_handler)JNS_REL32;
    instruction_state.connect.insTable_0f[0x8a] = (core_machine_cpu_instruction_handler)JP_REL32;
    instruction_state.connect.insTable_0f[0x8b] = (core_machine_cpu_instruction_handler)JNP_REL32;
    instruction_state.connect.insTable_0f[0x8c] = (core_machine_cpu_instruction_handler)JL_REL32;
    instruction_state.connect.insTable_0f[0x8d] = (core_machine_cpu_instruction_handler)JNL_REL32;
    instruction_state.connect.insTable_0f[0x8e] = (core_machine_cpu_instruction_handler)JNG_REL32;
    instruction_state.connect.insTable_0f[0x8f] = (core_machine_cpu_instruction_handler)JG_REL32;
    instruction_state.connect.insTable_0f[0x90] = (core_machine_cpu_instruction_handler)SETO_RM8;
    instruction_state.connect.insTable_0f[0x91] = (core_machine_cpu_instruction_handler)SETNO_RM8;
    instruction_state.connect.insTable_0f[0x92] = (core_machine_cpu_instruction_handler)SETC_RM8;
    instruction_state.connect.insTable_0f[0x93] = (core_machine_cpu_instruction_handler)SETNC_RM8;
    instruction_state.connect.insTable_0f[0x94] = (core_machine_cpu_instruction_handler)SETZ_RM8;
    instruction_state.connect.insTable_0f[0x95] = (core_machine_cpu_instruction_handler)SETNZ_RM8;
    instruction_state.connect.insTable_0f[0x96] = (core_machine_cpu_instruction_handler)SETNA_RM8;
    instruction_state.connect.insTable_0f[0x97] = (core_machine_cpu_instruction_handler)SETA_RM8;
    instruction_state.connect.insTable_0f[0x98] = (core_machine_cpu_instruction_handler)SETS_RM8;
    instruction_state.connect.insTable_0f[0x99] = (core_machine_cpu_instruction_handler)SETNS_RM8;
    instruction_state.connect.insTable_0f[0x9a] = (core_machine_cpu_instruction_handler)SETP_RM8;
    instruction_state.connect.insTable_0f[0x9b] = (core_machine_cpu_instruction_handler)SETNP_RM8;
    instruction_state.connect.insTable_0f[0x9c] = (core_machine_cpu_instruction_handler)SETL_RM8;
    instruction_state.connect.insTable_0f[0x9d] = (core_machine_cpu_instruction_handler)SETNL_RM8;
    instruction_state.connect.insTable_0f[0x9e] = (core_machine_cpu_instruction_handler)SETNG_RM8;
    instruction_state.connect.insTable_0f[0x9f] = (core_machine_cpu_instruction_handler)SETG_RM8;
    instruction_state.connect.insTable_0f[0xa0] = (core_machine_cpu_instruction_handler)PUSH_FS;
    instruction_state.connect.insTable_0f[0xa1] = (core_machine_cpu_instruction_handler)POP_FS;
    instruction_state.connect.insTable_0f[0xa2] = (core_machine_cpu_instruction_handler)CPUID;
    instruction_state.connect.insTable_0f[0xa3] = (core_machine_cpu_instruction_handler)BT_RM32_R32;
    instruction_state.connect.insTable_0f[0xa4] = (core_machine_cpu_instruction_handler)SHLD_RM32_R32_I8;
    instruction_state.connect.insTable_0f[0xa5] = (core_machine_cpu_instruction_handler)SHLD_RM32_R32_CL;
    instruction_state.connect.insTable_0f[0xa6] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xa7] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xa8] = (core_machine_cpu_instruction_handler)PUSH_GS;
    instruction_state.connect.insTable_0f[0xa9] = (core_machine_cpu_instruction_handler)POP_GS;
    instruction_state.connect.insTable_0f[0xaa] = (core_machine_cpu_instruction_handler)RSM;
    instruction_state.connect.insTable_0f[0xab] = (core_machine_cpu_instruction_handler)BTS_RM32_R32;
    instruction_state.connect.insTable_0f[0xac] = (core_machine_cpu_instruction_handler)SHRD_RM32_R32_I8;
    instruction_state.connect.insTable_0f[0xad] = (core_machine_cpu_instruction_handler)SHRD_RM32_R32_CL;
    instruction_state.connect.insTable_0f[0xae] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xaf] = (core_machine_cpu_instruction_handler)IMUL_R32_RM32;
    instruction_state.connect.insTable_0f[0xb0] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xb1] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xb2] = (core_machine_cpu_instruction_handler)LSS_R32_M16_32;
    instruction_state.connect.insTable_0f[0xb3] = (core_machine_cpu_instruction_handler)BTR_RM32_R32;
    instruction_state.connect.insTable_0f[0xb4] = (core_machine_cpu_instruction_handler)LFS_R32_M16_32;
    instruction_state.connect.insTable_0f[0xb5] = (core_machine_cpu_instruction_handler)LGS_R32_M16_32;
    instruction_state.connect.insTable_0f[0xb6] = (core_machine_cpu_instruction_handler)MOVZX_R32_RM8;
    instruction_state.connect.insTable_0f[0xb7] = (core_machine_cpu_instruction_handler)MOVZX_R32_RM16;
    instruction_state.connect.insTable_0f[0xb8] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xb9] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xba] = (core_machine_cpu_instruction_handler)INS_0F_BA;
    instruction_state.connect.insTable_0f[0xbb] = (core_machine_cpu_instruction_handler)BTC_RM32_R32;
    instruction_state.connect.insTable_0f[0xbc] = (core_machine_cpu_instruction_handler)BSF_R32_RM32;
    instruction_state.connect.insTable_0f[0xbd] = (core_machine_cpu_instruction_handler)BSR_R32_RM32;
    instruction_state.connect.insTable_0f[0xbe] = (core_machine_cpu_instruction_handler)MOVSX_R32_RM8;
    instruction_state.connect.insTable_0f[0xbf] = (core_machine_cpu_instruction_handler)MOVSX_R32_RM16;
    instruction_state.connect.insTable_0f[0xc0] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc1] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc2] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc3] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc4] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc5] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc6] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc7] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc8] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc9] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xca] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xcb] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xcc] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xcd] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xce] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xcf] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd0] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd1] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd2] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd3] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd4] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd5] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd6] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd7] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd8] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd9] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xda] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xdb] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xdc] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xdd] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xde] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xdf] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe0] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe1] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe2] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe3] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe4] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe5] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe6] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe7] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe8] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe9] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xea] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xeb] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xec] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xed] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xee] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xef] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf0] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf1] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf2] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf3] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf4] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf5] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf6] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf7] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf8] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf9] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xfa] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xfb] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xfc] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xfd] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xfe] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xff] = (core_machine_cpu_instruction_handler)UndefinedOpcode;
}
C_VOID core_machine_cpu_execution_reset(
    core_machine_cpu_execution_context *context)
{
    STD_MEMSET((C_VOID *)(&instruction_state.data), TYPE_ZERO_8, sizeof(t_cpuins_data));
    context->debug_trap_pending = TYPE_FALSE;
    context->debug_tf_before = TYPE_FALSE;
    context->debug_rf_before = TYPE_FALSE;
    context->debug_trap_cause = TYPE_ZERO_32;
    context->instruction_in_progress = TYPE_FALSE;
    context->instruction_fault_delivered = TYPE_FALSE;
}
C_VOID core_machine_cpu_execution_refresh(
    core_machine_cpu_execution_context *context)
{
    context->instruction_fault_delivered = TYPE_FALSE;
    if (!cpu_state.data.flagHalt)
    {
        context->instruction_in_progress = TYPE_TRUE;
        ExecIns(context);
        context->instruction_in_progress = TYPE_FALSE;
    }
    ExecInt(context);
}

type_bool core_machine_cpu_execution_consume_instruction_fault_delivery(
    core_machine_cpu_execution_context *context)
{
    type_bool delivered;

    if (context == STD_NULL) return TYPE_FALSE;
    delivered = context->instruction_fault_delivered;
    context->instruction_fault_delivered = TYPE_FALSE;
    return delivered;
}

C_VOID core_machine_cpu_execution_finalize(
    core_machine_cpu_execution_context *context)
{
    if (context != STD_NULL)
    {
        STD_FREE(context->trace);
        context->trace = STD_NULL;
    }
}
