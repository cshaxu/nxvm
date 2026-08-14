#include "type.h"

#include "core/machine/machine.h"

static C_VOID core_machine_cpu_diagnostic_copy_point(
    core_machine_cpu_execution_point *point, const t_cpu *cpu,
    const t_cpuins *instructions)
{
    if (point == STD_NULL || cpu == STD_NULL || instructions == STD_NULL) return;
    point->cs = cpu->data.cs.selector;
    point->cs_base = cpu->data.cs.base;
    point->eip = cpu->data.eip;
    point->linear_pc = instructions->data.linear;
    point->byte_count = (type_unsigned_8)instructions->data.oplen;
    STD_MEMCPY(point->bytes, instructions->data.opcodes, sizeof(point->bytes));
}

static C_VOID core_machine_cpu_diagnostic_record_instruction(C_VOID *opaque,
    const C_VOID *opaque_cpu, const t_cpuins *instructions)
{
    core_machine *machine = (core_machine *)opaque;
    const t_cpu *cpu = (const t_cpu *)opaque_cpu;
    core_machine_cpu_diagnostic_state *state;

    if (machine == STD_NULL) return;
    state = &machine->cpu_diagnostic;
    core_machine_cpu_diagnostic_copy_point(
        &state->snapshot.recent[state->next_index], cpu, instructions);
    state->next_index = (state->next_index + 1u) % CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY;
    if (state->snapshot.recent_count < CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY) {
        ++state->snapshot.recent_count;
    }
}

static C_VOID core_machine_cpu_diagnostic_record_fault(C_VOID *opaque,
    const C_VOID *opaque_cpu, const t_cpuins *instructions)
{
    core_machine *machine = (core_machine *)opaque;
    const t_cpu *cpu = (const t_cpu *)opaque_cpu;
    core_machine_cpu_fault_snapshot *fault;

    if (machine == STD_NULL || cpu == STD_NULL || instructions == STD_NULL) return;
    fault = &machine->cpu_diagnostic.snapshot.first_fault;
    if (fault->valid) return;
    STD_MEMSET(fault, 0, sizeof(*fault));
    fault->valid = 1;
    fault->exception_mask = instructions->data.except;
    fault->exception_code = instructions->data.excode;
    core_machine_cpu_diagnostic_copy_point(&fault->point, cpu, instructions);
    fault->eax = cpu->data.eax;
    fault->ebx = cpu->data.ebx;
    fault->ecx = cpu->data.ecx;
    fault->edx = cpu->data.edx;
    fault->cr2 = cpu->data.cr2;
    fault->esp = cpu->data.esp;
    fault->ebp = cpu->data.ebp;
    fault->esi = cpu->data.esi;
    fault->edi = cpu->data.edi;
    fault->eflags = cpu->data.eflags;
    (C_VOID)core_machine_report_fault(machine, fault->exception_mask);
}

static C_VOID core_machine_cpu_diagnostic_record_delivered_exception(
    C_VOID *opaque, const C_VOID *opaque_cpu, const t_cpuins *instructions)
{
    core_machine *machine = (core_machine *)opaque;
    const t_cpu *cpu = (const t_cpu *)opaque_cpu;
    core_machine_cpu_fault_snapshot *exception;

    if (machine == STD_NULL || cpu == STD_NULL || instructions == STD_NULL) return;
    exception = &machine->cpu_diagnostic.snapshot.last_delivered_exception;
    STD_MEMSET(exception, 0, sizeof(*exception));
    exception->valid = 1;
    exception->exception_mask = instructions->data.except;
    exception->exception_code = instructions->data.excode;
    core_machine_cpu_diagnostic_copy_point(&exception->point, cpu, instructions);
    exception->eax = cpu->data.eax;
    exception->ebx = cpu->data.ebx;
    exception->ecx = cpu->data.ecx;
    exception->edx = cpu->data.edx;
    exception->cr2 = cpu->data.cr2;
    exception->esp = cpu->data.esp;
    exception->ebp = cpu->data.ebp;
    exception->esi = cpu->data.esi;
    exception->edi = cpu->data.edi;
    exception->eflags = cpu->data.eflags;
    machine->cpu_diagnostic.snapshot.delivered_exception_count++;
}

static const core_machine_cpu_execution_diagnostic_provider
    core_machine_cpu_diagnostic_provider = {
        core_machine_cpu_diagnostic_record_instruction,
        core_machine_cpu_diagnostic_record_delivered_exception,
        core_machine_cpu_diagnostic_record_fault
    };

static C_VOID core_machine_cpu_diagnostic_ordered_copy(
    const core_machine_cpu_diagnostic_state *state,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    STD_SIZE_T index;
    STD_SIZE_T first;

    *out_diagnostic = state->snapshot;
    if (state->snapshot.recent_count < CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY ||
        state->next_index == 0u) return;
    first = state->next_index;
    for (index = 0u; index < CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY; ++index) {
        out_diagnostic->recent[index] = state->snapshot.recent[
            (first + index) % CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY];
    }
}

C_VOID core_machine_cpu_diagnostic_initialize(core_machine *machine)
{
    if (machine != STD_NULL) {
        STD_MEMSET(&machine->cpu_diagnostic, 0, sizeof(machine->cpu_diagnostic));
    }
}

C_VOID core_machine_cpu_diagnostic_reset(core_machine *machine)
{
    core_machine_cpu_diagnostic_initialize(machine);
}



static type_unsigned_32 core_machine_linear_pc(const core_machine *machine)
{
    return machine->executor_cpu.data.cs.base + machine->executor_cpu.data.eip;
}

static core_machine_cpu_profile core_machine_resolve_cpu_profile(
    core_machine_cpu_profile profile)
{
    return profile == CORE_MACHINE_CPU_PROFILE_DEFAULT ?
        CORE_MACHINE_CPU_PROFILE_80386 : profile;
}

static type_unsigned_32 core_machine_resolve_ticks_per_instruction(type_unsigned_32 ticks)
{
    return ticks == 0u ? 1u : ticks;
}

static C_VOID core_machine_resolve_instruction_timing(
    core_machine_instruction_timing *out_timing,
    const core_machine_instruction_timing *timing, type_unsigned_32 legacy_base)
{
    *out_timing = *timing;
    if (out_timing->base_ticks == 0u) {
        out_timing->base_ticks = core_machine_resolve_ticks_per_instruction(
            legacy_base);
    }
}

static C_INT core_machine_add_ticks(type_unsigned_64 *value, type_unsigned_64 delta)
{
    if (value == STD_NULL || UINT64_MAX - *value < delta) return 0;
    *value += delta;
    return 1;
}

static C_INT core_machine_instruction_is_prefix(type_unsigned_8 opcode)
{
    switch (opcode) {
    case 0xf0u: case 0xf2u: case 0xf3u: case 0x2eu: case 0x36u:
    case 0x3eu: case 0x26u: case 0x64u: case 0x65u: case 0x66u:
    case 0x67u:
        return 1;
    default:
        return 0;
    }
}

static type_unsigned_32 core_machine_instruction_prefix_count(const t_cpuins_data *data)
{
    type_unsigned_32 count = 0u;

    while (count < sizeof(data->opcodes) &&
        core_machine_instruction_is_prefix(data->opcodes[count])) {
        ++count;
    }
    return count;
}

typedef enum core_machine_source_timing_form {
    CORE_MACHINE_SOURCE_TIMING_NOP,
    CORE_MACHINE_SOURCE_TIMING_CLC,
    CORE_MACHINE_SOURCE_TIMING_MOV_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_MOV_RM_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_RM,
    CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_READ,
    CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_WRITE,
    CORE_MACHINE_SOURCE_TIMING_MOVSB,
    CORE_MACHINE_SOURCE_TIMING_IN_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_IN_DX,
    CORE_MACHINE_SOURCE_TIMING_OUT_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_OUT_DX
} core_machine_source_timing_form;

typedef struct core_machine_source_timing_entry {
    core_machine_source_timing_form form;
    type_unsigned_8 ticks;
} core_machine_source_timing_entry;

/* Intel 80386 PRM section 17.2.2.3 selected rows.  These are core clocks
 * under the manual's prefetched/no-wait/no-HOLD assumptions; they are not
 * device service, bus arbitration, or host-time values. */
static const core_machine_source_timing_entry
    core_machine_80386_source_timing_ledger[] = {
    { CORE_MACHINE_SOURCE_TIMING_NOP, 3u },
    { CORE_MACHINE_SOURCE_TIMING_CLC, 2u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_IMMEDIATE, 2u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_REGISTER, 2u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_RM_REGISTER, 2u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_RM, 4u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_READ, 4u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_WRITE, 2u },
    { CORE_MACHINE_SOURCE_TIMING_MOVSB, 7u },
    { CORE_MACHINE_SOURCE_TIMING_IN_IMMEDIATE, 12u },
    { CORE_MACHINE_SOURCE_TIMING_IN_DX, 13u },
    { CORE_MACHINE_SOURCE_TIMING_OUT_IMMEDIATE, 10u },
    { CORE_MACHINE_SOURCE_TIMING_OUT_DX, 11u }
};

#define CORE_MACHINE_80386_JCC_NOT_TAKEN_TICKS 3u
#define CORE_MACHINE_80386_JCC_TAKEN_TICKS 7u
#define CORE_MACHINE_80386_REP_MOVSB_SETUP_TICKS 5u
#define CORE_MACHINE_80386_REP_MOVSB_ITERATION_TICKS 4u
#define CORE_MACHINE_80386_SOURCE_UNALLOCATED_TICKS 1u
#define CORE_MACHINE_80386_SOURCE_MAXIMUM_TICKS 22u

static type_unsigned_64 core_machine_source_timing_lookup(
    core_machine_source_timing_form form)
{
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(core_machine_80386_source_timing_ledger) /
            sizeof(core_machine_80386_source_timing_ledger[0]); ++index) {
        if (core_machine_80386_source_timing_ledger[index].form == form) {
            return core_machine_80386_source_timing_ledger[index].ticks;
        }
    }
    return CORE_MACHINE_80386_SOURCE_UNALLOCATED_TICKS;
}

static C_INT core_machine_source_timing_modrm_is_memory(
    const t_cpuins_data *data, type_unsigned_32 opcode_index)
{
    return opcode_index + 1u < data->oplen &&
        (data->opcodes[opcode_index + 1u] >> 6u) != 3u;
}

static type_unsigned_64 core_machine_source_timing_rep_movsb(
    core_machine *machine, const t_cpuins_data *data)
{
    type_unsigned_32 count = data->prefix_addrsize ? data->oldcpu.data.ecx :
        data->oldcpu.data.cx;
    C_INT continuing = machine->source_repeat_active &&
        machine->source_repeat_cs == data->oldcpu.data.cs.selector &&
        machine->source_repeat_eip == data->oldcpu.data.eip;
    type_unsigned_64 ticks = count == 0u ?
        CORE_MACHINE_80386_REP_MOVSB_SETUP_TICKS : continuing ?
        CORE_MACHINE_80386_REP_MOVSB_ITERATION_TICKS :
        CORE_MACHINE_80386_REP_MOVSB_SETUP_TICKS +
            CORE_MACHINE_80386_REP_MOVSB_ITERATION_TICKS;

    machine->source_repeat_active = count != 0u &&
        machine->executor_cpu.data.eip == data->oldcpu.data.eip;
    if (machine->source_repeat_active) {
        machine->source_repeat_cs = data->oldcpu.data.cs.selector;
        machine->source_repeat_eip = data->oldcpu.data.eip;
    }
    return ticks;
}

static C_INT core_machine_source_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks)
{
    const t_cpuins_data *data = &machine->executor_cpu_instructions.data;
    type_unsigned_32 prefixes = core_machine_instruction_prefix_count(data);
    type_unsigned_8 opcode;
    type_unsigned_32 fallthrough;
    core_machine_cpu_instruction_lexeme lexeme;

    if (out_ticks == STD_NULL) return 0;
    if (prefixes >= data->oplen) {
        machine->source_repeat_active = TYPE_FALSE;
        *out_ticks = 0u;
        return 1;
    }
    opcode = data->opcodes[prefixes];
    if (data->prefix_rep == PREFIX_REP_REPZ && opcode == 0xa4u) {
        *out_ticks = core_machine_source_timing_rep_movsb(machine, data);
        return 1;
    }
    machine->source_repeat_active = TYPE_FALSE;
    if (prefixes != 0u) {
        *out_ticks = CORE_MACHINE_80386_SOURCE_UNALLOCATED_TICKS;
        return 1;
    }
    if (opcode >= 0x70u && opcode <= 0x7fu) {
        fallthrough = data->oldcpu.data.eip + 2u;
        if (!data->oldcpu.data.cs.seg.exec.defsize) fallthrough &= 0xffffu;
        if (machine->executor_cpu.data.eip == fallthrough) {
            *out_ticks = CORE_MACHINE_80386_JCC_NOT_TAKEN_TICKS;
        } else if (core_machine_cpu_execution_preview_lexeme(
                &machine->executor_cpu_execution, &lexeme) && lexeme.available) {
            *out_ticks = CORE_MACHINE_80386_JCC_TAKEN_TICKS +
                lexeme.component_count;
        } else {
            *out_ticks = CORE_MACHINE_80386_SOURCE_UNALLOCATED_TICKS;
        }
        return 1;
    }
    switch (opcode) {
    case 0x90u:
        *out_ticks = core_machine_source_timing_lookup(
            CORE_MACHINE_SOURCE_TIMING_NOP);
        return 1;
    case 0xf8u:
        *out_ticks = core_machine_source_timing_lookup(
            CORE_MACHINE_SOURCE_TIMING_CLC);
        return 1;
    case 0x88u: case 0x89u:
        *out_ticks = core_machine_source_timing_lookup(
            core_machine_source_timing_modrm_is_memory(data, prefixes) ?
            CORE_MACHINE_SOURCE_TIMING_MOV_RM_REGISTER :
            CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_REGISTER);
        return 1;
    case 0x8au: case 0x8bu:
        *out_ticks = core_machine_source_timing_lookup(
            core_machine_source_timing_modrm_is_memory(data, prefixes) ?
            CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_RM :
            CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_REGISTER);
        return 1;
    case 0xa0u: case 0xa1u:
        *out_ticks = core_machine_source_timing_lookup(
            CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_READ);
        return 1;
    case 0xa2u: case 0xa3u:
        *out_ticks = core_machine_source_timing_lookup(
            CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_WRITE);
        return 1;
    case 0xa4u:
        *out_ticks = core_machine_source_timing_lookup(
            CORE_MACHINE_SOURCE_TIMING_MOVSB);
        return 1;
    case 0xe4u: case 0xe5u:
        if ((machine->executor_cpu.data.cr0 & VCPU_CR0_PE) != 0u) break;
        *out_ticks = core_machine_source_timing_lookup(
            CORE_MACHINE_SOURCE_TIMING_IN_IMMEDIATE);
        return 1;
    case 0xecu: case 0xedu:
        if ((machine->executor_cpu.data.cr0 & VCPU_CR0_PE) != 0u) break;
        *out_ticks = core_machine_source_timing_lookup(
            CORE_MACHINE_SOURCE_TIMING_IN_DX);
        return 1;
    case 0xe6u: case 0xe7u:
        if ((machine->executor_cpu.data.cr0 & VCPU_CR0_PE) != 0u) break;
        *out_ticks = core_machine_source_timing_lookup(
            CORE_MACHINE_SOURCE_TIMING_OUT_IMMEDIATE);
        return 1;
    case 0xeeu: case 0xefu:
        if ((machine->executor_cpu.data.cr0 & VCPU_CR0_PE) != 0u) break;
        *out_ticks = core_machine_source_timing_lookup(
            CORE_MACHINE_SOURCE_TIMING_OUT_DX);
        return 1;
    default:
        if (opcode >= 0xb0u && opcode <= 0xbfu) {
            *out_ticks = core_machine_source_timing_lookup(
                CORE_MACHINE_SOURCE_TIMING_MOV_IMMEDIATE);
        } else {
            *out_ticks = CORE_MACHINE_80386_SOURCE_UNALLOCATED_TICKS;
        }
        return 1;
    }
    *out_ticks = CORE_MACHINE_80386_SOURCE_UNALLOCATED_TICKS;
    return 1;
}

static type_unsigned_64 core_machine_instruction_maximum_ticks(
    const core_machine_instruction_timing *timing)
{
    return (type_unsigned_64)timing->base_ticks +
        (type_unsigned_64)timing->prefix_surcharge * 15u +
        timing->taken_branch_surcharge + timing->data_memory_surcharge +
        timing->io_surcharge + timing->rep_iteration_surcharge;
}

static C_INT core_machine_compatibility_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks)
{
    const t_cpuins_data *data = &machine->executor_cpu_instructions.data;
    const core_machine_instruction_timing *timing =
        &machine->instruction_timing;
    type_unsigned_32 prefixes = core_machine_instruction_prefix_count(data);
    type_unsigned_8 opcode;
    type_unsigned_64 ticks = timing->base_ticks;
    type_unsigned_32 fallthrough;
    type_bool code32;

    if (prefixes >= sizeof(data->opcodes)) return 0;
    opcode = data->opcodes[prefixes];
    if (data->prefix_rep != PREFIX_REP_NONE && opcode == 0xa4u) {
        if (!core_machine_add_ticks(&ticks, timing->rep_iteration_surcharge)) {
            return 0;
        }
    } else if (!core_machine_add_ticks(&ticks,
            (type_unsigned_64)prefixes * timing->prefix_surcharge)) {
        return 0;
    }
    if (opcode >= 0x70u && opcode <= 0x7fu) {
        code32 = data->oldcpu.data.cs.seg.exec.defsize;
        fallthrough = data->oldcpu.data.eip + prefixes + 2u;
        if (!code32) fallthrough &= 0xffffu;
        if (machine->executor_cpu.data.eip != fallthrough &&
            !core_machine_add_ticks(&ticks, timing->taken_branch_surcharge)) {
            return 0;
        }
    }
    if (opcode >= 0xa0u && opcode <= 0xa3u &&
        !core_machine_add_ticks(&ticks, timing->data_memory_surcharge)) {
        return 0;
    }
    if (((opcode >= 0xe4u && opcode <= 0xe7u) ||
         (opcode >= 0xecu && opcode <= 0xefu)) &&
        !core_machine_add_ticks(&ticks, timing->io_surcharge)) {
        return 0;
    }
    *out_ticks = ticks;
    return 1;
}

static C_INT core_machine_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks)
{
    if (machine == STD_NULL || out_ticks == STD_NULL) return 0;
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) {
        return core_machine_source_instruction_cost(machine, out_ticks);
    }
    return core_machine_compatibility_instruction_cost(machine, out_ticks);
}

static C_VOID core_machine_transaction_trace(C_VOID *opaque,
    core_machine_transaction_owner owner, core_machine_transaction_kind kind,
    core_machine_transaction_phase phase, type_unsigned_32 address,
    type_unsigned_32 value, type_unsigned_32 detail)
{
    core_machine *machine = (core_machine *)opaque;
    core_machine_trace_event_type type;

    if (machine == STD_NULL) return;
    switch (phase) {
    case CORE_MACHINE_TRANSACTION_PHASE_BEGIN:
        type = CORE_MACHINE_TRACE_TRANSACTION_BEGIN;
        break;
    case CORE_MACHINE_TRANSACTION_PHASE_COMMIT:
        type = CORE_MACHINE_TRACE_TRANSACTION_COMMIT;
        break;
    case CORE_MACHINE_TRANSACTION_PHASE_CANCEL:
        type = CORE_MACHINE_TRACE_TRANSACTION_CANCEL;
        break;
    default:
        return;
    }
    core_machine_trace_record(machine, type, address, value,
        (type_unsigned_32)owner | ((type_unsigned_32)kind << 8u) |
        (detail << 16u));
}

static C_INT core_machine_clock_plan_is_valid(
    const core_machine_clock_plan *plan)
{
    return plan != STD_NULL &&
        core_machine_clock_ratio_is_valid(&plan->dma) &&
        core_machine_clock_ratio_is_valid(&plan->pit) &&
        core_machine_clock_ratio_is_valid(&plan->rtc) &&
        core_machine_clock_ratio_is_valid(&plan->vadp) &&
        core_machine_clock_ratio_is_valid(&plan->kbc) &&
        core_machine_clock_ratio_is_valid(&plan->provider);
}

/*
 * PIC/PIT/DMA share one immediate arbitration boundary. The callback owns one
 * machine tick; scheduling the next tick from the callback preserves both
 * deterministic due-time order and the existing one-grant DMA semantics.
 */
static C_VOID core_machine_arbitration_tick(C_VOID *opaque,
    type_unsigned_64 due_tick)
{
    core_machine *machine = (core_machine *)opaque;
    type_unsigned_64 dma_ticks;
    type_unsigned_64 pit_ticks;
    core_machine_timeline_token next;

    if (machine == STD_NULL) {
        return;
    }
    dma_ticks = core_machine_clock_domain_advance(&machine->dma_clock, 1u);
    pit_ticks = core_machine_clock_domain_advance(&machine->pit_clock, 1u);
    core_machine_dma_advance_transaction(&machine->shared_dma_latch,
        &machine->shared_dma_primary, &machine->shared_dma_secondary,
        &machine->executor_memory, &machine->transaction, dma_ticks);
    if (dma_ticks != 0u) {
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_DMA_ADVANCE,
            0u, (type_unsigned_32)dma_ticks, 0u);
    }
    core_machine_pit_advance(&machine->shared_pit, pit_ticks);
    if (pit_ticks != 0u) {
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_PIT_ADVANCE,
            0u, (type_unsigned_32)pit_ticks, 0u);
    }
    core_machine_pic_refresh(&machine->shared_pic_master,
        &machine->shared_pic_slave);
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_PIC_REFRESH,
        0u, 0u, 0u);
    if (due_tick != UINT64_MAX) {
        (C_VOID)core_machine_timeline_schedule(&machine->timeline,
            due_tick + 1u, core_machine_arbitration_tick, machine, &next);
    }
}

/*
 * RTC progression and removable-media observation have a distinct readiness
 * boundary.  This callback intentionally follows the immediate DMA/PIT/PIC
 * arbitration callback at a shared due tick: sources made ready here become
 * eligible for PIC arbitration at the following due tick. FDC and ATA command
 * and completion service are advanced here before their retained observation
 * refresh paths.
 */
static C_VOID core_machine_readiness_tick(C_VOID *opaque,
    type_unsigned_64 due_tick)
{
    core_machine *machine = (core_machine *)opaque;
    type_unsigned_64 rtc_ticks;
    core_machine_timeline_token next;

    if (machine == STD_NULL) {
        return;
    }
    core_machine_fdc_advance(&machine->fdc);
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_FDC_ADVANCE,
        0u, 0u, 0u);
    core_machine_fdc_refresh(&machine->fdc);
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_FDC_REFRESH,
        0u, 0u, 0u);
    core_machine_hdc_advance(&machine->hdc);
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_HDC_ADVANCE,
        0u, 0u, 0u);
    core_machine_hdc_refresh(&machine->hdc);
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_HDC_REFRESH,
        0u, 0u, 0u);
    rtc_ticks = core_machine_clock_domain_advance(&machine->rtc_clock, 1u);
    if (machine->rtc_cmos_configured) {
        core_machine_rtc_advance(&machine->shared_rtc, rtc_ticks);
    }
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_RTC_ADVANCE,
        0u, (type_unsigned_32)rtc_ticks, 0u);
    if (due_tick != UINT64_MAX) {
        (C_VOID)core_machine_timeline_schedule(&machine->timeline,
            due_tick + 1u, core_machine_readiness_tick, machine, &next);
    }
}

/*
 * Guest input and video state advance after the readiness boundary.  Host
 * presentation consumes only copied snapshots outside this callback and does
 * not participate in machine time.
 */
static C_VOID core_machine_peripheral_tick(C_VOID *opaque,
    type_unsigned_64 due_tick)
{
    core_machine *machine = (core_machine *)opaque;
    type_unsigned_64 kbc_ticks;
    type_unsigned_64 vadp_ticks;
    core_machine_timeline_token next;

    if (machine == STD_NULL) {
        return;
    }
    kbc_ticks = core_machine_clock_domain_advance(&machine->kbc_clock, 1u);
    core_machine_kbc_advance(&machine->shared_kbc, kbc_ticks);
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_KBC_ADVANCE,
        0u, (type_unsigned_32)kbc_ticks, 0u);
    vadp_ticks = core_machine_clock_domain_advance(&machine->vadp_clock, 1u);
    core_machine_vadp_advance(&machine->shared_vadp, &machine->executor_memory,
        vadp_ticks);
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_VADP_ADVANCE,
        0u, (type_unsigned_32)vadp_ticks, 0u);
    if (due_tick != UINT64_MAX) {
        (C_VOID)core_machine_timeline_schedule(&machine->timeline,
            due_tick + 1u, core_machine_peripheral_tick, machine, &next);
    }
}

static C_VOID core_machine_advance_scheduler(core_machine *machine,
    type_unsigned_64 elapsed_ticks)
{
    type_unsigned_64 provider_ticks;

    (C_VOID)core_machine_timeline_advance(&machine->timeline,
        machine->elapsed_ticks);
    provider_ticks = core_machine_clock_domain_advance(&machine->provider_clock,
        elapsed_ticks);
    if (machine->execution_provider != STD_NULL &&
        machine->execution_provider->advance_time != STD_NULL) {
        machine->execution_provider->advance_time(
            machine->execution_provider_context, provider_ticks);
    }
}

static C_INT core_machine_valid_cpu_profile(core_machine_cpu_profile profile)
{
    return profile >= CORE_MACHINE_CPU_PROFILE_8086 &&
        profile <= CORE_MACHINE_CPU_PROFILE_80386;
}

static C_INT core_machine_valid_fpu_profile(core_machine_fpu_profile profile)
{
    return profile >= CORE_MACHINE_FPU_PROFILE_NONE &&
        profile <= CORE_MACHINE_FPU_PROFILE_80387;
}

C_INT core_machine_configuration_is_open(const core_machine *machine)
{
    return machine != STD_NULL &&
        machine->lifecycle == CORE_MACHINE_INITIALIZED &&
        !machine->execution_provider_frozen && !machine->firmware_operation_active;
}

C_INT core_machine_mutable_operation_is_allowed(const core_machine *machine)
{
    return machine != STD_NULL && !machine->firmware_operation_active;
}

static type_status core_machine_firmware_invoke(core_machine *machine,
    C_INT configuring, type_status (*callback)(C_VOID *,
    core_machine_firmware_context *))
{
    type_status status;

    if (machine == STD_NULL || callback == STD_NULL ||
        machine->firmware_operation_active) return TYPE_STATUS_INVALID_STATE;
    machine->firmware_operation_active = 1;
    machine->firmware_context.machine = machine;
    machine->firmware_context.configuring = configuring;
    machine->firmware_context.active = 1;
    status = callback(machine->firmware_provider_context,
        &machine->firmware_context);
    machine->firmware_context.active = 0;
    machine->firmware_context.configuring = 0;
    machine->firmware_operation_active = 0;
    return status;
}

static C_INT core_machine_firmware_context_is_active(
    const core_machine_firmware_context *firmware, C_INT configuring)
{
    return firmware != STD_NULL && firmware->active &&
        firmware->machine != STD_NULL &&
        firmware->configuring == configuring &&
        firmware->machine->firmware_operation_active;
}

type_status core_machine_bind_firmware_provider(core_machine *machine,
    const core_machine_firmware_provider *provider, C_VOID *provider_context)
{
    type_status status;
    STD_SIZE_T rom_mapping_boundary;

    if (!core_machine_configuration_is_open(machine) ||
        machine->firmware_provider != STD_NULL || provider == STD_NULL ||
        provider->configure == STD_NULL || provider->reset == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    rom_mapping_boundary = machine->immutable_rom_mapping_count;
    machine->firmware_provider = provider;
    machine->firmware_provider_context = provider_context;
    status = core_machine_firmware_invoke(machine, 1, provider->configure);
    if (status != TYPE_STATUS_OK) {
        core_machine_rollback_immutable_rom_mappings(machine, rom_mapping_boundary);
        machine->firmware_provider = STD_NULL;
        machine->firmware_provider_context = STD_NULL;
        STD_MEMSET(&machine->firmware_context, 0, sizeof(machine->firmware_context));
        return status;
    }
    return TYPE_STATUS_OK;
}

type_status core_machine_firmware_register_immutable_rom(
    core_machine_firmware_context *firmware, type_unsigned_32 physical_start,
    const type_unsigned_8 *image, STD_SIZE_T bytes)
{
    if (!core_machine_firmware_context_is_active(firmware, 1)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return core_machine_register_immutable_rom_mapping_from_firmware(firmware->machine,
        physical_start, image, bytes);
}

type_status core_machine_firmware_memory_read(
    core_machine_firmware_context *firmware, type_unsigned_32 physical,
    C_VOID *out_data, STD_SIZE_T size)
{
    if (!core_machine_firmware_context_is_active(firmware, 0) ||
        out_data == STD_NULL || size == 0u) return TYPE_STATUS_INVALID_STATE;
    return core_machine_memory_read_physical(&firmware->machine->executor_memory,
        physical, (type_virtual_address)out_data, size);
}

type_status core_machine_firmware_memory_write(
    core_machine_firmware_context *firmware, type_unsigned_32 physical,
    const C_VOID *data, STD_SIZE_T size)
{
    if (!core_machine_firmware_context_is_active(firmware, 0) ||
        data == STD_NULL || size == 0u) return TYPE_STATUS_INVALID_STATE;
    return core_machine_memory_write_physical(&firmware->machine->executor_memory,
        physical, (type_virtual_address)data, size);
}

type_status core_machine_firmware_port_read(
    core_machine_firmware_context *firmware, type_unsigned_16 port, type_unsigned_32 *out_value)
{
    if (!core_machine_firmware_context_is_active(firmware, 0) ||
        out_value == STD_NULL) return TYPE_STATUS_INVALID_STATE;
    {
        type_status status = core_machine_port_execute_read(
            &firmware->machine->executor_port, port);

        if (status != TYPE_STATUS_OK) return status;
    }
    *out_value = firmware->machine->executor_port.data.ioDWord;
    return TYPE_STATUS_OK;
}

type_status core_machine_firmware_port_write(
    core_machine_firmware_context *firmware, type_unsigned_16 port, type_unsigned_32 value)
{
    if (!core_machine_firmware_context_is_active(firmware, 0)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    {
        type_unsigned_32 prior_value = firmware->machine->executor_port.data.ioDWord;
        type_status status;

        firmware->machine->executor_port.data.ioDWord = value;
        status = core_machine_port_execute_write(&firmware->machine->executor_port,
            port);
        if (status != TYPE_STATUS_OK) {
            firmware->machine->executor_port.data.ioDWord = prior_value;
        }
        return status;
    }
}

type_status core_machine_firmware_request_stop(
    core_machine_firmware_context *firmware)
{
    if (!core_machine_firmware_context_is_active(firmware, 0)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    STD_ATOMIC_STORE(&firmware->machine->stop_requested, 1);
    return TYPE_STATUS_OK;
}

type_status core_machine_capture_display_snapshot(const core_machine *machine,
    core_machine_display_snapshot *out_snapshot)
{
    core_machine *mutable_machine = (core_machine *)machine;

    if (machine == STD_NULL || out_snapshot == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return core_machine_vadp_capture_snapshot(&mutable_machine->shared_vadp,
        &mutable_machine->executor_memory, out_snapshot) ? TYPE_STATUS_OK :
        TYPE_STATUS_UNSUPPORTED;
}

static C_INT core_machine_display_ports_are_vadp(
    const core_machine_display_port_topology *ports)
{
    return ports != STD_NULL &&
        ports->attribute_first == CORE_MACHINE_VADP_PORT_ATTRIBUTE &&
        ports->attribute_last == CORE_MACHINE_VADP_PORT_ATTRIBUTE_DATA_READ &&
        ports->sequencer_first == CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX &&
        ports->sequencer_last == CORE_MACHINE_VADP_PORT_SEQUENCER_DATA &&
        ports->graphics_first == CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX &&
        ports->graphics_last == CORE_MACHINE_VADP_PORT_GRAPHICS_DATA &&
        ports->crtc_first == CORE_MACHINE_VADP_PORT_CRTC_INDEX &&
        ports->crtc_last == CORE_MACHINE_VADP_PORT_STATUS;
}

type_status core_machine_configure_display(core_machine *machine,
    const core_machine_display_config *config)
{
    type_status status;

    if (!core_machine_configuration_is_open(machine) || machine->display_configured) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (config == STD_NULL || !core_machine_display_ports_are_vadp(&config->ports)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_vadp_configure_text_timing(&machine->shared_vadp,
        &config->text_timing);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_vadp_configure_ega_sequencer(&machine->shared_vadp,
        &machine->executor_memory, &config->ega_sequencer);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_vadp_configure_ega_controllers(&machine->shared_vadp,
        &config->ega_controllers);
    if (status != TYPE_STATUS_OK) return status;
    machine->display_ports = config->ports;
    machine->display_configured = TYPE_TRUE;
    if (config->provider != STD_NULL) {
        core_machine_display_provider_slot_freeze(config->provider);
    }
    return TYPE_STATUS_OK;
}

static C_INT core_machine_rtc_cmos_config_is_valid(
    const core_machine_rtc_cmos_config *config)
{
    STD_SIZE_T index;

    if (config == STD_NULL || config->data_port !=
        (type_unsigned_16)(config->index_port + 1u) || config->nmi_mask_bit == 0u ||
        config->default_count > CORE_MACHINE_RTC_DEFAULT_COUNT) {
        return TYPE_FALSE;
    }
    for (index = 0u; index < config->default_count; ++index) {
        type_unsigned_8 register_index = config->defaults[index].index;

        if (register_index >= CORE_MACHINE_RTC_REGISTER_COUNT ||
            register_index == CORE_MACHINE_RTC_REG_A ||
            register_index == CORE_MACHINE_RTC_REG_B ||
            register_index == CORE_MACHINE_RTC_REG_C ||
            register_index == CORE_MACHINE_RTC_REG_D) {
            return TYPE_FALSE;
        }
    }
    return TYPE_TRUE;
}

static type_status core_machine_rtc_cmos_port_read(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 *out_value)
{
    core_machine *machine = (core_machine *)owner;

    if (machine == STD_NULL || out_value == STD_NULL ||
        port != machine->rtc_cmos_config.data_port) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_value = core_machine_rtc_read_selected(&machine->shared_rtc);
    return TYPE_STATUS_OK;
}

static type_status core_machine_rtc_cmos_port_write(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 value)
{
    core_machine *machine = (core_machine *)owner;

    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (port == machine->rtc_cmos_config.index_port) {
        (C_VOID)core_machine_set_nmi_mask(machine,
            (value & machine->rtc_cmos_config.nmi_mask_bit) != 0u ?
            TYPE_TRUE : TYPE_FALSE);
        core_machine_rtc_select_register(&machine->shared_rtc, (type_unsigned_8)value);
        return TYPE_STATUS_OK;
    }
    if (port == machine->rtc_cmos_config.data_port) {
        core_machine_rtc_write_selected(&machine->shared_rtc, (type_unsigned_8)value);
        return TYPE_STATUS_OK;
    }
    return TYPE_STATUS_INVALID_ARGUMENT;
}

static const core_machine_port_provider core_machine_rtc_cmos_port_provider = {
    core_machine_rtc_cmos_port_read,
    core_machine_rtc_cmos_port_write
};

static const core_machine_port_provider core_machine_rtc_cmos_index_port_provider = {
    STD_NULL,
    core_machine_rtc_cmos_port_write
};

static C_VOID core_machine_fdc_dma_request_assert(C_VOID *owner,
    const core_machine_dma_request_binding *binding)
{
    core_machine *machine = owner;

    if (machine == STD_NULL || binding == STD_NULL ||
        binding->core_token != machine->fdc_dma_request.core_token ||
        binding->channel != machine->fdc_dma_request.channel) return;
    core_machine_dma_request_assert(&machine->shared_dma_primary,
        &machine->shared_dma_secondary, binding);
}

static C_VOID core_machine_fdc_dma_request_deassert(C_VOID *owner,
    const core_machine_dma_request_binding *binding)
{
    core_machine *machine = owner;

    if (machine == STD_NULL || binding == STD_NULL ||
        binding->core_token != machine->fdc_dma_request.core_token ||
        binding->channel != machine->fdc_dma_request.channel) return;
    core_machine_dma_request_deassert(&machine->shared_dma_primary,
        &machine->shared_dma_secondary, binding);
}

type_status core_machine_configure_dma(core_machine *machine,
    const core_machine_dma_wiring *wiring,
    core_machine_dma_request_binding *out_fdc_request)
{
    type_status status;

    if (!core_machine_configuration_is_open(machine) || machine->dma_configured) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (wiring == STD_NULL || out_fdc_request == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_dma_bind_channel(&machine->shared_dma_latch,
        &machine->shared_dma_primary, &machine->shared_dma_secondary,
        wiring->fdc_channel, core_machine_fdc_dma_provider(), &machine->fdc,
        &machine->fdc_dma_request);
    if (status != TYPE_STATUS_OK) return status;
    machine->dma_wiring = *wiring;
    machine->dma_configured = TYPE_TRUE;
    *out_fdc_request = machine->fdc_dma_request;
    return TYPE_STATUS_OK;
}

type_status core_machine_configure_rtc_cmos(core_machine *machine,
    const core_machine_rtc_cmos_config *config)
{
    core_machine_rtc_config rtc_config;
    core_machine_port_provider_entry *port_checkpoint;
    type_status status;
    STD_SIZE_T index;

    if (!core_machine_configuration_is_open(machine) ||
        machine->rtc_cmos_configured) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (!core_machine_rtc_cmos_config_is_valid(config)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (core_machine_port_has_write(&machine->executor_port,
            config->index_port) || core_machine_port_has_read(
            &machine->executor_port, config->data_port) ||
        core_machine_port_has_write(&machine->executor_port,
            config->data_port)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    port_checkpoint = core_machine_port_registration_begin(&machine->executor_port);
    status = core_machine_install_port_provider(machine, config->index_port,
        config->index_port, &core_machine_rtc_cmos_index_port_provider, machine);
    if (status != TYPE_STATUS_OK) {
        core_machine_port_rollback_registration(&machine->executor_port,
            port_checkpoint);
        return status;
    }
    status = core_machine_install_port_provider(machine, config->data_port,
        config->data_port, &core_machine_rtc_cmos_port_provider, machine);
    if (status != TYPE_STATUS_OK) {
        core_machine_port_rollback_registration(&machine->executor_port,
            port_checkpoint);
        return status;
    }
    rtc_config.irq = config->irq;
    rtc_config.ticks_per_second = config->ticks_per_second;
    core_machine_rtc_initialize(&machine->shared_rtc, &machine->shared_pic_master,
        &machine->shared_pic_slave, &rtc_config);
    for (index = 0u; index < config->default_count; ++index) {
        core_machine_rtc_write_nvram(&machine->shared_rtc,
            config->defaults[index].index, config->defaults[index].value);
    }
    machine->rtc_cmos_config = *config;
    machine->rtc_cmos_configured = TYPE_TRUE;
    return TYPE_STATUS_OK;
}

static C_INT core_machine_fdc_topology_is_valid(
    const core_machine_fdc_topology *topology)
{
    STD_SIZE_T first;
    STD_SIZE_T second;

    if (topology == STD_NULL || topology->media_registry == STD_NULL ||
        topology->config.dma_channel != topology->dma_request.channel) {
        return 0;
    }
    for (first = 0u; first < CORE_MACHINE_FDC_DRIVE_COUNT; ++first) {
        if (topology->drives.media_id[first] == CORE_MACHINE_MEDIA_ID_INVALID) {
            continue;
        }
        for (second = first + 1u; second < CORE_MACHINE_FDC_DRIVE_COUNT; ++second) {
            if (topology->drives.media_id[first] == topology->drives.media_id[second]) {
                return 0;
            }
        }
    }
    return 1;
}

static C_INT core_machine_hdc_topology_is_valid(
    const core_machine_hdc_topology *topology)
{
    const core_machine_hdc_config *config;
    const type_unsigned_16 ports[] = {
        topology == STD_NULL ? 0u : topology->config.data_port,
        topology == STD_NULL ? 0u : topology->config.error_features_port,
        topology == STD_NULL ? 0u : topology->config.sector_count_port,
        topology == STD_NULL ? 0u : topology->config.sector_number_port,
        topology == STD_NULL ? 0u : topology->config.cylinder_low_port,
        topology == STD_NULL ? 0u : topology->config.cylinder_high_port,
        topology == STD_NULL ? 0u : topology->config.drive_head_port,
        topology == STD_NULL ? 0u : topology->config.status_command_port,
        topology == STD_NULL ? 0u :
            topology->config.alternate_status_device_control_port
    };
    STD_SIZE_T first;
    STD_SIZE_T second;

    if (topology == STD_NULL || topology->media_registry == STD_NULL ||
        topology->media_id == CORE_MACHINE_MEDIA_ID_INVALID) return 0;
    config = &topology->config;
    if (config->lba28_supported != TYPE_FALSE && config->lba28_supported != TYPE_TRUE) {
        return 0;
    }
    for (first = 0u; first < sizeof(ports) / sizeof(ports[0]); ++first) {
        for (second = first + 1u; second < sizeof(ports) / sizeof(ports[0]); ++second) {
            if (ports[first] == ports[second]) return 0;
        }
    }
    return 1;
}

typedef struct core_machine_port_direction_requirement {
    type_unsigned_16 port;
    type_bool read;
    type_bool write;
} core_machine_port_direction_requirement;

static C_INT core_machine_controller_ports_are_available(
    const core_machine *machine,
    const core_machine_port_direction_requirement *requirements,
    STD_SIZE_T count)
{
    STD_SIZE_T index;

    if (machine == STD_NULL || requirements == STD_NULL) return 0;
    for (index = 0u; index < count; ++index) {
        if ((requirements[index].read && core_machine_port_has_read(
                &machine->executor_port, requirements[index].port)) ||
            (requirements[index].write && core_machine_port_has_write(
                &machine->executor_port, requirements[index].port))) {
            return 0;
        }
    }
    return 1;
}

type_status core_machine_configure_fdc(core_machine *machine,
    const core_machine_fdc_topology *topology)
{
    const core_machine_port_direction_requirement ports[] = {
        {topology == STD_NULL ? 0u : topology->config.dor_port,
            TYPE_FALSE, TYPE_TRUE},
        {topology == STD_NULL ? 0u : topology->config.status_port,
            TYPE_TRUE, TYPE_FALSE},
        {topology == STD_NULL ? 0u : topology->config.data_port,
            TYPE_TRUE, TYPE_TRUE},
        {topology == STD_NULL ? 0u : topology->config.direction_port,
            TYPE_TRUE, TYPE_FALSE},
        {topology == STD_NULL ? 0u : topology->config.control_port,
            TYPE_FALSE, TYPE_TRUE}
    };
    core_machine_port_provider_entry *port_checkpoint;
    type_status status;

    if (!core_machine_configuration_is_open(machine) || !machine->dma_configured ||
        machine->fdc_configured) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (!core_machine_fdc_topology_is_valid(topology) ||
        topology->dma_request.core_token != machine->fdc_dma_request.core_token ||
        topology->dma_request.channel != machine->fdc_dma_request.channel) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (!core_machine_controller_ports_are_available(machine, ports,
            sizeof(ports) / sizeof(ports[0]))) return TYPE_STATUS_INVALID_STATE;
    port_checkpoint = core_machine_port_registration_begin(&machine->executor_port);
    machine->fdc_topology = *topology;
    core_machine_fdc_connect(&machine->fdc, machine->fdc_topology.media_registry,
        &machine->fdc_topology.drives, &machine->fdc_topology.dma_request,
        core_machine_fdc_dma_request_assert,
        core_machine_fdc_dma_request_deassert, machine,
        &machine->shared_pic_master, &machine->shared_pic_slave,
        &machine->executor_port, &machine->fdc_topology.config);
    core_machine_fdc_initialize(&machine->fdc);
    status = core_machine_port_registration_status(&machine->executor_port);
    if (status != TYPE_STATUS_OK) {
        core_machine_port_rollback_registration(&machine->executor_port,
            port_checkpoint);
        core_machine_fdc_finalize(&machine->fdc);
        STD_MEMSET(&machine->fdc_topology, TYPE_ZERO_8,
            sizeof(machine->fdc_topology));
        return status;
    }
    machine->fdc_configured = TYPE_TRUE;
    return TYPE_STATUS_OK;
}

type_status core_machine_configure_hdc(core_machine *machine,
    const core_machine_hdc_topology *topology)
{
    const core_machine_port_provider *provider;
    const core_machine_port_direction_requirement ports[] = {
        {topology == STD_NULL ? 0u : topology->config.data_port,
            TYPE_TRUE, TYPE_TRUE},
        {topology == STD_NULL ? 0u : topology->config.error_features_port,
            TYPE_TRUE, TYPE_TRUE},
        {topology == STD_NULL ? 0u : topology->config.sector_count_port,
            TYPE_TRUE, TYPE_TRUE},
        {topology == STD_NULL ? 0u : topology->config.sector_number_port,
            TYPE_TRUE, TYPE_TRUE},
        {topology == STD_NULL ? 0u : topology->config.cylinder_low_port,
            TYPE_TRUE, TYPE_TRUE},
        {topology == STD_NULL ? 0u : topology->config.cylinder_high_port,
            TYPE_TRUE, TYPE_TRUE},
        {topology == STD_NULL ? 0u : topology->config.drive_head_port,
            TYPE_TRUE, TYPE_TRUE},
        {topology == STD_NULL ? 0u : topology->config.status_command_port,
            TYPE_TRUE, TYPE_TRUE},
        {topology == STD_NULL ? 0u :
            topology->config.alternate_status_device_control_port,
            TYPE_TRUE, TYPE_TRUE}
    };
    type_status status;
    core_machine_port_provider_entry *port_checkpoint;

    if (!core_machine_configuration_is_open(machine) || machine->hdc_configured) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (!core_machine_hdc_topology_is_valid(topology)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (!core_machine_controller_ports_are_available(machine, ports,
            sizeof(ports) / sizeof(ports[0]))) return TYPE_STATUS_INVALID_STATE;
    provider = core_machine_hdc_port_provider();
    if (provider == STD_NULL) return TYPE_STATUS_FAULT;
    port_checkpoint = core_machine_port_registration_begin(&machine->executor_port);
    machine->hdc_topology = *topology;
    core_machine_hdc_connect(&machine->hdc, machine->hdc_topology.media_registry,
        machine->hdc_topology.media_id, &machine->shared_pic_master,
        &machine->shared_pic_slave, &machine->hdc_topology.config);
    core_machine_hdc_initialize(&machine->hdc);
    status = core_machine_install_port_provider(machine,
        machine->hdc_topology.config.data_port,
        machine->hdc_topology.config.status_command_port, provider, &machine->hdc);
    if (status != TYPE_STATUS_OK) {
        core_machine_port_rollback_registration(&machine->executor_port,
            port_checkpoint);
        core_machine_hdc_finalize(&machine->hdc);
        STD_MEMSET(&machine->hdc_topology, TYPE_ZERO_8,
            sizeof(machine->hdc_topology));
        return status;
    }
    status = core_machine_install_port_provider(machine,
        machine->hdc_topology.config.alternate_status_device_control_port,
        machine->hdc_topology.config.alternate_status_device_control_port,
        provider, &machine->hdc);
    if (status != TYPE_STATUS_OK) {
        core_machine_port_rollback_registration(&machine->executor_port,
            port_checkpoint);
        core_machine_hdc_finalize(&machine->hdc);
        STD_MEMSET(&machine->hdc_topology, TYPE_ZERO_8,
            sizeof(machine->hdc_topology));
        return status;
    }
    machine->hdc_configured = TYPE_TRUE;
    return TYPE_STATUS_OK;
}

type_status core_machine_bind_execution_provider(core_machine *machine,
    const core_machine_execution_provider *provider, C_VOID *context)
{
    if (!core_machine_configuration_is_open(machine)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (provider != STD_NULL && provider->reset == STD_NULL &&
        provider->refresh == STD_NULL && provider->advance_time == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    machine->execution_provider = provider;
    machine->execution_provider_context = context;
    return TYPE_STATUS_OK;
}

type_status core_machine_freeze_execution_providers(core_machine *machine)
{
    if (!core_machine_configuration_is_open(machine)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    machine->execution_provider_frozen = 1;
    core_machine_memory_freeze_mappings(&machine->executor_memory);
    return TYPE_STATUS_OK;
}

type_status core_machine_get_cpu_state(
    const core_machine *machine,
    core_machine_cpu_state *out_state)
{
    if (machine == STD_NULL || out_state == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_RUNNING) {
        return TYPE_STATUS_INVALID_STATE;
    }
    out_state->cs = machine->executor_cpu.data.cs.selector;
    out_state->cs_base = machine->executor_cpu.data.cs.base;
    out_state->eip = machine->executor_cpu.data.eip;
    out_state->eflags = machine->executor_cpu.data.eflags;
    out_state->halted = machine->executor_cpu.data.flagHalt;
    return TYPE_STATUS_OK;
}

type_status core_machine_get_cpu_profile(
    const core_machine *machine, core_machine_cpu_profile *out_profile)
{
    if (machine == STD_NULL || out_profile == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_profile = machine->cpu_profile;
    return TYPE_STATUS_OK;
}

type_status core_machine_get_fpu_profile(
    const core_machine *machine, core_machine_fpu_profile *out_profile)
{
    if (machine == STD_NULL || out_profile == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_profile = machine->fpu.profile;
    return TYPE_STATUS_OK;
}

type_status core_machine_get_fpu_state(
    const core_machine *machine, core_machine_fpu_state *out_state)
{
    if (machine == STD_NULL || out_state == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    core_machine_fpu_get_state(&machine->fpu, out_state);
    return TYPE_STATUS_OK;
}

type_status core_machine_get_memory_bytes(
    const core_machine *machine, STD_SIZE_T *out_memory_bytes)
{
    if (machine == STD_NULL || out_memory_bytes == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_memory_bytes = machine->executor_memory.connect.installed_bytes;
    return TYPE_STATUS_OK;
}

type_status core_machine_get_elapsed_ticks(
    const core_machine *machine, type_unsigned_64 *out_elapsed_ticks)
{
    if (machine == STD_NULL || out_elapsed_ticks == STD_NULL ||
        machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_RUNNING) {
        return TYPE_STATUS_INVALID_STATE;
    }
    *out_elapsed_ticks = machine->elapsed_ticks;
    return TYPE_STATUS_OK;
}

type_status core_machine_get_timeline_observation(const core_machine *machine,
    core_machine_timeline_observation *out_observation)
{
    if (machine == STD_NULL || out_observation == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    out_observation->now = machine->timeline.now;
    out_observation->next_sequence = machine->timeline.next_sequence;
    out_observation->pending_events = core_machine_timeline_pending_count(
        &machine->timeline);
    return TYPE_STATUS_OK;
}

type_status core_machine_get_cpu_diagnostic(
    const core_machine *machine, core_machine_cpu_diagnostic *out_diagnostic)
{
    if (machine == STD_NULL || out_diagnostic == STD_NULL ||
        machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_RUNNING) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    core_machine_cpu_diagnostic_ordered_copy(&machine->cpu_diagnostic,
        out_diagnostic);
    return TYPE_STATUS_OK;
}

type_status core_machine_capture_observation(
    const core_machine *machine, core_machine_observation *out_observation)
{
    if (machine == STD_NULL || out_observation == STD_NULL ||
        machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_RUNNING) {
        return TYPE_STATUS_INVALID_STATE;
    }
    out_observation->lifecycle = machine->lifecycle;
    out_observation->elapsed_ticks = machine->elapsed_ticks;
    if (core_machine_get_cpu_state(machine, &out_observation->cpu) !=
            TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_STATE;
    }
    core_machine_cpu_diagnostic_ordered_copy(&machine->cpu_diagnostic,
        &out_observation->diagnostic);
    return TYPE_STATUS_OK;
}

static type_status core_machine_create_internal(
    const core_machine_config *config,
    core_machine **out_machine,
    core_machine_memory_test_allocation *test_allocation,
    core_machine_port_test_allocation *port_test_allocation)
{
    core_machine *machine;
    core_machine_port_provider_entry *port_checkpoint;
    STD_SIZE_T memory_bytes;
    if (config == STD_NULL || out_machine == STD_NULL ||
        !core_machine_valid_cpu_profile(
            core_machine_resolve_cpu_profile(config->cpu_profile)) ||
        !core_machine_valid_fpu_profile(config->fpu_profile) ||
        !core_machine_clock_plan_is_valid(&config->clock_plan)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    *out_machine = STD_NULL;
    memory_bytes = config->memory_bytes == 0u ?
        CORE_MACHINE_DEFAULT_MEMORY_BYTES : config->memory_bytes;

    machine = (core_machine *)STD_CALLOC(1u, sizeof(*machine));
    if (machine == STD_NULL) {
        return TYPE_STATUS_NO_MEMORY;
    }

    machine->lifecycle = CORE_MACHINE_INITIALIZED;
    machine->cpu_profile = core_machine_resolve_cpu_profile(config->cpu_profile);
    if (core_machine_timeline_initialize(&machine->timeline) != TYPE_STATUS_OK) {
        STD_FREE(machine);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    core_machine_resolve_instruction_timing(&machine->instruction_timing,
        &config->instruction_timing, config->ticks_per_instruction);
    machine->maximum_instruction_ticks = machine->cpu_profile ==
        CORE_MACHINE_CPU_PROFILE_80386 ? CORE_MACHINE_80386_SOURCE_MAXIMUM_TICKS :
        core_machine_instruction_maximum_ticks(&machine->instruction_timing);
    if (core_machine_clock_domain_initialize(&machine->dma_clock,
            &config->clock_plan.dma) != TYPE_STATUS_OK ||
        core_machine_clock_domain_initialize(&machine->pit_clock,
            &config->clock_plan.pit) != TYPE_STATUS_OK ||
        core_machine_clock_domain_initialize(&machine->rtc_clock,
            &config->clock_plan.rtc) != TYPE_STATUS_OK ||
        core_machine_clock_domain_initialize(&machine->vadp_clock,
            &config->clock_plan.vadp) != TYPE_STATUS_OK ||
        core_machine_clock_domain_initialize(&machine->kbc_clock,
            &config->clock_plan.kbc) != TYPE_STATUS_OK ||
        core_machine_clock_domain_initialize(&machine->provider_clock,
            &config->clock_plan.provider) != TYPE_STATUS_OK) {
        STD_FREE(machine);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    /* Zero is an explicit profile choice: without a calibrated guest-time
     * mapping, core-generated keyboard repeat must remain disabled. */
    machine->kbc_typematic_initial_ticks = config->kbc_typematic_initial_ticks;
    machine->kbc_typematic_repeat_ticks = config->kbc_typematic_repeat_ticks;
    machine->kbc_command_response_ticks = config->kbc_command_response_ticks;
    core_machine_fpu_initialize(&machine->fpu, config->fpu_profile);
    STD_ATOMIC_INIT(&machine->stop_requested, 0);
    core_machine_trace_initialize(machine);
    core_machine_transaction_initialize(&machine->transaction);
    core_machine_transaction_bind_trace(&machine->transaction,
        core_machine_transaction_trace, machine);
    core_machine_cpu_diagnostic_initialize(machine);

    core_machine_cpu_execution_context_initialize(&machine->executor_cpu_execution,
        &machine->executor_cpu, &machine->executor_cpu_instructions,
        &machine->executor_memory, &machine->executor_port);
    core_machine_cpu_execution_context_bind_profiles(
        &machine->executor_cpu_execution, machine->cpu_profile,
        machine->fpu.profile);
    core_machine_cpu_execution_context_bind_fpu(
        &machine->executor_cpu_execution, &machine->fpu);
    core_machine_cpu_execution_context_bind_transaction(
        &machine->executor_cpu_execution, &machine->transaction);
    core_machine_cpu_execution_context_bind_diagnostic_provider(
        &machine->executor_cpu_execution, &core_machine_cpu_diagnostic_provider,
        machine);
    core_machine_cpu_state_initialize(&machine->executor_cpu_execution);
    core_machine_port_initialize(&machine->executor_port);
    core_machine_port_set_test_allocation(&machine->executor_port,
        port_test_allocation);
    if (core_machine_bus_initialize(machine) != TYPE_STATUS_OK) {
        core_machine_destroy(machine);
        return TYPE_STATUS_NO_MEMORY;
    }
    if (core_machine_memory_initialize_for(&machine->executor_memory,
            memory_bytes, test_allocation) != TYPE_STATUS_OK) {
        core_machine_destroy(machine);
        return TYPE_STATUS_NO_MEMORY;
    }
    /* The 80386 reset vector is at physical FFFFFFF0.  The PC/AT firmware
     * window aliases its final 64 KiB at F0000 so every supported profile has
     * a deterministic reset fetch without requiring a per-fixture mapping. */
    if (core_machine_memory_register_mapping(&machine->executor_memory,
            0xffff0000u, 0x000f0000u, 0x00010000u) != TYPE_STATUS_OK) {
        core_machine_destroy(machine);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    port_checkpoint = core_machine_port_registration_begin(&machine->executor_port);
    core_machine_memory_register_ports(&machine->executor_memory,
        &machine->executor_port);
    core_machine_vadp_initialize(&machine->shared_vadp, &machine->executor_port);
    core_machine_kbc_initialize(&machine->shared_kbc, &machine->executor_port);
    core_machine_dma_initialize(&machine->shared_dma_latch,
        &machine->shared_dma_primary, &machine->shared_dma_secondary,
        &machine->executor_port);
    core_machine_pic_initialize(&machine->shared_pic_master,
        &machine->shared_pic_slave, &machine->executor_port);
    core_machine_cpu_execution_context_bind_pic(&machine->executor_cpu_execution,
        &machine->shared_pic_master, &machine->shared_pic_slave);
    core_machine_pic_irq_source_bind(&machine->shared_pit_irq0_source,
        &machine->shared_pic_master, &machine->shared_pic_slave, 0u);
    core_machine_pit_initialize(&machine->shared_pit, &machine->executor_port);
    core_machine_pit_set_output(&machine->shared_pit, 0,
        core_machine_pic_timer_output, &machine->shared_pit_irq0_source);
    core_machine_kbc_bind_core_services(&machine->shared_kbc,
        &machine->shared_pic_master, &machine->shared_pic_slave,
        &machine->executor_memory, &machine->executor_cpu_execution);
    core_machine_kbc_set_typematic_timing(&machine->shared_kbc,
        machine->kbc_typematic_initial_ticks,
        machine->kbc_typematic_repeat_ticks);
    core_machine_kbc_set_command_response_timing(&machine->shared_kbc,
        machine->kbc_command_response_ticks);
    core_machine_pit_set_output(&machine->shared_pit, 1, STD_NULL, STD_NULL);
    {
        type_status status = core_machine_port_registration_status(
            &machine->executor_port);

        if (status != TYPE_STATUS_OK) {
            core_machine_port_rollback_registration(&machine->executor_port,
                port_checkpoint);
            core_machine_destroy(machine);
            return status;
        }
    }

    *out_machine = machine;

    return TYPE_STATUS_OK;
}

type_status core_machine_create(const core_machine_config *config,
    core_machine **out_machine)
{
    return core_machine_create_internal(config, out_machine, STD_NULL, STD_NULL);
}

type_status core_machine_create_with_test_memory_allocation(
    const core_machine_config *config, core_machine **out_machine,
    core_machine_memory_test_allocation *test_allocation)
{
    return core_machine_create_internal(config, out_machine, test_allocation,
        STD_NULL);
}

type_status core_machine_create_with_test_port_allocation(
    const core_machine_config *config, core_machine **out_machine,
    core_machine_port_test_allocation *test_allocation)
{
    return core_machine_create_internal(config, out_machine, STD_NULL,
        test_allocation);
}

static type_status core_machine_cold_reset(core_machine *machine)
{
    core_machine_cpu_state_reset(&machine->executor_cpu_execution);
    core_machine_fpu_reset(&machine->fpu);
    core_machine_port_reset(&machine->executor_port);
    core_machine_memory_reset(&machine->executor_memory);
    core_machine_kbc_reset(&machine->shared_kbc);
    core_machine_dma_reset(&machine->shared_dma_latch,
        &machine->shared_dma_primary, &machine->shared_dma_secondary);
    if (machine->rtc_cmos_configured) core_machine_rtc_reset(&machine->shared_rtc);
    core_machine_fdc_reset(&machine->fdc);
    core_machine_hdc_reset(&machine->hdc);
    core_machine_pic_reset(&machine->shared_pic_master,
        &machine->shared_pic_slave);
    core_machine_pit_reset(&machine->shared_pit);
    core_machine_vadp_reset(&machine->shared_vadp);

    STD_ATOMIC_STORE(&machine->stop_requested, 0);
    machine->fault_detail = 0u;
    machine->elapsed_ticks = 0u;
    machine->source_repeat_active = TYPE_FALSE;
    core_machine_transaction_reset(&machine->transaction);
    core_machine_timeline_reset(&machine->timeline);
    core_machine_clock_domain_reset(&machine->dma_clock);
    core_machine_clock_domain_reset(&machine->pit_clock);
    core_machine_clock_domain_reset(&machine->rtc_clock);
    core_machine_clock_domain_reset(&machine->vadp_clock);
    core_machine_clock_domain_reset(&machine->kbc_clock);
    core_machine_clock_domain_reset(&machine->provider_clock);
    {
        core_machine_timeline_token first_arbitration;
        core_machine_timeline_token first_readiness;
        core_machine_timeline_token first_peripheral;

        if (core_machine_timeline_schedule(&machine->timeline, 1u,
                core_machine_arbitration_tick, machine,
                &first_arbitration) != TYPE_STATUS_OK) {
            return TYPE_STATUS_FAULT;
        }
        if (core_machine_timeline_schedule(&machine->timeline, 1u,
                core_machine_readiness_tick, machine,
                &first_readiness) != TYPE_STATUS_OK) {
            return TYPE_STATUS_FAULT;
        }
        if (core_machine_timeline_schedule(&machine->timeline, 1u,
                core_machine_peripheral_tick, machine,
                &first_peripheral) != TYPE_STATUS_OK) {
            return TYPE_STATUS_FAULT;
        }
    }
    machine->entry_plan_applied = TYPE_FALSE;
    core_machine_cpu_diagnostic_reset(machine);
    if (machine->execution_provider != STD_NULL &&
        machine->execution_provider->reset != STD_NULL) {
        machine->execution_provider->reset(machine->execution_provider_context);
    }
    if (machine->firmware_provider != STD_NULL &&
        core_machine_firmware_invoke(machine, 0,
            machine->firmware_provider->reset) != TYPE_STATUS_OK) {
        return TYPE_STATUS_FAULT;
    }
    machine->lifecycle = CORE_MACHINE_STOPPED;
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_RESET, 0u, 0u, 0u);
    return TYPE_STATUS_OK;
}

type_status core_machine_reconfigure_memory(core_machine *machine,
    STD_SIZE_T memory_bytes)
{
    type_native_unsigned index;

    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        !machine->execution_provider_frozen ||
        machine->lifecycle != CORE_MACHINE_STOPPED ||
        memory_bytes < CORE_MACHINE_MINIMUM_MEMORY_BYTES ||
        memory_bytes > CORE_MACHINE_MAXIMUM_MEMORY_BYTES) {
        return TYPE_STATUS_INVALID_STATE;
    }
    for (index = 0u; index < machine->executor_memory.connect.mapping_count;
            ++index) {
        const core_machine_memory_mapping *mapping =
            &machine->executor_memory.connect.mappings[index];
        if (mapping->backing_start > memory_bytes ||
            mapping->bytes > memory_bytes - mapping->backing_start) {
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
    }
    if (core_machine_memory_allocate_for(&machine->executor_memory,
            memory_bytes) != TYPE_STATUS_OK) {
        return TYPE_STATUS_NO_MEMORY;
    }
    return core_machine_cold_reset(machine);
}

type_status core_machine_reset(core_machine *machine)
{
    if (machine == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    if (!core_machine_mutable_operation_is_allowed(machine) ||
        !machine->execution_provider_frozen ||
        machine->lifecycle == CORE_MACHINE_RUNNING) {
        return TYPE_STATUS_INVALID_STATE;
    }

    return core_machine_cold_reset(machine);
}

type_status core_machine_get_lifecycle(
    const core_machine *machine,
    core_machine_lifecycle *out_lifecycle)
{
    if (machine == STD_NULL || out_lifecycle == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    *out_lifecycle = machine->lifecycle;
    return TYPE_STATUS_OK;
}

type_status core_machine_run(
    core_machine *machine,
    core_machine_run_budget budget,
    core_machine_run_result *result)
{
    if (machine == STD_NULL || result == STD_NULL ||
        !core_machine_mutable_operation_is_allowed(machine)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    result->reason = CORE_MACHINE_STOP_NONE;
    result->executed = 0u;
    result->ticks = 0u;
    result->elapsed_ticks = machine->elapsed_ticks;
    result->linear_pc = core_machine_linear_pc(machine);
    result->detail = 0u;

    if (machine->lifecycle == CORE_MACHINE_FAULTED) {
        result->reason = CORE_MACHINE_STOP_FAULT;
        result->detail = machine->fault_detail;
        return TYPE_STATUS_FAULT;
    }

    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return TYPE_STATUS_INVALID_STATE;
    }

    if (budget.instructions == 0u && budget.ticks == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    if (STD_ATOMIC_LOAD(&machine->stop_requested)) {
        if (core_machine_cold_reset(machine) != TYPE_STATUS_OK) {
            return TYPE_STATUS_FAULT;
        }
        result->reason = CORE_MACHINE_STOP_REQUESTED;
        result->linear_pc = core_machine_linear_pc(machine);
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_STOP, 0u, 0u,
                               (type_unsigned_32)result->reason);
        return TYPE_STATUS_OK;
    }

    machine->lifecycle = CORE_MACHINE_RUNNING;
    {
        while ((budget.instructions == 0u ||
                result->executed < budget.instructions) &&
               (budget.ticks == 0u || result->ticks < budget.ticks)) {
            if (machine->lifecycle == CORE_MACHINE_FAULTED) {
                result->reason = CORE_MACHINE_STOP_FAULT;
                result->linear_pc = core_machine_linear_pc(machine);
                result->detail = machine->fault_detail;
                return TYPE_STATUS_FAULT;
            }
            if (STD_ATOMIC_LOAD(&machine->stop_requested) ||
                core_machine_cpu_execution_consume_stop_request(
                    &machine->executor_cpu_execution)) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                if (core_machine_cold_reset(machine) != TYPE_STATUS_OK) {
                    return TYPE_STATUS_FAULT;
                }
                result->reason = CORE_MACHINE_STOP_REQUESTED;
                result->linear_pc = core_machine_linear_pc(machine);
                core_machine_trace_record(machine, CORE_MACHINE_TRACE_STOP, 0u,
                    0u, (type_unsigned_32)result->reason);
                return TYPE_STATUS_OK;
            }
            if (core_machine_cpu_execution_consume_reset_request(
                    &machine->executor_cpu_execution)) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                if (core_machine_cold_reset(machine) != TYPE_STATUS_OK) {
                    return TYPE_STATUS_FAULT;
                }
                result->reason = CORE_MACHINE_STOP_RESET_REQUESTED;
                result->linear_pc = core_machine_linear_pc(machine);
                return TYPE_STATUS_OK;
            }
            if (budget.ticks != 0u && machine->maximum_instruction_ticks >
                budget.ticks - result->ticks) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                result->reason = CORE_MACHINE_STOP_BUDGET;
                result->linear_pc = core_machine_linear_pc(machine);
                result->elapsed_ticks = machine->elapsed_ticks;
                return TYPE_STATUS_OK;
            }
            if (machine->execution_provider != STD_NULL &&
                machine->execution_provider->refresh != STD_NULL) {
                machine->execution_provider->refresh(
                    machine->execution_provider_context);
            }
            core_machine_kbc_refresh(&machine->shared_kbc);
            core_machine_cpu_execution_refresh(&machine->executor_cpu_execution);
            if (machine->lifecycle == CORE_MACHINE_FAULTED) {
                result->reason = CORE_MACHINE_STOP_FAULT;
                result->linear_pc = core_machine_linear_pc(machine);
                result->detail = machine->fault_detail;
                result->elapsed_ticks = machine->elapsed_ticks;
                return TYPE_STATUS_FAULT;
            }
            {
                type_unsigned_64 instruction_ticks;

                if (!core_machine_instruction_cost(machine, &instruction_ticks) ||
                    UINT64_MAX - result->ticks < instruction_ticks ||
                    UINT64_MAX - machine->elapsed_ticks < instruction_ticks) {
                    (C_VOID)core_machine_report_fault(machine, 0x54494d45u);
                    result->reason = CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return TYPE_STATUS_FAULT;
                }
                ++result->executed;
                result->ticks += instruction_ticks;
                machine->elapsed_ticks += instruction_ticks;
                result->elapsed_ticks = machine->elapsed_ticks;
                core_machine_trace_record(machine, CORE_MACHINE_TRACE_CPU_RETIRE,
                    core_machine_linear_pc(machine),
                    (type_unsigned_32)instruction_ticks, 0u);
                core_machine_advance_scheduler(machine, instruction_ticks);
            }
            if (machine->executor_cpu.data.flagHalt) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                result->reason = CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                result->linear_pc = core_machine_linear_pc(machine);
                return TYPE_STATUS_OK;
            }
        }
        machine->lifecycle = CORE_MACHINE_PAUSED;
        result->reason = CORE_MACHINE_STOP_BUDGET;
        result->linear_pc = core_machine_linear_pc(machine);
        if (machine->firmware_provider != STD_NULL &&
            machine->firmware_provider->after_run != STD_NULL &&
            core_machine_firmware_invoke(machine, 0,
                machine->firmware_provider->after_run) != TYPE_STATUS_OK) {
            (C_VOID)core_machine_report_fault(machine, 0x46575245u);
            result->reason = CORE_MACHINE_STOP_FAULT;
            result->detail = machine->fault_detail;
            return TYPE_STATUS_FAULT;
        }
        if (STD_ATOMIC_LOAD(&machine->stop_requested)) {
            result->reason = CORE_MACHINE_STOP_REQUESTED;
        }
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_RUN_BOUNDARY,
            result->linear_pc, (type_unsigned_32)result->executed,
            (type_unsigned_32)result->reason);
        return TYPE_STATUS_OK;
    }
}

type_status core_machine_request_stop(core_machine *machine)
{
    if (machine == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (!core_machine_mutable_operation_is_allowed(machine)) return TYPE_STATUS_INVALID_STATE;

    STD_ATOMIC_STORE(&machine->stop_requested, 1);
    return TYPE_STATUS_OK;
}

type_status core_machine_set_nmi_mask(core_machine *machine, C_INT masked)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    machine->executor_cpu.data.flagMaskNMI = masked ? TYPE_TRUE : TYPE_FALSE;
    return TYPE_STATUS_OK;
}

type_status core_machine_get_nmi_mask(const core_machine *machine,
    C_INT *out_masked)
{
    if (machine == STD_NULL || out_masked == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_masked = machine->executor_cpu.data.flagMaskNMI ? TYPE_TRUE : TYPE_FALSE;
    return TYPE_STATUS_OK;
}

type_status core_machine_keyboard_submit_scan_code(core_machine *machine,
    type_unsigned_8 scan_code)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_FAULTED) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return core_machine_kbc_submit_scan_code(&machine->shared_kbc, scan_code);
}

type_status core_machine_keyboard_submit_scan_codes(core_machine *machine,
    const type_unsigned_8 *scan_codes, STD_SIZE_T count)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_FAULTED) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return core_machine_kbc_submit_scan_codes(&machine->shared_kbc, scan_codes,
        count);
}

type_status core_machine_mouse_submit_relative(core_machine *machine,
    type_signed_16 delta_x, type_signed_16 delta_y, type_unsigned_8 buttons)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        (machine->lifecycle != CORE_MACHINE_RUNNING &&
        machine->lifecycle != CORE_MACHINE_PAUSED &&
        machine->lifecycle != CORE_MACHINE_STOPPED)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return core_machine_kbc_submit_aux_report(&machine->shared_kbc, delta_x,
        delta_y, buttons);
}

type_status core_machine_report_fault(
    core_machine *machine,
    type_unsigned_32 detail)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }

    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED &&
        machine->lifecycle != CORE_MACHINE_RUNNING) {
        return TYPE_STATUS_INVALID_STATE;
    }

    machine->fault_detail = detail;
    machine->lifecycle = CORE_MACHINE_FAULTED;
    core_machine_trace_record(machine, CORE_MACHINE_TRACE_FAULT, 0u, 0u, detail);
    return TYPE_STATUS_OK;
}

C_VOID core_machine_destroy(core_machine *machine)
{
    if (machine != STD_NULL) {
        machine->firmware_context.active = 0;
        machine->firmware_context.machine = STD_NULL;
        machine->firmware_provider = STD_NULL;
        machine->firmware_provider_context = STD_NULL;
        core_machine_hdc_finalize(&machine->hdc);
        core_machine_fdc_finalize(&machine->fdc);
        core_machine_dma_finalize(&machine->shared_dma_latch,
            &machine->shared_dma_primary, &machine->shared_dma_secondary);
        core_machine_rtc_finalize(&machine->shared_rtc);
        core_machine_kbc_finalize(&machine->shared_kbc);
        core_machine_pic_finalize(&machine->shared_pic_master,
            &machine->shared_pic_slave);
        core_machine_pit_finalize(&machine->shared_pit);
        core_machine_vadp_finalize(&machine->shared_vadp);
        core_machine_cpu_execution_finalize(&machine->executor_cpu_execution);
        core_machine_port_finalize(&machine->executor_port);
        core_machine_memory_finalize(&machine->executor_memory);
        for (STD_SIZE_T index = 0u; index < machine->immutable_rom_mapping_count;
                ++index) {
            STD_FREE(machine->immutable_rom_mappings[index].image);
        }
    }
    core_machine_trace_finalize(machine);
    core_machine_bus_finalize(machine);
    STD_FREE(machine);
}
