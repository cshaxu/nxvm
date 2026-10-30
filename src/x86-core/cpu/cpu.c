/* Copyright 2012-2014 Neko. */

/* VCPU defines the Central Processing Unit. */
#include "lib/types/types_interface.h"


#include "x86-core/cpu/cpu_instructions.h"

#include "x86-core/cpu/cpu.h"
#include "x86-core/cpu/cpu_timing.h"

#define cpu_state (*context->cpu)
#define instruction_state (*context->instructions)

struct x86_core_cpu_prepared_entry {
    x86_core_cpu_execution_context *owner;
    t_cpu cpu;
};

#define X86_CORE_CPU_ENTRY_PLAN_FLAGS (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | \
    VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_TF | \
    VCPU_EFLAGS_IF | VCPU_EFLAGS_DF | VCPU_EFLAGS_OF)

static lib_status x86_core_cpu_prepare_patch(x86_core_cpu_execution_context *context,
    const x86_core_cpu_debug_register_patch *patch, lib_bool entry, t_cpu *out_cpu);

lib_status x86_core_cpu_prepare_entry(x86_core_cpu_execution_context *context,
    const x86_core_cpu_entry_plan_state *state,
    x86_core_cpu_prepared_entry **out_entry)
{
    x86_core_cpu_prepared_entry *entry;
    t_cpu *out_cpu;
    x86_core_cpu_debug_register_patch patch = {0};

    if (out_entry == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_entry = LIB_NULL;
    if (context == LIB_NULL || state == LIB_NULL ||
        (state->eflags & ~X86_CORE_CPU_ENTRY_PLAN_FLAGS) != 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    entry = lib_allocate_zero(1u, sizeof(*entry));
    if (entry == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    entry->owner = context;
    out_cpu = &entry->cpu;
    patch.mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
        X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
        X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
        X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS);
    patch.values[X86_CORE_CPU_DEBUG_CS] = state->cs;
    patch.values[X86_CORE_CPU_DEBUG_DS] = state->ds;
    patch.values[X86_CORE_CPU_DEBUG_ES] = state->es;
    patch.values[X86_CORE_CPU_DEBUG_SS] = state->ss;
    if (x86_core_cpu_prepare_patch(context, &patch, LIB_TRUE, out_cpu) != LIB_STATUS_OK) {
        lib_release(entry);
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    out_cpu->data.eip = state->ip;
    out_cpu->data.esp = state->sp;
    out_cpu->data.eax = state->eax;
    out_cpu->data.ebx = state->ebx;
    out_cpu->data.ecx = state->ecx;
    out_cpu->data.edx = state->edx;
    out_cpu->data.esi = state->esi;
    out_cpu->data.edi = state->edi;
    out_cpu->data.ebp = state->ebp;
    out_cpu->data.eflags = state->eflags | 0x00000002u |
        (context->cpu_profile == X86_CORE_CPU_PROFILE_V30 ? X86_CPU_NEC_PSW_MD : 0u);
    out_cpu->data.flagHalt = LIB_FALSE;
    *out_entry = entry;
    return LIB_STATUS_OK;
}

void x86_core_cpu_finish_entry(x86_core_cpu_prepared_entry *entry,
    lib_bool commit)
{
    if (entry == LIB_NULL) return;
    if (commit) {
        *entry->owner->cpu = entry->cpu;
        entry->owner->repeat_compare_flags_valid = LIB_FALSE;
        if (entry->owner->cpu_profile == X86_CORE_CPU_PROFILE_80486SX ||
            entry->owner->cpu_profile == X86_CORE_CPU_PROFILE_V30)
            entry->owner->source_repeat_active = LIB_FALSE;
        if (entry->owner->instructions != LIB_NULL) {
            entry->owner->instructions->data.timing_gpr_writes = 0u;
            entry->owner->instructions->data.timing_previous_gpr_writes = 0u;
        }
    }
    lib_release(entry);
}

typedef struct x86_core_cpu_instance {
    x86_core_cpu_execution_context execution;
    t_cpu cpu;
    t_cpuins instructions;
} x86_core_cpu_instance;

lib_status x86_core_cpu_create(const x86_core_cpu_bus_provider *bus,
    void *bus_context, x86_core_cpu_execution_context **out_context)
{
    x86_core_cpu_instance *instance;

    if (bus == LIB_NULL || out_context == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_context = LIB_NULL;
    instance = lib_allocate_zero(1u, sizeof(*instance));
    if (instance == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    x86_core_cpu_execution_context_initialize(&instance->execution,
        &instance->cpu, &instance->instructions, bus, bus_context);
    *out_context = &instance->execution;
    return LIB_STATUS_OK;
}

void x86_core_cpu_destroy(x86_core_cpu_execution_context *context)
{
    x86_core_cpu_execution_finalize(context);
    /* Execution is the first member of the sole CPU allocation. */
    lib_release((x86_core_cpu_instance *)context);
}

void x86_core_cpu_capture_state(const x86_core_cpu_execution_context *context,
    x86_core_cpu_state *out_state)
{
    out_state->cs = cpu_state.data.cs.selector;
    out_state->cs_base = cpu_state.data.cs.base;
    out_state->eip = cpu_state.data.eip;
    out_state->eflags = cpu_state.data.eflags;
    out_state->halted = cpu_state.data.flagHalt;
}

lib_u32 x86_core_cpu_linear_pc(const x86_core_cpu_execution_context *context)
{
    return cpu_state.data.cs.base + cpu_state.data.eip;
}

lib_bool x86_core_cpu_is_halted(const x86_core_cpu_execution_context *context)
{
    return cpu_state.data.flagHalt != 0u;
}

lib_bool x86_core_cpu_execution_consume_wait_stall(
    x86_core_cpu_execution_context *context)
{
    lib_bool stalled;

    if (context == LIB_NULL) return LIB_FALSE;
    stalled = context->instruction_wait_stall;
    context->instruction_wait_stall = LIB_FALSE;
    return stalled;
}

void x86_core_cpu_execution_poll_wait(
    x86_core_cpu_execution_context *context)
{
    if (context == LIB_NULL || context->wait_state == CPU_WAIT_STATE_NONE ||
        context->wait_poll_remaining_ticks != 0u ||
        context->bus == LIB_NULL || context->bus->wait_test_asserted == LIB_NULL ||
        context->bus->wait_test_asserted(context->bus_context)) return;
    context->wait_state = CPU_WAIT_STATE_NONE;
    cpu_state.data.flagHalt = LIB_FALSE;
}

void x86_core_cpu_execution_advance_wait(
    x86_core_cpu_execution_context *context, lib_u64 elapsed_ticks)
{
    if (context == LIB_NULL || context->wait_state == CPU_WAIT_STATE_NONE ||
        context->wait_poll_remaining_ticks == 0u || elapsed_ticks == 0u) return;
    if (elapsed_ticks < context->wait_poll_remaining_ticks) {
        context->wait_poll_remaining_ticks -= elapsed_ticks;
        return;
    }
    context->wait_poll_remaining_ticks = 0u;
    x86_core_cpu_execution_poll_wait(context);
    if (context->wait_state != CPU_WAIT_STATE_NONE)
        context->wait_poll_remaining_ticks = 5u;
}

lib_status x86_core_cpu_execution_wait_poll_ticks(
    const x86_core_cpu_execution_context *context, lib_u64 *out_ticks)
{
    if (context == LIB_NULL || out_ticks == LIB_NULL ||
        context->wait_state == CPU_WAIT_STATE_NONE ||
        context->bus == LIB_NULL || context->bus->wait_test_asserted == LIB_NULL) {
        return LIB_STATUS_INVALID_STATE;
    }
    *out_ticks = context->wait_poll_remaining_ticks;
    return LIB_STATUS_OK;
}

void x86_core_cpu_set_nmi_mask(x86_core_cpu_execution_context *context,
    lib_bool masked)
{
    context->nmi_masked = masked;
    if (!masked && context->nmi_asserted && !context->stop_granted && context->clock_running)
        context->nmi_pending = LIB_TRUE;
}

lib_bool x86_core_cpu_nmi_is_masked(const x86_core_cpu_execution_context *context)
{
    return context->nmi_masked;
}

lib_bool x86_core_cpu_request_nmi(x86_core_cpu_execution_context *context)
{
    if (context->nmi_masked || context->stop_granted || !context->clock_running)
        return LIB_FALSE;
    context->nmi_pending = LIB_TRUE;
    return LIB_TRUE;
}

lib_status x86_core_cpu_set_smi(x86_core_cpu_execution_context *context,
    lib_bool asserted)
{
    if (context == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX ||
        context->bus == LIB_NULL || context->bus->system_management_active == LIB_NULL)
        return LIB_STATUS_UNSUPPORTED;
    if (asserted && !context->smi_asserted && !context->stop_granted && context->clock_running)
        context->smi_pending = LIB_TRUE;
    context->smi_asserted = asserted;
    return LIB_STATUS_OK;
}

lib_status x86_core_cpu_set_stop_clock(x86_core_cpu_execution_context *context,
    lib_bool asserted)
{
    if (context == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX || context->bus == LIB_NULL ||
        context->bus->special_cycle == LIB_NULL || context->bus->cancel_special_cycle == LIB_NULL)
        return LIB_STATUS_UNSUPPORTED;
    if (!asserted && !context->clock_running) return LIB_STATUS_INVALID_STATE;
    context->stop_clock_asserted = asserted;
    return LIB_STATUS_OK;
}

lib_status x86_core_cpu_set_clock_running(x86_core_cpu_execution_context *context,
    lib_bool running)
{
    if (context == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX) return LIB_STATUS_UNSUPPORTED;
    if (running != context->clock_running && !context->stop_granted) return LIB_STATUS_INVALID_STATE;
    context->clock_running = running;
    return LIB_STATUS_OK;
}

lib_status x86_core_cpu_set_flush(x86_core_cpu_execution_context *context,
    lib_bool asserted)
{
    if (context == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX) return LIB_STATUS_UNSUPPORTED;
    if (!context->clock_running) return LIB_STATUS_INVALID_STATE;
    if (asserted && !context->flush_asserted) context->flush_pending = LIB_TRUE;
    context->flush_asserted = asserted;
    return LIB_STATUS_OK;
}

lib_status x86_core_cpu_set_nmi(x86_core_cpu_execution_context *context,
    lib_bool asserted)
{
    if (context == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX) return LIB_STATUS_UNSUPPORTED;
    if (asserted && !context->nmi_asserted && !context->stop_granted && context->clock_running &&
        !context->nmi_masked) context->nmi_pending = LIB_TRUE;
    context->nmi_asserted = asserted;
    return LIB_STATUS_OK;
}

lib_bool x86_core_cpu_is_system_waiting(const x86_core_cpu_execution_context *context)
{
    return context != LIB_NULL && (context->special_cycle != X86_CORE_CPU_SPECIAL_NONE ||
        context->stop_granted);
}

static void x86_core_cpu_debug_copy_segment(
    x86_core_cpu_debug_segment_snapshot *out_segment,
    const t_cpu_data_sreg *source)
{
    if (out_segment == LIB_NULL || source == LIB_NULL) return;
    *out_segment = (x86_core_cpu_debug_segment_snapshot) {
        .selector = source->selector, .base = source->base,
        .limit = source->limit, .dpl = source->dpl, .type = source->sys.type,
        .accessed = source->seg.accessed, .executable = source->seg.executable,
        .conform = source->seg.exec.conform, .readable = source->seg.exec.readable,
        .defsize = source->seg.exec.defsize, .big = source->seg.data.big,
        .expdown = source->seg.data.expdown, .writable = source->seg.data.writable
    };
}

static lib_i32 x86_core_cpu_debug_patch_segment(
    x86_core_cpu_execution_context *context, t_cpu *cpu,
    x86_core_cpu_debug_register register_id, lib_u32 value)
{
    switch (register_id) {
    case X86_CORE_CPU_DEBUG_ES:
        return x86_core_cpu_execution_load_segment(context, &cpu->data.es,
            (lib_u16)value);
    case X86_CORE_CPU_DEBUG_CS:
        return x86_core_cpu_execution_load_segment(context, &cpu->data.cs,
            (lib_u16)value);
    case X86_CORE_CPU_DEBUG_SS:
        return x86_core_cpu_execution_load_segment(context, &cpu->data.ss,
            (lib_u16)value);
    case X86_CORE_CPU_DEBUG_DS:
        return x86_core_cpu_execution_load_segment(context, &cpu->data.ds,
            (lib_u16)value);
    case X86_CORE_CPU_DEBUG_FS:
        return x86_core_cpu_execution_load_segment(context, &cpu->data.fs,
            (lib_u16)value);
    case X86_CORE_CPU_DEBUG_GS:
        return x86_core_cpu_execution_load_segment(context, &cpu->data.gs,
            (lib_u16)value);
    default: return 0;
    }
}

lib_status x86_core_cpu_debug_capture_snapshot(const x86_core_cpu_execution_context *context,
    x86_core_cpu_snapshot_point point,
    x86_core_cpu_debug_cpu_snapshot *out_snapshot)
{
    const t_cpu *cpu;

    if (context == LIB_NULL || out_snapshot == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (point != X86_CORE_CPU_SNAPSHOT_CURRENT &&
        point != X86_CORE_CPU_SNAPSHOT_INSTRUCTION_ENTRY)
        return LIB_STATUS_INVALID_ARGUMENT;
    cpu = point == X86_CORE_CPU_SNAPSHOT_CURRENT ? context->cpu :
        &context->instructions->data.oldcpu;
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    x86_core_cpu_debug_copy_segment(&out_snapshot->es, &cpu->data.es);
    x86_core_cpu_debug_copy_segment(&out_snapshot->cs, &cpu->data.cs);
    x86_core_cpu_debug_copy_segment(&out_snapshot->ss, &cpu->data.ss);
    x86_core_cpu_debug_copy_segment(&out_snapshot->ds, &cpu->data.ds);
    x86_core_cpu_debug_copy_segment(&out_snapshot->fs, &cpu->data.fs);
    x86_core_cpu_debug_copy_segment(&out_snapshot->gs, &cpu->data.gs);
    x86_core_cpu_debug_copy_segment(&out_snapshot->tr, &cpu->data.tr);
    x86_core_cpu_debug_copy_segment(&out_snapshot->ldtr, &cpu->data.ldtr);
    x86_core_cpu_debug_copy_segment(&out_snapshot->gdtr, &cpu->data.gdtr);
    x86_core_cpu_debug_copy_segment(&out_snapshot->idtr, &cpu->data.idtr);
    out_snapshot->cr0 = cpu->data.cr0;
    out_snapshot->cr2 = cpu->data.cr2;
    out_snapshot->cr3 = cpu->data.cr3;
    out_snapshot->eax = cpu->data.eax;
    out_snapshot->ecx = cpu->data.ecx;
    out_snapshot->edx = cpu->data.edx;
    out_snapshot->ebx = cpu->data.ebx;
    out_snapshot->esp = cpu->data.esp;
    out_snapshot->ebp = cpu->data.ebp;
    out_snapshot->esi = cpu->data.esi;
    out_snapshot->edi = cpu->data.edi;
    out_snapshot->eip = cpu->data.eip;
    out_snapshot->eflags = cpu->data.eflags;
    return LIB_STATUS_OK;
}

lib_status x86_core_cpu_debug_capture_instruction(
    const x86_core_cpu_execution_context *context,
    x86_core_cpu_debug_instruction_observation *out_observation)
{
    const t_cpu *cpu;
    const t_cpuins_data *instructions;
    lib_size index;

    if (context == LIB_NULL || out_observation == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    cpu = context->cpu;
    instructions = &context->instructions->data;
    lib_memory_set(out_observation, 0, sizeof(*out_observation));
    out_observation->cs = cpu->data.cs.selector;
    out_observation->ss = cpu->data.ss.selector;
    out_observation->ds = cpu->data.ds.selector;
    out_observation->es = cpu->data.es.selector;
    out_observation->fs = cpu->data.fs.selector;
    out_observation->gs = cpu->data.gs.selector;
    out_observation->cs_base = cpu->data.cs.base;
    out_observation->ss_base = cpu->data.ss.base;
    out_observation->eip = cpu->data.eip;
    out_observation->esp = cpu->data.esp;
    out_observation->eax = cpu->data.eax;
    out_observation->ecx = cpu->data.ecx;
    out_observation->edx = cpu->data.edx;
    out_observation->ebx = cpu->data.ebx;
    out_observation->ebp = cpu->data.ebp;
    out_observation->esi = cpu->data.esi;
    out_observation->edi = cpu->data.edi;
    out_observation->eflags = cpu->data.eflags;
    out_observation->code_default_size = cpu->data.cs.seg.exec.defsize;
    out_observation->instruction_cs = instructions->reccs;
    out_observation->instruction_eip = instructions->receip;
    out_observation->instruction_linear = instructions->linear;
    out_observation->instruction_byte_count = instructions->oplen;
    lib_memory_copy(out_observation->instruction_bytes, instructions->opcodes,
        sizeof(out_observation->instruction_bytes));
    out_observation->memory_access_count = instructions->msize <
        X86_CORE_CPU_DEBUG_MEMORY_ACCESS_CAPACITY ? instructions->msize :
        X86_CORE_CPU_DEBUG_MEMORY_ACCESS_CAPACITY;
    for (index = 0u; index < out_observation->memory_access_count; ++index) {
        out_observation->memory_accesses[index].write =
            instructions->mem[index].flagWrite;
        out_observation->memory_accesses[index].linear =
            instructions->mem[index].linear;
        out_observation->memory_accesses[index].bytes =
            instructions->mem[index].byte;
        out_observation->memory_accesses[index].data =
            instructions->mem[index].data;
    }
    out_observation->watch_hit = instructions->watch_hit;
    out_observation->watch_kind = (x86_core_cpu_debug_watch_kind)
        instructions->watch_kind;
    out_observation->watch_address = instructions->watch_address;
    return LIB_STATUS_OK;
}

lib_status x86_core_cpu_debug_read_register(const x86_core_cpu_execution_context *context,
    x86_core_cpu_debug_register register_id, lib_u32 *out_value)
{
    const t_cpu *cpu;

    if (context == LIB_NULL || out_value == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_V30 &&
        register_id >= X86_CORE_CPU_DEBUG_FS &&
        register_id < X86_CORE_CPU_DEBUG_REGISTER_COUNT)
        return LIB_STATUS_UNSUPPORTED;
    cpu = context->cpu;
    switch (register_id) {
    case X86_CORE_CPU_DEBUG_EAX: *out_value = cpu->data.eax; break;
    case X86_CORE_CPU_DEBUG_ECX: *out_value = cpu->data.ecx; break;
    case X86_CORE_CPU_DEBUG_EDX: *out_value = cpu->data.edx; break;
    case X86_CORE_CPU_DEBUG_EBX: *out_value = cpu->data.ebx; break;
    case X86_CORE_CPU_DEBUG_ESP: *out_value = cpu->data.esp; break;
    case X86_CORE_CPU_DEBUG_EBP: *out_value = cpu->data.ebp; break;
    case X86_CORE_CPU_DEBUG_ESI: *out_value = cpu->data.esi; break;
    case X86_CORE_CPU_DEBUG_EDI: *out_value = cpu->data.edi; break;
    case X86_CORE_CPU_DEBUG_EIP: *out_value = cpu->data.eip; break;
    case X86_CORE_CPU_DEBUG_EFLAGS: *out_value = cpu->data.eflags; break;
    case X86_CORE_CPU_DEBUG_ES: *out_value = cpu->data.es.selector; break;
    case X86_CORE_CPU_DEBUG_CS: *out_value = cpu->data.cs.selector; break;
    case X86_CORE_CPU_DEBUG_SS: *out_value = cpu->data.ss.selector; break;
    case X86_CORE_CPU_DEBUG_DS: *out_value = cpu->data.ds.selector; break;
    case X86_CORE_CPU_DEBUG_FS: *out_value = cpu->data.fs.selector; break;
    case X86_CORE_CPU_DEBUG_GS: *out_value = cpu->data.gs.selector; break;
    case X86_CORE_CPU_DEBUG_CR0: *out_value = cpu->data.cr0; break;
    case X86_CORE_CPU_DEBUG_CR1: *out_value = cpu->data.cr1; break;
    case X86_CORE_CPU_DEBUG_CR2: *out_value = cpu->data.cr2; break;
    case X86_CORE_CPU_DEBUG_CR3: *out_value = cpu->data.cr3; break;
    case X86_CORE_CPU_DEBUG_CR4: *out_value = cpu->data.cr4; break;
    default: return LIB_STATUS_INVALID_ARGUMENT;
    }
    return LIB_STATUS_OK;
}

static lib_status x86_core_cpu_debug_apply_patch(x86_core_cpu_execution_context *context,
    const x86_core_cpu_debug_register_patch *patch, lib_bool entry)
{
    static const x86_core_cpu_debug_register entry_order[] = {
        X86_CORE_CPU_DEBUG_CS, X86_CORE_CPU_DEBUG_DS,
        X86_CORE_CPU_DEBUG_ES, X86_CORE_CPU_DEBUG_SS};
    t_cpu *candidate_cpu = context->cpu;
    const lib_size count = entry ? sizeof(entry_order) / sizeof(entry_order[0]) :
        X86_CORE_CPU_DEBUG_REGISTER_COUNT;

    /* Segment validation uses the complete candidate paging context, even
     * though segment identifiers precede control registers in the patch enum. */
    if (patch->mask & X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR0))
        candidate_cpu->data.cr0 = patch->values[X86_CORE_CPU_DEBUG_CR0];
    if (patch->mask & X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR3))
        candidate_cpu->data.cr3 = patch->values[X86_CORE_CPU_DEBUG_CR3];
    if (patch->mask & (X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR0) |
        X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR3))) {
        x86_core_cpu_execution_invalidate_tlb(context);
    }
    /* Prepared entry reloads CS before data; Debug preserves enum order. */
    for (lib_size index = 0u; index < count; ++index) {
        const x86_core_cpu_debug_register register_id = entry ? entry_order[index] :
            (x86_core_cpu_debug_register)index;
        if ((patch->mask & X86_CORE_CPU_DEBUG_REGISTER_MASK(register_id)) == 0u)
            continue;
        switch (register_id) {
        case X86_CORE_CPU_DEBUG_EAX: candidate_cpu->data.eax = patch->values[register_id]; break;
        case X86_CORE_CPU_DEBUG_ECX: candidate_cpu->data.ecx = patch->values[register_id]; break;
        case X86_CORE_CPU_DEBUG_EDX: candidate_cpu->data.edx = patch->values[register_id]; break;
        case X86_CORE_CPU_DEBUG_EBX: candidate_cpu->data.ebx = patch->values[register_id]; break;
        case X86_CORE_CPU_DEBUG_ESP: candidate_cpu->data.esp = patch->values[register_id]; break;
        case X86_CORE_CPU_DEBUG_EBP: candidate_cpu->data.ebp = patch->values[register_id]; break;
        case X86_CORE_CPU_DEBUG_ESI: candidate_cpu->data.esi = patch->values[register_id]; break;
        case X86_CORE_CPU_DEBUG_EDI: candidate_cpu->data.edi = patch->values[register_id]; break;
        case X86_CORE_CPU_DEBUG_EIP: candidate_cpu->data.eip = patch->values[register_id]; break;
        case X86_CORE_CPU_DEBUG_EFLAGS:
            candidate_cpu->data.eflags = context->cpu_profile == X86_CORE_CPU_PROFILE_V30 ?
                (patch->values[register_id] & 0x0fd5u) | 0x02u |
                    (candidate_cpu->data.eflags & X86_CPU_NEC_PSW_MD) :
                context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX ?
                x86_core_cpu_eflags_normalize(context->cpu_profile, patch->values[register_id]) :
                patch->values[register_id];
            break;
        case X86_CORE_CPU_DEBUG_CR0: candidate_cpu->data.cr0 = patch->values[register_id]; break;
        case X86_CORE_CPU_DEBUG_CR1: candidate_cpu->data.cr1 = patch->values[register_id]; break;
        case X86_CORE_CPU_DEBUG_CR2: candidate_cpu->data.cr2 = patch->values[register_id]; break;
        case X86_CORE_CPU_DEBUG_CR3: candidate_cpu->data.cr3 = patch->values[register_id]; break;
        case X86_CORE_CPU_DEBUG_CR4: candidate_cpu->data.cr4 = patch->values[register_id]; break;
        default:
            if (x86_core_cpu_debug_patch_segment(context,
                    candidate_cpu, register_id, patch->values[register_id]))
                return LIB_STATUS_INVALID_STATE;
            break;
        }
    }
    return LIB_STATUS_OK;
}

static lib_status x86_core_cpu_prepare_patch(x86_core_cpu_execution_context *context,
    const x86_core_cpu_debug_register_patch *patch, lib_bool entry, t_cpu *out_cpu)
{
    x86_core_cpu_execution_context candidate_context;
    t_cpu candidate_cpu;
    t_cpuins candidate_instructions;
    t_cpu *original_cpu;
    t_cpuins *original_instructions;
    lib_status status;

    original_cpu = context->cpu;
    original_instructions = context->instructions;
    candidate_cpu = *original_cpu;
    candidate_instructions = *original_instructions;
    candidate_context = *context;
    candidate_context.cpu = &candidate_cpu;
    candidate_context.instructions = &candidate_instructions;
    candidate_context.preview_mode = LIB_TRUE;
    status = x86_core_cpu_debug_apply_patch(&candidate_context, patch, entry);
    if (status != LIB_STATUS_OK) return status;

    /* Preflight publishes nothing. Commit uses the live cache/TLB and bus,
     * while register and instruction candidates remain unpublished. */
    candidate_cpu = *original_cpu;
    candidate_instructions = *original_instructions;
    context->cpu = &candidate_cpu;
    context->instructions = &candidate_instructions;
    status = x86_core_cpu_debug_apply_patch(context, patch, entry);
    context->cpu = original_cpu;
    context->instructions = original_instructions;
    if (status == LIB_STATUS_OK) *out_cpu = candidate_cpu;
    if (patch->mask & (X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR0) |
        X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR3))) {
        x86_core_cpu_execution_invalidate_tlb(context);
        x86_core_cpu_execution_invalidate_prefetch(context);
    }
    return status;
}

lib_status x86_core_cpu_debug_patch_registers(x86_core_cpu_execution_context *context,
    const x86_core_cpu_debug_register_patch *patch)
{
    const lib_u32 valid_mask = (1u << X86_CORE_CPU_DEBUG_REGISTER_COUNT) - 1u;
    lib_status status;

    if (context == LIB_NULL || patch == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (patch->mask == 0u || (patch->mask & ~valid_mask) != 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_V30 &&
        (patch->mask >> X86_CORE_CPU_DEBUG_FS) != 0u)
        return LIB_STATUS_UNSUPPORTED;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
        (patch->mask & X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR4)) &&
        !x86_core_cpu_cr4_value_valid(context->cpu_profile,
            patch->values[X86_CORE_CPU_DEBUG_CR4]))
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_core_cpu_prepare_patch(context, patch, LIB_FALSE, context->cpu);
    if (status != LIB_STATUS_OK) return status;
    context->repeat_compare_flags_valid = LIB_FALSE;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX ||
        context->cpu_profile == X86_CORE_CPU_PROFILE_V30)
        context->source_repeat_active = LIB_FALSE;
    if (context->instructions != LIB_NULL) {
        instruction_state.data.timing_gpr_writes = 0u;
        instruction_state.data.timing_previous_gpr_writes = 0u;
    }
    return status;
}

static lib_u32 x86_core_cpu_reset_code_base(
    x86_core_cpu_profile profile)
{
    switch (profile) {
    case X86_CORE_CPU_PROFILE_8086:
    case X86_CORE_CPU_PROFILE_8088:
    case X86_CORE_CPU_PROFILE_80186:
    case X86_CORE_CPU_PROFILE_V30:
        return 0x000ffff0u;
    case X86_CORE_CPU_PROFILE_80286:
        return 0x00ff0000u;
    case X86_CORE_CPU_PROFILE_DEFAULT:
    case X86_CORE_CPU_PROFILE_80386:
    case X86_CORE_CPU_PROFILE_80486SX:
        return 0xffff0000u;
    }
    return 0xffff0000u;
}

void x86_core_cpu_execution_context_initialize(
    x86_core_cpu_execution_context *context, t_cpu *cpu,
    t_cpuins *instructions, const x86_core_cpu_bus_provider *bus,
    void *bus_context)
{
    if (context == LIB_NULL) return;
    context->cpu = cpu;
    context->instructions = instructions;
    context->instruction_timing = (x86_core_cpu_instruction_timing) { .base_ticks = 1u };
    context->timing_result = (x86_core_cpu_timing_result) {0};
    context->source_repeat_byte_count = 0u;
    context->source_repeat_data_misses = 0u;
    x86_core_cpu_bus_reset(context);
    context->timing_bus_active = LIB_FALSE;
    context->bus = bus;
    context->bus_context = bus_context;
    context->diagnostic_provider = LIB_NULL;
    context->diagnostic_context = LIB_NULL;
    context->external_cycle_provider = LIB_NULL;
    context->external_cycle_context = LIB_NULL;
    context->stop_requested = LIB_FALSE;
    context->debug_pause_requested = LIB_FALSE;
    context->reset_requested = X86_CORE_CPU_RESET_NONE;
    context->smbase = X86_CPU_DEFAULT_SMBASE;
    context->smm_active = context->smi_asserted = context->smi_pending = LIB_FALSE;
    context->special_cycle = X86_CORE_CPU_SPECIAL_NONE;
    context->stop_clock_asserted = context->stop_granted = LIB_FALSE;
    context->clock_running = LIB_TRUE;
    context->flush_asserted = context->flush_pending = context->nmi_asserted = LIB_FALSE;
    context->io_restart_valid = LIB_FALSE;
    context->io_restart_eip = context->io_restart_esi = 0u;
    context->io_restart_ecx = context->io_restart_edi = 0u;
    context->debug_trap_pending = LIB_FALSE;
    context->debug_tf_before = LIB_FALSE;
    context->debug_rf_before = LIB_FALSE;
    context->repeat_compare_flags_valid = LIB_FALSE;
    context->repeat_compare_flags = 0u;
    context->instruction_task_switched = LIB_FALSE;
    context->interrupt_shadow = CPU_INTERRUPT_SHADOW_NONE;
    context->debug_segment_shadow_before = LIB_FALSE;
    context->nmi_in_service = LIB_FALSE;
    context->nmi_masked = LIB_FALSE;
    context->nmi_pending = LIB_FALSE;
    context->debug_trap_cause = 0u;
    context->preview_mode = LIB_FALSE;
    context->preview_memory_source = LIB_NULL;
    context->nec_md_write_enabled = LIB_FALSE;
    lib_memory_set(context->tlb, 0u, sizeof(context->tlb));
    lib_memory_set(context->tlb_lru, 0u, sizeof(context->tlb_lru));
    lib_memory_set(context->cache, 0u, sizeof(context->cache));
    lib_memory_set(context->cache_lru, 0u, sizeof(context->cache_lru));
    lib_memory_set(context->cache_fill, 0u, sizeof(context->cache_fill));
    lib_memory_set(context->cache_read, 0u, sizeof(context->cache_read));
    context->memory_access_provenance = X86_CORE_CPU_MEMORY_ACCESS_DATA;
    context->prefetch_count = 0u;
    context->prefetch_capacity = 15u;
    context->prefetch_valid = LIB_FALSE;
    context->prefetch_expected_valid = LIB_FALSE;
    context->prefetch_reservation_valid = LIB_FALSE;
    context->prefetch_reservation_linear = 0u;
    context->prefetch_reservation_count = 0u;
    context->cpu_profile = X86_CORE_CPU_PROFILE_80386;
    context->fpu_profile = X86_FPU_PROFILE_NONE;
    context->cpu_80386_cr_mov_ignores_mod = LIB_FALSE;
    context->protected_data_reload_policy = X86_CORE_CPU_PROTECTED_DATA_RELOAD_STRICT;
    context->fpu = LIB_NULL;
    context->instruction_wait_stall = LIB_FALSE;
    context->wait_state = CPU_WAIT_STATE_NONE;
    context->wait_poll_remaining_ticks = 0u;
}

void x86_core_cpu_execution_context_bind_profiles(
    x86_core_cpu_execution_context *context,
    x86_core_cpu_profile cpu_profile,
    x86_fpu_profile fpu_profile,
    lib_u8 cpu_80386_cr_mov_ignores_mod,
    x86_core_cpu_protected_data_reload_policy protected_data_reload_policy,
    const x86_core_cpu_instruction_timing *timing)
{
    if (context == LIB_NULL) return;
    context->instruction_timing = *timing;
    if (context->cpu_profile != cpu_profile) x86_core_cpu_bus_reset(context);
    context->cpu_profile = cpu_profile;
    context->prefetch_capacity = cpu_profile_prefetch_capacity(cpu_profile);
    context->fpu_profile = fpu_profile;
    context->cpu_80386_cr_mov_ignores_mod = cpu_80386_cr_mov_ignores_mod;
    context->protected_data_reload_policy = protected_data_reload_policy;
}

void x86_core_cpu_execution_context_bind_fpu(
    x86_core_cpu_execution_context *context, x86_fpu *fpu)
{
    if (context != LIB_NULL) context->fpu = fpu;
}

void x86_core_cpu_execution_context_bind_external_cycle_provider(
    x86_core_cpu_execution_context *context,
    x86_core_cpu_external_cycle_provider provider, void *provider_context)
{
    if (context == LIB_NULL) return;
    context->external_cycle_provider = provider;
    context->external_cycle_context = provider_context;
}

const char *x86_core_cpu_profile_name(x86_core_cpu_profile profile)
{
    switch (profile) {
    case X86_CORE_CPU_PROFILE_8086: return "8086";
    case X86_CORE_CPU_PROFILE_8088: return "8088";
    case X86_CORE_CPU_PROFILE_80186: return "80186";
    case X86_CORE_CPU_PROFILE_80286: return "80286";
    case X86_CORE_CPU_PROFILE_80386: return "80386";
    case X86_CORE_CPU_PROFILE_80486SX: return "80486SX";
    case X86_CORE_CPU_PROFILE_V30: return "V30";
    case X86_CORE_CPU_PROFILE_DEFAULT: return "default";
    }
    return "invalid";
}

void x86_core_cpu_execution_context_bind_diagnostic_provider(
    x86_core_cpu_execution_context *context,
    const x86_core_cpu_execution_diagnostic_provider *provider,
    void *provider_context)
{
    if (context == LIB_NULL) return;
    context->diagnostic_provider = provider;
    context->diagnostic_context = provider_context;
}

void x86_core_cpu_state_initialize(
    x86_core_cpu_execution_context *context) {
    if (context == LIB_NULL || context->cpu == LIB_NULL ||
        context->instructions == LIB_NULL) return;
    if (context != LIB_NULL) {
        context->stop_requested = LIB_FALSE;
        context->debug_pause_requested = LIB_FALSE;
        context->reset_requested = X86_CORE_CPU_RESET_NONE;
        context->smbase = X86_CPU_DEFAULT_SMBASE;
        context->shutdown_requested = LIB_FALSE;
        context->debug_trap_pending = LIB_FALSE;
        context->shutdown_state = CPU_SHUTDOWN_NONE;
        context->debug_tf_before = LIB_FALSE;
        context->debug_rf_before = LIB_FALSE;
        context->instruction_task_switched = LIB_FALSE;
        context->interrupt_shadow = CPU_INTERRUPT_SHADOW_NONE;
        context->debug_segment_shadow_before = LIB_FALSE;
        context->nmi_in_service = LIB_FALSE;
        context->nmi_masked = LIB_FALSE;
        context->nmi_pending = LIB_FALSE;
        context->debug_trap_cause = 0u;
        context->prefetch_count = 0u;
        context->prefetch_capacity = cpu_profile_prefetch_capacity(context->cpu_profile);
        context->prefetch_valid = LIB_FALSE;
        context->prefetch_expected_valid = LIB_FALSE;
        context->prefetch_reservation_valid = LIB_FALSE;
        context->prefetch_reservation_linear = 0u;
        context->prefetch_reservation_count = 0u;
    }
    x86_core_cpu_execution_initialize(context);
}
static void x86_core_cpu_state_apply_reset(
    x86_core_cpu_execution_context *context, lib_bool preserve_smbase)
{
    if (context == LIB_NULL || context->cpu == LIB_NULL ||
        context->instructions == LIB_NULL) return;
    const lib_bool early = x86_core_cpu_profile_has_8086_semantics(
        context->cpu_profile) || context->cpu_profile == X86_CORE_CPU_PROFILE_80186 ||
        context->cpu_profile == X86_CORE_CPU_PROFILE_V30;
    const lib_u32 smbase = context->smbase;
    context->source_repeat_active = LIB_FALSE;
    context->repeat_compare_flags_valid = LIB_FALSE;
    context->repeat_compare_flags = 0u;
    context->source_repeat_cs = 0u;
    context->source_repeat_eip = 0u;
    context->source_repeat_opcode = 0u;
    context->source_repeat_prefix = 0u;
    context->source_repeat_operand_size = LIB_FALSE;
    context->source_repeat_address_size = LIB_FALSE;
    lib_memory_set((void *)context->cpu, 0u, sizeof(t_cpu));
    if (context != LIB_NULL) {
        context->stop_requested = LIB_FALSE;
        context->debug_pause_requested = LIB_FALSE;
        context->reset_requested = X86_CORE_CPU_RESET_NONE;
        context->shutdown_requested = LIB_FALSE;
        context->prefetch_count = 0u;
        context->prefetch_capacity = cpu_profile_prefetch_capacity(context->cpu_profile);
        context->prefetch_valid = LIB_FALSE;
        context->prefetch_expected_valid = LIB_FALSE;
        context->prefetch_reservation_valid = LIB_FALSE;
        context->prefetch_reservation_linear = 0u;
        context->prefetch_reservation_count = 0u;
    }

    cpu_state.data.eip = early ? 0u : 0x0000fff0u;
    cpu_state.data.eflags = 0x00000002;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_V30)
        cpu_state.data.eflags |= X86_CPU_NEC_PSW_MD;
    /* Intel 80386 PRM 10.1 defines DH=3 after RESET# for the 386DX.
     * The selected zero revision keeps the documented device identifier
     * without inventing a board-specific stepping value. */
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386)
        cpu_state.data.edx = 0x00000300u;
    else if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) {
        cpu_state.data.edx = VCPU_486SX_SIGNATURE;
        /* Intel486 PRM 10.1, Figure 10-2 and Table 10-1. */
        cpu_state.data.cr0 = VCPU_CR0_CD | VCPU_CR0_NW | VCPU_CR0_ET;
        cpu_state.data.dr6 = 0xffff0ff0u;
    }

    cpu_state.data.cs.base = x86_core_cpu_reset_code_base(context->cpu_profile);
    cpu_state.data.cs.dpl = 0u;
    cpu_state.data.cs.limit = 0xffffu;
    cpu_state.data.cs.seg.accessed = LIB_TRUE;
    cpu_state.data.cs.seg.executable = LIB_TRUE;
    cpu_state.data.cs.seg.exec.conform = LIB_FALSE;
    cpu_state.data.cs.seg.exec.defsize = LIB_FALSE;
    cpu_state.data.cs.seg.exec.readable = LIB_TRUE;
    cpu_state.data.cs.selector = early ? 0xffffu : 0xf000u;
    cpu_state.data.cs.sregtype = SREG_CODE;
    cpu_state.data.cs.flagValid = LIB_TRUE;

    cpu_state.data.ss.base = 0u;
    cpu_state.data.ss.dpl = 0u;
    cpu_state.data.ss.limit = 0xffffu;
    cpu_state.data.ss.seg.accessed = LIB_TRUE;
    cpu_state.data.ss.seg.executable = LIB_FALSE;
    cpu_state.data.ss.seg.data.big = LIB_FALSE;
    cpu_state.data.ss.seg.data.expdown = LIB_FALSE;
    cpu_state.data.ss.seg.data.writable = LIB_TRUE;
    cpu_state.data.ss.selector = 0u;
    cpu_state.data.ss.sregtype = SREG_STACK;
    cpu_state.data.ss.flagValid = LIB_TRUE;

    cpu_state.data.ds.base = 0u;
    cpu_state.data.ds.dpl = 0u;
    cpu_state.data.ds.limit = 0xffffu;
    cpu_state.data.ds.seg.accessed = LIB_TRUE;
    cpu_state.data.ss.seg.executable = LIB_FALSE;
    cpu_state.data.ds.seg.data.big = LIB_FALSE;
    cpu_state.data.ds.seg.data.expdown = LIB_FALSE;
    cpu_state.data.ds.seg.data.writable = LIB_TRUE;
    cpu_state.data.ds.selector = 0u;
    cpu_state.data.ds.sregtype = SREG_DATA;
    cpu_state.data.ds.flagValid = LIB_TRUE;
    cpu_state.data.gs = cpu_state.data.fs = cpu_state.data.es = cpu_state.data.ds;

    cpu_state.data.ldtr.base = 0u;
    cpu_state.data.ldtr.dpl = 0u;
    cpu_state.data.ldtr.limit = 0xffffu;
    cpu_state.data.ldtr.selector = 0u;
    cpu_state.data.ldtr.sregtype = SREG_LDTR;
    cpu_state.data.ldtr.sys.type = VCPU_DESC_SYS_TYPE_LDT;
    cpu_state.data.ldtr.flagValid = LIB_TRUE;

    cpu_state.data.tr.base = 0u;
    cpu_state.data.tr.dpl = 0u;
    cpu_state.data.tr.limit = 0xffffu;
    cpu_state.data.tr.selector = 0u;
    cpu_state.data.tr.sregtype = SREG_TR;
    cpu_state.data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_16_AVL;
    cpu_state.data.tr.flagValid = LIB_TRUE;

    cpu_state.data.idtr.base = 0u;
    cpu_state.data.idtr.limit = 0x03ff;
    cpu_state.data.idtr.sregtype = SREG_IDTR;
    cpu_state.data.idtr.flagValid = LIB_TRUE;

    cpu_state.data.gdtr.base = 0u;
    cpu_state.data.gdtr.limit = 0xffffu;
    cpu_state.data.gdtr.sregtype = SREG_GDTR;
    cpu_state.data.gdtr.flagValid = LIB_TRUE;

    x86_core_cpu_execution_reset(context);
    context->smbase = preserve_smbase ? smbase : X86_CPU_DEFAULT_SMBASE;
}

void x86_core_cpu_state_reset(x86_core_cpu_execution_context *context)
{
    x86_core_cpu_state_apply_reset(context, LIB_FALSE);
}

lib_status x86_core_cpu_state_soft_reset(x86_core_cpu_execution_context *context)
{
    if (context == LIB_NULL || context->cpu == LIB_NULL || context->instructions == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX)
        return LIB_STATUS_UNSUPPORTED;
    x86_core_cpu_state_apply_reset(context, LIB_TRUE);
    return LIB_STATUS_OK;
}

void x86_core_cpu_execution_reserve_prefetch(
    x86_core_cpu_execution_context *context)
{
    lib_u32 offset;

    if (context == LIB_NULL || context->cpu == LIB_NULL ||
        (context->cpu->data.cr0 & VCPU_CR0_PG) ||
        context->prefetch_reservation_valid || !context->prefetch_valid ||
        context->prefetch_expected_linear < context->prefetch_linear ||
        cpu_state.data.eip > cpu_state.data.cs.limit) return;
    if (context->cpu_profile != X86_CORE_CPU_PROFILE_8088) {
        context->prefetch_reservation_linear = context->prefetch_expected_linear;
        context->prefetch_reservation_count = context->prefetch_count;
        context->prefetch_reservation_valid = LIB_TRUE;
        return;
    }
    offset = context->prefetch_expected_linear - context->prefetch_linear;
    if (offset >= context->prefetch_count) return;
    if (offset != 0u) {
        context->prefetch_count = (lib_u8)(context->prefetch_count - offset);
        lib_memory_move(context->prefetch_bytes, context->prefetch_bytes + offset,
            context->prefetch_count);
        context->prefetch_linear = context->prefetch_expected_linear;
    }
    if (context->prefetch_count >= context->prefetch_capacity) return;
    context->prefetch_reservation_linear = context->prefetch_linear +
        context->prefetch_count;
    context->prefetch_reservation_count = 1u;
    context->prefetch_reservation_valid = LIB_TRUE;
}

void x86_core_cpu_execution_advance_prefetch_reservation(
    x86_core_cpu_execution_context *context)
{
    lib_u8 byte;

    if (context == LIB_NULL || !context->prefetch_reservation_valid) return;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_8088) {
        context->memory_access_provenance =
            X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH;
        if (!x86_core_cpu_execution_read_linear(context,
                context->prefetch_reservation_linear, (lib_uptr)&byte, 1u) &&
            context->prefetch_count < context->prefetch_capacity) {
            context->prefetch_bytes[context->prefetch_count++] = byte;
        }
        context->memory_access_provenance = X86_CORE_CPU_MEMORY_ACCESS_DATA;
    }
    context->prefetch_reservation_valid = LIB_FALSE;
    context->prefetch_reservation_linear = 0u;
    context->prefetch_reservation_count = 0u;
}
void x86_core_cpu_execution_invalidate_prefetch(
    x86_core_cpu_execution_context *context)
{
    if (context == LIB_NULL) return;
    if (!context->timing_bus_active && !context->preview_mode &&
        context->instructions != LIB_NULL)
        x86_core_cpu_bus_complete_memory_lock(context, LIB_FALSE);
    context->prefetch_count = 0u;
    context->prefetch_valid = LIB_FALSE;
    context->prefetch_expected_valid = LIB_FALSE;
    context->prefetch_reservation_valid = LIB_FALSE;
    context->prefetch_reservation_linear = 0u;
    context->prefetch_reservation_count = 0u;
}

void x86_core_cpu_execution_request_stop(
    x86_core_cpu_execution_context *context)
{
    if (context != LIB_NULL) context->stop_requested = LIB_TRUE;
}
lib_u8 x86_core_cpu_execution_consume_stop_request(
    x86_core_cpu_execution_context *context)
{
    lib_u8 requested = context != LIB_NULL && context->stop_requested;
    if (context != LIB_NULL) context->stop_requested = LIB_FALSE;
    return requested;
}
void x86_core_cpu_execution_request_debug_pause(
    x86_core_cpu_execution_context *context)
{
    if (context != LIB_NULL) context->debug_pause_requested = LIB_TRUE;
}

lib_u8 x86_core_cpu_execution_consume_debug_pause_request(
    x86_core_cpu_execution_context *context)
{
    lib_u8 requested = context != LIB_NULL && context->debug_pause_requested;

    if (context != LIB_NULL) context->debug_pause_requested = LIB_FALSE;
    return requested;
}
void x86_core_cpu_execution_request_reset(
    x86_core_cpu_execution_context *context)
{
    if (context != LIB_NULL) context->reset_requested = X86_CORE_CPU_RESET_HARD;
}

lib_status x86_core_cpu_execution_request_soft_reset(
    x86_core_cpu_execution_context *context)
{
    if (context == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX)
        return LIB_STATUS_UNSUPPORTED;
    if (context->reset_requested == X86_CORE_CPU_RESET_NONE)
        context->reset_requested = X86_CORE_CPU_RESET_SOFT;
    return LIB_STATUS_OK;
}

x86_core_cpu_reset_kind x86_core_cpu_execution_consume_reset_request(
    x86_core_cpu_execution_context *context)
{
    const x86_core_cpu_reset_kind requested = context != LIB_NULL ?
        context->reset_requested : X86_CORE_CPU_RESET_NONE;

    if (context != LIB_NULL) context->reset_requested = X86_CORE_CPU_RESET_NONE;
    return requested;
}
void x86_core_cpu_execution_request_shutdown(
    x86_core_cpu_execution_context *context)
{
    if (context != LIB_NULL) {
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
            !context->shutdown_requested && context->special_cycle == X86_CORE_CPU_SPECIAL_NONE)
            context->special_cycle = X86_CORE_CPU_SPECIAL_SHUTDOWN;
        context->shutdown_requested = LIB_TRUE;
        if (context->shutdown_state == CPU_SHUTDOWN_NONE)
            context->shutdown_state = CPU_SHUTDOWN_WAITING;
    }
}
lib_bool x86_core_cpu_is_shutdown(
    const x86_core_cpu_execution_context *context)
{
    return context != LIB_NULL && context->shutdown_state != CPU_SHUTDOWN_NONE;
}
lib_u8 x86_core_cpu_execution_consume_shutdown_request(
    x86_core_cpu_execution_context *context)
{
    lib_u8 requested = context != LIB_NULL && context->shutdown_requested;
    if (context != LIB_NULL) context->shutdown_requested = LIB_FALSE;
    return requested;
}

lib_i32 x86_core_cpu_read_linear(x86_core_cpu_execution_context *context, lib_u32 linear, void *out_data, lib_u8 size)
{
    return x86_core_cpu_execution_read_linear(context, linear,
        (lib_uptr)out_data, size);
}

lib_i32 x86_core_cpu_write_linear(x86_core_cpu_execution_context *context,
    lib_u32 linear, const void *in_data, lib_u8 size)
{
    return x86_core_cpu_execution_write_linear(context, linear,
        (lib_uptr)in_data, size);
}

lib_i32 x86_core_cpu_get_code_default_size(const x86_core_cpu_execution_context *context)
{
    return cpu_state.data.cs.seg.exec.defsize;
}

lib_u32 x86_core_cpu_get_code_base(const x86_core_cpu_execution_context *context)
{
    return cpu_state.data.cs.base;
}

void x86_core_cpu_set_watchpoint(x86_core_cpu_execution_context *context,
    x86_core_cpu_watchpoint kind, lib_u32 linear)
{
    switch (kind) {
    case X86_CORE_CPU_WATCH_READ:
        instruction_state.data.wrLinear = linear;
        instruction_state.data.flagWR = LIB_TRUE;
        break;
    case X86_CORE_CPU_WATCH_WRITE:
        instruction_state.data.wwLinear = linear;
        instruction_state.data.flagWW = LIB_TRUE;
        break;
    case X86_CORE_CPU_WATCH_EXECUTE:
        instruction_state.data.weLinear = linear;
        instruction_state.data.flagWE = LIB_TRUE;
        break;
    }
}

void x86_core_cpu_clear_watchpoint(x86_core_cpu_execution_context *context,
    x86_core_cpu_watchpoint kind)
{
    switch (kind) {
    case X86_CORE_CPU_WATCH_READ:
        instruction_state.data.flagWR = LIB_FALSE;
        break;
    case X86_CORE_CPU_WATCH_WRITE:
        instruction_state.data.flagWW = LIB_FALSE;
        break;
    case X86_CORE_CPU_WATCH_EXECUTE:
        instruction_state.data.flagWE = LIB_FALSE;
        break;
    }
}

void x86_core_cpu_get_watchpoint(const x86_core_cpu_execution_context *context,
    x86_core_cpu_watchpoint kind, lib_u8 *out_enabled,
    lib_u32 *out_linear)
{
    if (out_enabled == LIB_NULL || out_linear == LIB_NULL) return;
    *out_enabled = LIB_FALSE;
    *out_linear = 0u;
    switch (kind) {
    case X86_CORE_CPU_WATCH_READ:
        *out_enabled = instruction_state.data.flagWR;
        *out_linear = instruction_state.data.wrLinear;
        break;
    case X86_CORE_CPU_WATCH_WRITE:
        *out_enabled = instruction_state.data.flagWW;
        *out_linear = instruction_state.data.wwLinear;
        break;
    case X86_CORE_CPU_WATCH_EXECUTE:
        *out_enabled = instruction_state.data.flagWE;
        *out_linear = instruction_state.data.weLinear;
        break;
    }
}
