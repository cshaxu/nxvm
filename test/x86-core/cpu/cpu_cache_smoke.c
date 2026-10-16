#include "support/cpu_instruction_fixture.h"
#include "x86-core/cpu/cpu_timing.h"
#include "lib/types/file.h"

static lib_bool cache_run(cpu_instruction_fixture *fixture,
    const lib_u8 *code, lib_u8 bytes)
{
    t_cpu after;

    fixture->cpu.data.eip = 0u;
    x86_core_cpu_execution_invalidate_prefetch(&fixture->execution);
    return cpu_instruction_run(fixture, code, bytes, &after) == LIB_STATUS_OK &&
        !fixture->delivered_exception.valid && !fixture->fault.valid &&
        after.data.eip == bytes;
}

static lib_bool cache_write_register(cpu_instruction_fixture *fixture,
    lib_u8 reg, lib_u32 value)
{
    const lib_u8 code[] = {0x0fu, 0x26u, (lib_u8)(0xc0u | (reg << 3u))};

    fixture->cpu.data.eax = value;
    return cache_run(fixture, code, sizeof(code));
}

static lib_bool cache_read_register(cpu_instruction_fixture *fixture,
    lib_u8 reg, lib_u32 *value)
{
    const lib_u8 code[] = {0x0fu, 0x24u, (lib_u8)(0xc0u | (reg << 3u))};

    if (!cache_run(fixture, code, sizeof(code))) return LIB_FALSE;
    *value = fixture->cpu.data.eax;
    return LIB_TRUE;
}

/* Real MOV dispatch writes and reads every line; no test-only cache API. */
static lib_bool cache_test_geometry(void)
{
    cpu_instruction_fixture fixture;
    lib_u32 value;

    cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
    for (lib_u32 set = 0u; set < 128u; ++set)
    for (lib_u32 way = 0u; way < 4u; ++way) {
        for (lib_u32 word = 0u; word < 4u; ++word)
            if (!cache_write_register(&fixture, 5u, word << 2u) ||
                !cache_write_register(&fixture, 3u,
                    0x12340000u | (set << 8u) | (way << 4u) | word))
                return LIB_FALSE;
        if (!cache_write_register(&fixture, 4u, 0x10000000u | (way << 20u) | 0x7ffu) ||
            !cache_write_register(&fixture, 5u, (set << 4u) | (way << 2u) | 1u))
            return LIB_FALSE;
    }
    for (lib_u32 set = 0u; set < 128u; ++set)
    for (lib_u32 way = 0u; way < 4u; ++way) {
        if (!cache_write_register(&fixture, 5u, (set << 4u) | (way << 2u) | 2u) ||
            !cache_read_register(&fixture, 4u, &value) ||
            value != (0x10000000u | (way << 20u) | 0x478u)) return LIB_FALSE;
        for (lib_u32 word = 0u; word < 4u; ++word)
            if (!cache_write_register(&fixture, 5u, word << 2u) ||
                !cache_read_register(&fixture, 3u, &value) ||
                value != (0x12340000u | (set << 8u) | (way << 4u) | word))
                return LIB_FALSE;
    }
    if (fixture.writes != 0u || !cache_write_register(&fixture, 5u, LIB_UINT32_MAX))
        return LIB_FALSE;
    for (lib_u32 set = 0u; set < 128u; ++set)
    for (lib_u32 way = 0u; way < 4u; ++way) {
        if (!cache_write_register(&fixture, 5u, (set << 4u) | (way << 2u) | 2u) ||
            !cache_read_register(&fixture, 4u, &value) || value != (0x10000000u | (way << 20u)))
            return LIB_FALSE;
        for (lib_u32 word = 0u; word < 4u; ++word)
            if (!cache_write_register(&fixture, 5u, word << 2u) ||
                !cache_read_register(&fixture, 3u, &value) ||
                value != (0x12340000u | (set << 8u) | (way << 4u) | word))
                return LIB_FALSE;
    }
    return fixture.writes == 0u;
}

static lib_bool cache_test_forms(void)
{
    for (lib_u8 mode = 0u; mode < 4u; ++mode)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes)
    for (lib_u8 reg = 3u; reg < 6u; ++reg)
    for (lib_u8 write = 0u; write < 2u; ++write) {
        cpu_instruction_fixture fixture;
        x86_core_cpu_instruction_lexeme lexeme;
        const lib_bool code32 = (mode & 1u) != 0u;
        lib_u8 code[5], bytes = 0u;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        if (mode >= 2u) fixture.cpu.data.cr0 |= VCPU_CR0_PE;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0x0fu;
        code[bytes++] = write ? 0x26u : 0x24u;
        code[bytes++] = (lib_u8)(0xc0u | (reg << 3u));
        if (!write && reg == 5u) {
            if (x86_core_cpu_instruction_lexeme_scan(code, bytes,
                    X86_CORE_CPU_PROFILE_80486SX, code32, &lexeme))
                return LIB_FALSE;
            continue;
        }
        const lib_u32 flags = fixture.cpu.data.eflags;
        if (!x86_core_cpu_instruction_lexeme_scan(code, bytes,
                X86_CORE_CPU_PROFILE_80486SX, code32, &lexeme) ||
            lexeme.byte_count != bytes || !cache_run(&fixture, code, bytes) ||
            fixture.cpu.data.eflags != flags || fixture.writes != 0u)
            return LIB_FALSE;
        for (lib_u8 available = 0u; available < bytes; ++available)
            if (x86_core_cpu_instruction_lexeme_scan(code, available,
                    X86_CORE_CPU_PROFILE_80486SX, code32, &lexeme))
                return LIB_FALSE;
    }
    {
        cpu_instruction_fixture fixture;
        const lib_u8 read_control[] = {0x0fu, 0x24u, 0xe8u};

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        if (!cpu_instruction_expect_real_fault(&fixture, read_control,
                sizeof(read_control), 6u)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

typedef struct cache_fetch_probe {
    cpu_instruction_fixture instruction;
    lib_u32 fail_at;
} cache_fetch_probe;

static lib_status cache_fetch_read(void *opaque, lib_u32 address,
    void *destination, lib_u8 bytes,
    x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    cache_fetch_probe *probe = opaque;

    if (!observe_only && address <= probe->fail_at &&
        (lib_u64)address + bytes > probe->fail_at &&
        (provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH ||
         provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH))
        return LIB_STATUS_IO_ERROR;
    return cpu_instruction_read(&probe->instruction, address, destination,
        bytes, provenance, observe_only, reset_fetch, cache_control);
}

static lib_bool cache_test_observation_and_fetch(void)
{
    static const x86_core_cpu_bus_provider bus = {
        .read_memory = cache_fetch_read, .write_memory = cpu_instruction_write,
        .interrupt_pending = cpu_instruction_interrupt_pending};

    for (lib_u8 reg = 3u; reg < 6u; ++reg)
    for (lib_u8 write = 0u; write < 2u; ++write) {
        const lib_u8 code[] = {0x66u, 0x67u, 0x0fu, write ? 0x26u : 0x24u,
            (lib_u8)(0xc0u | (reg << 3u))};

        if (!write && reg == 5u) continue;
        for (lib_u32 fail_at = 0u; fail_at < sizeof(code); ++fail_at) {
            cache_fetch_probe probe;
            x86_core_cpu_execution_context before;
            x86_core_cpu_instruction_lexeme lexeme;
            t_cpu after, before_cpu;

            cpu_instruction_prepare_with_bus(&probe.instruction,
                X86_CORE_CPU_PROFILE_80486SX, &bus, &probe);
            probe.fail_at = LIB_UINT32_MAX;
            if (!cache_write_register(&probe.instruction, 3u, 0x12345678u) ||
                !cache_write_register(&probe.instruction, 4u, 0xabc00400u) ||
                !cache_write_register(&probe.instruction, 5u, 1u))
                return LIB_FALSE;
            probe.instruction.cpu.data.eip = 0u;
            probe.instruction.cpu.data.eax = 3u;
            x86_core_cpu_execution_invalidate_prefetch(&probe.instruction.execution);
            lib_memory_copy(probe.instruction.memory, code, sizeof(code));
            before = probe.instruction.execution;
            before_cpu = probe.instruction.cpu;
            if (!x86_core_cpu_execution_preview_lexeme(&probe.instruction.execution,
                    &lexeme) || lexeme.byte_count != sizeof(code) ||
                lib_memory_compare(&before, &probe.instruction.execution, sizeof(before)) ||
                lib_memory_compare(&before_cpu, &probe.instruction.cpu, sizeof(before_cpu)))
                return LIB_FALSE;
            probe.fail_at = fail_at;
            (void)cpu_instruction_run(&probe.instruction, code, sizeof(code), &after);
            if (!probe.instruction.fault.valid ||
                probe.instruction.fault.exception_mask != VCPUINS_EXCEPT_CE ||
                lib_memory_compare(&before_cpu, &after, sizeof(after)) ||
                lib_memory_compare(before.cache, probe.instruction.execution.cache,
                    sizeof(before.cache)) ||
                lib_memory_compare(before.cache_lru, probe.instruction.execution.cache_lru,
                    sizeof(before.cache_lru)) ||
                lib_memory_compare(before.cache_fill, probe.instruction.execution.cache_fill,
                    sizeof(before.cache_fill)) ||
                lib_memory_compare(before.cache_read, probe.instruction.execution.cache_read,
                    sizeof(before.cache_read))) return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

typedef struct cache_memory_probe {
    cpu_instruction_fixture instruction;
    lib_u32 completed_fetches;
    lib_u32 completed_fetch_bytes;
    lib_u32 data_reads;
    lib_u32 target_data_reads;
    lib_u32 target_data_writes;
    lib_u32 locked_data_reads;
    lib_u32 locked_data_writes;
    lib_u32 read_bytes;
    lib_u32 fill_probes;
    lib_u8 read_control;
    lib_u8 directory_control;
    lib_u8 table_control;
    lib_u8 directory_write_control;
    lib_u8 table_write_control;
    lib_u32 directory_writes;
    lib_u32 target_table_writes;
    lib_bool fail_page_write;
    lib_bool lock_active;
    lib_bool lock_protocol_error;
    lib_bool fail_lock_begin;
    lib_bool fail_second_read;
    lib_bool fail_table_read;
    lib_bool replace_directory;
    lib_bool replace_table;
    lib_bool replace_split_page;
    lib_u32 second_directory;
    lib_u32 second_table;
    lib_u32 second_split_page;
    lib_u32 lock_address;
    lib_u8 lock_control;
    lib_u32 lock_begins;
    lib_u32 lock_completions;
    lib_u32 lock_cancellations;
    lib_bool fail_read;
    lib_u32 fail_read_at;
    lib_bool fail_write;
    lib_bool fail_fetch;
    lib_bool cache_code;
    lib_bool cache_page_tables;
    lib_u32 fail_fetch_at;
    lib_u8 operation_count;
    lib_u8 fail_operation;
    x86_core_cpu_cache_operation operations[2];
} cache_memory_probe;

static lib_status cache_memory_read(void *opaque, lib_u32 address,
    void *destination, lib_u8 bytes,
    x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    cache_memory_probe *probe = opaque;
    if (!observe_only && probe->fail_fetch && address <= probe->fail_fetch_at &&
        (lib_u64)address + bytes > probe->fail_fetch_at &&
        (provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH ||
         provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH))
        return LIB_STATUS_IO_ERROR;
    if (!observe_only && provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA) {
        probe->read_control = cache_control;
        ++probe->data_reads;
        if (address >= 0x2000u && address < 0x6000u) ++probe->target_data_reads;
        if (probe->lock_active) ++probe->locked_data_reads;
        probe->read_bytes += bytes;
        if (probe->fail_read && (probe->fail_read_at == 0u ||
                (address <= probe->fail_read_at &&
                 (lib_u64)address + bytes > probe->fail_read_at)))
            return LIB_STATUS_IO_ERROR;
    }
    if (!observe_only && provenance == X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ) {
        if (address == 0x11010u && probe->fail_table_read) return LIB_STATUS_IO_ERROR;
        if (probe->lock_active) {
            probe->lock_address = address;
            probe->lock_control = cache_control;
            if (probe->fail_second_read) return LIB_STATUS_IO_ERROR;
            if (address == 0x10000u && probe->replace_directory)
                lib_memory_copy(probe->instruction.memory + address,
                    &probe->second_directory, sizeof(probe->second_directory));
            if (address == 0x11010u && probe->replace_table)
                lib_memory_copy(probe->instruction.memory + address,
                    &probe->second_table, sizeof(probe->second_table));
            if (address == 0x11014u && probe->replace_split_page)
                lib_memory_copy(probe->instruction.memory + address,
                    &probe->second_split_page, sizeof(probe->second_split_page));
        }
        if (address == 0x10000u) probe->directory_control = cache_control;
        else probe->table_control = cache_control;
    }
    const lib_status status = cpu_instruction_read(&probe->instruction, address, destination,
        bytes, provenance, observe_only, reset_fetch, cache_control);
    if (status == LIB_STATUS_OK && !observe_only &&
        (provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH ||
         provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH)) {
        ++probe->completed_fetches;
        probe->completed_fetch_bytes += bytes;
    }
    return status;
}

static lib_status cache_memory_write(void *opaque, lib_u32 address,
    const void *source, lib_u8 bytes,
    x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    cache_memory_probe *probe = opaque;
    if (provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA &&
        address >= 0x2000u && address < 0x6000u) {
        ++probe->target_data_writes;
        if (probe->lock_active) ++probe->locked_data_writes;
    }
    if (provenance == X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE) {
        if (probe->instruction.execution.cpu_profile == X86_CORE_CPU_PROFILE_80486SX &&
            (!probe->lock_active || address != probe->lock_address ||
             cache_control != probe->lock_control)) {
            probe->lock_protocol_error = LIB_TRUE;
            return LIB_STATUS_INVALID_STATE;
        }
        if (address == 0x10000u) {
            probe->directory_write_control = cache_control;
            ++probe->directory_writes;
        }
        else probe->table_write_control = cache_control;
        if (address == 0x11010u) ++probe->target_table_writes;
        if (probe->fail_page_write) return LIB_STATUS_IO_ERROR;
    }
    if (probe->fail_write) return LIB_STATUS_IO_ERROR;
    return cpu_instruction_write(&probe->instruction, address, source, bytes, provenance, cache_control);
}

static lib_status cache_memory_query(void *opaque, lib_u32 address,
    x86_core_cpu_cache_query query, x86_core_cpu_cache_memory *out_memory)
{
    cache_memory_probe *probe = opaque;
    if (query == X86_CORE_CPU_CACHE_FILL_PROBE &&
        address >= 0x2000u && address < 0x4000u) ++probe->fill_probes;
    *out_memory = (x86_core_cpu_cache_memory){.address = address,
        .fill_enabled = query == X86_CORE_CPU_CACHE_FILL_PROBE &&
            ((address >= 0x2000u && address < 0x4000u) ||
             (probe->cache_code && address < 0x1000u) ||
             (probe->cache_page_tables && address >= 0x10000u && address < 0x12000u))};
    return LIB_STATUS_OK;
}

static lib_status cache_memory_operation(void *opaque,
    x86_core_cpu_cache_operation operation)
{
    cache_memory_probe *probe = opaque;
    if (probe->operation_count >= 2u) return LIB_STATUS_LIMIT_EXCEEDED;
    probe->operations[probe->operation_count++] = operation;
    return probe->fail_operation == probe->operation_count ?
        LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}

static lib_status cache_memory_begin_memory_lock(void *opaque)
{
    cache_memory_probe *probe = opaque;
    if (probe->lock_active) {
        probe->lock_protocol_error = LIB_TRUE;
        return LIB_STATUS_INVALID_STATE;
    }
    if (probe->fail_lock_begin) return LIB_STATUS_IO_ERROR;
    probe->lock_active = LIB_TRUE;
    ++probe->lock_begins;
    return LIB_STATUS_OK;
}

static void cache_memory_complete_memory_lock(void *opaque, lib_bool commit)
{
    cache_memory_probe *probe = opaque;
    if (!probe->lock_active) probe->lock_protocol_error = LIB_TRUE;
    probe->lock_active = LIB_FALSE;
    ++probe->lock_completions;
    if (!commit) ++probe->lock_cancellations;
}

static const x86_core_cpu_bus_provider cache_memory_bus = {
    .read_memory = cache_memory_read, .write_memory = cache_memory_write,
    .interrupt_pending = cpu_instruction_interrupt_pending,
    .query_cache_memory = cache_memory_query, .cache_operation = cache_memory_operation,
    .begin_memory_lock = cache_memory_begin_memory_lock,
    .complete_memory_lock = cache_memory_complete_memory_lock};

static lib_bool cache_replacement_matrix(void)
{
    static const lib_u8 victims[] = {0u, 2u, 1u, 2u, 0u, 3u, 1u, 3u};
    static const lib_u8 touched[8][4] = {
        {3u, 1u, 4u, 0u}, {3u, 1u, 4u, 0u},
        {3u, 1u, 6u, 2u}, {3u, 1u, 6u, 2u},
        {7u, 5u, 4u, 0u}, {7u, 5u, 4u, 0u},
        {7u, 5u, 6u, 2u}, {7u, 5u, 6u, 2u}};
    const lib_u8 read[] = {0xa1u, 0u, 0x20u};

    for (lib_u8 lru = 0u; lru < 8u; ++lru)
    for (lib_u8 valid = 0u; valid < 16u; ++valid) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        lib_u8 victim = victims[lru];

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        fixture->execution.cache_lru[0] = lru;
        for (lib_u8 way = 0u; way < 4u; ++way) {
            fixture->execution.cache[0][way].tag = 0x10000000u | ((lib_u32)way << 20u);
            fixture->execution.cache[0][way].valid = (valid & (1u << way)) != 0u;
        }
        for (lib_u8 way = 0u; way < 4u; ++way)
            if (!(valid & (1u << way))) { victim = way; break; }
        if (!cache_run(fixture, read, sizeof(read)) || probe.data_reads != 1u ||
            probe.read_bytes != 16u || probe.fill_probes != 1u ||
            fixture->execution.cache_lru[0] != touched[lru][victim])
            return LIB_FALSE;
        for (lib_u8 way = 0u; way < 4u; ++way)
            if (fixture->execution.cache[0][way].tag != (way == victim ? 0x2000u :
                    (0x10000000u | ((lib_u32)way << 20u))) ||
                fixture->execution.cache[0][way].valid !=
                    (way == victim || (valid & (1u << way)) != 0u))
                return LIB_FALSE;
    }
    for (lib_u8 lru = 0u; lru < 8u; ++lru)
    for (lib_u8 hit = 0u; hit < 4u; ++hit)
    for (lib_u8 write = 0u; write < 2u; ++write) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u16 address = (lib_u16)(0x2000u + ((lib_u32)hit << 11u));
        const lib_u8 hit_access[] = {write ? 0xa3u : 0xa1u,
            (lib_u8)address, (lib_u8)(address >> 8u)};

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        fixture->cpu.data.ax = 0x5678u;
        fixture->execution.cache_lru[0] = lru;
        for (lib_u8 way = 0u; way < 4u; ++way) {
            fixture->execution.cache[0][way].tag = 0x2000u + ((lib_u32)way << 11u);
            fixture->execution.cache[0][way].valid = LIB_TRUE;
            fixture->execution.cache[0][way].data[0] = 0x1230u + way;
        }
        const lib_u32 expected = write ? 0x5678u : 0x1230u + hit;
        if (!cache_run(fixture, hit_access, sizeof(hit_access)) || probe.data_reads != 0u ||
            probe.fill_probes != 0u || fixture->cpu.data.ax != expected ||
            fixture->execution.cache[0][hit].data[0] != expected || fixture->writes != write ||
            fixture->execution.cache_lru[0] != touched[lru][hit]) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_normal_access(void)
{
    cache_memory_probe probe = {0};
    cpu_instruction_fixture *fixture = &probe.instruction;
    const lib_u8 read[] = {0xa1u, 0u, 0x20u};
    const lib_u8 write[] = {0xa3u, 0u, 0x20u};
    lib_u16 value = 0x1234u;

    cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
        &cache_memory_bus, &probe);
    fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
    lib_memory_copy(fixture->memory + 0x2000u, &value, sizeof(value));
    if (!cache_run(fixture, read, sizeof(read)) || fixture->cpu.data.ax != value ||
        probe.data_reads != 1u || probe.read_bytes != 16u || probe.fill_probes != 1u)
        return LIB_FALSE;
    value = 0x5678u;
    lib_memory_copy(fixture->memory + 0x2000u, &value, sizeof(value));
    if (!cache_run(fixture, read, sizeof(read)) || fixture->cpu.data.ax != 0x1234u ||
        probe.data_reads != 1u || probe.fill_probes != 1u) return LIB_FALSE;
    fixture->cpu.data.ax = 0x4321u;
    if (!cache_run(fixture, write, sizeof(write)) || fixture->writes != 1u)
        return LIB_FALSE;
    lib_memory_copy(&value, fixture->memory + 0x2000u, sizeof(value));
    if (value != 0x4321u || !cache_run(fixture, read, sizeof(read)) ||
        fixture->cpu.data.ax != value || probe.data_reads != 1u || probe.fill_probes != 1u)
        return LIB_FALSE;
    fixture->cpu.data.cr0 |= VCPU_CR0_CD;
    if (!cache_run(fixture, read, sizeof(read)) || probe.data_reads != 1u ||
        probe.fill_probes != 1u)
        return LIB_FALSE;
    fixture->cpu.data.cr0 |= VCPU_CR0_NW;
    fixture->cpu.data.ax = 0xabcdu;
    if (!cache_run(fixture, write, sizeof(write)) || fixture->writes != 1u ||
        !cache_run(fixture, read, sizeof(read)) || fixture->cpu.data.ax != 0xabcdu)
        return LIB_FALSE;
    lib_memory_copy(&value, fixture->memory + 0x2000u, sizeof(value));
    if (value != 0x4321u || !cache_run(fixture,
            (const lib_u8[]){0xa3u, 0x20u, 0x20u}, 3u) || fixture->writes != 2u ||
        probe.fill_probes != 1u)
        return LIB_FALSE;
    /* The normal line is readable through TR5/TR3, not a separate test cache. */
    if (!cache_write_register(fixture, 5u, 2u) ||
        !cache_write_register(fixture, 5u, 0u)) return LIB_FALSE;
    lib_u32 data;
    if (!cache_read_register(fixture, 3u, &data) || (data & 0xffffu) != 0xabcdu)
        return LIB_FALSE;
    return LIB_TRUE;
}

static lib_bool cache_locked_operands(void)
{
    static const lib_u8 widths[] = {1u, 2u, 4u};
    static const lib_u32 masks[] = {0xffu, 0xffffu, LIB_UINT32_MAX};
    for (lib_u8 exchange = 0u; exchange < 2u; ++exchange)
    for (lib_u8 prefix = 0u; prefix < 2u; ++prefix)
    for (lib_u8 size = 0u; size < 3u; ++size)
    for (lib_u8 mode = 0u; mode < 3u; ++mode) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_bool locked = exchange || prefix;
        const lib_bool bus_write = locked || mode != 2u;
        const lib_u32 ram = 0x11111111u, cached = 0x22222222u, input = 0x33333333u;
        const lib_u32 mask = masks[size];
        const lib_u32 read_value = locked ? ram : cached;
        const lib_u32 stored = exchange ? input : (read_value + input) & mask;
        lib_u8 code[6], bytes = 0u;
        lib_u32 observed;
        lib_u64 decode_ticks = LIB_UINT64_MAX;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        if (mode) fixture->cpu.data.cr0 |= VCPU_CR0_CD;
        if (mode == 2u) fixture->cpu.data.cr0 |= VCPU_CR0_NW;
        fixture->cpu.data.eax = input;
        fixture->execution.cache[0][0] = (cpu_cache_line){
            .tag = 0x2000u, .valid = LIB_TRUE, .data = {cached}};
        lib_memory_copy(fixture->memory + 0x2000u, &ram, sizeof(ram));
        if (prefix) code[bytes++] = 0xf0u;
        if (widths[size] == 4u) code[bytes++] = 0x66u;
        code[bytes++] = exchange ? (size ? 0x87u : 0x86u) : (size ? 0x01u : 0x00u);
        code[bytes++] = 0x06u;
        code[bytes++] = 0u;
        code[bytes++] = 0x20u;
        if (!cache_run(fixture, code, bytes) ||
            fixture->cpu.data.eax != (exchange ? ((input & ~mask) | (ram & mask)) : input) ||
            fixture->execution.cache[0][0].data[0] != ((cached & ~mask) | (stored & mask)) ||
            probe.data_reads != (lib_u32)locked || probe.locked_data_reads != (lib_u32)locked ||
            probe.target_data_writes != (lib_u32)bus_write || probe.locked_data_writes != (lib_u32)locked ||
            probe.lock_begins != (lib_u32)locked || probe.lock_completions != probe.lock_begins ||
            probe.lock_cancellations != 0u || probe.lock_active || probe.lock_protocol_error ||
            fixture->instructions.data.memory_lock_active ||
            fixture->instructions.data.flagLock != prefix || probe.fill_probes != 0u ||
            !x86_core_cpu_80486_decode_penalty(&fixture->execution, &decode_ticks) ||
            decode_ticks != (lib_u64)prefix + (size == 2u))
            return LIB_FALSE;
        lib_memory_copy(&observed, fixture->memory + 0x2000u, sizeof(observed));
        if (observed != (bus_write ? ((ram & ~mask) | (stored & mask)) : ram))
            return LIB_FALSE;
    }
    for (lib_u8 stage = 0u; stage < 3u; ++stage) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u8 code[] = {0x87u, 0x06u, 0u, 0x20u};

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        probe.fail_lock_begin = stage == 0u;
        probe.fail_read = stage == 1u;
        probe.fail_write = stage == 2u;
        if (cache_run(fixture, code, sizeof(code)) || !fixture->fault.valid ||
            fixture->fault.exception_mask != VCPUINS_EXCEPT_CE ||
            fixture->cpu.data.eip != 0u || fixture->instructions.data.memory_lock_active ||
            probe.lock_active || probe.lock_protocol_error ||
            probe.lock_begins != (lib_u32)(stage != 0u) || probe.lock_completions != probe.lock_begins ||
            probe.lock_cancellations != probe.lock_begins) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_page_attributes(void)
{
    for (lib_u8 control = 0u; control < 64u; ++control) {
        const lib_bool pcd = (control & 1u) != 0u;
        const lib_u8 data_control = (pcd ? 0x10u : 0u) | (control & 2u ? 8u : 0u);
        const lib_u8 directory_control = (control & 4u ? 0x10u : 0u) | (control & 8u ? 8u : 0u);
        const lib_u8 table_control = (control & 16u ? 0x10u : 0u) | (control & 32u ? 8u : 0u);
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u8 read[] = {0xa1u, 0u, 0x40u};
        lib_u32 entry = 0x11003u | table_control;
        lib_u16 value = 0x1234u;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 = VCPU_CR0_PE | VCPU_CR0_PG;
        fixture->cpu.data.cr3 = 0x10000u | directory_control;
        fixture->cpu.data.ds.selector = 0x10u;
        lib_memory_copy(fixture->memory + 0x10000u, &entry, sizeof(entry));
        entry = 0x23u;
        lib_memory_copy(fixture->memory + 0x11000u, &entry, sizeof(entry));
        entry = 0x2003u | data_control;
        lib_memory_copy(fixture->memory + 0x11010u, &entry, sizeof(entry));
        lib_memory_copy(fixture->memory + 0x2000u, &value, sizeof(value));
        if (!cache_run(fixture, read, sizeof(read)) || fixture->cpu.data.ax != value ||
            probe.data_reads != 1u || probe.read_bytes != (pcd ? 2u : 16u) ||
            probe.read_control != data_control || probe.directory_control != directory_control ||
            probe.table_control != table_control ||
            probe.directory_write_control != directory_control ||
            probe.table_write_control != table_control) {
            lib_c_printf("PCD %u first: ax=%x reads=%u bytes=%u fault=%x delivered=%x\n",
                (lib_u32)pcd, (lib_u32)fixture->cpu.data.ax, probe.data_reads,
                probe.read_bytes, fixture->fault.exception_mask,
                fixture->delivered_exception.exception_mask);
            return LIB_FALSE;
        }
        value = 0x5678u;
        lib_memory_copy(fixture->memory + 0x2000u, &value, sizeof(value));
        if (!cache_run(fixture, read, sizeof(read)) ||
            fixture->cpu.data.ax != (pcd ? value : 0x1234u) ||
            probe.data_reads != (pcd ? 2u : 1u)) {
            lib_c_printf("PCD %u second: ax=%x reads=%u bytes=%u\n", (lib_u32)pcd,
                (lib_u32)fixture->cpu.data.ax, probe.data_reads, probe.read_bytes);
            return LIB_FALSE;
        }
    }
    {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        lib_u32 entry = 0x11023u;
        const lib_u8 read[] = {0xa1u, 0xffu, 0x4fu};

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 = VCPU_CR0_PE | VCPU_CR0_PG;
        fixture->cpu.data.cr3 = 0x10000u;
        fixture->cpu.data.ds.selector = 0x10u;
        lib_memory_copy(fixture->memory + 0x10000u, &entry, sizeof(entry));
        entry = 0x23u;
        lib_memory_copy(fixture->memory + 0x11000u, &entry, sizeof(entry));
        entry = 0x2023u;
        lib_memory_copy(fixture->memory + 0x11010u, &entry, sizeof(entry));
        entry = 0x3033u;
        lib_memory_copy(fixture->memory + 0x11014u, &entry, sizeof(entry));
        fixture->memory[0x2fffu] = 0x11u;
        fixture->memory[0x3000u] = 0x22u;
        if (!cache_run(fixture, read, sizeof(read)) || fixture->cpu.data.ax != 0x2211u ||
            probe.data_reads != 2u || probe.read_bytes != 17u) return LIB_FALSE;
        fixture->memory[0x2fffu] = 0x33u;
        fixture->memory[0x3000u] = 0x44u;
        if (!cache_run(fixture, read, sizeof(read)) || fixture->cpu.data.ax != 0x4411u ||
            probe.data_reads != 3u || probe.read_bytes != 18u) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_page_second_values(void)
{
    for (lib_u8 directory = 0u; directory < 2u; ++directory)
    for (lib_u8 write = 0u; write < 2u; ++write) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u8 code[] = {write ? 0xa3u : 0xa1u, 0u, 0x40u};
        const lib_u16 old_value = 0x1234u, new_value = 0x5678u, stored = 0x7788u;
        lib_u16 copied;
        lib_u32 entry = directory ? 0x11003u : 0x11023u;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 = VCPU_CR0_PE | VCPU_CR0_PG;
        fixture->cpu.data.cr3 = 0x10000u;
        fixture->cpu.data.ds.selector = 0x10u;
        fixture->cpu.data.ax = stored;
        lib_memory_copy(fixture->memory + 0x10000u, &entry, sizeof(entry));
        entry = 0x23u;
        lib_memory_copy(fixture->memory + 0x11000u, &entry, sizeof(entry));
        lib_memory_copy(fixture->memory + 0x12000u, &entry, sizeof(entry));
        entry = 0x2003u;
        lib_memory_copy(fixture->memory + 0x11010u, &entry, sizeof(entry));
        entry = 0x3033u;
        lib_memory_copy(fixture->memory + 0x12010u, &entry, sizeof(entry));
        lib_memory_copy(fixture->memory + 0x2000u, &old_value, sizeof(old_value));
        lib_memory_copy(fixture->memory + 0x3000u, &new_value, sizeof(new_value));
        probe.replace_directory = directory != 0u;
        probe.second_directory = 0x1201bu;
        probe.replace_table = !directory;
        probe.second_table = 0x3013u;
        if (!cache_run(fixture, code, sizeof(code)) ||
            (!write && fixture->cpu.data.ax != new_value) ||
            probe.lock_active || probe.lock_protocol_error ||
            probe.lock_begins == 0u || probe.lock_completions != probe.lock_begins ||
            probe.lock_cancellations != 0u) return LIB_FALSE;
        lib_memory_copy(&copied, fixture->memory + 0x2000u, sizeof(copied));
        if (copied != old_value) return LIB_FALSE;
        lib_memory_copy(&copied, fixture->memory + 0x3000u, sizeof(copied));
        if (copied != (write ? stored : new_value) ||
            (!write && probe.read_control != X86_CORE_CPU_MEMORY_CACHE_PCD))
            return LIB_FALSE;
        lib_memory_copy(&entry, fixture->memory + (directory ? 0x10000u : 0x11010u), sizeof(entry));
        if (entry != (directory ? 0x1203bu : (write ? 0x3073u : 0x3033u)))
            return LIB_FALSE;
    }
    for (lib_u8 stage = 0u; stage < 3u; ++stage) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u8 read[] = {0xa1u, 0u, 0x40u};
        lib_u32 entry = 0x11023u;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 = VCPU_CR0_PE | VCPU_CR0_PG;
        fixture->cpu.data.cr3 = 0x10000u;
        fixture->cpu.data.ds.selector = 0x10u;
        lib_memory_copy(fixture->memory + 0x10000u, &entry, sizeof(entry));
        entry = 0x23u;
        lib_memory_copy(fixture->memory + 0x11000u, &entry, sizeof(entry));
        entry = 0x2003u;
        lib_memory_copy(fixture->memory + 0x11010u, &entry, sizeof(entry));
        probe.fail_lock_begin = stage == 0u;
        probe.fail_second_read = stage == 1u;
        probe.fail_page_write = stage == 2u;
        if (cache_run(fixture, read, sizeof(read)) || !fixture->fault.valid ||
            fixture->fault.exception_mask != VCPUINS_EXCEPT_CE ||
            probe.lock_active || probe.lock_protocol_error || probe.data_reads != 0u ||
            probe.lock_begins != (lib_u32)(stage != 0u) ||
            probe.lock_completions != probe.lock_begins ||
            probe.lock_cancellations != probe.lock_begins) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_page_second_permissions(void)
{
    static const lib_u32 entries[] = {0x3006u, 0x3003u, 0x3005u};
    for (lib_u8 directory = 0u; directory < 2u; ++directory)
    for (lib_u8 mode = 0u; mode < 5u; ++mode)
    for (lib_u8 wp = 0u; wp < 2u; ++wp)
    for (lib_u8 kind = 0u; kind < 3u; ++kind)
    for (lib_u8 write = 0u; write < 2u; ++write) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u8 cpl = mode == 4u ? 3u : mode;
        const lib_bool user = cpl == 3u;
        const lib_bool fault = kind == 0u || (kind == 1u && user) ||
            (kind == 2u && write && (user || wp));
        const lib_bool fetch_fault = directory && (kind == 0u || (kind == 1u && user));
        const lib_u32 error = (kind != 0u ? 1u : 0u) |
            (write && !fetch_fault ? 2u : 0u) | (user ? 4u : 0u);
        const lib_u16 stored = 0x5678u, initial = 0x1234u;
        const lib_u8 code[] = {write ? 0xa3u : 0xa1u, 0u, 0x40u};
        lib_u32 entry = directory ? 0x11007u : 0x11027u, frame[4];
        lib_u16 copied;
        t_cpu after;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        cpu_instruction_prepare_protected_exception(fixture, mode);
        fixture->cpu.data.cr0 |= VCPU_CR0_PG | VCPU_CR0_CD | (wp ? VCPU_CR0_WP : 0u);
        fixture->cpu.data.cr3 = 0x10000u;
        fixture->cpu.data.ds.selector = (lib_u16)(0x10u | cpl);
        fixture->cpu.data.ds.dpl = cpl;
        fixture->cpu.data.ax = stored;
        lib_memory_copy(fixture->memory + 0x470u, fixture->memory + 0x468u, 8u);
        lib_memory_copy(fixture->memory + 0x10000u, &entry, sizeof(entry));
        entry = 0x27u;
        lib_memory_copy(fixture->memory + 0x11000u, &entry, sizeof(entry));
        entry = 0x2007u;
        lib_memory_copy(fixture->memory + 0x11010u, &entry, sizeof(entry));
        if (directory) {
            /* Keep exception tables, handler and stack in a different PDE.
             * The current cached CS remains the original guest mapping. */
            fixture->cpu.data.gdtr.base += 0x400000u;
            fixture->cpu.data.idtr.base += 0x400000u;
            fixture->memory[0x20cu] = 0x40u;
            fixture->memory[0x214u] = 0x40u;
            if (mode == 4u) fixture->cpu.data.tr.base += 0x400000u;
            else fixture->cpu.data.ss.base = 0x400000u;
            entry = 0x12027u;
            lib_memory_copy(fixture->memory + 0x10004u, &entry, sizeof(entry));
            entry = 0x27u;
            lib_memory_copy(fixture->memory + 0x12000u, &entry, sizeof(entry));
        }
        probe.replace_directory = directory != 0u;
        probe.second_directory = 0x11000u | (entries[kind] & 0xfffu);
        probe.replace_table = !directory;
        probe.second_table = entries[kind];
        lib_memory_copy(fixture->memory + (directory ? 0x2000u : 0x3000u), &initial, sizeof(initial));
        if (cpu_instruction_run(fixture, code, sizeof(code), &after) != LIB_STATUS_OK ||
            fixture->fault.valid || fixture->delivered_exception.valid != fault ||
            (fault && (fixture->delivered_exception.exception_mask != VCPUINS_EXCEPT_PF ||
                fixture->delivered_exception.exception_code != error ||
                fixture->delivered_exception.point.eip != 0u || after.data.cr2 != (fetch_fault ? 0u : 0x4000u) ||
                after.data.eip != 0x300u || after.data.esp != (mode == 4u ? 0xfd8u : 0x7f0u))) ||
            (!fault && (after.data.eip != sizeof(code) || (!write && after.data.ax != initial))) ||
            probe.lock_active || probe.lock_protocol_error ||
            probe.lock_begins != (directory || fault ? 2u : 1u) || probe.lock_completions != probe.lock_begins ||
            probe.lock_cancellations != (lib_u32)(directory ? kind == 0u : fault) ||
            probe.target_table_writes != (lib_u32)!fault ||
            probe.target_data_reads != (lib_u32)(!fault && !write) ||
            probe.target_data_writes != (lib_u32)(!fault && write)) {
            lib_c_printf("Second PF %u/%u/%u/%u/%u: fault=%x delivered=%x error=%x cr2=%x eip=%x esp=%x begins=%u ends=%u cancel=%u reads=%u writes=%u\n",
                (lib_u32)directory, (lib_u32)mode, (lib_u32)wp, (lib_u32)kind, (lib_u32)write, fixture->fault.exception_mask,
                fixture->delivered_exception.exception_mask,
                fixture->delivered_exception.exception_code, after.data.cr2,
                after.data.eip, after.data.esp, probe.lock_begins,
                probe.lock_completions, probe.lock_cancellations,
                probe.data_reads, probe.target_table_writes);
            return LIB_FALSE;
        }
        lib_memory_copy(&entry, fixture->memory + 0x11010u, sizeof(entry));
        const lib_u32 expected_pte = directory ? 0x2007u : probe.second_table;
        if (entry != (fault ? expected_pte : (expected_pte | (write ? 0x60u : 0x20u))))
            return LIB_FALSE;
        if (directory) {
            lib_memory_copy(&entry, fixture->memory + 0x10000u, sizeof(entry));
            if (entry != (probe.second_directory | (kind == 0u ? 0u : 0x20u)))
                return LIB_FALSE;
        }
        if (fault) {
            lib_memory_copy(frame, fixture->memory + after.data.esp, sizeof(frame));
            if (frame[0] != error || frame[1] != 0u || frame[2] != (lib_u32)(8u | cpl))
                return LIB_FALSE;
        }
        lib_memory_copy(&copied, fixture->memory + (directory ? 0x2000u : 0x3000u), sizeof(copied));
        if (copied != (!fault && write ? stored : initial)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_page_split_redirect(void)
{
    for (lib_u8 fault = 0u; fault < 2u; ++fault)
    for (lib_u8 write = 0u; write < 2u; ++write) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u8 code[] = {write ? 0xa3u : 0xa1u, 0xffu, 0x4fu};
        lib_u32 entry = 0x11023u;
        t_cpu after;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        cpu_instruction_prepare_protected_exception(fixture, 0u);
        fixture->cpu.data.cr0 |= VCPU_CR0_PG | VCPU_CR0_WP | VCPU_CR0_CD;
        fixture->cpu.data.cr3 = 0x10000u;
        fixture->cpu.data.ds.selector = 0x10u;
        fixture->cpu.data.ax = 0x6655u;
        lib_memory_copy(fixture->memory + 0x470u, fixture->memory + 0x468u, 8u);
        lib_memory_copy(fixture->memory + 0x10000u, &entry, sizeof(entry));
        entry = 0x23u;
        lib_memory_copy(fixture->memory + 0x11000u, &entry, sizeof(entry));
        entry = 0x2003u;
        lib_memory_copy(fixture->memory + 0x11010u, &entry, sizeof(entry));
        entry = 0x3003u;
        lib_memory_copy(fixture->memory + 0x11014u, &entry, sizeof(entry));
        fixture->memory[0x2fffu] = 0x11u;
        fixture->memory[0x3000u] = 0x22u;
        fixture->memory[0x4fffu] = 0x33u;
        fixture->memory[0x5000u] = 0x44u;
        probe.replace_table = probe.replace_split_page = LIB_TRUE;
        probe.second_table = 0x4003u;
        probe.second_split_page = fault ? (write ? 0x5001u : 0x5002u) : 0x5003u;
        if (cpu_instruction_run(fixture, code, sizeof(code), &after) != LIB_STATUS_OK ||
            fixture->fault.valid || fixture->delivered_exception.valid != (lib_bool)fault ||
            (!fault && (after.data.eip != sizeof(code) || (!write && after.data.ax != 0x4433u))) ||
            (fault && (fixture->delivered_exception.exception_mask != VCPUINS_EXCEPT_PF ||
                fixture->delivered_exception.exception_code != (write ? 3u : 0u) ||
                fixture->delivered_exception.point.eip != 0u || after.data.cr2 != 0x5000u ||
                after.data.eip != 0x300u || after.data.esp != 0x7f0u)) ||
            probe.lock_active || probe.lock_protocol_error ||
            probe.lock_begins != (fault ? 3u : 2u) || probe.lock_completions != probe.lock_begins ||
            probe.lock_cancellations != fault ||
            probe.target_data_reads != (!fault && !write ? 2u : 0u) ||
            probe.target_data_writes != (!fault && write ? 2u : 0u)) return LIB_FALSE;
        if (fixture->memory[0x2fffu] != 0x11u || fixture->memory[0x3000u] != 0x22u ||
            fixture->memory[0x4fffu] != (!fault && write ? 0x55u : 0x33u) ||
            fixture->memory[0x5000u] != (!fault && write ? 0x66u : 0x44u)) return LIB_FALSE;
        lib_memory_copy(&entry, fixture->memory + 0x11010u, sizeof(entry));
        if (entry != (write ? 0x4063u : 0x4023u)) return LIB_FALSE;
        lib_memory_copy(&entry, fixture->memory + 0x11014u, sizeof(entry));
        if (entry != (fault ? probe.second_split_page : (write ? 0x5063u : 0x5023u)))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_page_update_masks(void)
{
    for (lib_u8 legacy = 0u; legacy < 2u; ++legacy)
    for (lib_u8 bits = 0u; bits < 8u; ++bits)
    for (lib_u8 write = 0u; write < 2u; ++write) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u8 code[] = {write ? 0xa3u : 0xa1u, 0u, 0x40u};
        const lib_u32 required = write ? 0x60u : 0x20u;
        const lib_u32 initial = 0x2003u | ((bits & 2u) ? 0x20u : 0u) |
            ((bits & 4u) ? 0x40u : 0u);
        lib_u32 entry = 0x11003u | ((bits & 1u) ? 0x20u : 0u);
        const lib_u16 value = 0x1234u;

        cpu_instruction_prepare_with_bus(fixture, legacy ?
            X86_CORE_CPU_PROFILE_80386 : X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 = VCPU_CR0_PE | VCPU_CR0_PG | VCPU_CR0_CD;
        fixture->cpu.data.cr3 = 0x10000u;
        fixture->cpu.data.ds.selector = 0x10u;
        fixture->cpu.data.ax = value;
        lib_memory_copy(fixture->memory + 0x10000u, &entry, sizeof(entry));
        entry = 0x23u;
        lib_memory_copy(fixture->memory + 0x11000u, &entry, sizeof(entry));
        lib_memory_copy(fixture->memory + 0x11010u, &initial, sizeof(initial));
        lib_memory_copy(fixture->memory + 0x2000u, &value, sizeof(value));
        probe.fail_page_write = !legacy && (bits & 1u) != 0u &&
            (initial & required) == required;
        if (!cache_run(fixture, code, sizeof(code)) ||
            (!write && fixture->cpu.data.ax != value) ||
            probe.target_table_writes !=
                (lib_u32)(legacy || (initial & required) != required) ||
            (!legacy && probe.directory_writes != (lib_u32)((bits & 1u) == 0u)) ||
            (legacy && probe.directory_writes < 2u)) return LIB_FALSE;
        lib_memory_copy(&entry, fixture->memory + 0x11010u, sizeof(entry));
        if (entry != (initial | required)) return LIB_FALSE;
    }
    for (lib_u8 write = 0u; write < 2u; ++write) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u8 code[] = {write ? 0xa3u : 0xa1u, 0u, 0x40u};
        lib_u32 entry = 0x11023u;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 = VCPU_CR0_PE | VCPU_CR0_PG | VCPU_CR0_CD;
        fixture->cpu.data.cr3 = 0x10000u;
        fixture->cpu.data.ds.selector = 0x10u;
        lib_memory_copy(fixture->memory + 0x10000u, &entry, sizeof(entry));
        entry = 0x23u;
        lib_memory_copy(fixture->memory + 0x11000u, &entry, sizeof(entry));
        entry = 0x2003u;
        lib_memory_copy(fixture->memory + 0x11010u, &entry, sizeof(entry));
        probe.fail_page_write = LIB_TRUE;
        if (cache_run(fixture, code, sizeof(code)) || !fixture->fault.valid ||
            fixture->fault.exception_mask != VCPUINS_EXCEPT_CE ||
            probe.target_table_writes != 1u || probe.data_reads != 0u ||
            fixture->cpu.data.eip != 0u) return LIB_FALSE;
        lib_memory_copy(&entry, fixture->memory + 0x11010u, sizeof(entry));
        if (entry != 0x2003u) return LIB_FALSE;
        for (lib_u8 way = 0u; way < X86_CPU_TLB_WAYS; ++way)
            if (fixture->execution.tlb[4][way].valid) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_failed_publication(void)
{
    for (lib_u8 write = 0u; write < 2u; ++write) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u8 code[] = {write ? 0xa3u : 0xa1u, 0u, 0x20u};
        cpu_cache_line before;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        if (write && !cache_run(fixture, (const lib_u8[]){0xa1u, 0u, 0x20u}, 3u))
            return LIB_FALSE;
        before = fixture->execution.cache[0][0];
        if (write) before.valid = LIB_FALSE;
        probe.fail_read = !write;
        probe.fail_write = write != 0u;
        fixture->cpu.data.ax = 0x1234u;
        if (cache_run(fixture, code, sizeof(code)) || !fixture->fault.valid ||
            fixture->fault.exception_mask != VCPUINS_EXCEPT_CE ||
            fixture->instructions.data.timing_data_misses != 0u ||
            fixture->instructions.data.timing_unaligned_accesses != 0u ||
            lib_memory_compare(&before, &fixture->execution.cache[0][0], sizeof(before)) ||
            fixture->memory[0x2000u] != 0u || fixture->memory[0x2001u] != 0u)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_snoop_write(void)
{
    for (lib_u8 nw = 0u; nw < 2u; ++nw) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        if (!cache_run(fixture, (const lib_u8[]){0xa1u, 0u, 0x20u}, 3u) ||
            !cache_run(fixture, (const lib_u8[]){0xa1u, 0x10u, 0x20u}, 3u) ||
            !cache_run(fixture, (const lib_u8[]){0xa1u, 0u, 0x30u}, 3u))
            return LIB_FALSE;
        if (nw) fixture->cpu.data.cr0 |= VCPU_CR0_CD | VCPU_CR0_NW;
        x86_core_cpu_signal_cache_write(&fixture->execution, 0x200fu, 2u);
        if (fixture->execution.cache[0][0].valid != (nw != 0u) ||
            fixture->execution.cache[1][0].valid != (nw != 0u) ||
            !fixture->execution.cache[0][1].valid) return LIB_FALSE;
        x86_core_cpu_signal_cache_write(&fixture->execution, 0u, 0x4000u);
        if (fixture->execution.cache[0][1].valid != (nw != 0u)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_reset_and_snoop_bounds(void)
{
    cpu_instruction_fixture fixture;

    cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
    fixture.cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
    fixture.execution.cache[0][0] = (cpu_cache_line){
        .tag = 0u, .valid = LIB_TRUE, .data = {0x1234u}};
    fixture.execution.cache[127][0] = (cpu_cache_line){
        .tag = 0xfffff800u, .valid = LIB_TRUE, .data = {0x5678u}};
    fixture.execution.cache_lru[127] = 7u;
    x86_core_cpu_signal_cache_write(&fixture.execution, 0xfffffff8u, 0u);
    if (!fixture.execution.cache[127][0].valid) return LIB_FALSE;
    x86_core_cpu_signal_cache_write(&fixture.execution, 0xfffffff8u,
        (lib_size)LIB_UINT64_MAX);
    if (fixture.execution.cache[127][0].valid ||
        !fixture.execution.cache[0][0].valid ||
        fixture.execution.cache[127][0].tag != 0xfffff800u ||
        fixture.execution.cache[127][0].data[0] != 0x5678u ||
        fixture.execution.cache_lru[127] != 7u) return LIB_FALSE;
    x86_core_cpu_signal_cache_write(&fixture.execution, 0u,
        (lib_size)LIB_UINT64_MAX);
    if (fixture.execution.cache[0][0].valid) return LIB_FALSE;

    lib_memory_set(fixture.execution.cache, 0xffu, sizeof(fixture.execution.cache));
    lib_memory_set(fixture.execution.cache_lru, 0xffu, sizeof(fixture.execution.cache_lru));
    lib_memory_set(fixture.execution.cache_fill, 0xffu, sizeof(fixture.execution.cache_fill));
    lib_memory_set(fixture.execution.cache_read, 0xffu, sizeof(fixture.execution.cache_read));
    x86_core_cpu_state_reset(&fixture.execution);
    for (lib_u32 set = 0u; set < 128u; ++set) {
        if (fixture.execution.cache_lru[set] != 0u) return LIB_FALSE;
        for (lib_u32 way = 0u; way < 4u; ++way) {
            const cpu_cache_line *line = &fixture.execution.cache[set][way];
            if (line->valid || line->tag != 0u) return LIB_FALSE;
            for (lib_u32 word = 0u; word < 4u; ++word)
                if (line->data[word] != 0u) return LIB_FALSE;
        }
    }
    for (lib_u32 word = 0u; word < 4u; ++word)
        if (fixture.execution.cache_fill[word] != 0u ||
            fixture.execution.cache_read[word] != 0u) return LIB_FALSE;
    return LIB_TRUE;
}

static lib_bool cache_flush_privilege(void)
{
    for (lib_u8 mode = 1u; mode <= 4u; ++mode)
    for (lib_u8 opcode = 8u; opcode <= 9u; ++opcode) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u8 code[] = {0x0fu, opcode};
        lib_u32 frame[4];
        t_cpu after;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        cpu_instruction_prepare_protected_exception(fixture, mode);
        if (cpu_instruction_run(fixture, code, sizeof(code), &after) != LIB_STATUS_OK ||
            fixture->fault.valid || !fixture->delivered_exception.valid ||
            fixture->delivered_exception.exception_mask != VCPUINS_EXCEPT_GP ||
            fixture->delivered_exception.exception_code != 0u ||
            fixture->delivered_exception.point.eip != 0u || probe.operation_count != 0u ||
            after.data.eip != 0x300u || after.data.esp != (mode == 4u ? 0xfd8u : 0x7f0u))
            return LIB_FALSE;
        lib_memory_copy(frame, fixture->memory + after.data.esp, sizeof(frame));
        if (frame[0] != 0u || frame[1] != 0u ||
            frame[2] != (8u | (mode == 4u ? 3u : mode)) ||
            frame[3] != (2u | VCPU_EFLAGS_RF | (mode == 4u ? VCPU_EFLAGS_VM : 0u)))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_flush_failed_fetch(void)
{
    for (lib_u8 opcode = 8u; opcode <= 9u; ++opcode)
    for (lib_u32 fail_at = 0u; fail_at < 4u; ++fail_at) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u8 code[] = {0x66u, 0x67u, 0x0fu, opcode};
        cpu_cache_line before;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        if (!cache_write_register(fixture, 3u, 0x12345678u) ||
            !cache_write_register(fixture, 4u, 0x2400u) ||
            !cache_write_register(fixture, 5u, 1u)) return LIB_FALSE;
        before = fixture->execution.cache[0][0];
        probe.fail_fetch = LIB_TRUE;
        probe.fail_fetch_at = fail_at;
        if (cache_run(fixture, code, sizeof(code)) || !fixture->fault.valid ||
            fixture->fault.exception_mask != VCPUINS_EXCEPT_CE ||
            probe.operation_count != 0u ||
            lib_memory_compare(&before, &fixture->execution.cache[0][0], sizeof(before)))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_flush_operations(void)
{
    for (lib_u8 writeback = 0u; writeback < 2u; ++writeback)
    for (lib_u8 mode = 0u; mode < 4u; ++mode)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes)
    for (lib_u8 populated = 0u; populated < 2u; ++populated) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        lib_u8 code[4], bytes = 0u;
        x86_core_cpu_instruction_lexeme lexeme;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        if (populated && !cache_run(fixture, (const lib_u8[]){0xa1u, 0u, 0x20u}, 3u))
            return LIB_FALSE;
        fixture->cpu.data.cs.seg.exec.defsize = (mode & 1u) != 0u;
        if (mode >= 2u) fixture->cpu.data.cr0 |= VCPU_CR0_PE;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0x0fu;
        code[bytes++] = writeback ? 9u : 8u;
        const lib_u32 flags = fixture->cpu.data.eflags;
        const lib_u32 tag = fixture->execution.cache[0][0].tag;
        lib_u64 nominal_ticks = LIB_UINT64_MAX;

        if (!x86_core_cpu_instruction_lexeme_scan(code, bytes,
                X86_CORE_CPU_PROFILE_80486SX, (mode & 1u) != 0u, &lexeme) ||
            !cache_run(fixture, code, bytes) || fixture->cpu.data.eflags != flags ||
            !x86_core_cpu_80486_nominal_instruction_cost(&fixture->execution, &nominal_ticks) ||
            nominal_ticks != (writeback ? 5u : 4u) ||
            probe.operation_count != (writeback ? 2u : 1u) ||
            probe.operations[0] != (writeback ? X86_CORE_CPU_CACHE_WRITEBACK :
                X86_CORE_CPU_CACHE_FLUSH) ||
            (writeback && probe.operations[1] != X86_CORE_CPU_CACHE_FLUSH) ||
            fixture->execution.cache[0][0].valid ||
            fixture->execution.cache[0][0].tag != tag) {
            lib_c_printf("Flush form %u/%u/%u/%u: count=%u fault=%x eip=%x\n",
                (lib_u32)writeback, (lib_u32)mode, (lib_u32)prefixes,
                (lib_u32)populated, (lib_u32)probe.operation_count,
                fixture->fault.exception_mask, fixture->cpu.data.eip);
            return LIB_FALSE;
        }
    }
    for (lib_u8 fail = 1u; fail <= 2u; ++fail) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        if (!cache_run(fixture, (const lib_u8[]){0xa1u, 0u, 0x20u}, 3u))
            return LIB_FALSE;
        probe.fail_operation = fail;
        if (cache_run(fixture, (const lib_u8[]){0x0fu, 9u}, 2u) ||
            !fixture->fault.valid || fixture->fault.exception_mask != VCPUINS_EXCEPT_CE ||
            fixture->execution.cache[0][0].valid || probe.operation_count != fail ||
            fixture->cpu.data.eip != 0u) {
            lib_c_printf("Flush failure %u: count=%u fault=%x eip=%x\n",
                (lib_u32)fail, (lib_u32)probe.operation_count,
                fixture->fault.exception_mask, fixture->cpu.data.eip);
            return LIB_FALSE;
        }
    }
    for (lib_u8 opcode = 8u; opcode <= 9u; ++opcode) {
        cpu_instruction_fixture fixture;
        const lib_u8 code[] = {0x0fu, opcode};
        const lib_u8 locked[] = {0xf0u, 0x0fu, opcode};

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80386);
        if (!cpu_instruction_expect_real_fault(&fixture, code, sizeof(code), 6u))
            return LIB_FALSE;
        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        if (!cpu_instruction_expect_real_fault(&fixture, locked, sizeof(locked), 6u))
            return LIB_FALSE;
        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        if (cache_run(&fixture, code, sizeof(code)) || !fixture.fault.valid ||
            fixture.fault.exception_mask != VCPUINS_EXCEPT_CE) return LIB_FALSE;
    }
    return LIB_TRUE;
}

/* Cold/warm and split operands run through the real bus/cache, rather than
 * assigning the timing counters or manufacturing a nominal cache outcome. */
static lib_bool cache_fetch_timing(void)
{
    static const struct { lib_u8 start, parts, raw_ticks, lines; } positions[] = {
        {0u, 1u, 5u, 1u}, {1u, 1u, 5u, 1u}, {15u, 2u, 7u, 2u}
    };
    static const lib_u8 lengths[] = {1u, 2u, 15u};

    for (lib_size position = 0u; position < sizeof(positions) / sizeof(positions[0]); ++position)
    for (lib_size length = 0u; length < sizeof(lengths); ++length)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 cacheable = 0u; cacheable < 2u; ++cacheable)
    for (lib_u8 disabled = 0u; disabled < 2u; ++disabled) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u8 start = positions[position].start, bytes = lengths[length];
        const lib_bool fill = cacheable && !disabled;
        x86_core_cpu_instruction_lexeme lexeme;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        probe.cache_code = cacheable;
        fixture->cpu.data.cs.seg.exec.defsize = code32;
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        if (disabled) fixture->cpu.data.cr0 |= VCPU_CR0_CD;
        lib_memory_set(fixture->memory, 0x90u, 48u);
        lib_memory_set(fixture->memory + start, 0x2eu, bytes - 1u);
        for (lib_u8 warm = 0u; warm < 2u; ++warm) {
            const lib_u32 reads = probe.completed_fetches, fetched = probe.completed_fetch_bytes;
            const lib_u32 expected = fill ? (warm ? 0u : 5u * positions[position].lines) :
                positions[position].raw_ticks;

            fixture->cpu.data.eip = start;
            x86_core_cpu_execution_invalidate_prefetch(&fixture->execution);
            {
                const x86_core_cpu_execution_context before = fixture->execution;
                const t_cpuins instructions = fixture->instructions;
                const t_cpu cpu = fixture->cpu;

                if (!x86_core_cpu_execution_preview_lexeme(&fixture->execution, &lexeme) ||
                    lexeme.byte_count != bytes ||
                    lib_memory_compare(&before, &fixture->execution, sizeof(before)) ||
                    lib_memory_compare(&instructions, &fixture->instructions, sizeof(instructions)) ||
                    lib_memory_compare(&cpu, &fixture->cpu, sizeof(cpu)) ||
                    probe.completed_fetches != reads || probe.completed_fetch_bytes != fetched)
                    return LIB_FALSE;
            }
            x86_core_cpu_execution_refresh(&fixture->execution);
            if (fixture->fault.valid || fixture->cpu.data.eip != start + bytes ||
                fixture->instructions.data.timing_fetch_bus_ticks != expected ||
                probe.completed_fetches - reads !=
                    (fill ? (warm ? 0u : positions[position].lines) : positions[position].parts) ||
                probe.completed_fetch_bytes - fetched !=
                    (fill ? (warm ? 0u : 16u * positions[position].lines) : 15u) ||
                fixture->instructions.data.timing_data_misses != 0u) {
                lib_c_printf("Fetch timing position=%u length=%u code32=%u cache=%u CD=%u warm=%u: ticks=%u/%u reads=%u bytes=%u\n",
                    (lib_u32)position, (lib_u32)length, (lib_u32)code32, (lib_u32)cacheable,
                    (lib_u32)disabled, (lib_u32)warm, fixture->instructions.data.timing_fetch_bus_ticks,
                    expected, probe.completed_fetches - reads, probe.completed_fetch_bytes - fetched);
                return LIB_FALSE;
            }
            if (bytes < 15u) {
                const lib_u32 completed = probe.completed_fetches;

                x86_core_cpu_execution_refresh(&fixture->execution);
                if (fixture->fault.valid || fixture->cpu.data.eip != start + bytes + 1u ||
                    fixture->instructions.data.timing_fetch_bus_ticks != 0u ||
                    probe.completed_fetches != completed) return LIB_FALSE;
            }
        }
    }
    /* A segment-clipped prefetch can be shorter than a cache line yet span
     * two bus words. Byte count alone must not hide its first-word offset. */
    for (lib_u8 start = 0u; start < 4u; ++start) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        fixture->cpu.data.cs.limit = start + 3u;
        fixture->cpu.data.eip = start;
        lib_memory_set(fixture->memory, 0x90u, 8u);
        x86_core_cpu_execution_refresh(&fixture->execution);
        if (fixture->fault.valid || fixture->cpu.data.eip != start + 1u ||
            probe.completed_fetches != 1u || probe.completed_fetch_bytes != 4u ||
            fixture->instructions.data.timing_fetch_bus_ticks != (start ? 3u : 2u)) {
            lib_c_printf("Clipped fetch start=%u ticks=%u expected=%u reads=%u bytes=%u\n",
                (lib_u32)start, fixture->instructions.data.timing_fetch_bus_ticks,
                start ? 3u : 2u, probe.completed_fetches, probe.completed_fetch_bytes);
            return LIB_FALSE;
        }
    }
    for (lib_u8 cacheable = 0u; cacheable < 2u; ++cacheable) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        fixture->cpu.data.eip = 15u;
        fixture->memory[15u] = 0x2eu;
        fixture->memory[16u] = 0x90u;
        probe.cache_code = cacheable;
        probe.fail_fetch = LIB_TRUE;
        probe.fail_fetch_at = 16u;
        x86_core_cpu_execution_refresh(&fixture->execution);
        if (!fixture->fault.valid || fixture->fault.exception_mask != VCPUINS_EXCEPT_CE ||
            fixture->instructions.data.timing_fetch_bus_ticks != (cacheable ? 5u : 2u) ||
            probe.completed_fetches != 1u || probe.completed_fetch_bytes != (cacheable ? 16u : 1u) ||
            fixture->execution.timing_result.ticks != 0u) return LIB_FALSE;
    }
    {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        lib_u64 operand = 999u;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        probe.cache_code = LIB_TRUE;
        if (!cache_run(fixture, (const lib_u8[]){0x66u, 0xa1u, 0u, 0x20u}, 4u) ||
            fixture->instructions.data.timing_fetch_bus_ticks != 5u ||
            fixture->instructions.data.timing_data_misses != 1u ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &operand, LIB_NULL) || operand != 2u)
            return LIB_FALSE;
        x86_core_cpu_state_reset(&fixture->execution);
        if (fixture->instructions.data.timing_fetch_bus_ticks != 0u) return LIB_FALSE;
        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80386,
            &cache_memory_bus, &probe);
        if (!cache_run(fixture, (const lib_u8[]){0x90u}, 1u) ||
            fixture->instructions.data.timing_fetch_bus_ticks != 0u) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_access_timing(void)
{
    static const lib_u8 widths[] = {1u, 2u, 4u};
    static const lib_u16 addresses[] = {0x2000u, 0x2001u, 0x200fu};

    for (lib_u8 store = 0u; store < 2u; ++store)
    for (lib_u8 form = 0u; form < 4u; ++form)
    for (lib_u8 size = 0u; size < 3u; ++size)
    for (lib_u8 address = 0u; address < 3u; ++address) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        lib_u8 code[10], bytes = 0u;
        const lib_bool unaligned = size != 0u && address != 0u;
        const lib_bool split = size != 0u && address == 2u;

        if (!store && form == 2u) continue;
        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        if (size == 2u) code[bytes++] = 0x66u;
        if (form == 3u) {
            /* UMOV is the selected-SX MOV alias model, not a new bus path. */
            code[bytes++] = 0x0fu;
            code[bytes++] = (store ? 0x10u : 0x12u) + (size != 0u);
        } else code[bytes++] = form == 2u ? (size ? 0xc7u : 0xc6u) :
            form == 1u ? (store ? (size ? 0x89u : 0x88u) : (size ? 0x8bu : 0x8au)) :
            store ? (size ? 0xa3u : 0xa2u) : (size ? 0xa1u : 0xa0u);
        if (form != 0u) code[bytes++] = 0x06u;
        code[bytes++] = (lib_u8)addresses[address];
        code[bytes++] = (lib_u8)(addresses[address] >> 8u);
        if (form == 2u)
            for (lib_u8 immediate = 0u; immediate < widths[size]; ++immediate)
                code[bytes++] = 0x5au;
        for (lib_u8 warm = 0u; warm < 2u; ++warm) {
            const lib_u32 misses = store || warm ? 0u : split ? 2u : 1u;
            lib_u64 ticks = 999u;
            lib_bool modeled = LIB_FALSE;

            if (!cache_run(fixture, code, bytes) ||
                fixture->instructions.data.timing_data_misses != misses ||
                fixture->instructions.data.timing_missed_reads != (lib_u32)(misses != 0u) ||
                fixture->instructions.data.timing_unaligned_accesses != (lib_u32)unaligned ||
                fixture->instructions.data.timing_locked_reads != 0u ||
                !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
                modeled != (form == 3u || misses > 1u) ||
                ticks != (misses ? 2u + 5u * (misses - 1u) : 0u) + 3u * unaligned) {
                lib_c_printf("Access timing %u/%u/%u/%u/%u: miss=%u align=%u ticks=%llu\n",
                    (lib_u32)store, (lib_u32)form, (lib_u32)widths[size], (lib_u32)address,
                    (lib_u32)warm, fixture->instructions.data.timing_data_misses,
                    fixture->instructions.data.timing_unaligned_accesses,
                    ticks);
                return LIB_FALSE;
            }
            {
                x86_core_cpu_instruction_lexeme lexeme;
                const t_cpuins before_instructions = fixture->instructions;
                const lib_u32 reads = probe.data_reads;
                const lib_u32 writes = fixture->writes;

                fixture->cpu.data.eip = 0u;
                const x86_core_cpu_execution_context before = fixture->execution;
                const t_cpu before_cpu = fixture->cpu;
                if (!x86_core_cpu_execution_preview_lexeme(&fixture->execution, &lexeme) ||
                    lexeme.byte_count != bytes ||
                    lib_memory_compare(&before, &fixture->execution, sizeof(before)) ||
                    lib_memory_compare(&before_cpu, &fixture->cpu, sizeof(before_cpu)) ||
                    lib_memory_compare(&before_instructions, &fixture->instructions,
                        sizeof(before_instructions)) || probe.data_reads != reads ||
                    fixture->writes != writes)
                    return LIB_FALSE;
            }
        }
        {
            lib_u64 ticks = 999u;

            if (!cache_run(fixture, (const lib_u8[]){0x90u}, 1u) ||
                !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, LIB_NULL) || ticks != 0u)
                return LIB_FALSE;
        }
    }
    {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        lib_u64 ticks = 999u;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        if (!cache_run(fixture, (const lib_u8[]){0x87u, 0x06u, 0u, 0x20u}, 4u) ||
            fixture->instructions.data.timing_locked_reads != 1u ||
            fixture->instructions.data.timing_data_misses != 0u ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, LIB_NULL) || ticks != 0u)
            return LIB_FALSE;
    }
    {
        cpu_instruction_fixture fixture;
        lib_u64 ticks = 999u;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        if (x86_core_cpu_80486_memory_penalty(LIB_NULL, &ticks, LIB_NULL) ||
            x86_core_cpu_80486_memory_penalty(&fixture.execution, LIB_NULL, LIB_NULL) ||
            x86_core_cpu_80486_memory_penalty(&fixture.execution, &ticks, LIB_NULL) || ticks != 999u)
            return LIB_FALSE;
        fixture.execution.cpu_profile = X86_CORE_CPU_PROFILE_80386;
        if (!cache_run(&fixture, (const lib_u8[]){0x90u}, 1u) ||
            x86_core_cpu_80486_memory_penalty(&fixture.execution, &ticks, LIB_NULL) || ticks != 999u)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

/* Each row is a manual cache-miss coefficient. Execute the real encoding in
 * register/memory, cold/warm and (where legal) explicit LOCK forms. */
static lib_bool cache_integer_timing(void)
{
    static const struct {
        /* Sizes is a byte/word/dword bitset; three-width rows use opcode W.
         * Immediate 2 means operand-width bytes, not a two-byte immediate. */
        lib_u8 opcode, secondary, extension, sizes, immediate, miss;
        lib_bool lockable, implicit_lock;
    } cases[] = {
        /* E-4 ALU: memory destination versus memory source. */
        {0x00u, 0u, 0u, 7u, 0u, 6u, LIB_TRUE, LIB_FALSE},
        {0x02u, 0u, 0u, 7u, 0u, 2u, LIB_FALSE, LIB_FALSE},
        {0x08u, 0u, 0u, 7u, 0u, 6u, LIB_TRUE, LIB_FALSE},
        {0x0au, 0u, 0u, 7u, 0u, 2u, LIB_FALSE, LIB_FALSE},
        {0x10u, 0u, 0u, 7u, 0u, 6u, LIB_TRUE, LIB_FALSE},
        {0x12u, 0u, 0u, 7u, 0u, 2u, LIB_FALSE, LIB_FALSE},
        {0x18u, 0u, 0u, 7u, 0u, 6u, LIB_TRUE, LIB_FALSE},
        {0x1au, 0u, 0u, 7u, 0u, 2u, LIB_FALSE, LIB_FALSE},
        {0x20u, 0u, 0u, 7u, 0u, 6u, LIB_TRUE, LIB_FALSE},
        {0x22u, 0u, 0u, 7u, 0u, 2u, LIB_FALSE, LIB_FALSE},
        {0x28u, 0u, 0u, 7u, 0u, 6u, LIB_TRUE, LIB_FALSE},
        {0x2au, 0u, 0u, 7u, 0u, 2u, LIB_FALSE, LIB_FALSE},
        {0x30u, 0u, 0u, 7u, 0u, 6u, LIB_TRUE, LIB_FALSE},
        {0x32u, 0u, 0u, 7u, 0u, 2u, LIB_FALSE, LIB_FALSE},
        {0x38u, 0u, 0u, 7u, 0u, 2u, LIB_FALSE, LIB_FALSE},
        {0x3au, 0u, 0u, 7u, 0u, 2u, LIB_FALSE, LIB_FALSE},
        /* Immediate value 1; marker 2 expands to the operand's width. */
        {0x80u, 0u, 0u, 7u, 2u, 6u, LIB_TRUE, LIB_FALSE},
        {0x80u, 0u, 1u, 7u, 2u, 6u, LIB_TRUE, LIB_FALSE},
        {0x80u, 0u, 2u, 7u, 2u, 6u, LIB_TRUE, LIB_FALSE},
        {0x80u, 0u, 3u, 7u, 2u, 6u, LIB_TRUE, LIB_FALSE},
        {0x80u, 0u, 4u, 7u, 2u, 6u, LIB_TRUE, LIB_FALSE},
        {0x80u, 0u, 5u, 7u, 2u, 6u, LIB_TRUE, LIB_FALSE},
        {0x80u, 0u, 6u, 7u, 2u, 6u, LIB_TRUE, LIB_FALSE},
        {0x80u, 0u, 7u, 7u, 2u, 2u, LIB_FALSE, LIB_FALSE},
        {0x83u, 0u, 0u, 6u, 1u, 6u, LIB_TRUE, LIB_FALSE},
        {0x83u, 0u, 1u, 6u, 1u, 6u, LIB_TRUE, LIB_FALSE},
        {0x83u, 0u, 2u, 6u, 1u, 6u, LIB_TRUE, LIB_FALSE},
        {0x83u, 0u, 3u, 6u, 1u, 6u, LIB_TRUE, LIB_FALSE},
        {0x83u, 0u, 4u, 6u, 1u, 6u, LIB_TRUE, LIB_FALSE},
        {0x83u, 0u, 5u, 6u, 1u, 6u, LIB_TRUE, LIB_FALSE},
        {0x83u, 0u, 6u, 6u, 1u, 6u, LIB_TRUE, LIB_FALSE},
        {0x83u, 0u, 7u, 6u, 1u, 2u, LIB_FALSE, LIB_FALSE},
        {0xfeu, 0u, 0u, 7u, 0u, 6u, LIB_TRUE, LIB_FALSE},
        {0xfeu, 0u, 1u, 7u, 0u, 6u, LIB_TRUE, LIB_FALSE},
        {0xf6u, 0u, 2u, 7u, 0u, 6u, LIB_TRUE, LIB_FALSE},
        {0xf6u, 0u, 3u, 7u, 0u, 6u, LIB_TRUE, LIB_FALSE},
        {0x84u, 0u, 0u, 7u, 0u, 2u, LIB_FALSE, LIB_FALSE},
        {0xf6u, 0u, 0u, 7u, 2u, 2u, LIB_FALSE, LIB_FALSE},
        {0x86u, 0u, 0u, 7u, 0u, 0u, LIB_TRUE, LIB_TRUE},
        /* E-6 single, CL and immediate shift/rotate counts. */
        {0xd0u, 0u, 0u, 7u, 0u, 6u, LIB_FALSE, LIB_FALSE},
        {0xd0u, 0u, 1u, 7u, 0u, 6u, LIB_FALSE, LIB_FALSE},
        {0xd0u, 0u, 2u, 7u, 0u, 6u, LIB_FALSE, LIB_FALSE},
        {0xd0u, 0u, 3u, 7u, 0u, 6u, LIB_FALSE, LIB_FALSE},
        {0xd0u, 0u, 4u, 7u, 0u, 6u, LIB_FALSE, LIB_FALSE},
        {0xd0u, 0u, 5u, 7u, 0u, 6u, LIB_FALSE, LIB_FALSE},
        {0xd0u, 0u, 7u, 7u, 0u, 6u, LIB_FALSE, LIB_FALSE},
        {0xd2u, 0u, 0u, 7u, 0u, 6u, LIB_FALSE, LIB_FALSE},
        {0xd2u, 0u, 1u, 7u, 0u, 6u, LIB_FALSE, LIB_FALSE},
        {0xd2u, 0u, 2u, 7u, 0u, 0u, LIB_FALSE, LIB_FALSE},
        {0xd2u, 0u, 3u, 7u, 0u, 0u, LIB_FALSE, LIB_FALSE},
        {0xd2u, 0u, 4u, 7u, 0u, 6u, LIB_FALSE, LIB_FALSE},
        {0xd2u, 0u, 5u, 7u, 0u, 6u, LIB_FALSE, LIB_FALSE},
        {0xd2u, 0u, 7u, 7u, 0u, 6u, LIB_FALSE, LIB_FALSE},
        {0xc0u, 0u, 0u, 7u, 1u, 6u, LIB_FALSE, LIB_FALSE},
        {0xc0u, 0u, 1u, 7u, 1u, 6u, LIB_FALSE, LIB_FALSE},
        {0xc0u, 0u, 2u, 7u, 1u, 0u, LIB_FALSE, LIB_FALSE},
        {0xc0u, 0u, 3u, 7u, 1u, 0u, LIB_FALSE, LIB_FALSE},
        {0xc0u, 0u, 4u, 7u, 1u, 6u, LIB_FALSE, LIB_FALSE},
        {0xc0u, 0u, 5u, 7u, 1u, 6u, LIB_FALSE, LIB_FALSE},
        {0xc0u, 0u, 7u, 7u, 1u, 6u, LIB_FALSE, LIB_FALSE},
        /* E-3/E-5/E-6 escaped and multiply/divide operand rows. */
        {0x0fu, 0xb6u, 0u, 6u, 0u, 2u, LIB_FALSE, LIB_FALSE},
        {0x0fu, 0xb7u, 0u, 6u, 0u, 2u, LIB_FALSE, LIB_FALSE},
        {0x0fu, 0xbeu, 0u, 6u, 0u, 2u, LIB_FALSE, LIB_FALSE},
        {0x0fu, 0xbfu, 0u, 6u, 0u, 2u, LIB_FALSE, LIB_FALSE},
        {0x0fu, 0xc0u, 0u, 7u, 0u, 6u, LIB_TRUE, LIB_FALSE},
        {0x0fu, 0xb0u, 0u, 7u, 0u, 2u, LIB_TRUE, LIB_FALSE},
        {0x0fu, 0xa4u, 0u, 6u, 1u, 6u, LIB_FALSE, LIB_FALSE},
        {0x0fu, 0xacu, 0u, 6u, 1u, 6u, LIB_FALSE, LIB_FALSE},
        {0x0fu, 0xa5u, 0u, 6u, 0u, 5u, LIB_FALSE, LIB_FALSE},
        {0x0fu, 0xadu, 0u, 6u, 0u, 5u, LIB_FALSE, LIB_FALSE},
        {0x0fu, 0xafu, 0u, 6u, 0u, 1u, LIB_FALSE, LIB_FALSE},
        {0x69u, 0u, 0u, 6u, 2u, 2u, LIB_FALSE, LIB_FALSE},
        {0x6bu, 0u, 0u, 6u, 1u, 2u, LIB_FALSE, LIB_FALSE},
        {0xf6u, 0u, 4u, 7u, 0u, 1u, LIB_FALSE, LIB_FALSE},
        {0xf6u, 0u, 5u, 7u, 0u, 0u, LIB_FALSE, LIB_FALSE},
        {0xf6u, 0u, 6u, 7u, 0u, 0u, LIB_FALSE, LIB_FALSE},
        {0xf6u, 0u, 7u, 7u, 0u, 0u, LIB_FALSE, LIB_FALSE},
        /* E-9 register versus immediate bit index and scan direction. */
        {0x0fu, 0xa3u, 0u, 6u, 0u, 2u, LIB_FALSE, LIB_FALSE},
        {0x0fu, 0xabu, 0u, 6u, 0u, 3u, LIB_TRUE, LIB_FALSE},
        {0x0fu, 0xb3u, 0u, 6u, 0u, 3u, LIB_TRUE, LIB_FALSE},
        {0x0fu, 0xbbu, 0u, 6u, 0u, 3u, LIB_TRUE, LIB_FALSE},
        {0x0fu, 0xbau, 4u, 6u, 1u, 1u, LIB_FALSE, LIB_FALSE},
        {0x0fu, 0xbau, 5u, 6u, 1u, 2u, LIB_TRUE, LIB_FALSE},
        {0x0fu, 0xbau, 6u, 6u, 1u, 2u, LIB_TRUE, LIB_FALSE},
        {0x0fu, 0xbau, 7u, 6u, 1u, 2u, LIB_TRUE, LIB_FALSE},
        {0x0fu, 0xbcu, 0u, 6u, 0u, 2u, LIB_FALSE, LIB_FALSE},
        {0x0fu, 0xbdu, 0u, 6u, 0u, 1u, LIB_FALSE, LIB_FALSE}
    };

    for (lib_size row = 0u; row < sizeof(cases) / sizeof(cases[0]); ++row)
    for (lib_u8 size = 0u; size < 3u; ++size)
    for (lib_u8 memory = 0u; memory < 2u; ++memory)
    for (lib_u8 warm = 0u; warm < 2u; ++warm)
    for (lib_u8 lock = 0u; lock < (memory && cases[row].lockable ? 2u : 1u); ++lock)
    for (lib_u8 equal = 0u; equal < (cases[row].secondary == 0xb0u ? 2u : 1u); ++equal) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        lib_u8 code[12], bytes = 0u;
        const lib_bool locked = memory && (lock || cases[row].implicit_lock);
        lib_u64 ticks = 999u;

        if ((cases[row].sizes & (1u << size)) == 0u) continue;
        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        lib_memory_copy(fixture->memory + 0x2000u, &(lib_u32){2u}, sizeof(lib_u32));
        if (warm && !cache_run(fixture, (const lib_u8[]){0xa0u, 0u, 0x20u}, 3u))
            return LIB_FALSE;
        fixture->cpu.data.eax = equal ? 2u : 12u;
        fixture->cpu.data.ebx = 2u;
        fixture->cpu.data.ecx = 1u;
        fixture->cpu.data.edx = 0u;
        fixture->cpu.data.eflags = 2u;
        if (lock) code[bytes++] = 0xf0u;
        if (size == 2u) code[bytes++] = 0x66u;
        code[bytes++] = cases[row].opcode +
            (cases[row].opcode != 0x0fu && cases[row].sizes == 7u && size != 0u);
        if (cases[row].opcode == 0x0fu)
            code[bytes++] = cases[row].secondary + (cases[row].sizes == 7u && size != 0u);
        code[bytes++] = (memory ? 0x06u : 0xc3u) | (cases[row].extension << 3u);
        if (memory) {
            code[bytes++] = 0u;
            code[bytes++] = 0x20u;
        }
        if (cases[row].immediate) {
            code[bytes++] = 1u;
            if (cases[row].immediate == 2u)
                for (lib_u8 index = 1u; index < (1u << size); ++index) code[bytes++] = 0u;
        }
        const lib_u32 misses = memory && !locked && !warm;
        const lib_u64 lock_clocks = cases[row].implicit_lock || cases[row].secondary == 0xbau ? 0u :
            cases[row].secondary == 0xabu || cases[row].secondary == 0xb3u ||
            cases[row].secondary == 0xbbu ? 1u : 2u;
        const lib_u64 expected = locked ? lock_clocks :
            (lib_u64)cases[row].miss * misses;
        if (!cache_run(fixture, code, bytes) ||
            fixture->instructions.data.timing_data_misses != misses ||
            fixture->instructions.data.timing_locked_reads != (lib_u32)locked ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, LIB_NULL) || ticks != expected ||
            probe.lock_protocol_error || probe.lock_active ||
            (cases[row].secondary == 0xb0u &&
             ((fixture->cpu.data.eflags & VCPU_EFLAGS_ZF) != 0u) != equal)) {
            lib_c_printf("Integer miss row=%u size=%u memory=%u warm=%u lock=%u equal=%u: miss=%u locked=%u ticks=%llu/%llu fault=%x\n",
                (lib_u32)row, (lib_u32)size, (lib_u32)memory, (lib_u32)warm,
                (lib_u32)lock, (lib_u32)equal, fixture->instructions.data.timing_data_misses,
                fixture->instructions.data.timing_locked_reads, ticks, expected,
                fixture->fault.exception_mask);
            return LIB_FALSE;
        }
        {
            const x86_core_cpu_execution_context before = fixture->execution;
            const t_cpuins before_instructions = fixture->instructions;
            const t_cpu before_cpu = fixture->cpu;
            const lib_u32 reads = probe.data_reads, writes = fixture->writes;

            if (!x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, LIB_NULL) || ticks != expected ||
                lib_memory_compare(&before, &fixture->execution, sizeof(before)) ||
                lib_memory_compare(&before_instructions, &fixture->instructions, sizeof(before_instructions)) ||
                lib_memory_compare(&before_cpu, &fixture->cpu, sizeof(before_cpu)) ||
                probe.data_reads != reads || fixture->writes != writes) return LIB_FALSE;
        }
    }
    for (lib_u8 lock = 0u; lock < 2u; ++lock)
    for (lib_u8 write = 0u; write < 2u; ++write) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        lib_u8 code[5], bytes = 0u;
        lib_u64 ticks = 999u;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        probe.fail_read = !write;
        probe.fail_write = write;
        if (lock) code[bytes++] = 0xf0u;
        code[bytes++] = 0x01u;
        code[bytes++] = 0x06u;
        code[bytes++] = 0u;
        code[bytes++] = 0x20u;
        if (cache_run(fixture, code, bytes) || !fixture->fault.valid ||
            x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, LIB_NULL) || ticks != 999u ||
            fixture->instructions.data.timing_data_misses != (lib_u32)(!lock && write) ||
            fixture->instructions.data.timing_locked_reads != (lib_u32)(lock && write))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_integer_alignment_timing(void)
{
    static const struct {
        lib_u8 opcode, secondary, extension, miss, accesses;
        lib_bool lockable, implicit_lock;
    } cases[] = {
        {0x01u, 0u, 0u, 6u, 2u, LIB_TRUE, LIB_FALSE},
        {0x03u, 0u, 0u, 2u, 1u, LIB_FALSE, LIB_FALSE},
        {0x39u, 0u, 0u, 2u, 1u, LIB_FALSE, LIB_FALSE},
        {0x0fu, 0xc1u, 0u, 6u, 2u, LIB_TRUE, LIB_FALSE},
        {0x0fu, 0xb1u, 0u, 2u, 2u, LIB_TRUE, LIB_FALSE},
        {0x87u, 0u, 0u, 0u, 2u, LIB_TRUE, LIB_TRUE},
        {0xc1u, 0u, 4u, 6u, 2u, LIB_FALSE, LIB_FALSE},
        {0x85u, 0u, 0u, 2u, 1u, LIB_FALSE, LIB_FALSE}
    };
    static const lib_u16 addresses[] = {0x2001u, 0x200fu};

    for (lib_size row = 0u; row < sizeof(cases) / sizeof(cases[0]); ++row)
    for (lib_u8 dword = 0u; dword < 2u; ++dword)
    for (lib_u8 split = 0u; split < 2u; ++split)
    for (lib_u8 warm = 0u; warm < 2u; ++warm)
    for (lib_u8 lock = 0u; lock < (cases[row].lockable ? 2u : 1u); ++lock) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        lib_u8 code[8], bytes = 0u;
        const lib_bool locked = lock || cases[row].implicit_lock;
        const lib_u32 misses = locked || warm ? 0u : split ? 2u : 1u;
        const lib_u32 locked_reads = !locked ? 0u : split ? 2u : 1u;
        const lib_u64 expected = 3u * (lib_u64)cases[row].accesses +
            (locked ? (cases[row].implicit_lock ? 0u : 2u) + 2u * (locked_reads - 1u) :
                misses ? cases[row].miss + 5u * (misses - 1u) : 0u);
        lib_u64 ticks = 999u;
        lib_bool modeled = LIB_FALSE;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        lib_memory_copy(fixture->memory + addresses[split], &(lib_u32){2u}, sizeof(lib_u32));
        if (warm) {
            const lib_u8 load[] = {0x66u, 0xa1u,
                (lib_u8)addresses[split], (lib_u8)(addresses[split] >> 8u)};

            if (!cache_run(fixture, load, sizeof(load))) return LIB_FALSE;
        }
        fixture->cpu.data.eax = 12u;
        fixture->cpu.data.ecx = 1u;
        if (lock) code[bytes++] = 0xf0u;
        if (dword) code[bytes++] = 0x66u;
        code[bytes++] = cases[row].opcode;
        if (cases[row].opcode == 0x0fu) code[bytes++] = cases[row].secondary;
        code[bytes++] = 0x06u | (cases[row].extension << 3u);
        code[bytes++] = (lib_u8)addresses[split];
        code[bytes++] = (lib_u8)(addresses[split] >> 8u);
        if (cases[row].opcode == 0xc1u) code[bytes++] = 1u;
        if (!cache_run(fixture, code, bytes) ||
            fixture->instructions.data.timing_unaligned_accesses != cases[row].accesses ||
            fixture->instructions.data.timing_data_misses != misses ||
            fixture->instructions.data.timing_locked_reads != locked_reads ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            modeled != (misses > 1u || locked_reads > 1u) || ticks != expected) {
            lib_c_printf("Integer alignment row=%u dword=%u split=%u warm=%u lock=%u: align=%u miss=%u locked=%u ticks=%llu/%llu\n",
                (lib_u32)row, (lib_u32)dword, (lib_u32)split, (lib_u32)warm, (lib_u32)lock,
                fixture->instructions.data.timing_unaligned_accesses,
                fixture->instructions.data.timing_data_misses,
                fixture->instructions.data.timing_locked_reads, ticks, expected);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

/* Both cache-line and page splits use the actual memory/lock path. */
static lib_bool cache_split_timing(void)
{
    static const lib_u16 addresses[] = {0x200fu, 0x2fffu};

    for (lib_u8 page = 0u; page < 2u; ++page)
    for (lib_u8 dword = 0u; dword < 2u; ++dword)
    for (lib_u8 locked = 0u; locked < 2u; ++locked)
    for (lib_u8 residency = 0u; residency < 6u; ++residency) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u16 address = addresses[page];
        const lib_bool fail = residency == 5u;
        const lib_u32 misses = locked ? 0u : residency == 3u ? 0u :
            residency == 1u || residency == 2u || fail ? 1u : 2u;
        lib_u8 code[6], bytes = 0u;
        lib_u64 ticks = 999u;
        lib_bool modeled = LIB_TRUE;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        if (residency != 4u) fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        for (lib_u8 line = 0u; line < 2u; ++line) {
            const lib_u16 warm_address = address + line;
            const lib_u8 load[] = {0xa0u, (lib_u8)warm_address, (lib_u8)(warm_address >> 8u)};

            if ((residency == line + 1u || residency == 3u) &&
                !cache_run(fixture, load, sizeof(load))) return LIB_FALSE;
        }
        if (dword) code[bytes++] = 0x66u;
        code[bytes++] = locked ? 0x87u : 0xa1u;
        if (locked) code[bytes++] = 0x06u;
        code[bytes++] = (lib_u8)address;
        code[bytes++] = (lib_u8)(address >> 8u);
        probe.fail_read = fail;
        probe.fail_read_at = address + 1u;
        if (cache_run(fixture, code, bytes) == fail ||
            fixture->instructions.data.timing_data_misses != misses ||
            fixture->instructions.data.timing_locked_reads != (locked ? (fail ? 1u : 2u) : 0u) ||
            probe.lock_active || probe.lock_protocol_error) return LIB_FALSE;
        if (fail) {
            if (!fixture->fault.valid || fixture->instructions.data.timing_unaligned_accesses != 0u ||
                fixture->instructions.data.timing_missed_reads != 0u ||
                x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
                ticks != 999u || !modeled) return LIB_FALSE;
            continue;
        }
        const lib_u64 expected = locked ? 8u :
            3u + (misses == 0u ? 0u : misses == 1u ? 2u : 7u);
        if (!x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            ticks != expected || modeled != (locked || misses == 2u)) return LIB_FALSE;
        if (!cache_run(fixture, (const lib_u8[]){0x90u}, 1u) ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            ticks != 0u || modeled) return LIB_FALSE;
    }
    return LIB_TRUE;
}

/* Source coefficients for single-read stack/control and primitive strings.
 * The data, stack and destination occupy distinct cache sets (E-3/E-7/E-9). */
static lib_bool cache_stack_string_timing(void)
{
    static const struct {
        lib_u8 code[5], length, reads, miss;
        lib_bool transfer;
    } cases[] = {
        {{0x50u}, 1u, 0u, 0u, LIB_FALSE},
        {{0x68u, 2u, 0u}, 3u, 0u, 0u, LIB_FALSE},
        {{0x6au, 2u}, 2u, 0u, 0u, LIB_FALSE},
        {{0xffu, 0xf0u}, 2u, 0u, 0u, LIB_FALSE},
        {{0xffu, 0x36u, 0u, 0x20u}, 4u, 1u, 1u, LIB_FALSE},
        {{0x58u}, 1u, 1u, 2u, LIB_FALSE},
        {{0x8fu, 0xc0u}, 2u, 1u, 1u, LIB_FALSE},
        {{0x8fu, 0x06u, 0u, 0x20u}, 4u, 1u, 2u, LIB_FALSE},
        {{0xc9u}, 1u, 1u, 1u, LIB_FALSE},
        {{0xe8u, 0u, 0u}, 3u, 0u, 0u, LIB_FALSE},
        {{0xffu, 0xd0u}, 2u, 0u, 0u, LIB_TRUE},
        {{0xffu, 0xe0u}, 2u, 0u, 0u, LIB_TRUE},
        {{0xffu, 0x16u, 0u, 0x20u}, 4u, 1u, 5u, LIB_TRUE},
        {{0xffu, 0x26u, 0u, 0x20u}, 4u, 1u, 5u, LIB_TRUE},
        {{0xc3u}, 1u, 1u, 5u, LIB_TRUE},
        {{0xc2u, 4u, 0u}, 3u, 1u, 5u, LIB_TRUE},
        {{0xa4u}, 1u, 1u, 2u, LIB_FALSE},
        {{0xa5u}, 1u, 1u, 2u, LIB_FALSE},
        {{0xacu}, 1u, 1u, 2u, LIB_FALSE},
        {{0xadu}, 1u, 1u, 2u, LIB_FALSE},
        {{0xaeu}, 1u, 1u, 2u, LIB_FALSE},
        {{0xafu}, 1u, 1u, 2u, LIB_FALSE},
        {{0xaau}, 1u, 0u, 0u, LIB_FALSE},
        {{0xabu}, 1u, 0u, 0u, LIB_FALSE},
        {{0xd7u}, 1u, 1u, 2u, LIB_FALSE},
        {{0x9cu}, 1u, 0u, 0u, LIB_FALSE},
        {{0x9du}, 1u, 1u, 0u, LIB_FALSE},
        {{0x60u}, 1u, 0u, 0u, LIB_FALSE},
        {{0x07u}, 1u, 1u, 2u, LIB_FALSE},
        {{0x17u}, 1u, 1u, 2u, LIB_FALSE},
        {{0x1fu}, 1u, 1u, 2u, LIB_FALSE},
        {{0x0fu, 0xa1u}, 2u, 1u, 2u, LIB_FALSE},
        {{0x0fu, 0xa9u}, 2u, 1u, 2u, LIB_FALSE},
        {{0x8eu, 0x1eu, 0u, 0x20u}, 4u, 1u, 2u, LIB_FALSE},
        {{0xc4u, 0x06u, 0u, 0x20u}, 4u, 1u, 7u, LIB_FALSE},
        {{0xc5u, 0x06u, 0u, 0x20u}, 4u, 1u, 7u, LIB_FALSE},
        {{0x0fu, 0xb2u, 0x06u, 0u, 0x20u}, 5u, 1u, 7u, LIB_FALSE},
        {{0x0fu, 0xb4u, 0x06u, 0u, 0x20u}, 5u, 1u, 7u, LIB_FALSE},
        {{0x0fu, 0xb5u, 0x06u, 0u, 0x20u}, 5u, 1u, 7u, LIB_FALSE},
        {{0x0fu, 0x01u, 0x16u, 0u, 0x20u}, 5u, 1u, 5u, LIB_FALSE},
        {{0x0fu, 0x01u, 0x1eu, 0u, 0x20u}, 5u, 1u, 5u, LIB_FALSE},
        {{0x0fu, 0x01u, 0x36u, 0u, 0x20u}, 5u, 1u, 1u, LIB_FALSE},
        {{0x0fu, 0x01u, 0x16u, 2u, 0x20u}, 5u, 1u, 5u, LIB_FALSE},
        {{0x0fu, 0x01u, 0x1eu, 2u, 0x20u}, 5u, 1u, 5u, LIB_FALSE}
    };

    for (lib_size row = 0u; row < sizeof(cases) / sizeof(cases[0]); ++row)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 operand32 = 0u; operand32 < 2u; ++operand32)
    for (lib_u8 phase = 0u; phase < 3u; ++phase) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_bool warm = phase == 1u, fail = phase == 2u;
        const lib_u32 value = cases[row].code[0] == 0x9du ? 2u :
            cases[row].code[0] == 0x07u || cases[row].code[0] == 0x17u ||
            cases[row].code[0] == 0x1fu || cases[row].code[0] == 0x8eu ||
            (cases[row].code[0] == 0x0fu && (cases[row].code[1] == 0xa1u ||
                cases[row].code[1] == 0xa9u)) ? 0u : 0x100u;
        lib_u8 code[10], bytes = 0u;
        lib_u64 ticks = 999u;
        t_cpu after;

        if (fail && cases[row].reads == 0u) continue;
        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        for (lib_u32 address = 0x2000u; address <= 0x2200u; address += 0x100u) {
            lib_memory_copy(fixture->memory + address, &value, sizeof(value));
            if (warm && !cache_run(fixture,
                    (const lib_u8[]){0xa0u, (lib_u8)address, (lib_u8)(address >> 8u)}, 3u))
                return LIB_FALSE;
        }
        fixture->cpu.data.cs.seg.exec.defsize = code32;
        fixture->cpu.data.eax = 0x100u;
        fixture->cpu.data.ebx = fixture->cpu.data.esi = 0x2000u;
        fixture->cpu.data.esp = fixture->cpu.data.ebp = 0x2100u;
        fixture->cpu.data.edi = 0x2200u;
        fixture->cpu.data.eflags = 2u;
        if (code32) code[bytes++] = 0x67u;
        if (code32 != operand32) code[bytes++] = 0x66u;
        lib_memory_copy(code + bytes, cases[row].code, cases[row].length);
        bytes += cases[row].length;
        if (operand32 && (cases[row].code[0] == 0x68u || cases[row].code[0] == 0xe8u)) {
            code[bytes++] = 0u;
            code[bytes++] = 0u;
        }
        fixture->cpu.data.eip = 0u;
        x86_core_cpu_execution_invalidate_prefetch(&fixture->execution);
        probe.fail_read = fail;
        const lib_u32 misses = warm ? 0u : cases[row].reads;
        const lib_u32 unaligned = cases[row].code[0] == 0x0fu &&
            cases[row].code[1] == 0x01u &&
            (cases[row].code[2] == 0x16u || cases[row].code[2] == 0x1eu) &&
            cases[row].code[3] == 0u;
        const lib_status status = cpu_instruction_run(fixture, code, bytes, &after);
        if (fail) {
            if ((status == LIB_STATUS_OK && !fixture->fault.valid &&
                    !fixture->delivered_exception.valid) ||
                x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, LIB_NULL) || ticks != 999u)
                return LIB_FALSE;
            continue;
        }
        if (status != LIB_STATUS_OK ||
            fixture->fault.valid || fixture->delivered_exception.valid ||
            after.data.eip != (cases[row].transfer ? 0x100u : bytes) ||
            fixture->instructions.data.timing_data_misses != misses ||
            fixture->instructions.data.timing_locked_reads != 0u ||
            fixture->instructions.data.timing_unaligned_accesses != unaligned ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, LIB_NULL) ||
            ticks != (warm ? 0u : cases[row].miss) + 3u * unaligned) {
            lib_c_printf("Stack/string timing row=%u code32=%u operand32=%u warm=%u: miss=%u ticks=%llu eip=%x fault=%x\n",
                (lib_u32)row, (lib_u32)code32, (lib_u32)operand32, (lib_u32)warm,
                fixture->instructions.data.timing_data_misses, ticks, after.data.eip,
                fixture->fault.exception_mask);
            return LIB_FALSE;
        }
        const t_cpu before_cpu = fixture->cpu;
        const t_cpuins before_instructions = fixture->instructions;
        const x86_core_cpu_execution_context before_execution = fixture->execution;
        const lib_u32 reads = probe.data_reads, writes = fixture->writes;
        if (!x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, LIB_NULL) ||
            lib_memory_compare(&before_cpu, &fixture->cpu, sizeof(before_cpu)) ||
            lib_memory_compare(&before_instructions, &fixture->instructions, sizeof(before_instructions)) ||
            lib_memory_compare(&before_execution, &fixture->execution, sizeof(before_execution)) ||
            probe.data_reads != reads || fixture->writes != writes)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_compare_timing(void)
{
    static const lib_u8 offsets[] = {0u, 1u, 15u};

    for (lib_u8 size = 0u; size < 3u; ++size)
    for (lib_u8 position = 0u; position < 3u; ++position)
    for (lib_u8 same = 0u; same < 2u; ++same)
    for (lib_u8 warm = 0u; warm < 4u; ++warm)
    for (lib_u8 disabled = 0u; disabled < 2u; ++disabled) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u16 source = 0x2100u + offsets[position];
        const lib_u16 destination = (same ? 0x2100u : 0x2200u) + offsets[position];
        const lib_u32 reads = disabled ? 2u : same ? (warm == 0u) :
            2u - (warm & 1u) - (warm >> 1u);
        const lib_u32 misses = reads * (size != 0u && position == 2u ? 2u : 1u);
        const lib_u32 unaligned = size != 0u && position != 0u ? 2u : 0u;
        lib_u8 code[2], bytes = 0u;
        lib_u64 ticks = 999u;
        lib_bool modeled = LIB_TRUE;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        if (!disabled) fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        for (lib_u8 operand = 0u; operand < 2u; ++operand) {
            const lib_u16 address = operand ? destination : source;
            lib_u8 load[4], length = 0u;

            if ((warm & (1u << operand)) == 0u) continue;
            if (size == 2u) load[length++] = 0x66u;
            load[length++] = size == 0u ? 0xa0u : 0xa1u;
            load[length++] = (lib_u8)address;
            load[length++] = (lib_u8)(address >> 8u);
            if (!cache_run(fixture, load, length)) return LIB_FALSE;
        }
        fixture->cpu.data.esi = source;
        fixture->cpu.data.edi = destination;
        if (size == 2u) code[bytes++] = 0x66u;
        code[bytes++] = size == 0u ? 0xa6u : 0xa7u;
        if (!cache_run(fixture, code, bytes) ||
            fixture->instructions.data.timing_missed_reads != reads ||
            fixture->instructions.data.timing_data_misses != misses ||
            fixture->instructions.data.timing_unaligned_accesses != unaligned ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            ticks != 3u * reads + 5u * (misses - reads) + 3u * unaligned ||
            modeled != (reads == 1u || misses > reads)) {
            lib_c_printf("Compare timing size=%u position=%u same=%u warm=%u disabled=%u: reads=%u misses=%u ticks=%llu\n",
                (lib_u32)size, (lib_u32)position, (lib_u32)same, (lib_u32)warm, (lib_u32)disabled,
                fixture->instructions.data.timing_missed_reads,
                fixture->instructions.data.timing_data_misses, ticks);
            return LIB_FALSE;
        }
        if (!cache_run(fixture, (const lib_u8[]){0x90u}, 1u) ||
            fixture->instructions.data.timing_missed_reads != 0u ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            ticks != 0u || modeled) return LIB_FALSE;
    }
    {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        lib_u64 ticks = 999u;
        lib_bool modeled = LIB_TRUE;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        fixture->cpu.data.esi = 0x2100u;
        fixture->cpu.data.edi = 0x2200u;
        probe.fail_read = LIB_TRUE;
        probe.fail_read_at = 0x2200u;
        if (cache_run(fixture, (const lib_u8[]){0xa6u}, 1u) || !fixture->fault.valid ||
            fixture->instructions.data.timing_missed_reads != 1u ||
            fixture->instructions.data.timing_data_misses != 1u ||
            x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            ticks != 999u || !modeled) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_stack_group_timing(void)
{
    static const lib_u8 levels[] = {0u, 1u, 2u, 3u, 8u, 9u, 16u, 31u, 255u};

    for (lib_u8 enter = 0u; enter < 2u; ++enter)
    for (lib_u8 index = 0u; index < (enter ? sizeof(levels) : 1u); ++index)
    for (lib_u8 dword = 0u; dword < 2u; ++dword)
    for (lib_u8 offset = 0u; offset < 2u; ++offset)
    for (lib_u8 phase = 0u; phase < 5u; ++phase) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u8 level = levels[index] & 31u;
        const lib_u8 width = dword ? 4u : 2u;
        const lib_u8 count = enter ? (level == 0u ? 0u : level - 1u) : 8u;
        const lib_u16 start = enter ? 0x2400u + offset : 0x2200u + offset;
        const lib_u16 first = enter ? start - width * count : start;
        const lib_u16 last = enter ? start - 1u : start + width * count - 1u;
        const lib_u8 lines = count == 0u ? 0u : (last >> 4u) - (first >> 4u) + 1u;
        const lib_u8 nominal_lines = (count * width + 15u) / 16u;
        const lib_bool fail = phase == 4u;
        lib_u32 misses = 0u, unaligned = offset ? count : 0u;
        lib_u8 code[5], bytes = 0u;
        lib_u64 ticks = 999u;
        lib_bool modeled = LIB_TRUE;

        if (fail && count < 2u) continue;
        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        if (phase < 3u) fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        for (lib_u8 line = 0u; line < lines; ++line) {
            const lib_u16 address = (first & ~15u) + 16u * line;
            const lib_u8 load[] = {0xa0u, (lib_u8)address, (lib_u8)(address >> 8u)};

            if (phase == 1u || (phase == 2u && line == 0u)) {
                if (!cache_run(fixture, load, sizeof(load))) return LIB_FALSE;
            } else ++misses;
        }
        if (phase >= 3u) {
            misses = count;
            if (offset)
                for (lib_u8 item = 0u; item < count; ++item) {
                    const lib_u16 address = enter ? start - width * (item + 1u) :
                        start + (lib_u32)width * item;

                    if ((address & 15u) + width > 16u) ++misses;
                }
        }
        fixture->cpu.data.esp = enter ? 0x2800u : start;
        fixture->cpu.data.ebp = start;
        if (dword) code[bytes++] = 0x66u;
        code[bytes++] = enter ? 0xc8u : 0x61u;
        if (enter) {
            code[bytes++] = 0u;
            code[bytes++] = 0u;
            code[bytes++] = levels[index];
        }
        probe.fail_read = fail;
        probe.fail_read_at = enter ? start - 2u * width : start + width;
        if (cache_run(fixture, code, bytes) == fail) return LIB_FALSE;
        if (fail) {
            if (!fixture->fault.valid || fixture->instructions.data.timing_missed_reads != 1u ||
                x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
                ticks != 999u || !modeled) return LIB_FALSE;
            continue;
        }
        const lib_u64 cold = dword ? 15u : 7u;
        const lib_u64 expected = misses == 0u ? 0u : enter ? 6u * misses :
            misses >= nominal_lines ? cold + 5u * (misses - nominal_lines) :
                (cold * misses + nominal_lines - 1u) / nominal_lines;
        const lib_bool expected_model = misses != 0u && (offset || misses != nominal_lines);
        if (fixture->instructions.data.timing_data_misses != misses ||
            fixture->instructions.data.timing_unaligned_accesses != unaligned ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            ticks != expected + 3u * unaligned || modeled != expected_model) {
            lib_c_printf("Stack group enter=%u level=%u dword=%u offset=%u phase=%u: misses=%u/%u align=%u/%u ticks=%llu expected=%llu model=%u/%u\n",
                (lib_u32)enter, (lib_u32)levels[index], (lib_u32)dword, (lib_u32)offset, (lib_u32)phase,
                fixture->instructions.data.timing_data_misses, misses,
                fixture->instructions.data.timing_unaligned_accesses, unaligned,
                ticks, expected + 3u * unaligned, (lib_u32)modeled, (lib_u32)expected_model);
            return LIB_FALSE;
        }
        if (!cache_run(fixture, (const lib_u8[]){0x90u}, 1u) ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            ticks != 0u || modeled) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_descriptor_timing(void)
{
    static const struct {
        lib_u8 code[5], length, operand_miss, kind;
        lib_bool memory, stack, pair, loads;
    } cases[] = {
        {{0x8eu, 0xd8u}, 2u, 0u, 0u, LIB_FALSE, LIB_FALSE, LIB_FALSE, LIB_TRUE},
        {{0x8eu, 0x1eu, 0u, 0x20u}, 4u, 2u, 0u, LIB_TRUE, LIB_FALSE, LIB_FALSE, LIB_TRUE},
        {{0x07u}, 1u, 2u, 0u, LIB_FALSE, LIB_TRUE, LIB_FALSE, LIB_TRUE},
        {{0x17u}, 1u, 2u, 0u, LIB_FALSE, LIB_TRUE, LIB_FALSE, LIB_TRUE},
        {{0x1fu}, 1u, 2u, 0u, LIB_FALSE, LIB_TRUE, LIB_FALSE, LIB_TRUE},
        {{0x0fu, 0xa1u}, 2u, 2u, 0u, LIB_FALSE, LIB_TRUE, LIB_FALSE, LIB_TRUE},
        {{0x0fu, 0xa9u}, 2u, 2u, 0u, LIB_FALSE, LIB_TRUE, LIB_FALSE, LIB_TRUE},
        {{0xc4u, 0x06u, 0u, 0x20u}, 4u, 7u, 0u, LIB_TRUE, LIB_FALSE, LIB_TRUE, LIB_TRUE},
        {{0xc5u, 0x06u, 0u, 0x20u}, 4u, 7u, 0u, LIB_TRUE, LIB_FALSE, LIB_TRUE, LIB_TRUE},
        {{0x0fu, 0xb2u, 0x06u, 0u, 0x20u}, 5u, 7u, 0u, LIB_TRUE, LIB_FALSE, LIB_TRUE, LIB_TRUE},
        {{0x0fu, 0xb4u, 0x06u, 0u, 0x20u}, 5u, 7u, 0u, LIB_TRUE, LIB_FALSE, LIB_TRUE, LIB_TRUE},
        {{0x0fu, 0xb5u, 0x06u, 0u, 0x20u}, 5u, 7u, 0u, LIB_TRUE, LIB_FALSE, LIB_TRUE, LIB_TRUE},
        {{0x0fu, 0x02u, 0xc0u}, 3u, 0u, 0u, LIB_FALSE, LIB_FALSE, LIB_FALSE, LIB_FALSE},
        {{0x0fu, 0x02u, 0x06u, 0u, 0x20u}, 5u, 2u, 0u, LIB_TRUE, LIB_FALSE, LIB_FALSE, LIB_FALSE},
        {{0x0fu, 0x03u, 0xc0u}, 3u, 0u, 0u, LIB_FALSE, LIB_FALSE, LIB_FALSE, LIB_FALSE},
        {{0x0fu, 0x03u, 0x06u, 0u, 0x20u}, 5u, 3u, 0u, LIB_TRUE, LIB_FALSE, LIB_FALSE, LIB_FALSE},
        {{0x0fu, 0x00u, 0xd0u}, 3u, 0u, 1u, LIB_FALSE, LIB_FALSE, LIB_FALSE, LIB_FALSE},
        {{0x0fu, 0x00u, 0x16u, 0u, 0x20u}, 5u, 3u, 1u, LIB_TRUE, LIB_FALSE, LIB_FALSE, LIB_FALSE},
        {{0x0fu, 0x00u, 0xe0u}, 3u, 0u, 0u, LIB_FALSE, LIB_FALSE, LIB_FALSE, LIB_FALSE},
        {{0x0fu, 0x00u, 0x26u, 0u, 0x20u}, 5u, 4u, 0u, LIB_TRUE, LIB_FALSE, LIB_FALSE, LIB_FALSE},
        {{0x0fu, 0x00u, 0xe8u}, 3u, 0u, 0u, LIB_FALSE, LIB_FALSE, LIB_FALSE, LIB_FALSE},
        {{0x0fu, 0x00u, 0x2eu, 0u, 0x20u}, 5u, 4u, 0u, LIB_TRUE, LIB_FALSE, LIB_FALSE, LIB_FALSE},
        {{0x0fu, 0x00u, 0xd8u}, 3u, 0u, 2u, LIB_FALSE, LIB_FALSE, LIB_FALSE, LIB_FALSE},
        {{0x0fu, 0x00u, 0x1eu, 0u, 0x20u}, 5u, 0u, 2u, LIB_TRUE, LIB_FALSE, LIB_FALSE, LIB_FALSE}
    };

    for (lib_size row = 0u; row < sizeof(cases) / sizeof(cases[0]); ++row)
    for (lib_u8 dword = 0u; dword < 2u; ++dword)
    for (lib_u8 split = 0u; split < 2u; ++split)
    for (lib_u8 unaccessed = 0u; unaccessed < (cases[row].loads ? 2u : 1u); ++unaccessed)
    for (lib_u8 phase = 0u; phase < 6u; ++phase) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u8 width = dword ? 4u : 2u;
        const lib_u16 selector = cases[row].kind ? 0x18u : 0x10u;
        const lib_u16 operand = cases[row].stack ? 0x2300u : 0x2000u;
        const lib_u16 directory = split ? 0x300fu - (selector & 15u) : 0x3000u;
        const lib_u16 descriptor = directory + selector;
        const lib_u32 reads = (phase & 2u) && phase != 4u ? 0u : 1u;
        const lib_u32 descriptor_misses = reads * (split ? 2u : 1u);
        const lib_u32 operand_misses = (cases[row].memory || cases[row].stack) && (phase & 1u) == 0u;
        const lib_bool fail = phase >= 4u;
        lib_u8 code[6], bytes = 0u;
        lib_u64 ticks = 999u;
        lib_bool modeled = LIB_TRUE;

        if (phase == 5u && !split) continue;
        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        cpu_instruction_prepare_protected_exception(fixture, 0u);
        fixture->cpu.data.ds = fixture->cpu.data.ss;
        fixture->cpu.data.ds.sregtype = SREG_DATA;
        fixture->cpu.data.gdtr.base = directory;
        fixture->cpu.data.gdtr.limit = 31u;
        lib_memory_copy(fixture->memory + descriptor, (const lib_u8[]){0xffu, 0xffu, 0u, 0u, 0u,
            cases[row].kind == 1u ? 0x82u : cases[row].kind == 2u ? 0x89u :
                unaccessed ? 0x92u : 0x93u, 0u, 0u}, 8u);
        lib_memory_copy(fixture->memory + operand + (cases[row].pair ? width : 0u),
            &selector, sizeof(selector));
        for (lib_u8 part = 0u; part < 2u; ++part) {
            const lib_u16 address = part ? descriptor : operand;
            const lib_u8 load[] = {0x66u, 0xa1u, (lib_u8)address, (lib_u8)(address >> 8u)};

            if (!fail && (phase & (1u << part)) &&
                !cache_run(fixture, load, sizeof(load))) return LIB_FALSE;
        }
        fixture->cpu.data.eax = selector;
        fixture->cpu.data.esp = 0x2300u;
        if (dword) code[bytes++] = 0x66u;
        lib_memory_copy(code + bytes, cases[row].code, cases[row].length);
        bytes += cases[row].length;
        probe.fail_read = fail;
        probe.fail_read_at = descriptor + (phase == 5u ? 1u : 0u);
        if (cache_run(fixture, code, bytes) == fail) {
            lib_c_printf("Descriptor execution row=%u dword=%u split=%u phase=%u fault=%x\n",
                (lib_u32)row, (lib_u32)dword, (lib_u32)split, (lib_u32)phase,
                fixture->fault.exception_mask);
            return LIB_FALSE;
        }
        if (fail) {
            if (!fixture->fault.valid || fixture->instructions.data.timing_missed_descriptors != 0u ||
                fixture->instructions.data.timing_descriptor_misses != (lib_u32)(phase == 5u) ||
                x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
                ticks != 999u || !modeled) return LIB_FALSE;
            x86_core_cpu_state_reset(&fixture->execution);
            if (fixture->instructions.data.timing_descriptor_misses != 0u ||
                fixture->instructions.data.timing_missed_descriptors != 0u ||
                fixture->instructions.data.timing_missed_reads != 0u) return LIB_FALSE;
            continue;
        }
        const lib_u64 expected = cases[row].operand_miss * (lib_u64)operand_misses +
            (cases[row].kind == 2u ? 0u : 3u * reads + 5u * (descriptor_misses - reads)) +
            11u * unaccessed;
        if (fixture->instructions.data.timing_descriptor_misses != descriptor_misses ||
            fixture->instructions.data.timing_missed_descriptors != reads ||
            fixture->instructions.data.timing_data_misses != descriptor_misses + operand_misses ||
            fixture->instructions.data.timing_unaccessed_descriptors != unaccessed ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            ticks != expected || modeled != (split && reads != 0u)) {
            lib_c_printf("Descriptor timing row=%u dword=%u split=%u phase=%u: desc=%u/%u data=%u/%u ticks=%llu/%llu model=%u\n",
                (lib_u32)row, (lib_u32)dword, (lib_u32)split, (lib_u32)phase,
                fixture->instructions.data.timing_descriptor_misses, descriptor_misses,
                fixture->instructions.data.timing_data_misses, descriptor_misses + operand_misses,
                ticks, expected, (lib_u32)modeled);
            return LIB_FALSE;
        }
        const t_cpu before_cpu = fixture->cpu;
        const t_cpuins before_instructions = fixture->instructions;
        const x86_core_cpu_execution_context before_execution = fixture->execution;
        const lib_u32 data_reads = probe.data_reads, writes = fixture->writes;
        if (!x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            lib_memory_compare(&before_cpu, &fixture->cpu, sizeof(before_cpu)) ||
            lib_memory_compare(&before_instructions, &fixture->instructions, sizeof(before_instructions)) ||
            lib_memory_compare(&before_execution, &fixture->execution, sizeof(before_execution)) ||
            probe.data_reads != data_reads || fixture->writes != writes) return LIB_FALSE;
        if (!cache_run(fixture, (const lib_u8[]){0x90u}, 1u) ||
            fixture->instructions.data.timing_descriptor_misses != 0u ||
            fixture->instructions.data.timing_missed_descriptors != 0u ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            ticks != 0u || modeled) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_pair_timing(void)
{
    static const struct {
        lib_u8 opcode, secondary, extension, miss;
    } cases[] = {
        {0x62u, 0u, 0u, 7u}, {0xc4u, 0u, 0u, 7u}, {0xc5u, 0u, 0u, 7u},
        {0x0fu, 0xb2u, 0u, 7u}, {0x0fu, 0xb4u, 0u, 7u}, {0x0fu, 0xb5u, 0u, 7u},
        {0x0fu, 0x01u, 2u, 5u}, {0x0fu, 0x01u, 3u, 5u}
    };
    static const lib_u16 addresses[] = {0x2000u, 0x2002u, 0x200fu, 0x2fffu};

    for (lib_size row = 0u; row < sizeof(cases) / sizeof(cases[0]); ++row)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 dword = 0u; dword < 2u; ++dword)
    for (lib_u8 position = 0u; position < sizeof(addresses) / sizeof(addresses[0]); ++position)
    for (lib_u8 phase = 0u; phase < 6u; ++phase) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_bool table = cases[row].secondary == 0x01u, fail = phase == 5u;
        const lib_u8 first = table ? 2u : dword ? 4u : 2u;
        const lib_u8 second = table ? 4u : cases[row].opcode == 0x62u ? first : 2u;
        const lib_u16 address = addresses[position], end = address + first + second - 1u;
        const lib_bool split = (address >> 4u) != (end >> 4u);
        const lib_u32 unaligned = ((address & (first - 1u)) != 0u) +
            (lib_u32)(((address + first) & (second - 1u)) != 0u);
        const lib_u32 upper = cases[row].opcode == 0x62u ?
            (dword ? (lib_u32)LIB_INT32_MAX : (lib_u32)LIB_INT16_MAX) : 0u;
        lib_u32 misses = phase == 0u ? (split ? 2u : 1u) :
            phase == 1u || phase == 2u ? (lib_u32)split : 0u;
        lib_u8 code[9], bytes = 0u;
        lib_u64 ticks = 999u;
        lib_bool modeled = LIB_TRUE;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        if (phase != 4u) fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        lib_memory_copy(fixture->memory + address + first, &upper, second);
        for (lib_u8 part = 0u; part < 2u; ++part) {
            const lib_u16 warm_address = part ? end : address;
            const lib_u8 load[] = {0xa0u, (lib_u8)warm_address, (lib_u8)(warm_address >> 8u)};

            if (phase < 4u && (phase & (1u << part)) &&
                !cache_run(fixture, load, sizeof(load))) return LIB_FALSE;
        }
        if (phase == 4u) misses = 2u + ((address & 15u) + first > 16u) +
            (lib_u32)(((address + first) & 15u) + second > 16u);
        fixture->cpu.data.eax = 0u;
        fixture->cpu.data.cs.seg.exec.defsize = code32;
        if (code32) code[bytes++] = 0x67u;
        if (code32 != dword) code[bytes++] = 0x66u;
        code[bytes++] = cases[row].opcode;
        if (cases[row].secondary) code[bytes++] = cases[row].secondary;
        code[bytes++] = 0x06u | (cases[row].extension << 3u);
        code[bytes++] = (lib_u8)address;
        code[bytes++] = (lib_u8)(address >> 8u);
        probe.fail_read = fail;
        probe.fail_read_at = address + first;
        if (cache_run(fixture, code, bytes) == fail) return LIB_FALSE;
        if (fail) {
            if (!fixture->fault.valid ||
                x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
                ticks != 999u || !modeled) return LIB_FALSE;
            continue;
        }
        const lib_u64 expected = (misses ? cases[row].miss + 5u * (misses - 1u) : 0u) +
            3u * (lib_u64)unaligned;
        if (fixture->instructions.data.timing_data_misses != misses ||
            fixture->instructions.data.timing_unaligned_accesses != unaligned ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            ticks != expected || modeled != (misses > 1u)) {
            lib_c_printf("Pair timing row=%u code32=%u dword=%u position=%u phase=%u: misses=%u/%u align=%u/%u ticks=%llu/%llu\n",
                (lib_u32)row, (lib_u32)code32, (lib_u32)dword, (lib_u32)position, (lib_u32)phase,
                fixture->instructions.data.timing_data_misses, misses,
                fixture->instructions.data.timing_unaligned_accesses, unaligned, ticks, expected);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool cache_far_timing(void)
{
    for (lib_u8 mode = 0u; mode < 4u; ++mode)
    for (lib_u8 form = 0u; form < 7u; ++form)
    for (lib_u8 dword = 0u; dword < 2u; ++dword)
    for (lib_u8 unaccessed = 0u; unaccessed < (mode == 1u || mode == 2u ? 2u : 1u); ++unaccessed)
    for (lib_u8 phase = 0u; phase < 6u; ++phase) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_bool returning = form >= 4u, outer = mode == 2u, virtual_return = mode == 3u;
        const lib_bool protected_mode = mode != 0u;
        const lib_bool operand = returning || form == 2u || form == 3u;
        const lib_u8 width = dword ? 4u : 2u;
        const lib_u8 items = virtual_return ? 9u : form == 6u ? (outer ? 5u : 3u) : outer ? 4u : 2u;
        const lib_u8 lines = (items * width + 15u) / 16u;
        const lib_u8 descriptors = !protected_mode || virtual_return ? 0u : outer ? 2u : 1u;
        const lib_u16 selector = !protected_mode || virtual_return ? 0u : outer ? 0x0bu : 8u;
        const lib_u32 target = 0x100u;
        const lib_u16 address = returning ? 0x2300u : 0x2000u;
        const lib_u32 misses = operand && (phase & 1u) == 0u ? (returning ? lines : 1u) : 0u;
        const lib_u32 descriptor_misses = (phase & 2u) == 0u ? descriptors : 0u;
        const lib_bool fail = phase >= 4u;
        lib_u8 code[10], bytes = 0u;
        lib_u64 ticks = 999u;
        lib_bool modeled = LIB_TRUE;
        t_cpu after;

        if ((outer && !returning) || (virtual_return && (form != 6u || !dword)) ||
            (phase == 4u && !operand) || (phase == 5u && descriptors == 0u)) continue;
        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        if (protected_mode) {
            cpu_instruction_prepare_protected_exception(fixture, 0u);
            fixture->cpu.data.ds = fixture->cpu.data.ss;
            fixture->cpu.data.ds.sregtype = SREG_DATA;
            fixture->cpu.data.gdtr.base = 0x3000u;
            lib_memory_copy(fixture->memory + 0x3008u, (const lib_u8[]){0xffu, 0xffu, 0u, 0u, 0u,
                (lib_u8)((outer ? 0xfau : 0x9au) | !unaccessed), 0u, 0u}, 8u);
            lib_memory_copy(fixture->memory + 0x3010u, (const lib_u8[]){0xffu, 0xffu, 0u, 0u, 0u,
                (lib_u8)(0xf2u | !unaccessed), 0u, 0u}, 8u);
        }
        lib_memory_copy(fixture->memory + address, &target, width);
        lib_memory_copy(fixture->memory + address + width, &selector, sizeof(selector));
        if (returning) {
            lib_u8 slot = 2u;
            const lib_u32 flags = virtual_return ? 2u | VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL : 2u;
            const lib_u16 stack_selector = virtual_return ? 0u : 0x13u;

            if (form == 6u) {
                lib_memory_copy(fixture->memory + address + width * slot++, &flags, width);
            }
            if (outer || virtual_return) {
                lib_memory_copy(fixture->memory + address + width * slot++, &(lib_u32){0x2500u}, width);
                lib_memory_copy(fixture->memory + address + width * slot, &stack_selector, sizeof(stack_selector));
            }
        }
        if (!fail && (phase & 1u) && operand)
            for (lib_u8 line = 0u; line < (returning ? lines : 1u); ++line) {
                const lib_u16 warm_address = address + 16u * line;
                const lib_u8 load[] = {0xa0u, (lib_u8)warm_address, (lib_u8)(warm_address >> 8u)};

                if (!cache_run(fixture, load, sizeof(load))) return LIB_FALSE;
            }
        if (!fail && (phase & 2u))
            for (lib_u8 part = 0u; part < descriptors; ++part) {
                const lib_u16 warm_address = 0x3008u + 8u * part;
                const lib_u8 load[] = {0xa0u, (lib_u8)warm_address, (lib_u8)(warm_address >> 8u)};

                if (!cache_run(fixture, load, sizeof(load))) return LIB_FALSE;
            }
        fixture->cpu.data.esp = 0x2300u;
        if (dword) code[bytes++] = 0x66u;
        if (form < 2u) {
            code[bytes++] = form == 0u ? 0x9au : 0xeau;
            lib_memory_copy(code + bytes, &target, width);
            bytes += width;
            lib_memory_copy(code + bytes, &selector, sizeof(selector));
            bytes += sizeof(selector);
        } else if (!returning) {
            code[bytes++] = 0xffu;
            code[bytes++] = form == 2u ? 0x1eu : 0x2eu;
            code[bytes++] = 0u;
            code[bytes++] = 0x20u;
        } else {
            code[bytes++] = form == 4u ? 0xcbu : form == 5u ? 0xcau : 0xcfu;
            if (form == 5u) { code[bytes++] = 0u; code[bytes++] = 0u; }
        }
        probe.fail_read = fail;
        probe.fail_read_at = phase == 5u ? 0x3008u : address;
        fixture->cpu.data.eip = 0u;
        x86_core_cpu_execution_invalidate_prefetch(&fixture->execution);
        const lib_status status = cpu_instruction_run(fixture, code, bytes, &after);
        if (fail) {
            if (!fixture->fault.valid ||
                x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
                ticks != 999u || !modeled) return LIB_FALSE;
            continue;
        }
        const lib_u64 operand_ticks = !misses ? 0u : returning ?
            (virtual_return ? 19u : form == 6u ? (outer ? 13u : 8u) : protected_mode ? 6u : 8u) :
            protected_mode ? 7u : form == 2u ? 8u : 9u;
        const lib_u64 expected = operand_ticks + 3u * descriptor_misses + 11u * unaccessed * descriptors;
        if (status != LIB_STATUS_OK || fixture->fault.valid || fixture->delivered_exception.valid ||
            after.data.eip != target ||
            fixture->instructions.data.timing_data_misses != misses + descriptor_misses ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            ticks != expected || modeled != (virtual_return && misses != 0u)) {
            lib_c_printf("Far timing mode=%u form=%u dword=%u phase=%u: miss=%u/%u desc=%u ticks=%llu/%llu model=%u fault=%x\n",
                (lib_u32)mode, (lib_u32)form, (lib_u32)dword, (lib_u32)phase,
                fixture->instructions.data.timing_data_misses, misses + descriptor_misses,
                fixture->instructions.data.timing_descriptor_misses, ticks, expected,
                (lib_u32)modeled, fixture->fault.exception_mask);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool cache_call_gate_timing(void)
{
    static const lib_u8 counts[] = {0u, 1u, 4u, 5u, 8u, 9u, 31u};

    for (lib_u8 dword = 0u; dword < 2u; ++dword)
    for (lib_u8 inner = 0u; inner < 2u; ++inner)
    for (lib_u8 indirect = 0u; indirect < 2u; ++indirect)
    for (lib_u8 row = 0u; row < sizeof(counts); ++row)
    for (lib_u8 offset = 0u; offset < 2u; ++offset)
    for (lib_u8 warm = 0u; warm < 16u; ++warm)
    for (lib_u8 failure = 0u; failure < (inner && warm == 0u && offset == 0u ? 4u : 1u); ++failure) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u8 width = dword ? 4u : 2u;
        const lib_u8 count = inner ? counts[row] : 0u;
        const lib_u8 lines = count == 0u ? 0u : (offset + count * width + 15u) / 16u;
        const lib_u32 pointer = indirect && (warm & 1u) == 0u;
        const lib_u32 descriptors = (warm & 2u) ? 0u : inner ? 3u : 2u;
        const lib_u32 tss = inner && (warm & 4u) == 0u;
        const lib_u32 parameters = (warm & 8u) ? 0u : lines;
        const lib_u64 expected = 7u * pointer + 3u * descriptors + 8u * tss + 4u * parameters +
            (offset ? 3u * (inner ? count : 2u) : 0u);
        lib_u8 code[8], bytes = 0u;
        lib_u64 ticks = 999u;
        lib_bool modeled = LIB_TRUE;
        t_cpu after;

        if (failure == 2u && count == 0u) continue;
        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        cpu_instruction_prepare_protected_exception(fixture, 3u);
        fixture->cpu.data.ds = fixture->cpu.data.ss;
        fixture->cpu.data.ds.sregtype = SREG_DATA;
        fixture->cpu.data.gdtr.base = 0x3000u;
        fixture->cpu.data.gdtr.limit = 0x37u;
        lib_memory_copy(fixture->memory + 0x3010u, (const lib_u8[]){0xffu, 0xffu, 0u, 0u, 0u,
            inner ? 0x9bu : 0xfbu, 0u, 0u}, 8u);
        lib_memory_copy(fixture->memory + 0x3020u,
            (const lib_u8[]){0xffu, 0xffu, 0u, 0u, 0u, 0x93u, 0u, 0u}, 8u);
        lib_memory_copy(fixture->memory + 0x3030u, (const lib_u8[]){0u, 1u, 0x10u, 0u,
            counts[row], dword ? 0xecu : 0xe4u, 0u, 0u}, 8u);
        fixture->cpu.data.tr = (t_cpu_data_sreg){.flagValid = LIB_TRUE,
            .sregtype = SREG_TR, .selector = 0x38u, .base = 0x2500u, .limit = 0x67u,
            .sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY};
        lib_memory_copy(fixture->memory + 0x2504u, &(lib_u32){0x2800u}, sizeof(lib_u32));
        lib_memory_copy(fixture->memory + 0x2508u, &(lib_u16){0x20u}, sizeof(lib_u16));
        lib_memory_copy(fixture->memory + 0x2000u + width, &(lib_u16){0x33u}, sizeof(lib_u16));
        for (lib_u8 parameter = 0u; parameter < count; ++parameter)
            lib_memory_copy(fixture->memory + 0x2300u + offset + width * parameter,
                &(lib_u32){parameter + 1u}, width);
        for (lib_u8 part = 0u; part < 4u; ++part) {
            const lib_u16 starts[] = {0x2000u, 0x3010u, 0x2500u, 0x2300u};
            const lib_u8 length = part == 1u ? 3u : part == 3u ? lines : 1u;

            if ((warm & (1u << part)) == 0u) continue;
            for (lib_u8 line = 0u; line < length; ++line) {
                const lib_u16 address = starts[part] + 16u * line;
                const lib_u8 load[] = {0xa0u, (lib_u8)address, (lib_u8)(address >> 8u)};

                if (!cache_run(fixture, load, sizeof(load))) return LIB_FALSE;
            }
        }
        fixture->cpu.data.esp = 0x2300u + offset;
        if (dword) code[bytes++] = 0x66u;
        if (indirect) {
            code[bytes++] = 0xffu;
            code[bytes++] = 0x1eu;
            code[bytes++] = 0u;
            code[bytes++] = 0x20u;
        } else {
            code[bytes++] = 0x9au;
            for (lib_u8 byte = 0u; byte < width; ++byte) code[bytes++] = 0u;
            code[bytes++] = 0x33u;
            code[bytes++] = 0u;
        }
        fixture->cpu.data.eip = 0u;
        x86_core_cpu_execution_invalidate_prefetch(&fixture->execution);
        probe.fail_read = failure == 1u || failure == 2u;
        probe.fail_read_at = failure == 1u ? 0x2504u : 0x2300u;
        probe.fail_write = failure == 3u;
        const lib_status status = cpu_instruction_run(fixture, code, bytes, &after);
        if (failure) {
            if (!fixture->fault.valid ||
                x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
                ticks != 999u || !modeled || probe.lock_active || probe.lock_protocol_error)
                return LIB_FALSE;
            continue;
        }
        if (status != LIB_STATUS_OK || fixture->fault.valid || fixture->delivered_exception.valid ||
            after.data.eip != 0x100u || after.data.cs.dpl != (inner ? 0u : 3u) ||
            after.data.esp != (inner ? 0x2800u : 0x2300u + offset) - width * (inner ? 4u + count : 2u) ||
            fixture->instructions.data.timing_data_misses != pointer + descriptors + tss + parameters ||
            fixture->instructions.data.timing_pair_misses != pointer ||
            fixture->instructions.data.timing_tss_field_misses != tss ||
            fixture->instructions.data.timing_descriptor_misses != descriptors ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            ticks != expected || modeled != (offset && parameters != 0u)) {
            lib_c_printf("Call gate dword=%u inner=%u indirect=%u count=%u warm=%u: miss=%u/%u pair=%u tss=%u desc=%u ticks=%llu/%llu fault=%x eip=%x\n",
                (lib_u32)dword, (lib_u32)inner, (lib_u32)indirect, (lib_u32)count, (lib_u32)warm,
                fixture->instructions.data.timing_data_misses, pointer + descriptors + tss + parameters,
                fixture->instructions.data.timing_pair_misses, fixture->instructions.data.timing_tss_field_misses,
                fixture->instructions.data.timing_descriptor_misses, ticks, expected,
                fixture->fault.exception_mask, after.data.eip);
            return LIB_FALSE;
        }
        if (count) {
            lib_u32 value = 0u;

            lib_memory_copy(&value, fixture->memory + after.data.esp + 2u * width, width);
            if (value != 1u) return LIB_FALSE;
        }
        if (!cache_run(fixture, (const lib_u8[]){0x90u}, 1u) ||
            fixture->instructions.data.timing_pair_misses != 0u ||
            fixture->instructions.data.timing_tss_field_misses != 0u ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            ticks != 0u || modeled) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_task_timing(void)
{
    static const lib_u8 descriptor_counts[] = {6u, 8u, 2u, 9u};
    static const lib_u8 cold_state[] = {16u, 34u, 34u, 34u};
    static const lib_u8 partial_state[] = {11u, 29u, 29u, 29u};

    for (lib_u8 mode = 0u; mode < 4u; ++mode)
    for (lib_u8 form = 0u; form < 9u; ++form)
    for (lib_u8 warm = 0u; warm < (form >= 5u ? 32u : 16u); ++warm)
    for (lib_u8 failure = 0u; failure < (warm == 0u ? 6u : 1u); ++failure) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_bool task32 = mode != 0u, vm86 = mode == 2u, ldt = mode == 3u;
        const lib_bool gated = form >= 5u;
        const lib_bool indirect = form == 1u || form == 3u || form >= 7u, returning = form == 4u;
        const lib_u8 lines = task32 ? 6u : 3u;
        const lib_u32 pointer = indirect && (warm & 1u) == 0u;
        const lib_u32 gate = gated && (warm & 16u) == 0u;
        const lib_u32 descriptors = ((warm & 2u) ? 0u : descriptor_counts[mode]) + gate;
        const lib_u8 residency = (warm >> 2u) & 3u;
        const lib_u32 state = residency == 1u ? 0u : residency == 0u ? lines : lines - 1u;
        const lib_u32 backlink = returning && residency != 1u;
        const lib_u64 expected = 7u * pointer + 3u * (descriptors - (gate && (warm & 2u) == 0u)) +
            (state == 0u ? 0u : state == lines ? cold_state[mode] : partial_state[mode]) + backlink;
        lib_u8 code[5], bytes = 0u;
        lib_u64 ticks = 999u;
        lib_bool modeled = LIB_TRUE;
        t_cpu after;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        cpu_instruction_prepare_protected_exception(fixture, 0u);
        fixture->cpu.data.ds = fixture->cpu.data.ss;
        fixture->cpu.data.ds.sregtype = SREG_DATA;
        fixture->cpu.data.gdtr.base = 0x3000u;
        fixture->cpu.data.gdtr.limit = gated ? 0xa7u : ldt ? 0x97u : 0x87u;
        if (gated)
            lib_memory_copy(fixture->memory + 0x30a0u,
                (const lib_u8[]){0u, 0u, 0x80u, 0u, 0u, 0x85u, 0u, 0u}, 8u);
        for (lib_u8 descriptor = 1u; descriptor <= 6u; ++descriptor)
            lib_memory_copy(fixture->memory + (ldt ? 0x3200u : 0x3000u) + 16u * descriptor,
                (const lib_u8[]){0xffu, 0xffu, 0u, 0u, 0u, descriptor == 1u ? 0x9bu : 0x93u, 0u, 0u}, 8u);
        if (ldt)
            lib_memory_copy(fixture->memory + 0x3090u,
                (const lib_u8[]){0x67u, 0u, 0u, 0x32u, 0u, 0x82u, 0u, 0u}, 8u);
        lib_memory_copy(fixture->memory + 0x3070u,
            (const lib_u8[]){0x67u, 0u, 0u, 0x24u, 0u, 0x8bu, 0u, 0u}, 8u);
        lib_memory_copy(fixture->memory + 0x3080u, (const lib_u8[]){task32 ? 0x67u : 0x2bu,
            0u, 0u, 0x26u, 0u, (lib_u8)((task32 ? 0x89u : 0x81u) | (returning ? 2u : 0u)), 0u, 0u}, 8u);
        fixture->cpu.data.tr = (t_cpu_data_sreg){.flagValid = LIB_TRUE,
            .sregtype = SREG_TR, .selector = 0x70u, .base = 0x2400u, .limit = 0x67u,
            .sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY};
        lib_memory_copy(fixture->memory + 0x2400u, &(lib_u16){0x80u}, sizeof(lib_u16));
        lib_memory_copy(fixture->memory + 0x2002u, &(lib_u16){gated ? 0xa0u : 0x80u}, sizeof(lib_u16));
        if (task32) {
            const lib_u32 image[] = {0u, 0x100u, vm86 ? 2u | VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL : 2u,
                0u, 0u, 0u, 0u, 0x2800u, 0u, 0u, 0u,
                vm86 ? 0u : 0x40u | (ldt ? 4u : 0u), vm86 ? 0u : 0x10u | (ldt ? 4u : 0u),
                vm86 ? 0u : 0x20u | (ldt ? 4u : 0u), vm86 ? 0u : 0x30u | (ldt ? 4u : 0u),
                vm86 ? 0u : 0x50u | (ldt ? 4u : 0u), vm86 ? 0u : 0x60u | (ldt ? 4u : 0u),
                ldt ? 0x90u : 0u};

            lib_memory_copy(fixture->memory + 0x261cu, image, sizeof(image));
        } else {
            const lib_u16 image[] = {0x100u, 2u, 0u, 0u, 0u, 0u, 0x2800u,
                0u, 0u, 0u, 0x40u, 0x10u, 0x20u, 0x30u, 0u};

            lib_memory_copy(fixture->memory + 0x260eu, image, sizeof(image));
        }
        for (lib_u8 part = 0u; part < 3u; ++part) {
            const lib_u16 starts[] = {0x2000u, 0x3010u, task32 ? 0x2610u : 0x2600u};
            const lib_u8 count = part == 0u ? 1u : part == 1u ? (ldt ? 9u : 8u) :
                residency == 1u ? lines : 1u;

            if (part == 2u ? residency == 0u : (warm & (1u << part)) == 0u) continue;
            for (lib_u8 line = 0u; line < count + (part == 2u && returning && residency == 1u); ++line) {
                const lib_u16 address = line == count ? 0x2400u : part == 1u && ldt ?
                    (line < 3u ? 0x3070u + 16u * line : 0x3210u + 16u * (line - 3u)) :
                    part == 2u && residency == 3u ? starts[part] + 16u * (lines - 1u) :
                    starts[part] + 16u * line;
                const lib_u8 load[] = {0xa0u, (lib_u8)address, (lib_u8)(address >> 8u)};

                if (!cache_run(fixture, load, sizeof(load))) return LIB_FALSE;
            }
        }
        if (gated && (warm & 16u) != 0u &&
            !cache_run(fixture, (const lib_u8[]){0xa0u, 0xa0u, 0x30u}, 3u)) return LIB_FALSE;
        fixture->cpu.data.esp = 0x2300u;
        if (returning) {
            fixture->cpu.data.eflags |= VCPU_EFLAGS_NT;
            code[bytes++] = 0xcfu;
        } else if (indirect) {
            code[bytes++] = 0xffu;
            code[bytes++] = form == 1u || form == 7u ? 0x2eu : 0x1eu;
            code[bytes++] = 0u;
            code[bytes++] = 0x20u;
        } else {
            code[bytes++] = form == 0u || form == 5u ? 0xeau : 0x9au;
            code[bytes++] = 0u;
            code[bytes++] = 0u;
            code[bytes++] = gated ? 0xa0u : 0x80u;
            code[bytes++] = 0u;
        }
        fixture->cpu.data.eip = 0u;
        x86_core_cpu_execution_invalidate_prefetch(&fixture->execution);
        probe.fail_read = failure != 0u && failure != 5u;
        probe.fail_read_at = failure == 1u ? 0x3080u : failure == 2u ? 0x3070u :
            failure == 3u ? (task32 ? 0x261cu : 0x260eu) : task32 ? 0x2660u : 0x2620u;
        probe.fail_write = failure == 5u;
        const lib_status status = cpu_instruction_run(fixture, code, bytes, &after);
        if (failure) {
            const lib_u32 accepted_state = failure <= 3u ? 0u : failure == 4u ? lines - 1u : lines;
            const lib_u32 accepted_descriptors = (failure == 1u ? 0u : failure == 2u ? 1u : 2u) + gate;

            if (!fixture->fault.valid ||
                fixture->instructions.data.timing_data_misses != pointer + backlink +
                    accepted_descriptors + accepted_state ||
                x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
                ticks != 999u || !modeled || probe.lock_active || probe.lock_protocol_error)
                return LIB_FALSE;
            continue;
        }
        if (status != LIB_STATUS_OK || fixture->fault.valid || fixture->delivered_exception.valid ||
            !fixture->execution.instruction_task_switched || after.data.eip != 0x100u ||
            after.data.tr.selector != 0x80u || ((after.data.eflags & VCPU_EFLAGS_VM) != 0u) != vm86 ||
            fixture->instructions.data.timing_data_misses != pointer + descriptors + state + backlink ||
            fixture->instructions.data.timing_descriptor_misses != descriptors ||
            fixture->instructions.data.timing_pair_misses != pointer ||
            fixture->instructions.data.timing_tss_field_misses != backlink ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            ticks != expected || modeled != (pointer + descriptors + state + backlink != 0u)) {
            lib_c_printf("Task memory mode=%u form=%u warm=%u: miss=%u/%u desc=%u pair=%u field=%u ticks=%llu/%llu model=%u fault=%x eip=%x\n",
                (lib_u32)mode, (lib_u32)form, (lib_u32)warm,
                fixture->instructions.data.timing_data_misses, pointer + descriptors + state + backlink,
                fixture->instructions.data.timing_descriptor_misses, fixture->instructions.data.timing_pair_misses,
                fixture->instructions.data.timing_tss_field_misses, ticks, expected,
                (lib_u32)modeled, fixture->fault.exception_mask, after.data.eip);
            return LIB_FALSE;
        }
        const t_cpu before_cpu = fixture->cpu;
        const t_cpuins before_instructions = fixture->instructions;
        const x86_core_cpu_execution_context before_execution = fixture->execution;
        const lib_u32 data_reads = probe.data_reads, writes = fixture->writes;
        if (!x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            lib_memory_compare(&before_cpu, &fixture->cpu, sizeof(before_cpu)) ||
            lib_memory_compare(&before_instructions, &fixture->instructions, sizeof(before_instructions)) ||
            lib_memory_compare(&before_execution, &fixture->execution, sizeof(before_execution)) ||
            data_reads != probe.data_reads || writes != fixture->writes) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_translation_timing(void)
{
    static const lib_u64 walk_ticks[] = {13u, 21u, 28u};

    for (lib_u8 store = 0u; store < 2u; ++store)
    for (lib_u8 updates = 0u; updates < 3u; ++updates)
    for (lib_u8 variant = 0u; variant < 3u; ++variant) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_bool cached = variant == 1u;
        const lib_bool raced = variant == 2u;
        const lib_u8 read[] = {0xa1u, 0u, 0x20u};
        const lib_u8 write[] = {0xc6u, 0x06u, 0u, 0x20u, 0x5au};
        lib_u32 entry = 0x11023u;
        lib_u64 ticks = 999u;

        /* Cached/raced differential costs are L2, not cold E.2 source rows. */
        if ((cached && updates != 0u) || (raced && (updates != 2u || store))) continue;
        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        probe.cache_page_tables = cached;
        fixture->cpu.data.cr0 = VCPU_CR0_PE | VCPU_CR0_PG;
        fixture->cpu.data.cr3 = 0x10000u;
        fixture->cpu.data.ds.selector = 0x10u;
        lib_memory_copy(fixture->memory + 0x10000u, &entry, sizeof(entry));
        entry = 0x23u;
        lib_memory_copy(fixture->memory + 0x11000u, &entry, sizeof(entry));
        entry = 0x2063u;
        lib_memory_copy(fixture->memory + 0x11008u, &entry, sizeof(entry));
        if (!cache_run(fixture, (const lib_u8[]){0x90u}, 1u)) return LIB_FALSE;
        entry = updates == 2u ? 0x11003u : 0x11023u;
        lib_memory_copy(fixture->memory + 0x10000u, &entry, sizeof(entry));
        entry = store ? (updates ? 0x2023u : 0x2063u) : (updates ? 0x2003u : 0x2023u);
        lib_memory_copy(fixture->memory + 0x11008u, &entry, sizeof(entry));
        if (raced) {
            probe.replace_directory = LIB_TRUE;
            probe.second_directory = 0x11023u;
        }
        if (!cache_run(fixture, store ? write : read, store ? sizeof(write) : sizeof(read)))
            return LIB_FALSE;
        const t_cpuins_data *data = &fixture->instructions.data;
        const lib_u8 actual_updates = raced ? 1u : updates;
        lib_bool modeled = LIB_FALSE;
        if (data->timing_tlb_walks[updates] != 1u ||
            data->timing_tlb_walks[(updates + 1u) % 3u] != 0u ||
            data->timing_tlb_walks[(updates + 2u) % 3u] != 0u ||
            data->timing_table_misses != (cached ? 0u : 2u) ||
            data->timing_tlb_updates != actual_updates ||
            data->timing_data_misses != (lib_u32)!store ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            modeled != (cached || raced) ||
            ticks != walk_ticks[updates] + (store ? 0u : 2u) - (cached ? 4u : raced ? 2u : 0u)) {
            lib_c_printf("Translation timing %u/%u/%u: walks=%u,%u,%u tables=%u updates=%u data=%u ticks=%llu\n",
                (lib_u32)store, (lib_u32)updates, (lib_u32)variant,
                data->timing_tlb_walks[0], data->timing_tlb_walks[1], data->timing_tlb_walks[2],
                data->timing_table_misses, data->timing_tlb_updates,
                data->timing_data_misses, ticks);
            return LIB_FALSE;
        }
        ticks = 999u;
        if (!cache_run(fixture, store ? write : read, store ? sizeof(write) : sizeof(read)) ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, LIB_NULL) || ticks != 0u)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_translation_failure_timing(void)
{
    for (lib_u8 failure = 0u; failure < 4u; ++failure) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        lib_u32 entry = 0x11023u;
        lib_u64 ticks = 999u;
        lib_bool modeled = LIB_TRUE;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 = VCPU_CR0_PE | VCPU_CR0_PG;
        fixture->cpu.data.cr3 = 0x10000u;
        fixture->cpu.data.ds.selector = 0x10u;
        lib_memory_copy(fixture->memory + 0x10000u, &entry, sizeof(entry));
        entry = 0x23u;
        lib_memory_copy(fixture->memory + 0x11000u, &entry, sizeof(entry));
        if (!cache_run(fixture, (const lib_u8[]){0x90u}, 1u)) return LIB_FALSE;
        entry = 0x11003u;
        lib_memory_copy(fixture->memory + 0x10000u, &entry, sizeof(entry));
        entry = 0x2003u;
        lib_memory_copy(fixture->memory + 0x11010u, &entry, sizeof(entry));
        probe.fail_table_read = failure == 0u;
        probe.fail_second_read = failure == 1u;
        probe.fail_page_write = failure == 2u;
        probe.fail_read = failure == 3u;
        if (cache_run(fixture, (const lib_u8[]){0xa1u, 0u, 0x40u}, 3u) ||
            !fixture->fault.valid ||
            fixture->instructions.data.timing_tlb_updates !=
                (failure == 0u ? 1u : failure == 3u ? 2u : 0u) ||
            fixture->instructions.data.timing_tlb_walks[0] != 0u ||
            fixture->instructions.data.timing_tlb_walks[1] != 0u ||
            fixture->instructions.data.timing_tlb_walks[2] != (lib_u32)(failure == 3u) ||
            probe.lock_active || probe.lock_protocol_error ||
            x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            ticks != 999u || !modeled)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_translation_residency_timing(void)
{
    static const lib_u64 cold_ticks[] = {13u, 21u, 28u};
    static const lib_u8 warm_mask[] = {0u, 3u, 1u, 2u, 0u};

    for (lib_u8 store = 0u; store < 2u; ++store)
    for (lib_u8 updates = 0u; updates < 3u; ++updates)
    for (lib_u8 residency = 0u; residency < 5u; ++residency) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u8 mask = warm_mask[residency];
        const lib_u8 hits = (mask & 1u) + ((mask >> 1u) & 1u);
        lib_u32 entry = 0x11023u;
        lib_u64 ticks = 999u;
        lib_bool modeled = LIB_FALSE;

        cpu_instruction_prepare_with_bus(fixture, X86_CORE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 = 0u;
        fixture->cpu.data.ds.limit = LIB_UINT32_MAX;
        probe.cache_page_tables = residency != 0u;
        lib_memory_copy(fixture->memory + 0x10000u, &entry, sizeof(entry));
        entry = 0x23u;
        lib_memory_copy(fixture->memory + 0x11000u, &entry, sizeof(entry));
        entry = updates == 2u ? 0x11003u : 0x11023u;
        lib_memory_copy(fixture->memory + 0x10010u, &entry, sizeof(entry));
        entry = store ? (updates ? 0x2023u : 0x2063u) : (updates ? 0x2003u : 0x2023u);
        lib_memory_copy(fixture->memory + 0x11010u, &entry, sizeof(entry));
        /* Prime through actual guest loads before enabling paging. Code and
         * data use different directory/PTE lines, so code warmup cannot
         * accidentally turn either intended cold entry into a hit. */
        if ((mask & 1u) && !cache_run(fixture,
                (const lib_u8[]){0x67u, 0xa1u, 0x10u, 0u, 1u, 0u}, 6u)) return LIB_FALSE;
        if ((mask & 2u) && !cache_run(fixture,
                (const lib_u8[]){0x67u, 0xa1u, 0x10u, 0x10u, 1u, 0u}, 6u)) return LIB_FALSE;
        fixture->cpu.data.cr0 = VCPU_CR0_PE | VCPU_CR0_PG;
        fixture->cpu.data.cr3 = 0x10000u;
        fixture->cpu.data.ds.selector = 0x10u;
        fixture->cpu.data.ds.base = 0x01000000u;
        if (!cache_run(fixture, (const lib_u8[]){0x90u}, 1u)) return LIB_FALSE;
        const lib_u8 read[] = {0xa1u, 0u, 0x40u};
        const lib_u8 write[] = {0xc6u, 0x06u, 0u, 0x40u, 0x5au};
        if (!cache_run(fixture, store ? write : read, store ? sizeof(write) : sizeof(read)))
            return LIB_FALSE;
        const t_cpuins_data *data = &fixture->instructions.data;
        if (data->timing_table_misses != (lib_u32)(2u - hits) ||
            data->timing_tlb_walks[updates] != 1u || data->timing_tlb_updates != updates ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            ticks != cold_ticks[updates] - 2u * hits + (store ? 0u : 2u) ||
            modeled != (hits != 0u) || probe.lock_active || probe.lock_protocol_error) {
            lib_c_printf("Translation residency store=%u updates=%u residency=%u: misses=%u ticks=%llu modeled=%u\n",
                (lib_u32)store, (lib_u32)updates, (lib_u32)residency,
                data->timing_table_misses, ticks, (lib_u32)modeled);
            return LIB_FALSE;
        }
        const t_cpu before_cpu = fixture->cpu;
        const t_cpuins before_instructions = fixture->instructions;
        const x86_core_cpu_execution_context before_execution = fixture->execution;
        const lib_u32 reads = probe.data_reads, writes = fixture->writes;
        if (!x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            lib_memory_compare(&before_cpu, &fixture->cpu, sizeof(before_cpu)) ||
            lib_memory_compare(&before_instructions, &fixture->instructions, sizeof(before_instructions)) ||
            lib_memory_compare(&before_execution, &fixture->execution, sizeof(before_execution)) ||
            probe.data_reads != reads || fixture->writes != writes)
            return LIB_FALSE;
        modeled = LIB_TRUE;
        if (!cache_run(fixture, store ? write : read, store ? sizeof(write) : sizeof(read)) ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks, &modeled) ||
            ticks != 0u || modeled) return LIB_FALSE;
    }
    return LIB_TRUE;
}

int main(void)
{
    if (!cache_test_geometry()) { lib_c_printf("Cache geometry failed\n"); return 1; }
    if (!cache_test_forms()) { lib_c_printf("Cache forms failed\n"); return 1; }
    if (!cache_test_observation_and_fetch()) { lib_c_printf("Cache observation failed\n"); return 1; }
    if (!cache_normal_access()) { lib_c_printf("Cache normal access failed\n"); return 1; }
    if (!cache_replacement_matrix()) { lib_c_printf("Cache replacement failed\n"); return 1; }
    if (!cache_locked_operands()) { lib_c_printf("Cache locked operands failed\n"); return 1; }
    if (!cache_page_attributes()) { lib_c_printf("Cache page attributes failed\n"); return 1; }
    if (!cache_page_update_masks()) { lib_c_printf("Cache page update masks failed\n"); return 1; }
    if (!cache_page_second_values()) { lib_c_printf("Cache page second values failed\n"); return 1; }
    if (!cache_page_second_permissions()) { lib_c_printf("Cache page second permissions failed\n"); return 1; }
    if (!cache_page_split_redirect()) { lib_c_printf("Cache page split redirect failed\n"); return 1; }
    if (!cache_failed_publication()) { lib_c_printf("Cache publication failed\n"); return 1; }
    if (!cache_flush_operations()) { lib_c_printf("Cache external operations failed\n"); return 1; }
    if (!cache_flush_privilege()) { lib_c_printf("Cache privilege failed\n"); return 1; }
    if (!cache_flush_failed_fetch()) { lib_c_printf("Cache flush fetch failed\n"); return 1; }
    if (!cache_snoop_write()) { lib_c_printf("Cache snoop failed\n"); return 1; }
    if (!cache_reset_and_snoop_bounds()) { lib_c_printf("Cache reset/bounds failed\n"); return 1; }
    if (!cache_fetch_timing()) { lib_c_printf("Cache fetch timing failed\n"); return 1; }
    if (!cache_access_timing()) { lib_c_printf("Cache access timing failed\n"); return 1; }
    if (!cache_integer_timing()) { lib_c_printf("Cache integer timing failed\n"); return 1; }
    if (!cache_integer_alignment_timing()) { lib_c_printf("Cache integer alignment failed\n"); return 1; }
    if (!cache_split_timing()) { lib_c_printf("Cache split timing failed\n"); return 1; }
    if (!cache_stack_string_timing()) { lib_c_printf("Cache stack/string timing failed\n"); return 1; }
    if (!cache_compare_timing()) { lib_c_printf("Cache compare timing failed\n"); return 1; }
    if (!cache_stack_group_timing()) { lib_c_printf("Cache stack group timing failed\n"); return 1; }
    if (!cache_descriptor_timing()) { lib_c_printf("Cache descriptor timing failed\n"); return 1; }
    if (!cache_pair_timing()) { lib_c_printf("Cache pair timing failed\n"); return 1; }
    if (!cache_far_timing()) { lib_c_printf("Cache far timing failed\n"); return 1; }
    if (!cache_call_gate_timing()) { lib_c_printf("Cache call gate timing failed\n"); return 1; }
    if (!cache_task_timing()) { lib_c_printf("Cache task timing failed\n"); return 1; }
    if (!cache_translation_timing()) { lib_c_printf("Cache translation timing failed\n"); return 1; }
    if (!cache_translation_failure_timing()) { lib_c_printf("Cache translation failure timing failed\n"); return 1; }
    if (!cache_translation_residency_timing()) { lib_c_printf("Cache translation residency timing failed\n"); return 1; }
    lib_c_printf("CPU cache state and access: OK\n");
    return 0;
}
