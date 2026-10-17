#include "lib/types/types_interface.h"
#include "x86-core/cpu/cpu_instructions.h"
#include "x86-core/cpu/cpu.h"
#include "x86-core/cpu/cpu_trace.h"
#include "x86-core/cpu/cpu_timing.h"

#define cpu_state (*context->cpu)
#define instruction_state (*context->instructions)
#define ExecCpuInstruction(handler) ((handler) ? ((handler)(context), 0) : 0)

#define CPU_TRACE_ERROR instruction_state.data.except
#define CPU_TRACE_SET_ERROR (_SetExcept_CE(0xffffffff))

/* Copy while the precise fault/trap state is still owned by CPU execution.
 * The callback may retain the value, never the borrowed snapshot pointer. */
static void x86_core_cpu_diagnostic_publish_snapshot(
    void (*publish)(void *, const x86_core_cpu_fault_snapshot *),
    void *opaque, const t_cpu *cpu, const t_cpuins *instructions)
{
    x86_core_cpu_fault_snapshot snapshot = {0};

    snapshot.valid = 1;
    snapshot.exception_mask = instructions->data.except;
    snapshot.exception_code = instructions->data.excode;
    snapshot.point.cs = cpu->data.cs.selector;
    snapshot.point.cs_base = cpu->data.cs.base;
    snapshot.point.eip = cpu->data.eip;
    snapshot.point.linear_pc = cpu->data.cs.base + cpu->data.eip;
    if (cpu->data.cs.selector == instructions->data.oldcpu.data.cs.selector &&
        cpu->data.cs.base == instructions->data.oldcpu.data.cs.base &&
        cpu->data.eip == instructions->data.oldcpu.data.eip &&
        cpu->data.tr.selector == instructions->data.oldcpu.data.tr.selector &&
        cpu->data.cr3 == instructions->data.oldcpu.data.cr3) {
        snapshot.point.byte_count = (lib_u8)instructions->data.oplen;
        lib_memory_copy(snapshot.point.bytes, instructions->data.opcodes,
            sizeof(snapshot.point.bytes));
    }
    snapshot.eax = cpu->data.eax;
    snapshot.ebx = cpu->data.ebx;
    snapshot.ecx = cpu->data.ecx;
    snapshot.edx = cpu->data.edx;
    snapshot.cr2 = cpu->data.cr2;
    snapshot.esp = cpu->data.esp;
    snapshot.ss = cpu->data.ss.selector;
    snapshot.ss_base = cpu->data.ss.base;
    snapshot.ebp = cpu->data.ebp;
    snapshot.esi = cpu->data.esi;
    snapshot.edi = cpu->data.edi;
    snapshot.eflags = cpu->data.eflags;
    publish(opaque, &snapshot);
}

void x86_core_cpu_execution_copy_observation(
    const x86_core_cpu_execution_context *context,
    x86_core_cpu_instruction_observation *observation)
{
    if (context == LIB_NULL || observation == LIB_NULL) return;
    if (context->completed_observation_valid) {
        *observation = context->completed_observation;
        return;
    }
    *observation = (x86_core_cpu_instruction_observation){0};
    observation->point.cs = cpu_state.data.cs.selector;
    observation->point.cs_base = cpu_state.data.cs.base;
    observation->point.eip = cpu_state.data.eip;
    observation->point.linear_pc = instruction_state.data.linear;
    observation->point.byte_count = (lib_u8)instruction_state.data.oplen;
    lib_memory_copy(observation->point.bytes, instruction_state.data.opcodes,
        sizeof(observation->point.bytes));
    observation->old_eip = instruction_state.data.oldcpu.data.eip;
    observation->old_default_size_32 =
        instruction_state.data.oldcpu.data.cs.seg.exec.defsize != 0u;
    observation->eax = cpu_state.data.eax;
    observation->dx = cpu_state.data.dx;
    observation->cpl = (lib_u8)cpu_state.data.cs.dpl;
    observation->protected_mode = (cpu_state.data.cr0 & VCPU_CR0_PE) != 0u;
    observation->virtual_8086_mode = (cpu_state.data.eflags & VCPU_EFLAGS_VM) != 0u;
    observation->operand_size_32 = cpu_state.data.cs.seg.exec.defsize ^
        instruction_state.data.prefix_oprsize;
    observation->address_size_32 = cpu_state.data.cs.seg.exec.defsize ^
        instruction_state.data.prefix_addrsize;
    observation->lock_prefix = instruction_state.data.flagLock;
    observation->repeat_prefix = (lib_u8)instruction_state.data.prefix_rep;
}

/* indicates functions not implemented */
#define _______todo static void
#define _new_code_path_ do { } while (0)

/* stack pointer size */
#define _GetStackSize (cpu_state.data.ss.seg.data.big ? 4 : 2)
/* operand size */
#define _GetOperandSize ((cpu_state.data.cs.seg.exec.defsize ^ instruction_state.data.prefix_oprsize) ? 4 : 2)
/* address size of the source operand */
#define _GetAddressSize ((cpu_state.data.cs.seg.exec.defsize ^ instruction_state.data.prefix_addrsize) ? 4 : 2)
/* if opcode indicates a prefix */
static void x86_core_cpu_execution_raise_exception(
    x86_core_cpu_execution_context *context, lib_u32 exception,
    lib_u32 code)
{
    X86_CPU_BIT_SET(instruction_state.data.except, exception);
    instruction_state.data.excode = code;
}
#define _SetExcept_DE(n) x86_core_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_DE, (n))
#define _SetExcept_PF(n) x86_core_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_PF, (n))
#define _SetExcept_GP(n) x86_core_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_GP, (n))
#define _SetExcept_SS(n) x86_core_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_SS, (n))
#define _SetExcept_UD(n) x86_core_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_UD, (n))
#define _SetExcept_NP(n) x86_core_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_NP, (n))
#define _SetExcept_BR(n) x86_core_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_BR, (n))
#define _SetExcept_TS(n) x86_core_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_TS, (n))
#define _SetExcept_NM(n) x86_core_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_NM, (n))
#define _SetExcept_DF(n) x86_core_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_DF, (n))
#define _SetExcept_MF(n) x86_core_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_MF, (n))
#define _SetExcept_AC(n) x86_core_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_AC, (n))
#define _SetExcept_FPU_UNSUPPORTED(n) x86_core_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_FPU_UNSUPPORTED, (n))
#define _SetExcept_CE(n) x86_core_cpu_execution_raise_exception(context, VCPUINS_EXCEPT_CE, (n))

#define VCPU_DR6_BD 0x00002000u
#define VCPU_DR6_BS 0x00004000u
#define VCPU_DR6_BT 0x00008000u
#define VCPU_DR7_LOCAL_ENABLE_MASK 0x00000155u
#define VCPU_DR7_GD 0x00002000u


static void UndefinedOpcode(x86_core_cpu_execution_context *context);

static void _debug_record_watchpoint(
    x86_core_cpu_execution_context *context,
    x86_core_cpu_watchpoint kind, lib_u32 address)
{
    if (!instruction_state.data.watch_hit) {
        instruction_state.data.watch_hit = LIB_TRUE;
        instruction_state.data.watch_kind = (lib_u8)kind;
        instruction_state.data.watch_address = address;
    }
    x86_core_cpu_execution_request_debug_pause(context);
}

/* Incoming writable fields remain canonical; an outgoing FLAGS image also
 * carries the family's fixed bits. Reserved image bits are not stored state. */
static lib_u16 _e_real_flags_defined_mask(
    const x86_core_cpu_execution_context *context)
{
    if (context == LIB_NULL ||
        !cpu_profile_has_protected_architecture(context->cpu_profile)) {
        return 0x0fd5u;
    }
    return 0x7fd5u;
}

static lib_u16 _e_real_flags_load_16(
    const x86_core_cpu_execution_context *context, lib_u16 flags)
{
    return X86_CPU_MASK_U16((flags &
        _e_real_flags_defined_mask(context)) | 0x02u |
        (context != LIB_NULL && context->cpu_profile == X86_CORE_CPU_PROFILE_V30 ?
            context->cpu->data.eflags & X86_CPU_NEC_PSW_MD : 0u));
}

static lib_u16 _e_real_flags_image_16(
    const x86_core_cpu_execution_context *context, lib_u16 flags)
{
    const lib_u16 fixed = context != LIB_NULL &&
        context->cpu_profile == X86_CORE_CPU_PROFILE_V30 ? 0x7000u :
        context != LIB_NULL &&
        (x86_core_cpu_profile_has_8086_semantics(context->cpu_profile) ||
         context->cpu_profile == X86_CORE_CPU_PROFILE_80186) ? 0xf000u : 0u;

    return X86_CPU_MASK_U16(_e_real_flags_load_16(context, flags) | fixed);
}

static lib_u32 _e_eflags_load(
    const x86_core_cpu_execution_context *context, lib_u32 flags)
{
    if (context == LIB_NULL ||
        !cpu_profile_has_32bit_architecture(context->cpu_profile)) {
        return _e_real_flags_load_16(context, X86_CPU_MASK_U16(flags));
    }
    return x86_core_cpu_eflags_normalize(context->cpu_profile, flags);
}

static lib_bool _e_vm86_interrupts_virtualized(
    const x86_core_cpu_execution_context *context)
{
    return context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        _GetCR0_PE && _GetEFLAGS_VM && _GetEFLAGS_IOPL < 3u &&
        (cpu_state.data.cr4 & VCPU_CR4_VME) != 0u;
}

/* memory management unit */
/* kernel memory accessing */
_Static_assert(lib_offsetof(t_cpu_data, ebx) == lib_offsetof(t_cpu_data, eax) + 4u &&
    lib_offsetof(t_cpu_data, ecx) == lib_offsetof(t_cpu_data, eax) + 8u &&
    lib_offsetof(t_cpu_data, edx) == lib_offsetof(t_cpu_data, eax) + 12u &&
    lib_offsetof(t_cpu_data, esp) == lib_offsetof(t_cpu_data, eax) + 16u &&
    lib_offsetof(t_cpu_data, ebp) == lib_offsetof(t_cpu_data, eax) + 20u &&
    lib_offsetof(t_cpu_data, esi) == lib_offsetof(t_cpu_data, eax) + 24u &&
    lib_offsetof(t_cpu_data, edi) == lib_offsetof(t_cpu_data, eax) + 28u &&
    lib_offsetof(t_cpu_data, eip) == lib_offsetof(t_cpu_data, eax) + 32u,
    "GPR destination slots must match the CPU register image");

/* Record the actual destination even when its value does not change. Byte
 * aliases share their parent GPR; instruction preview and debugger reads do
 * not publish a preceding-instruction dependency. */
static inline lib_uptr _kma_gpr_destination(
    x86_core_cpu_execution_context *context, lib_uptr reference)
{
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        context->instruction_in_progress && !context->preview_mode) {
        const lib_uptr offset = reference - X86_CPU_REFERENCE_OF(cpu_state.data.eax);
        static const lib_u8 masks[] = {1u, 8u, 2u, 4u, 16u, 32u, 64u, 128u};

        if (offset < 8u * sizeof(lib_u32))
            instruction_state.data.timing_gpr_writes |= masks[offset / sizeof(lib_u32)];
    }
    return reference;
}

#define _WriteGPR8(reg) (*(lib_u8 *)_kma_gpr_destination(context, X86_CPU_REFERENCE_OF(cpu_state.data.reg)))
#define _WriteGPR16(reg) (*(lib_u16 *)_kma_gpr_destination(context, X86_CPU_REFERENCE_OF(cpu_state.data.reg)))
#define _WriteGPR32(reg) (*(lib_u32 *)_kma_gpr_destination(context, X86_CPU_REFERENCE_OF(cpu_state.data.reg)))

/* read content from reference */
static void _kma_read_ref(x86_core_cpu_execution_context *context, lib_uptr ref, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_kma_read_ref");
    lib_memory_copy((void *)_kma_gpr_destination(context, rdata), (void *)ref, byte);
    CPU_TRACE_CALL_END;
}
/* write content to reference */
static void _kma_write_ref(x86_core_cpu_execution_context *context, lib_uptr ref, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_kma_write_ref");
    lib_memory_copy((void *)_kma_gpr_destination(context, ref), (void *)rdata, byte);
    CPU_TRACE_CALL_END;
}
/* Both spaces share accepted-cycle publication. Core computes the extra
 * waits; this observation publishes neither guest time nor physical overlap. */
static lib_u64 _kma_publish_external_cycle(
    x86_core_cpu_execution_context *context,
    x86_core_cpu_external_cycle_phase phase,
    x86_core_cpu_external_cycle_space space, lib_u32 address,
    lib_u8 byte, lib_u8 write,
    x86_core_cpu_memory_access_provenance provenance)
{
    if (context != LIB_NULL && !context->preview_mode &&
        context->external_cycle_provider != LIB_NULL) {
        const lib_u64 ticks = context->external_cycle_provider(
            context->external_cycle_context, phase, space, address, byte,
            write, provenance);

        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX && context->timing_bus_active &&
            phase == X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT) {
            if (ticks > LIB_UINT64_MAX - instruction_state.data.timing_external_wait_ticks)
                instruction_state.data.timing_external_wait_overflow = LIB_TRUE;
            else instruction_state.data.timing_external_wait_ticks += ticks;
        }
        return ticks;
    }
    return 0u;
}

static lib_i32 _kma_is_reset_vector_fetch(
    const x86_core_cpu_execution_context *context,
    lib_u32 physical, lib_u8 bytes,
    x86_core_cpu_memory_access_provenance provenance)
{
    lib_u32 reset_vector;

    if (context == LIB_NULL || bytes == 0u ||
        (provenance != X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH &&
         provenance != X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH)) {
        return 0;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286) {
        reset_vector = 0x00fffff0u;
    } else if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386 ||
        context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) {
        reset_vector = 0xfffffff0u;
    } else {
        return 0;
    }
    return physical >= reset_vector &&
        physical <= LIB_UINT32_MAX - (lib_u32)(bytes - 1u);
}

static void _kma_cache_touch(x86_core_cpu_execution_context *context,
    lib_u8 set, lib_u8 way);
static void _kma_cache_report(x86_core_cpu_execution_context *context,
    lib_u8 set, lib_u8 way);

static lib_u8 _kma_cache_way(x86_core_cpu_execution_context *context,
    lib_u32 address)
{
    const lib_u8 set = (address >> 4u) & (X86_CPU_CACHE_SETS - 1u);
    for (lib_u8 way = 0u; way < X86_CPU_CACHE_WAYS; ++way)
        if (context->cache[set][way].valid &&
            context->cache[set][way].tag == (address & 0xfffff800u)) return way;
    return X86_CPU_CACHE_WAYS;
}

static lib_u8 _kma_cache_replacement(x86_core_cpu_execution_context *context,
    lib_u8 set)
{
    const lib_u8 lru = context->cache_lru[set];
    for (lib_u8 way = 0u; way < X86_CPU_CACHE_WAYS; ++way)
        if (!context->cache[set][way].valid) return way;
    return lru & 1u ? (lib_u8)(2u + ((lru >> 2u) & 1u)) :
        (lib_u8)((lru >> 1u) & 1u);
}

static lib_u8 _kma_memory_cache_control(x86_core_cpu_execution_context *context,
    lib_u32 attributes)
{
    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX) return 0u;
    return (attributes & (X86_CORE_CPU_MEMORY_CACHE_PWT |
        X86_CORE_CPU_MEMORY_CACHE_PCD)) |
        ((cpu_state.data.cr0 & VCPU_CR0_CD) != 0u ? X86_CORE_CPU_MEMORY_CACHE_PCD : 0u);
}

static void _kma_begin_memory_lock(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("_kma_begin_memory_lock");
    if (instruction_state.data.memory_lock_active) return;
    if ((context->bus->begin_memory_lock != LIB_NULL) !=
        (context->bus->complete_memory_lock != LIB_NULL))
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(0u));
    x86_core_cpu_bus_drain(context, LIB_TRUE);
    if (context->bus->begin_memory_lock != LIB_NULL &&
        context->bus->begin_memory_lock(context->bus_context) != LIB_STATUS_OK)
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(0u));
    instruction_state.data.memory_lock_active = LIB_TRUE;
    CPU_TRACE_CALL_END;
}

static void _kma_complete_memory_lock(x86_core_cpu_execution_context *context,
    lib_bool commit)
{
    if (!instruction_state.data.memory_lock_active) return;
    x86_core_cpu_bus_drain(context, LIB_FALSE);
    instruction_state.data.memory_lock_active = LIB_FALSE;
    if (context->bus->complete_memory_lock != LIB_NULL)
        context->bus->complete_memory_lock(context->bus_context, commit);
}

/* A completed bus transfer is the sole external-read publication path. */
static void _kma_read_bus(x86_core_cpu_execution_context *context,
    lib_u32 physical, lib_uptr rdata, lib_u8 byte,
    x86_core_cpu_memory_access_provenance provenance, lib_u32 attributes)
{
    lib_status memory_status;
    CPU_TRACE_CALL_BEGIN("_kma_read_bus");
    x86_core_cpu_bus_prepare_read(context, instruction_state.data.memory_lock_active);
    _kma_publish_external_cycle(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN,
        X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, physical, byte, LIB_FALSE, provenance);
    memory_status = context->bus->read_memory(context->bus_context, physical,
        (void *)rdata, byte, provenance, context->preview_mode,
        _kma_is_reset_vector_fetch(context, physical, byte, provenance),
        _kma_memory_cache_control(context, attributes));
    if (memory_status != LIB_STATUS_OK) {
        _kma_publish_external_cycle(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, physical, byte, LIB_FALSE, provenance);
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(physical));
    }
    const lib_u64 wait_ticks = _kma_publish_external_cycle(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT,
        X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, physical, byte, LIB_FALSE, provenance);
    x86_core_cpu_bus_read_complete(context, physical, byte, wait_ticks);
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX && !context->preview_mode &&
        (provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH ||
         provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH))
        instruction_state.data.timing_fetch_bus_ticks +=
            1u + ((physical & 3u) + byte + 3u) / 4u;
    CPU_TRACE_CALL_END;
}

/* Cache eligibility affects fills, not hits in an already resident line. */
static void _kma_read_physical(x86_core_cpu_execution_context *context,
    lib_u32 physical, lib_uptr rdata, lib_u8 byte,
    x86_core_cpu_memory_access_provenance provenance, lib_u32 attributes)
{
    CPU_TRACE_CALL_BEGIN("_kma_read_physical");
    /* The processor owns the width of its external physical-address bus.
     * Board A20 routing remains in the memory owner and is applied afterwards.
     * In particular, the 80286 reset fetch at FFFFF0h wraps at 16 MiB rather
     * than falling into a synthetic 25th address bit. */
    if (!cpu_profile_has_protected_architecture(context->cpu_profile)) {
        physical &= 0x000fffffu;
    } else if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286) {
        physical &= 0x00ffffffu;
    }
    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX) {
        CPU_TRACE_CHECK_RETURN(_kma_read_bus(context, physical, rdata, byte, provenance, attributes));
        CPU_TRACE_CALL_END;
        return;
    }
    for (lib_u8 done = 0u; done < byte;) {
        x86_core_cpu_cache_memory memory = {.address = physical + done};
        const x86_core_cpu_cache_query query =
            _kma_is_reset_vector_fetch(context, physical + done, 1u, provenance) ?
                X86_CORE_CPU_CACHE_RESET_LOOKUP : X86_CORE_CPU_CACHE_LOOKUP;
        lib_u8 way, part, offset, set;
        lib_bool missed;

        if (context->bus->query_cache_memory != LIB_NULL &&
            context->bus->query_cache_memory(context->bus_context, physical + done,
                query, &memory) != LIB_STATUS_OK)
            CPU_TRACE_CHECK_RETURN(_SetExcept_CE(physical + done));
        offset = memory.address & 15u;
        part = byte - done;
        if (part > 16u - offset) part = 16u - offset;
        if (instruction_state.data.memory_lock_active) {
            CPU_TRACE_CHECK_RETURN(_kma_read_bus(context, memory.address,
                rdata + done, part, provenance, attributes));
            if (!context->preview_mode && provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA)
                ++instruction_state.data.timing_locked_reads;
            done += part;
            continue;
        }
        set = (memory.address >> 4u) & (X86_CPU_CACHE_SETS - 1u);
        way = _kma_cache_way(context, memory.address);
        missed = way == X86_CPU_CACHE_WAYS;
        if (way == X86_CPU_CACHE_WAYS && !context->preview_mode &&
            query == X86_CORE_CPU_CACHE_LOOKUP &&
            (cpu_state.data.cr0 & VCPU_CR0_CD) == 0u &&
            (attributes & X86_CORE_CPU_MEMORY_CACHE_PCD) == 0u &&
            context->bus->query_cache_memory != LIB_NULL &&
            context->bus->query_cache_memory(context->bus_context, physical + done,
                X86_CORE_CPU_CACHE_FILL_PROBE, &memory) != LIB_STATUS_OK)
            CPU_TRACE_CHECK_RETURN(_SetExcept_CE(physical + done));
        if (way != X86_CPU_CACHE_WAYS) {
            lib_memory_copy((void *)(rdata + done),
                (const lib_u8 *)context->cache[set][way].data + offset, part);
            if (!context->preview_mode) {
                _kma_cache_touch(context, set, way);
                if (provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA ||
                    provenance == X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ)
                    _kma_cache_report(context, set, way);
            }
        } else if (!context->preview_mode && memory.fill_enabled &&
            (cpu_state.data.cr0 & VCPU_CR0_CD) == 0u && (attributes & 0x10u) == 0u) {
            lib_u32 data[X86_CPU_CACHE_LINE_DWORDS];
            CPU_TRACE_CHECK_RETURN(_kma_read_bus(context, memory.address & ~15u,
                (lib_uptr)data, sizeof(data), provenance, attributes));
            way = _kma_cache_replacement(context, set);
            context->cache[set][way] = (cpu_cache_line){
                .tag = memory.address & 0xfffff800u, .valid = LIB_TRUE};
            lib_memory_copy(context->cache[set][way].data, data, sizeof(data));
            lib_memory_copy(context->cache_fill, data, sizeof(data));
            _kma_cache_touch(context, set, way);
            _kma_cache_report(context, set, way);
            lib_memory_copy((void *)(rdata + done), (const lib_u8 *)data + offset, part);
        } else {
            CPU_TRACE_CHECK_RETURN(_kma_read_bus(context, memory.address,
                rdata + done, part, provenance, attributes));
        }
        if (missed && !context->preview_mode) {
            if (provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA)
                ++instruction_state.data.timing_data_misses;
            else if (provenance == X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ)
                ++instruction_state.data.timing_table_misses;
        }
        done += part;
    }
    CPU_TRACE_CALL_END;
}

static void _kma_write_bus(x86_core_cpu_execution_context *context,
    lib_u32 physical, lib_uptr rdata, lib_u8 byte,
    x86_core_cpu_memory_access_provenance provenance, lib_u32 attributes)
{
    CPU_TRACE_CALL_BEGIN("_kma_write_bus");
    x86_core_cpu_bus_prepare_write(context);
    _kma_publish_external_cycle(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN,
        X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, physical, byte, LIB_TRUE, provenance);
    if (context->bus->write_memory(context->bus_context, physical,
            (const void *)rdata, byte, provenance,
            _kma_memory_cache_control(context, attributes)) != LIB_STATUS_OK) {
        _kma_publish_external_cycle(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, physical, byte, LIB_TRUE, provenance);
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(physical));
    }
    const lib_u64 wait_ticks = _kma_publish_external_cycle(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT,
        X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY, physical, byte, LIB_TRUE, provenance);
    x86_core_cpu_bus_write_complete(context, physical, byte, wait_ticks,
        context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
            _kma_cache_way(context, physical) != X86_CPU_CACHE_WAYS,
        (cpu_state.data.cr0 & VCPU_CR0_CD) == 0u);
    CPU_TRACE_CALL_END;
}

/* write content to physical */
static void _kma_write_physical(x86_core_cpu_execution_context *context, lib_u32 physical, lib_uptr rdata, lib_u8 byte, x86_core_cpu_memory_access_provenance provenance, lib_u32 attributes)
{
    CPU_TRACE_CALL_BEGIN("_kma_write_physical");
    if (!cpu_profile_has_protected_architecture(context->cpu_profile)) {
        physical &= 0x000fffffu;
    } else if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286) {
        physical &= 0x00ffffffu;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) {
        for (lib_u8 done = 0u; done < byte;) {
            x86_core_cpu_cache_memory memory = {.address = physical + done};
            lib_u8 way, part, offset, set;

            if (context->bus->query_cache_memory != LIB_NULL &&
                context->bus->query_cache_memory(context->bus_context,
                    physical + done, X86_CORE_CPU_CACHE_LOOKUP, &memory) != LIB_STATUS_OK)
                CPU_TRACE_CHECK_RETURN(_SetExcept_CE(physical + done));
            offset = memory.address & 15u;
            part = byte - done;
            if (part > 16u - offset) part = 16u - offset;
            set = (memory.address >> 4u) & (X86_CPU_CACHE_SETS - 1u);
            way = _kma_cache_way(context, memory.address);
            if (instruction_state.data.memory_lock_active || way == X86_CPU_CACHE_WAYS ||
                (cpu_state.data.cr0 & VCPU_CR0_NW) == 0u) {
                _kma_write_bus(context, memory.address,
                    rdata + done, part, provenance, attributes);
                if (instruction_state.data.except) {
                    if (way != X86_CPU_CACHE_WAYS)
                        context->cache[set][way].valid = LIB_FALSE;
                    CPU_TRACE_CALL_END;
                    return;
                }
            }
            if (way != X86_CPU_CACHE_WAYS) {
                lib_memory_copy((lib_u8 *)context->cache[set][way].data + offset,
                    (const void *)(rdata + done), part);
                _kma_cache_touch(context, set, way);
                _kma_cache_report(context, set, way);
            }
            done += part;
        }
        CPU_TRACE_CALL_END;
        return;
    }
    CPU_TRACE_CHECK_RETURN(_kma_write_bus(context, physical, rdata, byte, provenance, attributes));
    CPU_TRACE_CALL_END;
}

/* Invalidating does not erase the tag or data arrays (Intel 273021-001 B.2.5). */
static void _kma_cache_invalidate(x86_core_cpu_execution_context *context)
{
    x86_core_cpu_bus_invalidate(context);
    for (lib_u8 set = 0u; set < X86_CPU_CACHE_SETS; ++set) {
        for (lib_u8 way = 0u; way < X86_CPU_CACHE_WAYS; ++way)
            context->cache[set][way].valid = LIB_FALSE;
        context->cache_lru[set] = 0u;
    }
}

static void ExecFinal(x86_core_cpu_execution_context *context);
static lib_bool _e_timing_charge(x86_core_cpu_execution_context *context, lib_u64 ticks);

static lib_bool _system_flush_pending(x86_core_cpu_execution_context *context)
{
    if ((!context->flush_pending && !context->flush_asserted) ||
        context->stop_granted || !context->clock_running) return LIB_FALSE;
    if (cpu_state.data.flagHalt && !context->smi_pending &&
        (context->nmi_masked || context->nmi_in_service || !context->nmi_pending) &&
        (!X86_CPU_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_IF) ||
         !context->bus->interrupt_pending(context->bus_context))) return LIB_FALSE;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX && !_e_timing_charge(context, 1u)) {
        ExecFinal(context);
        return LIB_TRUE;
    }
    _kma_cache_invalidate(context);
    x86_core_cpu_execution_invalidate_prefetch(context);
    context->flush_pending = LIB_FALSE;
    return LIB_TRUE;
}

static void _kma_cache_touch(x86_core_cpu_execution_context *context,
    lib_u8 set, lib_u8 way)
{
    lib_u8 lru = context->cache_lru[set];

    if (context->timing_bus_active && !context->preview_mode)
        instruction_state.data.timing_cache_used = LIB_TRUE;
    if (way < 2u)
        lru = (lru & 4u) | 1u | (way == 0u ? 2u : 0u);
    else
        lru = (lru & 2u) | (way == 2u ? 4u : 0u);
    context->cache_lru[set] = lru;
}

static void _kma_cache_test(x86_core_cpu_execution_context *context,
    lib_u32 command)
{
    const lib_u8 set = (command >> 4u) & (X86_CPU_CACHE_SETS - 1u);
    const lib_u8 way = (command >> 2u) & (X86_CPU_CACHE_WAYS - 1u);
    cpu_cache_line *line = &context->cache[set][way];

    cpu_state.data.tr5 = command & 0x7ffu;
    switch (command & 3u) {
    case 0u:
        break;
    case 1u:
        x86_core_cpu_bus_invalidate(context);
        line->tag = cpu_state.data.tr4 & 0xfffff800u;
        line->valid = (cpu_state.data.tr4 & 0x400u) != 0u;
        lib_memory_copy(line->data, context->cache_fill, sizeof(line->data));
        _kma_cache_touch(context, set, way);
        break;
    case 2u:
        if (context->timing_bus_active && !context->preview_mode)
            instruction_state.data.timing_cache_used = LIB_TRUE;
        _kma_cache_report(context, set, way);
        break;
    case 3u:
        _kma_cache_invalidate(context);
        break;
    }
}

static void _kma_cache_report(x86_core_cpu_execution_context *context,
    lib_u8 set, lib_u8 way)
{
    const cpu_cache_line *line = &context->cache[set][way];
    cpu_state.data.tr4 = line->tag | (line->valid ? 0x400u : 0u) |
        ((lib_u32)context->cache_lru[set] << 7u);
    for (lib_u8 entry = 0u; entry < X86_CPU_CACHE_WAYS; ++entry)
        if (context->cache[set][entry].valid)
            cpu_state.data.tr4 |= 1u << (entry + 3u);
    lib_memory_copy(context->cache_read, line->data, sizeof(line->data));
}

void x86_core_cpu_signal_cache_write(x86_core_cpu_execution_context *context,
    lib_u32 physical, lib_size bytes)
{
    lib_u64 end, remaining;

    if (context == LIB_NULL || bytes == 0u ||
        context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX ||
        (cpu_state.data.cr0 & VCPU_CR0_NW) != 0u) return;
    x86_core_cpu_bus_invalidate(context);
    remaining = (lib_u64)LIB_UINT32_MAX + 1u - physical;
    end = (lib_u64)physical + (bytes > remaining ? remaining : bytes);
    if ((end - (physical & ~15u)) / 16u < X86_CPU_CACHE_SETS) {
        for (lib_u64 address = physical & ~15u; address < end; address += 16u) {
            const lib_u8 set = (address >> 4u) & (X86_CPU_CACHE_SETS - 1u);
            for (lib_u8 way = 0u; way < X86_CPU_CACHE_WAYS; ++way)
                if (context->cache[set][way].tag == ((lib_u32)address & 0xfffff800u))
                    context->cache[set][way].valid = LIB_FALSE;
        }
    } else {
        for (lib_u8 set = 0u; set < X86_CPU_CACHE_SETS; ++set)
        for (lib_u8 way = 0u; way < X86_CPU_CACHE_WAYS; ++way) {
            cpu_cache_line *line = &context->cache[set][way];
            const lib_u32 address = line->tag | ((lib_u32)set << 4u);
            if ((lib_u64)address < end && (lib_u64)address + 16u > physical)
                line->valid = LIB_FALSE;
        }
    }
}

void x86_core_cpu_execution_invalidate_tlb(
    x86_core_cpu_execution_context *context)
{
    for (lib_u8 set = 0u; set < X86_CPU_TLB_SETS; ++set)
        for (lib_u8 way = 0u; way < X86_CPU_TLB_WAYS; ++way)
            context->tlb[set][way].valid = LIB_FALSE;
}

static lib_u8 _kma_tlb_way(x86_core_cpu_execution_context *context,
    lib_u32 linear)
{
    const lib_u8 set = (linear >> 12u) & (X86_CPU_TLB_SETS - 1u);

    for (lib_u8 way = 0u; way < X86_CPU_TLB_WAYS; ++way) {
        const cpu_tlb_entry *entry = &context->tlb[set][way];
        if (entry->valid && entry->linear == (linear & VCPU_CR3_BASE))
            return way;
    }
    return X86_CPU_TLB_WAYS;
}

/* Three-bit tree replacement, shared by ordinary and test-register access. */
static lib_u8 _kma_tlb_replacement(lib_u8 lru)
{
    return lru & 1u ? (lib_u8)(2u + ((lru >> 2u) & 1u)) :
        (lib_u8)((lru >> 1u) & 1u);
}

static void _kma_tlb_touch(x86_core_cpu_execution_context *context,
    lib_u8 set, lib_u8 way)
{
    lib_u8 *lru = &context->tlb_lru[set];

    if (way < 2u)
        *lru = (lib_u8)((*lru & 4u) | 1u | (way == 0u ? 2u : 0u));
    else
        *lru = (lib_u8)((*lru & 2u) | (way == 2u ? 4u : 0u));
}

static void _kma_tlb_fill(x86_core_cpu_execution_context *context,
    lib_u32 linear, lib_u32 physical, lib_u32 attributes)
{
    const lib_u8 set = (linear >> 12u) & (X86_CPU_TLB_SETS - 1u);
    lib_u8 way = _kma_tlb_way(context, linear);

    if (way == X86_CPU_TLB_WAYS)
        way = _kma_tlb_replacement(context->tlb_lru[set]);
    context->tlb[set][way] = (cpu_tlb_entry) {
        .linear = linear & VCPU_CR3_BASE,
        .physical = physical & VCPU_CR3_BASE,
        .attributes = attributes, .valid = LIB_TRUE};
    _kma_tlb_touch(context, set, way);
}

static void _kma_tlb_test(x86_core_cpu_execution_context *context,
    lib_u32 command)
{
    const lib_u8 set = (command >> 12u) & (X86_CPU_TLB_SETS - 1u);
    const lib_u32 linear = command & VCPU_CR3_BASE;
    const lib_u32 data = cpu_state.data.tr7;
    const lib_bool valid = (command & 0x800u) != 0u;
    lib_u8 way, matches = 0u, found = 0u;

    cpu_state.data.tr6 = command & 0xffffffe1u;
    if (!(command & 1u))
    {
        way = data & 0x10u ? (lib_u8)((data >> 2u) & 3u) :
            _kma_tlb_replacement(context->tlb_lru[set]);
        context->tlb[set][way] = (cpu_tlb_entry) {
            .linear = linear, .physical = data & VCPU_CR3_BASE,
            .attributes = ((command & 0x400u) ? 0x40u : 0u) |
                ((command & 0x100u) ? 4u : 0u) |
                ((command & 0x40u) ? 2u : 0u) |
                ((data & 0x800u) ? 0x10u : 0u) |
                ((data & 0x400u) ? 8u : 0u),
            .valid = valid};
        _kma_tlb_touch(context, set, way);
        return;
    }
    for (way = 0u; way < X86_CPU_TLB_WAYS; ++way)
    {
        const cpu_tlb_entry *entry = &context->tlb[set][way];
        lib_bool match = entry->linear == linear && entry->valid == valid;
        const lib_u32 bits[] = {0x40u, 4u, 2u};
        const lib_u8 shifts[] = {9u, 7u, 5u};

        for (lib_u8 bit = 0u; bit < 3u; ++bit) {
            const lib_u8 pair = (command >> shifts[bit]) & 3u;
            if (!(pair & ((entry->attributes & bits[bit]) ? 2u : 1u)))
                match = LIB_FALSE;
        }
        if (match) { found = way; ++matches; }
    }
    /* Duplicate tags and invalid-tag initialization are not defined tests. */
    cpu_state.data.tr7 = (data & ~0x390u) |
        ((lib_u32)context->tlb_lru[set] << 7u);
    if (matches == 1u)
    {
        const cpu_tlb_entry *entry = &context->tlb[set][found];
        const lib_u32 attributes = entry->attributes;

        cpu_state.data.tr6 = linear | (valid ? 0x800u : 0u) | 1u |
            ((attributes & 0x40u) ? 0x400u : 0x200u) |
            ((attributes & 4u) ? 0x100u : 0x80u) |
            ((attributes & 2u) ? 0x40u : 0x20u);
        cpu_state.data.tr7 = entry->physical |
            ((attributes & 0x10u) ? 0x800u : 0u) |
            ((attributes & 8u) ? 0x400u : 0u) |
            ((lib_u32)context->tlb_lru[set] << 7u) |
            0x10u | ((lib_u32)found << 2u);
        _kma_tlb_touch(context, set, found);
    }
}

typedef struct t_kma_linear_translation {
    lib_u32 physical;
    lib_u32 linear;
    lib_u32 attributes;
    lib_u32 ppde;
    lib_u32 ppte;
    lib_u32 cpde;
    lib_u32 cpte;
    lib_u8 paging;
    lib_u8 pde_accessed;
    lib_u8 vpl;
    lib_bool tlb_hit;
    lib_u8 timing_update_attempts;
} t_kma_linear_translation;

/* The locked second read, not a cached first value, owns the A/D update. */
static void _kma_update_page_entry(x86_core_cpu_execution_context *context,
    t_kma_linear_translation *translation, lib_bool directory, lib_u8 write)
{
    const lib_u32 physical = directory ? translation->ppde : translation->ppte;
    const lib_u32 attributes = directory ? cpu_state.data.cr3 : translation->cpde;
    const lib_u32 required = directory || !write ? 0x20u : 0x60u;
    const lib_bool user = translation->vpl == 3u;
    const lib_bool owns_lock = !instruction_state.data.memory_lock_active;
    x86_core_cpu_cache_memory memory = {.address = physical};
    lib_u32 entry = 0u;
    lib_u8 set, way;
    CPU_TRACE_CALL_BEGIN("_kma_update_page_entry");

    if (context->bus->query_cache_memory != LIB_NULL &&
        context->bus->query_cache_memory(context->bus_context, physical,
            X86_CORE_CPU_CACHE_LOOKUP, &memory) != LIB_STATUS_OK)
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(physical));
    set = (memory.address >> 4u) & (X86_CPU_CACHE_SETS - 1u);
    way = _kma_cache_way(context, memory.address);
    CPU_TRACE_CHECK_RETURN(_kma_begin_memory_lock(context));
    ++translation->timing_update_attempts;

    _kma_read_bus(context, memory.address, X86_CPU_REFERENCE_OF(entry), 4u,
        X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ, attributes);
    if (!instruction_state.data.except && !_IsPageEntryPresent(entry)) {
        cpu_state.data.cr2 = translation->linear;
        _SetExcept_PF(_MakePageFaultErrorCode(0, write, user));
    }
    if (!instruction_state.data.except && !directory &&
        ((user && !_GetPageEntry_US(entry)) ||
         (write && (user || (cpu_state.data.cr0 & VCPU_CR0_WP) != 0u) &&
          !_IsPageEntryWritable(entry)))) {
        cpu_state.data.cr2 = translation->linear;
        _SetExcept_PF(_MakePageFaultErrorCode(1, write, user));
    }
    if (!instruction_state.data.except && (entry & required) != required) {
        entry |= required;
        _kma_write_bus(context, memory.address, X86_CPU_REFERENCE_OF(entry), 4u,
            X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE, attributes);
        if (!instruction_state.data.except) ++instruction_state.data.timing_tlb_updates;
        if (way != X86_CPU_CACHE_WAYS) {
            cpu_cache_line *line = &context->cache[set][way];
            if (instruction_state.data.except) line->valid = LIB_FALSE;
            else if (line->valid) {
                lib_memory_copy((lib_u8 *)line->data + (memory.address & 15u),
                    &entry, sizeof(entry));
                _kma_cache_touch(context, set, way);
                _kma_cache_report(context, set, way);
            }
        }
    }
    if (owns_lock) _kma_complete_memory_lock(context, instruction_state.data.except == 0u);
    if (!instruction_state.data.except) {
        if (directory) translation->cpde = entry;
        else translation->cpte = entry;
    }
    CPU_TRACE_CALL_END;
}

/* Validate a linear page translation without publishing page-table side effects. */
static void _kma_prepare_physical_linear(x86_core_cpu_execution_context *context, lib_u32 linear, lib_u8 byte, lib_u8 write, lib_u8 vpl, t_kma_linear_translation *translation)
{
    lib_u32 ppde, ppte; /* page table entries */
    lib_u32 cpde, cpte;
    CPU_TRACE_CALL_BEGIN("_kma_prepare_physical_linear");
    *translation = (t_kma_linear_translation) {0};
    translation->linear = linear;
    translation->vpl = vpl;
    if (_GetLinear_Offset(linear) > X86_CPU_MASK_U32(_GetPageSize - byte))
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (!_IsPaging)
    {
        translation->physical = linear;
        translation->paging = LIB_FALSE;
        CPU_TRACE_CALL_END;
        return;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
    {
        const lib_u8 set = (linear >> 12u) & (X86_CPU_TLB_SETS - 1u);
        const lib_u8 way = _kma_tlb_way(context, linear);

        if (way != X86_CPU_TLB_WAYS)
        {
            const cpu_tlb_entry *entry = &context->tlb[set][way];
            const lib_bool user = vpl == 3u;
            const lib_bool protected_write = user ||
                (cpu_state.data.cr0 & VCPU_CR0_WP) != 0u;

            if ((user && !_GetPageEntry_US(entry->attributes)) ||
                (write && protected_write && !_IsPageEntryWritable(entry->attributes)))
            {
                cpu_state.data.cr2 = linear;
                CPU_TRACE_CHECK_RETURN(_SetExcept_PF(_MakePageFaultErrorCode(1, write, user)));
            }
            /* A first write to a clean translation must publish the page-table
             * Dirty bit through the ordinary walk, not a stored host pointer. */
            if (!write || (entry->attributes & 0x40u) != 0u)
            {
                translation->physical = entry->physical + _GetLinear_Offset(linear);
                translation->attributes = entry->attributes;
                translation->paging = LIB_TRUE;
                translation->tlb_hit = LIB_TRUE;
                CPU_TRACE_CALL_END;
                return;
            }
        }
    }
    ppde = _GetCR3_Base + _GetLinear_Dir(linear) * 4;
    CPU_TRACE_CHECK_RETURN(_kma_read_physical(context, ppde, X86_CPU_REFERENCE_OF(cpde), 4, X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ, cpu_state.data.cr3));
    if (!_IsPageEntryPresent(cpde))
    {
        CPU_TRACE_BLOCK_BEGIN("!PageDirEntryPresent");
        cpu_state.data.cr2 = linear;
        CPU_TRACE_CHECK_RETURN(_SetExcept_PF(_MakePageFaultErrorCode(0, write, (vpl == 3))));
        return;
        CPU_TRACE_BLOCK_END;
    }
    /* The 80386 sets the directory Accessed bit while walking its
     * second-level page table (80386 PRM 5.3.4.3), even when that walk later
     * finds a protection violation in the page-table entry. */
    if (!context->preview_mode) {
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
            (cpde & 0x20u) == 0u) {
            translation->ppde = ppde;
            translation->cpde = cpde;
            CPU_TRACE_CHECK_RETURN(_kma_update_page_entry(context, translation, LIB_TRUE, write));
            cpde = translation->cpde;
        } else if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX) {
            _SetPageEntry_A(cpde);
            CPU_TRACE_CHECK_RETURN(_kma_write_physical(context, ppde,
                X86_CPU_REFERENCE_OF(cpde), 4,
                X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE, cpu_state.data.cr3));
        }
        translation->pde_accessed = LIB_TRUE;
    }
    if (vpl == 0x03 || (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        (cpu_state.data.cr0 & VCPU_CR0_WP) != 0u))
    {
        CPU_TRACE_BLOCK_BEGIN("PageDirPermission");
        if (vpl == 0x03 && !_GetPageEntry_US(cpde))
        {
            CPU_TRACE_BLOCK_BEGIN("PageDirEntry_US(0)");
            cpu_state.data.cr2 = linear;
            CPU_TRACE_CHECK_RETURN(_SetExcept_PF(_MakePageFaultErrorCode(1, write, 1)));
            return;
            CPU_TRACE_BLOCK_END;
        }
        if (write && !_IsPageEntryWritable(cpde))
        {
            CPU_TRACE_BLOCK_BEGIN("write,!PageDirEntryWritable");
            cpu_state.data.cr2 = linear;
            CPU_TRACE_CHECK_RETURN(_SetExcept_PF(_MakePageFaultErrorCode(1, 1, vpl == 3)));
            return;
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
    }
    ppte = _GetPageEntry_Base(cpde) + _GetLinear_Page(linear) * 4;
    CPU_TRACE_CHECK_RETURN(_kma_read_physical(context, ppte, X86_CPU_REFERENCE_OF(cpte), 4, X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ, cpde));
    if (!_IsPageEntryPresent(cpte))
    {
        CPU_TRACE_BLOCK_BEGIN("!PageTabEntryPresent");
        cpu_state.data.cr2 = linear;
        CPU_TRACE_CHECK_RETURN(_SetExcept_PF(_MakePageFaultErrorCode(0, write, (vpl == 3))));
        return;
        CPU_TRACE_BLOCK_END;
    }
    if (vpl == 0x03 || (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        (cpu_state.data.cr0 & VCPU_CR0_WP) != 0u))
    {
        CPU_TRACE_BLOCK_BEGIN("PageTabPermission");
        if (vpl == 0x03 && !_GetPageEntry_US(cpte))
        {
            CPU_TRACE_BLOCK_BEGIN("PageTabEntry_US(0)");
            cpu_state.data.cr2 = linear;
            CPU_TRACE_CHECK_RETURN(_SetExcept_PF(_MakePageFaultErrorCode(1, write, 1)));
            return;
            CPU_TRACE_BLOCK_END;
        }
        if (write && !_IsPageEntryWritable(cpte))
        {
            CPU_TRACE_BLOCK_BEGIN("write,!PageTabEntryWritable");
            cpu_state.data.cr2 = linear;
            CPU_TRACE_CHECK_RETURN(_SetExcept_PF(_MakePageFaultErrorCode(1, 1, vpl == 3)));
            return;
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
    }
    translation->physical = _GetPageEntry_Base(cpte) + _GetLinear_Offset(linear);
    translation->ppde = ppde;
    translation->ppte = ppte;
    translation->cpde = cpde;
    translation->cpte = cpte;
    translation->paging = LIB_TRUE;
    translation->attributes = (cpte & 0x58u) | ((cpde & cpte) & 6u);
    CPU_TRACE_CALL_END;
}

/* Publish A/D only after the complete checked access has been preflighted. */
static void _kma_commit_physical_linear(x86_core_cpu_execution_context *context, t_kma_linear_translation *translation, lib_u8 write)
{
    CPU_TRACE_CALL_BEGIN("_kma_commit_physical_linear");
    if (context->preview_mode || !translation->paging)
    {
        CPU_TRACE_CALL_END;
        return;
    }
    if (translation->tlb_hit)
    {
        _kma_tlb_fill(context, translation->linear, translation->physical,
            translation->attributes);
        CPU_TRACE_CALL_END;
        return;
    }
    if (!translation->pde_accessed) {
        _SetPageEntry_A(translation->cpde);
        CPU_TRACE_CHECK_RETURN(_kma_write_physical(context, translation->ppde,
            X86_CPU_REFERENCE_OF(translation->cpde), 4,
            X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE, cpu_state.data.cr3));
    }
    const lib_u32 required = write ? 0x60u : 0x20u;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        (translation->cpte & required) != required) {
        CPU_TRACE_CHECK_RETURN(_kma_update_page_entry(context, translation, LIB_FALSE, write));
        translation->physical = _GetPageEntry_Base(translation->cpte) +
            _GetLinear_Offset(translation->linear);
        translation->attributes = (translation->cpte & 0x58u) |
            ((translation->cpde & translation->cpte) & 6u);
    } else if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX) {
        _SetPageEntry_A(translation->cpte);
        if (write)
            _SetPageEntry_D(translation->cpte);
        CPU_TRACE_CHECK_RETURN(_kma_write_physical(context, translation->ppte,
            X86_CPU_REFERENCE_OF(translation->cpte), 4, X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE, translation->cpde));
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) {
        _kma_tlb_fill(context, translation->linear, translation->physical,
            translation->attributes | (write ? 0x40u : 0u));
        ++instruction_state.data.timing_tlb_walks[translation->timing_update_attempts];
    }
    CPU_TRACE_CALL_END;
}

/* Translate and publish a single checked linear page access. */
static lib_u32 _kma_physical_linear(x86_core_cpu_execution_context *context, lib_u32 linear, lib_u8 byte, lib_u8 write, lib_u8 vpl, lib_u32 *out_attributes)
{
    t_kma_linear_translation translation;

    CPU_TRACE_CALL_BEGIN("_kma_physical_linear");
    CPU_TRACE_CHECK_RETURN_ZERO(_kma_prepare_physical_linear(context, linear,
        byte, write, vpl, &translation));
    CPU_TRACE_CHECK_RETURN_ZERO(_kma_commit_physical_linear(context,
        &translation, write));
    if (out_attributes != LIB_NULL) *out_attributes = translation.attributes;
    CPU_TRACE_CALL_END;
    return translation.physical;
}
/* translate logical to linear - segmentation mechanism */
static lib_u32 _kma_linear_logical(x86_core_cpu_execution_context *context, t_cpu_data_sreg *rsreg, lib_u32 offset, lib_u8 byte, lib_u8 write, lib_u8 vpl, lib_u8 force)
{
    lib_u32 linear;
    lib_u32 upper;
    lib_u64 lower;
    CPU_TRACE_CALL_BEGIN("_kma_linear_logical");
    switch (rsreg->sregtype)
    {
    case SREG_CODE:
        CPU_TRACE_BLOCK_BEGIN("sregtype(SREG_CODE)");
        if (!rsreg->flagValid)
            CPU_TRACE_IMPOSSIBLE_RETURN_ZERO;
        if (_IsProtected)
        {
            CPU_TRACE_BLOCK_BEGIN("Protected");
            if (!force)
            {
                CPU_TRACE_BLOCK_BEGIN("force");
                if (write)
                {
                    CPU_TRACE_BLOCK_BEGIN("write");
                    CPU_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
                    CPU_TRACE_BLOCK_END;
                }
                if (!rsreg->seg.exec.readable)
                {
                    CPU_TRACE_BLOCK_BEGIN("!readable");
                    CPU_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
                    CPU_TRACE_BLOCK_END;
                }
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_BLOCK_END;
        }
        lower = 0x00000000;
        upper = rsreg->limit;
        CPU_TRACE_BLOCK_END;
        break;
    case SREG_STACK:
        CPU_TRACE_BLOCK_BEGIN("sregtype(SREG_STACK)");
        if (!rsreg->flagValid)
            CPU_TRACE_IMPOSSIBLE_RETURN_ZERO;
        if (_IsProtected)
        {
            CPU_TRACE_BLOCK_BEGIN("Protected");
            if (rsreg->seg.executable || !rsreg->seg.data.writable)
                CPU_TRACE_IMPOSSIBLE_RETURN_ZERO;
            CPU_TRACE_BLOCK_END;
        }
        if (rsreg->seg.data.expdown)
        {
            lower = (lib_u64)rsreg->limit + 1u;
            upper = rsreg->seg.data.big ? 0xffffffff : 0x0000ffff;
        }
        else
        {
            lower = 0x00000000;
            upper = rsreg->limit;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case SREG_DATA:
        CPU_TRACE_BLOCK_BEGIN("sregtype(SREG_DATA)");
        if (!rsreg->flagValid)
        {
            CPU_TRACE_BLOCK_BEGIN("selector(null)");
            CPU_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
            CPU_TRACE_BLOCK_END;
        }
        if (_IsProtected)
        {
            CPU_TRACE_BLOCK_BEGIN("Protected");
            if (_IsSelectorNull(rsreg->selector))
            {
                CPU_TRACE_BLOCK_BEGIN("selector(null)");
                CPU_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
                CPU_TRACE_BLOCK_END;
            }
            if (rsreg->seg.executable && !rsreg->seg.exec.readable)
                CPU_TRACE_IMPOSSIBLE_RETURN_ZERO;
            if (!force)
            {
                CPU_TRACE_BLOCK_BEGIN("force");
                if (write)
                {
                    CPU_TRACE_BLOCK_BEGIN("write");
                    if (rsreg->seg.executable)
                    {
                        CPU_TRACE_BLOCK_BEGIN("executable");
                        CPU_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
                        CPU_TRACE_BLOCK_END;
                    }
                    else
                    {
                        CPU_TRACE_BLOCK_BEGIN("!executable");
                        if (!rsreg->seg.data.writable)
                        {
                            CPU_TRACE_BLOCK_BEGIN("!writable");
                            CPU_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
                            CPU_TRACE_BLOCK_END;
                        }
                        CPU_TRACE_BLOCK_END;
                    }
                    CPU_TRACE_BLOCK_END;
                }
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_BLOCK_END;
        }
        if (rsreg->seg.data.expdown)
        {
            lower = (lib_u64)rsreg->limit + 1u;
            upper = rsreg->seg.data.big ? 0xffffffff : 0x0000ffff;
        }
        else
        {
            lower = 0x00000000;
            upper = rsreg->limit;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case SREG_GDTR:
        CPU_TRACE_BLOCK_BEGIN("sregtype(SREG_GDTR)");
        if (!_GetCR0_PE)
            CPU_TRACE_IMPOSSIBLE_RETURN_ZERO;
        lower = 0x00000000;
        upper = rsreg->limit;
        CPU_TRACE_BLOCK_END;
        break;
    case SREG_IDTR:
        CPU_TRACE_BLOCK_BEGIN("sregtype(SREG_IDTR)");
        lower = 0x00000000;
        upper = !cpu_profile_has_protected_architecture(context->cpu_profile) ?
            0x03ffu : rsreg->limit;
        CPU_TRACE_BLOCK_END;
        break;
    case SREG_LDTR:
        CPU_TRACE_BLOCK_BEGIN("sregtype(SREG_LDTR)");
        if (!_GetCR0_PE)
            CPU_TRACE_IMPOSSIBLE_RETURN_ZERO;
        if (_GetSelector_TI(rsreg->selector))
            CPU_TRACE_IMPOSSIBLE_RETURN_ZERO;
        if (!rsreg->flagValid || _IsSelectorNull(rsreg->selector))
        {
            CPU_TRACE_BLOCK_BEGIN("selector(null)");
            CPU_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
            CPU_TRACE_BLOCK_END;
        }
        lower = 0x00000000;
        upper = rsreg->limit;
        CPU_TRACE_BLOCK_END;
        break;
    case SREG_TR:
        CPU_TRACE_BLOCK_BEGIN("sregtype(SREG_TR)");
        if (!_GetCR0_PE)
            CPU_TRACE_IMPOSSIBLE_RETURN_ZERO;
        if (!rsreg->flagValid || _IsSelectorNull(rsreg->selector))
            CPU_TRACE_IMPOSSIBLE_RETURN_ZERO;
        if (_GetSelector_TI(rsreg->selector))
            CPU_TRACE_IMPOSSIBLE_RETURN_ZERO;
        lower = 0x00000000;
        upper = rsreg->limit;
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN_ZERO;
    }
    linear = (rsreg->sregtype == SREG_IDTR &&
        !cpu_profile_has_protected_architecture(context->cpu_profile) ? 0u : rsreg->base) + offset;
    if (offset < lower || offset > upper || (byte != 0u &&
        (lib_u32)(byte - 1u) > upper - offset))
    {
        CPU_TRACE_BLOCK_BEGIN("offset(<lower/>upper)");
        switch (rsreg->sregtype)
        {
        case SREG_STACK:
            CPU_TRACE_BLOCK_BEGIN("sregtype(SREG_STACK)");
            if (!_IsProtected &&
                context->cpu_profile == X86_CORE_CPU_PROFILE_80286)
                CPU_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
            else CPU_TRACE_CHECK_RETURN_ZERO(_SetExcept_SS(0));
            CPU_TRACE_BLOCK_END;
            break;
        case SREG_TR:
            CPU_TRACE_BLOCK_BEGIN("sregtype(SREG_TR)");
            CPU_TRACE_CHECK_RETURN_ZERO(_SetExcept_TS(rsreg->selector & 0xfffcu));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_BLOCK_BEGIN("sregtype(default)");
            CPU_TRACE_CHECK_RETURN_ZERO(_SetExcept_GP(0));
            CPU_TRACE_BLOCK_END;
            break;
        }
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
    return linear;
}
/* read content from logical */
static void _kma_read_linear(x86_core_cpu_execution_context *context, lib_u32 linear, lib_uptr rdata, lib_u8 byte, lib_u8 vpl, lib_u8 force)
{
    const lib_u32 misses = instruction_state.data.timing_data_misses;
    lib_u32 phy1, phy2;
    lib_u32 attributes1, attributes2;
    lib_u8 byte1, byte2;
    CPU_TRACE_CALL_BEGIN("_kma_read_logical");
    if (_GetLinear_Offset(linear) > X86_CPU_MASK_U32(_GetPageSize - byte))
    {
        CPU_TRACE_BLOCK_BEGIN("Linear_Offset(>PageSize)");
        byte1 = _GetPageSize - _GetLinear_Offset(linear);
        byte2 = byte - byte1;
        {
            t_kma_linear_translation translation1, translation2;

            CPU_TRACE_CHECK_RETURN(_kma_prepare_physical_linear(context,
                linear, byte1, 0, vpl, &translation1));
            CPU_TRACE_CHECK_RETURN(_kma_prepare_physical_linear(context,
                linear + byte1, byte2, 0, vpl, &translation2));
            CPU_TRACE_CHECK_RETURN(_kma_commit_physical_linear(context,
                &translation1, 0));
            CPU_TRACE_CHECK_RETURN(_kma_commit_physical_linear(context,
                &translation2, 0));
            phy1 = translation1.physical;
            phy2 = translation2.physical;
            attributes1 = translation1.attributes;
            attributes2 = translation2.attributes;
        }
        CPU_TRACE_CHECK_RETURN(_kma_read_physical(context, phy1, rdata, byte1, context->memory_access_provenance, attributes1));
        CPU_TRACE_CHECK_RETURN(_kma_read_physical(context, phy2, rdata + byte1, byte2, context->memory_access_provenance, attributes2));
        CPU_TRACE_BLOCK_END;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("Linear_Offset(<=PageSize)");
        byte1 = byte;
        CPU_TRACE_CHECK_RETURN(phy1 = _kma_physical_linear(context, linear, byte1, 0, vpl, &attributes1));
        CPU_TRACE_CHECK_RETURN(_kma_read_physical(context, phy1, rdata, byte1, context->memory_access_provenance, attributes1));
        CPU_TRACE_BLOCK_END;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        !context->preview_mode && context->memory_access_provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA) {
        if (instruction_state.data.timing_data_misses != misses)
            ++instruction_state.data.timing_missed_reads;
        if ((byte == 2u || byte == 4u) && (linear & (byte - 1u)) != 0u)
            ++instruction_state.data.timing_unaligned_accesses;
    }
    CPU_TRACE_CALL_END;
}
/* write content to logical */
static void _kma_write_linear(x86_core_cpu_execution_context *context, lib_u32 linear, lib_uptr rdata, lib_u8 byte, lib_u8 vpl, lib_u8 force)
{
    lib_u32 phy1, phy2;
    lib_u32 attributes1, attributes2;
    lib_u8 byte1, byte2;
    CPU_TRACE_CALL_BEGIN("_kma_write_linear");
    if (_GetLinear_Offset(linear) > X86_CPU_MASK_U32(_GetPageSize - byte))
    {
        CPU_TRACE_BLOCK_BEGIN("Linear_Offset(>PageSize)");
        byte1 = _GetPageSize - _GetLinear_Offset(linear);
        byte2 = byte - byte1;
        {
            t_kma_linear_translation translation1, translation2;

            CPU_TRACE_CHECK_RETURN(_kma_prepare_physical_linear(context,
                linear, byte1, 1, vpl, &translation1));
            CPU_TRACE_CHECK_RETURN(_kma_prepare_physical_linear(context,
                linear + byte1, byte2, 1, vpl, &translation2));
            CPU_TRACE_CHECK_RETURN(_kma_commit_physical_linear(context,
                &translation1, 1));
            CPU_TRACE_CHECK_RETURN(_kma_commit_physical_linear(context,
                &translation2, 1));
            phy1 = translation1.physical;
            phy2 = translation2.physical;
            attributes1 = translation1.attributes;
            attributes2 = translation2.attributes;
        }
        CPU_TRACE_CHECK_RETURN(_kma_write_physical(context, phy1, rdata, byte1, context->memory_access_provenance, attributes1));
        CPU_TRACE_CHECK_RETURN(_kma_write_physical(context, phy2, rdata + byte1, byte2, context->memory_access_provenance, attributes2));
        CPU_TRACE_BLOCK_END;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("Linear_Offset(<=PageSize)");
        byte1 = byte;
        CPU_TRACE_CHECK_RETURN(phy1 = _kma_physical_linear(context, linear, byte1, 1, vpl, &attributes1));
        CPU_TRACE_CHECK_RETURN(_kma_write_physical(context, phy1, rdata, byte1, context->memory_access_provenance, attributes1));
        CPU_TRACE_BLOCK_END;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        !context->preview_mode && context->memory_access_provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA &&
        (byte == 2u || byte == 4u) && (linear & (byte - 1u)) != 0u)
        ++instruction_state.data.timing_unaligned_accesses;
    CPU_TRACE_CALL_END;
}
static lib_u8 _kma_real_legacy_segment_wrap(
    x86_core_cpu_execution_context *context, lib_u32 offset,
    lib_u8 byte)
{
    return context != LIB_NULL && !_GetCR0_PE &&
        (context->cpu_profile == X86_CORE_CPU_PROFILE_8086 ||
         context->cpu_profile == X86_CORE_CPU_PROFILE_8088 ||
         context->cpu_profile == X86_CORE_CPU_PROFILE_80186) &&
        offset <= 0x0000ffffu && byte != 0u &&
        (lib_u32)(byte - 1u) > 0x0000ffffu - offset;
}
/* Intel486 PRM 9.4/9.9.16: segment checks precede #AC, which precedes
 * operand paging. Implicit supervisor references and fetches are exempt. */
static void _kma_check_alignment(x86_core_cpu_execution_context *context,
    lib_u32 linear, lib_u8 byte, lib_u8 vpl, lib_u8 force)
{
    CPU_TRACE_CALL_BEGIN("_kma_check_alignment");
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        _GetCR0_PE && !force && vpl == 3u &&
        (cpu_state.data.eflags & VCPU_EFLAGS_AC) != 0u &&
        (cpu_state.data.cr0 & VCPU_CR0_AM) != 0u &&
        (byte == 2u || byte == 4u || byte == 8u) &&
        (linear & (byte - 1u)) != 0u)
        CPU_TRACE_CHECK_RETURN(_SetExcept_AC(0));
    CPU_TRACE_CALL_END;
}
/* read content from logical */
static void _kma_read_logical(x86_core_cpu_execution_context *context, t_cpu_data_sreg *rsreg, lib_u32 offset, lib_uptr rdata, lib_u8 byte, lib_u8 vpl, lib_u8 force)
{
    /* lib_uptr i; */
    lib_u32 linear;
    CPU_TRACE_CALL_BEGIN("_kma_read_logical");
    if (_kma_real_legacy_segment_wrap(context, offset, byte)) {
        const lib_u8 first = (lib_u8)(0x00010000u - offset);

        CPU_TRACE_CHECK_RETURN(_kma_read_logical(context, rsreg, offset,
            rdata, first, vpl, force));
        CPU_TRACE_CHECK_RETURN(_kma_read_logical(context, rsreg, 0u,
            rdata + first, byte - first, vpl, force));
        CPU_TRACE_CALL_END;
        return;
    }
    CPU_TRACE_CHECK_RETURN(linear = _kma_linear_logical(context, rsreg, offset, byte, 0, vpl, force));
    CPU_TRACE_CHECK_RETURN(_kma_check_alignment(context, linear, byte, vpl, force));
    CPU_TRACE_CHECK_RETURN(_kma_read_linear(context, linear, rdata, byte, vpl, force));
    if (!force && instruction_state.data.msize <
        X86_CORE_CPU_INSTRUCTION_MEMORY_ACCESS_CAPACITY)
    {
        CPU_TRACE_BLOCK_BEGIN("!force");
        instruction_state.data.mem[instruction_state.data.msize].flagWrite = LIB_FALSE;
        instruction_state.data.mem[instruction_state.data.msize].data = 0;
        lib_memory_copy((void *)X86_CPU_REFERENCE_OF(instruction_state.data.mem[instruction_state.data.msize].data), (void *)rdata, byte);
        instruction_state.data.mem[instruction_state.data.msize].byte = byte;
        instruction_state.data.mem[instruction_state.data.msize].linear = linear;
        if (instruction_state.data.flagWR)
        {
            if (instruction_state.data.wrLinear >= instruction_state.data.mem[instruction_state.data.msize].linear &&
                instruction_state.data.wrLinear < instruction_state.data.mem[instruction_state.data.msize].linear + byte)
            {
                _debug_record_watchpoint(context, X86_CORE_CPU_WATCH_READ,
                    instruction_state.data.wrLinear);
            }
        }
        /* for (i = 0;i < instruction_state.data.msize;++i) {
            if (instruction_state.data.mem[i].flagWrite == instruction_state.data.mem[instruction_state.data.msize].flagWrite &&
                instruction_state.data.mem[i].linear == instruction_state.data.mem[instruction_state.data.msize].linear) {
                CPU_TRACE_BLOCK_BEGIN("mem(same)");
                CPU_TRACE_IMPOSSIBLE_RETURN_ZERO;
                CPU_TRACE_CALL_END;
            }
        } */
        instruction_state.data.msize++;
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
/* write content to logical */
static void _kma_write_logical(x86_core_cpu_execution_context *context, t_cpu_data_sreg *rsreg, lib_u32 offset, lib_uptr rdata, lib_u8 byte, lib_u8 vpl, lib_u8 force)
{
    /* lib_uptr i; */
    lib_u32 linear;
    CPU_TRACE_CALL_BEGIN("_kma_write_logical");
    if (_kma_real_legacy_segment_wrap(context, offset, byte)) {
        const lib_u8 first = (lib_u8)(0x00010000u - offset);

        CPU_TRACE_CHECK_RETURN(_kma_write_logical(context, rsreg, offset,
            rdata, first, vpl, force));
        CPU_TRACE_CHECK_RETURN(_kma_write_logical(context, rsreg, 0u,
            rdata + first, byte - first, vpl, force));
        CPU_TRACE_CALL_END;
        return;
    }
    CPU_TRACE_CHECK_RETURN(linear = _kma_linear_logical(context, rsreg, offset, byte, 1, vpl, force));
    CPU_TRACE_CHECK_RETURN(_kma_check_alignment(context, linear, byte, vpl, force));
    CPU_TRACE_CHECK_RETURN(_kma_write_linear(context, linear, rdata, byte, vpl, force));
    if (!force && instruction_state.data.msize <
        X86_CORE_CPU_INSTRUCTION_MEMORY_ACCESS_CAPACITY)
    {
        CPU_TRACE_BLOCK_BEGIN("!force");
        instruction_state.data.mem[instruction_state.data.msize].flagWrite = LIB_TRUE;
        instruction_state.data.mem[instruction_state.data.msize].data = 0;
        lib_memory_copy((void *)X86_CPU_REFERENCE_OF(instruction_state.data.mem[instruction_state.data.msize].data), (void *)rdata, byte);
        instruction_state.data.mem[instruction_state.data.msize].byte = byte;
        instruction_state.data.mem[instruction_state.data.msize].linear = linear;
        if (instruction_state.data.flagWW)
        {
            if (instruction_state.data.wwLinear >= instruction_state.data.mem[instruction_state.data.msize].linear &&
                instruction_state.data.wwLinear < instruction_state.data.mem[instruction_state.data.msize].linear + byte)
            {
                _debug_record_watchpoint(context, X86_CORE_CPU_WATCH_WRITE,
                    instruction_state.data.wwLinear);
            }
        }
        /* for (i = 0;i < instruction_state.data.msize;++i) {
            if (instruction_state.data.mem[i].flagWrite == instruction_state.data.mem[instruction_state.data.msize].flagWrite &&
                instruction_state.data.mem[i].linear == instruction_state.data.mem[instruction_state.data.msize].linear) {
                CPU_TRACE_BLOCK_BEGIN("mem(same)");
                CPU_TRACE_IMPOSSIBLE_RETURN;
                CPU_TRACE_CALL_END;
            }
        } */
        instruction_state.data.msize++;
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
/* test logical accessing */
static void _kma_test_linear(x86_core_cpu_execution_context *context, lib_u32 linear, lib_u8 byte, lib_u8 write, lib_u8 vpl, lib_u8 force)
{
    lib_u32 phy1, phy2;
    lib_u8 byte1, byte2;
    CPU_TRACE_CALL_BEGIN("_kma_test_linear");
    if (_GetLinear_Offset(linear) > X86_CPU_MASK_U32(_GetPageSize - byte))
    {
        CPU_TRACE_BLOCK_BEGIN("Linear_Offset(>PageSize)");
        byte1 = _GetPageSize - _GetLinear_Offset(linear);
        byte2 = byte - byte1;
        {
            t_kma_linear_translation translation1, translation2;

            CPU_TRACE_CHECK_RETURN(_kma_prepare_physical_linear(context,
                linear, byte1, write, vpl, &translation1));
            CPU_TRACE_CHECK_RETURN(_kma_prepare_physical_linear(context,
                linear + byte1, byte2, write, vpl, &translation2));
            CPU_TRACE_CHECK_RETURN(_kma_commit_physical_linear(context,
                &translation1, write));
            CPU_TRACE_CHECK_RETURN(_kma_commit_physical_linear(context,
                &translation2, write));
            phy1 = translation1.physical;
            phy2 = translation2.physical;
        }
        CPU_TRACE_BLOCK_END;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("Linear_Offset(<=PageSize)");
        byte1 = byte;
        CPU_TRACE_CHECK_RETURN(phy1 = _kma_physical_linear(context, linear, byte1, write, vpl, LIB_NULL));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
static void _kma_test_logical(x86_core_cpu_execution_context *context, t_cpu_data_sreg *rsreg, lib_u32 offset, lib_u8 byte, lib_u8 write, lib_u8 vpl, lib_u8 force)
{
    lib_u32 linear;
    CPU_TRACE_CALL_BEGIN("_kma_test_logical");
    if (_kma_real_legacy_segment_wrap(context, offset, byte)) {
        const lib_u8 first = (lib_u8)(0x00010000u - offset);

        CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, rsreg, offset,
            first, write, vpl, force));
        CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, rsreg, 0u,
            byte - first, write, vpl, force));
        CPU_TRACE_CALL_END;
        return;
    }
    CPU_TRACE_CHECK_RETURN(linear = _kma_linear_logical(context, rsreg, offset, byte, write, vpl, force));
    CPU_TRACE_CALL_END;
}
static void _kma_test_access(x86_core_cpu_execution_context *context, t_cpu_data_sreg *rsreg, lib_u32 offset, lib_u8 byte, lib_u8 write, lib_u8 vpl, lib_u8 force)
{
    lib_u32 linear;
    CPU_TRACE_CALL_BEGIN("_kma_test_access");
    if (_kma_real_legacy_segment_wrap(context, offset, byte)) {
        const lib_u8 first = (lib_u8)(0x00010000u - offset);

        CPU_TRACE_CHECK_RETURN(_kma_test_access(context, rsreg, offset,
            first, write, vpl, force));
        CPU_TRACE_CHECK_RETURN(_kma_test_access(context, rsreg, 0u,
            byte - first, write, vpl, force));
        CPU_TRACE_CALL_END;
        return;
    }
    CPU_TRACE_CHECK_RETURN(linear = _kma_linear_logical(context, rsreg, offset, byte, write, vpl, force));
    CPU_TRACE_CHECK_RETURN(_kma_check_alignment(context, linear, byte, vpl, force));
    CPU_TRACE_CHECK_RETURN(_kma_test_linear(context, linear, byte, write, vpl, force));
    CPU_TRACE_CALL_END;
}

/* general memory accessing */
static void _m_read_logical(x86_core_cpu_execution_context *context, t_cpu_data_sreg *rsreg, lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_m_read_logical");
    CPU_TRACE_CHECK_RETURN(_kma_read_logical(context, rsreg, offset, rdata, byte, _GetCPL, 0));
    (void)_kma_gpr_destination(context, rdata);
    CPU_TRACE_CALL_END;
}
static void _m_write_logical(x86_core_cpu_execution_context *context, t_cpu_data_sreg *rsreg, lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_m_write_logical");
    CPU_TRACE_CHECK_RETURN(_kma_write_logical(context, rsreg, offset, rdata, byte, _GetCPL, 0));
    CPU_TRACE_CALL_END;
}
static void _m_test_access(x86_core_cpu_execution_context *context, t_cpu_data_sreg *rsreg, lib_u32 offset, lib_u8 byte, lib_u8 write)
{
    lib_uptr ref = 0;
    CPU_TRACE_CALL_BEGIN("_m_test_access");
    CPU_TRACE_CHECK_RETURN(_kma_test_access(context, rsreg, offset, byte, write, _GetCPL, 0));
    CPU_TRACE_CALL_END;
}
static void _m_test_logical(x86_core_cpu_execution_context *context, t_cpu_data_sreg *rsreg, lib_u32 offset, lib_u8 byte, lib_u8 write)
{
    lib_uptr ref = 0;
    CPU_TRACE_CALL_BEGIN("_m_test_logical");
    CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, rsreg, offset, byte, write, _GetCPL, 0));
    CPU_TRACE_CALL_END;
}

static void _m_read_rm(x86_core_cpu_execution_context *context, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_m_read_rm");
    instruction_state.data.crm = 0;
    if (instruction_state.data.flagMem)
        CPU_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.mrm.rsreg, instruction_state.data.mrm.offset, X86_CPU_REFERENCE_OF(instruction_state.data.crm), byte));
    else
        CPU_TRACE_CHECK_RETURN(_kma_read_ref(context, instruction_state.data.rrm, X86_CPU_REFERENCE_OF(instruction_state.data.crm), byte));
    CPU_TRACE_CALL_END;
}
/* A modifying operand must pass write checks before its first data read.
 * Register forms retain the ordinary register read and need no memory probe. */
static void _m_read_rmw(x86_core_cpu_execution_context *context, lib_u8 byte,
    lib_bool implicit_lock)
{
    CPU_TRACE_CALL_BEGIN("_m_read_rmw");
    if (instruction_state.data.flagMem) {
        CPU_TRACE_CHECK_RETURN(_m_test_access(context,
            instruction_state.data.mrm.rsreg, instruction_state.data.mrm.offset,
            byte, LIB_TRUE));
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
            (implicit_lock || instruction_state.data.flagLock))
            CPU_TRACE_CHECK_RETURN(_kma_begin_memory_lock(context));
    }
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, byte));
    CPU_TRACE_CALL_END;
}
/* Validate the whole logical operand before either scalar transfer. Keep the
 * original bus phases and the sole early-family segment-wrap policy. */
static void _m_read_pair(x86_core_cpu_execution_context *context,
    lib_u8 first, lib_u8 second)
{
    const lib_u32 misses = instruction_state.data.timing_data_misses;
    lib_u64 low;
    const lib_bool wraps = _kma_real_legacy_segment_wrap(context,
        instruction_state.data.mrm.offset, first + second);
    CPU_TRACE_CALL_BEGIN("_m_read_pair");
    CPU_TRACE_CHECK_RETURN(_m_test_logical(context,
        instruction_state.data.mrm.rsreg, instruction_state.data.mrm.offset,
        first + second, 0));
    CPU_TRACE_CHECK_RETURN(_kma_check_alignment(context,
        instruction_state.data.mrm.rsreg->base + instruction_state.data.mrm.offset,
        first, _GetCPL, 0u));
    CPU_TRACE_CHECK_RETURN(_kma_check_alignment(context,
        instruction_state.data.mrm.rsreg->base + instruction_state.data.mrm.offset + first,
        second, _GetCPL, 0u));
    if (!wraps)
        CPU_TRACE_CHECK_RETURN(_kma_test_linear(context,
            instruction_state.data.mrm.rsreg->base + instruction_state.data.mrm.offset,
            first + second, 0u, _GetCPL, 0u));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, first));
    low = instruction_state.data.crm;
    instruction_state.data.mrm.offset += first;
    if (wraps)
        instruction_state.data.mrm.offset =
            X86_CPU_MASK_U16(instruction_state.data.mrm.offset);
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, second));
    instruction_state.data.crm = low | (instruction_state.data.crm << (first * 8u));
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX && !context->preview_mode)
        instruction_state.data.timing_pair_misses += instruction_state.data.timing_data_misses - misses;
    CPU_TRACE_CALL_END;
}
static void _m_write_rm(x86_core_cpu_execution_context *context, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_m_write_rm");
    if (instruction_state.data.flagMem)
        CPU_TRACE_CHECK_RETURN(_m_write_logical(context, instruction_state.data.mrm.rsreg, instruction_state.data.mrm.offset, X86_CPU_REFERENCE_OF(instruction_state.data.crm), byte));
    else
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rrm, X86_CPU_REFERENCE_OF(instruction_state.data.crm), byte));
    CPU_TRACE_CALL_END;
}

/* A masked-zero 486 shift checks its destination but performs no write. */
static void _m_write_shift(x86_core_cpu_execution_context *context,
    lib_u8 byte, lib_u8 count)
{
    CPU_TRACE_CALL_BEGIN("_m_write_shift");
    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX ||
        (count & 31u) != 0u)
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, byte));
    CPU_TRACE_CALL_END;
}

/* segment accessing unit: _s_ */
/* kernel segment accessing */
/* Intel486 PRM 5.2.5 places GDT/IDT in linear memory. Earlier profiles retain
 * their existing table-access contract pending their own qualification. */
static void _ksa_read_table(x86_core_cpu_execution_context *context,
    t_cpu_data_sreg *rtable, lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    const lib_u32 base = rtable->sregtype == SREG_IDTR &&
        !cpu_profile_has_protected_architecture(context->cpu_profile) ? 0u :
        rtable->base;

    CPU_TRACE_CALL_BEGIN("_ksa_read_table");
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
        CPU_TRACE_CHECK_RETURN(_kma_read_linear(context, base + offset,
            rdata, byte, 0u, 1u));
    else
        CPU_TRACE_CHECK_RETURN(_kma_read_physical(context, base + offset,
            rdata, byte, context->memory_access_provenance, 0u));
    CPU_TRACE_CALL_END;
}
static void _ksa_write_table(x86_core_cpu_execution_context *context,
    t_cpu_data_sreg *rtable, lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_ksa_write_table");
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
        CPU_TRACE_CHECK_RETURN(_kma_write_linear(context, rtable->base + offset,
            rdata, byte, 0u, 1u));
    else
        CPU_TRACE_CHECK_RETURN(_kma_write_physical(context, rtable->base + offset,
            rdata, byte, context->memory_access_provenance, 0u));
    CPU_TRACE_CALL_END;
}
static void _ksa_read_idt(x86_core_cpu_execution_context *context, lib_u8 intid, lib_uptr rdata)
{
    CPU_TRACE_CALL_BEGIN("_s_read_idt");
    if (!_GetCR0_PE)
    {
        CPU_TRACE_BLOCK_BEGIN("CR0_PE(0)");
        if (cpu_profile_has_protected_architecture(context->cpu_profile) &&
            X86_CPU_MASK_U16(intid * 4 + 3) > X86_CPU_MASK_U16(cpu_state.data.idtr.limit))
            CPU_TRACE_IMPOSSIBLE_RETURN;
        CPU_TRACE_CHECK_RETURN(_ksa_read_table(context,
            &cpu_state.data.idtr, intid * 4u, rdata, 4u));
        CPU_TRACE_BLOCK_END;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("CR0_PE(1)");
        if (X86_CPU_MASK_U16(intid * 8 + 7) > X86_CPU_MASK_U16(cpu_state.data.idtr.limit))
            CPU_TRACE_IMPOSSIBLE_RETURN;
        CPU_TRACE_CHECK_RETURN(_ksa_read_table(context,
            &cpu_state.data.idtr, intid * 8u, rdata, 8u));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
/* Called after the owning GDT/LDT route has checked its selector and limit. */
static void _ksa_read_descriptor(x86_core_cpu_execution_context *context,
    t_cpu_data_sreg *table, lib_u32 offset, lib_uptr rdata)
{
    const lib_u32 misses = instruction_state.data.timing_data_misses;
    const lib_u32 reads = instruction_state.data.timing_missed_reads;

    if (table->sregtype == SREG_LDTR)
        _kma_read_logical(context, table, offset, rdata, 8u, 0u, 1u);
    else
        _ksa_read_table(context, table, offset, rdata, 8u);
    if (!context->preview_mode) {
        instruction_state.data.timing_descriptor_misses += instruction_state.data.timing_data_misses - misses;
        instruction_state.data.timing_missed_descriptors += instruction_state.data.timing_missed_reads - reads;
        if (instruction_state.data.timing_missed_reads != reads) {
            lib_u64 descriptor;

            lib_memory_copy(&descriptor, (const void *)rdata, sizeof(descriptor));
            if (_IsDescTaskGate(descriptor))
                instruction_state.data.timing_task_gate_miss_mask |= 1u;
        }
    }
}

static void _ksa_read_ldt(x86_core_cpu_execution_context *context, lib_u16 selector, lib_uptr rdata)
{
    CPU_TRACE_CALL_BEGIN("_ksa_read_ldt");
    if (!_GetCR0_PE)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (!_GetSelector_TI(selector))
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (!cpu_state.data.ldtr.flagValid ||
        X86_CPU_MASK_U16(_GetSelector_Offset(selector) + 7) > X86_CPU_MASK_U16(cpu_state.data.ldtr.limit))
    {
        CPU_TRACE_BLOCK_BEGIN("!ldtr.flagValid/Selector_Offset(>ldtr.limit)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(selector & 0xfffcu));
        CPU_TRACE_BLOCK_END;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
        CPU_TRACE_CHECK_RETURN(_ksa_read_descriptor(context,
            &cpu_state.data.ldtr, _GetSelector_Offset(selector), rdata));
    else
        CPU_TRACE_CHECK_RETURN(_kma_read_logical(context, &cpu_state.data.ldtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
    CPU_TRACE_CALL_END;
}
static void _ksa_read_gdt(x86_core_cpu_execution_context *context, lib_u16 selector, lib_uptr rdata)
{
    CPU_TRACE_CALL_BEGIN("_ksa_read_gdt");
    if (!_GetCR0_PE)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (_GetSelector_TI(selector))
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (X86_CPU_MASK_U16(_GetSelector_Offset(selector) + 7) > X86_CPU_MASK_U16(cpu_state.data.gdtr.limit))
    {
        CPU_TRACE_BLOCK_BEGIN("Selector_Offset(>gdtr.limit)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(selector & 0xfffcu));
        CPU_TRACE_BLOCK_END;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
        CPU_TRACE_CHECK_RETURN(_ksa_read_descriptor(context,
            &cpu_state.data.gdtr, _GetSelector_Offset(selector), rdata));
    else
        CPU_TRACE_CHECK_RETURN(_ksa_read_table(context,
            &cpu_state.data.gdtr, _GetSelector_Offset(selector), rdata, 8u));
    CPU_TRACE_CALL_END;
}
static void _ksa_read_xdt(x86_core_cpu_execution_context *context, lib_u16 selector, lib_uptr rdata)
{
    CPU_TRACE_CALL_BEGIN("_ksa_read_xdt");
    if (!_GetCR0_PE)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (_GetSelector_TI(selector))
    {
        CPU_TRACE_BLOCK_BEGIN("Selector_TI");
        CPU_TRACE_CHECK_RETURN(_ksa_read_ldt(context, selector, rdata));
        CPU_TRACE_BLOCK_END;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("!Selector_TI");
        CPU_TRACE_CHECK_RETURN(_ksa_read_gdt(context, selector, rdata));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
static void _ksa_write_ldt(x86_core_cpu_execution_context *context, lib_u16 selector, lib_uptr rdata)
{
    CPU_TRACE_CALL_BEGIN("_ksa_write_ldt");
    if (!_GetCR0_PE)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (!_GetSelector_TI(selector))
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (X86_CPU_MASK_U16(_GetSelector_Offset(selector) + 7) > X86_CPU_MASK_U16(cpu_state.data.ldtr.limit))
        CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CHECK_RETURN(_kma_write_logical(context, &cpu_state.data.ldtr, _GetSelector_Offset(selector), rdata, 8, 0x00, 1));
    CPU_TRACE_CALL_END;
}
static void _ksa_write_gdt(x86_core_cpu_execution_context *context, lib_u16 selector, lib_uptr rdata)
{
    CPU_TRACE_CALL_BEGIN("_ksa_write_gdt");
    if (!_GetCR0_PE)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (_GetSelector_TI(selector))
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (X86_CPU_MASK_U16(_GetSelector_Offset(selector) + 7) > X86_CPU_MASK_U16(cpu_state.data.gdtr.limit))
        CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CHECK_RETURN(_ksa_write_table(context,
        &cpu_state.data.gdtr, _GetSelector_Offset(selector), rdata, 8u));
    CPU_TRACE_CALL_END;
}
static void _ksa_test_write_xdt(x86_core_cpu_execution_context *context,
    lib_u16 selector);
static void _ksa_write_xdt(x86_core_cpu_execution_context *context, lib_u16 selector, lib_uptr rdata)
{
    lib_u64 *descriptor = (lib_u64 *)rdata;
    const lib_bool unaccessed = _IsDescUser(*descriptor) && !_IsDescUserAccessed(*descriptor);

    CPU_TRACE_CALL_BEGIN("_ksa_write_xdt");
    if (!_GetCR0_PE)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    /* User descriptors are written only to set Accessed. System descriptors
     * use this route for distinct updates, including the TSS busy bit. */
    if (_IsDescUser(*descriptor))
    {
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
            _IsDescUserAccessed(*descriptor))
        {
            CPU_TRACE_CALL_END;
            return;
        }
        _SetDescUserAccessed(*descriptor);
    }
    if (context->preview_mode)
    {
        CPU_TRACE_CHECK_RETURN(_ksa_test_write_xdt(context, selector));
        CPU_TRACE_CALL_END;
        return;
    }
    if (_GetSelector_TI(selector))
    {
        CPU_TRACE_BLOCK_BEGIN("Selector_TI");
        CPU_TRACE_CHECK_RETURN(_ksa_write_ldt(context, selector, rdata));
        CPU_TRACE_BLOCK_END;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("!Selector_TI");
        CPU_TRACE_CHECK_RETURN(_ksa_write_gdt(context, selector, rdata));
        CPU_TRACE_BLOCK_END;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX && unaccessed)
        ++instruction_state.data.timing_unaccessed_descriptors;
    CPU_TRACE_CALL_END;
}
static void _ksa_test_write_xdt(x86_core_cpu_execution_context *context,
    lib_u16 selector)
{
    CPU_TRACE_CALL_BEGIN("_ksa_test_write_xdt");
    if (!_GetCR0_PE)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (_GetSelector_TI(selector))
    {
        if (X86_CPU_MASK_U16(_GetSelector_Offset(selector) + 7) >
            X86_CPU_MASK_U16(cpu_state.data.ldtr.limit))
            CPU_TRACE_IMPOSSIBLE_RETURN;
        CPU_TRACE_CHECK_RETURN(_kma_test_access(context, &cpu_state.data.ldtr,
            _GetSelector_Offset(selector), 8, 1, 0x00, 1));
    }
    else
    {
        if (X86_CPU_MASK_U16(_GetSelector_Offset(selector) + 7) >
            X86_CPU_MASK_U16(cpu_state.data.gdtr.limit))
            CPU_TRACE_IMPOSSIBLE_RETURN;
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
            CPU_TRACE_CHECK_RETURN(_kma_test_access(context, &cpu_state.data.gdtr,
                _GetSelector_Offset(selector), 8, 1, 0x00, 1));
    }
    CPU_TRACE_CALL_END;
}
static void _ksa_prepare_code_sreg(x86_core_cpu_execution_context *context,
    lib_u16 selector, lib_u8 cpl, t_cpu_data_sreg *rsreg,
    lib_u64 descriptor)
{
    CPU_TRACE_CALL_BEGIN("_ksa_prepare_code_sreg");
    if (!_IsDescCode(descriptor))
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(selector & 0xfffcu));
    if (!_IsDescPresent(descriptor))
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(selector & 0xfffcu));
    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX ||
        !_IsDescUserAccessed(descriptor))
        CPU_TRACE_CHECK_RETURN(_ksa_test_write_xdt(context, selector));
    rsreg->flagValid = LIB_TRUE;
    rsreg->selector = (selector & ~VCPU_SELECTOR_RPL) | cpl;
    rsreg->sregtype = SREG_CODE;
    rsreg->base = (lib_u32)_GetDescSeg_Base(descriptor);
    rsreg->dpl = (lib_u8)_GetDesc_DPL(descriptor);
    rsreg->limit = (lib_u32)(_IsDescSegGranularLarge(descriptor) ?
        ((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) :
        _GetDescSeg_Limit(descriptor));
    rsreg->seg.accessed = LIB_TRUE;
    rsreg->seg.executable = (lib_u8)_IsDescUserExecutable(descriptor);
    rsreg->seg.exec.defsize = (lib_u8)_IsDescCode32(descriptor);
    rsreg->seg.exec.conform = (lib_u8)_IsDescCodeConform(descriptor);
    rsreg->seg.exec.readable = (lib_u8)_IsDescCodeReadable(descriptor);
    CPU_TRACE_CALL_END;
}
static void _ksa_prepare_stack_sreg(x86_core_cpu_execution_context *context,
    lib_u16 selector, lib_u8 cpl, t_cpu_data_sreg *rsreg,
    lib_u64 descriptor)
{
    CPU_TRACE_CALL_BEGIN("_ksa_prepare_stack_sreg");
    if (!_IsDescDataWritable(descriptor) || _GetSelector_RPL(selector) != cpl ||
        _GetDesc_DPL(descriptor) != cpl)
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(selector & 0xfffcu));
    if (!_IsDescPresent(descriptor))
        CPU_TRACE_CHECK_RETURN(_SetExcept_SS(selector & 0xfffcu));
    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX ||
        !_IsDescUserAccessed(descriptor))
        CPU_TRACE_CHECK_RETURN(_ksa_test_write_xdt(context, selector));
    rsreg->flagValid = LIB_TRUE;
    rsreg->selector = selector;
    rsreg->sregtype = SREG_STACK;
    rsreg->base = (lib_u32)_GetDescSeg_Base(descriptor);
    rsreg->dpl = (lib_u8)_GetDesc_DPL(descriptor);
    rsreg->limit = (lib_u32)(_IsDescSegGranularLarge(descriptor) ?
        ((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) :
        _GetDescSeg_Limit(descriptor));
    rsreg->seg.accessed = LIB_TRUE;
    rsreg->seg.executable = LIB_FALSE;
    rsreg->seg.data.big = (lib_u8)_IsDescDataBig(descriptor);
    rsreg->seg.data.expdown = (lib_u8)_IsDescDataExpDown(descriptor);
    rsreg->seg.data.writable = (lib_u8)_IsDescDataWritable(descriptor);
    CPU_TRACE_CALL_END;
}
/* Clearing PE deliberately retains protected caches.  A later real-address
 * segment reload, including the far transfer which reloads CS, replaces the
 * selected cache with real-address attributes. */
static void _ksa_load_real_sreg(x86_core_cpu_execution_context *context,
    t_cpu_data_sreg *rsreg,
    lib_u16 selector, lib_u8 vm86)
{
    rsreg->flagValid = LIB_TRUE;
    rsreg->selector = selector;
    rsreg->base = (lib_u32)selector << 4u;
    if (context->smm_active && !_GetCR0_PE) return;
    rsreg->limit = 0x0000ffffu;
    rsreg->dpl = vm86 ? 3u : 0u;
    rsreg->seg.accessed = LIB_TRUE;
    rsreg->seg.executable = rsreg->sregtype == SREG_CODE;
    if (rsreg->sregtype == SREG_CODE) {
        rsreg->seg.exec.defsize = LIB_FALSE;
        rsreg->seg.exec.conform = LIB_FALSE;
        rsreg->seg.exec.readable = LIB_TRUE;
    } else {
        rsreg->seg.data.big = LIB_FALSE;
        rsreg->seg.data.expdown = LIB_FALSE;
        rsreg->seg.data.writable = LIB_TRUE;
    }
}
static void _ksa_load_sreg(x86_core_cpu_execution_context *context, t_cpu_data_sreg *rsreg, lib_u16 selector)
{
    lib_u64 descriptor = 0u;
    CPU_TRACE_CALL_BEGIN("_ksa_load_sreg");
    switch (rsreg->sregtype)
    {
    case SREG_CODE:
        /* note: privilege checking not performed */
        CPU_TRACE_BLOCK_BEGIN("sregtype(SREG_CODE)");
        if (_IsProtected)
        {
            CPU_TRACE_BLOCK_BEGIN("Protected(1)");
            if (_IsSelectorNull(selector))
            {
                CPU_TRACE_BLOCK_BEGIN("selector(null)");
                CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector, X86_CPU_REFERENCE_OF(descriptor)));
            if (!_IsDescCode(descriptor))
            {
                CPU_TRACE_BLOCK_BEGIN("!DescCode");
                CPU_TRACE_CHECK_RETURN(_SetExcept_GP(selector & 0xfffcu));
                CPU_TRACE_BLOCK_END;
            }
            if (!_IsDescPresent(descriptor))
            {
                CPU_TRACE_BLOCK_BEGIN("!DescPresent");
                CPU_TRACE_CHECK_RETURN(_SetExcept_NP(selector & 0xfffcu));
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_CHECK_RETURN(_ksa_write_xdt(context, selector, X86_CPU_REFERENCE_OF(descriptor)));
            rsreg->flagValid = LIB_TRUE;
            rsreg->base = (lib_u32)_GetDescSeg_Base(descriptor);
            if (_IsDescCodeNonConform(descriptor))
                rsreg->dpl = (lib_u8)_GetDesc_DPL(descriptor);
            rsreg->limit = (lib_u32)((_IsDescSegGranularLarge(descriptor) ? ((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
            rsreg->seg.accessed = (lib_u8)_IsDescUserAccessed(descriptor);
            rsreg->seg.executable = (lib_u8)_IsDescUserExecutable(descriptor);
            rsreg->seg.exec.defsize = (lib_u8)_IsDescCode32(descriptor);
            rsreg->seg.exec.conform = (lib_u8)_IsDescCodeConform(descriptor);
            rsreg->seg.exec.readable = (lib_u8)_IsDescCodeReadable(descriptor);
            rsreg->selector = (selector & ~VCPU_SELECTOR_RPL) | _GetCPL;
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("!Protected");
            _ksa_load_real_sreg(context, rsreg, selector,
                _GetCR0_PE && _GetEFLAGS_VM);
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case SREG_DATA:
        CPU_TRACE_BLOCK_BEGIN("sregtype(SREG_DATA)");
        if (_IsProtected)
        {
            CPU_TRACE_BLOCK_BEGIN("Protected");
            if (_IsSelectorNull(selector))
            {
                CPU_TRACE_BLOCK_BEGIN("selector(null)");
                rsreg->flagValid = LIB_FALSE;
                rsreg->selector = selector;
                CPU_TRACE_BLOCK_END;
            }
            else
            {
                CPU_TRACE_BLOCK_BEGIN("selector(!null)");
                CPU_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector, X86_CPU_REFERENCE_OF(descriptor)));
                if (!_IsDescData(descriptor) && !_IsDescCodeReadable(descriptor))
                {
                    CPU_TRACE_BLOCK_BEGIN("!DescData,!DescCodeReadable");
                    CPU_TRACE_CHECK_RETURN(_SetExcept_GP(selector & 0xfffcu));
                    CPU_TRACE_BLOCK_END;
                }
                if (_IsDescData(descriptor) || _IsDescCodeNonConform(descriptor))
                {
                    CPU_TRACE_BLOCK_BEGIN("DescData/DescCodeNonConform)");
                    if (_GetSelector_RPL(selector) > _GetDesc_DPL(descriptor) ||
                        _GetCPL > _GetDesc_DPL(descriptor))
                    {
                        CPU_TRACE_BLOCK_BEGIN("PL(fail)");
                        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(selector & 0xfffcu));
                        CPU_TRACE_BLOCK_END;
                    }
                    CPU_TRACE_BLOCK_END;
                }
                if (!_IsDescPresent(descriptor))
                {
                    CPU_TRACE_BLOCK_BEGIN("!DescPresent");
                    CPU_TRACE_CHECK_RETURN(_SetExcept_NP(selector & 0xfffcu));
                    CPU_TRACE_BLOCK_END;
                }
                CPU_TRACE_CHECK_RETURN(_ksa_write_xdt(context, selector, X86_CPU_REFERENCE_OF(descriptor)));
                rsreg->flagValid = LIB_TRUE;
                rsreg->selector = selector;
                rsreg->base = (lib_u32)_GetDescSeg_Base(descriptor);
                rsreg->dpl = (lib_u8)_GetDesc_DPL(descriptor);
                rsreg->limit = (lib_u32)((_IsDescSegGranularLarge(descriptor) ? ((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
                rsreg->seg.accessed = (lib_u8)_IsDescUserAccessed(descriptor);
                rsreg->seg.executable = (lib_u8)_IsDescUserExecutable(descriptor);
                if (rsreg->seg.executable)
                {
                    rsreg->seg.exec.defsize = (lib_u8)_IsDescCode32(descriptor);
                    rsreg->seg.exec.conform = (lib_u8)_IsDescCodeConform(descriptor);
                    rsreg->seg.exec.readable = (lib_u8)_IsDescCodeReadable(descriptor);
                }
                else
                {
                    rsreg->seg.data.big = (lib_u8)_IsDescDataBig(descriptor);
                    rsreg->seg.data.expdown = (lib_u8)_IsDescDataExpDown(descriptor);
                    rsreg->seg.data.writable = (lib_u8)_IsDescDataWritable(descriptor);
                }
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("!Protected");
            _ksa_load_real_sreg(context, rsreg, selector,
                _GetCR0_PE && _GetEFLAGS_VM);
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case SREG_STACK:
        CPU_TRACE_BLOCK_BEGIN("sregtype(SREG_STACK)");
        if (_IsProtected)
        {
            CPU_TRACE_BLOCK_BEGIN("Protected(1)");
            if (_IsSelectorNull(selector))
            {
                CPU_TRACE_BLOCK_BEGIN("selector(null)");
                CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                CPU_TRACE_BLOCK_END;
            }
            if (_GetSelector_RPL(selector) != _GetCPL)
            {
                CPU_TRACE_BLOCK_BEGIN("Selector_RPL(!CPL)");
                CPU_TRACE_CHECK_RETURN(_SetExcept_GP(selector & 0xfffcu));
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector, X86_CPU_REFERENCE_OF(descriptor)));
            if (!_IsDescDataWritable(descriptor))
            {
                CPU_TRACE_BLOCK_BEGIN("!DescDataWritable");
                CPU_TRACE_CHECK_RETURN(_SetExcept_GP(selector & 0xfffcu));
                CPU_TRACE_BLOCK_END;
            }
            if (_GetDesc_DPL(descriptor) != _GetCPL)
            {
                CPU_TRACE_BLOCK_BEGIN("Desc_DPL(!CPL)");
                CPU_TRACE_CHECK_RETURN(_SetExcept_GP(selector & 0xfffcu));
                CPU_TRACE_BLOCK_END;
            }
            if (!_IsDescPresent(descriptor))
            {
                CPU_TRACE_BLOCK_BEGIN("!DescPresent");
                CPU_TRACE_CHECK_RETURN(_SetExcept_SS(selector & 0xfffcu));
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_CHECK_RETURN(_ksa_write_xdt(context, selector, X86_CPU_REFERENCE_OF(descriptor)));
            rsreg->flagValid = LIB_TRUE;
            rsreg->selector = selector;
            rsreg->base = (lib_u32)_GetDescSeg_Base(descriptor);
            rsreg->dpl = (lib_u8)_GetDesc_DPL(descriptor);
            rsreg->limit = (lib_u32)((_IsDescSegGranularLarge(descriptor) ? ((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
            rsreg->seg.accessed = (lib_u8)_IsDescUserAccessed(descriptor);
            rsreg->seg.executable = (lib_u8)_IsDescUserExecutable(descriptor);
            rsreg->seg.data.big = (lib_u8)_IsDescDataBig(descriptor);
            rsreg->seg.data.expdown = (lib_u8)_IsDescDataExpDown(descriptor);
            rsreg->seg.data.writable = (lib_u8)_IsDescDataWritable(descriptor);
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("!Protected");
            _ksa_load_real_sreg(context, rsreg, selector,
                _GetCR0_PE && _GetEFLAGS_VM);
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case SREG_TR:
        CPU_TRACE_BLOCK_BEGIN("sregtype(SREG_TR)");
        if (!_IsProtected)
            CPU_TRACE_IMPOSSIBLE_RETURN;
        if (_GetCPL)
            CPU_TRACE_IMPOSSIBLE_RETURN;
        if (_GetSelector_TI(selector))
            CPU_TRACE_IMPOSSIBLE_RETURN;
        if (_IsSelectorNull(selector))
        {
            CPU_TRACE_BLOCK_BEGIN("selector(null)");
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector, X86_CPU_REFERENCE_OF(descriptor)));
        if (!_IsDescTSSAvl(descriptor))
        {
            CPU_TRACE_BLOCK_BEGIN("!DescTssAvl");
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(selector & 0xfffcu));
            CPU_TRACE_BLOCK_END;
        }
        if (!_IsDescPresent(descriptor))
        {
            CPU_TRACE_BLOCK_BEGIN("!DescPresent");
            CPU_TRACE_CHECK_RETURN(_SetExcept_NP(selector & 0xfffcu));
            CPU_TRACE_BLOCK_END;
        }
        _SetDescTSSBusy(descriptor);
        CPU_TRACE_CHECK_RETURN(_ksa_write_xdt(context, selector, X86_CPU_REFERENCE_OF(descriptor)));
        rsreg->flagValid = LIB_TRUE;
        rsreg->selector = selector;
        rsreg->base = (lib_u32)_GetDescSeg_Base(descriptor);
        rsreg->dpl = (lib_u8)_GetDesc_DPL(descriptor);
        rsreg->limit = (lib_u32)((_IsDescSegGranularLarge(descriptor) ? (_GetDescSeg_Limit(descriptor) << 12 | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
        rsreg->sys.type = (lib_u8)_GetDesc_Type(descriptor);
        CPU_TRACE_BLOCK_END;
        break;
    case SREG_LDTR:
        CPU_TRACE_BLOCK_BEGIN("sregtype(SREG_LDTR)");
        if (!_IsProtected)
            CPU_TRACE_IMPOSSIBLE_RETURN;
        if (_GetCPL)
            CPU_TRACE_IMPOSSIBLE_RETURN;
        if (_GetSelector_TI(selector))
            CPU_TRACE_IMPOSSIBLE_RETURN;
        if (_IsSelectorNull(selector))
        {
            CPU_TRACE_BLOCK_BEGIN("selector(null)");
            rsreg->flagValid = LIB_FALSE;
            rsreg->selector = selector;
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("selector(!null)");
            CPU_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector, X86_CPU_REFERENCE_OF(descriptor)));
            if (!_IsDescLDT(descriptor))
            {
                CPU_TRACE_BLOCK_BEGIN("descriptor(!LDT)");
                CPU_TRACE_CHECK_RETURN(_SetExcept_GP(selector & 0xfffcu));
                CPU_TRACE_BLOCK_END;
            }
            if (!_IsDescPresent(descriptor))
            {
                CPU_TRACE_BLOCK_BEGIN("descriptor(!P)");
                CPU_TRACE_CHECK_RETURN(_SetExcept_NP(selector & 0xfffcu));
                CPU_TRACE_BLOCK_END;
            }
            rsreg->flagValid = LIB_TRUE;
            rsreg->selector = selector;
            rsreg->base = (lib_u32)_GetDescSeg_Base(descriptor);
            rsreg->dpl = (lib_u8)_GetDesc_DPL(descriptor);
            rsreg->limit = (lib_u32)((_IsDescSegGranularLarge(descriptor) ? (_GetDescSeg_Limit(descriptor) << 12 | 0x0fff) : (_GetDescSeg_Limit(descriptor))));
            rsreg->sys.type = (lib_u8)_GetDesc_Type(descriptor);
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case SREG_GDTR:
    case SREG_IDTR:
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}

/* regular segment accessing */
static lib_u8 _s_check_selector(x86_core_cpu_execution_context *context, lib_u16 selector)
{
    /* 0 = succ, 1 = fail */
    CPU_TRACE_CALL_BEGIN("_s_check_selector");
    if (_IsSelectorNull(selector) ||
        (_GetSelector_TI(selector) && !cpu_state.data.ldtr.flagValid))
    {
        CPU_TRACE_CALL_END;
        return LIB_TRUE;
    }
    if (X86_CPU_MASK_U32(_GetSelector_Offset(selector) + 7) >
        (_GetSelector_TI(selector) ? X86_CPU_MASK_U32(cpu_state.data.ldtr.limit) : X86_CPU_MASK_U32(cpu_state.data.gdtr.limit)))
    {
        CPU_TRACE_CALL_END;
        return LIB_TRUE;
    }
    CPU_TRACE_CALL_END;
    return LIB_FALSE;
}
static void _s_read_idt(x86_core_cpu_execution_context *context, lib_u8 intid, lib_uptr rdata)
{
    const lib_u32 misses = instruction_state.data.timing_data_misses;
    CPU_TRACE_CALL_BEGIN("_s_read_idt");
    _ksa_read_idt(context, intid, rdata);
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX && !context->preview_mode) {
        const lib_u32 accepted = instruction_state.data.timing_data_misses - misses;
        instruction_state.data.timing_idt_misses += accepted;
        if (accepted != 0u && _GetCR0_PE && !instruction_state.data.except) {
            lib_u64 descriptor;
            lib_memory_copy(&descriptor, (const void *)rdata, sizeof(descriptor));
            if (_IsDescTaskGate(descriptor)) instruction_state.data.timing_task_gate_miss_mask |= 1u;
        }
    }
    CPU_TRACE_CALL_END;
}
static void _s_read_xdt(x86_core_cpu_execution_context *context, lib_u16 selector, lib_uptr rdata)
{
    CPU_TRACE_CALL_BEGIN("_s_read_xdt");
    if (!_GetCR0_PE)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CHECK_RETURN(_ksa_read_xdt(context, selector, rdata));
    CPU_TRACE_CALL_END;
}
static void _s_read_tss(x86_core_cpu_execution_context *context, lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    const lib_u32 misses = instruction_state.data.timing_data_misses;
    const lib_u32 reads = instruction_state.data.timing_missed_reads;
    CPU_TRACE_CALL_BEGIN("_s_read_tss");
    _kma_read_logical(context, &cpu_state.data.tr, offset, rdata, byte, 0, 1);
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX && !context->preview_mode) {
        instruction_state.data.timing_tss_field_misses += instruction_state.data.timing_data_misses - misses;
        instruction_state.data.timing_missed_tss_fields += instruction_state.data.timing_missed_reads - reads;
    }
    CPU_TRACE_CALL_END;
}
static void _s_read_es(x86_core_cpu_execution_context *context, lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_s_read_es");
    CPU_TRACE_CHECK_RETURN(_m_read_logical(context, &cpu_state.data.es, offset, rdata, byte));
    CPU_TRACE_CALL_END;
}
static void _s_read_cs(x86_core_cpu_execution_context *context, lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    x86_core_cpu_memory_access_provenance previous =
        context->memory_access_provenance;
    lib_u32 linear = cpu_state.data.cs.base + offset;
    lib_u32 prefetch_offset;
    lib_u32 instruction_offset = offset - instruction_state.data.receip;
    lib_u8 length_limit = context->cpu_profile == X86_CORE_CPU_PROFILE_80286 ?
        10u : cpu_profile_has_32bit_architecture(context->cpu_profile) ? 15u : 0u;
    CPU_TRACE_CALL_BEGIN("_s_read_cs");
    if (!cpu_profile_has_protected_architecture(context->cpu_profile))
        instruction_offset = X86_CPU_MASK_U16(instruction_offset);
    if (length_limit != 0u && (lib_u64)instruction_offset + byte > length_limit) {
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286)
            CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
    }
    if (context->prefetch_valid && linear >= context->prefetch_linear &&
        (prefetch_offset = linear - context->prefetch_linear) <=
            context->prefetch_count && byte <= context->prefetch_count -
            prefetch_offset) {
        lib_memory_copy((void *)rdata, context->prefetch_bytes + prefetch_offset,
            byte);
    } else {
        context->memory_access_provenance = X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH;
        _kma_read_logical(context, &cpu_state.data.cs, offset, rdata, byte,
            _GetCPL, 1);
    }
    context->memory_access_provenance = previous;
    if (!instruction_state.data.except &&
        (lib_u64)instruction_offset + byte <= sizeof(instruction_state.data.opcodes)) {
        lib_memory_copy(instruction_state.data.opcodes + instruction_offset,
            (const void *)rdata, byte);
        if (instruction_state.data.oplen < instruction_offset + byte)
            instruction_state.data.oplen = instruction_offset + byte;
    }
    CPU_TRACE_CALL_END;
}
static void _s_read_ss(x86_core_cpu_execution_context *context, lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_s_read_ss");
    CPU_TRACE_CHECK_RETURN(_m_read_logical(context, &cpu_state.data.ss, offset, rdata, byte));
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        context->instruction_in_progress && !context->preview_mode)
        instruction_state.data.timing_stack_access = LIB_TRUE;
    CPU_TRACE_CALL_END;
}
static void _s_read_ds(x86_core_cpu_execution_context *context, lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_s_read_ds");
    CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CALL_END;
}
static void _s_read_fs(x86_core_cpu_execution_context *context, lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_s_read_fs");
    CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CALL_END;
}
static void _s_read_gs(x86_core_cpu_execution_context *context, lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_s_read_gs");
    CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CALL_END;
}
static void _s_write_idt(x86_core_cpu_execution_context *context, lib_u8 intid, lib_uptr rdata)
{
    CPU_TRACE_CALL_BEGIN("_s_write_idt");
    CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CALL_END;
}
static void _s_write_xdt(x86_core_cpu_execution_context *context, lib_u16 selector, lib_uptr rdata)
{
    CPU_TRACE_CALL_BEGIN("_s_write_xdt");
    if (!_GetCR0_PE)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CHECK_RETURN(_ksa_write_xdt(context, selector, rdata));
    CPU_TRACE_CALL_END;
}
static void _s_write_tss(x86_core_cpu_execution_context *context, lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_s_write_tss");
    CPU_TRACE_CHECK_RETURN(_kma_write_logical(context, &cpu_state.data.tr,
        offset, rdata, byte, 0, 1));
    CPU_TRACE_CALL_END;
}
static void _s_write_es(x86_core_cpu_execution_context *context, lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_s_write_es");
    CPU_TRACE_CHECK_RETURN(_m_write_logical(context, &cpu_state.data.es, offset, rdata, byte));
    CPU_TRACE_CALL_END;
}
static void _s_write_cs(x86_core_cpu_execution_context *context, lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_s_write_cs");
    CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CALL_END;
}
static void _s_write_ss(x86_core_cpu_execution_context *context, lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_s_write_ss");
    CPU_TRACE_CHECK_RETURN(_m_write_logical(context, &cpu_state.data.ss, offset, rdata, byte));
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        context->instruction_in_progress && !context->preview_mode)
        instruction_state.data.timing_stack_access = LIB_TRUE;
    CPU_TRACE_CALL_END;
}
static void _s_write_ds(x86_core_cpu_execution_context *context, lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_s_write_ds");
    CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CALL_END;
}
static void _s_write_fs(x86_core_cpu_execution_context *context, lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_s_write_fs");
    CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CALL_END;
}
static void _s_write_gs(x86_core_cpu_execution_context *context, lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_s_write_gs");
    CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CALL_END;
}
static void _s_test_tss(x86_core_cpu_execution_context *context, lib_u32 offset, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_s_test_cs");
    CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, &cpu_state.data.tr, offset, byte, 0, 0x00, 1));
    CPU_TRACE_CALL_END;
}
static void _s_test_cs(x86_core_cpu_execution_context *context, lib_u32 offset, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_s_test_cs");
    CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, &cpu_state.data.cs, offset, byte, 0, 0x00, 1));
    CPU_TRACE_CALL_END;
}
static void _s_test_ss_push(x86_core_cpu_execution_context *context, lib_u8 byte)
{
    lib_u32 cesp = 0x00000000;
    CPU_TRACE_CALL_BEGIN("_s_test_ss_push");
    switch (_GetStackSize)
    {
    case 2:
        CPU_TRACE_BLOCK_BEGIN("StackSize(2)");
        if (cpu_profile_has_protected_architecture(context->cpu_profile) &&
            cpu_state.data.sp && cpu_state.data.sp < byte)
            CPU_TRACE_CHECK_RETURN(_SetExcept_SS(0));
        CPU_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.ss, X86_CPU_MASK_U16(cpu_state.data.sp - byte), byte, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("StackSize(4)");
        if (cpu_profile_has_protected_architecture(context->cpu_profile) &&
            cpu_state.data.esp && cpu_state.data.esp < byte)
            CPU_TRACE_CHECK_RETURN(_SetExcept_SS(0));
        CPU_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.ss, X86_CPU_MASK_U32(cpu_state.data.esp - byte), byte, 1));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}
/* Ordinary instruction admission checks segments only. Page/provider effects
 * stay ordered at the scalar transfer; inner gates retain translated probes. */
static void _s_test_ss_frame(x86_core_cpu_execution_context *context,
    lib_u8 width, lib_u16 elements, lib_u16 allocation, lib_bool write)
{
    const lib_u32 mask = _GetStackSize == 2 ? 0xffffu : 0xffffffffu;
    lib_u32 stack = cpu_state.data.esp & mask;
    lib_u16 index;

    CPU_TRACE_CALL_BEGIN("_s_test_ss_frame");
    for (index = 0u; index < elements; ++index)
    {
        if (write)
        {
            if (_IsProtected && stack && stack < width)
                CPU_TRACE_CHECK_RETURN(_SetExcept_SS(0));
            stack = (stack - width) & mask;
        }
        CPU_TRACE_CHECK_RETURN(_m_test_logical(context, &cpu_state.data.ss,
            stack, width, write));
        if (!write) stack = (stack + width) & mask;
    }
    if (_IsProtected && allocation)
    {
        if (stack && stack < allocation)
            CPU_TRACE_CHECK_RETURN(_SetExcept_SS(0));
        stack = (stack - allocation) & mask;
        CPU_TRACE_CHECK_RETURN(_m_test_logical(context, &cpu_state.data.ss,
            stack, 0u, LIB_TRUE));
    }
    CPU_TRACE_CALL_END;
}

static void _s_test_stack_frame_16(x86_core_cpu_execution_context *context,
    t_cpu_data_sreg *stack, lib_u16 sp, lib_u8 words,
    lib_u8 cpl)
{
    lib_u8 index;

    CPU_TRACE_CALL_BEGIN("_s_test_stack_frame_16");
    for (index = 0; index < words; ++index)
    {
        if (sp && sp < 2u)
            CPU_TRACE_CHECK_RETURN(_SetExcept_SS(0));
        sp = X86_CPU_MASK_U16(sp - 2u);
        CPU_TRACE_CHECK_RETURN(_kma_test_access(context, stack, sp, 2u, 1,
            cpl, 1));
    }
    CPU_TRACE_CALL_END;
}
static void _s_test_stack_frame_32(x86_core_cpu_execution_context *context,
    t_cpu_data_sreg *stack, lib_u32 esp, lib_u8 dwords,
    lib_u8 cpl)
{
    lib_u8 index;

    CPU_TRACE_CALL_BEGIN("_s_test_stack_frame_32");
    for (index = 0u; index < dwords; ++index)
    {
        if (stack->seg.data.big) {
            if (esp && esp < 4u)
                CPU_TRACE_CHECK_RETURN(_SetExcept_SS(0));
            esp -= 4u;
        }
        else {
            lib_u16 sp = X86_CPU_MASK_U16(esp);

            if (sp && sp < 4u)
                CPU_TRACE_CHECK_RETURN(_SetExcept_SS(0));
            esp = X86_CPU_MASK_U16(sp - 4u);
        }
        CPU_TRACE_CHECK_RETURN(_kma_test_access(context, stack, esp, 4u, 1,
            cpl, 1));
    }
    CPU_TRACE_CALL_END;
}
static void _s_test_ss_pop(x86_core_cpu_execution_context *context, lib_u8 byte)
{
    lib_u32 cesp = 0x00000000;
    CPU_TRACE_CALL_BEGIN("_s_test_ss_pop");
    switch (_GetStackSize)
    {
    case 2:
        CPU_TRACE_BLOCK_BEGIN("StackSize(2)");
        CPU_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.ss, cpu_state.data.sp, byte, 0));
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("StackSize(4)");
        CPU_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.ss, cpu_state.data.esp, byte, 0));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _s_test_ss_iret_pop_286(x86_core_cpu_execution_context *context)
{
    lib_u8 offset;

    CPU_TRACE_CALL_BEGIN("_s_test_ss_iret_pop_286");
    /* Intel's 80286 IRET real-mode exception is tied to an individual pop
     * beginning at FFFFh.  Its three words may therefore start at FFFEh,
     * 0000h, and 0002h.  RET and other stack instructions retain their own
     * documented boundary rules through the general frame preflight. */
    for (offset = 0u; offset < 6u; offset += 2u) {
        CPU_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.ss,
            X86_CPU_MASK_U16(cpu_state.data.sp + offset), 2u, 0));
    }
    CPU_TRACE_CALL_END;
}
static void _s_peek_ss_pop(x86_core_cpu_execution_context *context,
    lib_u32 offset, lib_uptr rdata, lib_u8 byte)
{
    lib_u32 cesp;

    CPU_TRACE_CALL_BEGIN("_s_peek_ss_pop");
    switch (_GetStackSize)
    {
    case 2:
        cesp = X86_CPU_MASK_U16(cpu_state.data.sp + offset);
        break;
    case 4:
        cesp = cpu_state.data.esp + offset;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CHECK_RETURN(_s_read_ss(context, cesp, rdata, byte));
    CPU_TRACE_CALL_END;
}
static void _s_load_sreg(x86_core_cpu_execution_context *context, t_cpu_data_sreg *rsreg, lib_u16 selector)
{
    CPU_TRACE_CALL_BEGIN("_s_load_sreg");
    CPU_TRACE_CHECK_RETURN(_ksa_load_sreg(context, rsreg, selector));
    CPU_TRACE_CALL_END;
}
static void _s_load_gdtr(x86_core_cpu_execution_context *context, lib_u32 base, lib_u16 limit, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_s_load_gdtr");
    if (_GetCPL)
    {
        CPU_TRACE_BLOCK_BEGIN("CPL(!0)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        CPU_TRACE_BLOCK_END;
    }
    cpu_state.data.gdtr.limit = limit;
    switch (byte)
    {
    case 2:
        cpu_state.data.gdtr.base = X86_CPU_MASK_U24(base);
        break;
    case 4:
        cpu_state.data.gdtr.base = X86_CPU_MASK_U32(base);
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("byte");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _s_load_idtr(x86_core_cpu_execution_context *context, lib_u32 base, lib_u16 limit, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_s_load_idtr");
    if (_GetCPL)
    {
        CPU_TRACE_BLOCK_BEGIN("CPL(!0)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        CPU_TRACE_BLOCK_END;
    }
    cpu_state.data.idtr.limit = limit;
    switch (byte)
    {
    case 2:
        cpu_state.data.idtr.base = X86_CPU_MASK_U24(base);
        break;
    case 4:
        cpu_state.data.idtr.base = X86_CPU_MASK_U32(base);
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("byte");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _s_load_ldtr(x86_core_cpu_execution_context *context, lib_u16 selector)
{
    CPU_TRACE_CALL_BEGIN("_s_load_ldtr");
    if (_GetCPL)
    {
        CPU_TRACE_BLOCK_BEGIN("CPL(!0)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        CPU_TRACE_BLOCK_END;
    }
    if (_GetSelector_TI(selector))
    {
        CPU_TRACE_BLOCK_BEGIN("Selector_TI(1)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(selector & 0xfffcu));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.ldtr, selector));
    CPU_TRACE_CALL_END;
}
static void _s_load_tr(x86_core_cpu_execution_context *context, lib_u16 selector)
{
    CPU_TRACE_CALL_BEGIN("_s_load_tr");
    if (_GetCPL)
    {
        CPU_TRACE_BLOCK_BEGIN("CPL(!0)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        CPU_TRACE_BLOCK_END;
    }
    if (_GetSelector_TI(selector))
    {
        CPU_TRACE_BLOCK_BEGIN("Selector_TI(1)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(selector & 0xfffcu));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.tr, selector));
    CPU_TRACE_CALL_END;
}
static void _s_load_cr0_msw(x86_core_cpu_execution_context *context, lib_u16 msw)
{
    CPU_TRACE_CALL_BEGIN("_s_load_cr0_msw");
    if (_GetCPL)
    {
        CPU_TRACE_BLOCK_BEGIN("CPL(!0)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        CPU_TRACE_BLOCK_END;
    }
    if (!_GetCR0_PE)
    {
        CPU_TRACE_BLOCK_BEGIN("CR0_PE(0)");
        cpu_state.data.cr0 = (cpu_state.data.cr0 & 0xfffffff0) | (msw & 0x000f);
        CPU_TRACE_BLOCK_END;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("CR0_PE(1)");
        cpu_state.data.cr0 = (cpu_state.data.cr0 & 0xfffffff0) | (msw & 0x000e) | 0x01;
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
static void _s_write_cr0(x86_core_cpu_execution_context *context,
    lib_u32 value)
{
    lib_u32 mutable_mask = VCPU_CR0_PE | VCPU_CR0_MP |
        VCPU_CR0_EM | VCPU_CR0_TS | VCPU_CR0_ET | VCPU_CR0_PG;
    const lib_bool is486 = context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX;

    CPU_TRACE_CALL_BEGIN("_s_write_cr0");
    if (is486)
    {
        mutable_mask |= VCPU_CR0_NE | VCPU_CR0_WP | VCPU_CR0_AM |
            VCPU_CR0_NW | VCPU_CR0_CD;
        /* Intel486 developer manual Tables 4-2 and 4-3. */
        if (((value & VCPU_CR0_PG) && !(value & VCPU_CR0_PE)) ||
            ((value & VCPU_CR0_NW) && !(value & VCPU_CR0_CD)))
        {
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        }
        /* ET is fixed at one. Unspecified reserved bits retain no data;
         * they are not an additional architectural fault condition. */
        value = (value & mutable_mask) | VCPU_CR0_ET;
    }
    else if ((value & ~mutable_mask) != (cpu_state.data.cr0 & ~mutable_mask))
    {
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
    }
    if (X86_CPU_BIT_IS_SET(value, VCPU_CR0_PG) &&
        !X86_CPU_BIT_IS_SET(value, VCPU_CR0_PE))
    {
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
    }
    /* On 80386 a privileged MOV CR0 may clear PE. The following far jump
     * supplies the real-address CS cache transition. A PG change also
     * invalidates bytes fetched through the former translation context. */
    if ((cpu_state.data.cr0 ^ value) & VCPU_CR0_PG) {
        x86_core_cpu_execution_invalidate_prefetch(context);
        x86_core_cpu_execution_invalidate_tlb(context);
    }
    cpu_state.data.cr0 = value;
    CPU_TRACE_CALL_END;
}
static void _s_write_cr3(x86_core_cpu_execution_context *context,
    lib_u32 value)
{
    CPU_TRACE_CALL_BEGIN("_s_write_cr3");
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
    {
        /* The directory base is page aligned; PWT and PCD remain controls. */
        value &= VCPU_CR3_BASE | VCPU_CR3_PWT | VCPU_CR3_PCD;
    }
    else if (value & ~VCPU_CR3_BASE)
    {
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
    }
    /* A new directory changes the translation context of queued bytes. */
    x86_core_cpu_execution_invalidate_prefetch(context);
    x86_core_cpu_execution_invalidate_tlb(context);
    cpu_state.data.cr3 = value;
    CPU_TRACE_CALL_END;
}
static void _s_load_cs(x86_core_cpu_execution_context *context, lib_u16 newcs)
{
    CPU_TRACE_CALL_BEGIN("_s_load_cs");
    CPU_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.cs, newcs));
    CPU_TRACE_CALL_END;
}
static void _s_load_ss(x86_core_cpu_execution_context *context, lib_u16 newss)
{
    CPU_TRACE_CALL_BEGIN("_s_load_ss");
    CPU_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.ss, newss));
    CPU_TRACE_CALL_END;
}
static void _s_load_ds(x86_core_cpu_execution_context *context, lib_u16 newds)
{
    CPU_TRACE_CALL_BEGIN("_s_load_ds");
    CPU_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.ds, newds));
    CPU_TRACE_CALL_END;
}
static void _s_load_es(x86_core_cpu_execution_context *context, lib_u16 newes)
{
    CPU_TRACE_CALL_BEGIN("_s_load_es");
    CPU_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.es, newes));
    CPU_TRACE_CALL_END;
}
static void _s_load_fs(x86_core_cpu_execution_context *context, lib_u16 newfs)
{
    CPU_TRACE_CALL_BEGIN("_s_load_fs");
    CPU_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.fs, newfs));
    CPU_TRACE_CALL_END;
}
static void _s_load_gs(x86_core_cpu_execution_context *context, lib_u16 newgs)
{
    CPU_TRACE_CALL_BEGIN("_s_load_gs");
    CPU_TRACE_CHECK_RETURN(_s_load_sreg(context, &cpu_state.data.gs, newgs));
    CPU_TRACE_CALL_END;
}
/* portid accessing unit */
/* kernel portid accessing */
_______todo _kpa_test_iomap(x86_core_cpu_execution_context *context, lib_u16 portid, lib_u8 byte)
{
    lib_u16 iomap_base;
    lib_u32 bitmap_offset;
    lib_u32 checked_port;
    lib_u8 bitmap_byte;

    CPU_TRACE_CALL_BEGIN("_kpa_test_iomap");
    if (byte != 1u && byte != 2u && byte != 4u) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
    }
    if (!cpu_profile_has_32bit_architecture(context->cpu_profile) ||
        !cpu_state.data.tr.flagValid ||
        cpu_state.data.tr.sys.type != VCPU_DESC_SYS_TYPE_TSS_32_BUSY) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
    }
    if (X86_CPU_MASK_U32(portid) + byte > (lib_u32)LIB_UINT16_MAX + 1u) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
    }
    if (cpu_state.data.tr.limit < 0x67u) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
    }
    CPU_TRACE_CHECK_RETURN(_s_read_tss(context, 0x66u,
        X86_CPU_REFERENCE_OF(iomap_base), 2u));
    for (checked_port = portid; checked_port < X86_CPU_MASK_U32(portid) +
            byte; ++checked_port) {
        bitmap_offset = X86_CPU_MASK_U32(iomap_base) +
            (checked_port >> 3u);
        if (bitmap_offset > cpu_state.data.tr.limit) {
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        }
        CPU_TRACE_CHECK_RETURN(_s_read_tss(context, bitmap_offset,
            X86_CPU_REFERENCE_OF(bitmap_byte), 1u));
        if (X86_CPU_BIT_IS_SET(bitmap_byte, 1u << (checked_port & 7u))) {
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        }
    }
    CPU_TRACE_CALL_END;
}
static void _kpa_test_mode(x86_core_cpu_execution_context *context, lib_u16 portid, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_p_test");
    if (_GetCR0_PE && (_GetCPL > (lib_u8)_GetEFLAGS_IOPL || _GetEFLAGS_VM))
    {
        CPU_TRACE_BLOCK_BEGIN("CR0_PE(1),(CPL>IOPL/EFLAGS_VM(1))");
        CPU_TRACE_CHECK_RETURN(_kpa_test_iomap(context, portid, byte));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
static void _smm_record_io(x86_core_cpu_execution_context *context)
{
    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX) return;
    context->io_restart_valid = LIB_TRUE;
    context->io_restart_eip = instruction_state.data.oldcpu.data.eip;
    context->io_restart_esi = instruction_state.data.oldcpu.data.esi;
    context->io_restart_ecx = instruction_state.data.oldcpu.data.ecx;
    context->io_restart_edi = instruction_state.data.oldcpu.data.edi;
}

/* regular portid accessing */
static void _p_input(x86_core_cpu_execution_context *context, lib_u16 portid, lib_uptr rdata, lib_u8 byte)
{
    lib_u32 value;
    lib_u16 word;
    lib_u8 octet;
    CPU_TRACE_CALL_BEGIN("_p_input");
    CPU_TRACE_CHECK_RETURN(_kpa_test_mode(context, portid, byte));
    if (byte != 1u && byte != 2u && byte != 4u) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
    }
    x86_core_cpu_bus_prepare_read(context, LIB_TRUE);
    _kma_publish_external_cycle(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN,
        X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT, portid, byte, LIB_FALSE,
        X86_CORE_CPU_MEMORY_ACCESS_DATA);
    if (context->bus->transfer_port(context->bus_context, portid, byte,
            LIB_FALSE, &value) != LIB_STATUS_OK) {
        _kma_publish_external_cycle(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT, portid, byte, LIB_FALSE,
            X86_CORE_CPU_MEMORY_ACCESS_DATA);
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(portid));
    }
    switch (byte)
    {
    case 1:
        CPU_TRACE_BLOCK_BEGIN("byte(1)");
        octet = (lib_u8)value;
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, rdata,
                                             X86_CPU_REFERENCE_OF(octet), 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 2:
        CPU_TRACE_BLOCK_BEGIN("byte(2)");
        word = (lib_u16)value;
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, rdata,
                                             X86_CPU_REFERENCE_OF(word), 2));
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("byte(4)");
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, rdata,
                                             X86_CPU_REFERENCE_OF(value), 4));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("byte");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    instruction_state.data.flagIgnore = LIB_TRUE;
    _smm_record_io(context);
    context->bus->complete_port(context->bus_context, portid, byte, LIB_FALSE);
    const lib_u64 wait_ticks = _kma_publish_external_cycle(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT,
        X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT, portid, byte, LIB_FALSE,
        X86_CORE_CPU_MEMORY_ACCESS_DATA);
    x86_core_cpu_bus_read_complete(context, 0u, byte, wait_ticks);
    CPU_TRACE_CALL_END;
}
static void _p_output(x86_core_cpu_execution_context *context, lib_u16 portid, lib_uptr rdata, lib_u8 byte)
{
    lib_u32 value = 0u;
    lib_u16 word;
    lib_u8 octet;
    CPU_TRACE_CALL_BEGIN("_p_output");
    CPU_TRACE_CHECK_RETURN(_kpa_test_mode(context, portid, byte));
    switch (byte)
    {
    case 1:
        CPU_TRACE_BLOCK_BEGIN("byte(1)");
        CPU_TRACE_CHECK_RETURN(_kma_read_ref(context, rdata,
                                            X86_CPU_REFERENCE_OF(octet), 1));
        value = octet;
        CPU_TRACE_BLOCK_END;
        break;
    case 2:
        CPU_TRACE_BLOCK_BEGIN("byte(2)");
        CPU_TRACE_CHECK_RETURN(_kma_read_ref(context, rdata,
                                            X86_CPU_REFERENCE_OF(word), 2));
        value = word;
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("byte(4)");
        CPU_TRACE_CHECK_RETURN(_kma_read_ref(context, rdata,
                                            X86_CPU_REFERENCE_OF(value), 4));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("byte");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    lib_bool buffered = LIB_FALSE;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        (cpu_state.data.cr0 & VCPU_CR0_CD) == 0u &&
        instruction_state.data.prefix_rep != PREFIX_REP_NONE) {
        const lib_u8 opcode = instruction_state.data.opcodes[
            x86_core_cpu_instruction_prefix_count(&instruction_state.data)];

        buffered = opcode == 0x6eu || opcode == 0x6fu;
    }
    if (buffered) x86_core_cpu_bus_prepare_write(context);
    else x86_core_cpu_bus_drain(context, LIB_TRUE);
    _kma_publish_external_cycle(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN,
        X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT, portid, byte, LIB_TRUE,
        X86_CORE_CPU_MEMORY_ACCESS_DATA);
    if (context->bus->transfer_port(context->bus_context, portid, byte,
            LIB_TRUE, &value) != LIB_STATUS_OK) {
        _kma_publish_external_cycle(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT, portid, byte, LIB_TRUE,
            X86_CORE_CPU_MEMORY_ACCESS_DATA);
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(portid));
    }
    instruction_state.data.flagIgnore = LIB_TRUE;
    _smm_record_io(context);
    context->bus->complete_port(context->bus_context, portid, byte, LIB_TRUE);
    const lib_u64 wait_ticks = _kma_publish_external_cycle(context, X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT,
        X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT, portid, byte, LIB_TRUE,
        X86_CORE_CPU_MEMORY_ACCESS_DATA);
    x86_core_cpu_bus_write_complete(context, 0u, byte, wait_ticks, LIB_FALSE, buffered);
    CPU_TRACE_CALL_END;
}

/* decoding unit */
/* kernel decoding function */
static lib_u8 _kdf_check_prefix(x86_core_cpu_execution_context *context, lib_u8 opcode)
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
        return LIB_TRUE;
        break;
    case 0x64:
    case 0x65:
    case 0x66:
    case 0x67:
        if (cpu_profile_has_32bit_architecture(context->cpu_profile))
            return LIB_TRUE;
        else
            return LIB_FALSE;
        break;
    default:
        return LIB_FALSE;
        break;
    }
    return LIB_FALSE;
}

static void _kdf_skip(x86_core_cpu_execution_context *context, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_kdf_skip");
    if (cpu_profile_has_protected_architecture(context->cpu_profile))
        CPU_TRACE_CHECK_RETURN(cpu_state.data.eip += byte);
    else
        CPU_TRACE_CHECK_RETURN(cpu_state.data.eip = X86_CPU_MASK_U16(
            cpu_state.data.eip + byte));
    context->prefetch_expected_linear = cpu_state.data.cs.base + cpu_state.data.eip;
    context->prefetch_expected_valid = LIB_TRUE;
    CPU_TRACE_CALL_END;
}
static void _kdf_code(x86_core_cpu_execution_context *context, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_kdf_code");
    CPU_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, rdata, byte));
    CPU_TRACE_CHECK_RETURN(_kdf_skip(context, byte));
    CPU_TRACE_CALL_END;
}
static void _kdf_modrm_with_mod_quirk(x86_core_cpu_execution_context *context, lib_u8 regbyte, lib_u8 rmbyte, lib_u8 ignore_mod)
{
    lib_i8 disp8;
    lib_u16 disp16;
    lib_u32 disp32;
    lib_u32 sibindex = 0u;
    lib_u8 modrm = 0u;
    lib_u8 sib = 0u;
    CPU_TRACE_CALL_BEGIN("_kdf_modrm");
    CPU_TRACE_CHECK_RETURN(_kdf_code(context, X86_CPU_REFERENCE_OF(modrm), 1));
    instruction_state.data.flagMem = LIB_TRUE;
    instruction_state.data.mrm.rsreg = LIB_NULL;
    instruction_state.data.mrm.offset = 0;
    instruction_state.data.cr = instruction_state.data.crm = 0;
    instruction_state.data.rrm = instruction_state.data.rr = (lib_uptr)LIB_NULL;
    /* The normal architecture requires MOD=11 for MOV CR. Selected original
     * 80386 silicon instead takes only the r/m register bits: no effective
     * address, SIB, displacement, or memory operation is decoded. */
    if (!ignore_mod || _GetModRM_MOD(modrm) == 3)
    {
        switch (_GetAddressSize)
    {
    case 2:
        CPU_TRACE_BLOCK_BEGIN("AddressSize(2)");
        switch (_GetModRM_MOD(modrm))
        {
        case 0:
            CPU_TRACE_BLOCK_BEGIN("ModRM_MOD(0)");
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.bx + cpu_state.data.si);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 1:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.bx + cpu_state.data.di);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 2:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.bp + cpu_state.data.si);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 3:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.bp + cpu_state.data.di);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 4:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.si);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 5:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.di);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 6:
                CPU_TRACE_BLOCK_BEGIN("ModRM_RM(6)");
                CPU_TRACE_CHECK_RETURN(_kdf_code(context, X86_CPU_REFERENCE_OF(disp16), 2));
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                CPU_TRACE_BLOCK_END;
                break;
            case 7:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.bx);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
            break;
        case 1:
            CPU_TRACE_BLOCK_BEGIN("ModRM_MOD(1)");
            CPU_TRACE_CHECK_RETURN(_kdf_code(context, X86_CPU_REFERENCE_OF(disp8), 1));
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.bx + cpu_state.data.si + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 1:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.bx + cpu_state.data.di + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 2:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.bp + cpu_state.data.si + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 3:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.bp + cpu_state.data.di + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 4:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.si + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 5:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.di + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 6:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.bp + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 7:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.bx + disp8);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
            break;
        case 2:
            CPU_TRACE_BLOCK_BEGIN("ModRM_MOD(2)");
            CPU_TRACE_CHECK_RETURN(_kdf_code(context, X86_CPU_REFERENCE_OF(disp16), 2));
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.bx + cpu_state.data.si + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 1:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.bx + cpu_state.data.di + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 2:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.bp + cpu_state.data.si + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 3:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.bp + cpu_state.data.di + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 4:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.si + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 5:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.di + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            case 6:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.bp + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverss;
                break;
            case 7:
                instruction_state.data.mrm.offset = X86_CPU_MASK_U16(cpu_state.data.bx + disp16);
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
            break;
        case 3:
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("AddressSize(4)");
        if (_GetModRM_MOD(modrm) != 3 && _GetModRM_RM(modrm) == 4)
        {
            CPU_TRACE_BLOCK_BEGIN("ModRM_MOD(!3),ModRM_RM(4)");
            CPU_TRACE_CHECK_RETURN(_kdf_code(context, X86_CPU_REFERENCE_OF(sib), 1));
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
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            sibindex <<= _GetSIB_SS(sib);
            CPU_TRACE_BLOCK_END;
        }
        switch (_GetModRM_MOD(modrm))
        {
        case 0:
            CPU_TRACE_BLOCK_BEGIN("ModRM_MOD(0)");
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
                CPU_TRACE_BLOCK_BEGIN("ModRM_RM(4)");
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
                    CPU_TRACE_BLOCK_BEGIN("SIB_Base(5)");
                    CPU_TRACE_CHECK_RETURN(_kdf_code(context, X86_CPU_REFERENCE_OF(disp32), 4));
                    instruction_state.data.mrm.offset = disp32 + sibindex;
                    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                    CPU_TRACE_BLOCK_END;
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
                    CPU_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                CPU_TRACE_BLOCK_END;
                break;
            case 5:
                CPU_TRACE_BLOCK_BEGIN("ModRM_RM(5)");
                CPU_TRACE_CHECK_RETURN(_kdf_code(context, X86_CPU_REFERENCE_OF(disp32), 4));
                instruction_state.data.mrm.offset = disp32;
                instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
                CPU_TRACE_BLOCK_END;
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
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
            break;
        case 1:
            CPU_TRACE_BLOCK_BEGIN("ModRM_MOD(1)");
            CPU_TRACE_CHECK_RETURN(_kdf_code(context, X86_CPU_REFERENCE_OF(disp8), 1));
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
                CPU_TRACE_BLOCK_BEGIN("ModRM_RM(4)");
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
                    CPU_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                CPU_TRACE_BLOCK_END;
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
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
            break;
        case 2:
            CPU_TRACE_BLOCK_BEGIN("ModRM_MOD(2)");
            CPU_TRACE_CHECK_RETURN(_kdf_code(context, X86_CPU_REFERENCE_OF(disp32), 4));
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
                CPU_TRACE_BLOCK_BEGIN("ModRM_RM(4)");
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
                    CPU_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                CPU_TRACE_BLOCK_END;
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
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
            break;
        case 3:
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        _GetModRM_MOD(modrm) != 3u && !ignore_mod) {
        const lib_u8 mod = _GetModRM_MOD(modrm);
        const lib_u8 rm = _GetModRM_RM(modrm);
        lib_u8 base;
        lib_bool has_base = LIB_TRUE;

        if (_GetAddressSize == 4u) {
            base = rm == 4u ? _GetSIB_Base(sib) : rm;
            has_base = mod != 0u || base != 5u;
            instruction_state.data.timing_ea_indexed =
                rm == 4u && _GetSIB_Index(sib) != 4u;
            instruction_state.data.timing_ea_displacement = mod != 0u || !has_base;
        } else {
            base = rm < 2u || rm == 7u ? 3u :
                rm < 4u || rm == 6u ? 5u : rm == 4u ? 6u : 7u;
            has_base = mod != 0u || rm != 6u;
            instruction_state.data.timing_ea_indexed = rm < 4u;
            instruction_state.data.timing_ea_displacement = mod != 0u || rm == 6u;
        }
        instruction_state.data.timing_ea_base_mask = has_base ?
            (lib_u8)(1u << base) : 0u;
    }
    if (_GetModRM_MOD(modrm) == 3 || ignore_mod)
    {
        CPU_TRACE_BLOCK_BEGIN("ModRM_MOD(3)");
        instruction_state.data.flagMem = LIB_FALSE;
        switch (rmbyte)
        {
        case 1:
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.al);
                break;
            case 1:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.cl);
                break;
            case 2:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.dl);
                break;
            case 3:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.bl);
                break;
            case 4:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.ah);
                break;
            case 5:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.ch);
                break;
            case 6:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.dh);
                break;
            case 7:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.bh);
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        case 2:
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.ax);
                break;
            case 1:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.cx);
                break;
            case 2:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.dx);
                break;
            case 3:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.bx);
                break;
            case 4:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.sp);
                break;
            case 5:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.bp);
                break;
            case 6:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.si);
                break;
            case 7:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.di);
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        case 4:
            switch (_GetModRM_RM(modrm))
            {
            case 0:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.eax);
                break;
            case 1:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.ecx);
                break;
            case 2:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.edx);
                break;
            case 3:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.ebx);
                break;
            case 4:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.esp);
                break;
            case 5:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.ebp);
                break;
            case 6:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.esi);
                break;
            case 7:
                instruction_state.data.rrm = (lib_uptr)(&cpu_state.data.edi);
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        default:
            CPU_TRACE_BLOCK_BEGIN("rmbyte");
            CPU_TRACE_CHECK_RETURN(_SetExcept_CE(rmbyte));
            CPU_TRACE_BLOCK_END;
            break;
        }
        CPU_TRACE_CHECK_RETURN(_kma_read_ref(context, instruction_state.data.rrm, X86_CPU_REFERENCE_OF(instruction_state.data.crm), rmbyte));
        CPU_TRACE_BLOCK_END;
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
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.al);
                break;
            case 1:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.cl);
                break;
            case 2:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.dl);
                break;
            case 3:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.bl);
                break;
            case 4:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.ah);
                break;
            case 5:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.ch);
                break;
            case 6:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.dh);
                break;
            case 7:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.bh);
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        case 2:
            switch (_GetModRM_REG(modrm))
            {
            case 0:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.ax);
                break;
            case 1:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.cx);
                break;
            case 2:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.dx);
                break;
            case 3:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.bx);
                break;
            case 4:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.sp);
                break;
            case 5:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.bp);
                break;
            case 6:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.si);
                break;
            case 7:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.di);
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        case 4:
            switch (_GetModRM_REG(modrm))
            {
            case 0:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.eax);
                break;
            case 1:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.ecx);
                break;
            case 2:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.edx);
                break;
            case 3:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.ebx);
                break;
            case 4:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.esp);
                break;
            case 5:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.ebp);
                break;
            case 6:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.esi);
                break;
            case 7:
                instruction_state.data.rr = (lib_uptr)(&cpu_state.data.edi);
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            break;
        default:
            CPU_TRACE_BLOCK_BEGIN("regbyte");
            CPU_TRACE_CHECK_RETURN(_SetExcept_CE(regbyte));
            CPU_TRACE_BLOCK_END;
            break;
        }
        CPU_TRACE_CHECK_RETURN(_kma_read_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.cr), regbyte));
    }
    CPU_TRACE_CALL_END;
}
static void _kdf_modrm(x86_core_cpu_execution_context *context,
    lib_u8 regbyte, lib_u8 rmbyte)
{
    _kdf_modrm_with_mod_quirk(context, regbyte, rmbyte, LIB_FALSE);
}

static void _kdf_modrm_creg(x86_core_cpu_execution_context *context)
{
    _kdf_modrm_with_mod_quirk(context, 0, 4,
        context->cpu_80386_cr_mov_ignores_mod);
}
static void _d_skip(x86_core_cpu_execution_context *context, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_d_skip");
    CPU_TRACE_CHECK_RETURN(_kdf_skip(context, byte));
    CPU_TRACE_CALL_END;
}
static void _d_code(x86_core_cpu_execution_context *context, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_d_code");
    CPU_TRACE_CHECK_RETURN(_kdf_code(context, rdata, byte));
    CPU_TRACE_CALL_END;
}
static void _d_imm(x86_core_cpu_execution_context *context, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_d_imm");
    instruction_state.data.cimm = 0;
    CPU_TRACE_CHECK_RETURN(_d_code(context, X86_CPU_REFERENCE_OF(instruction_state.data.cimm), byte));
    instruction_state.data.timing_immediate = LIB_TRUE;
    CPU_TRACE_CALL_END;
}
static void _d_moffs(x86_core_cpu_execution_context *context, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_d_moffs");
    instruction_state.data.flagMem = LIB_TRUE;
    instruction_state.data.mrm.rsreg = instruction_state.data.roverds;
    instruction_state.data.mrm.offset = 0;
    switch (_GetAddressSize)
    {
    case 2:
        CPU_TRACE_BLOCK_BEGIN("AddressSize(2)");
        CPU_TRACE_CHECK_RETURN(_d_code(context, X86_CPU_REFERENCE_OF(instruction_state.data.mrm.offset), 2));
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("AddressSize(4)");
        CPU_TRACE_CHECK_RETURN(_d_code(context, X86_CPU_REFERENCE_OF(instruction_state.data.mrm.offset), 4));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _d_modrm_sreg(x86_core_cpu_execution_context *context, lib_u8 rmbyte)
{
    CPU_TRACE_CALL_BEGIN("_d_modrm_sreg");
    CPU_TRACE_CHECK_RETURN(_kdf_modrm(context, 0, rmbyte));
    instruction_state.data.rmovsreg = LIB_NULL;
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
        if (!cpu_profile_has_32bit_architecture(context->cpu_profile))
            CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        instruction_state.data.rmovsreg = &cpu_state.data.fs;
        break;
    case 5:
        if (!cpu_profile_has_32bit_architecture(context->cpu_profile))
            CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        instruction_state.data.rmovsreg = &cpu_state.data.gs;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr");
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _d_modrm_ea(x86_core_cpu_execution_context *context, lib_u8 regbyte, lib_u8 rmbyte)
{
    CPU_TRACE_CALL_BEGIN("_d_modrm_ea");
    CPU_TRACE_CHECK_RETURN(_kdf_modrm(context, regbyte, rmbyte));
    if (!instruction_state.data.flagMem)
    {
        CPU_TRACE_BLOCK_BEGIN("flagMem(0)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
static void _d_modrm_table_memory(x86_core_cpu_execution_context *context,
    lib_u8 modrm)
{
    CPU_TRACE_CALL_BEGIN("_d_modrm_table_memory");
    /* Reject a register form before the six-byte memory operand is decoded. */
    if (_GetModRM_MOD(modrm) == 3)
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
    CPU_TRACE_CHECK_RETURN(_d_modrm_ea(context, 0, 6));
    CPU_TRACE_CALL_END;
}
/* Store one six-byte SGDT/SIDT pseudo-descriptor after a complete write check.
 * The 80286 manual calls byte six undefined, but physical 80286 software
 * observes FF there to distinguish a 286 from a 386.  Keep that observable
 * compatibility value at this sole table-register boundary. */
static void _m_write_table_pseudo_descriptor(
    x86_core_cpu_execution_context *context, lib_u16 limit,
    lib_u32 base)
{
    lib_u8 image[6];
    CPU_TRACE_CALL_BEGIN("_m_write_table_pseudo_descriptor");
    image[0] = X86_CPU_MASK_U8(limit);
    image[1] = X86_CPU_MASK_U8(limit >> 8u);
    image[2] = X86_CPU_MASK_U8(base);
    image[3] = X86_CPU_MASK_U8(base >> 8u);
    image[4] = X86_CPU_MASK_U8(base >> 16u);
    image[5] = context->cpu_profile == X86_CORE_CPU_PROFILE_80286 ?
        0xffu : X86_CPU_MASK_U8(base >> 24u);
    CPU_TRACE_CHECK_RETURN(_m_test_logical(context,
        instruction_state.data.mrm.rsreg, instruction_state.data.mrm.offset,
        6, LIB_TRUE));
    /* PRM 9.9.16: the dword base follows the word limit; an address
     * 2 modulo 4 aligns both fields of the stored pseudo-descriptor. */
    CPU_TRACE_CHECK_RETURN(_kma_check_alignment(context,
        instruction_state.data.mrm.rsreg->base +
            instruction_state.data.mrm.offset + 2u, 4u, _GetCPL, 0u));
    CPU_TRACE_CHECK_RETURN(_m_test_access(context,
        instruction_state.data.mrm.rsreg, instruction_state.data.mrm.offset,
        6, LIB_TRUE));
    CPU_TRACE_CHECK_RETURN(_m_write_logical(context,
        instruction_state.data.mrm.rsreg, instruction_state.data.mrm.offset,
        (lib_uptr)image, 6));
    CPU_TRACE_CALL_END;
}
static void _d_modrm(x86_core_cpu_execution_context *context, lib_u8 regbyte, lib_u8 rmbyte)
{
    CPU_TRACE_CALL_BEGIN("_d_modrm");
    CPU_TRACE_CHECK_RETURN(_kdf_modrm(context, regbyte, rmbyte));
    if (!instruction_state.data.flagMem && instruction_state.data.flagLock)
    {
        CPU_TRACE_BLOCK_BEGIN("flagMem(0),flagLock(1)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}

/* execution control unit: _e_ */
/* kernel execution control */
static void _kec_push(x86_core_cpu_execution_context *context, lib_uptr rdata, lib_u8 byte)
{
    lib_u32 cesp;
    CPU_TRACE_CALL_BEGIN("_kec_push");
    CPU_TRACE_CHECK_RETURN(_s_test_ss_push(context, byte));
    switch (_GetStackSize)
    {
    case 2:
        cesp = X86_CPU_MASK_U16(cpu_state.data.sp - byte);
        break;
    case 4:
        cesp = X86_CPU_MASK_U32(cpu_state.data.esp - byte);
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CHECK_RETURN(_s_write_ss(context, cesp, rdata, byte));
    switch (_GetStackSize)
    {
    case 2:
        _WriteGPR16(sp) -= byte;
        break;
    case 4:
        _WriteGPR32(esp) -= byte;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _kec_pop(x86_core_cpu_execution_context *context, lib_uptr rdata, lib_u8 byte)
{
    lib_u32 cesp;
    CPU_TRACE_CALL_BEGIN("_kec_pop");
    switch (_GetStackSize)
    {
    case 2:
        cesp = cpu_state.data.sp;
        break;
    case 4:
        cesp = cpu_state.data.esp;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CHECK_RETURN(_s_read_ss(context, cesp, rdata, byte));
    if (rdata != X86_CPU_REFERENCE_OF(cpu_state.data.esp))
    {
        switch (_GetStackSize)
        {
        case 2:
            _WriteGPR16(sp) += byte;
            break;
        case 4:
            _WriteGPR32(esp) += byte;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    CPU_TRACE_CALL_END;
}
static void _kec_call_far(x86_core_cpu_execution_context *context, lib_u16 newcs, lib_u32 neweip, lib_u8 byte)
{
    lib_u32 oldcs = cpu_state.data.cs.selector;
    t_cpu_data_sreg ccs = cpu_state.data.cs;
    CPU_TRACE_CALL_BEGIN("_kec_call_far");
    switch (byte)
    {
    case 2:
        CPU_TRACE_BLOCK_BEGIN("byte(2)");
        CPU_TRACE_CHECK_RETURN(_s_test_ss_push(context, 4));
        neweip = X86_CPU_MASK_U16(neweip);
        CPU_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
        CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldcs), 2));
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.ip), 2));
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("byte(4)");
        CPU_TRACE_CHECK_RETURN(_s_test_ss_push(context, 8));
        neweip = X86_CPU_MASK_U32(neweip);
        CPU_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
        CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldcs), 4));
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.eip), 4));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    cpu_state.data.cs = ccs;
    cpu_state.data.eip = neweip;
    CPU_TRACE_CALL_END;
}
static void _kec_call_near(x86_core_cpu_execution_context *context, lib_u32 neweip, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_kec_call_near");
    switch (byte)
    {
    case 2:
        CPU_TRACE_BLOCK_BEGIN("byte(2)");
        neweip = X86_CPU_MASK_U16(neweip);
        CPU_TRACE_CHECK_RETURN(_s_test_cs(context, neweip, 0x01));
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.ip), 2));
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("byte(4)");
        neweip = X86_CPU_MASK_U32(neweip);
        CPU_TRACE_CHECK_RETURN(_s_test_cs(context, neweip, 0x01));
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.eip), 4));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    cpu_state.data.eip = neweip;
    CPU_TRACE_CALL_END;
}
static void _kec_jmp_far(x86_core_cpu_execution_context *context, lib_u16 newcs, lib_u32 neweip, lib_u8 byte)
{
    t_cpu_data_sreg ccs = cpu_state.data.cs;
    CPU_TRACE_CALL_BEGIN("_kec_jmp_far");
    switch (byte)
    {
    case 2:
        neweip = X86_CPU_MASK_U16(neweip);
        break;
    case 4:
        neweip = X86_CPU_MASK_U32(neweip);
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("byte");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
    CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
    cpu_state.data.cs = ccs;
    cpu_state.data.eip = neweip;
    CPU_TRACE_CALL_END;
}
static void _kec_jmp_near(x86_core_cpu_execution_context *context, lib_u32 neweip, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_kec_jmp_near");
    switch (byte)
    {
    case 2:
        neweip = X86_CPU_MASK_U16(neweip);
        break;
    case 4:
        neweip = X86_CPU_MASK_U32(neweip);
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CHECK_RETURN(_s_test_cs(context, neweip, 0x01));
    cpu_state.data.eip = neweip;
    CPU_TRACE_CALL_END;
}
static void _kec_ret_far(x86_core_cpu_execution_context *context, lib_u32 newcs, lib_u32 neweip, lib_u16 parambyte, lib_u16 byte)
{
    t_cpu_data_sreg ccs = cpu_state.data.cs;
    CPU_TRACE_CALL_BEGIN("_kec_ret_far");
    switch (byte)
    {
    case 2:
        neweip = X86_CPU_MASK_U16(neweip);
        break;
    case 4:
        neweip = X86_CPU_MASK_U32(neweip);
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("byte");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
    CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
    cpu_state.data.cs = ccs;
    cpu_state.data.eip = neweip;
    switch (_GetStackSize)
    {
    case 2:
        _WriteGPR16(sp) += parambyte;
        break;
    case 4:
        _WriteGPR32(esp) += parambyte;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _kec_ret_near(x86_core_cpu_execution_context *context, lib_u16 parambyte, lib_u8 byte)
{
    lib_u32 neweip = 0;
    CPU_TRACE_CALL_BEGIN("_kec_ret_near");
    switch (byte)
    {
    case 2:
        CPU_TRACE_BLOCK_BEGIN("byte(2)");
        CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(neweip), 2));
        CPU_TRACE_CHECK_RETURN(_s_test_cs(context, neweip, 0x01));
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("byte(4)");
        CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(neweip), 4));
        CPU_TRACE_CHECK_RETURN(_s_test_cs(context, neweip, 0x01));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("byte");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    cpu_state.data.eip = neweip;
    switch (_GetStackSize)
    {
    case 2:
        _WriteGPR16(sp) += parambyte;
        break;
    case 4:
        _WriteGPR32(esp) += parambyte;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}
/* sub execution routine */
static void _ser_call_far_real(x86_core_cpu_execution_context *context, lib_u16 newcs, lib_u32 neweip, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_ser_call_far_real");
    if (_IsProtected)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CHECK_RETURN(_kec_call_far(context, newcs, neweip, byte));
    CPU_TRACE_CALL_END;
}
_______todo _ser_call_far_cs_conf(x86_core_cpu_execution_context *context, lib_u16 newcs, lib_u32 neweip, lib_u8 byte)
{
    lib_u64 descriptor = 0u;
    CPU_TRACE_CALL_BEGIN("_ser_call_far_cs_conf");
    if (!_IsProtected)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, X86_CPU_REFERENCE_OF(descriptor)));
    if (!_IsDescCodeConform(descriptor))
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (_GetDesc_DPL(descriptor) > _GetCPL)
    {
        CPU_TRACE_BLOCK_BEGIN("DPL(>CPL)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
        CPU_TRACE_BLOCK_END;
    }
    if (!_IsDescPresent(descriptor))
    {
        CPU_TRACE_BLOCK_BEGIN("!DescPresent");
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CHECK_RETURN(_kec_call_far(context, newcs, neweip, byte));
    CPU_TRACE_CALL_END;
}
_______todo _ser_call_far_cs_nonc(x86_core_cpu_execution_context *context, lib_u16 newcs, lib_u32 neweip, lib_u8 byte)
{
    lib_u64 descriptor;
    CPU_TRACE_CALL_BEGIN("_ser_call_far_cs_nonc");
    if (!_IsProtected)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, X86_CPU_REFERENCE_OF(descriptor)));
    if (!_IsDescCodeNonConform(descriptor))
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (_GetDesc_DPL(descriptor) != _GetCPL ||
        _GetSelector_RPL(newcs) > _GetCPL)
    {
        CPU_TRACE_BLOCK_BEGIN("DPL(!CPL)/RPL(>CPL)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
        CPU_TRACE_BLOCK_END;
    }
    if (!_IsDescPresent(descriptor))
    {
        CPU_TRACE_BLOCK_BEGIN("!DescPresent");
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CHECK_RETURN(_kec_call_far(context, newcs, neweip, byte));
    CPU_TRACE_CALL_END;
}
static void _ser_check_call_gate_stack_sreg(
    x86_core_cpu_execution_context *context, lib_u16 selector,
    lib_u8 target_cpl, lib_uptr out_descriptor)
{
    lib_u64 descriptor;

    CPU_TRACE_CALL_BEGIN("_ser_check_call_gate_stack_sreg");
    if (_IsSelectorNull(selector) || _GetSelector_TI(selector) ||
        _GetSelector_RPL(selector) != target_cpl)
        CPU_TRACE_CHECK_RETURN(_SetExcept_TS(selector & 0xfffcu));
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, selector,
        X86_CPU_REFERENCE_OF(descriptor)));
    if (!_IsDescDataWritable(descriptor) ||
        _GetDesc_DPL(descriptor) != target_cpl)
        CPU_TRACE_CHECK_RETURN(_SetExcept_TS(selector & 0xfffcu));
    if (!_IsDescPresent(descriptor))
        CPU_TRACE_CHECK_RETURN(_SetExcept_SS(selector & 0xfffcu));
    lib_memory_copy((void *)out_descriptor, (void *)X86_CPU_REFERENCE_OF(descriptor),
        sizeof(descriptor));
    CPU_TRACE_CALL_END;
}
static void _ser_call_far_call_gate_32(x86_core_cpu_execution_context *context,
    lib_u16 gate_selector)
{
    lib_u64 gate_desc;
    lib_u64 code_desc;
    lib_u64 ss_desc;
    lib_u16 newcs;
    lib_u16 newss;
    lib_u16 oldcs;
    lib_u16 oldss;
    lib_u32 newesp;
    lib_u32 oldeip;
    lib_u32 oldesp;
    lib_u32 oldcs_frame;
    lib_u32 oldss_frame;
    lib_u32 parameters[31];
    lib_u8 oldcpl;
    lib_u8 target_cpl;
    lib_u8 parameter_count;
    lib_u8 index;
    t_cpu_data_sreg newcs_cache;
    t_cpu_data_sreg newss_cache;

    CPU_TRACE_CALL_BEGIN("_ser_call_far_call_gate_32");
    if (!_IsProtected || _GetEFLAGS_VM)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, gate_selector,
        X86_CPU_REFERENCE_OF(gate_desc)));
    if (!_IsDescCallGate32(gate_desc))
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(gate_selector & 0xfffcu));
    oldcpl = _GetCPL;
    if (_GetDesc_DPL(gate_desc) < oldcpl ||
        _GetDesc_DPL(gate_desc) < _GetSelector_RPL(gate_selector))
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(gate_selector & 0xfffcu));
    if (!_IsDescPresent(gate_desc))
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(gate_selector & 0xfffcu));
    newcs = X86_CPU_MASK_U16(_GetDescGate_Selector(gate_desc));
    if (_IsSelectorNull(newcs) || _GetSelector_TI(newcs))
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
        X86_CPU_REFERENCE_OF(code_desc)));
    if (!_IsDescCodeNonConform(code_desc) ||
        _GetDesc_DPL(code_desc) > oldcpl)
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    target_cpl = (lib_u8)_GetDesc_DPL(code_desc);
    if (!_IsDescPresent(code_desc))
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
    oldcs = cpu_state.data.cs.selector;
    oldss = cpu_state.data.ss.selector;
    oldeip = cpu_state.data.eip;
    oldesp = cpu_state.data.esp;
    oldcs_frame = oldcs;
    oldss_frame = oldss;
    newcs_cache = cpu_state.data.cs;
    CPU_TRACE_CHECK_RETURN(_ksa_prepare_code_sreg(context, newcs, target_cpl,
        &newcs_cache, code_desc));
    CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, &newcs_cache,
        X86_CPU_MASK_U32(_GetDescGate_Offset(gate_desc)), 1u, 0,
        target_cpl, 1));
    if (target_cpl < oldcpl) {
        if (!cpu_state.data.tr.flagValid || cpu_state.data.tr.sys.type !=
            VCPU_DESC_SYS_TYPE_TSS_32_BUSY)
            CPU_TRACE_CHECK_RETURN(_SetExcept_TS(0));
        CPU_TRACE_CHECK_RETURN(_s_test_tss(context, 4u, 6u));
        CPU_TRACE_CHECK_RETURN(_s_read_tss(context, 4u,
            X86_CPU_REFERENCE_OF(newesp), 4u));
        CPU_TRACE_CHECK_RETURN(_s_read_tss(context, 8u,
            X86_CPU_REFERENCE_OF(newss), 2u));
        CPU_TRACE_CHECK_RETURN(_ser_check_call_gate_stack_sreg(context, newss,
            target_cpl, X86_CPU_REFERENCE_OF(ss_desc)));
        parameter_count = (lib_u8)_GetDescCall_Count(gate_desc);
        instruction_state.data.timing_transfer_parameters = parameter_count;
        newss_cache = cpu_state.data.ss;
        CPU_TRACE_CHECK_RETURN(_ksa_prepare_stack_sreg(context, newss,
            target_cpl, &newss_cache, ss_desc));
        CPU_TRACE_CHECK_RETURN(_s_test_stack_frame_32(context, &newss_cache,
            newesp, (lib_u8)(4u + parameter_count), target_cpl));
        for (index = 0u; index < parameter_count; ++index)
            CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context,
                (lib_u32)index * 4u,
                X86_CPU_REFERENCE_OF(parameters[index]), 4u));
        CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, newss,
            X86_CPU_REFERENCE_OF(ss_desc)));
        CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
            X86_CPU_REFERENCE_OF(code_desc)));
        cpu_state.data.ss = newss_cache;
        if (newss_cache.seg.data.big)
            _WriteGPR32(esp) = newesp;
        else
            _WriteGPR16(sp) = X86_CPU_MASK_U16(newesp);
        _MakeCPL(target_cpl);
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldss_frame),
            4u));
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldesp),
            4u));
        for (index = parameter_count; index > 0u; --index)
            CPU_TRACE_CHECK_RETURN(_kec_push(context,
                X86_CPU_REFERENCE_OF(parameters[index - 1u]), 4u));
    } else {
        CPU_TRACE_CHECK_RETURN(_s_test_ss_push(context, 8u));
        CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
            X86_CPU_REFERENCE_OF(code_desc)));
    }
    CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldcs_frame), 4u));
    CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldeip), 4u));
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.eip = X86_CPU_MASK_U32(_GetDescGate_Offset(gate_desc));
    CPU_TRACE_CALL_END;
}
static void _ser_call_far_call_gate(x86_core_cpu_execution_context *context,
    lib_u16 gate_selector, lib_u8 byte)
{
    lib_u64 gate_desc;
    lib_u64 code_desc;
    lib_u64 ss_desc;
    lib_u16 newcs;
    lib_u16 newss;
    lib_u16 newsp;
    lib_u32 newesp = 0;
    lib_u16 oldss;
    lib_u16 oldsp;
    lib_u16 oldcs;
    lib_u16 oldip;
    lib_u16 parameters[31];
    lib_u8 oldcpl;
    lib_u8 target_cpl;
    lib_u8 parameter_count;
    lib_u8 index;
    t_cpu_data_sreg newcs_cache;
    t_cpu_data_sreg newss_cache;

    CPU_TRACE_CALL_BEGIN("_ser_call_far_call_gate");
    if (!_IsProtected)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (byte == 4u) {
        CPU_TRACE_CHECK_RETURN(_ser_call_far_call_gate_32(context,
            gate_selector));
        CPU_TRACE_CALL_END;
        return;
    }
    if (byte != 2u)
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, gate_selector,
        X86_CPU_REFERENCE_OF(gate_desc)));
    if (!_IsDescCallGate16(gate_desc))
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(gate_selector & 0xfffcu));
    oldcpl = _GetCPL;
    if (_GetDesc_DPL(gate_desc) < oldcpl ||
        _GetDesc_DPL(gate_desc) < _GetSelector_RPL(gate_selector)) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(gate_selector & 0xfffcu));
    }
    if (!_IsDescPresent(gate_desc)) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(gate_selector & 0xfffcu));
    }
    newcs = X86_CPU_MASK_U16(_GetDescGate_Selector(gate_desc));
    if (_IsSelectorNull(newcs) || _GetSelector_TI(newcs)) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    }
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
        X86_CPU_REFERENCE_OF(code_desc)));
    if (!_IsDescCodeNonConform(code_desc)) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    }
    if (!_IsDescPresent(code_desc)) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
    }
    target_cpl = (lib_u8)_GetDesc_DPL(code_desc);
    if (target_cpl > oldcpl) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    }
    oldss = cpu_state.data.ss.selector;
    oldsp = cpu_state.data.sp;
    oldcs = cpu_state.data.cs.selector;
    oldip = cpu_state.data.ip;
    parameter_count = (lib_u8)_GetDescCall_Count(gate_desc);
    instruction_state.data.timing_transfer_parameters = parameter_count;
    newcs_cache = cpu_state.data.cs;
    CPU_TRACE_CHECK_RETURN(_ksa_prepare_code_sreg(context, newcs, target_cpl,
        &newcs_cache, code_desc));
    CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, &newcs_cache,
        X86_CPU_MASK_U16(_GetDescGate_Offset(gate_desc)), 1u, 0,
        target_cpl, 1));
    if (target_cpl < oldcpl) {
        if (!cpu_state.data.tr.flagValid) {
            CPU_TRACE_CHECK_RETURN(_SetExcept_TS(0));
        }
        switch (cpu_state.data.tr.sys.type) {
        case VCPU_DESC_SYS_TYPE_TSS_16_BUSY:
            CPU_TRACE_CHECK_RETURN(_s_read_tss(context, 2u,
                X86_CPU_REFERENCE_OF(newsp), 2u));
            CPU_TRACE_CHECK_RETURN(_s_read_tss(context, 4u,
                X86_CPU_REFERENCE_OF(newss), 2u));
            break;
        case VCPU_DESC_SYS_TYPE_TSS_32_BUSY:
            CPU_TRACE_CHECK_RETURN(_s_read_tss(context, 4u,
                X86_CPU_REFERENCE_OF(newesp), 4u));
            CPU_TRACE_CHECK_RETURN(_s_read_tss(context, 8u,
                X86_CPU_REFERENCE_OF(newss), 2u));
            if (newesp > 0xffffu) {
                CPU_TRACE_CHECK_RETURN(_SetExcept_TS(0));
            }
            newsp = X86_CPU_MASK_U16(newesp);
            break;
        default:
            CPU_TRACE_CHECK_RETURN(_SetExcept_TS(0));
            break;
        }
        CPU_TRACE_CHECK_RETURN(_ser_check_call_gate_stack_sreg(context, newss,
            target_cpl, X86_CPU_REFERENCE_OF(ss_desc)));
        newss_cache = cpu_state.data.ss;
        CPU_TRACE_CHECK_RETURN(_ksa_prepare_stack_sreg(context, newss,
            target_cpl, &newss_cache, ss_desc));
        CPU_TRACE_CHECK_RETURN(_s_test_stack_frame_16(context, &newss_cache,
            newsp, (lib_u8)(4u + parameter_count), target_cpl));
        for (index = 0u; index < parameter_count; ++index)
            CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context,
                (lib_u32)index * 2u,
                X86_CPU_REFERENCE_OF(parameters[index]), 2u));

        CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, newss,
            X86_CPU_REFERENCE_OF(ss_desc)));
        CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
            X86_CPU_REFERENCE_OF(code_desc)));
        cpu_state.data.ss = newss_cache;
        _WriteGPR16(sp) = newsp;
        _MakeCPL(target_cpl);
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldss), 2u));
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldsp), 2u));
        for (index = parameter_count; index > 0u; --index)
            CPU_TRACE_CHECK_RETURN(_kec_push(context,
                X86_CPU_REFERENCE_OF(parameters[index - 1u]), 2u));
    }
    else {
        CPU_TRACE_CHECK_RETURN(_s_test_ss_push(context, 4u));
        CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
            X86_CPU_REFERENCE_OF(code_desc)));
    }
    CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldcs), 2u));
    CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldip), 2u));
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.ip = X86_CPU_MASK_U16(_GetDescGate_Offset(gate_desc));
    CPU_TRACE_CALL_END;
}
static void _ser_task_switch_tss(x86_core_cpu_execution_context *context,
    lib_u16 newcs, lib_u8 nested, lib_u8 check_tss_privilege);
static void _ser_task_return_tss(x86_core_cpu_execution_context *context);

static void _ser_task_gate_descriptor(
    x86_core_cpu_execution_context *context, lib_u64 descriptor,
    lib_u16 gate_error, lib_u8 check_privilege,
    lib_u8 nested)
{
    lib_u16 target_selector;

    CPU_TRACE_CALL_BEGIN("_ser_task_gate_descriptor");
    if (!_IsDescTaskGate(descriptor))
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (check_privilege && _GetDesc_DPL(descriptor) < _GetCPL)
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(gate_error));
    if (!_IsDescPresent(descriptor))
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(gate_error));
    target_selector = X86_CPU_MASK_U16(_GetDescGate_Selector(descriptor));
    if (_IsSelectorNull(target_selector) || _GetSelector_TI(target_selector))
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(target_selector & 0xfffcu));
    CPU_TRACE_CHECK_RETURN(_ser_task_switch_tss(context, target_selector,
        nested, LIB_FALSE));
    CPU_TRACE_CALL_END;
}

static void _ser_task_gate_target(x86_core_cpu_execution_context *context,
    lib_u16 gate_selector, lib_u8 nested)
{
    lib_u64 descriptor;

    CPU_TRACE_CALL_BEGIN("_ser_task_gate_target");
    if (!_IsProtected || _GetEFLAGS_VM)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, gate_selector,
        X86_CPU_REFERENCE_OF(descriptor)));
    if (!_IsDescTaskGate(descriptor))
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (_GetDesc_DPL(descriptor) < _GetSelector_RPL(gate_selector))
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(gate_selector & 0xfffcu));
    CPU_TRACE_CHECK_RETURN(_ser_task_gate_descriptor(context, descriptor,
        gate_selector & 0xfffcu, LIB_TRUE, nested));
    CPU_TRACE_CALL_END;
}

static void _ser_call_far_task_gate(x86_core_cpu_execution_context *context,
    lib_u16 newcs)
{
    CPU_TRACE_CALL_BEGIN("_ser_call_far_task_gate");
    CPU_TRACE_CHECK_RETURN(_ser_task_gate_target(context, newcs, LIB_TRUE));
    CPU_TRACE_CALL_END;
}

static void _ser_call_far_tss(x86_core_cpu_execution_context *context,
    lib_u16 newcs)
{
    CPU_TRACE_CALL_BEGIN("_ser_call_far_tss");
    CPU_TRACE_CHECK_RETURN(_ser_task_switch_tss(context, newcs, LIB_TRUE,
        LIB_TRUE));
    CPU_TRACE_CALL_END;
}
static void _ser_int_real(x86_core_cpu_execution_context *context, lib_u8 intid, lib_u8 byte)
{
    lib_u16 cip;
    lib_u16 oldflags;
    lib_u32 vector;
    lib_u32 oldcs = cpu_state.data.cs.selector;
    CPU_TRACE_CALL_BEGIN("_ser_int_real");
    if (_IsProtected)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (cpu_profile_has_protected_architecture(context->cpu_profile) &&
        X86_CPU_MASK_U16(intid * 4 + 3) > X86_CPU_MASK_U16(cpu_state.data.idtr.limit))
    {
        CPU_TRACE_BLOCK_BEGIN("intid(>idtr.limit)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_DF(0));
        CPU_TRACE_BLOCK_END;
    }
    switch (byte)
    {
    case 2:
        CPU_TRACE_BLOCK_BEGIN("byte(2)");
        CPU_TRACE_CHECK_RETURN(_s_test_ss_push(context, 6));
        oldflags = _e_real_flags_image_16(context, cpu_state.data.flags);
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldflags), 2));
        _ClrEFLAGS_IF;
        _ClrEFLAGS_TF;
        _ClrEFLAGS_RF;
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldcs), 2));
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.ip), 2));
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("byte(4)");
        CPU_TRACE_CHECK_RETURN(_s_test_ss_push(context, 12));
        {
            lib_u32 frame_flags =
                (cpu_state.data.eflags & ~x86_core_cpu_eflags_reserved_mask(context->cpu_profile)) | 0x02u;
            CPU_TRACE_CHECK_RETURN(_kec_push(context,
                X86_CPU_REFERENCE_OF(frame_flags), 4));
        }
        _ClrEFLAGS_IF;
        _ClrEFLAGS_TF;
        _ClrEFLAGS_RF;
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldcs), 4));
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.eip), 4));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("byte");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CHECK_RETURN(_s_read_idt(context, intid, X86_CPU_REFERENCE_OF(vector)));
    cip = X86_CPU_MASK_U16(vector);
    /* The word target belongs to the CS reloaded below, not the old cache. */
    cpu_state.data.eip = cip;
    CPU_TRACE_CHECK_RETURN(_s_load_cs(context, X86_CPU_MASK_U16(vector >> 16)));
    CPU_TRACE_CALL_END;
}
static lib_u16 _ser_idt_error_code(lib_u8 intid)
{
    /* Entry wrappers add EXT for failed hardware/exception delivery. */
    return X86_CPU_MASK_U16(intid * 8u + 2u);
}
static void _ser_int_protected_16(x86_core_cpu_execution_context *context,
    lib_u8 intid, lib_u64 gate_desc, lib_u8 software_origin, lib_u8 error_frame)
{
    lib_u64 code_desc;
    lib_u64 ss_desc;
    lib_u16 newcs;
    lib_u16 newss;
    lib_u16 newsp;
    lib_u32 newesp;
    lib_u16 oldss;
    lib_u16 oldsp;
    lib_u16 oldcs;
    lib_u16 oldip;
    lib_u16 oldflags;
    lib_u16 error_code;
    lib_u8 oldcpl;
    lib_u8 target_cpl;
    lib_u8 frame_words;
    lib_u8 interrupt_gate;
    t_cpu_data_sreg newcs_cache;
    t_cpu_data_sreg newss_cache;

    CPU_TRACE_CALL_BEGIN("_ser_int_protected_16");
    if (!_IsProtected || X86_CPU_MASK_U16(intid * 8u + 7u) >
        X86_CPU_MASK_U16(cpu_state.data.idtr.limit)) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(_ser_idt_error_code(intid)));
    }
    interrupt_gate = _GetDesc_Type(gate_desc) == VCPU_DESC_SYS_TYPE_INTGATE_16;
    if (!interrupt_gate && _GetDesc_Type(gate_desc) !=
        VCPU_DESC_SYS_TYPE_TRAPGATE_16) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(_ser_idt_error_code(intid)));
    }
    if (!_IsDescPresent(gate_desc)) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(_ser_idt_error_code(intid)));
    }
    oldcpl = _GetCPL;
    if (software_origin && _GetDesc_DPL(gate_desc) < oldcpl) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(_ser_idt_error_code(intid)));
    }
    newcs = X86_CPU_MASK_U16(_GetDescGate_Selector(gate_desc));
    if (_IsSelectorNull(newcs) || _GetSelector_TI(newcs)) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    }
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
        X86_CPU_REFERENCE_OF(code_desc)));
    if (!_IsDescCodeNonConform(code_desc))
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    if (!_IsDescPresent(code_desc))
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
    target_cpl = (lib_u8)_GetDesc_DPL(code_desc);
    if (target_cpl > oldcpl) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    }

    oldss = cpu_state.data.ss.selector;
    oldsp = cpu_state.data.sp;
    oldcs = cpu_state.data.cs.selector;
    oldip = cpu_state.data.ip;
    oldflags = cpu_state.data.flags;
    error_code = X86_CPU_MASK_U16(instruction_state.data.excode);
    newcs_cache = cpu_state.data.cs;
    CPU_TRACE_CHECK_RETURN(_ksa_prepare_code_sreg(context, newcs, target_cpl,
        &newcs_cache, code_desc));
    CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, &newcs_cache,
        X86_CPU_MASK_U16(_GetDescGate_Offset(gate_desc)), 1u, 0,
        target_cpl, 1));
    frame_words = (lib_u8)(3u + (error_frame ? 1u : 0u));
    if (target_cpl < oldcpl) {
        if (!cpu_state.data.tr.flagValid) {
            CPU_TRACE_CHECK_RETURN(_SetExcept_TS(0));
        }
        switch (cpu_state.data.tr.sys.type) {
        case VCPU_DESC_SYS_TYPE_TSS_16_BUSY:
            CPU_TRACE_CHECK_RETURN(_s_read_tss(context, 2u,
                X86_CPU_REFERENCE_OF(newsp), 2u));
            CPU_TRACE_CHECK_RETURN(_s_read_tss(context, 4u,
                X86_CPU_REFERENCE_OF(newss), 2u));
            break;
        case VCPU_DESC_SYS_TYPE_TSS_32_BUSY:
            CPU_TRACE_CHECK_RETURN(_s_read_tss(context, 4u,
                X86_CPU_REFERENCE_OF(newesp), 4u));
            CPU_TRACE_CHECK_RETURN(_s_read_tss(context, 8u,
                X86_CPU_REFERENCE_OF(newss), 2u));
            if (newesp > 0xffffu) {
                CPU_TRACE_CHECK_RETURN(_SetExcept_TS(0));
            }
            newsp = X86_CPU_MASK_U16(newesp);
            break;
        default:
            CPU_TRACE_CHECK_RETURN(_SetExcept_TS(0));
            break;
        }
        if (_IsSelectorNull(newss) || _GetSelector_TI(newss) ||
            _GetSelector_RPL(newss) != target_cpl) {
            CPU_TRACE_CHECK_RETURN(_SetExcept_TS(newss & 0xfffcu));
        }
        CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newss,
            X86_CPU_REFERENCE_OF(ss_desc)));
        newss_cache = cpu_state.data.ss;
        CPU_TRACE_CHECK_RETURN(_ksa_prepare_stack_sreg(context, newss,
            target_cpl, &newss_cache, ss_desc));
        CPU_TRACE_CHECK_RETURN(_s_test_stack_frame_16(context, &newss_cache,
            newsp, (lib_u8)(frame_words + 2u), target_cpl));

        /* All faulting reads and writes are preflighted before CPU mutation. */
        CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, newss,
            X86_CPU_REFERENCE_OF(ss_desc)));
        CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
            X86_CPU_REFERENCE_OF(code_desc)));
        cpu_state.data.ss = newss_cache;
        _WriteGPR16(sp) = newsp;
        _MakeCPL(target_cpl);
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldss), 2u));
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldsp), 2u));
    }
    else {
        CPU_TRACE_CHECK_RETURN(_s_test_ss_push(context,
            (lib_u8)(frame_words * 2u)));
        CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
            X86_CPU_REFERENCE_OF(code_desc)));
    }
    CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldflags), 2u));
    CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldcs), 2u));
    CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldip), 2u));
    if (error_frame) {
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(error_code),
            2u));
    }
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.ip = X86_CPU_MASK_U16(_GetDescGate_Offset(gate_desc));
    if (interrupt_gate) _ClrEFLAGS_IF;
    _ClrEFLAGS_TF;
    _ClrEFLAGS_NT;
    _ClrEFLAGS_RF;
    CPU_TRACE_CALL_END;
}
static void _ser_int_protected_32_outer(x86_core_cpu_execution_context *context,
    lib_u8 intid, lib_u64 gate_desc, lib_u64 code_desc, lib_u8 software_origin,
    lib_u8 error_frame)
{
    lib_u64 ss_desc;
    lib_u16 newcs;
    lib_u16 newss;
    lib_u16 oldcs;
    lib_u16 oldss;
    lib_u16 oldes;
    lib_u16 oldds;
    lib_u16 oldfs;
    lib_u16 oldgs;
    lib_u32 newesp;
    lib_u32 oldeip;
    lib_u32 oldesp;
    lib_u32 oldeflags;
    lib_u32 oldcs_frame;
    lib_u32 oldss_frame;
    lib_u32 oldes_frame;
    lib_u32 oldds_frame;
    lib_u32 oldfs_frame;
    lib_u32 oldgs_frame;
    lib_u32 error_code;
    lib_u8 oldcpl;
    lib_u8 target_cpl;
    lib_u8 frame_dwords;
    lib_u8 interrupt_gate;
    lib_u8 vm86_origin;
    t_cpu_data_sreg newcs_cache;
    t_cpu_data_sreg newss_cache;

    CPU_TRACE_CALL_BEGIN("_ser_int_protected_32_outer");
    interrupt_gate = _GetDesc_Type(gate_desc) == VCPU_DESC_SYS_TYPE_INTGATE_32;
    if (!interrupt_gate && _GetDesc_Type(gate_desc) !=
        VCPU_DESC_SYS_TYPE_TRAPGATE_32)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (!_IsDescPresent(gate_desc))
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(_ser_idt_error_code(intid)));
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
        X86_CPU_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_VM);
    }
    if (software_origin && _GetDesc_DPL(gate_desc) < oldcpl)
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(_ser_idt_error_code(intid)));
    newcs = X86_CPU_MASK_U16(_GetDescGate_Selector(gate_desc));
    if (_IsSelectorNull(newcs) || _GetSelector_TI(newcs))
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    if (!_IsDescCodeNonConform(code_desc) || _GetDesc_DPL(code_desc) > oldcpl)
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    target_cpl = (lib_u8)_GetDesc_DPL(code_desc);
    if (target_cpl >= oldcpl)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (!_IsDescPresent(code_desc))
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
    if (!cpu_state.data.tr.flagValid || cpu_state.data.tr.sys.type !=
        VCPU_DESC_SYS_TYPE_TSS_32_BUSY)
        CPU_TRACE_CHECK_RETURN(_SetExcept_TS(0));
    CPU_TRACE_CHECK_RETURN(_s_test_tss(context, 4u, 6u));
    CPU_TRACE_CHECK_RETURN(_s_read_tss(context, 4u, X86_CPU_REFERENCE_OF(newesp),
        4u));
    CPU_TRACE_CHECK_RETURN(_s_read_tss(context, 8u, X86_CPU_REFERENCE_OF(newss),
        2u));
    if (_IsSelectorNull(newss) || _GetSelector_TI(newss) ||
        _GetSelector_RPL(newss) != target_cpl)
        CPU_TRACE_CHECK_RETURN(_SetExcept_TS(newss & 0xfffcu));

    if (!vm86_origin) {
        oldcs = cpu_state.data.cs.selector;
        oldss = cpu_state.data.ss.selector;
        oldeip = cpu_state.data.eip;
        oldesp = cpu_state.data.esp;
        oldeflags = cpu_state.data.eflags;
        oldcs_frame = oldcs;
        oldss_frame = oldss;
    }
    error_code = X86_CPU_MASK_U32(instruction_state.data.excode);
    frame_dwords = (lib_u8)(5u + (vm86_origin ? 4u : 0u) +
        (error_frame ? 1u : 0u));
    newcs_cache = cpu_state.data.cs;
    CPU_TRACE_CHECK_RETURN(_ksa_prepare_code_sreg(context, newcs, target_cpl,
        &newcs_cache, code_desc));
    CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, &newcs_cache,
        X86_CPU_MASK_U32(_GetDescGate_Offset(gate_desc)), 1u, 0,
        target_cpl, 1));
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newss,
        X86_CPU_REFERENCE_OF(ss_desc)));
    newss_cache = cpu_state.data.ss;
    CPU_TRACE_CHECK_RETURN(_ksa_prepare_stack_sreg(context, newss, target_cpl,
        &newss_cache, ss_desc));
    CPU_TRACE_CHECK_RETURN(_s_test_stack_frame_32(context, &newss_cache,
        newesp, frame_dwords, target_cpl));

    CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, newss,
        X86_CPU_REFERENCE_OF(ss_desc)));
    CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
        X86_CPU_REFERENCE_OF(code_desc)));
    cpu_state.data.ss = newss_cache;
    if ((context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX && vm86_origin) ||
        newss_cache.seg.data.big)
        _WriteGPR32(esp) = newesp;
    else
        _WriteGPR16(sp) = X86_CPU_MASK_U16(newesp);
    _MakeCPL(target_cpl);
    if (vm86_origin) {
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldgs_frame), 4u));
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldfs_frame), 4u));
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldds_frame), 4u));
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldes_frame), 4u));
    }
    CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldss_frame), 4u));
    CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldesp), 4u));
    CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldeflags), 4u));
    CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldcs_frame), 4u));
    CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldeip), 4u));
    if (error_frame)
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(error_code),
            4u));
    if (vm86_origin) {
        lib_memory_set(&cpu_state.data.es, 0, sizeof(cpu_state.data.es));
        lib_memory_set(&cpu_state.data.ds, 0, sizeof(cpu_state.data.ds));
        lib_memory_set(&cpu_state.data.fs, 0, sizeof(cpu_state.data.fs));
        lib_memory_set(&cpu_state.data.gs, 0, sizeof(cpu_state.data.gs));
    }
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.eip = X86_CPU_MASK_U32(_GetDescGate_Offset(gate_desc));
    if (interrupt_gate) _ClrEFLAGS_IF;
    _ClrEFLAGS_TF;
    _ClrEFLAGS_NT;
    _ClrEFLAGS_RF;
    CPU_TRACE_CALL_END;
}
static void _ser_int_protected_32_same(x86_core_cpu_execution_context *context,
    lib_u8 intid, lib_u64 gate_desc, lib_u8 software_origin,
    lib_u8 error_frame)
{
    lib_u64 code_desc;
    lib_u16 newcs;
    lib_u16 oldcs;
    lib_u32 oldeip;
    lib_u32 oldeflags;
    lib_u32 oldcs_frame;
    lib_u32 error_code;
    lib_u8 oldcpl;
    lib_u8 interrupt_gate;
    t_cpu_data_sreg newcs_cache;

    CPU_TRACE_CALL_BEGIN("_ser_int_protected_32_same");
    interrupt_gate = _GetDesc_Type(gate_desc) == VCPU_DESC_SYS_TYPE_INTGATE_32;
    if (!interrupt_gate && _GetDesc_Type(gate_desc) !=
        VCPU_DESC_SYS_TYPE_TRAPGATE_32)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (!_IsDescPresent(gate_desc))
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(_ser_idt_error_code(intid)));
    oldcpl = _GetCPL;
    if (software_origin && _GetDesc_DPL(gate_desc) < oldcpl)
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(_ser_idt_error_code(intid)));
    newcs = X86_CPU_MASK_U16(_GetDescGate_Selector(gate_desc));
    if (_IsSelectorNull(newcs) || _GetSelector_TI(newcs))
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
        X86_CPU_REFERENCE_OF(code_desc)));
    if (!_IsDescCodeNonConform(code_desc) || _GetDesc_DPL(code_desc) > oldcpl)
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    if (_GetDesc_DPL(code_desc) < oldcpl) {
        CPU_TRACE_CHECK_RETURN(_ser_int_protected_32_outer(context, intid,
            gate_desc, code_desc, software_origin, error_frame));
        CPU_TRACE_CALL_END;
        return;
    }
    if (!_IsDescPresent(code_desc))
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));

    oldcs = cpu_state.data.cs.selector;
    oldeip = cpu_state.data.eip;
    oldeflags = cpu_state.data.eflags;
    oldcs_frame = oldcs;
    error_code = X86_CPU_MASK_U32(instruction_state.data.excode);
    newcs_cache = cpu_state.data.cs;
    CPU_TRACE_CHECK_RETURN(_ksa_prepare_code_sreg(context, newcs, oldcpl,
        &newcs_cache, code_desc));
    CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, &newcs_cache,
        X86_CPU_MASK_U32(_GetDescGate_Offset(gate_desc)), 1u, 0,
        oldcpl, 1));
    CPU_TRACE_CHECK_RETURN(_s_test_ss_push(context, error_frame ? 16u : 12u));

    /* Descriptor access and each frame write are preflighted before state publish. */
    CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
        X86_CPU_REFERENCE_OF(code_desc)));
    CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldeflags), 4u));
    CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldcs_frame), 4u));
    CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(oldeip), 4u));
    if (error_frame)
        CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(error_code), 4u));
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.eip = X86_CPU_MASK_U32(_GetDescGate_Offset(gate_desc));
    if (interrupt_gate) _ClrEFLAGS_IF;
    _ClrEFLAGS_TF;
    _ClrEFLAGS_NT;
    _ClrEFLAGS_RF;
    CPU_TRACE_CALL_END;
}
static void _ser_int_protected(x86_core_cpu_execution_context *context,
    lib_u8 intid, lib_u8 byte, lib_u8 software_origin,
    lib_u8 flagext, lib_bool *out_task_entry)
{
    lib_u64 gate_desc;

    CPU_TRACE_CALL_BEGIN("_ser_int_protected");
    if (out_task_entry != LIB_NULL) *out_task_entry = LIB_FALSE;
    if (!_GetCR0_PE)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (byte != 2u && byte != 4u)
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
    if (X86_CPU_MASK_U16(intid * 8u + 7u) >
        X86_CPU_MASK_U16(cpu_state.data.idtr.limit))
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(_ser_idt_error_code(intid)));
    CPU_TRACE_CHECK_RETURN(_s_read_idt(context, intid,
        X86_CPU_REFERENCE_OF(gate_desc)));
    /* VM86 uses a task gate or a 32-bit interrupt/trap gate. */
    if (_GetEFLAGS_VM && _GetDesc_Type(gate_desc) !=
        VCPU_DESC_SYS_TYPE_INTGATE_32 && _GetDesc_Type(gate_desc) !=
        VCPU_DESC_SYS_TYPE_TRAPGATE_32 && _GetDesc_Type(gate_desc) !=
        VCPU_DESC_SYS_TYPE_TASKGATE)
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(_ser_idt_error_code(intid)));
    /* An unavailable target-stack facility is an architectural task fault. */
    if (_GetEFLAGS_VM && (!cpu_state.data.tr.flagValid ||
        cpu_state.data.tr.sys.type != VCPU_DESC_SYS_TYPE_TSS_32_BUSY))
        CPU_TRACE_CHECK_RETURN(_SetExcept_TS(0));
    switch (_GetDesc_Type(gate_desc)) {
    case VCPU_DESC_SYS_TYPE_INTGATE_16:
    case VCPU_DESC_SYS_TYPE_TRAPGATE_16:
        CPU_TRACE_CHECK_RETURN(_ser_int_protected_16(context, intid,
            gate_desc, software_origin, flagext));
        break;
    case VCPU_DESC_SYS_TYPE_INTGATE_32:
    case VCPU_DESC_SYS_TYPE_TRAPGATE_32:
        if (!cpu_profile_has_32bit_architecture(context->cpu_profile))
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(_ser_idt_error_code(intid)));
        CPU_TRACE_CHECK_RETURN(_ser_int_protected_32_same(context, intid,
            gate_desc, software_origin, flagext));
        break;
    case VCPU_DESC_SYS_TYPE_TASKGATE:
        if (out_task_entry != LIB_NULL) *out_task_entry = LIB_TRUE;
        CPU_TRACE_CHECK_RETURN(_ser_task_gate_descriptor(context, gate_desc,
            _ser_idt_error_code(intid), software_origin, LIB_TRUE));
        if (flagext) {
            const lib_u32 error = instruction_state.data.excode;
            CPU_TRACE_CHECK_RETURN(_kec_push(context, X86_CPU_REFERENCE_OF(error),
                cpu_state.data.tr.sys.type == VCPU_DESC_SYS_TYPE_TSS_16_BUSY ? 2u : 4u));
        }
        break;
    default:
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(_ser_idt_error_code(intid)));
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _ser_ret_far_real(x86_core_cpu_execution_context *context, lib_u16 newcs, lib_u32 neweip, lib_u16 parambyte, lib_u16 byte)
{
    CPU_TRACE_CALL_BEGIN("_ser_ret_far_real");
    if (_IsProtected)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CHECK_RETURN(_kec_ret_far(context, newcs, neweip, parambyte, byte));
    CPU_TRACE_CALL_END;
}
static void _ser_ret_far_same(x86_core_cpu_execution_context *context, lib_u16 newcs, lib_u32 neweip, lib_u16 parambyte, lib_u16 byte)
{
    CPU_TRACE_CALL_BEGIN("_ser_ret_far_same");
    if (!_IsProtected)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CHECK_RETURN(_kec_ret_far(context, newcs, neweip, parambyte, byte));
    CPU_TRACE_CALL_END;
}
static void _ser_return_outer_cleanup_sreg(t_cpu_data_sreg *rsreg,
    lib_u8 target_cpl)
{
    CPU_TRACE_CALL_BEGIN("_ser_return_outer_cleanup_sreg");
    if (rsreg->flagValid &&
        (!rsreg->seg.executable || !rsreg->seg.exec.conform) &&
        rsreg->dpl < target_cpl) {
        rsreg->flagValid = LIB_FALSE;
        rsreg->selector = 0u;
    }
    CPU_TRACE_CALL_END;
}

static void _ser_return_outer_cleanup_sregs(
    x86_core_cpu_execution_context *context, lib_u8 target_cpl)
{
    CPU_TRACE_CALL_BEGIN("_ser_return_outer_cleanup_sregs");
    _ser_return_outer_cleanup_sreg(&cpu_state.data.ds, target_cpl);
    _ser_return_outer_cleanup_sreg(&cpu_state.data.es, target_cpl);
    if (cpu_profile_has_32bit_architecture(context->cpu_profile)) {
        _ser_return_outer_cleanup_sreg(&cpu_state.data.fs, target_cpl);
        _ser_return_outer_cleanup_sreg(&cpu_state.data.gs, target_cpl);
    }
    CPU_TRACE_CALL_END;
}

static void _ser_ret_far_outer(x86_core_cpu_execution_context *context,
    lib_u16 newcs, lib_u32 neweip, lib_u16 parambyte,
    lib_u16 byte)
{
    lib_u64 code_desc;
    lib_u64 ss_desc;
    lib_u16 newss;
    lib_u32 newesp;
    lib_u32 xs_sel;
    t_cpu_data_sreg newcs_cache;
    t_cpu_data_sreg newss_cache;
    lib_u8 target_cpl;

    CPU_TRACE_CALL_BEGIN("_ser_ret_far_outer");
    if (!_IsProtected)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    target_cpl = (lib_u8)_GetSelector_RPL(newcs);
    switch (byte)
    {
    case 2:
        CPU_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 4u));
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context,
            (lib_u32)parambyte + 4u, X86_CPU_REFERENCE_OF(newesp), 2u));
        xs_sel = 0u;
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context,
            (lib_u32)parambyte + 6u, X86_CPU_REFERENCE_OF(xs_sel), 2u));
        neweip = X86_CPU_MASK_U16(neweip);
        newesp = X86_CPU_MASK_U16(newesp);
        break;
    case 4:
        CPU_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 8u));
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context,
            (lib_u32)parambyte + 8u, X86_CPU_REFERENCE_OF(newesp), 4u));
        xs_sel = 0u;
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context,
            (lib_u32)parambyte + 12u, X86_CPU_REFERENCE_OF(xs_sel), 4u));
        break;
    default:
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        break;
    }
    newss = X86_CPU_MASK_U16(xs_sel);
    if (_IsSelectorNull(newss) || _GetSelector_TI(newss) ||
        _GetSelector_RPL(newss) != target_cpl) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newss & 0xfffcu));
    }
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newss,
        X86_CPU_REFERENCE_OF(ss_desc)));
    newss_cache = cpu_state.data.ss;
    CPU_TRACE_CHECK_RETURN(_ksa_prepare_stack_sreg(context, newss, target_cpl,
        &newss_cache, ss_desc));
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
        X86_CPU_REFERENCE_OF(code_desc)));
    if (!_IsDescCode(code_desc) ||
        (_IsDescCodeNonConform(code_desc) &&
            _GetDesc_DPL(code_desc) != target_cpl) ||
        (_IsDescCodeConform(code_desc) &&
            _GetDesc_DPL(code_desc) > target_cpl))
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    if (!_IsDescPresent(code_desc))
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
    newcs_cache = cpu_state.data.cs;
    CPU_TRACE_CHECK_RETURN(_ksa_prepare_code_sreg(context, newcs, target_cpl,
        &newcs_cache, code_desc));
    CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, &newcs_cache,
        neweip, 1u, 0, target_cpl, 1));
    CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, newss,
        X86_CPU_REFERENCE_OF(ss_desc)));
    CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
        X86_CPU_REFERENCE_OF(code_desc)));
    cpu_state.data.ss = newss_cache;
    if (newss_cache.seg.data.big)
        _WriteGPR32(esp) = newesp + parambyte;
    else
        _WriteGPR16(sp) = X86_CPU_MASK_U16(newesp + parambyte);
    _MakeCPL(target_cpl);
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.eip = neweip;
    _ser_return_outer_cleanup_sregs(context, target_cpl);
    CPU_TRACE_CALL_END;
}
static void _ser_jmp_far_real(x86_core_cpu_execution_context *context, lib_u16 newcs, lib_u32 neweip, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_ser_jmp_far_real");
    if (_IsProtected)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CHECK_RETURN(_kec_jmp_far(context, newcs, neweip, byte));
    CPU_TRACE_CALL_END;
}
static void _ser_jmp_far_cs_conf(x86_core_cpu_execution_context *context, lib_u16 newcs, lib_u32 neweip, lib_u8 byte)
{
    lib_u64 descriptor;
    CPU_TRACE_CALL_BEGIN("_ser_jmp_far_cs_conf");
    if (!_IsProtected)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, X86_CPU_REFERENCE_OF(descriptor)));
    if (!_IsDescCodeConform(descriptor))
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (_GetDesc_DPL(descriptor) > _GetCPL)
    {
        CPU_TRACE_BLOCK_BEGIN("DPL(>CPL)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
        CPU_TRACE_BLOCK_END;
    }
    if (!_IsDescPresent(descriptor))
    {
        CPU_TRACE_BLOCK_BEGIN("!DescPresent");
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CHECK_RETURN(_kec_jmp_far(context, newcs, neweip, byte));
    CPU_TRACE_CALL_END;
}
static void _ser_jmp_far_cs_nonc(x86_core_cpu_execution_context *context, lib_u16 newcs, lib_u32 neweip, lib_u8 byte)
{
    lib_u64 descriptor;
    CPU_TRACE_CALL_BEGIN("_ser_jmp_far_cs_nonc");
    if (!_IsProtected)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, X86_CPU_REFERENCE_OF(descriptor)));
    if (!_IsDescCodeNonConform(descriptor))
        CPU_TRACE_IMPOSSIBLE_RETURN;
    if (_GetDesc_DPL(descriptor) != _GetCPL ||
        _GetSelector_RPL(newcs) > _GetCPL)
    {
        CPU_TRACE_BLOCK_BEGIN("DPL(!CPL)/RPL(>CPL)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
        CPU_TRACE_BLOCK_END;
    }
    if (!_IsDescPresent(descriptor))
    {
        CPU_TRACE_BLOCK_BEGIN("!DescPresent");
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CHECK_RETURN(_kec_jmp_far(context, newcs, neweip, byte));
    CPU_TRACE_CALL_END;
}
static void _ser_jmp_far_call_gate(x86_core_cpu_execution_context *context,
    lib_u16 gate_selector)
{
    lib_u64 gate_desc;
    lib_u64 code_desc;
    lib_u16 target_selector;
    lib_u8 cpl;
    lib_u8 gate32;
    t_cpu_data_sreg newcs_cache;

    CPU_TRACE_CALL_BEGIN("_ser_jmp_far_call_gate");
    if (!_IsProtected)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, gate_selector,
        X86_CPU_REFERENCE_OF(gate_desc)));
    /* The 80386 extends the same-privilege JMP gate route to both 16- and
     * 32-bit call gates.  It transfers directly to the target: no return
     * frame, stack switch, or parameter copy is performed. */
    gate32 = _IsDescCallGate32(gate_desc);
    if (!_IsDescCallGate16(gate_desc) && !gate32)
        CPU_TRACE_IMPOSSIBLE_RETURN;
    cpl = _GetCPL;
    if (_GetDesc_DPL(gate_desc) < cpl ||
        _GetDesc_DPL(gate_desc) < _GetSelector_RPL(gate_selector)) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(gate_selector & 0xfffcu));
    }
    if (!_IsDescPresent(gate_desc)) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(gate_selector & 0xfffcu));
    }
    target_selector = X86_CPU_MASK_U16(_GetDescGate_Selector(gate_desc));
    if (_IsSelectorNull(target_selector) || _GetSelector_TI(target_selector)) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(target_selector & 0xfffcu));
    }
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, target_selector,
        X86_CPU_REFERENCE_OF(code_desc)));
    if (!_IsDescCodeNonConform(code_desc) || _GetDesc_DPL(code_desc) != cpl) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(target_selector & 0xfffcu));
    }
    if (!_IsDescPresent(code_desc)) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(target_selector & 0xfffcu));
    }
    newcs_cache = cpu_state.data.cs;
    CPU_TRACE_CHECK_RETURN(_ksa_prepare_code_sreg(context, target_selector,
        cpl, &newcs_cache, code_desc));
    CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, &newcs_cache,
        gate32 ? X86_CPU_MASK_U32(_GetDescGate_Offset(gate_desc)) :
        X86_CPU_MASK_U16(_GetDescGate_Offset(gate_desc)), 1u, 0, cpl, 1));
    CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, target_selector,
        X86_CPU_REFERENCE_OF(code_desc)));
    cpu_state.data.cs = newcs_cache;
    if (gate32) cpu_state.data.eip = X86_CPU_MASK_U32(
        _GetDescGate_Offset(gate_desc));
    else cpu_state.data.ip = X86_CPU_MASK_U16(_GetDescGate_Offset(gate_desc));
    CPU_TRACE_CALL_END;
}
static void _ser_jmp_far_task_gate(x86_core_cpu_execution_context *context,
    lib_u16 newcs)
{
    CPU_TRACE_CALL_BEGIN("_ser_jmp_far_task_gate");
    CPU_TRACE_CHECK_RETURN(_ser_task_gate_target(context, newcs, LIB_FALSE));
    CPU_TRACE_CALL_END;
}
typedef struct task_switch_state_16 {
    lib_u16 ip;
    lib_u16 flags;
    lib_u16 ax;
    lib_u16 cx;
    lib_u16 dx;
    lib_u16 bx;
    lib_u16 sp;
    lib_u16 bp;
    lib_u16 si;
    lib_u16 di;
    lib_u16 es;
    lib_u16 cs;
    lib_u16 ss;
    lib_u16 ds;
    lib_u16 ldtr;
} task_switch_state_16;

static void _s_task_cache_descriptor(t_cpu_data_sreg *cache,
    lib_u16 selector, lib_u64 descriptor,
    t_cpu_data_sreg_type sregtype)
{
    CPU_TRACE_CALL_BEGIN("_s_task_cache_descriptor");
    lib_memory_set(cache, 0, sizeof(*cache));
    cache->flagValid = LIB_TRUE;
    cache->selector = selector;
    cache->sregtype = sregtype;
    cache->base = (lib_u32)_GetDescSeg_Base(descriptor);
    cache->limit = (lib_u32)(_IsDescSegGranularLarge(descriptor) ?
        (_GetDescSeg_Limit(descriptor) << 12u | 0x0fffu) :
        _GetDescSeg_Limit(descriptor));
    cache->dpl = (lib_u8)_GetDesc_DPL(descriptor);
    if (sregtype == SREG_TR || sregtype == SREG_LDTR) {
        cache->sys.type = (lib_u8)_GetDesc_Type(descriptor);
    } else if (_IsDescCode(descriptor)) {
        cache->seg.accessed = (lib_u8)_IsDescUserAccessed(descriptor);
        cache->seg.executable = LIB_TRUE;
        cache->seg.exec.defsize = (lib_u8)_IsDescCode32(descriptor);
        cache->seg.exec.conform = (lib_u8)_IsDescCodeConform(descriptor);
        cache->seg.exec.readable = (lib_u8)_IsDescCodeReadable(descriptor);
    } else {
        cache->seg.accessed = (lib_u8)_IsDescUserAccessed(descriptor);
        cache->seg.executable = LIB_FALSE;
        cache->seg.data.big = (lib_u8)_IsDescDataBig(descriptor);
        cache->seg.data.expdown = (lib_u8)_IsDescDataExpDown(descriptor);
        cache->seg.data.writable = (lib_u8)_IsDescDataWritable(descriptor);
    }
    CPU_TRACE_CALL_END;
}

/* A task switch installs selector values before the corresponding descriptor
 * checks.  Keep an unavailable cache explicit until its own check succeeds;
 * late task-switch faults must retain that incoming-task image. */
static void _s_task_set_unavailable(t_cpu_data_sreg *cache,
    lib_u16 selector, t_cpu_data_sreg_type sregtype)
{
    lib_memory_set(cache, 0, sizeof(*cache));
    cache->selector = selector;
    cache->sregtype = sregtype;
}

static void _s_task_checkpoint(
    x86_core_cpu_execution_context *context)
{
    context->instruction_task_checkpoint = cpu_state;
    context->instruction_task_switched = LIB_TRUE;
}

static void _s_task_prepare_ldtr(x86_core_cpu_execution_context *context,
    lib_u16 selector, t_cpu_data_sreg *out_cache)
{
    lib_u64 descriptor;

    CPU_TRACE_CALL_BEGIN("_s_task_prepare_ldtr");
    lib_memory_set(out_cache, 0, sizeof(*out_cache));
    out_cache->sregtype = SREG_LDTR;
    if (_IsSelectorNull(selector)) {
        out_cache->selector = selector;
        CPU_TRACE_CALL_END;
        return;
    }
    if (_GetSelector_TI(selector))
        CPU_TRACE_CHECK_RETURN(_SetExcept_TS(selector & 0xfffcu));
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, selector,
        X86_CPU_REFERENCE_OF(descriptor)));
    if (!_IsDescLDT(descriptor))
        CPU_TRACE_CHECK_RETURN(_SetExcept_TS(selector & 0xfffcu));
    if (!_IsDescPresent(descriptor))
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(selector & 0xfffcu));
    _s_task_cache_descriptor(out_cache, selector, descriptor, SREG_LDTR);
    CPU_TRACE_CALL_END;
}

static void _s_task_read_selector(x86_core_cpu_execution_context *context,
    t_cpu_data_sreg *ldtr, lib_u16 selector,
    lib_uptr rdata)
{
    CPU_TRACE_CALL_BEGIN("_s_task_read_selector");
    if (!_GetSelector_TI(selector)) {
        CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, selector, rdata));
        CPU_TRACE_CALL_END;
        return;
    }
    if (!ldtr->flagValid || X86_CPU_MASK_U16(_GetSelector_Offset(selector) +
        7u) > X86_CPU_MASK_U16(ldtr->limit))
        CPU_TRACE_CHECK_RETURN(_SetExcept_TS(selector & 0xfffcu));
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
        CPU_TRACE_CHECK_RETURN(_ksa_read_descriptor(context,
            ldtr, _GetSelector_Offset(selector), rdata));
    else
        CPU_TRACE_CHECK_RETURN(_kma_read_logical(context, ldtr,
            _GetSelector_Offset(selector), rdata, 8u, 0u, LIB_TRUE));
    CPU_TRACE_CALL_END;
}

static void _s_task_validate_data_selector(
    x86_core_cpu_execution_context *context, t_cpu_data_sreg *ldtr,
    lib_u16 selector, t_cpu_data_sreg_type sregtype, lib_u8 cpl,
    t_cpu_data_sreg *out_cache)
{
    lib_u64 descriptor = 0u;

    CPU_TRACE_CALL_BEGIN("_s_task_validate_data_selector");
    if (_GetEFLAGS_VM) {
        lib_memory_set(out_cache, 0, sizeof(*out_cache));
        out_cache->sregtype = sregtype;
        _ksa_load_real_sreg(context, out_cache, selector, LIB_TRUE);
        CPU_TRACE_CALL_END;
        return;
    }
    if (_IsSelectorNull(selector)) {
        if (sregtype == SREG_STACK)
            CPU_TRACE_CHECK_RETURN(_SetExcept_TS(0));
        lib_memory_set(out_cache, 0, sizeof(*out_cache));
        out_cache->selector = selector;
        out_cache->sregtype = sregtype;
        CPU_TRACE_CALL_END;
        return;
    }
    CPU_TRACE_CHECK_RETURN(_s_task_read_selector(context, ldtr, selector,
        X86_CPU_REFERENCE_OF(descriptor)));
    if (sregtype == SREG_STACK) {
        if (!_IsDescDataWritable(descriptor) ||
            _GetSelector_RPL(selector) != cpl ||
            _GetDesc_DPL(descriptor) != cpl)
            CPU_TRACE_CHECK_RETURN(_SetExcept_TS(selector & 0xfffcu));
    } else if ((!_IsDescData(descriptor) && !_IsDescCodeReadable(descriptor)) ||
        ((_IsDescData(descriptor) || _IsDescCodeNonConform(descriptor)) &&
            (_GetSelector_RPL(selector) > _GetDesc_DPL(descriptor) ||
             cpl > _GetDesc_DPL(descriptor)))) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_TS(selector & 0xfffcu));
    }
    if (!_IsDescPresent(descriptor)) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(selector & 0xfffcu));
    }
    _s_task_cache_descriptor(out_cache, selector, descriptor, sregtype);
    CPU_TRACE_CALL_END;
}

static void _s_task_validate_code_selector(
    x86_core_cpu_execution_context *context, t_cpu_data_sreg *ldtr,
    lib_u16 selector, lib_u32 eip, lib_u8 cpl,
    t_cpu_data_sreg *out_cache)
{
    lib_u64 descriptor;

    CPU_TRACE_CALL_BEGIN("_s_task_validate_code_selector");
    if (_GetEFLAGS_VM) {
        lib_memory_set(out_cache, 0, sizeof(*out_cache));
        out_cache->sregtype = SREG_CODE;
        _ksa_load_real_sreg(context, out_cache, selector, LIB_TRUE);
        CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, out_cache, eip, 1u,
            LIB_FALSE, 3u, LIB_TRUE));
        CPU_TRACE_CALL_END;
        return;
    }
    if (_IsSelectorNull(selector) || _GetSelector_RPL(selector) != cpl) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_TS(selector & 0xfffcu));
    }
    CPU_TRACE_CHECK_RETURN(_s_task_read_selector(context, ldtr, selector,
        X86_CPU_REFERENCE_OF(descriptor)));
    if (!_IsDescCode(descriptor) ||
        (_IsDescCodeNonConform(descriptor) && _GetDesc_DPL(descriptor) != cpl) ||
        (_IsDescCodeConform(descriptor) && _GetDesc_DPL(descriptor) > cpl)) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_TS(selector & 0xfffcu));
    }
    if (!_IsDescPresent(descriptor)) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(selector & 0xfffcu));
    }
    _s_task_cache_descriptor(out_cache, selector, descriptor, SREG_CODE);
    CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, out_cache, eip, 1u,
        LIB_FALSE, cpl, LIB_TRUE));
    CPU_TRACE_CALL_END;
}

typedef struct task_switch_sreg_32 {
    lib_u16 selector;
    lib_u16 reserved;
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
    lib_u32 cr3;
    lib_u32 eip;
    lib_u32 eflags;
    lib_u32 eax;
    lib_u32 ecx;
    lib_u32 edx;
    lib_u32 ebx;
    lib_u32 esp;
    lib_u32 ebp;
    lib_u32 esi;
    lib_u32 edi;
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

static void _e_except_n(x86_core_cpu_execution_context *context,
    lib_u8 exid, lib_u8 byte);

static void _s_task_write_state_32(x86_core_cpu_execution_context *context,
    const task_switch_state_32 *state)
{
    CPU_TRACE_CALL_BEGIN("_s_task_write_state_32");
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_CR3_OFFSET,
        X86_CPU_REFERENCE_OF(state->cr3), 4u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_EIP_OFFSET,
        X86_CPU_REFERENCE_OF(state->eip), 4u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_EFLAGS_OFFSET,
        X86_CPU_REFERENCE_OF(state->eflags), 4u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_EAX_OFFSET,
        X86_CPU_REFERENCE_OF(state->eax), 4u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_ECX_OFFSET,
        X86_CPU_REFERENCE_OF(state->ecx), 4u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_EDX_OFFSET,
        X86_CPU_REFERENCE_OF(state->edx), 4u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_EBX_OFFSET,
        X86_CPU_REFERENCE_OF(state->ebx), 4u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_ESP_OFFSET,
        X86_CPU_REFERENCE_OF(state->esp), 4u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_EBP_OFFSET,
        X86_CPU_REFERENCE_OF(state->ebp), 4u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_ESI_OFFSET,
        X86_CPU_REFERENCE_OF(state->esi), 4u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_EDI_OFFSET,
        X86_CPU_REFERENCE_OF(state->edi), 4u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_ES_OFFSET,
        X86_CPU_REFERENCE_OF(state->es.selector), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_CS_OFFSET,
        X86_CPU_REFERENCE_OF(state->cs.selector), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_SS_OFFSET,
        X86_CPU_REFERENCE_OF(state->ss.selector), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_DS_OFFSET,
        X86_CPU_REFERENCE_OF(state->ds.selector), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_FS_OFFSET,
        X86_CPU_REFERENCE_OF(state->fs.selector), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_GS_OFFSET,
        X86_CPU_REFERENCE_OF(state->gs.selector), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, TASK_SWITCH_TSS32_LDTR_OFFSET,
        X86_CPU_REFERENCE_OF(state->ldtr.selector), 2u));
    CPU_TRACE_CALL_END;
}
static void _s_task_write_state_16(x86_core_cpu_execution_context *context,
    const task_switch_state_16 *state)
{
    CPU_TRACE_CALL_BEGIN("_s_task_write_state_16");
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, 0x0eu,
        X86_CPU_REFERENCE_OF(state->ip), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, 0x10u,
        X86_CPU_REFERENCE_OF(state->flags), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, 0x12u,
        X86_CPU_REFERENCE_OF(state->ax), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, 0x14u,
        X86_CPU_REFERENCE_OF(state->cx), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, 0x16u,
        X86_CPU_REFERENCE_OF(state->dx), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, 0x18u,
        X86_CPU_REFERENCE_OF(state->bx), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, 0x1au,
        X86_CPU_REFERENCE_OF(state->sp), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, 0x1cu,
        X86_CPU_REFERENCE_OF(state->bp), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, 0x1eu,
        X86_CPU_REFERENCE_OF(state->si), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, 0x20u,
        X86_CPU_REFERENCE_OF(state->di), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, 0x22u,
        X86_CPU_REFERENCE_OF(state->es), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, 0x24u,
        X86_CPU_REFERENCE_OF(state->cs), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, 0x26u,
        X86_CPU_REFERENCE_OF(state->ss), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, 0x28u,
        X86_CPU_REFERENCE_OF(state->ds), 2u));
    CPU_TRACE_CHECK_RETURN(_s_write_tss(context, 0x2au,
        X86_CPU_REFERENCE_OF(state->ldtr), 2u));
    CPU_TRACE_CALL_END;
}

/* A task switch may save a 286-format TSS and load a 386-format TSS, or the
 * reverse.  The processor selects each image from its own descriptor; it does
 * not require the outgoing and incoming TSS formats to match.  Keep the two
 * architectural layouts explicit, but stage their common transition before
 * the first descriptor, TSS, or CPU-state publication. */
static void _ser_task_transition_tss_plan(
    x86_core_cpu_execution_context *context, lib_u16 newcs,
    lib_u8 nested, lib_u8 returning, lib_u8 old_is_32,
    lib_u8 new_is_32, lib_u8 check_tss_privilege)
{
    lib_u64 old_descriptor;
    lib_u64 new_descriptor;
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
    lib_u16 backlink;
    lib_u16 debug_trap = 0u;
    lib_u8 incoming_cpl;

    CPU_TRACE_CALL_BEGIN("_ser_task_transition_tss_plan");
    if (!_GetCR0_PE || _GetSelector_TI(newcs))
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    if (!cpu_state.data.tr.flagValid || _GetSelector_TI(cpu_state.data.tr.selector))
        CPU_TRACE_CHECK_RETURN(_SetExcept_TS(0));
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, cpu_state.data.tr.selector,
        X86_CPU_REFERENCE_OF(old_descriptor)));
    if ((old_is_32 && !_IsDescTSS32Busy(old_descriptor)) ||
        (!old_is_32 && !_IsDescTSS16Busy(old_descriptor)) ||
        !_IsDescPresent(old_descriptor) || cpu_state.data.tr.limit <
        (old_is_32 ? 0x67u : 0x2bu))
        CPU_TRACE_CHECK_RETURN(_SetExcept_TS(cpu_state.data.tr.selector & 0xfffcu));
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
        X86_CPU_REFERENCE_OF(new_descriptor)));
    if ((new_is_32 && ((returning && !_IsDescTSS32Busy(new_descriptor)) ||
        (!returning && (!_IsDescTSS32Avl(new_descriptor) ||
            _IsDescTSS32Busy(new_descriptor))))) ||
        (!new_is_32 && ((returning && !_IsDescTSS16Busy(new_descriptor)) ||
        (!returning && (!_IsDescTSS16Avl(new_descriptor) ||
            _IsDescTSS16Busy(new_descriptor))))) ||
        (check_tss_privilege && (_GetDesc_DPL(new_descriptor) < _GetCPL ||
            _GetDesc_DPL(new_descriptor) < _GetSelector_RPL(newcs))))
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    if (!_IsDescPresent(new_descriptor))
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
    _s_task_cache_descriptor(&newtr, newcs, new_descriptor, SREG_TR);
    if (newtr.limit < (new_is_32 ? 0x67u : 0x2bu))
        CPU_TRACE_CHECK_RETURN(_SetExcept_TS(newcs & 0xfffcu));

    CPU_TRACE_CHECK_RETURN(_kma_test_access(context, &cpu_state.data.tr,
        old_is_32 ? TASK_SWITCH_TSS32_CR3_OFFSET : 0x0eu,
        old_is_32 ? TASK_SWITCH_TSS32_STATE_BYTES : sizeof(outgoing16),
        LIB_TRUE, 0u, LIB_TRUE));
    CPU_TRACE_CHECK_RETURN(_kma_test_access(context, &newtr,
        new_is_32 ? TASK_SWITCH_TSS32_CR3_OFFSET : 0x0eu,
        new_is_32 ? TASK_SWITCH_TSS32_INCOMING_BYTES : sizeof(incoming16),
        LIB_FALSE, 0u, LIB_TRUE));
    if (nested)
        CPU_TRACE_CHECK_RETURN(_kma_test_access(context, &newtr,
            TASK_SWITCH_TSS_BACKLINK_OFFSET, 2u, LIB_TRUE, 0u, LIB_TRUE));
    CPU_TRACE_CHECK_RETURN(_ksa_test_write_xdt(context,
        cpu_state.data.tr.selector));
    CPU_TRACE_CHECK_RETURN(_ksa_test_write_xdt(context, newcs));
    if (new_is_32) {
        CPU_TRACE_CHECK_RETURN(_kma_read_logical(context, &newtr,
            TASK_SWITCH_TSS32_CR3_OFFSET, X86_CPU_REFERENCE_OF(incoming32),
            sizeof(incoming32), 0u, LIB_TRUE));
        CPU_TRACE_CHECK_RETURN(_kma_read_logical(context, &newtr,
            TASK_SWITCH_TSS32_DEBUG_TRAP_OFFSET, X86_CPU_REFERENCE_OF(debug_trap),
            2u, 0u, LIB_TRUE));
        incoming_cpl = (lib_u8)_GetSelector_RPL(incoming32.cs.selector);
    } else {
        CPU_TRACE_CHECK_RETURN(_kma_read_logical(context, &newtr, 0x0eu,
            X86_CPU_REFERENCE_OF(incoming16), sizeof(incoming16), 0u, LIB_TRUE));
        incoming_cpl = (lib_u8)_GetSelector_RPL(incoming16.cs);
    }

    if (old_is_32) {
        lib_memory_set(&outgoing32, 0, sizeof(outgoing32));
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
        CPU_TRACE_CHECK_RETURN(_s_task_write_state_32(context, &outgoing32));
    } else
        CPU_TRACE_CHECK_RETURN(_s_task_write_state_16(context, &outgoing16));
    if (nested)
        CPU_TRACE_CHECK_RETURN(_kma_write_logical(context, &newtr,
            TASK_SWITCH_TSS_BACKLINK_OFFSET, X86_CPU_REFERENCE_OF(backlink), 2u,
            0u, LIB_TRUE));
    CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, cpu_state.data.tr.selector,
        X86_CPU_REFERENCE_OF(old_descriptor)));
    CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, newtr.selector,
        X86_CPU_REFERENCE_OF(new_descriptor)));
    if (new_is_32) {
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
            CPU_TRACE_CHECK_RETURN(_s_write_cr3(context, incoming32.cr3));
        else
            cpu_state.data.cr3 = incoming32.cr3;
        cpu_state.data.eip = incoming32.eip;
        cpu_state.data.eflags = _e_eflags_load(context, incoming32.eflags);
        _WriteGPR32(eax) = incoming32.eax;
        _WriteGPR32(ecx) = incoming32.ecx;
        _WriteGPR32(edx) = incoming32.edx;
        _WriteGPR32(ebx) = incoming32.ebx;
        _WriteGPR32(esp) = incoming32.esp;
        _WriteGPR32(ebp) = incoming32.ebp;
        _WriteGPR32(esi) = incoming32.esi;
        _WriteGPR32(edi) = incoming32.edi;
        _s_task_set_unavailable(&cpu_state.data.es, incoming32.es.selector,
            SREG_DATA);
        _s_task_set_unavailable(&cpu_state.data.cs, incoming32.cs.selector,
            SREG_CODE);
        _s_task_set_unavailable(&cpu_state.data.ss, incoming32.ss.selector,
            SREG_STACK);
        _s_task_set_unavailable(&cpu_state.data.ds, incoming32.ds.selector,
            SREG_DATA);
        _s_task_set_unavailable(&cpu_state.data.fs, incoming32.fs.selector,
            SREG_DATA);
        _s_task_set_unavailable(&cpu_state.data.gs, incoming32.gs.selector,
            SREG_DATA);
    } else {
        const lib_u32 high = cpu_profile_has_32bit_architecture(context->cpu_profile) ? 0xffff0000u : 0u;

        _WriteGPR32(eax) = high | incoming16.ax;
        _WriteGPR32(ecx) = high | incoming16.cx;
        _WriteGPR32(edx) = high | incoming16.dx;
        _WriteGPR32(ebx) = high | incoming16.bx;
        _WriteGPR32(esp) = high | incoming16.sp;
        _WriteGPR32(ebp) = high | incoming16.bp;
        _WriteGPR32(esi) = high | incoming16.si;
        _WriteGPR32(edi) = high | incoming16.di;
        cpu_state.data.eip = incoming16.ip;
        cpu_state.data.eflags = _e_eflags_load(context, incoming16.flags);
        _s_task_set_unavailable(&cpu_state.data.es, incoming16.es, SREG_DATA);
        _s_task_set_unavailable(&cpu_state.data.cs, incoming16.cs, SREG_CODE);
        _s_task_set_unavailable(&cpu_state.data.ss, incoming16.ss, SREG_STACK);
        _s_task_set_unavailable(&cpu_state.data.ds, incoming16.ds, SREG_DATA);
        _s_task_set_unavailable(&cpu_state.data.fs, 0u, SREG_DATA);
        _s_task_set_unavailable(&cpu_state.data.gs, 0u, SREG_DATA);
    }
    _s_task_set_unavailable(&cpu_state.data.ldtr,
        new_is_32 ? incoming32.ldtr.selector : incoming16.ldtr, SREG_LDTR);
    newtr.sys.type = new_is_32 ? VCPU_DESC_SYS_TYPE_TSS_32_BUSY :
        VCPU_DESC_SYS_TYPE_TSS_16_BUSY;
    cpu_state.data.tr = newtr;
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
        cpu_state.data.dr7 &= ~VCPU_DR7_LOCAL_ENABLE_MASK;
    _SetCR0_TS;
    if (new_is_32 && X86_CPU_GET_LSB(debug_trap)) cpu_state.data.dr6 |= VCPU_DR6_BT;
    if (nested) _SetEFLAGS_NT;
    else if (!returning) _ClrEFLAGS_NT;
    _s_task_checkpoint(context);
    CPU_TRACE_CHECK_RETURN(_s_task_prepare_ldtr(context,
        new_is_32 ? incoming32.ldtr.selector : incoming16.ldtr, &newldtr));
    cpu_state.data.ldtr = newldtr;
    _s_task_checkpoint(context);
    CPU_TRACE_CHECK_RETURN(_s_task_validate_code_selector(context, &newldtr,
        new_is_32 ? incoming32.cs.selector : incoming16.cs,
        new_is_32 ? incoming32.eip : incoming16.ip, incoming_cpl,
        &newcs_cache));
    cpu_state.data.cs = newcs_cache;
    _s_task_checkpoint(context);
    CPU_TRACE_CHECK_RETURN(_s_task_validate_data_selector(context, &newldtr,
        new_is_32 ? incoming32.ss.selector : incoming16.ss, SREG_STACK,
        incoming_cpl, &newss_cache));
    cpu_state.data.ss = newss_cache;
    _s_task_checkpoint(context);
    CPU_TRACE_CHECK_RETURN(_s_task_validate_data_selector(context, &newldtr,
        new_is_32 ? incoming32.ds.selector : incoming16.ds, SREG_DATA,
        incoming_cpl, &newds_cache));
    cpu_state.data.ds = newds_cache;
    _s_task_checkpoint(context);
    CPU_TRACE_CHECK_RETURN(_s_task_validate_data_selector(context, &newldtr,
        new_is_32 ? incoming32.es.selector : incoming16.es, SREG_DATA,
        incoming_cpl, &newes_cache));
    cpu_state.data.es = newes_cache;
    _s_task_checkpoint(context);
    if (new_is_32) {
        CPU_TRACE_CHECK_RETURN(_s_task_validate_data_selector(context, &newldtr,
            incoming32.fs.selector, SREG_DATA, incoming_cpl, &newfs_cache));
        cpu_state.data.fs = newfs_cache;
        _s_task_checkpoint(context);
        CPU_TRACE_CHECK_RETURN(_s_task_validate_data_selector(context, &newldtr,
            incoming32.gs.selector, SREG_DATA, incoming_cpl, &newgs_cache));
        cpu_state.data.gs = newgs_cache;
        _s_task_checkpoint(context);
    }
    CPU_TRACE_CHECK_RETURN(_m_test_logical(context, &cpu_state.data.ss,
        new_is_32 ? incoming32.esp : incoming16.sp, 0u, LIB_FALSE));
    _s_task_checkpoint(context);
    if (new_is_32 && X86_CPU_GET_LSB(debug_trap)) {
        t_cpu trap_cpu = cpu_state;

        CPU_TRACE_CHECK_RETURN(_e_except_n(context, 0x01u, _GetOperandSize));
        if (context->diagnostic_provider != LIB_NULL &&
            context->diagnostic_provider->record_delivered_exception != LIB_NULL) {
            t_cpuins diagnostic_instructions = instruction_state;

            /* The task-switch #DB is a trap at the incoming task's EIP.  It is
             * delivered outside ExecFinal(), so preserve that point for the
             * fault-origin diagnostic contract without changing the current
             * instruction's saved oldcpu. */
            diagnostic_instructions.data.oldcpu = trap_cpu;
            diagnostic_instructions.data.except = VCPUINS_EXCEPT_DB;
            diagnostic_instructions.data.excode = 0u;
            x86_core_cpu_diagnostic_publish_snapshot(
                context->diagnostic_provider->record_delivered_exception,
                context->diagnostic_context, &trap_cpu, &diagnostic_instructions);
        }
    }
    CPU_TRACE_CALL_END;
}

static void _ser_task_transition_tss(x86_core_cpu_execution_context *context,
    lib_u16 newcs, lib_u8 nested, lib_u8 returning,
    lib_u8 check_tss_privilege)
{
    lib_u64 cross_descriptor = 0u;
    lib_u8 old_is_32;
    lib_u8 new_is_32;

    CPU_TRACE_CALL_BEGIN("_ser_task_transition_tss");
    old_is_32 = cpu_profile_has_32bit_architecture(context->cpu_profile) &&
        cpu_state.data.tr.flagValid && cpu_state.data.tr.sys.type ==
        VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
    if (cpu_profile_has_32bit_architecture(context->cpu_profile) &&
        !_GetSelector_TI(newcs)) {
        const lib_u32 misses = instruction_state.data.timing_missed_descriptors;

        CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
            X86_CPU_REFERENCE_OF(cross_descriptor)));
        if (!context->preview_mode && instruction_state.data.timing_missed_descriptors != misses)
            instruction_state.data.timing_task_gate_miss_mask |= 2u;
        new_is_32 = _IsDescTSS32Avl(cross_descriptor) ||
            _IsDescTSS32Busy(cross_descriptor);
    } else new_is_32 = LIB_FALSE;
    CPU_TRACE_CHECK_RETURN(_ser_task_transition_tss_plan(context, newcs,
        nested, returning, old_is_32, new_is_32, check_tss_privilege));
    CPU_TRACE_CALL_END;
}

static void _ser_task_switch_tss(x86_core_cpu_execution_context *context,
    lib_u16 newcs, lib_u8 nested, lib_u8 check_tss_privilege)
{
    CPU_TRACE_CALL_BEGIN("_ser_task_switch_tss");
    CPU_TRACE_CHECK_RETURN(_ser_task_transition_tss(context, newcs, nested,
        LIB_FALSE, check_tss_privilege));
    CPU_TRACE_CALL_END;
}

static void _ser_task_return_tss(x86_core_cpu_execution_context *context)
{
    lib_u16 backlink;

    CPU_TRACE_CALL_BEGIN("_ser_task_return_tss");
    if (!_IsProtected || !cpu_state.data.tr.flagValid ||
        _GetSelector_TI(cpu_state.data.tr.selector) ||
        (cpu_state.data.tr.sys.type != VCPU_DESC_SYS_TYPE_TSS_16_BUSY &&
            cpu_state.data.tr.sys.type != VCPU_DESC_SYS_TYPE_TSS_32_BUSY) ||
        cpu_state.data.tr.limit < 1u)
        CPU_TRACE_CHECK_RETURN(_SetExcept_TS(0));
    CPU_TRACE_CHECK_RETURN(_s_read_tss(context, TASK_SWITCH_TSS_BACKLINK_OFFSET,
        X86_CPU_REFERENCE_OF(backlink), 2u));
    if (_IsSelectorNull(backlink) || _GetSelector_TI(backlink))
        CPU_TRACE_CHECK_RETURN(_SetExcept_TS(backlink & 0xfffcu));
    CPU_TRACE_CHECK_RETURN(_ser_task_transition_tss(context, backlink,
        LIB_FALSE, LIB_TRUE, LIB_FALSE));
    CPU_TRACE_CALL_END;
}
/* regular execute control */
static void _e_push(x86_core_cpu_execution_context *context, lib_uptr rdata, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_e_push");
    if (cpu_profile_has_protected_architecture(context->cpu_profile) &&
        !_IsProtected && (_GetStackSize == 2 ? cpu_state.data.sp :
            cpu_state.data.esp) == 1u)
    {
        lib_u8 index = 0u;
        lib_u8 opcode;
        while (index < instruction_state.data.oplen &&
            _kdf_check_prefix(context, instruction_state.data.opcodes[index])) ++index;
        opcode = index < instruction_state.data.oplen ?
            instruction_state.data.opcodes[index] : 0u;
        if (opcode == 0x9cu || (cpu_profile_has_32bit_architecture(context->cpu_profile) &&
            ((opcode >= 0x50u && opcode <= 0x57u) || opcode == 0x68u ||
            opcode == 0x6au || opcode == 0x06u || opcode == 0x0eu ||
            opcode == 0x16u || opcode == 0x1eu || opcode == 0xffu ||
            opcode == 0x0fu)))
            CPU_TRACE_CHECK_RETURN(x86_core_cpu_execution_raise_exception(
                context, VCPUINS_EXCEPT_SHUTDOWN, 0u));
    }
    CPU_TRACE_CHECK_RETURN(_kec_push(context, rdata, byte));
    CPU_TRACE_CALL_END;
}
static void _e_call_far(x86_core_cpu_execution_context *context, lib_u16 newcs, lib_u32 neweip, lib_u8 byte)
{
    lib_u64 descriptor;
    CPU_TRACE_CALL_BEGIN("_e_call_far");
    if (!_IsProtected)
    {
        CPU_TRACE_BLOCK_BEGIN("!Protected");
        CPU_TRACE_CHECK_RETURN(_ser_call_far_real(context, newcs, neweip, byte));
        CPU_TRACE_BLOCK_END;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("Protected");
        if (_IsSelectorNull(newcs))
        {
            CPU_TRACE_BLOCK_BEGIN("newcs(null)");
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, X86_CPU_REFERENCE_OF(descriptor)));
        instruction_state.data.timing_transfer_type = _IsDescSys(descriptor) ?
            (lib_u8)_GetDesc_Type(descriptor) : 0u;
        if (_IsDescCodeConform(descriptor))
            CPU_TRACE_CHECK_RETURN(_ser_call_far_cs_conf(context, newcs, neweip, byte));
        else if (_IsDescCodeNonConform(descriptor))
            CPU_TRACE_CHECK_RETURN(_ser_call_far_cs_nonc(context, newcs, neweip, byte));
        else if (_IsDescCallGate(descriptor))
            CPU_TRACE_CHECK_RETURN(_ser_call_far_call_gate(context, newcs,
                context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX ?
                    (_IsDescCallGate32(descriptor) ? 4u : 2u) : byte));
        else if (_IsDescTaskGate(descriptor))
            CPU_TRACE_CHECK_RETURN(_ser_call_far_task_gate(context, newcs));
        else if (_IsDescTSS(descriptor))
            CPU_TRACE_CHECK_RETURN(_ser_call_far_tss(context, newcs));
        else
        {
            CPU_TRACE_BLOCK_BEGIN("desc(invalid)");
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
static void _e_call_near(x86_core_cpu_execution_context *context, lib_u32 neweip, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_e_call_near");
    CPU_TRACE_CHECK_RETURN(_kec_call_near(context, neweip, byte));
    CPU_TRACE_CALL_END;
}
_______todo _e_int3(x86_core_cpu_execution_context *context, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_e_int3");
    if (!_GetCR0_PE)
    {
        CPU_TRACE_BLOCK_BEGIN("Real");
        CPU_TRACE_CHECK_RETURN(_ser_int_real(context, 0x03, byte));
        CPU_TRACE_BLOCK_END;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("!Real");
        CPU_TRACE_CHECK_RETURN(_ser_int_protected(context, 0x03u, byte,
            LIB_TRUE, LIB_FALSE, LIB_NULL));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
_______todo _e_into(x86_core_cpu_execution_context *context, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_e_into");
    if (_GetEFLAGS_OF)
    {
        CPU_TRACE_BLOCK_BEGIN("EFLAGS_OF(1)");
        if (!_GetCR0_PE)
        {
            CPU_TRACE_BLOCK_BEGIN("Real");
            CPU_TRACE_CHECK_RETURN(_ser_int_real(context, 0x04, byte));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("!Real");
            CPU_TRACE_CHECK_RETURN(_ser_int_protected(context, 0x04u, byte,
                LIB_TRUE, LIB_FALSE, LIB_NULL));
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
static void _ser_int_vm86(x86_core_cpu_execution_context *context,
    lib_u8 intid, lib_u8 byte)
{
    lib_u16 iomap_base, oldflags;
    lib_u32 bitmap_offset, vector;
    lib_u8 bitmap;
    t_cpu_data_sreg newcs = cpu_state.data.cs;

    CPU_TRACE_CALL_BEGIN("_ser_int_vm86");
    if (!cpu_state.data.tr.flagValid ||
        cpu_state.data.tr.sys.type != VCPU_DESC_SYS_TYPE_TSS_32_BUSY ||
        cpu_state.data.tr.limit < 0x67u)
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
    CPU_TRACE_CHECK_RETURN(_s_read_tss(context, 0x66u,
        X86_CPU_REFERENCE_OF(iomap_base), 2u));
    bitmap_offset = (lib_u32)iomap_base - 32u + (intid >> 3u);
    if (bitmap_offset > cpu_state.data.tr.limit)
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
    CPU_TRACE_CHECK_RETURN(_s_read_tss(context, bitmap_offset,
        X86_CPU_REFERENCE_OF(bitmap), 1u));
    if (bitmap & (1u << (intid & 7u))) {
        if (_GetEFLAGS_IOPL < 3u)
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        CPU_TRACE_CHECK_RETURN(_ser_int_protected(context, intid, byte,
            LIB_TRUE, LIB_FALSE, LIB_NULL));
        CPU_TRACE_CALL_END;
        return;
    }
    CPU_TRACE_CHECK_RETURN(_s_test_ss_frame(context, 2u, 3u, 0u, LIB_TRUE));
    oldflags = _e_real_flags_image_16(context, cpu_state.data.flags);
    if (_GetEFLAGS_IOPL < 3u)
        oldflags = (oldflags & ~(VCPU_EFLAGS_IF | VCPU_EFLAGS_IOPL)) |
            VCPU_EFLAGS_IOPL | (_GetEFLAGS_VIF ? VCPU_EFLAGS_IF : 0u);
    CPU_TRACE_CHECK_RETURN(_kec_push(context,
        X86_CPU_REFERENCE_OF(oldflags), 2u));
    CPU_TRACE_CHECK_RETURN(_kec_push(context,
        X86_CPU_REFERENCE_OF(cpu_state.data.cs.selector), 2u));
    CPU_TRACE_CHECK_RETURN(_kec_push(context,
        X86_CPU_REFERENCE_OF(cpu_state.data.ip), 2u));
    CPU_TRACE_CHECK_RETURN(_kma_read_linear(context, (lib_u32)intid * 4u,
        X86_CPU_REFERENCE_OF(vector), 4u, 3u, 1u));
    CPU_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &newcs,
        X86_CPU_MASK_U16(vector >> 16u)));
    if (_GetEFLAGS_IOPL == 3u) _ClrEFLAGS_IF;
    else X86_CPU_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_VIF);
    _ClrEFLAGS_TF;
    cpu_state.data.cs = newcs;
    cpu_state.data.eip = X86_CPU_MASK_U16(vector);
    CPU_TRACE_CALL_END;
}
_______todo _e_int_n(x86_core_cpu_execution_context *context, lib_u8 intid, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_e_int_n");
    if (!_GetCR0_PE)
    {
        CPU_TRACE_BLOCK_BEGIN("Real");
        CPU_TRACE_CHECK_RETURN(_ser_int_real(context, intid, byte));
        CPU_TRACE_BLOCK_END;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("!Real");
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
            _GetEFLAGS_VM && (cpu_state.data.cr4 & VCPU_CR4_VME))
            CPU_TRACE_CHECK_RETURN(_ser_int_vm86(context, intid, byte));
        else if (_GetEFLAGS_VM && _GetEFLAGS_IOPL < 3)
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        else
            CPU_TRACE_CHECK_RETURN(_ser_int_protected(context, intid, byte,
                LIB_TRUE, LIB_FALSE, LIB_NULL));
        CPU_TRACE_BLOCK_END;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
        instruction_state.data.timing_gpr_writes = 0u;
    CPU_TRACE_CALL_END;
}
static lib_bool _e_timing_charge(x86_core_cpu_execution_context *context, lib_u64 ticks)
{
    if (ticks > LIB_UINT64_MAX - context->timing_result.event_ticks) {
        _SetExcept_CE(LIB_STATUS_LIMIT_EXCEEDED);
        return LIB_FALSE;
    }
    context->timing_result.event_ticks += ticks;
    return LIB_TRUE;
}

/* Failed work has no successful INT/TS latency. Its accepted transfer prefix
 * and actual service/waits are an explicit L2 event, never a retirement. */
static void _e_timing_failed_prefix(x86_core_cpu_execution_context *context)
{
    const lib_u64 accepted = instruction_state.data.timing_transfer_ticks;
    const lib_u64 allocated = instruction_state.data.timing_entry_transfer_ticks;
    lib_u64 ticks;

    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX) return;
    if (allocated > accepted ||
        !x86_core_cpu_80486_bus_scope_cost(context, accepted - allocated, &ticks)) {
        _SetExcept_CE(LIB_STATUS_LIMIT_EXCEEDED);
        return;
    }
    instruction_state.data.timing_entry_transfer_ticks = accepted;
    if (_e_timing_charge(context, ticks)) x86_core_cpu_bus_clear_observation(context);
}

typedef struct cpu_entry_access {
    lib_u32 data_misses;
    lib_u32 missed_reads;
    lib_u32 descriptor_misses;
    lib_u32 missed_descriptors;
    lib_u32 idt_misses;
    lib_u32 pair_misses;
    lib_u32 tss_field_misses;
    lib_u32 missed_tss_fields;
    lib_u32 locked_reads;
    lib_u32 table_misses;
    lib_u32 unaligned_accesses;
    lib_u32 tlb_updates;
    lib_u32 tlb_walks[3];
    lib_u8 unaccessed_descriptors, task_gate_miss_mask;
    lib_u64 bus_wait_ticks, bus_stall_ticks, bus_service_ticks, bus_invalidation_ticks;
} cpu_entry_access;

/* Recursive entries own only their access/service observations; the
 * interrupted form retains its counters. No cache/TLB/FIFO state is swapped. */
static cpu_entry_access _e_begin_access(x86_core_cpu_execution_context *context)
{
    cpu_entry_access access = {0};

    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX) return access;
    access.data_misses = instruction_state.data.timing_data_misses;
    instruction_state.data.timing_data_misses = 0u;
    access.missed_reads = instruction_state.data.timing_missed_reads;
    instruction_state.data.timing_missed_reads = 0u;
    access.descriptor_misses = instruction_state.data.timing_descriptor_misses;
    instruction_state.data.timing_descriptor_misses = 0u;
    access.missed_descriptors = instruction_state.data.timing_missed_descriptors;
    instruction_state.data.timing_missed_descriptors = 0u;
    access.idt_misses = instruction_state.data.timing_idt_misses;
    instruction_state.data.timing_idt_misses = 0u;
    access.pair_misses = instruction_state.data.timing_pair_misses;
    instruction_state.data.timing_pair_misses = 0u;
    access.tss_field_misses = instruction_state.data.timing_tss_field_misses;
    instruction_state.data.timing_tss_field_misses = 0u;
    access.missed_tss_fields = instruction_state.data.timing_missed_tss_fields;
    instruction_state.data.timing_missed_tss_fields = 0u;
    access.locked_reads = instruction_state.data.timing_locked_reads;
    instruction_state.data.timing_locked_reads = 0u;
    access.table_misses = instruction_state.data.timing_table_misses;
    instruction_state.data.timing_table_misses = 0u;
    access.unaligned_accesses = instruction_state.data.timing_unaligned_accesses;
    instruction_state.data.timing_unaligned_accesses = 0u;
    access.tlb_updates = instruction_state.data.timing_tlb_updates;
    instruction_state.data.timing_tlb_updates = 0u;
    access.unaccessed_descriptors = instruction_state.data.timing_unaccessed_descriptors;
    instruction_state.data.timing_unaccessed_descriptors = 0u;
    access.task_gate_miss_mask = instruction_state.data.timing_task_gate_miss_mask;
    instruction_state.data.timing_task_gate_miss_mask = 0u;
    for (lib_u8 index = 0u; index < 3u; ++index) {
        access.tlb_walks[index] = instruction_state.data.timing_tlb_walks[index];
        instruction_state.data.timing_tlb_walks[index] = 0u;
    }
    access.bus_wait_ticks = instruction_state.data.timing_bus_wait_ticks;
    access.bus_stall_ticks = instruction_state.data.timing_bus_stall_ticks;
    access.bus_service_ticks = instruction_state.data.timing_bus_service_ticks;
    access.bus_invalidation_ticks = instruction_state.data.timing_bus_invalidation_ticks;
    x86_core_cpu_bus_clear_observation(context);
    return access;
}

static void _e_end_access(x86_core_cpu_execution_context *context,
    const cpu_entry_access *access)
{
    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX) return;
    instruction_state.data.timing_data_misses = access->data_misses;
    instruction_state.data.timing_missed_reads = access->missed_reads;
    instruction_state.data.timing_descriptor_misses = access->descriptor_misses;
    instruction_state.data.timing_missed_descriptors = access->missed_descriptors;
    instruction_state.data.timing_idt_misses = access->idt_misses;
    instruction_state.data.timing_pair_misses = access->pair_misses;
    instruction_state.data.timing_tss_field_misses = access->tss_field_misses;
    instruction_state.data.timing_missed_tss_fields = access->missed_tss_fields;
    instruction_state.data.timing_locked_reads = access->locked_reads;
    instruction_state.data.timing_table_misses = access->table_misses;
    instruction_state.data.timing_unaligned_accesses = access->unaligned_accesses;
    instruction_state.data.timing_tlb_updates = access->tlb_updates;
    instruction_state.data.timing_unaccessed_descriptors = access->unaccessed_descriptors;
    instruction_state.data.timing_task_gate_miss_mask = access->task_gate_miss_mask;
    for (lib_u8 index = 0u; index < 3u; ++index)
        instruction_state.data.timing_tlb_walks[index] = access->tlb_walks[index];
    instruction_state.data.timing_bus_wait_ticks = access->bus_wait_ticks;
    instruction_state.data.timing_bus_stall_ticks = access->bus_stall_ticks;
    instruction_state.data.timing_bus_service_ticks = access->bus_service_ticks;
    instruction_state.data.timing_bus_invalidation_ticks = access->bus_invalidation_ticks;
}

static void _e_timing_entry(x86_core_cpu_execution_context *context,
    lib_u64 before, lib_u64 allocated_before, lib_u64 setup, lib_u64 base)
{
    const lib_u64 accepted = instruction_state.data.timing_transfer_ticks;
    const lib_u64 allocated = instruction_state.data.timing_entry_transfer_ticks;
    lib_u64 own;

    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX) return;
    if (context->bus_timing.invalid || allocated_before > before ||
        accepted < before || allocated < allocated_before ||
        allocated - allocated_before > accepted - before) {
        _SetExcept_CE(LIB_STATUS_LIMIT_EXCEEDED);
        return;
    }
    own = accepted - before - (allocated - allocated_before);
    instruction_state.data.timing_entry_transfer_ticks += own;
    if (instruction_state.data.except == 0u) {
        if (base == 0u) _SetExcept_CE(LIB_STATUS_INTERNAL_ERROR);
        else own = base;
    }
    if (own > LIB_UINT64_MAX - setup ||
        !x86_core_cpu_80486_bus_scope_cost(context, setup + own, &own)) {
        _SetExcept_CE(LIB_STATUS_LIMIT_EXCEEDED);
        return;
    }
    (void)_e_timing_charge(context, own);
}

static void _e_external_entry_error(x86_core_cpu_execution_context *context)
{
    if (instruction_state.data.except == VCPUINS_EXCEPT_GP ||
        instruction_state.data.except == VCPUINS_EXCEPT_NP ||
        instruction_state.data.except == VCPUINS_EXCEPT_SS ||
        instruction_state.data.except == VCPUINS_EXCEPT_TS)
        instruction_state.data.excode |= 1u;
}

_______todo _e_intr_n(x86_core_cpu_execution_context *context,
    lib_u8 intid, lib_u8 byte, lib_bool nmi)
{
    const lib_u32 entry_cr0 = cpu_state.data.cr0;
    const lib_u32 entry_flags = cpu_state.data.eflags;
    const lib_u8 entry_cpl = _GetCPL;
    const lib_u64 before = instruction_state.data.timing_transfer_ticks;
    const lib_u64 allocated_before = instruction_state.data.timing_entry_transfer_ticks;
    lib_bool task_entry = LIB_FALSE;
    const cpu_entry_access access = _e_begin_access(context);

    CPU_TRACE_CALL_BEGIN("_e_intr_n");
    context->repeat_compare_flags_valid = LIB_FALSE;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
        context->source_repeat_active = LIB_FALSE;
    if (!_GetCR0_PE)
    {
        CPU_TRACE_BLOCK_BEGIN("Real");
        _ser_int_real(context, intid, byte);
        CPU_TRACE_BLOCK_END;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("!Real");
        _ser_int_protected(context, intid, byte, LIB_FALSE, LIB_FALSE, &task_entry);
        _e_external_entry_error(context);
        CPU_TRACE_BLOCK_END;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) {
        if (!instruction_state.data.except) instruction_state.data.timing_gpr_writes = 0u;
        _e_timing_entry(context, before, allocated_before, nmi ? 3u : 11u,
            x86_core_cpu_80486_interrupt_entry_cost(context,
                entry_cr0, entry_flags, entry_cpl, task_entry));
    }
    _e_end_access(context, &access);
    CPU_TRACE_CALL_END;
}
static lib_u8 _e_exception_has_error_code(lib_u8 exid)
{
    return exid == 0x08u || exid == 0x0au || exid == 0x0bu ||
        exid == 0x0cu || exid == 0x0du || exid == 0x0eu || exid == 0x11u;
}

_______todo _e_except_n(x86_core_cpu_execution_context *context, lib_u8 exid, lib_u8 byte)
{
    lib_u32 oldflags = cpu_state.data.eflags;
    const lib_u32 entry_cr0 = cpu_state.data.cr0;
    const lib_u8 entry_cpl = _GetCPL;
    const lib_u64 before = instruction_state.data.timing_transfer_ticks;
    const lib_u64 allocated_before = instruction_state.data.timing_entry_transfer_ticks;
    const lib_u64 setup_ticks = context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX ?
        x86_core_cpu_80486_exception_setup_cost(context, exid) : 0u;
    lib_bool task_entry = LIB_FALSE;

    CPU_TRACE_CALL_BEGIN("_e_except_n");
    const cpu_entry_access access = _e_begin_access(context);
    context->repeat_compare_flags_valid = LIB_FALSE;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
        context->source_repeat_active = LIB_FALSE;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX && exid == 1u)
        cpu_state.data.dr7 &= ~VCPU_DR7_GD;
    /* Fault images set RF; the rollback checkpoint and trap images do not.
     * Gate entry clears live RF, while task entry loads the incoming TSS. */
    if (cpu_profile_has_32bit_architecture(context->cpu_profile) &&
        X86_CPU_BIT_IS_SET(instruction_state.data.except, UINT32_C(1) << exid) &&
        exid != 0x08u && exid != 0x09u) _SetEFLAGS_RF;
    instruction_state.data.except &= ~(UINT32_C(1) << exid);
    if (!_GetCR0_PE)
    {
        CPU_TRACE_BLOCK_BEGIN("Real");
        _ser_int_real(context, exid, byte);
        CPU_TRACE_BLOCK_END;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("!Real");
        _ser_int_protected(context, exid, byte,
            LIB_FALSE, _e_exception_has_error_code(exid), &task_entry);
        _e_external_entry_error(context);
        CPU_TRACE_BLOCK_END;
    }
    if (instruction_state.data.except) cpu_state.data.eflags = oldflags;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) {
        if (!instruction_state.data.except) instruction_state.data.timing_gpr_writes = 0u;
        _e_timing_entry(context, before, allocated_before, setup_ticks,
            x86_core_cpu_80486_interrupt_entry_cost(context,
                entry_cr0, oldflags, entry_cpl, task_entry));
    }
    _e_end_access(context, &access);
    CPU_TRACE_CALL_END;
}
static void _ser_iret_protected_outer(x86_core_cpu_execution_context *context,
    lib_u8 byte)
{
    lib_u16 newcs;
    lib_u16 newss;
    lib_u32 neweip;
    lib_u32 newesp;
    lib_u32 newflags;
    lib_u32 selector;
    lib_u64 code_desc;
    lib_u64 ss_desc;
    lib_u8 oldcpl;
    lib_u8 newcpl;
    lib_u32 flags_mask;
    t_cpu_data_sreg newcs_cache;
    t_cpu_data_sreg newss_cache;

    CPU_TRACE_CALL_BEGIN("_ser_iret_protected_outer");
    oldcpl = _GetCPL;
    /* Ordinary protected IRET never restores VM; old CPL controls IOPL/IF. */
    flags_mask = x86_core_cpu_eflags_reserved_mask(context->cpu_profile) | VCPU_EFLAGS_VM;
    if (oldcpl != 0u) {
        flags_mask |= VCPU_EFLAGS_IOPL;
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
            flags_mask |= VCPU_EFLAGS_VIF | VCPU_EFLAGS_VIP;
        if (oldcpl > _GetEFLAGS_IOPL) flags_mask |= VCPU_EFLAGS_IF;
    }
    switch (byte)
    {
    case 2:
        CPU_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 10u));
        flags_mask |= 0xffff0000u;
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 0u,
            X86_CPU_REFERENCE_OF(neweip), 2u));
        selector = 0u;
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 2u,
            X86_CPU_REFERENCE_OF(selector), 2u));
        newcs = X86_CPU_MASK_U16(selector);
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 4u,
            X86_CPU_REFERENCE_OF(newflags), 2u));
        neweip = X86_CPU_MASK_U16(neweip);
        newflags = X86_CPU_MASK_U16(newflags);
        break;
    case 4:
        CPU_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 20u));
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 0u,
            X86_CPU_REFERENCE_OF(neweip), 4u));
        selector = 0u;
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 4u,
            X86_CPU_REFERENCE_OF(selector), 4u));
        newcs = X86_CPU_MASK_U16(selector);
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 8u,
            X86_CPU_REFERENCE_OF(newflags), 4u));
        break;
    default:
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        break;
    }
    if (_IsSelectorNull(newcs) || _GetSelector_TI(newcs)) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    }
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
        X86_CPU_REFERENCE_OF(code_desc)));
    newcpl = (lib_u8)_GetSelector_RPL(newcs);
    if (!_IsDescCode(code_desc) ||
        (_IsDescCodeNonConform(code_desc) &&
            _GetDesc_DPL(code_desc) != newcpl) ||
        (_IsDescCodeConform(code_desc) &&
            _GetDesc_DPL(code_desc) > newcpl)) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    }
    if (!_IsDescPresent(code_desc)) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
    }
    if (newcpl <= oldcpl) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    }
    switch (byte)
    {
    case 2:
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 6u,
            X86_CPU_REFERENCE_OF(newesp), 2u));
        selector = 0u;
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 8u,
            X86_CPU_REFERENCE_OF(selector), 2u));
        newesp = X86_CPU_MASK_U16(newesp);
        break;
    case 4:
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 12u,
            X86_CPU_REFERENCE_OF(newesp), 4u));
        selector = 0u;
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 16u,
            X86_CPU_REFERENCE_OF(selector), 4u));
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    newss = X86_CPU_MASK_U16(selector);
    if (_IsSelectorNull(newss) || _GetSelector_TI(newss) ||
        _GetSelector_RPL(newss) != newcpl) {
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newss & 0xfffcu));
    }
    newcs_cache = cpu_state.data.cs;
    CPU_TRACE_CHECK_RETURN(_ksa_prepare_code_sreg(context, newcs, newcpl,
        &newcs_cache, code_desc));
    CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, &newcs_cache, neweip,
        1u, 0, newcpl, 1));
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newss,
        X86_CPU_REFERENCE_OF(ss_desc)));
    newss_cache = cpu_state.data.ss;
    CPU_TRACE_CHECK_RETURN(_ksa_prepare_stack_sreg(context, newss, newcpl,
        &newss_cache, ss_desc));
    CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
        X86_CPU_REFERENCE_OF(code_desc)));
    CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, newss,
        X86_CPU_REFERENCE_OF(ss_desc)));
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.ss = newss_cache;
    if (newss_cache.seg.data.big)
        _WriteGPR32(esp) = newesp;
    else
        _WriteGPR16(sp) = X86_CPU_MASK_U16(newesp);
    cpu_state.data.eip = neweip;
    cpu_state.data.eflags = _e_eflags_load(context,
        (cpu_state.data.eflags & flags_mask) | (newflags & ~flags_mask));
    _ser_return_outer_cleanup_sregs(context, newcpl);
    CPU_TRACE_CALL_END;
}
static void _ser_iret_protected_same(x86_core_cpu_execution_context *context,
    lib_u8 byte)
{
    lib_u16 newcs;
    lib_u32 neweip;
    lib_u32 neweflags = 0u;
    lib_u32 selector;
    lib_u32 mask = x86_core_cpu_eflags_reserved_mask(context->cpu_profile);
    lib_u64 code_desc;
    lib_u8 cpl;
    t_cpu_data_sreg newcs_cache;

    CPU_TRACE_CALL_BEGIN("_ser_iret_protected_same");
    cpl = _GetCPL;
    switch (byte)
    {
    case 2:
        CPU_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 6u));
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 0u,
            X86_CPU_REFERENCE_OF(neweip), 2u));
        selector = 0u;
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 2u,
            X86_CPU_REFERENCE_OF(selector), 2u));
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 4u,
            X86_CPU_REFERENCE_OF(neweflags), 2u));
        neweip = X86_CPU_MASK_U16(neweip);
        mask |= 0xffff0000u;
        break;
    case 4:
        CPU_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 12u));
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 0u,
            X86_CPU_REFERENCE_OF(neweip), 4u));
        selector = 0u;
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 4u,
            X86_CPU_REFERENCE_OF(selector), 4u));
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 8u,
            X86_CPU_REFERENCE_OF(neweflags), 4u));
        break;
    default:
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        break;
    }
    newcs = X86_CPU_MASK_U16(selector);
    if (_IsSelectorNull(newcs))
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
    CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs,
        X86_CPU_REFERENCE_OF(code_desc)));
    if (!_IsDescCode(code_desc))
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    if (_GetSelector_RPL(newcs) != cpl)
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    if ((_IsDescCodeConform(code_desc) && _GetDesc_DPL(code_desc) > cpl) ||
        (_IsDescCodeNonConform(code_desc) && _GetDesc_DPL(code_desc) != cpl))
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
    if (!_IsDescPresent(code_desc))
        CPU_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
    newcs_cache = cpu_state.data.cs;
    CPU_TRACE_CHECK_RETURN(_ksa_prepare_code_sreg(context, newcs, cpl,
        &newcs_cache, code_desc));
    CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, &newcs_cache, neweip,
        1u, 0, cpl, 1));
    if (cpl) {
        mask |= VCPU_EFLAGS_IOPL | VCPU_EFLAGS_VM;
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
            mask |= VCPU_EFLAGS_VIF | VCPU_EFLAGS_VIP;
        if (cpl > _GetEFLAGS_IOPL) mask |= VCPU_EFLAGS_IF;
    }
    CPU_TRACE_CHECK_RETURN(_s_write_xdt(context, newcs,
        X86_CPU_REFERENCE_OF(code_desc)));
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.eip = neweip;
    cpu_state.data.eflags = _e_eflags_load(context,
        (neweflags & ~mask) | (cpu_state.data.eflags & mask));
    switch (_GetStackSize)
    {
    case 2:
        _WriteGPR16(sp) += byte * 3u;
        break;
    case 4:
        _WriteGPR32(esp) += byte * 3u;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}
/* Build the architectural real-mode-style cache used after a CPL0 32-bit
 * IRET return to virtual-8086 mode.  The return-frame selectors are not
 * protected-mode selectors: they cannot require a descriptor-table lookup
 * and therefore this helper is deliberately infallible. */
static void _ser_iret_vm86_sreg(t_cpu_data_sreg *rsreg,
    lib_u16 selector, lib_u8 sregtype)
{
    rsreg->flagValid = LIB_TRUE;
    rsreg->selector = selector;
    rsreg->base = (lib_u32)selector << 4u;
    rsreg->limit = 0x0000ffffu;
    rsreg->dpl = 3u;
    rsreg->sregtype = sregtype;
    rsreg->seg.accessed = LIB_FALSE;
    rsreg->seg.executable = sregtype == SREG_CODE;
    if (sregtype == SREG_CODE) {
        rsreg->seg.exec.defsize = LIB_FALSE;
        rsreg->seg.exec.conform = LIB_FALSE;
        rsreg->seg.exec.readable = LIB_TRUE;
    } else {
        rsreg->seg.data.big = LIB_FALSE;
        rsreg->seg.data.expdown = LIB_FALSE;
        rsreg->seg.data.writable = LIB_TRUE;
    }
}
static void _ser_iret_protected_to_vm86(
    x86_core_cpu_execution_context *context)
{
    lib_u32 neweip;
    lib_u32 newflags;
    lib_u32 newesp;
    lib_u32 selector;
    lib_u16 newcs, newss, newes, newds, newfs, newgs;
    t_cpu_data_sreg newcs_cache = cpu_state.data.cs;
    t_cpu_data_sreg newss_cache = cpu_state.data.ss;
    t_cpu_data_sreg newes_cache = cpu_state.data.es;
    t_cpu_data_sreg newds_cache = cpu_state.data.ds;
    t_cpu_data_sreg newfs_cache = cpu_state.data.fs;
    t_cpu_data_sreg newgs_cache = cpu_state.data.gs;

    CPU_TRACE_CALL_BEGIN("_ser_iret_protected_to_vm86");
    /* This is the full 80386 32-bit VM86 return frame.  Every read precedes
     * publication so a failed stack read leaves the CPL0 frame intact. */
    CPU_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 36u));
    CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 0u,
        X86_CPU_REFERENCE_OF(neweip), 4u));
    CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 4u,
        X86_CPU_REFERENCE_OF(selector), 4u));
    newcs = X86_CPU_MASK_U16(selector);
    CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 8u,
        X86_CPU_REFERENCE_OF(newflags), 4u));
    CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 12u,
        X86_CPU_REFERENCE_OF(newesp), 4u));
    CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 16u,
        X86_CPU_REFERENCE_OF(selector), 4u));
    newss = X86_CPU_MASK_U16(selector);
    CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 20u,
        X86_CPU_REFERENCE_OF(selector), 4u));
    newes = X86_CPU_MASK_U16(selector);
    CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 24u,
        X86_CPU_REFERENCE_OF(selector), 4u));
    newds = X86_CPU_MASK_U16(selector);
    CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 28u,
        X86_CPU_REFERENCE_OF(selector), 4u));
    newfs = X86_CPU_MASK_U16(selector);
    CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 32u,
        X86_CPU_REFERENCE_OF(selector), 4u));
    newgs = X86_CPU_MASK_U16(selector);
    _ser_iret_vm86_sreg(&newcs_cache, newcs, SREG_CODE);
    _ser_iret_vm86_sreg(&newss_cache, newss, SREG_STACK);
    _ser_iret_vm86_sreg(&newes_cache, newes, SREG_DATA);
    _ser_iret_vm86_sreg(&newds_cache, newds, SREG_DATA);
    _ser_iret_vm86_sreg(&newfs_cache, newfs, SREG_DATA);
    _ser_iret_vm86_sreg(&newgs_cache, newgs, SREG_DATA);
    cpu_state.data.eflags = _e_eflags_load(context,
        (newflags & ~x86_core_cpu_eflags_reserved_mask(context->cpu_profile)) |
        (cpu_state.data.eflags & x86_core_cpu_eflags_reserved_mask(context->cpu_profile)));
    cpu_state.data.cs = newcs_cache;
    cpu_state.data.ss = newss_cache;
    cpu_state.data.es = newes_cache;
    cpu_state.data.ds = newds_cache;
    cpu_state.data.fs = newfs_cache;
    cpu_state.data.gs = newgs_cache;
    cpu_state.data.eip = neweip;
    _WriteGPR32(esp) = newesp;
    _MakeCPL(3u);
    CPU_TRACE_CALL_END;
}
_______todo _e_iret(x86_core_cpu_execution_context *context, lib_u8 byte)
{
    lib_u16 newcs, newss, newds, newes, newfs, newgs;
    lib_u16 return_cs;
    lib_u32 neweip = 0u, newesp, neweflags = 0u;
    lib_u32 xs_sel;
    lib_u32 mask = x86_core_cpu_eflags_reserved_mask(context->cpu_profile);
    t_cpu_data_sreg ccs = cpu_state.data.cs;
    CPU_TRACE_CALL_BEGIN("_e_iret");
    if (cpu_profile_has_protected_architecture(context->cpu_profile))
        context->nmi_in_service = LIB_FALSE;
    if (!_GetCR0_PE)
    {
        CPU_TRACE_BLOCK_BEGIN("Real");
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
            mask |= VCPU_EFLAGS_VM | VCPU_EFLAGS_VIF | VCPU_EFLAGS_VIP;
        switch (byte)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("byte(2)");
            if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286)
                CPU_TRACE_CHECK_RETURN(_s_test_ss_iret_pop_286(context));
            else CPU_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 6));
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(neweip), 2));
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(xs_sel), 2));
            newcs = X86_CPU_MASK_U16(xs_sel);
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(neweflags), 2));
            mask |= 0xffff0000;
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("byte(4)");
            CPU_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 12));
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(neweip), 4));
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(xs_sel), 4));
            newcs = X86_CPU_MASK_U16(xs_sel);
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(neweflags), 4));
            /* cpu_state.data.eflags = (neweflags & 0x00257fd5) | (cpu_state.data.eflags & 0x001a0000); */
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_BLOCK_BEGIN("byte");
            CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
            CPU_TRACE_BLOCK_END;
            break;
        }
        CPU_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
        CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
        cpu_state.data.cs = ccs;
        cpu_state.data.eip = neweip;
        cpu_state.data.eflags = _e_eflags_load(context,
            (neweflags & ~mask) | (cpu_state.data.eflags & mask));
        if (!cpu_profile_has_186_integer(context->cpu_profile))
            context->interrupt_shadow = CPU_INTERRUPT_SHADOW_INTR;
        CPU_TRACE_BLOCK_END;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("!Real");
        if (!_GetEFLAGS_VM && !_GetEFLAGS_NT) {
            lib_u32 return_flags = 0u;

            switch (byte) {
            case 2:
                CPU_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 6u));
                CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 4u,
                    X86_CPU_REFERENCE_OF(return_flags), 2u));
                break;
            case 4:
                CPU_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 12u));
                CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 8u,
                    X86_CPU_REFERENCE_OF(return_flags), 4u));
                if (X86_CPU_BIT_IS_SET(return_flags, VCPU_EFLAGS_VM) && !_GetCPL) {
                    CPU_TRACE_CHECK_RETURN(_ser_iret_protected_to_vm86(context));
                    CPU_TRACE_CALL_END;
                    return;
                }
                break;
            default:
                CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
                break;
            }
            if (!X86_CPU_BIT_IS_SET(return_flags, VCPU_EFLAGS_VM)) {
                CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, byte,
                    X86_CPU_REFERENCE_OF(return_cs), 2u));
                if (_GetSelector_RPL(return_cs) <= _GetCPL) {
                    CPU_TRACE_CHECK_RETURN(_ser_iret_protected_same(context,
                        byte));
                    CPU_TRACE_CALL_END;
                    return;
                }
                CPU_TRACE_CHECK_RETURN(_ser_iret_protected_outer(context, byte));
                CPU_TRACE_CALL_END;
                return;
            }
        }
        if (_GetEFLAGS_VM)
        {
            CPU_TRACE_BLOCK_BEGIN("V86");
            /* RETURN-FROM-VIRTUAL-8086-MODE */
            if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
                mask |= VCPU_EFLAGS_VIF | VCPU_EFLAGS_VIP;
            if (_GetEFLAGS_IOPL == 3)
            {
                CPU_TRACE_BLOCK_BEGIN("EFLAGS_IOPL(3)");
                switch (byte)
                {
                case 2:
                    CPU_TRACE_BLOCK_BEGIN("byte(2)");
                    CPU_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 6));
                    CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(neweip), 2));
                    CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(xs_sel), 2));
                    newcs = X86_CPU_MASK_U16(xs_sel);
                    CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(neweflags), 2));
                    mask |= (0xffff0000 | VCPU_EFLAGS_IOPL);
                    CPU_TRACE_BLOCK_END;
                    break;
                case 4:
                    CPU_TRACE_BLOCK_BEGIN("byte(4)");
                    CPU_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 12));
                    CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(neweip), 4));
                    CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(xs_sel), 4));
                    newcs = X86_CPU_MASK_U16(xs_sel);
                    CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(neweflags), 4));
                    mask |= (VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL);
                    CPU_TRACE_BLOCK_END;
                    break;
                default:
                    CPU_TRACE_BLOCK_BEGIN("byte");
                    CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
                    CPU_TRACE_BLOCK_END;
                    break;
                }
                CPU_TRACE_CHECK_RETURN(_ksa_load_sreg(context, &ccs, newcs));
                CPU_TRACE_CHECK_RETURN(_kma_test_logical(context, &ccs, neweip, 0x01, 0, 0x00, 1));
                cpu_state.data.cs = ccs;
                cpu_state.data.eip = neweip;
                cpu_state.data.eflags = _e_eflags_load(context,
                    (neweflags & ~mask) | (cpu_state.data.eflags & mask));
                CPU_TRACE_BLOCK_END;
            }
            else
            {
                CPU_TRACE_BLOCK_BEGIN("EFLAGS_IOPL(!3)");
                /* trap to virtual-8086 monitor */
                if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX)
                    instruction_state.data.oldcpu = cpu_state;
                CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_BLOCK_END;
        }
        else if (_GetEFLAGS_NT)
        {
            CPU_TRACE_BLOCK_BEGIN("Nested");
            CPU_TRACE_CHECK_RETURN(_ser_task_return_tss(context));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("Protected,!Nested");
            switch (byte)
            {
            case 2:
                CPU_TRACE_BLOCK_BEGIN("byte(2)");
                CPU_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 6));
                CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(neweip), 2));
                CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(xs_sel), 2));
                newcs = X86_CPU_MASK_U16(xs_sel);
                CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(neweflags), 2));
                CPU_TRACE_BLOCK_END;
                break;
            case 4:
                CPU_TRACE_BLOCK_BEGIN("byte(4)");
                CPU_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 12));
                CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(neweip), 4));
                CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(xs_sel), 4));
                newcs = X86_CPU_MASK_U16(xs_sel);
                CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(neweflags), 4));
                CPU_TRACE_BLOCK_END;
                break;
            default:
                CPU_TRACE_BLOCK_BEGIN("byte");
                CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
                CPU_TRACE_BLOCK_END;
                break;
            }
            /* CPL0 32-bit VM86 return is handled before this legacy pop path
             * can publish its first frame field.  This retained path has no
             * other supported protected return contract. */
            CPU_TRACE_BLOCK_BEGIN("unsupported protected return");
            CPU_TRACE_CHECK_RETURN(_SetExcept_CE(0));
            CPU_TRACE_BLOCK_END;
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
static void _e_jcc(x86_core_cpu_execution_context *context, lib_u32 csrc, lib_u8 byte, lib_u8 condition)
{
    lib_u32 neweip = cpu_state.data.eip;
    CPU_TRACE_CALL_BEGIN("_e_jcc");
    if (condition)
    {
        CPU_TRACE_BLOCK_BEGIN("condition(1)");
        switch (byte)
        {
        case 1:
            neweip += (lib_i8)csrc;
            break;
        case 2:
            neweip += (lib_i16)csrc;
            break;
        case 4:
            neweip += (lib_i32)csrc;
            break;
        default:
            CPU_TRACE_BLOCK_BEGIN("byte");
            CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
            CPU_TRACE_BLOCK_END;
            break;
        }
        CPU_TRACE_CHECK_RETURN(_kec_jmp_near(context, neweip, _GetOperandSize));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
static void _e_jmp_far(x86_core_cpu_execution_context *context, lib_u16 newcs, lib_u32 neweip, lib_u8 byte)
{
    lib_u64 descriptor;
    CPU_TRACE_CALL_BEGIN("_e_jmp_far");
    if (!_IsProtected)
    {
        CPU_TRACE_BLOCK_BEGIN("Protected(0)");
        CPU_TRACE_CHECK_RETURN(_ser_jmp_far_real(context, newcs, neweip, byte));
        CPU_TRACE_BLOCK_END;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("Protected(1)");
        if (_IsSelectorNull(newcs))
        {
            CPU_TRACE_BLOCK_BEGIN("selector(null)");
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, X86_CPU_REFERENCE_OF(descriptor)));
        instruction_state.data.timing_transfer_type = _IsDescSys(descriptor) ?
            (lib_u8)_GetDesc_Type(descriptor) : 0u;
        if (_IsDescCodeConform(descriptor))
            CPU_TRACE_CHECK_RETURN(_ser_jmp_far_cs_conf(context, newcs, neweip, byte));
        else if (_IsDescCodeNonConform(descriptor))
            CPU_TRACE_CHECK_RETURN(_ser_jmp_far_cs_nonc(context, newcs, neweip, byte));
        else if (_IsDescCallGate(descriptor))
            CPU_TRACE_CHECK_RETURN(_ser_jmp_far_call_gate(context, newcs));
        else if (_IsDescTaskGate(descriptor))
            CPU_TRACE_CHECK_RETURN(_ser_jmp_far_task_gate(context, newcs));
        else if (_IsDescTSS(descriptor))
            CPU_TRACE_CHECK_RETURN(_ser_task_switch_tss(context, newcs,
                LIB_FALSE, LIB_TRUE));
        else
        {
            CPU_TRACE_BLOCK_BEGIN("newcs(invalid)");
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
    }
    instruction_state.data.opr1 = cpu_state.data.cs.selector;
    instruction_state.data.opr2 = cpu_state.data.eip;
    CPU_TRACE_CALL_END;
}
static void _e_jmp_near(x86_core_cpu_execution_context *context, lib_u32 neweip, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_e_jmp_near");
    CPU_TRACE_CHECK_RETURN(_kec_jmp_near(context, neweip, byte));
    CPU_TRACE_CALL_END;
}
static void _e_load_far(x86_core_cpu_execution_context *context, t_cpu_data_sreg *rsreg, lib_uptr rdest, lib_u16 selector, lib_u32 offset, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_e_load_far");
    CPU_TRACE_CHECK_RETURN(_s_load_sreg(context, rsreg, selector));
    switch (byte)
    {
    case 2:
        CPU_TRACE_BLOCK_BEGIN("byte(2)");
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, rdest, X86_CPU_REFERENCE_OF(offset), 2));
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("byte(4)");
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, rdest, X86_CPU_REFERENCE_OF(offset), 4));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("byte");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _e_pop_sreg(x86_core_cpu_execution_context *context,
    t_cpu_data_sreg *rsreg, lib_u8 byte)
{
    lib_u32 selector;
    lib_u32 cesp;

    CPU_TRACE_CALL_BEGIN("_e_pop_sreg");
    switch (_GetStackSize)
    {
    case 2:
        cesp = cpu_state.data.sp;
        break;
    case 4:
        cesp = cpu_state.data.esp;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CHECK_RETURN(_s_read_ss(context, cesp, X86_CPU_REFERENCE_OF(selector),
        byte));
    CPU_TRACE_CHECK_RETURN(_s_load_sreg(context, rsreg,
        X86_CPU_MASK_U16(selector)));
    switch (_GetStackSize)
    {
    case 2:
        _WriteGPR16(sp) += byte;
        break;
    case 4:
        _WriteGPR32(esp) += byte;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    if (rsreg->sregtype == SREG_STACK ||
        !cpu_profile_has_186_integer(context->cpu_profile))
        context->interrupt_shadow = CPU_INTERRUPT_SHADOW_SEGMENT;
    CPU_TRACE_CALL_END;
}
static void _e_loopcc(x86_core_cpu_execution_context *context, lib_i8 csrc, lib_u8 condition)
{
    lib_u32 cecx;
    lib_u32 neweip = cpu_state.data.eip;
    CPU_TRACE_CALL_BEGIN("_e_loopcc");
    switch (_GetAddressSize)
    {
    case 2:
        cecx = X86_CPU_MASK_U16(cpu_state.data.cx - 1u);
        break;
    case 4:
        cecx = X86_CPU_MASK_U32(cpu_state.data.ecx - 1u);
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    if (cecx && condition)
    {
        CPU_TRACE_BLOCK_BEGIN("cecx(!0),condition(1)");
        neweip += csrc;
        CPU_TRACE_CHECK_RETURN(_kec_jmp_near(context, neweip, _GetOperandSize));
        CPU_TRACE_BLOCK_END;
    }
    switch (_GetAddressSize)
    {
    case 2:
        _WriteGPR16(cx) = X86_CPU_MASK_U16(cecx);
        break;
    case 4:
        _WriteGPR32(ecx) = cecx;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _e_ret_near(x86_core_cpu_execution_context *context, lib_u16 parambyte, lib_u8 byte)
{
    CPU_TRACE_CALL_BEGIN("_e_ret_near");
    CPU_TRACE_CHECK_RETURN(_kec_ret_near(context, parambyte, byte));
    CPU_TRACE_CALL_END;
}
static void _e_ret_far(x86_core_cpu_execution_context *context, lib_u16 parambyte, lib_u16 byte)
{
    lib_u16 newcs;
    lib_u32 xs_sel;
    lib_u32 neweip = 0;
    lib_u64 descriptor;
    CPU_TRACE_CALL_BEGIN("_e_ret_far");
    switch (byte)
    {
    case 2:
        CPU_TRACE_BLOCK_BEGIN("byte(2)");
        CPU_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 4));
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 0u, X86_CPU_REFERENCE_OF(neweip), 2));
        xs_sel = 0u;
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 2u, X86_CPU_REFERENCE_OF(xs_sel), 2));
        newcs = X86_CPU_MASK_U16(xs_sel);
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("byte(4)");
        CPU_TRACE_CHECK_RETURN(_s_test_ss_pop(context, 8));
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 0u, X86_CPU_REFERENCE_OF(neweip), 4));
        xs_sel = 0u;
        CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 4u, X86_CPU_REFERENCE_OF(xs_sel), 4));
        newcs = X86_CPU_MASK_U16(xs_sel);
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("byte");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    if (!_IsProtected)
    {
        CPU_TRACE_BLOCK_BEGIN("!Protected");
        CPU_TRACE_CHECK_RETURN(_ser_ret_far_real(context, newcs, neweip,
            X86_CPU_MASK_U16(parambyte + byte * 2u), byte));
        CPU_TRACE_BLOCK_END;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("Protected");
        if (_IsSelectorNull(newcs))
        {
            CPU_TRACE_BLOCK_BEGIN("selector(null)");
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, newcs, X86_CPU_REFERENCE_OF(descriptor)));
        if (!_IsDescCode(descriptor))
        {
            CPU_TRACE_BLOCK_BEGIN("!DescCode");
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
            CPU_TRACE_BLOCK_END;
        }
        if (_GetSelector_RPL(newcs) < _GetCPL)
        {
            CPU_TRACE_BLOCK_BEGIN("RPL(<CPL)");
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
            CPU_TRACE_BLOCK_END;
        }
        if (_IsDescCodeConform(descriptor))
        {
            CPU_TRACE_BLOCK_BEGIN("DescCodeConform");
            if (_GetDesc_DPL(descriptor) > _GetSelector_RPL(newcs))
            {
                CPU_TRACE_BLOCK_BEGIN("DPL(>RPL)");
                CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_BLOCK_END;
        }
        else if (_GetDesc_DPL(descriptor) != _GetSelector_RPL(newcs))
        {
            CPU_TRACE_BLOCK_BEGIN("DescCodeNonConform/DPL(!RPL)");
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(newcs & 0xfffcu));
            CPU_TRACE_BLOCK_END;
        }
        if (!_IsDescPresent(descriptor))
        {
            CPU_TRACE_BLOCK_BEGIN("!DescPresent");
            CPU_TRACE_CHECK_RETURN(_SetExcept_NP(newcs & 0xfffcu));
            CPU_TRACE_BLOCK_END;
        }
        if (_GetSelector_RPL(newcs) > _GetCPL)
        {
            CPU_TRACE_BLOCK_BEGIN("RPL(>CPL)");
            CPU_TRACE_CHECK_RETURN(_ser_ret_far_outer(context, newcs, neweip, parambyte, byte));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("RPL(<=CPL)");
            CPU_TRACE_CHECK_RETURN(_ser_ret_far_same(context, newcs, neweip,
                X86_CPU_MASK_U16(parambyte + byte * 2u), byte));
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
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

static void _kaf_calc_CF(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("_kaf_calc_CF");
    switch (instruction_state.data.type)
    {
    case ADC8:
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, ((_GetEFLAGS_CF && instruction_state.data.opr2 == 0xffu) ? 1 : ((instruction_state.data.result < instruction_state.data.opr1) || (instruction_state.data.result < instruction_state.data.opr2))));
        break;
    case ADC16:
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, ((_GetEFLAGS_CF && instruction_state.data.opr2 == 0xffffu) ? 1 : ((instruction_state.data.result < instruction_state.data.opr1) || (instruction_state.data.result < instruction_state.data.opr2))));
        break;
    case ADC32:
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, ((_GetEFLAGS_CF && instruction_state.data.opr2 == LIB_UINT32_MAX) ? 1 : ((instruction_state.data.result < instruction_state.data.opr1) || (instruction_state.data.result < instruction_state.data.opr2))));
        break;
    case ADD8:
    case ADD16:
    case ADD32:
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, (instruction_state.data.result < instruction_state.data.opr1) || (instruction_state.data.result < instruction_state.data.opr2));
        break;
    case SBB8:
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, (instruction_state.data.opr1 < instruction_state.data.result) || (_GetEFLAGS_CF && (instruction_state.data.opr2 == 0xffu)));
        break;
    case SBB16:
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, (instruction_state.data.opr1 < instruction_state.data.result) || (_GetEFLAGS_CF && (instruction_state.data.opr2 == 0xffffu)));
        break;
    case SBB32:
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, (instruction_state.data.opr1 < instruction_state.data.result) || (_GetEFLAGS_CF && (instruction_state.data.opr2 == LIB_UINT32_MAX)));
        break;
    case SUB8:
    case SUB16:
    case SUB32:
    case CMP8:
    case CMP16:
    case CMP32:
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, instruction_state.data.opr1 < instruction_state.data.opr2);
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("type");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(instruction_state.data.type));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _kaf_calc_OF(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("_kaf_calc_OF");
    switch (instruction_state.data.type)
    {
    case ADC8:
    case ADD8:
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                      (X86_CPU_GET_MSB_8(instruction_state.data.opr1) == X86_CPU_GET_MSB_8(instruction_state.data.opr2)) && (X86_CPU_GET_MSB_8(instruction_state.data.opr1) != X86_CPU_GET_MSB_8(instruction_state.data.result)));
        break;
    case ADC16:
    case ADD16:
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                      (X86_CPU_GET_MSB_16(instruction_state.data.opr1) == X86_CPU_GET_MSB_16(instruction_state.data.opr2)) && (X86_CPU_GET_MSB_16(instruction_state.data.opr1) != X86_CPU_GET_MSB_16(instruction_state.data.result)));
        break;
    case ADC32:
    case ADD32:
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                      (X86_CPU_GET_MSB_32(instruction_state.data.opr1) == X86_CPU_GET_MSB_32(instruction_state.data.opr2)) && (X86_CPU_GET_MSB_32(instruction_state.data.opr1) != X86_CPU_GET_MSB_32(instruction_state.data.result)));
        break;
    case SBB8:
    case SUB8:
    case CMP8:
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                      (X86_CPU_GET_MSB_8(instruction_state.data.opr1) != X86_CPU_GET_MSB_8(instruction_state.data.opr2)) && (X86_CPU_GET_MSB_8(instruction_state.data.opr2) == X86_CPU_GET_MSB_8(instruction_state.data.result)));
        break;
    case SBB16:
    case SUB16:
    case CMP16:
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                      (X86_CPU_GET_MSB_16(instruction_state.data.opr1) != X86_CPU_GET_MSB_16(instruction_state.data.opr2)) && (X86_CPU_GET_MSB_16(instruction_state.data.opr2) == X86_CPU_GET_MSB_16(instruction_state.data.result)));
        break;
    case SBB32:
    case SUB32:
    case CMP32:
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                      (X86_CPU_GET_MSB_32(instruction_state.data.opr1) != X86_CPU_GET_MSB_32(instruction_state.data.opr2)) && (X86_CPU_GET_MSB_32(instruction_state.data.opr2) == X86_CPU_GET_MSB_32(instruction_state.data.result)));
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("type");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(instruction_state.data.type));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _kaf_calc_AF(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("_kaf_calc_AF");
    X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_AF, ((instruction_state.data.opr1 ^ instruction_state.data.opr2) ^ instruction_state.data.result) & 0x10);
    CPU_TRACE_CALL_END;
}
static void _kaf_calc_PF(x86_core_cpu_execution_context *context)
{
    lib_u8 res8 = X86_CPU_MASK_U8(instruction_state.data.result);
    lib_u8 even = 1;
    CPU_TRACE_CALL_BEGIN("_kaf_calc_PF");
    while (res8)
    {
        even = 1 - even;
        res8 &= res8 - 1;
    }
    X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_PF, even);
    CPU_TRACE_CALL_END;
}
static void _kaf_calc_ZF(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("_kaf_calc_ZF");
    X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_ZF, !instruction_state.data.result);
    CPU_TRACE_CALL_END;
}
static void _kaf_calc_SF(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("_kaf_calc_SF");
    switch (instruction_state.data.bit)
    {
    case 8:
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_SF, X86_CPU_GET_MSB_8(instruction_state.data.result));
        break;
    case 16:
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_SF, X86_CPU_GET_MSB_16(instruction_state.data.result));
        break;
    case 32:
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_SF, X86_CPU_GET_MSB_32(instruction_state.data.result));
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(instruction_state.data.bit));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _kaf_set_flags(x86_core_cpu_execution_context *context, lib_u16 flags)
{
    CPU_TRACE_CALL_BEGIN("_kaf_set_flags");
    if (flags & VCPU_EFLAGS_CF)
        CPU_TRACE_CHECK_RETURN(_kaf_calc_CF(context));
    if (flags & VCPU_EFLAGS_PF)
        CPU_TRACE_CHECK_RETURN(_kaf_calc_PF(context));
    if (flags & VCPU_EFLAGS_AF)
        CPU_TRACE_CHECK_RETURN(_kaf_calc_AF(context));
    if (flags & VCPU_EFLAGS_ZF)
        CPU_TRACE_CHECK_RETURN(_kaf_calc_ZF(context));
    if (flags & VCPU_EFLAGS_SF)
        CPU_TRACE_CHECK_RETURN(_kaf_calc_SF(context));
    if (flags & VCPU_EFLAGS_OF)
        CPU_TRACE_CHECK_RETURN(_kaf_calc_OF(context));
    CPU_TRACE_CALL_END;
}
static void _kas_move_index(x86_core_cpu_execution_context *context, lib_u8 byte, lib_u8 flagsi, lib_u8 flagdi)
{
    CPU_TRACE_CALL_BEGIN("_kas_move_index");
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
        instruction_state.data.timing_ea_base_mask |= (flagsi ? 0x40u : 0u) | (flagdi ? 0x80u : 0u);
    switch (_GetAddressSize)
    {
    case 2:
        if (_GetEFLAGS_DF)
        {
            if (flagdi)
                _WriteGPR16(di) -= byte;
            if (flagsi)
                _WriteGPR16(si) -= byte;
        }
        else
        {
            if (flagdi)
                _WriteGPR16(di) += byte;
            if (flagsi)
                _WriteGPR16(si) += byte;
        }
        break;
    case 4:
        if (_GetEFLAGS_DF)
        {
            if (flagdi)
                _WriteGPR32(edi) -= byte;
            if (flagsi)
                _WriteGPR32(esi) -= byte;
        }
        else
        {
            if (flagdi)
                _WriteGPR32(edi) += byte;
            if (flagsi)
                _WriteGPR32(esi) += byte;
        }
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}

#define _kac_arith1(funflag, type8, expr8, type16, expr16, type32, expr32) \
    do                                                                     \
    {                                                                      \
        switch (bit)                                                       \
        {                                                                  \
        case 8:                                                            \
            instruction_state.data.bit = 8;                                \
            instruction_state.data.type = (type8);                         \
            instruction_state.data.opr1 = X86_CPU_MASK_U8(cdest);     \
            instruction_state.data.result = X86_CPU_MASK_U8(expr8);   \
            break;                                                         \
        case 16:                                                           \
            instruction_state.data.bit = 16;                               \
            instruction_state.data.type = (type16);                        \
            instruction_state.data.opr1 = X86_CPU_MASK_U16(cdest);    \
            instruction_state.data.result = X86_CPU_MASK_U16(expr16); \
            break;                                                         \
        case 32:                                                           \
            instruction_state.data.bit = 32;                               \
            instruction_state.data.type = (type32);                        \
            instruction_state.data.opr1 = X86_CPU_MASK_U32(cdest);    \
            instruction_state.data.result = X86_CPU_MASK_U32(expr32); \
            break;                                                         \
        default:                                                           \
            CPU_TRACE_BLOCK_BEGIN("bit");                                 \
            CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));                   \
            CPU_TRACE_BLOCK_END;                                          \
            break;                                                         \
        }                                                                  \
        CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, funflag));         \
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
            instruction_state.data.opr1 = X86_CPU_MASK_U8(cdest);                \
            instruction_state.data.opr2 = X86_CPU_MASK_U8(csrc);                 \
            instruction_state.data.result = X86_CPU_MASK_U8(expr8);              \
            break;                                                                    \
        case 12:                                                                      \
            instruction_state.data.bit = 16;                                          \
            instruction_state.data.type = (type12);                                   \
            instruction_state.data.opr1 = X86_CPU_MASK_U16(cdest);               \
            instruction_state.data.opr2 = X86_CPU_MASK_U16((lib_i8)csrc); \
            instruction_state.data.result = X86_CPU_MASK_U16(expr12);            \
            break;                                                                    \
        case 16:                                                                      \
            instruction_state.data.bit = 16;                                          \
            instruction_state.data.type = (type16);                                   \
            instruction_state.data.opr1 = X86_CPU_MASK_U16(cdest);               \
            instruction_state.data.opr2 = X86_CPU_MASK_U16(csrc);                \
            instruction_state.data.result = X86_CPU_MASK_U16(expr16);            \
            break;                                                                    \
        case 20:                                                                      \
            instruction_state.data.bit = 32;                                          \
            instruction_state.data.type = (type20);                                   \
            instruction_state.data.opr1 = X86_CPU_MASK_U32(cdest);               \
            instruction_state.data.opr2 = X86_CPU_MASK_U32((lib_i8)csrc); \
            instruction_state.data.result = X86_CPU_MASK_U32(expr20);            \
            break;                                                                    \
        case 32:                                                                      \
            instruction_state.data.bit = 32;                                          \
            instruction_state.data.type = (type32);                                   \
            instruction_state.data.opr1 = X86_CPU_MASK_U32(cdest);               \
            instruction_state.data.opr2 = X86_CPU_MASK_U32(csrc);                \
            instruction_state.data.result = X86_CPU_MASK_U32(expr32);            \
            break;                                                                    \
        default:                                                                      \
            CPU_TRACE_BLOCK_BEGIN("bit");                                            \
            CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));                              \
            CPU_TRACE_BLOCK_END;                                                     \
            break;                                                                    \
        }                                                                             \
        CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, funflag));                    \
    } while (0)
static void _a_add(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u64 csrc, lib_u8 bit)
{
    CPU_TRACE_CALL_BEGIN("_a_add");
    _kac_arith2(ADD_FLAG,
                ADD8, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD16, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD16, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD32, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD32, (instruction_state.data.opr1 + instruction_state.data.opr2));
    CPU_TRACE_CALL_END;
}
static void _a_adc(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u64 csrc, lib_u8 bit)
{
    CPU_TRACE_CALL_BEGIN("_a_adc");
    _kac_arith2(ADC_FLAG,
                ADC8, (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF),
                ADC16, (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF),
                ADC16, (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF),
                ADC32, (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF),
                ADC32, (instruction_state.data.opr1 + instruction_state.data.opr2 + _GetEFLAGS_CF));
    CPU_TRACE_CALL_END;
}
static void _a_and(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u64 csrc, lib_u8 bit)
{
    CPU_TRACE_CALL_BEGIN("_a_and");
    _kac_arith2(AND_FLAG,
                AND8, (instruction_state.data.opr1 & instruction_state.data.opr2),
                AND16, (instruction_state.data.opr1 & instruction_state.data.opr2),
                AND16, (instruction_state.data.opr1 & instruction_state.data.opr2),
                AND32, (instruction_state.data.opr1 & instruction_state.data.opr2),
                AND32, (instruction_state.data.opr1 & instruction_state.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    instruction_state.data.udf |= VCPU_EFLAGS_AF;
    CPU_TRACE_CALL_END;
}
static void _a_or(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u64 csrc, lib_u8 bit)
{
    CPU_TRACE_CALL_BEGIN("_a_or");
    _kac_arith2(OR_FLAG,
                OR8, (instruction_state.data.opr1 | instruction_state.data.opr2),
                OR16, (instruction_state.data.opr1 | instruction_state.data.opr2),
                OR16, (instruction_state.data.opr1 | instruction_state.data.opr2),
                OR32, (instruction_state.data.opr1 | instruction_state.data.opr2),
                OR32, (instruction_state.data.opr1 | instruction_state.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    instruction_state.data.udf |= VCPU_EFLAGS_AF;
    CPU_TRACE_CALL_END;
}
static void _a_sbb(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u64 csrc, lib_u8 bit)
{
    CPU_TRACE_CALL_BEGIN("_a_sbb");
    _kac_arith2(SBB_FLAG,
                SBB8, (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)),
                SBB16, (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)),
                SBB16, (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)),
                SBB32, (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)),
                SBB32, (instruction_state.data.opr1 - (instruction_state.data.opr2 + _GetEFLAGS_CF)));
    CPU_TRACE_CALL_END;
}
static void _a_sub(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u64 csrc, lib_u8 bit)
{
    CPU_TRACE_CALL_BEGIN("_a_sub");
    _kac_arith2(SUB_FLAG,
                SUB8, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB16, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB16, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB32, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB32, (instruction_state.data.opr1 - instruction_state.data.opr2));
    CPU_TRACE_CALL_END;
}
static void _a_xor(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u64 csrc, lib_u8 bit)
{
    CPU_TRACE_CALL_BEGIN("_a_xor");
    _kac_arith2(XOR_FLAG,
                XOR8, (instruction_state.data.opr1 ^ instruction_state.data.opr2),
                XOR16, (instruction_state.data.opr1 ^ instruction_state.data.opr2),
                XOR16, (instruction_state.data.opr1 ^ instruction_state.data.opr2),
                XOR32, (instruction_state.data.opr1 ^ instruction_state.data.opr2),
                XOR32, (instruction_state.data.opr1 ^ instruction_state.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    instruction_state.data.udf |= VCPU_EFLAGS_AF;
    CPU_TRACE_CALL_END;
}
static void _a_cmp(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u64 csrc, lib_u8 bit)
{
    CPU_TRACE_CALL_BEGIN("_a_cmp");
    _kac_arith2(CMP_FLAG,
                CMP8, ((lib_u8)instruction_state.data.opr1 - (lib_i8)instruction_state.data.opr2),
                CMP16, ((lib_u16)instruction_state.data.opr1 - (lib_i8)instruction_state.data.opr2),
                CMP16, ((lib_u16)instruction_state.data.opr1 - (lib_i16)instruction_state.data.opr2),
                CMP32, ((lib_u32)instruction_state.data.opr1 - (lib_i8)instruction_state.data.opr2),
                CMP32, ((lib_u32)instruction_state.data.opr1 - (lib_i32)instruction_state.data.opr2));
    CPU_TRACE_CALL_END;
}
static void _a_test(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u64 csrc, lib_u8 bit)
{
    CPU_TRACE_CALL_BEGIN("_a_test");
    _kac_arith2(TEST_FLAG,
                TEST8, (instruction_state.data.opr1 & instruction_state.data.opr2),
                TEST16, (instruction_state.data.opr1 & instruction_state.data.opr2),
                TEST16, (instruction_state.data.opr1 & instruction_state.data.opr2),
                TEST32, (instruction_state.data.opr1 & instruction_state.data.opr2),
                TEST32, (instruction_state.data.opr1 & instruction_state.data.opr2));
    _ClrEFLAGS_OF;
    _ClrEFLAGS_CF;
    instruction_state.data.udf |= VCPU_EFLAGS_AF;
    CPU_TRACE_CALL_END;
}
static void _a_inc(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u8 bit)
{
    CPU_TRACE_CALL_BEGIN("_a_inc");
    instruction_state.data.opr2 = 1;
    _kac_arith1(INC_FLAG,
                ADD8, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD16, (instruction_state.data.opr1 + instruction_state.data.opr2),
                ADD32, (instruction_state.data.opr1 + instruction_state.data.opr2));
    CPU_TRACE_CALL_END;
}
static void _a_dec(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u8 bit)
{
    CPU_TRACE_CALL_BEGIN("_a_dec");
    instruction_state.data.opr2 = 1;
    _kac_arith1(DEC_FLAG,
                SUB8, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB16, (instruction_state.data.opr1 - instruction_state.data.opr2),
                SUB32, (instruction_state.data.opr1 - instruction_state.data.opr2));
    CPU_TRACE_CALL_END;
}
static void _a_not(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u8 bit)
{
    CPU_TRACE_CALL_BEGIN("_a_not");
    instruction_state.data.opr2 = 0;
    _kac_arith1(NOT_FLAG,
                ARITHTYPE_NULL, (~instruction_state.data.opr1),
                ARITHTYPE_NULL, (~instruction_state.data.opr1),
                ARITHTYPE_NULL, (~instruction_state.data.opr1));
    CPU_TRACE_CALL_END;
}
static void _a_neg(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u8 bit)
{
    CPU_TRACE_CALL_BEGIN("_a_neg");
    instruction_state.data.opr2 = cdest;
    cdest = 0;
    _kac_arith1(NEG_FLAG,
                SUB8, (X86_CPU_MASK_U8(instruction_state.data.opr1) - X86_CPU_MASK_U8(instruction_state.data.opr2)),
                SUB16, (X86_CPU_MASK_U16(instruction_state.data.opr1) - X86_CPU_MASK_U16(instruction_state.data.opr2)),
                SUB32, (X86_CPU_MASK_U32(instruction_state.data.opr1) - X86_CPU_MASK_U32(instruction_state.data.opr2)));
    X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!instruction_state.data.opr2);
    CPU_TRACE_CALL_END;
}
static void _a_mul(x86_core_cpu_execution_context *context, lib_u64 csrc, lib_u8 bit)
{
    lib_u64 cdest;
    CPU_TRACE_CALL_BEGIN("_a_mul");
    switch (bit)
    {
    case 8:
        CPU_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = cpu_state.data.al;
        instruction_state.data.opr2 = X86_CPU_MASK_U8(csrc);
        cdest = X86_CPU_MASK_U16(cpu_state.data.al * instruction_state.data.opr2);
        _WriteGPR16(ax) = X86_CPU_MASK_U16(cdest);
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF, !!cpu_state.data.ah);
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!cpu_state.data.ah);
        instruction_state.data.result = X86_CPU_MASK_U16(cdest);
        CPU_TRACE_BLOCK_END;
        break;
    case 16:
        CPU_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = cpu_state.data.ax;
        instruction_state.data.opr2 = X86_CPU_MASK_U16(csrc);
        cdest = X86_CPU_MASK_U32(cpu_state.data.ax * instruction_state.data.opr2);
        _WriteGPR16(dx) = X86_CPU_MASK_U16(cdest >> 16);
        _WriteGPR16(ax) = X86_CPU_MASK_U16(cdest);
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF, !!cpu_state.data.dx);
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!cpu_state.data.dx);
        instruction_state.data.result = X86_CPU_MASK_U32(cdest);
        CPU_TRACE_BLOCK_END;
        break;
    case 32:
        CPU_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = cpu_state.data.eax;
        instruction_state.data.opr2 = X86_CPU_MASK_U32(csrc);
        cdest = X86_CPU_MASK_U64(cpu_state.data.eax * instruction_state.data.opr2);
        _WriteGPR32(edx) = X86_CPU_MASK_U32(cdest >> 32);
        _WriteGPR32(eax) = X86_CPU_MASK_U32(cdest);
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF, !!cpu_state.data.edx);
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!cpu_state.data.edx);
        instruction_state.data.result = X86_CPU_MASK_U64(cdest);
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        CPU_TRACE_BLOCK_END;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                                   VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    CPU_TRACE_CALL_END;
}
static void _a_imul(x86_core_cpu_execution_context *context, lib_u64 csrc, lib_u8 bit)
{
    lib_i64 cdest;
    CPU_TRACE_CALL_BEGIN("_a_imul");
    switch (bit)
    {
    case 8:
        CPU_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = cpu_state.data.al;
        instruction_state.data.opr2 = X86_CPU_MASK_U8((lib_i8)csrc);
        cdest = X86_CPU_MASK_U16((lib_i8)cpu_state.data.al * (lib_i8)instruction_state.data.opr2);
        _WriteGPR16(ax) = X86_CPU_MASK_U16(cdest);
        if ((lib_i16)cdest == (lib_i16)(lib_i8)cpu_state.data.al)
        {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        else
        {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        instruction_state.data.result = X86_CPU_MASK_U16(cdest);
        CPU_TRACE_BLOCK_END;
        break;
    case 16:
        CPU_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = cpu_state.data.ax;
        instruction_state.data.opr2 = X86_CPU_MASK_U16((lib_i16)csrc);
        cdest = X86_CPU_MASK_U32((lib_i16)cpu_state.data.ax * (lib_i16)instruction_state.data.opr2);
        _WriteGPR16(ax) = X86_CPU_MASK_U16(cdest);
        _WriteGPR16(dx) = X86_CPU_MASK_U16(cdest >> 16);
        if ((lib_i32)cdest == (lib_i32)(lib_i16)cpu_state.data.ax)
        {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        else
        {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        instruction_state.data.result = X86_CPU_MASK_U32(cdest);
        CPU_TRACE_BLOCK_END;
        break;
    case 32:
        CPU_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = cpu_state.data.eax;
        instruction_state.data.opr2 = X86_CPU_MASK_U32((lib_i32)csrc);
        cdest = X86_CPU_MASK_U64((lib_i64)(lib_i32)cpu_state.data.eax *
                                      (lib_i64)(lib_i32)instruction_state.data.opr2);
        _WriteGPR32(eax) = X86_CPU_MASK_U32(cdest);
        _WriteGPR32(edx) = X86_CPU_MASK_U32(cdest >> 32);
        if ((lib_i64)cdest == (lib_i64)(lib_i32)cpu_state.data.eax)
        {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        else
        {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        instruction_state.data.result = X86_CPU_MASK_U64(cdest);
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        CPU_TRACE_BLOCK_END;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                                   VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    CPU_TRACE_CALL_END;
}
static void _a_div(x86_core_cpu_execution_context *context, lib_u64 csrc, lib_u8 bit)
{
    lib_u64 temp = 0x0000000000000000;
    CPU_TRACE_CALL_BEGIN("_a_div");
    switch (bit)
    {
    case 8:
        CPU_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = X86_CPU_MASK_U16(cpu_state.data.ax);
        instruction_state.data.opr2 = X86_CPU_MASK_U8(csrc);
        if (!instruction_state.data.opr2)
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(0)");
            CPU_TRACE_CHECK_RETURN(_SetExcept_DE(0));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(!0)");
            temp = X86_CPU_MASK_U16((lib_u16)instruction_state.data.opr1 / (lib_u8)instruction_state.data.opr2);
            if (temp > 0xffu)
            {
                CPU_TRACE_BLOCK_BEGIN("temp(>0xff)");
                CPU_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                CPU_TRACE_BLOCK_END;
            }
            else
            {
                _WriteGPR8(al) = X86_CPU_MASK_U8(temp);
                _WriteGPR8(ah) = X86_CPU_MASK_U8((lib_u16)instruction_state.data.opr1 % (lib_u8)instruction_state.data.opr2);
            }
            CPU_TRACE_BLOCK_END;
        }
        instruction_state.data.result = cpu_state.data.ax;
        CPU_TRACE_BLOCK_END;
        break;
    case 16:
        CPU_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = X86_CPU_MASK_U32(((lib_u32)cpu_state.data.dx << 16) | cpu_state.data.ax);
        instruction_state.data.opr2 = X86_CPU_MASK_U16(csrc);
        if (!instruction_state.data.opr2)
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(0)");
            CPU_TRACE_CHECK_RETURN(_SetExcept_DE(0));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(!0)");
            temp = X86_CPU_MASK_U32((lib_u32)instruction_state.data.opr1 / (lib_u16)instruction_state.data.opr2);
            if (temp > 0xffffu)
            {
                CPU_TRACE_BLOCK_BEGIN("temp(>0xffff)");
                CPU_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                CPU_TRACE_BLOCK_END;
            }
            else
            {
                _WriteGPR16(ax) = X86_CPU_MASK_U16(temp);
                _WriteGPR16(dx) = X86_CPU_MASK_U16((lib_u32)instruction_state.data.opr1 % (lib_u16)instruction_state.data.opr2);
            }
            CPU_TRACE_BLOCK_END;
        }
        instruction_state.data.result = ((lib_u32)cpu_state.data.dx << 16) | cpu_state.data.ax;
        CPU_TRACE_BLOCK_END;
        break;
    case 32:
        CPU_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = X86_CPU_MASK_U64(((lib_u64)cpu_state.data.edx << 32) | cpu_state.data.eax);
        instruction_state.data.opr2 = X86_CPU_MASK_U32(csrc);
        if (!instruction_state.data.opr2)
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(0)");
            CPU_TRACE_CHECK_RETURN(_SetExcept_DE(0));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(!0)");
            temp = X86_CPU_MASK_U64((lib_u64)instruction_state.data.opr1 / (lib_u32)instruction_state.data.opr2);
            if (temp > LIB_UINT32_MAX)
            {
                CPU_TRACE_BLOCK_BEGIN("temp(>0xffffffff)");
                CPU_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                CPU_TRACE_BLOCK_END;
            }
            else
            {
                _WriteGPR32(eax) = X86_CPU_MASK_U32(temp);
                _WriteGPR32(edx) = X86_CPU_MASK_U32((lib_u64)instruction_state.data.opr1 % (lib_u32)instruction_state.data.opr2);
            }
            CPU_TRACE_BLOCK_END;
        }
        instruction_state.data.result = ((lib_u64)cpu_state.data.edx << 32) | cpu_state.data.eax;
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        CPU_TRACE_BLOCK_END;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF |
                                   VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_OF);
    CPU_TRACE_CALL_END;
}
static void _a_idiv(x86_core_cpu_execution_context *context, lib_u64 csrc, lib_u8 bit)
{
    lib_i64 temp;
    lib_i64 dividend;
    lib_i64 divisor;
    CPU_TRACE_CALL_BEGIN("_a_idiv");
    switch (bit)
    {
    case 8:
        CPU_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = X86_CPU_MASK_U16((lib_i16)cpu_state.data.ax);
        instruction_state.data.opr2 = X86_CPU_MASK_U8((lib_i8)csrc);
        if (!instruction_state.data.opr2)
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(0)");
            CPU_TRACE_CHECK_RETURN(_SetExcept_DE(0));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(!0)");
            dividend = (lib_i16)cpu_state.data.ax;
            divisor = (lib_i8)csrc;
            temp = dividend / divisor;
            if (temp > 0x7f || temp < -0x80)
            {
                CPU_TRACE_BLOCK_BEGIN("temp(>0x7f/<0x80)");
                CPU_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                CPU_TRACE_BLOCK_END;
            }
            else
            {
                _WriteGPR8(al) = X86_CPU_MASK_U8(temp);
                _WriteGPR8(ah) = X86_CPU_MASK_U8(dividend % divisor);
            }
            CPU_TRACE_BLOCK_END;
        }
        instruction_state.data.result = cpu_state.data.ax;
        CPU_TRACE_BLOCK_END;
        break;
    case 16:
        CPU_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = X86_CPU_MASK_U32((lib_i32)(((lib_u32)cpu_state.data.dx << 16) | cpu_state.data.ax));
        instruction_state.data.opr2 = X86_CPU_MASK_U16((lib_i16)csrc);
        if (!instruction_state.data.opr2)
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(0)");
            CPU_TRACE_CHECK_RETURN(_SetExcept_DE(0));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(!0)");
            dividend = (lib_i32)(((lib_u32)cpu_state.data.dx << 16) | cpu_state.data.ax);
            divisor = (lib_i16)csrc;
            if (dividend == ((lib_i32)-2147483647 - 1) && divisor == -1)
            {
                CPU_TRACE_BLOCK_BEGIN("temp(overflow)");
                CPU_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                CPU_TRACE_BLOCK_END;
            }
            else if ((temp = dividend / divisor) > 0x7fff || temp < -0x8000)
            {
                CPU_TRACE_BLOCK_BEGIN("temp(>0x7fff/<0x8000)");
                CPU_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                CPU_TRACE_BLOCK_END;
            }
            else
            {
                _WriteGPR16(ax) = X86_CPU_MASK_U16(temp);
                _WriteGPR16(dx) = X86_CPU_MASK_U16(dividend % divisor);
            }
            CPU_TRACE_BLOCK_END;
        }
        instruction_state.data.result = ((lib_u32)cpu_state.data.dx << 16) | cpu_state.data.ax;
        CPU_TRACE_BLOCK_END;
        break;
    case 32:
        CPU_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = X86_CPU_MASK_U64((lib_i64)(((lib_u64)cpu_state.data.edx << 32) | cpu_state.data.eax));
        instruction_state.data.opr2 = X86_CPU_MASK_U32((lib_i32)csrc);
        if (!instruction_state.data.opr2)
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(0)");
            CPU_TRACE_CHECK_RETURN(_SetExcept_DE(0));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(!0)");
            dividend = (lib_i64)(((lib_u64)cpu_state.data.edx << 32) | cpu_state.data.eax);
            divisor = (lib_i32)csrc;
            if (dividend == ((lib_i64)-9223372036854775807LL - 1LL) && divisor == -1)
            {
                CPU_TRACE_BLOCK_BEGIN("temp(overflow)");
                CPU_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                CPU_TRACE_BLOCK_END;
            }
            else if ((temp = dividend / divisor) > 0x7fffffff || temp < -0x80000000LL)
            {
                CPU_TRACE_BLOCK_BEGIN("temp(>0x7fffffff/<0x80000000)");
                CPU_TRACE_CHECK_RETURN(_SetExcept_DE(0));
                CPU_TRACE_BLOCK_END;
            }
            else
            {
                _WriteGPR32(eax) = X86_CPU_MASK_U32(temp);
                _WriteGPR32(edx) = X86_CPU_MASK_U32(dividend % divisor);
            }
            CPU_TRACE_BLOCK_END;
        }
        instruction_state.data.result = ((lib_u64)cpu_state.data.edx << 32) | cpu_state.data.eax;
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        CPU_TRACE_BLOCK_END;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF |
                                   VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_OF);
    CPU_TRACE_CALL_END;
}

static void _a_imul3(x86_core_cpu_execution_context *context, lib_u64 csrc1, lib_u64 csrc2, lib_u8 bit)
{
    lib_i64 cdest;
    CPU_TRACE_CALL_BEGIN("_a_imul3");
    switch (bit)
    {
    case 12:
        CPU_TRACE_BLOCK_BEGIN("bit(16+8)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = (lib_i16)csrc1;
        instruction_state.data.opr2 = (lib_i8)csrc2;
        cdest = X86_CPU_MASK_U32((lib_i16)instruction_state.data.opr1 * (lib_i8)instruction_state.data.opr2);
        instruction_state.data.result = X86_CPU_MASK_U16(cdest);
        if (X86_CPU_MASK_U32(cdest) != X86_CPU_MASK_U32((lib_i16)instruction_state.data.result))
        {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        else
        {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case 16:
        CPU_TRACE_BLOCK_BEGIN("bit(16+16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = (lib_i16)csrc1;
        instruction_state.data.opr2 = (lib_i16)csrc2;
        cdest = X86_CPU_MASK_U32((lib_i16)instruction_state.data.opr1 * (lib_i16)instruction_state.data.opr2);
        instruction_state.data.result = X86_CPU_MASK_U16(cdest);
        if (X86_CPU_MASK_U32(cdest) != X86_CPU_MASK_U32((lib_i16)instruction_state.data.result))
        {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        else
        {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case 20:
        CPU_TRACE_BLOCK_BEGIN("bit(32+8)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = (lib_i32)csrc1;
        instruction_state.data.opr2 = (lib_i8)csrc2;
        cdest = X86_CPU_MASK_U64((lib_i64)(lib_i32)instruction_state.data.opr1 *
            (lib_i64)(lib_i8)instruction_state.data.opr2);
        instruction_state.data.result = X86_CPU_MASK_U32(cdest);
        if (X86_CPU_MASK_U64(cdest) != X86_CPU_MASK_U64((lib_i32)instruction_state.data.result))
        {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        else
        {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case 32:
        CPU_TRACE_BLOCK_BEGIN("bit(32+32");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = (lib_i32)csrc1;
        instruction_state.data.opr2 = (lib_i32)csrc2;
        cdest = X86_CPU_MASK_U64((lib_i64)(lib_i32)instruction_state.data.opr1 *
            (lib_i64)(lib_i32)instruction_state.data.opr2);
        instruction_state.data.result = X86_CPU_MASK_U32(cdest);
        if (X86_CPU_MASK_U64(cdest) != X86_CPU_MASK_U64((lib_i32)instruction_state.data.result))
        {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        else
        {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        CPU_TRACE_BLOCK_END;
        break;
    }
    instruction_state.data.udf |= (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                                   VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    CPU_TRACE_CALL_END;
}

static lib_u8 _a_shift_rotate_count(x86_core_cpu_execution_context *context,
    lib_u8 csrc)
{
    return x86_core_cpu_profile_has_8086_semantics(context->cpu_profile) ? csrc :
        (lib_u8)(csrc & 0x1fu);
}
static void _a_rol(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u8 csrc, lib_u8 bit)
{
    lib_u8 count;
    lib_u32 flagcf;
    CPU_TRACE_CALL_BEGIN("_a_rol");
    switch (bit)
    {
    case 8:
        CPU_TRACE_BLOCK_BEGIN("bit(8)");
        count = _a_shift_rotate_count(context, csrc);
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = X86_CPU_MASK_U8(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (count == 0)
            break;
        while (count)
        {
            flagcf = !!X86_CPU_GET_MSB_8(instruction_state.data.result);
            instruction_state.data.result = X86_CPU_MASK_U8(instruction_state.data.result << 1) | flagcf;
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!X86_CPU_GET_LSB_U8(instruction_state.data.result));
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!X86_CPU_GET_MSB_8(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        CPU_TRACE_BLOCK_END;
        break;
    case 16:
        CPU_TRACE_BLOCK_BEGIN("bit(16)");
        count = _a_shift_rotate_count(context, csrc);
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = X86_CPU_MASK_U16(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (count == 0)
            break;
        while (count)
        {
            flagcf = !!X86_CPU_GET_MSB_16(instruction_state.data.result);
            instruction_state.data.result = X86_CPU_MASK_U16(instruction_state.data.result << 1) | flagcf;
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!X86_CPU_GET_LSB_U16(instruction_state.data.result));
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!X86_CPU_GET_MSB_16(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        CPU_TRACE_BLOCK_END;
        break;
    case 32:
        CPU_TRACE_BLOCK_BEGIN("bit(32)");
        count = _a_shift_rotate_count(context, csrc);
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = X86_CPU_MASK_U32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (count == 0)
            break;
        while (count)
        {
            flagcf = !!X86_CPU_GET_MSB_32(instruction_state.data.result);
            instruction_state.data.result = X86_CPU_MASK_U32(instruction_state.data.result << 1) | flagcf;
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!X86_CPU_GET_LSB_U32(instruction_state.data.result));
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!X86_CPU_GET_MSB_32(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _a_ror(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u8 csrc, lib_u8 bit)
{
    lib_u8 count;
    lib_u32 flagcf;
    CPU_TRACE_CALL_BEGIN("_a_ror");
    switch (bit)
    {
    case 8:
        CPU_TRACE_BLOCK_BEGIN("bit(8)");
        count = _a_shift_rotate_count(context, csrc);
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = X86_CPU_MASK_U8(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (count == 0)
            break;
        while (count)
        {
            flagcf = X86_CPU_GET_LSB_U8(instruction_state.data.result) ? X86_CPU_MSB_8 : 0;
            instruction_state.data.result = X86_CPU_MASK_U8(instruction_state.data.result >> 1) | flagcf;
            count--;
        }
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!X86_CPU_GET_MSB_8(instruction_state.data.result));
        if (instruction_state.data.opr2 == 1)
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!X86_CPU_GET_MSB_8(instruction_state.data.result)) ^ (!!X86_CPU_GET_MSB_7(instruction_state.data.result))));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        CPU_TRACE_BLOCK_END;
        break;
    case 16:
        CPU_TRACE_BLOCK_BEGIN("bit(16)");
        count = _a_shift_rotate_count(context, csrc);
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = X86_CPU_MASK_U16(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (count == 0)
            break;
        while (count)
        {
            flagcf = X86_CPU_GET_LSB_U16(instruction_state.data.result) ? X86_CPU_MSB_16 : 0;
            instruction_state.data.result = X86_CPU_MASK_U16(instruction_state.data.result >> 1) | flagcf;
            count--;
        }
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!X86_CPU_GET_MSB_16(instruction_state.data.result));
        if (instruction_state.data.opr2 == 1)
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!X86_CPU_GET_MSB_16(instruction_state.data.result)) ^ (!!X86_CPU_GET_MSB_15(instruction_state.data.result))));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        CPU_TRACE_BLOCK_END;
        break;
    case 32:
        CPU_TRACE_BLOCK_BEGIN("bit(32)");
        count = _a_shift_rotate_count(context, csrc);
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = X86_CPU_MASK_U32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (count == 0)
            break;
        while (count)
        {
            flagcf = X86_CPU_GET_LSB_U32(instruction_state.data.result) ? X86_CPU_MSB_32 : 0;
            instruction_state.data.result = X86_CPU_MASK_U32(instruction_state.data.result >> 1) | flagcf;
            count--;
        }
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!X86_CPU_GET_MSB_32(instruction_state.data.result));
        if (instruction_state.data.opr2 == 1)
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!X86_CPU_GET_MSB_32(instruction_state.data.result)) ^ (!!X86_CPU_GET_MSB_31(instruction_state.data.result))));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _a_rcl(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u8 csrc, lib_u8 bit)
{
    lib_u8 count;
    lib_u32 flagcf;
    CPU_TRACE_CALL_BEGIN("_a_rcl");
    switch (bit)
    {
    case 8:
        CPU_TRACE_BLOCK_BEGIN("bit(8)");
        count = _a_shift_rotate_count(context, csrc);
        instruction_state.data.opr2 = count;
        if (!x86_core_cpu_profile_has_8086_semantics(context->cpu_profile))
            count %= 9;
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = X86_CPU_MASK_U8(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        if (instruction_state.data.opr2 == 0)
            break;
        while (count)
        {
            flagcf = !!X86_CPU_GET_MSB_8(instruction_state.data.result);
            instruction_state.data.result = X86_CPU_MASK_U8(instruction_state.data.result << 1) | _GetEFLAGS_CF;
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, flagcf);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!X86_CPU_GET_MSB_8(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        CPU_TRACE_BLOCK_END;
        break;
    case 16:
        CPU_TRACE_BLOCK_BEGIN("bit(16)");
        count = _a_shift_rotate_count(context, csrc);
        instruction_state.data.opr2 = count;
        if (!x86_core_cpu_profile_has_8086_semantics(context->cpu_profile))
            count %= 17;
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = X86_CPU_MASK_U16(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        if (instruction_state.data.opr2 == 0)
            break;
        while (count)
        {
            flagcf = !!X86_CPU_GET_MSB_16(instruction_state.data.result);
            instruction_state.data.result = X86_CPU_MASK_U16(instruction_state.data.result << 1) | _GetEFLAGS_CF;
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, flagcf);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!X86_CPU_GET_MSB_16(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        CPU_TRACE_BLOCK_END;
        break;
    case 32:
        CPU_TRACE_BLOCK_BEGIN("bit(32)");
        count = (csrc & 0x1f);
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = X86_CPU_MASK_U32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (instruction_state.data.opr2 == 0)
            break;
        while (count)
        {
            flagcf = !!X86_CPU_GET_MSB_32(instruction_state.data.result);
            instruction_state.data.result = X86_CPU_MASK_U32(instruction_state.data.result << 1) | _GetEFLAGS_CF;
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, flagcf);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!X86_CPU_GET_MSB_32(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _a_rcr(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u8 csrc, lib_u8 bit)
{
    lib_u8 count;
    lib_u32 flagcf;
    CPU_TRACE_CALL_BEGIN("_a_rcr");
    switch (bit)
    {
    case 8:
        CPU_TRACE_BLOCK_BEGIN("bit(8)");
        count = _a_shift_rotate_count(context, csrc);
        instruction_state.data.opr2 = count;
        if (!x86_core_cpu_profile_has_8086_semantics(context->cpu_profile))
            count %= 9;
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = X86_CPU_MASK_U8(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        if (instruction_state.data.opr2 == 0)
            break;
        if (instruction_state.data.opr2 == 1)
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!X86_CPU_GET_MSB_8(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        while (count)
        {
            flagcf = X86_CPU_GET_LSB_U8(instruction_state.data.result);
            instruction_state.data.result = X86_CPU_MASK_U8(instruction_state.data.result >> 1) | (_GetEFLAGS_CF ? X86_CPU_MSB_8 : 0);
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!flagcf);
            count--;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case 16:
        CPU_TRACE_BLOCK_BEGIN("bit(16)");
        count = _a_shift_rotate_count(context, csrc);
        instruction_state.data.opr2 = count;
        if (!x86_core_cpu_profile_has_8086_semantics(context->cpu_profile))
            count %= 17;
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = X86_CPU_MASK_U16(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        if (instruction_state.data.opr2 == 0)
            break;
        if (instruction_state.data.opr2 == 1)
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!X86_CPU_GET_MSB_16(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        while (count)
        {
            flagcf = X86_CPU_GET_LSB_U16(instruction_state.data.result);
            instruction_state.data.result = X86_CPU_MASK_U16(instruction_state.data.result >> 1) | (_GetEFLAGS_CF ? X86_CPU_MSB_16 : 0);
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!flagcf);
            count--;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case 32:
        CPU_TRACE_BLOCK_BEGIN("bit(32)");
        count = (csrc & 0x1f);
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = X86_CPU_MASK_U32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        if (instruction_state.data.opr2 == 0)
            break;
        if (instruction_state.data.opr2 == 1)
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!X86_CPU_GET_MSB_32(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        while (count)
        {
            flagcf = X86_CPU_GET_LSB_U32(instruction_state.data.result);
            instruction_state.data.result = X86_CPU_MASK_U32(instruction_state.data.result >> 1) | (_GetEFLAGS_CF ? X86_CPU_MSB_32 : 0);
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!flagcf);
            count--;
        }
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _a_shl(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u8 csrc, lib_u8 bit)
{
    lib_u8 count;
    CPU_TRACE_CALL_BEGIN("_a_shl");
    count = _a_shift_rotate_count(context, csrc);
    instruction_state.data.opr2 = count;
    if (count >= bit)
        instruction_state.data.udf |= VCPU_EFLAGS_CF;
    switch (bit)
    {
    case 8:
        CPU_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = X86_CPU_MASK_U8(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count)
        {
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!X86_CPU_GET_MSB_8(instruction_state.data.result));
            instruction_state.data.result = X86_CPU_MASK_U8(instruction_state.data.result << 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!X86_CPU_GET_MSB_8(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else if (instruction_state.data.opr2 != 0)
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0)
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(!0)");
            CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHL_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case 16:
        CPU_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = X86_CPU_MASK_U16(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count)
        {
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!X86_CPU_GET_MSB_16(instruction_state.data.result));
            instruction_state.data.result = X86_CPU_MASK_U16(instruction_state.data.result << 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!X86_CPU_GET_MSB_16(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else if (instruction_state.data.opr2 != 0)
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0)
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(!0)");
            CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHL_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case 32:
        CPU_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = X86_CPU_MASK_U32(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count)
        {
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, !!X86_CPU_GET_MSB_32(instruction_state.data.result));
            instruction_state.data.result = X86_CPU_MASK_U32(instruction_state.data.result << 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                          ((!!X86_CPU_GET_MSB_32(instruction_state.data.result)) ^ _GetEFLAGS_CF));
        else if (instruction_state.data.opr2 != 0)
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0)
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(!0)");
            CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHL_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _a_shr(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u8 csrc, lib_u8 bit)
{
    lib_u8 count;
    CPU_TRACE_CALL_BEGIN("_a_shr");
    count = _a_shift_rotate_count(context, csrc);
    instruction_state.data.opr2 = count;
    if (count >= bit)
        instruction_state.data.udf |= VCPU_EFLAGS_CF;
    switch (bit)
    {
    case 8:
        CPU_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = X86_CPU_MASK_U8(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count)
        {
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!X86_CPU_GET_LSB_U8(instruction_state.data.result)));
            instruction_state.data.result = X86_CPU_MASK_U8(instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF, (!!X86_CPU_GET_MSB_8(instruction_state.data.opr1)));
        else if (instruction_state.data.opr2 != 0)
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0)
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(!0)");
            CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case 16:
        CPU_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = X86_CPU_MASK_U16(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count)
        {
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!X86_CPU_GET_LSB_U16(instruction_state.data.result)));
            instruction_state.data.result = X86_CPU_MASK_U16(instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF, (!!X86_CPU_GET_MSB_16(instruction_state.data.opr1)));
        else if (instruction_state.data.opr2 != 0)
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0)
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(!0)");
            CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case 32:
        CPU_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = X86_CPU_MASK_U32(cdest);
        instruction_state.data.result = instruction_state.data.opr1;
        while (count)
        {
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!X86_CPU_GET_LSB_U32(instruction_state.data.result)));
            instruction_state.data.result = X86_CPU_MASK_U32(instruction_state.data.result >> 1);
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF, (!!X86_CPU_GET_MSB_32(instruction_state.data.opr1)));
        else if (instruction_state.data.opr2 != 0)
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0)
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(!0)");
            CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _a_sar(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u8 csrc, lib_u8 bit)
{
    lib_u8 count;
    CPU_TRACE_CALL_BEGIN("_a_sar");
    count = _a_shift_rotate_count(context, csrc);
    instruction_state.data.opr2 = count;
    switch (bit)
    {
    case 8:
        CPU_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.opr1 = X86_CPU_MASK_U8(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count)
        {
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!X86_CPU_GET_LSB_U8(instruction_state.data.result)));
            instruction_state.data.result = X86_CPU_MASK_U8((instruction_state.data.result >> 1) |
                (instruction_state.data.result & X86_CPU_MSB_8));
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            _ClrEFLAGS_OF;
        else if (instruction_state.data.opr2 != 0)
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0)
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(!0)");
            CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, SAR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case 16:
        CPU_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = X86_CPU_MASK_U16(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count)
        {
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!X86_CPU_GET_LSB_U16(instruction_state.data.result)));
            instruction_state.data.result = X86_CPU_MASK_U16((instruction_state.data.result >> 1) |
                (instruction_state.data.result & X86_CPU_MSB_16));
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            _ClrEFLAGS_OF;
        else if (instruction_state.data.opr2 != 0)
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0)
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(!0)");
            CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, SAR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case 32:
        CPU_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = X86_CPU_MASK_U32(cdest);
        instruction_state.data.opr2 = count;
        instruction_state.data.result = instruction_state.data.opr1;
        while (count)
        {
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, (!!X86_CPU_GET_LSB_U32(instruction_state.data.result)));
            instruction_state.data.result = X86_CPU_MASK_U32((instruction_state.data.result >> 1) |
                (instruction_state.data.result & X86_CPU_MSB_32));
            count--;
        }
        if (instruction_state.data.opr2 == 1)
            _ClrEFLAGS_OF;
        else if (instruction_state.data.opr2 != 0)
            instruction_state.data.udf |= VCPU_EFLAGS_OF;
        if (instruction_state.data.opr2 != 0)
        {
            CPU_TRACE_BLOCK_BEGIN("opr2(!0)");
            CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, SAR_FLAG));
            instruction_state.data.udf |= VCPU_EFLAGS_AF;
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}

static void _p_ins(x86_core_cpu_execution_context *context, lib_u8 byte)
{
    lib_u32 cedi, data = 0;
    CPU_TRACE_CALL_BEGIN("_p_ins");
    switch (_GetAddressSize)
    {
    case 2:
        cedi = cpu_state.data.di;
        break;
    case 4:
        cedi = cpu_state.data.edi;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (byte)
    {
    case 1:
        CPU_TRACE_BLOCK_BEGIN("byte(1)");
        CPU_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.es, cedi, 1, 1));
        CPU_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, X86_CPU_REFERENCE_OF(data), 1));
        CPU_TRACE_CHECK_RETURN(_s_write_es(context, cedi, X86_CPU_REFERENCE_OF(data), 1));
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 0, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 2:
        CPU_TRACE_BLOCK_BEGIN("byte(2)");
        CPU_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.es, cedi, 2, 1));
        CPU_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, X86_CPU_REFERENCE_OF(data), 2));
        CPU_TRACE_CHECK_RETURN(_s_write_es(context, cedi, X86_CPU_REFERENCE_OF(data), 2));
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 0, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("byte(4)");
        CPU_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.es, cedi, 4, 1));
        CPU_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, X86_CPU_REFERENCE_OF(data), 4));
        CPU_TRACE_CHECK_RETURN(_s_write_es(context, cedi, X86_CPU_REFERENCE_OF(data), 4));
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 0, 1));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("byte");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _p_outs(x86_core_cpu_execution_context *context, lib_u8 byte)
{
    lib_u32 cesi, data = 0;
    CPU_TRACE_CALL_BEGIN("_p_outs");
    switch (_GetAddressSize)
    {
    case 2:
        cesi = cpu_state.data.si;
        break;
    case 4:
        cesi = cpu_state.data.esi;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (byte)
    {
    case 1:
        CPU_TRACE_BLOCK_BEGIN("byte(1)");
        CPU_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, X86_CPU_REFERENCE_OF(data), 1));
        CPU_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx, X86_CPU_REFERENCE_OF(data), 1));
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 1, 0));
        CPU_TRACE_BLOCK_END;
        break;
    case 2:
        CPU_TRACE_BLOCK_BEGIN("byte(2)");
        CPU_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, X86_CPU_REFERENCE_OF(data), 2));
        CPU_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx, X86_CPU_REFERENCE_OF(data), 2));
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 1, 0));
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("byte(4)");
        CPU_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, X86_CPU_REFERENCE_OF(data), 4));
        CPU_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx, X86_CPU_REFERENCE_OF(data), 4));
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 1, 0));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("byte");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
/* Validate both operands of one string iteration before reading either.
 * 486 ordering is segment, alignment, then paging across both operands. */
static void _m_test_string(x86_core_cpu_execution_context *context,
    lib_u32 source, lib_u32 destination, lib_u8 byte, lib_bool write)
{
    lib_u32 source_linear, destination_linear;
    CPU_TRACE_CALL_BEGIN("_m_test_string");
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) {
        CPU_TRACE_CHECK_RETURN(source_linear = _kma_linear_logical(context,
            instruction_state.data.roverds, source, byte, LIB_FALSE, _GetCPL, 0u));
        CPU_TRACE_CHECK_RETURN(destination_linear = _kma_linear_logical(context,
            &cpu_state.data.es, destination, byte, write, _GetCPL, 0u));
        CPU_TRACE_CHECK_RETURN(_kma_check_alignment(context,
            source_linear, byte, _GetCPL, 0u));
        CPU_TRACE_CHECK_RETURN(_kma_check_alignment(context,
            destination_linear, byte, _GetCPL, 0u));
        CPU_TRACE_CHECK_RETURN(_kma_test_linear(context,
            source_linear, byte, LIB_FALSE, _GetCPL, 0u));
        CPU_TRACE_CHECK_RETURN(_kma_test_linear(context,
            destination_linear, byte, write, _GetCPL, 0u));
    }
    CPU_TRACE_CALL_END;
}
static void _m_movs(x86_core_cpu_execution_context *context, lib_u8 byte)
{
    lib_u32 data = 0;
    lib_u32 cesi, cedi;
    CPU_TRACE_CALL_BEGIN("_m_movs");
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
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CHECK_RETURN(_m_test_string(context, cesi, cedi, byte, LIB_TRUE));
    switch (byte)
    {
    case 1:
        CPU_TRACE_BLOCK_BEGIN("byte(1)");
        CPU_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, X86_CPU_REFERENCE_OF(data), 1));
        CPU_TRACE_CHECK_RETURN(_s_write_es(context, cedi, X86_CPU_REFERENCE_OF(data), 1));
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 1, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 2:
        CPU_TRACE_BLOCK_BEGIN("byte(2)");
        CPU_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, X86_CPU_REFERENCE_OF(data), 2));
        CPU_TRACE_CHECK_RETURN(_s_write_es(context, cedi, X86_CPU_REFERENCE_OF(data), 2));
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 1, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("byte(4)");
        CPU_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, X86_CPU_REFERENCE_OF(data), 4));
        CPU_TRACE_CHECK_RETURN(_s_write_es(context, cedi, X86_CPU_REFERENCE_OF(data), 4));
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 1, 1));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("byte");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _m_stos(x86_core_cpu_execution_context *context, lib_u8 byte)
{
    lib_u32 cedi;
    CPU_TRACE_CALL_BEGIN("_m_stos");
    switch (_GetAddressSize)
    {
    case 2:
        cedi = cpu_state.data.di;
        break;
    case 4:
        cedi = cpu_state.data.edi;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (byte)
    {
    case 1:
        CPU_TRACE_BLOCK_BEGIN("byte(1)");
        CPU_TRACE_CHECK_RETURN(_s_write_es(context, cedi, X86_CPU_REFERENCE_OF(cpu_state.data.al), 1));
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 0, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 2:
        CPU_TRACE_BLOCK_BEGIN("byte(2)");
        CPU_TRACE_CHECK_RETURN(_s_write_es(context, cedi, X86_CPU_REFERENCE_OF(cpu_state.data.ax), 2));
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 0, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("byte(4)");
        CPU_TRACE_CHECK_RETURN(_s_write_es(context, cedi, X86_CPU_REFERENCE_OF(cpu_state.data.eax), 4));
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 0, 1));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("byte");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _m_lods(x86_core_cpu_execution_context *context, lib_u8 byte)
{
    lib_u32 cesi = 0x00000000;
    CPU_TRACE_CALL_BEGIN("_m_lods");
    switch (_GetAddressSize)
    {
    case 2:
        cesi = cpu_state.data.si;
        break;
    case 4:
        cesi = cpu_state.data.esi;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (byte)
    {
    case 1:
        CPU_TRACE_BLOCK_BEGIN("byte(1)");
        instruction_state.data.bit = 8;
        CPU_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, X86_CPU_REFERENCE_OF(cpu_state.data.al), 1));
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 1, 0));
        instruction_state.data.result = cpu_state.data.al;
        CPU_TRACE_BLOCK_END;
        break;
    case 2:
        CPU_TRACE_BLOCK_BEGIN("byte(2)");
        instruction_state.data.bit = 16;
        CPU_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, X86_CPU_REFERENCE_OF(cpu_state.data.ax), 2));
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 1, 0));
        instruction_state.data.result = cpu_state.data.ax;
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("byte(4)");
        instruction_state.data.bit = 32;
        CPU_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, X86_CPU_REFERENCE_OF(cpu_state.data.eax), 4));
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 1, 0));
        instruction_state.data.result = cpu_state.data.eax;
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("byte");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(byte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _a_cmps(x86_core_cpu_execution_context *context, lib_u8 bit)
{
    lib_u32 cesi, cedi;
    CPU_TRACE_CALL_BEGIN("_a_cmps");
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
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CHECK_RETURN(_m_test_string(context, cesi, cedi, bit / 8u, LIB_FALSE));
    switch (bit)
    {
    case 8:
        CPU_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.type = CMP8;
        CPU_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, X86_CPU_REFERENCE_OF(instruction_state.data.opr1), 1));
        CPU_TRACE_CHECK_RETURN(_s_read_es(context, cedi, X86_CPU_REFERENCE_OF(instruction_state.data.opr2), 1));
        instruction_state.data.result = X86_CPU_MASK_U8(instruction_state.data.opr1 - instruction_state.data.opr2);
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 1, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 16:
        CPU_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.type = CMP16;
        CPU_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, X86_CPU_REFERENCE_OF(instruction_state.data.opr1), 2));
        CPU_TRACE_CHECK_RETURN(_s_read_es(context, cedi, X86_CPU_REFERENCE_OF(instruction_state.data.opr2), 2));
        instruction_state.data.result = X86_CPU_MASK_U16(instruction_state.data.opr1 - instruction_state.data.opr2);
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 1, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 32:
        CPU_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.type = CMP32;
        CPU_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, cesi, X86_CPU_REFERENCE_OF(instruction_state.data.opr1), 4));
        CPU_TRACE_CHECK_RETURN(_s_read_es(context, cedi, X86_CPU_REFERENCE_OF(instruction_state.data.opr2), 4));
        instruction_state.data.result = X86_CPU_MASK_U32(instruction_state.data.opr1 - instruction_state.data.opr2);
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 1, 1));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, CMP_FLAG));
    CPU_TRACE_CALL_END;
}
static void _a_scas(x86_core_cpu_execution_context *context, lib_u8 bit)
{
    lib_u32 cedi;
    CPU_TRACE_CALL_BEGIN("_a_scas");
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
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    switch (bit)
    {
    case 8:
        CPU_TRACE_BLOCK_BEGIN("bit(8)");
        instruction_state.data.bit = 8;
        instruction_state.data.type = CMP8;
        CPU_TRACE_CHECK_RETURN(instruction_state.data.opr1 = cpu_state.data.al);
        CPU_TRACE_CHECK_RETURN(_s_read_es(context, cedi, X86_CPU_REFERENCE_OF(instruction_state.data.opr2), 1));
        instruction_state.data.result = X86_CPU_MASK_U8(instruction_state.data.opr1 - instruction_state.data.opr2);
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 1, 0, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 16:
        CPU_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.bit = 16;
        instruction_state.data.type = CMP16;
        CPU_TRACE_CHECK_RETURN(instruction_state.data.opr1 = cpu_state.data.ax);
        CPU_TRACE_CHECK_RETURN(_s_read_es(context, cedi, X86_CPU_REFERENCE_OF(instruction_state.data.opr2), 2));
        instruction_state.data.result = X86_CPU_MASK_U16(instruction_state.data.opr1 - instruction_state.data.opr2);
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 2, 0, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 32:
        CPU_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.bit = 32;
        instruction_state.data.type = CMP32;
        CPU_TRACE_CHECK_RETURN(instruction_state.data.opr1 = cpu_state.data.eax);
        CPU_TRACE_CHECK_RETURN(_s_read_es(context, cedi, X86_CPU_REFERENCE_OF(instruction_state.data.opr2), 4));
        instruction_state.data.result = X86_CPU_MASK_U32(instruction_state.data.opr1 - instruction_state.data.opr2);
        CPU_TRACE_CHECK_RETURN(_kas_move_index(context, 4, 0, 1));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, CMP_FLAG));
    CPU_TRACE_CALL_END;
}
#define _adv CPU_TRACE_CHECK_RETURN(_d_skip(context, 1))

static lib_u8 x86_core_cpu_instruction_lexeme_is_prefix(
    lib_u8 byte, x86_core_cpu_profile profile)
{
    switch (byte) {
    case 0x26u: case 0x2eu: case 0x36u: case 0x3eu:
    case 0xf0u: case 0xf2u: case 0xf3u:
        return LIB_TRUE;
    case 0x64u: case 0x65u: case 0x66u: case 0x67u:
        return cpu_profile_has_32bit_architecture(profile);
    default:
        return LIB_FALSE;
    }
}

static lib_u8 x86_core_cpu_instruction_lexeme_has_modrm(
    lib_u8 opcode, lib_u8 extended)
{
    if (extended) {
        return opcode <= 0x03u ||
            (opcode >= 0x10u && opcode <= 0x13u) ||
            (opcode >= 0x20u && opcode <= 0x24u) || opcode == 0x26u ||
            (opcode >= 0x90u && opcode <= 0x9fu) || opcode == 0xa3u ||
            (opcode >= 0xa4u && opcode <= 0xa5u) || opcode == 0xabu ||
            (opcode >= 0xacu && opcode <= 0xadu) || opcode == 0xafu ||
            (opcode >= 0xb0u && opcode <= 0xbfu) ||
            opcode == 0xc0u || opcode == 0xc1u;
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

static lib_u8 x86_core_cpu_instruction_lexeme_modrm_form_valid(
    lib_u8 opcode, lib_u8 extended, lib_u8 modrm,
    lib_u8 cpu_80386_cr_mov_ignores_mod, x86_core_cpu_profile profile)
{
    lib_u8 reg = (modrm >> 3u) & 7u;

    if (extended) {
        if ((opcode == 0x24u || opcode == 0x26u) &&
            (reg < (profile == X86_CORE_CPU_PROFILE_80486SX ? 3u : 6u) ||
             (opcode == 0x24u && reg == 5u))) return LIB_FALSE;
        if (opcode == 0x01u && (reg == 5u ||
            (reg == 7u && profile != X86_CORE_CPU_PROFILE_80486SX))) return LIB_FALSE;
        if ((opcode >= 0x20u && opcode <= 0x24u) || opcode == 0x26u) {
            if ((opcode == 0x20u || opcode == 0x22u) &&
                cpu_80386_cr_mov_ignores_mod) return LIB_TRUE;
            return (modrm >> 6u) == 3u;
        }
        if ((opcode == 0x01u && (reg <= 3u || reg == 7u)) || opcode == 0xb2u ||
            opcode == 0xb4u || opcode == 0xb5u)
            return (modrm >> 6u) != 3u;
        return LIB_TRUE;
    }
    if (opcode == 0x62u || opcode == 0x8du || opcode == 0xc4u || opcode == 0xc5u)
        return (modrm >> 6u) != 3u;
    if (opcode == 0xffu && (reg == 3u || reg == 5u))
        return (modrm >> 6u) != 3u;
    if (opcode == 0x8fu || opcode == 0xc6u || opcode == 0xc7u)
        return reg == 0u;
    if (opcode == 0xfeu)
        return reg <= 1u;
    if (opcode == 0xffu)
        return reg <= 6u;
    return LIB_TRUE;
}
static lib_u8 x86_core_cpu_instruction_lexeme_immediate_bytes(
    lib_u8 opcode, lib_u8 extended, lib_u8 modrm,
    lib_u8 operand_bytes, lib_u8 address_bytes)
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

typedef enum x86_core_cpu_nec_format {
    X86_CPU_NEC_FORMAT_UNSUPPORTED,
    X86_CPU_NEC_FORMAT_MODRM,
    X86_CPU_NEC_FORMAT_MODRM_IMMEDIATE,
    X86_CPU_NEC_FORMAT_IMPLICIT
} x86_core_cpu_nec_format;

static x86_core_cpu_nec_format x86_core_cpu_nec_instruction_format(lib_u8 opcode)
{
    if ((opcode >= 0x10u && opcode <= 0x17u) || opcode == 0x28u || opcode == 0x2au)
        return X86_CPU_NEC_FORMAT_MODRM;
    if (opcode >= 0x18u && opcode <= 0x1fu)
        return X86_CPU_NEC_FORMAT_MODRM_IMMEDIATE;
    if (opcode == 0x20u || opcode == 0x22u || opcode == 0x26u)
        return X86_CPU_NEC_FORMAT_IMPLICIT;
    return X86_CPU_NEC_FORMAT_UNSUPPORTED;
}

static lib_u8 x86_core_cpu_instruction_lexeme_scan_with_options(
    const lib_u8 *bytes, lib_u8 available_bytes,
    x86_core_cpu_profile profile, lib_u8 code_32,
    lib_u8 cpu_80386_cr_mov_ignores_mod,
    x86_core_cpu_instruction_lexeme *out_lexeme)
{
    lib_u8 index = 0u;
    lib_u8 components = 0u;
    lib_u8 opcode;
    lib_u8 modrm = 0u;
    lib_u8 operand_bytes;
    lib_u8 address_bytes;
    lib_u8 displacement = 0u;
    lib_u8 immediate;
    lib_u8 extended = LIB_FALSE;
    lib_bool nec = LIB_FALSE;
    x86_core_cpu_nec_format nec_format = X86_CPU_NEC_FORMAT_UNSUPPORTED;
    lib_u8 lock_prefix = LIB_FALSE;
    lib_u8 operand_size_prefix = LIB_FALSE;
    lib_u8 address_size_prefix = LIB_FALSE;
    x86_core_cpu_instruction_metadata metadata;

    if (out_lexeme == LIB_NULL) return LIB_FALSE;
    *out_lexeme = (x86_core_cpu_instruction_lexeme) { 0u, 0u, LIB_FALSE };
    if (bytes == LIB_NULL || available_bytes == 0u ||
        !cpu_profile_allows_minimum(profile, X86_CORE_CPU_PROFILE_8086) ||
        (profile == X86_CORE_CPU_PROFILE_V30 && code_32)) return LIB_FALSE;
    while (index < available_bytes && index < 15u &&
        x86_core_cpu_instruction_lexeme_is_prefix(bytes[index], profile)) {
        if (bytes[index] == 0xf0u) lock_prefix = LIB_TRUE;
        ++index;
        ++components;
    }
    if (index < available_bytes && !cpu_profile_has_32bit_architecture(profile) &&
        (bytes[index] == 0x64u || bytes[index] == 0x65u ||
         bytes[index] == 0x66u || bytes[index] == 0x67u)) return LIB_FALSE;
    if (index >= available_bytes || index >= 15u) return LIB_FALSE;
    opcode = bytes[index++];
    ++components;
    operand_bytes = code_32 ? 4u : 2u;
    address_bytes = code_32 ? 4u : 2u;
    if (cpu_profile_has_32bit_architecture(profile)) {
        lib_u8 prefix_index;

        for (prefix_index = 0u; prefix_index < index - 1u; ++prefix_index) {
            if (bytes[prefix_index] == 0x66u) operand_size_prefix = LIB_TRUE;
            if (bytes[prefix_index] == 0x67u) address_size_prefix = LIB_TRUE;
        }
        if (operand_size_prefix)
            operand_bytes = code_32 ? 2u : 4u;
        if (address_size_prefix)
            address_bytes = code_32 ? 2u : 4u;
    }
    if (opcode == 0x0fu && !x86_core_cpu_profile_has_8086_semantics(profile)) {
        nec = profile == X86_CORE_CPU_PROFILE_V30;
        if (!nec && profile != X86_CORE_CPU_PROFILE_80286 &&
            profile != X86_CORE_CPU_PROFILE_80386 &&
            profile != X86_CORE_CPU_PROFILE_80486SX) return LIB_FALSE;
        if (index >= available_bytes || index >= 15u) return LIB_FALSE;
        opcode = bytes[index++];
        if (nec) nec_format = x86_core_cpu_nec_instruction_format(opcode);
        ++components;
        extended = LIB_TRUE;
    }
    if (nec ? nec_format == X86_CPU_NEC_FORMAT_MODRM ||
        nec_format == X86_CPU_NEC_FORMAT_MODRM_IMMEDIATE :
        x86_core_cpu_instruction_lexeme_has_modrm(opcode, extended)) {
        lib_u8 mod;
        lib_u8 rm;

        if (index >= available_bytes || index >= 15u) return LIB_FALSE;
        modrm = bytes[index++];
        ++components;
        mod = modrm >> 6u;
        rm = modrm & 7u;
        if (mod != 3u && !(extended && cpu_80386_cr_mov_ignores_mod &&
            (opcode == 0x20u || opcode == 0x22u))) {
            if (address_bytes == 4u) {
                if (rm == 4u) {
                    lib_u8 sib;

                    if (index >= available_bytes || index >= 15u) return LIB_FALSE;
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
    if (!nec && !x86_core_cpu_instruction_lexeme_modrm_form_valid(opcode, extended, modrm,
        cpu_80386_cr_mov_ignores_mod, profile))
        return LIB_FALSE;
    metadata = x86_core_cpu_instruction_metadata_get(nec ?
        X86_CORE_CPU_INSTRUCTION_NEC_0F : extended ?
        X86_CORE_CPU_INSTRUCTION_0F : X86_CORE_CPU_INSTRUCTION_PRIMARY,
        opcode, modrm);
    if (extended && opcode >= 0xc8u && opcode <= 0xcfu && operand_bytes != 4u)
        return LIB_FALSE;
    if (!metadata.valid || !cpu_profile_allows_minimum(profile, metadata.minimum_cpu) || lock_prefix) {
        return LIB_FALSE;
    }
    immediate = nec ? (nec_format == X86_CPU_NEC_FORMAT_MODRM_IMMEDIATE ? 1u : 0u) :
        x86_core_cpu_instruction_lexeme_immediate_bytes(opcode,
            extended, modrm, operand_bytes, address_bytes);
    if ((lib_u32)index + displacement + immediate > available_bytes ||
        (lib_u32)index + displacement + immediate > 15u) return LIB_FALSE;
    if (displacement != 0u) {
        index = (lib_u8)(index + displacement);
        ++components;
    }
    if (immediate != 0u) {
        index = (lib_u8)(index + immediate);
        ++components;
    }
    out_lexeme->byte_count = index;
    out_lexeme->component_count = components;
    out_lexeme->available = LIB_TRUE;
    return LIB_TRUE;
}

x86_core_cpu_instruction_metadata x86_core_cpu_instruction_metadata_get(
    x86_core_cpu_instruction_space space, lib_u8 opcode, lib_u8 modrm)
{
    x86_core_cpu_instruction_metadata metadata = {
        X86_CORE_CPU_PROFILE_8086, X86_FPU_PROFILE_NONE, 1};

    switch (space)
    {
    case X86_CORE_CPU_INSTRUCTION_PRIMARY:
        if (opcode >= 0xd8u && opcode <= 0xdfu)
        {
            metadata.minimum_fpu = X86_FPU_PROFILE_8087;
        }
        else if ((opcode >= 0x60u && opcode <= 0x62u) || opcode == 0x68u ||
                 opcode == 0x69u || opcode == 0x6au || opcode == 0x6bu ||
                 (opcode >= 0x6cu && opcode <= 0x6fu) ||
                 opcode == 0xc8u || opcode == 0xc9u)
        {
            metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80186;
        }
        else if (opcode == 0x63u)
        {
            metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80286;
        }
        else if (opcode >= 0x64u && opcode <= 0x67u)
        {
            metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80386;
        }
        else if (opcode == 0xd6u || opcode == 0xf1u)
        {
            metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80486SX;
        }
        else if (opcode == 0x8cu)
        {
            if (((modrm >> 3u) & 7u) >= 6u) metadata.valid = 0;
            else if (((modrm >> 3u) & 7u) >= 4u)
                metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80386;
        }
        else if (opcode == 0x8eu)
        {
            if (((modrm >> 3u) & 7u) == 1u ||
                ((modrm >> 3u) & 7u) >= 6u) metadata.valid = 0;
            else if (((modrm >> 3u) & 7u) >= 4u)
                metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80386;
        }
        else if (opcode == 0xc0u || opcode == 0xc1u ||
                 (opcode >= 0xd0u && opcode <= 0xd3u))
        {
            if (opcode == 0xc0u || opcode == 0xc1u)
                metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80186;
            if (((modrm >> 3u) & 7u) == 6u)
                metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80486SX;
        }
        else if ((opcode == 0xf6u || opcode == 0xf7u) &&
                 ((modrm >> 3u) & 7u) == 1u)
        {
            metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80486SX;
        }
        break;
    case X86_CORE_CPU_INSTRUCTION_0F:
        metadata.valid = 0;
        if (opcode == 0x00u)
        {
            metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80286;
            metadata.valid = ((modrm >> 3u) & 7u) <= 5u;
        }
        else if (opcode == 0x01u)
        {
            metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80286;
            metadata.valid = ((modrm >> 3u) & 7u) <= 6u;
            if (((modrm >> 3u) & 7u) == 7u) {
                metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80486SX;
                metadata.valid = (modrm >> 6u) != 3u;
            }
        }
        else if (opcode == 0x02u || opcode == 0x03u)
        {
            metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80286;
            metadata.valid = 1;
        }
        else if (opcode == 0x06u)
        {
            metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80286;
            metadata.valid = 1;
        }
        else if (opcode >= 0x10u && opcode <= 0x13u)
        {
            metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80486SX;
            metadata.valid = LIB_TRUE;
        }
        else if (opcode == 0x20u || opcode == 0x22u)
        {
            const lib_u8 reg = (modrm >> 3u) & 7u;

            metadata.minimum_cpu = reg == 4u ? X86_CORE_CPU_PROFILE_80486SX :
                X86_CORE_CPU_PROFILE_80386;
            metadata.valid = reg != 1u && reg <= 4u;
        }
        else if ((opcode == 0x21u || opcode == 0x23u) &&
                 (((modrm >> 3u) & 7u) == 4u || ((modrm >> 3u) & 7u) == 5u))
        {
            metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80486SX;
            metadata.valid = LIB_TRUE;
        }
        else if (opcode == 0x21u || opcode == 0x23u || opcode == 0x24u || opcode == 0x26u ||
                 (opcode >= 0x80u && opcode <= 0x8fu) ||
                 (opcode >= 0x90u && opcode <= 0x9fu) ||
                 opcode == 0xa0u || opcode == 0xa1u || opcode == 0xa3u ||
                 opcode == 0xa4u || opcode == 0xa5u || opcode == 0xa8u ||
                 opcode == 0xa9u || opcode == 0xabu || opcode == 0xacu ||
                 opcode == 0xadu || opcode == 0xafu ||
                 (opcode >= 0xb2u && opcode <= 0xb7u) ||
                 (opcode >= 0xbbu && opcode <= 0xbfu))
        {
            metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80386;
            metadata.valid = 1;
        }
        else if (opcode == 0xbau)
        {
            metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80386;
            metadata.valid = ((modrm >> 3u) & 7u) >= 4u;
        }
        else if (opcode == 0x08u || opcode == 0x09u ||
                 opcode == 0xa2u || opcode == 0xaau || opcode == 0xb0u || opcode == 0xb1u ||
                 opcode == 0xc0u || opcode == 0xc1u ||
                 (opcode >= 0xc8u && opcode <= 0xcfu))
        {
            metadata.minimum_cpu = X86_CORE_CPU_PROFILE_80486SX;
            metadata.valid = LIB_TRUE;
        }
        break;
    case X86_CORE_CPU_INSTRUCTION_FPU_ESCAPE:
        metadata.minimum_fpu = X86_FPU_PROFILE_8087;
        metadata.valid = opcode >= 0xd8u && opcode <= 0xdfu;
        break;
    case X86_CORE_CPU_INSTRUCTION_NEC_0F: {
        const x86_core_cpu_nec_format format = x86_core_cpu_nec_instruction_format(opcode);
        metadata.minimum_cpu = X86_CORE_CPU_PROFILE_V30;
        metadata.valid = format != X86_CPU_NEC_FORMAT_UNSUPPORTED &&
            (format == X86_CPU_NEC_FORMAT_IMPLICIT || (modrm & 0x38u) == 0u);
        break;
    }
    default:
        metadata.valid = LIB_FALSE;
        break;
    }
    return metadata;
}

static lib_i32 x86_core_cpu_profile_allows_form(
    const x86_core_cpu_execution_context *context,
    x86_core_cpu_instruction_space space, lib_u8 opcode, lib_u8 modrm)
{
    x86_core_cpu_instruction_metadata metadata;

    if (context == LIB_NULL)
        return 0;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_V30 &&
        space != X86_CORE_CPU_INSTRUCTION_PRIMARY &&
        space != X86_CORE_CPU_INSTRUCTION_NEC_0F) return 0;
    metadata = x86_core_cpu_instruction_metadata_get(space, opcode, modrm);
    return metadata.valid && cpu_profile_allows_minimum(context->cpu_profile, metadata.minimum_cpu);
}

lib_u8 x86_core_cpu_instruction_lexeme_scan(
    const lib_u8 *bytes, lib_u8 available_bytes,
    x86_core_cpu_profile profile, lib_u8 code_32,
    x86_core_cpu_instruction_lexeme *out_lexeme)
{
    return x86_core_cpu_instruction_lexeme_scan_with_options(bytes,
        available_bytes, profile, code_32, LIB_FALSE, out_lexeme);
}
static void UndefinedOpcode(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("UndefinedOpcode");
    cpu_state = instruction_state.data.oldcpu;
    CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
    CPU_TRACE_CALL_END;
}
static lib_bool x86_core_cpu_allows_fpu(x86_core_cpu_profile cpu,
    x86_fpu_profile fpu)
{
    if (fpu == X86_FPU_PROFILE_NONE) return LIB_TRUE;
    if (fpu == X86_FPU_PROFILE_8087) {
        return x86_core_cpu_profile_has_8086_semantics(cpu) ||
            cpu == X86_CORE_CPU_PROFILE_80186;
    }
    if (fpu == X86_FPU_PROFILE_80287) {
        return cpu == X86_CORE_CPU_PROFILE_80286 ||
            cpu == X86_CORE_CPU_PROFILE_80386;
    }
    return fpu == X86_FPU_PROFILE_80387 &&
        cpu == X86_CORE_CPU_PROFILE_80386;
}

static lib_bool x86_core_cpu_wait_test_asserted(
    const x86_core_cpu_execution_context *context)
{
    return context->bus->wait_test_asserted != LIB_NULL &&
        context->bus->wait_test_asserted(context->bus_context);
}

static void x86_core_cpu_begin_wait(
    x86_core_cpu_execution_context *context, cpu_wait_state state)
{
    if (state == CPU_WAIT_STATE_ESC_RESTART) {
        cpu_state = instruction_state.data.oldcpu;
        context->instruction_wait_stall = LIB_TRUE;
    }
    context->wait_state = state;
    /* The initial CPU-side WAIT row has not yet been published.  The next
     * re-sample is five clocks after that row, yielding the 3/6 + 5n source
     * formulas for 8086/8088 and 80186.  ESC restart has no retired WAIT row,
     * so it starts at the bounded five-clock L2 control cadence instead.
     * Later CPU profiles retain that cadence until a board gives a stronger
     * contract. */
    context->wait_poll_remaining_ticks = state == CPU_WAIT_STATE_ESC_RESTART ?
        5u : (context->cpu_profile == X86_CORE_CPU_PROFILE_80186 ? 6u :
            cpu_profile_has_32bit_architecture(context->cpu_profile) ? 7u : 3u) + 5u;
    cpu_state.data.flagHalt = LIB_TRUE;
}

static void FPU_ESCAPE(x86_core_cpu_execution_context *context)
{
    x86_fpu_escape_action action;
    x86_core_cpu_instruction_metadata metadata;
    x86_fpu_operation_metadata fpu_metadata;
    lib_u8 escape_opcode = 0u;
    lib_u8 modrm = 0u;
    lib_u32 fpu_m32;

    CPU_TRACE_CALL_BEGIN("FPU_ESCAPE");
    CPU_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip,
                                       X86_CPU_REFERENCE_OF(escape_opcode), 1));
    _adv;
    CPU_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip,
                                       X86_CPU_REFERENCE_OF(modrm), 1));
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
    metadata = x86_core_cpu_instruction_metadata_get(
        X86_CORE_CPU_INSTRUCTION_FPU_ESCAPE, escape_opcode, modrm);
    if (!metadata.valid)
    {
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80186 &&
        context->bus->escape_trap_enabled != LIB_NULL &&
        context->bus->escape_trap_enabled(context->bus_context))
    {
        CPU_TRACE_CHECK_RETURN(_SetExcept_NM(0));
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX ||
        _GetCR0_EM || _GetCR0_TS)
    {
        CPU_TRACE_CHECK_RETURN(_SetExcept_NM(0));
    }
    else
    {
        action = x86_core_cpu_allows_fpu(context->cpu_profile,
            x86_fpu_get_profile(context->fpu)) ?
            x86_fpu_escape_dispatch(context->fpu, escape_opcode, modrm) :
            X86_FPU_ESCAPE_UNSUPPORTED;
        if (action == X86_FPU_ESCAPE_UNSUPPORTED)
        {
            CPU_TRACE_CHECK_RETURN(_SetExcept_FPU_UNSUPPORTED(0));
        }
        else if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 &&
            action != X86_FPU_ESCAPE_CONSUME_NONE &&
            x86_core_cpu_wait_test_asserted(context))
        {
            /* 80286 waits for the 80287 BUSY/TEST connection before starting
             * an ESC command.  Restore the ESC so interrupt return retries
             * it, and let the existing Core deadline advance the FPU. */
            x86_core_cpu_begin_wait(context, CPU_WAIT_STATE_ESC_RESTART);
            CPU_TRACE_CALL_END;
            return;
        }
        else if (action != X86_FPU_ESCAPE_CONSUME_NONE &&
            !context->preview_mode)
        {
            context->bus->extension_command(context->bus_context,
                escape_opcode, modrm);
        }
        if (action == X86_FPU_ESCAPE_EXECUTE_8087)
        {
            fpu_metadata = x86_fpu_operation_metadata_get(escape_opcode, modrm);
            switch (fpu_metadata.operation)
            {
            case X86_FPU_OPERATION_FNINIT:
                x86_fpu_reset(context->fpu);
                break;
            case X86_FPU_OPERATION_FLD_M32:
                CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 4));
                (void)x86_fpu_load_m32(context->fpu,
                    X86_CPU_MASK_U32(instruction_state.data.crm));
                break;
            case X86_FPU_OPERATION_FSTP_M32:
                if (x86_fpu_store_m32(context->fpu, &fpu_m32) ==
                    X86_FPU_EXECUTE_COMPLETED) {
                    instruction_state.data.crm = fpu_m32;
                    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 4));
                }
                break;
            case X86_FPU_OPERATION_FLDCW_M16:
                CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
                x86_fpu_load_control_word(context->fpu,
                    X86_CPU_MASK_U16(instruction_state.data.crm));
                break;
            case X86_FPU_OPERATION_FADD_ST0_STI:
            case X86_FPU_OPERATION_FMUL_ST0_STI:
            case X86_FPU_OPERATION_FSUB_ST0_STI:
            case X86_FPU_OPERATION_FDIV_ST0_STI:
                (void)x86_fpu_binary_st0_sti(context->fpu,
                    fpu_metadata.operation, (lib_u8)(modrm & 7u));
                break;
            default: break;
            }
            /* Arithmetic coverage is deliberately partial.  An operation
             * outside that semantic subset remains a valid coprocessor
             * command handoff; it is not a CPU #UD or model-only fault. */
        }
        if (action != X86_FPU_ESCAPE_CONSUME_NONE) {
            x86_fpu_begin_command(context->fpu, escape_opcode, modrm);
        }
    }
    CPU_TRACE_CALL_END;
}
static void ADD_RM8_R8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("ADD_RM8_R8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 1, LIB_FALSE));
    CPU_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    CPU_TRACE_CALL_END;
}
static void ADD_RM32_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("ADD_RM32_R32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
        CPU_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 2, LIB_FALSE));
        CPU_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    CPU_TRACE_CALL_END;
}
static void ADD_R8_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("ADD_R8_RM8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), 1));
    CPU_TRACE_CALL_END;
}
static void ADD_R32_RM32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("ADD_R32_RM32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        CPU_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), 2));
    }
    CPU_TRACE_CALL_END;
}
static void ADD_AL_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("ADD_AL_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_add(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    _WriteGPR8(al) = X86_CPU_MASK_U8(instruction_state.data.result);
    CPU_TRACE_CALL_END;
}
static void ADD_EAX_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("ADD_EAX_I32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
            CPU_TRACE_CHECK_RETURN(_a_add(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 4));
            CPU_TRACE_CHECK_RETURN(_a_add(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            _WriteGPR32(eax) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        CPU_TRACE_CHECK_RETURN(_a_add(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void PUSH_ES(x86_core_cpu_execution_context *context)
{
    lib_u32 xs_sel;
    CPU_TRACE_CALL_BEGIN("PUSH_ES");
    _adv;
    xs_sel = cpu_state.data.es.selector;
    CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(xs_sel), _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void POP_ES(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("POP_ES");
    _adv;
    CPU_TRACE_CHECK_RETURN(_e_pop_sreg(context, &cpu_state.data.es,
        _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void OR_RM8_R8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("OR_RM8_R8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 1, LIB_FALSE));
    CPU_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    CPU_TRACE_CALL_END;
}
static void OR_RM32_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("OR_RM32_R32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
        CPU_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 2, LIB_FALSE));
        CPU_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    CPU_TRACE_CALL_END;
}
static void OR_R8_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("OR_R8_RM8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), 1));
    CPU_TRACE_CALL_END;
}
static void OR_R32_RM32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("OR_R32_RM32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        CPU_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), 2));
    }
    CPU_TRACE_CALL_END;
}
static void OR_AL_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("OR_AL_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_or(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    _WriteGPR8(al) = X86_CPU_MASK_U8(instruction_state.data.result);
    CPU_TRACE_CALL_END;
}
static void OR_EAX_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("OR_EAX_I32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
            CPU_TRACE_CHECK_RETURN(_a_or(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 4));
            CPU_TRACE_CHECK_RETURN(_a_or(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            _WriteGPR32(eax) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        CPU_TRACE_CHECK_RETURN(_a_or(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void PUSH_CS(x86_core_cpu_execution_context *context)
{
    lib_u32 xs_sel;
    CPU_TRACE_CALL_BEGIN("PUSH_CS");
    _adv;
    xs_sel = cpu_state.data.cs.selector;
    CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(xs_sel), _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void POP_CS(x86_core_cpu_execution_context *context)
{
    /* note: not sure if operand size is 32,
        push/pop selector only or with higher 16 bit */
    lib_u32 xs_sel;
    CPU_TRACE_CALL_BEGIN("POP_CS");
    _adv;
    CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(xs_sel), _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_s_load_cs(context, X86_CPU_MASK_U16(xs_sel)));
    CPU_TRACE_CALL_END;
}
static void NEC_BIT_NIBBLE(x86_core_cpu_execution_context *context,
    lib_u8 opcode, x86_core_cpu_nec_format format)
{
    lib_u8 count;
    lib_u8 byte;
    lib_u8 operation;
    lib_u8 al;
    lib_u16 mask;
    CPU_TRACE_CALL_BEGIN("NEC_BIT_NIBBLE");
    byte = opcode < 0x20u ? (lib_u8)(1u + (opcode & 1u)) : 1u;
    operation = (opcode >> 1u) & 3u;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, byte));
    count = cpu_state.data.cl;
    if (format == X86_CPU_NEC_FORMAT_MODRM_IMMEDIATE)
    {
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
        count = X86_CPU_MASK_U8(instruction_state.data.cimm);
    }
    if (opcode < 0x20u && operation == 0u)
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, byte));
    else
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, byte, LIB_FALSE));
    if (opcode < 0x20u)
    {
        mask = (lib_u16)(1u << (count & (byte * 8u - 1u)));
        switch (operation)
        {
        case 0:
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_ZF,
                (instruction_state.data.crm & mask) == 0u);
            cpu_state.data.eflags &= ~(VCPU_EFLAGS_CF | VCPU_EFLAGS_OF);
            instruction_state.data.udf |= VCPU_EFLAGS_SF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF;
            break;
        case 1:
            instruction_state.data.crm &= ~(lib_u64)mask;
            break;
        case 2:
            instruction_state.data.crm |= mask;
            break;
        case 3:
            instruction_state.data.crm ^= mask;
            break;
        }
        if (operation != 0u)
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, byte));
    }
    else
    {
        al = cpu_state.data.al;
        count = X86_CPU_MASK_U8(instruction_state.data.crm);
        instruction_state.data.crm = opcode == 0x28u ?
            X86_CPU_MASK_U8((count << 4u) | (al & 0x0fu)) :
            (lib_u8)(((al & 0x0fu) << 4u) | (count >> 4u));
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        /* The upper AL nibble is unspecified; retain its post-write value. */
        cpu_state.data.al = (cpu_state.data.al & 0xf0u) |
            (opcode == 0x28u ? count >> 4u : count & 0x0fu);
    }
    CPU_TRACE_CALL_END;
}
static void NEC_PACKED_BCD(x86_core_cpu_execution_context *context, lib_u8 opcode)
{
    const lib_u16 bytes = ((lib_u16)cpu_state.data.cl + 1u) / 2u;
    lib_bool carry = LIB_FALSE;
    lib_bool zero = LIB_TRUE;
    CPU_TRACE_CALL_BEGIN("NEC_PACKED_BCD");
    if (cpu_state.data.cl == 0u || cpu_state.data.cl == 255u)
    {
        /* These lengths are outside the documented 1--254 digit contract. */
        UndefinedOpcode(context);
        CPU_TRACE_CALL_END;
        return;
    }
    for (lib_u16 index = 0u; index < bytes; ++index)
    {
        const lib_u16 source_offset = X86_CPU_MASK_U16(cpu_state.data.si + index);
        const lib_u16 destination_offset = X86_CPU_MASK_U16(cpu_state.data.di + index);
        lib_u8 source = 0u;
        lib_u8 destination = 0u;
        lib_u8 result;
        lib_i32 value;
        lib_i32 source_value;
        if (opcode != 0x26u)
            CPU_TRACE_CHECK_RETURN(_m_test_access(context, &cpu_state.data.es,
                destination_offset, 1, LIB_TRUE));
        CPU_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds,
            source_offset, X86_CPU_REFERENCE_OF(source), 1));
        CPU_TRACE_CHECK_RETURN(_s_read_es(context, destination_offset,
            X86_CPU_REFERENCE_OF(destination), 1));
        if ((cpu_state.data.cl & 1u) && index + 1u == bytes)
        {
            /* The unused final nibble and odd-count flags are unspecified. */
            source &= 0x0fu;
            destination &= 0x0fu;
        }
        if ((source & 0x0fu) > 9u || (source >> 4u) > 9u ||
            (destination & 0x0fu) > 9u || (destination >> 4u) > 9u)
        {
            /* Do not invent NEC results for non-decimal operand digits. */
            UndefinedOpcode(context);
            CPU_TRACE_CALL_END;
            return;
        }
        source_value = (lib_i32)(source >> 4u) * 10 + (lib_i32)(source & 0x0fu);
        value = (lib_i32)(destination >> 4u) * 10 + (lib_i32)(destination & 0x0fu);
        if (opcode == 0x20u)
        {
            value += source_value + (carry ? 1 : 0);
            carry = value > 99;
            if (carry) value -= 100;
        }
        else
        {
            value -= source_value + (carry ? 1 : 0);
            carry = value < 0;
            if (carry) value += 100;
        }
        result = (lib_u8)(((lib_u32)(value / 10) << 4u) | (lib_u32)(value % 10));
        zero = zero && result == 0u;
        if (opcode != 0x26u)
            CPU_TRACE_CHECK_RETURN(_s_write_es(context, destination_offset,
                X86_CPU_REFERENCE_OF(result), 1));
    }
    X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, carry);
    X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_ZF, zero);
    instruction_state.data.udf |= VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF;
    if (cpu_state.data.cl & 1u)
        instruction_state.data.udf |= VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
    CPU_TRACE_CALL_END;
}
static void INS_NEC_0F(x86_core_cpu_execution_context *context)
{
    lib_u8 opcode;
    lib_u8 modrm = 0u;
    x86_core_cpu_nec_format format;
    CPU_TRACE_CALL_BEGIN("INS_NEC_0F");
    _adv;
    CPU_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip,
        X86_CPU_REFERENCE_OF(opcode), 1));
    format = x86_core_cpu_nec_instruction_format(opcode);
    if (format == X86_CPU_NEC_FORMAT_MODRM || format == X86_CPU_NEC_FORMAT_MODRM_IMMEDIATE)
        CPU_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip + 1u,
            X86_CPU_REFERENCE_OF(modrm), 1));
    if (!x86_core_cpu_profile_allows_form(context,
        X86_CORE_CPU_INSTRUCTION_NEC_0F, opcode, modrm))
    {
        UndefinedOpcode(context);
        CPU_TRACE_CALL_END;
        return;
    }
    _adv;
    if (format == X86_CPU_NEC_FORMAT_IMPLICIT)
        NEC_PACKED_BCD(context, opcode);
    else
        NEC_BIT_NIBBLE(context, opcode, format);
    CPU_TRACE_CALL_END;
}
static void INS_0F(x86_core_cpu_execution_context *context)
{
    lib_u8 opcode = 0x00;
    lib_u8 modrm = 0x00;
    CPU_TRACE_CALL_BEGIN("INS_0F");
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_V30)
        INS_NEC_0F(context);
    else if (cpu_profile_has_protected_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, X86_CPU_REFERENCE_OF(opcode), 1));
        if (opcode == 0x00u || opcode == 0x01u || opcode == 0xbau)
        {
            CPU_TRACE_CHECK_RETURN(_s_read_cs(context,
                                               cpu_state.data.eip + 1u, X86_CPU_REFERENCE_OF(modrm), 1));
        }
        if (!x86_core_cpu_profile_allows_form(context,
                                                  X86_CORE_CPU_INSTRUCTION_0F, opcode, modrm))
        {
            UndefinedOpcode(context);
            CPU_TRACE_CALL_END;
            return;
        }
        CPU_TRACE_CHECK_RETURN(ExecCpuInstruction(instruction_state.connect.insTable_0f[opcode]));
    }
    else if (x86_core_cpu_profile_has_8086_semantics(context->cpu_profile))
        POP_CS(context);
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void ADC_RM8_R8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("ADC_RM8_R8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 1, LIB_FALSE));
    CPU_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    CPU_TRACE_CALL_END;
}
static void ADC_RM32_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("ADC_RM32_R32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
        CPU_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 2, LIB_FALSE));
        CPU_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    CPU_TRACE_CALL_END;
}
static void ADC_R8_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("ADC_R8_RM8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), 1));
    CPU_TRACE_CALL_END;
}
static void ADC_R32_RM32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("ADC_R32_RM32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        CPU_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), 2));
    }
    CPU_TRACE_CALL_END;
}
static void ADC_AL_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("ADC_AL_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_adc(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    _WriteGPR8(al) = X86_CPU_MASK_U8(instruction_state.data.result);
    CPU_TRACE_CALL_END;
}
static void ADC_EAX_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("ADC_EAX_I32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
            CPU_TRACE_CHECK_RETURN(_a_adc(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 4));
            CPU_TRACE_CHECK_RETURN(_a_adc(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            _WriteGPR32(eax) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        CPU_TRACE_CHECK_RETURN(_a_adc(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void PUSH_SS(x86_core_cpu_execution_context *context)
{
    lib_u32 xs_sel;
    CPU_TRACE_CALL_BEGIN("PUSH_SS");
    _adv;
    xs_sel = cpu_state.data.ss.selector;
    CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(xs_sel), _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void POP_SS(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("POP_SS");
    _adv;
    CPU_TRACE_CHECK_RETURN(_e_pop_sreg(context, &cpu_state.data.ss,
        _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void SBB_RM8_R8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SBB_RM8_R8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 1, LIB_FALSE));
    CPU_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    CPU_TRACE_CALL_END;
}
static void SBB_RM32_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SBB_RM32_R32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
        CPU_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 2, LIB_FALSE));
        CPU_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    CPU_TRACE_CALL_END;
}
static void SBB_R8_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SBB_R8_RM8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), 1));
    CPU_TRACE_CALL_END;
}
static void SBB_R32_RM32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SBB_R32_RM32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        CPU_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), 2));
    }
    CPU_TRACE_CALL_END;
}
static void SBB_AL_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SBB_AL_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_sbb(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    _WriteGPR8(al) = X86_CPU_MASK_U8(instruction_state.data.result);
    CPU_TRACE_CALL_END;
}
static void SBB_EAX_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SBB_EAX_I32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
            CPU_TRACE_CHECK_RETURN(_a_sbb(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 4));
            CPU_TRACE_CHECK_RETURN(_a_sbb(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            _WriteGPR32(eax) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        CPU_TRACE_CHECK_RETURN(_a_sbb(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void PUSH_DS(x86_core_cpu_execution_context *context)
{
    lib_u32 xs_sel;
    CPU_TRACE_CALL_BEGIN("PUSH_DS");
    _adv;
    xs_sel = cpu_state.data.ds.selector;
    CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(xs_sel), _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void POP_DS(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("POP_DS");
    _adv;
    CPU_TRACE_CHECK_RETURN(_e_pop_sreg(context, &cpu_state.data.ds,
        _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void AND_RM8_R8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("AND_RM8_R8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 1, LIB_FALSE));
    CPU_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    CPU_TRACE_CALL_END;
}
static void AND_RM32_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("AND_RM32_R32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
        CPU_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 2, LIB_FALSE));
        CPU_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    CPU_TRACE_CALL_END;
}
static void AND_R8_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("AND_R8_RM8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), 1));
    CPU_TRACE_CALL_END;
}
static void AND_R32_RM32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("AND_R32_RM32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        CPU_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), 2));
    }
    CPU_TRACE_CALL_END;
}
static void AND_AL_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("AND_AL_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_and(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    _WriteGPR8(al) = X86_CPU_MASK_U8(instruction_state.data.result);
    CPU_TRACE_CALL_END;
}
static void AND_EAX_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("AND_EAX_I32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
            CPU_TRACE_CHECK_RETURN(_a_and(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 4));
            CPU_TRACE_CHECK_RETURN(_a_and(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            _WriteGPR32(eax) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        CPU_TRACE_CHECK_RETURN(_a_and(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void PREFIX_ES(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PREFIX_ES");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.es;
        instruction_state.data.roverss = &cpu_state.data.es;
    }
    else
    {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.es;
        instruction_state.data.roverss = &cpu_state.data.es;
    }
    CPU_TRACE_CALL_END;
}
static void DAA(x86_core_cpu_execution_context *context)
{
    lib_u8 oldAL = cpu_state.data.al;
    CPU_TRACE_CALL_BEGIN("DAA");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
        _adv;
    else
        _adv;
    if (((cpu_state.data.al & 0x0f) > 0x09) || _GetEFLAGS_AF)
    {
        _WriteGPR8(al) += 0x06;
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                      _GetEFLAGS_CF || ((cpu_state.data.al < oldAL) || (cpu_state.data.al < 0x06)));
        _SetEFLAGS_AF;
    }
    else
        _ClrEFLAGS_AF;
    if (((cpu_state.data.al & 0xf0) > 0x90) || _GetEFLAGS_CF)
    {
        _WriteGPR8(al) += 0x60;
        _SetEFLAGS_CF;
    }
    else
        _ClrEFLAGS_CF;
    instruction_state.data.bit = 8;
    instruction_state.data.result = (lib_u32)cpu_state.data.al;
    CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, DAA_FLAG));
    instruction_state.data.udf |= VCPU_EFLAGS_OF;
    /* The AL destination still exists when neither decimal adjustment fires. */
    (void)_kma_gpr_destination(context, X86_CPU_REFERENCE_OF(cpu_state.data.al));
    CPU_TRACE_CALL_END;
}
static void SUB_RM8_R8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SUB_RM8_R8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 1, LIB_FALSE));
    CPU_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    CPU_TRACE_CALL_END;
}
static void SUB_RM32_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SUB_RM32_R32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
        CPU_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 2, LIB_FALSE));
        CPU_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    CPU_TRACE_CALL_END;
}
static void SUB_R8_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SUB_R8_RM8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), 1));
    CPU_TRACE_CALL_END;
}
static void SUB_R32_RM32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SUB_R32_RM32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        CPU_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), 2));
    }
    CPU_TRACE_CALL_END;
}
static void SUB_AL_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SUB_AL_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_sub(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    _WriteGPR8(al) = X86_CPU_MASK_U8(instruction_state.data.result);
    CPU_TRACE_CALL_END;
}
static void SUB_EAX_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SUB_EAX_I32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
            CPU_TRACE_CHECK_RETURN(_a_sub(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 4));
            CPU_TRACE_CHECK_RETURN(_a_sub(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            _WriteGPR32(eax) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        CPU_TRACE_CHECK_RETURN(_a_sub(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void PREFIX_CS(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PREFIX_CS");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.cs;
        instruction_state.data.roverss = &cpu_state.data.cs;
    }
    else
    {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.cs;
        instruction_state.data.roverss = &cpu_state.data.cs;
    }
    CPU_TRACE_CALL_END;
}
static void DAS(x86_core_cpu_execution_context *context)
{
    lib_u8 oldAL = cpu_state.data.al;
    const lib_bool oldCF = _GetEFLAGS_CF;
    CPU_TRACE_CALL_BEGIN("DAS");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
        _adv;
    else
        _adv;
    if (((cpu_state.data.al & 0x0f) > 0x09) || _GetEFLAGS_AF)
    {
        _WriteGPR8(al) -= 0x06;
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                      _GetEFLAGS_CF || (oldAL < 0x06));
        _SetEFLAGS_AF;
    }
    else
        _ClrEFLAGS_AF;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX ?
        (oldAL > 0x99u || oldCF) : (cpu_state.data.al > 0x9f) || _GetEFLAGS_CF)
    {
        _WriteGPR8(al) -= 0x60;
        _SetEFLAGS_CF;
    }
    else if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX)
        _ClrEFLAGS_CF;
    instruction_state.data.bit = 8;
    instruction_state.data.result = (lib_u32)cpu_state.data.al;
    CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, DAS_FLAG));
    instruction_state.data.udf |= VCPU_EFLAGS_OF;
    (void)_kma_gpr_destination(context, X86_CPU_REFERENCE_OF(cpu_state.data.al));
    CPU_TRACE_CALL_END;
}
static void XOR_RM8_R8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("XOR_RM8_R8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 1, LIB_FALSE));
    CPU_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    instruction_state.data.crm = instruction_state.data.result;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    CPU_TRACE_CALL_END;
}
static void XOR_RM32_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("XOR_RM32_R32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
        CPU_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 2, LIB_FALSE));
        CPU_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cr, 16));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    CPU_TRACE_CALL_END;
}
static void XOR_R8_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("XOR_R8_RM8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), 1));
    CPU_TRACE_CALL_END;
}
static void XOR_R32_RM32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("XOR_R32_RM32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        CPU_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.cr, instruction_state.data.crm, 16));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), 2));
    }
    CPU_TRACE_CALL_END;
}
static void XOR_AL_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("XOR_AL_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_xor(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    _WriteGPR8(al) = X86_CPU_MASK_U8(instruction_state.data.result);
    CPU_TRACE_CALL_END;
}
static void XOR_EAX_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("XOR_EAX_I32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperxorSize(2)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
            CPU_TRACE_CHECK_RETURN(_a_xor(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperxorSize(4)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 4));
            CPU_TRACE_CHECK_RETURN(_a_xor(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            _WriteGPR32(eax) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        CPU_TRACE_CHECK_RETURN(_a_xor(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
        _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void PREFIX_SS(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PREFIX_SS");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.ss;
        instruction_state.data.roverss = &cpu_state.data.ss;
    }
    else
    {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.ss;
        instruction_state.data.roverss = &cpu_state.data.ss;
    }
    CPU_TRACE_CALL_END;
}
static void AAA(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("AAA");
    _adv;
    if (((cpu_state.data.al & 0x0f) > 0x09) || _GetEFLAGS_AF)
    {
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
            _WriteGPR16(ax) += 0x0106u;
        else
        {
            _WriteGPR8(al) += 0x06;
            _WriteGPR8(ah) += 0x01;
        }
        _SetEFLAGS_AF;
        _SetEFLAGS_CF;
    }
    else
    {
        _ClrEFLAGS_AF;
        _ClrEFLAGS_CF;
    }
    _WriteGPR8(al) &= 0x0f;
    instruction_state.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF);
    CPU_TRACE_CALL_END;
}
static void CMP_RM8_R8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("CMP_RM8_R8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    CPU_TRACE_CALL_END;
}
static void CMP_RM32_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("CMP_RM32_R32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        CPU_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cr, 16));
    }
    CPU_TRACE_CALL_END;
}
static void CMP_R8_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("CMP_R8_RM8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.cr, instruction_state.data.crm, 8));
    CPU_TRACE_CALL_END;
}
static void CMP_R32_RM32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("CMP_R32_RM32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        CPU_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.cr, instruction_state.data.crm, 16));
    }
    CPU_TRACE_CALL_END;
}
static void CMP_AL_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("CMP_AL_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_cmp(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    CPU_TRACE_CALL_END;
}
static void CMP_EAX_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("CMP_EAX_I32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
            CPU_TRACE_CHECK_RETURN(_a_cmp(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 4));
            CPU_TRACE_CHECK_RETURN(_a_cmp(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        CPU_TRACE_CHECK_RETURN(_a_cmp(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
    }
    CPU_TRACE_CALL_END;
}
static void PREFIX_DS(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PREFIX_DS");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.ds;
        instruction_state.data.roverss = &cpu_state.data.ds;
    }
    else
    {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.ds;
        instruction_state.data.roverss = &cpu_state.data.ds;
    }
    CPU_TRACE_CALL_END;
}
static void AAS(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("AAS");
    _adv;
    if (((cpu_state.data.al & 0x0f) > 0x09) || _GetEFLAGS_AF)
    {
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
            _WriteGPR16(ax) -= 0x0106u;
        else
        {
            _WriteGPR8(al) -= 0x06;
            _WriteGPR8(ah) -= 0x01;
        }
        _SetEFLAGS_AF;
        _SetEFLAGS_CF;
    }
    else
    {
        _ClrEFLAGS_CF;
        _ClrEFLAGS_AF;
    }
    _WriteGPR8(al) &= 0x0f;
    instruction_state.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF);
    CPU_TRACE_CALL_END;
}
static void INC_EAX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INC_EAX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.ax, 16));
            _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.eax, 32));
            _WriteGPR32(eax) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.ax, 16));
        _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void INC_ECX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INC_ECX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.cx, 16));
            _WriteGPR16(cx) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.ecx, 32));
            _WriteGPR32(ecx) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.cx, 16));
        _WriteGPR16(cx) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void INC_EDX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INC_EDX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.dx, 16));
            _WriteGPR16(dx) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.edx, 32));
            _WriteGPR32(edx) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.dx, 16));
        _WriteGPR16(dx) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void INC_EBX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INC_EBX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.bx, 16));
            _WriteGPR16(bx) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.ebx, 32));
            _WriteGPR32(ebx) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.bx, 16));
        _WriteGPR16(bx) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void INC_ESP(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INC_ESP");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.sp, 16));
            _WriteGPR16(sp) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.esp, 32));
            _WriteGPR32(esp) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.sp, 16));
        _WriteGPR16(sp) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void INC_EBP(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INC_EBP");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.bp, 16));
            _WriteGPR16(bp) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.ebp, 32));
            _WriteGPR32(ebp) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.bp, 16));
        _WriteGPR16(bp) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void INC_ESI(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INC_ESI");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.si, 16));
            _WriteGPR16(si) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.esi, 32));
            _WriteGPR32(esi) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.si, 16));
        _WriteGPR16(si) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void INC_EDI(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INC_EDI");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.di, 16));
            _WriteGPR16(di) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.edi, 32));
            _WriteGPR32(edi) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_a_inc(context, cpu_state.data.di, 16));
        _WriteGPR16(di) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void DEC_EAX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("DEC_EAX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.ax, 16));
            _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.eax, 32));
            _WriteGPR32(eax) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.ax, 16));
        _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void DEC_ECX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("DEC_ECX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.cx, 16));
            _WriteGPR16(cx) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.ecx, 32));
            _WriteGPR32(ecx) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.cx, 16));
        _WriteGPR16(cx) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void DEC_EDX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("DEC_EDX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.dx, 16));
            _WriteGPR16(dx) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.edx, 32));
            _WriteGPR32(edx) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.dx, 16));
        _WriteGPR16(dx) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void DEC_EBX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("DEC_EBX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.bx, 16));
            _WriteGPR16(bx) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.ebx, 32));
            _WriteGPR32(ebx) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.bx, 16));
        _WriteGPR16(bx) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void DEC_ESP(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("DEC_ESP");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.sp, 16));
            _WriteGPR16(sp) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.esp, 32));
            _WriteGPR32(esp) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.sp, 16));
        _WriteGPR16(sp) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void DEC_EBP(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("DEC_EBP");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.bp, 16));
            _WriteGPR16(bp) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.ebp, 32));
            _WriteGPR32(ebp) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.bp, 16));
        _WriteGPR16(bp) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void DEC_ESI(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("DEC_ESI");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.si, 16));
            _WriteGPR16(si) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.esi, 32));
            _WriteGPR32(esi) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.si, 16));
        _WriteGPR16(si) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void DEC_EDI(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("DEC_EDI");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.di, 16));
            _WriteGPR16(di) = X86_CPU_MASK_U16(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.edi, 32));
            _WriteGPR32(edi) = X86_CPU_MASK_U32(instruction_state.data.result);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_a_dec(context, cpu_state.data.di, 16));
        _WriteGPR16(di) = X86_CPU_MASK_U16(instruction_state.data.result);
    }
    CPU_TRACE_CALL_END;
}
static void PUSH_EAX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PUSH_EAX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.ax), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.eax), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.ax), 2));
    }
    CPU_TRACE_CALL_END;
}
static void PUSH_ECX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PUSH_ECX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.cx), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.ecx), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.cx), 2));
    }
    CPU_TRACE_CALL_END;
}
static void PUSH_EDX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PUSH_EDX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.dx), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.edx), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.dx), 2));
    }
    CPU_TRACE_CALL_END;
}
static void PUSH_EBX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PUSH_EBX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.bx), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.ebx), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.bx), 2));
    }
    CPU_TRACE_CALL_END;
}
static void PUSH_ESP(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PUSH_ESP");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.sp), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.esp), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        /* 8086/8088 and 80186/80188 expose the decremented SP when SP is
         * the PUSH source.  The 80286 changes this one observable case to
         * push the pre-instruction value. */
        if (!cpu_profile_has_protected_architecture(context->cpu_profile))
        {
            lib_u16 value = cpu_state.data.sp - 2;

            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(value), 2));
        }
        else
            CPU_TRACE_CHECK_RETURN(_e_push(context,
                X86_CPU_REFERENCE_OF(cpu_state.data.sp), 2));
    }
    CPU_TRACE_CALL_END;
}
static void PUSH_EBP(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PUSH_EBP");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.bp), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.ebp), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.bp), 2));
    }
    CPU_TRACE_CALL_END;
}
static void PUSH_ESI(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PUSH_ESI");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.si), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.esi), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.si), 2));
    }
    CPU_TRACE_CALL_END;
}
static void PUSH_EDI(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PUSH_EDI");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.di), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.edi), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.di), 2));
    }
    CPU_TRACE_CALL_END;
}
static void POP_EAX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("POP_EAX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.ax), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.eax), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.ax), 2));
    }
    CPU_TRACE_CALL_END;
}
static void POP_ECX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("POP_ECX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.cx), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.ecx), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.cx), 2));
    }
    CPU_TRACE_CALL_END;
}
static void POP_EDX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("POP_EDX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.dx), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.edx), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.dx), 2));
    }
    CPU_TRACE_CALL_END;
}
static void POP_EBX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("POP_EBX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.bx), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.ebx), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.bx), 2));
    }
    CPU_TRACE_CALL_END;
}
static void POP_ESP(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("POP_ESP");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.sp), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.esp), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.sp), 2));
    }
    CPU_TRACE_CALL_END;
}
static void POP_EBP(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("POP_EBP");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.bp), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.ebp), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.bp), 2));
    }
    CPU_TRACE_CALL_END;
}
static void POP_ESI(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("POP_ESI");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.si), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.esi), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.si), 2));
    }
    CPU_TRACE_CALL_END;
}
static void POP_EDI(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("POP_EDI");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.di), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.edi), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.di), 2));
    }
    CPU_TRACE_CALL_END;
}
static void PUSHA(x86_core_cpu_execution_context *context)
{
    lib_u32 cesp;
    CPU_TRACE_CALL_BEGIN("PUSHA");
    if (cpu_profile_has_186_integer(context->cpu_profile))
    {
        _adv;
        if (cpu_profile_has_protected_architecture(context->cpu_profile) &&
            !_IsProtected)
        {
            cesp = _GetStackSize == 2 ? cpu_state.data.sp : cpu_state.data.esp;
            if (cesp == 1u || cesp == 3u || cesp == 5u)
                CPU_TRACE_CHECK_RETURN(x86_core_cpu_execution_raise_exception(
                    context, VCPUINS_EXCEPT_SHUTDOWN, 0u));
            if (cesp >= 7u && cesp <= 15u && (cesp & 1u))
                CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        }
        CPU_TRACE_CHECK_RETURN(_s_test_ss_frame(context, _GetOperandSize,
            8u, 0u, LIB_TRUE));
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            cesp = cpu_state.data.sp;
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.ax), 2));
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.cx), 2));
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.dx), 2));
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.bx), 2));
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cesp), 2));
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.bp), 2));
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.si), 2));
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.di), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            cesp = cpu_state.data.esp;
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.eax), 4));
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.ecx), 4));
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.edx), 4));
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.ebx), 4));
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cesp), 4));
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.ebp), 4));
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.esi), 4));
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.edi), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void POPA(x86_core_cpu_execution_context *context)
{
    lib_u32 cesp;
    CPU_TRACE_CALL_BEGIN("POPA");
    if (cpu_profile_has_186_integer(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_s_test_ss_frame(context, _GetOperandSize,
            8u, 0u, LIB_FALSE));
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.di), 2));
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.si), 2));
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.bp), 2));
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cesp), 2));
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.bx), 2));
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.dx), 2));
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.cx), 2));
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.ax), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.edi), 4));
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.esi), 4));
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.ebp), 4));
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cesp), 4));
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.ebx), 4));
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.edx), 4));
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.ecx), 4));
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.eax), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void BOUND_R16_M16_16(x86_core_cpu_execution_context *context)
{
    lib_i16 a16, l16, u16;
    lib_i32 a32, l32, u32;
    lib_u8 modrm;
    CPU_TRACE_CALL_BEGIN("BOUND_R16_M16_16");
    if (cpu_profile_has_186_integer(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip,
            X86_CPU_REFERENCE_OF(modrm), 1));
        if (_GetModRM_MOD(modrm) == 3)
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize * 2));
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            a16 = (lib_i16)instruction_state.data.cr;
            CPU_TRACE_CHECK_RETURN(_m_read_pair(context, 2, 2));
            l16 = (lib_i16)instruction_state.data.crm;
            u16 = (lib_i16)(instruction_state.data.crm >> 16u);
            if (a16 < l16 || a16 > u16)
                CPU_TRACE_CHECK_RETURN(_SetExcept_BR(0));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            a32 = (lib_i32)instruction_state.data.cr;
            CPU_TRACE_CHECK_RETURN(_m_read_pair(context, 4, 4));
            l32 = (lib_i32)instruction_state.data.crm;
            u32 = (lib_i32)(instruction_state.data.crm >> 32u);
            if (a32 < l32 || a32 > u32)
                CPU_TRACE_CHECK_RETURN(_SetExcept_BR(0));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void ARPL_RM16_R16(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("ARPL_RM16_R16");
    if (cpu_profile_has_protected_architecture(context->cpu_profile))
    {
        if (_IsProtected)
        {
            _adv;
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
            CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 2, LIB_FALSE));
            instruction_state.data.cr = X86_CPU_MASK_U16(instruction_state.data.cr);
            if (_GetSelector_RPL(instruction_state.data.crm) < _GetSelector_RPL(instruction_state.data.cr))
            {
                _SetEFLAGS_ZF;
                instruction_state.data.result = X86_CPU_MASK_U16((instruction_state.data.crm & ~VCPU_SELECTOR_RPL) |
                                                                      (instruction_state.data.cr & VCPU_SELECTOR_RPL));
                instruction_state.data.crm = instruction_state.data.result;
                CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            }
            else
                _ClrEFLAGS_ZF;
        }
        else
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void PREFIX_FS(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PREFIX_FS");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.fs;
        instruction_state.data.roverss = &cpu_state.data.fs;
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void PREFIX_GS(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PREFIX_GS");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        instruction_state.data.roverds = &cpu_state.data.gs;
        instruction_state.data.roverss = &cpu_state.data.gs;
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void PREFIX_OprSize(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PREFIX_OprSize");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        instruction_state.data.prefix_oprsize = LIB_TRUE;
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void PREFIX_AddrSize(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PREFIX_AddrSize");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        instruction_state.data.prefix_addrsize = LIB_TRUE;
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void PUSH_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PUSH_I32");
    if (cpu_profile_has_186_integer(context->cpu_profile))
    {
        _adv;
        if (instruction_state.data.flagLock)
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(instruction_state.data.cimm), _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void IMUL_R32_RM32_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("IMUL_R32_RM32_I32");
    if (cpu_profile_has_186_integer(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_a_imul3(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void PUSH_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PUSH_I8");
    if (cpu_profile_has_186_integer(context->cpu_profile))
    {
        _adv;
        if (instruction_state.data.flagLock)
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
        instruction_state.data.cimm = (lib_i8)instruction_state.data.cimm;
        CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(instruction_state.data.cimm), _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void IMUL_R32_RM32_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("IMUL_R32_RM32_I8");
    if (cpu_profile_has_186_integer(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
        CPU_TRACE_CHECK_RETURN(_a_imul3(context, instruction_state.data.crm, instruction_state.data.cimm, ((_GetOperandSize * 8 + 8) >> 1)));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void INSB(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INSB");
    if (cpu_profile_has_186_integer(context->cpu_profile))
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            CPU_TRACE_CHECK_RETURN(_p_ins(context, 1));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    CPU_TRACE_BLOCK_BEGIN("cx(!0)");
                    CPU_TRACE_CHECK_RETURN(_p_ins(context, 1));
                    _WriteGPR16(cx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            case 4:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    CPU_TRACE_BLOCK_BEGIN("ecx(!0)");
                    CPU_TRACE_CHECK_RETURN(_p_ins(context, 1));
                    _WriteGPR32(ecx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
        }
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void INSW(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INSW");
    if (cpu_profile_has_186_integer(context->cpu_profile))
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            CPU_TRACE_CHECK_RETURN(_p_ins(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    CPU_TRACE_BLOCK_BEGIN("cx(!0)");
                    CPU_TRACE_CHECK_RETURN(_p_ins(context, _GetOperandSize));
                    _WriteGPR16(cx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            case 4:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    CPU_TRACE_BLOCK_BEGIN("ecx(!0)");
                    CPU_TRACE_CHECK_RETURN(_p_ins(context, _GetOperandSize));
                    _WriteGPR32(ecx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
        }
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void OUTSB(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("OUTSB");
    if (cpu_profile_has_186_integer(context->cpu_profile))
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            CPU_TRACE_CHECK_RETURN(_p_outs(context, 1));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    CPU_TRACE_BLOCK_BEGIN("cx(!0)");
                    CPU_TRACE_CHECK_RETURN(_p_outs(context, 1));
                    _WriteGPR16(cx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            case 4:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    CPU_TRACE_BLOCK_BEGIN("ecx(!0)");
                    CPU_TRACE_CHECK_RETURN(_p_outs(context, 1));
                    _WriteGPR32(ecx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
        }
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void OUTSW(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("OUTSW");
    if (cpu_profile_has_186_integer(context->cpu_profile))
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            CPU_TRACE_CHECK_RETURN(_p_outs(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    CPU_TRACE_BLOCK_BEGIN("cx(!0)");
                    CPU_TRACE_CHECK_RETURN(_p_outs(context, _GetOperandSize));
                    _WriteGPR16(cx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            case 4:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    CPU_TRACE_BLOCK_BEGIN("ecx(!0)");
                    CPU_TRACE_CHECK_RETURN(_p_outs(context, _GetOperandSize));
                    _WriteGPR32(ecx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
        }
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void JO_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JO_REL8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1, _GetEFLAGS_OF));
    CPU_TRACE_CALL_END;
}
static void JNO_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JNO_REL8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1, !_GetEFLAGS_OF));
    CPU_TRACE_CALL_END;
}
static void JC_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JC_REL8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1, _GetEFLAGS_CF));
    CPU_TRACE_CALL_END;
}
static void JNC_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JNC_REL8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1, !_GetEFLAGS_CF));
    CPU_TRACE_CALL_END;
}
static void JZ_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JZ_REL8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1, _GetEFLAGS_ZF));
    CPU_TRACE_CALL_END;
}
static void JNZ_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JNZ_REL8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1, !_GetEFLAGS_ZF));
    CPU_TRACE_CALL_END;
}
static void JNA_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JNA_REL8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1,
                                   (_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    CPU_TRACE_CALL_END;
}
static void JA_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JA_REL8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1,
                                   !(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    CPU_TRACE_CALL_END;
}
static void JS_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JS_REL8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1, _GetEFLAGS_SF));
    CPU_TRACE_CALL_END;
}
static void JNS_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JNS_REL8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1, !_GetEFLAGS_SF));
    CPU_TRACE_CALL_END;
}
static void JP_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JP_REL8");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1, _GetEFLAGS_PF));
    CPU_TRACE_CALL_END;
}
static void JNP_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JNP_REL8");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1, !_GetEFLAGS_PF));
    CPU_TRACE_CALL_END;
}
static void JL_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JL_REL8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1, (_GetEFLAGS_SF != _GetEFLAGS_OF)));
    CPU_TRACE_CALL_END;
}
static void JNL_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JNL_REL8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1, (_GetEFLAGS_SF == _GetEFLAGS_OF)));
    CPU_TRACE_CALL_END;
}
static void JNG_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JNG_REL8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1,
                                   (_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF))));
    CPU_TRACE_CALL_END;
}
static void JG_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JG_REL8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1,
                                   (!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF))));
    CPU_TRACE_CALL_END;
}
static void INS_80(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INS_80");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    if (instruction_state.data.cr != 7u)
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 1, LIB_FALSE));
    else
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    switch (instruction_state.data.cr)
    {
    case 0: /* ADD_RM8_I8 */
        CPU_TRACE_BLOCK_BEGIN("ADD_RM8_I8");
        CPU_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 1: /* OR_RM8_I8 */
        CPU_TRACE_BLOCK_BEGIN("OR_RM8_I8");
        CPU_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 2: /* ADC_RM8_I8 */
        CPU_TRACE_BLOCK_BEGIN("ADC_RM8_I8");
        CPU_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 3: /* SBB_RM8_I8 */
        CPU_TRACE_BLOCK_BEGIN("SBB_RM8_I8");
        _new_code_path_;
        CPU_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 4: /* AND_RM8_I8 */
        CPU_TRACE_BLOCK_BEGIN("AND_RM8_I8");
        CPU_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 5: /* SUB_RM8_I8 */
        CPU_TRACE_BLOCK_BEGIN("SUB_RM8_I8");
        CPU_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 6: /* XOR_RM8_I8 */
        CPU_TRACE_BLOCK_BEGIN("XOR_RM8_I8");
        CPU_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 7: /* CMP_RM8_I8 */
        CPU_TRACE_BLOCK_BEGIN("CMP_RM8_I8");
        CPU_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void INS_81(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INS_81");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        if (instruction_state.data.cr != 7u)
            CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
        else
            CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        switch (instruction_state.data.cr)
        {
        case 0: /* ADD_RM32_I32 */
            CPU_TRACE_BLOCK_BEGIN("ADD_RM32_I32");
            CPU_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 1: /* OR_RM32_I32 */
            CPU_TRACE_BLOCK_BEGIN("OR_RM32_I32");
            CPU_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 2: /* ADC_RM32_I32 */
            CPU_TRACE_BLOCK_BEGIN("ADC_RM32_I32");
            CPU_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 3: /* SBB_RM32_I32 */
            CPU_TRACE_BLOCK_BEGIN("SBB_RM32_I32");
            _new_code_path_;
            CPU_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 4: /* AND_RM32_I32 */
            CPU_TRACE_BLOCK_BEGIN("AND_RM32_I32");
            CPU_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 5: /* SUB_RM32_I32 */
            CPU_TRACE_BLOCK_BEGIN("SUB_RM32_I32");
            CPU_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 6: /* XOR_RM32_I32 */
            CPU_TRACE_BLOCK_BEGIN("XOR_RM32_I32");
            CPU_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 7: /* CMP_RM32_I32 */
            CPU_TRACE_BLOCK_BEGIN("CMP_RM32_I32");
            CPU_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        if (instruction_state.data.cr != 7u)
            CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 2, LIB_FALSE));
        else
            CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        switch (instruction_state.data.cr)
        {
        case 0: /* ADD_RM16_I16 */
            CPU_TRACE_BLOCK_BEGIN("ADD_RM16_I16");
            CPU_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 1: /* OR_RM16_I16 */
            CPU_TRACE_BLOCK_BEGIN("OR_RM16_I16");
            CPU_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 2: /* ADC_RM16_I16 */
            CPU_TRACE_BLOCK_BEGIN("ADC_RM16_I16");
            CPU_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 3: /* SBB_RM16_I16 */
            CPU_TRACE_BLOCK_BEGIN("SBB_RM16_I16");
            _new_code_path_;
            CPU_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4: /* AND_RM16_I16 */
            CPU_TRACE_BLOCK_BEGIN("AND_RM16_I16");
            CPU_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 5: /* SUB_RM16_I16 */
            CPU_TRACE_BLOCK_BEGIN("SUB_RM16_I16");
            CPU_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 6: /* XOR_RM16_I16 */
            CPU_TRACE_BLOCK_BEGIN("XOR_RM16_I16");
            CPU_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 7: /* CMP_RM16_I16 */
            CPU_TRACE_BLOCK_BEGIN("CMP_RM16_I16");
            CPU_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, 2 * 8));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    CPU_TRACE_CALL_END;
}
static void INS_83(x86_core_cpu_execution_context *context)
{
    lib_u8 bit;
    CPU_TRACE_CALL_BEGIN("INS_83");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
        if (instruction_state.data.cr != 7u)
            CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
        else
            CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        bit = (_GetOperandSize * 8 + 8) >> 1;
        switch (instruction_state.data.cr)
        {
        case 0: /* ADD_RM32_I8 */
            CPU_TRACE_BLOCK_BEGIN("ADD_RM32_I8");
            CPU_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 1: /* OR_RM32_I8 */
            CPU_TRACE_BLOCK_BEGIN("OR_RM32_I8");
            CPU_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 2: /* ADC_RM32_I8 */
            CPU_TRACE_BLOCK_BEGIN("ADC_RM32_I8");
            CPU_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 3: /* SBB_RM32_I8 */
            CPU_TRACE_BLOCK_BEGIN("SBB_RM32_I8");
            CPU_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 4: /* AND_RM32_I8 */
            CPU_TRACE_BLOCK_BEGIN("AND_RM32_I8");
            CPU_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 5: /* SUB_RM32_I8 */
            CPU_TRACE_BLOCK_BEGIN("SUB_RM32_I8");
            CPU_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 6: /* XOR_RM32_I8 */
            CPU_TRACE_BLOCK_BEGIN("XOR_RM32_I8");
            CPU_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 7: /* CMP_RM32_I8 */
            CPU_TRACE_BLOCK_BEGIN("CMP_RM32_I8");
            CPU_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
        if (instruction_state.data.cr != 7u)
            CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 2, LIB_FALSE));
        else
            CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        bit = 12;
        switch (instruction_state.data.cr)
        {
        case 0: /* ADD_RM16_I8 */
            CPU_TRACE_BLOCK_BEGIN("ADD_RM16_I8");
            CPU_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 1: /* OR_RM16_I8 */
            CPU_TRACE_BLOCK_BEGIN("OR_RM16_I8");
            CPU_TRACE_CHECK_RETURN(_a_or(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 2: /* ADC_RM16_I8 */
            CPU_TRACE_BLOCK_BEGIN("ADC_RM16_I8");
            CPU_TRACE_CHECK_RETURN(_a_adc(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 3: /* SBB_RM16_I8 */
            CPU_TRACE_BLOCK_BEGIN("SBB_RM16_I8");
            CPU_TRACE_CHECK_RETURN(_a_sbb(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4: /* AND_RM16_I8 */
            CPU_TRACE_BLOCK_BEGIN("AND_RM16_I8");
            CPU_TRACE_CHECK_RETURN(_a_and(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 5: /* SUB_RM16_I8 */
            CPU_TRACE_BLOCK_BEGIN("SUB_RM16_I8");
            CPU_TRACE_CHECK_RETURN(_a_sub(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 6: /* XOR_RM16_I8 */
            CPU_TRACE_BLOCK_BEGIN("XOR_RM16_I8");
            CPU_TRACE_CHECK_RETURN(_a_xor(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 7: /* CMP_RM16_I8 */
            CPU_TRACE_BLOCK_BEGIN("CMP_RM16_I8");
            CPU_TRACE_CHECK_RETURN(_a_cmp(context, instruction_state.data.crm, instruction_state.data.cimm, bit));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    CPU_TRACE_CALL_END;
}
static void TEST_RM8_R8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("TEST_RM8_R8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_test(context, instruction_state.data.crm, instruction_state.data.cr, 8));
    CPU_TRACE_CALL_END;
}
static void TEST_RM32_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("TEST_RM32_R32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_a_test(context, instruction_state.data.crm, instruction_state.data.cr, _GetOperandSize * 8));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        _a_test(context, instruction_state.data.crm, instruction_state.data.cr, 16);
    }
    CPU_TRACE_CALL_END;
}
static void XCHG_RM8_R8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("XCHG_RM8_R8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 1, LIB_TRUE));
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.crm), 1));
    instruction_state.data.crm = instruction_state.data.cr;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    CPU_TRACE_CALL_END;
}
static void XCHG_RM32_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("XCHG_RM32_R32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_TRUE));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
        instruction_state.data.crm = instruction_state.data.cr;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 2, LIB_TRUE));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.crm), 2));
        instruction_state.data.crm = instruction_state.data.cr;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    CPU_TRACE_CALL_END;
}
static void MOV_RM8_R8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_RM8_R8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    instruction_state.data.crm = instruction_state.data.cr;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    CPU_TRACE_CALL_END;
}
static void MOV_RM32_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_RM32_R32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        instruction_state.data.crm = instruction_state.data.cr;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        instruction_state.data.crm = instruction_state.data.cr;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    CPU_TRACE_CALL_END;
}
static void MOV_R8_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_R8_RM8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.crm), 1));
    CPU_TRACE_CALL_END;
}
static void MOV_R32_RM32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_R32_RM32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.crm), 2));
    }
    CPU_TRACE_CALL_END;
}
static void MOV_RM16_SREG(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_RM16_SREG");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm_sreg(context, 2));
    instruction_state.data.crm = instruction_state.data.rmovsreg->selector;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    CPU_TRACE_CALL_END;
}
static void LEA_R32_M32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("LEA_R32_M32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm_ea(context, _GetOperandSize, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.mrm.offset), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.mrm.offset), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm_ea(context, 2, 2));
        _kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.mrm.offset), 2);
    }
    CPU_TRACE_CALL_END;
}
static void MOV_SREG_RM16(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_SREG_RM16");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm_sreg(context, 2));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm_sreg(context, 2));
    }
    if (instruction_state.data.rmovsreg->sregtype == SREG_CODE)
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
    CPU_TRACE_CHECK_RETURN(_s_load_sreg(context, instruction_state.data.rmovsreg,
        X86_CPU_MASK_U16(instruction_state.data.crm)));
    if (instruction_state.data.rmovsreg->sregtype == SREG_STACK ||
        !cpu_profile_has_186_integer(context->cpu_profile))
        context->interrupt_shadow = CPU_INTERRUPT_SHADOW_SEGMENT;
    CPU_TRACE_CALL_END;
}
static void INS_8F(x86_core_cpu_execution_context *context)
{
    lib_u8 modrm;
    lib_u32 value;

    CPU_TRACE_CALL_BEGIN("INS_8F");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip,
            X86_CPU_REFERENCE_OF(modrm), 1));
        if (instruction_state.data.flagLock)
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        switch (_GetModRM_REG(modrm))
        {
        case 0: /* POP_RM32 */
            CPU_TRACE_BLOCK_BEGIN("POP_RM32");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context,
                X86_CPU_REFERENCE_OF(value), _GetOperandSize));
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            instruction_state.data.crm = value;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 1:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 2:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 3:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 5:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 6:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 7:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.ip,
            X86_CPU_REFERENCE_OF(modrm), 1));
        if (instruction_state.data.flagLock)
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        switch (_GetModRM_REG(modrm))
        {
        case 0: /* POP_RM16 */
            CPU_TRACE_BLOCK_BEGIN("POP_RM16");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context,
                X86_CPU_REFERENCE_OF(value), 2));
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            instruction_state.data.crm = value;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 1:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 2:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 3:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 5:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 6:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 7:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    CPU_TRACE_CALL_END;
}
static void NOP(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("NOP");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
    }
    else
        _adv;
    CPU_TRACE_CALL_END;
}
static void XCHG_ECX_EAX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("XCHG_ECX_EAX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            _WriteGPR16(ax) = cpu_state.data.cx;
            _WriteGPR16(cx) = X86_CPU_MASK_U16(instruction_state.data.cr);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            _WriteGPR32(eax) = cpu_state.data.ecx;
            _WriteGPR32(ecx) = X86_CPU_MASK_U32(instruction_state.data.cr);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        instruction_state.data.cr = cpu_state.data.ax;
        _WriteGPR16(ax) = cpu_state.data.cx;
        _WriteGPR16(cx) = X86_CPU_MASK_U16(instruction_state.data.cr);
    }
    CPU_TRACE_CALL_END;
}
static void XCHG_EDX_EAX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("XCHG_EDX_EAX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            _WriteGPR16(ax) = cpu_state.data.dx;
            _WriteGPR16(dx) = X86_CPU_MASK_U16(instruction_state.data.cr);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            _WriteGPR32(eax) = cpu_state.data.edx;
            _WriteGPR32(edx) = X86_CPU_MASK_U32(instruction_state.data.cr);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        instruction_state.data.cr = cpu_state.data.ax;
        _WriteGPR16(ax) = cpu_state.data.dx;
        _WriteGPR16(dx) = X86_CPU_MASK_U16(instruction_state.data.cr);
    }
    CPU_TRACE_CALL_END;
}
static void XCHG_EBX_EAX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("XCHG_EBX_EAX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            _WriteGPR16(ax) = cpu_state.data.bx;
            _WriteGPR16(bx) = X86_CPU_MASK_U16(instruction_state.data.cr);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            _WriteGPR32(eax) = cpu_state.data.ebx;
            _WriteGPR32(ebx) = X86_CPU_MASK_U32(instruction_state.data.cr);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        instruction_state.data.cr = cpu_state.data.ax;
        _WriteGPR16(ax) = cpu_state.data.bx;
        _WriteGPR16(bx) = X86_CPU_MASK_U16(instruction_state.data.cr);
    }
    CPU_TRACE_CALL_END;
}
static void XCHG_ESP_EAX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("XCHG_ESP_EAX");
    _new_code_path_;
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            _WriteGPR16(ax) = cpu_state.data.sp;
            _WriteGPR16(sp) = X86_CPU_MASK_U16(instruction_state.data.cr);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            _WriteGPR32(eax) = cpu_state.data.esp;
            _WriteGPR32(esp) = X86_CPU_MASK_U32(instruction_state.data.cr);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        instruction_state.data.cr = cpu_state.data.ax;
        _WriteGPR16(ax) = cpu_state.data.sp;
        _WriteGPR16(sp) = X86_CPU_MASK_U16(instruction_state.data.cr);
    }
    CPU_TRACE_CALL_END;
}
static void XCHG_EBP_EAX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("XCHG_EBP_EAX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            _WriteGPR16(ax) = cpu_state.data.bp;
            _WriteGPR16(bp) = X86_CPU_MASK_U16(instruction_state.data.cr);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            _WriteGPR32(eax) = cpu_state.data.ebp;
            _WriteGPR32(ebp) = X86_CPU_MASK_U32(instruction_state.data.cr);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        instruction_state.data.cr = cpu_state.data.ax;
        _WriteGPR16(ax) = cpu_state.data.bp;
        _WriteGPR16(bp) = X86_CPU_MASK_U16(instruction_state.data.cr);
    }
    CPU_TRACE_CALL_END;
}
static void XCHG_ESI_EAX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("XCHG_ESI_EAX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            _WriteGPR16(ax) = cpu_state.data.si;
            _WriteGPR16(si) = X86_CPU_MASK_U16(instruction_state.data.cr);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            _WriteGPR32(eax) = cpu_state.data.esi;
            _WriteGPR32(esi) = X86_CPU_MASK_U32(instruction_state.data.cr);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        instruction_state.data.cr = cpu_state.data.ax;
        _WriteGPR16(ax) = cpu_state.data.si;
        _WriteGPR16(si) = X86_CPU_MASK_U16(instruction_state.data.cr);
    }
    CPU_TRACE_CALL_END;
}
static void XCHG_EDI_EAX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("XCHG_EDI_EAX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            instruction_state.data.cr = cpu_state.data.ax;
            _WriteGPR16(ax) = cpu_state.data.di;
            _WriteGPR16(di) = X86_CPU_MASK_U16(instruction_state.data.cr);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            instruction_state.data.cr = cpu_state.data.eax;
            _WriteGPR32(eax) = cpu_state.data.edi;
            _WriteGPR32(edi) = X86_CPU_MASK_U32(instruction_state.data.cr);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        instruction_state.data.cr = cpu_state.data.ax;
        _WriteGPR16(ax) = cpu_state.data.di;
        _WriteGPR16(di) = X86_CPU_MASK_U16(instruction_state.data.cr);
    }
    CPU_TRACE_CALL_END;
}
static void CBW(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("CBW");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            _WriteGPR16(ax) = (lib_i8)cpu_state.data.al;
            break;
        case 4:
            _WriteGPR32(eax) = (lib_i16)cpu_state.data.ax;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        _WriteGPR16(ax) = (lib_i8)cpu_state.data.al;
    }
    CPU_TRACE_CALL_END;
}
static void CWD(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("CWD");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            _WriteGPR16(dx) = X86_CPU_GET_MSB_16(cpu_state.data.ax) ? 0xffff : 0x0000;
            break;
        case 4:
            _WriteGPR32(edx) = X86_CPU_GET_MSB_32(cpu_state.data.eax) ? 0xffffffff : 0x00000000;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        if (cpu_state.data.ax & 0x8000)
            _WriteGPR16(dx) = 0xffff;
        else
            _WriteGPR16(dx) = 0x0000;
    }
    CPU_TRACE_CALL_END;
}
static void CALL_PTR16_32(x86_core_cpu_execution_context *context)
{
    lib_u16 newcs;
    lib_u32 neweip;
    CPU_TRACE_CALL_BEGIN("CALL_PTR16_32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 4));
            neweip = X86_CPU_MASK_U16(instruction_state.data.cimm);
            newcs = X86_CPU_MASK_U16(instruction_state.data.cimm >> 16);
            CPU_TRACE_CHECK_RETURN(_e_call_far(context, newcs, neweip, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 6));
            neweip = X86_CPU_MASK_U32(instruction_state.data.cimm);
            newcs = X86_CPU_MASK_U16(instruction_state.data.cimm >> 32);
            CPU_TRACE_CHECK_RETURN(_e_call_far(context, newcs, neweip, 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 4));
        neweip = X86_CPU_MASK_U16(instruction_state.data.cimm);
        newcs = X86_CPU_MASK_U16(instruction_state.data.cimm >> 16);
        _e_call_far(context, newcs, neweip, 2);
    }
    CPU_TRACE_CALL_END;
}
static void WAIT(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("WAIT");
    _new_code_path_;
    _adv;
    if (_GetCR0_TS && _GetCR0_MP)
    {
        CPU_TRACE_BLOCK_BEGIN("CR0_TS_AND_MP(1)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_NM(0));
        CPU_TRACE_BLOCK_END;
    }
    else if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX)
    {
        if (x86_fpu_wait_pending(context->fpu)) {
            CPU_TRACE_BLOCK_BEGIN("FPU_PENDING(1)");
            CPU_TRACE_CHECK_RETURN(_SetExcept_MF(0));
            CPU_TRACE_BLOCK_END;
        }
        else if (x86_core_cpu_wait_test_asserted(context))
            x86_core_cpu_begin_wait(context, CPU_WAIT_STATE_TEST);
    }
    CPU_TRACE_CALL_END;
}
static void PUSHF(x86_core_cpu_execution_context *context)
{
    lib_u32 ceflags;
    CPU_TRACE_CALL_BEGIN("PUSHF");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        if (_e_vm86_interrupts_virtualized(context) && _GetOperandSize == 2u) {
            ceflags = _e_real_flags_image_16(context, cpu_state.data.flags);
            ceflags = (ceflags & ~(VCPU_EFLAGS_IF | VCPU_EFLAGS_IOPL)) |
                VCPU_EFLAGS_IOPL | (_GetEFLAGS_VIF ? VCPU_EFLAGS_IF : 0u);
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(ceflags), 2u));
            CPU_TRACE_CALL_END;
            return;
        }
        if (!_GetCR0_PE || (_GetCR0_PE && (!_GetEFLAGS_VM || (_GetEFLAGS_VM && (_GetEFLAGS_IOPL == 3)))))
        {
            CPU_TRACE_BLOCK_BEGIN("Real/Protected/(V86,IOPL(3))");
            switch (_GetOperandSize)
            {
            case 2:
                CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
                ceflags = _e_real_flags_image_16(context, cpu_state.data.flags);
                CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(ceflags), 2));
                CPU_TRACE_BLOCK_END;
                break;
            case 4:
                CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
                ceflags = (cpu_state.data.eflags & ~(VCPU_EFLAGS_VM | VCPU_EFLAGS_RF |
                    x86_core_cpu_eflags_reserved_mask(context->cpu_profile))) | 0x02u;
                CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(ceflags), 4));
                CPU_TRACE_BLOCK_END;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("EFLAGS_VM(1),EFLAGS_IOPL(!3)");
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            CPU_TRACE_BLOCK_END;
        }
    }
    else
    {
        _adv;
        ceflags = _e_real_flags_image_16(context, cpu_state.data.flags);
        CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(ceflags), 2));
    }
    CPU_TRACE_CALL_END;
}
static void POPF(x86_core_cpu_execution_context *context)
{
    lib_u32 mask = x86_core_cpu_eflags_reserved_mask(context->cpu_profile) | VCPU_EFLAGS_RF;
    lib_u32 ceflags = 0u;
    CPU_TRACE_CALL_BEGIN("POPF");
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
        mask |= VCPU_EFLAGS_VM | VCPU_EFLAGS_VIF | VCPU_EFLAGS_VIP;
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        if (!_GetCR0_PE || !_GetEFLAGS_VM)
        {
            CPU_TRACE_BLOCK_BEGIN("!V86");
            if (!_GetCPL)
            {
                CPU_TRACE_BLOCK_BEGIN("CPL(0)");
                switch (_GetOperandSize)
                {
                case 2:
                    CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
                    CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(ceflags), 2));
                    mask |= 0xffff0000;
                    CPU_TRACE_BLOCK_END;
                    break;
                case 4:
                    CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
                    CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(ceflags), 4));
                    mask |= VCPU_EFLAGS_VM;
                    CPU_TRACE_BLOCK_END;
                    break;
                default:
                    CPU_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                CPU_TRACE_BLOCK_END;
            }
            else
            {
                CPU_TRACE_BLOCK_BEGIN("CPL(!0)");
                _new_code_path_;
                switch (_GetOperandSize)
                {
                case 2:
                    CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
                    CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(ceflags), 2));
                    mask |= (0xffff0000 | VCPU_EFLAGS_IOPL);
                    CPU_TRACE_BLOCK_END;
                    break;
                case 4:
                    CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
                    CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(ceflags), 4));
                    mask |= (VCPU_EFLAGS_VM | VCPU_EFLAGS_RF | VCPU_EFLAGS_IOPL);
                    CPU_TRACE_BLOCK_END;
                    break;
                default:
                    CPU_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                if (_GetCPL > _GetEFLAGS_IOPL)
                    mask |= VCPU_EFLAGS_IF;
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("V86");
            if (_GetEFLAGS_IOPL == 0x03)
            {
                CPU_TRACE_BLOCK_BEGIN("EFLAGS_IOPL(3)");
                switch (_GetOperandSize)
                {
                case 2:
                    CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
                    CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(ceflags), 2));
                    mask |= (0xffff0000 | VCPU_EFLAGS_IOPL);
                    CPU_TRACE_BLOCK_END;
                    break;
                case 4:
                    CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
                    CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(ceflags), 4));
                    mask |= (VCPU_EFLAGS_VM | VCPU_EFLAGS_RF | VCPU_EFLAGS_IOPL);
                    CPU_TRACE_BLOCK_END;
                    break;
                default:
                    CPU_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                CPU_TRACE_BLOCK_END;
            }
            else
            {
                CPU_TRACE_BLOCK_BEGIN("EFLAGS_IOPL(!3)");
                if (!_e_vm86_interrupts_virtualized(context) || _GetOperandSize != 2u)
                    CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                CPU_TRACE_CHECK_RETURN(_s_peek_ss_pop(context, 0u,
                    X86_CPU_REFERENCE_OF(ceflags), 2u));
                if ((ceflags & VCPU_EFLAGS_TF) ||
                    (_GetEFLAGS_VIP && (ceflags & VCPU_EFLAGS_IF)))
                    CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                mask |= 0xffff0000u | VCPU_EFLAGS_IOPL | VCPU_EFLAGS_IF;
                if (_GetStackSize == 2u) _WriteGPR16(sp) += 2u;
                else _WriteGPR32(esp) += 2u;
                X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_VIF,
                    (ceflags & VCPU_EFLAGS_IF) != 0u);
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_BLOCK_END;
        }
        cpu_state.data.eflags = _e_eflags_load(context,
            (ceflags & ~mask) | (cpu_state.data.eflags & mask));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(ceflags), 2));
        mask = 0u;
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286)
        {
            if (!_GetCR0_PE)
                mask = VCPU_EFLAGS_NT | VCPU_EFLAGS_IOPL;
            else
            {
                if (_GetCPL != 0u) mask |= VCPU_EFLAGS_IOPL;
                if (_GetCPL > _GetEFLAGS_IOPL) mask |= VCPU_EFLAGS_IF;
            }
        }
        cpu_state.data.eflags = _e_eflags_load(context,
            (ceflags & ~mask) | (cpu_state.data.eflags & mask));
    }
    CPU_TRACE_CALL_END;
}
static void SAHF(x86_core_cpu_execution_context *context)
{
    lib_u32 mask = (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
                             VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_CF);
    CPU_TRACE_CALL_BEGIN("SAHF");
    _adv;
    cpu_state.data.eflags = (cpu_state.data.ah & mask) | (cpu_state.data.eflags & ~mask);
    CPU_TRACE_CALL_END;
}
static void LAHF(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("LAHF");
    _adv;
    _WriteGPR8(ah) = X86_CPU_MASK_U8(cpu_state.data.flags) | 0x02u;
    CPU_TRACE_CALL_END;
}
static void MOV_AL_MOFFS8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_AL_MOFFS8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_moffs(context, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    _WriteGPR8(al) = X86_CPU_MASK_U8(instruction_state.data.crm);
    CPU_TRACE_CALL_END;
}
static void MOV_EAX_MOFFS32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_EAX_MOFFS32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_moffs(context, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.crm);
            break;
        case 4:
            _WriteGPR32(eax) = X86_CPU_MASK_U32(instruction_state.data.crm);
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_moffs(context, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.crm);
    }
    CPU_TRACE_CALL_END;
}
static void MOV_MOFFS8_AL(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_MOFFS8_AL");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_moffs(context, 1));
    instruction_state.data.result = cpu_state.data.al;
    instruction_state.data.crm = instruction_state.data.result;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    CPU_TRACE_CALL_END;
}
static void MOV_MOFFS32_EAX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_MOFFS32_EAX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_moffs(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            instruction_state.data.result = cpu_state.data.ax;
            break;
        case 4:
            instruction_state.data.result = cpu_state.data.eax;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_moffs(context, 2));
        instruction_state.data.crm = cpu_state.data.ax;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    CPU_TRACE_CALL_END;
}
static void MOVSB(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOVSB");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            CPU_TRACE_CHECK_RETURN(_m_movs(context, 1));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    CPU_TRACE_BLOCK_BEGIN("cx(!0)");
                    CPU_TRACE_CHECK_RETURN(_m_movs(context, 1));
                    _WriteGPR16(cx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            case 4:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    CPU_TRACE_BLOCK_BEGIN("ecx(!0)");
                    CPU_TRACE_CHECK_RETURN(_m_movs(context, 1));
                    _WriteGPR32(ecx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
        }
    }
    else
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            CPU_TRACE_CHECK_RETURN(_m_movs(context, 1));
        else
        {
            if (cpu_state.data.cx)
            {
                CPU_TRACE_CHECK_RETURN(_m_movs(context, 1));
                _WriteGPR16(cx)--;
            }
            if (cpu_state.data.cx)
                instruction_state.data.flagInsLoop = LIB_TRUE;
        }
    }
    CPU_TRACE_CALL_END;
}
static void MOVSW(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOVSW");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            CPU_TRACE_CHECK_RETURN(_m_movs(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    CPU_TRACE_BLOCK_BEGIN("cx(!0)");
                    CPU_TRACE_CHECK_RETURN(_m_movs(context, _GetOperandSize));
                    _WriteGPR16(cx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            case 4:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    CPU_TRACE_BLOCK_BEGIN("ecx(!0)");
                    CPU_TRACE_CHECK_RETURN(_m_movs(context, _GetOperandSize));
                    _WriteGPR32(ecx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
        }
    }
    else
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            CPU_TRACE_CHECK_RETURN(_m_movs(context, 2));
        else
        {
            if (cpu_state.data.cx)
            {
                CPU_TRACE_CHECK_RETURN(_m_movs(context, 2));
                _WriteGPR16(cx)--;
            }
            if (cpu_state.data.cx)
                instruction_state.data.flagInsLoop = LIB_TRUE;
        }
    }
    CPU_TRACE_CALL_END;
}
static void CMPSB(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("CMPSB");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            CPU_TRACE_CHECK_RETURN(_a_cmps(context, 8));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    CPU_TRACE_BLOCK_BEGIN("cx(!0)");
                    CPU_TRACE_CHECK_RETURN(_a_cmps(context, 8));
                    _WriteGPR16(cx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            case 4:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    CPU_TRACE_BLOCK_BEGIN("ecx(!0)");
                    CPU_TRACE_CHECK_RETURN(_a_cmps(context, 8));
                    _WriteGPR32(ecx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
        }
    }
    else
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            CPU_TRACE_CHECK_RETURN(_a_cmps(context, 8));
        else
        {
            if (cpu_state.data.cx)
            {
                CPU_TRACE_CHECK_RETURN(_a_cmps(context, 8));
                _WriteGPR16(cx)--;
            }
            if (cpu_state.data.cx &&
                !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                instruction_state.data.flagInsLoop = LIB_TRUE;
        }
    }
    CPU_TRACE_CALL_END;
}
static void CMPSW(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("CMPSW");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            CPU_TRACE_CHECK_RETURN(_a_cmps(context, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    CPU_TRACE_BLOCK_BEGIN("cx(!0)");
                    CPU_TRACE_CHECK_RETURN(_a_cmps(context, _GetOperandSize * 8));
                    _WriteGPR16(cx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            case 4:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    CPU_TRACE_BLOCK_BEGIN("ecx(!0)");
                    CPU_TRACE_CHECK_RETURN(_a_cmps(context, _GetOperandSize * 8));
                    _WriteGPR32(ecx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
        }
    }
    else
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            CPU_TRACE_CHECK_RETURN(_a_cmps(context, 16));
        else
        {
            if (cpu_state.data.cx)
            {
                CPU_TRACE_CHECK_RETURN(_a_cmps(context, 16));
                _WriteGPR16(cx)--;
            }
            if (cpu_state.data.cx &&
                !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                instruction_state.data.flagInsLoop = LIB_TRUE;
        }
    }
    CPU_TRACE_CALL_END;
}
static void TEST_AL_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("TEST_AL_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_a_test(context, cpu_state.data.al, instruction_state.data.cimm, 8));
    CPU_TRACE_CALL_END;
}
static void TEST_EAX_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("TEST_EAX_I32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
            CPU_TRACE_CHECK_RETURN(_a_test(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 4));
            CPU_TRACE_CHECK_RETURN(_a_test(context, cpu_state.data.eax, instruction_state.data.cimm, 32));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        CPU_TRACE_CHECK_RETURN(_a_test(context, cpu_state.data.ax, instruction_state.data.cimm, 16));
    }
    CPU_TRACE_CALL_END;
}
static void STOSB(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("STOSB");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            CPU_TRACE_CHECK_RETURN(_m_stos(context, 1));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    CPU_TRACE_BLOCK_BEGIN("cx(!0)");
                    CPU_TRACE_CHECK_RETURN(_m_stos(context, 1));
                    _WriteGPR16(cx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            case 4:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    CPU_TRACE_BLOCK_BEGIN("ecx(!0)");
                    CPU_TRACE_CHECK_RETURN(_m_stos(context, 1));
                    _WriteGPR32(ecx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
        }
    }
    else
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            CPU_TRACE_CHECK_RETURN(_m_stos(context, 1));
        else
        {
            if (cpu_state.data.cx)
            {
                CPU_TRACE_CHECK_RETURN(_m_stos(context, 1));
                _WriteGPR16(cx)--;
            }
            if (cpu_state.data.cx)
                instruction_state.data.flagInsLoop = LIB_TRUE;
        }
    }
    CPU_TRACE_CALL_END;
}
static void STOSW(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("STOSW");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            CPU_TRACE_CHECK_RETURN(_m_stos(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    CPU_TRACE_BLOCK_BEGIN("cx(!0)");
                    CPU_TRACE_CHECK_RETURN(_m_stos(context, _GetOperandSize));
                    _WriteGPR16(cx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            case 4:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    CPU_TRACE_BLOCK_BEGIN("ecx(!0)");
                    CPU_TRACE_CHECK_RETURN(_m_stos(context, _GetOperandSize));
                    _WriteGPR32(ecx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
        }
    }
    else
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            CPU_TRACE_CHECK_RETURN(_m_stos(context, 2));
        else
        {
            if (cpu_state.data.cx)
            {
                CPU_TRACE_CHECK_RETURN(_m_stos(context, 2));
                _WriteGPR16(cx)--;
            }
            if (cpu_state.data.cx)
                instruction_state.data.flagInsLoop = LIB_TRUE;
        }
    }
    CPU_TRACE_CALL_END;
}
static void LODSB(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("LODSB");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            CPU_TRACE_CHECK_RETURN(_m_lods(context, 1));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    CPU_TRACE_BLOCK_BEGIN("cx(!0)");
                    CPU_TRACE_CHECK_RETURN(_m_lods(context, 1));
                    _WriteGPR16(cx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            case 4:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    CPU_TRACE_BLOCK_BEGIN("ecx(!0)");
                    CPU_TRACE_CHECK_RETURN(_m_lods(context, 1));
                    _WriteGPR32(ecx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
        }
    }
    else
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            CPU_TRACE_CHECK_RETURN(_m_lods(context, 1));
        else
        {
            if (cpu_state.data.cx)
            {
                CPU_TRACE_CHECK_RETURN(_m_lods(context, 1));
                _WriteGPR16(cx)--;
            }
            if (cpu_state.data.cx)
                instruction_state.data.flagInsLoop = LIB_TRUE;
        }
    }
    CPU_TRACE_CALL_END;
}
static void LODSW(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("LODSW");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            CPU_TRACE_CHECK_RETURN(_m_lods(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    CPU_TRACE_BLOCK_BEGIN("cx(!0)");
                    CPU_TRACE_CHECK_RETURN(_m_lods(context, _GetOperandSize));
                    _WriteGPR16(cx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            case 4:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    CPU_TRACE_BLOCK_BEGIN("ecx(!0)");
                    CPU_TRACE_CHECK_RETURN(_m_lods(context, _GetOperandSize));
                    _WriteGPR32(ecx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx)
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
        }
    }
    else
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            CPU_TRACE_CHECK_RETURN(_m_lods(context, 2));
        else
        {
            if (cpu_state.data.cx)
            {
                CPU_TRACE_CHECK_RETURN(_m_lods(context, 2));
                _WriteGPR16(cx)--;
            }
            if (cpu_state.data.cx)
                instruction_state.data.flagInsLoop = LIB_TRUE;
        }
    }
    CPU_TRACE_CALL_END;
}
static void SCASB(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SCASB");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            CPU_TRACE_CHECK_RETURN(_a_scas(context, 8));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    CPU_TRACE_BLOCK_BEGIN("cx(!0)");
                    CPU_TRACE_CHECK_RETURN(_a_scas(context, 8));
                    _WriteGPR16(cx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            case 4:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    CPU_TRACE_BLOCK_BEGIN("ecx(!0)");
                    CPU_TRACE_CHECK_RETURN(_a_scas(context, 8));
                    _WriteGPR32(ecx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
        }
    }
    else
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            CPU_TRACE_CHECK_RETURN(_a_scas(context, 8));
        else
        {
            if (cpu_state.data.cx)
            {
                CPU_TRACE_CHECK_RETURN(_a_scas(context, 8));
                _WriteGPR16(cx)--;
            }
            if (cpu_state.data.cx &&
                !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                instruction_state.data.flagInsLoop = LIB_TRUE;
        }
    }
    CPU_TRACE_CALL_END;
}
static void SCASW(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SCASW");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(PREFIX_REP_NONE)");
            CPU_TRACE_CHECK_RETURN(_a_scas(context, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("prefix_rep(!PREFIX_REP_NONE)");
            switch (_GetAddressSize)
            {
            case 2:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(2)");
                if (cpu_state.data.cx)
                {
                    CPU_TRACE_BLOCK_BEGIN("cx(!0)");
                    CPU_TRACE_CHECK_RETURN(_a_scas(context, _GetOperandSize * 8));
                    _WriteGPR16(cx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.cx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            case 4:
                CPU_TRACE_BLOCK_BEGIN("AddressSize(4)");
                if (cpu_state.data.ecx)
                {
                    CPU_TRACE_BLOCK_BEGIN("ecx(!0)");
                    CPU_TRACE_CHECK_RETURN(_a_scas(context, _GetOperandSize * 8));
                    _WriteGPR32(ecx)--;
                    CPU_TRACE_BLOCK_END;
                }
                if (cpu_state.data.ecx &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                    !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                    instruction_state.data.flagInsLoop = LIB_TRUE;
                CPU_TRACE_BLOCK_END;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
        }
    }
    else
    {
        _adv;
        if (instruction_state.data.prefix_rep == PREFIX_REP_NONE)
            CPU_TRACE_CHECK_RETURN(_a_scas(context, 16));
        else
        {
            if (cpu_state.data.cx)
            {
                CPU_TRACE_CHECK_RETURN(_a_scas(context, 16));
                _WriteGPR16(cx)--;
            }
            if (cpu_state.data.cx &&
                !(instruction_state.data.prefix_rep == PREFIX_REP_REPZ && !_GetEFLAGS_ZF) &&
                !(instruction_state.data.prefix_rep == PREFIX_REP_REPZNZ && _GetEFLAGS_ZF))
                instruction_state.data.flagInsLoop = LIB_TRUE;
        }
    }
    CPU_TRACE_CALL_END;
}
static void MOV_AL_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_AL_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    _WriteGPR8(al) = X86_CPU_MASK_U8(instruction_state.data.cimm);
    CPU_TRACE_CALL_END;
}
static void MOV_CL_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_CL_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    _WriteGPR8(cl) = X86_CPU_MASK_U8(instruction_state.data.cimm);
    CPU_TRACE_CALL_END;
}
static void MOV_DL_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_DL_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    _WriteGPR8(dl) = X86_CPU_MASK_U8(instruction_state.data.cimm);
    CPU_TRACE_CALL_END;
}
static void MOV_BL_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_BL_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    _WriteGPR8(bl) = X86_CPU_MASK_U8(instruction_state.data.cimm);
    CPU_TRACE_CALL_END;
}
static void MOV_AH_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_AH_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    _WriteGPR8(ah) = X86_CPU_MASK_U8(instruction_state.data.cimm);
    CPU_TRACE_CALL_END;
}
static void MOV_CH_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_CH_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    _WriteGPR8(ch) = X86_CPU_MASK_U8(instruction_state.data.cimm);
    CPU_TRACE_CALL_END;
}
static void MOV_DH_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_DH_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    _WriteGPR8(dh) = X86_CPU_MASK_U8(instruction_state.data.cimm);
    CPU_TRACE_CALL_END;
}
static void MOV_BH_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_BH_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    _WriteGPR8(bh) = X86_CPU_MASK_U8(instruction_state.data.cimm);
    CPU_TRACE_CALL_END;
}
static void MOV_EAX_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_EAX_I32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.cimm);
            break;
        case 4:
            _WriteGPR32(eax) = X86_CPU_MASK_U32(instruction_state.data.cimm);
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        _WriteGPR16(ax) = X86_CPU_MASK_U16(instruction_state.data.cimm);
    }
    CPU_TRACE_CALL_END;
}
static void MOV_ECX_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_ECX_I32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            _WriteGPR16(cx) = X86_CPU_MASK_U16(instruction_state.data.cimm);
            break;
        case 4:
            _WriteGPR32(ecx) = X86_CPU_MASK_U32(instruction_state.data.cimm);
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        _WriteGPR16(cx) = X86_CPU_MASK_U16(instruction_state.data.cimm);
    }
    CPU_TRACE_CALL_END;
}
static void MOV_EDX_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_EDX_I32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            _WriteGPR16(dx) = X86_CPU_MASK_U16(instruction_state.data.cimm);
            break;
        case 4:
            _WriteGPR32(edx) = X86_CPU_MASK_U32(instruction_state.data.cimm);
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        _WriteGPR16(dx) = X86_CPU_MASK_U16(instruction_state.data.cimm);
    }
    CPU_TRACE_CALL_END;
}
static void MOV_EBX_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_EBX_I32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            _WriteGPR16(bx) = X86_CPU_MASK_U16(instruction_state.data.cimm);
            break;
        case 4:
            _WriteGPR32(ebx) = X86_CPU_MASK_U32(instruction_state.data.cimm);
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        _WriteGPR16(bx) = X86_CPU_MASK_U16(instruction_state.data.cimm);
    }
    CPU_TRACE_CALL_END;
}
static void MOV_ESP_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_ESP_I32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            _WriteGPR16(sp) = X86_CPU_MASK_U16(instruction_state.data.cimm);
            break;
        case 4:
            _WriteGPR32(esp) = X86_CPU_MASK_U32(instruction_state.data.cimm);
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        _WriteGPR16(sp) = X86_CPU_MASK_U16(instruction_state.data.cimm);
    }
    CPU_TRACE_CALL_END;
}
static void MOV_EBP_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_EBP_I32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            _WriteGPR16(bp) = X86_CPU_MASK_U16(instruction_state.data.cimm);
            break;
        case 4:
            _WriteGPR32(ebp) = X86_CPU_MASK_U32(instruction_state.data.cimm);
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        _WriteGPR16(bp) = X86_CPU_MASK_U16(instruction_state.data.cimm);
    }
    CPU_TRACE_CALL_END;
}
static void MOV_ESI_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_ESI_I32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            _WriteGPR16(si) = X86_CPU_MASK_U16(instruction_state.data.cimm);
            break;
        case 4:
            _WriteGPR32(esi) = X86_CPU_MASK_U32(instruction_state.data.cimm);
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        _WriteGPR16(si) = X86_CPU_MASK_U16(instruction_state.data.cimm);
    }
    CPU_TRACE_CALL_END;
}
static void MOV_EDI_I32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_EDI_I32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        switch (_GetOperandSize)
        {
        case 2:
            _WriteGPR16(di) = X86_CPU_MASK_U16(instruction_state.data.cimm);
            break;
        case 4:
            _WriteGPR32(edi) = X86_CPU_MASK_U32(instruction_state.data.cimm);
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        _WriteGPR16(di) = X86_CPU_MASK_U16(instruction_state.data.cimm);
    }
    CPU_TRACE_CALL_END;
}
static void INS_C0(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INS_C0");
    _new_code_path_;
    if (cpu_profile_has_186_integer(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 1, LIB_FALSE));
        switch (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
            instruction_state.data.cr == 6u ? 4u : instruction_state.data.cr)
        {
        case 0: /* ROL_RM8_I8 */
            CPU_TRACE_BLOCK_BEGIN("ROL_RM8_I8");
            CPU_TRACE_CHECK_RETURN(_a_rol(context, X86_CPU_MASK_U8(instruction_state.data.crm), X86_CPU_MASK_U8(instruction_state.data.cimm), 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 1: /* ROR_RM8_I8 */
            CPU_TRACE_BLOCK_BEGIN("ROR_RM8_I8");
            CPU_TRACE_CHECK_RETURN(_a_ror(context, X86_CPU_MASK_U8(instruction_state.data.crm), X86_CPU_MASK_U8(instruction_state.data.cimm), 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 2: /* RCL_RM8_I8 */
            CPU_TRACE_BLOCK_BEGIN("RCL_RM8_I8");
            CPU_TRACE_CHECK_RETURN(_a_rcl(context, X86_CPU_MASK_U8(instruction_state.data.crm), X86_CPU_MASK_U8(instruction_state.data.cimm), 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 3: /* RCR_RM8_I8 */
            CPU_TRACE_BLOCK_BEGIN("RCR_RM8_I8");
            CPU_TRACE_CHECK_RETURN(_a_rcr(context, X86_CPU_MASK_U8(instruction_state.data.crm), X86_CPU_MASK_U8(instruction_state.data.cimm), 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 4: /* SHL_RM8_I8 */
            CPU_TRACE_BLOCK_BEGIN("SHL_RM8_I8");
            CPU_TRACE_CHECK_RETURN(_a_shl(context, X86_CPU_MASK_U8(instruction_state.data.crm), X86_CPU_MASK_U8(instruction_state.data.cimm), 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 5: /* SHR_RM8_I8 */
            CPU_TRACE_BLOCK_BEGIN("SHR_RM8_I8");
            CPU_TRACE_CHECK_RETURN(_a_shr(context, X86_CPU_MASK_U8(instruction_state.data.crm), X86_CPU_MASK_U8(instruction_state.data.cimm), 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 6: /* UndefinedOpcode */
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 7: /* SAR_RM8_I8 */
            CPU_TRACE_BLOCK_BEGIN("SAR_RM8_I8");
            CPU_TRACE_CHECK_RETURN(_a_sar(context, X86_CPU_MASK_U8(instruction_state.data.crm), X86_CPU_MASK_U8(instruction_state.data.cimm), 8));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_shift(context, 1, (lib_u8)instruction_state.data.cimm));
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void INS_C1(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INS_C1");
    if (cpu_profile_has_186_integer(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
        switch (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
            instruction_state.data.cr == 6u ? 4u : instruction_state.data.cr)
        {
        case 0: /* ROL_RM32_I8 */
            CPU_TRACE_BLOCK_BEGIN("ROL_RM32_I8");
            CPU_TRACE_CHECK_RETURN(_a_rol(context, X86_CPU_MASK_U32(instruction_state.data.crm), X86_CPU_MASK_U8(instruction_state.data.cimm), _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 1: /* ROR_RM32_I8 */
            CPU_TRACE_BLOCK_BEGIN("ROR_RM32_I8");
            CPU_TRACE_CHECK_RETURN(_a_ror(context, X86_CPU_MASK_U32(instruction_state.data.crm), X86_CPU_MASK_U8(instruction_state.data.cimm), _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 2: /* RCL_RM32_I8 */
            CPU_TRACE_BLOCK_BEGIN("RCL_RM32_I8");
            CPU_TRACE_CHECK_RETURN(_a_rcl(context, X86_CPU_MASK_U32(instruction_state.data.crm), X86_CPU_MASK_U8(instruction_state.data.cimm), _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 3: /* RCR_RM32_I8 */
            CPU_TRACE_BLOCK_BEGIN("RCR_RM32_I8");
            CPU_TRACE_CHECK_RETURN(_a_rcr(context, X86_CPU_MASK_U32(instruction_state.data.crm), X86_CPU_MASK_U8(instruction_state.data.cimm), _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 4: /* SHL_RM32_I8 */
            CPU_TRACE_BLOCK_BEGIN("SHL_RM32_I8");
            CPU_TRACE_CHECK_RETURN(_a_shl(context, X86_CPU_MASK_U32(instruction_state.data.crm), X86_CPU_MASK_U8(instruction_state.data.cimm), _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 5: /* SHR_RM32_I8 */
            CPU_TRACE_BLOCK_BEGIN("SHR_RM32_I8");
            CPU_TRACE_CHECK_RETURN(_a_shr(context, X86_CPU_MASK_U32(instruction_state.data.crm), X86_CPU_MASK_U8(instruction_state.data.cimm), _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 6: /* UndefinedOpcode */
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 7: /* SAR_RM32_I8 */
            CPU_TRACE_BLOCK_BEGIN("SAR_RM32_I8");
            CPU_TRACE_CHECK_RETURN(_a_sar(context, X86_CPU_MASK_U32(instruction_state.data.crm), X86_CPU_MASK_U8(instruction_state.data.cimm), _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_shift(context, _GetOperandSize, (lib_u8)instruction_state.data.cimm));
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void RET_I16(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("RET_I16");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        CPU_TRACE_CHECK_RETURN(_e_ret_near(context, X86_CPU_MASK_U16(instruction_state.data.cimm), _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        _e_ret_near(context, X86_CPU_MASK_U16(instruction_state.data.cimm), 2);
    }
    CPU_TRACE_CALL_END;
}
static void RET(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("RET");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_e_ret_near(context, 0, _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_e_ret_near(context, 0, 2));
    }
    CPU_TRACE_CALL_END;
}
static void LES_R32_M16_32(x86_core_cpu_execution_context *context)
{
    lib_u16 selector;
    lib_u32 offset;
    CPU_TRACE_CALL_BEGIN("LES_R32_M16_32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm_ea(context, _GetOperandSize,
            _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_pair(context, _GetOperandSize, 2));
        switch (_GetOperandSize)
        {
        case 2:
            offset = X86_CPU_MASK_U16(instruction_state.data.crm);
            break;
        case 4:
            offset = X86_CPU_MASK_U32(instruction_state.data.crm);
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        selector = X86_CPU_MASK_U16(instruction_state.data.crm >> (_GetOperandSize * 8u));
        CPU_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.es, instruction_state.data.rr, selector, offset, _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm_ea(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_pair(context, 2, 2));
        offset = X86_CPU_MASK_U16(instruction_state.data.crm);
        selector = X86_CPU_MASK_U16(instruction_state.data.crm >> (2 * 8u));
        CPU_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.es, instruction_state.data.rr, selector, offset, 2));
    }
    CPU_TRACE_CALL_END;
}
static void LDS_R32_M16_32(x86_core_cpu_execution_context *context)
{
    lib_u16 selector;
    lib_u32 offset;
    CPU_TRACE_CALL_BEGIN("LDS_R32_M16_32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm_ea(context, _GetOperandSize,
            _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_pair(context, _GetOperandSize, 2));
        switch (_GetOperandSize)
        {
        case 2:
            offset = X86_CPU_MASK_U16(instruction_state.data.crm);
            break;
        case 4:
            offset = X86_CPU_MASK_U32(instruction_state.data.crm);
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        selector = X86_CPU_MASK_U16(instruction_state.data.crm >> (_GetOperandSize * 8u));
        CPU_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.ds, instruction_state.data.rr, selector, offset, _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm_ea(context, 2, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_pair(context, 2, 2));
        offset = X86_CPU_MASK_U16(instruction_state.data.crm);
        selector = X86_CPU_MASK_U16(instruction_state.data.crm >> (2 * 8u));
        CPU_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.ds, instruction_state.data.rr, selector, offset, 2));
    }
    CPU_TRACE_CALL_END;
}
static void INS_C6(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INS_C6");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    switch (instruction_state.data.cr)
    {
    case 0: /* MOV_RM8_I8 */
        CPU_TRACE_BLOCK_BEGIN("MOV_RM8_I8");
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
        instruction_state.data.crm = instruction_state.data.cimm;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 1:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 2:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 3:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 5:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 6:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 7:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void INS_C7(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INS_C7");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        switch (instruction_state.data.cr)
        {
        case 0: /* MOV_RM32_I32 */
            CPU_TRACE_BLOCK_BEGIN("MOV_RM32_I32");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
            instruction_state.data.crm = instruction_state.data.cimm;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 1:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 2:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 3:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 5:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 6:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 7:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        switch (instruction_state.data.cr)
        {
        case 0: /* MOV_RM16_I16 */
            CPU_TRACE_BLOCK_BEGIN("MOV_RM16_I16");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
            instruction_state.data.crm = instruction_state.data.cimm;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 1:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 2:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 3:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 5:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 6:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 7:
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    CPU_TRACE_CALL_END;
}
static void ENTER(x86_core_cpu_execution_context *context)
{
    lib_u32 data = 0;
    lib_u32 temp = 0;
    lib_uptr i = 0;
    lib_u16 size = 0u;
    lib_u8 level = 0u;
    CPU_TRACE_CALL_BEGIN("ENTER");
    if (cpu_profile_has_186_integer(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        size = (lib_u16)instruction_state.data.cimm;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
        level = (lib_u8)instruction_state.data.cimm;
        /* The 80186 defines the byte as an unsigned lexical level through
         * 255.  The later 80286/80386 architecture limits it to 0--31. */
        if (cpu_profile_has_protected_architecture(context->cpu_profile))
            level %= 32;
        CPU_TRACE_CHECK_RETURN(_s_test_ss_frame(context, _GetOperandSize,
            (lib_u16)1u + level, size, LIB_TRUE));
        data = _GetOperandSize == 2 ? cpu_state.data.bp : cpu_state.data.ebp;
        for (i = 1u; i < level; ++i)
        {
            data = _GetOperandSize == 2 ? X86_CPU_MASK_U16(data - 2u) :
                X86_CPU_MASK_U32(data - 4u);
            CPU_TRACE_CHECK_RETURN(_m_test_logical(context, &cpu_state.data.ss,
                data, _GetOperandSize, LIB_FALSE));
        }
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.bp), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(cpu_state.data.ebp), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        temp = cpu_state.data.esp;
        if (level)
        {
            CPU_TRACE_BLOCK_BEGIN("level(!0)");
            for (i = 1; i < level; ++i)
            {
                CPU_TRACE_BLOCK_BEGIN("for");
                switch (_GetOperandSize)
                {
                case 2:
                    CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
                    _WriteGPR16(bp) -= 2;
                    CPU_TRACE_CHECK_RETURN(_s_read_ss(context, cpu_state.data.bp, X86_CPU_REFERENCE_OF(data), 2));
                    CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(data), 2));
                    CPU_TRACE_BLOCK_END;
                    break;
                case 4:
                    CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
                    _WriteGPR32(ebp) -= 4;
                    CPU_TRACE_CHECK_RETURN(_s_read_ss(context, cpu_state.data.ebp, X86_CPU_REFERENCE_OF(data), 4));
                    CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(data), 4));
                    CPU_TRACE_BLOCK_END;
                    break;
                default:
                    CPU_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                CPU_TRACE_BLOCK_END;
            }
            switch (_GetOperandSize)
            {
            case 2:
                CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
                CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(temp), 2));
                CPU_TRACE_BLOCK_END;
                break;
            case 4:
                CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
                CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(temp), 4));
                CPU_TRACE_BLOCK_END;
                break;
            default:
                CPU_TRACE_IMPOSSIBLE_RETURN;
                break;
            }
            CPU_TRACE_BLOCK_END;
        }
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            _WriteGPR16(bp) = X86_CPU_MASK_U16(temp);
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _WriteGPR32(ebp) = X86_CPU_MASK_U32(temp);
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        switch (_GetStackSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("StackSize(2)");
            _WriteGPR16(sp) -= size;
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("StackSize(4)");
            _WriteGPR32(esp) -= size;
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void LEAVE(x86_core_cpu_execution_context *context)
{
    lib_u32 stack = 0u;
    CPU_TRACE_CALL_BEGIN("LEAVE");
    if (cpu_profile_has_186_integer(context->cpu_profile))
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
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        CPU_TRACE_CHECK_RETURN(_m_test_logical(context, &cpu_state.data.ss,
            stack, _GetOperandSize, 0));
        switch (_GetStackSize)
        {
        case 2:
            _WriteGPR16(sp) = cpu_state.data.bp;
            break;
        case 4:
            _WriteGPR32(esp) = cpu_state.data.ebp;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.bp), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_kec_pop(context, X86_CPU_REFERENCE_OF(cpu_state.data.ebp), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
        UndefinedOpcode(context);
    CPU_TRACE_CALL_END;
}
static void RETF_I16(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("RETF_I16");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        CPU_TRACE_CHECK_RETURN(_e_ret_far(context, X86_CPU_MASK_U16(instruction_state.data.cimm), _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        _e_ret_far(context, X86_CPU_MASK_U16(instruction_state.data.cimm), 2);
    }
    CPU_TRACE_CALL_END;
}
static void RETF(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("RETF");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_e_ret_far(context, 0, _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_e_ret_far(context, 0, 2));
    }
    CPU_TRACE_CALL_END;
}
static void INT3(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INT3");
    _new_code_path_;
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_e_int3(context, _GetOperandSize));
    }
    else
    {
        _adv;
        _e_int_n(context, 0x03, _GetOperandSize);
    }
    CPU_TRACE_CALL_END;
}
static void INT_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INT_I8");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
        CPU_TRACE_CHECK_RETURN(_e_int_n(context,
            (lib_u8)instruction_state.data.cimm, _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
        _e_int_n(context, (lib_u8)instruction_state.data.cimm, 2);
    }
    CPU_TRACE_CALL_END;
}
static void INTO(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INTO");
    _new_code_path_;
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_e_into(context, _GetOperandSize));
    }
    else
    {
        _adv;
        if (_GetEFLAGS_OF)
            _e_int_n(context, 0x04, _GetOperandSize);
    }
    CPU_TRACE_CALL_END;
}
static void IRET(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("IRET");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_e_iret(context, _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_e_iret(context, 2));
    }
    CPU_TRACE_CALL_END;
}
static void INS_D0(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INS_D0");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 1, LIB_FALSE));
    switch (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        instruction_state.data.cr == 6u ? 4u : instruction_state.data.cr)
    {
    case 0: /* ROL_RM8 */
        CPU_TRACE_BLOCK_BEGIN("ROL_RM8");
        CPU_TRACE_CHECK_RETURN(_a_rol(context, instruction_state.data.crm, 1, 8));
        CPU_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM8 */
        CPU_TRACE_BLOCK_BEGIN("ROR_RM8");
        CPU_TRACE_CHECK_RETURN(_a_ror(context, instruction_state.data.crm, 1, 8));
        CPU_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM8 */
        CPU_TRACE_BLOCK_BEGIN("RCL_RM8");
        CPU_TRACE_CHECK_RETURN(_a_rcl(context, instruction_state.data.crm, 1, 8));
        CPU_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM8 */
        CPU_TRACE_BLOCK_BEGIN("RCR_RM8");
        CPU_TRACE_CHECK_RETURN(_a_rcr(context, instruction_state.data.crm, 1, 8));
        CPU_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM8 */
        CPU_TRACE_BLOCK_BEGIN("SHL_RM8");
        CPU_TRACE_CHECK_RETURN(_a_shl(context, instruction_state.data.crm, 1, 8));
        CPU_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM8 */
        CPU_TRACE_BLOCK_BEGIN("SHR_RM8");
        CPU_TRACE_CHECK_RETURN(_a_shr(context, instruction_state.data.crm, 1, 8));
        CPU_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM8 */
        CPU_TRACE_BLOCK_BEGIN("SAR_RM8");
        CPU_TRACE_CHECK_RETURN(_a_sar(context, instruction_state.data.crm, 1, 8));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    instruction_state.data.crm = instruction_state.data.result;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    CPU_TRACE_CALL_END;
}
static void INS_D1(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INS_D1");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
        switch (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
            instruction_state.data.cr == 6u ? 4u : instruction_state.data.cr)
        {
        case 0: /* ROL_RM32 */
            CPU_TRACE_BLOCK_BEGIN("ROL_RM32");
            CPU_TRACE_CHECK_RETURN(_a_rol(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 1: /* ROR_RM32 */
            CPU_TRACE_BLOCK_BEGIN("ROR_RM32");
            CPU_TRACE_CHECK_RETURN(_a_ror(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 2: /* RCL_RM32 */
            CPU_TRACE_BLOCK_BEGIN("RCL_RM32");
            CPU_TRACE_CHECK_RETURN(_a_rcl(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 3: /* RCR_RM32 */
            CPU_TRACE_BLOCK_BEGIN("RCR_RM32");
            CPU_TRACE_CHECK_RETURN(_a_rcr(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 4: /* SHL_RM32 */
            CPU_TRACE_BLOCK_BEGIN("SHL_RM32");
            CPU_TRACE_CHECK_RETURN(_a_shl(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 5: /* SHR_RM32 */
            CPU_TRACE_BLOCK_BEGIN("SHR_RM32");
            CPU_TRACE_CHECK_RETURN(_a_shr(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 6: /* UndefinedOpcode */
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 7: /* SAR_RM32 */
            CPU_TRACE_BLOCK_BEGIN("SAR_RM32");
            CPU_TRACE_CHECK_RETURN(_a_sar(context, instruction_state.data.crm, 1, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 2, LIB_FALSE));
        switch (instruction_state.data.cr)
        {
        case 0: /* ROL_RM16 */
            CPU_TRACE_BLOCK_BEGIN("ROL_RM16");
            CPU_TRACE_CHECK_RETURN(_a_rol(context, instruction_state.data.crm, 1, 16));
            CPU_TRACE_BLOCK_END;
            break;
        case 1: /* ROR_RM16 */
            CPU_TRACE_BLOCK_BEGIN("ROR_RM16");
            CPU_TRACE_CHECK_RETURN(_a_ror(context, instruction_state.data.crm, 1, 16));
            CPU_TRACE_BLOCK_END;
            break;
        case 2: /* RCL_RM16 */
            CPU_TRACE_BLOCK_BEGIN("RCL_RM16");
            CPU_TRACE_CHECK_RETURN(_a_rcl(context, instruction_state.data.crm, 1, 16));
            CPU_TRACE_BLOCK_END;
            break;
        case 3: /* RCR_RM16 */
            CPU_TRACE_BLOCK_BEGIN("RCR_RM16");
            CPU_TRACE_CHECK_RETURN(_a_rcr(context, instruction_state.data.crm, 1, 16));
            CPU_TRACE_BLOCK_END;
            break;
        case 4: /* SHL_RM16 */
            CPU_TRACE_BLOCK_BEGIN("SHL_RM16");
            CPU_TRACE_CHECK_RETURN(_a_shl(context, instruction_state.data.crm, 1, 16));
            CPU_TRACE_BLOCK_END;
            break;
        case 5: /* SHR_RM16 */
            CPU_TRACE_BLOCK_BEGIN("SHR_RM16");
            CPU_TRACE_CHECK_RETURN(_a_shr(context, instruction_state.data.crm, 1, 16));
            CPU_TRACE_BLOCK_END;
            break;
        case 6: /* UndefinedOpcode */
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 7: /* SAR_RM16 */
            CPU_TRACE_BLOCK_BEGIN("SAR_RM16");
            CPU_TRACE_CHECK_RETURN(_a_sar(context, instruction_state.data.crm, 1, 16));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
    }
    CPU_TRACE_CALL_END;
}
static void INS_D2(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INS_D2");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 1, LIB_FALSE));
    switch (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        instruction_state.data.cr == 6u ? 4u : instruction_state.data.cr)
    {
    case 0: /* ROL_RM8_CL */
        CPU_TRACE_BLOCK_BEGIN("ROL_RM8_CL");
        CPU_TRACE_CHECK_RETURN(_a_rol(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        CPU_TRACE_BLOCK_END;
        break;
    case 1: /* ROR_RM8_CL */
        CPU_TRACE_BLOCK_BEGIN("ROR_RM8_CL");
        CPU_TRACE_CHECK_RETURN(_a_ror(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        CPU_TRACE_BLOCK_END;
        break;
    case 2: /* RCL_RM8_CL */
        CPU_TRACE_BLOCK_BEGIN("RCL_RM8_CL");
        CPU_TRACE_CHECK_RETURN(_a_rcl(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        CPU_TRACE_BLOCK_END;
        break;
    case 3: /* RCR_RM8_CL */
        CPU_TRACE_BLOCK_BEGIN("RCR_RM8_CL");
        CPU_TRACE_CHECK_RETURN(_a_rcr(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        CPU_TRACE_BLOCK_END;
        break;
    case 4: /* SHL_RM8_CL */
        CPU_TRACE_BLOCK_BEGIN("SHL_RM8_CL");
        CPU_TRACE_CHECK_RETURN(_a_shl(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        CPU_TRACE_BLOCK_END;
        break;
    case 5: /* SHR_RM8_CL */
        CPU_TRACE_BLOCK_BEGIN("SHR_RM8_CL");
        CPU_TRACE_CHECK_RETURN(_a_shr(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        CPU_TRACE_BLOCK_END;
        break;
    case 6: /* UndefinedOpcode */
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 7: /* SAR_RM8_CL */
        CPU_TRACE_BLOCK_BEGIN("SAR_RM8_CL");
        CPU_TRACE_CHECK_RETURN(_a_sar(context, instruction_state.data.crm, cpu_state.data.cl, 8));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    instruction_state.data.crm = instruction_state.data.result;
    CPU_TRACE_CHECK_RETURN(_m_write_shift(context, 1, cpu_state.data.cl));
    CPU_TRACE_CALL_END;
}
static void INS_D3(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INS_D3");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
        switch (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
            instruction_state.data.cr == 6u ? 4u : instruction_state.data.cr)
        {
        case 0: /* ROL_RM32_CL */
            CPU_TRACE_BLOCK_BEGIN("ROL_RM32_CL");
            CPU_TRACE_CHECK_RETURN(_a_rol(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 1: /* ROR_RM32_CL */
            CPU_TRACE_BLOCK_BEGIN("ROR_RM32_CL");
            CPU_TRACE_CHECK_RETURN(_a_ror(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 2: /* RCL_RM32_CL */
            CPU_TRACE_BLOCK_BEGIN("RCL_RM32_CL");
            CPU_TRACE_CHECK_RETURN(_a_rcl(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 3: /* RCR_RM32_CL */
            CPU_TRACE_BLOCK_BEGIN("RCR_RM32_CL");
            CPU_TRACE_CHECK_RETURN(_a_rcr(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 4: /* SHL_RM32_CL */
            CPU_TRACE_BLOCK_BEGIN("SHL_RM32_CL");
            CPU_TRACE_CHECK_RETURN(_a_shl(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 5: /* SHR_RM32_CL */
            CPU_TRACE_BLOCK_BEGIN("SHR_RM32_CL");
            CPU_TRACE_CHECK_RETURN(_a_shr(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 6: /* UndefinedOpcode */
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 7: /* SAR_RM32_CL */
            CPU_TRACE_BLOCK_BEGIN("SAR_RM32_CL");
            CPU_TRACE_CHECK_RETURN(_a_sar(context, instruction_state.data.crm, cpu_state.data.cl, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_shift(context, _GetOperandSize, cpu_state.data.cl));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 2, LIB_FALSE));
        switch (instruction_state.data.cr)
        {
        case 0: /* ROL_RM16_CL */
            CPU_TRACE_BLOCK_BEGIN("ROL_RM16_CL");
            CPU_TRACE_CHECK_RETURN(_a_rol(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            CPU_TRACE_BLOCK_END;
            break;
        case 1: /* ROR_RM16_CL */
            CPU_TRACE_BLOCK_BEGIN("ROR_RM16_CL");
            CPU_TRACE_CHECK_RETURN(_a_ror(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            CPU_TRACE_BLOCK_END;
            break;
        case 2: /* RCL_RM16_CL */
            CPU_TRACE_BLOCK_BEGIN("RCL_RM16_CL");
            CPU_TRACE_CHECK_RETURN(_a_rcl(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            CPU_TRACE_BLOCK_END;
            break;
        case 3: /* RCR_RM16_CL */
            CPU_TRACE_BLOCK_BEGIN("RCR_RM16_CL");
            CPU_TRACE_CHECK_RETURN(_a_rcr(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            CPU_TRACE_BLOCK_END;
            break;
        case 4: /* SHL_RM16_CL */
            CPU_TRACE_BLOCK_BEGIN("SHL_RM16_CL");
            CPU_TRACE_CHECK_RETURN(_a_shl(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            CPU_TRACE_BLOCK_END;
            break;
        case 5: /* SHR_RM16_CL */
            CPU_TRACE_BLOCK_BEGIN("SHR_RM16_CL");
            CPU_TRACE_CHECK_RETURN(_a_shr(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            CPU_TRACE_BLOCK_END;
            break;
        case 6: /* UndefinedOpcode */
            CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 7: /* SAR_RM16_CL */
            CPU_TRACE_BLOCK_BEGIN("SAR_RM16_CL");
            CPU_TRACE_CHECK_RETURN(_a_sar(context, instruction_state.data.crm, cpu_state.data.cl, 16));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_shift(context, 2, cpu_state.data.cl));
    }
    CPU_TRACE_CALL_END;
}
static void AAM(x86_core_cpu_execution_context *context)
{
    lib_u8 base;
    CPU_TRACE_CALL_BEGIN("AAM");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    }
    base = X86_CPU_MASK_U8(instruction_state.data.cimm);
    if (base == 0)
    {
        CPU_TRACE_CHECK_RETURN(_SetExcept_DE(0));
        return;
    }
    _WriteGPR8(ah) = cpu_state.data.al / base;
    _WriteGPR8(al) = cpu_state.data.al % base;
    instruction_state.data.bit = 0x08;
    instruction_state.data.result = X86_CPU_MASK_U8(cpu_state.data.al);
    CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, AAM_FLAG));
    instruction_state.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF);
    CPU_TRACE_CALL_END;
}
static void AAD(x86_core_cpu_execution_context *context)
{
    lib_u8 base;
    CPU_TRACE_CALL_BEGIN("AAD");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    }
    base = X86_CPU_MASK_U8(instruction_state.data.cimm);
    _WriteGPR8(al) = X86_CPU_MASK_U8(cpu_state.data.al + (cpu_state.data.ah * base));
    _WriteGPR8(ah) = 0x00;
    instruction_state.data.bit = 0x08;
    instruction_state.data.result = X86_CPU_MASK_U8(cpu_state.data.al);
    CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, AAD_FLAG));
    instruction_state.data.udf |= (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF);
    CPU_TRACE_CALL_END;
}
static void SALC(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SALC");
    _adv;
    _WriteGPR8(al) = _GetEFLAGS_CF ? 0xffu : 0u;
    CPU_TRACE_CALL_END;
}
static void XLAT(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("XLAT");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetAddressSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("AddressSize(2)");
            CPU_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, X86_CPU_MASK_U16(cpu_state.data.bx + cpu_state.data.al), X86_CPU_REFERENCE_OF(cpu_state.data.al), 1));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("AddressSize(4)");
            CPU_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, (cpu_state.data.ebx + cpu_state.data.al), X86_CPU_REFERENCE_OF(cpu_state.data.al), 1));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_m_read_logical(context, instruction_state.data.roverds, X86_CPU_MASK_U16(cpu_state.data.bx + cpu_state.data.al), X86_CPU_REFERENCE_OF(cpu_state.data.al), 1));
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
        instruction_state.data.timing_ea_base_mask |= 8u;
    CPU_TRACE_CALL_END;
}
static void LOOPNZ_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("LOOPNZ_REL8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_loopcc(context, (lib_i8)instruction_state.data.cimm, !_GetEFLAGS_ZF));
    CPU_TRACE_CALL_END;
}
static void LOOPZ_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("LOOPZ_REL8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_loopcc(context, (lib_i8)instruction_state.data.cimm, _GetEFLAGS_ZF));
    CPU_TRACE_CALL_END;
}
static void LOOP_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("LOOP_REL8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_loopcc(context, (lib_i8)instruction_state.data.cimm, 1));
    CPU_TRACE_CALL_END;
}
static void JCXZ_REL8(x86_core_cpu_execution_context *context)
{
    lib_u32 cecx = 0x00000000;
    CPU_TRACE_CALL_BEGIN("JCXZ_REL8");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
        switch (_GetAddressSize)
        {
        case 2:
            cecx = cpu_state.data.cx;
            break;
        case 4:
            cecx = cpu_state.data.ecx;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1, !cecx));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
        _e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1, !cpu_state.data.cx);
    }
    CPU_TRACE_CALL_END;
}
static void IN_AL_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("IN_AL_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_p_input(context, X86_CPU_MASK_U8(instruction_state.data.cimm), X86_CPU_REFERENCE_OF(cpu_state.data.al), 1));
    CPU_TRACE_CALL_END;
}
static void IN_EAX_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("IN_EAX_I8");
    _new_code_path_;
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_p_input(context, X86_CPU_MASK_U8(instruction_state.data.cimm), X86_CPU_REFERENCE_OF(cpu_state.data.ax), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_p_input(context, X86_CPU_MASK_U8(instruction_state.data.cimm), X86_CPU_REFERENCE_OF(cpu_state.data.eax), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
        CPU_TRACE_CHECK_RETURN(_p_input(context, X86_CPU_MASK_U8(instruction_state.data.cimm), X86_CPU_REFERENCE_OF(cpu_state.data.ax), 2));
    }
    CPU_TRACE_CALL_END;
}
static void OUT_I8_AL(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("OUT_I8_AL");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_p_output(context, X86_CPU_MASK_U8(instruction_state.data.cimm), X86_CPU_REFERENCE_OF(cpu_state.data.al), 1));
    CPU_TRACE_CALL_END;
}
static void OUT_I8_EAX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("OUT_I8_EAX");
    _new_code_path_;
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_p_output(context, X86_CPU_MASK_U8(instruction_state.data.cimm), X86_CPU_REFERENCE_OF(cpu_state.data.ax), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_p_output(context, X86_CPU_MASK_U8(instruction_state.data.cimm), X86_CPU_REFERENCE_OF(cpu_state.data.eax), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
        CPU_TRACE_CHECK_RETURN(_p_output(context,
            X86_CPU_MASK_U8(instruction_state.data.cimm),
            X86_CPU_REFERENCE_OF(cpu_state.data.ax), 2));
    }
    CPU_TRACE_CALL_END;
}
static void CALL_REL32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("CALL_REL32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
            CPU_TRACE_CHECK_RETURN(_e_call_near(context, X86_CPU_MASK_U16(cpu_state.data.ip + (lib_i16)instruction_state.data.cimm), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 4));
            CPU_TRACE_CHECK_RETURN(_e_call_near(context, X86_CPU_MASK_U32(cpu_state.data.eip + (lib_i32)instruction_state.data.cimm), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_BLOCK_BEGIN("OperandSize");
            CPU_TRACE_CHECK_RETURN(_SetExcept_CE(_GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        CPU_TRACE_CHECK_RETURN(_e_call_near(context, X86_CPU_MASK_U16(cpu_state.data.ip + (lib_i16)instruction_state.data.cimm), 2));
    }
    CPU_TRACE_CALL_END;
}
static void JMP_REL32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JMP_REL32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), _GetOperandSize, 1));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        _e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 2, 1);
    }
    CPU_TRACE_CALL_END;
}
static void JMP_PTR16_32(x86_core_cpu_execution_context *context)
{
    lib_u16 newcs = 0x0000;
    lib_u32 neweip = 0x00000000;
    CPU_TRACE_CALL_BEGIN("JMP_PTR16_32");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
            neweip = (lib_u16)instruction_state.data.cimm;
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
            newcs = (lib_u16)instruction_state.data.cimm;
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            _new_code_path_;
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 4));
            neweip = (lib_u32)instruction_state.data.cimm;
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
            newcs = (lib_u16)instruction_state.data.cimm;
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        CPU_TRACE_CHECK_RETURN(_e_jmp_far(context, newcs, neweip, _GetOperandSize));
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        neweip = (lib_u16)instruction_state.data.cimm;
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
        newcs = (lib_u16)instruction_state.data.cimm;
        CPU_TRACE_CHECK_RETURN(_e_jmp_far(context, newcs, neweip, 2));
    }
    CPU_TRACE_CALL_END;
}
static void JMP_REL8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JMP_REL8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), 1, 1));
    CPU_TRACE_CALL_END;
}
static void IN_AL_DX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("IN_AL_DX");
    _adv;
    CPU_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, X86_CPU_REFERENCE_OF(cpu_state.data.al), 1));
    CPU_TRACE_CALL_END;
}
static void IN_EAX_DX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("IN_EAX_DX");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, X86_CPU_REFERENCE_OF(cpu_state.data.ax), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, X86_CPU_REFERENCE_OF(cpu_state.data.eax), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_p_input(context, cpu_state.data.dx, X86_CPU_REFERENCE_OF(cpu_state.data.ax), 2));
    }
    CPU_TRACE_CALL_END;
}
static void OUT_DX_AL(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("OUT_DX_AL");
    _adv;
    CPU_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx, X86_CPU_REFERENCE_OF(cpu_state.data.al), 1));
    CPU_TRACE_CALL_END;
}
static void OUT_DX_EAX(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("OUT_DX_EAX");
    _new_code_path_;
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        switch (_GetOperandSize)
        {
        case 2:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
            CPU_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx, X86_CPU_REFERENCE_OF(cpu_state.data.ax), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4:
            CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
            CPU_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx, X86_CPU_REFERENCE_OF(cpu_state.data.eax), 4));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_p_output(context, cpu_state.data.dx,
            X86_CPU_REFERENCE_OF(cpu_state.data.ax), 2));
    }
    CPU_TRACE_CALL_END;
}
static void PREFIX_LOCK(x86_core_cpu_execution_context *context)
{
    lib_u8 opcode = 0x00;
    lib_u8 modrm = 0x00;
    lib_u8 opcode_0f = 0x00;
    lib_u32 ceip = cpu_state.data.eip;
    CPU_TRACE_CALL_BEGIN("PREFIX_LOCK");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        do
        {
            CPU_TRACE_CHECK_RETURN(_s_read_cs(context, ceip, X86_CPU_REFERENCE_OF(opcode), 1));
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
            instruction_state.data.flagLock = LIB_TRUE;
            break;
        case 0xfe:
            CPU_TRACE_BLOCK_BEGIN("opcode(0xfe)");
            CPU_TRACE_CHECK_RETURN(_s_read_cs(context, ceip,
                X86_CPU_REFERENCE_OF(modrm), 1));
            if (_GetModRM_REG(modrm) <= 1u)
                instruction_state.data.flagLock = LIB_TRUE;
            else
                CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
            CPU_TRACE_BLOCK_END;
            break;
        case 0x80:
        case 0x81:
        case 0x83:
            CPU_TRACE_BLOCK_BEGIN("opcode(0x80/0x81/0x83)");
            CPU_TRACE_CHECK_RETURN(_s_read_cs(context, ceip, X86_CPU_REFERENCE_OF(modrm), 1));
            if (_GetModRM_REG(modrm) != 7)
                instruction_state.data.flagLock = LIB_TRUE;
            else
            {
                CPU_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
                CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_BLOCK_END;
            break;
        case 0x0f:
            CPU_TRACE_BLOCK_BEGIN("opcode(0f)");
            CPU_TRACE_CHECK_RETURN(_s_read_cs(context, ceip, X86_CPU_REFERENCE_OF(opcode_0f), 1));
            switch (opcode_0f)
            {
            case 0xab: /* BTS */
            case 0xb3: /* BTR */
            case 0xbb: /* BTC */
            case 0xb0: /* CMPXCHG */
            case 0xb1:
            case 0xc0: /* XADD */
            case 0xc1:
                instruction_state.data.flagLock = LIB_TRUE;
                break;
            case 0xba:
                CPU_TRACE_BLOCK_BEGIN("opcode_0f(0xba)");
                CPU_TRACE_CHECK_RETURN(_s_read_cs(context, ceip + 1u,
                    X86_CPU_REFERENCE_OF(modrm), 1));
                if (_GetModRM_REG(modrm) >= 5u)
                    instruction_state.data.flagLock = LIB_TRUE;
                else
                    CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
                CPU_TRACE_BLOCK_END;
                break;
            default:
                CPU_TRACE_BLOCK_BEGIN("opcode_0f");
                CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
                CPU_TRACE_BLOCK_END;
                break;
            }
            CPU_TRACE_BLOCK_END;
            break;
        case 0xf6:
        case 0xf7:
            CPU_TRACE_BLOCK_BEGIN("opcode(0xf6/0xf7)");
            CPU_TRACE_CHECK_RETURN(_s_read_cs(context, ceip,
                X86_CPU_REFERENCE_OF(modrm), 1));
            if (_GetModRM_REG(modrm) == 2u || _GetModRM_REG(modrm) == 3u)
                instruction_state.data.flagLock = LIB_TRUE;
            else
                CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
            CPU_TRACE_BLOCK_END;
            break;
        case 0xff:
            CPU_TRACE_BLOCK_BEGIN("opcode(0xff)");
            CPU_TRACE_CHECK_RETURN(_s_read_cs(context, ceip,
                X86_CPU_REFERENCE_OF(modrm), 1));
            if (_GetModRM_REG(modrm) <= 1u)
                instruction_state.data.flagLock = LIB_TRUE;
            else
                CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_BLOCK_BEGIN("opcode");
            CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
            CPU_TRACE_BLOCK_END;
            break;
        }
    }
    else
    {
        /* Before the 80386, LOCK is a bus prefix for the following valid
         * instruction.  The 80286 additionally treats it as IOPL-sensitive
         * in protected mode; it does not use the later 80386 opcode whitelist.
         */
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 &&
            _IsProtected && _GetCPL > (lib_u8)_GetEFLAGS_IOPL)
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        else
            _adv;
    }
    CPU_TRACE_CALL_END;
}
static void INT1(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INT1");
    _adv;
    /* Presence and DR6 cause are independent: INT1 adds no cause bits. */
    context->debug_trap_pending = LIB_TRUE;
    CPU_TRACE_CALL_END;
}
static void PREFIX_REPNZ(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PREFIX_REPNZ");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        instruction_state.data.prefix_rep = PREFIX_REP_REPZNZ;
    }
    else
    {
        _adv;
        instruction_state.data.prefix_rep = PREFIX_REP_REPZNZ;
    }
    CPU_TRACE_CALL_END;
}
static void PREFIX_REPZ(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("PREFIX_REPZ");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        instruction_state.data.prefix_rep = PREFIX_REP_REPZ;
    }
    else
    {
        _adv;
        instruction_state.data.prefix_rep = PREFIX_REP_REPZ;
    }
    CPU_TRACE_CALL_END;
}
static void HLT(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("HLT");
    _new_code_path_;
    if (_GetCR0_PE && _GetCPL)
    {
        CPU_TRACE_BLOCK_BEGIN("CR0_PE(1),CPL(!0)");
        _SetExcept_GP(0);
        CPU_TRACE_BLOCK_END;
    }
    _adv;
    cpu_state.data.flagHalt = LIB_TRUE;
    if (!instruction_state.data.except && context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
        context->special_cycle = X86_CORE_CPU_SPECIAL_HALT;
    CPU_TRACE_CALL_END;
}
static void CMC(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("CMC");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        cpu_state.data.eflags ^= VCPU_EFLAGS_CF;
    }
    else
    {
        _adv;
        cpu_state.data.eflags ^= VCPU_EFLAGS_CF;
    }
    CPU_TRACE_CALL_END;
}
static void INS_F6(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INS_F6");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    if (instruction_state.data.cr == 2u || instruction_state.data.cr == 3u)
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 1, LIB_FALSE));
    else
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    switch (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        instruction_state.data.cr == 1u ? 0u : instruction_state.data.cr)
    {
    case 0: /* TEST_RM8_I8 */
        CPU_TRACE_BLOCK_BEGIN("TEST_RM8_I8");
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
        CPU_TRACE_CHECK_RETURN(_a_test(context, instruction_state.data.crm, instruction_state.data.cimm, 8));
        CPU_TRACE_BLOCK_END;
        break;
    case 1: /* UndefinedOpcode */
        CPU_TRACE_BLOCK_BEGIN("ModRM_REG(1)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 2: /* NOT_RM8 */
        CPU_TRACE_BLOCK_BEGIN("NOT_RM8");
        CPU_TRACE_CHECK_RETURN(_a_not(context, instruction_state.data.crm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 3: /* NEG_RM8 */
        CPU_TRACE_BLOCK_BEGIN("NEG_RM8");
        CPU_TRACE_CHECK_RETURN(_a_neg(context, instruction_state.data.crm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 4: /* MUL_RM8 */
        CPU_TRACE_BLOCK_BEGIN("MUL_RM8");
        CPU_TRACE_CHECK_RETURN(_a_mul(context, instruction_state.data.crm, 8));
        CPU_TRACE_BLOCK_END;
        break;
    case 5: /* IMUL_RM8 */
        CPU_TRACE_BLOCK_BEGIN("IMUL_RM8");
        CPU_TRACE_CHECK_RETURN(_a_imul(context, instruction_state.data.crm, 8));
        CPU_TRACE_BLOCK_END;
        break;
    case 6: /* DIV_RM8 */
        CPU_TRACE_BLOCK_BEGIN("DIV_RM8");
        CPU_TRACE_CHECK_RETURN(_a_div(context, instruction_state.data.crm, 8));
        CPU_TRACE_BLOCK_END;
        break;
    case 7: /* IDIV_RM8 */
        CPU_TRACE_BLOCK_BEGIN("IDIV_RM8");
        _new_code_path_;
        CPU_TRACE_CHECK_RETURN(_a_idiv(context, instruction_state.data.crm, 8));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void INS_F7(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INS_F7");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
        if (instruction_state.data.cr == 2u || instruction_state.data.cr == 3u)
            CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
        else
            CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
        switch (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
            instruction_state.data.cr == 1u ? 0u : instruction_state.data.cr)
        {
        case 0: /* TEST_RM32_I32 */
            CPU_TRACE_BLOCK_BEGIN("TEST_RM32_I32");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
            CPU_TRACE_CHECK_RETURN(_a_test(context, instruction_state.data.crm, instruction_state.data.cimm, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 1: /* UndefinedOpcode */
            CPU_TRACE_BLOCK_BEGIN("ModRM_REG(1)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 2: /* NOT_RM32 */
            CPU_TRACE_BLOCK_BEGIN("NOT_RM32");
            CPU_TRACE_CHECK_RETURN(_a_not(context, instruction_state.data.crm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 3: /* NEG_RM32 */
            CPU_TRACE_BLOCK_BEGIN("NEG_RM32");
            CPU_TRACE_CHECK_RETURN(_a_neg(context, instruction_state.data.crm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 4: /* MUL_RM32 */
            CPU_TRACE_BLOCK_BEGIN("MUL_RM32");
            CPU_TRACE_CHECK_RETURN(_a_mul(context, instruction_state.data.crm, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 5: /* IMUL_RM32 */
            CPU_TRACE_BLOCK_BEGIN("IMUL_RM32");
            CPU_TRACE_CHECK_RETURN(_a_imul(context, instruction_state.data.crm, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 6: /* DIV_RM32 */
            CPU_TRACE_BLOCK_BEGIN("DIV_RM32");
            CPU_TRACE_CHECK_RETURN(_a_div(context, instruction_state.data.crm, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        case 7: /* IDIV_RM32 */
            CPU_TRACE_BLOCK_BEGIN("IDIV_RM32");
            CPU_TRACE_CHECK_RETURN(_a_idiv(context, instruction_state.data.crm, _GetOperandSize * 8));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        if (instruction_state.data.cr == 2u || instruction_state.data.cr == 3u)
            CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 2, LIB_FALSE));
        else
            CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        switch (instruction_state.data.cr)
        {
        case 0: /* TEST_RM16_I16 */
            CPU_TRACE_BLOCK_BEGIN("TEST_RM16_I16");
            CPU_TRACE_CHECK_RETURN(_d_imm(context, 2));
            CPU_TRACE_CHECK_RETURN(_a_test(context, instruction_state.data.crm, instruction_state.data.cimm, 16));
            CPU_TRACE_BLOCK_END;
            break;
        case 1: /* UndefinedOpcode */
            CPU_TRACE_BLOCK_BEGIN("ModRM_REG(1)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 2: /* NOT_RM16 */
            CPU_TRACE_BLOCK_BEGIN("NOT_RM16");
            CPU_TRACE_CHECK_RETURN(_a_not(context, instruction_state.data.crm, 16));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 3: /* NEG_RM16 */
            CPU_TRACE_BLOCK_BEGIN("NEG_RM16");
            CPU_TRACE_CHECK_RETURN(_a_neg(context, instruction_state.data.crm, 16));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4: /* MUL_RM16 */
            CPU_TRACE_BLOCK_BEGIN("MUL_RM16");
            CPU_TRACE_CHECK_RETURN(_a_mul(context, instruction_state.data.crm, 16));
            CPU_TRACE_BLOCK_END;
            break;
        case 5: /* IMUL_RM16 */
            CPU_TRACE_BLOCK_BEGIN("IMUL_RM16");
            CPU_TRACE_CHECK_RETURN(_a_imul(context, instruction_state.data.crm, 16));
            CPU_TRACE_BLOCK_END;
            break;
        case 6: /* DIV_RM16 */
            CPU_TRACE_BLOCK_BEGIN("DIV_RM16");
            CPU_TRACE_CHECK_RETURN(_a_div(context, instruction_state.data.crm, 16));
            CPU_TRACE_BLOCK_END;
            break;
        case 7: /* IDIV_RM16 */
            CPU_TRACE_BLOCK_BEGIN("IDIV_RM16");
            CPU_TRACE_CHECK_RETURN(_a_idiv(context, instruction_state.data.crm, 16));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    CPU_TRACE_CALL_END;
}
static void CLC(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("CLC");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        _ClrEFLAGS_CF;
    }
    else
    {
        _adv;
        _ClrEFLAGS_CF;
    }
    CPU_TRACE_CALL_END;
}
static void STC(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("STC");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        _SetEFLAGS_CF;
    }
    else
    {
        _adv;
        _SetEFLAGS_CF;
    }
    CPU_TRACE_CALL_END;
}
static void CLI(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("CLI");
    if (cpu_profile_has_protected_architecture(context->cpu_profile))
    {
        _adv;
        if (!_GetCR0_PE)
            _ClrEFLAGS_IF;
        else
        {
            CPU_TRACE_BLOCK_BEGIN("CR0_PE(1)");
            _new_code_path_;
            if (!_GetEFLAGS_VM)
            {
                CPU_TRACE_BLOCK_BEGIN("EFLAGS_VM(0)");
                if (_GetCPL <= (lib_u8)(_GetEFLAGS_IOPL))
                    _ClrEFLAGS_IF;
                else
                    CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                CPU_TRACE_BLOCK_END;
            }
            else
            {
                CPU_TRACE_BLOCK_BEGIN("EFLAGS_VM(1)");
                if (_GetEFLAGS_IOPL == 3)
                    _ClrEFLAGS_IF;
                else if (_e_vm86_interrupts_virtualized(context))
                    X86_CPU_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_VIF);
                else
                    CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_BLOCK_END;
        }
    }
    else
    {
        _adv;
        _ClrEFLAGS_IF;
    }
    CPU_TRACE_CALL_END;
}
static void STI(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("STI");
    if (cpu_profile_has_protected_architecture(context->cpu_profile))
    {
        _adv;
        if (!_GetCR0_PE)
            _SetEFLAGS_IF;
        else
        {
            CPU_TRACE_BLOCK_BEGIN("CR0_PE(1)");
            _new_code_path_;
            if (!_GetEFLAGS_VM)
            {
                CPU_TRACE_BLOCK_BEGIN("EFLAGS_VM(0)");
                if (_GetCPL <= (lib_u8)(_GetEFLAGS_IOPL))
                    _SetEFLAGS_IF;
                else
                    CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                CPU_TRACE_BLOCK_END;
            }
            else
            {
                CPU_TRACE_BLOCK_BEGIN("EFLAGS_VM(1)");
                if (_GetEFLAGS_IOPL == 3)
                    _SetEFLAGS_IF;
                else if (_e_vm86_interrupts_virtualized(context)) {
                    if (_GetEFLAGS_VIP) CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                    X86_CPU_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_VIF);
                }
                else
                    CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_BLOCK_END;
        }
    }
    else
    {
        _adv;
        _SetEFLAGS_IF;
    }
    if (!_e_vm86_interrupts_virtualized(context))
        context->interrupt_shadow = CPU_INTERRUPT_SHADOW_INTR;
    CPU_TRACE_CALL_END;
}
static void CLD(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("CLD");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        _ClrEFLAGS_DF;
    }
    else
    {
        _adv;
        _ClrEFLAGS_DF;
    }
    CPU_TRACE_CALL_END;
}
static void STD(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("STD");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        _SetEFLAGS_DF;
    }
    else
    {
        _adv;
        _SetEFLAGS_DF;
    }
    CPU_TRACE_CALL_END;
}
static void INS_FE(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INS_FE");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    if (instruction_state.data.cr < 2u)
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 1, LIB_FALSE));
    switch (instruction_state.data.cr)
    {
    case 0: /* INC_RM8 */
        CPU_TRACE_BLOCK_BEGIN("INC_RM8");
        CPU_TRACE_CHECK_RETURN(_a_inc(context, instruction_state.data.crm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 1: /* DEC_RM8 */
        CPU_TRACE_BLOCK_BEGIN("DEC_RM8");
        CPU_TRACE_CHECK_RETURN(_a_dec(context, instruction_state.data.crm, 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
        CPU_TRACE_BLOCK_END;
        break;
    case 2:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 3:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 5:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 6:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 7:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void INS_FF(x86_core_cpu_execution_context *context)
{
    lib_u8 modrm;
    lib_u16 newcs;
    lib_u32 neweip;
    CPU_TRACE_CALL_BEGIN("INS_FF");
    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, X86_CPU_REFERENCE_OF(modrm), 1));
        switch (_GetModRM_REG(modrm))
        {
        case 0: /* INC_RM32 */
            CPU_TRACE_BLOCK_BEGIN("INC_RM32");
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
            CPU_TRACE_CHECK_RETURN(_a_inc(context, instruction_state.data.crm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 1: /* DEC_RM32 */
            CPU_TRACE_BLOCK_BEGIN("DEC_RM32");
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
            CPU_TRACE_CHECK_RETURN(_a_dec(context, instruction_state.data.crm, _GetOperandSize * 8));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 2: /* CALL_RM32 */
            CPU_TRACE_BLOCK_BEGIN("CALL_RM32");
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
            neweip = X86_CPU_MASK_U32(instruction_state.data.crm);
            CPU_TRACE_CHECK_RETURN(_e_call_near(context, neweip, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 3: /* CALL_M16_32 */
            CPU_TRACE_BLOCK_BEGIN("CALL_M16_32");
            if (instruction_state.data.flagLock)
                CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            if (!instruction_state.data.flagMem)
            {
                CPU_TRACE_BLOCK_BEGIN("flagMem(0)");
                CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_CHECK_RETURN(_m_read_pair(context, _GetOperandSize, 2));
            neweip = _GetOperandSize == 2 ?
                X86_CPU_MASK_U16(instruction_state.data.crm) :
                X86_CPU_MASK_U32(instruction_state.data.crm);
            newcs = X86_CPU_MASK_U16(instruction_state.data.crm >> (_GetOperandSize * 8u));
            instruction_state.data.crm = newcs;
            CPU_TRACE_CHECK_RETURN(_e_call_far(context, newcs, neweip, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 4: /* JMP_RM32 */
            CPU_TRACE_BLOCK_BEGIN("JMP_RM32");
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
            neweip = X86_CPU_MASK_U32(instruction_state.data.crm);
            CPU_TRACE_CHECK_RETURN(_e_jmp_near(context, neweip, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 5: /* JMP_M16_32 */
            CPU_TRACE_BLOCK_BEGIN("JMP_M16_32");
            if (instruction_state.data.flagLock)
                CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            if (!instruction_state.data.flagMem)
            {
                CPU_TRACE_BLOCK_BEGIN("flagMem(0)");
                CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_CHECK_RETURN(_m_read_pair(context, _GetOperandSize, 2));
            neweip = _GetOperandSize == 2 ?
                X86_CPU_MASK_U16(instruction_state.data.crm) :
                X86_CPU_MASK_U32(instruction_state.data.crm);
            newcs = X86_CPU_MASK_U16(instruction_state.data.crm >> (_GetOperandSize * 8u));
            instruction_state.data.crm = newcs;
            CPU_TRACE_CHECK_RETURN(_e_jmp_far(context, newcs, neweip, _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 6: /* PUSH_RM32 */
            CPU_TRACE_BLOCK_BEGIN("PUSH_RM32");
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, _GetOperandSize));
            if (instruction_state.data.flagLock)
                CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
            CPU_TRACE_BLOCK_END;
            break;
        case 7: /* UndefinedOpcode */
            CPU_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    else
    {
        _adv;
        CPU_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, X86_CPU_REFERENCE_OF(modrm), 1));
        switch (_GetModRM_REG(modrm))
        {
        case 0: /* INC_RM16 */
            CPU_TRACE_BLOCK_BEGIN("INC_RM16");
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 2, LIB_FALSE));
            CPU_TRACE_CHECK_RETURN(_a_inc(context, instruction_state.data.crm, 16));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 1: /* DEC_RM16 */
            CPU_TRACE_BLOCK_BEGIN("DEC_RM16");
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 2, LIB_FALSE));
            CPU_TRACE_CHECK_RETURN(_a_dec(context, instruction_state.data.crm, 16));
            instruction_state.data.crm = instruction_state.data.result;
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 2: /* CALL_RM16 */
            CPU_TRACE_BLOCK_BEGIN("CALL_RM16");
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            neweip = X86_CPU_MASK_U16(instruction_state.data.crm);
            CPU_TRACE_CHECK_RETURN(_e_call_near(context, neweip, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 3: /* CALL_M16_16 */
            CPU_TRACE_BLOCK_BEGIN("CALL_M16_16");
            if (instruction_state.data.flagLock)
                CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            if (!instruction_state.data.flagMem)
            {
                CPU_TRACE_BLOCK_BEGIN("flagMem(0)");
                CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_CHECK_RETURN(_m_read_pair(context, 2, 2));
            neweip = (lib_u16)instruction_state.data.crm;
            newcs = X86_CPU_MASK_U16(instruction_state.data.crm >> (2 * 8u));
            instruction_state.data.crm = newcs;
            CPU_TRACE_CHECK_RETURN(_e_call_far(context, newcs, neweip, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 4: /* JMP_RM16 */
            CPU_TRACE_BLOCK_BEGIN("JMP_RM16");
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            neweip = X86_CPU_MASK_U16(instruction_state.data.crm);
            CPU_TRACE_CHECK_RETURN(_e_jmp_near(context, neweip, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 5: /* JMP_M16_16 */
            CPU_TRACE_BLOCK_BEGIN("JMP_M16_16");
            if (instruction_state.data.flagLock)
                CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            if (!instruction_state.data.flagMem)
            {
                CPU_TRACE_BLOCK_BEGIN("flagMem(0)");
                CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_CHECK_RETURN(_m_read_pair(context, 2, 2));
            neweip = (lib_u16)instruction_state.data.crm;
            newcs = X86_CPU_MASK_U16(instruction_state.data.crm >> (2 * 8u));
            instruction_state.data.crm = newcs;
            CPU_TRACE_CHECK_RETURN(_e_jmp_far(context, newcs, neweip, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 6: /* PUSH_RM16 */
            CPU_TRACE_BLOCK_BEGIN("PUSH_RM16");
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            if (instruction_state.data.flagLock)
                CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(instruction_state.data.crm), 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 7: /* UndefinedOpcode */
            CPU_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
    }
    CPU_TRACE_CALL_END;
}

static void _d_modrm_creg(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("_d_modrm_creg");
    CPU_TRACE_CHECK_RETURN(_kdf_modrm_creg(context));
    if (instruction_state.data.flagMem &&
        !context->cpu_80386_cr_mov_ignores_mod)
    {
        CPU_TRACE_BLOCK_BEGIN("flagMem(1)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        CPU_TRACE_BLOCK_END;
    }
    switch (instruction_state.data.cr)
    {
    case 0:
        instruction_state.data.rr = (lib_uptr)&cpu_state.data.cr0;
        instruction_state.data.cr = cpu_state.data.cr0;
        break;
    case 1:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        CPU_TRACE_BLOCK_END;
        break;
    case 2:
        instruction_state.data.rr = (lib_uptr)&cpu_state.data.cr2;
        instruction_state.data.cr = cpu_state.data.cr2;
        break;
    case 3:
        instruction_state.data.rr = (lib_uptr)&cpu_state.data.cr3;
        instruction_state.data.cr = cpu_state.data.cr3;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX)
            CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        instruction_state.data.rr = (lib_uptr)&cpu_state.data.cr4;
        instruction_state.data.cr = cpu_state.data.cr4;
        CPU_TRACE_BLOCK_END;
        break;
    case 5:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        CPU_TRACE_BLOCK_END;
        break;
    case 6:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(6)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        CPU_TRACE_BLOCK_END;
        break;
    case 7:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(7)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _d_modrm_dreg(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("_d_modrm_dreg");
    CPU_TRACE_CHECK_RETURN(_kdf_modrm(context, 0, 4));
    if (instruction_state.data.flagMem)
    {
        CPU_TRACE_BLOCK_BEGIN("flagMem(1)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        CPU_TRACE_BLOCK_END;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        (cpu_state.data.dr7 & VCPU_DR7_GD)) {
        cpu_state.data.dr6 |= VCPU_DR6_BD;
        CPU_TRACE_CHECK_RETURN(x86_core_cpu_execution_raise_exception(
            context, VCPUINS_EXCEPT_DB, 0u));
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        (instruction_state.data.cr == 4u || instruction_state.data.cr == 5u))
        instruction_state.data.cr += 2u;
    switch (instruction_state.data.cr)
    {
    case 0:
        instruction_state.data.rr = (lib_uptr)&cpu_state.data.dr0;
        instruction_state.data.cr = cpu_state.data.dr0;
        break;
    case 1:
        instruction_state.data.rr = (lib_uptr)&cpu_state.data.dr1;
        instruction_state.data.cr = cpu_state.data.dr1;
        break;
    case 2:
        instruction_state.data.rr = (lib_uptr)&cpu_state.data.dr2;
        instruction_state.data.cr = cpu_state.data.dr2;
        break;
    case 3:
        instruction_state.data.rr = (lib_uptr)&cpu_state.data.dr3;
        instruction_state.data.cr = cpu_state.data.dr3;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(4)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        CPU_TRACE_BLOCK_END;
        break;
    case 5:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(5)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        CPU_TRACE_BLOCK_END;
        break;
    case 6:
        instruction_state.data.rr = (lib_uptr)&cpu_state.data.dr6;
        instruction_state.data.cr = cpu_state.data.dr6;
        break;
    case 7:
        instruction_state.data.rr = (lib_uptr)&cpu_state.data.dr7;
        instruction_state.data.cr = cpu_state.data.dr7;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _d_modrm_treg(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("_d_modrm_treg");
    CPU_TRACE_CHECK_RETURN(_kdf_modrm(context, 0, 4));
    if (instruction_state.data.flagMem)
    {
        CPU_TRACE_BLOCK_BEGIN("flagMem(1)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        CPU_TRACE_BLOCK_END;
    }
    switch (instruction_state.data.cr)
    {
    case 0:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(0)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        CPU_TRACE_BLOCK_END;
        break;
    case 1:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        CPU_TRACE_BLOCK_END;
        break;
    case 2:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        CPU_TRACE_BLOCK_END;
        break;
    case 3:
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX)
            CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        instruction_state.data.rr = (lib_uptr)&cpu_state.data.tr3;
        instruction_state.data.cr = cpu_state.data.tr3;
        break;
    case 4:
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX)
            CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        instruction_state.data.rr = (lib_uptr)&cpu_state.data.tr4;
        instruction_state.data.cr = cpu_state.data.tr4;
        break;
    case 5:
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX)
            CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
        instruction_state.data.rr = (lib_uptr)&cpu_state.data.tr5;
        instruction_state.data.cr = cpu_state.data.tr5;
        break;
    case 6:
        instruction_state.data.rr = (lib_uptr)&cpu_state.data.tr6;
        instruction_state.data.cr = cpu_state.data.tr6;
        break;
    case 7:
        instruction_state.data.rr = (lib_uptr)&cpu_state.data.tr7;
        instruction_state.data.cr = cpu_state.data.tr7;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _d_bit_rmimm(x86_core_cpu_execution_context *context, lib_u8 regbyte, lib_u8 rmbyte, lib_u8 write)
{
    /* xrm = actual destination, cimm = (1 << bitoffset) */
    lib_i16 bitoff16 = 0;
    lib_i32 bitoff32 = 0;
    lib_u32 bitoperand = 0;
    CPU_TRACE_CALL_BEGIN("_d_bit_rmimm");
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, regbyte, rmbyte));
    if (!regbyte)
        CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    switch (rmbyte)
    {
    case 2:
        CPU_TRACE_BLOCK_BEGIN("rmbyte(2)");
        if (instruction_state.data.flagMem && regbyte)
        {
            /* valid for btcc_m16_r16 */
            CPU_TRACE_BLOCK_BEGIN("flagMem(1),regbyte(1)");
            bitoff16 = (lib_i16)instruction_state.data.cr;
            if (bitoff16 >= 0)
                instruction_state.data.mrm.offset += 2 * (bitoff16 / 16);
            else
                instruction_state.data.mrm.offset += 2 * ((bitoff16 - 15) / 16);
            bitoperand = ((lib_u16)bitoff16) % 16;
            CPU_TRACE_BLOCK_END;
        }
        else if (regbyte)
        {
            bitoperand = instruction_state.data.cr % 16;
        }
        else
        {
            bitoperand = (X86_CPU_MASK_U16(instruction_state.data.cimm) % 16);
        }
        CPU_TRACE_CHECK_RETURN(instruction_state.data.cimm = X86_CPU_MASK_U16((1u << bitoperand)));
        CPU_TRACE_BLOCK_END;
        break;
    case 4:
        CPU_TRACE_BLOCK_BEGIN("rmbyte(4)");
        if (instruction_state.data.flagMem && regbyte)
        {
            CPU_TRACE_BLOCK_BEGIN("flagMem(1),regbyte(1)");
            bitoff32 = (lib_i32)instruction_state.data.cr;
            if (bitoff32 >= 0)
                instruction_state.data.mrm.offset += 4 * (bitoff32 / 32);
            else
                instruction_state.data.mrm.offset += 4 * (((lib_i64)bitoff32 - 31) / 32);
            bitoperand = ((lib_u32)bitoff32) % 32;
            CPU_TRACE_BLOCK_END;
        }
        else if (regbyte)
        {
            bitoperand = instruction_state.data.cr % 32;
        }
        else
        {
            bitoperand = (X86_CPU_MASK_U32(instruction_state.data.cimm) % 32);
        }
        CPU_TRACE_CHECK_RETURN(instruction_state.data.cimm = X86_CPU_MASK_U32((1u << bitoperand)));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("rmbyte");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(rmbyte));
        CPU_TRACE_BLOCK_END;
        break;
    }
    /* Bit-string displacement is part of EA formation, not operand-span wrap. */
    if (instruction_state.data.flagMem && _GetAddressSize == 2)
        instruction_state.data.mrm.offset =
            X86_CPU_MASK_U16(instruction_state.data.mrm.offset);
    CPU_TRACE_CALL_END;
}

#define SHLD_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)
#define SHRD_FLAG (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)

#define _kac_btcc                                                                                                                     \
    do                                                                                                                                \
    {                                                                                                                                 \
        switch (bit)                                                                                                                  \
        {                                                                                                                             \
        case 16:                                                                                                                      \
            instruction_state.data.opr1 = X86_CPU_MASK_U16(cdest);                                                               \
            break;                                                                                                                    \
        case 32:                                                                                                                      \
            instruction_state.data.opr1 = X86_CPU_MASK_U32(cdest);                                                               \
            break;                                                                                                                    \
        default:                                                                                                                      \
            CPU_TRACE_BLOCK_BEGIN("bit");                                                                                            \
            CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));                                                                              \
            CPU_TRACE_BLOCK_END;                                                                                                     \
            break;                                                                                                                    \
        }                                                                                                                             \
        instruction_state.data.opr2 = bitoperand;                                                                                     \
        instruction_state.data.result = instruction_state.data.opr1;                                                                  \
        X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF, X86_CPU_BIT_IS_SET(instruction_state.data.opr1, instruction_state.data.opr2)); \
    } while (0)

static void _a_bscc(x86_core_cpu_execution_context *context, lib_u64 csrc, lib_u8 bit, lib_u8 forward)
{
    lib_u32 temp;
    CPU_TRACE_CALL_BEGIN("_a_bscc");
    if (forward)
        temp = 0;
    else
        temp = bit - 1;
    switch (bit)
    {
    case 16:
        CPU_TRACE_BLOCK_BEGIN("bit(16)");
        instruction_state.data.opr1 = X86_CPU_MASK_U16(csrc);
        if (!instruction_state.data.opr1)
            _SetEFLAGS_ZF;
        else
        {
            _ClrEFLAGS_ZF;
            while (!X86_CPU_BIT_IS_SET(instruction_state.data.opr1, X86_CPU_MASK_U64(UINT64_C(1) << temp)))
            {
                if (forward)
                    temp++;
                else
                    temp--;
            }
            instruction_state.data.result = X86_CPU_MASK_U16(temp);
        }
        CPU_TRACE_BLOCK_END;
        break;
    case 32:
        CPU_TRACE_BLOCK_BEGIN("bit(32)");
        instruction_state.data.opr1 = X86_CPU_MASK_U32(csrc);
        if (!instruction_state.data.opr1)
            _SetEFLAGS_ZF;
        else
        {
            _ClrEFLAGS_ZF;
            while (!X86_CPU_BIT_IS_SET(instruction_state.data.opr1, X86_CPU_MASK_U64(UINT64_C(1) << temp)))
            {
                if (forward)
                    temp++;
                else
                    temp--;
            }
            instruction_state.data.result = X86_CPU_MASK_U32(temp);
        }
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _a_bt(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u32 bitoperand, lib_u8 bit)
{
    CPU_TRACE_CALL_BEGIN("_a_bt");
    _kac_btcc;
    CPU_TRACE_CALL_END;
}
static void _a_btc(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u32 bitoperand, lib_u8 bit)
{
    CPU_TRACE_CALL_BEGIN("_a_btc");
    _kac_btcc;
    X86_CPU_BIT_MAKE(instruction_state.data.result, instruction_state.data.opr2, !_GetEFLAGS_CF);
    CPU_TRACE_CALL_END;
}
static void _a_btr(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u32 bitoperand, lib_u8 bit)
{
    CPU_TRACE_CALL_BEGIN("_a_btr");
    _kac_btcc;
    X86_CPU_BIT_CLEAR(instruction_state.data.result, instruction_state.data.opr2);
    CPU_TRACE_CALL_END;
}
static void _a_bts(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u32 bitoperand, lib_u8 bit)
{
    CPU_TRACE_CALL_BEGIN("_a_bts");
    _kac_btcc;
    X86_CPU_BIT_SET(instruction_state.data.result, instruction_state.data.opr2);
    CPU_TRACE_CALL_END;
}

static void _a_imul2(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u64 csrc, lib_u8 bit)
{
    lib_u64 temp;
    CPU_TRACE_CALL_BEGIN("_a_imul2");
    switch (bit)
    {
    case 16:
        CPU_TRACE_BLOCK_BEGIN("bit(16+16)");
        _new_code_path_;
        instruction_state.data.bit = 16;
        instruction_state.data.opr1 = X86_CPU_MASK_U16((lib_i16)cdest);
        instruction_state.data.opr2 = X86_CPU_MASK_U16((lib_i16)csrc);
        temp = X86_CPU_MASK_U32((lib_i16)instruction_state.data.opr1 * (lib_i16)instruction_state.data.opr2);
        instruction_state.data.result = X86_CPU_MASK_U16(temp);
        if (X86_CPU_MASK_U32(temp) != X86_CPU_MASK_U32((lib_i16)instruction_state.data.result))
        {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        else
        {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        CPU_TRACE_BLOCK_END;
        break;
    case 32:
        CPU_TRACE_BLOCK_BEGIN("bit(32+32");
        _new_code_path_;
        instruction_state.data.bit = 32;
        instruction_state.data.opr1 = X86_CPU_MASK_U32((lib_i32)cdest);
        instruction_state.data.opr2 = X86_CPU_MASK_U32((lib_i32)csrc);
        temp = X86_CPU_MASK_U64((lib_i64)(lib_i32)instruction_state.data.opr1 *
                                     (lib_i64)(lib_i32)instruction_state.data.opr2);
        instruction_state.data.result = X86_CPU_MASK_U32(temp);
        if (X86_CPU_MASK_U64(temp) != X86_CPU_MASK_U64((lib_i32)instruction_state.data.result))
        {
            _SetEFLAGS_CF;
            _SetEFLAGS_OF;
        }
        else
        {
            _ClrEFLAGS_CF;
            _ClrEFLAGS_OF;
        }
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_BLOCK_BEGIN("bit");
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
        CPU_TRACE_BLOCK_END;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void _a_shld(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u64 csrc, lib_u8 count, lib_u8 bit)
{
    lib_u8 flagcf;
    lib_u8 flagbit;
    lib_i32 i;
    CPU_TRACE_CALL_BEGIN("_a_shld");
    count &= 0x1f;
    if (!count)
    {
        CPU_TRACE_CALL_END;
        return;
    }
    if (count > bit)
    {
        instruction_state.data.bit = bit;
        instruction_state.data.opr1 = bit == 16u ? X86_CPU_MASK_U16(cdest) : X86_CPU_MASK_U32(cdest);
        instruction_state.data.opr2 = bit == 16u ? X86_CPU_MASK_U16(csrc) : X86_CPU_MASK_U32(csrc);
        instruction_state.data.result = instruction_state.data.opr1;
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
            CPU_TRACE_BLOCK_BEGIN("bit(16)");
            instruction_state.data.bit = 16;
            instruction_state.data.opr1 = X86_CPU_MASK_U16(cdest);
            instruction_state.data.opr2 = X86_CPU_MASK_U16(csrc);
            instruction_state.data.result = instruction_state.data.opr1;
            flagcf = !!X86_CPU_GET_MSB_16(instruction_state.data.result);
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                          X86_CPU_BIT_IS_SET(instruction_state.data.result, X86_CPU_MASK_U64(UINT64_C(1) << (bit - count))));
            for (i = (lib_i32)(bit - 1); i >= (lib_i32)count; --i)
            {
                flagbit = X86_CPU_BIT_IS_SET(instruction_state.data.opr1, X86_CPU_MASK_U64(UINT64_C(1) << (i - count)));
                X86_CPU_BIT_MAKE(instruction_state.data.result, X86_CPU_MASK_U64(UINT64_C(1) << i), flagbit);
            }
            for (i = (lib_i32)(count - 1); i >= 0; --i)
            {
                flagbit = X86_CPU_BIT_IS_SET(instruction_state.data.opr2, X86_CPU_MASK_U64(UINT64_C(1) << (i - count + bit)));
                X86_CPU_BIT_MAKE(instruction_state.data.result, X86_CPU_MASK_U64(UINT64_C(1) << i), flagbit);
            }
            if (count == 1)
                X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                              ((!!X86_CPU_GET_MSB_16(instruction_state.data.result)) ^ flagcf));
            else
                instruction_state.data.udf |= VCPU_EFLAGS_OF;
            CPU_TRACE_BLOCK_END;
            break;
        case 32:
            CPU_TRACE_BLOCK_BEGIN("bit(32)");
            instruction_state.data.bit = 32;
            instruction_state.data.opr1 = X86_CPU_MASK_U32(cdest);
            instruction_state.data.opr2 = X86_CPU_MASK_U32(csrc);
            instruction_state.data.result = instruction_state.data.opr1;
            flagcf = !!X86_CPU_GET_MSB_32(instruction_state.data.result);
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                          X86_CPU_BIT_IS_SET(instruction_state.data.result, X86_CPU_MASK_U64(UINT64_C(1) << (bit - count))));
            for (i = (lib_i32)(bit - 1); i >= (lib_i32)count; --i)
            {
                flagbit = X86_CPU_BIT_IS_SET(instruction_state.data.opr1, X86_CPU_MASK_U64(UINT64_C(1) << (i - count)));
                X86_CPU_BIT_MAKE(instruction_state.data.result, X86_CPU_MASK_U64(UINT64_C(1) << i), flagbit);
            }
            for (i = (lib_i32)(count - 1); i >= 0; --i)
            {
                flagbit = X86_CPU_BIT_IS_SET(instruction_state.data.opr2, X86_CPU_MASK_U64(UINT64_C(1) << (i - count + bit)));
                X86_CPU_BIT_MAKE(instruction_state.data.result, X86_CPU_MASK_U64(UINT64_C(1) << i), flagbit);
            }
            if (count == 1)
                X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                              ((!!X86_CPU_GET_MSB_32(instruction_state.data.result)) ^ flagcf));
            else
                instruction_state.data.udf |= VCPU_EFLAGS_OF;
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_BLOCK_BEGIN("bit");
            CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
            CPU_TRACE_BLOCK_END;
            break;
        }
        CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHLD_FLAG));
        instruction_state.data.udf |= VCPU_EFLAGS_AF;
        if (count == bit)
            instruction_state.data.udf |= VCPU_EFLAGS_OF | VCPU_EFLAGS_SF |
                VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF;
    }
    CPU_TRACE_CALL_END;
}
static void _a_shrd(x86_core_cpu_execution_context *context, lib_u64 cdest, lib_u64 csrc, lib_u8 count, lib_u8 bit)
{
    lib_u8 flagcf;
    lib_u8 flagbit;
    lib_i32 i;
    CPU_TRACE_CALL_BEGIN("_a_shrd");
    count &= 0x1f;
    if (!count)
    {
        CPU_TRACE_CALL_END;
        return;
    }
    if (count > bit)
    {
        instruction_state.data.bit = bit;
        instruction_state.data.opr1 = bit == 16u ? X86_CPU_MASK_U16(cdest) : X86_CPU_MASK_U32(cdest);
        instruction_state.data.opr2 = bit == 16u ? X86_CPU_MASK_U16(csrc) : X86_CPU_MASK_U32(csrc);
        instruction_state.data.result = instruction_state.data.opr1;
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
            CPU_TRACE_BLOCK_BEGIN("bit(16)");
            instruction_state.data.bit = 16;
            instruction_state.data.opr1 = X86_CPU_MASK_U16(cdest);
            instruction_state.data.opr2 = X86_CPU_MASK_U16(csrc);
            instruction_state.data.result = instruction_state.data.opr1;
            flagcf = !!X86_CPU_GET_MSB_16(instruction_state.data.result);
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                          X86_CPU_BIT_IS_SET(instruction_state.data.result, X86_CPU_MASK_U64(UINT64_C(1) << (count - 1))));
            for (i = 0; i <= (lib_i32)(bit - count - 1); ++i)
            {
                flagbit = X86_CPU_BIT_IS_SET(instruction_state.data.opr1, X86_CPU_MASK_U64(UINT64_C(1) << (i + count)));
                X86_CPU_BIT_MAKE(instruction_state.data.result, X86_CPU_MASK_U64(UINT64_C(1) << i), flagbit);
            }
            for (i = (lib_i32)(bit - count); i <= (lib_i32)(bit - 1); ++i)
            {
                flagbit = X86_CPU_BIT_IS_SET(instruction_state.data.opr2, X86_CPU_MASK_U64(UINT64_C(1) << (i + count - bit)));
                X86_CPU_BIT_MAKE(instruction_state.data.result, X86_CPU_MASK_U64(UINT64_C(1) << i), flagbit);
            }
            if (count == 1)
                X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                              ((!!X86_CPU_GET_MSB_16(instruction_state.data.result)) ^ flagcf));
            else
                instruction_state.data.udf |= VCPU_EFLAGS_OF;
            CPU_TRACE_BLOCK_END;
            break;
        case 32:
            CPU_TRACE_BLOCK_BEGIN("bit(32)");
            instruction_state.data.bit = 32;
            instruction_state.data.opr1 = X86_CPU_MASK_U32(cdest);
            instruction_state.data.opr2 = X86_CPU_MASK_U32(csrc);
            instruction_state.data.result = instruction_state.data.opr1;
            flagcf = !!X86_CPU_GET_MSB_32(instruction_state.data.result);
            X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_CF,
                          X86_CPU_BIT_IS_SET(instruction_state.data.result, X86_CPU_MASK_U64(UINT64_C(1) << (count - 1))));
            for (i = 0; i <= (lib_i32)(bit - count - 1); ++i)
            {
                flagbit = X86_CPU_BIT_IS_SET(instruction_state.data.opr1, X86_CPU_MASK_U64(UINT64_C(1) << (i + count)));
                X86_CPU_BIT_MAKE(instruction_state.data.result, X86_CPU_MASK_U64(UINT64_C(1) << i), flagbit);
            }
            for (i = (lib_i32)(bit - count); i <= (lib_i32)(bit - 1); ++i)
            {
                flagbit = X86_CPU_BIT_IS_SET(instruction_state.data.opr2, X86_CPU_MASK_U64(UINT64_C(1) << (i + count - bit)));
                X86_CPU_BIT_MAKE(instruction_state.data.result, X86_CPU_MASK_U64(UINT64_C(1) << i), flagbit);
            }
            if (count == 1)
                X86_CPU_BIT_MAKE(cpu_state.data.eflags, VCPU_EFLAGS_OF,
                              ((!!X86_CPU_GET_MSB_32(instruction_state.data.result)) ^ flagcf));
            else
                instruction_state.data.udf |= VCPU_EFLAGS_OF;
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_BLOCK_BEGIN("bit");
            CPU_TRACE_CHECK_RETURN(_SetExcept_CE(bit));
            CPU_TRACE_BLOCK_END;
            break;
        }
        CPU_TRACE_CHECK_RETURN(_kaf_set_flags(context, SHRD_FLAG));
        instruction_state.data.udf |= VCPU_EFLAGS_AF;
        if (count == bit)
            instruction_state.data.udf |= VCPU_EFLAGS_OF | VCPU_EFLAGS_SF |
                VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF;
    }
    CPU_TRACE_CALL_END;
}

static void _m_setcc_rm(x86_core_cpu_execution_context *context, lib_u8 condition)
{
    CPU_TRACE_CALL_BEGIN("_m_setcc_rm");
    instruction_state.data.result = X86_CPU_MASK_U8(!!condition);
    instruction_state.data.crm = instruction_state.data.result;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    CPU_TRACE_CALL_END;
}

static void INS_0F_00(x86_core_cpu_execution_context *context)
{
    lib_u8 modrm;
    lib_u64 descriptor;
    CPU_TRACE_CALL_BEGIN("INS_0F_00");
    _adv;
    if (_IsProtected)
    {
        CPU_TRACE_BLOCK_BEGIN("Protected");
        CPU_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, X86_CPU_REFERENCE_OF(modrm), 1));
        switch (_GetModRM_REG(modrm))
        {
        case 0: /* SLDT_RM16 */
            CPU_TRACE_BLOCK_BEGIN("SLDT_RM16");
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            instruction_state.data.crm = X86_CPU_MASK_U16(cpu_state.data.ldtr.selector);
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 1: /* STR_RM16 */
            CPU_TRACE_BLOCK_BEGIN("STR_RM16");
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            instruction_state.data.crm = X86_CPU_MASK_U16(cpu_state.data.tr.selector);
            CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
            CPU_TRACE_BLOCK_END;
            break;
        case 2: /* LLDT_RM16 */
            CPU_TRACE_BLOCK_BEGIN("LLDT_RM16");
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            if (_GetCPL) CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            CPU_TRACE_CHECK_RETURN(_s_load_ldtr(context, X86_CPU_MASK_U16(instruction_state.data.crm)));
            CPU_TRACE_BLOCK_END;
            break;
        case 3: /* LTR_RM16 */
            CPU_TRACE_BLOCK_BEGIN("LTR_RM16");
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            if (_GetCPL) CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
            CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            CPU_TRACE_CHECK_RETURN(_s_load_tr(context, X86_CPU_MASK_U16(instruction_state.data.crm)));
            CPU_TRACE_BLOCK_END;
            break;
        case 4: /* VERR_RM16 */
            CPU_TRACE_BLOCK_BEGIN("VERR_RM16");
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            if (_s_check_selector(context, X86_CPU_MASK_U16(instruction_state.data.crm)))
            {
                _ClrEFLAGS_ZF;
            }
            else
            {
                CPU_TRACE_BLOCK_BEGIN("selector(valid)");
                CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, X86_CPU_MASK_U16(instruction_state.data.crm), X86_CPU_REFERENCE_OF(descriptor)));
                if (_IsDescSys(descriptor) ||
                    (!_IsDescCodeConform(descriptor) &&
                     (_GetCPL > _GetDesc_DPL(descriptor) ||
                      _GetSelector_RPL(X86_CPU_MASK_U16(instruction_state.data.crm)) > _GetDesc_DPL(descriptor))))
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
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_BLOCK_END;
            break;
        case 5: /* VERW_RM16 */
            CPU_TRACE_BLOCK_BEGIN("VERW_RM16");
            CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
            CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
            if (_s_check_selector(context, X86_CPU_MASK_U16(instruction_state.data.crm)))
            {
                _ClrEFLAGS_ZF;
            }
            else
            {
                CPU_TRACE_BLOCK_BEGIN("selector(valid)");
                CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, X86_CPU_MASK_U16(instruction_state.data.crm), X86_CPU_REFERENCE_OF(descriptor)));
                if (_IsDescSys(descriptor) ||
                    (!_IsDescCodeConform(descriptor) &&
                     (_GetCPL > _GetDesc_DPL(descriptor) ||
                      _GetSelector_RPL(X86_CPU_MASK_U16(instruction_state.data.crm)) > _GetDesc_DPL(descriptor))))
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
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_BLOCK_END;
            break;
        case 6:
            CPU_TRACE_BLOCK_BEGIN("ModRM_REG(6)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        case 7:
            CPU_TRACE_BLOCK_BEGIN("ModRM_REG(7)");
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
            CPU_TRACE_BLOCK_END;
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        CPU_TRACE_BLOCK_END;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("!Protected");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
static void INS_0F_01(x86_core_cpu_execution_context *context)
{
    lib_u8 modrm;
    lib_u16 limit;
    lib_u32 base;
    CPU_TRACE_CALL_BEGIN("INS_0F_01");
    _adv;
    CPU_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, X86_CPU_REFERENCE_OF(modrm), 1));
    switch (_GetModRM_REG(modrm))
    {
    case 0: /* SGDT_M32_16 */
        CPU_TRACE_BLOCK_BEGIN("SGDT_M32_16");
        CPU_TRACE_CHECK_RETURN(_d_modrm_table_memory(context, modrm));
        CPU_TRACE_CHECK_RETURN(_m_write_table_pseudo_descriptor(context,
            cpu_state.data.gdtr.limit, _GetOperandSize == 2 ?
            X86_CPU_MASK_U24(cpu_state.data.gdtr.base) :
            cpu_state.data.gdtr.base));
        CPU_TRACE_BLOCK_END;
        break;
    case 1: /* SIDT_M32_16 */
        CPU_TRACE_BLOCK_BEGIN("SIDT_M32_16");
        CPU_TRACE_CHECK_RETURN(_d_modrm_table_memory(context, modrm));
        CPU_TRACE_CHECK_RETURN(_m_write_table_pseudo_descriptor(context,
            cpu_state.data.idtr.limit, _GetOperandSize == 2 ?
            X86_CPU_MASK_U24(cpu_state.data.idtr.base) :
            cpu_state.data.idtr.base));
        CPU_TRACE_BLOCK_END;
        break;
    case 2: /* LGDT_M32_16 */
        CPU_TRACE_BLOCK_BEGIN("LGDT_M32_16");
        if (_GetCPL)
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        CPU_TRACE_CHECK_RETURN(_d_modrm_table_memory(context, modrm));
        CPU_TRACE_CHECK_RETURN(_m_read_pair(context, 2, 4));
        limit = X86_CPU_MASK_U16(instruction_state.data.crm);
        switch (_GetOperandSize)
        {
        case 2:
            base = X86_CPU_MASK_U24(instruction_state.data.crm >> 16u);
            break;
        case 4:
            base = X86_CPU_MASK_U32(instruction_state.data.crm >> 16u);
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        CPU_TRACE_CHECK_RETURN(_s_load_gdtr(context, base, limit, _GetOperandSize));
        CPU_TRACE_BLOCK_END;
        break;
    case 3: /* LIDT_M32_16 */
        CPU_TRACE_BLOCK_BEGIN("LIDT_M32_16");
        if (_GetCPL)
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        CPU_TRACE_CHECK_RETURN(_d_modrm_table_memory(context, modrm));
        CPU_TRACE_CHECK_RETURN(_m_read_pair(context, 2, 4));
        limit = X86_CPU_MASK_U16(instruction_state.data.crm);
        switch (_GetOperandSize)
        {
        case 2:
            base = X86_CPU_MASK_U24(instruction_state.data.crm >> 16u);
            break;
        case 4:
            base = X86_CPU_MASK_U32(instruction_state.data.crm >> 16u);
            break;
        default:
            CPU_TRACE_IMPOSSIBLE_RETURN;
            break;
        }
        CPU_TRACE_CHECK_RETURN(_s_load_idtr(context, base, limit, _GetOperandSize));
        CPU_TRACE_BLOCK_END;
        break;
    case 4: /* SMSW_RM16 */
        CPU_TRACE_BLOCK_BEGIN("SMSW_RM16");
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        instruction_state.data.crm = X86_CPU_MASK_U16(cpu_state.data.cr0);
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286)
            instruction_state.data.crm = (instruction_state.data.crm & 0x000fu) | 0xfff0u;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 2));
        CPU_TRACE_BLOCK_END;
        break;
    case 5:
        CPU_TRACE_BLOCK_BEGIN("ModRM_REG(5)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 6: /* LMSW_RM16 */
        CPU_TRACE_BLOCK_BEGIN("LMSW_RM16");
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 2));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        CPU_TRACE_CHECK_RETURN(_s_load_cr0_msw(context, X86_CPU_MASK_U16(instruction_state.data.crm)));
        CPU_TRACE_BLOCK_END;
        break;
    case 7: /* INVLPG_M */
        CPU_TRACE_BLOCK_BEGIN("INVLPG_M");
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX ||
            _GetModRM_MOD(modrm) == 3u)
            CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        if (_GetCPL)
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        CPU_TRACE_CHECK_RETURN(_d_modrm_ea(context, 0u, 1u));
        {
            const lib_u32 linear = instruction_state.data.mrm.rsreg->base +
                instruction_state.data.mrm.offset;
            const lib_u8 set = (linear >> 12u) & (X86_CPU_TLB_SETS - 1u);

            for (lib_u8 way = 0u; way < X86_CPU_TLB_WAYS; ++way)
                if (context->tlb[set][way].linear == (linear & VCPU_CR3_BASE)) {
                    if (context->tlb[set][way].valid)
                        instruction_state.data.timing_invlpg_hit = LIB_TRUE;
                    context->tlb[set][way].valid = LIB_FALSE;
                }
        }
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void LAR_R32_RM32(x86_core_cpu_execution_context *context)
{
    lib_u16 selector;
    lib_u64 descriptor;
    CPU_TRACE_CALL_BEGIN("LAR_R32_RM32");
    _adv;
    if (_IsProtected)
    {
        CPU_TRACE_BLOCK_BEGIN("Protected(1)");
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        selector = X86_CPU_MASK_U16(instruction_state.data.crm);
        if (_s_check_selector(context, selector))
        {
            _ClrEFLAGS_ZF;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("selector(valid)");
            CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, selector, X86_CPU_REFERENCE_OF(descriptor)));
            if (_IsDescUser(descriptor))
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
                    _SetEFLAGS_ZF;
                    break;
                case VCPU_DESC_SYS_TYPE_TSS_32_AVL:
                case VCPU_DESC_SYS_TYPE_TSS_32_BUSY:
                case VCPU_DESC_SYS_TYPE_CALLGATE_32:
                    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
                        _SetEFLAGS_ZF;
                    else
                        _ClrEFLAGS_ZF;
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
                CPU_TRACE_BLOCK_BEGIN("EFLAGS_ZF");
                switch (_GetOperandSize)
                {
                case 2:
                    CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
                    descriptor = (X86_CPU_MASK_U16(descriptor >> 32) & 0xff00);
                    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(descriptor), 2));
                    CPU_TRACE_BLOCK_END;
                    break;
                case 4:
                    CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
                    descriptor = (X86_CPU_MASK_U32(descriptor >> 32) & 0x00f0ff00);
                    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(descriptor), 4));
                    CPU_TRACE_BLOCK_END;
                    break;
                default:
                    CPU_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("Protected(0)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
static void LSL_R32_RM32(x86_core_cpu_execution_context *context)
{
    lib_u16 selector;
    lib_u32 limit;
    lib_u64 descriptor;
    CPU_TRACE_CALL_BEGIN("LSL_R32_RM32");
    _adv;
    if (_IsProtected)
    {
        CPU_TRACE_BLOCK_BEGIN("Protected(1)");
        CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
        selector = X86_CPU_MASK_U16(instruction_state.data.crm);
        if (_s_check_selector(context, selector))
        {
            _ClrEFLAGS_ZF;
        }
        else
        {
            CPU_TRACE_BLOCK_BEGIN("selector(valid)");
            CPU_TRACE_CHECK_RETURN(_s_read_xdt(context, selector, X86_CPU_REFERENCE_OF(descriptor)));
            if (_IsDescUser(descriptor))
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
                    _SetEFLAGS_ZF;
                    break;
                case VCPU_DESC_SYS_TYPE_TSS_32_AVL:
                case VCPU_DESC_SYS_TYPE_TSS_32_BUSY:
                    if (cpu_profile_has_32bit_architecture(context->cpu_profile))
                        _SetEFLAGS_ZF;
                    else
                        _ClrEFLAGS_ZF;
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
                CPU_TRACE_BLOCK_BEGIN("EFLAGS_ZF");
                limit = _IsDescSegGranularLarge(descriptor) ? ((_GetDescSeg_Limit(descriptor) << 12) | 0x0fff) : _GetDescSeg_Limit(descriptor);
                if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386) {
                    instruction_state.data.source_lsl_granularity_valid = LIB_TRUE;
                    instruction_state.data.source_lsl_page_granular =
                        _IsDescSegGranularLarge(descriptor);
                }
                switch (_GetOperandSize)
                {
                case 2:
                    CPU_TRACE_BLOCK_BEGIN("OperandSize(2)");
                    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(limit), 2));
                    CPU_TRACE_BLOCK_END;
                    break;
                case 4:
                    CPU_TRACE_BLOCK_BEGIN("OperandSize(4)");
                    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(limit), 4));
                    CPU_TRACE_BLOCK_END;
                    break;
                default:
                    CPU_TRACE_IMPOSSIBLE_RETURN;
                    break;
                }
                CPU_TRACE_BLOCK_END;
            }
            CPU_TRACE_BLOCK_END;
        }
        CPU_TRACE_BLOCK_END;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("Protected(0)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
static void CLTS(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("CLTS");
    _new_code_path_;
    _adv;
    if (!_GetCR0_PE)
    {
        _ClrCR0_TS;
    }
    else
    {
        CPU_TRACE_BLOCK_BEGIN("CR0_PE(1)");
        if (_GetCPL > 0)
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        else
            _ClrCR0_TS;
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
static void _kma_cache_flush_instruction(x86_core_cpu_execution_context *context,
    lib_bool writeback)
{
    CPU_TRACE_CALL_BEGIN("_kma_cache_flush_instruction");
    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX)
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
    _adv;
    if (_GetCPL) CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
    _kma_cache_invalidate(context);
    x86_core_cpu_execution_invalidate_prefetch(context);
    if (context->bus->cache_operation == LIB_NULL)
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(0u));
    if (writeback && context->bus->cache_operation(context->bus_context,
            X86_CORE_CPU_CACHE_WRITEBACK) != LIB_STATUS_OK)
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(0u));
    if (context->bus->cache_operation(context->bus_context,
            X86_CORE_CPU_CACHE_FLUSH) != LIB_STATUS_OK)
        CPU_TRACE_CHECK_RETURN(_SetExcept_CE(0u));
    CPU_TRACE_CALL_END;
}
static void INVD(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("INVD");
    CPU_TRACE_CHECK_RETURN(_kma_cache_flush_instruction(context, LIB_FALSE));
    CPU_TRACE_CALL_END;
}
static void WBINVD(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("WBINVD");
    CPU_TRACE_CHECK_RETURN(_kma_cache_flush_instruction(context, LIB_TRUE));
    CPU_TRACE_CALL_END;
}
static void MOV_R32_CR(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_R32_CR");
    _adv;
    if (_GetCPL)
    {
        CPU_TRACE_BLOCK_BEGIN("CPL(!0)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CHECK_RETURN(_d_modrm_creg(context));
    if (instruction_state.data.rr != (lib_uptr)&cpu_state.data.cr0 &&
        instruction_state.data.rr != (lib_uptr)&cpu_state.data.cr2 &&
        instruction_state.data.rr != (lib_uptr)&cpu_state.data.cr3 &&
        instruction_state.data.rr != (lib_uptr)&cpu_state.data.cr4)
    {
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
    }
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rrm, X86_CPU_REFERENCE_OF(instruction_state.data.cr), 4));
    CPU_TRACE_CALL_END;
}
static void MOV_R32_DR(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_R32_DR");
    _new_code_path_;
    _adv;
    if (_GetCPL)
    {
        CPU_TRACE_BLOCK_BEGIN("CPL(!0)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CHECK_RETURN(_d_modrm_dreg(context));
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rrm, X86_CPU_REFERENCE_OF(instruction_state.data.cr), 4));
    CPU_TRACE_CALL_END;
}
static void MOV_CR_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_CR_R32");
    _adv;
    if (_GetCPL)
    {
        CPU_TRACE_BLOCK_BEGIN("CPL(!0)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CHECK_RETURN(_d_modrm_creg(context));
    if (instruction_state.data.rr == (lib_uptr)&cpu_state.data.cr0)
    {
        CPU_TRACE_CHECK_RETURN(_s_write_cr0(context,
            instruction_state.data.crm));
    }
    else if (instruction_state.data.rr == (lib_uptr)&cpu_state.data.cr2)
    {
        cpu_state.data.cr2 = instruction_state.data.crm;
    }
    else if (instruction_state.data.rr == (lib_uptr)&cpu_state.data.cr3)
    {
        CPU_TRACE_CHECK_RETURN(_s_write_cr3(context,
            instruction_state.data.crm));
    }
    else if (instruction_state.data.rr == (lib_uptr)&cpu_state.data.cr4)
    {
        if (!x86_core_cpu_cr4_value_valid(context->cpu_profile,
                X86_CPU_MASK_U32(instruction_state.data.crm)))
            CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        cpu_state.data.cr4 = X86_CPU_MASK_U32(instruction_state.data.crm);
    }
    else
    {
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
    }
    CPU_TRACE_CALL_END;
}
static void MOV_DR_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_DR_R32");
    _new_code_path_;
    _adv;
    if (_GetCPL)
    {
        CPU_TRACE_BLOCK_BEGIN("CPL(!0)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CHECK_RETURN(_d_modrm_dreg(context));
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.crm), 4));
    CPU_TRACE_CALL_END;
}
static void MOV_R32_TR(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_R32_TR");
    _new_code_path_;
    _adv;
    if (_GetCPL)
    {
        CPU_TRACE_BLOCK_BEGIN("CPL(!0)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CHECK_RETURN(_d_modrm_treg(context));
    if (instruction_state.data.rr == (lib_uptr)&cpu_state.data.tr5)
        CPU_TRACE_CHECK_RETURN(_SetExcept_UD(0));
    if (instruction_state.data.rr == (lib_uptr)&cpu_state.data.tr3) {
        cpu_state.data.tr3 = context->cache_read[(cpu_state.data.tr5 >> 2u) & 3u];
        instruction_state.data.cr = cpu_state.data.tr3;
    }
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rrm, X86_CPU_REFERENCE_OF(instruction_state.data.cr), 4));
    CPU_TRACE_CALL_END;
}
static void MOV_TR_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOV_TR_R32");
    _new_code_path_;
    _adv;
    if (_GetCPL)
    {
        CPU_TRACE_BLOCK_BEGIN("CPL(!0)");
        CPU_TRACE_CHECK_RETURN(_SetExcept_GP(0));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CHECK_RETURN(_d_modrm_treg(context));
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
    {
        if (instruction_state.data.rr == (lib_uptr)&cpu_state.data.tr3) {
            cpu_state.data.tr3 = X86_CPU_MASK_U32(instruction_state.data.crm);
            context->cache_fill[(cpu_state.data.tr5 >> 2u) & 3u] = cpu_state.data.tr3;
        } else if (instruction_state.data.rr == (lib_uptr)&cpu_state.data.tr4)
            cpu_state.data.tr4 = X86_CPU_MASK_U32(instruction_state.data.crm);
        else if (instruction_state.data.rr == (lib_uptr)&cpu_state.data.tr5)
            _kma_cache_test(context, X86_CPU_MASK_U32(instruction_state.data.crm));
        else if (instruction_state.data.rr == (lib_uptr)&cpu_state.data.tr6)
            _kma_tlb_test(context, X86_CPU_MASK_U32(instruction_state.data.crm));
        else
            cpu_state.data.tr7 = X86_CPU_MASK_U32(instruction_state.data.crm) & 0xfffffc1cu;
    }
    else
    {
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr,
            X86_CPU_REFERENCE_OF(instruction_state.data.crm), 4));
    }
    CPU_TRACE_CALL_END;
}
_______todo WRMSR(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("WRMSR");
    UndefinedOpcode(context);
    instruction_state.data.flagIgnore = LIB_TRUE;
    CPU_TRACE_CALL_END;
}
_______todo RDMSR(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("RDMSR");
    UndefinedOpcode(context);
    instruction_state.data.flagIgnore = LIB_TRUE;
    CPU_TRACE_CALL_END;
}
static void JO_REL32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JO_REL32");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_OF));
    CPU_TRACE_CALL_END;
}
static void JNO_REL32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JNO_REL32");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_OF));
    CPU_TRACE_CALL_END;
}
static void JC_REL32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JC_REL32");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_CF));
    CPU_TRACE_CALL_END;
}
static void JNC_REL32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JNC_REL32");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_CF));
    CPU_TRACE_CALL_END;
}
static void JZ_REL32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JZ_REL32");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_ZF));
    CPU_TRACE_CALL_END;
}
static void JNZ_REL32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JNZ_REL32");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_ZF));
    CPU_TRACE_CALL_END;
}
static void JNA_REL32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JNA_REL32");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), _GetOperandSize,
                                   (_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    CPU_TRACE_CALL_END;
}
static void JA_REL32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JA_REL32");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), _GetOperandSize,
                                   !(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    CPU_TRACE_CALL_END;
}
static void JS_REL32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JS_REL32");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_SF));
    CPU_TRACE_CALL_END;
}
static void JNS_REL32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JNS_REL32");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_SF));
    CPU_TRACE_CALL_END;
}
static void JP_REL32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JP_REL32");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), _GetOperandSize, _GetEFLAGS_PF));
    CPU_TRACE_CALL_END;
}
static void JNP_REL32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JNP_REL32");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), _GetOperandSize, !_GetEFLAGS_PF));
    CPU_TRACE_CALL_END;
}
static void JL_REL32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JL_REL32");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), _GetOperandSize, (_GetEFLAGS_SF != _GetEFLAGS_OF)));
    CPU_TRACE_CALL_END;
}
static void JNL_REL32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JNL_REL32");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), _GetOperandSize, (_GetEFLAGS_SF == _GetEFLAGS_OF)));
    CPU_TRACE_CALL_END;
}
static void JNG_REL32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JNG_REL32");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), _GetOperandSize,
                                   (_GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF))));
    CPU_TRACE_CALL_END;
}
static void JG_REL32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("JG_REL32");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_imm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_e_jcc(context, X86_CPU_MASK_U32(instruction_state.data.cimm), _GetOperandSize,
                                   (!_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF))));
    CPU_TRACE_CALL_END;
}
static void SETO_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SETO_RM8");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_OF));
    CPU_TRACE_CALL_END;
}
static void SETNO_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SETO_RM8");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_m_setcc_rm(context, !_GetEFLAGS_OF));
    CPU_TRACE_CALL_END;
}
static void SETC_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SETC_RM8");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_CF));
    CPU_TRACE_CALL_END;
}
static void SETNC_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SETNC_RM8");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_m_setcc_rm(context, !_GetEFLAGS_CF));
    CPU_TRACE_CALL_END;
}
static void SETZ_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SETZ_RM8");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_ZF));
    CPU_TRACE_CALL_END;
}
static void SETNZ_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SETNZ_RM8");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_m_setcc_rm(context, !_GetEFLAGS_ZF));
    CPU_TRACE_CALL_END;
}
static void SETNA_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SETNA_RM8");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_CF || _GetEFLAGS_ZF));
    CPU_TRACE_CALL_END;
}
static void SETA_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SETA_RM8");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_m_setcc_rm(context, !(_GetEFLAGS_CF || _GetEFLAGS_ZF)));
    CPU_TRACE_CALL_END;
}
static void SETS_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SETS_RM8");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_SF));
    CPU_TRACE_CALL_END;
}
static void SETNS_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SETNS_RM8");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_m_setcc_rm(context, !_GetEFLAGS_SF));
    CPU_TRACE_CALL_END;
}
static void SETP_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SETP_RM8");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_PF));
    CPU_TRACE_CALL_END;
}
static void SETNP_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SETNP_RM8");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_m_setcc_rm(context, !_GetEFLAGS_PF));
    CPU_TRACE_CALL_END;
}
static void SETL_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SETL_RM8");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_SF != _GetEFLAGS_OF));
    CPU_TRACE_CALL_END;
}
static void SETNL_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SETNL_RM8");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_SF == _GetEFLAGS_OF));
    CPU_TRACE_CALL_END;
}
static void SETNG_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SETNG_RM8");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_m_setcc_rm(context, _GetEFLAGS_ZF || (_GetEFLAGS_SF != _GetEFLAGS_OF)));
    CPU_TRACE_CALL_END;
}
static void SETG_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SETG_RM8");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 0, 1));
    CPU_TRACE_CHECK_RETURN(_m_setcc_rm(context, !_GetEFLAGS_ZF && (_GetEFLAGS_SF == _GetEFLAGS_OF)));
    CPU_TRACE_CALL_END;
}
static void PUSH_FS(x86_core_cpu_execution_context *context)
{
    lib_u32 xs_sel;
    CPU_TRACE_CALL_BEGIN("PUSH_FS");
    _adv;
    xs_sel = cpu_state.data.fs.selector;
    CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(xs_sel), _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void POP_FS(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("POP_FS");
    _adv;
    CPU_TRACE_CHECK_RETURN(_e_pop_sreg(context, &cpu_state.data.fs,
        _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void CPUID(x86_core_cpu_execution_context *context)
{
    const lib_u32 leaf = cpu_state.data.eax;
    CPU_TRACE_CALL_BEGIN("CPUID");
    _adv;
    /* Intel 272769-004 defines leaves 0/1. Reserved outputs and undefined
     * higher leaves use deterministic zeros, not a claimed hardware value. */
    _WriteGPR32(eax) = 0u;
    _WriteGPR32(ebx) = 0u;
    _WriteGPR32(ecx) = 0u;
    _WriteGPR32(edx) = 0u;
    if (leaf == 0u)
    {
        _WriteGPR32(eax) = 1u;
        _WriteGPR32(ebx) = 0x756e6547u;
        _WriteGPR32(edx) = 0x49656e69u;
        _WriteGPR32(ecx) = 0x6c65746eu;
    }
    else if (leaf == 1u)
    {
        _WriteGPR32(eax) = VCPU_486SX_SIGNATURE;
        _WriteGPR32(edx) = 2u;
    }
    CPU_TRACE_CALL_END;
}
static void CMPXCHG_RM8_R8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("CMPXCHG_RM8_R8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 1, LIB_FALSE));
    CPU_TRACE_CHECK_RETURN(_a_cmp(context, cpu_state.data.al,
        instruction_state.data.crm, 8));
    if (cpu_state.data.al == instruction_state.data.crm)
        instruction_state.data.crm = instruction_state.data.cr;
    else
        _WriteGPR8(al) = (lib_u8)instruction_state.data.crm;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    CPU_TRACE_CALL_END;
}
static void CMPXCHG_RM32_R32(x86_core_cpu_execution_context *context)
{
    lib_u32 accumulator;
    CPU_TRACE_CALL_BEGIN("CMPXCHG_RM32_R32");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
    accumulator = _GetOperandSize == 2 ? cpu_state.data.ax : cpu_state.data.eax;
    CPU_TRACE_CHECK_RETURN(_a_cmp(context, accumulator,
        instruction_state.data.crm, _GetOperandSize * 8));
    if (accumulator == instruction_state.data.crm)
        instruction_state.data.crm = instruction_state.data.cr;
    else if (_GetOperandSize == 2)
        _WriteGPR16(ax) = (lib_u16)instruction_state.data.crm;
    else
        _WriteGPR32(eax) = (lib_u32)instruction_state.data.crm;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void XADD_RM8_R8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("XADD_RM8_R8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, 1, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, 1, LIB_FALSE));
    CPU_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm,
        instruction_state.data.cr, 8));
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr,
        X86_CPU_REFERENCE_OF(instruction_state.data.crm), 1));
    instruction_state.data.crm = instruction_state.data.result;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, 1));
    CPU_TRACE_CALL_END;
}
static void XADD_RM32_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("XADD_RM32_R32");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
    CPU_TRACE_CHECK_RETURN(_a_add(context, instruction_state.data.crm,
        instruction_state.data.cr, _GetOperandSize * 8));
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr,
        X86_CPU_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
    instruction_state.data.crm = instruction_state.data.result;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void BSWAP_R32(x86_core_cpu_execution_context *context)
{
    lib_u8 opcode;
    lib_u32 *reg;
    lib_u32 value;
    CPU_TRACE_CALL_BEGIN("BSWAP_R32");
    /* Intel defines no 16-bit result. The selected model rejects that form;
     * this is a deterministic policy, not a hardware #UD claim. */
    if (_GetOperandSize != 4u)
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
    CPU_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip,
        X86_CPU_REFERENCE_OF(opcode), 1));
    _adv;
    switch (opcode & 7u)
    {
    case 0: reg = &cpu_state.data.eax; break;
    case 1: reg = &cpu_state.data.ecx; break;
    case 2: reg = &cpu_state.data.edx; break;
    case 3: reg = &cpu_state.data.ebx; break;
    case 4: reg = &cpu_state.data.esp; break;
    case 5: reg = &cpu_state.data.ebp; break;
    case 6: reg = &cpu_state.data.esi; break;
    case 7: reg = &cpu_state.data.edi; break;
    default: CPU_TRACE_IMPOSSIBLE_RETURN; break;
    }
    value = *reg;
    value = (value << 24u) | ((value & 0x0000ff00u) << 8u) |
        ((value & 0x00ff0000u) >> 8u) | (value >> 24u);
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, (lib_uptr)reg,
        X86_CPU_REFERENCE_OF(value), sizeof(value)));
    CPU_TRACE_CALL_END;
}
static void BT_RM32_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("BT_RM32_R32");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_bit_rmimm(context, _GetOperandSize, _GetOperandSize, 0));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_a_bt(context, (lib_u32)instruction_state.data.crm, (lib_u32)instruction_state.data.cimm, _GetOperandSize * 8));
    CPU_TRACE_CALL_END;
}
static void SHLD_RM32_R32_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SHLD_RM32_R32_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
    CPU_TRACE_CHECK_RETURN(_a_shld(context, instruction_state.data.crm, instruction_state.data.cr, X86_CPU_MASK_U8(instruction_state.data.cimm), _GetOperandSize * 8));
    if (X86_CPU_MASK_U8(instruction_state.data.cimm) & 0x1f)
    {
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    CPU_TRACE_CALL_END;
}
static void SHLD_RM32_R32_CL(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SHLD_RM32_R32_CL");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
    CPU_TRACE_CHECK_RETURN(_a_shld(context, instruction_state.data.crm, instruction_state.data.cr, cpu_state.data.cl, _GetOperandSize * 8));
    if (cpu_state.data.cl & 0x1f)
    {
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    CPU_TRACE_CALL_END;
}
static void PUSH_GS(x86_core_cpu_execution_context *context)
{
    lib_u32 xs_sel;
    CPU_TRACE_CALL_BEGIN("PUSH_GS");
    _adv;
    xs_sel = cpu_state.data.gs.selector;
    CPU_TRACE_CHECK_RETURN(_e_push(context, X86_CPU_REFERENCE_OF(xs_sel), _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void POP_GS(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("POP_GS");
    _adv;
    CPU_TRACE_CHECK_RETURN(_e_pop_sreg(context, &cpu_state.data.gs,
        _GetOperandSize));
    CPU_TRACE_CALL_END;
}
#define X86_CPU_SMM_IMAGE_BYTES 512u
#define X86_CPU_SMM_IMAGE_OFFSET 0x7e00u

/* Instruction fetch and asynchronous entry share bookkeeping, not prefetch.
 * An interrupt must not depend on fetching the interrupted next instruction. */
static void ExecBoundary(x86_core_cpu_execution_context *context)
{
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) {
        lib_u8 prefixes = 0u;

        while (prefixes < instruction_state.data.oplen &&
            _kdf_check_prefix(context, instruction_state.data.opcodes[prefixes])) ++prefixes;
        instruction_state.data.timing_previous_opcode = prefixes < instruction_state.data.oplen ?
            instruction_state.data.opcodes[prefixes] : 0u;
        if (prefixes + 1u < instruction_state.data.oplen)
            instruction_state.data.timing_previous_opcode |=
                (lib_u16)instruction_state.data.opcodes[prefixes + 1u] << 8u;
        instruction_state.data.timing_previous_gpr_writes = instruction_state.data.timing_gpr_writes;
    } else {
        instruction_state.data.timing_previous_opcode = 0u;
        instruction_state.data.timing_previous_gpr_writes = 0u;
    }
    instruction_state.data.timing_gpr_writes = 0u;
    instruction_state.data.timing_stack_access = LIB_FALSE;
    instruction_state.data.flagIgnore = LIB_FALSE;
    instruction_state.data.msize = 0u;
    instruction_state.data.watch_hit = LIB_FALSE;
    instruction_state.data.reccs = cpu_state.data.cs.selector;
    instruction_state.data.receip = cpu_state.data.eip;
    instruction_state.data.linear = cpu_state.data.cs.base + cpu_state.data.eip;
    instruction_state.data.oldcpu = cpu_state;
    instruction_state.data.except = 0u;
    instruction_state.data.excode = 0u;
    instruction_state.data.oplen = 0u;
    lib_memory_set(instruction_state.data.opcodes, 0u, sizeof(instruction_state.data.opcodes));
    instruction_state.data.flagLock = LIB_FALSE;
    instruction_state.data.roverds = &cpu_state.data.ds;
    instruction_state.data.roverss = &cpu_state.data.ss;
    instruction_state.data.prefix_rep = PREFIX_REP_NONE;
    instruction_state.data.prefix_oprsize = LIB_FALSE;
    instruction_state.data.prefix_addrsize = LIB_FALSE;
    instruction_state.data.flagMem = LIB_FALSE;
    instruction_state.data.timing_ea_base_mask = 0u;
    instruction_state.data.timing_ea_indexed = LIB_FALSE;
    instruction_state.data.timing_ea_displacement = LIB_FALSE;
    instruction_state.data.timing_immediate = LIB_FALSE;
    instruction_state.data.timing_transfer_type = 0u;
    instruction_state.data.timing_transfer_parameters = 0u;
    instruction_state.data.timing_unaccessed_descriptors = 0u;
    instruction_state.data.timing_invlpg_hit = LIB_FALSE;
    instruction_state.data.timing_fetch_bus_ticks = 0u;
    instruction_state.data.timing_external_wait_ticks = 0u;
    instruction_state.data.timing_external_wait_overflow = LIB_FALSE;
    instruction_state.data.timing_bus_service_ticks = 0u;
    instruction_state.data.timing_bus_stall_ticks = 0u;
    instruction_state.data.timing_bus_wait_ticks = 0u;
    instruction_state.data.timing_bus_invalidation_ticks = 0u;
    instruction_state.data.timing_transfer_ticks = 0u;
    instruction_state.data.timing_entry_transfer_ticks = 0u;
    instruction_state.data.timing_cache_used = LIB_FALSE;
    instruction_state.data.timing_data_misses = 0u;
    instruction_state.data.timing_missed_reads = 0u;
    instruction_state.data.timing_descriptor_misses = 0u;
    instruction_state.data.timing_missed_descriptors = 0u;
    instruction_state.data.timing_idt_misses = 0u;
    instruction_state.data.timing_task_gate_miss_mask = 0u;
    instruction_state.data.timing_pair_misses = 0u;
    instruction_state.data.timing_tss_field_misses = 0u;
    instruction_state.data.timing_missed_tss_fields = 0u;
    instruction_state.data.timing_repeat_previous_misses = 0u;
    instruction_state.data.timing_locked_reads = 0u;
    instruction_state.data.timing_table_misses = 0u;
    instruction_state.data.timing_unaligned_accesses = 0u;
    lib_memory_set(instruction_state.data.timing_tlb_walks, 0,
        sizeof(instruction_state.data.timing_tlb_walks));
    instruction_state.data.timing_tlb_updates = 0u;
    instruction_state.data.source_lsl_granularity_valid = LIB_FALSE;
    instruction_state.data.source_lsl_page_granular = LIB_FALSE;
    instruction_state.data.flagInsLoop = LIB_FALSE;
    instruction_state.data.bit = 0u;
    instruction_state.data.opr1 = 0u;
    instruction_state.data.opr2 = 0u;
    instruction_state.data.result = 0u;
    instruction_state.data.udf = 0u;
    instruction_state.data.mrm.rsreg = LIB_NULL;
    instruction_state.data.mrm.offset = 0u;
    context->debug_tf_before = _GetEFLAGS_TF;
    context->debug_rf_before = _GetEFLAGS_RF;
    context->instruction_task_switched = LIB_FALSE;
}

/* Public 486 slots and explicitly modeled reserved slots share one physical
 * image. Hidden attributes use the S7 L2 encoding, never native C layout. */
static void _smm_put(lib_u8 *image, lib_u16 offset, lib_u32 value)
{
    for (lib_u8 byte = 0u; byte < 4u; ++byte)
        image[offset - X86_CPU_SMM_IMAGE_OFFSET + byte] = (lib_u8)(value >> (byte * 8u));
}

static lib_u32 _smm_get(const lib_u8 *image, lib_u16 offset)
{
    lib_u32 value = 0u;

    for (lib_u8 byte = 0u; byte < 4u; ++byte)
        value |= (lib_u32)image[offset - X86_CPU_SMM_IMAGE_OFFSET + byte] << (byte * 8u);
    return value;
}

static void _smm_save_segment(lib_u8 *image, lib_u16 offset,
    const t_cpu_data_sreg *segment)
{
    lib_u32 attributes = (segment->flagValid ? 0x80u : 0u) |
        ((lib_u32)segment->dpl << 5u);

    if (segment->sregtype <= SREG_CODE) {
        attributes |= 0x10u | (segment->seg.accessed ? 1u : 0u) |
            (segment->seg.executable ? 8u : 0u);
        if (segment->seg.executable) {
            attributes |= (segment->seg.exec.readable ? 2u : 0u) |
                (segment->seg.exec.conform ? 4u : 0u) |
                (segment->seg.exec.defsize ? 0x4000u : 0u);
        } else {
            attributes |= (segment->seg.data.writable ? 2u : 0u) |
                (segment->seg.data.expdown ? 4u : 0u) |
                (segment->seg.data.big ? 0x4000u : 0u);
        }
    } else attributes |= segment->sys.type & 15u;
    _smm_put(image, offset, attributes);
    _smm_put(image, offset - 4u, segment->base);
    _smm_put(image, offset - 8u, segment->limit);
}

static void _smm_restore_segment(const lib_u8 *image, lib_u16 offset,
    t_cpu_data_sreg *segment)
{
    const lib_u32 attributes = _smm_get(image, offset);

    segment->flagValid = (attributes & 0x80u) != 0u;
    segment->dpl = (attributes >> 5u) & 3u;
    segment->base = _smm_get(image, offset - 4u);
    segment->limit = _smm_get(image, offset - 8u);
    if (segment->sregtype <= SREG_CODE) {
        segment->seg.accessed = (attributes & 1u) != 0u;
        segment->seg.executable = (attributes & 8u) != 0u;
        if (segment->seg.executable) {
            segment->seg.exec.readable = (attributes & 2u) != 0u;
            segment->seg.exec.conform = (attributes & 4u) != 0u;
            segment->seg.exec.defsize = (attributes & 0x4000u) != 0u;
        } else {
            segment->seg.data.writable = (attributes & 2u) != 0u;
            segment->seg.data.expdown = (attributes & 4u) != 0u;
            segment->seg.data.big = (attributes & 0x4000u) != 0u;
        }
    } else segment->sys.type = attributes & 15u;
}

static void _smm_build_image(x86_core_cpu_execution_context *context,
    lib_u8 *image)
{
    const lib_u32 registers[] = {cpu_state.data.eax, cpu_state.data.ecx,
        cpu_state.data.edx, cpu_state.data.ebx, cpu_state.data.esp,
        cpu_state.data.ebp, cpu_state.data.esi, cpu_state.data.edi};
    const t_cpu_data_sreg *selectors[] = {&cpu_state.data.es, &cpu_state.data.cs,
        &cpu_state.data.ss, &cpu_state.data.ds, &cpu_state.data.fs, &cpu_state.data.gs,
        &cpu_state.data.ldtr, &cpu_state.data.tr};
    const t_cpu_data_sreg *descriptors[] = {&cpu_state.data.tr, &cpu_state.data.idtr,
        &cpu_state.data.gdtr, &cpu_state.data.ldtr, &cpu_state.data.gs, &cpu_state.data.fs,
        &cpu_state.data.ds, &cpu_state.data.ss, &cpu_state.data.cs, &cpu_state.data.es};

    lib_memory_set(image, 0u, X86_CPU_SMM_IMAGE_BYTES);
    for (lib_u8 index = 0u; index < 8u; ++index) {
        _smm_put(image, 0x7fd0u + index * 4u, registers[index]);
        _smm_put(image, 0x7fa8u + index * 4u, selectors[index]->selector);
    }
    for (lib_u8 index = 0u; index < 10u; ++index)
        _smm_save_segment(image, 0x7fa4u - index * 12u, descriptors[index]);
    _smm_put(image, 0x7ffcu, cpu_state.data.cr0);
    _smm_put(image, 0x7ff8u, cpu_state.data.cr3);
    _smm_put(image, 0x7ff4u, cpu_state.data.eflags);
    _smm_put(image, 0x7ff0u, cpu_state.data.eip);
    _smm_put(image, 0x7fccu, cpu_state.data.dr6);
    _smm_put(image, 0x7fc8u, cpu_state.data.dr7);
    _smm_put(image, 0x7f28u, cpu_state.data.cr4);
    _smm_put(image, 0x7f00u, cpu_state.data.flagHalt ? 0x00010000u : 0u);
    _smm_put(image, 0x7efcu, 0x00030000u);
    _smm_put(image, 0x7ef8u, context->smbase);
    _smm_put(image, 0x7ef4u, cpu_state.data.cr2);
    _smm_put(image, 0x7ef0u, cpu_state.data.cr1);
    _smm_put(image, 0x7eecu, (lib_u32)context->interrupt_shadow |
        (context->debug_trap_pending ? 0x100u : 0u));
    _smm_put(image, 0x7ee8u, context->debug_trap_cause);
    if (context->io_restart_valid) {
        _smm_put(image, 0x7f10u, context->io_restart_eip);
        _smm_put(image, 0x7f0cu, context->io_restart_esi);
        _smm_put(image, 0x7f08u, context->io_restart_ecx);
        _smm_put(image, 0x7f04u, context->io_restart_edi);
    }
}

static void _smm_initialize_segment(t_cpu_data_sreg *segment,
    lib_u16 selector, lib_u32 base)
{
    segment->flagValid = LIB_TRUE;
    segment->selector = selector;
    segment->base = base;
    segment->limit = LIB_UINT32_MAX;
    segment->dpl = 0u;
    segment->seg.accessed = LIB_TRUE;
    segment->seg.executable = segment->sregtype == SREG_CODE;
    if (segment->seg.executable) {
        segment->seg.exec.defsize = LIB_FALSE;
        segment->seg.exec.conform = LIB_FALSE;
        segment->seg.exec.readable = LIB_TRUE;
    } else {
        segment->seg.data.big = LIB_FALSE;
        segment->seg.data.expdown = LIB_FALSE;
        segment->seg.data.writable = LIB_TRUE;
    }
}

static lib_bool _smm_enter(x86_core_cpu_execution_context *context)
{
    lib_u8 image[X86_CPU_SMM_IMAGE_BYTES];
    t_cpu_data_sreg *segments[] = {&cpu_state.data.es, &cpu_state.data.ss,
        &cpu_state.data.ds, &cpu_state.data.fs, &cpu_state.data.gs};
    const lib_u32 physical = context->smbase + 0xfe00u;
    lib_u64 prefix_service, ticks;

    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX ||
        !context->smi_pending || context->smm_active || x86_core_cpu_is_system_waiting(context) ||
        context->stop_requested || context->debug_pause_requested ||
        (context->shutdown_state != CPU_SHUTDOWN_NONE &&
         context->shutdown_state != CPU_SHUTDOWN_SMM) ||
        context->reset_requested != X86_CORE_CPU_RESET_NONE ||
        instruction_state.data.memory_lock_active) return LIB_FALSE;
    context->repeat_compare_flags_valid = LIB_FALSE;
    _smm_build_image(context, image);
    /* No ExecInit/prefetch: SMI must work even when the next guest instruction
     * cannot be fetched. The existing finalizer owns any provider failure. */
    ExecBoundary(context);
    context->smi_pending = LIB_FALSE;
    x86_core_cpu_bus_drain(context, LIB_TRUE);
    if (context->bus_timing.invalid) {
        _SetExcept_CE(LIB_STATUS_LIMIT_EXCEEDED);
        return LIB_TRUE;
    }
    prefix_service = instruction_state.data.timing_bus_service_ticks;
    context->smm_active = LIB_TRUE;
    context->bus->system_management_active(context->bus_context, LIB_TRUE);
    for (lib_u16 remaining = X86_CPU_SMM_IMAGE_BYTES; remaining != 0u; remaining -= 4u) {
        _kma_write_physical(context, physical + remaining - 4u,
            (lib_uptr)(image + remaining - 4u), 4u,
            X86_CORE_CPU_MEMORY_ACCESS_DATA, 0u);
        if (instruction_state.data.except) break;
    }
    x86_core_cpu_bus_drain(context, LIB_TRUE);
    if (!x86_core_cpu_80486_smm_cost(context, LIB_TRUE,
            instruction_state.data.except == 0u, prefix_service, &ticks)) {
        _SetExcept_CE(LIB_STATUS_LIMIT_EXCEEDED);
        return LIB_TRUE;
    }
    if (!x86_core_cpu_timing_add_ticks(&ticks, instruction_state.data.timing_external_wait_ticks) ||
        !_e_timing_charge(context, ticks)) {
        _SetExcept_CE(LIB_STATUS_LIMIT_EXCEEDED);
        return LIB_TRUE;
    }
    instruction_state.data.timing_entry_transfer_ticks = instruction_state.data.timing_transfer_ticks;
    x86_core_cpu_bus_clear_observation(context);
    if (instruction_state.data.except) return LIB_TRUE;
    cpu_state.data.eflags = 2u;
    cpu_state.data.cr0 &= ~(VCPU_CR0_PE | VCPU_CR0_EM | VCPU_CR0_TS | VCPU_CR0_PG);
    cpu_state.data.dr6 = 0u;
    cpu_state.data.dr7 &= 0x00003000u;
    cpu_state.data.eip = 0x8000u;
    cpu_state.data.flagHalt = LIB_FALSE;
    _smm_initialize_segment(&cpu_state.data.cs, 0x3000u, context->smbase);
    for (lib_u8 index = 0u; index < 5u; ++index)
        _smm_initialize_segment(segments[index], 0u, 0u);
    context->debug_trap_pending = LIB_FALSE;
    context->debug_trap_cause = 0u;
    context->interrupt_shadow = CPU_INTERRUPT_SHADOW_NONE;
    context->nmi_in_service = LIB_TRUE;
    context->wait_state = CPU_WAIT_STATE_NONE;
    context->wait_poll_remaining_ticks = 0u;
    context->shutdown_state = CPU_SHUTDOWN_NONE;
    context->shutdown_requested = LIB_FALSE;
    context->source_repeat_active = LIB_FALSE;
    x86_core_cpu_execution_invalidate_prefetch(context);
    x86_core_cpu_execution_invalidate_tlb(context);
    return LIB_TRUE;
}

static void _smm_failed_restore(x86_core_cpu_execution_context *context, lib_bool released)
{
    const lib_u64 fetch = instruction_state.data.timing_fetch_bus_ticks;
    const lib_u64 exit_ticks = released ? 4u : 0u;
    lib_u64 ticks;

    if (!x86_core_cpu_80486_smm_cost(context, LIB_FALSE, LIB_FALSE, fetch, &ticks) ||
        fetch > LIB_UINT64_MAX - ticks || exit_ticks > LIB_UINT64_MAX - ticks - fetch) {
        _SetExcept_CE(LIB_STATUS_LIMIT_EXCEEDED);
        return;
    }
    /* Failed RSM does not retire: accepted fetch/restore work is an event
     * prefix, without the successful restore floor or normal-state restart. */
    ticks += fetch + exit_ticks;
    if (!x86_core_cpu_timing_add_ticks(&ticks, instruction_state.data.timing_external_wait_ticks)) {
        _SetExcept_CE(LIB_STATUS_LIMIT_EXCEEDED);
        return;
    }
    (void)_e_timing_charge(context, ticks);
    instruction_state.data.timing_entry_transfer_ticks = instruction_state.data.timing_transfer_ticks;
    x86_core_cpu_bus_clear_observation(context);
}

static void RSM(x86_core_cpu_execution_context *context)
{
    lib_u8 image[X86_CPU_SMM_IMAGE_BYTES];
    t_cpu candidate = cpu_state;
    lib_u32 *registers[] = {&candidate.data.eax, &candidate.data.ecx,
        &candidate.data.edx, &candidate.data.ebx, &candidate.data.esp,
        &candidate.data.ebp, &candidate.data.esi, &candidate.data.edi};
    t_cpu_data_sreg *selectors[] = {&candidate.data.es, &candidate.data.cs,
        &candidate.data.ss, &candidate.data.ds, &candidate.data.fs, &candidate.data.gs,
        &candidate.data.ldtr, &candidate.data.tr};
    t_cpu_data_sreg *descriptors[] = {&candidate.data.tr, &candidate.data.idtr,
        &candidate.data.gdtr, &candidate.data.ldtr, &candidate.data.gs, &candidate.data.fs,
        &candidate.data.ds, &candidate.data.ss, &candidate.data.cs, &candidate.data.es};
    const lib_u32 physical = context->smbase + 0xfe00u;
    lib_u32 smbase, restart, pending;

    CPU_TRACE_CALL_BEGIN("RSM");
    if (!context->smm_active) CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
    _adv;
    for (lib_u16 remaining = X86_CPU_SMM_IMAGE_BYTES; remaining != 0u; remaining -= 4u) {
        _kma_read_physical(context, physical + remaining - 4u,
            (lib_uptr)(image + remaining - 4u), 4u,
            X86_CORE_CPU_MEMORY_ACCESS_DATA, 0u);
        if (instruction_state.data.except) {
            _smm_failed_restore(context, LIB_FALSE);
            CPU_TRACE_CALL_END;
            return;
        }
    }
    smbase = _smm_get(image, 0x7ef8u);
    candidate.data.cr0 = _smm_get(image, 0x7ffcu);
    candidate.data.cr4 = _smm_get(image, 0x7f28u);
    context->smm_active = LIB_FALSE;
    context->bus->system_management_active(context->bus_context, LIB_FALSE);
    context->nmi_in_service = LIB_FALSE;
    if ((smbase & 0x7fffu) || (candidate.data.cr4 & ~VCPU_CR4_VME) ||
        ((candidate.data.cr0 & (VCPU_CR0_PG | VCPU_CR0_PE)) == VCPU_CR0_PG) ||
        ((candidate.data.cr0 & (VCPU_CR0_NW | VCPU_CR0_CD)) == VCPU_CR0_NW)) {
        _smm_failed_restore(context, LIB_TRUE);
        if (instruction_state.data.except) {
            CPU_TRACE_CALL_END;
            return;
        }
        context->shutdown_state = CPU_SHUTDOWN_SMM;
        CPU_TRACE_CHECK_RETURN(x86_core_cpu_execution_raise_exception(context,
            VCPUINS_EXCEPT_SHUTDOWN, 0u));
    }
    for (lib_u8 index = 0u; index < 8u; ++index) {
        *registers[index] = _smm_get(image, 0x7fd0u + index * 4u);
        selectors[index]->selector = (lib_u16)_smm_get(image, 0x7fa8u + index * 4u);
    }
    for (lib_u8 index = 0u; index < 10u; ++index)
        _smm_restore_segment(image, 0x7fa4u - index * 12u, descriptors[index]);
    candidate.data.cr1 = _smm_get(image, 0x7ef0u);
    candidate.data.cr2 = _smm_get(image, 0x7ef4u);
    candidate.data.cr3 = _smm_get(image, 0x7ff8u);
    candidate.data.dr6 = _smm_get(image, 0x7fccu);
    candidate.data.dr7 = _smm_get(image, 0x7fc8u);
    candidate.data.eip = _smm_get(image, 0x7ff0u);
    candidate.data.eflags = _e_eflags_load(context, _smm_get(image, 0x7ff4u));
    candidate.data.flagHalt = LIB_FALSE;
    restart = _smm_get(image, 0x7f00u);
    if ((restart & 0xffffu) == 0x00ffu) {
        candidate.data.eip = _smm_get(image, 0x7f10u);
        candidate.data.esi = _smm_get(image, 0x7f0cu);
        candidate.data.ecx = _smm_get(image, 0x7f08u);
        candidate.data.edi = _smm_get(image, 0x7f04u);
    }
    if (restart & 0x00010000u) --candidate.data.eip;
    cpu_state = candidate;
    instruction_state.data.timing_gpr_writes = 0xffu;
    context->smbase = smbase;
    pending = _smm_get(image, 0x7eecu);
    context->interrupt_shadow = (cpu_interrupt_shadow)(pending & 3u);
    context->debug_trap_pending = (pending & 0x100u) != 0u;
    context->debug_trap_cause = _smm_get(image, 0x7ee8u);
    context->source_repeat_active = LIB_FALSE;
    x86_core_cpu_execution_invalidate_prefetch(context);
    x86_core_cpu_execution_invalidate_tlb(context);
    CPU_TRACE_CALL_END;
}
static void BTS_RM32_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("BTS_RM32_R32");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_bit_rmimm(context, _GetOperandSize, _GetOperandSize, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
    CPU_TRACE_CHECK_RETURN(_a_bts(context, (lib_u32)instruction_state.data.crm, (lib_u32)instruction_state.data.cimm, _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void SHRD_RM32_R32_I8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SHRD_RM32_R32_I8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_d_imm(context, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
    CPU_TRACE_CHECK_RETURN(_a_shrd(context, instruction_state.data.crm, instruction_state.data.cr, X86_CPU_MASK_U8(instruction_state.data.cimm), _GetOperandSize * 8));
    if (X86_CPU_MASK_U8(instruction_state.data.cimm) & 0x1f)
    {
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    CPU_TRACE_CALL_END;
}
static void SHRD_RM32_R32_CL(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("SHRD_RM32_R32_CL");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
    CPU_TRACE_CHECK_RETURN(_a_shrd(context, instruction_state.data.crm, instruction_state.data.cr, cpu_state.data.cl, _GetOperandSize * 8));
    if (cpu_state.data.cl & 0x1f)
    {
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    }
    CPU_TRACE_CALL_END;
}
static void IMUL_R32_RM32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("IMUL_R32_RM32");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_a_imul2(context, instruction_state.data.cr, instruction_state.data.crm, _GetOperandSize * 8));
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void LSS_R32_M16_32(x86_core_cpu_execution_context *context)
{
    lib_u16 selector;
    lib_u32 offset;
    CPU_TRACE_CALL_BEGIN("LSS_R32_M16_32");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm_ea(context, _GetOperandSize,
        _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_m_read_pair(context, _GetOperandSize, 2));
    switch (_GetOperandSize)
    {
    case 2:
        offset = X86_CPU_MASK_U16(instruction_state.data.crm);
        break;
    case 4:
        offset = X86_CPU_MASK_U32(instruction_state.data.crm);
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    selector = X86_CPU_MASK_U16(instruction_state.data.crm >> (_GetOperandSize * 8u));
    CPU_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.ss, instruction_state.data.rr, selector, offset, _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void BTR_RM32_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("BTR_RM32_R32");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_bit_rmimm(context, _GetOperandSize, _GetOperandSize, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
    CPU_TRACE_CHECK_RETURN(_a_btr(context, (lib_u32)instruction_state.data.crm, (lib_u32)instruction_state.data.cimm, _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void LFS_R32_M16_32(x86_core_cpu_execution_context *context)
{
    lib_u16 selector;
    lib_u32 offset;
    CPU_TRACE_CALL_BEGIN("LFS_R32_M16_32");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm_ea(context, _GetOperandSize,
        _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_m_read_pair(context, _GetOperandSize, 2));
    switch (_GetOperandSize)
    {
    case 2:
        offset = X86_CPU_MASK_U16(instruction_state.data.crm);
        break;
    case 4:
        offset = X86_CPU_MASK_U32(instruction_state.data.crm);
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    selector = X86_CPU_MASK_U16(instruction_state.data.crm >> (_GetOperandSize * 8u));
    CPU_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.fs, instruction_state.data.rr, selector, offset, _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void LGS_R32_M16_32(x86_core_cpu_execution_context *context)
{
    lib_u16 selector;
    lib_u32 offset;
    CPU_TRACE_CALL_BEGIN("LGS_R32_M16_32");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm_ea(context, _GetOperandSize,
        _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_m_read_pair(context, _GetOperandSize, 2));
    switch (_GetOperandSize)
    {
    case 2:
        offset = X86_CPU_MASK_U16(instruction_state.data.crm);
        break;
    case 4:
        offset = X86_CPU_MASK_U32(instruction_state.data.crm);
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    selector = X86_CPU_MASK_U16(instruction_state.data.crm >> (_GetOperandSize * 8u));
    CPU_TRACE_CHECK_RETURN(_e_load_far(context, &cpu_state.data.gs, instruction_state.data.rr, selector, offset, _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void MOVZX_R32_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOVZX_R32_RM8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    instruction_state.data.crm = (lib_u8)instruction_state.data.crm;
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void MOVZX_R32_RM16(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOVZX_R32_RM16");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, 2));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
    instruction_state.data.crm = (lib_u16)instruction_state.data.crm;
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void INS_0F_BA(x86_core_cpu_execution_context *context)
{
    lib_u8 write = 0;
    lib_u8 modrm = 0x00;
    CPU_TRACE_CALL_BEGIN("INS_0F_BA");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_s_read_cs(context, cpu_state.data.eip, X86_CPU_REFERENCE_OF(modrm), 1));
    if (_GetModRM_REG(modrm) == 4)
        write = 0;
    else
        write = 1;
    CPU_TRACE_CHECK_RETURN(_d_bit_rmimm(context, 0, _GetOperandSize, write));
    if (write)
        CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
    else
        CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    switch (instruction_state.data.cr)
    {
    case 0:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(0)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 1:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(1)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 2:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(2)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 3:
        CPU_TRACE_BLOCK_BEGIN("instruction_state.data.cr(3)");
        CPU_TRACE_CHECK_RETURN(UndefinedOpcode(context));
        CPU_TRACE_BLOCK_END;
        break;
    case 4: /* BT_RM32_I8 */
        CPU_TRACE_BLOCK_BEGIN("BT_RM32_I8");
        CPU_TRACE_CHECK_RETURN(_a_bt(context, (lib_u32)instruction_state.data.crm, (lib_u32)instruction_state.data.cimm, _GetOperandSize * 8));
        CPU_TRACE_BLOCK_END;
        break;
    case 5: /* BTS_RM32_I8 */
        CPU_TRACE_BLOCK_BEGIN("BTS_RM32_I8");
        CPU_TRACE_CHECK_RETURN(_a_bts(context, (lib_u32)instruction_state.data.crm, (lib_u32)instruction_state.data.cimm, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
        CPU_TRACE_BLOCK_END;
        break;
    case 6: /* BTR_RM32_I8 */
        CPU_TRACE_BLOCK_BEGIN("BTR_RM32_I8");
        CPU_TRACE_CHECK_RETURN(_a_btr(context, (lib_u32)instruction_state.data.crm, (lib_u32)instruction_state.data.cimm, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
        CPU_TRACE_BLOCK_END;
        break;
    case 7: /* BTC_RM32_I8 */
        CPU_TRACE_BLOCK_BEGIN("BTC_RM32_I8");
        CPU_TRACE_CHECK_RETURN(_a_btc(context, (lib_u32)instruction_state.data.crm, (lib_u32)instruction_state.data.cimm, _GetOperandSize * 8));
        instruction_state.data.crm = instruction_state.data.result;
        CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
        CPU_TRACE_BLOCK_END;
        break;
    default:
        CPU_TRACE_IMPOSSIBLE_RETURN;
        break;
    }
    CPU_TRACE_CALL_END;
}
static void BTC_RM32_R32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("BTC_RM32_R32");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_bit_rmimm(context, _GetOperandSize, _GetOperandSize, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rmw(context, _GetOperandSize, LIB_FALSE));
    CPU_TRACE_CHECK_RETURN(_a_btc(context, (lib_u32)instruction_state.data.crm, (lib_u32)instruction_state.data.cimm, _GetOperandSize * 8));
    instruction_state.data.crm = instruction_state.data.result;
    CPU_TRACE_CHECK_RETURN(_m_write_rm(context, _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void BSF_R32_RM32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("BSF_R32_RM32");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_a_bscc(context, instruction_state.data.crm, _GetOperandSize * 8, 1));
    if (!_GetEFLAGS_ZF)
    {
        CPU_TRACE_BLOCK_BEGIN("EFLAGS_ZF(0)");
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
static void BSR_R32_RM32(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("BSR_R32_RM32");
    _new_code_path_;
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, _GetOperandSize));
    CPU_TRACE_CHECK_RETURN(_a_bscc(context, instruction_state.data.crm, _GetOperandSize * 8, 0));
    if (!_GetEFLAGS_ZF)
    {
        CPU_TRACE_BLOCK_BEGIN("EFLAGS_ZF(0)");
        CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.result), _GetOperandSize));
        CPU_TRACE_BLOCK_END;
    }
    CPU_TRACE_CALL_END;
}
static void MOVSX_R32_RM8(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOVSX_R32_RM8");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, 1));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 1));
    instruction_state.data.crm = (lib_i8)instruction_state.data.crm;
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
    CPU_TRACE_CALL_END;
}
static void MOVSX_R32_RM16(x86_core_cpu_execution_context *context)
{
    CPU_TRACE_CALL_BEGIN("MOVSX_R32_RM16");
    _adv;
    CPU_TRACE_CHECK_RETURN(_d_modrm(context, _GetOperandSize, 2));
    CPU_TRACE_CHECK_RETURN(_m_read_rm(context, 2));
    instruction_state.data.crm = (lib_i16)instruction_state.data.crm;
    CPU_TRACE_CHECK_RETURN(_kma_write_ref(context, instruction_state.data.rr, X86_CPU_REFERENCE_OF(instruction_state.data.crm), _GetOperandSize));
    CPU_TRACE_CALL_END;
}


static void ExecInit(x86_core_cpu_execution_context *context)
{
    ExecBoundary(context);
    if (context->prefetch_expected_valid && instruction_state.data.linear !=
        context->prefetch_expected_linear) x86_core_cpu_execution_invalidate_prefetch(context);
    if (!context->prefetch_valid || instruction_state.data.linear <
        context->prefetch_linear || instruction_state.data.linear -
        context->prefetch_linear >= context->prefetch_count) {
        lib_u8 prefetch_bytes = context->prefetch_capacity;
        lib_u32 page_bytes = _GetPageSize - _GetLinear_Offset(instruction_state.data.linear);

        x86_core_cpu_execution_invalidate_prefetch(context);
        if (prefetch_bytes > page_bytes) prefetch_bytes = (lib_u8)page_bytes;
        if (cpu_state.data.eip <= cpu_state.data.cs.limit &&
            (lib_u64)cpu_state.data.cs.limit - cpu_state.data.eip + 1u <
                prefetch_bytes) {
            prefetch_bytes = (lib_u8)((lib_u64)
                cpu_state.data.cs.limit - cpu_state.data.eip + 1u);
        }
        context->memory_access_provenance = X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH;
        _kma_read_logical(context, &cpu_state.data.cs, cpu_state.data.eip,
            (lib_uptr)context->prefetch_bytes, prefetch_bytes,
            _GetCPL, 1);
        context->memory_access_provenance = X86_CORE_CPU_MEMORY_ACCESS_DATA;
        if (!instruction_state.data.except) {
            context->prefetch_linear = instruction_state.data.linear;
            context->prefetch_count = prefetch_bytes;
            context->prefetch_valid = LIB_TRUE;
        }
    }
    if (context->prefetch_valid && instruction_state.data.linear >=
        context->prefetch_linear && instruction_state.data.linear -
        context->prefetch_linear < context->prefetch_count) {
        lib_u8 available = context->prefetch_count -
            (lib_u8)(instruction_state.data.linear - context->prefetch_linear);
        lib_memory_copy(instruction_state.data.opcodes, context->prefetch_bytes +
            (instruction_state.data.linear - context->prefetch_linear), available);
        instruction_state.data.oplen = available;
    }

    /* Entry preparation may fetch through this helper, but it does not begin
     * another instruction or replace the pending retirement observation. */
    if (context->instruction_in_progress && context->diagnostic_provider != LIB_NULL &&
        context->diagnostic_provider->record_instruction != LIB_NULL)
    {
        x86_core_cpu_instruction_observation observation;

        x86_core_cpu_execution_copy_observation(context, &observation);
        context->diagnostic_provider->record_instruction(context->diagnostic_context,
            &observation);
    }
}

lib_u8 x86_core_cpu_execution_preview_lexeme(
    const x86_core_cpu_execution_context *context,
    x86_core_cpu_instruction_lexeme *out_lexeme)
{
    x86_core_cpu_execution_context preview;
    t_cpu preview_cpu;
    t_cpuins preview_instructions;

    if (out_lexeme == LIB_NULL) return LIB_FALSE;
    *out_lexeme = (x86_core_cpu_instruction_lexeme) { 0u, 0u, LIB_FALSE };
    if (context == LIB_NULL || context->cpu == LIB_NULL ||
        context->instructions == LIB_NULL || context->bus == LIB_NULL ||
        context->bus->read_memory == LIB_NULL) {
        return LIB_FALSE;
    }
    preview_cpu = *context->cpu;
    preview_instructions = *context->instructions;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) {
        preview = *context;
    } else {
        /* Old-family previews never access 486 cache/TLB storage. Preserve the
         * scalar execution state without copying a dormant 12-KiB cache. */
        const lib_size first = lib_offsetof(x86_core_cpu_execution_context, tlb);
        const lib_size last = lib_offsetof(x86_core_cpu_execution_context,
            memory_access_provenance);
        lib_memory_copy(&preview, context, first);
        lib_memory_copy((lib_u8 *)&preview + last, (const lib_u8 *)context + last,
            sizeof(preview) - last);
    }
    preview.cpu = &preview_cpu;
    preview.instructions = &preview_instructions;
    preview.diagnostic_provider = LIB_NULL;
    preview.diagnostic_context = LIB_NULL;
    preview.preview_mode = LIB_TRUE;
    ExecInit(&preview);
    if (preview_instructions.data.except) return LIB_FALSE;
    for (;;) {
        lib_u8 byte;
        lib_u32 offset;

        if (x86_core_cpu_instruction_lexeme_scan_with_options(
                preview_instructions.data.opcodes, preview_instructions.data.oplen,
                preview.cpu_profile, preview_cpu.data.cs.seg.exec.defsize,
                preview.cpu_80386_cr_mov_ignores_mod, out_lexeme)) return LIB_TRUE;
        if (preview_instructions.data.oplen >= sizeof(preview_instructions.data.opcodes))
            return LIB_FALSE;
        offset = preview_instructions.data.receip + preview_instructions.data.oplen;
        if (!cpu_profile_has_protected_architecture(preview.cpu_profile))
            offset = X86_CPU_MASK_U16(offset);
        _s_read_cs(&preview, offset, (lib_uptr)&byte, 1u);
        if (preview_instructions.data.except) return LIB_FALSE;
    }
}

static lib_u32 _debug_breakpoint_address(lib_u8 index,
    const t_cpu *cpu)
{
    switch (index) {
    case 0u: return cpu->data.dr0;
    case 1u: return cpu->data.dr1;
    case 2u: return cpu->data.dr2;
    case 3u: return cpu->data.dr3;
    default: return 0u;
    }
}

static lib_u8 _debug_breakpoint_enabled(lib_u8 index,
    const t_cpu *cpu)
{
    lib_u32 enable_mask = ((lib_u32)1u << (index * 2u)) |
        ((lib_u32)1u << (index * 2u + 1u));

    return (cpu->data.dr7 & enable_mask) != 0u;
}

static lib_u8 _debug_breakpoint_length(lib_u8 length)
{
    switch (length) {
    case 0u: return 1u;
    case 1u: return 2u;
    case 3u: return 4u;
    default: return 0u;
    }
}

static lib_u32 _debug_match_instruction_breakpoint(
    x86_core_cpu_execution_context *context, lib_u32 *matched)
{
    lib_u32 enabled = 0u;
    lib_u8 index;

    *matched = 0u;
    if (!cpu_profile_has_32bit_architecture(context->cpu_profile) ||
        context->debug_rf_before || context->debug_segment_shadow_before) return 0u;
    for (index = 0u; index < 4u; ++index) {
        lib_u32 control = (cpu_state.data.dr7 >> (16u + index * 4u)) &
            0x0fu;

        if ((control & 3u) != 0u || (control >> 2u) != 0u ||
            _debug_breakpoint_address(index, &cpu_state) !=
                instruction_state.data.linear) continue;
        *matched |= (lib_u32)1u << index;
        if (_debug_breakpoint_enabled(index, &cpu_state)) {
            enabled |= (lib_u32)1u << index;
        }
    }
    return enabled;
}

static lib_u32 _debug_match_data_breakpoint(
    x86_core_cpu_execution_context *context, lib_u32 *matched)
{
    lib_u32 enabled = 0u;
    lib_u8 index;
    lib_u16 access_index;

    *matched = 0u;
    if (!cpu_profile_has_32bit_architecture(context->cpu_profile)) return 0u;
    for (access_index = 0u; access_index < instruction_state.data.msize;
        ++access_index) {
        const t_cpuins_data_memory *access =
            &instruction_state.data.mem[access_index];

        for (index = 0u; index < 4u; ++index) {
            lib_u32 control =
                (cpu_state.data.dr7 >> (16u + index * 4u)) & 0x0fu;
            lib_u8 rw = control & 3u;
            lib_u8 length = _debug_breakpoint_length(control >> 2u);
            lib_u64 address;
            lib_u64 first;
            lib_u64 last;

            if (access->byte == 0u || rw == 0u || rw == 2u ||
                (rw == 1u && !access->flagWrite) ||
                length == 0u) continue;
            address = _debug_breakpoint_address(index, &cpu_state) &
                ~(lib_u32)(length - 1u);
            first = access->linear;
            last = first + access->byte - 1u;
            if (last < address || first >= address + length) continue;
            *matched |= (lib_u32)1u << index;
            if (_debug_breakpoint_enabled(index, &cpu_state)) {
                enabled |= (lib_u32)1u << index;
            }
        }
    }
    return enabled;
}

static void _debug_schedule_trap(x86_core_cpu_execution_context *context,
    lib_u32 cause)
{
    if (cause == 0u) return;
    cpu_state.data.dr6 |= cause;
    context->debug_trap_pending = LIB_TRUE;
    context->debug_trap_cause |= cause;
}

static void _debug_complete_instruction(
    x86_core_cpu_execution_context *context, lib_u8 opcode)
{
    lib_u32 cause;
    lib_u32 matched = 0u;

    if (instruction_state.data.except) return;
    cause = context->interrupt_shadow == CPU_INTERRUPT_SHADOW_SEGMENT ?
        0u : _debug_match_data_breakpoint(context, &matched);
    if (cause != 0u) cpu_state.data.dr6 |= matched;
    /* Explicit interrupt and task entry suppress the outgoing step trap;
     * ordinary completion uses TF sampled before execution, including POPF. */
    if (context->debug_tf_before && !context->instruction_task_switched &&
        context->interrupt_shadow != CPU_INTERRUPT_SHADOW_SEGMENT &&
        opcode != 0xf1u && opcode != 0xccu && opcode != 0xcdu && !(opcode == 0xceu &&
            X86_CPU_BIT_IS_SET(instruction_state.data.oldcpu.data.eflags,
                VCPU_EFLAGS_OF))) cause |= VCPU_DR6_BS;
    _debug_schedule_trap(context, cause);
    if (context->debug_rf_before && opcode != 0xcfu && opcode != 0x9du &&
        !context->instruction_task_switched) _ClrEFLAGS_RF;
}

static void _debug_deliver_trap(x86_core_cpu_execution_context *context)
{
    t_cpu trap_cpu;
    lib_u32 cause;

    if (!context->debug_trap_pending ||
        context->interrupt_shadow == CPU_INTERRUPT_SHADOW_SEGMENT) return;
    cause = context->debug_trap_cause;
    context->debug_trap_pending = LIB_FALSE;
    context->debug_trap_cause = 0u;
    trap_cpu = cpu_state;
    /* PRM Table 9-2: this trap precedes a fault fetching the next instruction. */
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) ExecBoundary(context);
    else ExecInit(context);
    /* The completed instruction's size does not select an exception frame. */
    _e_except_n(context, 0x01u, 2u);
    if (!instruction_state.data.except && context->diagnostic_provider !=
        LIB_NULL && context->diagnostic_provider->record_delivered_exception !=
        LIB_NULL) {
        instruction_state.data.except = VCPUINS_EXCEPT_DB;
        instruction_state.data.excode = cause;
        x86_core_cpu_diagnostic_publish_snapshot(
            context->diagnostic_provider->record_delivered_exception,
            context->diagnostic_context, &trap_cpu, &instruction_state);
        instruction_state.data.except = 0u;
        instruction_state.data.excode = 0u;
    }
    ExecFinal(context);
}
static lib_u8 _e_exception_vector(x86_core_cpu_execution_context *context,
    lib_u32 exception)
{
    const lib_u32 supported = VCPUINS_EXCEPT_DE | VCPUINS_EXCEPT_DB |
        VCPUINS_EXCEPT_BR | VCPUINS_EXCEPT_UD | VCPUINS_EXCEPT_NM |
        VCPUINS_EXCEPT_DF | VCPUINS_EXCEPT_09 | VCPUINS_EXCEPT_TS |
        VCPUINS_EXCEPT_NP | VCPUINS_EXCEPT_SS | VCPUINS_EXCEPT_GP |
        VCPUINS_EXCEPT_PF | VCPUINS_EXCEPT_MF | VCPUINS_EXCEPT_AC;
    lib_u8 vector;

    if ((exception & supported) == 0u) return 0xffu;
    /* Unsupported NEC forms are diagnostics, not an invented hardware INT 6. */
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_V30 &&
        exception == VCPUINS_EXCEPT_UD) return 0xffu;
    for (vector = 0u; vector <= 17u; ++vector) {
        if (exception != (UINT32_C(1) << vector)) continue;
        /* Earlier chips have no protected-mode delivery or vectors 8-16,
         * except the 8087 WAIT-error interface, which reports #MF through
         * the real-mode vector-16 table.  A strict unsupported-op diagnostic
         * is not hardware INT 6. */
        if (!cpu_profile_has_protected_architecture(context->cpu_profile) &&
            (X86_CPU_BIT_IS_SET(cpu_state.data.cr0, VCPU_CR0_PE) ||
             (vector >= 8u && exception != VCPUINS_EXCEPT_MF)))
            return 0xffu;
        if (!cpu_profile_has_186_integer(context->cpu_profile) &&
            vector >= 5u && exception != VCPUINS_EXCEPT_MF)
            return 0xffu;
        if (!cpu_profile_has_32bit_architecture(context->cpu_profile) &&
            vector == 14u) return 0xffu;
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX &&
            vector == 17u) return 0xffu;
        if (!X86_CPU_BIT_IS_SET(cpu_state.data.cr0, VCPU_CR0_PE) &&
            (vector == 9u || vector == 10u || vector == 11u || vector == 14u))
            return 0xffu;
        return vector;
    }
    return 0xffu;
}

static void _e_mark_instruction_fault_delivered(
    x86_core_cpu_execution_context *context)
{
    if (context != LIB_NULL && context->instruction_in_progress)
        context->instruction_fault_delivered = LIB_TRUE;
}

static void ExecFinal(x86_core_cpu_execution_context *context)
{
    t_cpu fault_cpu;
    const lib_u32 next_eip = cpu_state.data.eip;
    lib_u32 active, code, secondary, secondary_code;
    lib_u8 vector;

    _kma_complete_memory_lock(context, instruction_state.data.except == 0u);
    if (instruction_state.data.flagInsLoop) {
        cpu_state.data.cs = instruction_state.data.oldcpu.data.cs;
        cpu_state.data.eip = instruction_state.data.oldcpu.data.eip;
    }
    if (!instruction_state.data.except) {
        if (!instruction_state.data.flagInsLoop)
            context->repeat_compare_flags_valid = LIB_FALSE;
        return;
    }
    _e_timing_failed_prefix(context);
    fault_cpu = context->instruction_task_switched ?
        context->instruction_task_checkpoint : instruction_state.data.oldcpu;
    if (context->repeat_compare_flags_valid &&
        X86_CPU_BIT_IS_SET(instruction_state.data.except,
            VCPUINS_EXCEPT_GP | VCPUINS_EXCEPT_SS | VCPUINS_EXCEPT_PF | VCPUINS_EXCEPT_AC))
        fault_cpu.data.eflags = context->repeat_compare_flags;
    context->repeat_compare_flags_valid = LIB_FALSE;
    active = instruction_state.data.except;
    code = instruction_state.data.excode;
    if (active == VCPUINS_EXCEPT_DB) fault_cpu.data.dr6 = cpu_state.data.dr6;
    if (active == VCPUINS_EXCEPT_PF) fault_cpu.data.cr2 = cpu_state.data.cr2;

    for (;;) {
        if (active == VCPUINS_EXCEPT_SHUTDOWN) {
            cpu_state = fault_cpu;
            instruction_state.data.except = active;
            instruction_state.data.excode = code;
            if (context->diagnostic_provider != LIB_NULL &&
                context->diagnostic_provider->record_delivered_exception != LIB_NULL)
                x86_core_cpu_diagnostic_publish_snapshot(
                    context->diagnostic_provider->record_delivered_exception,
                    context->diagnostic_context, &fault_cpu, &instruction_state);
            x86_core_cpu_execution_request_shutdown(context);
            _e_mark_instruction_fault_delivered(context);
            return;
        }
        vector = _e_exception_vector(context, active);
        if (vector == 0xffu) break;
        cpu_state = fault_cpu;
        if (active == VCPUINS_EXCEPT_DE &&
            !cpu_profile_has_186_integer(context->cpu_profile))
            cpu_state.data.eip = next_eip;
        instruction_state.data.except = active;
        instruction_state.data.excode = code;
        /* Exceptions do not inherit the interrupted instruction's override.
         * Protected entry selects its own gate/TSS width. */
        _e_except_n(context, vector, 2u);
        if (!instruction_state.data.except) {
            instruction_state.data.except = active;
            instruction_state.data.excode = code;
            if (context->diagnostic_provider != LIB_NULL &&
                context->diagnostic_provider->record_delivered_exception != LIB_NULL)
                x86_core_cpu_diagnostic_publish_snapshot(
                    context->diagnostic_provider->record_delivered_exception,
                    context->diagnostic_context, &fault_cpu, &instruction_state);
            instruction_state.data.except = 0u;
            _e_mark_instruction_fault_delivered(context);
            return;
        }
        secondary = instruction_state.data.except;
        secondary_code = instruction_state.data.excode;
        if (context->instruction_task_switched)
            fault_cpu = context->instruction_task_checkpoint;
        if (secondary == VCPUINS_EXCEPT_PF)
            fault_cpu.data.cr2 = cpu_state.data.cr2;
        if (secondary == VCPUINS_EXCEPT_SHUTDOWN) {
            active = secondary;
            code = secondary_code;
            continue;
        }
        /* A failed provider is not an architectural second exception. */
        if (_e_exception_vector(context, secondary) == 0xffu) {
            active = secondary;
            code = secondary_code;
            break;
        }
        if (active == VCPUINS_EXCEPT_DF) {
            active = VCPUINS_EXCEPT_SHUTDOWN;
            code = 0u;
            continue;
        }
        if (cpu_exception_requires_double_fault(context->cpu_profile, active, secondary)) {
            active = VCPUINS_EXCEPT_DF;
            code = 0u;
        } else {
            active = secondary;
            code = secondary_code;
        }
    }
    cpu_state = fault_cpu;
    instruction_state.data.except = active;
    instruction_state.data.excode = code;
    if (context->diagnostic_provider != LIB_NULL &&
        context->diagnostic_provider->record_fault != LIB_NULL)
        x86_core_cpu_diagnostic_publish_snapshot(
            context->diagnostic_provider->record_fault,
            context->diagnostic_context, &fault_cpu, &instruction_state);
    x86_core_cpu_execution_request_stop(context);
}
static void ExecIns(x86_core_cpu_execution_context *context)
{
    lib_u8 opcode = 0;
    lib_u32 debug_cause;
    lib_u32 debug_matches;
    const lib_bool smm_before = context->smm_active;

    context->io_restart_valid = LIB_FALSE;
    ExecInit(context);
    if (instruction_state.data.except) {
        ExecFinal(context);
        return;
    }
    debug_cause = _debug_match_instruction_breakpoint(context, &debug_matches);
    if (debug_cause != 0u) {
        cpu_state.data.dr6 |= debug_matches;
        instruction_state.data.except = VCPUINS_EXCEPT_DB;
        ExecFinal(context);
        return;
    }
    if (_e_vm86_interrupts_virtualized(context) && _GetEFLAGS_VIF && _GetEFLAGS_VIP) {
        instruction_state.data.except = VCPUINS_EXCEPT_GP;
        instruction_state.data.excode = 0u;
        ExecFinal(context);
        return;
    }
    do
    {
        CPU_TRACE_CALL_BEGIN("ExecIns");
        CPU_TRACE_CHECK_BREAK(_s_read_cs(context, cpu_state.data.eip, X86_CPU_REFERENCE_OF(opcode), 1));
        if (!x86_core_cpu_profile_allows_form(context,
                                                  X86_CORE_CPU_INSTRUCTION_PRIMARY, opcode, 0u))
        {
            UndefinedOpcode(context);
            break;
        }
        if (!_kdf_check_prefix(context, opcode)) {
            const lib_bool comparing = context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
                instruction_state.data.prefix_rep != PREFIX_REP_NONE &&
                ((opcode & 0xfeu) == 0xa6u || (opcode & 0xfeu) == 0xaeu);

            if (comparing && !context->repeat_compare_flags_valid) {
                context->repeat_compare_flags = instruction_state.data.oldcpu.data.eflags;
                context->repeat_compare_flags_valid = LIB_TRUE;
            } else if (!comparing) context->repeat_compare_flags_valid = LIB_FALSE;
        }
        CPU_TRACE_CHECK_BREAK(ExecCpuInstruction(instruction_state.connect.insTable[opcode]));
        CPU_TRACE_CALL_END;
    } while (_kdf_check_prefix(context, opcode));
    if (instruction_state.data.except) context->io_restart_valid = LIB_FALSE;
    if (!smm_before || context->smm_active) _debug_complete_instruction(context, opcode);
    if (instruction_state.data.flagWE && instruction_state.data.weLinear == instruction_state.data.linear)
    {
        _debug_record_watchpoint(context, X86_CORE_CPU_WATCH_EXECUTE,
            instruction_state.data.weLinear);
    }
    ExecFinal(context);
}

static void x86_core_cpu_legacy_string_interrupt_resume(
    x86_core_cpu_execution_context *context)
{
    lib_u8 prefixes = 0u;

    if (cpu_profile_has_186_integer(context->cpu_profile) ||
        !instruction_state.data.flagInsLoop) return;
    while (prefixes < instruction_state.data.oplen &&
        _kdf_check_prefix(context, instruction_state.data.opcodes[prefixes]))
        ++prefixes;
    if (prefixes > 1u)
        cpu_state.data.eip = X86_CPU_MASK_U16(
            instruction_state.data.oldcpu.data.eip + prefixes - 1u);
}

static void ExecInt(x86_core_cpu_execution_context *context)
{
    lib_u8 intr = 0x00;
    lib_status acknowledge_status;
    const lib_bool debug_first =
        cpu_profile_has_protected_architecture(context->cpu_profile);

    if (context->stop_requested || x86_core_cpu_is_system_waiting(context)) return;
    if (_system_flush_pending(context) && context->flush_asserted) return;
    if (_smm_enter(context)) {
        if (instruction_state.data.except) ExecFinal(context);
        return;
    }
    if (cpu_state.data.flagHalt) {
        const lib_bool nmi_ready =
            context->interrupt_shadow != CPU_INTERRUPT_SHADOW_SEGMENT &&
            !context->nmi_masked && !context->nmi_in_service && context->nmi_pending;
        const lib_bool intr_ready =
            context->interrupt_shadow == CPU_INTERRUPT_SHADOW_NONE && _GetEFLAGS_IF &&
            context->bus->interrupt_pending(context->bus_context);
        /* A queued step trap cannot wake HALT. Recheck delivery inputs after
         * the accepted wake, since handler entry can itself produce an edge. */
        if (!nmi_ready && !intr_ready) return;
        context->wait_state = CPU_WAIT_STATE_NONE;
        context->wait_poll_remaining_ticks = 0u;
        cpu_state.data.flagHalt = LIB_FALSE;
    }
    if (debug_first) _debug_deliver_trap(context);
    if (context->stop_requested) return;
    /* hardware interrupt handler */
    if (context->interrupt_shadow != CPU_INTERRUPT_SHADOW_SEGMENT &&
        !context->nmi_masked && !context->nmi_in_service && context->nmi_pending)
    {
        context->nmi_pending = LIB_FALSE;
        context->nmi_in_service =
            cpu_profile_has_protected_architecture(context->cpu_profile);
        x86_core_cpu_legacy_string_interrupt_resume(context);
        /* SX checkpoints the accepted entry without fetching an interrupted
         * next instruction. Older-family prefetch behavior remains intact. */
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) ExecBoundary(context);
        else ExecInit(context);
        _e_intr_n(context, 0x02, _GetOperandSize, LIB_TRUE);
        if (!instruction_state.data.except) {
            cpu_state.data.flagHalt = LIB_FALSE;
        }
        else context->nmi_pending = LIB_TRUE;
        ExecFinal(context);
    }
    if (context->stop_requested) return;
    if (context->interrupt_shadow == CPU_INTERRUPT_SHADOW_NONE && _GetEFLAGS_IF &&
        context->bus->interrupt_pending(context->bus_context))
    {
        acknowledge_status = context->bus->acknowledge_interrupt(
            context->bus_context, &intr);
        if (acknowledge_status != LIB_STATUS_OK) {
            if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) ExecBoundary(context);
            else ExecInit(context);
            _SetExcept_CE((lib_u32)acknowledge_status);
            ExecFinal(context);
            return;
        }
        x86_core_cpu_legacy_string_interrupt_resume(context);
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) ExecBoundary(context);
        else ExecInit(context);
        _e_intr_n(context, intr, _GetOperandSize, LIB_FALSE);
        if (!instruction_state.data.except) {
            cpu_state.data.flagHalt = LIB_FALSE;
            instruction_state.data.flagIgnore = LIB_TRUE;
        }
        ExecFinal(context);
    }
    if (!context->stop_requested && !debug_first) _debug_deliver_trap(context);
}

/* external interface */
lib_u8 x86_core_cpu_execution_load_segment(
    x86_core_cpu_execution_context *context, t_cpu_data_sreg *rsreg,
    lib_u16 selector)
{
    lib_u8 fail;
    lib_u32 oldexcept = instruction_state.data.except;
    instruction_state.data.except = 0;
    _ksa_load_sreg(context, rsreg, selector);
    fail = !!instruction_state.data.except;
    instruction_state.data.except = oldexcept;
    return fail;
}
lib_u8 x86_core_cpu_execution_read_linear(
    x86_core_cpu_execution_context *context, lib_u32 linear,
    lib_uptr rdata, lib_u8 byte)
{
    lib_u8 fail;
    lib_u32 oldexcept = instruction_state.data.except;
    instruction_state.data.except = 0;
    _kma_read_linear(context, linear, rdata, byte, 0x00, 1);
    fail = !!instruction_state.data.except;
    instruction_state.data.except = oldexcept;
    return fail;
}
lib_u8 x86_core_cpu_execution_write_linear(
    x86_core_cpu_execution_context *context, lib_u32 linear,
    lib_uptr rdata, lib_u8 byte)
{
    lib_u8 fail;
    lib_u32 oldexcept = instruction_state.data.except;
    instruction_state.data.except = 0;
    _kma_write_linear(context, linear, rdata, byte, 0x00, 1);
    fail = !!instruction_state.data.except;
    instruction_state.data.except = oldexcept;
    return fail;
}

void x86_core_cpu_execution_initialize(
    x86_core_cpu_execution_context *context)
{
    instruction_state.connect.insTable[0x00] = (x86_core_cpu_instruction_handler)ADD_RM8_R8;
    instruction_state.connect.insTable[0x01] = (x86_core_cpu_instruction_handler)ADD_RM32_R32;
    instruction_state.connect.insTable[0x02] = (x86_core_cpu_instruction_handler)ADD_R8_RM8;
    instruction_state.connect.insTable[0x03] = (x86_core_cpu_instruction_handler)ADD_R32_RM32;
    instruction_state.connect.insTable[0x04] = (x86_core_cpu_instruction_handler)ADD_AL_I8;
    instruction_state.connect.insTable[0x05] = (x86_core_cpu_instruction_handler)ADD_EAX_I32;
    instruction_state.connect.insTable[0x06] = (x86_core_cpu_instruction_handler)PUSH_ES;
    instruction_state.connect.insTable[0x07] = (x86_core_cpu_instruction_handler)POP_ES;
    instruction_state.connect.insTable[0x08] = (x86_core_cpu_instruction_handler)OR_RM8_R8;
    instruction_state.connect.insTable[0x09] = (x86_core_cpu_instruction_handler)OR_RM32_R32;
    instruction_state.connect.insTable[0x0a] = (x86_core_cpu_instruction_handler)OR_R8_RM8;
    instruction_state.connect.insTable[0x0b] = (x86_core_cpu_instruction_handler)OR_R32_RM32;
    instruction_state.connect.insTable[0x0c] = (x86_core_cpu_instruction_handler)OR_AL_I8;
    instruction_state.connect.insTable[0x0d] = (x86_core_cpu_instruction_handler)OR_EAX_I32;
    instruction_state.connect.insTable[0x0e] = (x86_core_cpu_instruction_handler)PUSH_CS;
    instruction_state.connect.insTable[0x0f] = (x86_core_cpu_instruction_handler)INS_0F;
    instruction_state.connect.insTable[0x10] = (x86_core_cpu_instruction_handler)ADC_RM8_R8;
    instruction_state.connect.insTable[0x11] = (x86_core_cpu_instruction_handler)ADC_RM32_R32;
    instruction_state.connect.insTable[0x12] = (x86_core_cpu_instruction_handler)ADC_R8_RM8;
    instruction_state.connect.insTable[0x13] = (x86_core_cpu_instruction_handler)ADC_R32_RM32;
    instruction_state.connect.insTable[0x14] = (x86_core_cpu_instruction_handler)ADC_AL_I8;
    instruction_state.connect.insTable[0x15] = (x86_core_cpu_instruction_handler)ADC_EAX_I32;
    instruction_state.connect.insTable[0x16] = (x86_core_cpu_instruction_handler)PUSH_SS;
    instruction_state.connect.insTable[0x17] = (x86_core_cpu_instruction_handler)POP_SS;
    instruction_state.connect.insTable[0x18] = (x86_core_cpu_instruction_handler)SBB_RM8_R8;
    instruction_state.connect.insTable[0x19] = (x86_core_cpu_instruction_handler)SBB_RM32_R32;
    instruction_state.connect.insTable[0x1a] = (x86_core_cpu_instruction_handler)SBB_R8_RM8;
    instruction_state.connect.insTable[0x1b] = (x86_core_cpu_instruction_handler)SBB_R32_RM32;
    instruction_state.connect.insTable[0x1c] = (x86_core_cpu_instruction_handler)SBB_AL_I8;
    instruction_state.connect.insTable[0x1d] = (x86_core_cpu_instruction_handler)SBB_EAX_I32;
    instruction_state.connect.insTable[0x1e] = (x86_core_cpu_instruction_handler)PUSH_DS;
    instruction_state.connect.insTable[0x1f] = (x86_core_cpu_instruction_handler)POP_DS;
    instruction_state.connect.insTable[0x20] = (x86_core_cpu_instruction_handler)AND_RM8_R8;
    instruction_state.connect.insTable[0x21] = (x86_core_cpu_instruction_handler)AND_RM32_R32;
    instruction_state.connect.insTable[0x22] = (x86_core_cpu_instruction_handler)AND_R8_RM8;
    instruction_state.connect.insTable[0x23] = (x86_core_cpu_instruction_handler)AND_R32_RM32;
    instruction_state.connect.insTable[0x24] = (x86_core_cpu_instruction_handler)AND_AL_I8;
    instruction_state.connect.insTable[0x25] = (x86_core_cpu_instruction_handler)AND_EAX_I32;
    instruction_state.connect.insTable[0x26] = (x86_core_cpu_instruction_handler)PREFIX_ES;
    instruction_state.connect.insTable[0x27] = (x86_core_cpu_instruction_handler)DAA;
    instruction_state.connect.insTable[0x28] = (x86_core_cpu_instruction_handler)SUB_RM8_R8;
    instruction_state.connect.insTable[0x29] = (x86_core_cpu_instruction_handler)SUB_RM32_R32;
    instruction_state.connect.insTable[0x2a] = (x86_core_cpu_instruction_handler)SUB_R8_RM8;
    instruction_state.connect.insTable[0x2b] = (x86_core_cpu_instruction_handler)SUB_R32_RM32;
    instruction_state.connect.insTable[0x2c] = (x86_core_cpu_instruction_handler)SUB_AL_I8;
    instruction_state.connect.insTable[0x2d] = (x86_core_cpu_instruction_handler)SUB_EAX_I32;
    instruction_state.connect.insTable[0x2e] = (x86_core_cpu_instruction_handler)PREFIX_CS;
    instruction_state.connect.insTable[0x2f] = (x86_core_cpu_instruction_handler)DAS;
    instruction_state.connect.insTable[0x30] = (x86_core_cpu_instruction_handler)XOR_RM8_R8;
    instruction_state.connect.insTable[0x31] = (x86_core_cpu_instruction_handler)XOR_RM32_R32;
    instruction_state.connect.insTable[0x32] = (x86_core_cpu_instruction_handler)XOR_R8_RM8;
    instruction_state.connect.insTable[0x33] = (x86_core_cpu_instruction_handler)XOR_R32_RM32;
    instruction_state.connect.insTable[0x34] = (x86_core_cpu_instruction_handler)XOR_AL_I8;
    instruction_state.connect.insTable[0x35] = (x86_core_cpu_instruction_handler)XOR_EAX_I32;
    instruction_state.connect.insTable[0x36] = (x86_core_cpu_instruction_handler)PREFIX_SS;
    instruction_state.connect.insTable[0x37] = (x86_core_cpu_instruction_handler)AAA;
    instruction_state.connect.insTable[0x38] = (x86_core_cpu_instruction_handler)CMP_RM8_R8;
    instruction_state.connect.insTable[0x39] = (x86_core_cpu_instruction_handler)CMP_RM32_R32;
    instruction_state.connect.insTable[0x3a] = (x86_core_cpu_instruction_handler)CMP_R8_RM8;
    instruction_state.connect.insTable[0x3b] = (x86_core_cpu_instruction_handler)CMP_R32_RM32;
    instruction_state.connect.insTable[0x3c] = (x86_core_cpu_instruction_handler)CMP_AL_I8;
    instruction_state.connect.insTable[0x3d] = (x86_core_cpu_instruction_handler)CMP_EAX_I32;
    instruction_state.connect.insTable[0x3e] = (x86_core_cpu_instruction_handler)PREFIX_DS;
    instruction_state.connect.insTable[0x3f] = (x86_core_cpu_instruction_handler)AAS;
    instruction_state.connect.insTable[0x40] = (x86_core_cpu_instruction_handler)INC_EAX;
    instruction_state.connect.insTable[0x41] = (x86_core_cpu_instruction_handler)INC_ECX;
    instruction_state.connect.insTable[0x42] = (x86_core_cpu_instruction_handler)INC_EDX;
    instruction_state.connect.insTable[0x43] = (x86_core_cpu_instruction_handler)INC_EBX;
    instruction_state.connect.insTable[0x44] = (x86_core_cpu_instruction_handler)INC_ESP;
    instruction_state.connect.insTable[0x45] = (x86_core_cpu_instruction_handler)INC_EBP;
    instruction_state.connect.insTable[0x46] = (x86_core_cpu_instruction_handler)INC_ESI;
    instruction_state.connect.insTable[0x47] = (x86_core_cpu_instruction_handler)INC_EDI;
    instruction_state.connect.insTable[0x48] = (x86_core_cpu_instruction_handler)DEC_EAX;
    instruction_state.connect.insTable[0x49] = (x86_core_cpu_instruction_handler)DEC_ECX;
    instruction_state.connect.insTable[0x4a] = (x86_core_cpu_instruction_handler)DEC_EDX;
    instruction_state.connect.insTable[0x4b] = (x86_core_cpu_instruction_handler)DEC_EBX;
    instruction_state.connect.insTable[0x4c] = (x86_core_cpu_instruction_handler)DEC_ESP;
    instruction_state.connect.insTable[0x4d] = (x86_core_cpu_instruction_handler)DEC_EBP;
    instruction_state.connect.insTable[0x4e] = (x86_core_cpu_instruction_handler)DEC_ESI;
    instruction_state.connect.insTable[0x4f] = (x86_core_cpu_instruction_handler)DEC_EDI;
    instruction_state.connect.insTable[0x50] = (x86_core_cpu_instruction_handler)PUSH_EAX;
    instruction_state.connect.insTable[0x51] = (x86_core_cpu_instruction_handler)PUSH_ECX;
    instruction_state.connect.insTable[0x52] = (x86_core_cpu_instruction_handler)PUSH_EDX;
    instruction_state.connect.insTable[0x53] = (x86_core_cpu_instruction_handler)PUSH_EBX;
    instruction_state.connect.insTable[0x54] = (x86_core_cpu_instruction_handler)PUSH_ESP;
    instruction_state.connect.insTable[0x55] = (x86_core_cpu_instruction_handler)PUSH_EBP;
    instruction_state.connect.insTable[0x56] = (x86_core_cpu_instruction_handler)PUSH_ESI;
    instruction_state.connect.insTable[0x57] = (x86_core_cpu_instruction_handler)PUSH_EDI;
    instruction_state.connect.insTable[0x58] = (x86_core_cpu_instruction_handler)POP_EAX;
    instruction_state.connect.insTable[0x59] = (x86_core_cpu_instruction_handler)POP_ECX;
    instruction_state.connect.insTable[0x5a] = (x86_core_cpu_instruction_handler)POP_EDX;
    instruction_state.connect.insTable[0x5b] = (x86_core_cpu_instruction_handler)POP_EBX;
    instruction_state.connect.insTable[0x5c] = (x86_core_cpu_instruction_handler)POP_ESP;
    instruction_state.connect.insTable[0x5d] = (x86_core_cpu_instruction_handler)POP_EBP;
    instruction_state.connect.insTable[0x5e] = (x86_core_cpu_instruction_handler)POP_ESI;
    instruction_state.connect.insTable[0x5f] = (x86_core_cpu_instruction_handler)POP_EDI;
    instruction_state.connect.insTable[0x60] = (x86_core_cpu_instruction_handler)PUSHA;
    instruction_state.connect.insTable[0x61] = (x86_core_cpu_instruction_handler)POPA;
    instruction_state.connect.insTable[0x62] = (x86_core_cpu_instruction_handler)BOUND_R16_M16_16;
    instruction_state.connect.insTable[0x63] = (x86_core_cpu_instruction_handler)ARPL_RM16_R16;
    instruction_state.connect.insTable[0x64] = (x86_core_cpu_instruction_handler)PREFIX_FS;
    instruction_state.connect.insTable[0x65] = (x86_core_cpu_instruction_handler)PREFIX_GS;
    instruction_state.connect.insTable[0x66] = (x86_core_cpu_instruction_handler)PREFIX_OprSize;
    instruction_state.connect.insTable[0x67] = (x86_core_cpu_instruction_handler)PREFIX_AddrSize;
    instruction_state.connect.insTable[0x68] = (x86_core_cpu_instruction_handler)PUSH_I32;
    instruction_state.connect.insTable[0x69] = (x86_core_cpu_instruction_handler)IMUL_R32_RM32_I32;
    instruction_state.connect.insTable[0x6a] = (x86_core_cpu_instruction_handler)PUSH_I8;
    instruction_state.connect.insTable[0x6b] = (x86_core_cpu_instruction_handler)IMUL_R32_RM32_I8;
    instruction_state.connect.insTable[0x6c] = (x86_core_cpu_instruction_handler)INSB;
    instruction_state.connect.insTable[0x6d] = (x86_core_cpu_instruction_handler)INSW;
    instruction_state.connect.insTable[0x6e] = (x86_core_cpu_instruction_handler)OUTSB;
    instruction_state.connect.insTable[0x6f] = (x86_core_cpu_instruction_handler)OUTSW;
    instruction_state.connect.insTable[0x70] = (x86_core_cpu_instruction_handler)JO_REL8;
    instruction_state.connect.insTable[0x71] = (x86_core_cpu_instruction_handler)JNO_REL8;
    instruction_state.connect.insTable[0x72] = (x86_core_cpu_instruction_handler)JC_REL8;
    instruction_state.connect.insTable[0x73] = (x86_core_cpu_instruction_handler)JNC_REL8;
    instruction_state.connect.insTable[0x74] = (x86_core_cpu_instruction_handler)JZ_REL8;
    instruction_state.connect.insTable[0x75] = (x86_core_cpu_instruction_handler)JNZ_REL8;
    instruction_state.connect.insTable[0x76] = (x86_core_cpu_instruction_handler)JNA_REL8;
    instruction_state.connect.insTable[0x77] = (x86_core_cpu_instruction_handler)JA_REL8;
    instruction_state.connect.insTable[0x78] = (x86_core_cpu_instruction_handler)JS_REL8;
    instruction_state.connect.insTable[0x79] = (x86_core_cpu_instruction_handler)JNS_REL8;
    instruction_state.connect.insTable[0x7a] = (x86_core_cpu_instruction_handler)JP_REL8;
    instruction_state.connect.insTable[0x7b] = (x86_core_cpu_instruction_handler)JNP_REL8;
    instruction_state.connect.insTable[0x7c] = (x86_core_cpu_instruction_handler)JL_REL8;
    instruction_state.connect.insTable[0x7d] = (x86_core_cpu_instruction_handler)JNL_REL8;
    instruction_state.connect.insTable[0x7e] = (x86_core_cpu_instruction_handler)JNG_REL8;
    instruction_state.connect.insTable[0x7f] = (x86_core_cpu_instruction_handler)JG_REL8;
    instruction_state.connect.insTable[0x80] = (x86_core_cpu_instruction_handler)INS_80;
    instruction_state.connect.insTable[0x81] = (x86_core_cpu_instruction_handler)INS_81;
    instruction_state.connect.insTable[0x82] = (x86_core_cpu_instruction_handler)INS_80;
    instruction_state.connect.insTable[0x83] = (x86_core_cpu_instruction_handler)INS_83;
    instruction_state.connect.insTable[0x84] = (x86_core_cpu_instruction_handler)TEST_RM8_R8;
    instruction_state.connect.insTable[0x85] = (x86_core_cpu_instruction_handler)TEST_RM32_R32;
    instruction_state.connect.insTable[0x86] = (x86_core_cpu_instruction_handler)XCHG_RM8_R8;
    instruction_state.connect.insTable[0x87] = (x86_core_cpu_instruction_handler)XCHG_RM32_R32;
    instruction_state.connect.insTable[0x88] = (x86_core_cpu_instruction_handler)MOV_RM8_R8;
    instruction_state.connect.insTable[0x89] = (x86_core_cpu_instruction_handler)MOV_RM32_R32;
    instruction_state.connect.insTable[0x8a] = (x86_core_cpu_instruction_handler)MOV_R8_RM8;
    instruction_state.connect.insTable[0x8b] = (x86_core_cpu_instruction_handler)MOV_R32_RM32;
    instruction_state.connect.insTable[0x8c] = (x86_core_cpu_instruction_handler)MOV_RM16_SREG;
    instruction_state.connect.insTable[0x8d] = (x86_core_cpu_instruction_handler)LEA_R32_M32;
    instruction_state.connect.insTable[0x8e] = (x86_core_cpu_instruction_handler)MOV_SREG_RM16;
    instruction_state.connect.insTable[0x8f] = (x86_core_cpu_instruction_handler)INS_8F;
    instruction_state.connect.insTable[0x90] = (x86_core_cpu_instruction_handler)NOP;
    instruction_state.connect.insTable[0x91] = (x86_core_cpu_instruction_handler)XCHG_ECX_EAX;
    instruction_state.connect.insTable[0x92] = (x86_core_cpu_instruction_handler)XCHG_EDX_EAX;
    instruction_state.connect.insTable[0x93] = (x86_core_cpu_instruction_handler)XCHG_EBX_EAX;
    instruction_state.connect.insTable[0x94] = (x86_core_cpu_instruction_handler)XCHG_ESP_EAX;
    instruction_state.connect.insTable[0x95] = (x86_core_cpu_instruction_handler)XCHG_EBP_EAX;
    instruction_state.connect.insTable[0x96] = (x86_core_cpu_instruction_handler)XCHG_ESI_EAX;
    instruction_state.connect.insTable[0x97] = (x86_core_cpu_instruction_handler)XCHG_EDI_EAX;
    instruction_state.connect.insTable[0x98] = (x86_core_cpu_instruction_handler)CBW;
    instruction_state.connect.insTable[0x99] = (x86_core_cpu_instruction_handler)CWD;
    instruction_state.connect.insTable[0x9a] = (x86_core_cpu_instruction_handler)CALL_PTR16_32;
    instruction_state.connect.insTable[0x9b] = (x86_core_cpu_instruction_handler)WAIT;
    instruction_state.connect.insTable[0x9c] = (x86_core_cpu_instruction_handler)PUSHF;
    instruction_state.connect.insTable[0x9d] = (x86_core_cpu_instruction_handler)POPF;
    instruction_state.connect.insTable[0x9e] = (x86_core_cpu_instruction_handler)SAHF;
    instruction_state.connect.insTable[0x9f] = (x86_core_cpu_instruction_handler)LAHF;
    instruction_state.connect.insTable[0xa0] = (x86_core_cpu_instruction_handler)MOV_AL_MOFFS8;
    instruction_state.connect.insTable[0xa1] = (x86_core_cpu_instruction_handler)MOV_EAX_MOFFS32;
    instruction_state.connect.insTable[0xa2] = (x86_core_cpu_instruction_handler)MOV_MOFFS8_AL;
    instruction_state.connect.insTable[0xa3] = (x86_core_cpu_instruction_handler)MOV_MOFFS32_EAX;
    instruction_state.connect.insTable[0xa4] = (x86_core_cpu_instruction_handler)MOVSB;
    instruction_state.connect.insTable[0xa5] = (x86_core_cpu_instruction_handler)MOVSW;
    instruction_state.connect.insTable[0xa6] = (x86_core_cpu_instruction_handler)CMPSB;
    instruction_state.connect.insTable[0xa7] = (x86_core_cpu_instruction_handler)CMPSW;
    instruction_state.connect.insTable[0xa8] = (x86_core_cpu_instruction_handler)TEST_AL_I8;
    instruction_state.connect.insTable[0xa9] = (x86_core_cpu_instruction_handler)TEST_EAX_I32;
    instruction_state.connect.insTable[0xaa] = (x86_core_cpu_instruction_handler)STOSB;
    instruction_state.connect.insTable[0xab] = (x86_core_cpu_instruction_handler)STOSW;
    instruction_state.connect.insTable[0xac] = (x86_core_cpu_instruction_handler)LODSB;
    instruction_state.connect.insTable[0xad] = (x86_core_cpu_instruction_handler)LODSW;
    instruction_state.connect.insTable[0xae] = (x86_core_cpu_instruction_handler)SCASB;
    instruction_state.connect.insTable[0xaf] = (x86_core_cpu_instruction_handler)SCASW;
    instruction_state.connect.insTable[0xb0] = (x86_core_cpu_instruction_handler)MOV_AL_I8;
    instruction_state.connect.insTable[0xb1] = (x86_core_cpu_instruction_handler)MOV_CL_I8;
    instruction_state.connect.insTable[0xb2] = (x86_core_cpu_instruction_handler)MOV_DL_I8;
    instruction_state.connect.insTable[0xb3] = (x86_core_cpu_instruction_handler)MOV_BL_I8;
    instruction_state.connect.insTable[0xb4] = (x86_core_cpu_instruction_handler)MOV_AH_I8;
    instruction_state.connect.insTable[0xb5] = (x86_core_cpu_instruction_handler)MOV_CH_I8;
    instruction_state.connect.insTable[0xb6] = (x86_core_cpu_instruction_handler)MOV_DH_I8;
    instruction_state.connect.insTable[0xb7] = (x86_core_cpu_instruction_handler)MOV_BH_I8;
    instruction_state.connect.insTable[0xb8] = (x86_core_cpu_instruction_handler)MOV_EAX_I32;
    instruction_state.connect.insTable[0xb9] = (x86_core_cpu_instruction_handler)MOV_ECX_I32;
    instruction_state.connect.insTable[0xba] = (x86_core_cpu_instruction_handler)MOV_EDX_I32;
    instruction_state.connect.insTable[0xbb] = (x86_core_cpu_instruction_handler)MOV_EBX_I32;
    instruction_state.connect.insTable[0xbc] = (x86_core_cpu_instruction_handler)MOV_ESP_I32;
    instruction_state.connect.insTable[0xbd] = (x86_core_cpu_instruction_handler)MOV_EBP_I32;
    instruction_state.connect.insTable[0xbe] = (x86_core_cpu_instruction_handler)MOV_ESI_I32;
    instruction_state.connect.insTable[0xbf] = (x86_core_cpu_instruction_handler)MOV_EDI_I32;
    instruction_state.connect.insTable[0xc0] = (x86_core_cpu_instruction_handler)INS_C0;
    instruction_state.connect.insTable[0xc1] = (x86_core_cpu_instruction_handler)INS_C1;
    instruction_state.connect.insTable[0xc2] = (x86_core_cpu_instruction_handler)RET_I16;
    instruction_state.connect.insTable[0xc3] = (x86_core_cpu_instruction_handler)RET;
    instruction_state.connect.insTable[0xc4] = (x86_core_cpu_instruction_handler)LES_R32_M16_32;
    instruction_state.connect.insTable[0xc5] = (x86_core_cpu_instruction_handler)LDS_R32_M16_32;
    instruction_state.connect.insTable[0xc6] = (x86_core_cpu_instruction_handler)INS_C6;
    instruction_state.connect.insTable[0xc7] = (x86_core_cpu_instruction_handler)INS_C7;
    instruction_state.connect.insTable[0xc8] = (x86_core_cpu_instruction_handler)ENTER;
    instruction_state.connect.insTable[0xc9] = (x86_core_cpu_instruction_handler)LEAVE;
    instruction_state.connect.insTable[0xca] = (x86_core_cpu_instruction_handler)RETF_I16;
    instruction_state.connect.insTable[0xcb] = (x86_core_cpu_instruction_handler)RETF;
    instruction_state.connect.insTable[0xcc] = (x86_core_cpu_instruction_handler)INT3;
    instruction_state.connect.insTable[0xcd] = (x86_core_cpu_instruction_handler)INT_I8;
    instruction_state.connect.insTable[0xce] = (x86_core_cpu_instruction_handler)INTO;
    instruction_state.connect.insTable[0xcf] = (x86_core_cpu_instruction_handler)IRET;
    instruction_state.connect.insTable[0xd0] = (x86_core_cpu_instruction_handler)INS_D0;
    instruction_state.connect.insTable[0xd1] = (x86_core_cpu_instruction_handler)INS_D1;
    instruction_state.connect.insTable[0xd2] = (x86_core_cpu_instruction_handler)INS_D2;
    instruction_state.connect.insTable[0xd3] = (x86_core_cpu_instruction_handler)INS_D3;
    instruction_state.connect.insTable[0xd4] = (x86_core_cpu_instruction_handler)AAM;
    instruction_state.connect.insTable[0xd5] = (x86_core_cpu_instruction_handler)AAD;
    instruction_state.connect.insTable[0xd6] = (x86_core_cpu_instruction_handler)SALC;
    instruction_state.connect.insTable[0xd7] = (x86_core_cpu_instruction_handler)XLAT;
    instruction_state.connect.insTable[0xd8] = (x86_core_cpu_instruction_handler)FPU_ESCAPE;
    instruction_state.connect.insTable[0xd9] = (x86_core_cpu_instruction_handler)FPU_ESCAPE;
    instruction_state.connect.insTable[0xda] = (x86_core_cpu_instruction_handler)FPU_ESCAPE;
    instruction_state.connect.insTable[0xdb] = (x86_core_cpu_instruction_handler)FPU_ESCAPE;
    instruction_state.connect.insTable[0xdc] = (x86_core_cpu_instruction_handler)FPU_ESCAPE;
    instruction_state.connect.insTable[0xdd] = (x86_core_cpu_instruction_handler)FPU_ESCAPE;
    instruction_state.connect.insTable[0xde] = (x86_core_cpu_instruction_handler)FPU_ESCAPE;
    instruction_state.connect.insTable[0xdf] = (x86_core_cpu_instruction_handler)FPU_ESCAPE;
    instruction_state.connect.insTable[0xe0] = (x86_core_cpu_instruction_handler)LOOPNZ_REL8;
    instruction_state.connect.insTable[0xe1] = (x86_core_cpu_instruction_handler)LOOPZ_REL8;
    instruction_state.connect.insTable[0xe2] = (x86_core_cpu_instruction_handler)LOOP_REL8;
    instruction_state.connect.insTable[0xe3] = (x86_core_cpu_instruction_handler)JCXZ_REL8;
    instruction_state.connect.insTable[0xe4] = (x86_core_cpu_instruction_handler)IN_AL_I8;
    instruction_state.connect.insTable[0xe5] = (x86_core_cpu_instruction_handler)IN_EAX_I8;
    instruction_state.connect.insTable[0xe6] = (x86_core_cpu_instruction_handler)OUT_I8_AL;
    instruction_state.connect.insTable[0xe7] = (x86_core_cpu_instruction_handler)OUT_I8_EAX;
    instruction_state.connect.insTable[0xe8] = (x86_core_cpu_instruction_handler)CALL_REL32;
    instruction_state.connect.insTable[0xe9] = (x86_core_cpu_instruction_handler)JMP_REL32;
    instruction_state.connect.insTable[0xea] = (x86_core_cpu_instruction_handler)JMP_PTR16_32;
    instruction_state.connect.insTable[0xeb] = (x86_core_cpu_instruction_handler)JMP_REL8;
    instruction_state.connect.insTable[0xec] = (x86_core_cpu_instruction_handler)IN_AL_DX;
    instruction_state.connect.insTable[0xed] = (x86_core_cpu_instruction_handler)IN_EAX_DX;
    instruction_state.connect.insTable[0xee] = (x86_core_cpu_instruction_handler)OUT_DX_AL;
    instruction_state.connect.insTable[0xef] = (x86_core_cpu_instruction_handler)OUT_DX_EAX;
    instruction_state.connect.insTable[0xf0] = (x86_core_cpu_instruction_handler)PREFIX_LOCK;
    instruction_state.connect.insTable[0xf1] = (x86_core_cpu_instruction_handler)INT1;
    instruction_state.connect.insTable[0xf2] = (x86_core_cpu_instruction_handler)PREFIX_REPNZ;
    instruction_state.connect.insTable[0xf3] = (x86_core_cpu_instruction_handler)PREFIX_REPZ;
    instruction_state.connect.insTable[0xf4] = (x86_core_cpu_instruction_handler)HLT;
    instruction_state.connect.insTable[0xf5] = (x86_core_cpu_instruction_handler)CMC;
    instruction_state.connect.insTable[0xf6] = (x86_core_cpu_instruction_handler)INS_F6;
    instruction_state.connect.insTable[0xf7] = (x86_core_cpu_instruction_handler)INS_F7;
    instruction_state.connect.insTable[0xf8] = (x86_core_cpu_instruction_handler)CLC;
    instruction_state.connect.insTable[0xf9] = (x86_core_cpu_instruction_handler)STC;
    instruction_state.connect.insTable[0xfa] = (x86_core_cpu_instruction_handler)CLI;
    instruction_state.connect.insTable[0xfb] = (x86_core_cpu_instruction_handler)STI;
    instruction_state.connect.insTable[0xfc] = (x86_core_cpu_instruction_handler)CLD;
    instruction_state.connect.insTable[0xfd] = (x86_core_cpu_instruction_handler)STD;
    instruction_state.connect.insTable[0xfe] = (x86_core_cpu_instruction_handler)INS_FE;
    instruction_state.connect.insTable[0xff] = (x86_core_cpu_instruction_handler)INS_FF;
    instruction_state.connect.insTable_0f[0x00] = (x86_core_cpu_instruction_handler)INS_0F_00;
    instruction_state.connect.insTable_0f[0x01] = (x86_core_cpu_instruction_handler)INS_0F_01;
    instruction_state.connect.insTable_0f[0x02] = (x86_core_cpu_instruction_handler)LAR_R32_RM32;
    instruction_state.connect.insTable_0f[0x03] = (x86_core_cpu_instruction_handler)LSL_R32_RM32;
    instruction_state.connect.insTable_0f[0x04] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x05] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x06] = (x86_core_cpu_instruction_handler)CLTS;
    instruction_state.connect.insTable_0f[0x07] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x08] = (x86_core_cpu_instruction_handler)INVD;
    instruction_state.connect.insTable_0f[0x09] = (x86_core_cpu_instruction_handler)WBINVD;
    instruction_state.connect.insTable_0f[0x0a] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x0b] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x0c] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x0d] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x0e] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x0f] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x10] = (x86_core_cpu_instruction_handler)MOV_RM8_R8;
    instruction_state.connect.insTable_0f[0x11] = (x86_core_cpu_instruction_handler)MOV_RM32_R32;
    instruction_state.connect.insTable_0f[0x12] = (x86_core_cpu_instruction_handler)MOV_R8_RM8;
    instruction_state.connect.insTable_0f[0x13] = (x86_core_cpu_instruction_handler)MOV_R32_RM32;
    instruction_state.connect.insTable_0f[0x14] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x15] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x16] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x17] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x18] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x19] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x1a] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x1b] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x1c] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x1d] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x1e] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x1f] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x20] = (x86_core_cpu_instruction_handler)MOV_R32_CR;
    instruction_state.connect.insTable_0f[0x21] = (x86_core_cpu_instruction_handler)MOV_R32_DR;
    instruction_state.connect.insTable_0f[0x22] = (x86_core_cpu_instruction_handler)MOV_CR_R32;
    instruction_state.connect.insTable_0f[0x23] = (x86_core_cpu_instruction_handler)MOV_DR_R32;
    instruction_state.connect.insTable_0f[0x24] = (x86_core_cpu_instruction_handler)MOV_R32_TR;
    instruction_state.connect.insTable_0f[0x25] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x26] = (x86_core_cpu_instruction_handler)MOV_TR_R32;
    instruction_state.connect.insTable_0f[0x27] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x28] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x29] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x2a] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x2b] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x2c] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x2d] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x2e] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x2f] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x30] = (x86_core_cpu_instruction_handler)WRMSR;
    instruction_state.connect.insTable_0f[0x31] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x32] = (x86_core_cpu_instruction_handler)RDMSR;
    instruction_state.connect.insTable_0f[0x33] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x34] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x35] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x36] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x37] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x38] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x39] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x3a] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x3b] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x3c] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x3d] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x3e] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x3f] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x40] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x41] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x42] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x43] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x44] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x45] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x46] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x47] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x48] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x49] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x4a] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x4b] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x4c] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x4d] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x4e] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x4f] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x50] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x51] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x52] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x53] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x54] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x55] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x56] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x57] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x58] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x59] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x5a] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x5b] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x5c] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x5d] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x5e] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x5f] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x60] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x61] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x62] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x63] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x64] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x65] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x66] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x67] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x68] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x69] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x6a] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x6b] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x6c] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x6d] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x6e] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x6f] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x70] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x71] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x72] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x73] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x74] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x75] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x76] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x77] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x78] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x79] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x7a] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x7b] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x7c] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x7d] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x7e] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x7f] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0x80] = (x86_core_cpu_instruction_handler)JO_REL32;
    instruction_state.connect.insTable_0f[0x81] = (x86_core_cpu_instruction_handler)JNO_REL32;
    instruction_state.connect.insTable_0f[0x82] = (x86_core_cpu_instruction_handler)JC_REL32;
    instruction_state.connect.insTable_0f[0x83] = (x86_core_cpu_instruction_handler)JNC_REL32;
    instruction_state.connect.insTable_0f[0x84] = (x86_core_cpu_instruction_handler)JZ_REL32;
    instruction_state.connect.insTable_0f[0x85] = (x86_core_cpu_instruction_handler)JNZ_REL32;
    instruction_state.connect.insTable_0f[0x86] = (x86_core_cpu_instruction_handler)JNA_REL32;
    instruction_state.connect.insTable_0f[0x87] = (x86_core_cpu_instruction_handler)JA_REL32;
    instruction_state.connect.insTable_0f[0x88] = (x86_core_cpu_instruction_handler)JS_REL32;
    instruction_state.connect.insTable_0f[0x89] = (x86_core_cpu_instruction_handler)JNS_REL32;
    instruction_state.connect.insTable_0f[0x8a] = (x86_core_cpu_instruction_handler)JP_REL32;
    instruction_state.connect.insTable_0f[0x8b] = (x86_core_cpu_instruction_handler)JNP_REL32;
    instruction_state.connect.insTable_0f[0x8c] = (x86_core_cpu_instruction_handler)JL_REL32;
    instruction_state.connect.insTable_0f[0x8d] = (x86_core_cpu_instruction_handler)JNL_REL32;
    instruction_state.connect.insTable_0f[0x8e] = (x86_core_cpu_instruction_handler)JNG_REL32;
    instruction_state.connect.insTable_0f[0x8f] = (x86_core_cpu_instruction_handler)JG_REL32;
    instruction_state.connect.insTable_0f[0x90] = (x86_core_cpu_instruction_handler)SETO_RM8;
    instruction_state.connect.insTable_0f[0x91] = (x86_core_cpu_instruction_handler)SETNO_RM8;
    instruction_state.connect.insTable_0f[0x92] = (x86_core_cpu_instruction_handler)SETC_RM8;
    instruction_state.connect.insTable_0f[0x93] = (x86_core_cpu_instruction_handler)SETNC_RM8;
    instruction_state.connect.insTable_0f[0x94] = (x86_core_cpu_instruction_handler)SETZ_RM8;
    instruction_state.connect.insTable_0f[0x95] = (x86_core_cpu_instruction_handler)SETNZ_RM8;
    instruction_state.connect.insTable_0f[0x96] = (x86_core_cpu_instruction_handler)SETNA_RM8;
    instruction_state.connect.insTable_0f[0x97] = (x86_core_cpu_instruction_handler)SETA_RM8;
    instruction_state.connect.insTable_0f[0x98] = (x86_core_cpu_instruction_handler)SETS_RM8;
    instruction_state.connect.insTable_0f[0x99] = (x86_core_cpu_instruction_handler)SETNS_RM8;
    instruction_state.connect.insTable_0f[0x9a] = (x86_core_cpu_instruction_handler)SETP_RM8;
    instruction_state.connect.insTable_0f[0x9b] = (x86_core_cpu_instruction_handler)SETNP_RM8;
    instruction_state.connect.insTable_0f[0x9c] = (x86_core_cpu_instruction_handler)SETL_RM8;
    instruction_state.connect.insTable_0f[0x9d] = (x86_core_cpu_instruction_handler)SETNL_RM8;
    instruction_state.connect.insTable_0f[0x9e] = (x86_core_cpu_instruction_handler)SETNG_RM8;
    instruction_state.connect.insTable_0f[0x9f] = (x86_core_cpu_instruction_handler)SETG_RM8;
    instruction_state.connect.insTable_0f[0xa0] = (x86_core_cpu_instruction_handler)PUSH_FS;
    instruction_state.connect.insTable_0f[0xa1] = (x86_core_cpu_instruction_handler)POP_FS;
    instruction_state.connect.insTable_0f[0xa2] = (x86_core_cpu_instruction_handler)CPUID;
    instruction_state.connect.insTable_0f[0xa3] = (x86_core_cpu_instruction_handler)BT_RM32_R32;
    instruction_state.connect.insTable_0f[0xa4] = (x86_core_cpu_instruction_handler)SHLD_RM32_R32_I8;
    instruction_state.connect.insTable_0f[0xa5] = (x86_core_cpu_instruction_handler)SHLD_RM32_R32_CL;
    instruction_state.connect.insTable_0f[0xa6] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xa7] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xa8] = (x86_core_cpu_instruction_handler)PUSH_GS;
    instruction_state.connect.insTable_0f[0xa9] = (x86_core_cpu_instruction_handler)POP_GS;
    instruction_state.connect.insTable_0f[0xaa] = (x86_core_cpu_instruction_handler)RSM;
    instruction_state.connect.insTable_0f[0xab] = (x86_core_cpu_instruction_handler)BTS_RM32_R32;
    instruction_state.connect.insTable_0f[0xac] = (x86_core_cpu_instruction_handler)SHRD_RM32_R32_I8;
    instruction_state.connect.insTable_0f[0xad] = (x86_core_cpu_instruction_handler)SHRD_RM32_R32_CL;
    instruction_state.connect.insTable_0f[0xae] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xaf] = (x86_core_cpu_instruction_handler)IMUL_R32_RM32;
    instruction_state.connect.insTable_0f[0xb0] = (x86_core_cpu_instruction_handler)CMPXCHG_RM8_R8;
    instruction_state.connect.insTable_0f[0xb1] = (x86_core_cpu_instruction_handler)CMPXCHG_RM32_R32;
    instruction_state.connect.insTable_0f[0xb2] = (x86_core_cpu_instruction_handler)LSS_R32_M16_32;
    instruction_state.connect.insTable_0f[0xb3] = (x86_core_cpu_instruction_handler)BTR_RM32_R32;
    instruction_state.connect.insTable_0f[0xb4] = (x86_core_cpu_instruction_handler)LFS_R32_M16_32;
    instruction_state.connect.insTable_0f[0xb5] = (x86_core_cpu_instruction_handler)LGS_R32_M16_32;
    instruction_state.connect.insTable_0f[0xb6] = (x86_core_cpu_instruction_handler)MOVZX_R32_RM8;
    instruction_state.connect.insTable_0f[0xb7] = (x86_core_cpu_instruction_handler)MOVZX_R32_RM16;
    instruction_state.connect.insTable_0f[0xb8] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xb9] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xba] = (x86_core_cpu_instruction_handler)INS_0F_BA;
    instruction_state.connect.insTable_0f[0xbb] = (x86_core_cpu_instruction_handler)BTC_RM32_R32;
    instruction_state.connect.insTable_0f[0xbc] = (x86_core_cpu_instruction_handler)BSF_R32_RM32;
    instruction_state.connect.insTable_0f[0xbd] = (x86_core_cpu_instruction_handler)BSR_R32_RM32;
    instruction_state.connect.insTable_0f[0xbe] = (x86_core_cpu_instruction_handler)MOVSX_R32_RM8;
    instruction_state.connect.insTable_0f[0xbf] = (x86_core_cpu_instruction_handler)MOVSX_R32_RM16;
    instruction_state.connect.insTable_0f[0xc0] = (x86_core_cpu_instruction_handler)XADD_RM8_R8;
    instruction_state.connect.insTable_0f[0xc1] = (x86_core_cpu_instruction_handler)XADD_RM32_R32;
    instruction_state.connect.insTable_0f[0xc2] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc3] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc4] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc5] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc6] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc7] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xc8] = (x86_core_cpu_instruction_handler)BSWAP_R32;
    instruction_state.connect.insTable_0f[0xc9] = (x86_core_cpu_instruction_handler)BSWAP_R32;
    instruction_state.connect.insTable_0f[0xca] = (x86_core_cpu_instruction_handler)BSWAP_R32;
    instruction_state.connect.insTable_0f[0xcb] = (x86_core_cpu_instruction_handler)BSWAP_R32;
    instruction_state.connect.insTable_0f[0xcc] = (x86_core_cpu_instruction_handler)BSWAP_R32;
    instruction_state.connect.insTable_0f[0xcd] = (x86_core_cpu_instruction_handler)BSWAP_R32;
    instruction_state.connect.insTable_0f[0xce] = (x86_core_cpu_instruction_handler)BSWAP_R32;
    instruction_state.connect.insTable_0f[0xcf] = (x86_core_cpu_instruction_handler)BSWAP_R32;
    instruction_state.connect.insTable_0f[0xd0] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd1] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd2] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd3] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd4] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd5] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd6] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd7] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd8] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xd9] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xda] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xdb] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xdc] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xdd] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xde] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xdf] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe0] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe1] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe2] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe3] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe4] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe5] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe6] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe7] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe8] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xe9] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xea] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xeb] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xec] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xed] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xee] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xef] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf0] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf1] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf2] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf3] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf4] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf5] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf6] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf7] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf8] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xf9] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xfa] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xfb] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xfc] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xfd] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xfe] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
    instruction_state.connect.insTable_0f[0xff] = (x86_core_cpu_instruction_handler)UndefinedOpcode;
}
static void _system_poll_cycle(x86_core_cpu_execution_context *context)
{
    const x86_core_cpu_special_cycle cycle = context->special_cycle;
    lib_bool acknowledged = LIB_FALSE;

    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) {
        const lib_u64 before = instruction_state.data.timing_bus_stall_ticks;

        x86_core_cpu_bus_drain(context, LIB_TRUE);
        if (context->bus_timing.invalid || !_e_timing_charge(context,
                instruction_state.data.timing_bus_stall_ticks - before)) {
            _SetExcept_CE(LIB_STATUS_LIMIT_EXCEEDED);
            ExecFinal(context);
            return;
        }
    }
    const lib_bool provided = context->bus->special_cycle != LIB_NULL &&
        context->bus->cancel_special_cycle != LIB_NULL;
    const lib_status status = provided ?
        context->bus->special_cycle(context->bus_context, cycle, &acknowledged) : LIB_STATUS_UNSUPPORTED;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
        x86_core_cpu_bus_clear_observation(context);

    if (status != LIB_STATUS_OK) {
        if (provided)
            context->bus->cancel_special_cycle(context->bus_context, cycle);
        context->special_cycle = X86_CORE_CPU_SPECIAL_NONE;
        _SetExcept_CE((lib_u32)status);
        ExecFinal(context);
    } else if (acknowledged) {
        context->special_cycle = X86_CORE_CPU_SPECIAL_NONE;
        if (cycle == X86_CORE_CPU_SPECIAL_STOP_GRANT) context->stop_granted = LIB_TRUE;
        /* L2 minimum nonburst service (273021-001 10.3.1/10.3.11).
         * HLT already owns its indication in its nominal/wake envelope. */
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
            cycle != X86_CORE_CPU_SPECIAL_HALT && !_e_timing_charge(context, 2u)) ExecFinal(context);
    }
}

static lib_bool _system_boundary(x86_core_cpu_execution_context *context)
{
    lib_bool released = LIB_FALSE;

    if (context->special_cycle != X86_CORE_CPU_SPECIAL_NONE) {
        ExecBoundary(context);
        _system_poll_cycle(context);
        return LIB_TRUE;
    }
    if (context->stop_granted) {
        if (!context->clock_running || context->stop_clock_asserted) return LIB_TRUE;
        /* 273021-001 9.6.4.2 gives approximately 10--20 CLK. Use the
         * upper endpoint as L2, not the figure's ten-clock example as L3. */
        if (!_e_timing_charge(context, 20u)) {
            ExecFinal(context);
            return LIB_TRUE;
        }
        context->stop_granted = LIB_FALSE;
        released = LIB_TRUE;
        if (context->smi_asserted) context->smi_pending = LIB_TRUE;
        if (context->nmi_asserted && !context->nmi_masked) context->nmi_pending = LIB_TRUE;
        if (cpu_state.data.flagHalt) {
            ExecBoundary(context);
            context->special_cycle = X86_CORE_CPU_SPECIAL_HALT;
            _system_poll_cycle(context);
            return LIB_TRUE;
        }
    }
    if (_system_flush_pending(context) && context->flush_asserted) return LIB_TRUE;
    if (released || (context->stop_clock_asserted && !x86_core_cpu_is_shutdown(context) &&
        !context->stop_requested && !context->debug_pause_requested &&
        context->reset_requested == X86_CORE_CPU_RESET_NONE &&
        !instruction_state.data.memory_lock_active)) {
        ExecBoundary(context);
        ExecInt(context);
        if (context->stop_requested || context->instruction_fault_delivered ||
            cpu_state.data.eip != instruction_state.data.oldcpu.data.eip ||
            cpu_state.data.cs.base != instruction_state.data.oldcpu.data.cs.base ||
            cpu_state.data.esp != instruction_state.data.oldcpu.data.esp) {
            context->instruction_fault_delivered = LIB_TRUE;
            return LIB_TRUE;
        }
        if (!context->stop_clock_asserted) return LIB_FALSE;
        context->special_cycle = X86_CORE_CPU_SPECIAL_STOP_GRANT;
        _system_poll_cycle(context);
        return LIB_TRUE;
    }
    return LIB_FALSE;
}

void x86_core_cpu_execution_reset(
    x86_core_cpu_execution_context *context)
{
    x86_core_cpu_execution_finalize(context);
    context->smi_asserted = context->smi_pending = LIB_FALSE;
    context->stop_clock_asserted = context->stop_granted = LIB_FALSE;
    context->clock_running = LIB_TRUE;
    context->flush_asserted = context->flush_pending = context->nmi_asserted = LIB_FALSE;
    context->io_restart_valid = LIB_FALSE;
    context->io_restart_eip = context->io_restart_esi = 0u;
    context->io_restart_ecx = context->io_restart_edi = 0u;
    lib_memory_set(context->tlb, 0u, sizeof(context->tlb));
    lib_memory_set(context->tlb_lru, 0u, sizeof(context->tlb_lru));
    lib_memory_set(context->cache, 0u, sizeof(context->cache));
    lib_memory_set(context->cache_lru, 0u, sizeof(context->cache_lru));
    lib_memory_set(context->cache_fill, 0u, sizeof(context->cache_fill));
    lib_memory_set(context->cache_read, 0u, sizeof(context->cache_read));
    x86_core_cpu_bus_reset(context);
    lib_memory_set((void *)(&instruction_state.data), 0u, sizeof(t_cpuins_data));
    context->debug_pause_requested = LIB_FALSE;
    context->debug_trap_pending = LIB_FALSE;
    context->debug_tf_before = LIB_FALSE;
    context->debug_rf_before = LIB_FALSE;
    context->instruction_task_switched = LIB_FALSE;
    context->interrupt_shadow = CPU_INTERRUPT_SHADOW_NONE;
    context->debug_segment_shadow_before = LIB_FALSE;
    context->nmi_in_service = LIB_FALSE;
    context->nmi_masked = LIB_FALSE;
    context->nmi_pending = LIB_FALSE;
    context->shutdown_state = CPU_SHUTDOWN_NONE;
    context->shutdown_requested = LIB_FALSE;
    context->debug_trap_cause = 0u;
    context->instruction_in_progress = LIB_FALSE;
    context->completed_observation = (x86_core_cpu_instruction_observation){0};
    context->completed_observation_valid = LIB_FALSE;
    context->instruction_fault_delivered = LIB_FALSE;
    context->instruction_wait_stall = LIB_FALSE;
    context->wait_state = CPU_WAIT_STATE_NONE;
    context->wait_poll_remaining_ticks = 0u;
}
static void _execution_refresh(
    x86_core_cpu_execution_context *context)
{
    lib_bool executed = LIB_FALSE;

    context->completed_observation = (x86_core_cpu_instruction_observation){0};
    context->completed_observation_valid = LIB_FALSE;
    context->timing_result = (x86_core_cpu_timing_result) {0};
    context->instruction_fault_delivered = LIB_FALSE;
    context->instruction_wait_stall = LIB_FALSE;
    /* The emulation-mode executor is a later complete V30 batch. */
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_V30 &&
        (cpu_state.data.eflags & X86_CPU_NEC_PSW_MD) == 0u) {
        x86_core_cpu_execution_request_stop(context);
        return;
    }
    if (_system_boundary(context)) {
        if (!context->instruction_fault_delivered) context->instruction_wait_stall = LIB_TRUE;
        return;
    }
    if (_smm_enter(context)) {
        if (instruction_state.data.except) ExecFinal(context);
        context->instruction_fault_delivered = LIB_TRUE;
        return;
    }
    if (x86_core_cpu_is_shutdown(context)) {
        const lib_bool nmi = !context->nmi_masked &&
            !context->nmi_in_service && context->nmi_pending;
        const lib_bool intr = context->shutdown_state == CPU_SHUTDOWN_SMM &&
            context->bus->interrupt_pending(context->bus_context);
        lib_u8 vector = 0x02u;

        /* Only invalid-RSM shutdown additionally admits INTR/SMI; ordinary
         * shutdown and the 286 RESET-only state retain their existing rule. */
        if (context->shutdown_state == CPU_SHUTDOWN_RESET_ONLY || (!nmi && !intr)) return;
        ExecBoundary(context);
        if (nmi) context->nmi_pending = LIB_FALSE;
        else {
            const lib_status status = context->bus->acknowledge_interrupt(context->bus_context, &vector);

            if (status != LIB_STATUS_OK) {
                _SetExcept_CE((lib_u32)status);
                ExecFinal(context);
                return;
            }
        }
        _e_intr_n(context, vector, 2u, nmi);
        if (!instruction_state.data.except) {
            context->shutdown_state = CPU_SHUTDOWN_NONE;
            context->shutdown_requested = LIB_FALSE;
            context->nmi_in_service = nmi;
            cpu_state.data.flagHalt = LIB_FALSE;
            /* Entry alone is not a retired instruction; Core starts the NMI
             * handler in the next execution round through this existing outcome. */
            context->instruction_fault_delivered = LIB_TRUE;
        } else if (_e_exception_vector(context, instruction_state.data.except) == 0xffu &&
            instruction_state.data.except != VCPUINS_EXCEPT_SHUTDOWN) {
            ExecFinal(context);
        } else {
            /* The 286 requires RESET after unsuccessful shutdown-NMI service. */
            if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286)
                context->shutdown_state = CPU_SHUTDOWN_RESET_ONLY;
            instruction_state.data.except = 0u;
        }
        return;
    }
    x86_core_cpu_execution_poll_wait(context);
    if (!cpu_state.data.flagHalt)
    {
        executed = LIB_TRUE;
        context->debug_segment_shadow_before =
            context->interrupt_shadow == CPU_INTERRUPT_SHADOW_SEGMENT;
        context->interrupt_shadow = CPU_INTERRUPT_SHADOW_NONE;
        context->instruction_in_progress = LIB_TRUE;
        ExecIns(context);
        context->instruction_in_progress = LIB_FALSE;
        if (context->instruction_fault_delivered || context->instruction_wait_stall ||
            context->stop_requested) {
            instruction_state.data.timing_gpr_writes = 0u;
            if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
                context->source_repeat_active = LIB_FALSE;
        }
        /* Interrupt entry reuses the decoder record. Select the completed
         * instruction's timing while both its operands and result are intact. */
        if (!context->instruction_fault_delivered &&
            !context->instruction_wait_stall && !context->stop_requested) {
            if (!x86_core_cpu_timing_select(context, &context->timing_result))
                context->timing_result = (x86_core_cpu_timing_result) {0};
            else {
                const lib_u8 next_components = context->completed_observation.next_lexeme_components;

                x86_core_cpu_execution_copy_observation(context, &context->completed_observation);
                context->completed_observation.next_lexeme_components = next_components;
                context->completed_observation_valid = LIB_TRUE;
            }
        }
    }
    const lib_bool retired_sx = context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        context->timing_result.ticks != 0u;
    cpu_entry_access event_scope = {0};
    if (retired_sx) {
        instruction_state.data.timing_entry_transfer_ticks = instruction_state.data.timing_transfer_ticks;
        event_scope = _e_begin_access(context);
    }
    if (context->special_cycle != X86_CORE_CPU_SPECIAL_NONE) {
        _system_poll_cycle(context);
    }
    const lib_bool dispatch_ready = context->special_cycle == X86_CORE_CPU_SPECIAL_NONE &&
        !context->stop_requested && !context->debug_pause_requested;
    if (dispatch_ready) ExecInt(context);
    if (retired_sx) _e_end_access(context, &event_scope);
    if (dispatch_ready && !executed && !cpu_state.data.flagHalt)
        context->instruction_fault_delivered = LIB_TRUE;
}

/* One actual execution scope, including asynchronous entry; host debugger
 * accesses and preview never become write-buffer timing observations. */
void x86_core_cpu_execution_refresh(x86_core_cpu_execution_context *context)
{
    if (context == LIB_NULL) return;
    context->timing_bus_active = LIB_TRUE;
    _execution_refresh(context);
    context->timing_bus_active = LIB_FALSE;
}

lib_u8 x86_core_cpu_execution_consume_instruction_fault_delivery(
    x86_core_cpu_execution_context *context)
{
    lib_u8 delivered;

    if (context == LIB_NULL) return LIB_FALSE;
    delivered = context->instruction_fault_delivered;
    context->instruction_fault_delivered = LIB_FALSE;
    return delivered;
}

void x86_core_cpu_execution_finalize(
    x86_core_cpu_execution_context *context)
{
    if (context == LIB_NULL) return;
    if (context->special_cycle != X86_CORE_CPU_SPECIAL_NONE) {
        if (context->bus->cancel_special_cycle != LIB_NULL)
            context->bus->cancel_special_cycle(context->bus_context, context->special_cycle);
        context->special_cycle = X86_CORE_CPU_SPECIAL_NONE;
    }
    if (context->smm_active) {
        context->smm_active = LIB_FALSE;
        context->bus->system_management_active(context->bus_context, LIB_FALSE);
    }
}
