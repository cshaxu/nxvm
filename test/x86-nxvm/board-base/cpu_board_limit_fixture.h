#ifndef TEST_CPU_BOARD_LIMIT_FIXTURE_H
#define TEST_CPU_BOARD_LIMIT_FIXTURE_H

#include "board_board_fixture.h"
#include "x86-nxvm/board-base/machine_board_interface.h"

/* Board-owned descriptor loading and fault delivery, not a CPU cache edit. */
static lib_i32 test_cpu_board_limit_prepare_exact(x86_core_machine **out_machine,
    const lib_u8 *code, lib_size bytes, lib_bool writable, lib_u16 ds_limit)
{
    const x86_nxvm_board_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP)
    };
    static const lib_u8 pointer[] = { 0x1fu,0,0,0x03u,0,0 };
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0x0fu,0,0,0x30u,0,0x92u,0,0,0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0xb8u,0x18u,0x00u,
        0x8eu,0xd0u,0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    x86_core_machine_run_result result = {0};
    x86_core_machine *machine = LIB_NULL;

    if (out_machine == LIB_NULL || code == LIB_NULL || bytes == 0u) return 0;
    *out_machine = LIB_NULL;
    gdt[16u] = (lib_u8)ds_limit;
    gdt[17u] = (lib_u8)(ds_limit >> 8u);
    gdt[21u] = writable ? 0x92u : 0x90u;
    if (x86_nxvm_board_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x0100u, pointer,
            sizeof(pointer)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x0300u, gdt,
            sizeof(gdt)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0u, bootstrap,
            sizeof(bootstrap)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x2000u, code, bytes) !=
            LIB_STATUS_OK ||
        x86_core_machine_run(machine, (x86_core_machine_run_budget){ 10u, 0u },
            &result) != LIB_STATUS_OK ||
        result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.executed != 10u) {
        x86_core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

static inline lib_i32 test_cpu_board_limit_prepare(x86_core_machine **out_machine,
    const lib_u8 *code, lib_size bytes, lib_bool writable,
    lib_bool out_of_limit)
{
    return test_cpu_board_limit_prepare_exact(out_machine, code, bytes,
        writable, out_of_limit ? 0x000fu : 0xffffu);
}

#endif
