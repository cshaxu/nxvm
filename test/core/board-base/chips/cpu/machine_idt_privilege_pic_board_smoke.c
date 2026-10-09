#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "support/cpu_instruction_fixture.h"
#include "../../pic_fixture.h"
#include "core/x86/device_support_interface.h"
#include "core/board-base/pic_bus_interface.h"
#include "../../composition/composition_fixture.h"
#define PIC_IDT_GDT_BASE 0x0300u
#define PIC_IDT_IDT_BASE 0x0400u
#define PIC_IDT_TSS_BASE 0x0600u
#define PIC_IDT_KERNEL_CODE_BASE 0x2000u
#define PIC_IDT_USER_CODE_BASE 0x3000u
#define PIC_IDT_HANDLER_OFFSET 0x0100u
#define PIC_IDT_VECTOR 0x30u

typedef struct idt_pic_board {
    cpu_instruction_fixture cpu;
    core_machine *machine;
    core_machine_pic_bus *master;
    core_machine_pic_bus *slave;
} idt_pic_board;

static lib_status idt_pic_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch)
{
    idt_pic_board *const board = opaque;
    return cpu_instruction_read(&board->cpu, address, destination, bytes,
        provenance, observe_only, reset_fetch);
}

static lib_status idt_pic_write(void *opaque, lib_u32 address,
    const void *source, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance)
{
    idt_pic_board *const board = opaque;
    return cpu_instruction_write(&board->cpu, address, source, bytes, provenance);
}

static lib_bool idt_pic_interrupt_pending(void *opaque)
{
    idt_pic_board *const board = opaque;
    return core_machine_pic_peek_interrupt(board->master, board->slave) != 0u;
}

static lib_status idt_pic_acknowledge(void *opaque, lib_u8 *out_vector)
{
    idt_pic_board *const board = opaque;
    lib_u8 vector = core_machine_pic_get_interrupt(board->master, board->slave);

    if (out_vector == LIB_NULL || vector == 0u) return LIB_STATUS_INVALID_STATE;
    *out_vector = vector;
    return LIB_STATUS_OK;
}

static const core_machine_cpu_bus_provider idt_pic_bus = {
    .read_memory = idt_pic_read,
    .write_memory = idt_pic_write,
    .interrupt_pending = idt_pic_interrupt_pending,
    .acknowledge_interrupt = idt_pic_acknowledge
};

static lib_bool idt_pic_prepare(idt_pic_board *board)
{
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0xcfu,0,
        0xffu,0xffu,0,0x30u,0,0xfau,0x40u,0,
        0xffu,0xffu,0,0,0,0xf2u,0xcfu,0,
        0x67u,0,0,0x06u,0,0x8bu,0,0
    };
    const lib_u8 user_code[] = {0x90u};
    const lib_u8 handler[] = {0xf4u};
    lib_u8 idt[PIC_IDT_VECTOR * 8u + 8u] = {0};
    lib_u8 tss[10] = {0};
    const lib_u32 esp0 = 0x00009000u;
    const lib_u16 ss0 = 0x0010u;
    t_cpu *cpu;

    if (board == LIB_NULL) return LIB_FALSE;
    lib_memory_set(board, 0, sizeof(*board));
    board->machine = test_core_port_owner_create();
    if (board->machine == LIB_NULL) return LIB_FALSE;
    if (core_machine_pic_initialize(&board->master, &board->slave, board->machine,
            CORE_MACHINE_PIC_TOPOLOGY_SINGLE) != LIB_STATUS_OK) return LIB_FALSE;
    cpu_instruction_prepare_with_bus(&board->cpu, CORE_MACHINE_CPU_PROFILE_80386,
        &idt_pic_bus, board);
    idt[0x30u * 8u] = PIC_IDT_HANDLER_OFFSET & 0xffu;
    idt[0x30u * 8u + 1u] = PIC_IDT_HANDLER_OFFSET >> 8u;
    idt[0x30u * 8u + 2u] = 0x08u;
    idt[0x30u * 8u + 5u] = 0x8eu;
    lib_memory_copy(tss + 4u, &esp0, sizeof(esp0));
    lib_memory_copy(tss + 8u, &ss0, sizeof(ss0));
    lib_memory_copy(board->cpu.memory + PIC_IDT_GDT_BASE, gdt, sizeof(gdt));
    lib_memory_copy(board->cpu.memory + PIC_IDT_IDT_BASE, idt, sizeof(idt));
    lib_memory_copy(board->cpu.memory + PIC_IDT_TSS_BASE, tss, sizeof(tss));
    lib_memory_copy(board->cpu.memory + PIC_IDT_USER_CODE_BASE, user_code,
        sizeof(user_code));
    lib_memory_copy(board->cpu.memory + PIC_IDT_KERNEL_CODE_BASE +
        PIC_IDT_HANDLER_OFFSET, handler, sizeof(handler));
    cpu = &board->cpu.cpu;
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.gdtr.flagValid = cpu->data.idtr.flagValid = LIB_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.gdtr.base = PIC_IDT_GDT_BASE;
    cpu->data.gdtr.limit = (lib_u16)(sizeof(gdt) - 1u);
    cpu->data.idtr.base = PIC_IDT_IDT_BASE;
    cpu->data.idtr.limit = (lib_u16)(sizeof(idt) - 1u);
    cpu->data.cs.flagValid = cpu->data.ss.flagValid = LIB_TRUE;
    cpu->data.cs.selector = 0x001bu;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = PIC_IDT_USER_CODE_BASE;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = 3u;
    cpu->data.cs.seg.executable = cpu->data.cs.seg.exec.defsize =
        cpu->data.cs.seg.exec.readable = LIB_TRUE;
    cpu->data.ss.selector = 0x0023u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = 0u;
    cpu->data.ss.limit = 0xffffffffu;
    cpu->data.ss.dpl = 3u;
    cpu->data.ss.seg.data.big = cpu->data.ss.seg.data.writable = LIB_TRUE;
    cpu->data.tr.flagValid = LIB_TRUE;
    cpu->data.tr.selector = 0x0028u;
    cpu->data.tr.sregtype = SREG_TR;
    cpu->data.tr.base = PIC_IDT_TSS_BASE;
    cpu->data.tr.limit = 0x67u;
    cpu->data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
    cpu->data.esp = 0x00008800u;
    cpu->data.eflags = 0x00000202u;
    test_pic_program_vector(board->master, PIC_IDT_VECTOR);
    return LIB_TRUE;
}

int main(void)
{
    idt_pic_board board;
    core_machine_pic_irq_source *source = LIB_NULL;
    t_cpu after;
    lib_i32 failed = !idt_pic_prepare(&board);

    if (!failed) {
        failed = core_machine_pic_irq_source_bind(&source, board.master,
            board.slave, 0u) != LIB_STATUS_OK;
    }
    if (!failed) {
        core_machine_pic_irq_source_assert(source);
        core_machine_pic_irq_source_deassert(source);
        core_machine_cpu_execution_refresh(&board.cpu.execution);
        core_machine_cpu_execution_refresh(&board.cpu.execution);
        after = board.cpu.cpu;
        failed = board.cpu.execution.stop_requested || board.cpu.fault.valid ||
            after.data.cs.selector != 0x0008u || after.data.cs.dpl != 0u ||
            after.data.ss.selector != 0x0010u || after.data.esp != 0x00008fecu ||
            CORE_MACHINE_BIT_IS_SET(after.data.eflags, CORE_MACHINE_DEBUG_EFLAGS_IF) ||
            !CORE_MACHINE_BIT_IS_SET(test_pic_read(board.master, 0x0bu), 1u) ||
            CORE_MACHINE_BIT_IS_SET(test_pic_read(board.master, 0x0au), 1u);
    }
    core_machine_pic_finalize(board.master, board.slave);
    test_core_port_owner_destroy(board.machine);
    if (failed) return 1;
    lib_c_printf("%s\n", "M5::IDT-PRIVILEGE-ENTRY:PIC:OK");
    return 0;
}
