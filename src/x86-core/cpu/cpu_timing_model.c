#include "lib/types/types_interface.h"

#include "x86-core/cpu/cpu_instructions.h"
#include "x86-core/cpu/cpu_timing.h"

lib_i32 x86_core_cpu_timing_add_ticks(lib_u64 *value,
    lib_u64 delta)
{
    if (value == LIB_NULL || LIB_UINT64_MAX - *value < delta) return 0;
    *value += delta;
    return 1;
}

lib_u32 x86_core_cpu_instruction_prefix_count(
    const x86_core_cpu_execution_context *context, const t_cpuins_data *data)
{
    lib_u32 count = 0u;

    if (context == LIB_NULL || data == LIB_NULL ||
        (context->cpu_profile == X86_CORE_CPU_PROFILE_V30 &&
         !(data->oldcpu.data.eflags & X86_CPU_NEC_PSW_MD))) return 0u;
    while (count < sizeof(data->opcodes) && count < data->oplen &&
        x86_core_cpu_instruction_is_prefix(data->opcodes[count], context->cpu_profile)) {
        ++count;
    }
    return count;
}

lib_bool x86_core_cpu_instruction_has_lock_prefix(const t_cpuins_data *data,
    lib_u32 prefixes)
{
    lib_u32 index;

    if (data == LIB_NULL) return LIB_FALSE;
    for (index = 0u; index < prefixes && index < sizeof(data->opcodes); ++index) {
        if (data->opcodes[index] == 0xf0u) return LIB_TRUE;
    }
    return LIB_FALSE;
}

static lib_i32 x86_core_cpu_80386_timing_has_source_prefixes(
    const t_cpuins_data *data, lib_u32 prefixes)
{
    lib_u32 index;

    if (data == LIB_NULL || prefixes == 0u) return prefixes == 0u;
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

typedef enum x86_core_cpu_source_timing_form {
    X86_CORE_CPU_SOURCE_TIMING_NOP,
    X86_CORE_CPU_SOURCE_TIMING_CLC,
    X86_CORE_CPU_SOURCE_TIMING_CLD,
    X86_CORE_CPU_SOURCE_TIMING_CMC,
    X86_CORE_CPU_SOURCE_TIMING_STC,
    X86_CORE_CPU_SOURCE_TIMING_STD,
    X86_CORE_CPU_SOURCE_TIMING_STI,
    X86_CORE_CPU_SOURCE_TIMING_GROUP2_ROTATE_REGISTER,
    X86_CORE_CPU_SOURCE_TIMING_GROUP2_ROTATE_MEMORY,
    X86_CORE_CPU_SOURCE_TIMING_GROUP2_CARRY_REGISTER,
    X86_CORE_CPU_SOURCE_TIMING_GROUP2_CARRY_MEMORY,
    X86_CORE_CPU_SOURCE_TIMING_MOV_EXTEND_REGISTER,
    X86_CORE_CPU_SOURCE_TIMING_MOV_EXTEND_MEMORY,
    X86_CORE_CPU_SOURCE_TIMING_MOV_POINTER_REAL,
    X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_MUL,
    X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_IMUL,
    X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_IMUL_IMMEDIATE,
    X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_IMUL_TWO_OPERAND,
    X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_BSF,
    X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_BSR,
    X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_BT,
    X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_BTC,
    X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_BTR,
    X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_BTS,
    X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_SHLD,
    X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_SHRD,
    X86_CORE_CPU_SOURCE_TIMING_CLI,
    X86_CORE_CPU_SOURCE_TIMING_SAHF,
    X86_CORE_CPU_SOURCE_TIMING_LAHF,
    X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_REGISTER,
    X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_LOAD_MEMORY,
    X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_REGISTER_PROTECTED,
    X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_LOAD_MEMORY_PROTECTED,
    X86_CORE_CPU_SOURCE_TIMING_MOV_IMMEDIATE,
    X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_REGISTER,
    X86_CORE_CPU_SOURCE_TIMING_MOV_RM_REGISTER,
    X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_RM,
    X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_READ,
    X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_WRITE,
    X86_CORE_CPU_SOURCE_TIMING_STRING_MOVS,
    X86_CORE_CPU_SOURCE_TIMING_STRING_CMPS,
    X86_CORE_CPU_SOURCE_TIMING_STRING_STOS,
    X86_CORE_CPU_SOURCE_TIMING_STRING_LODS,
    X86_CORE_CPU_SOURCE_TIMING_STRING_SCAS,
    X86_CORE_CPU_SOURCE_TIMING_STRING_INS,
    X86_CORE_CPU_SOURCE_TIMING_STRING_OUTS,
    X86_CORE_CPU_SOURCE_TIMING_IN_IMMEDIATE,
    X86_CORE_CPU_SOURCE_TIMING_IN_DX,
    X86_CORE_CPU_SOURCE_TIMING_OUT_IMMEDIATE,
    X86_CORE_CPU_SOURCE_TIMING_OUT_DX,
    X86_CORE_CPU_SOURCE_TIMING_IN_IMMEDIATE_PROTECTED,
    X86_CORE_CPU_SOURCE_TIMING_IN_IMMEDIATE_PERMISSION,
    X86_CORE_CPU_SOURCE_TIMING_IN_DX_PROTECTED,
    X86_CORE_CPU_SOURCE_TIMING_IN_DX_PERMISSION,
    X86_CORE_CPU_SOURCE_TIMING_OUT_IMMEDIATE_PROTECTED,
    X86_CORE_CPU_SOURCE_TIMING_OUT_IMMEDIATE_PERMISSION,
    X86_CORE_CPU_SOURCE_TIMING_OUT_DX_PROTECTED,
    X86_CORE_CPU_SOURCE_TIMING_OUT_DX_PERMISSION,
    X86_CORE_CPU_SOURCE_TIMING_ALU_ADD_RM_REGISTER,
    X86_CORE_CPU_SOURCE_TIMING_ALU_ADD_REGISTER_RM,
    X86_CORE_CPU_SOURCE_TIMING_ALU_SUB_RM_REGISTER,
    X86_CORE_CPU_SOURCE_TIMING_ALU_SUB_REGISTER_RM,
    X86_CORE_CPU_SOURCE_TIMING_ALU_ACCUMULATOR_IMMEDIATE,
    X86_CORE_CPU_SOURCE_TIMING_ALU_REGISTER_IMMEDIATE,
    X86_CORE_CPU_SOURCE_TIMING_ALU_RM_IMMEDIATE,
    X86_CORE_CPU_SOURCE_TIMING_CMP_RM_REGISTER,
    X86_CORE_CPU_SOURCE_TIMING_CMP_REGISTER_RM,
    X86_CORE_CPU_SOURCE_TIMING_CMP_RM_IMMEDIATE,
    X86_CORE_CPU_SOURCE_TIMING_TEST_RM_REGISTER,
    X86_CORE_CPU_SOURCE_TIMING_TEST_ACCUMULATOR_IMMEDIATE,
    X86_CORE_CPU_SOURCE_TIMING_TEST_REGISTER_IMMEDIATE,
    X86_CORE_CPU_SOURCE_TIMING_TEST_RM_IMMEDIATE,
    X86_CORE_CPU_SOURCE_TIMING_XCHG_REGISTER,
    X86_CORE_CPU_SOURCE_TIMING_XCHG_RM_REGISTER,
    X86_CORE_CPU_SOURCE_TIMING_INC_DEC_REGISTER,
    X86_CORE_CPU_SOURCE_TIMING_INC_DEC_RM,
    X86_CORE_CPU_SOURCE_TIMING_MOV_RM_IMMEDIATE,
    X86_CORE_CPU_SOURCE_TIMING_LEA,
    X86_CORE_CPU_SOURCE_TIMING_ADJUST_SIMPLE,
    X86_CORE_CPU_SOURCE_TIMING_ADJUST_AAM,
    X86_CORE_CPU_SOURCE_TIMING_ADJUST_AAD,
    X86_CORE_CPU_SOURCE_TIMING_CONVERSION,
    X86_CORE_CPU_SOURCE_TIMING_GROUP3_NOT,
    X86_CORE_CPU_SOURCE_TIMING_GROUP3_NEG,
    X86_CORE_CPU_SOURCE_TIMING_GROUP3_MUL,
    X86_CORE_CPU_SOURCE_TIMING_GROUP3_IMUL,
    X86_CORE_CPU_SOURCE_TIMING_GROUP3_DIV,
    X86_CORE_CPU_SOURCE_TIMING_GROUP3_IDIV,
    X86_CORE_CPU_SOURCE_TIMING_IMUL_IMMEDIATE,
    X86_CORE_CPU_SOURCE_TIMING_SETCC,
    X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_DIRECT,
    X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_REGISTER,
    X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_MEMORY,
    X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_DIRECT,
    X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_MEMORY,
    X86_CORE_CPU_SOURCE_TIMING_JMP_DIRECT,
    X86_CORE_CPU_SOURCE_TIMING_JMP_REGISTER,
    X86_CORE_CPU_SOURCE_TIMING_JMP_MEMORY,
    X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_DIRECT,
    X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_MEMORY,
    X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_MEMORY_PROTECTED,
    X86_CORE_CPU_SOURCE_TIMING_RET_NEAR,
    X86_CORE_CPU_SOURCE_TIMING_RET_NEAR_IMMEDIATE,
    X86_CORE_CPU_SOURCE_TIMING_RET_FAR,
    X86_CORE_CPU_SOURCE_TIMING_RET_FAR_IMMEDIATE,
    X86_CORE_CPU_SOURCE_TIMING_PUSH_REGISTER,
    X86_CORE_CPU_SOURCE_TIMING_PUSH_MEMORY,
    X86_CORE_CPU_SOURCE_TIMING_PUSH_IMMEDIATE,
    X86_CORE_CPU_SOURCE_TIMING_POP_REGISTER,
    X86_CORE_CPU_SOURCE_TIMING_POP_MEMORY,
    X86_CORE_CPU_SOURCE_TIMING_PUSHA,
    X86_CORE_CPU_SOURCE_TIMING_POPA,
    X86_CORE_CPU_SOURCE_TIMING_PUSHF,
    X86_CORE_CPU_SOURCE_TIMING_POPF,
    X86_CORE_CPU_SOURCE_TIMING_ENTER_LEVEL_ZERO,
    X86_CORE_CPU_SOURCE_TIMING_ENTER_LEVEL_ONE,
    X86_CORE_CPU_SOURCE_TIMING_BOUND,
    X86_CORE_CPU_SOURCE_TIMING_LEAVE,
    X86_CORE_CPU_SOURCE_TIMING_JCC,
    X86_CORE_CPU_SOURCE_TIMING_INTO,
    X86_CORE_CPU_SOURCE_TIMING_HLT,
    X86_CORE_CPU_SOURCE_TIMING_INT3,
    X86_CORE_CPU_SOURCE_TIMING_INT_IMMEDIATE,
    X86_CORE_CPU_SOURCE_TIMING_IRET,
    X86_CORE_CPU_SOURCE_TIMING_8086_LOAD_POINTER,
    X86_CORE_CPU_SOURCE_TIMING_8086_RET_FAR,
    X86_CORE_CPU_SOURCE_TIMING_8086_GROUP2,
    X86_CORE_CPU_SOURCE_TIMING_8086_FLAG,
    X86_CORE_CPU_SOURCE_TIMING_WAIT,
    X86_CORE_CPU_SOURCE_TIMING_8086_ESC,
    X86_CORE_CPU_SOURCE_TIMING_8086_JCC,
    X86_CORE_CPU_SOURCE_TIMING_8086_LOOP,
    X86_CORE_CPU_SOURCE_TIMING_8086_INTO,
    X86_CORE_CPU_SOURCE_TIMING_8086_XLAT,
    X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_STORE_MEMORY
} x86_core_cpu_source_timing_form;

typedef struct x86_core_cpu_source_timing_entry {
    x86_core_cpu_source_timing_form form;
    lib_u8 ticks;
} x86_core_cpu_source_timing_entry;

typedef struct x86_core_cpu_source_repeat_timing_entry {
    x86_core_cpu_source_timing_form form;
    lib_u8 primitive_ticks;
    lib_u8 repeat_setup_ticks;
    lib_u8 repeat_iteration_ticks;
} x86_core_cpu_source_repeat_timing_entry;

typedef struct x86_core_cpu_source_repeat_timing_contract {
    const x86_core_cpu_source_repeat_timing_entry *entries;
    lib_size entry_count;
} x86_core_cpu_source_repeat_timing_contract;

/* A source evaluator consumes this immediately; it is never context state. */
typedef struct x86_core_cpu_source_transfer_plan {
    x86_core_cpu_source_timing_form form;
    lib_u8 word_transfers;
    lib_u8 complete;
} x86_core_cpu_source_transfer_plan;

/* Intel 8086 Family User's Manual, Tables 2-20 and 2-21.  The selected
 * memory rows receive the table's EA, segment-override, and odd-word
 * additions below; prefetch and bus availability remain outside this owner. */
static const x86_core_cpu_source_timing_entry
    x86_core_cpu_8086_source_timing_ledger[] = {
    { X86_CORE_CPU_SOURCE_TIMING_NOP, 3u },
    { X86_CORE_CPU_SOURCE_TIMING_CLC, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_IMMEDIATE, 4u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_REGISTER, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_RM_REGISTER, 9u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_RM, 8u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_READ, 10u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_WRITE, 10u },
    { X86_CORE_CPU_SOURCE_TIMING_IN_IMMEDIATE, 10u },
    { X86_CORE_CPU_SOURCE_TIMING_IN_DX, 8u },
    { X86_CORE_CPU_SOURCE_TIMING_OUT_IMMEDIATE, 10u },
    { X86_CORE_CPU_SOURCE_TIMING_OUT_DX, 8u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_DIRECT, 19u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_REGISTER, 16u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_MEMORY, 21u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_DIRECT, 28u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_MEMORY, 37u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_DIRECT, 15u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_REGISTER, 11u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_MEMORY, 18u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_DIRECT, 15u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_MEMORY, 24u },
    { X86_CORE_CPU_SOURCE_TIMING_RET_NEAR, 8u },
    { X86_CORE_CPU_SOURCE_TIMING_RET_NEAR_IMMEDIATE, 12u },
    { X86_CORE_CPU_SOURCE_TIMING_RET_FAR, 18u },
    { X86_CORE_CPU_SOURCE_TIMING_RET_FAR_IMMEDIATE, 17u },
    { X86_CORE_CPU_SOURCE_TIMING_PUSH_REGISTER, 11u },
    { X86_CORE_CPU_SOURCE_TIMING_PUSH_MEMORY, 16u },
    { X86_CORE_CPU_SOURCE_TIMING_POP_REGISTER, 8u },
    { X86_CORE_CPU_SOURCE_TIMING_POP_MEMORY, 17u },
    { X86_CORE_CPU_SOURCE_TIMING_PUSHF, 10u },
    { X86_CORE_CPU_SOURCE_TIMING_POPF, 8u },
    { X86_CORE_CPU_SOURCE_TIMING_HLT, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_INT3, 52u },
    { X86_CORE_CPU_SOURCE_TIMING_INT_IMMEDIATE, 51u },
    { X86_CORE_CPU_SOURCE_TIMING_INTO, 53u },
    { X86_CORE_CPU_SOURCE_TIMING_IRET, 24u }
};

/* Intel iAPX 86/88/186/188 User's Manual, Table 2-9 gives the primary
 * 80186 instruction values.  Its prefetch/no-wait and even-word assumptions
 * remain an explicit later bus/cycle-exact transfer. */
static const x86_core_cpu_source_timing_entry
    x86_core_cpu_80186_source_timing_ledger[] = {
    { X86_CORE_CPU_SOURCE_TIMING_NOP, 3u },
    { X86_CORE_CPU_SOURCE_TIMING_CLC, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_IMMEDIATE, 4u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_REGISTER, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_RM_REGISTER, 12u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_RM, 9u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_READ, 9u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_WRITE, 8u },
    { X86_CORE_CPU_SOURCE_TIMING_IN_IMMEDIATE, 10u },
    { X86_CORE_CPU_SOURCE_TIMING_IN_DX, 8u },
    { X86_CORE_CPU_SOURCE_TIMING_OUT_IMMEDIATE, 9u },
    { X86_CORE_CPU_SOURCE_TIMING_OUT_DX, 7u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_DIRECT, 15u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_REGISTER, 13u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_MEMORY, 19u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_DIRECT, 23u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_MEMORY, 38u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_DIRECT, 14u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_REGISTER, 11u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_MEMORY, 17u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_DIRECT, 14u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_MEMORY, 26u },
    { X86_CORE_CPU_SOURCE_TIMING_RET_NEAR, 16u },
    { X86_CORE_CPU_SOURCE_TIMING_RET_NEAR_IMMEDIATE, 18u },
    { X86_CORE_CPU_SOURCE_TIMING_RET_FAR, 22u },
    { X86_CORE_CPU_SOURCE_TIMING_RET_FAR_IMMEDIATE, 25u },
    { X86_CORE_CPU_SOURCE_TIMING_PUSH_REGISTER, 10u },
    { X86_CORE_CPU_SOURCE_TIMING_PUSH_MEMORY, 16u },
    { X86_CORE_CPU_SOURCE_TIMING_PUSH_IMMEDIATE, 10u },
    { X86_CORE_CPU_SOURCE_TIMING_POP_REGISTER, 10u },
    { X86_CORE_CPU_SOURCE_TIMING_POP_MEMORY, 20u },
    { X86_CORE_CPU_SOURCE_TIMING_PUSHA, 36u },
    { X86_CORE_CPU_SOURCE_TIMING_POPA, 51u },
    { X86_CORE_CPU_SOURCE_TIMING_PUSHF, 9u },
    { X86_CORE_CPU_SOURCE_TIMING_POPF, 8u },
    { X86_CORE_CPU_SOURCE_TIMING_ENTER_LEVEL_ZERO, 15u },
    { X86_CORE_CPU_SOURCE_TIMING_ENTER_LEVEL_ONE, 25u },
    { X86_CORE_CPU_SOURCE_TIMING_BOUND, 34u },
    { X86_CORE_CPU_SOURCE_TIMING_LEAVE, 8u },
    { X86_CORE_CPU_SOURCE_TIMING_HLT, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_INT3, 45u },
    { X86_CORE_CPU_SOURCE_TIMING_INT_IMMEDIATE, 47u },
    { X86_CORE_CPU_SOURCE_TIMING_IRET, 28u }
};

/* Intel 210498-005, Appendix B p. B-78: the per-instruction NOP row is three
 * clocks.  Chapter 3 p. 3-15 has a two-clock overview example; T361 S4
 * records that distinct context without making it a second runtime owner. */
static const x86_core_cpu_source_timing_entry
    x86_core_cpu_80286_source_timing_ledger[] = {
    { X86_CORE_CPU_SOURCE_TIMING_NOP, 3u },
    { X86_CORE_CPU_SOURCE_TIMING_CLC, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_IMMEDIATE, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_REGISTER, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_RM_REGISTER, 3u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_RM, 5u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_READ, 5u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_WRITE, 3u },
    { X86_CORE_CPU_SOURCE_TIMING_IN_IMMEDIATE, 5u },
    { X86_CORE_CPU_SOURCE_TIMING_IN_DX, 5u },
    { X86_CORE_CPU_SOURCE_TIMING_OUT_IMMEDIATE, 3u },
    { X86_CORE_CPU_SOURCE_TIMING_OUT_DX, 3u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_DIRECT, 7u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_REGISTER, 7u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_MEMORY, 11u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_DIRECT, 13u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_MEMORY, 16u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_DIRECT, 7u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_REGISTER, 7u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_MEMORY, 11u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_DIRECT, 11u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_MEMORY, 15u },
    { X86_CORE_CPU_SOURCE_TIMING_RET_NEAR, 11u },
    { X86_CORE_CPU_SOURCE_TIMING_RET_NEAR_IMMEDIATE, 11u },
    { X86_CORE_CPU_SOURCE_TIMING_RET_FAR, 15u },
    { X86_CORE_CPU_SOURCE_TIMING_RET_FAR_IMMEDIATE, 15u },
    { X86_CORE_CPU_SOURCE_TIMING_PUSH_REGISTER, 3u },
    { X86_CORE_CPU_SOURCE_TIMING_PUSH_MEMORY, 5u },
    { X86_CORE_CPU_SOURCE_TIMING_PUSH_IMMEDIATE, 3u },
    { X86_CORE_CPU_SOURCE_TIMING_POP_REGISTER, 5u },
    { X86_CORE_CPU_SOURCE_TIMING_POP_MEMORY, 5u },
    { X86_CORE_CPU_SOURCE_TIMING_PUSHA, 17u },
    { X86_CORE_CPU_SOURCE_TIMING_POPA, 19u },
    { X86_CORE_CPU_SOURCE_TIMING_PUSHF, 3u },
    { X86_CORE_CPU_SOURCE_TIMING_POPF, 5u },
    { X86_CORE_CPU_SOURCE_TIMING_ENTER_LEVEL_ZERO, 11u },
    { X86_CORE_CPU_SOURCE_TIMING_ENTER_LEVEL_ONE, 15u },
    { X86_CORE_CPU_SOURCE_TIMING_LEAVE, 5u },
    { X86_CORE_CPU_SOURCE_TIMING_HLT, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_INT3, 23u },
    { X86_CORE_CPU_SOURCE_TIMING_INT_IMMEDIATE, 23u },
    { X86_CORE_CPU_SOURCE_TIMING_IRET, 17u }
};

/* Intel 80386 Programmer's Reference Manual selected timing rows.  These are
 * Core clocks under the manual's prefetched/no-wait/no-HOLD assumptions; they
 * are not device service, bus arbitration, or host-time values. */
static const x86_core_cpu_source_timing_entry
    x86_core_cpu_80386_source_timing_ledger[] = {
    { X86_CORE_CPU_SOURCE_TIMING_NOP, 3u },
    { X86_CORE_CPU_SOURCE_TIMING_CLC, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_CLD, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_CMC, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_STC, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_STD, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_STI, 3u },
    { X86_CORE_CPU_SOURCE_TIMING_GROUP2_ROTATE_REGISTER, 3u },
    { X86_CORE_CPU_SOURCE_TIMING_GROUP2_ROTATE_MEMORY, 7u },
    { X86_CORE_CPU_SOURCE_TIMING_GROUP2_CARRY_REGISTER, 9u },
    { X86_CORE_CPU_SOURCE_TIMING_GROUP2_CARRY_MEMORY, 10u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_EXTEND_REGISTER, 3u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_EXTEND_MEMORY, 6u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_POINTER_REAL, 7u },
    { X86_CORE_CPU_SOURCE_TIMING_BOUND, 10u },
    { X86_CORE_CPU_SOURCE_TIMING_CLI, 3u },
    { X86_CORE_CPU_SOURCE_TIMING_SAHF, 3u },
    { X86_CORE_CPU_SOURCE_TIMING_LAHF, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_REGISTER, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_LOAD_MEMORY, 5u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_REGISTER_PROTECTED, 18u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_LOAD_MEMORY_PROTECTED, 19u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_STORE_MEMORY, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_IMMEDIATE, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_REGISTER, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_RM_REGISTER, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_RM, 4u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_READ, 4u },
    { X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_WRITE, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_IN_IMMEDIATE, 12u },
    { X86_CORE_CPU_SOURCE_TIMING_IN_DX, 13u },
    { X86_CORE_CPU_SOURCE_TIMING_OUT_IMMEDIATE, 10u },
    { X86_CORE_CPU_SOURCE_TIMING_OUT_DX, 11u },
    { X86_CORE_CPU_SOURCE_TIMING_IN_IMMEDIATE_PROTECTED, 6u },
    { X86_CORE_CPU_SOURCE_TIMING_IN_IMMEDIATE_PERMISSION, 26u },
    { X86_CORE_CPU_SOURCE_TIMING_IN_DX_PROTECTED, 7u },
    { X86_CORE_CPU_SOURCE_TIMING_IN_DX_PERMISSION, 27u },
    { X86_CORE_CPU_SOURCE_TIMING_OUT_IMMEDIATE_PROTECTED, 4u },
    { X86_CORE_CPU_SOURCE_TIMING_OUT_IMMEDIATE_PERMISSION, 24u },
    { X86_CORE_CPU_SOURCE_TIMING_OUT_DX_PROTECTED, 5u },
    { X86_CORE_CPU_SOURCE_TIMING_OUT_DX_PERMISSION, 25u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_DIRECT, 7u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_REGISTER, 7u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_MEMORY, 10u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_DIRECT, 17u },
    { X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_MEMORY, 22u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_DIRECT, 7u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_REGISTER, 7u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_MEMORY, 10u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_DIRECT, 12u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_MEMORY, 43u },
    { X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_MEMORY_PROTECTED, 31u },
    { X86_CORE_CPU_SOURCE_TIMING_RET_NEAR, 10u },
    { X86_CORE_CPU_SOURCE_TIMING_RET_NEAR_IMMEDIATE, 10u },
    { X86_CORE_CPU_SOURCE_TIMING_RET_FAR, 18u },
    { X86_CORE_CPU_SOURCE_TIMING_RET_FAR_IMMEDIATE, 18u },
    { X86_CORE_CPU_SOURCE_TIMING_PUSH_REGISTER, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_PUSH_MEMORY, 5u },
    { X86_CORE_CPU_SOURCE_TIMING_PUSH_IMMEDIATE, 2u },
    { X86_CORE_CPU_SOURCE_TIMING_POP_REGISTER, 4u },
    { X86_CORE_CPU_SOURCE_TIMING_POP_MEMORY, 5u },
    { X86_CORE_CPU_SOURCE_TIMING_PUSHA, 18u },
    { X86_CORE_CPU_SOURCE_TIMING_POPA, 24u },
    { X86_CORE_CPU_SOURCE_TIMING_PUSHF, 4u },
    { X86_CORE_CPU_SOURCE_TIMING_POPF, 5u },
    { X86_CORE_CPU_SOURCE_TIMING_ENTER_LEVEL_ZERO, 10u },
    { X86_CORE_CPU_SOURCE_TIMING_ENTER_LEVEL_ONE, 12u },
    { X86_CORE_CPU_SOURCE_TIMING_LEAVE, 4u },
    { X86_CORE_CPU_SOURCE_TIMING_HLT, 5u },
    { X86_CORE_CPU_SOURCE_TIMING_WAIT, 7u },
    { X86_CORE_CPU_SOURCE_TIMING_INT3, 33u },
    { X86_CORE_CPU_SOURCE_TIMING_INT_IMMEDIATE, 37u },
    { X86_CORE_CPU_SOURCE_TIMING_IRET, 22u }
};

/* The string rows are intentionally distinct from the ordinary instruction
 * ledger.  A REP string retires one primitive at a time in this executor, so
 * the source formula is split into its one-time setup and per-primitive parts
 * at the sole post-refresh timing publisher below. */
static const x86_core_cpu_source_repeat_timing_entry
    x86_core_cpu_8086_source_repeat_timing_ledger[] = {
    { X86_CORE_CPU_SOURCE_TIMING_STRING_MOVS, 18u, 9u, 17u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_CMPS, 22u, 9u, 22u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_STOS, 11u, 9u, 10u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_LODS, 12u, 9u, 13u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_SCAS, 15u, 9u, 15u }
};

static const x86_core_cpu_source_repeat_timing_entry
    x86_core_cpu_80186_source_repeat_timing_ledger[] = {
    { X86_CORE_CPU_SOURCE_TIMING_STRING_MOVS, 14u, 8u, 8u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_CMPS, 22u, 5u, 22u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_STOS, 10u, 6u, 9u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_LODS, 12u, 6u, 11u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_SCAS, 15u, 5u, 15u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_INS, 14u, 8u, 8u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_OUTS, 14u, 8u, 8u }
};

static const x86_core_cpu_source_repeat_timing_entry
    x86_core_cpu_80286_source_repeat_timing_ledger[] = {
    { X86_CORE_CPU_SOURCE_TIMING_STRING_MOVS, 5u, 5u, 4u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_CMPS, 8u, 5u, 9u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_STOS, 3u, 4u, 3u },
    /* Appendix B: REP LODS follows the S + 4*CX string formula. */
    { X86_CORE_CPU_SOURCE_TIMING_STRING_LODS, 5u, 5u, 4u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_SCAS, 7u, 5u, 8u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_INS, 5u, 5u, 4u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_OUTS, 5u, 5u, 4u }
};

static const x86_core_cpu_source_repeat_timing_entry
    x86_core_cpu_80386_source_repeat_timing_ledger[] = {
    { X86_CORE_CPU_SOURCE_TIMING_STRING_MOVS, 7u, 5u, 4u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_CMPS, 9u, 5u, 9u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_STOS, 4u, 5u, 5u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_LODS, 5u, 5u, 6u },
    { X86_CORE_CPU_SOURCE_TIMING_STRING_SCAS, 7u, 5u, 8u }
};

static const x86_core_cpu_source_repeat_timing_contract
    x86_core_cpu_8086_source_repeat_timing_contract = {
    x86_core_cpu_8086_source_repeat_timing_ledger,
    sizeof(x86_core_cpu_8086_source_repeat_timing_ledger) /
        sizeof(x86_core_cpu_8086_source_repeat_timing_ledger[0])
};

static const x86_core_cpu_source_repeat_timing_contract
    x86_core_cpu_80186_source_repeat_timing_contract = {
    x86_core_cpu_80186_source_repeat_timing_ledger,
    sizeof(x86_core_cpu_80186_source_repeat_timing_ledger) /
        sizeof(x86_core_cpu_80186_source_repeat_timing_ledger[0])
};

static const x86_core_cpu_source_repeat_timing_contract
    x86_core_cpu_80286_source_repeat_timing_contract = {
    x86_core_cpu_80286_source_repeat_timing_ledger,
    sizeof(x86_core_cpu_80286_source_repeat_timing_ledger) /
        sizeof(x86_core_cpu_80286_source_repeat_timing_ledger[0])
};

static const x86_core_cpu_source_repeat_timing_contract
    x86_core_cpu_80386_source_repeat_timing_contract = {
    x86_core_cpu_80386_source_repeat_timing_ledger,
    sizeof(x86_core_cpu_80386_source_repeat_timing_ledger) /
        sizeof(x86_core_cpu_80386_source_repeat_timing_ledger[0])
};

#define X86_CORE_CPU_80386_JCC_NOT_TAKEN_TICKS 3u
#define X86_CORE_CPU_80386_JCC_TAKEN_TICKS 7u
#define X86_CORE_CPU_SOURCE_UNALLOCATED_TICKS 1u

static void x86_core_cpu_source_timing_mark_unallocated(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks)
{
    if (context != LIB_NULL) context->timing_result.source_timing_unallocated = LIB_TRUE;
    if (out_ticks != LIB_NULL) *out_ticks = X86_CORE_CPU_SOURCE_UNALLOCATED_TICKS;
}
#define X86_CORE_CPU_80386_SOURCE_MAXIMUM_TICKS 106u
#define X86_CORE_CPU_8086_JCC_NOT_TAKEN_TICKS 4u
#define X86_CORE_CPU_8086_JCC_TAKEN_TICKS 16u
#define X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS 2u
#define X86_CORE_CPU_8086_ODD_WORD_TICKS 4u
#define X86_CORE_CPU_8086_SOURCE_MAXIMUM_TICKS 83u
#define X86_CORE_CPU_80186_JCC_NOT_TAKEN_TICKS 4u
#define X86_CORE_CPU_80186_JCC_TAKEN_TICKS 13u
#define X86_CORE_CPU_80186_SOURCE_MAXIMUM_TICKS 27u
#define X86_CORE_CPU_80286_JCC_NOT_TAKEN_TICKS 3u
#define X86_CORE_CPU_80286_JCC_TAKEN_TICKS 7u
#define X86_CORE_CPU_80286_BASE_INDEX_DISPLACEMENT_TICKS 1u
#define X86_CORE_CPU_80286_ODD_WORD_TICKS 2u
#define X86_CORE_CPU_80286_SOURCE_MAXIMUM_TICKS 28u

typedef struct x86_core_cpu_legacy_source_timing_contract {
    const x86_core_cpu_source_timing_entry *ledger;
    lib_size ledger_entries;
    lib_u64 jcc_not_taken_ticks;
    lib_u64 jcc_taken_ticks;
} x86_core_cpu_legacy_source_timing_contract;

static const x86_core_cpu_legacy_source_timing_contract
    x86_core_cpu_8086_source_timing_contract = {
    x86_core_cpu_8086_source_timing_ledger,
    sizeof(x86_core_cpu_8086_source_timing_ledger) /
        sizeof(x86_core_cpu_8086_source_timing_ledger[0]),
    X86_CORE_CPU_8086_JCC_NOT_TAKEN_TICKS,
    X86_CORE_CPU_8086_JCC_TAKEN_TICKS
};

static const x86_core_cpu_legacy_source_timing_contract
    x86_core_cpu_80186_source_timing_contract = {
    x86_core_cpu_80186_source_timing_ledger,
    sizeof(x86_core_cpu_80186_source_timing_ledger) /
        sizeof(x86_core_cpu_80186_source_timing_ledger[0]),
    X86_CORE_CPU_80186_JCC_NOT_TAKEN_TICKS,
    X86_CORE_CPU_80186_JCC_TAKEN_TICKS
};

static lib_u64 x86_core_cpu_source_timing_lookup(x86_core_cpu_execution_context *context,
    const x86_core_cpu_source_timing_entry *ledger, lib_size ledger_entries,
    x86_core_cpu_source_timing_form form)
{
    lib_size index;

    for (index = 0u; index < ledger_entries; ++index) {
        if (ledger[index].form == form) {
            if (context != LIB_NULL) context->timing_result.form_id = (lib_u32)form;
            return ledger[index].ticks;
        }
    }
    if (context != LIB_NULL) context->timing_result.source_timing_unallocated = LIB_TRUE;
    return X86_CORE_CPU_SOURCE_UNALLOCATED_TICKS;
}

static lib_u64 x86_core_cpu_80386_source_timing_lookup(
    x86_core_cpu_execution_context *context, x86_core_cpu_source_timing_form form)
{
    return x86_core_cpu_source_timing_lookup(context,
        x86_core_cpu_80386_source_timing_ledger,
        sizeof(x86_core_cpu_80386_source_timing_ledger) /
        sizeof(x86_core_cpu_80386_source_timing_ledger[0]), form);
}

static lib_u64 x86_core_cpu_80286_source_timing_lookup(
    x86_core_cpu_execution_context *context, x86_core_cpu_source_timing_form form);
static const x86_core_cpu_source_repeat_timing_entry
    *x86_core_cpu_source_repeat_timing_lookup(
        const x86_core_cpu_source_repeat_timing_contract *contract,
        x86_core_cpu_source_timing_form form);
static lib_i32 x86_core_cpu_source_timing_string_form(lib_u8 opcode,
    x86_core_cpu_source_timing_form *out_form);
static lib_i32 x86_core_cpu_source_timing_string_repeat_is_defined(
    x86_core_cpu_source_timing_form form, t_cpuins_data_prefix_rep prefix);
static x86_core_cpu_source_transfer_plan
    x86_core_cpu_source_timing_string_transfer_plan(
        const x86_core_cpu_execution_context *context,
        const t_cpuins_data *data, x86_core_cpu_source_timing_form form);
static lib_bool x86_core_cpu_source_timing_repeat_continuing(
    const x86_core_cpu_execution_context *context, const t_cpuins_data *data)
{
    if (!context->source_repeat_active) return LIB_FALSE;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_V30) {
        if (context->source_repeat_nec_prefix_bytes != data->nec_prefix_bytes ||
            context->source_repeat_nec_segment != data->roverds ||
            context->source_repeat_nec_lock_prefix != data->nec_lock_prefix)
            return LIB_FALSE;
    } else if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) {
        const lib_u32 bytes = x86_core_cpu_instruction_prefix_count(context, data) + 1u;

        if (bytes > sizeof(context->source_repeat_bytes) ||
            bytes != context->source_repeat_byte_count ||
            lib_memory_compare(data->opcodes, context->source_repeat_bytes, bytes) != 0)
            return LIB_FALSE;
    }
    return context->source_repeat_cs == data->oldcpu.data.cs.selector &&
        context->source_repeat_eip == data->oldcpu.data.eip &&
        context->source_repeat_opcode == (context->cpu_profile == X86_CORE_CPU_PROFILE_V30 ?
            data->nec_opcode : data->opcodes[x86_core_cpu_instruction_prefix_count(context, data)]) &&
        context->source_repeat_prefix == (lib_u8)data->prefix_rep &&
        context->source_repeat_operand_size ==
            (data->oldcpu.data.cs.seg.exec.defsize != (data->prefix_oprsize != LIB_FALSE)) &&
        context->source_repeat_address_size ==
            (data->oldcpu.data.cs.seg.exec.defsize != (data->prefix_addrsize != LIB_FALSE));
}

static lib_u64 x86_core_cpu_source_timing_repeat_string(
    x86_core_cpu_execution_context *context, const t_cpuins_data *data,
    const x86_core_cpu_source_repeat_timing_entry *entry,
    const x86_core_cpu_source_transfer_plan *transfer_plan);

static lib_i32 x86_core_cpu_80386_timing_uses_permission_map(
    const t_cpuins_data *data)
{
    lib_u32 iopl;

    if ((data->oldcpu.data.cr0 & VCPU_CR0_PE) == 0u) return 0;
    iopl = (data->oldcpu.data.eflags & VCPU_EFLAGS_IOPL) >> 12u;
    return (data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ||
        data->oldcpu.data.cs.dpl > iopl;
}

static lib_u64 x86_core_cpu_80386_source_timing_port_cost(
    x86_core_cpu_execution_context *context, const t_cpuins_data *data, x86_core_cpu_source_timing_form real_form,
    x86_core_cpu_source_timing_form protected_form,
    x86_core_cpu_source_timing_form permission_form)
{
    if ((data->oldcpu.data.cr0 & VCPU_CR0_PE) == 0u) {
        return x86_core_cpu_80386_source_timing_lookup(context, real_form);
    }
    return x86_core_cpu_80386_source_timing_lookup(context,
        x86_core_cpu_80386_timing_uses_permission_map(data) ?
        permission_form : protected_form);
}

static const x86_core_cpu_source_repeat_timing_contract
    *x86_core_cpu_source_repeat_timing_contract_for_profile(
        x86_core_cpu_profile profile)
{
    switch (profile) {
    case X86_CORE_CPU_PROFILE_8086:
    case X86_CORE_CPU_PROFILE_8088:
        return &x86_core_cpu_8086_source_repeat_timing_contract;
    case X86_CORE_CPU_PROFILE_80186:
        return &x86_core_cpu_80186_source_repeat_timing_contract;
    case X86_CORE_CPU_PROFILE_80286:
        return &x86_core_cpu_80286_source_repeat_timing_contract;
    case X86_CORE_CPU_PROFILE_80386:
        return &x86_core_cpu_80386_source_repeat_timing_contract;
    default:
        return LIB_NULL;
    }
}

static lib_i32 x86_core_cpu_80386_source_string_port_entry(
    const t_cpuins_data *data, x86_core_cpu_source_timing_form form,
    x86_core_cpu_source_repeat_timing_entry *out_entry)
{
    lib_i32 permission;

    if (data == LIB_NULL || out_entry == LIB_NULL) return 0;
    permission = x86_core_cpu_80386_timing_uses_permission_map(data);
    out_entry->form = form;
    if (form == X86_CORE_CPU_SOURCE_TIMING_STRING_INS) {
        out_entry->primitive_ticks = (data->oldcpu.data.cr0 & VCPU_CR0_PE) == 0u ?
            15u : permission ? 29u : 9u;
        out_entry->repeat_setup_ticks =
            (data->oldcpu.data.cr0 & VCPU_CR0_PE) == 0u ? 13u :
            permission ? 27u : 7u;
        out_entry->repeat_iteration_ticks = 6u;
        return 1;
    }
    if (form == X86_CORE_CPU_SOURCE_TIMING_STRING_OUTS) {
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

lib_i32 x86_core_cpu_string_io_source_instruction_cost(
    x86_core_cpu_execution_context *context, lib_u64 *out_ticks)
{
    const t_cpuins_data *data;
    const x86_core_cpu_source_repeat_timing_contract *contract;
    const x86_core_cpu_source_repeat_timing_entry *entry;
    x86_core_cpu_source_repeat_timing_entry port_entry;
    x86_core_cpu_source_transfer_plan transfer_plan = {0};
    x86_core_cpu_source_timing_form form;
    lib_u32 prefixes;
    lib_u8 opcode = 0u;

    if (context == LIB_NULL || out_ticks == LIB_NULL) return 0;
    data = &context->instructions->data;
    prefixes = x86_core_cpu_instruction_prefix_count(context, data);
    if (prefixes >= data->oplen) return 0;
    opcode = data->opcodes[prefixes];
    if (x86_core_cpu_source_timing_string_form(opcode, &form)) {
        if (!x86_core_cpu_source_timing_string_repeat_is_defined(form,
                data->prefix_rep)) {
            context->source_repeat_active = LIB_FALSE;
            return 0;
        }
        context->timing_result.form_id = (lib_u32)form;
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386 &&
            x86_core_cpu_80386_source_string_port_entry(data, form,
                &port_entry)) {
            *out_ticks = x86_core_cpu_source_timing_repeat_string(context, data,
                &port_entry, LIB_NULL);
            return 1;
        }
        contract = x86_core_cpu_source_repeat_timing_contract_for_profile(
            context->cpu_profile);
        entry = x86_core_cpu_source_repeat_timing_lookup(contract, form);
        if (entry == LIB_NULL || (data->prefix_rep != PREFIX_REP_NONE &&
            entry->repeat_iteration_ticks == 0u)) {
            context->source_repeat_active = LIB_FALSE;
            return 0;
        }
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_8088) {
            transfer_plan = x86_core_cpu_source_timing_string_transfer_plan(context, data,
                form);
            if (!transfer_plan.complete) {
                context->source_repeat_active = LIB_FALSE;
                return 0;
            }
        }
        *out_ticks = x86_core_cpu_source_timing_repeat_string(context, data,
            entry, &transfer_plan);
        return 1;
    }
    if (data->prefix_rep != PREFIX_REP_NONE) {
        context->source_repeat_active = LIB_FALSE;
        return 0;
    }
    context->source_repeat_active = LIB_FALSE;
    switch (opcode) {
    case 0xe4u: case 0xe5u:
        form = X86_CORE_CPU_SOURCE_TIMING_IN_IMMEDIATE;
        break;
    case 0xecu: case 0xedu:
        form = X86_CORE_CPU_SOURCE_TIMING_IN_DX;
        break;
    case 0xe6u: case 0xe7u:
        form = X86_CORE_CPU_SOURCE_TIMING_OUT_IMMEDIATE;
        break;
    case 0xeeu: case 0xefu:
        form = X86_CORE_CPU_SOURCE_TIMING_OUT_DX;
        break;
    default:
        return 0;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386) {
        switch (form) {
        case X86_CORE_CPU_SOURCE_TIMING_IN_IMMEDIATE:
            *out_ticks = x86_core_cpu_80386_source_timing_port_cost(context, data, form,
                X86_CORE_CPU_SOURCE_TIMING_IN_IMMEDIATE_PROTECTED,
                X86_CORE_CPU_SOURCE_TIMING_IN_IMMEDIATE_PERMISSION);
            break;
        case X86_CORE_CPU_SOURCE_TIMING_IN_DX:
            *out_ticks = x86_core_cpu_80386_source_timing_port_cost(context, data, form,
                X86_CORE_CPU_SOURCE_TIMING_IN_DX_PROTECTED,
                X86_CORE_CPU_SOURCE_TIMING_IN_DX_PERMISSION);
            break;
        case X86_CORE_CPU_SOURCE_TIMING_OUT_IMMEDIATE:
            *out_ticks = x86_core_cpu_80386_source_timing_port_cost(context, data, form,
                X86_CORE_CPU_SOURCE_TIMING_OUT_IMMEDIATE_PROTECTED,
                X86_CORE_CPU_SOURCE_TIMING_OUT_IMMEDIATE_PERMISSION);
            break;
        case X86_CORE_CPU_SOURCE_TIMING_OUT_DX:
            *out_ticks = x86_core_cpu_80386_source_timing_port_cost(context, data, form,
                X86_CORE_CPU_SOURCE_TIMING_OUT_DX_PROTECTED,
                X86_CORE_CPU_SOURCE_TIMING_OUT_DX_PERMISSION);
            break;
        default:
            return 0;
        }
        return 1;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_8086) {
        *out_ticks = x86_core_cpu_source_timing_lookup(context,
            x86_core_cpu_8086_source_timing_ledger,
            sizeof(x86_core_cpu_8086_source_timing_ledger) /
                sizeof(x86_core_cpu_8086_source_timing_ledger[0]), form);
        return 1;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_8088) {
        transfer_plan.form = form;
        transfer_plan.word_transfers = opcode & 1u;
        transfer_plan.complete = LIB_TRUE;
        *out_ticks = x86_core_cpu_source_timing_lookup(context,
            x86_core_cpu_8086_source_timing_ledger,
            sizeof(x86_core_cpu_8086_source_timing_ledger) /
                sizeof(x86_core_cpu_8086_source_timing_ledger[0]), form);
        return x86_core_cpu_timing_add_ticks(out_ticks,
            (lib_u64)transfer_plan.word_transfers *
                X86_CORE_CPU_8086_ODD_WORD_TICKS);
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80186) {
        *out_ticks = x86_core_cpu_source_timing_lookup(context,
            x86_core_cpu_80186_source_timing_ledger,
            sizeof(x86_core_cpu_80186_source_timing_ledger) /
                sizeof(x86_core_cpu_80186_source_timing_ledger[0]), form);
        return 1;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286) {
        *out_ticks = x86_core_cpu_80286_source_timing_lookup(context, form);
        return 1;
    }
    return 0;
}

static lib_i32 x86_core_cpu_source_timing_modrm_is_memory(
    const t_cpuins_data *data, lib_u32 opcode_index)
{
    return opcode_index + 1u < data->oplen &&
        (data->opcodes[opcode_index + 1u] >> 6u) != 3u;
}

static const x86_core_cpu_source_repeat_timing_entry
    *x86_core_cpu_source_repeat_timing_lookup(
        const x86_core_cpu_source_repeat_timing_contract *contract,
        x86_core_cpu_source_timing_form form)
{
    lib_size index;

    if (contract == LIB_NULL) return LIB_NULL;
    for (index = 0u; index < contract->entry_count; ++index) {
        if (contract->entries[index].form == form) return &contract->entries[index];
    }
    return LIB_NULL;
}

static lib_i32 x86_core_cpu_source_timing_string_form(lib_u8 opcode,
    x86_core_cpu_source_timing_form *out_form)
{
    if (out_form == LIB_NULL) return 0;
    switch (opcode & 0xfeu) {
    case 0xa4u:
        *out_form = X86_CORE_CPU_SOURCE_TIMING_STRING_MOVS;
        return 1;
    case 0xa6u:
        *out_form = X86_CORE_CPU_SOURCE_TIMING_STRING_CMPS;
        return 1;
    case 0xaau:
        *out_form = X86_CORE_CPU_SOURCE_TIMING_STRING_STOS;
        return 1;
    case 0xacu:
        *out_form = X86_CORE_CPU_SOURCE_TIMING_STRING_LODS;
        return 1;
    case 0xaeu:
        *out_form = X86_CORE_CPU_SOURCE_TIMING_STRING_SCAS;
        return 1;
    case 0x6cu:
        *out_form = X86_CORE_CPU_SOURCE_TIMING_STRING_INS;
        return 1;
    case 0x6eu:
        *out_form = X86_CORE_CPU_SOURCE_TIMING_STRING_OUTS;
        return 1;
    default:
        return 0;
    }
}

static lib_i32 x86_core_cpu_source_timing_string_repeat_is_defined(
    x86_core_cpu_source_timing_form form, t_cpuins_data_prefix_rep prefix)
{
    if (prefix == PREFIX_REP_NONE) return 1;
    if (form == X86_CORE_CPU_SOURCE_TIMING_STRING_CMPS ||
        form == X86_CORE_CPU_SOURCE_TIMING_STRING_SCAS) {
        return prefix == PREFIX_REP_REPZ || prefix == PREFIX_REP_REPZNZ;
    }
    return prefix == PREFIX_REP_REPZ;
}

static x86_core_cpu_source_transfer_plan
    x86_core_cpu_source_timing_string_transfer_plan(
        const x86_core_cpu_execution_context *context,
        const t_cpuins_data *data, x86_core_cpu_source_timing_form form)
{
    x86_core_cpu_source_transfer_plan plan = { form, 0u, LIB_FALSE };
    lib_u32 prefixes;

    if (data == LIB_NULL) return plan;
    prefixes = x86_core_cpu_instruction_prefix_count(context, data);
    if (prefixes >= data->oplen) return plan;
    if ((data->opcodes[prefixes] & 1u) == 0u) {
        plan.complete = LIB_TRUE;
        return plan;
    }
    switch (form) {
    case X86_CORE_CPU_SOURCE_TIMING_STRING_MOVS:
    case X86_CORE_CPU_SOURCE_TIMING_STRING_CMPS:
        plan.word_transfers = 2u;
        break;
    case X86_CORE_CPU_SOURCE_TIMING_STRING_STOS:
    case X86_CORE_CPU_SOURCE_TIMING_STRING_LODS:
    case X86_CORE_CPU_SOURCE_TIMING_STRING_SCAS:
        plan.word_transfers = 1u;
        break;
    default:
        return plan;
    }
    plan.complete = LIB_TRUE;
    return plan;
}

static lib_u64 x86_core_cpu_source_timing_string_modifiers(
    const x86_core_cpu_execution_context *context, const t_cpuins_data *data,
    x86_core_cpu_source_timing_form form,
    const x86_core_cpu_source_transfer_plan *transfer_plan)
{
    lib_u32 index;
    lib_u8 opcode = 0u;
    lib_u64 modifiers = 0u;
    lib_i32 segment_override = 0;
    lib_i32 word;
    lib_i32 source_transfer;
    lib_i32 destination_transfer;
    lib_u64 odd_word_ticks;

    if (context == LIB_NULL || data == LIB_NULL ||
        (context->cpu_profile != X86_CORE_CPU_PROFILE_8086 &&
         context->cpu_profile != X86_CORE_CPU_PROFILE_8088 &&
         context->cpu_profile != X86_CORE_CPU_PROFILE_80186 &&
         context->cpu_profile != X86_CORE_CPU_PROFILE_80286)) return 0u;
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
    if (segment_override && (form == X86_CORE_CPU_SOURCE_TIMING_STRING_MOVS ||
        form == X86_CORE_CPU_SOURCE_TIMING_STRING_CMPS ||
        form == X86_CORE_CPU_SOURCE_TIMING_STRING_LODS)) {
        modifiers += X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_8088) {
        if (transfer_plan == LIB_NULL || !transfer_plan->complete ||
            context->timing_result.repeat_phase ==
            X86_CORE_CPU_RETIREMENT_REPEAT_ZERO_COUNT) return modifiers;
        return modifiers + (lib_u64)transfer_plan->word_transfers *
            X86_CORE_CPU_8086_ODD_WORD_TICKS;
    }
    word = (opcode & 1u) != 0u;
    if (!word || context->timing_result.repeat_phase ==
        X86_CORE_CPU_RETIREMENT_REPEAT_ZERO_COUNT) return modifiers;
    source_transfer = form == X86_CORE_CPU_SOURCE_TIMING_STRING_MOVS ||
        form == X86_CORE_CPU_SOURCE_TIMING_STRING_CMPS ||
        form == X86_CORE_CPU_SOURCE_TIMING_STRING_LODS ||
        form == X86_CORE_CPU_SOURCE_TIMING_STRING_OUTS;
    destination_transfer = form == X86_CORE_CPU_SOURCE_TIMING_STRING_MOVS ||
        form == X86_CORE_CPU_SOURCE_TIMING_STRING_CMPS ||
        form == X86_CORE_CPU_SOURCE_TIMING_STRING_STOS ||
        form == X86_CORE_CPU_SOURCE_TIMING_STRING_SCAS ||
        form == X86_CORE_CPU_SOURCE_TIMING_STRING_INS;
    odd_word_ticks = context->cpu_profile == X86_CORE_CPU_PROFILE_80286 ?
        X86_CORE_CPU_80286_ODD_WORD_TICKS : X86_CORE_CPU_8086_ODD_WORD_TICKS;
    if (source_transfer && (data->oldcpu.data.si & 1u) != 0u) {
        modifiers += odd_word_ticks;
    }
    if (destination_transfer && (data->oldcpu.data.di & 1u) != 0u) {
        modifiers += odd_word_ticks;
    }
    return modifiers;
}

void x86_core_cpu_source_timing_record_repeat(
    x86_core_cpu_execution_context *context, const t_cpuins_data *data)
{
    const lib_bool operand_size = data->oldcpu.data.cs.seg.exec.defsize !=
        (data->prefix_oprsize != LIB_FALSE);
    const lib_bool address_size = data->oldcpu.data.cs.seg.exec.defsize !=
        (data->prefix_addrsize != LIB_FALSE);

    if (data->prefix_rep == PREFIX_REP_NONE) {
        context->timing_result.repeat_phase =
            X86_CORE_CPU_RETIREMENT_REPEAT_PRIMITIVE;
        context->source_repeat_active = LIB_FALSE;
        return;
    }
    const lib_u32 count = address_size ? data->oldcpu.data.ecx : data->oldcpu.data.cx;
    const lib_bool continuing = x86_core_cpu_source_timing_repeat_continuing(context, data);
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX)
        context->instructions->data.timing_repeat_previous_misses = continuing ?
            context->source_repeat_data_misses : 0u;
    context->timing_result.repeat_phase = count == 0u ?
        X86_CORE_CPU_RETIREMENT_REPEAT_ZERO_COUNT : continuing ?
        X86_CORE_CPU_RETIREMENT_REPEAT_CONTINUATION :
        X86_CORE_CPU_RETIREMENT_REPEAT_FIRST;
    context->source_repeat_active = count != 0u &&
        context->cpu->data.eip == data->oldcpu.data.eip;
    if (context->source_repeat_active) {
        context->source_repeat_cs = data->oldcpu.data.cs.selector;
        context->source_repeat_eip = data->oldcpu.data.eip;
        context->source_repeat_opcode = context->cpu_profile == X86_CORE_CPU_PROFILE_V30 ?
            data->nec_opcode : data->opcodes[x86_core_cpu_instruction_prefix_count(context, data)];
        context->source_repeat_prefix = (lib_u8)data->prefix_rep;
        context->source_repeat_operand_size = operand_size;
        context->source_repeat_address_size = address_size;
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_V30) {
            context->source_repeat_nec_prefix_bytes = data->nec_prefix_bytes;
            context->source_repeat_nec_segment = data->roverds;
            context->source_repeat_nec_lock_prefix = data->nec_lock_prefix;
        } else if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) {
            context->source_repeat_byte_count = (lib_u8)(x86_core_cpu_instruction_prefix_count(context, data) + 1u);
            lib_memory_copy(context->source_repeat_bytes, data->opcodes,
                context->source_repeat_byte_count);
        }
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80486SX) {
            context->source_repeat_data_misses = data->timing_data_misses -
                data->timing_tss_field_misses;
        }
    }
}

static lib_u64 x86_core_cpu_source_timing_repeat_string(
    x86_core_cpu_execution_context *context, const t_cpuins_data *data,
    const x86_core_cpu_source_repeat_timing_entry *entry,
    const x86_core_cpu_source_transfer_plan *transfer_plan)
{
    lib_u64 ticks;

    if (context == LIB_NULL || data == LIB_NULL || entry == LIB_NULL) return 0u;
    x86_core_cpu_source_timing_record_repeat(context, data);
    switch (context->timing_result.repeat_phase) {
    case X86_CORE_CPU_RETIREMENT_REPEAT_PRIMITIVE: ticks = entry->primitive_ticks; break;
    case X86_CORE_CPU_RETIREMENT_REPEAT_ZERO_COUNT: ticks = entry->repeat_setup_ticks; break;
    case X86_CORE_CPU_RETIREMENT_REPEAT_CONTINUATION: ticks = entry->repeat_iteration_ticks; break;
    default: ticks = entry->repeat_setup_ticks + entry->repeat_iteration_ticks; break;
    }
    return ticks + x86_core_cpu_source_timing_string_modifiers(context, data,
        entry->form, transfer_plan);
}

static lib_i32 x86_core_cpu_8086_timing_has_segment_override(
    const t_cpuins_data *data, lib_u32 prefixes)
{
    lib_u32 index;
    lib_i32 found = 0;

    if (data == LIB_NULL) return 0;
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

static lib_u64 x86_core_cpu_8086_timing_effective_address(
    const t_cpuins_data *data, lib_u32 opcode_index)
{
    lib_u8 modrm;
    lib_u8 mode;
    lib_u8 rm;

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

static lib_u64 x86_core_cpu_8086_timing_odd_word(
    const t_cpuins_data *data)
{
    return data->mrm.rsreg != LIB_NULL &&
        ((data->mrm.rsreg->base + data->mrm.offset) & 1u) != 0u ?
        X86_CORE_CPU_8086_ODD_WORD_TICKS : 0u;
}

typedef struct x86_core_cpu_primary_timing_shape {
    x86_core_cpu_source_timing_form form;
    lib_i32 memory;
    lib_i32 word;
} x86_core_cpu_primary_timing_shape;

static lib_u8 x86_core_cpu_source_timing_primary_word_transfers(
    const x86_core_cpu_primary_timing_shape *shape)
{
    if (shape == LIB_NULL || !shape->memory || !shape->word) return 0u;
    switch (shape->form) {
    case X86_CORE_CPU_SOURCE_TIMING_ALU_ADD_RM_REGISTER:
    case X86_CORE_CPU_SOURCE_TIMING_ALU_SUB_RM_REGISTER:
    case X86_CORE_CPU_SOURCE_TIMING_ALU_RM_IMMEDIATE:
    case X86_CORE_CPU_SOURCE_TIMING_XCHG_RM_REGISTER:
    case X86_CORE_CPU_SOURCE_TIMING_INC_DEC_RM:
    case X86_CORE_CPU_SOURCE_TIMING_GROUP3_NOT:
    case X86_CORE_CPU_SOURCE_TIMING_GROUP3_NEG:
        return 2u;
    case X86_CORE_CPU_SOURCE_TIMING_MOV_RM_IMMEDIATE:
    case X86_CORE_CPU_SOURCE_TIMING_MOV_RM_REGISTER:
    case X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_RM:
        return 1u;
    case X86_CORE_CPU_SOURCE_TIMING_CMP_RM_REGISTER:
    case X86_CORE_CPU_SOURCE_TIMING_CMP_REGISTER_RM:
    case X86_CORE_CPU_SOURCE_TIMING_CMP_RM_IMMEDIATE:
    case X86_CORE_CPU_SOURCE_TIMING_TEST_RM_REGISTER:
    case X86_CORE_CPU_SOURCE_TIMING_TEST_RM_IMMEDIATE:
    case X86_CORE_CPU_SOURCE_TIMING_ALU_ADD_REGISTER_RM:
    case X86_CORE_CPU_SOURCE_TIMING_ALU_SUB_REGISTER_RM:
        return 1u;
    case X86_CORE_CPU_SOURCE_TIMING_GROUP3_MUL:
    case X86_CORE_CPU_SOURCE_TIMING_GROUP3_IMUL:
    case X86_CORE_CPU_SOURCE_TIMING_GROUP3_DIV:
    case X86_CORE_CPU_SOURCE_TIMING_GROUP3_IDIV:
    case X86_CORE_CPU_SOURCE_TIMING_IMUL_IMMEDIATE:
        return 1u;
    default:
        return 0u;
    }
}

static x86_core_cpu_source_transfer_plan
    x86_core_cpu_source_timing_primary_transfer_plan(
        const x86_core_cpu_primary_timing_shape *shape)
{
    x86_core_cpu_source_transfer_plan plan = {0};

    if (shape == LIB_NULL) return plan;
    plan.form = shape->form;
    plan.word_transfers = x86_core_cpu_source_timing_primary_word_transfers(shape);
    plan.complete = LIB_TRUE;
    return plan;
}

/* This is intentionally an encoding classifier, not an instruction-handler
 * classifier.  A shared handler does not make r/m read, r/m write, and
 * register forms one timing row. */
static lib_i32 x86_core_cpu_source_timing_primary_shape(
    const t_cpuins_data *data, lib_u32 opcode_index,
    x86_core_cpu_primary_timing_shape *out_shape)
{
    lib_u8 opcode;
    lib_u8 modrm;
    lib_u8 group;
    lib_i32 memory;
    lib_i32 word;

    if (data == LIB_NULL || out_shape == LIB_NULL ||
        opcode_index >= data->oplen) return 0;
    opcode = data->opcodes[opcode_index];
    memory = x86_core_cpu_source_timing_modrm_is_memory(data, opcode_index);
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
        out_shape->form = X86_CORE_CPU_SOURCE_TIMING_IMUL_IMMEDIATE;
        return 1;
    }
    if (opcode >= 0x88u && opcode <= 0x8bu) {
        out_shape->form = !memory ?
            X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_REGISTER :
            (opcode == 0x88u || opcode == 0x89u ?
                X86_CORE_CPU_SOURCE_TIMING_MOV_RM_REGISTER :
                X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_RM);
        return 1;
    }
    if (opcode >= 0xa0u && opcode <= 0xa3u) {
        out_shape->form = opcode == 0xa0u || opcode == 0xa1u ?
            X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_READ :
            X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_WRITE;
        out_shape->memory = LIB_TRUE;
        return 1;
    }
    if (opcode >= 0xb0u && opcode <= 0xbfu) {
        out_shape->form = X86_CORE_CPU_SOURCE_TIMING_MOV_IMMEDIATE;
        out_shape->memory = LIB_FALSE;
        return 1;
    }
    if (opcode <= 0x3du && (opcode & 7u) <= 5u) {
        group = opcode >> 3u;
        if ((opcode & 7u) == 4u || (opcode & 7u) == 5u) {
            out_shape->form = group == 7u ?
                X86_CORE_CPU_SOURCE_TIMING_CMP_RM_IMMEDIATE :
                X86_CORE_CPU_SOURCE_TIMING_ALU_ACCUMULATOR_IMMEDIATE;
            out_shape->memory = LIB_FALSE;
            return 1;
        }
        if (group == 7u) {
            out_shape->form = (opcode & 2u) == 0u ?
                X86_CORE_CPU_SOURCE_TIMING_CMP_RM_REGISTER :
                X86_CORE_CPU_SOURCE_TIMING_CMP_REGISTER_RM;
            return 1;
        }
        if ((opcode & 2u) == 0u) {
            out_shape->form = group == 0u || group == 2u || group == 4u ?
                X86_CORE_CPU_SOURCE_TIMING_ALU_ADD_RM_REGISTER :
                X86_CORE_CPU_SOURCE_TIMING_ALU_SUB_RM_REGISTER;
        } else {
            out_shape->form = group == 0u || group == 2u || group == 4u ?
                X86_CORE_CPU_SOURCE_TIMING_ALU_ADD_REGISTER_RM :
                X86_CORE_CPU_SOURCE_TIMING_ALU_SUB_REGISTER_RM;
        }
        return 1;
    }

    if (opcode >= 0x80u && opcode <= 0x83u) {
        group = (modrm >> 3u) & 7u;
        out_shape->form = group == 7u ?
            X86_CORE_CPU_SOURCE_TIMING_CMP_RM_IMMEDIATE :
            X86_CORE_CPU_SOURCE_TIMING_ALU_RM_IMMEDIATE;
        return 1;
    }
    if (opcode == 0x84u || opcode == 0x85u) {
        out_shape->form = X86_CORE_CPU_SOURCE_TIMING_TEST_RM_REGISTER;
        return 1;
    }
    if (opcode == 0xa8u || opcode == 0xa9u) {
        out_shape->form = X86_CORE_CPU_SOURCE_TIMING_TEST_ACCUMULATOR_IMMEDIATE;
        out_shape->memory = LIB_FALSE;
        return 1;
    }
    if (opcode == 0x86u || opcode == 0x87u) {
        out_shape->form = memory ?
            X86_CORE_CPU_SOURCE_TIMING_XCHG_RM_REGISTER :
            X86_CORE_CPU_SOURCE_TIMING_XCHG_REGISTER;
        return 1;
    }
    if (opcode >= 0x91u && opcode <= 0x97u) {
        out_shape->form = X86_CORE_CPU_SOURCE_TIMING_XCHG_REGISTER;
        out_shape->memory = LIB_FALSE;
        out_shape->word = LIB_TRUE;
        return 1;
    }
    if (opcode >= 0x40u && opcode <= 0x4fu) {
        out_shape->form = X86_CORE_CPU_SOURCE_TIMING_INC_DEC_REGISTER;
        out_shape->memory = LIB_FALSE;
        out_shape->word = LIB_TRUE;
        return 1;
    }
    if (opcode == 0xfeu || opcode == 0xffu) {
        group = (modrm >> 3u) & 7u;
        if (group > 1u) return 0;
        out_shape->form = memory ? X86_CORE_CPU_SOURCE_TIMING_INC_DEC_RM :
            X86_CORE_CPU_SOURCE_TIMING_INC_DEC_REGISTER;
        return 1;
    }
    if (opcode == 0xf6u || opcode == 0xf7u) {
        group = (modrm >> 3u) & 7u;
        switch (group) {
        case 0u:
            out_shape->form = memory ?
                X86_CORE_CPU_SOURCE_TIMING_TEST_RM_IMMEDIATE :
                X86_CORE_CPU_SOURCE_TIMING_TEST_REGISTER_IMMEDIATE;
            break;
        case 2u:
            out_shape->form = X86_CORE_CPU_SOURCE_TIMING_GROUP3_NOT;
            break;
        case 3u:
            out_shape->form = X86_CORE_CPU_SOURCE_TIMING_GROUP3_NEG;
            break;
        case 4u:
            out_shape->form = X86_CORE_CPU_SOURCE_TIMING_GROUP3_MUL;
            break;
        case 5u:
            out_shape->form = X86_CORE_CPU_SOURCE_TIMING_GROUP3_IMUL;
            break;
        case 6u:
            out_shape->form = X86_CORE_CPU_SOURCE_TIMING_GROUP3_DIV;
            break;
        case 7u:
            out_shape->form = X86_CORE_CPU_SOURCE_TIMING_GROUP3_IDIV;
            break;
        default:
            return 0;
        }
        return 1;
    }
    if (opcode == 0xc6u || opcode == 0xc7u) {
        if (((modrm >> 3u) & 7u) != 0u) return 0;
        out_shape->form = memory ? X86_CORE_CPU_SOURCE_TIMING_MOV_RM_IMMEDIATE :
            X86_CORE_CPU_SOURCE_TIMING_MOV_IMMEDIATE;
        return 1;
    }
    if (opcode == 0x8du) {
        out_shape->form = X86_CORE_CPU_SOURCE_TIMING_LEA;
        return memory;
    }
    if (opcode == 0x27u || opcode == 0x2fu || opcode == 0x37u ||
        opcode == 0x3fu) {
        out_shape->form = X86_CORE_CPU_SOURCE_TIMING_ADJUST_SIMPLE;
        out_shape->memory = LIB_FALSE;
        return 1;
    }
    if (opcode == 0xd4u || opcode == 0xd5u) {
        out_shape->form = opcode == 0xd4u ? X86_CORE_CPU_SOURCE_TIMING_ADJUST_AAM :
            X86_CORE_CPU_SOURCE_TIMING_ADJUST_AAD;
        out_shape->memory = LIB_FALSE;
        return 1;
    }
    if (opcode == 0x98u || opcode == 0x99u) {
        out_shape->form = X86_CORE_CPU_SOURCE_TIMING_CONVERSION;
        out_shape->memory = LIB_FALSE;
        return 1;
    }
    if (opcode == 0x0fu && opcode_index + 2u < data->oplen &&
        data->opcodes[opcode_index + 1u] >= 0x90u &&
        data->opcodes[opcode_index + 1u] <= 0x9fu) {
        out_shape->form = X86_CORE_CPU_SOURCE_TIMING_SETCC;
        out_shape->memory = x86_core_cpu_source_timing_modrm_is_memory(data,
            opcode_index + 1u);
        out_shape->word = LIB_FALSE;
        return 1;
    }
    return 0;
}

/* Table 2-9 gives the immediate-IMUL rows as ranges.  S1 chose their fixed
 * midpoints, so neither the ModR/M form nor an external model may select an
 * endpoint or constrained value. */
static lib_i32 x86_core_cpu_80186_immediate_imul_midpoint_cost(
    lib_u8 opcode, lib_u64 *out_ticks)
{
    if (out_ticks == LIB_NULL) return 0;
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
 * The reference establishes a serial shift/add or restoring-divide context;
 * it cannot be used as a scalar because its wait accounting is coupled to
 * its own prefetch queue.  We therefore count that context's architectural
 * iteration decisions and normalize the result to Intel's published range.
 * No reference source is copied here. */
static lib_u64 x86_core_cpu_8086_group3_bound(
    lib_u64 minimum, lib_u64 maximum,
    lib_u64 value)
{
    if (value < minimum) return minimum;
    return value > maximum ? maximum : value;
}

static lib_u8 x86_core_cpu_8086_group3_popcount(
    lib_u64 value, lib_u8 bits)
{
    lib_u8 count = 0u;

    while (bits-- != 0u) {
        count += (lib_u8)(value & 1u);
        value >>= 1u;
    }
    return count;
}

static lib_u64 x86_core_cpu_8086_group3_magnitude(
    lib_u64 value, lib_u8 bits, lib_i32 *out_negative)
{
    lib_u64 mask = (UINT64_C(1) << bits) - 1u;
    lib_u64 sign = UINT64_C(1) << (bits - 1u);

    value &= mask;
    *out_negative = (value & sign) != 0u;
    return *out_negative ? ((~value + 1u) & mask) : value;
}

static lib_u8 x86_core_cpu_8086_group3_division_steps(
    lib_u64 dividend, lib_u64 divisor, lib_u8 bits)
{
    lib_u64 mask = (UINT64_C(1) << bits) - 1u;
    lib_u64 high = (dividend >> bits) & mask;
    lib_u64 low = dividend & mask;
    lib_u64 top = UINT64_C(1) << (bits - 1u);
    lib_u8 count = 0u;
    lib_u8 index;
    lib_i32 carry = 1;

    for (index = 0u; index < bits; ++index) {
        lib_i32 next_carry = (low & top) != 0u;

        low = ((low << 1u) | (lib_u64)carry) & mask;
        carry = (high & top) != 0u;
        high = ((high << 1u) | (lib_u64)next_carry) & mask;
        if (carry || high >= divisor) {
            high = (high - divisor) & mask;
            ++count;
        }
    }
    return count;
}

static lib_u64 x86_core_cpu_8086_group3_model_cost(
    x86_core_cpu_source_timing_form form, lib_i32 word, lib_i32 memory,
    lib_u64 operand1, lib_u64 operand2)
{
    lib_u8 bits = word ? 16u : 8u;
    lib_u64 minimum;
    lib_u64 maximum;
    lib_u64 work;
    lib_i32 operand1_negative;
    lib_i32 operand2_negative;

    switch (form) {
    case X86_CORE_CPU_SOURCE_TIMING_GROUP3_MUL:
        minimum = word ? (memory ? 124u : 118u) : (memory ? 76u : 70u);
        maximum = word ? (memory ? 139u : 133u) : (memory ? 83u : 77u);
        work = x86_core_cpu_8086_group3_popcount(operand1, bits);
        break;
    case X86_CORE_CPU_SOURCE_TIMING_GROUP3_IMUL:
        minimum = word ? (memory ? 134u : 128u) : (memory ? 86u : 80u);
        maximum = word ? (memory ? 160u : 154u) : (memory ? 104u : 98u);
        operand1 = x86_core_cpu_8086_group3_magnitude(operand1, bits,
            &operand1_negative);
        operand2 = x86_core_cpu_8086_group3_magnitude(operand2, bits,
            &operand2_negative);
        work = x86_core_cpu_8086_group3_popcount(operand1, bits) +
            (operand1_negative ? (operand2_negative ? 11u : 24u) :
                (operand2_negative ? 21u : 10u));
        break;
    case X86_CORE_CPU_SOURCE_TIMING_GROUP3_DIV:
        minimum = word ? (memory ? 150u : 144u) : (memory ? 86u : 80u);
        maximum = word ? (memory ? 168u : 162u) : (memory ? 96u : 90u);
        work = x86_core_cpu_8086_group3_division_steps(operand1, operand2,
            bits);
        break;
    case X86_CORE_CPU_SOURCE_TIMING_GROUP3_IDIV:
        minimum = word ? (memory ? 171u : 165u) : (memory ? 107u : 101u);
        maximum = word ? (memory ? 190u : 184u) : (memory ? 118u : 112u);
        operand1 = x86_core_cpu_8086_group3_magnitude(operand1, bits * 2u,
            &operand1_negative);
        operand2 = x86_core_cpu_8086_group3_magnitude(operand2, bits,
            &operand2_negative);
        work = x86_core_cpu_8086_group3_division_steps(operand1, operand2,
            bits) + (operand1_negative ? 4u : 0u) +
            (operand2_negative ? 1u : 0u) + 9u;
        break;
    default:
        return 0u;
    }
    return x86_core_cpu_8086_group3_bound(minimum, maximum, minimum + work);
}

/* The 8086 entries retain their source-manual memory additions.  The selected
 * 80186 reference model already includes effective-address time; Table 1-16
 * still supplies its independent odd-word and segment-prefix terms. */
lib_i32 x86_core_cpu_l2_dynamic_arithmetic_model_cost(
    x86_core_cpu_execution_context *context, lib_u64 *out_ticks)
{
    const t_cpuins_data *data;
    x86_core_cpu_primary_timing_shape shape;
    lib_u32 prefixes;
    lib_u8 opcode;
    lib_u64 ticks;
    lib_i32 segment_override;
    lib_i32 lock_prefix;

    if (context == LIB_NULL || out_ticks == LIB_NULL ||
        (context->cpu_profile != X86_CORE_CPU_PROFILE_8086 &&
            context->cpu_profile != X86_CORE_CPU_PROFILE_8088 &&
            context->cpu_profile != X86_CORE_CPU_PROFILE_80186)) {
        return 0;
    }
    data = &context->instructions->data;
    prefixes = x86_core_cpu_instruction_prefix_count(context, data);
    if (prefixes >= data->oplen ||
        !x86_core_cpu_source_timing_primary_shape(data, prefixes, &shape)) {
        return 0;
    }
    segment_override = x86_core_cpu_8086_timing_has_segment_override(data,
        prefixes);
    lock_prefix = x86_core_cpu_instruction_has_lock_prefix(data, prefixes);
    if (prefixes != 0u && !segment_override && !lock_prefix) return 0;
    opcode = data->opcodes[prefixes];

    if (context->cpu_profile == X86_CORE_CPU_PROFILE_8086 ||
        context->cpu_profile == X86_CORE_CPU_PROFILE_8088) {
        ticks = x86_core_cpu_8086_group3_model_cost(shape.form, shape.word,
            shape.memory, data->opr1, data->opr2);
        if (ticks == 0u) return 0;
        if (shape.memory) {
            ticks += x86_core_cpu_8086_timing_effective_address(data, prefixes);
            if (shape.word) ticks += context->cpu_profile ==
                X86_CORE_CPU_PROFILE_8088 ? X86_CORE_CPU_8086_ODD_WORD_TICKS :
                x86_core_cpu_8086_timing_odd_word(data);
            if (segment_override) ticks += X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS;
        }
        context->timing_result.form_id = (lib_u32)shape.form;
        *out_ticks = ticks;
        return 1;
    }

    switch (shape.form) {
    case X86_CORE_CPU_SOURCE_TIMING_GROUP3_MUL:
        if (!shape.memory) ticks = shape.word ? 36u : 27u;
        else ticks = shape.word ? 42u : 33u;
        break;
    case X86_CORE_CPU_SOURCE_TIMING_GROUP3_IMUL:
        if (!shape.memory) ticks = shape.word ? 36u : 27u;
        else ticks = shape.word ? 42u : 33u;
        break;
    case X86_CORE_CPU_SOURCE_TIMING_GROUP3_DIV:
        if (!shape.memory) ticks = shape.word ? 38u : 29u;
        else ticks = shape.word ? 44u : 35u;
        break;
    case X86_CORE_CPU_SOURCE_TIMING_GROUP3_IDIV:
        if (!shape.memory) ticks = shape.word ? 57u : 48u;
        else ticks = shape.word ? 63u : 54u;
        break;
    case X86_CORE_CPU_SOURCE_TIMING_IMUL_IMMEDIATE:
        if (!x86_core_cpu_80186_immediate_imul_midpoint_cost(opcode,
            &ticks)) return 0;
        break;
    default:
        return 0;
    }
    if (shape.memory) {
        if (shape.word) ticks += x86_core_cpu_8086_timing_odd_word(data);
        if (segment_override) ticks += X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS;
    }
    *out_ticks = ticks;
    return 1;
}

static lib_i32 x86_core_cpu_control_stack_short_branch_taken(
    const t_cpuins_data *data, const x86_core_cpu_execution_context *context,
    lib_u32 prefixes, lib_i32 *out_taken);

static lib_i32 x86_core_cpu_legacy_source_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks,
    const x86_core_cpu_legacy_source_timing_contract *contract)
{
    const t_cpuins_data *data = &context->instructions->data;
    lib_u32 prefixes = x86_core_cpu_instruction_prefix_count(context, data);
    lib_u8 opcode;
    lib_i32 segment_override;
    lib_i32 lock_prefix;

    if (out_ticks == LIB_NULL || contract == LIB_NULL) return 0;
    if (prefixes >= data->oplen) {
        context->source_repeat_active = LIB_FALSE;
        *out_ticks = 0u;
        return 1;
    }
    opcode = data->opcodes[prefixes];
    context->source_repeat_active = LIB_FALSE;
    segment_override = x86_core_cpu_8086_timing_has_segment_override(data,
        prefixes);
    lock_prefix = x86_core_cpu_instruction_has_lock_prefix(data, prefixes);
    /* The 8086 LOCK byte is a two-clock prefix for any following valid
     * instruction.  The common selector appends that term after this source
     * row succeeds, so accept it here together with an optional segment
     * override instead of sending otherwise-valid forms to UNALLOCATED. */
    if (prefixes != 0u && !segment_override && !lock_prefix) {
        x86_core_cpu_source_timing_mark_unallocated(context, out_ticks);
        return 1;
    }
    if (opcode >= 0x70u && opcode <= 0x7fu) {
        lib_i32 taken;

        context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_8086_JCC;
        if (!x86_core_cpu_control_stack_short_branch_taken(data, context,
                prefixes, &taken)) return 0;
        *out_ticks = taken ? contract->jcc_taken_ticks :
            contract->jcc_not_taken_ticks;
        return 1;
    }
    switch (opcode) {
    case 0x90u:
        if (prefixes != 0u && !lock_prefix) break;
        *out_ticks = x86_core_cpu_source_timing_lookup(context, contract->ledger,
            contract->ledger_entries,
            X86_CORE_CPU_SOURCE_TIMING_NOP);
        return 1;
    case 0xf8u:
        if (prefixes != 0u && !lock_prefix) break;
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_8086) {
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_8086_FLAG;
            *out_ticks = 2u;
            return 1;
        }
        *out_ticks = x86_core_cpu_source_timing_lookup(context, contract->ledger,
            contract->ledger_entries,
            X86_CORE_CPU_SOURCE_TIMING_CLC);
        return 1;
    case 0x88u: case 0x89u: case 0x8au: case 0x8bu:
        if (!data->flagMem) {
            if (prefixes != 0u && !lock_prefix) break;
            *out_ticks = x86_core_cpu_source_timing_lookup(context, contract->ledger,
                contract->ledger_entries,
                X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_REGISTER);
            return 1;
        }
        *out_ticks = x86_core_cpu_source_timing_lookup(context, contract->ledger,
            contract->ledger_entries,
            opcode == 0x88u || opcode == 0x89u ?
            X86_CORE_CPU_SOURCE_TIMING_MOV_RM_REGISTER :
            X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_RM) +
            (segment_override ? X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS : 0u) +
            ((opcode == 0x89u || opcode == 0x8bu) ?
                x86_core_cpu_8086_timing_odd_word(data) : 0u);
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_80186) {
            *out_ticks += x86_core_cpu_8086_timing_effective_address(data,
                prefixes);
        }
        return 1;
    case 0xa0u: case 0xa1u: case 0xa2u: case 0xa3u:
        *out_ticks = x86_core_cpu_source_timing_lookup(context, contract->ledger,
            contract->ledger_entries,
            opcode == 0xa0u || opcode == 0xa1u ?
            X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_READ :
            X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_WRITE) +
            (segment_override ? X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS : 0u) +
            ((opcode == 0xa1u || opcode == 0xa3u) ?
                x86_core_cpu_8086_timing_odd_word(data) : 0u);
        return 1;
    case 0x8cu:
        if ((context->cpu_profile != X86_CORE_CPU_PROFILE_8086 &&
             context->cpu_profile != X86_CORE_CPU_PROFILE_80186) ||
            prefixes + 1u >= data->oplen ||
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) > 3u) break;
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80186) {
            context->timing_result.form_id =
                X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_STORE_MEMORY;
            *out_ticks = data->flagMem ? 9u +
                x86_core_cpu_8086_timing_odd_word(data) +
                (segment_override ? X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS : 0u) :
                2u;
            return 1;
        }
        if (!data->flagMem) {
            context->timing_result.form_id =
                X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_REGISTER;
            *out_ticks = 2u;
            return 1;
        }
        context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_STORE_MEMORY;
        *out_ticks = 9u + x86_core_cpu_8086_timing_effective_address(data,
            prefixes) + x86_core_cpu_8086_timing_odd_word(data) +
            (segment_override ? X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS : 0u);
        return 1;
    case 0x8eu:
        if ((context->cpu_profile != X86_CORE_CPU_PROFILE_8086 &&
             context->cpu_profile != X86_CORE_CPU_PROFILE_80186) ||
            prefixes + 1u >= data->oplen ||
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) == 1u ||
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) > 3u) break;
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80186) {
            context->timing_result.form_id =
                X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_LOAD_MEMORY;
            *out_ticks = data->flagMem ? 11u +
                x86_core_cpu_8086_timing_odd_word(data) +
                (segment_override ? X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS : 0u) :
                2u;
            return 1;
        }
        if (!data->flagMem) {
            context->timing_result.form_id =
                X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_REGISTER;
            *out_ticks = 2u;
            return 1;
        }
        context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_LOAD_MEMORY;
        *out_ticks = 8u + x86_core_cpu_8086_timing_effective_address(data,
            prefixes) + x86_core_cpu_8086_timing_odd_word(data) +
            (segment_override ? X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS : 0u);
        return 1;
    case 0xc4u: case 0xc5u:
        if ((context->cpu_profile != X86_CORE_CPU_PROFILE_8086 &&
             context->cpu_profile != X86_CORE_CPU_PROFILE_80186) ||
            !data->flagMem) break;
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80186) {
            context->timing_result.form_id =
                X86_CORE_CPU_SOURCE_TIMING_8086_LOAD_POINTER;
            *out_ticks = 18u + (lib_u64)2u *
                x86_core_cpu_8086_timing_odd_word(data) +
                (segment_override ? X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS : 0u);
            return 1;
        }
        context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_8086_LOAD_POINTER;
        *out_ticks = 16u + x86_core_cpu_8086_timing_effective_address(data,
            prefixes) + (lib_u64)2u *
            x86_core_cpu_8086_timing_odd_word(data) +
            (segment_override ? X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS : 0u);
        return 1;
    case 0xcau: case 0xcbu:
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_8086) break;
        context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_8086_RET_FAR;
        *out_ticks = opcode == 0xcau ? 17u : 18u;
        return 1;
    case 0xc0u: case 0xc1u: case 0xd0u: case 0xd1u:
    case 0xd2u: case 0xd3u: {
        lib_u8 count;

        if ((context->cpu_profile != X86_CORE_CPU_PROFILE_8086 &&
             context->cpu_profile != X86_CORE_CPU_PROFILE_80186) ||
            prefixes + 1u >= data->oplen ||
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) == 6u) break;
        context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_8086_GROUP2;
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80186) {
            count = (opcode == 0xd0u || opcode == 0xd1u) ? 1u :
                (opcode == 0xd2u || opcode == 0xd3u) ?
                (X86_CPU_MASK_U8(data->oldcpu.data.cx) & 0x1fu) :
                (X86_CPU_MASK_U8(data->cimm) & 0x1fu);
            *out_ticks = data->flagMem ?
                ((opcode == 0xd0u || opcode == 0xd1u) ? 15u : 17u + count) :
                ((opcode == 0xd0u || opcode == 0xd1u) ? 2u : 5u + count);
            if (data->flagMem) {
                /* A word memory rotate/shift is a read-modify-write: Table
                 * 1-16 charges the odd-address term to both 16-bit transfers. */
                *out_ticks += (opcode & 1u ? (lib_u64)2u *
                    x86_core_cpu_8086_timing_odd_word(data) : 0u) +
                    (segment_override ?
                    X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS : 0u);
            }
            return 1;
        }
        if (opcode == 0xd0u || opcode == 0xd1u) {
            *out_ticks = data->flagMem ? 15u : 2u;
        } else {
            count = X86_CPU_MASK_U8(data->oldcpu.data.cx);
            *out_ticks = (data->flagMem ? 20u : 8u) + 4u * count;
        }
        if (data->flagMem) {
            *out_ticks += x86_core_cpu_8086_timing_effective_address(data,
                prefixes) + (opcode & 1u ? (lib_u64)2u *
                x86_core_cpu_8086_timing_odd_word(data) : 0u) +
                (segment_override ? X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS : 0u);
        }
        return 1;
    }
    case 0x06u: case 0x0eu: case 0x16u: case 0x1eu:
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_8086 &&
            context->cpu_profile != X86_CORE_CPU_PROFILE_80186) break;
        context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_PUSH_REGISTER;
        *out_ticks = context->cpu_profile == X86_CORE_CPU_PROFILE_80186 ?
            9u : 10u;
        return 1;
    /* 0F is POP CS only on the 8086.  It is the 0F escape on later
     * processors, so keep this source rule in the 8086 profile branch. */
    case 0x07u: case 0x17u: case 0x1fu:
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_8086 &&
            context->cpu_profile != X86_CORE_CPU_PROFILE_80186) break;
        context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_POP_REGISTER;
        *out_ticks = 8u;
        return 1;
    case 0x0fu:
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_8086) break;
        context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_POP_REGISTER;
        *out_ticks = 8u;
        return 1;
    case 0xf5u: case 0xf9u: case 0xfau: case 0xfbu: case 0xfcu:
    case 0xfdu:
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_8086 &&
            context->cpu_profile != X86_CORE_CPU_PROFILE_80186) break;
        context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_8086_FLAG;
        *out_ticks = 2u;
        return 1;
    case 0x9eu: case 0x9fu:
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_8086 &&
            context->cpu_profile != X86_CORE_CPU_PROFILE_80186) break;
        context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_8086_FLAG;
        *out_ticks = context->cpu_profile == X86_CORE_CPU_PROFILE_80186 ?
            (opcode == 0x9eu ? 3u : 2u) : 4u;
        return 1;
    case 0x9bu:
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_8086 &&
            context->cpu_profile != X86_CORE_CPU_PROFILE_80186) break;
        context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_WAIT;
        *out_ticks = context->cpu_profile == X86_CORE_CPU_PROFILE_80186 ?
            6u : 3u;
        return 1;
    case 0xd7u:
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_8086 &&
            context->cpu_profile != X86_CORE_CPU_PROFILE_80186) break;
        context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_8086_XLAT;
        *out_ticks = 11u + (context->cpu_profile == X86_CORE_CPU_PROFILE_8086 &&
            segment_override ?
            X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS : 0u);
        return 1;
    default:
        if ((context->cpu_profile == X86_CORE_CPU_PROFILE_8086 ||
             context->cpu_profile == X86_CORE_CPU_PROFILE_80186) &&
            opcode >= 0xd8u && opcode <= 0xdfu) {
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_8086_ESC;
            *out_ticks = context->cpu_profile == X86_CORE_CPU_PROFILE_80186 ?
                (data->flagMem ? 6u : 2u) : data->flagMem ? 8u +
                x86_core_cpu_8086_timing_effective_address(data, prefixes) +
                x86_core_cpu_8086_timing_odd_word(data) : 2u;
            if (data->flagMem && segment_override &&
                context->cpu_profile == X86_CORE_CPU_PROFILE_8086) {
                *out_ticks += X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS;
            }
            return 1;
        }
        if (opcode >= 0xb0u && opcode <= 0xbfu &&
            (prefixes == 0u || lock_prefix)) {
            *out_ticks = x86_core_cpu_source_timing_lookup(context, contract->ledger,
                contract->ledger_entries,
                X86_CORE_CPU_SOURCE_TIMING_MOV_IMMEDIATE);
            return 1;
        }
        break;
    }
    x86_core_cpu_source_timing_mark_unallocated(context, out_ticks);
    return 1;
}

lib_i32 x86_core_cpu_8086_source_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks)
{
    return x86_core_cpu_legacy_source_instruction_cost(context, out_ticks,
        &x86_core_cpu_8086_source_timing_contract);
}

lib_i32 x86_core_cpu_80186_source_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks)
{
    return x86_core_cpu_legacy_source_instruction_cost(context, out_ticks,
        &x86_core_cpu_80186_source_timing_contract);
}

static lib_u64 x86_core_cpu_80286_source_timing_lookup(
    x86_core_cpu_execution_context *context, x86_core_cpu_source_timing_form form)
{
    return x86_core_cpu_source_timing_lookup(context,
        x86_core_cpu_80286_source_timing_ledger,
        sizeof(x86_core_cpu_80286_source_timing_ledger) /
            sizeof(x86_core_cpu_80286_source_timing_ledger[0]), form);
}

static lib_u64 x86_core_cpu_80286_timing_effective_address(
    const t_cpuins_data *data, lib_u32 opcode_index)
{
    lib_u8 modrm;
    lib_u8 mode;
    lib_u8 rm;

    if (opcode_index + 1u >= data->oplen) return 0u;
    modrm = data->opcodes[opcode_index + 1u];
    mode = modrm >> 6u;
    rm = modrm & 7u;
    return mode != 0u && mode != 3u && rm <= 3u ?
        X86_CORE_CPU_80286_BASE_INDEX_DISPLACEMENT_TICKS : 0u;
}

static lib_u64 x86_core_cpu_80286_timing_odd_word(
    const t_cpuins_data *data)
{
    return data->mrm.rsreg != LIB_NULL &&
        ((data->mrm.rsreg->base + data->mrm.offset) & 1u) != 0u ?
        X86_CORE_CPU_80286_ODD_WORD_TICKS : 0u;
}

static lib_u8 x86_core_cpu_80286_group2_count(
    const t_cpuins_data *data, lib_u32 prefixes, lib_u8 opcode)
{
    if (opcode == 0xd2u || opcode == 0xd3u) {
        return X86_CPU_MASK_U8(data->oldcpu.data.cx) & 0x1fu;
    }
    if ((opcode == 0xc0u || opcode == 0xc1u) && prefixes + 2u < data->oplen) {
        return X86_CPU_MASK_U8(data->cimm) & 0x1fu;
    }
    return 0u;
}

static lib_i32 x86_core_cpu_80286_system_source_instruction_cost(
    x86_core_cpu_execution_context *context, lib_u64 *out_ticks);
static lib_i32 x86_core_cpu_control_stack_is_protected(
    const t_cpuins_data *data);

lib_i32 x86_core_cpu_primary_source_instruction_cost(
    x86_core_cpu_execution_context *context, lib_u64 *out_ticks)
{
    const t_cpuins_data *data;
    x86_core_cpu_primary_timing_shape shape;
    lib_u32 prefixes;
    lib_u8 opcode;
    lib_u64 ticks;
    lib_u8 transfers;
    x86_core_cpu_source_transfer_plan transfer_plan;
    lib_i32 segment_override;
    lib_i32 lock_prefix;
    lib_i32 memory;

    if (context == LIB_NULL || out_ticks == LIB_NULL) return 0;
    data = &context->instructions->data;
    prefixes = x86_core_cpu_instruction_prefix_count(context, data);
    segment_override = x86_core_cpu_8086_timing_has_segment_override(data,
        prefixes);
    lock_prefix = x86_core_cpu_instruction_has_lock_prefix(data, prefixes);
    if (prefixes >= data->oplen ||
        (prefixes != 0u &&
            ((context->cpu_profile == X86_CORE_CPU_PROFILE_80386 &&
                !x86_core_cpu_80386_timing_has_source_prefixes(data,
                    prefixes)) ||
             (context->cpu_profile != X86_CORE_CPU_PROFILE_80386 &&
                !segment_override && !lock_prefix)))) {
        return 0;
    }
    opcode = data->opcodes[prefixes];
    /* The 8088 fixed-cost rows remain owned here when LOCK prefixes a valid
     * instruction.  The selector owns LOCK's separate two-clock term. */
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_8088 &&
        (prefixes == 0u || segment_override || lock_prefix)) {
        switch (opcode) {
        case 0x90u:
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_NOP;
            *out_ticks = 3u;
            return 1;
        case 0xf8u:
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_CLC;
            *out_ticks = 2u;
            return 1;
        case 0xfcu:
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_CLD;
            *out_ticks = 2u;
            return 1;
        case 0xfau:
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_CLI;
            *out_ticks = 2u;
            return 1;
        case 0xf5u:
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_CMC;
            *out_ticks = 2u;
            return 1;
        case 0xf9u:
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_STC;
            *out_ticks = 2u;
            return 1;
        case 0xfdu:
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_STD;
            *out_ticks = 2u;
            return 1;
        case 0xfbu:
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_STI;
            *out_ticks = 2u;
            return 1;
        case 0x9eu:
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_SAHF;
            *out_ticks = 4u;
            return 1;
        case 0x9fu:
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_LAHF;
            *out_ticks = 4u;
            return 1;
        case 0x9bu:
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_WAIT;
            *out_ticks = 3u;
            return 1;
        case 0xa0u: case 0xa1u: case 0xa2u: case 0xa3u:
            context->timing_result.form_id = opcode == 0xa0u || opcode == 0xa1u ?
                X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_READ :
                X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_WRITE;
            *out_ticks = 10u + ((opcode & 1u) != 0u ?
                X86_CORE_CPU_8086_ODD_WORD_TICKS : 0u) +
                (segment_override ? X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS : 0u);
            return 1;
        default:
            break;
        }
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 &&
        opcode == 0x0fu && !lock_prefix) {
        return x86_core_cpu_80286_system_source_instruction_cost(context,
            out_ticks);
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 && prefixes == 0u &&
        opcode >= 0x70u && opcode <= 0x7fu) {
        lib_i32 taken;

        if (!x86_core_cpu_control_stack_short_branch_taken(data, context,
                prefixes, &taken)) return 0;
        *out_ticks = taken ? X86_CORE_CPU_80286_JCC_TAKEN_TICKS :
            X86_CORE_CPU_80286_JCC_NOT_TAKEN_TICKS;
        return 1;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 && prefixes == 0u &&
        opcode == 0x63u && x86_core_cpu_control_stack_is_protected(data)) {
        *out_ticks = data->flagMem ? 11u : 10u;
        return 1;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 && prefixes == 0u &&
        opcode >= 0xa0u && opcode <= 0xa3u) {
        *out_ticks = (opcode == 0xa0u || opcode == 0xa1u) ? 5u : 3u;
        if ((opcode & 1u) != 0u) {
            *out_ticks += x86_core_cpu_80286_timing_odd_word(data);
        }
        return 1;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 && prefixes == 0u &&
        opcode >= 0xd8u && opcode <= 0xdfu) {
        *out_ticks = 1u;
        return 1;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386 &&
        !data->flagLock && x86_core_cpu_80386_timing_has_source_prefixes(data,
            prefixes) && opcode == 0xd7u) {
        context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_8086_XLAT;
        *out_ticks = 5u;
        return 1;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386 &&
        !data->flagLock && prefixes == 0u && opcode == 0x9bu) {
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_WAIT);
        return 1;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386 &&
        !data->flagLock && x86_core_cpu_80386_timing_has_source_prefixes(data,
            prefixes) && opcode == 0x62u &&
        x86_core_cpu_source_timing_modrm_is_memory(data, prefixes)) {
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_BOUND);
        return 1;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386 &&
        !x86_core_cpu_control_stack_is_protected(data) && !data->flagLock &&
        x86_core_cpu_80386_timing_has_source_prefixes(data, prefixes) &&
        (opcode == 0xc4u || opcode == 0xc5u) &&
        x86_core_cpu_source_timing_modrm_is_memory(data, prefixes)) {
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_MOV_POINTER_REAL);
        return 1;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 &&
        (prefixes == 0u || segment_override) && !lock_prefix) {
        switch (opcode) {
        case 0x90u: *out_ticks = 3u; return 1;
        case 0xf8u: case 0xf5u: case 0xf9u: case 0xfcu: case 0xfdu:
        case 0xfbu: case 0x9eu: case 0x9fu:
            *out_ticks = 2u;
            return 1;
        case 0xfau: *out_ticks = 3u; return 1;
        case 0xd7u: *out_ticks = 5u; return 1;
        case 0x9bu: *out_ticks = 3u; return 1;
        default: break;
        }
    }
    memory = x86_core_cpu_source_timing_modrm_is_memory(data, prefixes);
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 && prefixes == 0u &&
        (opcode == 0xd0u || opcode == 0xd1u || opcode == 0xd2u ||
            opcode == 0xd3u || opcode == 0xc0u || opcode == 0xc1u) &&
        prefixes + 1u < data->oplen &&
        ((data->opcodes[prefixes + 1u] >> 3u) & 7u) != 6u) {
        if (opcode == 0xd0u || opcode == 0xd1u) {
            ticks = data->flagMem ? 7u + x86_core_cpu_80286_timing_effective_address(
                data, prefixes) : 2u;
        } else {
            ticks = x86_core_cpu_80286_group2_count(data, prefixes, opcode);
            ticks += data->flagMem ? 8u + x86_core_cpu_80286_timing_effective_address(
                data, prefixes) : 5u;
        }
        *out_ticks = ticks;
        return 1;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 &&
        (opcode == 0x8cu || opcode == 0x8eu) && prefixes + 1u < data->oplen &&
        ((data->opcodes[prefixes + 1u] >> 3u) & 7u) <= 3u &&
        (opcode != 0x8eu || ((data->opcodes[prefixes + 1u] >> 3u) & 7u) != 1u)) {
        if (opcode == 0x8cu) ticks = memory ? 3u : 2u;
        else if ((data->oldcpu.data.cr0 & VCPU_CR0_PE) != 0u &&
            (data->oldcpu.data.eflags & VCPU_EFLAGS_VM) == 0u) {
            ticks = memory ? 19u : 17u;
        } else ticks = memory ? 5u : 2u;
        if (memory) ticks += x86_core_cpu_80286_timing_effective_address(data,
            prefixes) + x86_core_cpu_80286_timing_odd_word(data);
        *out_ticks = ticks;
        return 1;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 &&
        (opcode == 0xc4u || opcode == 0xc5u) && data->flagMem) {
        *out_ticks = (x86_core_cpu_control_stack_is_protected(data) ? 21u : 7u) +
            x86_core_cpu_80286_timing_effective_address(data, prefixes) +
            2u * x86_core_cpu_80286_timing_odd_word(data);
        return 1;
    }
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_8088) {
        lib_u8 extension = prefixes + 1u < data->oplen ?
            (data->opcodes[prefixes + 1u] >> 3u) & 7u : 8u;

        if (opcode == 0x8cu || opcode == 0x8eu) {
            if (extension > 3u || (opcode == 0x8eu && extension == 1u)) return 0;
            context->timing_result.form_id = data->flagMem ?
                (opcode == 0x8cu ?
                X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_STORE_MEMORY :
                X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_LOAD_MEMORY) :
                X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_REGISTER;
            if (!data->flagMem) {
                if (segment_override) return 0;
                *out_ticks = 2u;
                return 1;
            }
            *out_ticks = (opcode == 0x8cu ? 9u : 8u) +
                x86_core_cpu_8086_timing_effective_address(data, prefixes) + 4u;
            if (segment_override) *out_ticks += X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS;
            return 1;
        }
        if ((opcode == 0xc4u || opcode == 0xc5u) && data->flagMem) {
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_8086_LOAD_POINTER;
            *out_ticks = 16u + x86_core_cpu_8086_timing_effective_address(data,
                prefixes) + 8u;
            if (segment_override) *out_ticks += X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS;
            return 1;
        }
        if (opcode == 0xd7u) {
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_8086_XLAT;
            *out_ticks = 15u;
            if (segment_override) *out_ticks += X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS;
            return 1;
        }
        if (opcode >= 0xd8u && opcode <= 0xdfu) {
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_8086_ESC;
            if (!data->flagMem) {
                if (segment_override) return 0;
                *out_ticks = 2u;
                return 1;
            }
            *out_ticks = 12u + x86_core_cpu_8086_timing_effective_address(data,
                prefixes);
            if (segment_override) *out_ticks += X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS;
            return 1;
        }
        if ((opcode == 0xd0u || opcode == 0xd1u || opcode == 0xd2u ||
            opcode == 0xd3u) && extension != 6u) {
            lib_u64 count = (opcode == 0xd0u || opcode == 0xd1u) ?
                1u : X86_CPU_MASK_U8(data->oldcpu.data.cx);

            if (segment_override && !data->flagMem) return 0;
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_8086_GROUP2;
            *out_ticks = data->flagMem ?
                ((opcode == 0xd0u || opcode == 0xd1u) ? 15u : 20u + 4u * count) +
                    x86_core_cpu_8086_timing_effective_address(data, prefixes) + 8u :
                ((opcode == 0xd0u || opcode == 0xd1u) ? 2u : 8u + 4u * count);
            if (segment_override) *out_ticks += X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS;
            return 1;
        }
    }
    if (!x86_core_cpu_source_timing_primary_shape(data, prefixes, &shape)) {
        return 0;
    }
    transfer_plan = x86_core_cpu_source_timing_primary_transfer_plan(&shape);
    if (!transfer_plan.complete) return 0;
    transfers = transfer_plan.word_transfers;

    switch (context->cpu_profile) {
    case X86_CORE_CPU_PROFILE_8086:
    case X86_CORE_CPU_PROFILE_8088:
        switch (shape.form) {
        case X86_CORE_CPU_SOURCE_TIMING_ALU_ADD_RM_REGISTER:
        case X86_CORE_CPU_SOURCE_TIMING_ALU_SUB_RM_REGISTER:
            ticks = shape.memory ? 16u : 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ALU_ADD_REGISTER_RM:
        case X86_CORE_CPU_SOURCE_TIMING_ALU_SUB_REGISTER_RM:
            ticks = shape.memory ? 9u : 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ALU_ACCUMULATOR_IMMEDIATE:
        case X86_CORE_CPU_SOURCE_TIMING_ALU_REGISTER_IMMEDIATE:
            ticks = 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ALU_RM_IMMEDIATE:
            ticks = shape.memory ? 17u : 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_CMP_RM_REGISTER:
        case X86_CORE_CPU_SOURCE_TIMING_CMP_REGISTER_RM:
            ticks = shape.memory ? 9u : 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_CMP_RM_IMMEDIATE:
            ticks = shape.memory ? 10u : 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_TEST_RM_REGISTER:
            ticks = shape.memory ? 9u : 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_TEST_ACCUMULATOR_IMMEDIATE:
            ticks = 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_TEST_REGISTER_IMMEDIATE:
            ticks = 5u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_TEST_RM_IMMEDIATE:
            ticks = shape.memory ? 11u : 5u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_XCHG_REGISTER:
            ticks = opcode >= 0x91u && opcode <= 0x97u ? 3u : 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_XCHG_RM_REGISTER:
            ticks = 17u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_INC_DEC_REGISTER:
            ticks = shape.word ? 2u : 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_INC_DEC_RM:
            ticks = 15u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_MOV_IMMEDIATE:
            ticks = 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_REGISTER:
            ticks = 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_MOV_RM_REGISTER:
            ticks = 9u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_RM:
            ticks = 8u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_MOV_RM_IMMEDIATE:
            ticks = shape.memory ? 10u : 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_LEA:
            ticks = 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ADJUST_SIMPLE:
            ticks = 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ADJUST_AAM:
            ticks = 83u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ADJUST_AAD:
            ticks = 60u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_CONVERSION:
            ticks = opcode == 0x99u ? 5u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_NOT:
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_NEG:
            ticks = shape.memory ? 16u : 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_MUL:
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_IMUL:
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_DIV:
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_IDIV:
        case X86_CORE_CPU_SOURCE_TIMING_IMUL_IMMEDIATE:
            return 0;
        default:
            return 0;
        }
        if (shape.memory) {
            ticks += x86_core_cpu_8086_timing_effective_address(data, prefixes);
            ticks += (lib_u64)transfer_plan.word_transfers *
                (context->cpu_profile == X86_CORE_CPU_PROFILE_8088 ?
                X86_CORE_CPU_8086_ODD_WORD_TICKS :
                x86_core_cpu_8086_timing_odd_word(data));
            if (segment_override) ticks += X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS;
        }
        break;
    case X86_CORE_CPU_PROFILE_80186:
        switch (shape.form) {
        case X86_CORE_CPU_SOURCE_TIMING_ALU_ADD_RM_REGISTER:
        case X86_CORE_CPU_SOURCE_TIMING_ALU_ADD_REGISTER_RM:
        case X86_CORE_CPU_SOURCE_TIMING_ALU_SUB_RM_REGISTER:
        case X86_CORE_CPU_SOURCE_TIMING_ALU_SUB_REGISTER_RM:
            ticks = shape.memory ? 10u : 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ALU_ACCUMULATOR_IMMEDIATE:
            ticks = 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ALU_REGISTER_IMMEDIATE:
            ticks = 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ALU_RM_IMMEDIATE:
            ticks = shape.memory ? 16u : 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_CMP_RM_REGISTER:
        case X86_CORE_CPU_SOURCE_TIMING_CMP_REGISTER_RM:
            ticks = shape.memory ? 10u : 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_CMP_RM_IMMEDIATE:
            ticks = (opcode == 0x3cu || opcode == 0x3du) ? 4u :
                (shape.memory ? 10u : (shape.word ? 4u : 3u));
            break;
        case X86_CORE_CPU_SOURCE_TIMING_TEST_RM_REGISTER:
            ticks = shape.memory ? 10u : 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_TEST_ACCUMULATOR_IMMEDIATE:
            ticks = 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_TEST_REGISTER_IMMEDIATE:
            ticks = 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_TEST_RM_IMMEDIATE:
            ticks = shape.memory ? 10u : 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_XCHG_REGISTER:
            ticks = opcode >= 0x91u ? 3u : 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_XCHG_RM_REGISTER:
            ticks = 17u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_INC_DEC_REGISTER:
            ticks = 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_INC_DEC_RM:
            ticks = 15u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_MOV_IMMEDIATE:
            ticks = 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_MOV_RM_IMMEDIATE:
            ticks = shape.memory ? 13u : 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_LEA:
            ticks = 6u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ADJUST_SIMPLE:
            if (opcode == 0x37u) ticks = 8u;
            else if (opcode == 0x3fu) ticks = 7u;
            else ticks = 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ADJUST_AAM:
            ticks = 19u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ADJUST_AAD:
            ticks = 15u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_CONVERSION:
            ticks = opcode == 0x99u ? 4u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_NOT:
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_NEG:
            ticks = 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_MUL:
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_IMUL:
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_DIV:
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_IDIV:
        case X86_CORE_CPU_SOURCE_TIMING_IMUL_IMMEDIATE:
            return 0;
        default:
            return 0;
        }
        /* Table 2-9 excludes EA work but assumes even word transfers.
         * Table 1-16 supplies the independent odd-word and segment terms. */
        if (shape.memory) {
            ticks += (lib_u64)transfers *
                x86_core_cpu_8086_timing_odd_word(data);
            if (segment_override) ticks += X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS;
        }
        break;
    case X86_CORE_CPU_PROFILE_80286:
        switch (shape.form) {
        case X86_CORE_CPU_SOURCE_TIMING_ALU_ADD_RM_REGISTER:
        case X86_CORE_CPU_SOURCE_TIMING_ALU_ADD_REGISTER_RM:
        case X86_CORE_CPU_SOURCE_TIMING_ALU_SUB_RM_REGISTER:
        case X86_CORE_CPU_SOURCE_TIMING_ALU_SUB_REGISTER_RM:
            ticks = shape.memory ? 7u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ALU_ACCUMULATOR_IMMEDIATE:
        case X86_CORE_CPU_SOURCE_TIMING_ALU_REGISTER_IMMEDIATE:
            ticks = 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ALU_RM_IMMEDIATE:
            ticks = shape.memory ? 7u : 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_CMP_RM_REGISTER:
            ticks = shape.memory ? 7u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_CMP_REGISTER_RM:
            ticks = shape.memory ? 6u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_CMP_RM_IMMEDIATE:
            ticks = shape.memory ? 6u : 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_TEST_RM_REGISTER:
            ticks = shape.memory ? 6u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_TEST_ACCUMULATOR_IMMEDIATE:
        case X86_CORE_CPU_SOURCE_TIMING_TEST_REGISTER_IMMEDIATE:
            ticks = 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_TEST_RM_IMMEDIATE:
            ticks = shape.memory ? 6u : 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_XCHG_REGISTER:
            ticks = 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_XCHG_RM_REGISTER:
            ticks = shape.memory ? 5u : 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_INC_DEC_REGISTER:
            ticks = 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_INC_DEC_RM:
            ticks = shape.memory ? 7u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_MOV_IMMEDIATE:
        case X86_CORE_CPU_SOURCE_TIMING_MOV_RM_IMMEDIATE:
            ticks = shape.memory ? 3u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_REGISTER:
            ticks = 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_MOV_RM_REGISTER:
            ticks = shape.memory ? 3u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_RM:
            ticks = shape.memory ? 5u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_READ:
            ticks = 5u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_WRITE:
            ticks = 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_LEA:
            ticks = 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ADJUST_SIMPLE:
            ticks = 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ADJUST_AAM:
            ticks = 16u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ADJUST_AAD:
            ticks = 14u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_CONVERSION:
            ticks = 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_NOT:
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_NEG:
            ticks = shape.memory ? 7u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_MUL:
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_IMUL:
        case X86_CORE_CPU_SOURCE_TIMING_IMUL_IMMEDIATE:
            ticks = shape.word ? (shape.memory ? 24u : 21u) :
                (shape.memory ? 16u : 13u);
            break;
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_DIV:
            ticks = shape.word ? (shape.memory ? 25u : 22u) :
                (shape.memory ? 17u : 14u);
            break;
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_IDIV:
            ticks = shape.word ? (shape.memory ? 28u : 25u) :
                (shape.memory ? 20u : 17u);
            break;
        default:
            return 0;
        }
        if (shape.memory) {
            ticks += x86_core_cpu_80286_timing_effective_address(data, prefixes);
            ticks += (lib_u64)transfers *
                x86_core_cpu_80286_timing_odd_word(data);
        }
        break;
    case X86_CORE_CPU_PROFILE_80386:
        switch (shape.form) {
        case X86_CORE_CPU_SOURCE_TIMING_ALU_ADD_RM_REGISTER:
            ticks = shape.memory ? 7u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ALU_ADD_REGISTER_RM:
            ticks = shape.memory ? 6u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ALU_SUB_RM_REGISTER:
            ticks = shape.memory ? 6u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ALU_SUB_REGISTER_RM:
            ticks = shape.memory ? 7u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ALU_ACCUMULATOR_IMMEDIATE:
        case X86_CORE_CPU_SOURCE_TIMING_ALU_REGISTER_IMMEDIATE:
            ticks = 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ALU_RM_IMMEDIATE:
            ticks = shape.memory ? 7u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_CMP_RM_REGISTER:
            ticks = shape.memory ? 5u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_CMP_REGISTER_RM:
            ticks = shape.memory ? 6u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_CMP_RM_IMMEDIATE:
            ticks = shape.memory ? 5u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_TEST_RM_REGISTER:
        case X86_CORE_CPU_SOURCE_TIMING_TEST_RM_IMMEDIATE:
            ticks = shape.memory ? 5u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_TEST_ACCUMULATOR_IMMEDIATE:
        case X86_CORE_CPU_SOURCE_TIMING_TEST_REGISTER_IMMEDIATE:
            ticks = 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_XCHG_REGISTER:
            ticks = 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_XCHG_RM_REGISTER:
            ticks = shape.memory ? 5u : 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_INC_DEC_REGISTER:
            ticks = 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_INC_DEC_RM:
            ticks = shape.memory ? 6u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_REGISTER:
        case X86_CORE_CPU_SOURCE_TIMING_MOV_IMMEDIATE:
            ticks = 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_MOV_RM_REGISTER:
            ticks = shape.memory ? 2u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_RM:
        case X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_READ:
            ticks = shape.memory ? 4u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_WRITE:
            ticks = 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_MOV_RM_IMMEDIATE:
            ticks = 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_LEA:
            ticks = 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ADJUST_SIMPLE:
            ticks = 4u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ADJUST_AAM:
            ticks = 17u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_ADJUST_AAD:
            ticks = 19u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_CONVERSION:
            ticks = 3u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_NOT:
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_NEG:
            ticks = shape.memory ? 6u : 2u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_DIV:
            if (!shape.word) ticks = shape.memory ? 17u : 14u;
            else if (data->prefix_oprsize) ticks = shape.memory ? 41u : 38u;
            else ticks = shape.memory ? 25u : 22u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_GROUP3_IDIV:
            if (!shape.word) ticks = shape.memory ? 22u : 19u;
            else if (data->prefix_oprsize) ticks = shape.memory ? 46u : 43u;
            else ticks = shape.memory ? 30u : 27u;
            break;
        case X86_CORE_CPU_SOURCE_TIMING_SETCC:
            ticks = shape.memory ? 5u : 4u;
            break;
        default:
            return 0;
        }
        break;
    default:
        return 0;
    }
    context->timing_result.form_id = (lib_u32)transfer_plan.form;
    *out_ticks = ticks;
    return 1;
}

/* Control and stack rows are selected after a successful instruction refresh.
 * The decoder capture supplies the executed shape, oldcpu supplies the source
 * mode, and the post-refresh CPU supplies the published transfer outcome.
 * No handler owns a clock; paths needing a gate, privilege change, task switch
 * or exception delivery deliberately remain outside this classifier. */
static lib_u64 x86_core_cpu_control_stack_source_lookup(
    x86_core_cpu_execution_context *context, x86_core_cpu_source_timing_form form)
{
    if (context == LIB_NULL) return X86_CORE_CPU_SOURCE_UNALLOCATED_TICKS;
    switch (context->cpu_profile) {
    case X86_CORE_CPU_PROFILE_8086:
    case X86_CORE_CPU_PROFILE_8088:
        return x86_core_cpu_source_timing_lookup(context,
            x86_core_cpu_8086_source_timing_ledger,
            sizeof(x86_core_cpu_8086_source_timing_ledger) /
                sizeof(x86_core_cpu_8086_source_timing_ledger[0]), form);
    case X86_CORE_CPU_PROFILE_80186:
        return x86_core_cpu_source_timing_lookup(context,
            x86_core_cpu_80186_source_timing_ledger,
            sizeof(x86_core_cpu_80186_source_timing_ledger) /
                sizeof(x86_core_cpu_80186_source_timing_ledger[0]), form);
    case X86_CORE_CPU_PROFILE_80286:
        return x86_core_cpu_80286_source_timing_lookup(context, form);
    case X86_CORE_CPU_PROFILE_80386:
        return x86_core_cpu_80386_source_timing_lookup(context, form);
    default:
        return X86_CORE_CPU_SOURCE_UNALLOCATED_TICKS;
    }
}

static lib_i32 x86_core_cpu_control_stack_is_protected(
    const t_cpuins_data *data)
{
    return data != LIB_NULL && (data->oldcpu.data.cr0 & VCPU_CR0_PE) != 0u &&
        (data->oldcpu.data.eflags & VCPU_EFLAGS_VM) == 0u;
}

static lib_i32 x86_core_cpu_control_stack_next_term(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks)
{
    x86_core_cpu_instruction_lexeme lexeme;

    if (context == LIB_NULL || out_ticks == LIB_NULL) return 0;
    if (context->cpu_profile != X86_CORE_CPU_PROFILE_80286 &&
        context->cpu_profile != X86_CORE_CPU_PROFILE_80386) {
        *out_ticks = 0u;
        return 1;
    }
    if (!x86_core_cpu_execution_preview_lexeme(
            context, &lexeme) || !lexeme.available) {
        /* Following instruction cannot be decoded: estimate one opcode component,
         * retaining the transfer's source-backed base without claiming L3. */
        *out_ticks = 1u;
        context->timing_result.retirement_origin =
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL;
        return 1;
    }
    *out_ticks = context->cpu_profile == X86_CORE_CPU_PROFILE_80286 ?
        lexeme.byte_count : lexeme.component_count;
    context->completed_observation.next_lexeme_components = lexeme.component_count;
    return 1;
}

static lib_i32 x86_core_cpu_control_stack_prefixes_are_source_backed(
    const x86_core_cpu_execution_context *context, const t_cpuins_data *data,
    lib_u32 prefixes)
{
    lib_i32 segment_override;

    if (context == LIB_NULL || data == LIB_NULL) return 0;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386) {
        return x86_core_cpu_80386_timing_has_source_prefixes(data, prefixes) &&
            !data->flagLock;
    }
    segment_override = x86_core_cpu_8086_timing_has_segment_override(data,
        prefixes);
    return prefixes == 0u || segment_override ||
        x86_core_cpu_instruction_has_lock_prefix(data, prefixes);
}

static lib_i32 x86_core_cpu_control_stack_add_next_term(x86_core_cpu_execution_context *context,
    lib_u64 base_ticks, lib_u64 *out_ticks)
{
    lib_u64 next_ticks;

    if (out_ticks == LIB_NULL ||
        !x86_core_cpu_control_stack_next_term(context, &next_ticks) ||
        !x86_core_cpu_timing_add_ticks(&base_ticks, next_ticks)) {
        return 0;
    }
    *out_ticks = base_ticks;
    return 1;
}

static lib_u64 x86_core_cpu_control_stack_memory_additions(
    const x86_core_cpu_execution_context *context, const t_cpuins_data *data,
    lib_u32 prefixes, lib_u8 word_transfers)
{
    lib_u64 ticks = 0u;

    if (context == LIB_NULL || data == LIB_NULL) return 0u;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_8086 ||
        context->cpu_profile == X86_CORE_CPU_PROFILE_8088) {
        ticks = x86_core_cpu_8086_timing_effective_address(data, prefixes) +
            (context->cpu_profile == X86_CORE_CPU_PROFILE_8086 ?
                (lib_u64)word_transfers *
                    x86_core_cpu_8086_timing_odd_word(data) : 0u);
        if (x86_core_cpu_8086_timing_has_segment_override(data, prefixes)) {
            ticks += X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS;
        }
    } else if (context->cpu_profile == X86_CORE_CPU_PROFILE_80186) {
        ticks = (lib_u64)word_transfers *
            x86_core_cpu_8086_timing_odd_word(data);
        if (x86_core_cpu_8086_timing_has_segment_override(data, prefixes)) {
            ticks += X86_CORE_CPU_8086_SEGMENT_OVERRIDE_TICKS;
        }
    } else if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286) {
        ticks = x86_core_cpu_80286_timing_effective_address(data, prefixes) +
            (lib_u64)word_transfers *
                x86_core_cpu_80286_timing_odd_word(data);
    }
    return ticks;
}

/* Table 2-21 assigns 8088 four clocks for every listed word transfer.  This
 * plan is consumed immediately by the retained evaluator; it is not CPU or
 * bus state and deliberately names only rows whose transfer count is explicit.
 */
static x86_core_cpu_source_transfer_plan
    x86_core_cpu_8088_control_stack_transfer_plan(
        x86_core_cpu_source_timing_form form)
{
    x86_core_cpu_source_transfer_plan plan = { form, 0u, LIB_TRUE };

    switch (form) {
    case X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_DIRECT:
    case X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_REGISTER:
    case X86_CORE_CPU_SOURCE_TIMING_JMP_MEMORY:
    case X86_CORE_CPU_SOURCE_TIMING_RET_NEAR:
    case X86_CORE_CPU_SOURCE_TIMING_RET_NEAR_IMMEDIATE:
    case X86_CORE_CPU_SOURCE_TIMING_PUSH_REGISTER:
    case X86_CORE_CPU_SOURCE_TIMING_POP_REGISTER:
    case X86_CORE_CPU_SOURCE_TIMING_PUSHF:
    case X86_CORE_CPU_SOURCE_TIMING_POPF:
        plan.word_transfers = 1u;
        break;
    case X86_CORE_CPU_SOURCE_TIMING_HLT:
        break;
    case X86_CORE_CPU_SOURCE_TIMING_INT3:
    case X86_CORE_CPU_SOURCE_TIMING_INT_IMMEDIATE:
    case X86_CORE_CPU_SOURCE_TIMING_INTO:
        plan.word_transfers = 5u;
        break;
    case X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_MEMORY:
    case X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_DIRECT:
    case X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_MEMORY:
    case X86_CORE_CPU_SOURCE_TIMING_RET_FAR:
    case X86_CORE_CPU_SOURCE_TIMING_RET_FAR_IMMEDIATE:
    case X86_CORE_CPU_SOURCE_TIMING_PUSH_MEMORY:
    case X86_CORE_CPU_SOURCE_TIMING_POP_MEMORY:
        plan.word_transfers = 2u;
        break;
    case X86_CORE_CPU_SOURCE_TIMING_IRET:
        plan.word_transfers = 3u;
        break;
    case X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_MEMORY:
        plan.word_transfers = 4u;
        break;
    case X86_CORE_CPU_SOURCE_TIMING_JMP_DIRECT:
    case X86_CORE_CPU_SOURCE_TIMING_JMP_REGISTER:
    case X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_DIRECT:
        break;
    default:
        plan.complete = LIB_FALSE;
        break;
    }
    return plan;
}

static lib_i32 x86_core_cpu_control_stack_source_result(x86_core_cpu_execution_context *context,
    x86_core_cpu_source_timing_form form, lib_u64 additions,
    lib_i32 include_next_term, lib_u64 *out_ticks)
{
    x86_core_cpu_source_transfer_plan transfer_plan;
    lib_u64 ticks;

    if (context == LIB_NULL || out_ticks == LIB_NULL) return 0;
    ticks = x86_core_cpu_control_stack_source_lookup(context, form);
    if (ticks == X86_CORE_CPU_SOURCE_UNALLOCATED_TICKS ||
        !x86_core_cpu_timing_add_ticks(&ticks, additions)) return 0;
    if (context->cpu_profile == X86_CORE_CPU_PROFILE_8088) {
        transfer_plan = x86_core_cpu_8088_control_stack_transfer_plan(form);
        if (!transfer_plan.complete || !x86_core_cpu_timing_add_ticks(&ticks,
                (lib_u64)transfer_plan.word_transfers *
                    X86_CORE_CPU_8086_ODD_WORD_TICKS)) return 0;
    }
    return include_next_term ? x86_core_cpu_control_stack_add_next_term(context,
        ticks, out_ticks) : ((*out_ticks = ticks), 1);
}

static lib_i32 x86_core_cpu_control_stack_direct_target_is_task_gate(
    const t_cpuins_data *data, lib_u32 prefixes, const t_cpu *cpu)
{
    lib_u16 selector;
    lib_u32 selector_index;
    lib_i32 operand32;

    if (data == LIB_NULL || cpu == LIB_NULL || prefixes >= data->oplen) {
        return 0;
    }
    operand32 = data->oldcpu.data.cs.seg.exec.defsize != data->prefix_oprsize;
    selector_index = prefixes + (operand32 ? 5u : 3u);
    if (selector_index + 1u >= data->oplen) return 0;
    selector = (lib_u16)data->opcodes[selector_index] |
        (lib_u16)((lib_u16)data->opcodes[selector_index + 1u] << 8u);
    return selector != cpu->data.tr.selector;
}

static lib_i32 x86_core_cpu_control_stack_direct_target_is_gate(
    const t_cpuins_data *data, lib_u32 prefixes, const t_cpu *cpu)
{
    lib_u16 selector;
    lib_u32 selector_index;
    lib_i32 operand32;

    if (data == LIB_NULL || cpu == LIB_NULL || prefixes >= data->oplen) {
        return 0;
    }
    operand32 = data->oldcpu.data.cs.seg.exec.defsize != data->prefix_oprsize;
    selector_index = prefixes + (operand32 ? 5u : 3u);
    if (selector_index + 1u >= data->oplen) return 0;
    selector = (lib_u16)data->opcodes[selector_index] |
        (lib_u16)((lib_u16)data->opcodes[selector_index + 1u] << 8u);
    return X86_CPU_MASK_U16(selector & VCPU_SELECTOR_IDX) !=
        X86_CPU_MASK_U16(cpu->data.cs.selector & VCPU_SELECTOR_IDX);
}

static lib_i32 x86_core_cpu_control_stack_direct_call_gate_parameters(
    x86_core_cpu_execution_context *context, const t_cpuins_data *data, lib_u32 prefixes,
    lib_u8 *out_parameters)
{
    lib_u16 selector;
    lib_u32 table_base;
    lib_u32 selector_index;
    lib_i32 operand32;

    if (context == LIB_NULL || data == LIB_NULL || out_parameters == LIB_NULL ||
        prefixes >= data->oplen) return 0;
    operand32 = data->oldcpu.data.cs.seg.exec.defsize != data->prefix_oprsize;
    selector_index = prefixes + (operand32 ? 5u : 3u);
    if (selector_index + 1u >= data->oplen) return 0;
    selector = (lib_u16)data->opcodes[selector_index] |
        (lib_u16)((lib_u16)data->opcodes[selector_index + 1u] << 8u);
    table_base = (selector & VCPU_SELECTOR_TI) != 0u ?
        data->oldcpu.data.ldtr.base : data->oldcpu.data.gdtr.base;
    return context->bus->read_memory(
        context->bus_context,
        table_base + (selector & VCPU_SELECTOR_IDX) + 4u,
        out_parameters, 1u, X86_CORE_CPU_MEMORY_ACCESS_DATA,
        LIB_TRUE, LIB_FALSE, 0u) == LIB_STATUS_OK;
}

static lib_i32 x86_core_cpu_control_stack_call_gate_parameters(x86_core_cpu_execution_context *context,
    lib_u16 selector, lib_u8 *out_parameters)
{
    lib_u32 table_base;

    if (context == LIB_NULL || out_parameters == LIB_NULL) return 0;
    table_base = (selector & VCPU_SELECTOR_TI) != 0u ?
        context->cpu->data.ldtr.base : context->cpu->data.gdtr.base;
    return context->bus->read_memory(
        context->bus_context,
        table_base + (selector & VCPU_SELECTOR_IDX) + 4u,
        out_parameters, 1u, X86_CORE_CPU_MEMORY_ACCESS_DATA,
        LIB_TRUE, LIB_FALSE, 0u) == LIB_STATUS_OK;
}

static lib_i32 x86_core_cpu_control_stack_selector_is_task_gate(x86_core_cpu_execution_context *context,
    lib_u16 selector)
{
    lib_u32 table_base;
    lib_u8 access;

    if (context == LIB_NULL) return 0;
    table_base = (selector & VCPU_SELECTOR_TI) != 0u ?
        context->cpu->data.ldtr.base : context->cpu->data.gdtr.base;
    if (context->bus->read_memory(
            context->bus_context,
            table_base + (selector & VCPU_SELECTOR_IDX) + 5u,
            &access, 1u, X86_CORE_CPU_MEMORY_ACCESS_DATA,
            LIB_TRUE, LIB_FALSE, 0u) != LIB_STATUS_OK) return 0;
    return (access & 0x0fu) == VCPU_DESC_SYS_TYPE_TASKGATE;
}

static lib_i32 x86_core_cpu_control_stack_condition_taken(lib_u32 eflags,
    lib_u8 condition, lib_i32 *out_taken)
{
    lib_i32 carry;
    lib_i32 overflow;
    lib_i32 parity;
    lib_i32 sign;
    lib_i32 zero;

    if (out_taken == LIB_NULL) return 0;
    carry = (eflags & VCPU_EFLAGS_CF) != 0u;
    parity = (eflags & VCPU_EFLAGS_PF) != 0u;
    zero = (eflags & VCPU_EFLAGS_ZF) != 0u;
    sign = (eflags & VCPU_EFLAGS_SF) != 0u;
    overflow = (eflags & VCPU_EFLAGS_OF) != 0u;
    switch (condition & 0x0fu) {
    case 0u: *out_taken = overflow; break;
    case 1u: *out_taken = !overflow; break;
    case 2u: *out_taken = carry; break;
    case 3u: *out_taken = !carry; break;
    case 4u: *out_taken = zero; break;
    case 5u: *out_taken = !zero; break;
    case 6u: *out_taken = carry || zero; break;
    case 7u: *out_taken = !carry && !zero; break;
    case 8u: *out_taken = sign; break;
    case 9u: *out_taken = !sign; break;
    case 10u: *out_taken = parity; break;
    case 11u: *out_taken = !parity; break;
    case 12u: *out_taken = sign != overflow; break;
    case 13u: *out_taken = sign == overflow; break;
    case 14u: *out_taken = zero || sign != overflow; break;
    default: *out_taken = !zero && sign == overflow; break;
    }
    return 1;
}

static lib_i32 x86_core_cpu_control_stack_short_branch_taken(
    const t_cpuins_data *data, const x86_core_cpu_execution_context *context,
    lib_u32 prefixes, lib_i32 *out_taken)
{
    lib_u8 opcode;
    lib_u32 count;

    if (data == LIB_NULL || context == LIB_NULL || out_taken == LIB_NULL ||
        (context->cpu_profile != X86_CORE_CPU_PROFILE_V30 && prefixes >= data->oplen)) return 0;
    opcode = context->cpu_profile == X86_CORE_CPU_PROFILE_V30 ?
        data->nec_opcode : data->opcodes[prefixes];
    if (opcode >= 0x70u && opcode <= 0x7fu) {
        return x86_core_cpu_control_stack_condition_taken(data->oldcpu.data.eflags,
            opcode, out_taken);
    }
    const lib_bool address32 = data->oldcpu.data.cs.seg.exec.defsize != data->prefix_addrsize;
    count = address32 ? data->oldcpu.data.ecx : data->oldcpu.data.cx;
    if (opcode == 0xe3u) {
        *out_taken = count == 0u;
        return 1;
    }
    if (opcode < 0xe0u || opcode > 0xe2u) return 0;
    count = address32 ? count - 1u : (lib_u16)(count - 1u);
    *out_taken = count != 0u && (opcode == 0xe2u ||
        (opcode == 0xe1u) == ((data->oldcpu.data.eflags & VCPU_EFLAGS_ZF) != 0u));
    return 1;
}

lib_bool x86_core_cpu_timing_branch_taken(
    const x86_core_cpu_execution_context *context, lib_u32 opcode_index, lib_bool *out_taken)
{
    if (context == LIB_NULL || context->instructions == LIB_NULL || out_taken == LIB_NULL)
        return LIB_FALSE;
    const t_cpuins_data *data = &context->instructions->data;
    const lib_bool nec = context->cpu_profile == X86_CORE_CPU_PROFILE_V30;
    if (!nec && opcode_index >= data->oplen) return LIB_FALSE;
    const lib_u8 opcode = nec ? data->nec_opcode : data->opcodes[opcode_index];
    const lib_u8 next = nec ? (opcode == 0x0fu ? data->nec_suffix : data->nec_modrm) :
        opcode_index + 1u < data->oplen ? data->opcodes[opcode_index + 1u] : 0u;
    const lib_u8 extension = (next >> 3u) & 7u;
    lib_i32 taken;

    if ((opcode >= 0x70u && opcode <= 0x7fu) || (opcode >= 0xe0u && opcode <= 0xe3u)) {
        if (!x86_core_cpu_control_stack_short_branch_taken(data, context, opcode_index, &taken))
            return LIB_FALSE;
    } else if (opcode == 0x0fu && next >= 0x80u && next <= 0x8fu) {
        if (!x86_core_cpu_control_stack_condition_taken(data->oldcpu.data.eflags, next, &taken))
            return LIB_FALSE;
    } else if (opcode == 0xe9u || opcode == 0xeau || opcode == 0xebu ||
        (opcode == 0xffu && extension >= 2u && extension <= 5u)) taken = LIB_TRUE;
    else return LIB_FALSE;
    *out_taken = taken != 0;
    return LIB_TRUE;
}

/* CALL/JMP task clocks are ts, not ts+m (386 DX PRM 17-40/17-86).
 * Columns select new 286, new 386 protected, or new 386 virtual-8086 task. */
static lib_u64 x86_core_cpu_80386_task_transfer_ticks(
    const x86_core_cpu_execution_context *context, const t_cpuins_data *data,
    lib_bool task_gate)
{
    static const lib_u16 clocks[2][3][2] = {
        {{285u,294u},{310u,316u},{229u,238u}},
        {{285u,294u},{392u,401u},{309u,321u}}
    };
    const lib_bool old32 = data->oldcpu.data.tr.sys.type ==
        VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
    const lib_u8 target = context->cpu->data.tr.sys.type ==
        VCPU_DESC_SYS_TYPE_TSS_32_BUSY ?
        ((context->cpu->data.eflags & VCPU_EFLAGS_VM) != 0u ? 2u : 1u) : 0u;

    return clocks[old32][target][task_gate];
}

lib_i32 x86_core_cpu_control_stack_source_instruction_cost(
    x86_core_cpu_execution_context *context, lib_u64 *out_ticks)
{
    const t_cpuins_data *data;
    lib_u32 prefixes;
    lib_u8 opcode;
    lib_u8 extension;
    lib_u64 ticks;
    lib_i32 memory;
    lib_i32 protected_mode;
    lib_i32 operand32;
    lib_i32 protected_execution;
    lib_i32 same_privilege;
    lib_i32 task_switch;
    lib_i32 direct_gate;

    if (context == LIB_NULL || out_ticks == LIB_NULL) return 0;
    data = &context->instructions->data;
    prefixes = x86_core_cpu_instruction_prefix_count(context, data);
    if (prefixes >= data->oplen ||
        !x86_core_cpu_control_stack_prefixes_are_source_backed(context, data,
            prefixes)) {
        return 0;
    }
    opcode = data->opcodes[prefixes];
    protected_mode = x86_core_cpu_control_stack_is_protected(data);
    operand32 = data->oldcpu.data.cs.seg.exec.defsize;
    if (data->prefix_oprsize) operand32 = !operand32;
    protected_execution = (data->oldcpu.data.cr0 & VCPU_CR0_PE) != 0u;
    same_privilege = !protected_execution ||
        ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) == 0u &&
        data->oldcpu.data.cs.dpl == context->cpu->data.cs.dpl);
    /* A successful 80286 task transfer replaces TR.  Select this published
     * outcome before the code-segment and privilege paths: task switches can
     * otherwise look like ordinary same-level far transfers after refresh. */
    task_switch = (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 ||
        context->cpu_profile == X86_CORE_CPU_PROFILE_80386) &&
        protected_execution && data->oldcpu.data.tr.selector !=
        context->cpu->data.tr.selector;
    direct_gate = (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 ||
        context->cpu_profile == X86_CORE_CPU_PROFILE_80386) &&
        protected_mode && x86_core_cpu_control_stack_direct_target_is_gate(data,
            prefixes, context->cpu);
    memory = x86_core_cpu_source_timing_modrm_is_memory(data, prefixes);
    extension = prefixes + 1u < data->oplen ?
        (data->opcodes[prefixes + 1u] >> 3u) & 7u : 8u;

    if (context->cpu_profile == X86_CORE_CPU_PROFILE_8088 &&
        opcode >= 0x70u && opcode <= 0x7fu) {
        lib_i32 taken;

        if (!x86_core_cpu_control_stack_short_branch_taken(data, context,
                prefixes, &taken)) return 0;
        context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_JCC;
        *out_ticks = taken ? 16u : 4u;
        return 1;
    }

    switch (opcode) {
    case 0x06u: case 0x0eu: case 0x16u: case 0x1eu:
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_8088) {
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_PUSH_REGISTER;
            *out_ticks = 14u;
            return 1;
        }
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_80286 &&
            context->cpu_profile != X86_CORE_CPU_PROFILE_80386) return 0;
        *out_ticks = x86_core_cpu_control_stack_source_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_PUSH_REGISTER);
        return 1;
    case 0x07u: case 0x0fu: case 0x17u: case 0x1fu:
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_8088) {
            return x86_core_cpu_control_stack_source_result(context,
                X86_CORE_CPU_SOURCE_TIMING_POP_REGISTER, 0u, 0, out_ticks);
        }
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_80286 &&
            context->cpu_profile != X86_CORE_CPU_PROFILE_80386) {
            return 0;
        }
        /* Appendix B distinguishes the protected selector-validation path
         * from the real-mode stack transfer. */
        *out_ticks = protected_mode ? 20u :
            x86_core_cpu_control_stack_source_lookup(context,
                X86_CORE_CPU_SOURCE_TIMING_POP_REGISTER);
        return 1;
    case 0xe8u:
        return x86_core_cpu_control_stack_source_result(context,
            X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_DIRECT, 0u, 1, out_ticks);
    case 0x9au:
        if (task_switch) {
            if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386) {
                (void)x86_core_cpu_control_stack_source_lookup(context,
                    X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_DIRECT);
                *out_ticks = x86_core_cpu_80386_task_transfer_ticks(context, data,
                    x86_core_cpu_control_stack_direct_target_is_task_gate(
                        data, prefixes, context->cpu) != 0);
                return 1;
            }
            *out_ticks = x86_core_cpu_control_stack_direct_target_is_task_gate(
                data, prefixes, context->cpu) ? 182u : 177u;
            return x86_core_cpu_control_stack_add_next_term(context,
                *out_ticks, out_ticks);
        }
        if (direct_gate) {
            lib_u8 parameters = 0u;

            if (!same_privilege &&
                !x86_core_cpu_control_stack_direct_call_gate_parameters(context,
                    data, prefixes, &parameters)) return 0;
            if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386) {
                (void)x86_core_cpu_control_stack_source_lookup(context,
                    X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_DIRECT);
                *out_ticks = same_privilege ? 52u :
                    (parameters == 0u ? 86u : 94u + 4u * parameters);
                return x86_core_cpu_control_stack_add_next_term(context,
                    *out_ticks, out_ticks);
            }
            *out_ticks = same_privilege ? 41u : 82u + 4u * parameters;
            return x86_core_cpu_control_stack_add_next_term(context,
                *out_ticks, out_ticks);
        }
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 &&
            protected_mode && same_privilege) {
            *out_ticks = 26u;
            return x86_core_cpu_control_stack_add_next_term(context,
                *out_ticks, out_ticks);
        }
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386 &&
            protected_mode && same_privilege) {
            (void)x86_core_cpu_control_stack_source_lookup(context,
                X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_DIRECT);
            return x86_core_cpu_control_stack_add_next_term(context, 34u,
                out_ticks);
        }
        if (!same_privilege || protected_mode) return 0;
        return x86_core_cpu_control_stack_source_result(context,
            X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_DIRECT, 0u, 1, out_ticks);
    case 0xe9u: case 0xebu:
        return x86_core_cpu_control_stack_source_result(context,
            X86_CORE_CPU_SOURCE_TIMING_JMP_DIRECT, 0u, 1, out_ticks);
    case 0xeau:
        if (task_switch) {
            if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386) {
                (void)x86_core_cpu_control_stack_source_lookup(context,
                    X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_DIRECT);
                *out_ticks = x86_core_cpu_80386_task_transfer_ticks(context, data,
                    x86_core_cpu_control_stack_direct_target_is_task_gate(
                        data, prefixes, context->cpu) != 0);
                return 1;
            }
            *out_ticks = x86_core_cpu_control_stack_direct_target_is_task_gate(
                data, prefixes, context->cpu) ? 180u : 175u;
            return x86_core_cpu_control_stack_add_next_term(context,
                *out_ticks, out_ticks);
        }
        if (direct_gate && same_privilege) {
            if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386) {
                (void)x86_core_cpu_control_stack_source_lookup(context,
                    X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_DIRECT);
                return x86_core_cpu_control_stack_add_next_term(context, 45u,
                    out_ticks);
            }
            *out_ticks = 38u;
            return x86_core_cpu_control_stack_add_next_term(context,
                *out_ticks, out_ticks);
        }
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 &&
            protected_mode && same_privilege) {
            *out_ticks = 23u;
            return x86_core_cpu_control_stack_add_next_term(context,
                *out_ticks, out_ticks);
        }
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386 &&
            protected_mode && same_privilege) {
            (void)x86_core_cpu_control_stack_source_lookup(context,
                X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_DIRECT);
            return x86_core_cpu_control_stack_add_next_term(context, 27u,
                out_ticks);
        }
        if (!same_privilege || protected_mode) return 0;
        return x86_core_cpu_control_stack_source_result(context,
            X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_DIRECT, 0u, 1, out_ticks);
    case 0xc2u:
        return x86_core_cpu_control_stack_source_result(context,
            X86_CORE_CPU_SOURCE_TIMING_RET_NEAR_IMMEDIATE, 0u, 1, out_ticks);
    case 0xc3u:
        return x86_core_cpu_control_stack_source_result(context,
            X86_CORE_CPU_SOURCE_TIMING_RET_NEAR, 0u, 1, out_ticks);
    case 0xcau:
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_8088 &&
            context->cpu_profile != X86_CORE_CPU_PROFILE_80186 &&
            context->cpu_profile != X86_CORE_CPU_PROFILE_80286 &&
            context->cpu_profile != X86_CORE_CPU_PROFILE_80386) return 0;
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 &&
            protected_mode) {
            *out_ticks = same_privilege ? 25u : 55u;
            return x86_core_cpu_control_stack_add_next_term(context,
                *out_ticks, out_ticks);
        }
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386 &&
            protected_mode) {
            (void)x86_core_cpu_control_stack_source_lookup(context,
                X86_CORE_CPU_SOURCE_TIMING_RET_FAR_IMMEDIATE);
            if (!same_privilege) {
                *out_ticks = 69u;
                return 1;
            }
            return x86_core_cpu_control_stack_add_next_term(context, 32u,
                out_ticks);
        }
        return x86_core_cpu_control_stack_source_result(context,
            X86_CORE_CPU_SOURCE_TIMING_RET_FAR_IMMEDIATE, 0u, 1, out_ticks);
    case 0xcbu:
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_8088 &&
            context->cpu_profile != X86_CORE_CPU_PROFILE_80186 &&
            context->cpu_profile != X86_CORE_CPU_PROFILE_80286 &&
            context->cpu_profile != X86_CORE_CPU_PROFILE_80386) return 0;
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 &&
            protected_mode) {
            *out_ticks = same_privilege ? 25u : 55u;
            return x86_core_cpu_control_stack_add_next_term(context,
                *out_ticks, out_ticks);
        }
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386 &&
            protected_mode) {
            (void)x86_core_cpu_control_stack_source_lookup(context,
                X86_CORE_CPU_SOURCE_TIMING_RET_FAR);
            if (!same_privilege) {
                *out_ticks = 69u;
                return 1;
            }
            return x86_core_cpu_control_stack_add_next_term(context, 32u,
                out_ticks);
        }
        return x86_core_cpu_control_stack_source_result(context,
            X86_CORE_CPU_SOURCE_TIMING_RET_FAR, 0u, 1, out_ticks);
    case 0x50u: case 0x51u: case 0x52u: case 0x53u:
    case 0x54u: case 0x55u: case 0x56u: case 0x57u:
        return x86_core_cpu_control_stack_source_result(context,
            X86_CORE_CPU_SOURCE_TIMING_PUSH_REGISTER, 0u, 0, out_ticks);
    case 0x58u: case 0x59u: case 0x5au: case 0x5bu:
    case 0x5cu: case 0x5du: case 0x5eu: case 0x5fu:
        return x86_core_cpu_control_stack_source_result(context,
            X86_CORE_CPU_SOURCE_TIMING_POP_REGISTER, 0u, 0, out_ticks);
    case 0x60u:
        *out_ticks = x86_core_cpu_control_stack_source_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_PUSHA);
        return cpu_profile_has_186_integer(context->cpu_profile);
    case 0x61u:
        *out_ticks = x86_core_cpu_control_stack_source_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_POPA);
        return cpu_profile_has_186_integer(context->cpu_profile);
    case 0x62u:
        if (!memory) return 0;
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286) {
            *out_ticks = 13u;
            return 1;
        }
        if (context->cpu_profile != X86_CORE_CPU_PROFILE_80186) return 0;
        *out_ticks = x86_core_cpu_control_stack_source_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_BOUND);
        return 1;
    case 0x68u: case 0x6au:
        *out_ticks = x86_core_cpu_control_stack_source_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_PUSH_IMMEDIATE);
        return cpu_profile_has_186_integer(context->cpu_profile);
    case 0x8fu:
        if (extension != 0u) return 0;
        return x86_core_cpu_control_stack_source_result(context, memory ?
            X86_CORE_CPU_SOURCE_TIMING_POP_MEMORY :
            X86_CORE_CPU_SOURCE_TIMING_POP_REGISTER, memory ?
            x86_core_cpu_control_stack_memory_additions(context, data, prefixes,
                1u) : 0u, 0, out_ticks);
    case 0x9cu:
        return x86_core_cpu_control_stack_source_result(context,
            X86_CORE_CPU_SOURCE_TIMING_PUSHF, 0u, 0, out_ticks);
    case 0x9du:
        return x86_core_cpu_control_stack_source_result(context,
            X86_CORE_CPU_SOURCE_TIMING_POPF, 0u, 0, out_ticks);
    case 0xc8u:
        if (!cpu_profile_has_186_integer(context->cpu_profile)) return 0;
        if (prefixes + 3u >= data->oplen) return 0;
        extension = data->opcodes[prefixes + 3u];
        if (cpu_profile_has_protected_architecture(context->cpu_profile))
            extension &= 0x1fu;
        if (extension == 0u) {
            *out_ticks = x86_core_cpu_control_stack_source_lookup(context,
                X86_CORE_CPU_SOURCE_TIMING_ENTER_LEVEL_ZERO);
        } else if (extension == 1u) {
            *out_ticks = x86_core_cpu_control_stack_source_lookup(context,
                X86_CORE_CPU_SOURCE_TIMING_ENTER_LEVEL_ONE);
        } else {
            ticks = context->cpu_profile == X86_CORE_CPU_PROFILE_80186 ?
                22u + 16u * (extension - 1u) :
                context->cpu_profile == X86_CORE_CPU_PROFILE_80286 ?
                12u + 4u * extension : 15u + 4u * (extension - 1u);
            /* ENTER's level-N row is a documented formula derived from the
             * level-one family; retain its concrete form identity rather than
             * publishing a classified-but-unattributed retirement. */
            context->timing_result.form_id =
                X86_CORE_CPU_SOURCE_TIMING_ENTER_LEVEL_ONE;
            *out_ticks = ticks;
        }
        return 1;
    case 0xc9u:
        *out_ticks = x86_core_cpu_control_stack_source_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_LEAVE);
        return cpu_profile_has_186_integer(context->cpu_profile);
    case 0xe0u: case 0xe1u: case 0xe2u: case 0xe3u: {
        lib_i32 taken;

        if (context->cpu_profile == X86_CORE_CPU_PROFILE_8086) {
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_8086_LOOP;
        }
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386) {
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_JCC;
            ticks = opcode == 0xe3u ?
                (context->cpu->data.eip == data->oldcpu.data.eip +
                    prefixes + 2u ? 5u : 9u) : 11u;
            return x86_core_cpu_control_stack_add_next_term(context, ticks,
                out_ticks);
        }
        if (!x86_core_cpu_control_stack_short_branch_taken(data, context,
                prefixes, &taken)) return 0;
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_8088) {
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_JCC;
        }
        if (opcode == 0xe3u) {
            *out_ticks = !taken ?
                (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 ? 4u :
                    context->cpu_profile == X86_CORE_CPU_PROFILE_80186 ? 5u :
                    6u) : (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 ?
                    8u : context->cpu_profile == X86_CORE_CPU_PROFILE_80186 ?
                    16u : 18u);
        } else {
            if (opcode == 0xe2u) {
                *out_ticks = taken ? (context->cpu_profile ==
                    X86_CORE_CPU_PROFILE_80286 ? 8u :
                    context->cpu_profile == X86_CORE_CPU_PROFILE_80186 ? 16u :
                    17u) : (context->cpu_profile ==
                    X86_CORE_CPU_PROFILE_80286 ? 4u : 5u);
            } else if (opcode == 0xe1u) {
                *out_ticks = taken ? (context->cpu_profile ==
                    X86_CORE_CPU_PROFILE_80286 ? 8u :
                    context->cpu_profile == X86_CORE_CPU_PROFILE_80186 ? 16u :
                    18u) : (context->cpu_profile ==
                    X86_CORE_CPU_PROFILE_80286 ? 4u : 6u);
            } else {
                *out_ticks = taken ? (context->cpu_profile ==
                    X86_CORE_CPU_PROFILE_80286 ? 8u :
                    context->cpu_profile == X86_CORE_CPU_PROFILE_80186 ? 16u :
                    19u) : (context->cpu_profile ==
                    X86_CORE_CPU_PROFILE_80286 ? 4u :
                    context->cpu_profile == X86_CORE_CPU_PROFILE_80186 ? 6u :
                    5u);
            }
        }
        return 1;
    }
    case 0xf4u:
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_8088) {
            return x86_core_cpu_control_stack_source_result(context,
                X86_CORE_CPU_SOURCE_TIMING_HLT, 0u, 0, out_ticks);
        }
        *out_ticks = x86_core_cpu_control_stack_source_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_HLT);
        return 1;
    case 0xccu: case 0xcdu:
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_8088) {
            return x86_core_cpu_control_stack_source_result(context,
                opcode == 0xccu || (prefixes + 1u < data->oplen &&
                data->opcodes[prefixes + 1u] == 3u) ?
                X86_CORE_CPU_SOURCE_TIMING_INT3 :
                X86_CORE_CPU_SOURCE_TIMING_INT_IMMEDIATE, 0u, 0, out_ticks);
        }
        if (task_switch) {
            if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386) {
                (void)x86_core_cpu_control_stack_source_lookup(context,
                    opcode == 0xccu ? X86_CORE_CPU_SOURCE_TIMING_INT3 :
                    X86_CORE_CPU_SOURCE_TIMING_INT_IMMEDIATE);
                *out_ticks = 309u;
                return 1;
            }
            *out_ticks = 167u;
            return x86_core_cpu_control_stack_add_next_term(context,
                *out_ticks, out_ticks);
        }
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 &&
            protected_mode) {
            *out_ticks = same_privilege ? 40u : 78u;
            return x86_core_cpu_control_stack_add_next_term(context,
                *out_ticks, out_ticks);
        }
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386 &&
            protected_execution) {
            (void)x86_core_cpu_control_stack_source_lookup(context,
                opcode == 0xccu ? X86_CORE_CPU_SOURCE_TIMING_INT3 :
                X86_CORE_CPU_SOURCE_TIMING_INT_IMMEDIATE);
            *out_ticks = same_privilege ? 59u :
                ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ? 119u : 99u);
            return 1;
        }
        if (!same_privilege) return 0;
        if (protected_mode) return 0;
        *out_ticks = x86_core_cpu_control_stack_source_lookup(context,
            opcode == 0xccu ? X86_CORE_CPU_SOURCE_TIMING_INT3 :
            X86_CORE_CPU_SOURCE_TIMING_INT_IMMEDIATE);
        return context->cpu_profile == X86_CORE_CPU_PROFILE_80286 ?
            x86_core_cpu_control_stack_add_next_term(context, *out_ticks,
                out_ticks) : 1;
    case 0xceu:
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_8088) {
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_INTO;
            if ((data->oldcpu.data.eflags & VCPU_EFLAGS_OF) == 0u) {
                *out_ticks = 4u;
                return 1;
            }
            return x86_core_cpu_control_stack_source_result(context,
                X86_CORE_CPU_SOURCE_TIMING_INTO, 0u, 0, out_ticks);
        }
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386) {
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_INTO;
        }
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_8086) {
            context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_8086_INTO;
        }
        if ((data->oldcpu.data.eflags & VCPU_EFLAGS_OF) == 0u) {
            *out_ticks = context->cpu_profile == X86_CORE_CPU_PROFILE_80286 ||
                context->cpu_profile == X86_CORE_CPU_PROFILE_80386 ? 3u : 4u;
            return context->cpu_profile == X86_CORE_CPU_PROFILE_80286 ?
                x86_core_cpu_control_stack_add_next_term(context, *out_ticks,
                    out_ticks) : 1;
        }
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386 &&
            protected_execution) {
            if (task_switch) {
                *out_ticks = 309u;
                return 1;
            }
            *out_ticks = same_privilege ? 59u :
                ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ? 119u : 99u);
            return 1;
        }
        if (!same_privilege || protected_mode) return 0;
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386) {
            *out_ticks = 35u;
        } else if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286) {
            *out_ticks = 24u;
        } else {
            *out_ticks = context->cpu_profile == X86_CORE_CPU_PROFILE_80186 ?
                48u : 53u;
        }
        return context->cpu_profile == X86_CORE_CPU_PROFILE_80286 ?
            x86_core_cpu_control_stack_add_next_term(context, *out_ticks,
                out_ticks) : 1;
    case 0xcfu:
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_8088) {
            return x86_core_cpu_control_stack_source_result(context,
                X86_CORE_CPU_SOURCE_TIMING_IRET, 0u, 0, out_ticks);
        }
        if (task_switch) {
            if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386) {
                (void)x86_core_cpu_control_stack_source_lookup(context,
                    X86_CORE_CPU_SOURCE_TIMING_IRET);
                *out_ticks = 275u;
                return 1;
            }
            *out_ticks = 169u;
            return x86_core_cpu_control_stack_add_next_term(context,
                *out_ticks, out_ticks);
        }
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 &&
            protected_mode) {
            *out_ticks = same_privilege ? 31u : 55u;
            return x86_core_cpu_control_stack_add_next_term(context,
                *out_ticks, out_ticks);
        }
        if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386 &&
            protected_mode) {
            (void)x86_core_cpu_control_stack_source_lookup(context,
                X86_CORE_CPU_SOURCE_TIMING_IRET);
            if ((context->cpu->data.eflags & VCPU_EFLAGS_VM) != 0u) {
                *out_ticks = 60u;
                return 1;
            }
            if (!same_privilege) {
                *out_ticks = 82u;
                return 1;
            }
            *out_ticks = 38u;
            return 1;
        }
        if (!same_privilege || protected_mode) return 0;
        *out_ticks = x86_core_cpu_control_stack_source_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_IRET);
        return context->cpu_profile == X86_CORE_CPU_PROFILE_80286 ?
            x86_core_cpu_control_stack_add_next_term(context, *out_ticks,
                out_ticks) : 1;
    case 0xffu:
        if (extension == 2u || extension == 4u) {
            return x86_core_cpu_control_stack_source_result(context,
                memory ? (extension == 2u ?
                    X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_MEMORY :
                    X86_CORE_CPU_SOURCE_TIMING_JMP_MEMORY) :
                (extension == 2u ?
                    X86_CORE_CPU_SOURCE_TIMING_CALL_NEAR_REGISTER :
                    X86_CORE_CPU_SOURCE_TIMING_JMP_REGISTER), memory ?
                x86_core_cpu_control_stack_memory_additions(context, data, prefixes,
                    1u) : 0u, 1, out_ticks);
        }
        if (extension == 3u || extension == 5u) {
            if (task_switch) {
                if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386) {
                    (void)x86_core_cpu_control_stack_source_lookup(context,
                        extension == 3u ? X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_MEMORY :
                        X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_MEMORY_PROTECTED);
                    *out_ticks = 5u + x86_core_cpu_80386_task_transfer_ticks(context, data,
                        x86_core_cpu_control_stack_selector_is_task_gate(
                            context, data->crm) != 0);
                    return 1;
                }
                *out_ticks = extension == 3u ? 180u : 178u;
                *out_ticks += x86_core_cpu_control_stack_memory_additions(
                    context, data, prefixes, 2u);
                return x86_core_cpu_control_stack_add_next_term(context,
                    *out_ticks, out_ticks);
            }
            if (!memory || (!same_privilege && !(context->cpu_profile ==
                X86_CORE_CPU_PROFILE_80386 && protected_mode && extension == 3u &&
                data->crm != context->cpu->data.cs.selector)) ||
                (protected_mode && extension == 5u &&
                data->crm != context->cpu->data.cs.selector &&
                context->cpu_profile != X86_CORE_CPU_PROFILE_80386)) return 0;
            if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386 &&
                protected_mode && data->crm != context->cpu->data.cs.selector) {
                lib_u8 parameters = 0u;

                if (extension == 3u && !same_privilege &&
                    !x86_core_cpu_control_stack_call_gate_parameters(context,
                        data->crm, &parameters)) return 0;
                (void)x86_core_cpu_control_stack_source_lookup(context,
                    extension == 3u ? X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_MEMORY :
                    X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_MEMORY_PROTECTED);
                *out_ticks = extension == 3u ? (same_privilege ? 56u :
                    (parameters == 0u ? 90u : 98u + 4u * parameters)) : 49u;
                return x86_core_cpu_control_stack_add_next_term(context,
                    *out_ticks, out_ticks);
            }
            if (context->cpu_profile == X86_CORE_CPU_PROFILE_80286 &&
                protected_mode) {
                *out_ticks = extension == 3u ? 29u : 26u;
            } else if (context->cpu_profile == X86_CORE_CPU_PROFILE_80386 &&
                protected_mode) {
                (void)x86_core_cpu_control_stack_source_lookup(context,
                    extension == 3u ? X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_MEMORY :
                    X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_MEMORY_PROTECTED);
                *out_ticks = extension == 3u ? 38u : 31u;
            } else {
                *out_ticks = x86_core_cpu_control_stack_source_lookup(context,
                    extension == 3u ? X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_MEMORY :
                    protected_mode ? X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_MEMORY_PROTECTED :
                    X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_MEMORY);
            }
            if (context->cpu_profile == X86_CORE_CPU_PROFILE_8088) {
                return x86_core_cpu_control_stack_source_result(context,
                    extension == 3u ? X86_CORE_CPU_SOURCE_TIMING_CALL_FAR_MEMORY :
                    X86_CORE_CPU_SOURCE_TIMING_JMP_FAR_MEMORY,
                    x86_core_cpu_control_stack_memory_additions(context, data,
                        prefixes, 2u), 1, out_ticks);
            }
            *out_ticks += x86_core_cpu_control_stack_memory_additions(context,
                data, prefixes, 2u);
            return x86_core_cpu_control_stack_add_next_term(context, *out_ticks,
                out_ticks);
        }
        if (extension == 6u) {
            return x86_core_cpu_control_stack_source_result(context, memory ?
                X86_CORE_CPU_SOURCE_TIMING_PUSH_MEMORY :
                X86_CORE_CPU_SOURCE_TIMING_PUSH_REGISTER, memory ?
                x86_core_cpu_control_stack_memory_additions(context, data, prefixes,
                    1u) : 0u, 0, out_ticks);
        }
        return 0;
    default:
        return 0;
    }
}

static lib_u64 x86_core_cpu_source_timing_signed_magnitude(
    lib_u64 value, lib_u8 bytes)
{
    lib_u64 mask;
    lib_u64 sign;

    if (bytes == 0u || bytes > sizeof(value)) return 0u;
    mask = bytes == sizeof(value) ? LIB_UINT64_MAX :
        (UINT64_C(1) << (bytes * 8u)) - 1u;
    value &= mask;
    sign = UINT64_C(1) << (bytes * 8u - 1u);
    return (value & sign) == 0u ? value : ((~value + 1u) & mask);
}

static lib_u64 x86_core_cpu_source_timing_ceiling_log2(
    lib_u64 value)
{
    lib_u64 result = 0u;

    while (value > 1u) {
        value = (value + 1u) >> 1u;
        ++result;
    }
    return result;
}

/* Intel 80386 PRM, IMUL/MUL timing tables: use the underlined optimizing
 * multiplier only.  `crm` and `cimm` are decoder-owned values captured during
 * the real execution; this timing path never rereads a register or memory. */
lib_i32 x86_core_cpu_80386_dynamic_multiply_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks)
{
    const t_cpuins_data *data;
    lib_u32 prefixes;
    lib_u8 opcode;
    lib_u8 extension;
    lib_u8 operand_bytes;
    lib_u64 multiplier;
    lib_u64 magnitude;
    lib_u64 scale;
    x86_core_cpu_source_timing_form form;
    lib_i32 signed_multiplier;
    lib_i32 memory_multiplier;

    if (context == LIB_NULL || out_ticks == LIB_NULL ||
        context->cpu_profile != X86_CORE_CPU_PROFILE_80386) return 0;
    data = &context->instructions->data;
    prefixes = x86_core_cpu_instruction_prefix_count(context, data);
    if (prefixes + 1u >= data->oplen ||
        !x86_core_cpu_80386_timing_has_source_prefixes(data, prefixes)) {
        return 0;
    }
    opcode = data->opcodes[prefixes];
    extension = (data->opcodes[prefixes + 1u] >> 3u) & 7u;
    operand_bytes = data->oldcpu.data.cs.seg.exec.defsize ? 4u : 2u;
    if (data->prefix_oprsize) operand_bytes = operand_bytes == 4u ? 2u : 4u;
    signed_multiplier = LIB_FALSE;
    memory_multiplier = LIB_FALSE;

    if (opcode == 0xf6u || opcode == 0xf7u) {
        if (extension != 4u && extension != 5u) return 0;
        operand_bytes = opcode == 0xf6u ? 1u : operand_bytes;
        multiplier = data->crm;
        signed_multiplier = extension == 5u;
        memory_multiplier = x86_core_cpu_source_timing_modrm_is_memory(data,
            prefixes);
        form = signed_multiplier ? X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_IMUL :
            X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_MUL;
    } else if (opcode == 0x69u) {
        multiplier = data->crm;
        signed_multiplier = LIB_TRUE;
        memory_multiplier = x86_core_cpu_source_timing_modrm_is_memory(data,
            prefixes);
        form = X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_IMUL_IMMEDIATE;
    } else if (opcode == 0x6bu) {
        multiplier = data->cimm;
        operand_bytes = 1u;
        signed_multiplier = LIB_TRUE;
        form = X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_IMUL_IMMEDIATE;
    } else if (opcode == 0x0fu && prefixes + 2u < data->oplen &&
        data->opcodes[prefixes + 1u] == 0xafu) {
        multiplier = data->crm;
        signed_multiplier = LIB_TRUE;
        memory_multiplier = x86_core_cpu_source_timing_modrm_is_memory(data,
            prefixes + 1u);
        form = X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_IMUL_TWO_OPERAND;
    } else {
        return 0;
    }
    magnitude = signed_multiplier ? x86_core_cpu_source_timing_signed_magnitude(
        multiplier, operand_bytes) : multiplier &
        (operand_bytes == 4u ? LIB_UINT32_MAX :
            operand_bytes == 2u ? LIB_UINT16_MAX : LIB_UINT8_MAX);
    scale = x86_core_cpu_source_timing_ceiling_log2(magnitude);
    *out_ticks = magnitude == 0u ? 9u : (scale < 3u ? 3u : scale) + 6u;
    if (memory_multiplier) *out_ticks += 3u;
    context->timing_result.form_id = (lib_u32)form;
    return 1;
}

/* Intel 80386 PRM section 17.2.2.3 supplies the fixed secondary-form rows
 * below.  The dynamic IMUL early-out remains in the dedicated helper above;
 * both helpers consume the decoder's completed operand capture after the one
 * successful-retirement publisher, never a handler-local timing decision. */
static lib_u64 x86_core_cpu_80386_timing_zero_scan_count(
    lib_u64 value, lib_u8 operand_bytes, lib_i32 reverse)
{
    lib_u64 mask;
    lib_u64 bit;
    lib_u64 count = 0u;
    lib_u8 bits;

    bits = (lib_u8)(operand_bytes * 8u);
    mask = operand_bytes == 4u ? LIB_UINT32_MAX : LIB_UINT16_MAX;
    value &= mask;
    if (value == 0u) return bits;
    bit = reverse ? UINT64_C(1) << (bits - 1u) : 1u;
    while ((value & bit) == 0u) {
        ++count;
        bit = reverse ? bit >> 1u : bit << 1u;
    }
    return count;
}

lib_i32 x86_core_cpu_80386_secondary_source_instruction_cost(
    x86_core_cpu_execution_context *context, lib_u64 *out_ticks)
{
    const t_cpuins_data *data;
    lib_u32 prefixes;
    lib_u8 opcode;
    lib_u8 secondary;
    lib_u8 extension;
    lib_u8 operand_bytes;
    lib_i32 memory;

    if (context == LIB_NULL || out_ticks == LIB_NULL ||
        context->cpu_profile != X86_CORE_CPU_PROFILE_80386) return 0;
    data = &context->instructions->data;
    prefixes = x86_core_cpu_instruction_prefix_count(context, data);
    if (prefixes + 1u >= data->oplen ||
        !x86_core_cpu_80386_timing_has_source_prefixes(data, prefixes)) {
        return 0;
    }
    opcode = data->opcodes[prefixes];
    if (opcode != 0x0fu) return 0;
    secondary = data->opcodes[prefixes + 1u];
    operand_bytes = data->oldcpu.data.cs.seg.exec.defsize ? 4u : 2u;
    if (data->prefix_oprsize) operand_bytes = operand_bytes == 4u ? 2u : 4u;

    if (secondary >= 0x80u && secondary <= 0x8fu) {
        lib_i32 taken;

        context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_JCC;
        if (!x86_core_cpu_control_stack_condition_taken(data->oldcpu.data.eflags,
                secondary, &taken)) return 0;
        if (!taken) {
            *out_ticks = X86_CORE_CPU_80386_JCC_NOT_TAKEN_TICKS;
            return 1;
        }
        return x86_core_cpu_control_stack_add_next_term(context,
            X86_CORE_CPU_80386_JCC_TAKEN_TICKS, out_ticks);
    }
    if (secondary < 0xa3u || prefixes + 2u >= data->oplen) return 0;
    memory = x86_core_cpu_source_timing_modrm_is_memory(data, prefixes + 1u);
    extension = (data->opcodes[prefixes + 2u] >> 3u) & 7u;
    switch (secondary) {
    case 0xa3u:
        *out_ticks = memory ? 12u : 3u;
        context->timing_result.form_id =
            X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_BT;
        return 1;
    case 0xabu: case 0xb3u: case 0xbbu:
        *out_ticks = memory ? 13u : 6u;
        context->timing_result.form_id = (lib_u32)(secondary == 0xabu ?
            X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_BTS : secondary == 0xb3u ?
            X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_BTR :
            X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_BTC);
        return 1;
    case 0xbau:
        if (extension == 4u) {
            *out_ticks = memory ? 6u : 3u;
            context->timing_result.form_id =
                X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_BT;
            return 1;
        }
        if (extension >= 5u) {
            *out_ticks = memory ? 8u : 6u;
            context->timing_result.form_id = (lib_u32)(extension == 5u ?
                X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_BTS : extension == 6u ?
                X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_BTR :
                X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_BTC);
            return 1;
        }
        return 0;
    case 0xa4u: case 0xa5u: case 0xacu: case 0xadu:
        *out_ticks = memory ? 7u : 3u;
        context->timing_result.form_id = (lib_u32)(
            secondary == 0xa4u || secondary == 0xa5u ?
            X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_SHLD :
            X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_SHRD);
        return 1;
    case 0xb6u: case 0xb7u: case 0xbeu: case 0xbfu:
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            memory ? X86_CORE_CPU_SOURCE_TIMING_MOV_EXTEND_MEMORY :
            X86_CORE_CPU_SOURCE_TIMING_MOV_EXTEND_REGISTER);
        return 1;
    case 0xbcu: case 0xbdu:
        *out_ticks = (secondary == 0xbcu ? 11u : 9u) + 3u *
            x86_core_cpu_80386_timing_zero_scan_count(data->crm, operand_bytes,
                secondary == 0xbdu);
        context->timing_result.form_id = (lib_u32)(secondary == 0xbcu ?
            X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_BSF :
            X86_CORE_CPU_SOURCE_TIMING_DYNAMIC_BSR);
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
lib_i32 x86_core_cpu_80386_privileged_source_instruction_cost(
    x86_core_cpu_execution_context *context, lib_u64 *out_ticks)
{
    const t_cpuins_data *data;
    lib_u32 prefixes;
    lib_u8 opcode;
    lib_u8 secondary;
    lib_u8 modrm;
    lib_u8 extension;
    lib_i32 memory;
    lib_i32 protected_mode;

    lib_i32 operand32;

    if (context == LIB_NULL || out_ticks == LIB_NULL ||
        context->cpu_profile != X86_CORE_CPU_PROFILE_80386) return 0;
    data = &context->instructions->data;
    prefixes = x86_core_cpu_instruction_prefix_count(context, data);
    if (prefixes >= data->oplen || data->flagLock ||
        !x86_core_cpu_80386_timing_has_source_prefixes(data, prefixes)) {
        return 0;
    }
    opcode = data->opcodes[prefixes];
    protected_mode = x86_core_cpu_control_stack_is_protected(data);
    operand32 = data->oldcpu.data.cs.seg.exec.defsize;
    if (data->prefix_oprsize) operand32 = !operand32;

    if (opcode == 0x63u) {
        if (!protected_mode || (data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ||
            prefixes + 1u >= data->oplen) return 0;
        *out_ticks = x86_core_cpu_source_timing_modrm_is_memory(data, prefixes) ?
            21u : 20u;
        return 1;
    }
    if (opcode != 0x0fu) return 0;
    if (prefixes + 1u >= data->oplen) return 0;
    secondary = data->opcodes[prefixes + 1u];
    if (secondary == 0x06u) {
        if ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ||
            (protected_mode && data->oldcpu.data.cs.dpl != 0u)) return 0;
        *out_ticks = 6u;
        return 1;
    }
    if (secondary == 0xa0u || secondary == 0xa8u) {
        *out_ticks = 2u;
        return 1;
    }
    if (secondary == 0xa1u || secondary == 0xa9u) {
        *out_ticks = protected_mode ? 21u : 7u;
        return 1;
    }
    if (secondary == 0xb2u || secondary == 0xb4u || secondary == 0xb5u) {
        if (!protected_mode) {
            *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
                X86_CORE_CPU_SOURCE_TIMING_MOV_POINTER_REAL);
            return 1;
        }
        if ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u) return 0;
        if (secondary == 0xb2u) *out_ticks = operand32 ? 28u : 26u;
        else *out_ticks = operand32 ? 31u : 29u;
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
            *out_ticks = memory ? 24u : 20u;
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
            *out_ticks = memory ? 14u : 11u;
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
            *out_ticks = memory ? 22u : 21u;
        }
        return 1;
    case 0x20u:
        if ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ||
            (protected_mode && data->oldcpu.data.cs.dpl != 0u) ||
            (memory && !context->cpu_80386_cr_mov_ignores_mod) ||
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
            (memory && !context->cpu_80386_cr_mov_ignores_mod) ||
            (extension != 0u && extension != 2u && extension != 3u)) return 0;
        *out_ticks = extension == 0u ? 11u : (extension == 2u ? 4u : 5u);
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

static lib_i32 x86_core_cpu_80286_system_source_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks)
{
    const t_cpuins_data *data = &context->instructions->data;
    lib_u32 prefixes = x86_core_cpu_instruction_prefix_count(context, data);
    lib_u64 memory_ea = data->flagMem ?
        x86_core_cpu_80286_timing_effective_address(data, prefixes + 1u) : 0u;
    lib_u8 opcode;

    if (out_ticks == LIB_NULL) return 0;
    if (prefixes >= data->oplen) {
        context->source_repeat_active = LIB_FALSE;
        *out_ticks = 0u;
        return 1;
    }
    opcode = data->opcodes[prefixes];
    context->source_repeat_active = LIB_FALSE;
    if (opcode != 0x0fu) return 0;
    switch (opcode) {
    case 0x0fu:
        if (prefixes + 2u < data->oplen &&
            data->opcodes[prefixes + 1u] == 0x00u &&
            (((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 2u ||
             ((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 3u) &&
            x86_core_cpu_control_stack_is_protected(data) &&
            data->oldcpu.data.cs.dpl == 0u) {
            *out_ticks = (data->flagMem ? 19u : 17u) + memory_ea;
            return 1;
        }
        if (prefixes + 2u < data->oplen &&
            data->opcodes[prefixes + 1u] == 0x01u && data->flagMem &&
            (((data->opcodes[prefixes + 2u] >> 3u) & 7u) <= 3u) &&
            (!x86_core_cpu_control_stack_is_protected(data) ||
             data->oldcpu.data.cs.dpl == 0u)) {
            *out_ticks = ((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 0u ||
                ((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 2u ? 11u : 12u;
            *out_ticks += memory_ea;
            return 1;
        }
        if (prefixes + 1u < data->oplen && data->opcodes[prefixes + 1u] == 0x06u &&
            (!x86_core_cpu_control_stack_is_protected(data) ||
             data->oldcpu.data.cs.dpl == 0u)) {
            *out_ticks = 2u;
            return 1;
        }
        if (prefixes + 2u < data->oplen &&
            data->opcodes[prefixes + 1u] == 0x00u &&
            ((((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 4u) ||
             (((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 5u)) &&
            x86_core_cpu_control_stack_is_protected(data)) {
            *out_ticks = (data->flagMem ? 16u : 14u) + memory_ea;
            return 1;
        }
        if (prefixes + 2u < data->oplen &&
            ((data->opcodes[prefixes + 1u] == 0x01u &&
             (((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 4u ||
              (((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 6u &&
               (!x86_core_cpu_control_stack_is_protected(data) ||
                data->oldcpu.data.cs.dpl == 0u)))) ||
            (x86_core_cpu_control_stack_is_protected(data) &&
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
            *out_ticks += memory_ea;
            return 1;
        }
        x86_core_cpu_source_timing_mark_unallocated(context, out_ticks);
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
        x86_core_cpu_source_timing_mark_unallocated(context, out_ticks);
        return 1;
    case 0x63u:
        if (x86_core_cpu_control_stack_is_protected(data)) {
            *out_ticks = data->flagMem ? 11u : 10u;
            return 1;
        }
        x86_core_cpu_source_timing_mark_unallocated(context, out_ticks);
        return 1;
    case 0x8cu:
        if (prefixes + 1u < data->oplen &&
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) <= 3u) {
            *out_ticks = data->flagMem ? 3u +
                x86_core_cpu_80286_timing_effective_address(data, prefixes) +
                x86_core_cpu_80286_timing_odd_word(data) : 2u;
            return 1;
        }
        x86_core_cpu_source_timing_mark_unallocated(context, out_ticks);
        return 1;
    case 0x8eu:
        if (prefixes + 1u < data->oplen &&
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) != 1u &&
            ((data->opcodes[prefixes + 1u] >> 3u) & 7u) <= 3u) {
            *out_ticks = x86_core_cpu_control_stack_is_protected(data) ?
                (data->flagMem ? 19u : 17u) : (data->flagMem ? 5u : 2u);
            if (data->flagMem) *out_ticks +=
                x86_core_cpu_80286_timing_effective_address(data, prefixes) +
                x86_core_cpu_80286_timing_odd_word(data);
            return 1;
        }
        x86_core_cpu_source_timing_mark_unallocated(context, out_ticks);
        return 1;
    case 0xc4u: case 0xc5u:
        if (data->flagMem) {
            *out_ticks = 7u;
            return 1;
        }
        x86_core_cpu_source_timing_mark_unallocated(context, out_ticks);
        return 1;
    default:
        if (opcode >= 0xb0u && opcode <= 0xbfu) {
            *out_ticks = x86_core_cpu_80286_source_timing_lookup(context,
                X86_CORE_CPU_SOURCE_TIMING_MOV_IMMEDIATE);
        } else {
            x86_core_cpu_source_timing_mark_unallocated(context, out_ticks);
        }
        return 1;
    }
}

lib_i32 x86_core_cpu_80286_source_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks)
{
    const t_cpuins_data *data;

    if (context == LIB_NULL || out_ticks == LIB_NULL) return 0;
    data = &context->instructions->data;
    context->source_repeat_active = LIB_FALSE;
    if (x86_core_cpu_instruction_prefix_count(context, data) >= data->oplen) {
        *out_ticks = 0u;
        return 1;
    }
    x86_core_cpu_source_timing_mark_unallocated(context, out_ticks);
    return 1;
}

lib_i32 x86_core_cpu_80386_source_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks)
{
    const t_cpuins_data *data = &context->instructions->data;
    lib_u32 prefixes = x86_core_cpu_instruction_prefix_count(context, data);
    lib_u8 opcode;
    lib_u8 group2_extension;
    lib_i32 group2_memory;
    if (out_ticks == LIB_NULL) return 0;
    if (prefixes >= data->oplen) {
        context->source_repeat_active = LIB_FALSE;
        *out_ticks = 0u;
        return 1;
    }
    opcode = data->opcodes[prefixes];
    context->source_repeat_active = LIB_FALSE;
    if (data->flagLock ||
        !x86_core_cpu_80386_timing_has_source_prefixes(data, prefixes)) {
        x86_core_cpu_source_timing_mark_unallocated(context, out_ticks);
        return 1;
    }
    if ((opcode == 0xc4u || opcode == 0xc5u) &&
        x86_core_cpu_control_stack_is_protected(data) &&
        x86_core_cpu_source_timing_modrm_is_memory(data, prefixes)) {
        lib_i32 operand32 = data->oldcpu.data.cs.seg.exec.defsize;

        if (data->prefix_oprsize) operand32 = !operand32;
        *out_ticks = operand32 ? 28u : 26u;
        return 1;
    }
    if (opcode >= 0x70u && opcode <= 0x7fu) {
        lib_i32 taken;

        context->timing_result.form_id = X86_CORE_CPU_SOURCE_TIMING_JCC;
        if (!x86_core_cpu_control_stack_short_branch_taken(data, context,
                prefixes, &taken)) return 0;
        if (!taken) {
            *out_ticks = X86_CORE_CPU_80386_JCC_NOT_TAKEN_TICKS;
            return 1;
        }
        return x86_core_cpu_control_stack_add_next_term(context,
            X86_CORE_CPU_80386_JCC_TAKEN_TICKS, out_ticks);
    }
    switch (opcode) {
    case 0x90u:
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_NOP);
        return 1;
    case 0xd0u: case 0xd1u: case 0xd2u: case 0xd3u:
    case 0xc0u: case 0xc1u:
        if (prefixes + 1u >= data->oplen) {
            x86_core_cpu_source_timing_mark_unallocated(context, out_ticks);
            return 1;
        }
        group2_extension = (data->opcodes[prefixes + 1u] >> 3u) & 7u;
        group2_memory = x86_core_cpu_source_timing_modrm_is_memory(data,
            prefixes);
        if (group2_extension == 0u || group2_extension == 1u ||
            group2_extension == 4u || group2_extension == 5u ||
            group2_extension == 7u) {
            *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
                group2_memory ? X86_CORE_CPU_SOURCE_TIMING_GROUP2_ROTATE_MEMORY :
                X86_CORE_CPU_SOURCE_TIMING_GROUP2_ROTATE_REGISTER);
        } else if (group2_extension == 2u || group2_extension == 3u) {
            *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
                group2_memory ? X86_CORE_CPU_SOURCE_TIMING_GROUP2_CARRY_MEMORY :
                X86_CORE_CPU_SOURCE_TIMING_GROUP2_CARRY_REGISTER);
        } else {
            x86_core_cpu_source_timing_mark_unallocated(context, out_ticks);
        }
        return 1;
    case 0xf8u:
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_CLC);
        return 1;
    case 0xf5u:
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_CMC);
        return 1;
    case 0xf9u:
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_STC);
        return 1;
    case 0xfcu:
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_CLD);
        return 1;
    case 0xfdu:
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_STD);
        return 1;
    case 0xfau:
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_CLI);
        return 1;
    case 0xfbu:
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_STI);
        return 1;
    case 0x9eu:
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_SAHF);
        return 1;
    case 0x9fu:
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_LAHF);
        return 1;
    case 0x8cu:
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            x86_core_cpu_source_timing_modrm_is_memory(data, prefixes) ?
            X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_STORE_MEMORY :
            X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_REGISTER);
        return 1;
    case 0x8eu:
        if (x86_core_cpu_control_stack_is_protected(data)) {
            *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
                x86_core_cpu_source_timing_modrm_is_memory(data, prefixes) ?
                X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_LOAD_MEMORY_PROTECTED :
                X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_REGISTER_PROTECTED);
        } else if (!x86_core_cpu_source_timing_modrm_is_memory(data, prefixes)) {
            *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
                X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_REGISTER);
        } else {
            *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
                X86_CORE_CPU_SOURCE_TIMING_MOV_SREG_LOAD_MEMORY);
        }
        return 1;
    case 0x88u: case 0x89u:
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            x86_core_cpu_source_timing_modrm_is_memory(data, prefixes) ?
            X86_CORE_CPU_SOURCE_TIMING_MOV_RM_REGISTER :
            X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_REGISTER);
        return 1;
    case 0x8au: case 0x8bu:
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            x86_core_cpu_source_timing_modrm_is_memory(data, prefixes) ?
            X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_RM :
            X86_CORE_CPU_SOURCE_TIMING_MOV_REGISTER_REGISTER);
        return 1;
    case 0xa0u: case 0xa1u:
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_READ);
        return 1;
    case 0xa2u: case 0xa3u:
        *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
            X86_CORE_CPU_SOURCE_TIMING_MOV_MOFFS_WRITE);
        return 1;
    default:
        if (opcode >= 0xb0u && opcode <= 0xbfu) {
            *out_ticks = x86_core_cpu_80386_source_timing_lookup(context,
                X86_CORE_CPU_SOURCE_TIMING_MOV_IMMEDIATE);
        } else {
            x86_core_cpu_source_timing_mark_unallocated(context, out_ticks);
        }
        return 1;
    }
    x86_core_cpu_source_timing_mark_unallocated(context, out_ticks);
    return 1;
}

lib_u64 x86_core_cpu_timing_maximum_ticks(
    x86_core_cpu_profile profile,
    const x86_core_cpu_instruction_timing *timing)
{
    switch (profile) {
    case X86_CORE_CPU_PROFILE_8086:
        return X86_CORE_CPU_8086_SOURCE_MAXIMUM_TICKS;
    case X86_CORE_CPU_PROFILE_80186:
        return X86_CORE_CPU_80186_SOURCE_MAXIMUM_TICKS;
    case X86_CORE_CPU_PROFILE_80286:
        return X86_CORE_CPU_80286_SOURCE_MAXIMUM_TICKS;
    case X86_CORE_CPU_PROFILE_80386:
        return X86_CORE_CPU_80386_SOURCE_MAXIMUM_TICKS;
    case X86_CORE_CPU_PROFILE_80486SX:
    case X86_CORE_CPU_PROFILE_V30:
        return 0u;
    default:
        break;
    }
    if (timing == LIB_NULL) return 0u;
    return (lib_u64)timing->base_ticks +
        (lib_u64)timing->prefix_surcharge * 15u +
        timing->taken_branch_surcharge + timing->data_memory_surcharge +
        timing->io_surcharge + timing->rep_iteration_surcharge;
}

lib_i32 x86_core_cpu_compatibility_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks)
{
    const t_cpuins_data *data = &context->instructions->data;
    const x86_core_cpu_instruction_timing *timing =
        &context->instruction_timing;
    lib_u32 prefixes = x86_core_cpu_instruction_prefix_count(context, data);
    lib_u8 opcode;
    lib_u64 ticks = timing->base_ticks;

    if (prefixes >= sizeof(data->opcodes)) return 0;
    /* This retained compatibility recipe has no source-form allocation.  It
     * may drive deterministic execution, but can never qualify physical
     * retirement time for any CPU profile. */
    x86_core_cpu_source_timing_mark_unallocated(context, LIB_NULL);
    opcode = data->opcodes[prefixes];
    if (data->prefix_rep != PREFIX_REP_NONE && opcode == 0xa4u) {
        if (!x86_core_cpu_timing_add_ticks(&ticks, timing->rep_iteration_surcharge)) {
            return 0;
        }
    } else if (!x86_core_cpu_timing_add_ticks(&ticks,
            (lib_u64)prefixes * timing->prefix_surcharge)) {
        return 0;
    }
    if (opcode >= 0x70u && opcode <= 0x7fu) {
        lib_i32 taken;

        if (!x86_core_cpu_control_stack_short_branch_taken(data, context,
                prefixes, &taken)) return 0;
        if (taken &&
            !x86_core_cpu_timing_add_ticks(&ticks, timing->taken_branch_surcharge)) {
            return 0;
        }
    }
    if (opcode >= 0xa0u && opcode <= 0xa3u &&
        !x86_core_cpu_timing_add_ticks(&ticks, timing->data_memory_surcharge)) {
        return 0;
    }
    if (((opcode >= 0xe4u && opcode <= 0xe7u) ||
         (opcode >= 0xecu && opcode <= 0xefu)) &&
        !x86_core_cpu_timing_add_ticks(&ticks, timing->io_surcharge)) {
        return 0;
    }
    *out_ticks = ticks;
    return 1;
}

lib_i32 x86_core_cpu_80486_source_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks)
{
    x86_core_cpu_source_timing_form form;
    const t_cpuins_data *data;
    lib_u32 prefixes;
    lib_u64 ticks, memory, decode;
    lib_bool original, memory_model, decode_model, bus_model;

    if (context == LIB_NULL || out_ticks == LIB_NULL || context->instructions == LIB_NULL ||
        context->cpu == LIB_NULL ||
        context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX) return 0;
    data = &context->instructions->data;
    prefixes = x86_core_cpu_instruction_prefix_count(context, data);
    if (prefixes >= data->oplen) return 0;
    if (x86_core_cpu_source_timing_string_form(data->opcodes[prefixes], &form)) {
        x86_core_cpu_source_timing_record_repeat(context, data);
    } else context->source_repeat_active = LIB_FALSE;
    original = x86_core_cpu_80486_nominal_instruction_cost(context, &ticks);
    if ((!original && !x86_core_cpu_80486_l2_instruction_cost(context, &ticks)) ||
        !x86_core_cpu_80486_memory_penalty(context, &memory, &memory_model) ||
        !x86_core_cpu_80486_decode_penalty(context, &decode, &decode_model) ||
        !x86_core_cpu_timing_add_ticks(&ticks, memory) ||
        !x86_core_cpu_timing_add_ticks(&ticks, decode) ||
        !x86_core_cpu_80486_bus_instruction_cost(context, ticks, &ticks, &bus_model)) return 0;
    /* Source identity is the decoded opcode family. Existing eligibility
     * fields retain ModR/M, mode, size, branch and REP variants separately. */
    context->timing_result.form_id = data->opcodes[prefixes] == 0x0fu ?
        0x100u + data->opcodes[prefixes + 1u] : data->opcodes[prefixes];
    context->timing_result.retirement_origin = !original || memory_model || decode_model || bus_model ?
        X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_80486_MODEL :
        X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80486_SOURCE;
    context->timing_result.formula_inputs =
        (data->timing_cache_used ? X86_CORE_CPU_TIMING_INPUT_CACHE : 0u) |
        (data->timing_fetch_bus_ticks != 0u ? X86_CORE_CPU_TIMING_INPUT_FETCH : 0u) |
        (memory != 0u ? X86_CORE_CPU_TIMING_INPUT_MEMORY : 0u) |
        (data->timing_tlb_walks[0] != 0u || data->timing_tlb_walks[1] != 0u ||
            data->timing_tlb_walks[2] != 0u || data->timing_tlb_updates != 0u ?
            X86_CORE_CPU_TIMING_INPUT_TRANSLATION : 0u) |
        (decode != 0u ? X86_CORE_CPU_TIMING_INPUT_DECODE : 0u) |
        (data->timing_bus_service_ticks != 0u || data->timing_bus_wait_ticks != 0u ||
            data->timing_bus_stall_ticks != 0u || data->timing_bus_invalidation_ticks != 0u ?
            X86_CORE_CPU_TIMING_INPUT_BUS : 0u);
    *out_ticks = ticks;
    return 1;
}

/* E-13: TS depends on the incoming TSS/mode, not the outgoing task. */
static lib_u64 x86_core_cpu_80486_nominal_task_cost(
    const x86_core_cpu_execution_context *context)
{
    const t_cpu_data *incoming = &context->instruction_task_checkpoint.data;

    if ((incoming->eflags & VCPU_EFLAGS_VM) != 0u) return 140u;
    return incoming->tr.sys.type == VCPU_DESC_SYS_TYPE_TSS_16_BUSY ? 143u : 162u;
}

/* E-8/E-9. The executing route captures the actual gate class and copied
 * parameter count; timing never rereads a potentially changed descriptor. */
static lib_u64 x86_core_cpu_80486_nominal_far_cost(
    const x86_core_cpu_execution_context *context,
    const t_cpuins_data *data, lib_bool call, lib_bool indirect)
{
    const lib_u8 gate = data->timing_transfer_type;

    if ((data->oldcpu.data.cr0 & VCPU_CR0_PE) == 0u ||
        (data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u)
        return call ? (indirect ? 17u : 18u) : (indirect ? 13u : 17u);
    if (context->instruction_task_switched)
        return x86_core_cpu_80486_nominal_task_cost(context) +
            (call ? 37u : indirect ? 41u : 42u) +
            (gate == VCPU_DESC_SYS_TYPE_TASKGATE ? 1u : 0u);
    if (gate == VCPU_DESC_SYS_TYPE_CALLGATE_16 || gate == VCPU_DESC_SYS_TYPE_CALLGATE_32) {
        if (!call) return indirect ? 31u : 32u;
        if (data->oldcpu.data.cs.dpl == context->cpu->data.cs.dpl) return 35u;
        return data->timing_transfer_parameters == 0u ? 69u :
            77u + 4u * data->timing_transfer_parameters;
    }
    return call ? 20u : indirect ? 18u : 19u;
}

/* E-13 setup precedes the separate E-14 INT subtotal. Read the original
 * faulting opcode before entry; the handler must not become the timing key. */
lib_u64 x86_core_cpu_80486_exception_setup_cost(
    const x86_core_cpu_execution_context *context, lib_u8 vector)
{
    const t_cpuins_data *data = &context->instructions->data;
    const lib_u32 prefixes = x86_core_cpu_instruction_prefix_count(context, data);
    lib_u8 opcode;

    if (vector == 14u || vector == 5u) return 24u;
    if (vector != 13u || (context->cpu->data.eflags & VCPU_EFLAGS_VM) == 0u ||
        prefixes >= data->oplen) return 0u;
    opcode = data->opcodes[prefixes];
    switch (opcode) {
    case 0xfau: case 0xfbu: case 0x9du:
        return 8u;
    case 0xcdu: case 0x9cu: case 0xcfu:
        return 9u;
    case 0xe4u: case 0xe5u: case 0xe6u: case 0xe7u:
        return 50u;
    case 0xecu: case 0xedu: case 0xeeu: case 0xefu:
        return 51u;
    case 0x6cu: case 0x6du: case 0x6eu: case 0x6fu:
        return data->prefix_rep == PREFIX_REP_NONE ? 50u : 51u;
    default:
        return 0u;
    }
}

/* E-14's INT value; instruction-specific setup is a separate E-13 term. */
static lib_u64 x86_core_cpu_80486_nominal_interrupt_entry_cost(
    const x86_core_cpu_execution_context *context,
    lib_u32 entry_cr0, lib_u32 entry_flags, lib_u8 entry_cpl, lib_bool task_entry)
{
    if (task_entry)
        return 37u + x86_core_cpu_80486_nominal_task_cost(context);
    if ((entry_cr0 & VCPU_CR0_PE) == 0u) return 26u;
    if ((entry_flags & VCPU_EFLAGS_VM) != 0u)
        return (context->cpu->data.eflags & VCPU_EFLAGS_VM) != 0u ? 26u : 82u;
    return entry_cpl == context->cpu->data.cs.dpl ? 44u : 71u;
}

/* Intel 273021-001 Table 8-1 SX column. Its save/restore figures are
 * approximate and assume a silicon image footprint. The full admitted image
 * and serialized service envelope therefore retain an explicit L2 grade. */
lib_bool x86_core_cpu_80486_smm_cost(
    const x86_core_cpu_execution_context *context, lib_bool entry,
    lib_bool complete, lib_u64 prefix_service, lib_u64 *out_ticks)
{
    const t_cpuins_data *data;
    lib_u64 service, ticks;

    if (context == LIB_NULL || context->instructions == LIB_NULL || out_ticks == LIB_NULL ||
        context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX || context->bus_timing.invalid)
        return LIB_FALSE;
    data = &context->instructions->data;
    if (data->timing_external_wait_overflow ||
        prefix_service > data->timing_bus_service_ticks ||
        data->timing_external_wait_ticks > data->timing_bus_service_ticks - prefix_service)
        return LIB_FALSE;
    service = data->timing_bus_service_ticks - prefix_service - data->timing_external_wait_ticks;
    if (complete && service < (entry ? 139u : 236u)) service = entry ? 139u : 236u;
    ticks = entry ? 42u : complete ? 24u : 0u;
    if (entry) {
        if (prefix_service > LIB_UINT64_MAX - ticks) return LIB_FALSE;
        ticks += prefix_service;
    }
    if (service > LIB_UINT64_MAX - ticks) return LIB_FALSE;
    *out_ticks = ticks + service;
    return LIB_TRUE;
}

/* E-15: real, protected without/with I/O permission checks, then VM86.
 * Columns are IN imm/DX, OUT imm/DX, string primitive, REP IN/OUT setup. */
static lib_u64 x86_core_cpu_80486_nominal_io_cost(
    const x86_core_cpu_execution_context *context,
    const t_cpuins_data *data, lib_u8 opcode)
{
    static const lib_u8 clocks[4][7] = {
        {14u, 14u, 16u, 16u, 17u, 16u, 17u},
        {9u, 8u, 11u, 10u, 10u, 10u, 11u},
        {29u, 28u, 31u, 30u, 32u, 30u, 31u},
        {27u, 27u, 29u, 29u, 30u, 29u, 30u}
    };
    const lib_u8 mode = (data->oldcpu.data.cr0 & VCPU_CR0_PE) == 0u ? 0u :
        (data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ? 3u :
        data->oldcpu.data.cs.dpl <=
            ((data->oldcpu.data.eflags & VCPU_EFLAGS_IOPL) >> 12u) ? 1u : 2u;

    if (opcode >= 0xe4u)
        return clocks[mode][((opcode & 2u) != 0u ? 2u : 0u) +
            (opcode >= 0xecu ? 1u : 0u)];
    if (data->prefix_rep == PREFIX_REP_NONE) return clocks[mode][4];
    const lib_bool address32 = data->oldcpu.data.cs.seg.exec.defsize != data->prefix_addrsize;
    const lib_u32 count = address32 ? data->oldcpu.data.ecx : data->oldcpu.data.cx;
    const lib_bool output = opcode >= 0x6eu;
    const lib_u64 iteration = output ? 5u : 8u;

    if (count == 0u) return clocks[mode][output ? 6u : 5u];
    return iteration + (context->timing_result.repeat_phase == X86_CORE_CPU_RETIREMENT_REPEAT_CONTINUATION ?
        0u : clocks[mode][output ? 6u : 5u]);
}

static lib_bool x86_core_cpu_80486_push_pop_opcode(lib_u16 opcode)
{
    const lib_u8 primary = (lib_u8)opcode;
    const lib_u8 secondary = (lib_u8)(opcode >> 8u);

    if (primary >= 0x50u && primary <= 0x61u) return LIB_TRUE;
    switch (primary) {
    case 0x06u: case 0x07u: case 0x0eu: case 0x16u: case 0x17u:
    case 0x1eu: case 0x1fu: case 0x68u: case 0x6au: case 0x9cu: case 0x9du:
        return LIB_TRUE;
    case 0x8fu: return ((secondary >> 3u) & 7u) == 0u;
    case 0xffu: return ((secondary >> 3u) & 7u) == 6u;
    case 0x0fu:
        return secondary == 0xa0u || secondary == 0xa1u || secondary == 0xa8u || secondary == 0xa9u;
    default: return LIB_FALSE;
    }
}

/* Prefix costs are per decoded macroinstruction. The selected REP owner must
 * not charge these again for a continuation of that same instruction. */
lib_bool x86_core_cpu_80486_decode_penalty(
    const x86_core_cpu_execution_context *context, lib_u64 *out_ticks, lib_bool *out_modeled)
{
    const t_cpuins_data *data;
    lib_u32 prefixes;
    lib_u8 opcode, repeats = 0u;
    lib_u64 ticks = 0u;
    lib_bool modeled = LIB_FALSE;
    x86_core_cpu_source_timing_form string_form;

    if (context == LIB_NULL || out_ticks == LIB_NULL ||
        context->instructions == LIB_NULL || context->cpu == LIB_NULL ||
        context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX ||
        context->instruction_fault_delivered || context->stop_requested)
        return LIB_FALSE;
    data = &context->instructions->data;
    if (data->except != 0u) return LIB_FALSE;
    prefixes = x86_core_cpu_instruction_prefix_count(context, data);
    if (prefixes >= data->oplen) return LIB_FALSE;
    opcode = data->opcodes[prefixes];
    for (lib_u32 index = 0u; index < prefixes; ++index) {
        if (data->opcodes[index] == 0xf2u || data->opcodes[index] == 0xf3u) ++repeats;
        else ++ticks;
    }
    /* One effective string REP belongs to setup. Extra/ignored REP bytes
     * use the explicit L2 one-clock decode model, not a new nominal row. */
    if (repeats != 0u) {
        const lib_bool string = x86_core_cpu_source_timing_string_form(opcode, &string_form);

        modeled = repeats > 1u || !string ||
            !x86_core_cpu_source_timing_string_repeat_is_defined(string_form, data->prefix_rep);
        ticks += repeats - (lib_u64)string;
    }
    if (context->timing_result.repeat_phase == X86_CORE_CPU_RETIREMENT_REPEAT_CONTINUATION) {
        *out_ticks = 0u;
        if (out_modeled != LIB_NULL) *out_modeled = modeled;
        return LIB_TRUE;
    }
    if (data->timing_ea_indexed && opcode != 0x8du) ++ticks;
    if (data->timing_ea_displacement && data->timing_immediate) ++ticks;
    {
        const lib_u16 current_opcode = opcode | (prefixes + 1u < data->oplen ?
            (lib_u32)data->opcodes[prefixes + 1u] << 8u : 0u);
        lib_u8 writes = data->timing_previous_gpr_writes;
        const lib_u8 bases = data->timing_ea_base_mask | (data->timing_stack_access ? 0x10u : 0u);

        /* E.2 exempts back-to-back PUSH/POP stack adjustment, not a POP's
         * non-SP destination subsequently used as an ordinary address base. */
        if (x86_core_cpu_80486_push_pop_opcode(current_opcode) &&
            x86_core_cpu_80486_push_pop_opcode(data->timing_previous_opcode)) writes &= (lib_u8)~0x10u;
        if ((writes & bases) != 0u) ++ticks;
    }
    *out_ticks = ticks;
    if (out_modeled != LIB_NULL) *out_modeled = modeled;
    return LIB_TRUE;
}

/* E-10/E-14 notes 17--20 and E-15 note 3. The previous missing-line fact
 * belongs to the validated macroinstruction, not a second cache/debt queue. */
static lib_bool x86_core_cpu_80486_repeat_cache_penalty(
    const x86_core_cpu_execution_context *context,
    x86_core_cpu_source_timing_form form, lib_u64 *out_ticks, lib_bool *out_modeled)
{
    const t_cpuins_data *data = &context->instructions->data;
    const lib_u64 permission = data->timing_tss_field_misses;
    const lib_u64 fields = data->timing_missed_tss_fields;
    const lib_u64 previous = data->timing_repeat_previous_misses;
    const lib_u64 misses = data->timing_data_misses - permission;
    lib_u64 first = 0u, deferred = 0u;

    if (data->timing_locked_reads != 0u || data->timing_descriptor_misses != 0u ||
        fields > data->timing_missed_reads || fields > permission) return LIB_FALSE;
    const lib_u64 reads = data->timing_missed_reads - fields;
    if (misses < reads || misses > 2u * reads || reads > 2u) return LIB_FALSE;
    switch (form) {
    case X86_CORE_CPU_SOURCE_TIMING_STRING_MOVS: first = 1u; deferred = 3u; break;
    case X86_CORE_CPU_SOURCE_TIMING_STRING_SCAS: first = 2u; deferred = 2u; break;
    case X86_CORE_CPU_SOURCE_TIMING_STRING_OUTS: deferred = 2u; break;
    case X86_CORE_CPU_SOURCE_TIMING_STRING_LODS: first = 2u; break;
    case X86_CORE_CPU_SOURCE_TIMING_STRING_CMPS: first = 3u; break;
    case X86_CORE_CPU_SOURCE_TIMING_STRING_STOS:
    case X86_CORE_CPU_SOURCE_TIMING_STRING_INS:
        if (misses != 0u) return LIB_FALSE;
        break;
    default: return LIB_FALSE;
    }
    *out_ticks = 2u * fields + 5u * (permission - fields) +
        first * reads + 5u * (misses - reads) + deferred * previous;
    /* The cross-primitive handoff and nonreference residency are models;
     * the weights retain the published first/second-operation schedule. */
    *out_modeled = permission != 0u || (deferred != 0u && previous != 0u) ||
        misses > reads || (form == X86_CORE_CPU_SOURCE_TIMING_STRING_CMPS && reads == 1u) ||
        (misses != 0u && (data->oldcpu.data.cr0 & VCPU_CR0_CD) != 0u);
    return LIB_TRUE;
}

/* E-3--E-9 allocate actual access groups. Split and partial-residency models
 * keep their qualification beside the corresponding source coefficient. */
static lib_bool x86_core_cpu_80486_interrupt_cache_penalty(
    const t_cpuins_data *data, lib_bool real, lib_bool inner,
    lib_u64 *out_ticks, lib_bool *out_modeled)
{
    const lib_u64 misses = (lib_u64)data->timing_data_misses - data->timing_descriptor_misses;
    const lib_u64 tss = data->timing_tss_field_misses;
    const lib_u64 gate = misses - tss;
    const lib_bool cold = gate == 1u && tss == (lib_u64)inner &&
        data->timing_descriptor_misses == (real ? 0u : inner ? 2u : 1u);

    if (data->timing_locked_reads != 0u) return LIB_FALSE;
    /* E-14: IVT 2; protected IDT/CS 3+3; inner IDT/CS/SS/TSS
     * 3+3+3+8. Descriptor terms belong to the common access owner.
     * Partial residency and additional misses are explicit L2 splits. */
    *out_ticks = (gate ? (real ? 2u : 3u) + 5u * (gate - 1u) : 0u) +
        (tss ? 8u + 5u * (tss - 1u) : 0u);
    *out_modeled = (data->timing_data_misses != 0u && !cold) ||
        (data->timing_data_misses != 0u && (data->oldcpu.data.cr0 & VCPU_CR0_CD) != 0u);
    return LIB_TRUE;
}

static lib_bool x86_core_cpu_80486_task_cache_penalty(
    const x86_core_cpu_execution_context *context, lib_u64 *out_ticks, lib_bool *out_modeled)
{
    const t_cpuins_data *data = &context->instructions->data;
    const lib_bool task32 = context->instruction_task_checkpoint.data.tr.sys.type !=
        VCPU_DESC_SYS_TYPE_TSS_16_BUSY;
    const lib_u8 lines = task32 ? 6u : 3u;
    const lib_u8 cold = task32 ? 34u : 16u;
    const lib_u64 pointer = data->timing_pair_misses;
    const lib_u64 backlink = data->timing_tss_field_misses;
    const lib_u64 idt = data->timing_idt_misses;
    const lib_u64 state = (lib_u64)data->timing_data_misses - data->timing_descriptor_misses -
        pointer - backlink - idt;

    if (data->timing_locked_reads != 0u) return LIB_FALSE;
    /* E-8/E-9 and E-14 share the TS memory model. IDT is not TSS image
     * traffic; descriptor/Gate coalescing remains in the common owner.
     * The inferred role split, including partial state, is explicitly L2. */
    *out_ticks = (state >= lines ? cold + 5u * (state - lines) :
        ((lib_u64)cold * state + lines - 1u) / lines) +
        (pointer ? 7u + 5u * (pointer - 1u) : 0u) +
        (backlink ? 1u + 5u * (backlink - 1u) : 0u) +
        (idt ? 3u + 5u * (idt - 1u) : 0u);
    *out_modeled = data->timing_data_misses != 0u;
    return LIB_TRUE;
}

static lib_bool x86_core_cpu_80486_operand_cache_penalty(
    const x86_core_cpu_execution_context *context, lib_u32 prefixes,
    lib_u64 *out_ticks, lib_bool *out_modeled)
{
    const t_cpuins_data *data = &context->instructions->data;
    const lib_u8 opcode = data->opcodes[prefixes];
    const lib_u8 extension = prefixes + 1u < data->oplen ?
        (data->opcodes[prefixes + 1u] >> 3u) & 7u : 0u;
    lib_bool read = LIB_FALSE, lockable = LIB_FALSE, implicit_lock = LIB_FALSE;
    lib_u8 miss = 0u, locked = 2u;
    const lib_bool operand32 = data->oldcpu.data.cs.seg.exec.defsize != data->prefix_oprsize;
    const lib_u32 misses = data->timing_data_misses - data->timing_descriptor_misses;
    const lib_u32 reads = data->timing_missed_reads - data->timing_missed_descriptors;
    const lib_bool inner_gate = (data->timing_transfer_type == VCPU_DESC_SYS_TYPE_CALLGATE_16 ||
        data->timing_transfer_type == VCPU_DESC_SYS_TYPE_CALLGATE_32) &&
        data->oldcpu.data.cs.dpl > context->cpu->data.cs.dpl;
    x86_core_cpu_source_timing_form string_form;

    *out_modeled = opcode == 0x0fu && prefixes + 1u < data->oplen &&
        data->opcodes[prefixes + 1u] >= 0x10u && data->opcodes[prefixes + 1u] <= 0x13u;
    if (opcode == 0x0fu && prefixes + 1u < data->oplen &&
        data->opcodes[prefixes + 1u] == 0xaau) {
        /* RSM's physical image traffic already belongs to its SMM envelope. */
        *out_ticks = 0u;
        *out_modeled = LIB_TRUE;
        return LIB_TRUE;
    }
    if (data->prefix_rep != PREFIX_REP_NONE &&
        x86_core_cpu_source_timing_string_form(opcode, &string_form))
        return x86_core_cpu_80486_repeat_cache_penalty(context, string_form, out_ticks, out_modeled);
    if (context->instruction_task_switched)
        return x86_core_cpu_80486_task_cache_penalty(context, out_ticks, out_modeled);
    if (inner_gate && (opcode == 0x9au || (opcode == 0xffu && extension == 3u))) {
        const lib_u64 pointer = data->timing_pair_misses;
        const lib_u64 tss = data->timing_tss_field_misses;
        const lib_u64 parameters = misses - pointer - tss;
        const lib_u8 width = data->timing_transfer_type == VCPU_DESC_SYS_TYPE_CALLGATE_32 ? 4u : 2u;
        const lib_u8 lines = (width * data->timing_transfer_parameters + 15u) / 16u;

        if (data->timing_locked_reads != 0u) return LIB_FALSE;
        *out_ticks = (pointer ? 7u + 5u * (pointer - 1u) : 0u) +
            (tss ? 8u + 5u * (tss - 1u) : 0u) + 4u * parameters;
        *out_modeled = pointer > 1u || tss > 1u ||
            (parameters != 0u && ((data->oldcpu.data.esp & 15u) != 0u || parameters != lines));
        return LIB_TRUE;
    }
    if (opcode == 0xccu || opcode == 0xcdu || opcode == 0xceu) {
        const lib_bool real = (data->oldcpu.data.cr0 & VCPU_CR0_PE) == 0u ||
            (context->cpu->data.eflags & VCPU_EFLAGS_VM) != 0u;
        const lib_bool inner = !real &&
            ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ||
             data->oldcpu.data.cs.dpl > context->cpu->data.cs.dpl);
        return x86_core_cpu_80486_interrupt_cache_penalty(data, real, inner, out_ticks, out_modeled);
    }
    if (misses == 0u && data->timing_locked_reads == 0u) {
        *out_ticks = 0u;
        return LIB_TRUE;
    }
    if (opcode <= 0x3du && (opcode & 7u) <= 5u) {
        const lib_bool compare = (opcode & 0x38u) == 0x38u;

        read = data->flagMem && (opcode & 7u) < 4u;
        lockable = !compare && (opcode & 2u) == 0u;
        miss = compare || (opcode & 2u) != 0u ? 2u : 6u;
    } else if (opcode >= 0x58u && opcode <= 0x5fu) {
        read = LIB_TRUE;
        miss = 2u;
    } else switch (opcode) {
    case 0x61u: {
        const lib_u8 lines = operand32 ? 2u : 1u;
        const lib_u8 cold = operand32 ? 15u : 7u;

        if (data->timing_locked_reads != 0u) return LIB_FALSE;
        *out_ticks = misses >= lines ? cold + 5u * (lib_u64)(misses - lines) :
            ((lib_u64)cold * misses + lines - 1u) / lines;
        *out_modeled = (data->oldcpu.data.esp & 15u) != 0u ||
            misses != lines;
        return LIB_TRUE;
    }
    case 0xc8u: {
        const lib_u8 level = prefixes + 3u < data->oplen ?
            data->opcodes[prefixes + 3u] & 31u : 0u;
        const lib_u8 lines = ((level == 0u ? 0u : level - 1u) * (operand32 ? 4u : 2u) + 15u) / 16u;

        if (data->timing_locked_reads != 0u || lines == 0u) return LIB_FALSE;
        *out_ticks = 6u * (lib_u64)misses;
        *out_modeled = (data->oldcpu.data.ebp & 15u) != 0u ||
            misses != lines;
        return LIB_TRUE;
    }
    case 0x62u: case 0xc4u: case 0xc5u:
        if (data->timing_locked_reads != 0u) return LIB_FALSE;
        *out_ticks = 7u + 5u * (lib_u64)(misses - 1u);
        *out_modeled = misses != 1u;
        return LIB_TRUE;
    case 0x07u: case 0x17u: case 0x1fu: case 0x8eu:
        read = LIB_TRUE;
        miss = 2u;
        break;
    case 0x63u:
        read = data->flagMem;
        break;
    case 0x69u: case 0x6bu:
        read = data->flagMem;
        miss = 2u;
        break;
    case 0x6cu: case 0x6du: case 0x6eu: case 0x6fu:
    case 0xe4u: case 0xe5u: case 0xe6u: case 0xe7u:
    case 0xecu: case 0xedu: case 0xeeu: case 0xefu:
        if (data->timing_locked_reads != 0u ||
            reads == 0u || misses < reads) return LIB_FALSE;
        if ((data->oldcpu.data.cr0 & VCPU_CR0_PE) != 0u &&
            ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ||
             data->oldcpu.data.cs.dpl > ((data->oldcpu.data.eflags & VCPU_EFLAGS_IOPL) >> 12u))) {
            /* E-15 has no separate permission-cache column. Keep this
             * completed-transfer differential explicitly modeled. */
            *out_ticks = 2u * (lib_u64)reads + 5u * (lib_u64)(misses - reads);
            *out_modeled = LIB_TRUE;
            return LIB_TRUE;
        }
        if (opcode != 0x6eu && opcode != 0x6fu) return LIB_FALSE;
        read = LIB_TRUE;
        miss = 2u;
        break;
    case 0x80u: case 0x81u: case 0x82u: case 0x83u:
        read = data->flagMem;
        lockable = extension != 7u;
        miss = extension == 7u ? 2u : 6u;
        break;
    case 0x84u: case 0x85u:
        read = data->flagMem;
        miss = 2u;
        break;
    case 0x86u: case 0x87u:
        read = data->flagMem;
        implicit_lock = lockable = LIB_TRUE;
        locked = 0u; /* E-3 note 2: the five nominal clocks already use LOCK. */
        break;
    case 0x88u: case 0x89u: case 0x8au: case 0x8bu:
        read = data->flagMem && (opcode & 2u) != 0u;
        miss = 2u;
        break;
    case 0x8fu:
        read = LIB_TRUE;
        miss = data->flagMem ? 2u : 1u;
        break;
    case 0x9du:
        read = LIB_TRUE;
        break;
    case 0xa0u: case 0xa1u:
        read = LIB_TRUE;
        miss = 2u;
        break;
    case 0xa4u: case 0xa5u: case 0xaau: case 0xabu:
    case 0xacu: case 0xadu: case 0xaeu: case 0xafu:
        read = opcode != 0xaau && opcode != 0xabu;
        miss = 2u;
        break;
    case 0xa6u: case 0xa7u:
        if (data->timing_locked_reads != 0u ||
            reads == 0u || reads > 2u || misses < reads || misses > 2u * reads)
            return LIB_FALSE;
        /* E-9 gives six clocks for the pair. A partial hit uses the L2
         * symmetric share; extra physical fragments use E.2's fill bound. */
        *out_ticks = 3u * (lib_u64)reads + 5u * (lib_u64)(misses - reads);
        *out_modeled = reads == 1u || misses > reads;
        return LIB_TRUE;
    case 0xc2u: case 0xc3u:
        read = LIB_TRUE;
        miss = 5u;
        break;
    case 0xcau: case 0xcbu: case 0xcfu: {
        const lib_bool protected_mode = x86_core_cpu_control_stack_is_protected(data);
        const lib_bool outer = protected_mode &&
            data->oldcpu.data.cs.dpl != context->cpu->data.cs.dpl;
        const lib_bool virtual_return = protected_mode &&
            (context->cpu->data.eflags & VCPU_EFLAGS_VM) != 0u;
        const lib_u8 items = virtual_return ? 9u : opcode == 0xcfu ?
            (outer ? 5u : 3u) : outer ? 4u : 2u;
        const lib_u8 lines = (items * (operand32 ? 4u : 2u) + 15u) / 16u;
        const lib_u8 cold = virtual_return ? 19u : opcode == 0xcfu ?
            (outer ? 13u : 8u) : protected_mode ? 6u : 8u;

        if (data->timing_locked_reads != 0u) return LIB_FALSE;
        *out_ticks = misses >= lines ? cold + 5u * (lib_u64)(misses - lines) :
            ((lib_u64)cold * misses + lines - 1u) / lines;
        *out_modeled = (data->oldcpu.data.esp & 15u) != 0u || misses != lines ||
            virtual_return || (opcode == 0xcau && data->cimm != 0u);
        return LIB_TRUE;
    }
    case 0xc9u:
        read = LIB_TRUE;
        miss = 1u;
        break;
    case 0xd7u:
        read = LIB_TRUE;
        miss = 2u;
        break;
    case 0xc0u: case 0xc1u: case 0xd0u: case 0xd1u:
    case 0xd2u: case 0xd3u:
        read = data->flagMem;
        /* Variable carry-rotate has no listed cache-miss surcharge. Its
         * unresolved nominal formula is not qualified by this access row. */
        miss = (extension == 2u || extension == 3u) &&
            opcode != 0xd0u && opcode != 0xd1u ? 0u : 6u;
        break;
    case 0xf6u: case 0xf7u:
        read = data->flagMem;
        lockable = extension == 2u || extension == 3u;
        miss = extension < 2u ? 2u : extension < 4u ? 6u : extension == 4u ? 1u : 0u;
        break;
    case 0xfeu: case 0xffu:
        if (opcode == 0xffu && (extension == 3u || extension == 5u)) {
            if (data->timing_locked_reads != 0u) return LIB_FALSE;
            *out_ticks = (x86_core_cpu_control_stack_is_protected(data) ? 7u :
                extension == 3u ? 8u : 9u) + 5u * (lib_u64)(misses - 1u);
            *out_modeled = misses != 1u;
            return LIB_TRUE;
        }
        if (opcode == 0xffu &&
            (extension == 2u || extension == 4u || extension == 6u)) {
            read = data->flagMem;
            miss = extension == 6u ? 1u : 5u;
            break;
        }
        if (extension > 1u) return LIB_FALSE;
        read = data->flagMem;
        lockable = LIB_TRUE;
        miss = 6u;
        break;
    case 0x0fu: {
        lib_u8 secondary;

        if (prefixes + 2u >= data->oplen) return LIB_FALSE;
        secondary = data->opcodes[prefixes + 1u];
        read = data->flagMem;
        switch (secondary) {
        /* Selected UMOV aliases use the same L2 MOV model as their nominal
         * owner; stores do not acquire a read-miss surcharge. */
        case 0x10u: case 0x11u: read = LIB_FALSE; break;
        case 0x12u: case 0x13u: miss = 2u; break;
        case 0x00u: {
            const lib_u8 operation = (data->opcodes[prefixes + 2u] >> 3u) & 7u;

            if (operation == 2u) miss = 3u;
            else if (operation == 4u || operation == 5u) miss = 4u;
            else if (operation != 3u) return LIB_FALSE;
            break;
        }
        case 0x01u: {
            const lib_u8 operation = (data->opcodes[prefixes + 2u] >> 3u) & 7u;

            if (operation == 2u || operation == 3u) {
                if (data->timing_locked_reads != 0u) return LIB_FALSE;
                *out_ticks = 5u * (lib_u64)misses;
                *out_modeled = misses != 1u;
                return LIB_TRUE;
            }
            if (operation != 6u) return LIB_FALSE;
            miss = 1u;
            break;
        }
        case 0x02u: miss = 2u; break;
        case 0x03u: miss = 3u; break;
        case 0xa1u: case 0xa9u: read = LIB_TRUE; miss = 2u; break;
        case 0xb2u: case 0xb4u: case 0xb5u:
            if (data->timing_locked_reads != 0u) return LIB_FALSE;
            *out_ticks = 7u + 5u * (lib_u64)(misses - 1u);
            *out_modeled = misses != 1u;
            return LIB_TRUE;
        case 0xafu: miss = 1u; break;
        case 0xb6u: case 0xb7u: case 0xbeu: case 0xbfu: miss = 2u; break;
        case 0xa4u: case 0xacu: miss = 6u; break;
        case 0xa5u: case 0xadu: miss = 5u; break;
        case 0xb0u: case 0xb1u: lockable = LIB_TRUE; miss = 2u; break;
        case 0xc0u: case 0xc1u: lockable = LIB_TRUE; miss = 6u; break;
        case 0xa3u: case 0xbcu: miss = 2u; break;
        case 0xbdu: miss = 1u; break;
        case 0xabu: case 0xb3u: case 0xbbu:
            lockable = LIB_TRUE;
            miss = 3u;
            locked = 1u;
            break;
        case 0xbau: {
            const lib_u8 operation = (data->opcodes[prefixes + 2u] >> 3u) & 7u;

            if (operation < 4u) return LIB_FALSE;
            lockable = operation != 4u;
            miss = lockable ? 2u : 1u;
            locked = 0u;
            break;
        }
        default: return LIB_FALSE;
        }
        break;
    }
    default:
        return LIB_FALSE;
    }
    if (!read) return LIB_FALSE;
    if (implicit_lock || data->flagLock) {
        if (!lockable || misses != 0u ||
            data->timing_locked_reads == 0u || data->timing_locked_reads > 2u)
            return LIB_FALSE;
        *out_ticks = locked + 2u * (lib_u64)(data->timing_locked_reads - 1u);
        *out_modeled |= data->timing_locked_reads > 1u;
    } else {
        if (data->timing_locked_reads != 0u || reads != 1u || misses > 2u)
            return LIB_FALSE;
        /* E.2's extra line-fill upper bound, not a second whole-instruction
         * penalty. Split fragments carry an explicit L2 origin below. */
        *out_ticks = miss + 5u * (lib_u64)(misses - 1u);
        *out_modeled |= misses > 1u;
    }
    return LIB_TRUE;
}

/* Common E.2 translation/alignment and E-14 descriptor access terms. */
static lib_bool x86_core_cpu_80486_access_penalty(const t_cpuins_data *data,
    lib_bool descriptors, lib_bool task, lib_u64 *out_ticks, lib_bool *out_modeled)
{
    lib_u64 walks, attempts, saved_transfers, ticks;

    if (data->timing_missed_reads > data->timing_data_misses ||
        data->timing_descriptor_misses > data->timing_data_misses ||
        data->timing_missed_descriptors > data->timing_missed_reads ||
        data->timing_missed_descriptors > data->timing_descriptor_misses ||
        (lib_u64)data->timing_pair_misses + data->timing_tss_field_misses +
            data->timing_descriptor_misses + data->timing_idt_misses > data->timing_data_misses)
        return LIB_FALSE;
    walks = (lib_u64)data->timing_tlb_walks[0] + data->timing_tlb_walks[1] +
        data->timing_tlb_walks[2];
    attempts = (lib_u64)data->timing_tlb_walks[1] + 2u * (lib_u64)data->timing_tlb_walks[2];
    if (data->timing_table_misses > 2u * walks || data->timing_tlb_updates > attempts)
        return LIB_FALSE;
    /* Cold 13/21/28 remains source-backed. Cached reads and raced updates
     * use an explicit L2 differential model: save two fastest-bus clocks
     * per avoided read miss or write, not a second complete bus subtotal. */
    saved_transfers = 2u * walks - data->timing_table_misses +
        attempts - data->timing_tlb_updates;
    ticks = 13u * (lib_u64)data->timing_tlb_walks[0] +
        21u * (lib_u64)data->timing_tlb_walks[1] +
        28u * (lib_u64)data->timing_tlb_walks[2] - 2u * saved_transfers +
        3u * (lib_u64)data->timing_unaligned_accesses +
        11u * (lib_u64)data->timing_unaccessed_descriptors;
    if (descriptors)
        ticks += 3u * (lib_u64)data->timing_missed_descriptors +
            5u * (lib_u64)(data->timing_descriptor_misses - data->timing_missed_descriptors);
    /* A cold Gate and target TSS share one transfer term, whether the Gate
     * came from GDT/LDT or IDT. Preserve independent partial residency. */
    if (task && data->timing_task_gate_miss_mask == 3u) {
        if (ticks < 3u) return LIB_FALSE;
        ticks -= 3u;
    }
    *out_ticks = ticks;
    *out_modeled = saved_transfers != 0u ||
        data->timing_descriptor_misses > data->timing_missed_descriptors;
    return LIB_TRUE;
}

/* Intel 1992 PRM Appendix E.2--E-15: source rows and qualified access models. */
lib_bool x86_core_cpu_80486_memory_penalty(
    const x86_core_cpu_execution_context *context, lib_u64 *out_ticks, lib_bool *out_modeled)
{
    const t_cpuins_data *data;
    lib_u32 prefixes;
    lib_u64 ticks, operand;
    lib_bool operand_modeled, access_modeled, descriptors;

    if (context == LIB_NULL || out_ticks == LIB_NULL ||
        context->instructions == LIB_NULL || context->cpu == LIB_NULL ||
        context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX ||
        context->instruction_fault_delivered || context->stop_requested)
        return LIB_FALSE;
    data = &context->instructions->data;
    if (data->except != 0u) return LIB_FALSE;
    prefixes = x86_core_cpu_instruction_prefix_count(context, data);
    if (prefixes >= data->oplen) return LIB_FALSE;
    /* E-12 deliberately leaves LTR's miss column blank. */
    descriptors = !(data->opcodes[prefixes] == 0x0fu && prefixes + 2u < data->oplen &&
        data->opcodes[prefixes + 1u] == 0x00u &&
        ((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 3u);
    if (!x86_core_cpu_80486_access_penalty(data, descriptors,
            context->instruction_task_switched, &ticks, &access_modeled) ||
        !x86_core_cpu_80486_operand_cache_penalty(context, prefixes, &operand, &operand_modeled))
        return LIB_FALSE;
    ticks += operand;
    *out_ticks = ticks;
    if (out_modeled != LIB_NULL)
        *out_modeled = access_modeled || operand_modeled;
    return LIB_TRUE;
}

/* Entry counters are scoped by the executor; there is no decoded opcode and
 * no instruction-wide exception/stop gate at this internal calculation seam. */
lib_u64 x86_core_cpu_80486_interrupt_entry_cost(
    const x86_core_cpu_execution_context *context,
    lib_u32 entry_cr0, lib_u32 entry_flags, lib_u8 entry_cpl, lib_bool task_entry)
{
    const t_cpuins_data *data = &context->instructions->data;
    lib_u64 ticks = x86_core_cpu_80486_nominal_interrupt_entry_cost(context,
        entry_cr0, entry_flags, entry_cpl, task_entry);
    lib_u64 access, operand;
    lib_bool modeled;
    const lib_bool real = (entry_cr0 & VCPU_CR0_PE) == 0u ||
        (context->cpu->data.eflags & VCPU_EFLAGS_VM) != 0u;
    const lib_bool inner = !real && ((entry_flags & VCPU_EFLAGS_VM) != 0u ||
        entry_cpl > context->cpu->data.cs.dpl);

    if (data->except != 0u) return 0u;
    if (!x86_core_cpu_80486_access_penalty(data, LIB_TRUE, task_entry, &access, &modeled) ||
        !(task_entry ? x86_core_cpu_80486_task_cache_penalty(context, &operand, &modeled) :
            x86_core_cpu_80486_interrupt_cache_penalty(data, real, inner, &operand, &modeled)) ||
        access > LIB_UINT64_MAX - ticks || operand > LIB_UINT64_MAX - ticks - access)
        return 0u;
    return ticks + access + operand;
}

/* Explicit L2 choices for conflicting formulas and unallocated special forms,
 * not hardware-exact
 * cache-hit rows. Keep access/prefix costs and result qualification separate. */
lib_bool x86_core_cpu_80486_l2_instruction_cost(
    const x86_core_cpu_execution_context *context, lib_u64 *out_ticks)
{
    const t_cpuins_data *data;
    lib_u32 prefixes;
    lib_u8 opcode, extension, bytes;
    lib_u64 multiplier, magnitude, scale, ticks;

    if (context == LIB_NULL || out_ticks == LIB_NULL || context->instructions == LIB_NULL ||
        context->cpu == LIB_NULL || context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX ||
        context->instruction_fault_delivered || context->stop_requested) return LIB_FALSE;
    data = &context->instructions->data;
    if (data->except != 0u) return LIB_FALSE;
    prefixes = x86_core_cpu_instruction_prefix_count(context, data);
    if (prefixes >= data->oplen) return LIB_FALSE;
    opcode = data->opcodes[prefixes];
    extension = prefixes + 1u < data->oplen ? (data->opcodes[prefixes + 1u] >> 3u) & 7u : 0u;
    bytes = data->oldcpu.data.cs.seg.exec.defsize != data->prefix_oprsize ? 4u : 2u;

    if (opcode == 0x0fu && prefixes + 1u < data->oplen &&
        data->opcodes[prefixes + 1u] == 0xaau) {
        return x86_core_cpu_80486_smm_cost(context, LIB_FALSE, LIB_TRUE,
            data->timing_fetch_bus_ticks, out_ticks);
    } else if (opcode == 0x9bu) {
        /* PRM 26-280 gives 1--3, not an exact no-FPU SX value. This L2
         * model selects the upper endpoint for a successful WAIT only. */
        ticks = 3u;
    } else if (opcode >= 0xd8u && opcode <= 0xdfu &&
        (data->oldcpu.data.cr0 & (VCPU_CR0_EM | VCPU_CR0_TS)) == 0u) {
        /* No numeric device: use the one-clock no-op decode envelope as L2.
         * Actual stores, fetch, prefixes and waits keep their existing owners. */
        ticks = 1u;
    } else if (opcode == 0xd6u || (opcode == 0x0fu && prefixes + 2u < data->oplen &&
            data->opcodes[prefixes + 1u] >= 0x10u &&
            data->opcodes[prefixes + 1u] <= 0x13u)) {
        /* SALC uses the selected-SX reference model's register clock;
         * ordinary UMOV uses the canonical MOV cache-hit model. Neither
         * undocumented encoding is an original-table timing claim. */
        ticks = 1u;
    } else if ((opcode == 0xc0u || opcode == 0xc1u || opcode == 0xd2u || opcode == 0xd3u) &&
        (extension == 2u || extension == 3u)) {
        const lib_u8 width = (opcode & 1u) != 0u ? bytes * 8u : 8u;
        const lib_u8 count = (opcode < 0xd0u ? data->cimm : data->oldcpu.data.ecx) & 31u;

        /* Notes 4/5: preserve the pre-modulo count, and follow their printed
         * +9 equation despite the inconsistent memory range endpoint. */
        ticks = count <= width ? (data->flagMem ? 9u : 8u) : (count / width) * 7u + 9u;
    } else {
        if ((opcode == 0xf6u || opcode == 0xf7u) && extension == 5u) {
            if (opcode == 0xf6u) bytes = 1u;
            multiplier = data->crm;
        } else if (opcode == 0x69u || opcode == 0x6bu) {
            multiplier = data->cimm;
            if (opcode == 0x6bu) bytes = 1u;
        } else if (opcode == 0x0fu && prefixes + 2u < data->oplen &&
            data->opcodes[prefixes + 1u] == 0xafu) multiplier = data->crm;
        else return LIB_FALSE;
        magnitude = x86_core_cpu_source_timing_signed_magnitude(multiplier, bytes);
        scale = x86_core_cpu_source_timing_ceiling_log2(magnitude);
        const lib_u8 minimum = (multiplier & (UINT64_C(1) << (bytes * 8u - 1u))) != 0u ? 5u : 3u;

        /* Note 3 supplies 10/n; chapter 26 supplies ceiling rounding. The
         * final encoded source is this model's optimizing multiplier. */
        ticks = magnitude == 0u ? 13u : 10u + (scale < minimum ? minimum : scale);
    }
    *out_ticks = ticks;
    return LIB_TRUE;
}

/* Intel 1992 PRM Appendix E, E-3--E-13: cache-hit costs, before the
 * separately specified prefix, access and event terms. Use the decoder's
 * completed operands; timing must not read guest memory a second time. */
lib_bool x86_core_cpu_80486_nominal_instruction_cost(
    const x86_core_cpu_execution_context *context, lib_u64 *out_ticks)
{
    const t_cpuins_data *data;
    lib_u32 prefixes;
    lib_u8 opcode, extension;
    lib_bool memory, protected_mode;
    lib_u64 ticks;

    if (context == LIB_NULL || out_ticks == LIB_NULL ||
        context->instructions == LIB_NULL || context->cpu == LIB_NULL ||
        context->cpu_profile != X86_CORE_CPU_PROFILE_80486SX ||
        context->instruction_fault_delivered || context->stop_requested)
        return LIB_FALSE;
    data = &context->instructions->data;
    if (data->except != 0u) return LIB_FALSE;
    prefixes = x86_core_cpu_instruction_prefix_count(context, data);
    if (prefixes >= data->oplen) return LIB_FALSE;
    opcode = data->opcodes[prefixes];
    extension = prefixes + 1u < data->oplen ?
        (data->opcodes[prefixes + 1u] >> 3u) & 7u : 0u;
    memory = data->flagMem != 0u;
    protected_mode = (data->oldcpu.data.cr0 & VCPU_CR0_PE) != 0u &&
        (data->oldcpu.data.eflags & VCPU_EFLAGS_VM) == 0u;

    if (opcode <= 0x3du && (opcode & 7u) <= 5u) {
        const lib_u8 form = opcode & 7u;
        const lib_bool compare = (opcode & 0x38u) == 0x38u;

        ticks = !memory || form >= 4u ? 1u :
            compare || (form & 2u) != 0u ? 2u : 3u;
    } else if (opcode >= 0x40u && opcode <= 0x5fu) {
        ticks = 1u;
    } else if (opcode >= 0xb0u && opcode <= 0xbfu) {
        ticks = 1u;
    } else if (opcode >= 0x91u && opcode <= 0x97u) {
        ticks = 3u;
    } else if (opcode >= 0x70u && opcode <= 0x7fu) {
        lib_i32 taken;

        if (!x86_core_cpu_control_stack_condition_taken(data->oldcpu.data.eflags,
                opcode, &taken)) return LIB_FALSE;
        ticks = taken ? 3u : 1u;
    } else switch (opcode) {
    case 0x06u: case 0x0eu: case 0x16u: case 0x1eu:
        ticks = 3u;
        break;
    case 0x07u: case 0x17u: case 0x1fu:
        ticks = protected_mode ? 9u : 3u;
        break;
    case 0x27u: case 0x2fu:
        ticks = 2u;
        break;
    case 0x37u: case 0x3fu:
        ticks = 3u;
        break;
    case 0x60u:
        ticks = 11u;
        break;
    case 0x61u:
        ticks = 9u;
        break;
    case 0x63u:
        ticks = 9u;
        break;
    case 0x62u:
        ticks = 7u;
        break;
    case 0x68u: case 0x6au:
        ticks = 1u;
        break;
    case 0x6cu: case 0x6du: case 0x6eu: case 0x6fu:
        ticks = x86_core_cpu_80486_nominal_io_cost(context, data, opcode);
        break;
    case 0x80u: case 0x81u: case 0x82u: case 0x83u:
        ticks = !memory ? 1u : extension == 7u ? 2u : 3u;
        break;
    case 0x84u: case 0x85u:
        ticks = memory ? 2u : 1u;
        break;
    case 0x86u: case 0x87u:
        ticks = memory ? 5u : 3u;
        break;
    case 0x88u: case 0x89u: case 0x8au: case 0x8bu:
    case 0x90u: case 0xa0u: case 0xa1u: case 0xa2u: case 0xa3u:
    case 0xc6u: case 0xc7u:
        ticks = 1u;
        break;
    case 0x8cu:
        ticks = 3u;
        break;
    case 0x8du:
        ticks = data->timing_ea_indexed ? 2u : 1u;
        break;
    case 0x8eu:
        ticks = protected_mode ? 9u : 3u;
        break;
    case 0x8fu:
        ticks = memory ? 5u : 4u;
        break;
    case 0x98u: case 0x99u: case 0x9fu:
        ticks = 3u;
        break;
    case 0x9cu:
        ticks = protected_mode ? 3u : 4u;
        break;
    case 0x9du:
        ticks = protected_mode ? 6u : 9u;
        break;
    case 0x9eu:
        ticks = 2u;
        break;
    case 0xa8u: case 0xa9u:
        ticks = 1u;
        break;
    case 0xa4u: case 0xa5u: case 0xa6u: case 0xa7u:
    case 0xaau: case 0xabu: case 0xacu: case 0xadu:
    case 0xaeu: case 0xafu:
        ticks = opcode <= 0xa5u ? 7u : opcode <= 0xa7u ? 8u :
            opcode <= 0xadu ? 5u : 6u;
        if (data->prefix_rep != PREFIX_REP_NONE) {
            const lib_bool address32 = data->oldcpu.data.cs.seg.exec.defsize !=
                data->prefix_addrsize;
            const lib_u32 count = address32 ? data->oldcpu.data.ecx : data->oldcpu.data.cx;
            const lib_bool continuing = context->timing_result.repeat_phase ==
                X86_CORE_CPU_RETIREMENT_REPEAT_CONTINUATION;

            if (count == 0u) ticks = 5u;
            else if (opcode <= 0xa5u) ticks = continuing ? 3u : count == 1u ? 13u : 15u;
            else {
                const lib_u64 iteration = opcode <= 0xa7u ? 7u : opcode <= 0xadu ? 4u : 5u;

                ticks = iteration + (continuing ? 0u : 7u);
            }
        }
        break;
    case 0xc0u: case 0xc1u: case 0xd0u: case 0xd1u:
    case 0xd2u: case 0xd3u:
        if ((extension == 2u || extension == 3u) &&
            opcode != 0xd0u && opcode != 0xd1u) return LIB_FALSE;
        ticks = memory ? 4u : opcode == 0xc0u || opcode == 0xc1u ? 2u : 3u;
        break;
    case 0xc4u: case 0xc5u:
        ticks = protected_mode ? 12u : 6u;
        break;
    case 0xc2u: case 0xc3u:
        ticks = 5u;
        break;
    case 0x9au: case 0xeau:
        ticks = x86_core_cpu_80486_nominal_far_cost(context, data, opcode == 0x9au, LIB_FALSE);
        break;
    case 0xcau: case 0xcbu:
        ticks = !protected_mode ? 13u :
            data->oldcpu.data.cs.dpl == context->cpu->data.cs.dpl ? 17u : 35u;
        if (opcode == 0xcau) ++ticks;
        break;
    case 0xccu: case 0xcdu: case 0xceu:
        if (opcode == 0xceu && (data->oldcpu.data.eflags & VCPU_EFLAGS_OF) == 0u)
            ticks = 3u;
        else {
            ticks = x86_core_cpu_80486_nominal_interrupt_entry_cost(context,
                data->oldcpu.data.cr0, data->oldcpu.data.eflags, data->oldcpu.data.cs.dpl,
                context->instruction_task_switched);
            if (opcode == 0xcdu && !protected_mode) ticks += 4u;
            if (opcode == 0xceu) ticks += 2u;
        }
        break;
    case 0xcfu:
        ticks = !protected_mode ? 15u : context->instruction_task_switched ?
            32u + x86_core_cpu_80486_nominal_task_cost(context) :
            data->oldcpu.data.cs.dpl == context->cpu->data.cs.dpl ? 20u : 36u;
        break;
    case 0xc8u: {
        lib_u8 level;

        if (prefixes + 3u >= data->oplen) return LIB_FALSE;
        level = data->opcodes[prefixes + 3u] & 31u;
        ticks = level == 0u ? 14u : level == 1u ? 17u : 17u + 3u * level;
        break;
    }
    case 0xc9u:
        ticks = 5u;
        break;
    case 0xd4u:
        ticks = 15u;
        break;
    case 0xd5u:
        ticks = 14u;
        break;
    case 0xd7u:
        ticks = 4u;
        break;
    case 0xe0u: case 0xe1u: case 0xe2u: case 0xe3u: {
        const lib_bool address32 = data->oldcpu.data.cs.seg.exec.defsize !=
            data->prefix_addrsize;
        lib_u32 count = address32 ? data->oldcpu.data.ecx : data->oldcpu.data.cx;
        const lib_bool zero = (data->oldcpu.data.eflags & VCPU_EFLAGS_ZF) != 0u;
        lib_bool taken;

        if (opcode == 0xe3u) ticks = count == 0u ? 8u : 5u;
        else {
            count = address32 ? count - 1u : (lib_u16)(count - 1u);
            taken = count != 0u && (opcode == 0xe2u ||
                (opcode == 0xe1u ? zero : !zero));
            ticks = taken ? (opcode == 0xe2u ? 7u : 9u) : 6u;
        }
        break;
    }
    case 0xe4u: case 0xe5u: case 0xe6u: case 0xe7u:
    case 0xecu: case 0xedu: case 0xeeu: case 0xefu:
        ticks = x86_core_cpu_80486_nominal_io_cost(context, data, opcode);
        break;
    case 0xe8u: case 0xe9u: case 0xebu:
        ticks = 3u;
        break;
    case 0xf4u:
        ticks = 4u;
        break;
    case 0xf5u: case 0xf8u: case 0xf9u: case 0xfcu: case 0xfdu:
        ticks = 2u;
        break;
    case 0xfau: case 0xfbu:
        ticks = 5u;
        break;
    case 0xf6u: case 0xf7u:
        if (extension == 5u) return LIB_FALSE;
        if (extension == 4u) {
            const lib_bool dword = data->oldcpu.data.cs.seg.exec.defsize !=
                data->prefix_oprsize;
            const lib_u64 mask = opcode == 0xf6u ? LIB_UINT8_MAX :
                dword ? LIB_UINT32_MAX : LIB_UINT16_MAX;
            const lib_u64 scale = x86_core_cpu_source_timing_ceiling_log2(data->crm & mask);

            /* 1992 PRM 26-218, corroborated by E-5/E-14. */
            ticks = 10u + (scale < 3u ? 3u : scale);
        } else if (extension >= 6u) {
            const lib_bool dword = data->oldcpu.data.cs.seg.exec.defsize !=
                data->prefix_oprsize;

            ticks = opcode == 0xf6u ? 16u : dword ? 40u : 24u;
            if (extension == 7u) ticks += 3u + memory;
        } else ticks = !memory ? 1u : extension < 2u ? 2u : 3u;
        break;
    case 0xfeu:
        if (extension > 1u) return LIB_FALSE;
        ticks = memory ? 3u : 1u;
        break;
    case 0xffu:
        if (extension < 2u) ticks = memory ? 3u : 1u;
        else if (extension == 6u) ticks = 4u;
        else if (extension == 2u || extension == 4u) ticks = 5u;
        else if (extension == 3u || extension == 5u)
            ticks = x86_core_cpu_80486_nominal_far_cost(context, data, extension == 3u, LIB_TRUE);
        else return LIB_FALSE;
        break;
    case 0x0fu: {
        lib_u8 secondary;

        if (prefixes + 1u >= data->oplen) return LIB_FALSE;
        secondary = data->opcodes[prefixes + 1u];
        extension = prefixes + 2u < data->oplen ?
            (data->opcodes[prefixes + 2u] >> 3u) & 7u : 0u;
        if (secondary >= 0xc8u && secondary <= 0xcfu) ticks = 1u;
        else if (secondary >= 0x80u && secondary <= 0x9fu) {
            lib_i32 taken;

            if (!x86_core_cpu_control_stack_condition_taken(data->oldcpu.data.eflags,
                    secondary, &taken)) return LIB_FALSE;
            ticks = secondary < 0x90u ? (taken ? 3u : 1u) :
                memory ? (taken ? 3u : 4u) : (taken ? 4u : 3u);
        }
        else switch (secondary) {
        case 0x00u:
            if (extension == 0u || extension == 1u) ticks = memory ? 3u : 2u;
            else if (extension == 2u || extension == 4u || extension == 5u) ticks = 11u;
            else if (extension == 3u) ticks = 20u;
            else return LIB_FALSE;
            break;
        case 0x01u:
            if (extension < 2u) ticks = 10u;
            else if (extension < 4u) ticks = 12u;
            else if (extension == 4u) ticks = memory ? 3u : 2u;
            else if (extension == 6u) ticks = 13u;
            else if (extension == 7u) ticks = data->timing_invlpg_hit ? 12u : 11u;
            else return LIB_FALSE;
            break;
        case 0x02u:
            ticks = 11u;
            break;
        case 0x03u:
            ticks = 10u;
            break;
        case 0x06u:
            ticks = 7u;
            break;
        case 0x08u:
            ticks = 4u;
            break;
        case 0x09u:
            ticks = 5u;
            break;
        case 0x20u:
            ticks = 4u;
            break;
        case 0x21u:
            ticks = 9u;
            break;
        case 0x22u:
            ticks = extension == 0u ? 17u : 4u;
            break;
        case 0x23u:
            ticks = 10u;
            break;
        case 0x24u:
            ticks = extension == 3u ? 3u : 4u;
            break;
        case 0x26u:
            ticks = 4u;
            break;
        case 0xa0u: case 0xa8u:
            ticks = 3u;
            break;
        case 0xa1u: case 0xa9u:
            ticks = protected_mode ? 9u : 3u;
            break;
        case 0xa2u:
            /* Intel 273021-001, Table D-1: selected late-SX supplement. */
            ticks = data->oldcpu.data.eax == 1u ? 14u : 9u;
            break;
        case 0xa3u:
            ticks = memory ? 8u : 3u;
            break;
        case 0xabu: case 0xb3u: case 0xbbu:
            ticks = memory ? 13u : 6u;
            break;
        case 0xbau:
            if (extension < 4u) return LIB_FALSE;
            ticks = extension == 4u ? 3u : memory ? 8u : 6u;
            break;
        case 0xa4u: case 0xacu:
            ticks = memory ? 3u : 2u;
            break;
        case 0xa5u: case 0xadu:
            ticks = memory ? 4u : 3u;
            break;
        case 0xb0u: case 0xb1u:
            ticks = !memory ? 6u :
                (context->cpu->data.eflags & VCPU_EFLAGS_ZF) != 0u ? 7u : 10u;
            break;
        case 0xb2u: case 0xb4u: case 0xb5u:
            ticks = protected_mode ? 12u : 6u;
            break;
        case 0xb6u: case 0xb7u: case 0xbeu: case 0xbfu:
            ticks = 3u;
            break;
        case 0xbcu: case 0xbdu:
            if (data->opr1 == 0u) ticks = 6u + memory;
            else if (secondary == 0xbdu) ticks = 7u + 3u * (32u - data->result) + memory;
            else {
                const lib_u32 bit = (lib_u32)data->result;

                ticks = 8u + 4u * ((bit >> 3u) + 1u) +
                    3u * (((bit >> 2u) & 1u) + 1u) + 3u * ((bit & 3u) + 1u) + memory;
            }
            break;
        case 0xc0u: case 0xc1u:
            ticks = memory ? 4u : 3u;
            break;
        default:
            return LIB_FALSE;
        }
        break;
    }
    default:
        return LIB_FALSE;
    }
    *out_ticks = ticks;
    return LIB_TRUE;
}
