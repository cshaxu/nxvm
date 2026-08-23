#include "type.h"

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

static C_INT timing_80386_manifest_run_nop(C_VOID)
{
    static const type_unsigned_8 program[] = { 0x90u };
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

    record = timing_80386_manifest_find("I386-FLAG-NOP");
    if (record == STD_NULL || !timing_80386_manifest_is_i386(record)) return 1;
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
        TIMING_80386_MANIFEST_RESET_PHYSICAL, program, sizeof(program));
    if (status == TYPE_STATUS_OK) status =
        core_machine_set_retirement_observation_provider(machine, &active_provider);
    if (status == TYPE_STATUS_OK) status = core_machine_run(machine, budget, &run);
    if (status != TYPE_STATUS_OK || run.reason != CORE_MACHINE_STOP_BUDGET ||
        run.executed != 1u || capture.count != 1u ||
        capture.observation.timing_disposition ==
            CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED ||
        capture.observation.source_timing_form_id ==
            CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
        capture.observation.timing_origin ==
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED) {
        core_machine_destroy(machine);
        return 1;
    }
    core_machine_destroy(machine);
    return 0;
}

C_INT main(C_VOID)
{
    if (timing_80386_manifest_expected_count() != 1411u ||
        timing_80386_manifest_run_nop() ||
        timing_80386_manifest_observed_count() == 0u ||
        timing_80386_manifest_write_results(
            "docs/etc/cpu-timing/t437-s8-80386-timing-results.json", 1) == 0) {
        return 1;
    }
    STD_PRINTF("M5:T437:S2:I386-RESULT-PRODUCER:PASS:observed=%u:canonical=%u\n",
        timing_80386_manifest_observed_count(),
        timing_80386_manifest_expected_count());
    STD_PRINTF("M5:T437:S2:I386-INCOMPLETE-RESULT-REFUSED:PASS\n");
    return 0;
}
