#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "core/machine/pic.h"
#include "../support/core_machine_cpu_fixture.h"

/* T337_REAL_UD_VECTOR6_DELIVERY: this owner installs and observes vector 6. */

#define TF_DB_S60_GDT_POINTER 0x0100u
#define TF_DB_S60_GDT 0x0300u
#define TF_DB_S60_IDT 0x0400u
#define TF_DB_S60_CODE 0x2000u

typedef struct tf_db_s60_machine {
    core_machine *machine;
} tf_db_s60_machine;

static C_VOID tf_db_s60_reset(C_VOID *opaque)
{
    tf_db_s60_machine *state = (tf_db_s60_machine *)opaque;

    if (state != STD_NULL) {
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
    }
}

static const core_machine_execution_provider tf_db_s60_execution_provider = {
    tf_db_s60_reset, STD_NULL
};

static C_INT tf_db_s60_prepare(tf_db_s60_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (!test_core_machine_fixture_create_bind_freeze_reset(&config,
            &tf_db_s60_execution_provider, state, &state->machine)) {
        if (state->machine != STD_NULL) core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT tf_db_s60_run(tf_db_s60_machine *state,
    const type_unsigned_8 *code, STD_SIZE_T code_bytes, type_unsigned_32 code_address,
    core_machine_run_result *out_result, t_cpu *out_cpu,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    type_status status;

    if (state == STD_NULL || state->machine == STD_NULL || code == STD_NULL ||
        out_result == STD_NULL || out_cpu == STD_NULL || out_diagnostic == STD_NULL ||
        core_machine_memory_write(state->machine, code_address, code,
            code_bytes) != TYPE_STATUS_OK) return 0;
    test_core_machine_fixture_resume_after_halt_at(state->machine, code_address -
        state->machine->executor_cpu.data.cs.base);
    status = core_machine_run(state->machine, (core_machine_run_budget){ 3u, 0u },
        out_result);
    if (core_machine_get_cpu_diagnostic(state->machine, out_diagnostic) !=
        TYPE_STATUS_OK) return 0;
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return status == TYPE_STATUS_OK;
}

static C_INT tf_db_s60_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return STD_MEMCMP(&before->data.es, &after->data.es, sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.cs, &after->data.cs, sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs, sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static C_INT tf_db_s60_install_real_vector(tf_db_s60_machine *state)
{
    static const type_unsigned_8 handler[] = { 0xf4u };
    static const type_unsigned_8 vector[] = { 0x00u, 0x01u, 0x00u, 0x00u };

    return core_machine_memory_write(state->machine, 4u, vector, sizeof(vector)) ==
        TYPE_STATUS_OK && core_machine_memory_write(state->machine, 0x0100u,
            handler, sizeof(handler)) == TYPE_STATUS_OK;
}

static C_INT tf_db_s60_install_real_ud_vector(tf_db_s60_machine *state)
{
    static const type_unsigned_8 handler[] = { 0xf4u };
    static const type_unsigned_8 vector[] = { 0x00u, 0x01u, 0x00u, 0x00u };

    return core_machine_memory_write(state->machine, 0x18u, vector,
        sizeof(vector)) == TYPE_STATUS_OK && core_machine_memory_write(
            state->machine, 0x0100u, handler, sizeof(handler)) == TYPE_STATUS_OK;
}

static C_INT tf_db_s60_install_real_irq0_vector(tf_db_s60_machine *state)
{
    static const type_unsigned_8 handler[] = { 0xf4u };
    static const type_unsigned_8 vector[] = { 0x10u, 0x01u, 0x00u, 0x00u };

    return core_machine_memory_write(state->machine, 0x80u, vector,
        sizeof(vector)) == TYPE_STATUS_OK && core_machine_memory_write(
            state->machine, 0x0110u, handler, sizeof(handler)) == TYPE_STATUS_OK;
}

static C_INT tf_db_s60_boot_protected(tf_db_s60_machine *state)
{
    static const type_unsigned_8 gdt_pointer[] = { 0x17u, 0x00u, 0x00u, 0x03u, 0u, 0u };
    static const type_unsigned_8 gdt[] = {
        0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x20u, 0u, 0x9au, 0u, 0u,
        0xffu, 0xffu, 0u, 0u, 0u, 0x92u, 0u, 0u
    };
    static const type_unsigned_8 bootstrap[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u, 0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u, 0x8eu, 0xc0u,
        0x8eu, 0xd0u, 0xbcu, 0x00u, 0x80u,
        0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const type_unsigned_8 halt[] = { 0xf4u };
    core_machine_run_result result;

    if (state == STD_NULL || state->machine == STD_NULL ||
        core_machine_memory_write(state->machine, TF_DB_S60_GDT_POINTER,
            gdt_pointer, sizeof(gdt_pointer)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, TF_DB_S60_GDT, gdt,
            sizeof(gdt)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, 0u, bootstrap,
            sizeof(bootstrap)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, TF_DB_S60_CODE, halt,
            sizeof(halt)) != TYPE_STATUS_OK ||
        core_machine_run(state->machine, (core_machine_run_budget){ 64u, 0u },
            &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) return 0;
    return 1;
}

static C_INT tf_db_s60_install_protected_vector(tf_db_s60_machine *state)
{
    static const type_unsigned_8 handler[] = { 0xf4u };
    type_unsigned_8 gate[8u] = { 0u };

    gate[0] = 0x00u;
    gate[1] = 0x01u;
    gate[2] = 0x08u;
    gate[5] = 0x8eu;
    state->machine->executor_cpu.data.idtr.flagValid = TYPE_TRUE;
    state->machine->executor_cpu.data.idtr.sregtype = SREG_IDTR;
    state->machine->executor_cpu.data.idtr.base = TF_DB_S60_IDT;
    state->machine->executor_cpu.data.idtr.limit = 0x000fu;
    return core_machine_memory_write(state->machine, TF_DB_S60_IDT + 8u, gate,
        sizeof(gate)) == TYPE_STATUS_OK && core_machine_memory_write(
            state->machine, TF_DB_S60_CODE + 0x100u, handler,
            sizeof(handler)) == TYPE_STATUS_OK;
}

static C_INT tf_db_s60_frame_real(tf_db_s60_machine *state, const t_cpu *after,
    type_unsigned_16 expected_ip, type_unsigned_16 expected_flags,
    core_machine_cpu_profile profile)
{
    type_unsigned_16 frame[3u] = { 0u, 0u, 0u };
    const type_unsigned_16 known_mask = profile < CORE_MACHINE_CPU_PROFILE_80286 ?
        0x0fd5u : 0x7fd5u;
    const type_unsigned_16 expected_image = TYPE_MASK_UNSIGNED_16(
        (expected_flags & ~VCPU_EFLAGS_RESERVED) | 0x02u);

    return core_machine_memory_read_physical(&state->machine->executor_memory,
        after->data.ss.base + (type_unsigned_16)after->data.esp,
        (type_virtual_address)frame, sizeof(frame)) == TYPE_STATUS_OK &&
        frame[0] == expected_ip && frame[1] == 0u &&
        (frame[2] & known_mask) == (expected_image & known_mask) &&
        (profile != CORE_MACHINE_CPU_PROFILE_80386 || (frame[2] & 0x8000u) == 0u);
}

static C_INT tf_db_s60_test_real(C_VOID)
{
    static const type_unsigned_8 code[] = { 0x90u };
    tf_db_s60_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    C_INT failed = !tf_db_s60_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) failed = !tf_db_s60_install_real_vector(&state);
    if (!failed) {
        state.machine->executor_cpu.data.esp = 0x8000u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_TF | VCPU_EFLAGS_IF |
            VCPU_EFLAGS_CF | 0x8000u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !tf_db_s60_run(&state, code, sizeof(code), 0u, &result, &after,
            &diagnostic);
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            diagnostic.first_fault.valid || after.data.eip != 0x0101u ||
            TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_TF) ||
            TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_IF) ||
            after.data.eax != before.data.eax || after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi || !tf_db_s60_sregs_same(&before, &after) ||
            !tf_db_s60_frame_real(&state, &after, 1u,
                (type_unsigned_16)before.data.eflags, CORE_MACHINE_CPU_PROFILE_80386);
    }
    if (state.machine != STD_NULL) core_machine_destroy(state.machine);
    return failed;
}

static C_INT tf_db_s60_test_protected_attributes(C_VOID)
{
    static const type_unsigned_8 prefixes[][2] = {
        { 0u, 0u }, { 0x66u, 0u }, { 0x67u, 0u }, { 0x66u, 0x67u }
    };
    C_INT failed = 0;
    STD_SIZE_T i;

    for (i = 0u; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
        tf_db_s60_machine state;
        type_unsigned_8 code[4u] = { 0u, 0u, 0x90u, 0u };
        type_unsigned_8 length = prefixes[i][1] == 0u ? (prefixes[i][0] == 0u ? 1u : 2u) : 3u;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        type_unsigned_32 frame[3u] = { 0u, 0u, 0u };

        if (!tf_db_s60_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !tf_db_s60_boot_protected(&state) ||
            !tf_db_s60_install_protected_vector(&state)) return 1;
        if (prefixes[i][0] != 0u) code[0] = prefixes[i][0];
        if (prefixes[i][1] != 0u) code[1] = prefixes[i][1];
        code[length - 1u] = 0x90u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_TF | VCPU_EFLAGS_IF |
            VCPU_EFLAGS_CF;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !tf_db_s60_run(&state, code, length, TF_DB_S60_CODE, &result,
            &after, &diagnostic);

        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            diagnostic.first_fault.valid || after.data.eip != 0x101u ||
            TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_TF) ||
            TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_IF) ||
            !tf_db_s60_sregs_same(&before, &after) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                (type_virtual_address)frame, sizeof(frame)) != TYPE_STATUS_OK ||
            frame[0] != length || frame[1] != before.data.cs.selector ||
            frame[2] != before.data.eflags;

        core_machine_destroy(state.machine);
    }
    return failed;
}

static C_INT tf_db_s60_expect_ud_no_trap(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, STD_SIZE_T code_bytes)
{
    tf_db_s60_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_status status;
    C_INT failed = !tf_db_s60_prepare(&state, profile);

    if (!failed) failed = !tf_db_s60_install_real_vector(&state) ||
        !tf_db_s60_install_real_ud_vector(&state);
    if (!failed) {
        state.machine->executor_cpu.data.esp = 0x8000u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_TF | VCPU_EFLAGS_IF |
            VCPU_EFLAGS_CF;
        failed |= core_machine_memory_write(state.machine, 0u, code, code_bytes) !=
            TYPE_STATUS_OK;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        status = core_machine_run(state.machine, (core_machine_run_budget){ 2u, 0u },
            &result);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK || status != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            diagnostic.first_fault.valid || !diagnostic.last_delivered_exception.valid ||
            !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_UD) || after.data.eip != 0x0101u ||
            after.data.esp != ((before.data.esp & 0xffff0000u) |
                (type_unsigned_16)(before.data.esp - 6u)) ||
            after.data.eflags != (before.data.eflags &
                ~(VCPU_EFLAGS_IF | VCPU_EFLAGS_TF)) || after.data.eax !=
            before.data.eax || after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi || !tf_db_s60_sregs_same(&before,
                &after) || !tf_db_s60_frame_real(&state, &after, 0u,
                (type_unsigned_16)before.data.eflags, profile);
    }
    if (state.machine != STD_NULL) core_machine_destroy(state.machine);
    return failed;
}

static C_INT tf_db_s60_test_rejections(C_VOID)
{
    static const type_unsigned_8 prefix_66[] = { 0x66u, 0x90u };
    static const type_unsigned_8 prefix_67[] = { 0x67u, 0x90u };
    static const type_unsigned_8 prefixes[] = { 0x66u, 0x67u, 0x90u };
    static const type_unsigned_8 lock[] = { 0xf0u, 0x90u };
    static const type_unsigned_8 lock_66[] = { 0xf0u, 0x66u, 0x90u };
    static const type_unsigned_8 lock_67[] = { 0xf0u, 0x67u, 0x90u };
    static const type_unsigned_8 lock_prefixes[] = { 0xf0u, 0x66u, 0x67u, 0x90u };
    const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    C_INT failed = 0;
    STD_SIZE_T i;

    for (i = 0u; i < sizeof(legacy) / sizeof(legacy[0]); ++i) {
        failed |= tf_db_s60_expect_ud_no_trap(legacy[i], prefix_66,
            sizeof(prefix_66));
        failed |= tf_db_s60_expect_ud_no_trap(legacy[i], prefix_67,
            sizeof(prefix_67));
        failed |= tf_db_s60_expect_ud_no_trap(legacy[i], prefixes,
            sizeof(prefixes));
    }
    failed |= tf_db_s60_expect_ud_no_trap(CORE_MACHINE_CPU_PROFILE_80386, lock,
        sizeof(lock));
    failed |= tf_db_s60_expect_ud_no_trap(CORE_MACHINE_CPU_PROFILE_80386, lock_66,
        sizeof(lock_66));
    failed |= tf_db_s60_expect_ud_no_trap(CORE_MACHINE_CPU_PROFILE_80386, lock_67,
        sizeof(lock_67));
    failed |= tf_db_s60_expect_ud_no_trap(CORE_MACHINE_CPU_PROFILE_80386,
        lock_prefixes, sizeof(lock_prefixes));
    return failed;
}

static C_INT tf_db_s60_test_hardware_real(C_VOID)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    static const type_unsigned_8 write[] = {
        0xc6u, 0x06u, 0x00u, 0x10u, 0x5au
    };
    static const type_unsigned_8 rf_code[] = { 0x90u, 0xf4u };
    tf_db_s60_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 value = 0u;
    C_INT failed = !tf_db_s60_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) failed = !tf_db_s60_install_real_vector(&state);
    if (!failed) {
        state.machine->executor_cpu.data.esp = 0x8000u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_IF | VCPU_EFLAGS_CF;
        state.machine->executor_cpu.data.dr0 = 0u;
        state.machine->executor_cpu.data.dr6 = 0u;
        state.machine->executor_cpu.data.dr7 = 0x00000001u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !tf_db_s60_run(&state, nop, sizeof(nop), 0u, &result, &after,
            &diagnostic) || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            diagnostic.first_fault.valid || !diagnostic.last_delivered_exception.valid ||
            diagnostic.last_delivered_exception.exception_mask != VCPUINS_EXCEPT_DB ||
            after.data.eip != 0x0101u || (after.data.dr6 & 1u) == 0u ||
            after.data.eax != before.data.eax || after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi || !tf_db_s60_sregs_same(&before,
                &after) || !tf_db_s60_frame_real(&state, &after, 0u,
                (type_unsigned_16)before.data.eflags, CORE_MACHINE_CPU_PROFILE_80386);
    }
    if (state.machine != STD_NULL) core_machine_destroy(state.machine);
    if (failed) return 1;

    failed = !tf_db_s60_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) failed = !tf_db_s60_install_real_vector(&state);
    if (!failed) {
        static const type_unsigned_8 read[] = { 0xa1u, 0x00u, 0x10u };
        static const type_unsigned_8 source[] = { 0x5au, 0x34u, 0x56u, 0x78u };

        state.machine->executor_cpu.data.esp = 0x8000u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_IF |
            VCPU_EFLAGS_CF;
        state.machine->executor_cpu.data.dr0 = 0x1001u;
        state.machine->executor_cpu.data.dr1 = 0x1001u;
        state.machine->executor_cpu.data.dr6 = 0u;
        state.machine->executor_cpu.data.dr7 = 0x00ff0008u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_write(state.machine, 0x1000u, source,
                sizeof(source)) != TYPE_STATUS_OK || !tf_db_s60_run(&state,
                read, sizeof(read), 0x0200u, &result, &after, &diagnostic) ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            diagnostic.first_fault.valid || !diagnostic.last_delivered_exception.valid ||
            diagnostic.last_delivered_exception.exception_mask != VCPUINS_EXCEPT_DB ||
            after.data.eip != 0x0101u || (after.data.dr6 & 3u) != 2u ||
            (after.data.eax & 0xffffu) != 0x345au ||
            !tf_db_s60_frame_real(&state, &after, 0x0200u + sizeof(read),
                (type_unsigned_16)before.data.eflags, CORE_MACHINE_CPU_PROFILE_80386);
    }
    if (state.machine != STD_NULL) core_machine_destroy(state.machine);
    if (failed) return 1;

    failed = !tf_db_s60_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) failed = !tf_db_s60_install_real_vector(&state);
    if (!failed) {
        state.machine->executor_cpu.data.esp = 0x8000u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_TF | VCPU_EFLAGS_IF |
            VCPU_EFLAGS_CF;
        state.machine->executor_cpu.data.dr0 = 0x1000u;
        state.machine->executor_cpu.data.dr6 = 0u;
        state.machine->executor_cpu.data.dr7 = 0x00010101u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !tf_db_s60_run(&state, write, sizeof(write), 0x0200u, &result,
            &after, &diagnostic) || result.reason !=
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT || diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            diagnostic.last_delivered_exception.exception_mask != VCPUINS_EXCEPT_DB ||
            after.data.eip != 0x0101u || (after.data.dr6 &
                (1u | 0x00004000u)) != (1u | 0x00004000u) ||
            core_machine_memory_read(state.machine, 0x1000u, &value,
                sizeof(value)) != TYPE_STATUS_OK || value != 0x5au ||
            !tf_db_s60_frame_real(&state, &after, 0x0200u + sizeof(write),
                (type_unsigned_16)before.data.eflags, CORE_MACHINE_CPU_PROFILE_80386);
    }
    if (state.machine != STD_NULL) core_machine_destroy(state.machine);
    if (failed) return 1;

    failed = !tf_db_s60_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) {
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_RF | VCPU_EFLAGS_IF;
        state.machine->executor_cpu.data.dr0 = 0u;
        state.machine->executor_cpu.data.dr6 = 0u;
        state.machine->executor_cpu.data.dr7 = 0x00000001u;
        failed |= !tf_db_s60_run(&state, rf_code, sizeof(rf_code), 0u, &result,
            &after, &diagnostic) || result.reason !=
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT || diagnostic.first_fault.valid ||
            diagnostic.last_delivered_exception.valid || after.data.eip !=
                sizeof(rf_code) || TYPE_GET_BIT(after.data.eflags,
                    VCPU_EFLAGS_RF) || after.data.dr6 != 0u;
    }
    if (state.machine != STD_NULL) core_machine_destroy(state.machine);
    return failed;
}

static C_INT tf_db_s60_test_hardware_protected(C_VOID)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    tf_db_s60_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_32 frame[3u] = { 0u, 0u, 0u };
    C_INT failed = !tf_db_s60_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) failed = !tf_db_s60_boot_protected(&state) ||
        !tf_db_s60_install_protected_vector(&state);
    if (!failed) {
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_IF | VCPU_EFLAGS_CF;
        state.machine->executor_cpu.data.dr0 = TF_DB_S60_CODE;
        state.machine->executor_cpu.data.dr6 = 0u;
        state.machine->executor_cpu.data.dr7 = 0x00000001u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !tf_db_s60_run(&state, nop, sizeof(nop), TF_DB_S60_CODE,
            &result, &after, &diagnostic) || result.reason !=
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT || diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            diagnostic.last_delivered_exception.exception_mask != VCPUINS_EXCEPT_DB ||
            after.data.eip != 0x101u || (after.data.dr6 & 1u) == 0u ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + after.data.esp, (type_virtual_address)frame,
                sizeof(frame)) != TYPE_STATUS_OK || frame[0] != 0u || frame[1] !=
                before.data.cs.selector || frame[2] != (before.data.eflags |
                    VCPU_EFLAGS_RF);
    }
    if (state.machine != STD_NULL) core_machine_destroy(state.machine);
    return failed;
}

static C_INT tf_db_s60_test_tf_priority_over_irq(C_VOID)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    tf_db_s60_machine state;
    core_machine_pic_irq_source irq;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    type_unsigned_16 frame[3u] = { 0u, 0u, 0u };
    C_INT failed = !tf_db_s60_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) failed = !tf_db_s60_install_real_vector(&state) ||
        !tf_db_s60_install_real_irq0_vector(&state);
    if (!failed) {
        state.machine->executor_cpu.data.esp = 0x8000u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_TF |
            VCPU_EFLAGS_IF;
        STD_MEMSET(&irq, TYPE_ZERO_8, sizeof(irq));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed |= !tf_db_s60_run(&state, nop, sizeof(nop), 0u, &result, &after,
            &diagnostic) || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            diagnostic.first_fault.valid || !diagnostic.last_delivered_exception.valid ||
            diagnostic.last_delivered_exception.exception_mask != VCPUINS_EXCEPT_DB ||
            (after.data.dr6 & 0x00004000u) == 0u || after.data.eip != 0x0101u ||
            TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || !TYPE_GET_BIT(
                    state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                (type_virtual_address)frame, sizeof(frame)) != TYPE_STATUS_OK ||
            frame[0] != 1u || frame[1] != 0u;
    }
    if (state.machine != STD_NULL) core_machine_destroy(state.machine);
    return failed;
}

C_INT main(C_VOID)
{
    C_INT real = tf_db_s60_test_real();
    C_INT protected_attributes = tf_db_s60_test_protected_attributes();
    C_INT rejections = tf_db_s60_test_rejections();
    C_INT hardware_real = tf_db_s60_test_hardware_real();
    C_INT hardware_protected = tf_db_s60_test_hardware_protected();
    C_INT priority = tf_db_s60_test_tf_priority_over_irq();
    if (real || protected_attributes || rejections || hardware_real ||
        hardware_protected || priority) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S60:TF-DB failed real=%d protected=%d reject=%d hardware-real=%d hardware-protected=%d priority=%d\n", real, protected_attributes, rejections, hardware_real, hardware_protected, priority);
        return 1;
    }
    STD_PRINTF("M5:T316:S60:TF-DB:OK\n");
    return 0;
}
