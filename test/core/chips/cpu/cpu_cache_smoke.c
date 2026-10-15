#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

static lib_bool cache_run(cpu_instruction_fixture *fixture,
    const lib_u8 *code, lib_u8 bytes)
{
    t_cpu after;

    fixture->cpu.data.eip = 0u;
    core_machine_cpu_execution_invalidate_prefetch(&fixture->execution);
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

    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
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
        core_machine_cpu_instruction_lexeme lexeme;
        const lib_bool code32 = (mode & 1u) != 0u;
        lib_u8 code[5], bytes = 0u;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        if (mode >= 2u) fixture.cpu.data.cr0 |= VCPU_CR0_PE;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0x0fu;
        code[bytes++] = write ? 0x26u : 0x24u;
        code[bytes++] = (lib_u8)(0xc0u | (reg << 3u));
        if (!write && reg == 5u) {
            if (core_machine_cpu_instruction_lexeme_scan(code, bytes,
                    CORE_MACHINE_CPU_PROFILE_80486SX, code32, &lexeme))
                return LIB_FALSE;
            continue;
        }
        const lib_u32 flags = fixture.cpu.data.eflags;
        if (!core_machine_cpu_instruction_lexeme_scan(code, bytes,
                CORE_MACHINE_CPU_PROFILE_80486SX, code32, &lexeme) ||
            lexeme.byte_count != bytes || !cache_run(&fixture, code, bytes) ||
            fixture.cpu.data.eflags != flags || fixture.writes != 0u)
            return LIB_FALSE;
        for (lib_u8 available = 0u; available < bytes; ++available)
            if (core_machine_cpu_instruction_lexeme_scan(code, available,
                    CORE_MACHINE_CPU_PROFILE_80486SX, code32, &lexeme))
                return LIB_FALSE;
    }
    {
        cpu_instruction_fixture fixture;
        const lib_u8 read_control[] = {0x0fu, 0x24u, 0xe8u};

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
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
    core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    cache_fetch_probe *probe = opaque;

    if (!observe_only && address <= probe->fail_at &&
        (lib_u64)address + bytes > probe->fail_at &&
        (provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH ||
         provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH))
        return LIB_STATUS_IO_ERROR;
    return cpu_instruction_read(&probe->instruction, address, destination,
        bytes, provenance, observe_only, reset_fetch, cache_control);
}

static lib_bool cache_test_observation_and_fetch(void)
{
    static const core_machine_cpu_bus_provider bus = {
        .read_memory = cache_fetch_read, .write_memory = cpu_instruction_write,
        .interrupt_pending = cpu_instruction_interrupt_pending};

    for (lib_u8 reg = 3u; reg < 6u; ++reg)
    for (lib_u8 write = 0u; write < 2u; ++write) {
        const lib_u8 code[] = {0x66u, 0x67u, 0x0fu, write ? 0x26u : 0x24u,
            (lib_u8)(0xc0u | (reg << 3u))};

        if (!write && reg == 5u) continue;
        for (lib_u32 fail_at = 0u; fail_at < sizeof(code); ++fail_at) {
            cache_fetch_probe probe;
            core_machine_cpu_execution_context before;
            core_machine_cpu_instruction_lexeme lexeme;
            t_cpu after, before_cpu;

            cpu_instruction_prepare_with_bus(&probe.instruction,
                CORE_MACHINE_CPU_PROFILE_80486SX, &bus, &probe);
            probe.fail_at = LIB_UINT32_MAX;
            if (!cache_write_register(&probe.instruction, 3u, 0x12345678u) ||
                !cache_write_register(&probe.instruction, 4u, 0xabc00400u) ||
                !cache_write_register(&probe.instruction, 5u, 1u))
                return LIB_FALSE;
            probe.instruction.cpu.data.eip = 0u;
            probe.instruction.cpu.data.eax = 3u;
            core_machine_cpu_execution_invalidate_prefetch(&probe.instruction.execution);
            lib_memory_copy(probe.instruction.memory, code, sizeof(code));
            before = probe.instruction.execution;
            before_cpu = probe.instruction.cpu;
            if (!core_machine_cpu_execution_preview_lexeme(&probe.instruction.execution,
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
    lib_u32 fail_fetch_at;
    lib_u8 operation_count;
    lib_u8 fail_operation;
    core_machine_cpu_cache_operation operations[2];
} cache_memory_probe;

static lib_status cache_memory_read(void *opaque, lib_u32 address,
    void *destination, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    cache_memory_probe *probe = opaque;
    if (!observe_only && probe->fail_fetch && address <= probe->fail_fetch_at &&
        (lib_u64)address + bytes > probe->fail_fetch_at &&
        (provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH ||
         provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH))
        return LIB_STATUS_IO_ERROR;
    if (!observe_only && provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) {
        probe->read_control = cache_control;
        ++probe->data_reads;
        if (address >= 0x2000u && address < 0x6000u) ++probe->target_data_reads;
        if (probe->lock_active) ++probe->locked_data_reads;
        probe->read_bytes += bytes;
        if (probe->fail_read) return LIB_STATUS_IO_ERROR;
    }
    if (!observe_only && provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ) {
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
    return cpu_instruction_read(&probe->instruction, address, destination,
        bytes, provenance, observe_only, reset_fetch, cache_control);
}

static lib_status cache_memory_write(void *opaque, lib_u32 address,
    const void *source, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    cache_memory_probe *probe = opaque;
    if (provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_DATA &&
        address >= 0x2000u && address < 0x6000u) {
        ++probe->target_data_writes;
        if (probe->lock_active) ++probe->locked_data_writes;
    }
    if (provenance == CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE) {
        if (probe->instruction.execution.cpu_profile == CORE_MACHINE_CPU_PROFILE_80486SX &&
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
    core_machine_cpu_cache_query query, core_machine_cpu_cache_memory *out_memory)
{
    cache_memory_probe *probe = opaque;
    if (query == CORE_MACHINE_CPU_CACHE_FILL_PROBE &&
        address >= 0x2000u && address < 0x4000u) ++probe->fill_probes;
    *out_memory = (core_machine_cpu_cache_memory){.address = address,
        .fill_enabled = query == CORE_MACHINE_CPU_CACHE_FILL_PROBE &&
            address >= 0x2000u && address < 0x4000u};
    return LIB_STATUS_OK;
}

static lib_status cache_memory_operation(void *opaque,
    core_machine_cpu_cache_operation operation)
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

static const core_machine_cpu_bus_provider cache_memory_bus = {
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

        cpu_instruction_prepare_with_bus(fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
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

        cpu_instruction_prepare_with_bus(fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
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

    cpu_instruction_prepare_with_bus(fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
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

        cpu_instruction_prepare_with_bus(fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
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
            fixture->instructions.data.flagLock != prefix || probe.fill_probes != 0u)
            return LIB_FALSE;
        lib_memory_copy(&observed, fixture->memory + 0x2000u, sizeof(observed));
        if (observed != (bus_write ? ((ram & ~mask) | (stored & mask)) : ram))
            return LIB_FALSE;
    }
    for (lib_u8 stage = 0u; stage < 3u; ++stage) {
        cache_memory_probe probe = {0};
        cpu_instruction_fixture *fixture = &probe.instruction;
        const lib_u8 code[] = {0x87u, 0x06u, 0u, 0x20u};

        cpu_instruction_prepare_with_bus(fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
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

        cpu_instruction_prepare_with_bus(fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
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

        cpu_instruction_prepare_with_bus(fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
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

        cpu_instruction_prepare_with_bus(fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
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
            (!write && probe.read_control != CORE_MACHINE_CPU_MEMORY_CACHE_PCD))
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

        cpu_instruction_prepare_with_bus(fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
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

        cpu_instruction_prepare_with_bus(fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
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

        cpu_instruction_prepare_with_bus(fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
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
            CORE_MACHINE_CPU_PROFILE_80386 : CORE_MACHINE_CPU_PROFILE_80486SX,
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

        cpu_instruction_prepare_with_bus(fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
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

        cpu_instruction_prepare_with_bus(fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
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

        cpu_instruction_prepare_with_bus(fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
            &cache_memory_bus, &probe);
        fixture->cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        if (!cache_run(fixture, (const lib_u8[]){0xa1u, 0u, 0x20u}, 3u) ||
            !cache_run(fixture, (const lib_u8[]){0xa1u, 0x10u, 0x20u}, 3u) ||
            !cache_run(fixture, (const lib_u8[]){0xa1u, 0u, 0x30u}, 3u))
            return LIB_FALSE;
        if (nw) fixture->cpu.data.cr0 |= VCPU_CR0_CD | VCPU_CR0_NW;
        core_machine_cpu_signal_cache_write(&fixture->execution, 0x200fu, 2u);
        if (fixture->execution.cache[0][0].valid != (nw != 0u) ||
            fixture->execution.cache[1][0].valid != (nw != 0u) ||
            !fixture->execution.cache[0][1].valid) return LIB_FALSE;
        core_machine_cpu_signal_cache_write(&fixture->execution, 0u, 0x4000u);
        if (fixture->execution.cache[0][1].valid != (nw != 0u)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cache_reset_and_snoop_bounds(void)
{
    cpu_instruction_fixture fixture;

    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
    fixture.cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
    fixture.execution.cache[0][0] = (cpu_cache_line){
        .tag = 0u, .valid = LIB_TRUE, .data = {0x1234u}};
    fixture.execution.cache[127][0] = (cpu_cache_line){
        .tag = 0xfffff800u, .valid = LIB_TRUE, .data = {0x5678u}};
    fixture.execution.cache_lru[127] = 7u;
    core_machine_cpu_signal_cache_write(&fixture.execution, 0xfffffff8u, 0u);
    if (!fixture.execution.cache[127][0].valid) return LIB_FALSE;
    core_machine_cpu_signal_cache_write(&fixture.execution, 0xfffffff8u,
        (lib_size)LIB_UINT64_MAX);
    if (fixture.execution.cache[127][0].valid ||
        !fixture.execution.cache[0][0].valid ||
        fixture.execution.cache[127][0].tag != 0xfffff800u ||
        fixture.execution.cache[127][0].data[0] != 0x5678u ||
        fixture.execution.cache_lru[127] != 7u) return LIB_FALSE;
    core_machine_cpu_signal_cache_write(&fixture.execution, 0u,
        (lib_size)LIB_UINT64_MAX);
    if (fixture.execution.cache[0][0].valid) return LIB_FALSE;

    lib_memory_set(fixture.execution.cache, 0xffu, sizeof(fixture.execution.cache));
    lib_memory_set(fixture.execution.cache_lru, 0xffu, sizeof(fixture.execution.cache_lru));
    lib_memory_set(fixture.execution.cache_fill, 0xffu, sizeof(fixture.execution.cache_fill));
    lib_memory_set(fixture.execution.cache_read, 0xffu, sizeof(fixture.execution.cache_read));
    core_machine_cpu_state_reset(&fixture.execution);
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

        cpu_instruction_prepare_with_bus(fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
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

        cpu_instruction_prepare_with_bus(fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
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
        core_machine_cpu_instruction_lexeme lexeme;

        cpu_instruction_prepare_with_bus(fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
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
        if (!core_machine_cpu_instruction_lexeme_scan(code, bytes,
                CORE_MACHINE_CPU_PROFILE_80486SX, (mode & 1u) != 0u, &lexeme) ||
            !cache_run(fixture, code, bytes) || fixture->cpu.data.eflags != flags ||
            probe.operation_count != (writeback ? 2u : 1u) ||
            probe.operations[0] != (writeback ? CORE_MACHINE_CPU_CACHE_WRITEBACK :
                CORE_MACHINE_CPU_CACHE_FLUSH) ||
            (writeback && probe.operations[1] != CORE_MACHINE_CPU_CACHE_FLUSH) ||
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

        cpu_instruction_prepare_with_bus(fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
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

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        if (!cpu_instruction_expect_real_fault(&fixture, code, sizeof(code), 6u))
            return LIB_FALSE;
        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        if (!cpu_instruction_expect_real_fault(&fixture, locked, sizeof(locked), 6u))
            return LIB_FALSE;
        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        if (cache_run(&fixture, code, sizeof(code)) || !fixture.fault.valid ||
            fixture.fault.exception_mask != VCPUINS_EXCEPT_CE) return LIB_FALSE;
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
    lib_c_printf("CPU cache state and access: OK\n");
    return 0;
}
