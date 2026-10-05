#include "pic_fixture.h"
#include "ibmpc/board-common/machine_board_state.h"
#include "core_machine_board_fixture.h"
#include "../../x86/core/exception_fixture.h"
#include "x86/chips/cpu/cpu_interface.h"
#include "x86/core/device_support_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include <stdio.h>

/* T337_REAL_UD_TERMINAL_IVT_REJECT: vector 6 is deliberately unreadable. */

typedef struct control_board_reset_context {
    core_machine *machine;
    core_machine_board_state *board;
} control_board_reset_context;

static lib_i32 control_board_create(core_machine_cpu_profile profile,
    lib_bool cpu_80386_cr_mov_ignores_mod, control_board_reset_context *context,
    core_machine **out_machine)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile, .fpu_profile = X86_FPU_PROFILE_NONE,
        .cpu_80386_cr_mov_ignores_mod = cpu_80386_cr_mov_ignores_mod
    };

    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };

    *out_machine = LIB_NULL;
    lib_memory_set(context, 0, sizeof(*context));
    return core_machine_create(&config, &context->machine, &context->board) ==
            LIB_STATUS_OK &&
        test_core_exception_block_vector(context->machine, 6u, context) ==
            LIB_STATUS_OK &&
        core_machine_freeze_execution_providers(context->machine) == LIB_STATUS_OK &&
        core_machine_reset(context->machine) == LIB_STATUS_OK &&
        core_machine_debug_patch_registers(context->machine, &entry) == LIB_STATUS_OK &&
        ((*out_machine = context->machine) != LIB_NULL);
}

static lib_i32 control_board_fault(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_u32 exception)
{
    core_machine *machine = LIB_NULL;
    control_board_reset_context context;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    lib_i32 failed = !control_board_create(profile, LIB_FALSE, &context,
        &machine);

    if (!failed)
        failed = core_machine_memory_write(machine, 0u, code, bytes) !=
            LIB_STATUS_OK || core_machine_run(machine,
            (core_machine_run_budget){1u,0u}, &result) !=
            LIB_STATUS_INTERNAL_ERROR || core_machine_get_cpu_diagnostic(machine,
            &diagnostic) != LIB_STATUS_OK;
    if (!failed)
        failed = !diagnostic.first_fault.valid ||
            (diagnostic.first_fault.exception_mask & exception) == 0u;
    core_machine_destroy(machine);
    return !failed;
}

static lib_i32 control_board_profile_and_lock(void)
{
    static const lib_u8 clts[] = {0x0fu,0x06u};
    static const lib_u8 clts_attributes[][4] = {
        {0x66u,0x0fu,0x06u}, {0x67u,0x0fu,0x06u},
        {0x66u,0x67u,0x0fu,0x06u}
    };
    static const lib_u8 clts_attribute_bytes[] = {3u,3u,4u};
    static const lib_u8 msw_attributes[][6] = {
        {0x66u,0x0fu,0x01u,0xe0u}, {0x67u,0x0fu,0x01u,0xe0u},
        {0x66u,0x67u,0x0fu,0x01u,0xe0u},
        {0x66u,0x0fu,0x01u,0xf0u}, {0x67u,0x0fu,0x01u,0xf0u},
        {0x66u,0x67u,0x0fu,0x01u,0xf0u}
    };
    static const lib_u8 msw_attribute_bytes[] = {4u,4u,5u,4u,4u,5u};
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 lock_clts[] = {0xf0u,0x0fu,0x06u};
    static const lib_u8 lock_msw[][6] = {
        {0xf0u,0x0fu,0x01u,0xe0u},
        {0xf0u,0x66u,0x0fu,0x01u,0xe0u},
        {0xf0u,0x67u,0x0fu,0x01u,0xe0u},
        {0xf0u,0x66u,0x67u,0x0fu,0x01u,0xe0u},
        {0xf0u,0x0fu,0x01u,0xf0u},
        {0xf0u,0x66u,0x0fu,0x01u,0xf0u},
        {0xf0u,0x67u,0x0fu,0x01u,0xf0u},
        {0xf0u,0x66u,0x67u,0x0fu,0x01u,0xf0u}
    };
    static const lib_u8 lock_msw_bytes[] = {4u,5u,5u,6u,4u,5u,5u,6u};

    lib_size profile, form;

    if (!control_board_fault(CORE_MACHINE_CPU_PROFILE_80186, clts,
            sizeof(clts), VCPUINS_EXCEPT_UD)) {
        printf("control board legacy\n");
        return 0;
    }
    for (profile = 0u; profile != 3u; ++profile)
        for (form = 0u; form != 3u; ++form)
            if (!control_board_fault(legacy[profile], clts_attributes[form],
                    clts_attribute_bytes[form], VCPUINS_EXCEPT_UD)) return 0;
    for (profile = 0u; profile != 3u; ++profile)
        for (form = 0u; form != 6u; ++form)
            if (!control_board_fault(legacy[profile], msw_attributes[form],
                    msw_attribute_bytes[form], VCPUINS_EXCEPT_UD)) return 0;
    if (!control_board_fault(CORE_MACHINE_CPU_PROFILE_80386, lock_clts,
            sizeof(lock_clts), VCPUINS_EXCEPT_UD)) {
        printf("control board lock-clts\n");
        return 0;
    }
    for (form = 0u; form != 8u; ++form)
        if (!control_board_fault(CORE_MACHINE_CPU_PROFILE_80386,
                lock_msw[form], lock_msw_bytes[form], VCPUINS_EXCEPT_UD))
            return 0;
    return 1;
}

static lib_i32 control_board_irq(void)
{
    static const lib_u8 clts[] = {0x0fu,0x06u,0x90u};
    static const lib_u8 smsw[] = {0x0fu,0x01u,0xe0u,0x90u};
    static const lib_u8 lmsw[] = {0x0fu,0x01u,0xf0u,0x90u};
    static const lib_u16 vector[] = {0x0100u,0u};
    static const lib_u8 handler = 0xf4u;
    const lib_u8 *codes[] = {clts, smsw, lmsw};
    const lib_u8 code_bytes[] = {sizeof(clts),sizeof(smsw),sizeof(lmsw)};
    const lib_u8 return_ip[] = {2u,3u,3u};
    lib_size operation;

    for (operation = 0u; operation != 3u; ++operation) {
        const core_machine_debug_register_patch patch = {
            .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CR0),
            .values = {
                [CORE_MACHINE_DEBUG_EAX] = 0xdead000cu,
                [CORE_MACHINE_DEBUG_ESP] = 0x8000u,
                [CORE_MACHINE_DEBUG_EFLAGS] = CORE_MACHINE_DEBUG_EFLAGS_IF,
                [CORE_MACHINE_DEBUG_CR0] = operation == 1u ? 0x00a50000u :
                    VCPU_CR0_TS
            }
        };
        core_machine *machine = LIB_NULL;
        control_board_reset_context context;
        core_machine_pic_irq_source *source = LIB_NULL;
        core_machine_run_result result = {0};
        core_machine_debug_cpu_snapshot after = {0};
        lib_u16 frame_ip = 0u;
        lib_i32 failed = !control_board_create(CORE_MACHINE_CPU_PROFILE_80386,
            LIB_FALSE, &context, &machine);

        if (!failed)
            failed = core_machine_memory_write(machine, 0u, codes[operation],
                    code_bytes[operation]) != LIB_STATUS_OK ||
                core_machine_memory_write(machine, 0x80u, vector,
                    sizeof(vector)) != LIB_STATUS_OK || core_machine_memory_write(
                    machine, 0x100u, &handler, sizeof(handler)) != LIB_STATUS_OK ||
                core_machine_debug_patch_registers(machine, &patch) != LIB_STATUS_OK;
        if (!failed) {
            test_pic_program_vector(context.board->shared_pic_master, 0x20u);
            test_pic_bind_source(&source, context.board->shared_pic_master,
                context.board->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(source);
            core_machine_pic_irq_source_deassert(source);
            failed = core_machine_run(machine, (core_machine_run_budget){2u,0u},
                &result) != LIB_STATUS_OK || core_machine_debug_capture_cpu_snapshot(
                machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
                core_machine_debug_read_memory(machine, after.ss.base +
                (lib_u16)after.esp, &frame_ip, sizeof(frame_ip)) != LIB_STATUS_OK;
        }
        if (!failed)
            failed = result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
                after.eip != 0x0101u || (after.eflags & CORE_MACHINE_DEBUG_EFLAGS_IF) != 0u ||
                (test_pic_read(context.board->shared_pic_master, 0x0bu) &
                VPIC_ISR_IRQ(0u)) == 0u || (operation == 0u &&
                (after.cr0 & VCPU_CR0_TS) != 0u) || (operation == 1u &&
                (after.eax & 0xffffu) != 0u) || (operation == 2u &&
                (after.cr0 & 0x0fu) != 0x0cu) || frame_ip != return_ip[operation];
        core_machine_destroy(machine);
        if (failed) return 0;
    }
    return 1;
}

static lib_i32 control_board_early_80386_mov_cr(void)
{
    static const lib_u8 read[] = {0x66u,0x0fu,0x20u,0x80u,0xf4u};
    static const lib_u8 write[] = {0x66u,0x0fu,0x22u,0x80u,0xf4u};
    const core_machine_debug_register_patch initial = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CR0),
        .values = {
            [CORE_MACHINE_DEBUG_EAX] = 0xdeadbeefu,
            [CORE_MACHINE_DEBUG_CR0] = 0x0000000cu
        }
    };
    const core_machine_debug_register_patch write_initial = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX),
        .values = {[CORE_MACHINE_DEBUG_EAX] = 0u}
    };
    const lib_u8 *codes[] = {read, write};
    const core_machine_debug_register_patch *patches[] = {&initial,
        &write_initial};
    lib_size index;

    for (index = 0u; index != 2u; ++index) {
        core_machine *machine = LIB_NULL;
        control_board_reset_context context;
        core_machine_run_result result = {0};
        core_machine_debug_cpu_snapshot after = {0};
        lib_i32 failed = !control_board_create(CORE_MACHINE_CPU_PROFILE_80386,
            LIB_TRUE, &context, &machine);

        if (!failed)
            failed = core_machine_memory_write(machine, 0u, codes[index], 5u) !=
                    LIB_STATUS_OK || core_machine_debug_patch_registers(machine,
                    patches[index]) != LIB_STATUS_OK || core_machine_run(machine,
                    (core_machine_run_budget){32u,0u}, &result) != LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        if (!failed)
            failed = result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
                (index == 0u && after.eax != 0x0000000cu) ||
                (index == 1u && after.cr0 != 0u);
        core_machine_destroy(machine);
        if (failed) return 0;
    }
    return 1;
}

int main(void)
{
    if (!control_board_profile_and_lock()) {
        printf("control-state-board stage=profile-lock\n");
        return 1;
    }
    if (!control_board_irq()) {
        printf("control-state-board stage=irq\n");
        return 1;
    }
    if (!control_board_early_80386_mov_cr()) {
        printf("control-state-board stage=early-80386-mov-cr\n");
        return 1;
    }
    printf("M5:T539:S45:CONTROL-STATE-BOARD:OK\n");
    return 0;
}
