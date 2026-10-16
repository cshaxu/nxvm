#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "pic_fixture.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-base/machine_board_state.h"
#include "board_board_fixture.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"
typedef struct gpr_push_pop_machine {
    x86_core_machine *machine;
    x86_nxvm_board_state *board;
} gpr_push_pop_machine;

static lib_i32 gpr_push_pop_prepare(x86_core_cpu_profile profile, gpr_push_pop_machine *state)
{
    const x86_nxvm_board_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = profile, .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP)
    };
    lib_memory_set(state, 0, sizeof(*state));
    return x86_nxvm_board_create(&config, &state->machine, &state->board) == LIB_STATUS_OK &&
        x86_core_machine_freeze_execution_providers(state->machine) == LIB_STATUS_OK &&
        x86_core_machine_reset(state->machine) == LIB_STATUS_OK &&
        x86_core_machine_debug_patch_registers(state->machine, &entry) == LIB_STATUS_OK;
}

static lib_status gpr_push_pop_seed(gpr_push_pop_machine *state, lib_u32 esp, lib_u32 eax)
{
    const x86_core_cpu_debug_register_patch registers = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EAX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ECX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EBX) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EBP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESI) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EDI) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS),
        .values = {
            [X86_CORE_CPU_DEBUG_EAX] = eax, [X86_CORE_CPU_DEBUG_ECX] = 0xb1b25566u,
            [X86_CORE_CPU_DEBUG_EDX] = 0xc1c27788u, [X86_CORE_CPU_DEBUG_EBX] = 0xd1d299aau,
            [X86_CORE_CPU_DEBUG_ESP] = esp, [X86_CORE_CPU_DEBUG_EBP] = 0xe1e2bbcdu,
            [X86_CORE_CPU_DEBUG_ESI] = 0xf1f2ddefu, [X86_CORE_CPU_DEBUG_EDI] = 0x1122a5a5u,
            [X86_CORE_CPU_DEBUG_EFLAGS] = 0x245u
        }
    };
    return x86_core_machine_debug_patch_registers(state->machine, &registers);
}

static lib_i32 gpr_push_pop_boot_protected(gpr_push_pop_machine *state, lib_u8 limit_segment,
    lib_u16 limit, lib_bool expdown)
{
    static const lib_u8 pointer[] = {0x1fu, 0u, 0u, 0x03u, 0u, 0u};
    lib_u8 gdt[] = {
        0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x20u, 0u, 0x9au, 0u, 0u,
        0xffu, 0xffu, 0u, 0x30u, 0u, 0x92u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x40u, 0u, 0x92u, 0u, 0u
    };
    static const lib_u8 bootstrap[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u, 0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u, 0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u,
        0x8eu, 0xc0u, 0xb8u, 0x18u, 0x00u, 0x8eu, 0xd0u, 0xbcu,
        0x00u, 0x80u, 0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    x86_core_machine_run_result result;

    lib_i32 ready = x86_core_machine_memory_write(state->machine, 0x0100u, pointer,
        sizeof(pointer)) == LIB_STATUS_OK && x86_core_machine_memory_write(
        state->machine, 0x0300u, gdt, sizeof(gdt)) == LIB_STATUS_OK &&
        x86_core_machine_memory_write(state->machine, 0u, bootstrap,
        sizeof(bootstrap)) == LIB_STATUS_OK &&
        x86_core_machine_run(state->machine, (x86_core_machine_run_budget){10u, 0u},
        &result) == LIB_STATUS_OK && result.reason ==
        X86_CORE_MACHINE_STOP_BUDGET && result.executed == 10u;

    const x86_core_cpu_debug_register reg = limit_segment == 0u ?
        X86_CORE_CPU_DEBUG_SS : X86_CORE_CPU_DEBUG_DS;
    const x86_core_cpu_debug_register_patch segment = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(reg),
        .values = {
            [X86_CORE_CPU_DEBUG_SS] = 0x18u,
            [X86_CORE_CPU_DEBUG_DS] = 0x10u
        }
    };
    lib_u8 descriptor = limit_segment == 0u ? 24u : 16u;

    /* Prepare the original invalid-stack precondition at the paused boundary.
     * Executing MOV SS with that ESP would fault before the tested instruction. */
    if (!ready) return 0;
    gdt[descriptor] = (lib_u8)limit;
    gdt[descriptor + 1u] = (lib_u8)(limit >> 8);
    if (limit_segment == 0u && expdown) gdt[descriptor + 5u] = 0x96u;
    return x86_core_machine_memory_write(state->machine, 0x0300u, gdt,
        sizeof(gdt)) == LIB_STATUS_OK &&
        x86_core_machine_debug_patch_registers(state->machine, &segment) == LIB_STATUS_OK;
}

static lib_i32 gpr_push_pop_sregs_same(const x86_core_cpu_debug_cpu_snapshot *before, const x86_core_cpu_debug_cpu_snapshot *after)
{
    return lib_memory_compare(&before->es, &after->es,
        sizeof(before->es)) == 0 && lib_memory_compare(&before->cs,
        &after->cs, sizeof(before->cs)) == 0 && lib_memory_compare(
        &before->ss, &after->ss, sizeof(before->ss)) == 0 &&
        lib_memory_compare(&before->ds, &after->ds,
        sizeof(before->ds)) == 0 && lib_memory_compare(&before->fs,
        &after->fs, sizeof(before->fs)) == 0 && lib_memory_compare(
        &before->gs, &after->gs, sizeof(before->gs)) == 0;
}

static lib_i32 gpr_push_pop_protected_fault(const lib_u8 *code, lib_u8 bytes,
    lib_u8 limit_segment, lib_u32 limit, lib_i32 expdown)
{
    gpr_push_pop_machine state;
    x86_core_machine_run_result result;
    x86_core_cpu_debug_cpu_snapshot before;
    x86_core_cpu_debug_cpu_snapshot after;
    lib_status fault_status;
    lib_u32 sentinel = 0xdeadbeefu;
    lib_u32 observed = 0u;
    lib_i32 failed = !gpr_push_pop_prepare(X86_CORE_CPU_PROFILE_80386,
        &state);

    if (!failed)
        failed |= !gpr_push_pop_boot_protected(&state, limit_segment, (lib_u16)limit, expdown);
    if (!failed)
    {
        failed |= gpr_push_pop_seed(&state, 0x12348000u, 0xa1a23344u) != LIB_STATUS_OK;
        failed |= x86_core_machine_memory_write(state.machine, 0x3010u, &sentinel,
            sizeof(sentinel)) != LIB_STATUS_OK || x86_core_machine_memory_write(
            state.machine, 0x4010u, &sentinel, sizeof(sentinel)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(state.machine, 0x47ffeu, &sentinel,
            sizeof(sentinel)) != LIB_STATUS_OK || x86_core_machine_memory_write(
            state.machine, 0x2000u, code, bytes) != LIB_STATUS_OK;
        failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
        fault_status = x86_core_machine_run(state.machine,
            (x86_core_machine_run_budget){1u, 0u}, &result);
        failed |= !test_x86_nxvm_machine_fixture_shutdown_wait(fault_status, &result);
        failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        if (!failed)
            failed |= after.eip != 0u || after.eax != before.eax ||
            after.ecx != before.ecx || after.edx != before.edx ||
            after.ebx != before.ebx || after.esp != before.esp ||
            after.ebp != before.ebp || after.esi != before.esi ||
            after.edi != before.edi || after.eflags !=
            before.eflags || !gpr_push_pop_sregs_same(&before, &after) ||
            x86_core_machine_memory_inspect(state.machine, 0x3010u, (void *)lib_pointer_to_uptr(&(observed)),
            sizeof(observed)) != LIB_STATUS_OK || observed != sentinel ||
            x86_core_machine_memory_inspect(state.machine,
            0x4010u, (void *)lib_pointer_to_uptr(&(observed)), sizeof(observed)) !=
            LIB_STATUS_OK || observed != sentinel ||
            x86_core_machine_memory_inspect(state.machine,
            0x47ffeu, (void *)lib_pointer_to_uptr(&(observed)), sizeof(observed)) !=
            LIB_STATUS_OK || observed != sentinel;
    }
    x86_core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 gpr_push_pop_test_protected_faults(void)
{
    static const lib_u8 push[] = {0x50u};
    static const lib_u8 pop[] = {0x59u};
    static const lib_u8 push_source[] = {0xffu, 0x36u, 0x10u, 0x00u};
    static const lib_u8 pop_dest[] = {0x8fu, 0x06u, 0x10u, 0x00u};

    if (!gpr_push_pop_protected_fault(push, sizeof(push), 0u, 0xffffu,
        LIB_TRUE))
    {
        lib_c_printf("protected push\n");
        return 0;
    }
    if (!gpr_push_pop_protected_fault(pop, sizeof(pop), 0u, 0x7fffu,
        LIB_FALSE))
    {
        lib_c_printf("protected pop\n");
        return 0;
    }
    if (!gpr_push_pop_protected_fault(push_source, sizeof(push_source), 1u,
        0x0fu, LIB_FALSE))
    {
        lib_c_printf("protected push-source\n");
        return 0;
    }
    if (!gpr_push_pop_protected_fault(pop_dest, sizeof(pop_dest), 1u, 0x0fu,
        LIB_FALSE))
    {
        lib_c_printf("protected pop-dest\n");
        return 0;
    }
    return 1;
}

static lib_i32 gpr_push_pop_test_irq_no_shadow(void)
{
    static const lib_u8 push_reg[] = {0x50u, 0x90u};
    static const lib_u8 pop_reg[] = {0x59u, 0x90u};
    static const lib_u8 push_rm[] = {0xffu, 0xf0u, 0x90u};
    static const lib_u8 pop_rm[] = {0x8fu, 0x06u, 0x20u, 0x00u, 0x90u};
    static const lib_u8 halt = 0xf4u;
    const lib_u8 *codes[] = {push_reg, pop_reg, push_rm, pop_rm};
    const lib_u8 lengths[] = {1u, 1u, 2u, 4u};
    lib_u8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form)
    {
        gpr_push_pop_machine state;
        x86_nxvm_board_pic_irq_source *source = LIB_NULL;
        x86_core_machine_run_result result;
        x86_core_cpu_debug_cpu_snapshot before;
        x86_core_cpu_debug_cpu_snapshot after;
        lib_u16 vector_offset = 0x100u;
        lib_u16 vector_segment = 0u;
        lib_u16 frame_ip = 0u;
        lib_u16 image = 0xfaceu;
        lib_u16 source_word = 0x7788u;
        lib_u16 observed = 0u;
        lib_i32 failed = !gpr_push_pop_prepare(X86_CORE_CPU_PROFILE_80386,
            &state);

        if (!failed)
        {
            failed |= gpr_push_pop_seed(&state, 0x12348000u, form == 2u ? 0xa1a27788u : 0xa1a23344u) != LIB_STATUS_OK;
            if (form == 1u || form == 3u)
                failed |= x86_core_machine_memory_write(state.machine, 0x8000u,
                    &image, sizeof(image)) != LIB_STATUS_OK;
            failed |= x86_core_machine_memory_write(state.machine, 0x20u,
                &source_word, sizeof(source_word)) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(state.machine, 0u, codes[form],
                lengths[form] + 1u) != LIB_STATUS_OK || x86_core_machine_memory_write(
                state.machine, 0x80u, &vector_offset, sizeof(vector_offset)) !=
                LIB_STATUS_OK || x86_core_machine_memory_write(state.machine, 0x82u,
                &vector_segment, sizeof(vector_segment)) != LIB_STATUS_OK ||
                x86_core_machine_memory_write(state.machine, 0x100u, &halt,
                sizeof(halt)) != LIB_STATUS_OK;
        }
        if (!failed)
        {
            lib_memory_set(&source, 0, sizeof(source));
            test_pic_program_vector(state.board->shared_pic_master, 0x20u);
            test_pic_bind_source(&source,
                state.board->shared_pic_master, state.board->shared_pic_slave,
                0u);
            x86_nxvm_board_pic_irq_source_assert(source);
            x86_nxvm_board_pic_irq_source_deassert(source);
            failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
            failed |= x86_core_machine_run(state.machine,
                (x86_core_machine_run_budget){2u, 0u}, &result) != LIB_STATUS_OK ||
                result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            failed |= x86_core_machine_debug_capture_cpu_snapshot(state.machine,
            X86_CORE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
            if (!failed)
                failed |= x86_core_machine_memory_inspect(state.machine,
                after.ss.base + (lib_u16)after.esp,
                (void *)lib_pointer_to_uptr(&(frame_ip)), sizeof(frame_ip)) != LIB_STATUS_OK ||
                after.eip != 0x101u || frame_ip != lengths[form] ||
                !(((test_pic_read(state.board->shared_pic_master, 0x0bu)) & (VPIC_ISR_IRQ(0u))) != 0u) || (((test_pic_read(state.board->shared_pic_master, 0x0au)) & (VPIC_IRR_IRQ(0u))) != 0u) ||
                after.eflags != (before.eflags & ~0x200u);
            if (!failed && (form == 0u || form == 2u))
            {
                lib_u16 expected = form == 0u ?
                    (lib_u16)before.eax : source_word;

                failed |= after.esp != 0x12347ff8u ||
                    x86_core_machine_memory_inspect(state.machine,
                    0x7ffeu, (void *)lib_pointer_to_uptr(&(observed)), sizeof(observed)) !=
                    LIB_STATUS_OK || observed != expected;
            }
            else if (!failed && form == 1u)
                failed |= after.ecx != 0xb1b2faceu ||
                    after.esp != 0x12347ffcu;
            else if (!failed)
            {
                failed |= after.esp != 0x12347ffcu ||
                    x86_core_machine_memory_inspect(state.machine,
                    0x20u, (void *)lib_pointer_to_uptr(&(observed)), sizeof(observed)) !=
                    LIB_STATUS_OK || observed != image;
            }
        }
        x86_core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!gpr_push_pop_test_protected_faults())
    {
        lib_c_printf("GPR-PUSH-POP stage=protected\n");
        return 1;
    }
    if (!gpr_push_pop_test_irq_no_shadow())
    {
        lib_c_printf("GPR-PUSH-POP stage=irq\n");
        return 1;
    }
    lib_c_printf("GPR-PUSH-POP:OK\n");
    lib_c_printf("GPR-PUSH-POP-PROFILES:OK\n");
    lib_c_printf("GROUP5-PUSH-RM-PROFILES:OK\n");
    return 0;
}
