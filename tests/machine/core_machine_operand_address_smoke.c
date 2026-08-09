#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define OAS_GDT_POINTER 0x0100u
#define OAS_GDT_ADDRESS 0x0300u
#define OAS_CODE_ADDRESS 0x2000u
#define OAS_DATA_ADDRESS 0x3000u
#define OAS_STACK_ADDRESS 0x4000u

typedef struct oas_machine {
    core_machine *machine;
} oas_machine;

static C_VOID oas_reset(C_VOID *opaque)
{
    oas_machine *state = (oas_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider oas_provider = {
    oas_reset, STD_NULL, STD_NULL
};

static C_INT oas_write(oas_machine *state, uint32_t address,
    const C_VOID *bytes, STD_SIZE_T byte_count)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_write(state->machine, address, bytes, byte_count) ==
            TYPE_STATUS_OK;
}

static C_INT oas_prepare(oas_machine *state, core_machine_cpu_profile profile,
    C_INT code32)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    const uint8_t gdt_pointer[] = { 0x1fu,0,0,0x03u,0,0 };
    uint8_t gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0xcfu,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0x40u,0
    };
    const uint8_t real_code[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,
        0xb8u,0x18u,0x00u,0x8eu,0xd0u,
        0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    const uint8_t halt[] = { 0xf4u };
    const core_machine_run_budget budget = { 96u, 0u };
    core_machine_run_result result;

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    gdt[14] = code32 ? 0x40u : 0u;
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK ||
        core_machine_bind_execution_provider(state->machine, &oas_provider,
            state) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(state->machine) != TYPE_STATUS_OK ||
        core_machine_reset(state->machine) != TYPE_STATUS_OK ||
        !oas_write(state, OAS_GDT_POINTER, gdt_pointer, sizeof(gdt_pointer)) ||
        !oas_write(state, OAS_GDT_ADDRESS, gdt, sizeof(gdt)) ||
        !oas_write(state, 0u, real_code, sizeof(real_code)) ||
        !oas_write(state, OAS_CODE_ADDRESS, halt, sizeof(halt)) ||
        core_machine_run(state->machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT oas_run_halt(oas_machine *state, const uint8_t *code,
    STD_SIZE_T code_size, t_cpu *out_cpu)
{
    const core_machine_run_budget budget = { 48u, 0u };
    core_machine_run_result result;
    type_status status;

    if (!oas_write(state, OAS_CODE_ADDRESS, code, code_size)) return 0;
    test_core_machine_fixture_resume_after_halt_at(state->machine, 0u);
    status = core_machine_run(state->machine, budget, &result);
    if (status != TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) return 0;
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static C_INT oas_run_gp(oas_machine *state, const uint8_t *code,
    STD_SIZE_T code_size, t_cpu *out_cpu)
{
    const core_machine_run_budget budget = { 16u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_status status;

    if (!oas_write(state, OAS_CODE_ADDRESS, code, code_size)) return 0;
    test_core_machine_fixture_resume_after_halt_at(state->machine, 0u);
    status = core_machine_run(state->machine, budget, &result);
    if (status != TYPE_STATUS_FAULT ||
        result.reason != CORE_MACHINE_STOP_FAULT ||
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) != TYPE_STATUS_OK ||
        !diagnostic.first_fault.valid ||
        !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_GP)) return 0;
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static C_INT oas_test_prefix_and_ea(C_VOID)
{
    static const uint8_t repeat_prefix[] = {
        0x66u,0x66u,0xb8u,0x34u,0x12u,0xb9u,0x78u,0x56u,0x34u,0x12u,0xf4u
    };
    static const uint8_t address16[] = { 0x67u,0x8bu,0x00u,0xf4u };
    static const uint8_t operand16[] = { 0x66u,0x8bu,0x00u,0xf4u };
    static const uint8_t moffs16[] = { 0x67u,0xa1u,0x60u,0x00u,0xf4u };
    static const uint8_t sib_scaled[] = { 0x8bu,0x44u,0x88u,0xfcu,0xf4u };
    static const uint8_t sib_absolute[] = {
        0x8bu,0x04u,0x25u,0x20u,0x01u,0x00u,0x00u,0xf4u
    };
    static const uint8_t ss_default[] = { 0x8bu,0x45u,0x00u,0xf4u };
    static const uint8_t ds_override[] = { 0x3eu,0x8bu,0x45u,0x00u,0xf4u };
    const uint32_t word_source = 0x1234beefu;
    const uint32_t sib_source = 0x87654321u;
    const uint32_t absolute_source = 0x0badf00du;
    const uint32_t ds_source = 0x13579bdfu;
    const uint32_t ss_source = 0x2468ace0u;
    oas_machine state;
    t_cpu cpu;
    C_INT failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);

    if (!failed) failed |= !oas_run_halt(&state, repeat_prefix,
        sizeof(repeat_prefix), &cpu) || cpu.data.eax != 0x00001234u ||
        cpu.data.ecx != 0x12345678u;
    core_machine_destroy(state.machine);
    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.machine->executor_cpu.data.ebx = 0x0040u;
        state.machine->executor_cpu.data.esi = 0x0010u;
        failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x0050u, &word_source,
                sizeof(word_source)) || !oas_run_halt(&state, address16,
                sizeof(address16), &cpu) || cpu.data.eax != word_source;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0x0070u;
        failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x0070u, &word_source,
                sizeof(word_source)) || !oas_run_halt(&state, operand16,
                sizeof(operand16), &cpu) || cpu.data.eax != 0x0000beefu;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x0060u, &word_source,
                sizeof(word_source)) || !oas_run_halt(&state, moffs16,
                sizeof(moffs16), &cpu) || cpu.data.eax != word_source;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0x0100u;
        state.machine->executor_cpu.data.ecx = 1u;
        failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x0100u, &sib_source,
                sizeof(sib_source)) || !oas_run_halt(&state, sib_scaled,
                sizeof(sib_scaled), &cpu) || cpu.data.eax != sib_source;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x0120u,
            &absolute_source, sizeof(absolute_source)) || !oas_run_halt(&state,
            sib_absolute, sizeof(sib_absolute), &cpu) ||
            cpu.data.eax != absolute_source;
    core_machine_destroy(state.machine);
    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.machine->executor_cpu.data.ebp = 0x0080u;
        failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x0080u, &ds_source,
                sizeof(ds_source)) || !oas_write(&state, OAS_STACK_ADDRESS + 0x0080u,
                &ss_source, sizeof(ss_source)) || !oas_run_halt(&state,
                ss_default, sizeof(ss_default), &cpu) || cpu.data.eax != ss_source;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.machine->executor_cpu.data.ebp = 0x0080u;
        failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x0080u, &ds_source,
                sizeof(ds_source)) || !oas_write(&state, OAS_STACK_ADDRESS + 0x0080u,
                &ss_source, sizeof(ss_source)) || !oas_run_halt(&state,
                ds_override, sizeof(ds_override), &cpu) || cpu.data.eax != ds_source;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT oas_test_16bit_code_and_faults(C_VOID)
{
    static const uint8_t defaults16[] = {
        0x66u,0xb8u,0x78u,0x56u,0x34u,0x12u,0xb9u,0x34u,0x12u,0xf4u
    };
    static const uint8_t address32[] = { 0x67u,0x8bu,0x00u,0xf4u };
    static const uint8_t invalid_data[] = {
        0x8bu,0x05u,0x20u,0x00u,0x00u,0x00u
    };
    static const uint8_t invalid_expand_down[] = {
        0x8bu,0x05u,0x10u,0x00u,0x00u,0x00u
    };
    static const uint8_t nop[] = { 0x90u };
    static const uint8_t halt[] = { 0xf4u };
    oas_machine state;
    t_cpu before;
    t_cpu after;
    const uint32_t source = 0xcafebabeu;
    C_INT failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 0);

    if (!failed) {
        state.machine->executor_cpu.data.ecx = 0xdead0000u;
        failed |= !oas_run_halt(&state, defaults16, sizeof(defaults16), &after) ||
            after.data.eax != 0x12345678u || after.data.ecx != 0xdead1234u;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.machine->executor_cpu.data.ds.limit = 0x10u;
        state.machine->executor_cpu.data.eax = 0xaabbccddu;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !oas_run_gp(&state, invalid_data, sizeof(invalid_data), &after) ||
            after.data.eax != before.data.eax;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.machine->executor_cpu.data.ds.limit = 0x10u;
        state.machine->executor_cpu.data.ds.seg.data.expdown = TYPE_TRUE;
        state.machine->executor_cpu.data.ds.seg.data.big = TYPE_FALSE;
        state.machine->executor_cpu.data.eax = 0x11223344u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !oas_run_gp(&state, invalid_expand_down,
            sizeof(invalid_expand_down), &after) ||
            after.data.eax != before.data.eax;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 0);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0x00010070u;
        failed |= !oas_write(&state, OAS_DATA_ADDRESS + 0x00010070u, &source,
                sizeof(source)) || !oas_run_halt(&state, address32,
                sizeof(address32), &after) || after.data.eax != 0x0001babeu;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 0);
    if (!failed) {
        failed |= !oas_write(&state, OAS_CODE_ADDRESS + 0xffffu, nop,
                sizeof(nop)) || !oas_write(&state, OAS_CODE_ADDRESS, halt,
                sizeof(halt));
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0xffffu);
        { const core_machine_run_budget budget = { 8u, 0u }; core_machine_run_result result;
          failed |= core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
              result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT; }
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= after.data.eip != 1u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_VOID oas_set_stack32(oas_machine *state, uint32_t esp)
{
    state->machine->executor_cpu.data.ss.seg.data.big = TYPE_TRUE;
    state->machine->executor_cpu.data.ss.limit = 0xffffffffu;
    state->machine->executor_cpu.data.esp = esp;
}

static C_INT oas_test_stack_forms(C_VOID)
{
    static const uint8_t push_pop32[] = { 0x50u,0x59u,0xf4u };
    static const uint8_t push_pop16[] = { 0x66u,0x50u,0x66u,0x5bu,0xf4u };
    static const uint8_t pusha[] = { 0x60u,0xf4u };
    static const uint8_t popa[] = { 0x61u,0xf4u };
    static const uint8_t pushf[] = { 0x9cu,0xf4u };
    static const uint8_t popf[] = { 0x9du,0xf4u };
    static const uint8_t enter_leave[] = {
        0xc8u,0x10u,0x00u,0x02u,0xc9u,0xf4u
    };
    static const uint8_t enter_leave16[] = {
        0x66u,0xc8u,0x08u,0x00u,0x01u,0x66u,0xc9u,0xf4u
    };
    const uint32_t ignored_slot = 0xfeedfaceu;
    const uint32_t flags = VCPU_EFLAGS_IF | 0x00000002u;
    oas_machine state;
    t_cpu before;
    t_cpu after;
    C_INT failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);

    if (!failed) {
        state.machine->executor_cpu.data.esp = 0x0100u;
        state.machine->executor_cpu.data.eax = 0x11223344u;
        failed |= !oas_run_halt(&state, push_pop32, sizeof(push_pop32), &after) ||
            after.data.esp != 0x0100u || after.data.ecx != 0x11223344u;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.machine->executor_cpu.data.esp = 0x0100u;
        state.machine->executor_cpu.data.eax = 0x11223344u;
        failed |= !oas_run_halt(&state, push_pop16, sizeof(push_pop16), &after) ||
            after.data.esp != 0x0100u || after.data.ebx != 0x00003344u;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        oas_set_stack32(&state, 0x00010100u);
        state.machine->executor_cpu.data.eax = 0x55667788u;
        failed |= !oas_run_halt(&state, push_pop16, sizeof(push_pop16), &after) ||
            after.data.esp != 0x00010100u || after.data.ebx != 0x00007788u;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        oas_set_stack32(&state, 0x00000100u);
        state.machine->executor_cpu.data.eax = 0x11111111u;
        state.machine->executor_cpu.data.ecx = 0x22222222u;
        state.machine->executor_cpu.data.edx = 0x33333333u;
        state.machine->executor_cpu.data.ebx = 0x44444444u;
        state.machine->executor_cpu.data.ebp = 0x55555555u;
        state.machine->executor_cpu.data.esi = 0x66666666u;
        state.machine->executor_cpu.data.edi = 0x77777777u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !oas_run_halt(&state, pusha, sizeof(pusha), &after) ||
            after.data.esp != 0x000000e0u ||
            !oas_write(&state, OAS_STACK_ADDRESS + 0x000000ecu, &ignored_slot,
                sizeof(ignored_slot)) || !oas_run_halt(&state, popa,
                sizeof(popa), &after) || after.data.esp != before.data.esp ||
            after.data.eax != before.data.eax || after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        oas_set_stack32(&state, 0x00000100u);
        state.machine->executor_cpu.data.eflags = 0x00000002u;
        failed |= !oas_run_halt(&state, pushf, sizeof(pushf), &after) ||
            !oas_write(&state, OAS_STACK_ADDRESS + after.data.esp, &flags,
                sizeof(flags)) || !oas_run_halt(&state, popf, sizeof(popf),
                &after) || after.data.esp != 0x00000100u ||
            (after.data.eflags & VCPU_EFLAGS_IF) == 0u;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        oas_set_stack32(&state, 0x00000100u);
        state.machine->executor_cpu.data.ebp = 0x00000080u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !oas_run_halt(&state, enter_leave, sizeof(enter_leave), &after) ||
            after.data.esp != before.data.esp || after.data.ebp != before.data.ebp;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !oas_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        oas_set_stack32(&state, 0x00000100u);
        state.machine->executor_cpu.data.ebp = 0x00000080u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !oas_run_halt(&state, enter_leave16, sizeof(enter_leave16),
            &after) || after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    if (!oas_test_prefix_and_ea()) {
        return 1;
    }
    if (!oas_test_16bit_code_and_faults()) {
        return 1;
    }
    if (!oas_test_stack_forms()) {
        return 1;
    }
    STD_PRINTF("M5:T302:OPERAND-ADDRESS-STACK:OK\n");
    return 0;
}
