#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

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
    tf_db_s60_reset, STD_NULL, STD_NULL
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
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK ||
        core_machine_bind_execution_provider(state->machine,
            &tf_db_s60_execution_provider, state) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(state->machine) != TYPE_STATUS_OK ||
        core_machine_reset(state->machine) != TYPE_STATUS_OK) {
        if (state->machine != STD_NULL) core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT tf_db_s60_run(tf_db_s60_machine *state,
    const uint8_t *code, STD_SIZE_T code_bytes, uint32_t code_address,
    core_machine_run_result *out_result, t_cpu *out_cpu,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    type_status status;

    if (state == STD_NULL || state->machine == STD_NULL || code == STD_NULL ||
        out_result == STD_NULL || out_cpu == STD_NULL || out_diagnostic == STD_NULL ||
        core_machine_memory_write(state->machine, code_address, code,
            code_bytes) != TYPE_STATUS_OK) return 0;
    test_core_machine_fixture_resume_after_halt_at(state->machine, 0u);
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
    static const uint8_t handler[] = { 0xf4u };
    static const uint8_t vector[] = { 0x00u, 0x01u, 0x00u, 0x00u };

    return core_machine_memory_write(state->machine, 4u, vector, sizeof(vector)) ==
        TYPE_STATUS_OK && core_machine_memory_write(state->machine, 0x0100u,
            handler, sizeof(handler)) == TYPE_STATUS_OK;
}

static C_INT tf_db_s60_boot_protected(tf_db_s60_machine *state)
{
    static const uint8_t gdt_pointer[] = { 0x17u, 0x00u, 0x00u, 0x03u, 0u, 0u };
    static const uint8_t gdt[] = {
        0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x20u, 0u, 0x9au, 0u, 0u,
        0xffu, 0xffu, 0u, 0u, 0u, 0x92u, 0u, 0u
    };
    static const uint8_t bootstrap[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u, 0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u, 0x8eu, 0xc0u,
        0x8eu, 0xd0u, 0xbcu, 0x00u, 0x80u,
        0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const uint8_t halt[] = { 0xf4u };
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
    static const uint8_t handler[] = { 0xf4u };
    uint8_t gate[8u] = { 0u };

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
    uint16_t expected_ip, uint16_t expected_flags)
{
    uint16_t frame[3u] = { 0u, 0u, 0u };

    return core_machine_memory_read_physical(&state->machine->executor_memory,
        after->data.ss.base + (uint16_t)after->data.esp,
        (type_virtual_address)frame, sizeof(frame)) == TYPE_STATUS_OK &&
        frame[0] == expected_ip && frame[1] == 0u && frame[2] == expected_flags;
}

static C_INT tf_db_s60_test_real(C_VOID)
{
    static const uint8_t code[] = { 0x90u };
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
            VCPU_EFLAGS_CF;
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
            !tf_db_s60_frame_real(&state, &after, 1u, (uint16_t)before.data.eflags);
    }
    if (state.machine != STD_NULL) core_machine_destroy(state.machine);
    return failed;
}

static C_INT tf_db_s60_test_protected_attributes(C_VOID)
{
    static const uint8_t prefixes[][2] = {
        { 0u, 0u }, { 0x66u, 0u }, { 0x67u, 0u }, { 0x66u, 0x67u }
    };
    C_INT failed = 0;
    STD_SIZE_T i;

    for (i = 0u; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
        tf_db_s60_machine state;
        uint8_t code[4u] = { 0u, 0u, 0x90u, 0u };
        uint8_t length = prefixes[i][1] == 0u ? (prefixes[i][0] == 0u ? 1u : 2u) : 3u;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        uint32_t frame[3u] = { 0u, 0u, 0u };

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
                after.data.ss.base + (uint16_t)after.data.esp,
                (type_virtual_address)frame, sizeof(frame)) != TYPE_STATUS_OK ||
            frame[0] != length || frame[1] != before.data.cs.selector ||
            frame[2] != before.data.eflags;

        core_machine_destroy(state.machine);
    }
    return failed;
}

static C_INT tf_db_s60_expect_ud_no_trap(core_machine_cpu_profile profile,
    const uint8_t *code, STD_SIZE_T code_bytes)
{
    tf_db_s60_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_status status;
    C_INT failed = !tf_db_s60_prepare(&state, profile);

    if (!failed) failed = !tf_db_s60_install_real_vector(&state);
    if (!failed) {
        state.machine->executor_cpu.data.esp = 0x8000u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_TF | VCPU_EFLAGS_IF |
            VCPU_EFLAGS_CF;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_write(state.machine, 0u, code, code_bytes) !=
            TYPE_STATUS_OK;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        status = core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
            &result);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK || status != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || after.data.eip == 0x0101u;
    }
    if (state.machine != STD_NULL) core_machine_destroy(state.machine);
    return failed;
}

static C_INT tf_db_s60_test_rejections(C_VOID)
{
    static const uint8_t prefix_66[] = { 0x66u, 0x90u };
    static const uint8_t prefix_67[] = { 0x67u, 0x90u };
    static const uint8_t prefixes[] = { 0x66u, 0x67u, 0x90u };
    static const uint8_t lock[] = { 0xf0u, 0x90u };
    static const uint8_t lock_66[] = { 0xf0u, 0x66u, 0x90u };
    static const uint8_t lock_67[] = { 0xf0u, 0x67u, 0x90u };
    static const uint8_t lock_prefixes[] = { 0xf0u, 0x66u, 0x67u, 0x90u };
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

C_INT main(C_VOID)
{
    C_INT real = tf_db_s60_test_real();
    C_INT protected_attributes = tf_db_s60_test_protected_attributes();
    C_INT rejections = tf_db_s60_test_rejections();
    if (real || protected_attributes || rejections) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S60:TF-DB failed real=%d protected=%d reject=%d\n", real, protected_attributes, rejections);
        return 1;
    }
    STD_PRINTF("M5:T316:S60:TF-DB:OK\n");
    return 0;
}