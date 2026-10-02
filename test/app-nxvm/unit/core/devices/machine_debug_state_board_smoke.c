#include "support/pic_fixture.h"
#include "app-nxvm/devices/machine_board_state.h"
#include "support/core_machine_board_fixture.h"
#include "support/machine_cpu_fixture.h"
#include "x86/chips/cpu/cpu.h"
#include "app-nxvm/devices/device_support.h"
#include "app-nxvm/devices/machine_interface.h"
#include <stdio.h>

/* T337_REAL_UD_VECTOR6_DELIVERY: this owner installs and observes vector 6. */

typedef struct debug_board_context {
    core_machine *machine;
} debug_board_context;

#define DEBUG_BOARD_GDT_POINTER 0x0100u
#define DEBUG_BOARD_GDT 0x0300u
#define DEBUG_BOARD_IDT_POINTER 0x0120u
#define DEBUG_BOARD_IDT 0x0400u
#define DEBUG_BOARD_CODE 0x2000u

static void debug_board_reset(void *opaque)
{
    debug_board_context *context = (debug_board_context *)opaque;

    if (context != LIB_NULL)
        (void)test_core_machine_fixture_reset_real_mode(context->machine);
}

static const core_machine_execution_provider debug_board_provider = {
    debug_board_reset, LIB_NULL
};

static lib_i32 debug_board_create(core_machine_cpu_profile profile,
    debug_board_context *context, core_machine **out_machine)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile, .fpu_profile = X86_FPU_PROFILE_NONE
    };

    *out_machine = LIB_NULL;
    lib_memory_set(context, 0, sizeof(*context));
    return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &debug_board_provider, context, &context->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(context->machine,
            0u) && ((*out_machine = context->machine) != LIB_NULL);
}

static lib_i32 debug_board_patch(core_machine *machine, lib_u32 mask,
    const lib_u32 *values)
{
    core_machine_debug_register_patch patch = {.mask = mask};

    lib_memory_copy(patch.values, values, sizeof(patch.values));
    return core_machine_debug_patch_registers(machine, &patch) == LIB_STATUS_OK;
}

static lib_i32 debug_board_snapshot(core_machine *machine,
    core_machine_debug_cpu_snapshot *out)
{
    return core_machine_debug_capture_cpu_snapshot(machine,
        CORE_MACHINE_CPU_SNAPSHOT_CURRENT, out) == LIB_STATUS_OK;
}

static lib_i32 debug_board_install_real_vector(core_machine *machine,
    lib_u8 vector, lib_u16 offset)
{
    const lib_u8 handler = 0xf4u;
    const lib_u8 entry[] = {(lib_u8)offset, (lib_u8)(offset >> 8u), 0u, 0u};

    return core_machine_memory_write(machine, (lib_u32)vector * 4u, entry,
        sizeof(entry)) == LIB_STATUS_OK && core_machine_memory_write(machine,
        offset, &handler, sizeof(handler)) == LIB_STATUS_OK;
}

static lib_i32 debug_board_read_real_frame(core_machine *machine,
    const core_machine_debug_cpu_snapshot *after, lib_u16 *out)
{
    return core_machine_debug_read_memory(machine, after->ss.base +
        (lib_u16)after->esp, out, 3u * sizeof(*out)) == LIB_STATUS_OK;
}

static lib_i32 debug_board_run_at(core_machine *machine, lib_u32 address,
    const lib_u8 *code, lib_size bytes, core_machine_run_budget budget,
    core_machine_run_result *out_result,
    core_machine_debug_cpu_snapshot *out_after,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    return core_machine_memory_write(machine, address, code, bytes) == LIB_STATUS_OK &&
        core_machine_run(machine, budget, out_result) ==
            LIB_STATUS_OK && core_machine_get_cpu_diagnostic(machine,
            out_diagnostic) == LIB_STATUS_OK && debug_board_snapshot(machine,
            out_after);
}

static lib_i32 debug_board_run(core_machine *machine, const lib_u8 *code,
    lib_size bytes, core_machine_run_result *out_result,
    core_machine_debug_cpu_snapshot *out_after,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    return debug_board_run_at(machine, 0u, code, bytes,
        (core_machine_run_budget){32u,0u}, out_result, out_after,
        out_diagnostic);
}

static lib_i32 debug_board_boot_protected(core_machine *machine)
{
    static const lib_u8 gdt_pointer[] = {0x17u,0x00u,0x00u,0x03u,0u,0u};
    static const lib_u8 gdt[] = {
        0u,0u,0u,0u,0u,0u,0u,0u,
        0xffu,0xffu,0u,0x20u,0u,0x9au,0u,0u,
        0xffu,0xffu,0u,0u,0u,0x92u,0u,0u
    };
    static const lib_u8 code[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,
        0x8eu,0xd0u,0xbcu,0x00u,0x80u,
        0xeau,0x00u,0x00u,0x08u,0x00u
    };
    core_machine_run_result result = {0};
    lib_status status;
    lib_i32 ready;

    ready = core_machine_memory_write(machine, DEBUG_BOARD_GDT_POINTER,
        gdt_pointer, sizeof(gdt_pointer)) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, DEBUG_BOARD_GDT, gdt, sizeof(gdt)) ==
            LIB_STATUS_OK && core_machine_memory_write(machine, 0u, code,
            sizeof(code)) == LIB_STATUS_OK;
    status = ready ? core_machine_run(machine, (core_machine_run_budget){9u,0u},
        &result) : LIB_STATUS_INVALID_STATE;
    return ready && status == LIB_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET;
}

static lib_i32 debug_board_install_protected_vector(core_machine *machine)
{
    static const lib_u8 idt_pointer[] = {0x0fu,0x00u,0x00u,0x04u,0u,0u};
    static const lib_u8 lidt[] = {0x0fu,0x01u,0x1eu,0x20u,0x01u};
    static const lib_u8 handler = 0xf4u;
    const lib_u8 gate[] = {0x00u,0x01u,0x08u,0x00u,0u,0x8eu,0u,0u};
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    lib_i32 ready;

    ready = core_machine_memory_write(machine, DEBUG_BOARD_IDT_POINTER,
        idt_pointer, sizeof(idt_pointer)) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, DEBUG_BOARD_IDT + 8u, gate,
            sizeof(gate)) == LIB_STATUS_OK && core_machine_memory_write(machine,
            DEBUG_BOARD_CODE + 0x100u, &handler, sizeof(handler)) == LIB_STATUS_OK &&
        debug_board_run_at(machine, DEBUG_BOARD_CODE, lidt, sizeof(lidt),
            (core_machine_run_budget){1u,0u}, &result, &after, &diagnostic) &&
        result.reason == CORE_MACHINE_STOP_BUDGET && after.eip == sizeof(lidt);
    return ready;
}

static lib_i32 debug_board_test_real_delivery(void)
{
    static const lib_u8 mov_dr[] = {0x0fu,0x21u,0xc0u};
    static const lib_u8 nop[] = {0x90u};
    debug_board_context context;
    core_machine *machine = LIB_NULL;
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    lib_u16 frame[3u] = {0};
    const lib_u32 values[CORE_MACHINE_DEBUG_REGISTER_COUNT] = {
        [CORE_MACHINE_DEBUG_ESP] = 0x8000u,
        [CORE_MACHINE_DEBUG_EFLAGS] = VCPU_EFLAGS_IF | VCPU_EFLAGS_TF |
            VCPU_EFLAGS_CF
    };
    lib_i32 failed = !debug_board_create(CORE_MACHINE_CPU_PROFILE_80186,
        &context, &machine);

    if (!failed) failed = !debug_board_install_real_vector(machine, 6u, 0x0100u) ||
        !debug_board_install_real_vector(machine, 1u, 0x0110u) ||
        !debug_board_patch(machine,
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS), values) ||
        !debug_board_run(machine, mov_dr, sizeof(mov_dr), &result, &after,
            &diagnostic) || result.reason != CORE_MACHINE_STOP_BUDGET ||
        !diagnostic.last_delivered_exception.valid ||
        diagnostic.last_delivered_exception.exception_mask != VCPUINS_EXCEPT_UD ||
        after.eip != 0x0100u || !debug_board_read_real_frame(machine, &after,
            frame) || frame[0] != 0u || frame[1] != 0u;
    core_machine_destroy(machine);
    if (failed) return 0;

    failed = !debug_board_create(CORE_MACHINE_CPU_PROFILE_80286, &context,
        &machine);
    if (!failed) failed = !debug_board_install_real_vector(machine, 6u, 0x0100u) ||
        !debug_board_patch(machine,
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS), values) ||
        !debug_board_run(machine, mov_dr, sizeof(mov_dr), &result, &after,
            &diagnostic) || result.reason != CORE_MACHINE_STOP_BUDGET ||
        !diagnostic.last_delivered_exception.valid ||
        diagnostic.last_delivered_exception.exception_mask != VCPUINS_EXCEPT_UD ||
        after.eip != 0x0100u || !debug_board_read_real_frame(machine, &after,
            frame) || frame[0] != 0u || frame[1] != 0u;
    core_machine_destroy(machine);
    if (failed) return 0;

    failed = !debug_board_create(CORE_MACHINE_CPU_PROFILE_80386, &context,
        &machine);
    if (!failed) failed = !debug_board_install_real_vector(machine, 1u, 0x0100u) ||
        !debug_board_patch(machine,
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS), values) ||
        !debug_board_run(machine, nop, sizeof(nop), &result, &after,
            &diagnostic) || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        !diagnostic.last_delivered_exception.valid ||
        diagnostic.last_delivered_exception.exception_mask != VCPUINS_EXCEPT_DB ||
        after.eip != 0x0101u || !debug_board_read_real_frame(machine, &after,
            frame) || frame[0] != 1u || frame[1] != 0u;
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 debug_board_test_pic_order(void)
{
    static const lib_u8 mov_dr_nop[] = {0x0fu,0x23u,0xc1u,0x90u};
    debug_board_context context;
    core_machine *machine = LIB_NULL;
    core_machine_pic_irq_source source = {0};
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    const lib_u32 values[CORE_MACHINE_DEBUG_REGISTER_COUNT] = {
        [CORE_MACHINE_DEBUG_ECX] = 0x11223344u,
        [CORE_MACHINE_DEBUG_ESP] = 0x8000u,
        [CORE_MACHINE_DEBUG_EFLAGS] = VCPU_EFLAGS_IF
    };
    lib_i32 failed = !debug_board_create(CORE_MACHINE_CPU_PROFILE_80386,
        &context, &machine);

    if (!failed) failed = !debug_board_install_real_vector(machine, 0x20u,
        0x0100u) || !debug_board_patch(machine,
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS), values);
    if (!failed) {
        test_pic_program_vector(&machine->board->shared_pic_master, 0x20u);
        core_machine_pic_irq_source_bind(&source, &machine->board->shared_pic_master,
            &machine->board->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed = !debug_board_run(machine, mov_dr_nop, sizeof(mov_dr_nop),
            &result, &after, &diagnostic) || result.reason !=
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT || diagnostic.first_fault.valid ||
            after.eip != 0x0101u ||
            (test_pic_read(&machine->board->shared_pic_master, 0x0bu) & VPIC_ISR_IRQ(0u)) == 0u;
    }
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 debug_board_test_protected_trap(void)
{
    static const lib_u8 prefixes[][2] = {
        {0u,0u}, {0x66u,0u}, {0x67u,0u}, {0x66u,0x67u}
    };
    const lib_u32 values[CORE_MACHINE_DEBUG_REGISTER_COUNT] = {
        [CORE_MACHINE_DEBUG_EFLAGS] = VCPU_EFLAGS_TF | VCPU_EFLAGS_IF |
            VCPU_EFLAGS_CF
    };
    lib_size index;

    for (index = 0u; index != sizeof(prefixes) / sizeof(prefixes[0]); ++index) {
        debug_board_context context;
        core_machine *machine = LIB_NULL;
        core_machine_run_result result = {0};
        core_machine_debug_cpu_snapshot after = {0};
        core_machine_cpu_diagnostic diagnostic = {0};
        lib_u8 code[3u] = {0x90u,0u,0u};
        const lib_size bytes = prefixes[index][0] == 0u ? 1u :
            prefixes[index][1] == 0u ? 2u : 3u;
        lib_i32 failed = !debug_board_create(CORE_MACHINE_CPU_PROFILE_80386,
            &context, &machine);

        if (prefixes[index][0] != 0u) code[0] = prefixes[index][0];
        if (prefixes[index][1] != 0u) code[1] = prefixes[index][1];
        code[bytes - 1u] = 0x90u;
        if (!failed) failed = !debug_board_boot_protected(machine) ||
            !debug_board_install_protected_vector(machine) ||
            !debug_board_patch(machine,
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS), values) ||
            !debug_board_run_at(machine, DEBUG_BOARD_CODE + 5u, code, bytes,
                (core_machine_run_budget){32u,0u}, &result, &after, &diagnostic) ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            !diagnostic.last_delivered_exception.valid ||
            diagnostic.last_delivered_exception.exception_mask != VCPUINS_EXCEPT_DB ||
            after.eip != 0x0101u;
        core_machine_destroy(machine);
        if (failed) return 0;
    }
    return 1;
}

static lib_i32 debug_board_expect_real_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_size bytes)
{
    debug_board_context context;
    core_machine *machine = LIB_NULL;
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    lib_u16 frame[3u] = {0};
    const lib_u32 values[CORE_MACHINE_DEBUG_REGISTER_COUNT] = {
        [CORE_MACHINE_DEBUG_ESP] = 0x8000u,
        [CORE_MACHINE_DEBUG_EFLAGS] = VCPU_EFLAGS_TF | VCPU_EFLAGS_IF |
            VCPU_EFLAGS_CF
    };
    lib_i32 passed = debug_board_create(profile, &context, &machine) &&
        debug_board_install_real_vector(machine, 1u, 0x0110u) &&
        debug_board_install_real_vector(machine, 6u, 0x0100u) &&
        debug_board_patch(machine,
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS), values) &&
        debug_board_run(machine, code, bytes, &result, &after, &diagnostic) &&
        result.reason == CORE_MACHINE_STOP_BUDGET &&
        diagnostic.last_delivered_exception.valid &&
        diagnostic.last_delivered_exception.exception_mask == VCPUINS_EXCEPT_UD &&
        after.eip == 0x0100u && debug_board_read_real_frame(machine, &after,
            frame) && frame[0] == 0u && frame[1] == 0u;

    core_machine_destroy(machine);
    return passed;
}

static lib_i32 debug_board_test_rejected_traps(void)
{
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 legacy_forms[][3u] = {
        {0x66u,0x90u,0u}, {0x67u,0x90u,0u}, {0x66u,0x67u,0x90u}
    };
    static const lib_u8 lock_forms[][4u] = {
        {0xf0u,0x90u,0u,0u}, {0xf0u,0x66u,0x90u,0u},
        {0xf0u,0x67u,0x90u,0u}, {0xf0u,0x66u,0x67u,0x90u}
    };
    lib_size profile, form;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]); ++profile)
        for (form = 0u; form != sizeof(legacy_forms) / sizeof(legacy_forms[0]);
            ++form)
            if (!debug_board_expect_real_ud(legacy[profile], legacy_forms[form],
                    legacy_forms[form][2] == 0u ? 2u : 3u)) return 0;
    for (form = 0u; form != sizeof(lock_forms) / sizeof(lock_forms[0]); ++form)
        if (!debug_board_expect_real_ud(CORE_MACHINE_CPU_PROFILE_80386,
                lock_forms[form], lock_forms[form][3] == 0u ?
                (lock_forms[form][2] == 0u ? 2u : 3u) : 4u)) return 0;
    return 1;
}

static lib_i32 debug_board_test_protected_breakpoint(void)
{
    static const lib_u8 code[60u] = {
        0x66u,0xb8u,0x40u,0x20u,0x00u,0x00u,
        0x66u,0x0fu,0x23u,0xc0u,
        0x66u,0xb8u,0x01u,0x00u,0x00u,0x00u,
        0x66u,0x0fu,0x23u,0xf8u,
        0xe9u,0x24u,0x00u,
        [59u] = 0x90u
    };
    debug_board_context context;
    core_machine *machine = LIB_NULL;
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    lib_i32 failed = !debug_board_create(CORE_MACHINE_CPU_PROFILE_80386,
        &context, &machine);

    if (!failed) failed = !debug_board_boot_protected(machine) ||
        !debug_board_install_protected_vector(machine) ||
        !debug_board_run_at(machine, DEBUG_BOARD_CODE + 5u, code, sizeof(code),
            (core_machine_run_budget){32u,0u}, &result, &after, &diagnostic) ||
        result.reason != CORE_MACHINE_STOP_BUDGET ||
        !diagnostic.last_delivered_exception.valid ||
        diagnostic.last_delivered_exception.exception_mask != VCPUINS_EXCEPT_DB ||
        after.eip != 0x0100u;
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 debug_board_test_trap_priority(void)
{
    static const lib_u8 nop[] = {0x90u};
    debug_board_context context;
    core_machine *machine = LIB_NULL;
    core_machine_pic_irq_source source = {0};
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    const lib_u32 values[CORE_MACHINE_DEBUG_REGISTER_COUNT] = {
        [CORE_MACHINE_DEBUG_ESP] = 0x8000u,
        [CORE_MACHINE_DEBUG_EFLAGS] = VCPU_EFLAGS_TF | VCPU_EFLAGS_IF
    };
    lib_i32 failed = !debug_board_create(CORE_MACHINE_CPU_PROFILE_80386,
        &context, &machine);

    if (!failed) failed = !debug_board_install_real_vector(machine, 1u, 0x0100u) ||
        !debug_board_install_real_vector(machine, 0x20u, 0x0110u) ||
        !debug_board_patch(machine,
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS), values);
    if (!failed) {
        test_pic_program_vector(&machine->board->shared_pic_master, 0x20u);
        core_machine_pic_irq_source_bind(&source, &machine->board->shared_pic_master,
            &machine->board->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed = !debug_board_run(machine, nop, sizeof(nop), &result, &after,
            &diagnostic) || result.reason !=
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            !diagnostic.last_delivered_exception.valid ||
            diagnostic.last_delivered_exception.exception_mask != VCPUINS_EXCEPT_DB ||
            after.eip != 0x0101u ||
            (test_pic_read(&machine->board->shared_pic_master, 0x0bu) & VPIC_ISR_IRQ(0u)) != 0u ||
            (test_pic_read(&machine->board->shared_pic_master, 0x0au) & VPIC_IRR_IRQ(0u)) == 0u;
    }
    core_machine_destroy(machine);
    return !failed;
}

int main(void)
{
    if (!debug_board_test_real_delivery()) {
        printf("debug-state-board stage=real-delivery\n");
        return 1;
    }
    if (!debug_board_test_pic_order()) {
        printf("debug-state-board stage=pic-order\n");
        return 1;
    }
    if (!debug_board_test_protected_trap()) {
        printf("debug-state-board stage=protected-trap\n");
        return 1;
    }
    if (!debug_board_test_rejected_traps()) {
        printf("debug-state-board stage=rejected-traps\n");
        return 1;
    }
    if (!debug_board_test_trap_priority()) {
        printf("debug-state-board stage=trap-priority\n");
        return 1;
    }
    if (!debug_board_test_protected_breakpoint()) {
        printf("debug-state-board stage=protected-breakpoint\n");
        return 1;
    }
    printf("M5:T539:S46:DEBUG-STATE-BOARD:OK\n");
    return 0;
}
