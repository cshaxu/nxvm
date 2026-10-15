#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

typedef struct integer_case {
    const char *name;
    lib_u8 width;
    lib_u8 extension;
    lib_u32 accumulator;
    lib_u32 high;
    lib_u32 source;
    lib_u32 result;
    lib_u32 remainder;
    lib_bool fault;
    lib_bool carry;
} integer_case;

/* Fixed boundary results, not another implementation of multiply or divide. */
static const integer_case integer_cases[] = {
    {"mul8-high", 8u,4u,0xffu,0u,2u,0x1feu,0u,LIB_FALSE,LIB_TRUE},
    {"mul8-zero", 8u,4u,0u,0u,0xffu,0u,0u,LIB_FALSE,LIB_FALSE},
    {"imul8-negative", 8u,5u,0xffu,0u,2u,0xfffeu,0u,LIB_FALSE,LIB_FALSE},
    {"imul8-overflow", 8u,5u,0x80u,0u,0xffu,0x80u,0u,LIB_FALSE,LIB_TRUE},
    {"div8-remainder", 8u,6u,0xffu,0u,2u,0x17fu,0u,LIB_FALSE,LIB_FALSE},
    {"div8-limit", 8u,6u,0x1feu,0u,2u,0xffu,0u,LIB_FALSE,LIB_FALSE},
    {"div8-overflow", 8u,6u,0x100u,0u,1u,0u,0u,LIB_TRUE,LIB_FALSE},
    {"div8-zero", 8u,6u,0x1234u,0u,0u,0u,0u,LIB_TRUE,LIB_FALSE},
    {"idiv8-limit", 8u,7u,0xff80u,0u,1u,0x80u,0u,LIB_FALSE,LIB_FALSE},
    {"idiv8-overflow", 8u,7u,0xff80u,0u,0xffu,0u,0u,LIB_TRUE,LIB_FALSE},
    {"idiv8-minimum", 8u,7u,0x8000u,0u,0xffu,0u,0u,LIB_TRUE,LIB_FALSE},
    {"idiv8-negative", 8u,7u,0xfff6u,0u,3u,0xfffdu,0u,LIB_FALSE,LIB_FALSE},
    {"mul16-high", 16u,4u,0xffffu,0u,2u,0xfffeu,1u,LIB_FALSE,LIB_TRUE},
    {"mul16-full", 16u,4u,0xffffu,0u,0xffffu,1u,0xfffeu,LIB_FALSE,LIB_TRUE},
    {"imul16-limit", 16u,5u,0x8000u,0u,1u,0x8000u,0xffffu,LIB_FALSE,LIB_FALSE},
    {"imul16-overflow", 16u,5u,0x8000u,0u,0xffffu,0x8000u,0u,LIB_FALSE,LIB_TRUE},
    {"div16-remainder", 16u,6u,0xffffu,0u,2u,0x7fffu,1u,LIB_FALSE,LIB_FALSE},
    {"div16-wide", 16u,6u,0u,1u,0xffffu,1u,1u,LIB_FALSE,LIB_FALSE},
    {"div16-overflow", 16u,6u,0xffffu,0xffffu,0xffffu,0u,0u,LIB_TRUE,LIB_FALSE},
    {"div16-zero", 16u,6u,0x1234u,0u,0u,0u,0u,LIB_TRUE,LIB_FALSE},
    {"idiv16-limit", 16u,7u,0x8000u,0xffffu,1u,0x8000u,0u,LIB_FALSE,LIB_FALSE},
    {"idiv16-overflow", 16u,7u,0x8000u,0xffffu,0xffffu,0u,0u,LIB_TRUE,LIB_FALSE},
    {"idiv16-minimum", 16u,7u,0u,0x8000u,0xffffu,0u,0u,LIB_TRUE,LIB_FALSE},
    {"idiv16-negative", 16u,7u,0xfff6u,0xffffu,3u,0xfffdu,0xffffu,LIB_FALSE,LIB_FALSE},
    {"mul32-high", 32u,4u,0xffffffffu,0u,2u,0xfffffffeu,1u,LIB_FALSE,LIB_TRUE},
    {"mul32-full", 32u,4u,0xffffffffu,0u,0xffffffffu,1u,0xfffffffeu,LIB_FALSE,LIB_TRUE},
    {"imul32-limit", 32u,5u,0x80000000u,0u,1u,0x80000000u,0xffffffffu,LIB_FALSE,LIB_FALSE},
    {"imul32-overflow", 32u,5u,0x80000000u,0u,0xffffffffu,0x80000000u,0u,LIB_FALSE,LIB_TRUE},
    {"div32-remainder", 32u,6u,0xffffffffu,0u,2u,0x7fffffffu,1u,LIB_FALSE,LIB_FALSE},
    {"div32-wide", 32u,6u,0u,1u,0xffffffffu,1u,1u,LIB_FALSE,LIB_FALSE},
    {"div32-overflow", 32u,6u,0xffffffffu,0xffffffffu,0xffffffffu,0u,0u,LIB_TRUE,LIB_FALSE},
    {"div32-zero", 32u,6u,0x12345678u,0u,0u,0u,0u,LIB_TRUE,LIB_FALSE},
    {"idiv32-limit", 32u,7u,0x80000000u,0xffffffffu,1u,0x80000000u,0u,LIB_FALSE,LIB_FALSE},
    {"idiv32-overflow", 32u,7u,0x80000000u,0xffffffffu,0xffffffffu,0u,0u,LIB_TRUE,LIB_FALSE},
    {"idiv32-minimum", 32u,7u,0u,0x80000000u,0xffffffffu,0u,0u,LIB_TRUE,LIB_FALSE},
    {"idiv32-negative", 32u,7u,0xfffffff6u,0xffffffffu,3u,0xfffffffdu,0xffffffffu,LIB_FALSE,LIB_FALSE}
};

static lib_status integer_failed_read(void *opaque, lib_u32 address,
    void *destination, lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    if (address == 0x5000u && !observe_only &&
        provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA) return LIB_STATUS_IO_ERROR;
    return cpu_instruction_read(opaque, address, destination, bytes, provenance,
        observe_only, reset_fetch, cache_control);
}

static const x86_core_cpu_bus_provider integer_failed_bus = {
    .read_memory = integer_failed_read,
    .write_memory = cpu_instruction_write,
    .interrupt_pending = cpu_instruction_interrupt_pending
};

static lib_bool integer_run(cpu_instruction_fixture *state,
    x86_core_cpu_profile profile, const integer_case *item,
    lib_u8 mode, lib_bool code_32, lib_bool address_32, lib_bool memory,
    lib_bool fail_source)
{
    const lib_u32 mask = item->width == 32u ? LIB_UINT32_MAX : 0xffffu;
    const lib_u32 operand_mask = item->width == 8u ? 0xffu : mask;
    const lib_u8 source_bytes = item->width / 8u;
    const lib_u32 control_flags = VCPU_EFLAGS_IF | VCPU_EFLAGS_DF | VCPU_EFLAGS_VM;
    lib_u8 code[8] = {0};
    lib_u8 bytes = 0u;
    t_cpu before;
    lib_bool valid;

    x86_core_cpu_execution_finalize(&state->execution);
    cpu_instruction_prepare_with_bus(state, profile,
        fail_source ? &integer_failed_bus : &cpu_instruction_bus, state);
    if (mode != 0u) {
        const lib_u8 cpl = mode == 5u ? 3u : mode - 1u;

        cpu_instruction_prepare_protected_exception(state, mode == 5u ? 4u : cpl);
        state->cpu.data.ds.selector = (lib_u16)(0x10u | cpl);
        state->cpu.data.ds.dpl = cpl;
        /* DE uses the same real gate/stack wiring, not the GP vector. */
        lib_memory_copy(state->memory + 0x400u, state->memory + 0x468u, 8u);
    } else {
        const lib_u16 vector[] = {0x300u,0u};

        lib_memory_copy(state->memory, vector, sizeof(vector));
        state->cpu.data.esp = 0x8000u;
    }
    state->cpu.data.cs.seg.exec.defsize = code_32;
    if (item->width != 8u && code_32 != (item->width == 32u)) code[bytes++] = 0x66u;
    if (address_32 != code_32) code[bytes++] = 0x67u;
    code[bytes++] = item->width == 8u ? 0xf6u : 0xf7u;
    code[bytes++] = (lib_u8)((item->extension << 3u) |
        (memory ? (address_32 ? 5u : 6u) : 0xc1u));
    if (memory) {
        code[bytes++] = 0u;
        code[bytes++] = 0x50u;
        if (address_32) {
            code[bytes++] = 0u;
            code[bytes++] = 0u;
        }
    }
    state->cpu.data.eax = (0x11220000u & ~mask) | item->accumulator;
    state->cpu.data.edx = item->width == 8u ? 0x778899aau :
        (0x77880000u & ~mask) | item->high;
    state->cpu.data.ecx = (0x55667700u & ~operand_mask) | item->source;
    state->cpu.data.ebx = 0x12345678u;
    state->cpu.data.eip = 0x1000u;
    state->cpu.data.eflags |= VCPU_EFLAGS_IF | VCPU_EFLAGS_DF |
        VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
    lib_memory_copy(state->memory + 0x5000u, &item->source, source_bytes);
    lib_memory_copy(state->memory + 0x1000u, code, bytes);
    before = state->cpu;
    x86_core_cpu_execution_refresh(&state->execution);
    valid = !state->execution.stop_requested && !state->fault.valid &&
        state->cpu.data.ecx == before.data.ecx && state->cpu.data.ebx == before.data.ebx &&
        lib_memory_compare(state->memory + 0x5000u, &item->source, source_bytes) == 0;
    if (fail_source) {
        valid = state->execution.stop_requested && state->fault.valid &&
            !state->delivered_exception.valid &&
            state->fault.exception_mask == VCPUINS_EXCEPT_CE &&
            state->fault.exception_code == 0x5000u &&
            lib_memory_compare(&before, &state->cpu, sizeof(before)) == 0;
    } else if (item->fault) {
        valid = valid && state->delivered_exception.valid &&
            state->delivered_exception.exception_mask == VCPUINS_EXCEPT_DE &&
            state->delivered_exception.exception_code == 0u &&
            state->delivered_exception.point.eip == before.data.eip &&
            state->delivered_exception.point.cs == before.data.cs.selector &&
            state->cpu.data.eip == 0x300u && state->cpu.data.eax == before.data.eax &&
            state->cpu.data.edx == before.data.edx;
    } else {
        valid = valid && !state->delivered_exception.valid &&
            state->cpu.data.eip == 0x1000u + bytes &&
            state->cpu.data.eax == ((before.data.eax & ~mask) | item->result) &&
            state->cpu.data.edx == (item->width == 8u ? before.data.edx :
                (before.data.edx & ~mask) | item->remainder) &&
            (state->cpu.data.eflags & control_flags) == (before.data.eflags & control_flags);
        if (item->extension < 6u)
            valid = valid && (state->cpu.data.eflags & (VCPU_EFLAGS_CF | VCPU_EFLAGS_OF)) ==
                (item->carry ? VCPU_EFLAGS_CF | VCPU_EFLAGS_OF : 0u);
    }
    if (!valid)
        lib_c_fprintf(lib_c_stderr,
            "INTEGER: %s cpu=%u mode=%u code32=%u address32=%u memory=%u fail=%u eip=%08X exception=%08X\n",
            item->name, (lib_u32)profile, (lib_u32)mode, (lib_u32)code_32,
            (lib_u32)address_32, (lib_u32)memory, (lib_u32)fail_source, state->cpu.data.eip,
            state->delivered_exception.exception_mask);
    return valid;
}

lib_i32 main(void)
{
    const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_80386, X86_CORE_CPU_PROFILE_80486SX
    };
    cpu_instruction_fixture *state = lib_allocate_zero(1u, sizeof(*state));
    lib_bool valid = state != LIB_NULL;

    for (lib_size profile = 0u; valid && profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (lib_size item = 0u; valid && item != sizeof(integer_cases) / sizeof(integer_cases[0]); ++item)
    for (lib_u8 mode = 0u; valid && mode != 6u; ++mode)
    for (lib_u8 code_32 = 0u; valid && code_32 != (mode == 0u || mode == 5u ? 1u : 2u); ++code_32)
    for (lib_u8 address_32 = 0u; valid && address_32 != 2u; ++address_32)
    for (lib_u8 memory = 0u; valid && memory != 2u; ++memory) {
        valid = integer_run(state, profiles[profile], &integer_cases[item], mode,
            code_32, address_32, memory, LIB_FALSE);
        if (valid && memory)
            valid = integer_run(state, profiles[profile], &integer_cases[item], mode,
                code_32, address_32, memory, LIB_TRUE);
    }
    if (state != LIB_NULL) x86_core_cpu_execution_finalize(&state->execution);
    lib_release(state);
    if (!valid) return 1;
    lib_c_printf("CPU-INTEGER-MULTIPLY-DIVIDE:OK\n");
    return 0;
}
