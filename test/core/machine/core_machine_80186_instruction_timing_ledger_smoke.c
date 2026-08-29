#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define TIMING_80186_RESET_LINEAR 0x000ffff0u
#define TIMING_80186_RESET_PHYSICAL 0x000ffff0u

typedef struct timing_80186_state {
    type_unsigned_32 reads;
    type_unsigned_32 writes;
    type_unsigned_64 advanced_ticks;
} timing_80186_state;

static type_status timing_80186_read(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 *out_value)
{
    timing_80186_state *state = (timing_80186_state *)owner;

    if (state == STD_NULL || out_value == STD_NULL || port != 0x00e0u)
        return TYPE_STATUS_INVALID_ARGUMENT;
    ++state->reads;
    *out_value = 0x5au;
    return TYPE_STATUS_OK;
}

static type_status timing_80186_write(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 value)
{
    timing_80186_state *state = (timing_80186_state *)owner;

    if (state == STD_NULL || port != 0x00e0u || value > 0xffffu)
        return TYPE_STATUS_INVALID_ARGUMENT;
    ++state->writes;
    return TYPE_STATUS_OK;
}

static const core_machine_port_provider timing_80186_ports = {
    timing_80186_read, timing_80186_write
};

static C_VOID timing_80186_reset(C_VOID *opaque)
{
    timing_80186_state *state = (timing_80186_state *)opaque;
    if (state != STD_NULL) state->advanced_ticks = 0u;
}

static C_VOID timing_80186_advance(C_VOID *opaque, type_unsigned_64 ticks)
{
    timing_80186_state *state = (timing_80186_state *)opaque;
    if (state != STD_NULL) state->advanced_ticks += ticks;
}

static const core_machine_execution_provider timing_80186_execution = {
    timing_80186_reset, timing_80186_advance
};

static C_INT timing_80186_prepare(core_machine **out_machine,
    timing_80186_state *state)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80186,
        .ticks_per_instruction = 29u,
        .instruction_timing = { 29u, 7u, 31u, 37u, 41u, 43u }
    };
    core_machine *machine = STD_NULL;

    if (out_machine == STD_NULL || state == STD_NULL ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_80186_RESET_LINEAR, TIMING_80186_RESET_PHYSICAL, 16u) !=
            TYPE_STATUS_OK || core_machine_install_port_provider(machine,
            0x00e0u, 0x00e0u, &timing_80186_ports, state) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(machine,
            &timing_80186_execution, state)) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static C_INT timing_80186_load(core_machine *machine,
    const type_unsigned_8 *program, STD_SIZE_T bytes)
{
    return core_machine_reset(machine) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, TIMING_80186_RESET_LINEAR,
            program, bytes) == TYPE_STATUS_OK;
}

static C_INT timing_80186_run(core_machine *machine, timing_80186_state *state,
    type_unsigned_64 instructions, type_unsigned_64 ticks)
{
    const core_machine_run_budget budget = { instructions, 0u };
    core_machine_run_result result;

    return core_machine_run(machine, budget, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET &&
        result.executed == instructions && result.ticks == ticks &&
        result.elapsed_ticks == ticks && state->advanced_ticks == ticks;
}

static C_INT timing_80186_case(const type_unsigned_8 *program, STD_SIZE_T bytes,
    type_unsigned_64 ticks)
{
    timing_80186_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80186_prepare(&machine, &state) ||
        !timing_80186_load(machine, program, bytes) ||
        !timing_80186_run(machine, &state, 1u, ticks);

    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80186_lea(C_VOID)
{
    static const type_unsigned_8 program[] = { 0x8du, 0x1eu, 0x00u, 0x10u };
    timing_80186_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80186_prepare(&machine, &state) ||
        !timing_80186_load(machine, program, sizeof(program)) ||
        !timing_80186_run(machine, &state, 1u, 6u) ||
        machine->executor_cpu.data.bx != 0x1000u;

    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80186_pointer_loads(C_VOID)
{
    static const type_unsigned_8 lds[] = { 0xc5u, 0x1eu, 0x00u, 0x10u };
    static const type_unsigned_8 les[] = { 0xc4u, 0x1eu, 0x00u, 0x10u };
    static const type_unsigned_16 pointer[] = { 0x2000u, 0x0800u };
    timing_80186_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80186_prepare(&machine, &state);

    if (!failed) failed |= !timing_80186_load(machine, lds, sizeof(lds)) ||
        core_machine_memory_write(machine, 0x1000u, pointer, sizeof(pointer)) !=
            TYPE_STATUS_OK || !timing_80186_run(machine, &state, 1u, 18u) ||
        machine->executor_cpu.data.bx != 0x2000u ||
        machine->executor_cpu.data.ds.selector != 0x0800u;
    if (!failed) failed |= !timing_80186_load(machine, les, sizeof(les)) ||
        core_machine_memory_write(machine, 0x1000u, pointer, sizeof(pointer)) !=
            TYPE_STATUS_OK || !timing_80186_run(machine, &state, 1u, 18u) ||
        machine->executor_cpu.data.bx != 0x2000u ||
        machine->executor_cpu.data.es.selector != 0x0800u;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80186_bound(C_VOID)
{
    static const type_unsigned_8 program[] = { 0x62u, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_16 bounds[] = { 0x1000u, 0x2000u };
    timing_80186_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80186_prepare(&machine, &state) ||
        !timing_80186_load(machine, program, sizeof(program)) ||
        core_machine_memory_write(machine, 0x1000u, bounds, sizeof(bounds)) !=
            TYPE_STATUS_OK || ((machine->executor_cpu.data.ax = 0x1800u), 0) ||
        !timing_80186_run(machine, &state, 1u, 34u);

    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80186_alu_matrix(C_VOID)
{
    static const type_unsigned_8 bases[] = {
        0x00u, 0x08u, 0x10u, 0x18u, 0x20u, 0x28u, 0x30u
    };
    static const type_unsigned_64 ticks[] = { 3u, 10u, 10u, 4u, 16u, 4u };
    STD_SIZE_T operation;
    STD_SIZE_T form;

    for (operation = 0u; operation < sizeof(bases) / sizeof(bases[0]); ++operation) {
        for (form = 0u; form < sizeof(ticks) / sizeof(ticks[0]); ++form) {
            type_unsigned_8 program[5] = { 0 };
            STD_SIZE_T bytes;
            const type_unsigned_16 value = 1u;
            timing_80186_state state = { 0u, 0u, 0u };
            core_machine *machine = STD_NULL;
            C_INT failed;

            switch (form) {
            case 0u: program[0] = bases[operation] + 2u; program[1] = 0xc1u;
                bytes = 2u; break;
            case 1u: program[0] = bases[operation] + 2u; program[1] = 0x06u;
                program[2] = 0u; program[3] = 0x10u; bytes = 4u; break;
            case 2u: program[0] = bases[operation]; program[1] = 0x0eu;
                program[2] = 0u; program[3] = 0x10u; bytes = 4u; break;
            case 3u: program[0] = 0x80u; program[1] =
                (type_unsigned_8)(0xc0u | (operation << 3u)); program[2] = 1u;
                bytes = 3u; break;
            case 4u: program[0] = 0x80u; program[1] =
                (type_unsigned_8)(0x06u | (operation << 3u)); program[2] = 0u;
                program[3] = 0x10u; program[4] = 1u; bytes = 5u; break;
            default: program[0] = bases[operation] + 4u; program[1] = 1u;
                bytes = 2u; break;
            }
            failed = !timing_80186_prepare(&machine, &state) ||
                !timing_80186_load(machine, program, bytes) ||
                core_machine_memory_write(machine, 0x1000u, &value,
                    sizeof(value)) != TYPE_STATUS_OK ||
                ((machine->executor_cpu.data.ax = 1u),
                    (machine->executor_cpu.data.cx = 1u), 0) ||
                !timing_80186_run(machine, &state, 1u, ticks[form]);
            core_machine_destroy(machine);
            if (failed) {
                STD_PRINTF("I186 ALU timing case failed: operation=%u form=%u\n",
                    (type_unsigned_32)operation, (type_unsigned_32)form);
                return 1;
            }
        }
    }
    return 0;
}

static C_INT timing_80186_cmp_test_matrix(C_VOID)
{
    typedef struct timing_80186_recipe {
        type_unsigned_8 program[5];
        STD_SIZE_T bytes;
        type_unsigned_64 ticks;
    } timing_80186_recipe;
    static const timing_80186_recipe recipes[] = {
        { { 0x3au,0xc1u }, 2u, 3u }, { { 0x3au,0x06u,0u,0x10u }, 4u, 10u },
        { { 0x38u,0x0eu,0u,0x10u }, 4u, 10u }, { { 0x80u,0xf8u,1u }, 3u, 3u },
        { { 0x80u,0x3eu,0u,0x10u,1u }, 5u, 10u }, { { 0x3cu,1u }, 2u, 4u },
        { { 0x84u,0xc1u }, 2u, 3u }, { { 0x84u,0x06u,0u,0x10u }, 4u, 10u },
        { { 0xf6u,0xc0u,1u }, 3u, 4u }, { { 0xf6u,0x06u,0u,0x10u,1u }, 5u, 10u },
        { { 0xa8u,1u }, 2u, 4u }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        const type_unsigned_16 value = 1u;
        timing_80186_state state = { 0u, 0u, 0u };
        core_machine *machine = STD_NULL;
        C_INT failed = !timing_80186_prepare(&machine, &state) ||
            !timing_80186_load(machine, recipes[index].program,
                recipes[index].bytes) || core_machine_memory_write(machine,
                0x1000u, &value, sizeof(value)) != TYPE_STATUS_OK ||
            ((machine->executor_cpu.data.ax = 1u),
                (machine->executor_cpu.data.cx = 1u), 0) ||
            !timing_80186_run(machine, &state, 1u, recipes[index].ticks);

        core_machine_destroy(machine);
        if (failed) {
            STD_PRINTF("I186 CMP/TEST timing case failed: index=%u\n",
                (type_unsigned_32)index);
            return 1;
        }
    }
    return 0;
}

static C_INT timing_80186_adjustment_matrix(C_VOID)
{
    typedef struct timing_80186_adjustment_recipe {
        type_unsigned_8 program[2];
        STD_SIZE_T bytes;
        type_unsigned_64 ticks;
    } timing_80186_adjustment_recipe;
    static const timing_80186_adjustment_recipe recipes[] = {
        { { 0x37u }, 1u, 8u }, { { 0x3fu }, 1u, 7u },
        { { 0xd5u,0x0au }, 2u, 15u }, { { 0xd4u,0x0au }, 2u, 19u },
        { { 0x98u }, 1u, 2u }, { { 0x99u }, 1u, 4u },
        { { 0x27u }, 1u, 4u }, { { 0x2fu }, 1u, 4u }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        timing_80186_state state = { 0u, 0u, 0u };
        core_machine *machine = STD_NULL;
        C_INT failed = !timing_80186_prepare(&machine, &state) ||
            !timing_80186_load(machine, recipes[index].program,
                recipes[index].bytes) || ((machine->executor_cpu.data.ax = 0x0012u),
                0) || !timing_80186_run(machine, &state, 1u, recipes[index].ticks);

        core_machine_destroy(machine);
        if (failed) return 1;
    }
    return 0;
}

static C_INT timing_80186_unary_matrix(C_VOID)
{
    typedef struct timing_80186_recipe {
        type_unsigned_8 program[4];
        STD_SIZE_T bytes;
        type_unsigned_64 ticks;
    } timing_80186_recipe;
    static const timing_80186_recipe recipes[] = {
        { { 0x40u },1u,3u }, { { 0xfeu,0xc0u },2u,3u }, { { 0xffu,0x06u,0u,0x10u },4u,15u },
        { { 0x48u },1u,3u }, { { 0xfeu,0xc8u },2u,3u }, { { 0xffu,0x0eu,0u,0x10u },4u,15u },
        { { 0xf6u,0xd8u },2u,3u }, { { 0xf6u,0x1eu,0u,0x10u },4u,3u },
        { { 0xf6u,0xd0u },2u,3u }, { { 0xf6u,0x16u,0u,0x10u },4u,3u },
        { { 0x91u },1u,3u }, { { 0x87u,0xc1u },2u,4u }, { { 0x87u,0x06u,0u,0x10u },4u,17u }
    };
    STD_SIZE_T index;
    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        const type_unsigned_16 value = 1u;
        timing_80186_state state = { 0u,0u,0u };
        core_machine *machine = STD_NULL;
        C_INT failed = !timing_80186_prepare(&machine, &state) || !timing_80186_load(machine,
            recipes[index].program, recipes[index].bytes) || core_machine_memory_write(machine,
            0x1000u, &value, sizeof(value)) != TYPE_STATUS_OK || !timing_80186_run(machine,
            &state, 1u, recipes[index].ticks);
        core_machine_destroy(machine);
        if (failed) return 1;
    }
    return 0;
}

static C_INT timing_80186_flag_controls(C_VOID)
{
    typedef struct timing_80186_flag_recipe {
        type_unsigned_8 opcode;
        type_unsigned_32 initial_flags;
        type_unsigned_32 required_set;
        type_unsigned_32 required_clear;
        type_unsigned_64 ticks;
    } timing_80186_flag_recipe;
    static const timing_80186_flag_recipe recipes[] = {
        { 0xf9u, 0u, VCPU_EFLAGS_CF, 0u, 2u },
        { 0xfau, VCPU_EFLAGS_IF, 0u, VCPU_EFLAGS_IF, 2u },
        { 0xfbu, 0u, VCPU_EFLAGS_IF, 0u, 2u },
        { 0xfcu, VCPU_EFLAGS_DF, 0u, VCPU_EFLAGS_DF, 2u },
        { 0xfdu, 0u, VCPU_EFLAGS_DF, 0u, 2u },
        { 0x90u, 0u, 0u, 0u, 3u }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        const timing_80186_flag_recipe *recipe = &recipes[index];
        timing_80186_state state = { 0u,0u,0u };
        core_machine *machine = STD_NULL;
        C_INT failed = !timing_80186_prepare(&machine, &state) ||
            !timing_80186_load(machine, &recipe->opcode, 1u) ||
            ((machine->executor_cpu.data.eflags = recipe->initial_flags), 0) ||
            !timing_80186_run(machine, &state, 1u, recipe->ticks) ||
            (machine->executor_cpu.data.eflags & recipe->required_set) !=
                recipe->required_set ||
            (machine->executor_cpu.data.eflags & recipe->required_clear) != 0u;

        core_machine_destroy(machine);
        if (failed) return 1;
    }
    return 0;
}

static C_INT timing_80186_group2_matrix(C_VOID)
{
    static const type_unsigned_8 operations[] = { 0u,1u,2u,3u,4u,5u,7u };
    static const type_unsigned_64 ticks[] = { 2u,8u,15u,20u,8u,20u };
    STD_SIZE_T operation;
    STD_SIZE_T form;

    for (operation = 0u; operation < sizeof(operations) / sizeof(operations[0]);
            ++operation) {
        for (form = 0u; form < sizeof(ticks) / sizeof(ticks[0]); ++form) {
            type_unsigned_8 program[5] = { 0 };
            const type_unsigned_16 value = 1u;
            STD_SIZE_T bytes;
            timing_80186_state state = { 0u,0u,0u };
            core_machine *machine = STD_NULL;
            C_INT failed;

            switch (form) {
            case 0u: program[0] = 0xd0u; program[1] =
                (type_unsigned_8)(0xc0u | (operations[operation] << 3u));
                bytes = 2u; break;
            case 1u: program[0] = 0xd2u; program[1] =
                (type_unsigned_8)(0xc0u | (operations[operation] << 3u));
                bytes = 2u; break;
            case 2u: program[0] = 0xd0u; program[1] =
                (type_unsigned_8)(0x06u | (operations[operation] << 3u));
                program[2] = 0u; program[3] = 0x10u; bytes = 4u; break;
            case 3u: program[0] = 0xd2u; program[1] =
                (type_unsigned_8)(0x06u | (operations[operation] << 3u));
                program[2] = 0u; program[3] = 0x10u; bytes = 4u; break;
            case 4u: program[0] = 0xc0u; program[1] =
                (type_unsigned_8)(0xc0u | (operations[operation] << 3u));
                program[2] = 3u; bytes = 3u; break;
            default: program[0] = 0xc0u; program[1] =
                (type_unsigned_8)(0x06u | (operations[operation] << 3u));
                program[2] = 0u; program[3] = 0x10u; program[4] = 3u;
                bytes = 5u; break;
            }
            failed = !timing_80186_prepare(&machine, &state) ||
                !timing_80186_load(machine, program, bytes) ||
                core_machine_memory_write(machine, 0x1000u, &value,
                    sizeof(value)) != TYPE_STATUS_OK ||
                ((machine->executor_cpu.data.cx = 3u), 0) ||
                !timing_80186_run(machine, &state, 1u, ticks[form]);
            core_machine_destroy(machine);
            if (failed) return 1;
        }
    }
    return 0;
}

static C_INT timing_80186_l2_arithmetic_matrix(C_VOID)
{
    typedef struct timing_80186_recipe {
        type_unsigned_8 program[5];
        STD_SIZE_T bytes;
        type_unsigned_64 ticks;
    } timing_80186_recipe;
    static const timing_80186_recipe recipes[] = {
        { { 0xf6u,0xe0u },2u,27u }, { { 0xf7u,0xe0u },2u,36u },
        { { 0xf6u,0x26u,0u,0x10u },4u,33u }, { { 0xf7u,0x26u,0u,0x10u },4u,42u },
        { { 0xf6u,0xe8u },2u,27u }, { { 0xf7u,0xe8u },2u,36u },
        { { 0xf6u,0x2eu,0u,0x10u },4u,33u }, { { 0xf7u,0x2eu,0u,0x10u },4u,42u },
        { { 0xf6u,0xf8u },2u,48u }, { { 0xf7u,0xf8u },2u,57u },
        { { 0xf6u,0x3eu,0u,0x10u },4u,54u }, { { 0xf7u,0x3eu,0u,0x10u },4u,63u },
        { { 0x6bu,0xc0u,1u },3u,24u }, { { 0x69u,0xc0u,1u,0u },4u,31u }
    };
    STD_SIZE_T index;
    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        const type_unsigned_16 value = 1u;
        timing_80186_state state = { 0u,0u,0u };
        core_machine *machine = STD_NULL;
        C_INT failed = !timing_80186_prepare(&machine, &state) ||
            !timing_80186_load(machine, recipes[index].program,
                recipes[index].bytes) || core_machine_memory_write(machine,
                0x1000u, &value, sizeof(value)) != TYPE_STATUS_OK ||
            ((machine->executor_cpu.data.ax = 1u),
                (machine->executor_cpu.data.dx = 0u), 0) ||
            !timing_80186_run(machine, &state, 1u, recipes[index].ticks);
        core_machine_destroy(machine);
        if (failed) return 1;
    }
    return 0;
}

static C_INT timing_80186_div_matrix(C_VOID)
{
    typedef struct timing_80186_recipe {
        type_unsigned_8 program[4];
        STD_SIZE_T bytes;
        type_unsigned_64 ticks;
    } timing_80186_recipe;
    static const timing_80186_recipe recipes[] = {
        { { 0xf6u,0xf0u },2u,29u }, { { 0xf7u,0xf0u },2u,38u },
        { { 0xf6u,0x36u,0u,0x10u },4u,35u }, { { 0xf7u,0x36u,0u,0x10u },4u,44u }
    };
    STD_SIZE_T index;
    for (index = 0u; index < sizeof(recipes) / sizeof(recipes[0]); ++index) {
        const type_unsigned_16 value = 1u;
        timing_80186_state state = { 0u,0u,0u };
        core_machine *machine = STD_NULL;
        C_INT failed = !timing_80186_prepare(&machine, &state) ||
            !timing_80186_load(machine, recipes[index].program,
                recipes[index].bytes) || core_machine_memory_write(machine,
                0x1000u, &value, sizeof(value)) != TYPE_STATUS_OK ||
            ((machine->executor_cpu.data.ax = 1u),
                (machine->executor_cpu.data.dx = 0u), 0) ||
            !timing_80186_run(machine, &state, 1u, recipes[index].ticks);
        core_machine_destroy(machine);
        if (failed) return 1;
    }
    return 0;
}

static C_INT timing_80186_stack_frame(C_VOID)
{
    static const type_unsigned_8 enter[] = { 0xc8u, 0u, 0u, 0u };
    static const type_unsigned_8 leave[] = { 0xc9u };
    const type_unsigned_16 prior_bp = 0x1234u;
    timing_80186_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80186_prepare(&machine, &state);

    if (!failed) failed |= !timing_80186_load(machine, enter, sizeof(enter)) ||
        ((machine->executor_cpu.data.sp = 0x3000u),
            (machine->executor_cpu.data.bp = prior_bp), 0) ||
        !timing_80186_run(machine, &state, 1u, 15u) ||
        machine->executor_cpu.data.sp != 0x2ffeu ||
        machine->executor_cpu.data.bp != 0x2ffeu;
    if (!failed) failed |= !timing_80186_load(machine, leave, sizeof(leave)) ||
        ((machine->executor_cpu.data.bp = 0x2ffeu), 0) ||
        core_machine_memory_write(machine, 0x2ffeu, &prior_bp, sizeof(prior_bp)) !=
            TYPE_STATUS_OK || !timing_80186_run(machine, &state, 1u, 8u) ||
        machine->executor_cpu.data.sp != 0x3000u ||
        machine->executor_cpu.data.bp != prior_bp;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80186_memory(C_VOID)
{
    static const type_unsigned_8 read[] = { 0x8bu, 0x0eu, 0x00u, 0x10u };
    static const type_unsigned_8 write[] = { 0x89u, 0x0eu, 0x00u, 0x10u };
    static const type_unsigned_8 moffs_read[] = { 0xa1u, 0x01u, 0x10u };
    static const type_unsigned_8 moffs_write[] = { 0xa3u, 0x01u, 0x10u };
    static const type_unsigned_8 override[] = { 0x26u, 0x8bu, 0x0eu, 0x00u, 0x10u };
    static const type_unsigned_8 locked_add[] = {
        0xf0u, 0x01u, 0x0eu, 0x00u, 0x10u
    };
    static const type_unsigned_8 move_memory_immediate[] = {
        0xc6u, 0x06u, 0x00u, 0x10u, 0x5au
    };
    static const type_unsigned_8 segment_odd_shift[] = {
        0x26u, 0xd1u, 0x26u, 0x01u, 0x10u
    };
    const type_unsigned_16 value = 0x5aa5u;
    type_unsigned_8 byte_value = 0u;
    timing_80186_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80186_prepare(&machine, &state);

    if (!failed) failed |= !timing_80186_load(machine, read, sizeof(read)) ||
        core_machine_memory_write(machine, 0x1000u, &value, sizeof(value)) !=
            TYPE_STATUS_OK || !timing_80186_run(machine, &state, 1u, 9u) ||
        machine->executor_cpu.data.cx != value;
    if (!failed) failed |= !timing_80186_load(machine, write, sizeof(write)) ||
        ((machine->executor_cpu.data.cx = value), 0) ||
        !timing_80186_run(machine, &state, 1u, 12u);
    if (!failed) failed |= !timing_80186_load(machine, moffs_read,
        sizeof(moffs_read)) || core_machine_memory_write(machine, 0x1001u,
        &value, sizeof(value)) != TYPE_STATUS_OK ||
        !timing_80186_run(machine, &state, 1u, 13u);
    if (!failed) failed |= !timing_80186_load(machine, moffs_write,
        sizeof(moffs_write)) || ((machine->executor_cpu.data.ax = value), 0) ||
        !timing_80186_run(machine, &state, 1u, 12u);
    if (!failed) failed |= !timing_80186_load(machine, override,
        sizeof(override)) || core_machine_memory_write(machine, 0x1000u,
        &value, sizeof(value)) != TYPE_STATUS_OK ||
        !timing_80186_run(machine, &state, 1u, 11u);
    if (!failed) failed |= !timing_80186_load(machine, locked_add,
        sizeof(locked_add)) || core_machine_memory_write(machine, 0x1000u,
        &value, sizeof(value)) != TYPE_STATUS_OK ||
        !timing_80186_run(machine, &state, 1u, 12u);
    if (!failed) failed |= !timing_80186_load(machine, move_memory_immediate,
        sizeof(move_memory_immediate)) || !timing_80186_run(machine, &state,
        1u, 13u) || core_machine_memory_read(machine, 0x1000u, &byte_value,
        sizeof(byte_value)) != TYPE_STATUS_OK || byte_value != 0x5au;
    if (!failed) failed |= !timing_80186_load(machine, segment_odd_shift,
        sizeof(segment_odd_shift)) || core_machine_memory_write(machine, 0x1001u,
        &value, sizeof(value)) != TYPE_STATUS_OK ||
        !timing_80186_run(machine, &state, 1u, 25u);
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80186_control_ports(C_VOID)
{
    static const type_unsigned_8 taken[] = { 0x74u, 0x01u, 0x90u, 0x90u };
    static const type_unsigned_8 not_taken[] = { 0x75u, 0x01u, 0x90u, 0x90u };
    static const type_unsigned_8 rep[] = { 0xf3u, 0xa4u };
    static const type_unsigned_8 rep_segment[] = { 0x26u, 0xf3u, 0xa4u };
    static const type_unsigned_8 rep_odd_word[] = { 0xf3u, 0xa5u };
    static const type_unsigned_8 source[] = { 1u, 2u, 3u };
    static const type_unsigned_8 out_imm[] = { 0xe6u, 0xe0u };
    static const type_unsigned_8 out_dx[] = { 0xeeu };
    static const type_unsigned_8 in_imm[] = { 0xe4u, 0xe0u };
    static const type_unsigned_8 in_dx[] = { 0xecu };
    timing_80186_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80186_prepare(&machine, &state);

    if (!failed) failed |= !timing_80186_load(machine, taken, sizeof(taken)) ||
        ((machine->executor_cpu.data.eflags |= VCPU_EFLAGS_ZF), 0) ||
        !timing_80186_run(machine, &state, 1u, 13u);
    if (!failed) failed |= !timing_80186_load(machine, not_taken, sizeof(not_taken)) ||
        ((machine->executor_cpu.data.eflags |= VCPU_EFLAGS_ZF), 0) ||
        !timing_80186_run(machine, &state, 1u, 4u);
    if (!failed) failed |= !timing_80186_load(machine, rep, sizeof(rep)) ||
        core_machine_memory_write(machine, 0x1000u, source, sizeof(source)) !=
            TYPE_STATUS_OK || ((machine->executor_cpu.data.cx = 3u),
            (machine->executor_cpu.data.si = 0x1000u),
            (machine->executor_cpu.data.di = 0x1100u), 0) ||
        !timing_80186_run(machine, &state, 3u, 32u);
    if (!failed) failed |= !timing_80186_load(machine, rep_segment,
        sizeof(rep_segment)) || core_machine_memory_write(machine, 0x1000u,
        source, sizeof(source)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.cx = 3u),
        (machine->executor_cpu.data.si = 0x1000u),
        (machine->executor_cpu.data.di = 0x1100u), 0) ||
        !timing_80186_run(machine, &state, 3u, 38u);
    if (!failed) failed |= !timing_80186_load(machine, rep_odd_word,
        sizeof(rep_odd_word)) || core_machine_memory_write(machine, 0x1001u,
        source, sizeof(source)) != TYPE_STATUS_OK ||
        ((machine->executor_cpu.data.cx = 2u),
        (machine->executor_cpu.data.si = 0x1001u),
        (machine->executor_cpu.data.di = 0x1101u), 0) ||
        !timing_80186_run(machine, &state, 2u, 40u);
    if (!failed) failed |= !timing_80186_load(machine, out_imm, sizeof(out_imm)) ||
        !timing_80186_run(machine, &state, 1u, 9u) || state.writes != 1u;
    if (!failed) failed |= !timing_80186_load(machine, out_dx, sizeof(out_dx)) ||
        ((machine->executor_cpu.data.dx = 0x00e0u), 0) ||
        !timing_80186_run(machine, &state, 1u, 7u) || state.writes != 2u;
    if (!failed) failed |= !timing_80186_load(machine, in_imm, sizeof(in_imm)) ||
        !timing_80186_run(machine, &state, 1u, 10u) || state.reads != 1u;
    if (!failed) failed |= !timing_80186_load(machine, in_dx, sizeof(in_dx)) ||
        ((machine->executor_cpu.data.dx = 0x00e0u), 0) ||
        !timing_80186_run(machine, &state, 1u, 8u) || state.reads != 2u;
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_80186_boundaries(C_VOID)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    static const type_unsigned_8 rotate[] = { 0xd0u, 0xc0u };
    static const type_unsigned_8 fault[] = { 0x66u, 0x90u };
    static const type_unsigned_8 maximum[] = { 0x26u, 0x89u, 0x8bu, 0x00u, 0x10u };
    const core_machine_run_budget one = { 1u, 0u };
    const core_machine_run_budget insufficient = { 1u, 18u };
    core_machine_run_result result;
    timing_80186_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_80186_prepare(&machine, &state);

    if (!failed) failed |= !timing_80186_load(machine, rotate,
        sizeof(rotate)) || !timing_80186_run(machine, &state, 1u, 2u);
    if (!failed) failed |= !timing_80186_load(machine, fault, sizeof(fault)) ||
        !test_core_machine_fixture_preflight_real_ud_terminal(machine) ||
        core_machine_run(machine, one, &result) != TYPE_STATUS_FAULT ||
        result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
        result.ticks != 0u || state.advanced_ticks != 0u;
    if (!failed) failed |= !timing_80186_load(machine, nop, sizeof(nop)) ||
        !timing_80186_run(machine, &state, 1u, 3u) ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        machine->elapsed_ticks != 0u || state.advanced_ticks != 0u ||
        !timing_80186_load(machine, nop, sizeof(nop)) ||
        core_machine_request_stop(machine) != TYPE_STATUS_OK ||
        core_machine_run(machine, one, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_REQUESTED || result.executed != 0u ||
        result.ticks != 0u || state.advanced_ticks != 0u;
    if (!failed) failed |= !timing_80186_load(machine, maximum, sizeof(maximum)) ||
        ((machine->executor_cpu.data.bp = 1u),
            (machine->executor_cpu.data.di = 0u), 0) ||
        core_machine_run(machine, insufficient, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
        result.ticks != 0u || !timing_80186_run(machine, &state, 1u, 18u);
    if (!failed) failed |= !timing_80186_load(machine, nop, sizeof(nop));
    if (!failed) {
        machine->elapsed_ticks = UINT64_MAX - 2u;
        state.advanced_ticks = 0u;
        failed |= core_machine_run(machine, one, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            result.ticks != 0u || machine->elapsed_ticks != UINT64_MAX - 2u ||
            state.advanced_ticks != 0u;
    }
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    static const type_unsigned_8 clc[] = { 0xf8u };
    static const type_unsigned_8 immediate[] = { 0xb8u, 0x34u, 0x12u };
    static const type_unsigned_8 registers[] = { 0x8bu, 0xc1u };
    static const type_unsigned_8 near_call[] = { 0xe8u, 0u, 0u };
    static const type_unsigned_8 direct_jump[] = { 0xebu, 0u };
    static const type_unsigned_8 mul8[] = { 0xf6u, 0xe0u };
    static const type_unsigned_8 mul16[] = { 0xf7u, 0xe0u };
    static const type_unsigned_8 imul8[] = { 0xf6u, 0xe8u };
    static const type_unsigned_8 imul16[] = { 0xf7u, 0xe8u };
    static const type_unsigned_8 imul_imm8[] = { 0x6bu, 0xc0u, 0x01u };
    static const type_unsigned_8 imul_imm16[] = {
        0x69u, 0xc0u, 0x01u, 0x00u
    };
    static const type_unsigned_8 rotate_imm[] = { 0xc0u, 0xc0u, 0x03u };
    static const type_unsigned_8 xlat[] = { 0xd7u };
    static const type_unsigned_8 sreg_to_register[] = { 0x8cu, 0xc0u };
    static const type_unsigned_8 sreg_from_register[] = { 0x8eu, 0xc0u };
    static const type_unsigned_8 cmc[] = { 0xf5u };
    static const type_unsigned_8 sahf[] = { 0x9eu };
    static const type_unsigned_8 lahf[] = { 0x9fu };

    if (timing_80186_case(nop, sizeof(nop), 3u) ||
        timing_80186_case(clc, sizeof(clc), 2u) ||
        timing_80186_case(immediate, sizeof(immediate), 4u) ||
        timing_80186_case(registers, sizeof(registers), 2u) ||
        timing_80186_lea() ||
        timing_80186_pointer_loads() ||
        timing_80186_bound() ||
        timing_80186_alu_matrix() ||
        timing_80186_cmp_test_matrix() ||
        timing_80186_adjustment_matrix() ||
        timing_80186_unary_matrix() ||
        timing_80186_flag_controls() ||
        timing_80186_group2_matrix() ||
        timing_80186_l2_arithmetic_matrix() ||
        timing_80186_div_matrix() ||
        timing_80186_stack_frame() ||
        timing_80186_case(near_call, sizeof(near_call), 15u) ||
        timing_80186_case(direct_jump, sizeof(direct_jump), 13u) ||
        timing_80186_case(mul8, sizeof(mul8), 27u) ||
        timing_80186_case(mul16, sizeof(mul16), 36u) ||
        timing_80186_case(imul8, sizeof(imul8), 27u) ||
        timing_80186_case(imul16, sizeof(imul16), 36u) ||
        timing_80186_case(imul_imm8, sizeof(imul_imm8), 24u) ||
        timing_80186_case(imul_imm16, sizeof(imul_imm16), 31u) ||
        timing_80186_case(rotate_imm, sizeof(rotate_imm), 8u) ||
        timing_80186_case(xlat, sizeof(xlat), 11u) ||
        timing_80186_case(sreg_to_register, sizeof(sreg_to_register), 2u) ||
        timing_80186_case(sreg_from_register, sizeof(sreg_from_register), 2u) ||
        timing_80186_case(cmc, sizeof(cmc), 2u) ||
        timing_80186_case(sahf, sizeof(sahf), 3u) ||
        timing_80186_case(lahf, sizeof(lahf), 2u)) return 1;
    if (timing_80186_memory()) return 2;
    if (timing_80186_control_ports()) return 3;
    if (timing_80186_boundaries()) return 4;
    STD_PRINTF("M5:T357:S5:80186-INSTRUCTION-TIMING-LEDGER:OK\n");
    return 0;
}
