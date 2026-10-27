#ifndef TEST_PROTECTED_PIC_MACHINE_FIXTURE_H
#define TEST_PROTECTED_PIC_MACHINE_FIXTURE_H

#include "lib/types/types_interface.h"
#include "x86-core/machine/debug_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-base/machine_board_state.h"
#include "../../../pic_fixture.h"

#define CPU_PROTECTED_CODE_BASE 0x2000u
#define CPU_PROTECTED_DATA_BASE 0x3000u
#define CPU_PROTECTED_STACK_BASE 0x4000u

typedef struct protected_pic_board_fixture {
    x86_core_machine *machine;
    x86_nxvm_board_state *board;
} protected_pic_board_fixture;

/* Guest instructions establish hidden CPU state; one Machine owns both
 * execution and the board's PIC ports. No private CPU layout is borrowed. */
static inline lib_bool protected_pic_board_prepare(protected_pic_board_fixture *board,
    lib_bool data_layout)
{
    static const lib_u8 reset_jump[] = {0xeau,0,0,0,0};
    static const lib_u8 gdt_pointer[] = {0x1fu,0,0,3,0,0};
    static const lib_u8 idt_pointer[] = {7,1,0,6,0,0};
    static const lib_u8 gate[] = {0,1,8,0,0,0x86u,0,0};
    static const lib_u8 handler = 0xf4u;
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x9au,0,0
    };
    lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0,1,
        0x0fu,0x01u,0x1eu,0x20u,1,
        0xb8u,1,0,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0,0x8eu,0xd8u,0x8eu,0xc0u,
        0xb8u,0x10u,0,0x8eu,0xd0u,0xbcu,0,0x80u,
        0xeau,0,0,8,0
    };
    const x86_nxvm_board_config config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    x86_core_machine_run_result result = {0};
    x86_core_cpu_debug_cpu_snapshot snapshot = {0};

    lib_memory_set(board, 0, sizeof(*board));
    if (data_layout) {
        gdt[29] = 0x92u;
        bootstrap[24] = 0x18u;
    }
    if (x86_nxvm_board_create(&config, &board->machine, &board->board) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(board->machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(board->machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(board->machine, 0xffff0u, reset_jump,
            sizeof(reset_jump)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(board->machine, 0x100u, gdt_pointer,
            sizeof(gdt_pointer)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(board->machine, 0x120u, idt_pointer,
            sizeof(idt_pointer)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(board->machine, 0x300u, gdt,
            sizeof(gdt)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(board->machine, 0x700u, gate,
            sizeof(gate)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(board->machine, CPU_PROTECTED_CODE_BASE + 0x100u,
            &handler, sizeof(handler)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(board->machine, 0u, bootstrap,
            sizeof(bootstrap)) != LIB_STATUS_OK ||
        x86_core_machine_run(board->machine,
            (x86_core_machine_run_budget){12u,0u}, &result) != LIB_STATUS_OK ||
        result.reason != X86_CORE_MACHINE_STOP_BUDGET ||
        x86_core_machine_debug_capture_cpu_snapshot(board->machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &snapshot) != LIB_STATUS_OK ||
        snapshot.cs.selector != 8u || snapshot.cs.base != CPU_PROTECTED_CODE_BASE ||
        snapshot.ss.base != (data_layout ? CPU_PROTECTED_STACK_BASE : CPU_PROTECTED_DATA_BASE) ||
        snapshot.esp != 0x8000u || snapshot.eip != 0u ||
        x86_core_machine_debug_write_register(board->machine,
            X86_CORE_CPU_DEBUG_EFLAGS, 0x202u) != LIB_STATUS_OK) return LIB_FALSE;
    test_pic_program_vector(board->board->shared_pic_master, 0x20u);
    return LIB_TRUE;
}

static inline void protected_pic_board_finalize(protected_pic_board_fixture *board)
{
    x86_core_machine_destroy(board->machine);
}

static inline lib_bool protected_pic_board_raise(protected_pic_board_fixture *board)
{
    x86_nxvm_board_pic_irq_source *source = LIB_NULL;

    if (x86_nxvm_board_pic_irq_source_bind(&source, board->board->shared_pic_master,
            board->board->shared_pic_slave, 0u) != LIB_STATUS_OK) return LIB_FALSE;
    x86_nxvm_board_pic_irq_source_assert(source);
    return LIB_TRUE;
}

#endif
