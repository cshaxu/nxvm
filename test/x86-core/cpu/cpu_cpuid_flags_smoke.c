#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

static lib_bool cpuid_forms(void)
{
    static const lib_u32 leaves[] = {0u, 1u, 2u, 0x80000000u, LIB_UINT32_MAX};

    for (lib_u8 mode = 0u; mode != 6u; ++mode) {
        for (lib_u8 prefixes = 0u; prefixes != 16u; ++prefixes) {
            for (lib_size leaf = 0u; leaf != sizeof(leaves) / sizeof(leaves[0]); ++leaf) {
                cpu_instruction_fixture fixture;
                x86_core_cpu_instruction_lexeme lexeme;
                t_cpu expected, after;
                lib_u8 code[4], bytes = 0u;

                cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
                fixture.cpu.data.cs.seg.exec.defsize = (prefixes & 4u) != 0u;
                if (mode != 0u) {
                    fixture.cpu.data.cr0 |= VCPU_CR0_PE;
                    fixture.cpu.data.cs.dpl = mode == 5u ? 3u : mode - 1u;
                }
                fixture.cpu.data.eflags = 0x8d7u | VCPU_EFLAGS_AC |
                    ((prefixes & 8u) ? VCPU_EFLAGS_ID : 0u) |
                    (mode == 5u ? VCPU_EFLAGS_VM : 0u);
                fixture.cpu.data.eax = leaves[leaf];
                fixture.cpu.data.ebx = 0xaabbccddu;
                fixture.cpu.data.ecx = 0x11223344u;
                fixture.cpu.data.edx = 0x55667788u;
                if (prefixes & 1u) code[bytes++] = 0x66u;
                if (prefixes & 2u) code[bytes++] = 0x67u;
                code[bytes++] = 0x0fu;
                code[bytes++] = 0xa2u;
                expected = fixture.cpu;
                expected.data.eip = bytes;
                expected.data.eax = leaf == 0u ? 1u : leaf == 1u ? 0x42bu : 0u;
                expected.data.ebx = leaf == 0u ? 0x756e6547u : 0u;
                expected.data.ecx = leaf == 0u ? 0x6c65746eu : 0u;
                expected.data.edx = leaf == 0u ? 0x49656e69u : leaf == 1u ? 2u : 0u;
                if (!x86_core_cpu_instruction_lexeme_scan(code, bytes,
                        X86_CORE_CPU_PROFILE_80486SX,
                        fixture.cpu.data.cs.seg.exec.defsize, &lexeme) ||
                    lexeme.byte_count != bytes ||
                    cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
                    fixture.fault.valid || fixture.delivered_exception.valid ||
                    fixture.writes != 0u ||
                    lib_memory_compare(&expected, &after, sizeof(after)) != 0) {
                    lib_c_fprintf(lib_c_stderr,
                        "CPUID form: mode=%u prefixes=%u leaf=%u\n",
                        mode, prefixes, (lib_u32)leaf);
                    return LIB_FALSE;
                }
                for (lib_u8 available = 0u; available < bytes; ++available) {
                    if (x86_core_cpu_instruction_lexeme_scan(code, available,
                            X86_CORE_CPU_PROFILE_80486SX,
                            fixture.cpu.data.cs.seg.exec.defsize, &lexeme))
                        return LIB_FALSE;
                }
            }
        }
    }
    return LIB_TRUE;
}

static lib_bool cpuid_rejections(void)
{
    static const lib_u8 code[] = {0x0fu, 0xa2u};
    static const lib_u8 locked[] = {0xf0u, 0x0fu, 0xa2u};
    cpu_instruction_fixture fixture;
    x86_core_cpu_instruction_lexeme lexeme;

    for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_8086;
         profile <= X86_CORE_CPU_PROFILE_80386; ++profile) {
        if (x86_core_cpu_instruction_lexeme_scan(code, sizeof(code),
                profile, LIB_FALSE, &lexeme) && lexeme.byte_count == sizeof(code))
            return LIB_FALSE;
    }
    cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80386);
    if (!cpu_instruction_expect_real_fault(&fixture, code, sizeof(code), 6u))
        return LIB_FALSE;
    cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
    return cpu_instruction_expect_real_fault(&fixture, locked, sizeof(locked), 6u);
}

static lib_bool flags_stack_transfer(void)
{
    for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_80386;
         profile <= X86_CORE_CPU_PROFILE_80486SX; ++profile) {
        for (lib_u8 word = 0u; word != 2u; ++word) {
            for (lib_u8 clear = 0u; clear != 2u; ++clear) {
                cpu_instruction_fixture fixture;
                t_cpu after;
                const lib_u8 pop[] = {0x66u, 0x9du};
                const lib_u8 push[] = {0x66u, 0x9cu};
                const lib_u32 bits = VCPU_EFLAGS_AC | VCPU_EFLAGS_ID;
                const lib_u32 image = clear ? 0u : bits | 0x80188028u;
                const lib_u32 expected = word ? (clear ? bits : 0u) :
                    profile == X86_CORE_CPU_PROFILE_80486SX && !clear ? bits : 0u;
                lib_u32 observed = 0u;

                cpu_instruction_prepare(&fixture, profile);
                fixture.cpu.data.esp = 0x800u;
                fixture.cpu.data.eflags = 2u | (clear && (word ||
                    profile == X86_CORE_CPU_PROFILE_80486SX) ? bits : 0u);
                lib_memory_copy(fixture.memory + 0x800u, &image, sizeof(image));
                if (cpu_instruction_run(&fixture, pop + word, sizeof(pop) - word,
                        &after) != LIB_STATUS_OK || fixture.fault.valid ||
                    fixture.delivered_exception.valid || after.data.eflags != (2u | expected) ||
                    after.data.esp != (word ? 0x802u : 0x804u)) return LIB_FALSE;
                fixture.cpu.data.eip = 0u;
                if (cpu_instruction_run(&fixture, push + word, sizeof(push) - word,
                        &after) != LIB_STATUS_OK || after.data.esp != 0x800u)
                    return LIB_FALSE;
                lib_memory_copy(&observed, fixture.memory + 0x800u, word ? 2u : 4u);
                if (observed != (2u | (word ? 0u : expected))) return LIB_FALSE;
            }
        }
    }
    return LIB_TRUE;
}

static lib_bool flags_iret_transfer(void)
{
    const lib_u32 bits = VCPU_EFLAGS_AC | VCPU_EFLAGS_ID;

    for (lib_u8 vm86 = 0u; vm86 != 2u; ++vm86) {
        for (lib_u8 word = 0u; word != 2u; ++word) {
            for (lib_u8 clear = 0u; clear != 2u; ++clear) {
                cpu_instruction_fixture fixture;
                t_cpu after;
                const lib_u8 code[] = {0x66u, 0xcfu};
                const lib_u32 retained = vm86 ? VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL : 0u;
                const lib_u32 image = 2u | (clear ? 0u : bits);
                const lib_u32 frame32[] = {0x100u, 0u, image};
                const lib_u16 frame16[] = {0x100u, 0u, (lib_u16)image};
                const lib_u32 expected = word ? (clear ? bits : 0u) :
                    (clear ? 0u : bits);

                cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
                fixture.cpu.data.esp = 0x800u;
                fixture.cpu.data.eflags = 2u | retained | (clear ? bits : 0u);
                if (vm86) {
                    fixture.cpu.data.cr0 |= VCPU_CR0_PE;
                    fixture.cpu.data.cs.dpl = 3u;
                    fixture.cpu.data.ss.dpl = 3u;
                }
                if (word)
                    lib_memory_copy(fixture.memory + 0x800u, frame16, sizeof(frame16));
                else
                    lib_memory_copy(fixture.memory + 0x800u, frame32, sizeof(frame32));
                if (cpu_instruction_run(&fixture, code + word, sizeof(code) - word,
                        &after) != LIB_STATUS_OK || fixture.fault.valid ||
                    fixture.delivered_exception.valid || after.data.eip != 0x100u ||
                    after.data.esp != (word ? 0x806u : 0x80cu) ||
                    after.data.eflags != (2u | retained | expected)) {
                    lib_c_fprintf(lib_c_stderr,
                        "IRET flags: vm86=%u word=%u clear=%u\n", vm86, word, clear);
                    return LIB_FALSE;
                }
            }
        }
    }
    return LIB_TRUE;
}

typedef struct reset_fetch_fixture {
    cpu_instruction_fixture instruction;
    lib_u32 high_fetches;
    lib_u32 low_fetches;
    lib_bool invalid_provenance;
} reset_fetch_fixture;

static lib_status reset_fetch_read(void *opaque, lib_u32 address,
    void *destination, lib_u8 bytes,
    x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    reset_fetch_fixture *fixture = (reset_fetch_fixture *)opaque;
    const lib_bool high = address >= 0xfffffff0u;

    if (!observe_only) {
        if (high) ++fixture->high_fetches;
        else ++fixture->low_fetches;
        if (reset_fetch != high ||
            (provenance != X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH &&
             provenance != X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH))
            fixture->invalid_provenance = LIB_TRUE;
    }
    return cpu_instruction_read(&fixture->instruction,
        high ? address & 0xffffu : address, destination, bytes,
        provenance, observe_only, reset_fetch, cache_control);
}

static lib_bool cpuid_reset_fetch(void)
{
    static const x86_core_cpu_bus_provider bus = {
        .read_memory = reset_fetch_read,
        .interrupt_pending = cpu_instruction_interrupt_pending
    };
    static const lib_u8 jump[] = {0xeau, 0u, 1u, 0u, 0u};
    reset_fetch_fixture fixture;

    lib_memory_set(&fixture, 0, sizeof(fixture));
    cpu_instruction_prepare_with_bus(&fixture.instruction,
        X86_CORE_CPU_PROFILE_80486SX, &bus, &fixture);
    lib_memory_copy(fixture.instruction.memory + 0xfff0u, jump, sizeof(jump));
    fixture.instruction.memory[0x100u] = 0x90u;
    x86_core_cpu_state_reset(&fixture.instruction.execution);
    x86_core_cpu_execution_refresh(&fixture.instruction.execution);
    if (fixture.instruction.fault.valid ||
        fixture.instruction.delivered_exception.valid ||
        fixture.high_fetches == 0u || fixture.invalid_provenance ||
        fixture.instruction.cpu.data.cs.selector != 0u ||
        fixture.instruction.cpu.data.cs.base != 0u ||
        fixture.instruction.cpu.data.eip != 0x100u)
        return LIB_FALSE;
    x86_core_cpu_execution_refresh(&fixture.instruction.execution);
    return !fixture.instruction.fault.valid &&
        !fixture.instruction.delivered_exception.valid &&
        fixture.low_fetches != 0u && !fixture.invalid_provenance &&
        fixture.instruction.cpu.data.eip == 0x101u;
}

static lib_bool cpuid_reset(void)
{
    cpu_instruction_fixture fixture;

    cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
    fixture.cpu.data.eflags |= VCPU_EFLAGS_AC | VCPU_EFLAGS_ID;
    fixture.cpu.data.edx = LIB_UINT32_MAX;
    fixture.cpu.data.cr0 = LIB_UINT32_MAX;
    fixture.cpu.data.dr6 = 0u;
    fixture.cpu.data.dr7 = LIB_UINT32_MAX;
    x86_core_cpu_state_reset(&fixture.execution);
    return fixture.cpu.data.eflags == 2u && fixture.cpu.data.edx == 0x42bu &&
        fixture.cpu.data.cr0 == 0x60000010u &&
        fixture.cpu.data.dr6 == 0xffff0ff0u && fixture.cpu.data.dr7 == 0u &&
        fixture.cpu.data.cs.selector == 0xf000u &&
        fixture.cpu.data.cs.base == 0xffff0000u &&
        fixture.cpu.data.cs.limit == 0xffffu && fixture.cpu.data.eip == 0xfff0u &&
        fixture.cpu.data.idtr.base == 0u && fixture.cpu.data.idtr.limit == 0x3ffu;
}

static lib_bool flags_pop_privilege(void)
{
    for (lib_u8 cpl = 0u; cpl != 4u; ++cpl)
    for (lib_u8 iopl = 0u; iopl != 4u; ++iopl)
    for (lib_u8 word = 0u; word != 2u; ++word)
    for (lib_u8 bits = 0u; bits != 4u; ++bits) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        const lib_u8 code[] = {0x66u, 0x9du};
        const lib_u32 high = ((bits & 1u) ? VCPU_EFLAGS_AC : 0u) |
            ((bits & 2u) ? VCPU_EFLAGS_ID : 0u);
        const lib_u32 old = 2u | ((lib_u32)iopl << 12u) |
            ((VCPU_EFLAGS_AC | VCPU_EFLAGS_ID) & ~high);
        const lib_u32 image = 2u | VCPU_EFLAGS_IOPL | VCPU_EFLAGS_IF | high;
        const lib_u32 preserve = (word ? 0xffff0000u : 0u) |
            (cpl ? VCPU_EFLAGS_IOPL : 0u) | (cpl > iopl ? VCPU_EFLAGS_IF : 0u);

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.cr0 |= VCPU_CR0_PE;
        fixture.cpu.data.cs.dpl = cpl;
        fixture.cpu.data.ss.dpl = cpl;
        fixture.cpu.data.esp = 0x800u;
        fixture.cpu.data.eflags = old;
        lib_memory_copy(fixture.memory + 0x800u, &image, sizeof(image));
        if (cpu_instruction_run(&fixture, code + word, sizeof(code) - word,
                &after) != LIB_STATUS_OK || fixture.fault.valid ||
            fixture.delivered_exception.valid ||
            after.data.eflags != ((image & ~preserve) | (old & preserve))) {
            lib_c_fprintf(lib_c_stderr, "POPF privilege: cpl=%u iopl=%u word=%u bits=%u\n",
                cpl, iopl, word, bits);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

typedef struct cpuid_fetch_fixture {
    cpu_instruction_fixture instruction;
    lib_u32 fail_at;
    lib_u32 failures;
} cpuid_fetch_fixture;

static lib_status cpuid_fetch(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    cpuid_fetch_fixture *fixture = opaque;

    if (!observe_only &&
        (provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH ||
         provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH) &&
        address <= fixture->fail_at && (lib_u64)address + bytes > fixture->fail_at) {
        ++fixture->failures;
        return LIB_STATUS_IO_ERROR;
    }
    return cpu_instruction_read(&fixture->instruction, address, destination,
        bytes, provenance, observe_only, reset_fetch, cache_control);
}

static lib_bool cpuid_failed_fetch(void)
{
    static const x86_core_cpu_bus_provider bus = {
        .read_memory = cpuid_fetch,
        .write_memory = cpu_instruction_write,
        .interrupt_pending = cpu_instruction_interrupt_pending
    };
    static const lib_u8 code[] = {0x66u, 0x67u, 0x0fu, 0xa2u};

    for (lib_u32 fail_at = 0u; fail_at < sizeof(code); ++fail_at) {
        cpuid_fetch_fixture fixture;
        t_cpu before, after;

        cpu_instruction_prepare_with_bus(&fixture.instruction,
            X86_CORE_CPU_PROFILE_80486SX, &bus, &fixture);
        fixture.fail_at = fail_at;
        fixture.failures = 0u;
        fixture.instruction.cpu.data.eax = 1u;
        fixture.instruction.cpu.data.ebx = 0x12345678u;
        before = fixture.instruction.cpu;
        (void)cpu_instruction_run(&fixture.instruction, code, sizeof(code), &after);
        if (fixture.failures == 0u || !fixture.instruction.fault.valid ||
            fixture.instruction.fault.exception_mask != VCPUINS_EXCEPT_CE ||
            fixture.instruction.writes != 0u ||
            lib_memory_compare(&before, &after, sizeof(after)) != 0)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

int main(void)
{
    if (!cpuid_reset() || !cpuid_reset_fetch() ||
        !flags_iret_transfer() || !flags_pop_privilege() ||
        !cpuid_failed_fetch()) return 1;
    if (!cpuid_forms()) return 1;
    if (!cpuid_rejections()) {
        lib_c_fprintf(lib_c_stderr, "CPUID rejection: FAIL\n");
        return 1;
    }
    if (!flags_stack_transfer()) {
        lib_c_fprintf(lib_c_stderr, "Flags stack transfer: FAIL\n");
        return 1;
    }
    lib_c_printf("CPU-CPUID-FLAGS:OK\n");
    return 0;
}
