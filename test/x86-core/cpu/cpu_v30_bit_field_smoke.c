#include "support/cpu_bus_fixture.h"
#include "lib/types/file.h"

static lib_u8 *v30_field_register(t_cpu *cpu, lib_u8 reg)
{
    switch (reg) {
    case 0u: return &cpu->data.al;
    case 1u: return &cpu->data.cl;
    case 2u: return &cpu->data.dl;
    case 3u: return &cpu->data.bl;
    case 4u: return &cpu->data.ah;
    case 5u: return &cpu->data.ch;
    case 6u: return &cpu->data.dh;
    default: return &cpu->data.bh;
    }
}

static lib_size v30_field_address(lib_u32 base, lib_u16 index, lib_u8 bit)
{
    const lib_u16 word = (lib_u16)(index + (bit / 16u) * 2u);
    return (base + word + (bit % 16u) / 8u) % sizeof(((cpu_bus_fixture *)LIB_NULL)->memory);
}

static void v30_field_prepare(cpu_bus_fixture *fixture, lib_u16 index)
{
    cpu_bus_prepare(fixture, X86_CORE_CPU_PROFILE_V30);
    fixture->cpu.data.ds.base = 0x100u;
    fixture->cpu.data.es.base = 0x200u;
    fixture->cpu.data.ss.base = 0x300u;
    fixture->cpu.data.si = index;
    fixture->cpu.data.di = index;
    fixture->cpu.data.ax = 0xa55au;
    for (lib_u8 byte = 0u; byte < 4u; ++byte) {
        fixture->memory[(0x100u + index + byte) % sizeof(fixture->memory)] = (lib_u8)(0x55u ^ byte);
        fixture->memory[(0x200u + index + byte) % sizeof(fixture->memory)] = (lib_u8)(0x33u ^ byte);
        fixture->memory[(0x300u + index + byte) % sizeof(fixture->memory)] = (lib_u8)(0xacu ^ byte);
    }
}

static lib_i32 v30_field_run(cpu_bus_fixture *fixture, lib_u8 opcode,
    lib_u8 offset_reg, lib_u8 length_reg, lib_u8 immediate, lib_u8 prefix)
{
    const lib_bool insert = (opcode & 2u) == 0u;
    const lib_bool immediate_form = (opcode & 8u) != 0u;
    const lib_u8 offset = *v30_field_register(&fixture->cpu, offset_reg);
    const lib_u8 length = (immediate_form ? immediate :
        *v30_field_register(&fixture->cpu, length_reg)) + 1u;
    const lib_u16 index = insert ? fixture->cpu.data.di : fixture->cpu.data.si;
    const lib_u32 base = insert || prefix == 0x26u ? fixture->cpu.data.es.base :
        prefix == 0x36u ? fixture->cpu.data.ss.base :
        prefix == 0x2eu ? fixture->cpu.data.cs.base : fixture->cpu.data.ds.base;
    lib_u8 expected[sizeof(fixture->memory)];
    t_cpu result = fixture->cpu;
    lib_u16 value = 0u;
    lib_u8 code[5];
    lib_u8 size = 0u;
    x86_core_cpu_instruction_lexeme lexeme;

    if (prefix) code[size++] = prefix;
    code[size++] = 0x0fu;
    code[size++] = opcode;
    code[size++] = 0xc0u | (immediate_form ? 0u : ((lib_u32)length_reg << 3u)) | offset_reg;
    if (immediate_form) code[size++] = immediate;
    lib_memory_copy(&fixture->memory[0x100u], code, size);
    lib_memory_copy(expected, fixture->memory, sizeof(expected));
    /* Independent bit-at-a-time oracle; production transfers masked words. */
    for (lib_u8 bit = 0u; bit < length; ++bit) {
        const lib_size address = v30_field_address(base, index, offset + bit);
        const lib_u8 mask = (lib_u8)(1u << ((offset + bit) % 8u));
        if (insert) {
            expected[address] &= (lib_u8)~mask;
            if (result.data.ax & (1u << bit)) expected[address] |= mask;
        } else if (expected[address] & mask) value |= (lib_u16)(1u << bit);
    }
    if (insert) result.data.di = (lib_u16)(index + (offset + length >= 16u ? 2u : 0u));
    else {
        result.data.ax = value;
        result.data.si = (lib_u16)(index + (offset + length >= 16u ? 2u : 0u));
    }
    /* Source-order model for the otherwise unspecified AX/AL/AH output alias. */
    *v30_field_register(&result, offset_reg) = (offset + length) % 16u;
    if (!x86_core_cpu_instruction_lexeme_scan(code, size, X86_CORE_CPU_PROFILE_V30,
        LIB_FALSE, &lexeme) || lexeme.byte_count != size) return 1;
    for (lib_u8 available = 0u; available < size; ++available)
        if (x86_core_cpu_instruction_lexeme_scan(code, available,
            X86_CORE_CPU_PROFILE_V30, LIB_FALSE, &lexeme)) return 1;
    if (!x86_core_cpu_execution_preview_lexeme(&fixture->execution, &lexeme) ||
        lexeme.byte_count != size || fixture->writes) return 1;
    fixture->observations = 0u;
    x86_core_cpu_execution_refresh(&fixture->execution);
    /* All four source rows give ranges, not an exact input-to-clock formula. */
    const x86_core_cpu_timing_result timing = x86_core_cpu_capture_timing(&fixture->execution);
    if (!timing.source_timing_unallocated || timing.ticks || timing.event_ticks || timing.formula_inputs ||
        timing.form_id != X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED || timing.key_id != timing.form_id ||
        timing.retirement_origin != X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED || fixture->observations)
        return 1;
    if (fixture->execution.stop_requested || fixture->faults || fixture->delivered_exceptions ||
        fixture->cpu.data.eip != 0x100u + size || fixture->cpu.data.eax != result.data.eax ||
        fixture->cpu.data.ebx != result.data.ebx || fixture->cpu.data.ecx != result.data.ecx ||
        fixture->cpu.data.edx != result.data.edx || fixture->cpu.data.esi != result.data.esi ||
        fixture->cpu.data.edi != result.data.edi || fixture->cpu.data.ebp != result.data.ebp ||
        fixture->cpu.data.esp != result.data.esp ||
        lib_memory_compare(expected, fixture->memory, sizeof(expected)) != 0 ||
        fixture->writes != (lib_u32)(insert ? (offset + length > 16u ? 2u : 1u) *
            ((index & 1u) ? 2u : 1u) : 0u) ||
        fixture->last_address != ((base + (lib_u16)(index +
            (offset + length > 16u ? 2u : 0u)) + ((index & 1u) ? 1u : 0u)) & 0x000fffffu)) return 1;
    const lib_u32 undefined_flags = VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_CF;
    return (fixture->instructions.data.udf & undefined_flags) != undefined_flags ||
        (fixture->cpu.data.eflags & ~undefined_flags) != (result.data.eflags & ~undefined_flags);
}

static lib_i32 v30_field_operands(void)
{
    static const lib_u8 opcodes[] = {0x31u,0x33u,0x39u,0x3bu};
    cpu_bus_fixture fixture;
    for (lib_size op = 0u; op < sizeof(opcodes); ++op)
        for (lib_u8 offset = 0u; offset < 16u; ++offset)
            for (lib_u8 length = 0u; length < 16u; ++length)
                for (lib_u8 reg = 0u; reg < 8u; ++reg)
                    for (lib_u8 length_reg = 0u; length_reg < 8u; ++length_reg) {
                        if (op >= 2u && length_reg != 0u) continue;
                        if (op < 2u && reg == length_reg && offset != length) continue;
                        v30_field_prepare(&fixture, 0x400u);
                        if (op < 2u) *v30_field_register(&fixture.cpu, length_reg) = length;
                        *v30_field_register(&fixture.cpu, reg) = offset;
                        if (v30_field_run(&fixture, opcodes[op], reg, length_reg, length, 0u)) {
                            lib_c_printf("field op=%02X offset=%u length=%u registers=%u,%u\n",
                                opcodes[op], offset, length + 1u, reg, length_reg);
                            return 1;
                        }
                    }
    for (lib_size op = 0u; op < sizeof(opcodes); ++op)
        for (lib_u8 prefix_index = 0u; prefix_index < 5u; ++prefix_index)
            for (lib_u8 boundary = 0u; boundary < 3u; ++boundary) {
                static const lib_u8 prefixes[] = {0u,0x26u,0x2eu,0x36u,0x3eu};
                v30_field_prepare(&fixture, boundary == 0u ? 0x401u :
                    boundary == 1u ? 0xfffeu : 0xffffu);
                fixture.cpu.data.cl = 15u;
                fixture.cpu.data.dl = 15u;
                fixture.cpu.data.eflags |= VCPU_EFLAGS_DF;
                if (v30_field_run(&fixture, opcodes[op], 1u, 2u, 15u, prefixes[prefix_index])) {
                    lib_c_printf("field addressing op=%02X prefix=%02X index=%04X\n",
                        opcodes[op], prefixes[prefix_index], fixture.cpu.data.si);
                    return 1;
                }
            }
    return 0;
}

static lib_i32 v30_field_forms(void)
{
    static const lib_u8 opcodes[] = {0x31u,0x33u,0x39u,0x3bu};
    cpu_bus_fixture fixture;
    x86_core_cpu_instruction_lexeme lexeme;
    for (lib_size op = 0u; op < sizeof(opcodes); ++op)
        for (lib_u16 modrm = 0u; modrm < 256u; ++modrm) {
            const lib_bool valid = op < 2u ? modrm >= 0xc0u : (modrm & 0xf8u) == 0xc0u;
            const lib_u8 code[] = {0x0fu,opcodes[op],(lib_u8)modrm,0u,0u,0u};
            if (!!x86_core_cpu_instruction_metadata_get(X86_CORE_CPU_INSTRUCTION_NEC_0F,
                opcodes[op], (lib_u8)modrm).valid != valid ||
                !!x86_core_cpu_instruction_lexeme_scan(code, sizeof(code),
                    X86_CORE_CPU_PROFILE_V30, LIB_FALSE, &lexeme) != valid) return 1;
            if (valid) continue;
            v30_field_prepare(&fixture, 0x400u);
            lib_memory_copy(&fixture.memory[0x100u], code, sizeof(code));
            x86_core_cpu_execution_refresh(&fixture.execution);
            if (!fixture.execution.stop_requested || fixture.writes ||
                fixture.cpu.data.eip != 0x100u || fixture.delivered_exceptions) return 1;
        }
    for (lib_size op = 0u; op < sizeof(opcodes); ++op)
        for (lib_u8 operand = 0u; operand < 2u; ++operand) {
            const lib_u8 code[] = {0x0fu,opcodes[op],0xd1u,0x10u};
            const lib_u8 immediate_code[] = {0x0fu,opcodes[op],0xc1u,0x10u};
            v30_field_prepare(&fixture, 0x400u);
            fixture.cpu.data.cl = operand ? 0u : 0x10u;
            fixture.cpu.data.dl = operand ? 0x10u : 0u;
            lib_memory_copy(&fixture.memory[0x100u], op < 2u ? code : immediate_code, 4u);
            x86_core_cpu_execution_refresh(&fixture.execution);
            if (!fixture.execution.stop_requested || fixture.writes ||
                fixture.cpu.data.eip != 0x100u || fixture.delivered_exceptions) return 1;
        }
    return 0;
}

static lib_i32 v30_field_examples(void)
{
    static const lib_u8 insert[] = {0x0fu,0x31u,0xd1u,0x0fu,0x39u,0xc1u,12u};
    static const lib_u8 extract[] = {0x0fu,0x33u,0xd1u,0x0fu,0x3bu,0xc1u,12u};
    cpu_bus_fixture fixture;
    v30_field_prepare(&fixture, 0x400u);
    fixture.cpu.data.ax = 0x5555u;
    fixture.cpu.data.cl = 3u;
    fixture.cpu.data.dl = 4u;
    lib_memory_set(&fixture.memory[0x600u], 0, 4u);
    lib_memory_copy(&fixture.memory[0x100u], insert, sizeof(insert));
    x86_core_cpu_execution_refresh(&fixture.execution);
    if (fixture.execution.stop_requested || fixture.cpu.data.cl != 8u ||
        fixture.cpu.data.di != 0x400u || fixture.memory[0x600u] != 0xa8u ||
        fixture.memory[0x601u] != 0u || fixture.cpu.data.eip != 0x103u) return 1;
    x86_core_cpu_execution_refresh(&fixture.execution);
    if (fixture.execution.stop_requested || fixture.cpu.data.cl != 5u ||
        fixture.cpu.data.di != 0x402u || fixture.memory[0x600u] != 0xa8u ||
        fixture.memory[0x601u] != 0x55u || fixture.memory[0x602u] != 0x15u ||
        fixture.memory[0x603u] != 0u || fixture.cpu.data.eip != 0x107u) return 1;
    v30_field_prepare(&fixture, 0x400u);
    fixture.cpu.data.cl = 3u;
    fixture.cpu.data.dl = 4u;
    fixture.memory[0x500u] = fixture.memory[0x501u] = 0x55u;
    fixture.memory[0x502u] = fixture.memory[0x503u] = 0x33u;
    lib_memory_copy(&fixture.memory[0x100u], extract, sizeof(extract));
    x86_core_cpu_execution_refresh(&fixture.execution);
    if (fixture.execution.stop_requested || fixture.cpu.data.ax != 0x000au ||
        fixture.cpu.data.cl != 8u || fixture.cpu.data.si != 0x400u ||
        fixture.cpu.data.eip != 0x103u) return 1;
    x86_core_cpu_execution_refresh(&fixture.execution);
    return fixture.execution.stop_requested || fixture.cpu.data.ax != 0x1355u ||
        fixture.cpu.data.cl != 5u || fixture.cpu.data.si != 0x402u ||
        fixture.cpu.data.eip != 0x107u;
}

typedef struct v30_field_failure_fixture {
    cpu_bus_fixture bus;
    lib_u32 reads;
    lib_u32 attempts;
    lib_u32 fail_read;
    lib_u32 fail_write;
} v30_field_failure_fixture;

static lib_status v30_field_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    v30_field_failure_fixture *fixture = opaque;
    if (provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA && !observe_only &&
        ++fixture->reads == fixture->fail_read) return LIB_STATUS_IO_ERROR;
    return cpu_bus_read(&fixture->bus, address, destination, bytes, provenance,
        observe_only, reset_fetch, cache_control);
}

static lib_status v30_field_write(void *opaque, lib_u32 address, const void *source,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    v30_field_failure_fixture *fixture = opaque;
    if (++fixture->attempts == fixture->fail_write) return LIB_STATUS_IO_ERROR;
    return cpu_bus_write(&fixture->bus, address, source, bytes, provenance, cache_control);
}

static lib_i32 v30_field_failures(void)
{
    static const lib_u8 opcodes[] = {0x31u,0x33u,0x39u,0x3bu};
    for (lib_u8 op = 0u; op < sizeof(opcodes); ++op) {
        const lib_bool extract = (opcodes[op] & 2u) != 0u;
        for (lib_u8 stage = 1u; stage <= (extract ? 2u : 4u); ++stage) {
            v30_field_failure_fixture fixture = {0};
            x86_core_cpu_bus_provider provider = cpu_bus_provider;
            const lib_u8 code[] = {0x0fu,opcodes[op],op < 2u ? 0xd1u : 0xc1u,15u};
            lib_u32 flags;
            v30_field_prepare(&fixture.bus, 0x400u);
            fixture.bus.cpu.data.ax = 0xa55bu;
            fixture.bus.cpu.data.cl = 15u;
            fixture.bus.cpu.data.dl = 15u;
            flags = fixture.bus.cpu.data.eflags;
            provider.read_memory = v30_field_read;
            provider.write_memory = v30_field_write;
            fixture.bus.execution.bus = &provider;
            fixture.bus.execution.bus_context = &fixture;
            fixture.fail_read = stage <= 2u ? stage : 0u;
            fixture.fail_write = stage > 2u ? stage - 2u : 0u;
            lib_memory_copy(&fixture.bus.memory[0x100u], code, sizeof(code));
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (!fixture.bus.execution.stop_requested || fixture.bus.delivered_exceptions ||
                fixture.bus.execution.completed_observation_valid || fixture.bus.execution.timing_result.ticks ||
                fixture.bus.execution.timing_result.retirement_origin != X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED ||
                fixture.bus.cpu.data.eip != 0x100u || fixture.bus.cpu.data.ax != 0xa55bu ||
                fixture.bus.cpu.data.si != 0x400u || fixture.bus.cpu.data.di != 0x400u ||
                fixture.bus.cpu.data.cl != 15u || fixture.bus.cpu.data.eflags != flags ||
                fixture.bus.writes != (lib_u32)(!extract && (stage == 2u || stage == 4u))) return 1;
            if (fixture.reads != (lib_u32)(stage <= 2u ? stage : stage - 2u) ||
                fixture.attempts != (lib_u32)(extract || stage == 1u ? 0u :
                    stage == 2u ? 1u : stage - 2u) ||
                fixture.bus.memory[0x602u] != 0x31u || fixture.bus.memory[0x603u] != 0x30u ||
                fixture.bus.memory[0x600u] != 0x33u ||
                fixture.bus.memory[0x601u] != (lib_u8)(!extract &&
                    (stage == 2u || stage == 4u) ? 0xb2u : 0x32u)) return 1;
        }
    }
    return 0;
}

lib_i32 main(void)
{
    if (v30_field_operands()) { lib_c_printf("V30:BIT-FIELD-OPERANDS:FAIL\n"); return 1; }
    if (v30_field_forms()) { lib_c_printf("V30:BIT-FIELD-FORMS:FAIL\n"); return 1; }
    if (v30_field_examples()) { lib_c_printf("V30:BIT-FIELD-EXAMPLES:FAIL\n"); return 1; }
    if (v30_field_failures()) { lib_c_printf("V30:BIT-FIELD-FAILURES:FAIL\n"); return 1; }
    lib_c_printf("V30:BIT-FIELD:OK\n");
    return 0;
}
