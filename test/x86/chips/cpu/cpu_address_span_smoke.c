#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

typedef struct span_fixture {
    cpu_instruction_fixture chip;
    lib_u32 tracked_first, tracked_last;
    lib_u32 reads, writes, bytes_read, fail_read;
    lib_u32 addresses[8];
    lib_u8 widths[8];
    lib_u32 sparse_address;
    lib_u8 sparse[8];
    lib_u8 sparse_bytes;
    lib_u32 first_gate;
} span_fixture;

static lib_status span_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch)
{
    span_fixture *state = opaque;
    if (!observe_only && provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_DATA &&
        address >= state->tracked_first && address <= state->tracked_last) {
        const lib_u32 index = state->reads++;
        if (index < 8u) {
            state->addresses[index] = address;
            state->widths[index] = bytes;
        }
        if (state->fail_read && state->reads == state->fail_read)
            return LIB_STATUS_IO_ERROR;
        state->bytes_read += bytes;
    }
    if (!observe_only && state->first_gate == LIB_UINT32_MAX &&
        address >= 0x600u && address < 0x680u) state->first_gate = address;
    if (state->sparse_bytes && address >= state->sparse_address &&
        (lib_u64)address + bytes <= (lib_u64)state->sparse_address + state->sparse_bytes) {
        lib_memory_copy(destination, state->sparse + address - state->sparse_address, bytes);
        return LIB_STATUS_OK;
    }
    return cpu_instruction_read(&state->chip, address, destination, bytes,
        provenance, observe_only, reset_fetch);
}

static lib_status span_write(void *opaque, lib_u32 address, const void *source,
    lib_u8 bytes, core_machine_cpu_memory_access_provenance provenance)
{
    span_fixture *state = opaque;
    if (address >= state->tracked_first && address <= state->tracked_last)
        ++state->writes;
    if (state->sparse_bytes && address >= state->sparse_address &&
        (lib_u64)address + bytes <= (lib_u64)state->sparse_address + state->sparse_bytes) {
        lib_memory_copy(state->sparse + address - state->sparse_address, source, bytes);
        return LIB_STATUS_OK;
    }
    return cpu_instruction_write(&state->chip, address, source, bytes, provenance);
}

static const core_machine_cpu_bus_provider span_bus = {
    .read_memory = span_read, .write_memory = span_write,
    .interrupt_pending = cpu_instruction_interrupt_pending
};

static void span_prepare(span_fixture *state, core_machine_cpu_profile profile)
{
    lib_memory_set(state, 0, sizeof(*state));
    cpu_instruction_prepare_with_bus(&state->chip, profile, &span_bus, state);
    state->chip.cpu.data.ds.base = 0x10000u;
    state->chip.cpu.data.ds.selector = 0x1000u;
    state->chip.cpu.data.ss.base = 0x40000u;
    state->chip.cpu.data.ss.selector = 0x4000u;
    state->chip.cpu.data.sp = 0x700u;
    state->tracked_first = 0x10000u;
    state->tracked_last = 0x2ffffu;
    state->first_gate = LIB_UINT32_MAX;
    /* Code-owned real handlers let GP/SS report their actual vector without
     * inventing a firmware input. Protected rejection uses a zero IDT gate. */
    lib_memory_copy(state->chip.memory + 0x30u,
        (const lib_u8[]){0x00u,0x02u,0u,0u,0x00u,0x02u,0u,0u}, 8u);
}

static lib_i32 span_xlat(void)
{
    static span_fixture state;
    t_cpu after;
    for (core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_8086;
        profile <= CORE_MACHINE_CPU_PROFILE_80386; ++profile) {
        for (lib_u8 override = 0u; override < 2u; ++override) {
            const lib_u8 code[] = {override ? 0x26u : 0x3eu, 0xd7u};
            span_prepare(&state, profile);
            state.chip.cpu.data.es = state.chip.cpu.data.ds;
            state.chip.cpu.data.ebx = 0x1234ffffu;
            state.chip.cpu.data.eax = 0x12340001u;
            state.chip.memory[0x10000u] = 0xa5u;
            state.chip.memory[0x20000u] = 0x5au;
            if (cpu_instruction_run(&state.chip, code, sizeof(code), &after) !=
                LIB_STATUS_OK || after.data.eax != 0x123400a5u ||
                state.reads != 1u || state.addresses[0] != 0x10000u) return 1;
        }
    }
    span_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.chip.cpu.data.ds.limit = 0x1ffffu;
    state.chip.cpu.data.ebx = 0xffffu;
    state.chip.cpu.data.al = 1u;
    state.chip.memory[0x20000u] = 0x5au;
    return cpu_instruction_run(&state.chip, (const lib_u8[]){0x67u,0xd7u}, 2u,
        &after) != LIB_STATUS_OK || after.data.al != 0x5au ||
        state.reads != 1u || state.addresses[0] != 0x20000u;
}

static lib_i32 span_real_limits(void)
{
    static span_fixture state;
    static const lib_u8 word[] = {0xa1u,0xffu,0xffu};
    static const lib_u8 store[] = {0xa3u,0xffu,0xffu};
    t_cpu after;
    for (core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_8086;
        profile <= CORE_MACHINE_CPU_PROFILE_80386; ++profile) {
        for (lib_u8 write = 0u; write < 2u; ++write) {
            span_prepare(&state, profile);
            state.chip.cpu.data.ax = 0x5678u;
            state.chip.memory[0x1ffffu] = 0x34u;
            state.chip.memory[0x10000u] = 0x12u;
            if (cpu_instruction_run(&state.chip, write ? store : word, 3u,
                &after) != LIB_STATUS_OK) return 1;
            if (profile < CORE_MACHINE_CPU_PROFILE_80286) {
                if (write ? state.writes != 2u :
                    (after.data.ax != 0x1234u || state.reads != 2u ||
                    state.addresses[1] != 0x10000u)) return 1;
            } else if (state.reads || state.writes ||
                !state.chip.delivered_exception.valid ||
                state.chip.delivered_exception.exception_mask != VCPUINS_EXCEPT_GP ||
                after.data.eip != 0x200u || after.data.ax != 0x5678u) return 1;
        }
    }
    /* An ordinary SS operand has a valid delivery stack: it is not shutdown. */
    span_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.tracked_first = 0x4fff0u; state.tracked_last = 0x50003u;
    state.chip.cpu.data.ax = 0x5678u;
    if (cpu_instruction_run(&state.chip, (const lib_u8[]){0x36u,0xa1u,0xffu,0xffu},
        4u, &after) != LIB_STATUS_OK || state.reads || state.writes ||
        state.chip.delivered_exception.exception_mask != VCPUINS_EXCEPT_SS ||
        after.data.eip != 0x200u || after.data.ax != 0x5678u ||
        state.chip.execution.shutdown_requested) return 1;
    return 0;
}

static lib_i32 span_expand_down(void)
{
    static span_fixture state;
    static const lib_u32 limits[] = {0u,0xfffeu,0xffffu,0xfffffffeu,0xffffffffu};
    static const lib_u32 offsets[] = {0u,1u,0xfffcu,0xfffeu,0xffffu,
        0xfffffffcu,0xfffffffeu,0xffffffffu};
    t_cpu after;
    for (core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_80286;
        profile <= CORE_MACHINE_CPU_PROFILE_80386; ++profile) {
        for (lib_u8 stack = 0u; stack < 2u; ++stack) {
            for (lib_u8 big = 0u; big < 2u; ++big) {
                if (profile == CORE_MACHINE_CPU_PROFILE_80286 && big) continue;
                for (lib_size l = 0u; l < sizeof(limits)/sizeof(limits[0]); ++l) {
                    if (profile == CORE_MACHINE_CPU_PROFILE_80286 && l > 2u) continue;
                    for (lib_size o = 0u; o < sizeof(offsets)/sizeof(offsets[0]); ++o) {
                        if (profile == CORE_MACHINE_CPU_PROFILE_80286 && o > 4u) continue;
                        for (lib_u8 width = 1u; width <= 4u; width *= 2u) {
                            if (profile == CORE_MACHINE_CPU_PROFILE_80286 && width == 4u) continue;
                            lib_u8 code[9]; lib_u8 n = 0u;
                            const lib_u32 upper = big ? LIB_UINT32_MAX : 0xffffu;
                            const lib_bool valid = offsets[o] > limits[l] &&
                                offsets[o] <= upper && width - 1u <= upper - offsets[o];
                            span_prepare(&state, profile);
                            state.chip.cpu.data.cr0 |= VCPU_CR0_PE;
                            state.chip.cpu.data.cs.selector = 0x08u;
                            state.chip.cpu.data.idtr.base = 0x600u;
                            state.chip.cpu.data.idtr.limit = 0x7fu;
                            t_cpu_data_sreg *segment = stack ? &state.chip.cpu.data.ss : &state.chip.cpu.data.ds;
                            segment->selector = stack ? 0x18u : 0x10u;
                            segment->seg.data.expdown = LIB_TRUE;
                            segment->seg.data.big = big ? LIB_TRUE : LIB_FALSE;
                            segment->limit = limits[l];
                            state.sparse_address = segment->base + offsets[o];
                            state.sparse_bytes = width;
                            lib_memory_set(state.sparse,0xa5u,width);
                            state.tracked_first = state.sparse_address;
                            state.tracked_last = state.sparse_address + width - 1u;
                            code[n++] = stack ? 0x36u : 0x3eu;
                            if (width == 4u) code[n++] = 0x66u;
                            if (profile == CORE_MACHINE_CPU_PROFILE_80386) code[n++] = 0x67u;
                            code[n++] = width == 1u ? 0xa0u : 0xa1u;
                            const lib_u8 address_bytes = profile == CORE_MACHINE_CPU_PROFILE_80386 ? 4u : 2u;
                            lib_memory_copy(code + n, &offsets[o],address_bytes); n += address_bytes;
                            (void)cpu_instruction_run(&state.chip, code, n, &after);
                            if (valid ? (state.bytes_read != width || after.data.al != 0xa5u ||
                                state.chip.execution.stop_requested) :
                                (state.reads != 0u || state.first_gate !=
                                0x600u + (stack ? 12u : 13u) * 8u)) return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

static lib_i32 span_bit_indices(void)
{
    static span_fixture state;
    static const lib_u8 opcodes[] = {0xa3u,0xabu,0xb3u,0xbbu};
    static const lib_i32 indices[] = {-1,-16,-32,0,15,16,31,32,(-2147483647 - 1)};
    t_cpu after;
    for (lib_u8 size = 2u; size <= 4u; size += 2u) {
        for (lib_u8 address32 = 0u; address32 < 2u; ++address32) {
            for (lib_u8 form = 0u; form < 4u; ++form) {
                for (lib_size i = 0u; i < sizeof(indices)/sizeof(indices[0]); ++i) {
                    lib_u8 code[10]; lib_u8 n = 0u;
                    const lib_i64 index = size == 2u ? (lib_i16)indices[i] : indices[i];
                    const lib_i64 quotient = index >= 0 ? index / (size * 8u) :
                        (index - (size * 8u - 1u)) / (size * 8u);
                    lib_u32 offset = (lib_u32)(0x200u + quotient * size);
                    if (!address32) offset &= 0xffffu;
                    const lib_u32 bit = (lib_u32)index & (size * 8u - 1u);
                    const lib_u32 mask = 1u << bit;
                    const lib_u32 initial = size == 2u ? 0xffffu : LIB_UINT32_MAX;
                    const lib_u32 expected = form < 2u ? initial : initial & ~mask;
                    span_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
                    state.chip.cpu.data.ds.limit = LIB_UINT32_MAX;
                    state.chip.cpu.data.ecx = (lib_u32)indices[i];
                    state.sparse_address = 0x10000u + offset;
                    state.sparse_bytes = size;
                    lib_memory_copy(state.sparse, &initial, size);
                    state.tracked_first = state.sparse_address; state.tracked_last = state.sparse_address;
                    if (size == 4u) code[n++] = 0x66u;
                    if (address32) code[n++] = 0x67u;
                    code[n++] = 0x0fu; code[n++] = opcodes[form];
                    code[n++] = address32 ? 0x0du : 0x0eu;
                    code[n++] = 0u; code[n++] = 2u;
                    if (address32) {code[n++] = 0u; code[n++] = 0u;}
                    lib_u32 observed = 0u;
                    if (cpu_instruction_run(&state.chip, code, n, &after) != LIB_STATUS_OK ||
                        state.reads != 1u || state.addresses[0] != state.sparse_address ||
                        !X86_CPU_BIT_IS_SET(after.data.eflags,VCPU_EFLAGS_CF)) return 1;
                    lib_memory_copy(&observed, state.sparse, size);
                    if (observed != expected) return 1;
                }
            }
        }
    }
    return 0;
}

static lib_i32 span_bit_immediates(void)
{
    static span_fixture state;
    static const lib_u8 immediates[] = {0u,15u,16u,31u,32u,33u,127u,128u,255u};
    t_cpu after;
    for (lib_u8 size = 2u; size <= 4u; size += 2u) {
        for (lib_u8 form = 0u; form < 4u; ++form) {
            for (lib_size i = 0u; i < sizeof(immediates)/sizeof(immediates[0]); ++i) {
                lib_u8 code[8]; lib_u8 n = 0u;
                const lib_u32 bit = immediates[i] & (size * 8u - 1u);
                const lib_u32 initial = size == 2u ? 0xffffu : LIB_UINT32_MAX;
                const lib_u32 expected = form < 2u ? initial : initial & ~(1u << bit);
                lib_u32 observed = 0u;
                span_prepare(&state,CORE_MACHINE_CPU_PROFILE_80386);
                lib_memory_copy(state.chip.memory + 0x10200u,&initial,size);
                if (size == 4u) code[n++] = 0x66u;
                code[n++] = 0x0fu; code[n++] = 0xbau;
                code[n++] = 0x26u + form * 8u;
                code[n++] = 0u; code[n++] = 2u; code[n++] = immediates[i];
                if (cpu_instruction_run(&state.chip,code,n,&after) != LIB_STATUS_OK ||
                    state.reads != 1u || state.addresses[0] != 0x10200u ||
                    !X86_CPU_BIT_IS_SET(after.data.eflags,VCPU_EFLAGS_CF)) return 1;
                lib_memory_copy(&observed,state.chip.memory + 0x10200u,size);
                if (observed != expected) return 1;
            }
        }
    }
    return 0;
}


/* All fifteen paired-read callers retain their physical scalar phases.
 * Legal boundary/wrap and a rejected first/second provider are independent. */
static lib_i32 span_composite_phases(void)
{
    static span_fixture state;
    static const struct {
        lib_u8 opcode[2], count, reg, kind;
        core_machine_cpu_profile minimum;
    } forms[] = {
        {{0x62u,0u},1u,0u,0u,CORE_MACHINE_CPU_PROFILE_80186},
        {{0xc4u,0u},1u,0u,1u,CORE_MACHINE_CPU_PROFILE_8086},
        {{0xc5u,0u},1u,0u,1u,CORE_MACHINE_CPU_PROFILE_8086},
        {{0xffu,0u},1u,3u,1u,CORE_MACHINE_CPU_PROFILE_8086},
        {{0xffu,0u},1u,5u,1u,CORE_MACHINE_CPU_PROFILE_8086},
        {{0x0fu,0xb2u},2u,0u,1u,CORE_MACHINE_CPU_PROFILE_80386},
        {{0x0fu,0xb4u},2u,0u,1u,CORE_MACHINE_CPU_PROFILE_80386},
        {{0x0fu,0xb5u},2u,0u,1u,CORE_MACHINE_CPU_PROFILE_80386},
        {{0x0fu,0x01u},2u,2u,2u,CORE_MACHINE_CPU_PROFILE_80286},
        {{0x0fu,0x01u},2u,3u,2u,CORE_MACHINE_CPU_PROFILE_80286}
    };
    t_cpu before, after;
    for (core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_8086;
        profile <= CORE_MACHINE_CPU_PROFILE_80386; ++profile) {
        for (lib_size f = 0u; f < sizeof(forms)/sizeof(forms[0]); ++f) {
            if (profile < forms[f].minimum) continue;
            for (lib_u8 size = 2u; size <=
                (profile == CORE_MACHINE_CPU_PROFILE_80386 ? 4u : 2u); size += 2u) {
                const lib_u8 first = forms[f].kind == 2u ? 2u : size;
                const lib_u8 second = forms[f].kind == 2u ? 4u :
                    forms[f].kind == 0u ? size : 2u;
                for (lib_u8 address32 = 0u; address32 <
                    (profile == CORE_MACHINE_CPU_PROFILE_80386 ? 2u : 1u); ++address32) {
                    for (lib_u8 test = 0u; test < 8u; ++test) {
                        if (test == 7u && profile < CORE_MACHINE_CPU_PROFILE_80286) continue;
                        lib_u8 code[10], image[8] = {0}; lib_u8 n = 0u;
                        lib_u32 start = test < 3u || test == 7u ? 0x200u :
                            test == 3u ? 0x10000u - first - second :
                            test == 4u ? 0x10001u - first - second :
                            test == 5u ? 0xfffeu : 0xffffu;
                        const lib_u32 low = forms[f].kind == 0u ? 0u : 0x200u;
                        const lib_u32 high = forms[f].kind == 0u ? 4u :
                            forms[f].kind == 2u ? 0x300u : 0x100u;
                        span_prepare(&state, profile);
                        if (test == 7u) {
                            state.chip.cpu.data.cr0 |= VCPU_CR0_PE;
                            state.chip.cpu.data.cs.selector = 0x08u;
                            state.chip.cpu.data.ds.selector = 0x10u;
                            state.chip.cpu.data.ds.limit = start + first + second - 2u;
                            state.chip.cpu.data.idtr.base = 0x600u;
                            state.chip.cpu.data.idtr.limit = 0x7fu;
                        }
                        state.chip.cpu.data.ax = 2u;
                        lib_memory_copy(image, &low, first);
                        lib_memory_copy(image + first, &high, second);
                        for (lib_u8 b = 0u; b < first + second; ++b) {
                            lib_u32 position = start + b;
                            if (profile < CORE_MACHINE_CPU_PROFILE_80286) position &= 0xffffu;
                            state.chip.memory[0x10000u + position] = image[b];
                        }
                        state.fail_read = test == 1u ? 1u : test == 2u ? 2u : 0u;
                        before = state.chip.cpu;
                        if (size == 4u) code[n++] = 0x66u;
                        if (address32) code[n++] = 0x67u;
                        for (lib_u8 b = 0u; b < forms[f].count; ++b) code[n++] = forms[f].opcode[b];
                        code[n++] = (address32 ? 0x05u : 0x06u) | (forms[f].reg << 3u);
                        code[n++] = (lib_u8)start; code[n++] = (lib_u8)(start >> 8u);
                        if (address32) {code[n++] = 0u; code[n++] = 0u;}
                        const lib_status status = cpu_instruction_run(&state.chip, code, n, &after);
                        if (state.fail_read) {
                            if (status != LIB_STATUS_INTERNAL_ERROR ||
                                state.reads != state.fail_read || state.writes ||
                                state.chip.fault.exception_mask != VCPUINS_EXCEPT_CE ||
                                state.chip.fault.exception_code != 0x10000u + start +
                                (state.fail_read == 2u ? first : 0u) ||
                                state.chip.fault.point.eip != before.data.eip ||
                                lib_memory_compare(&before,&after,sizeof(before))) return 1;
                        } else if (test == 7u) {
                            if (state.reads || state.writes || state.first_gate != 0x668u ||
                                after.data.eax != before.data.eax) return 1;
                        } else if (profile >= CORE_MACHINE_CPU_PROFILE_80286 &&
                            (lib_u64)start + first + second > 0x10000u) {
                            if (status != LIB_STATUS_OK || state.reads || state.writes ||
                                state.chip.delivered_exception.exception_mask != VCPUINS_EXCEPT_GP)
                                return 1;
                        } else {
                            if (status != LIB_STATUS_OK || state.chip.fault.valid ||
                                state.chip.delivered_exception.valid ||
                                state.bytes_read != first + second ||
                                state.addresses[0] != 0x10000u + start) return 1;
                            if ((f == 3u || f == 4u) &&
                                (after.data.cs.selector != 0x100u ||
                                 after.data.eip != 0x200u)) return 1;
                            if (test < 4u && (state.reads != 2u || state.widths[0] != first ||
                                state.widths[1] != second ||
                                state.addresses[1] != 0x10000u + start + first)) return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

static lib_i32 span_vm_and_cached_limits(void)
{
    static span_fixture state;
    t_cpu after;
    for (lib_u8 vm = 0u; vm < 2u; ++vm) {
        for (lib_u8 wide = 0u; wide < 2u; ++wide) {
            /* VM86 entry creates 64K caches; an invented wider VM cache is
             * not a hardware eligibility oracle. PE-clear preserves caches. */
            if (vm && wide) continue;
            lib_u8 code[] = {0x67u,0xa0u,0u,0u,1u,0u};
            span_prepare(&state,CORE_MACHINE_CPU_PROFILE_80386);
            state.chip.cpu.data.ds.limit = wide ? 0x1ffffu : 0xffffu;
            state.chip.memory[0x20000u] = 0xa5u;
            if (vm) {
                state.chip.cpu.data.cr0 |= VCPU_CR0_PE;
                state.chip.cpu.data.eflags |= VCPU_EFLAGS_VM;
                state.chip.cpu.data.idtr.base = 0x600u;
                state.chip.cpu.data.idtr.limit = 0x7fu;
            }
            (void)cpu_instruction_run(&state.chip,code,sizeof(code),&after);
            if (wide) {
                if (state.reads != 1u || after.data.al != 0xa5u) return 1;
            } else if (state.reads || (vm ? state.first_gate != 0x668u :
                state.chip.delivered_exception.exception_mask != VCPUINS_EXCEPT_GP)) return 1;
        }
    }
    return 0;
}

lib_i32 main(void)
{
    static const struct {const char *name; lib_i32 (*run)(void);} cases[] = {
        {"xlat",span_xlat}, {"real-limits",span_real_limits},
        {"expand-down",span_expand_down}, {"bit-indices",span_bit_indices},
        {"bit-immediates",span_bit_immediates},
        {"composite-phases",span_composite_phases},
        {"vm-cached-limits",span_vm_and_cached_limits}
    };
    for (lib_size i = 0u; i < sizeof(cases)/sizeof(cases[0]); ++i) {
        if (cases[i].run()) {
            lib_c_printf("CPU-ADDRESS-SPAN:%s:FAILED\n",cases[i].name);
            return 1;
        }
    }
    lib_c_printf("CPU-ADDRESS-SPAN:OK\n");
    return 0;
}
