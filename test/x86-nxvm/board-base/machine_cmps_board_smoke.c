#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "board_board_fixture.h"
#include "x86-nxvm/board-base/machine_board_state.h"
#include "pic_fixture.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#define CMPS_FLAGS (X86_CORE_CPU_DEBUG_EFLAGS_CF | X86_CORE_CPU_DEBUG_EFLAGS_PF | X86_CORE_CPU_DEBUG_EFLAGS_AF | \
    X86_CORE_CPU_DEBUG_EFLAGS_ZF | X86_CORE_CPU_DEBUG_EFLAGS_SF | X86_CORE_CPU_DEBUG_EFLAGS_OF)

static lib_i32 cmps_prepare_protected(x86_core_machine **out_machine,
    const lib_u8 *code, lib_u8 bytes, lib_u16 ds_limit, lib_u16 es_limit)
{
    const x86_nxvm_board_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
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
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP)
    };
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_result result = {0};

    *out_machine = LIB_NULL;
    gdt[16u] = (lib_u8)ds_limit;
    gdt[17u] = (lib_u8)(ds_limit >> 8u);
    gdt[24u] = (lib_u8)es_limit;
    gdt[25u] = (lib_u8)(es_limit >> 8u);
    if (x86_nxvm_board_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x100u, pointer,
            sizeof(pointer)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x300u, gdt,
            sizeof(gdt)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0u, bootstrap,
            sizeof(bootstrap)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x2000u, code, bytes) !=
            LIB_STATUS_OK ||
        x86_core_machine_run(machine, (x86_core_machine_run_budget){11u, 0u},
            &result) != LIB_STATUS_OK ||
        result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.executed != 11u) {
        x86_core_machine_destroy(machine);
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
    x86_core_machine *machine = LIB_NULL;
    x86_core_cpu_debug_register_patch patch = {0};
    x86_core_cpu_debug_cpu_snapshot after = {0};
    x86_core_cpu_diagnostic diagnostic = {0};
    x86_core_machine_run_result result = {0};
    lib_u8 left[] = {0x10u, 1u}, right[] = {0x10u, 1u};
    lib_u16 ds_limit = repeated && !source_fault ? 0xffffu :
        repeated ? 0x10u : 0x0fu;
    lib_u16 es_limit = repeated && source_fault ? 0xffffu :
        repeated ? 0x10u : 0x0fu;
    lib_i32 failed = !cmps_prepare_protected(&machine, code, bytes,
        ds_limit, es_limit);

    if (!failed) {
        patch.mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EAX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ECX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESI) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDI) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS);
        patch.values[X86_CORE_CPU_DEBUG_EAX] = 0xaabbcc10u;
        patch.values[X86_CORE_CPU_DEBUG_ECX] = 0x11220003u;
        patch.values[X86_CORE_CPU_DEBUG_ESI] =
            repeated || source_fault ? 0x10u : 0u;
        patch.values[X86_CORE_CPU_DEBUG_EDI] =
            repeated || !source_fault ? 0x10u : 0u;
        patch.values[X86_CORE_CPU_DEBUG_EFLAGS] = X86_CORE_CPU_DEBUG_EFLAGS_IF;
        failed = x86_core_machine_debug_patch_registers(machine, &patch) !=
            LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x3010u, left,
                sizeof(left)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x4010u, right,
                sizeof(right)) != LIB_STATUS_OK;
    }
    if (!failed)
        failed = x86_core_machine_run(machine,
            (x86_core_machine_run_budget){repeated ? 2u : 1u, 0u},
            &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            result.detail != VCPUINS_EXCEPT_SHUTDOWN ||
            x86_core_machine_get_cpu_diagnostic(machine, &diagnostic) != LIB_STATUS_OK ||
            x86_core_machine_debug_capture_cpu_snapshot(machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
    if (!failed)
        failed = diagnostic.first_fault.valid || !diagnostic.last_delivered_exception.valid ||
            !(diagnostic.last_delivered_exception.exception_mask & VCPUINS_EXCEPT_SHUTDOWN) ||
            after.eip != 0u || after.eax != 0xaabbcc10u ||
            after.ecx != (repeated ? 0x11220002u : 0x11220003u) ||
            after.esi != (repeated ? 0x11u : source_fault ? 0x10u : 0u) ||
            after.edi != (repeated ? 0x11u : source_fault ? 0u : 0x10u) ||
            after.eflags != (repeated ? X86_CORE_CPU_DEBUG_EFLAGS_IF | X86_CORE_CPU_DEBUG_EFLAGS_PF |
                X86_CORE_CPU_DEBUG_EFLAGS_ZF : X86_CORE_CPU_DEBUG_EFLAGS_IF);
    x86_core_machine_destroy(machine);
    return !failed;
}

static lib_i32 cmps_irq_case(lib_bool repeated)
{
    static const lib_u8 single[] = {0xa6u, 0x90u};
    static const lib_u8 rep[] = {0xf3u, 0xa6u, 0x90u};
    static const lib_u8 halt = 0xf4u;
    const x86_nxvm_board_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const lib_u8 *code = repeated ? rep : single;
    lib_u8 bytes = repeated ? sizeof(rep) : sizeof(single);
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    x86_nxvm_board_pic_irq_source *irq = LIB_NULL;
    x86_core_cpu_debug_register_patch patch = {0};
    x86_core_cpu_debug_cpu_snapshot after = {0};
    x86_core_machine_run_result result = {0};
    lib_u16 offset = 0x100u, segment = 0u, frame_ip = 0xffffu;
    lib_u8 left[] = {0x10u, 1u, 1u};
    lib_u8 right[] = {0x10u, 1u, 1u};
    lib_u8 observed_left[sizeof(left)] = {0};
    lib_u8 observed_right[sizeof(right)] = {0};
    lib_i32 failed = x86_nxvm_board_create(&config, &machine, &board) != LIB_STATUS_OK;

    if (!failed) {
        patch.mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EAX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ECX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESI) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDI);
        patch.values[X86_CORE_CPU_DEBUG_DS] = 0x2000u;
        patch.values[X86_CORE_CPU_DEBUG_ES] = 0x3000u;
        patch.values[X86_CORE_CPU_DEBUG_ESP] = 0x8000u;
        patch.values[X86_CORE_CPU_DEBUG_EFLAGS] = X86_CORE_CPU_DEBUG_EFLAGS_IF;
        patch.values[X86_CORE_CPU_DEBUG_EAX] = 0xaabbcc10u;
        patch.values[X86_CORE_CPU_DEBUG_ECX] = 0x11220003u;
        patch.values[X86_CORE_CPU_DEBUG_ESI] = 0x10u;
        patch.values[X86_CORE_CPU_DEBUG_EDI] = 0x20u;
        failed = x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            x86_core_machine_debug_patch_registers(machine, &patch) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x20010u, left,
                sizeof(left)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x30020u, right,
                sizeof(right)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0u, code, bytes) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x80u, &offset,
                sizeof(offset)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x82u, &segment,
                sizeof(segment)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x100u, &halt,
                sizeof(halt)) != LIB_STATUS_OK;
    }
    if (!failed) {
        test_pic_program_vector(board->shared_pic_master, 0x20u);
        test_pic_bind_source(&irq, board->shared_pic_master,
            board->shared_pic_slave, 0u);
        x86_nxvm_board_pic_irq_source_assert(irq);
        x86_nxvm_board_pic_irq_source_deassert(irq);
        failed = x86_core_machine_run(machine,
            (x86_core_machine_run_budget){repeated ? 4u : 2u, 0u},
            &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            x86_core_machine_debug_capture_cpu_snapshot(machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            x86_core_machine_debug_read_memory(machine,
                after.ss.base + (lib_u16)after.esp, &frame_ip,
                sizeof(frame_ip)) != LIB_STATUS_OK ||
            x86_core_machine_debug_read_memory(machine, 0x20010u,
                observed_left, sizeof(observed_left)) != LIB_STATUS_OK ||
            x86_core_machine_debug_read_memory(machine, 0x30020u,
                observed_right, sizeof(observed_right)) != LIB_STATUS_OK;
        if (!failed)
            failed = after.eip != 0x101u ||
                frame_ip != (repeated ? 0u : 1u) ||
                after.esi != 0x11u || after.edi != 0x21u ||
                after.ecx != (repeated ? 0x11220002u : 0x11220003u) ||
                (after.eflags & CMPS_FLAGS) !=
                    (X86_CORE_CPU_DEBUG_EFLAGS_PF | X86_CORE_CPU_DEBUG_EFLAGS_ZF) ||
                lib_memory_compare(observed_left, left, sizeof(left)) != 0 ||
                lib_memory_compare(observed_right, right, sizeof(right)) != 0 ||
                !(test_pic_read(board->shared_pic_master, 0x0bu) &
                    VPIC_ISR_IRQ(0u)) ||
                (test_pic_read(board->shared_pic_master, 0x0au) &
                    VPIC_IRR_IRQ(0u));
    }
    x86_core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!cmps_protected_case(LIB_TRUE, LIB_FALSE) ||
        !cmps_protected_case(LIB_FALSE, LIB_FALSE) ||
        !cmps_protected_case(LIB_TRUE, LIB_TRUE) ||
        !cmps_protected_case(LIB_FALSE, LIB_TRUE) ||
        !cmps_irq_case(LIB_FALSE) || !cmps_irq_case(LIB_TRUE)) {
        lib_c_printf("CMPS board fault/IRQ failed\n");
        return 1;
    }
    lib_c_printf("CMPS-BOARD:OK\n");
    return 0;
}
