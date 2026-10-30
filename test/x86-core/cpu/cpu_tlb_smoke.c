#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"
#include "x86-core/cpu/cpu_timing.h"

/* Commands and operands go through real CPU dispatch, not a test TLB API. */
static lib_bool tlb_run(cpu_instruction_fixture *fixture,
    const lib_u8 *code, lib_u8 bytes)
{
    t_cpu after;

    fixture->cpu.data.eip = 0u;
    x86_core_cpu_execution_invalidate_prefetch(&fixture->execution);
    return cpu_instruction_run(fixture, code, bytes, &after) == LIB_STATUS_OK &&
        !fixture->delivered_exception.valid && !fixture->fault.valid &&
        !x86_core_cpu_is_shutdown(&fixture->execution) &&
        after.data.eip == bytes;
}

static lib_bool tlb_write_test(cpu_instruction_fixture *fixture,
    lib_u32 command, lib_u32 data)
{
    static const lib_u8 write_data[] = {0x0fu, 0x26u, 0xfau};
    static const lib_u8 write_command[] = {0x0fu, 0x26u, 0xf2u};

    fixture->cpu.data.edx = data;
    if (!tlb_run(fixture, write_data, sizeof(write_data))) return LIB_FALSE;
    fixture->cpu.data.edx = command;
    return tlb_run(fixture, write_command, sizeof(write_command));
}

static lib_bool tlb_lookup(cpu_instruction_fixture *fixture,
    lib_u32 command, lib_u32 *result)
{
    static const lib_u8 lookup[] = {0x0fu, 0x26u, 0xf2u};
    static const lib_u8 read_data[] = {0x0fu, 0x24u, 0xf9u};

    fixture->cpu.data.edx = command | 1u;
    if (!tlb_run(fixture, lookup, sizeof(lookup)) ||
        !tlb_run(fixture, read_data, sizeof(read_data))) return LIB_FALSE;
    *result = fixture->cpu.data.ecx;
    return LIB_TRUE;
}

static void tlb_prepare_paging(cpu_instruction_fixture *fixture)
{
    lib_u32 entry = 0x2007u;

    cpu_instruction_prepare(fixture, X86_CORE_CPU_PROFILE_80486SX);
    fixture->cpu.data.cr0 |= VCPU_CR0_PE | VCPU_CR0_PG;
    fixture->cpu.data.cr3 = 0x1000u;
    fixture->cpu.data.ds.limit = LIB_UINT32_MAX;
    fixture->cpu.data.ds.selector = 0x10u;
    lib_memory_copy(fixture->memory + 0x1000u, &entry, sizeof(entry));
    for (lib_u32 page = 0u; page < 128u; ++page) {
        entry = (page << 12u) | 7u;
        lib_memory_copy(fixture->memory + 0x2000u + page * 4u,
            &entry, sizeof(entry));
    }
}

typedef struct tlb_probe {
    cpu_instruction_fixture instruction;
    lib_u32 data_reads;
    lib_u32 fail_at;
    lib_bool fail_fetch;
    lib_bool fail_table_write;
} tlb_probe;

static lib_status tlb_probe_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    tlb_probe *probe = opaque;

    if (!observe_only && provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA)
        ++probe->data_reads;
    if (!observe_only && probe->fail_fetch && address <= probe->fail_at &&
        (lib_u64)address + bytes > probe->fail_at &&
        (provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH ||
         provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH))
        return LIB_STATUS_IO_ERROR;
    return cpu_instruction_read(&probe->instruction, address, destination,
        bytes, provenance, observe_only, reset_fetch, cache_control);
}

static lib_status tlb_probe_write(void *opaque, lib_u32 address, const void *source,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    tlb_probe *probe = opaque;

    if (probe->fail_table_write && address == 0x2100u &&
        provenance == X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE)
        return LIB_STATUS_IO_ERROR;
    return cpu_instruction_write(&probe->instruction, address, source, bytes, provenance, cache_control);
}

static const x86_core_cpu_bus_provider tlb_probe_bus = {
    .read_memory = tlb_probe_read, .write_memory = tlb_probe_write,
    .interrupt_pending = cpu_instruction_interrupt_pending};

static lib_bool tlb_instruction_forms(void)
{
    static const lib_u8 overrides[] = {0u, 0x26u, 0x2eu, 0x36u, 0x64u, 0x65u};

    for (lib_u8 mode = 0u; mode < 4u; ++mode)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes)
    for (lib_u8 segment = 0u; segment < sizeof(overrides); ++segment) {
        tlb_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_bool code32 = (mode & 1u) != 0u;
        const lib_bool address32 = code32 != ((prefixes & 2u) != 0u);
        const lib_u32 linear = segment == 2u ? 0x1234u : 0x11234u;
        lib_u8 code[10], bytes = 0u;
        lib_u32 result;
        lib_u64 nominal_ticks;
        x86_core_cpu_instruction_lexeme lexeme;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &tlb_probe_bus, &probe);
        fixture->cpu.data.cs.seg.exec.defsize = code32;
        if (mode >= 2u) fixture->cpu.data.cr0 |= VCPU_CR0_PE;
        fixture->cpu.data.ds.base = fixture->cpu.data.es.base = 0x10000u;
        fixture->cpu.data.ss.base = fixture->cpu.data.fs.base =
            fixture->cpu.data.gs.base = 0x10000u;
        if (!tlb_write_test(fixture, (linear & VCPU_CR3_BASE) | 0xd40u, 0x30010u))
            return LIB_FALSE;
        if (overrides[segment]) code[bytes++] = overrides[segment];
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0x0fu; code[bytes++] = 0x01u;
        code[bytes++] = address32 ? 0x3du : 0x3eu;
        code[bytes++] = 0x34u; code[bytes++] = 0x12u;
        if (address32) { code[bytes++] = 0u; code[bytes++] = 0u; }
        const lib_u32 flags = fixture->cpu.data.eflags;
        const lib_u32 writes = fixture->writes;
        if (!x86_core_cpu_instruction_lexeme_scan(code, bytes,
                X86_CORE_CPU_PROFILE_80486SX, code32, &lexeme) ||
            lexeme.byte_count != bytes || !tlb_run(fixture, code, bytes) ||
            fixture->cpu.data.eflags != flags || fixture->writes != writes ||
            probe.data_reads != 0u ||
            !x86_core_cpu_80486_nominal_instruction_cost(&fixture->execution, &nominal_ticks) ||
            nominal_ticks != 12u || !fixture->instructions.data.timing_invlpg_hit ||
            !tlb_run(fixture, code, bytes) ||
            !x86_core_cpu_80486_nominal_instruction_cost(&fixture->execution, &nominal_ticks) ||
            nominal_ticks != 11u || fixture->instructions.data.timing_invlpg_hit ||
            probe.data_reads != 0u || fixture->writes != writes ||
            !tlb_lookup(fixture, (linear & VCPU_CR3_BASE) | 0xfe0u, &result) ||
            (result & 0x10u)) return LIB_FALSE;
        for (lib_u8 available = 0u; available < bytes; ++available)
            if (x86_core_cpu_instruction_lexeme_scan(code, available,
                    X86_CORE_CPU_PROFILE_80486SX, code32, &lexeme))
                return LIB_FALSE;
    }
    /* Test-register operands stay dword-sized despite 66h and code size. */
    for (lib_u8 mode = 0u; mode < 4u; ++mode)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes)
    for (lib_u8 reg = 6u; reg < 8u; ++reg) {
        cpu_instruction_fixture fixture;
        lib_u8 code[5], bytes = 0u;
        const lib_u32 value = reg == 6u ? 0x10d40u : 0x30010u;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.cs.seg.exec.defsize = (mode & 1u) != 0u;
        if (mode >= 2u) fixture.cpu.data.cr0 |= VCPU_CR0_PE;
        fixture.cpu.data.edx = value;
        fixture.cpu.data.tr7 = 0x30010u;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0x0fu; code[bytes++] = 0x26u;
        code[bytes++] = (lib_u8)(0xc2u | (reg << 3u));
        if (!tlb_run(&fixture, code, bytes)) return LIB_FALSE;
        code[bytes - 2u] = 0x24u;
        fixture.cpu.data.edx = 0u;
        if (!tlb_run(&fixture, code, bytes) || fixture.cpu.data.edx != value)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool tlb_effective_addresses(void)
{
    static const lib_u32 registers[] = {
        0x1000u, 0x1100u, 0x1200u, 0x1300u, 0x1400u, 0x1500u, 0x1600u, 0x1700u};
    static const lib_u32 offsets16[] = {
        0x2900u, 0x2a00u, 0x2b00u, 0x2c00u, 0x1600u, 0x1700u, 0x1500u, 0x1300u};

    for (lib_u8 address32 = 0u; address32 < 2u; ++address32)
    for (lib_u8 mod = 0u; mod < 3u; ++mod)
    for (lib_u8 rm = 0u; rm < 8u; ++rm)
    for (lib_u16 sib = 0u; sib < (address32 && rm == 4u ? 256u : 1u); ++sib) {
        tlb_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        lib_u8 code[10], bytes = 0u, displacement = 0u;
        lib_u32 offset, segment_base, result;

        if (address32) {
            lib_u8 base = rm;
            const lib_u8 index = (sib >> 3u) & 7u;
            if (rm == 4u) base = sib & 7u;
            const lib_bool no_base = mod == 0u && base == 5u;
            offset = no_base ? 0u : registers[base];
            if (rm == 4u && index != 4u)
                offset += registers[index] << (sib >> 6u);
            segment_base = !no_base && (base == 4u || base == 5u) ? 0x20000u : 0x10000u;
            displacement = no_base ? 4u : 0u;
        } else {
            offset = offsets16[rm];
            segment_base = rm == 2u || rm == 3u || (rm == 6u && mod != 0u) ?
                0x20000u : 0x10000u;
            if (mod == 0u && rm == 6u) { offset = 0u; displacement = 2u; }
        }
        const lib_u32 delta = mod == 0u ? 0x1234u : LIB_UINT32_MAX - 15u;
        if (mod == 1u) displacement = 1u;
        if (mod == 2u) displacement = address32 ? 4u : 2u;
        if (displacement) offset += delta;
        if (!address32) offset &= 0xffffu;
        const lib_u32 linear = segment_base + offset;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &tlb_probe_bus, &probe);
        fixture->cpu.data.ds.base = 0x10000u;
        fixture->cpu.data.ss.base = 0x20000u;
        if (!tlb_write_test(fixture, (linear & VCPU_CR3_BASE) | 0xd40u, 0x30010u))
            return LIB_FALSE;
        fixture->cpu.data.eax = registers[0]; fixture->cpu.data.ecx = registers[1];
        fixture->cpu.data.edx = registers[2]; fixture->cpu.data.ebx = registers[3];
        fixture->cpu.data.esp = registers[4]; fixture->cpu.data.ebp = registers[5];
        fixture->cpu.data.esi = registers[6]; fixture->cpu.data.edi = registers[7];
        if (address32) code[bytes++] = 0x67u;
        code[bytes++] = 0x0fu; code[bytes++] = 0x01u;
        code[bytes++] = (lib_u8)((mod << 6u) | 0x38u | rm);
        if (address32 && rm == 4u) code[bytes++] = (lib_u8)sib;
        for (lib_u8 byte = 0u; byte < displacement; ++byte)
            code[bytes++] = (lib_u8)(delta >> (byte * 8u));
        if (!tlb_run(fixture, code, bytes) || probe.data_reads != 0u ||
            !tlb_lookup(fixture, (linear & VCPU_CR3_BASE) | 0xfe0u, &result) ||
            (result & 0x10u)) {
            lib_c_fprintf(lib_c_stderr, "INVLPG EA address32=%u mod=%u rm=%u sib=%u linear=%x\n",
                address32, mod, rm, sib, linear);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool tlb_failed_fetch_and_commit(void)
{
    static const lib_u8 code[] = {0x66u, 0x67u, 0x0fu, 0x01u, 0x3du,
        0u, 0u, 1u, 0u};

    for (lib_u32 fail_at = 0u; fail_at < sizeof(code); ++fail_at) {
        tlb_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        x86_core_cpu_execution_context before;
        t_cpu cpu_before, after;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &tlb_probe_bus, &probe);
        if (!tlb_write_test(fixture, 0x10d40u, 0x30010u)) return LIB_FALSE;
        fixture->cpu.data.eip = 0u;
        cpu_before = fixture->cpu;
        before = fixture->execution;
        probe.fail_fetch = LIB_TRUE;
        probe.fail_at = fail_at;
        if (cpu_instruction_run(fixture, code, sizeof(code), &after) !=
                LIB_STATUS_INTERNAL_ERROR || !fixture->fault.valid ||
            fixture->fault.exception_mask != VCPUINS_EXCEPT_CE ||
            lib_memory_compare(&cpu_before, &after, sizeof(after)) != 0 ||
            lib_memory_compare(before.tlb, fixture->execution.tlb, sizeof(before.tlb)) != 0 ||
            lib_memory_compare(before.tlb_lru, fixture->execution.tlb_lru,
                sizeof(before.tlb_lru)) != 0) return LIB_FALSE;
    }
    {
        static const lib_u8 read[] = {0x67u, 0xa0u, 0u, 0u, 4u, 0u};
        tlb_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        t_cpu before, after;

        tlb_prepare_paging(fixture);
        fixture->execution.bus = &tlb_probe_bus;
        fixture->execution.bus_context = &probe;
        probe.fail_table_write = LIB_TRUE;
        before = fixture->cpu;
        if (cpu_instruction_run(fixture, read, sizeof(read), &after) !=
                LIB_STATUS_INTERNAL_ERROR || !fixture->fault.valid ||
            fixture->fault.exception_mask != VCPUINS_EXCEPT_CE ||
            probe.data_reads != 0u || after.data.eax != before.data.eax ||
            after.data.eip != before.data.eip) return LIB_FALSE;
        for (lib_u8 way = 0u; way < 4u; ++way)
            if (fixture->execution.tlb[0][way].valid &&
                fixture->execution.tlb[0][way].linear == 0x40000u)
                return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool tlb_invlpg_rejections(void)
{
    static const lib_u8 forms[][7] = {
        {0x0fu, 0x01u, 0xf8u},
        {0xf0u, 0x0fu, 0x01u, 0x3eu, 0u, 1u},
        {0x0fu, 0x01u, 0x3eu, 0u, 1u}};
    static const lib_u8 lengths[] = {3u, 6u, 5u};

    for (lib_u8 form = 0u; form < 3u; ++form) {
        cpu_instruction_fixture fixture;
        x86_core_cpu_execution_context before;

        cpu_instruction_prepare(&fixture, form == 2u ?
            X86_CORE_CPU_PROFILE_80386 : X86_CORE_CPU_PROFILE_80486SX);
        if (form != 2u && !tlb_write_test(&fixture, 0x10d40u, 0x30010u))
            return LIB_FALSE;
        fixture.cpu.data.eip = 0u;
        before = fixture.execution;
        if (!cpu_instruction_expect_real_fault(&fixture, forms[form], lengths[form], 6u) ||
            lib_memory_compare(before.tlb, fixture.execution.tlb, sizeof(before.tlb)) != 0)
            return LIB_FALSE;
    }
    for (lib_u8 mode = 1u; mode < 5u; ++mode) {
        const lib_bool vm86 = mode == 4u;
        const lib_u8 cpl = vm86 ? 3u : mode;
        const lib_u8 target_cpl = vm86 ? 0u : cpl;
        const lib_u8 descriptor[] = {0xffu, 0xffu, 0u, 0u, 0u,
            (lib_u8)(0x9bu | (target_cpl << 5u)), 0u, 0u};
        const lib_u8 stack[] = {0xffu, 0xffu, 0u, 0u, 0u, 0x93u, 0u, 0u};
        const lib_u8 gate[] = {0u, 3u, (lib_u8)(8u | target_cpl),
            0u, 0u, 0x8eu, 0u, 0u};
        const lib_u8 code[] = {0x0fu, 0x01u, 0x3eu, 0u, 1u};
        cpu_instruction_fixture fixture;
        x86_core_cpu_execution_context before;
        t_cpu after;
        const lib_u32 esp0 = 0x1000u;
        const lib_u16 ss0 = 0x10u;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        if (!tlb_write_test(&fixture, 0x10d40u, 0x30010u)) return LIB_FALSE;
        fixture.cpu.data.eip = 0u;
        fixture.cpu.data.cr0 |= VCPU_CR0_PE;
        fixture.cpu.data.cs.selector = (lib_u16)(8u | cpl);
        fixture.cpu.data.cs.dpl = cpl;
        fixture.cpu.data.ss.selector = (lib_u16)(0x10u | cpl);
        fixture.cpu.data.ss.dpl = cpl;
        fixture.cpu.data.esp = 0x800u;
        fixture.cpu.data.eflags = 2u | (vm86 ? VCPU_EFLAGS_VM : 0u);
        fixture.cpu.data.gdtr.base = 0x200u;
        fixture.cpu.data.gdtr.limit = 23u;
        fixture.cpu.data.idtr.base = 0x400u;
        fixture.cpu.data.idtr.limit = 0x87u;
        lib_memory_copy(fixture.memory + 0x208u, descriptor, sizeof(descriptor));
        lib_memory_copy(fixture.memory + 0x210u, stack, sizeof(stack));
        lib_memory_copy(fixture.memory + 0x468u, gate, sizeof(gate));
        if (vm86) {
            fixture.cpu.data.tr = (t_cpu_data_sreg) {
                .flagValid = LIB_TRUE, .sregtype = SREG_TR, .selector = 0x18u,
                .base = 0x600u, .limit = 0x67u};
            fixture.cpu.data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
            lib_memory_copy(fixture.memory + 0x604u, &esp0, sizeof(esp0));
            lib_memory_copy(fixture.memory + 0x608u, &ss0, sizeof(ss0));
        }
        before = fixture.execution;
        if (cpu_instruction_run(&fixture, code, sizeof(code), &after) != LIB_STATUS_OK ||
            fixture.fault.valid || x86_core_cpu_is_shutdown(&fixture.execution) ||
            !fixture.delivered_exception.valid ||
            fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_GP ||
            fixture.delivered_exception.exception_code != 0u ||
            fixture.delivered_exception.point.eip != 0u || after.data.eip != 0x300u ||
            lib_memory_compare(before.tlb, fixture.execution.tlb, sizeof(before.tlb)) != 0)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool tlb_all_sets_ways(void)
{
    for (lib_u8 set = 0u; set < 8u; ++set) {
        for (lib_u8 way = 0u; way < 4u; ++way) {
            cpu_instruction_fixture fixture;
            const lib_u32 linear = 0x40000u + (set << 12u) + (way << 15u);
            const lib_u32 physical = 0x30000u + (set << 12u) + (way << 15u);
            const lib_u32 cache_bits = ((way & 1u) ? 0x800u : 0u) |
                ((way & 2u) ? 0x400u : 0u);
            const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | 2u;
            const lib_u8 read[] = {0x67u, 0xa0u, (lib_u8)linear,
                (lib_u8)(linear >> 8u), (lib_u8)(linear >> 16u),
                (lib_u8)(linear >> 24u)};
            lib_u32 result = 0u;

            tlb_prepare_paging(&fixture);
            fixture.cpu.data.eflags = flags;
            fixture.memory[physical] = (lib_u8)(0x80u + set * 4u + way);
            fixture.memory[linear] = 0x5au;
            if (!tlb_write_test(&fixture, linear | 0xd40u,
                    physical | cache_bits | 0x10u | (way << 2u)) ||
                !tlb_lookup(&fixture, linear | 0xfe0u, &result) ||
                (result & 0xfffffc1cu) !=
                    (physical | cache_bits | 0x10u | (way << 2u)) ||
                !tlb_run(&fixture, read, sizeof(read)) ||
                fixture.cpu.data.al != fixture.memory[physical] ||
                fixture.cpu.data.eflags != flags) {
                lib_c_fprintf(lib_c_stderr,
                    "TLB set=%u way=%u result=%08x al=%02x expected=%02x eip=%x fault=%x exception=%x\n",
                    set, way, result, fixture.cpu.data.al, fixture.memory[physical],
                    fixture.cpu.data.eip, fixture.fault.exception_mask,
                    fixture.delivered_exception.exception_mask);
                return LIB_FALSE;
            }
        }
    }
    return LIB_TRUE;
}

static lib_bool tlb_attribute_matching(void)
{
    cpu_instruction_fixture fixture;

    cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
    for (lib_u8 attributes = 0u; attributes < 8u; ++attributes) {
        const lib_u32 command = 0x41000u | 0x800u |
            ((attributes & 4u) ? 0x400u : 0x200u) |
            ((attributes & 2u) ? 0x100u : 0x80u) |
            ((attributes & 1u) ? 0x40u : 0x20u);

        if (!tlb_write_test(&fixture, command, 0x30010u)) return LIB_FALSE;
        for (lib_u8 pairs = 0u; pairs < 64u; ++pairs) {
            lib_u32 result;
            const lib_u8 dirty = (pairs >> 4u) & 3u;
            const lib_u8 user = (pairs >> 2u) & 3u;
            const lib_u8 writable = pairs & 3u;
            const lib_bool expected =
                (dirty & ((attributes & 4u) ? 2u : 1u)) &&
                (user & ((attributes & 2u) ? 2u : 1u)) &&
                (writable & ((attributes & 1u) ? 2u : 1u));

            if (!tlb_lookup(&fixture, 0x41800u | (dirty << 9u) |
                    (user << 7u) | (writable << 5u), &result) ||
                ((result & 0x10u) != 0u) != expected)
                return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool tlb_replacement_and_invalidation(void)
{
    static const lib_u8 invalidate[] = {0x67u, 0x0fu, 0x01u, 0x3du,
        0u, 0x80u, 1u, 0u};
    static const lib_u8 flush[] = {0x0fu, 0x22u, 0xdau};
    static const lib_u8 enable_paging[] = {0x0fu, 0x22u, 0xc2u};
    static const lib_u8 expected_lru[] = {0u, 3u, 1u, 4u};
    cpu_instruction_fixture fixture;
    lib_u32 result;

    cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
    for (lib_u8 way = 0u; way < 4u; ++way)
        if (!tlb_write_test(&fixture, (0x10000u + (way << 15u)) | 0xd40u,
                0x30010u | (way << 2u))) return LIB_FALSE;
    for (lib_u8 way = 0u; way < 4u; ++way) {
        if (!tlb_lookup(&fixture, (0x10000u + (way << 15u)) | 0xfe0u,
                &result) || (result & 0x10u) == 0u ||
            ((result >> 2u) & 3u) != way ||
            ((result >> 7u) & 7u) != expected_lru[way]) return LIB_FALSE;
    }
    if (!tlb_write_test(&fixture, 0x90d40u, 0x31000u) ||
        !tlb_lookup(&fixture, 0x90fe0u, &result) ||
        (result & 0x1cu) != 0x10u ||
        !tlb_lookup(&fixture, 0x10fe0u, &result) || (result & 0x10u))
        return LIB_FALSE;
    if (!tlb_run(&fixture, invalidate, sizeof(invalidate)) ||
        !tlb_lookup(&fixture, 0x18fe0u, &result) || (result & 0x10u) ||
        !tlb_lookup(&fixture, 0x187e0u, &result) ||
        (result & 0xfffff01cu) != 0x30014u ||
        !tlb_lookup(&fixture, 0x20fe0u, &result) || !(result & 0x10u) ||
        !tlb_run(&fixture, invalidate, sizeof(invalidate)))
        return LIB_FALSE;
    fixture.cpu.data.edx = fixture.cpu.data.cr3;
    if (!tlb_run(&fixture, flush, sizeof(flush))) return LIB_FALSE;
    for (lib_u8 way = 0u; way < 4u; ++way)
        if (!tlb_lookup(&fixture, (0x10000u + (way << 15u)) | 0xfe0u,
                &result) || (result & 0x10u)) return LIB_FALSE;
    if (!tlb_write_test(&fixture, 0x10d40u, 0x30010u)) return LIB_FALSE;
    fixture.cpu.data.edx = VCPU_CR0_PE | VCPU_CR0_PG | VCPU_CR0_ET;
    if (!tlb_run(&fixture, enable_paging, sizeof(enable_paging)) ||
        !(fixture.cpu.data.cr0 & VCPU_CR0_PG)) return LIB_FALSE;
    for (lib_u8 set = 0u; set < X86_CPU_TLB_SETS; ++set)
        for (lib_u8 way = 0u; way < X86_CPU_TLB_WAYS; ++way)
            if (fixture.execution.tlb[set][way].valid) return LIB_FALSE;
    return LIB_TRUE;
}

static lib_bool tlb_walk_preview_and_debug(void)
{
    static const lib_u8 read[] = {0x67u, 0xa0u, 0u, 0u, 4u, 0u};
    static const lib_u8 invalidate[] = {0x67u, 0x0fu, 0x01u, 0x3du,
        0u, 0u, 4u, 0u};
    cpu_instruction_fixture fixture;
    x86_core_cpu_execution_context before;
    x86_core_cpu_instruction_lexeme lexeme;
    x86_core_cpu_debug_register_patch patch = {0};
    lib_u32 result, entry = 0x30007u;

    tlb_prepare_paging(&fixture);
    fixture.memory[0x40000u] = 0x90u;
    fixture.memory[0x30000u] = 0xb8u;
    if (!tlb_run(&fixture, read, sizeof(read)) || fixture.cpu.data.al != 0x90u ||
        !tlb_lookup(&fixture, 0x40fe0u, &result) ||
        (result & 0xfffff010u) != 0x40010u) return LIB_FALSE;
    lib_memory_copy(fixture.memory + 0x2100u, &entry, sizeof(entry));
    if (!tlb_run(&fixture, read, sizeof(read)) || fixture.cpu.data.al != 0x90u)
        return LIB_FALSE;
    /* Preview must see the cached old instruction page, without LRU/A-D
     * publication. Its old NOP is one byte; the remapped MOV is three. */
    fixture.cpu.data.cs.base = 0x40000u;
    fixture.cpu.data.eip = 0u;
    x86_core_cpu_execution_invalidate_prefetch(&fixture.execution);
    before = fixture.execution;
    if (!x86_core_cpu_execution_preview_lexeme(&fixture.execution, &lexeme) ||
        lexeme.byte_count != 1u || fixture.execution.preview_memory_source != LIB_NULL ||
        lib_memory_compare(&before, &fixture.execution, sizeof(before)) != 0)
        return LIB_FALSE;
    lib_memory_copy(&result, fixture.memory + 0x2100u, sizeof(result));
    if (result != entry) return LIB_FALSE;
    fixture.cpu.data.cs.base = 0u;
    if (!tlb_run(&fixture, invalidate, sizeof(invalidate)) ||
        !tlb_run(&fixture, read, sizeof(read)) || fixture.cpu.data.al != 0xb8u)
        return LIB_FALSE;
    entry = 0x40007u;
    lib_memory_copy(fixture.memory + 0x2100u, &entry, sizeof(entry));
    if (!tlb_run(&fixture, read, sizeof(read)) || fixture.cpu.data.al != 0xb8u)
        return LIB_FALSE;
    /* A rejected multi-register debug patch must not invalidate the live
     * mapping merely because its uncommitted candidate changed CR3. */
    patch.mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR3) |
        X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS);
    patch.values[X86_CORE_CPU_DEBUG_CR3] = 0x3000u;
    patch.values[X86_CORE_CPU_DEBUG_CS] = 0u;
    before = fixture.execution;
    const t_cpu cpu_before = fixture.cpu;
    if (x86_core_cpu_debug_patch_registers(&fixture.execution, &patch) != LIB_STATUS_INVALID_STATE ||
        fixture.execution.preview_memory_source != LIB_NULL ||
        lib_memory_compare(&cpu_before, &fixture.cpu, sizeof(cpu_before)) != 0 ||
        lib_memory_compare(before.tlb, fixture.execution.tlb, sizeof(before.tlb)) != 0 ||
        lib_memory_compare(before.tlb_lru, fixture.execution.tlb_lru,
            sizeof(before.tlb_lru)) != 0) return LIB_FALSE;
    patch.mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR3);
    patch.values[X86_CORE_CPU_DEBUG_CR3] = fixture.cpu.data.cr3;
    if (x86_core_cpu_debug_patch_registers(&fixture.execution, &patch) != LIB_STATUS_OK ||
        !tlb_run(&fixture, read, sizeof(read)) || fixture.cpu.data.al != 0x90u)
        return LIB_FALSE;
    x86_core_cpu_state_reset(&fixture.execution);
    for (lib_u8 set = 0u; set < 8u; ++set)
        for (lib_u8 way = 0u; way < 4u; ++way)
            if (fixture.execution.tlb[set][way].valid) return LIB_FALSE;
    return LIB_TRUE;
}

static lib_bool tlb_dirty_transition(void)
{
    static const lib_u8 read[] = {0x67u, 0xa0u, 0u, 0u, 4u, 0u};
    static const lib_u8 write[] = {0x67u, 0xa2u, 0u, 0u, 4u, 0u};
    cpu_instruction_fixture fixture;
    lib_u32 entry, result, writes;

    tlb_prepare_paging(&fixture);
    if (!tlb_run(&fixture, read, sizeof(read)) ||
        !tlb_lookup(&fixture, 0x40be0u, &result) || !(result & 0x10u))
        return LIB_FALSE;
    fixture.cpu.data.al = 0x5cu;
    if (!tlb_run(&fixture, write, sizeof(write)) || fixture.memory[0x40000u] != 0x5cu)
        return LIB_FALSE;
    lib_memory_copy(&entry, fixture.memory + 0x2100u, sizeof(entry));
    if (!(entry & 0x40u) || !tlb_lookup(&fixture, 0x40de0u, &result) ||
        !(result & 0x10u)) return LIB_FALSE;
    /* Clearing memory D without invalidation must not clear cached D or
     * provoke repeated table updates on an already-dirty translation. */
    entry &= ~0x40u;
    lib_memory_copy(fixture.memory + 0x2100u, &entry, sizeof(entry));
    fixture.cpu.data.al = 0x6du;
    writes = fixture.writes;
    if (!tlb_run(&fixture, write, sizeof(write)) || fixture.writes != writes + 1u ||
        fixture.memory[0x40000u] != 0x6du) return LIB_FALSE;
    lib_memory_copy(&result, fixture.memory + 0x2100u, sizeof(result));
    return result == entry;
}

int main(void)
{
    if (!tlb_instruction_forms() || !tlb_effective_addresses() ||
        !tlb_failed_fetch_and_commit() ||
        !tlb_invlpg_rejections()) {
        lib_c_fprintf(lib_c_stderr, "TLB stage=forms-failure\n");
        return 1;
    }
    if (!tlb_all_sets_ways()) {
        lib_c_fprintf(lib_c_stderr, "TLB stage=sets-ways\n");
        return 1;
    }
    if (!tlb_attribute_matching()) {
        lib_c_fprintf(lib_c_stderr, "TLB stage=attributes\n");
        return 1;
    }
    if (!tlb_replacement_and_invalidation()) {
        lib_c_fprintf(lib_c_stderr, "TLB stage=replacement-invalidation\n");
        return 1;
    }
    if (!tlb_walk_preview_and_debug()) {
        lib_c_fprintf(lib_c_stderr, "TLB stage=walk-preview-debug\n");
        return 1;
    }
    if (!tlb_dirty_transition()) {
        lib_c_fprintf(lib_c_stderr, "TLB stage=dirty-transition\n");
        return 1;
    }
    lib_c_fprintf(lib_c_stdout, "80486:TLB:OK\n");
    return 0;
}
