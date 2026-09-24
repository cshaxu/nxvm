#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

#define MOVX_SOURCE_MEMORY 0x5000u

typedef struct movx_provider {
    lib_u32 reads;
    lib_u8 value[2];
    lib_status read_status;
} movx_provider;

typedef struct movx_machine {
    core_machine *machine;
} movx_machine;

typedef struct movx_form {
    lib_u8 opcode;
    lib_u32 source;
    lib_u32 result;
    lib_u8 source_bytes;
} movx_form;

static lib_status movx_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes)
{
    movx_provider *provider = (movx_provider *)owner;

    if (provider == LIB_NULL || physical != MOVX_SOURCE_MEMORY ||
        (bytes != 1u && bytes != 2u)) return LIB_STATUS_INVALID_ARGUMENT;
    ++provider->reads;
    if (provider->read_status != LIB_STATUS_OK) return provider->read_status;
    lib_memory_copy((void *)destination, provider->value, bytes);
    return LIB_STATUS_OK;
}

static lib_status movx_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    (void)owner;
    (void)physical;
    (void)source;
    (void)bytes;
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status movx_query(void *owner, lib_u32 physical,
    lib_uptr bytes, core_machine_memory_access access)
{
    (void)owner;
    return physical == MOVX_SOURCE_MEMORY && (bytes == 1u || bytes == 2u) &&
        (access == CORE_MACHINE_MEMORY_ACCESS_READ ||
         access == CORE_MACHINE_MEMORY_ACCESS_WRITE) ? LIB_STATUS_OK :
        LIB_STATUS_UNSUPPORTED;
}

static void movx_reset(void *opaque)
{
    movx_machine *state = (movx_machine *)opaque;

    if (state != LIB_NULL) (void)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider movx_execution_provider = {
    movx_reset, LIB_NULL
};

static lib_i32 movx_prepare(core_machine_cpu_profile profile,
    movx_provider *provider, movx_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == LIB_NULL) return 0;
    lib_memory_set(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != LIB_STATUS_OK ||
        (provider != LIB_NULL && test_core_machine_fixture_register_memory_device_provider(
            state->machine, MOVX_SOURCE_MEMORY, 2u, movx_read, movx_write,
            movx_query, provider) != LIB_STATUS_OK) ||
        !test_core_machine_fixture_bind_freeze_reset(state->machine,
            &movx_execution_provider, state)) {
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 0;
    }
    return 1;
}

static lib_i32 movx_run(movx_machine *state, const lib_u8 *code,
    lib_size code_size, lib_i32 expect_fault, t_cpu *out_cpu,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    const core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    lib_status status;

    if (state == LIB_NULL || state->machine == LIB_NULL || code == LIB_NULL ||
        out_cpu == LIB_NULL || out_diagnostic == LIB_NULL ||
        !test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u) ||
        core_machine_memory_write(state->machine, 0u, code, code_size) !=
            LIB_STATUS_OK) return 0;
    if (expect_fault && !test_core_machine_fixture_preflight_real_ud_terminal(
            state->machine)) return 0;
    status = core_machine_run(state->machine, budget, &result);
    if (status != (expect_fault ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) ||
        result.reason != (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_BUDGET) || core_machine_get_cpu_diagnostic(
            state->machine, out_diagnostic) != LIB_STATUS_OK) return 0;
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static lib_i32 movx_test_forms(void)
{
    static const movx_form forms[] = {
        {0xb6u, 0x00000080u, 0x00000080u, 1u},
        {0xb7u, 0x00008001u, 0x00008001u, 2u},
        {0xbeu, 0x00000080u, 0xffffff80u, 1u},
        {0xbfu, 0x00008001u, 0xffff8001u, 2u}
    };
    lib_u8 form_index;
    lib_i32 operand32;
    lib_i32 memory;

    for (form_index = 0u; form_index != sizeof(forms) / sizeof(forms[0]); ++form_index) {
        for (operand32 = 0; operand32 != 2; ++operand32) {
            for (memory = 0; memory != 2; ++memory) {
                lib_u8 code[6] = {0};
                lib_u8 source[2] = {
                    (lib_u8)forms[form_index].source,
                    (lib_u8)(forms[form_index].source >> 8u)
                };
                const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF |
                    VCPU_EFLAGS_SF;
                const lib_u32 expected = operand32 ? forms[form_index].result :
                    (0xaabb0000u | (forms[form_index].result & 0xffffu));
                const lib_size code_size = (operand32 ? 1u : 0u) +
                    (memory ? 5u : 3u);
                movx_machine state;
                t_cpu after;
                core_machine_cpu_diagnostic diagnostic;
                lib_i32 failed = !movx_prepare(CORE_MACHINE_CPU_PROFILE_80386,
                    LIB_NULL, &state);
                lib_size index = 0u;

                if (operand32) code[index++] = 0x66u;
                code[index++] = 0x0fu;
                code[index++] = forms[form_index].opcode;
                if (memory) {
                    code[index++] = 0x0eu;
                    code[index++] = 0x00u;
                    code[index++] = 0x40u;
                } else code[index++] = 0xc8u;
                if (!failed) {
                    state.machine->executor_cpu.data.eax = forms[form_index].source;
                    state.machine->executor_cpu.data.ecx = 0xaabbccddu;
                    state.machine->executor_cpu.data.eflags = flags;
                    failed |= memory && core_machine_memory_write(state.machine,
                        0x4000u, source, forms[form_index].source_bytes) != LIB_STATUS_OK;
                    failed |= !movx_run(&state, code, code_size, 0, &after,
                        &diagnostic) || diagnostic.first_fault.valid ||
                        after.data.ecx != expected || after.data.eflags != flags ||
                        after.data.eip != code_size;
                }
                core_machine_destroy(state.machine);
                if (failed) return 0;
            }
        }
    }
    return 1;
}

static lib_i32 movx_test_address_prefix(void)
{
    static const lib_u8 code[] = {0x67u,0x66u,0x0fu,0xbfu,0x0eu};
    const lib_u8 source[] = {0x01u,0x80u};
    const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
    movx_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    lib_i32 failed = !movx_prepare(CORE_MACHINE_CPU_PROFILE_80386, LIB_NULL,
        &state);

    if (!failed) {
        state.machine->executor_cpu.data.ecx = 0xaabbccddu;
        state.machine->executor_cpu.data.esi = 0x4000u;
        state.machine->executor_cpu.data.eflags = flags;
        failed |= core_machine_memory_write(state.machine, 0x4000u, source,
            sizeof(source)) != LIB_STATUS_OK || !movx_run(&state, code,
            sizeof(code), 0, &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.ecx != 0xffff8001u || after.data.esi != 0x4000u ||
            after.data.eflags != flags || after.data.eip != sizeof(code);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 movx_prepare_protected_limit(movx_machine *state)
{
    static const lib_u8 gdt_pointer[] = {0x1fu,0,0,0x03u,0,0};
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0x0fu,0,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0x40u,0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,
        0xb8u,0x18u,0x00u,0x8eu,0xd0u,
        0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const lib_u8 halt[] = {0xf4u};
    const core_machine_run_budget budget = {96u, 0u};
    core_machine_run_result result;

    return movx_prepare(CORE_MACHINE_CPU_PROFILE_80386, LIB_NULL, state) &&
        core_machine_memory_write(state->machine, 0x0100u, gdt_pointer,
            sizeof(gdt_pointer)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0300u, gdt, sizeof(gdt)) ==
            LIB_STATUS_OK && core_machine_memory_write(state->machine, 0u,
            bootstrap, sizeof(bootstrap)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x2000u, halt, sizeof(halt)) ==
            LIB_STATUS_OK && core_machine_run(state->machine, budget, &result) ==
            LIB_STATUS_OK && result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static lib_i32 movx_test_read_boundaries(void)
{
    static const lib_u8 code[] = {0x0fu,0xb6u,0x0eu,0x00u,0x50u};
    const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
    core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286
    };
    lib_u8 opcode;
    lib_u8 profile_index;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
         ++profile_index) {
        for (opcode = 0xb6u; opcode <= 0xbfu; ++opcode) {
            movx_provider provider = {0u, {0x80u,0x01u}, LIB_STATUS_OK};
            movx_machine state;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            lib_i32 failed = !movx_prepare(profiles[profile_index], &provider,
                &state);
            lib_u8 form_code[sizeof(code)];

            if (opcode == 0xb8u || opcode == 0xb9u || opcode == 0xbau ||
                opcode == 0xbbu || opcode == 0xbcu || opcode == 0xbdu) continue;
            lib_memory_copy(form_code, code, sizeof(code));
            form_code[2] = opcode;
            if (!failed) {
                state.machine->executor_cpu.data.ecx = 0xaabbccddu;
                state.machine->executor_cpu.data.eflags = flags;
                failed |= !movx_run(&state, form_code, sizeof(form_code), 1,
                    &after, &diagnostic) || !diagnostic.first_fault.valid ||
                    !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                        VCPUINS_EXCEPT_UD) || provider.reads != 0u ||
                    after.data.ecx != 0xaabbccddu || after.data.eflags != flags ||
                    after.data.eip != 0u;
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }

    {
        static const lib_u8 limit_code[] = {0x0fu,0xbfu,0x0eu,0x10u,0x00u};
        const core_machine_run_budget budget = {1u, 0u};
        movx_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        lib_i32 failed = !movx_prepare_protected_limit(&state);

        if (!failed) {
            state.machine->executor_cpu.data.ecx = 0xaabbccddu;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x2000u,
                limit_code, sizeof(limit_code)) != LIB_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, budget, &result) != LIB_STATUS_INTERNAL_ERROR ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                    LIB_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                after.data.ecx != 0xaabbccddu || after.data.eflags != flags ||
                after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!movx_test_forms() || !movx_test_address_prefix() ||
        !movx_test_read_boundaries()) return 1;
    printf("M5:T310:S4:MOVX:OK\n");
    printf("M5:T401:S64:MOVX-PROFILES:OK\n");
    return 0;
}
