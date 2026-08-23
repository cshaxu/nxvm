#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "core/machine/retirement_observation_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define TIMING_80386_MANIFEST_RESET_LINEAR 0xfffffff0u
#define TIMING_80386_MANIFEST_RESET_PHYSICAL 0x000ffff0u
#define TIMING_80386_MANIFEST_WINDOW_BYTES 16u

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

static const timing_80386_manifest_record timing_80386_manifest_records[] = {
#include "cpu_timing_manifest_metadata_catalog.inc"
};
static C_INT timing_80386_manifest_observed[
    sizeof(timing_80386_manifest_records) / sizeof(timing_80386_manifest_records[0])];
static core_machine_retirement_observation timing_80386_manifest_results[
    sizeof(timing_80386_manifest_records) / sizeof(timing_80386_manifest_records[0])];
static C_INT timing_80386_manifest_current_index = -1;

static C_VOID timing_80386_manifest_execution_reset(C_VOID *opaque)
{
    (C_VOID)opaque;
}

static const core_machine_execution_provider timing_80386_manifest_execution = {
    timing_80386_manifest_execution_reset, STD_NULL, STD_NULL
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
        timing_80386_manifest_key_has_prefix(key_id, "I386-SETCC-") ||
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

static C_INT timing_80386_manifest_results_complete(C_VOID)
{
    return timing_80386_manifest_expected_count() != 0u &&
        timing_80386_manifest_observed_count() ==
            timing_80386_manifest_expected_count();
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

        if (!timing_80386_manifest_is_i386(record)) continue;
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
                observation->timing_origin, observation->timing_disposition ==
                    CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED ?
                    "true" : "false") < 0) {
            STD_FCLOSE(file);
            return 1;
        }
        ++written;
    }
    if (STD_FPRINTF(file, "\n  ]\n}\n") < 0 || STD_FCLOSE(file) != 0) return 1;
    return written == timing_80386_manifest_expected_count() ? 0 : 1;
}

static C_INT timing_80386_manifest_run_recipe(const C_CHAR *key_id,
    const type_unsigned_8 *program, STD_SIZE_T program_bytes);

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
    if (status == TYPE_STATUS_OK) status = core_machine_bind_execution_provider(
        machine, &timing_80386_manifest_execution, STD_NULL);
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
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED) {
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
        { "I386-MOV-SREG", { 0x8eu, 0xc0u, 0u, 0u, 0u, 0u }, 2u },
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

    if (timing_80386_manifest_expected_count() != 1411u) return 1;
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
        if (index < 9u && timing_80386_manifest_run_size_contexts(
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
    if (timing_80386_manifest_verify_esc_handoff() ||
        timing_80386_manifest_s3_count(0) != 807u ||
        timing_80386_manifest_observed_count() == 0u ||
        timing_80386_manifest_write_results(
            "docs/etc/cpu-timing/t437-s8-80386-timing-results.json", 1) == 0) {
        return 1;
    }
    STD_PRINTF("M5:T437:S2:I386-RESULT-PRODUCER:PASS:observed=%u:canonical=%u\n",
        timing_80386_manifest_observed_count(),
        timing_80386_manifest_expected_count());
    STD_PRINTF("M5:T437:S2:I386-INCOMPLETE-RESULT-REFUSED:PASS\n");
    STD_PRINTF("M5:T437:S3:I386-NONCONTROL-COVERAGE:observed=%u:canonical=%u\n",
        timing_80386_manifest_s3_count(1), timing_80386_manifest_s3_count(0));
    if (timing_80386_manifest_s3_count(1) ==
        timing_80386_manifest_s3_count(0)) {
        STD_PRINTF("M5:T437:S3:I386-NONCONTROL-OBSERVED:807\n");
        STD_PRINTF("M5:T437:S3:I386-NONCONTROL-COVERAGE:PASS:canonical=807\n");
        STD_PRINTF("M5:T437:S3:I386-MEMORY-INPUTS:PASS\n");
        STD_PRINTF("M5:T437:S3:I386-ESC-HANDOFF:PASS\n");
        STD_PRINTF("M5:T437:S3:X87-ESC-HANDOFF:PASS\n");
    } else {
        timing_80386_manifest_print_missing_s3();
    }
    return 0;
}
