#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define T359_S3_RESET_LINEAR 0xfffffff0u
#define T359_S3_RESET_PHYSICAL 0x000ffff0u
#define T359_S3_WINDOW_BYTES 16u

typedef struct t359_s3_timing_state {
    type_unsigned_64 advanced_ticks;
} t359_s3_timing_state;

typedef C_INT (*t359_s3_setup)(core_machine *machine, C_VOID *opaque);

typedef struct t359_s3_interrupt_setup {
    type_unsigned_8 vector;
    C_INT set_overflow;
} t359_s3_interrupt_setup;

static C_VOID t359_s3_timing_reset(C_VOID *opaque)
{
    t359_s3_timing_state *state = (t359_s3_timing_state *)opaque;

    if (state != STD_NULL) state->advanced_ticks = 0u;
}

static C_VOID t359_s3_timing_advance(C_VOID *opaque,
    type_unsigned_64 elapsed_ticks)
{
    t359_s3_timing_state *state = (t359_s3_timing_state *)opaque;

    if (state != STD_NULL) state->advanced_ticks += elapsed_ticks;
}

static const core_machine_execution_provider t359_s3_timing_provider = {
    t359_s3_timing_reset,
    t359_s3_timing_advance
};

static C_INT t359_s3_prepare(core_machine_cpu_profile profile,
    core_machine **out_machine, t359_s3_timing_state *state)
{
    const core_machine_config config = { .cpu_profile = profile };
    core_machine *machine = STD_NULL;

    if (out_machine == STD_NULL || state == STD_NULL ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            T359_S3_RESET_LINEAR, T359_S3_RESET_PHYSICAL,
            T359_S3_WINDOW_BYTES) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine, 0x00001000u,
            0x00001000u, T359_S3_WINDOW_BYTES) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(machine,
            &t359_s3_timing_provider, state)) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static C_INT t359_s3_run(core_machine *machine,
    const type_unsigned_8 *program, STD_SIZE_T program_bytes,
    type_unsigned_64 expected_ticks, t359_s3_timing_state *state,
    t359_s3_setup setup, C_VOID *opaque)
{
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;

    return machine != STD_NULL && program != STD_NULL && state != STD_NULL &&
        core_machine_reset(machine) == TYPE_STATUS_OK &&
        (setup == STD_NULL || setup(machine, opaque)) &&
        core_machine_memory_write(machine, T359_S3_RESET_LINEAR, program,
            program_bytes) == TYPE_STATUS_OK &&
        core_machine_run(machine, budget, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET && result.executed == 1u &&
        result.ticks == expected_ticks && result.elapsed_ticks == expected_ticks &&
        state->advanced_ticks == expected_ticks;
}

static C_INT t359_s3_seed_pop(core_machine *machine, C_VOID *opaque)
{
    const type_unsigned_16 value = 0x1234u;

    (C_VOID)opaque;
    if (machine == STD_NULL) return 0;
    machine->executor_cpu.data.sp = 0x1000u;
    return core_machine_memory_write(machine, 0x1000u, &value, sizeof(value)) ==
        TYPE_STATUS_OK;
}

static C_INT t359_s3_seed_loop_not_taken(core_machine *machine, C_VOID *opaque)
{
    (C_VOID)opaque;
    if (machine == STD_NULL) return 0;
    machine->executor_cpu.data.cx = 1u;
    return 1;
}

static C_INT t359_s3_seed_transfer(core_machine *machine, C_VOID *opaque)
{
    const type_unsigned_16 target = 0xfff5u;
    const type_unsigned_8 nop = 0x90u;

    (C_VOID)opaque;
    if (machine == STD_NULL) return 0;
    machine->executor_cpu.data.ax = target;
    machine->executor_cpu.data.sp = 0x1020u;
    return core_machine_memory_write(machine, 0x1000u, &target,
        sizeof(target)) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, 0x000ffff5u, &nop, sizeof(nop)) ==
        TYPE_STATUS_OK && core_machine_memory_write(machine, 0x1020u, &target,
            sizeof(target)) == TYPE_STATUS_OK;
}

static C_INT t359_s3_seed_interrupt(core_machine *machine, C_VOID *opaque)
{
    const t359_s3_interrupt_setup *setup =
        (const t359_s3_interrupt_setup *)opaque;
    const type_unsigned_16 offset = 0xfff5u;
    const type_unsigned_16 segment = 0xf000u;
    const type_unsigned_8 nop = 0x90u;
    type_unsigned_32 vector;

    if (machine == STD_NULL || setup == STD_NULL) return 0;
    vector = (type_unsigned_32)setup->vector * 4u;
    if (setup->set_overflow) machine->executor_cpu.data.eflags |= VCPU_EFLAGS_OF;
    return
        core_machine_memory_write(machine, vector, &offset,
            sizeof(offset)) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, vector + sizeof(offset), &segment,
            sizeof(segment)) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, 0x000ffff5u, &nop,
            sizeof(nop)) == TYPE_STATUS_OK;
}

static C_INT t359_s3_seed_iret(core_machine *machine, C_VOID *opaque)
{
    const type_unsigned_16 frame[] = { 0xfff5u, 0xf000u, 0x0002u };
    const type_unsigned_8 nop = 0x90u;

    (C_VOID)opaque;
    if (machine == STD_NULL) return 0;
    machine->executor_cpu.data.sp = 0x1020u;
    return core_machine_memory_write(machine, 0x1020u, frame,
        sizeof(frame)) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, 0x000ffff5u, &nop,
            sizeof(nop)) == TYPE_STATUS_OK;
}

static C_INT t359_s3_test_profile_rows(core_machine_cpu_profile profile,
    type_unsigned_64 call_ticks, type_unsigned_64 jmp_ticks,
    type_unsigned_64 push_ticks, type_unsigned_64 pop_ticks,
    type_unsigned_64 loop_ticks, type_unsigned_64 jcxz_ticks,
    type_unsigned_64 hlt_ticks)
{
    static const type_unsigned_8 call_near[] = {
        0xe8u, 0x02u, 0x00u, 0x90u, 0x90u, 0x90u
    };
    static const type_unsigned_8 jmp_near[] = {
        0xebu, 0x01u, 0x90u, 0x90u
    };
    static const type_unsigned_8 push_register[] = { 0x50u };
    static const type_unsigned_8 pop_register[] = { 0x58u };
    static const type_unsigned_8 loop_not_taken[] = { 0xe2u, 0x00u, 0x90u };
    static const type_unsigned_8 jcxz_not_taken[] = { 0xe3u, 0x00u, 0x90u };
    static const type_unsigned_8 hlt[] = { 0xf4u };
    t359_s3_timing_state state = { 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !t359_s3_prepare(profile, &machine, &state);

    if (!failed) {
        failed |= !t359_s3_run(machine, call_near, sizeof(call_near),
            call_ticks, &state, STD_NULL, STD_NULL) ||
            machine->executor_cpu.data.ip != 0xfff5u;
    }
    if (!failed) {
        failed |= !t359_s3_run(machine, jmp_near, sizeof(jmp_near), jmp_ticks,
            &state, STD_NULL, STD_NULL) || machine->executor_cpu.data.ip != 0xfff3u;
    }
    if (!failed) {
        failed |= !t359_s3_run(machine, push_register, sizeof(push_register),
            push_ticks, &state, STD_NULL, STD_NULL);
    }
    if (!failed) {
        failed |= !t359_s3_run(machine, pop_register, sizeof(pop_register),
            pop_ticks, &state, t359_s3_seed_pop, STD_NULL) ||
            machine->executor_cpu.data.ax != 0x1234u;
    }
    if (!failed) {
        failed |= !t359_s3_run(machine, loop_not_taken, sizeof(loop_not_taken),
            loop_ticks, &state, t359_s3_seed_loop_not_taken, STD_NULL) ||
            machine->executor_cpu.data.cx != 0u;
    }
    if (!failed) {
        failed |= !t359_s3_run(machine, jcxz_not_taken,
            sizeof(jcxz_not_taken), jcxz_ticks, &state,
            t359_s3_seed_loop_not_taken, STD_NULL);
    }
    if (!failed) {
        const core_machine_run_budget budget = { 1u, 0u };
        core_machine_run_result result;

        failed |= core_machine_reset(machine) != TYPE_STATUS_OK ||
            core_machine_memory_write(machine, T359_S3_RESET_LINEAR, hlt,
                sizeof(hlt)) != TYPE_STATUS_OK ||
            core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            result.ticks != hlt_ticks || state.advanced_ticks != hlt_ticks;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s3_test_80186_stack_rows(C_VOID)
{
    static const type_unsigned_8 pusha[] = { 0x60u };
    static const type_unsigned_8 popa[] = { 0x61u };
    static const type_unsigned_8 enter_zero[] = {
        0xc8u, 0x00u, 0x00u, 0x00u
    };
    static const type_unsigned_8 leave[] = { 0xc9u };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_64 pusha_ticks[] = { 36u, 17u, 18u };
    static const type_unsigned_64 popa_ticks[] = { 51u, 19u, 24u };
    static const type_unsigned_64 enter_ticks[] = { 15u, 11u, 10u };
    static const type_unsigned_64 leave_ticks[] = { 8u, 8u, 4u };
    type_unsigned_32 index;

    for (index = 0u; index < sizeof(profiles) / sizeof(profiles[0u]); ++index) {
        t359_s3_timing_state state = { 0u };
        core_machine *machine = STD_NULL;
        C_INT failed = !t359_s3_prepare(profiles[index], &machine, &state);

        if (!failed) {
            failed |= !t359_s3_run(machine, pusha, sizeof(pusha),
                pusha_ticks[index], &state, STD_NULL, STD_NULL) ||
                !t359_s3_run(machine, popa, sizeof(popa), popa_ticks[index],
                    &state, STD_NULL, STD_NULL) ||
                !t359_s3_run(machine, enter_zero, sizeof(enter_zero),
                    enter_ticks[index], &state, STD_NULL, STD_NULL) ||
                !t359_s3_run(machine, leave, sizeof(leave), leave_ticks[index],
                    &state, STD_NULL, STD_NULL);
        }
        core_machine_destroy(machine);
        if (failed) return 1;
    }
    return 0;
}

static C_INT t359_s3_test_transfer_shapes(core_machine_cpu_profile profile,
    type_unsigned_64 call_register_ticks, type_unsigned_64 call_memory_ticks,
    type_unsigned_64 jmp_register_ticks, type_unsigned_64 jmp_memory_ticks,
    type_unsigned_64 ret_ticks, type_unsigned_64 push_memory_ticks,
    type_unsigned_64 pop_memory_ticks)
{
    static const type_unsigned_8 call_register[] = { 0xffu, 0xd0u };
    static const type_unsigned_8 call_memory[] = {
        0xffu, 0x16u, 0x00u, 0x10u
    };
    static const type_unsigned_8 jmp_register[] = { 0xffu, 0xe0u };
    static const type_unsigned_8 jmp_memory[] = {
        0xffu, 0x26u, 0x00u, 0x10u
    };
    static const type_unsigned_8 ret_near[] = { 0xc3u };
    static const type_unsigned_8 push_memory[] = {
        0xffu, 0x36u, 0x00u, 0x10u
    };
    static const type_unsigned_8 pop_memory[] = {
        0x8fu, 0x06u, 0x00u, 0x10u
    };
    t359_s3_timing_state state = { 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !t359_s3_prepare(profile, &machine, &state);

    if (!failed) {
        failed |= !t359_s3_run(machine, call_register, sizeof(call_register),
            call_register_ticks, &state, t359_s3_seed_transfer, STD_NULL) ||
            machine->executor_cpu.data.ip != 0xfff5u;
    }
    if (!failed) {
        failed |= !t359_s3_run(machine, call_memory, sizeof(call_memory),
            call_memory_ticks, &state, t359_s3_seed_transfer, STD_NULL) ||
            machine->executor_cpu.data.ip != 0xfff5u;
    }
    if (!failed) {
        failed |= !t359_s3_run(machine, jmp_register, sizeof(jmp_register),
            jmp_register_ticks, &state, t359_s3_seed_transfer, STD_NULL) ||
            machine->executor_cpu.data.ip != 0xfff5u;
    }
    if (!failed) {
        failed |= !t359_s3_run(machine, jmp_memory, sizeof(jmp_memory),
            jmp_memory_ticks, &state, t359_s3_seed_transfer, STD_NULL) ||
            machine->executor_cpu.data.ip != 0xfff5u;
    }
    if (!failed) {
        failed |= !t359_s3_run(machine, ret_near, sizeof(ret_near), ret_ticks,
            &state, t359_s3_seed_transfer, STD_NULL) ||
            machine->executor_cpu.data.ip != 0xfff5u;
    }
    if (!failed) {
        failed |= !t359_s3_run(machine, push_memory, sizeof(push_memory),
            push_memory_ticks, &state, t359_s3_seed_transfer, STD_NULL);
    }
    if (!failed) {
        failed |= !t359_s3_run(machine, pop_memory, sizeof(pop_memory),
            pop_memory_ticks, &state, t359_s3_seed_transfer, STD_NULL) ||
            machine->executor_cpu.data.ip != 0xfff4u;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s3_test_far_direct(core_machine_cpu_profile profile,
    type_unsigned_64 call_ticks, type_unsigned_64 jmp_ticks)
{
    static const type_unsigned_8 call_far[] = {
        0x9au, 0xf5u, 0xffu, 0x00u, 0xf0u
    };
    static const type_unsigned_8 jmp_far[] = {
        0xeau, 0xf5u, 0xffu, 0x00u, 0xf0u
    };
    t359_s3_timing_state state = { 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !t359_s3_prepare(profile, &machine, &state);

    if (!failed) {
        failed |= !t359_s3_run(machine, call_far, sizeof(call_far), call_ticks,
            &state, t359_s3_seed_transfer, STD_NULL) ||
            machine->executor_cpu.data.cs.selector != 0xf000u ||
            machine->executor_cpu.data.ip != 0xfff5u;
    }
    if (!failed) {
        failed |= !t359_s3_run(machine, jmp_far, sizeof(jmp_far), jmp_ticks,
            &state, t359_s3_seed_transfer, STD_NULL) ||
            machine->executor_cpu.data.cs.selector != 0xf000u ||
            machine->executor_cpu.data.ip != 0xfff5u;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT t359_s3_test_real_interrupt_rows(core_machine_cpu_profile profile,
    type_unsigned_64 int_ticks, type_unsigned_64 into_clear_ticks,
    type_unsigned_64 int3_ticks, type_unsigned_64 into_taken_ticks,
    type_unsigned_64 iret_ticks)
{
    static const type_unsigned_8 int_immediate[] = { 0xcdu, 0x20u };
    static const type_unsigned_8 int3[] = { 0xccu };
    static const type_unsigned_8 into_clear[] = { 0xceu };
    static const type_unsigned_8 into_taken[] = { 0xceu };
    static const type_unsigned_8 iret[] = { 0xcfu };
    static const t359_s3_interrupt_setup int_setup = { 0x20u, 0 };
    static const t359_s3_interrupt_setup int3_setup = { 0x03u, 0 };
    static const t359_s3_interrupt_setup into_setup = { 0x04u, 1 };
    t359_s3_timing_state state = { 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !t359_s3_prepare(profile, &machine, &state);

    if (!failed) {
        failed |= !t359_s3_run(machine, int_immediate, sizeof(int_immediate),
            int_ticks, &state, t359_s3_seed_interrupt, (C_VOID *)&int_setup) ||
            machine->executor_cpu.data.cs.selector != 0xf000u ||
            machine->executor_cpu.data.ip != 0xfff5u;
    }
    if (!failed) {
        failed |= !t359_s3_run(machine, int3, sizeof(int3), int3_ticks, &state,
            t359_s3_seed_interrupt, (C_VOID *)&int3_setup) ||
            machine->executor_cpu.data.cs.selector != 0xf000u ||
            machine->executor_cpu.data.ip != 0xfff5u;
    }
    if (!failed) {
        failed |= !t359_s3_run(machine, into_clear, sizeof(into_clear),
            into_clear_ticks, &state, STD_NULL, STD_NULL);
    }
    if (!failed) {
        failed |= !t359_s3_run(machine, into_taken, sizeof(into_taken),
            into_taken_ticks, &state, t359_s3_seed_interrupt,
            (C_VOID *)&into_setup) ||
            machine->executor_cpu.data.cs.selector != 0xf000u ||
            machine->executor_cpu.data.ip != 0xfff5u;
    }
    if (!failed) {
        failed |= !t359_s3_run(machine, iret, sizeof(iret), iret_ticks,
            &state, t359_s3_seed_iret, STD_NULL) ||
            machine->executor_cpu.data.cs.selector != 0xf000u ||
            machine->executor_cpu.data.ip != 0xfff5u;
    }
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    (C_VOID)argc;
    (C_VOID)argv;

    if (t359_s3_test_profile_rows(CORE_MACHINE_CPU_PROFILE_8086,
            19u, 15u, 11u, 8u, 5u, 6u, 2u)) return 1;
    if (t359_s3_test_profile_rows(CORE_MACHINE_CPU_PROFILE_80186,
            15u, 14u, 10u, 10u, 5u, 5u, 2u)) return 2;
    if (t359_s3_test_profile_rows(CORE_MACHINE_CPU_PROFILE_80286,
            8u, 8u, 3u, 5u, 4u, 4u, 2u)) return 3;
    if (t359_s3_test_profile_rows(CORE_MACHINE_CPU_PROFILE_80386,
            8u, 8u, 2u, 4u, 12u, 6u, 5u)) return 4;
    if (t359_s3_test_80186_stack_rows()) return 5;
    if (t359_s3_test_transfer_shapes(CORE_MACHINE_CPU_PROFILE_8086,
            16u, 27u, 11u, 24u, 8u, 22u, 23u)) return 6;
    if (t359_s3_test_transfer_shapes(CORE_MACHINE_CPU_PROFILE_80186,
            13u, 19u, 11u, 17u, 16u, 16u, 20u)) return 7;
    if (t359_s3_test_transfer_shapes(CORE_MACHINE_CPU_PROFILE_80286,
            8u, 12u, 8u, 12u, 12u, 5u, 5u)) return 8;
    if (t359_s3_test_transfer_shapes(CORE_MACHINE_CPU_PROFILE_80386,
            8u, 11u, 8u, 11u, 11u, 5u, 5u)) return 9;
    if (t359_s3_test_far_direct(CORE_MACHINE_CPU_PROFILE_8086, 28u, 15u)) {
        return 10;
    }
    if (t359_s3_test_far_direct(CORE_MACHINE_CPU_PROFILE_80186, 23u, 14u)) {
        return 11;
    }
    if (t359_s3_test_far_direct(CORE_MACHINE_CPU_PROFILE_80286, 14u, 12u)) {
        return 12;
    }
    if (t359_s3_test_far_direct(CORE_MACHINE_CPU_PROFILE_80386, 18u, 13u)) {
        return 13;
    }
    if (t359_s3_test_real_interrupt_rows(CORE_MACHINE_CPU_PROFILE_8086,
            51u, 4u, 52u, 53u, 24u)) return 14;
    if (t359_s3_test_real_interrupt_rows(CORE_MACHINE_CPU_PROFILE_80186,
            47u, 4u, 45u, 48u, 28u)) return 15;
    if (t359_s3_test_real_interrupt_rows(CORE_MACHINE_CPU_PROFILE_80286,
            24u, 5u, 24u, 25u, 18u)) return 16;
    if (t359_s3_test_real_interrupt_rows(CORE_MACHINE_CPU_PROFILE_80386,
            37u, 3u, 33u, 35u, 22u)) return 17;
    STD_PRINTF("M5:T359:S3:CONTROL-STACK-TIMING:OK\n");
    return 0;
}
