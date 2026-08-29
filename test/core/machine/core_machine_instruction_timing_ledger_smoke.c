#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define TIMING_LEDGER_RESET_LINEAR 0xfffffff0u
#define TIMING_LEDGER_RESET_PHYSICAL 0x000ffff0u
#define TIMING_LEDGER_WINDOW_BYTES 16u

typedef struct timing_ledger_state {
    type_unsigned_32 reads;
    type_unsigned_32 writes;
    type_unsigned_64 advanced_ticks;
} timing_ledger_state;

static type_status timing_ledger_port_read(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 *out_value)
{
    timing_ledger_state *state = (timing_ledger_state *)owner;

    if (state == STD_NULL || out_value == STD_NULL || port != 0x00e0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    ++state->reads;
    *out_value = 0x5au;
    return TYPE_STATUS_OK;
}

static type_status timing_ledger_port_write(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 value)
{
    timing_ledger_state *state = (timing_ledger_state *)owner;

    if (state == STD_NULL || port != 0x00e0u || value > 0xffu) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    ++state->writes;
    return TYPE_STATUS_OK;
}

static const core_machine_port_provider timing_ledger_port_provider = {
    timing_ledger_port_read,
    timing_ledger_port_write
};

static C_VOID timing_ledger_execution_reset(C_VOID *opaque)
{
    timing_ledger_state *state = (timing_ledger_state *)opaque;

    if (state != STD_NULL) state->advanced_ticks = 0u;
}

static C_VOID timing_ledger_execution_advance(C_VOID *opaque,
    type_unsigned_64 elapsed_ticks)
{
    timing_ledger_state *state = (timing_ledger_state *)opaque;

    if (state != STD_NULL) state->advanced_ticks += elapsed_ticks;
}

static const core_machine_execution_provider timing_ledger_execution_provider = {
    timing_ledger_execution_reset,
    timing_ledger_execution_advance
};

typedef struct timing_ledger_qualification_probe {
    core_machine_retirement_eligibility_key key;
    type_bool captured;
} timing_ledger_qualification_probe;

static C_VOID timing_ledger_qualification_record(C_VOID *context,
    const core_machine_retirement_observation *observation)
{
    timing_ledger_qualification_probe *probe =
        (timing_ledger_qualification_probe *)context;

    if (probe != STD_NULL && observation != STD_NULL) {
        probe->key = observation->eligibility_key;
        probe->captured = TYPE_TRUE;
    }
}
static C_INT timing_ledger_prepare(core_machine **out_machine,
    timing_ledger_state *state)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    core_machine *machine = STD_NULL;

    if (out_machine == STD_NULL || state == STD_NULL ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_LEDGER_RESET_LINEAR, TIMING_LEDGER_RESET_PHYSICAL,
            TIMING_LEDGER_WINDOW_BYTES) != TYPE_STATUS_OK ||
        core_machine_install_port_provider(machine, 0x00e0u, 0x00e0u,
            &timing_ledger_port_provider, state) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(machine,
            &timing_ledger_execution_provider, state)) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static C_INT timing_ledger_load(core_machine *machine,
    const type_unsigned_8 *program, STD_SIZE_T program_bytes)
{
    return machine != STD_NULL && program != STD_NULL &&
        core_machine_reset(machine) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, TIMING_LEDGER_RESET_LINEAR, program,
            program_bytes) == TYPE_STATUS_OK;
}

static C_INT timing_ledger_execute(core_machine *machine,
    type_unsigned_64 instructions, type_unsigned_64 expected_ticks,
    timing_ledger_state *state)
{
    const core_machine_run_budget budget = { instructions, 0u };
    core_machine_run_result result;

    return machine != STD_NULL && state != STD_NULL &&
        core_machine_run(machine, budget, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET &&
        result.executed == instructions && result.ticks == expected_ticks &&
        result.elapsed_ticks == expected_ticks &&
        state->advanced_ticks == expected_ticks;
}

static C_INT timing_ledger_case(const type_unsigned_8 *program,
    STD_SIZE_T program_bytes, type_unsigned_64 instructions,
    type_unsigned_64 expected_ticks)
{
    timing_ledger_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_ledger_prepare(&machine, &state) ||
        !timing_ledger_load(machine, program, program_bytes) ||
        !timing_ledger_execute(machine, instructions, expected_ticks, &state);

    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_ledger_test_baseline(C_VOID)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    static const type_unsigned_8 clc[] = { 0xf8u };
    static const type_unsigned_8 cld[] = { 0xfcu };
    static const type_unsigned_8 sal_register_one[] = { 0xd0u, 0xe3u };
    static const type_unsigned_8 rcl_register_one_32[] = {
        0x66u, 0xd1u, 0xd0u
    };
    static const type_unsigned_8 cli[] = { 0xfau };
    static const type_unsigned_8 sahf[] = { 0x9eu };
    static const type_unsigned_8 lahf[] = { 0x9fu };
    static const type_unsigned_8 rcl_register_cl_32[] = { 0x66u, 0xd3u, 0xd3u };
    static const type_unsigned_8 mov_sreg_register[] = { 0x8eu, 0xd8u };
    static const type_unsigned_8 mov_imm[] = { 0xb8u, 0x34u, 0x12u };
    static const type_unsigned_8 mov_register[] = { 0x8bu, 0xc1u };

    return !timing_ledger_case(nop, sizeof(nop), 1u, 3u) &&
        !timing_ledger_case(clc, sizeof(clc), 1u, 2u) &&
        !timing_ledger_case(cld, sizeof(cld), 1u, 2u) &&
        !timing_ledger_case(sal_register_one, sizeof(sal_register_one), 1u, 3u) &&
        !timing_ledger_case(rcl_register_one_32, sizeof(rcl_register_one_32),
            1u, 9u) &&
        !timing_ledger_case(cli, sizeof(cli), 1u, 3u) &&
        !timing_ledger_case(sahf, sizeof(sahf), 1u, 3u) &&
        !timing_ledger_case(lahf, sizeof(lahf), 1u, 2u) &&
        !timing_ledger_case(rcl_register_cl_32, sizeof(rcl_register_cl_32),
            1u, 9u) &&
        !timing_ledger_case(mov_sreg_register, sizeof(mov_sreg_register), 1u,
            2u) && !timing_ledger_case(mov_imm, sizeof(mov_imm), 1u, 2u) &&
        !timing_ledger_case(mov_register, sizeof(mov_register), 1u, 2u);
}

static C_INT timing_ledger_capture_qualification(const type_unsigned_8 *program,
    STD_SIZE_T program_bytes, core_machine_retirement_eligibility_key *out_key)
{
    timing_ledger_state state = { 0u, 0u, 0u };
    timing_ledger_qualification_probe probe = { { 0 }, TYPE_FALSE };
    const core_machine_retirement_observation_provider provider = {
        timing_ledger_qualification_record, &probe
    };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine *machine = STD_NULL;
    C_INT failed = out_key == STD_NULL || !timing_ledger_prepare(&machine, &state) ||
        core_machine_set_retirement_observation_provider(machine, &provider) !=
            TYPE_STATUS_OK || !timing_ledger_load(machine, program, program_bytes) ||
        core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        !probe.captured;

    if (!failed) *out_key = probe.key;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_ledger_physical_case(const type_unsigned_8 *program,
    STD_SIZE_T program_bytes, type_status expected_status,
    type_unsigned_64 expected_ticks)
{
    core_machine_retirement_eligibility_key entry = { 0 };
    const core_machine_retirement_qualification_descriptor qualification = {
        &entry, 1u
    };
    core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .time_axis = { CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL, 8000000u },
        .retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_PHYSICAL
    };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    timing_ledger_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = 0;

    if (expected_status == TYPE_STATUS_OK) {
        failed = timing_ledger_capture_qualification(program, program_bytes, &entry);
        config.retirement_qualification = &qualification;
    }
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_LEDGER_RESET_LINEAR, TIMING_LEDGER_RESET_PHYSICAL,
            TIMING_LEDGER_WINDOW_BYTES) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(machine,
            &timing_ledger_execution_provider, &state) ||
        !timing_ledger_load(machine, program, program_bytes) ||
        core_machine_run(machine, budget, &result) != expected_status;

    if (!failed && expected_status == TYPE_STATUS_OK) {
        failed |= result.reason != CORE_MACHINE_STOP_BUDGET ||
            result.executed != 1u || result.ticks != expected_ticks ||
            result.elapsed_ticks != expected_ticks ||
            state.advanced_ticks != expected_ticks;
    }
    if (!failed && expected_status == TYPE_STATUS_FAULT) {
        failed |= result.reason != CORE_MACHINE_STOP_FAULT ||
            result.executed != 0u || result.ticks != 0u ||
            result.elapsed_ticks != 0u || state.advanced_ticks != 0u;
    }
    core_machine_destroy(machine);
    return failed;
}
static C_INT timing_ledger_physical_protected_mov_sreg_memory(C_VOID)
{
    static const type_unsigned_8 gdt_pointer[] = { 0x3fu, 0u, 0u, 0x03u, 0u, 0u };
    static const type_unsigned_8 gdt[] = {
        0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x20u, 0u, 0x9au, 0u, 0u,
        0xffu, 0xffu, 0u, 0x30u, 0u, 0x92u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x30u, 0u, 0x12u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x30u, 0u, 0x98u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x50u, 0u, 0x92u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x50u, 0u, 0x92u, 0u, 0u,
        0x0fu, 0u, 0u, 0x50u, 0u, 0x92u, 0u, 0u
    };
    static const type_unsigned_8 boot[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u, 0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u, 0x8eu, 0xc0u,
        0xb8u, 0x10u, 0x00u, 0x8eu, 0xd0u,
        0xbcu, 0x00u, 0x80u, 0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const type_unsigned_8 program[] = { 0x8eu, 0x1eu, 0x00u, 0x10u };
    static const type_unsigned_8 selector[] = { 0x30u, 0x00u };
    static const type_unsigned_8 halt[] = { 0xf4u };
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    timing_ledger_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    type_unsigned_64 elapsed_before = 0u;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(machine,
            &timing_ledger_execution_provider, &state) ||
        !test_core_machine_fixture_prepare_real_mode_execution(machine, 0u) ||
        core_machine_memory_write(machine, 0x0100u, gdt_pointer,
            sizeof(gdt_pointer)) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0x0300u, gdt, sizeof(gdt)) !=
            TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0u, boot, sizeof(boot)) !=
            TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0x2000u, halt, sizeof(halt)) !=
            TYPE_STATUS_OK ||
        core_machine_run(machine, (core_machine_run_budget){96u, 0u},
            &result) != TYPE_STATUS_OK || result.reason !=
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;

    if (!failed) {
        failed |= core_machine_memory_write(machine, 0x4000u, selector,
            sizeof(selector)) != TYPE_STATUS_OK ||
            core_machine_memory_write(machine, 0x2000u, program,
                sizeof(program)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        test_core_machine_fixture_resume_after_halt_at(machine, 0u);
        elapsed_before = machine->elapsed_ticks;
        state.advanced_ticks = 0u;
        failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            result.ticks != 19u || result.elapsed_ticks != elapsed_before + 19u ||
            state.advanced_ticks != 19u;
    }
    core_machine_destroy(machine);
    return failed;
}
static C_INT timing_ledger_physical_far_jmp_memory(C_INT protected_mode)
{
    static const type_unsigned_8 instruction[] = {
        0x2eu, 0xffu, 0x2eu, 0xf6u, 0xffu
    };
    static const type_unsigned_8 pointer[] = { 0xfeu, 0xffu, 0x00u, 0xf0u };
    static const type_unsigned_8 target[] = { 0x90u };
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .time_axis = { CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL, 8000000u },
        .retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_PHYSICAL
    };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    timing_ledger_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_LEDGER_RESET_LINEAR, TIMING_LEDGER_RESET_PHYSICAL,
            TIMING_LEDGER_WINDOW_BYTES) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(machine,
            &timing_ledger_execution_provider, &state) ||
        !timing_ledger_load(machine, instruction, sizeof(instruction)) ||
        core_machine_memory_write(machine, TIMING_LEDGER_RESET_LINEAR + 6u,
            pointer, sizeof(pointer)) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, TIMING_LEDGER_RESET_LINEAR + 14u,
            target, sizeof(target)) != TYPE_STATUS_OK;

    if (!failed && protected_mode) machine->executor_cpu.data.cr0 |= VCPU_CR0_PE;
    if (!failed) {
        failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != 0u ||
            state.advanced_ticks != 0u;
    }
    core_machine_destroy(machine);
    return failed;
}
static C_INT timing_ledger_physical_protected_far_jmp_memory(C_VOID)
{
    static const type_unsigned_8 gdt_pointer[] = { 0x17u, 0u, 0u, 0x03u, 0u, 0u };
    static const type_unsigned_8 gdt[] = {
        0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x20u, 0u, 0x9au, 0u, 0u,
        0xffu, 0xffu, 0u, 0u, 0u, 0x92u, 0xcfu, 0u
    };
    static const type_unsigned_8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u, 0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u, 0x8eu, 0xc0u,
        0xb8u, 0x10u, 0x00u, 0x8eu, 0xd0u,
        0xbcu, 0x00u, 0x80u, 0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const type_unsigned_8 instruction[] = {
        0x2eu, 0xffu, 0x2eu, 0x1fu, 0x00u
    };
    static const type_unsigned_8 pointer[] = { 0x28u, 0u, 0x08u, 0u };
    static const type_unsigned_8 target[] = { 0x90u };
    static const type_unsigned_8 halt[] = { 0xf4u };
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    const core_machine_run_budget setup_budget = { 96u, 0u };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    timing_ledger_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    type_unsigned_64 elapsed_before = 0u;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(machine,
            &timing_ledger_execution_provider, &state) ||
        !test_core_machine_fixture_prepare_real_mode_execution(machine, 0u) ||
        core_machine_memory_write(machine, 0x0100u, gdt_pointer,
            sizeof(gdt_pointer)) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0x0300u, gdt, sizeof(gdt)) !=
            TYPE_STATUS_OK || core_machine_memory_write(machine, 0u, real_code,
            sizeof(real_code)) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0x2000u, halt, sizeof(halt)) !=
            TYPE_STATUS_OK || core_machine_run(machine, setup_budget, &result) !=
            TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    if (!failed) {
        failed |= core_machine_memory_write(machine, 0x2000u, instruction,
            sizeof(instruction)) != TYPE_STATUS_OK ||
            core_machine_memory_write(machine, 0x201fu, pointer,
                sizeof(pointer)) != TYPE_STATUS_OK ||
            core_machine_memory_write(machine, 0x2028u, target,
                sizeof(target)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        test_core_machine_fixture_resume_after_halt_at(machine, 0u);
        machine->retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_PHYSICAL;
        elapsed_before = machine->elapsed_ticks;
        state.advanced_ticks = 0u;
        failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != elapsed_before ||
            state.advanced_ticks != 0u;
    }
    core_machine_destroy(machine);
    return failed;
}
static C_INT timing_ledger_test_physical_classifier_boundary(C_VOID)
{
    static const type_unsigned_8 cli[] = { 0xfau };
    static const type_unsigned_8 cld[] = { 0xfcu };
    static const type_unsigned_8 prefixed_cld[] = { 0x26u, 0xfcu };
    static const type_unsigned_8 sal_register_one[] = { 0xd0u, 0xe3u };
    static const type_unsigned_8 sal_memory_one[] = { 0xd0u, 0x26u, 0x00u, 0x10u };
    static const type_unsigned_8 rcl_register_one_32[] = {
        0x66u, 0xd1u, 0xd0u
    };
    static const type_unsigned_8 rcl_register_one_16[] = { 0xd1u, 0xd0u };
    static const type_unsigned_8 rcl_memory_one_32[] = {
        0x66u, 0xd1u, 0x16u, 0x00u, 0x10u
    };
    static const type_unsigned_8 rcl_register_cl_32[] = { 0x66u, 0xd3u, 0xd3u };
    static const type_unsigned_8 rcl_memory_cl_32[] = {
        0x66u, 0xd3u, 0x16u, 0x00u, 0x10u
    };
    static const type_unsigned_8 lahf[] = { 0x9fu };
    static const type_unsigned_8 prefixed_lahf[] = { 0x26u, 0x9fu };
    static const type_unsigned_8 sahf[] = { 0x9eu };
    static const type_unsigned_8 mov_sreg_register[] = { 0x8eu, 0xd8u };
    static const type_unsigned_8 mov_sreg_memory[] = {
        0x8eu, 0x1eu, 0x00u, 0x10u
    };

    return timing_ledger_physical_case(cli, sizeof(cli), TYPE_STATUS_OK, 3u) ||
        timing_ledger_physical_case(cld, sizeof(cld), TYPE_STATUS_OK, 2u) ||
        timing_ledger_physical_case(prefixed_cld, sizeof(prefixed_cld),
            TYPE_STATUS_FAULT, 0u) ||
        timing_ledger_physical_case(sal_register_one, sizeof(sal_register_one),
            TYPE_STATUS_OK, 3u) ||
        timing_ledger_physical_case(sal_memory_one, sizeof(sal_memory_one),
            TYPE_STATUS_FAULT, 0u) ||
        timing_ledger_physical_case(rcl_register_one_32,
            sizeof(rcl_register_one_32), TYPE_STATUS_OK, 9u) ||
        timing_ledger_physical_case(rcl_register_one_16,
            sizeof(rcl_register_one_16), TYPE_STATUS_FAULT, 0u) ||
        timing_ledger_physical_case(rcl_memory_one_32,
            sizeof(rcl_memory_one_32), TYPE_STATUS_FAULT, 0u) ||
        timing_ledger_physical_case(rcl_register_cl_32,
            sizeof(rcl_register_cl_32), TYPE_STATUS_OK, 9u) ||
        timing_ledger_physical_case(rcl_memory_cl_32,
            sizeof(rcl_memory_cl_32), TYPE_STATUS_FAULT, 0u) ||
        timing_ledger_physical_case(lahf, sizeof(lahf), TYPE_STATUS_OK, 2u) ||
        timing_ledger_physical_case(prefixed_lahf, sizeof(prefixed_lahf),
            TYPE_STATUS_FAULT, 0u) ||
        timing_ledger_physical_case(sahf, sizeof(sahf), TYPE_STATUS_OK, 3u) ||
        timing_ledger_physical_case(mov_sreg_register,
            sizeof(mov_sreg_register), TYPE_STATUS_OK, 2u) ||
        timing_ledger_physical_case(mov_sreg_memory,
            sizeof(mov_sreg_memory), TYPE_STATUS_OK, 5u) ||
        timing_ledger_physical_protected_mov_sreg_memory() ||
        timing_ledger_physical_far_jmp_memory(0) ||
        timing_ledger_physical_far_jmp_memory(1) ||
        timing_ledger_physical_protected_far_jmp_memory();
}

static C_INT timing_ledger_test_memory(C_VOID)
{
    static const type_unsigned_8 mov_read[] = { 0x8bu, 0x0eu, 0x00u, 0x10u };
    static const type_unsigned_8 mov_write[] = { 0x89u, 0x0eu, 0x00u, 0x10u };
    static const type_unsigned_8 moffs_read[] = { 0xa1u, 0x00u, 0x10u };
    static const type_unsigned_8 moffs_write[] = { 0xa3u, 0x00u, 0x10u };
    const type_unsigned_16 value = 0x5aa5u;
    timing_ledger_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_ledger_prepare(&machine, &state);

    if (!failed) {
        failed |= !timing_ledger_load(machine, mov_read, sizeof(mov_read)) ||
            core_machine_memory_write(machine, 0x1000u, &value, sizeof(value)) !=
                TYPE_STATUS_OK || !timing_ledger_execute(machine, 1u, 4u, &state) ||
            machine->executor_cpu.data.cx != value;
    }
    if (!failed) {
        failed |= !timing_ledger_load(machine, mov_write, sizeof(mov_write)) ||
            ((machine->executor_cpu.data.cx = value), 0) ||
            !timing_ledger_execute(machine, 1u, 2u, &state);
    }
    if (!failed) {
        failed |= !timing_ledger_load(machine, moffs_read, sizeof(moffs_read)) ||
            core_machine_memory_write(machine, 0x1000u, &value, sizeof(value)) !=
                TYPE_STATUS_OK || !timing_ledger_execute(machine, 1u, 4u, &state) ||
            machine->executor_cpu.data.ax != value;
    }
    if (!failed) {
        failed |= !timing_ledger_load(machine, moffs_write, sizeof(moffs_write)) ||
            ((machine->executor_cpu.data.ax = value), 0) ||
            !timing_ledger_execute(machine, 1u, 2u, &state);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_ledger_test_ports(C_VOID)
{
    static const type_unsigned_8 in_immediate[] = { 0xe4u, 0xe0u };
    static const type_unsigned_8 out_immediate[] = { 0xe6u, 0xe0u };
    static const type_unsigned_8 in_dx[] = { 0xecu };
    static const type_unsigned_8 out_dx[] = { 0xeeu };
    timing_ledger_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_ledger_prepare(&machine, &state);

    if (!failed) {
        failed |= !timing_ledger_load(machine, in_immediate, sizeof(in_immediate)) ||
            !timing_ledger_execute(machine, 1u, 12u, &state) || state.reads != 1u;
    }
    if (!failed) {
        failed |= !timing_ledger_load(machine, out_immediate,
            sizeof(out_immediate)) || !timing_ledger_execute(machine, 1u, 10u,
                &state) || state.writes != 1u;
    }
    if (!failed) {
        failed |= !timing_ledger_load(machine, in_dx, sizeof(in_dx)) ||
            ((machine->executor_cpu.data.dx = 0x00e0u), 0) ||
            !timing_ledger_execute(machine, 1u, 13u, &state) || state.reads != 2u;
    }
    if (!failed) {
        failed |= !timing_ledger_load(machine, out_dx, sizeof(out_dx)) ||
            ((machine->executor_cpu.data.dx = 0x00e0u), 0) ||
            !timing_ledger_execute(machine, 1u, 11u, &state) || state.writes != 2u;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_ledger_test_jcc_and_repeat(C_VOID)
{
    static const type_unsigned_8 taken[] = { 0x74u, 0x01u, 0x90u, 0x90u };
    static const type_unsigned_8 not_taken[] = { 0x75u, 0x01u, 0x90u, 0x90u };
    static const type_unsigned_8 rep_movsb[] = { 0xf3u, 0xa4u };
    static const type_unsigned_8 source[] = { 0x11u, 0x22u, 0x33u };
    timing_ledger_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_ledger_prepare(&machine, &state);

    if (!failed) {
        failed |= !timing_ledger_load(machine, taken, sizeof(taken)) ||
            ((machine->executor_cpu.data.eflags |= VCPU_EFLAGS_ZF), 0) ||
            !timing_ledger_execute(machine, 1u, 8u, &state);
    }
    if (!failed) {
        failed |= !timing_ledger_load(machine, not_taken, sizeof(not_taken)) ||
            ((machine->executor_cpu.data.eflags |= VCPU_EFLAGS_ZF), 0) ||
            !timing_ledger_execute(machine, 1u, 3u, &state);
    }
    if (!failed) {
        failed |= !timing_ledger_load(machine, rep_movsb, sizeof(rep_movsb)) ||
            core_machine_memory_write(machine, 0x1000u, source, sizeof(source)) !=
                TYPE_STATUS_OK ||
            ((machine->executor_cpu.data.cx = 3u),
                (machine->executor_cpu.data.si = 0x1000u),
                (machine->executor_cpu.data.di = 0x1100u), 0) ||
            !timing_ledger_execute(machine, 3u, 17u, &state);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_ledger_test_unavailable_and_fault(C_VOID)
{
    static const type_unsigned_8 rol_register_one[] = { 0xd0u, 0xc0u };
    static const type_unsigned_8 fault[] = { 0xf0u, 0x90u };
    timing_ledger_state state = { 0u, 0u, 0u };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_ledger_prepare(&machine, &state);

    if (!failed) {
        failed |= !timing_ledger_load(machine, rol_register_one,
            sizeof(rol_register_one)) || !timing_ledger_execute(machine, 1u, 3u,
                &state);
    }
    if (!failed) {
        failed |= !timing_ledger_load(machine, fault, sizeof(fault)) ||
            !test_core_machine_fixture_preflight_real_ud_terminal(machine) ||
            core_machine_run(machine, budget, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != 0u ||
            state.advanced_ticks != 0u;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_ledger_test_budget_overflow_and_reset(C_VOID)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    const core_machine_run_budget insufficient = { 1u, 105u };
    const core_machine_run_budget sufficient = { 1u, 106u };
    core_machine_run_result result;
    timing_ledger_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_ledger_prepare(&machine, &state) ||
        !timing_ledger_load(machine, nop, sizeof(nop));

    if (!failed) {
        failed |= core_machine_run(machine, insufficient, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != 0u ||
            machine->executor_cpu.data.eip != 0xfff0u || state.advanced_ticks != 0u;
    }
    if (!failed) {
        failed |= core_machine_run(machine, sufficient, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            result.ticks != 3u || result.elapsed_ticks != 3u ||
            state.advanced_ticks != 3u || core_machine_reset(machine) != TYPE_STATUS_OK ||
            core_machine_get_elapsed_ticks(machine, &result.elapsed_ticks) !=
                TYPE_STATUS_OK || result.elapsed_ticks != 0u ||
            !timing_ledger_load(machine, nop, sizeof(nop)) ||
            !timing_ledger_execute(machine, 1u, 3u, &state);
    }
    if (!failed) {
        failed |= !timing_ledger_load(machine, nop, sizeof(nop));
        machine->elapsed_ticks = UINT64_MAX - 2u;
        state.advanced_ticks = 0u;
        failed |= core_machine_run(machine, sufficient, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != UINT64_MAX - 2u ||
            machine->elapsed_ticks != UINT64_MAX - 2u || state.advanced_ticks != 0u;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_ledger_test_compatibility_is_not_source_truth(C_VOID)
{
    static const type_unsigned_8 prefixed_nop[] = { 0x26u, 0x90u };
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .ticks_per_instruction = 10u,
        .instruction_timing = { 10u, 2u, 7u, 3u, 5u, 4u }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine *machine = STD_NULL;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_LEDGER_RESET_LINEAR, TIMING_LEDGER_RESET_PHYSICAL,
            TIMING_LEDGER_WINDOW_BYTES) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, TIMING_LEDGER_RESET_LINEAR,
            prefixed_nop, sizeof(prefixed_nop)) != TYPE_STATUS_OK ||
        core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        result.ticks != 3u || result.elapsed_ticks != 3u;

    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    if (!timing_ledger_test_baseline()) return 1;
    if (timing_ledger_test_memory()) return 2;
    if (timing_ledger_test_ports()) return 3;
    if (timing_ledger_test_jcc_and_repeat()) return 4;
    if (timing_ledger_test_unavailable_and_fault()) return 5;
    if (timing_ledger_test_budget_overflow_and_reset()) return 6;
    if (timing_ledger_test_compatibility_is_not_source_truth()) return 7;
    if (timing_ledger_test_physical_classifier_boundary()) return 8;
    STD_PRINTF("M5:T357:S3:INSTRUCTION-TIMING-LEDGER:OK\n");
    return 0;
}
