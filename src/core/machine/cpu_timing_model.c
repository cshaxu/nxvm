#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/cpu_timing.h"

C_INT core_machine_timing_add_ticks(type_unsigned_64 *value,
    type_unsigned_64 delta)
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
    CORE_MACHINE_SOURCE_TIMING_CMC,
    CORE_MACHINE_SOURCE_TIMING_STC,
    CORE_MACHINE_SOURCE_TIMING_STD,
    CORE_MACHINE_SOURCE_TIMING_STI,
    CORE_MACHINE_SOURCE_TIMING_GROUP2_ROTATE_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_GROUP2_ROTATE_MEMORY,
    CORE_MACHINE_SOURCE_TIMING_GROUP2_CARRY_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_GROUP2_CARRY_MEMORY,
    CORE_MACHINE_SOURCE_TIMING_MOV_EXTEND_REGISTER,
    CORE_MACHINE_SOURCE_TIMING_MOV_EXTEND_MEMORY,
    CORE_MACHINE_SOURCE_TIMING_MOV_POINTER_REAL,
    CORE_MACHINE_SOURCE_TIMING_DYNAMIC_MUL,
    CORE_MACHINE_SOURCE_TIMING_DYNAMIC_IMUL,
    CORE_MACHINE_SOURCE_TIMING_DYNAMIC_IMUL_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_DYNAMIC_IMUL_TWO_OPERAND,
    CORE_MACHINE_SOURCE_TIMING_DYNAMIC_BSF,
    CORE_MACHINE_SOURCE_TIMING_DYNAMIC_BSR,
    CORE_MACHINE_SOURCE_TIMING_DYNAMIC_BT,
    CORE_MACHINE_SOURCE_TIMING_DYNAMIC_BTC,
    CORE_MACHINE_SOURCE_TIMING_DYNAMIC_BTR,
    CORE_MACHINE_SOURCE_TIMING_DYNAMIC_BTS,
    CORE_MACHINE_SOURCE_TIMING_DYNAMIC_SHLD,
    CORE_MACHINE_SOURCE_TIMING_DYNAMIC_SHRD,
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
    CORE_MACHINE_SOURCE_TIMING_RET_FAR,
    CORE_MACHINE_SOURCE_TIMING_RET_FAR_IMMEDIATE,
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
    CORE_MACHINE_SOURCE_TIMING_JCC,
    CORE_MACHINE_SOURCE_TIMING_INTO,
    CORE_MACHINE_SOURCE_TIMING_HLT,
    CORE_MACHINE_SOURCE_TIMING_INT3,
    CORE_MACHINE_SOURCE_TIMING_INT_IMMEDIATE,
    CORE_MACHINE_SOURCE_TIMING_IRET,
    CORE_MACHINE_SOURCE_TIMING_8086_LOAD_POINTER,
    CORE_MACHINE_SOURCE_TIMING_8086_RET_FAR,
    CORE_MACHINE_SOURCE_TIMING_8086_GROUP2,
    CORE_MACHINE_SOURCE_TIMING_8086_FLAG,
    CORE_MACHINE_SOURCE_TIMING_WAIT,
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

/* A source evaluator consumes this immediately; it is never machine state. */
typedef struct core_machine_source_transfer_plan {
    core_machine_source_timing_form form;
    type_unsigned_8 word_transfers;
    type_bool complete;
} core_machine_source_transfer_plan;

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
    { CORE_MACHINE_SOURCE_TIMING_RET_FAR, 18u },
    { CORE_MACHINE_SOURCE_TIMING_RET_FAR_IMMEDIATE, 17u },
    { CORE_MACHINE_SOURCE_TIMING_PUSH_REGISTER, 11u },
    { CORE_MACHINE_SOURCE_TIMING_PUSH_MEMORY, 16u },
    { CORE_MACHINE_SOURCE_TIMING_POP_REGISTER, 8u },
    { CORE_MACHINE_SOURCE_TIMING_POP_MEMORY, 17u },
    { CORE_MACHINE_SOURCE_TIMING_PUSHF, 10u },
    { CORE_MACHINE_SOURCE_TIMING_POPF, 8u },
    { CORE_MACHINE_SOURCE_TIMING_HLT, 2u },
    { CORE_MACHINE_SOURCE_TIMING_INT3, 52u },
    { CORE_MACHINE_SOURCE_TIMING_INT_IMMEDIATE, 51u },
    { CORE_MACHINE_SOURCE_TIMING_INTO, 53u },
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
    { CORE_MACHINE_SOURCE_TIMING_RET_FAR, 22u },
    { CORE_MACHINE_SOURCE_TIMING_RET_FAR_IMMEDIATE, 25u },
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
    { CORE_MACHINE_SOURCE_TIMING_RET_FAR, 15u },
    { CORE_MACHINE_SOURCE_TIMING_RET_FAR_IMMEDIATE, 15u },
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
    { CORE_MACHINE_SOURCE_TIMING_CMC, 2u },
    { CORE_MACHINE_SOURCE_TIMING_STC, 2u },
    { CORE_MACHINE_SOURCE_TIMING_STD, 2u },
    { CORE_MACHINE_SOURCE_TIMING_STI, 3u },
    { CORE_MACHINE_SOURCE_TIMING_GROUP2_ROTATE_REGISTER, 3u },
    { CORE_MACHINE_SOURCE_TIMING_GROUP2_ROTATE_MEMORY, 7u },
    { CORE_MACHINE_SOURCE_TIMING_GROUP2_CARRY_REGISTER, 9u },
    { CORE_MACHINE_SOURCE_TIMING_GROUP2_CARRY_MEMORY, 10u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_EXTEND_REGISTER, 3u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_EXTEND_MEMORY, 6u },
    { CORE_MACHINE_SOURCE_TIMING_MOV_POINTER_REAL, 7u },
    { CORE_MACHINE_SOURCE_TIMING_BOUND, 10u },
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
    { CORE_MACHINE_SOURCE_TIMING_RET_FAR, 18u },
    { CORE_MACHINE_SOURCE_TIMING_RET_FAR_IMMEDIATE, 18u },
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
    { CORE_MACHINE_SOURCE_TIMING_WAIT, 7u },
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
    { CORE_MACHINE_SOURCE_TIMING_STRING_CMPS, 8u, 5u, 9u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_STOS, 3u, 4u, 3u },
    /* Appendix B: REP LODS follows the S + 4*CX string formula. */
    { CORE_MACHINE_SOURCE_TIMING_STRING_LODS, 5u, 5u, 4u },
    { CORE_MACHINE_SOURCE_TIMING_STRING_SCAS, 7u, 5u, 8u },
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
static core_machine_source_transfer_plan
    core_machine_source_timing_string_transfer_plan(
        const t_cpuins_data *data, core_machine_source_timing_form form);
static type_unsigned_64 core_machine_source_timing_repeat_string(
    core_machine *machine, const t_cpuins_data *data,
    const core_machine_source_repeat_timing_entry *entry,
    const core_machine_source_transfer_plan *transfer_plan);

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
    case CORE_MACHINE_CPU_PROFILE_8088:
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
    core_machine_source_transfer_plan transfer_plan = {0};
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
                &port_entry, STD_NULL);
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
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088) {
            transfer_plan = core_machine_source_timing_string_transfer_plan(data,
                form);
            if (!transfer_plan.complete) {
                machine->source_repeat_active = TYPE_FALSE;
                return 0;
            }
        }
        *out_ticks = core_machine_source_timing_repeat_string(machine, data,
            entry, &transfer_plan);
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
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088) {
        transfer_plan.form = form;
        transfer_plan.word_transfers = opcode & 1u;
        transfer_plan.complete = TYPE_TRUE;
        *out_ticks = core_machine_source_timing_lookup(machine,
            core_machine_8086_source_timing_ledger,
            sizeof(core_machine_8086_source_timing_ledger) /
                sizeof(core_machine_8086_source_timing_ledger[0]), form);
        return core_machine_timing_add_ticks(out_ticks,
            (type_unsigned_64)transfer_plan.word_transfers *
                CORE_MACHINE_8086_ODD_WORD_TICKS);
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

static core_machine_source_transfer_plan
    core_machine_source_timing_string_transfer_plan(
        const t_cpuins_data *data, core_machine_source_timing_form form)
{
    core_machine_source_transfer_plan plan = { form, 0u, TYPE_FALSE };
    type_unsigned_32 prefixes;

    if (data == STD_NULL) return plan;
    prefixes = core_machine_instruction_prefix_count(data);
    if (prefixes >= data->oplen) return plan;
    if ((data->opcodes[prefixes] & 1u) == 0u) {
        plan.complete = TYPE_TRUE;
        return plan;
    }
    switch (form) {
    case CORE_MACHINE_SOURCE_TIMING_STRING_MOVS:
    case CORE_MACHINE_SOURCE_TIMING_STRING_CMPS:
        plan.word_transfers = 2u;
        break;
    case CORE_MACHINE_SOURCE_TIMING_STRING_STOS:
    case CORE_MACHINE_SOURCE_TIMING_STRING_LODS:
    case CORE_MACHINE_SOURCE_TIMING_STRING_SCAS:
        plan.word_transfers = 1u;
        break;
    default:
        return plan;
    }
    plan.complete = TYPE_TRUE;
    return plan;
}

static type_unsigned_64 core_machine_source_timing_string_modifiers(
    const core_machine *machine, const t_cpuins_data *data,
    core_machine_source_timing_form form,
    const core_machine_source_transfer_plan *transfer_plan)
{
    type_unsigned_32 index;
    type_unsigned_8 opcode = 0u;
    type_unsigned_64 modifiers = 0u;
    C_INT segment_override = 0;
    C_INT word;
    C_INT source_transfer;
    C_INT destination_transfer;
    type_unsigned_64 odd_word_ticks;

    if (machine == STD_NULL || data == STD_NULL ||
        (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8086 &&
         machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8088 &&
         machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186 &&
         machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80286)) return 0u;
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
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088) {
        if (transfer_plan == STD_NULL || !transfer_plan->complete ||
            machine->source_timing_repeat_phase ==
            CORE_MACHINE_RETIREMENT_REPEAT_ZERO_COUNT) return modifiers;
        return modifiers + (type_unsigned_64)transfer_plan->word_transfers *
            CORE_MACHINE_8086_ODD_WORD_TICKS;
    }
    word = (opcode & 1u) != 0u;
    if (!word || machine->source_timing_repeat_phase ==
        CORE_MACHINE_RETIREMENT_REPEAT_ZERO_COUNT) return modifiers;
    source_transfer = form == CORE_MACHINE_SOURCE_TIMING_STRING_MOVS ||
        form == CORE_MACHINE_SOURCE_TIMING_STRING_CMPS ||
        form == CORE_MACHINE_SOURCE_TIMING_STRING_LODS ||
        form == CORE_MACHINE_SOURCE_TIMING_STRING_OUTS;
    destination_transfer = form == CORE_MACHINE_SOURCE_TIMING_STRING_MOVS ||
        form == CORE_MACHINE_SOURCE_TIMING_STRING_CMPS ||
        form == CORE_MACHINE_SOURCE_TIMING_STRING_STOS ||
        form == CORE_MACHINE_SOURCE_TIMING_STRING_SCAS ||
        form == CORE_MACHINE_SOURCE_TIMING_STRING_INS;
    odd_word_ticks = machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ?
        CORE_MACHINE_80286_ODD_WORD_TICKS : CORE_MACHINE_8086_ODD_WORD_TICKS;
    if (source_transfer && (data->oldcpu.data.si & 1u) != 0u) {
        modifiers += odd_word_ticks;
    }
    if (destination_transfer && (data->oldcpu.data.di & 1u) != 0u) {
        modifiers += odd_word_ticks;
    }
    return modifiers;
}

static type_unsigned_64 core_machine_source_timing_repeat_string(
    core_machine *machine, const t_cpuins_data *data,
    const core_machine_source_repeat_timing_entry *entry,
    const core_machine_source_transfer_plan *transfer_plan)
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
        return entry->primitive_ticks + core_machine_source_timing_string_modifiers(
            machine, data, entry->form, transfer_plan);
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
    return ticks + core_machine_source_timing_string_modifiers(machine, data,
        entry->form, transfer_plan);
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
    case CORE_MACHINE_SOURCE_TIMING_MOV_RM_REGISTER:
    case CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_RM:
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

static core_machine_source_transfer_plan
    core_machine_source_timing_primary_transfer_plan(
        const core_machine_primary_timing_shape *shape)
{
    core_machine_source_transfer_plan plan = {0};

    if (shape == STD_NULL) return plan;
    plan.form = shape->form;
    plan.word_transfers = core_machine_source_timing_primary_word_transfers(shape);
    plan.complete = TYPE_TRUE;
    return plan;
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
 * 80186 reference model already includes effective-address time; Table 1-16
 * still supplies its independent odd-word and segment-prefix terms. */
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
    if (shape.memory) {
        if (shape.word) ticks += core_machine_8086_timing_odd_word(data);
        if (segment_override) ticks += CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS;
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
                /* A word memory rotate/shift is a read-modify-write: Table
                 * 1-16 charges the odd-address term to both 16-bit transfers. */
                *out_ticks += (opcode & 1u ? (type_unsigned_64)2u *
                    core_machine_8086_timing_odd_word(data) : 0u) +
                    (segment_override ?
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
                prefixes) + (opcode & 1u ? (type_unsigned_64)2u *
                core_machine_8086_timing_odd_word(data) : 0u) +
                (segment_override ? CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS : 0u);
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
        machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_WAIT;
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

static C_INT core_machine_80286_system_source_instruction_cost(
    core_machine *machine, type_unsigned_64 *out_ticks);
static C_INT core_machine_control_stack_is_protected(
    const t_cpuins_data *data);

C_INT core_machine_primary_source_instruction_cost(
    core_machine *machine, type_unsigned_64 *out_ticks)
{
    const t_cpuins_data *data;
    core_machine_primary_timing_shape shape;
    type_unsigned_32 prefixes;
    type_unsigned_8 opcode;
    type_unsigned_64 ticks;
    type_unsigned_8 transfers;
    core_machine_source_transfer_plan transfer_plan;
    C_INT segment_override;
    C_INT lock_prefix;
    C_INT memory;

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
                !segment_override && !lock_prefix)))) {
        return 0;
    }
    opcode = data->opcodes[prefixes];
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088 && prefixes == 0u) {
        switch (opcode) {
        case 0x90u:
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_NOP;
            *out_ticks = 3u;
            return 1;
        case 0xf8u:
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_CLC;
            *out_ticks = 2u;
            return 1;
        case 0xfcu:
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_CLD;
            *out_ticks = 2u;
            return 1;
        case 0xfau:
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_CLI;
            *out_ticks = 2u;
            return 1;
        case 0xf5u:
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_CMC;
            *out_ticks = 2u;
            return 1;
        case 0xf9u:
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_STC;
            *out_ticks = 2u;
            return 1;
        case 0xfdu:
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_STD;
            *out_ticks = 2u;
            return 1;
        case 0xfbu:
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_STI;
            *out_ticks = 2u;
            return 1;
        case 0x9eu:
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_SAHF;
            *out_ticks = 4u;
            return 1;
        case 0x9fu:
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_LAHF;
            *out_ticks = 4u;
            return 1;
        default:
            break;
        }
    }
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 &&
        opcode == 0x0fu && !lock_prefix) {
        return core_machine_80286_system_source_instruction_cost(machine,
            out_ticks);
    }
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 && prefixes == 0u &&
        opcode >= 0x70u && opcode <= 0x7fu) {
        *out_ticks = machine->executor_cpu.data.eip ==
            TYPE_MASK_UNSIGNED_16(data->oldcpu.data.eip + 2u) ?
            CORE_MACHINE_80286_JCC_NOT_TAKEN_TICKS :
            CORE_MACHINE_80286_JCC_TAKEN_TICKS;
        return 1;
    }
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 && prefixes == 0u &&
        opcode == 0x63u && core_machine_control_stack_is_protected(data)) {
        *out_ticks = data->flagMem ? 11u : 10u;
        return 1;
    }
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 && prefixes == 0u &&
        opcode >= 0xa0u && opcode <= 0xa3u) {
        *out_ticks = (opcode == 0xa0u || opcode == 0xa1u) ? 5u : 3u;
        if ((opcode & 1u) != 0u) {
            *out_ticks += core_machine_80286_timing_odd_word(data);
        }
        return 1;
    }
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 && prefixes == 0u &&
        opcode >= 0xd8u && opcode <= 0xdfu) {
        *out_ticks = 1u;
        return 1;
    }
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 &&
        !data->flagLock && core_machine_80386_timing_has_source_prefixes(data,
            prefixes) && opcode == 0xd7u) {
        machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_XLAT;
        *out_ticks = 5u;
        return 1;
    }
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 &&
        !data->flagLock && prefixes == 0u && opcode == 0x9bu) {
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_WAIT);
        return 1;
    }
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 &&
        !data->flagLock && core_machine_80386_timing_has_source_prefixes(data,
            prefixes) && opcode == 0x62u &&
        core_machine_source_timing_modrm_is_memory(data, prefixes)) {
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_BOUND);
        return 1;
    }
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 &&
        !core_machine_control_stack_is_protected(data) && !data->flagLock &&
        core_machine_80386_timing_has_source_prefixes(data, prefixes) &&
        (opcode == 0xc4u || opcode == 0xc5u) &&
        core_machine_source_timing_modrm_is_memory(data, prefixes)) {
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_MOV_POINTER_REAL);
        return 1;
    }
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 &&
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
    memory = core_machine_source_timing_modrm_is_memory(data, prefixes);
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 && prefixes == 0u &&
        (opcode == 0xd0u || opcode == 0xd1u || opcode == 0xd2u ||
            opcode == 0xd3u || opcode == 0xc0u || opcode == 0xc1u) &&
        prefixes + 1u < data->oplen &&
        ((data->opcodes[prefixes + 1u] >> 3u) & 7u) != 6u) {
        if (opcode == 0xd0u || opcode == 0xd1u) {
            ticks = data->flagMem ? 7u + core_machine_80286_timing_effective_address(
                data, prefixes) : 2u;
        } else {
            ticks = core_machine_80286_group2_count(data, prefixes, opcode);
            ticks += data->flagMem ? 8u + core_machine_80286_timing_effective_address(
                data, prefixes) : 5u;
        }
        *out_ticks = ticks;
        return 1;
    }
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 &&
        (opcode == 0x8cu || opcode == 0x8eu) && prefixes + 1u < data->oplen &&
        ((data->opcodes[prefixes + 1u] >> 3u) & 7u) <= 3u &&
        (opcode != 0x8eu || ((data->opcodes[prefixes + 1u] >> 3u) & 7u) != 1u)) {
        if (opcode == 0x8cu) ticks = memory ? 3u : 2u;
        else if ((data->oldcpu.data.cr0 & VCPU_CR0_PE) != 0u &&
            (data->oldcpu.data.eflags & VCPU_EFLAGS_VM) == 0u) {
            ticks = memory ? 19u : 17u;
        } else ticks = memory ? 5u : 2u;
        if (memory) ticks += core_machine_80286_timing_effective_address(data,
            prefixes) + core_machine_80286_timing_odd_word(data);
        *out_ticks = ticks;
        return 1;
    }
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 &&
        (opcode == 0xc4u || opcode == 0xc5u) && data->flagMem) {
        *out_ticks = (core_machine_control_stack_is_protected(data) ? 21u : 7u) +
            core_machine_80286_timing_effective_address(data, prefixes) +
            2u * core_machine_80286_timing_odd_word(data);
        return 1;
    }
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088) {
        type_unsigned_8 extension = prefixes + 1u < data->oplen ?
            (data->opcodes[prefixes + 1u] >> 3u) & 7u : 8u;

        if (opcode == 0x8cu || opcode == 0x8eu) {
            if (extension > 3u || (opcode == 0x8eu && extension == 1u)) return 0;
            machine->source_timing_form_id = data->flagMem ?
                CORE_MACHINE_SOURCE_TIMING_MOV_SREG_MEMORY :
                CORE_MACHINE_SOURCE_TIMING_MOV_SREG_REGISTER;
            if (!data->flagMem) {
                if (segment_override) return 0;
                *out_ticks = 2u;
                return 1;
            }
            *out_ticks = (opcode == 0x8cu ? 9u : 8u) +
                core_machine_8086_timing_effective_address(data, prefixes) + 4u;
            if (segment_override) *out_ticks += CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS;
            return 1;
        }
        if ((opcode == 0xc4u || opcode == 0xc5u) && data->flagMem) {
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_LOAD_POINTER;
            *out_ticks = 16u + core_machine_8086_timing_effective_address(data,
                prefixes) + 8u;
            if (segment_override) *out_ticks += CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS;
            return 1;
        }
        if (opcode == 0xd7u) {
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_XLAT;
            *out_ticks = 15u;
            if (segment_override) *out_ticks += CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS;
            return 1;
        }
        if (opcode >= 0xd8u && opcode <= 0xdfu) {
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_ESC;
            if (!data->flagMem) {
                if (segment_override) return 0;
                *out_ticks = 2u;
                return 1;
            }
            *out_ticks = 12u + core_machine_8086_timing_effective_address(data,
                prefixes);
            if (segment_override) *out_ticks += CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS;
            return 1;
        }
        if ((opcode == 0xd0u || opcode == 0xd1u || opcode == 0xd2u ||
            opcode == 0xd3u) && extension != 6u) {
            type_unsigned_64 count = (opcode == 0xd0u || opcode == 0xd1u) ?
                1u : TYPE_MASK_UNSIGNED_8(data->oldcpu.data.cx);

            if (segment_override && !data->flagMem) return 0;
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_GROUP2;
            *out_ticks = data->flagMem ?
                ((opcode == 0xd0u || opcode == 0xd1u) ? 15u : 20u + 4u * count) +
                    core_machine_8086_timing_effective_address(data, prefixes) + 8u :
                ((opcode == 0xd0u || opcode == 0xd1u) ? 2u : 8u + 4u * count);
            if (segment_override) *out_ticks += CORE_MACHINE_8086_SEGMENT_OVERRIDE_TICKS;
            return 1;
        }
    }
    if (!core_machine_source_timing_primary_shape(data, prefixes, &shape)) {
        return 0;
    }
    transfer_plan = core_machine_source_timing_primary_transfer_plan(&shape);
    if (!transfer_plan.complete) return 0;
    transfers = transfer_plan.word_transfers;

    switch (machine->cpu_profile) {
    case CORE_MACHINE_CPU_PROFILE_8086:
    case CORE_MACHINE_CPU_PROFILE_8088:
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
            ticks += (type_unsigned_64)transfer_plan.word_transfers *
                (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088 ?
                CORE_MACHINE_8086_ODD_WORD_TICKS :
                core_machine_8086_timing_odd_word(data));
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
            ticks = (opcode == 0x3cu || opcode == 0x3du) ? 4u :
                (shape.memory ? 10u : (shape.word ? 4u : 3u));
            break;
        case CORE_MACHINE_SOURCE_TIMING_TEST_RM_REGISTER:
            ticks = shape.memory ? 10u : 3u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_TEST_ACCUMULATOR_IMMEDIATE:
            ticks = 4u;
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
            ticks = shape.memory ? 13u : 4u;
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
        case CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_REGISTER:
            ticks = 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_MOV_RM_REGISTER:
            ticks = shape.memory ? 3u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_MOV_REGISTER_RM:
            ticks = shape.memory ? 5u : 2u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_READ:
            ticks = 5u;
            break;
        case CORE_MACHINE_SOURCE_TIMING_MOV_MOFFS_WRITE:
            ticks = 3u;
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
    machine->source_timing_form_id = (type_unsigned_32)transfer_plan.form;
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
    case CORE_MACHINE_CPU_PROFILE_8088:
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
        !core_machine_timing_add_ticks(&base_ticks, next_ticks)) {
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
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8086 ||
        machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088) {
        ticks = core_machine_8086_timing_effective_address(data, prefixes) +
            (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8086 ?
                (type_unsigned_64)word_transfers *
                    core_machine_8086_timing_odd_word(data) : 0u);
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

/* Table 2-21 assigns 8088 four clocks for every listed word transfer.  This
 * plan is consumed immediately by the retained evaluator; it is not CPU or
 * bus state and deliberately names only rows whose transfer count is explicit.
 */
static core_machine_source_transfer_plan
    core_machine_8088_control_stack_transfer_plan(
        core_machine_source_timing_form form)
{
    core_machine_source_transfer_plan plan = { form, 0u, TYPE_TRUE };

    switch (form) {
    case CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_DIRECT:
    case CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_REGISTER:
    case CORE_MACHINE_SOURCE_TIMING_JMP_MEMORY:
    case CORE_MACHINE_SOURCE_TIMING_RET_NEAR:
    case CORE_MACHINE_SOURCE_TIMING_RET_NEAR_IMMEDIATE:
    case CORE_MACHINE_SOURCE_TIMING_PUSH_REGISTER:
    case CORE_MACHINE_SOURCE_TIMING_POP_REGISTER:
    case CORE_MACHINE_SOURCE_TIMING_PUSHF:
    case CORE_MACHINE_SOURCE_TIMING_POPF:
        plan.word_transfers = 1u;
        break;
    case CORE_MACHINE_SOURCE_TIMING_HLT:
        break;
    case CORE_MACHINE_SOURCE_TIMING_INT3:
    case CORE_MACHINE_SOURCE_TIMING_INT_IMMEDIATE:
    case CORE_MACHINE_SOURCE_TIMING_INTO:
        plan.word_transfers = 5u;
        break;
    case CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_MEMORY:
    case CORE_MACHINE_SOURCE_TIMING_CALL_FAR_DIRECT:
    case CORE_MACHINE_SOURCE_TIMING_JMP_FAR_MEMORY:
    case CORE_MACHINE_SOURCE_TIMING_RET_FAR:
    case CORE_MACHINE_SOURCE_TIMING_RET_FAR_IMMEDIATE:
    case CORE_MACHINE_SOURCE_TIMING_PUSH_MEMORY:
    case CORE_MACHINE_SOURCE_TIMING_POP_MEMORY:
        plan.word_transfers = 2u;
        break;
    case CORE_MACHINE_SOURCE_TIMING_IRET:
        plan.word_transfers = 3u;
        break;
    case CORE_MACHINE_SOURCE_TIMING_CALL_FAR_MEMORY:
        plan.word_transfers = 4u;
        break;
    case CORE_MACHINE_SOURCE_TIMING_JMP_DIRECT:
    case CORE_MACHINE_SOURCE_TIMING_JMP_REGISTER:
    case CORE_MACHINE_SOURCE_TIMING_JMP_FAR_DIRECT:
        break;
    default:
        plan.complete = TYPE_FALSE;
        break;
    }
    return plan;
}

static C_INT core_machine_control_stack_source_result(core_machine *machine,
    core_machine_source_timing_form form, type_unsigned_64 additions,
    C_INT include_next_term, type_unsigned_64 *out_ticks)
{
    core_machine_source_transfer_plan transfer_plan;
    type_unsigned_64 ticks;

    if (machine == STD_NULL || out_ticks == STD_NULL) return 0;
    ticks = core_machine_control_stack_source_lookup(machine, form);
    if (ticks == CORE_MACHINE_SOURCE_UNALLOCATED_TICKS ||
        !core_machine_timing_add_ticks(&ticks, additions)) return 0;
    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088) {
        transfer_plan = core_machine_8088_control_stack_transfer_plan(form);
        if (!transfer_plan.complete || !core_machine_timing_add_ticks(&ticks,
                (type_unsigned_64)transfer_plan.word_transfers *
                    CORE_MACHINE_8086_ODD_WORD_TICKS)) return 0;
    }
    return include_next_term ? core_machine_control_stack_add_next_term(machine,
        ticks, out_ticks) : ((*out_ticks = ticks), 1);
}

static C_INT core_machine_control_stack_direct_target_is_task_gate(
    const t_cpuins_data *data, type_unsigned_32 prefixes, const t_cpu *cpu)
{
    type_unsigned_16 selector;
    type_unsigned_32 selector_index;
    C_INT operand32;

    if (data == STD_NULL || cpu == STD_NULL || prefixes >= data->oplen) {
        return 0;
    }
    operand32 = data->oldcpu.data.cs.seg.exec.defsize != data->prefix_oprsize;
    selector_index = prefixes + (operand32 ? 5u : 3u);
    if (selector_index + 1u >= data->oplen) return 0;
    selector = (type_unsigned_16)data->opcodes[selector_index] |
        (type_unsigned_16)((type_unsigned_16)data->opcodes[selector_index + 1u] << 8u);
    return selector != cpu->data.tr.selector;
}

static C_INT core_machine_control_stack_direct_target_is_gate(
    const t_cpuins_data *data, type_unsigned_32 prefixes, const t_cpu *cpu)
{
    type_unsigned_16 selector;
    type_unsigned_32 selector_index;
    C_INT operand32;

    if (data == STD_NULL || cpu == STD_NULL || prefixes >= data->oplen) {
        return 0;
    }
    operand32 = data->oldcpu.data.cs.seg.exec.defsize != data->prefix_oprsize;
    selector_index = prefixes + (operand32 ? 5u : 3u);
    if (selector_index + 1u >= data->oplen) return 0;
    selector = (type_unsigned_16)data->opcodes[selector_index] |
        (type_unsigned_16)((type_unsigned_16)data->opcodes[selector_index + 1u] << 8u);
    return TYPE_MASK_UNSIGNED_16(selector & VCPU_SELECTOR_IDX) !=
        TYPE_MASK_UNSIGNED_16(cpu->data.cs.selector & VCPU_SELECTOR_IDX);
}

static C_INT core_machine_control_stack_direct_call_gate_parameters(
    core_machine *machine, const t_cpuins_data *data, type_unsigned_32 prefixes,
    type_unsigned_8 *out_parameters)
{
    type_unsigned_16 selector;
    type_unsigned_32 table_base;
    type_unsigned_32 selector_index;
    C_INT operand32;

    if (machine == STD_NULL || data == STD_NULL || out_parameters == STD_NULL ||
        prefixes >= data->oplen) return 0;
    operand32 = data->oldcpu.data.cs.seg.exec.defsize != data->prefix_oprsize;
    selector_index = prefixes + (operand32 ? 5u : 3u);
    if (selector_index + 1u >= data->oplen) return 0;
    selector = (type_unsigned_16)data->opcodes[selector_index] |
        (type_unsigned_16)((type_unsigned_16)data->opcodes[selector_index + 1u] << 8u);
    table_base = (selector & VCPU_SELECTOR_TI) != 0u ?
        data->oldcpu.data.ldtr.base : data->oldcpu.data.gdtr.base;
    return core_machine_memory_read_physical(&machine->executor_memory,
        table_base + (selector & VCPU_SELECTOR_IDX) + 4u,
        (type_virtual_address)out_parameters, 1u) == TYPE_STATUS_OK;
}

static C_INT core_machine_control_stack_call_gate_parameters(core_machine *machine,
    type_unsigned_16 selector, type_unsigned_8 *out_parameters)
{
    type_unsigned_32 table_base;

    if (machine == STD_NULL || out_parameters == STD_NULL) return 0;
    table_base = (selector & VCPU_SELECTOR_TI) != 0u ?
        machine->executor_cpu.data.ldtr.base : machine->executor_cpu.data.gdtr.base;
    return core_machine_memory_read_physical(&machine->executor_memory,
        table_base + (selector & VCPU_SELECTOR_IDX) + 4u,
        (type_virtual_address)out_parameters, 1u) == TYPE_STATUS_OK;
}

static C_INT core_machine_control_stack_selector_is_task_gate(core_machine *machine,
    type_unsigned_16 selector)
{
    type_unsigned_32 table_base;
    type_unsigned_8 access;

    if (machine == STD_NULL) return 0;
    table_base = (selector & VCPU_SELECTOR_TI) != 0u ?
        machine->executor_cpu.data.ldtr.base : machine->executor_cpu.data.gdtr.base;
    if (core_machine_memory_read_physical(&machine->executor_memory,
            table_base + (selector & VCPU_SELECTOR_IDX) + 5u,
            (type_virtual_address)&access, 1u) != TYPE_STATUS_OK) return 0;
    return (access & 0x0fu) == VCPU_DESC_SYS_TYPE_TASKGATE;
}

static C_INT core_machine_control_stack_short_branch_taken(
    const t_cpuins_data *data, const core_machine *machine,
    type_unsigned_32 prefixes, C_INT *out_taken)
{
    if (data == STD_NULL || machine == STD_NULL || out_taken == STD_NULL) return 0;
    *out_taken = machine->executor_cpu.data.eip !=
        TYPE_MASK_UNSIGNED_16(data->oldcpu.data.eip + prefixes + 2u);
    return 1;
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
    C_INT operand32;
    C_INT protected_execution;
    C_INT same_privilege;
    C_INT task_switch;
    C_INT direct_gate;

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
    operand32 = data->oldcpu.data.cs.seg.exec.defsize;
    if (data->prefix_oprsize) operand32 = !operand32;
    protected_execution = (data->oldcpu.data.cr0 & VCPU_CR0_PE) != 0u;
    same_privilege = !protected_execution ||
        ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) == 0u &&
        data->oldcpu.data.cs.dpl == machine->executor_cpu.data.cs.dpl);
    /* A successful 80286 task transfer replaces TR.  Select this published
     * outcome before the code-segment and privilege paths: task switches can
     * otherwise look like ordinary same-level far transfers after refresh. */
    task_switch = (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ||
        machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) &&
        protected_execution && data->oldcpu.data.tr.selector !=
        machine->executor_cpu.data.tr.selector;
    direct_gate = (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ||
        machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) &&
        protected_mode && core_machine_control_stack_direct_target_is_gate(data,
            prefixes, &machine->executor_cpu);
    memory = core_machine_source_timing_modrm_is_memory(data, prefixes);
    extension = prefixes + 1u < data->oplen ?
        (data->opcodes[prefixes + 1u] >> 3u) & 7u : 8u;

    if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088 &&
        opcode >= 0x70u && opcode <= 0x7fu) {
        C_INT taken;

        if (!core_machine_control_stack_short_branch_taken(data, machine,
                prefixes, &taken)) return 0;
        machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_JCC;
        *out_ticks = taken ? 16u : 4u;
        return 1;
    }

    switch (opcode) {
    case 0x06u: case 0x0eu: case 0x16u: case 0x1eu:
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088) {
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_PUSH_REGISTER;
            *out_ticks = 14u;
            return 1;
        }
        if (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80286 &&
            machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80386) return 0;
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_PUSH_REGISTER);
        return 1;
    case 0x07u: case 0x0fu: case 0x17u: case 0x1fu:
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088) {
            return core_machine_control_stack_source_result(machine,
                CORE_MACHINE_SOURCE_TIMING_POP_REGISTER, 0u, 0, out_ticks);
        }
        if (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80286 &&
            machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80386) {
            return 0;
        }
        /* Appendix B distinguishes the protected selector-validation path
         * from the real-mode stack transfer. */
        *out_ticks = protected_mode ? 20u :
            core_machine_control_stack_source_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_POP_REGISTER);
        return 1;
    case 0xe8u:
        return core_machine_control_stack_source_result(machine,
            CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_DIRECT, 0u, 1, out_ticks);
    case 0x9au:
        if (task_switch) {
            if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) {
                (C_VOID)core_machine_control_stack_source_lookup(machine,
                    CORE_MACHINE_SOURCE_TIMING_CALL_FAR_DIRECT);
                *out_ticks = core_machine_control_stack_direct_target_is_task_gate(
                    data, prefixes, &machine->executor_cpu) ? 401u : 392u;
                return core_machine_control_stack_add_next_term(machine,
                    *out_ticks, out_ticks);
            }
            *out_ticks = core_machine_control_stack_direct_target_is_task_gate(
                data, prefixes, &machine->executor_cpu) ? 182u : 177u;
            return core_machine_control_stack_add_next_term(machine,
                *out_ticks, out_ticks);
        }
        if (direct_gate) {
            type_unsigned_8 parameters = 0u;

            if (!same_privilege &&
                !core_machine_control_stack_direct_call_gate_parameters(machine,
                    data, prefixes, &parameters)) return 0;
            if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) {
                (C_VOID)core_machine_control_stack_source_lookup(machine,
                    CORE_MACHINE_SOURCE_TIMING_CALL_FAR_DIRECT);
                *out_ticks = same_privilege ? 52u :
                    (parameters == 0u ? 86u : 94u + 4u * parameters);
                return core_machine_control_stack_add_next_term(machine,
                    *out_ticks, out_ticks);
            }
            *out_ticks = same_privilege ? 41u : 82u + 4u * parameters;
            return core_machine_control_stack_add_next_term(machine,
                *out_ticks, out_ticks);
        }
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 &&
            protected_mode && same_privilege) {
            *out_ticks = 26u;
            return core_machine_control_stack_add_next_term(machine,
                *out_ticks, out_ticks);
        }
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 &&
            protected_mode && same_privilege) {
            (C_VOID)core_machine_control_stack_source_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_CALL_FAR_DIRECT);
            return core_machine_control_stack_add_next_term(machine, 34u,
                out_ticks);
        }
        if (!same_privilege || protected_mode) return 0;
        return core_machine_control_stack_source_result(machine,
            CORE_MACHINE_SOURCE_TIMING_CALL_FAR_DIRECT, 0u, 1, out_ticks);
    case 0xe9u: case 0xebu:
        return core_machine_control_stack_source_result(machine,
            CORE_MACHINE_SOURCE_TIMING_JMP_DIRECT, 0u, 1, out_ticks);
    case 0xeau:
        if (task_switch) {
            if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) {
                (C_VOID)core_machine_control_stack_source_lookup(machine,
                    CORE_MACHINE_SOURCE_TIMING_JMP_FAR_DIRECT);
                *out_ticks = core_machine_control_stack_direct_target_is_task_gate(
                    data, prefixes, &machine->executor_cpu) ? 401u : 392u;
                return core_machine_control_stack_add_next_term(machine,
                    *out_ticks, out_ticks);
            }
            *out_ticks = core_machine_control_stack_direct_target_is_task_gate(
                data, prefixes, &machine->executor_cpu) ? 180u : 175u;
            return core_machine_control_stack_add_next_term(machine,
                *out_ticks, out_ticks);
        }
        if (direct_gate && same_privilege) {
            if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) {
                (C_VOID)core_machine_control_stack_source_lookup(machine,
                    CORE_MACHINE_SOURCE_TIMING_JMP_FAR_DIRECT);
                return core_machine_control_stack_add_next_term(machine, 45u,
                    out_ticks);
            }
            *out_ticks = 38u;
            return core_machine_control_stack_add_next_term(machine,
                *out_ticks, out_ticks);
        }
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 &&
            protected_mode && same_privilege) {
            *out_ticks = 23u;
            return core_machine_control_stack_add_next_term(machine,
                *out_ticks, out_ticks);
        }
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 &&
            protected_mode && same_privilege) {
            (C_VOID)core_machine_control_stack_source_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_JMP_FAR_DIRECT);
            return core_machine_control_stack_add_next_term(machine, 27u,
                out_ticks);
        }
        if (!same_privilege || protected_mode) return 0;
        return core_machine_control_stack_source_result(machine,
            CORE_MACHINE_SOURCE_TIMING_JMP_FAR_DIRECT, 0u, 1, out_ticks);
    case 0xc2u:
        return core_machine_control_stack_source_result(machine,
            CORE_MACHINE_SOURCE_TIMING_RET_NEAR_IMMEDIATE, 0u, 1, out_ticks);
    case 0xc3u:
        return core_machine_control_stack_source_result(machine,
            CORE_MACHINE_SOURCE_TIMING_RET_NEAR, 0u, 1, out_ticks);
    case 0xcau:
        if (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8088 &&
            machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186 &&
            machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80286 &&
            machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80386) return 0;
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 &&
            protected_mode) {
            *out_ticks = same_privilege ? 25u : 55u;
            return core_machine_control_stack_add_next_term(machine,
                *out_ticks, out_ticks);
        }
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 &&
            protected_mode) {
            (C_VOID)core_machine_control_stack_source_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_RET_FAR_IMMEDIATE);
            if (!same_privilege) {
                *out_ticks = 69u;
                return 1;
            }
            return core_machine_control_stack_add_next_term(machine, 32u,
                out_ticks);
        }
        return core_machine_control_stack_source_result(machine,
            CORE_MACHINE_SOURCE_TIMING_RET_FAR_IMMEDIATE, 0u, 1, out_ticks);
    case 0xcbu:
        if (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_8088 &&
            machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186 &&
            machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80286 &&
            machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80386) return 0;
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 &&
            protected_mode) {
            *out_ticks = same_privilege ? 25u : 55u;
            return core_machine_control_stack_add_next_term(machine,
                *out_ticks, out_ticks);
        }
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 &&
            protected_mode) {
            (C_VOID)core_machine_control_stack_source_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_RET_FAR);
            if (!same_privilege) {
                *out_ticks = 69u;
                return 1;
            }
            return core_machine_control_stack_add_next_term(machine, 32u,
                out_ticks);
        }
        return core_machine_control_stack_source_result(machine,
            CORE_MACHINE_SOURCE_TIMING_RET_FAR, 0u, 1, out_ticks);
    case 0x50u: case 0x51u: case 0x52u: case 0x53u:
    case 0x54u: case 0x55u: case 0x56u: case 0x57u:
        return core_machine_control_stack_source_result(machine,
            CORE_MACHINE_SOURCE_TIMING_PUSH_REGISTER, 0u, 0, out_ticks);
    case 0x58u: case 0x59u: case 0x5au: case 0x5bu:
    case 0x5cu: case 0x5du: case 0x5eu: case 0x5fu:
        return core_machine_control_stack_source_result(machine,
            CORE_MACHINE_SOURCE_TIMING_POP_REGISTER, 0u, 0, out_ticks);
    case 0x60u:
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_PUSHA);
        return machine->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186;
    case 0x61u:
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_POPA);
        return machine->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186;
    case 0x62u:
        if (!memory) return 0;
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286) {
            *out_ticks = 13u;
            return 1;
        }
        if (machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80186) return 0;
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_BOUND);
        return 1;
    case 0x68u: case 0x6au:
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_PUSH_IMMEDIATE);
        return machine->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186;
    case 0x8fu:
        if (extension != 0u) return 0;
        return core_machine_control_stack_source_result(machine, memory ?
            CORE_MACHINE_SOURCE_TIMING_POP_MEMORY :
            CORE_MACHINE_SOURCE_TIMING_POP_REGISTER, memory ?
            core_machine_control_stack_memory_additions(machine, data, prefixes,
                1u) : 0u, 0, out_ticks);
    case 0x9cu:
        return core_machine_control_stack_source_result(machine,
            CORE_MACHINE_SOURCE_TIMING_PUSHF, 0u, 0, out_ticks);
    case 0x9du:
        return core_machine_control_stack_source_result(machine,
            CORE_MACHINE_SOURCE_TIMING_POPF, 0u, 0, out_ticks);
    case 0xc8u:
        if (machine->cpu_profile < CORE_MACHINE_CPU_PROFILE_80186) return 0;
        if (prefixes + 3u >= data->oplen) return 0;
        extension = data->opcodes[prefixes + 3u] & 0x1fu;
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
            /* ENTER's level-N row is a documented formula derived from the
             * level-one family; retain its concrete form identity rather than
             * publishing a classified-but-unattributed retirement. */
            machine->source_timing_form_id =
                CORE_MACHINE_SOURCE_TIMING_ENTER_LEVEL_ONE;
            *out_ticks = ticks;
        }
        return 1;
    case 0xc9u:
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_LEAVE);
        return machine->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80186;
    case 0xe0u: case 0xe1u: case 0xe2u: case 0xe3u: {
        C_INT taken;

        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8086) {
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_LOOP;
        }
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) {
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_JCC;
            ticks = opcode == 0xe3u ?
                (machine->executor_cpu.data.eip == data->oldcpu.data.eip +
                    prefixes + 2u ? 5u : 9u) : 11u;
            return core_machine_control_stack_add_next_term(machine, ticks,
                out_ticks);
        }
        if (!core_machine_control_stack_short_branch_taken(data, machine,
                prefixes, &taken)) return 0;
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088) {
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_JCC;
        }
        if (opcode == 0xe3u) {
            *out_ticks = !taken ?
                (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ? 4u :
                    machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186 ? 5u :
                    6u) : (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ?
                    8u : machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186 ?
                    15u : 18u);
        } else {
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
                    CORE_MACHINE_CPU_PROFILE_80286 ? 4u :
                    machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80186 ? 6u :
                    5u);
            }
        }
        return 1;
    }
    case 0xf4u:
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088) {
            return core_machine_control_stack_source_result(machine,
                CORE_MACHINE_SOURCE_TIMING_HLT, 0u, 0, out_ticks);
        }
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_HLT);
        return 1;
    case 0xccu: case 0xcdu:
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088) {
            return core_machine_control_stack_source_result(machine,
                opcode == 0xccu || (prefixes + 1u < data->oplen &&
                data->opcodes[prefixes + 1u] == 3u) ?
                CORE_MACHINE_SOURCE_TIMING_INT3 :
                CORE_MACHINE_SOURCE_TIMING_INT_IMMEDIATE, 0u, 0, out_ticks);
        }
        if (task_switch) {
            if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) {
                (C_VOID)core_machine_control_stack_source_lookup(machine,
                    opcode == 0xccu ? CORE_MACHINE_SOURCE_TIMING_INT3 :
                    CORE_MACHINE_SOURCE_TIMING_INT_IMMEDIATE);
                *out_ticks = 309u;
                return 1;
            }
            *out_ticks = 167u;
            return core_machine_control_stack_add_next_term(machine,
                *out_ticks, out_ticks);
        }
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 &&
            protected_mode) {
            *out_ticks = same_privilege ? 40u : 78u;
            return core_machine_control_stack_add_next_term(machine,
                *out_ticks, out_ticks);
        }
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 &&
            protected_execution) {
            (C_VOID)core_machine_control_stack_source_lookup(machine,
                opcode == 0xccu ? CORE_MACHINE_SOURCE_TIMING_INT3 :
                CORE_MACHINE_SOURCE_TIMING_INT_IMMEDIATE);
            return core_machine_control_stack_add_next_term(machine,
                same_privilege ? 59u :
                ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ? 119u : 99u),
                out_ticks);
        }
        if (!same_privilege) return 0;
        if (protected_mode) return 0;
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            opcode == 0xccu ? CORE_MACHINE_SOURCE_TIMING_INT3 :
            CORE_MACHINE_SOURCE_TIMING_INT_IMMEDIATE);
        return machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ?
            core_machine_control_stack_add_next_term(machine, *out_ticks,
                out_ticks) : 1;
    case 0xceu:
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088) {
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_INTO;
            if ((data->oldcpu.data.eflags & VCPU_EFLAGS_OF) == 0u) {
                *out_ticks = 4u;
                return 1;
            }
            return core_machine_control_stack_source_result(machine,
                CORE_MACHINE_SOURCE_TIMING_INTO, 0u, 0, out_ticks);
        }
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) {
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_INTO;
        }
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8086) {
            machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_8086_INTO;
        }
        if ((data->oldcpu.data.eflags & VCPU_EFLAGS_OF) == 0u) {
            *out_ticks = machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ||
                machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 ? 3u : 4u;
            return machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ?
                core_machine_control_stack_add_next_term(machine, *out_ticks,
                    out_ticks) : 1;
        }
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 &&
            protected_execution) {
            if (task_switch) {
                *out_ticks = 309u;
                return 1;
            }
            return core_machine_control_stack_add_next_term(machine,
                same_privilege ? 59u :
                ((data->oldcpu.data.eflags & VCPU_EFLAGS_VM) != 0u ? 119u : 99u),
                out_ticks);
        }
        if (!same_privilege || protected_mode) return 0;
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
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088) {
            return core_machine_control_stack_source_result(machine,
                CORE_MACHINE_SOURCE_TIMING_IRET, 0u, 0, out_ticks);
        }
        if (task_switch) {
            if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) {
                (C_VOID)core_machine_control_stack_source_lookup(machine,
                    CORE_MACHINE_SOURCE_TIMING_IRET);
                *out_ticks = 275u;
                return 1;
            }
            *out_ticks = 169u;
            return core_machine_control_stack_add_next_term(machine,
                *out_ticks, out_ticks);
        }
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 &&
            protected_mode) {
            *out_ticks = same_privilege ? 31u : 55u;
            return core_machine_control_stack_add_next_term(machine,
                *out_ticks, out_ticks);
        }
        if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 &&
            protected_mode) {
            (C_VOID)core_machine_control_stack_source_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_IRET);
            if ((machine->executor_cpu.data.eflags & VCPU_EFLAGS_VM) != 0u) {
                *out_ticks = 60u;
                return 1;
            }
            if (!same_privilege) {
                *out_ticks = 82u;
                return 1;
            }
            return core_machine_control_stack_add_next_term(machine, 38u,
                out_ticks);
        }
        if (!same_privilege || protected_mode) return 0;
        *out_ticks = core_machine_control_stack_source_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_IRET);
        return machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ?
            core_machine_control_stack_add_next_term(machine, *out_ticks,
                out_ticks) : 1;
    case 0xffu:
        if (extension == 2u || extension == 4u) {
            return core_machine_control_stack_source_result(machine,
                memory ? (extension == 2u ?
                    CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_MEMORY :
                    CORE_MACHINE_SOURCE_TIMING_JMP_MEMORY) :
                (extension == 2u ?
                    CORE_MACHINE_SOURCE_TIMING_CALL_NEAR_REGISTER :
                    CORE_MACHINE_SOURCE_TIMING_JMP_REGISTER), memory ?
                core_machine_control_stack_memory_additions(machine, data, prefixes,
                    1u) : 0u, 1, out_ticks);
        }
        if (extension == 3u || extension == 5u) {
            if (task_switch) {
                if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) {
                    (C_VOID)core_machine_control_stack_source_lookup(machine,
                        extension == 3u ? CORE_MACHINE_SOURCE_TIMING_CALL_FAR_MEMORY :
                        CORE_MACHINE_SOURCE_TIMING_JMP_FAR_MEMORY_PROTECTED);
                    *out_ticks = 397u + (core_machine_control_stack_selector_is_task_gate(
                        machine, data->crm) ? 9u : 0u);
                    return core_machine_control_stack_add_next_term(machine,
                        *out_ticks, out_ticks);
                }
                *out_ticks = extension == 3u ? 180u : 178u;
                *out_ticks += core_machine_control_stack_memory_additions(
                    machine, data, prefixes, 2u);
                return core_machine_control_stack_add_next_term(machine,
                    *out_ticks, out_ticks);
            }
            if (!memory || (!same_privilege && !(machine->cpu_profile ==
                CORE_MACHINE_CPU_PROFILE_80386 && protected_mode && extension == 3u &&
                data->crm != machine->executor_cpu.data.cs.selector)) ||
                (protected_mode && extension == 5u &&
                data->crm != machine->executor_cpu.data.cs.selector &&
                machine->cpu_profile != CORE_MACHINE_CPU_PROFILE_80386)) return 0;
            if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 &&
                protected_mode && data->crm != machine->executor_cpu.data.cs.selector) {
                type_unsigned_8 parameters = 0u;

                if (extension == 3u && !same_privilege &&
                    !core_machine_control_stack_call_gate_parameters(machine,
                        data->crm, &parameters)) return 0;
                (C_VOID)core_machine_control_stack_source_lookup(machine,
                    extension == 3u ? CORE_MACHINE_SOURCE_TIMING_CALL_FAR_MEMORY :
                    CORE_MACHINE_SOURCE_TIMING_JMP_FAR_MEMORY_PROTECTED);
                *out_ticks = extension == 3u ? (same_privilege ? 56u :
                    (parameters == 0u ? 90u : 98u + 4u * parameters)) : 49u;
                return core_machine_control_stack_add_next_term(machine,
                    *out_ticks, out_ticks);
            }
            if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 &&
                protected_mode) {
                *out_ticks = extension == 3u ? 29u : 26u;
            } else if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386 &&
                protected_mode) {
                (C_VOID)core_machine_control_stack_source_lookup(machine,
                    extension == 3u ? CORE_MACHINE_SOURCE_TIMING_CALL_FAR_MEMORY :
                    CORE_MACHINE_SOURCE_TIMING_JMP_FAR_MEMORY_PROTECTED);
                *out_ticks = extension == 3u ? 38u : 31u;
            } else {
                *out_ticks = core_machine_control_stack_source_lookup(machine,
                    extension == 3u ? CORE_MACHINE_SOURCE_TIMING_CALL_FAR_MEMORY :
                    protected_mode ? CORE_MACHINE_SOURCE_TIMING_JMP_FAR_MEMORY_PROTECTED :
                    CORE_MACHINE_SOURCE_TIMING_JMP_FAR_MEMORY);
            }
            if (machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_8088) {
                return core_machine_control_stack_source_result(machine,
                    extension == 3u ? CORE_MACHINE_SOURCE_TIMING_CALL_FAR_MEMORY :
                    CORE_MACHINE_SOURCE_TIMING_JMP_FAR_MEMORY,
                    core_machine_control_stack_memory_additions(machine, data,
                        prefixes, 2u), 1, out_ticks);
            }
            *out_ticks += core_machine_control_stack_memory_additions(machine,
                data, prefixes, 2u);
            return core_machine_control_stack_add_next_term(machine, *out_ticks,
                out_ticks);
        }
        if (extension == 6u) {
            return core_machine_control_stack_source_result(machine, memory ?
                CORE_MACHINE_SOURCE_TIMING_PUSH_MEMORY :
                CORE_MACHINE_SOURCE_TIMING_PUSH_REGISTER, memory ?
                core_machine_control_stack_memory_additions(machine, data, prefixes,
                    1u) : 0u, 0, out_ticks);
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
    core_machine_source_timing_form form;
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
        form = signed_multiplier ? CORE_MACHINE_SOURCE_TIMING_DYNAMIC_IMUL :
            CORE_MACHINE_SOURCE_TIMING_DYNAMIC_MUL;
    } else if (opcode == 0x69u) {
        multiplier = data->crm;
        signed_multiplier = TYPE_TRUE;
        memory_multiplier = core_machine_source_timing_modrm_is_memory(data,
            prefixes);
        form = CORE_MACHINE_SOURCE_TIMING_DYNAMIC_IMUL_IMMEDIATE;
    } else if (opcode == 0x6bu) {
        multiplier = data->cimm;
        operand_bytes = 1u;
        signed_multiplier = TYPE_TRUE;
        form = CORE_MACHINE_SOURCE_TIMING_DYNAMIC_IMUL_IMMEDIATE;
    } else if (opcode == 0x0fu && prefixes + 2u < data->oplen &&
        data->opcodes[prefixes + 1u] == 0xafu) {
        multiplier = data->crm;
        signed_multiplier = TYPE_TRUE;
        memory_multiplier = core_machine_source_timing_modrm_is_memory(data,
            prefixes + 1u);
        form = CORE_MACHINE_SOURCE_TIMING_DYNAMIC_IMUL_TWO_OPERAND;
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
    machine->source_timing_form_id = (type_unsigned_32)form;
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
        machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_JCC;
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
        machine->source_timing_form_id =
            CORE_MACHINE_SOURCE_TIMING_DYNAMIC_BT;
        return 1;
    case 0xabu: case 0xb3u: case 0xbbu:
        *out_ticks = memory ? 13u : 6u;
        machine->source_timing_form_id = (type_unsigned_32)(secondary == 0xabu ?
            CORE_MACHINE_SOURCE_TIMING_DYNAMIC_BTS : secondary == 0xb3u ?
            CORE_MACHINE_SOURCE_TIMING_DYNAMIC_BTR :
            CORE_MACHINE_SOURCE_TIMING_DYNAMIC_BTC);
        return 1;
    case 0xbau:
        if (extension == 4u) {
            *out_ticks = memory ? 6u : 3u;
            machine->source_timing_form_id =
                CORE_MACHINE_SOURCE_TIMING_DYNAMIC_BT;
            return 1;
        }
        if (extension >= 5u) {
            *out_ticks = memory ? 8u : 6u;
            machine->source_timing_form_id = (type_unsigned_32)(extension == 5u ?
                CORE_MACHINE_SOURCE_TIMING_DYNAMIC_BTS : extension == 6u ?
                CORE_MACHINE_SOURCE_TIMING_DYNAMIC_BTR :
                CORE_MACHINE_SOURCE_TIMING_DYNAMIC_BTC);
            return 1;
        }
        return 0;
    case 0xa4u: case 0xa5u: case 0xacu: case 0xadu:
        *out_ticks = memory ? 7u : 3u;
        machine->source_timing_form_id = (type_unsigned_32)(
            secondary == 0xa4u || secondary == 0xa5u ?
            CORE_MACHINE_SOURCE_TIMING_DYNAMIC_SHLD :
            CORE_MACHINE_SOURCE_TIMING_DYNAMIC_SHRD);
        return 1;
    case 0xb6u: case 0xb7u: case 0xbeu: case 0xbfu:
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            memory ? CORE_MACHINE_SOURCE_TIMING_MOV_EXTEND_MEMORY :
            CORE_MACHINE_SOURCE_TIMING_MOV_EXTEND_REGISTER);
        return 1;
    case 0xbcu: case 0xbdu:
        *out_ticks = (secondary == 0xbcu ? 11u : 9u) + 3u *
            core_machine_80386_timing_zero_scan_count(data->crm, operand_bytes,
                secondary == 0xbdu);
        machine->source_timing_form_id = (type_unsigned_32)(secondary == 0xbcu ?
            CORE_MACHINE_SOURCE_TIMING_DYNAMIC_BSF :
            CORE_MACHINE_SOURCE_TIMING_DYNAMIC_BSR);
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

    C_INT operand32;

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
    operand32 = data->oldcpu.data.cs.seg.exec.defsize;
    if (data->prefix_oprsize) operand32 = !operand32;

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
        *out_ticks = 6u;
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
            *out_ticks = core_machine_80386_source_timing_lookup(machine,
                CORE_MACHINE_SOURCE_TIMING_MOV_POINTER_REAL);
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

static C_INT core_machine_80286_system_source_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks)
{
    const t_cpuins_data *data = &machine->executor_cpu_instructions.data;
    type_unsigned_32 prefixes = core_machine_instruction_prefix_count(data);
    type_unsigned_64 memory_ea = data->flagMem ?
        core_machine_80286_timing_effective_address(data, prefixes + 1u) : 0u;
    type_unsigned_8 opcode;

    if (out_ticks == STD_NULL) return 0;
    if (prefixes >= data->oplen) {
        machine->source_repeat_active = TYPE_FALSE;
        *out_ticks = 0u;
        return 1;
    }
    opcode = data->opcodes[prefixes];
    machine->source_repeat_active = TYPE_FALSE;
    if (opcode != 0x0fu) return 0;
    switch (opcode) {
    case 0x0fu:
        if (prefixes + 2u < data->oplen &&
            data->opcodes[prefixes + 1u] == 0x00u &&
            (((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 2u ||
             ((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 3u) &&
            core_machine_control_stack_is_protected(data) &&
            data->oldcpu.data.cs.dpl == 0u) {
            *out_ticks = (data->flagMem ? 19u : 17u) + memory_ea;
            return 1;
        }
        if (prefixes + 2u < data->oplen &&
            data->opcodes[prefixes + 1u] == 0x01u && data->flagMem &&
            (((data->opcodes[prefixes + 2u] >> 3u) & 7u) <= 3u) &&
            (!core_machine_control_stack_is_protected(data) ||
             data->oldcpu.data.cs.dpl == 0u)) {
            *out_ticks = ((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 0u ||
                ((data->opcodes[prefixes + 2u] >> 3u) & 7u) == 2u ? 11u : 12u;
            *out_ticks += memory_ea;
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
            *out_ticks = (data->flagMem ? 16u : 14u) + memory_ea;
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
            *out_ticks += memory_ea;
            return 1;
        }
        core_machine_source_timing_mark_unallocated(machine, out_ticks);
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

C_INT core_machine_80286_source_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks)
{
    const t_cpuins_data *data;

    if (machine == STD_NULL || out_ticks == STD_NULL) return 0;
    data = &machine->executor_cpu_instructions.data;
    machine->source_repeat_active = TYPE_FALSE;
    if (core_machine_instruction_prefix_count(data) >= data->oplen) {
        *out_ticks = 0u;
        return 1;
    }
    core_machine_source_timing_mark_unallocated(machine, out_ticks);
    return 1;
}

C_INT core_machine_80386_source_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks)
{
    const t_cpuins_data *data = &machine->executor_cpu_instructions.data;
    type_unsigned_32 prefixes = core_machine_instruction_prefix_count(data);
    type_unsigned_8 opcode;
    type_unsigned_8 group2_extension;
    C_INT group2_memory;
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
    if (data->flagLock ||
        !core_machine_80386_timing_has_source_prefixes(data, prefixes)) {
        core_machine_source_timing_mark_unallocated(machine, out_ticks);
        return 1;
    }
    if ((opcode == 0xc4u || opcode == 0xc5u) &&
        core_machine_control_stack_is_protected(data) &&
        core_machine_source_timing_modrm_is_memory(data, prefixes)) {
        C_INT operand32 = data->oldcpu.data.cs.seg.exec.defsize;

        if (data->prefix_oprsize) operand32 = !operand32;
        *out_ticks = operand32 ? 28u : 26u;
        return 1;
    }
    if (opcode >= 0x70u && opcode <= 0x7fu) {
        machine->source_timing_form_id = CORE_MACHINE_SOURCE_TIMING_JCC;
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
    case 0xd0u: case 0xd1u: case 0xd2u: case 0xd3u:
    case 0xc0u: case 0xc1u:
        if (prefixes + 1u >= data->oplen) {
            core_machine_source_timing_mark_unallocated(machine, out_ticks);
            return 1;
        }
        group2_extension = (data->opcodes[prefixes + 1u] >> 3u) & 7u;
        group2_memory = core_machine_source_timing_modrm_is_memory(data,
            prefixes);
        if (group2_extension == 0u || group2_extension == 1u ||
            group2_extension == 4u || group2_extension == 5u ||
            group2_extension == 7u) {
            *out_ticks = core_machine_80386_source_timing_lookup(machine,
                group2_memory ? CORE_MACHINE_SOURCE_TIMING_GROUP2_ROTATE_MEMORY :
                CORE_MACHINE_SOURCE_TIMING_GROUP2_ROTATE_REGISTER);
        } else if (group2_extension == 2u || group2_extension == 3u) {
            *out_ticks = core_machine_80386_source_timing_lookup(machine,
                group2_memory ? CORE_MACHINE_SOURCE_TIMING_GROUP2_CARRY_MEMORY :
                CORE_MACHINE_SOURCE_TIMING_GROUP2_CARRY_REGISTER);
        } else {
            core_machine_source_timing_mark_unallocated(machine, out_ticks);
        }
        return 1;
    case 0xf8u:
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_CLC);
        return 1;
    case 0xf5u:
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_CMC);
        return 1;
    case 0xf9u:
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_STC);
        return 1;
    case 0xfcu:
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_CLD);
        return 1;
    case 0xfdu:
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_STD);
        return 1;
    case 0xfau:
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_CLI);
        return 1;
    case 0xfbu:
        *out_ticks = core_machine_80386_source_timing_lookup(machine,
            CORE_MACHINE_SOURCE_TIMING_STI);
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

type_unsigned_64 core_machine_cpu_timing_maximum_ticks(
    core_machine_cpu_profile profile,
    const core_machine_instruction_timing *timing)
{
    switch (profile) {
    case CORE_MACHINE_CPU_PROFILE_8086:
        return CORE_MACHINE_8086_SOURCE_MAXIMUM_TICKS;
    case CORE_MACHINE_CPU_PROFILE_80186:
        return CORE_MACHINE_80186_SOURCE_MAXIMUM_TICKS;
    case CORE_MACHINE_CPU_PROFILE_80286:
        return CORE_MACHINE_80286_SOURCE_MAXIMUM_TICKS;
    case CORE_MACHINE_CPU_PROFILE_80386:
        return CORE_MACHINE_80386_SOURCE_MAXIMUM_TICKS;
    default:
        break;
    }
    if (timing == STD_NULL) return 0u;
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
    /* This retained compatibility recipe has no source-form allocation.  It
     * may drive deterministic execution, but can never qualify physical
     * retirement time for any CPU profile. */
    core_machine_source_timing_mark_unallocated(machine, STD_NULL);
    opcode = data->opcodes[prefixes];
    if (data->prefix_rep != PREFIX_REP_NONE && opcode == 0xa4u) {
        if (!core_machine_timing_add_ticks(&ticks, timing->rep_iteration_surcharge)) {
            return 0;
        }
    } else if (!core_machine_timing_add_ticks(&ticks,
            (type_unsigned_64)prefixes * timing->prefix_surcharge)) {
        return 0;
    }
    if (opcode >= 0x70u && opcode <= 0x7fu) {
        code32 = data->oldcpu.data.cs.seg.exec.defsize;
        fallthrough = data->oldcpu.data.eip + prefixes + 2u;
        if (!code32) fallthrough &= 0xffffu;
        if (machine->executor_cpu.data.eip != fallthrough &&
            !core_machine_timing_add_ticks(&ticks, timing->taken_branch_surcharge)) {
            return 0;
        }
    }
    if (opcode >= 0xa0u && opcode <= 0xa3u &&
        !core_machine_timing_add_ticks(&ticks, timing->data_memory_surcharge)) {
        return 0;
    }
    if (((opcode >= 0xe4u && opcode <= 0xe7u) ||
         (opcode >= 0xecu && opcode <= 0xefu)) &&
        !core_machine_timing_add_ticks(&ticks, timing->io_surcharge)) {
        return 0;
    }
    *out_ticks = ticks;
    return 1;
}

C_VOID core_machine_external_cycle_invalidate(core_machine *machine);

C_VOID core_machine_transaction_trace(C_VOID *opaque,
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
            &machine->transaction_contract.external_access_wait_windows[index];
        if (window->wait_ticks != 0u && window->space == space &&
            address >= window->first_address && address <= window->last_address) {
            return window->wait_ticks;
        }
    }
    return 0u;
}

C_VOID core_machine_external_cycle_invalidate(core_machine *machine)
{
    if (machine == STD_NULL) return;
    machine->external_cycle_page_valid = TYPE_FALSE;
    machine->external_cycle_pending_valid = TYPE_FALSE;
    machine->external_cycle_overlap_valid = TYPE_FALSE;
}

C_VOID core_machine_cpu_external_cycle_trace(C_VOID *opaque,
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
        machine->transaction_contract.external_cycle_timing.page_bytes != 0u &&
        ((machine->transaction_contract.external_cycle_timing.first_eligible_address == 0u &&
          machine->transaction_contract.external_cycle_timing.last_eligible_address == 0u) ||
         (address >= machine->transaction_contract.external_cycle_timing.first_eligible_address &&
          address <= machine->transaction_contract.external_cycle_timing.last_eligible_address));
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
        if (page_timing_enabled && machine->transaction_contract.external_cycle_timing.overlap_policy ==
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
            type_unsigned_32 page_tag = address /
                machine->transaction_contract.external_cycle_timing.page_bytes;
            type_unsigned_32 wait_ticks = !machine->external_cycle_page_valid ||
                !machine->external_cycle_overlap_valid ||
                machine->external_cycle_overlap_next_physical != address ||
                machine->external_cycle_page_tag != page_tag ?
                machine->transaction_contract.external_cycle_timing.page_miss_ticks :
                machine->transaction_contract.external_cycle_timing.page_hit_ticks;
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
}
