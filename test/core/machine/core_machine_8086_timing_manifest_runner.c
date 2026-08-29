#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/cpu_timing.h"
#include "core/machine/retirement_observation_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define TIMING_MANIFEST_RESET_LINEAR 0x000ffff0u
#define TIMING_MANIFEST_RESET_PHYSICAL 0x000ffff0u
#define TIMING_MANIFEST_WINDOW_BYTES 16u

/* This runner is deliberately not a passing CTest target until every I86 key
 * has a real recipe.  Its generated metadata prevents handwritten provenance
 * from drifting from the T435 S2 manifest. */
typedef struct timing_manifest_record {
    const C_CHAR *key_id;
    const C_CHAR *profile;
    const C_CHAR *level;
    const C_CHAR *source_rule;
    const C_CHAR *context;
} timing_manifest_record;

typedef struct timing_manifest_capture {
    core_machine_retirement_observation observation;
    type_unsigned_32 count;
} timing_manifest_capture;

typedef struct timing_manifest_recipe {
    const C_CHAR *key_id;
    type_unsigned_8 program[8];
    type_unsigned_8 program_bytes;
    type_unsigned_64 expected_ticks;
    core_machine_retirement_timing_origin expected_origin;
} timing_manifest_recipe;

static C_INT timing_manifest_text_contains(const C_CHAR *text,
    const C_CHAR *needle);

static C_VOID timing_manifest_execution_reset(C_VOID *opaque)
{
    (C_VOID)opaque;
}

static const core_machine_execution_provider timing_manifest_execution_provider = {
    timing_manifest_execution_reset, STD_NULL
};

static type_status timing_manifest_port_read(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 *out_value)
{
    (C_VOID)owner;
    if (out_value == STD_NULL || port != 0x0080u) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_value = 0x5au;
    return TYPE_STATUS_OK;
}

static type_status timing_manifest_port_write(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 value)
{
    (C_VOID)owner;
    return port == 0x0080u && value <= 0xffffu ? TYPE_STATUS_OK :
        TYPE_STATUS_INVALID_ARGUMENT;
}

static const core_machine_port_provider timing_manifest_port_provider = {
    timing_manifest_port_read, timing_manifest_port_write
};

static const timing_manifest_record timing_manifest_records[] = {
#include "cpu_timing_manifest_metadata_catalog.inc"
};
static C_INT timing_manifest_covered[sizeof(timing_manifest_records) /
    sizeof(timing_manifest_records[0])];
static core_machine_retirement_observation timing_manifest_results[
    sizeof(timing_manifest_records) / sizeof(timing_manifest_records[0])];
static C_INT timing_manifest_observed[sizeof(timing_manifest_records) /
    sizeof(timing_manifest_records[0])];
static C_INT timing_manifest_current_index = -1;

static C_INT timing_manifest_is_i86(const timing_manifest_record *record)
{
    return record != STD_NULL && record->key_id[0] == 'I' &&
        record->key_id[1] == '8' && record->key_id[2] == '6' &&
        record->key_id[3] == '-';
}

static const timing_manifest_record *timing_manifest_find(const C_CHAR *key_id)
{
    STD_SIZE_T index;

    timing_manifest_current_index = -1;
    if (key_id == STD_NULL) return STD_NULL;
    for (index = 0u; index < sizeof(timing_manifest_records) /
            sizeof(timing_manifest_records[0]); ++index) {
        const timing_manifest_record *record = &timing_manifest_records[index];

        if (STD_STRCMP(record->key_id, key_id) == 0) {
            timing_manifest_covered[index] = 1;
            timing_manifest_current_index = (C_INT)index;
            return record;
        }
    }
    return STD_NULL;
}

static C_VOID timing_manifest_capture_retirement(C_VOID *opaque,
    const core_machine_retirement_observation *observation)
{
    timing_manifest_capture *capture = (timing_manifest_capture *)opaque;

    if (capture == STD_NULL || observation == STD_NULL) return;
    if (capture->count == 0u) capture->observation = *observation;
    if (timing_manifest_current_index >= 0 &&
            (!timing_manifest_observed[timing_manifest_current_index] ||
             (observation->formula_inputs & timing_manifest_results[
                 timing_manifest_current_index].formula_inputs) ==
                 timing_manifest_results[timing_manifest_current_index].formula_inputs)) {
        timing_manifest_results[timing_manifest_current_index] = *observation;
        timing_manifest_observed[timing_manifest_current_index] = 1;
    }
    /* A record selection applies to exactly one real retirement.  Leaving it
     * live let later semantic-only probes overwrite an unrelated result row. */
    timing_manifest_current_index = -1;
    ++capture->count;
}

static C_INT timing_manifest_prepare(core_machine **out_machine,
    timing_manifest_capture *capture, const type_unsigned_8 *program,
    STD_SIZE_T program_bytes)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .ticks_per_instruction = 29u,
        .instruction_timing = { 29u, 7u, 31u, 37u, 41u, 43u }
    };
    const core_machine_retirement_observation_provider provider = {
        timing_manifest_capture_retirement, capture
    };
    core_machine *machine = STD_NULL;
    type_status status = TYPE_STATUS_OK;

    if (out_machine == STD_NULL || capture == STD_NULL || program == STD_NULL) return 0;
    status = core_machine_create(&config, &machine);
    if (status == TYPE_STATUS_OK) {
        status = test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_MANIFEST_RESET_LINEAR, TIMING_MANIFEST_RESET_PHYSICAL,
            TIMING_MANIFEST_WINDOW_BYTES);
    }
    if (status == TYPE_STATUS_OK) {
        status = core_machine_install_port_provider(machine, 0x0080u, 0x0080u,
            &timing_manifest_port_provider, STD_NULL);
    }
    if (status == TYPE_STATUS_OK) {
        status = core_machine_bind_execution_provider(machine,
            &timing_manifest_execution_provider, STD_NULL);
    }
    if (status == TYPE_STATUS_OK) {
        status = core_machine_freeze_execution_providers(machine);
    }
    if (status == TYPE_STATUS_OK) {
        status = core_machine_reset(machine);
    }
    if (status == TYPE_STATUS_OK) {
        status = core_machine_set_a20(machine, 1);
    }
    if (status == TYPE_STATUS_OK) {
        status = core_machine_memory_write(machine, TIMING_MANIFEST_RESET_LINEAR,
            program, program_bytes);
    }
    if (status == TYPE_STATUS_OK) {
        status = core_machine_set_retirement_observation_provider(machine, &provider);
    }
    if (status != TYPE_STATUS_OK) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

/* Each S1 base instruction gains one 8086 LOCK context.  Callers invoke this
 * after proving their unprefixed recipe so the companion is a second real
 * retirement, never a synthesized result. */
static C_INT timing_manifest_run_lock_companion(
    const timing_manifest_record *base_record, const C_CHAR *base_key,
    const type_unsigned_8 *program, type_unsigned_8 program_bytes,
    type_unsigned_64 expected_ticks,
    core_machine_retirement_timing_origin expected_origin,
    type_unsigned_32 initial_eflags, type_unsigned_16 initial_cx,
    type_unsigned_16 initial_dx, type_unsigned_32 required_formula_inputs,
    core_machine_retirement_control_outcome expected_control_outcome)
{
    const core_machine_run_budget budget = { 1u, 0u };
    C_CHAR key[160];
    type_unsigned_8 locked_program[9];
    timing_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    const timing_manifest_record *record;
    core_machine *machine = STD_NULL;
    C_INT failed;

    if (base_record == STD_NULL || base_key == STD_NULL || program == STD_NULL ||
        STD_STRCMP(base_record->context, "BASE") != 0) return 0;
    if (program_bytes == 0u || program_bytes >= sizeof(locked_program) ||
        STD_SNPRINTF(key, sizeof(key), "%s-LOCK", base_key) < 0) return 1;
    locked_program[0] = 0xf0u;
    STD_MEMCPY(locked_program + 1u, program, program_bytes);
    record = timing_manifest_find(key);
    if (record == STD_NULL) return 0;
    failed = !timing_manifest_prepare(&machine, &capture, locked_program,
        program_bytes + 1u);
    if (!failed) {
        machine->executor_cpu.data.eflags = initial_eflags;
        machine->executor_cpu.data.cx = initial_cx;
        machine->executor_cpu.data.dx = initial_dx;
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != expected_ticks + 2u || capture.count != 1u ||
            capture.observation.source_ticks != expected_ticks + 2u ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            capture.observation.timing_origin != expected_origin ||
            (capture.observation.formula_inputs &
                (required_formula_inputs | CORE_MACHINE_CPU_TIMING_INPUT_LOCK)) !=
                (required_formula_inputs | CORE_MACHINE_CPU_TIMING_INPUT_LOCK) ||
            capture.observation.control_outcome != expected_control_outcome;
    }
    core_machine_destroy(machine);
    if (failed) {
        STD_PRINTF("I86 LOCK result ticks=%llu source=%llu origin=%d inputs=%u control=%d count=%u\n",
            run.ticks, capture.observation.source_ticks,
            capture.observation.timing_origin, capture.observation.formula_inputs,
            capture.observation.control_outcome, capture.count);
        STD_PRINTF("M5:T435:S5:I86-LOCK-COMPANION:FAIL:%s\n", key);
    }
    return failed;
}

static C_INT timing_manifest_run_exact_recipe_with_inputs_and_formula(
    const timing_manifest_recipe *recipe, type_unsigned_32 initial_eflags,
    type_unsigned_16 initial_cx, type_unsigned_16 initial_dx,
    type_unsigned_32 required_formula_inputs,
    core_machine_retirement_control_outcome expected_control_outcome)
{
    const core_machine_run_budget budget = { 1u, 0u };
    const timing_manifest_record *record = recipe == STD_NULL ? STD_NULL :
        timing_manifest_find(recipe->key_id);
    core_machine_run_result run;
    timing_manifest_capture capture = { { 0 }, 0u };
    core_machine *machine = STD_NULL;
    C_INT prepared = recipe != STD_NULL && timing_manifest_prepare(&machine,
        &capture, recipe->program, recipe->program_bytes);
    C_INT failed = record == STD_NULL || !timing_manifest_is_i86(record) ||
        STD_STRCMP(record->profile, "8086") != 0 ||
        STD_STRCMP(record->level, "L3") != 0 ||
        record->source_rule[0] == '\0' ||
        !prepared;

    if (!failed) {
        machine->executor_cpu.data.eflags = initial_eflags;
        machine->executor_cpu.data.cx = initial_cx;
        machine->executor_cpu.data.dx = initial_dx;
        type_status status = core_machine_run(machine, budget, &run);

        failed |= status != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != recipe->expected_ticks || capture.count != 1u ||
            capture.observation.cpu_profile != CORE_MACHINE_CPU_PROFILE_8086 ||
            capture.observation.source_ticks != recipe->expected_ticks ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            capture.observation.timing_origin != recipe->expected_origin ||
            capture.observation.source_timing_form_id ==
                CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
            capture.observation.timing_key_id ==
                CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
            (required_formula_inputs != 0u &&
                (capture.observation.formula_inputs &
                    required_formula_inputs) != required_formula_inputs) ||
            capture.observation.control_outcome !=
                expected_control_outcome;
    }
    if (failed) {
        STD_PRINTF("I86 result ticks=%llu source=%llu origin=%d form=%u key=%u count=%u control=%d disposition=%d\n",
            run.ticks, capture.observation.source_ticks,
            capture.observation.timing_origin,
            capture.observation.source_timing_form_id,
            capture.observation.timing_key_id, capture.count,
            capture.observation.control_outcome,
            capture.observation.timing_disposition);
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-RECIPE:FAIL:%s\n", recipe->key_id);
    }
    if (!failed && timing_manifest_run_lock_companion(record, recipe->key_id,
            recipe->program, recipe->program_bytes, recipe->expected_ticks,
            recipe->expected_origin, initial_eflags, initial_cx, initial_dx,
            required_formula_inputs, expected_control_outcome)) {
        failed = 1;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_manifest_run_exact_recipe_with_inputs(
    const timing_manifest_recipe *recipe, type_unsigned_32 initial_eflags,
    type_unsigned_16 initial_cx, type_unsigned_16 initial_dx,
    core_machine_retirement_control_outcome expected_control_outcome)
{
    return timing_manifest_run_exact_recipe_with_inputs_and_formula(recipe,
        initial_eflags, initial_cx, initial_dx, 0u, expected_control_outcome);
}

static C_INT timing_manifest_run_exact_recipe_with_control(
    const timing_manifest_recipe *recipe, type_unsigned_32 initial_eflags,
    type_unsigned_16 initial_cx,
    core_machine_retirement_control_outcome expected_control_outcome)
{
    return timing_manifest_run_exact_recipe_with_inputs(recipe, initial_eflags,
        initial_cx, 0u, expected_control_outcome);
}

static C_INT timing_manifest_run_exact_recipe(
    const timing_manifest_recipe *recipe)
{
    return timing_manifest_run_exact_recipe_with_control(recipe, 0u, 0u,
        CORE_MACHINE_RETIREMENT_CONTROL_NONE);
}

typedef struct timing_manifest_memory_recipe {
    const C_CHAR *key_id;
    type_unsigned_8 program[9];
    type_unsigned_8 program_bytes;
    type_unsigned_64 expected_ticks;
    core_machine_retirement_timing_origin expected_origin;
    type_unsigned_16 initial_ax;
    type_unsigned_16 initial_cx;
    type_unsigned_16 memory_value;
    type_unsigned_16 expected_ax;
    type_unsigned_16 expected_cx;
    type_unsigned_16 expected_memory_value;
} timing_manifest_memory_recipe;

static C_INT timing_manifest_lock_key_for_context(const timing_manifest_record *record,
    const C_CHAR *base_key, C_CHAR *out_key, STD_SIZE_T out_size)
{
    static const C_CHAR segment[] = "-SEGMENT";
    static const C_CHAR odd[] = "-ODD-WORD";
    static const C_CHAR segment_odd[] = "-SEGMENT-ODD-WORD";
    STD_SIZE_T base_length;

    if (record == STD_NULL || base_key == STD_NULL || out_key == STD_NULL ||
        timing_manifest_text_contains(record->context, "LOCK")) return 0;
    if (STD_STRCMP(record->context, "BASE") == 0) {
        return STD_SNPRINTF(out_key, out_size, "%s-LOCK", base_key) >= 0;
    }
    base_length = STD_STRLEN(base_key);
    if (STD_STRCMP(record->context, "SEGMENT") == 0 &&
        base_length > STD_STRLEN(segment)) {
        return STD_SNPRINTF(out_key, out_size, "%.*s-LOCK-SEGMENT",
            (C_INT)(base_length - STD_STRLEN(segment)), base_key) >= 0;
    }
    if (STD_STRCMP(record->context, "ODD-WORD") == 0 &&
        base_length > STD_STRLEN(odd)) {
        return STD_SNPRINTF(out_key, out_size, "%.*s-LOCK-ODD-WORD",
            (C_INT)(base_length - STD_STRLEN(odd)), base_key) >= 0;
    }
    if (STD_STRCMP(record->context, "SEGMENT-ODD-WORD") == 0 &&
        base_length > STD_STRLEN(segment_odd)) {
        return STD_SNPRINTF(out_key, out_size, "%.*s-LOCK-SEGMENT-ODD-WORD",
            (C_INT)(base_length - STD_STRLEN(segment_odd)), base_key) >= 0;
    }
    return 0;
}

static C_INT timing_manifest_run_l3_memory_recipe_with_inputs_internal(
    const timing_manifest_memory_recipe *recipe, type_unsigned_32 extra_required_inputs,
    C_INT run_lock_companion)
{
    const core_machine_run_budget budget = { 1u, 0u };
    const timing_manifest_record *record = recipe == STD_NULL ? STD_NULL :
        timing_manifest_find(recipe->key_id);
    timing_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    type_unsigned_16 memory_value = 0u;
    type_unsigned_32 required_inputs = CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS;
    type_unsigned_8 opcode = recipe == STD_NULL ? 0u : recipe->program[0];
    type_unsigned_8 opcode_index = 0u;
    type_unsigned_32 memory_linear;
    C_INT failed = record == STD_NULL || !timing_manifest_is_i86(record) ||
        STD_STRCMP(record->profile, "8086") != 0 ||
        STD_STRCMP(record->level, "L3") != 0 || record->source_rule[0] == '\0' ||
        !timing_manifest_prepare(&machine, &capture, recipe->program,
            recipe->program_bytes);

    while (recipe != STD_NULL && opcode_index < recipe->program_bytes) {
        opcode = recipe->program[opcode_index];
        if (opcode == 0x26u || opcode == 0x2eu || opcode == 0x36u ||
            opcode == 0x3eu) {
            required_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE;
        } else if (opcode == 0xf0u) {
            required_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_LOCK;
        } else {
            break;
        }
        ++opcode_index;
    }
    if (recipe != STD_NULL && opcode != 0xa0u && opcode != 0xa1u &&
        opcode != 0xa2u && opcode != 0xa3u) {
        required_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_MODRM;
    }
    memory_linear = (opcode == 0xa0u || opcode == 0xa1u || opcode == 0xa2u ||
        opcode == 0xa3u) ? (type_unsigned_32)recipe->program[opcode_index + 1u] |
            ((type_unsigned_32)recipe->program[opcode_index + 2u] << 8u) :
        (type_unsigned_32)recipe->program[opcode_index + 2u] |
            ((type_unsigned_32)recipe->program[opcode_index + 3u] << 8u);
    if (!failed) {
        machine->executor_cpu.data.es.base = machine->executor_cpu.data.ds.base;
        machine->executor_cpu.data.es.selector = machine->executor_cpu.data.ds.selector;
        failed = core_machine_memory_write(machine, memory_linear, &recipe->memory_value,
            sizeof(recipe->memory_value)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        machine->executor_cpu.data.ax = recipe->initial_ax;
        machine->executor_cpu.data.cx = recipe->initial_cx;
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != recipe->expected_ticks || capture.count != 1u ||
            capture.observation.source_ticks != recipe->expected_ticks ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            capture.observation.timing_origin != recipe->expected_origin ||
            (capture.observation.formula_inputs &
                (required_inputs | extra_required_inputs)) !=
                (required_inputs | extra_required_inputs) ||
            machine->executor_cpu.data.ax != recipe->expected_ax ||
            machine->executor_cpu.data.cx != recipe->expected_cx ||
            core_machine_memory_read(machine, memory_linear, &memory_value,
                sizeof(memory_value)) != TYPE_STATUS_OK ||
            memory_value != recipe->expected_memory_value;
    }
    if (failed) {
        STD_PRINTF("I86 memory ticks=%llu source=%llu origin=%d inputs=%u ax=%u cx=%u mem=%u count=%u\n",
            run.ticks, capture.observation.source_ticks,
            capture.observation.timing_origin, capture.observation.formula_inputs,
            machine == STD_NULL ? 0u : machine->executor_cpu.data.ax,
            machine == STD_NULL ? 0u : machine->executor_cpu.data.cx,
            memory_value, capture.count);
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-RECIPE:FAIL:%s\n", recipe->key_id);
    }
    core_machine_destroy(machine);
    if (!failed && run_lock_companion && record != STD_NULL &&
        !timing_manifest_text_contains(record->context, "LOCK")) {
        timing_manifest_memory_recipe locked = *recipe;
        C_CHAR key[160];
        STD_SIZE_T index;

        if (locked.program_bytes >= sizeof(locked.program) ||
            !timing_manifest_lock_key_for_context(record, recipe->key_id, key,
                sizeof(key))) {
            return 1;
        }
        for (index = locked.program_bytes; index != 0u; --index) {
            locked.program[index] = locked.program[index - 1u];
        }
        locked.program[0] = 0xf0u;
        ++locked.program_bytes;
        locked.expected_ticks += 2u;
        locked.key_id = key;
        return timing_manifest_run_l3_memory_recipe_with_inputs_internal(&locked,
            extra_required_inputs, 0);
    }
    return failed;
}

static C_INT timing_manifest_run_l3_memory_recipe_with_inputs(
    const timing_manifest_memory_recipe *recipe, type_unsigned_32 extra_required_inputs)
{
    return timing_manifest_run_l3_memory_recipe_with_inputs_internal(recipe,
        extra_required_inputs, 1);
}

static C_INT timing_manifest_run_l3_memory_recipe(
    const timing_manifest_memory_recipe *recipe)
{
    return timing_manifest_run_l3_memory_recipe_with_inputs(recipe, 0u);
}

static C_INT timing_manifest_run_lock_memory_context(
    const timing_manifest_memory_recipe *base_recipe, const C_CHAR *key_id,
    C_INT segment_override, C_INT odd_word)
{
    timing_manifest_memory_recipe recipe = *base_recipe;
    STD_SIZE_T index;
    STD_SIZE_T address_index;

    recipe.key_id = key_id;
    if (segment_override) {
        for (index = recipe.program_bytes; index > 1u; --index) {
            recipe.program[index] = recipe.program[index - 1u];
        }
        recipe.program[1] = 0x26u;
        ++recipe.program_bytes;
        recipe.expected_ticks += 2u;
    }
    if (odd_word) {
        address_index = segment_override ? 4u : 3u;
        recipe.program[address_index] = 1u;
        recipe.expected_ticks += 8u;
    }
    return timing_manifest_run_l3_memory_recipe_with_inputs(&recipe,
        odd_word ? CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD : 0u);
}

typedef struct timing_manifest_string_recipe {
    const C_CHAR *key_id;
    type_unsigned_8 opcode;
    type_unsigned_64 expected_ticks;
} timing_manifest_string_recipe;

static C_INT timing_manifest_run_string_primitive_with_prefix_internal(
    const timing_manifest_string_recipe *recipe, type_unsigned_8 prefix,
    C_INT odd_addresses, type_unsigned_32 extra_required_inputs,
    C_INT run_lock_companion, C_INT lock_prefix)
{
    const core_machine_run_budget budget = { 1u, 0u };
    const timing_manifest_record *record = recipe == STD_NULL ? STD_NULL :
        timing_manifest_find(recipe->key_id);
    const type_unsigned_16 source_word = 0x5aa5u;
    type_unsigned_16 destination_word = 0u;
    timing_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    type_unsigned_8 program[3];
    type_unsigned_8 program_bytes = 0u;
    const type_unsigned_32 source_linear = odd_addresses ? 0x1001u : 0x1000u;
    const type_unsigned_32 destination_linear = odd_addresses ? 0x1101u : 0x1100u;
    C_INT word = recipe != STD_NULL && (recipe->opcode & 1u) != 0u;
    if (lock_prefix) program[program_bytes++] = 0xf0u;
    if (prefix != 0u) program[program_bytes++] = prefix;
    program[program_bytes++] = recipe == STD_NULL ? 0u : recipe->opcode;
    C_INT failed = record == STD_NULL || !timing_manifest_is_i86(record) ||
        STD_STRCMP(record->profile, "8086") != 0 ||
        STD_STRCMP(record->level, "L3") != 0 || record->source_rule[0] == '\0' ||
        !timing_manifest_prepare(&machine, &capture, program, program_bytes);

    if (!failed) {
        machine->executor_cpu.data.es.base = machine->executor_cpu.data.ds.base;
        machine->executor_cpu.data.es.selector = machine->executor_cpu.data.ds.selector;
        failed = core_machine_memory_write(machine, source_linear, &source_word,
            sizeof(source_word)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        machine->executor_cpu.data.si = (type_unsigned_16)source_linear;
        machine->executor_cpu.data.di = (type_unsigned_16)destination_linear;
        machine->executor_cpu.data.ax = 0x1234u;
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != recipe->expected_ticks || capture.count != 1u ||
            capture.observation.source_ticks != recipe->expected_ticks ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO ||
            capture.observation.repeat_phase !=
                CORE_MACHINE_RETIREMENT_REPEAT_PRIMITIVE ||
            (capture.observation.formula_inputs &
                CORE_MACHINE_CPU_TIMING_INPUT_REPEAT) != 0u;
        failed |= (capture.observation.formula_inputs & extra_required_inputs) !=
            extra_required_inputs;
        if (!failed && (recipe->opcode == 0xa4u || recipe->opcode == 0xa5u ||
            recipe->opcode == 0xaau || recipe->opcode == 0xabu)) {
            failed = core_machine_memory_read(machine, destination_linear,
                &destination_word,
                sizeof(destination_word)) != TYPE_STATUS_OK ||
                destination_word != (recipe->opcode == 0xa4u ?
                    (source_word & 0x00ffu) : recipe->opcode == 0xa5u ?
                    source_word : recipe->opcode == 0xaau ? 0x0034u : 0x1234u);
        }
        if (!failed && (recipe->opcode == 0xacu || recipe->opcode == 0xadu)) {
            failed = machine->executor_cpu.data.ax != (word ? source_word :
                (type_unsigned_16)(0x1200u | (source_word & 0x00ffu)));
        }
        if (!failed) {
            const type_unsigned_16 step = word ? 2u : 1u;
            const type_unsigned_16 expected_si = recipe->opcode == 0xaau ||
                recipe->opcode == 0xabu || recipe->opcode == 0xae ||
                recipe->opcode == 0xafu ? (type_unsigned_16)source_linear :
                (type_unsigned_16)(source_linear + step);
            const type_unsigned_16 expected_di = recipe->opcode == 0xacu ||
                recipe->opcode == 0xadu ? (type_unsigned_16)destination_linear :
                (type_unsigned_16)(destination_linear + step);
            failed = machine->executor_cpu.data.si != expected_si ||
                machine->executor_cpu.data.di != expected_di;
        }
    }
    if (failed) {
        STD_PRINTF("I86 string ticks=%llu source=%llu inputs=%u phase=%d ax=%u si=%u di=%u\n",
            run.ticks, capture.observation.source_ticks,
            capture.observation.formula_inputs, capture.observation.repeat_phase,
            machine == STD_NULL ? 0u : machine->executor_cpu.data.ax,
            machine == STD_NULL ? 0u : machine->executor_cpu.data.si,
            machine == STD_NULL ? 0u : machine->executor_cpu.data.di);
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-RECIPE:FAIL:%s\n", recipe->key_id);
    }
    core_machine_destroy(machine);
    if (!failed && run_lock_companion && record != STD_NULL &&
        !timing_manifest_text_contains(record->context, "LOCK")) {
        timing_manifest_string_recipe locked = *recipe;
        C_CHAR key[160];

        if (!timing_manifest_lock_key_for_context(record, recipe->key_id, key,
                sizeof(key))) {
            return 1;
        }
        locked.key_id = key;
        locked.expected_ticks += 2u;
        return timing_manifest_run_string_primitive_with_prefix_internal(&locked,
            prefix, odd_addresses,
            extra_required_inputs | CORE_MACHINE_CPU_TIMING_INPUT_LOCK, 0, 1);
    }
    return failed;
}

static C_INT timing_manifest_run_string_primitive_with_prefix(
    const timing_manifest_string_recipe *recipe, type_unsigned_8 prefix,
    C_INT odd_addresses, type_unsigned_32 extra_required_inputs)
{
    return timing_manifest_run_string_primitive_with_prefix_internal(recipe, prefix,
        odd_addresses, extra_required_inputs, 1, 0);
}

static C_INT timing_manifest_run_string_primitive(
    const timing_manifest_string_recipe *recipe)
{
    return timing_manifest_run_string_primitive_with_prefix(recipe, 0u, 0,
        0u);
}

typedef struct timing_manifest_repeat_recipe {
    const C_CHAR *key_id;
    type_unsigned_8 prefix;
    type_unsigned_8 opcode;
    type_unsigned_64 first_ticks;
    type_unsigned_64 continuation_ticks;
    type_unsigned_64 zero_ticks;
    type_unsigned_8 segment_prefix;
    C_INT odd_addresses;
    type_unsigned_32 required_formula_inputs;
} timing_manifest_repeat_recipe;

static C_INT timing_manifest_run_repeat_step(core_machine *machine,
    timing_manifest_capture *capture, const timing_manifest_repeat_recipe *recipe,
    core_machine_retirement_repeat_phase expected_phase,
    type_unsigned_64 expected_ticks)
{
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result run = { 0 };
    C_INT failed;

    if (machine == STD_NULL || capture == STD_NULL || recipe == STD_NULL) return 1;
    capture->count = 0u;
    STD_MEMSET(&capture->observation, 0, sizeof(capture->observation));
    failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
        run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
        run.ticks != expected_ticks || capture->count != 1u ||
        capture->observation.source_ticks != expected_ticks ||
        capture->observation.timing_disposition !=
            CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        capture->observation.timing_origin !=
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO ||
        capture->observation.repeat_phase != expected_phase ||
        (capture->observation.formula_inputs &
            (CORE_MACHINE_CPU_TIMING_INPUT_REPEAT |
             CORE_MACHINE_CPU_TIMING_INPUT_REPEAT_PHASE)) !=
            (CORE_MACHINE_CPU_TIMING_INPUT_REPEAT |
             CORE_MACHINE_CPU_TIMING_INPUT_REPEAT_PHASE);
    if (failed) {
        STD_PRINTF("I86 repeat ticks=%llu expected=%llu phase=%d expected_phase=%d inputs=%u\n",
            run.ticks, expected_ticks, capture->observation.repeat_phase,
            expected_phase, capture->observation.formula_inputs);
    }
    return failed;
}

static C_INT timing_manifest_run_repeat_recipe(
    const timing_manifest_repeat_recipe *recipe)
{
    const timing_manifest_record *record = recipe == STD_NULL ? STD_NULL :
        timing_manifest_find(recipe->key_id);
    type_unsigned_8 program[4];
    STD_SIZE_T program_bytes;
    const type_unsigned_16 source_word = 0x5aa5u;
    type_unsigned_16 destination_word = 0u;
    timing_manifest_capture capture = { { 0 }, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed;

    if (recipe == STD_NULL) return 1;
    program_bytes = 0u;
    if (timing_manifest_text_contains(recipe->key_id, "-LOCK")) {
        program[program_bytes++] = 0xf0u;
    }
    if (recipe->segment_prefix != 0u) program[program_bytes++] = recipe->segment_prefix;
    program[program_bytes++] = recipe->prefix;
    program[program_bytes++] = recipe->opcode;
    failed = record == STD_NULL || !timing_manifest_is_i86(record) ||
        STD_STRCMP(record->profile, "8086") != 0 ||
        STD_STRCMP(record->level, "L3") != 0 || record->source_rule[0] == '\0' ||
        !timing_manifest_prepare(&machine, &capture, program, program_bytes);

    if (!failed) {
        machine->executor_cpu.data.es.base = machine->executor_cpu.data.ds.base;
        machine->executor_cpu.data.es.selector = machine->executor_cpu.data.ds.selector;
        failed = core_machine_memory_write(machine, recipe->odd_addresses ? 0x1001u :
            0x1000u, &source_word,
            sizeof(source_word)) != TYPE_STATUS_OK;
    }
    if (!failed && (recipe->opcode == 0xa6u || recipe->opcode == 0xa7u)) {
        destination_word = recipe->prefix == 0xf3u ? source_word : 0u;
        failed = core_machine_memory_write(machine, recipe->odd_addresses ? 0x1101u :
            0x1100u, &destination_word,
            sizeof(destination_word)) != TYPE_STATUS_OK;
    }
    if (!failed && (recipe->opcode == 0xaeu || recipe->opcode == 0xafu)) {
        destination_word = recipe->prefix == 0xf3u ? 0x1234u : 0u;
        failed = core_machine_memory_write(machine, recipe->odd_addresses ? 0x1101u :
            0x1100u, &destination_word,
            sizeof(destination_word)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        machine->executor_cpu.data.si = recipe->odd_addresses ? 0x1001u : 0x1000u;
        machine->executor_cpu.data.di = recipe->odd_addresses ? 0x1101u : 0x1100u;
        machine->executor_cpu.data.ax = 0x1234u;
        machine->executor_cpu.data.cx = 2u;
        failed = timing_manifest_run_repeat_step(machine, &capture, recipe,
            CORE_MACHINE_RETIREMENT_REPEAT_FIRST, recipe->first_ticks) ||
            timing_manifest_run_repeat_step(machine, &capture, recipe,
                CORE_MACHINE_RETIREMENT_REPEAT_CONTINUATION,
                recipe->continuation_ticks) ||
            (capture.observation.formula_inputs & recipe->required_formula_inputs) !=
                recipe->required_formula_inputs;
    }
    core_machine_destroy(machine);
    machine = STD_NULL;
    if (!failed) {
        capture.count = 0u;
        failed = !timing_manifest_prepare(&machine, &capture, program, program_bytes);
    }
    if (!failed) {
        machine->executor_cpu.data.cx = 0u;
        failed = timing_manifest_run_repeat_step(machine, &capture, recipe,
            CORE_MACHINE_RETIREMENT_REPEAT_ZERO_COUNT, recipe->zero_ticks);
    }
    if (failed) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-RECIPE:FAIL:%s\n", recipe->key_id);
    }
    core_machine_destroy(machine);
    return failed;
}

/* This is only a scanner smoke: an opcode without ModRM is intentionally
 * accepted once for each filler byte, so its count is not an instruction-form
 * cardinality and must never be used as a closure denominator. */
static C_INT timing_manifest_probe_decoder_lexeme_candidates(C_VOID)
{
    const C_CHAR *const path =
        "docs/etc/cpu-timing/t435-s5-8086-decoder-inventory.json";
    type_unsigned_16 opcode;
    type_unsigned_16 modrm;
    type_unsigned_32 accepted = 0u;
    type_unsigned_32 accepted_pairs;
    type_unsigned_32 accepted_opcodes = 0u;
    type_bool opcode_seen[0x100] = { TYPE_FALSE };
    STD_FILE *file;

    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        for (modrm = 0u; modrm <= 0xffu; ++modrm) {
            const type_unsigned_8 bytes[15] = {
                (type_unsigned_8)opcode, (type_unsigned_8)modrm
            };
            core_machine_cpu_instruction_lexeme lexeme;

            if (!core_machine_cpu_instruction_lexeme_scan(bytes, sizeof(bytes),
                    CORE_MACHINE_CPU_PROFILE_8086, TYPE_FALSE, &lexeme) ||
                !lexeme.available || lexeme.byte_count == 0u) continue;
            ++accepted;
            opcode_seen[opcode] = TYPE_TRUE;
        }
    }
    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        if (opcode_seen[opcode]) ++accepted_opcodes;
    }
    /* This is a decoder-boundary sentinel, not the form denominator: six
     * segment/repeat prefix bytes are accepted only before another opcode and
     * LOCK is rejected by this lexical helper because its target legality is
     * semantic.  The form contract must refine these 233 opcode candidates. */
    if (accepted == 0u || accepted_opcodes != 233u) return 1;
    accepted_pairs = accepted;
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL || STD_FPRINTF(file,
            "{\n  \"schema\": \"nxvm.8086-decoder-inventory.v1\",\n"
            "  \"lexeme_opcode_modrm_candidates\": %u,\n"
            "  \"lexeme_primary_opcodes\": [", accepted_pairs) < 0) {
        if (file != STD_NULL) STD_FCLOSE(file);
        return 1;
    }
    accepted = 0u;
    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        if (!opcode_seen[opcode]) continue;
        if ((accepted != 0u && STD_FPRINTF(file, ",") < 0) ||
                STD_FPRINTF(file, "\"%02X\"", opcode) < 0) {
            STD_FCLOSE(file);
            return 1;
        }
        ++accepted;
    }
    if (STD_FPRINTF(file, "],\n  \"semantic_only_prefixes\": "
            "[\"F0\"]\n}\n") < 0 || STD_FCLOSE(file) != 0) return 1;
    STD_PRINTF("M5:T435:S5:I86-DECODER-LEXEME-CANDIDATES:%u:%u\n", accepted,
        accepted_opcodes);
    return 0;
}

static C_INT timing_manifest_probe_decoder_form_rejections(C_VOID)
{
    static const type_unsigned_8 invalid_forms[][2] = {
        { 0xd0u, 0xf0u }, { 0x8cu, 0xe0u }, { 0x8eu, 0xc8u }
    };
    static const type_unsigned_8 valid_forms[][2] = {
        { 0xd0u, 0xd0u }, { 0x8cu, 0xd8u }, { 0x8eu, 0xd0u }
    };
    core_machine_cpu_instruction_lexeme lexeme;
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(invalid_forms) / sizeof(invalid_forms[0]);
        ++index) {
        if (core_machine_cpu_instruction_lexeme_scan(invalid_forms[index], 2u,
                CORE_MACHINE_CPU_PROFILE_8086, TYPE_FALSE, &lexeme)) return 1;
    }
    for (index = 0u; index < sizeof(valid_forms) / sizeof(valid_forms[0]);
        ++index) {
        if (!core_machine_cpu_instruction_lexeme_scan(valid_forms[index], 2u,
                CORE_MACHINE_CPU_PROFILE_8086, TYPE_FALSE, &lexeme) ||
            !lexeme.available) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_xlat_function(C_VOID)
{
    static const type_unsigned_8 program[] = { 0xd7u };
    const timing_manifest_record *record = timing_manifest_find("I86-XLAT");
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 expected_value = 0xa5u;
    timing_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run;
    core_machine *machine = STD_NULL;
    C_INT failed = record == STD_NULL || !timing_manifest_prepare(&machine,
        &capture, program, sizeof(program));

    if (!failed) {
        machine->executor_cpu.data.bx = 0x0010u;
        machine->executor_cpu.data.al = 0x0004u;
        failed = core_machine_memory_write(machine, 0x0014u, &expected_value,
                sizeof(expected_value)) != TYPE_STATUS_OK ||
            core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 11u || capture.count != 1u ||
            machine->executor_cpu.data.al != expected_value ||
            capture.observation.source_ticks != 11u ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY;
    }
    if (failed) STD_PRINTF("M5:T435:S5:I86-MANIFEST-RECIPE:FAIL:I86-XLAT-FUNCTION\n");
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_manifest_probe_pop_cs_function(C_VOID)
{
    static const type_unsigned_8 program[] = { 0x0fu };
    static const type_unsigned_8 new_cs[] = { 0x34u, 0x12u };
    const timing_manifest_record *record = timing_manifest_find("I86-POP-SEG-CS");
    const core_machine_run_budget budget = { 1u, 0u };
    timing_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run;
    core_machine *machine = STD_NULL;
    C_INT failed = record == STD_NULL || !timing_manifest_prepare(&machine,
        &capture, program, sizeof(program));

    if (!failed) {
        machine->executor_cpu.data.sp = 0x0200u;
        failed = core_machine_memory_write(machine,
                machine->executor_cpu.data.ss.base + 0x0200u, new_cs,
                sizeof(new_cs)) != TYPE_STATUS_OK ||
            core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != 8u || capture.count != 1u ||
            machine->executor_cpu.data.cs.selector != 0x1234u ||
            machine->executor_cpu.data.cs.base != 0x12340u ||
            machine->executor_cpu.data.sp != 0x0202u ||
            capture.observation.source_ticks != 8u ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY;
    }
    if (failed) STD_PRINTF("M5:T435:S5:I86-MANIFEST-RECIPE:FAIL:I86-POP-SEG-CS-FUNCTION\n");
    core_machine_destroy(machine);
    return failed;
}

/* Functional predicates are deliberately separate from the timing recipes:
 * a classified retirement alone cannot establish the 8086 architectural
 * result. These are the first semantic family checks in the S5 runner. */
static C_INT timing_manifest_probe_alu_function(C_VOID)
{
    typedef struct timing_manifest_alu_function_recipe {
        type_unsigned_8 program[2];
        type_unsigned_16 ax;
        type_unsigned_16 bx;
        type_unsigned_32 eflags;
        type_unsigned_16 expected_ax;
        type_unsigned_32 expected_flags;
    } timing_manifest_alu_function_recipe;
    static const timing_manifest_alu_function_recipe recipes[] = {
        { { 0x03u, 0xc3u }, 1u, 2u, 0u, 3u, 0u },
        { { 0x13u, 0xc3u }, 1u, 2u, VCPU_EFLAGS_CF, 4u, 0u },
        { { 0x2bu, 0xc3u }, 1u, 2u, 0u, 0xffffu,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_SF },
        { { 0x1bu, 0xc3u }, 4u, 2u, VCPU_EFLAGS_CF, 1u, 0u },
        { { 0x0bu, 0xc3u }, 0x00f0u, 0x0f00u, 0u, 0x0ff0u, 0u },
        { { 0x23u, 0xc3u }, 0x00f0u, 0x0f00u, 0u, 0u, VCPU_EFLAGS_ZF },
        { { 0x33u, 0xc3u }, 0x00f0u, 0x0f00u, 0u, 0x0ff0u, 0u }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        timing_manifest_capture capture = { { 0 }, 0u };
        core_machine_run_result run = { 0 };
        core_machine *machine = STD_NULL;
        const type_unsigned_32 observed_mask = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF |
            VCPU_EFLAGS_SF;
        C_INT failed = !timing_manifest_prepare(&machine, &capture,
            recipes[index].program, sizeof(recipes[index].program));

        if (!failed) {
            machine->executor_cpu.data.ax = recipes[index].ax;
            machine->executor_cpu.data.bx = recipes[index].bx;
            machine->executor_cpu.data.eflags = recipes[index].eflags;
            failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
                run.executed != 1u || machine->executor_cpu.data.ax !=
                    recipes[index].expected_ax ||
                (machine->executor_cpu.data.eflags & observed_mask) !=
                    recipes[index].expected_flags;
        }
        core_machine_destroy(machine);
        if (failed) {
            STD_PRINTF("M5:T435:S5:I86-FUNCTION:FAIL:ALU:%u\n", (unsigned)index);
            return 1;
        }
    }
    return 0;
}

static C_INT timing_manifest_probe_adjustment_function(C_VOID)
{
    typedef struct timing_manifest_adjustment_function_recipe {
        type_unsigned_8 program[2];
        type_unsigned_8 bytes;
        type_unsigned_16 ax;
        type_unsigned_16 expected_ax;
        type_unsigned_16 expected_dx;
        type_unsigned_32 observed_flags;
        type_unsigned_32 expected_flags;
    } timing_manifest_adjustment_function_recipe;
    static const timing_manifest_adjustment_function_recipe recipes[] = {
        { { 0x37u, 0u }, 1u, 0x000bu, 0x0101u, 0u, VCPU_EFLAGS_CF | VCPU_EFLAGS_AF,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_AF },
        { { 0x3fu, 0u }, 1u, 0x010bu, 0x0005u, 0u, VCPU_EFLAGS_CF | VCPU_EFLAGS_AF,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_AF },
        { { 0x27u, 0u }, 1u, 0x000au, 0x0010u, 0u, VCPU_EFLAGS_CF | VCPU_EFLAGS_AF,
            VCPU_EFLAGS_AF },
        { { 0x2fu, 0u }, 1u, 0x000au, 0x0004u, 0u, VCPU_EFLAGS_CF | VCPU_EFLAGS_AF,
            VCPU_EFLAGS_AF },
        { { 0xd5u, 0x0au }, 2u, 0x0203u, 0x0017u, 0u, 0u, 0u },
        { { 0xd4u, 0x0au }, 2u, 0x0023u, 0x0305u, 0u, 0u, 0u },
        { { 0x98u, 0u }, 1u, 0x0080u, 0xff80u, 0u, 0u, 0u },
        { { 0x99u, 0u }, 1u, 0x8000u, 0x8000u, 0xffffu, 0u, 0u }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        timing_manifest_capture capture = { { 0 }, 0u };
        core_machine_run_result run = { 0 };
        core_machine *machine = STD_NULL;
        C_INT failed = !timing_manifest_prepare(&machine, &capture,
            recipes[index].program, recipes[index].bytes);

        if (!failed) {
            machine->executor_cpu.data.ax = recipes[index].ax;
            failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
                run.executed != 1u || machine->executor_cpu.data.ax !=
                    recipes[index].expected_ax || machine->executor_cpu.data.dx !=
                    recipes[index].expected_dx ||
                (machine->executor_cpu.data.eflags & recipes[index].observed_flags) !=
                    recipes[index].expected_flags;
        }
        core_machine_destroy(machine);
        if (failed) {
            STD_PRINTF("M5:T435:S5:I86-FUNCTION:FAIL:ADJUST:%u\n", (unsigned)index);
            return 1;
        }
    }
    return 0;
}

static C_INT timing_manifest_probe_data_stack_function(C_VOID)
{
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 mov[] = { 0xb8u, 0x34u, 0x12u };
    const type_unsigned_8 xchg[] = { 0x93u };
    const type_unsigned_8 push[] = { 0x50u };
    const type_unsigned_8 pop[] = { 0x5bu };
    const type_unsigned_16 pushed = 0x4a3cu;
    timing_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    type_unsigned_16 observed = 0u;
    C_INT failed = !timing_manifest_prepare(&machine, &capture, mov, sizeof(mov));

    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            machine->executor_cpu.data.ax != 0x1234u;
    }
    core_machine_destroy(machine);
    machine = STD_NULL;
    if (!failed && timing_manifest_prepare(&machine, &capture, xchg, sizeof(xchg))) {
        machine->executor_cpu.data.ax = 0x1234u;
        machine->executor_cpu.data.bx = 0x5678u;
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            machine->executor_cpu.data.ax != 0x5678u ||
            machine->executor_cpu.data.bx != 0x1234u;
    } else if (!failed) failed = 1;
    core_machine_destroy(machine);
    machine = STD_NULL;
    if (!failed && timing_manifest_prepare(&machine, &capture, push, sizeof(push))) {
        machine->executor_cpu.data.ax = pushed;
        machine->executor_cpu.data.sp = 0x8000u;
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            machine->executor_cpu.data.sp != 0x7ffeu ||
            core_machine_memory_read(machine, 0x7ffeu, &observed, sizeof(observed)) !=
                TYPE_STATUS_OK || observed != pushed;
    } else if (!failed) failed = 1;
    core_machine_destroy(machine);
    machine = STD_NULL;
    if (!failed && timing_manifest_prepare(&machine, &capture, pop, sizeof(pop))) {
        machine->executor_cpu.data.sp = 0x8000u;
        failed = core_machine_memory_write(machine, 0x8000u, &pushed, sizeof(pushed)) !=
                TYPE_STATUS_OK || core_machine_run(machine, budget, &run) !=
                TYPE_STATUS_OK || machine->executor_cpu.data.bx != pushed ||
            machine->executor_cpu.data.sp != 0x8002u;
    } else if (!failed) failed = 1;
    core_machine_destroy(machine);
    if (failed) STD_PRINTF("M5:T435:S5:I86-FUNCTION:FAIL:DATA-STACK\n");
    return failed;
}

static C_INT timing_manifest_probe_group3_function(C_VOID)
{
    typedef struct timing_manifest_group3_function_recipe {
        type_unsigned_8 program[2];
        type_unsigned_16 ax;
        type_unsigned_16 bx;
        type_unsigned_16 expected_ax;
        type_unsigned_16 expected_dx;
    } timing_manifest_group3_function_recipe;
    static const timing_manifest_group3_function_recipe recipes[] = {
        { { 0xf6u, 0xe3u }, 2u, 3u, 6u, 0u },
        { { 0xf6u, 0xebu }, 0xfffeu, 3u, 0xfffau, 0u },
        { { 0xf6u, 0xf3u }, 7u, 3u, 0x0102u, 0u },
        { { 0xf6u, 0xfbu }, 0xfff9u, 3u, 0xfffeu, 0u },
        { { 0xf7u, 0xe3u }, 2u, 3u, 6u, 0u },
        { { 0xf7u, 0xebu }, 0xfffeu, 3u, 0xfffau, 0xffffu },
        { { 0xf7u, 0xf3u }, 7u, 3u, 2u, 1u },
        { { 0xf7u, 0xfbu }, 0xfff9u, 3u, 0xfffeu, 0xffffu }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        timing_manifest_capture capture = { { 0 }, 0u };
        core_machine_run_result run = { 0 };
        core_machine *machine = STD_NULL;
        C_INT failed = !timing_manifest_prepare(&machine, &capture,
            recipes[index].program, sizeof(recipes[index].program));

        if (!failed) {
            machine->executor_cpu.data.ax = recipes[index].ax;
            machine->executor_cpu.data.bx = recipes[index].bx;
            machine->executor_cpu.data.dx = recipes[index].program[0] == 0xf7u &&
                recipes[index].program[1] == 0xfbu ? 0xffffu : 0u;
            failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
                run.executed != 1u || machine->executor_cpu.data.ax !=
                    recipes[index].expected_ax || machine->executor_cpu.data.dx !=
                    recipes[index].expected_dx;
        }
        core_machine_destroy(machine);
        if (failed) {
            STD_PRINTF("M5:T435:S5:I86-FUNCTION:FAIL:GROUP3:%u\n", (unsigned)index);
            return 1;
        }
    }
    return 0;
}

static C_INT timing_manifest_probe_branch_function(C_VOID)
{
    typedef struct timing_manifest_branch_function_recipe {
        type_unsigned_8 program[2];
        type_unsigned_16 cx;
        type_unsigned_32 eflags;
        type_unsigned_16 expected_ip;
        type_unsigned_16 expected_cx;
    } timing_manifest_branch_function_recipe;
    static const timing_manifest_branch_function_recipe recipes[] = {
        { { 0x74u, 0x02u }, 0u, VCPU_EFLAGS_ZF, 0xfff4u, 0u },
        { { 0x74u, 0x02u }, 0u, 0u, 0xfff2u, 0u },
        { { 0xe2u, 0x02u }, 2u, 0u, 0xfff4u, 1u },
        { { 0xe2u, 0x02u }, 1u, 0u, 0xfff2u, 0u },
        { { 0xe3u, 0x02u }, 0u, 0u, 0xfff4u, 0u }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        timing_manifest_capture capture = { { 0 }, 0u };
        core_machine_run_result run = { 0 };
        core_machine *machine = STD_NULL;
        C_INT failed = !timing_manifest_prepare(&machine, &capture,
            recipes[index].program, sizeof(recipes[index].program));

        if (!failed) {
            machine->executor_cpu.data.cx = recipes[index].cx;
            machine->executor_cpu.data.eflags = recipes[index].eflags;
            failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
                run.executed != 1u || machine->executor_cpu.data.ip !=
                    recipes[index].expected_ip || machine->executor_cpu.data.cx !=
                    recipes[index].expected_cx;
        }
        if (failed) {
            STD_PRINTF("M5:T435:S5:I86-FUNCTION:FAIL:BRANCH:%u:ip=%u:cx=%u\n",
                (unsigned)index, machine == STD_NULL ? 0u : machine->executor_cpu.data.ip,
                machine == STD_NULL ? 0u : machine->executor_cpu.data.cx);
            core_machine_destroy(machine);
            return 1;
        }
        core_machine_destroy(machine);
    }
    return 0;
}

static C_INT timing_manifest_probe_flag_function(C_VOID)
{
    typedef struct timing_manifest_flag_function_recipe {
        type_unsigned_8 opcode;
        type_unsigned_32 initial_flags;
        type_unsigned_32 expected_flags;
    } timing_manifest_flag_function_recipe;
    static const timing_manifest_flag_function_recipe recipes[] = {
        { 0xf8u, VCPU_EFLAGS_CF, 0u },
        { 0xf9u, 0u, VCPU_EFLAGS_CF },
        { 0xf5u, VCPU_EFLAGS_CF, 0u },
        { 0xfcu, VCPU_EFLAGS_DF, 0u },
        { 0xfdu, 0u, VCPU_EFLAGS_DF },
        { 0xfau, VCPU_EFLAGS_IF, 0u },
        { 0xfbu, 0u, VCPU_EFLAGS_IF }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_32 observed = VCPU_EFLAGS_CF | VCPU_EFLAGS_DF |
        VCPU_EFLAGS_IF;
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        timing_manifest_capture capture = { { 0 }, 0u };
        core_machine_run_result run = { 0 };
        core_machine *machine = STD_NULL;
        C_INT failed = !timing_manifest_prepare(&machine, &capture,
            &recipes[index].opcode, 1u);

        if (!failed) {
            machine->executor_cpu.data.eflags = recipes[index].initial_flags;
            failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
                run.executed != 1u || (machine->executor_cpu.data.eflags & observed) !=
                    recipes[index].expected_flags;
        }
        core_machine_destroy(machine);
        if (failed) {
            STD_PRINTF("M5:T435:S5:I86-FUNCTION:FAIL:FLAGS:%u\n", (unsigned)index);
            return 1;
        }
    }
    return 0;
}

static C_INT timing_manifest_probe_compare_function(C_VOID)
{
    typedef struct timing_manifest_compare_function_recipe {
        type_unsigned_8 program[2];
        type_unsigned_16 ax;
        type_unsigned_16 bx;
        type_unsigned_32 expected_flags;
    } timing_manifest_compare_function_recipe;
    static const timing_manifest_compare_function_recipe recipes[] = {
        { { 0x3bu, 0xc3u }, 1u, 2u, VCPU_EFLAGS_CF | VCPU_EFLAGS_SF },
        { { 0x3bu, 0xc3u }, 2u, 2u, VCPU_EFLAGS_ZF },
        { { 0x85u, 0xc3u }, 0x00f0u, 0x0f00u, VCPU_EFLAGS_ZF }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_32 observed = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF |
        VCPU_EFLAGS_SF | VCPU_EFLAGS_OF;
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        timing_manifest_capture capture = { { 0 }, 0u };
        core_machine_run_result run = { 0 };
        core_machine *machine = STD_NULL;
        C_INT failed = !timing_manifest_prepare(&machine, &capture,
            recipes[index].program, sizeof(recipes[index].program));

        if (!failed) {
            machine->executor_cpu.data.ax = recipes[index].ax;
            machine->executor_cpu.data.bx = recipes[index].bx;
            failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
                run.executed != 1u || machine->executor_cpu.data.ax !=
                    recipes[index].ax || (machine->executor_cpu.data.eflags & observed) !=
                    recipes[index].expected_flags;
        }
        core_machine_destroy(machine);
        if (failed) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_unary_function(C_VOID)
{
    typedef struct timing_manifest_unary_function_recipe {
        type_unsigned_8 program[2];
        type_unsigned_16 ax;
        type_unsigned_16 expected_ax;
        type_unsigned_32 initial_flags;
        type_unsigned_32 expected_flags;
    } timing_manifest_unary_function_recipe;
    static const timing_manifest_unary_function_recipe recipes[] = {
        { { 0x40u, 0u }, 0xffffu, 0u, VCPU_EFLAGS_CF, VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF },
        { { 0x48u, 0u }, 0u, 0xffffu, VCPU_EFLAGS_CF, VCPU_EFLAGS_CF | VCPU_EFLAGS_SF },
        { { 0xf7u, 0xd0u }, 0x00f0u, 0xff0fu, 0u, 0u },
        { { 0xf7u, 0xd8u }, 1u, 0xffffu, 0u, VCPU_EFLAGS_CF | VCPU_EFLAGS_SF }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_32 observed = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF |
        VCPU_EFLAGS_SF;
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        timing_manifest_capture capture = { { 0 }, 0u };
        core_machine_run_result run = { 0 };
        core_machine *machine = STD_NULL;
        C_INT failed = !timing_manifest_prepare(&machine, &capture,
            recipes[index].program, recipes[index].program[1] == 0u ? 1u : 2u);

        if (!failed) {
            machine->executor_cpu.data.ax = recipes[index].ax;
            machine->executor_cpu.data.eflags = recipes[index].initial_flags;
            failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
                machine->executor_cpu.data.ax != recipes[index].expected_ax ||
                (machine->executor_cpu.data.eflags & observed) != recipes[index].expected_flags;
        }
        core_machine_destroy(machine);
        if (failed) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_lahf_sahf_function(C_VOID)
{
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 lahf = 0x9fu;
    const type_unsigned_8 sahf = 0x9eu;
    const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF;
    timing_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_manifest_prepare(&machine, &capture, &lahf, 1u);

    if (!failed) {
        machine->executor_cpu.data.ax = 0x1200u;
        machine->executor_cpu.data.eflags = flags;
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            machine->executor_cpu.data.ax != 0xd700u;
    }
    core_machine_destroy(machine);
    machine = STD_NULL;
    if (!failed && timing_manifest_prepare(&machine, &capture, &sahf, 1u)) {
        machine->executor_cpu.data.ax = 0xd700u;
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            (machine->executor_cpu.data.eflags & flags) != flags;
    } else if (!failed) failed = 1;
    core_machine_destroy(machine);
    return failed;
}

/* LOCK is an 8086 prefix, not an RMW-form whitelist.  These representatives
 * deliberately cross the legacy, primary, control-stack and string owners;
 * each must retire once with the documented two-clock additive term. */
static C_INT timing_manifest_probe_general_lock_prefix(C_VOID)
{
    static const timing_manifest_recipe recipes[] = {
        { "LOCK-NOP", { 0xf0u, 0x90u }, 2u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "LOCK-MOV-RI", { 0xf0u, 0xb8u, 0x34u, 0x12u }, 4u, 6u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "LOCK-PUSH-R", { 0xf0u, 0x50u }, 2u, 13u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "LOCK-MOVSB", { 0xf0u, 0xa4u }, 2u, 20u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        timing_manifest_capture capture = { { 0 }, 0u };
        core_machine_run_result run = { 0 };
        core_machine *machine = STD_NULL;
        C_INT failed = !timing_manifest_prepare(&machine, &capture,
            recipes[index].program, recipes[index].program_bytes);

        if (!failed) {
            failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
                run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
                run.ticks != recipes[index].expected_ticks || capture.count != 1u ||
                capture.observation.source_ticks != recipes[index].expected_ticks ||
                capture.observation.timing_disposition !=
                    CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
                capture.observation.timing_origin != recipes[index].expected_origin ||
                (capture.observation.formula_inputs &
                    CORE_MACHINE_CPU_TIMING_INPUT_LOCK) == 0u;
        }
        core_machine_destroy(machine);
        if (failed) {
            STD_PRINTF("M5:T435:S5:I86-LOCK-GENERAL:FAIL:%s\n",
                recipes[index].key_id);
            return 1;
        }
    }
    return 0;
}

static C_INT timing_manifest_probe_adjustments(C_VOID)
{
    static const timing_manifest_recipe recipes[] = {
        { "I86-ADJ-AAA", { 0x37u, 0u }, 1u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-ADJ-AAS", { 0x3fu, 0u }, 1u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-ADJ-DAA", { 0x27u, 0u }, 1u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-ADJ-DAS", { 0x2fu, 0u }, 1u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-ADJ-AAD", { 0xd5u, 0x0au }, 2u, 60u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-ADJ-AAM", { 0xd4u, 0x0au }, 2u, 83u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-CONV-CBW", { 0x98u, 0u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-CONV-CWD", { 0x99u, 0u }, 1u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-XLAT", { 0xd7u, 0u }, 1u, 11u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-FLAG-CLC", { 0xf8u, 0u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-FLAG-CLD", { 0xfcu, 0u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-FLAG-CLI", { 0xfau, 0u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-FLAG-CMC", { 0xf5u, 0u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-FLAG-STC", { 0xf9u, 0u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-FLAG-STD", { 0xfdu, 0u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-FLAG-STI", { 0xfbu, 0u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-FLAG-LAHF", { 0x9fu, 0u }, 1u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-FLAG-SAHF", { 0x9eu, 0u }, 1u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-FLAG-NOP", { 0x90u, 0u }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_manifest_run_exact_recipe(&recipes[index])) {
            STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:%s\n",
                recipes[index].key_id);
            return 1;
        }
    }
    {
        static const timing_manifest_recipe segment_recipe = {
            "I86-XLAT-SEGMENT", { 0x26u, 0xd7u }, 2u, 13u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY
        };
        static const timing_manifest_recipe locked_segment_recipe = {
            "I86-XLAT-LOCK-SEGMENT", { 0xf0u, 0x26u, 0xd7u }, 3u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY
        };

        if (timing_manifest_run_exact_recipe_with_inputs_and_formula(
                &segment_recipe, 0u, 0u, 0u,
                CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE,
                CORE_MACHINE_RETIREMENT_CONTROL_NONE)) return 1;
        if (timing_manifest_run_exact_recipe_with_inputs_and_formula(
                &locked_segment_recipe, 0u, 0u, 0u,
                CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
                CORE_MACHINE_CPU_TIMING_INPUT_LOCK,
                CORE_MACHINE_RETIREMENT_CONTROL_NONE)) return 1;
    }
    if (timing_manifest_probe_xlat_function()) return 1;
    return 0;
}

static C_INT timing_manifest_probe_alu_register_forms(C_VOID)
{
    static const C_CHAR *const rr_keys[] = {
        "I86-ALU-ADD-RR", "I86-ALU-OR-RR", "I86-ALU-ADC-RR",
        "I86-ALU-SBB-RR", "I86-ALU-AND-RR", "I86-ALU-SUB-RR",
        "I86-ALU-XOR-RR"
    };
    static const C_CHAR *const ri_keys[] = {
        "I86-ALU-ADD-RI", "I86-ALU-OR-RI", "I86-ALU-ADC-RI",
        "I86-ALU-SBB-RI", "I86-ALU-AND-RI", "I86-ALU-SUB-RI",
        "I86-ALU-XOR-RI"
    };
    static const C_CHAR *const ai_keys[] = {
        "I86-ALU-ADD-AI", "I86-ALU-OR-AI", "I86-ALU-ADC-AI",
        "I86-ALU-SBB-AI", "I86-ALU-AND-AI", "I86-ALU-SUB-AI",
        "I86-ALU-XOR-AI"
    };
    static const type_unsigned_8 bases[] = {
        0x00u, 0x08u, 0x10u, 0x18u, 0x20u, 0x28u, 0x30u
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(bases) / sizeof(bases[0]); ++index) {
        timing_manifest_recipe recipe = {
            rr_keys[index], { (type_unsigned_8)(bases[index] + 3u), 0xc1u }, 2u,
            3u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY
        };

        if (timing_manifest_run_exact_recipe(&recipe)) return 1;
        recipe.key_id = ri_keys[index];
        recipe.program[0] = 0x81u;
        recipe.program[1] = (type_unsigned_8)(0xc0u | (index << 3u));
        recipe.program[2] = 0x01u;
        recipe.program[3] = 0u;
        recipe.program_bytes = 4u;
        recipe.expected_ticks = 4u;
        if (timing_manifest_run_exact_recipe(&recipe)) return 1;
        recipe.key_id = ai_keys[index];
        recipe.program[0] = (type_unsigned_8)(bases[index] + 5u);
        recipe.program[1] = 0x01u;
        recipe.program[2] = 0u;
        recipe.program_bytes = 3u;
        if (timing_manifest_run_exact_recipe(&recipe)) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_compare_and_test_register_forms(C_VOID)
{
    static const timing_manifest_recipe recipes[] = {
        { "I86-CMP-RR", { 0x3bu, 0xc1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-CMP-RI", { 0x81u, 0xf8u, 0x01u, 0u }, 4u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-CMP-AI", { 0x3du, 0x01u, 0u }, 3u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-TEST-RR", { 0x85u, 0xc1u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-TEST-RI", { 0xf7u, 0xc0u, 0x01u, 0u }, 4u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-TEST-AI", { 0xa9u, 0x01u, 0u }, 3u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY }
    };
    {
        STD_SIZE_T index;

        for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
            if (timing_manifest_run_exact_recipe(&recipes[index])) return 1;
        }
    }
    return 0;
}

static C_INT timing_manifest_probe_register_data_forms(C_VOID)
{
    static const timing_manifest_recipe recipes[] = {
        { "I86-INC-R16", { 0x40u, 0u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-INC-R8", { 0xfeu, 0xc0u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-DEC-R16", { 0x48u, 0u }, 1u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-DEC-R8", { 0xfeu, 0xc8u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-NOT-R", { 0xf7u, 0xd0u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-NEG-R", { 0xf7u, 0xd8u }, 2u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-XCHG-AXR", { 0x91u, 0u }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-XCHG-RR", { 0x87u, 0xc1u }, 2u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-MOV-RR", { 0x8bu, 0xc1u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-MOV-RI", { 0xb8u, 0x34u, 0x12u }, 3u, 4u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_manifest_run_exact_recipe(&recipes[index])) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_stack_register_forms(C_VOID)
{
    static const timing_manifest_recipe recipes[] = {
        { "I86-PUSH-R", { 0x50u, 0u }, 1u, 11u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I86-PUSH-SEG-ES", { 0x06u, 0u }, 1u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-PUSH-SEG-CS", { 0x0eu, 0u }, 1u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-PUSH-SEG-SS", { 0x16u, 0u }, 1u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-PUSH-SEG-DS", { 0x1eu, 0u }, 1u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-PUSH-F", { 0x9cu, 0u }, 1u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I86-POP-R", { 0x58u, 0u }, 1u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I86-POP-SEG-ES", { 0x07u, 0u }, 1u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-POP-SEG-CS", { 0x0fu, 0u }, 1u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-POP-SEG-SS", { 0x17u, 0u }, 1u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-POP-SEG-DS", { 0x1fu, 0u }, 1u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-POP-F", { 0x9du, 0u }, 1u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_manifest_run_exact_recipe(&recipes[index])) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_conditional_branches(C_VOID)
{
    static const C_CHAR *const taken_keys[] = {
        "I86-JCC-JO-TAKEN", "I86-JCC-JNO-TAKEN", "I86-JCC-JB-TAKEN",
        "I86-JCC-JAE-TAKEN", "I86-JCC-JE-TAKEN", "I86-JCC-JNE-TAKEN",
        "I86-JCC-JBE-TAKEN", "I86-JCC-JA-TAKEN", "I86-JCC-JS-TAKEN",
        "I86-JCC-JNS-TAKEN", "I86-JCC-JP-TAKEN", "I86-JCC-JNP-TAKEN",
        "I86-JCC-JL-TAKEN", "I86-JCC-JGE-TAKEN", "I86-JCC-JLE-TAKEN",
        "I86-JCC-JG-TAKEN"
    };
    static const C_CHAR *const not_keys[] = {
        "I86-JCC-JO-NOT", "I86-JCC-JNO-NOT", "I86-JCC-JB-NOT",
        "I86-JCC-JAE-NOT", "I86-JCC-JE-NOT", "I86-JCC-JNE-NOT",
        "I86-JCC-JBE-NOT", "I86-JCC-JA-NOT", "I86-JCC-JS-NOT",
        "I86-JCC-JNS-NOT", "I86-JCC-JP-NOT", "I86-JCC-JNP-NOT",
        "I86-JCC-JL-NOT", "I86-JCC-JGE-NOT", "I86-JCC-JLE-NOT",
        "I86-JCC-JG-NOT"
    };
    static const type_unsigned_32 taken_flags[] = {
        VCPU_EFLAGS_OF, 0u, VCPU_EFLAGS_CF, 0u,
        VCPU_EFLAGS_ZF, 0u, VCPU_EFLAGS_CF, 0u,
        VCPU_EFLAGS_SF, 0u, VCPU_EFLAGS_PF, 0u,
        VCPU_EFLAGS_SF, 0u, VCPU_EFLAGS_ZF, 0u
    };
    static const type_unsigned_32 not_flags[] = {
        0u, VCPU_EFLAGS_OF, 0u, VCPU_EFLAGS_CF,
        0u, VCPU_EFLAGS_ZF, 0u, VCPU_EFLAGS_CF,
        0u, VCPU_EFLAGS_SF, 0u, VCPU_EFLAGS_PF,
        0u, VCPU_EFLAGS_SF, 0u, VCPU_EFLAGS_ZF
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(taken_keys) / sizeof(taken_keys[0]); ++index) {
        timing_manifest_recipe recipe = {
            taken_keys[index], { (type_unsigned_8)(0x70u + index), 0x01u },
            2u, 16u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY
        };

        if (timing_manifest_run_exact_recipe_with_control(&recipe,
                taken_flags[index], 0u,
                CORE_MACHINE_RETIREMENT_CONTROL_TAKEN)) return 1;
        recipe.key_id = not_keys[index];
        recipe.expected_ticks = 4u;
        if (timing_manifest_run_exact_recipe_with_control(&recipe,
                not_flags[index], 0u,
                CORE_MACHINE_RETIREMENT_CONTROL_FALLTHROUGH)) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_counted_branches(C_VOID)
{
    static const timing_manifest_recipe recipes[] = {
        { "I86-JCXZ-TAKEN", { 0xe3u, 0x01u }, 2u, 18u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I86-JCXZ-NOT", { 0xe3u, 0x01u }, 2u, 6u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I86-LOOP-TAKEN", { 0xe2u, 0x01u }, 2u, 17u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I86-LOOP-NOT", { 0xe2u, 0x01u }, 2u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I86-LOOPE-TAKEN", { 0xe1u, 0x01u }, 2u, 18u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I86-LOOPE-NOT", { 0xe1u, 0x01u }, 2u, 6u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I86-LOOPNE-TAKEN", { 0xe0u, 0x01u }, 2u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I86-LOOPNE-NOT", { 0xe0u, 0x01u }, 2u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }
    };
    static const type_unsigned_32 flags[] = {
        0u, 0u, 0u, 0u, VCPU_EFLAGS_ZF, VCPU_EFLAGS_ZF, 0u, 0u
    };
    static const type_unsigned_16 cx[] = {
        0u, 1u, 2u, 1u, 2u, 1u, 2u, 1u
    };
    static const core_machine_retirement_control_outcome outcomes[] = {
        CORE_MACHINE_RETIREMENT_CONTROL_TAKEN,
        CORE_MACHINE_RETIREMENT_CONTROL_FALLTHROUGH,
        CORE_MACHINE_RETIREMENT_CONTROL_TAKEN,
        CORE_MACHINE_RETIREMENT_CONTROL_FALLTHROUGH,
        CORE_MACHINE_RETIREMENT_CONTROL_TAKEN,
        CORE_MACHINE_RETIREMENT_CONTROL_FALLTHROUGH,
        CORE_MACHINE_RETIREMENT_CONTROL_TAKEN,
        CORE_MACHINE_RETIREMENT_CONTROL_FALLTHROUGH
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_manifest_run_exact_recipe_with_control(&recipes[index],
                flags[index], cx[index], outcomes[index])) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_wait_and_escape(C_VOID)
{
    static const timing_manifest_recipe recipes[] = {
        { "I86-WAIT", { 0x9bu, 0u }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-ESC-R", { 0xd8u, 0xc0u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_manifest_run_exact_recipe(&recipes[index])) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_escape_memory_forms(C_VOID)
{
    static const timing_manifest_memory_recipe recipes[] = {
        { "I86-ESC-M", { 0xd8u, 0x06u, 0u, 0x10u }, 4u, 14u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 0u, 0u, 0u, 0u },
        { "I86-ESC-M-SEGMENT", { 0x26u, 0xd8u, 0x06u, 0u, 0x10u }, 5u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 0u, 0u, 0u, 0u }
    };

    return timing_manifest_run_l3_memory_recipe_with_inputs(&recipes[0],
        CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
        CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS) ||
        timing_manifest_run_l3_memory_recipe_with_inputs(&recipes[1],
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE);
}

static C_INT timing_manifest_probe_group2_register_one(C_VOID)
{
    static const C_CHAR *const keys[] = {
        "I86-ROL-R1", "I86-ROR-R1", "I86-RCL-R1", "I86-RCR-R1",
        "I86-SHL-R1", "I86-SHR-R1", "I86-SAR-R1"
    };
    static const type_unsigned_8 extensions[] = { 0u, 1u, 2u, 3u, 4u, 5u, 7u };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(keys) / sizeof(keys[0]); ++index) {
        timing_manifest_recipe recipe = {
            keys[index], { 0xd1u, (type_unsigned_8)(0xc0u |
                (extensions[index] << 3u)) },
            2u, 2u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY
        };

        if (timing_manifest_run_exact_recipe(&recipe)) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_group2_register_cl(C_VOID)
{
    static const C_CHAR *const keys[] = {
        "I86-ROL-RCL", "I86-ROR-RCL", "I86-RCL-RCL", "I86-RCR-RCL",
        "I86-SHL-RCL", "I86-SHR-RCL", "I86-SAR-RCL"
    };
    static const type_unsigned_8 extensions[] = { 0u, 1u, 2u, 3u, 4u, 5u, 7u };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(keys) / sizeof(keys[0]); ++index) {
        timing_manifest_recipe recipe = {
            keys[index], { 0xd3u, (type_unsigned_8)(0xc0u |
                (extensions[index] << 3u)) }, 2u, 12u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY
        };

        if (timing_manifest_run_exact_recipe_with_control(&recipe, 0u, 1u,
                CORE_MACHINE_RETIREMENT_CONTROL_NONE)) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_immediate_port_io(C_VOID)
{
    static const timing_manifest_recipe recipes[] = {
        { "I86-IN-IMM", { 0xe4u, 0x80u }, 2u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I86-OUT-IMM", { 0xe6u, 0x80u }, 2u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_manifest_run_exact_recipe(&recipes[index])) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_dx_port_io(C_VOID)
{
    static const timing_manifest_recipe recipes[] = {
        { "I86-IN-DX", { 0xecu, 0u }, 1u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO },
        { "I86-OUT-DX", { 0xeeu, 0u }, 1u, 8u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_manifest_run_exact_recipe_with_inputs(&recipes[index], 0u,
                0u, 0x0080u, CORE_MACHINE_RETIREMENT_CONTROL_NONE)) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_near_control_transfers(C_VOID)
{
    static const timing_manifest_recipe recipes[] = {
        { "I86-CALL-NEAR", { 0xe8u, 0u, 0u }, 3u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I86-JMP-NEAR", { 0xe9u, 0u, 0u }, 3u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }
    };
    return timing_manifest_run_exact_recipe(&recipes[0]) ||
        timing_manifest_run_exact_recipe_with_control(&recipes[1], 0u, 0u,
            CORE_MACHINE_RETIREMENT_CONTROL_TAKEN);
}

/* Group 3 is deliberately a named L2 model, not an L3 value.  Keep its
 * verification separate so this runner can require the ledger's exact model
 * identity and still exercise the instruction's arithmetic result. */
static C_INT timing_manifest_probe_group3_l2(C_VOID)
{
    static const timing_manifest_recipe recipes[] = {
        { "I86-MUL-R8", { 0xf6u, 0xe3u }, 2u, 71u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I86-MUL-R16", { 0xf7u, 0xe3u }, 2u, 119u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I86-IMUL-R8", { 0xf6u, 0xebu }, 2u, 91u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I86-IMUL-R16", { 0xf7u, 0xebu }, 2u, 139u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I86-DIV-R8", { 0xf6u, 0xf3u }, 2u, 81u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I86-DIV-R16", { 0xf7u, 0xf3u }, 2u, 145u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I86-IDIV-R8", { 0xf6u, 0xfbu }, 2u, 111u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC },
        { "I86-IDIV-R16", { 0xf7u, 0xfbu }, 2u, 175u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC }
    };
    static const C_CHAR *const memory_keys[] = {
        "I86-MUL-M8", "I86-MUL-M16", "I86-IMUL-M8", "I86-IMUL-M16",
        "I86-DIV-M8", "I86-DIV-M16", "I86-IDIV-M8", "I86-IDIV-M16"
    };
    static const type_unsigned_8 memory_extensions[] = {
        4u, 4u, 5u, 5u, 6u, 6u, 7u, 7u
    };
    static const type_unsigned_64 memory_ticks[] = {
        83u, 131u, 103u, 151u, 93u, 157u, 123u, 187u
    };
    const type_unsigned_8 operand8 = 3u;
    const type_unsigned_16 operand16 = 3u;
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]) +
            sizeof(memory_keys) / sizeof(memory_keys[0]); ++index) {
        timing_manifest_recipe recipe;
        const timing_manifest_record *record;
        const core_machine_run_budget budget = { 1u, 0u };
        timing_manifest_capture capture = { { 0 }, 0u };
        core_machine_run_result run = { 0 };
        core_machine *machine = STD_NULL;
        C_INT memory_operand = index >= sizeof(recipes) / sizeof(recipes[0]);
        C_INT division = (index >= 4u && index < 8u) || index >= 12u;
        C_INT failed;

        if (memory_operand) {
            STD_SIZE_T memory_index = index - sizeof(recipes) / sizeof(recipes[0]);

            recipe.key_id = memory_keys[memory_index];
            recipe.program[0] = memory_index & 1u ? 0xf7u : 0xf6u;
            recipe.program[1] = (type_unsigned_8)(0x06u |
                (memory_extensions[memory_index] << 3u));
            recipe.program[2] = 0x00u;
            recipe.program[3] = 0x10u;
            recipe.program_bytes = 4u;
            recipe.expected_ticks = memory_ticks[memory_index];
            recipe.expected_origin =
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC;
        } else {
            recipe = recipes[index];
        }
        record = timing_manifest_find(recipe.key_id);
        failed = record == STD_NULL || !timing_manifest_is_i86(record) ||
            STD_STRCMP(record->profile, "8086") != 0 ||
            STD_STRCMP(record->level, "L2:G3") != 0 ||
            STD_STRCMP(record->source_rule, "S1:L2-86BOX-8086-G3 bounds") != 0 ||
            !timing_manifest_prepare(&machine, &capture, recipe.program,
                recipe.program_bytes);
        if (!failed && memory_operand) {
            failed = core_machine_memory_write(machine, 0x1000u,
                (index & 1u) != 0u ? (const C_VOID *)&operand16 :
                (const C_VOID *)&operand8, (index & 1u) != 0u ?
                sizeof(operand16) : sizeof(operand8)) != TYPE_STATUS_OK;
        }
        if (!failed) {
            machine->executor_cpu.data.ax = 2u;
            machine->executor_cpu.data.bx = 3u;
            machine->executor_cpu.data.dx = 0u;
            if (division) {
                machine->executor_cpu.data.ax = 6u;
            }
            failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
                run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
                run.ticks != recipe.expected_ticks || capture.count != 1u ||
                machine->executor_cpu.data.ax != (division ? 2u : 6u) ||
                capture.observation.source_ticks != recipe.expected_ticks ||
                capture.observation.timing_origin != recipe.expected_origin ||
                (capture.observation.formula_inputs &
                    CORE_MACHINE_CPU_TIMING_INPUT_GROUP3_OPERAND) == 0u ||
                (memory_operand && (capture.observation.formula_inputs &
                    (CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
                     CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS)) == 0u);
        }
        if (failed) {
            STD_PRINTF("I86 G3 ticks=%llu source=%llu origin=%d inputs=%u ax=%u dx=%u count=%u\n",
                run.ticks, capture.observation.source_ticks,
                capture.observation.timing_origin, capture.observation.formula_inputs,
                machine == STD_NULL ? 0u : machine->executor_cpu.data.ax,
                machine == STD_NULL ? 0u : machine->executor_cpu.data.dx,
                capture.count);
            STD_PRINTF("M5:T435:S4:I86-MANIFEST-RECIPE:FAIL:%s\n", recipe.key_id);
            core_machine_destroy(machine);
            return 1;
        }
        core_machine_destroy(machine);
    }
    /* The L2:G3 rows have operand-sensitive inputs, so execute their LOCK
     * companions with the same operands rather than borrowing an L3 helper. */
    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]) +
            sizeof(memory_keys) / sizeof(memory_keys[0]); ++index) {
        timing_manifest_recipe recipe;
        C_CHAR key[160];
        const timing_manifest_record *record;
        const core_machine_run_budget budget = { 1u, 0u };
        timing_manifest_capture capture = { { 0 }, 0u };
        core_machine_run_result run = { 0 };
        core_machine *machine = STD_NULL;
        C_INT memory_operand = index >= sizeof(recipes) / sizeof(recipes[0]);
        C_INT division = (index >= 4u && index < 8u) || index >= 12u;
        C_INT failed;

        if (memory_operand) {
            STD_SIZE_T memory_index = index - sizeof(recipes) / sizeof(recipes[0]);

            recipe.key_id = memory_keys[memory_index];
            recipe.program[0] = memory_index & 1u ? 0xf7u : 0xf6u;
            recipe.program[1] = (type_unsigned_8)(0x06u |
                (memory_extensions[memory_index] << 3u));
            recipe.program[2] = 0x00u;
            recipe.program[3] = 0x10u;
            recipe.program_bytes = 4u;
            recipe.expected_ticks = memory_ticks[memory_index];
            recipe.expected_origin =
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC;
        } else {
            recipe = recipes[index];
        }
        if (recipe.program_bytes >= sizeof(recipe.program) ||
            STD_SNPRINTF(key, sizeof(key), "%s-LOCK", recipe.key_id) < 0) {
            return 1;
        }
        {
            STD_SIZE_T byte_index;
            for (byte_index = recipe.program_bytes; byte_index != 0u; --byte_index) {
                recipe.program[byte_index] = recipe.program[byte_index - 1u];
            }
        }
        recipe.program[0] = 0xf0u;
        ++recipe.program_bytes;
        recipe.expected_ticks += 2u;
        recipe.key_id = key;
        record = timing_manifest_find(recipe.key_id);
        failed = record == STD_NULL || !timing_manifest_is_i86(record) ||
            STD_STRCMP(record->profile, "8086") != 0 ||
            STD_STRCMP(record->level, "L2:G3") != 0 ||
            !timing_manifest_prepare(&machine, &capture, recipe.program,
                recipe.program_bytes);
        if (!failed && memory_operand) {
            failed = core_machine_memory_write(machine, 0x1000u,
                (index & 1u) != 0u ? (const C_VOID *)&operand16 :
                (const C_VOID *)&operand8, (index & 1u) != 0u ?
                sizeof(operand16) : sizeof(operand8)) != TYPE_STATUS_OK;
        }
        if (!failed) {
            machine->executor_cpu.data.ax = division ? 6u : 2u;
            machine->executor_cpu.data.bx = 3u;
            machine->executor_cpu.data.dx = 0u;
            failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
                run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
                run.ticks != recipe.expected_ticks || capture.count != 1u ||
                machine->executor_cpu.data.ax != (division ? 2u : 6u) ||
                capture.observation.source_ticks != recipe.expected_ticks ||
                capture.observation.timing_origin != recipe.expected_origin ||
                (capture.observation.formula_inputs &
                    (CORE_MACHINE_CPU_TIMING_INPUT_GROUP3_OPERAND |
                     CORE_MACHINE_CPU_TIMING_INPUT_LOCK)) !=
                    (CORE_MACHINE_CPU_TIMING_INPUT_GROUP3_OPERAND |
                     CORE_MACHINE_CPU_TIMING_INPUT_LOCK);
        }
        core_machine_destroy(machine);
        if (failed) {
            STD_PRINTF("M5:T435:S5:I86-G3-LOCK:FAIL:%s\n", recipe.key_id);
            return 1;
        }
    }
    return 0;
}

static C_INT timing_manifest_probe_group3_memory_contexts(C_VOID)
{
    typedef struct timing_manifest_group3_context_recipe {
        const C_CHAR *key_id;
        type_unsigned_8 extension;
        type_unsigned_8 word;
        type_unsigned_8 prefix;
        type_unsigned_16 address;
        type_unsigned_64 expected_ticks;
        C_INT division;
        type_unsigned_32 required_formula_inputs;
    } timing_manifest_group3_context_recipe;
    static const timing_manifest_group3_context_recipe recipes[] = {
        { "I86-MUL-M8-SEGMENT", 4u, 0u, 0x26u, 0x1000u, 85u, 0,
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE },
        { "I86-MUL-M16-SEGMENT", 4u, 1u, 0x26u, 0x1000u, 133u, 0,
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE },
        { "I86-IMUL-M8-SEGMENT", 5u, 0u, 0x26u, 0x1000u, 105u, 0,
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE },
        { "I86-IMUL-M16-SEGMENT", 5u, 1u, 0x26u, 0x1000u, 153u, 0,
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE },
        { "I86-DIV-M8-SEGMENT", 6u, 0u, 0x26u, 0x1000u, 95u, 1,
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE },
        { "I86-DIV-M16-SEGMENT", 6u, 1u, 0x26u, 0x1000u, 159u, 1,
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE },
        { "I86-IDIV-M8-SEGMENT", 7u, 0u, 0x26u, 0x1000u, 125u, 1,
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE },
        { "I86-IDIV-M16-SEGMENT", 7u, 1u, 0x26u, 0x1000u, 189u, 1,
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE },
        { "I86-MUL-M16-ODD-WORD", 4u, 1u, 0u, 0x1001u, 135u, 0,
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-IMUL-M16-ODD-WORD", 5u, 1u, 0u, 0x1001u, 155u, 0,
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-DIV-M16-ODD-WORD", 6u, 1u, 0u, 0x1001u, 161u, 1,
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-IDIV-M16-ODD-WORD", 7u, 1u, 0u, 0x1001u, 191u, 1,
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-MUL-M16-SEGMENT-ODD-WORD", 4u, 1u, 0x26u, 0x1001u, 137u, 0,
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-IMUL-M16-SEGMENT-ODD-WORD", 5u, 1u, 0x26u, 0x1001u, 157u, 0,
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-DIV-M16-SEGMENT-ODD-WORD", 6u, 1u, 0x26u, 0x1001u, 163u, 1,
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-IDIV-M16-SEGMENT-ODD-WORD", 7u, 1u, 0x26u, 0x1001u, 193u, 1,
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 operand8 = 3u;
    const type_unsigned_16 operand16 = 3u;
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        const timing_manifest_group3_context_recipe *recipe = &recipes[index];
        const timing_manifest_record *record = timing_manifest_find(recipe->key_id);
        type_unsigned_8 program[] = { 0xf6u, 0x06u, 0u, 0x10u, 0u };
        timing_manifest_capture capture = { { 0 }, 0u };
        core_machine_run_result run = { 0 };
        core_machine *machine = STD_NULL;
        C_INT failed;

        program[0] = recipe->word ? 0xf7u : 0xf6u;
        program[1] = (type_unsigned_8)(0x06u | (recipe->extension << 3u));
        program[2] = TYPE_MASK_UNSIGNED_8(recipe->address);
        program[3] = TYPE_MASK_UNSIGNED_8(recipe->address >> 8u);
        if (recipe->prefix != 0u) {
            program[4] = program[3]; program[3] = program[2]; program[2] = program[1];
            program[1] = program[0]; program[0] = recipe->prefix;
        }
        failed = record == STD_NULL || !timing_manifest_is_i86(record) ||
            STD_STRCMP(record->profile, "8086") != 0 ||
            STD_STRCMP(record->level, "L2:G3") != 0 || record->source_rule[0] == '\0' ||
            !timing_manifest_prepare(&machine, &capture, program,
                recipe->prefix == 0u ? 4u : 5u);
        if (!failed) {
            failed = core_machine_memory_write(machine, recipe->address,
                recipe->word ? (const C_VOID *)&operand16 : (const C_VOID *)&operand8,
                recipe->word ? sizeof(operand16) : sizeof(operand8)) != TYPE_STATUS_OK;
        }
        if (!failed) {
            machine->executor_cpu.data.ax = recipe->division ? 6u : 2u;
            machine->executor_cpu.data.dx = 0u;
            failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
                run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
                run.ticks != recipe->expected_ticks || capture.count != 1u ||
                machine->executor_cpu.data.ax != (recipe->division ? 2u : 6u) ||
                capture.observation.timing_origin !=
                    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC ||
                (capture.observation.formula_inputs &
                    (CORE_MACHINE_CPU_TIMING_INPUT_GROUP3_OPERAND |
                     CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
                     CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
                     recipe->required_formula_inputs)) !=
                    (CORE_MACHINE_CPU_TIMING_INPUT_GROUP3_OPERAND |
                     CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
                     CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
                     recipe->required_formula_inputs);
        }
        if (failed) {
            STD_PRINTF("M5:T435:S4:I86-MANIFEST-RECIPE:FAIL:%s\n", recipe->key_id);
            core_machine_destroy(machine);
            return 1;
        }
        core_machine_destroy(machine);
        {
            const timing_manifest_record *base_record =
                timing_manifest_find(recipe->key_id);
            C_CHAR key[160];
            type_unsigned_8 locked_program[6];
            type_unsigned_8 locked_bytes = 0u;
            timing_manifest_capture locked_capture = { { 0 }, 0u };
            core_machine_run_result locked_run = { 0 };
            core_machine *locked_machine = STD_NULL;
            C_INT locked_failed = base_record == STD_NULL ||
                !timing_manifest_lock_key_for_context(base_record, recipe->key_id,
                    key, sizeof(key));

            if (!locked_failed) {
                locked_program[locked_bytes++] = 0xf0u;
                if (recipe->prefix != 0u) locked_program[locked_bytes++] = recipe->prefix;
                locked_program[locked_bytes++] = recipe->word ? 0xf7u : 0xf6u;
                locked_program[locked_bytes++] = (type_unsigned_8)(0x06u |
                    (recipe->extension << 3u));
                locked_program[locked_bytes++] = TYPE_MASK_UNSIGNED_8(recipe->address);
                locked_program[locked_bytes++] = TYPE_MASK_UNSIGNED_8(recipe->address >> 8u);
                locked_failed = timing_manifest_find(key) == STD_NULL ||
                    !timing_manifest_prepare(&locked_machine,
                    &locked_capture, locked_program, locked_bytes);
            }
            if (!locked_failed) {
                locked_failed = core_machine_memory_write(locked_machine, recipe->address,
                    recipe->word ? (const C_VOID *)&operand16 :
                    (const C_VOID *)&operand8, recipe->word ? sizeof(operand16) :
                    sizeof(operand8)) != TYPE_STATUS_OK;
            }
            if (!locked_failed) {
                locked_machine->executor_cpu.data.ax = recipe->division ? 6u : 2u;
                locked_machine->executor_cpu.data.dx = 0u;
                locked_failed = core_machine_run(locked_machine, budget, &locked_run) !=
                        TYPE_STATUS_OK ||
                    locked_run.reason != CORE_MACHINE_STOP_BUDGET ||
                    locked_run.executed != 1u ||
                    locked_run.ticks != recipe->expected_ticks + 2u ||
                    locked_capture.count != 1u ||
                    locked_machine->executor_cpu.data.ax !=
                        (recipe->division ? 2u : 6u) ||
                    locked_capture.observation.timing_origin !=
                        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC ||
                    (locked_capture.observation.formula_inputs &
                        (CORE_MACHINE_CPU_TIMING_INPUT_GROUP3_OPERAND |
                         CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
                         CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
                         CORE_MACHINE_CPU_TIMING_INPUT_LOCK |
                         recipe->required_formula_inputs)) !=
                        (CORE_MACHINE_CPU_TIMING_INPUT_GROUP3_OPERAND |
                         CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
                         CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
                         CORE_MACHINE_CPU_TIMING_INPUT_LOCK |
                         recipe->required_formula_inputs);
            }
            core_machine_destroy(locked_machine);
            if (locked_failed) {
                STD_PRINTF("M5:T435:S5:I86-G3-CONTEXT-LOCK:FAIL:%s\n", key);
                return 1;
            }
        }
    }
    return 0;
}

static C_INT timing_manifest_probe_alu_memory_forms(C_VOID)
{
    static const C_CHAR *const rm_keys[] = {
        "I86-ALU-ADD-RM", "I86-ALU-OR-RM", "I86-ALU-ADC-RM",
        "I86-ALU-SBB-RM", "I86-ALU-AND-RM", "I86-ALU-SUB-RM",
        "I86-ALU-XOR-RM"
    };
    static const C_CHAR *const mr_keys[] = {
        "I86-ALU-ADD-MR", "I86-ALU-OR-MR", "I86-ALU-ADC-MR",
        "I86-ALU-SBB-MR", "I86-ALU-AND-MR", "I86-ALU-SUB-MR",
        "I86-ALU-XOR-MR"
    };
    static const C_CHAR *const mi_keys[] = {
        "I86-ALU-ADD-MI", "I86-ALU-OR-MI", "I86-ALU-ADC-MI",
        "I86-ALU-SBB-MI", "I86-ALU-AND-MI", "I86-ALU-SUB-MI",
        "I86-ALU-XOR-MI"
    };
    static const type_unsigned_8 bases[] = {
        0x00u, 0x08u, 0x10u, 0x18u, 0x20u, 0x28u, 0x30u
    };
    static const type_unsigned_16 expected_rm[] = {
        3u, 3u, 3u, 0xffffu, 0u, 0xffffu, 3u
    };
    static const type_unsigned_16 expected_mr[] = {
        3u, 3u, 3u, 1u, 0u, 1u, 3u
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(bases) / sizeof(bases[0]); ++index) {
        timing_manifest_memory_recipe recipe = {
            rm_keys[index], { (type_unsigned_8)(bases[index] + 3u), 0x0eu,
                0x00u, 0x10u }, 4u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 1u, 2u, 0u, expected_rm[index], 2u
        };

        if (timing_manifest_run_l3_memory_recipe(&recipe)) return 1;
        recipe.key_id = mr_keys[index];
        recipe.program[0] = (type_unsigned_8)(bases[index] + 1u);
        recipe.expected_ticks = 22u;
        recipe.expected_cx = 1u;
        recipe.expected_memory_value = expected_mr[index];
        if (timing_manifest_run_l3_memory_recipe(&recipe)) return 1;
        recipe.key_id = mi_keys[index];
        recipe.program[0] = 0x81u;
        recipe.program[1] = (type_unsigned_8)(0x06u | (index << 3u));
        recipe.program[2] = 0x00u;
        recipe.program[3] = 0x10u;
        recipe.program[4] = 0x01u;
        recipe.program[5] = 0x00u;
        recipe.program_bytes = 6u;
        recipe.expected_ticks = 23u;
        recipe.expected_memory_value = expected_mr[index];
        if (timing_manifest_run_l3_memory_recipe(&recipe)) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_primary_memory_forms(C_VOID)
{
    static const timing_manifest_memory_recipe recipes[] = {
        { "I86-CMP-RM", { 0x3bu, 0x0eu, 0u, 0x10u }, 4u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 1u, 2u, 0u, 1u, 2u },
        { "I86-CMP-MR", { 0x39u, 0x0eu, 0u, 0x10u }, 4u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 1u, 2u, 0u, 1u, 2u },
        { "I86-CMP-MI", { 0x81u, 0x3eu, 0u, 0x10u, 1u, 0u }, 6u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 2u, 0u, 0u, 2u },
        { "I86-TEST-RM", { 0x85u, 0x0eu, 0u, 0x10u }, 4u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 1u, 2u, 0u, 1u, 2u },
        { "I86-TEST-MI", { 0xf7u, 0x06u, 0u, 0x10u, 1u, 0u }, 6u, 17u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 2u, 0u, 0u, 2u },
        { "I86-INC-M", { 0xffu, 0x06u, 0u, 0x10u }, 4u, 21u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 2u, 0u, 0u, 3u },
        { "I86-DEC-M", { 0xffu, 0x0eu, 0u, 0x10u }, 4u, 21u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 2u, 0u, 0u, 1u },
        { "I86-NOT-M", { 0xf7u, 0x16u, 0u, 0x10u }, 4u, 22u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 2u, 0u, 0u, 0xfffdu },
        { "I86-NEG-M", { 0xf7u, 0x1eu, 0u, 0x10u }, 4u, 22u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 2u, 0u, 0u, 0xfffeu },
        { "I86-XCHG-MR", { 0x87u, 0x0eu, 0u, 0x10u }, 4u, 23u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 1u, 2u, 0u, 2u, 1u },
        { "I86-MOV-RM", { 0x8bu, 0x0eu, 0u, 0x10u }, 4u, 14u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 2u, 0u, 2u, 2u },
        { "I86-MOV-MR", { 0x89u, 0x0eu, 0u, 0x10u }, 4u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 1u, 2u, 0u, 1u, 1u },
        { "I86-MOV-MI", { 0xc7u, 0x06u, 0u, 0x10u, 0x34u, 0x12u },
            6u, 16u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 2u, 0u, 0u, 0x1234u },
        { "I86-MOV-MOFFS-W", { 0xa1u, 0u, 0x10u }, 3u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 2u, 2u, 0u, 2u },
        { "I86-MOV-MOFFS-R", { 0xa3u, 0u, 0x10u }, 3u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            1u, 0u, 2u, 1u, 0u, 1u }
    };
    static const C_CHAR *const segment_keys[] = {
        "I86-CMP-RM-SEGMENT", "I86-CMP-MR-SEGMENT", "I86-CMP-MI-SEGMENT",
        "I86-TEST-RM-SEGMENT", "I86-TEST-MI-SEGMENT", "I86-INC-M-SEGMENT",
        "I86-DEC-M-SEGMENT", "I86-NOT-M-SEGMENT", "I86-NEG-M-SEGMENT",
        STD_NULL, "I86-MOV-RM-SEGMENT", "I86-MOV-MR-SEGMENT",
        "I86-MOV-MI-SEGMENT", "I86-MOV-MOFFS-W-SEGMENT",
        "I86-MOV-MOFFS-R-SEGMENT"
    };
    static const C_CHAR *const odd_keys[] = {
        "I86-CMP-RM-ODD-WORD", "I86-CMP-MR-ODD-WORD", "I86-CMP-MI-ODD-WORD",
        "I86-TEST-RM-ODD-WORD", "I86-TEST-MI-ODD-WORD", "I86-INC-M-ODD-WORD",
        "I86-DEC-M-ODD-WORD", "I86-NOT-M-ODD-WORD", "I86-NEG-M-ODD-WORD",
        STD_NULL, "I86-MOV-RM-ODD-WORD", "I86-MOV-MR-ODD-WORD",
        "I86-MOV-MI-ODD-WORD", "I86-MOV-MOFFS-W-ODD-WORD",
        "I86-MOV-MOFFS-R-ODD-WORD"
    };
    static const C_CHAR *const combined_keys[] = {
        "I86-CMP-RM-SEGMENT-ODD-WORD", "I86-CMP-MR-SEGMENT-ODD-WORD",
        "I86-CMP-MI-SEGMENT-ODD-WORD", "I86-TEST-RM-SEGMENT-ODD-WORD",
        "I86-TEST-MI-SEGMENT-ODD-WORD", "I86-INC-M-SEGMENT-ODD-WORD",
        "I86-DEC-M-SEGMENT-ODD-WORD", "I86-NOT-M-SEGMENT-ODD-WORD",
        "I86-NEG-M-SEGMENT-ODD-WORD", STD_NULL,
        "I86-MOV-RM-SEGMENT-ODD-WORD", "I86-MOV-MR-SEGMENT-ODD-WORD",
        "I86-MOV-MI-SEGMENT-ODD-WORD", "I86-MOV-MOFFS-W-SEGMENT-ODD-WORD",
        "I86-MOV-MOFFS-R-SEGMENT-ODD-WORD"
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_manifest_run_l3_memory_recipe(&recipes[index])) return 1;
    }
    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        const C_CHAR *const keys[] = { segment_keys[index], odd_keys[index],
            combined_keys[index] };
        STD_SIZE_T context;

        for (context = 0u; context < sizeof(keys) / sizeof(keys[0]); ++context) {
            timing_manifest_memory_recipe recipe;
            type_unsigned_8 opcode_index;
            type_unsigned_8 address_index;
            C_INT segment = context == 0u || context == 2u;
            C_INT odd = context == 1u || context == 2u;
            type_unsigned_64 odd_ticks = index >= 5u && index <= 8u ? 8u : 4u;

            if (keys[context] == STD_NULL) continue;
            recipe = recipes[index];
            recipe.key_id = keys[context];
            recipe.expected_ticks += segment ? 2u : 0u;
            recipe.expected_ticks += odd ? odd_ticks : 0u;
            if (segment) {
                type_unsigned_8 byte_index;
                for (byte_index = recipe.program_bytes; byte_index > 0u; --byte_index) {
                    recipe.program[byte_index] = recipe.program[byte_index - 1u];
                }
                recipe.program[0] = 0x26u;
                ++recipe.program_bytes;
            }
            opcode_index = segment ? 1u : 0u;
            address_index = (recipe.program[opcode_index] == 0xa0u ||
                recipe.program[opcode_index] == 0xa1u ||
                recipe.program[opcode_index] == 0xa2u ||
                recipe.program[opcode_index] == 0xa3u) ? opcode_index + 1u :
                opcode_index + 2u;
            if (odd) {
                recipe.program[address_index] = 1u;
                recipe.program[address_index + 1u] = 0x10u;
            }
            if (timing_manifest_run_l3_memory_recipe_with_inputs(&recipe,
                    odd ? CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD : 0u)) return 1;
        }
    }
    return 0;
}

static C_INT timing_manifest_probe_alu_segment_contexts(C_VOID)
{
    static const C_CHAR *const rm_keys[] = {
        "I86-ALU-ADD-RM-SEGMENT", "I86-ALU-OR-RM-SEGMENT",
        "I86-ALU-ADC-RM-SEGMENT", "I86-ALU-SBB-RM-SEGMENT",
        "I86-ALU-AND-RM-SEGMENT", "I86-ALU-SUB-RM-SEGMENT",
        "I86-ALU-XOR-RM-SEGMENT"
    };
    static const C_CHAR *const mr_keys[] = {
        "I86-ALU-ADD-MR-SEGMENT", "I86-ALU-OR-MR-SEGMENT",
        "I86-ALU-ADC-MR-SEGMENT", "I86-ALU-SBB-MR-SEGMENT",
        "I86-ALU-AND-MR-SEGMENT", "I86-ALU-SUB-MR-SEGMENT",
        "I86-ALU-XOR-MR-SEGMENT"
    };
    static const C_CHAR *const mi_keys[] = {
        "I86-ALU-ADD-MI-SEGMENT", "I86-ALU-OR-MI-SEGMENT",
        "I86-ALU-ADC-MI-SEGMENT", "I86-ALU-SBB-MI-SEGMENT",
        "I86-ALU-AND-MI-SEGMENT", "I86-ALU-SUB-MI-SEGMENT",
        "I86-ALU-XOR-MI-SEGMENT"
    };
    static const type_unsigned_8 bases[] = {
        0x00u, 0x08u, 0x10u, 0x18u, 0x20u, 0x28u, 0x30u
    };
    static const type_unsigned_16 expected_rm[] = {
        3u, 3u, 3u, 0xffffu, 0u, 0xffffu, 3u
    };
    static const type_unsigned_16 expected_mr[] = {
        3u, 3u, 3u, 1u, 0u, 1u, 3u
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(bases) / sizeof(bases[0]); ++index) {
        timing_manifest_memory_recipe recipe = {
            rm_keys[index], { 0x26u, (type_unsigned_8)(bases[index] + 3u),
                0x0eu, 0x00u, 0x10u }, 5u, 17u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 1u, 2u, 0u, expected_rm[index], 2u
        };

        if (timing_manifest_run_l3_memory_recipe(&recipe)) return 1;
        recipe.key_id = mr_keys[index];
        recipe.program[1] = (type_unsigned_8)(bases[index] + 1u);
        recipe.expected_ticks = 24u;
        recipe.expected_cx = 1u;
        recipe.expected_memory_value = expected_mr[index];
        if (timing_manifest_run_l3_memory_recipe(&recipe)) return 1;
        recipe.key_id = mi_keys[index];
        recipe.program[1] = 0x81u;
        recipe.program[2] = (type_unsigned_8)(0x06u | (index << 3u));
        recipe.program[3] = 0x00u;
        recipe.program[4] = 0x10u;
        recipe.program[5] = 0x01u;
        recipe.program[6] = 0x00u;
        recipe.program_bytes = 7u;
        recipe.expected_ticks = 25u;
        recipe.expected_memory_value = expected_mr[index];
        if (timing_manifest_run_l3_memory_recipe(&recipe)) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_alu_odd_word_contexts(C_VOID)
{
    static const C_CHAR *const rm_keys[] = {
        "I86-ALU-ADD-RM-ODD-WORD", "I86-ALU-OR-RM-ODD-WORD",
        "I86-ALU-ADC-RM-ODD-WORD", "I86-ALU-SBB-RM-ODD-WORD",
        "I86-ALU-AND-RM-ODD-WORD", "I86-ALU-SUB-RM-ODD-WORD",
        "I86-ALU-XOR-RM-ODD-WORD"
    };
    static const C_CHAR *const mr_keys[] = {
        "I86-ALU-ADD-MR-ODD-WORD", "I86-ALU-OR-MR-ODD-WORD",
        "I86-ALU-ADC-MR-ODD-WORD", "I86-ALU-SBB-MR-ODD-WORD",
        "I86-ALU-AND-MR-ODD-WORD", "I86-ALU-SUB-MR-ODD-WORD",
        "I86-ALU-XOR-MR-ODD-WORD"
    };
    static const C_CHAR *const mi_keys[] = {
        "I86-ALU-ADD-MI-ODD-WORD", "I86-ALU-OR-MI-ODD-WORD",
        "I86-ALU-ADC-MI-ODD-WORD", "I86-ALU-SBB-MI-ODD-WORD",
        "I86-ALU-AND-MI-ODD-WORD", "I86-ALU-SUB-MI-ODD-WORD",
        "I86-ALU-XOR-MI-ODD-WORD"
    };
    static const type_unsigned_8 bases[] = {
        0x00u, 0x08u, 0x10u, 0x18u, 0x20u, 0x28u, 0x30u
    };
    static const type_unsigned_16 expected_rm[] = {
        3u, 3u, 3u, 0xffffu, 0u, 0xffffu, 3u
    };
    static const type_unsigned_16 expected_mr[] = {
        3u, 3u, 3u, 1u, 0u, 1u, 3u
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(bases) / sizeof(bases[0]); ++index) {
        timing_manifest_memory_recipe recipe = {
            rm_keys[index], { (type_unsigned_8)(bases[index] + 3u), 0x0eu,
                0x01u, 0x10u }, 4u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 1u, 2u, 0u, expected_rm[index], 2u
        };

        if (timing_manifest_run_l3_memory_recipe_with_inputs(&recipe,
                CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD)) return 1;
        recipe.key_id = mr_keys[index];
        recipe.program[0] = (type_unsigned_8)(bases[index] + 1u);
        recipe.expected_ticks = 30u;
        recipe.expected_cx = 1u;
        recipe.expected_memory_value = expected_mr[index];
        if (timing_manifest_run_l3_memory_recipe_with_inputs(&recipe,
                CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD)) return 1;
        recipe.key_id = mi_keys[index];
        recipe.program[0] = 0x81u;
        recipe.program[1] = (type_unsigned_8)(0x06u | (index << 3u));
        recipe.program[2] = 0x01u;
        recipe.program[3] = 0x10u;
        recipe.program[4] = 0x01u;
        recipe.program[5] = 0x00u;
        recipe.program_bytes = 6u;
        recipe.expected_ticks = 31u;
        recipe.expected_memory_value = expected_mr[index];
        if (timing_manifest_run_l3_memory_recipe_with_inputs(&recipe,
                CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD)) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_lock_contexts(C_VOID)
{
    static const C_CHAR *const mr_keys[] = {
        "I86-ALU-ADD-MR-LOCK", "I86-ALU-OR-MR-LOCK",
        "I86-ALU-ADC-MR-LOCK", "I86-ALU-SBB-MR-LOCK",
        "I86-ALU-AND-MR-LOCK", "I86-ALU-SUB-MR-LOCK",
        "I86-ALU-XOR-MR-LOCK"
    };
    static const C_CHAR *const mi_keys[] = {
        "I86-ALU-ADD-MI-LOCK", "I86-ALU-OR-MI-LOCK",
        "I86-ALU-ADC-MI-LOCK", "I86-ALU-SBB-MI-LOCK",
        "I86-ALU-AND-MI-LOCK", "I86-ALU-SUB-MI-LOCK",
        "I86-ALU-XOR-MI-LOCK"
    };
    static const C_CHAR *const mr_context_keys[][7] = {
        { "I86-ALU-ADD-MR-LOCK-SEGMENT", "I86-ALU-OR-MR-LOCK-SEGMENT",
            "I86-ALU-ADC-MR-LOCK-SEGMENT", "I86-ALU-SBB-MR-LOCK-SEGMENT",
            "I86-ALU-AND-MR-LOCK-SEGMENT", "I86-ALU-SUB-MR-LOCK-SEGMENT",
            "I86-ALU-XOR-MR-LOCK-SEGMENT" },
        { "I86-ALU-ADD-MR-LOCK-ODD-WORD", "I86-ALU-OR-MR-LOCK-ODD-WORD",
            "I86-ALU-ADC-MR-LOCK-ODD-WORD", "I86-ALU-SBB-MR-LOCK-ODD-WORD",
            "I86-ALU-AND-MR-LOCK-ODD-WORD", "I86-ALU-SUB-MR-LOCK-ODD-WORD",
            "I86-ALU-XOR-MR-LOCK-ODD-WORD" },
        { "I86-ALU-ADD-MR-LOCK-SEGMENT-ODD-WORD",
            "I86-ALU-OR-MR-LOCK-SEGMENT-ODD-WORD",
            "I86-ALU-ADC-MR-LOCK-SEGMENT-ODD-WORD",
            "I86-ALU-SBB-MR-LOCK-SEGMENT-ODD-WORD",
            "I86-ALU-AND-MR-LOCK-SEGMENT-ODD-WORD",
            "I86-ALU-SUB-MR-LOCK-SEGMENT-ODD-WORD",
            "I86-ALU-XOR-MR-LOCK-SEGMENT-ODD-WORD" }
    };
    static const C_CHAR *const mi_context_keys[][7] = {
        { "I86-ALU-ADD-MI-LOCK-SEGMENT", "I86-ALU-OR-MI-LOCK-SEGMENT",
            "I86-ALU-ADC-MI-LOCK-SEGMENT", "I86-ALU-SBB-MI-LOCK-SEGMENT",
            "I86-ALU-AND-MI-LOCK-SEGMENT", "I86-ALU-SUB-MI-LOCK-SEGMENT",
            "I86-ALU-XOR-MI-LOCK-SEGMENT" },
        { "I86-ALU-ADD-MI-LOCK-ODD-WORD", "I86-ALU-OR-MI-LOCK-ODD-WORD",
            "I86-ALU-ADC-MI-LOCK-ODD-WORD", "I86-ALU-SBB-MI-LOCK-ODD-WORD",
            "I86-ALU-AND-MI-LOCK-ODD-WORD", "I86-ALU-SUB-MI-LOCK-ODD-WORD",
            "I86-ALU-XOR-MI-LOCK-ODD-WORD" },
        { "I86-ALU-ADD-MI-LOCK-SEGMENT-ODD-WORD",
            "I86-ALU-OR-MI-LOCK-SEGMENT-ODD-WORD",
            "I86-ALU-ADC-MI-LOCK-SEGMENT-ODD-WORD",
            "I86-ALU-SBB-MI-LOCK-SEGMENT-ODD-WORD",
            "I86-ALU-AND-MI-LOCK-SEGMENT-ODD-WORD",
            "I86-ALU-SUB-MI-LOCK-SEGMENT-ODD-WORD",
            "I86-ALU-XOR-MI-LOCK-SEGMENT-ODD-WORD" }
    };
    static const C_CHAR *const scalar_context_keys[][4] = {
        { "I86-INC-M-LOCK-SEGMENT", "I86-DEC-M-LOCK-SEGMENT",
            "I86-NOT-M-LOCK-SEGMENT", "I86-NEG-M-LOCK-SEGMENT" },
        { "I86-INC-M-LOCK-ODD-WORD", "I86-DEC-M-LOCK-ODD-WORD",
            "I86-NOT-M-LOCK-ODD-WORD", "I86-NEG-M-LOCK-ODD-WORD" },
        { "I86-INC-M-LOCK-SEGMENT-ODD-WORD",
            "I86-DEC-M-LOCK-SEGMENT-ODD-WORD",
            "I86-NOT-M-LOCK-SEGMENT-ODD-WORD",
            "I86-NEG-M-LOCK-SEGMENT-ODD-WORD" }
    };
    static const type_unsigned_8 bases[] = {
        0x00u, 0x08u, 0x10u, 0x18u, 0x20u, 0x28u, 0x30u
    };
    static const type_unsigned_16 expected_mr[] = {
        3u, 3u, 3u, 1u, 0u, 1u, 3u
    };
    static const timing_manifest_memory_recipe scalar_recipes[] = {
        { "I86-INC-M-LOCK", { 0xf0u, 0xffu, 0x06u, 0u, 0x10u }, 5u, 23u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 2u, 0u, 0u, 3u },
        { "I86-DEC-M-LOCK", { 0xf0u, 0xffu, 0x0eu, 0u, 0x10u }, 5u, 23u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 2u, 0u, 0u, 1u },
        { "I86-NOT-M-LOCK", { 0xf0u, 0xf7u, 0x16u, 0u, 0x10u }, 5u, 24u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 2u, 0u, 0u, 0xfffdu },
        { "I86-NEG-M-LOCK", { 0xf0u, 0xf7u, 0x1eu, 0u, 0x10u }, 5u, 24u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 2u, 0u, 0u, 0xfffeu },
        { "I86-XCHG-MR-LOCK", { 0xf0u, 0x87u, 0x0eu, 0u, 0x10u }, 5u, 25u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 1u, 2u, 0u, 2u, 1u }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(bases) / sizeof(bases[0]); ++index) {
        timing_manifest_memory_recipe recipe = {
            mr_keys[index], { 0xf0u, (type_unsigned_8)(bases[index] + 1u),
                0x0eu, 0x00u, 0x10u }, 5u, 24u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 1u, 2u, 0u, 1u, expected_mr[index]
        };

        if (timing_manifest_run_l3_memory_recipe(&recipe)) return 1;
        recipe.key_id = mi_keys[index];
        recipe.program[1] = 0x81u;
        recipe.program[2] = (type_unsigned_8)(0x06u | (index << 3u));
        recipe.program[3] = 0x00u;
        recipe.program[4] = 0x10u;
        recipe.program[5] = 0x01u;
        recipe.program[6] = 0x00u;
        recipe.program_bytes = 7u;
        recipe.expected_ticks = 25u;
        recipe.expected_memory_value = expected_mr[index];
        if (timing_manifest_run_l3_memory_recipe(&recipe)) return 1;
    }
    for (index = 0u; index < sizeof(scalar_recipes) / sizeof(scalar_recipes[0]);
            ++index) {
        if (timing_manifest_run_l3_memory_recipe(&scalar_recipes[index])) return 1;
    }
    for (index = 0u; index < sizeof(bases) / sizeof(bases[0]); ++index) {
        timing_manifest_memory_recipe mr_recipe = {
            mr_keys[index], { 0xf0u, (type_unsigned_8)(bases[index] + 1u),
                0x0eu, 0u, 0x10u }, 5u, 24u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 1u, 2u, 0u, 1u, expected_mr[index]
        };
        timing_manifest_memory_recipe mi_recipe = mr_recipe;
        STD_SIZE_T context;

        mi_recipe.program[1] = 0x81u;
        mi_recipe.program[2] = (type_unsigned_8)(0x06u | (index << 3u));
        mi_recipe.program[5] = 1u;
        mi_recipe.program[6] = 0u;
        mi_recipe.program_bytes = 7u;
        mi_recipe.expected_ticks = 25u;
        mi_recipe.expected_cx = 1u;
        for (context = 0u; context < 3u; ++context) {
            C_INT segment_override = context == 0u || context == 2u;
            C_INT odd_word = context == 1u || context == 2u;

            if (timing_manifest_run_lock_memory_context(&mr_recipe,
                    mr_context_keys[context][index], segment_override,
                    odd_word)) return 1;
            if (timing_manifest_run_lock_memory_context(&mi_recipe,
                    mi_context_keys[context][index], segment_override,
                    odd_word)) return 1;
        }
    }
    for (index = 0u; index < 4u; ++index) {
        STD_SIZE_T context;

        for (context = 0u; context < 3u; ++context) {
            C_INT segment_override = context == 0u || context == 2u;
            C_INT odd_word = context == 1u || context == 2u;

            if (timing_manifest_run_lock_memory_context(&scalar_recipes[index],
                    scalar_context_keys[context][index], segment_override,
                    odd_word)) return 1;
        }
    }
    return 0;
}

static C_INT timing_manifest_probe_alu_segment_odd_word_contexts(C_VOID)
{
    static const C_CHAR *const rm_keys[] = {
        "I86-ALU-ADD-RM-SEGMENT-ODD-WORD",
        "I86-ALU-OR-RM-SEGMENT-ODD-WORD",
        "I86-ALU-ADC-RM-SEGMENT-ODD-WORD",
        "I86-ALU-SBB-RM-SEGMENT-ODD-WORD",
        "I86-ALU-AND-RM-SEGMENT-ODD-WORD",
        "I86-ALU-SUB-RM-SEGMENT-ODD-WORD",
        "I86-ALU-XOR-RM-SEGMENT-ODD-WORD"
    };
    static const C_CHAR *const mr_keys[] = {
        "I86-ALU-ADD-MR-SEGMENT-ODD-WORD",
        "I86-ALU-OR-MR-SEGMENT-ODD-WORD",
        "I86-ALU-ADC-MR-SEGMENT-ODD-WORD",
        "I86-ALU-SBB-MR-SEGMENT-ODD-WORD",
        "I86-ALU-AND-MR-SEGMENT-ODD-WORD",
        "I86-ALU-SUB-MR-SEGMENT-ODD-WORD",
        "I86-ALU-XOR-MR-SEGMENT-ODD-WORD"
    };
    static const C_CHAR *const mi_keys[] = {
        "I86-ALU-ADD-MI-SEGMENT-ODD-WORD",
        "I86-ALU-OR-MI-SEGMENT-ODD-WORD",
        "I86-ALU-ADC-MI-SEGMENT-ODD-WORD",
        "I86-ALU-SBB-MI-SEGMENT-ODD-WORD",
        "I86-ALU-AND-MI-SEGMENT-ODD-WORD",
        "I86-ALU-SUB-MI-SEGMENT-ODD-WORD",
        "I86-ALU-XOR-MI-SEGMENT-ODD-WORD"
    };
    static const type_unsigned_8 bases[] = {
        0x00u, 0x08u, 0x10u, 0x18u, 0x20u, 0x28u, 0x30u
    };
    static const type_unsigned_16 expected_rm[] = {
        3u, 3u, 3u, 0xffffu, 0u, 0xffffu, 3u
    };
    static const type_unsigned_16 expected_mr[] = {
        3u, 3u, 3u, 1u, 0u, 1u, 3u
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(bases) / sizeof(bases[0]); ++index) {
        timing_manifest_memory_recipe recipe = {
            rm_keys[index], { 0x26u, (type_unsigned_8)(bases[index] + 3u),
                0x0eu, 0x01u, 0x10u }, 5u, 21u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 1u, 2u, 0u, expected_rm[index], 2u
        };

        if (timing_manifest_run_l3_memory_recipe_with_inputs(&recipe,
                CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD)) return 1;
        recipe.key_id = mr_keys[index];
        recipe.program[1] = (type_unsigned_8)(bases[index] + 1u);
        recipe.expected_ticks = 32u;
        recipe.expected_cx = 1u;
        recipe.expected_memory_value = expected_mr[index];
        if (timing_manifest_run_l3_memory_recipe_with_inputs(&recipe,
                CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD)) return 1;
        recipe.key_id = mi_keys[index];
        recipe.program[1] = 0x81u;
        recipe.program[2] = (type_unsigned_8)(0x06u | (index << 3u));
        recipe.program[3] = 0x01u;
        recipe.program[4] = 0x10u;
        recipe.program[5] = 0x01u;
        recipe.program[6] = 0x00u;
        recipe.program_bytes = 7u;
        recipe.expected_ticks = 33u;
        recipe.expected_memory_value = expected_mr[index];
        if (timing_manifest_run_l3_memory_recipe_with_inputs(&recipe,
                CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD)) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_group2_memory_forms(C_VOID)
{
    static const C_CHAR *const one_keys[] = {
        "I86-ROL-M1", "I86-ROR-M1", "I86-RCL-M1", "I86-RCR-M1",
        "I86-SHL-M1", "I86-SHR-M1", "I86-SAR-M1"
    };
    static const C_CHAR *const cl_keys[] = {
        "I86-ROL-MCL", "I86-ROR-MCL", "I86-RCL-MCL", "I86-RCR-MCL",
        "I86-SHL-MCL", "I86-SHR-MCL", "I86-SAR-MCL"
    };
    static const type_unsigned_8 extensions[] = { 0u, 1u, 2u, 3u, 4u, 5u, 7u };
    static const type_unsigned_16 one_results[] = {
        4u, 1u, 4u, 1u, 4u, 1u, 1u
    };
    static const type_unsigned_16 cl_results[] = {
        8u, 0x8000u, 8u, 0u, 8u, 0u, 0u
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(extensions) / sizeof(extensions[0]); ++index) {
        timing_manifest_memory_recipe recipe = {
            one_keys[index], { 0xd1u, (type_unsigned_8)(0x06u |
                (extensions[index] << 3u)), 0u, 0x10u }, 4u, 21u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 2u, 0u, 0u, one_results[index]
        };

        if (timing_manifest_run_l3_memory_recipe(&recipe)) return 1;
        recipe.key_id = cl_keys[index];
        recipe.program[0] = 0xd3u;
        recipe.expected_ticks = 34u;
        recipe.initial_cx = 2u;
        recipe.expected_cx = 2u;
        recipe.expected_memory_value = cl_results[index];
        if (timing_manifest_run_l3_memory_recipe(&recipe)) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_group2_segment_contexts(C_VOID)
{
    static const C_CHAR *const one_keys[] = {
        "I86-ROL-M1-SEGMENT", "I86-ROR-M1-SEGMENT",
        "I86-RCL-M1-SEGMENT", "I86-RCR-M1-SEGMENT",
        "I86-SHL-M1-SEGMENT", "I86-SHR-M1-SEGMENT",
        "I86-SAR-M1-SEGMENT"
    };
    static const C_CHAR *const cl_keys[] = {
        "I86-ROL-MCL-SEGMENT", "I86-ROR-MCL-SEGMENT",
        "I86-RCL-MCL-SEGMENT", "I86-RCR-MCL-SEGMENT",
        "I86-SHL-MCL-SEGMENT", "I86-SHR-MCL-SEGMENT",
        "I86-SAR-MCL-SEGMENT"
    };
    static const type_unsigned_8 extensions[] = { 0u, 1u, 2u, 3u, 4u, 5u, 7u };
    static const type_unsigned_16 one_results[] = {
        4u, 1u, 4u, 1u, 4u, 1u, 1u
    };
    static const type_unsigned_16 cl_results[] = {
        8u, 0x8000u, 8u, 0u, 8u, 0u, 0u
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(extensions) / sizeof(extensions[0]); ++index) {
        timing_manifest_memory_recipe recipe = {
            one_keys[index], { 0x26u, 0xd1u, (type_unsigned_8)(0x06u |
                (extensions[index] << 3u)), 0u, 0x10u }, 5u, 23u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 2u, 0u, 0u, one_results[index]
        };

        if (timing_manifest_run_l3_memory_recipe(&recipe)) return 1;
        recipe.key_id = cl_keys[index];
        recipe.program[1] = 0xd3u;
        recipe.expected_ticks = 36u;
        recipe.initial_cx = 2u;
        recipe.expected_cx = 2u;
        recipe.expected_memory_value = cl_results[index];
        if (timing_manifest_run_l3_memory_recipe(&recipe)) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_group2_odd_word_contexts(C_VOID)
{
    static const C_CHAR *const one_keys[] = {
        "I86-ROL-M1-ODD-WORD", "I86-ROR-M1-ODD-WORD",
        "I86-RCL-M1-ODD-WORD", "I86-RCR-M1-ODD-WORD",
        "I86-SHL-M1-ODD-WORD", "I86-SHR-M1-ODD-WORD",
        "I86-SAR-M1-ODD-WORD"
    };
    static const C_CHAR *const cl_keys[] = {
        "I86-ROL-MCL-ODD-WORD", "I86-ROR-MCL-ODD-WORD",
        "I86-RCL-MCL-ODD-WORD", "I86-RCR-MCL-ODD-WORD",
        "I86-SHL-MCL-ODD-WORD", "I86-SHR-MCL-ODD-WORD",
        "I86-SAR-MCL-ODD-WORD"
    };
    static const type_unsigned_8 extensions[] = { 0u, 1u, 2u, 3u, 4u, 5u, 7u };
    static const type_unsigned_16 one_results[] = {
        4u, 1u, 4u, 1u, 4u, 1u, 1u
    };
    static const type_unsigned_16 cl_results[] = {
        8u, 0x8000u, 8u, 0u, 8u, 0u, 0u
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(extensions) / sizeof(extensions[0]); ++index) {
        timing_manifest_memory_recipe recipe = {
            one_keys[index], { 0xd1u, (type_unsigned_8)(0x06u |
                (extensions[index] << 3u)), 1u, 0x10u }, 4u, 29u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 2u, 0u, 0u, one_results[index]
        };

        if (timing_manifest_run_l3_memory_recipe_with_inputs(&recipe,
                CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD)) return 1;
        recipe.key_id = cl_keys[index];
        recipe.program[0] = 0xd3u;
        recipe.expected_ticks = 42u;
        recipe.initial_cx = 2u;
        recipe.expected_cx = 2u;
        recipe.expected_memory_value = cl_results[index];
        if (timing_manifest_run_l3_memory_recipe_with_inputs(&recipe,
                CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD)) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_group2_segment_odd_word_contexts(C_VOID)
{
    static const C_CHAR *const one_keys[] = {
        "I86-ROL-M1-SEGMENT-ODD-WORD", "I86-ROR-M1-SEGMENT-ODD-WORD",
        "I86-RCL-M1-SEGMENT-ODD-WORD", "I86-RCR-M1-SEGMENT-ODD-WORD",
        "I86-SHL-M1-SEGMENT-ODD-WORD", "I86-SHR-M1-SEGMENT-ODD-WORD",
        "I86-SAR-M1-SEGMENT-ODD-WORD"
    };
    static const C_CHAR *const cl_keys[] = {
        "I86-ROL-MCL-SEGMENT-ODD-WORD", "I86-ROR-MCL-SEGMENT-ODD-WORD",
        "I86-RCL-MCL-SEGMENT-ODD-WORD", "I86-RCR-MCL-SEGMENT-ODD-WORD",
        "I86-SHL-MCL-SEGMENT-ODD-WORD", "I86-SHR-MCL-SEGMENT-ODD-WORD",
        "I86-SAR-MCL-SEGMENT-ODD-WORD"
    };
    static const type_unsigned_8 extensions[] = { 0u, 1u, 2u, 3u, 4u, 5u, 7u };
    static const type_unsigned_16 one_results[] = {
        4u, 1u, 4u, 1u, 4u, 1u, 1u
    };
    static const type_unsigned_16 cl_results[] = {
        8u, 0x8000u, 8u, 0u, 8u, 0u, 0u
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(extensions) / sizeof(extensions[0]); ++index) {
        timing_manifest_memory_recipe recipe = {
            one_keys[index], { 0x26u, 0xd1u, (type_unsigned_8)(0x06u |
                (extensions[index] << 3u)), 1u, 0x10u }, 5u, 31u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 2u, 0u, 0u, one_results[index]
        };

        if (timing_manifest_run_l3_memory_recipe_with_inputs(&recipe,
                CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD)) return 1;
        recipe.key_id = cl_keys[index];
        recipe.program[1] = 0xd3u;
        recipe.expected_ticks = 44u;
        recipe.initial_cx = 2u;
        recipe.expected_cx = 2u;
        recipe.expected_memory_value = cl_results[index];
        if (timing_manifest_run_l3_memory_recipe_with_inputs(&recipe,
                CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD)) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_string_primitives(C_VOID)
{
    static const timing_manifest_string_recipe recipes[] = {
        { "I86-STRING-MOVS-B-NONE", 0xa4u, 18u },
        { "I86-STRING-MOVS-W-NONE", 0xa5u, 18u },
        { "I86-STRING-CMPS-B-NONE", 0xa6u, 22u },
        { "I86-STRING-CMPS-W-NONE", 0xa7u, 22u },
        { "I86-STRING-STOS-B-NONE", 0xaau, 11u },
        { "I86-STRING-STOS-W-NONE", 0xabu, 11u },
        { "I86-STRING-LODS-B-NONE", 0xacu, 12u },
        { "I86-STRING-LODS-W-NONE", 0xadu, 12u },
        { "I86-STRING-SCAS-B-NONE", 0xaeu, 15u },
        { "I86-STRING-SCAS-W-NONE", 0xafu, 15u }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_manifest_run_string_primitive(&recipes[index])) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_repeat_base_forms(C_VOID)
{
    static const timing_manifest_repeat_recipe recipes[] = {
        { "I86-REP-MOVS-B", 0xf3u, 0xa4u, 26u, 17u, 9u, 0u, 0, 0u },
        { "I86-REP-MOVS-W", 0xf3u, 0xa5u, 26u, 17u, 9u, 0u, 0, 0u },
        { "I86-REP-CMPS-REPE-B", 0xf3u, 0xa6u, 31u, 22u, 9u, 0u, 0, 0u },
        { "I86-REP-CMPS-REPE-W", 0xf3u, 0xa7u, 31u, 22u, 9u, 0u, 0, 0u },
        { "I86-REP-CMPS-REPNE-B", 0xf2u, 0xa6u, 31u, 22u, 9u, 0u, 0, 0u },
        { "I86-REP-CMPS-REPNE-W", 0xf2u, 0xa7u, 31u, 22u, 9u, 0u, 0, 0u },
        { "I86-REP-STOS-B", 0xf3u, 0xaau, 19u, 10u, 9u, 0u, 0, 0u },
        { "I86-REP-STOS-W", 0xf3u, 0xabu, 19u, 10u, 9u, 0u, 0, 0u },
        { "I86-REP-LODS-B", 0xf3u, 0xacu, 22u, 13u, 9u, 0u, 0, 0u },
        { "I86-REP-LODS-W", 0xf3u, 0xadu, 22u, 13u, 9u, 0u, 0, 0u },
        { "I86-REP-SCAS-REPE-B", 0xf3u, 0xaeu, 24u, 15u, 9u, 0u, 0, 0u },
        { "I86-REP-SCAS-REPE-W", 0xf3u, 0xafu, 24u, 15u, 9u, 0u, 0, 0u },
        { "I86-REP-SCAS-REPNE-B", 0xf2u, 0xaeu, 24u, 15u, 9u, 0u, 0, 0u },
        { "I86-REP-SCAS-REPNE-W", 0xf2u, 0xafu, 24u, 15u, 9u, 0u, 0, 0u }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_manifest_run_repeat_recipe(&recipes[index])) return 1;
    }
    return 0;
}

static C_INT timing_manifest_text_contains(const C_CHAR *text, const C_CHAR *needle)
{
    STD_SIZE_T offset;
    STD_SIZE_T index;

    if (text == STD_NULL || needle == STD_NULL || needle[0] == '\0') return 0;
    for (offset = 0u; text[offset] != '\0'; ++offset) {
        for (index = 0u; needle[index] != '\0' && text[offset + index] != '\0' &&
                text[offset + index] == needle[index]; ++index) {}
        if (needle[index] == '\0') return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_repeat_manifest_contexts(C_VOID)
{
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(timing_manifest_records) /
            sizeof(timing_manifest_records[0]); ++index) {
        const timing_manifest_record *record = &timing_manifest_records[index];
        timing_manifest_repeat_recipe recipe = { 0 };
        type_unsigned_64 odd_addition = 0u;
        C_INT segment;
        C_INT odd;

        if (!timing_manifest_text_contains(record->key_id, "I86-REP-")) continue;
        recipe.key_id = record->key_id;
        recipe.prefix = timing_manifest_text_contains(record->key_id, "REPNE") ?
            0xf2u : 0xf3u;
        if (timing_manifest_text_contains(record->key_id, "MOVS-B")) {
            recipe.opcode = 0xa4u; recipe.first_ticks = 26u;
            recipe.continuation_ticks = 17u; recipe.zero_ticks = 9u;
        } else if (timing_manifest_text_contains(record->key_id, "MOVS-W")) {
            recipe.opcode = 0xa5u; recipe.first_ticks = 26u;
            recipe.continuation_ticks = 17u; recipe.zero_ticks = 9u; odd_addition = 8u;
        } else if (timing_manifest_text_contains(record->key_id, "CMPS-REPE-B") ||
                timing_manifest_text_contains(record->key_id, "CMPS-REPNE-B")) {
            recipe.opcode = 0xa6u; recipe.first_ticks = 31u;
            recipe.continuation_ticks = 22u; recipe.zero_ticks = 9u;
        } else if (timing_manifest_text_contains(record->key_id, "CMPS-REPE-W") ||
                timing_manifest_text_contains(record->key_id, "CMPS-REPNE-W")) {
            recipe.opcode = 0xa7u; recipe.first_ticks = 31u;
            recipe.continuation_ticks = 22u; recipe.zero_ticks = 9u; odd_addition = 8u;
        } else if (timing_manifest_text_contains(record->key_id, "STOS-B")) {
            recipe.opcode = 0xaau; recipe.first_ticks = 19u;
            recipe.continuation_ticks = 10u; recipe.zero_ticks = 9u;
        } else if (timing_manifest_text_contains(record->key_id, "STOS-W")) {
            recipe.opcode = 0xabu; recipe.first_ticks = 19u;
            recipe.continuation_ticks = 10u; recipe.zero_ticks = 9u; odd_addition = 4u;
        } else if (timing_manifest_text_contains(record->key_id, "LODS-B")) {
            recipe.opcode = 0xacu; recipe.first_ticks = 22u;
            recipe.continuation_ticks = 13u; recipe.zero_ticks = 9u;
        } else if (timing_manifest_text_contains(record->key_id, "LODS-W")) {
            recipe.opcode = 0xadu; recipe.first_ticks = 22u;
            recipe.continuation_ticks = 13u; recipe.zero_ticks = 9u; odd_addition = 4u;
        } else if (timing_manifest_text_contains(record->key_id, "SCAS-REPE-B") ||
                timing_manifest_text_contains(record->key_id, "SCAS-REPNE-B")) {
            recipe.opcode = 0xaeu; recipe.first_ticks = 24u;
            recipe.continuation_ticks = 15u; recipe.zero_ticks = 9u;
        } else if (timing_manifest_text_contains(record->key_id, "SCAS-REPE-W") ||
                timing_manifest_text_contains(record->key_id, "SCAS-REPNE-W")) {
            recipe.opcode = 0xafu; recipe.first_ticks = 24u;
            recipe.continuation_ticks = 15u; recipe.zero_ticks = 9u; odd_addition = 4u;
        } else {
            STD_PRINTF("M5:T435:S4:I86-MANIFEST-RECIPE:FAIL:UNMAPPED-REP:%s\n",
                record->key_id);
            return 1;
        }
        segment = timing_manifest_text_contains(record->key_id, "-SEGMENT");
        odd = timing_manifest_text_contains(record->key_id, "-ODD-WORD");
        if (timing_manifest_text_contains(record->key_id, "-LOCK")) {
            recipe.first_ticks += 2u;
            recipe.continuation_ticks += 2u;
            recipe.zero_ticks += 2u;
            recipe.required_formula_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_LOCK;
        }
        if (segment) {
            recipe.segment_prefix = 0x26u;
            recipe.first_ticks += 2u;
            recipe.continuation_ticks += 2u;
            recipe.zero_ticks += 2u;
            recipe.required_formula_inputs |=
                CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE;
        }
        if (odd) {
            recipe.odd_addresses = 1;
            recipe.first_ticks += odd_addition;
            recipe.continuation_ticks += odd_addition;
            recipe.required_formula_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD;
        }
        if (timing_manifest_run_repeat_recipe(&recipe)) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_string_segment_contexts(C_VOID)
{
    static const timing_manifest_string_recipe recipes[] = {
        { "I86-STRING-MOVS-B-NONE-SEGMENT", 0xa4u, 20u },
        { "I86-STRING-MOVS-W-NONE-SEGMENT", 0xa5u, 20u },
        { "I86-STRING-CMPS-B-NONE-SEGMENT", 0xa6u, 24u },
        { "I86-STRING-CMPS-W-NONE-SEGMENT", 0xa7u, 24u },
        { "I86-STRING-LODS-B-NONE-SEGMENT", 0xacu, 14u },
        { "I86-STRING-LODS-W-NONE-SEGMENT", 0xadu, 14u }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_manifest_run_string_primitive_with_prefix(&recipes[index],
                0x26u, 0, CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE)) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_string_odd_word_contexts(C_VOID)
{
    static const timing_manifest_string_recipe recipes[] = {
        { "I86-STRING-MOVS-W-NONE-ODD-WORD", 0xa5u, 26u },
        { "I86-STRING-CMPS-W-NONE-ODD-WORD", 0xa7u, 30u },
        { "I86-STRING-STOS-W-NONE-ODD-WORD", 0xabu, 15u },
        { "I86-STRING-LODS-W-NONE-ODD-WORD", 0xadu, 16u },
        { "I86-STRING-SCAS-W-NONE-ODD-WORD", 0xafu, 19u }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_manifest_run_string_primitive_with_prefix(&recipes[index],
                0u, 1, CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD)) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_string_segment_odd_word_contexts(C_VOID)
{
    static const timing_manifest_string_recipe recipes[] = {
        { "I86-STRING-MOVS-W-NONE-SEGMENT-ODD-WORD", 0xa5u, 28u },
        { "I86-STRING-CMPS-W-NONE-SEGMENT-ODD-WORD", 0xa7u, 32u },
        { "I86-STRING-LODS-W-NONE-SEGMENT-ODD-WORD", 0xadu, 18u }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_manifest_run_string_primitive_with_prefix(&recipes[index],
                0x26u, 1, CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
                CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD)) return 1;
    }
    return 0;
}

static C_INT timing_manifest_probe_pointer_load_forms(C_VOID)
{
    typedef struct timing_manifest_pointer_recipe {
        const C_CHAR *key_id;
        type_unsigned_8 opcode;
        type_unsigned_8 prefix;
        type_unsigned_16 pointer_address;
        type_unsigned_64 expected_ticks;
        type_unsigned_32 required_formula_inputs;
    } timing_manifest_pointer_recipe;
    static const timing_manifest_pointer_recipe recipes[] = {
        { "I86-LDS-M", 0xc5u, 0u, 0x1000u, 22u,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS },
        { "I86-LES-M", 0xc4u, 0u, 0x1000u, 22u,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS },
        { "I86-LDS-M-SEGMENT", 0xc5u, 0x26u, 0x1000u, 24u,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE },
        { "I86-LES-M-SEGMENT", 0xc4u, 0x26u, 0x1000u, 24u,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE },
        { "I86-LDS-M-ODD-WORD", 0xc5u, 0u, 0x1001u, 30u,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-LES-M-ODD-WORD", 0xc4u, 0u, 0x1001u, 30u,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-LDS-M-SEGMENT-ODD-WORD", 0xc5u, 0x26u, 0x1001u, 32u,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-LES-M-SEGMENT-ODD-WORD", 0xc4u, 0x26u, 0x1001u, 32u,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-LDS-M-LOCK", 0xc5u, 0xf0u, 0x1000u, 24u,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_LOCK },
        { "I86-LES-M-LOCK", 0xc4u, 0xf0u, 0x1000u, 24u,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_LOCK },
        { "I86-LDS-M-LOCK-SEGMENT", 0xc5u, 0x26u, 0x1000u, 26u,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_LOCK },
        { "I86-LES-M-LOCK-SEGMENT", 0xc4u, 0x26u, 0x1000u, 26u,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_LOCK },
        { "I86-LDS-M-LOCK-ODD-WORD", 0xc5u, 0xf0u, 0x1001u, 32u,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD |
            CORE_MACHINE_CPU_TIMING_INPUT_LOCK },
        { "I86-LES-M-LOCK-ODD-WORD", 0xc4u, 0xf0u, 0x1001u, 32u,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD |
            CORE_MACHINE_CPU_TIMING_INPUT_LOCK },
        { "I86-LDS-M-LOCK-SEGMENT-ODD-WORD", 0xc5u, 0x26u, 0x1001u, 34u,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD |
            CORE_MACHINE_CPU_TIMING_INPUT_LOCK },
        { "I86-LES-M-LOCK-SEGMENT-ODD-WORD", 0xc4u, 0x26u, 0x1001u, 34u,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD |
            CORE_MACHINE_CPU_TIMING_INPUT_LOCK }
    };
    const type_unsigned_16 pointer[] = { 0x2000u, 0x0800u };
    const core_machine_run_budget budget = { 1u, 0u };
    STD_SIZE_T index;

    {
        const timing_manifest_record *record = timing_manifest_find("I86-LEA-M");
        const type_unsigned_8 program[] = { 0x8du, 0x1eu, 0u, 0x10u };
        timing_manifest_capture capture = { { 0 }, 0u };
        core_machine_run_result run = { 0 };
        core_machine *machine = STD_NULL;
        C_INT failed = record == STD_NULL || STD_STRCMP(record->level, "L3") != 0 ||
            !timing_manifest_prepare(&machine, &capture, program, sizeof(program));

        if (!failed) {
            failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
                run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
                run.ticks != 8u || capture.count != 1u ||
                capture.observation.timing_origin !=
                    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY ||
                (capture.observation.formula_inputs &
                    (CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
                     CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS)) !=
                    (CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
                     CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS) ||
                machine->executor_cpu.data.bx != 0x1000u;
        }
        if (failed) {
            STD_PRINTF("M5:T435:S4:I86-MANIFEST-RECIPE:FAIL:I86-LEA-M\n");
            core_machine_destroy(machine);
            return 1;
        }
        core_machine_destroy(machine);
    }
    {
        const timing_manifest_memory_recipe recipe = {
            "I86-LEA-M-LOCK", { 0xf0u, 0x8du, 0x1eu, 0u, 0x10u }, 5u, 10u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 0u, 0u, 0u, 0u
        };
        if (timing_manifest_run_l3_memory_recipe_with_inputs(&recipe,
                CORE_MACHINE_CPU_TIMING_INPUT_LOCK)) return 1;
    }

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        const timing_manifest_pointer_recipe *recipe = &recipes[index];
        const timing_manifest_record *record = timing_manifest_find(recipe->key_id);
        type_unsigned_8 program[] = { recipe->opcode, 0x1eu,
            TYPE_MASK_UNSIGNED_8(recipe->pointer_address),
            TYPE_MASK_UNSIGNED_8(recipe->pointer_address >> 8u), 0u, 0u };
        STD_SIZE_T program_bytes = 4u;
        timing_manifest_capture capture = { { 0 }, 0u };
        core_machine_run_result run = { 0 };
        core_machine *machine = STD_NULL;
        C_INT failed;

        if (recipe->prefix != 0u) {
            program[4] = program[3]; program[3] = program[2]; program[2] = program[1];
            program[1] = program[0]; program[0] = recipe->prefix;
        }
        if (STD_STRCMP(recipe->key_id, "I86-LDS-M-LOCK-SEGMENT") == 0 ||
            STD_STRCMP(recipe->key_id, "I86-LES-M-LOCK-SEGMENT") == 0 ||
            STD_STRCMP(recipe->key_id, "I86-LDS-M-LOCK-SEGMENT-ODD-WORD") == 0 ||
            STD_STRCMP(recipe->key_id, "I86-LES-M-LOCK-SEGMENT-ODD-WORD") == 0) {
            program[5] = program[4]; program[4] = program[3]; program[3] = program[2];
            program[2] = program[1]; program[1] = program[0]; program[0] = 0xf0u;
        }
        failed = record == STD_NULL || STD_STRCMP(record->level, "L3") != 0 ||
            !timing_manifest_prepare(&machine, &capture, program,
                recipe->prefix == 0u ? program_bytes :
                    (timing_manifest_text_contains(recipe->key_id, "-LOCK-SEGMENT") ?
                        program_bytes + 2u : program_bytes + 1u));
        if (!failed) {
            failed = core_machine_memory_write(machine, recipe->pointer_address, pointer,
                sizeof(pointer)) != TYPE_STATUS_OK;
        }
        if (!failed) {
            failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
                run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
                run.ticks != recipe->expected_ticks || capture.count != 1u ||
                capture.observation.timing_origin !=
                    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY ||
                (capture.observation.formula_inputs & recipe->required_formula_inputs) !=
                    recipe->required_formula_inputs ||
                machine->executor_cpu.data.bx != 0x2000u ||
                (recipe->opcode == 0xc5u &&
                    machine->executor_cpu.data.ds.selector != 0x0800u) ||
                (recipe->opcode == 0xc4u &&
                    machine->executor_cpu.data.es.selector != 0x0800u);
        }
        if (failed) {
            STD_PRINTF("M5:T435:S4:I86-MANIFEST-RECIPE:FAIL:%s\n", recipe->key_id);
            core_machine_destroy(machine);
            return 1;
        }
        core_machine_destroy(machine);
    }
    return 0;
}

static C_INT timing_manifest_probe_segment_mov_forms(C_VOID)
{
    static const timing_manifest_recipe register_recipes[] = {
        { "I86-MOV-SREG-TO-R", { 0x8cu, 0xc0u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-MOV-SREG-FROM-R", { 0x8eu, 0xc0u }, 2u, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY }
    };
    static const timing_manifest_memory_recipe memory_recipes[] = {
        { "I86-MOV-SREG-TO-M", { 0x8cu, 0x06u, 0u, 0x10u }, 4u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 0u, 0u, 0u, 0u },
        { "I86-MOV-SREG-FROM-M", { 0x8eu, 0x06u, 0u, 0x10u }, 4u, 14u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 0u, 0u, 0u, 0u, 0u }
    };
    static const C_CHAR *const context_keys[][2] = {
        { "I86-MOV-SREG-TO-M-SEGMENT", "I86-MOV-SREG-FROM-M-SEGMENT" },
        { "I86-MOV-SREG-TO-M-ODD-WORD", "I86-MOV-SREG-FROM-M-ODD-WORD" },
        { "I86-MOV-SREG-TO-M-SEGMENT-ODD-WORD",
            "I86-MOV-SREG-FROM-M-SEGMENT-ODD-WORD" }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(register_recipes) / sizeof(register_recipes[0]);
            ++index) {
        if (timing_manifest_run_exact_recipe(&register_recipes[index])) return 1;
    }
    for (index = 0u; index < sizeof(memory_recipes) / sizeof(memory_recipes[0]);
            ++index) {
        if (timing_manifest_run_l3_memory_recipe(&memory_recipes[index])) return 1;
    }
    for (index = 0u; index < 3u; ++index) {
        C_INT segment_override = index == 0u || index == 2u;
        C_INT odd_word = index == 1u || index == 2u;
        STD_SIZE_T recipe_index;

        for (recipe_index = 0u; recipe_index < sizeof(memory_recipes) /
                sizeof(memory_recipes[0]); ++recipe_index) {
            timing_manifest_memory_recipe recipe = memory_recipes[recipe_index];
            STD_SIZE_T byte_index;

            recipe.key_id = context_keys[index][recipe_index];
            if (segment_override) {
                for (byte_index = recipe.program_bytes; byte_index > 0u;
                        --byte_index) {
                    recipe.program[byte_index] = recipe.program[byte_index - 1u];
                }
                recipe.program[0] = 0x26u;
                ++recipe.program_bytes;
                recipe.expected_ticks += 2u;
            }
            if (odd_word) {
                recipe.program[segment_override ? 3u : 2u] = 1u;
                recipe.expected_ticks += 4u;
            }
            if (timing_manifest_run_l3_memory_recipe_with_inputs(&recipe,
                    odd_word ? CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD : 0u)) return 1;
        }
    }
    return 0;
}

static C_INT timing_manifest_probe_far_direct_control_transfers(C_VOID)
{
    static const timing_manifest_recipe recipes[] = {
        { "I86-CALL-FAR", { 0x9au, 0u, 0u, 0u, 0u }, 5u, 28u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I86-JMP-FAR", { 0xeau, 0u, 0u, 0u, 0u }, 5u, 15u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }
    };

    return timing_manifest_run_exact_recipe(&recipes[0]) ||
        timing_manifest_run_exact_recipe_with_control(&recipes[1], 0u, 0u,
            CORE_MACHINE_RETIREMENT_CONTROL_TAKEN);
}

typedef struct timing_manifest_indirect_control_recipe {
    const C_CHAR *key_id;
    type_unsigned_8 program[8];
    type_unsigned_8 program_bytes;
    type_unsigned_64 expected_ticks;
    type_unsigned_16 target_ip;
    type_unsigned_16 target_cs;
    type_unsigned_16 expected_sp;
    type_unsigned_16 pointer_address;
    C_INT pointer_is_far;
    type_unsigned_32 required_formula_inputs;
} timing_manifest_indirect_control_recipe;

static C_INT timing_manifest_write_word(core_machine *machine,
    type_unsigned_32 address, type_unsigned_16 value)
{
    type_unsigned_8 bytes[2];

    bytes[0] = TYPE_MASK_UNSIGNED_8(value);
    bytes[1] = TYPE_MASK_UNSIGNED_8(value >> 8u);
    return core_machine_memory_write(machine, address, bytes, sizeof(bytes)) ==
        TYPE_STATUS_OK;
}

static C_INT timing_manifest_run_indirect_control_recipe(
    const timing_manifest_indirect_control_recipe *recipe)
{
    const core_machine_run_budget budget = { 1u, 0u };
    const timing_manifest_record *record = recipe == STD_NULL ? STD_NULL :
        timing_manifest_find(recipe->key_id);
    timing_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_INT failed = recipe == STD_NULL || record == STD_NULL ||
        !timing_manifest_is_i86(record) || STD_STRCMP(record->level, "L3") != 0 ||
        !timing_manifest_prepare(&machine, &capture, recipe->program,
            recipe->program_bytes);

    if (!failed) {
        machine->executor_cpu.data.ax = recipe->target_ip;
        machine->executor_cpu.data.sp = 0x8000u;
        if (recipe->pointer_address != 0u) {
            failed = !timing_manifest_write_word(machine, recipe->pointer_address,
                recipe->target_ip);
            if (!failed && recipe->pointer_is_far) {
                failed = !timing_manifest_write_word(machine,
                    recipe->pointer_address + 2u, recipe->target_cs);
            }
        }
    }
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != recipe->expected_ticks || capture.count != 1u ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            capture.observation.source_timing_form_id ==
                CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
            capture.observation.timing_key_id ==
                CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
            (recipe->required_formula_inputs != 0u &&
                (capture.observation.formula_inputs &
                    recipe->required_formula_inputs) !=
                    recipe->required_formula_inputs) ||
            machine->executor_cpu.data.eip != recipe->target_ip ||
            (recipe->target_cs != 0u &&
                machine->executor_cpu.data.cs.selector != recipe->target_cs) ||
            machine->executor_cpu.data.sp != recipe->expected_sp;
    }
    if (failed) {
        STD_PRINTF("I86 indirect ticks=%llu origin=%d ip=%u cs=%u sp=%u form=%u key=%u\n",
            run.ticks, capture.observation.timing_origin,
            machine == STD_NULL ? 0u : machine->executor_cpu.data.eip,
            machine == STD_NULL ? 0u : machine->executor_cpu.data.cs.selector,
            machine == STD_NULL ? 0u : machine->executor_cpu.data.sp,
            capture.observation.source_timing_form_id,
            capture.observation.timing_key_id);
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-RECIPE:FAIL:%s\n", recipe->key_id);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_manifest_probe_indirect_control_transfers(C_VOID)
{
    static const timing_manifest_indirect_control_recipe recipes[] = {
        { "I86-CALL-RM16", { 0xffu, 0xd0u }, 2u, 16u, 0x0200u, 0u, 0x7ffeu,
            0u, 0, 0u },
        { "I86-CALL-M1616", { 0xffu, 0x1eu, 0u, 0x10u }, 4u, 43u,
            0x0200u, 0xf000u, 0x7ffcu, 0x1000u, 1,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS },
        { "I86-JMP-RM16", { 0xffu, 0xe0u }, 2u, 11u, 0x0200u, 0u, 0x8000u,
            0u, 0, 0u },
        { "I86-JMP-M1616", { 0xffu, 0x2eu, 0u, 0x10u }, 4u, 30u,
            0x0200u, 0xf000u, 0x8000u, 0x1000u, 1,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS },
        { "I86-CALL-RM16-SEGMENT", { 0x26u, 0xffu, 0x16u, 0u, 0x10u }, 5u,
            29u, 0x0200u, 0u, 0x7ffeu, 0x1000u, 0,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE },
        { "I86-CALL-M1616-SEGMENT", { 0x26u, 0xffu, 0x1eu, 0u, 0x10u }, 5u,
            45u, 0x0200u, 0xf000u, 0x7ffcu, 0x1000u, 1,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE },
        { "I86-JMP-RM16-SEGMENT", { 0x26u, 0xffu, 0x26u, 0u, 0x10u }, 5u,
            26u, 0x0200u, 0u, 0x8000u, 0x1000u, 0,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE },
        { "I86-JMP-M1616-SEGMENT", { 0x26u, 0xffu, 0x2eu, 0u, 0x10u }, 5u,
            32u, 0x0200u, 0xf000u, 0x8000u, 0x1000u, 1,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE },
        { "I86-CALL-RM16-ODD-WORD", { 0xffu, 0x16u, 1u, 0x10u }, 4u, 31u,
            0x0200u, 0u, 0x7ffeu, 0x1001u, 0,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-CALL-M1616-ODD-WORD", { 0xffu, 0x1eu, 1u, 0x10u }, 4u, 51u,
            0x0200u, 0xf000u, 0x7ffcu, 0x1001u, 1,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-JMP-RM16-ODD-WORD", { 0xffu, 0x26u, 1u, 0x10u }, 4u, 28u,
            0x0200u, 0u, 0x8000u, 0x1001u, 0,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-JMP-M1616-ODD-WORD", { 0xffu, 0x2eu, 1u, 0x10u }, 4u, 38u,
            0x0200u, 0xf000u, 0x8000u, 0x1001u, 1,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-CALL-RM16-SEGMENT-ODD-WORD", { 0x26u, 0xffu, 0x16u, 1u, 0x10u },
            5u, 33u, 0x0200u, 0u, 0x7ffeu, 0x1001u, 0,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-CALL-M1616-SEGMENT-ODD-WORD", { 0x26u, 0xffu, 0x1eu, 1u, 0x10u },
            5u, 53u, 0x0200u, 0xf000u, 0x7ffcu, 0x1001u, 1,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-JMP-RM16-SEGMENT-ODD-WORD", { 0x26u, 0xffu, 0x26u, 1u, 0x10u },
            5u, 30u, 0x0200u, 0u, 0x8000u, 0x1001u, 0,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-JMP-M1616-SEGMENT-ODD-WORD", { 0x26u, 0xffu, 0x2eu, 1u, 0x10u },
            5u, 40u, 0x0200u, 0xf000u, 0x8000u, 0x1001u, 1,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_manifest_run_indirect_control_recipe(&recipes[index])) return 1;
        {
            timing_manifest_indirect_control_recipe locked = recipes[index];
            const timing_manifest_record *base_record =
                timing_manifest_find(locked.key_id);
            C_CHAR key[160];
            STD_SIZE_T byte_index;

            if (base_record != STD_NULL &&
                !timing_manifest_text_contains(base_record->context, "LOCK")) {
                if (locked.program_bytes >= sizeof(locked.program) ||
                    !timing_manifest_lock_key_for_context(base_record, locked.key_id,
                        key, sizeof(key))) return 1;
                for (byte_index = locked.program_bytes; byte_index != 0u;
                        --byte_index) {
                    locked.program[byte_index] = locked.program[byte_index - 1u];
                }
                locked.program[0] = 0xf0u;
                ++locked.program_bytes;
                locked.expected_ticks += 2u;
                locked.required_formula_inputs |= CORE_MACHINE_CPU_TIMING_INPUT_LOCK;
                locked.key_id = key;
                if (timing_manifest_run_indirect_control_recipe(&locked)) return 1;
            }
        }
    }
    return 0;
}

typedef struct timing_manifest_return_recipe {
    const C_CHAR *key_id;
    type_unsigned_8 program[4];
    type_unsigned_8 program_bytes;
    type_unsigned_64 expected_ticks;
    type_unsigned_16 target_ip;
    type_unsigned_16 target_cs;
    type_unsigned_16 expected_sp;
    type_unsigned_16 frame_words[3];
    type_unsigned_8 frame_word_count;
    core_machine_retirement_timing_origin expected_origin;
} timing_manifest_return_recipe;

static C_INT timing_manifest_probe_return_forms(C_VOID)
{
    static const timing_manifest_return_recipe recipes[] = {
        { "I86-RET-NEAR", { 0xc3u }, 1u, 8u, 0x0200u, 0u, 0x8002u,
            { 0x0200u, 0u, 0u }, 1u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I86-RET-NEAR-IMM", { 0xc2u, 4u, 0u }, 3u, 12u, 0x0200u, 0u,
            0x8006u, { 0x0200u, 0u, 0u }, 1u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I86-RET-FAR", { 0xcbu }, 1u, 18u, 0x0200u, 0xf000u, 0x8004u,
            { 0x0200u, 0xf000u, 0u }, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-RET-FAR-IMM", { 0xcau, 4u, 0u }, 3u, 17u, 0x0200u, 0xf000u,
            0x8008u, { 0x0200u, 0xf000u, 0u }, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-RET-IRET", { 0xcfu }, 1u, 24u, 0x0200u, 0xf000u, 0x8006u,
            { 0x0200u, 0xf000u, 0x0002u }, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I86-RET-NEAR-LOCK", { 0xf0u, 0xc3u }, 2u, 10u, 0x0200u, 0u,
            0x8002u, { 0x0200u, 0u, 0u }, 1u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I86-RET-NEAR-IMM-LOCK", { 0xf0u, 0xc2u, 4u, 0u }, 4u, 14u,
            0x0200u, 0u, 0x8006u, { 0x0200u, 0u, 0u }, 1u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I86-RET-FAR-LOCK", { 0xf0u, 0xcbu }, 2u, 20u, 0x0200u, 0xf000u,
            0x8004u, { 0x0200u, 0xf000u, 0u }, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-RET-FAR-IMM-LOCK", { 0xf0u, 0xcau, 4u, 0u }, 4u, 19u,
            0x0200u, 0xf000u, 0x8008u, { 0x0200u, 0xf000u, 0u }, 2u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I86-RET-IRET-LOCK", { 0xf0u, 0xcfu }, 2u, 26u, 0x0200u, 0xf000u,
            0x8006u, { 0x0200u, 0xf000u, 0x0002u }, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        const timing_manifest_return_recipe *recipe = &recipes[index];
        const timing_manifest_record *record = timing_manifest_find(recipe->key_id);
        timing_manifest_capture capture = { { 0 }, 0u };
        core_machine_run_result run = { 0 };
        core_machine *machine = STD_NULL;
        type_unsigned_8 word_index;
        C_INT failed = record == STD_NULL || !timing_manifest_is_i86(record) ||
            STD_STRCMP(record->level, "L3") != 0 ||
            !timing_manifest_prepare(&machine, &capture, recipe->program,
                recipe->program_bytes);

        if (!failed) {
            machine->executor_cpu.data.sp = 0x8000u;
            for (word_index = 0u; word_index < recipe->frame_word_count; ++word_index) {
                failed |= !timing_manifest_write_word(machine,
                    0x8000u + (type_unsigned_32)word_index * 2u,
                    recipe->frame_words[word_index]);
            }
        }
        if (!failed) {
            failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
                run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
                run.ticks != recipe->expected_ticks || capture.count != 1u ||
                capture.observation.timing_disposition !=
                    CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
                capture.observation.timing_origin != recipe->expected_origin ||
                capture.observation.source_timing_form_id ==
                    CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
                capture.observation.timing_key_id ==
                    CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
                machine->executor_cpu.data.eip != recipe->target_ip ||
                (recipe->target_cs != 0u &&
                    machine->executor_cpu.data.cs.selector != recipe->target_cs) ||
                machine->executor_cpu.data.sp != recipe->expected_sp;
        }
        if (failed) {
            STD_PRINTF("I86 return ticks=%llu origin=%d ip=%u cs=%u sp=%u form=%u key=%u\n",
                run.ticks, capture.observation.timing_origin,
                machine == STD_NULL ? 0u : machine->executor_cpu.data.eip,
                machine == STD_NULL ? 0u : machine->executor_cpu.data.cs.selector,
                machine == STD_NULL ? 0u : machine->executor_cpu.data.sp,
                capture.observation.source_timing_form_id,
                capture.observation.timing_key_id);
            STD_PRINTF("M5:T435:S4:I86-MANIFEST-RECIPE:FAIL:%s\n", recipe->key_id);
            core_machine_destroy(machine);
            return 1;
        }
        core_machine_destroy(machine);
    }
    return 0;
}

typedef struct timing_manifest_interrupt_recipe {
    const C_CHAR *key_id;
    type_unsigned_8 program[3];
    type_unsigned_8 program_bytes;
    type_unsigned_32 initial_eflags;
    type_unsigned_16 vector;
    type_unsigned_64 expected_ticks;
    type_unsigned_16 expected_ip;
    type_unsigned_16 expected_sp;
} timing_manifest_interrupt_recipe;

static C_INT timing_manifest_probe_software_interrupt_forms(C_VOID)
{
    static const timing_manifest_interrupt_recipe recipes[] = {
        { "I86-INT3", { 0xccu, 0u }, 1u, 0u, 3u, 52u, 0x0200u, 0x7ffau },
        { "I86-INT-IMM", { 0xcdu, 4u }, 2u, 0u, 4u, 51u, 0x0200u, 0x7ffau },
        { "I86-INTO-TAKEN", { 0xceu, 0u }, 1u, VCPU_EFLAGS_OF, 4u, 53u,
            0x0200u, 0x7ffau },
        { "I86-INTO-NOT", { 0xceu, 0u }, 1u, 0u, 0u, 4u, 0xfff1u, 0x8000u },
        { "I86-INT3-LOCK", { 0xf0u, 0xccu }, 2u, 0u, 3u, 54u, 0x0200u,
            0x7ffau },
        { "I86-INT-IMM-LOCK", { 0xf0u, 0xcdu, 4u }, 3u, 0u, 4u, 53u,
            0x0200u, 0x7ffau },
        { "I86-INTO-TAKEN-LOCK", { 0xf0u, 0xceu }, 2u, VCPU_EFLAGS_OF,
            4u, 55u, 0x0200u, 0x7ffau },
        { "I86-INTO-NOT-LOCK", { 0xf0u, 0xceu }, 2u, 0u, 0u, 6u, 0xfff2u,
            0x8000u }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        const timing_manifest_interrupt_recipe *recipe = &recipes[index];
        const timing_manifest_record *record = timing_manifest_find(recipe->key_id);
        timing_manifest_capture capture = { { 0 }, 0u };
        core_machine_run_result run = { 0 };
        core_machine *machine = STD_NULL;
        C_INT failed = record == STD_NULL || !timing_manifest_is_i86(record) ||
            STD_STRCMP(record->level, "L3") != 0 ||
            !timing_manifest_prepare(&machine, &capture, recipe->program,
                recipe->program_bytes);

        if (!failed) {
            machine->executor_cpu.data.sp = 0x8000u;
            machine->executor_cpu.data.eflags = recipe->initial_eflags;
            if (recipe->vector != 0u) {
                failed = !timing_manifest_write_word(machine,
                    (type_unsigned_32)recipe->vector * 4u, recipe->expected_ip) ||
                    !timing_manifest_write_word(machine,
                        (type_unsigned_32)recipe->vector * 4u + 2u, 0xf000u);
            }
        }
        if (!failed) {
            failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
                run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
                run.ticks != recipe->expected_ticks || capture.count != 1u ||
                capture.observation.timing_disposition !=
                    CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
                capture.observation.timing_origin !=
                    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
                capture.observation.source_timing_form_id ==
                    CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
                capture.observation.timing_key_id ==
                    CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
                machine->executor_cpu.data.eip != recipe->expected_ip ||
                machine->executor_cpu.data.sp != recipe->expected_sp ||
                (recipe->vector != 0u &&
                    machine->executor_cpu.data.cs.selector != 0xf000u);
        }
        if (failed) {
            STD_PRINTF("I86 int ticks=%llu origin=%d ip=%u cs=%u sp=%u control=%d form=%u key=%u\n",
                run.ticks, capture.observation.timing_origin,
                machine == STD_NULL ? 0u : machine->executor_cpu.data.eip,
                machine == STD_NULL ? 0u : machine->executor_cpu.data.cs.selector,
                machine == STD_NULL ? 0u : machine->executor_cpu.data.sp,
                capture.observation.control_outcome,
                capture.observation.source_timing_form_id,
                capture.observation.timing_key_id);
            STD_PRINTF("M5:T435:S4:I86-MANIFEST-RECIPE:FAIL:%s\n", recipe->key_id);
            core_machine_destroy(machine);
            return 1;
        }
        core_machine_destroy(machine);
    }
    return 0;
}

typedef struct timing_manifest_memory_stack_recipe {
    const C_CHAR *key_id;
    type_unsigned_8 program[6];
    type_unsigned_8 program_bytes;
    type_unsigned_16 memory_address;
    type_unsigned_64 expected_ticks;
    C_INT push;
    type_unsigned_32 required_formula_inputs;
} timing_manifest_memory_stack_recipe;

static C_INT timing_manifest_probe_memory_stack_forms(C_VOID)
{
    static const timing_manifest_memory_stack_recipe recipes[] = {
        { "I86-PUSH-M", { 0xffu, 0x36u, 0u, 0x10u }, 4u, 0x1000u, 22u, 1,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS },
        { "I86-POP-M", { 0x8fu, 0x06u, 0u, 0x10u }, 4u, 0x1000u, 23u, 0,
            CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS },
        { "I86-PUSH-M-SEGMENT", { 0x26u, 0xffu, 0x36u, 0u, 0x10u }, 5u,
            0x1000u, 24u, 1, CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE },
        { "I86-POP-M-SEGMENT", { 0x26u, 0x8fu, 0x06u, 0u, 0x10u }, 5u,
            0x1000u, 25u, 0, CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE },
        { "I86-PUSH-M-ODD-WORD", { 0xffu, 0x36u, 1u, 0x10u }, 4u, 0x1001u,
            26u, 1, CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-POP-M-ODD-WORD", { 0x8fu, 0x06u, 1u, 0x10u }, 4u, 0x1001u,
            27u, 0, CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-PUSH-M-SEGMENT-ODD-WORD", { 0x26u, 0xffu, 0x36u, 1u, 0x10u },
            5u, 0x1001u, 28u, 1, CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-POP-M-SEGMENT-ODD-WORD", { 0x26u, 0x8fu, 0x06u, 1u, 0x10u },
            5u, 0x1001u, 29u, 0, CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD },
        { "I86-PUSH-M-LOCK", { 0xf0u, 0xffu, 0x36u, 0u, 0x10u }, 5u,
            0x1000u, 24u, 1, CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_LOCK },
        { "I86-POP-M-LOCK", { 0xf0u, 0x8fu, 0x06u, 0u, 0x10u }, 5u,
            0x1000u, 25u, 0, CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_LOCK },
        { "I86-PUSH-M-LOCK-SEGMENT", { 0xf0u, 0x26u, 0xffu, 0x36u, 0u, 0x10u }, 6u,
            0x1000u, 26u, 1, CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_LOCK },
        { "I86-POP-M-LOCK-SEGMENT", { 0xf0u, 0x26u, 0x8fu, 0x06u, 0u, 0x10u }, 6u,
            0x1000u, 27u, 0, CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_LOCK },
        { "I86-PUSH-M-LOCK-ODD-WORD", { 0xf0u, 0xffu, 0x36u, 1u, 0x10u }, 5u,
            0x1001u, 28u, 1, CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD |
            CORE_MACHINE_CPU_TIMING_INPUT_LOCK },
        { "I86-POP-M-LOCK-ODD-WORD", { 0xf0u, 0x8fu, 0x06u, 1u, 0x10u }, 5u,
            0x1001u, 29u, 0, CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD |
            CORE_MACHINE_CPU_TIMING_INPUT_LOCK },
        { "I86-PUSH-M-LOCK-SEGMENT-ODD-WORD", { 0xf0u, 0x26u, 0xffu, 0x36u, 1u, 0x10u }, 6u,
            0x1001u, 30u, 1, CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD |
            CORE_MACHINE_CPU_TIMING_INPUT_LOCK },
        { "I86-POP-M-LOCK-SEGMENT-ODD-WORD", { 0xf0u, 0x26u, 0x8fu, 0x06u, 1u, 0x10u }, 6u,
            0x1001u, 31u, 0, CORE_MACHINE_CPU_TIMING_INPUT_MODRM |
            CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD |
            CORE_MACHINE_CPU_TIMING_INPUT_LOCK }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_16 transfer_word = 0x4a3cu;
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        const timing_manifest_memory_stack_recipe *recipe = &recipes[index];
        const timing_manifest_record *record = timing_manifest_find(recipe->key_id);
        timing_manifest_capture capture = { { 0 }, 0u };
        core_machine_run_result run = { 0 };
        core_machine *machine = STD_NULL;
        type_unsigned_16 observed = 0u;
        C_INT failed = record == STD_NULL || !timing_manifest_is_i86(record) ||
            STD_STRCMP(record->level, "L3") != 0 ||
            !timing_manifest_prepare(&machine, &capture, recipe->program,
                recipe->program_bytes);

        if (!failed) {
            machine->executor_cpu.data.sp = 0x8000u;
            failed = !timing_manifest_write_word(machine,
                recipe->push ? recipe->memory_address : 0x8000u, transfer_word);
        }
        if (!failed) {
            failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
                run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
                run.ticks != recipe->expected_ticks || capture.count != 1u ||
                capture.observation.timing_disposition !=
                    CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
                capture.observation.timing_origin !=
                    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
                (capture.observation.formula_inputs & recipe->required_formula_inputs) !=
                    recipe->required_formula_inputs ||
                machine->executor_cpu.data.sp != (recipe->push ? 0x7ffeu : 0x8002u) ||
                core_machine_memory_read(machine,
                    recipe->push ? 0x7ffeu : recipe->memory_address,
                    &observed, sizeof(observed)) != TYPE_STATUS_OK ||
                observed != transfer_word;
        }
        if (failed) {
            STD_PRINTF("M5:T435:S4:I86-MANIFEST-RECIPE:FAIL:%s\n", recipe->key_id);
            core_machine_destroy(machine);
            return 1;
        }
        core_machine_destroy(machine);
    }
    return 0;
}

static C_INT timing_manifest_probe_hlt(C_VOID)
{
    const timing_manifest_record *record = timing_manifest_find("I86-FLAG-HLT");
    const type_unsigned_8 program[] = { 0xf4u };
    const core_machine_run_budget budget = { 1u, 0u };
    timing_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_INT failed = record == STD_NULL || !timing_manifest_is_i86(record) ||
        STD_STRCMP(record->level, "L3") != 0 ||
        !timing_manifest_prepare(&machine, &capture, program, sizeof(program));

    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            run.executed != 1u || run.ticks != 2u || capture.count != 1u ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            !machine->executor_cpu.data.flagHalt;
    }
    if (failed) STD_PRINTF("M5:T435:S4:I86-MANIFEST-RECIPE:FAIL:I86-FLAG-HLT\n");
    core_machine_destroy(machine);
    if (failed) return 1;
    {
        const timing_manifest_record *locked_record =
            timing_manifest_find("I86-FLAG-HLT-LOCK");
        const type_unsigned_8 locked_program[] = { 0xf0u, 0xf4u };
        timing_manifest_capture locked_capture = { { 0 }, 0u };
        core_machine_run_result locked_run = { 0 };
        core_machine *locked_machine = STD_NULL;
        C_INT locked_failed = locked_record == STD_NULL ||
            !timing_manifest_prepare(&locked_machine, &locked_capture,
                locked_program, sizeof(locked_program));

        if (!locked_failed) {
            locked_failed = core_machine_run(locked_machine, budget, &locked_run) !=
                    TYPE_STATUS_OK ||
                locked_run.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
                locked_run.executed != 1u || locked_run.ticks != 4u ||
                locked_capture.count != 1u ||
                locked_capture.observation.timing_disposition !=
                    CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
                locked_capture.observation.timing_origin !=
                    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
                (locked_capture.observation.formula_inputs &
                    CORE_MACHINE_CPU_TIMING_INPUT_LOCK) == 0u ||
                !locked_machine->executor_cpu.data.flagHalt;
        }
        core_machine_destroy(locked_machine);
        if (locked_failed) {
            STD_PRINTF("M5:T435:S5:I86-MANIFEST-RECIPE:FAIL:I86-FLAG-HLT-LOCK\n");
        }
        return locked_failed;
    }
}

static C_INT timing_manifest_probe_xchg_memory_contexts(C_VOID)
{
    static const timing_manifest_memory_recipe recipes[] = {
        { "I86-XCHG-MR-SEGMENT", { 0x26u, 0x87u, 0x0eu, 0u, 0x10u }, 5u,
            25u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 1u, 2u, 0u, 2u, 1u },
        { "I86-XCHG-MR-ODD-WORD", { 0x87u, 0x0eu, 1u, 0x10u }, 4u,
            31u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 1u, 2u, 0u, 2u, 1u },
        { "I86-XCHG-MR-LOCK-SEGMENT", { 0xf0u, 0x26u, 0x87u, 0x0eu, 0u, 0x10u },
            6u, 27u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 1u, 2u, 0u, 2u, 1u },
        { "I86-XCHG-MR-LOCK-ODD-WORD", { 0xf0u, 0x87u, 0x0eu, 1u, 0x10u },
            5u, 33u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 1u, 2u, 0u, 2u, 1u },
        { "I86-XCHG-MR-SEGMENT-ODD-WORD", { 0x26u, 0x87u, 0x0eu, 1u, 0x10u },
            5u, 33u, CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 1u, 2u, 0u, 2u, 1u },
        { "I86-XCHG-MR-LOCK-SEGMENT-ODD-WORD",
            { 0xf0u, 0x26u, 0x87u, 0x0eu, 1u, 0x10u }, 6u, 35u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
            0u, 1u, 2u, 0u, 2u, 1u }
    };
    static const type_unsigned_32 required_inputs[] = {
        CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE,
        CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD,
        CORE_MACHINE_CPU_TIMING_INPUT_LOCK |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE,
        CORE_MACHINE_CPU_TIMING_INPUT_LOCK |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD,
        CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD,
        CORE_MACHINE_CPU_TIMING_INPUT_LOCK |
            CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        if (timing_manifest_run_l3_memory_recipe_with_inputs(&recipes[index],
                required_inputs[index])) return 1;
    }
    return 0;
}

static C_INT timing_manifest_write_results(C_VOID)
{
    const C_CHAR *const path =
        "docs/etc/cpu-timing/t435-s5-8086-timing-results.json";
    STD_FILE *file = STD_FOPEN(path, "wb");
    STD_SIZE_T index;
    STD_SIZE_T written = 0u;

    if (file == STD_NULL) return 1;
    if (STD_FPRINTF(file, "{\n  \"schema\": \"nxvm.cpu-timing-results.v1\",\n"
            "  \"profile\": \"8086\",\n  \"results\": [\n") < 0) {
        STD_FCLOSE(file);
        return 1;
    }
    for (index = 0u; index < sizeof(timing_manifest_records) /
            sizeof(timing_manifest_records[0]); ++index) {
        const timing_manifest_record *record = &timing_manifest_records[index];
        const core_machine_retirement_observation *observation =
            &timing_manifest_results[index];

        if (!timing_manifest_is_i86(record)) continue;
        if (!timing_manifest_observed[index]) {
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
                observation->source_timing_form_id ==
                    CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ?
                    "true" : "false") < 0) {
            STD_FCLOSE(file);
            return 1;
        }
        ++written;
    }
    if (STD_FPRINTF(file, "\n  ]\n}\n") < 0 || STD_FCLOSE(file) != 0) return 1;
    return written == 1053u ? 0 : 1;
}

C_INT main(C_VOID)
{
    STD_SIZE_T index;
    STD_SIZE_T i86_count = 0u;
    STD_SIZE_T covered_count = 0u;

    for (index = 0u; index < sizeof(timing_manifest_records) /
            sizeof(timing_manifest_records[0]); ++index) {
        const timing_manifest_record *record = &timing_manifest_records[index];

        if (!timing_manifest_is_i86(record)) continue;
        if (STD_STRCMP(record->profile, "8086") != 0 ||
            record->level[0] == '\0' || record->source_rule[0] == '\0' ||
            record->context[0] == '\0') return 1;
        ++i86_count;
    }
    if (i86_count != 1053u || timing_manifest_probe_decoder_lexeme_candidates() ||
            timing_manifest_probe_decoder_form_rejections() ||
            timing_manifest_probe_pop_cs_function() ||
            timing_manifest_probe_alu_function() ||
            timing_manifest_probe_adjustment_function() ||
            timing_manifest_probe_data_stack_function() ||
            timing_manifest_probe_group3_function() ||
            timing_manifest_probe_branch_function() ||
            timing_manifest_probe_flag_function() ||
            timing_manifest_probe_compare_function() ||
            timing_manifest_probe_unary_function() ||
            timing_manifest_probe_lahf_sahf_function() ||
            timing_manifest_probe_general_lock_prefix() ||
            timing_manifest_probe_adjustments()) return 1;
    if (timing_manifest_probe_alu_register_forms()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:ALU-REGISTER\n");
        return 1;
    }
    if (timing_manifest_probe_compare_and_test_register_forms()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:CMP-TEST-REGISTER\n");
        return 1;
    }
    if (timing_manifest_probe_register_data_forms()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:REGISTER-DATA\n");
        return 1;
    }
    if (timing_manifest_probe_stack_register_forms()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:STACK-REGISTER\n");
        return 1;
    }
    if (timing_manifest_probe_conditional_branches()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:JCC\n");
        return 1;
    }
    if (timing_manifest_probe_counted_branches()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:LOOP\n");
        return 1;
    }
    if (timing_manifest_probe_wait_and_escape()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:WAIT-ESC\n");
        return 1;
    }
    if (timing_manifest_probe_escape_memory_forms()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:ESC-MEMORY\n");
        return 1;
    }
    if (timing_manifest_probe_group2_register_one()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:GROUP2-R1\n");
        return 1;
    }
    if (timing_manifest_probe_group2_register_cl()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:GROUP2-RCL\n");
        return 1;
    }
    if (timing_manifest_probe_immediate_port_io()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:PORT-IMM\n");
        return 1;
    }
    if (timing_manifest_probe_dx_port_io()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:PORT-DX\n");
        return 1;
    }
    if (timing_manifest_probe_near_control_transfers()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:NEAR-CONTROL\n");
        return 1;
    }
    if (timing_manifest_probe_group3_l2()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:GROUP3-L2\n");
        return 1;
    }
    if (timing_manifest_probe_group3_memory_contexts()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:GROUP3-CONTEXT\n");
        return 1;
    }
    if (timing_manifest_probe_alu_memory_forms()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:ALU-MEMORY\n");
        return 1;
    }
    if (timing_manifest_probe_primary_memory_forms()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:PRIMARY-MEMORY\n");
        return 1;
    }
    if (timing_manifest_probe_alu_segment_contexts()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:ALU-SEGMENT\n");
        return 1;
    }
    if (timing_manifest_probe_alu_odd_word_contexts()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:ALU-ODD-WORD\n");
        return 1;
    }
    if (timing_manifest_probe_lock_contexts()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:LOCK\n");
        return 1;
    }
    if (timing_manifest_probe_alu_segment_odd_word_contexts()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:ALU-SEGMENT-ODD-WORD\n");
        return 1;
    }
    if (timing_manifest_probe_group2_memory_forms()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:GROUP2-MEMORY\n");
        return 1;
    }
    if (timing_manifest_probe_group2_segment_contexts()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:GROUP2-SEGMENT\n");
        return 1;
    }
    if (timing_manifest_probe_group2_odd_word_contexts()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:GROUP2-ODD-WORD\n");
        return 1;
    }
    if (timing_manifest_probe_group2_segment_odd_word_contexts()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:GROUP2-SEGMENT-ODD\n");
        return 1;
    }
    if (timing_manifest_probe_string_primitives()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:STRING-PRIMITIVE\n");
        return 1;
    }
    if (timing_manifest_probe_repeat_base_forms()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:REP-BASE\n");
        return 1;
    }
    if (timing_manifest_probe_repeat_manifest_contexts()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:REP-CONTEXT\n");
        return 1;
    }
    if (timing_manifest_probe_string_segment_contexts()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:STRING-SEGMENT\n");
        return 1;
    }
    if (timing_manifest_probe_string_odd_word_contexts()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:STRING-ODD-WORD\n");
        return 1;
    }
    if (timing_manifest_probe_string_segment_odd_word_contexts()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:STRING-SEGMENT-ODD\n");
        return 1;
    }
    if (timing_manifest_probe_pointer_load_forms()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:POINTER-LOAD\n");
        return 1;
    }
    if (timing_manifest_probe_segment_mov_forms()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:MOV-SREG\n");
        return 1;
    }
    if (timing_manifest_probe_far_direct_control_transfers()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:FAR-DIRECT\n");
        return 1;
    }
    if (timing_manifest_probe_indirect_control_transfers()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:INDIRECT-CONTROL\n");
        return 1;
    }
    if (timing_manifest_probe_return_forms()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:RETURN\n");
        return 1;
    }
    if (timing_manifest_probe_software_interrupt_forms()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:SOFTWARE-INT\n");
        return 1;
    }
    if (timing_manifest_probe_memory_stack_forms()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:MEMORY-STACK\n");
        return 1;
    }
    if (timing_manifest_probe_hlt()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:HLT\n");
        return 1;
    }
    if (timing_manifest_probe_xchg_memory_contexts()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:XCHG-CONTEXT\n");
        return 1;
    }
    for (index = 0u; index < sizeof(timing_manifest_records) /
            sizeof(timing_manifest_records[0]); ++index) {
        if (timing_manifest_is_i86(&timing_manifest_records[index]) &&
                timing_manifest_covered[index]) ++covered_count;
        else if (timing_manifest_is_i86(&timing_manifest_records[index])) {
            STD_PRINTF("M5:T435:S4:I86-MANIFEST-MISSING:%s\n",
                timing_manifest_records[index].key_id);
        }
    }
    if (covered_count != i86_count) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:PARTIAL:%u/%u\n",
            covered_count, i86_count);
        return 1;
    }
    if (timing_manifest_write_results()) {
        STD_PRINTF("M5:T435:S4:I86-MANIFEST-PROBE:FAIL:RESULTS\n");
        return 1;
    }
    STD_PRINTF("M5:T435:S5:I86-MANIFEST-PROBE:PASS:%u/%u\n", covered_count,
        i86_count);
    return 0;
}
