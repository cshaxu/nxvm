/* Copyright 2012-2014 Neko. */

/* VCPU defines the Central Processing Unit. */
#include "lib/types/types_interface.h"


#include "x86/chips/cpu/cpu_instructions.h"

#include "x86/chips/cpu/cpu.h"

#define cpu_state (*context->cpu)
#define instruction_state (*context->instructions)

struct core_machine_cpu_prepared_entry {
    core_machine_cpu_execution_context *owner;
    t_cpu cpu;
};

#define CORE_MACHINE_ENTRY_PLAN_FLAGS (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | \
    VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_TF | \
    VCPU_EFLAGS_IF | VCPU_EFLAGS_DF | VCPU_EFLAGS_OF)

lib_status core_machine_cpu_prepare_entry(core_machine_cpu_execution_context *context,
    const core_machine_entry_plan_state *state,
    core_machine_cpu_prepared_entry **out_entry)
{
    core_machine_cpu_execution_context candidate;
    core_machine_cpu_prepared_entry *entry;
    t_cpu *out_cpu;

    if (out_entry == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_entry = LIB_NULL;
    if (context == LIB_NULL || state == LIB_NULL ||
        (state->eflags & ~CORE_MACHINE_ENTRY_PLAN_FLAGS) != 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    entry = lib_allocate_zero(1u, sizeof(*entry));
    if (entry == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    entry->owner = context;
    out_cpu = &entry->cpu;
    *out_cpu = *context->cpu;
    candidate = *context;
    candidate.cpu = out_cpu;
    if (core_machine_cpu_execution_load_segment(&candidate, &out_cpu->data.cs,
            state->cs) || core_machine_cpu_execution_load_segment(&candidate,
            &out_cpu->data.ds, state->ds) ||
        core_machine_cpu_execution_load_segment(&candidate, &out_cpu->data.es,
            state->es) || core_machine_cpu_execution_load_segment(&candidate,
            &out_cpu->data.ss, state->ss)) {
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
    out_cpu->data.eflags = state->eflags | 0x00000002u;
    out_cpu->data.flagHalt = LIB_FALSE;
    *out_entry = entry;
    return LIB_STATUS_OK;
}

void core_machine_cpu_finish_entry(core_machine_cpu_prepared_entry *entry,
    lib_bool commit)
{
    if (entry == LIB_NULL) return;
    if (commit) *entry->owner->cpu = entry->cpu;
    lib_release(entry);
}

typedef struct core_machine_cpu_instance {
    core_machine_cpu_execution_context execution;
    t_cpu cpu;
    t_cpuins instructions;
} core_machine_cpu_instance;

lib_status core_machine_cpu_create(const core_machine_cpu_bus_provider *bus,
    void *bus_context, core_machine_cpu_execution_context **out_context)
{
    core_machine_cpu_instance *instance;

    if (bus == LIB_NULL || out_context == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_context = LIB_NULL;
    instance = lib_allocate_zero(1u, sizeof(*instance));
    if (instance == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    core_machine_cpu_execution_context_initialize(&instance->execution,
        &instance->cpu, &instance->instructions, bus, bus_context);
    *out_context = &instance->execution;
    return LIB_STATUS_OK;
}

void core_machine_cpu_destroy(core_machine_cpu_execution_context *context)
{
    /* Execution is the first member of the sole CPU allocation. */
    lib_release((core_machine_cpu_instance *)context);
}

void core_machine_cpu_capture_state(const core_machine_cpu_execution_context *context,
    core_machine_cpu_state *out_state)
{
    out_state->cs = cpu_state.data.cs.selector;
    out_state->cs_base = cpu_state.data.cs.base;
    out_state->eip = cpu_state.data.eip;
    out_state->eflags = cpu_state.data.eflags;
    out_state->halted = cpu_state.data.flagHalt;
}

lib_u32 core_machine_cpu_linear_pc(const core_machine_cpu_execution_context *context)
{
    return cpu_state.data.cs.base + cpu_state.data.eip;
}

lib_bool core_machine_cpu_is_halted(const core_machine_cpu_execution_context *context)
{
    return cpu_state.data.flagHalt != 0u;
}

void core_machine_cpu_set_nmi_mask(core_machine_cpu_execution_context *context,
    lib_bool masked)
{
    cpu_state.data.flagMaskNMI = masked ? LIB_TRUE : LIB_FALSE;
}

lib_bool core_machine_cpu_nmi_is_masked(const core_machine_cpu_execution_context *context)
{
    return cpu_state.data.flagMaskNMI != 0u;
}

lib_bool core_machine_cpu_request_nmi(core_machine_cpu_execution_context *context)
{
    if (cpu_state.data.flagMaskNMI) return LIB_FALSE;
    cpu_state.data.flagNMI = LIB_TRUE;
    return LIB_TRUE;
}

static void core_machine_debug_copy_segment(
    core_machine_debug_segment_snapshot *out_segment,
    const t_cpu_data_sreg *source)
{
    if (out_segment == LIB_NULL || source == LIB_NULL) return;
    *out_segment = (core_machine_debug_segment_snapshot) {
        .selector = source->selector, .base = source->base,
        .limit = source->limit, .dpl = source->dpl, .type = source->sys.type,
        .accessed = source->seg.accessed, .executable = source->seg.executable,
        .conform = source->seg.exec.conform, .readable = source->seg.exec.readable,
        .defsize = source->seg.exec.defsize, .big = source->seg.data.big,
        .expdown = source->seg.data.expdown, .writable = source->seg.data.writable
    };
}

static lib_i32 core_machine_debug_patch_segment(
    core_machine_cpu_execution_context *context, t_cpu *cpu,
    core_machine_debug_register register_id, lib_u32 value)
{
    switch (register_id) {
    case CORE_MACHINE_DEBUG_ES:
        return core_machine_cpu_execution_load_segment(context, &cpu->data.es,
            (lib_u16)value);
    case CORE_MACHINE_DEBUG_CS:
        return core_machine_cpu_execution_load_segment(context, &cpu->data.cs,
            (lib_u16)value);
    case CORE_MACHINE_DEBUG_SS:
        return core_machine_cpu_execution_load_segment(context, &cpu->data.ss,
            (lib_u16)value);
    case CORE_MACHINE_DEBUG_DS:
        return core_machine_cpu_execution_load_segment(context, &cpu->data.ds,
            (lib_u16)value);
    case CORE_MACHINE_DEBUG_FS:
        return core_machine_cpu_execution_load_segment(context, &cpu->data.fs,
            (lib_u16)value);
    case CORE_MACHINE_DEBUG_GS:
        return core_machine_cpu_execution_load_segment(context, &cpu->data.gs,
            (lib_u16)value);
    default: return 0;
    }
}

lib_status core_machine_cpu_debug_capture_snapshot(const core_machine_cpu_execution_context *context,
    core_machine_cpu_snapshot_point point,
    core_machine_debug_cpu_snapshot *out_snapshot)
{
    const t_cpu *cpu;

    if (context == LIB_NULL || out_snapshot == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (point != CORE_MACHINE_CPU_SNAPSHOT_CURRENT &&
        point != CORE_MACHINE_CPU_SNAPSHOT_INSTRUCTION_ENTRY)
        return LIB_STATUS_INVALID_ARGUMENT;
    cpu = point == CORE_MACHINE_CPU_SNAPSHOT_CURRENT ? context->cpu :
        &context->instructions->data.oldcpu;
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    core_machine_debug_copy_segment(&out_snapshot->es, &cpu->data.es);
    core_machine_debug_copy_segment(&out_snapshot->cs, &cpu->data.cs);
    core_machine_debug_copy_segment(&out_snapshot->ss, &cpu->data.ss);
    core_machine_debug_copy_segment(&out_snapshot->ds, &cpu->data.ds);
    core_machine_debug_copy_segment(&out_snapshot->fs, &cpu->data.fs);
    core_machine_debug_copy_segment(&out_snapshot->gs, &cpu->data.gs);
    core_machine_debug_copy_segment(&out_snapshot->tr, &cpu->data.tr);
    core_machine_debug_copy_segment(&out_snapshot->ldtr, &cpu->data.ldtr);
    core_machine_debug_copy_segment(&out_snapshot->gdtr, &cpu->data.gdtr);
    core_machine_debug_copy_segment(&out_snapshot->idtr, &cpu->data.idtr);
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

lib_status core_machine_cpu_debug_capture_instruction(
    const core_machine_cpu_execution_context *context,
    core_machine_debug_instruction_observation *out_observation)
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
        CORE_MACHINE_DEBUG_MEMORY_ACCESS_CAPACITY ? instructions->msize :
        CORE_MACHINE_DEBUG_MEMORY_ACCESS_CAPACITY;
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
    out_observation->watch_kind = (core_machine_debug_watch_kind)
        instructions->watch_kind;
    out_observation->watch_address = instructions->watch_address;
    return LIB_STATUS_OK;
}

lib_status core_machine_cpu_debug_read_register(const core_machine_cpu_execution_context *context,
    core_machine_debug_register register_id, lib_u32 *out_value)
{
    const t_cpu *cpu;

    if (context == LIB_NULL || out_value == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    cpu = context->cpu;
    switch (register_id) {
    case CORE_MACHINE_DEBUG_EAX: *out_value = cpu->data.eax; break;
    case CORE_MACHINE_DEBUG_ECX: *out_value = cpu->data.ecx; break;
    case CORE_MACHINE_DEBUG_EDX: *out_value = cpu->data.edx; break;
    case CORE_MACHINE_DEBUG_EBX: *out_value = cpu->data.ebx; break;
    case CORE_MACHINE_DEBUG_ESP: *out_value = cpu->data.esp; break;
    case CORE_MACHINE_DEBUG_EBP: *out_value = cpu->data.ebp; break;
    case CORE_MACHINE_DEBUG_ESI: *out_value = cpu->data.esi; break;
    case CORE_MACHINE_DEBUG_EDI: *out_value = cpu->data.edi; break;
    case CORE_MACHINE_DEBUG_EIP: *out_value = cpu->data.eip; break;
    case CORE_MACHINE_DEBUG_EFLAGS: *out_value = cpu->data.eflags; break;
    case CORE_MACHINE_DEBUG_ES: *out_value = cpu->data.es.selector; break;
    case CORE_MACHINE_DEBUG_CS: *out_value = cpu->data.cs.selector; break;
    case CORE_MACHINE_DEBUG_SS: *out_value = cpu->data.ss.selector; break;
    case CORE_MACHINE_DEBUG_DS: *out_value = cpu->data.ds.selector; break;
    case CORE_MACHINE_DEBUG_FS: *out_value = cpu->data.fs.selector; break;
    case CORE_MACHINE_DEBUG_GS: *out_value = cpu->data.gs.selector; break;
    case CORE_MACHINE_DEBUG_CR0: *out_value = cpu->data.cr0; break;
    case CORE_MACHINE_DEBUG_CR1: *out_value = cpu->data.cr1; break;
    case CORE_MACHINE_DEBUG_CR2: *out_value = cpu->data.cr2; break;
    case CORE_MACHINE_DEBUG_CR3: *out_value = cpu->data.cr3; break;
    case CORE_MACHINE_DEBUG_CR4: *out_value = cpu->data.cr4; break;
    default: return LIB_STATUS_INVALID_ARGUMENT;
    }
    return LIB_STATUS_OK;
}

lib_status core_machine_cpu_debug_patch_registers(core_machine_cpu_execution_context *context,
    const core_machine_debug_register_patch *patch)
{
    const lib_u32 valid_mask =
        (1u << CORE_MACHINE_DEBUG_REGISTER_COUNT) - 1u;
    core_machine_cpu_execution_context candidate_context;
    t_cpu candidate_cpu;
    core_machine_debug_register register_id;

    if (context == LIB_NULL || patch == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (patch->mask == 0u || (patch->mask & ~valid_mask) != 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    candidate_cpu = *context->cpu;
    candidate_context = *context;
    candidate_context.cpu = &candidate_cpu;
    for (register_id = CORE_MACHINE_DEBUG_EAX;
         register_id < CORE_MACHINE_DEBUG_REGISTER_COUNT; ++register_id) {
        if ((patch->mask & CORE_MACHINE_DEBUG_REGISTER_MASK(register_id)) == 0u)
            continue;
        switch (register_id) {
        case CORE_MACHINE_DEBUG_EAX: candidate_cpu.data.eax = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_ECX: candidate_cpu.data.ecx = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_EDX: candidate_cpu.data.edx = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_EBX: candidate_cpu.data.ebx = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_ESP: candidate_cpu.data.esp = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_EBP: candidate_cpu.data.ebp = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_ESI: candidate_cpu.data.esi = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_EDI: candidate_cpu.data.edi = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_EIP: candidate_cpu.data.eip = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_EFLAGS: candidate_cpu.data.eflags = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_CR0: candidate_cpu.data.cr0 = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_CR1: candidate_cpu.data.cr1 = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_CR2: candidate_cpu.data.cr2 = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_CR3: candidate_cpu.data.cr3 = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_CR4: candidate_cpu.data.cr4 = patch->values[register_id]; break;
        default:
            if (core_machine_debug_patch_segment(&candidate_context,
                    &candidate_cpu, register_id, patch->values[register_id]))
                return LIB_STATUS_INVALID_STATE;
            break;
        }
    }
    *context->cpu = candidate_cpu;
    return LIB_STATUS_OK;
}

static lib_u32 core_machine_cpu_reset_code_base(
    core_machine_cpu_profile profile)
{
    switch (profile) {
    case CORE_MACHINE_CPU_PROFILE_8086:
    case CORE_MACHINE_CPU_PROFILE_8088:
    case CORE_MACHINE_CPU_PROFILE_80186:
        return 0x000ffff0u;
    case CORE_MACHINE_CPU_PROFILE_80286:
        return 0x00ff0000u;
    case CORE_MACHINE_CPU_PROFILE_DEFAULT:
    case CORE_MACHINE_CPU_PROFILE_80386:
        return 0xffff0000u;
    }
    return 0xffff0000u;
}

void core_machine_cpu_execution_context_initialize(
    core_machine_cpu_execution_context *context, t_cpu *cpu,
    t_cpuins *instructions, const core_machine_cpu_bus_provider *bus,
    void *bus_context)
{
    if (context == LIB_NULL) return;
    context->cpu = cpu;
    context->instructions = instructions;
    context->instruction_timing = (core_machine_instruction_timing) { .base_ticks = 1u };
    context->timing_result = (core_machine_cpu_timing_result) {0};
    context->bus = bus;
    context->bus_context = bus_context;
    context->diagnostic_provider = LIB_NULL;
    context->diagnostic_context = LIB_NULL;
    context->external_cycle_provider = LIB_NULL;
    context->external_cycle_context = LIB_NULL;
    context->stop_requested = LIB_FALSE;
    context->debug_pause_requested = LIB_FALSE;
    context->reset_requested = LIB_FALSE;
    context->debug_trap_pending = LIB_FALSE;
    context->debug_tf_before = LIB_FALSE;
    context->debug_rf_before = LIB_FALSE;
    context->debug_trap_cause = 0u;
    context->preview_mode = LIB_FALSE;
    context->memory_access_provenance = CORE_MACHINE_CPU_MEMORY_ACCESS_DATA;
    context->prefetch_count = 0u;
    context->prefetch_capacity = 15u;
    context->prefetch_valid = LIB_FALSE;
    context->prefetch_expected_valid = LIB_FALSE;
    context->prefetch_reservation_valid = LIB_FALSE;
    context->prefetch_reservation_linear = 0u;
    context->prefetch_reservation_count = 0u;
    context->cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    context->fpu_profile = X86_FPU_PROFILE_NONE;
    context->cpu_80386_cr_mov_ignores_mod = LIB_FALSE;
    context->fpu = LIB_NULL;
}

void core_machine_cpu_execution_context_bind_profiles(
    core_machine_cpu_execution_context *context,
    core_machine_cpu_profile cpu_profile,
    x86_fpu_profile fpu_profile,
    lib_u8 cpu_80386_cr_mov_ignores_mod, const core_machine_instruction_timing *timing)
{
    if (context == LIB_NULL) return;
    context->instruction_timing = *timing;
    context->cpu_profile = cpu_profile;
    context->prefetch_capacity = cpu_profile == CORE_MACHINE_CPU_PROFILE_8088 ?
        4u : 15u;
    context->fpu_profile = fpu_profile;
    context->cpu_80386_cr_mov_ignores_mod = cpu_80386_cr_mov_ignores_mod;
}

void core_machine_cpu_execution_context_bind_fpu(
    core_machine_cpu_execution_context *context, x86_fpu *fpu)
{
    if (context != LIB_NULL) context->fpu = fpu;
}

void core_machine_cpu_execution_context_bind_external_cycle_provider(
    core_machine_cpu_execution_context *context,
    core_machine_cpu_external_cycle_provider provider, void *provider_context)
{
    if (context == LIB_NULL) return;
    context->external_cycle_provider = provider;
    context->external_cycle_context = provider_context;
}

const char *core_machine_cpu_profile_name(core_machine_cpu_profile profile)
{
    switch (profile) {
    case CORE_MACHINE_CPU_PROFILE_8086: return "8086";
    case CORE_MACHINE_CPU_PROFILE_8088: return "8088";
    case CORE_MACHINE_CPU_PROFILE_80186: return "80186";
    case CORE_MACHINE_CPU_PROFILE_80286: return "80286";
    case CORE_MACHINE_CPU_PROFILE_80386: return "80386";
    case CORE_MACHINE_CPU_PROFILE_DEFAULT: return "default";
    }
    return "invalid";
}

void core_machine_cpu_execution_context_bind_diagnostic_provider(
    core_machine_cpu_execution_context *context,
    const core_machine_cpu_execution_diagnostic_provider *provider,
    void *provider_context)
{
    if (context == LIB_NULL) return;
    context->diagnostic_provider = provider;
    context->diagnostic_context = provider_context;
}

void core_machine_cpu_state_initialize(
    core_machine_cpu_execution_context *context) {
    if (context == LIB_NULL || context->cpu == LIB_NULL ||
        context->instructions == LIB_NULL) return;
    if (context != LIB_NULL) {
        context->stop_requested = LIB_FALSE;
        context->debug_pause_requested = LIB_FALSE;
        context->reset_requested = LIB_FALSE;
        context->shutdown_requested = LIB_FALSE;
        context->debug_trap_pending = LIB_FALSE;
        context->debug_tf_before = LIB_FALSE;
        context->debug_rf_before = LIB_FALSE;
        context->debug_trap_cause = 0u;
        context->prefetch_count = 0u;
        context->prefetch_capacity = context->cpu_profile ==
            CORE_MACHINE_CPU_PROFILE_8088 ? 4u : 15u;
        context->prefetch_valid = LIB_FALSE;
        context->prefetch_expected_valid = LIB_FALSE;
        context->prefetch_reservation_valid = LIB_FALSE;
        context->prefetch_reservation_linear = 0u;
        context->prefetch_reservation_count = 0u;
    }
    core_machine_cpu_execution_initialize(context);
}
void core_machine_cpu_state_reset(core_machine_cpu_execution_context *context) {
    if (context == LIB_NULL || context->cpu == LIB_NULL ||
        context->instructions == LIB_NULL) return;
    const lib_bool early = core_machine_cpu_profile_has_8086_semantics(
        context->cpu_profile) || context->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186;
    context->source_repeat_active = LIB_FALSE;
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
        context->reset_requested = LIB_FALSE;
        context->shutdown_requested = LIB_FALSE;
        context->prefetch_count = 0u;
        context->prefetch_capacity = context->cpu_profile ==
            CORE_MACHINE_CPU_PROFILE_8088 ? 4u : 15u;
        context->prefetch_valid = LIB_FALSE;
        context->prefetch_expected_valid = LIB_FALSE;
        context->prefetch_reservation_valid = LIB_FALSE;
        context->prefetch_reservation_linear = 0u;
        context->prefetch_reservation_count = 0u;
    }

    cpu_state.data.eip = early ? 0u : 0x0000fff0u;
    cpu_state.data.eflags = 0x00000002;
    /* Intel 80386 PRM 10.1 defines DH=3 after RESET# for the 386DX.
     * The selected zero revision keeps the documented device identifier
     * without inventing a board-specific stepping value. */
    if (context->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386)
        cpu_state.data.edx = 0x00000300u;

    cpu_state.data.cs.base = core_machine_cpu_reset_code_base(context->cpu_profile);
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

    core_machine_cpu_execution_reset(context);

}

void core_machine_cpu_execution_reserve_prefetch(
    core_machine_cpu_execution_context *context)
{
    lib_u32 offset;

    if (context == LIB_NULL || context->cpu == LIB_NULL ||
        (context->cpu->data.cr0 & VCPU_CR0_PG) ||
        context->prefetch_reservation_valid || !context->prefetch_valid ||
        context->prefetch_expected_linear < context->prefetch_linear ||
        cpu_state.data.eip > cpu_state.data.cs.limit) return;
    if (context->cpu_profile != CORE_MACHINE_CPU_PROFILE_8088) {
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

void core_machine_cpu_execution_advance_prefetch_reservation(
    core_machine_cpu_execution_context *context)
{
    lib_u8 byte;

    if (context == LIB_NULL || !context->prefetch_reservation_valid) return;
    if (context->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088) {
        context->memory_access_provenance =
            CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH;
        if (!core_machine_cpu_execution_read_linear(context,
                context->prefetch_reservation_linear, (lib_uptr)&byte, 1u) &&
            context->prefetch_count < context->prefetch_capacity) {
            context->prefetch_bytes[context->prefetch_count++] = byte;
        }
        context->memory_access_provenance = CORE_MACHINE_CPU_MEMORY_ACCESS_DATA;
    }
    context->prefetch_reservation_valid = LIB_FALSE;
    context->prefetch_reservation_linear = 0u;
    context->prefetch_reservation_count = 0u;
}
void core_machine_cpu_execution_invalidate_prefetch(
    core_machine_cpu_execution_context *context)
{
    if (context == LIB_NULL) return;
    context->prefetch_count = 0u;
    context->prefetch_valid = LIB_FALSE;
    context->prefetch_expected_valid = LIB_FALSE;
    context->prefetch_reservation_valid = LIB_FALSE;
    context->prefetch_reservation_linear = 0u;
    context->prefetch_reservation_count = 0u;
}

void core_machine_cpu_execution_request_stop(
    core_machine_cpu_execution_context *context)
{
    if (context != LIB_NULL) context->stop_requested = LIB_TRUE;
}
lib_u8 core_machine_cpu_execution_consume_stop_request(
    core_machine_cpu_execution_context *context)
{
    lib_u8 requested = context != LIB_NULL && context->stop_requested;
    if (context != LIB_NULL) context->stop_requested = LIB_FALSE;
    return requested;
}
void core_machine_cpu_execution_request_debug_pause(
    core_machine_cpu_execution_context *context)
{
    if (context != LIB_NULL) context->debug_pause_requested = LIB_TRUE;
}

lib_u8 core_machine_cpu_execution_consume_debug_pause_request(
    core_machine_cpu_execution_context *context)
{
    lib_u8 requested = context != LIB_NULL && context->debug_pause_requested;

    if (context != LIB_NULL) context->debug_pause_requested = LIB_FALSE;
    return requested;
}
void core_machine_cpu_execution_request_reset(
    core_machine_cpu_execution_context *context)
{
    if (context != LIB_NULL) context->reset_requested = LIB_TRUE;
}
lib_u8 core_machine_cpu_execution_consume_reset_request(
    core_machine_cpu_execution_context *context)
{
    lib_u8 requested = context != LIB_NULL && context->reset_requested;
    if (context != LIB_NULL) context->reset_requested = LIB_FALSE;
    return requested;
}
void core_machine_cpu_execution_request_shutdown(
    core_machine_cpu_execution_context *context)
{
    if (context != LIB_NULL) context->shutdown_requested = LIB_TRUE;
}
lib_u8 core_machine_cpu_execution_consume_shutdown_request(
    core_machine_cpu_execution_context *context)
{
    lib_u8 requested = context != LIB_NULL && context->shutdown_requested;
    if (context != LIB_NULL) context->shutdown_requested = LIB_FALSE;
    return requested;
}

lib_i32 core_machine_cpu_read_linear(core_machine_cpu_execution_context *context, lib_u32 linear, void *out_data, lib_u8 size)
{
    return core_machine_cpu_execution_read_linear(context, linear,
        (lib_uptr)out_data, size);
}

lib_i32 core_machine_cpu_write_linear(core_machine_cpu_execution_context *context,
    lib_u32 linear, const void *in_data, lib_u8 size)
{
    return core_machine_cpu_execution_write_linear(context, linear,
        (lib_uptr)in_data, size);
}

lib_i32 core_machine_cpu_get_code_default_size(const core_machine_cpu_execution_context *context)
{
    return cpu_state.data.cs.seg.exec.defsize;
}

lib_u32 core_machine_cpu_get_code_base(const core_machine_cpu_execution_context *context)
{
    return cpu_state.data.cs.base;
}

void core_machine_cpu_set_watchpoint(core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind, lib_u32 linear)
{
    switch (kind) {
    case CORE_MACHINE_CPU_WATCH_READ:
        instruction_state.data.wrLinear = linear;
        instruction_state.data.flagWR = LIB_TRUE;
        break;
    case CORE_MACHINE_CPU_WATCH_WRITE:
        instruction_state.data.wwLinear = linear;
        instruction_state.data.flagWW = LIB_TRUE;
        break;
    case CORE_MACHINE_CPU_WATCH_EXECUTE:
        instruction_state.data.weLinear = linear;
        instruction_state.data.flagWE = LIB_TRUE;
        break;
    }
}

void core_machine_cpu_clear_watchpoint(core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind)
{
    switch (kind) {
    case CORE_MACHINE_CPU_WATCH_READ:
        instruction_state.data.flagWR = LIB_FALSE;
        break;
    case CORE_MACHINE_CPU_WATCH_WRITE:
        instruction_state.data.flagWW = LIB_FALSE;
        break;
    case CORE_MACHINE_CPU_WATCH_EXECUTE:
        instruction_state.data.flagWE = LIB_FALSE;
        break;
    }
}

void core_machine_cpu_get_watchpoint(const core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind, lib_u8 *out_enabled,
    lib_u32 *out_linear)
{
    if (out_enabled == LIB_NULL || out_linear == LIB_NULL) return;
    *out_enabled = LIB_FALSE;
    *out_linear = 0u;
    switch (kind) {
    case CORE_MACHINE_CPU_WATCH_READ:
        *out_enabled = instruction_state.data.flagWR;
        *out_linear = instruction_state.data.wrLinear;
        break;
    case CORE_MACHINE_CPU_WATCH_WRITE:
        *out_enabled = instruction_state.data.flagWW;
        *out_linear = instruction_state.data.wwLinear;
        break;
    case CORE_MACHINE_CPU_WATCH_EXECUTE:
        *out_enabled = instruction_state.data.flagWE;
        *out_linear = instruction_state.data.weLinear;
        break;
    }
}
