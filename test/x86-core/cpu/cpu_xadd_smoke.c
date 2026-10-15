#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

static lib_u32 *xadd_register(t_cpu *cpu, lib_u8 index)
{
    switch (index) {
    case 0: return &cpu->data.eax;
    case 1: return &cpu->data.ecx;
    case 2: return &cpu->data.edx;
    case 3: return &cpu->data.ebx;
    case 4: return &cpu->data.esp;
    case 5: return &cpu->data.ebp;
    case 6: return &cpu->data.esi;
    default: return &cpu->data.edi;
    }
}

static lib_u32 xadd_operand(t_cpu *cpu, lib_u8 index, lib_u8 width)
{
    const lib_u8 reg = width == 1u ? index & 3u : index;
    const lib_u8 shift = width == 1u && index >= 4u ? 8u : 0u;
    const lib_u32 mask = width == 1u ? 0xffu : width == 2u ? 0xffffu : LIB_UINT32_MAX;

    return (*xadd_register(cpu, reg) >> shift) & mask;
}

static void xadd_set_operand(t_cpu *cpu, lib_u8 index, lib_u8 width, lib_u32 value)
{
    const lib_u8 reg = width == 1u ? index & 3u : index;
    const lib_u8 shift = width == 1u && index >= 4u ? 8u : 0u;
    const lib_u32 mask = width == 1u ? 0xffu : width == 2u ? 0xffffu : LIB_UINT32_MAX;
    lib_u32 *target = xadd_register(cpu, reg);

    *target = (*target & ~(mask << shift)) | ((value & mask) << shift);
}

static lib_u32 xadd_flags(lib_u32 flags, lib_u32 destination,
    lib_u32 source, lib_u8 width)
{
    const lib_u32 mask = width == 1u ? 0xffu : width == 2u ? 0xffffu : LIB_UINT32_MAX;
    const lib_u32 sign = UINT32_C(1) << (width * 8u - 1u);
    const lib_u64 sum = (lib_u64)destination + source;
    const lib_u32 result = (lib_u32)sum & mask;
    lib_u8 parity = 0u;

    flags &= ~(VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF);
    if (sum > mask) flags |= VCPU_EFLAGS_CF;
    if ((destination ^ source ^ result) & 0x10u) flags |= VCPU_EFLAGS_AF;
    if ((~(destination ^ source) & (destination ^ result)) & sign) flags |= VCPU_EFLAGS_OF;
    if (result == 0u) flags |= VCPU_EFLAGS_ZF;
    if (result & sign) flags |= VCPU_EFLAGS_SF;
    for (lib_u8 bit = 0u; bit != 8u; ++bit) parity ^= (lib_u8)((result >> bit) & 1u);
    if (parity == 0u) flags |= VCPU_EFLAGS_PF;
    return flags;
}

static lib_bool xadd_register_forms(lib_bool code32)
{
    static const lib_u32 values[] = {0u, 1u, 0x7fu, 0x80u, 0x7fffu,
        0x8000u, 0x7fffffffu, 0x80000000u, LIB_UINT32_MAX};

    for (lib_u8 width = 1u; width <= 4u; width *= 2u) {
        for (lib_u8 source = 0u; source != 8u; ++source) {
            for (lib_u8 destination = 0u; destination != 8u; ++destination) {
                for (lib_size vector = 0u; vector != sizeof(values) / sizeof(values[0]); ++vector) {
                    cpu_instruction_fixture fixture;
                    t_cpu expected, after;
                    lib_u8 code[4], bytes = 0u;
                    lib_u32 old_source, old_destination;

                    cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
                    fixture.cpu.data.cs.seg.exec.defsize = code32;
                    fixture.cpu.data.eflags = 0x8d7u;
                    fixture.cpu.data.cr0 |= (vector & 1u ? VCPU_CR0_EM : 0u) |
                        (vector & 2u ? VCPU_CR0_TS : 0u);
                    for (lib_u8 reg = 0u; reg != 8u; ++reg)
                        *xadd_register(&fixture.cpu, reg) = 0xaabbccddu;
                    xadd_set_operand(&fixture.cpu, destination, width, 1u);
                    xadd_set_operand(&fixture.cpu, source, width, values[vector]);
                    old_source = xadd_operand(&fixture.cpu, source, width);
                    old_destination = xadd_operand(&fixture.cpu, destination, width);
                    if (width != 1u && (width == 4u) != code32) code[bytes++] = 0x66u;
                    code[bytes++] = 0x0fu;
                    code[bytes++] = width == 1u ? 0xc0u : 0xc1u;
                    code[bytes++] = (lib_u8)(0xc0u | (source << 3u) | destination);
                    expected = fixture.cpu;
                    xadd_set_operand(&expected, source, width, old_destination);
                    xadd_set_operand(&expected, destination, width, old_source + old_destination);
                    expected.data.eflags = xadd_flags(expected.data.eflags,
                        old_destination, old_source, width);
                    expected.data.eip = bytes;
                    if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
                        fixture.delivered_exception.valid || fixture.fault.valid ||
                        lib_memory_compare(&expected, &after, sizeof(after)) != 0) {
                        lib_c_fprintf(lib_c_stderr, "XADD register width=%u source=%u destination=%u vector=%u\n",
                            width, source, destination, (unsigned)vector);
                        return LIB_FALSE;
                    }
                }
            }
        }
    }
    return LIB_TRUE;
}

static lib_bool xadd_memory_forms(lib_bool code32)
{
    for (lib_u8 width = 1u; width <= 4u; width *= 2u) {
        for (lib_u8 source = 0u; source != 8u; ++source) {
            for (lib_u8 address32 = 0u; address32 != 2u; ++address32) {
                for (lib_u8 locked = 0u; locked != 2u; ++locked) {
                    cpu_instruction_fixture fixture;
                    t_cpu expected, after;
                    lib_u8 code[10], bytes = 0u;
                    lib_u32 memory = 0u;
                    x86_core_cpu_instruction_lexeme lexeme;

                    cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
                    fixture.cpu.data.cs.seg.exec.defsize = code32;
                    fixture.cpu.data.eflags = 0x8d7u;
                    xadd_set_operand(&fixture.cpu, source, width, 1u);
                    lib_memory_set(fixture.memory + 0x100u, 0xff, width);
                    if (locked) code[bytes++] = 0xf0u;
                    if (width != 1u && (width == 4u) != code32) code[bytes++] = 0x66u;
                    if (address32 != code32) code[bytes++] = 0x67u;
                    code[bytes++] = 0x0fu;
                    code[bytes++] = width == 1u ? 0xc0u : 0xc1u;
                    code[bytes++] = (lib_u8)((source << 3u) | (address32 ? 5u : 6u));
                    code[bytes++] = 0u;
                    code[bytes++] = 1u;
                    if (address32) { code[bytes++] = 0u; code[bytes++] = 0u; }
                    expected = fixture.cpu;
                    lib_memory_copy(&memory, fixture.memory + 0x100u, width);
                    xadd_set_operand(&expected, source, width, memory);
                    expected.data.eflags = xadd_flags(expected.data.eflags, memory, 1u, width);
                    expected.data.eip = bytes;
                    if (!locked) {
                        if (!x86_core_cpu_instruction_lexeme_scan(code, bytes,
                                X86_CORE_CPU_PROFILE_80486SX, code32, &lexeme) ||
                            lexeme.byte_count != bytes) return LIB_FALSE;
                        for (lib_u8 available = 0u; available < bytes; ++available) {
                            if (x86_core_cpu_instruction_lexeme_scan(code, available,
                                    X86_CORE_CPU_PROFILE_80486SX, code32, &lexeme))
                                return LIB_FALSE;
                        }
                    }
                    if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
                        fixture.delivered_exception.valid || fixture.fault.valid ||
                        lib_memory_compare(&expected, &after, sizeof(after)) != 0)
                        return LIB_FALSE;
                    memory = 0u;
                    lib_memory_copy(&memory, fixture.memory + 0x100u, width);
                    if (memory != 0u || fixture.writes != 1u) return LIB_FALSE;
                }
            }
        }
    }
    return LIB_TRUE;
}

static lib_bool xadd_address_alias(void)
{
    static const lib_u8 code[] = {0x67u, 0x66u, 0x0fu, 0xc1u, 0x14u, 0x12u};

    for (lib_u8 mode = 0u; mode != 3u; ++mode) {
        cpu_instruction_fixture fixture;
        t_cpu expected, after;
        lib_u32 memory = 7u;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.cr0 |= VCPU_CR0_PE;
        fixture.cpu.data.cs.dpl = mode == 0u ? 0u : 3u;
        fixture.cpu.data.ds.dpl = fixture.cpu.data.cs.dpl;
        fixture.cpu.data.ss.dpl = fixture.cpu.data.cs.dpl;
        fixture.cpu.data.cs.selector = (lib_u16)(8u | fixture.cpu.data.cs.dpl);
        fixture.cpu.data.ds.selector = (lib_u16)(16u | fixture.cpu.data.ds.dpl);
        fixture.cpu.data.ss.selector = fixture.cpu.data.ds.selector;
        fixture.cpu.data.eflags = 2u | (mode == 2u ? VCPU_EFLAGS_VM : 0u);
        fixture.cpu.data.edx = 0x100u;
        lib_memory_copy(fixture.memory + 0x200u, &memory, sizeof(memory));
        expected = fixture.cpu;
        expected.data.edx = memory;
        expected.data.eip = sizeof(code);
        expected.data.eflags = xadd_flags(expected.data.eflags, memory, 0x100u, 4u);
        if (cpu_instruction_run(&fixture, code, sizeof(code), &after) != LIB_STATUS_OK ||
            fixture.delivered_exception.valid || fixture.fault.valid ||
            lib_memory_compare(&expected, &after, sizeof(after)) != 0) {
            lib_c_fprintf(lib_c_stderr, "XADD SIB mode=%u mask=%x ip=%x edx=%x flags=%x\n",
                mode, fixture.delivered_exception.exception_mask, after.data.eip,
                after.data.edx, after.data.eflags);
            return LIB_FALSE;
        }
        lib_memory_copy(&memory, fixture.memory + 0x200u, sizeof(memory));
        if (memory != 0x107u || fixture.writes != 1u) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool xadd_rejected_forms(void)
{
    cpu_instruction_fixture fixture;

    for (lib_u8 opcode = 0xc0u; opcode != 0xc2u; ++opcode) {
        const lib_u8 locked[] = {0xf0u, 0x0fu, opcode, 0xc0u};
        const lib_u8 plain[] = {0x0fu, opcode, 0xc0u};
        const x86_core_cpu_instruction_metadata metadata =
            x86_core_cpu_instruction_metadata_get(
                X86_CORE_CPU_INSTRUCTION_0F, opcode, 0xc0u);
        x86_core_cpu_instruction_lexeme lexeme;

        if (!metadata.valid || metadata.minimum_cpu != X86_CORE_CPU_PROFILE_80486SX)
            return LIB_FALSE;
        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        if (!cpu_instruction_expect_real_fault(&fixture, locked, sizeof(locked), 6u))
            return LIB_FALSE;
        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80386);
        if (!cpu_instruction_expect_real_fault(&fixture, plain, sizeof(plain), 6u))
            return LIB_FALSE;
        for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_8086;
            profile <= X86_CORE_CPU_PROFILE_80386; ++profile) {
            if (x86_core_cpu_instruction_lexeme_scan(plain, sizeof(plain),
                    profile, LIB_FALSE, &lexeme) &&
                lexeme.byte_count == sizeof(plain)) return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

typedef struct xadd_failure_fixture {
    cpu_instruction_fixture instruction;
    lib_u32 fail_at;
    lib_u32 failures;
    lib_bool fail_write;
    lib_bool enabled;
} xadd_failure_fixture;

static lib_status xadd_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    xadd_failure_fixture *fixture = opaque;

    if (fixture->enabled && !fixture->fail_write && !observe_only &&
        address <= fixture->fail_at && (lib_u64)address + bytes > fixture->fail_at) {
        ++fixture->failures;
        return LIB_STATUS_IO_ERROR;
    }
    return cpu_instruction_read(&fixture->instruction, address, destination,
        bytes, provenance, observe_only, reset_fetch, cache_control);
}

static lib_status xadd_write(void *opaque, lib_u32 address, const void *source,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    xadd_failure_fixture *fixture = opaque;

    if (fixture->enabled && fixture->fail_write && address <= fixture->fail_at &&
        (lib_u64)address + bytes > fixture->fail_at) {
        ++fixture->failures;
        return LIB_STATUS_IO_ERROR;
    }
    return cpu_instruction_write(&fixture->instruction, address, source, bytes, provenance, cache_control);
}

static lib_bool xadd_bus_failures(void)
{
    static const x86_core_cpu_bus_provider bus = {
        .read_memory = xadd_read,
        .write_memory = xadd_write,
        .interrupt_pending = cpu_instruction_interrupt_pending
    };
    static const lib_u8 code[] = {0xf0u, 0x66u, 0x67u, 0x0fu, 0xc1u,
        0x15u, 0x00u, 0x01u, 0x00u, 0x00u};

    for (lib_u32 failure = 0u; failure != sizeof(code) + 2u; ++failure) {
        xadd_failure_fixture fixture;
        t_cpu before, after;
        lib_u32 memory = 7u;

        cpu_instruction_prepare_with_bus(&fixture.instruction,
            X86_CORE_CPU_PROFILE_80486SX, &bus, &fixture);
        fixture.fail_at = failure < sizeof(code) ? failure : 0x100u;
        fixture.failures = 0u;
        fixture.fail_write = failure == sizeof(code) + 1u;
        fixture.enabled = LIB_TRUE;
        fixture.instruction.cpu.data.edx = 5u;
        fixture.instruction.cpu.data.eflags = 0x8d7u;
        lib_memory_copy(fixture.instruction.memory + 0x100u, &memory, sizeof(memory));
        before = fixture.instruction.cpu;
        (void)cpu_instruction_run(&fixture.instruction, code, sizeof(code), &after);
        lib_memory_copy(&memory, fixture.instruction.memory + 0x100u, sizeof(memory));
        if (fixture.failures == 0u || !fixture.instruction.fault.valid ||
            fixture.instruction.fault.exception_mask != VCPUINS_EXCEPT_CE ||
            lib_memory_compare(&before, &after, sizeof(before)) != 0 ||
            fixture.instruction.writes != 0u || memory != 7u) {
            lib_c_fprintf(lib_c_stderr, "XADD failure=%u injected=%u mask=%x writes=%u\n",
                failure, fixture.failures, fixture.instruction.fault.exception_mask,
                fixture.instruction.writes);
            return LIB_FALSE;
        }
        fixture.enabled = LIB_FALSE;
        fixture.instruction.execution.stop_requested = LIB_FALSE;
        if (cpu_instruction_run(&fixture.instruction, code, sizeof(code), &after) != LIB_STATUS_OK ||
            after.data.edx != 7u || after.data.eip != sizeof(code)) return LIB_FALSE;
        lib_memory_copy(&memory, fixture.instruction.memory + 0x100u, sizeof(memory));
        if (memory != 12u || fixture.instruction.writes != 1u) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool xadd_protection_failures(void)
{
    static const lib_u8 descriptor[] = {0xffu, 0xffu, 0u, 0u, 0u, 0xfau, 0u, 0u};
    static const lib_u8 gate[] = {0u, 3u, 0x0bu, 0u, 0u, 0x8eu, 0u, 0u};
    static const lib_u8 data_code[] = {0x66u, 0x0fu, 0xc1u, 0x16u, 0u, 0x10u};
    static const lib_u8 stack_code[] = {0x36u, 0x66u, 0x0fu, 0xc1u, 0x16u, 0u, 0x10u};

    for (lib_u8 scenario = 0u; scenario != 6u; ++scenario) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        const lib_bool page = scenario >= 3u;
        const lib_u8 vector = page && scenario != 5u ? 14u : scenario == 2u ? 12u : 13u;
        const lib_u32 error = scenario == 4u ? 7u : scenario == 3u ? 6u : 0u;
        const lib_u32 directory = 0x11007u;
        lib_u32 frame[4], memory = 7u;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.cr0 = VCPU_CR0_PE;
        fixture.cpu.data.cs.selector = 0x0bu;
        fixture.cpu.data.ss.selector = 0x13u;
        fixture.cpu.data.ds.selector = 0x13u;
        fixture.cpu.data.cs.dpl = 3u;
        fixture.cpu.data.ss.dpl = 3u;
        fixture.cpu.data.ds.dpl = 3u;
        fixture.cpu.data.esp = 0x800u;
        fixture.cpu.data.edx = 5u;
        fixture.cpu.data.eflags = 0x8d7u;
        fixture.cpu.data.gdtr = (t_cpu_data_sreg){
            .flagValid = LIB_TRUE, .sregtype = SREG_GDTR, .base = 0x200u, .limit = 15u};
        fixture.cpu.data.idtr = (t_cpu_data_sreg){
            .flagValid = LIB_TRUE, .sregtype = SREG_IDTR, .base = 0x400u, .limit = 0x87u};
        lib_memory_copy(fixture.memory + 0x208u, descriptor, sizeof(descriptor));
        lib_memory_copy(fixture.memory + 0x400u + vector * 8u, gate, sizeof(gate));
        lib_memory_copy(fixture.memory + 0x1000u, &memory, sizeof(memory));
        if (scenario == 0u || scenario == 5u)
            fixture.cpu.data.ds.seg.data.writable = LIB_FALSE;
        if (scenario == 1u) fixture.cpu.data.ds.limit = 0x1001u;
        if (scenario == 2u) fixture.cpu.data.ss.limit = 0x1001u;
        if (page) {
            fixture.cpu.data.cr0 |= VCPU_CR0_PG;
            fixture.cpu.data.cr3 = 0x10000u;
            lib_memory_copy(fixture.memory + 0x10000u, &directory, sizeof(directory));
            for (lib_u32 index = 0u; index != 128u; ++index) {
                const lib_u32 entry = index == 1u ?
                    (scenario != 4u ? 0u : 0x1005u) : index * 0x1000u + 7u;
                lib_memory_copy(fixture.memory + 0x11000u + index * 4u, &entry, sizeof(entry));
            }
        }
        if (cpu_instruction_run(&fixture, scenario == 2u ? stack_code : data_code,
                scenario == 2u ? sizeof(stack_code) : sizeof(data_code), &after) != LIB_STATUS_OK ||
            fixture.fault.valid || !fixture.delivered_exception.valid ||
            fixture.delivered_exception.exception_mask != (UINT32_C(1) << vector) ||
            fixture.delivered_exception.exception_code != error ||
            fixture.delivered_exception.point.eip != 0u ||
            fixture.delivered_exception.eflags != 0x8d7u ||
            after.data.edx != 5u || after.data.eip != 0x300u || after.data.esp != 0x7f0u) {
            lib_c_fprintf(lib_c_stderr, "XADD protection=%u mask=%x error=%x ip=%x sp=%x edx=%x\n",
                scenario, fixture.delivered_exception.exception_mask,
                fixture.delivered_exception.exception_code, after.data.eip,
                after.data.esp, after.data.edx);
            return LIB_FALSE;
        }
        lib_memory_copy(frame, fixture.memory + 0x7f0u, sizeof(frame));
        lib_memory_copy(&memory, fixture.memory + 0x1000u, sizeof(memory));
        if (frame[0] != error || frame[1] != 0u || frame[2] != 0x0bu ||
            frame[3] != (0x8d7u | VCPU_EFLAGS_RF) || memory != 7u ||
            (vector == 14u && after.data.cr2 != 0x1000u) ||
            (vector != 14u && after.data.cr2 != 0u)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

int main(void)
{
    if (!xadd_register_forms(LIB_FALSE) || !xadd_register_forms(LIB_TRUE) ||
        !xadd_memory_forms(LIB_FALSE) || !xadd_memory_forms(LIB_TRUE) ||
        !xadd_address_alias() || !xadd_rejected_forms() ||
        !xadd_bus_failures() || !xadd_protection_failures()) {
        lib_c_fprintf(lib_c_stderr, "CPU-XADD:FAIL\n");
        return 1;
    }
    lib_c_printf("CPU-XADD:OK\n");
    return 0;
}
