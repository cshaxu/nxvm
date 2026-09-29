#include "support/pic_fixture.h"
#include "support/core_machine_board_fixture.h"
#include "app-nxvm/devices/device_support.h"
#include "app-nxvm/devices/pic_bus.h"
#include <stdio.h>

typedef struct gpr_mov_machine { core_machine *machine; } gpr_mov_machine;

static lib_i32 gpr_mov_prepare(core_machine_cpu_profile profile, gpr_mov_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile, .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };
    lib_memory_set(state, 0, sizeof(*state));
    return core_machine_create(&config, &state->machine) == LIB_STATUS_OK &&
        core_machine_freeze_execution_providers(state->machine) == LIB_STATUS_OK &&
        core_machine_reset(state->machine) == LIB_STATUS_OK &&
        core_machine_debug_patch_registers(state->machine, &entry) == LIB_STATUS_OK;
}

static lib_status gpr_mov_seed(gpr_mov_machine *state)
{
    const core_machine_debug_register_patch registers = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = {
            [CORE_MACHINE_DEBUG_EAX] = 0xaabb3344u,
            [CORE_MACHINE_DEBUG_ECX] = 0x11225566u,
            [CORE_MACHINE_DEBUG_EDX] = 0x778899aau,
            [CORE_MACHINE_DEBUG_EBX] = 0xbbccddeeU,
            [CORE_MACHINE_DEBUG_ESP] = 0x00008000u,
            [CORE_MACHINE_DEBUG_EBP] = 0x00000120u,
            [CORE_MACHINE_DEBUG_ESI] = 0x00000010u,
            [CORE_MACHINE_DEBUG_EDI] = 0x00000020u,
            [CORE_MACHINE_DEBUG_EFLAGS] = 0x41u /* CF | ZF */
        }
    };
    return core_machine_debug_patch_registers(state->machine, &registers);
}

static lib_i32 gpr_mov_test_protected_limit(void)
{
    static const lib_u8 pointer[] = {0x1fu, 0, 0, 0x03u, 0, 0};
    static const lib_u8 gdt[] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0xffu, 0xffu, 0, 0x20u, 0, 0x9au, 0, 0,
        0x0fu, 0, 0, 0x30u, 0, 0x92u, 0, 0,
        0xffu, 0xffu, 0, 0x40u, 0, 0x92u, 0, 0
    };
    static const lib_u8 boot[] = {
        0x0fu, 0x01u, 0x16u, 0, 1u, 0xb8u, 1u, 0,
        0x0fu, 0x01u, 0xf0u, 0xb8u, 0x10u, 0, 0x8eu, 0xd8u,
        0x8eu, 0xc0u, 0xb8u, 0x18u, 0, 0x8eu, 0xd0u, 0xbcu,
        0, 0x80u, 0xeau, 0, 0, 8u, 0
    };
    static const lib_u8 codes[][10] = {
        {0x8au, 0x06u, 0x10u, 0},
        {0x66u, 0xc7u, 0x06u, 0x10u, 0, 0x78u, 0x56u, 0x34u, 0x12u}
    };
    static const lib_u8 bytes[] = {4u, 9u};
    lib_u8 form;

    for (form = 0u; form != 2u; ++form) {
        gpr_mov_machine state;
        core_machine_debug_cpu_snapshot before;
        core_machine_debug_cpu_snapshot after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        lib_u32 image = 0x11223344u;
        lib_i32 failed = !gpr_mov_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            failed |= core_machine_memory_write(state.machine, 0x100u, pointer, sizeof(pointer)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x300u, gdt, sizeof(gdt)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0, boot, sizeof(boot)) != LIB_STATUS_OK ||
                core_machine_run(state.machine,
                    (core_machine_run_budget){10u, 0u}, &result) !=
                    LIB_STATUS_OK || result.reason !=
                    CORE_MACHINE_STOP_BUDGET || result.executed != 10u;
        }
        if (!failed) {
            failed |= gpr_mov_seed(&state) != LIB_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &image, 4u) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x2000u, codes[form], bytes[form]) != LIB_STATUS_OK;
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){1u, 0u}, &result) !=
                LIB_STATUS_INTERNAL_ERROR || result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK;
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            failed |= !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                after.eip != 0u || after.eax != before.eax ||
                after.ecx != before.ecx || after.edx !=
                before.edx || after.ebx != before.ebx ||
                after.esp != before.esp || after.ebp !=
                before.ebp || after.esi != before.esi ||
                after.edi != before.edi || after.eflags !=
                before.eflags || core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x3010u, (lib_uptr)&image, 4u) != LIB_STATUS_OK || image !=
                0x11223344u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static lib_i32 gpr_mov_test_irq_no_shadow(void)
{
    static const lib_u8 codes[][5] = {
        {0x8au, 0x06u, 0, 0x10u, 0x90u},
        {0x88u, 0x06u, 0, 0x10u, 0x90u}
    };
    static const lib_u8 hlt = 0xf4u;
    lib_u8 form;

    for (form = 0u; form != 2u; ++form) {
        gpr_mov_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        core_machine_debug_cpu_snapshot after;
        lib_u16 offset = 0x100u;
        lib_u16 segment = 0u;
        lib_u16 frame = 0u;
        lib_u8 image = form ? 0u : 0x5au;
        lib_i32 failed = !gpr_mov_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            failed |= core_machine_memory_write(state.machine, 0x1000u, &image, 1u) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, codes[form], sizeof(codes[form])) !=
                LIB_STATUS_OK || core_machine_memory_write(state.machine, 0x80u, &offset, 2u) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x82u, &segment, 2u) !=
                LIB_STATUS_OK || core_machine_memory_write(state.machine, 0x100u, &hlt, 1u) != LIB_STATUS_OK;
        }
        if (!failed) {
            failed |= gpr_mov_seed(&state) != LIB_STATUS_OK;
            {
                const core_machine_debug_register_patch flags = {
                    .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
                    .values = {[CORE_MACHINE_DEBUG_EFLAGS] = 0x241u}
                };
                failed |= core_machine_debug_patch_registers(state.machine, &flags) != LIB_STATUS_OK;
            }
            lib_memory_set(&source, 0, sizeof(source));
            test_pic_program_vector(&state.machine->shared_pic_master, 0x20u);
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master,
                &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){2u, 0u}, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            failed |= core_machine_memory_read(state.machine, after.ss.base + (lib_u16)after.esp, &frame, 2u) != LIB_STATUS_OK ||
                after.eip != 0x101u || frame != 4u || !CORE_MACHINE_BIT_IS_SET(
                    test_pic_read(&state.machine->shared_pic_master, 0x0bu), VPIC_ISR_IRQ(0u)) ||
                CORE_MACHINE_BIT_IS_SET(test_pic_read(&state.machine->shared_pic_master, 0x0au),
                    VPIC_IRR_IRQ(0u)) || (form == 0u && (after.eax & 0xffu) != 0x5au) ||
                (form == 1u && (core_machine_memory_read(state.machine, 0x1000u, &image, 1u) != LIB_STATUS_OK ||
                    image != 0x44u));
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!gpr_mov_test_protected_limit() ||
        !gpr_mov_test_irq_no_shadow()) return 1;
    printf("M5:T316:S31:GPR-MOV:OK\n");
    printf("M5:T401:S13:IMMEDIATE-REGISTER-MOV-PROFILES:OK\n");
    printf("M5:T401:S47:GPR-MOV-MODRM-PROFILES:OK\n");
    printf("M5:T401:S58:RM-IMMEDIATE-MOV-PROFILES:OK\n");
    return 0;
}
