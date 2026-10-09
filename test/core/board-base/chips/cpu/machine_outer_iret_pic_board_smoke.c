#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "support/cpu_outer_return_fixture.h"
#include "../../pic_fixture.h"
#include "core/board-base/pic_bus_interface.h"
#include "../../composition/composition_fixture.h"
typedef struct outer_iret_pic_board {
    cpu_instruction_fixture cpu;
    core_machine *machine;
    core_machine_pic_bus *master;
    core_machine_pic_bus *slave;
} outer_iret_pic_board;

static lib_status outer_iret_pic_read(void *opaque, lib_u32 address,
    void *destination, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance, lib_bool observe_only,
    lib_bool reset_fetch)
{
    outer_iret_pic_board *const board = opaque;

    return cpu_instruction_read(&board->cpu, address, destination, bytes,
        provenance, observe_only, reset_fetch);
}

static lib_status outer_iret_pic_write(void *opaque, lib_u32 address,
    const void *source, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance)
{
    outer_iret_pic_board *const board = opaque;

    return cpu_instruction_write(&board->cpu, address, source, bytes, provenance);
}

static lib_bool outer_iret_pic_pending(void *opaque)
{
    outer_iret_pic_board *const board = opaque;

    return core_machine_pic_peek_interrupt(board->master, board->slave) != 0u;
}

static lib_status outer_iret_pic_acknowledge(void *opaque, lib_u8 *out_vector)
{
    outer_iret_pic_board *const board = opaque;
    const lib_u8 vector = core_machine_pic_get_interrupt(board->master,
        board->slave);

    if (out_vector == LIB_NULL || vector == 0u) return LIB_STATUS_INVALID_STATE;
    *out_vector = vector;
    return LIB_STATUS_OK;
}

static const core_machine_cpu_bus_provider outer_iret_pic_bus = {
    .read_memory = outer_iret_pic_read,
    .write_memory = outer_iret_pic_write,
    .interrupt_pending = outer_iret_pic_pending,
    .acknowledge_interrupt = outer_iret_pic_acknowledge
};

static lib_bool outer_iret_pic_prepare(outer_iret_pic_board *board)
{
    static const lib_u8 outer_iret[] = {0xcfu};
    static const lib_u8 user_nop[] = {0x90u};
    static const lib_u32 outer_frame[] = {
        0x00000010u,0x0000001bu,CORE_MACHINE_DEBUG_EFLAGS_IF | 0x02u,0x00001000u,0x00000023u
    };
    const lib_u8 vector = 0x20u;
    lib_u8 *const memory = board->cpu.memory;
    lib_u8 *const gate = memory + CPU_OUTER_IDT_BASE + (lib_u16)vector * 8u;

    lib_memory_set(board, 0, sizeof(*board));
    board->machine = test_core_port_owner_create();
    if (board->machine == LIB_NULL) return LIB_FALSE;
    if (core_machine_pic_initialize(&board->master, &board->slave, board->machine,
            CORE_MACHINE_PIC_TOPOLOGY_SINGLE) != LIB_STATUS_OK) return LIB_FALSE;
    cpu_instruction_prepare_with_bus(&board->cpu, CORE_MACHINE_CPU_PROFILE_80386,
        &outer_iret_pic_bus, board);
    cpu_outer_return_configure(&board->cpu);
    board->cpu.cpu.data.cs.seg.exec.defsize = LIB_TRUE;
    board->cpu.cpu.data.esp = 0x12348000u;
    cpu_outer_set_gate(gate, 0u, 0x0100u);
    lib_memory_copy(memory + CPU_OUTER_KERNEL_BASE, outer_iret, sizeof(outer_iret));
    lib_memory_copy(memory + CPU_OUTER_KERNEL_STACK_BASE + 0x8000u, outer_frame,
        sizeof(outer_frame));
    lib_memory_copy(memory + CPU_OUTER_USER_CODE_BASE + 0x10u, user_nop,
        sizeof(user_nop));
    test_pic_program_vector(board->master, vector);
    return LIB_TRUE;
}

int main(void)
{
    outer_iret_pic_board board;
    core_machine_pic_irq_source *source = LIB_NULL;
    t_cpu after = {0};
    lib_bool failed = !outer_iret_pic_prepare(&board);

    if (!failed) {
        failed = core_machine_pic_irq_source_bind(&source, board.master,
            board.slave, 0u) != LIB_STATUS_OK;
    }
    if (!failed) {
        core_machine_pic_irq_source_assert(source);
        core_machine_pic_irq_source_deassert(source);
        core_machine_cpu_execution_refresh(&board.cpu.execution);
        core_machine_cpu_execution_refresh(&board.cpu.execution);
        core_machine_cpu_execution_refresh(&board.cpu.execution);
        core_machine_cpu_execution_refresh(&board.cpu.execution);
        after = board.cpu.cpu;
        failed = board.cpu.fault.valid || after.data.cs.selector != 0x0008u ||
            after.data.ss.selector != 0x0010u || after.data.eip != 0x101u ||
            !after.data.flagHalt ||
            (test_pic_read(board.master, 0x0bu) & 1u) == 0u ||
            (test_pic_read(board.master, 0x0au) & 1u) != 0u;
    }
    if (failed) {
        lib_c_fprintf(lib_c_stderr, "outer PIC cs=%04x ss=%04x ip=%08x sp=%08x flags=%08x irr=%02x isr=%02x fault=%u\\n",
            after.data.cs.selector, after.data.ss.selector, after.data.eip,
            after.data.esp, after.data.eflags,
            board.master != LIB_NULL ? test_pic_read(board.master, 0x0au) : 0u,
            board.master != LIB_NULL ? test_pic_read(board.master, 0x0bu) : 0u,
            board.cpu.fault.valid);
        core_machine_pic_finalize(board.master, board.slave);
        test_core_port_owner_destroy(board.machine);
        return 1;
    }
    core_machine_pic_finalize(board.master, board.slave);
    test_core_port_owner_destroy(board.machine);
    lib_c_printf("%s\n", "OUTER-IRET:PIC-BOARD:OK");
    return 0;
}
