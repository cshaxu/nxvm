#include "lib/types/file.h"
#include "support/cpu_task_switch16_fixture.h"
#include "../../board-common/pic_fixture.h"
#include "ibmpc/board-common/pic_bus_interface.h"
#include "../../core/composition_fixture.h"
typedef struct task16_pic_board {
    cpu_instruction_fixture cpu;
    core_machine *machine;
    core_machine_pic_bus *master;
    core_machine_pic_bus *slave;
} task16_pic_board;

static lib_status task16_pic_read(void *opaque, lib_u32 address, void *to,
    lib_u8 bytes, core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe, lib_bool reset_fetch)
{
    task16_pic_board *const board = opaque;
    return cpu_instruction_read(&board->cpu, address, to, bytes, provenance,
        observe, reset_fetch);
}

static lib_status task16_pic_write(void *opaque, lib_u32 address,
    const void *from, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance)
{
    task16_pic_board *const board = opaque;
    return cpu_instruction_write(&board->cpu, address, from, bytes, provenance);
}

static lib_bool task16_pic_pending(void *opaque)
{
    task16_pic_board *const board = opaque;
    return core_machine_pic_peek_interrupt(board->master, board->slave) != 0u;
}

static lib_status task16_pic_acknowledge(void *opaque, lib_u8 *vector)
{
    task16_pic_board *const board = opaque;
    const lib_u8 value = core_machine_pic_get_interrupt(board->master,
        board->slave);
    if (vector == LIB_NULL || value == 0u) return LIB_STATUS_INVALID_STATE;
    *vector = value;
    return LIB_STATUS_OK;
}

static const core_machine_cpu_bus_provider task16_pic_bus = {
    .read_memory = task16_pic_read, .write_memory = task16_pic_write,
    .interrupt_pending = task16_pic_pending, .acknowledge_interrupt = task16_pic_acknowledge
};

int main(void)
{
    task16_pic_board board = {0};
    core_machine_pic_irq_source *irq = LIB_NULL;
    t_cpu after = {0};
    board.machine = test_core_port_owner_create();
    if (board.machine == LIB_NULL) return 1;
    lib_bool failed = core_machine_pic_initialize(&board.master, &board.slave,
        board.machine, CORE_MACHINE_PIC_TOPOLOGY_SINGLE) != LIB_STATUS_OK;
    if (!failed) {
        cpu_instruction_prepare_with_bus(&board.cpu, CORE_MACHINE_CPU_PROFILE_80386,
            &task16_pic_bus, &board);
        cpu_task16_configure(&board.cpu, CPU_TASK16_DIRECT);
        board.cpu.memory[CPU_TASK16_B_BASE + 16u] = 0x02u;
        board.cpu.memory[CPU_TASK16_B_BASE + 17u] = 0x02u;
        cpu_task16_set_gate(board.cpu.memory + CPU_TASK16_IDT_BASE, 0x20u, 0x0180u);
        test_pic_program_vector(board.master, 0x20u);
        failed = core_machine_pic_irq_source_bind(&irq, board.master,
            board.slave, 0u) != LIB_STATUS_OK;
    }
    if (!failed) {
        core_machine_pic_irq_source_assert(irq);
        core_machine_pic_irq_source_deassert(irq);
        cpu_task16_refresh(&board.cpu, 4u);
        after = board.cpu.cpu;
        failed = board.cpu.fault.valid || !after.data.flagHalt ||
            after.data.cs.selector != 0x08u || after.data.eip != 0x0181u ||
            (test_pic_read(board.master, 0x0bu) & 1u) == 0u ||
            (test_pic_read(board.master, 0x0au) & 1u) != 0u;
    }
    core_machine_pic_finalize(board.master, board.slave);
    test_core_port_owner_destroy(board.machine);
    if (failed) { lib_c_fprintf(lib_c_stderr, "%s", "TASK16-PIC:FAIL\n"); return 1; }
    lib_c_printf("%s\n", "TASK16-PIC:OK");
    return 0;
}
