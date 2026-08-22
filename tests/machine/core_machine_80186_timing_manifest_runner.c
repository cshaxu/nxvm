#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/cpu.h"
#include "core/machine/cpu_timing.h"
#include "core/machine/retirement_observation_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define TIMING_80186_MANIFEST_RESET_LINEAR 0xfffffff0u
#define TIMING_80186_MANIFEST_RESET_PHYSICAL 0x000ffff0u
#define TIMING_80186_MANIFEST_WINDOW_BYTES 16u

/* This is the incremental real-observation runner for the 80186 manifest.
 * It intentionally does not write the final result document until every
 * canonical I186 key has a real recipe; partial output is not a pass. */
typedef struct timing_80186_manifest_record {
    const C_CHAR *key_id;
    const C_CHAR *profile;
    const C_CHAR *level;
    const C_CHAR *source_rule;
    const C_CHAR *context;
} timing_80186_manifest_record;

typedef struct timing_80186_manifest_capture {
    core_machine_retirement_observation observation;
    type_unsigned_32 count;
} timing_80186_manifest_capture;

typedef struct timing_80186_manifest_recipe {
    const C_CHAR *key_id;
    type_unsigned_8 program[4];
    STD_SIZE_T bytes;
    type_unsigned_64 ticks;
    core_machine_retirement_timing_origin origin;
} timing_80186_manifest_recipe;

typedef struct timing_80186_manifest_inputs {
    const C_CHAR *key_id;
    type_unsigned_16 cx;
    type_unsigned_32 memory_address;
    type_unsigned_16 memory_value;
} timing_80186_manifest_inputs;

static const timing_80186_manifest_record timing_80186_manifest_records[] = {
#include "cpu_timing_manifest_metadata_catalog.inc"
};
static C_INT timing_80186_manifest_observed[
    sizeof(timing_80186_manifest_records) / sizeof(timing_80186_manifest_records[0])];
static core_machine_retirement_observation timing_80186_manifest_results[
    sizeof(timing_80186_manifest_records) / sizeof(timing_80186_manifest_records[0])];
static C_INT timing_80186_manifest_current_index = -1;

static C_VOID timing_80186_manifest_execution_reset(C_VOID *opaque)
{
    (C_VOID)opaque;
}

static const core_machine_execution_provider timing_80186_manifest_execution = {
    timing_80186_manifest_execution_reset, STD_NULL, STD_NULL
};

static C_INT timing_80186_manifest_is_i186(
    const timing_80186_manifest_record *record)
{
    return record != STD_NULL && record->key_id[0] == 'I' &&
        record->key_id[1] == '1' && record->key_id[2] == '8' &&
        record->key_id[3] == '6' && record->key_id[4] == '-';
}

static const timing_80186_manifest_record *timing_80186_manifest_find(
    const C_CHAR *key_id)
{
    STD_SIZE_T index;

    timing_80186_manifest_current_index = -1;
    for (index = 0u; index < sizeof(timing_80186_manifest_records) /
            sizeof(timing_80186_manifest_records[0]); ++index) {
        const timing_80186_manifest_record *record =
            &timing_80186_manifest_records[index];

        if (STD_STRCMP(record->key_id, key_id) == 0) {
            timing_80186_manifest_current_index = (C_INT)index;
            return record;
        }
    }
    return STD_NULL;
}

static const timing_80186_manifest_inputs *timing_80186_manifest_inputs_find(
    const C_CHAR *key_id)
{
    static const timing_80186_manifest_inputs inputs[] = {
        { "I186-ROL-RCL", 2u, 0u, 0u }, { "I186-ROR-RCL", 2u, 0u, 0u },
        { "I186-RCL-RCL", 2u, 0u, 0u }, { "I186-RCR-RCL", 2u, 0u, 0u },
        { "I186-SHL-RCL", 2u, 0u, 0u }, { "I186-SHR-RCL", 2u, 0u, 0u },
        { "I186-SAR-RCL", 2u, 0u, 0u },
        { "I186-ROL-M1", 0u, 0x1000u, 1u },
        { "I186-ROR-M1", 0u, 0x1000u, 1u },
        { "I186-RCL-M1", 0u, 0x1000u, 1u },
        { "I186-RCR-M1", 0u, 0x1000u, 1u },
        { "I186-SHL-M1", 0u, 0x1000u, 1u },
        { "I186-SHR-M1", 0u, 0x1000u, 1u },
        { "I186-SAR-M1", 0u, 0x1000u, 1u },
        { "I186-ROL-MCL", 2u, 0x1000u, 1u },
        { "I186-ROR-MCL", 2u, 0x1000u, 1u },
        { "I186-RCL-MCL", 2u, 0x1000u, 1u },
        { "I186-RCR-MCL", 2u, 0x1000u, 1u },
        { "I186-SHL-MCL", 2u, 0x1000u, 1u },
        { "I186-SHR-MCL", 2u, 0x1000u, 1u },
        { "I186-SAR-MCL", 2u, 0x1000u, 1u }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(inputs) / sizeof(inputs[0]); ++index) {
        if (STD_STRCMP(inputs[index].key_id, key_id) == 0) return &inputs[index];
    }
    return STD_NULL;
}

static C_VOID timing_80186_manifest_capture_retirement(C_VOID *opaque,
    const core_machine_retirement_observation *observation)
{
    timing_80186_manifest_capture *capture =
        (timing_80186_manifest_capture *)opaque;

    if (capture == STD_NULL || observation == STD_NULL) return;
    if (capture->count == 0u) capture->observation = *observation;
    if (timing_80186_manifest_current_index >= 0) {
        timing_80186_manifest_results[timing_80186_manifest_current_index] =
            *observation;
        timing_80186_manifest_observed[timing_80186_manifest_current_index] = 1;
    }
    timing_80186_manifest_current_index = -1;
    ++capture->count;
}

static C_INT timing_80186_manifest_prepare(core_machine **out_machine,
    timing_80186_manifest_capture *capture, const type_unsigned_8 *program,
    STD_SIZE_T bytes, const timing_80186_manifest_inputs *inputs)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80186,
        .ticks_per_instruction = 29u,
        .instruction_timing = { 29u, 7u, 31u, 37u, 41u, 43u }
    };
    const core_machine_retirement_observation_provider provider = {
        timing_80186_manifest_capture_retirement, capture
    };
    core_machine *machine = STD_NULL;
    type_status status;

    if (out_machine == STD_NULL || capture == STD_NULL || program == STD_NULL) return 0;
    status = core_machine_create(&config, &machine);
    if (status == TYPE_STATUS_OK) status =
        test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_80186_MANIFEST_RESET_LINEAR,
            TIMING_80186_MANIFEST_RESET_PHYSICAL,
            TIMING_80186_MANIFEST_WINDOW_BYTES);
    if (status == TYPE_STATUS_OK) status = core_machine_bind_execution_provider(
        machine, &timing_80186_manifest_execution, STD_NULL);
    if (status == TYPE_STATUS_OK) status = core_machine_freeze_execution_providers(machine);
    if (status == TYPE_STATUS_OK) status = core_machine_reset(machine);
    if (status == TYPE_STATUS_OK) status = core_machine_set_a20(machine, 1);
    if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
        TIMING_80186_MANIFEST_RESET_LINEAR, program, bytes);
    if (status == TYPE_STATUS_OK && inputs != STD_NULL) {
        machine->executor_cpu.data.cx = inputs->cx;
        if (inputs->memory_value != 0u) status = core_machine_memory_write(machine,
            inputs->memory_address, &inputs->memory_value,
            sizeof(inputs->memory_value));
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

static C_INT timing_80186_manifest_run_recipe(
    const timing_80186_manifest_recipe *recipe)
{
    const core_machine_run_budget budget = { 1u, 0u };
    timing_80186_manifest_capture capture = { { 0 }, 0u };
    const timing_80186_manifest_record *record;
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_INT failed;

    if (recipe == STD_NULL) return 1;
    record = timing_80186_manifest_find(recipe->key_id);
    failed = record == STD_NULL || !timing_80186_manifest_is_i186(record) ||
        STD_STRCMP(record->profile, "80186") != 0 ||
        !timing_80186_manifest_prepare(&machine, &capture, recipe->program,
            recipe->bytes, timing_80186_manifest_inputs_find(recipe->key_id));
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != recipe->ticks || capture.count != 1u ||
            capture.observation.source_ticks != recipe->ticks ||
            capture.observation.timing_origin != recipe->origin ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    static const timing_80186_manifest_recipe recipes[] = {
        { "I186-FLAG-NOP", { 0x90u }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-FLAG-CLC", { 0xf8u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-FLAG-CLD", { 0xfcu }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-FLAG-CLI", { 0xfau }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-FLAG-CMC", { 0xf5u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-FLAG-STC", { 0xf9u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-FLAG-STD", { 0xfdu }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-FLAG-STI", { 0xfbu }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-FLAG-LAHF", { 0x9fu }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-FLAG-SAHF", { 0x9eu }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-ROL-R1", { 0xd0u,0xc0u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-ROR-R1", { 0xd0u,0xc8u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-RCL-R1", { 0xd0u,0xd0u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-RCR-R1", { 0xd0u,0xd8u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-SHL-R1", { 0xd0u,0xe0u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-SHR-R1", { 0xd0u,0xe8u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-SAR-R1", { 0xd0u,0xf8u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-ROL-RCL", { 0xd2u,0xc0u }, 2u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-ROR-RCL", { 0xd2u,0xc8u }, 2u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-RCL-RCL", { 0xd2u,0xd0u }, 2u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-RCR-RCL", { 0xd2u,0xd8u }, 2u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-SHL-RCL", { 0xd2u,0xe0u }, 2u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-SHR-RCL", { 0xd2u,0xe8u }, 2u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-SAR-RCL", { 0xd2u,0xf8u }, 2u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-ROL-RIMM8", { 0xc0u,0xc0u,2u }, 3u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-ROR-RIMM8", { 0xc0u,0xc8u,2u }, 3u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-RCL-RIMM8", { 0xc0u,0xd0u,2u }, 3u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-RCR-RIMM8", { 0xc0u,0xd8u,2u }, 3u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-SHL-RIMM8", { 0xc0u,0xe0u,2u }, 3u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-SHR-RIMM8", { 0xc0u,0xe8u,2u }, 3u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-SAR-RIMM8", { 0xc0u,0xf8u,2u }, 3u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-ROL-M1", { 0xd0u,0x06u,0u,0x10u }, 4u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-ROR-M1", { 0xd0u,0x0eu,0u,0x10u }, 4u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-RCL-M1", { 0xd0u,0x16u,0u,0x10u }, 4u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-RCR-M1", { 0xd0u,0x1eu,0u,0x10u }, 4u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-SHL-M1", { 0xd0u,0x26u,0u,0x10u }, 4u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-SHR-M1", { 0xd0u,0x2eu,0u,0x10u }, 4u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-SAR-M1", { 0xd0u,0x3eu,0u,0x10u }, 4u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-ROL-MCL", { 0xd2u,0x06u,0u,0x10u }, 4u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-ROR-MCL", { 0xd2u,0x0eu,0u,0x10u }, 4u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-RCL-MCL", { 0xd2u,0x16u,0u,0x10u }, 4u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-RCR-MCL", { 0xd2u,0x1eu,0u,0x10u }, 4u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-SHL-MCL", { 0xd2u,0x26u,0u,0x10u }, 4u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-SHR-MCL", { 0xd2u,0x2eu,0u,0x10u }, 4u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-SAR-MCL", { 0xd2u,0x3eu,0u,0x10u }, 4u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-ADJ-AAA", { 0x37u }, 1u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ADJ-AAS", { 0x3fu }, 1u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ADJ-AAD", { 0xd5u,0x0au }, 2u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ADJ-AAM", { 0xd4u,0x0au }, 2u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ADJ-CBW", { 0x98u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ADJ-CWD", { 0x99u }, 1u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ADJ-DAA", { 0x27u }, 1u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ADJ-DAS", { 0x2fu }, 1u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-ADD-RR", { 0x02u,0xc1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-ADD-RI", { 0x80u,0xc0u,1u }, 3u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-ADD-AI", { 0x04u,1u }, 2u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-ADC-RR", { 0x12u,0xc1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-ADC-RI", { 0x80u,0xd0u,1u }, 3u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-ADC-AI", { 0x14u,1u }, 2u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-AND-RR", { 0x22u,0xc1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-AND-RI", { 0x80u,0xe0u,1u }, 3u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-AND-AI", { 0x24u,1u }, 2u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-OR-RR", { 0x0au,0xc1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-OR-RI", { 0x80u,0xc8u,1u }, 3u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-OR-AI", { 0x0cu,1u }, 2u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-SBB-RR", { 0x1au,0xc1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-SBB-RI", { 0x80u,0xd8u,1u }, 3u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-SBB-AI", { 0x1cu,1u }, 2u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-SUB-RR", { 0x2au,0xc1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-SUB-RI", { 0x80u,0xe8u,1u }, 3u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-SUB-AI", { 0x2cu,1u }, 2u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-XOR-RR", { 0x32u,0xc1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-XOR-RI", { 0x80u,0xf0u,1u }, 3u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-XOR-AI", { 0x34u,1u }, 2u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY }
    };
    STD_SIZE_T index;
    STD_SIZE_T observed = 0u;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_80186_manifest_run_recipe(&recipes[index])) return 1;
        ++observed;
    }
    if (observed != sizeof(recipes) / sizeof(recipes[0])) return 1;
    STD_PRINTF("M5:T435:S9:I186-MANIFEST-OBSERVED:%u\n",
        (type_unsigned_32)observed);
    return 0;
}
