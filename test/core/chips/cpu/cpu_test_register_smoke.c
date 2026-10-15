#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

/* Retained 386 storage behavior, not proof of 486 cache/TLB test operations. */
static lib_bool test_register_retained_transfers(void)
{
    for (lib_u8 mode = 0u; mode != 4u; ++mode) {
        const lib_bool code32 = (mode & 1u) != 0u;
        for (lib_u8 prefixes = 0u; prefixes != 4u; ++prefixes) {
            for (lib_u8 reg = 6u; reg != 8u; ++reg) {
                for (lib_u8 write = 0u; write != 2u; ++write) {
                    cpu_instruction_fixture fixture;
                    core_machine_cpu_instruction_lexeme lexeme;
                    lib_u8 code[5], bytes = 0u;
                    t_cpu expected, after;

                    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
                    fixture.cpu.data.cs.seg.exec.defsize = code32;
                    if (mode >= 2u) fixture.cpu.data.cr0 |= VCPU_CR0_PE;
                    fixture.cpu.data.eax = 0x12345678u;
                    fixture.cpu.data.tr6 = 0x87654321u;
                    fixture.cpu.data.tr7 = 0xabcdef01u;
                    fixture.cpu.data.eflags = 0x8d7u;
                    if (prefixes & 1u) code[bytes++] = 0x66u;
                    if (prefixes & 2u) code[bytes++] = 0x67u;
                    code[bytes++] = 0x0fu;
                    code[bytes++] = write ? 0x26u : 0x24u;
                    code[bytes++] = (lib_u8)(0xc0u | (reg << 3u));
                    expected = fixture.cpu;
                    expected.data.eip = bytes;
                    if (write) {
                        if (reg == 6u) expected.data.tr6 = expected.data.eax;
                        else expected.data.tr7 = expected.data.eax;
                    } else {
                        expected.data.eax = reg == 6u ?
                            expected.data.tr6 : expected.data.tr7;
                    }
                    if (!core_machine_cpu_instruction_lexeme_scan(code, bytes,
                            CORE_MACHINE_CPU_PROFILE_80386, code32, &lexeme) ||
                        lexeme.byte_count != bytes ||
                        cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
                        fixture.instructions.data.except != 0u || fixture.writes != 0u ||
                        lib_memory_compare(&expected, &after, sizeof(after)) != 0)
                        return LIB_FALSE;
                    for (lib_u8 available = 0u; available < bytes; ++available) {
                        if (core_machine_cpu_instruction_lexeme_scan(code, available,
                                CORE_MACHINE_CPU_PROFILE_80386, code32, &lexeme))
                            return LIB_FALSE;
                    }
                }
            }
        }
    }
    return LIB_TRUE;
}

static lib_bool test_register_invalid_forms(void)
{
    for (core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_80386;
         profile <= CORE_MACHINE_CPU_PROFILE_80486SX; ++profile) {
        for (lib_u8 write = 0u; write != 2u; ++write) {
            for (lib_u8 reg = 0u; reg != 8u; ++reg) {
                cpu_instruction_fixture fixture;
                core_machine_cpu_instruction_lexeme lexeme;
                lib_u8 code[] = {0x0fu, write ? 0x26u : 0x24u,
                    (lib_u8)(0xc0u | (reg << 3u))};

                /* 486 TR3--7 need actual test hardware; do not assert #UD
                 * for their legal register forms merely because it is absent. */
                if (reg < (profile == CORE_MACHINE_CPU_PROFILE_80386 ? 6u : 3u)) {
                    cpu_instruction_prepare(&fixture, profile);
                    if (core_machine_cpu_instruction_lexeme_scan(code, sizeof(code),
                            profile, LIB_FALSE, &lexeme) ||
                        !cpu_instruction_expect_real_fault(&fixture, code, sizeof(code), 6u))
                        return LIB_FALSE;
                }
                code[2] = (lib_u8)(reg << 3u);
                cpu_instruction_prepare(&fixture, profile);
                if (core_machine_cpu_instruction_lexeme_scan(code, sizeof(code),
                        profile, LIB_FALSE, &lexeme) ||
                    !cpu_instruction_expect_real_fault(&fixture, code, sizeof(code), 6u))
                    return LIB_FALSE;
                {
                    const lib_u8 locked[] = {0xf0u, 0x0fu, write ? 0x26u : 0x24u,
                        (lib_u8)(0xc0u | (reg << 3u))};
                    cpu_instruction_prepare(&fixture, profile);
                    if (!cpu_instruction_expect_real_fault(&fixture,
                            locked, sizeof(locked), 6u)) return LIB_FALSE;
                }
            }
        }
    }
    return LIB_TRUE;
}

static lib_bool test_register_privilege(void)
{
    for (core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_80386;
         profile <= CORE_MACHINE_CPU_PROFILE_80486SX; ++profile) {
        for (lib_u8 mode = 1u; mode != 5u; ++mode) {
            const lib_bool vm86 = mode == 4u;
            const lib_u8 cpl = vm86 ? 3u : mode;
            const lib_u8 target_cpl = vm86 ? 0u : cpl;
            for (lib_u8 write = 0u; write != 2u; ++write) {
                for (lib_u8 reg = profile == CORE_MACHINE_CPU_PROFILE_80386 ? 6u : 3u;
                     reg != 8u; ++reg) {
                    cpu_instruction_fixture fixture;
                    t_cpu after;
                    lib_u32 frame[4];
                    const lib_u8 code[] = {0x0fu, write ? 0x26u : 0x24u,
                        (lib_u8)(0xc0u | (reg << 3u))};
                    cpu_instruction_prepare(&fixture, profile);
                    cpu_instruction_prepare_protected_exception(&fixture, mode);
                    fixture.cpu.data.eax = 0x12345678u;
                    fixture.cpu.data.tr6 = 0x87654321u;
                    fixture.cpu.data.tr7 = 0xabcdef01u;
                    if (cpu_instruction_run(&fixture, code, sizeof(code), &after) != LIB_STATUS_OK ||
                        fixture.fault.valid || !fixture.delivered_exception.valid ||
                        fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_GP ||
                        fixture.delivered_exception.exception_code != 0u ||
                        fixture.delivered_exception.point.eip != 0u ||
                        after.data.eip != 0x300u || after.data.eax != 0x12345678u ||
                        after.data.tr6 != 0x87654321u || after.data.tr7 != 0xabcdef01u ||
                        after.data.cs.dpl != target_cpl ||
                        after.data.esp != (vm86 ? 0xfd8u : 0x7f0u)) {
                        lib_c_fprintf(lib_c_stderr, "TR privilege profile=%u mode=%u write=%u reg=%u exception=%x ip=%x sp=%x\n",
                            profile, mode, write, reg, fixture.delivered_exception.exception_mask,
                            after.data.eip, after.data.esp);
                        return LIB_FALSE;
                    }
                    lib_memory_copy(frame, fixture.memory + after.data.esp, sizeof(frame));
                    if (frame[0] != 0u || frame[1] != 0u || frame[2] != (8u | cpl) ||
                        frame[3] != (2u | VCPU_EFLAGS_RF | (vm86 ? VCPU_EFLAGS_VM : 0u)))
                        return LIB_FALSE;
                }
            }
        }
    }
    return LIB_TRUE;
}

typedef struct test_register_fetch_fixture {
    cpu_instruction_fixture instruction;
    lib_u32 fail_at;
    lib_u32 failures;
} test_register_fetch_fixture;

static lib_status test_register_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    test_register_fetch_fixture *fixture = opaque;

    if (!observe_only && address <= fixture->fail_at &&
        (lib_u64)address + bytes > fixture->fail_at &&
        (provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH ||
         provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH)) {
        ++fixture->failures;
        return LIB_STATUS_IO_ERROR;
    }
    return cpu_instruction_read(&fixture->instruction, address, destination,
        bytes, provenance, observe_only, reset_fetch, cache_control);
}

static lib_bool test_register_failed_fetch(void)
{
    static const core_machine_cpu_bus_provider bus = {
        .read_memory = test_register_read, .write_memory = cpu_instruction_write,
        .interrupt_pending = cpu_instruction_interrupt_pending};

    for (core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_80386;
         profile <= CORE_MACHINE_CPU_PROFILE_80486SX; ++profile)
    for (lib_u8 write = 0u; write != 2u; ++write) {
        for (lib_u8 reg = 6u; reg != 8u; ++reg) {
            const lib_u8 code[] = {0x66u, 0x67u, 0x0fu, write ? 0x26u : 0x24u,
                (lib_u8)(0xc0u | (reg << 3u))};
            for (lib_u32 fail_at = 0u; fail_at != sizeof(code); ++fail_at) {
                test_register_fetch_fixture fixture;
                t_cpu before, after;

                cpu_instruction_prepare_with_bus(&fixture.instruction,
                    profile, &bus, &fixture);
                fixture.fail_at = fail_at;
                fixture.failures = 0u;
                fixture.instruction.cpu.data.eax = 0x12345678u;
                fixture.instruction.cpu.data.tr6 = 0x87654321u;
                fixture.instruction.cpu.data.tr7 = 0xabcdef01u;
                before = fixture.instruction.cpu;
                (void)cpu_instruction_run(&fixture.instruction, code, sizeof(code), &after);
                if (fixture.failures == 0u || !fixture.instruction.fault.valid ||
                    fixture.instruction.fault.exception_mask != VCPUINS_EXCEPT_CE ||
                    fixture.instruction.writes != 0u ||
                    lib_memory_compare(&before, &after, sizeof(before)) != 0)
                    return LIB_FALSE;
            }
        }
    }
    return LIB_TRUE;
}

int main(void)
{
    if (!test_register_retained_transfers() || !test_register_invalid_forms() ||
        !test_register_privilege() || !test_register_failed_fetch()) {
        lib_c_fprintf(lib_c_stderr, "CPU-TEST-REGISTER:FAIL\n");
        return 1;
    }
    lib_c_printf("CPU-TEST-REGISTER:OK\n");
    return 0;
}
