#include "lib/types/types_interface.h"
#include <stdio.h>

#include "x86/core/debug_interface.h"
#include "x86/core/entry_plan_interface.h"
#include "app-nxvm/devices/machine_board_interface.h"

#define GDT_POINTER 0x0100u
#define GDT_BASE 0x0300u
#define IDT_BASE 0x0400u
#define TASK_B_BASE 0x0700u
#define KERNEL_BASE 0x2000u

typedef struct task_tss32_selector { lib_u16 selector, reserved; } task_tss32_selector;
typedef struct task_tss32_state {
    lib_u32 cr3, eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    task_tss32_selector es, cs, ss, ds, fs, gs, ldtr;
} task_tss32_state;
_Static_assert(sizeof(task_tss32_state) == 0x48u,
    "The 80386 TSS saved-state span is architecturally fixed.");

static lib_i32 write_bytes(core_machine *machine, lib_u32 physical,
    const void *bytes, lib_size byte_count)
{
    return core_machine_memory_write(machine, physical, bytes, byte_count) ==
        LIB_STATUS_OK;
}

static lib_i32 write_u32(core_machine *machine, lib_u32 physical, lib_u32 value)
{
    return write_bytes(machine, physical, &value, sizeof(value));
}

static lib_i32 prepare(core_machine **out_machine,
    core_machine_board_state **out_board)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_entry_plan plan = {
        .state = { .cs = 0u, .ds = 0u, .es = 0u, .ss = 0u,
            .ip = 0u, .sp = 0u, .edx = 0x00000300u, .eflags = 0u },
        .entry_physical = 0u,
        .entry_route = CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM
    };

    return out_machine != LIB_NULL && core_machine_create(&config, out_machine, out_board) ==
        LIB_STATUS_OK && core_machine_freeze_execution_providers(*out_machine) ==
        LIB_STATUS_OK && core_machine_reset(*out_machine) == LIB_STATUS_OK &&
        core_machine_apply_entry_plan(*out_machine, &plan) == LIB_STATUS_OK;
}

static lib_i32 install_pages(core_machine *machine, lib_bool fault_case)
{
    lib_u32 page;
    for (page = 0u; page < 12u; ++page) {
        if (fault_case && page == 7u) continue;
        if (!write_u32(machine, 0xa000u + page * 4u, page * 0x1000u | 0x003u))
            return 0;
    }
    return write_u32(machine, 0x1000u, 0xa003u);
}

static lib_i32 install(core_machine *machine, lib_bool fault_case,
    lib_bool pending_irq, lib_bool nested)
{
    static const lib_u8 gdt_pointer[] = { 0x47u,0u,0u,0x03u,0u,0u };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u, 0x0fu,0x01u,0x1eu,0x80u,0x01u,
        0xb0u,0x11u,0xe6u,0x20u, 0xb0u,0x20u,0xe6u,0x21u,
        0xb0u,0x04u,0xe6u,0x21u, 0xb0u,0x01u,0xe6u,0x21u,
        0xb0u,0xfdu,0xe6u,0x21u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x28u,0x00u,0x0fu,0x00u,0xd8u, 0xb8u,0x10u,0x00u,0x8eu,0xd0u,
        0x8eu,0xd8u,0x8eu,0xc0u,0x8eu,0xe0u,0x8eu,0xe8u,0xbcu,0x00u,0x80u,
        0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const lib_u8 source_success[] = {
        0x66u,0xb8u,0x00u,0x10u,0x00u,0x00u,0x0fu,0x22u,0xd8u,
        0x66u,0xb8u,0x01u,0x00u,0x00u,0x80u,0x0fu,0x22u,0xc0u,
        0x66u,0xb8u,0x11u,0x11u,0x11u,0x11u,0xeau,0u,0u,0x30u,0u
    };
    static const lib_u8 source_fault[] = {
        0x66u,0xb8u,0x00u,0x10u,0x00u,0x00u,0x0fu,0x22u,0xd8u,
        0x66u,0xb8u,0x01u,0x00u,0x00u,0x80u,0x0fu,0x22u,0xc0u,
        0xeau,0u,0u,0x30u,0u
    };
    static const lib_u8 source_irq_jmp[] = { 0xeau,0u,0u,0x30u,0u };
    static const lib_u8 source_irq_call[] = { 0x9au,0u,0u,0x30u,0u };
    static const lib_u8 target_code[] = { 0x66u,0xb8u,0x34u,0x12u,0u,0u,0xf4u };
    static const lib_u8 halt[] = { 0xf4u };
    static const lib_u8 idtr[] = { 0x77u,0u,0x00u,0x04u,0u,0u };
    const lib_u8 fault_gate[] = { 0x80u,0x01u,0x08u,0u,0u,0x86u,0u,0u };
    const lib_u8 irq_gate[] = { 0x80u,0x01u,0x08u,0u,0u,0x86u,0u,0u };
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0xff,0,0,0x06,0,0x89,0,0,
        0xff,0,0, fault_case ? 0x70u : 0x07u,0,0x89,0,0,
        0,0,0x30u,0,0x85u,0,0
    };
    const task_tss32_state target = {
        .cr3 = fault_case ? 0x1000u : 0x4000u, .eip = 0x100u,
        .eflags = pending_irq ? 0x202u : 0x2u, .eax = 0xa1a12222u, .ecx = 0xc1c13333u,
        .edx = 0xd1d14444u, .ebx = 0xb1b15555u, .esp = 0x8000u,
        .ebp = 0xe1e16666u, .esi = 0xf1f17777u, .edi = 0x81818888u,
        .es = {0x10u,0u}, .cs = {0x08u,0u}, .ss = {0x10u,0u},
        .ds = {0x10u,0u}, .fs = {0x10u,0u}, .gs = {0x10u,0u}
    };
    const lib_u8 *source = pending_irq ? (nested ? source_irq_call : source_irq_jmp) :
        (fault_case ? source_fault : source_success);
    const lib_size source_bytes = pending_irq ? (nested ? sizeof(source_irq_call) :
        sizeof(source_irq_jmp)) : (fault_case ? sizeof(source_fault) : sizeof(source_success));

    if (!write_bytes(machine, GDT_POINTER, gdt_pointer, sizeof(gdt_pointer)) ||
        !write_bytes(machine, GDT_BASE, gdt, sizeof(gdt)) ||
        !write_bytes(machine, 0x0600u, (const lib_u8[44]){0}, 44u) ||
        !write_bytes(machine, 0x0180u, idtr, sizeof(idtr)) ||
        !write_bytes(machine, 0u, bootstrap, sizeof(bootstrap)) ||
        !write_bytes(machine, KERNEL_BASE, source, source_bytes) ||
        !write_bytes(machine, (fault_case ? 0x7000u : TASK_B_BASE) + 0x1cu,
        &target, sizeof(target)) || !install_pages(machine, fault_case)) return 0;
    if (fault_case) return write_bytes(machine, IDT_BASE + 14u * 8u, fault_gate,
        sizeof(fault_gate)) && write_bytes(machine, KERNEL_BASE + 0x180u,
        halt, sizeof(halt));
    if (pending_irq) return write_bytes(machine, IDT_BASE + 0x21u * 8u, irq_gate,
        sizeof(irq_gate)) && write_bytes(machine, KERNEL_BASE + 0x180u,
        halt, sizeof(halt));
    return write_u32(machine, 0x4000u, 0xc003u) &&
        write_u32(machine, 0xc000u + 2u * 4u, 0xb003u) &&
        write_bytes(machine, 0xb100u, target_code, sizeof(target_code));
}

static lib_i32 run_until_waiting_for_interrupt(core_machine *machine,
    core_machine_run_result *out_result)
{
    lib_u32 round;

    for (round = 0u; round < 2u; ++round) {
        if (core_machine_run(machine, (core_machine_run_budget){128u,0u},
                out_result) != LIB_STATUS_OK) return 0;
        if (out_result->reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT)
            return 1;
        if (out_result->reason != CORE_MACHINE_STOP_BUDGET) return 0;
    }
    return 0;
}

static lib_i32 run_case(lib_bool fault_case, lib_bool pending_irq, lib_bool nested)
{
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot cpu = {0};
    lib_i32 failed = !prepare(&machine, &board);

    if (!failed) failed |= !install(machine, fault_case, pending_irq, nested);
    if (!failed && pending_irq) failed |= core_machine_keyboard_receive_native_byte(
        board, 0x1eu) != LIB_STATUS_OK;
    if (!failed) {
        failed |= !run_until_waiting_for_interrupt(machine, &result) ||
            core_machine_get_cpu_diagnostic(machine, &diagnostic) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK;
        if (fault_case) failed |= diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            diagnostic.last_delivered_exception.exception_mask != VCPUINS_EXCEPT_PF ||
            cpu.eip != 0x181u || cpu.tr.selector != 0x28u || cpu.cr3 != 0x1000u;
        else if (pending_irq) failed |= diagnostic.first_fault.valid ||
            diagnostic.last_delivered_exception.valid || cpu.tr.selector != 0x30u ||
            cpu.eip != 0x181u || cpu.eax != 0xa1a12222u;
        else failed |= diagnostic.first_fault.valid || cpu.tr.selector != 0x30u ||
            cpu.eax != 0x00001234u || cpu.cr3 != 0x4000u;
    }
    core_machine_destroy(machine);
    return failed;
}

int main(void)
{
    if (run_case(LIB_FALSE, LIB_FALSE, LIB_FALSE) ||
        run_case(LIB_TRUE, LIB_FALSE, LIB_FALSE) ||
        run_case(LIB_FALSE, LIB_TRUE, LIB_FALSE) ||
        run_case(LIB_FALSE, LIB_TRUE, LIB_TRUE)) return 1;
    puts("M5:T539:S59:TASK32-PAGING:OK");
    puts("M5:T539:S61:TASK32-PENDING-IRQ:OK");
    return 0;
}
