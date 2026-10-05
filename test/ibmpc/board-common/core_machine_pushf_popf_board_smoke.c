#include "lib/types/types_interface.h"
#include <stdio.h>
#include "cpu_board_irq_fixture.h"
#include "cpu_board_limit_fixture.h"

static lib_i32 pushf_board_gprs_same(
    const core_machine_debug_cpu_snapshot *before,
    const core_machine_debug_cpu_snapshot *after)
{
    return before->eax == after->eax && before->ecx == after->ecx &&
        before->edx == after->edx && before->ebx == after->ebx &&
        before->ebp == after->ebp && before->esi == after->esi &&
        before->edi == after->edi;
}

static lib_i32 pushf_board_segments_same(
    const core_machine_debug_cpu_snapshot *before,
    const core_machine_debug_cpu_snapshot *after)
{
    return lib_memory_compare(&before->cs, &after->cs,
            sizeof(before->cs)) == 0 &&
        lib_memory_compare(&before->ss, &after->ss,
            sizeof(before->ss)) == 0 &&
        lib_memory_compare(&before->ds, &after->ds,
            sizeof(before->ds)) == 0 &&
        lib_memory_compare(&before->es, &after->es,
            sizeof(before->es)) == 0 &&
        lib_memory_compare(&before->fs, &after->fs,
            sizeof(before->fs)) == 0 &&
        lib_memory_compare(&before->gs, &after->gs,
            sizeof(before->gs)) == 0;
}

static lib_i32 pushf_board_irq(void)
{
    for (lib_u8 form = 0u; form != 2u; ++form) {
        const lib_u16 image = CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_IF;
        const test_cpu_board_irq_case entry = {
            .code = {form == 0u ? 0x9cu : 0x9du, 0x90u},
            .flags = CORE_MACHINE_DEBUG_EFLAGS_IF | 0x02u,
            .stack_image = image,
            .write_stack_image = form != 0u,
            .budget = 3u
        };
        test_cpu_board_irq_result result;

        if (!test_cpu_board_irq_run(&entry, &result) ||
            result.after.eip != 0x101u || result.frame_ip != 1u ||
            (result.isr & VPIC_ISR_IRQ(0u)) == 0u ||
            (result.irr & VPIC_IRR_IRQ(0u)) != 0u ||
            !pushf_board_gprs_same(&result.before, &result.after) ||
            !pushf_board_segments_same(&result.before, &result.after))
            return 0;
        if (form == 0u) {
            if (result.after.esp != 0x7ff8u ||
                result.stack_word != (lib_u16)((result.before.eflags &
                    ~0xfffc802au) | 0x02u)) return 0;
        } else if (result.after.esp != 0x7ffcu ||
            (result.after.eflags & 0xffffu) !=
                ((image & ~(0xfffc802au | CORE_MACHINE_DEBUG_EFLAGS_IF)) |
                    0x02u)) return 0;
    }
    return 1;
}

static lib_i32 pushf_board_stack_limit_fault(void)
{
    /* Reload a real guest SS descriptor whose 0x7fff limit excludes ESP. */
    static const lib_u8 code[] = {
        0xb8u,0x18u,0x00u,0x8eu,0xd0u,0x66u,0x9du
    };
    const lib_u8 limit_high = 0x7fu;
    const lib_u32 image = CORE_MACHINE_DEBUG_EFLAGS_ZF | CORE_MACHINE_DEBUG_EFLAGS_IF;
    core_machine *machine = LIB_NULL;
    core_machine_debug_cpu_snapshot before = {0};
    core_machine_debug_cpu_snapshot after = {0};
    core_machine_debug_register_patch patch = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP)
    };
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_run_result result = {0};
    lib_status fault_status = LIB_STATUS_OK;
    lib_i32 failed = !test_cpu_board_limit_prepare(&machine, code,
        sizeof(code), LIB_TRUE, LIB_FALSE);

    if (!failed) {
        patch.values[CORE_MACHINE_DEBUG_ESP] = 0x7ffcu;
        failed = core_machine_memory_write(machine, 0x0319u,
                &limit_high, sizeof(limit_high)) != LIB_STATUS_OK ||
            core_machine_debug_patch_registers(machine, &patch) !=
                LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0xc000u,
                &image, sizeof(image)) != LIB_STATUS_OK ||
            core_machine_run(machine, (core_machine_run_budget){2u,0u},
                &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET;
        patch.values[CORE_MACHINE_DEBUG_ESP] = 0x8000u;
        if (!failed) failed = core_machine_debug_patch_registers(machine,
                &patch) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) !=
                LIB_STATUS_OK || before.eip != 5u;
        if (!failed) fault_status = core_machine_run(machine,
            (core_machine_run_budget){1u,0u}, &result);
        if (!failed) {
            failed = fault_status != LIB_STATUS_INTERNAL_ERROR ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
                    LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) !=
                    LIB_STATUS_OK;
        }
        if (!failed) failed = !diagnostic.first_fault.valid ||
            after.eip != before.eip || after.esp != before.esp ||
            after.eflags != before.eflags;
    }
    core_machine_destroy(machine);
    return !failed;
}

int main(void)
{
    if (!pushf_board_irq() || !pushf_board_stack_limit_fault()) {
        fputs("M5:T539:S36:PUSHF-POPF-BOARD:FAIL\n", stderr);
        return 1;
    }
    puts("M5:T539:S36:PUSHF-POPF-BOARD:OK");
    return 0;
}
