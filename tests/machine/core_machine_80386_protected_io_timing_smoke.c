#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define TIMING_S7_RESET_LINEAR 0xfffffff0u
#define TIMING_S7_RESET_PHYSICAL 0x000ffff0u
#define TIMING_S7_TSS_BASE 0x0600u
#define TIMING_S7_IOMAP_BASE 0x0080u

typedef struct timing_s7_state {
    type_unsigned_32 reads;
    type_unsigned_32 writes;
    type_unsigned_64 advanced_ticks;
} timing_s7_state;

typedef struct timing_s7_form {
    type_unsigned_8 opcode;
    type_unsigned_64 protected_ticks;
    type_unsigned_64 permission_ticks;
    C_INT input;
} timing_s7_form;

static type_status timing_s7_port_read(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 *out_value)
{
    timing_s7_state *state = (timing_s7_state *)owner;

    if (state == STD_NULL || out_value == STD_NULL || port != 0x0080u)
        return TYPE_STATUS_INVALID_ARGUMENT;
    ++state->reads;
    *out_value = 0x5au;
    return TYPE_STATUS_OK;
}

static type_status timing_s7_port_write(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 value)
{
    timing_s7_state *state = (timing_s7_state *)owner;

    if (state == STD_NULL || port != 0x0080u || value > 0xffffu)
        return TYPE_STATUS_INVALID_ARGUMENT;
    ++state->writes;
    return TYPE_STATUS_OK;
}

static const core_machine_port_provider timing_s7_ports = {
    timing_s7_port_read, timing_s7_port_write
};

static C_VOID timing_s7_reset(C_VOID *opaque)
{
    timing_s7_state *state = (timing_s7_state *)opaque;
    if (state != STD_NULL) state->advanced_ticks = 0u;
}

static C_VOID timing_s7_advance(C_VOID *opaque, type_unsigned_64 ticks)
{
    timing_s7_state *state = (timing_s7_state *)opaque;
    if (state != STD_NULL) state->advanced_ticks += ticks;
}

static const core_machine_execution_provider timing_s7_execution = {
    timing_s7_reset, timing_s7_advance
};

static C_INT timing_s7_prepare(core_machine **out_machine, timing_s7_state *state)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .ticks_per_instruction = 29u,
        .instruction_timing = { 29u, 7u, 31u, 37u, 41u, 43u }
    };
    core_machine *machine = STD_NULL;

    if (out_machine == STD_NULL || state == STD_NULL ||
        core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            TIMING_S7_RESET_LINEAR, TIMING_S7_RESET_PHYSICAL, 16u) !=
            TYPE_STATUS_OK || core_machine_install_port_provider(machine,
            0x0080u, 0x0080u, &timing_s7_ports, state) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(machine,
            &timing_s7_execution, state)) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static C_INT timing_s7_load(core_machine *machine, type_unsigned_8 opcode)
{
    type_unsigned_8 code[] = { opcode, 0x80u };
    STD_SIZE_T bytes = opcode >= 0xecu ? 1u : sizeof(code);

    return core_machine_reset(machine) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, TIMING_S7_RESET_LINEAR, code, bytes) ==
            TYPE_STATUS_OK;
}

static C_INT timing_s7_allow_permission(core_machine *machine, C_INT vm86,
    type_unsigned_8 bitmap)
{
    type_unsigned_16 iomap_base = TIMING_S7_IOMAP_BASE;

    if (machine == STD_NULL) return 0;
    machine->executor_cpu.data.cr0 |= VCPU_CR0_PE;
    machine->executor_cpu.data.eflags = vm86 ?
        VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL : 0u;
    machine->executor_cpu.data.cs.selector = 0x001bu;
    machine->executor_cpu.data.cs.dpl = 3u;
    machine->executor_cpu.data.ss.dpl = 3u;
    machine->executor_cpu.data.ds.selector = 0x0023u;
    machine->executor_cpu.data.ds.dpl = 3u;
    machine->executor_cpu.data.es.selector = 0x0023u;
    machine->executor_cpu.data.es.dpl = 3u;
    machine->executor_cpu.data.tr.flagValid = TYPE_TRUE;
    machine->executor_cpu.data.tr.selector = 0x0028u;
    machine->executor_cpu.data.tr.base = TIMING_S7_TSS_BASE;
    machine->executor_cpu.data.tr.limit = 0x00ffu;
    machine->executor_cpu.data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
    return core_machine_memory_write(machine, TIMING_S7_TSS_BASE + 0x66u,
        &iomap_base, sizeof(iomap_base)) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, TIMING_S7_TSS_BASE + iomap_base +
            0x10u, &bitmap, sizeof(bitmap)) == TYPE_STATUS_OK;
}

static C_INT timing_s7_run_form(const timing_s7_form *form, C_INT mode)
{
    timing_s7_state state = { 0u, 0u, 0u };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine *machine = STD_NULL;
    type_unsigned_64 ticks = mode == 0 ? form->protected_ticks :
        form->permission_ticks;
    C_INT failed = !timing_s7_prepare(&machine, &state) ||
        !timing_s7_load(machine, form->opcode);

    if (!failed && mode == 0) {
        machine->executor_cpu.data.cr0 |= VCPU_CR0_PE;
        machine->executor_cpu.data.cs.selector = 0x0008u;
        machine->executor_cpu.data.cs.dpl = 0u;
        machine->executor_cpu.data.eflags = 0u;
    }
    if (!failed && mode != 0) failed |= !timing_s7_allow_permission(machine,
        mode == 2, 0u);
    if (!failed) {
        machine->executor_cpu.data.eax = 0x11223344u;
        machine->executor_cpu.data.edx = 0x00000080u;
        failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            result.ticks != ticks || result.elapsed_ticks != ticks ||
            state.advanced_ticks != ticks ||
            (form->input ? state.reads != 1u || state.writes != 0u :
                state.reads != 0u || state.writes != 1u);
    }
    core_machine_destroy(machine);
    return !failed;
}

static C_INT timing_s7_test_success(C_VOID)
{
    static const timing_s7_form forms[] = {
        { 0xe4u, 6u, 26u, 1 }, { 0xe5u, 6u, 26u, 1 },
        { 0xecu, 7u, 27u, 1 }, { 0xedu, 7u, 27u, 1 },
        { 0xe6u, 4u, 24u, 0 }, { 0xe7u, 4u, 24u, 0 },
        { 0xeeu, 5u, 25u, 0 }, { 0xefu, 5u, 25u, 0 }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
        if (!timing_s7_run_form(&forms[index], 0) ||
            !timing_s7_run_form(&forms[index], 1) ||
            !timing_s7_run_form(&forms[index], 2)) return 0;
    }
    return 1;
}

static C_INT timing_s7_test_denied(C_VOID)
{
    timing_s7_state state = { 0u, 0u, 0u };
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_s7_prepare(&machine, &state) ||
        !timing_s7_load(machine, 0xe4u);

    if (!failed) failed |= !timing_s7_allow_permission(machine, 0, 0x01u);
    if (!failed) {
        failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != 0u ||
            state.advanced_ticks != 0u || state.reads != 0u || state.writes != 0u;
    }
    core_machine_destroy(machine);
    return !failed;
}

static C_INT timing_s7_test_permission_strings(C_VOID)
{
    static const timing_s7_form forms[] = {
        { 0x6cu, 0u, 0u, 1 }, { 0x6eu, 0u, 0u, 0 }
    };
    C_INT vm86;
    STD_SIZE_T index;

    for (vm86 = 0; vm86 != 2; ++vm86) {
        for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
            type_unsigned_8 bitmap;

            for (bitmap = 0u; bitmap != 2u; ++bitmap) {
                timing_s7_state state = { 0u, 0u, 0u };
                const core_machine_run_budget budget = { 1u, 0u };
                core_machine_run_result result;
                core_machine *machine = STD_NULL;
                type_unsigned_8 source = 0x4au;
                type_unsigned_8 destination = 0u;
                C_INT failed = !timing_s7_prepare(&machine, &state) ||
                    !timing_s7_load(machine, forms[index].opcode) ||
                    !timing_s7_allow_permission(machine, vm86, bitmap);

                if (!failed) {
                    machine->executor_cpu.data.edx = 0x00000080u;
                    machine->executor_cpu.data.esi = 0x00000200u;
                    machine->executor_cpu.data.edi = 0x00000100u;
                    failed |= core_machine_memory_write(machine, 0x0200u, &source,
                        sizeof(source)) != TYPE_STATUS_OK;
                }
                if (!failed && bitmap == 0u) {
                    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
                        result.reason != CORE_MACHINE_STOP_BUDGET ||
                        result.executed != 1u ||
                        (forms[index].input ? state.reads != 1u || state.writes != 0u ||
                            machine->executor_cpu.data.edi != 0x00000101u ||
                            core_machine_memory_read(machine, 0x0100u, &destination,
                                sizeof(destination)) != TYPE_STATUS_OK || destination != 0x5au :
                            state.reads != 0u || state.writes != 1u ||
                            machine->executor_cpu.data.esi != 0x00000201u);
                }
                if (!failed && bitmap != 0u) {
                    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_FAULT ||
                        result.reason != CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
                        result.ticks != 0u || result.elapsed_ticks != 0u ||
                        state.reads != 0u || state.writes != 0u ||
                        machine->executor_cpu.data.esi != 0x00000200u ||
                        machine->executor_cpu.data.edi != 0x00000100u;
                }
                core_machine_destroy(machine);
                if (failed) return 0;
            }
        }
    }
    return 1;
}

static C_INT timing_s7_test_permission_budget(C_VOID)
{
    const core_machine_run_budget insufficient = { 1u, 26u };
    const core_machine_run_budget sufficient = { 1u, 106u };
    core_machine_run_result result;
    timing_s7_state state = { 0u, 0u, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = !timing_s7_prepare(&machine, &state) ||
        !timing_s7_load(machine, 0xecu) ||
        !timing_s7_allow_permission(machine, 0, 0u);

    if (!failed) machine->executor_cpu.data.edx = 0x00000080u;
    if (!failed) {
        failed |= core_machine_run(machine, insufficient, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 0u ||
            result.ticks != 0u || result.elapsed_ticks != 0u ||
            state.advanced_ticks != 0u || state.reads != 0u;
    }
    if (!failed) {
        failed |= core_machine_run(machine, sufficient, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            result.ticks != 27u || result.elapsed_ticks != 27u ||
            state.advanced_ticks != 27u || state.reads != 1u;
    }
    core_machine_destroy(machine);
    return !failed;
}

C_INT main(C_VOID)
{
    if (!timing_s7_test_success() || !timing_s7_test_denied() ||
        !timing_s7_test_permission_strings() ||
        !timing_s7_test_permission_budget())
        return 1;
    STD_PRINTF("M5:T357:S7:80386-PROTECTED-IO-TIMING:OK\n");
    STD_PRINTF("M5:T358:S1:IO-PERMISSION:OK\n");
    return 0;
}
