#include "support/cpu_port_instruction_fixture.h"
#include "lib/types/file.h"
/* T337_REAL_UD_TERMINAL_CPU_OWNER: rejected string ports stay CPU-owned. */

static void port_strings_seed(cpu_port_instruction_fixture *state)
{
    t_cpu *cpu = &state->instruction.cpu;

    cpu->data.eax = 0xaabbccddu;
    cpu->data.ecx = 0x11220003u;
    cpu->data.edx = 0x778800e0u;
    cpu->data.ebx = 0xbbccddeeu;
    cpu->data.esp = 0x8000u;
    cpu->data.ebp = 0x120u;
    cpu->data.esi = 0x10u;
    cpu->data.edi = 0x20u;
    cpu->data.eflags = VCPU_EFLAGS_IF;
    cpu->data.ds.base = 0x20000u;
    cpu->data.es.base = 0x30000u;
    cpu->data.fs.base = 0x40000u;
}

static lib_i32 port_strings_case(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_bool input, lib_u8 width,
    lib_bool address32, lib_u8 count, lib_bool direction,
    lib_u32 source_base)
{
    cpu_port_instruction_fixture state;
    t_cpu before, after;
    lib_u32 index, address, value;
    lib_u8 item;
    lib_bool repeat = code[0] == 0xf3u;

    cpu_port_prepare(&state, profile);
    port_strings_seed(&state);
    state.instruction.cpu.data.ecx = address32 ? count :
        0x11220000u | count;
    if (address32) {
        state.instruction.cpu.data.esi = 0x1010u;
        state.instruction.cpu.data.edi = 0x1020u;
    }
    if (direction)
        state.instruction.cpu.data.eflags |= VCPU_EFLAGS_DF;
    index = input ? (address32 ? 0x1020u : 0x20u) :
        (address32 ? 0x1010u : 0x10u);
    address = (input ? 0x30000u : source_base) + index;
    if (count == 0u) {
        state.instruction.memory[0x20010u] = 0x5au;
        state.instruction.memory[0x30020u] = 0xa5u;
    }
    state.input_value = 0x5au;
    for (item = 0u; item != count; ++item) {
        value = 0x11u + item * 0x11u;
        if (!input)
            lib_memory_copy(state.instruction.memory + address +
                (direction ? 0u - item * width : item * width),
                &value, width);
        else if (item == 0u)
            state.input_value = value;
    }
    before = state.instruction.cpu;
    lib_memory_copy(state.instruction.memory, code, bytes);
    for (item = 0u; item != (repeat ? (count == 0u ? 1u : count) : 1u); ++item)
        core_machine_cpu_execution_refresh(&state.instruction.execution);
    after = state.instruction.cpu;
    if (state.instruction.execution.stop_requested ||
        state.instruction.fault.valid || after.data.eip != bytes ||
        state.transfer_count != count || state.complete_count != count ||
        after.data.ecx != (repeat ? (address32 ? 0u : 0x11220000u) :
            before.data.ecx) ||
        after.data.eax != before.data.eax || after.data.edx != before.data.edx ||
        after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
        after.data.ebp != before.data.ebp ||
        after.data.eflags != before.data.eflags ||
        after.data.esi != (input ? before.data.esi :
            before.data.esi + (direction ? 0u - count * width : count * width)) ||
        after.data.edi != (input ? before.data.edi +
            (direction ? 0u - count * width : count * width) :
            before.data.edi)) return 0;
    if (count == 0u && (state.instruction.memory[0x20010u] != 0x5au ||
        state.instruction.memory[0x30020u] != 0xa5u)) return 0;
    for (item = 0u; item != count; ++item) {
        const cpu_port_transfer_record *transfer = &state.transfers[item];
        lib_u32 expected = input ? state.input_value : 0x11u + item * 0x11u;
        lib_u32 observed = 0u;

        lib_memory_copy(&observed, state.instruction.memory + address +
            (direction ? 0u - item * width : item * width), width);
        if (transfer->port != 0x00e0u || transfer->bytes != width ||
            transfer->write == input || (input ? observed != expected :
            transfer->value != expected || observed != expected)) return 0;
    }
    return 1;
}

static lib_i32 port_strings_test_single_and_rep(void)
{
    static const lib_u8 insb[] = {0x6cu}, insw[] = {0x6du};
    static const lib_u8 outsb[] = {0x6eu}, outsw[] = {0x6fu};
    static const lib_u8 insd[] = {0x66u,0x6du}, outsd[] = {0x66u,0x6fu};
    static const lib_u8 ins32[] = {0x67u,0x6cu}, out32[] = {0x67u,0x6eu};
    static const lib_u8 ins_both[] = {0x66u,0x67u,0x6du};
    static const lib_u8 out_both[] = {0x66u,0x67u,0x6fu};
    static const lib_u8 rep_insb[] = {0xf3u,0x6cu};
    static const lib_u8 rep_insw[] = {0xf3u,0x6du};
    static const lib_u8 rep_outsb[] = {0xf3u,0x6eu};
    static const lib_u8 rep_outsw[] = {0xf3u,0x6fu};
    static const lib_u8 rep_insd[] = {0xf3u,0x66u,0x67u,0x6du};
    static const lib_u8 rep_outsd[] = {0xf3u,0x66u,0x67u,0x6fu};
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile;

    for (profile = 0u; profile != 2u; ++profile) {
#define CASE(code, input, width, address32, count) \
        port_strings_case(profiles[profile], code, sizeof(code), input, \
            width, address32, count, LIB_FALSE, 0x20000u)
        if (!CASE(insb, LIB_TRUE, 1u, LIB_FALSE, 1u) ||
            !CASE(insw, LIB_TRUE, 2u, LIB_FALSE, 1u) ||
            !CASE(outsb, LIB_FALSE, 1u, LIB_FALSE, 1u) ||
            !CASE(outsw, LIB_FALSE, 2u, LIB_FALSE, 1u)) return 0;
        if (profile == 1u) {
            if (!CASE(insd, LIB_TRUE, 4u, LIB_FALSE, 1u) ||
                !CASE(outsd, LIB_FALSE, 4u, LIB_FALSE, 1u) ||
                !CASE(ins32, LIB_TRUE, 1u, LIB_TRUE, 1u) ||
                !CASE(out32, LIB_FALSE, 1u, LIB_TRUE, 1u) ||
                !CASE(ins_both, LIB_TRUE, 4u, LIB_TRUE, 1u) ||
                !CASE(out_both, LIB_FALSE, 4u, LIB_TRUE, 1u)) return 0;
        }
        if (profile == 0u &&
            (!CASE(rep_insb, LIB_TRUE, 1u, LIB_FALSE, 3u) ||
            !CASE(rep_insw, LIB_TRUE, 2u, LIB_FALSE, 3u) ||
            !CASE(rep_outsb, LIB_FALSE, 1u, LIB_FALSE, 3u) ||
            !CASE(rep_outsw, LIB_FALSE, 2u, LIB_FALSE, 3u))) return 0;
        if (profile == 1u &&
            (!CASE(rep_insd, LIB_TRUE, 4u, LIB_TRUE, 3u) ||
            !CASE(rep_outsd, LIB_FALSE, 4u, LIB_TRUE, 3u))) return 0;
#undef CASE
    }
    return 1;
}

static lib_i32 port_strings_test_boundaries(void)
{
    static const lib_u8 rep_ins[] = {0xf3u,0x6cu};
    static const lib_u8 rep_out[] = {0xf3u,0x6eu};
    static const lib_u8 rep_ins32[] = {0xf3u,0x67u,0x6cu};
    static const lib_u8 rep_out32[] = {0xf3u,0x67u,0x6eu};
    static const lib_u8 ins_cs[] = {0x2eu,0x6cu};
    static const lib_u8 ins_fs[] = {0x64u,0x6cu};
    static const lib_u8 out_cs[] = {0x2eu,0x6eu};
    static const lib_u8 out_fs[] = {0x64u,0x6eu};
    static const lib_u8 ins[] = {0x6cu}, out[] = {0x6eu};
    const core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_80386;

#define CASE(code, input, address32, count, direction, base) \
    port_strings_case(profile, code, sizeof(code), input, 1u, address32, \
        count, direction, base)
    return CASE(rep_ins, LIB_TRUE, LIB_FALSE, 0u, LIB_FALSE, 0x20000u) &&
        CASE(rep_out, LIB_FALSE, LIB_FALSE, 0u, LIB_FALSE, 0x20000u) &&
        CASE(rep_ins32, LIB_TRUE, LIB_TRUE, 0u, LIB_FALSE, 0x20000u) &&
        CASE(rep_out32, LIB_FALSE, LIB_TRUE, 0u, LIB_FALSE, 0x20000u) &&
        CASE(rep_ins, LIB_TRUE, LIB_FALSE, 1u, LIB_FALSE, 0x20000u) &&
        CASE(rep_out, LIB_FALSE, LIB_FALSE, 1u, LIB_FALSE, 0x20000u) &&
        CASE(ins_cs, LIB_TRUE, LIB_FALSE, 1u, LIB_FALSE, 0x20000u) &&
        CASE(ins_fs, LIB_TRUE, LIB_FALSE, 1u, LIB_FALSE, 0x20000u) &&
        CASE(out_cs, LIB_FALSE, LIB_FALSE, 1u, LIB_FALSE, 0u) &&
        CASE(out_fs, LIB_FALSE, LIB_FALSE, 1u, LIB_FALSE, 0x40000u) &&
        CASE(ins, LIB_TRUE, LIB_FALSE, 1u, LIB_TRUE, 0x20000u) &&
        CASE(out, LIB_FALSE, LIB_FALSE, 1u, LIB_TRUE, 0x20000u);
#undef CASE
}

static lib_i32 port_strings_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_port_instruction_fixture state;
    t_cpu before, after;
    const lib_u8 source = 0x5au, destination = 0xa5u;

    cpu_port_prepare(&state, profile);
    port_strings_seed(&state);
    state.instruction.cpu.data.idtr.limit = 0x17u;
    state.instruction.memory[0x20010u] = source;
    state.instruction.memory[0x30020u] = destination;
    before = state.instruction.cpu;
    (void)cpu_instruction_run(&state.instruction, code, bytes, &after);
    return state.instruction.fault.valid &&
        (state.instruction.fault.exception_mask & VCPUINS_EXCEPT_UD) &&
        after.data.eip == 0u && after.data.eax == before.data.eax &&
        after.data.ecx == before.data.ecx && after.data.edx == before.data.edx &&
        after.data.ebx == before.data.ebx && after.data.esp == before.data.esp &&
        after.data.ebp == before.data.ebp && after.data.esi == before.data.esi &&
        after.data.edi == before.data.edi &&
        after.data.eflags == before.data.eflags &&
        state.transfer_count == 0u && state.complete_count == 0u &&
        state.instruction.memory[0x20010u] == source &&
        state.instruction.memory[0x30020u] == destination;
}

static lib_i32 port_strings_test_rejections(void)
{
    static const core_machine_cpu_profile pre386[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 forms[][5] = {
        {0x6cu,0,0,0,0}, {0x6du,0,0,0,0}, {0x6eu,0,0,0,0}, {0x6fu,0,0,0,0},
        {0xf3u,0x6cu,0,0,0}, {0xf3u,0x6fu,0,0,0}, {0x66u,0x6du,0,0,0},
        {0x67u,0x6eu,0,0,0}, {0x66u,0x67u,0x6fu,0,0},
        {0xf0u,0x6cu,0,0,0}, {0xf0u,0x6du,0,0,0}, {0xf0u,0x6eu,0,0,0},
        {0xf0u,0x6fu,0,0,0}, {0xf0u,0xf3u,0x66u,0x67u,0x6du}
    };
    static const lib_u8 bytes[] = {1u,1u,1u,1u,2u,2u,2u,2u,3u,2u,2u,2u,2u,5u};
    lib_u8 profile, form;

    for (form = 0u; form != 6u; ++form)
        if (!port_strings_expect_ud(CORE_MACHINE_CPU_PROFILE_8086,
            forms[form], bytes[form])) { lib_c_printf("UD 8086 %u\n",form); return 0; }
    for (profile = 0u; profile != sizeof(pre386) / sizeof(pre386[0]); ++profile)
        for (form = 6u; form != 9u; ++form)
            if (!port_strings_expect_ud(pre386[profile], forms[form],
                bytes[form])) { lib_c_printf("UD old %u %u\n",profile,form); return 0; }
    for (form = 9u; form != sizeof(bytes); ++form)
        if (!port_strings_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
            forms[form], bytes[form])) { lib_c_printf("UD 386 %u\n",form); return 0; }
    return 1;
}

static void port_strings_prepare_protected(cpu_port_instruction_fixture *state,
    lib_bool input, lib_bool repeated)
{
    t_cpu *cpu;

    cpu_port_prepare(state, CORE_MACHINE_CPU_PROFILE_80386);
    port_strings_seed(state);
    cpu = &state->instruction.cpu;
    cpu->data.cr0 |= VCPU_CR0_PE;
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.selector = 0x08u;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = 0x2000u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.seg.executable = LIB_TRUE;
    cpu->data.cs.seg.exec.readable = LIB_TRUE;
    cpu->data.ds.flagValid = LIB_TRUE;
    cpu->data.ds.selector = 0x10u;
    cpu->data.ds.sregtype = SREG_DATA;
    cpu->data.ds.base = 0x3000u;
    cpu->data.ds.limit = input ? 0xffffu : (repeated ? 0x10u : 0x0fu);
    cpu->data.ds.seg.data.writable = LIB_TRUE;
    cpu->data.es.flagValid = LIB_TRUE;
    cpu->data.es.selector = 0x18u;
    cpu->data.es.sregtype = SREG_DATA;
    cpu->data.es.base = 0x4000u;
    cpu->data.es.limit = input ? (repeated ? 0x10u : 0x0fu) : 0xffffu;
    cpu->data.es.seg.data.writable = LIB_TRUE;
    cpu->data.ss.flagValid = LIB_TRUE;
    cpu->data.ss.selector = 0x20u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = 0x5000u;
    cpu->data.ss.limit = 0xffffu;
    cpu->data.ss.seg.data.writable = LIB_TRUE;
    cpu->data.esi = 0x10u;
    cpu->data.edi = 0x10u;
    cpu->data.ecx = 0x11220003u;
    state->input_value = 0x5au;
}

static lib_i32 port_strings_protected_case(lib_bool input, lib_bool repeated)
{
    static const lib_u8 ins[] = {0x6cu};
    static const lib_u8 outs[] = {0x6eu};
    static const lib_u8 rep_ins[] = {0xf3u,0x6cu};
    static const lib_u8 rep_outs[] = {0xf3u,0x6eu};
    const lib_u8 *code = repeated ? (input ? rep_ins : rep_outs) :
        (input ? ins : outs);
    lib_u8 bytes = repeated ? 2u : 1u;
    cpu_port_instruction_fixture state;
    t_cpu before, after;
    const lib_u8 source[] = {0x5au,0x6bu};
    const lib_u8 destination[] = {0xa5u,0xb6u};

    port_strings_prepare_protected(&state, input, repeated);
    lib_memory_copy(state.instruction.memory + 0x3010u, source, sizeof(source));
    lib_memory_copy(state.instruction.memory + 0x4010u, destination,
        sizeof(destination));
    lib_memory_copy(state.instruction.memory + 0x2000u, code, bytes);
    before = state.instruction.cpu;
    core_machine_cpu_execution_refresh(&state.instruction.execution);
    if (repeated && state.instruction.execution.stop_requested) return 0;
    if (repeated)
        core_machine_cpu_execution_refresh(&state.instruction.execution);
    after = state.instruction.cpu;
    return state.instruction.execution.stop_requested &&
        state.instruction.fault.valid &&
        (state.instruction.fault.exception_mask & VCPUINS_EXCEPT_DF) &&
        after.data.eip == 0u && after.data.eax == before.data.eax &&
        after.data.edx == before.data.edx && after.data.ebx == before.data.ebx &&
        after.data.esp == before.data.esp && after.data.ebp == before.data.ebp &&
        after.data.ecx == (repeated ? 0x11220002u : before.data.ecx) &&
        after.data.esi == (repeated && !input ? 0x11u : before.data.esi) &&
        after.data.edi == (repeated && input ? 0x11u : before.data.edi) &&
        after.data.eflags == before.data.eflags &&
        lib_memory_compare(&before.data.ds, &after.data.ds,
            sizeof(before.data.ds)) == 0 &&
        lib_memory_compare(&before.data.es, &after.data.es,
            sizeof(before.data.es)) == 0 &&
        state.transfer_count == (repeated ? 1u : 0u) &&
        state.complete_count == (repeated ? 1u : 0u) &&
        state.instruction.memory[0x3010u] == source[0] &&
        state.instruction.memory[0x3011u] == source[1] &&
        state.instruction.memory[0x4010u] == (repeated && input ?
            source[0] : destination[0]) &&
        state.instruction.memory[0x4011u] == destination[1];
}

lib_i32 main(void)
{
    if (!port_strings_test_single_and_rep()) {
        lib_c_printf("PORT-STRINGS stage=single-rep\n");
        return 1;
    }
    if (!port_strings_test_boundaries()) {
        lib_c_printf("PORT-STRINGS stage=boundaries\n");
        return 1;
    }
    if (!port_strings_test_rejections()) {
        lib_c_printf("PORT-STRINGS stage=rejections\n");
        return 1;
    }
    if (!port_strings_protected_case(LIB_TRUE, LIB_FALSE) ||
        !port_strings_protected_case(LIB_FALSE, LIB_FALSE) ||
        !port_strings_protected_case(LIB_TRUE, LIB_TRUE) ||
        !port_strings_protected_case(LIB_FALSE, LIB_TRUE)) {
        lib_c_printf("PORT-STRINGS stage=protected\n");
        return 1;
    }
    lib_c_printf("M5:T316:S38:PORT-STRINGS:OK\n");
    return 0;
}
