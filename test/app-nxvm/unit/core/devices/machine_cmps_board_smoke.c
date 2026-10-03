#include "support/core_machine_board_fixture.h"
#include "app-nxvm/devices/machine_board_state.h"
#include "support/pic_fixture.h"
#include "x86/core/device_support_interface.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include <stdio.h>

#define CMPS_FLAGS (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF)

static lib_i32 cmps_prepare_protected(core_machine **out_machine,
    const lib_u8 *code, lib_u8 bytes, lib_u16 ds_limit, lib_u16 es_limit)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    static const lib_u8 pointer[] = {0x27u,0,0,0x03u,0,0};
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0,0,0,0x30u,0,0x92u,0,0,
        0,0,0,0x40u,0,0x92u,0,0,
        0xffu,0xffu,0,0x50u,0,0x92u,0,0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0,1u, 0xb8u,1u,0, 0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0,0x8eu,0xd8u, 0xb8u,0x18u,0,0x8eu,0xc0u,
        0xb8u,0x20u,0,0x8eu,0xd0u, 0xbcu,0,0x80u,
        0xeau,0,0,8u,0
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };
    core_machine *machine = LIB_NULL;
    core_machine_run_result result = {0};

    *out_machine = LIB_NULL;
    gdt[16u] = (lib_u8)ds_limit;
    gdt[17u] = (lib_u8)(ds_limit >> 8u);
    gdt[24u] = (lib_u8)es_limit;
    gdt[25u] = (lib_u8)(es_limit >> 8u);
    if (core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x100u, pointer,
            sizeof(pointer)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x300u, gdt,
            sizeof(gdt)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0u, bootstrap,
            sizeof(bootstrap)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x2000u, code, bytes) !=
            LIB_STATUS_OK ||
        core_machine_run(machine, (core_machine_run_budget){11u, 0u},
            &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 11u) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static lib_i32 cmps_protected_case(lib_bool source_fault,
    lib_bool repeated)
{
    static const lib_u8 single[] = {0xa6u};
    static const lib_u8 rep[] = {0xf3u, 0xa6u};
    const lib_u8 *code = repeated ? rep : single;
    lib_u8 bytes = repeated ? sizeof(rep) : sizeof(single);
    core_machine *machine = LIB_NULL;
    core_machine_debug_register_patch patch = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_run_result result = {0};
    lib_u8 left[] = {0x10u, 1u}, right[] = {0x10u, 1u};
    lib_u16 ds_limit = repeated && !source_fault ? 0xffffu :
        repeated ? 0x10u : 0x0fu;
    lib_u16 es_limit = repeated && source_fault ? 0xffffu :
        repeated ? 0x10u : 0x0fu;
    lib_i32 failed = !cmps_prepare_protected(&machine, code, bytes,
        ds_limit, es_limit);

    if (!failed) {
        patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
        patch.values[CORE_MACHINE_DEBUG_EAX] = 0xaabbcc10u;
        patch.values[CORE_MACHINE_DEBUG_ECX] = 0x11220003u;
        patch.values[CORE_MACHINE_DEBUG_ESI] =
            repeated || source_fault ? 0x10u : 0u;
        patch.values[CORE_MACHINE_DEBUG_EDI] =
            repeated || !source_fault ? 0x10u : 0u;
        patch.values[CORE_MACHINE_DEBUG_EFLAGS] = VCPU_EFLAGS_IF;
        failed = core_machine_debug_patch_registers(machine, &patch) !=
            LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x3010u, left,
                sizeof(left)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x4010u, right,
                sizeof(right)) != LIB_STATUS_OK;
    }
    if (!failed)
        failed = core_machine_run(machine,
            (core_machine_run_budget){repeated ? 2u : 1u, 0u},
            &result) != LIB_STATUS_INTERNAL_ERROR ||
            result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(machine, &diagnostic) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
    if (!failed)
        failed = !diagnostic.first_fault.valid ||
            !(diagnostic.first_fault.exception_mask & VCPUINS_EXCEPT_DF) ||
            after.eip != 0u || after.eax != 0xaabbcc10u ||
            after.ecx != (repeated ? 0x11220002u : 0x11220003u) ||
            after.esi != (repeated ? 0x11u : source_fault ? 0x10u : 0u) ||
            after.edi != (repeated ? 0x11u : source_fault ? 0u : 0x10u) ||
            after.eflags != (repeated ? VCPU_EFLAGS_IF | VCPU_EFLAGS_PF |
                VCPU_EFLAGS_ZF : VCPU_EFLAGS_IF);
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 cmps_irq_case(lib_bool repeated)
{
    static const lib_u8 single[] = {0xa6u, 0x90u};
    static const lib_u8 rep[] = {0xf3u, 0xa6u, 0x90u};
    static const lib_u8 halt = 0xf4u;
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const lib_u8 *code = repeated ? rep : single;
    lib_u8 bytes = repeated ? sizeof(rep) : sizeof(single);
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_pic_irq_source irq = {0};
    core_machine_debug_register_patch patch = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_run_result result = {0};
    lib_u16 offset = 0x100u, segment = 0u, frame_ip = 0xffffu;
    lib_u8 left[] = {0x10u, 1u, 1u};
    lib_u8 right[] = {0x10u, 1u, 1u};
    lib_u8 observed_left[sizeof(left)] = {0};
    lib_u8 observed_right[sizeof(right)] = {0};
    lib_i32 failed = core_machine_create(&config, &machine, &board) != LIB_STATUS_OK;

    if (!failed) {
        patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI);
        patch.values[CORE_MACHINE_DEBUG_DS] = 0x2000u;
        patch.values[CORE_MACHINE_DEBUG_ES] = 0x3000u;
        patch.values[CORE_MACHINE_DEBUG_ESP] = 0x8000u;
        patch.values[CORE_MACHINE_DEBUG_EFLAGS] = VCPU_EFLAGS_IF;
        patch.values[CORE_MACHINE_DEBUG_EAX] = 0xaabbcc10u;
        patch.values[CORE_MACHINE_DEBUG_ECX] = 0x11220003u;
        patch.values[CORE_MACHINE_DEBUG_ESI] = 0x10u;
        patch.values[CORE_MACHINE_DEBUG_EDI] = 0x20u;
        failed = core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            core_machine_reset(machine) != LIB_STATUS_OK ||
            core_machine_debug_patch_registers(machine, &patch) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x20010u, left,
                sizeof(left)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x30020u, right,
                sizeof(right)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0u, code, bytes) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x80u, &offset,
                sizeof(offset)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x82u, &segment,
                sizeof(segment)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x100u, &halt,
                sizeof(halt)) != LIB_STATUS_OK;
    }
    if (!failed) {
        test_pic_program_vector(&board->shared_pic_master, 0x20u);
        core_machine_pic_irq_source_bind(&irq, &board->shared_pic_master,
            &board->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed = core_machine_run(machine,
            (core_machine_run_budget){repeated ? 4u : 2u, 0u},
            &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine,
                after.ss.base + (lib_u16)after.esp, &frame_ip,
                sizeof(frame_ip)) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine, 0x20010u,
                observed_left, sizeof(observed_left)) != LIB_STATUS_OK ||
            core_machine_debug_read_memory(machine, 0x30020u,
                observed_right, sizeof(observed_right)) != LIB_STATUS_OK;
        if (!failed)
            failed = after.eip != 0x101u ||
                frame_ip != (repeated ? 0u : 1u) ||
                after.esi != 0x11u || after.edi != 0x21u ||
                after.ecx != (repeated ? 0x11220002u : 0x11220003u) ||
                (after.eflags & CMPS_FLAGS) !=
                    (VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF) ||
                lib_memory_compare(observed_left, left, sizeof(left)) != 0 ||
                lib_memory_compare(observed_right, right, sizeof(right)) != 0 ||
                !(test_pic_read(&board->shared_pic_master, 0x0bu) &
                    VPIC_ISR_IRQ(0u)) ||
                (test_pic_read(&board->shared_pic_master, 0x0au) &
                    VPIC_IRR_IRQ(0u));
    }
    core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!cmps_protected_case(LIB_TRUE, LIB_FALSE) ||
        !cmps_protected_case(LIB_FALSE, LIB_FALSE) ||
        !cmps_protected_case(LIB_TRUE, LIB_TRUE) ||
        !cmps_protected_case(LIB_FALSE, LIB_TRUE) ||
        !cmps_irq_case(LIB_FALSE) || !cmps_irq_case(LIB_TRUE)) {
        printf("CMPS board fault/IRQ failed\n");
        return 1;
    }
    printf("M5:T539:S38:CMPS-BOARD:OK\n");
    return 0;
}
