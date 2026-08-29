#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/cpu.h"
#include "core/machine/cpu_timing.h"
#include "core/machine/retirement_observation_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define TIMING_80186_MANIFEST_RESET_LINEAR 0x000ffff0u
#define TIMING_80186_MANIFEST_RESET_PHYSICAL 0x000ffff0u
#define TIMING_80186_MANIFEST_WINDOW_BYTES 16u
#define TIMING_80186_MANIFEST_STACK_LINEAR 0x00001000u
#define TIMING_80186_MANIFEST_STACK_BYTES 16u

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
    type_unsigned_8 program[8];
    STD_SIZE_T bytes;
    type_unsigned_64 ticks;
    core_machine_retirement_timing_origin origin;
} timing_80186_manifest_recipe;

typedef struct timing_80186_manifest_flag_recipe {
    timing_80186_manifest_recipe recipe;
    type_unsigned_32 eflags;
} timing_80186_manifest_flag_recipe;

typedef struct timing_80186_manifest_repeat_recipe {
    const C_CHAR *key_id;
    type_unsigned_8 prefix;
    type_unsigned_8 opcode;
    type_unsigned_64 first_ticks;
    type_unsigned_64 continuation_ticks;
    type_unsigned_64 zero_ticks;
} timing_80186_manifest_repeat_recipe;

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
static C_INT timing_80186_manifest_flags_active = 0;
static type_unsigned_32 timing_80186_manifest_eflags;

static C_VOID timing_80186_manifest_report_failure(
    const timing_80186_manifest_recipe *recipe);

static C_VOID timing_80186_manifest_execution_reset(C_VOID *opaque)
{
    (C_VOID)opaque;
}

static const core_machine_execution_provider timing_80186_manifest_execution = {
    timing_80186_manifest_execution_reset, STD_NULL
};

static C_INT timing_80186_manifest_is_i186(
    const timing_80186_manifest_record *record)
{
    return record != STD_NULL && record->key_id[0] == 'I' &&
        record->key_id[1] == '1' && record->key_id[2] == '8' &&
        record->key_id[3] == '6' && record->key_id[4] == '-';
}

static C_INT timing_80186_manifest_is_return_recipe(const C_CHAR *key_id)
{
    return key_id != STD_NULL && (STD_STRCMP(key_id, "I186-RET-NEAR") == 0 ||
        STD_STRCMP(key_id, "I186-RET-NEAR-IMM") == 0 ||
        STD_STRCMP(key_id, "I186-RET-FAR") == 0 ||
        STD_STRCMP(key_id, "I186-RET-FAR-IMM") == 0 ||
        STD_STRCMP(key_id, "I186-RET-IRET") == 0);
}

static C_INT timing_80186_manifest_is_interrupt_recipe(const C_CHAR *key_id)
{
    return key_id != STD_NULL && (STD_STRCMP(key_id, "I186-INT3") == 0 ||
        STD_STRCMP(key_id, "I186-INT-IMM") == 0 ||
        STD_STRCMP(key_id, "I186-INTO-TAKEN") == 0 ||
        STD_STRCMP(key_id, "I186-INTO-NOT") == 0);
}

static C_INT timing_80186_manifest_is_halt_recipe(const C_CHAR *key_id)
{
    return key_id != STD_NULL && STD_STRCMP(key_id, "I186-HLT") == 0;
}

static C_INT timing_80186_manifest_is_bound_recipe(const C_CHAR *key_id)
{
    return key_id != STD_NULL && STD_STRCMP(key_id, "I186-BOUND") == 0;
}

static C_INT timing_80186_manifest_is_repeat_phase_context(
    const timing_80186_manifest_record *record)
{
    return record != STD_NULL && record->context[0] == 'R' &&
        record->context[1] == 'E' && record->context[2] == 'P' &&
        record->context[3] == '-' && record->context[4] == 'P' &&
        record->context[5] == 'H' && record->context[6] == 'A' &&
        record->context[7] == 'S' && record->context[8] == 'E' &&
        record->context[9] == '-';
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
        { "I186-SAR-MCL", 2u, 0x1000u, 1u },
        { "I186-ROL-MIMM8", 0u, 0x1000u, 1u },
        { "I186-ROR-MIMM8", 0u, 0x1000u, 1u },
        { "I186-RCL-MIMM8", 0u, 0x1000u, 1u },
        { "I186-RCR-MIMM8", 0u, 0x1000u, 1u },
        { "I186-SHL-MIMM8", 0u, 0x1000u, 1u },
        { "I186-SHR-MIMM8", 0u, 0x1000u, 1u },
        { "I186-SAR-MIMM8", 0u, 0x1000u, 1u },
        { "I186-CMP-RM", 0u, 0x1000u, 1u },
        { "I186-CMP-MR", 0u, 0x1000u, 1u },
        { "I186-CMP-MI", 0u, 0x1000u, 1u },
        { "I186-TEST-RM", 0u, 0x1000u, 1u },
        { "I186-TEST-MI", 0u, 0x1000u, 1u },
        { "I186-INC-M", 0u, 0x1000u, 1u },
        { "I186-DEC-M", 0u, 0x1000u, 1u },
        { "I186-NEG-M", 0u, 0x1000u, 1u },
        { "I186-NOT-M", 0u, 0x1000u, 1u },
        { "I186-XCHG-MR", 0u, 0x1000u, 1u },
        { "I186-MOV-MOFFS-R", 0u, 0x1000u, 1u },
        { "I186-MOV-RM", 0u, 0x1000u, 1u },
        { "I186-MOV-MR", 0u, 0x1000u, 1u },
        { "I186-MOV-SREG-TO-M", 0u, 0x1000u, 1u },
        { "I186-MOV-SREG-FROM-M", 0u, 0x1000u, 1u },
        { "I186-MOV-MI", 0u, 0x1000u, 1u },
        { "I186-ALU-ADD-RM", 0u, 0x1000u, 1u },
        { "I186-ALU-ADD-MR", 0u, 0x1000u, 1u },
        { "I186-ALU-ADD-MI", 0u, 0x1000u, 1u },
        { "I186-ALU-OR-RM", 0u, 0x1000u, 1u },
        { "I186-ALU-OR-MR", 0u, 0x1000u, 1u },
        { "I186-ALU-OR-MI", 0u, 0x1000u, 1u },
        { "I186-ALU-ADC-RM", 0u, 0x1000u, 1u },
        { "I186-ALU-ADC-MR", 0u, 0x1000u, 1u },
        { "I186-ALU-ADC-MI", 0u, 0x1000u, 1u },
        { "I186-ALU-SBB-RM", 0u, 0x1000u, 1u },
        { "I186-ALU-SBB-MR", 0u, 0x1000u, 1u },
        { "I186-ALU-SBB-MI", 0u, 0x1000u, 1u },
        { "I186-ALU-AND-RM", 0u, 0x1000u, 1u },
        { "I186-ALU-AND-MR", 0u, 0x1000u, 1u },
        { "I186-ALU-AND-MI", 0u, 0x1000u, 1u },
        { "I186-ALU-SUB-RM", 0u, 0x1000u, 1u },
        { "I186-ALU-SUB-MR", 0u, 0x1000u, 1u },
        { "I186-ALU-SUB-MI", 0u, 0x1000u, 1u },
        { "I186-ALU-XOR-RM", 0u, 0x1000u, 1u },
        { "I186-ALU-XOR-MR", 0u, 0x1000u, 1u },
        { "I186-ALU-XOR-MI", 0u, 0x1000u, 1u },
        { "I186-MUL-M8", 0u, 0x1000u, 1u },
        { "I186-MUL-M16", 0u, 0x1000u, 1u },
        { "I186-IMUL-M8", 0u, 0x1000u, 1u },
        { "I186-IMUL-M16", 0u, 0x1000u, 1u },
        { "I186-IDIV-M8", 0u, 0x1000u, 1u },
        { "I186-IDIV-M16", 0u, 0x1000u, 1u },
        { "I186-DIV-M8", 0u, 0x1000u, 1u },
        { "I186-DIV-M16", 0u, 0x1000u, 1u },
        { "I186-LDS-M", 0u, 0x1000u, 1u },
        { "I186-LES-M", 0u, 0x1000u, 1u },
        { "I186-PUSH-M", 0u, 0x1000u, 1u },
        { "I186-POP-M", 0u, 0x1000u, 1u },
        { "I186-JCXZ-TAKEN", 0u, 0u, 0u },
        { "I186-JCXZ-NOT", 1u, 0u, 0u },
        { "I186-LOOP-TAKEN", 2u, 0u, 0u },
        { "I186-LOOP-NOT", 1u, 0u, 0u },
        { "I186-LOOPE-TAKEN", 2u, 0u, 0u },
        { "I186-LOOPE-NOT", 2u, 0u, 0u },
        { "I186-LOOPNE-TAKEN", 2u, 0u, 0u },
        { "I186-LOOPNE-NOT", 2u, 0u, 0u },
        { "I186-CALL-M1616", 0u, 0x1000u, 1u },
        { "I186-JMP-M1616", 0u, 0x1000u, 1u },
        { "I186-CALL-RM16", 0u, 0x1000u, 1u },
        { "I186-JMP-RM16", 0u, 0x1000u, 1u }
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
    STD_SIZE_T bytes, const timing_80186_manifest_inputs *inputs,
    const C_CHAR *key_id)
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
    if (status == TYPE_STATUS_OK && (timing_80186_manifest_is_return_recipe(key_id) ||
            timing_80186_manifest_is_interrupt_recipe(key_id))) {
        status = test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_80186_MANIFEST_STACK_LINEAR,
            TIMING_80186_MANIFEST_STACK_LINEAR,
            TIMING_80186_MANIFEST_STACK_BYTES);
    }
    if (status == TYPE_STATUS_OK) status = core_machine_bind_execution_provider(
        machine, &timing_80186_manifest_execution, STD_NULL);
    if (status == TYPE_STATUS_OK) status = core_machine_freeze_execution_providers(machine);
    if (status == TYPE_STATUS_OK) status = core_machine_reset(machine);
    if (status == TYPE_STATUS_OK) status = core_machine_set_a20(machine, 1);
    if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
        TIMING_80186_MANIFEST_RESET_LINEAR, program, bytes);
    /* Keep arithmetic recipes on their successful-retirement path: a nonzero
     * accumulator divisor and a zero high half avoid an incidental #DE. */
    if (status == TYPE_STATUS_OK) {
        machine->executor_cpu.data.ax = 1u;
        machine->executor_cpu.data.dx = 0u;
    }
    if (status == TYPE_STATUS_OK && timing_80186_manifest_flags_active) {
        machine->executor_cpu.data.eflags = timing_80186_manifest_eflags;
    }
    if (status == TYPE_STATUS_OK && inputs != STD_NULL) {
        type_unsigned_32 memory_value = inputs->memory_value;

        machine->executor_cpu.data.cx = inputs->cx;
        if (inputs->memory_value != 0u) status = core_machine_memory_write(machine,
            inputs->memory_address, &memory_value, sizeof(memory_value));
    }
    if (status == TYPE_STATUS_OK && timing_80186_manifest_is_return_recipe(key_id)) {
        const type_unsigned_16 frame[] = { 0xfff5u, 0xf000u, 0x0002u };

        machine->executor_cpu.data.sp = TIMING_80186_MANIFEST_STACK_LINEAR;
        status = core_machine_memory_write(machine,
            TIMING_80186_MANIFEST_STACK_LINEAR, frame, sizeof(frame));
    }
    if (status == TYPE_STATUS_OK && timing_80186_manifest_is_interrupt_recipe(key_id)) {
        const type_unsigned_16 handler[] = { 0xfff5u, 0xf000u };

        machine->executor_cpu.data.sp = TIMING_80186_MANIFEST_STACK_LINEAR +
            TIMING_80186_MANIFEST_STACK_BYTES;
        status = core_machine_memory_write(machine, 3u * 4u, handler,
            sizeof(handler));
        if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
            4u * 4u, handler, sizeof(handler));
        if (status == TYPE_STATUS_OK) status = core_machine_memory_write(machine,
            0x60u * 4u, handler, sizeof(handler));
    }
    if (status == TYPE_STATUS_OK && timing_80186_manifest_is_bound_recipe(key_id)) {
        const type_unsigned_16 bounds[] = { 0u, 2u };

        status = core_machine_memory_write(machine, 0x1000u, bounds,
            sizeof(bounds));
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

static C_INT timing_80186_manifest_run_recipe_with_inputs(
    const timing_80186_manifest_recipe *recipe, const C_CHAR *input_key_id,
    type_unsigned_32 input_address_delta)
{
    const core_machine_run_budget budget = { 1u, 0u };
    timing_80186_manifest_capture capture = { { 0 }, 0u };
    timing_80186_manifest_inputs adjusted_inputs;
    const timing_80186_manifest_inputs *inputs;
    const timing_80186_manifest_record *record;
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_INT failed;

    if (recipe == STD_NULL) return 1;
    inputs = timing_80186_manifest_inputs_find(input_key_id);
    if (inputs != STD_NULL && input_address_delta != 0u) {
        adjusted_inputs = *inputs;
        adjusted_inputs.memory_address += input_address_delta;
        inputs = &adjusted_inputs;
    }
    record = timing_80186_manifest_find(recipe->key_id);
    failed = record == STD_NULL || !timing_80186_manifest_is_i186(record) ||
        STD_STRCMP(record->profile, "80186") != 0 ||
        !timing_80186_manifest_prepare(&machine, &capture, recipe->program,
            recipe->bytes, inputs, recipe->key_id);
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != (timing_80186_manifest_is_halt_recipe(recipe->key_id) ?
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT : CORE_MACHINE_STOP_BUDGET) ||
            run.executed != 1u ||
            run.ticks != recipe->ticks || capture.count != 1u ||
            capture.observation.source_ticks != recipe->ticks ||
            capture.observation.timing_origin != recipe->origin ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80186_manifest_run_recipe(
    const timing_80186_manifest_recipe *recipe)
{
    return recipe == STD_NULL ? 1 :
        timing_80186_manifest_run_recipe_with_inputs(recipe, recipe->key_id, 0u);
}

static C_INT timing_80186_manifest_run_segment_recipe(
    const timing_80186_manifest_recipe *base_recipe)
{
    timing_80186_manifest_recipe recipe;
    C_CHAR key_id[96];
    STD_SIZE_T offset;

    if (base_recipe == STD_NULL || base_recipe->bytes >= sizeof(recipe.program) ||
        STD_STRCMP(base_recipe->key_id, "I186-CALL-RM16") == 0 ||
        STD_STRCMP(base_recipe->key_id, "I186-JMP-RM16") == 0 ||
        STD_SNPRINTF(key_id, sizeof(key_id), "%s-SEGMENT",
            base_recipe->key_id) < 0 || timing_80186_manifest_find(key_id) == STD_NULL)
        return 0;
    recipe = *base_recipe;
    for (offset = recipe.bytes; offset > 0u; --offset) {
        recipe.program[offset] = recipe.program[offset - 1u];
    }
    recipe.program[0] = 0x26u;
    ++recipe.bytes;
    recipe.ticks += 2u;
    recipe.key_id = key_id;
    return timing_80186_manifest_run_recipe_with_inputs(&recipe,
        base_recipe->key_id, 0u);
}

static type_unsigned_8 timing_80186_manifest_odd_word_transfers(
    const C_CHAR *key_id)
{
    static const C_CHAR *const read_modify_write[] = {
        "I186-ALU-ADD-MR", "I186-ALU-ADD-MI",
        "I186-ALU-OR-MR", "I186-ALU-OR-MI",
        "I186-ALU-ADC-MR", "I186-ALU-ADC-MI",
        "I186-ALU-SBB-MR", "I186-ALU-SBB-MI",
        "I186-ALU-AND-MR", "I186-ALU-AND-MI",
        "I186-ALU-SUB-MR", "I186-ALU-SUB-MI",
        "I186-ALU-XOR-MR", "I186-ALU-XOR-MI",
        "I186-INC-M", "I186-DEC-M", "I186-NEG-M", "I186-NOT-M",
        "I186-XCHG-MR",
        "I186-ROL-M1", "I186-ROR-M1", "I186-RCL-M1", "I186-RCR-M1",
        "I186-SHL-M1", "I186-SHR-M1", "I186-SAR-M1",
        "I186-ROL-MCL", "I186-ROR-MCL", "I186-RCL-MCL", "I186-RCR-MCL",
        "I186-SHL-MCL", "I186-SHR-MCL", "I186-SAR-MCL",
        "I186-ROL-MIMM8", "I186-ROR-MIMM8", "I186-RCL-MIMM8",
        "I186-RCR-MIMM8", "I186-SHL-MIMM8", "I186-SHR-MIMM8",
        "I186-SAR-MIMM8"
    };
    static const C_CHAR *const double_word_read[] = {
        "I186-LDS-M", "I186-LES-M", "I186-CALL-M1616", "I186-JMP-M1616"
    };
    STD_SIZE_T index;

    if (key_id == STD_NULL) return 0u;
    for (index = 0u; index < sizeof(read_modify_write) /
            sizeof(read_modify_write[0]); ++index) {
        if (STD_STRCMP(key_id, read_modify_write[index]) == 0) return 2u;
    }
    for (index = 0u; index < sizeof(double_word_read) /
            sizeof(double_word_read[0]); ++index) {
        if (STD_STRCMP(key_id, double_word_read[index]) == 0) return 2u;
    }
    return 1u;
}

static C_INT timing_80186_manifest_run_odd_word_recipe(
    const timing_80186_manifest_recipe *base_recipe)
{
    timing_80186_manifest_recipe recipe;
    C_CHAR key_id[96];
    STD_SIZE_T index;

    if (base_recipe == STD_NULL || STD_SNPRINTF(key_id, sizeof(key_id),
            "%s-ODD-WORD", base_recipe->key_id) < 0 ||
        timing_80186_manifest_find(key_id) == STD_NULL) return 1;
    recipe = *base_recipe;
    switch (recipe.program[0]) {
        case 0x00u: case 0x02u: case 0x08u: case 0x0au:
        case 0x10u: case 0x12u: case 0x18u: case 0x1au:
        case 0x20u: case 0x22u: case 0x28u: case 0x2au:
        case 0x30u: case 0x32u: case 0x38u: case 0x3au:
        case 0x80u: case 0x84u: case 0x86u: case 0x88u:
        case 0x8au: case 0xa0u: case 0xa2u: case 0xc0u:
        case 0xd0u: case 0xd2u: case 0xf6u: case 0xfeu:
            ++recipe.program[0];
            break;
        default:
            break;
    }
    for (index = 0u; index + 1u < recipe.bytes; ++index) {
        if (recipe.program[index] == 0u && recipe.program[index + 1u] == 0x10u) {
            recipe.program[index] = 1u;
            recipe.ticks += 4u * timing_80186_manifest_odd_word_transfers(
                base_recipe->key_id);
            recipe.key_id = key_id;
            if (timing_80186_manifest_run_recipe_with_inputs(&recipe,
                    base_recipe->key_id, 1u)) {
                timing_80186_manifest_report_failure(&recipe);
                return 1;
            }
            return 0;
        }
    }
    return 1;
}

static C_INT timing_80186_manifest_run_flag_recipe(
    const timing_80186_manifest_flag_recipe *recipe)
{
    C_INT failed;

    if (recipe == STD_NULL) return 1;
    timing_80186_manifest_eflags = recipe->eflags;
    timing_80186_manifest_flags_active = 1;
    failed = timing_80186_manifest_run_recipe(&recipe->recipe);
    timing_80186_manifest_flags_active = 0;
    return failed;
}

static C_INT timing_80186_manifest_run_repeat_step(core_machine *machine,
    timing_80186_manifest_capture *capture,
    const C_CHAR *key_id,
    core_machine_retirement_repeat_phase expected_phase,
    type_unsigned_64 expected_ticks, type_unsigned_32 required_inputs)
{
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result run = { 0 };

    if (machine == STD_NULL || capture == STD_NULL || key_id == STD_NULL ||
        timing_80186_manifest_find(key_id) == STD_NULL) return 1;
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
             CORE_MACHINE_CPU_TIMING_INPUT_REPEAT_PHASE)) !=
            (CORE_MACHINE_CPU_TIMING_INPUT_REPEAT |
             CORE_MACHINE_CPU_TIMING_INPUT_REPEAT_PHASE) ||
        (capture->observation.formula_inputs & required_inputs) !=
            required_inputs;
}

static C_INT timing_80186_manifest_run_repeat_recipe(
    const timing_80186_manifest_repeat_recipe *recipe)
{
    const timing_80186_manifest_record *record;
    const type_unsigned_8 program[] = { recipe == STD_NULL ? 0u : recipe->prefix,
        recipe == STD_NULL ? 0u : recipe->opcode };
    const type_unsigned_16 source = 1u;
    type_unsigned_16 destination;
    C_CHAR first_key[96];
    C_CHAR continuation_key[96];
    C_CHAR zero_key[96];
    timing_80186_manifest_capture capture = { { 0 }, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed;

    if (recipe == STD_NULL) return 1;
    if (STD_SNPRINTF(first_key, sizeof(first_key), "%s-REP-PHASE-FIRST",
            recipe->key_id) < 0 || STD_SNPRINTF(continuation_key,
            sizeof(continuation_key), "%s-REP-PHASE-CONTINUE", recipe->key_id) < 0 ||
        STD_SNPRINTF(zero_key, sizeof(zero_key), "%s-REP-PHASE-ZERO",
            recipe->key_id) < 0) return 1;
    record = timing_80186_manifest_find(recipe->key_id);
    failed = record == STD_NULL || !timing_80186_manifest_is_i186(record) ||
        STD_STRCMP(record->profile, "80186") != 0 ||
        !timing_80186_manifest_prepare(&machine, &capture, program,
            sizeof(program), STD_NULL, recipe->key_id);
    if (!failed) {
        machine->executor_cpu.data.es.base = machine->executor_cpu.data.ds.base;
        machine->executor_cpu.data.es.selector = machine->executor_cpu.data.ds.selector;
        destination = recipe->prefix == 0xf3u ? source : 0u;
        failed = core_machine_memory_write(machine, 0x1000u, &source,
            sizeof(source)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
            0x1100u, &destination, sizeof(destination)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        machine->executor_cpu.data.si = 0x1000u;
        machine->executor_cpu.data.di = 0x1100u;
        machine->executor_cpu.data.ax = 1u;
        machine->executor_cpu.data.cx = 2u;
        failed = timing_80186_manifest_run_repeat_step(machine, &capture,
            recipe->key_id,
            CORE_MACHINE_RETIREMENT_REPEAT_FIRST, recipe->first_ticks, 0u) ||
            timing_80186_manifest_run_repeat_step(machine, &capture,
                continuation_key,
                CORE_MACHINE_RETIREMENT_REPEAT_CONTINUATION,
                recipe->continuation_ticks, 0u);
    }
    core_machine_destroy(machine);
    machine = STD_NULL;
    if (!failed) failed = !timing_80186_manifest_prepare(&machine, &capture,
        program, sizeof(program), STD_NULL, recipe->key_id);
    if (!failed) {
        machine->executor_cpu.data.cx = 0u;
        failed = timing_80186_manifest_run_repeat_step(machine, &capture, zero_key,
            CORE_MACHINE_RETIREMENT_REPEAT_ZERO_COUNT, recipe->zero_ticks, 0u);
    }
    core_machine_destroy(machine);
    machine = STD_NULL;
    if (!failed) failed = !timing_80186_manifest_prepare(&machine, &capture,
        program, sizeof(program), STD_NULL, first_key);
    if (!failed) {
        machine->executor_cpu.data.es.base = machine->executor_cpu.data.ds.base;
        machine->executor_cpu.data.es.selector = machine->executor_cpu.data.ds.selector;
        destination = recipe->prefix == 0xf3u ? source : 0u;
        failed = core_machine_memory_write(machine, 0x1000u, &source,
            sizeof(source)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
            0x1100u, &destination, sizeof(destination)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        machine->executor_cpu.data.si = 0x1000u;
        machine->executor_cpu.data.di = 0x1100u;
        machine->executor_cpu.data.ax = 1u;
        machine->executor_cpu.data.cx = 2u;
        failed = timing_80186_manifest_run_repeat_step(machine, &capture, first_key,
            CORE_MACHINE_RETIREMENT_REPEAT_FIRST, recipe->first_ticks, 0u);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80186_manifest_run_repeat_phase_context(
    const timing_80186_manifest_repeat_recipe *recipe, const C_CHAR *context,
    C_INT segment_override, C_INT odd_word)
{
    type_unsigned_8 program[3];
    const type_unsigned_16 value = 1u;
    type_unsigned_16 destination;
    type_unsigned_32 required_inputs = 0u;
    type_unsigned_64 first_ticks;
    type_unsigned_64 continuation_ticks;
    type_unsigned_64 zero_ticks;
    C_CHAR first_key[112];
    C_CHAR continuation_key[112];
    C_CHAR zero_key[112];
    timing_80186_manifest_capture capture = { { 0 }, 0u };
    core_machine *machine = STD_NULL;
    STD_SIZE_T bytes = 0u;
    C_INT source_odd;
    C_INT failed;

    if (recipe == STD_NULL || context == STD_NULL) return 1;
    if (segment_override) {
        program[bytes++] = 0x26u;
        required_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE;
    }
    program[bytes++] = recipe->prefix;
    program[bytes++] = recipe->opcode;
    source_odd = odd_word && (recipe->opcode == 0xa5u ||
        recipe->opcode == 0xa7u || recipe->opcode == 0xadu ||
        recipe->opcode == 0x6fu);
    if (odd_word) required_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD;
    if (STD_SNPRINTF(first_key, sizeof(first_key), "%s-%s-FIRST",
            recipe->key_id, context) < 0 || STD_SNPRINTF(continuation_key,
            sizeof(continuation_key), "%s-%s-CONTINUE", recipe->key_id,
            context) < 0 || STD_SNPRINTF(zero_key, sizeof(zero_key),
            "%s-%s-ZERO", recipe->key_id, context) < 0 ||
        timing_80186_manifest_find(first_key) == STD_NULL ||
        timing_80186_manifest_find(continuation_key) == STD_NULL ||
        timing_80186_manifest_find(zero_key) == STD_NULL) return 0;
    first_ticks = recipe->first_ticks + (segment_override ? 2u : 0u) +
        (odd_word ? 4u : 0u);
    continuation_ticks = recipe->continuation_ticks +
        (segment_override ? 2u : 0u) + (odd_word ? 4u : 0u);
    zero_ticks = recipe->zero_ticks + (segment_override ? 2u : 0u);
    failed = !timing_80186_manifest_prepare(&machine, &capture, program, bytes,
        STD_NULL, first_key);
    if (!failed) {
        machine->executor_cpu.data.es.base = machine->executor_cpu.data.ds.base;
        machine->executor_cpu.data.es.selector = machine->executor_cpu.data.ds.selector;
        machine->executor_cpu.data.si = source_odd ? 0x1001u : 0x1000u;
        machine->executor_cpu.data.di = source_odd ? 0x1100u :
            odd_word ? 0x1101u : 0x1100u;
        machine->executor_cpu.data.ax = value;
        machine->executor_cpu.data.cx = 2u;
        destination = recipe->prefix == 0xf3u ? value : 0u;
        failed = core_machine_memory_write(machine, source_odd ? 0x1001u : 0x1000u,
            &value, sizeof(value)) != TYPE_STATUS_OK ||
            core_machine_memory_write(machine, source_odd ? 0x1100u :
                odd_word ? 0x1101u : 0x1100u,
                &destination, sizeof(destination)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = timing_80186_manifest_run_repeat_step(machine, &capture,
            first_key, CORE_MACHINE_RETIREMENT_REPEAT_FIRST, first_ticks,
            required_inputs);
        if (failed) STD_PRINTF("M5:T435:S9:I186-REP-CONTEXT-STEP:FAIL:%s:expected=%llu:observed=%llu:inputs=%u\\n",
            first_key, first_ticks, capture.observation.source_ticks,
            capture.observation.formula_inputs);
        if (!failed) {
            failed = timing_80186_manifest_run_repeat_step(machine, &capture,
                continuation_key, CORE_MACHINE_RETIREMENT_REPEAT_CONTINUATION,
                continuation_ticks, required_inputs);
            if (failed) STD_PRINTF("M5:T435:S9:I186-REP-CONTEXT-STEP:FAIL:%s:expected=%llu:observed=%llu:inputs=%u\\n",
                continuation_key, continuation_ticks,
                capture.observation.source_ticks,
                capture.observation.formula_inputs);
        }
    }
    core_machine_destroy(machine);
    machine = STD_NULL;
    if (!failed) failed = !timing_80186_manifest_prepare(&machine, &capture,
        program, bytes, STD_NULL, zero_key);
    if (!failed) {
        machine->executor_cpu.data.cx = 0u;
        failed = timing_80186_manifest_run_repeat_step(machine, &capture, zero_key,
            CORE_MACHINE_RETIREMENT_REPEAT_ZERO_COUNT, zero_ticks,
            segment_override ? CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE : 0u);
        if (failed) STD_PRINTF("M5:T435:S9:I186-REP-CONTEXT-STEP:FAIL:%s:expected=%llu:observed=%llu:inputs=%u\\n",
            zero_key, zero_ticks, capture.observation.source_ticks,
            capture.observation.formula_inputs);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80186_manifest_run_repeat_segment_recipe(
    const timing_80186_manifest_repeat_recipe *recipe)
{
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0x26u,
        recipe == STD_NULL ? 0u : recipe->prefix,
        recipe == STD_NULL ? 0u : recipe->opcode };
    const type_unsigned_16 value = 1u;
    C_CHAR key_id[96];
    timing_80186_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_INT failed;

    if (recipe == STD_NULL || STD_SNPRINTF(key_id, sizeof(key_id), "%s-SEGMENT",
            recipe->key_id) < 0 || timing_80186_manifest_find(key_id) == STD_NULL)
        return 0;
    failed = !timing_80186_manifest_prepare(&machine, &capture, program,
        sizeof(program), STD_NULL, key_id);
    if (!failed) {
        machine->executor_cpu.data.es.base = machine->executor_cpu.data.ds.base;
        machine->executor_cpu.data.es.selector = machine->executor_cpu.data.ds.selector;
        machine->executor_cpu.data.si = 0x1000u;
        machine->executor_cpu.data.di = 0x1100u;
        machine->executor_cpu.data.ax = value;
        machine->executor_cpu.data.cx = 1u;
        failed = core_machine_memory_write(machine, 0x1000u, &value,
            sizeof(value)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
            0x1100u, &value, sizeof(value)) != TYPE_STATUS_OK;
    }
    if (!failed) failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
        run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
        run.ticks != recipe->first_ticks + 2u || capture.count != 1u ||
        capture.observation.source_ticks != recipe->first_ticks + 2u ||
        capture.observation.timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO ||
        capture.observation.timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80186_manifest_run_string_odd_recipe(const C_CHAR *key_id,
    type_unsigned_8 opcode, C_INT source_odd, type_unsigned_64 expected_ticks)
{
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { opcode };
    const type_unsigned_16 value = 1u;
    timing_80186_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_INT failed;

    failed = key_id == STD_NULL || timing_80186_manifest_find(key_id) == STD_NULL ||
        !timing_80186_manifest_prepare(&machine, &capture, program,
            sizeof(program), STD_NULL, key_id);
    if (!failed) {
        machine->executor_cpu.data.es.base = machine->executor_cpu.data.ds.base;
        machine->executor_cpu.data.es.selector = machine->executor_cpu.data.ds.selector;
        machine->executor_cpu.data.si = source_odd ? 0x1001u : 0x1000u;
        machine->executor_cpu.data.di = source_odd ? 0x1100u : 0x1101u;
        machine->executor_cpu.data.ax = value;
        failed = core_machine_memory_write(machine, 0x1000u, &value,
            sizeof(value)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
            0x1100u, &value, sizeof(value)) != TYPE_STATUS_OK;
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
            (capture.observation.formula_inputs &
                CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD) == 0u;
    }
    if (failed) STD_PRINTF("M5:T435:S9:I186-STRING-ODD:FAIL:%s:expected=%llu:observed=%llu:inputs=%u\n",
        key_id, expected_ticks, capture.observation.source_ticks,
        capture.observation.formula_inputs);
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80186_manifest_run_string_segment_odd_recipe(
    const C_CHAR *key_id, type_unsigned_8 opcode, type_unsigned_64 expected_ticks)
{
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0x26u, opcode };
    const type_unsigned_16 value = 1u;
    timing_80186_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_INT failed;

    failed = key_id == STD_NULL || timing_80186_manifest_find(key_id) == STD_NULL ||
        !timing_80186_manifest_prepare(&machine, &capture, program,
            sizeof(program), STD_NULL, key_id);
    if (!failed) {
        machine->executor_cpu.data.es.base = machine->executor_cpu.data.ds.base;
        machine->executor_cpu.data.es.selector = machine->executor_cpu.data.ds.selector;
        machine->executor_cpu.data.si = 0x1001u;
        machine->executor_cpu.data.di = 0x1100u;
        failed = core_machine_memory_write(machine, 0x1001u, &value,
            sizeof(value)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
            0x1100u, &value, sizeof(value)) != TYPE_STATUS_OK;
    }
    if (!failed) failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
        run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
        run.ticks != expected_ticks || capture.count != 1u ||
        capture.observation.source_ticks != expected_ticks ||
        capture.observation.timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO ||
        capture.observation.timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        (capture.observation.formula_inputs &
            (CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
             CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD)) !=
            (CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
             CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD);
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80186_manifest_run_repeat_odd_recipe(
    const timing_80186_manifest_repeat_recipe *recipe)
{
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { recipe == STD_NULL ? 0u : recipe->prefix,
        recipe == STD_NULL ? 0u : recipe->opcode };
    const type_unsigned_16 value = 1u;
    C_CHAR key_id[96];
    timing_80186_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_INT source_odd;
    C_INT failed;

    if (recipe == STD_NULL || STD_SNPRINTF(key_id, sizeof(key_id),
            "%s-ODD-WORD", recipe->key_id) < 0) return 1;
    source_odd = recipe->opcode == 0xa5u || recipe->opcode == 0xa7u ||
        recipe->opcode == 0xadu || recipe->opcode == 0x6fu;
    failed = timing_80186_manifest_find(key_id) == STD_NULL ||
        !timing_80186_manifest_prepare(&machine, &capture, program,
            sizeof(program), STD_NULL, key_id);
    if (!failed) {
        machine->executor_cpu.data.es.base = machine->executor_cpu.data.ds.base;
        machine->executor_cpu.data.es.selector = machine->executor_cpu.data.ds.selector;
        machine->executor_cpu.data.si = source_odd ? 0x1001u : 0x1000u;
        machine->executor_cpu.data.di = source_odd ? 0x1100u : 0x1101u;
        machine->executor_cpu.data.ax = value;
        machine->executor_cpu.data.cx = 1u;
        failed = core_machine_memory_write(machine, 0x1000u, &value,
            sizeof(value)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
            0x1100u, &value, sizeof(value)) != TYPE_STATUS_OK;
    }
    if (!failed) failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
        run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
        run.ticks != recipe->first_ticks + 4u || capture.count != 1u ||
        capture.observation.source_ticks != recipe->first_ticks + 4u ||
        capture.observation.timing_origin != CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO ||
        (capture.observation.formula_inputs & CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD) == 0u;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80186_manifest_run_repeat_segment_odd_recipe(
    const timing_80186_manifest_repeat_recipe *recipe)
{
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 program[] = { 0x26u,
        recipe == STD_NULL ? 0u : recipe->prefix,
        recipe == STD_NULL ? 0u : recipe->opcode };
    const type_unsigned_16 value = 1u;
    C_CHAR key_id[96];
    timing_80186_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_INT source_odd;
    C_INT failed;

    if (recipe == STD_NULL || STD_SNPRINTF(key_id, sizeof(key_id),
            "%s-SEGMENT-ODD-WORD", recipe->key_id) < 0 ||
        timing_80186_manifest_find(key_id) == STD_NULL) return 0;
    source_odd = recipe->opcode == 0xa5u || recipe->opcode == 0xa7u ||
        recipe->opcode == 0xadu;
    failed = !timing_80186_manifest_prepare(&machine, &capture, program,
        sizeof(program), STD_NULL, key_id);
    if (!failed) {
        machine->executor_cpu.data.es.base = machine->executor_cpu.data.ds.base;
        machine->executor_cpu.data.es.selector = machine->executor_cpu.data.ds.selector;
        machine->executor_cpu.data.si = source_odd ? 0x1001u : 0x1000u;
        machine->executor_cpu.data.di = source_odd ? 0x1100u : 0x1101u;
        machine->executor_cpu.data.ax = value;
        machine->executor_cpu.data.cx = 1u;
        failed = core_machine_memory_write(machine, 0x1000u, &value,
            sizeof(value)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
            0x1100u, &value, sizeof(value)) != TYPE_STATUS_OK;
    }
    if (!failed) failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
        run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
        run.ticks != recipe->first_ticks + 6u || capture.count != 1u ||
        capture.observation.source_ticks != recipe->first_ticks + 6u ||
        capture.observation.timing_origin != CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO ||
        (capture.observation.formula_inputs &
            (CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
             CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD)) !=
            (CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
             CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD);
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80186_manifest_write_results(C_VOID)
{
    const C_CHAR *const path =
        "docs/etc/cpu-timing/t435-s9-80186-timing-results.json";
    STD_FILE *file = STD_FOPEN(path, "wb");
    STD_SIZE_T index;
    STD_SIZE_T written = 0u;

    if (file == STD_NULL) return 1;
    if (STD_FPRINTF(file, "{\n  \"schema\": \"nxvm.cpu-timing-results.v1\",\n"
            "  \"profile\": \"80186\",\n  \"results\": [\n") < 0) {
        STD_FCLOSE(file);
        return 1;
    }
    for (index = 0u; index < sizeof(timing_80186_manifest_records) /
            sizeof(timing_80186_manifest_records[0]); ++index) {
        const timing_80186_manifest_record *record =
            &timing_80186_manifest_records[index];
        const core_machine_retirement_observation *observation =
            &timing_80186_manifest_results[index];

        if (!timing_80186_manifest_is_i186(record)) continue;
        if (!timing_80186_manifest_observed[index]) {
            STD_FCLOSE(file);
            return 1;
        }
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
    return written == 616u ? 0 : 1;
}

static C_VOID timing_80186_manifest_report_failure(
    const timing_80186_manifest_recipe *recipe)
{
    const timing_80186_manifest_record *record;
    const core_machine_retirement_observation *observation = STD_NULL;

    if (recipe == STD_NULL) return;
    record = timing_80186_manifest_find(recipe->key_id);
    if (record != STD_NULL && timing_80186_manifest_current_index >= 0) {
        observation = &timing_80186_manifest_results[
            timing_80186_manifest_current_index];
    }
    STD_PRINTF("M5:T435:S9:I186-MANIFEST-RECIPE:FAIL:%s:expected=%llu:observed=%llu:origin=%u:disposition=%u:inputs=%u\n",
        recipe->key_id, recipe->ticks,
        observation != STD_NULL ? observation->source_ticks : 0u,
        observation != STD_NULL ? (type_unsigned_32)observation->timing_origin : 0u,
        observation != STD_NULL ? (type_unsigned_32)observation->timing_disposition : 0u,
        observation != STD_NULL ? observation->formula_inputs : 0u);
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
        { "I186-ROL-MIMM8", { 0xc0u,0x06u,0u,0x10u,2u }, 5u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-ROR-MIMM8", { 0xc0u,0x0eu,0u,0x10u,2u }, 5u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-RCL-MIMM8", { 0xc0u,0x16u,0u,0x10u,2u }, 5u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-RCR-MIMM8", { 0xc0u,0x1eu,0u,0x10u,2u }, 5u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-SHL-MIMM8", { 0xc0u,0x26u,0u,0x10u,2u }, 5u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-SHR-MIMM8", { 0xc0u,0x2eu,0u,0x10u,2u }, 5u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-SAR-MIMM8", { 0xc0u,0x3eu,0u,0x10u,2u }, 5u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-CMP-RR", { 0x3au,0xc1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-CMP-RM", { 0x3au,0x06u,0u,0x10u }, 4u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-CMP-MR", { 0x38u,0x0eu,0u,0x10u }, 4u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-CMP-RI", { 0x80u,0xf8u,1u }, 3u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-CMP-MI", { 0x80u,0x3eu,0u,0x10u,1u }, 5u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-CMP-AI", { 0x3cu,1u }, 2u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-TEST-RR", { 0x84u,0xc1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-TEST-RM", { 0x84u,0x06u,0u,0x10u }, 4u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-TEST-RI", { 0xf6u,0xc0u,1u }, 3u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-TEST-MI", { 0xf6u,0x06u,0u,0x10u,1u }, 5u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-TEST-AI", { 0xa8u,1u }, 2u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-INC-R16", { 0x40u }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-INC-R8", { 0xfeu,0xc0u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-INC-M", { 0xffu,0x06u,0u,0x10u }, 4u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-DEC-R16", { 0x48u }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-DEC-R8", { 0xfeu,0xc8u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-DEC-M", { 0xffu,0x0eu,0u,0x10u }, 4u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-NEG-R", { 0xf6u,0xd8u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-NEG-M", { 0xf6u,0x1eu,0u,0x10u }, 4u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-NOT-R", { 0xf6u,0xd0u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-NOT-M", { 0xf6u,0x16u,0u,0x10u }, 4u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-XCHG-AXR", { 0x91u }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-XCHG-RR", { 0x87u,0xc1u }, 2u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-XCHG-MR", { 0x87u,0x06u,0u,0x10u }, 4u, 17u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-MOV-MOFFS-R", { 0xa1u,0u,0x10u }, 3u, 9u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-MOV-MOFFS-W", { 0xa3u,0u,0x10u }, 3u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-MOV-RR", { 0x8bu,0xc1u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-MOV-RM", { 0x8bu,0x0eu,0u,0x10u }, 4u, 9u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-MOV-MR", { 0x89u,0x0eu,0u,0x10u }, 4u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-MOV-SREG-TO-R", { 0x8cu,0xc0u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-MOV-SREG-TO-M", { 0x8cu,0x06u,0u,0x10u }, 4u, 9u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-MOV-SREG-FROM-R", { 0x8eu,0xc0u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-MOV-SREG-FROM-M", { 0x8eu,0x06u,0u,0x10u }, 4u, 11u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-MOV-RI", { 0xb8u,1u,0u }, 3u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-MOV-MI", { 0xc7u,0x06u,0u,0x10u,1u }, 5u, 13u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-ADD-RM", { 0x02u,0x06u,0u,0x10u }, 4u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-ADD-MR", { 0x00u,0x0eu,0u,0x10u }, 4u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-ADD-MI", { 0x80u,0x06u,0u,0x10u,1u }, 5u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-OR-RM", { 0x0au,0x06u,0u,0x10u }, 4u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-OR-MR", { 0x08u,0x0eu,0u,0x10u }, 4u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-OR-MI", { 0x80u,0x0eu,0u,0x10u,1u }, 5u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-ADC-RM", { 0x12u,0x06u,0u,0x10u }, 4u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-ADC-MR", { 0x10u,0x0eu,0u,0x10u }, 4u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-ADC-MI", { 0x80u,0x16u,0u,0x10u,1u }, 5u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-SBB-RM", { 0x1au,0x06u,0u,0x10u }, 4u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-SBB-MR", { 0x18u,0x0eu,0u,0x10u }, 4u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-SBB-MI", { 0x80u,0x1eu,0u,0x10u,1u }, 5u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-AND-RM", { 0x22u,0x06u,0u,0x10u }, 4u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-AND-MR", { 0x20u,0x0eu,0u,0x10u }, 4u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-AND-MI", { 0x80u,0x26u,0u,0x10u,1u }, 5u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-SUB-RM", { 0x2au,0x06u,0u,0x10u }, 4u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-SUB-MR", { 0x28u,0x0eu,0u,0x10u }, 4u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-SUB-MI", { 0x80u,0x2eu,0u,0x10u,1u }, 5u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-XOR-RM", { 0x32u,0x06u,0u,0x10u }, 4u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-XOR-MR", { 0x30u,0x0eu,0u,0x10u }, 4u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-XOR-MI", { 0x80u,0x36u,0u,0x10u,1u }, 5u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-MUL-R8", { 0xf6u,0xe0u }, 2u, 27u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I186-MUL-R16", { 0xf7u,0xe0u }, 2u, 36u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I186-MUL-M8", { 0xf6u,0x26u,0u,0x10u }, 4u, 33u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I186-MUL-M16", { 0xf7u,0x26u,0u,0x10u }, 4u, 42u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I186-IMUL-R8", { 0xf6u,0xe8u }, 2u, 27u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I186-IMUL-R16", { 0xf7u,0xe8u }, 2u, 36u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I186-IMUL-M8", { 0xf6u,0x2eu,0u,0x10u }, 4u, 33u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I186-IMUL-M16", { 0xf7u,0x2eu,0u,0x10u }, 4u, 42u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I186-IDIV-R8", { 0xf6u,0xf8u }, 2u, 48u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I186-IDIV-R16", { 0xf7u,0xf8u }, 2u, 57u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I186-IDIV-M8", { 0xf6u,0x3eu,0u,0x10u }, 4u, 54u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I186-IDIV-M16", { 0xf7u,0x3eu,0u,0x10u }, 4u, 63u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I186-IMUL-IMM-IMM8", { 0x6bu,0xc0u,1u }, 3u, 24u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I186-IMUL-IMM-IMM16", { 0x69u,0xc0u,1u,0u }, 4u, 31u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I186-DIV-R8", { 0xf6u,0xf0u }, 2u, 29u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I186-DIV-R16", { 0xf7u,0xf0u }, 2u, 38u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I186-DIV-M8", { 0xf6u,0x36u,0u,0x10u }, 4u, 35u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I186-DIV-M16", { 0xf7u,0x36u,0u,0x10u }, 4u, 44u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I186-CALL-NEAR", { 0xe8u,0u,0u }, 3u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-JMP-SHORT", { 0xebu,0u }, 2u, 13u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-CALL-FAR", { 0x9au,0u,0u,0u,0u }, 5u, 23u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-CALL-RM16", { 0xffu,0xd0u }, 2u, 13u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-CALL-M1616", { 0xffu,0x1eu,0u,0x10u }, 4u, 38u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-JMP-NEAR", { 0xe9u,0u,0u }, 3u, 13u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-JMP-FAR", { 0xeau,0u,0u,0u,0u }, 5u, 13u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-JMP-RM16", { 0xffu,0xe0u }, 2u, 11u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-JMP-M1616", { 0xffu,0x2eu,0u,0x10u }, 4u, 26u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-RET-NEAR", { 0xc3u }, 1u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-RET-NEAR-IMM", { 0xc2u,0u,0u }, 3u, 18u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-RET-FAR", { 0xcbu }, 1u, 22u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-RET-FAR-IMM", { 0xcau,0u,0u }, 3u, 25u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-RET-IRET", { 0xcfu }, 1u, 28u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-INT3", { 0xccu }, 1u, 45u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-INT-IMM", { 0xcdu,0x60u }, 2u, 47u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-IN-IMM", { 0xe4u,0x80u }, 2u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I186-IN-DX", { 0xecu }, 1u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I186-OUT-IMM", { 0xe6u,0x80u }, 2u, 9u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I186-OUT-DX", { 0xeeu }, 1u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I186-BOUND", { 0x62u,0x06u,0u,0x10u }, 4u, 34u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-ESC-R", { 0xd8u,0xc0u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-ESC-M", { 0xd8u,0x06u,0u,0x10u }, 4u, 6u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-WAIT", { 0x9bu }, 1u, 6u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-HLT", { 0xf4u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-STRING-MOVS-B", { 0xa4u }, 1u, 14u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I186-STRING-MOVS-W", { 0xa5u }, 1u, 14u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I186-STRING-CMPS-B", { 0xa6u }, 1u, 22u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I186-STRING-CMPS-W", { 0xa7u }, 1u, 22u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I186-STRING-STOS-B", { 0xaau }, 1u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I186-STRING-STOS-W", { 0xabu }, 1u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I186-STRING-LODS-B", { 0xacu }, 1u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I186-STRING-LODS-W", { 0xadu }, 1u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I186-STRING-SCAS-B", { 0xaeu }, 1u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I186-STRING-SCAS-W", { 0xafu }, 1u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I186-STRING-INS-B", { 0x6cu }, 1u, 14u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I186-STRING-INS-W", { 0x6du }, 1u, 14u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I186-STRING-OUTS-B", { 0x6eu }, 1u, 14u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I186-STRING-OUTS-W", { 0x6fu }, 1u, 14u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I186-XLAT", { 0xd7u }, 1u, 11u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-LEA-M", { 0x8du,0x06u,0u,0x10u }, 4u, 6u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-LDS-M", { 0xc5u,0x06u,0u,0x10u }, 4u, 18u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-LES-M", { 0xc4u,0x06u,0u,0x10u }, 4u, 18u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-PUSH-R", { 0x50u }, 1u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-PUSH-SEG-ES", { 0x06u }, 1u, 9u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-PUSH-SEG-CS", { 0x0eu }, 1u, 9u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-PUSH-SEG-SS", { 0x16u }, 1u, 9u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-PUSH-SEG-DS", { 0x1eu }, 1u, 9u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-PUSH-M", { 0xffu,0x36u,0u,0x10u }, 4u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-PUSH-IMM16", { 0x68u,1u,0u }, 3u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-PUSH-IMM8", { 0x6au,1u }, 2u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-PUSH-F", { 0x9cu }, 1u, 9u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-POP-R", { 0x58u }, 1u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-POP-SEG-ES", { 0x07u }, 1u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-POP-SEG-SS", { 0x17u }, 1u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-POP-SEG-DS", { 0x1fu }, 1u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK },
        { "I186-POP-M", { 0x8fu,0x06u,0u,0x10u }, 4u, 20u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-POP-F", { 0x9du }, 1u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-POP-PUSHA", { 0x60u }, 1u, 36u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-POP-POPA", { 0x61u }, 1u, 51u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-ENTER-L0", { 0xc8u,0u,0u,0u }, 4u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-ENTER-L1", { 0xc8u,0u,0u,1u }, 4u, 25u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-ENTER-LN", { 0xc8u,0u,0u,2u }, 4u, 38u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I186-LEAVE", { 0xc9u }, 1u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
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
#define TIMING_80186_JCC(KEY, OPCODE, FLAGS, TICKS) \
    { { KEY, { OPCODE,1u }, 2u, TICKS, \
        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK }, FLAGS }
    static const timing_80186_manifest_flag_recipe branch_recipes[] = {
        { { "I186-INTO-TAKEN", { 0xceu }, 1u, 48u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }, VCPU_EFLAGS_OF },
        { { "I186-INTO-NOT", { 0xceu }, 1u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }, 0u },
        TIMING_80186_JCC("I186-JCC-JO-TAKEN", 0x70u, VCPU_EFLAGS_OF, 13u),
        TIMING_80186_JCC("I186-JCC-JO-NOT", 0x70u, 0u, 4u),
        TIMING_80186_JCC("I186-JCC-JNO-TAKEN", 0x71u, 0u, 13u),
        TIMING_80186_JCC("I186-JCC-JNO-NOT", 0x71u, VCPU_EFLAGS_OF, 4u),
        TIMING_80186_JCC("I186-JCC-JB-TAKEN", 0x72u, VCPU_EFLAGS_CF, 13u),
        TIMING_80186_JCC("I186-JCC-JB-NOT", 0x72u, 0u, 4u),
        TIMING_80186_JCC("I186-JCC-JAE-TAKEN", 0x73u, 0u, 13u),
        TIMING_80186_JCC("I186-JCC-JAE-NOT", 0x73u, VCPU_EFLAGS_CF, 4u),
        TIMING_80186_JCC("I186-JCC-JE-TAKEN", 0x74u, VCPU_EFLAGS_ZF, 13u),
        TIMING_80186_JCC("I186-JCC-JE-NOT", 0x74u, 0u, 4u),
        TIMING_80186_JCC("I186-JCC-JNE-TAKEN", 0x75u, 0u, 13u),
        TIMING_80186_JCC("I186-JCC-JNE-NOT", 0x75u, VCPU_EFLAGS_ZF, 4u),
        TIMING_80186_JCC("I186-JCC-JBE-TAKEN", 0x76u, VCPU_EFLAGS_CF, 13u),
        TIMING_80186_JCC("I186-JCC-JBE-NOT", 0x76u, 0u, 4u),
        TIMING_80186_JCC("I186-JCC-JA-TAKEN", 0x77u, 0u, 13u),
        TIMING_80186_JCC("I186-JCC-JA-NOT", 0x77u, VCPU_EFLAGS_CF, 4u),
        TIMING_80186_JCC("I186-JCC-JS-TAKEN", 0x78u, VCPU_EFLAGS_SF, 13u),
        TIMING_80186_JCC("I186-JCC-JS-NOT", 0x78u, 0u, 4u),
        TIMING_80186_JCC("I186-JCC-JNS-TAKEN", 0x79u, 0u, 13u),
        TIMING_80186_JCC("I186-JCC-JNS-NOT", 0x79u, VCPU_EFLAGS_SF, 4u),
        TIMING_80186_JCC("I186-JCC-JP-TAKEN", 0x7au, VCPU_EFLAGS_PF, 13u),
        TIMING_80186_JCC("I186-JCC-JP-NOT", 0x7au, 0u, 4u),
        TIMING_80186_JCC("I186-JCC-JNP-TAKEN", 0x7bu, 0u, 13u),
        TIMING_80186_JCC("I186-JCC-JNP-NOT", 0x7bu, VCPU_EFLAGS_PF, 4u),
        TIMING_80186_JCC("I186-JCC-JL-TAKEN", 0x7cu, VCPU_EFLAGS_SF, 13u),
        TIMING_80186_JCC("I186-JCC-JL-NOT", 0x7cu, 0u, 4u),
        TIMING_80186_JCC("I186-JCC-JGE-TAKEN", 0x7du, 0u, 13u),
        TIMING_80186_JCC("I186-JCC-JGE-NOT", 0x7du, VCPU_EFLAGS_SF, 4u),
        TIMING_80186_JCC("I186-JCC-JLE-TAKEN", 0x7eu, VCPU_EFLAGS_ZF, 13u),
        TIMING_80186_JCC("I186-JCC-JLE-NOT", 0x7eu, 0u, 4u),
        TIMING_80186_JCC("I186-JCC-JG-TAKEN", 0x7fu, 0u, 13u),
        TIMING_80186_JCC("I186-JCC-JG-NOT", 0x7fu, VCPU_EFLAGS_ZF, 4u),
        { { "I186-JCXZ-TAKEN", { 0xe3u,1u }, 2u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }, 0u },
        { { "I186-JCXZ-NOT", { 0xe3u,1u }, 2u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }, 0u },
        { { "I186-LOOP-TAKEN", { 0xe2u,1u }, 2u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }, 0u },
        { { "I186-LOOP-NOT", { 0xe2u,1u }, 2u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }, 0u },
        { { "I186-LOOPE-TAKEN", { 0xe1u,1u }, 2u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }, VCPU_EFLAGS_ZF },
        { { "I186-LOOPE-NOT", { 0xe1u,1u }, 2u, 6u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }, 0u },
        { { "I186-LOOPNE-TAKEN", { 0xe0u,1u }, 2u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }, 0u },
        { { "I186-LOOPNE-NOT", { 0xe0u,1u }, 2u, 6u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }, VCPU_EFLAGS_ZF }
    };
    static const timing_80186_manifest_repeat_recipe repeat_recipes[] = {
        { "I186-REP-MOVS-B", 0xf3u, 0xa4u, 16u, 8u, 8u },
        { "I186-REP-MOVS-W", 0xf3u, 0xa5u, 16u, 8u, 8u },
        { "I186-REP-CMPS-REPE-B", 0xf3u, 0xa6u, 27u, 22u, 5u },
        { "I186-REP-CMPS-REPE-W", 0xf3u, 0xa7u, 27u, 22u, 5u },
        { "I186-REP-CMPS-REPNE-B", 0xf2u, 0xa6u, 27u, 22u, 5u },
        { "I186-REP-CMPS-REPNE-W", 0xf2u, 0xa7u, 27u, 22u, 5u },
        { "I186-REP-STOS-B", 0xf3u, 0xaau, 15u, 9u, 6u },
        { "I186-REP-STOS-W", 0xf3u, 0xabu, 15u, 9u, 6u },
        { "I186-REP-LODS-B", 0xf3u, 0xacu, 17u, 11u, 6u },
        { "I186-REP-LODS-W", 0xf3u, 0xadu, 17u, 11u, 6u },
        { "I186-REP-SCAS-REPE-B", 0xf3u, 0xaeu, 20u, 15u, 5u },
        { "I186-REP-SCAS-REPE-W", 0xf3u, 0xafu, 20u, 15u, 5u },
        { "I186-REP-SCAS-REPNE-B", 0xf2u, 0xaeu, 20u, 15u, 5u },
        { "I186-REP-SCAS-REPNE-W", 0xf2u, 0xafu, 20u, 15u, 5u },
        { "I186-REP-INS-B", 0xf3u, 0x6cu, 16u, 8u, 8u },
        { "I186-REP-INS-W", 0xf3u, 0x6du, 16u, 8u, 8u },
        { "I186-REP-OUTS-B", 0xf3u, 0x6eu, 16u, 8u, 8u },
        { "I186-REP-OUTS-W", 0xf3u, 0x6fu, 16u, 8u, 8u }
    };
    static const timing_80186_manifest_recipe lock_recipes[] = {
        { "I186-ALU-ADD-MR-LOCK", { 0xf0u,0x01u,0x06u,0u,0x10u }, 5u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-OR-MR-LOCK", { 0xf0u,0x09u,0x06u,0u,0x10u }, 5u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-ADC-MR-LOCK", { 0xf0u,0x11u,0x06u,0u,0x10u }, 5u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-SBB-MR-LOCK", { 0xf0u,0x19u,0x06u,0u,0x10u }, 5u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-AND-MR-LOCK", { 0xf0u,0x21u,0x06u,0u,0x10u }, 5u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-SUB-MR-LOCK", { 0xf0u,0x29u,0x06u,0u,0x10u }, 5u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-XOR-MR-LOCK", { 0xf0u,0x31u,0x06u,0u,0x10u }, 5u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-ADD-MI-LOCK", { 0xf0u,0x81u,0x06u,0u,0x10u,1u,0u }, 7u, 18u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-OR-MI-LOCK", { 0xf0u,0x81u,0x0eu,0u,0x10u,1u,0u }, 7u, 18u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-ADC-MI-LOCK", { 0xf0u,0x81u,0x16u,0u,0x10u,1u,0u }, 7u, 18u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-SBB-MI-LOCK", { 0xf0u,0x81u,0x1eu,0u,0x10u,1u,0u }, 7u, 18u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-AND-MI-LOCK", { 0xf0u,0x81u,0x26u,0u,0x10u,1u,0u }, 7u, 18u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-SUB-MI-LOCK", { 0xf0u,0x81u,0x2eu,0u,0x10u,1u,0u }, 7u, 18u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-ALU-XOR-MI-LOCK", { 0xf0u,0x81u,0x36u,0u,0x10u,1u,0u }, 7u, 18u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-INC-M-LOCK", { 0xf0u,0xffu,0x06u,0u,0x10u }, 5u, 17u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-DEC-M-LOCK", { 0xf0u,0xffu,0x0eu,0u,0x10u }, 5u, 17u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-NOT-M-LOCK", { 0xf0u,0xf7u,0x16u,0u,0x10u }, 5u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-NEG-M-LOCK", { 0xf0u,0xf7u,0x1eu,0u,0x10u }, 5u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I186-XCHG-MR-LOCK", { 0xf0u,0x87u,0x06u,0u,0x10u }, 5u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY }
    };
#undef TIMING_80186_JCC
    STD_SIZE_T index;
    STD_SIZE_T observed = 0u;
    STD_SIZE_T base_records = 0u;
    STD_SIZE_T repeat_phase_records = 0u;
    STD_SIZE_T lock_records = 0u;
    STD_SIZE_T lock_segment_records = 0u;
    STD_SIZE_T odd_word_records = 0u;
    STD_SIZE_T segment_records = 0u;
    STD_SIZE_T repeat_combination_records = 0u;
    STD_SIZE_T combined_records = 0u;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_80186_manifest_run_recipe(&recipes[index])) {
            timing_80186_manifest_report_failure(&recipes[index]);
            return 1;
        }
        ++observed;
    }
    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_80186_manifest_run_segment_recipe(&recipes[index])) {
            timing_80186_manifest_report_failure(&recipes[index]);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        C_CHAR key_id[96];
        STD_SIZE_T byte_index;
        C_INT has_memory_displacement = 0;

        if (STD_SNPRINTF(key_id, sizeof(key_id), "%s-ODD-WORD",
                recipes[index].key_id) < 0 ||
            timing_80186_manifest_find(key_id) == STD_NULL) continue;
        for (byte_index = 0u; byte_index + 1u < recipes[index].bytes;
                ++byte_index) {
            if (recipes[index].program[byte_index] == 0u &&
                recipes[index].program[byte_index + 1u] == 0x10u) {
                has_memory_displacement = 1;
                break;
            }
        }
        if (!has_memory_displacement) continue;
        if (timing_80186_manifest_run_odd_word_recipe(&recipes[index])) {
            timing_80186_manifest_report_failure(&recipes[index]);
            return 1;
        }
    }
    if (timing_80186_manifest_run_string_odd_recipe(
            "I186-STRING-MOVS-W-ODD-WORD", 0xa5u, 1, 18u) ||
        timing_80186_manifest_run_string_odd_recipe(
            "I186-STRING-CMPS-W-ODD-WORD", 0xa7u, 1, 26u) ||
        timing_80186_manifest_run_string_odd_recipe(
            "I186-STRING-STOS-W-ODD-WORD", 0xabu, 0, 14u) ||
        timing_80186_manifest_run_string_odd_recipe(
            "I186-STRING-LODS-W-ODD-WORD", 0xadu, 1, 16u) ||
        timing_80186_manifest_run_string_odd_recipe(
            "I186-STRING-SCAS-W-ODD-WORD", 0xafu, 0, 19u) ||
        timing_80186_manifest_run_string_odd_recipe(
            "I186-STRING-INS-W-ODD-WORD", 0x6du, 0, 18u) ||
        timing_80186_manifest_run_string_odd_recipe(
            "I186-STRING-OUTS-W-ODD-WORD", 0x6fu, 1, 18u)) return 1;
    if (timing_80186_manifest_run_string_segment_odd_recipe(
            "I186-STRING-MOVS-W-SEGMENT-ODD-WORD", 0xa5u, 20u) ||
        timing_80186_manifest_run_string_segment_odd_recipe(
            "I186-STRING-CMPS-W-SEGMENT-ODD-WORD", 0xa7u, 28u) ||
        timing_80186_manifest_run_string_segment_odd_recipe(
            "I186-STRING-LODS-W-SEGMENT-ODD-WORD", 0xadu, 18u)) return 1;
    {
        const timing_80186_manifest_recipe control_odd_recipes[] = {
            { "I186-CALL-RM16-ODD-WORD", { 0xffu,0x16u,1u,0x10u }, 4u, 23u,
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
            { "I186-JMP-RM16-ODD-WORD", { 0xffu,0x26u,1u,0x10u }, 4u, 21u,
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }
        };

        for (index = 0u; index < sizeof(control_odd_recipes) /
                sizeof(control_odd_recipes[0]); ++index) {
            const C_CHAR *input_key = index == 0u ? "I186-CALL-RM16" :
                "I186-JMP-RM16";

            if (timing_80186_manifest_run_recipe_with_inputs(
                    &control_odd_recipes[index], input_key, 1u)) {
                timing_80186_manifest_report_failure(&control_odd_recipes[index]);
                return 1;
            }
        }
    }
    {
        const timing_80186_manifest_recipe control_segment_recipes[] = {
            { "I186-CALL-RM16-SEGMENT", { 0x26u,0xffu,0x16u,0u,0x10u },
                5u, 21u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
            { "I186-JMP-RM16-SEGMENT", { 0x26u,0xffu,0x26u,0u,0x10u },
                5u, 19u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }
        };

        for (index = 0u; index < sizeof(control_segment_recipes) /
                sizeof(control_segment_recipes[0]); ++index) {
            const C_CHAR *input_key = index == 0u ? "I186-CALL-RM16" :
                "I186-JMP-RM16";

            if (timing_80186_manifest_run_recipe_with_inputs(
                    &control_segment_recipes[index], input_key, 0u)) {
                timing_80186_manifest_report_failure(
                    &control_segment_recipes[index]);
                return 1;
            }
        }
    }
    for (index = 0u; index < sizeof(branch_recipes) / sizeof(branch_recipes[0]);
            ++index) {
        if (timing_80186_manifest_run_flag_recipe(&branch_recipes[index])) {
            timing_80186_manifest_report_failure(&branch_recipes[index].recipe);
            return 1;
        }
        ++observed;
    }
    for (index = 0u; index < sizeof(repeat_recipes) / sizeof(repeat_recipes[0]);
            ++index) {
        if (timing_80186_manifest_run_repeat_recipe(&repeat_recipes[index])) {
            STD_PRINTF("M5:T435:S9:I186-MANIFEST-REPEAT:FAIL:%s\n",
                repeat_recipes[index].key_id);
            return 1;
        }
        ++observed;
    }
    for (index = 0u; index < sizeof(repeat_recipes) / sizeof(repeat_recipes[0]);
            ++index) {
        if (timing_80186_manifest_run_repeat_segment_recipe(&repeat_recipes[index]))
            return 1;
        if (timing_80186_manifest_run_repeat_phase_context(&repeat_recipes[index],
                "SEGMENT-REP-PHASE", 1, 0)) {
            STD_PRINTF("M5:T435:S9:I186-REP-COMBINATION:FAIL:%s:SEGMENT\n",
                repeat_recipes[index].key_id);
            return 1;
        }
        if (timing_80186_manifest_run_repeat_phase_context(&repeat_recipes[index],
                "ODD-WORD-REP-PHASE", 0, 1)) {
            STD_PRINTF("M5:T435:S9:I186-REP-COMBINATION:FAIL:%s:ODD\n",
                repeat_recipes[index].key_id);
            return 1;
        }
        if (timing_80186_manifest_run_repeat_phase_context(&repeat_recipes[index],
                "SEGMENT-ODD-WORD-REP-PHASE", 1, 1)) {
            STD_PRINTF("M5:T435:S9:I186-REP-COMBINATION:FAIL:%s:SEGMENT-ODD\n",
                repeat_recipes[index].key_id);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(repeat_recipes) / sizeof(repeat_recipes[0]);
            ++index) {
        if ((repeat_recipes[index].opcode & 1u) == 0u) continue;
        if (timing_80186_manifest_run_repeat_odd_recipe(&repeat_recipes[index]) ||
            timing_80186_manifest_run_repeat_segment_odd_recipe(
                &repeat_recipes[index]))
            return 1;
    }
    for (index = 0u; index < sizeof(lock_recipes) / sizeof(lock_recipes[0]);
            ++index) {
        if (timing_80186_manifest_run_recipe(&lock_recipes[index])) {
            timing_80186_manifest_report_failure(&lock_recipes[index]);
            return 1;
        }
    }
    for (index = 0u; index < sizeof(lock_recipes) / sizeof(lock_recipes[0]);
            ++index) {
        timing_80186_manifest_recipe recipe = lock_recipes[index];
        C_CHAR key_id[96];
        STD_SIZE_T offset;

        if (recipe.bytes >= sizeof(recipe.program) || STD_SNPRINTF(key_id,
                sizeof(key_id), "%s-SEGMENT", recipe.key_id) < 0) return 1;
        for (offset = recipe.bytes; offset > 0u; --offset) {
            recipe.program[offset] = recipe.program[offset - 1u];
        }
        recipe.program[0] = 0x26u;
        ++recipe.bytes;
        recipe.ticks += 2u;
        recipe.key_id = key_id;
        if (timing_80186_manifest_run_recipe(&recipe)) {
            timing_80186_manifest_report_failure(&recipe);
            return 1;
        }
    }
    if (observed != sizeof(recipes) / sizeof(recipes[0]) +
            sizeof(branch_recipes) / sizeof(branch_recipes[0]) +
            sizeof(repeat_recipes) / sizeof(repeat_recipes[0])) return 1;
    for (index = 0u; index < sizeof(timing_80186_manifest_records) /
            sizeof(timing_80186_manifest_records[0]); ++index) {
        const timing_80186_manifest_record *record =
            &timing_80186_manifest_records[index];

        if (!timing_80186_manifest_is_i186(record) ||
            STD_STRCMP(record->context, "BASE") != 0) continue;
        if (!timing_80186_manifest_observed[index]) return 1;
        ++base_records;
    }
    for (index = 0u; index < sizeof(timing_80186_manifest_records) /
            sizeof(timing_80186_manifest_records[0]); ++index) {
        const timing_80186_manifest_record *record =
            &timing_80186_manifest_records[index];

        if (!timing_80186_manifest_is_i186(record) ||
            !timing_80186_manifest_is_repeat_phase_context(record)) continue;
        if (!timing_80186_manifest_observed[index]) return 1;
        ++repeat_phase_records;
    }
    for (index = 0u; index < sizeof(timing_80186_manifest_records) /
            sizeof(timing_80186_manifest_records[0]); ++index) {
        const timing_80186_manifest_record *record =
            &timing_80186_manifest_records[index];

        if (!timing_80186_manifest_is_i186(record) ||
            STD_STRCMP(record->context, "LOCK") != 0) continue;
        if (!timing_80186_manifest_observed[index]) return 1;
        ++lock_records;
    }
    for (index = 0u; index < sizeof(timing_80186_manifest_records) /
            sizeof(timing_80186_manifest_records[0]); ++index) {
        const timing_80186_manifest_record *record =
            &timing_80186_manifest_records[index];

        if (!timing_80186_manifest_is_i186(record) ||
            STD_STRCMP(record->context, "LOCK-SEGMENT") != 0) continue;
        if (!timing_80186_manifest_observed[index]) return 1;
        ++lock_segment_records;
    }
    for (index = 0u; index < sizeof(timing_80186_manifest_records) /
            sizeof(timing_80186_manifest_records[0]); ++index) {
        const timing_80186_manifest_record *record =
            &timing_80186_manifest_records[index];

        if (!timing_80186_manifest_is_i186(record) ||
            STD_STRCMP(record->context, "ODD-WORD") != 0) continue;
        if (timing_80186_manifest_observed[index]) ++odd_word_records;
        else STD_PRINTF("M5:T435:S9:I186-MANIFEST-ODD-WORD-MISSING:%s\n",
            record->key_id);
    }
    for (index = 0u; index < sizeof(timing_80186_manifest_records) /
            sizeof(timing_80186_manifest_records[0]); ++index) {
        const timing_80186_manifest_record *record =
            &timing_80186_manifest_records[index];

        if (!timing_80186_manifest_is_i186(record) ||
            STD_STRCMP(record->context, "SEGMENT") != 0) continue;
        if (timing_80186_manifest_observed[index]) ++segment_records;
        else STD_PRINTF("M5:T435:S9:I186-MANIFEST-SEGMENT-MISSING:%s\n",
            record->key_id);
    }
    for (index = 0u; index < sizeof(timing_80186_manifest_records) /
            sizeof(timing_80186_manifest_records[0]); ++index) {
        const timing_80186_manifest_record *record =
            &timing_80186_manifest_records[index];
        C_INT repeat_combination = STD_STRCMP(record->context,
            "SEGMENT-REP-PHASE-FIRST") == 0 || STD_STRCMP(record->context,
            "SEGMENT-REP-PHASE-CONTINUE") == 0 || STD_STRCMP(record->context,
            "SEGMENT-REP-PHASE-ZERO") == 0 || STD_STRCMP(record->context,
            "ODD-WORD-REP-PHASE-FIRST") == 0 || STD_STRCMP(record->context,
            "ODD-WORD-REP-PHASE-CONTINUE") == 0 || STD_STRCMP(record->context,
            "ODD-WORD-REP-PHASE-ZERO") == 0 || STD_STRCMP(record->context,
            "SEGMENT-ODD-WORD-REP-PHASE-FIRST") == 0 || STD_STRCMP(record->context,
            "SEGMENT-ODD-WORD-REP-PHASE-CONTINUE") == 0 || STD_STRCMP(record->context,
            "SEGMENT-ODD-WORD-REP-PHASE-ZERO") == 0;

        if (!timing_80186_manifest_is_i186(record) || !repeat_combination) continue;
        if (!timing_80186_manifest_observed[index]) return 1;
        ++repeat_combination_records;
    }
    for (index = 0u; index < sizeof(timing_80186_manifest_records) /
            sizeof(timing_80186_manifest_records[0]); ++index) {
        const timing_80186_manifest_record *record =
            &timing_80186_manifest_records[index];
        C_INT combined = STD_STRCMP(record->context, "LOCK-SEGMENT") == 0 ||
            STD_STRCMP(record->context, "SEGMENT-ODD-WORD") == 0 ||
            STD_STRCMP(record->context, "SEGMENT-REP-PHASE-FIRST") == 0 ||
            STD_STRCMP(record->context, "SEGMENT-REP-PHASE-CONTINUE") == 0 ||
            STD_STRCMP(record->context, "SEGMENT-REP-PHASE-ZERO") == 0 ||
            STD_STRCMP(record->context, "ODD-WORD-REP-PHASE-FIRST") == 0 ||
            STD_STRCMP(record->context, "ODD-WORD-REP-PHASE-CONTINUE") == 0 ||
            STD_STRCMP(record->context, "ODD-WORD-REP-PHASE-ZERO") == 0 ||
            STD_STRCMP(record->context,
                "SEGMENT-ODD-WORD-REP-PHASE-FIRST") == 0 ||
            STD_STRCMP(record->context,
                "SEGMENT-ODD-WORD-REP-PHASE-CONTINUE") == 0 ||
            STD_STRCMP(record->context,
                "SEGMENT-ODD-WORD-REP-PHASE-ZERO") == 0;

        if (!timing_80186_manifest_is_i186(record) || !combined) continue;
        if (!timing_80186_manifest_observed[index]) return 1;
        ++combined_records;
    }
    if (base_records != observed) return 1;
    if (repeat_phase_records != 54u) return 1;
    if (lock_records != 19u) return 1;
    if (lock_segment_records != 19u) return 1;
    STD_PRINTF("M5:T435:S9:I186-MANIFEST-BASE-COVERAGE:%u\n",
        (type_unsigned_32)base_records);
    STD_PRINTF("M5:T435:S9:I186-MANIFEST-REP-PHASE-COVERAGE:%u\n",
        (type_unsigned_32)repeat_phase_records);
    STD_PRINTF("M5:T435:S9:I186-MANIFEST-LOCK-COVERAGE:%u\n",
        (type_unsigned_32)lock_records);
    STD_PRINTF("M5:T435:S9:I186-MANIFEST-LOCK-SEGMENT-COVERAGE:%u\n",
        (type_unsigned_32)lock_segment_records);
    STD_PRINTF("M5:T435:S9:I186-MANIFEST-ODD-WORD-COVERAGE:%u\n",
        (type_unsigned_32)odd_word_records);
    if (odd_word_records != 87u) return 1;
    STD_PRINTF("M5:T435:S9:I186-MANIFEST-SEGMENT-COVERAGE:%u\n",
        (type_unsigned_32)segment_records);
    if (segment_records != 88u) return 1;
    STD_PRINTF("M5:T435:S9:I186-MANIFEST-REP-COMBINATION-COVERAGE:%u\n",
        (type_unsigned_32)repeat_combination_records);
    if (repeat_combination_records != 63u) return 1;
    STD_PRINTF("M5:T435:S9:I186-MANIFEST-COMBINATION-COVERAGE:%u\n",
        (type_unsigned_32)combined_records);
    if (combined_records != 89u) return 1;
    if (timing_80186_manifest_write_results()) return 1;
    STD_PRINTF("M5:T435:S9:I186-MANIFEST-OBSERVED:%u\n",
        (type_unsigned_32)observed);
    return 0;
}
