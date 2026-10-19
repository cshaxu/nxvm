#ifndef TEST_CPU_V30_STRING_TIMING_H
#define TEST_CPU_V30_STRING_TIMING_H

#include "lib/types/file.h"
#include "x86-core/cpu/cpu.h"
#include "x86-core/cpu/cpu_instructions.h"
#include "cpu_v30_timing.h"

/* Independent NEC section 12 rows: even, one odd endpoint, two odd endpoints.
 * MOVBK's printed repeat row deliberately differs from its primitive row. */
static lib_i32 v30_string_timing(const x86_core_cpu_execution_context *execution,
    const t_cpu *before, lib_u8 opcode, lib_u8 prefix, lib_u8 ordinal,
    lib_u32 additional_prefixes, lib_u32 prefix_inputs,
    lib_u32 fetch, lib_u64 events)
{
    static const struct {
        lib_u8 opcode, setup, primitive[3], iteration[3];
    } rows[] = {
        {0xa4u,11u,{11u,15u,19u},{8u,16u,16u}},
        {0xa6u,7u,{13u,17u,21u},{14u,18u,22u}},
        {0xaeu,7u,{7u,11u,11u},{10u,14u,14u}},
        {0xacu,7u,{7u,11u,11u},{9u,13u,13u}},
        {0xaau,7u,{7u,11u,11u},{4u,8u,8u}},
        {0x6cu,9u,{10u,14u,18u},{8u,12u,16u}},
        {0x6eu,9u,{10u,14u,18u},{8u,12u,16u}}
    };
    const x86_core_cpu_timing_result actual = x86_core_cpu_capture_timing(execution);
    lib_u64 clocks = 0u;
    lib_u32 inputs = 0u, key = X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED;
    x86_core_cpu_retirement_repeat_phase phase = X86_CORE_CPU_RETIREMENT_REPEAT_NONE;
    lib_size row = 0u;
    while (row < sizeof(rows) / sizeof(rows[0]) && rows[row].opcode != (opcode & 0xfeu)) ++row;
    if (row == sizeof(rows) / sizeof(rows[0])) return 1;
    const lib_bool source = row == 0u || row == 1u || row == 3u || row == 6u;
    const lib_bool destination = row != 3u && row != 6u;
    const lib_u8 alignment = !(opcode & 1u) ? (lib_u8)0u :
        (lib_u8)((source && (before->data.si & 1u)) + (destination && (before->data.di & 1u)) +
        (row >= 5u && (before->data.dx & 1u)));
    key = opcode | ((lib_u32)prefix << 16u);
    if (!prefix) {
        clocks = rows[row].primitive[alignment];
        inputs = X86_CORE_CPU_TIMING_INPUT_MEMORY;
        phase = X86_CORE_CPU_RETIREMENT_REPEAT_PRIMITIVE;
    } else {
        phase = !before->data.cx ? X86_CORE_CPU_RETIREMENT_REPEAT_ZERO_COUNT : ordinal ?
            X86_CORE_CPU_RETIREMENT_REPEAT_CONTINUATION : X86_CORE_CPU_RETIREMENT_REPEAT_FIRST;
        clocks = ordinal ? 0u : rows[row].setup;
        inputs = X86_CORE_CPU_TIMING_INPUT_REPEAT | X86_CORE_CPU_TIMING_INPUT_REPEAT_PHASE;
        if (before->data.cx) {
            clocks += rows[row].iteration[alignment];
            inputs |= X86_CORE_CPU_TIMING_INPUT_MEMORY;
        }
    }
    if ((!prefix || before->data.cx) && alignment) inputs |= X86_CORE_CPU_TIMING_INPUT_ODD_WORD;
    if (!prefix || !ordinal) clocks += 2u * additional_prefixes;
    inputs |= prefix_inputs;
    const x86_core_cpu_retirement_timing_origin origin = prefix && (additional_prefixes ||
        (prefix==0xf2u && row!=1u && row!=2u)) ?
        X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL :
        X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE;
    if (v30_timing_check(execution,(x86_core_cpu_timing_result){
        .ticks=clocks,.form_id=key,.key_id=key,.formula_inputs=inputs,
        .repeat_phase=phase,.retirement_origin=origin,.event_ticks=events},fetch)) {
        lib_c_printf("V30:NOMINAL-STRING:op=%02x prefix=%02x step=%u clocks=%llu/%llu phase=%u/%u\n",
            opcode,prefix,ordinal,(lib_u64)actual.ticks,(lib_u64)clocks,
            actual.repeat_phase,phase);
        return 1;
    }
    return 0;
}

#endif
