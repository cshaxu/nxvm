#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "core/machine/retirement_observation_interface.h"
#include "../support/core_machine_cpu_fixture.h"

/* S6 needs the same private, execution-frozen protected-mode fixture as the
 * retained gate smoke.  Keep it local to the runner; this does not widen the
 * shared fixture surface or turn an independent smoke into timing evidence. */
#define main timing_80386_manifest_retained_gate_smoke_main
#include "core_machine_protected_16_gate_s3_smoke.c"
#undef main

#define main timing_80386_manifest_retained_task_switch_smoke_main
#include "core_machine_task_switch_smoke.c"
#undef main

#define TIMING_80386_MANIFEST_RESET_LINEAR 0xfffffff0u
#define TIMING_80386_MANIFEST_RESET_PHYSICAL 0x000ffff0u
#define TIMING_80386_MANIFEST_WINDOW_BYTES 16u
#define TIMING_80386_MANIFEST_REAL_LINEAR 0x00000000u
#define TIMING_80386_MANIFEST_REAL_BYTES 0x00002000u

/* S2 owns capture and final-emission containment, not any later form family.
 * S3--S7 add recipes; this runner never treats an absent recipe as a result. */
typedef struct timing_80386_manifest_record {
    const C_CHAR *key_id;
    const C_CHAR *profile;
    const C_CHAR *level;
    const C_CHAR *source_rule;
    const C_CHAR *context;
} timing_80386_manifest_record;

typedef struct timing_80386_manifest_capture {
    core_machine_retirement_observation observation;
    type_unsigned_32 count;
} timing_80386_manifest_capture;

/* Chapter 17 explicitly directs ESC clock accounting to the selected 80287
 * or 80387 data sheet.  Preserve that L2 non-CPU range handoff rather than
 * manufacturing a scalar source_ticks observation. */
typedef struct timing_80386_manifest_esc_handoff {
    C_INT verified;
    type_unsigned_8 opcode;
    type_unsigned_8 modrm;
    type_unsigned_64 ticks_min;
    type_unsigned_64 ticks_max;
} timing_80386_manifest_esc_handoff;

static const timing_80386_manifest_record timing_80386_manifest_records[] = {
#include "cpu_timing_manifest_metadata_catalog.inc"
};
static C_INT timing_80386_manifest_observed[
    sizeof(timing_80386_manifest_records) / sizeof(timing_80386_manifest_records[0])];
static core_machine_retirement_observation timing_80386_manifest_results[
    sizeof(timing_80386_manifest_records) / sizeof(timing_80386_manifest_records[0])];
static C_INT timing_80386_manifest_current_index = -1;
static timing_80386_manifest_esc_handoff timing_80386_manifest_esc;

typedef struct timing_80386_manifest_port_state {
    type_unsigned_32 reads;
    type_unsigned_32 writes;
} timing_80386_manifest_port_state;

static type_status timing_80386_manifest_port_read(C_VOID *opaque,
    type_unsigned_16 port, type_unsigned_32 *out_value)
{
    timing_80386_manifest_port_state *state =
        (timing_80386_manifest_port_state *)opaque;

    if (state == STD_NULL || out_value == STD_NULL || port != 0x00e0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    ++state->reads;
    *out_value = 0x5au;
    return TYPE_STATUS_OK;
}

static type_status timing_80386_manifest_port_write(C_VOID *opaque,
    type_unsigned_16 port, type_unsigned_32 value)
{
    timing_80386_manifest_port_state *state =
        (timing_80386_manifest_port_state *)opaque;

    if (state == STD_NULL || port != 0x00e0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    (C_VOID)value;
    ++state->writes;
    return TYPE_STATUS_OK;
}

static const core_machine_port_provider timing_80386_manifest_ports = {
    timing_80386_manifest_port_read, timing_80386_manifest_port_write
};

static C_VOID timing_80386_manifest_execution_reset(C_VOID *opaque)
{
    (C_VOID)opaque;
}

static const core_machine_execution_provider timing_80386_manifest_execution = {
    timing_80386_manifest_execution_reset, STD_NULL
};

static C_INT timing_80386_manifest_is_i386(
    const timing_80386_manifest_record *record)
{
    return record != STD_NULL && STD_STRCMP(record->profile, "80386DX") == 0;
}

static const timing_80386_manifest_record *timing_80386_manifest_find(
    const C_CHAR *key_id)
{
    STD_SIZE_T index;

    timing_80386_manifest_current_index = -1;
    for (index = 0u; index < sizeof(timing_80386_manifest_records) /
            sizeof(timing_80386_manifest_records[0]); ++index) {
        if (STD_STRCMP(timing_80386_manifest_records[index].key_id, key_id) == 0) {
            timing_80386_manifest_current_index = (C_INT)index;
            return &timing_80386_manifest_records[index];
        }
    }
    return STD_NULL;
}

static C_INT timing_80386_manifest_key_has_suffix(const C_CHAR *key_id,
    const C_CHAR *suffix)
{
    STD_SIZE_T index;

    if (key_id == STD_NULL || suffix == STD_NULL) return 0;
    for (index = 0u; key_id[index] != '\0'; ++index) {
        if (STD_STRCMP(key_id + index, suffix) == 0) return 1;
    }
    return 0;
}

static C_INT timing_80386_manifest_key_has_prefix(const C_CHAR *key_id,
    const C_CHAR *prefix)
{
    STD_SIZE_T index;

    if (key_id == STD_NULL || prefix == STD_NULL) return 0;
    for (index = 0u; prefix[index] != '\0'; ++index) {
        if (key_id[index] != prefix[index]) return 0;
    }
    return 1;
}

static C_INT timing_80386_manifest_key_is_s3(const C_CHAR *key_id)
{
    if (key_id == STD_NULL) return 0;
    /* The 80386 manual delegates ESC execution clocks to the selected MCP;
     * S3 verifies its CPU/FPU handoff separately instead of inventing a
     * scalar observation for this manifest key. */
    if (STD_STRCMP(key_id, "I386-ESC") == 0) return 0;
    if (timing_80386_manifest_key_has_prefix(key_id, "I386-STRING-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-REP-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-IN-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-OUT-")) return 0;
    if (timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-INTO-NOT-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-STACK-") ||
        ((timing_80386_manifest_key_has_prefix(key_id, "I386-CALL-") ||
          timing_80386_manifest_key_has_prefix(key_id, "I386-JMP-") ||
          timing_80386_manifest_key_has_prefix(key_id, "I386-RET-")) &&
         (timing_80386_manifest_key_has_suffix(key_id, "-NEAR") ||
          timing_80386_manifest_key_has_suffix(key_id, "-FAR-REAL"))) ||
        STD_STRCMP(key_id, "I386-INT3-REAL") == 0 ||
        STD_STRCMP(key_id, "I386-INT-IMM-REAL") == 0 ||
        STD_STRCMP(key_id, "I386-INTO-REAL") == 0) return 0;
    if (timing_80386_manifest_key_has_prefix(key_id, "I386-CALL-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-JMP-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-RET-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-IRET-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-INT3-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-INT-IMM-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-INTO-")) return 0;
    if (timing_80386_manifest_key_has_prefix(key_id, "I386-MOV-SREG") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-MOV-LDS") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-MOV-LES") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-MOV-LFS") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-MOV-LGS") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-MOV-LSS") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-MOV-R32-CR") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-MOV-R32-DR") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-MOV-R32-TR") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-MOV-CR") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-MOV-DR") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-MOV-TR") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-ARPL-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-SYSTEM-")) return 0;
    return 1;
}

static C_INT timing_80386_manifest_key_is_s4(const C_CHAR *key_id)
{
    return key_id != STD_NULL && (timing_80386_manifest_key_has_prefix(key_id,
        "I386-STRING-") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-REP-") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-IN-") || timing_80386_manifest_key_has_prefix(key_id, "I386-OUT-"));
}

static C_INT timing_80386_manifest_key_is_s5(const C_CHAR *key_id)
{
    return key_id != STD_NULL && (timing_80386_manifest_key_has_prefix(key_id,
        "I386-STACK-") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-JCC-") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-CALL-NEAR-") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-CALL-FAR-REAL-") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-JMP-NEAR-") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-JMP-FAR-REAL-") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-RET-NEAR") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-RET-FAR-REAL") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-IRET-REAL") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-INT3-REAL") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-INT-IMM-REAL") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-INTO-REAL") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-INTO-NOT"));
}

/* S6 owns every non-real control transfer which reaches the protected-mode,
 * virtual-8086, gate, or task state machines.  Keep the partition explicit:
 * an S6 absence is diagnostic evidence, never silently absorbed by S3/S5. */
static C_INT timing_80386_manifest_key_is_s6(const C_CHAR *key_id)
{
    if (key_id == STD_NULL) return 0;
    return timing_80386_manifest_key_has_prefix(key_id, "I386-CALL-FAR-PM-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-CALL-GATE-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-CALL-TASK-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-JMP-FAR-PM-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-JMP-GATE-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-JMP-TASK-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-RET-FAR-PM-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-IRET-PM-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-IRET-TASK") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-INT3-PM-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-INT3-VM86-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-INT3-TASK") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-INT-IMM-PM-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-INT-IMM-VM86-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-INT-IMM-TASK") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-INTO-PM-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-INTO-VM86-") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-INTO-TASK");
}

static C_INT timing_80386_manifest_key_is_s7(const C_CHAR *key_id)
{
    return key_id != STD_NULL && (timing_80386_manifest_key_has_prefix(key_id,
        "I386-MOV-SREG") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-MOV-LDS") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-MOV-LES") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-MOV-LFS") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-MOV-LGS") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-MOV-LSS") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-MOV-R32-CR") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-MOV-R32-DR") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-MOV-R32-TR") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-MOV-CR") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-MOV-DR") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-MOV-TR") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-ARPL-") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-SYSTEM-"));
}

static C_VOID timing_80386_manifest_capture_retirement(C_VOID *opaque,
    const core_machine_retirement_observation *observation)
{
    timing_80386_manifest_capture *capture =
        (timing_80386_manifest_capture *)opaque;

    if (capture == STD_NULL || observation == STD_NULL) return;
    if (capture->count == 0u) capture->observation = *observation;
    if (timing_80386_manifest_current_index >= 0 &&
        !timing_80386_manifest_observed[timing_80386_manifest_current_index]) {
        timing_80386_manifest_results[timing_80386_manifest_current_index] =
            *observation;
        timing_80386_manifest_observed[timing_80386_manifest_current_index] = 1;
    }
    timing_80386_manifest_current_index = -1;
    ++capture->count;
}

static type_unsigned_32 timing_80386_manifest_expected_count(C_VOID)
{
    STD_SIZE_T index;
    type_unsigned_32 expected = 0u;

    for (index = 0u; index < sizeof(timing_80386_manifest_records) /
            sizeof(timing_80386_manifest_records[0]); ++index) {
        if (timing_80386_manifest_is_i386(&timing_80386_manifest_records[index])) {
            ++expected;
        }
    }
    return expected;
}

static type_unsigned_32 timing_80386_manifest_observed_count(C_VOID)
{
    STD_SIZE_T index;
    type_unsigned_32 observed = 0u;

    for (index = 0u; index < sizeof(timing_80386_manifest_records) /
            sizeof(timing_80386_manifest_records[0]); ++index) {
        if (timing_80386_manifest_is_i386(&timing_80386_manifest_records[index]) &&
            timing_80386_manifest_observed[index]) ++observed;
    }
    return observed;
}

static C_INT timing_80386_manifest_is_esc(
    const timing_80386_manifest_record *record)
{
    return timing_80386_manifest_is_i386(record) &&
        STD_STRCMP(record->key_id, "I386-ESC") == 0;
}

static type_unsigned_32 timing_80386_manifest_s3_count(C_INT observed_only)
{
    STD_SIZE_T index;
    type_unsigned_32 count = 0u;

    for (index = 0u; index < sizeof(timing_80386_manifest_records) /
            sizeof(timing_80386_manifest_records[0]); ++index) {
        if (timing_80386_manifest_is_i386(&timing_80386_manifest_records[index]) &&
            timing_80386_manifest_key_is_s3(
                timing_80386_manifest_records[index].key_id) &&
            (!observed_only || timing_80386_manifest_observed[index])) ++count;
    }
    return count;
}

static type_unsigned_32 timing_80386_manifest_s4_count(C_INT observed_only)
{
    STD_SIZE_T index;
    type_unsigned_32 count = 0u;

    for (index = 0u; index < sizeof(timing_80386_manifest_records) /
            sizeof(timing_80386_manifest_records[0]); ++index) {
        if (timing_80386_manifest_is_i386(&timing_80386_manifest_records[index]) &&
            timing_80386_manifest_key_is_s4(timing_80386_manifest_records[index].key_id) &&
            (!observed_only || timing_80386_manifest_observed[index])) ++count;
    }
    return count;
}

static type_unsigned_32 timing_80386_manifest_s5_count(C_INT observed_only)
{
    STD_SIZE_T index;
    type_unsigned_32 count = 0u;

    for (index = 0u; index < sizeof(timing_80386_manifest_records) /
            sizeof(timing_80386_manifest_records[0]); ++index) {
        if (timing_80386_manifest_is_i386(&timing_80386_manifest_records[index]) &&
            timing_80386_manifest_key_is_s5(timing_80386_manifest_records[index].key_id) &&
            (!observed_only || timing_80386_manifest_observed[index])) ++count;
    }
    return count;
}

static type_unsigned_32 timing_80386_manifest_s6_count(C_INT observed_only)
{
    STD_SIZE_T index;
    type_unsigned_32 count = 0u;

    for (index = 0u; index < sizeof(timing_80386_manifest_records) /
            sizeof(timing_80386_manifest_records[0]); ++index) {
        if (timing_80386_manifest_is_i386(&timing_80386_manifest_records[index]) &&
            timing_80386_manifest_key_is_s6(timing_80386_manifest_records[index].key_id) &&
            (!observed_only || timing_80386_manifest_observed[index])) ++count;
    }
    return count;
}

static type_unsigned_32 timing_80386_manifest_s7_count(C_INT observed_only)
{
    STD_SIZE_T index; type_unsigned_32 count = 0u;
    for (index = 0u; index < sizeof(timing_80386_manifest_records) /
            sizeof(timing_80386_manifest_records[0]); ++index) {
        if (timing_80386_manifest_is_i386(&timing_80386_manifest_records[index]) &&
            timing_80386_manifest_key_is_s7(timing_80386_manifest_records[index].key_id) &&
            (!observed_only || timing_80386_manifest_observed[index])) ++count;
    }
    return count;
}

static C_VOID timing_80386_manifest_print_missing_s3(C_VOID)
{
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(timing_80386_manifest_records) /
            sizeof(timing_80386_manifest_records[0]); ++index) {
        if (timing_80386_manifest_is_i386(&timing_80386_manifest_records[index]) &&
            timing_80386_manifest_key_is_s3(
                timing_80386_manifest_records[index].key_id) &&
            !timing_80386_manifest_observed[index]) {
            STD_PRINTF("M5:T437:S3:I386-NONCONTROL-MISSING:%s\n",
                timing_80386_manifest_records[index].key_id);
        }
    }
}

static C_VOID timing_80386_manifest_print_missing_s6(C_VOID)
{
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(timing_80386_manifest_records) /
            sizeof(timing_80386_manifest_records[0]); ++index) {
        if (timing_80386_manifest_is_i386(&timing_80386_manifest_records[index]) &&
            timing_80386_manifest_key_is_s6(
                timing_80386_manifest_records[index].key_id) &&
            !timing_80386_manifest_observed[index]) {
            STD_PRINTF("M5:T437:S6:I386-PROTECTED-MISSING:%s\n",
                timing_80386_manifest_records[index].key_id);
        }
    }
}

static C_VOID timing_80386_manifest_print_missing_s7(C_VOID)
{
    STD_SIZE_T index;
    for (index = 0u; index < sizeof(timing_80386_manifest_records) /
            sizeof(timing_80386_manifest_records[0]); ++index) {
        if (timing_80386_manifest_is_i386(&timing_80386_manifest_records[index]) &&
            timing_80386_manifest_key_is_s7(timing_80386_manifest_records[index].key_id) &&
            !timing_80386_manifest_observed[index]) STD_PRINTF(
                "M5:T437:S7:I386-PROTECTED-SYSTEM-MISSING:%s\n",
                timing_80386_manifest_records[index].key_id);
    }
}

static C_INT timing_80386_manifest_results_complete(C_VOID)
{
    return timing_80386_manifest_expected_count() != 0u &&
        timing_80386_manifest_observed_count() + 1u ==
            timing_80386_manifest_expected_count() &&
        timing_80386_manifest_esc.verified;
}

/* The completeness check precedes any file operation.  Later S units may use
 * this writer only after supplying a real observation for every I386 key. */
static C_INT timing_80386_manifest_write_results(const C_CHAR *path,
    C_INT final_results_authorized)
{
    STD_FILE *file;
    STD_SIZE_T index;
    STD_SIZE_T written = 0u;

    if (path == STD_NULL || !final_results_authorized ||
        !timing_80386_manifest_results_complete()) return 1;
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL || STD_FPRINTF(file,
            "{\n  \"schema\": \"nxvm.cpu-timing-results.v1\",\n"
            "  \"profile\": \"80386DX\",\n  \"results\": [\n") < 0) {
        if (file != STD_NULL) STD_FCLOSE(file);
        return 1;
    }
    for (index = 0u; index < sizeof(timing_80386_manifest_records) /
            sizeof(timing_80386_manifest_records[0]); ++index) {
        const timing_80386_manifest_record *record =
            &timing_80386_manifest_records[index];
        const core_machine_retirement_observation *observation =
            &timing_80386_manifest_results[index];

        if (!timing_80386_manifest_is_i386(record) ||
            timing_80386_manifest_is_esc(record)) continue;
        if ((written != 0u && STD_FPRINTF(file, ",\n") < 0) ||
            STD_FPRINTF(file, "    {\"key_id\":\"%s\","
                "\"profile\":\"%s\",\"level\":\"%s\","
                "\"source_rule\":\"%s\",\"context\":\"%s\","
                "\"timing_domain\":\"cpu\",\"ticks\":%llu,\"formula_inputs\":%u,"
                "\"form_id\":%u,\"retirement_origin\":%d,"
                "\"source_timing_unallocated\":%s,\"passed\":true}",
                record->key_id, record->profile, record->level,
                record->source_rule, record->context, observation->source_ticks,
                observation->formula_inputs, observation->source_timing_form_id,
                observation->timing_origin, observation->timing_disposition ==
                    CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED ?
                    "true" : "false") < 0) {
            STD_FCLOSE(file);
            return 1;
        }
        ++written;
    }
    if ((written != 0u && STD_FPRINTF(file, ",\n") < 0) ||
        STD_FPRINTF(file, "    {\"key_id\":\"I386-ESC\","
            "\"profile\":\"80386DX\",\"level\":\"L2\","
            "\"source_rule\":\"I386DX-PRM-1990 Ch.17 delegates completion clocks to 80287/80387 data sheets; selected 80387 operation is a range\","
            "\"context\":\"BASE\",\"timing_domain\":\"mcp\","
            "\"ticks\":null,\"formula_inputs\":0,\"form_id\":4294967295,"
            "\"retirement_origin\":0,\"source_timing_unallocated\":false,"
            "\"coprocessor_profile\":\"80387\","
            "\"escape_opcode\":%u,\"escape_modrm\":%u,"
            "\"coprocessor_ticks_min\":%llu,\"coprocessor_ticks_max\":%llu,"
            "\"handoff_kind\":\"CPU_FPU_COMMAND\",\"passed\":true}",
            timing_80386_manifest_esc.opcode, timing_80386_manifest_esc.modrm,
            timing_80386_manifest_esc.ticks_min,
            timing_80386_manifest_esc.ticks_max) < 0) {
        STD_FCLOSE(file);
        return 1;
    }
    ++written;
    if (STD_FPRINTF(file, "\n  ]\n}\n") < 0 || STD_FCLOSE(file) != 0) return 1;
    return written == timing_80386_manifest_expected_count() ? 0 : 1;
}

static C_INT timing_80386_manifest_run_recipe(const C_CHAR *key_id,
    const type_unsigned_8 *program, STD_SIZE_T program_bytes);

static C_INT timing_80386_manifest_key_is_s5_real_control(const C_CHAR *key_id)
{
    return key_id != STD_NULL && (timing_80386_manifest_key_has_prefix(key_id,
        "I386-STACK-") || timing_80386_manifest_key_has_prefix(key_id,
        "I386-JCC-") || timing_80386_manifest_key_has_suffix(key_id, "-NEAR-REL") ||
        timing_80386_manifest_key_has_suffix(key_id, "-NEAR-R") ||
        timing_80386_manifest_key_has_suffix(key_id, "-NEAR-M") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-CALL-FAR-REAL") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-JMP-FAR-REAL") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-RET-NEAR") ||
        timing_80386_manifest_key_has_prefix(key_id, "I386-RET-FAR-REAL") ||
        STD_STRCMP(key_id, "I386-IRET-REAL") == 0 ||
        STD_STRCMP(key_id, "I386-INT3-REAL") == 0 ||
        STD_STRCMP(key_id, "I386-INT-IMM-REAL") == 0 ||
        STD_STRCMP(key_id, "I386-INTO-REAL") == 0 ||
        STD_STRCMP(key_id, "I386-INTO-NOT") == 0);
}

static type_status timing_80386_manifest_prepare_recipe_machine(
    core_machine *machine, const C_CHAR *key_id, const type_unsigned_8 *program,
    STD_SIZE_T program_bytes)
{
    type_status status;

    status = core_machine_memory_write(machine, TIMING_80386_MANIFEST_RESET_PHYSICAL,
        program, program_bytes);
    if (status == TYPE_STATUS_OK && timing_80386_manifest_key_has_prefix(key_id,
            "I386-XLAT")) {
        static const type_unsigned_8 xlat_value[] = { 0x5au };

        machine->executor_cpu.data.ebx = 0x1000u;
        machine->executor_cpu.data.eax = 1u;
        status = core_machine_memory_write(machine, 0x1001u, xlat_value,
            sizeof(xlat_value));
    }
    if (status == TYPE_STATUS_OK && key_id[5] == 'M' &&
        (key_id[6] == 'U' || key_id[6] == 'I')) {
        type_unsigned_32 multiplier = 2u;

        if (timing_80386_manifest_key_has_suffix(key_id, "-MULTIPLIER-ZERO")) {
            multiplier = 0u;
        } else if (timing_80386_manifest_key_has_suffix(key_id,
                "-MULTIPLIER-HIGH")) {
            multiplier = 0x8000u;
        }
        machine->executor_cpu.data.ecx = multiplier;
        status = core_machine_memory_write(machine, 0x1000u, &multiplier,
            sizeof(multiplier));
    }
    if (status == TYPE_STATUS_OK &&
        ((key_id[5] == 'D' && key_id[6] == 'I' && key_id[7] == 'V') ||
         (key_id[5] == 'I' && key_id[6] == 'D' && key_id[7] == 'I' &&
          key_id[8] == 'V'))) {
        static const type_unsigned_32 divisor = 2u;

        machine->executor_cpu.data.eax = 4u;
        machine->executor_cpu.data.edx = 0u;
        machine->executor_cpu.data.ecx = divisor;
        status = core_machine_memory_write(machine, 0x1000u, &divisor,
            sizeof(divisor));
    }
    if (status == TYPE_STATUS_OK && key_id[5] == 'B' && key_id[6] == 'S' &&
        (key_id[7] == 'F' || key_id[7] == 'R')) {
        static const type_unsigned_32 scanned_value = 2u;

        machine->executor_cpu.data.ecx = scanned_value;
        status = core_machine_memory_write(machine, 0x1000u, &scanned_value,
            sizeof(scanned_value));
    }
    if (status == TYPE_STATUS_OK && key_id[5] == 'S' && key_id[6] == 'H' &&
        key_id[7] == 'L' && (key_id[8] == 'D' || key_id[8] == 'R')) {
        machine->executor_cpu.data.ecx = 1u;
    }
    if (status == TYPE_STATUS_OK && (timing_80386_manifest_key_has_prefix(key_id,
            "I386-IN-") || timing_80386_manifest_key_has_prefix(key_id,
            "I386-OUT-") || timing_80386_manifest_key_has_prefix(key_id,
            "I386-STRING-INS") || timing_80386_manifest_key_has_prefix(key_id,
            "I386-STRING-OUTS") || timing_80386_manifest_key_has_prefix(key_id,
            "I386-REP-INS") || timing_80386_manifest_key_has_prefix(key_id,
            "I386-REP-OUTS"))) {
        machine->executor_cpu.data.edx = 0x00e0u;
    }
    if (status == TYPE_STATUS_OK && (timing_80386_manifest_key_has_prefix(key_id,
            "I386-STRING-") || timing_80386_manifest_key_has_prefix(key_id,
            "I386-REP-"))) {
        static const type_unsigned_32 string_input = 0x11223344u;

        machine->executor_cpu.data.esi = 0x1000u;
        machine->executor_cpu.data.edi = 0x1100u;
        machine->executor_cpu.data.ecx = timing_80386_manifest_key_has_prefix(
            key_id, "I386-REP-") ? 1u : 0u;
        if (timing_80386_manifest_key_has_suffix(key_id, "-REP-PHASE-ZERO")) {
            machine->executor_cpu.data.ecx = 0u;
        } else if (timing_80386_manifest_key_has_suffix(key_id,
                "-REP-PHASE-CONTINUE")) {
            machine->executor_cpu.data.ecx = 2u;
        }
        status = core_machine_memory_write(machine, 0x1000u, &string_input,
            sizeof(string_input));
        if (status == TYPE_STATUS_OK && (timing_80386_manifest_key_has_prefix(key_id,
                "I386-REP-CMPS-REPE") || timing_80386_manifest_key_has_prefix(key_id,
                "I386-REP-SCAS-"))) {
            status = core_machine_memory_write(machine, 0x1100u, &string_input,
                sizeof(string_input));
        }
        if (timing_80386_manifest_key_has_prefix(key_id, "I386-REP-SCAS-REPE")) {
            machine->executor_cpu.data.eax = string_input;
        }
    }
    return status;
}

static C_INT timing_80386_manifest_run_size_contexts(const C_CHAR *base_key_id,
    const type_unsigned_8 *program, STD_SIZE_T program_bytes)
{
    C_CHAR key_id[96];
    type_unsigned_8 size32_program[32];
    STD_SIZE_T input_index;
    STD_SIZE_T output_index = 0u;
    C_INT expanded = 0;

    if (base_key_id == STD_NULL || program == STD_NULL || program_bytes == 0u ||
        STD_SNPRINTF(key_id, sizeof(key_id), "%s-SIZE16", base_key_id) < 0) {
        return 1;
    }
    if (timing_80386_manifest_find(key_id) != STD_NULL &&
        timing_80386_manifest_run_recipe(key_id, program, program_bytes)) return 1;
    if (STD_SNPRINTF(key_id, sizeof(key_id), "%s-SIZE32", base_key_id) < 0 ||
        timing_80386_manifest_find(key_id) == STD_NULL) return 0;
    size32_program[output_index++] = 0x67u;
    for (input_index = 0u; input_index < program_bytes; ++input_index) {
        if (input_index + 2u < program_bytes &&
            (program[input_index] & 0xc7u) == 0x06u &&
            program[input_index + 1u] == 0u && program[input_index + 2u] == 0x10u) {
            if (output_index + 5u > sizeof(size32_program)) return 1;
            size32_program[output_index++] =
                (type_unsigned_8)((program[input_index] & 0x38u) | 0x05u);
            size32_program[output_index++] = 0u;
            size32_program[output_index++] = 0x10u;
            size32_program[output_index++] = 0u;
            size32_program[output_index++] = 0u;
            input_index += 2u;
            expanded = 1;
        } else {
            if (output_index >= sizeof(size32_program)) return 1;
            size32_program[output_index++] = program[input_index];
        }
    }
    if (!expanded) {
        for (input_index = 1u; input_index + 2u < output_index; ++input_index) {
            if ((size32_program[input_index] == 0xa0u ||
                 size32_program[input_index] == 0xa1u ||
                 size32_program[input_index] == 0xa2u ||
                 size32_program[input_index] == 0xa3u) &&
                size32_program[input_index + 1u] == 0u &&
                size32_program[input_index + 2u] == 0x10u) {
                if (output_index + 2u > sizeof(size32_program)) return 1;
                size32_program[input_index + 3u] = 0u;
                size32_program[input_index + 4u] = 0u;
                output_index += 2u;
                break;
            }
        }
    }
    return timing_80386_manifest_run_recipe(key_id, size32_program, output_index);
}

static C_INT timing_80386_manifest_verify_esc_handoff(C_VOID)
{
    static const type_unsigned_8 fadd[] = { 0xd8u, 0xc0u };
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_80387
    };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    type_status status = core_machine_create(&config, &machine);
    C_INT failed = status != TYPE_STATUS_OK;

    if (!failed) status = test_core_machine_fixture_register_reset_mapping(machine,
        TIMING_80386_MANIFEST_RESET_LINEAR, TIMING_80386_MANIFEST_RESET_PHYSICAL,
        TIMING_80386_MANIFEST_WINDOW_BYTES);
    if (!failed && status == TYPE_STATUS_OK) status = core_machine_bind_execution_provider(
        machine, &timing_80386_manifest_execution, STD_NULL);
    if (!failed && status == TYPE_STATUS_OK) status = core_machine_freeze_execution_providers(machine);
    if (!failed && status == TYPE_STATUS_OK) status = core_machine_reset(machine);
    if (!failed && status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
        TIMING_80386_MANIFEST_RESET_PHYSICAL, fadd, sizeof(fadd));
    if (!failed && status == TYPE_STATUS_OK) status = core_machine_run(machine,
        (core_machine_run_budget){ 1u, 0u }, &run);
    failed |= status != TYPE_STATUS_OK || run.executed != 1u || !machine->fpu.busy ||
        machine->fpu.last_escape_opcode != fadd[0] ||
        machine->fpu.last_escape_modrm != fadd[1] ||
        machine->fpu.operation_ticks_min != 12u ||
        machine->fpu.operation_ticks_max != 26u ||
        machine->transaction.kind != CORE_MACHINE_TRANSACTION_CPU_FPU_COMMAND;
    if (!failed) timing_80386_manifest_esc = (timing_80386_manifest_esc_handoff) {
        1, fadd[0], fadd[1], machine->fpu.operation_ticks_min,
        machine->fpu.operation_ticks_max
    };
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80386_manifest_run_recipe(const C_CHAR *key_id,
    const type_unsigned_8 *program, STD_SIZE_T program_bytes)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .ticks_per_instruction = 29u,
        .instruction_timing = { 29u, 7u, 31u, 37u, 41u, 43u }
    };
    const core_machine_retirement_observation_provider provider = {
        timing_80386_manifest_capture_retirement, STD_NULL
    };
    core_machine_retirement_observation_provider active_provider = provider;
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result run = { 0 };
    timing_80386_manifest_capture capture = { { 0 }, 0u };
    const timing_80386_manifest_record *record;
    core_machine *machine = STD_NULL;
    timing_80386_manifest_port_state port_state = { 0u, 0u };
    type_status status;
    C_INT halt_recipe;

    record = timing_80386_manifest_find(key_id);
    if (record == STD_NULL || !timing_80386_manifest_is_i386(record) ||
        program == STD_NULL || program_bytes == 0u) return 1;
    halt_recipe = STD_STRCMP(key_id, "I386-HLT") == 0;
    active_provider.context = &capture;
    status = core_machine_create(&config, &machine);
    if (status == TYPE_STATUS_OK) status =
        test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_80386_MANIFEST_RESET_LINEAR,
            TIMING_80386_MANIFEST_RESET_PHYSICAL,
            TIMING_80386_MANIFEST_WINDOW_BYTES);
    if (status == TYPE_STATUS_OK && timing_80386_manifest_key_is_s5_real_control(key_id)) {
        status = test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_80386_MANIFEST_REAL_LINEAR,
            TIMING_80386_MANIFEST_REAL_LINEAR,
            TIMING_80386_MANIFEST_REAL_BYTES);
    }
    if (status == TYPE_STATUS_OK) status = core_machine_bind_execution_provider(
        machine, &timing_80386_manifest_execution, STD_NULL);
    if (status == TYPE_STATUS_OK) status = core_machine_install_port_provider(
        machine, 0x00e0u, 0x00e0u, &timing_80386_manifest_ports, &port_state);
    if (status == TYPE_STATUS_OK) status = core_machine_freeze_execution_providers(machine);
    if (status == TYPE_STATUS_OK) status = core_machine_reset(machine);
    if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
        TIMING_80386_MANIFEST_RESET_PHYSICAL, program, program_bytes);
    if (status == TYPE_STATUS_OK && timing_80386_manifest_key_has_prefix(key_id,
            "I386-XLAT")) {
        static const type_unsigned_8 xlat_value[] = { 0x5au };

        machine->executor_cpu.data.ebx = 0x1000u;
        machine->executor_cpu.data.eax = 1u;
        status = core_machine_memory_write(machine, 0x1001u, xlat_value,
            sizeof(xlat_value));
    }
    if (status == TYPE_STATUS_OK && key_id[5] == 'M' &&
        (key_id[6] == 'U' || key_id[6] == 'I')) {
        type_unsigned_32 multiplier = 2u;

        if (timing_80386_manifest_key_has_suffix(key_id,
                "-MULTIPLIER-ZERO")) {
            multiplier = 0u;
        } else if (timing_80386_manifest_key_has_suffix(key_id,
                "-MULTIPLIER-HIGH")) {
            multiplier = 0x8000u;
        }

        machine->executor_cpu.data.ecx = multiplier;
        status = core_machine_memory_write(machine, 0x1000u, &multiplier,
            sizeof(multiplier));
    }
    if (status == TYPE_STATUS_OK &&
        ((key_id[5] == 'D' && key_id[6] == 'I' && key_id[7] == 'V') ||
         (key_id[5] == 'I' && key_id[6] == 'D' && key_id[7] == 'I' &&
          key_id[8] == 'V'))) {
        static const type_unsigned_32 divisor = 2u;

        machine->executor_cpu.data.eax = 4u;
        machine->executor_cpu.data.edx = 0u;
        machine->executor_cpu.data.ecx = divisor;
        status = core_machine_memory_write(machine, 0x1000u, &divisor,
            sizeof(divisor));
    }
    if (status == TYPE_STATUS_OK && key_id[5] == 'B' && key_id[6] == 'S' &&
        (key_id[7] == 'F' || key_id[7] == 'R')) {
        static const type_unsigned_32 scanned_value = 2u;

        machine->executor_cpu.data.ecx = scanned_value;
        status = core_machine_memory_write(machine, 0x1000u, &scanned_value,
            sizeof(scanned_value));
    }
    if (status == TYPE_STATUS_OK && key_id[5] == 'S' && key_id[6] == 'H' &&
        key_id[7] == 'L' && (key_id[8] == 'D' || key_id[8] == 'R')) {
        machine->executor_cpu.data.ecx = 1u;
    }
    if (status == TYPE_STATUS_OK && (timing_80386_manifest_key_has_prefix(key_id,
            "I386-IN-") || timing_80386_manifest_key_has_prefix(key_id,
            "I386-OUT-") || timing_80386_manifest_key_has_prefix(key_id,
            "I386-STRING-INS") || timing_80386_manifest_key_has_prefix(key_id,
            "I386-STRING-OUTS") || timing_80386_manifest_key_has_prefix(key_id,
            "I386-REP-INS") || timing_80386_manifest_key_has_prefix(key_id,
            "I386-REP-OUTS"))) {
        machine->executor_cpu.data.edx = 0x00e0u;
    }
    if (status == TYPE_STATUS_OK && (timing_80386_manifest_key_has_prefix(key_id,
            "I386-STRING-") || timing_80386_manifest_key_has_prefix(key_id,
            "I386-REP-"))) {
        static const type_unsigned_32 string_input = 0x11223344u;

        machine->executor_cpu.data.esi = 0x1000u;
        machine->executor_cpu.data.edi = 0x1100u;
        machine->executor_cpu.data.ecx = timing_80386_manifest_key_has_prefix(
            key_id, "I386-REP-") ? 1u : 0u;
        if (timing_80386_manifest_key_has_suffix(key_id, "-REP-PHASE-ZERO")) {
            machine->executor_cpu.data.ecx = 0u;
        }
        status = core_machine_memory_write(machine, 0x1000u, &string_input,
            sizeof(string_input));
        if (status == TYPE_STATUS_OK && (timing_80386_manifest_key_has_prefix(key_id,
                "I386-REP-CMPS-REPE") || timing_80386_manifest_key_has_prefix(key_id,
                "I386-REP-SCAS-"))) {
            status = core_machine_memory_write(machine, 0x1100u, &string_input,
                sizeof(string_input));
        }
        if (timing_80386_manifest_key_has_prefix(key_id, "I386-REP-SCAS-REPE")) {
            machine->executor_cpu.data.eax = string_input;
        }
    }
    if (status == TYPE_STATUS_OK && timing_80386_manifest_key_has_prefix(key_id,
            "I386-JCC-")) {
        type_unsigned_32 flags = 0u;
        C_INT taken = timing_80386_manifest_key_has_suffix(key_id, "-TAKEN");

        if (timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JO-")) {
            flags = taken ? VCPU_EFLAGS_OF : 0u;
        } else if (timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JNO-")) {
            flags = taken ? 0u : VCPU_EFLAGS_OF;
        } else if (timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JB-") ||
            timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JAE-") ||
            timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JBE-") ||
            timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JA-")) {
            flags = (timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JB-") ||
                timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JBE-")) == taken ?
                VCPU_EFLAGS_CF : 0u;
        } else if (timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JE-") ||
            timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JNE-") ||
            timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JLE-") ||
            timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JG-")) {
            flags = (timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JE-") ||
                timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JLE-")) == taken ?
                VCPU_EFLAGS_ZF : 0u;
        } else if (timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JS-") ||
            timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JNS-") ||
            timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JL-")) {
            flags = (timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JS-") ||
                timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JL-")) == taken ?
                VCPU_EFLAGS_SF : 0u;
        } else if (timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JP-") ||
            timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JNP-")) {
            flags = (timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JP-")) == taken ?
                VCPU_EFLAGS_PF : 0u;
        }
        machine->executor_cpu.data.eflags = flags;
        if (timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JCXZ-")) {
            machine->executor_cpu.data.ecx = taken ? 0u : 1u;
        } else if (timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-JECXZ-")) {
            machine->executor_cpu.data.ecx = taken ? 0u : 1u;
        } else if (timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-LOOP")) {
            machine->executor_cpu.data.ecx = taken ? 2u : 1u;
        }
        if (timing_80386_manifest_key_has_prefix(key_id, "I386-JCC-LOOPE-") &&
            taken) machine->executor_cpu.data.eflags = VCPU_EFLAGS_ZF;
    }
    if (status == TYPE_STATUS_OK && STD_STRCMP(key_id, "I386-INTO-REAL") == 0) {
        machine->executor_cpu.data.eflags = VCPU_EFLAGS_OF;
    }
    if (status == TYPE_STATUS_OK) status =
        core_machine_set_retirement_observation_provider(machine, &active_provider);
    if (status == TYPE_STATUS_OK) status = core_machine_run(machine, budget, &run);
    if (status != TYPE_STATUS_OK || run.reason != (halt_recipe ?
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT : CORE_MACHINE_STOP_BUDGET) ||
        run.executed != 1u || capture.count != 1u ||
        capture.observation.timing_disposition ==
            CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED ||
        capture.observation.source_timing_form_id ==
            CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
        capture.observation.timing_origin ==
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED ||
        (timing_80386_manifest_key_has_suffix(key_id, "-REP-PHASE-FIRST") &&
         capture.observation.repeat_phase != CORE_MACHINE_RETIREMENT_REPEAT_FIRST) ||
        (timing_80386_manifest_key_has_suffix(key_id, "-REP-PHASE-ZERO") &&
         capture.observation.repeat_phase != CORE_MACHINE_RETIREMENT_REPEAT_ZERO_COUNT)) {
        STD_PRINTF("M5:T437:S3:I386-RECIPE-DETAIL:%s:status=%d:reason=%d:executed=%llu:capture=%u:disposition=%d:form=%u:origin=%d\n",
            key_id, status, run.reason, run.executed, capture.count,
            capture.observation.timing_disposition,
            capture.observation.source_timing_form_id,
            capture.observation.timing_origin);
        core_machine_destroy(machine);
        return 1;
    }
    core_machine_destroy(machine);
    return 0;
}

/* S7 deliberately starts from the retained 80386 protected gate fixture,
 * rather than borrowing a 80286 runner.  The fixture supplies a frozen
 * protected-mode machine; this wrapper supplies the legal data segments and
 * captures exactly one 80386 retirement. */
static C_INT timing_80386_manifest_run_s7_protected_recipe(
    const C_CHAR *key_id, const type_unsigned_8 *program, STD_SIZE_T program_bytes,
    const C_VOID *operand, STD_SIZE_T operand_bytes, type_unsigned_64 expected_ticks)
{
    const core_machine_retirement_observation_provider provider = {
        timing_80386_manifest_capture_retirement, STD_NULL
    };
    core_machine_retirement_observation_provider active_provider = provider;
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result run = { 0 };
    timing_80386_manifest_capture capture = { { 0 }, 0u };
    const timing_80386_manifest_record *record;
    s3_gate_machine state;
    t_cpu *cpu;
    type_status status = TYPE_STATUS_OK;

    record = timing_80386_manifest_find(key_id);
    if (record == STD_NULL || !timing_80386_manifest_is_i386(record) ||
        program == STD_NULL || program_bytes == 0u) return 1;
    if (!s3_gate_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE)) return 1;
    cpu = &state.machine->executor_cpu;
    cpu->data.ds.selector = 0x0010u;
    cpu->data.ds.base = 0u;
    cpu->data.ds.limit = 0xffffu;
    cpu->data.ds.dpl = 0u;
    cpu->data.ds.flagValid = TYPE_TRUE;
    cpu->data.ds.sregtype = SREG_DATA;
    cpu->data.ds.seg.executable = TYPE_FALSE;
    cpu->data.ds.seg.data.writable = TYPE_TRUE;
    cpu->data.es = cpu->data.ds;
    /* The inherited gate smoke enables TF to test interrupt delivery.  S7
     * measures the instruction itself, so suppress the unrelated #DB path. */
    cpu->data.eflags = VCPU_EFLAGS_CF;
    cpu->data.eax = 1u;
    if (timing_80386_manifest_key_has_prefix(key_id, "I386-MOV-CR3-R32"))
        cpu->data.eax = 0x00001000u;
    cpu->data.ecx = 0x0010u;
    if (timing_80386_manifest_key_has_prefix(key_id, "I386-SYSTEM-LLDT")) {
        static const type_unsigned_8 ldt_descriptor[] = {
            0x17u,0u,0u,0x09u,0u,0x82u,0u,0u
        };

        cpu->data.gdtr.limit = 0x002fu;
        cpu->data.ecx = 0x0018u;
        if (!s3_gate_write(&state, S3_GDT_BASE + 0x18u, ldt_descriptor,
                sizeof(ldt_descriptor))) {
            core_machine_destroy(state.machine);
            return 1;
        }
    }
    if (timing_80386_manifest_key_has_prefix(key_id, "I386-SYSTEM-LTR")) {
        static const type_unsigned_8 tss_descriptor[] = {
            0x67u,0u,0u,0x06u,0u,0x89u,0u,0u
        };

        cpu->data.gdtr.limit = 0x002fu;
        cpu->data.ecx = 0x0020u;
        if (!s3_gate_write(&state, S3_GDT_BASE + 0x20u, tss_descriptor,
                sizeof(tss_descriptor))) {
            core_machine_destroy(state.machine);
            return 1;
        }
    }
    if (timing_80386_manifest_key_has_prefix(key_id, "I386-SYSTEM-STR")) {
        static const type_unsigned_8 tss_descriptor[] = {
            0x67u,0u,0u,0x06u,0u,0x89u,0u,0u
        };
        static const type_unsigned_8 ltr[] = { 0x0fu,0x00u,0xd9u };
        core_machine_run_result precondition = { 0 };

        cpu->data.gdtr.limit = 0x002fu;
        cpu->data.ecx = 0x0020u;
        if (!s3_gate_write(&state, S3_GDT_BASE + 0x20u, tss_descriptor,
                sizeof(tss_descriptor)) || !s3_gate_write(&state, S3_CODE_BASE,
                ltr, sizeof(ltr)) || core_machine_run(state.machine,
                (core_machine_run_budget){ 1u,0u }, &precondition) != TYPE_STATUS_OK ||
            precondition.reason != CORE_MACHINE_STOP_BUDGET ||
            precondition.executed != 1u) {
            core_machine_destroy(state.machine);
            return 1;
        }
        cpu->data.eip = 0u;
        cpu->data.flagHalt = TYPE_FALSE;
    }
    if (!s3_gate_write(&state, S3_CODE_BASE, program, program_bytes)) {
        core_machine_destroy(state.machine);
        return 1;
    }
    if (operand != STD_NULL && operand_bytes != 0u &&
        !s3_gate_write(&state, 0x1000u, operand, operand_bytes)) {
        core_machine_destroy(state.machine);
        return 1;
    }
    active_provider.context = &capture;
    status = core_machine_set_retirement_observation_provider(state.machine,
        &active_provider);
    if (status == TYPE_STATUS_OK) status = core_machine_run(state.machine, budget, &run);
    if (status != TYPE_STATUS_OK || run.reason != CORE_MACHINE_STOP_BUDGET ||
        run.executed != 1u || capture.count != 1u ||
        capture.observation.timing_disposition ==
            CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED ||
        capture.observation.source_ticks != expected_ticks) {
        STD_PRINTF("M5:T437:S7:I386-PROTECTED-RECIPE-DETAIL:%s:status=%d:reason=%d:executed=%llu:capture=%u:ticks=%llu:expected=%llu\\n",
            key_id, status, run.reason, run.executed, capture.count,
            capture.observation.source_ticks, expected_ticks);
        core_machine_destroy(state.machine);
        return 1;
    }
    core_machine_destroy(state.machine);
    return 0;
}

static C_INT timing_80386_manifest_run_s7_arpl_recipes(C_VOID)
{
    static const type_unsigned_8 arpl_r[] = { 0x63u, 0xc8u };
    static const type_unsigned_8 arpl_m[] = { 0x63u, 0x0eu, 0u, 0x10u };
    static const type_unsigned_8 arpl_m_segment[] = {
        0x26u, 0x63u, 0x0eu, 0u, 0x10u
    };
    static const type_unsigned_16 selector = 0x0001u;

    return timing_80386_manifest_run_s7_protected_recipe("I386-ARPL-R",
            arpl_r, sizeof(arpl_r), STD_NULL, 0u, 20u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-ARPL-M",
            arpl_m, sizeof(arpl_m), &selector, sizeof(selector), 21u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-ARPL-M-SEGMENT",
            arpl_m_segment, sizeof(arpl_m_segment), &selector, sizeof(selector), 21u);
}

static C_INT timing_80386_manifest_run_s7_lar_lsl_recipes(C_VOID)
{
    static const type_unsigned_8 lar_r16[] = { 0x0fu, 0x02u, 0xc1u };
    static const type_unsigned_8 lar_m16[] = { 0x0fu, 0x02u, 0x06u, 0u, 0x10u };
    static const type_unsigned_8 lar_r32[] = { 0x66u, 0x0fu, 0x02u, 0xc1u };
    static const type_unsigned_8 lar_m32[] = {
        0x66u, 0x0fu, 0x02u, 0x06u, 0u, 0x10u
    };
    static const type_unsigned_8 lar_m16_segment[] = {
        0x26u, 0x0fu, 0x02u, 0x06u, 0u, 0x10u
    };
    static const type_unsigned_8 lar_m32_segment[] = {
        0x26u, 0x66u, 0x0fu, 0x02u, 0x06u, 0u, 0x10u
    };
    static const type_unsigned_8 lsl_r16[] = { 0x0fu, 0x03u, 0xc1u };
    static const type_unsigned_8 lsl_m16[] = { 0x0fu, 0x03u, 0x06u, 0u, 0x10u };
    static const type_unsigned_8 lsl_r32[] = { 0x66u, 0x0fu, 0x03u, 0xc1u };
    static const type_unsigned_8 lsl_m32[] = {
        0x66u, 0x0fu, 0x03u, 0x06u, 0u, 0x10u
    };
    static const type_unsigned_8 lsl_m16_segment[] = {
        0x26u, 0x0fu, 0x03u, 0x06u, 0u, 0x10u
    };
    static const type_unsigned_8 lsl_m32_segment[] = {
        0x26u, 0x66u, 0x0fu, 0x03u, 0x06u, 0u, 0x10u
    };
    static const type_unsigned_16 selector = 0x0010u;

    return timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LAR-R16",
            lar_r16, sizeof(lar_r16), STD_NULL, 0u, 15u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LAR-M16",
            lar_m16, sizeof(lar_m16), &selector, sizeof(selector), 16u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LAR-R32",
            lar_r32, sizeof(lar_r32), STD_NULL, 0u, 15u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LAR-M32",
            lar_m32, sizeof(lar_m32), &selector, sizeof(selector), 16u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LAR-M16-SEGMENT",
            lar_m16_segment, sizeof(lar_m16_segment), &selector, sizeof(selector), 16u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LAR-M32-SEGMENT",
            lar_m32_segment, sizeof(lar_m32_segment), &selector, sizeof(selector), 16u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LSL-R16",
            lsl_r16, sizeof(lsl_r16), STD_NULL, 0u, 21u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LSL-M16",
            lsl_m16, sizeof(lsl_m16), &selector, sizeof(selector), 22u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LSL-R32",
            lsl_r32, sizeof(lsl_r32), STD_NULL, 0u, 21u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LSL-M32",
            lsl_m32, sizeof(lsl_m32), &selector, sizeof(selector), 22u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LSL-M16-SEGMENT",
            lsl_m16_segment, sizeof(lsl_m16_segment), &selector, sizeof(selector), 22u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LSL-M32-SEGMENT",
            lsl_m32_segment, sizeof(lsl_m32_segment), &selector, sizeof(selector), 22u);
}

static C_INT timing_80386_manifest_run_s7_verify_recipes(C_VOID)
{
    static const type_unsigned_8 verr_r[] = { 0x0fu, 0x00u, 0xe1u };
    static const type_unsigned_8 verr_m[] = { 0x0fu, 0x00u, 0x26u, 0u, 0x10u };
    static const type_unsigned_8 verr_m_segment[] = {
        0x26u, 0x0fu, 0x00u, 0x26u, 0u, 0x10u
    };
    static const type_unsigned_8 verw_r[] = { 0x0fu, 0x00u, 0xe9u };
    static const type_unsigned_8 verw_m[] = { 0x0fu, 0x00u, 0x2eu, 0u, 0x10u };
    static const type_unsigned_8 verw_m_segment[] = {
        0x26u, 0x0fu, 0x00u, 0x2eu, 0u, 0x10u
    };
    static const type_unsigned_16 selector = 0x0010u;

    return timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-VERR-R",
            verr_r, sizeof(verr_r), STD_NULL, 0u, 10u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-VERR-M",
            verr_m, sizeof(verr_m), &selector, sizeof(selector), 11u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-VERR-M-SEGMENT",
            verr_m_segment, sizeof(verr_m_segment), &selector, sizeof(selector), 11u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-VERW-R",
            verw_r, sizeof(verw_r), STD_NULL, 0u, 15u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-VERW-M",
            verw_m, sizeof(verw_m), &selector, sizeof(selector), 16u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-VERW-M-SEGMENT",
            verw_m_segment, sizeof(verw_m_segment), &selector, sizeof(selector), 16u);
}

static C_INT timing_80386_manifest_run_s7_clts_recipe(C_VOID)
{
    static const type_unsigned_8 clts[] = { 0x0fu, 0x06u };

    return timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-CLTS",
        clts, sizeof(clts), STD_NULL, 0u, 6u);
}

static C_INT timing_80386_manifest_run_s7_descriptor_table_recipes(C_VOID)
{
    static const C_CHAR *const names[] = { "LGDT", "LIDT", "SGDT", "SIDT" };
    static const type_unsigned_8 extensions[] = { 2u, 3u, 0u, 1u };
    static const type_unsigned_8 table_pointer[] = { 0x1fu, 0u, 0u, 3u, 0u, 0u };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(names) / sizeof(names[0]); ++index) {
        C_CHAR key_id[64];
        type_unsigned_8 program[7];
        type_unsigned_64 ticks = index < 2u ? 11u : 9u;
        STD_SIZE_T bytes;
        type_bool size32;
        type_bool segment;

        for (size32 = TYPE_FALSE; size32 <= TYPE_TRUE; ++size32)
        for (segment = TYPE_FALSE; segment <= TYPE_TRUE; ++segment) {
            bytes = 0u;
            if (segment) program[bytes++] = 0x26u;
            if (size32) program[bytes++] = 0x66u;
            program[bytes++] = 0x0fu;
            program[bytes++] = 0x01u;
            program[bytes++] = (type_unsigned_8)(extensions[index] << 3u | 0x06u);
            program[bytes++] = 0u;
            program[bytes++] = 0x10u;
            if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-SYSTEM-%s-M%s%s",
                    names[index], size32 ? "32" : "16",
                    segment ? "-SEGMENT" : "") < 0 ||
                timing_80386_manifest_run_s7_protected_recipe(key_id, program, bytes,
                    table_pointer, sizeof(table_pointer), ticks)) return 1;
        }
    }
    return 0;
}

static C_INT timing_80386_manifest_run_s7_smsw_recipes(C_VOID)
{
    static const type_unsigned_8 smsw_r[] = { 0x0fu, 0x01u, 0xe1u };
    static const type_unsigned_8 smsw_m[] = { 0x0fu, 0x01u, 0x26u, 0u, 0x10u };
    static const type_unsigned_8 smsw_m_segment[] = {
        0x26u, 0x0fu, 0x01u, 0x26u, 0u, 0x10u
    };

    return timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-SMSW-R",
            smsw_r, sizeof(smsw_r), STD_NULL, 0u, 2u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-SMSW-M",
            smsw_m, sizeof(smsw_m), STD_NULL, 0u, 2u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-SMSW-M-SEGMENT",
            smsw_m_segment, sizeof(smsw_m_segment), STD_NULL, 0u, 2u);
}

static C_INT timing_80386_manifest_run_s7_control_register_recipes(C_VOID)
{
    static const C_CHAR *const read_keys[] = {
        "I386-MOV-R32-CR0", "I386-MOV-R32-CR2", "I386-MOV-R32-CR3"
    };
    static const C_CHAR *const write_keys[] = {
        "I386-MOV-CR0-R32", "I386-MOV-CR2-R32", "I386-MOV-CR3-R32"
    };
    static const type_unsigned_8 registers[] = { 0u, 2u, 3u };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(registers) / sizeof(registers[0]); ++index) {
        type_unsigned_8 read[] = { 0x0fu, 0x20u,
            (type_unsigned_8)(0xc0u | registers[index] << 3u) };
        type_unsigned_8 write[] = { 0x0fu, 0x22u,
            (type_unsigned_8)(0xc0u | registers[index] << 3u) };
        type_unsigned_64 write_ticks = registers[index] == 0u ? 11u :
            (registers[index] == 2u ? 4u : 5u);

        if (timing_80386_manifest_run_s7_protected_recipe(read_keys[index], read,
                sizeof(read), STD_NULL, 0u, 6u) ||
            timing_80386_manifest_run_s7_protected_recipe(write_keys[index], write,
                sizeof(write), STD_NULL, 0u, write_ticks)) return 1;
    }
    return 0;
}

static C_INT timing_80386_manifest_run_s7_debug_test_register_recipes(C_VOID)
{
    static const C_CHAR *const keys[] = {
        "I386-MOV-R32-DR0-3", "I386-MOV-R32-DR6-7",
        "I386-MOV-DR0-3-R32", "I386-MOV-DR6-7-R32",
        "I386-MOV-R32-TR6-7", "I386-MOV-TR6-7-R32"
    };
    static const type_unsigned_8 secondary[] = {
        0x21u, 0x21u, 0x23u, 0x23u, 0x24u, 0x26u
    };
    static const type_unsigned_8 registers[] = { 0u, 6u, 0u, 6u, 6u, 6u };
    static const type_unsigned_64 ticks[] = { 22u, 14u, 22u, 16u, 12u, 12u };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(keys) / sizeof(keys[0]); ++index) {
        const type_unsigned_8 program[] = { 0x0fu, secondary[index],
            (type_unsigned_8)(0xc0u | registers[index] << 3u) };

        if (timing_80386_manifest_run_s7_protected_recipe(keys[index], program,
                sizeof(program), STD_NULL, 0u, ticks[index])) return 1;
    }
    return 0;
}

static C_INT timing_80386_manifest_run_s7_special_register_size_contexts(C_VOID)
{
    static const C_CHAR *const bases[] = {
        "I386-MOV-R32-CR0", "I386-MOV-R32-CR2", "I386-MOV-R32-CR3",
        "I386-MOV-CR0-R32", "I386-MOV-CR2-R32", "I386-MOV-CR3-R32",
        "I386-MOV-R32-DR0-3", "I386-MOV-R32-DR6-7",
        "I386-MOV-DR0-3-R32", "I386-MOV-DR6-7-R32",
        "I386-MOV-R32-TR6-7", "I386-MOV-TR6-7-R32"
    };
    static const type_unsigned_8 secondary[] = {
        0x20u,0x20u,0x20u,0x22u,0x22u,0x22u,
        0x21u,0x21u,0x23u,0x23u,0x24u,0x26u
    };
    static const type_unsigned_8 registers[] = { 0u,2u,3u,0u,2u,3u,0u,6u,0u,6u,6u,6u };
    static const type_unsigned_64 ticks[] = { 6u,6u,6u,11u,4u,5u,22u,14u,22u,16u,12u,12u };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(bases) / sizeof(bases[0]); ++index) {
        C_CHAR key_id[64];
        type_unsigned_8 program[] = { 0x0fu, secondary[index],
            (type_unsigned_8)(0xc0u | registers[index] << 3u) };
        type_bool size32;

        for (size32 = TYPE_FALSE; size32 <= TYPE_TRUE; ++size32) {
            if (STD_SNPRINTF(key_id, sizeof(key_id), "%s-SIZE%s", bases[index],
                    size32 ? "32" : "16") < 0) return 1;
            if (size32) {
                const type_unsigned_8 prefixed[] = { 0x66u, program[0], program[1], program[2] };
                if (timing_80386_manifest_run_s7_protected_recipe(key_id, prefixed,
                        sizeof(prefixed), STD_NULL, 0u, ticks[index])) return 1;
            } else if (timing_80386_manifest_run_s7_protected_recipe(key_id, program,
                    sizeof(program), STD_NULL, 0u, ticks[index])) return 1;
        }
    }
    return 0;
}

static C_INT timing_80386_manifest_run_s7_lmsw_sldt_recipes(C_VOID)
{
    static const type_unsigned_8 lmsw_r[] = { 0x0fu, 0x01u, 0xf0u };
    static const type_unsigned_8 lmsw_m[] = { 0x0fu, 0x01u, 0x36u, 0u, 0x10u };
    static const type_unsigned_8 lmsw_m_segment[] = {
        0x26u, 0x0fu, 0x01u, 0x36u, 0u, 0x10u
    };
    static const type_unsigned_8 sldt_r[] = { 0x0fu, 0x00u, 0xc0u };
    static const type_unsigned_8 sldt_m[] = { 0x0fu, 0x00u, 0x06u, 0u, 0x10u };
    static const type_unsigned_8 sldt_m_segment[] = {
        0x26u, 0x0fu, 0x00u, 0x06u, 0u, 0x10u
    };
    static const type_unsigned_16 msw = 1u;

    return timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LMSW-R",
            lmsw_r, sizeof(lmsw_r), STD_NULL, 0u, 11u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LMSW-M",
            lmsw_m, sizeof(lmsw_m), &msw, sizeof(msw), 14u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LMSW-M-SEGMENT",
            lmsw_m_segment, sizeof(lmsw_m_segment), &msw, sizeof(msw), 14u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-SLDT-R",
            sldt_r, sizeof(sldt_r), STD_NULL, 0u, 2u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-SLDT-M",
            sldt_m, sizeof(sldt_m), STD_NULL, 0u, 2u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-SLDT-M-SEGMENT",
            sldt_m_segment, sizeof(sldt_m_segment), STD_NULL, 0u, 2u);
}

static C_INT timing_80386_manifest_run_s7_pointer_size_recipes(C_VOID)
{
    static const C_CHAR *const keys[] = {
        "I386-MOV-LDS", "I386-MOV-LES", "I386-MOV-LFS", "I386-MOV-LGS", "I386-MOV-LSS"
    };
    static const type_unsigned_8 primary[] = { 0xc5u, 0xc4u, 0u, 0u, 0u };
    static const type_unsigned_8 secondary[] = { 0u, 0u, 0xb4u, 0xb5u, 0xb2u };
    static const type_unsigned_8 pointer16[] = { 0u, 0u, 0x10u, 0u };
    static const type_unsigned_8 pointer32[] = { 0u, 0u, 0u, 0u, 0x10u, 0u };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(keys) / sizeof(keys[0]); ++index) {
        C_CHAR key_id[64];
        type_unsigned_8 program16[5];
        type_unsigned_8 program32[6];
        STD_SIZE_T bytes16 = 0u;
        STD_SIZE_T bytes32 = 0u;
        type_unsigned_64 ticks16 = index < 2u || index == 4u ? 26u : 29u;
        type_unsigned_64 ticks32 = index < 2u || index == 4u ? 28u : 31u;

        if (secondary[index] == 0u) {
            program16[bytes16++] = primary[index];
            program32[bytes32++] = 0x66u;
            program32[bytes32++] = primary[index];
        } else {
            program16[bytes16++] = 0x0fu;
            program16[bytes16++] = secondary[index];
            program32[bytes32++] = 0x66u;
            program32[bytes32++] = 0x0fu;
            program32[bytes32++] = secondary[index];
        }
        program16[bytes16++] = 0x06u;
        program16[bytes16++] = 0u;
        program16[bytes16++] = 0x10u;
        program32[bytes32++] = 0x06u;
        program32[bytes32++] = 0u;
        program32[bytes32++] = 0x10u;
        if (STD_SNPRINTF(key_id, sizeof(key_id), "%s-SIZE16", keys[index]) < 0 ||
            timing_80386_manifest_run_s7_protected_recipe(key_id, program16,
                bytes16, pointer16, sizeof(pointer16), ticks16)) return 1;
        if (STD_SNPRINTF(key_id, sizeof(key_id), "%s-SIZE32", keys[index]) < 0 ||
            timing_80386_manifest_run_s7_protected_recipe(key_id, program32,
                bytes32, pointer32, sizeof(pointer32), ticks32)) return 1;
    }
    return 0;
}

static C_INT timing_80386_manifest_run_s7_lldt_recipes(C_VOID)
{
    static const type_unsigned_8 lldt_r[] = { 0x0fu, 0x00u, 0xd1u };
    static const type_unsigned_8 lldt_m[] = { 0x0fu, 0x00u, 0x16u, 0u, 0x10u };
    static const type_unsigned_8 lldt_m_segment[] = {
        0x26u, 0x0fu, 0x00u, 0x16u, 0u, 0x10u
    };
    static const type_unsigned_16 selector = 0x0018u;

    return timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LLDT-R",
            lldt_r, sizeof(lldt_r), STD_NULL, 0u, 20u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LLDT-M",
            lldt_m, sizeof(lldt_m), &selector, sizeof(selector), 24u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LLDT-M-SEGMENT",
            lldt_m_segment, sizeof(lldt_m_segment), &selector, sizeof(selector), 24u);
}

static C_INT timing_80386_manifest_run_s7_ltr_recipes(C_VOID)
{
    static const type_unsigned_8 ltr_r[] = { 0x0fu, 0x00u, 0xd9u };
    static const type_unsigned_8 ltr_m[] = { 0x0fu, 0x00u, 0x1eu, 0u, 0x10u };
    static const type_unsigned_8 ltr_m_segment[] = {
        0x26u, 0x0fu, 0x00u, 0x1eu, 0u, 0x10u
    };
    static const type_unsigned_16 selector = 0x0020u;

    return timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LTR-R",
            ltr_r, sizeof(ltr_r), STD_NULL, 0u, 23u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LTR-M",
            ltr_m, sizeof(ltr_m), &selector, sizeof(selector), 27u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-LTR-M-SEGMENT",
            ltr_m_segment, sizeof(ltr_m_segment), &selector, sizeof(selector), 27u);
}

static C_INT timing_80386_manifest_run_s7_str_recipes(C_VOID)
{
    static const type_unsigned_8 str_r[] = { 0x0fu, 0x00u, 0xc9u };
    static const type_unsigned_8 str_m[] = { 0x0fu, 0x00u, 0x0eu, 0u, 0x10u };
    static const type_unsigned_8 str_m_segment[] = {
        0x26u, 0x0fu, 0x00u, 0x0eu, 0u, 0x10u
    };

    return timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-STR-R",
            str_r, sizeof(str_r), STD_NULL, 0u, 23u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-STR-M",
            str_m, sizeof(str_m), STD_NULL, 0u, 27u) ||
        timing_80386_manifest_run_s7_protected_recipe("I386-SYSTEM-STR-M-SEGMENT",
            str_m_segment, sizeof(str_m_segment), STD_NULL, 0u, 27u);
}

static C_INT timing_80386_manifest_run_s4_base_recipes(C_VOID)
{
    static const C_CHAR *const io_names[] = { "IN", "OUT" };
    static const C_CHAR *const ports[] = { "IMM", "DX" };
    static const C_CHAR *const widths[] = { "B", "W", "D" };
    static const C_CHAR *const string_names[] = {
        "MOVS", "CMPS", "STOS", "LODS", "SCAS", "INS", "OUTS"
    };
    static const C_CHAR *const rep_names[] = {
        "MOVS", "CMPS-REPE", "CMPS-REPNE", "STOS", "LODS",
        "SCAS-REPE", "SCAS-REPNE", "INS", "OUTS"
    };
    STD_SIZE_T io; STD_SIZE_T port; STD_SIZE_T width; STD_SIZE_T string;

    for (io = 0u; io < 2u; ++io) for (port = 0u; port < 2u; ++port)
    for (width = 0u; width < 3u; ++width) {
        C_CHAR key_id[48];
        type_unsigned_8 program[3];
        STD_SIZE_T bytes = 0u;
        type_unsigned_8 opcode = (type_unsigned_8)((io == 0u ?
            (port == 0u ? 0xe4u : 0xecu) : (port == 0u ? 0xe6u : 0xeeu)) +
            (width == 0u ? 0u : 1u));

        if (width == 2u) program[bytes++] = 0x66u;
        program[bytes++] = opcode;
        if (port == 0u) program[bytes++] = 0xe0u;
        if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-%s-%s-%s", io_names[io],
                ports[port], widths[width]) < 0 ||
            timing_80386_manifest_run_recipe(key_id, program, bytes)) return 1;
    }
    for (string = 0u; string < 7u; ++string) for (width = 0u; width < 3u; ++width) {
        C_CHAR key_id[48];
        type_unsigned_8 program[2];
        STD_SIZE_T bytes = 0u;
        type_unsigned_8 opcode = (type_unsigned_8)((string < 5u ? 0xa4u +
            string * 2u : 0x6cu + (string - 5u) * 2u) + (width == 0u ? 0u : 1u));

        if (width == 2u) program[bytes++] = 0x66u;
        program[bytes++] = opcode;
        if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-STRING-%s-%s",
                string_names[string], widths[width]) < 0 ||
            timing_80386_manifest_run_recipe(key_id, program, bytes)) return 1;
    }
    for (string = 0u; string < 9u; ++string) for (width = 0u; width < 3u; ++width) {
        C_CHAR key_id[56];
        type_unsigned_8 program[3];
        STD_SIZE_T bytes = 0u;
        type_unsigned_8 base = string == 0u ? 0xa4u : string < 3u ? 0xa6u :
            string == 3u ? 0xaau : string == 4u ? 0xacu : string < 7u ? 0xaeu :
            string == 7u ? 0x6cu : 0x6eu;

        program[bytes++] = (string == 2u || string == 6u) ? 0xf2u : 0xf3u;
        if (width == 2u) program[bytes++] = 0x66u;
        program[bytes++] = (type_unsigned_8)(base + (width == 0u ? 0u : 1u));
        if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-REP-%s-%s",
                rep_names[string], widths[width]) < 0 ||
            timing_80386_manifest_run_recipe(key_id, program, bytes)) return 1;
    }
    return 0;
}

static C_INT timing_80386_manifest_run_s4_repeat_continuation(
    const C_CHAR *key_id, const type_unsigned_8 *program, STD_SIZE_T program_bytes)
{
    const core_machine_config config = { .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386 };
    const core_machine_retirement_observation_provider provider = {
        timing_80386_manifest_capture_retirement, STD_NULL
    };
    core_machine_retirement_observation_provider active_provider = provider;
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result run = { 0 };
    timing_80386_manifest_capture capture = { { 0 }, 0u };
    timing_80386_manifest_port_state port_state = { 0u, 0u };
    const timing_80386_manifest_record *record;
    core_machine *machine = STD_NULL;
    type_status status;

    record = timing_80386_manifest_find(key_id);
    if (record == STD_NULL || !timing_80386_manifest_is_i386(record)) return 1;
    active_provider.context = &capture;
    status = core_machine_create(&config, &machine);
    if (status == TYPE_STATUS_OK) status = test_core_machine_fixture_register_reset_mapping(
        machine, TIMING_80386_MANIFEST_RESET_LINEAR,
        TIMING_80386_MANIFEST_RESET_PHYSICAL, TIMING_80386_MANIFEST_WINDOW_BYTES);
    if (status == TYPE_STATUS_OK) status = core_machine_bind_execution_provider(
        machine, &timing_80386_manifest_execution, STD_NULL);
    if (status == TYPE_STATUS_OK) status = core_machine_install_port_provider(machine,
        0x00e0u, 0x00e0u, &timing_80386_manifest_ports, &port_state);
    if (status == TYPE_STATUS_OK) status = core_machine_freeze_execution_providers(machine);
    if (status == TYPE_STATUS_OK) status = core_machine_reset(machine);
    if (status == TYPE_STATUS_OK) status = timing_80386_manifest_prepare_recipe_machine(
        machine, key_id, program, program_bytes);
    if (status == TYPE_STATUS_OK && timing_80386_manifest_key_is_s5_real_control(key_id)) {
        const type_unsigned_16 return_frame[] = { 0xfff5u, 0xf000u, 0x0002u };
        const type_unsigned_16 handler[] = { 0xfff5u, 0xf000u };

        machine->executor_cpu.data.esp = 0x00001000u;
        if (timing_80386_manifest_key_has_prefix(key_id, "I386-RET-") ||
            STD_STRCMP(key_id, "I386-IRET-REAL") == 0) {
            status = core_machine_memory_write(machine, 0x1000u, return_frame,
                sizeof(return_frame));
        }
        if (status == TYPE_STATUS_OK && (timing_80386_manifest_key_has_prefix(key_id,
                "I386-INT") || STD_STRCMP(key_id, "I386-INTO-REAL") == 0)) {
            status = core_machine_memory_write(machine, 3u * 4u, handler,
                sizeof(handler));
            if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
                0x60u * 4u, handler, sizeof(handler));
        }
        if (status == TYPE_STATUS_OK && (timing_80386_manifest_key_has_suffix(key_id,
                "-NEAR-M") || timing_80386_manifest_key_has_prefix(key_id,
                "I386-CALL-FAR-REAL-M") || timing_80386_manifest_key_has_prefix(key_id,
                "I386-JMP-FAR-REAL-M"))) {
            const type_unsigned_16 indirect_target[] = { 0xfff5u, 0xf000u };

            status = core_machine_memory_write(machine, 0x1000u, indirect_target,
                sizeof(indirect_target));
        }
    }
    if (status == TYPE_STATUS_OK) status = core_machine_set_retirement_observation_provider(
        machine, &active_provider);

    /* The first iteration establishes the CPU-owned repeat identity.  It is
     * deliberately unrecorded: the continuation key must capture iteration two. */
    timing_80386_manifest_current_index = -1;
    if (status == TYPE_STATUS_OK) status = core_machine_run(machine, budget, &run);
    if (status != TYPE_STATUS_OK || run.reason != CORE_MACHINE_STOP_BUDGET ||
        run.executed != 1u || capture.count != 1u ||
        capture.observation.repeat_phase != CORE_MACHINE_RETIREMENT_REPEAT_FIRST) {
        STD_PRINTF("M5:T437:S4:I386-REPEAT-FIRST-DETAIL:%s:status=%d:executed=%llu:phase=%d\n",
            key_id, status, run.executed, capture.observation.repeat_phase);
        core_machine_destroy(machine);
        return 1;
    }
    capture.count = 0u;
    timing_80386_manifest_find(key_id);
    status = core_machine_run(machine, budget, &run);
    if (status != TYPE_STATUS_OK || run.reason != CORE_MACHINE_STOP_BUDGET ||
        run.executed != 1u || capture.count != 1u ||
        capture.observation.repeat_phase !=
            CORE_MACHINE_RETIREMENT_REPEAT_CONTINUATION ||
        capture.observation.timing_disposition ==
            CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED ||
        capture.observation.source_timing_form_id ==
            CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED) {
        STD_PRINTF("M5:T437:S4:I386-REPEAT-CONTINUE-DETAIL:%s:status=%d:executed=%llu:phase=%d\n",
            key_id, status, run.executed, capture.observation.repeat_phase);
        core_machine_destroy(machine);
        return 1;
    }
    core_machine_destroy(machine);
    return 0;
}

static C_INT timing_80386_manifest_run_s4_context_recipes(C_VOID)
{
    static const C_CHAR *const widths[] = { "B", "W", "D" };
    static const C_CHAR *const primitive_names[] = { "MOVS", "CMPS", "LODS" };
    static const C_CHAR *const rep_names[] = { "MOVS", "CMPS-REPE", "CMPS-REPNE", "LODS" };
    static const C_CHAR *const phase_names[] = { "FIRST", "CONTINUE", "ZERO" };
    static const C_CHAR *const all_rep_names[] = { "MOVS", "CMPS-REPE", "CMPS-REPNE",
        "STOS", "LODS", "SCAS-REPE", "SCAS-REPNE", "INS", "OUTS" };
    STD_SIZE_T width; STD_SIZE_T name; STD_SIZE_T phase;

    for (name = 0u; name < 3u; ++name) for (width = 0u; width < 3u; ++width) {
        C_CHAR key_id[64];
        type_unsigned_8 program[3] = { 0x26u, 0u, 0u };
        STD_SIZE_T bytes = 1u;
        type_unsigned_8 base = name == 0u ? 0xa4u : name == 1u ? 0xa6u : 0xacu;
        type_unsigned_8 opcode = (type_unsigned_8)(base + (width == 0u ? 0u : 1u));

        if (width == 2u) program[bytes++] = 0x66u;
        program[bytes++] = opcode;
        if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-STRING-%s-%s-SEGMENT",
                primitive_names[name], widths[width]) < 0 ||
            timing_80386_manifest_run_recipe(key_id, program, bytes)) return 1;
    }
    for (name = 0u; name < 4u; ++name) for (width = 0u; width < 3u; ++width) {
        C_CHAR key_id[72];
        type_unsigned_8 program[4] = { 0x26u, 0xf3u, 0u, 0u };
        STD_SIZE_T bytes = 2u;
        type_unsigned_8 base = name == 0u ? 0xa4u : name < 3u ? 0xa6u : 0xacu;

        if (name == 2u) program[1] = 0xf2u;
        if (width == 2u) program[bytes++] = 0x66u;
        program[bytes++] = (type_unsigned_8)(base + (width == 0u ? 0u : 1u));
        if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-REP-%s-%s-SEGMENT",
                rep_names[name], widths[width]) < 0 ||
            timing_80386_manifest_run_recipe(key_id, program, bytes)) return 1;
    }
    for (name = 0u; name < 9u; ++name) for (width = 0u; width < 3u; ++width)
    for (phase = 0u; phase < 3u; ++phase) {
        C_CHAR key_id[88];
        type_unsigned_8 program[3];
        STD_SIZE_T bytes = 0u;
        type_unsigned_8 base = name == 0u ? 0xa4u : name < 3u ? 0xa6u :
            name == 3u ? 0xaau : name == 4u ? 0xacu : name < 7u ? 0xaeu :
            name == 7u ? 0x6cu : 0x6eu;

        program[bytes++] = (name == 2u || name == 6u) ? 0xf2u : 0xf3u;
        if (width == 2u) program[bytes++] = 0x66u;
        program[bytes++] = (type_unsigned_8)(base + (width == 0u ? 0u : 1u));
        if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-REP-%s-%s-REP-PHASE-%s",
                all_rep_names[name], widths[width], phase_names[phase]) < 0 ||
            (phase == 1u ? timing_80386_manifest_run_s4_repeat_continuation(
                key_id, program, bytes) : timing_80386_manifest_run_recipe(
                key_id, program, bytes))) return 1;
    }
    return 0;
}

static C_INT timing_80386_manifest_run_s5_real_base_recipes(C_VOID)
{
    static const struct {
        const C_CHAR *key_id;
        type_unsigned_8 bytes[6];
        STD_SIZE_T count;
    } recipes[] = {
        { "I386-STACK-PUSH-R", { 0x50u }, 1u },
        { "I386-STACK-PUSH-M", { 0xffu, 0x36u, 0u, 0x10u }, 4u },
        { "I386-STACK-PUSH-IMM", { 0x68u, 1u, 0u }, 3u },
        { "I386-STACK-PUSH-SREG", { 0x06u }, 1u },
        { "I386-STACK-POP-R", { 0x58u }, 1u },
        { "I386-STACK-POP-M", { 0x8fu, 0x06u, 0u, 0x10u }, 4u },
        { "I386-STACK-POP-SREG", { 0x07u }, 1u },
        { "I386-STACK-PUSHA", { 0x60u }, 1u },
        { "I386-STACK-PUSHAD", { 0x66u, 0x60u }, 2u },
        { "I386-STACK-POPA", { 0x61u }, 1u },
        { "I386-STACK-POPAD", { 0x66u, 0x61u }, 2u },
        { "I386-STACK-PUSHF", { 0x9cu }, 1u },
        { "I386-STACK-PUSHFD", { 0x66u, 0x9cu }, 2u },
        { "I386-STACK-POPF", { 0x9du }, 1u },
        { "I386-STACK-POPFD", { 0x66u, 0x9du }, 2u },
        { "I386-STACK-ENTER-L0", { 0xc8u, 0u, 0u, 0u }, 4u },
        { "I386-STACK-ENTER-L1", { 0xc8u, 0u, 0u, 1u }, 4u },
        { "I386-STACK-ENTER-LN", { 0xc8u, 0u, 0u, 2u }, 4u },
        { "I386-STACK-LEAVE", { 0xc9u }, 1u },
        { "I386-CALL-NEAR-REL", { 0xe8u, 0u, 0u }, 3u },
        { "I386-CALL-NEAR-R", { 0xffu, 0xd0u }, 2u },
        { "I386-CALL-NEAR-M", { 0xffu, 0x16u, 0u, 0x10u }, 4u },
        { "I386-JMP-NEAR-REL", { 0xe9u, 0u, 0u }, 3u },
        { "I386-JMP-NEAR-R", { 0xffu, 0xe0u }, 2u },
        { "I386-JMP-NEAR-M", { 0xffu, 0x26u, 0u, 0x10u }, 4u },
        { "I386-CALL-FAR-REAL-DIRECT", { 0x9au, 0u, 0u, 0u, 0xf0u }, 5u },
        { "I386-CALL-FAR-REAL-M", { 0xffu, 0x1eu, 0u, 0x10u }, 4u },
        { "I386-JMP-FAR-REAL-DIRECT", { 0xeau, 0u, 0u, 0u, 0xf0u }, 5u },
        { "I386-JMP-FAR-REAL-M", { 0xffu, 0x2eu, 0u, 0x10u }, 4u },
        { "I386-RET-NEAR", { 0xc3u }, 1u },
        { "I386-RET-NEAR-IMM", { 0xc2u, 0u, 0u }, 3u },
        { "I386-RET-FAR-REAL", { 0xcbu }, 1u },
        { "I386-RET-FAR-REAL-IMM", { 0xcau, 0u, 0u }, 3u },
        { "I386-IRET-REAL", { 0xcfu }, 1u },
        { "I386-INT3-REAL", { 0xccu }, 1u },
        { "I386-INT-IMM-REAL", { 0xcdu, 0x60u }, 2u },
        { "I386-INTO-REAL", { 0xceu }, 1u },
        { "I386-INTO-NOT", { 0xceu }, 1u }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_80386_manifest_run_recipe(recipes[index].key_id,
                recipes[index].bytes, recipes[index].count) ||
            timing_80386_manifest_run_size_contexts(recipes[index].key_id,
                recipes[index].bytes, recipes[index].count)) return 1;
    }
    return 0;
}

static C_INT timing_80386_manifest_run_s5_branch_recipes(C_VOID)
{
    static const C_CHAR *const names[] = { "JO", "JNO", "JB", "JAE", "JE", "JNE",
        "JBE", "JA", "JS", "JNS", "JP", "JNP", "JL", "JGE", "JLE", "JG" };
    static const C_CHAR *const outcomes[] = { "TAKEN", "NOT" };
    STD_SIZE_T name; STD_SIZE_T outcome;

    for (name = 0u; name < 16u; ++name) for (outcome = 0u; outcome < 2u; ++outcome) {
        C_CHAR key_id[48];
        const type_unsigned_8 program[] = { (type_unsigned_8)(0x70u + name), 0u };

        if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-JCC-%s-%s", names[name],
                outcomes[outcome]) < 0 || timing_80386_manifest_run_recipe(key_id,
                program, sizeof(program)) || timing_80386_manifest_run_size_contexts(
                key_id, program, sizeof(program))) return 1;
    }
    for (name = 0u; name < 2u; ++name) {
        C_CHAR key_id[48];
        const type_unsigned_8 jcxz[] = { 0xe3u, 0u };
        const type_unsigned_8 jecxz[] = { 0x67u, 0xe3u, 0u };
        const type_unsigned_8 loop[] = { 0xe2u, 0u };
        const type_unsigned_8 loope[] = { 0xe1u, 0u };
        const type_unsigned_8 loopne[] = { 0xe0u, 0u };
        const C_CHAR *outcome_name = outcomes[name];

        if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-JCC-JCXZ-%s", outcome_name) < 0 ||
            timing_80386_manifest_run_recipe(key_id, jcxz, sizeof(jcxz)) ||
            timing_80386_manifest_run_size_contexts(key_id, jcxz, sizeof(jcxz)) ||
            STD_SNPRINTF(key_id, sizeof(key_id), "I386-JCC-JECXZ-%s", outcome_name) < 0 ||
            timing_80386_manifest_run_recipe(key_id, jecxz, sizeof(jecxz)) ||
            timing_80386_manifest_run_size_contexts(key_id, jecxz, sizeof(jecxz)) ||
            STD_SNPRINTF(key_id, sizeof(key_id), "I386-JCC-LOOP-%s", outcome_name) < 0 ||
            timing_80386_manifest_run_recipe(key_id, loop, sizeof(loop)) ||
            timing_80386_manifest_run_size_contexts(key_id, loop, sizeof(loop)) ||
            STD_SNPRINTF(key_id, sizeof(key_id), "I386-JCC-LOOPE-%s", outcome_name) < 0 ||
            timing_80386_manifest_run_recipe(key_id, loope, sizeof(loope)) ||
            timing_80386_manifest_run_size_contexts(key_id, loope, sizeof(loope)) ||
            STD_SNPRINTF(key_id, sizeof(key_id), "I386-JCC-LOOPNE-%s", outcome_name) < 0 ||
            timing_80386_manifest_run_recipe(key_id, loopne, sizeof(loopne)) ||
            timing_80386_manifest_run_size_contexts(key_id, loopne, sizeof(loopne))) return 1;
    }
    return 0;
}

static C_INT timing_80386_manifest_run_s5_segment_recipes(C_VOID)
{
    static const type_unsigned_8 push_memory[] = { 0x26u, 0xffu, 0x36u, 0u, 0x10u };
    static const type_unsigned_8 pop_memory[] = { 0x26u, 0x8fu, 0x06u, 0u, 0x10u };

    return timing_80386_manifest_run_recipe("I386-STACK-PUSH-M-SEGMENT",
        push_memory, sizeof(push_memory)) ||
        timing_80386_manifest_run_recipe("I386-STACK-POP-M-SEGMENT",
            pop_memory, sizeof(pop_memory));
}

static C_INT timing_80386_manifest_run_s6_direct_recipe(const C_CHAR *key_id,
    const type_unsigned_8 *program, STD_SIZE_T program_bytes,
    type_unsigned_64 expected_ticks)
{
    static const type_unsigned_8 target[] = { 0x90u };
    const core_machine_run_budget budget = { 1u, 0u };
    timing_80386_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80386_manifest_capture_retirement, &capture
    };
    core_machine_run_result run = { 0 };
    type_status run_status = TYPE_STATUS_OK;
    s3_gate_machine state;
    type_unsigned_8 gate[8u] = { 0x10u,0u,0x08u,0u,0u,0u,0u,0u };
    C_INT call_gate = timing_80386_manifest_key_has_prefix(key_id,
        "I386-CALL-GATE-") || timing_80386_manifest_key_has_prefix(key_id,
            "I386-JMP-GATE-");
    C_INT gate32 = timing_80386_manifest_key_has_suffix(key_id, "-SIZE32");
    C_INT failed;

    failed = key_id == STD_NULL || program == STD_NULL || program_bytes == 0u ||
        timing_80386_manifest_find(key_id) == STD_NULL ||
        !s3_gate_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE);
    if (!failed) {
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
        state.machine->executor_cpu.data.ds = state.machine->executor_cpu.data.ss;
        state.machine->executor_cpu.data.ds.sregtype = SREG_DATA;
    }
    if (!failed && call_gate) {
        gate[5] = (type_unsigned_8)(0x80u | (gate32 ?
            VCPU_DESC_SYS_TYPE_CALLGATE_32 : VCPU_DESC_SYS_TYPE_CALLGATE_16));
        state.machine->executor_cpu.data.gdtr.limit = 55u;
    }
    if (!failed) failed = (call_gate && !s3_gate_write(&state, S3_GDT_BASE + 48u,
        gate, sizeof(gate))) || !s3_gate_write(&state, S3_CODE_BASE, program,
        program_bytes) || !s3_gate_write(&state, S3_CODE_BASE + 0x10u, target,
            sizeof(target)) || core_machine_set_retirement_observation_provider(
                state.machine, &provider) != TYPE_STATUS_OK;
    if (!failed) run_status = core_machine_run(state.machine, budget, &run);
    if (!failed) failed = run_status != TYPE_STATUS_OK ||
        run.reason != CORE_MACHINE_STOP_BUDGET ||
        run.executed != 1u || run.ticks != expected_ticks || capture.count != 1u ||
        capture.observation.source_ticks != expected_ticks ||
        capture.observation.timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        capture.observation.timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        state.machine->executor_cpu.data.cs.selector != 0x0008u ||
        state.machine->executor_cpu.data.eip != 0x0010u;
    if (failed) STD_PRINTF("M5:T437:S6:I386-PROTECTED-DIRECT-DETAIL:%s:run=%llu:source=%llu:count=%u:cs=%04x:eip=%08x\n",
        key_id, run.ticks, capture.observation.source_ticks, capture.count,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.cs.selector : 0u,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.eip : 0u);
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT timing_80386_manifest_run_s6_memory_recipe(const C_CHAR *key_id,
    const type_unsigned_8 *program, STD_SIZE_T program_bytes,
    const C_VOID *pointer, STD_SIZE_T pointer_bytes, type_unsigned_64 expected_ticks)
{
    static const type_unsigned_8 target[] = { 0x90u };
    const core_machine_run_budget budget = { 1u, 0u };
    timing_80386_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80386_manifest_capture_retirement, &capture
    };
    core_machine_run_result run = { 0 };
    type_status run_status = TYPE_STATUS_OK;
    s3_gate_machine state;
    type_unsigned_8 gate[8u] = { 0x10u,0u,0x08u,0u,0u,0u,0u,0u };
    C_INT call_gate = timing_80386_manifest_key_has_prefix(key_id,
        "I386-CALL-GATE-") || timing_80386_manifest_key_has_prefix(key_id,
            "I386-JMP-GATE-");
    C_INT gate32 = timing_80386_manifest_key_has_suffix(key_id, "-SIZE32");
    C_INT failed;

    failed = key_id == STD_NULL || program == STD_NULL || program_bytes == 0u ||
        pointer == STD_NULL || pointer_bytes == 0u ||
        timing_80386_manifest_find(key_id) == STD_NULL ||
        !s3_gate_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE);
    if (!failed) {
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
        state.machine->executor_cpu.data.ds = state.machine->executor_cpu.data.ss;
        state.machine->executor_cpu.data.ds.sregtype = SREG_DATA;
    }
    if (!failed && call_gate) {
        gate[5] = (type_unsigned_8)(0x80u | (gate32 ?
            VCPU_DESC_SYS_TYPE_CALLGATE_32 : VCPU_DESC_SYS_TYPE_CALLGATE_16));
        state.machine->executor_cpu.data.gdtr.limit = 55u;
    }
    if (!failed) failed = (call_gate && !s3_gate_write(&state, S3_GDT_BASE + 48u,
        gate, sizeof(gate))) || !s3_gate_write(&state, S3_CODE_BASE, program,
        program_bytes) || !s3_gate_write(&state, 0x4000u, pointer, pointer_bytes) ||
        !s3_gate_write(&state, S3_CODE_BASE + 0x10u, target, sizeof(target)) ||
        core_machine_set_retirement_observation_provider(state.machine,
            &provider) != TYPE_STATUS_OK;
    if (!failed) run_status = core_machine_run(state.machine, budget, &run);
    if (!failed) failed = run_status != TYPE_STATUS_OK ||
        run.reason != CORE_MACHINE_STOP_BUDGET ||
        run.executed != 1u || run.ticks != expected_ticks || capture.count != 1u ||
        capture.observation.source_ticks != expected_ticks ||
        capture.observation.timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        capture.observation.timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        state.machine->executor_cpu.data.cs.selector != 0x0008u ||
        state.machine->executor_cpu.data.eip != 0x0010u;
    if (failed) STD_PRINTF("M5:T437:S6:I386-PROTECTED-MEMORY-DETAIL:%s:run=%llu:source=%llu:count=%u:cs=%04x:eip=%08x\n",
        key_id, run.ticks, capture.observation.source_ticks, capture.count,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.cs.selector : 0u,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.eip : 0u);
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT timing_80386_manifest_run_s6_return_recipe(const C_CHAR *key_id,
    const type_unsigned_8 *program, STD_SIZE_T program_bytes, const C_VOID *frame,
    STD_SIZE_T frame_bytes, type_unsigned_64 expected_ticks)
{
    static const type_unsigned_8 target[] = { 0x90u };
    const core_machine_run_budget budget = { 1u, 0u };
    timing_80386_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80386_manifest_capture_retirement, &capture
    };
    core_machine_run_result run = { 0 };
    s3_gate_machine state;
    C_INT failed;

    failed = key_id == STD_NULL || program == STD_NULL || program_bytes == 0u ||
        frame == STD_NULL || frame_bytes == 0u ||
        timing_80386_manifest_find(key_id) == STD_NULL ||
        !s3_gate_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE);
    if (!failed) state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
    if (!failed) failed = !s3_gate_write(&state, S3_CODE_BASE, program,
        program_bytes) || !s3_gate_write(&state, S3_STACK_TOP, frame, frame_bytes) ||
        !s3_gate_write(&state, S3_CODE_BASE + 0x10u, target, sizeof(target)) ||
        core_machine_set_retirement_observation_provider(state.machine,
            &provider) != TYPE_STATUS_OK;
    if (!failed) failed = core_machine_run(state.machine, budget, &run) !=
        TYPE_STATUS_OK || run.reason != CORE_MACHINE_STOP_BUDGET ||
        run.executed != 1u || run.ticks != expected_ticks || capture.count != 1u ||
        capture.observation.source_ticks != expected_ticks ||
        capture.observation.timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        capture.observation.timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        state.machine->executor_cpu.data.cs.selector != 0x0008u ||
        state.machine->executor_cpu.data.eip != 0x0010u;
    if (failed) STD_PRINTF("M5:T437:S6:I386-PROTECTED-RETURN-DETAIL:%s:run=%llu:source=%llu:count=%u:cs=%04x:eip=%08x\n",
        key_id, run.ticks, capture.observation.source_ticks, capture.count,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.cs.selector : 0u,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.eip : 0u);
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT timing_80386_manifest_run_s6_interrupt_recipe(const C_CHAR *key_id,
    type_unsigned_8 vector, const type_unsigned_8 *program, STD_SIZE_T program_bytes)
{
    static const type_unsigned_8 target[] = { 0x90u };
    const core_machine_run_budget budget = { 1u, 0u };
    timing_80386_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80386_manifest_capture_retirement, &capture
    };
    core_machine_run_result run = { 0 };
    s3_gate_machine state;
    C_INT failed;

    failed = key_id == STD_NULL || program == STD_NULL || program_bytes == 0u ||
        timing_80386_manifest_find(key_id) == STD_NULL ||
        !s3_gate_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE);
    if (!failed) state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
        (vector == 4u ? VCPU_EFLAGS_OF : 0u);
    if (!failed && vector != S3_VECTOR) failed = !s3_gate_install(&state, vector,
        0x0008u, VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE);
    if (!failed) failed = !s3_gate_write(&state, S3_CODE_BASE, program,
        program_bytes) || !s3_gate_write(&state, S3_CODE_BASE + S3_HANDLER, target,
            sizeof(target)) || core_machine_set_retirement_observation_provider(
                state.machine, &provider) != TYPE_STATUS_OK;
    if (!failed) failed = core_machine_run(state.machine, budget, &run) !=
        TYPE_STATUS_OK || run.reason != CORE_MACHINE_STOP_BUDGET ||
        run.executed != 1u || run.ticks != 60u || capture.count != 1u ||
        capture.observation.source_ticks != 60u ||
        capture.observation.timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        capture.observation.timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        state.machine->executor_cpu.data.cs.selector != 0x0008u ||
        state.machine->executor_cpu.data.eip != S3_HANDLER;
    if (failed) STD_PRINTF("M5:T437:S6:I386-PROTECTED-INTERRUPT-DETAIL:%s:run=%llu:source=%llu:count=%u:cs=%04x:eip=%08x\n",
        key_id, run.ticks, capture.observation.source_ticks, capture.count,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.cs.selector : 0u,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.eip : 0u);
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT timing_80386_manifest_prepare_outer_call_gate(s3_gate_machine *state,
    type_unsigned_8 parameters, type_bool gate32)
{
    static const type_unsigned_8 user_data[] = {
        0xffu,0xffu,0,0,0,0xf2u,0,0
    };
    type_unsigned_8 tss[16u] = { 0u };
    type_unsigned_8 tss_descriptor[8u] = { 0u };
    type_unsigned_8 gate[8u] = { 0u };
    t_cpu_data_sreg *tr;

    if (!s3_gate_prepare(state, CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE) ||
        !s3_gate_write(state, S3_GDT_BASE + 32u, user_data,
            sizeof(user_data))) return 0;
    tss[4u] = 0u;
    tss[5u] = 0x70u;
    tss[8u] = 0x10u;
    tss_descriptor[0] = sizeof(tss) - 1u;
    tss_descriptor[2] = 0u;
    tss_descriptor[3] = 0x05u;
    tss_descriptor[5] = 0x80u | VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
    gate[0] = 0u;
    gate[1] = 0x01u;
    gate[2] = 0x08u;
    gate[4] = parameters;
    gate[5] = (type_unsigned_8)(0xe0u | (gate32 ?
        VCPU_DESC_SYS_TYPE_CALLGATE_32 : VCPU_DESC_SYS_TYPE_CALLGATE_16));
    state->machine->executor_cpu.data.gdtr.limit = 55u;
    if (!s3_gate_write(state, 0x0500u, tss, sizeof(tss)) ||
        !s3_gate_write(state, S3_GDT_BASE + 40u, tss_descriptor,
            sizeof(tss_descriptor)) || !s3_gate_write(state, S3_GDT_BASE + 48u,
                gate, sizeof(gate))) return 0;
    tr = &state->machine->executor_cpu.data.tr;
    STD_MEMSET(tr, 0, sizeof(*tr));
    tr->flagValid = TYPE_TRUE;
    tr->selector = 0x0028u;
    tr->sregtype = SREG_TR;
    tr->base = 0x0500u;
    tr->limit = sizeof(tss) - 1u;
    tr->sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
    state->machine->executor_cpu.data.ss.selector = 0x0023u;
    state->machine->executor_cpu.data.ss.dpl = 3u;
    state->machine->executor_cpu.data.ds = state->machine->executor_cpu.data.ss;
    state->machine->executor_cpu.data.ds.sregtype = SREG_DATA;
    return 1;
}

static C_INT timing_80386_manifest_run_s6_outer_call_gate_recipe(
    const C_CHAR *key_id, const type_unsigned_8 *program, STD_SIZE_T program_bytes,
    type_unsigned_8 parameters, type_unsigned_64 expected_ticks)
{
    static const type_unsigned_8 target[] = { 0x90u };
    static const type_unsigned_32 arguments[] = { 0x12345678u,0x9abcdef0u };
    const core_machine_run_budget budget = { 1u, 0u };
    timing_80386_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80386_manifest_capture_retirement, &capture
    };
    core_machine_run_result run = { 0 };
    type_status run_status = TYPE_STATUS_OK;
    s3_gate_machine state;
    C_INT failed;

    failed = key_id == STD_NULL || program == STD_NULL || program_bytes == 0u ||
        parameters > 2u || timing_80386_manifest_find(key_id) == STD_NULL ||
        !timing_80386_manifest_prepare_outer_call_gate(&state, parameters,
            timing_80386_manifest_key_has_suffix(key_id, "-SIZE32"));
    if (!failed) state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
    if (!failed) failed = !s3_gate_write(&state, S3_CODE_BASE, program,
        program_bytes) || (parameters != 0u && !s3_gate_write(&state,
            S3_STACK_TOP, arguments, parameters * sizeof(arguments[0]))) ||
        !s3_gate_write(&state,
                S3_CODE_BASE + 0x100u, target, sizeof(target)) ||
        core_machine_set_retirement_observation_provider(state.machine,
            &provider) != TYPE_STATUS_OK;
    if (!failed) run_status = core_machine_run(state.machine, budget, &run);
    if (!failed) failed = run_status != TYPE_STATUS_OK ||
        run.reason != CORE_MACHINE_STOP_BUDGET ||
        run.executed != 1u || run.ticks != expected_ticks || capture.count != 1u ||
        capture.observation.source_ticks != expected_ticks ||
        capture.observation.timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        capture.observation.timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        state.machine->executor_cpu.data.cs.selector != 0x0008u ||
        state.machine->executor_cpu.data.eip != 0x0100u ||
        state.machine->executor_cpu.data.ss.selector != 0x0010u;
    if (failed) STD_PRINTF("M5:T437:S6:I386-PROTECTED-OUTER-GATE-DETAIL:%s:status=%d:reason=%d:run=%llu:source=%llu:count=%u:cs=%04x:eip=%08x:ss=%04x\n",
        key_id, (C_INT)run_status, (C_INT)run.reason, run.ticks, capture.observation.source_ticks, capture.count,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.cs.selector : 0u,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.eip : 0u,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.ss.selector : 0u);
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT timing_80386_manifest_run_s6_outer_call_gate_memory_recipe(
    const C_CHAR *key_id, const type_unsigned_8 *program, STD_SIZE_T program_bytes,
    const C_VOID *pointer, STD_SIZE_T pointer_bytes, type_unsigned_8 parameters,
    type_unsigned_64 expected_ticks)
{
    static const type_unsigned_8 target[] = { 0x90u };
    static const type_unsigned_32 arguments[] = { 0x12345678u,0x9abcdef0u };
    const core_machine_run_budget budget = { 1u, 0u };
    timing_80386_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80386_manifest_capture_retirement, &capture
    };
    core_machine_run_result run = { 0 };
    s3_gate_machine state;
    C_INT failed;

    failed = key_id == STD_NULL || program == STD_NULL || program_bytes == 0u ||
        pointer == STD_NULL || pointer_bytes == 0u || parameters > 2u ||
        timing_80386_manifest_find(key_id) == STD_NULL ||
        !timing_80386_manifest_prepare_outer_call_gate(&state, parameters,
            timing_80386_manifest_key_has_suffix(key_id, "-SIZE32"));
    if (!failed) state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
    if (!failed) failed = !s3_gate_write(&state, S3_CODE_BASE, program,
        program_bytes) || !s3_gate_write(&state, 0x4000u, pointer, pointer_bytes) ||
        (parameters != 0u && !s3_gate_write(&state, S3_STACK_TOP, arguments,
            parameters * sizeof(arguments[0]))) || !s3_gate_write(&state,
                S3_CODE_BASE + 0x100u, target, sizeof(target)) ||
        core_machine_set_retirement_observation_provider(state.machine,
            &provider) != TYPE_STATUS_OK;
    if (!failed) failed = core_machine_run(state.machine, budget, &run) !=
        TYPE_STATUS_OK || run.reason != CORE_MACHINE_STOP_BUDGET ||
        run.executed != 1u || run.ticks != expected_ticks || capture.count != 1u ||
        capture.observation.source_ticks != expected_ticks ||
        capture.observation.timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        capture.observation.timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        state.machine->executor_cpu.data.cs.selector != 0x0008u ||
        state.machine->executor_cpu.data.eip != 0x0100u ||
        state.machine->executor_cpu.data.ss.selector != 0x0010u;
    if (failed) STD_PRINTF("M5:T437:S6:I386-PROTECTED-OUTER-GATE-MEMORY-DETAIL:%s:run=%llu:source=%llu:count=%u:cs=%04x:eip=%08x:ss=%04x\n",
        key_id, run.ticks, capture.observation.source_ticks, capture.count,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.cs.selector : 0u,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.eip : 0u,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.ss.selector : 0u);
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT timing_80386_manifest_prepare_inner_interrupt(s3_gate_machine *state,
    type_unsigned_8 vector)
{
    static const type_unsigned_8 user_data[] = { 0xffu,0xffu,0,0,0,0xf2u,0,0 };
    type_unsigned_8 tss[8u] = { 0u };
    type_unsigned_8 descriptor[8u] = { 7u,0u,0u,5u,0u,
        0x80u | VCPU_DESC_SYS_TYPE_TSS_16_BUSY,0u,0u };
    t_cpu_data_sreg *tr;

    if (!s3_gate_prepare(state, CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE) ||
        !s3_gate_write(state, S3_GDT_BASE + 32u, user_data, sizeof(user_data)) ||
        !s3_gate_install(state, vector, 0x0008u,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 3u, TYPE_TRUE)) return 0;
    tss[2u] = 0u;
    tss[3u] = 0x70u;
    tss[4u] = 0x10u;
    if (!s3_gate_write(state, 0x0500u, tss, sizeof(tss)) || !s3_gate_write(
            state, S3_GDT_BASE + 40u, descriptor, sizeof(descriptor))) return 0;
    state->machine->executor_cpu.data.gdtr.limit = 47u;
    tr = &state->machine->executor_cpu.data.tr;
    STD_MEMSET(tr, 0, sizeof(*tr));
    tr->flagValid = TYPE_TRUE;
    tr->selector = 0x0028u;
    tr->sregtype = SREG_TR;
    tr->base = 0x0500u;
    tr->limit = sizeof(tss) - 1u;
    tr->sys.type = VCPU_DESC_SYS_TYPE_TSS_16_BUSY;
    state->machine->executor_cpu.data.ss.selector = 0x0023u;
    state->machine->executor_cpu.data.ss.dpl = 3u;
    return 1;
}

static C_INT timing_80386_manifest_run_s6_inner_interrupt_recipe(
    const C_CHAR *key_id, type_unsigned_8 vector, const type_unsigned_8 *program,
    STD_SIZE_T program_bytes)
{
    static const type_unsigned_8 target[] = { 0x90u };
    const core_machine_run_budget budget = { 1u, 0u };
    timing_80386_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80386_manifest_capture_retirement, &capture
    };
    core_machine_run_result run = { 0 };
    s3_gate_machine state;
    C_INT failed = key_id == STD_NULL || program == STD_NULL || program_bytes == 0u ||
        timing_80386_manifest_find(key_id) == STD_NULL ||
        !timing_80386_manifest_prepare_inner_interrupt(&state, vector);

    if (!failed) state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
        (vector == 4u ? VCPU_EFLAGS_OF : 0u);
    if (!failed) failed = !s3_gate_write(&state, S3_CODE_BASE, program,
        program_bytes) || !s3_gate_write(&state, S3_CODE_BASE + S3_HANDLER,
            target, sizeof(target)) || core_machine_set_retirement_observation_provider(
                state.machine, &provider) != TYPE_STATUS_OK;
    if (!failed) failed = core_machine_run(state.machine, budget, &run) !=
        TYPE_STATUS_OK || run.reason != CORE_MACHINE_STOP_BUDGET ||
        run.executed != 1u || run.ticks != 100u || capture.count != 1u ||
        capture.observation.source_ticks != 100u ||
        capture.observation.timing_origin != CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        capture.observation.timing_disposition != CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        state.machine->executor_cpu.data.cs.selector != 0x0008u ||
        state.machine->executor_cpu.data.eip != S3_HANDLER;
    if (failed) STD_PRINTF("M5:T437:S6:I386-PROTECTED-INNER-INTERRUPT-DETAIL:%s:run=%llu:source=%llu:count=%u:cs=%04x:eip=%08x\n",
        key_id, run.ticks, capture.observation.source_ticks, capture.count,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.cs.selector : 0u,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.eip : 0u);
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT timing_80386_manifest_run_s6_task_recipe(const C_CHAR *key_id,
    task_switch_case test_case, type_unsigned_32 bootstrap_instructions,
    type_bool task_gate_jump, type_bool task_gate_memory,
    type_bool memory_call, type_unsigned_64 expected_ticks)
{
    const core_machine_run_budget bootstrap_budget = { bootstrap_instructions, 0u };
    const core_machine_run_budget task_budget = { 1u, 0u };
    timing_80386_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80386_manifest_capture_retirement, &capture
    };
    core_machine_run_result bootstrap = { 0 };
    core_machine_run_result run = { 0 };
    task_switch_fixture fixture;
    static const type_unsigned_8 jmp_far = 0xeau;
    static const type_unsigned_8 task_gate_selector[] = { 0x38u,0u };
    static const type_unsigned_8 call_far_memory = 0x1eu;
    static const type_unsigned_8 call_far32[] = {
        0x66u,0x9au,0u,0u,0u,0u,0x30u,0u
    };
    static const type_unsigned_8 jmp_task_gate32[] = {
        0x66u,0xeau,0u,0u,0u,0u,0x38u,0u
    };
    C_INT failed = key_id == STD_NULL || timing_80386_manifest_find(key_id) ==
        STD_NULL || !task_switch_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386) ||
        !task_switch_install(&fixture, test_case);

    if (!failed && task_gate_jump) failed = !write_bytes(fixture.machine,
        KERNEL_BASE + 3u, &jmp_far, sizeof(jmp_far));
    if (!failed && task_gate_memory) failed = !write_bytes(fixture.machine,
        test_case == TASK_SWITCH_CASE_INDIRECT_OPERAND32_SUCCESS ? 0x5204u :
        0x5202u, task_gate_selector, sizeof(task_gate_selector));
    if (!failed && memory_call) failed = !write_bytes(fixture.machine,
        KERNEL_BASE + (test_case == TASK_SWITCH_CASE_INDIRECT_OPERAND32_SUCCESS ?
            5u : 4u), &call_far_memory, sizeof(call_far_memory));
    if (!failed && timing_80386_manifest_key_has_prefix(key_id,
            "I386-CALL-TASK-DIRECT") && timing_80386_manifest_key_has_suffix(
                key_id, "-SIZE32")) failed = !write_bytes(fixture.machine,
            KERNEL_BASE + 3u, call_far32, sizeof(call_far32));
    if (!failed && timing_80386_manifest_key_has_prefix(key_id,
            "I386-JMP-TASK-GATE-DIRECT") && timing_80386_manifest_key_has_suffix(
                key_id, "-SIZE32")) failed = !write_bytes(fixture.machine,
            KERNEL_BASE + 3u, jmp_task_gate32, sizeof(jmp_task_gate32));
    if (!failed) failed = core_machine_run(fixture.machine, bootstrap_budget,
        &bootstrap) != TYPE_STATUS_OK || bootstrap.reason != CORE_MACHINE_STOP_BUDGET ||
        bootstrap.executed != bootstrap_budget.instructions ||
        core_machine_set_retirement_observation_provider(fixture.machine,
            &provider) != TYPE_STATUS_OK;
    if (!failed) failed = core_machine_run(fixture.machine, task_budget, &run) !=
        TYPE_STATUS_OK || run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
        run.ticks != expected_ticks || capture.count != 1u ||
        capture.observation.source_ticks != expected_ticks ||
        capture.observation.timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        capture.observation.timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        fixture.machine->executor_cpu.data.tr.selector != 0x0030u;
    if (failed) STD_PRINTF("M5:T437:S6:I386-TASK-DETAIL:%s:bootstrap=%llu/%u:run=%llu:source=%llu:count=%u:tr=%04x\n",
        key_id, bootstrap.ticks, bootstrap.executed, run.ticks,
        capture.observation.source_ticks, capture.count, fixture.machine != STD_NULL ?
        fixture.machine->executor_cpu.data.tr.selector : 0u);
    core_machine_destroy(fixture.machine);
    return failed;
}

static C_INT timing_80386_manifest_run_s6_iret_task_recipe(const C_CHAR *key_id)
{
    static const type_unsigned_8 iret16[] = { 0xcfu };
    static const type_unsigned_8 iret32[] = { 0x66u,0xcfu };
    const core_machine_run_budget bootstrap_budget = { 12u, 0u };
    const core_machine_run_budget one_instruction = { 1u, 0u };
    timing_80386_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80386_manifest_capture_retirement, &capture
    };
    core_machine_run_result bootstrap = { 0 };
    core_machine_run_result enter_task = { 0 };
    core_machine_run_result run = { 0 };
    task_switch_fixture fixture;
    const type_unsigned_8 *iret = key_id != STD_NULL &&
        timing_80386_manifest_key_has_suffix(key_id, "-SIZE32") ? iret32 : iret16;
    STD_SIZE_T iret_bytes = iret == iret32 ? sizeof(iret32) : sizeof(iret16);
    C_INT failed = key_id == STD_NULL || timing_80386_manifest_find(key_id) ==
        STD_NULL ||
        !task_switch_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386) ||
        !task_switch_install(&fixture, TASK_SWITCH_CASE_NESTED_RETURN);

    if (!failed) failed = core_machine_run(fixture.machine, bootstrap_budget,
        &bootstrap) != TYPE_STATUS_OK || bootstrap.reason != CORE_MACHINE_STOP_BUDGET ||
        bootstrap.executed != bootstrap_budget.instructions;
    if (!failed) failed = core_machine_run(fixture.machine, one_instruction,
        &enter_task) != TYPE_STATUS_OK || enter_task.reason != CORE_MACHINE_STOP_BUDGET ||
        enter_task.executed != 1u || fixture.machine->executor_cpu.data.tr.selector !=
            0x0030u || !TYPE_GET_BIT(fixture.machine->executor_cpu.data.eflags,
                VCPU_EFLAGS_NT);
    if (!failed) failed = !write_bytes(fixture.machine, KERNEL_BASE + 0x100u,
        iret, iret_bytes) || core_machine_set_retirement_observation_provider(
            fixture.machine, &provider) != TYPE_STATUS_OK;
    if (!failed) failed = core_machine_run(fixture.machine, one_instruction, &run) !=
        TYPE_STATUS_OK || run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
        run.ticks != 275u || capture.count != 1u ||
        capture.observation.source_ticks != 275u ||
        capture.observation.timing_origin != CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        capture.observation.timing_disposition != CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        fixture.machine->executor_cpu.data.tr.selector != 0x0028u;
    if (failed) STD_PRINTF("M5:T437:S6:I386-IRET-TASK-DETAIL:%s:bootstrap=%u:enter=%llu:run=%llu:source=%llu:count=%u:tr=%04x\n",
        key_id, bootstrap.executed, enter_task.ticks, run.ticks, capture.observation.source_ticks,
        capture.count, fixture.machine != STD_NULL ? fixture.machine->executor_cpu.data.tr.selector : 0u);
    core_machine_destroy(fixture.machine);
    return failed;
}

static C_INT timing_80386_manifest_run_s6_task_interrupt_recipe(
    const C_CHAR *key_id, type_unsigned_8 vector, const type_unsigned_8 *program,
    STD_SIZE_T program_bytes, type_bool overflow)
{
    static const type_unsigned_8 task_gate[] = {
        0u,0u,0x30u,0u,0u,0x85u,0u,0u
    };
    const core_machine_run_budget bootstrap_budget = { 10u, 0u };
    const core_machine_run_budget one_instruction = { 1u, 0u };
    timing_80386_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80386_manifest_capture_retirement, &capture
    };
    core_machine_run_result bootstrap = { 0 };
    core_machine_run_result run = { 0 };
    task_switch_fixture fixture;
    C_INT failed = key_id == STD_NULL || program == STD_NULL || program_bytes == 0u ||
        timing_80386_manifest_find(key_id) == STD_NULL ||
        !task_switch_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386) ||
        !task_switch_install(&fixture, TASK_SWITCH_CASE_IDT_TASK_GATE);

    if (!failed) failed = !write_bytes(fixture.machine, IDT_BASE +
        (type_unsigned_32)vector * 8u, task_gate, sizeof(task_gate)) ||
        !write_bytes(fixture.machine, KERNEL_BASE + 3u, program, program_bytes);
    if (!failed) {
        fixture.machine->executor_cpu.data.idtr.flagValid = TYPE_TRUE;
        fixture.machine->executor_cpu.data.idtr.sregtype = SREG_IDTR;
        fixture.machine->executor_cpu.data.idtr.base = IDT_BASE;
        fixture.machine->executor_cpu.data.idtr.limit =
            (type_unsigned_16)((type_unsigned_16)vector * 8u + 7u);
        failed = core_machine_run(fixture.machine, bootstrap_budget, &bootstrap) !=
            TYPE_STATUS_OK || bootstrap.reason != CORE_MACHINE_STOP_BUDGET ||
            bootstrap.executed != bootstrap_budget.instructions;
    }
    if (!failed && overflow) fixture.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_OF;
    if (!failed) failed = core_machine_set_retirement_observation_provider(
        fixture.machine, &provider) != TYPE_STATUS_OK || core_machine_run(
            fixture.machine, one_instruction, &run) != TYPE_STATUS_OK ||
        run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
        run.ticks != 309u || capture.count != 1u ||
        capture.observation.source_ticks != 309u ||
        capture.observation.timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        capture.observation.timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        fixture.machine->executor_cpu.data.tr.selector != 0x0030u;
    if (failed) STD_PRINTF("M5:T437:S6:I386-TASK-INTERRUPT-DETAIL:%s:bootstrap=%llu/%u:run=%llu:source=%llu:count=%u:tr=%04x\n",
        key_id, bootstrap.ticks, bootstrap.executed, run.ticks,
        capture.observation.source_ticks, capture.count, fixture.machine != STD_NULL ?
        fixture.machine->executor_cpu.data.tr.selector : 0u);
    core_machine_destroy(fixture.machine);
    return failed;
}

static C_INT timing_80386_manifest_run_s6_vm86_interrupt_recipe(const C_CHAR *key_id,
    type_unsigned_8 vector, const type_unsigned_8 *code, STD_SIZE_T code_bytes)
{
    static const type_unsigned_8 target[] = { 0x90u };
    type_unsigned_8 tss[16u] = { 0u };
    type_unsigned_8 tss_descriptor[8u] = { 15u,0u,0u,5u,0u,
        0x80u | VCPU_DESC_SYS_TYPE_TSS_32_BUSY,0u,0u };
    const core_machine_run_budget budget = { 1u, 0u };
    timing_80386_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80386_manifest_capture_retirement, &capture
    };
    core_machine_run_result run = { 0 };
    s3_gate_machine state;
    t_cpu_data_sreg *tr;
    C_INT failed = key_id == STD_NULL || code == STD_NULL || code_bytes == 0u ||
        timing_80386_manifest_find(key_id) == STD_NULL ||
        !s3_gate_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE,
            VCPU_DESC_SYS_TYPE_INTGATE_32, 3u, TYPE_TRUE) ||
        !s3_gate_install(&state, vector, 0x0008u, VCPU_DESC_SYS_TYPE_INTGATE_32,
            3u, TYPE_TRUE);

    tss[4u] = 0u;
    tss[5u] = 0x70u;
    tss[8u] = 0x10u;
    if (!failed) failed = !s3_gate_write(&state, 0x0500u, tss, sizeof(tss)) ||
        !s3_gate_write(&state, S3_GDT_BASE + 40u, tss_descriptor,
            sizeof(tss_descriptor));
    if (!failed) {
        state.machine->executor_cpu.data.gdtr.limit = 47u;
        tr = &state.machine->executor_cpu.data.tr;
        STD_MEMSET(tr, 0, sizeof(*tr));
        tr->flagValid = TYPE_TRUE;
        tr->selector = 0x0028u;
        tr->sregtype = SREG_TR;
        tr->base = 0x0500u;
        tr->limit = sizeof(tss) - 1u;
        tr->sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
        state.machine->executor_cpu.data.cs.selector = 0x0200u;
        state.machine->executor_cpu.data.cs.base = S3_CODE_BASE;
        state.machine->executor_cpu.data.cs.dpl = 3u;
        state.machine->executor_cpu.data.ss.selector = 0u;
        state.machine->executor_cpu.data.ss.base = 0u;
        state.machine->executor_cpu.data.ss.dpl = 3u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_VM |
            VCPU_EFLAGS_IOPL | (vector == 4u ? VCPU_EFLAGS_OF : 0u);
    }
    if (!failed) failed = !s3_gate_write(&state, S3_CODE_BASE, code, code_bytes) ||
        !s3_gate_write(&state, S3_CODE_BASE + S3_HANDLER, target, sizeof(target)) ||
        core_machine_set_retirement_observation_provider(state.machine,
            &provider) != TYPE_STATUS_OK;
    if (!failed) failed = core_machine_run(state.machine, budget, &run) !=
        TYPE_STATUS_OK || run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
        run.ticks != 120u || capture.count != 1u ||
        capture.observation.source_ticks != 120u ||
        capture.observation.timing_origin != CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        capture.observation.timing_disposition != CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        state.machine->executor_cpu.data.cs.selector != 0x0008u ||
        state.machine->executor_cpu.data.eip != S3_HANDLER;
    if (failed) STD_PRINTF("M5:T437:S6:I386-VM86-INTERRUPT-DETAIL:%s:run=%llu:source=%llu:count=%u:cs=%04x:eip=%08x\n",
        key_id, run.ticks, capture.observation.source_ticks, capture.count,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.cs.selector : 0u,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.eip : 0u);
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT timing_80386_manifest_run_s6_iret_vm86_recipe(const C_CHAR *key_id)
{
    static const type_unsigned_8 iret[] = { 0x66u,0xcfu };
    static const type_unsigned_8 target[] = { 0x90u };
    static const type_unsigned_32 frame[] = {
        0x10u,0x0200u,VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL,0x8000u,0u,0u,0u,0u,0u
    };
    const core_machine_run_budget budget = { 1u, 0u };
    timing_80386_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80386_manifest_capture_retirement, &capture
    };
    core_machine_run_result run = { 0 };
    s3_gate_machine state;
    C_INT failed = key_id == STD_NULL || timing_80386_manifest_find(key_id) ==
        STD_NULL ||
        !s3_gate_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE);

    if (!failed) state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
    if (!failed) failed = !s3_gate_write(&state, S3_CODE_BASE, iret, sizeof(iret)) ||
        !s3_gate_write(&state, S3_STACK_TOP, frame, sizeof(frame)) ||
        !s3_gate_write(&state, S3_CODE_BASE + 0x10u, target, sizeof(target)) ||
        core_machine_set_retirement_observation_provider(state.machine,
            &provider) != TYPE_STATUS_OK;
    if (!failed) failed = core_machine_run(state.machine, budget, &run) !=
        TYPE_STATUS_OK || run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
        run.ticks != 60u || capture.count != 1u ||
        capture.observation.source_ticks != 60u ||
        capture.observation.timing_origin != CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        capture.observation.timing_disposition != CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        (state.machine->executor_cpu.data.eflags & VCPU_EFLAGS_VM) == 0u ||
        state.machine->executor_cpu.data.cs.selector != 0x0200u ||
        state.machine->executor_cpu.data.cs.base != S3_CODE_BASE ||
        state.machine->executor_cpu.data.eip != 0x10u;
    if (failed) STD_PRINTF("M5:T437:S6:I386-IRET-VM86-DETAIL:%s:run=%llu:source=%llu:count=%u:cs=%04x:base=%08x:eip=%08x:eflags=%08x\n",
        key_id, run.ticks, capture.observation.source_ticks, capture.count,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.cs.selector : 0u,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.cs.base : 0u,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.eip : 0u,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.eflags : 0u);
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT timing_80386_manifest_run_s6_outer_return_recipe(const C_CHAR *key_id,
    const type_unsigned_8 *program, STD_SIZE_T program_bytes, const C_VOID *frame,
    STD_SIZE_T frame_bytes, type_unsigned_64 expected_ticks)
{
    static const type_unsigned_8 user_data[] = { 0xffu,0xffu,0,0,0,0xf2u,0,0 };
    static const type_unsigned_8 target[] = { 0x90u };
    const core_machine_run_budget budget = { 1u, 0u };
    timing_80386_manifest_capture capture = { { 0 }, 0u };
    const core_machine_retirement_observation_provider provider = {
        timing_80386_manifest_capture_retirement, &capture
    };
    core_machine_run_result run = { 0 };
    s3_gate_machine state;
    C_INT failed;

    failed = key_id == STD_NULL || program == STD_NULL || program_bytes == 0u ||
        frame == STD_NULL || frame_bytes == 0u ||
        timing_80386_manifest_find(key_id) == STD_NULL ||
        !s3_gate_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE);
    if (!failed) {
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
        state.machine->executor_cpu.data.gdtr.limit = 39u;
    }
    if (!failed) failed = !s3_gate_write(&state, S3_GDT_BASE + 32u, user_data,
        sizeof(user_data)) || !s3_gate_write(&state, S3_CODE_BASE, program,
            program_bytes) || !s3_gate_write(&state, S3_STACK_TOP, frame,
                frame_bytes) || !s3_gate_write(&state, S3_CODE_BASE + 0x10u,
                    target, sizeof(target)) ||
        core_machine_set_retirement_observation_provider(state.machine,
            &provider) != TYPE_STATUS_OK;
    if (!failed) failed = core_machine_run(state.machine, budget, &run) !=
        TYPE_STATUS_OK || run.reason != CORE_MACHINE_STOP_BUDGET ||
        run.executed != 1u || run.ticks != expected_ticks || capture.count != 1u ||
        capture.observation.source_ticks != expected_ticks ||
        capture.observation.timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
        capture.observation.timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        state.machine->executor_cpu.data.cs.selector != 0x001bu ||
        state.machine->executor_cpu.data.ss.selector != 0x0023u ||
        state.machine->executor_cpu.data.eip != 0x0010u;
    if (failed) STD_PRINTF("M5:T437:S6:I386-PROTECTED-OUTER-RETURN-DETAIL:%s:run=%llu:source=%llu:count=%u:cs=%04x:ss=%04x:eip=%08x\n",
        key_id, run.ticks, capture.observation.source_ticks, capture.count,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.cs.selector : 0u,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.ss.selector : 0u,
        state.machine != STD_NULL ? state.machine->executor_cpu.data.eip : 0u);
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT timing_80386_manifest_run_s6_direct_recipes(C_VOID)
{
    static const type_unsigned_8 call16[] = { 0x9au,0x10u,0u,0x08u,0u };
    static const type_unsigned_8 call32[] = {
        0x66u,0x9au,0x10u,0u,0u,0u,0x08u,0u
    };
    static const type_unsigned_8 jmp16[] = { 0xeau,0x10u,0u,0x08u,0u };
    static const type_unsigned_8 jmp32[] = {
        0x66u,0xeau,0x10u,0u,0u,0u,0x08u,0u
    };
    static const type_unsigned_8 pointer16[] = { 0x10u,0u,0x08u,0u };
    static const type_unsigned_8 pointer32[] = { 0x10u,0u,0u,0u,0x08u,0u };
    static const type_unsigned_8 call_memory16[] = { 0xffu,0x1eu,0u,0x40u };
    static const type_unsigned_8 call_memory32[] = { 0x66u,0xffu,0x1eu,0u,0x40u };
    static const type_unsigned_8 jmp_memory16[] = { 0xffu,0x2eu,0u,0x40u };
    static const type_unsigned_8 jmp_memory32[] = { 0x66u,0xffu,0x2eu,0u,0x40u };
    static const type_unsigned_16 ret_frame[] = { 0x0010u,0x0008u };
    static const type_unsigned_16 iret_frame[] = { 0x0010u,0x0008u,VCPU_EFLAGS_CF };
    static const type_unsigned_32 ret_frame32[] = { 0x00000010u,0x00000008u };
    static const type_unsigned_32 iret_frame32[] = {
        0x00000010u,0x00000008u,VCPU_EFLAGS_CF
    };
    static const type_unsigned_16 ret_outer_frame[] = { 0x0010u,0x001bu,0x4000u,0x0023u };
    static const type_unsigned_16 iret_outer_frame[] = {
        0x0010u,0x001bu,VCPU_EFLAGS_CF,0x4000u,0x0023u
    };
    static const type_unsigned_32 ret_outer_frame32[] = {
        0x00000010u,0x0000001bu,0x00004000u,0x00000023u
    };
    static const type_unsigned_32 iret_outer_frame32[] = {
        0x00000010u,0x0000001bu,VCPU_EFLAGS_CF,0x00004000u,0x00000023u
    };

    return timing_80386_manifest_run_s6_direct_recipe("I386-CALL-FAR-PM-DIRECT",
        call16, sizeof(call16), 35u) ||
        timing_80386_manifest_run_s6_direct_recipe(
            "I386-CALL-FAR-PM-DIRECT-SIZE16", call16, sizeof(call16), 35u) ||
        timing_80386_manifest_run_s6_direct_recipe(
            "I386-CALL-FAR-PM-DIRECT-SIZE32", call32, sizeof(call32), 35u) ||
        timing_80386_manifest_run_s6_direct_recipe("I386-JMP-FAR-PM-DIRECT",
            jmp16, sizeof(jmp16), 28u) ||
        timing_80386_manifest_run_s6_direct_recipe(
            "I386-JMP-FAR-PM-DIRECT-SIZE16", jmp16, sizeof(jmp16), 28u) ||
        timing_80386_manifest_run_s6_direct_recipe(
            "I386-JMP-FAR-PM-DIRECT-SIZE32", jmp32, sizeof(jmp32), 28u) ||
        timing_80386_manifest_run_s6_memory_recipe("I386-CALL-FAR-PM-M",
            call_memory16, sizeof(call_memory16), pointer16, sizeof(pointer16), 39u) ||
        timing_80386_manifest_run_s6_memory_recipe("I386-CALL-FAR-PM-M-SIZE16",
            call_memory16, sizeof(call_memory16), pointer16, sizeof(pointer16), 39u) ||
        timing_80386_manifest_run_s6_memory_recipe("I386-CALL-FAR-PM-M-SIZE32",
            call_memory32, sizeof(call_memory32), pointer32, sizeof(pointer32), 39u) ||
        timing_80386_manifest_run_s6_memory_recipe("I386-JMP-FAR-PM-M",
            jmp_memory16, sizeof(jmp_memory16), pointer16, sizeof(pointer16), 32u) ||
        timing_80386_manifest_run_s6_memory_recipe("I386-JMP-FAR-PM-M-SIZE16",
            jmp_memory16, sizeof(jmp_memory16), pointer16, sizeof(pointer16), 32u) ||
        timing_80386_manifest_run_s6_memory_recipe("I386-JMP-FAR-PM-M-SIZE32",
            jmp_memory32, sizeof(jmp_memory32), pointer32, sizeof(pointer32), 32u) ||
        timing_80386_manifest_run_s6_return_recipe("I386-RET-FAR-PM-SAME",
            (const type_unsigned_8[]){ 0xcbu }, 1u, ret_frame, sizeof(ret_frame), 33u) ||
        timing_80386_manifest_run_s6_return_recipe("I386-RET-FAR-PM-SAME-SIZE16",
            (const type_unsigned_8[]){ 0xcbu }, 1u, ret_frame, sizeof(ret_frame), 33u) ||
        timing_80386_manifest_run_s6_return_recipe("I386-RET-FAR-PM-SAME-IMM",
            (const type_unsigned_8[]){ 0xcau,0u,0u }, 3u, ret_frame, sizeof(ret_frame), 33u) ||
        timing_80386_manifest_run_s6_return_recipe("I386-RET-FAR-PM-SAME-IMM-SIZE16",
            (const type_unsigned_8[]){ 0xcau,0u,0u }, 3u, ret_frame, sizeof(ret_frame), 33u) ||
        timing_80386_manifest_run_s6_return_recipe("I386-IRET-PM-SAME",
            (const type_unsigned_8[]){ 0xcfu }, 1u, iret_frame, sizeof(iret_frame), 39u) ||
        timing_80386_manifest_run_s6_return_recipe("I386-IRET-PM-SAME-SIZE16",
            (const type_unsigned_8[]){ 0xcfu }, 1u, iret_frame, sizeof(iret_frame), 39u) ||
        timing_80386_manifest_run_s6_return_recipe("I386-RET-FAR-PM-SAME-SIZE32",
            (const type_unsigned_8[]){ 0x66u,0xcbu }, 2u, ret_frame32,
            sizeof(ret_frame32), 33u) ||
        timing_80386_manifest_run_s6_return_recipe("I386-RET-FAR-PM-SAME-IMM-SIZE32",
            (const type_unsigned_8[]){ 0x66u,0xcau,0u,0u }, 4u, ret_frame32,
            sizeof(ret_frame32), 33u) ||
        timing_80386_manifest_run_s6_return_recipe("I386-IRET-PM-SAME-SIZE32",
            (const type_unsigned_8[]){ 0x66u,0xcfu }, 2u, iret_frame32,
            sizeof(iret_frame32), 39u) ||
        timing_80386_manifest_run_s6_interrupt_recipe("I386-INT3-PM-SAME", 3u,
            (const type_unsigned_8[]){ 0xccu }, 1u) ||
        timing_80386_manifest_run_s6_interrupt_recipe("I386-INT-IMM-PM-SAME",
            S3_VECTOR, (const type_unsigned_8[]){ 0xcdu,S3_VECTOR }, 2u) ||
        timing_80386_manifest_run_s6_interrupt_recipe("I386-INTO-PM-SAME", 4u,
            (const type_unsigned_8[]){ 0xceu }, 1u) ||
        timing_80386_manifest_run_s6_outer_return_recipe("I386-RET-FAR-PM-OUTER",
            (const type_unsigned_8[]){ 0xcbu }, 1u, ret_outer_frame,
            sizeof(ret_outer_frame), 69u) ||
        timing_80386_manifest_run_s6_outer_return_recipe("I386-RET-FAR-PM-OUTER-IMM",
            (const type_unsigned_8[]){ 0xcau,0u,0u }, 3u, ret_outer_frame,
            sizeof(ret_outer_frame), 69u) ||
        timing_80386_manifest_run_s6_outer_return_recipe("I386-IRET-PM-OUTER",
            (const type_unsigned_8[]){ 0xcfu }, 1u, iret_outer_frame,
            sizeof(iret_outer_frame), 82u) ||
        timing_80386_manifest_run_s6_outer_return_recipe("I386-RET-FAR-PM-OUTER-SIZE16",
            (const type_unsigned_8[]){ 0xcbu }, 1u, ret_outer_frame,
            sizeof(ret_outer_frame), 69u) ||
        timing_80386_manifest_run_s6_outer_return_recipe("I386-RET-FAR-PM-OUTER-IMM-SIZE16",
            (const type_unsigned_8[]){ 0xcau,0u,0u }, 3u, ret_outer_frame,
            sizeof(ret_outer_frame), 69u) ||
        timing_80386_manifest_run_s6_outer_return_recipe("I386-IRET-PM-OUTER-SIZE16",
            (const type_unsigned_8[]){ 0xcfu }, 1u, iret_outer_frame,
            sizeof(iret_outer_frame), 82u) ||
        timing_80386_manifest_run_s6_outer_return_recipe("I386-RET-FAR-PM-OUTER-SIZE32",
            (const type_unsigned_8[]){ 0x66u,0xcbu }, 2u, ret_outer_frame32,
            sizeof(ret_outer_frame32), 69u) ||
        timing_80386_manifest_run_s6_outer_return_recipe("I386-RET-FAR-PM-OUTER-IMM-SIZE32",
            (const type_unsigned_8[]){ 0x66u,0xcau,0u,0u }, 4u, ret_outer_frame32,
            sizeof(ret_outer_frame32), 69u) ||
        timing_80386_manifest_run_s6_outer_return_recipe("I386-IRET-PM-OUTER-SIZE32",
            (const type_unsigned_8[]){ 0x66u,0xcfu }, 2u, iret_outer_frame32,
            sizeof(iret_outer_frame32), 82u) ||
        timing_80386_manifest_run_s6_direct_recipe("I386-CALL-GATE-SAME-DIRECT",
            (const type_unsigned_8[]){ 0x9au,0u,0u,0x30u,0u }, 5u, 53u) ||
        timing_80386_manifest_run_s6_direct_recipe("I386-JMP-GATE-SAME-DIRECT",
            (const type_unsigned_8[]){ 0xeau,0u,0u,0x30u,0u }, 5u, 46u) ||
        timing_80386_manifest_run_s6_memory_recipe("I386-CALL-GATE-SAME-M",
            call_memory16, sizeof(call_memory16),
            (const type_unsigned_8[]){ 0u,0u,0x30u,0u }, 4u, 57u) ||
        timing_80386_manifest_run_s6_memory_recipe("I386-JMP-GATE-SAME-M",
            jmp_memory16, sizeof(jmp_memory16),
            (const type_unsigned_8[]){ 0u,0u,0x30u,0u }, 4u, 50u) ||
        timing_80386_manifest_run_s6_direct_recipe("I386-CALL-GATE-SAME-DIRECT-SIZE16",
            (const type_unsigned_8[]){ 0x9au,0u,0u,0x30u,0u }, 5u, 53u) ||
        timing_80386_manifest_run_s6_direct_recipe("I386-CALL-GATE-SAME-DIRECT-SIZE32",
            (const type_unsigned_8[]){ 0x66u,0x9au,0u,0u,0u,0u,0x30u,0u }, 8u, 53u) ||
        timing_80386_manifest_run_s6_memory_recipe("I386-CALL-GATE-SAME-M-SIZE16",
            call_memory16, sizeof(call_memory16),
            (const type_unsigned_8[]){ 0u,0u,0x30u,0u }, 4u, 57u) ||
        timing_80386_manifest_run_s6_memory_recipe("I386-CALL-GATE-SAME-M-SIZE32",
            call_memory32, sizeof(call_memory32),
            (const type_unsigned_8[]){ 0u,0u,0u,0u,0x30u,0u }, 6u, 57u) ||
        timing_80386_manifest_run_s6_direct_recipe("I386-JMP-GATE-SAME-DIRECT-SIZE16",
            (const type_unsigned_8[]){ 0xeau,0u,0u,0x30u,0u }, 5u, 46u) ||
        timing_80386_manifest_run_s6_direct_recipe("I386-JMP-GATE-SAME-DIRECT-SIZE32",
            (const type_unsigned_8[]){ 0x66u,0xeau,0u,0u,0u,0u,0x30u,0u }, 8u, 46u) ||
        timing_80386_manifest_run_s6_memory_recipe("I386-JMP-GATE-SAME-M-SIZE16",
            jmp_memory16, sizeof(jmp_memory16),
            (const type_unsigned_8[]){ 0u,0u,0x30u,0u }, 4u, 50u) ||
        timing_80386_manifest_run_s6_memory_recipe("I386-JMP-GATE-SAME-M-SIZE32",
            jmp_memory32, sizeof(jmp_memory32),
            (const type_unsigned_8[]){ 0u,0u,0u,0u,0x30u,0u }, 6u, 50u) ||
        timing_80386_manifest_run_s6_outer_call_gate_recipe(
            "I386-CALL-GATE-MORE0-DIRECT",
            (const type_unsigned_8[]){ 0x9au,0u,0u,0x33u,0u }, 5u, 0u, 87u) ||
        timing_80386_manifest_run_s6_outer_call_gate_recipe(
            "I386-CALL-GATE-MOREP-DIRECT",
            (const type_unsigned_8[]){ 0x9au,0u,0u,0x33u,0u }, 5u, 2u, 103u) ||
        timing_80386_manifest_run_s6_outer_call_gate_recipe(
            "I386-CALL-GATE-MORE0-DIRECT-SIZE16",
            (const type_unsigned_8[]){ 0x9au,0u,0u,0x33u,0u }, 5u, 0u, 87u) ||
        timing_80386_manifest_run_s6_outer_call_gate_recipe(
            "I386-CALL-GATE-MORE0-DIRECT-SIZE32",
            (const type_unsigned_8[]){ 0x66u,0x9au,0u,0u,0u,0u,0x33u,0u }, 8u,
            0u, 87u) ||
        timing_80386_manifest_run_s6_outer_call_gate_recipe(
            "I386-CALL-GATE-MOREP-DIRECT-SIZE16",
            (const type_unsigned_8[]){ 0x9au,0u,0u,0x33u,0u }, 5u, 2u, 103u) ||
        timing_80386_manifest_run_s6_outer_call_gate_recipe(
            "I386-CALL-GATE-MOREP-DIRECT-SIZE32",
            (const type_unsigned_8[]){ 0x66u,0x9au,0u,0u,0u,0u,0x33u,0u }, 8u,
            2u, 103u) ||
        timing_80386_manifest_run_s6_outer_call_gate_memory_recipe(
            "I386-CALL-GATE-MORE0-M",
            (const type_unsigned_8[]){ 0xffu,0x1eu,0u,0x40u }, 4u,
            (const type_unsigned_8[]){ 0u,0u,0x33u,0u }, 4u, 0u, 91u) ||
        timing_80386_manifest_run_s6_outer_call_gate_memory_recipe(
            "I386-CALL-GATE-MOREP-M",
            (const type_unsigned_8[]){ 0xffu,0x1eu,0u,0x40u }, 4u,
            (const type_unsigned_8[]){ 0u,0u,0x33u,0u }, 4u, 2u, 107u) ||
        timing_80386_manifest_run_s6_outer_call_gate_memory_recipe(
            "I386-CALL-GATE-MORE0-M-SIZE16",
            (const type_unsigned_8[]){ 0xffu,0x1eu,0u,0x40u }, 4u,
            (const type_unsigned_8[]){ 0u,0u,0x33u,0u }, 4u, 0u, 91u) ||
        timing_80386_manifest_run_s6_outer_call_gate_memory_recipe(
            "I386-CALL-GATE-MORE0-M-SIZE32",
            (const type_unsigned_8[]){ 0x66u,0xffu,0x1eu,0u,0x40u }, 5u,
            (const type_unsigned_8[]){ 0u,0u,0u,0u,0x33u,0u }, 6u, 0u, 91u) ||
        timing_80386_manifest_run_s6_outer_call_gate_memory_recipe(
            "I386-CALL-GATE-MOREP-M-SIZE16",
            (const type_unsigned_8[]){ 0xffu,0x1eu,0u,0x40u }, 4u,
            (const type_unsigned_8[]){ 0u,0u,0x33u,0u }, 4u, 2u, 107u) ||
        timing_80386_manifest_run_s6_outer_call_gate_memory_recipe(
            "I386-CALL-GATE-MOREP-M-SIZE32",
            (const type_unsigned_8[]){ 0x66u,0xffu,0x1eu,0u,0x40u }, 5u,
            (const type_unsigned_8[]){ 0u,0u,0u,0u,0x33u,0u }, 6u, 2u, 107u) ||
        timing_80386_manifest_run_s6_inner_interrupt_recipe("I386-INT3-PM-INNER",
            3u, (const type_unsigned_8[]){ 0xccu }, 1u) ||
        timing_80386_manifest_run_s6_inner_interrupt_recipe("I386-INT-IMM-PM-INNER",
            S3_VECTOR, (const type_unsigned_8[]){ 0xcdu,S3_VECTOR }, 2u) ||
        timing_80386_manifest_run_s6_inner_interrupt_recipe("I386-INTO-PM-INNER",
            4u, (const type_unsigned_8[]){ 0xceu }, 1u) ||
        timing_80386_manifest_run_s6_task_interrupt_recipe("I386-INT3-TASK",
            3u, (const type_unsigned_8[]){ 0xccu }, 1u, TYPE_FALSE) ||
        timing_80386_manifest_run_s6_task_interrupt_recipe("I386-INT-IMM-TASK",
            3u, (const type_unsigned_8[]){ 0xcdu,3u }, 2u, TYPE_FALSE) ||
        timing_80386_manifest_run_s6_task_interrupt_recipe("I386-INTO-TASK",
            4u, (const type_unsigned_8[]){ 0xceu }, 1u, TYPE_TRUE) ||
        timing_80386_manifest_run_s6_task_recipe("I386-JMP-TASK-TSS-DIRECT",
            TASK_SWITCH_CASE_SUCCESS, 10u, TYPE_FALSE, TYPE_FALSE, TYPE_FALSE, 394u) ||
        timing_80386_manifest_run_s6_task_recipe("I386-CALL-TASK-DIRECT",
            TASK_SWITCH_CASE_CALL_SUCCESS, 10u, TYPE_FALSE, TYPE_FALSE, TYPE_FALSE, 394u) ||
        timing_80386_manifest_run_s6_task_recipe("I386-JMP-TASK-TSS-M",
            TASK_SWITCH_CASE_INDIRECT_SUCCESS, 12u, TYPE_FALSE, TYPE_FALSE, TYPE_FALSE, 399u) ||
        timing_80386_manifest_run_s6_task_recipe("I386-JMP-TASK-GATE-DIRECT",
            TASK_SWITCH_CASE_TASK_GATE_SUCCESS, 10u, TYPE_TRUE, TYPE_FALSE, TYPE_FALSE, 403u) ||
        timing_80386_manifest_run_s6_task_recipe("I386-JMP-TASK-GATE-DIRECT-SIZE16",
            TASK_SWITCH_CASE_TASK_GATE_SUCCESS, 10u, TYPE_TRUE, TYPE_FALSE, TYPE_FALSE, 403u) ||
        timing_80386_manifest_run_s6_task_recipe("I386-JMP-TASK-GATE-DIRECT-SIZE32",
            TASK_SWITCH_CASE_TASK_GATE_SUCCESS, 10u, TYPE_TRUE, TYPE_FALSE, TYPE_FALSE, 403u) ||
        timing_80386_manifest_run_s6_task_recipe("I386-CALL-TASK-DIRECT",
            TASK_SWITCH_CASE_TASK_GATE_SUCCESS, 10u, TYPE_FALSE, TYPE_FALSE, TYPE_FALSE, 403u) ||
        timing_80386_manifest_run_s6_task_recipe("I386-JMP-TASK-GATE-M",
            TASK_SWITCH_CASE_INDIRECT_SUCCESS, 12u, TYPE_FALSE, TYPE_TRUE, TYPE_FALSE, 408u) ||
        timing_80386_manifest_run_s6_task_recipe("I386-JMP-TASK-TSS-DIRECT-SIZE16",
            TASK_SWITCH_CASE_SUCCESS, 10u, TYPE_FALSE, TYPE_FALSE, TYPE_FALSE, 394u) ||
        timing_80386_manifest_run_s6_task_recipe("I386-JMP-TASK-TSS-DIRECT-SIZE32",
            TASK_SWITCH_CASE_OPERAND32_SUCCESS, 10u, TYPE_FALSE, TYPE_FALSE, TYPE_FALSE, 394u) ||
        timing_80386_manifest_run_s6_iret_task_recipe("I386-IRET-TASK") ||
        timing_80386_manifest_run_s6_iret_task_recipe("I386-IRET-TASK-SIZE16") ||
        timing_80386_manifest_run_s6_iret_task_recipe("I386-IRET-TASK-SIZE32") ||
        timing_80386_manifest_run_s6_task_recipe("I386-JMP-TASK-TSS-M-SIZE16",
            TASK_SWITCH_CASE_INDIRECT_SUCCESS, 12u, TYPE_FALSE, TYPE_FALSE, TYPE_FALSE, 399u) ||
        timing_80386_manifest_run_s6_task_recipe("I386-JMP-TASK-TSS-M-SIZE32",
            TASK_SWITCH_CASE_INDIRECT_OPERAND32_SUCCESS, 12u, TYPE_FALSE,
            TYPE_FALSE, TYPE_FALSE, 399u) ||
        timing_80386_manifest_run_s6_task_recipe("I386-JMP-TASK-GATE-M-SIZE16",
            TASK_SWITCH_CASE_INDIRECT_SUCCESS, 12u, TYPE_FALSE, TYPE_TRUE, TYPE_FALSE, 408u) ||
        timing_80386_manifest_run_s6_task_recipe("I386-JMP-TASK-GATE-M-SIZE32",
            TASK_SWITCH_CASE_INDIRECT_OPERAND32_SUCCESS, 12u, TYPE_FALSE,
            TYPE_TRUE, TYPE_FALSE, 408u) ||
        timing_80386_manifest_run_s6_task_recipe("I386-CALL-TASK-M",
            TASK_SWITCH_CASE_INDIRECT_SUCCESS, 12u, TYPE_FALSE, TYPE_FALSE,
            TYPE_TRUE, 399u) ||
        timing_80386_manifest_run_s6_task_recipe("I386-CALL-TASK-DIRECT-SIZE16",
            TASK_SWITCH_CASE_CALL_SUCCESS, 10u, TYPE_FALSE, TYPE_FALSE,
            TYPE_FALSE, 394u) ||
        timing_80386_manifest_run_s6_task_recipe("I386-CALL-TASK-DIRECT-SIZE32",
            TASK_SWITCH_CASE_CALL_SUCCESS, 10u, TYPE_FALSE, TYPE_FALSE,
            TYPE_FALSE, 394u) ||
        timing_80386_manifest_run_s6_task_recipe("I386-CALL-TASK-M-SIZE16",
            TASK_SWITCH_CASE_INDIRECT_SUCCESS, 12u, TYPE_FALSE, TYPE_FALSE,
            TYPE_TRUE, 399u) ||
        timing_80386_manifest_run_s6_task_recipe("I386-CALL-TASK-M-SIZE32",
            TASK_SWITCH_CASE_INDIRECT_OPERAND32_SUCCESS, 12u, TYPE_FALSE,
            TYPE_FALSE, TYPE_TRUE, 399u) ||
        timing_80386_manifest_run_s6_vm86_interrupt_recipe("I386-INT3-VM86-INNER",
            3u, (const type_unsigned_8[]){ 0xccu }, 1u) ||
        timing_80386_manifest_run_s6_vm86_interrupt_recipe("I386-INT-IMM-VM86-INNER",
            3u, (const type_unsigned_8[]){ 0xcdu,3u }, 2u) ||
        timing_80386_manifest_run_s6_vm86_interrupt_recipe("I386-INTO-VM86-INNER",
            4u, (const type_unsigned_8[]){ 0xceu }, 1u) ||
        timing_80386_manifest_run_s6_iret_vm86_recipe("I386-IRET-PM-VM86") ||
        timing_80386_manifest_run_s6_iret_vm86_recipe(
            "I386-IRET-PM-VM86-SIZE32");
}

C_INT main(C_VOID)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    static const type_unsigned_8 xlat[] = { 0xd7u };
    static const struct {
        const C_CHAR *key_id;
        type_unsigned_8 opcode;
    } flag_recipes[] = {
        { "I386-FLAG-CLC", 0xf8u }, { "I386-FLAG-CLD", 0xfcu },
        { "I386-FLAG-CLI", 0xfau }, { "I386-FLAG-SAHF", 0x9eu },
        { "I386-FLAG-LAHF", 0x9fu }, { "I386-FLAG-CMC", 0xf5u },
        { "I386-FLAG-STC", 0xf9u }, { "I386-FLAG-STD", 0xfdu },
        { "I386-FLAG-STI", 0xfbu }
    };
    static const struct {
        const C_CHAR *key_id;
        type_unsigned_8 bytes[2];
        STD_SIZE_T byte_count;
    } adjust_recipes[] = {
        { "I386-ADJ-AAA", { 0x37u, 0u }, 1u },
        { "I386-ADJ-AAS", { 0x3fu, 0u }, 1u },
        { "I386-ADJ-DAA", { 0x27u, 0u }, 1u },
        { "I386-ADJ-DAS", { 0x2fu, 0u }, 1u },
        { "I386-ADJ-AAD", { 0xd5u, 0x0au }, 2u },
        { "I386-ADJ-AAM", { 0xd4u, 0x0au }, 2u },
        { "I386-ADJ-CBW", { 0x98u, 0u }, 1u },
        { "I386-ADJ-CWD", { 0x99u, 0u }, 1u },
        { "I386-ADJ-CWDE", { 0x66u, 0x98u }, 2u },
        { "I386-ADJ-CDQ", { 0x66u, 0x99u }, 2u }
    };
    static const struct {
        const C_CHAR *key_id;
        type_unsigned_8 bytes[5];
        STD_SIZE_T byte_count;
    } mov_recipes[] = {
        { "I386-MOV-RR", { 0x89u, 0xc0u, 0u, 0u }, 2u },
        { "I386-MOV-RI", { 0xb8u, 0x01u, 0u, 0u }, 3u }
    };
    static const struct {
        const C_CHAR *key_id;
        type_unsigned_8 bytes[6];
        STD_SIZE_T byte_count;
    } mov_extended_recipes[] = {
        { "I386-MOV-RM", { 0x8bu, 0x06u, 0u, 0x10u, 0u, 0u }, 4u },
        { "I386-MOV-MR", { 0x89u, 0x0eu, 0u, 0x10u, 0u, 0u }, 4u },
        { "I386-MOV-MI", { 0xc7u, 0x06u, 0u, 0x10u, 1u, 0u }, 6u },
        { "I386-MOV-MOFFS-R", { 0xa1u, 0u, 0x10u, 0u, 0u, 0u }, 3u },
        { "I386-MOV-MOFFS-W", { 0xa3u, 0u, 0x10u, 0u, 0u, 0u }, 3u },
        { "I386-MOV-SREG-LOAD", { 0x8eu, 0xc0u, 0u, 0u, 0u, 0u }, 2u },
        { "I386-MOV-SREG-STORE", { 0x8cu, 0x06u, 0u, 0x10u, 0u, 0u }, 4u },
        { "I386-MOV-MOVSX", { 0x0fu, 0xbeu, 0xc0u, 0u, 0u, 0u }, 3u },
        { "I386-MOV-MOVZX", { 0x0fu, 0xb6u, 0xc0u, 0u, 0u, 0u }, 3u },
        { "I386-MOV-LEA", { 0x8du, 0x06u, 0u, 0x10u, 0u, 0u }, 4u },
        { "I386-MOV-LDS", { 0xc5u, 0x06u, 0u, 0x10u, 0u, 0u }, 4u },
        { "I386-MOV-LES", { 0xc4u, 0x06u, 0u, 0x10u, 0u, 0u }, 4u },
        { "I386-MOV-LFS", { 0x0fu, 0xb4u, 0x06u, 0u, 0x10u, 0u }, 5u },
        { "I386-MOV-LGS", { 0x0fu, 0xb5u, 0x06u, 0u, 0x10u, 0u }, 5u },
        { "I386-MOV-LSS", { 0x0fu, 0xb2u, 0x06u, 0u, 0x10u, 0u }, 5u }
    };
    static const struct {
        const C_CHAR *key_id;
        type_unsigned_8 bytes[2];
    } alu_recipes[] = {
        { "I386-ALU-ADD-RR", { 0x01u, 0xc0u } },
        { "I386-ALU-OR-RR", { 0x09u, 0xc0u } },
        { "I386-ALU-ADC-RR", { 0x11u, 0xc0u } },
        { "I386-ALU-SBB-RR", { 0x19u, 0xc0u } },
        { "I386-ALU-AND-RR", { 0x21u, 0xc0u } },
        { "I386-ALU-SUB-RR", { 0x29u, 0xc0u } },
        { "I386-ALU-XOR-RR", { 0x31u, 0xc0u } },
        { "I386-ALU-CMP-RR", { 0x39u, 0xc0u } },
        { "I386-ALU-TEST-RR", { 0x85u, 0xc0u } }
    };
    static const struct {
        const C_CHAR *key_id;
        type_unsigned_8 bytes[4];
        STD_SIZE_T byte_count;
    } test_recipes[] = {
        { "I386-ALU-TEST-RM", { 0x85u, 0x06u, 0u, 0x10u }, 4u },
        { "I386-ALU-TEST-MR", { 0x85u, 0x06u, 0u, 0x10u }, 4u },
        { "I386-ALU-TEST-AI", { 0xa9u, 1u, 0u, 0u }, 3u },
        { "I386-ALU-TEST-RMI", { 0xf7u, 0xc0u, 1u, 0u }, 4u }
    };
    static const struct {
        const C_CHAR *op;
        type_unsigned_8 opcode_base;
        type_unsigned_8 group_extension;
    } alu_operations[] = {
        { "ADD", 0x00u, 0u }, { "OR", 0x08u, 1u },
        { "ADC", 0x10u, 2u }, { "SBB", 0x18u, 3u },
        { "AND", 0x20u, 4u }, { "SUB", 0x28u, 5u },
        { "XOR", 0x30u, 6u }, { "CMP", 0x38u, 7u }
    };
    static const C_CHAR *const alu_nonregister_forms[] = {
        "RM", "MR", "AI", "RMI"
    };
    static const struct {
        const C_CHAR *key_id;
        type_unsigned_8 bytes[2];
        STD_SIZE_T byte_count;
    } unary_recipes[] = {
        { "I386-INC-R", { 0x40u, 0u }, 1u },
        { "I386-DEC-R", { 0x48u, 0u }, 1u },
        { "I386-NEG-R", { 0xf7u, 0xd8u }, 2u },
        { "I386-NOT-R", { 0xf7u, 0xd0u }, 2u },
        { "I386-XCHG-R", { 0x91u, 0u }, 1u }
    };
    static const struct {
        const C_CHAR *key_id;
        type_unsigned_8 bytes[4];
    } unary_memory_recipes[] = {
        { "I386-INC-M", { 0xfeu, 0x06u, 0u, 0x10u } },
        { "I386-DEC-M", { 0xfeu, 0x0eu, 0u, 0x10u } },
        { "I386-NEG-M", { 0xf7u, 0x1eu, 0u, 0x10u } },
        { "I386-NOT-M", { 0xf7u, 0x16u, 0u, 0x10u } },
        { "I386-XCHG-M", { 0x87u, 0x0eu, 0u, 0x10u } }
    };
    static const struct {
        const C_CHAR *key_id;
        type_unsigned_8 bytes[10];
        STD_SIZE_T byte_count;
    } multiply_recipes[] = {
        { "I386-MUL-R8", { 0xf6u, 0xe1u, 0u, 0u, 0u, 0u, 0u, 0u }, 2u },
        { "I386-MUL-R16", { 0xf7u, 0xe1u, 0u, 0u, 0u, 0u, 0u, 0u }, 2u },
        { "I386-MUL-R32", { 0x66u, 0xf7u, 0xe1u, 0u, 0u, 0u, 0u, 0u }, 3u },
        { "I386-MUL-M8", { 0xf6u, 0x26u, 0u, 0x10u, 0u, 0u, 0u, 0u }, 4u },
        { "I386-MUL-M16", { 0xf7u, 0x26u, 0u, 0x10u, 0u, 0u, 0u, 0u }, 4u },
        { "I386-MUL-M32", { 0x66u, 0xf7u, 0x26u, 0u, 0x10u, 0u, 0u, 0u }, 5u },
        { "I386-IMUL1-R8", { 0xf6u, 0xe9u, 0u, 0u, 0u, 0u, 0u, 0u }, 2u },
        { "I386-IMUL1-R16", { 0xf7u, 0xe9u, 0u, 0u, 0u, 0u, 0u, 0u }, 2u },
        { "I386-IMUL1-R32", { 0x66u, 0xf7u, 0xe9u, 0u, 0u, 0u, 0u, 0u }, 3u },
        { "I386-IMUL1-M8", { 0xf6u, 0x2eu, 0u, 0x10u, 0u, 0u, 0u, 0u }, 4u },
        { "I386-IMUL1-M16", { 0xf7u, 0x2eu, 0u, 0x10u, 0u, 0u, 0u, 0u }, 4u },
        { "I386-IMUL1-M32", { 0x66u, 0xf7u, 0x2eu, 0u, 0x10u, 0u, 0u, 0u }, 5u },
        { "I386-IMUL2-RR16", { 0x0fu, 0xafu, 0xc1u, 0u, 0u, 0u, 0u, 0u }, 3u },
        { "I386-IMUL2-MR16", { 0x0fu, 0xafu, 0x06u, 0u, 0x10u, 0u, 0u, 0u }, 5u },
        { "I386-IMUL2-RR32", { 0x66u, 0x0fu, 0xafu, 0xc1u, 0u, 0u, 0u, 0u }, 4u },
        { "I386-IMUL2-MR32", { 0x66u, 0x0fu, 0xafu, 0x06u, 0u, 0x10u, 0u, 0u }, 6u },
        { "I386-IMUL3-RR16-IMM8", { 0x6bu, 0xc1u, 1u, 0u, 0u, 0u, 0u, 0u }, 3u },
        { "I386-IMUL3-MR16-IMM8", { 0x6bu, 0x06u, 0u, 0x10u, 1u, 0u, 0u, 0u }, 5u },
        { "I386-IMUL3-RR32-IMM8", { 0x66u, 0x6bu, 0xc1u, 1u, 0u, 0u, 0u, 0u }, 4u },
        { "I386-IMUL3-MR32-IMM8", { 0x66u, 0x6bu, 0x06u, 0u, 0x10u, 1u, 0u, 0u }, 6u },
        { "I386-IMUL3-RR16-IMM", { 0x69u, 0xc1u, 1u, 0u, 0u, 0u, 0u, 0u }, 4u },
        { "I386-IMUL3-MR16-IMM", { 0x69u, 0x06u, 0u, 0x10u, 1u, 0u, 0u, 0u }, 6u },
        { "I386-IMUL3-RR32-IMM", { 0x66u, 0x69u, 0xc1u, 1u, 0u, 0u, 0u, 0u }, 7u },
        { "I386-IMUL3-MR32-IMM", { 0x66u, 0x69u, 0x06u, 0u, 0x10u, 1u, 0u, 0u, 0u, 0u }, 9u }
    };
    static const struct {
        const C_CHAR *key_id;
        type_unsigned_8 bytes[5];
        STD_SIZE_T byte_count;
    } divide_recipes[] = {
        { "I386-DIV-R8", { 0xf6u, 0xf1u, 0u, 0u, 0u }, 2u },
        { "I386-DIV-R16", { 0xf7u, 0xf1u, 0u, 0u, 0u }, 2u },
        { "I386-DIV-R32", { 0x66u, 0xf7u, 0xf1u, 0u, 0u }, 3u },
        { "I386-DIV-M8", { 0xf6u, 0x36u, 0u, 0x10u, 0u }, 4u },
        { "I386-DIV-M16", { 0xf7u, 0x36u, 0u, 0x10u, 0u }, 4u },
        { "I386-DIV-M32", { 0x66u, 0xf7u, 0x36u, 0u, 0x10u }, 5u },
        { "I386-IDIV-R8", { 0xf6u, 0xf9u, 0u, 0u, 0u }, 2u },
        { "I386-IDIV-R16", { 0xf7u, 0xf9u, 0u, 0u, 0u }, 2u },
        { "I386-IDIV-R32", { 0x66u, 0xf7u, 0xf9u, 0u, 0u }, 3u },
        { "I386-IDIV-M8", { 0xf6u, 0x3eu, 0u, 0x10u, 0u }, 4u },
        { "I386-IDIV-M16", { 0xf7u, 0x3eu, 0u, 0x10u, 0u }, 4u },
        { "I386-IDIV-M32", { 0x66u, 0xf7u, 0x3eu, 0u, 0x10u }, 5u }
    };
    static const struct {
        const C_CHAR *key_id;
        type_unsigned_8 bytes[6];
        STD_SIZE_T byte_count;
    } bound_recipes[] = {
        { "I386-BOUND-M16", { 0x62u, 0x06u, 0u, 0x10u, 0u, 0u }, 4u },
        { "I386-BOUND-M32", { 0x66u, 0x62u, 0x06u, 0u, 0x10u, 0u }, 5u }
    };
    static const struct {
        const C_CHAR *key_id;
        type_unsigned_8 bytes[6];
        STD_SIZE_T byte_count;
    } bit_scan_recipes[] = {
        { "I386-BSF-R16", { 0x0fu, 0xbcu, 0xc1u, 0u, 0u, 0u }, 3u },
        { "I386-BSF-M16", { 0x0fu, 0xbcu, 0x06u, 0u, 0x10u, 0u }, 5u },
        { "I386-BSF-R32", { 0x66u, 0x0fu, 0xbcu, 0xc1u, 0u, 0u }, 4u },
        { "I386-BSF-M32", { 0x66u, 0x0fu, 0xbcu, 0x06u, 0u, 0x10u }, 6u },
        { "I386-BSR-R16", { 0x0fu, 0xbdu, 0xc1u, 0u, 0u, 0u }, 3u },
        { "I386-BSR-M16", { 0x0fu, 0xbdu, 0x06u, 0u, 0x10u, 0u }, 5u },
        { "I386-BSR-R32", { 0x66u, 0x0fu, 0xbdu, 0xc1u, 0u, 0u }, 4u },
        { "I386-BSR-M32", { 0x66u, 0x0fu, 0xbdu, 0x06u, 0u, 0x10u }, 6u }
    };
    static const struct {
        const C_CHAR *op;
        type_unsigned_8 register_opcode;
        type_unsigned_8 immediate_extension;
    } bit_operations[] = {
        { "BT", 0xa3u, 4u }, { "BTC", 0xbbu, 7u },
        { "BTR", 0xb3u, 6u }, { "BTS", 0xabu, 5u }
    };
    static const struct {
        const C_CHAR *form;
        C_INT operand32;
        C_INT immediate;
        C_INT memory;
    } bit_forms[] = {
        { "RR16", 0, 0, 0 }, { "MR16", 0, 0, 1 },
        { "RI16", 0, 1, 0 }, { "MI16", 0, 1, 1 },
        { "RR32", 1, 0, 0 }, { "MR32", 1, 0, 1 },
        { "RI32", 1, 1, 0 }, { "MI32", 1, 1, 1 }
    };
    static const struct {
        const C_CHAR *op;
        type_unsigned_8 immediate_opcode;
        type_unsigned_8 cl_opcode;
    } double_shift_operations[] = {
        { "SHLD", 0xa4u, 0xa5u }, { "SHRD", 0xacu, 0xadu }
    };
    static const struct {
        const C_CHAR *op;
        type_unsigned_8 opcode_base;
        type_unsigned_8 group_extension;
    } lock_alu_operations[] = {
        { "ADC", 0x10u, 2u }, { "ADD", 0x00u, 0u },
        { "AND", 0x20u, 4u }, { "OR", 0x08u, 1u },
        { "SBB", 0x18u, 3u }, { "SUB", 0x28u, 5u },
        { "XOR", 0x30u, 6u }
    };
    static const struct {
        const C_CHAR *op;
        type_unsigned_8 opcode;
        type_unsigned_8 modrm;
    } lock_unary_operations[] = {
        { "INC", 0xfeu, 0x06u }, { "DEC", 0xfeu, 0x0eu },
        { "NEG", 0xf7u, 0x1eu }, { "NOT", 0xf7u, 0x16u },
        { "XCHG", 0x87u, 0x0eu }
    };
    static const struct {
        const C_CHAR *op;
        type_unsigned_8 opcode_base;
        type_unsigned_8 group_extension;
        C_INT test_operation;
    } segment_alu_operations[] = {
        { "ADC", 0x10u, 2u, 0 }, { "ADD", 0x00u, 0u, 0 },
        { "AND", 0x20u, 4u, 0 }, { "OR", 0x08u, 1u, 0 },
        { "SBB", 0x18u, 3u, 0 }, { "SUB", 0x28u, 5u, 0 },
        { "XOR", 0x30u, 6u, 0 }, { "CMP", 0x38u, 7u, 0 },
        { "TEST", 0u, 0u, 1 }
    };
    static const struct {
        const C_CHAR *form;
        C_INT operand32;
        C_INT cl_count;
    } double_shift_forms[] = {
        { "RMCL-16", 0, 1 }, { "RMIMM-16", 0, 0 },
        { "RMCL-32", 1, 1 }, { "RMIMM-32", 1, 0 }
    };
    static const struct {
        const C_CHAR *key_id;
        type_unsigned_8 bytes[5];
        STD_SIZE_T byte_count;
    } setcc_recipes[] = {
        { "I386-SETCC-R8", { 0x0fu, 0x97u, 0xc0u, 0u, 0u }, 3u },
        { "I386-SETCC-M8", { 0x0fu, 0x97u, 0x06u, 0u, 0x10u }, 5u }
    };
    static const type_unsigned_8 hlt[] = { 0xf4u };
    static const struct {
        const C_CHAR *key_id;
        type_unsigned_8 bytes[7];
        STD_SIZE_T byte_count;
    } mov_segment_recipes[] = {
        { "I386-MOV-RM-SEGMENT", { 0x26u, 0x8bu, 0x06u, 0u, 0x10u, 0u, 0u }, 5u },
        { "I386-MOV-MR-SEGMENT", { 0x26u, 0x89u, 0x0eu, 0u, 0x10u, 0u, 0u }, 5u },
        { "I386-MOV-MI-SEGMENT", { 0x26u, 0xc7u, 0x06u, 0u, 0x10u, 1u, 0u }, 7u },
        { "I386-MOV-MOFFS-R-SEGMENT", { 0x26u, 0xa1u, 0u, 0x10u, 0u, 0u, 0u }, 4u },
        { "I386-MOV-MOFFS-W-SEGMENT", { 0x26u, 0xa3u, 0u, 0x10u, 0u, 0u, 0u }, 4u },
        { "I386-MOV-LDS-SEGMENT", { 0x26u, 0xc5u, 0x06u, 0u, 0x10u, 0u, 0u }, 5u },
        { "I386-MOV-LES-SEGMENT", { 0x26u, 0xc4u, 0x06u, 0u, 0x10u, 0u, 0u }, 5u },
        { "I386-MOV-LFS-SEGMENT", { 0x26u, 0x0fu, 0xb4u, 0x06u, 0u, 0x10u, 0u }, 6u },
        { "I386-MOV-LGS-SEGMENT", { 0x26u, 0x0fu, 0xb5u, 0x06u, 0u, 0x10u, 0u }, 6u },
        { "I386-MOV-LSS-SEGMENT", { 0x26u, 0x0fu, 0xb2u, 0x06u, 0u, 0x10u, 0u }, 6u }
    };
    static const struct {
        const C_CHAR *op;
        type_unsigned_8 extension;
    } group2_operations[] = {
        { "ROL", 0u }, { "ROR", 1u }, { "RCL", 2u }, { "RCR", 3u },
        { "SHL", 4u }, { "SHR", 5u }, { "SAR", 7u }
    };
    static const struct {
        const C_CHAR *form;
        type_unsigned_8 opcode;
        C_INT operand32;
        C_INT has_immediate;
    } group2_forms[] = {
        { "RM1-16", 0xd0u, 0, 0 }, { "RMCL-16", 0xd2u, 0, 0 },
        { "RMIMM-16", 0xc0u, 0, 1 }, { "RM1-32", 0xd1u, 1, 0 },
        { "RMCL-32", 0xd3u, 1, 0 }, { "RMIMM-32", 0xc1u, 1, 1 }
    };
    STD_SIZE_T index;

    if (timing_80386_manifest_expected_count() != 1413u) return 1;
    if (timing_80386_manifest_run_recipe("I386-FLAG-NOP", nop, sizeof(nop))) {
        STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:I386-FLAG-NOP\n");
        return 1;
    }
    if (timing_80386_manifest_run_recipe("I386-XLAT", xlat, sizeof(xlat))) {
        STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:I386-XLAT\n");
        return 1;
    }
    if (timing_80386_manifest_run_size_contexts("I386-XLAT", xlat,
            sizeof(xlat))) {
        STD_PRINTF("M5:T437:S3:I386-SIZE-RECIPE-FAIL:I386-XLAT\n");
        return 1;
    }
    if (timing_80386_manifest_run_recipe("I386-WAIT",
            (const type_unsigned_8[]){ 0x9bu }, 1u)) {
        STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:I386-WAIT\n");
        return 1;
    }
    for (index = 0u; index < sizeof(flag_recipes) / sizeof(flag_recipes[0]);
        ++index) {
        if (timing_80386_manifest_run_recipe(flag_recipes[index].key_id,
                &flag_recipes[index].opcode, 1u)) {
            STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:%s\n",
                flag_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(adjust_recipes) /
            sizeof(adjust_recipes[0]); ++index) {
        if (timing_80386_manifest_run_recipe(adjust_recipes[index].key_id,
                adjust_recipes[index].bytes, adjust_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:%s\n",
                adjust_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(mov_recipes) / sizeof(mov_recipes[0]);
        ++index) {
        if (timing_80386_manifest_run_recipe(mov_recipes[index].key_id,
                mov_recipes[index].bytes, mov_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:%s\n",
                mov_recipes[index].key_id);
            return 1;
        }
        if (timing_80386_manifest_run_size_contexts(mov_recipes[index].key_id,
                mov_recipes[index].bytes, mov_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-SIZE-RECIPE-FAIL:%s\n",
                mov_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(mov_extended_recipes) /
            sizeof(mov_extended_recipes[0]); ++index) {
        if (timing_80386_manifest_run_recipe(
                mov_extended_recipes[index].key_id,
                mov_extended_recipes[index].bytes,
                mov_extended_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:%s\n",
                mov_extended_recipes[index].key_id);
            return 1;
        }
        if (index < 10u && timing_80386_manifest_run_size_contexts(
                mov_extended_recipes[index].key_id,
                mov_extended_recipes[index].bytes,
                mov_extended_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-SIZE-RECIPE-FAIL:%s\n",
                mov_extended_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(alu_recipes) / sizeof(alu_recipes[0]);
        ++index) {
        if (timing_80386_manifest_run_recipe(alu_recipes[index].key_id,
                alu_recipes[index].bytes, sizeof(alu_recipes[index].bytes))) {
            STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:%s\n",
                alu_recipes[index].key_id);
            return 1;
        }
        if (timing_80386_manifest_run_size_contexts(alu_recipes[index].key_id,
                alu_recipes[index].bytes, sizeof(alu_recipes[index].bytes))) {
            STD_PRINTF("M5:T437:S3:I386-SIZE-RECIPE-FAIL:%s\n",
                alu_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(test_recipes) / sizeof(test_recipes[0]);
        ++index) {
        if (timing_80386_manifest_run_recipe(test_recipes[index].key_id,
                test_recipes[index].bytes, test_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:%s\n",
                test_recipes[index].key_id);
            return 1;
        }
        if (timing_80386_manifest_run_size_contexts(test_recipes[index].key_id,
                test_recipes[index].bytes, test_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-SIZE-RECIPE-FAIL:%s\n",
                test_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(alu_operations) /
            sizeof(alu_operations[0]); ++index) {
        STD_SIZE_T form_index;

        for (form_index = 0u; form_index < sizeof(alu_nonregister_forms) /
                sizeof(alu_nonregister_forms[0]); ++form_index) {
            C_CHAR key_id[48];
            type_unsigned_8 program[6] = { 0 };
            STD_SIZE_T program_bytes = 0u;
            const C_CHAR *form = alu_nonregister_forms[form_index];

            if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-ALU-%s-%s",
                    alu_operations[index].op, form) < 0) return 1;
            if (STD_STRCMP(form, "RM") == 0) {
                program[program_bytes++] = alu_operations[index].opcode_base + 3u;
                program[program_bytes++] = 0x06u;
                program[program_bytes++] = 0x00u;
                program[program_bytes++] = 0x10u;
            } else if (STD_STRCMP(form, "MR") == 0) {
                program[program_bytes++] = alu_operations[index].opcode_base + 1u;
                program[program_bytes++] = 0x0eu;
                program[program_bytes++] = 0x00u;
                program[program_bytes++] = 0x10u;
            } else if (STD_STRCMP(form, "AI") == 0) {
                program[program_bytes++] = alu_operations[index].opcode_base + 5u;
                program[program_bytes++] = 1u;
                program[program_bytes++] = 0u;
            } else {
                program[program_bytes++] = 0x81u;
                program[program_bytes++] = (type_unsigned_8)(0xc0u |
                    (alu_operations[index].group_extension << 3u));
                program[program_bytes++] = 1u;
                program[program_bytes++] = 0u;
            }
            if (timing_80386_manifest_run_recipe(key_id, program, program_bytes)) {
                STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:%s\n", key_id);
                return 1;
            }
            if (timing_80386_manifest_run_size_contexts(key_id, program,
                    program_bytes)) {
                STD_PRINTF("M5:T437:S3:I386-SIZE-RECIPE-FAIL:%s\n", key_id);
                return 1;
            }
        }
    }
    for (index = 0u; index < sizeof(unary_recipes) /
            sizeof(unary_recipes[0]); ++index) {
        if (timing_80386_manifest_run_recipe(unary_recipes[index].key_id,
                unary_recipes[index].bytes, unary_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:%s\n",
                unary_recipes[index].key_id);
            return 1;
        }
        if (timing_80386_manifest_run_size_contexts(unary_recipes[index].key_id,
                unary_recipes[index].bytes, unary_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-SIZE-RECIPE-FAIL:%s\n",
                unary_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(unary_memory_recipes) /
            sizeof(unary_memory_recipes[0]); ++index) {
        if (timing_80386_manifest_run_recipe(
                unary_memory_recipes[index].key_id,
                unary_memory_recipes[index].bytes,
                sizeof(unary_memory_recipes[index].bytes))) {
            STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:%s\n",
                unary_memory_recipes[index].key_id);
            return 1;
        }
        if (timing_80386_manifest_run_size_contexts(
                unary_memory_recipes[index].key_id,
                unary_memory_recipes[index].bytes,
                sizeof(unary_memory_recipes[index].bytes))) {
            STD_PRINTF("M5:T437:S3:I386-SIZE-RECIPE-FAIL:%s\n",
                unary_memory_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(multiply_recipes) /
            sizeof(multiply_recipes[0]); ++index) {
        if (timing_80386_manifest_run_recipe(multiply_recipes[index].key_id,
                multiply_recipes[index].bytes,
                multiply_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:%s\n",
                multiply_recipes[index].key_id);
            return 1;
        }
        if (timing_80386_manifest_run_size_contexts(
                multiply_recipes[index].key_id, multiply_recipes[index].bytes,
                multiply_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-SIZE-RECIPE-FAIL:%s\n",
                multiply_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(multiply_recipes) /
            sizeof(multiply_recipes[0]); ++index) {
        static const C_CHAR *const kinds[] = {
            "ZERO", "LOW", "HIGH", "MEMORY"
        };
        STD_SIZE_T kind_index;

        for (kind_index = 0u; kind_index < sizeof(kinds) / sizeof(kinds[0]);
                ++kind_index) {
            C_CHAR key_id[80];

            if (STD_SNPRINTF(key_id, sizeof(key_id), "%s-MULTIPLIER-%s",
                    multiply_recipes[index].key_id, kinds[kind_index]) < 0 ||
                timing_80386_manifest_run_recipe(key_id,
                    multiply_recipes[index].bytes,
                    multiply_recipes[index].byte_count)) {
                STD_PRINTF("M5:T437:S3:I386-MULTIPLIER-RECIPE-FAIL:%s\n",
                    key_id);
                return 1;
            }
        }
    }
    for (index = 0u; index < sizeof(divide_recipes) /
            sizeof(divide_recipes[0]); ++index) {
        C_CHAR key_id[56];
        type_unsigned_8 program[6];

        if (divide_recipes[index].key_id[9] != 'M' &&
            divide_recipes[index].key_id[10] != 'M') continue;
        program[0] = 0x26u;
        STD_MEMCPY(program + 1u, divide_recipes[index].bytes,
            divide_recipes[index].byte_count);
        if (STD_SNPRINTF(key_id, sizeof(key_id), "%s-SEGMENT",
                divide_recipes[index].key_id) < 0 ||
            timing_80386_manifest_run_recipe(key_id, program,
                divide_recipes[index].byte_count + 1u)) {
            STD_PRINTF("M5:T437:S3:I386-SEGMENT-RECIPE-FAIL:%s\n", key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(bit_scan_recipes) /
            sizeof(bit_scan_recipes[0]); ++index) {
        C_CHAR key_id[56];
        type_unsigned_8 program[7];

        if (bit_scan_recipes[index].key_id[9] != 'M') continue;
        program[0] = 0x26u;
        STD_MEMCPY(program + 1u, bit_scan_recipes[index].bytes,
            bit_scan_recipes[index].byte_count);
        if (STD_SNPRINTF(key_id, sizeof(key_id), "%s-SEGMENT",
                bit_scan_recipes[index].key_id) < 0 ||
            timing_80386_manifest_run_recipe(key_id, program,
                bit_scan_recipes[index].byte_count + 1u)) {
            STD_PRINTF("M5:T437:S3:I386-SEGMENT-RECIPE-FAIL:%s\n", key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(bound_recipes) / sizeof(bound_recipes[0]);
            ++index) {
        C_CHAR key_id[56];
        type_unsigned_8 program[7];

        program[0] = 0x26u;
        STD_MEMCPY(program + 1u, bound_recipes[index].bytes,
            bound_recipes[index].byte_count);
        if (STD_SNPRINTF(key_id, sizeof(key_id), "%s-SEGMENT",
                bound_recipes[index].key_id) < 0 ||
            timing_80386_manifest_run_recipe(key_id, program,
                bound_recipes[index].byte_count + 1u)) {
            STD_PRINTF("M5:T437:S3:I386-SEGMENT-RECIPE-FAIL:%s\n", key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(divide_recipes) /
            sizeof(divide_recipes[0]); ++index) {
        if (timing_80386_manifest_run_recipe(divide_recipes[index].key_id,
                divide_recipes[index].bytes,
                divide_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:%s\n",
                divide_recipes[index].key_id);
            return 1;
        }
        if (timing_80386_manifest_run_size_contexts(divide_recipes[index].key_id,
                divide_recipes[index].bytes, divide_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-SIZE-RECIPE-FAIL:%s\n",
                divide_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(bound_recipes) /
            sizeof(bound_recipes[0]); ++index) {
        if (timing_80386_manifest_run_recipe(bound_recipes[index].key_id,
                bound_recipes[index].bytes, bound_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:%s\n",
                bound_recipes[index].key_id);
            return 1;
        }
        if (timing_80386_manifest_run_size_contexts(bound_recipes[index].key_id,
                bound_recipes[index].bytes, bound_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-SIZE-RECIPE-FAIL:%s\n",
                bound_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(bit_scan_recipes) /
            sizeof(bit_scan_recipes[0]); ++index) {
        if (timing_80386_manifest_run_recipe(bit_scan_recipes[index].key_id,
                bit_scan_recipes[index].bytes,
                bit_scan_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:%s\n",
                bit_scan_recipes[index].key_id);
            return 1;
        }
        if (timing_80386_manifest_run_size_contexts(
                bit_scan_recipes[index].key_id, bit_scan_recipes[index].bytes,
                bit_scan_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-SIZE-RECIPE-FAIL:%s\n",
                bit_scan_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(bit_operations) /
            sizeof(bit_operations[0]); ++index) {
        STD_SIZE_T form_index;

        for (form_index = 0u; form_index < sizeof(bit_forms) /
                sizeof(bit_forms[0]); ++form_index) {
            C_CHAR key_id[48];
            type_unsigned_8 program[8];
            STD_SIZE_T program_bytes = 0u;

            if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-%s-%s",
                    bit_operations[index].op, bit_forms[form_index].form) < 0) {
                return 1;
            }
            if (bit_forms[form_index].operand32) program[program_bytes++] = 0x66u;
            program[program_bytes++] = 0x0fu;
            if (bit_forms[form_index].immediate) {
                program[program_bytes++] = 0xbau;
                program[program_bytes++] = (type_unsigned_8)((
                    bit_forms[form_index].memory ? 0x06u : 0xc0u) |
                    (bit_operations[index].immediate_extension << 3u));
            } else {
                program[program_bytes++] = bit_operations[index].register_opcode;
                program[program_bytes++] = bit_forms[form_index].memory ? 0x0eu :
                    0xc1u;
            }
            if (bit_forms[form_index].memory) {
                program[program_bytes++] = 0x00u;
                program[program_bytes++] = 0x10u;
            }
            if (bit_forms[form_index].immediate) program[program_bytes++] = 1u;
            if (timing_80386_manifest_run_recipe(key_id, program, program_bytes)) {
                STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:%s\n", key_id);
                return 1;
            }
            if (timing_80386_manifest_run_size_contexts(key_id, program,
                    program_bytes)) {
                STD_PRINTF("M5:T437:S3:I386-SIZE-RECIPE-FAIL:%s\n", key_id);
                return 1;
            }
            if (bit_forms[form_index].memory) {
                type_unsigned_8 segment_program[9];

                segment_program[0] = 0x26u;
                STD_MEMCPY(segment_program + 1u, program, program_bytes);
                if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-%s-%s-SEGMENT",
                        bit_operations[index].op,
                        bit_forms[form_index].form) < 0 ||
                    timing_80386_manifest_run_recipe(key_id, segment_program,
                        program_bytes + 1u)) {
                    STD_PRINTF("M5:T437:S3:I386-SEGMENT-RECIPE-FAIL:%s\n",
                        key_id);
                    return 1;
                }
            }
        }
    }
    for (index = 0u; index < sizeof(double_shift_operations) /
            sizeof(double_shift_operations[0]); ++index) {
        STD_SIZE_T form_index;

        for (form_index = 0u; form_index < sizeof(double_shift_forms) /
                sizeof(double_shift_forms[0]); ++form_index) {
            C_CHAR key_id[48];
            type_unsigned_8 program[8];
            STD_SIZE_T program_bytes = 0u;

            if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-%s-%s",
                    double_shift_operations[index].op,
                    double_shift_forms[form_index].form) < 0) return 1;
            if (double_shift_forms[form_index].operand32) program[program_bytes++] = 0x66u;
            program[program_bytes++] = 0x0fu;
            program[program_bytes++] = double_shift_forms[form_index].cl_count ?
                double_shift_operations[index].cl_opcode :
                double_shift_operations[index].immediate_opcode;
            program[program_bytes++] = 0xc1u;
            if (!double_shift_forms[form_index].cl_count) program[program_bytes++] = 1u;
            if (timing_80386_manifest_run_recipe(key_id, program, program_bytes)) {
                STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:%s\n", key_id);
                return 1;
            }
            if (timing_80386_manifest_run_size_contexts(key_id, program,
                    program_bytes)) {
                STD_PRINTF("M5:T437:S3:I386-SIZE-RECIPE-FAIL:%s\n", key_id);
                return 1;
            }
            program_bytes = 0u;
            if (double_shift_forms[form_index].operand32) program[program_bytes++] = 0x66u;
            program[program_bytes++] = 0x0fu;
            program[program_bytes++] = double_shift_forms[form_index].cl_count ?
                double_shift_operations[index].cl_opcode :
                double_shift_operations[index].immediate_opcode;
            program[program_bytes++] = 0x0eu;
            program[program_bytes++] = 0x00u;
            program[program_bytes++] = 0x10u;
            if (!double_shift_forms[form_index].cl_count) program[program_bytes++] = 1u;
            if (timing_80386_manifest_run_recipe(key_id, program, program_bytes)) {
                STD_PRINTF("M5:T437:S3:I386-MEMORY-RECIPE-FAIL:%s\n", key_id);
                return 1;
            }
            if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-%s-%s-SEGMENT",
                    double_shift_operations[index].op,
                    double_shift_forms[form_index].form) < 0) return 1;
            program_bytes = 0u;
            program[program_bytes++] = 0x26u;
            if (double_shift_forms[form_index].operand32) program[program_bytes++] = 0x66u;
            program[program_bytes++] = 0x0fu;
            program[program_bytes++] = double_shift_forms[form_index].cl_count ?
                double_shift_operations[index].cl_opcode :
                double_shift_operations[index].immediate_opcode;
            program[program_bytes++] = 0x0eu;
            program[program_bytes++] = 0x00u;
            program[program_bytes++] = 0x10u;
            if (!double_shift_forms[form_index].cl_count) program[program_bytes++] = 1u;
            if (timing_80386_manifest_run_recipe(key_id, program, program_bytes)) {
                STD_PRINTF("M5:T437:S3:I386-SEGMENT-RECIPE-FAIL:%s\n", key_id);
                return 1;
            }
        }
    }
    for (index = 0u; index < sizeof(setcc_recipes) / sizeof(setcc_recipes[0]);
        ++index) {
        if (timing_80386_manifest_run_recipe(setcc_recipes[index].key_id,
                setcc_recipes[index].bytes, setcc_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:%s\n",
                setcc_recipes[index].key_id);
            return 1;
        }
    }
    if (timing_80386_manifest_run_recipe("I386-HLT", hlt, sizeof(hlt))) {
        STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:I386-HLT\n");
        return 1;
    }
    if (timing_80386_manifest_run_recipe("I386-XLAT-SEGMENT",
            (const type_unsigned_8[]){ 0x26u, 0xd7u }, 2u)) {
        STD_PRINTF("M5:T437:S3:I386-SEGMENT-RECIPE-FAIL:I386-XLAT-SEGMENT\n");
        return 1;
    }
    for (index = 0u; index < sizeof(mov_segment_recipes) /
            sizeof(mov_segment_recipes[0]); ++index) {
        if (timing_80386_manifest_run_recipe(mov_segment_recipes[index].key_id,
                mov_segment_recipes[index].bytes,
                mov_segment_recipes[index].byte_count)) {
            STD_PRINTF("M5:T437:S3:I386-SEGMENT-RECIPE-FAIL:%s\n",
                mov_segment_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(lock_alu_operations) /
            sizeof(lock_alu_operations[0]); ++index) {
        C_CHAR key_id[56];
        type_unsigned_8 mr_program[] = { 0xf0u,
            (type_unsigned_8)(lock_alu_operations[index].opcode_base + 1u),
            0x0eu, 0u, 0x10u };
        type_unsigned_8 rmi_program[] = { 0xf0u, 0x81u,
            (type_unsigned_8)(lock_alu_operations[index].group_extension << 3u | 0x06u),
            0u, 0x10u, 1u, 0u };

        if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-ALU-%s-MR-LOCK",
                lock_alu_operations[index].op) < 0 ||
            timing_80386_manifest_run_recipe(key_id, mr_program,
                sizeof(mr_program))) {
            STD_PRINTF("M5:T437:S3:I386-LOCK-RECIPE-FAIL:%s\n", key_id);
            return 1;
        }
        if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-ALU-%s-RMI-LOCK",
                lock_alu_operations[index].op) < 0 ||
            timing_80386_manifest_run_recipe(key_id, rmi_program,
                sizeof(rmi_program))) {
            STD_PRINTF("M5:T437:S3:I386-LOCK-RECIPE-FAIL:%s\n", key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(lock_unary_operations) /
            sizeof(lock_unary_operations[0]); ++index) {
        C_CHAR key_id[48];
        type_unsigned_8 program[] = { 0xf0u, lock_unary_operations[index].opcode,
            lock_unary_operations[index].modrm, 0u, 0x10u };

        if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-%s-M-LOCK",
                lock_unary_operations[index].op) < 0 ||
            timing_80386_manifest_run_recipe(key_id, program, sizeof(program))) {
            STD_PRINTF("M5:T437:S3:I386-LOCK-RECIPE-FAIL:%s\n", key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(multiply_recipes) /
            sizeof(multiply_recipes[0]); ++index) {
        C_CHAR key_id[64];
        type_unsigned_8 program[11];
        STD_SIZE_T key_index;
        C_INT memory_form = 0;

        for (key_index = 0u; multiply_recipes[index].key_id[key_index] != '\0';
                ++key_index) {
            if (key_index > 7u && multiply_recipes[index].key_id[key_index] == '-' &&
                multiply_recipes[index].key_id[key_index + 1u] == 'M') {
                memory_form = 1;
                break;
            }
        }
        if (!memory_form) continue;
        program[0] = 0x26u;
        STD_MEMCPY(program + 1u, multiply_recipes[index].bytes,
            multiply_recipes[index].byte_count);
        if (STD_SNPRINTF(key_id, sizeof(key_id), "%s-SEGMENT",
                multiply_recipes[index].key_id) < 0 ||
            timing_80386_manifest_run_recipe(key_id, program,
                multiply_recipes[index].byte_count + 1u)) {
            STD_PRINTF("M5:T437:S3:I386-SEGMENT-RECIPE-FAIL:%s\n", key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(segment_alu_operations) /
            sizeof(segment_alu_operations[0]); ++index) {
        static const C_CHAR *const forms[] = { "RM", "MR", "RMI" };
        STD_SIZE_T form_index;

        for (form_index = 0u; form_index < sizeof(forms) / sizeof(forms[0]);
                ++form_index) {
            C_CHAR key_id[56];
            type_unsigned_8 program[8];
            STD_SIZE_T program_bytes = 0u;

            if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-ALU-%s-%s-SEGMENT",
                    segment_alu_operations[index].op, forms[form_index]) < 0) {
                return 1;
            }
            program[program_bytes++] = 0x26u;
            if (segment_alu_operations[index].test_operation) {
                if (STD_STRCMP(forms[form_index], "RMI") == 0) {
                    program[program_bytes++] = 0xf7u;
                    program[program_bytes++] = 0x06u;
                } else {
                    program[program_bytes++] = 0x85u;
                    program[program_bytes++] = 0x06u;
                }
            } else if (STD_STRCMP(forms[form_index], "RM") == 0) {
                program[program_bytes++] =
                    segment_alu_operations[index].opcode_base + 3u;
                program[program_bytes++] = 0x06u;
            } else if (STD_STRCMP(forms[form_index], "MR") == 0) {
                program[program_bytes++] =
                    segment_alu_operations[index].opcode_base + 1u;
                program[program_bytes++] = 0x0eu;
            } else {
                program[program_bytes++] = 0x81u;
                program[program_bytes++] = (type_unsigned_8)(
                    segment_alu_operations[index].group_extension << 3u | 0x06u);
            }
            program[program_bytes++] = 0u;
            program[program_bytes++] = 0x10u;
            if (STD_STRCMP(forms[form_index], "RMI") == 0) {
                program[program_bytes++] = 1u;
                program[program_bytes++] = 0u;
            }
            if (timing_80386_manifest_run_recipe(key_id, program, program_bytes)) {
                STD_PRINTF("M5:T437:S3:I386-SEGMENT-RECIPE-FAIL:%s\n", key_id);
                return 1;
            }
        }
    }
    for (index = 0u; index < sizeof(lock_unary_operations) /
            sizeof(lock_unary_operations[0]); ++index) {
        C_CHAR key_id[48];
        type_unsigned_8 program[] = { 0x26u, lock_unary_operations[index].opcode,
            lock_unary_operations[index].modrm, 0u, 0x10u };

        if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-%s-M-SEGMENT",
                lock_unary_operations[index].op) < 0 ||
            timing_80386_manifest_run_recipe(key_id, program, sizeof(program))) {
            STD_PRINTF("M5:T437:S3:I386-SEGMENT-RECIPE-FAIL:%s\n", key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(group2_operations) /
            sizeof(group2_operations[0]); ++index) {
        STD_SIZE_T form_index;

        for (form_index = 0u; form_index < sizeof(group2_forms) /
                sizeof(group2_forms[0]); ++form_index) {
            C_CHAR key_id[48];
            type_unsigned_8 program[7];
            STD_SIZE_T program_bytes = 0u;

            if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-%s-%s",
                    group2_operations[index].op,
                    group2_forms[form_index].form) < 0) return 1;
            if (group2_forms[form_index].operand32) program[program_bytes++] = 0x66u;
            program[program_bytes++] = group2_forms[form_index].opcode;
            program[program_bytes++] = (type_unsigned_8)(0xc0u |
                (group2_operations[index].extension << 3u));
            if (group2_forms[form_index].has_immediate) program[program_bytes++] = 1u;
            if (timing_80386_manifest_run_recipe(key_id, program, program_bytes)) {
                STD_PRINTF("M5:T437:S3:I386-RECIPE-FAIL:%s\n", key_id);
                return 1;
            }
            if (timing_80386_manifest_run_size_contexts(key_id, program,
                    program_bytes)) {
                STD_PRINTF("M5:T437:S3:I386-SIZE-RECIPE-FAIL:%s\n", key_id);
                return 1;
            }
            program_bytes = 0u;
            if (group2_forms[form_index].operand32) program[program_bytes++] = 0x66u;
            program[program_bytes++] = group2_forms[form_index].opcode;
            program[program_bytes++] = (type_unsigned_8)(
                group2_operations[index].extension << 3u | 0x06u);
            program[program_bytes++] = 0x00u;
            program[program_bytes++] = 0x10u;
            if (group2_forms[form_index].has_immediate) program[program_bytes++] = 1u;
            if (timing_80386_manifest_run_recipe(key_id, program, program_bytes)) {
                STD_PRINTF("M5:T437:S3:I386-MEMORY-RECIPE-FAIL:%s\n", key_id);
                return 1;
            }
            if (STD_SNPRINTF(key_id, sizeof(key_id), "I386-%s-%s-SEGMENT",
                    group2_operations[index].op,
                    group2_forms[form_index].form) < 0) return 1;
            program_bytes = 0u;
            program[program_bytes++] = 0x26u;
            if (group2_forms[form_index].operand32) program[program_bytes++] = 0x66u;
            program[program_bytes++] = group2_forms[form_index].opcode;
            program[program_bytes++] = (type_unsigned_8)(
                group2_operations[index].extension << 3u | 0x06u);
            program[program_bytes++] = 0x00u;
            program[program_bytes++] = 0x10u;
            if (group2_forms[form_index].has_immediate) program[program_bytes++] = 1u;
            if (timing_80386_manifest_run_recipe(key_id, program, program_bytes)) {
                STD_PRINTF("M5:T437:S3:I386-SEGMENT-RECIPE-FAIL:%s\n", key_id);
                return 1;
            }
        }
    }
    if (timing_80386_manifest_run_s4_base_recipes()) {
        STD_PRINTF("M5:T437:S4:I386-STRING-IO-BASE-RECIPE-FAIL\n");
        return 1;
    }
    if (timing_80386_manifest_run_s4_context_recipes() ||
        timing_80386_manifest_s4_count(0) != 162u ||
        timing_80386_manifest_s4_count(1) != 162u) {
        STD_PRINTF("M5:T437:S4:I386-STRING-IO-CONTEXT-RECIPE-FAIL\n");
        return 1;
    }
    if (timing_80386_manifest_run_s5_real_base_recipes()) {
        STD_PRINTF("M5:T437:S5:I386-REAL-CONTROL-BASE-RECIPE-FAIL\n");
        return 1;
    }
    if (timing_80386_manifest_run_s5_branch_recipes()) {
        STD_PRINTF("M5:T437:S5:I386-BRANCH-RECIPE-FAIL\n");
        return 1;
    }
    if (timing_80386_manifest_run_s5_segment_recipes()) {
        STD_PRINTF("M5:T437:S5:I386-SEGMENT-RECIPE-FAIL\n");
        return 1;
    }
    if (timing_80386_manifest_run_s6_direct_recipes()) {
        STD_PRINTF("M5:T437:S6:I386-PROTECTED-DIRECT-RECIPE-FAIL\n");
        return 1;
    }
    if (timing_80386_manifest_run_s7_arpl_recipes() ||
        timing_80386_manifest_run_s7_lar_lsl_recipes() ||
        timing_80386_manifest_run_s7_verify_recipes() ||
        timing_80386_manifest_run_s7_clts_recipe() ||
        timing_80386_manifest_run_s7_descriptor_table_recipes() ||
        timing_80386_manifest_run_s7_smsw_recipes() ||
        timing_80386_manifest_run_s7_control_register_recipes() ||
        timing_80386_manifest_run_s7_debug_test_register_recipes() ||
        timing_80386_manifest_run_s7_special_register_size_contexts() ||
        timing_80386_manifest_run_s7_lmsw_sldt_recipes() ||
        timing_80386_manifest_run_s7_pointer_size_recipes() ||
        timing_80386_manifest_run_s7_lldt_recipes() ||
        timing_80386_manifest_run_s7_ltr_recipes() ||
        timing_80386_manifest_run_s7_str_recipes()) {
        STD_PRINTF("M5:T437:S7:I386-PROTECTED-SYSTEM-RECIPE-FAIL\n");
        return 1;
    }
    if (timing_80386_manifest_s5_count(0) != 234u ||
        timing_80386_manifest_s5_count(1) != 234u) {
        STD_PRINTF("M5:T437:S5:I386-ORDINARY-CONTROL-COVERAGE-FAIL:observed=%u:canonical=%u\n",
            timing_80386_manifest_s5_count(1), timing_80386_manifest_s5_count(0));
        return 1;
    }
    if (timing_80386_manifest_verify_esc_handoff() ||
        timing_80386_manifest_s3_count(0) != 809u ||
        timing_80386_manifest_observed_count() == 0u ||
        timing_80386_manifest_write_results(
            PROJECT_TEST_80386_RESULTS_PATH, 1) != 0) {
        STD_PRINTF("M5:T437:S3:I386-POSTCHECK-FAIL:canonical=%u:observed=%u:total=%u\n",
            timing_80386_manifest_s3_count(0), timing_80386_manifest_s3_count(1),
            timing_80386_manifest_observed_count());
        return 1;
    }
    STD_PRINTF("M5:T437:S2:I386-RESULT-PRODUCER:PASS:observed=%u:canonical=%u\n",
        timing_80386_manifest_observed_count(),
        timing_80386_manifest_expected_count());
    STD_PRINTF("M5:T437:S8:I386-RESULT-CLOSURE:PASS:canonical=%u:cpu=1412:mcp=1\n",
        timing_80386_manifest_expected_count());
    STD_PRINTF("M5:T437:S3:I386-NONCONTROL-COVERAGE:observed=%u:canonical=%u\n",
        timing_80386_manifest_s3_count(1), timing_80386_manifest_s3_count(0));
    STD_PRINTF("M5:T437:S4:I386-STRING-IO-OBSERVED:%u\n",
        timing_80386_manifest_s4_count(1));
    STD_PRINTF("M5:T437:S4:I386-STRING-IO-COVERAGE:PASS:canonical=%u\n",
        timing_80386_manifest_s4_count(0));
    STD_PRINTF("M5:T437:S4:I386-STRING-IO-INPUTS:PASS\n");
    STD_PRINTF("M5:T437:S5:I386-ORDINARY-CONTROL-OBSERVED:%u\n",
        timing_80386_manifest_s5_count(1));
    STD_PRINTF("M5:T437:S5:I386-ORDINARY-CONTROL-COVERAGE:PASS:canonical=%u\n",
        timing_80386_manifest_s5_count(0));
    STD_PRINTF("M5:T437:S5:I386-ORDINARY-CONTROL-INPUTS:PASS\n");
    STD_PRINTF("M5:T437:S6:I386-PROTECTED-CONTROL-OBSERVED:%u:canonical=%u\n",
        timing_80386_manifest_s6_count(1), timing_80386_manifest_s6_count(0));
    if (timing_80386_manifest_s6_count(1) ==
        timing_80386_manifest_s6_count(0)) {
        STD_PRINTF("M5:T437:S6:I386-PROTECTED-CONTROL-COVERAGE:PASS:canonical=%u\n",
            timing_80386_manifest_s6_count(0));
    } else {
        timing_80386_manifest_print_missing_s6();
    }
    STD_PRINTF("M5:T437:S7:I386-PROTECTED-SYSTEM-OBSERVED:%u:canonical=%u\n",
        timing_80386_manifest_s7_count(1), timing_80386_manifest_s7_count(0));
    if (timing_80386_manifest_s7_count(1) != timing_80386_manifest_s7_count(0))
        timing_80386_manifest_print_missing_s7();
    if (timing_80386_manifest_s3_count(1) ==
        timing_80386_manifest_s3_count(0)) {
        STD_PRINTF("M5:T437:S3:I386-NONCONTROL-OBSERVED:809\n");
        STD_PRINTF("M5:T437:S3:I386-NONCONTROL-COVERAGE:PASS:canonical=809\n");
        STD_PRINTF("M5:T437:S3:I386-MEMORY-INPUTS:PASS\n");
        STD_PRINTF("M5:T437:S3:I386-ESC-HANDOFF:PASS\n");
        STD_PRINTF("M5:T437:S3:X87-ESC-HANDOFF:PASS\n");
    } else {
        timing_80386_manifest_print_missing_s3();
    }
    return 0;
}
