#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/cpu_timing.h"
#include "core/machine/machine_interface.h"
#include "core/machine/retirement_observation_interface.h"
#include "../support/core_machine_cpu_fixture.h"

/* Reuse the retained DPL3 outer-return fixture; its main is not part of this
 * runner, but keeps the fixture's private helpers live under -Werror. */
#define main timing_80286_manifest_retained_gate_main
#include "core_machine_protected_16_gate_s3_smoke.c"
#undef main

/* Reuse the retained protected-mode task-switch fixture so task-transfer
 * observations exercise the product mechanism rather than a shadow state. */
#define main timing_80286_manifest_retained_task_switch_main
#include "core_machine_task_switch_smoke.c"
#undef main

/* Reuse the retained 80286 call-gate bootstrap for both same- and
 * more-privileged transfer observations. */
#define main timing_80286_manifest_retained_call_gate_main
#include "core_machine_call_gate_smoke.c"
#undef main

#define TIMING_80286_MANIFEST_RESET_LINEAR 0x00fffff0u
#define TIMING_80286_MANIFEST_RESET_PHYSICAL 0x000ffff0u
#define TIMING_80286_MANIFEST_WINDOW_BYTES 16u
#define TIMING_80286_MANIFEST_STACK_LINEAR 0x00001000u
#define TIMING_80286_MANIFEST_STACK_BYTES 16u

/* Incremental real-observation runner for the I286 manifest.  A partial
 * recipe set must never write the final result document: the result verifier
 * accepts every observed canonical key. */
typedef struct timing_80286_manifest_record {
    const C_CHAR *key_id;
    const C_CHAR *profile;
    const C_CHAR *level;
    const C_CHAR *source_rule;
    const C_CHAR *context;
} timing_80286_manifest_record;

typedef struct timing_80286_manifest_capture {
    core_machine_retirement_observation observation;
    type_unsigned_32 count;
} timing_80286_manifest_capture;

typedef struct timing_80286_manifest_recipe {
    const C_CHAR *key_id;
    type_unsigned_8 program[16];
    STD_SIZE_T bytes;
    type_unsigned_64 ticks;
    core_machine_retirement_timing_origin origin;
} timing_80286_manifest_recipe;

typedef struct timing_80286_manifest_control_recipe {
    const C_CHAR *key_id;
    type_unsigned_8 opcode;
    type_unsigned_32 eflags;
    type_unsigned_64 ticks;
} timing_80286_manifest_control_recipe;

typedef struct timing_80286_manifest_repeat_recipe {
    const C_CHAR *key_id;
    type_unsigned_8 prefix;
    type_unsigned_8 opcode;
    type_unsigned_64 first_ticks;
    type_unsigned_64 continuation_ticks;
    type_unsigned_64 zero_ticks;
} timing_80286_manifest_repeat_recipe;

static const timing_80286_manifest_record timing_80286_manifest_records[] = {
#include "cpu_timing_manifest_metadata_catalog.inc"
};
static C_INT timing_80286_manifest_observed[
    sizeof(timing_80286_manifest_records) / sizeof(timing_80286_manifest_records[0])];
static core_machine_retirement_observation timing_80286_manifest_results[
    sizeof(timing_80286_manifest_records) / sizeof(timing_80286_manifest_records[0])];
static C_INT timing_80286_manifest_current_index = -1;
static C_INT timing_80286_manifest_base_index = -1;
static C_INT timing_80286_manifest_flags_active = 0;
static type_unsigned_32 timing_80286_manifest_eflags;

static type_status timing_80286_manifest_port_read(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 *out_value)
{
    (C_VOID)owner;
    if (port != 0x0080u || out_value == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_value = 0x5au;
    return TYPE_STATUS_OK;
}

static type_status timing_80286_manifest_port_write(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 value)
{
    (C_VOID)owner;
    return port == 0x0080u && value <= 0xffffu ? TYPE_STATUS_OK :
        TYPE_STATUS_INVALID_ARGUMENT;
}

static const core_machine_port_provider timing_80286_manifest_ports = {
    timing_80286_manifest_port_read, timing_80286_manifest_port_write
};

static C_INT timing_80286_manifest_key_has_prefix(const C_CHAR *key,
    const C_CHAR *prefix);

static C_INT timing_80286_manifest_is_dx_port(const C_CHAR *key_id)
{
    return key_id != STD_NULL && (STD_STRCMP(key_id, "I286-IN-DX-B") == 0 ||
        STD_STRCMP(key_id, "I286-IN-DX-W") == 0 ||
        STD_STRCMP(key_id, "I286-OUT-DX-B") == 0 ||
        STD_STRCMP(key_id, "I286-OUT-DX-W") == 0);
}

static C_INT timing_80286_manifest_is_interrupt(const C_CHAR *key_id)
{
    static const C_CHAR *const keys[] = {
        "I286-INT3-REAL-NEXT-BYTE-1", "I286-INT3-REAL-NEXT-BYTE-2",
        "I286-INT3-REAL-NEXT-BYTE-3", "I286-INT3-REAL-NEXT-BYTE-4",
        "I286-INT3-REAL-NEXT-BYTE-5", "I286-INT3-REAL-NEXT-BYTE-6",
        "I286-INT-IMM-REAL-NEXT-BYTE-1", "I286-INT-IMM-REAL-NEXT-BYTE-2",
        "I286-INT-IMM-REAL-NEXT-BYTE-3", "I286-INT-IMM-REAL-NEXT-BYTE-4",
        "I286-INT-IMM-REAL-NEXT-BYTE-5", "I286-INT-IMM-REAL-NEXT-BYTE-6"
    };
    STD_SIZE_T index;

    if (key_id == STD_NULL) return 0;
    if (timing_80286_manifest_key_has_prefix(key_id, "I286-INTO-TAKEN")) {
        return 1;
    }
    for (index = 0u; index < sizeof(keys) / sizeof(keys[0]); ++index) {
        if (STD_STRCMP(key_id, keys[index]) == 0) return 1;
    }
    return 0;
}

static C_INT timing_80286_manifest_uses_stack(const C_CHAR *key_id)
{
    static const C_CHAR *const stack_keys[] = {
        "I286-CALL-NEAR-DIRECT-NEXT-BYTE-1", "I286-CALL-NEAR-RM-SEGMENT",
        "I286-CALL-NEAR-RM-EA-BID", "I286-STACK-PUSH-M",
        "I286-STACK-POP-M", "I286-STACK-PUSH-M-ODD-WORD",
        "I286-STACK-POP-M-ODD-WORD", "I286-CALL-NEAR-DIRECT-NEXT-BYTE-2",
        "I286-CALL-NEAR-DIRECT-NEXT-BYTE-3",
        "I286-CALL-NEAR-DIRECT-NEXT-BYTE-4",
        "I286-CALL-NEAR-DIRECT-NEXT-BYTE-5",
        "I286-CALL-NEAR-DIRECT-NEXT-BYTE-6",
        "I286-RET-NEAR-NEXT-BYTE-1", "I286-STACK-PUSH-R",
        "I286-STACK-PUSH-SEG", "I286-STACK-PUSH-IMM",
        "I286-STACK-PUSH-M-EA-BID", "I286-STACK-POP-M-EA-BID",
        "I286-STACK-PUSH-M-EA-BID-ODD-WORD",
        "I286-STACK-POP-M-EA-BID-ODD-WORD",
        "I286-STACK-PUSHA", "I286-STACK-PUSHF", "I286-STACK-POP-R",
        "I286-STACK-POP-SEG-REAL", "I286-STACK-POPA",
        "I286-STACK-POPF", "I286-STACK-LEAVE", "I286-ENTER-L0",
        "I286-ENTER-L1", "I286-ENTER-LN"
    };
    STD_SIZE_T index;

    if (key_id == STD_NULL) return 0;
    for (index = 0u; index < sizeof(stack_keys) / sizeof(stack_keys[0]); ++index) {
        if (STD_STRCMP(key_id, stack_keys[index]) == 0) return 1;
    }
    return 0;
}

static C_INT timing_80286_manifest_uses_far_target(const C_CHAR *key_id)
{
    return key_id != STD_NULL && (STD_STRCMP(key_id,
        "I286-JMP-FAR-REAL-NEXT-BYTE-2") == 0 || STD_STRCMP(key_id,
        "I286-RET-NEAR-NEXT-BYTE-1") == 0);
}

static C_INT timing_80286_manifest_is_ret_near_next_byte(const C_CHAR *key_id)
{
    static const C_CHAR *const keys[] = {
        "I286-RET-NEAR-NEXT-BYTE-1", "I286-RET-NEAR-NEXT-BYTE-2",
        "I286-RET-NEAR-NEXT-BYTE-3", "I286-RET-NEAR-NEXT-BYTE-4",
        "I286-RET-NEAR-NEXT-BYTE-5", "I286-RET-NEAR-NEXT-BYTE-6"
    };
    STD_SIZE_T index;

    if (key_id == STD_NULL) return 0;
    for (index = 0u; index < sizeof(keys) / sizeof(keys[0]); ++index) {
        if (STD_STRCMP(key_id, keys[index]) == 0) return 1;
    }
    return 0;
}

static C_INT timing_80286_manifest_is_bound(const C_CHAR *key_id)
{
    return key_id != STD_NULL && STD_STRCMP(key_id, "I286-BOUND") == 0;
}

static C_INT timing_80286_manifest_uses_odd_memory_operand(const C_CHAR *key_id)
{
    static const C_CHAR *const keys[] = {
        "I286-MUL-M16-ODD-WORD", "I286-IMUL-M16-ODD-WORD",
        "I286-DIV-M16-ODD-WORD", "I286-IDIV-M16-ODD-WORD",
        "I286-IMUL-IMM-IMM16-M-ODD-WORD", "I286-MOV-RM-ODD-WORD",
        "I286-MOV-MR-ODD-WORD", "I286-MOV-MI-ODD-WORD",
        "I286-STACK-PUSH-M-ODD-WORD", "I286-STACK-POP-M-ODD-WORD",
        "I286-MUL-M16-EA-BID-ODD-WORD",
        "I286-IMUL-M16-EA-BID-ODD-WORD",
        "I286-DIV-M16-EA-BID-ODD-WORD",
        "I286-IDIV-M16-EA-BID-ODD-WORD",
        "I286-MOV-RM-EA-BID-ODD-WORD",
        "I286-MOV-MR-EA-BID-ODD-WORD",
        "I286-MOV-MI-EA-BID-ODD-WORD",
        "I286-MOV-SREG-STORE-EA-BID-ODD-WORD",
        "I286-MOV-SREG-LOAD-REAL-EA-BID-ODD-WORD",
        "I286-MOV-SREG-LOAD-PM-EA-BID-ODD-WORD",
        "I286-LDS-M-REAL-EA-BID-ODD-WORD",
        "I286-LDS-M-PM-EA-BID-ODD-WORD",
        "I286-LES-M-REAL-EA-BID-ODD-WORD",
        "I286-LES-M-PM-EA-BID-ODD-WORD",
        "I286-STACK-PUSH-M-EA-BID-ODD-WORD",
        "I286-STACK-POP-M-EA-BID-ODD-WORD"
    };
    STD_SIZE_T index;

    if (key_id == STD_NULL) return 0;
    for (index = 0u; index < sizeof(keys) / sizeof(keys[0]); ++index) {
        if (STD_STRCMP(key_id, keys[index]) == 0) return 1;
    }
    return 0;
}

static type_unsigned_16 timing_80286_manifest_control_cx(const C_CHAR *key_id)
{
    if (key_id == STD_NULL) return 2u;
    if (STD_STRCMP(key_id, "I286-JCXZ-TAKEN") == 0) return 0u;
    if (STD_STRCMP(key_id, "I286-JCXZ-NOT") == 0 ||
        STD_STRCMP(key_id, "I286-LOOP-NOT") == 0) return 1u;
    return 2u;
}

static core_machine_retirement_timing_origin
timing_80286_manifest_control_origin(type_unsigned_8 opcode)
{
    if (opcode >= 0x70u && opcode <= 0x7fu) {
        return CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY;
    }
    return (opcode == 0xceu || (opcode >= 0xe0u && opcode <= 0xe3u)) ?
        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK :
        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80286_FALLBACK;
}

static C_INT timing_80286_manifest_is_i286(
    const timing_80286_manifest_record *record)
{
    return record != STD_NULL && record->key_id[0] == 'I' &&
        record->key_id[1] == '2' && record->key_id[2] == '8' &&
        record->key_id[3] == '6' && record->key_id[4] == '-';
}

static const timing_80286_manifest_record *timing_80286_manifest_find(
    const C_CHAR *key_id)
{
    STD_SIZE_T index;

    timing_80286_manifest_current_index = -1;
    timing_80286_manifest_base_index = -1;
    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        if (STD_STRCMP(timing_80286_manifest_records[index].key_id, key_id) == 0) {
            timing_80286_manifest_current_index = (C_INT)index;
            break;
        }
    }
    if (timing_80286_manifest_current_index < 0) return STD_NULL;
    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        const C_CHAR *base = timing_80286_manifest_records[index].key_id;
        STD_SIZE_T length = 0u;

        while (base[length] != '\0' && key_id[length] == base[length]) ++length;
        if (base[length] == '\0' && STD_STRCMP(key_id + length,
                "-NEXT-BYTE-1") == 0) {
            timing_80286_manifest_base_index = (C_INT)index;
            break;
        }
    }
    return &timing_80286_manifest_records[timing_80286_manifest_current_index];
}

static C_VOID timing_80286_manifest_capture_retirement(C_VOID *opaque,
    const core_machine_retirement_observation *observation)
{
    timing_80286_manifest_capture *capture =
        (timing_80286_manifest_capture *)opaque;

    if (capture == STD_NULL || observation == STD_NULL) return;
    if (capture->count == 0u) capture->observation = *observation;
    if (timing_80286_manifest_current_index >= 0 &&
        !timing_80286_manifest_observed[timing_80286_manifest_current_index]) {
        timing_80286_manifest_results[timing_80286_manifest_current_index] =
            *observation;
        timing_80286_manifest_observed[timing_80286_manifest_current_index] = 1;
    }
    if (timing_80286_manifest_base_index >= 0 &&
        !timing_80286_manifest_observed[timing_80286_manifest_base_index]) {
        timing_80286_manifest_results[timing_80286_manifest_base_index] =
            *observation;
        timing_80286_manifest_observed[timing_80286_manifest_base_index] = 1;
    }
    timing_80286_manifest_current_index = -1;
    timing_80286_manifest_base_index = -1;
    ++capture->count;
}

static C_INT timing_80286_manifest_results_complete(C_VOID)
{
    STD_SIZE_T index;
    STD_SIZE_T observed = 0u;
    STD_SIZE_T expected = 0u;

    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        if (!timing_80286_manifest_is_i286(&timing_80286_manifest_records[index])) {
            continue;
        }
        ++expected;
        if (!timing_80286_manifest_observed[index]) return 0;
        ++observed;
    }
    return expected != 0u && observed == expected;
}

static type_unsigned_32 timing_80286_manifest_observed_count(C_VOID)
{
    STD_SIZE_T index;
    type_unsigned_32 observed = 0u;

    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        if (timing_80286_manifest_is_i286(&timing_80286_manifest_records[index]) &&
            timing_80286_manifest_observed[index]) {
            ++observed;
        }
    }
    return observed;
}

static type_unsigned_32 timing_80286_manifest_expected_count(C_VOID)
{
    STD_SIZE_T index;
    type_unsigned_32 expected = 0u;

    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        if (timing_80286_manifest_is_i286(&timing_80286_manifest_records[index])) {
            ++expected;
        }
    }
    return expected;
}

static C_INT timing_80286_manifest_key_has_prefix(const C_CHAR *key,
    const C_CHAR *prefix)
{
    STD_SIZE_T index;

    if (key == STD_NULL || prefix == STD_NULL) return 0;
    for (index = 0u; prefix[index] != '\0'; ++index) {
        if (key[index] != prefix[index]) return 0;
    }
    return 1;
}

static C_INT timing_80286_manifest_key_has_token(const C_CHAR *key,
    const C_CHAR *token)
{
    STD_SIZE_T key_index;

    if (key == STD_NULL || token == STD_NULL || token[0] == '\0') return 0;
    for (key_index = 0u; key[key_index] != '\0'; ++key_index) {
        if (timing_80286_manifest_key_has_prefix(key + key_index, token)) return 1;
    }
    return 0;
}

/* Kept byte-for-byte aligned with the S1 partition's later-S boundaries. */
static C_INT timing_80286_manifest_is_s3(
    const timing_80286_manifest_record *record)
{
    const C_CHAR *key = record != STD_NULL ? record->key_id : STD_NULL;

    if (!timing_80286_manifest_is_i286(record)) return 0;
    if (timing_80286_manifest_key_has_prefix(key, "I286-STRING-") ||
        timing_80286_manifest_key_has_prefix(key, "I286-REP-") ||
        timing_80286_manifest_key_has_prefix(key, "I286-JCC-") ||
        timing_80286_manifest_key_has_prefix(key, "I286-JCXZ-") ||
        timing_80286_manifest_key_has_prefix(key, "I286-LOOP") ||
        timing_80286_manifest_key_has_prefix(key, "I286-INTO-") ||
        timing_80286_manifest_key_has_prefix(key, "I286-CALL-") ||
        timing_80286_manifest_key_has_prefix(key, "I286-RET-") ||
        timing_80286_manifest_key_has_prefix(key, "I286-JMP-") ||
        timing_80286_manifest_key_has_prefix(key, "I286-INT3-") ||
        timing_80286_manifest_key_has_prefix(key, "I286-INT-IMM-") ||
        timing_80286_manifest_key_has_prefix(key, "I286-INT-PM-") ||
        timing_80286_manifest_key_has_prefix(key, "I286-INT-TASK") ||
        timing_80286_manifest_key_has_prefix(key, "I286-MOV-SREG-") ||
        timing_80286_manifest_key_has_prefix(key, "I286-LEA-") ||
        timing_80286_manifest_key_has_prefix(key, "I286-LDS-") ||
        timing_80286_manifest_key_has_prefix(key, "I286-LES-") ||
        timing_80286_manifest_key_has_prefix(key, "I286-SYSTEM-") ||
        timing_80286_manifest_key_has_prefix(key, "I286-STACK-POP-SEG-")) {
        return 0;
    }
    return 1;
}

static type_unsigned_32 timing_80286_manifest_s3_observed_count(C_VOID)
{
    STD_SIZE_T index;
    type_unsigned_32 observed = 0u;

    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        if (timing_80286_manifest_is_s3(&timing_80286_manifest_records[index]) &&
            timing_80286_manifest_observed[index]) {
            ++observed;
        }
    }
    return observed;
}

static type_unsigned_32 timing_80286_manifest_s3_expected_count(C_VOID)
{
    STD_SIZE_T index;
    type_unsigned_32 expected = 0u;

    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        if (timing_80286_manifest_is_s3(&timing_80286_manifest_records[index])) {
            ++expected;
        }
    }
    return expected;
}

static C_INT timing_80286_manifest_s3_results_complete(C_VOID)
{
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        const timing_80286_manifest_record *record =
            &timing_80286_manifest_records[index];
        const core_machine_retirement_observation *observation =
            &timing_80286_manifest_results[index];
        type_unsigned_32 required_inputs = 0u;

        if (!timing_80286_manifest_is_s3(record)) continue;
        if (!timing_80286_manifest_observed[index] ||
            observation->timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED) return 0;
        if (timing_80286_manifest_key_has_token(record->key_id, "-LOCK")) {
            required_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_LOCK;
        }
        if (timing_80286_manifest_key_has_token(record->key_id, "-SEGMENT")) {
            required_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE;
        }
        if (timing_80286_manifest_key_has_token(record->key_id, "-EA-BID")) {
            required_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS;
        }
        if (timing_80286_manifest_key_has_token(record->key_id, "-ODD-WORD")) {
            required_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD;
        }
        if ((observation->formula_inputs & required_inputs) != required_inputs) {
            return 0;
        }
    }
    return timing_80286_manifest_s3_observed_count() ==
        timing_80286_manifest_s3_expected_count();
}

static C_INT timing_80286_manifest_is_s4(
    const timing_80286_manifest_record *record)
{
    const C_CHAR *key = record != STD_NULL ? record->key_id : STD_NULL;

    return timing_80286_manifest_is_i286(record) &&
        (timing_80286_manifest_key_has_prefix(key, "I286-STRING-") ||
         timing_80286_manifest_key_has_prefix(key, "I286-REP-"));
}

static C_INT timing_80286_manifest_is_s5(
    const timing_80286_manifest_record *record)
{
    const C_CHAR *key = record != STD_NULL ? record->key_id : STD_NULL;

    return timing_80286_manifest_is_i286(record) &&
        (timing_80286_manifest_key_has_prefix(key, "I286-JCC-") ||
         timing_80286_manifest_key_has_prefix(key, "I286-JCXZ-") ||
         timing_80286_manifest_key_has_prefix(key, "I286-LOOP") ||
         timing_80286_manifest_key_has_prefix(key, "I286-INTO-"));
}

static C_INT timing_80286_manifest_is_s6(
    const timing_80286_manifest_record *record)
{
    const C_CHAR *key = record != STD_NULL ? record->key_id : STD_NULL;

    return timing_80286_manifest_is_i286(record) &&
        (timing_80286_manifest_key_has_prefix(key, "I286-CALL-") ||
         timing_80286_manifest_key_has_prefix(key, "I286-RET-") ||
         timing_80286_manifest_key_has_prefix(key, "I286-JMP-") ||
         timing_80286_manifest_key_has_prefix(key, "I286-INT3-") ||
         timing_80286_manifest_key_has_prefix(key, "I286-INT-IMM-") ||
         timing_80286_manifest_key_has_prefix(key, "I286-INT-PM-") ||
         timing_80286_manifest_key_has_prefix(key, "I286-INT-TASK"));
}

/* S1 assigns every remaining 80286 manifest record to S7.  Expressing this
 * as the disjoint complement keeps the executable partition tied to the
 * earlier S3--S6 ownership predicates. */
static C_INT timing_80286_manifest_is_s7(
    const timing_80286_manifest_record *record)
{
    return timing_80286_manifest_is_i286(record) &&
        !timing_80286_manifest_is_s3(record) &&
        !timing_80286_manifest_is_s4(record) &&
        !timing_80286_manifest_is_s5(record) &&
        !timing_80286_manifest_is_s6(record);
}

static type_unsigned_32 timing_80286_manifest_s5_observed_count(C_VOID)
{
    STD_SIZE_T index;
    type_unsigned_32 observed = 0u;

    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        if (timing_80286_manifest_is_s5(&timing_80286_manifest_records[index]) &&
            timing_80286_manifest_observed[index]) {
            ++observed;
        }
    }
    return observed;
}

static type_unsigned_32 timing_80286_manifest_s6_observed_count(C_VOID)
{
    STD_SIZE_T index;
    type_unsigned_32 observed = 0u;

    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        if (timing_80286_manifest_is_s6(&timing_80286_manifest_records[index]) &&
            timing_80286_manifest_observed[index]) {
            ++observed;
        }
    }
    return observed;
}

static type_unsigned_32 timing_80286_manifest_s7_observed_count(C_VOID)
{
    STD_SIZE_T index;
    type_unsigned_32 observed = 0u;

    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        if (timing_80286_manifest_is_s7(&timing_80286_manifest_records[index]) &&
            timing_80286_manifest_observed[index]) {
            ++observed;
        }
    }
    return observed;
}

static type_unsigned_32 timing_80286_manifest_s5_expected_count(C_VOID)
{
    STD_SIZE_T index;
    type_unsigned_32 expected = 0u;

    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        if (timing_80286_manifest_is_s5(&timing_80286_manifest_records[index])) {
            ++expected;
        }
    }
    return expected;
}

static C_INT timing_80286_manifest_s5_results_complete(C_VOID)
{
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        const timing_80286_manifest_record *record =
            &timing_80286_manifest_records[index];
        const core_machine_retirement_observation *observation =
            &timing_80286_manifest_results[index];
        core_machine_retirement_timing_origin origin;
        type_unsigned_32 required_inputs = 0u;

        if (!timing_80286_manifest_is_s5(record)) continue;
        origin = timing_80286_manifest_key_has_prefix(record->key_id,
            "I286-JCC-") ? CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY :
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK;
        if (!timing_80286_manifest_observed[index] ||
            observation->timing_origin != origin ||
            observation->timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED) return 0;
        if (timing_80286_manifest_key_has_token(record->key_id,
                "-NEXT-BYTE")) {
            required_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_CONTROL;
        }
        if ((observation->formula_inputs & required_inputs) != required_inputs) {
            return 0;
        }
    }
    return timing_80286_manifest_s5_observed_count() ==
        timing_80286_manifest_s5_expected_count();
}

static type_unsigned_32 timing_80286_manifest_s6_expected_count(C_VOID)
{
    STD_SIZE_T index;
    type_unsigned_32 expected = 0u;

    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        if (timing_80286_manifest_is_s6(&timing_80286_manifest_records[index])) {
            ++expected;
        }
    }
    return expected;
}

static type_unsigned_32 timing_80286_manifest_s7_expected_count(C_VOID)
{
    STD_SIZE_T index;
    type_unsigned_32 expected = 0u;

    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        if (timing_80286_manifest_is_s7(&timing_80286_manifest_records[index])) {
            ++expected;
        }
    }
    return expected;
}

static C_INT timing_80286_manifest_s6_results_complete(C_VOID)
{
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        const timing_80286_manifest_record *record =
            &timing_80286_manifest_records[index];
        const core_machine_retirement_observation *observation =
            &timing_80286_manifest_results[index];
        type_unsigned_32 required_inputs = 0u;

        if (!timing_80286_manifest_is_s6(record)) continue;
        if (!timing_80286_manifest_observed[index] ||
            observation->timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            observation->timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED) return 0;
        if (timing_80286_manifest_key_has_token(record->key_id,
                "-NEXT-BYTE")) {
            required_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_CONTROL;
        }
        if (timing_80286_manifest_key_has_token(record->key_id, "-EA-BID")) {
            required_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS;
        }
        if (timing_80286_manifest_key_has_token(record->key_id, "-SEGMENT")) {
            required_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE;
        }
        if ((observation->formula_inputs & required_inputs) != required_inputs) {
            return 0;
        }
    }
    return timing_80286_manifest_s6_observed_count() ==
        timing_80286_manifest_s6_expected_count();
}

static C_INT timing_80286_manifest_s7_results_complete(C_VOID)
{
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        const timing_80286_manifest_record *record =
            &timing_80286_manifest_records[index];
        const core_machine_retirement_observation *observation =
            &timing_80286_manifest_results[index];
        core_machine_retirement_timing_origin origin;
        type_unsigned_32 required_inputs = 0u;

        if (!timing_80286_manifest_is_s7(record)) continue;
        origin = timing_80286_manifest_key_has_prefix(record->key_id,
            "I286-STACK-POP-SEG-") ?
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK :
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY;
        if (!timing_80286_manifest_observed[index] ||
            observation->timing_origin != origin ||
            observation->timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED) return 0;
        if (timing_80286_manifest_key_has_token(record->key_id, "-EA-BID")) {
            required_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS;
        }
        if (timing_80286_manifest_key_has_token(record->key_id, "-SEGMENT")) {
            required_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE;
        }
        if (timing_80286_manifest_key_has_token(record->key_id, "-ODD-WORD")) {
            required_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD;
        }
        if ((observation->formula_inputs & required_inputs) != required_inputs) {
            return 0;
        }
    }
    return timing_80286_manifest_s7_observed_count() ==
        timing_80286_manifest_s7_expected_count();
}

static type_unsigned_32 timing_80286_manifest_s4_observed_count(C_VOID)
{
    STD_SIZE_T index;
    type_unsigned_32 observed = 0u;

    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        if (timing_80286_manifest_is_s4(&timing_80286_manifest_records[index]) &&
            timing_80286_manifest_observed[index]) {
            ++observed;
        }
    }
    return observed;
}

static type_unsigned_32 timing_80286_manifest_s4_expected_count(C_VOID)
{
    STD_SIZE_T index;
    type_unsigned_32 expected = 0u;

    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        if (timing_80286_manifest_is_s4(&timing_80286_manifest_records[index])) {
            ++expected;
        }
    }
    return expected;
}

static C_INT timing_80286_manifest_s4_results_complete(C_VOID)
{
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        const timing_80286_manifest_record *record =
            &timing_80286_manifest_records[index];
        const core_machine_retirement_observation *observation =
            &timing_80286_manifest_results[index];
        type_unsigned_32 required_inputs = 0u;

        if (!timing_80286_manifest_is_s4(record)) continue;
        if (!timing_80286_manifest_observed[index] ||
            observation->timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO ||
            observation->timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED) return 0;
        if (timing_80286_manifest_key_has_prefix(record->key_id, "I286-REP-")) {
            required_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_REPEAT |
                CORE_MACHINE_CPU_TIMING_INPUT_REPEAT_PHASE;
        }
        if (timing_80286_manifest_key_has_token(record->key_id, "-ODD-WORD") &&
            !timing_80286_manifest_key_has_token(record->key_id,
                "-REP-PHASE-ZERO")) {
            required_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD;
        }
        if ((observation->formula_inputs & required_inputs) != required_inputs) {
            return 0;
        }
    }
    return timing_80286_manifest_s4_observed_count() ==
        timing_80286_manifest_s4_expected_count();
}

/* This writer is intentionally unavailable to a partial or pre-S8 runner.
 * S3--S7 may add real recipes, but only the final closure packet can publish
 * the all-key artifact after every canonical record was observed. */
static C_INT timing_80286_manifest_write_results(const C_CHAR *path,
    C_INT final_results_authorized)
{
    STD_FILE *file;
    STD_SIZE_T index;
    STD_SIZE_T written = 0u;

    if (path == STD_NULL || !final_results_authorized ||
        !timing_80286_manifest_results_complete()) return 1;
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL) return 1;
    if (STD_FPRINTF(file, "{\n  \"schema\": \"nxvm.cpu-timing-results.v1\",\n"
            "  \"profile\": \"80286\",\n  \"results\": [\n") < 0) {
        STD_FCLOSE(file);
        return 1;
    }
    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        const timing_80286_manifest_record *record =
            &timing_80286_manifest_records[index];
        const core_machine_retirement_observation *observation =
            &timing_80286_manifest_results[index];

        if (!timing_80286_manifest_is_i286(record)) continue;
        if ((written != 0u && STD_FPRINTF(file, ",\n") < 0) ||
            STD_FPRINTF(file, "    {\"key_id\":\"%s\","
                "\"profile\":\"%s\",\"level\":\"%s\","
                "\"source_rule\":\"%s\",\"context\":\"%s\","
                "\"ticks\":%llu,\"formula_inputs\":%u,"
                "\"form_id\":%u,\"retirement_origin\":%d,"
                "\"source_timing_unallocated\":%s,\"passed\":true}",
                record->key_id, record->profile, record->level,
                record->source_rule, record->context, observation->source_ticks,
                observation->formula_inputs, observation->source_timing_form_id,
                observation->timing_origin,
                observation->timing_disposition ==
                    CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED ?
                    "true" : "false") < 0) {
            STD_FCLOSE(file);
            return 1;
        }
        ++written;
    }
    if (STD_FPRINTF(file, "\n  ]\n}\n") < 0 || STD_FCLOSE(file) != 0) return 1;
    return written == timing_80286_manifest_expected_count() ? 0 : 1;
}

static C_VOID timing_80286_manifest_execution_reset(C_VOID *opaque)
{
    (C_VOID)opaque;
}

static const core_machine_execution_provider timing_80286_manifest_execution = {
    timing_80286_manifest_execution_reset, STD_NULL
};

static C_INT timing_80286_manifest_prepare(core_machine **out_machine,
    timing_80286_manifest_capture *capture,
    const C_CHAR *key_id, const type_unsigned_8 *program, STD_SIZE_T bytes)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286,
        .ticks_per_instruction = 29u,
        .instruction_timing = { 29u, 7u, 31u, 37u, 41u, 43u }
    };
    const core_machine_retirement_observation_provider provider = {
        timing_80286_manifest_capture_retirement, capture
    };
    core_machine *machine = STD_NULL;
    type_status status;

    if (out_machine == STD_NULL || capture == STD_NULL || key_id == STD_NULL ||
        program == STD_NULL || bytes == 0u) return 0;
    status = core_machine_create(&config, &machine);
    if (status == TYPE_STATUS_OK) status =
        test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_80286_MANIFEST_RESET_LINEAR,
            TIMING_80286_MANIFEST_RESET_PHYSICAL,
            TIMING_80286_MANIFEST_WINDOW_BYTES);
    if (status == TYPE_STATUS_OK) status = core_machine_install_port_provider(
        machine, 0x0080u, 0x0080u, &timing_80286_manifest_ports, STD_NULL);
    if (status == TYPE_STATUS_OK) status = core_machine_bind_execution_provider(
        machine, &timing_80286_manifest_execution, STD_NULL);
    if (status == TYPE_STATUS_OK) status = core_machine_freeze_execution_providers(machine);
    if (status == TYPE_STATUS_OK) status = core_machine_reset(machine);
    if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
        TIMING_80286_MANIFEST_RESET_PHYSICAL, program, bytes);
    if (status == TYPE_STATUS_OK && timing_80286_manifest_is_interrupt(key_id)) {
        const type_unsigned_16 handler[] = { 0xfff5u, 0xf000u };
        const type_unsigned_8 handler_code[] = { 0x00u, 0xc0u };

        machine->executor_cpu.data.sp = TIMING_80286_MANIFEST_STACK_LINEAR +
            TIMING_80286_MANIFEST_STACK_BYTES;
        status = core_machine_memory_write(machine, 3u * 4u, handler,
            sizeof(handler));
        if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
            4u * 4u, handler, sizeof(handler));
        if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
            0x60u * 4u, handler, sizeof(handler));
        if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
            0x000ffff5u, handler_code, sizeof(handler_code));
    }
    if (status == TYPE_STATUS_OK) {
        const type_unsigned_16 operand = 1u;

        machine->executor_cpu.data.eax = 1u;
        machine->executor_cpu.data.ecx = 2u;
        machine->executor_cpu.data.edx = 0u;
        machine->executor_cpu.data.ebp = 0x0800u;
        machine->executor_cpu.data.esi = 0x0800u;
        if (timing_80286_manifest_is_dx_port(key_id)) {
            machine->executor_cpu.data.edx = 0x0080u;
        }
        status = core_machine_memory_write(machine, 0x1000u, &operand,
            sizeof(operand));
        if (status == TYPE_STATUS_OK &&
            timing_80286_manifest_uses_odd_memory_operand(key_id)) {
            status = core_machine_memory_write(machine, 0x1001u, &operand,
                sizeof(operand));
        }
    }
    if (status == TYPE_STATUS_OK && timing_80286_manifest_is_bound(key_id)) {
        const type_unsigned_16 bounds[] = { 0u, 2u };

        status = core_machine_memory_write(machine, 0x1000u, bounds,
            sizeof(bounds));
    }
    if (status == TYPE_STATUS_OK && timing_80286_manifest_uses_stack(key_id)) {
        machine->executor_cpu.data.sp = TIMING_80286_MANIFEST_STACK_LINEAR +
            TIMING_80286_MANIFEST_STACK_BYTES;
    }
    if (status == TYPE_STATUS_OK &&
        timing_80286_manifest_is_ret_near_next_byte(key_id)) {
        const type_unsigned_16 return_ip = 0xfff5u;

        machine->executor_cpu.data.sp = TIMING_80286_MANIFEST_STACK_LINEAR;
        status = core_machine_memory_write(machine,
            TIMING_80286_MANIFEST_STACK_LINEAR, &return_ip, sizeof(return_ip));
    }
    if (status == TYPE_STATUS_OK && STD_STRCMP(key_id,
            "I286-STACK-LEAVE") == 0) {
        machine->executor_cpu.data.ebp = TIMING_80286_MANIFEST_STACK_LINEAR;
    }
    if (status == TYPE_STATUS_OK && timing_80286_manifest_uses_far_target(key_id)) {
        static const type_unsigned_8 one_byte_target[] = { 0x90u };
        static const type_unsigned_8 two_byte_target[] = { 0x00u, 0xc0u };
        const C_VOID *target_code = STD_STRCMP(key_id,
            "I286-RET-NEAR-NEXT-BYTE-1") == 0 ? one_byte_target :
            two_byte_target;
        STD_SIZE_T target_bytes = target_code == one_byte_target ?
            sizeof(one_byte_target) : sizeof(two_byte_target);

        status = core_machine_memory_write(machine, 0x000ffff5u, target_code,
            target_bytes);
    }
    if (status == TYPE_STATUS_OK && timing_80286_manifest_flags_active) {
        machine->executor_cpu.data.eflags = timing_80286_manifest_eflags;
    }
    if (status == TYPE_STATUS_OK && (STD_STRCMP(key_id, "I286-XLAT") == 0 ||
            STD_STRCMP(key_id, "I286-XLAT-SEGMENT") == 0)) {
        static const type_unsigned_8 value[] = { 0x5au };

        machine->executor_cpu.data.ebx = 0x1000u;
        machine->executor_cpu.data.eax = 1u;
        status = core_machine_memory_write(machine, 0x1001u, value, sizeof(value));
    }
    if (status == TYPE_STATUS_OK) status =
        core_machine_set_retirement_observation_provider(machine, &provider);
    if (status != TYPE_STATUS_OK) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

/* System descriptor instructions must execute after the 80286's protected
 * transition.  Bootstrap runs without the observer; the observer therefore
 * records only the canonical target retirement. */
static C_INT timing_80286_manifest_prepare_protected_system(
    core_machine **out_machine, timing_80286_manifest_capture *capture,
    const C_CHAR *key_id, const type_unsigned_8 *program, STD_SIZE_T bytes)
{
    static const type_unsigned_8 gdt_pointer[] = { 0x37u, 0u, 0u, 0x03u,
        0u, 0u };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x30u,0,0x12u,0,0,
        0xffu,0xffu,0,0x30u,0,0x98u,0,0,
        0x0fu,0,0,0x50u,0,0x82u,0,0,
        0xffu,0xffu,0,0,0,0x89u,0,0
    };
    static const type_unsigned_8 boot[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u, 0xb8u,0x01u,0u,
        0x0fu,0x01u,0xf0u, 0xb8u,0x10u,0u, 0x8eu,0xd0u,
        0x8eu,0xd8u, 0x8eu,0xc0u, 0xeau,0u,0u,0x08u,0u
    };
    static const type_unsigned_8 halt[] = { 0xf4u };
    static const type_unsigned_8 table_pointer[] = { 0x37u, 0u, 0u, 0x03u,
        0u, 0u };
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286,
        .ticks_per_instruction = 29u,
        .instruction_timing = { 29u, 7u, 31u, 37u, 41u, 43u }
    };
    const core_machine_retirement_observation_provider provider = {
        timing_80286_manifest_capture_retirement, capture
    };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    type_unsigned_16 operand = 0x0010u;
    type_status status;

    if (out_machine == STD_NULL || capture == STD_NULL || key_id == STD_NULL ||
        program == STD_NULL || bytes == 0u) return 0;
    status = core_machine_create(&config, &machine);
    if (status == TYPE_STATUS_OK) status = core_machine_install_port_provider(
        machine, 0x0080u, 0x0080u, &timing_80286_manifest_ports, STD_NULL);
    if (status == TYPE_STATUS_OK) status = core_machine_bind_execution_provider(
        machine, &timing_80286_manifest_execution, STD_NULL);
    if (status == TYPE_STATUS_OK) status = core_machine_freeze_execution_providers(machine);
    if (status == TYPE_STATUS_OK) status = core_machine_reset(machine);
    if (status == TYPE_STATUS_OK) {
        machine->executor_cpu.data.cr0 = 0u;
        status = test_core_machine_fixture_reset_real_mode(machine) ?
            TYPE_STATUS_OK : TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
        0x100u, gdt_pointer, sizeof(gdt_pointer));
    if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
        0x300u, gdt, sizeof(gdt));
    if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
        0u, boot, sizeof(boot));
    if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
        0x2000u, halt, sizeof(halt));
    if (status == TYPE_STATUS_OK && (core_machine_run(machine,
            (core_machine_run_budget){64u, 0u}, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT)) {
        status = TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
        0x2000u, program, bytes);
    if (STD_STRCMP(key_id, "I286-SYSTEM-LLDT-R") == 0 ||
        STD_STRCMP(key_id, "I286-SYSTEM-LLDT-M") == 0 ||
        STD_STRCMP(key_id, "I286-SYSTEM-LLDT-M-EA-BID") == 0 ||
        STD_STRCMP(key_id, "I286-SYSTEM-LLDT-M-SEGMENT") == 0) {
        operand = 0x0028u;
    } else if (STD_STRCMP(key_id, "I286-SYSTEM-LTR-R") == 0 ||
        STD_STRCMP(key_id, "I286-SYSTEM-LTR-M") == 0 ||
        STD_STRCMP(key_id, "I286-SYSTEM-LTR-M-EA-BID") == 0 ||
        STD_STRCMP(key_id, "I286-SYSTEM-LTR-M-SEGMENT") == 0) {
        operand = 0x0030u;
    }
    if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
        0x4000u, &operand, sizeof(operand));
    if (status == TYPE_STATUS_OK && (STD_STRCMP(key_id,
            "I286-LDS-M-PM") == 0 || STD_STRCMP(key_id,
            "I286-LES-M-PM") == 0 || STD_STRCMP(key_id,
            "I286-LDS-M-PM-SEGMENT") == 0 || STD_STRCMP(key_id,
            "I286-LES-M-PM-SEGMENT") == 0 || STD_STRCMP(key_id,
            "I286-LDS-M-PM-EA-BID") == 0 || STD_STRCMP(key_id,
            "I286-LES-M-PM-EA-BID") == 0)) {
        const type_unsigned_16 pointer[] = { 1u, 0x0010u };

        status = core_machine_memory_write(machine, 0x4000u, pointer,
            sizeof(pointer));
    }
    if (status == TYPE_STATUS_OK && (STD_STRCMP(key_id,
            "I286-LDS-M-PM-EA-BID-ODD-WORD") == 0 || STD_STRCMP(key_id,
            "I286-LES-M-PM-EA-BID-ODD-WORD") == 0)) {
        const type_unsigned_16 pointer[] = { 1u, 0x0010u };

        status = core_machine_memory_write(machine, 0x4001u, pointer,
            sizeof(pointer));
    }
    if (status == TYPE_STATUS_OK && (STD_STRCMP(key_id,
            "I286-LDS-M-PM-ODD-WORD") == 0 || STD_STRCMP(key_id,
            "I286-LES-M-PM-ODD-WORD") == 0)) {
        const type_unsigned_16 pointer[] = { 1u, 0x0010u };

        status = core_machine_memory_write(machine, 0x4001u, pointer,
            sizeof(pointer));
    }
    if (status == TYPE_STATUS_OK && STD_STRCMP(key_id,
            "I286-MOV-SREG-LOAD-PM-ODD-WORD") == 0) {
        const type_unsigned_16 selector = 0x0010u;

        status = core_machine_memory_write(machine, 0x4001u, &selector,
            sizeof(selector));
    }
    if (status == TYPE_STATUS_OK && STD_STRCMP(key_id,
            "I286-MOV-SREG-LOAD-PM-EA-BID-ODD-WORD") == 0) {
        const type_unsigned_16 selector = 0x0010u;

        status = core_machine_memory_write(machine, 0x4001u, &selector,
            sizeof(selector));
    }
    if (status == TYPE_STATUS_OK && (STD_STRCMP(key_id,
            "I286-SYSTEM-LGDT-M-EA-BID") == 0 || STD_STRCMP(key_id,
            "I286-SYSTEM-LIDT-M-EA-BID") == 0 || STD_STRCMP(key_id,
            "I286-SYSTEM-LGDT-M-SEGMENT") == 0 || STD_STRCMP(key_id,
            "I286-SYSTEM-LIDT-M-SEGMENT") == 0)) {
        status = core_machine_memory_write(machine, 0x4000u, table_pointer,
            sizeof(table_pointer));
    }
    if (status == TYPE_STATUS_OK && STD_STRCMP(key_id,
            "I286-STACK-POP-SEG-PM") == 0) {
        const type_unsigned_16 selector = 0x0010u;

        machine->executor_cpu.data.sp = 0x1000u;
        status = core_machine_memory_write(machine, 0x4000u, &selector,
            sizeof(selector));
    }
    if (status == TYPE_STATUS_OK && STD_STRCMP(key_id,
            "I286-RET-FAR-SAME-NEXT-BYTE-2") == 0) {
        const type_unsigned_16 return_frame[] = { 0x0010u, 0x0008u };
        const type_unsigned_8 target[] = { 0x00u, 0xc0u };

        machine->executor_cpu.data.sp = 0x1000u;
        status = core_machine_memory_write(machine, 0x4000u, return_frame,
            sizeof(return_frame));
        if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
            0x2010u, target, sizeof(target));
    }
    if (status == TYPE_STATUS_OK && STD_STRCMP(key_id,
            "I286-RET-IRET-NORMAL-NEXT-BYTE-2") == 0) {
        const type_unsigned_16 return_frame[] = { 0x0010u, 0x0008u, 0x0002u };
        const type_unsigned_8 target[] = { 0x00u, 0xc0u };

        machine->executor_cpu.data.sp = 0x1000u;
        status = core_machine_memory_write(machine, 0x4000u, return_frame,
            sizeof(return_frame));
        if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
            0x2010u, target, sizeof(target));
    }
    if (status == TYPE_STATUS_OK && STD_STRCMP(key_id,
            "I286-CALL-FAR-DIRECT-PM-NEXT-BYTE-2") == 0) {
        const type_unsigned_8 target[] = { 0x00u, 0xc0u };

        machine->executor_cpu.data.sp = 0x1000u;
        status = core_machine_memory_write(machine, 0x2010u, target,
            sizeof(target));
    }
    if (status == TYPE_STATUS_OK && STD_STRCMP(key_id,
            "I286-JMP-FAR-PM-NEXT-BYTE-2") == 0) {
        const type_unsigned_16 pointer[] = { 0x0010u, 0x0008u };
        const type_unsigned_8 target[] = { 0x00u, 0xc0u };

        status = core_machine_memory_write(machine, 0x4000u, pointer,
            sizeof(pointer));
        if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
            0x2010u, target,
            sizeof(target));
    }
    if (status == TYPE_STATUS_OK && STD_STRCMP(key_id,
            "I286-CALL-FAR-M-PM-NEXT-BYTE-2") == 0) {
        const type_unsigned_16 pointer[] = { 0x0010u, 0x0008u };
        const type_unsigned_8 target[] = { 0x00u, 0xc0u };

        machine->executor_cpu.data.sp = 0x1000u;
        status = core_machine_memory_write(machine, 0x4000u, pointer,
            sizeof(pointer));
        if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
            0x2010u, target, sizeof(target));
    }
    if (status == TYPE_STATUS_OK) {
        machine->executor_cpu.data.eax = operand;
        machine->executor_cpu.data.ebp = 0x0800u;
        machine->executor_cpu.data.esi = 0x0800u;
        machine->elapsed_ticks = 0u;
        test_core_machine_fixture_resume_after_halt_at(machine, 0u);
    }
    if (status == TYPE_STATUS_OK) status =
        core_machine_set_retirement_observation_provider(machine, &provider);
    if (status != TYPE_STATUS_OK) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static C_INT timing_80286_manifest_run(
    const timing_80286_manifest_recipe *recipe)
{
    const core_machine_run_budget budget = { 1u, 0u };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    const timing_80286_manifest_record *record;
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_INT failed;

    if (recipe == STD_NULL) return 1;
    record = timing_80286_manifest_find(recipe->key_id);
    failed = record == STD_NULL || !timing_80286_manifest_is_i286(record) ||
        STD_STRCMP(record->profile, "80286") != 0 ||
        !timing_80286_manifest_prepare(&machine, &capture, recipe->key_id,
            recipe->program, recipe->bytes);
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != recipe->ticks || capture.count != 1u ||
            capture.observation.source_ticks != recipe->ticks ||
            capture.observation.timing_origin != recipe->origin ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    if (failed) {
        STD_PRINTF("M5:T435:S10:I286-MANIFEST-DETAIL:%s:expected=%llu:run=%llu:source=%llu:count=%u:origin=%u:disposition=%u\n",
            recipe->key_id, recipe->ticks, run.ticks,
            capture.observation.source_ticks, capture.count,
            (type_unsigned_32)capture.observation.timing_origin,
            (type_unsigned_32)capture.observation.timing_disposition);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_repeat_step(core_machine *machine,
    timing_80286_manifest_capture *capture, const C_CHAR *key_id,
    core_machine_retirement_repeat_phase expected_phase,
    type_unsigned_64 expected_ticks, type_unsigned_32 required_inputs)
{
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result run = { 0 };

    if (machine == STD_NULL || capture == STD_NULL || key_id == STD_NULL ||
        timing_80286_manifest_find(key_id) == STD_NULL) return 1;
    capture->count = 0u;
    STD_MEMSET(&capture->observation, 0, sizeof(capture->observation));
    return core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
        run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
        run.ticks != expected_ticks || capture->count != 1u ||
        capture->observation.source_ticks != expected_ticks ||
        capture->observation.timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO ||
        capture->observation.timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        capture->observation.repeat_phase != expected_phase ||
        (capture->observation.formula_inputs &
            (CORE_MACHINE_CPU_TIMING_INPUT_REPEAT |
             CORE_MACHINE_CPU_TIMING_INPUT_REPEAT_PHASE | required_inputs)) !=
            (CORE_MACHINE_CPU_TIMING_INPUT_REPEAT |
             CORE_MACHINE_CPU_TIMING_INPUT_REPEAT_PHASE | required_inputs);
}

static C_INT timing_80286_manifest_run_repeat_recipe(
    const timing_80286_manifest_repeat_recipe *recipe, C_INT odd_word)
{
    type_unsigned_8 program[2] = { 0u, 0u };
    const type_unsigned_16 source = 1u;
    type_unsigned_16 destination;
    C_CHAR first_key[96];
    C_CHAR continuation_key[96];
    C_CHAR zero_key[96];
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine *machine = STD_NULL;
    type_unsigned_32 required_inputs = odd_word ?
        CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD : 0u;
    type_unsigned_64 odd_ticks = odd_word ? 2u : 0u;
    C_INT source_odd;
    C_INT failed;

    if (recipe == STD_NULL) return 1;
    program[0] = recipe->prefix;
    program[1] = recipe->opcode;
    source_odd = odd_word && (recipe->opcode == 0xa5u ||
        recipe->opcode == 0xa7u || recipe->opcode == 0xadu ||
        recipe->opcode == 0x6fu);
    if (STD_SNPRINTF(first_key, sizeof(first_key), "%s%s-REP-PHASE-FIRST",
            recipe->key_id, odd_word ? "-ODD-WORD" : "") < 0 ||
        STD_SNPRINTF(continuation_key, sizeof(continuation_key),
            "%s%s-REP-PHASE-CONTINUE", recipe->key_id,
            odd_word ? "-ODD-WORD" : "") < 0 ||
        STD_SNPRINTF(zero_key, sizeof(zero_key), "%s%s-REP-PHASE-ZERO",
            recipe->key_id, odd_word ? "-ODD-WORD" : "") < 0) return 1;
    failed = !timing_80286_manifest_prepare(&machine, &capture, first_key,
        program, sizeof(program));
    if (!failed) {
        machine->executor_cpu.data.es.base = machine->executor_cpu.data.ds.base;
        machine->executor_cpu.data.es.selector = machine->executor_cpu.data.ds.selector;
        machine->executor_cpu.data.si = source_odd ? 0x1001u : 0x1000u;
        machine->executor_cpu.data.di = source_odd ? 0x1100u :
            odd_word ? 0x1101u : 0x1100u;
        machine->executor_cpu.data.ax = 1u;
        machine->executor_cpu.data.edx = 0x0080u;
        machine->executor_cpu.data.cx = 2u;
        destination = recipe->prefix == 0xf3u ? source : 0u;
        failed = core_machine_memory_write(machine, source_odd ? 0x1001u : 0x1000u,
            &source, sizeof(source)) != TYPE_STATUS_OK ||
            core_machine_memory_write(machine, source_odd ? 0x1100u :
                odd_word ? 0x1101u : 0x1100u, &destination,
                sizeof(destination)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = timing_80286_manifest_run_repeat_step(machine, &capture,
            first_key, CORE_MACHINE_RETIREMENT_REPEAT_FIRST,
            recipe->first_ticks + odd_ticks, required_inputs);
        if (failed) STD_PRINTF("M5:T435:S10:I286-REP-DETAIL:%s:expected=%llu:observed=%llu:phase=%u:inputs=%u\n",
            first_key, recipe->first_ticks + odd_ticks,
            capture.observation.source_ticks,
            (type_unsigned_32)capture.observation.repeat_phase,
            capture.observation.formula_inputs);
        if (!failed) {
            failed = timing_80286_manifest_run_repeat_step(machine, &capture,
                continuation_key, CORE_MACHINE_RETIREMENT_REPEAT_CONTINUATION,
                recipe->continuation_ticks + odd_ticks, required_inputs);
            if (failed) STD_PRINTF("M5:T435:S10:I286-REP-DETAIL:%s:expected=%llu:observed=%llu:phase=%u:inputs=%u\n",
                continuation_key, recipe->continuation_ticks + odd_ticks,
                capture.observation.source_ticks,
                (type_unsigned_32)capture.observation.repeat_phase,
                capture.observation.formula_inputs);
        }
    }
    core_machine_destroy(machine);
    machine = STD_NULL;
    if (!failed) failed = !timing_80286_manifest_prepare(&machine, &capture,
        zero_key, program, sizeof(program));
    if (!failed) {
        machine->executor_cpu.data.edx = 0x0080u;
        machine->executor_cpu.data.cx = 0u;
        failed = timing_80286_manifest_run_repeat_step(machine, &capture, zero_key,
            CORE_MACHINE_RETIREMENT_REPEAT_ZERO_COUNT, recipe->zero_ticks,
            0u);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_string_recipe(
    const timing_80286_manifest_recipe *recipe, C_INT odd_word)
{
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_16 value = 1u;
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_CHAR key_id[96];
    type_unsigned_64 expected_ticks;
    C_INT source_odd;
    C_INT failed;

    if (recipe == STD_NULL) return 1;
    if (STD_SNPRINTF(key_id, sizeof(key_id), "%s%s", recipe->key_id,
            odd_word ? "-ODD-WORD" : "") < 0) return 1;
    expected_ticks = recipe->ticks + (odd_word ? 2u : 0u);
    source_odd = odd_word && (recipe->program[0] == 0xa5u ||
        recipe->program[0] == 0xa7u || recipe->program[0] == 0xadu ||
        recipe->program[0] == 0x6fu);
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !timing_80286_manifest_prepare(&machine, &capture, key_id,
            recipe->program, recipe->bytes);
    if (!failed) {
        machine->executor_cpu.data.es.base = machine->executor_cpu.data.ds.base;
        machine->executor_cpu.data.es.selector = machine->executor_cpu.data.ds.selector;
        machine->executor_cpu.data.si = source_odd ? 0x1001u : 0x1000u;
        machine->executor_cpu.data.di = source_odd ? 0x1100u :
            odd_word ? 0x1101u : 0x1100u;
        machine->executor_cpu.data.ax = value;
        machine->executor_cpu.data.edx = 0x0080u;
        failed = core_machine_memory_write(machine, source_odd ? 0x1001u : 0x1000u,
            &value, sizeof(value)) != TYPE_STATUS_OK || core_machine_memory_write(
            machine, source_odd ? 0x1100u : odd_word ? 0x1101u : 0x1100u,
            &value, sizeof(value)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != expected_ticks || capture.count != 1u ||
            capture.observation.source_ticks != expected_ticks ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            (odd_word && (capture.observation.formula_inputs &
                CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD) == 0u);
    }
    if (failed) STD_PRINTF("M5:T435:S10:I286-STRING-DETAIL:%s:expected=%llu:run=%llu:source=%llu:count=%u\n",
        key_id, expected_ticks, run.ticks,
        capture.observation.source_ticks, capture.count);
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_hlt_recipe(C_VOID)
{
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0xf4u };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_INT failed;

    failed = timing_80286_manifest_find("I286-HLT") == STD_NULL ||
        !timing_80286_manifest_prepare(&machine, &capture, "I286-HLT",
            program, sizeof(program));
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            run.executed != 1u || run.ticks != 2u || capture.count != 1u ||
            capture.observation.source_ticks != 2u ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    if (failed) STD_PRINTF("M5:T435:S10:I286-HLT-DETAIL:run=%llu:source=%llu:count=%u:reason=%u\n",
        run.ticks, capture.observation.source_ticks, capture.count,
        (type_unsigned_32)run.reason);
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_lock_ea_recipe(
    const timing_80286_manifest_recipe *base_recipe)
{
    timing_80286_manifest_recipe recipe;
    C_CHAR key_id[96];

    if (base_recipe == STD_NULL || base_recipe->bytes < 5u ||
        STD_SNPRINTF(key_id, sizeof(key_id), "%s-EA-BID", base_recipe->key_id) < 0 ||
        timing_80286_manifest_find(key_id) == STD_NULL) return 1;
    recipe = *base_recipe;
    /* All LOCK base recipes are direct disp16 memory forms.  Re-select the
     * same reg/extension with 16-bit [BP+SI+disp16] addressing at 0x1000. */
    recipe.program[2] = (type_unsigned_8)((recipe.program[2] & 0x38u) | 0x82u);
    recipe.program[3] = 0u;
    recipe.program[4] = 0u;
    recipe.key_id = key_id;
    recipe.ticks += 1u;
    return timing_80286_manifest_run(&recipe);
}

static C_INT timing_80286_manifest_run_repeat_base_recipe(
    const timing_80286_manifest_repeat_recipe *recipe, C_INT odd_word)
{
    const core_machine_run_budget budget = { 1u, 0u };
    type_unsigned_8 program[2];
    const type_unsigned_16 value = 1u;
    C_CHAR key_id[96];
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_INT source_odd;
    C_INT failed;

    if (recipe == STD_NULL || STD_SNPRINTF(key_id, sizeof(key_id), "%s%s",
            recipe->key_id, odd_word ? "-ODD-WORD" : "") < 0) return 1;
    program[0] = recipe->prefix;
    program[1] = recipe->opcode;
    source_odd = odd_word && (recipe->opcode == 0xa5u || recipe->opcode == 0xa7u ||
        recipe->opcode == 0xadu || recipe->opcode == 0x6fu);
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !timing_80286_manifest_prepare(&machine, &capture, key_id,
            program, sizeof(program));
    if (!failed) {
        machine->executor_cpu.data.es.base = machine->executor_cpu.data.ds.base;
        machine->executor_cpu.data.es.selector = machine->executor_cpu.data.ds.selector;
        machine->executor_cpu.data.si = source_odd ? 0x1001u : 0x1000u;
        machine->executor_cpu.data.di = source_odd ? 0x1100u :
            odd_word ? 0x1101u : 0x1100u;
        machine->executor_cpu.data.ax = value;
        machine->executor_cpu.data.edx = 0x0080u;
        machine->executor_cpu.data.cx = 1u;
        failed = core_machine_memory_write(machine, source_odd ? 0x1001u : 0x1000u,
            &value, sizeof(value)) != TYPE_STATUS_OK ||
            core_machine_memory_write(machine, source_odd ? 0x1100u :
                odd_word ? 0x1101u : 0x1100u,
                &value, sizeof(value)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != recipe->first_ticks + (odd_word ? 2u : 0u) ||
            capture.count != 1u ||
            capture.observation.source_ticks != recipe->first_ticks +
                (odd_word ? 2u : 0u) ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            (odd_word && (capture.observation.formula_inputs &
                CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD) == 0u);
    }
    if (failed) STD_PRINTF("M5:T435:S10:I286-REP-ODD-DETAIL:%s:expected=%llu:run=%llu:source=%llu:inputs=%u\n",
            key_id, recipe->first_ticks + (odd_word ? 2u : 0u), run.ticks,
        capture.observation.source_ticks, capture.observation.formula_inputs);
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_jmp_next_byte_recipe(
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u },
        { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u },
        { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u },
        { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    timing_80286_manifest_recipe recipe = {
        STD_NULL, { 0xebu, 0u, 0u, 0u, 0u, 0u, 0u, 0u }, 0u, 0u,
        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK
    };
    C_CHAR key_id[96];
    STD_SIZE_T index;

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id), "I286-JMP-NEAR-NEXT-BYTE-%u",
            (type_unsigned_32)next_bytes) < 0) return 1;
    recipe.key_id = key_id;
    recipe.bytes = (STD_SIZE_T)next_bytes + 2u;
    recipe.ticks = 7u + next_bytes;
    for (index = 0u; index < next_bytes; ++index) {
        recipe.program[index + 2u] = target[next_bytes][index];
    }
    return timing_80286_manifest_run(&recipe);
}

static C_INT timing_80286_manifest_run_call_next_byte_recipe(
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u },
        { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u },
        { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u },
        { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    timing_80286_manifest_recipe recipe = {
        STD_NULL, { 0xe8u, 0u, 0u, 0u, 0u, 0u, 0u, 0u }, 0u, 0u,
        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK
    };
    C_CHAR key_id[96];
    STD_SIZE_T index;

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id),
            "I286-CALL-NEAR-DIRECT-NEXT-BYTE-%u",
            (type_unsigned_32)next_bytes) < 0) return 1;
    recipe.key_id = key_id;
    recipe.bytes = (STD_SIZE_T)next_bytes + 3u;
    recipe.ticks = 7u + next_bytes;
    for (index = 0u; index < next_bytes; ++index) {
        recipe.program[index + 3u] = target[next_bytes][index];
    }
    return timing_80286_manifest_run(&recipe);
}

static C_INT timing_80286_manifest_run_ret_next_byte_recipe(
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u },
        { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u },
        { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u },
        { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0xc3u };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_CHAR key_id[96];
    C_INT failed;

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id), "I286-RET-NEAR-NEXT-BYTE-%u",
            (type_unsigned_32)next_bytes) < 0) return 1;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !timing_80286_manifest_prepare(&machine, &capture, key_id,
            program, sizeof(program));
    if (!failed) failed = core_machine_memory_write(machine, 0x000ffff5u,
        target[next_bytes], next_bytes) != TYPE_STATUS_OK;
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 11u + next_bytes || capture.count != 1u ||
            capture.observation.source_ticks != 11u + next_bytes ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_far_jmp_next_byte_recipe(
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u },
        { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u },
        { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u },
        { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0xeau, 0xf5u, 0xffu, 0u, 0xf0u };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_CHAR key_id[96];
    C_INT failed;

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id),
            "I286-JMP-FAR-REAL-NEXT-BYTE-%u",
            (type_unsigned_32)next_bytes) < 0) return 1;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !timing_80286_manifest_prepare(&machine, &capture, key_id,
            program, sizeof(program));
    if (!failed) failed = core_machine_memory_write(machine, 0x000ffff5u,
        target[next_bytes], next_bytes) != TYPE_STATUS_OK;
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 11u + next_bytes || capture.count != 1u ||
            capture.observation.source_ticks != 11u + next_bytes ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_ret_imm_next_byte_recipe(
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u },
        { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u },
        { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u },
        { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0xc2u, 0u, 0u };
    const type_unsigned_16 return_ip = 0xfff5u;
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_CHAR key_id[96];
    C_INT failed;

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id), "I286-RET-NEAR-IMM-NEXT-BYTE-%u",
            (type_unsigned_32)next_bytes) < 0) return 1;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !timing_80286_manifest_prepare(&machine, &capture, key_id,
            program, sizeof(program));
    if (!failed) {
        machine->executor_cpu.data.sp = TIMING_80286_MANIFEST_STACK_LINEAR;
        failed = core_machine_memory_write(machine, TIMING_80286_MANIFEST_STACK_LINEAR,
            &return_ip, sizeof(return_ip)) != TYPE_STATUS_OK ||
            core_machine_memory_write(machine, 0x000ffff5u, target[next_bytes],
                next_bytes) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 11u + next_bytes || capture.count != 1u ||
            capture.observation.source_ticks != 11u + next_bytes ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_interrupt_next_byte_recipe(
    C_INT immediate, type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u },
        { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u },
        { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u },
        { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 int3_program[] = { 0xccu };
    const type_unsigned_8 immediate_program[] = { 0xcdu, 0x60u };
    const type_unsigned_8 *program = immediate ? immediate_program : int3_program;
    STD_SIZE_T program_bytes = immediate ? sizeof(immediate_program) :
        sizeof(int3_program);
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_CHAR key_id[96];
    C_INT failed;

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id), "I286-%s-NEXT-BYTE-%u",
            immediate ? "INT-IMM-REAL" : "INT3-REAL",
            (type_unsigned_32)next_bytes) < 0) return 1;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !timing_80286_manifest_prepare(&machine, &capture, key_id,
            program, program_bytes);
    if (!failed) failed = core_machine_memory_write(machine, 0x000ffff5u,
        target[next_bytes], next_bytes) != TYPE_STATUS_OK;
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 23u + next_bytes || capture.count != 1u ||
            capture.observation.source_ticks != 23u + next_bytes ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_into_next_byte_recipe(C_INT taken,
    type_unsigned_8 next_bytes, const C_CHAR *key_id)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u },
        { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u },
        { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u },
        { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    type_unsigned_8 program[7] = { 0xceu };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_INT failed;

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        key_id == STD_NULL) return 1;
    if (!taken) {
        STD_SIZE_T index;

        for (index = 0u; index < next_bytes; ++index) {
            program[1u + index] = target[next_bytes][index];
        }
    }
    timing_80286_manifest_eflags = taken ? VCPU_EFLAGS_OF : 0u;
    timing_80286_manifest_flags_active = 1;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !timing_80286_manifest_prepare(&machine, &capture, key_id,
            program, taken ? 1u : 1u + next_bytes);
    if (!failed && taken) failed = core_machine_memory_write(machine, 0x000ffff5u,
        target[next_bytes], next_bytes) != TYPE_STATUS_OK;
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != (taken ? 24u : 3u) + next_bytes || capture.count != 1u ||
            capture.observation.source_ticks !=
                (taken ? 24u : 3u) + next_bytes ||
            machine->executor_cpu.data.eip != (taken ? 0xfff5u : 0xfff1u) ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    if (failed) {
        STD_PRINTF("M5:T436:S5:I286-INTO-DETAIL:%s:taken=%u:bytes=%u:run=%llu:source=%llu:count=%u:origin=%u:disposition=%u\n",
            key_id, (type_unsigned_32)taken, (type_unsigned_32)next_bytes,
            run.ticks, capture.observation.source_ticks, capture.count,
            (type_unsigned_32)capture.observation.timing_origin,
            (type_unsigned_32)capture.observation.timing_disposition);
    }
    timing_80286_manifest_flags_active = 0;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_indirect_jmp_next_byte_recipe(
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u },
        { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u },
        { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u },
        { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0xffu, 0x26u, 0u, 0x10u };
    const type_unsigned_16 target_ip = 0xfff5u;
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_CHAR key_id[96];
    C_INT failed;

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id), "I286-JMP-RM-NEXT-BYTE-%u",
            (type_unsigned_32)next_bytes) < 0) return 1;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !timing_80286_manifest_prepare(&machine, &capture, key_id,
            program, sizeof(program));
    if (!failed) failed = core_machine_memory_write(machine, 0x1000u,
        &target_ip, sizeof(target_ip)) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0x000ffff5u, target[next_bytes],
            next_bytes) != TYPE_STATUS_OK;
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 11u + next_bytes || capture.count != 1u ||
            capture.observation.source_ticks != 11u + next_bytes ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_indirect_call_next_byte_recipe(
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u },
        { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u },
        { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u },
        { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0xffu, 0x16u, 0u, 0x10u };
    const type_unsigned_16 target_ip = 0xfff5u;
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_CHAR key_id[96];
    C_INT failed;

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id), "I286-CALL-NEAR-RM-NEXT-BYTE-%u",
            (type_unsigned_32)next_bytes) < 0) return 1;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !timing_80286_manifest_prepare(&machine, &capture, key_id,
            program, sizeof(program));
    if (!failed) {
        machine->executor_cpu.data.sp = TIMING_80286_MANIFEST_STACK_LINEAR +
            TIMING_80286_MANIFEST_STACK_BYTES;
        failed = core_machine_memory_write(machine, 0x1000u, &target_ip,
            sizeof(target_ip)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
            0x000ffff5u, target[next_bytes], next_bytes) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 11u + next_bytes || capture.count != 1u ||
            capture.observation.source_ticks != 11u + next_bytes ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_far_call_next_byte_recipe(
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u },
        { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u },
        { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u },
        { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0x9au, 0xf5u, 0xffu, 0u, 0xf0u };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_CHAR key_id[96];
    C_INT failed;

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id),
            "I286-CALL-FAR-DIRECT-REAL-NEXT-BYTE-%u",
            (type_unsigned_32)next_bytes) < 0) return 1;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !timing_80286_manifest_prepare(&machine, &capture, key_id,
            program, sizeof(program));
    if (!failed) {
        machine->executor_cpu.data.sp = TIMING_80286_MANIFEST_STACK_LINEAR +
            TIMING_80286_MANIFEST_STACK_BYTES;
        failed = core_machine_memory_write(machine, 0x000ffff5u,
            target[next_bytes], next_bytes) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 13u + next_bytes || capture.count != 1u ||
            capture.observation.source_ticks != 13u + next_bytes ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_far_indirect_call_next_byte_recipe(
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u },
        { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u },
        { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u },
        { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    const type_unsigned_8 far_pointer[] = { 0xf5u, 0xffu, 0u, 0xf0u };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0xffu, 0x1eu, 0u, 0x10u };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_CHAR key_id[96];
    C_INT failed;

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id),
            "I286-CALL-FAR-M-REAL-NEXT-BYTE-%u",
            (type_unsigned_32)next_bytes) < 0) return 1;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !timing_80286_manifest_prepare(&machine, &capture, key_id,
            program, sizeof(program));
    if (!failed) {
        machine->executor_cpu.data.sp = TIMING_80286_MANIFEST_STACK_LINEAR +
            TIMING_80286_MANIFEST_STACK_BYTES;
        failed = core_machine_memory_write(machine, 0x1000u, far_pointer,
            sizeof(far_pointer)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
            0x000ffff5u, target[next_bytes], next_bytes) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 16u + next_bytes || capture.count != 1u ||
            capture.observation.source_ticks != 16u + next_bytes ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_far_ret_next_byte_recipe(
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u },
        { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u },
        { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u },
        { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    const type_unsigned_16 return_frame[] = { 0xfff5u, 0xf000u };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0xcbu };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_CHAR key_id[96];
    C_INT failed;

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id),
            "I286-RET-FAR-SAME-NEXT-BYTE-%u",
            (type_unsigned_32)next_bytes) < 0) return 1;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !timing_80286_manifest_prepare(&machine, &capture, key_id,
            program, sizeof(program));
    if (!failed) {
        machine->executor_cpu.data.sp = TIMING_80286_MANIFEST_STACK_LINEAR;
        failed = core_machine_memory_write(machine,
            TIMING_80286_MANIFEST_STACK_LINEAR, return_frame,
            sizeof(return_frame)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
            0x000ffff5u, target[next_bytes], next_bytes) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 15u + next_bytes || capture.count != 1u ||
            capture.observation.source_ticks != 15u + next_bytes ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_iret_next_byte_recipe(
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u },
        { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u },
        { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u },
        { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    const type_unsigned_16 return_frame[] = { 0xfff5u, 0xf000u, 0x0002u };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0xcfu };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_CHAR key_id[96];
    C_INT failed;

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id),
            "I286-RET-IRET-NORMAL-NEXT-BYTE-%u",
            (type_unsigned_32)next_bytes) < 0) return 1;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !timing_80286_manifest_prepare(&machine, &capture, key_id,
            program, sizeof(program));
    if (!failed) {
        machine->executor_cpu.data.sp = TIMING_80286_MANIFEST_STACK_LINEAR;
        failed = core_machine_memory_write(machine,
            TIMING_80286_MANIFEST_STACK_LINEAR, return_frame,
            sizeof(return_frame)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
            0x000ffff5u, target[next_bytes], next_bytes) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 17u + next_bytes || capture.count != 1u ||
            capture.observation.source_ticks != 17u + next_bytes ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_protected_far_memory_call_next_byte_recipe(
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u },
        { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u },
        { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u },
        { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    const type_unsigned_16 pointer[] = { 0x0010u, 0x0008u };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0xffu,0x1eu,0u,0x10u };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_CHAR key_id[96];
    C_INT failed;

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id),
            "I286-CALL-FAR-M-PM-NEXT-BYTE-%u",
            (type_unsigned_32)next_bytes) < 0) return 1;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !timing_80286_manifest_prepare_protected_system(&machine, &capture,
            key_id, program, sizeof(program));
    if (!failed) {
        machine->executor_cpu.data.sp = 0x1000u;
        failed = core_machine_memory_write(machine, 0x4000u, pointer,
            sizeof(pointer)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
            0x2010u, target[next_bytes], next_bytes) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 29u + next_bytes || capture.count != 1u ||
            capture.observation.source_ticks != 29u + next_bytes ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_protected_far_call_next_byte_recipe(
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u },
        { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u },
        { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u },
        { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0x9au,0x10u,0u,0x08u,0u };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_CHAR key_id[96];
    C_INT failed;

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id),
            "I286-CALL-FAR-DIRECT-PM-NEXT-BYTE-%u",
            (type_unsigned_32)next_bytes) < 0) return 1;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !timing_80286_manifest_prepare_protected_system(&machine, &capture,
            key_id, program, sizeof(program));
    if (!failed) {
        machine->executor_cpu.data.sp = 0x1000u;
        failed = core_machine_memory_write(machine, 0x2010u,
            target[next_bytes], next_bytes) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 26u + next_bytes || capture.count != 1u ||
            capture.observation.source_ticks != 26u + next_bytes ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_protected_far_memory_jmp_next_byte_recipe(
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u },
        { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u },
        { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u },
        { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    const type_unsigned_16 pointer[] = { 0x0010u, 0x0008u };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0xffu,0x2eu,0u,0x10u };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_CHAR key_id[96];
    C_INT failed;

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id),
            "I286-JMP-FAR-PM-NEXT-BYTE-%u",
            (type_unsigned_32)next_bytes) < 0) return 1;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !timing_80286_manifest_prepare_protected_system(&machine, &capture,
            key_id, program, sizeof(program));
    if (!failed) {
        failed = core_machine_memory_write(machine, 0x4000u, pointer,
            sizeof(pointer)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
            0x2010u, target[next_bytes], next_bytes) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 26u + next_bytes || capture.count != 1u ||
            capture.observation.source_ticks != 26u + next_bytes ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_protected_far_ret_next_byte_recipe(
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u },
        { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u },
        { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u },
        { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    const type_unsigned_16 return_frame[] = { 0x0010u, 0x0008u };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0xcbu };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_CHAR key_id[96];
    C_INT failed;

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id),
            "I286-RET-FAR-SAME-NEXT-BYTE-%u",
            (type_unsigned_32)next_bytes) < 0) return 1;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !timing_80286_manifest_prepare_protected_system(&machine, &capture,
            key_id, program, sizeof(program));
    if (!failed) {
        machine->executor_cpu.data.sp = 0x1000u;
        failed = core_machine_memory_write(machine, 0x4000u, return_frame,
            sizeof(return_frame)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
            0x2010u, target[next_bytes], next_bytes) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 25u + next_bytes || capture.count != 1u ||
            capture.observation.source_ticks != 25u + next_bytes ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_protected_iret_next_byte_recipe(
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u },
        { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u },
        { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u },
        { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    const type_unsigned_16 return_frame[] = { 0x0010u, 0x0008u, 0x0002u };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0xcfu };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_CHAR key_id[96];
    C_INT failed;

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id),
            "I286-RET-IRET-NORMAL-NEXT-BYTE-%u",
            (type_unsigned_32)next_bytes) < 0) return 1;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !timing_80286_manifest_prepare_protected_system(&machine, &capture,
            key_id, program, sizeof(program));
    if (!failed) {
        machine->executor_cpu.data.sp = 0x1000u;
        failed = core_machine_memory_write(machine, 0x4000u, return_frame,
            sizeof(return_frame)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
            0x2010u, target[next_bytes], next_bytes) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 31u + next_bytes || capture.count != 1u ||
            capture.observation.source_ticks != 31u + next_bytes ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_protected_outer_ret_recipe(
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u }, { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u }, { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u }, { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    static const type_unsigned_8 user_data[] = {
        0xffu,0xffu,0,0,0,0xf2u,0,0
    };
    static const type_unsigned_8 retf[] = { 0xcbu };
    static const type_unsigned_16 frame[] = { 0x0010u,0x001bu,0x4000u,0x0023u };
    const core_machine_run_budget budget = { 1u, 0u };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider capture_provider = {
        timing_80286_manifest_capture_retirement, &capture
    };
    const timing_80286_manifest_record *record;
    core_machine_run_result run = { 0 };
    s3_gate_machine state;
    C_INT failed;

    C_CHAR key_id[96];

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id), "I286-RET-FAR-LESS-NEXT-BYTE-%u",
            (type_unsigned_32)next_bytes) < 0) return 1;
    record = timing_80286_manifest_find(key_id);
    failed = record == STD_NULL || !timing_80286_manifest_is_i286(record) ||
        !s3_gate_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286, TYPE_FALSE,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE);
    if (!failed) {
        failed = !s3_gate_write(&state, S3_GDT_BASE + 32u, user_data,
            sizeof(user_data)) || !s3_gate_write(&state, S3_CODE_BASE, retf,
            sizeof(retf)) || !s3_gate_write(&state, S3_CODE_BASE + 0x10u,
            target[next_bytes], next_bytes) || !s3_gate_write(&state, S3_STACK_TOP,
            frame, sizeof(frame));
        state.machine->executor_cpu.data.gdtr.limit = 39u;
        state.machine->executor_cpu.data.esp = 0x12348000u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
        if (!failed) failed = core_machine_set_retirement_observation_provider(
            state.machine, &capture_provider) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = core_machine_run(state.machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 55u + next_bytes || capture.count != 1u ||
            capture.observation.source_ticks != 55u + next_bytes ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT timing_80286_manifest_run_protected_outer_iret_recipe(
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        {0,0,0,0,0,0},{0x90,0,0,0,0,0},{0,0xc0,0,0,0,0},
        {0x80,0xc0,1,0,0,0},{0xc6,0x46,0,1,0,0},
        {0xc6,6,0,0x10,1,0},{0xc7,6,0,0x10,1,0}
    };
    static const type_unsigned_8 user_data[] = {
        0xffu,0xffu,0,0,0,0xf2u,0,0
    };
    static const type_unsigned_8 iret[] = { 0xcfu };
    static const type_unsigned_16 frame[] = {
        0x0010u,0x001bu,VCPU_EFLAGS_CF,0x4000u,0x0023u
    };
    const core_machine_run_budget budget = { 1u, 0u };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80286_manifest_capture_retirement, &capture
    };
    const timing_80286_manifest_record *record;
    core_machine_run_result run = { 0 };
    s3_gate_machine state;
    C_INT failed;

    C_CHAR key_id[96];

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0]) ||
        STD_SNPRINTF(key_id, sizeof(key_id),
            "I286-RET-IRET-PRIVILEGE-NEXT-BYTE-%u",
            (type_unsigned_32)next_bytes) < 0) return 1;
    record = timing_80286_manifest_find(key_id);
    failed = record == STD_NULL || !timing_80286_manifest_is_i286(record) ||
        !s3_gate_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286, TYPE_FALSE,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE);
    if (!failed) {
        failed = !s3_gate_write(&state, S3_GDT_BASE + 32u, user_data,
            sizeof(user_data)) || !s3_gate_write(&state, S3_CODE_BASE, iret,
            sizeof(iret)) || !s3_gate_write(&state, S3_CODE_BASE + 0x10u,
            target[next_bytes], next_bytes) || !s3_gate_write(&state, S3_STACK_TOP,
            frame, sizeof(frame));
        state.machine->executor_cpu.data.gdtr.limit = 39u;
        state.machine->executor_cpu.data.esp = 0x12348000u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
        if (!failed) failed = core_machine_set_retirement_observation_provider(
            state.machine, &provider) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = core_machine_run(state.machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 55u + next_bytes || capture.count != 1u ||
            capture.observation.source_ticks != 55u + next_bytes ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT timing_80286_manifest_run_task_transfer_recipe(
    task_switch_case task_case, const C_CHAR *key_id,
    type_unsigned_8 next_bytes, type_unsigned_64 base_ticks)
{
    static const type_unsigned_8 target[][6] = {
        { 0u,0u,0u,0u,0u,0u }, { 0x90u,0u,0u,0u,0u,0u },
        { 0x00u,0xc0u,0u,0u,0u,0u }, { 0x80u,0xc0u,1u,0u,0u,0u },
        { 0xc6u,0x46u,0u,1u,0u,0u }, { 0xc6u,0x06u,0u,0x10u,1u,0u },
        { 0xc7u,0x06u,0u,0x10u,1u,0u }
    };
    const core_machine_run_budget step = { 1u, 0u };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80286_manifest_capture_retirement, &capture
    };
    core_machine_run_result run = { 0 };
    task_switch_fixture fixture;
    C_INT failed;
    type_unsigned_32 steps;
    static const type_unsigned_8 idt_task_gate[] = {
        0u,0u,0x30u,0u,0u,0x85u,0u,0u
    };

    if (next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0])) {
        return 1;
    }
    failed = !task_switch_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80286) ||
        !task_switch_install(&fixture, task_case);
    if (!failed && task_case == TASK_SWITCH_CASE_IDT_TASK_GATE) {
        failed = !write_bytes(fixture.machine, IDT_BASE + 3u * 8u,
            idt_task_gate, sizeof(idt_task_gate));
        fixture.machine->executor_cpu.data.idtr.flagValid = TYPE_TRUE;
        fixture.machine->executor_cpu.data.idtr.sregtype = SREG_IDTR;
        fixture.machine->executor_cpu.data.idtr.base = IDT_BASE;
        fixture.machine->executor_cpu.data.idtr.limit = 0x001fu;
    }
    if (!failed) failed = core_machine_memory_write(fixture.machine,
        KERNEL_BASE + 0x100u, target[next_bytes], next_bytes) != TYPE_STATUS_OK;
    for (steps = 0u; !failed && steps < 32u &&
            (fixture.machine->executor_cpu.data.cs.selector != 0x0008u ||
            fixture.machine->executor_cpu.data.eip != 3u); ++steps) {
        failed = core_machine_run(fixture.machine, step, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u;
    }
    if (!failed && (fixture.machine->executor_cpu.data.cs.selector != 0x0008u ||
            fixture.machine->executor_cpu.data.eip != 3u)) failed = 1;
    if (!failed && key_id != STD_NULL) failed =
        timing_80286_manifest_find(key_id) == STD_NULL ||
        core_machine_set_retirement_observation_provider(fixture.machine,
            &provider) != TYPE_STATUS_OK;
    if (!failed) failed = core_machine_run(fixture.machine, step, &run) !=
        TYPE_STATUS_OK || run.reason != CORE_MACHINE_STOP_BUDGET ||
        run.executed != 1u || run.ticks != base_ticks + next_bytes ||
        fixture.machine->executor_cpu.data.tr.selector != 0x0030u;
    if (!failed && key_id != STD_NULL) failed = capture.count != 1u ||
        capture.observation.source_ticks != run.ticks ||
        capture.observation.timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        capture.observation.timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    if (failed) {
        STD_PRINTF("M5:T436:S6:I286-TASK-DETAIL:%s:run=%llu:source=%llu:count=%u:tr=%04x\n",
            key_id != STD_NULL ? key_id : "task-gate", run.ticks,
            capture.observation.source_ticks, capture.count,
            fixture.machine != STD_NULL ? fixture.machine->executor_cpu.data.tr.selector : 0u);
    } else {
        STD_PRINTF("M5:T436:S6:I286-TASK-OBSERVED:%s:ticks=%llu\n",
            key_id != STD_NULL ? key_id : "task-gate", run.ticks);
    }
    core_machine_destroy(fixture.machine);
    return failed;
}

static C_INT timing_80286_manifest_run_call_gate_recipe(const C_CHAR *key_id,
    type_unsigned_8 opcode, C_INT same_privilege, type_unsigned_8 next_bytes,
    type_unsigned_64 base_ticks)
{
    static const type_unsigned_8 target[][6] = {
        {0,0,0,0,0,0},{0x90u,0,0,0,0,0},{0,0xc0u,0,0,0,0},
        {0x80u,0xc0u,1u,0,0,0},{0xc6u,0x46u,0,1u,0,0},
        {0xc6u,6u,0,0x10u,1u,0},{0xc7u,6u,0,0x10u,1u,0}
    };
    type_unsigned_8 transfer[] = { 0u,0u,0u,0x33u,0u };
    const type_unsigned_8 target_selector[] = { 0x1bu,0u };
    const core_machine_run_budget step = { 1u, 0u };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80286_manifest_capture_retirement, &capture
    };
    core_machine_run_result run = { 0 };
    call_gate_machine state;
    type_unsigned_32 steps;
    C_INT failed;

    if (key_id == STD_NULL || (opcode != 0x9au && opcode != 0xeau) ||
        next_bytes == 0u || next_bytes >= sizeof(target) / sizeof(target[0])) {
        return 1;
    }
    transfer[0] = opcode;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !call_gate_prepare(&state) || !call_gate_install(&state);
    for (steps = 0u; !failed && steps < 48u &&
            (state.machine->executor_cpu.data.cs.selector != 0x001bu ||
            state.machine->executor_cpu.data.eip != 0u); ++steps) {
        failed = core_machine_run(state.machine, step, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u;
    }
    if (!failed && (state.machine->executor_cpu.data.cs.selector != 0x001bu ||
            state.machine->executor_cpu.data.eip != 0u)) failed = 1;
    if (!failed && same_privilege) failed = !call_gate_write(&state,
        CALL_GATE_GDT_BASE + 48u + 2u, target_selector, sizeof(target_selector));
    if (!failed) failed = !call_gate_write(&state, CALL_GATE_USER_CODE_BASE,
        transfer, sizeof(transfer)) || !call_gate_write(&state,
        (same_privilege ? CALL_GATE_USER_CODE_BASE : CALL_GATE_KERNEL_BASE) + 0x100u,
        target[next_bytes], next_bytes) ||
        core_machine_set_retirement_observation_provider(state.machine,
            &provider) != TYPE_STATUS_OK;
    if (!failed) failed = core_machine_run(state.machine, step, &run) !=
        TYPE_STATUS_OK || run.reason != CORE_MACHINE_STOP_BUDGET ||
        run.executed != 1u || run.ticks != base_ticks + next_bytes ||
        capture.count != 1u || capture.observation.source_ticks != run.ticks ||
        capture.observation.timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        capture.observation.timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        state.machine->executor_cpu.data.cs.selector !=
            (same_privilege ? 0x001bu : 0x0008u) ||
        state.machine->executor_cpu.data.eip != 0x0100u;
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT timing_80286_manifest_run_protected_int_same_recipe(
    const C_CHAR *key_id, type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        {0,0,0,0,0,0},{0x90u,0,0,0,0,0},{0,0xc0u,0,0,0,0},
        {0x80u,0xc0u,1u,0,0,0},{0xc6u,0x46u,0,1u,0,0},
        {0xc6u,6u,0,0x10u,1u,0},{0xc7u,6u,0,0x10u,1u,0}
    };
    const type_unsigned_8 interrupt[] = { 0xcdu,S3_VECTOR };
    const core_machine_run_budget budget = { 1u, 0u };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80286_manifest_capture_retirement, &capture
    };
    core_machine_run_result run = { 0 };
    s3_gate_machine state;
    C_INT failed;

    if (key_id == STD_NULL || next_bytes == 0u ||
        next_bytes >= sizeof(target) / sizeof(target[0])) return 1;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !s3_gate_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286, TYPE_FALSE,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE);
    if (!failed) failed = !s3_gate_write(&state, S3_CODE_BASE, interrupt,
        sizeof(interrupt)) || !s3_gate_write(&state, S3_CODE_BASE + S3_HANDLER,
        target[next_bytes], next_bytes) ||
        core_machine_set_retirement_observation_provider(state.machine,
            &provider) != TYPE_STATUS_OK;
    if (!failed) failed = core_machine_run(state.machine, budget, &run) !=
        TYPE_STATUS_OK || run.reason != CORE_MACHINE_STOP_BUDGET ||
        run.executed != 1u || run.ticks != 40u + next_bytes ||
        capture.count != 1u || capture.observation.source_ticks != run.ticks ||
        capture.observation.timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        capture.observation.timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        state.machine->executor_cpu.data.cs.selector != 0x0008u ||
        state.machine->executor_cpu.data.eip != S3_HANDLER;
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT timing_80286_manifest_run_protected_int_more_recipe(
    const C_CHAR *key_id, type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        {0,0,0,0,0,0},{0x90u,0,0,0,0,0},{0,0xc0u,0,0,0,0},
        {0x80u,0xc0u,1u,0,0,0},{0xc6u,0x46u,0,1u,0,0},
        {0xc6u,6u,0,0x10u,1u,0},{0xc7u,6u,0,0x10u,1u,0}
    };
    type_unsigned_8 gate[8u] = { 0u };
    const type_unsigned_8 interrupt[] = { 0xcdu,0x60u };
    const core_machine_run_budget step = { 1u, 0u };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80286_manifest_capture_retirement, &capture
    };
    core_machine_run_result run = { 0 };
    call_gate_machine state;
    type_unsigned_32 steps;
    C_INT failed;

    if (key_id == STD_NULL || next_bytes == 0u ||
        next_bytes >= sizeof(target) / sizeof(target[0])) return 1;
    gate[0] = 0u;
    gate[1] = 1u;
    gate[2] = 8u;
    gate[5] = 0xe6u;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !call_gate_prepare(&state) || !call_gate_install(&state);
    for (steps = 0u; !failed && steps < 48u &&
            (state.machine->executor_cpu.data.cs.selector != 0x001bu ||
            state.machine->executor_cpu.data.eip != 0u); ++steps) {
        failed = core_machine_run(state.machine, step, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u;
    }
    if (!failed && (state.machine->executor_cpu.data.cs.selector != 0x001bu ||
            state.machine->executor_cpu.data.eip != 0u)) failed = 1;
    if (!failed) failed = !call_gate_write(&state, 0x0400u + 0x60u * 8u, gate,
        sizeof(gate)) || !call_gate_write(&state, CALL_GATE_USER_CODE_BASE,
        interrupt, sizeof(interrupt)) || !call_gate_write(&state,
        CALL_GATE_KERNEL_BASE + 0x100u, target[next_bytes], next_bytes);
    if (!failed) {
        state.machine->executor_cpu.data.idtr.flagValid = TYPE_TRUE;
        state.machine->executor_cpu.data.idtr.sregtype = SREG_IDTR;
        state.machine->executor_cpu.data.idtr.base = 0x0400u;
        state.machine->executor_cpu.data.idtr.limit = 0x0307u;
        failed = core_machine_set_retirement_observation_provider(state.machine,
            &provider) != TYPE_STATUS_OK;
    }
    if (!failed) failed = core_machine_run(state.machine, step, &run) !=
        TYPE_STATUS_OK || run.reason != CORE_MACHINE_STOP_BUDGET ||
        run.executed != 1u || run.ticks != 78u + next_bytes ||
        capture.count != 1u || capture.observation.source_ticks != run.ticks ||
        capture.observation.timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        capture.observation.timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        state.machine->executor_cpu.data.cs.selector != 0x0008u ||
        state.machine->executor_cpu.data.eip != 0x0100u;
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT timing_80286_manifest_run_task_iret_recipe(const C_CHAR *key_id,
    type_unsigned_8 next_bytes)
{
    static const type_unsigned_8 target[][6] = {
        {0,0,0,0,0,0},{0x90u,0,0,0,0,0},{0,0xc0u,0,0,0,0},
        {0x80u,0xc0u,1u,0,0,0},{0xc6u,0x46u,0,1u,0,0},
        {0xc6u,6u,0,0x10u,1u,0},{0xc7u,6u,0,0x10u,1u,0}
    };
    const type_unsigned_8 iret[] = { 0xcfu };
    const core_machine_run_budget step = { 1u, 0u };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80286_manifest_capture_retirement, &capture
    };
    core_machine_run_result run = { 0 };
    task_switch_fixture fixture;
    type_unsigned_32 steps;
    C_INT failed;

    if (key_id == STD_NULL || next_bytes == 0u ||
        next_bytes >= sizeof(target) / sizeof(target[0])) return 1;
    failed = timing_80286_manifest_find(key_id) == STD_NULL ||
        !task_switch_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80286) ||
        !task_switch_install(&fixture, TASK_SWITCH_CASE_NESTED_RETURN);
    for (steps = 0u; !failed && steps < 32u &&
            (fixture.machine->executor_cpu.data.tr.selector != 0x0030u ||
            fixture.machine->executor_cpu.data.eip != 0x0100u); ++steps) {
        failed = core_machine_run(fixture.machine, step, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u;
    }
    if (!failed && (fixture.machine->executor_cpu.data.tr.selector != 0x0030u ||
            fixture.machine->executor_cpu.data.eip != 0x0100u)) failed = 1;
    if (!failed) failed = !write_bytes(fixture.machine, KERNEL_BASE + 0x100u,
        iret, sizeof(iret)) || !write_bytes(fixture.machine, KERNEL_BASE + 8u,
        target[next_bytes], next_bytes) ||
        core_machine_set_retirement_observation_provider(fixture.machine,
            &provider) != TYPE_STATUS_OK;
    if (!failed) failed = core_machine_run(fixture.machine, step, &run) !=
        TYPE_STATUS_OK || run.reason != CORE_MACHINE_STOP_BUDGET ||
        run.executed != 1u || run.ticks != 169u + next_bytes ||
        capture.count != 1u || capture.observation.source_ticks != run.ticks ||
        capture.observation.timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        capture.observation.timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        fixture.machine->executor_cpu.data.tr.selector != 0x0028u ||
        fixture.machine->executor_cpu.data.eip != 8u;
    if (failed) {
        STD_PRINTF("M5:T436:S6:I286-IRET-TASK-DETAIL:%s:run=%llu:source=%llu:count=%u:tr=%04x:eip=%04x\n",
            key_id, run.ticks, capture.observation.source_ticks, capture.count,
            fixture.machine != STD_NULL ? fixture.machine->executor_cpu.data.tr.selector : 0u,
            fixture.machine != STD_NULL ? fixture.machine->executor_cpu.data.eip : 0u);
    }
    core_machine_destroy(fixture.machine);
    return failed;
}

static C_INT timing_80286_manifest_run_protected_system(
    const timing_80286_manifest_recipe *recipe)
{
    const core_machine_run_budget budget = { 1u, 0u };
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    const timing_80286_manifest_record *record;
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_INT failed;

    if (recipe == STD_NULL) return 1;
    record = timing_80286_manifest_find(recipe->key_id);
    failed = record == STD_NULL || !timing_80286_manifest_is_i286(record) ||
        STD_STRCMP(record->profile, "80286") != 0 ||
        !timing_80286_manifest_prepare_protected_system(&machine, &capture,
            recipe->key_id, recipe->program, recipe->bytes);
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != recipe->ticks || capture.count != 1u ||
            capture.observation.source_ticks != recipe->ticks ||
            capture.observation.timing_origin != recipe->origin ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    if (failed) {
        STD_PRINTF("M5:T435:S10:I286-PROTECTED-DETAIL:%s:expected=%llu:run=%llu:source=%llu:count=%u:origin=%u:disposition=%u\n",
            recipe->key_id, recipe->ticks, run.ticks,
            capture.observation.source_ticks, capture.count,
            (type_unsigned_32)capture.observation.timing_origin,
            (type_unsigned_32)capture.observation.timing_disposition);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80286_manifest_run_control(
    const timing_80286_manifest_control_recipe *recipe)
{
    const core_machine_run_budget budget = { 1u, 0u };
    type_unsigned_8 program[3];
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    const timing_80286_manifest_record *record;
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_INT failed;

    if (recipe == STD_NULL) return 1;
    program[0] = recipe->opcode;
    /* A nonzero displacement makes a taken branch observably distinct from
     * fall-through; the selector classifies the post-execution EIP. */
    program[1] = 0x01u;
    program[2] = 0x90u;
    timing_80286_manifest_eflags = recipe->eflags;
    timing_80286_manifest_flags_active = 1;
    record = timing_80286_manifest_find(recipe->key_id);
    failed = record == STD_NULL || !timing_80286_manifest_is_i286(record) ||
        STD_STRCMP(record->profile, "80286") != 0 ||
        !timing_80286_manifest_prepare(&machine, &capture, recipe->key_id,
            program, sizeof(program));
    if (!failed) {
        machine->executor_cpu.data.ecx = timing_80286_manifest_control_cx(
            recipe->key_id);
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != recipe->ticks || capture.count != 1u ||
            capture.observation.source_ticks != recipe->ticks ||
            machine->executor_cpu.data.eip !=
                (recipe->opcode == 0xceu ? 0xfff5u :
                timing_80286_manifest_key_has_token(recipe->key_id, "-TAKEN") ?
                    0xfff3u : 0xfff2u) ||
            capture.observation.timing_origin !=
                timing_80286_manifest_control_origin(recipe->opcode) ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    if (failed) {
        STD_PRINTF("M5:T435:S10:I286-CONTROL-DETAIL:%s:expected=%llu:run=%llu:source=%llu:count=%u:origin=%u:disposition=%u\\n",
            recipe->key_id, recipe->ticks, run.ticks,
            capture.observation.source_ticks, capture.count,
            (type_unsigned_32)capture.observation.timing_origin,
            (type_unsigned_32)capture.observation.timing_disposition);
    }
    core_machine_destroy(machine);
    timing_80286_manifest_flags_active = 0;
    return failed;
}

C_INT main(C_VOID)
{
    static const timing_80286_manifest_recipe recipes[] = {
        { "I286-ADJ-AAA", { 0x37u }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ADJ-AAS", { 0x3fu }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ADJ-DAA", { 0x27u }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ADJ-DAS", { 0x2fu }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ADJ-AAD", { 0xd5u, 0x0au }, 2u, 14u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ADJ-AAM", { 0xd4u, 0x0au }, 2u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ADJ-CBW", { 0x98u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ADJ-CWD", { 0x99u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADD-RR", { 0x02u, 0xc1u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADD-RM", { 0x02u, 0x06u, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADD-MR", { 0x00u, 0x0eu, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADD-AI", { 0x04u, 1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADD-RMI", { 0x80u, 0xc0u, 1u }, 3u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-OR-RR", { 0x0au, 0xc1u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-OR-RM", { 0x0au, 0x06u, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-OR-MR", { 0x08u, 0x0eu, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-OR-AI", { 0x0cu, 1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-OR-RMI", { 0x80u, 0xc8u, 1u }, 3u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADC-RR", { 0x12u, 0xc1u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADC-RM", { 0x12u, 0x06u, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADC-MR", { 0x10u, 0x0eu, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADC-AI", { 0x14u, 1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADC-RMI", { 0x80u, 0xd0u, 1u }, 3u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SBB-RR", { 0x1au, 0xc1u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SBB-RM", { 0x1au, 0x06u, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SBB-MR", { 0x18u, 0x0eu, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SBB-AI", { 0x1cu, 1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SBB-RMI", { 0x80u, 0xd8u, 1u }, 3u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-AND-RR", { 0x22u, 0xc1u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-AND-RM", { 0x22u, 0x06u, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-AND-MR", { 0x20u, 0x0eu, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-AND-AI", { 0x24u, 1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-AND-RMI", { 0x80u, 0xe0u, 1u }, 3u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SUB-RR", { 0x2au, 0xc1u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SUB-RM", { 0x2au, 0x06u, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SUB-MR", { 0x28u, 0x0eu, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SUB-AI", { 0x2cu, 1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SUB-RMI", { 0x80u, 0xe8u, 1u }, 3u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-XOR-RR", { 0x32u, 0xc1u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-XOR-RM", { 0x32u, 0x06u, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-XOR-MR", { 0x30u, 0x0eu, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-XOR-AI", { 0x34u, 1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-XOR-RMI", { 0x80u, 0xf0u, 1u }, 3u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-CMP-RR", { 0x3au, 0xc1u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-CMP-RM", { 0x3au, 0x06u, 0u, 0x10u }, 4u, 6u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-CMP-MR", { 0x38u, 0x0eu, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-CMP-AI", { 0x3cu, 1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-CMP-RMI", { 0x80u, 0xf8u, 1u }, 3u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-TEST-RR", { 0x84u, 0xc1u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-TEST-RM", { 0x84u, 0x06u, 0u, 0x10u }, 4u, 6u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-TEST-AI", { 0xa8u, 1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-TEST-RMI", { 0xf6u, 0xc0u, 1u }, 3u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-INC-R", { 0x40u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-INC-M", { 0xfeu, 0x06u, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DEC-R", { 0x48u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DEC-M", { 0xfeu, 0x0eu, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-NEG-R", { 0xf6u, 0xd8u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-NEG-M", { 0xf6u, 0x1eu, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-NOT-R", { 0xf6u, 0xd0u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-NOT-M", { 0xf6u, 0x16u, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-XCHG-AXR", { 0x91u }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-XCHG-RR", { 0x86u, 0xc1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-XCHG-MR", { 0x86u, 0x0eu, 0u, 0x10u }, 4u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MUL-R8", { 0xf6u, 0xe0u }, 2u, 13u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MUL-R16", { 0xf7u, 0xe0u }, 2u, 21u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MUL-M8", { 0xf6u, 0x26u, 0u, 0x10u }, 4u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MUL-M16", { 0xf7u, 0x26u, 0u, 0x10u }, 4u, 24u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MUL-M16-ODD-WORD", { 0xf7u, 0x26u, 1u, 0x10u }, 4u, 26u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-R8", { 0xf6u, 0xe8u }, 2u, 13u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-R16", { 0xf7u, 0xe8u }, 2u, 21u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-M8", { 0xf6u, 0x2eu, 0u, 0x10u }, 4u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-M16", { 0xf7u, 0x2eu, 0u, 0x10u }, 4u, 24u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-M16-ODD-WORD", { 0xf7u, 0x2eu, 1u, 0x10u }, 4u, 26u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DIV-R8", { 0xf6u, 0xf0u }, 2u, 14u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DIV-R16", { 0xf7u, 0xf0u }, 2u, 22u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DIV-M8", { 0xf6u, 0x36u, 0u, 0x10u }, 4u, 17u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DIV-M16", { 0xf7u, 0x36u, 0u, 0x10u }, 4u, 25u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DIV-M16-ODD-WORD", { 0xf7u, 0x36u, 1u, 0x10u }, 4u, 27u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IDIV-R8", { 0xf6u, 0xf8u }, 2u, 17u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IDIV-R16", { 0xf7u, 0xf8u }, 2u, 25u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IDIV-M8", { 0xf6u, 0x3eu, 0u, 0x10u }, 4u, 20u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IDIV-M16", { 0xf7u, 0x3eu, 0u, 0x10u }, 4u, 28u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IDIV-M16-ODD-WORD", { 0xf7u, 0x3eu, 1u, 0x10u }, 4u, 30u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-IMM-IMM8-R", { 0x6bu, 0xc0u, 1u }, 3u, 21u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-IMM-IMM8-M", { 0x6bu, 0x06u, 0u, 0x10u, 1u }, 5u, 24u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-IMM-IMM16-R", { 0x69u, 0xc0u, 1u, 0u }, 4u, 21u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-IMM-IMM16-M", { 0x69u, 0x06u, 0u, 0x10u, 1u, 0u }, 6u, 24u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ROL-RM1", { 0xd0u, 0xc0u }, 2u, 2u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ROR-RM1", { 0xd0u, 0xc8u }, 2u, 2u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-RCL-RM1", { 0xd0u, 0xd0u }, 2u, 2u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-RCR-RM1", { 0xd0u, 0xd8u }, 2u, 2u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SHL-RM1", { 0xd0u, 0xe0u }, 2u, 2u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SHR-RM1", { 0xd0u, 0xe8u }, 2u, 2u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SAR-RM1", { 0xd0u, 0xf8u }, 2u, 2u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ROL-RMCL", { 0xd2u, 0xc0u }, 2u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ROR-RMCL", { 0xd2u, 0xc8u }, 2u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-RCL-RMCL", { 0xd2u, 0xd0u }, 2u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-RCR-RMCL", { 0xd2u, 0xd8u }, 2u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SHL-RMCL", { 0xd2u, 0xe0u }, 2u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SHR-RMCL", { 0xd2u, 0xe8u }, 2u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SAR-RMCL", { 0xd2u, 0xf8u }, 2u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ROL-RMIMM8", { 0xc0u, 0xc0u, 2u }, 3u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ROR-RMIMM8", { 0xc0u, 0xc8u, 2u }, 3u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-RCL-RMIMM8", { 0xc0u, 0xd0u, 2u }, 3u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-RCR-RMIMM8", { 0xc0u, 0xd8u, 2u }, 3u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SHL-RMIMM8", { 0xc0u, 0xe0u, 2u }, 3u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SHR-RMIMM8", { 0xc0u, 0xe8u, 2u }, 3u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SAR-RMIMM8", { 0xc0u, 0xf8u, 2u }, 3u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-RR", { 0x8au, 0xc1u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-RM", { 0x8au, 0x06u, 0u, 0x10u }, 4u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-MR", { 0x88u, 0x0eu, 0u, 0x10u }, 4u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-RI", { 0xb0u, 1u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-MI", { 0xc6u, 0x06u, 0u, 0x10u, 1u }, 5u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-MOFFS-R", { 0xa0u, 0u, 0x10u }, 3u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-MOFFS-W", { 0xa2u, 0u, 0x10u }, 3u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-SREG-STORE", { 0x8cu, 0x06u, 0u, 0x10u }, 4u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-SREG-STORE-ODD-WORD", { 0x8cu, 0x06u, 1u, 0x10u }, 4u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-SREG-LOAD-REAL", { 0x8eu, 0x1eu, 0u, 0x10u }, 4u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-SREG-LOAD-REAL-ODD-WORD", { 0x8eu, 0x1eu, 1u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LEA-M-REAL", { 0x8du, 0x06u, 0u, 0x10u }, 4u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LDS-M-REAL", { 0xc5u, 0x06u, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LDS-M-REAL-ODD-WORD", { 0xc5u, 0x06u, 1u, 0x10u }, 4u, 11u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LES-M-REAL", { 0xc4u, 0x06u, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LES-M-REAL-ODD-WORD", { 0xc4u, 0x06u, 1u, 0x10u }, 4u, 11u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-NOP", { 0x90u }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-CLC", { 0xf8u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-CMC", { 0xf5u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-STC", { 0xf9u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-CLD", { 0xfcu }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-STD", { 0xfdu }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-CLI", { 0xfau }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-STI", { 0xfbu }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LAHF", { 0x9fu }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SAHF", { 0x9eu }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-XLAT", { 0xd7u }, 1u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-WAIT", { 0x9bu }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ESC", { 0xdbu, 0xe3u }, 2u, 1u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-SGDT-M", { 0x0fu, 0x01u, 0x06u, 0u, 0x10u }, 5u, 11u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-SIDT-M", { 0x0fu, 0x01u, 0x0eu, 0u, 0x10u }, 5u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LGDT-M", { 0x0fu, 0x01u, 0x16u, 0u, 0x10u }, 5u, 11u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LIDT-M", { 0x0fu, 0x01u, 0x1eu, 0u, 0x10u }, 5u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-CLTS", { 0x0fu, 0x06u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IN-IMM-B", { 0xe4u, 0x80u }, 2u, 5u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-IN-IMM-W", { 0xe5u, 0x80u }, 2u, 5u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-IN-DX-B", { 0xecu }, 1u, 5u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-IN-DX-W", { 0xedu }, 1u, 5u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-OUT-IMM-B", { 0xe6u, 0x80u }, 2u, 3u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-OUT-IMM-W", { 0xe7u, 0x80u }, 2u, 3u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-OUT-DX-B", { 0xeeu }, 1u, 3u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-OUT-DX-W", { 0xefu }, 1u, 3u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-INT3-REAL-NEXT-BYTE-2", { 0xccu }, 1u, 25u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-INT-IMM-REAL-NEXT-BYTE-2", { 0xcdu, 0x60u }, 2u, 25u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-CALL-NEAR-DIRECT-NEXT-BYTE-1", { 0xe8u, 0x01u, 0x00u, 0x90u,
            0x90u }, 5u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-JMP-NEAR-NEXT-BYTE-1", { 0xebu, 0x01u, 0x90u, 0x90u }, 4u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-JMP-FAR-REAL-NEXT-BYTE-2", { 0xeau, 0xf5u, 0xffu, 0x00u, 0xf0u },
            5u, 13u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-RET-NEAR-NEXT-BYTE-1", { 0xc3u }, 1u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-PUSH-R", { 0x50u }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-PUSH-SEG", { 0x06u }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-PUSH-IMM", { 0x68u, 0x01u, 0x00u }, 3u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-PUSH-M", { 0xffu, 0x36u, 0u, 0x10u }, 4u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-PUSH-M-ODD-WORD", { 0xffu, 0x36u, 1u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-PUSHA", { 0x60u }, 1u, 17u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-PUSHF", { 0x9cu }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-POP-R", { 0x58u }, 1u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-POP-M", { 0x8fu, 0x06u, 0u, 0x10u }, 4u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-POP-M-ODD-WORD", { 0x8fu, 0x06u, 1u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-POP-SEG-REAL", { 0x1fu }, 1u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-POPA", { 0x61u }, 1u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-POPF", { 0x9du }, 1u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-LEAVE", { 0xc9u }, 1u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-ENTER-L0", { 0xc8u, 0x00u, 0x00u, 0x00u }, 4u, 11u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-ENTER-L1", { 0xc8u, 0x00u, 0x00u, 0x01u }, 4u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-ENTER-LN", { 0xc8u, 0x00u, 0x00u, 0x02u }, 4u, 20u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-BOUND", { 0x62u, 0x06u, 0x00u, 0x10u }, 4u, 13u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }
    };
    static const timing_80286_manifest_control_recipe control_recipes[] = {
        { "I286-JCC-JO-TAKEN", 0x70u, VCPU_EFLAGS_OF, 7u },
        { "I286-JCC-JO-NOT", 0x70u, 0u, 3u },
        { "I286-JCC-JNO-TAKEN", 0x71u, 0u, 7u },
        { "I286-JCC-JNO-NOT", 0x71u, VCPU_EFLAGS_OF, 3u },
        { "I286-JCC-JB-TAKEN", 0x72u, VCPU_EFLAGS_CF, 7u },
        { "I286-JCC-JB-NOT", 0x72u, 0u, 3u },
        { "I286-JCC-JAE-TAKEN", 0x73u, 0u, 7u },
        { "I286-JCC-JAE-NOT", 0x73u, VCPU_EFLAGS_CF, 3u },
        { "I286-JCC-JE-TAKEN", 0x74u, VCPU_EFLAGS_ZF, 7u },
        { "I286-JCC-JE-NOT", 0x74u, 0u, 3u },
        { "I286-JCC-JNE-TAKEN", 0x75u, 0u, 7u },
        { "I286-JCC-JNE-NOT", 0x75u, VCPU_EFLAGS_ZF, 3u },
        { "I286-JCC-JBE-TAKEN", 0x76u, VCPU_EFLAGS_CF, 7u },
        { "I286-JCC-JBE-NOT", 0x76u, 0u, 3u },
        { "I286-JCC-JA-TAKEN", 0x77u, 0u, 7u },
        { "I286-JCC-JA-NOT", 0x77u, VCPU_EFLAGS_CF, 3u },
        { "I286-JCC-JS-TAKEN", 0x78u, VCPU_EFLAGS_SF, 7u },
        { "I286-JCC-JS-NOT", 0x78u, 0u, 3u },
        { "I286-JCC-JNS-TAKEN", 0x79u, 0u, 7u },
        { "I286-JCC-JNS-NOT", 0x79u, VCPU_EFLAGS_SF, 3u },
        { "I286-JCC-JP-TAKEN", 0x7au, VCPU_EFLAGS_PF, 7u },
        { "I286-JCC-JP-NOT", 0x7au, 0u, 3u },
        { "I286-JCC-JNP-TAKEN", 0x7bu, 0u, 7u },
        { "I286-JCC-JNP-NOT", 0x7bu, VCPU_EFLAGS_PF, 3u },
        { "I286-JCC-JL-TAKEN", 0x7cu, VCPU_EFLAGS_SF, 7u },
        { "I286-JCC-JL-NOT", 0x7cu, 0u, 3u },
        { "I286-JCC-JGE-TAKEN", 0x7du, 0u, 7u },
        { "I286-JCC-JGE-NOT", 0x7du, VCPU_EFLAGS_SF, 3u },
        { "I286-JCC-JLE-TAKEN", 0x7eu, VCPU_EFLAGS_ZF, 7u },
        { "I286-JCC-JLE-NOT", 0x7eu, 0u, 3u },
        { "I286-JCC-JG-TAKEN", 0x7fu, 0u, 7u },
        { "I286-JCC-JG-NOT", 0x7fu, VCPU_EFLAGS_ZF, 3u },
        { "I286-JCXZ-TAKEN", 0xe3u, 0u, 8u },
        { "I286-JCXZ-NOT", 0xe3u, 0u, 4u },
        { "I286-LOOP-TAKEN", 0xe2u, 0u, 8u },
        { "I286-LOOP-NOT", 0xe2u, 0u, 4u },
        { "I286-LOOPE-TAKEN", 0xe1u, VCPU_EFLAGS_ZF, 8u },
        { "I286-LOOPE-NOT", 0xe1u, 0u, 4u },
        { "I286-LOOPNE-TAKEN", 0xe0u, 0u, 8u },
        { "I286-LOOPNE-NOT", 0xe0u, VCPU_EFLAGS_ZF, 4u },
        { "I286-INTO-TAKEN-NEXT-BYTE-2", 0xceu, VCPU_EFLAGS_OF, 26u }
    };
    static const timing_80286_manifest_recipe protected_system_recipes[] = {
        { "I286-MOV-SREG-LOAD-PM", { 0x8eu, 0x1eu, 0u, 0x10u }, 4u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-SREG-LOAD-PM-ODD-WORD", { 0x8eu, 0x1eu, 1u, 0x10u }, 4u, 21u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LEA-M-PM", { 0x8du, 0x06u, 0u, 0x10u }, 4u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LDS-M-PM", { 0xc5u, 0x06u, 0u, 0x10u }, 4u, 21u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LDS-M-PM-ODD-WORD", { 0xc5u, 0x06u, 1u, 0x10u }, 4u, 25u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LES-M-PM", { 0xc4u, 0x06u, 0u, 0x10u }, 4u, 21u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LES-M-PM-ODD-WORD", { 0xc4u, 0x06u, 1u, 0x10u }, 4u, 25u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-STACK-POP-SEG-PM", { 0x1fu }, 1u, 20u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-SYSTEM-VERR-R", { 0x0fu, 0x00u, 0xe0u }, 3u, 14u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-VERW-R", { 0x0fu, 0x00u, 0xe8u }, 3u, 14u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LAR-R", { 0x0fu, 0x02u, 0xc8u }, 3u, 14u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LSL-R", { 0x0fu, 0x03u, 0xc8u }, 3u, 14u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LLDT-R", { 0x0fu, 0x00u, 0xd0u }, 3u, 17u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LTR-R", { 0x0fu, 0x00u, 0xd8u }, 3u, 17u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LMSW-R", { 0x0fu, 0x01u, 0xf0u }, 3u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-SLDT-R", { 0x0fu, 0x00u, 0xc0u }, 3u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-SMSW-R", { 0x0fu, 0x01u, 0xe0u }, 3u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-STR-R", { 0x0fu, 0x00u, 0xc8u }, 3u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-VERR-M", { 0x0fu, 0x00u, 0x26u, 0u, 0x10u }, 5u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-VERW-M", { 0x0fu, 0x00u, 0x2eu, 0u, 0x10u }, 5u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LAR-M", { 0x0fu, 0x02u, 0x0eu, 0u, 0x10u }, 5u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LSL-M", { 0x0fu, 0x03u, 0x0eu, 0u, 0x10u }, 5u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LLDT-M", { 0x0fu, 0x00u, 0x16u, 0u, 0x10u }, 5u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LTR-M", { 0x0fu, 0x00u, 0x1eu, 0u, 0x10u }, 5u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LMSW-M", { 0x0fu, 0x01u, 0x36u, 0u, 0x10u }, 5u, 6u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-SLDT-M", { 0x0fu, 0x00u, 0x06u, 0u, 0x10u }, 5u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-SMSW-M", { 0x0fu, 0x01u, 0x26u, 0u, 0x10u }, 5u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-STR-M", { 0x0fu, 0x00u, 0x0eu, 0u, 0x10u }, 5u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-VERR-M-EA-BID", { 0x0fu, 0x00u, 0xa2u, 0u, 0u }, 5u, 17u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-VERW-M-EA-BID", { 0x0fu, 0x00u, 0xaau, 0u, 0u }, 5u, 17u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LAR-M-EA-BID", { 0x0fu, 0x02u, 0x8au, 0u, 0u }, 5u, 17u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LSL-M-EA-BID", { 0x0fu, 0x03u, 0x8au, 0u, 0u }, 5u, 17u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LLDT-M-EA-BID", { 0x0fu, 0x00u, 0x92u, 0u, 0u }, 5u, 20u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LTR-M-EA-BID", { 0x0fu, 0x00u, 0x9au, 0u, 0u }, 5u, 20u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-SGDT-M-EA-BID", { 0x0fu, 0x01u, 0x82u, 0u, 0u }, 5u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-SIDT-M-EA-BID", { 0x0fu, 0x01u, 0x8au, 0u, 0u }, 5u, 13u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LGDT-M-EA-BID", { 0x0fu, 0x01u, 0x92u, 0u, 0u }, 5u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LIDT-M-EA-BID", { 0x0fu, 0x01u, 0x9au, 0u, 0u }, 5u, 13u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LMSW-M-EA-BID", { 0x0fu, 0x01u, 0xb2u, 0u, 0u }, 5u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-SLDT-M-EA-BID", { 0x0fu, 0x00u, 0x82u, 0u, 0u }, 5u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-SMSW-M-EA-BID", { 0x0fu, 0x01u, 0xa2u, 0u, 0u }, 5u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-STR-M-EA-BID", { 0x0fu, 0x00u, 0x8au, 0u, 0u }, 5u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-VERR-M-SEGMENT", { 0x26u, 0x0fu, 0x00u, 0x26u, 0u, 0x10u }, 6u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-VERW-M-SEGMENT", { 0x26u, 0x0fu, 0x00u, 0x2eu, 0u, 0x10u }, 6u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LAR-M-SEGMENT", { 0x26u, 0x0fu, 0x02u, 0x0eu, 0u, 0x10u }, 6u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LSL-M-SEGMENT", { 0x26u, 0x0fu, 0x03u, 0x0eu, 0u, 0x10u }, 6u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LLDT-M-SEGMENT", { 0x26u, 0x0fu, 0x00u, 0x16u, 0u, 0x10u }, 6u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LTR-M-SEGMENT", { 0x26u, 0x0fu, 0x00u, 0x1eu, 0u, 0x10u }, 6u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LMSW-M-SEGMENT", { 0x26u, 0x0fu, 0x01u, 0x36u, 0u, 0x10u }, 6u, 6u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-SLDT-M-SEGMENT", { 0x26u, 0x0fu, 0x00u, 0x06u, 0u, 0x10u }, 6u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-SMSW-M-SEGMENT", { 0x26u, 0x0fu, 0x01u, 0x26u, 0u, 0x10u }, 6u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-STR-M-SEGMENT", { 0x26u, 0x0fu, 0x00u, 0x0eu, 0u, 0x10u }, 6u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-SGDT-M-SEGMENT", { 0x26u, 0x0fu, 0x01u, 0x06u, 0u, 0x10u }, 6u, 11u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-SIDT-M-SEGMENT", { 0x26u, 0x0fu, 0x01u, 0x0eu, 0u, 0x10u }, 6u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LGDT-M-SEGMENT", { 0x26u, 0x0fu, 0x01u, 0x16u, 0u, 0x10u }, 6u, 11u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-SYSTEM-LIDT-M-SEGMENT", { 0x26u, 0x0fu, 0x01u, 0x1eu, 0u, 0x10u }, 6u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-SREG-LOAD-PM-SEGMENT", { 0x26u, 0x8eu, 0x1eu, 0u, 0x10u }, 5u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LDS-M-PM-SEGMENT", { 0x26u, 0xc5u, 0x06u, 0u, 0x10u }, 5u, 21u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LES-M-PM-SEGMENT", { 0x26u, 0xc4u, 0x06u, 0u, 0x10u }, 5u, 21u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-SREG-LOAD-PM-EA-BID", { 0x8eu, 0x9au, 0u, 0u }, 4u, 20u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LDS-M-PM-EA-BID", { 0xc5u, 0x82u, 0u, 0u }, 4u, 22u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LES-M-PM-EA-BID", { 0xc4u, 0x82u, 0u, 0u }, 4u, 22u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-SREG-LOAD-PM-EA-BID-ODD-WORD",
            { 0x8eu, 0x9au, 1u, 0u }, 4u, 22u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LDS-M-PM-EA-BID-ODD-WORD",
            { 0xc5u, 0x82u, 1u, 0u }, 4u, 26u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LES-M-PM-EA-BID-ODD-WORD",
            { 0xc4u, 0x82u, 1u, 0u }, 4u, 26u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ARPL", { 0x63u, 0xc8u }, 2u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY }
    };
    static const timing_80286_manifest_recipe segment_recipes[] = {
        { "I286-ALU-ADD-RM-SEGMENT", { 0x26u, 0x02u, 0x06u, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADD-MR-SEGMENT", { 0x26u, 0x00u, 0x0eu, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADD-RMI-SEGMENT", { 0x26u, 0x80u, 0x06u, 0u, 0x10u, 1u }, 6u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-OR-RM-SEGMENT", { 0x26u, 0x0au, 0x06u, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-OR-MR-SEGMENT", { 0x26u, 0x08u, 0x0eu, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-OR-RMI-SEGMENT", { 0x26u, 0x80u, 0x0eu, 0u, 0x10u, 1u }, 6u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADC-RM-SEGMENT", { 0x26u, 0x12u, 0x06u, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADC-MR-SEGMENT", { 0x26u, 0x10u, 0x0eu, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADC-RMI-SEGMENT", { 0x26u, 0x80u, 0x16u, 0u, 0x10u, 1u }, 6u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SBB-RM-SEGMENT", { 0x26u, 0x1au, 0x06u, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SBB-MR-SEGMENT", { 0x26u, 0x18u, 0x0eu, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SBB-RMI-SEGMENT", { 0x26u, 0x80u, 0x1eu, 0u, 0x10u, 1u }, 6u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-AND-RM-SEGMENT", { 0x26u, 0x22u, 0x06u, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-AND-MR-SEGMENT", { 0x26u, 0x20u, 0x0eu, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-AND-RMI-SEGMENT", { 0x26u, 0x80u, 0x26u, 0u, 0x10u, 1u }, 6u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SUB-RM-SEGMENT", { 0x26u, 0x2au, 0x06u, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SUB-MR-SEGMENT", { 0x26u, 0x28u, 0x0eu, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SUB-RMI-SEGMENT", { 0x26u, 0x80u, 0x2eu, 0u, 0x10u, 1u }, 6u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-XOR-RM-SEGMENT", { 0x26u, 0x32u, 0x06u, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-XOR-MR-SEGMENT", { 0x26u, 0x30u, 0x0eu, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-XOR-RMI-SEGMENT", { 0x26u, 0x80u, 0x36u, 0u, 0x10u, 1u }, 6u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-CMP-RM-SEGMENT", { 0x26u, 0x3au, 0x06u, 0u, 0x10u }, 5u, 6u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-CMP-MR-SEGMENT", { 0x26u, 0x38u, 0x0eu, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-CMP-RMI-SEGMENT", { 0x26u, 0x80u, 0x3eu, 0u, 0x10u, 1u }, 6u, 6u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-TEST-RM-SEGMENT", { 0x26u, 0x84u, 0x06u, 0u, 0x10u }, 5u, 6u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-TEST-RMI-SEGMENT", { 0x26u, 0xf6u, 0x06u, 0u, 0x10u, 1u }, 6u, 6u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-INC-M-SEGMENT", { 0x26u, 0xfeu, 0x06u, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DEC-M-SEGMENT", { 0x26u, 0xfeu, 0x0eu, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-NEG-M-SEGMENT", { 0x26u, 0xf6u, 0x1eu, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-NOT-M-SEGMENT", { 0x26u, 0xf6u, 0x16u, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-XCHG-MR-SEGMENT", { 0x26u, 0x86u, 0x0eu, 0u, 0x10u }, 5u, 5u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MUL-M8-SEGMENT", { 0x26u, 0xf6u, 0x26u, 0u, 0x10u }, 5u, 16u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MUL-M16-SEGMENT", { 0x26u, 0xf7u, 0x26u, 0u, 0x10u }, 5u, 24u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-M8-SEGMENT", { 0x26u, 0xf6u, 0x2eu, 0u, 0x10u }, 5u, 16u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-M16-SEGMENT", { 0x26u, 0xf7u, 0x2eu, 0u, 0x10u }, 5u, 24u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DIV-M8-SEGMENT", { 0x26u, 0xf6u, 0x36u, 0u, 0x10u }, 5u, 17u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DIV-M16-SEGMENT", { 0x26u, 0xf7u, 0x36u, 0u, 0x10u }, 5u, 25u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IDIV-M8-SEGMENT", { 0x26u, 0xf6u, 0x3eu, 0u, 0x10u }, 5u, 20u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IDIV-M16-SEGMENT", { 0x26u, 0xf7u, 0x3eu, 0u, 0x10u }, 5u, 28u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-IMM-IMM8-M-SEGMENT", { 0x26u, 0x6bu, 0x06u, 0u, 0x10u, 1u }, 6u, 24u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-IMM-IMM16-M-SEGMENT", { 0x26u, 0x69u, 0x06u, 0u, 0x10u, 1u, 0u }, 7u, 24u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-RM-SEGMENT", { 0x26u, 0x8au, 0x06u, 0u, 0x10u }, 5u, 5u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-MR-SEGMENT", { 0x26u, 0x88u, 0x0eu, 0u, 0x10u }, 5u, 3u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-MI-SEGMENT", { 0x26u, 0xc6u, 0x06u, 0u, 0x10u, 1u }, 6u, 3u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-MOFFS-R-SEGMENT", { 0x26u, 0xa0u, 0u, 0x10u }, 4u, 5u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-MOFFS-W-SEGMENT", { 0x26u, 0xa2u, 0u, 0x10u }, 4u, 3u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-XLAT-SEGMENT", { 0x26u, 0xd7u }, 2u, 5u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-SREG-STORE-SEGMENT", { 0x26u, 0x8cu, 0x06u, 0u, 0x10u }, 5u, 3u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-SREG-LOAD-REAL-SEGMENT", { 0x26u, 0x8eu, 0x1eu, 0u, 0x10u }, 5u, 5u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LDS-M-REAL-SEGMENT", { 0x26u, 0xc5u, 0x06u, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LES-M-REAL-SEGMENT", { 0x26u, 0xc4u, 0x06u, 0u, 0x10u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-STACK-PUSH-M-SEGMENT", { 0x26u, 0xffu, 0x36u, 0u, 0x10u }, 5u, 5u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-POP-M-SEGMENT", { 0x26u, 0x8fu, 0x06u, 0u, 0x10u }, 5u, 5u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-CALL-NEAR-RM-SEGMENT", { 0x26u, 0xffu, 0x16u, 0u, 0x10u }, 5u, 13u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-JMP-RM-SEGMENT", { 0x26u, 0xffu, 0x26u, 0u, 0x10u }, 5u, 13u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-PUSH-M-EA-BID", { 0xffu, 0xb2u, 0u, 0u }, 4u, 6u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-POP-M-EA-BID", { 0x8fu, 0x82u, 0u, 0u }, 4u, 6u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-CALL-NEAR-RM-EA-BID", { 0xffu, 0x92u, 0u, 0u }, 4u, 14u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-JMP-RM-EA-BID", { 0xffu, 0xa2u, 0u, 0u }, 4u, 14u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }
    };
    static const timing_80286_manifest_recipe ea_recipes[] = {
        { "I286-ALU-ADD-RM-EA-BID", { 0x02u, 0x82u, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADD-MR-EA-BID", { 0x00u, 0x8au, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADD-RMI-EA-BID", { 0x80u, 0x82u, 0u, 0u, 1u }, 5u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-OR-RM-EA-BID", { 0x0au, 0x82u, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-OR-MR-EA-BID", { 0x08u, 0x8au, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-OR-RMI-EA-BID", { 0x80u, 0x8au, 0u, 0u, 1u }, 5u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADC-RM-EA-BID", { 0x12u, 0x82u, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADC-MR-EA-BID", { 0x10u, 0x8au, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADC-RMI-EA-BID", { 0x80u, 0x92u, 0u, 0u, 1u }, 5u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SBB-RM-EA-BID", { 0x1au, 0x82u, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SBB-MR-EA-BID", { 0x18u, 0x8au, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SBB-RMI-EA-BID", { 0x80u, 0x9au, 0u, 0u, 1u }, 5u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-AND-RM-EA-BID", { 0x22u, 0x82u, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-AND-MR-EA-BID", { 0x20u, 0x8au, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-AND-RMI-EA-BID", { 0x80u, 0xa2u, 0u, 0u, 1u }, 5u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SUB-RM-EA-BID", { 0x2au, 0x82u, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SUB-MR-EA-BID", { 0x28u, 0x8au, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SUB-RMI-EA-BID", { 0x80u, 0xaau, 0u, 0u, 1u }, 5u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-XOR-RM-EA-BID", { 0x32u, 0x82u, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-XOR-MR-EA-BID", { 0x30u, 0x8au, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-XOR-RMI-EA-BID", { 0x80u, 0xb2u, 0u, 0u, 1u }, 5u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-CMP-RM-EA-BID", { 0x3au, 0x82u, 0u, 0u }, 4u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-CMP-MR-EA-BID", { 0x38u, 0x8au, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-CMP-RMI-EA-BID", { 0x80u, 0xbau, 0u, 0u, 1u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-TEST-RM-EA-BID", { 0x84u, 0x82u, 0u, 0u }, 4u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-TEST-RMI-EA-BID", { 0xf6u, 0x82u, 0u, 0u, 1u }, 5u, 7u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-INC-M-EA-BID", { 0xfeu, 0x82u, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DEC-M-EA-BID", { 0xfeu, 0x8au, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-NEG-M-EA-BID", { 0xf6u, 0x9au, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-NOT-M-EA-BID", { 0xf6u, 0x92u, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-XCHG-MR-EA-BID", { 0x86u, 0x8au, 0u, 0u }, 4u, 6u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MUL-M8-EA-BID", { 0xf6u, 0xa2u, 0u, 0u }, 4u, 17u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MUL-M16-EA-BID", { 0xf7u, 0xa2u, 0u, 0u }, 4u, 25u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-M8-EA-BID", { 0xf6u, 0xaau, 0u, 0u }, 4u, 17u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-M16-EA-BID", { 0xf7u, 0xaau, 0u, 0u }, 4u, 25u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DIV-M8-EA-BID", { 0xf6u, 0xb2u, 0u, 0u }, 4u, 18u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DIV-M16-EA-BID", { 0xf7u, 0xb2u, 0u, 0u }, 4u, 26u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IDIV-M8-EA-BID", { 0xf6u, 0xbau, 0u, 0u }, 4u, 21u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IDIV-M16-EA-BID", { 0xf7u, 0xbau, 0u, 0u }, 4u, 29u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-IMM-IMM8-M-EA-BID", { 0x6bu, 0x82u, 0u, 0u, 1u }, 5u, 25u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-IMM-IMM16-M-EA-BID", { 0x69u, 0x82u, 0u, 0u, 1u, 0u }, 6u, 25u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-RM-EA-BID", { 0x8au, 0x82u, 0u, 0u }, 4u, 6u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-MR-EA-BID", { 0x88u, 0x8au, 0u, 0u }, 4u, 4u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-MI-EA-BID", { 0xc6u, 0x82u, 0u, 0u, 1u }, 5u, 4u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-SREG-STORE-EA-BID", { 0x8cu, 0x82u, 0u, 0u }, 4u, 4u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-SREG-LOAD-REAL-EA-BID", { 0x8eu, 0x9au, 0u, 0u }, 4u, 6u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LDS-M-REAL-EA-BID", { 0xc5u, 0x82u, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LES-M-REAL-EA-BID", { 0xc4u, 0x82u, 0u, 0u }, 4u, 8u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY }
    };
    static const timing_80286_manifest_recipe odd_word_recipes[] = {
        { "I286-IMUL-IMM-IMM16-M-ODD-WORD",
            { 0x69u, 0x06u, 1u, 0x10u, 1u, 0u }, 6u, 26u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-RM-ODD-WORD", { 0x8bu, 0x06u, 1u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-MR-ODD-WORD", { 0x89u, 0x0eu, 1u, 0x10u }, 4u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-MI-ODD-WORD", { 0xc7u, 0x06u, 1u, 0x10u, 1u, 0u }, 6u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-MOFFS-R-ODD-WORD", { 0xa1u, 1u, 0x10u }, 3u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-MOFFS-W-ODD-WORD", { 0xa3u, 1u, 0x10u }, 3u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY }
    };
    static const timing_80286_manifest_recipe ea_odd_word_recipes[] = {
        { "I286-MUL-M16-EA-BID-ODD-WORD", { 0xf7u, 0xa2u, 1u, 0u }, 4u, 27u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-M16-EA-BID-ODD-WORD", { 0xf7u, 0xaau, 1u, 0u }, 4u, 27u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DIV-M16-EA-BID-ODD-WORD", { 0xf7u, 0xb2u, 1u, 0u }, 4u, 28u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IDIV-M16-EA-BID-ODD-WORD", { 0xf7u, 0xbau, 1u, 0u }, 4u, 31u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-RM-EA-BID-ODD-WORD", { 0x8bu, 0x82u, 1u, 0u }, 4u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-MR-EA-BID-ODD-WORD", { 0x89u, 0x8au, 1u, 0u }, 4u, 6u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-MI-EA-BID-ODD-WORD",
            { 0xc7u, 0x82u, 1u, 0u, 1u, 0u }, 6u, 6u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-SREG-STORE-EA-BID-ODD-WORD",
            { 0x8cu, 0x82u, 1u, 0u }, 4u, 6u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-MOV-SREG-LOAD-REAL-EA-BID-ODD-WORD",
            { 0x8eu, 0x9au, 1u, 0u }, 4u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LDS-M-REAL-EA-BID-ODD-WORD",
            { 0xc5u, 0x82u, 1u, 0u }, 4u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LES-M-REAL-EA-BID-ODD-WORD",
            { 0xc4u, 0x82u, 1u, 0u }, 4u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-STACK-PUSH-M-EA-BID-ODD-WORD",
            { 0xffu, 0xb2u, 1u, 0u }, 4u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-POP-M-EA-BID-ODD-WORD", { 0x8fu, 0x82u, 1u, 0u }, 4u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }
    };
    static const timing_80286_manifest_repeat_recipe repeat_recipes[] = {
        { "I286-REP-MOVS-B", 0xf3u, 0xa4u, 9u, 4u, 5u },
        { "I286-REP-MOVS-W", 0xf3u, 0xa5u, 9u, 4u, 5u },
        { "I286-REP-CMPS-REPE-B", 0xf3u, 0xa6u, 14u, 9u, 5u },
        { "I286-REP-CMPS-REPE-W", 0xf3u, 0xa7u, 14u, 9u, 5u },
        { "I286-REP-CMPS-REPNE-B", 0xf2u, 0xa6u, 14u, 9u, 5u },
        { "I286-REP-CMPS-REPNE-W", 0xf2u, 0xa7u, 14u, 9u, 5u },
        { "I286-REP-STOS-B", 0xf3u, 0xaau, 7u, 3u, 4u },
        { "I286-REP-STOS-W", 0xf3u, 0xabu, 7u, 3u, 4u },
        { "I286-REP-LODS-B", 0xf3u, 0xacu, 9u, 4u, 5u },
        { "I286-REP-LODS-W", 0xf3u, 0xadu, 9u, 4u, 5u },
        { "I286-REP-SCAS-REPE-B", 0xf3u, 0xaeu, 13u, 8u, 5u },
        { "I286-REP-SCAS-REPE-W", 0xf3u, 0xafu, 13u, 8u, 5u },
        { "I286-REP-SCAS-REPNE-B", 0xf2u, 0xaeu, 13u, 8u, 5u },
        { "I286-REP-SCAS-REPNE-W", 0xf2u, 0xafu, 13u, 8u, 5u },
        { "I286-REP-INS-B", 0xf3u, 0x6cu, 9u, 4u, 5u },
        { "I286-REP-INS-W", 0xf3u, 0x6du, 9u, 4u, 5u },
        { "I286-REP-OUTS-B", 0xf3u, 0x6eu, 9u, 4u, 5u },
        { "I286-REP-OUTS-W", 0xf3u, 0x6fu, 9u, 4u, 5u }
    };
    static const timing_80286_manifest_recipe string_recipes[] = {
        { "I286-STRING-MOVS-B", { 0xa4u }, 1u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-STRING-MOVS-W", { 0xa5u }, 1u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-STRING-CMPS-B", { 0xa6u }, 1u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-STRING-CMPS-W", { 0xa7u }, 1u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-STRING-STOS-B", { 0xaau }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-STRING-STOS-W", { 0xabu }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-STRING-LODS-B", { 0xacu }, 1u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-STRING-LODS-W", { 0xadu }, 1u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-STRING-SCAS-B", { 0xaeu }, 1u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-STRING-SCAS-W", { 0xafu }, 1u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-STRING-INS-B", { 0x6cu }, 1u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-STRING-INS-W", { 0x6du }, 1u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-STRING-OUTS-B", { 0x6eu }, 1u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I286-STRING-OUTS-W", { 0x6fu }, 1u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO }
    };
    static const timing_80286_manifest_recipe lock_recipes[] = {
        { "I286-ALU-ADD-MR-LOCK", { 0xf0u, 0x00u, 0x0eu, 0u, 0x10u }, 5u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-OR-MR-LOCK", { 0xf0u, 0x08u, 0x0eu, 0u, 0x10u }, 5u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-OR-RMI-LOCK", { 0xf0u, 0x80u, 0x0eu, 0u, 0x10u, 1u }, 6u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADC-MR-LOCK", { 0xf0u, 0x10u, 0x0eu, 0u, 0x10u }, 5u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADC-RMI-LOCK", { 0xf0u, 0x80u, 0x16u, 0u, 0x10u, 1u }, 6u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SBB-MR-LOCK", { 0xf0u, 0x18u, 0x0eu, 0u, 0x10u }, 5u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SBB-RMI-LOCK", { 0xf0u, 0x80u, 0x1eu, 0u, 0x10u, 1u }, 6u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-AND-MR-LOCK", { 0xf0u, 0x20u, 0x0eu, 0u, 0x10u }, 5u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-AND-RMI-LOCK", { 0xf0u, 0x80u, 0x26u, 0u, 0x10u, 1u }, 6u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SUB-MR-LOCK", { 0xf0u, 0x28u, 0x0eu, 0u, 0x10u }, 5u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-SUB-RMI-LOCK", { 0xf0u, 0x80u, 0x2eu, 0u, 0x10u, 1u }, 6u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-XOR-MR-LOCK", { 0xf0u, 0x30u, 0x0eu, 0u, 0x10u }, 5u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-XOR-RMI-LOCK", { 0xf0u, 0x80u, 0x36u, 0u, 0x10u, 1u }, 6u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADD-RMI-LOCK", { 0xf0u, 0x80u, 0x06u, 0u, 0x10u, 1u }, 6u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-INC-M-LOCK", { 0xf0u, 0xfeu, 0x06u, 0u, 0x10u }, 5u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DEC-M-LOCK", { 0xf0u, 0xfeu, 0x0eu, 0u, 0x10u }, 5u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-NEG-M-LOCK", { 0xf0u, 0xf6u, 0x1eu, 0u, 0x10u }, 5u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-NOT-M-LOCK", { 0xf0u, 0xf6u, 0x16u, 0u, 0x10u }, 5u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-XCHG-MR-LOCK", { 0xf0u, 0x86u, 0x0eu, 0u, 0x10u }, 5u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_80286_manifest_run(&recipes[index])) {
            STD_PRINTF("M5:T435:S10:I286-MANIFEST-RECIPE:FAIL:%s\n",
                recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(control_recipes) / sizeof(control_recipes[0]);
        ++index) {
        if (timing_80286_manifest_run_control(&control_recipes[index])) {
            STD_PRINTF("M5:T435:S10:I286-CONTROL-RECIPE:FAIL:%s\n",
                control_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 2u; index <= 6u; ++index) {
        if (timing_80286_manifest_run_jmp_next_byte_recipe((type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-JMP-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 1u; index <= 6u; ++index) {
        if (index == 2u) continue;
        if (timing_80286_manifest_run_far_jmp_next_byte_recipe(
                (type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-FAR-JMP-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 1u; index <= 6u; ++index) {
        if (timing_80286_manifest_run_far_indirect_call_next_byte_recipe(
                (type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-FAR-INDIRECT-CALL-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 1u; index <= 6u; ++index) {
        if (timing_80286_manifest_run_far_ret_next_byte_recipe(
                (type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-FAR-RET-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 1u; index <= 6u; ++index) {
        if (timing_80286_manifest_run_iret_next_byte_recipe(
                (type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-IRET-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 1u; index <= 6u; ++index) {
        if (timing_80286_manifest_run_far_call_next_byte_recipe(
                (type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-FAR-CALL-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 1u; index <= 6u; ++index) {
        if (timing_80286_manifest_run_ret_imm_next_byte_recipe(
                (type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-RET-IMM-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 1u; index <= 6u; ++index) {
        if (timing_80286_manifest_run_indirect_call_next_byte_recipe(
                (type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-INDIRECT-CALL-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 1u; index <= 6u; ++index) {
        if (timing_80286_manifest_run_indirect_jmp_next_byte_recipe(
                (type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-INDIRECT-JMP-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 1u; index <= 6u; ++index) {
        C_CHAR taken_key[96];
        C_CHAR not_key[96];

        if (STD_SNPRINTF(taken_key, sizeof(taken_key),
                "I286-INTO-TAKEN-NEXT-BYTE-%u", (type_unsigned_32)index) < 0 ||
            STD_SNPRINTF(not_key, sizeof(not_key),
                "I286-INTO-NOT-NEXT-BYTE-%u", (type_unsigned_32)index) < 0 ||
            (index != 2u && timing_80286_manifest_run_into_next_byte_recipe(1,
                (type_unsigned_8)index, taken_key)) ||
            timing_80286_manifest_run_into_next_byte_recipe(0,
                (type_unsigned_8)index, not_key)) {
            STD_PRINTF("M5:T435:S10:I286-INTO-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    if (timing_80286_manifest_run_into_next_byte_recipe(1, 1u,
            "I286-INTO-TAKEN")) {
        STD_PRINTF("M5:T436:S5:I286-INTO-BASE-RECIPE:FAIL\n");
        return 1;
    }
    if (timing_80286_manifest_run_into_next_byte_recipe(0, 1u,
            "I286-INTO-NOT")) {
        STD_PRINTF("M5:T436:S5:I286-INTO-NOT-BASE-RECIPE:FAIL\n");
        return 1;
    }
    for (index = 1u; index <= 6u; ++index) {
        if (index == 2u) continue;
        if (timing_80286_manifest_run_interrupt_next_byte_recipe(0,
                (type_unsigned_8)index) ||
            timing_80286_manifest_run_interrupt_next_byte_recipe(1,
                (type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-INT-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 2u; index <= 6u; ++index) {
        if (timing_80286_manifest_run_ret_next_byte_recipe((type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-RET-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 2u; index <= 6u; ++index) {
        if (timing_80286_manifest_run_call_next_byte_recipe((type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-CALL-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 1u; index <= 6u; ++index) {
        if (timing_80286_manifest_run_protected_far_memory_call_next_byte_recipe(
                (type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-PROTECTED-FAR-MEMORY-CALL-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 1u; index <= 6u; ++index) {
        if (timing_80286_manifest_run_protected_far_call_next_byte_recipe(
                (type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-PROTECTED-FAR-CALL-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 1u; index <= 6u; ++index) {
        if (timing_80286_manifest_run_protected_far_memory_jmp_next_byte_recipe(
                (type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-PROTECTED-FAR-MEMORY-JMP-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 1u; index <= 6u; ++index) {
        if (timing_80286_manifest_run_protected_far_ret_next_byte_recipe(
                (type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-PROTECTED-FAR-RET-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 1u; index <= 6u; ++index) {
        if (timing_80286_manifest_run_protected_iret_next_byte_recipe(
                (type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-PROTECTED-IRET-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 1u; index <= 6u; ++index) {
        if (timing_80286_manifest_run_protected_outer_ret_recipe(
                (type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-PROTECTED-OUTER-RET-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 1u; index <= 6u; ++index) {
        if (timing_80286_manifest_run_protected_outer_iret_recipe(
                (type_unsigned_8)index)) {
            STD_PRINTF("M5:T435:S10:I286-PROTECTED-OUTER-IRET-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    for (index = 1u; index <= 6u; ++index) {
        C_CHAR call_key[64];
        C_CHAR jmp_key[64];

        if (STD_SNPRINTF(call_key, sizeof(call_key),
                "I286-CALL-TASK-NEXT-BYTE-%u", (type_unsigned_32)index) < 0 ||
            STD_SNPRINTF(jmp_key, sizeof(jmp_key),
                "I286-JMP-TASK-NEXT-BYTE-%u", (type_unsigned_32)index) < 0 ||
            timing_80286_manifest_run_task_transfer_recipe(
                TASK_SWITCH_CASE_CALL_SUCCESS, call_key, (type_unsigned_8)index,
                177u) ||
            timing_80286_manifest_run_task_transfer_recipe(
                TASK_SWITCH_CASE_SUCCESS, jmp_key, (type_unsigned_8)index,
                175u)) {
            STD_PRINTF("M5:T436:S6:I286-TASK-NEXT-BYTE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    if (timing_80286_manifest_run_task_transfer_recipe(
            TASK_SWITCH_CASE_CALL_SUCCESS, "I286-CALL-TASK", 1u, 177u) ||
        timing_80286_manifest_run_task_transfer_recipe(
            TASK_SWITCH_CASE_SUCCESS, "I286-JMP-TASK", 1u, 175u) ||
        timing_80286_manifest_run_task_transfer_recipe(
            TASK_SWITCH_CASE_TASK_GATE_SUCCESS, STD_NULL, 1u, 182u)) {
        STD_PRINTF("M5:T436:S6:I286-TASK-BASE-RECIPE:FAIL\n");
        return 1;
    }
    for (index = 1u; index <= 6u; ++index) {
        C_CHAR call_key[64];

        if (STD_SNPRINTF(call_key, sizeof(call_key),
                "I286-CALL-GATE-MORE-NEXT-BYTE-%u",
                (type_unsigned_32)index) < 0 ||
            timing_80286_manifest_run_call_gate_recipe(call_key, 0x9au,
                TYPE_FALSE, (type_unsigned_8)index, 82u)) {
            STD_PRINTF("M5:T436:S6:I286-MORE-GATE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    if (timing_80286_manifest_run_call_gate_recipe("I286-CALL-GATE-MORE",
            0x9au, TYPE_FALSE, 1u, 82u)) {
        STD_PRINTF("M5:T436:S6:I286-MORE-GATE-BASE-RECIPE:FAIL\n");
        return 1;
    }
    for (index = 1u; index <= 6u; ++index) {
        C_CHAR call_key[64];
        C_CHAR jmp_key[64];

        if (STD_SNPRINTF(call_key, sizeof(call_key),
                "I286-CALL-GATE-SAME-NEXT-BYTE-%u", (type_unsigned_32)index) <
                0 || STD_SNPRINTF(jmp_key, sizeof(jmp_key),
                "I286-JMP-GATE-NEXT-BYTE-%u", (type_unsigned_32)index) < 0 ||
            timing_80286_manifest_run_call_gate_recipe(call_key, 0x9au,
                TYPE_TRUE, (type_unsigned_8)index, 41u) ||
            timing_80286_manifest_run_call_gate_recipe(jmp_key, 0xeau,
                TYPE_TRUE, (type_unsigned_8)index, 38u)) {
            STD_PRINTF("M5:T436:S6:I286-SAME-GATE-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    if (timing_80286_manifest_run_call_gate_recipe("I286-CALL-GATE-SAME",
            0x9au, TYPE_TRUE, 1u, 41u) ||
        timing_80286_manifest_run_call_gate_recipe("I286-JMP-GATE", 0xeau,
            TYPE_TRUE, 1u, 38u)) {
        STD_PRINTF("M5:T436:S6:I286-SAME-GATE-BASE-RECIPE:FAIL\n");
        return 1;
    }
    for (index = 1u; index <= 6u; ++index) {
        C_CHAR same_key[64];
        C_CHAR more_key[64];

        if (STD_SNPRINTF(same_key, sizeof(same_key),
                "I286-INT-PM-SAME-NEXT-BYTE-%u", (type_unsigned_32)index) < 0 ||
            STD_SNPRINTF(more_key, sizeof(more_key),
                "I286-INT-PM-MORE-NEXT-BYTE-%u", (type_unsigned_32)index) < 0 ||
            timing_80286_manifest_run_protected_int_same_recipe(same_key,
                (type_unsigned_8)index) ||
            timing_80286_manifest_run_protected_int_more_recipe(more_key,
                (type_unsigned_8)index)) {
            STD_PRINTF("M5:T436:S6:I286-PROTECTED-INT-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    if (timing_80286_manifest_run_protected_int_same_recipe("I286-INT-PM-SAME",
            1u) || timing_80286_manifest_run_protected_int_more_recipe(
            "I286-INT-PM-MORE", 1u)) {
        STD_PRINTF("M5:T436:S6:I286-PROTECTED-INT-BASE-RECIPE:FAIL\n");
        return 1;
    }
    for (index = 1u; index <= 6u; ++index) {
        C_CHAR key_id[64];

        if (STD_SNPRINTF(key_id, sizeof(key_id),
                "I286-INT-TASK-NEXT-BYTE-%u", (type_unsigned_32)index) < 0 ||
            timing_80286_manifest_run_task_transfer_recipe(
                TASK_SWITCH_CASE_IDT_TASK_GATE, key_id, (type_unsigned_8)index,
                167u)) {
            STD_PRINTF("M5:T436:S6:I286-INT-TASK-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    if (timing_80286_manifest_run_task_transfer_recipe(
            TASK_SWITCH_CASE_IDT_TASK_GATE, "I286-INT-TASK", 1u, 167u)) {
        STD_PRINTF("M5:T436:S6:I286-INT-TASK-BASE-RECIPE:FAIL\n");
        return 1;
    }
    for (index = 1u; index <= 6u; ++index) {
        C_CHAR key_id[64];

        if (STD_SNPRINTF(key_id, sizeof(key_id),
                "I286-RET-IRET-TASK-NEXT-BYTE-%u", (type_unsigned_32)index) <
                0 || timing_80286_manifest_run_task_iret_recipe(key_id,
                (type_unsigned_8)index)) {
            STD_PRINTF("M5:T436:S6:I286-IRET-TASK-RECIPE:FAIL:%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    if (timing_80286_manifest_run_task_iret_recipe("I286-RET-IRET-TASK", 1u)) {
        STD_PRINTF("M5:T436:S6:I286-IRET-TASK-BASE-RECIPE:FAIL\n");
        return 1;
    }
    for (index = 0u; index < sizeof(protected_system_recipes) /
        sizeof(protected_system_recipes[0]); ++index) {
        if (timing_80286_manifest_run_protected_system(
                &protected_system_recipes[index])) {
            STD_PRINTF("M5:T435:S10:I286-PROTECTED-RECIPE:FAIL:%s\n",
                protected_system_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(segment_recipes) / sizeof(segment_recipes[0]);
        ++index) {
        if (timing_80286_manifest_run(&segment_recipes[index])) {
            STD_PRINTF("M5:T435:S10:I286-SEGMENT-RECIPE:FAIL:%s\n",
                segment_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(ea_recipes) / sizeof(ea_recipes[0]); ++index) {
        if (timing_80286_manifest_run(&ea_recipes[index])) {
            STD_PRINTF("M5:T435:S10:I286-EA-RECIPE:FAIL:%s\n",
                ea_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(odd_word_recipes) /
        sizeof(odd_word_recipes[0]); ++index) {
        if (timing_80286_manifest_run(&odd_word_recipes[index])) {
            STD_PRINTF("M5:T435:S10:I286-ODD-WORD-RECIPE:FAIL:%s\n",
                odd_word_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(ea_odd_word_recipes) /
        sizeof(ea_odd_word_recipes[0]); ++index) {
        if (timing_80286_manifest_run(&ea_odd_word_recipes[index])) {
            STD_PRINTF("M5:T436:S3:I286-EA-ODD-WORD-RECIPE:FAIL:%s\n",
                ea_odd_word_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(string_recipes) / sizeof(string_recipes[0]);
        ++index) {
        if (timing_80286_manifest_run_string_recipe(&string_recipes[index], 0)) {
            STD_PRINTF("M5:T435:S10:I286-STRING-RECIPE:FAIL:%s\n",
                string_recipes[index].key_id);
            return 1;
        }
        if ((string_recipes[index].program[0] & 1u) != 0u &&
            timing_80286_manifest_run_string_recipe(&string_recipes[index], 1)) {
            STD_PRINTF("M5:T435:S10:I286-STRING-ODD-RECIPE:FAIL:%s\n",
                string_recipes[index].key_id);
            return 1;
        }
    }
    if (timing_80286_manifest_run_hlt_recipe()) {
        STD_PRINTF("M5:T435:S10:I286-HLT-RECIPE:FAIL\n");
        return 1;
    }
    for (index = 0u; index < sizeof(repeat_recipes) / sizeof(repeat_recipes[0]);
        ++index) {
        if (timing_80286_manifest_run_repeat_recipe(&repeat_recipes[index], 0)) {
            STD_PRINTF("M5:T435:S10:I286-REP-RECIPE:FAIL:%s\n",
                repeat_recipes[index].key_id);
            return 1;
        }
        if ((repeat_recipes[index].opcode & 1u) != 0u &&
            timing_80286_manifest_run_repeat_recipe(&repeat_recipes[index], 1)) {
            STD_PRINTF("M5:T435:S10:I286-REP-ODD-RECIPE:FAIL:%s\n",
                repeat_recipes[index].key_id);
            return 1;
        }
        if ((repeat_recipes[index].opcode & 1u) != 0u &&
            timing_80286_manifest_run_repeat_base_recipe(
                &repeat_recipes[index], 1)) {
            STD_PRINTF("M5:T436:S4:I286-REP-ODD-BASE-RECIPE:FAIL:%s\n",
                repeat_recipes[index].key_id);
            return 1;
        }
        if (timing_80286_manifest_run_repeat_base_recipe(
                &repeat_recipes[index], 0)) {
            STD_PRINTF("M5:T436:S4:I286-REP-BASE-RECIPE:FAIL:%s\n",
                repeat_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(lock_recipes) / sizeof(lock_recipes[0]);
        ++index) {
        if (timing_80286_manifest_run(&lock_recipes[index])) {
            STD_PRINTF("M5:T435:S10:I286-LOCK-RECIPE:FAIL:%s\n",
                lock_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(lock_recipes) / sizeof(lock_recipes[0]);
        ++index) {
        if (timing_80286_manifest_run_lock_ea_recipe(&lock_recipes[index])) {
            STD_PRINTF("M5:T435:S10:I286-LOCK-EA-RECIPE:FAIL:%s\n",
                lock_recipes[index].key_id);
            return 1;
        }
    }
    {
        const type_unsigned_32 probes = (type_unsigned_32)(sizeof(recipes) / sizeof(recipes[0]) +
            sizeof(control_recipes) / sizeof(control_recipes[0]) +
            250u +
            sizeof(protected_system_recipes) /
                sizeof(protected_system_recipes[0]) +
            sizeof(segment_recipes) / sizeof(segment_recipes[0]) +
            sizeof(ea_recipes) / sizeof(ea_recipes[0]) +
            sizeof(odd_word_recipes) / sizeof(odd_word_recipes[0]) +
            sizeof(ea_odd_word_recipes) / sizeof(ea_odd_word_recipes[0]) +
            sizeof(string_recipes) / sizeof(string_recipes[0]) +
            7u +
            1u +
            sizeof(repeat_recipes) / sizeof(repeat_recipes[0]) + 9u +
            27u +
            2u * sizeof(lock_recipes) / sizeof(lock_recipes[0]));

        const type_unsigned_32 captured = timing_80286_manifest_observed_count();
        const type_unsigned_32 s3_captured =
            timing_80286_manifest_s3_observed_count();
        const type_unsigned_32 s4_captured =
            timing_80286_manifest_s4_observed_count();
        const type_unsigned_32 s5_captured =
            timing_80286_manifest_s5_observed_count();
        const type_unsigned_32 s6_captured =
            timing_80286_manifest_s6_observed_count();
        const type_unsigned_32 s7_captured =
            timing_80286_manifest_s7_observed_count();

        if (captured > probes || !timing_80286_manifest_results_complete() ||
            !timing_80286_manifest_s3_results_complete() ||
            !timing_80286_manifest_s4_results_complete() ||
            !timing_80286_manifest_s5_results_complete() ||
            !timing_80286_manifest_s6_results_complete() ||
            !timing_80286_manifest_s7_results_complete()) {
            STD_PRINTF("M5:T436:S7:I286-GATE-DETAIL:captured=%u:probes=%u:s3=%u:s4=%u:s5=%u:s6=%u:s7=%u:s7expected=%u\n",
                captured, probes, s3_captured, s4_captured, s5_captured,
                s6_captured, s7_captured,
                timing_80286_manifest_s7_expected_count());
            for (index = 0u; index < sizeof(timing_80286_manifest_records) /
                    sizeof(timing_80286_manifest_records[0]); ++index) {
                if (timing_80286_manifest_is_s7(
                        &timing_80286_manifest_records[index]) &&
                    !timing_80286_manifest_observed[index]) {
                    STD_PRINTF("M5:T436:S7:I286-SYSTEM-MISSING:%s\n",
                        timing_80286_manifest_records[index].key_id);
                }
            }
            return 1;
        }
        if (timing_80286_manifest_write_results(
                PROJECT_TEST_80286_RESULTS_PATH, 1) != 0) {
            return 1;
        }
        STD_PRINTF("M5:T436:S2:I286-RESULT-PRODUCER:PASS:observed=%u:canonical=%u\n",
            captured, timing_80286_manifest_expected_count());
        STD_PRINTF("M5:T436:S8:I286-RESULT-CLOSURE:PASS:canonical=%u\n",
            timing_80286_manifest_expected_count());
        STD_PRINTF("M5:T436:S3:I286-NONCONTROL-OBSERVED:%u\n", s3_captured);
        STD_PRINTF("M5:T436:S3:I286-NONCONTROL-COVERAGE:PASS:canonical=%u\n",
            timing_80286_manifest_s3_expected_count());
        STD_PRINTF("M5:T436:S3:I286-MEMORY-INPUTS:PASS\n");
        STD_PRINTF("M5:T436:S4:I286-STRING-OBSERVED:%u\n", s4_captured);
        STD_PRINTF("M5:T436:S4:I286-STRING-COVERAGE:PASS:canonical=%u\n",
            timing_80286_manifest_s4_expected_count());
        STD_PRINTF("M5:T436:S4:I286-REPEAT-INPUTS:PASS\n");
        STD_PRINTF("M5:T436:S5:I286-CONDITIONAL-OBSERVED:%u\n", s5_captured);
        STD_PRINTF("M5:T436:S5:I286-CONDITIONAL-COVERAGE:PASS:canonical=%u\n",
            timing_80286_manifest_s5_expected_count());
        STD_PRINTF("M5:T436:S5:I286-OUTCOME-NEXT-BYTE:PASS\n");
        STD_PRINTF("M5:T436:S6:I286-TRANSFER-OBSERVED:%u:canonical=%u\n",
            s6_captured, timing_80286_manifest_s6_expected_count());
        STD_PRINTF("M5:T436:S6:I286-TRANSFER-COVERAGE:PASS:canonical=%u\n",
            timing_80286_manifest_s6_expected_count());
        STD_PRINTF("M5:T436:S6:I286-PROTECTED-PATHS:PASS\n");
        STD_PRINTF("M5:T436:S7:I286-SYSTEM-OBSERVED:%u:canonical=%u\n",
            s7_captured, timing_80286_manifest_s7_expected_count());
        STD_PRINTF("M5:T436:S7:I286-SYSTEM-COVERAGE:PASS:canonical=%u\n",
            timing_80286_manifest_s7_expected_count());
        STD_PRINTF("M5:T436:S7:I286-SEGMENT-DESCRIPTOR-PATHS:PASS\n");
        for (index = 0u; index < sizeof(timing_80286_manifest_records) /
                sizeof(timing_80286_manifest_records[0]); ++index) {
            if (timing_80286_manifest_is_s6(&timing_80286_manifest_records[index]) &&
                !timing_80286_manifest_observed[index]) {
                STD_PRINTF("M5:T436:S6:I286-TRANSFER-MISSING:%s\n",
                    timing_80286_manifest_records[index].key_id);
            }
        }
        for (index = 0u; index < sizeof(timing_80286_manifest_records) /
                sizeof(timing_80286_manifest_records[0]); ++index) {
            if (timing_80286_manifest_is_s7(&timing_80286_manifest_records[index]) &&
                !timing_80286_manifest_observed[index]) {
                STD_PRINTF("M5:T436:S7:I286-SYSTEM-MISSING:%s\n",
                    timing_80286_manifest_records[index].key_id);
            }
        }
        STD_PRINTF("M5:T435:S10:I286-MANIFEST-FOUNDATION:PASS:observed=%u\n",
            captured);
    }
    return 0;
}
