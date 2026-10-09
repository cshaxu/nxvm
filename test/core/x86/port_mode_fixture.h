#ifndef TEST_X86_CORE_PORT_MODE_FIXTURE_H
#define TEST_X86_CORE_PORT_MODE_FIXTURE_H

#include "core/x86/machine_interface.h"
#include "core/x86/debug_interface.h"

/* Privilege state comes from guest LGDT/LMSW/LTR/IRETD, not borrowed caches.
 * Setup time remains on the one timeline; the row measures only its own cost. */
static inline lib_i32 test_core_80386_enter_port_mode(core_machine *machine,
    lib_i32 mode, const lib_u8 *program, lib_size bytes, lib_u8 bitmap,
    lib_u64 *out_setup_ticks)
{
    static const lib_u8 gdt[] = {
        0u,0u,0u,0u,0u,0u,0u,0u,
        0xffu,0xffu,0u,0u,0u,0x9au,0u,0u,
        0xffu,0xffu,0u,0u,0u,0x92u,0u,0u,
        0xffu,0xffu,0u,0u,0u,0xfau,0u,0u,
        0xffu,0xffu,0u,0u,0u,0xf2u,0u,0u,
        0xffu,0u,0u,0x06u,0u,0x89u,0u,0u
    };
    static const lib_u8 pointer[] = {0x2fu,0u,0u,0x03u,0u,0u};
    lib_u8 setup[] = {
        0x0fu,0x01u,0x16u,0u,0x01u, /* lgdt [100h] */
        0xb8u,0x01u,0u,0x0fu,0x01u,0xf0u, /* lmsw ax */
        0xb8u,0x10u,0u,0x8eu,0xd8u,0x8eu,0xc0u,0x8eu,0xd0u,
        0xeau,0u,0u,0x08u,0u
    };
    static const lib_u8 user_setup[] = {
        0xb8u,0x28u,0u,0x0fu,0u,0xd8u, /* ltr ax */
        0xb8u,0x23u,0u,0x8eu,0xd8u,0x8eu,0xc0u,
        0xbcu,0u,0x08u,0x66u,0xcfu /* iretd */
    };
    const lib_u16 iomap_base = 0x0080u;
    const lib_u32 frame[] = {
        0u, mode == 2 ? 0u : 0x1bu,
        mode == 2 ? CORE_MACHINE_DEBUG_EFLAGS_VM | 0x3000u | 2u : 2u,
        0x7000u, mode == 2 ? 0u : 0x23u, 0u,0u,0u,0u
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = {[CORE_MACHINE_DEBUG_EIP] = 0x0200u,
            [CORE_MACHINE_DEBUG_EFLAGS] = 2u}
    };
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot snapshot = {0};

    if (mode != 0) setup[sizeof(setup) - 3u] = 0x04u;
    if (core_machine_memory_write(machine, 0u, program, bytes) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0300u, gdt, sizeof(gdt)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0100u, pointer, sizeof(pointer)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0200u, setup, sizeof(setup)) != LIB_STATUS_OK ||
        core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        core_machine_run(machine, (core_machine_run_budget){8u,0u}, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 8u) return 0;
    if (mode != 0 &&
        (core_machine_memory_write(machine, 0x0400u, user_setup, sizeof(user_setup)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0800u, frame, sizeof(frame)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0600u + 0x66u,
            &iomap_base, sizeof(iomap_base)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x0600u + iomap_base + 0x1cu,
            &bitmap, sizeof(bitmap)) != LIB_STATUS_OK ||
        core_machine_run(machine, (core_machine_run_budget){7u,0u}, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 7u)) return 0;
    if (core_machine_debug_capture_cpu_snapshot(machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT,
            &snapshot) != LIB_STATUS_OK || (snapshot.cr0 & VCPU_CR0_PE) == 0u ||
        snapshot.eip != 0u || snapshot.cs.base != 0u || snapshot.ds.base != 0u ||
        snapshot.es.base != 0u || snapshot.ss.base != 0u ||
        snapshot.cs.selector != (mode == 0 ? 8u : mode == 1 ? 0x1bu : 0u) ||
        snapshot.cs.dpl != (mode == 0 ? 0u : 3u) ||
        ((snapshot.eflags & CORE_MACHINE_DEBUG_EFLAGS_VM) != 0u) != (mode == 2) ||
        (mode != 0 && (snapshot.tr.type != 0x0bu ||
            snapshot.tr.selector != 0x28u ||
            snapshot.tr.base != 0x0600u || snapshot.tr.limit != 0xffu)))
        return 0;
    *out_setup_ticks = result.elapsed_ticks;
    return 1;
}

#endif
