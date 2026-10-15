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
        if (probe->fail_read) return LIB_STATUS_IO_ERROR;
    }
    if (!observe_only && provenance == X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ) {
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
        if (!x86_core_cpu_instruction_lexeme_scan(code, bytes,
                X86_CORE_CPU_PROFILE_80486SX, (mode & 1u) != 0u, &lexeme) ||
            !cache_run(fixture, code, bytes) || fixture->cpu.data.eflags != flags ||
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
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &operand) || operand != 2u)
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
    for (lib_u8 form = 0u; form < 3u; ++form)
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
        code[bytes++] = form == 2u ? (size ? 0xc7u : 0xc6u) :
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
            const lib_bool allocated = !split || store || warm;
            lib_u64 ticks = 999u;

            if (!cache_run(fixture, code, bytes) ||
                fixture->instructions.data.timing_data_misses != misses ||
                fixture->instructions.data.timing_unaligned_accesses != (lib_u32)unaligned ||
                fixture->instructions.data.timing_locked_reads != 0u ||
                x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks) != allocated ||
                ticks != (allocated ? 2u * misses + 3u * unaligned : 999u)) {
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
                !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks) || ticks != 0u)
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
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks) || ticks != 0u)
            return LIB_FALSE;
    }
    {
        cpu_instruction_fixture fixture;
        lib_u64 ticks = 999u;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        if (x86_core_cpu_80486_memory_penalty(LIB_NULL, &ticks) ||
            x86_core_cpu_80486_memory_penalty(&fixture.execution, LIB_NULL) ||
            x86_core_cpu_80486_memory_penalty(&fixture.execution, &ticks) || ticks != 999u)
            return LIB_FALSE;
        fixture.execution.cpu_profile = X86_CORE_CPU_PROFILE_80386;
        if (!cache_run(&fixture, (const lib_u8[]){0x90u}, 1u) ||
            x86_core_cpu_80486_memory_penalty(&fixture.execution, &ticks) || ticks != 999u)
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
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks) || ticks != expected ||
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

            if (!x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks) || ticks != expected ||
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
            x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks) || ticks != 999u ||
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
        const lib_bool allocated = locked_reads <= 1u && misses <= 1u;
        const lib_u64 expected = 3u * (lib_u64)cases[row].accesses +
            (locked ? (cases[row].implicit_lock ? 0u : 2u) : cases[row].miss * misses);
        lib_u64 ticks = 999u;

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
            x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks) != allocated ||
            ticks != (allocated ? expected : 999u)) {
            lib_c_printf("Integer alignment row=%u dword=%u split=%u warm=%u lock=%u: align=%u miss=%u locked=%u ticks=%llu/%llu\n",
                (lib_u32)row, (lib_u32)dword, (lib_u32)split, (lib_u32)warm, (lib_u32)lock,
                fixture->instructions.data.timing_unaligned_accesses,
                fixture->instructions.data.timing_data_misses,
                fixture->instructions.data.timing_locked_reads, ticks, allocated ? expected : 999u);
            return LIB_FALSE;
        }
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

        /* Cached and raced cases deliberately have no allocated numerical
         * row; they must not silently inherit the cold 13-clock assumption. */
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
        const lib_bool allocated = !cached && !raced;
        if (data->timing_tlb_walks[actual_updates] != 1u ||
            data->timing_tlb_walks[(actual_updates + 1u) % 3u] != 0u ||
            data->timing_tlb_walks[(actual_updates + 2u) % 3u] != 0u ||
            data->timing_table_misses != (cached ? 0u : 2u) ||
            data->timing_tlb_update_attempts != (raced ? 2u : updates) ||
            data->timing_data_misses != (lib_u32)!store ||
            x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks) != allocated ||
            ticks != (allocated ? walk_ticks[updates] + (store ? 0u : 2u) : 999u)) {
            lib_c_printf("Translation timing %u/%u/%u: walks=%u,%u,%u tables=%u attempts=%u data=%u ticks=%llu\n",
                (lib_u32)store, (lib_u32)updates, (lib_u32)variant,
                data->timing_tlb_walks[0], data->timing_tlb_walks[1], data->timing_tlb_walks[2],
                data->timing_table_misses, data->timing_tlb_update_attempts,
                data->timing_data_misses, ticks);
            return LIB_FALSE;
        }
        ticks = 999u;
        if (!cache_run(fixture, store ? write : read, store ? sizeof(write) : sizeof(read)) ||
            !x86_core_cpu_80486_memory_penalty(&fixture->execution, &ticks) || ticks != 0u)
            return LIB_FALSE;
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
    if (!cache_translation_timing()) { lib_c_printf("Cache translation timing failed\n"); return 1; }
    lib_c_printf("CPU cache state and access: OK\n");
    return 0;
}
