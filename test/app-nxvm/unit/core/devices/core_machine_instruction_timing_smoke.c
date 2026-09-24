#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

#define TIMING_RESET_LINEAR 0xfffffff0u
#define TIMING_RESET_PHYSICAL 0x000ffff0u
#define TIMING_WINDOW_BYTES 16u

typedef struct timing_port_state {
    lib_u32 reads;
    lib_u32 writes;
} timing_port_state;

static lib_status timing_port_read(void *owner, lib_u16 port,
    lib_u32 *out_value)
{
    timing_port_state *state = (timing_port_state *)owner;

    if (state == LIB_NULL || out_value == LIB_NULL || port != 0x00e0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    ++state->reads;
    *out_value = 0x5au;
    return LIB_STATUS_OK;
}

static lib_status timing_port_write(void *owner, lib_u16 port,
    lib_u32 value)
{
    timing_port_state *state = (timing_port_state *)owner;

    if (state == LIB_NULL || port != 0x00e0u || value > 0xffu) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    ++state->writes;
    return LIB_STATUS_OK;
}

static const core_machine_port_provider timing_port_provider = {
    timing_port_read,
    timing_port_write
};

static lib_i32 timing_prepare(core_machine **out_machine,
    timing_port_state *port_state)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286,
        .ticks_per_instruction = 1u,
        .instruction_timing = { 10u, 2u, 7u, 3u, 5u, 4u }
    };
    core_machine *machine = LIB_NULL;

    if (out_machine == LIB_NULL || core_machine_create(&config, &machine) !=
            LIB_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_RESET_LINEAR, TIMING_RESET_PHYSICAL, TIMING_WINDOW_BYTES) !=
            LIB_STATUS_OK ||
        core_machine_install_port_provider(machine, 0x00e0u, 0x00e0u,
            &timing_port_provider, port_state) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static lib_i32 timing_run(core_machine *machine, const lib_u8 *program,
    lib_size program_bytes, lib_u64 instructions, lib_u64 *out_ticks)
{
    core_machine_run_budget budget = { instructions, 0u };
    core_machine_run_result result;

    if (machine == LIB_NULL || program == LIB_NULL || out_ticks == LIB_NULL ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, TIMING_RESET_LINEAR, program,
            program_bytes) != LIB_STATUS_OK ||
        core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET ||
        result.executed != instructions) {
        return 0;
    }
    *out_ticks = result.ticks;
    return 1;
}

static lib_i32 timing_case(const lib_u8 *program, lib_size program_bytes,
    lib_u64 instructions, lib_u64 expected_ticks)
{
    timing_port_state port_state = { 0u, 0u };
    core_machine *machine = LIB_NULL;
    lib_u64 ticks = 0u;
    lib_i32 failed = !timing_prepare(&machine, &port_state);

    if (!failed) {
        failed |= !timing_run(machine, program, program_bytes, instructions,
            &ticks) || ticks != expected_ticks;
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_test_quantum_and_reset(void)
{
    static const lib_u8 program[] = { 0x90u, 0x26u, 0x90u, 0xa0u, 0x00u, 0x00u };
    timing_port_state port_state = { 0u, 0u };
    core_machine_run_budget one = { 1u, 0u };
    core_machine_run_budget all = { 3u, 0u };
    core_machine_run_result result;
    core_machine *machine = LIB_NULL;
    lib_u64 split_ticks = 0u;
    lib_u64 single_ticks = 0u;
    lib_i32 failed = !timing_prepare(&machine, &port_state);
    lib_u32 index;

    if (!failed) {
        failed |= core_machine_memory_write(machine, TIMING_RESET_LINEAR,
            program, sizeof(program)) != LIB_STATUS_OK;
        for (index = 0u; !failed && index < 3u; ++index) {
            failed |= core_machine_run(machine, one, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u;
            split_ticks += result.ticks;
        }
        failed |= split_ticks != 11u;
        failed |= core_machine_reset(machine) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, TIMING_RESET_LINEAR, program,
                sizeof(program)) != LIB_STATUS_OK ||
            core_machine_run(machine, all, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET;
        single_ticks = result.ticks;
        failed |= single_ticks != split_ticks;
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_test_fault(void)
{
    static const lib_u8 fault[] = { 0x66u, 0x90u };
    timing_port_state port_state = { 0u, 0u };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_prepare(&machine, &port_state);

    if (!failed) {
        failed |= core_machine_memory_write(machine, TIMING_RESET_LINEAR, fault,
            sizeof(fault)) != LIB_STATUS_OK ||
            !test_core_machine_fixture_preflight_real_ud_terminal(machine) ||
            core_machine_run(machine, budget, &result) != LIB_STATUS_INTERNAL_ERROR ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != 0u;
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_test_stop(void)
{
    static const lib_u8 nop[] = { 0x90u };
    timing_port_state port_state = { 0u, 0u };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine *machine = LIB_NULL;
    lib_i32 failed = !timing_prepare(&machine, &port_state);

    if (!failed) {
        failed |= core_machine_memory_write(machine, TIMING_RESET_LINEAR, nop,
            sizeof(nop)) != LIB_STATUS_OK ||
            core_machine_request_stop(machine) != LIB_STATUS_OK ||
            core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_REQUESTED || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != 0u;
    }
    core_machine_destroy(machine);
    return failed;
}

typedef struct timing_qualification_probe {
    core_machine_retirement_eligibility_key key;
    lib_u8 captured;
} timing_qualification_probe;

static void timing_qualification_record(void *context,
    const core_machine_retirement_observation *observation)
{
    timing_qualification_probe *probe = (timing_qualification_probe *)context;

    if (probe != LIB_NULL && observation != LIB_NULL) {
        probe->key = observation->eligibility_key;
        probe->captured = LIB_TRUE;
    }
}

static lib_i32 timing_capture_qualification(const lib_u8 *program,
    lib_size program_bytes, core_machine_retirement_eligibility_key *out_key)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .ticks_per_instruction = 1u,
        .instruction_timing = { 10u, 2u, 7u, 3u, 5u, 4u }
    };
    timing_qualification_probe probe = { { 0 }, LIB_FALSE };
    const core_machine_retirement_observation_provider provider = {
        timing_qualification_record, &probe
    };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine *machine = LIB_NULL;
    lib_i32 failed = out_key == LIB_NULL ||
        core_machine_create(&config, &machine) != LIB_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_RESET_LINEAR, TIMING_RESET_PHYSICAL, TIMING_WINDOW_BYTES) !=
            LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_set_retirement_observation_provider(machine, &provider) !=
            LIB_STATUS_OK ||
        core_machine_memory_write(machine, TIMING_RESET_LINEAR, program,
            program_bytes) != LIB_STATUS_OK ||
        core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        !probe.captured;

    if (!failed) *out_key = probe.key;
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 timing_test_invalid_qualification(void)
{
    core_machine_retirement_eligibility_key entry = { 0 };
    const core_machine_retirement_qualification_descriptor missing_entries = {
        LIB_NULL, 1u
    };
    const core_machine_retirement_qualification_descriptor empty_entries = {
        &entry, 0u
    };
    const core_machine_config missing_config = {
        .retirement_qualification = &missing_entries
    };
    const core_machine_config empty_config = {
        .retirement_qualification = &empty_entries
    };
    core_machine *machine = LIB_NULL;

    return core_machine_create(&missing_config, &machine) != LIB_STATUS_INVALID_ARGUMENT ||
        machine != LIB_NULL ||
        core_machine_create(&empty_config, &machine) != LIB_STATUS_INVALID_ARGUMENT ||
        machine != LIB_NULL;
}
static lib_i32 timing_test_physical_contract(void)
{
    static const lib_u8 exact[] = { 0x90u };
    static const lib_u8 jcc[] = { 0x75u, 0xfeu };
    static const lib_u8 classified_unqualified[] = { 0xb8u, 0x34u, 0x12u };
    static const lib_u8 equivalent_prefixed_nop[] = { 0x26u, 0x90u };
    core_machine_retirement_eligibility_key entries[2];
    const core_machine_retirement_qualification_descriptor qualification = {
        entries, sizeof(entries) / sizeof(entries[0])
    };
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .ticks_per_instruction = 1u,
        .instruction_timing = { 10u, 2u, 7u, 3u, 5u, 4u },
        .time_axis = { CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL, 8000000u },
        .retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_PHYSICAL,
        .retirement_qualification = &qualification
    };
    timing_port_state port_state = { 0u, 0u };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine *machine = LIB_NULL;
    lib_i32 failed = timing_capture_qualification(exact, sizeof(exact),
            &entries[0]) ||
        timing_capture_qualification(jcc, sizeof(jcc), &entries[1]) ||
        core_machine_create(&config, &machine) != LIB_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_RESET_LINEAR, TIMING_RESET_PHYSICAL, TIMING_WINDOW_BYTES) !=
            LIB_STATUS_OK ||
        core_machine_install_port_provider(machine, 0x00e0u, 0x00e0u,
            &timing_port_provider, &port_state) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK;

    if (!failed) {
        entries[0].opcode ^= 1u; /* create copied the descriptor. */
        failed |= core_machine_advance_time(machine, 1u) != LIB_STATUS_INVALID_STATE ||
            core_machine_memory_write(machine, TIMING_RESET_LINEAR, exact,
                sizeof(exact)) != LIB_STATUS_OK ||
            core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            result.ticks != 3u || result.elapsed_ticks != 3u ||
            core_machine_reset(machine) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, TIMING_RESET_LINEAR, jcc,
                sizeof(jcc)) != LIB_STATUS_OK ||
            core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            result.ticks != 9u || result.elapsed_ticks != 9u ||
            core_machine_reset(machine) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, TIMING_RESET_LINEAR,
                classified_unqualified, sizeof(classified_unqualified)) != LIB_STATUS_OK ||
            core_machine_run(machine, budget, &result) != LIB_STATUS_INTERNAL_ERROR ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != 0u ||
            core_machine_reset(machine) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, TIMING_RESET_LINEAR,
                equivalent_prefixed_nop, sizeof(equivalent_prefixed_nop)) !=
                LIB_STATUS_OK || core_machine_run(machine, budget, &result) !=
                LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            result.executed != 1u || result.ticks != 3u ||
            result.elapsed_ticks != 3u;
    }
    if (!failed) {
        printf("M5:T394:S4:ELIGIBILITY-KEY:OK\n");
        printf("M5:T394:S4:PHYSICAL-ABSENT-KEY:OK\n");
    }
    core_machine_destroy(machine);
    return failed;
}
lib_i32 main(void)
{
    static const lib_u8 nop[] = { 0x90u };
    static const lib_u8 register_mov[] = { 0xb8u, 0x34u, 0x12u };
    static const lib_u8 prefixed_nop[] = { 0x26u, 0x90u };
    static const lib_u8 memory_mov[] = { 0xa0u, 0x00u, 0x00u };
    static const lib_u8 out_port[] = { 0xe6u, 0xe0u };
    static const lib_u8 in_port[] = { 0xe4u, 0xe0u };
    static const lib_u8 taken_branch[] = { 0x31u, 0xc0u, 0x74u, 0x01u, 0x90u };
    static const lib_u8 not_taken_branch[] = { 0x31u, 0xc0u, 0x75u, 0x00u };
    static const lib_u8 rep_movsb[] = { 0xb9u, 0x03u, 0x00u, 0xf3u, 0xa4u };
    lib_i32 failed = 0;

    failed |= timing_case(nop, sizeof(nop), 1u, 3u);
    failed |= timing_case(register_mov, sizeof(register_mov), 1u, 2u);
    failed |= timing_case(prefixed_nop, sizeof(prefixed_nop), 1u, 3u);
    failed |= timing_case(memory_mov, sizeof(memory_mov), 1u, 5u);
    failed |= timing_case(out_port, sizeof(out_port), 1u, 3u);
    failed |= timing_case(in_port, sizeof(in_port), 1u, 5u);
    if (failed) return 1;
    if (timing_case(taken_branch, sizeof(taken_branch), 2u, 9u)) return 2;
    if (timing_case(not_taken_branch, sizeof(not_taken_branch), 2u, 5u)) return 3;
    if (timing_case(rep_movsb, sizeof(rep_movsb), 4u, 19u)) return 4;
    if (timing_test_quantum_and_reset()) return 5;
    if (timing_test_fault()) return 6;
    if (timing_test_stop()) return 7;
    if (timing_test_invalid_qualification()) return 8;
    if (timing_test_physical_contract()) return 9;
    printf("M5:T265:S3:INSTRUCTION-TIMING:OK\n");
    return 0;
}
