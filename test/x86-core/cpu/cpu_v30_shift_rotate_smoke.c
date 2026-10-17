#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

typedef struct v30_shift_fixture {
    cpu_instruction_fixture instruction;
    lib_u32 reads, writes, fail_read, fail_write;
    lib_u32 read_address[2], write_address[2];
    lib_u8 read_bytes[2], write_bytes[2];
    lib_bool invalid;
} v30_shift_fixture;

static lib_status v30_shift_read(void *opaque, lib_u32 address, void *data,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe, lib_bool reset_fetch, lib_u8 cache_control)
{
    v30_shift_fixture *fixture = opaque;
    if (!observe && provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA) {
        if (fixture->reads >= 2u) { fixture->invalid = LIB_TRUE; return LIB_STATUS_IO_ERROR; }
        fixture->read_address[fixture->reads] = address;
        fixture->read_bytes[fixture->reads++] = bytes;
        if (fixture->reads == fixture->fail_read) return LIB_STATUS_IO_ERROR;
    }
    return cpu_instruction_read(opaque, address, data, bytes, provenance, observe, reset_fetch, cache_control);
}

static lib_status v30_shift_write(void *opaque, lib_u32 address, const void *data,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    v30_shift_fixture *fixture = opaque;
    if (fixture->writes >= 2u) { fixture->invalid = LIB_TRUE; return LIB_STATUS_IO_ERROR; }
    fixture->write_address[fixture->writes] = address;
    fixture->write_bytes[fixture->writes++] = bytes;
    if (fixture->writes == fixture->fail_write) return LIB_STATUS_IO_ERROR;
    return cpu_instruction_write(opaque, address, data, bytes, provenance, cache_control);
}

static void v30_shift_prepare(v30_shift_fixture *fixture, x86_core_cpu_profile profile)
{
    static const x86_core_cpu_bus_provider provider = {
        .read_memory = v30_shift_read, .write_memory = v30_shift_write,
        .interrupt_pending = cpu_instruction_interrupt_pending
    };
    lib_memory_set(fixture, 0, sizeof(*fixture));
    cpu_instruction_prepare_with_bus(&fixture->instruction, profile, &provider, fixture);
    fixture->instruction.cpu.data.eax = 0x12349876u;
    fixture->instruction.cpu.data.ecx = 0x23458765u;
    fixture->instruction.cpu.data.edx = 0x34567654u;
    fixture->instruction.cpu.data.ebx = 0x45676543u;
    fixture->instruction.cpu.data.esp = 0x56765432u;
    fixture->instruction.cpu.data.ebp = 0x67874321u;
    fixture->instruction.cpu.data.esi = 0x78983210u;
    fixture->instruction.cpu.data.edi = 0x89a9210fu;
}

static lib_u32 *v30_shift_register(t_cpu *cpu, lib_u8 index)
{
    switch (index) {
    case 0u: return &cpu->data.eax;
    case 1u: return &cpu->data.ecx;
    case 2u: return &cpu->data.edx;
    case 3u: return &cpu->data.ebx;
    case 4u: return &cpu->data.esp;
    case 5u: return &cpu->data.ebp;
    case 6u: return &cpu->data.esi;
    default: return &cpu->data.edi;
    }
}

static lib_u16 v30_shift_oracle(lib_u8 operation, lib_u8 bits, lib_u16 value,
    lib_u8 count, lib_u32 *flags, lib_u32 *defined)
{
    const lib_u32 mask = bits == 8u ? 0xffu : 0xffffu;
    const lib_u32 sign = (lib_u32)1u << (bits - 1u);
    lib_u32 result = value & mask;
    lib_u32 carry = (*flags & VCPU_EFLAGS_CF) != 0u;
    *defined = LIB_UINT32_MAX;
    for (lib_u16 step = 0u; step < count; ++step) {
        const lib_u32 high = (result & sign) != 0u, low = result % 2u;
        const lib_u32 prior_carry = carry;
        carry = operation == 0u || operation == 2u || operation == 4u ? high : low;
        switch (operation) {
        case 0u: result = ((result * 2u) | high) & mask; break;
        case 1u: result = result / 2u | low * sign; break;
        case 2u: result = ((result * 2u) | prior_carry) & mask; break;
        case 3u: result = result / 2u | prior_carry * sign; break;
        case 4u: result = result * 2u & mask; break;
        case 5u: result /= 2u; break;
        default: result = result / 2u | high * sign; break;
        }
    }
    if (count) {
        *flags = (*flags & ~VCPU_EFLAGS_CF) | (carry ? VCPU_EFLAGS_CF : 0u);
        *defined &= ~VCPU_EFLAGS_OF;
        if (operation >= 4u) {
            lib_u8 parity = 0u;
            for (lib_u8 bit = 0u; bit < 8u; ++bit) parity ^= (result >> bit) & 1u;
            *flags = (*flags & ~(VCPU_EFLAGS_PF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF)) |
                (parity == 0u ? VCPU_EFLAGS_PF : 0u) | (result == 0u ? VCPU_EFLAGS_ZF : 0u) |
                (result & sign ? VCPU_EFLAGS_SF : 0u);
            *defined &= ~VCPU_EFLAGS_AF;
        }
        if (count == 1u) {
            const lib_bool overflow = operation == 7u ? LIB_FALSE : operation == 5u ?
                (value & sign) != 0u : operation == 1u || operation == 3u ?
                ((result & sign) != 0u) != ((result & (sign / 2u)) != 0u) :
                ((result & sign) != 0u) != (carry != 0u);
            *flags = (*flags & ~VCPU_EFLAGS_OF) | (overflow ? VCPU_EFLAGS_OF : 0u);
        }
    }
    return (lib_u16)result;
}

static lib_i32 v30_shift_case(lib_u8 operation, lib_u8 width, lib_u8 form,
    lib_u8 destination, lib_u8 count, lib_u16 value, lib_bool carry, lib_u8 segment)
{
    static const lib_u8 prefixes[] = {0u,0x26u,0x2eu,0x36u,0x3eu};
    static const lib_u32 bases[] = {0x10000u,0x20000u,0u,0x30000u,0x10000u};
    static v30_shift_fixture fixture;
    const lib_bool memory = destination >= 8u;
    const lib_u32 address = bases[segment] + (destination == 8u ? 0x400u : 0xffffu);
    const lib_u8 bits = width * 8u;
    v30_shift_prepare(&fixture, X86_CORE_CPU_PROFILE_V30);
    fixture.instruction.cpu.data.ds.base = 0x10000u;
    fixture.instruction.cpu.data.es.base = 0x20000u;
    fixture.instruction.cpu.data.ss.base = 0x30000u;
    fixture.instruction.cpu.data.cl = count;
    fixture.instruction.cpu.data.eflags |= VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF |
        VCPU_EFLAGS_PF | VCPU_EFLAGS_SF | (carry ? VCPU_EFLAGS_CF : 0u);
    lib_u8 shift = width == 1u && destination >= 4u && !memory ? 8u : 0u;
    lib_u32 mask = (width == 1u ? 0xffu : 0xffffu) << shift;
    if (memory) lib_memory_copy(fixture.instruction.memory + address, &value, width);
    else {
        lib_u32 *reg = v30_shift_register(&fixture.instruction.cpu, width == 1u ? destination % 4u : destination);
        *reg = (*reg & ~mask) | ((lib_u32)value << shift & mask);
    }
    const t_cpu before = fixture.instruction.cpu;
    t_cpu expected = before;
    const lib_u8 actual_count = form == 0u ? 1u : form == 1u ? before.data.cl : count;
    lib_u32 defined, expected_flags = before.data.eflags;
    const lib_u16 result = v30_shift_oracle(operation, bits, value, actual_count, &expected_flags, &defined);
    if (form == 0u) defined |= VCPU_EFLAGS_OF;
    if (!memory) {
        lib_u32 *reg = v30_shift_register(&expected, width == 1u ? destination % 4u : destination);
        *reg = (*reg & ~mask) | ((lib_u32)result << shift & mask);
    }
    lib_u8 code[6], size = 0u;
    if (prefixes[segment]) code[size++] = prefixes[segment];
    code[size++] = (lib_u8)((form == 0u ? 0xd0u : form == 1u ? 0xd2u : 0xc0u) + width - 1u);
    code[size++] = (lib_u8)((operation << 3u) | (memory ? 6u : 0xc0u | destination));
    if (memory) { code[size++] = (lib_u8)address; code[size++] = (lib_u8)(address >> 8u); }
    if (form == 2u) code[size++] = count;
    lib_memory_copy(fixture.instruction.memory, code, size);
    x86_core_cpu_instruction_lexeme lexeme;
    if (!x86_core_cpu_execution_preview_lexeme(&fixture.instruction.execution, &lexeme) ||
        lexeme.byte_count != size || fixture.reads || fixture.writes ||
        lib_memory_compare(&fixture.instruction.cpu, &before, sizeof(before)) != 0) return 1;
    x86_core_cpu_execution_refresh(&fixture.instruction.execution);
    lib_u16 actual = 0u;
    if (memory) lib_memory_copy(&actual, fixture.instruction.memory + address, width);
    expected.data.eip = size;
    expected.data.eflags = (expected_flags & defined) | (fixture.instruction.cpu.data.eflags & ~defined);
    if (fixture.instruction.execution.stop_requested || fixture.invalid || fixture.instruction.delivered_exception.valid ||
        lib_memory_compare(&fixture.instruction.cpu, &expected, sizeof(expected)) != 0 ||
        (actual_count && (fixture.instruction.instructions.data.udf & VCPU_EFLAGS_CF)) ||
        (memory && actual != result) || fixture.reads != (memory ? width == 2u && (address & 1u) ? 2u : 1u : 0u) ||
        fixture.writes != fixture.reads) return 1;
    for (lib_u32 part = 0u; part < fixture.reads; ++part)
        if (fixture.read_address[part] != address + part || fixture.write_address[part] != address + part ||
            fixture.read_bytes[part] != (fixture.reads == 2u ? 1u : width) ||
            fixture.write_bytes[part] != fixture.read_bytes[part]) return 1;
    return 0;
}

static lib_i32 v30_shift_forms(void)
{
    static const lib_u8 operations[] = {0u,1u,2u,3u,4u,5u,7u};
    static const lib_u16 values[] = {0u,1u,0x81u,0x8001u,0xffffu};
    static const lib_u8 boundaries[] = {0u,1u,8u,16u,31u,32u,33u,255u};
    for (lib_size op = 0u; op < sizeof(operations); ++op)
        for (lib_u8 width = 1u; width <= 2u; ++width)
            for (lib_u16 count = 0u; count <= 255u; ++count)
                for (lib_u8 form = 1u; form <= 2u; ++form)
                    for (lib_u8 carry = 0u; carry < 2u; ++carry)
                        if (v30_shift_case(operations[op], width, form, 0u, (lib_u8)count,
                            width == 1u ? 0x81u : 0x8001u, carry != 0u, 0u)) return 1;
    for (lib_size op = 0u; op < sizeof(operations); ++op)
        for (lib_u8 width = 1u; width <= 2u; ++width)
            for (lib_u8 form = 0u; form < 3u; ++form)
                for (lib_u8 destination = 0u; destination < 10u; ++destination)
                    for (lib_size item = 0u; item < sizeof(values) / sizeof(values[0]); ++item)
                        for (lib_u8 segment = 0u; segment < (destination >= 8u ? 5u : 1u); ++segment)
                            if (v30_shift_case(operations[op], width, form, destination, 33u,
                                values[item], LIB_TRUE, segment)) return 1;
    for (lib_size op = 0u; op < sizeof(operations); ++op)
        for (lib_u8 width = 1u; width <= 2u; ++width)
            for (lib_u8 form = 1u; form <= 2u; ++form)
                for (lib_u8 destination = 8u; destination < 10u; ++destination)
                    for (lib_u8 segment = 0u; segment < 5u; ++segment)
                        for (lib_size item = 0u; item < sizeof(boundaries); ++item)
                            if (v30_shift_case(operations[op], width, form, destination,
                                boundaries[item], 0x8001u, LIB_FALSE, segment)) return 1;
    return 0;
}

static lib_i32 v30_shift_failures(void)
{
    static const lib_u8 operations[] = {0u,1u,2u,3u,4u,5u,7u};
    static v30_shift_fixture fixture;
    for (lib_size op = 0u; op < sizeof(operations); ++op)
        for (lib_u8 form = 0u; form < 3u; ++form)
            for (lib_u8 width = 1u; width <= 2u; ++width)
                for (lib_u8 write = 0u; write < 2u; ++write)
                    for (lib_u8 stage = 1u; stage <= width; ++stage) {
                        const lib_u8 code[] = {(lib_u8)((form == 0u ? 0xd0u : form == 1u ? 0xd2u : 0xc0u) + width - 1u),
                            (lib_u8)((operations[op] << 3u) | 6u),0xffu,0xffu,33u};
                        v30_shift_prepare(&fixture, X86_CORE_CPU_PROFILE_V30);
                        fixture.instruction.cpu.data.cl = 33u;
                        const lib_u16 value = 0x8001u;
                        lib_memory_copy(fixture.instruction.memory + 0xffffu, &value, width);
                        const t_cpu before = fixture.instruction.cpu;
                        if (write) fixture.fail_write = stage;
                        else fixture.fail_read = stage;
                        lib_memory_copy(fixture.instruction.memory, code, form == 2u ? 5u : 4u);
                        x86_core_cpu_execution_refresh(&fixture.instruction.execution);
                        lib_u32 flags = before.data.eflags, defined;
                        const lib_u16 result = v30_shift_oracle(operations[op], width * 8u, value,
                            form == 0u ? 1u : 33u, &flags, &defined);
                        lib_u16 actual = 0u;
                        lib_memory_copy(&actual, fixture.instruction.memory + 0xffffu, width);
                        if (!fixture.instruction.execution.stop_requested || fixture.invalid ||
                            fixture.instruction.delivered_exception.valid ||
                            lib_memory_compare(&fixture.instruction.cpu, &before, sizeof(before)) != 0 ||
                            (write ? fixture.writes : fixture.reads) != stage ||
                            (write ? fixture.reads != width : fixture.writes != 0u) ||
                            actual != (write && stage == 2u ? (value & 0xff00u) | (result & 0xffu) :
                                width == 1u ? value & 0xffu : value)) return 1;
                    }
    return 0;
}

static lib_i32 v30_shift_rejection_and_intel(void)
{
    static v30_shift_fixture fixture;
    for (lib_u8 form = 0u; form < 3u; ++form)
        for (lib_u8 width = 1u; width <= 2u; ++width) {
            const lib_u8 code[] = {(lib_u8)((form == 0u ? 0xd0u : form == 1u ? 0xd2u : 0xc0u) + width - 1u),0xf0u,33u};
            v30_shift_prepare(&fixture, X86_CORE_CPU_PROFILE_V30);
            x86_core_cpu_instruction_lexeme lexeme;
            lib_memory_copy(fixture.instruction.memory, code, sizeof(code));
            x86_core_cpu_execution_refresh(&fixture.instruction.execution);
            if (!fixture.instruction.execution.stop_requested || fixture.reads || fixture.writes ||
                fixture.instruction.cpu.data.eip || fixture.instruction.delivered_exception.valid ||
                x86_core_cpu_instruction_lexeme_scan(code, sizeof(code), X86_CORE_CPU_PROFILE_V30,
                    LIB_FALSE, &lexeme)) return 1;
        }
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_80186, X86_CORE_CPU_PROFILE_80286,
        X86_CORE_CPU_PROFILE_80386, X86_CORE_CPU_PROFILE_80486SX
    };
    for (lib_size profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        const lib_u8 code[] = {0xd3u,0xe0u};
        v30_shift_prepare(&fixture, profiles[profile]);
        fixture.instruction.cpu.data.ax = 1u;
        fixture.instruction.cpu.data.cl = 32u;
        lib_memory_copy(fixture.instruction.memory, code, sizeof(code));
        x86_core_cpu_execution_refresh(&fixture.instruction.execution);
        if (fixture.instruction.execution.stop_requested ||
            fixture.instruction.cpu.data.ax != (profile == 0u ? 0u : 1u)) return 1;
    }
    return 0;
}

lib_i32 main(void)
{
    if (v30_shift_forms()) { lib_c_printf("V30:SHIFT-FORMS:FAIL\n"); return 1; }
    if (v30_shift_failures()) { lib_c_printf("V30:SHIFT-FAILURES:FAIL\n"); return 1; }
    if (v30_shift_rejection_and_intel()) { lib_c_printf("V30:SHIFT-ADMISSION:FAIL\n"); return 1; }
    lib_c_printf("V30:SHIFT-ROTATE:OK\n");
    return 0;
}
