#include "support/cpu_bus_fixture.h"
#include "lib/types/file.h"

typedef struct decode_fixture {
    cpu_bus_fixture bus;
    lib_u32 fail_address;
    lib_u32 required_reads;
    lib_u32 data_reads;
    lib_u32 operand_reads;
    lib_u32 injected;
    lib_u32 injected_address;
    lib_bool fail_required;
} decode_fixture;

static lib_status decode_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch)
{
    decode_fixture *fixture = (decode_fixture *)opaque;

    if (!observe_only) {
        if (provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH) {
            ++fixture->required_reads;
            if (fixture->fail_required && address <= fixture->fail_address &&
                (lib_u64)address + bytes > fixture->fail_address) {
                ++fixture->injected;
                fixture->injected_address = address;
                return LIB_STATUS_IO_ERROR;
            }
        } else if (provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) {
            ++fixture->data_reads;
            if (address <= 0x400u && (lib_u64)address + bytes > 0x400u)
                ++fixture->operand_reads;
        }
    }
    return cpu_bus_read(&fixture->bus, address, destination, bytes, provenance,
        observe_only, reset_fetch);
}

static const core_machine_cpu_bus_provider decode_provider = {
    .read_memory = decode_read,
    .write_memory = cpu_bus_write,
    .transfer_port = cpu_bus_port,
    .complete_port = cpu_bus_complete,
    .interrupt_pending = cpu_bus_pending,
    .acknowledge_interrupt = cpu_bus_acknowledge,
    .extension_command = cpu_bus_extension
};

static void decode_prepare(decode_fixture *fixture,
    core_machine_cpu_profile profile)
{
    lib_memory_set(fixture, 0, sizeof(*fixture));
    cpu_bus_prepare(&fixture->bus, profile);
    fixture->bus.execution.bus = &decode_provider;
    fixture->bus.execution.bus_context = fixture;
    fixture->bus.cpu.data.idtr.limit = 0u;
}

/* These decode cases deliberately leave protected-mode exception delivery
 * unconfigured. The CPU keeps source classification internally, but its
 * observable terminal result is resident shutdown, not a host fault. */
static lib_bool decode_expect_shutdown(const decode_fixture *fixture, lib_u32 eip)
{
    return core_machine_cpu_is_shutdown(&fixture->bus.execution) &&
        !fixture->bus.execution.stop_requested && fixture->bus.faults == 0u &&
        fixture->bus.instructions.data.except == VCPUINS_EXCEPT_SHUTDOWN &&
        fixture->bus.cpu.data.eip == eip;
}

/* One table covers the complete unchecked-call class and each required suffix
 * byte. Providers distinguish failed fetch from any dependent operand effect. */
static lib_i32 decode_failed_suffixes(void)
{
    static const struct {
        const char *name;
        lib_u8 code[6];
        lib_u8 bytes;
    } cases[] = {
        { "TEST", {0x85u,0x06u,0x00u,0x04u}, 4u },
        { "XCHG", {0x87u,0x06u,0x00u,0x04u}, 4u },
        { "MOV-store", {0x89u,0x06u,0x00u,0x04u}, 4u },
        { "MOV-load", {0x8bu,0x06u,0x00u,0x04u}, 4u },
        { "LEA", {0x8du,0x06u,0x00u,0x04u}, 4u },
        { "CALL-far", {0x9au,0x00u,0x02u,0x00u,0x00u}, 5u },
        { "RET", {0xc2u,0x02u,0x00u}, 3u },
        { "RETF", {0xcau,0x02u,0x00u}, 3u },
        { "INT", {0xcdu,0x20u}, 2u },
        { "AAM", {0xd4u,0x0au}, 2u },
        { "AAD", {0xd5u,0x0au}, 2u },
        { "JCXZ", {0xe3u,0x02u}, 2u },
        { "JMP", {0xe9u,0x02u,0x00u}, 3u }
    };

    for (core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_8086;
        profile <= CORE_MACHINE_CPU_PROFILE_80386; ++profile) {
        for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
            for (lib_u8 fail_byte = 1u; fail_byte < cases[index].bytes; ++fail_byte) {
                decode_fixture fixture;
                t_cpu before;

                decode_prepare(&fixture, profile);
                fixture.bus.execution.prefetch_capacity = 1u;
                fixture.fail_required = LIB_TRUE;
                fixture.fail_address = fixture.bus.cpu.data.eip + fail_byte;
                lib_memory_copy(fixture.bus.memory + fixture.bus.cpu.data.eip,
                    cases[index].code, cases[index].bytes);
                before = fixture.bus.cpu;
                core_machine_cpu_execution_refresh(&fixture.bus.execution);
                if (fixture.injected != 1u || fixture.data_reads != 0u ||
                    fixture.bus.writes != 0u || fixture.bus.completions != 0u ||
                    fixture.bus.faults != 1u ||
                    fixture.bus.fault.exception_mask != VCPUINS_EXCEPT_CE ||
                    fixture.bus.fault.exception_code != fixture.injected_address ||
                    fixture.bus.fault.point.eip != before.data.eip ||
                    lib_memory_compare(&before, &fixture.bus.cpu, sizeof(before))) {
                    lib_c_printf("decode suffix: %s profile=%u byte=%u\n",
                        cases[index].name, (lib_u32)profile, (lib_u32)fail_byte);
                    return 1;
                }
            }
        }
    }
    return 0;
}

static lib_i32 decode_family_lengths(void)
{
    for (core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_8086;
        profile <= CORE_MACHINE_CPU_PROFILE_80386; ++profile) {
        lib_u8 legal_bytes = profile == CORE_MACHINE_CPU_PROFILE_80286 ? 10u :
            profile == CORE_MACHINE_CPU_PROFILE_80386 ? 15u : 32u;

        for (lib_u8 over = 0u; over != 2u; ++over) {
            for (lib_u8 cached = 0u; cached != 2u; ++cached) {
                decode_fixture fixture;
                lib_u8 bytes = legal_bytes + over;
                lib_u32 start;
                lib_bool rejected = over && profile >= CORE_MACHINE_CPU_PROFILE_80286;

                decode_prepare(&fixture, profile);
                start = fixture.bus.cpu.data.eip;
                if (!cached) fixture.bus.execution.prefetch_capacity = 1u;
                lib_memory_set(fixture.bus.memory + start, 0x26u, bytes - 1u);
                fixture.bus.memory[start + bytes - 1u] = 0x90u;
                core_machine_cpu_execution_refresh(&fixture.bus.execution);
                if ((rejected && !decode_expect_shutdown(&fixture, start)) ||
                    (!rejected && (fixture.bus.instructions.data.except != 0u ||
                        fixture.bus.faults != 0u || fixture.bus.cpu.data.eip != start + bytes))) {
                    lib_c_printf("decode length: profile=%u bytes=%u cached=%u mask=%u\n",
                        (lib_u32)profile, (lib_u32)bytes, (lib_u32)cached,
                        fixture.bus.instructions.data.except);
                    return 1;
                }
            }
        }
    }
    return 0;
}

static lib_i32 decode_endpoint_publication(void)
{
    for (core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_8086;
        profile <= CORE_MACHINE_CPU_PROFILE_80386; ++profile) {
        decode_fixture fixture;
        lib_u32 next = profile >= CORE_MACHINE_CPU_PROFILE_80286 ? 0x10000u : 0u;

        decode_prepare(&fixture, profile);
        fixture.bus.cpu.data.eip = 0xffffu;
        fixture.bus.memory[0x7ffu] = 0x90u;
        /* NOP must not validate a pointer that it never dereferences. */
        fixture.bus.cpu.data.ss.limit = 0u;
        core_machine_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.faults || fixture.bus.cpu.data.eip != next) return 1;
        if (profile >= CORE_MACHINE_CPU_PROFILE_80286) {
            core_machine_cpu_execution_refresh(&fixture.bus.execution);
            if (!decode_expect_shutdown(&fixture, next)) return 1;
        }
    }
    return 0;
}

static lib_i32 decode_component_lengths(void)
{
    static const struct {
        lib_u8 code[9];
        lib_u8 bytes;
        lib_bool later;
    } cases[] = {
        {{0xb8u,0x34u,0x12u}, 3u, LIB_FALSE},
        {{0x8bu,0x06u,0x00u,0x04u}, 4u, LIB_FALSE},
        {{0x66u,0xb8u,0x34u,0x12u,0x78u,0x56u}, 6u, LIB_TRUE},
        {{0x67u,0x8bu,0x84u,0x25u,0x00u,0x04u,0x00u,0x00u}, 8u, LIB_TRUE},
        {{0x66u,0x67u,0x8bu,0x84u,0x25u,0x00u,0x04u,0x00u,0x00u}, 9u, LIB_TRUE}
    };

    for (core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_80286;
        profile <= CORE_MACHINE_CPU_PROFILE_80386; ++profile) {
        lib_u8 limit = profile == CORE_MACHINE_CPU_PROFILE_80286 ? 10u : 15u;

        for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
            if (cases[index].later && profile != CORE_MACHINE_CPU_PROFILE_80386)
                continue;
            for (lib_u8 over = 0u; over != 2u; ++over) {
                decode_fixture fixture;
                lib_u8 prefixes = limit + over - cases[index].bytes;
                lib_bool rejected = over != 0u;
                lib_u32 start;

                decode_prepare(&fixture, profile);
                start = fixture.bus.cpu.data.eip;
                lib_memory_set(fixture.bus.memory + start, 0x26u, prefixes);
                lib_memory_copy(fixture.bus.memory + start + prefixes,
                    cases[index].code, cases[index].bytes);
                core_machine_cpu_execution_refresh(&fixture.bus.execution);
                if ((rejected && (!decode_expect_shutdown(&fixture, start) ||
                    fixture.data_reads || fixture.bus.writes)) ||
                    (!rejected && (fixture.bus.instructions.data.except != 0u ||
                        fixture.bus.cpu.data.eip != start + limit))) return 1;
            }
        }
    }
    return 0;
}

static lib_i32 decode_wrapped_immediate(void)
{
    for (core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_8086;
        profile <= CORE_MACHINE_CPU_PROFILE_80386; ++profile) {
        decode_fixture fixture;

        decode_prepare(&fixture, profile);
        fixture.bus.cpu.data.eip = 0xffffu;
        fixture.bus.memory[0x7ffu] = 0xb8u;
        fixture.bus.memory[0u] = 0x34u;
        fixture.bus.memory[1u] = 0x12u;
        core_machine_cpu_execution_refresh(&fixture.bus.execution);
        if (profile < CORE_MACHINE_CPU_PROFILE_80286) {
            if (fixture.bus.faults || fixture.bus.cpu.data.eip != 2u ||
                fixture.bus.cpu.data.ax != 0x1234u ||
                fixture.bus.instructions.data.oplen != 3u ||
                lib_memory_compare(fixture.bus.instructions.data.opcodes,
                    (const lib_u8[]){0xb8u,0x34u,0x12u}, 3u)) return 1;
        } else if (!decode_expect_shutdown(&fixture, 0xffffu) ||
            fixture.bus.cpu.data.ax != 0x3210u) return 1;
    }
    return 0;
}

typedef struct decode_paging_fixture {
    decode_fixture decode;
    lib_u8 memory[0x5000];
    lib_u32 runtime_page_one_reads;
} decode_paging_fixture;

static lib_status decode_paging_read(void *opaque, lib_u32 address,
    void *destination, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch)
{
    decode_paging_fixture *fixture = (decode_paging_fixture *)opaque;

    (void)reset_fetch;
    if ((lib_u64)address + bytes > sizeof(fixture->memory))
        return LIB_STATUS_IO_ERROR;
    if (!observe_only && (provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH ||
        provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH) && address >= 0x4000u)
        ++fixture->runtime_page_one_reads;
    lib_memory_copy(destination, fixture->memory + address, bytes);
    return LIB_STATUS_OK;
}

static lib_status decode_paging_write(void *opaque, lib_u32 address,
    const void *source, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance)
{
    decode_paging_fixture *fixture = (decode_paging_fixture *)opaque;

    (void)provenance;
    if ((lib_u64)address + bytes > sizeof(fixture->memory))
        return LIB_STATUS_IO_ERROR;
    lib_memory_copy(fixture->memory + address, source, bytes);
    return LIB_STATUS_OK;
}

static const core_machine_cpu_bus_provider decode_paging_provider = {
    .read_memory = decode_paging_read,
    .write_memory = decode_paging_write,
    .transfer_port = cpu_bus_port,
    .complete_port = cpu_bus_complete,
    .interrupt_pending = cpu_bus_pending,
    .acknowledge_interrupt = cpu_bus_acknowledge,
    .extension_command = cpu_bus_extension
};

static lib_i32 decode_page_boundary(void)
{
    for (lib_u8 immediate = 0u; immediate != 2u; ++immediate) {
        for (lib_u8 mapped = 0u; mapped != 2u; ++mapped) {
            decode_paging_fixture fixture;
            core_machine_cpu_instruction_lexeme lexeme;
            lib_u32 pde = 0x2007u;
            lib_u32 ptes[] = {0x3007u, mapped ? 0x4007u : 0u};
            t_cpu before;
            lib_u8 before_tables[0x1008];

            lib_memory_set(&fixture, 0, sizeof(fixture));
            decode_prepare(&fixture.decode, CORE_MACHINE_CPU_PROFILE_80386);
            fixture.decode.bus.execution.bus = &decode_paging_provider;
            fixture.decode.bus.execution.bus_context = &fixture;
            fixture.decode.bus.cpu.data.cr0 = VCPU_CR0_PE | VCPU_CR0_PG;
            fixture.decode.bus.cpu.data.cr3 = 0x1000u;
            fixture.decode.bus.cpu.data.eip = 0xfffu;
            lib_memory_copy(fixture.memory + 0x1000u, &pde, sizeof(pde));
            lib_memory_copy(fixture.memory + 0x2000u, ptes, sizeof(ptes));
            fixture.memory[0x3fffu] = immediate ? 0xb8u : 0x90u;
            fixture.memory[0x4000u] = 0x34u;
            fixture.memory[0x4001u] = 0x12u;
            before = fixture.decode.bus.cpu;
            lib_memory_copy(before_tables, fixture.memory + 0x1000u, sizeof(before_tables));
            if (core_machine_cpu_execution_preview_lexeme(&fixture.decode.bus.execution,
                    &lexeme) != (!immediate || mapped) ||
                lib_memory_compare(&before, &fixture.decode.bus.cpu, sizeof(before)) ||
                lib_memory_compare(before_tables, fixture.memory + 0x1000u,
                    sizeof(before_tables))) return 1;
            core_machine_cpu_execution_refresh(&fixture.decode.bus.execution);
            if (immediate && !mapped) {
                if (!decode_expect_shutdown(&fixture.decode, 0xfffu) ||
                    fixture.decode.bus.cpu.data.cr2 != 0x1000u ||
                    fixture.decode.bus.cpu.data.eip != 0xfffu) return 1;
            } else if (fixture.decode.bus.faults ||
                fixture.decode.bus.cpu.data.eip != (immediate ? 0x1002u : 0x1000u) ||
                fixture.runtime_page_one_reads != (immediate ? 1u : 0u) ||
                (immediate && fixture.decode.bus.cpu.data.ax != 0x1234u)) return 1;
        }
    }
    return 0;
}

static lib_i32 decode_privileged_operands(void)
{
    static const lib_u8 forms[][5] = {
        {0x0fu,0x00u,0x16u,0x00u,0x04u}, /* LLDT */
        {0x0fu,0x00u,0x1eu,0x00u,0x04u}, /* LTR */
        {0x0fu,0x01u,0x16u,0x00u,0x04u}, /* LGDT */
        {0x0fu,0x01u,0x1eu,0x00u,0x04u}  /* LIDT */
    };

    for (core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_80286;
        profile <= CORE_MACHINE_CPU_PROFILE_80386; ++profile) {
        for (lib_size index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
            for (lib_u8 mode = 0u; mode <
                (profile == CORE_MACHINE_CPU_PROFILE_80386 ? 3u : 1u); ++mode) {
                decode_fixture fixture;
                lib_u32 start;
                lib_u32 entry;

                decode_prepare(&fixture, profile);
                entry = fixture.bus.cpu.data.eip;
                fixture.bus.cpu.data.cr0 = VCPU_CR0_PE;
                if (mode == 2u) fixture.bus.cpu.data.eflags |= VCPU_EFLAGS_VM;
                else {
                    fixture.bus.cpu.data.cs.selector = 0x0bu;
                    fixture.bus.cpu.data.cs.dpl = 3u;
                    fixture.bus.cpu.data.ds.selector = 0x13u;
                    fixture.bus.cpu.data.ds.dpl = 3u;
                }
                start = fixture.bus.cpu.data.eip;
                if (mode == 1u) {
                    fixture.bus.cpu.data.cs.seg.exec.defsize = LIB_TRUE;
                    fixture.bus.memory[start++] = 0x67u;
                }
                lib_memory_copy(fixture.bus.memory + start,
                    forms[index], sizeof(forms[index]));
                core_machine_cpu_execution_refresh(&fixture.bus.execution);
                /* The deliberately invalid IDT rejects delivery. Operand reads
                    * must be absent before CPU-owned resident shutdown. */
                if ((mode != 2u && fixture.data_reads != 0u) || fixture.operand_reads != 0u ||
                    !decode_expect_shutdown(&fixture, entry) ||
                    fixture.bus.cpu.data.ldtr.selector != 0u ||
                    fixture.bus.cpu.data.tr.selector != 0u) {
                    lib_c_printf("privileged: profile=%u mode=%u form=%u reads=%u mask=%X\n",
                        (lib_u32)profile, (lib_u32)mode, (lib_u32)index,
                        fixture.data_reads, fixture.bus.instructions.data.except);
                    return 1;
                }
            }
        }
    }
    return 0;
}

static lib_i32 decode_initial_fetch_failure(void)
{
    decode_fixture fixture;
    t_cpu before;

    decode_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
    fixture.bus.fail_transfer = LIB_TRUE;
    fixture.bus.cpu.data.dr0 = fixture.bus.cpu.data.eip;
    fixture.bus.cpu.data.dr7 = 1u;
    before = fixture.bus.cpu;
    core_machine_cpu_execution_refresh(&fixture.bus.execution);
    return fixture.bus.faults != 1u ||
        fixture.bus.fault.exception_mask != VCPUINS_EXCEPT_CE ||
        fixture.required_reads != 0u || fixture.data_reads != 0u ||
        lib_memory_compare(&before, &fixture.bus.cpu, sizeof(before));
}

lib_i32 main(void)
{
    static const struct {
        const char *name;
        lib_i32 (*run)(void);
    } cases[] = {
        {"failed-suffixes", decode_failed_suffixes},
        {"family-lengths", decode_family_lengths},
        {"endpoint-publication", decode_endpoint_publication},
        {"component-lengths", decode_component_lengths},
        {"wrapped-immediate", decode_wrapped_immediate},
        {"page-boundary", decode_page_boundary},
        {"privileged-operands", decode_privileged_operands},
        {"initial-fetch-failure", decode_initial_fetch_failure}
    };

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        if (cases[index].run()) {
            lib_c_printf("CPU-DECODE-ADMISSION:%s:FAILED\n", cases[index].name);
            return 1;
        }
    }
    lib_c_printf("%s\n", "CPU-DECODE-ADMISSION:OK");
    return 0;
}
