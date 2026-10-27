#include "lib/types/file.h"
#include "pic_fixture.h"
#include "x86-nxvm/board-base/machine_board_state.h"
#include "lib/types/types_interface.h"

#include "x86-core/cpu/cpu_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"
#include "composition/debug_fixture.h"
#include "board_board_fixture.h"

/* Retained owners supply the detailed gate and VM86 frame matrices. */
#define main hardware_delivery_interrupt_entry_main
#include "machine_interrupt_entry_smoke.c"
#undef main
#define main hardware_delivery_vm86_delivery_main
#include "machine_vm86_delivery_smoke.c"
#undef main

typedef struct hardware_delivery_real_machine {
    x86_core_machine *machine;
    x86_nxvm_board_state *board;
} hardware_delivery_real_machine;

static lib_i32 hardware_delivery_real_priority(void)
{
    const x86_nxvm_board_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    static const lib_u8 program[] = { 0x90u };
    static const lib_u8 handler[] = { 0xf4u };
    static const lib_u8 nmi_vector[] = { 0x00u, 0x01u, 0x00u, 0x00u };
    static const lib_u8 irq_vector[] = { 0x20u, 0x01u, 0x00u, 0x00u };
    hardware_delivery_real_machine state;
    x86_nxvm_board_pic_irq_source *irq = LIB_NULL;
    x86_core_machine_run_result result;
    lib_u16 frame[3u] = { 0u, 0u, 0u };
    lib_status status;
    lib_i32 failed = 0;

    lib_memory_set(&state, 0, sizeof(state));
    lib_memory_set(&irq, 0, sizeof(irq));
    if (!test_x86_nxvm_machine_fixture_create_bind_freeze_reset(&config,
            LIB_NULL, LIB_NULL, &state.machine, &state.board)) {
        x86_core_machine_destroy(state.machine);
        return 0;
    }
    const x86_core_cpu_debug_register_patch real_entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {0}
    };
    failed |= x86_core_machine_debug_patch_registers(state.machine, &real_entry) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(state.machine, 0u, program,
            sizeof(program)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(state.machine, 8u, nmi_vector,
            sizeof(nmi_vector)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(state.machine, 0x80u, irq_vector,
            sizeof(irq_vector)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(state.machine, 0x0100u, handler,
            sizeof(handler)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(state.machine, 0x0120u, handler,
            sizeof(handler)) != LIB_STATUS_OK;
    if (!failed) {
        test_x86_nxvm_machine_fixture_write_register(state.machine, X86_CORE_CPU_DEBUG_ESP, 0x8000u);
        test_x86_nxvm_machine_fixture_write_register(state.machine, X86_CORE_CPU_DEBUG_EFLAGS,
            X86_CORE_CPU_DEBUG_EFLAGS_CF | X86_CORE_CPU_DEBUG_EFLAGS_IF);
        failed |= !x86_core_machine_signal_nmi(state.machine);
    }
    if (!failed) {
        test_pic_program_vector(state.board->shared_pic_master, 0x20u);
        test_pic_bind_source(&irq, state.board->shared_pic_master,
            state.board->shared_pic_slave, 0u);
        x86_nxvm_board_pic_irq_source_assert(irq);
        status = x86_core_machine_run(state.machine, (x86_core_machine_run_budget){ 8u, 0u },
            &result);
        failed |= status != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            vm86_capture(state.machine).eip != 0x0101u ||
            vm86_capture(state.machine).esp != 0x00007ffau ||
            (((vm86_capture(state.machine).eflags) & (X86_CORE_CPU_DEBUG_EFLAGS_IF)) != 0u) ||
            (((vm86_capture(state.machine).eflags) & (X86_CORE_CPU_DEBUG_EFLAGS_TF)) != 0u) ||
            !(((test_pic_read(state.board->shared_pic_master, 0x0au)) & (VPIC_IRR_IRQ(0u))) != 0u) ||
            (((test_pic_read(state.board->shared_pic_master, 0x0bu)) & (VPIC_ISR_IRQ(0u))) != 0u) ||
            x86_core_machine_memory_read(state.machine,
                0x00007ffau, (void *)frame, sizeof(frame)) !=
                LIB_STATUS_OK ||
            frame[0] != 1u || frame[1] != 0u ||
            frame[2] != (X86_CORE_CPU_DEBUG_EFLAGS_CF | X86_CORE_CPU_DEBUG_EFLAGS_IF | 0x02u);
        x86_nxvm_board_pic_irq_source_deassert(irq);
    }
    x86_core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 hardware_delivery_protected_priority(void)
{
    interrupt_entry_machine state;
    x86_nxvm_board_pic_irq_source *irq = LIB_NULL;
    x86_core_cpu_diagnostic diagnostic;
    x86_core_cpu_debug_cpu_snapshot after;
    lib_u32 frame[3u] = { 0u, 0u, 0u };
    static const lib_u8 program[] = { 0x90u };
    lib_i32 failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
        IE_INTGATE_32);

    lib_memory_set(&irq, 0, sizeof(irq));
    if (!failed) {
        test_x86_nxvm_machine_fixture_write_register(state.machine, X86_CORE_CPU_DEBUG_EFLAGS,
            X86_CORE_CPU_DEBUG_EFLAGS_CF | X86_CORE_CPU_DEBUG_EFLAGS_IF);
        failed |= !x86_core_machine_signal_nmi(state.machine);
    }
    if (!failed) {
        test_pic_program_vector(state.board->shared_pic_master, IE_VECTOR);
        test_pic_bind_source(&irq, state.board->shared_pic_master,
            state.board->shared_pic_slave, 0u);
        x86_nxvm_board_pic_irq_source_assert(irq);
        failed |= !ie_install_gate(&state, 0x02u, 0x0008u,
                (lib_u8)(0x80u | IE_INTGATE_32)) ||
            !ie_write(&state, IE_CODE_BASE, program, sizeof(program)) ||
            !ie_run(&state, 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.eip != IE_HANDLER_OFFSET + 1u ||
            after.esp != IE_STACK_BASE - 12u ||
            (((after.eflags) & (X86_CORE_CPU_DEBUG_EFLAGS_IF)) != 0u) ||
            (((after.eflags) & (X86_CORE_CPU_DEBUG_EFLAGS_TF)) != 0u) ||
            !(((test_pic_read(state.board->shared_pic_master, 0x0au)) & (VPIC_IRR_IRQ(0u))) != 0u) ||
            (((test_pic_read(state.board->shared_pic_master, 0x0bu)) & (VPIC_ISR_IRQ(0u))) != 0u) ||
            !ie_read(&state, IE_STACK_BASE - 12u, frame, sizeof(frame)) ||
            frame[0] != 1u || frame[1] != 0x0008u ||
            frame[2] != (X86_CORE_CPU_DEBUG_EFLAGS_CF | X86_CORE_CPU_DEBUG_EFLAGS_IF);
        x86_nxvm_board_pic_irq_source_deassert(irq);
    }
    x86_core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 hardware_delivery_vm86_install_gate(
    vm86_delivery_state *state, lib_u8 vector)
{
    lib_u8 gate[8u] = { 0u };

    gate[0] = 0u;
    gate[1] = 0x01u;
    gate[2] = 0x08u;
    gate[5] = 0x8eu;
    return x86_core_machine_memory_write(state->machine,
        VM86_IDT_BASE + (lib_u32)vector * 8u, gate,
        sizeof(gate)) == LIB_STATUS_OK;
}

static lib_i32 hardware_delivery_vm86_pic_matches(
    const vm86_delivery_state *state, lib_i32 nmi_masked)
{
    lib_u8 irq_pending = (((test_pic_read(state->board->shared_pic_master, 0x0au)) & (VPIC_IRR_IRQ(0u))) != 0u);
    lib_u8 irq_active = (((test_pic_read(state->board->shared_pic_master, 0x0bu)) & (VPIC_ISR_IRQ(0u))) != 0u);

    if (nmi_masked) {
        return !irq_pending && irq_active;
    }
    return irq_pending && !irq_active;
}

static lib_i32 hardware_delivery_vm86_priority(lib_i32 mask_nmi)
{
    vm86_delivery_state state;
    x86_nxvm_board_pic_irq_source *irq = LIB_NULL;
    x86_core_machine_run_result result;
    lib_u32 frame[9u] = { 0u };
    lib_status status;
    lib_i32 failed = !vm86_delivery_prepare(&state, mask_nmi ? 0x20u : 0x02u);

    lib_memory_set(&irq, 0, sizeof(irq));
    if (!failed) {
        failed |= !x86_core_machine_signal_nmi(state.machine) ||
            x86_core_machine_set_nmi_mask(state.machine, mask_nmi) != LIB_STATUS_OK;
    }
    if (!failed) {
        test_pic_program_vector(state.board->shared_pic_master, 0x20u);
        test_pic_bind_source(&irq, state.board->shared_pic_master,
            state.board->shared_pic_slave, 0u);
        x86_nxvm_board_pic_irq_source_assert(irq);
        failed |= !hardware_delivery_vm86_install_gate(&state, 0x20u) ||
            x86_core_machine_memory_write(state.machine, 0x2000u,
                (const lib_u8[]){ 0x90u }, 1u) != LIB_STATUS_OK;
        status = failed ? LIB_STATUS_INTERNAL_ERROR : x86_core_machine_run(state.machine,
            (x86_core_machine_run_budget){ 8u, 0u }, &result);
        failed |= status != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            vm86_capture(state.machine).eip != 0x0101u ||
            vm86_capture(state.machine).esp != VM86_STACK_TOP - 36u ||
            (((vm86_capture(state.machine).eflags) & (X86_CORE_CPU_DEBUG_EFLAGS_VM)) != 0u) ||
            (((vm86_capture(state.machine).eflags) & (X86_CORE_CPU_DEBUG_EFLAGS_IF)) != 0u) ||
            (((vm86_capture(state.machine).eflags) & (X86_CORE_CPU_DEBUG_EFLAGS_TF)) != 0u) ||
            !hardware_delivery_vm86_pic_matches(&state, mask_nmi) ||
            x86_core_machine_memory_read(state.machine,
                VM86_STACK_TOP - 36u, (void *)frame,
                sizeof(frame)) != LIB_STATUS_OK || frame[0] != 1u ||
            frame[1] != 0x0200u || frame[2] != (X86_CORE_CPU_DEBUG_EFLAGS_VM |
                X86_CORE_CPU_DEBUG_EFLAGS_IF);
        x86_nxvm_board_pic_irq_source_deassert(irq);
    }
    x86_core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!hardware_delivery_real_priority() ||
        !hardware_delivery_protected_priority() ||
        !hardware_delivery_vm86_priority(0) ||
        !hardware_delivery_vm86_priority(1)) {
        return 1;
    }
    lib_c_printf("HARDWARE-DELIVERY:OK\n");
    return 0;
}
