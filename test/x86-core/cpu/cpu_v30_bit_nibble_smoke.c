#include "support/cpu_bus_fixture.h"
#include "lib/types/file.h"

_Static_assert(X86_CORE_CPU_INSTRUCTION_PRIMARY == 0 &&
    X86_CORE_CPU_INSTRUCTION_0F == 1 && X86_CORE_CPU_INSTRUCTION_FPU_ESCAPE == 2 &&
    X86_CORE_CPU_INSTRUCTION_NEC_0F == 3, "Existing instruction spaces must not move");

static lib_u16 *v30_word_register(cpu_bus_fixture *fixture, lib_u8 index)
{
    switch (index) {
    case 0: return &fixture->cpu.data.ax;
    case 1: return &fixture->cpu.data.cx;
    case 2: return &fixture->cpu.data.dx;
    case 3: return &fixture->cpu.data.bx;
    case 4: return &fixture->cpu.data.sp;
    case 5: return &fixture->cpu.data.bp;
    case 6: return &fixture->cpu.data.si;
    default: return &fixture->cpu.data.di;
    }
}

static lib_u8 *v30_byte_register(cpu_bus_fixture *fixture, lib_u8 index)
{
    return (lib_u8 *)v30_word_register(fixture, index & 3u) + (index >> 2u);
}

static lib_i32 v30_bit_case(lib_u8 opcode, lib_u8 mode, lib_u8 reg,
    lib_u8 count, lib_u16 value, lib_bool override)
{
    cpu_bus_fixture fixture;
    lib_u8 code[8];
    lib_u8 size = 0u;
    const lib_u8 byte = (lib_u8)(1u + (opcode & 1u));
    const lib_u8 operation = (opcode >> 1u) & 3u;
    lib_u16 address = override ? 0x701u : mode == 2u ? 0x601u : 0x501u;
    lib_u16 original;
    lib_u16 result;
    lib_u16 expected;
    lib_u32 flags;
    lib_u16 mask;
    x86_core_cpu_instruction_lexeme lexeme;

    cpu_bus_prepare(&fixture, X86_CORE_CPU_PROFILE_V30);
    fixture.cpu.data.bx = 0x4fcu;
    fixture.cpu.data.bp = 0x400u;
    fixture.cpu.data.ss.base = 0x100u;
    fixture.cpu.data.es.base = 0x200u;
    fixture.cpu.data.cl = count;
    fixture.cpu.data.eflags = X86_CPU_NEC_PSW_MD | 2u | VCPU_EFLAGS_CF |
        VCPU_EFLAGS_OF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF;
    if (mode == 3u) {
        if (byte == 1u) *v30_byte_register(&fixture, reg) = (lib_u8)value;
        else *v30_word_register(&fixture, reg) = value;
        original = byte == 1u ? *v30_byte_register(&fixture, reg) :
            *v30_word_register(&fixture, reg);
    } else {
        fixture.memory[address] = (lib_u8)value;
        fixture.memory[address + 1u] = (lib_u8)(value >> 8u);
        original = byte == 1u ? (lib_u8)value : value;
    }
    if (override) code[size++] = 0x26u;
    code[size++] = 0x0fu;
    code[size++] = opcode;
    code[size++] = mode == 3u ? (lib_u8)(0xc0u | reg) :
        mode == 0u ? 0x06u : mode == 1u ? 0x47u : 0x86u;
    if (mode == 0u) { code[size++] = 0x01u; code[size++] = 0x05u; }
    if (mode == 1u) code[size++] = 0x05u;
    if (mode == 2u) { code[size++] = 0x01u; code[size++] = 0x01u; }
    if (opcode >= 0x18u) code[size++] = count;
    else count = fixture.cpu.data.cl;
    mask = (lib_u16)(1u << (count & (byte * 8u - 1u)));
    expected = operation == 0u ? original : operation == 1u ?
        (lib_u16)(original & ~mask) : operation == 2u ?
        (lib_u16)(original | mask) : (lib_u16)(original ^ mask);
    flags = fixture.cpu.data.eflags;
    lib_memory_copy(&fixture.memory[0x100u], code, size);
    if (!x86_core_cpu_instruction_lexeme_scan(code, size,
        X86_CORE_CPU_PROFILE_V30, LIB_FALSE, &lexeme) || lexeme.byte_count != size)
        return 1;
    for (lib_u8 available = 0u; available < size; ++available)
        if (x86_core_cpu_instruction_lexeme_scan(code, available,
            X86_CORE_CPU_PROFILE_V30, LIB_FALSE, &lexeme)) return 1;
    if (!x86_core_cpu_execution_preview_lexeme(&fixture.execution, &lexeme) ||
        lexeme.byte_count != size || fixture.writes != 0u) return 1;
    x86_core_cpu_execution_refresh(&fixture.execution);
    result = mode == 3u ? (byte == 1u ? *v30_byte_register(&fixture, reg) :
        *v30_word_register(&fixture, reg)) : fixture.memory[address] |
        (byte == 1u ? 0u : (lib_u32)fixture.memory[address + 1u] << 8u);
    if (fixture.execution.stop_requested || fixture.faults ||
        fixture.delivered_exceptions || fixture.cpu.data.eip != 0x100u + size ||
        result != expected || fixture.writes != (lib_u32)(mode != 3u && operation != 0u))
        return 1;
    if (operation == 0u) {
        const lib_u32 defined = VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
        return (fixture.cpu.data.eflags & defined) !=
            ((original & mask) == 0u ? VCPU_EFLAGS_ZF : 0u) ||
            (fixture.cpu.data.eflags & X86_CPU_NEC_PSW_MD) == 0u;
    }
    return fixture.cpu.data.eflags != flags;
}

static lib_i32 v30_bits(void)
{
    static const lib_u8 counts[] = {0u,7u,8u,15u,16u,31u,255u};
    static const lib_u16 values[] = {0u,0xa55au,0xffffu};

    for (lib_u8 opcode = 0x10u; opcode <= 0x1fu; ++opcode)
        for (lib_u8 mode = 0u; mode < 4u; ++mode)
            for (lib_u8 reg = 0u; reg < (mode == 3u ? 8u : 1u); ++reg)
                for (lib_size c = 0u; c < sizeof(counts); ++c)
                    for (lib_size v = 0u; v < sizeof(values) / sizeof(values[0]); ++v)
                        if (v30_bit_case(opcode, mode, reg, counts[c], values[v], LIB_FALSE) ||
                            (mode != 3u && v30_bit_case(opcode, mode, reg,
                                counts[c], values[v], LIB_TRUE))) {
                            lib_c_printf("V30:NEC-BIT:opcode=%02x mode=%u reg=%u count=%u value=%04x\n",
                                opcode, mode, reg, counts[c], values[v]);
                            return 1;
                        }
    return 0;
}

static lib_i32 v30_nibbles(void)
{
    cpu_bus_fixture fixture;
    x86_core_cpu_instruction_lexeme lexeme;
    for (lib_u8 direction = 0u; direction < 2u; ++direction)
        for (lib_u8 reg = 0u; reg < 9u; ++reg)
            for (lib_u16 value = 0u; value < 256u; ++value) {
                const lib_u8 opcode = direction ? 0x2au : 0x28u;
                const lib_u8 code[] = {0x0fu,opcode,
                    reg < 8u ? (lib_u8)(0xc0u | reg) : 0x06u,0x01u,0x05u};
                const lib_u8 size = reg < 8u ? 3u : 5u;
                lib_u8 al;
                lib_u8 result;
                lib_u8 expected;
                lib_u8 expected_al;
                lib_u32 flags;

                cpu_bus_prepare(&fixture, X86_CORE_CPU_PROFILE_V30);
                fixture.cpu.data.eflags |= VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
                    VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF;
                fixture.cpu.data.al = 0xb3u;
                if (reg < 8u) *v30_byte_register(&fixture, reg) = (lib_u8)value;
                else fixture.memory[0x501u] = (lib_u8)value;
                al = fixture.cpu.data.al;
                flags = fixture.cpu.data.eflags;
                expected = direction ? (lib_u8)(((al & 15u) << 4u) | (value >> 4u)) :
                    (lib_u8)((value << 4u) | (al & 15u));
                expected_al = direction ? (lib_u8)(value & 15u) : (lib_u8)(value >> 4u);
                lib_memory_copy(&fixture.memory[0x100u], code, size);
                if (!x86_core_cpu_execution_preview_lexeme(&fixture.execution, &lexeme) ||
                    lexeme.byte_count != size) return 1;
                x86_core_cpu_execution_refresh(&fixture.execution);
                result = reg < 8u ? *v30_byte_register(&fixture, reg) : fixture.memory[0x501u];
                /* AL's upper nibble is unspecified, including an AL destination. */
                if (fixture.execution.stop_requested || fixture.faults ||
                    fixture.cpu.data.eip != 0x100u + size ||
                    (reg != 0u && result != expected) ||
                    (fixture.cpu.data.al & 15u) != expected_al ||
                    fixture.cpu.data.eflags != flags ||
                    fixture.writes != (lib_u32)(reg == 8u)) return 1;
            }
    return 0;
}

static lib_i32 v30_invalid_forms(void)
{
    cpu_bus_fixture fixture;
    x86_core_cpu_instruction_lexeme lexeme;
    for (lib_u16 suffix = 0u; suffix < 256u; ++suffix) {
        const lib_bool valid = (suffix >= 0x10u && suffix <= 0x1fu) ||
            suffix == 0x28u || suffix == 0x2au || suffix == 0x20u ||
            suffix == 0x22u || suffix == 0x26u;
        const lib_u8 code[] = {0x0fu,(lib_u8)suffix,0xc0u,0u};
        const x86_core_cpu_instruction_metadata metadata =
            x86_core_cpu_instruction_metadata_get(X86_CORE_CPU_INSTRUCTION_NEC_0F,
                (lib_u8)suffix, 0xc0u);
        if (!!metadata.valid != valid ||
            !!x86_core_cpu_instruction_lexeme_scan(code, sizeof(code),
                X86_CORE_CPU_PROFILE_V30, LIB_FALSE, &lexeme) != valid) return 1;
        if (!valid) {
            cpu_bus_prepare(&fixture, X86_CORE_CPU_PROFILE_V30);
            lib_memory_copy(&fixture.memory[0x100u], code, sizeof(code));
            x86_core_cpu_execution_refresh(&fixture.execution);
            if (!fixture.execution.stop_requested || fixture.cpu.data.eip != 0x100u ||
                fixture.writes || fixture.delivered_exceptions) return 1;
        }
    }
    for (lib_u8 field = 1u; field < 8u; ++field) {
        const lib_u8 code[] = {0x0fu,0x1cu,(lib_u8)(0xc0u | (field << 3u)),0u};
        cpu_bus_prepare(&fixture, X86_CORE_CPU_PROFILE_V30);
        lib_memory_copy(&fixture.memory[0x100u], code, sizeof(code));
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (!fixture.execution.stop_requested || fixture.writes ||
            x86_core_cpu_instruction_lexeme_scan(code, sizeof(code),
                X86_CORE_CPU_PROFILE_V30, LIB_FALSE, &lexeme)) return 1;
    }
    return x86_core_cpu_instruction_metadata_get((x86_core_cpu_instruction_space)4,
        0x10u, 0xc0u).valid ||
        cpu_profile_allows_minimum(X86_CORE_CPU_PROFILE_80386, X86_CORE_CPU_PROFILE_V30);
}

typedef struct v30_bus_failure_fixture {
    cpu_bus_fixture bus;
    lib_u8 fail_stage;
    lib_u32 reads;
    lib_u32 writes;
} v30_bus_failure_fixture;

static lib_status v30_failure_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    v30_bus_failure_fixture *fixture = opaque;
    if (provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA && !observe_only) {
        ++fixture->reads;
        if (fixture->fail_stage == 1u) return LIB_STATUS_IO_ERROR;
    }
    return cpu_bus_read(&fixture->bus, address, destination, bytes, provenance,
        observe_only, reset_fetch, cache_control);
}

static lib_status v30_failure_write(void *opaque, lib_u32 address, const void *source,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    v30_bus_failure_fixture *fixture = opaque;
    ++fixture->writes;
    if (fixture->fail_stage == 2u) return LIB_STATUS_IO_ERROR;
    return cpu_bus_write(&fixture->bus, address, source, bytes, provenance, cache_control);
}

static lib_i32 v30_bus_failures(void)
{
    for (lib_u8 index = 0u; index < 18u; ++index)
        for (lib_u8 stage = 1u; stage <= 2u; ++stage) {
            v30_bus_failure_fixture fixture = {0};
            x86_core_cpu_bus_provider provider = cpu_bus_provider;
            const lib_u8 opcode = index < 16u ? (lib_u8)(0x10u + index) :
                index == 16u ? 0x28u : 0x2au;
            const lib_bool modifying = opcode >= 0x28u || ((opcode >> 1u) & 3u) != 0u;
            const lib_bool failed = stage == 1u || modifying;
            const lib_u8 code[] = {0x0fu,opcode,0x06u,0x00u,0x05u,0x07u};
            lib_u32 eax;
            lib_u32 flags;

            cpu_bus_prepare(&fixture.bus, X86_CORE_CPU_PROFILE_V30);
            provider.read_memory = v30_failure_read;
            provider.write_memory = v30_failure_write;
            fixture.bus.execution.bus = &provider;
            fixture.bus.execution.bus_context = &fixture;
            fixture.fail_stage = stage;
            fixture.bus.memory[0x500u] = 0x5au;
            fixture.bus.memory[0x501u] = 0xa5u;
            eax = fixture.bus.cpu.data.eax;
            flags = fixture.bus.cpu.data.eflags;
            lib_memory_copy(&fixture.bus.memory[0x100u], code, sizeof(code));
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (!!fixture.bus.execution.stop_requested != failed || fixture.reads == 0u ||
                fixture.bus.writes != 0u || fixture.bus.delivered_exceptions ||
                fixture.bus.memory[0x500u] != 0x5au || fixture.bus.memory[0x501u] != 0xa5u ||
                (failed && (fixture.bus.cpu.data.eip != 0x100u ||
                    fixture.bus.cpu.data.eax != eax || fixture.bus.cpu.data.eflags != flags)) ||
                fixture.writes != (lib_u32)(stage == 2u && modifying)) return 1;
        }
    return 0;
}

lib_i32 main(void)
{
    if (v30_bits()) { lib_c_printf("V30:NEC-BITS:FAIL\n"); return 1; }
    if (v30_nibbles()) { lib_c_printf("V30:NEC-NIBBLES:FAIL\n"); return 1; }
    if (v30_invalid_forms()) { lib_c_printf("V30:NEC-FORMS:FAIL\n"); return 1; }
    if (v30_bus_failures()) { lib_c_printf("V30:NEC-BUS:FAIL\n"); return 1; }
    lib_c_printf("V30:NEC-BIT-NIBBLE:OK\n");
    return 0;
}
