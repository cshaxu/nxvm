#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

static lib_u32 *bswap_register(t_cpu *cpu, lib_u8 index)
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

static lib_bool bswap_forms(void)
{
    for (lib_u8 code32 = 0u; code32 != 2u; ++code32) {
        for (lib_u8 address_prefix = 0u; address_prefix != 2u; ++address_prefix) {
            for (lib_u8 reg = 0u; reg != 8u; ++reg) {
                cpu_instruction_fixture fixture;
                t_cpu expected, after;
                lib_u8 code[4], bytes = 0u;
                core_machine_cpu_instruction_lexeme lexeme;

                cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
                fixture.cpu.data.cs.seg.exec.defsize = code32;
                fixture.cpu.data.eflags = 0x8d7u;
                for (lib_u8 index = 0u; index != 8u; ++index)
                    *bswap_register(&fixture.cpu, index) = 0x12345678u + index;
                if (!code32) code[bytes++] = 0x66u;
                if (address_prefix) code[bytes++] = 0x67u;
                code[bytes++] = 0x0fu;
                code[bytes++] = (lib_u8)(0xc8u + reg);
                expected = fixture.cpu;
                *bswap_register(&expected, reg) =
                    ((0x78u + reg) << 24u) | 0x00563412u;
                expected.data.eip = bytes;
                if (!core_machine_cpu_instruction_lexeme_scan(code, bytes,
                        CORE_MACHINE_CPU_PROFILE_80486SX, code32, &lexeme) ||
                    lexeme.byte_count != bytes ||
                    cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
                    fixture.instructions.data.except != 0u ||
                    lib_memory_compare(&expected, &after, sizeof(after)) != 0)
                    return LIB_FALSE;
                for (lib_u8 available = 0u; available < bytes; ++available) {
                    if (core_machine_cpu_instruction_lexeme_scan(code, available,
                            CORE_MACHINE_CPU_PROFILE_80486SX, code32, &lexeme))
                        return LIB_FALSE;
                }
            }
        }
    }
    return LIB_TRUE;
}

static lib_bool bswap_rejected_forms(void)
{
    static const lib_u8 word[] = {0x0fu, 0xc8u};
    static const lib_u8 locked[] = {0xf0u, 0x66u, 0x0fu, 0xc8u};
    static const lib_u8 dword[] = {0x66u, 0x0fu, 0xc8u};
    cpu_instruction_fixture fixture;

    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
    if (!cpu_instruction_expect_real_fault(&fixture, word, sizeof(word), 6u))
        return LIB_FALSE;
    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
    if (!cpu_instruction_expect_real_fault(&fixture, locked, sizeof(locked), 6u))
        return LIB_FALSE;
    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
    return cpu_instruction_expect_real_fault(&fixture, dword, sizeof(dword), 6u);
}

typedef struct bswap_fetch_fixture {
    cpu_instruction_fixture instruction;
    lib_u32 fail_at;
    lib_u32 failures;
} bswap_fetch_fixture;

static lib_status bswap_fetch(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch)
{
    bswap_fetch_fixture *fixture = opaque;

    if (!observe_only &&
        (provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH ||
         provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH) &&
        address <= fixture->fail_at && (lib_u64)address + bytes > fixture->fail_at) {
        ++fixture->failures;
        return LIB_STATUS_IO_ERROR;
    }
    return cpu_instruction_read(&fixture->instruction, address, destination,
        bytes, provenance, observe_only, reset_fetch);
}

static lib_bool bswap_failed_fetch(void)
{
    static const core_machine_cpu_bus_provider bus = {
        .read_memory = bswap_fetch,
        .write_memory = cpu_instruction_write,
        .interrupt_pending = cpu_instruction_interrupt_pending
    };
    static const lib_u8 code[] = {0x66u, 0x67u, 0x0fu, 0xc8u};

    for (lib_u32 fail_at = 0u; fail_at != sizeof(code); ++fail_at) {
        bswap_fetch_fixture fixture;
        t_cpu after;

        cpu_instruction_prepare_with_bus(&fixture.instruction,
            CORE_MACHINE_CPU_PROFILE_80486SX, &bus, &fixture);
        fixture.fail_at = fail_at;
        fixture.failures = 0u;
        fixture.instruction.cpu.data.eax = 0x12345678u;
        (void)cpu_instruction_run(&fixture.instruction, code, sizeof(code), &after);
        if (fixture.failures == 0u || !fixture.instruction.fault.valid ||
            fixture.instruction.fault.exception_mask != VCPUINS_EXCEPT_CE ||
            after.data.eax != 0x12345678u || after.data.eip != 0u ||
            fixture.instruction.writes != 0u) {
            lib_c_fprintf(lib_c_stderr,
                "BSWAP fetch=%u injected=%u fault=%u mask=%x eax=%x eip=%x writes=%u\n",
                fail_at, fixture.failures, fixture.instruction.fault.valid,
                fixture.instruction.fault.exception_mask, after.data.eax,
                after.data.eip, fixture.instruction.writes);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool bswap_profile_metadata(void)
{
    for (lib_u8 opcode = 0xc8u; opcode != 0xd0u; ++opcode) {
        const core_machine_cpu_instruction_metadata metadata =
            core_machine_cpu_instruction_metadata_get(
                CORE_MACHINE_CPU_INSTRUCTION_0F, opcode, 0u);
        const lib_u8 code[] = {0x66u, 0x0fu, opcode};
        core_machine_cpu_instruction_lexeme lexeme;

        if (!metadata.valid || metadata.minimum_cpu !=
                CORE_MACHINE_CPU_PROFILE_80486SX) return LIB_FALSE;
        for (core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_8086;
            profile <= CORE_MACHINE_CPU_PROFILE_80386; ++profile) {
            if (core_machine_cpu_instruction_lexeme_scan(code, sizeof(code),
                    profile, LIB_FALSE, &lexeme)) return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

int main(void)
{
    if (!bswap_forms() || !bswap_rejected_forms() ||
        !bswap_failed_fetch() || !bswap_profile_metadata()) {
        lib_c_fprintf(lib_c_stderr, "CPU-BSWAP:FAIL\n");
        return 1;
    }
    lib_c_printf("CPU-BSWAP:OK\n");
    return 0;
}
