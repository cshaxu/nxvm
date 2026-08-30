#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define TIMING_8086_RESET_LINEAR 0x000ffff0u
#define TIMING_8086_RESET_PHYSICAL 0x000ffff0u
#define TIMING_8086_WINDOW_BYTES 16u

typedef struct timing_8086_state {
    type_unsigned_32 reads;
    type_unsigned_32 writes;
    type_unsigned_64 advanced_ticks;
} timing_8086_state;

static type_status timing_8086_port_read(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 *out_value)
{
    timing_8086_state *state = (timing_8086_state *)owner;

    if (state == STD_NULL || out_value == STD_NULL || port != 0x00e0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    ++state->reads;
    *out_value = 0x5au;
    return TYPE_STATUS_OK;
}

static type_status timing_8086_port_write(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 value)
{
    timing_8086_state *state = (timing_8086_state *)owner;

    if (state == STD_NULL || port != 0x00e0u || value > 0xffffu) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    ++state->writes;
    return TYPE_STATUS_OK;
}

static const core_machine_port_provider timing_8086_port_provider = {
    timing_8086_port_read,
    timing_8086_port_write
};

static C_VOID timing_8086_execution_reset(C_VOID *opaque)
{
    timing_8086_state *state = (timing_8086_state *)opaque;

    if (state != STD_NULL) state->advanced_ticks = 0u;
}

static C_VOID timing_8086_execution_advance(C_VOID *opaque,
    type_unsigned_64 elapsed_ticks)
{
    timing_8086_state *state = (timing_8086_state *)opaque;

    if (state != STD_NULL) state->advanced_ticks += elapsed_ticks;
}

static const core_machine_execution_provider timing_8086_execution_provider = {
    timing_8086_execution_reset,
    timing_8086_execution_advance
};

static C_INT timing_8086_prepare(core_machine **out_machine,
    timing_8086_state *state)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .ticks_per_instruction = 29u,
        .instruction_timing = { 29u, 7u, 31u, 37u, 41u, 43u }
    };
    core_machine *machine = STD_NULL;

    if (out_machine == STD_NULL || state == STD_NULL ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_8086_RESET_LINEAR, TIMING_8086_RESET_PHYSICAL,
            TIMING_8086_WINDOW_BYTES) != TYPE_STATUS_OK ||
        core_machine_install_port_provider(machine, 0x00e0u, 0x00e0u,
            &timing_8086_port_provider, state) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(machine,
            &timing_8086_execution_provider, state)) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static C_INT timing_8086_load(core_machine *machine,
    const type_unsigned_8 *program, STD_SIZE_T program_bytes)
{
    return machine != STD_NULL && program != STD_NULL &&
        core_machine_reset(machine) == TYPE_STATUS_OK &&
        core_machine_set_a20(machine, 1) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, TIMING_8086_RESET_LINEAR, program,
            program_bytes) == TYPE_STATUS_OK;
}

static C_INT timing_8086_execute(core_machine *machine,
    type_unsigned_64 instructions, type_unsigned_64 expected_ticks,
    timing_8086_state *state)
{
    const core_machine_run_budget budget = { instructions, 0u };
    core_machine_run_result result = { 0 };
    C_INT succeeded = machine != STD_NULL && state != STD_NULL &&
        core_machine_run(machine, budget, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET &&
        result.executed == instructions && result.ticks == expected_ticks &&
        result.elapsed_ticks == expected_ticks && state->advanced_ticks == expected_ticks;

    if (!succeeded) {
        STD_PRINTF("8086 ledger expected=%llu actual=%llu executed=%llu reason=%d advanced=%llu\n",
            expected_ticks, result.ticks, result.executed, result.reason,
            state == STD_NULL ? 0u : state->advanced_ticks);
    }
    return succeeded;
}

static C_INT timing_8086_case(const type_unsigned_8 *program,
    STD_SIZE_T program_bytes, type_unsigned_64 instructions,
    type_unsigned_64 expected_ticks)
{
    timing_8086_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT prepared = timing_8086_prepare(&machine, &state);
    C_INT loaded = prepared && timing_8086_load(machine, program, program_bytes);
    C_INT executed = loaded && timing_8086_execute(machine, instructions,
        expected_ticks, &state);
    C_INT failed = !prepared || !loaded || !executed;

    if (failed) {
        STD_PRINTF("8086 ledger case opcode=%u prepared=%d loaded=%d executed=%d\n",
            program == STD_NULL ? 0u : program[0], prepared, loaded, executed);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_8086_test_baseline(C_VOID)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    static const type_unsigned_8 clc[] = { 0xf8u };
    static const type_unsigned_8 aaa[] = { 0x37u };
    static const type_unsigned_8 cmc[] = { 0xf5u };
    static const type_unsigned_8 cld[] = { 0xfcu };
    static const type_unsigned_8 cli[] = { 0xfau };
    static const type_unsigned_8 stc[] = { 0xf9u };
    static const type_unsigned_8 std[] = { 0xfdu };
    static const type_unsigned_8 sti[] = { 0xfbu };
    static const type_unsigned_8 lahf[] = { 0x9fu };
    static const type_unsigned_8 sahf[] = { 0x9eu };
    static const type_unsigned_8 mov_ax_es[] = { 0x8cu, 0xc0u };
    static const type_unsigned_8 xchg_register[] = { 0x87u, 0xc1u };
    static const type_unsigned_8 mov_immediate[] = { 0xb8u, 0x34u, 0x12u };
    static const type_unsigned_8 mov_register[] = { 0x8bu, 0xc1u };

    return timing_8086_case(nop, sizeof(nop), 1u, 3u) ||
        timing_8086_case(clc, sizeof(clc), 1u, 2u) ||
        timing_8086_case(aaa, sizeof(aaa), 1u, 4u) ||
        timing_8086_case(cmc, sizeof(cmc), 1u, 2u) ||
        timing_8086_case(cld, sizeof(cld), 1u, 2u) ||
        timing_8086_case(cli, sizeof(cli), 1u, 2u) ||
        timing_8086_case(stc, sizeof(stc), 1u, 2u) ||
        timing_8086_case(std, sizeof(std), 1u, 2u) ||
        timing_8086_case(sti, sizeof(sti), 1u, 2u) ||
        timing_8086_case(lahf, sizeof(lahf), 1u, 4u) ||
        timing_8086_case(sahf, sizeof(sahf), 1u, 4u) ||
        timing_8086_case(mov_ax_es, sizeof(mov_ax_es), 1u, 2u) ||
        timing_8086_case(xchg_register, sizeof(xchg_register), 1u, 4u) ||
        timing_8086_case(mov_immediate, sizeof(mov_immediate), 1u, 4u) ||
        timing_8086_case(mov_register, sizeof(mov_register), 1u, 2u);
}

static C_INT timing_8086_test_memory(C_VOID)
{
    static const type_unsigned_8 read_direct[] = { 0x8bu, 0x0eu, 0x00u, 0x10u };
    static const type_unsigned_8 write_direct[] = { 0x89u, 0x0eu, 0x00u, 0x10u };
    static const type_unsigned_8 read_odd[] = { 0x8bu, 0x0eu, 0x01u, 0x10u };
    static const type_unsigned_8 read_override[] = { 0x26u, 0x8bu, 0x0eu, 0x00u, 0x10u };
    static const type_unsigned_8 read_indexed[] = { 0x8bu, 0x8bu, 0x00u, 0x10u };
    static const type_unsigned_8 lock_add_memory[] = {
        0xf0u, 0x01u, 0x06u, 0x00u, 0x10u
    };
    static const type_unsigned_8 moffs_read_odd[] = { 0xa1u, 0x01u, 0x10u };
    static const type_unsigned_8 moffs_write_odd[] = { 0xa3u, 0x01u, 0x10u };
    static const type_unsigned_8 esc_memory_odd[] = {
        0xd8u, 0x06u, 0x01u, 0x10u
    };
    const type_unsigned_16 value = 0x5aa5u;
    timing_8086_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_8086_prepare(&machine, &state);

    if (!failed) {
        failed |= !timing_8086_load(machine, read_direct, sizeof(read_direct)) ||
            core_machine_memory_write(machine, 0x1000u, &value, sizeof(value)) !=
                TYPE_STATUS_OK || !timing_8086_execute(machine, 1u, 14u, &state) ||
            machine->executor_cpu.data.cx != value;
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, write_direct, sizeof(write_direct)) ||
            ((machine->executor_cpu.data.cx = value), 0) ||
            !timing_8086_execute(machine, 1u, 15u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, read_odd, sizeof(read_odd)) ||
            core_machine_memory_write(machine, 0x1001u, &value, sizeof(value)) !=
                TYPE_STATUS_OK || !timing_8086_execute(machine, 1u, 18u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, read_override,
            sizeof(read_override)) || core_machine_memory_write(machine, 0x1000u,
                &value, sizeof(value)) != TYPE_STATUS_OK ||
            !timing_8086_execute(machine, 1u, 16u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, read_indexed, sizeof(read_indexed)) ||
            ((machine->executor_cpu.data.bp = 0u),
                (machine->executor_cpu.data.di = 0u), 0) ||
            core_machine_memory_write(machine, 0x1000u, &value, sizeof(value)) !=
                TYPE_STATUS_OK || !timing_8086_execute(machine, 1u, 20u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, moffs_read_odd,
            sizeof(moffs_read_odd)) || core_machine_memory_write(machine, 0x1001u,
                &value, sizeof(value)) != TYPE_STATUS_OK ||
            !timing_8086_execute(machine, 1u, 14u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, moffs_write_odd,
            sizeof(moffs_write_odd)) || ((machine->executor_cpu.data.ax = value), 0) ||
            !timing_8086_execute(machine, 1u, 14u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, lock_add_memory,
            sizeof(lock_add_memory)) ||
            core_machine_memory_write(machine, 0x1000u, &value, sizeof(value)) !=
                TYPE_STATUS_OK || ((machine->executor_cpu.data.ax = 1u), 0) ||
            !timing_8086_execute(machine, 1u, 24u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, esc_memory_odd,
            sizeof(esc_memory_odd)) || !timing_8086_execute(machine, 1u, 18u,
                &state);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_8086_test_alu_and_cmp_forms(C_VOID)
{
    static const type_unsigned_8 alu_bases[] = {
        0x00u, 0x08u, 0x10u, 0x18u, 0x20u, 0x28u, 0x30u
    };
    type_unsigned_8 index;

    for (index = 0u; index < sizeof(alu_bases); ++index) {
        type_unsigned_8 group = index;
        type_unsigned_8 base = alu_bases[index];
        type_unsigned_8 register_register[] = { (type_unsigned_8)(base + 3u), 0xc1u };
        type_unsigned_8 register_memory[] = {
            (type_unsigned_8)(base + 3u), 0x06u, 0x00u, 0x10u
        };
        type_unsigned_8 memory_register[] = {
            (type_unsigned_8)(base + 1u), 0x06u, 0x00u, 0x10u
        };
        type_unsigned_8 register_immediate[] = {
            0x81u, (type_unsigned_8)(0xc0u | (group << 3u)), 0x01u, 0x00u
        };
        type_unsigned_8 memory_immediate[] = {
            0x81u, (type_unsigned_8)(0x06u | (group << 3u)), 0x00u, 0x10u,
            0x01u, 0x00u
        };
        type_unsigned_8 accumulator_immediate[] = {
            (type_unsigned_8)(base + 5u), 0x01u, 0x00u
        };

        if (timing_8086_case(register_register, sizeof(register_register), 1u, 3u) ||
            timing_8086_case(register_memory, sizeof(register_memory), 1u, 15u) ||
            timing_8086_case(memory_register, sizeof(memory_register), 1u, 22u) ||
            timing_8086_case(register_immediate, sizeof(register_immediate), 1u, 4u) ||
            timing_8086_case(memory_immediate, sizeof(memory_immediate), 1u, 23u) ||
            timing_8086_case(accumulator_immediate, sizeof(accumulator_immediate),
                1u, 4u)) return 1;
    }
    return timing_8086_case((const type_unsigned_8[]){ 0x3bu, 0xc1u }, 2u,
        1u, 3u) || timing_8086_case((const type_unsigned_8[]){
            0x3bu, 0x06u, 0x00u, 0x10u }, 4u, 1u, 15u) ||
        timing_8086_case((const type_unsigned_8[]){
            0x39u, 0x06u, 0x00u, 0x10u }, 4u, 1u, 15u) ||
        timing_8086_case((const type_unsigned_8[]){ 0x81u, 0xf8u, 0x01u, 0x00u },
            4u, 1u, 4u) || timing_8086_case((const type_unsigned_8[]){
            0x81u, 0x3eu, 0x00u, 0x10u, 0x01u, 0x00u }, 6u, 1u, 16u) ||
        timing_8086_case((const type_unsigned_8[]){ 0x3du, 0x01u, 0x00u },
            3u, 1u, 4u);
}

static C_INT timing_8086_test_primary_remaining_forms(C_VOID)
{
    static const type_unsigned_8 test_register[] = { 0x85u, 0xc1u };
    static const type_unsigned_8 test_memory[] = { 0x85u, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_8 test_register_immediate[] = {
        0xf7u, 0xc0u, 0x01u, 0x00u
    };
    static const type_unsigned_8 test_memory_immediate[] = {
        0xf7u, 0x06u, 0x00u, 0x10u, 0x01u, 0x00u
    };
    static const type_unsigned_8 test_accumulator[] = { 0xa9u, 0x01u, 0x00u };
    static const type_unsigned_8 inc_register[] = { 0x40u };
    static const type_unsigned_8 dec_register[] = { 0x48u };
    static const type_unsigned_8 inc_byte_register[] = { 0xfeu, 0xc0u };
    static const type_unsigned_8 dec_byte_register[] = { 0xfeu, 0xc8u };
    static const type_unsigned_8 inc_memory[] = { 0xffu, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_8 dec_memory[] = { 0xffu, 0x0eu, 0x00u, 0x10u };
    static const type_unsigned_8 not_register[] = { 0xf7u, 0xd0u };
    static const type_unsigned_8 neg_register[] = { 0xf7u, 0xd8u };
    static const type_unsigned_8 not_memory[] = { 0xf7u, 0x16u, 0x00u, 0x10u };
    static const type_unsigned_8 neg_memory[] = { 0xf7u, 0x1eu, 0x00u, 0x10u };
    static const type_unsigned_8 xchg_ax_register[] = { 0x91u };
    static const type_unsigned_8 xchg_memory[] = { 0x87u, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_8 mov_memory_immediate[] = {
        0xc7u, 0x06u, 0x00u, 0x10u, 0x01u, 0x00u
    };

    return timing_8086_case(test_register, sizeof(test_register), 1u, 3u) ||
        timing_8086_case(test_memory, sizeof(test_memory), 1u, 15u) ||
        timing_8086_case(test_register_immediate, sizeof(test_register_immediate),
            1u, 5u) || timing_8086_case(test_memory_immediate,
                sizeof(test_memory_immediate), 1u, 17u) ||
        timing_8086_case(test_accumulator, sizeof(test_accumulator), 1u, 4u) ||
        timing_8086_case(inc_register, sizeof(inc_register), 1u, 2u) ||
        timing_8086_case(dec_register, sizeof(dec_register), 1u, 2u) ||
        timing_8086_case(inc_byte_register, sizeof(inc_byte_register), 1u, 3u) ||
        timing_8086_case(dec_byte_register, sizeof(dec_byte_register), 1u, 3u) ||
        timing_8086_case(inc_memory, sizeof(inc_memory), 1u, 21u) ||
        timing_8086_case(dec_memory, sizeof(dec_memory), 1u, 21u) ||
        timing_8086_case(not_register, sizeof(not_register), 1u, 3u) ||
        timing_8086_case(neg_register, sizeof(neg_register), 1u, 3u) ||
        timing_8086_case(not_memory, sizeof(not_memory), 1u, 22u) ||
        timing_8086_case(neg_memory, sizeof(neg_memory), 1u, 22u) ||
        timing_8086_case(xchg_ax_register, sizeof(xchg_ax_register), 1u, 3u) ||
        timing_8086_case(xchg_memory, sizeof(xchg_memory), 1u, 23u) ||
        timing_8086_case(mov_memory_immediate, sizeof(mov_memory_immediate),
            1u, 16u);
}

static C_INT timing_8086_test_segment_and_pointer_transfers(C_VOID)
{
    static const type_unsigned_8 sreg_to_register[] = { 0x8cu, 0xc0u };
    static const type_unsigned_8 sreg_to_memory[] = { 0x8cu, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_8 sreg_from_register[] = { 0x8eu, 0xc0u };
    static const type_unsigned_8 sreg_from_memory[] = { 0x8eu, 0x06u, 0x00u, 0x10u };
    static const type_unsigned_8 lds_memory[] = { 0xc5u, 0x1eu, 0x00u, 0x10u };
    static const type_unsigned_8 les_memory[] = { 0xc4u, 0x1eu, 0x00u, 0x10u };
    const type_unsigned_16 segment = 0x0800u;
    const type_unsigned_16 pointer[2] = { 0x2000u, 0x0800u };
    timing_8086_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_8086_prepare(&machine, &state);

    if (!failed) {
        failed |= !timing_8086_load(machine, sreg_to_register,
            sizeof(sreg_to_register)) || !timing_8086_execute(machine, 1u, 2u,
                &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, sreg_to_memory,
            sizeof(sreg_to_memory)) || !timing_8086_execute(machine, 1u, 15u,
                &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, sreg_from_register,
            sizeof(sreg_from_register)) || ((machine->executor_cpu.data.ax = segment),
                0) || !timing_8086_execute(machine, 1u, 2u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, sreg_from_memory,
            sizeof(sreg_from_memory)) || core_machine_memory_write(machine, 0x1000u,
                &segment, sizeof(segment)) != TYPE_STATUS_OK ||
            !timing_8086_execute(machine, 1u, 14u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, lds_memory, sizeof(lds_memory)) ||
            core_machine_memory_write(machine, 0x1000u, pointer, sizeof(pointer)) !=
                TYPE_STATUS_OK || !timing_8086_execute(machine, 1u, 22u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, les_memory, sizeof(les_memory)) ||
            core_machine_memory_write(machine, 0x1000u, pointer, sizeof(pointer)) !=
                TYPE_STATUS_OK || !timing_8086_execute(machine, 1u, 22u, &state);
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_8086_test_wait_ticks(C_VOID)
{
    static const type_unsigned_8 wait[] = { 0x9bu };
    timing_8086_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_8086_prepare(&machine, &state);

    if (!failed) {
        failed |= !timing_8086_load(machine, wait, sizeof(wait)) ||
            ((machine->fpu.busy = TYPE_TRUE), 0) ||
            ((machine->fpu.completion_remaining_ticks = 3u), 0) ||
            !timing_8086_execute(machine, 1u, 6u, &state) ||
            machine->fpu.completion_remaining_ticks != 0u ||
            machine->fpu.last_wait_ticks != 3u;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_8086_test_group3_operand_model(C_VOID)
{
    static const type_unsigned_8 mul_r8[] = { 0xf6u, 0xe3u };
    static const type_unsigned_8 mul_r16[] = { 0xf7u, 0xe3u };
    static const type_unsigned_8 imul_r8[] = { 0xf6u, 0xebu };
    static const type_unsigned_8 imul_r16[] = { 0xf7u, 0xebu };
    static const type_unsigned_8 div_r8[] = { 0xf6u, 0xf3u };
    static const type_unsigned_8 div_r16[] = { 0xf7u, 0xf3u };
    static const type_unsigned_8 idiv_r8[] = { 0xf6u, 0xfbu };
    static const type_unsigned_8 idiv_r16[] = { 0xf7u, 0xfbu };
    static const type_unsigned_8 mul_m8[] = { 0xf6u, 0x26u, 0x00u, 0x10u };
    static const type_unsigned_8 mul_m16[] = { 0xf7u, 0x26u, 0x00u, 0x10u };
    static const type_unsigned_8 imul_m8[] = { 0xf6u, 0x2eu, 0x00u, 0x10u };
    static const type_unsigned_8 imul_m16[] = { 0xf7u, 0x2eu, 0x00u, 0x10u };
    static const type_unsigned_8 div_m8[] = { 0xf6u, 0x36u, 0x00u, 0x10u };
    static const type_unsigned_8 div_m16[] = { 0xf7u, 0x36u, 0x00u, 0x10u };
    static const type_unsigned_8 idiv_m8[] = { 0xf6u, 0x3eu, 0x00u, 0x10u };
    static const type_unsigned_8 idiv_m16[] = { 0xf7u, 0x3eu, 0x00u, 0x10u };
    const type_unsigned_8 operand8 = 3u;
    const type_unsigned_16 operand16 = 3u;
    timing_8086_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_8086_prepare(&machine, &state);

#define TIMING_8086_GROUP3_REGISTER_CASE(bytes, expected, setup) \
    do { \
        if (!failed) { \
            failed |= !timing_8086_load(machine, bytes, sizeof(bytes)) || \
                ((setup), 0) || !timing_8086_execute(machine, 1u, expected, &state); \
        } \
    } while (0)
#define TIMING_8086_GROUP3_MEMORY_CASE(bytes, expected, setup, value, size) \
    do { \
        if (!failed) { \
            failed |= !timing_8086_load(machine, bytes, sizeof(bytes)) || \
                core_machine_memory_write(machine, 0x1000u, value, size) != \
                    TYPE_STATUS_OK || ((setup), 0) || \
                !timing_8086_execute(machine, 1u, expected, &state); \
        } \
    } while (0)

    TIMING_8086_GROUP3_REGISTER_CASE(mul_r8, 71u,
        (machine->executor_cpu.data.ax = 2u, machine->executor_cpu.data.bx = 3u));
    TIMING_8086_GROUP3_REGISTER_CASE(mul_r8, 70u,
        (machine->executor_cpu.data.ax = 0u, machine->executor_cpu.data.bx = 3u));
    TIMING_8086_GROUP3_REGISTER_CASE(mul_r8, 77u,
        (machine->executor_cpu.data.ax = 0xffu, machine->executor_cpu.data.bx = 3u));
    TIMING_8086_GROUP3_REGISTER_CASE(mul_r16, 119u,
        (machine->executor_cpu.data.ax = 2u, machine->executor_cpu.data.bx = 3u));
    TIMING_8086_GROUP3_REGISTER_CASE(imul_r8, 91u,
        (machine->executor_cpu.data.ax = 2u, machine->executor_cpu.data.bx = 3u));
    TIMING_8086_GROUP3_REGISTER_CASE(imul_r8, 98u,
        (machine->executor_cpu.data.ax = 0x80u, machine->executor_cpu.data.bx = 1u));
    TIMING_8086_GROUP3_REGISTER_CASE(imul_r16, 139u,
        (machine->executor_cpu.data.ax = 2u, machine->executor_cpu.data.bx = 3u));
    TIMING_8086_GROUP3_REGISTER_CASE(div_r8, 81u,
        (machine->executor_cpu.data.ax = 6u, machine->executor_cpu.data.bx = 3u));
    TIMING_8086_GROUP3_REGISTER_CASE(div_r8, 80u,
        (machine->executor_cpu.data.ax = 0u, machine->executor_cpu.data.bx = 1u));
    TIMING_8086_GROUP3_REGISTER_CASE(div_r8, 88u,
        (machine->executor_cpu.data.ax = 0xffu, machine->executor_cpu.data.bx = 1u));
    TIMING_8086_GROUP3_REGISTER_CASE(div_r16, 145u,
        (machine->executor_cpu.data.dx = 0u, machine->executor_cpu.data.ax = 6u,
            machine->executor_cpu.data.bx = 3u));
    TIMING_8086_GROUP3_REGISTER_CASE(idiv_r8, 111u,
        (machine->executor_cpu.data.ax = 6u, machine->executor_cpu.data.bx = 3u));
    TIMING_8086_GROUP3_REGISTER_CASE(idiv_r16, 175u,
        (machine->executor_cpu.data.dx = 0u, machine->executor_cpu.data.ax = 6u,
            machine->executor_cpu.data.bx = 3u));
    TIMING_8086_GROUP3_MEMORY_CASE(mul_m8, 83u,
        (machine->executor_cpu.data.ax = 2u), &operand8, sizeof(operand8));
    TIMING_8086_GROUP3_MEMORY_CASE(mul_m16, 131u,
        (machine->executor_cpu.data.ax = 2u), &operand16, sizeof(operand16));
    TIMING_8086_GROUP3_MEMORY_CASE(imul_m8, 103u,
        (machine->executor_cpu.data.ax = 2u), &operand8, sizeof(operand8));
    TIMING_8086_GROUP3_MEMORY_CASE(imul_m16, 151u,
        (machine->executor_cpu.data.ax = 2u), &operand16, sizeof(operand16));
    TIMING_8086_GROUP3_MEMORY_CASE(div_m8, 93u,
        (machine->executor_cpu.data.ax = 6u), &operand8, sizeof(operand8));
    TIMING_8086_GROUP3_MEMORY_CASE(div_m16, 157u,
        (machine->executor_cpu.data.dx = 0u, machine->executor_cpu.data.ax = 6u),
        &operand16, sizeof(operand16));
    TIMING_8086_GROUP3_MEMORY_CASE(idiv_m8, 123u,
        (machine->executor_cpu.data.ax = 6u), &operand8, sizeof(operand8));
    TIMING_8086_GROUP3_MEMORY_CASE(idiv_m16, 187u,
        (machine->executor_cpu.data.dx = 0u, machine->executor_cpu.data.ax = 6u),
        &operand16, sizeof(operand16));

#undef TIMING_8086_GROUP3_MEMORY_CASE
#undef TIMING_8086_GROUP3_REGISTER_CASE
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_8086_test_group2_forms(C_VOID)
{
    static const type_unsigned_8 extensions[] = {
        0u, 1u, 2u, 3u, 4u, 5u, 7u
    };
    type_unsigned_8 index;
    const type_unsigned_16 value = 0x5aa5u;

    for (index = 0u; index < sizeof(extensions); ++index) {
        type_unsigned_8 extension = extensions[index];
        type_unsigned_8 register_one[] = {
            0xd0u, (type_unsigned_8)(0xc0u | (extension << 3u))
        };
        type_unsigned_8 register_cl[] = {
            0xd2u, (type_unsigned_8)(0xc0u | (extension << 3u))
        };
        type_unsigned_8 memory_one[] = {
            0xd1u, (type_unsigned_8)(0x06u | (extension << 3u)), 0x00u, 0x10u
        };
        type_unsigned_8 memory_cl[] = {
            0xd3u, (type_unsigned_8)(0x06u | (extension << 3u)), 0x00u, 0x10u
        };
        timing_8086_state state = { 0u, 0u, 0u };
        core_machine *machine = STD_NULL;
        C_INT failed = !timing_8086_prepare(&machine, &state);

        if (!failed) {
            failed |= !timing_8086_load(machine, register_one,
                sizeof(register_one)) || !timing_8086_execute(machine, 1u, 2u,
                    &state);
        }
        if (!failed) {
            failed |= !timing_8086_load(machine, register_cl,
                sizeof(register_cl)) || ((machine->executor_cpu.data.cx = 2u), 0) ||
                !timing_8086_execute(machine, 1u, 16u, &state);
        }
        if (!failed) {
            failed |= !timing_8086_load(machine, memory_one, sizeof(memory_one)) ||
                core_machine_memory_write(machine, 0x1000u, &value, sizeof(value)) !=
                    TYPE_STATUS_OK || !timing_8086_execute(machine, 1u, 21u, &state);
        }
        if (!failed) {
            failed |= !timing_8086_load(machine, memory_cl, sizeof(memory_cl)) ||
                core_machine_memory_write(machine, 0x1000u, &value, sizeof(value)) !=
                    TYPE_STATUS_OK || ((machine->executor_cpu.data.cx = 2u), 0) ||
                !timing_8086_execute(machine, 1u, 34u, &state);
        }
        core_machine_destroy(machine);
        if (failed) return 1;
    }
    return timing_8086_case((const type_unsigned_8[]){
        0x26u, 0xd1u, 0x06u, 0x01u, 0x10u }, 5u, 1u, 31u);
}

static C_INT timing_8086_test_control_repeat_and_ports(C_VOID)
{
    static const type_unsigned_8 taken[] = { 0x74u, 0x01u, 0x90u, 0x90u };
    static const type_unsigned_8 not_taken[] = { 0x75u, 0x01u, 0x90u, 0x90u };
    static const type_unsigned_8 movsb[] = { 0xa4u };
    static const type_unsigned_8 segment_movsb[] = { 0x26u, 0xa4u };
    static const type_unsigned_8 movsw[] = { 0xa5u };
    static const type_unsigned_8 rep_movsb[] = { 0xf3u, 0xa4u };
    static const type_unsigned_8 segment_rep_movsb[] = { 0x26u, 0xf3u, 0xa4u };
    static const type_unsigned_8 source[] = { 0x11u, 0x22u, 0x33u };
    static const type_unsigned_8 in_immediate[] = { 0xe4u, 0xe0u };
    static const type_unsigned_8 in_dx[] = { 0xecu };
    static const type_unsigned_8 out_immediate[] = { 0xe6u, 0xe0u };
    static const type_unsigned_8 out_dx[] = { 0xeeu };
    timing_8086_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_8086_prepare(&machine, &state);

    if (!failed) {
        failed |= !timing_8086_load(machine, taken, sizeof(taken)) ||
            ((machine->executor_cpu.data.eflags |= VCPU_EFLAGS_ZF), 0) ||
            !timing_8086_execute(machine, 1u, 16u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, not_taken, sizeof(not_taken)) ||
            ((machine->executor_cpu.data.eflags |= VCPU_EFLAGS_ZF), 0) ||
            !timing_8086_execute(machine, 1u, 4u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, movsb, sizeof(movsb)) ||
            !timing_8086_execute(machine, 1u, 18u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, segment_movsb,
            sizeof(segment_movsb)) || !timing_8086_execute(machine, 1u, 20u,
                &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, movsw, sizeof(movsw)) ||
            ((machine->executor_cpu.data.si = 0x1001u),
                (machine->executor_cpu.data.di = 0x1101u), 0) ||
            !timing_8086_execute(machine, 1u, 26u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, rep_movsb, sizeof(rep_movsb)) ||
            core_machine_memory_write(machine, 0x1000u, source, sizeof(source)) !=
                TYPE_STATUS_OK || ((machine->executor_cpu.data.cx = 3u),
                (machine->executor_cpu.data.si = 0x1000u),
                (machine->executor_cpu.data.di = 0x1100u), 0) ||
            !timing_8086_execute(machine, 3u, 60u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, segment_rep_movsb,
            sizeof(segment_rep_movsb)) || core_machine_memory_write(machine,
                0x1000u, source, sizeof(source)) != TYPE_STATUS_OK ||
            ((machine->executor_cpu.data.cx = 3u),
                (machine->executor_cpu.data.si = 0x1000u),
                (machine->executor_cpu.data.di = 0x1100u), 0) ||
            !timing_8086_execute(machine, 3u, 66u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, in_immediate, sizeof(in_immediate)) ||
            !timing_8086_execute(machine, 1u, 10u, &state) || state.reads != 1u;
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, in_dx, sizeof(in_dx)) ||
            ((machine->executor_cpu.data.dx = 0x00e0u), 0) ||
            !timing_8086_execute(machine, 1u, 8u, &state) || state.reads != 2u;
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, out_immediate,
            sizeof(out_immediate)) || !timing_8086_execute(machine, 1u, 10u,
                &state) || state.writes != 1u;
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, out_dx, sizeof(out_dx)) ||
            ((machine->executor_cpu.data.dx = 0x00e0u), 0) ||
            !timing_8086_execute(machine, 1u, 8u, &state) || state.writes != 2u;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT timing_8086_test_fallback_fault_budget_and_overflow(C_VOID)
{
    static const type_unsigned_8 group2[] = { 0xd0u, 0xc0u };
    static const type_unsigned_8 fault[] = { 0x66u, 0x90u };
    static const type_unsigned_8 maximum[] = { 0x26u, 0x89u, 0x8bu, 0x00u, 0x10u };
    static const type_unsigned_8 nop[] = { 0x90u };
    const core_machine_run_budget one = { 1u, 0u };
    const core_machine_run_budget insufficient = { 1u, 26u };
    core_machine_run_result result;
    timing_8086_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_8086_prepare(&machine, &state);

    if (!failed) {
        failed |= !timing_8086_load(machine, group2, sizeof(group2)) ||
            !timing_8086_execute(machine, 1u, 2u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, nop, sizeof(nop)) ||
            !timing_8086_execute(machine, 1u, 3u, &state) ||
            core_machine_reset(machine) != TYPE_STATUS_OK ||
            machine->elapsed_ticks != 0u || state.advanced_ticks != 0u ||
            !timing_8086_load(machine, nop, sizeof(nop)) ||
            !timing_8086_execute(machine, 1u, 3u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, nop, sizeof(nop)) ||
            core_machine_request_stop(machine) != TYPE_STATUS_OK ||
            core_machine_run(machine, one, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_REQUESTED || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != 0u ||
            state.advanced_ticks != 0u;
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, fault, sizeof(fault)) ||
            !test_core_machine_fixture_preflight_real_ud_terminal(machine) ||
            core_machine_run(machine, one, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != 0u ||
            state.advanced_ticks != 0u;
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, maximum, sizeof(maximum)) ||
            ((machine->executor_cpu.data.bp = 1u),
                (machine->executor_cpu.data.di = 0u),
                (machine->executor_cpu.data.cx = 0x5aa5u), 0) ||
            core_machine_run(machine, insufficient, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
            result.ticks != 0u || !timing_8086_execute(machine, 1u, 27u, &state);
    }
    if (!failed) {
        failed |= !timing_8086_load(machine, nop, sizeof(nop));
        machine->elapsed_ticks = UINT64_MAX - 2u;
        state.advanced_ticks = 0u;
        failed |= core_machine_run(machine, one, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != UINT64_MAX - 2u ||
            machine->elapsed_ticks != UINT64_MAX - 2u || state.advanced_ticks != 0u;
    }
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    C_INT failure = timing_8086_test_baseline() ? 1 :
        timing_8086_test_memory() ? 2 :
        timing_8086_test_alu_and_cmp_forms() ? 3 :
        timing_8086_test_primary_remaining_forms() ? 4 :
        timing_8086_test_segment_and_pointer_transfers() ? 5 :
        timing_8086_test_wait_ticks() ? 6 :
        timing_8086_test_group3_operand_model() ? 7 :
        timing_8086_test_group2_forms() ? 8 :
        timing_8086_test_control_repeat_and_ports() ? 9 :
        timing_8086_test_fallback_fault_budget_and_overflow() ? 10 : 0;

    if (failure != 0) {
        STD_PRINTF("M5:T357:S4:8086-INSTRUCTION-TIMING-LEDGER:FAIL:%d\n",
            failure);
        return failure;
    }
    STD_PRINTF("M5:T357:S4:8086-INSTRUCTION-TIMING-LEDGER:OK\n");
    return 0;
}
