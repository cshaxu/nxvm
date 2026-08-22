#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/cpu_timing.h"
#include "core/machine/machine_interface.h"
#include "core/machine/retirement_observation_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define TIMING_80286_MANIFEST_RESET_LINEAR 0xfffffff0u
#define TIMING_80286_MANIFEST_RESET_PHYSICAL 0x000ffff0u
#define TIMING_80286_MANIFEST_WINDOW_BYTES 16u
#define TIMING_80286_MANIFEST_STACK_LINEAR 0x00001000u
#define TIMING_80286_MANIFEST_STACK_BYTES 16u

/* Incremental real-observation runner for the I286 manifest.  A partial
 * recipe set must never write the final result document: the result verifier
 * accepts exactly 807 observed canonical keys. */
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
    type_unsigned_8 program[8];
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
static C_INT timing_80286_manifest_current_index = -1;
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

static C_INT timing_80286_manifest_is_dx_port(const C_CHAR *key_id)
{
    return key_id != STD_NULL && (STD_STRCMP(key_id, "I286-IN-DX-B") == 0 ||
        STD_STRCMP(key_id, "I286-IN-DX-W") == 0 ||
        STD_STRCMP(key_id, "I286-OUT-DX-B") == 0 ||
        STD_STRCMP(key_id, "I286-OUT-DX-W") == 0);
}

static C_INT timing_80286_manifest_is_interrupt(const C_CHAR *key_id)
{
    return key_id != STD_NULL &&
        (STD_STRCMP(key_id, "I286-INT3-REAL-NEXT-BYTE-2") == 0 ||
            STD_STRCMP(key_id, "I286-INT-IMM-REAL-NEXT-BYTE-2") == 0 ||
            STD_STRCMP(key_id, "I286-INTO-TAKEN-NEXT-BYTE-2") == 0 ||
            STD_STRCMP(key_id, "I286-INTO-NOT") == 0);
}

static C_INT timing_80286_manifest_uses_stack(const C_CHAR *key_id)
{
    static const C_CHAR *const stack_keys[] = {
        "I286-CALL-NEAR-DIRECT-NEXT-BYTE-1", "I286-CALL-NEAR-RM-SEGMENT",
        "I286-CALL-NEAR-RM-EA-BID", "I286-STACK-PUSH-M",
        "I286-STACK-POP-M",
        "I286-RET-NEAR-NEXT-BYTE-1", "I286-STACK-PUSH-R",
        "I286-STACK-PUSH-SEG", "I286-STACK-PUSH-IMM",
        "I286-STACK-PUSH-M-EA-BID", "I286-STACK-POP-M-EA-BID",
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

static C_INT timing_80286_manifest_is_bound(const C_CHAR *key_id)
{
    return key_id != STD_NULL && STD_STRCMP(key_id, "I286-BOUND") == 0;
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
    for (index = 0u; index < sizeof(timing_80286_manifest_records) /
            sizeof(timing_80286_manifest_records[0]); ++index) {
        if (STD_STRCMP(timing_80286_manifest_records[index].key_id, key_id) == 0) {
            timing_80286_manifest_current_index = (C_INT)index;
            return &timing_80286_manifest_records[index];
        }
    }
    return STD_NULL;
}

static C_VOID timing_80286_manifest_capture_retirement(C_VOID *opaque,
    const core_machine_retirement_observation *observation)
{
    timing_80286_manifest_capture *capture =
        (timing_80286_manifest_capture *)opaque;

    if (capture == STD_NULL || observation == STD_NULL) return;
    if (capture->count == 0u) capture->observation = *observation;
    timing_80286_manifest_current_index = -1;
    ++capture->count;
}

static C_VOID timing_80286_manifest_execution_reset(C_VOID *opaque)
{
    (C_VOID)opaque;
}

static const core_machine_execution_provider timing_80286_manifest_execution = {
    timing_80286_manifest_execution_reset, STD_NULL, STD_NULL
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
    if (status == TYPE_STATUS_OK && STD_STRCMP(key_id,
            "I286-RET-NEAR-NEXT-BYTE-1") == 0) {
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
            "I286-SYSTEM-LGDT-M-EA-BID") == 0 || STD_STRCMP(key_id,
            "I286-SYSTEM-LIDT-M-EA-BID") == 0 || STD_STRCMP(key_id,
            "I286-SYSTEM-LGDT-M-SEGMENT") == 0 || STD_STRCMP(key_id,
            "I286-SYSTEM-LIDT-M-SEGMENT") == 0)) {
        status = core_machine_memory_write(machine, 0x4000u, table_pointer,
            sizeof(table_pointer));
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
    const timing_80286_manifest_recipe *recipe)
{
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_16 value = 1u;
    timing_80286_manifest_capture capture = { { 0 }, 0u };
    core_machine_run_result run = { 0 };
    core_machine *machine = STD_NULL;
    C_INT failed;

    if (recipe == STD_NULL) return 1;
    failed = timing_80286_manifest_find(recipe->key_id) == STD_NULL ||
        !timing_80286_manifest_prepare(&machine, &capture, recipe->key_id,
            recipe->program, recipe->bytes);
    if (!failed) {
        machine->executor_cpu.data.es.base = machine->executor_cpu.data.ds.base;
        machine->executor_cpu.data.es.selector = machine->executor_cpu.data.ds.selector;
        machine->executor_cpu.data.si = 0x1000u;
        machine->executor_cpu.data.di = 0x1100u;
        machine->executor_cpu.data.ax = value;
        machine->executor_cpu.data.edx = 0x0080u;
        failed = core_machine_memory_write(machine, 0x1000u, &value,
            sizeof(value)) != TYPE_STATUS_OK || core_machine_memory_write(machine,
            0x1100u, &value, sizeof(value)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed = core_machine_run(machine, budget, &run) != TYPE_STATUS_OK ||
            run.reason != CORE_MACHINE_STOP_BUDGET || run.executed != 1u ||
            run.ticks != recipe->ticks || capture.count != 1u ||
            capture.observation.source_ticks != recipe->ticks ||
            capture.observation.timing_origin !=
                CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO ||
            capture.observation.timing_disposition !=
                CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    }
    if (failed) STD_PRINTF("M5:T435:S10:I286-STRING-DETAIL:%s:expected=%llu:run=%llu:source=%llu:count=%u\n",
        recipe->key_id, recipe->ticks, run.ticks,
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
        { "I286-IMUL-R8", { 0xf6u, 0xe8u }, 2u, 13u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-R16", { 0xf7u, 0xe8u }, 2u, 21u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-M8", { 0xf6u, 0x2eu, 0u, 0x10u }, 4u, 16u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IMUL-M16", { 0xf7u, 0x2eu, 0u, 0x10u }, 4u, 24u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DIV-R8", { 0xf6u, 0xf0u }, 2u, 14u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DIV-R16", { 0xf7u, 0xf0u }, 2u, 22u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DIV-M8", { 0xf6u, 0x36u, 0u, 0x10u }, 4u, 17u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-DIV-M16", { 0xf7u, 0x36u, 0u, 0x10u }, 4u, 25u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IDIV-R8", { 0xf6u, 0xf8u }, 2u, 17u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IDIV-R16", { 0xf7u, 0xf8u }, 2u, 25u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IDIV-M8", { 0xf6u, 0x3eu, 0u, 0x10u }, 4u, 20u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-IDIV-M16", { 0xf7u, 0x3eu, 0u, 0x10u }, 4u, 28u,
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
        { "I286-MOV-SREG-LOAD-REAL", { 0x8eu, 0x1eu, 0u, 0x10u }, 4u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LEA-M-REAL", { 0x8du, 0x06u, 0u, 0x10u }, 4u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LDS-M-REAL", { 0xc5u, 0x06u, 0u, 0x10u }, 4u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LES-M-REAL", { 0xc4u, 0x06u, 0u, 0x10u }, 4u, 7u,
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
        { "I286-STACK-PUSHA", { 0x60u }, 1u, 17u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-PUSHF", { 0x9cu }, 1u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-POP-R", { 0x58u }, 1u, 5u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK },
        { "I286-STACK-POP-M", { 0x8fu, 0x06u, 0u, 0x10u }, 4u, 5u,
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
        { "I286-INTO-TAKEN-NEXT-BYTE-2", 0xceu, VCPU_EFLAGS_OF, 26u },
        { "I286-INTO-NOT", 0xceu, 0u, 3u }
    };
    static const timing_80286_manifest_recipe protected_system_recipes[] = {
        { "I286-MOV-SREG-LOAD-PM", { 0x8eu, 0x1eu, 0u, 0x10u }, 4u, 19u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LEA-M-PM", { 0x8du, 0x06u, 0u, 0x10u }, 4u, 3u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LDS-M-PM", { 0xc5u, 0x06u, 0u, 0x10u }, 4u, 21u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-LES-M-PM", { 0xc4u, 0x06u, 0u, 0x10u }, 4u, 21u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
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
        { "I286-ALU-SUB-MR-LOCK", { 0xf0u, 0x28u, 0x0eu, 0u, 0x10u }, 5u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-ALU-ADD-RMI-LOCK", { 0xf0u, 0x80u, 0x06u, 0u, 0x10u, 1u }, 6u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-INC-M-LOCK", { 0xf0u, 0xfeu, 0x06u, 0u, 0x10u }, 5u, 7u,
            CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY },
        { "I286-NEG-M-LOCK", { 0xf0u, 0xf6u, 0x1eu, 0u, 0x10u }, 5u, 7u,
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
    for (index = 0u; index < sizeof(string_recipes) / sizeof(string_recipes[0]);
        ++index) {
        if (timing_80286_manifest_run_string_recipe(&string_recipes[index])) {
            STD_PRINTF("M5:T435:S10:I286-STRING-RECIPE:FAIL:%s\n",
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
    }
    for (index = 0u; index < sizeof(lock_recipes) / sizeof(lock_recipes[0]);
        ++index) {
        if (timing_80286_manifest_run(&lock_recipes[index])) {
            STD_PRINTF("M5:T435:S10:I286-LOCK-RECIPE:FAIL:%s\n",
                lock_recipes[index].key_id);
            return 1;
        }
    }
    STD_PRINTF("M5:T435:S10:I286-MANIFEST-FOUNDATION:PASS:observed=%u\n",
        (type_unsigned_32)(sizeof(recipes) / sizeof(recipes[0]) +
            sizeof(control_recipes) / sizeof(control_recipes[0]) +
            sizeof(protected_system_recipes) /
                sizeof(protected_system_recipes[0]) +
            sizeof(segment_recipes) / sizeof(segment_recipes[0]) +
            sizeof(ea_recipes) / sizeof(ea_recipes[0]) +
            sizeof(odd_word_recipes) / sizeof(odd_word_recipes[0]) +
            sizeof(string_recipes) / sizeof(string_recipes[0]) +
            1u +
            sizeof(repeat_recipes) / sizeof(repeat_recipes[0]) + 9u +
            sizeof(lock_recipes) / sizeof(lock_recipes[0])));
    return 0;
}
