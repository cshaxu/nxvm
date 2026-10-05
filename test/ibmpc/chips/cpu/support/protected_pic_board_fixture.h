#include "lib/types/types_interface.h"
#ifndef TEST_PROTECTED_PIC_BOARD_FIXTURE_H
#define TEST_PROTECTED_PIC_BOARD_FIXTURE_H

#include "cpu_protected_fixture.h"
#include "../../../board-common/pic_fixture.h"
#include "x86/core/device_support_interface.h"
#include "ibmpc/board-common/pic_bus_interface.h"
#include "../../../core/composition_fixture.h"

typedef struct protected_pic_board_fixture {
    cpu_instruction_fixture cpu;
    core_machine *machine;
    core_machine_pic_bus *master;
    core_machine_pic_bus *slave;
} protected_pic_board_fixture;

static inline lib_status protected_pic_board_read(void *opaque, lib_u32 address,
    void *destination, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance, lib_bool observe_only,
    lib_bool reset_fetch)
{
    protected_pic_board_fixture *board = opaque;
    return cpu_instruction_read(&board->cpu, address, destination, bytes,
        provenance, observe_only, reset_fetch);
}

static inline lib_status protected_pic_board_write(void *opaque, lib_u32 address,
    const void *source, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance)
{
    protected_pic_board_fixture *board = opaque;
    return cpu_instruction_write(&board->cpu, address, source, bytes, provenance);
}

static inline lib_bool protected_pic_board_pending(void *opaque)
{
    protected_pic_board_fixture *board = opaque;
    return core_machine_pic_peek_interrupt(board->master, board->slave) != 0u;
}

static inline lib_status protected_pic_board_acknowledge(void *opaque,
    lib_u8 *out_vector)
{
    protected_pic_board_fixture *board = opaque;
    lib_u8 vector = core_machine_pic_get_interrupt(board->master, board->slave);

    if (out_vector == LIB_NULL || vector == 0u) return LIB_STATUS_INVALID_STATE;
    *out_vector = vector;
    return LIB_STATUS_OK;
}

static const core_machine_cpu_bus_provider protected_pic_board_bus = {
    .read_memory = protected_pic_board_read,
    .write_memory = protected_pic_board_write,
    .interrupt_pending = protected_pic_board_pending,
    .acknowledge_interrupt = protected_pic_board_acknowledge
};

static inline lib_bool protected_pic_board_prepare(protected_pic_board_fixture *board,
    lib_bool data_layout)
{
    static const lib_u8 gate[] = {0,1,8,0,0,0x86u,0,0};
    static const core_machine_instruction_timing timing = {.base_ticks = 1u};
    const lib_u8 handler[] = {0xf4u};

    lib_memory_set(board, 0, sizeof(*board));
    board->machine = test_core_port_owner_create();
    if (board->machine == LIB_NULL) return LIB_FALSE;
    if (core_machine_pic_initialize(&board->master, &board->slave, board->machine,
        CORE_MACHINE_PIC_TOPOLOGY_SINGLE) != LIB_STATUS_OK) return LIB_FALSE;
    cpu_instruction_prepare_with_bus(&board->cpu, CORE_MACHINE_CPU_PROFILE_80386,
        &protected_pic_board_bus, board);
    if (data_layout) cpu_protected_prepare_data(&board->cpu,
        CORE_MACHINE_CPU_PROFILE_80386);
    else cpu_protected_prepare_far(&board->cpu, CORE_MACHINE_CPU_PROFILE_80386);
    /* prepare_* replaces the bus, so rebind the board's IRQ-bearing bus. */
    core_machine_cpu_execution_context_initialize(&board->cpu.execution,
        &board->cpu.cpu, &board->cpu.instructions, &protected_pic_board_bus, board);
    core_machine_cpu_execution_context_bind_profiles(&board->cpu.execution,
        CORE_MACHINE_CPU_PROFILE_80386, X86_FPU_PROFILE_NONE, LIB_FALSE,
        &timing);
    core_machine_cpu_execution_context_bind_diagnostic_provider(&board->cpu.execution,
        &cpu_instruction_diagnostics, &board->cpu);
    board->cpu.cpu.data.idtr.flagValid = LIB_TRUE;
    board->cpu.cpu.data.idtr.sregtype = SREG_IDTR;
    board->cpu.cpu.data.idtr.base = 0x0600u;
    board->cpu.cpu.data.idtr.limit = 0x0107u;
    board->cpu.cpu.data.eflags |= CORE_MACHINE_DEBUG_EFLAGS_IF;
    lib_memory_copy(board->cpu.memory + 0x0700u, gate, sizeof(gate));
    lib_memory_copy(board->cpu.memory + CPU_PROTECTED_CODE_BASE + 0x100u,
        handler, sizeof(handler));
    test_pic_program_vector(board->master, 0x20u);
    return LIB_TRUE;
}

static inline void protected_pic_board_finalize(protected_pic_board_fixture *board)
{
    core_machine_pic_finalize(board->master, board->slave);
    test_core_port_owner_destroy(board->machine);
}

static inline lib_bool protected_pic_board_raise(protected_pic_board_fixture *board)
{
    core_machine_pic_irq_source *source = LIB_NULL;

    test_pic_bind_source(&source, board->master, board->slave, 0u);
    core_machine_pic_irq_source_assert(source);
    core_machine_pic_irq_source_deassert(source);
    return LIB_TRUE;
}

#endif
