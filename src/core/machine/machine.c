#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/cpu_timing.h"

_Static_assert(CORE_MACHINE_TIMING_CAPABILITY_PRODUCT_DEBUG + 1u ==
    CORE_MACHINE_TIMING_CAPABILITY_COUNT,
    "timing capability count must match the frozen T433 universe");

static C_VOID core_machine_cpu_diagnostic_copy_point(
    core_machine_cpu_execution_point *point, const t_cpu *cpu,
    const t_cpuins *instructions, type_bool fault_origin)
{
    const t_cpu *source;

    if (point == STD_NULL || cpu == STD_NULL || instructions == STD_NULL) return;
    source = fault_origin ? &instructions->data.oldcpu : cpu;
    point->cs = source->data.cs.selector;
    point->cs_base = source->data.cs.base;
    point->eip = source->data.eip;
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
        &state->snapshot.recent[state->next_index], cpu, instructions, TYPE_FALSE);
    core_machine_retirement_observation_capture_instruction(machine, cpu, instructions);
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
    core_machine_cpu_diagnostic_copy_point(&fault->point, cpu, instructions, TYPE_TRUE);
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
    core_machine_cpu_diagnostic_copy_point(&exception->point, cpu, instructions, TYPE_TRUE);
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

static C_INT core_machine_retirement_qualification_contains(
    const core_machine *machine);

/* Both immediate and externally delayed successful retirements meet here.
 * CPU timing selection is complete before this seam; board-cycle time has
 * already been added by the caller and never enters cpu_timing.c. */
static C_INT core_machine_publish_successful_retirement(core_machine *machine)
{
    if (machine == STD_NULL) return 0;
    core_machine_retirement_observation_publish(machine,
        machine->cpu_retirement_source_ticks);
    return machine->retirement_time_contract != CORE_MACHINE_RETIREMENT_TIME_PHYSICAL ||
        (!machine->source_timing_unallocated &&
         core_machine_retirement_qualification_contains(machine));
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

static C_INT core_machine_instruction_has_lock_prefix(const t_cpuins_data *data,
    type_unsigned_32 prefixes)
{
    type_unsigned_32 index;

    if (data == STD_NULL) return 0;
    for (index = 0u; index < prefixes; ++index) {
        if (data->opcodes[index] == 0xf0u) return 1;
    }
    return 0;
}

static C_INT core_machine_80386_timing_has_source_prefixes(
    const t_cpuins_data *data, type_unsigned_32 prefixes)
{
    type_unsigned_32 index;

    if (data == STD_NULL || prefixes == 0u) return prefixes == 0u;
    for (index = 0u; index < prefixes; ++index) {
        switch (data->opcodes[index]) {
        case 0x26u: case 0x2eu: case 0x36u: case 0x3eu:
        case 0x64u: case 0x65u: case 0x66u: case 0x67u:
        case 0xf0u:
            break;
        default:
            return 0;
        }
    }
    return 1;
}

typedef enum core_machine_source_timing_form {
    CORE_MACHINE_SOURCE_TIMING_NOP,
    CORE_MACHINE_SOURCE_TIMING_CLC,
    CORE_MACHINE_SOURCE_TIMING_CLD,
    CORE_MACHINE_SOURCE_TIMING_SAL_REGISTER_ONE,
    CORE_MACHINE_SOURCE_TIMING_RCL_REGISTER_ONE_32,
    CORE_MACHINE_SOURCE_TIMING_RCL_REGISTER_CL_32,
    CORE_MACHINE_SOURCE_TIMING_CLI,
    CORE_MACHINE_SOURCE_TIMING_SAHF,
    CORE_MACHINE_SOURCE_TIMING_LAHF,
    CORE_MACHINE_SOURCE_TIMING_MOV_SREG_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_MOV_SREG_MEMORY,
    CORE_MACHINE_SOURCE_TIMING_MOV_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_MOV_RM_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_RM,
    CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_READ,
    CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_WRITE,
    CORE_MACHINE_SOURCE_TIMING_STRING_MOVS,
    CORE_MACHINE_SOURCE_TIMING_STRING_CMPS,
    CORE_MACHINE_SOURCE_TIMING_STRING_STOS,
    CORE_MACHINE_SOURCE_TIMING_STRING_LODS,
    CORE_MACHINE_SOURCE_TIMING_STRING_SCAS,
    CORE_MACHINE_SOURCE_TIMING_STRING_INS,
    CORE_MACHINE_SOURCE_TIMING_STRING_OUTS,
    CORE_MACHINE_SOURCE_TIMING_IN_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_IN_DX,
    CORE_MACHINE_SOURCE_TIMING_OUT_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_OUT_DX,
    CORE_MACHINE_SOURCE_TIMING_IN_IMMEDIATE_PROTECTED,
    CORE_MACHINE_SOURCE_TIMING_IN_IMMEDIATE_PERMISSION,
    CORE_MACHINE_SOURCE_TIMING_IN_DX_PROTECTED,
    CORE_MACHINE_SOURCE_TIMING_IN_DX_PERMISSION,
    CORE_MACHINE_SOURCE_TIMING_OUT_IMMEDIATE_PROTECTED,
    CORE_MACHINE_SOURCE_TIMING_OUT_IMMEDIATE_PERMISSION,
    CORE_MACHINE_SOURCE_TIMING_OUT_DX_PROTECTED,
    CORE_MACHINE_SOURCE_TIMING_OUT_DX_PERMISSION,
    CORE_MACHINE_SOURCE_TIMING_ALU_ADD_RM_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_ALU_ADD_REGISTER_RM,
    CORE_MACHINE_SOURCE_TIMING_ALU_SUB_RM_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_ALU_SUB_REGISTER_RM,
    CORE_MACHINE_SOURCE_TIMING_ALU_ACCUMULATOR_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_ALU_REGISTER_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_ALU_RM_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_CMP_RM_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_CMP_REGISTER_RM,
    CORE_MACHINE_SOURCE_TIMING_CMP_RM_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_TEST_RM_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_TEST_ACCUMULATOR_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_TEST_REGISTER_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_TEST_RM_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_XCHG_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_XCHG_RM_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_INC_DEC_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_INC_DEC_RM,
    CORE_MACHINE_SOURCE_TIMING_MOV_RM_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_LEA,
    CORE_MACHINE_SOURCE_TIMING_ADJUST_SIMPLE,
    CORE_MACHINE_SOURCE_TIMING_ADJUST_AAM,
    CORE_MACHINE_SOURCE_TIMING_ADJUST_AAD,
    CORE_MACHINE_SOURCE_TIMING_CONVERSION,
    CORE_MACHINE_SOURCE_TIMING_GROUP3_NOT,
    CORE_MACHINE_SOURCE_TIMING_GROUP3_NEG,
    CORE_MACHINE_SOURCE_TIMING_GROUP3_MUL,
    CORE_MACHINE_SOURCE_TIMING_GROUP3_IMUL,
    CORE_MACHINE_SOURCE_TIMING_GROUP3_DIV,
    CORE_MACHINE_SOURCE_TIMING_GROUP3_IDIV,
    CORE_MACHINE_SOURCE_TIMING_IMUL_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_SETCC,
    CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_DIRECT,
    CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_MEMORY,
    CORE_MACHINE_SOURCE_TIMING_CALL_FAR_DIRECT,
    CORE_MACHINE_SOURCE_TIMING_CALL_FAR_MEMORY,
    CORE_MACHINE_SOURCE_TIMING_JMP_DIRECT,
    CORE_MACHINE_SOURCE_TIMING_JMP_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_JMP_MEMORY,
    CORE_MACHINE_SOURCE_TIMING_JMP_FAR_DIRECT,
    CORE_MACHINE_SOURCE_TIMING_JMP_FAR_MEMORY,
    CORE_MACHINE_SOURCE_TIMING_JMP_FAR_MEMORY_PROTECTED,
    CORE_MACHINE_SOURCE_TIMING_RET_NEAR,
    CORE_MACHINE_SOURCE_TIMING_RET_NEAR_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_PUSH_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_PUSH_MEMORY,
    CORE_MACHINE_SOURCE_TIMING_PUSH_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_POP_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_POP_MEMORY,
    CORE_MACHINE_SOURCE_TIMING_PUSHA,
    CORE_MACHINE_SOURCE_TIMING_POPA,
    CORE_MACHINE_SOURCE_TIMING_PUSHF,
    CORE_MACHINE_SOURCE_TIMING_POPF,
    CORE_MACHINE_SOURCE_TIMING_ENTER_LEVEL_ZERO,
    CORE_MACHINE_SOURCE_TIMING_ENTER_LEVEL_ONE,
    CORE_MACHINE_SOURCE_TIMING_BOUND,
    CORE_MACHINE_SOURCE_TIMING_LEAVE,
    CORE_MACHINE_SOURCE_TIMING_HLT,
    CORE_MACHINE_SOURCE_TIMING_INT3,
    CORE_MACHINE_SOURCE_TIMING_INT_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_IRET,
    CORE_MACHINE_SOURCE_TIMING_8086_LOAD_POINTER,
    CORE_MACHINE_SOURCE_TIMING_8086_RET_FAR,
    CORE_MACHINE_SOURCE_TIMING_8086_GROUP2,
    CORE_MACHINE_SOURCE_TIMING_8086_FLAG,
    CORE_MACHINE_SOURCE_TIMING_8086_WAIT,
    CORE_MACHINE_SOURCE_TIMING_8086_ESC,
    CORE_MACHINE_SOURCE_TIMING_8086_JCC,
    CORE_MACHINE_SOURCE_TIMING_8086_LOOP,
    CORE_MACHINE_SOURCE_TIMING_8086_INTO,
    CORE_MACHINE_SOURCE_TIMING_8086_XLAT
} core_machine_source_timing_form;

typedef struct core_machine_source_timing_entry {
    core_machine_source_timing_form form;
    type_unsigned_8 ticks;
} core_machine_source_timing_entry;

typedef struct core_machine_source_repeat_timing_entry {
    core_machine_source_timing_form form;
    type_unsigned_8 primitive_ticks;
    type_unsigned_8 repeat_setup_ticks;
    type_unsigned_8 repeat_iteration_ticks;
} core_machine_source_repeat_timing_entry;

typedef struct core_machine_source_repeat_timing_contract {
    const core_machine_source_repeat_timing_entry *entries;
    STD_SIZE_T entry_count;
} core_machine_source_repeat_timing_contract;

/* Intel 8086 Family User's Manual, Tables 2-20 and 2-21.  The selected
 * memory rows receive the table's EA, segment-override, and odd-word
 * additions below; prefetch and bus availability remain outside this owner. */
static const core_machine_source_timing_entry
    core_machine_8086_source_timing_ledger[] = {
    { CORE_MACHINE_SOURCE_TIMING_NOP, 3u },
    { CORE_MACHINE_SOURCE_TIMING_CLC, 2u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_IMMEDIATE, 4u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_REGISTER, 2u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_RM_REGISTER, 9u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_RM, 8u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_READ, 10u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_WRITE, 10u },
    { CORE_MACHINE_SOURCE_TIMING_IN_IMMEDIATE, 10u },
    { CORE_MACHINE_SOURCE_TIMING_IN_DX, 8u },
    { CORE_MACHINE_SOURCE_TIMING_OUT_IMMEDIATE, 10u },
    { CORE_MACHINE_SOURCE_TIMING_OUT_DX, 8u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_DIRECT, 19u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_REGISTER, 16u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_MEMORY, 21u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_FAR_DIRECT, 28u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_FAR_MEMORY, 37u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_DIRECT, 15u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_REGISTER, 11u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_MEMORY, 18u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_FAR_DIRECT, 15u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_FAR_MEMORY, 24u },
    { CORE_MACHINE_SOURCE_TIMING_RET_NEAR, 8u },
    { CORE_MACHINE_SOURCE_TIMING_RET_NEAR_IMMEDIATE, 12u },
    { CORE_MACHINE_SOURCE_TIMING_PUSH_REGISTER, 11u },
    { CORE_MACHINE_SOURCE_TIMING_PUSH_MEMORY, 16u },
    { CORE_MACHINE_SOURCE_TIMING_POP_REGISTER, 8u },
    { CORE_MACHINE_SOURCE_TIMING_POP_MEMORY, 17u },
    { CORE_MACHINE_SOURCE_TIMING_PUSHF, 10u },
    { CORE_MACHINE_SOURCE_TIMING_POPF, 8u },
    { CORE_MACHINE_SOURCE_TIMING_HLT, 2u },
    { CORE_MACHINE_SOURCE_TIMING_INT3, 52u },
    { CORE_MACHINE_SOURCE_TIMING_INT_IMMEDIATE, 51u },
    { CORE_MACHINE_SOURCE_TIMING_IRET, 24u }
};

/* Intel iAPX 86/88/186/188 User's Manual, Table 2-9 gives the primary
 * 80186 instruction values.  Its prefetch/no-wait and even-word assumptions
 * remain an explicit later bus/cycle-exact transfer. */
static const core_machine_source_timing_entry
    core_machine_80186_source_timing_ledger[] = {
    { CORE_MACHINE_SOURCE_TIMING_NOP, 3u },
    { CORE_MACHINE_SOURCE_TIMING_CLC, 2u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_IMMEDIATE, 4u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_REGISTER, 2u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_RM_REGISTER, 12u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_RM, 9u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_READ, 9u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_WRITE, 8u },
    { CORE_MACHINE_SOURCE_TIMING_IN_IMMEDIATE, 10u },
    { CORE_MACHINE_SOURCE_TIMING_IN_DX, 8u },
    { CORE_MACHINE_SOURCE_TIMING_OUT_IMMEDIATE, 9u },
    { CORE_MACHINE_SOURCE_TIMING_OUT_DX, 7u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_DIRECT, 15u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_REGISTER, 13u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_MEMORY, 19u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_FAR_DIRECT, 23u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_FAR_MEMORY, 38u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_DIRECT, 13u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_REGISTER, 11u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_MEMORY, 17u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_FAR_DIRECT, 13u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_FAR_MEMORY, 26u },
    { CORE_MACHINE_SOURCE_TIMING_RET_NEAR, 16u },
    { CORE_MACHINE_SOURCE_TIMING_RET_NEAR_IMMEDIATE, 18u },
    { CORE_MACHINE_SOURCE_TIMING_PUSH_REGISTER, 10u },
    { CORE_MACHINE_SOURCE_TIMING_PUSH_MEMORY, 16u },
    { CORE_MACHINE_SOURCE_TIMING_PUSH_IMMEDIATE, 10u },
    { CORE_MACHINE_SOURCE_TIMING_POP_REGISTER, 10u },
    { CORE_MACHINE_SOURCE_TIMING_POP_MEMORY, 20u },
    { CORE_MACHINE_SOURCE_TIMING_PUSHA, 36u },
    { CORE_MACHINE_SOURCE_TIMING_POPA, 51u },
    { CORE_MACHINE_SOURCE_TIMING_PUSHF, 9u },
    { CORE_MACHINE_SOURCE_TIMING_POPF, 8u },
    { CORE_MACHINE_SOURCE_TIMING_ENTER_LEVEL_ZERO, 15u },
    { CORE_MACHINE_SOURCE_TIMING_ENTER_LEVEL_ONE, 25u },
    { CORE_MACHINE_SOURCE_TIMING_BOUND, 34u },
    { CORE_MACHINE_SOURCE_TIMING_LEAVE, 8u },
    { CORE_MACHINE_SOURCE_TIMING_HLT, 2u },
    { CORE_MACHINE_SOURCE_TIMING_INT3, 45u },
    { CORE_MACHINE_SOURCE_TIMING_INT_IMMEDIATE, 47u },
    { CORE_MACHINE_SOURCE_TIMING_IRET, 28u }
};

/* Intel 210498-005, Appendix B p. B-78: the per-instruction NOP row is three
 * clocks.  Chapter 3 p. 3-15 has a two-clock overview example; T361 S4
 * records that distinct context without making it a second runtime owner. */
static const core_machine_source_timing_entry
    core_machine_80286_source_timing_ledger[] = {
    { CORE_MACHINE_SOURCE_TIMING_NOP, 3u },
    { CORE_MACHINE_SOURCE_TIMING_CLC, 2u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_IMMEDIATE, 2u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_REGISTER, 2u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_RM_REGISTER, 3u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_RM, 5u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_READ, 5u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_WRITE, 3u },
    { CORE_MACHINE_SOURCE_TIMING_IN_IMMEDIATE, 5u },
    { CORE_MACHINE_SOURCE_TIMING_IN_DX, 5u },
    { CORE_MACHINE_SOURCE_TIMING_OUT_IMMEDIATE, 3u },
    { CORE_MACHINE_SOURCE_TIMING_OUT_DX, 3u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_DIRECT, 7u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_REGISTER, 7u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_MEMORY, 11u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_FAR_DIRECT, 13u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_FAR_MEMORY, 16u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_DIRECT, 7u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_REGISTER, 7u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_MEMORY, 11u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_FAR_DIRECT, 11u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_FAR_MEMORY, 15u },
    { CORE_MACHINE_SOURCE_TIMING_RET_NEAR, 11u },
    { CORE_MACHINE_SOURCE_TIMING_RET_NEAR_IMMEDIATE, 11u },
    { CORE_MACHINE_SOURCE_TIMING_PUSH_REGISTER, 3u },
    { CORE_MACHINE_SOURCE_TIMING_PUSH_MEMORY, 5u },
    { CORE_MACHINE_SOURCE_TIMING_PUSH_IMMEDIATE, 3u },
    { CORE_MACHINE_SOURCE_TIMING_POP_REGISTER, 5u },
    { CORE_MACHINE_SOURCE_TIMING_POP_MEMORY, 5u },
    { CORE_MACHINE_SOURCE_TIMING_PUSHA, 17u },
    { CORE_MACHINE_SOURCE_TIMING_POPA, 19u },
    { CORE_MACHINE_SOURCE_TIMING_PUSHF, 3u },
    { CORE_MACHINE_SOURCE_TIMING_POPF, 5u },
    { CORE_MACHINE_SOURCE_TIMING_ENTER_LEVEL_ZERO, 11u },
    { CORE_MACHINE_SOURCE_TIMING_ENTER_LEVEL_ONE, 15u },
    { CORE_MACHINE_SOURCE_TIMING_LEAVE, 8u },
    { CORE_MACHINE_SOURCE_TIMING_HLT, 2u },
    { CORE_MACHINE_SOURCE_TIMING_INT3, 23u },
    { CORE_MACHINE_SOURCE_TIMING_INT_IMMEDIATE, 23u },
    { CORE_MACHINE_SOURCE_TIMING_IRET, 17u }
};

/* Intel 80386 Programmer's Reference Manual selected timing rows.  These are
 * Core clocks under the manual's prefetched/no-wait/no-HOLD assumptions; they
 * are not device service, bus arbitration, or host-time values. */
static const core_machine_source_timing_entry
    core_machine_80386_source_timing_ledger[] = {
    { CORE_MACHINE_SOURCE_TIMING_NOP, 3u },
    { CORE_MACHINE_SOURCE_TIMING_CLC, 2u },
    { CORE_MACHINE_SOURCE_TIMING_CLD, 2u },
    { CORE_MACHINE_SOURCE_TIMING_SAL_REGISTER_ONE, 3u },
    { CORE_MACHINE_SOURCE_TIMING_RCL_REGISTER_ONE_32, 9u },
    { CORE_MACHINE_SOURCE_TIMING_RCL_REGISTER_CL_32, 9u },
    { CORE_MACHINE_SOURCE_TIMING_CLI, 3u },
    { CORE_MACHINE_SOURCE_TIMING_SAHF, 3u },
    { CORE_MACHINE_SOURCE_TIMING_LAHF, 2u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_SREG_REGISTER, 2u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_SREG_MEMORY, 5u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_IMMEDIATE, 2u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_REGISTER, 2u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_RM_REGISTER, 2u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_RM, 4u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_READ, 4u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_WRITE, 2u },
    { CORE_MACHINE_SOURCE_TIMING_IN_IMMEDIATE, 12u },
    { CORE_MACHINE_SOURCE_TIMING_IN_DX, 13u },
    { CORE_MACHINE_SOURCE_TIMING_OUT_IMMEDIATE, 10u },
    { CORE_MACHINE_SOURCE_TIMING_OUT_DX, 11u },
    { CORE_MACHINE_SOURCE_TIMING_IN_IMMEDIATE_PROTECTED, 6u },
    { CORE_MACHINE_SOURCE_TIMING_IN_IMMEDIATE_PERMISSION, 26u },
    { CORE_MACHINE_SOURCE_TIMING_IN_DX_PROTECTED, 7u },
    { CORE_MACHINE_SOURCE_TIMING_IN_DX_PERMISSION, 27u },
    { CORE_MACHINE_SOURCE_TIMING_OUT_IMMEDIATE_PROTECTED, 4u },
    { CORE_MACHINE_SOURCE_TIMING_OUT_IMMEDIATE_PERMISSION, 24u },
    { CORE_MACHINE_SOURCE_TIMING_OUT_DX_PROTECTED, 5u },
    { CORE_MACHINE_SOURCE_TIMING_OUT_DX_PERMISSION, 25u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_DIRECT, 7u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_REGISTER, 7u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_MEMORY, 10u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_FAR_DIRECT, 17u },
    { CORE_MACHINE_SOURCE_TIMING_CALL_FAR_MEMORY, 22u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_DIRECT, 7u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_REGISTER, 7u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_MEMORY, 10u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_FAR_DIRECT, 12u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_FAR_MEMORY, 43u },
    { CORE_MACHINE_SOURCE_TIMING_JMP_FAR_MEMORY_PROTECTED, 31u },
    { CORE_MACHINE_SOURCE_TIMING_RET_NEAR, 10u },
    { CORE_MACHINE_SOURCE_TIMING_RET_NEAR_IMMEDIATE, 10u },
    { CORE_MACHINE_SOURCE_TIMING_PUSH_REGISTER, 2u },
    { CORE_MACHINE_SOURCE_TIMING_PUSH_MEMORY, 5u },
    { CORE_MACHINE_SOURCE_TIMING_PUSH_IMMEDIATE, 2u },
    { CORE_MACHINE_SOURCE_TIMING_POP_REGISTER, 4u },
    { CORE_MACHINE_SOURCE_TIMING_POP_MEMORY, 5u },
    { CORE_MACHINE_SOURCE_TIMING_PUSHA, 18u },
    { CORE_MACHINE_SOURCE_TIMING_POPA, 24u },
    { CORE_MACHINE_SOURCE_TIMING_PUSHF, 4u },
    { CORE_MACHINE_SOURCE_TIMING_POPF, 5u },
    { CORE_MACHINE_SOURCE_TIMING_ENTER_LEVEL_ZERO, 10u },
    { CORE_MACHINE_SOURCE_TIMING_ENTER_LEVEL_ONE, 12u },
    { CORE_MACHINE_SOURCE_TIMING_LEAVE, 4u },
    { CORE_MACHINE_SOURCE_TIMING_HLT, 5u },
    { CORE_MACHINE_SOURCE_TIMING_INT3, 33u },
    { CORE_MACHINE_SOURCE_TIMING_INT_IMMEDIATE, 37u },
    { CORE_MACHINE_SOURCE_TIMING_IRET, 22u }
};

/* The string rows are intentionally distinct from the ordinary instruction
 * ledger.  A REP string retires one primitive at a time in this executor, so
 * the source formula is split into its one-time setup and per-primitive parts
 * at the sole post-refresh timing publisher below. */
static const core_machine_source_repeat_timing_entry
    core_machine_8086_source_repeat_timing_ledger[] = {
    { CORE_MACHINE_SOURCE_TIMING_STRING_MOVS, 18u, 9u, 17u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_CMPS, 22u, 9u, 22u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_STOS, 11u, 9u, 10u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_LODS, 12u, 9u, 13u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_SCAS, 15u, 9u, 15u }
};

static const core_machine_source_repeat_timing_entry
    core_machine_80186_source_repeat_timing_ledger[] = {
    { CORE_MACHINE_SOURCE_TIMING_STRING_MOVS, 14u, 8u, 8u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_CMPS, 22u, 5u, 22u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_STOS, 10u, 6u, 9u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_LODS, 12u, 6u, 11u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_SCAS, 15u, 5u, 15u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_INS, 14u, 8u, 8u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_OUTS, 14u, 8u, 8u }
};

static const core_machine_source_repeat_timing_entry
    core_machine_80286_source_repeat_timing_ledger[] = {
    { CORE_MACHINE_SOURCE_TIMING_STRING_MOVS, 5u, 5u, 4u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_CMPS, 9u, 5u, 9u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_STOS, 3u, 4u, 3u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_LODS, 5u, 0u, 0u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_SCAS, 8u, 5u, 8u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_INS, 5u, 5u, 4u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_OUTS, 5u, 5u, 4u }
};

static const core_machine_source_repeat_timing_entry
    core_machine_80386_source_repeat_timing_ledger[] = {
    { CORE_MACHINE_SOURCE_TIMING_STRING_MOVS, 7u, 5u, 4u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_CMPS, 9u, 5u, 9u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_STOS, 4u, 5u, 5u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_LODS, 5u, 5u, 6u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_SCAS, 7u, 5u, 8u }
};

static const core_machine_source_repeat_timing_contract
    core_machine_8086_source_repeat_timing_contract = {
    core_machine_8086_source_repeat_timing_ledger,
    sizeof(core_machine_8086_source_repeat_timing_ledger) /
        sizeof(core_machine_8086_source_repeat_timing_ledger[0])
};

static const core_machine_source_repeat_timing_contract
    core_machine_80186_source_repeat_timing_contract = {
    core_machine_80186_source_repeat_timing_ledger,
    sizeof(core_machine_80186_source_repeat_timing_ledger) /
        sizeof(core_machine_80186_source_repeat_timing_ledger[0])
};

static const core_machine_source_repeat_timing_contract
    core_machine_80286_source_repeat_timing_contract = {
    core_machine_80286_source_repeat_timing_ledger,
    sizeof(core_machine_80286_source_repeat_timing_ledger) /
        sizeof(core_machine_80286_source_repeat_timing_ledger[0])
};

static const core_machine_source_repeat_timing_contract
    core_machine_80386_source_repeat_timing_contract = {
    core_machine_80386_source_repeat_timing_ledger,
    sizeof(core_machine_80386_source_repeat_timing_ledger) /
        sizeof(core_machine_80386_source_repeat_timing_ledger[0])
};

#define CORE_MACHINE_80386_JCC_NOT_TAKEN_TICKS 3u
#define CORE_MACHINE_80386_JCC_TAKEN_TICKS 7u
#define CORE_MACHINE_SOURCE_UNALLOCATED_TICKS 1u

static C_VOID core_machine_source_timing_mark_unallocated(core_machine *machine,
    type_unsigned_64 *out_ticks)
{
    if (machine != STD_NULL) machine->source_timing_unallocated = TYPE_TRUE;
    if (out_ticks != STD_NULL) *out_ticks = CORE_MACHINE_SOURCE_UNALLOCATED_TICKS;
}
#define CORE_MACHINE_80386_SOURCE_MAXIMUM_TICKS 106u
#define CORE_MACHINE_8086_JCC_NOT_TAKEN_TICKS 4u
#define CORE_MACHINE_8086_JCC_TAKEN_TICKS 16u
#define CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS 2u
#define CORE_MACHINE_8086_ODD_WORD_TICKS 4u
#define CORE_MACHINE_8086_SOURCE_MAXIMUM_TICKS 83u
#define CORE_MACHINE_80186_JCC_NOT_TAKEN_TICKS 4u
#define CORE_MACHINE_80186_JCC_TAKEN_TICKS 13u
#define CORE_MACHINE_80186_SOURCE_MAXIMUM_TICKS 27u
#define CORE_MACHINE_80286_JCC_NOT_TAKEN_TICKS 3u
#define CORE_MACHINE_80286_JCC_TAKEN_TICKS 7u
#define CORE_MACHINE_80286_BASE_INDEX_DISPLACEMENT_TICKS 1u
#define CORE_MACHINE_80286_ODD_WORD_TICKS 2u
#define CORE_MACHINE_80286_SOURCE_MAXIMUM_TICKS 28u

typedef struct core_machine_legacy_source_timing_contract {
    const core_machine_source_timing_entry *ledger;
    STD_SIZE_T ledger_entries;
    type_unsigned_64 jcc_not_taken_ticks;
    type_unsigned_64 jcc_taken_ticks;
} core_machine_legacy_source_timing_contract;

static const core_machine_legacy_source_timing_contract
    core_machine_8086_source_timing_contract = {
    core_machine_8086_source_timing_ledger,
    sizeof(core_machine_8086_source_timing_ledger) /
        sizeof(core_machine_8086_source_timing_ledger[0]),
    CORE_MACHINE_8086_JCC_NOT_TAKEN_TICKS,
    CORE_MACHINE_8086_JCC_TAKEN_TICKS
};

static const core_machine_legacy_source_timing_contract
    core_machine_80186_source_timing_contract = {
    core_machine_80186_source_timing_ledger,
    sizeof(core_machine_80186_source_timing_ledger) /
        sizeof(core_machine_80186_source_timing_ledger[0]),
    CORE_MACHINE_80186_JCC_NOT_TAKEN_TICKS,
    CORE_MACHINE_80186_JCC_TAKEN_TICKS
};

static type_unsigned_64 core_machine_source_timing_lookup(core_machine *machine,
    const core_machine_source_timing_entry *ledger, STD_SIZE_T ledger_entries,
    core_machine_source_timing_form form)
{
    STD_SIZE_T index;

    for (index = 0u; index < ledger_entries; ++index) {
        if (ledger[index].form == form) {
            if (machine != STD_NULL) machine->source_timing_form_id = (type_unsigned_32)form;
            return ledger[index].ticks;
        }
    }
    if (machine != STD_NULL) machine->source_timing_unallocated = TYPE_TRUE;
    return CORE_MACHINE_SOURCE_UNALLOCATED_TICKS;
}

static type_unsigned_64 core_machine_80386_source_timing_lookup(
    core_machine *machine, core_machine_source_timing_form form)
{
    return core_machine_source_timing_lookup(machine,
        core_machine_80386_source_timing_ledger,
        sizeof(core_machine_80386_source_timing_ledger) /
        sizeof(core_machine_80386_source_timing_ledger[0]), form);
}

static type_unsigned_64 core_machine_80286_source_timing_lookup(
    core_machine *machine, core_machine_source_timing_form form);
static const core_machine_source_repeat_timing_entry
    *core_machine_source_repeat_timing_lookup(
        const core_machine_source_repeat_timing_contract *contract,
        core_machine_source_timing_form form);
static C_INT core_machine_source_timing_string_form(type_unsigned_8 opcode,
    core_machine_source_timing_form *out_form);
static C_INT core_machine_source_timing_string_repeat_is_defined(
    core_machine_source_timing_form form, t_cpuins_data_prefix_rep prefix);
static type_unsigned_64 core_machine_source_timing_repeat_string(
    core_machine *machine, const t_cpuins_data *data,
    const core_machine_source_repeat_timing_entry *entry);

static C_INT core_machine_80386_timing_uses_permission_map(
    const t_cpuins_data *data)
{
    type_unsigned_32 iopl;

    if ((data->oldcpu.data.cr0 & VCPU_CR0_PE) == 0u) return 0;
    iopl = (data->oldcpu.data.eflags & VCPU_EFLAGS_IOPL) >> 12u;
    return (data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ||
        data->oldcpu.data.cs.dpl > iopl;
}

static type_unsigned_64 core_machine_80386_source_timing_port_cost(
    core_machine *machine, const t_cpuins_data *data, core_machine_source_timing_form real_form,
    core_machine_source_timing_form protected_form,
    core_machine_source_timing_form permission_form)
{
    if ((data->oldcpu.data.cr0 & VCPU_CR0_PE) == 0u) {
        return core_machine_80386_source_timing_lookup(machine, real_form);
    }
    return core_machine_80386_source_timing_lookup(machine,
        core_machine_80386_timing_uses_permission_map(data) ?
        permission_form : protected_form);
}

static const core_machine_source_repeat_timing_contract
    *core_machine_source_repeat_timing_contract_for_profile(
        core_machine_cpu_profile profile)
{
    switch (profile) {
    case CORE_MACHINE_CPU_PROFILE_8086:
        return &core_machine_8086_source_repeat_timing_contract;
    case CORE_MACHINE_CPU_PROFILE_80186:
        return &core_machine_80186_source_repeat_timing_contract;
    case CORE_MACHINE_CPU_PROFILE_80286:
        return &core_machine_80286_source_repeat_timing_contract;
    case CORE_MACHINE_CPU_PROFILE_80386:
        return &core_machine_80386_source_repeat_timing_contract;
    default:
        return STD_NULL;
    }
}

static C_INT core_machine_80386_source_string_port_entry(
    const t_cpuins_data *data, core_machine_source_timing_form form,
    core_machine_source_repeat_timing_entry *out_entry)
{
    C_INT permission;

    if (data == STD_NULL || out_entry == STD_NULL) return 0;
    permission = core_machine_80386_timing_uses_permission_map(data);
    out_entry->form = form;
    if (form == CORE_MACHINE_SOURCE_TIMING_STRING_INS) {
        out_entry->primitive_ticks = (data->oldcpu.data.cr0 & VCPU_CR0_PE) == 0u ?
            15u : permission ? 29u : 9u;
        out_entry->repeat_setup_ticks =
            (data->oldcpu.data.cr0 & VCPU_CR0_PE) == 0u ? 13u :
            permission ? 27u : 7u;
        out_entry->repeat_iteration_ticks = 6u;
        return 1;
    }
    if (form == CORE_MACHINE_SOURCE_TIMING_STRING_OUTS) {
        out_entry->primitive_ticks = (data->oldcpu.data.cr0 & VCPU_CR0_PE) == 0u ?
            12u : permission ? 26u : 6u;
        out_entry->repeat_setup_ticks =
            (data->oldcpu.data.cr0 & VCPU_CR0_PE) == 0u ? 12u :
            permission ? 26u : 6u;
        out_entry->repeat_iteration_ticks = 5u;
        return 1;
    }
    return 0;
}

C_INT core_machine_string_io_source_instruction_cost(
    core_machine *machine, type_unsigned_64 *out_ticks)
{
    const t_cpuins_data *data;
    const core_machine_source_repeat_timing_contract *contract;
    const core_machine_source_repeat_timing_entry *entry;
    core_machine_source_repeat_timing_entry port_entry;
    core_machine_source_timing_form form;
    type_unsigned_32 prefixes;
    type_unsigned_8 opcode = 0u;

    if (machine == STD_NULL || out_ticks == STD_NULL) return 0;
    data = &machine->executor_cpu_instructions.data;
    prefixes = core_machine_instruction_prefix_count(data);
    if (prefixes >= data->oplen) return 0;
    opcode = data->opcodes[prefixes];
    if (core_machine_source_timing_string_form(opcode, &form)) {
        if (!core_machine_source_timing_string_repeat_is_defined(form,
                data->prefix_rep)) {
            machine->source_repeat_active = TYPE_FALSE;
            return 0;
        }
        machine->source_timing_form_id = (type_unsigned_32)form;
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 &&
            core_machine_80386_source_string_port_entry(data, form,
                &port_entry)) {
            *out_ticks = core_machine_source_timing_repeat_string(machine, data,
                &port_entry);
            return 1;
        }
        contract = core_machine_source_repeat_timing_contract_for_profile(
            machine->cpu_profile);
        entry = core_machine_source_repeat_timing_lookup(contract, form);
        if (entry == STD_NULL || (data->prefix_rep != PREFIX_REP_NONE &&
            entry->repeat_iteration_ticks == 0u)) {
            machine->source_repeat_active = TYPE_FALSE;
            return 0;
        }
        *out_ticks = core_machine_source_timing_repeat_string(machine, data,
            entry);
        return 1;
    }
    if (data->prefix_rep != PREFIX_REP_NONE) {
        machine->source_repeat_active = TYPE_FALSE;
        return 0;
    }
    machine->source_repeat_active = TYPE_FALSE;
    switch (opcode) {
    case 0xe4u: case 0xe5u:
        form = CORE_MACHINE_SOURCE_TIMING_IN_IMMEDIATE;
        break;
    case 0xecu: case 0xedu:
        form = CORE_MACHINE_SOURCE_TIMING_IN_DX;
        break;
    case 0xe6u: case 0xe7u:
        form = CORE_MACHINE_SOURCE_TIMING_OUT_IMMEDIATE;
        break;
    case 0xeeu: case 0xefu:
        form = CORE_MACHINE_SOURCE_TIMING_OUT_DX;
        break;
    default:
        return 0;
    }
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) {
        switch (form) {
        case CORE_MACHINE_SOURCE_TIMING_IN_IMMEDIATE:
            *out_ticks = core_machine_80386_source_timing_port_cost(machine, data, form,
                CORE_MACHINE_SOURCE_TIMING_IN_IMMEDIATE_PROTECTED,
                CORE_MACHINE_SOURCE_TIMING_IN_IMMEDIATE_PERMISSION);
            break;
        case CORE_MACHINE_SOURCE_TIMING_IN_DX:
            *out_ticks = core_machine_80386_source_timing_port_cost(machine, data, form,
                CORE_MACHINE_SOURCE_TIMING_IN_DX_PROTECTED,
                CORE_MACHINE_SOURCE_TIMING_IN_DX_PERMISSION);
            break;
        case CORE_MACHINE_SOURCE_TIMING_OUT_IMMEDIATE:
            *out_ticks = core_machine_80386_source_timing_port_cost(machine, data, form,
                CORE_MACHINE_SOURCE_TIMING_OUT_IMMEDIATE_PROTECTED,
                CORE_MACHINE_SOURCE_TIMING_OUT_IMMEDIATE_PERMISSION);
            break;
        case CORE_MACHINE_SOURCE_TIMING_OUT_DX:
            *out_ticks = core_machine_80386_source_timing_port_cost(machine, data, form,
                CORE_MACHINE_SOURCE_TIMING_OUT_DX_PROTECTED,
                CORE_MACHINE_SOURCE_TIMING_OUT_DX_PERMISSION);
            break;
        default:
            return 0;
        }
        return 1;
    }
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8086) {
        *out_ticks = core_machine_source_timing_lookup(machine,
            core_machine_8086_source_timing_ledger,
            sizeof(core_machine_8086_source_timing_ledger) /
                sizeof(core_machine_8086_source_timing_ledger[0]), form);
        return 1;
    }
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186) {
        *out_ticks = core_machine_source_timing_lookup(machine,
            core_machine_80186_source_timing_ledger,
            sizeof(core_machine_80186_source_timing_ledger) /
                sizeof(core_machine_80186_source_timing_ledger[0]), form);
        return 1;
    }
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286) {
        *out_ticks = core_machine_80286_source_timing_lookup(machine, form);
        return 1;
    }
    return 0;
}

static C_INT core_machine_source_timing_modrm_is_memory(
    const t_cpuins_data *data, type_unsigned_32 opcode_index)
{
    return opcode_index + 1u < data->oplen &&
        (data->opcodes[opcode_index + 1u] >> 6u) != 3u;
}

static const core_machine_source_repeat_timing_entry
    *core_machine_source_repeat_timing_lookup(
        const core_machine_source_repeat_timing_contract *contract,
        core_machine_source_timing_form form)
{
    STD_SIZE_T index;

    if (contract == STD_NULL) return STD_NULL;
    for (index = 0u; index < contract->entry_count; ++index) {
        if (contract->entries[index].form == form) return &contract->entries[index];
    }
    return STD_NULL;
}

static C_INT core_machine_source_timing_string_form(type_unsigned_8 opcode,
    core_machine_source_timing_form *out_form)
{
    if (out_form == STD_NULL) return 0;
    switch (opcode & 0xfeu) {
    case 0xa4u:
        *out_form = CORE_MACHINE_SOURCE_TIMING_STRING_MOVS;
        return 1;
    case 0xa6u:
        *out_form = CORE_MACHINE_SOURCE_TIMING_STRING_CMPS;
        return 1;
    case 0xaau:
        *out_form = CORE_MACHINE_SOURCE_TIMING_STRING_STOS;
        return 1;
    case 0xacu:
        *out_form = CORE_MACHINE_SOURCE_TIMING_STRING_LODS;
        return 1;
    case 0xaeu:
        *out_form = CORE_MACHINE_SOURCE_TIMING_STRING_SCAS;
        return 1;
    case 0x6cu:
        *out_form = CORE_MACHINE_SOURCE_TIMING_STRING_INS;
        return 1;
    case 0x6eu:
        *out_form = CORE_MACHINE_SOURCE_TIMING_STRING_OUTS;
        return 1;
    default:
        return 0;
    }
}

static C_INT core_machine_source_timing_string_repeat_is_defined(
    core_machine_source_timing_form form, t_cpuins_data_prefix_rep prefix)
{
    if (prefix == PREFIX_REP_NONE) return 1;
    if (form == CORE_MACHINE_SOURCE_TIMING_STRING_CMPS ||
        form == CORE_MACHINE_SOURCE_TIMING_STRING_SCAS) {
        return prefix == PREFIX_REP_REPZ || prefix == PREFIX_REP_REPZNZ;
    }
    return prefix == PREFIX_REP_REPZ;
}

static type_unsigned_64 core_machine_8086_timing_string_modifiers(
    const core_machine *machine, const t_cpuins_data *data,
    core_machine_source_timing_form form)
{
    type_unsigned_32 index;
    type_unsigned_8 opcode = 0u;
    type_unsigned_64 modifiers = 0u;
    C_INT segment_override = 0;
    C_INT word;
    C_INT source_transfer;
    C_INT destination_transfer;

    if (machine == STD_NULL || data == STD_NULL ||
        (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8086 &&
         machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186)) return 0u;
    for (index = 0u; index < data->oplen; ++index) {
        switch (data->opcodes[index]) {
        case 0x26u: case 0x2eu: case 0x36u: case 0x3eu:
            segment_override = 1;
            break;
        case 0xf0u: case 0xf2u: case 0xf3u:
            break;
        default:
            opcode = data->opcodes[index];
            index = data->oplen;
            break;
        }
    }
    if (segment_override && (form == CORE_MACHINE_SOURCE_TIMING_STRING_MOVS ||
        form == CORE_MACHINE_SOURCE_TIMING_STRING_CMPS ||
        form == CORE_MACHINE_SOURCE_TIMING_STRING_LODS)) {
        modifiers += CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS;
    }
    word = (opcode & 1u) != 0u;
    if (!word || machine->source_timing_repeat_phase ==
        CORE_MACHINE_RETIREMENT_REPEAT_ZERO_COUNT) return modifiers;
    source_transfer = form == CORE_MACHINE_SOURCE_TIMING_STRING_MOVS ||
        form == CORE_MACHINE_SOURCE_TIMING_STRING_CMPS ||
        form == CORE_MACHINE_SOURCE_TIMING_STRING_LODS;
    destination_transfer = form == CORE_MACHINE_SOURCE_TIMING_STRING_MOVS ||
        form == CORE_MACHINE_SOURCE_TIMING_STRING_CMPS ||
        form == CORE_MACHINE_SOURCE_TIMING_STRING_STOS ||
        form == CORE_MACHINE_SOURCE_TIMING_STRING_SCAS;
    if (source_transfer && (data->oldcpu.data.si & 1u) != 0u) {
        modifiers += CORE_MACHINE_8086_ODD_WORD_TICKS;
    }
    if (destination_transfer && (data->oldcpu.data.di & 1u) != 0u) {
        modifiers += CORE_MACHINE_8086_ODD_WORD_TICKS;
    }
    return modifiers;
}

static type_unsigned_64 core_machine_source_timing_repeat_string(
    core_machine *machine, const t_cpuins_data *data,
    const core_machine_source_repeat_timing_entry *entry)
{
    type_bool operand_size;
    type_bool address_size;
    type_unsigned_32 count;
    C_INT continuing;
    type_unsigned_64 ticks;

    if (machine == STD_NULL || data == STD_NULL || entry == STD_NULL) return 0u;
    operand_size = data->oldcpu.data.cs.seg.exec.defsize !=
        (data->prefix_oprsize != TYPE_FALSE);
    address_size = data->oldcpu.data.cs.seg.exec.defsize !=
        (data->prefix_addrsize != TYPE_FALSE);
    if (data->prefix_rep == PREFIX_REP_NONE) {
        machine->source_timing_repeat_phase =
            CORE_MACHINE_RETIREMENT_REPEAT_PRIMITIVE;
        machine->source_repeat_active = TYPE_FALSE;
        return entry->primitive_ticks + core_machine_8086_timing_string_modifiers(
            machine, data, entry->form);
    }
    count = address_size ? data->oldcpu.data.ecx : data->oldcpu.data.cx;
    continuing = machine->source_repeat_active &&
        machine->source_repeat_cs == data->oldcpu.data.cs.selector &&
        machine->source_repeat_eip == data->oldcpu.data.eip &&
        machine->source_repeat_opcode == data->opcodes[
            core_machine_instruction_prefix_count(data)] &&
        machine->source_repeat_prefix == (type_unsigned_8)data->prefix_rep &&
        machine->source_repeat_operand_size == operand_size &&
        machine->source_repeat_address_size == address_size;
    machine->source_timing_repeat_phase = count == 0u ?
        CORE_MACHINE_RETIREMENT_REPEAT_ZERO_COUNT : continuing ?
        CORE_MACHINE_RETIREMENT_REPEAT_CONTINUATION :
        CORE_MACHINE_RETIREMENT_REPEAT_FIRST;
    ticks = count == 0u ? entry->repeat_setup_ticks : continuing ?
        entry->repeat_iteration_ticks : entry->repeat_setup_ticks +
        entry->repeat_iteration_ticks;
    machine->source_repeat_active = count != 0u &&
        machine->executor_cpu.data.eip == data->oldcpu.data.eip;
    if (machine->source_repeat_active) {
        machine->source_repeat_cs = data->oldcpu.data.cs.selector;
        machine->source_repeat_eip = data->oldcpu.data.eip;
        machine->source_repeat_opcode = data->opcodes[
            core_machine_instruction_prefix_count(data)];
        machine->source_repeat_prefix = (type_unsigned_8)data->prefix_rep;
        machine->source_repeat_operand_size = operand_size;
        machine->source_repeat_address_size = address_size;
    }
    return ticks + core_machine_8086_timing_string_modifiers(machine, data,
        entry->form);
}

static C_INT core_machine_8086_timing_has_segment_override(
    const t_cpuins_data *data, type_unsigned_32 prefixes)
{
    type_unsigned_32 index;
    C_INT found = 0;

    if (data == STD_NULL) return 0;
    for (index = 0u; index < prefixes; ++index) {
        switch (data->opcodes[index]) {
        case 0x26u: case 0x2eu: case 0x36u: case 0x3eu:
            if (found) return 0;
            found = 1;
            break;
        case 0xf0u:
            break;
        default:
            return 0;
        }
    }
    return found;
}

static type_unsigned_64 core_machine_8086_timing_effective_address(
    const t_cpuins_data *data, type_unsigned_32 opcode_index)
{
    type_unsigned_8 modrm;
    type_unsigned_8 mode;
    type_unsigned_8 rm;

    if (opcode_index + 1u >= data->oplen) return 0u;
    modrm = data->opcodes[opcode_index + 1u];
    mode = modrm >> 6u;
    rm = modrm & 7u;
    if (mode == 3u) return 0u;
    if (mode == 0u) {
        switch (rm) {
        case 0u: case 2u: return 7u;
        case 1u: case 3u: return 8u;
        case 4u: case 5u: case 7u: return 5u;
        case 6u: return 6u;
        default: return 0u;
        }
    }
    switch (rm) {
    case 0u: case 2u: return 11u;
    case 1u: case 3u: return 12u;
    default: return 9u;
    }
}

static type_unsigned_64 core_machine_8086_timing_odd_word(
    const t_cpuins_data *data)
{
    return data->mrm.rsreg != STD_NULL &&
        ((data->mrm.rsreg->base + data->mrm.offset) & 1u) != 0u ?
        CORE_MACHINE_8086_ODD_WORD_TICKS : 0u;
}

typedef struct core_machine_primary_timing_shape {
    core_machine_source_timing_form form;
    C_INT memory;
    C_INT word;
} core_machine_primary_timing_shape;

static type_unsigned_8 core_machine_source_timing_primary_word_transfers(
    const core_machine_primary_timing_shape *shape)
{
    if (shape == STD_NULL || !shape->memory || !shape->word) return 0u;
    switch (shape->form) {
    case CORE_MACHINE_SOURCE_TIMING_ALU_ADD_RM_REGISTER:
    case CORE_MACHINE_SOURCE_TIMING_ALU_SUB_RM_REGISTER:
    case CORE_MACHINE_SOURCE_TIMING_ALU_RM_IMMEDIATE:
    case CORE_MACHINE_SOURCE_TIMING_XCHG_RM_REGISTER:
    case CORE_MACHINE_SOURCE_TIMING_INC_DEC_RM:
    case CORE_MACHINE_SOURCE_TIMING_GROUP3_NOT:
    case CORE_MACHINE_SOURCE_TIMING_GROUP3_NEG:
        return 2u;
    case CORE_MACHINE_SOURCE_TIMING_MOV_RM_IMMEDIATE:
        return 1u;
    case CORE_MACHINE_SOURCE_TIMING_CMP_RM_REGISTER:
    case CORE_MACHINE_SOURCE_TIMING_CMP_REGISTER_RM:
    case CORE_MACHINE_SOURCE_TIMING_CMP_RM_IMMEDIATE:
    case CORE_MACHINE_SOURCE_TIMING_TEST_RM_REGISTER:
    case CORE_MACHINE_SOURCE_TIMING_TEST_RM_IMMEDIATE:
    case CORE_MACHINE_SOURCE_TIMING_ALU_ADD_REGISTER_RM:
    case CORE_MACHINE_SOURCE_TIMING_ALU_SUB_REGISTER_RM:
        return 1u;
    case CORE_MACHINE_SOURCE_TIMING_GROUP3_MUL:
    case CORE_MACHINE_SOURCE_TIMING_GROUP3_IMUL:
    case CORE_MACHINE_SOURCE_TIMING_GROUP3_DIV:
    case CORE_MACHINE_SOURCE_TIMING_GROUP3_IDIV:
    case CORE_MACHINE_SOURCE_TIMING_IMUL_IMMEDIATE:
        return 1u;
    default:
        return 0u;
    }
}

/* This is intentionally an encoding classifier, not an instruction-handler
 * classifier.  A shared handler does not make r/m read, r/m write, and
 * register forms one timing row. */
static C_INT core_machine_source_timing_primary_shape(
    const t_cpuins_data *data, type_unsigned_32 opcode_index,
    core_machine_primary_timing_shape *out_shape)
{
    type_unsigned_8 opcode;
    type_unsigned_8 modrm;
    type_unsigned_8 group;
    C_INT memory;
    C_INT word;

    if (data == STD_NULL || out_shape == STD_NULL ||
        opcode_index >= data->oplen) return 0;
    opcode = data->opcodes[opcode_index];
    memory = core_machine_source_timing_modrm_is_memory(data, opcode_index);
    word = (opcode & 1u) != 0u;
    out_shape->memory = memory;
    out_shape->word = word;

    if ((opcode <= 0x3du && (opcode & 7u) <= 5u) ||
        opcode == 0x84u || opcode == 0x85u || opcode == 0x86u ||
        opcode == 0x87u || (opcode >= 0x88u && opcode <= 0x8bu) ||
        opcode == 0x8du || opcode == 0xa8u ||
        opcode == 0xa9u || opcode == 0xc6u || opcode == 0xc7u ||
        opcode == 0xfeu || opcode == 0xffu || opcode == 0xf6u ||
        opcode == 0xf7u || opcode == 0x69u || opcode == 0x6bu ||
        (opcode >= 0x80u && opcode <= 0x83u)) {
        if (opcode_index + 1u >= data->oplen) return 0;
        modrm = data->opcodes[opcode_index + 1u];
    } else {
        modrm = 0u;
    }

    if (opcode == 0x69u || opcode == 0x6bu) {
        out_shape->form = CORE_MACHINE_SOURCE_TIMING_IMUL_IMMEDIATE;
        return 1;
    }
    if (opcode >= 0x88u && opcode <= 0x8bu) {
        out_shape->form = !memory ?
            CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_REGISTER :
            (opcode == 0x88u || opcode == 0x89u ?
                CORE_MACHINE_SOURCE_TIMING_MOV_RM_REGISTER :
                CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_RM);
        return 1;
    }
    if (opcode >= 0xa0u && opcode <= 0xa3u) {
        out_shape->form = opcode == 0xa0u || opcode == 0xa1u ?
            CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_READ :
            CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_WRITE;
        out_shape->memory = TYPE_TRUE;
        return 1;
    }
    if (opcode >= 0xb0u && opcode <= 0xbfu) {
        out_shape->form = CORE_MACHINE_SOURCE_TIMING_MOV_IMMEDIATE;
        out_shape->memory = TYPE_FALSE;
        return 1;
    }
    if (opcode <= 0x3du && (opcode & 7u) <= 5u) {
        group = opcode >> 3u;
        if ((opcode & 7u) == 4u || (opcode & 7u) == 5u) {
            out_shape->form = group == 7u ?
                CORE_MACHINE_SOURCE_TIMING_CMP_RM_IMMEDIATE :
                CORE_MACHINE_SOURCE_TIMING_ALU_ACCUMULATOR_IMMEDIATE;
            out_shape->memory = TYPE_FALSE;
            return 1;
        }
        if (group == 7u) {
            out_shape->form = (opcode & 2u) == 0u ?
                CORE_MACHINE_SOURCE_TIMING_CMP_RM_REGISTER :
                CORE_MACHINE_SOURCE_TIMING_CMP_REGISTER_RM;
            return 1;
        }
        if ((opcode & 2u) == 0u) {
            out_shape->form = group == 0u || group == 2u || group == 4u ?
                CORE_MACHINE_SOURCE_TIMING_ALU_ADD_RM_REGISTER :
                CORE_MACHINE_SOURCE_TIMING_ALU_SUB_RM_REGISTER;
        } else {
            out_shape->form = group == 0u || group == 2u || group == 4u ?
                CORE_MACHINE_SOURCE_TIMING_ALU_ADD_REGISTER_RM :
                CORE_MACHINE_SOURCE_TIMING_ALU_SUB_REGISTER_RM;
        }
        return 1;
    }

    if (opcode >= 0x80u && opcode <= 0x83u) {
        group = (modrm >> 3u) & 7u;
        out_shape->form = group == 7u ?
            CORE_MACHINE_SOURCE_TIMING_CMP_RM_IMMEDIATE :
            CORE_MACHINE_SOURCE_TIMING_ALU_RM_IMMEDIATE;
        return 1;
    }
    if (opcode == 0x84u || opcode == 0x85u) {
        out_shape->form = CORE_MACHINE_SOURCE_TIMING_TEST_RM_REGISTER;
        return 1;
    }
    if (opcode == 0xa8u || opcode == 0xa9u) {
        out_shape->form = CORE_MACHINE_SOURCE_TIMING_TEST_ACCUMULATOR_IMMEDIATE;
        out_shape->memory = TYPE_FALSE;
        return 1;
    }
    if (opcode == 0x86u || opcode == 0x87u) {
        out_shape->form = memory ?
            CORE_MACHINE_SOURCE_TIMING_XCHG_RM_REGISTER :
            CORE_MACHINE_SOURCE_TIMING_XCHG_REGISTER;
        return 1;
    }
    if (opcode >= 0x91u && opcode <= 0x97u) {
        out_shape->form = CORE_MACHINE_SOURCE_TIMING_XCHG_REGISTER;
        out_shape->memory = TYPE_FALSE;
        out_shape->word = TYPE_TRUE;
        return 1;
    }
    if (opcode >= 0x40u && opcode <= 0x4fu) {
        out_shape->form = CORE_MACHINE_SOURCE_TIMING_INC_DEC_REGISTER;
        out_shape->memory = TYPE_FALSE;
        out_shape->word = TYPE_TRUE;
        return 1;
    }
    if (opcode == 0xfeu || opcode == 0xffu) {
        group = (modrm >> 3u) & 7u;
        if (group > 1u) return 0;
        out_shape->form = memory ? CORE_MACHINE_SOURCE_TIMING_INC_DEC_RM :
            CORE_MACHINE_SOURCE_TIMING_INC_DEC_REGISTER;
        return 1;
    }
    if (opcode == 0xf6u || opcode == 0xf7u) {
        group = (modrm >> 3u) & 7u;
        switch (group) {
        case 0u:
            out_shape->form = memory ?
                CORE_MACHINE_SOURCE_TIMING_TEST_RM_IMMEDIATE :
                CORE_MACHINE_SOURCE_TIMING_TEST_REGISTER_IMMEDIATE;
            break;
        case 2u:
            out_shape->form = CORE_MACHINE_SOURCE_TIMING_GROUP3_NOT;
            break;
        case 3u:
            out_shape->form = CORE_MACHINE_SOURCE_TIMING_GROUP3_NEG;
            break;
        case 4u:
            out_shape->form = CORE_MACHINE_SOURCE_TIMING_GROUP3_MUL;
            break;
        case 5u:
            out_shape->form = CORE_MACHINE_SOURCE_TIMING_GROUP3_IMUL;
            break;
        case 6u:
            out_shape->form = CORE_MACHINE_SOURCE_TIMING_GROUP3_DIV;
            break;
        case 7u:
            out_shape->form = CORE_MACHINE_SOURCE_TIMING_GROUP3_IDIV;
            break;
        default:
            return 0;
        }
        return 1;
    }
    if (opcode == 0xc6u || opcode == 0xc7u) {
        if (((modrm >> 3u) & 7u) != 0u) return 0;
        out_shape->form = memory ? CORE_MACHINE_SOURCE_TIMING_MOV_RM_IMMEDIATE :
            CORE_MACHINE_SOURCE_TIMING_MOV_IMMEDIATE;
        return 1;
    }
    if (opcode == 0x8du) {
        out_shape->form = CORE_MACHINE_SOURCE_TIMING_LEA;
        return memory;
    }
    if (opcode == 0x27u || opcode == 0x2fu || opcode == 0x37u ||
        opcode == 0x3fu) {
        out_shape->form = CORE_MACHINE_SOURCE_TIMING_ADJUST_SIMPLE;
        out_shape->memory = TYPE_FALSE;
        return 1;
    }
    if (opcode == 0xd4u || opcode == 0xd5u) {
        out_shape->form = opcode == 0xd4u ? CORE_MACHINE_SOURCE_TIMING_ADJUST_AAM :
            CORE_MACHINE_SOURCE_TIMING_ADJUST_AAD;
        out_shape->memory = TYPE_FALSE;
        return 1;
    }
    if (opcode == 0x98u || opcode == 0x99u) {
        out_shape->form = CORE_MACHINE_SOURCE_TIMING_CONVERSION;
        out_shape->memory = TYPE_FALSE;
        return 1;
    }
    if (opcode == 0x0fu && opcode_index + 2u < data->oplen &&
        data->opcodes[opcode_index + 1u] >= 0x90u &&
        data->opcodes[opcode_index + 1u] <= 0x9fu) {
        out_shape->form = CORE_MACHINE_SOURCE_TIMING_SETCC;
        out_shape->memory = core_machine_source_timing_modrm_is_memory(data,
            opcode_index + 1u);
        out_shape->word = TYPE_FALSE;
        return 1;
    }
    return 0;
}

/* Table 2-9 gives the immediate-IMUL rows as ranges.  S1 chose their fixed
 * midpoints, so neither the ModR/M form nor an external model may select an
 * endpoint or constrained value. */
static C_INT core_machine_80186_immediate_imul_midpoint_cost(
    type_unsigned_8 opcode, type_unsigned_64 *out_ticks)
{
    if (out_ticks == STD_NULL) return 0;
    if (opcode == 0x6bu) {
        *out_ticks = 24u;
        return 1;
    }
    if (opcode == 0x69u) {
        *out_ticks = 31u;
        return 1;
    }
    return 0;
}

/* L2-86BOX-8086-G3 is an independently written, operand-sensitive model.
 * The reference establishes a serial shift/add or restoring-divide machine;
 * it cannot be used as a scalar because its wait accounting is coupled to
 * its own prefetch queue.  We therefore count that machine's architectural
 * iteration decisions and normalize the result to Intel's published range.
 * No reference source is copied here. */
static type_unsigned_64 core_machine_8086_group3_bound(
    type_unsigned_64 minimum, type_unsigned_64 maximum,
    type_unsigned_64 value)
{
    if (value < minimum) return minimum;
    return value > maximum ? maximum : value;
}

static type_unsigned_8 core_machine_8086_group3_popcount(
    type_unsigned_64 value, type_unsigned_8 bits)
{
    type_unsigned_8 count = 0u;

    while (bits-- != 0u) {
        count += (type_unsigned_8)(value & 1u);
        value >>= 1u;
    }
    return count;
}

static type_unsigned_64 core_machine_8086_group3_magnitude(
    type_unsigned_64 value, type_unsigned_8 bits, C_INT *out_negative)
{
    type_unsigned_64 mask = (UINT64_C(1) << bits) - 1u;
    type_unsigned_64 sign = UINT64_C(1) << (bits - 1u);

    value &= mask;
    *out_negative = (value & sign) != 0u;
    return *out_negative ? ((~value + 1u) & mask) : value;
}

static type_unsigned_8 core_machine_8086_group3_division_steps(
    type_unsigned_64 dividend, type_unsigned_64 divisor, type_unsigned_8 bits)
{
    type_unsigned_64 mask = (UINT64_C(1) << bits) - 1u;
    type_unsigned_64 high = (dividend >> bits) & mask;
    type_unsigned_64 low = dividend & mask;
    type_unsigned_64 top = UINT64_C(1) << (bits - 1u);
    type_unsigned_8 count = 0u;
    type_unsigned_8 index;
    C_INT carry = 1;

    for (index = 0u; index < bits; ++index) {
        C_INT next_carry = (low & top) != 0u;

        low = ((low << 1u) | (type_unsigned_64)carry) & mask;
        carry = (high & top) != 0u;
        high = ((high << 1u) | (type_unsigned_64)next_carry) & mask;
        if (carry || high >= divisor) {
            high = (high - divisor) & mask;
            ++count;
        }
    }
    return count;
}

static type_unsigned_64 core_machine_8086_group3_model_cost(
    core_machine_source_timing_form form, C_INT word, C_INT memory,
    type_unsigned_64 operand1, type_unsigned_64 operand2)
{
    type_unsigned_8 bits = word ? 16u : 8u;
    type_unsigned_64 minimum;
    type_unsigned_64 maximum;
    type_unsigned_64 work;
    C_INT operand1_negative;
    C_INT operand2_negative;

    switch (form) {
    case CORE_MACHINE_SOURCE_TIMING_GROUP3_MUL:
        minimum = word ? (memory ? 124u : 118u) : (memory ? 76u : 70u);
        maximum = word ? (memory ? 139u : 133u) : (memory ? 83u : 77u);
        work = core_machine_8086_group3_popcount(operand1, bits);
        break;
    case CORE_MACHINE_SOURCE_TIMING_GROUP3_IMUL:
        minimum = word ? (memory ? 134u : 128u) : (memory ? 86u : 80u);
        maximum = word ? (memory ? 160u : 154u) : (memory ? 104u : 98u);
        operand1 = core_machine_8086_group3_magnitude(operand1, bits,
            &operand1_negative);
        operand2 = core_machine_8086_group3_magnitude(operand2, bits,
            &operand2_negative);
        work = core_machine_8086_group3_popcount(operand1, bits) +
            (operand1_negative ? (operand2_negative ? 11u : 24u) :
                (operand2_negative ? 21u : 10u));
        break;
    case CORE_MACHINE_SOURCE_TIMING_GROUP3_DIV:
        minimum = word ? (memory ? 150u : 144u) : (memory ? 86u : 80u);
        maximum = word ? (memory ? 168u : 162u) : (memory ? 96u : 90u);
        work = core_machine_8086_group3_division_steps(operand1, operand2,
            bits);
        break;
    case CORE_MACHINE_SOURCE_TIMING_GROUP3_IDIV:
        minimum = word ? (memory ? 171u : 165u) : (memory ? 107u : 101u);
        maximum = word ? (memory ? 190u : 184u) : (memory ? 118u : 112u);
        operand1 = core_machine_8086_group3_magnitude(operand1, bits * 2u,
            &operand1_negative);
        operand2 = core_machine_8086_group3_magnitude(operand2, bits,
            &operand2_negative);
        work = core_machine_8086_group3_division_steps(operand1, operand2,
            bits) + (operand1_negative ? 4u : 0u) +
            (operand2_negative ? 1u : 0u) + 9u;
        break;
    default:
        return 0u;
    }
    return core_machine_8086_group3_bound(minimum, maximum, minimum + work);
}

/* The 8086 entries retain their source-manual memory additions.  The selected
 * 80186 reference model already includes effective-address time, so only its
 * documented segment-prefix cost is added here. */
C_INT core_machine_l2_dynamic_arithmetic_model_cost(
    core_machine *machine, type_unsigned_64 *out_ticks)
{
    const t_cpuins_data *data;
    core_machine_primary_timing_shape shape;
    type_unsigned_32 prefixes;
    type_unsigned_8 opcode;
    type_unsigned_64 ticks;
    C_INT segment_override;
    C_INT lock_prefix;

    if (machine == STD_NULL || out_ticks == STD_NULL ||
        (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8086 &&
            machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186)) {
        return 0;
    }
    data = &machine->executor_cpu_instructions.data;
    prefixes = core_machine_instruction_prefix_count(data);
    if (prefixes >= data->oplen ||
        !core_machine_source_timing_primary_shape(data, prefixes, &shape)) {
        return 0;
    }
    segment_override = core_machine_8086_timing_has_segment_override(data,
        prefixes);
    lock_prefix = core_machine_instruction_has_lock_prefix(data, prefixes);
    if (prefixes != 0u && !segment_override && !lock_prefix) return 0;
    opcode = data->opcodes[prefixes];

    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8086) {
        ticks = core_machine_8086_group3_model_cost(shape.form, shape.word,
            shape.memory, data->opr1, data->opr2);
        if (ticks == 0u) return 0;
        if (shape.memory) {
            ticks += core_machine_8086_timing_effective_address(data, prefixes);
            if (shape.word) ticks += core_machine_8086_timing_odd_word(data);
            if (segment_override) ticks += CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS;
        }
        machine->source_timing_form_id = (type_unsigned_32)shape.form;
        *out_ticks = ticks;
        return 1;
    }

    switch (shape.form) {
    case CORE_MACHINE_SOURCE_TIMING_GROUP3_MUL:
        if (!shape.memory) ticks = shape.word ? 36u : 27u;
        else ticks = shape.word ? 42u : 33u;
        break;
    case CORE_MACHINE_SOURCE_TIMING_GROUP3_IMUL:
        if (!shape.memory) ticks = shape.word ? 36u : 27u;
        else ticks = shape.word ? 42u : 33u;
        break;
    case CORE_MACHINE_SOURCE_TIMING_GROUP3_DIV:
        if (!shape.memory) ticks = shape.word ? 38u : 29u;
        else ticks = shape.word ? 44u : 35u;
        break;
    case CORE_MACHINE_SOURCE_TIMING_GROUP3_IDIV:
        if (!shape.memory) ticks = shape.word ? 57u : 48u;
        else ticks = shape.word ? 63u : 54u;
        break;
    case CORE_MACHINE_SOURCE_TIMING_IMUL_IMMEDIATE:
        if (!core_machine_80186_immediate_imul_midpoint_cost(opcode,
            &ticks)) return 0;
        break;
    default:
        return 0;
    }
    if (segment_override && shape.memory) {
        ticks += CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS;
    }
    *out_ticks = ticks;
    return 1;
}

static C_INT core_machine_legacy_source_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks,
    const core_machine_legacy_source_timing_contract *contract)
{
    const t_cpuins_data *data = &machine->executor_cpu_instructions.data;
    type_unsigned_32 prefixes = core_machine_instruction_prefix_count(data);
    type_unsigned_8 opcode;
    type_unsigned_32 fallthrough;
    C_INT segment_override;
    C_INT lock_prefix;

    if (out_ticks == STD_NULL || contract == STD_NULL) return 0;
    if (prefixes >= data->oplen) {
        machine->source_repeat_active = TYPE_FALSE;
        *out_ticks = 0u;
        return 1;
    }
    opcode = data->opcodes[prefixes];
    machine->source_repeat_active = TYPE_FALSE;
    segment_override = core_machine_8086_timing_has_segment_override(data,
        prefixes);
    lock_prefix = core_machine_instruction_has_lock_prefix(data, prefixes);
    /* The 8086 LOCK byte is a two-clock prefix for any following valid
     * instruction.  The common selector appends that term after this source
     * row succeeds, so accept it here together with an optional segment
     * override instead of sending otherwise-valid forms to UNALLOCATED. */
    if (prefixes != 0u && !segment_override && !lock_prefix) {
        core_machine_source_timing_mark_unallocated(machine, out_ticks);
        return 1;
    }
    if (opcode >= 0x70u && opcode <= 0x7fu) {
        machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_JCC;
        fallthrough = TYPE_MASK_UNSIGNED_16(data->oldcpu.data.eip + prefixes + 2u);
        *out_ticks = machine->executor_cpu.data.eip == fallthrough ?
            contract->jcc_not_taken_ticks : contract->jcc_taken_ticks;
        return 1;
    }
    switch (opcode) {
    case 0x90u:
        if (prefixes != 0u && !lock_prefix) break;
        *out_ticks = core_machine_source_timing_lookup(machine, contract->ledger,
            contract->ledger_entries,
            CORE_MACHINE_SOURCE_TIMING_NOP);
        return 1;
    case 0xf8u:
        if (prefixes != 0u && !lock_prefix) break;
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8086) {
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_FLAG;
            *out_ticks = 2u;
            return 1;
        }
        *out_ticks = core_machine_source_timing_lookup(machine, contract->ledger,
            contract->ledger_entries,
            CORE_MACHINE_SOURCE_TIMING_CLC);
        return 1;
    case 0x88u: case 0x89u: case 0x8au: case 0x8bu:
        if (!data->flagMem) {
            if (prefixes != 0u && !lock_prefix) break;
            *out_ticks = core_machine_source_timing_lookup(machine, contract->ledger,
                contract->ledger_entries,
                CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_REGISTER);
            return 1;
        }
        *out_ticks = core_machine_source_timing_lookup(machine, contract->ledger,
            contract->ledger_entries,
            opcode == 0x88u || opcode == 0x89u ?
            CORE_MACHINE_SOURCE_TIMING_MOV_RM_REGISTER :
            CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_RM) +
            (segment_override ? CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS : 0u) +
            ((opcode == 0x89u || opcode == 0x8bu) ?
                core_machine_8086_timing_odd_word(data) : 0u);
        if (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186) {
            *out_ticks += core_machine_8086_timing_effective_address(data,
                prefixes);
        }
        return 1;
    case 0xa0u: case 0xa1u: case 0xa2u: case 0xa3u:
        *out_ticks = core_machine_source_timing_lookup(machine, contract->ledger,
            contract->ledger_entries,
            opcode == 0xa0u || opcode == 0xa1u ?
            CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_READ :
            CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_WRITE) +
            (segment_override ? CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS : 0u) +
            ((opcode == 0xa1u || opcode == 0xa3u) ?
                core_machine_8086_timing_odd_word(data) : 0u);
        return 1;
    case 0x8cu:
        if ((machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8086 &&
             machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186) ||
            prefixes + 1u >= data->oplen ||
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) > 3u) break;
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186) {
            machine->source_timing_form_id =
                CORE_MACHINE_SOURCE_TIMING_MOV_SREG_MEMORY;
            *out_ticks = data->flagMem ? 9u +
                core_machine_8086_timing_odd_word(data) +
                (segment_override ? CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS : 0u) :
                2u;
            return 1;
        }
        if (!data->flagMem) {
            machine->source_timing_form_id =
                CORE_MACHINE_SOURCE_TIMING_MOV_SREG_REGISTER;
            *out_ticks = 2u;
            return 1;
        }
        machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_MOV_SREG_MEMORY;
        *out_ticks = 9u + core_machine_8086_timing_effective_address(data,
            prefixes) + core_machine_8086_timing_odd_word(data) +
            (segment_override ? CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS : 0u);
        return 1;
    case 0x8eu:
        if ((machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8086 &&
             machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186) ||
            prefixes + 1u >= data->oplen ||
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) == 1u ||
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) > 3u) break;
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186) {
            machine->source_timing_form_id =
                CORE_MACHINE_SOURCE_TIMING_MOV_SREG_MEMORY;
            *out_ticks = data->flagMem ? 11u +
                core_machine_8086_timing_odd_word(data) +
                (segment_override ? CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS : 0u) :
                2u;
            return 1;
        }
        if (!data->flagMem) {
            machine->source_timing_form_id =
                CORE_MACHINE_SOURCE_TIMING_MOV_SREG_REGISTER;
            *out_ticks = 2u;
            return 1;
        }
        machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_MOV_SREG_MEMORY;
        *out_ticks = 8u + core_machine_8086_timing_effective_address(data,
            prefixes) + core_machine_8086_timing_odd_word(data) +
            (segment_override ? CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS : 0u);
        return 1;
    case 0xc4u: case 0xc5u:
        if ((machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8086 &&
             machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186) ||
            !data->flagMem) break;
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186) {
            machine->source_timing_form_id =
                CORE_MACHINE_SOURCE_TIMING_8086_LOAD_POINTER;
            *out_ticks = 18u + (type_unsigned_64)2u *
                core_machine_8086_timing_odd_word(data) +
                (segment_override ? CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS : 0u);
            return 1;
        }
        machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_LOAD_POINTER;
        *out_ticks = 16u + core_machine_8086_timing_effective_address(data,
            prefixes) + (type_unsigned_64)2u *
            core_machine_8086_timing_odd_word(data) +
            (segment_override ? CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS : 0u);
        return 1;
    case 0xcau: case 0xcbu:
        if (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8086) break;
        machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_RET_FAR;
        *out_ticks = opcode == 0xcau ? 17u : 18u;
        return 1;
    case 0xc0u: case 0xc1u: case 0xd0u: case 0xd1u:
    case 0xd2u: case 0xd3u: {
        type_unsigned_8 count;

        if ((machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8086 &&
             machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186) ||
            prefixes + 1u >= data->oplen ||
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) == 6u) break;
        machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_GROUP2;
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186) {
            count = (opcode == 0xd0u || opcode == 0xd1u) ? 1u :
                (opcode == 0xd2u || opcode == 0xd3u) ?
                (TYPE_MASK_UNSIGNED_8(data->oldcpu.data.cx) & 0x1fu) :
                (TYPE_MASK_UNSIGNED_8(data->cimm) & 0x1fu);
            *out_ticks = data->flagMem ?
                ((opcode == 0xd0u || opcode == 0xd1u) ? 15u : 17u + count) :
                ((opcode == 0xd0u || opcode == 0xd1u) ? 2u : 5u + count);
            if (data->flagMem) {
                *out_ticks += (opcode & 1u ? core_machine_8086_timing_odd_word(data) :
                    0u) + (segment_override ?
                    CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS : 0u);
            }
            return 1;
        }
        if (opcode == 0xd0u || opcode == 0xd1u) {
            *out_ticks = data->flagMem ? 15u : 2u;
        } else {
            count = TYPE_MASK_UNSIGNED_8(data->oldcpu.data.cx);
            *out_ticks = (data->flagMem ? 20u : 8u) + 4u * count;
        }
        if (data->flagMem) {
            *out_ticks += core_machine_8086_timing_effective_address(data,
                prefixes) + (opcode & 1u ? core_machine_8086_timing_odd_word(data) :
                0u) + (segment_override ? CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS : 0u);
        }
        return 1;
    }
    case 0x06u: case 0x0eu: case 0x16u: case 0x1eu:
        if (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8086 &&
            machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186) break;
        machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_PUSH_REGISTER;
        *out_ticks = machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186 ?
            9u : 10u;
        return 1;
    /* 0F is POP CS only on the 8086.  It is the 0F escape on later
     * processors, so keep this source rule in the 8086 profile branch. */
    case 0x07u: case 0x17u: case 0x1fu:
        if (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8086 &&
            machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186) break;
        machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_POP_REGISTER;
        *out_ticks = 8u;
        return 1;
    case 0x0fu:
        if (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8086) break;
        machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_POP_REGISTER;
        *out_ticks = 8u;
        return 1;
    case 0xf5u: case 0xf9u: case 0xfau: case 0xfbu: case 0xfcu:
    case 0xfdu:
        if (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8086 &&
            machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186) break;
        machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_FLAG;
        *out_ticks = 2u;
        return 1;
    case 0x9eu: case 0x9fu:
        if (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8086 &&
            machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186) break;
        machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_FLAG;
        *out_ticks = machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186 ?
            (opcode == 0x9eu ? 3u : 2u) : 4u;
        return 1;
    case 0x9bu:
        if (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8086 &&
            machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186) break;
        machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_WAIT;
        *out_ticks = (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186 ?
            6u : 3u) + (type_unsigned_64)5u *
            core_machine_fpu_last_wait_iterations(&machine->fpu);
        return 1;
    case 0xd7u:
        if (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8086 &&
            machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186) break;
        machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_XLAT;
        *out_ticks = 11u + (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8086 &&
            segment_override ?
            CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS : 0u);
        return 1;
    default:
        if ((machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8086 ||
             machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186) &&
            opcode >= 0xd8u && opcode <= 0xdfu) {
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_ESC;
            *out_ticks = machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186 ?
                (data->flagMem ? 6u : 2u) : data->flagMem ? 8u +
                core_machine_8086_timing_effective_address(data, prefixes) +
                core_machine_8086_timing_odd_word(data) : 2u;
            if (data->flagMem && segment_override &&
                machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8086) {
                *out_ticks += CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS;
            }
            return 1;
        }
        if (opcode >= 0xb0u && opcode <= 0xbfu &&
            (prefixes == 0u || lock_prefix)) {
            *out_ticks = core_machine_source_timing_lookup(machine, contract->ledger,
                contract->ledger_entries,
                CORE_MACHINE_SOURCE_TIMING_MOV_IMMEDIATE);
            return 1;
        }
        break;
    }
    core_machine_source_timing_mark_unallocated(machine, out_ticks);
    return 1;
}

C_INT core_machine_8086_source_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks)
{
    return core_machine_legacy_source_instruction_cost(machine, out_ticks,
        &core_machine_8086_source_timing_contract);
}

C_INT core_machine_80186_source_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks)
{
    return core_machine_legacy_source_instruction_cost(machine, out_ticks,
        &core_machine_80186_source_timing_contract);
}

static type_unsigned_64 core_machine_80286_source_timing_lookup(
    core_machine *machine, core_machine_source_timing_form form)
{
    return core_machine_source_timing_lookup(machine,
        core_machine_80286_source_timing_ledger,
        sizeof(core_machine_80286_source_timing_ledger) /
            sizeof(core_machine_80286_source_timing_ledger[0]), form);
}

static type_unsigned_64 core_machine_80286_timing_effective_address(
    const t_cpuins_data *data, type_unsigned_32 opcode_index)
{
    type_unsigned_8 modrm;
    type_unsigned_8 mode;
    type_unsigned_8 rm;

    if (opcode_index + 1u >= data->oplen) return 0u;
    modrm = data->opcodes[opcode_index + 1u];
    mode = modrm >> 6u;
    rm = modrm & 7u;
    return mode != 0u && mode != 3u && rm <= 3u ?
        CORE_MACHINE_80286_BASE_INDEX_DISPLACEMENT_TICKS : 0u;
}

static type_unsigned_64 core_machine_80286_timing_odd_word(
    const t_cpuins_data *data)
{
    return data->mrm.rsreg != STD_NULL &&
        ((data->mrm.rsreg->base + data->mrm.offset) & 1u) != 0u ?
        CORE_MACHINE_80286_ODD_WORD_TICKS : 0u;
}

static type_unsigned_8 core_machine_80286_group2_count(
    const t_cpuins_data *data, type_unsigned_32 prefixes, type_unsigned_8 opcode)
{
    if (opcode == 0xd2u || opcode == 0xd3u) {
        return TYPE_MASK_UNSIGNED_8(data->oldcpu.data.cx) & 0x1fu;
    }
    if ((opcode == 0xc0u || opcode == 0xc1u) && prefixes + 2u < data->oplen) {
        return TYPE_MASK_UNSIGNED_8(data->cimm) & 0x1fu;
    }
    return 0u;
}

C_INT core_machine_primary_source_instruction_cost(
    core_machine *machine, type_unsigned_64 *out_ticks)
{
    const t_cpuins_data *data;
    core_machine_primary_timing_shape shape;
    type_unsigned_32 prefixes;
    type_unsigned_8 opcode;
    type_unsigned_64 ticks;
    type_unsigned_8 transfers;
    C_INT segment_override;
    C_INT lock_prefix;

    if (machine == STD_NULL || out_ticks == STD_NULL) return 0;
    data = &machine->executor_cpu_instructions.data;
    prefixes = core_machine_instruction_prefix_count(data);
    segment_override = core_machine_8086_timing_has_segment_override(data,
        prefixes);
    lock_prefix = core_machine_instruction_has_lock_prefix(data, prefixes);
    if (prefixes >= data->oplen ||
        (prefixes != 0u &&
            ((machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 &&
                !core_machine_80386_timing_has_source_prefixes(data,
                    prefixes)) ||
             (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 &&
                !segment_override && !lock_prefix))) ||
        !core_machine_source_timing_primary_shape(data, prefixes, &shape)) {
        return 0;
    }
    opcode = data->opcodes[prefixes];
    transfers = core_machine_source_timing_primary_word_transfers(&shape);

    switch (machine->cpu_profile) {
    case CORE_MACHINE_CPU_PROFILE_8086:
        switch (shape.form) {
        case CORE_MACHINE_SOURCE_TIMING_ALU_ADD_RM_REGISTER:
        case CORE_MACHINE_SOURCE_TIMING_ALU_SUB_RM_REGISTER:
            ticks = shape.memory ? 16u : 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ALU_ADD_REGISTER_RM:
        case CORE_MACHINE_SOURCE_TIMING_ALU_SUB_REGISTER_RM:
            ticks = shape.memory ? 9u : 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ALU_ACCUMULATOR_IMMEDIATE:
        case CORE_MACHINE_SOURCE_TIMING_ALU_REGISTER_IMMEDIATE:
            ticks = 4u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ALU_RM_IMMEDIATE:
            ticks = shape.memory ? 17u : 4u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_CMP_RM_REGISTER:
        case CORE_MACHINE_SOURCE_TIMING_CMP_REGISTER_RM:
            ticks = shape.memory ? 9u : 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_CMP_RM_IMMEDIATE:
            ticks = shape.memory ? 10u : 4u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_TEST_RM_REGISTER:
            ticks = shape.memory ? 9u : 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_TEST_ACCUMULATOR_IMMEDIATE:
            ticks = 4u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_TEST_REGISTER_IMMEDIATE:
            ticks = 5u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_TEST_RM_IMMEDIATE:
            ticks = shape.memory ? 11u : 5u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_XCHG_REGISTER:
            ticks = opcode >= 0x91u && opcode <= 0x97u ? 3u : 4u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_XCHG_RM_REGISTER:
            ticks = 17u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_INC_DEC_REGISTER:
            ticks = shape.word ? 2u : 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_INC_DEC_RM:
            ticks = 15u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_MOV_IMMEDIATE:
            ticks = 4u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_MOV_RM_IMMEDIATE:
            ticks = shape.memory ? 10u : 4u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_LEA:
            ticks = 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ADJUST_SIMPLE:
            ticks = 4u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ADJUST_AAM:
            ticks = 83u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ADJUST_AAD:
            ticks = 60u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_CONVERSION:
            ticks = opcode == 0x99u ? 5u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_NOT:
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_NEG:
            ticks = shape.memory ? 16u : 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_MUL:
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_IMUL:
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_DIV:
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_IDIV:
        case CORE_MACHINE_SOURCE_TIMING_IMUL_IMMEDIATE:
            return 0;
        default:
            return 0;
        }
        if (shape.memory) {
            ticks += core_machine_8086_timing_effective_address(data, prefixes);
            ticks += (type_unsigned_64)transfers *
                core_machine_8086_timing_odd_word(data);
            if (segment_override) ticks += CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS;
        }
        break;
    case CORE_MACHINE_CPU_PROFILE_80186:
        switch (shape.form) {
        case CORE_MACHINE_SOURCE_TIMING_ALU_ADD_RM_REGISTER:
        case CORE_MACHINE_SOURCE_TIMING_ALU_ADD_REGISTER_RM:
        case CORE_MACHINE_SOURCE_TIMING_ALU_SUB_RM_REGISTER:
        case CORE_MACHINE_SOURCE_TIMING_ALU_SUB_REGISTER_RM:
            ticks = shape.memory ? 10u : 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ALU_ACCUMULATOR_IMMEDIATE:
            ticks = 4u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ALU_REGISTER_IMMEDIATE:
            ticks = 4u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ALU_RM_IMMEDIATE:
            ticks = shape.memory ? 16u : 4u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_CMP_RM_REGISTER:
        case CORE_MACHINE_SOURCE_TIMING_CMP_REGISTER_RM:
            ticks = shape.memory ? 10u : 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_CMP_RM_IMMEDIATE:
            ticks = shape.memory ? 10u : (shape.word ? 4u : 3u);
            break;
        case CORE_MACHINE_SOURCE_TIMING_TEST_RM_REGISTER:
            ticks = shape.memory ? 10u : 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_TEST_ACCUMULATOR_IMMEDIATE:
            ticks = shape.word ? 4u : 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_TEST_REGISTER_IMMEDIATE:
            ticks = 4u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_TEST_RM_IMMEDIATE:
            ticks = shape.memory ? 10u : 4u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_XCHG_REGISTER:
            ticks = opcode >= 0x91u ? 3u : 4u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_XCHG_RM_REGISTER:
            ticks = 17u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_INC_DEC_REGISTER:
            ticks = 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_INC_DEC_RM:
            ticks = 15u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_MOV_IMMEDIATE:
            ticks = 4u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_MOV_RM_IMMEDIATE:
            ticks = shape.memory ? (shape.word ? 13u : 12u) : 4u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_LEA:
            ticks = 6u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ADJUST_SIMPLE:
            if (opcode == 0x37u) ticks = 8u;
            else if (opcode == 0x3fu) ticks = 7u;
            else ticks = 4u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ADJUST_AAM:
            ticks = 19u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ADJUST_AAD:
            ticks = 15u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_CONVERSION:
            ticks = opcode == 0x99u ? 4u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_NOT:
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_NEG:
            ticks = 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_MUL:
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_IMUL:
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_DIV:
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_IDIV:
        case CORE_MACHINE_SOURCE_TIMING_IMUL_IMMEDIATE:
            return 0;
        default:
            return 0;
        }
        /* Table 2-9 excludes EA work but assumes even word transfers.
         * Table 1-16 supplies the independent odd-word and segment terms. */
        if (shape.memory) {
            ticks += (type_unsigned_64)transfers *
                core_machine_8086_timing_odd_word(data);
            if (segment_override) ticks += CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS;
        }
        break;
    case CORE_MACHINE_CPU_PROFILE_80286:
        switch (shape.form) {
        case CORE_MACHINE_SOURCE_TIMING_ALU_ADD_RM_REGISTER:
        case CORE_MACHINE_SOURCE_TIMING_ALU_ADD_REGISTER_RM:
        case CORE_MACHINE_SOURCE_TIMING_ALU_SUB_RM_REGISTER:
        case CORE_MACHINE_SOURCE_TIMING_ALU_SUB_REGISTER_RM:
            ticks = shape.memory ? 7u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ALU_ACCUMULATOR_IMMEDIATE:
        case CORE_MACHINE_SOURCE_TIMING_ALU_REGISTER_IMMEDIATE:
            ticks = 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ALU_RM_IMMEDIATE:
            ticks = shape.memory ? 7u : 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_CMP_RM_REGISTER:
            ticks = shape.memory ? 7u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_CMP_REGISTER_RM:
            ticks = shape.memory ? 6u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_CMP_RM_IMMEDIATE:
            ticks = shape.memory ? 6u : 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_TEST_RM_REGISTER:
            ticks = shape.memory ? 6u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_TEST_ACCUMULATOR_IMMEDIATE:
        case CORE_MACHINE_SOURCE_TIMING_TEST_REGISTER_IMMEDIATE:
            ticks = 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_TEST_RM_IMMEDIATE:
            ticks = shape.memory ? 6u : 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_XCHG_REGISTER:
            ticks = 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_XCHG_RM_REGISTER:
            ticks = shape.memory ? 5u : 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_INC_DEC_REGISTER:
            ticks = 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_INC_DEC_RM:
            ticks = shape.memory ? 7u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_MOV_IMMEDIATE:
        case CORE_MACHINE_SOURCE_TIMING_MOV_RM_IMMEDIATE:
            ticks = shape.memory ? 3u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_LEA:
            ticks = 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ADJUST_SIMPLE:
            ticks = 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ADJUST_AAM:
            ticks = 16u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ADJUST_AAD:
            ticks = 14u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_CONVERSION:
            ticks = 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_NOT:
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_NEG:
            ticks = shape.memory ? 7u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_MUL:
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_IMUL:
        case CORE_MACHINE_SOURCE_TIMING_IMUL_IMMEDIATE:
            ticks = shape.word ? (shape.memory ? 24u : 21u) :
                (shape.memory ? 16u : 13u);
            break;
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_DIV:
            ticks = shape.word ? (shape.memory ? 25u : 22u) :
                (shape.memory ? 17u : 14u);
            break;
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_IDIV:
            ticks = shape.word ? (shape.memory ? 28u : 25u) :
                (shape.memory ? 20u : 17u);
            break;
        default:
            return 0;
        }
        if (shape.memory) {
            ticks += core_machine_80286_timing_effective_address(data, prefixes);
            ticks += (type_unsigned_64)transfers *
                core_machine_80286_timing_odd_word(data);
        }
        break;
    case CORE_MACHINE_CPU_PROFILE_80386:
        switch (shape.form) {
        case CORE_MACHINE_SOURCE_TIMING_ALU_ADD_RM_REGISTER:
            ticks = shape.memory ? 7u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ALU_ADD_REGISTER_RM:
            ticks = shape.memory ? 6u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ALU_SUB_RM_REGISTER:
            ticks = shape.memory ? 6u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ALU_SUB_REGISTER_RM:
            ticks = shape.memory ? 7u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ALU_ACCUMULATOR_IMMEDIATE:
        case CORE_MACHINE_SOURCE_TIMING_ALU_REGISTER_IMMEDIATE:
            ticks = 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ALU_RM_IMMEDIATE:
            ticks = shape.memory ? 7u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_CMP_RM_REGISTER:
            ticks = shape.memory ? 5u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_CMP_REGISTER_RM:
            ticks = shape.memory ? 6u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_CMP_RM_IMMEDIATE:
            ticks = shape.memory ? 5u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_TEST_RM_REGISTER:
        case CORE_MACHINE_SOURCE_TIMING_TEST_RM_IMMEDIATE:
            ticks = shape.memory ? 5u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_TEST_ACCUMULATOR_IMMEDIATE:
        case CORE_MACHINE_SOURCE_TIMING_TEST_REGISTER_IMMEDIATE:
            ticks = 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_XCHG_REGISTER:
            ticks = 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_XCHG_RM_REGISTER:
            ticks = shape.memory ? 5u : 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_INC_DEC_REGISTER:
            ticks = 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_INC_DEC_RM:
            ticks = shape.memory ? 6u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_REGISTER:
        case CORE_MACHINE_SOURCE_TIMING_MOV_IMMEDIATE:
            ticks = 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_MOV_RM_REGISTER:
            ticks = shape.memory ? 2u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_RM:
        case CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_READ:
            ticks = shape.memory ? 4u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_WRITE:
            ticks = 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_MOV_RM_IMMEDIATE:
            ticks = 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_LEA:
            ticks = 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ADJUST_SIMPLE:
            ticks = 4u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ADJUST_AAM:
            ticks = 17u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_ADJUST_AAD:
            ticks = 19u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_CONVERSION:
            ticks = 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_NOT:
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_NEG:
            ticks = shape.memory ? 6u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_DIV:
            if (!shape.word) ticks = shape.memory ? 17u : 14u;
            else if (data->prefix_oprsize) ticks = shape.memory ? 41u : 38u;
            else ticks = shape.memory ? 25u : 22u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_GROUP3_IDIV:
            if (!shape.word) ticks = shape.memory ? 22u : 19u;
            else if (data->prefix_oprsize) ticks = shape.memory ? 46u : 43u;
            else ticks = shape.memory ? 30u : 27u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_SETCC:
            ticks = shape.memory ? 5u : 4u;
            break;
        default:
            return 0;
        }
        break;
    default:
        return 0;
    }
    machine->source_timing_form_id = (type_unsigned_32)shape.form;
    *out_ticks = ticks;
    return 1;
}

/* Control and stack rows are selected after a successful instruction refresh.
 * The decoder capture supplies the executed shape, oldcpu supplies the source
 * mode, and the post-refresh CPU supplies the published transfer outcome.
 * No handler owns a clock; paths needing a gate, privilege change, task switch
 * or exception delivery deliberately remain outside this classifier. */
static type_unsigned_64 core_machine_control_stack_source_lookup(
    core_machine *machine, core_machine_source_timing_form form)
{
    if (machine == STD_NULL) return CORE_MACHINE_SOURCE_UNALLOCATED_TICKS;
    switch (machine->cpu_profile) {
    case CORE_MACHINE_CPU_PROFILE_8086:
        return core_machine_source_timing_lookup(machine,
            core_machine_8086_source_timing_ledger,
            sizeof(core_machine_8086_source_timing_ledger) /
                sizeof(core_machine_8086_source_timing_ledger[0]), form);
    case CORE_MACHINE_CPU_PROFILE_80186:
        return core_machine_source_timing_lookup(machine,
            core_machine_80186_source_timing_ledger,
            sizeof(core_machine_80186_source_timing_ledger) /
                sizeof(core_machine_80186_source_timing_ledger[0]), form);
    case CORE_MACHINE_CPU_PROFILE_80286:
        return core_machine_80286_source_timing_lookup(machine, form);
    case CORE_MACHINE_CPU_PROFILE_80386:
        return core_machine_80386_source_timing_lookup(machine, form);
    default:
        return CORE_MACHINE_SOURCE_UNALLOCATED_TICKS;
    }
}

static C_INT core_machine_control_stack_is_protected(
    const t_cpuins_data *data)
{
    return data != STD_NULL && (data->oldcpu.data.cr0 & VCPU_CR0_PE) != 0u &&
        (data->oldcpu.data.eflags & VCPU_EFLAGS_VM) == 0u;
}

static C_INT core_machine_control_stack_next_term(core_machine *machine,
    type_unsigned_64 *out_ticks)
{
    core_machine_cpu_instruction_lexeme lexeme;

    if (machine == STD_NULL || out_ticks == STD_NULL) return 0;
    if (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80286 &&
        machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80386) {
        *out_ticks = 0u;
        return 1;
    }
    if (!core_machine_cpu_execution_preview_lexeme(
            &machine->executor_cpu_execution, &lexeme) || !lexeme.available) {
        return 0;
    }
    *out_ticks = machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ?
        lexeme.byte_count : lexeme.component_count;
    return 1;
}

static C_INT core_machine_control_stack_prefixes_are_source_backed(
    const core_machine *machine, const t_cpuins_data *data,
    type_unsigned_32 prefixes)
{
    C_INT segment_override;

    if (machine == STD_NULL || data == STD_NULL) return 0;
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) {
        return core_machine_80386_timing_has_source_prefixes(data, prefixes) &&
            !data->flagLock;
    }
    segment_override = core_machine_8086_timing_has_segment_override(data,
        prefixes);
    return prefixes == 0u || segment_override ||
        core_machine_instruction_has_lock_prefix(data, prefixes);
}

static C_INT core_machine_control_stack_add_next_term(core_machine *machine,
    type_unsigned_64 base_ticks, type_unsigned_64 *out_ticks)
{
    type_unsigned_64 next_ticks;

    if (out_ticks == STD_NULL ||
        !core_machine_control_stack_next_term(machine, &next_ticks) ||
        !core_machine_add_ticks(&base_ticks, next_ticks)) {
        return 0;
    }
    *out_ticks = base_ticks;
    return 1;
}

static type_unsigned_64 core_machine_control_stack_memory_additions(
    const core_machine *machine, const t_cpuins_data *data,
    type_unsigned_32 prefixes, type_unsigned_8 word_transfers)
{
    type_unsigned_64 ticks = 0u;

    if (machine == STD_NULL || data == STD_NULL) return 0u;
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8086) {
        ticks = core_machine_8086_timing_effective_address(data, prefixes) +
            (type_unsigned_64)word_transfers *
                core_machine_8086_timing_odd_word(data);
        if (core_machine_8086_timing_has_segment_override(data, prefixes)) {
            ticks += CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS;
        }
    } else if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186) {
        ticks = (type_unsigned_64)word_transfers *
            core_machine_8086_timing_odd_word(data);
        if (core_machine_8086_timing_has_segment_override(data, prefixes)) {
            ticks += CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS;
        }
    } else if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286) {
        ticks = core_machine_80286_timing_effective_address(data, prefixes) +
            (type_unsigned_64)word_transfers *
                core_machine_80286_timing_odd_word(data);
    }
    return ticks;
}

C_INT core_machine_control_stack_source_instruction_cost(
    core_machine *machine, type_unsigned_64 *out_ticks)
{
    const t_cpuins_data *data;
    type_unsigned_32 prefixes;
    type_unsigned_8 opcode;
    type_unsigned_8 extension;
    type_unsigned_64 ticks;
    C_INT memory;
    C_INT protected_mode;
    C_INT same_privilege;

    if (machine == STD_NULL || out_ticks == STD_NULL) return 0;
    data = &machine->executor_cpu_instructions.data;
    prefixes = core_machine_instruction_prefix_count(data);
    if (prefixes >= data->oplen ||
        !core_machine_control_stack_prefixes_are_source_backed(machine, data,
            prefixes)) {
        return 0;
    }
    opcode = data->opcodes[prefixes];
    protected_mode = core_machine_control_stack_is_protected(data);
    same_privilege = !protected_mode ||
        data->oldcpu.data.cs.dpl == machine->executor_cpu.data.cs.dpl;
    memory = core_machine_source_timing_modrm_is_memory(data, prefixes);
    extension = prefixes + 1u < data->oplen ?
        (data->opcodes[prefixes + 1u] >> 3u) & 7u : 8u;

    switch (opcode) {
    case 0xe8u:
        return core_machine_control_stack_add_next_term(machine,
            core_machine_control_stack_source_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_DIRECT), out_ticks);
    case 0x9au:
        if (!same_privilege || protected_mode) return 0;
        ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_CALL_FAR_DIRECT);
        return core_machine_control_stack_add_next_term(machine, ticks, out_ticks);
    case 0xe9u: case 0xebu:
        return core_machine_control_stack_add_next_term(machine,
            core_machine_control_stack_source_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_JMP_DIRECT), out_ticks);
    case 0xeau:
        if (!same_privilege || protected_mode) return 0;
        ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_JMP_FAR_DIRECT);
        return core_machine_control_stack_add_next_term(machine, ticks, out_ticks);
    case 0xc2u:
        return core_machine_control_stack_add_next_term(machine,
            core_machine_control_stack_source_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_RET_NEAR_IMMEDIATE), out_ticks);
    case 0xc3u:
        return core_machine_control_stack_add_next_term(machine,
            core_machine_control_stack_source_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_RET_NEAR), out_ticks);
    case 0x50u: case 0x51u: case 0x52u: case 0x53u:
    case 0x54u: case 0x55u: case 0x56u: case 0x57u:
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_PUSH_REGISTER);
        return 1;
    case 0x58u: case 0x59u: case 0x5au: case 0x5bu:
    case 0x5cu: case 0x5du: case 0x5eu: case 0x5fu:
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_POP_REGISTER);
        return 1;
    case 0x60u:
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_PUSHA);
        return machine->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186;
    case 0x61u:
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_POPA);
        return machine->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186;
    case 0x62u:
        if (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186 ||
            !memory) return 0;
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_BOUND);
        return 1;
    case 0x68u: case 0x6au:
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_PUSH_IMMEDIATE);
        return machine->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186;
    case 0x8fu:
        if (extension != 0u) return 0;
        *out_ticks = core_machine_control_stack_source_lookup(machine, memory ?
            CORE_MACHINE_SOURCE_TIMING_POP_MEMORY :
            CORE_MACHINE_SOURCE_TIMING_POP_REGISTER);
        if (memory) {
            *out_ticks += core_machine_control_stack_memory_additions(machine,
                data, prefixes, 2u);
        }
        return 1;
    case 0x9cu:
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_PUSHF);
        return 1;
    case 0x9du:
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_POPF);
        return 1;
    case 0xc8u:
        if (machine->cpu_profile < CORE_MACHINE_CPU_PROFILE_80186) return 0;
        extension = data->opcodes[data->oplen - 1u] & 0x1fu;
        if (extension == 0u) {
            *out_ticks = core_machine_control_stack_source_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_ENTER_LEVEL_ZERO);
        } else if (extension == 1u) {
            *out_ticks = core_machine_control_stack_source_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_ENTER_LEVEL_ONE);
        } else {
            ticks = machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186 ?
                22u + 16u * (extension - 1u) :
                machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ?
                12u + 4u * extension : 15u + 4u * (extension - 1u);
            *out_ticks = ticks;
        }
        return 1;
    case 0xc9u:
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_LEAVE);
        return machine->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186;
    case 0xe0u: case 0xe1u: case 0xe2u: case 0xe3u:
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8086) {
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_LOOP;
        }
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) {
            ticks = opcode == 0xe3u ?
                (machine->executor_cpu.data.eip == data->oldcpu.data.eip +
                    prefixes + 2u ? 5u : 9u) : 11u;
            return core_machine_control_stack_add_next_term(machine, ticks,
                out_ticks);
        }
        if (opcode == 0xe3u) {
            *out_ticks = machine->executor_cpu.data.eip ==
                TYPE_MASK_UNSIGNED_16(data->oldcpu.data.eip + prefixes + 2u) ?
                (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ? 4u :
                    machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186 ? 5u :
                    6u) : (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ?
                    8u : 18u);
        } else {
            C_INT taken = machine->executor_cpu.data.eip !=
                TYPE_MASK_UNSIGNED_16(data->oldcpu.data.eip + prefixes + 2u);
            if (opcode == 0xe2u) {
                *out_ticks = taken ? (machine->cpu_profile ==
                    CORE_MACHINE_CPU_PROFILE_80286 ? 8u :
                    machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186 ? 15u :
                    17u) : (machine->cpu_profile ==
                    CORE_MACHINE_CPU_PROFILE_80286 ? 4u : 5u);
            } else if (opcode == 0xe1u) {
                *out_ticks = taken ? (machine->cpu_profile ==
                    CORE_MACHINE_CPU_PROFILE_80286 ? 8u :
                    machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186 ? 16u :
                    18u) : (machine->cpu_profile ==
                    CORE_MACHINE_CPU_PROFILE_80286 ? 4u : 6u);
            } else {
                *out_ticks = taken ? (machine->cpu_profile ==
                    CORE_MACHINE_CPU_PROFILE_80286 ? 8u :
                    machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186 ? 16u :
                    19u) : (machine->cpu_profile ==
                    CORE_MACHINE_CPU_PROFILE_80286 ? 4u : 5u);
            }
        }
        return 1;
    case 0xf4u:
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_HLT);
        return 1;
    case 0xccu: case 0xcdu:
        if (!same_privilege) return 0;
        if (protected_mode) return 0;
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            opcode == 0xccu ? CORE_MACHINE_SOURCE_TIMING_INT3 :
            CORE_MACHINE_SOURCE_TIMING_INT_IMMEDIATE);
        return machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ?
            core_machine_control_stack_add_next_term(machine, *out_ticks,
                out_ticks) : 1;
    case 0xceu:
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8086) {
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_INTO;
        }
        if ((data->oldcpu.data.eflags & VCPU_EFLAGS_OF) == 0u) {
            *out_ticks = machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ||
                machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 ? 3u : 4u;
            return 1;
        }
        if (!same_privilege) return 0;
        if (protected_mode) return 0;
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) {
            *out_ticks = 35u;
        } else if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286) {
            *out_ticks = 24u;
        } else {
            *out_ticks = machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186 ?
                48u : 53u;
        }
        return machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ?
            core_machine_control_stack_add_next_term(machine, *out_ticks,
                out_ticks) : 1;
    case 0xcfu:
        if (!same_privilege) return 0;
        if (protected_mode) return 0;
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_IRET);
        return machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ?
            core_machine_control_stack_add_next_term(machine, *out_ticks,
                out_ticks) : 1;
    case 0xffu:
        if (extension == 2u || extension == 4u) {
            *out_ticks = core_machine_control_stack_source_lookup(machine,
                memory ? (extension == 2u ? CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_MEMORY :
                    CORE_MACHINE_SOURCE_TIMING_JMP_MEMORY) :
                (extension == 2u ? CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_REGISTER :
                    CORE_MACHINE_SOURCE_TIMING_JMP_REGISTER));
            if (memory) {
                *out_ticks += core_machine_control_stack_memory_additions(
                    machine, data, prefixes, extension == 2u ? 2u : 1u);
            }
            return core_machine_control_stack_add_next_term(machine, *out_ticks,
                out_ticks);
        }
        if (extension == 3u || extension == 5u) {
            if (!memory || !same_privilege ||
                (protected_mode && (extension != 5u || data->crm !=
                    machine->executor_cpu.data.cs.selector))) return 0;
            *out_ticks = core_machine_control_stack_source_lookup(machine,
                extension == 3u ? CORE_MACHINE_SOURCE_TIMING_CALL_FAR_MEMORY :
                protected_mode ? CORE_MACHINE_SOURCE_TIMING_JMP_FAR_MEMORY_PROTECTED :
                CORE_MACHINE_SOURCE_TIMING_JMP_FAR_MEMORY);
            *out_ticks += core_machine_control_stack_memory_additions(machine,
                data, prefixes, extension == 3u ? 4u : 2u);
            return core_machine_control_stack_add_next_term(machine, *out_ticks,
                out_ticks);
        }
        if (extension == 6u) {
            *out_ticks = core_machine_control_stack_source_lookup(machine,
                memory ? CORE_MACHINE_SOURCE_TIMING_PUSH_MEMORY :
                CORE_MACHINE_SOURCE_TIMING_PUSH_REGISTER);
            if (memory) {
                *out_ticks += core_machine_control_stack_memory_additions(
                    machine, data, prefixes, 2u);
            }
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}

static type_unsigned_64 core_machine_80386_timing_signed_magnitude(
    type_unsigned_64 value, type_unsigned_8 bytes)
{
    type_unsigned_64 mask;
    type_unsigned_64 sign;

    if (bytes == 0u || bytes > sizeof(value)) return 0u;
    mask = bytes == sizeof(value) ? UINT64_MAX :
        (UINT64_C(1) << (bytes * 8u)) - 1u;
    value &= mask;
    sign = UINT64_C(1) << (bytes * 8u - 1u);
    return (value & sign) == 0u ? value : ((~value + 1u) & mask);
}

static type_unsigned_64 core_machine_80386_timing_ceiling_log2(
    type_unsigned_64 value)
{
    type_unsigned_64 result = 0u;

    while (value > 1u) {
        value = (value + 1u) >> 1u;
        ++result;
    }
    return result;
}

/* Intel 80386 PRM, IMUL/MUL timing tables: use the underlined optimizing
 * multiplier only.  `crm` and `cimm` are decoder-owned values captured during
 * the real execution; this timing path never rereads a register or memory. */
C_INT core_machine_80386_dynamic_multiply_cost(core_machine *machine,
    type_unsigned_64 *out_ticks)
{
    const t_cpuins_data *data;
    type_unsigned_32 prefixes;
    type_unsigned_8 opcode;
    type_unsigned_8 extension;
    type_unsigned_8 operand_bytes;
    type_unsigned_64 multiplier;
    type_unsigned_64 magnitude;
    type_unsigned_64 scale;
    C_INT signed_multiplier;
    C_INT memory_multiplier;

    if (machine == STD_NULL || out_ticks == STD_NULL ||
        machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80386) return 0;
    data = &machine->executor_cpu_instructions.data;
    prefixes = core_machine_instruction_prefix_count(data);
    if (prefixes + 1u >= data->oplen ||
        !core_machine_80386_timing_has_source_prefixes(data, prefixes)) {
        return 0;
    }
    opcode = data->opcodes[prefixes];
    extension = (data->opcodes[prefixes + 1u] >> 3u) & 7u;
    operand_bytes = data->oldcpu.data.cs.seg.exec.defsize ? 4u : 2u;
    if (data->prefix_oprsize) operand_bytes = operand_bytes == 4u ? 2u : 4u;
    signed_multiplier = TYPE_FALSE;
    memory_multiplier = TYPE_FALSE;

    if (opcode == 0xf6u || opcode == 0xf7u) {
        if (extension != 4u && extension != 5u) return 0;
        operand_bytes = opcode == 0xf6u ? 1u : operand_bytes;
        multiplier = data->crm;
        signed_multiplier = extension == 5u;
        memory_multiplier = core_machine_source_timing_modrm_is_memory(data,
            prefixes);
    } else if (opcode == 0x69u) {
        multiplier = data->crm;
        signed_multiplier = TYPE_TRUE;
        memory_multiplier = core_machine_source_timing_modrm_is_memory(data,
            prefixes);
    } else if (opcode == 0x6bu) {
        multiplier = data->cimm;
        operand_bytes = 1u;
        signed_multiplier = TYPE_TRUE;
    } else if (opcode == 0x0fu && prefixes + 2u < data->oplen &&
        data->opcodes[prefixes + 1u] == 0xafu) {
        multiplier = data->crm;
        signed_multiplier = TYPE_TRUE;
        memory_multiplier = core_machine_source_timing_modrm_is_memory(data,
            prefixes + 1u);
    } else {
        return 0;
    }
    magnitude = signed_multiplier ? core_machine_80386_timing_signed_magnitude(
        multiplier, operand_bytes) : multiplier &
        (operand_bytes == 4u ? UINT32_MAX :
            operand_bytes == 2u ? UINT16_MAX : UINT8_MAX);
    scale = core_machine_80386_timing_ceiling_log2(magnitude);
    *out_ticks = magnitude == 0u ? 9u : (scale < 3u ? 3u : scale) + 6u;
    if (memory_multiplier) *out_ticks += 3u;
    return 1;
}

/* Intel 80386 PRM section 17.2.2.3 supplies the fixed secondary-form rows
 * below.  The dynamic IMUL early-out remains in the dedicated helper above;
 * both helpers consume the decoder's completed operand capture after the one
 * successful-retirement publisher, never a handler-local timing decision. */
static type_unsigned_64 core_machine_80386_timing_zero_scan_count(
    type_unsigned_64 value, type_unsigned_8 operand_bytes, C_INT reverse)
{
    type_unsigned_64 mask;
    type_unsigned_64 bit;
    type_unsigned_64 count = 0u;
    type_unsigned_8 bits;

    bits = (type_unsigned_8)(operand_bytes * 8u);
    mask = operand_bytes == 4u ? UINT32_MAX : UINT16_MAX;
    value &= mask;
    if (value == 0u) return bits;
    bit = reverse ? UINT64_C(1) << (bits - 1u) : 1u;
    while ((value & bit) == 0u) {
        ++count;
        bit = reverse ? bit >> 1u : bit << 1u;
    }
    return count;
}

C_INT core_machine_80386_secondary_source_instruction_cost(
    core_machine *machine, type_unsigned_64 *out_ticks)
{
    const t_cpuins_data *data;
    core_machine_cpu_instruction_lexeme lexeme;
    type_unsigned_32 prefixes;
    type_unsigned_8 opcode;
    type_unsigned_8 secondary;
    type_unsigned_8 extension;
    type_unsigned_8 operand_bytes;
    type_unsigned_32 fallthrough;
    C_INT memory;

    if (machine == STD_NULL || out_ticks == STD_NULL ||
        machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80386) return 0;
    data = &machine->executor_cpu_instructions.data;
    prefixes = core_machine_instruction_prefix_count(data);
    if (prefixes + 1u >= data->oplen ||
        !core_machine_80386_timing_has_source_prefixes(data, prefixes)) {
        return 0;
    }
    opcode = data->opcodes[prefixes];
    if (opcode != 0x0fu) return 0;
    secondary = data->opcodes[prefixes + 1u];
    operand_bytes = data->oldcpu.data.cs.seg.exec.defsize ? 4u : 2u;
    if (data->prefix_oprsize) operand_bytes = operand_bytes == 4u ? 2u : 4u;

    if (secondary >= 0x80u && secondary <= 0x8fu) {
        fallthrough = data->oldcpu.data.eip + prefixes + 2u + operand_bytes;
        if (!data->oldcpu.data.cs.seg.exec.defsize) fallthrough &= 0xffffu;
        if (machine->executor_cpu.data.eip == fallthrough) {
            *out_ticks = CORE_MACHINE_80386_JCC_NOT_TAKEN_TICKS;
        } else if (core_machine_cpu_execution_preview_lexeme(
                &machine->executor_cpu_execution, &lexeme) && lexeme.available) {
            *out_ticks = CORE_MACHINE_80386_JCC_TAKEN_TICKS +
                lexeme.component_count;
        } else {
            core_machine_source_timing_mark_unallocated(machine, out_ticks);
        }
        return 1;
    }
    if (secondary < 0xa3u || prefixes + 2u >= data->oplen) return 0;
    memory = core_machine_source_timing_modrm_is_memory(data, prefixes + 1u);
    extension = (data->opcodes[prefixes + 2u] >> 3u) & 7u;
    switch (secondary) {
    case 0xa3u:
        *out_ticks = memory ? 12u : 3u;
        return 1;
    case 0xabu: case 0xb3u: case 0xbbu:
        *out_ticks = memory ? 13u : 6u;
        return 1;
    case 0xbau:
        if (extension == 4u) {
            *out_ticks = memory ? 6u : 3u;
            return 1;
        }
        if (extension >= 5u) {
            *out_ticks = memory ? 8u : 6u;
            return 1;
        }
        return 0;
    case 0xa4u: case 0xa5u: case 0xacu: case 0xadu:
        *out_ticks = memory ? 7u : 3u;
        return 1;
    case 0xb6u: case 0xb7u: case 0xbeu: case 0xbfu:
        *out_ticks = memory ? 6u : 3u;
        return 1;
    case 0xbcu: case 0xbdu:
        *out_ticks = 10u + 3u * core_machine_80386_timing_zero_scan_count(
            data->crm, operand_bytes, secondary == 0xbdu);
        return 1;
    default:
        return 0;
    }
}

/* Intel 80386 PRM section 17.2.2.3 gives the fixed successful-retirement
 * rows below for the system forms whose timing does not depend on a later
 * delivery, task switch, or descriptor-dependent outcome.  This keeps the
 * source row at the sole publisher: handlers, decoder, and delivery owners do
 * not acquire a second clock policy. */
C_INT core_machine_80386_privileged_source_instruction_cost(
    core_machine *machine, type_unsigned_64 *out_ticks)
{
    const t_cpuins_data *data;
    type_unsigned_32 prefixes;
    type_unsigned_8 opcode;
    type_unsigned_8 secondary;
    type_unsigned_8 modrm;
    type_unsigned_8 extension;
    C_INT memory;
    C_INT protected_mode;

    if (machine == STD_NULL || out_ticks == STD_NULL ||
        machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80386) return 0;
    data = &machine->executor_cpu_instructions.data;
    prefixes = core_machine_instruction_prefix_count(data);
    if (prefixes >= data->oplen || data->flagLock ||
        !core_machine_80386_timing_has_source_prefixes(data, prefixes)) {
        return 0;
    }
    opcode = data->opcodes[prefixes];
    protected_mode = core_machine_control_stack_is_protected(data);

    if (opcode == 0x63u) {
        if (!protected_mode || (data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ||
            prefixes + 1u >= data->oplen) return 0;
        *out_ticks = core_machine_source_timing_modrm_is_memory(data, prefixes) ?
            21u : 20u;
        return 1;
    }
    if (opcode != 0x0fu) return 0;
    if (prefixes + 1u >= data->oplen) return 0;
    secondary = data->opcodes[prefixes + 1u];
    if (secondary == 0x06u) {
        if ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ||
            (protected_mode && data->oldcpu.data.cs.dpl != 0u)) return 0;
        *out_ticks = 5u;
        return 1;
    }
    if (secondary == 0xa0u || secondary == 0xa8u) {
        *out_ticks = 2u;
        return 1;
    }
    if (secondary == 0xa1u || secondary == 0xa9u) {
        if ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u) return 0;
        *out_ticks = protected_mode ? 21u : 7u;
        return 1;
    }
    if (secondary == 0xb2u || secondary == 0xb4u || secondary == 0xb5u) {
        if (!protected_mode) {
            *out_ticks = 7u;
            return 1;
        }
        if ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u) return 0;
        *out_ticks = secondary == 0xb2u ? 22u : 25u;
        return 1;
    }
    if (prefixes + 2u >= data->oplen) return 0;
    modrm = data->opcodes[prefixes + 2u];
    extension = (modrm >> 3u) & 7u;
    memory = (modrm >> 6u) != 3u;

    switch (secondary) {
    case 0x00u:
        if (!protected_mode || (data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u) {
            return 0;
        }
        switch (extension) {
        case 0u:
            *out_ticks = 2u;
            return 1;
        case 1u:
            *out_ticks = memory ? 27u : 23u;
            return 1;
        case 2u:
            *out_ticks = 20u;
            return 1;
        case 3u:
            *out_ticks = memory ? 27u : 23u;
            return 1;
        case 4u:
            *out_ticks = memory ? 11u : 10u;
            return 1;
        case 5u:
            *out_ticks = memory ? 16u : 15u;
            return 1;
        default:
            return 0;
        }
    case 0x01u:
        switch (extension) {
        case 0u: case 1u:
            if (memory) {
                *out_ticks = 9u;
                return 1;
            }
            return 0;
        case 2u: case 3u:
            if ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ||
                (protected_mode && data->oldcpu.data.cs.dpl != 0u) || !memory) {
                return 0;
            }
            *out_ticks = 11u;
            return 1;
        case 4u:
            *out_ticks = protected_mode ? 2u : (memory ? 3u : 2u);
            return 1;
        case 6u:
            if ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ||
                (protected_mode && data->oldcpu.data.cs.dpl != 0u)) return 0;
            *out_ticks = memory ? 13u : 10u;
            return 1;
        default:
            return 0;
        }
    case 0x02u:
        if (!protected_mode || (data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u) {
            return 0;
        }
        *out_ticks = memory ? 16u : 15u;
        return 1;
    case 0x03u:
        if (!protected_mode || (data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ||
            !data->source_lsl_granularity_valid) {
            return 0;
        }
        if (data->source_lsl_page_granular) {
            *out_ticks = memory ? 26u : 25u;
        } else {
            *out_ticks = memory ? 21u : 20u;
        }
        return 1;
    case 0x20u:
        if ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ||
            (protected_mode && data->oldcpu.data.cs.dpl != 0u) ||
            (memory && !machine->cpu_80386_cr_mov_ignores_mod) ||
            (extension != 0u && extension != 2u && extension != 3u)) return 0;
        *out_ticks = 6u;
        return 1;
    case 0x21u:
        if ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ||
            (protected_mode && data->oldcpu.data.cs.dpl != 0u) || memory ||
            (extension != 0u && extension != 1u && extension != 2u &&
             extension != 3u && extension != 6u && extension != 7u)) return 0;
        *out_ticks = extension <= 3u ? 22u : 14u;
        return 1;
    case 0x22u:
        if ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ||
            (protected_mode && data->oldcpu.data.cs.dpl != 0u) ||
            (memory && !machine->cpu_80386_cr_mov_ignores_mod) ||
            (extension != 0u && extension != 2u && extension != 3u)) return 0;
        *out_ticks = extension == 0u ? 10u : (extension == 2u ? 4u : 5u);
        return 1;
    case 0x23u:
        if ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ||
            (protected_mode && data->oldcpu.data.cs.dpl != 0u) || memory ||
            (extension != 0u && extension != 1u && extension != 2u &&
             extension != 3u && extension != 6u && extension != 7u)) return 0;
        *out_ticks = extension <= 3u ? 22u : 16u;
        return 1;
    case 0x24u: case 0x26u:
        if ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ||
            (protected_mode && data->oldcpu.data.cs.dpl != 0u) || memory ||
            (extension != 6u && extension != 7u)) return 0;
        *out_ticks = 12u;
        return 1;
    default:
        return 0;
    }
}

C_INT core_machine_80286_source_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks)
{
    const t_cpuins_data *data = &machine->executor_cpu_instructions.data;
    type_unsigned_32 prefixes = core_machine_instruction_prefix_count(data);
    type_unsigned_8 opcode;
    type_unsigned_32 fallthrough;
    type_unsigned_64 memory_ticks;

    if (out_ticks == STD_NULL) return 0;
    if (prefixes >= data->oplen) {
        machine->source_repeat_active = TYPE_FALSE;
        *out_ticks = 0u;
        return 1;
    }
    opcode = data->opcodes[prefixes];
    machine->source_repeat_active = TYPE_FALSE;
    if (prefixes != 0u) {
        core_machine_source_timing_mark_unallocated(machine, out_ticks);
        return 1;
    }
    if (opcode >= 0x70u && opcode <= 0x7fu) {
        fallthrough = TYPE_MASK_UNSIGNED_16(data->oldcpu.data.eip + 2u);
        *out_ticks = machine->executor_cpu.data.eip == fallthrough ?
            CORE_MACHINE_80286_JCC_NOT_TAKEN_TICKS :
            CORE_MACHINE_80286_JCC_TAKEN_TICKS;
        return 1;
    }
    switch (opcode) {
    case 0x0fu:
        if (prefixes + 2u < data->oplen &&
            data->opcodes[prefixes + 1u] == 0x00u &&
            (((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 2u ||
             ((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 3u) &&
            core_machine_control_stack_is_protected(data) &&
            data->oldcpu.data.cs.dpl == 0u) {
            *out_ticks = data->flagMem ? 19u : 17u;
            return 1;
        }
        if (prefixes + 2u < data->oplen &&
            data->opcodes[prefixes + 1u] == 0x01u && data->flagMem &&
            (((data->opcodes[prefixes + 2u] >> 3u) & 7u) <= 3u) &&
            (!core_machine_control_stack_is_protected(data) ||
             data->oldcpu.data.cs.dpl == 0u)) {
            *out_ticks = ((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 0u ||
                ((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 2u ? 11u : 12u;
            return 1;
        }
        if (prefixes + 1u < data->oplen && data->opcodes[prefixes + 1u] == 0x06u &&
            (!core_machine_control_stack_is_protected(data) ||
             data->oldcpu.data.cs.dpl == 0u)) {
            *out_ticks = 2u;
            return 1;
        }
        if (prefixes + 2u < data->oplen &&
            data->opcodes[prefixes + 1u] == 0x00u &&
            ((((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 4u) ||
             (((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 5u)) &&
            core_machine_control_stack_is_protected(data)) {
            *out_ticks = data->flagMem ? 16u : 14u;
            return 1;
        }
        if (prefixes + 2u < data->oplen &&
            ((data->opcodes[prefixes + 1u] == 0x01u &&
             (((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 4u ||
              (((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 6u &&
               (!core_machine_control_stack_is_protected(data) ||
                data->oldcpu.data.cs.dpl == 0u)))) ||
            (core_machine_control_stack_is_protected(data) &&
             ((data->opcodes[prefixes + 1u] == 0x00u &&
               ((data->opcodes[prefixes + 2u] >> 3u) & 7u) <= 1u) ||
              data->opcodes[prefixes + 1u] == 0x02u ||
              data->opcodes[prefixes + 1u] == 0x03u)))) {
            *out_ticks = data->opcodes[prefixes + 1u] == 0x01u &&
                ((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 6u ?
                (data->flagMem ? 6u : 3u) :
                (data->opcodes[prefixes + 1u] == 0x02u ||
                data->opcodes[prefixes + 1u] == 0x03u ?
                (data->flagMem ? 16u : 14u) : (data->flagMem ? 3u : 2u));
            return 1;
        }
        core_machine_source_timing_mark_unallocated(machine, out_ticks);
        return 1;
    case 0x90u:
        *out_ticks = core_machine_80286_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_NOP);
        return 1;
    case 0xf8u:
        *out_ticks = core_machine_80286_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_CLC);
        return 1;
    case 0xf5u: case 0xf9u: case 0xfcu: case 0xfdu:
        *out_ticks = 2u;
        return 1;
    case 0xfau:
        *out_ticks = 3u;
        return 1;
    case 0xfbu:
        *out_ticks = 2u;
        return 1;
    case 0xd7u:
        *out_ticks = 5u;
        return 1;
    case 0x9eu: case 0x9fu:
        *out_ticks = 2u;
        return 1;
    case 0x06u: case 0x0eu: case 0x16u: case 0x1eu:
        *out_ticks = 3u;
        return 1;
    case 0x07u: case 0x17u: case 0x1fu:
        *out_ticks = 5u;
        return 1;
    case 0x62u:
        if (data->flagMem) {
            *out_ticks = 13u;
            return 1;
        }
        core_machine_source_timing_mark_unallocated(machine, out_ticks);
        return 1;
    case 0x63u:
        if (core_machine_control_stack_is_protected(data)) {
            *out_ticks = data->flagMem ? 11u : 10u;
            return 1;
        }
        core_machine_source_timing_mark_unallocated(machine, out_ticks);
        return 1;
    case 0x8cu:
        if (prefixes + 1u < data->oplen &&
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) <= 3u) {
            *out_ticks = data->flagMem ? 3u +
                core_machine_80286_timing_effective_address(data, prefixes) +
                core_machine_80286_timing_odd_word(data) : 2u;
            return 1;
        }
        core_machine_source_timing_mark_unallocated(machine, out_ticks);
        return 1;
    case 0x8eu:
        if (prefixes + 1u < data->oplen &&
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) != 1u &&
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) <= 3u) {
            *out_ticks = core_machine_control_stack_is_protected(data) ?
                (data->flagMem ? 19u : 17u) : (data->flagMem ? 5u : 2u);
            if (data->flagMem) *out_ticks +=
                core_machine_80286_timing_effective_address(data, prefixes) +
                core_machine_80286_timing_odd_word(data);
            return 1;
        }
        core_machine_source_timing_mark_unallocated(machine, out_ticks);
        return 1;
    case 0xc4u: case 0xc5u:
        if (data->flagMem) {
            *out_ticks = 7u;
            return 1;
        }
        core_machine_source_timing_mark_unallocated(machine, out_ticks);
        return 1;
    case 0x88u: case 0x89u: case 0x8au: case 0x8bu:
        if (!data->flagMem) {
            *out_ticks = core_machine_80286_source_timing_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_REGISTER);
            return 1;
        }
        memory_ticks = core_machine_80286_timing_effective_address(data,
            prefixes);
        *out_ticks = core_machine_80286_source_timing_lookup(machine,
            opcode == 0x88u || opcode == 0x89u ?
            CORE_MACHINE_SOURCE_TIMING_MOV_RM_REGISTER :
            CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_RM) + memory_ticks +
            ((opcode == 0x89u || opcode == 0x8bu) ?
                core_machine_80286_timing_odd_word(data) : 0u);
        return 1;
    case 0xa0u: case 0xa1u: case 0xa2u: case 0xa3u:
        *out_ticks = core_machine_80286_source_timing_lookup(machine,
            opcode == 0xa0u || opcode == 0xa1u ?
            CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_READ :
            CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_WRITE) +
            ((opcode == 0xa1u || opcode == 0xa3u) ?
                core_machine_80286_timing_odd_word(data) : 0u);
        return 1;
    case 0xd0u: case 0xd1u:
        if (!data->flagMem && prefixes + 1u < data->oplen &&
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) != 6u) {
            *out_ticks = 2u;
            return 1;
        }
        if (data->flagMem && prefixes + 1u < data->oplen &&
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) != 6u) {
            *out_ticks = 7u + core_machine_80286_timing_effective_address(
                data, prefixes);
            return 1;
        }
        core_machine_source_timing_mark_unallocated(machine, out_ticks);
        return 1;
    case 0xc0u: case 0xc1u: case 0xd2u: case 0xd3u:
        if (prefixes + 1u < data->oplen &&
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) != 6u) {
            memory_ticks = core_machine_80286_group2_count(data, prefixes,
                opcode);
            *out_ticks = data->flagMem ? 8u + memory_ticks +
                core_machine_80286_timing_effective_address(data, prefixes) :
                5u + memory_ticks;
            return 1;
        }
        core_machine_source_timing_mark_unallocated(machine, out_ticks);
        return 1;
    default:
        if (opcode >= 0xb0u && opcode <= 0xbfu) {
            *out_ticks = core_machine_80286_source_timing_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_MOV_IMMEDIATE);
        } else {
            core_machine_source_timing_mark_unallocated(machine, out_ticks);
        }
        return 1;
    }
}

C_INT core_machine_80386_source_instruction_cost(core_machine *machine,
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
    machine->source_repeat_active = TYPE_FALSE;
    if (prefixes != 0u && !(prefixes == 1u && data->opcodes[0] == 0x66u &&
            (opcode == 0xd1u || opcode == 0xd3u))) {
        core_machine_source_timing_mark_unallocated(machine, out_ticks);
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
            core_machine_source_timing_mark_unallocated(machine, out_ticks);
        }
        return 1;
    }
    switch (opcode) {
    case 0x90u:
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_NOP);
        return 1;
    case 0xd0u:
        if (prefixes + 1u < data->oplen &&
            !core_machine_source_timing_modrm_is_memory(data, prefixes) &&
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) == 4u) {
            *out_ticks = core_machine_80386_source_timing_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_SAL_REGISTER_ONE);
        } else {
            core_machine_source_timing_mark_unallocated(machine, out_ticks);
        }
        return 1;
    case 0xd1u:
        if (prefixes == 1u && data->opcodes[0] == 0x66u &&
            prefixes + 1u < data->oplen &&
            !core_machine_source_timing_modrm_is_memory(data, prefixes) &&
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) == 2u) {
            *out_ticks = core_machine_80386_source_timing_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_RCL_REGISTER_ONE_32);
        } else {
            core_machine_source_timing_mark_unallocated(machine, out_ticks);
        }
        return 1;
    case 0xd3u:
        if (prefixes == 1u && data->opcodes[0] == 0x66u &&
            prefixes + 1u < data->oplen &&
            !core_machine_source_timing_modrm_is_memory(data, prefixes) &&
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) == 2u) {
            *out_ticks = core_machine_80386_source_timing_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_RCL_REGISTER_CL_32);
        } else {
            core_machine_source_timing_mark_unallocated(machine, out_ticks);
        }
        return 1;
    case 0xf8u:
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_CLC);
        return 1;
    case 0xfcu:
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_CLD);
        return 1;
    case 0xfau:
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_CLI);
        return 1;
    case 0x9eu:
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_SAHF);
        return 1;
    case 0x9fu:
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_LAHF);
        return 1;
    case 0x8eu:
        if (!core_machine_source_timing_modrm_is_memory(data, prefixes)) {
            *out_ticks = core_machine_80386_source_timing_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_MOV_SREG_REGISTER);
        } else if (!core_machine_control_stack_is_protected(data)) {
            *out_ticks = core_machine_80386_source_timing_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_MOV_SREG_MEMORY);
        } else {
            core_machine_source_timing_mark_unallocated(machine, out_ticks);
        }
        return 1;
    case 0x88u: case 0x89u:
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            core_machine_source_timing_modrm_is_memory(data, prefixes) ?
            CORE_MACHINE_SOURCE_TIMING_MOV_RM_REGISTER :
            CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_REGISTER);
        return 1;
    case 0x8au: case 0x8bu:
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            core_machine_source_timing_modrm_is_memory(data, prefixes) ?
            CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_RM :
            CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_REGISTER);
        return 1;
    case 0xa0u: case 0xa1u:
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_READ);
        return 1;
    case 0xa2u: case 0xa3u:
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_WRITE);
        return 1;
    default:
        if (opcode >= 0xb0u && opcode <= 0xbfu) {
            *out_ticks = core_machine_80386_source_timing_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_MOV_IMMEDIATE);
        } else {
            core_machine_source_timing_mark_unallocated(machine, out_ticks);
        }
        return 1;
    }
    core_machine_source_timing_mark_unallocated(machine, out_ticks);
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

C_INT core_machine_compatibility_instruction_cost(core_machine *machine,
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

static C_VOID core_machine_external_cycle_invalidate(core_machine *machine);

static C_VOID core_machine_transaction_trace(C_VOID *opaque,
    core_machine_transaction_owner owner, core_machine_transaction_kind kind,
    core_machine_transaction_phase phase, type_unsigned_32 address,
    type_unsigned_32 value, type_unsigned_32 detail)
{
    core_machine *machine = (core_machine *)opaque;
    core_machine_trace_event_type type;

    if (machine == STD_NULL) return;
    /* Generic-AT policy: an acknowledged DMA bus handoff breaks CPU-side
     * locality. D4 establishes the HOLD/HLDA topology, not this page-retention
     * behavior or any physical phase duration. */
    if (phase == CORE_MACHINE_TRANSACTION_PHASE_HOLD_ACKNOWLEDGE &&
        owner == CORE_MACHINE_TRANSACTION_OWNER_DMA) {
        core_machine_external_cycle_invalidate(machine);
    }
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
    case CORE_MACHINE_TRANSACTION_PHASE_HOLD_REQUEST:
        type = CORE_MACHINE_TRACE_TRANSACTION_HOLD_REQUEST;
        break;
    case CORE_MACHINE_TRANSACTION_PHASE_HOLD_ACKNOWLEDGE:
        type = CORE_MACHINE_TRACE_TRANSACTION_HOLD_ACKNOWLEDGE;
        break;
    case CORE_MACHINE_TRANSACTION_PHASE_HOLD_RELEASE:
        type = CORE_MACHINE_TRACE_TRANSACTION_HOLD_RELEASE;
        break;
    default:
        return;
    }
    core_machine_trace_record(machine, type, address, value,
        (type_unsigned_32)owner | ((type_unsigned_32)kind << 8u) |
        (detail << 16u));
}

static C_INT core_machine_external_cycle_access_is_chargeable(type_bool write,
    core_machine_cpu_memory_access_provenance provenance)
{
    return (!write && (provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH ||
        provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_DATA ||
        provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ)) ||
        (write && (provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_DATA ||
        provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE));
}

static C_INT core_machine_external_cycle_pending_matches(const core_machine *machine,
    core_machine_cpu_external_cycle_space space, type_unsigned_32 address,
    type_unsigned_8 bytes, type_bool write,
    core_machine_cpu_memory_access_provenance provenance)
{
    return machine != STD_NULL && machine->external_cycle_pending_valid &&
        machine->external_cycle_pending_space == space &&
        machine->external_cycle_pending_physical == address &&
        machine->external_cycle_pending_bytes == bytes &&
        machine->external_cycle_pending_write == write &&
        machine->external_cycle_pending_provenance == provenance;
}

static type_unsigned_32 core_machine_external_access_wait_ticks(
    const core_machine *machine, core_machine_cpu_external_cycle_space space,
    type_unsigned_32 address)
{
    STD_SIZE_T index;

    if (machine == STD_NULL) return 0u;
    for (index = 0u; index < CORE_MACHINE_EXTERNAL_ACCESS_WAIT_WINDOW_CAPACITY;
            ++index) {
        const core_machine_external_access_wait_window *window =
            &machine->external_access_wait_windows[index];
        if (window->wait_ticks != 0u && window->space == space &&
            address >= window->first_address && address <= window->last_address) {
            return window->wait_ticks;
        }
    }
    return 0u;
}

static C_VOID core_machine_external_cycle_invalidate(core_machine *machine)
{
    if (machine == STD_NULL) return;
    machine->external_cycle_page_valid = TYPE_FALSE;
    machine->external_cycle_pending_valid = TYPE_FALSE;
    machine->external_cycle_overlap_valid = TYPE_FALSE;
}

static C_VOID core_machine_cpu_external_cycle_trace(C_VOID *opaque,
    core_machine_cpu_external_cycle_phase phase,
    core_machine_cpu_external_cycle_space space, type_unsigned_32 address,
    type_unsigned_8 bytes, type_bool write,
    core_machine_cpu_memory_access_provenance provenance)
{
    core_machine *machine = (core_machine *)opaque;
    core_machine_trace_event_type type;
    C_INT pending_matches;
    type_bool page_timing_enabled;

    if (machine == STD_NULL) return;
    page_timing_enabled = space == CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY &&
        machine->external_cycle_timing.page_bytes != 0u &&
        ((machine->external_cycle_timing.first_eligible_address == 0u &&
          machine->external_cycle_timing.last_eligible_address == 0u) ||
         (address >= machine->external_cycle_timing.first_eligible_address &&
          address <= machine->external_cycle_timing.last_eligible_address));
    switch (phase) {
    case CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN:
        if (machine->external_cycle_pending_valid) {
            core_machine_external_cycle_invalidate(machine);
        } else if (page_timing_enabled && machine->external_cycle_overlap_valid &&
            machine->external_cycle_overlap_next_physical != address) {
            machine->external_cycle_overlap_valid = TYPE_FALSE;
        }
        machine->external_cycle_pending_valid = TYPE_TRUE;
        machine->external_cycle_pending_space = space;
        machine->external_cycle_pending_physical = address;
        machine->external_cycle_pending_bytes = bytes;
        machine->external_cycle_pending_write = write;
        machine->external_cycle_pending_provenance = provenance;
        type = CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_BEGIN;
        break;
    case CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_OVERLAP_DECLARE:
        if (page_timing_enabled && machine->external_cycle_timing.overlap_policy ==
                CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL &&
            machine->external_cycle_pending_valid &&
            machine->external_cycle_pending_space ==
                CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY &&
            !machine->external_cycle_pending_write && !write &&
            machine->external_cycle_pending_provenance ==
                CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH &&
            provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH &&
            machine->external_cycle_pending_physical <= UINT32_MAX -
                machine->external_cycle_pending_bytes &&
            address == machine->external_cycle_pending_physical +
                machine->external_cycle_pending_bytes) {
            machine->external_cycle_overlap_valid = TYPE_TRUE;
            machine->external_cycle_overlap_next_physical = address;
        }
        type = CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_OVERLAP_DECLARE;
        break;
    case CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT:
        pending_matches = core_machine_external_cycle_pending_matches(machine,
            space, address, bytes, write, provenance);
        if (pending_matches && page_timing_enabled &&
            core_machine_external_cycle_access_is_chargeable(write, provenance)) {
            type_unsigned_32 page_tag = address / machine->external_cycle_timing.page_bytes;
            type_unsigned_32 wait_ticks = !machine->external_cycle_page_valid ||
                !machine->external_cycle_overlap_valid ||
                machine->external_cycle_overlap_next_physical != address ||
                machine->external_cycle_page_tag != page_tag ?
                machine->external_cycle_timing.page_miss_ticks :
                machine->external_cycle_timing.page_hit_ticks;
            if (machine->external_cycle_overlap_valid &&
                machine->external_cycle_overlap_next_physical == address) {
                machine->external_cycle_overlap_valid = TYPE_FALSE;
            }
            machine->external_cycle_page_valid = TYPE_TRUE;
            machine->external_cycle_page_tag = page_tag;
            if (UINT64_MAX - machine->external_cycle_round_ticks < wait_ticks) {
                machine->external_cycle_round_overflow = TYPE_TRUE;
            } else {
                machine->external_cycle_round_ticks += wait_ticks;
            }
        }
        if (pending_matches) {
            type_unsigned_32 wait_ticks = core_machine_external_access_wait_ticks(
                machine, space, address);
            if (UINT64_MAX - machine->external_cycle_round_ticks < wait_ticks) {
                machine->external_cycle_round_overflow = TYPE_TRUE;
            } else {
                machine->external_cycle_round_ticks += wait_ticks;
            }
            machine->external_cycle_pending_valid = TYPE_FALSE;
        }
        type = CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_COMMIT;
        break;
    case CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL:
        if (core_machine_external_cycle_pending_matches(machine, space, address,
                bytes, write, provenance)) {
            core_machine_external_cycle_invalidate(machine);
        }
        type = CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_CANCEL;
        break;
    default:
        return;
    }
    core_machine_trace_record(machine, type, address, bytes,
        (type_unsigned_32)provenance);
}static C_INT core_machine_retirement_qualification_contains(
    const core_machine *machine)
{
    STD_SIZE_T index;

    if (machine == STD_NULL || !machine->retirement_eligibility_key_valid) return 0;
    for (index = 0u; index < machine->retirement_qualification_count; ++index) {
        const core_machine_retirement_eligibility_key *candidate =
            &machine->retirement_qualification[index];
        const core_machine_retirement_eligibility_key *key =
            &machine->retirement_eligibility_key;
        if (candidate->cpu_profile == key->cpu_profile &&
            candidate->timing_origin == key->timing_origin &&
            candidate->source_timing_form_id == key->source_timing_form_id &&
            candidate->opcode == key->opcode &&
            candidate->escape_opcode == key->escape_opcode &&
            candidate->modrm_form == key->modrm_form &&
            candidate->modrm_extension == key->modrm_extension &&
            candidate->control_outcome == key->control_outcome &&
            candidate->next_lexeme_components == key->next_lexeme_components &&
            candidate->repeat_phase == key->repeat_phase &&
            candidate->cpl == key->cpl &&
            candidate->protected_mode == key->protected_mode &&
            candidate->virtual_8086_mode == key->virtual_8086_mode &&
            candidate->operand_size_32 == key->operand_size_32 &&
            candidate->address_size_32 == key->address_size_32 &&
            candidate->lock_prefix == key->lock_prefix &&
            candidate->repeat_prefix == key->repeat_prefix) {
            return 1;
        }
    }
    return 0;
}
static C_INT core_machine_retirement_time_contract_is_valid(
    core_machine_retirement_time_contract contract)
{
    return contract == CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC ||
        contract == CORE_MACHINE_RETIREMENT_TIME_PHYSICAL;
}

static C_INT core_machine_timing_capability_is_valid(
    core_machine_timing_capability capability)
{
    return capability >= CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC &&
        capability <= CORE_MACHINE_TIMING_CAPABILITY_PRODUCT_DEBUG;
}

static C_INT core_machine_timing_disposition_is_valid(
    core_machine_timing_disposition disposition)
{
    return disposition == CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK ||
        disposition == CORE_MACHINE_TIMING_DISPOSITION_NON_GUEST_TIME ||
        disposition == CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
}

static C_INT core_machine_timing_capability_is_non_guest_time(
    core_machine_timing_capability capability)
{
    return capability == CORE_MACHINE_TIMING_CAPABILITY_DISPLAY_PRESENT ||
        capability == CORE_MACHINE_TIMING_CAPABILITY_INPUT_HOST ||
        capability == CORE_MACHINE_TIMING_CAPABILITY_TRACE_DEBUG ||
        capability == CORE_MACHINE_TIMING_CAPABILITY_PLATFORM_MAILBOX ||
        capability == CORE_MACHINE_TIMING_CAPABILITY_PLATFORM_RESOURCE ||
        capability == CORE_MACHINE_TIMING_CAPABILITY_PLATFORM_WAIT ||
        capability == CORE_MACHINE_TIMING_CAPABILITY_SESSION_COMMAND ||
        capability == CORE_MACHINE_TIMING_CAPABILITY_PRODUCT_DEBUG;
}

static core_machine_timing_seam core_machine_timing_capability_seam(
    core_machine_timing_capability capability)
{
    if (capability == CORE_MACHINE_TIMING_CAPABILITY_CPU_RETIRE) {
        return CORE_MACHINE_TIMING_SEAM_RETIREMENT;
    }
    if (capability <= CORE_MACHINE_TIMING_CAPABILITY_CPU_FPU) {
        return CORE_MACHINE_TIMING_SEAM_CPU_PROGRAM;
    }
    if (capability == CORE_MACHINE_TIMING_CAPABILITY_TIME_CLOCK) {
        return CORE_MACHINE_TIMING_SEAM_CLOCK;
    }
    if (capability == CORE_MACHINE_TIMING_CAPABILITY_TIME_LIFECYCLE) {
        return CORE_MACHINE_TIMING_SEAM_LIFECYCLE;
    }
    if (capability <= CORE_MACHINE_TIMING_CAPABILITY_TXN_ARBITRATION) {
        return CORE_MACHINE_TIMING_SEAM_TRANSACTION;
    }
    if (capability <= CORE_MACHINE_TIMING_CAPABILITY_MEM_ROM_FIRMWARE) {
        return CORE_MACHINE_TIMING_SEAM_MEMORY;
    }
    if (capability == CORE_MACHINE_TIMING_CAPABILITY_MACHINE_CONFIG) {
        return CORE_MACHINE_TIMING_SEAM_CONFIGURATION;
    }
    if (capability <= CORE_MACHINE_TIMING_CAPABILITY_DISPLAY_VADP) {
        return CORE_MACHINE_TIMING_SEAM_DEVICE;
    }
    return CORE_MACHINE_TIMING_SEAM_OBSERVATION;
}

static type_status core_machine_plan_validate(const core_machine_plan *plan)
{
    type_bool seen[CORE_MACHINE_TIMING_CAPABILITY_COUNT] = {0};
    STD_SIZE_T index;

    if (plan == STD_NULL || plan->declaration_count !=
        CORE_MACHINE_TIMING_CAPABILITY_COUNT) return TYPE_STATUS_INVALID_ARGUMENT;
    if ((plan->topology.absent_memory_present != TYPE_FALSE &&
         plan->topology.absent_memory_present != TYPE_TRUE) ||
        (plan->topology.planar_parity_present != TYPE_FALSE &&
         plan->topology.planar_parity_present != TYPE_TRUE) ||
        (plan->topology.d4_platform_present != TYPE_FALSE &&
         plan->topology.d4_platform_present != TYPE_TRUE) ||
        (plan->topology.display_present != TYPE_FALSE &&
         plan->topology.display_present != TYPE_TRUE) ||
        (plan->topology.dma_present != TYPE_FALSE &&
         plan->topology.dma_present != TYPE_TRUE) ||
        (plan->topology.rtc_cmos_present != TYPE_FALSE &&
         plan->topology.rtc_cmos_present != TYPE_TRUE) ||
        (plan->topology.fdc_present != TYPE_FALSE &&
         plan->topology.fdc_present != TYPE_TRUE) ||
        (plan->topology.hdc_present != TYPE_FALSE &&
         plan->topology.hdc_present != TYPE_TRUE) ||
        (plan->topology.fdc_present && !plan->topology.dma_present) ||
        (plan->topology.hdc_present && !plan->topology.fdc_present &&
         plan->topology.hdc.config.protocol ==
             CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB) ||
        plan->topology.memory_device_count > CORE_MACHINE_PLAN_MEMORY_DEVICE_COUNT ||
        (plan->topology.d4_memory_parity_present != TYPE_FALSE &&
         plan->topology.d4_memory_parity_present != TYPE_TRUE) ||
        (plan->topology.d4_memory_parity_present &&
         (!plan->topology.d4_platform_present ||
          plan->topology.d4_memory_parity_mask == STD_NULL))) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    for (index = 0u; index < plan->declaration_count; ++index) {
        const core_machine_timing_declaration *declaration =
            &plan->declarations[index];

        if (!core_machine_timing_capability_is_valid(declaration->capability) ||
            !core_machine_timing_disposition_is_valid(declaration->disposition) ||
            declaration->seam != core_machine_timing_capability_seam(
                declaration->capability) || seen[declaration->capability]) {
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
        if (core_machine_timing_capability_is_non_guest_time(
                declaration->capability)) {
            if (declaration->disposition !=
                CORE_MACHINE_TIMING_DISPOSITION_NON_GUEST_TIME) {
                return TYPE_STATUS_INVALID_ARGUMENT;
            }
        } else if (declaration->disposition !=
            CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK) {
            /* S1 has no registered L3 rule; it must reject such a request. */
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
        seen[declaration->capability] = TYPE_TRUE;
    }
    for (index = 0u; index < CORE_MACHINE_TIMING_CAPABILITY_COUNT; ++index) {
        if (!seen[index]) return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return TYPE_STATUS_OK;
}

static C_VOID core_machine_plan_d4_parity_fault(C_VOID *owner,
    type_unsigned_32 physical)
{
    core_machine *machine = (core_machine *)owner;

    if (machine == STD_NULL || machine->d4_plan_parity_mask == STD_NULL) return;
    *machine->d4_plan_parity_mask |= (type_unsigned_8)(1u << (physical & 3u));
    (C_VOID)core_machine_report_d4_iochk_fault(machine);
}

static C_VOID core_machine_plan_d4_memory_write(C_VOID *owner,
    type_unsigned_32 physical, type_native_unsigned bytes)
{
    (C_VOID)physical;
    (C_VOID)bytes;
    (C_VOID)core_machine_clear_d4_iochk_fault((core_machine *)owner);
}

static type_status core_machine_plan_apply_topology(core_machine *machine,
    const core_machine_plan_topology *topology)
{
    core_machine_fdc_topology fdc;
    type_status status;
    STD_SIZE_T index;

    if (machine == STD_NULL || topology == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (topology->absent_memory_present && (status = core_machine_configure_absent_memory(
            machine, &topology->absent_memory)) != TYPE_STATUS_OK) return status;
    for (index = 0u; index < topology->memory_device_count; ++index) {
        const core_machine_plan_memory_device *device = &topology->memory_devices[index];

        if ((status = core_machine_register_memory_device(machine,
                device->physical_start, device->bytes, &device->callbacks,
                device->owner)) != TYPE_STATUS_OK) return status;
    }
    if (topology->planar_parity_present && (status = core_machine_configure_planar_parity(
            machine, &topology->planar_parity)) != TYPE_STATUS_OK) return status;
    if (topology->d4_platform_present && (status = core_machine_configure_d4_platform(
            machine, &topology->d4_platform)) != TYPE_STATUS_OK) return status;
    if (topology->d4_memory_parity_present) {
        machine->d4_plan_parity_mask = topology->d4_memory_parity_mask;
        status = core_machine_enable_memory_parity(machine, 1024u * 1024u,
            core_machine_plan_d4_parity_fault, machine);
        if (status != TYPE_STATUS_OK) return status;
        status = core_machine_register_memory_write_observer(machine,
            core_machine_plan_d4_memory_write, machine);
        if (status != TYPE_STATUS_OK) return status;
    }
    if (topology->display_present && (status = core_machine_configure_display(
            machine, &topology->display)) != TYPE_STATUS_OK) return status;
    if (topology->dma_present && (status = core_machine_configure_dma(machine,
            &topology->dma, &machine->fdc_dma_request)) != TYPE_STATUS_OK) return status;
    if (topology->rtc_cmos_present && (status = core_machine_configure_rtc_cmos(
            machine, &topology->rtc_cmos)) != TYPE_STATUS_OK) return status;
    if (topology->fdc_present) {
        fdc = topology->fdc;
        fdc.dma_request = machine->fdc_dma_request;
        if ((status = core_machine_configure_fdc(machine, &fdc)) != TYPE_STATUS_OK) {
            return status;
        }
    }
    if (topology->hdc_present && (status = core_machine_configure_hdc(machine,
            &topology->hdc)) != TYPE_STATUS_OK) return status;
    return TYPE_STATUS_OK;
}

static const core_machine_timing_declaration *
core_machine_plan_declaration_find(const core_machine_plan *plan,
    core_machine_timing_capability capability)
{
    STD_SIZE_T index;

    if (plan == STD_NULL) return STD_NULL;
    for (index = 0u; index < plan->declaration_count; ++index) {
        if (plan->declarations[index].capability == capability) {
            return &plan->declarations[index];
        }
    }
    return STD_NULL;
}

C_VOID core_machine_plan_initialize(core_machine_plan *out_plan,
    const core_machine_config *configuration)
{
    STD_SIZE_T index;

    if (out_plan == STD_NULL) return;
    STD_MEMSET(out_plan, 0, sizeof(*out_plan));
    if (configuration != STD_NULL) out_plan->configuration = *configuration;
    out_plan->declaration_count = CORE_MACHINE_TIMING_CAPABILITY_COUNT;
    for (index = 0u; index < out_plan->declaration_count; ++index) {
        core_machine_timing_capability capability =
            (core_machine_timing_capability)index;

        out_plan->declarations[index].capability = capability;
        out_plan->declarations[index].seam =
            core_machine_timing_capability_seam(capability);
        out_plan->declarations[index].disposition =
            core_machine_timing_capability_is_non_guest_time(capability) ?
            CORE_MACHINE_TIMING_DISPOSITION_NON_GUEST_TIME :
            CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK;
    }
}
static C_INT core_machine_external_cycle_timing_is_valid(
    const core_machine_external_cycle_timing *timing)
{
    if (timing == STD_NULL || (timing->overlap_policy !=
        CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED && timing->overlap_policy !=
        CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL)) return 0;
    if (timing->page_bytes == 0u) {
        return timing->page_miss_ticks == 0u && timing->page_hit_ticks == 0u &&
            timing->overlap_policy == CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED;
    }
    return (timing->page_bytes & (timing->page_bytes - 1u)) == 0u &&
        ((timing->first_eligible_address == 0u &&
          timing->last_eligible_address == 0u) ||
         timing->first_eligible_address <= timing->last_eligible_address);
}
static C_INT core_machine_external_access_wait_windows_are_valid(
    const core_machine_external_access_wait_window *windows)
{
    STD_SIZE_T index;

    if (windows == STD_NULL) return 0;
    for (index = 0u; index < CORE_MACHINE_EXTERNAL_ACCESS_WAIT_WINDOW_CAPACITY;
            ++index) {
        const core_machine_external_access_wait_window *window = &windows[index];
        if (window->wait_ticks == 0u) continue;
        if ((window->space != CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY &&
                window->space != CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT) ||
            window->first_address > window->last_address ||
            (window->space == CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT &&
                window->last_address > 0xffffu)) return 0;
    }
    return 1;
}static C_INT core_machine_clock_plan_is_valid(
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
static C_VOID core_machine_dma_grant_advance(core_machine *machine)
{
    if (machine == STD_NULL) return;
    if ((machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ||
        machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) &&
        core_machine_dma_has_pending_request(&machine->shared_dma_primary,
            &machine->shared_dma_secondary) &&
        core_machine_transaction_hold_request(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) == TYPE_STATUS_OK) {
        if (core_machine_transaction_hold_acknowledge(&machine->transaction,
                CORE_MACHINE_TRANSACTION_OWNER_DMA) == TYPE_STATUS_OK) {
            core_machine_dma_advance_transaction(&machine->shared_dma_latch,
                &machine->shared_dma_primary, &machine->shared_dma_secondary,
                &machine->executor_memory, &machine->transaction, 1u);
        }
        core_machine_transaction_hold_release(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA);
    } else {
        core_machine_dma_advance_transaction(&machine->shared_dma_latch,
            &machine->shared_dma_primary, &machine->shared_dma_secondary,
            &machine->executor_memory, &machine->transaction, 1u);
    }
}
static C_VOID core_machine_d4_refresh_hold_advance(core_machine *machine)
{
    if (machine == STD_NULL || !machine->d4_refresh_hold_pending) return;
    if (core_machine_transaction_hold_request(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_REFRESH, machine->d4_refresh_address) !=
        TYPE_STATUS_OK) return;
    if (core_machine_transaction_hold_acknowledge(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_REFRESH) == TYPE_STATUS_OK &&
        core_machine_transaction_begin(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_REFRESH,
            CORE_MACHINE_TRANSACTION_REFRESH_MEMORY_CYCLE,
            machine->d4_refresh_address, 0u, 0u) == TYPE_STATUS_OK) {
        /* Bus occupation only: Core has no DRAM electrical refresh model. */
        core_machine_transaction_commit(&machine->transaction);
        machine->d4_refresh_address = (type_unsigned_8)(machine->d4_refresh_address + 1u);
        machine->d4_refresh_hold_pending = TYPE_FALSE;
    }
    core_machine_transaction_hold_release(&machine->transaction,
        CORE_MACHINE_TRANSACTION_OWNER_REFRESH);
}
static C_VOID core_machine_arbitration_tick(C_VOID *opaque,
    type_unsigned_64 due_tick)
{
    core_machine *machine = (core_machine *)opaque;
    type_unsigned_64 dma_ticks;
    type_unsigned_64 pit_ticks;
    type_bool refresh_pending;
    core_machine_timeline_token next;

    if (machine == STD_NULL) {
        return;
    }
    dma_ticks = core_machine_clock_domain_advance(&machine->dma_clock, 1u);
    pit_ticks = core_machine_clock_domain_advance(&machine->pit_clock, 1u);
    refresh_pending = machine->d4_refresh_hold_pending;
    core_machine_d4_refresh_hold_advance(machine);
    if (machine->dma_cycle_wait_quanta != 0u && dma_ticks != 0u) {
        type_unsigned_64 tick;
        for (tick = 0u; tick < dma_ticks; ++tick) {
            if (core_machine_dma_has_pending_request(&machine->shared_dma_primary,
                    &machine->shared_dma_secondary)) {
                if (machine->dma_cycle_bus_ready_gate_enabled &&
                    !machine->dma_cycle_bus_ready) {
                    continue;
                }
                if (machine->dma_cycle_wait_remaining < machine->dma_cycle_wait_quanta) {
                    ++machine->dma_cycle_wait_remaining;
                } else {
                    core_machine_dma_grant_advance(machine);
                    machine->dma_cycle_wait_remaining = 0u;
                }
            }
        }
    } else if ((machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ||
        machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) &&
        dma_ticks != 0u &&
        core_machine_dma_has_pending_request(&machine->shared_dma_primary,
            &machine->shared_dma_secondary) &&
        core_machine_transaction_hold_request(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA, 0u) == TYPE_STATUS_OK) {
        if (core_machine_transaction_hold_acknowledge(&machine->transaction,
                CORE_MACHINE_TRANSACTION_OWNER_DMA) == TYPE_STATUS_OK) {
            core_machine_dma_advance_transaction(&machine->shared_dma_latch,
                &machine->shared_dma_primary, &machine->shared_dma_secondary,
                &machine->executor_memory, &machine->transaction, dma_ticks);
        }
        core_machine_transaction_hold_release(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA);
    } else {
        core_machine_dma_advance_transaction(&machine->shared_dma_latch,
            &machine->shared_dma_primary, &machine->shared_dma_secondary,
            &machine->executor_memory, &machine->transaction, dma_ticks);
    }
    if (machine->cpu_prefetch_reservation_enabled && !refresh_pending &&
        !machine->d4_refresh_hold_pending &&
        !core_machine_dma_has_pending_request(&machine->shared_dma_primary,
            &machine->shared_dma_secondary) &&
        machine->transaction.owner == CORE_MACHINE_TRANSACTION_OWNER_NONE &&
        machine->transaction.hold_owner == CORE_MACHINE_TRANSACTION_OWNER_NONE) {
        core_machine_cpu_execution_advance_prefetch_reservation(
            &machine->executor_cpu_execution);
    }
    if (dma_ticks != 0u) {
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_DMA_ADVANCE,
            0u, (type_unsigned_32)dma_ticks, 0u);
    }
    core_machine_pit_advance(&machine->shared_pit, pit_ticks);
    if (machine->auxiliary_pit_configured) {
        core_machine_pit_advance(&machine->auxiliary_pit, pit_ticks);
    }
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
    core_machine_fdc_advance_at(&machine->fdc, due_tick);
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

static type_status core_machine_publish_elapsed_ticks(core_machine *machine,
    type_unsigned_64 elapsed_ticks, type_bool cpu_retired)
{
    if (machine == STD_NULL || elapsed_ticks == 0u ||
        UINT64_MAX - machine->elapsed_ticks < elapsed_ticks) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    machine->elapsed_ticks += elapsed_ticks;
    if (cpu_retired) {
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_CPU_RETIRE,
            core_machine_linear_pc(machine), (type_unsigned_32)elapsed_ticks, 0u);
    } else {
        core_machine_trace_record(machine, CORE_MACHINE_TRACE_EXTERNAL_TIME,
            0u, (type_unsigned_32)elapsed_ticks, 0u);
    }
    core_machine_advance_scheduler(machine, elapsed_ticks);
    return TYPE_STATUS_OK;
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

type_status core_machine_firmware_register_immutable_rom_alias(
    core_machine_firmware_context *firmware, type_unsigned_32 source_start,
    type_unsigned_32 physical_start, STD_SIZE_T bytes)
{
    if (!core_machine_firmware_context_is_active(firmware, 1)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return core_machine_register_immutable_rom_mapping_alias_from_firmware(
        firmware->machine, source_start, physical_start, bytes);
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
    const core_machine_display_config *config)
{
    const core_machine_display_port_topology *ports;

    if (config == STD_NULL) return TYPE_FALSE;
    ports = &config->ports;
    if (ports->crtc_first != CORE_MACHINE_VADP_PORT_CRTC_INDEX ||
        ports->crtc_last != CORE_MACHINE_VADP_PORT_STATUS) return TYPE_FALSE;
    return !config->ega_present ||
        (ports->attribute_first == CORE_MACHINE_VADP_PORT_ATTRIBUTE &&
        ports->attribute_last == CORE_MACHINE_VADP_PORT_ATTRIBUTE_DATA_READ &&
        ports->sequencer_first == CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX &&
        ports->sequencer_last == CORE_MACHINE_VADP_PORT_SEQUENCER_DATA &&
        ports->graphics_first == CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX &&
        ports->graphics_last == CORE_MACHINE_VADP_PORT_GRAPHICS_DATA);
}

type_status core_machine_configure_display(core_machine *machine,
    const core_machine_display_config *config)
{
    core_machine_port_provider_entry *port_checkpoint;
    type_status status;

    if (!core_machine_configuration_is_open(machine) || machine->display_configured) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (config == STD_NULL || !core_machine_display_ports_are_vadp(config) ||
        (config->ega_present && config->ega_personality ==
        CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR &&
        !core_machine_vadp_cecg_config_is_valid(&config->cecg))) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_vadp_configure_text_timing(&machine->shared_vadp,
        &config->text_timing);
    if (status != TYPE_STATUS_OK) return status;
    if (config->cga_vram_present) {
        status = core_machine_vadp_configure_cga_memory(&machine->shared_vadp,
            &machine->executor_memory);
        if (status != TYPE_STATUS_OK) return status;
    }
    if (config->ega_present) {
        port_checkpoint = core_machine_port_registration_begin(
            &machine->executor_port);
        core_machine_vadp_configure_ega_ports(&machine->shared_vadp,
            &machine->executor_port);
        status = core_machine_port_registration_status(&machine->executor_port);
        if (status != TYPE_STATUS_OK) {
            core_machine_port_rollback_registration(&machine->executor_port,
                port_checkpoint);
            return status;
        }
        status = core_machine_vadp_configure_ega_sequencer(&machine->shared_vadp,
            &machine->executor_memory, &config->ega_sequencer);
        if (status != TYPE_STATUS_OK) return status;
        status = core_machine_vadp_configure_ega_controllers(&machine->shared_vadp,
            &config->ega_controllers);
        if (status != TYPE_STATUS_OK) return status;
        status = core_machine_vadp_configure_ega_personality(
            &machine->shared_vadp, &machine->executor_port,
            config->ega_personality);
        if (status == TYPE_STATUS_OK && config->ega_personality ==
            CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) {
            status = core_machine_vadp_configure_cecg(&machine->shared_vadp,
                &config->cecg);
        }
        if (status != TYPE_STATUS_OK) {
            core_machine_port_rollback_registration(&machine->executor_port,
                port_checkpoint);
            return status;
        }
    }
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

static C_VOID core_machine_planar_parity_refresh_nmi(core_machine *machine)
{
    if (machine != STD_NULL && machine->planar_parity_configured &&
        machine->planar_parity_latched &&
        (machine->planar_parity_port_b & 0x04u) != 0u &&
        !machine->executor_cpu.data.flagMaskNMI &&
        !machine->planar_parity_nmi_signaled) {
        machine->executor_cpu.data.flagNMI = TYPE_TRUE;
        machine->planar_parity_nmi_signaled = TYPE_TRUE;
    }
}

/* PC/AT-compatible port B exposes the system 8254's refresh and speaker
 * channel outputs independently of the board-specific NMI latches. */
static type_unsigned_8 core_machine_pc_at_port_b_timer_status(
    const core_machine *machine)
{
    type_unsigned_8 value = 0u;

    if (machine == STD_NULL) return 0u;
    if (core_machine_pit_get_output(&machine->shared_pit, 1u)) value |= 0x10u;
    if (core_machine_pit_get_output(&machine->shared_pit, 2u)) value |= 0x20u;
    return value;
}

/* PC/AT-compatible boards wire system PIT counter 1 to DRAM refresh and
 * expose its output at port 61h bit 4. The board programs mode 2 with the
 * fixed refresh divider on every cold reset; channel 0 and channel 2 remain
 * firmware-owned timer and speaker resources. */
static C_VOID core_machine_d4_kbc_output(C_VOID *opaque, type_unsigned_8 value)
{
    core_machine *machine = (core_machine *)opaque;
    if (machine != STD_NULL) machine->d4_slowdown_enabled =
        (value & 0x08u) == 0u ? TYPE_TRUE : TYPE_FALSE;
}

static C_VOID core_machine_d4_refresh_output(C_VOID *opaque, type_bool asserted)
{
    core_machine *machine = (core_machine *)opaque;
    /* Generic-AT policy: the counter-1 refresh pulse ends CPU-side locality.
     * D4 establishes this refresh topology, but not a physical page-retention
     * interval or any calibrated phase duration. */
    if (machine != STD_NULL) {
        if (asserted) {
            machine->d4_refresh_pulse_active = TYPE_FALSE;
        } else if (!machine->d4_refresh_pulse_active) {
            machine->d4_refresh_pulse_active = TYPE_TRUE;
            core_machine_external_cycle_invalidate(machine);
            if (machine->d4_slowdown_enabled) {
                core_machine_pit_set_gate(&machine->auxiliary_pit,
                    machine->d4_platform_config.slowdown_pit_counter, TYPE_FALSE);
                core_machine_pit_set_gate(&machine->auxiliary_pit,
                    machine->d4_platform_config.slowdown_pit_counter, TYPE_TRUE);
            }
            machine->d4_refresh_hold_pending = TYPE_TRUE;
        }
    }
}

static C_VOID core_machine_pc_at_refresh_timer_program(core_machine *machine)
{
    if (machine == STD_NULL) return;
    core_machine_port_write(&machine->executor_port, 0x0043u, 0x74u);
    core_machine_port_write(&machine->executor_port, 0x0041u, 18u);
    core_machine_port_write(&machine->executor_port, 0x0041u, 0u);
}

static type_unsigned_8 core_machine_pc_at_port_b_speaker_value(
    const core_machine *machine)
{
    if (machine == STD_NULL) return 0u;
    if (machine->d4_platform_configured) return machine->d4_platform_port_b;
    if (machine->planar_parity_configured) return machine->planar_parity_port_b;
    return 0u;
}

static C_VOID core_machine_pc_at_speaker_refresh(core_machine *machine)
{
    type_unsigned_8 value;

    if (machine == STD_NULL) return;
    value = core_machine_pc_at_port_b_speaker_value(machine);
    machine->speaker_output = (value & 0x02u) != 0u &&
        ((value & 0x01u) == 0u ||
        core_machine_pit_get_output(&machine->shared_pit, 2u));
}

static C_VOID core_machine_pc_at_speaker_timer_output(C_VOID *owner,
    type_bool asserted)
{
    core_machine *machine = (core_machine *)owner;

    (C_VOID)asserted;
    core_machine_pc_at_speaker_refresh(machine);
}

static C_VOID core_machine_pc_at_port_b_set_speaker_gate(core_machine *machine,
    type_unsigned_8 value)
{
    if (machine == STD_NULL) return;
    core_machine_pit_set_gate(&machine->shared_pit, 2u,
        (value & 0x01u) != 0u ? TYPE_TRUE : TYPE_FALSE);
    core_machine_pc_at_speaker_refresh(machine);
}

static C_VOID core_machine_planar_parity_memory_fault(C_VOID *owner,
    type_unsigned_32 physical)
{
    (C_VOID)physical;
    (C_VOID)core_machine_report_planar_parity_fault((core_machine *)owner);
}

static type_status core_machine_planar_parity_port_read(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 *out_value)
{
    core_machine *machine = (core_machine *)owner;

    if (machine == STD_NULL || out_value == STD_NULL || !machine->planar_parity_configured ||
        port != machine->planar_parity_config.port) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_value = (type_unsigned_32)(machine->planar_parity_port_b & 0x4fu) |
        core_machine_pc_at_port_b_timer_status(machine) |
        (machine->planar_parity_latched ? 0x80u : 0u);
    return TYPE_STATUS_OK;
}

static type_status core_machine_planar_parity_port_write(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 value)
{
    core_machine *machine = (core_machine *)owner;

    if (machine == STD_NULL || !machine->planar_parity_configured ||
        port != machine->planar_parity_config.port) return TYPE_STATUS_INVALID_ARGUMENT;
    machine->planar_parity_port_b = (type_unsigned_8)value;
    core_machine_pc_at_port_b_set_speaker_gate(machine, machine->planar_parity_port_b);
    if ((machine->planar_parity_port_b & 0x04u) == 0u) {
        machine->planar_parity_latched = TYPE_FALSE;
        machine->planar_parity_nmi_signaled = TYPE_FALSE;
    } else {
        core_machine_planar_parity_refresh_nmi(machine);
    }
    return TYPE_STATUS_OK;
}

static C_VOID core_machine_d4_platform_refresh_nmi(core_machine *machine)
{
    type_bool pending;

    if (machine == STD_NULL || !machine->d4_platform_configured) return;
    pending = ((machine->d4_platform_port_b & 0x08u) == 0u &&
        machine->d4_platform_iochk_latched) ||
        ((machine->d4_platform_port_b & 0x04u) == 0u &&
        machine->d4_platform_failsafe_latched);
    if (pending && !machine->executor_cpu.data.flagMaskNMI &&
        !machine->d4_platform_nmi_signaled) {
        machine->executor_cpu.data.flagNMI = TYPE_TRUE;
        machine->d4_platform_nmi_signaled = TYPE_TRUE;
    }
}

static C_VOID core_machine_d4_platform_failsafe_output(C_VOID *owner,
    type_bool asserted)
{
    core_machine *machine = (core_machine *)owner;

    if (machine == STD_NULL || !machine->d4_platform_configured || !asserted) return;
    machine->d4_platform_failsafe_latched = TYPE_TRUE;
    core_machine_d4_platform_refresh_nmi(machine);
}

static type_status core_machine_d4_platform_port_read(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 *out_value)
{
    core_machine *machine = (core_machine *)owner;

    if (machine == STD_NULL || out_value == STD_NULL ||
        !machine->d4_platform_configured ||
        port != machine->d4_platform_config.port) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_value = (type_unsigned_32)(machine->d4_platform_port_b & 0x0fu) |
        core_machine_pc_at_port_b_timer_status(machine) |
        (machine->d4_platform_iochk_latched ? 0x40u : 0u) |
        (machine->d4_platform_failsafe_latched ? 0x80u : 0u);
    return TYPE_STATUS_OK;
}

static type_status core_machine_d4_platform_port_write(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 value)
{
    core_machine *machine = (core_machine *)owner;

    if (machine == STD_NULL || !machine->d4_platform_configured ||
        port != machine->d4_platform_config.port) return TYPE_STATUS_INVALID_ARGUMENT;
    machine->d4_platform_port_b = (type_unsigned_8)value & 0x3fu;
    core_machine_pc_at_port_b_set_speaker_gate(machine, machine->d4_platform_port_b);
    /* DeskPro port 61h bits 3 and 2 disable IOCHK and RAM/fail-safe NMI.
     * A high pulse clears the corresponding latched status; this records the
     * bounded logical effect, not electrical pulse timing. */
    if ((machine->d4_platform_port_b & 0x08u) != 0u) {
        machine->d4_platform_iochk_latched = TYPE_FALSE;
    }
    if ((machine->d4_platform_port_b & 0x04u) != 0u) {
        machine->d4_platform_failsafe_latched = TYPE_FALSE;
    }
    if (!machine->d4_platform_iochk_latched &&
        !machine->d4_platform_failsafe_latched) {
        machine->d4_platform_nmi_signaled = TYPE_FALSE;
    }
    core_machine_d4_platform_refresh_nmi(machine);
    return TYPE_STATUS_OK;
}

static const core_machine_port_provider core_machine_d4_platform_port_provider = {
    core_machine_d4_platform_port_read,
    core_machine_d4_platform_port_write
};
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

static type_bool core_machine_dma_wiring_is_valid(
    const core_machine_dma_wiring *wiring)
{
    return wiring != STD_NULL &&
        wiring->controller_count == CORE_MACHINE_DMA_CONTROLLER_COUNT &&
        wiring->cascade_channel == CORE_MACHINE_DMA_CASCADE_CHANNEL &&
        wiring->fdc_channel < VDMA_CHANNEL_COUNT;
}

type_status core_machine_configure_dma(core_machine *machine,
    const core_machine_dma_wiring *wiring,
    core_machine_dma_request_binding *out_fdc_request)
{
    type_status status;

    if (!core_machine_configuration_is_open(machine) || machine->dma_configured) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (!core_machine_dma_wiring_is_valid(wiring) || out_fdc_request == STD_NULL) {
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

type_status core_machine_get_fdc_dma_request_binding(const core_machine *machine,
    core_machine_dma_request_binding *out_binding)
{
    if (machine == STD_NULL || out_binding == STD_NULL || !machine->dma_configured) {
        return TYPE_STATUS_INVALID_STATE;
    }
    *out_binding = machine->fdc_dma_request;
    return TYPE_STATUS_OK;
}

type_status core_machine_set_dma_bus_ready(core_machine *machine, C_INT ready)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        !machine->dma_cycle_bus_ready_gate_enabled) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    machine->dma_cycle_bus_ready = ready ? TYPE_TRUE : TYPE_FALSE;
    return TYPE_STATUS_OK;
}
type_status core_machine_set_cpu_bus_ready(core_machine *machine, C_INT ready)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        !machine->cpu_cycle_bus_ready_gate_enabled) return TYPE_STATUS_INVALID_ARGUMENT;
    machine->cpu_cycle_bus_ready = ready ? TYPE_TRUE : TYPE_FALSE;
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

type_status core_machine_enable_memory_parity(core_machine *machine,
    STD_SIZE_T bytes, core_machine_memory_parity_fault_observer fault, C_VOID *owner)
{
    if (!core_machine_configuration_is_open(machine)) return TYPE_STATUS_INVALID_STATE;
    return core_machine_memory_enable_parity(&machine->executor_memory, bytes,
        fault, owner);
}
type_status core_machine_configure_planar_parity(core_machine *machine,
    const core_machine_planar_parity_config *config)
{
    core_machine_port_provider provider = { core_machine_planar_parity_port_read,
        core_machine_planar_parity_port_write };
    core_machine_port_provider_entry *checkpoint;
    type_status status;

    if (!core_machine_configuration_is_open(machine) || machine->planar_parity_configured)
        return TYPE_STATUS_INVALID_STATE;
    if (config == STD_NULL || config->port != CORE_MACHINE_PC_AT_PORT_B ||
        config->memory_bytes == 0u || config->memory_bytes >
            machine->executor_memory.connect.installed_bytes ||
        core_machine_port_has_read(&machine->executor_port,
            config->port) || core_machine_port_has_write(&machine->executor_port,
            config->port)) return TYPE_STATUS_INVALID_ARGUMENT;
    checkpoint = core_machine_port_registration_begin(&machine->executor_port);
    status = core_machine_install_port_provider(machine, config->port, config->port,
        &provider, machine);
    if (status != TYPE_STATUS_OK) {
        core_machine_port_rollback_registration(&machine->executor_port, checkpoint);
        return status;
    }
    machine->planar_parity_config = *config;
    machine->planar_parity_port_b = 0x04u;
    machine->planar_parity_configured = TYPE_TRUE;
    core_machine_pit_set_output(&machine->shared_pit, 2u,
        core_machine_pc_at_speaker_timer_output, machine);
    core_machine_pc_at_refresh_timer_program(machine);
    core_machine_pc_at_port_b_set_speaker_gate(machine, machine->planar_parity_port_b);
    status = core_machine_memory_enable_parity(&machine->executor_memory,
        config->memory_bytes, core_machine_planar_parity_memory_fault, machine);
    if (status != TYPE_STATUS_OK) {
        machine->planar_parity_configured = TYPE_FALSE;
        core_machine_port_rollback_registration(&machine->executor_port, checkpoint);
        return status;
    }
    return TYPE_STATUS_OK;
}

type_status core_machine_configure_d4_platform(core_machine *machine,
    const core_machine_d4_platform_config *config)
{
    core_machine_port_provider_entry *checkpoint;
    type_status status;

    if (!core_machine_configuration_is_open(machine) ||
        machine->d4_platform_configured) return TYPE_STATUS_INVALID_STATE;
    if (config == STD_NULL || config->port != CORE_MACHINE_PC_AT_PORT_B ||
        config->failsafe_pit_counter >= 3u || config->slowdown_pit_counter >= 3u ||
        config->failsafe_pit_counter == config->slowdown_pit_counter || !machine->auxiliary_pit_configured ||
        machine->auxiliary_pit.connect.output[config->failsafe_pit_counter] != STD_NULL ||
        machine->auxiliary_pit.connect.output[config->slowdown_pit_counter] != STD_NULL ||
        machine->shared_kbc.connect.output_port != STD_NULL ||
        core_machine_port_has_read(&machine->executor_port, config->port) ||
        core_machine_port_has_write(&machine->executor_port, config->port)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    checkpoint = core_machine_port_registration_begin(&machine->executor_port);
    status = core_machine_install_port_provider(machine, config->port, config->port,
        &core_machine_d4_platform_port_provider, machine);
    if (status != TYPE_STATUS_OK) {
        core_machine_port_rollback_registration(&machine->executor_port, checkpoint);
        return status;
    }
    machine->d4_platform_config = *config;
    machine->d4_platform_port_b = 0x0fu;
    machine->d4_platform_configured = TYPE_TRUE;
    core_machine_pit_set_output(&machine->shared_pit, 2u,
        core_machine_pc_at_speaker_timer_output, machine);
    core_machine_pc_at_refresh_timer_program(machine);
    core_machine_pit_set_output(&machine->shared_pit, 1u,
        core_machine_d4_refresh_output, machine);
    core_machine_pc_at_port_b_set_speaker_gate(machine, machine->d4_platform_port_b);
    core_machine_pit_set_output(&machine->auxiliary_pit,
        config->failsafe_pit_counter, core_machine_d4_platform_failsafe_output,
        machine);
    if (!core_machine_kbc_bind_output_port(&machine->shared_kbc,
            core_machine_d4_kbc_output, machine)) return TYPE_STATUS_INVALID_ARGUMENT;
    return TYPE_STATUS_OK;
}
type_status core_machine_report_planar_parity_fault(core_machine *machine)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        !machine->planar_parity_configured) return TYPE_STATUS_INVALID_STATE;
    machine->planar_parity_latched = TYPE_TRUE;
    core_machine_planar_parity_refresh_nmi(machine);
    return TYPE_STATUS_OK;
}

static type_status core_machine_absent_memory_read(C_VOID *owner,
    type_unsigned_32 physical, type_virtual_address destination,
    type_native_unsigned bytes)
{
    const core_machine_absent_memory *absent =
        (const core_machine_absent_memory *)owner;

    (C_VOID)physical;
    if (absent == STD_NULL || !absent->configured || destination == 0u ||
        bytes == 0u) return TYPE_STATUS_FAULT;
    STD_MEMSET((C_VOID *)destination, absent->config.read_value, bytes);
    return TYPE_STATUS_OK;
}

static type_status core_machine_absent_memory_write(C_VOID *owner,
    type_unsigned_32 physical, type_virtual_address source,
    type_native_unsigned bytes)
{
    const core_machine_absent_memory *absent =
        (const core_machine_absent_memory *)owner;

    (C_VOID)physical;
    if (absent == STD_NULL || !absent->configured || source == 0u ||
        bytes == 0u) return TYPE_STATUS_FAULT;
    return TYPE_STATUS_OK;
}

static type_status core_machine_absent_memory_query(C_VOID *owner,
    type_unsigned_32 physical, type_native_unsigned bytes,
    core_machine_memory_access access)
{
    const core_machine_absent_memory *absent =
        (const core_machine_absent_memory *)owner;

    (C_VOID)physical;
    if (absent == STD_NULL || !absent->configured || bytes == 0u ||
        (access != CORE_MACHINE_MEMORY_ACCESS_READ &&
        access != CORE_MACHINE_MEMORY_ACCESS_WRITE)) return TYPE_STATUS_FAULT;
    return TYPE_STATUS_OK;
}

type_status core_machine_clear_d4_iochk_fault(core_machine *machine)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        !machine->d4_platform_configured) return TYPE_STATUS_INVALID_STATE;
    machine->d4_platform_iochk_latched = TYPE_FALSE;
    if (!machine->d4_platform_failsafe_latched) machine->d4_platform_nmi_signaled = TYPE_FALSE;
    core_machine_d4_platform_refresh_nmi(machine);
    return TYPE_STATUS_OK;
}

type_status core_machine_report_d4_iochk_fault(core_machine *machine)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        !machine->d4_platform_configured) return TYPE_STATUS_INVALID_STATE;
    machine->d4_platform_iochk_latched = TYPE_TRUE;
    core_machine_d4_platform_refresh_nmi(machine);
    return TYPE_STATUS_OK;
}

type_status core_machine_get_d4_platform_observation(const core_machine *machine,
    core_machine_d4_platform_observation *out_observation)
{
    if (machine == STD_NULL || out_observation == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    out_observation->configured = machine->d4_platform_configured;
    out_observation->iochk_enabled = (machine->d4_platform_port_b & 0x08u) == 0u;
    out_observation->failsafe_enabled =
        (machine->d4_platform_port_b & 0x04u) == 0u;
    out_observation->iochk_latched = machine->d4_platform_iochk_latched;
    out_observation->failsafe_latched = machine->d4_platform_failsafe_latched;
    out_observation->nmi_signaled = machine->d4_platform_nmi_signaled;
    return TYPE_STATUS_OK;
}
type_status core_machine_get_speaker_observation(const core_machine *machine,
    core_machine_speaker_observation *out_observation)
{
    type_unsigned_8 value;

    if (machine == STD_NULL || out_observation == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    value = core_machine_pc_at_port_b_speaker_value(machine);
    out_observation->configured = machine->d4_platform_configured ||
        machine->planar_parity_configured;
    out_observation->timer_gate = (value & 0x01u) != 0u;
    out_observation->data_enabled = (value & 0x02u) != 0u;
    out_observation->timer_output = core_machine_pit_get_output(
        &machine->shared_pit, 2u);
    out_observation->output = machine->speaker_output;
    return TYPE_STATUS_OK;
}
type_status core_machine_configure_absent_memory(core_machine *machine,
    const core_machine_absent_memory_config *config)
{
    type_status status;

    if (!core_machine_configuration_is_open(machine) ||
        machine->absent_memory.configured) return TYPE_STATUS_INVALID_STATE;
    if (config == STD_NULL || config->bytes == 0u ||
        (type_unsigned_64)config->physical_start + config->bytes >
            (type_unsigned_64)TYPE_MAX_UNSIGNED_32 + 1u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    machine->absent_memory.config = *config;
    machine->absent_memory.configured = TYPE_TRUE;
    status = core_machine_memory_register_device_provider(&machine->executor_memory,
        config->physical_start, config->bytes, core_machine_absent_memory_read,
        core_machine_absent_memory_write, core_machine_absent_memory_query,
        &machine->absent_memory);
    if (status != TYPE_STATUS_OK) {
        STD_MEMSET(&machine->absent_memory, 0, sizeof(machine->absent_memory));
        return status;
    }
    return TYPE_STATUS_OK;
}

type_status core_machine_get_planar_parity_observation(const core_machine *machine,
    core_machine_planar_parity_observation *out_observation)
{
    if (machine == STD_NULL || out_observation == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    out_observation->configured = machine->planar_parity_configured;
    out_observation->enabled = (machine->planar_parity_port_b & 0x04u) != 0u;
    out_observation->latched = machine->planar_parity_latched;
    out_observation->nmi_signaled = machine->planar_parity_nmi_signaled;
    return TYPE_STATUS_OK;
}

static C_INT core_machine_fdc_topology_is_valid(
    const core_machine_fdc_topology *topology)
{
    STD_SIZE_T first;
    STD_SIZE_T second;

    if (topology == STD_NULL || topology->media_registry == STD_NULL ||
        topology->config.dma_channel != topology->dma_request.channel ||
        topology->config.unready_read_policy >
            CORE_MACHINE_FDC_UNREADY_READ_DESKPRO_REFERENCE) {
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
        topology->media_id == CORE_MACHINE_MEDIA_ID_INVALID ||
        topology->slave_media_id == topology->media_id) return 0;
    config = &topology->config;
    if (config->lba28_supported != TYPE_FALSE && config->lba28_supported != TYPE_TRUE) {
        return 0;
    }
    if (config->protocol != CORE_MACHINE_HDC_PROTOCOL_ATA_PIO &&
        config->protocol != CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB) return 0;
    if ((config->protocol == CORE_MACHINE_HDC_PROTOCOL_ATA_PIO &&
            config->drive_address_port != 0u) ||
        (config->protocol == CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB &&
            (config->lba28_supported || config->drive_address_port == 0u))) return 0;
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
        &machine->executor_port, &machine->fdc_topology.config,
        &machine->fdc_topology.observation_provider);
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
    if (topology->config.protocol == CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB &&
        (!machine->fdc_configured ||
            topology->config.drive_address_port != machine->fdc_topology.config.direction_port ||
            !core_machine_port_has_read(&machine->executor_port,
                topology->config.drive_address_port))) return TYPE_STATUS_INVALID_STATE;
    if (!core_machine_controller_ports_are_available(machine, ports,
            sizeof(ports) / sizeof(ports[0]))) return TYPE_STATUS_INVALID_STATE;
    provider = core_machine_hdc_port_provider();
    if (provider == STD_NULL) return TYPE_STATUS_FAULT;
    port_checkpoint = core_machine_port_registration_begin(&machine->executor_port);
    machine->hdc_topology = *topology;
    core_machine_hdc_connect(&machine->hdc, machine->hdc_topology.media_registry,
        machine->hdc_topology.media_id, machine->hdc_topology.slave_media_id,
        &machine->shared_pic_master,
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
    if (machine->hdc_topology.config.protocol == CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB) {
        status = core_machine_port_add_read_wired_or_provider(&machine->executor_port,
            machine->hdc_topology.config.drive_address_port, provider->read, &machine->hdc);
        if (status != TYPE_STATUS_OK) {
            core_machine_port_rollback_registration(&machine->executor_port,
                port_checkpoint);
            core_machine_hdc_finalize(&machine->hdc);
            STD_MEMSET(&machine->hdc_topology, TYPE_ZERO_8,
                sizeof(machine->hdc_topology));
            return status;
        }
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
        (config->a20_wrap_policy != CORE_MACHINE_A20_WRAP_GLOBAL_MASK &&
        config->a20_wrap_policy != CORE_MACHINE_A20_WRAP_FIRST_TO_SECOND_MIB) ||
        !core_machine_clock_plan_is_valid(&config->clock_plan) ||
        !core_machine_retirement_time_contract_is_valid(
            config->retirement_time_contract) ||
        !core_machine_external_cycle_timing_is_valid(
            &config->external_cycle_timing) ||
        !core_machine_external_access_wait_windows_are_valid(
            config->external_access_wait_windows) ||
        (config->cpu_cycle_bus_ready_gate_enabled != TYPE_FALSE &&
        config->cpu_cycle_bus_ready_gate_enabled != TYPE_TRUE) ||
        (config->cpu_prefetch_reservation_enabled != TYPE_FALSE &&
        config->cpu_prefetch_reservation_enabled != TYPE_TRUE) ||
        (config->auxiliary_pit_present != TYPE_FALSE &&
        config->auxiliary_pit_present != TYPE_TRUE) ||
        (config->dma_cycle_bus_ready_gate_enabled != TYPE_FALSE &&
        config->dma_cycle_bus_ready_gate_enabled != TYPE_TRUE) ||
        (config->auxiliary_pit_present && config->auxiliary_pit_base_port > 0xfffcu)) {
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
    machine->retirement_time_contract = config->retirement_time_contract;
    machine->external_cycle_timing = config->external_cycle_timing;
    STD_MEMCPY(machine->external_access_wait_windows,
        config->external_access_wait_windows,
        sizeof(machine->external_access_wait_windows));
    machine->dma_cycle_wait_quanta = config->dma_cycle_wait_quanta;
    machine->dma_cycle_bus_ready_gate_enabled = config->dma_cycle_bus_ready_gate_enabled;
    machine->cpu_cycle_bus_ready_gate_enabled = config->cpu_cycle_bus_ready_gate_enabled;
    machine->cpu_prefetch_reservation_enabled = config->cpu_prefetch_reservation_enabled;
    machine->dma_cycle_bus_ready = TYPE_TRUE;
    machine->cpu_cycle_bus_ready = TYPE_TRUE;
    if (config->retirement_qualification != STD_NULL) {
        if (config->retirement_qualification->entries == STD_NULL ||
            config->retirement_qualification->entry_count == 0u ||
            config->retirement_qualification->entry_count >
                CORE_MACHINE_RETIREMENT_QUALIFICATION_CAPACITY) {
            STD_FREE(machine);
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
        machine->retirement_qualification_count =
            config->retirement_qualification->entry_count;
        STD_MEMCPY(machine->retirement_qualification,
            config->retirement_qualification->entries,
            machine->retirement_qualification_count *
                sizeof(machine->retirement_qualification[0]));
    }
    machine->cpu_80386_cr_mov_ignores_mod =
        config->cpu_80386_cr_mov_ignores_mod;
    if (machine->cpu_80386_cr_mov_ignores_mod &&
        machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80386) {
        STD_FREE(machine);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (core_machine_timeline_initialize(&machine->timeline) != TYPE_STATUS_OK) {
        STD_FREE(machine);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    core_machine_resolve_instruction_timing(&machine->instruction_timing,
        &config->instruction_timing, config->ticks_per_instruction);
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8086) {
        machine->maximum_instruction_ticks = CORE_MACHINE_8086_SOURCE_MAXIMUM_TICKS;
    } else if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186) {
        machine->maximum_instruction_ticks = CORE_MACHINE_80186_SOURCE_MAXIMUM_TICKS;
    } else if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286) {
        machine->maximum_instruction_ticks = CORE_MACHINE_80286_SOURCE_MAXIMUM_TICKS;
    } else if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) {
        machine->maximum_instruction_ticks = CORE_MACHINE_80386_SOURCE_MAXIMUM_TICKS;
    } else {
        machine->maximum_instruction_ticks = core_machine_instruction_maximum_ticks(
            &machine->instruction_timing);
    }
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
    machine->kbc_serial_delivery_ticks = config->kbc_serial_delivery_ticks;
    core_machine_fpu_initialize(&machine->fpu, config->fpu_profile);
    STD_ATOMIC_INIT(&machine->stop_requested, 0);
    core_machine_trace_initialize(machine);
    core_machine_transaction_initialize(&machine->transaction);
    core_machine_transaction_bind_trace(&machine->transaction,
        core_machine_transaction_trace, machine);
    core_machine_cpu_diagnostic_initialize(machine);
    core_machine_retirement_observation_initialize(machine);

    core_machine_cpu_execution_context_initialize(&machine->executor_cpu_execution,
        &machine->executor_cpu, &machine->executor_cpu_instructions,
        &machine->executor_memory, &machine->executor_port);
    core_machine_cpu_execution_context_bind_profiles(
        &machine->executor_cpu_execution, machine->cpu_profile,
        machine->fpu.profile, machine->cpu_80386_cr_mov_ignores_mod);
    core_machine_cpu_execution_context_bind_fpu(
        &machine->executor_cpu_execution, &machine->fpu);
    core_machine_cpu_execution_context_bind_external_cycle_provider(
        &machine->executor_cpu_execution, core_machine_cpu_external_cycle_trace,
        machine);    core_machine_cpu_execution_context_bind_transaction(
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
    if (core_machine_memory_set_a20_wrap_policy(&machine->executor_memory,
            config->a20_wrap_policy) != TYPE_STATUS_OK) {
        core_machine_destroy(machine);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    /* The 80386 reset vector is at physical FFFFFFF0.  Configurations with
     * at least 1 MiB of backing RAM retain the PC/AT firmware window's final
     * 64 KiB alias at F0000.  Low-RAM configurations instead provide their
     * reset ROM through an explicit immutable mapping. */
    if (memory_bytes >= 0x00100000u &&
        core_machine_memory_register_mapping(&machine->executor_memory,
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
    if (config->auxiliary_pit_present) {
        core_machine_pit_initialize_at(&machine->auxiliary_pit,
            &machine->executor_port, config->auxiliary_pit_base_port);
        machine->auxiliary_pit_configured = TYPE_TRUE;
    }
    core_machine_pit_set_output(&machine->shared_pit, 0,
        core_machine_pic_timer_output, &machine->shared_pit_irq0_source);
    core_machine_kbc_bind_core_services(&machine->shared_kbc,
        &machine->shared_pic_master, &machine->shared_pic_slave,
        &machine->executor_memory, &machine->executor_cpu_execution,
        !config->kbc_aux_absent);
    core_machine_kbc_set_typematic_timing(&machine->shared_kbc,
        machine->kbc_typematic_initial_ticks,
        machine->kbc_typematic_repeat_ticks);
    core_machine_kbc_set_command_response_timing(&machine->shared_kbc,
        machine->kbc_command_response_ticks);
    core_machine_kbc_set_serial_delivery_timing(&machine->shared_kbc,
        machine->kbc_serial_delivery_ticks);
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

type_status core_machine_create_from_plan(const core_machine_plan *plan,
    core_machine **out_machine)
{
    type_status status;

    if (out_machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_machine = STD_NULL;
    if (core_machine_plan_validate(plan) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_create_internal(&plan->configuration, out_machine,
        STD_NULL, STD_NULL);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_plan_apply_topology(*out_machine, &plan->topology);
    if (status != TYPE_STATUS_OK) {
        core_machine_destroy(*out_machine);
        *out_machine = STD_NULL;
        return status;
    }
    (*out_machine)->timing_plan = *plan;
    /* Configuration-owned retirement qualification is already copied by
     * create_internal; the plan copy must retain no caller-owned pointer. */
    (*out_machine)->timing_plan.configuration.retirement_qualification = STD_NULL;
    (*out_machine)->timing_plan_copied = TYPE_TRUE;
    return TYPE_STATUS_OK;
}

type_status core_machine_get_timing_disposition(const core_machine *machine,
    core_machine_timing_capability capability,
    core_machine_timing_disposition *out_disposition)
{
    const core_machine_timing_declaration *declaration;

    if (machine == STD_NULL || out_disposition == STD_NULL ||
        !machine->timing_plan_copied ||
        !core_machine_timing_capability_is_valid(capability)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    declaration = core_machine_plan_declaration_find(&machine->timing_plan,
        capability);
    if (declaration == STD_NULL) return TYPE_STATUS_INVALID_STATE;
    *out_disposition = declaration->disposition;
    return TYPE_STATUS_OK;
}

type_status core_machine_get_timing_declaration(const core_machine *machine,
    core_machine_timing_capability capability,
    core_machine_timing_declaration *out_declaration)
{
    if (machine == STD_NULL || out_declaration == STD_NULL ||
        !machine->timing_plan_copied ||
        !core_machine_timing_capability_is_valid(capability)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    {
        const core_machine_timing_declaration *declaration =
            core_machine_plan_declaration_find(&machine->timing_plan, capability);

        if (declaration == STD_NULL) return TYPE_STATUS_INVALID_STATE;
        *out_declaration = *declaration;
    }
    return TYPE_STATUS_OK;
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
    machine->planar_parity_port_b = machine->planar_parity_configured ? 0x04u : 0u;
    machine->planar_parity_latched = TYPE_FALSE;
    machine->planar_parity_nmi_signaled = TYPE_FALSE;
    machine->speaker_output = TYPE_FALSE;
    machine->d4_platform_port_b = machine->d4_platform_configured ? 0x0fu : 0u;
    machine->d4_platform_iochk_latched = TYPE_FALSE;
    machine->d4_platform_failsafe_latched = TYPE_FALSE;
    machine->d4_platform_nmi_signaled = TYPE_FALSE;
    core_machine_fdc_reset(&machine->fdc);
    core_machine_hdc_reset(&machine->hdc);
    core_machine_pic_reset(&machine->shared_pic_master,
        &machine->shared_pic_slave);
    core_machine_pit_reset(&machine->shared_pit);
    if (machine->planar_parity_configured || machine->d4_platform_configured) {
        core_machine_pc_at_refresh_timer_program(machine);
    }
    if (machine->planar_parity_configured) {
        core_machine_pc_at_port_b_set_speaker_gate(machine, machine->planar_parity_port_b);
    }
    if (machine->d4_platform_configured) {
        core_machine_pc_at_port_b_set_speaker_gate(machine, machine->d4_platform_port_b);
    }
    if (machine->auxiliary_pit_configured) {
        core_machine_pit_reset(&machine->auxiliary_pit);
    }
    if (machine->d4_platform_configured) {
        core_machine_pit_set_output(&machine->shared_pit, 1u,
            core_machine_d4_refresh_output, machine);
        core_machine_pit_set_output(&machine->auxiliary_pit,
            machine->d4_platform_config.failsafe_pit_counter,
            core_machine_d4_platform_failsafe_output, machine);
    }
    machine->d4_refresh_hold_pending = TYPE_FALSE;
    machine->d4_refresh_pulse_active = TYPE_FALSE;
    machine->d4_refresh_address = 0u;
    machine->d4_slowdown_enabled = TYPE_FALSE;
    core_machine_vadp_reset(&machine->shared_vadp);

    STD_ATOMIC_STORE(&machine->stop_requested, 0);
    machine->fault_detail = 0u;
    machine->elapsed_ticks = 0u;
    machine->dma_cycle_wait_remaining = 0u;
    machine->dma_cycle_bus_ready = TYPE_TRUE;
    machine->cpu_cycle_bus_ready = TYPE_TRUE;
    machine->external_cycle_page_tag = 0u;
    machine->external_cycle_round_ticks = 0u;
    machine->cpu_retirement_wait_ticks = 0u;
    machine->cpu_retirement_completion_ticks = 0u;
    machine->cpu_retirement_source_ticks = 0u;
    machine->external_cycle_page_valid = TYPE_FALSE;
    machine->external_cycle_pending_valid = TYPE_FALSE;
    machine->external_cycle_pending_space = CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY;
    machine->external_cycle_pending_physical = 0u;
    machine->external_cycle_pending_bytes = 0u;
    machine->external_cycle_pending_write = TYPE_FALSE;
    machine->external_cycle_pending_provenance =
        CORE_MACHINE_CPU_MEMORY_ACCESS_DATA;
    machine->external_cycle_overlap_valid = TYPE_FALSE;
    machine->external_cycle_overlap_next_physical = 0u;
    machine->external_cycle_round_overflow = TYPE_FALSE;
    machine->cpu_retirement_wait_pending = TYPE_FALSE;
    machine->retirement_eligibility_key_valid = TYPE_FALSE;
    machine->source_repeat_active = TYPE_FALSE;
    machine->source_repeat_cs = 0u;
    machine->source_repeat_eip = 0u;
    machine->source_repeat_opcode = 0u;
    machine->source_repeat_prefix = 0u;
    machine->source_repeat_operand_size = TYPE_FALSE;
    machine->source_repeat_address_size = TYPE_FALSE;
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
    core_machine_retirement_observation_reset(machine);
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
        machine->planar_parity_configured ||
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
            if (machine->cpu_retirement_wait_pending) {
                if (machine->cpu_cycle_bus_ready_gate_enabled && !machine->cpu_cycle_bus_ready) {
                    if (result->ticks == UINT64_MAX || machine->elapsed_ticks == UINT64_MAX) {
                        (C_VOID)core_machine_report_fault(machine, 0x54494d45u);
                        result->reason = CORE_MACHINE_STOP_FAULT;
                        result->linear_pc = core_machine_linear_pc(machine);
                        result->detail = machine->fault_detail;
                        return TYPE_STATUS_FAULT;
                    }
                    ++result->ticks;
                    if (core_machine_publish_elapsed_ticks(machine, 1u, TYPE_FALSE) != TYPE_STATUS_OK) return TYPE_STATUS_FAULT;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    continue;
                }
                if (machine->cpu_retirement_wait_ticks != 0u) {
                    if (result->ticks == UINT64_MAX ||
                        machine->elapsed_ticks == UINT64_MAX) {
                        (C_VOID)core_machine_report_fault(machine, 0x54494d45u);
                        result->reason = CORE_MACHINE_STOP_FAULT;
                        result->linear_pc = core_machine_linear_pc(machine);
                        result->detail = machine->fault_detail;
                        result->elapsed_ticks = machine->elapsed_ticks;
                        return TYPE_STATUS_FAULT;
                    }
                    ++result->ticks;
                    --machine->cpu_retirement_wait_ticks;
                    if (core_machine_publish_elapsed_ticks(machine, 1u, TYPE_FALSE) != TYPE_STATUS_OK) return TYPE_STATUS_FAULT;
                    result->elapsed_ticks = machine->elapsed_ticks;
                if (machine->executor_cpu.data.flagHalt) {
                    machine->lifecycle = CORE_MACHINE_PAUSED;
                    result->reason = CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    return TYPE_STATUS_OK;
                }
                    continue;
                }
                if (budget.ticks != 0u && machine->cpu_retirement_completion_ticks >
                    budget.ticks - result->ticks) {
                    machine->lifecycle = CORE_MACHINE_PAUSED;
                    result->reason = CORE_MACHINE_STOP_BUDGET;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return TYPE_STATUS_OK;
                }
                if (UINT64_MAX - result->ticks < machine->cpu_retirement_completion_ticks ||
                    UINT64_MAX - machine->elapsed_ticks <
                        machine->cpu_retirement_completion_ticks) {
                    (C_VOID)core_machine_report_fault(machine, 0x54494d45u);
                    result->reason = CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return TYPE_STATUS_FAULT;
                }
                if (!core_machine_publish_successful_retirement(machine)) {
                    (C_VOID)core_machine_report_fault(machine, 0x54494d55u);
                    result->reason = CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return TYPE_STATUS_FAULT;
                }
                ++result->executed;
                result->ticks += machine->cpu_retirement_completion_ticks;
                if (core_machine_publish_elapsed_ticks(machine, machine->cpu_retirement_completion_ticks, TYPE_TRUE) != TYPE_STATUS_OK) return TYPE_STATUS_FAULT;
                machine->cpu_retirement_wait_pending = TYPE_FALSE;
                machine->cpu_retirement_completion_ticks = 0u;
                machine->cpu_retirement_source_ticks = 0u;
                result->elapsed_ticks = machine->elapsed_ticks;
                continue;
            }
            if (budget.ticks != 0u && machine->maximum_instruction_ticks >
                budget.ticks - result->ticks) {
                machine->lifecycle = CORE_MACHINE_PAUSED;
                result->reason = CORE_MACHINE_STOP_BUDGET;
                result->linear_pc = core_machine_linear_pc(machine);
                result->elapsed_ticks = machine->elapsed_ticks;
                return TYPE_STATUS_OK;
            }
            if (machine->d4_platform_configured && machine->d4_slowdown_enabled &&
                !core_machine_pit_get_output(&machine->auxiliary_pit,
                    machine->d4_platform_config.slowdown_pit_counter)) {
                ++result->ticks;
                if (core_machine_publish_elapsed_ticks(machine, 1u, TYPE_FALSE) != TYPE_STATUS_OK) return TYPE_STATUS_FAULT;
                result->elapsed_ticks = machine->elapsed_ticks;
                continue;
            }
            if (machine->execution_provider != STD_NULL &&
                machine->execution_provider->refresh != STD_NULL) {
                machine->execution_provider->refresh(
                    machine->execution_provider_context);
            }
            core_machine_kbc_refresh(&machine->shared_kbc);
            {
                type_bool was_halted = machine->executor_cpu.data.flagHalt;

                machine->external_cycle_round_ticks = 0u;
                /* A completed instruction round cannot inherit an undeclared
                 * external-cycle overlap into the next CPU refresh. */
                core_machine_external_cycle_invalidate(machine);
                core_machine_cpu_execution_refresh(&machine->executor_cpu_execution);
                if (machine->d4_platform_configured &&
                    core_machine_cpu_execution_consume_shutdown_request(
                        &machine->executor_cpu_execution)) {
                    machine->lifecycle = CORE_MACHINE_PAUSED;
                    if (core_machine_cold_reset(machine) != TYPE_STATUS_OK) {
                        return TYPE_STATUS_FAULT;
                    }
                    result->reason = CORE_MACHINE_STOP_RESET_REQUESTED;
                    result->linear_pc = core_machine_linear_pc(machine);
                    return TYPE_STATUS_OK;
                }
                if (machine->lifecycle == CORE_MACHINE_FAULTED) {
                    result->reason = CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return TYPE_STATUS_FAULT;
                }
                if (core_machine_cpu_execution_consume_instruction_fault_delivery(
                        &machine->executor_cpu_execution)) {
                    /* The synchronous exception frame and vector are committed, but
                     * the faulting instruction did not retire.  The handler starts
                     * at the next public execution round, without publishing CPU
                     * or device time for this faulting round. */
                    machine->lifecycle = CORE_MACHINE_PAUSED;
                    result->reason = CORE_MACHINE_STOP_BUDGET;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return TYPE_STATUS_OK;
                }
                if (was_halted && machine->executor_cpu.data.flagHalt) {
                    machine->lifecycle = CORE_MACHINE_PAUSED;
                    result->reason = CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return TYPE_STATUS_OK;
                }
                if (machine->cpu_prefetch_reservation_enabled) {
                    core_machine_cpu_execution_reserve_prefetch(
                        &machine->executor_cpu_execution);
                }
            }
            {
                core_machine_cpu_timing_result timing_result;
                type_unsigned_64 instruction_ticks;

                if (!core_machine_cpu_timing_select(machine, &timing_result) ||
                    machine->external_cycle_round_overflow) {
                    (C_VOID)core_machine_report_fault(machine, 0x54494d45u);
                    result->reason = CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return TYPE_STATUS_FAULT;
                }
                instruction_ticks = timing_result.ticks;
                if (!core_machine_add_ticks(&instruction_ticks,
                        machine->external_cycle_round_ticks) ||
                    UINT64_MAX - result->ticks < instruction_ticks ||
                    UINT64_MAX - machine->elapsed_ticks < instruction_ticks) {
                    (C_VOID)core_machine_report_fault(machine, 0x54494d45u);
                    result->reason = CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return TYPE_STATUS_FAULT;
                }
                core_machine_retirement_observation_capture_eligibility_key(machine);
                if (machine->external_cycle_round_ticks != 0u) {
                    machine->cpu_retirement_wait_pending = TYPE_TRUE;
                    machine->cpu_retirement_wait_ticks = machine->external_cycle_round_ticks;
                    machine->cpu_retirement_completion_ticks = instruction_ticks - machine->external_cycle_round_ticks;
                    machine->cpu_retirement_source_ticks = instruction_ticks;
                    continue;
                }
                machine->cpu_retirement_source_ticks = instruction_ticks;
                if (!core_machine_publish_successful_retirement(machine)) {
                    (C_VOID)core_machine_report_fault(machine, 0x54494d55u);
                    result->reason = CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return TYPE_STATUS_FAULT;
                }
                ++result->executed;
                result->ticks += instruction_ticks;
                if (core_machine_publish_elapsed_ticks(machine,
                        instruction_ticks, TYPE_TRUE) != TYPE_STATUS_OK) {
                    (C_VOID)core_machine_report_fault(machine, 0x54494d45u);
                    result->reason = CORE_MACHINE_STOP_FAULT;
                    result->linear_pc = core_machine_linear_pc(machine);
                    result->detail = machine->fault_detail;
                    result->elapsed_ticks = machine->elapsed_ticks;
                    return TYPE_STATUS_FAULT;
                }
                machine->cpu_retirement_source_ticks = 0u;
                result->elapsed_ticks = machine->elapsed_ticks;
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

type_status core_machine_advance_time(core_machine *machine,
    type_unsigned_64 source_ticks)
{
    if (machine == STD_NULL || machine->retirement_time_contract ==
        CORE_MACHINE_RETIREMENT_TIME_PHYSICAL ||
        !core_machine_mutable_operation_is_allowed(machine) ||
        (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return core_machine_publish_elapsed_ticks(machine, source_ticks, TYPE_FALSE);
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
    if (!masked) {
        core_machine_planar_parity_refresh_nmi(machine);
        core_machine_d4_platform_refresh_nmi(machine);
    }
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

type_status core_machine_keyboard_receive_native_byte(core_machine *machine,
    type_unsigned_8 native_byte)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_FAULTED) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return core_machine_kbc_submit_native_byte(&machine->shared_kbc, native_byte);
}

type_status core_machine_keyboard_get_native_scan_set(const core_machine *machine,
    type_unsigned_8 *out_scan_set)
{
    if (machine == STD_NULL || out_scan_set == STD_NULL ||
        machine->lifecycle == CORE_MACHINE_INITIALIZED) {
        return TYPE_STATUS_INVALID_STATE;
    }
    *out_scan_set = machine->shared_kbc.data.scan_set;
    return TYPE_STATUS_OK;
}

type_status core_machine_keyboard_receive_native_bytes(core_machine *machine,
    const type_unsigned_8 *native_bytes, STD_SIZE_T count)
{
    if (machine == STD_NULL || !core_machine_mutable_operation_is_allowed(machine) ||
        machine->lifecycle == CORE_MACHINE_INITIALIZED ||
        machine->lifecycle == CORE_MACHINE_FAULTED) {
        return TYPE_STATUS_INVALID_STATE;
    }
    return core_machine_kbc_submit_native_bytes(&machine->shared_kbc, native_bytes,
        count);
}

type_status core_machine_mouse_receive_relative(core_machine *machine,
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
        if (machine->auxiliary_pit_configured) {
            core_machine_pit_finalize(&machine->auxiliary_pit);
        }
        core_machine_vadp_finalize(&machine->shared_vadp);
        core_machine_cpu_execution_finalize(&machine->executor_cpu_execution);
        core_machine_port_finalize(&machine->executor_port);
        core_machine_memory_finalize(&machine->executor_memory);
        for (STD_SIZE_T index = 0u; index < machine->immutable_rom_mapping_count;
                ++index) {
            if (machine->immutable_rom_mappings[index].owns_image) {
                STD_FREE(machine->immutable_rom_mappings[index].image);
            }
        }
    }
    core_machine_trace_finalize(machine);
    core_machine_bus_finalize(machine);
    STD_FREE(machine);
}
