#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct port_strings_port {
    type_unsigned_32 input;
    type_unsigned_32 reads;
    type_unsigned_32 writes;
    type_unsigned_32 last_write;
    type_unsigned_32 write_log[3];
} port_strings_port;

typedef struct port_strings_machine {
    core_machine *machine;
    port_strings_port port;
} port_strings_machine;

static type_status port_strings_read(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 *value)
{
    port_strings_port *state = (port_strings_port *)owner;

    if (state == STD_NULL || value == STD_NULL || port != 0x00e0u)
        return TYPE_STATUS_INVALID_ARGUMENT;
    ++state->reads;
    *value = state->input;
    return TYPE_STATUS_OK;
}

static type_status port_strings_write(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 value)
{
    port_strings_port *state = (port_strings_port *)owner;

    if (state == STD_NULL || port != 0x00e0u)
        return TYPE_STATUS_INVALID_ARGUMENT;
    ++state->writes;
    state->last_write = value;
    if (state->writes <= sizeof(state->write_log) / sizeof(state->write_log[0]))
        state->write_log[state->writes - 1u] = value;
    return TYPE_STATUS_OK;
}

static const core_machine_port_provider port_strings_provider = {
    port_strings_read, port_strings_write
};

static C_VOID port_strings_reset(C_VOID *opaque)
{
    port_strings_machine *state = (port_strings_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider port_strings_execution_provider = {
    port_strings_reset, STD_NULL
};

static C_INT port_strings_prepare(core_machine_cpu_profile profile,
    port_strings_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
    return core_machine_create(&config, &state->machine) == TYPE_STATUS_OK &&
        core_machine_install_port_provider(state->machine, 0x00e0u, 0x00e0u,
            &port_strings_provider, &state->port) == TYPE_STATUS_OK &&
        test_core_machine_fixture_bind_freeze_reset(state->machine,
            &port_strings_execution_provider, state) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_VOID port_strings_seed(port_strings_machine *state)
{
    t_cpu *cpu = &state->machine->executor_cpu;

    cpu->data.eax = 0xaabbccddU;
    cpu->data.ecx = 0x11220003u;
    cpu->data.edx = 0x778800e0u;
    cpu->data.ebx = 0xbbccddeeU;
    cpu->data.esp = 0x8000u;
    cpu->data.ebp = 0x120u;
    cpu->data.esi = 0x10u;
    cpu->data.edi = 0x20u;
    cpu->data.eflags = VCPU_EFLAGS_IF;
    cpu->data.cs.base = 0u;
    cpu->data.ds.base = 0x20000u;
    cpu->data.es.base = 0x30000u;
    cpu->data.fs.base = 0x40000u;
}

static C_INT port_strings_nonindexes_same(const t_cpu *before,
    const t_cpu *after)
{
    return after->data.eax == before->data.eax &&
        after->data.edx == before->data.edx &&
        after->data.ebx == before->data.ebx &&
        after->data.esp == before->data.esp &&
        after->data.ebp == before->data.ebp;
}

static C_INT port_strings_run(port_strings_machine *state, const type_unsigned_8 *code,
    type_unsigned_8 bytes, type_unsigned_32 budget, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic, type_status *status,
    core_machine_run_result *result)
{
    if (core_machine_memory_write(state->machine, 0u, code, bytes) !=
        TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){budget, 0u}, result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        TYPE_STATUS_OK;
}

static C_INT port_strings_memory(port_strings_machine *state, type_unsigned_32 address,
    type_unsigned_32 expected, type_unsigned_8 width)
{
    type_unsigned_32 observed = 0u;

    return core_machine_memory_read_physical(&state->machine->executor_memory,
        address, TYPE_REFERENCE_OF(observed), width) == TYPE_STATUS_OK &&
        STD_MEMCMP(&observed, &expected, width) == 0;
}

static C_INT port_strings_single(core_machine_cpu_profile profile, C_INT input,
    const type_unsigned_8 *code, type_unsigned_8 bytes, type_unsigned_8 width,
    C_INT address32, type_unsigned_32 memory, type_unsigned_32 value)
{
    port_strings_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_32 index = address32 ? 0x1020u : input ? 0x20u : 0x10u;
    C_INT failed = !port_strings_prepare(profile, &state);

    if (!failed) {
        port_strings_seed(&state);
        if (address32) {
            if (input)
                state.machine->executor_cpu.data.edi = index;
            else
                state.machine->executor_cpu.data.esi = 0x1010u;
        }
        if (input)
            state.port.input = value;
        else
            failed |= core_machine_memory_write(state.machine, memory, &value,
                width) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !port_strings_run(&state, code, bytes, 1u, &after,
            &diagnostic, &status, &result) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != bytes ||
            !port_strings_nonindexes_same(&before, &after) ||
            after.data.ecx != before.data.ecx || after.data.eflags !=
            before.data.eflags || after.data.esi != (input ? before.data.esi :
            (address32 ? 0x1010u + width : (before.data.esi & 0xffff0000u) |
            (type_unsigned_16)(0x10u + width))) || after.data.edi != (input ?
            (address32 ? 0x1020u + width : (before.data.edi & 0xffff0000u) |
            (type_unsigned_16)(0x20u + width)) : before.data.edi) ||
            (input ? state.port.reads != 1u || state.port.writes != 0u ||
            !port_strings_memory(&state, memory, value, width) :
            state.port.reads != 0u || state.port.writes != 1u ||
            state.port.last_write != value || !port_strings_memory(&state,
            memory, value, width));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT port_strings_test_single_profile(core_machine_cpu_profile profile)
{
    static const type_unsigned_8 insb = 0x6cu;
    static const type_unsigned_8 insw = 0x6du;
    static const type_unsigned_8 outsb = 0x6eu;
    static const type_unsigned_8 outsw = 0x6fu;
    static const type_unsigned_8 insd[] = {0x66u, 0x6du};
    static const type_unsigned_8 outsd[] = {0x66u, 0x6fu};
    static const type_unsigned_8 ins32[] = {0x67u, 0x6cu};
    static const type_unsigned_8 out32[] = {0x67u, 0x6eu};
    static const type_unsigned_8 ins_combined[] = {0x66u, 0x67u, 0x6du};
    static const type_unsigned_8 out_combined[] = {0x66u, 0x67u, 0x6fu};

    if (!port_strings_single(profile, 1, &insb, 1u, 1u, 0, 0x30020u, 0x5au) ||
        !port_strings_single(profile, 1, &insw, 1u, 2u, 0, 0x30020u,
            0x5a5au) || !port_strings_single(profile, 0, &outsb, 1u, 1u, 0,
            0x20010u, 0x5au) || !port_strings_single(profile, 0, &outsw, 1u,
            2u, 0, 0x20010u, 0x5a5au)) return 0;
    if (profile != CORE_MACHINE_CPU_PROFILE_80386) return 1;
    return port_strings_single(profile, 1, insd, sizeof(insd), 4u, 0,
        0x30020u, 0x5a5a5a5au) && port_strings_single(profile, 0, outsd,
        sizeof(outsd), 4u, 0, 0x20010u, 0x5a5a5a5au) &&
        port_strings_single(profile, 1, ins32, sizeof(ins32), 1u, 1,
        0x31020u, 0x5au) && port_strings_single(profile, 0, out32,
        sizeof(out32), 1u, 1, 0x21010u, 0x5au) &&
        port_strings_single(profile, 1, ins_combined, sizeof(ins_combined),
        4u, 1, 0x31020u, 0x5a5a5a5au) && port_strings_single(profile, 0,
        out_combined, sizeof(out_combined), 4u, 1, 0x21010u, 0x5a5a5a5au);
}

static C_INT port_strings_test_single(C_VOID)
{
    return port_strings_test_single_profile(CORE_MACHINE_CPU_PROFILE_80186) &&
        port_strings_test_single_profile(CORE_MACHINE_CPU_PROFILE_80386);
}

static C_INT port_strings_rep(core_machine_cpu_profile profile, C_INT input,
    const type_unsigned_8 *code, type_unsigned_8 bytes, type_unsigned_8 width,
    C_INT address32)
{
    port_strings_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_32 values[] = {0x11u, 0x22u, 0x33u};
    type_unsigned_32 base = input ? (address32 ? 0x31020u : 0x30020u) :
        (address32 ? 0x21010u : 0x20010u);
    type_unsigned_8 item;
    C_INT failed = !port_strings_prepare(profile, &state);

    if (!failed) {
        port_strings_seed(&state);
        state.machine->executor_cpu.data.ecx = address32 ? 3u : 0x11220003u;
        if (address32) {
            if (input)
                state.machine->executor_cpu.data.edi = 0x1020u;
            else
                state.machine->executor_cpu.data.esi = 0x1010u;
        }
        if (input)
            state.port.input = values[0];
        for (item = 0u; item != 3u; ++item) {
            if (!input)
                failed |= core_machine_memory_write(state.machine, base + item * width,
                    &values[item], width) != TYPE_STATUS_OK;
        }
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !port_strings_run(&state, code, bytes, 3u, &after,
            &diagnostic, &status, &result) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != bytes ||
            !port_strings_nonindexes_same(&before, &after) ||
            after.data.ecx != (address32 ? 0u : 0x11220000u) ||
            after.data.eflags != before.data.eflags || after.data.esi !=
            (input ? before.data.esi : (address32 ? 0x1010u + width * 3u :
            (before.data.esi & 0xffff0000u) | (type_unsigned_16)(0x10u + width * 3u))) ||
            after.data.edi != (input ? (address32 ? 0x1020u + width * 3u :
            (before.data.edi & 0xffff0000u) | (type_unsigned_16)(0x20u + width * 3u)) :
            before.data.edi) || (input ? state.port.reads != 3u ||
            state.port.writes != 0u : state.port.reads != 0u ||
            state.port.writes != 3u || state.port.write_log[0] != values[0] ||
            state.port.write_log[1] != values[1] || state.port.write_log[2] != values[2]);
        if (input)
            for (item = 0u; item != 3u; ++item)
                failed |= !port_strings_memory(&state, base + item * width,
                    values[0], width);
        else
            for (item = 0u; item != 3u; ++item)
                failed |= !port_strings_memory(&state, base + item * width,
                    values[item], width);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT port_strings_test_rep(C_VOID)
{
    static const type_unsigned_8 rep_insb[] = {0xf3u, 0x6cu};
    static const type_unsigned_8 rep_insw[] = {0xf3u, 0x6du};
    static const type_unsigned_8 rep_outsb[] = {0xf3u, 0x6eu};
    static const type_unsigned_8 rep_outsw[] = {0xf3u, 0x6fu};
    static const type_unsigned_8 rep_insd[] = {0xf3u, 0x66u, 0x67u, 0x6du};
    static const type_unsigned_8 rep_outsd[] = {0xf3u, 0x66u, 0x67u, 0x6fu};

    return port_strings_rep(CORE_MACHINE_CPU_PROFILE_80186, 1, rep_insb,
        sizeof(rep_insb), 1u, 0) && port_strings_rep(
        CORE_MACHINE_CPU_PROFILE_80186, 1, rep_insw, sizeof(rep_insw), 2u,
        0) && port_strings_rep(
        CORE_MACHINE_CPU_PROFILE_80186, 0, rep_outsb, sizeof(rep_outsb), 1u,
        0) && port_strings_rep(CORE_MACHINE_CPU_PROFILE_80186, 0, rep_outsw,
        sizeof(rep_outsw), 2u, 0) && port_strings_rep(
        CORE_MACHINE_CPU_PROFILE_80386, 1, rep_insd,
        sizeof(rep_insd), 4u, 1) && port_strings_rep(
        CORE_MACHINE_CPU_PROFILE_80386, 0, rep_outsd, sizeof(rep_outsd), 4u,
        1);
}

static C_INT port_strings_rep_zero(C_INT input, const type_unsigned_8 *code,
    type_unsigned_8 bytes, C_INT address32)
{
    port_strings_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_8 source = 0x5au;
    type_unsigned_8 destination = 0xa5u;
    C_INT failed = !port_strings_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        port_strings_seed(&state);
        state.machine->executor_cpu.data.ecx = address32 ? 0u : 0x11220000u;
        failed |= core_machine_memory_write(state.machine, 0x20010u, &source,
            sizeof(source)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x30020u, &destination, sizeof(destination)) !=
            TYPE_STATUS_OK;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !port_strings_run(&state, code, bytes, 1u, &after,
            &diagnostic, &status, &result) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != bytes ||
            after.data.eax != before.data.eax || after.data.ecx !=
            before.data.ecx || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.esp !=
            before.data.esp || after.data.ebp != before.data.ebp ||
            after.data.esi != before.data.esi || after.data.edi !=
            before.data.edi || after.data.eflags != before.data.eflags ||
            state.port.reads != 0u || state.port.writes != 0u ||
            !port_strings_memory(&state, 0x20010u, source, sizeof(source)) ||
            !port_strings_memory(&state, 0x30020u, destination,
            sizeof(destination));
    }
    core_machine_destroy(state.machine);
    (C_VOID)input;
    return !failed;
}

static C_INT port_strings_test_rep_counts(C_VOID)
{
    static const type_unsigned_8 rep_insb[] = {0xf3u, 0x6cu};
    static const type_unsigned_8 rep_outsb[] = {0xf3u, 0x6eu};
    static const type_unsigned_8 rep_ins32[] = {0xf3u, 0x67u, 0x6cu};
    static const type_unsigned_8 rep_out32[] = {0xf3u, 0x67u, 0x6eu};

    return port_strings_rep_zero(1, rep_insb, sizeof(rep_insb), 0) &&
        port_strings_rep_zero(0, rep_outsb, sizeof(rep_outsb), 0) &&
        port_strings_rep_zero(1, rep_ins32, sizeof(rep_ins32), 1) &&
        port_strings_rep_zero(0, rep_out32, sizeof(rep_out32), 1);
}

static C_INT port_strings_rep_one(C_INT input)
{
    static const type_unsigned_8 ins[] = {0xf3u, 0x6cu};
    static const type_unsigned_8 outs[] = {0xf3u, 0x6eu};
    port_strings_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_8 value = 0x5au;
    const type_unsigned_8 *code = input ? ins : outs;
    C_INT failed = !port_strings_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        port_strings_seed(&state);
        state.machine->executor_cpu.data.ecx = 0x11220001u;
        if (input)
            state.port.input = value;
        else
            failed |= core_machine_memory_write(state.machine, 0x20010u, &value,
                sizeof(value)) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !port_strings_run(&state, code, sizeof(ins), 1u, &after,
            &diagnostic, &status, &result) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != 2u ||
            !port_strings_nonindexes_same(&before, &after) ||
            after.data.ecx != 0x11220000u || after.data.esi !=
            (input ? before.data.esi : 0x11u) || after.data.edi !=
            (input ? 0x21u : before.data.edi) || after.data.eflags !=
            before.data.eflags || (input ? state.port.reads != 1u ||
            state.port.writes != 0u || !port_strings_memory(&state, 0x30020u,
            value, sizeof(value)) : state.port.reads != 0u ||
            state.port.writes != 1u || state.port.last_write != value ||
            !port_strings_memory(&state, 0x20010u, value, sizeof(value)));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT port_strings_test_segments_and_df(C_VOID)
{
    static const type_unsigned_8 ins_cs[] = {0x2eu, 0x6cu};
    static const type_unsigned_8 ins_fs[] = {0x64u, 0x6cu};
    static const type_unsigned_8 out_cs[] = {0x2eu, 0x6eu};
    static const type_unsigned_8 out_fs[] = {0x64u, 0x6eu};
    static const type_unsigned_8 ins_df = 0x6cu;
    static const type_unsigned_8 out_df = 0x6eu;
    const type_unsigned_8 *ins_forms[] = {ins_cs, ins_fs};
    const type_unsigned_8 *out_forms[] = {out_cs, out_fs};
    const type_unsigned_8 ins_bytes[] = {sizeof(ins_cs), sizeof(ins_fs)};
    const type_unsigned_8 out_bytes[] = {sizeof(out_cs), sizeof(out_fs)};
    type_unsigned_8 form;

    for (form = 0u; form != 2u; ++form) {
        port_strings_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_status status;
        type_unsigned_8 source = 0x5au;
        type_unsigned_8 expected = 0x5au;
        type_unsigned_32 source_address = form == 0u ? 0x10u : 0x40010u;
        C_INT failed = !port_strings_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            port_strings_seed(&state);
            state.port.input = expected;
            failed |= core_machine_memory_write(state.machine, source_address,
                &source, sizeof(source)) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !port_strings_run(&state, ins_forms[form], ins_bytes[form],
                1u, &after, &diagnostic, &status, &result) ||
                status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != ins_bytes[form] ||
                !port_strings_nonindexes_same(&before, &after) ||
                after.data.ecx != before.data.ecx || after.data.esi !=
                before.data.esi || after.data.edi != 0x21u ||
                after.data.eflags != before.data.eflags || state.port.reads != 1u ||
                state.port.writes != 0u || !port_strings_memory(&state, 0x30020u,
                expected, sizeof(expected));
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;

        failed = !port_strings_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed) {
            port_strings_seed(&state);
            failed |= core_machine_memory_write(state.machine, source_address,
                &source, sizeof(source)) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !port_strings_run(&state, out_forms[form], out_bytes[form],
                1u, &after, &diagnostic, &status, &result) ||
                status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != out_bytes[form] ||
                !port_strings_nonindexes_same(&before, &after) ||
                after.data.ecx != before.data.ecx || after.data.esi != 0x11u ||
                after.data.edi != before.data.edi || after.data.eflags !=
                before.data.eflags || state.port.reads != 0u ||
                state.port.writes != 1u || state.port.last_write != source ||
                !port_strings_memory(&state, source_address, source,
                sizeof(source));
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }

    {
        port_strings_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_status status;
        type_unsigned_8 value = 0x5au;
        C_INT failed = !port_strings_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            port_strings_seed(&state);
            state.port.input = value;
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_DF;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !port_strings_run(&state, &ins_df, sizeof(ins_df), 1u,
                &after, &diagnostic, &status, &result) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != 1u ||
                !port_strings_nonindexes_same(&before, &after) ||
                after.data.ecx != before.data.ecx || after.data.esi !=
                before.data.esi || after.data.edi != 0x1fu ||
                after.data.eflags != before.data.eflags || state.port.reads != 1u ||
                !port_strings_memory(&state, 0x30020u, value, sizeof(value));
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;

        failed = !port_strings_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed) {
            port_strings_seed(&state);
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_DF;
            failed |= core_machine_memory_write(state.machine, 0x20010u, &value,
                sizeof(value)) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !port_strings_run(&state, &out_df, sizeof(out_df), 1u,
                &after, &diagnostic, &status, &result) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != 1u ||
                !port_strings_nonindexes_same(&before, &after) ||
                after.data.ecx != before.data.ecx || after.data.esi != 0x0fu ||
                after.data.edi != before.data.edi || after.data.eflags !=
                before.data.eflags || state.port.writes != 1u ||
                state.port.last_write != value || !port_strings_memory(&state,
                0x20010u, value, sizeof(value));
        }
        core_machine_destroy(state.machine);
        return !failed;
    }
}

static C_INT port_strings_boot_protected(port_strings_machine *state)
{
    static const type_unsigned_8 pointer[] = {0x27u,0,0,0x03u,0,0};
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0x0fu,0,0,0x30u,0,0x92u,0,0, 0x0fu,0,0,0x40u,0,0x92u,0,0,
        0xffu,0xffu,0,0x50u,0,0x92u,0,0
    };
    static const type_unsigned_8 boot[] = {
        0x0fu,0x01u,0x16u,0,1u, 0xb8u,1u,0,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0,0x8eu,0xd8u, 0xb8u,0x18u,0,0x8eu,0xc0u,
        0xb8u,0x20u,0,0x8eu,0xd0u,0xbcu,0,0x80u, 0xeau,0,0,8u,0
    };
    static const type_unsigned_8 halt = 0xf4u;
    core_machine_run_result result;

    return core_machine_memory_write(state->machine, 0x100u, pointer,
        sizeof(pointer)) == TYPE_STATUS_OK && core_machine_memory_write(
        state->machine, 0x300u, gdt, sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, boot, sizeof(boot)) ==
        TYPE_STATUS_OK && core_machine_memory_write(state->machine, 0x2000u,
        &halt, sizeof(halt)) == TYPE_STATUS_OK && core_machine_run(
        state->machine, (core_machine_run_budget){96u,0u}, &result) ==
        TYPE_STATUS_OK && result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT port_strings_protected_single(C_INT input)
{
    static const type_unsigned_8 ins = 0x6cu;
    static const type_unsigned_8 outs = 0x6eu;
    port_strings_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_unsigned_8 source = 0x5au;
    type_unsigned_8 destination = 0xa5u;
    const type_unsigned_8 *code = input ? &ins : &outs;
    C_INT failed = !port_strings_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed)
        failed |= !port_strings_boot_protected(&state);
    if (!failed) {
        t_cpu_data_sreg cs = state.machine->executor_cpu.data.cs;
        t_cpu_data_sreg ds = state.machine->executor_cpu.data.ds;
        t_cpu_data_sreg es = state.machine->executor_cpu.data.es;
        t_cpu_data_sreg ss = state.machine->executor_cpu.data.ss;

        port_strings_seed(&state);
        state.machine->executor_cpu.data.cs = cs;
        state.machine->executor_cpu.data.ds = ds;
        state.machine->executor_cpu.data.es = es;
        state.machine->executor_cpu.data.ss = ss;
        if (input) {
            state.machine->executor_cpu.data.edi = 0x10u;
            state.port.input = source;
        } else
            state.machine->executor_cpu.data.esi = 0x10u;
        failed |= core_machine_memory_write(state.machine, 0x3010u, &source,
            sizeof(source)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x4010u, &destination, sizeof(destination)) !=
            TYPE_STATUS_OK || core_machine_memory_write(state.machine, 0x2000u,
            code, 1u) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){1u,0u}, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || core_machine_get_cpu_diagnostic(
            state.machine, &diagnostic) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            after.data.eip != 0u || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx || after.data.edx !=
            before.data.edx || after.data.ebx != before.data.ebx ||
            after.data.esp != before.data.esp || after.data.ebp !=
            before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi || after.data.eflags !=
            before.data.eflags || STD_MEMCMP(&before.data.ds,
            &after.data.ds, sizeof(before.data.ds)) != 0 || STD_MEMCMP(
            &before.data.es, &after.data.es, sizeof(before.data.es)) != 0 ||
            state.port.reads != 0u || state.port.writes != 0u ||
            !port_strings_memory(&state, 0x3010u, source, sizeof(source)) ||
            !port_strings_memory(&state, 0x4010u, destination,
            sizeof(destination));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT port_strings_test_protected(C_VOID)
{
    if (!port_strings_protected_single(1)) {
        STD_PRINTF("PORT-STRINGS protected=ins\n");
        return 0;
    }
    if (!port_strings_protected_single(0)) {
        STD_PRINTF("PORT-STRINGS protected=outs\n");
        return 0;
    }
    return 1;
}

static C_INT port_strings_protected_rep(C_INT input)
{
    static const type_unsigned_8 ins[] = {0xf3u, 0x6cu};
    static const type_unsigned_8 outs[] = {0xf3u, 0x6eu};
    port_strings_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_unsigned_8 source[] = {0x5au, 0x6bu};
    type_unsigned_8 destination[] = {0xa5u, 0xb6u};
    const type_unsigned_8 *code = input ? ins : outs;
    C_INT failed = !port_strings_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed)
        failed |= !port_strings_boot_protected(&state);
    if (!failed) {
        t_cpu_data_sreg cs = state.machine->executor_cpu.data.cs;
        t_cpu_data_sreg ds = state.machine->executor_cpu.data.ds;
        t_cpu_data_sreg es = state.machine->executor_cpu.data.es;
        t_cpu_data_sreg ss = state.machine->executor_cpu.data.ss;

        port_strings_seed(&state);
        state.machine->executor_cpu.data.cs = cs;
        state.machine->executor_cpu.data.ds = ds;
        state.machine->executor_cpu.data.es = es;
        state.machine->executor_cpu.data.ss = ss;
        if (input) {
            state.machine->executor_cpu.data.es.limit = 0x10u;
            state.machine->executor_cpu.data.edi = 0x10u;
            state.port.input = source[0];
        } else {
            state.machine->executor_cpu.data.ds.limit = 0x10u;
            state.machine->executor_cpu.data.esi = 0x10u;
        }
        state.machine->executor_cpu.data.ecx = 0x11220003u;
        failed |= core_machine_memory_write(state.machine, 0x3010u, source,
            sizeof(source)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x4010u, destination, sizeof(destination)) !=
            TYPE_STATUS_OK || core_machine_memory_write(state.machine, 0x2000u,
            code, sizeof(ins)) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){2u,0u}, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || core_machine_get_cpu_diagnostic(
            state.machine, &diagnostic) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            after.data.eip != 0u || after.data.eax != before.data.eax ||
            after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
            after.data.esp != before.data.esp || after.data.ebp != before.data.ebp ||
            after.data.ecx != 0x11220002u || after.data.esi !=
            (input ? before.data.esi : 0x11u) || after.data.edi !=
            (input ? 0x11u : before.data.edi) || after.data.eflags !=
            before.data.eflags || STD_MEMCMP(&before.data.ds, &after.data.ds,
            sizeof(before.data.ds)) != 0 || STD_MEMCMP(&before.data.es,
            &after.data.es, sizeof(before.data.es)) != 0 || (input ?
            state.port.reads != 1u || state.port.writes != 0u ||
            !port_strings_memory(&state, 0x4010u, source[0], sizeof(source[0])) ||
            !port_strings_memory(&state, 0x4011u, destination[1],
            sizeof(destination[1])) : state.port.reads != 0u ||
            state.port.writes != 1u || state.port.last_write != source[0] ||
            !port_strings_memory(&state, 0x3010u, source[0], sizeof(source[0])) ||
            !port_strings_memory(&state, 0x3011u, source[1], sizeof(source[1])));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT port_strings_irq_case(C_INT input, C_INT repeat)
{
    static const type_unsigned_8 ins[] = {0x6cu, 0x90u};
    static const type_unsigned_8 outs[] = {0x6eu, 0x90u};
    static const type_unsigned_8 rep_ins[] = {0xf3u, 0x6cu, 0x90u};
    static const type_unsigned_8 rep_outs[] = {0xf3u, 0x6eu, 0x90u};
    static const type_unsigned_8 hlt = 0xf4u;
    port_strings_machine state;
    core_machine_pic_irq_source irq;
    core_machine_run_result result;
    t_cpu after;
    type_unsigned_16 offset = 0x100u;
    type_unsigned_16 segment = 0u;
    type_unsigned_16 frame_ip = 0u;
    type_unsigned_8 source[] = {0x5au, 0x6bu, 0x7cu};
    const type_unsigned_8 *code = repeat ? (input ? rep_ins : rep_outs) :
        (input ? ins : outs);
    type_unsigned_8 bytes = repeat ? 3u : 2u;
    C_INT failed = !port_strings_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0x20010u, source,
            sizeof(source)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x80u, &offset, sizeof(offset)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x82u, &segment,
            sizeof(segment)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x100u, &hlt, sizeof(hlt)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0u, code, bytes) !=
            TYPE_STATUS_OK;
    }
    if (!failed) {
        port_strings_seed(&state);
        if (input)
            state.port.input = source[0];
        if (repeat)
            state.machine->executor_cpu.data.ecx = 0x11220003u;
        state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
        STD_MEMSET(&irq, 0, sizeof(irq));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){repeat ? 4u : 2u,0u}, &result) !=
            TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            after.data.ss.base + (type_unsigned_16)after.data.esp,
            TYPE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != TYPE_STATUS_OK ||
            after.data.eip != 0x101u || frame_ip != (repeat ? 0u : 1u) ||
            after.data.esi != (input ? 0x10u : 0x11u) || after.data.edi !=
            (input ? 0x21u : 0x20u) || after.data.ecx !=
            (repeat ? 0x11220002u : 0x11220003u) || after.data.eflags != 0u ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
            VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
            VPIC_IRR_IRQ(0u)) || (input ? state.port.reads != 1u ||
            state.port.writes != 0u || !port_strings_memory(&state, 0x30020u,
            source[0], sizeof(source[0])) : state.port.reads != 0u ||
            state.port.writes != 1u || state.port.last_write != source[0] ||
            !port_strings_memory(&state, 0x20010u, source[0], sizeof(source[0])));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT port_strings_test_irq(C_VOID)
{
    return port_strings_irq_case(1, 0) && port_strings_irq_case(0, 0) &&
        port_strings_irq_case(1, 1) && port_strings_irq_case(0, 1);
}

static C_INT port_strings_expect_ud(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes)
{
    port_strings_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_8 source = 0x5au;
    type_unsigned_8 destination = 0xa5u;
    C_INT failed = !port_strings_prepare(profile, &state);

    if (!failed) {
        port_strings_seed(&state);
        failed |= core_machine_memory_write(state.machine, 0x20010u, &source,
            sizeof(source)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x30020u, &destination, sizeof(destination)) !=
            TYPE_STATUS_OK;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !port_strings_run(&state, code, bytes, 1u, &after,
            &diagnostic, &status, &result) || status != TYPE_STATUS_FAULT ||
            !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eip != 0u || STD_MEMCMP(&before.data, &after.data,
            sizeof(before.data)) != 0 || state.port.reads != 0u ||
            state.port.writes != 0u || !port_strings_memory(&state, 0x20010u,
            source, sizeof(source)) || !port_strings_memory(&state, 0x30020u,
            destination, sizeof(destination));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT port_strings_test_rejections(C_VOID)
{
    static const core_machine_cpu_profile pre386[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 forms[][5] = {
        {0x6cu,0,0,0,0}, {0x6du,0,0,0,0}, {0x6eu,0,0,0,0}, {0x6fu,0,0,0,0},
        {0xf3u,0x6cu,0,0,0}, {0xf3u,0x6fu,0,0,0}, {0x66u,0x6du,0,0,0},
        {0x67u,0x6eu,0,0,0}, {0x66u,0x67u,0x6fu,0,0},
        {0xf0u,0x6cu,0,0,0}, {0xf0u,0x6du,0,0,0}, {0xf0u,0x6eu,0,0,0},
        {0xf0u,0x6fu,0,0,0}, {0xf0u,0xf3u,0x66u,0x67u,0x6du}
    };
    static const type_unsigned_8 bytes[] = {1u,1u,1u,1u,2u,2u,2u,2u,3u,2u,2u,2u,2u,5u};
    type_unsigned_8 profile;
    type_unsigned_8 form;

    for (form = 0u; form != 6u; ++form)
        if (!port_strings_expect_ud(CORE_MACHINE_CPU_PROFILE_8086,
                forms[form], bytes[form])) return 0;
    for (profile = 0u; profile != sizeof(pre386) / sizeof(pre386[0]); ++profile)
        for (form = 6u; form != 9u; ++form)
            if (!port_strings_expect_ud(pre386[profile], forms[form],
                    bytes[form])) return 0;
    for (form = 9u; form != sizeof(bytes); ++form)
        if (!port_strings_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, forms[form],
            bytes[form]))
            return 0;
    return 1;
}

C_INT main(C_VOID)
{
    if (!port_strings_test_single()) {
        STD_PRINTF("PORT-STRINGS stage=single\n");
        return 1;
    }
    if (!port_strings_test_rep()) {
        STD_PRINTF("PORT-STRINGS stage=rep\n");
        return 1;
    }
    if (!port_strings_test_rep_counts()) {
        STD_PRINTF("PORT-STRINGS stage=rep-counts\n");
        return 1;
    }
    if (!port_strings_rep_one(1) || !port_strings_rep_one(0)) {
        STD_PRINTF("PORT-STRINGS stage=rep-one\n");
        return 1;
    }
    if (!port_strings_test_segments_and_df()) {
        STD_PRINTF("PORT-STRINGS stage=segments-df\n");
        return 1;
    }
    if (!port_strings_test_rejections()) {
        STD_PRINTF("PORT-STRINGS stage=rejections\n");
        return 1;
    }
    if (!port_strings_test_protected()) {
        STD_PRINTF("PORT-STRINGS stage=protected\n");
        return 1;
    }
    if (!port_strings_protected_rep(1) || !port_strings_protected_rep(0)) {
        STD_PRINTF("PORT-STRINGS stage=protected-rep\n");
        return 1;
    }
    if (!port_strings_test_irq()) {
        STD_PRINTF("PORT-STRINGS stage=irq\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S38:PORT-STRINGS:OK\n");
    return 0;
}
