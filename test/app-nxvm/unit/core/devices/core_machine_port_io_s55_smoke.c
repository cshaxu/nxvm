#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/pic.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct port_io_s55_port {
    lib_u32 input;
    lib_u32 last_write;
    lib_u16 last_port;
    lib_u32 reads;
    lib_u32 writes;
    lib_i32 fail;
} port_io_s55_port;

typedef struct port_io_s55_machine {
    core_machine *machine;
    port_io_s55_port port;
} port_io_s55_machine;

static lib_status port_io_s55_read(void *owner, lib_u16 port,
    lib_u32 *value)
{
    port_io_s55_port *state = (port_io_s55_port *)owner;

    if (state == LIB_NULL || value == LIB_NULL || state->fail)
        return LIB_STATUS_INVALID_ARGUMENT;
    ++state->reads;
    state->last_port = port;
    *value = state->input;
    return LIB_STATUS_OK;
}

static lib_status port_io_s55_write(void *owner, lib_u16 port,
    lib_u32 value)
{
    port_io_s55_port *state = (port_io_s55_port *)owner;

    if (state == LIB_NULL || state->fail)
        return LIB_STATUS_INVALID_ARGUMENT;
    ++state->writes;
    state->last_port = port;
    state->last_write = value;
    return LIB_STATUS_OK;
}

static const core_machine_port_provider port_io_s55_provider = {
    port_io_s55_read,
    port_io_s55_write
};

static void port_io_s55_reset(void *opaque)
{
    port_io_s55_machine *state = (port_io_s55_machine *)opaque;

    if (state != LIB_NULL)
        (void)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider port_io_s55_execution_provider = {
    port_io_s55_reset,
    LIB_NULL
};

static lib_i32 port_io_s55_prepare(port_io_s55_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    lib_memory_set(state, 0, sizeof(*state));
    return core_machine_create(&config, &state->machine) == LIB_STATUS_OK &&
        core_machine_install_port_provider(state->machine, 0x005au, 0x005au,
            &port_io_s55_provider, &state->port) == LIB_STATUS_OK &&
        core_machine_install_port_provider(state->machine, 0x00e0u, 0x00e0u,
            &port_io_s55_provider, &state->port) == LIB_STATUS_OK &&
        test_core_machine_fixture_bind_freeze_reset(state->machine,
            &port_io_s55_execution_provider, state) &&
        test_core_machine_fixture_prepare_real_mode_execution(
                state->machine, 0u);
}

static void port_io_s55_seed(t_cpu *cpu)
{
    cpu->data.eax = 0xa1a1b2b2u;
    cpu->data.ecx = 0xc3c3d4d4u;
    cpu->data.edx = 0xe5e500e0u;
    cpu->data.ebx = 0xf6f60707u;
    cpu->data.esp = 0x00008000u;
    cpu->data.ebp = 0x08080909u;
    cpu->data.esi = 0x10101111u;
    cpu->data.edi = 0x12121313u;
    cpu->data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
}

static lib_i32 port_io_s55_gprs_same_except_eax(const t_cpu *before,
    const t_cpu *after)
{
    return before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi;
}

static lib_i32 port_io_s55_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return lib_memory_compare(&before->data.es, &after->data.es,
            sizeof(before->data.es)) == 0 &&
        lib_memory_compare(&before->data.cs, &after->data.cs,
            sizeof(before->data.cs)) == 0 &&
        lib_memory_compare(&before->data.ss, &after->data.ss,
            sizeof(before->data.ss)) == 0 &&
        lib_memory_compare(&before->data.ds, &after->data.ds,
            sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs,
            sizeof(before->data.fs)) == 0 &&
        lib_memory_compare(&before->data.gs, &after->data.gs,
            sizeof(before->data.gs)) == 0;
}

static lib_i32 port_io_s55_success(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_i32 input, lib_u8 width,
    lib_u16 port)
{
    port_io_s55_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    lib_status status;
    lib_u32 expected;
    lib_i32 failed = !port_io_s55_prepare(&state, profile);

    if (!failed) {
        port_io_s55_seed(&state.machine->executor_cpu);
        state.port.input = 0x11223344u;
        before = state.machine->executor_cpu;
        failed = core_machine_memory_write(state.machine, 0u, code, bytes) !=
            LIB_STATUS_OK;
        if (!failed) {
            status = core_machine_run(state.machine,
                (core_machine_run_budget){ 1u, 0u }, &result);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed = core_machine_get_cpu_diagnostic(state.machine,
                &diagnostic) != LIB_STATUS_OK;
            failed |= status != LIB_STATUS_OK;
            failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
            failed |= diagnostic.first_fault.valid;
            failed |= after.data.eip != bytes;
            failed |= after.data.ecx != before.data.ecx;
            failed |= after.data.edx != before.data.edx;
            failed |= after.data.ebx != before.data.ebx;
            failed |= after.data.esp != before.data.esp;
            failed |= after.data.ebp != before.data.ebp;
            failed |= after.data.esi != before.data.esi;
            failed |= after.data.edi != before.data.edi;
            failed |= after.data.eflags != before.data.eflags;
            failed |= !port_io_s55_sregs_same(&before, &after);
            if (input) {
                expected = width == 1u ? (before.data.eax & 0xffffff00u) |
                    0x44u : width == 2u ? (before.data.eax & 0xffff0000u) |
                    0x3344u : 0x11223344u;
                failed |= state.port.reads != 1u || state.port.writes != 0u;
                failed |= state.port.last_port != port;
                failed |= after.data.eax != expected;
            }
            else {
                expected = width == 1u ? before.data.eax & 0xffu :
                    width == 2u ? before.data.eax & 0xffffu : before.data.eax;
                failed |= state.port.reads != 0u || state.port.writes != 1u;
                failed |= state.port.last_port != port;
                failed |= state.port.last_write != expected;
                failed |= after.data.eax != before.data.eax;
            }
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 port_io_s55_test_default_forms(void)
{
    static const lib_u8 codes[][2] = {
        { 0xe4u, 0x5au }, { 0xe5u, 0x5au }, { 0xe6u, 0x5au },
        { 0xe7u, 0x5au }, { 0xecu, 0u }, { 0xedu, 0u },
        { 0xeeu, 0u }, { 0xefu, 0u }
    };
    static const lib_u8 input[] = { 1u, 1u, 0u, 0u, 1u, 1u, 0u, 0u };
    static const lib_u8 widths[] = { 1u, 2u, 1u, 2u, 1u, 2u, 1u, 2u };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile;
    lib_u8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
            ++profile) {
        for (form = 0u; form != sizeof(input); ++form) {
            if (!port_io_s55_success(profiles[profile], codes[form],
                    form < 4u ? 2u : 1u, input[form], widths[form],
                    form < 4u ? 0x005au : 0x00e0u)) {
                fprintf(stderr, "S55 default p=%u f=%u\n", profile,
                    form);
                return 0;
            }
        }
    }
    return 1;
}

static lib_i32 port_io_s55_test_386_attributes(void)
{
    static const lib_u8 opcodes[] = {
        0xe4u, 0xe5u, 0xe6u, 0xe7u, 0xecu, 0xedu, 0xeeu, 0xefu
    };
    static const lib_u8 input[] = {
        1u, 1u, 0u, 0u, 1u, 1u, 0u, 0u
    };
    lib_u8 attribute;
    lib_u8 form;

    for (attribute = 0u; attribute != 3u; ++attribute) {
        for (form = 0u; form != sizeof(opcodes); ++form) {
            lib_u8 code[4];
            lib_u8 prefix_bytes = attribute == 2u ? 2u : 1u;
            lib_u8 code_bytes;
            lib_u8 width = form % 2u == 0u ? 1u :
                (attribute == 0u || attribute == 2u ? 4u : 2u);

            if (attribute == 0u)
                code[0] = 0x66u;
            else if (attribute == 1u)
                code[0] = 0x67u;
            else {
                code[0] = 0x66u;
                code[1] = 0x67u;
            }
            code[prefix_bytes] = opcodes[form];
            if (form < 4u)
                code[prefix_bytes + 1u] = 0x5au;
            code_bytes = prefix_bytes + 1u + (form < 4u ? 1u : 0u);
            if (!port_io_s55_success(CORE_MACHINE_CPU_PROFILE_80386, code,
                    code_bytes, input[form], width,
                    form < 4u ? 0x005au : 0x00e0u))
                return 0;
        }
    }
    return 1;
}

static lib_i32 port_io_s55_test_provider_error(void)
{
    static const lib_u8 in_code[] = { 0xe4u, 0x5au };
    static const lib_u8 out_code[] = { 0xe7u, 0x5au };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_size profile;
    lib_u8 form;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        for (form = 0u; form != 2u; ++form) {
            port_io_s55_machine state;
            core_machine_run_result result;
            core_machine_cpu_diagnostic diagnostic;
            t_cpu before;
            t_cpu after;
            lib_status status;
            const lib_u8 *code = form == 0u ? in_code : out_code;
            lib_i32 failed = !port_io_s55_prepare(&state,
                profiles[profile]);

            if (!failed) {
                port_io_s55_seed(&state.machine->executor_cpu);
                state.port.fail = 1;
                before = state.machine->executor_cpu;
                failed = core_machine_memory_write(state.machine, 0u, code, 2u) !=
                    LIB_STATUS_OK;
                if (!failed) {
                    status = core_machine_run(state.machine,
                        (core_machine_run_budget){ 1u, 0u }, &result);
                    after = test_core_machine_fixture_capture_cpu_after_run(
                        state.machine);
                    failed = core_machine_get_cpu_diagnostic(state.machine,
                        &diagnostic) != LIB_STATUS_OK;
                    failed |= status != LIB_STATUS_INTERNAL_ERROR;
                    failed |= result.reason != CORE_MACHINE_STOP_FAULT;
                    failed |= !diagnostic.first_fault.valid;
                    failed |= after.data.eip != before.data.eip;
                    failed |= after.data.eax != before.data.eax;
                    failed |= after.data.eflags != before.data.eflags;
                    failed |= !port_io_s55_gprs_same_except_eax(&before, &after);
                    failed |= !port_io_s55_sregs_same(&before, &after);
                    failed |= state.port.reads != 0u || state.port.writes != 0u;
                }
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static lib_i32 port_io_s55_test_vm86(void)
{
    static const lib_u8 code[] = { 0xe4u, 0x5au };
    port_io_s55_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    lib_status status;
    lib_i32 failed = !port_io_s55_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        port_io_s55_seed(&state.machine->executor_cpu);
        state.machine->executor_cpu.data.cr0 |= VCPU_CR0_PE;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_VM |
            VCPU_EFLAGS_IOPL | VCPU_EFLAGS_IF;
        state.machine->executor_cpu.data.cs.dpl = 3u;
        state.machine->executor_cpu.data.ss.dpl = 3u;
        before = state.machine->executor_cpu;
        failed = core_machine_memory_write(state.machine, 0u, code,
            sizeof(code)) != LIB_STATUS_OK;
        if (!failed) {
            status = core_machine_run(state.machine,
                (core_machine_run_budget){ 1u, 0u }, &result);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed = core_machine_get_cpu_diagnostic(state.machine,
                &diagnostic) != LIB_STATUS_OK;
            failed |= status != LIB_STATUS_INTERNAL_ERROR;
            failed |= result.reason != CORE_MACHINE_STOP_FAULT;
            failed |= !diagnostic.first_fault.valid;
            failed |= !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_DF);
            failed |= after.data.eip != before.data.eip;
            failed |= after.data.eax != before.data.eax;
            failed |= after.data.eflags != before.data.eflags;
            failed |= !port_io_s55_gprs_same_except_eax(&before, &after);
            failed |= !port_io_s55_sregs_same(&before, &after);
            failed |= state.port.reads != 0u || state.port.writes != 0u;
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 port_io_s55_test_tss_iomap(void)
{
    static const lib_u8 code[] = { 0xe4u, 0xe0u };
    const lib_u16 iomap_base = 0x0080u;
    lib_u8 denied;

    for (denied = 0u; denied != 3u; ++denied) {
        port_io_s55_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        lib_status status;
        lib_u8 bitmap = denied == 1u ? 0x01u : 0u;
        lib_i32 code_write;
        lib_i32 iomap_write;
        lib_i32 bitmap_write;
        lib_i32 diagnostic_status;
        lib_i32 failed = !port_io_s55_prepare(&state,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            port_io_s55_seed(&state.machine->executor_cpu);
            state.machine->executor_cpu.data.cr0 |= VCPU_CR0_PE;
            state.machine->executor_cpu.data.eflags = denied == 2u ?
                VCPU_EFLAGS_IF | VCPU_EFLAGS_IOPL : VCPU_EFLAGS_IF;
            state.machine->executor_cpu.data.cs.dpl = 3u;
            state.machine->executor_cpu.data.ss.dpl = 3u;
            state.machine->executor_cpu.data.tr.flagValid = LIB_TRUE;
            state.machine->executor_cpu.data.tr.selector = 0x0028u;
            state.machine->executor_cpu.data.tr.base = 0x0600u;
            state.machine->executor_cpu.data.tr.limit = 0x00ffu;
            state.machine->executor_cpu.data.tr.sys.type =
                VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
            state.port.input = 0x11223344u;
            before = state.machine->executor_cpu;
            code_write = core_machine_memory_write(state.machine, 0u, code,
                sizeof(code)) == LIB_STATUS_OK;
            iomap_write = core_machine_memory_write(state.machine, 0x0666u,
                &iomap_base, sizeof(iomap_base)) == LIB_STATUS_OK;
            bitmap_write = core_machine_memory_write(state.machine, 0x069cu,
                &bitmap, sizeof(bitmap)) == LIB_STATUS_OK;
            failed = !code_write || !iomap_write || !bitmap_write;
            if (!failed) {
                status = core_machine_run(state.machine,
                    (core_machine_run_budget){ 1u, 0u }, &result);
                after = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                diagnostic_status = core_machine_get_cpu_diagnostic(
                    state.machine, &diagnostic) == LIB_STATUS_OK;
                failed = !diagnostic_status;
                if (denied == 1u) {
                    failed |= status != LIB_STATUS_INTERNAL_ERROR;
                    failed |= result.reason != CORE_MACHINE_STOP_FAULT;
                    failed |= !diagnostic.first_fault.valid;
                    failed |= !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                        VCPUINS_EXCEPT_DF);
                    failed |= after.data.eip != before.data.eip;
                    failed |= after.data.eax != before.data.eax;
                    failed |= after.data.eflags != before.data.eflags;
                    failed |= state.port.reads != 0u || state.port.writes != 0u;
                }
                else {
                    failed |= status != LIB_STATUS_OK;
                    failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
                    failed |= diagnostic.first_fault.valid;
                    failed |= after.data.eip != sizeof(code);
                    failed |= after.data.eax !=
                        ((before.data.eax & 0xffffff00u) | 0x44u);
                    failed |= after.data.eflags != before.data.eflags;
                    failed |= state.port.reads != 1u || state.port.writes != 0u;
                    failed |= state.port.last_port != 0x00e0u;
                }
                failed |= !port_io_s55_gprs_same_except_eax(&before, &after);
                failed |= !port_io_s55_sregs_same(&before, &after);
            }
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 port_io_s55_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    port_io_s55_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    lib_status status;
    lib_i32 failed = !port_io_s55_prepare(&state, profile);

    if (!failed) {
        port_io_s55_seed(&state.machine->executor_cpu);
        failed = !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = state.machine->executor_cpu;
        failed |= core_machine_memory_write(state.machine, 0u, code, bytes) !=
            LIB_STATUS_OK;
        if (!failed) {
            status = core_machine_run(state.machine,
                (core_machine_run_budget){ 1u, 0u }, &result);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed = core_machine_get_cpu_diagnostic(state.machine,
                &diagnostic) != LIB_STATUS_OK;
            failed |= status != LIB_STATUS_INTERNAL_ERROR;
            failed |= result.reason != CORE_MACHINE_STOP_FAULT;
            failed |= !diagnostic.first_fault.valid;
            failed |= !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_UD);
            failed |= after.data.eip != before.data.eip;
            failed |= after.data.eax != before.data.eax;
            failed |= after.data.eflags != before.data.eflags;
            failed |= !port_io_s55_gprs_same_except_eax(&before, &after);
            failed |= !port_io_s55_sregs_same(&before, &after);
            failed |= state.port.reads != 0u || state.port.writes != 0u;
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 port_io_s55_test_rejections(void)
{
    static const lib_u8 opcodes[] = {
        0xe4u, 0xe5u, 0xe6u, 0xe7u, 0xecu, 0xedu, 0xeeu, 0xefu
    };
    static const core_machine_cpu_profile pre386[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    lib_u8 profile;
    lib_u8 opcode;

    for (profile = 0u; profile != sizeof(pre386) / sizeof(pre386[0]);
            ++profile) {
        for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
            lib_u8 attr66[] = { 0x66u, opcodes[opcode], 0x5au };
            lib_u8 attr67[] = { 0x67u, opcodes[opcode], 0x5au };
            lib_u8 combined[] = { 0x66u, 0x67u, opcodes[opcode], 0x5au };
            lib_u8 bytes = opcode < 4u ? 3u : 2u;

            if (!port_io_s55_expect_ud(pre386[profile], attr66,
                    bytes) || !port_io_s55_expect_ud(pre386[profile],
                    attr67, bytes) || !port_io_s55_expect_ud(pre386[profile],
                    combined, (lib_u8)(bytes + 1u)))
                return 0;
        }
    }
    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        lib_u8 lock[] = { 0xf0u, opcodes[opcode], 0x5au };
        lib_u8 lock66[] = { 0xf0u, 0x66u, opcodes[opcode], 0x5au };
        lib_u8 lock67[] = { 0xf0u, 0x67u, opcodes[opcode], 0x5au };
        lib_u8 lock_combined[] = {
            0xf0u, 0x66u, 0x67u, opcodes[opcode], 0x5au
        };
        lib_u8 bytes = opcode < 4u ? 3u : 2u;

        if (!port_io_s55_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock,
                bytes) || !port_io_s55_expect_ud(
                CORE_MACHINE_CPU_PROFILE_80386, lock66,
                (lib_u8)(bytes + 1u)) || !port_io_s55_expect_ud(
                CORE_MACHINE_CPU_PROFILE_80386, lock67,
                (lib_u8)(bytes + 1u)) || !port_io_s55_expect_ud(
                CORE_MACHINE_CPU_PROFILE_80386, lock_combined,
                (lib_u8)(bytes + 2u)))
            return 0;
    }
    return 1;
}

static lib_i32 port_io_s55_test_irq_no_shadow(void)
{
    static const lib_u8 in_code[] = { 0xe4u, 0x5au, 0x90u };
    static const lib_u8 out_code[] = { 0xeeu, 0x90u };
    static const lib_u8 hlt = 0xf4u;
    lib_u8 form;

    for (form = 0u; form != 2u; ++form) {
        port_io_s55_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        lib_u16 vector_offset = 0x0100u;
        lib_u16 vector_segment = 0u;
        lib_u16 frame_ip = 0u;
        const lib_u8 *code = form == 0u ? in_code : out_code;
        lib_u8 bytes = form == 0u ? sizeof(in_code) : sizeof(out_code);
        lib_i32 failed = !port_io_s55_prepare(&state,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            port_io_s55_seed(&state.machine->executor_cpu);
            state.port.input = 0x11223344u;
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
            state.machine->executor_cpu.data.idtr.limit = 0x017fu;
            before = state.machine->executor_cpu;
            failed = core_machine_memory_write(state.machine, 0u, code,
                bytes) != LIB_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, 0x20u * 4u,
                &vector_offset, sizeof(vector_offset)) != LIB_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, 0x20u * 4u + 2u,
                &vector_segment, sizeof(vector_segment)) != LIB_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, 0x0100u, &hlt,
                sizeof(hlt)) != LIB_STATUS_OK;
        }
        if (!failed) {
            lib_memory_set(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master,
                &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed = core_machine_run(state.machine,
                (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            failed |= after.data.eip != 0x0101u;
            failed |= after.data.ecx != before.data.ecx;
            failed |= after.data.edx != before.data.edx;
            failed |= after.data.ebx != before.data.ebx;
            failed |= after.data.ebp != before.data.ebp;
            failed |= after.data.esi != before.data.esi;
            failed |= after.data.edi != before.data.edi;
            failed |= !port_io_s55_sregs_same(&before, &after);
            failed |= core_machine_memory_read_physical(
                &state.machine->executor_memory, after.data.ss.base +
                (lib_u16)after.data.esp, (lib_uptr)&frame_ip,
                sizeof(frame_ip)) != LIB_STATUS_OK;
            failed |= frame_ip != (form == 0u ? 2u : 1u);
            failed |= !CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u));
            failed |= CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u));
            if (form == 0u) {
                failed |= after.data.eax !=
                    ((before.data.eax & 0xffffff00u) | 0x44u);
                failed |= state.port.reads != 1u || state.port.writes != 0u;
            }
            else {
                failed |= after.data.eax != before.data.eax;
                failed |= state.port.reads != 0u || state.port.writes != 1u;
                failed |= state.port.last_write != (before.data.eax & 0xffu);
            }
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!port_io_s55_test_default_forms()) {
        fprintf(stderr, "S55 default forms failed\n");
        return 1;
    }
    if (!port_io_s55_test_386_attributes()) {
        fprintf(stderr, "S55 attributes failed\n");
        return 1;
    }
    if (!port_io_s55_test_provider_error()) {
        fprintf(stderr, "S55 provider error failed\n");
        return 1;
    }
    if (!port_io_s55_test_vm86()) {
        fprintf(stderr, "S55 VM86 failed\n");
        return 1;
    }
    if (!port_io_s55_test_tss_iomap()) {
        fprintf(stderr, "S55 TSS I/O bitmap failed\n");
        return 1;
    }
    if (!port_io_s55_test_rejections()) {
        fprintf(stderr, "S55 rejection matrix failed\n");
        return 1;
    }
    if (!port_io_s55_test_irq_no_shadow()) {
        fprintf(stderr, "S55 PIC no-shadow failed\n");
        return 1;
    }
    printf("M5:T316:S55:PORT-IO:OK\n");
    return 0;
}
