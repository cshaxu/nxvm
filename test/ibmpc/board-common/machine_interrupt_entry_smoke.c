#include "lib/types/test.h"
#include "lib/types/file.h"
#include "pic_fixture.h"
#include "ibmpc/board-common/machine_board_state.h"
#include "lib/types/types_interface.h"
#include "x86/core/device_support_interface.h"

#include "x86/chips/cpu/cpu_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "../core/debug_fixture.h"
#include "core_machine_board_fixture.h"

#define IE_GDT_BASE 0x0300u
#define IE_IDT_BASE 0x0400u
#define IE_CODE_BASE 0x2000u
#define IE_STACK_BASE 0x8000u
#define IE_HANDLER_OFFSET 0x0100u
#define IE_VECTOR 0x30u
#define IE_INTGATE_32 0x0eu
#define IE_TRAPGATE_32 0x0fu

typedef struct interrupt_entry_machine {
    core_machine *machine;
    core_machine_board_state *board;
} interrupt_entry_machine;

typedef enum interrupt_entry_negative {
    INTERRUPT_ENTRY_NEGATIVE_NONE,
    INTERRUPT_ENTRY_NEGATIVE_IDT_LIMIT,
    INTERRUPT_ENTRY_NEGATIVE_GATE_TYPE,
    INTERRUPT_ENTRY_NEGATIVE_GATE_DPL,
    INTERRUPT_ENTRY_NEGATIVE_GATE_NOT_PRESENT,
    INTERRUPT_ENTRY_NEGATIVE_CODE_TYPE,
    INTERRUPT_ENTRY_NEGATIVE_CODE_NOT_PRESENT,
    INTERRUPT_ENTRY_NEGATIVE_CODE_LIMIT,
    INTERRUPT_ENTRY_NEGATIVE_STACK_LIMIT
} interrupt_entry_negative;

typedef enum interrupt_entry_delivery_failure {
    INTERRUPT_ENTRY_DELIVERY_INVALID_GATE,
    INTERRUPT_ENTRY_DELIVERY_NONPRESENT_GATE,
    INTERRUPT_ENTRY_DELIVERY_TARGET_NOT_PRESENT,
    INTERRUPT_ENTRY_DELIVERY_STACK_LIMIT
} interrupt_entry_delivery_failure;

static core_machine_debug_cpu_snapshot ie_capture(const core_machine *machine)
{
    core_machine_debug_cpu_snapshot snapshot = {0};
    if (core_machine_debug_capture_cpu_snapshot(machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT,
            &snapshot) != LIB_STATUS_OK) lib_test_assert(LIB_FALSE);
    return snapshot;
}

static lib_i32 ie_prepare_user_code(interrupt_entry_machine *state);

static lib_i32 ie_write(interrupt_entry_machine *state, lib_u32 address,
    const void *data, lib_size bytes)
{
    return state != LIB_NULL && state->machine != LIB_NULL &&
        core_machine_memory_write(state->machine, address, data, bytes) ==
            LIB_STATUS_OK;
}

static lib_i32 ie_read(interrupt_entry_machine *state, lib_u32 address,
    void *data, lib_size bytes)
{
    return state != LIB_NULL && state->machine != LIB_NULL &&
        core_machine_memory_inspect(state->machine,
            address, (void *)data, bytes) == LIB_STATUS_OK;
}

static lib_i32 ie_install_gate(interrupt_entry_machine *state, lib_u8 vector,
    lib_u16 selector, lib_u8 gate_type)
{
    lib_u8 gate[8] = {0};

    gate[0] = IE_HANDLER_OFFSET & 0xffu;
    gate[1] = IE_HANDLER_OFFSET >> 8u;
    gate[2] = selector & 0xffu;
    gate[3] = selector >> 8u;
    gate[5] = gate_type;
    return ie_write(state, IE_IDT_BASE + (lib_u32)vector * 8u, gate,
        sizeof(gate));
}

static lib_i32 ie_prepare(interrupt_entry_machine *state,
    interrupt_entry_negative negative, lib_u8 gate_type)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0xcfu,0,
        0,0,0,0,0,0,0,0
    };
    lib_u8 idt[0x188u] = {0};
    static const lib_u8 code[] = {0xcdu,IE_VECTOR};
    static const lib_u8 handler[] = {0xf4u};
    lib_u8 setup[] = {
        0x0fu,0x01u,0x16u,0,1, 0x0fu,0x01u,0x1eu,6,1,
        0xb8u,1,0, 0x0fu,0x01u,0xf0u, 0xb8u,0x10u,0,
        0x8eu,0xd8u, 0x8eu,0xc0u, 0x8eu,0xd0u,
        0xbcu,0,0x80u, 0xeau,0,0,8,0
    };
    lib_u8 pointers[] = {0x1fu,0,0,3,0,0, 0x87u,1,0,4,0,0};
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = {[CORE_MACHINE_DEBUG_EIP] = 0x0600u, [CORE_MACHINE_DEBUG_EFLAGS] = 2u}
    };
    core_machine_run_result result;
    core_machine_debug_cpu_snapshot after = {0};

    if (state == LIB_NULL) return 0;
    lib_memory_set(state, 0, sizeof(*state));
    idt[IE_VECTOR * 8u] = IE_HANDLER_OFFSET & 0xffu;
    idt[IE_VECTOR * 8u + 1u] = IE_HANDLER_OFFSET >> 8u;
    idt[IE_VECTOR * 8u + 2u] = 0x08u;
    idt[IE_VECTOR * 8u + 5u] = (lib_u8)(0xe0u | gate_type);
    if (negative == INTERRUPT_ENTRY_NEGATIVE_GATE_TYPE)
        idt[IE_VECTOR * 8u + 5u] = 0x80u;
    if (negative == INTERRUPT_ENTRY_NEGATIVE_GATE_DPL) {
        idt[IE_VECTOR * 8u + 2u] = 0x0bu;
        idt[IE_VECTOR * 8u + 5u] = (lib_u8)(0x80u | gate_type);
    }
    if (negative == INTERRUPT_ENTRY_NEGATIVE_GATE_NOT_PRESENT)
        idt[IE_VECTOR * 8u + 5u] &= 0x7fu;
    if (negative == INTERRUPT_ENTRY_NEGATIVE_CODE_LIMIT) {
        gdt[8] = 0u;
        gdt[9] = 0u;
    }
    if (negative == INTERRUPT_ENTRY_NEGATIVE_STACK_LIMIT) {
        gdt[16] = 0x10u;
        gdt[17] = 0u;
        gdt[22] = 0x40u;
        setup[27] = 0u;
    }
    if (negative == INTERRUPT_ENTRY_NEGATIVE_IDT_LIMIT) {
        pointers[6] = 7u;
        pointers[7] = 0u;
    }
    if (core_machine_create(&config, &state->machine, &state->board) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(state->machine) != LIB_STATUS_OK ||
        core_machine_reset(state->machine) != LIB_STATUS_OK ||
        core_machine_debug_patch_registers(state->machine, &entry) != LIB_STATUS_OK ||
        !ie_write(state, 0x0100u, pointers, sizeof(pointers)) ||
        !ie_write(state, 0x0600u, setup, sizeof(setup)) ||
        !ie_write(state, IE_GDT_BASE, gdt, sizeof(gdt)) ||
        !ie_write(state, IE_IDT_BASE, idt, sizeof(idt)) ||
        !ie_write(state, IE_CODE_BASE, code, sizeof(code)) ||
        !ie_write(state, IE_CODE_BASE + IE_HANDLER_OFFSET, handler, sizeof(handler)))
        return 0;
    const lib_status setup_status = core_machine_run(state->machine,
        (core_machine_run_budget){10u,0u}, &result);
    if (setup_status != LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET)
        return 0;
    after = ie_capture(state->machine);
    if (after.eip != 0u || after.cs.selector != 8u || after.cs.base != IE_CODE_BASE ||
        after.ss.selector != 0x10u || after.ss.base != 0u ||
        after.esp != (negative == INTERRUPT_ENTRY_NEGATIVE_STACK_LIMIT ? 0u : IE_STACK_BASE) ||
        after.gdtr.base != IE_GDT_BASE || after.idtr.base != IE_IDT_BASE ||
        after.idtr.limit != (negative == INTERRUPT_ENTRY_NEGATIVE_IDT_LIMIT ? 7u : sizeof(idt)-1u))
        return 0;
    /* Delivery reads the descriptor again; changing it does not change the
     * already loaded code cache, matching the original negative contract. */
    if (negative == INTERRUPT_ENTRY_NEGATIVE_CODE_TYPE ||
        negative == INTERRUPT_ENTRY_NEGATIVE_CODE_NOT_PRESENT) {
        const lib_u8 access = negative == INTERRUPT_ENTRY_NEGATIVE_CODE_TYPE ? 0x92u : 0x1au;
        if (!ie_write(state, IE_GDT_BASE + 13u, &access, sizeof(access))) return 0;
    }
    test_core_machine_fixture_write_register(state->machine, CORE_MACHINE_DEBUG_EFLAGS, 0x302u);
    /* Load the limited stack with an in-range pointer. The original negative
     * seeds an out-of-range stopped ESP for interrupt-frame rollback. */
    if (negative == INTERRUPT_ENTRY_NEGATIVE_STACK_LIMIT)
        test_core_machine_fixture_write_register(state->machine, CORE_MACHINE_DEBUG_ESP, IE_STACK_BASE);
    return negative != INTERRUPT_ENTRY_NEGATIVE_GATE_DPL || ie_prepare_user_code(state);
}

static lib_i32 ie_run(interrupt_entry_machine *state, lib_i32 expect_fault,
    core_machine_debug_cpu_snapshot *out_cpu, core_machine_cpu_diagnostic *out_diagnostic)
{
    const core_machine_run_budget budget = {32u, 0u};
    core_machine_run_result result;
    lib_status status;

    status = test_core_machine_fixture_run_after_delivery(state->machine, budget, &result);
    if (core_machine_get_cpu_diagnostic(state->machine, out_diagnostic) !=
        LIB_STATUS_OK) return 0;
    *out_cpu = ie_capture(state->machine);
    return status == (expect_fault ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) &&
        result.reason == (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT);
}

static lib_i32 ie_run_external(interrupt_entry_machine *state, lib_i32 expect_fault,
    core_machine_debug_cpu_snapshot *out_cpu, core_machine_cpu_diagnostic *out_diagnostic)
{
    const core_machine_run_budget budget = {32u, 0u};
    core_machine_run_result result;
    lib_status status;

    status = test_core_machine_fixture_run_after_delivery(state->machine, budget, &result);
    if (core_machine_get_cpu_diagnostic(state->machine, out_diagnostic) !=
        LIB_STATUS_OK) return 0;
    *out_cpu = ie_capture(state->machine);
    return status == (expect_fault ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) &&
        result.reason == (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_BUDGET);
}

static lib_i32 ie_run_budget(interrupt_entry_machine *state, lib_i32 expect_fault,
    core_machine_debug_cpu_snapshot *out_cpu, core_machine_cpu_diagnostic *out_diagnostic)
{
    return ie_run_external(state, expect_fault, out_cpu, out_diagnostic);
}

static lib_i32 ie_fault_is(const core_machine_cpu_diagnostic *diagnostic,
    lib_u32 mask, lib_u32 code)
{
    return diagnostic->first_fault.valid && CORE_MACHINE_BIT_IS_SET(
        diagnostic->first_fault.exception_mask, mask) &&
        diagnostic->first_fault.exception_code == code;
}

static lib_i32 ie_test_success(lib_u8 gate_type, lib_i32 expect_if)
{
    interrupt_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot after = {0};
    lib_u32 frame[3] = {0u, 0u, 0u};
    lib_u8 code_access = 0u;
    lib_i32 failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE, gate_type);

    if (!failed) {
        failed |= !ie_run(&state, 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.cs.selector != 0x0008u ||
            after.eip != IE_HANDLER_OFFSET + 1u ||
            after.esp != IE_STACK_BASE - 12u ||
            CORE_MACHINE_BIT_IS_SET(after.eflags, CORE_MACHINE_DEBUG_EFLAGS_TF) ||
            (CORE_MACHINE_BIT_IS_SET(after.eflags, CORE_MACHINE_DEBUG_EFLAGS_IF) != expect_if) ||
            !ie_read(&state, IE_STACK_BASE - 12u, frame, sizeof(frame)) ||
            frame[0] != 2u || frame[1] != 0x0008u || frame[2] != 0x00000302u ||
            !ie_read(&state, IE_GDT_BASE + 13u, &code_access,
                sizeof(code_access)) || code_access != 0x9bu;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 ie_test_prefix_keeps_gate_width(void)
{
    interrupt_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot after = {0};
    lib_u32 frame[3] = {0u, 0u, 0u};
    static const lib_u8 code[] = {0x66u,0xcdu,IE_VECTOR};
    lib_i32 failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
        IE_INTGATE_32);

    if (!failed) {
        failed |= !ie_write(&state, IE_CODE_BASE, code, sizeof(code)) ||
            !ie_run(&state, 0, &after, &diagnostic) ||
            after.esp != IE_STACK_BASE - 12u ||
            !ie_read(&state, IE_STACK_BASE - 12u, frame, sizeof(frame)) ||
            frame[0] != 3u || frame[1] != 0x0008u || frame[2] != 0x00000302u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 ie_test_failure(interrupt_entry_negative negative, lib_u32 mask,
    lib_u32 code)
{
    interrupt_entry_machine state;
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot before = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_u8 access_before = 0u;
    lib_u8 access_after = 0u;
    lib_i32 failed = !ie_prepare(&state, negative, IE_INTGATE_32);

    if (!failed) {
        before = ie_capture(state.machine);
        failed |= !ie_read(&state, IE_GDT_BASE + 13u, &access_before,
            sizeof(access_before)) || !ie_run(&state, 1, &after, &diagnostic) ||
            !ie_fault_is(&diagnostic, mask, code) ||
            !ie_read(&state, IE_GDT_BASE + 13u, &access_after,
                sizeof(access_after)) || after.cs.selector != before.cs.selector ||
            after.cs.base != before.cs.base ||
            after.cs.limit != before.cs.limit ||
            after.esp != before.esp || after.eflags != before.eflags ||
            after.eip != before.eip || access_after != access_before;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 ie_prepare_user_code(interrupt_entry_machine *state)
{
    const lib_u8 code_access = 0xfau, stack_access = 0xf2u;
    const lib_u8 selector[] = {0x0bu, 0u};
    const lib_u8 transfer[] = {0xcfu};
    const lib_u32 frame[] = {0u,0x0bu,2u,IE_STACK_BASE,0x13u};
    const lib_u32 flags = ie_capture(state->machine).eflags;
    core_machine_run_result result;
    core_machine_debug_cpu_snapshot after = {0};

    if (!ie_write(state, IE_GDT_BASE + 13u, &code_access, sizeof(code_access)) ||
        !ie_write(state, IE_GDT_BASE + 21u, &stack_access, sizeof(stack_access)) ||
        !ie_write(state, IE_IDT_BASE + IE_VECTOR * 8u + 2u, selector, sizeof(selector)) ||
        !ie_write(state, IE_CODE_BASE + 0x0600u, transfer, sizeof(transfer)) ||
        !ie_write(state, IE_STACK_BASE, frame, sizeof(frame)))
        return 0;
    test_core_machine_fixture_write_register(state->machine, CORE_MACHINE_DEBUG_EFLAGS, 2u);
    test_core_machine_fixture_write_register(state->machine, CORE_MACHINE_DEBUG_EIP, 0x0600u);
    if (core_machine_run(state->machine, (core_machine_run_budget){1u,0u},
            &result) != LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET) return 0;
    after = ie_capture(state->machine);
    if (after.cs.selector != 0x0bu || after.cs.dpl != 3u || after.ss.selector != 0x13u ||
        after.ss.dpl != 3u || after.esp != IE_STACK_BASE || after.eip != 0u) return 0;
    test_core_machine_fixture_write_register(state->machine, CORE_MACHINE_DEBUG_EFLAGS, flags);
    return 1;
}

static lib_i32 ie_set_stack_limit(interrupt_entry_machine *state, lib_u32 limit)
{
    const lib_u8 descriptor[] = {(lib_u8)limit,(lib_u8)(limit >> 8u),0,0,0,0xf2u,0x40u,0};
    const lib_u8 reload[] = {0x66u,0xb8u,0x13u,0,0x8eu,0xd0u};
    const lib_u32 flags = ie_capture(state->machine).eflags;
    const lib_u32 esp = ie_capture(state->machine).esp;
    core_machine_run_result result;
    core_machine_debug_cpu_snapshot after = {0};

    if (!ie_write(state, IE_GDT_BASE + 16u, descriptor, sizeof(descriptor)) ||
        !ie_write(state, IE_CODE_BASE + 0x0600u, reload, sizeof(reload))) return 0;
    test_core_machine_fixture_write_register(state->machine, CORE_MACHINE_DEBUG_EFLAGS, 2u);
    test_core_machine_fixture_write_register(state->machine, CORE_MACHINE_DEBUG_ESP, 0u);
    test_core_machine_fixture_write_register(state->machine, CORE_MACHINE_DEBUG_EIP, 0x0600u);
    const lib_status reload_status = core_machine_run(state->machine,
        (core_machine_run_budget){2u,0u}, &result);
    if (reload_status != LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET)
        return 0;
    after = ie_capture(state->machine);
    if (after.ss.limit != limit || after.ss.selector != 0x13u || after.ss.dpl != 3u)
        return 0;
    test_core_machine_fixture_write_register(state->machine, CORE_MACHINE_DEBUG_EIP, 0u);
    test_core_machine_fixture_write_register(state->machine, CORE_MACHINE_DEBUG_EFLAGS, flags);
    test_core_machine_fixture_write_register(state->machine, CORE_MACHINE_DEBUG_ESP, esp);
    return 1;
}

static lib_i32 ie_test_software_frontends(void)
{
    static const lib_u8 int3[] = {0xccu};
    static const lib_u8 into[] = {0xceu};
    static const lib_u8 into_clear[] = {0xceu,0xf4u};
    const lib_u8 *programs[] = {int3, into};
    const lib_u8 vectors[] = {0x03u, 0x04u};
    const lib_u32 returns[] = {1u, 1u};
    lib_size index;

    for (index = 0u; index < sizeof(programs) / sizeof(programs[0]); ++index) {
        interrupt_entry_machine state;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_debug_cpu_snapshot after = {0};
        lib_u32 frame[3] = {0u, 0u, 0u};
        lib_u32 flags = 0x00000302u;
        lib_i32 failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
            IE_TRAPGATE_32);

        if (!failed) {
            if (vectors[index] == 0x04u) flags |= CORE_MACHINE_DEBUG_EFLAGS_OF;
            test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_EFLAGS, flags);
            failed |= !ie_install_gate(&state, vectors[index], 0x0008u,
                    (lib_u8)(0xe0u | IE_TRAPGATE_32)) ||
                !ie_write(&state, IE_CODE_BASE, programs[index],
                    index == 0u ? sizeof(int3) : sizeof(into)) ||
                !ie_run(&state, 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || after.esp != IE_STACK_BASE - 12u ||
                CORE_MACHINE_BIT_IS_SET(after.eflags, CORE_MACHINE_DEBUG_EFLAGS_TF) ||
                !CORE_MACHINE_BIT_IS_SET(after.eflags, CORE_MACHINE_DEBUG_EFLAGS_IF) ||
                !ie_read(&state, IE_STACK_BASE - 12u, frame, sizeof(frame)) ||
                frame[0] != returns[index] || frame[1] != 0x0008u ||
                frame[2] != flags;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        interrupt_entry_machine state;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_debug_cpu_snapshot after = {0};
        lib_i32 failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
            IE_INTGATE_32);

        if (!failed) {
            test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_EFLAGS, 0x00000202u);
            failed |= !ie_install_gate(&state, 0x04u, 0x0008u,
                    (lib_u8)(0xe0u | IE_INTGATE_32)) ||
                !ie_write(&state, IE_CODE_BASE, into_clear, sizeof(into_clear)) ||
                !ie_run(&state, 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || after.eip != sizeof(into_clear) ||
                after.esp != IE_STACK_BASE || after.eflags != 0x00000202u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static lib_i32 ie_test_external_origin(lib_i32 nmi, lib_i32 reject)
{
    interrupt_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_pic_irq_source *source = LIB_NULL;
    core_machine_debug_cpu_snapshot before = {0};
    core_machine_debug_cpu_snapshot after = {0};
    static const lib_u8 code[] = {0x90u};
    static const lib_u8 handler[] = {0xebu,0xfeu};
    lib_u8 vector = nmi ? 0x02u : IE_VECTOR;
    lib_i32 failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
        IE_INTGATE_32);

    if (!failed) {
        test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_EFLAGS, 0x00000202u);
        failed |= !ie_prepare_user_code(&state) ||
            !ie_install_gate(&state, vector, 0x000bu,
                reject ? 0x80u : (lib_u8)(0x80u | IE_INTGATE_32)) ||
            !ie_write(&state, IE_CODE_BASE, code, sizeof(code)) ||
            !ie_write(&state, IE_CODE_BASE + IE_HANDLER_OFFSET, handler,
                sizeof(handler));
        if (!failed && !nmi) {
            lib_memory_set(&source, 0, sizeof(source));
            test_pic_program_vector(state.board->shared_pic_master, IE_VECTOR);
            test_pic_bind_source(&source,
                state.board->shared_pic_master, state.board->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(source);
            core_machine_pic_irq_source_deassert(source);
        } else if (!failed) {
            failed |= !core_machine_signal_nmi(state.machine);
        }
        if (!failed) {
            before = ie_capture(state.machine);
            failed |= !ie_run_external(&state, reject, &after, &diagnostic);
        }
        if (!failed && !reject) {
            failed |= after.cs.selector != 0x000bu ||
                after.esp != IE_STACK_BASE - 12u ||
                (!CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0bu), 1u) && !nmi) ||
                CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0au), 1u);
            /* CPU-local tests retain the private pending-bit assertions.
             * A second quantum proves that successful delivery is not replayed. */
            if (!failed && nmi) {
                core_machine_debug_cpu_snapshot again;
                failed |= !ie_run_external(&state, 0, &again, &diagnostic) ||
                    again.esp != after.esp;
            }
        } else if (!failed) {
            failed |= after.cs.selector != before.cs.selector ||
                after.esp != before.esp || after.eflags != before.eflags ||
                (nmi ? 0 :
                    (!CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0bu), 1u) ||
                     CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0au), 1u)));
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 ie_delivered_is(const core_machine_cpu_diagnostic *diagnostic,
    lib_u32 mask, lib_u32 code)
{
    return !diagnostic->first_fault.valid &&
        diagnostic->last_delivered_exception.valid &&
        diagnostic->delivered_exception_count == 1u && CORE_MACHINE_BIT_IS_SET(
            diagnostic->last_delivered_exception.exception_mask, mask) &&
        diagnostic->last_delivered_exception.exception_code == code;
}

static lib_i32 ie_delivery_state_equal(const core_machine_debug_cpu_snapshot *before, const core_machine_debug_cpu_snapshot *after)
{
    return before->eip == after->eip &&
        before->esp == after->esp &&
        before->eflags == after->eflags &&
        lib_memory_compare(&before->cs, &after->cs,
            sizeof(before->cs)) == 0 &&
        lib_memory_compare(&before->ss, &after->ss,
            sizeof(before->ss)) == 0;
}

static lib_i32 ie_test_fault_delivery(lib_u32 mask, lib_u8 vector,
    lib_u32 code, lib_i32 user_source)
{
    interrupt_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot after = {0};
    lib_u32 frame[4] = {0u, 0u, 0u, 0u};
    lib_u8 access_before = 0u;
    lib_u8 access_after = 0u;
    static const lib_u8 gp_code[] = {0x0fu,0x01u,0xf0u};
    static const lib_u8 np_code[] = {0xb8u,0x18u,0,0,0,0x8eu,0xd8u};
    static const lib_u8 ss_code[] = {0xb8u,0x18u,0,0,0,0x8eu,0xd0u};
    static const lib_u8 loop[] = {0xebu,0xfeu};
    static const lib_u8 halt[] = {0xf4u};
    lib_u8 ss_descriptor[] = {0xffu,0xffu,0,0,0,0x12u,0xcfu,0};
    const lib_u8 *program = mask == VCPUINS_EXCEPT_GP ? gp_code :
        (mask == VCPUINS_EXCEPT_NP ? np_code : ss_code);
    lib_size bytes = mask == VCPUINS_EXCEPT_GP ? sizeof(gp_code) :
        (mask == VCPUINS_EXCEPT_NP ? sizeof(np_code) : sizeof(ss_code));
    lib_u16 selector = user_source ? 0x000bu : 0x0008u;
    lib_i32 failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
        IE_INTGATE_32);

    if (!failed) {
        if (user_source) failed |= !ie_prepare_user_code(&state);
        if (!user_source) test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_EFLAGS, 0x00000202u);
        if (mask == VCPUINS_EXCEPT_SS || mask == VCPUINS_EXCEPT_NP) {
            /* GDT already covers selector 18h. */
            failed |= !ie_write(&state, IE_GDT_BASE + 24u, ss_descriptor,
                sizeof(ss_descriptor));
        }
    }
    if (!failed) {
        failed |= !ie_install_gate(&state, vector, selector,
                (lib_u8)(0x80u | IE_INTGATE_32)) ||
            !ie_write(&state, IE_CODE_BASE, program, bytes) ||
            !ie_write(&state, IE_CODE_BASE + IE_HANDLER_OFFSET,
                user_source ? loop : halt, user_source ? sizeof(loop) :
                sizeof(halt)) || !ie_read(&state, IE_GDT_BASE + 13u,
                &access_before, sizeof(access_before)) || !(user_source ?
                ie_run_budget(&state, 0, &after, &diagnostic) : ie_run(&state,
                0, &after, &diagnostic)) || !ie_delivered_is(&diagnostic, mask,
                code) || after.cs.selector != selector ||
            after.esp != IE_STACK_BASE - 16u ||
            !ie_read(&state, IE_STACK_BASE - 16u, frame, sizeof(frame)) ||
            frame[0] != code || frame[1] != diagnostic.last_delivered_exception.point.eip ||
            frame[2] != selector || frame[3] != (user_source ? 0x00000302u :
                0x00000202u) ||
            !ie_read(&state, IE_GDT_BASE + 13u, &access_after,
                sizeof(access_after)) || access_after != (lib_u8)(access_before | 1u);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 ie_test_t305_fault_delivery(void)
{
    interrupt_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot after = {0};
    lib_u32 frame[4] = {0u,0u,0u,0u};
    static const lib_u8 code[] = {0xcdu,IE_VECTOR};
    lib_i32 failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
        IE_INTGATE_32);

    if (!failed) {
        test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_EFLAGS, 0x00000202u);
        failed |= !ie_install_gate(&state, IE_VECTOR, 0x0008u, 0x80u) ||
            !ie_install_gate(&state, 0x0du, 0x0008u,
                (lib_u8)(0x80u | IE_INTGATE_32)) ||
            !ie_write(&state, IE_CODE_BASE, code, sizeof(code)) ||
            !ie_write(&state, IE_CODE_BASE + IE_HANDLER_OFFSET,
                (const lib_u8[]){0xf4u}, 1u) || !ie_run(&state, 0, &after,
                &diagnostic) || !ie_delivered_is(&diagnostic,
                VCPUINS_EXCEPT_GP, IE_VECTOR * 8u + 2u) ||
            after.cs.selector != 0x0008u ||
            after.esp != IE_STACK_BASE - 16u ||
            !ie_read(&state, IE_STACK_BASE - 16u, frame, sizeof(frame)) ||
            frame[0] != IE_VECTOR * 8u + 2u || frame[1] != 0u ||
            frame[2] != 0x0008u || frame[3] != 0x00000202u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 ie_test_fault_delivery_failure(
    interrupt_entry_delivery_failure failure)
{
    interrupt_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot before = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_u8 access_before = 0u;
    lib_u8 access_after = 0u;
    lib_u32 stack_before[4] = {0u,0u,0u,0u};
    lib_u32 stack_after[4] = {0u,0u,0u,0u};
    lib_u8 not_present_access = 0x7au;
    static const lib_u8 code[] = {0x0fu,0x01u,0xf0u};
    lib_u8 gate_access = (lib_u8)(0x80u | IE_INTGATE_32);
    lib_i32 failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
        IE_INTGATE_32);

    if (!failed) {
        if (failure == INTERRUPT_ENTRY_DELIVERY_INVALID_GATE) gate_access = 0x80u;
        if (failure == INTERRUPT_ENTRY_DELIVERY_NONPRESENT_GATE)
            gate_access = IE_INTGATE_32;
        failed |= !ie_prepare_user_code(&state) ||
            !ie_install_gate(&state, 0x0du, 0x000bu, gate_access) ||
            !ie_write(&state, IE_CODE_BASE, code, sizeof(code));
        if (!failed && failure == INTERRUPT_ENTRY_DELIVERY_TARGET_NOT_PRESENT) {
            failed |= !ie_write(&state, IE_GDT_BASE + 13u, &not_present_access,
                sizeof(not_present_access));
        }
        if (!failed && failure == INTERRUPT_ENTRY_DELIVERY_STACK_LIMIT)
            failed |= !ie_set_stack_limit(&state, IE_STACK_BASE - 2u);
    }
    if (!failed) {
        before = ie_capture(state.machine);
        failed |= !ie_read(&state, IE_GDT_BASE + 13u, &access_before,
            sizeof(access_before)) || !ie_read(&state, IE_STACK_BASE - 16u,
            stack_before, sizeof(stack_before)) || !ie_run_budget(&state, 1, &after,
            &diagnostic) || !ie_fault_is(&diagnostic, VCPUINS_EXCEPT_DF, 0u) ||
            diagnostic.last_delivered_exception.valid ||
            diagnostic.delivered_exception_count != 0u ||
            !ie_read(&state, IE_GDT_BASE + 13u, &access_after,
                sizeof(access_after)) || !ie_read(&state, IE_STACK_BASE - 16u,
                stack_after, sizeof(stack_after)) ||
            !ie_delivery_state_equal(&before, &after) ||
            access_after != access_before || lib_memory_compare(stack_before, stack_after,
                sizeof(stack_before)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

int main(void)
{
    lib_i32 failed = !ie_test_success(IE_INTGATE_32, 0) ||
        !ie_test_success(IE_TRAPGATE_32, 1) ||
        !ie_test_prefix_keeps_gate_width() ||
        !ie_test_failure(INTERRUPT_ENTRY_NEGATIVE_IDT_LIMIT, VCPUINS_EXCEPT_DF,
            0u) ||
        !ie_test_failure(INTERRUPT_ENTRY_NEGATIVE_GATE_TYPE, VCPUINS_EXCEPT_DF,
            0u) ||
        !ie_test_failure(INTERRUPT_ENTRY_NEGATIVE_GATE_DPL, VCPUINS_EXCEPT_DF,
            0u) ||
        !ie_test_failure(INTERRUPT_ENTRY_NEGATIVE_GATE_NOT_PRESENT,
            VCPUINS_EXCEPT_DF, 0u) ||
        !ie_test_failure(INTERRUPT_ENTRY_NEGATIVE_CODE_TYPE, VCPUINS_EXCEPT_DF,
            0u) ||
        !ie_test_failure(INTERRUPT_ENTRY_NEGATIVE_CODE_NOT_PRESENT,
            VCPUINS_EXCEPT_DF, 0u) ||
        !ie_test_failure(INTERRUPT_ENTRY_NEGATIVE_CODE_LIMIT, VCPUINS_EXCEPT_DF,
            0u) ||
        !ie_test_failure(INTERRUPT_ENTRY_NEGATIVE_STACK_LIMIT,
            VCPUINS_EXCEPT_DF, 0u) ||
        !ie_test_software_frontends() ||
        !ie_test_external_origin(0, 0) || !ie_test_external_origin(1, 0) ||
        !ie_test_external_origin(0, 1) || !ie_test_external_origin(1, 1);

    failed |= !ie_test_fault_delivery(VCPUINS_EXCEPT_GP, 0x0du, 0u, 1);
    failed |= !ie_test_fault_delivery(VCPUINS_EXCEPT_NP, 0x0bu, 0x0018u, 0);
    failed |= !ie_test_fault_delivery(VCPUINS_EXCEPT_SS, 0x0cu, 0x0018u, 0);
    failed |= !ie_test_t305_fault_delivery();
    failed |= !ie_test_fault_delivery_failure(
        INTERRUPT_ENTRY_DELIVERY_INVALID_GATE);
    failed |= !ie_test_fault_delivery_failure(
        INTERRUPT_ENTRY_DELIVERY_NONPRESENT_GATE);
    failed |= !ie_test_fault_delivery_failure(
        INTERRUPT_ENTRY_DELIVERY_TARGET_NOT_PRESENT);
    failed |= !ie_test_fault_delivery_failure(
        INTERRUPT_ENTRY_DELIVERY_STACK_LIMIT);

    if (failed) return 1;
    lib_c_printf("M5:T305:INTERRUPT-ENTRY:OK\n");
    lib_c_printf("M5:T308:S2:SAME-CPL-ERROR-DELIVERY:OK\n");
    lib_c_printf("M5:T539:S66:INT-ENTRY:OK\n");
    return 0;
}
