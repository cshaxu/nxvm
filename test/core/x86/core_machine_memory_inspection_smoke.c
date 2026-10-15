#include "lib/types/types_interface.h"
#include "core/x86/machine.h"
#include "core/x86/debug_interface.h"
#include "core/chips/cpu/cpu.h"
#include "core/chips/cpu/cpu_instructions.h"

typedef struct inspection_probe {
    lib_u32 reads;
    lib_u32 parity;
    lib_u32 queries;
    lib_status status;
    lib_status query_status;
    lib_status write_status;
    lib_status control_status;
    lib_u32 control_count;
    lib_u8 last_control;
    lib_u8 cache_count;
    lib_bool cache_fail;
    core_machine_cpu_cache_operation cache_operations[2];
    lib_bool page_mode;
    lib_bool replace_page_entry;
    lib_bool page_lock_seen;
    lib_bool page_protocol_error;
    lib_u32 second_page_entry;
    lib_u32 pages[4];
    lib_u32 page_reads[4];
    lib_u32 page_writes[4];
    core_machine *page_machine;
    lib_bool request_dma_locked;
    lib_bool dma_lock_error;
    lib_u32 denied_dma_cycles;
} inspection_probe;

static lib_status probe_read(void *opaque, lib_u32 address,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    inspection_probe *probe = opaque;
    (void)address;
    if (!observe_only) ++probe->reads;
    if (probe->status != LIB_STATUS_OK) return probe->status;
    if (probe->page_mode) {
        const lib_u32 index = (address - 0x5000u) / 4u;
        if (!observe_only) {
            ++probe->page_reads[index];
            if (index == 3u && probe->page_reads[index] == 2u) {
                probe->page_lock_seen = probe->page_machine->transaction.owner ==
                    CORE_MACHINE_TRANSACTION_OWNER_CPU &&
                    probe->page_machine->transaction.kind == CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK;
                if (probe->replace_page_entry) probe->pages[index] = probe->second_page_entry;
            }
        }
        lib_memory_copy((void *)destination, (const lib_u8 *)probe->pages +
            (address - 0x5000u), bytes);
        return LIB_STATUS_OK;
    }
    lib_memory_set((void *)destination, 0x90u, bytes);
    return LIB_STATUS_OK;
}

static lib_status probe_write(void *opaque, lib_u32 address,
    lib_uptr source, lib_uptr bytes)
{
    inspection_probe *probe = opaque;
    (void)address; (void)source; (void)bytes;
    if (probe->write_status != LIB_STATUS_OK) return probe->write_status;
    if (probe->page_mode) {
        const lib_u32 index = (address - 0x5000u) / 4u;
        ++probe->page_writes[index];
        if (probe->page_machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_CPU ||
            probe->page_machine->transaction.kind != CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK)
            probe->page_protocol_error = LIB_TRUE;
        lib_memory_copy((lib_u8 *)probe->pages + (address - 0x5000u),
            (const void *)source, bytes);
    }
    return probe->write_status;
}

static lib_status probe_query(void *opaque, lib_u32 address, lib_uptr bytes,
    core_machine_memory_access access)
{
    inspection_probe *probe = opaque;
    (void)address; (void)bytes; (void)access;
    ++probe->queries;
    return probe->query_status;
}

static void probe_parity(void *opaque, lib_u32 address)
{
    inspection_probe *probe = opaque;
    (void)address;
    ++probe->parity;
}

static lib_status probe_cache_operation(void *opaque,
    core_machine_cpu_cache_operation operation)
{
    inspection_probe *probe = opaque;
    if (probe->cache_count >= 2u) return LIB_STATUS_LIMIT_EXCEEDED;
    probe->cache_operations[probe->cache_count++] = operation;
    return probe->cache_fail ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}

static void probe_finalize(void *opaque)
{
    (void)opaque;
}

static lib_status probe_memory_control(void *opaque, lib_u32 address,
    lib_u8 bytes, lib_bool write, lib_u8 cache_control)
{
    inspection_probe *probe = opaque;
    (void)address; (void)bytes; (void)write;
    ++probe->control_count;
    probe->last_control = cache_control;
    if (probe->request_dma_locked && probe->page_machine->transaction.kind ==
            CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK) {
        lib_u16 value = 0x9999u;
        const lib_status status = core_machine_dma_memory_cycle(probe->page_machine,
            0x3000u, 2u, 2u, CORE_MACHINE_MEMORY_ACCESS_WRITE, &value,
            LIB_NULL, LIB_NULL, LIB_NULL);
        ++probe->denied_dma_cycles;
        if (status == LIB_STATUS_OK || probe->page_machine->transaction.owner !=
                CORE_MACHINE_TRANSACTION_OWNER_CPU || probe->page_machine->transaction.kind !=
                CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK) probe->dma_lock_error = LIB_TRUE;
    }
    return probe->control_status;
}

/* Bind the selected chip in a Core-owned unit fixture, without opening its
 * production machine-admission gate or substituting the real memory bus. */
static lib_bool inspection_cache_execution(void)
{
    const core_machine_executor_config config = {.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386};
    const core_machine_instruction_timing timing = {.base_ticks = 1u};
    const lib_u8 read[] = {0xa1u, 0u, 0x30u};
    const lib_u8 write[] = {0xa3u, 0u, 0x30u};
    core_machine *machine = LIB_NULL;
    core_machine_cpu_execution_context *cpu;
    lib_u16 value = 0x1234u;
    lib_bool failed = LIB_FALSE;
    inspection_probe device = {0};

    if (core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return LIB_FALSE;
    const core_machine_attachment attachment = {.context = &device,
        .finalize_devices = probe_finalize, .memory_cache_control = probe_memory_control};
    device.page_machine = machine;
    const core_machine_memory_alias_config alias = {
        .physical_start = 0x8000u, .backing_start = 0x3000u, .bytes = 16u};
    const core_machine_memory_device_route route = {.physical_start = 0x5000u,
        .bytes = 16u, .callbacks = {probe_read, probe_write, probe_query}};
    if (core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
        core_machine_install_memory_device_routes(machine, &route, 1u,
            LIB_NULL, LIB_NULL, &device) != LIB_STATUS_OK ||
        core_machine_install_memory_aliases(machine, &alias, 1u, LIB_TRUE) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_reconfigure_memory(machine, 2u * 1024u * 1024u) != LIB_STATUS_OK ||
        machine->executor_memory.connect.cpu_snoop != core_machine_cpu_bus_snoop_write ||
        machine->executor_memory.connect.cpu_snoop_context != machine) {
        core_machine_destroy(machine);
        return LIB_FALSE;
    }
    cpu = machine->executor_cpu_execution;
    core_machine_cpu_execution_context_bind_profiles(cpu, CORE_MACHINE_CPU_PROFILE_80486SX,
        X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
    core_machine_cpu_state_reset(cpu);
    core_machine_cpu_execution_load_segment(cpu, &cpu->cpu->data.cs, 0u);
    core_machine_cpu_execution_load_segment(cpu, &cpu->cpu->data.ds, 0u);
    cpu->cpu->data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
    failed |= core_machine_memory_write(machine, 0u, read, sizeof(read)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x3000u, &value, sizeof(value)) != LIB_STATUS_OK;
    cpu->cpu->data.eip = 0u;
    core_machine_cpu_execution_refresh(cpu);
    failed |= cpu->cpu->data.ax != value;
    /* Same-owner integration fixture seeds one real line to isolate provider
     * publication, not to duplicate or test CPU cache algorithms. */
    cpu->cache[0][3] = (cpu_cache_line){.tag = 0x5000u, .valid = LIB_TRUE};
    device.write_status = LIB_STATUS_IO_ERROR;
    failed |= core_machine_memory_write(machine, 0x5000u, &value, sizeof(value)) !=
        LIB_STATUS_IO_ERROR || cpu->cache[0][3].valid;
    cpu->cache[0][3].valid = LIB_TRUE;
    device.write_status = LIB_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0x5000u, &value, sizeof(value)) !=
        LIB_STATUS_OK || cpu->cache[0][3].valid;
    value = 0x4321u;
    failed |= core_machine_memory_write(machine, 0x8000u, &value, sizeof(value)) != LIB_STATUS_OK;
    cpu->cpu->data.eip = 0u;
    core_machine_cpu_execution_invalidate_prefetch(cpu);
    core_machine_cpu_execution_refresh(cpu);
    failed |= cpu->cpu->data.ax != value;
    value = 0x5678u;
    failed |= core_machine_dma_memory_cycle(machine, 0x3000u, 2u, 2u,
        CORE_MACHINE_MEMORY_ACCESS_WRITE, &value, LIB_NULL, LIB_NULL, LIB_NULL) != LIB_STATUS_OK;
    cpu->cpu->data.eip = 0u;
    core_machine_cpu_execution_invalidate_prefetch(cpu);
    core_machine_cpu_execution_refresh(cpu);
    failed |= cpu->cpu->data.ax != value;
    {
        const lib_u8 write_alias[] = {0xa3u, 0u, 0x80u};
        failed |= core_machine_memory_write(machine, 0u, write_alias,
            sizeof(write_alias)) != LIB_STATUS_OK;
        cpu->cpu->data.eip = 0u;
        cpu->cpu->data.ax = 0x1357u;
        core_machine_cpu_execution_refresh(cpu);
        failed |= core_machine_memory_write(machine, 0u, read, sizeof(read)) != LIB_STATUS_OK;
        cpu->cpu->data.eip = 0u;
        cpu->cpu->data.ax = 0u;
        core_machine_cpu_execution_refresh(cpu);
        failed |= cpu->cpu->data.ax != 0x1357u;
    }
    failed |= core_machine_memory_write(machine, 0u, write, sizeof(write)) != LIB_STATUS_OK;
    cpu->cpu->data.ax = 0xabcdu;
    cpu->cpu->data.eip = 0u;
    core_machine_cpu_execution_refresh(cpu);
    failed |= core_machine_memory_read(machine, 0x3000u, &value, sizeof(value)) != LIB_STATUS_OK ||
        value != 0xabcdu;
    lib_bool resident = LIB_FALSE;
    for (lib_u8 way = 0u; way < 4u; ++way)
        resident |= cpu->cache[0][way].valid && cpu->cache[0][way].tag == 0x3000u;
    failed |= !resident;
    failed |= core_machine_memory_write(machine, 0u, read, sizeof(read)) != LIB_STATUS_OK;
    cpu->cpu->data.cr0 |= VCPU_CR0_CD | VCPU_CR0_NW;
    value = 0x8765u;
    failed |= core_machine_memory_write(machine, 0x3000u, &value, sizeof(value)) != LIB_STATUS_OK;
    cpu->cpu->data.ax = 0x1357u;
    cpu->cpu->data.eip = 0u;
    core_machine_cpu_execution_refresh(cpu);
    failed |= cpu->cpu->data.ax != 0xabcdu || cpu->stop_requested;
    /* Actual selected CPU, production bus and provider: the second PTE read
     * redirects a paged operand while the Core transaction remains held. */
    {
        const lib_u32 directory = 0x5023u;
        const lib_u16 old_value = 0x1234u, new_value = 0x5678u;
        const lib_u64 cancelled = machine->transaction.cancelled_count;

        core_machine_cpu_state_reset(cpu);
        core_machine_cpu_execution_load_segment(cpu, &cpu->cpu->data.cs, 0u);
        core_machine_cpu_execution_load_segment(cpu, &cpu->cpu->data.ds, 0u);
        cpu->cpu->data.cr0 = VCPU_CR0_PE | VCPU_CR0_PG;
        cpu->cpu->data.cr3 = 0x4000u;
        cpu->cpu->data.ds.selector = 0x10u;
        device.page_mode = LIB_TRUE;
        device.page_machine = machine;
        device.pages[0] = 0x23u;
        device.pages[3] = 0x9003u;
        device.replace_page_entry = LIB_TRUE;
        device.second_page_entry = 0xa003u;
        failed |= core_machine_memory_write(machine, 0u, read, sizeof(read)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x4000u, &directory, sizeof(directory)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x9000u, &old_value, sizeof(old_value)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0xa000u, &new_value, sizeof(new_value)) != LIB_STATUS_OK;
        cpu->cpu->data.eip = 0u;
        core_machine_cpu_execution_refresh(cpu);
        failed |= cpu->cpu->data.ax != new_value || cpu->cpu->data.eip != sizeof(read) ||
            cpu->stop_requested || !device.page_lock_seen || device.page_protocol_error ||
            device.page_reads[3] != 2u || device.page_writes[3] != 1u ||
            device.pages[3] != 0xa023u ||
            machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
            machine->transaction.cancelled_count != cancelled;
        value = 0x6789u;
        failed |= core_machine_memory_write(machine, 0xa000u, &value, sizeof(value)) != LIB_STATUS_OK;
        cpu->cpu->data.eip = 0u;
        core_machine_cpu_execution_invalidate_prefetch(cpu);
        core_machine_cpu_execution_refresh(cpu);
        failed |= cpu->cpu->data.ax != value || device.page_reads[3] != 2u ||
            device.page_writes[3] != 1u;
        failed |= core_machine_memory_write(machine, 0u, write, sizeof(write)) != LIB_STATUS_OK;
        cpu->cpu->data.eip = 0u;
        cpu->cpu->data.ax = 0xbeefu;
        core_machine_cpu_execution_invalidate_prefetch(cpu);
        core_machine_cpu_execution_refresh(cpu);
        failed |= core_machine_memory_read(machine, 0xa000u, &value, sizeof(value)) != LIB_STATUS_OK ||
            value != 0xbeefu || device.page_reads[3] != 4u ||
            device.page_writes[3] != 2u || device.pages[3] != 0xa063u ||
            device.page_protocol_error || cpu->stop_requested ||
            machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
            machine->transaction.cancelled_count != cancelled;
        failed |= core_machine_memory_read(machine, 0x9000u, &value, sizeof(value)) != LIB_STATUS_OK ||
            value != old_value;
    }
    /* A stale real cache line supplies the first PTE; the locked read must
     * instead use RAM and keep a successful hit update coherent. */
    for (lib_u8 cache_ram = 0u; cache_ram < 2u; ++cache_ram) {
        const lib_u32 directory = 0x11023u;
        const lib_u32 pages[4] = {0x23u, 0u, 0u, 0xa003u};
        lib_u32 updated = 0u;

        cpu->cpu->data.cr0 = VCPU_CR0_PE | VCPU_CR0_PG;
        failed |= core_machine_memory_write(machine, 0u, read, sizeof(read)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x4000u, &directory, sizeof(directory)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x11000u, pages, sizeof(pages)) != LIB_STATUS_OK;
        cpu->cache[0][3] = (cpu_cache_line){.tag = 0x11000u, .valid = LIB_TRUE,
            .data = {0x23u, 0u, 0u, 0x9003u}};
        if (cache_ram) cpu->cpu->data.cr0 |= VCPU_CR0_CD | VCPU_CR0_NW;
        core_machine_cpu_execution_invalidate_tlb(cpu);
        core_machine_cpu_execution_invalidate_prefetch(cpu);
        cpu->cpu->data.eip = 0u;
        cpu->cpu->data.ax = 0u;
        core_machine_cpu_execution_refresh(cpu);
        failed |= cpu->cpu->data.ax != 0xbeefu || cpu->stop_requested ||
            cpu->cpu->data.eip != sizeof(read) ||
            !cpu->cache[0][3].valid || cpu->cache[0][3].tag != 0x11000u ||
            cpu->cache[0][3].data[3] != 0xa023u ||
            machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE;
        failed |= core_machine_memory_read(machine, 0x1100cu, &updated, sizeof(updated)) != LIB_STATUS_OK ||
            updated != 0xa023u;
    }
    for (lib_u8 cache_ram = 0u; cache_ram < 2u; ++cache_ram) {
        const lib_u8 exchange[] = {0x87u, 0x06u, 0u, 0x80u};
        const lib_u16 original = 0x1111u;

        core_machine_cpu_state_reset(cpu);
        core_machine_cpu_execution_load_segment(cpu, &cpu->cpu->data.cs, 0u);
        core_machine_cpu_execution_load_segment(cpu, &cpu->cpu->data.ds, 0u);
        if (!cache_ram) cpu->cpu->data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        failed |= core_machine_memory_write(machine, 0u, exchange, sizeof(exchange)) != LIB_STATUS_OK ||
            core_machine_memory_write(machine, 0x3000u, &original, sizeof(original)) != LIB_STATUS_OK;
        cpu->cache[0][0] = (cpu_cache_line){.tag = 0x8000u, .valid = LIB_TRUE,
            .data = {0x2222u}};
        cpu->cache[0][1] = (cpu_cache_line){.tag = 0x3000u, .valid = LIB_TRUE,
            .data = {0x4444u}};
        cpu->cpu->data.eip = 0u;
        cpu->cpu->data.ax = 0x3333u;
        device.request_dma_locked = LIB_TRUE;
        device.denied_dma_cycles = 0u;
        device.dma_lock_error = LIB_FALSE;
        core_machine_cpu_execution_refresh(cpu);
        failed |= cpu->cpu->data.ax != original || cpu->cpu->data.eip != sizeof(exchange) ||
            cpu->stop_requested || cpu->instructions->data.memory_lock_active ||
            cpu->instructions->data.flagLock ||
            machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
            !cpu->cache[0][0].valid || cpu->cache[0][0].data[0] != 0x3333u ||
            cpu->cache[0][1].valid != (lib_bool)cache_ram ||
            device.denied_dma_cycles != 2u || device.dma_lock_error;
        failed |= core_machine_memory_read(machine, 0x3000u, &value, sizeof(value)) != LIB_STATUS_OK ||
            value != 0x3333u;
    }
    core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
    const core_machine_executor_config config = {.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386};
    core_machine *machine = LIB_NULL;
    core_machine_cpu_instruction_lexeme lexeme;
    inspection_probe probe = {0};
    lib_u8 bytes[4] = {0};
    lib_u8 expected[4] = {0x11u, 0x90u, 0x90u, 0x22u};
    t_ram *memory;
    lib_bool failed = LIB_FALSE;

    if (core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
    memory = &machine->executor_memory;
    /* With no external cache, Core still completes the special transaction. */
    failed |= core_machine_cpu_bus.cache_operation(machine,
        CORE_MACHINE_CPU_CACHE_FLUSH) != LIB_STATUS_OK ||
        machine->transaction.committed_count != 1u;
    {
        const core_machine_attachment attachment = {.context = &probe,
            .finalize_devices = probe_finalize, .cache_operation = probe_cache_operation,
            .memory_cache_control = probe_memory_control};
        failed |= core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK;
    }
    failed |= core_machine_memory_register_device_provider(memory, 0x1001u, 2u,
        probe_read, probe_write, probe_query, &probe) != LIB_STATUS_OK;
    failed |= core_machine_memory_register_overlay_device_provider(memory,
        0xfffffff0u, 16u, probe_read, probe_write, probe_query, &probe) != LIB_STATUS_OK;
    failed |= core_machine_memory_enable_parity(memory, 0x2000u,
        probe_parity, &probe) != LIB_STATUS_OK;
    {
        const core_machine_memory_alias_config alias = {
            .physical_start = 0x8000u, .backing_start = 0x3000u, .bytes = 16u};
        failed |= core_machine_install_memory_aliases(machine, &alias, 1u,
            LIB_TRUE) != LIB_STATUS_OK;
    }
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    if (failed) { core_machine_destroy(machine); return 1; }

    {
        const lib_u8 value = 0xa5u;
        lib_u8 observed = 0xffu;
        const lib_u64 cancelled = machine->transaction.cancelled_count;
        probe.control_status = LIB_STATUS_IO_ERROR;
        failed |= core_machine_cpu_bus.write_memory(machine, 0x3010u, &value, 1u,
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, 0x18u) != LIB_STATUS_IO_ERROR ||
            probe.last_control != 0x18u || probe.control_count != 1u ||
            machine->transaction.cancelled_count != cancelled + 1u;
        probe.control_status = LIB_STATUS_OK;
        failed |= core_machine_cpu_bus.read_memory(machine, 0x3010u, &observed, 1u,
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 0x10u) != LIB_STATUS_OK ||
            observed != 0u || probe.last_control != 0x10u || probe.control_count != 2u;
        failed |= core_machine_cpu_bus.read_memory(machine, 0x3010u, &observed, 1u,
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_TRUE, LIB_FALSE, 0x18u) != LIB_STATUS_OK ||
            probe.control_count != 2u;
        failed |= core_machine_cpu_bus.write_memory(machine, 0x3010u, &value, 1u,
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, 1u) != LIB_STATUS_INVALID_ARGUMENT ||
            probe.control_count != 2u ||
            machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE;
    }

    {
        const lib_u64 committed = machine->transaction.committed_count;
        const lib_u64 cancelled = machine->transaction.cancelled_count;
        probe.cache_fail = LIB_TRUE;
        failed |= core_machine_cpu_bus.cache_operation(machine,
            CORE_MACHINE_CPU_CACHE_WRITEBACK) != LIB_STATUS_IO_ERROR ||
            machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
            machine->transaction.cancelled_count != cancelled + 1u ||
            machine->transaction.committed_count != committed;
        probe.cache_fail = LIB_FALSE;
        failed |= core_machine_cpu_bus.cache_operation(machine,
            CORE_MACHINE_CPU_CACHE_FLUSH) != LIB_STATUS_OK ||
            machine->transaction.committed_count != committed + 1u ||
            probe.cache_count != 2u ||
            probe.cache_operations[0] != CORE_MACHINE_CPU_CACHE_WRITEBACK ||
            probe.cache_operations[1] != CORE_MACHINE_CPU_CACHE_FLUSH;
        failed |= core_machine_cpu_bus.cache_operation(machine,
            (core_machine_cpu_cache_operation)2) != LIB_STATUS_INVALID_ARGUMENT ||
            probe.cache_count != 2u;
    }

    /* Use the production CPU bus query: a narrow provider inside the line
     * disables fills even when the requested first byte is ordinary RAM. */
    {
        core_machine_cpu_cache_memory cache;
        const lib_u32 queries = probe.queries;

        failed |= core_machine_cpu_bus.query_cache_memory(machine, 0x103010u,
            CORE_MACHINE_CPU_CACHE_LOOKUP, &cache) != LIB_STATUS_OK ||
            cache.address != 0x3010u || cache.fill_enabled || probe.queries != queries;
        failed |= core_machine_cpu_bus.query_cache_memory(machine, 0xfffffff0u,
            CORE_MACHINE_CPU_CACHE_RESET_LOOKUP, &cache) != LIB_STATUS_OK ||
            cache.address != 0xfffffff0u || cache.fill_enabled || probe.queries != queries;
        failed |= core_machine_cpu_bus.query_cache_memory(machine, 0x1000u,
            (core_machine_cpu_cache_query)3, &cache) != LIB_STATUS_INVALID_ARGUMENT ||
            probe.queries != queries;

        failed |= core_machine_cpu_bus.query_cache_memory(machine, 0x1000u,
            CORE_MACHINE_CPU_CACHE_FILL_PROBE, &cache) != LIB_STATUS_OK || cache.address != 0x1000u ||
            cache.fill_enabled;
        failed |= core_machine_cpu_bus.query_cache_memory(machine, 0x3010u,
            CORE_MACHINE_CPU_CACHE_FILL_PROBE, &cache) != LIB_STATUS_OK || cache.address != 0x3010u ||
            !cache.fill_enabled;
        failed |= core_machine_cpu_bus.query_cache_memory(machine, 0x103010u,
            CORE_MACHINE_CPU_CACHE_FILL_PROBE, &cache) != LIB_STATUS_OK || cache.address != 0x3010u ||
            !cache.fill_enabled;
        failed |= core_machine_cpu_bus.query_cache_memory(machine, 0xfffffff0u,
            CORE_MACHINE_CPU_CACHE_RESET_LOOKUP, &cache) != LIB_STATUS_OK || cache.address != 0xfffffff0u ||
            cache.fill_enabled || probe.reads != 0u || probe.parity != 0u;
        failed |= core_machine_cpu_bus.query_cache_memory(machine, 0x8000u,
            CORE_MACHINE_CPU_CACHE_FILL_PROBE, &cache) != LIB_STATUS_OK || cache.fill_enabled;
        probe.query_status = LIB_STATUS_IO_ERROR;
        failed |= core_machine_cpu_bus.query_cache_memory(machine, 0x1000u,
            CORE_MACHINE_CPU_CACHE_FILL_PROBE, &cache) != LIB_STATUS_OK || cache.fill_enabled;
        failed |= core_machine_memory_read_physical(memory, 0x1001u,
            (lib_uptr)bytes, 1u) != LIB_STATUS_IO_ERROR || probe.reads != 0u;
        probe.query_status = LIB_STATUS_OK;
        failed |= core_machine_signal_a20(machine, LIB_TRUE) != LIB_STATUS_OK;
        failed |= core_machine_cpu_bus.query_cache_memory(machine, 0x103010u,
            CORE_MACHINE_CPU_CACHE_FILL_PROBE, &cache) != LIB_STATUS_OK || cache.address != 0x103010u;
        failed |= core_machine_signal_a20(machine, LIB_FALSE) != LIB_STATUS_OK;
    }

    /* The real Core transaction owns the entire page-table read/write pair. */
    {
        const lib_u64 committed = machine->transaction.committed_count;
        const lib_u64 cancelled = machine->transaction.cancelled_count;
        lib_u32 entry = 0u;
        const lib_u32 updated = 0x12345623u;

        failed |= core_machine_cpu_bus.begin_memory_lock(machine) != LIB_STATUS_OK;
        failed |= core_machine_cpu_bus.read_memory(machine, 0x3010u, &entry, 4u,
            CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ, LIB_FALSE, LIB_FALSE, 0u) != LIB_STATUS_OK ||
            machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_CPU ||
            machine->transaction.committed_count != committed;
        failed |= core_machine_transaction_begin(&machine->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA, CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE,
            0x3010u, 4u, 0u) == LIB_STATUS_OK;
        failed |= core_machine_cpu_bus.read_memory(machine, 0x3010u, &entry, 4u,
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 0u) != LIB_STATUS_OK;
        failed |= core_machine_cpu_bus.write_memory(machine, 0x3010u, &updated, 4u,
            CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE, 0u) != LIB_STATUS_OK ||
            machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_CPU ||
            machine->transaction.committed_count != committed;
        core_machine_cpu_bus.complete_memory_lock(machine, LIB_TRUE);
        failed |= machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
            machine->transaction.committed_count != committed + 1u ||
            machine->transaction.cancelled_count != cancelled;
        failed |= core_machine_memory_inspect_physical(memory, 0x3010u,
            (lib_uptr)&entry, 4u, LIB_FALSE) != LIB_STATUS_OK || entry != updated;

        for (lib_u8 write = 0u; write < 2u; ++write) {
            const lib_u64 before_cancel = machine->transaction.cancelled_count;
            failed |= core_machine_cpu_bus.begin_memory_lock(machine) != LIB_STATUS_OK;
            probe.control_status = LIB_STATUS_IO_ERROR;
            if (write)
                failed |= core_machine_cpu_bus.write_memory(machine, 0x3010u, &updated, 4u,
                    CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE, 0u) != LIB_STATUS_IO_ERROR;
            else
                failed |= core_machine_cpu_bus.read_memory(machine, 0x3010u, &entry, 4u,
                    CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ, LIB_FALSE, LIB_FALSE, 0u) != LIB_STATUS_IO_ERROR;
            failed |= machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_CPU ||
                machine->transaction.cancelled_count != before_cancel;
            core_machine_cpu_bus.complete_memory_lock(machine, LIB_FALSE);
            failed |= machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
                machine->transaction.cancelled_count != before_cancel + 1u;
            probe.control_status = LIB_STATUS_OK;
        }
        failed |= core_machine_cpu_bus.begin_memory_lock(LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
            core_machine_cpu_bus.begin_memory_lock(machine) != LIB_STATUS_OK ||
            core_machine_cpu_bus.begin_memory_lock(machine) != LIB_STATUS_INVALID_ARGUMENT;
        failed |= core_machine_cpu_bus.read_memory(machine, 0x3011u, &entry, 1u,
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 1u) != LIB_STATUS_INVALID_ARGUMENT ||
            machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_CPU;
        core_machine_cpu_bus.complete_memory_lock(machine, LIB_FALSE);
        failed |= machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE;
    }

    /* Corrupt backing without updating parity, as a same-owner hardware fixture. */
    ((lib_u8 *)memory->connect.backing)[0x1000u] = expected[0];
    ((lib_u8 *)memory->connect.backing)[0x1003u] = expected[3];
    ((lib_u8 *)memory->connect.parity)[0x1000u] ^= 1u;
    failed |= core_machine_memory_inspect_physical(memory, 0x1000u,
        (lib_uptr)bytes, sizeof(bytes), LIB_FALSE) != LIB_STATUS_OK;
    failed |= lib_memory_compare(bytes, expected, sizeof(bytes)) != 0 ||
        probe.reads != 0u || probe.parity != 0u;
    failed |= core_machine_memory_read_physical(memory, 0x1000u,
        (lib_uptr)bytes, sizeof(bytes)) != LIB_STATUS_OK;
    failed |= lib_memory_compare(bytes, expected, sizeof(bytes)) != 0 ||
        probe.reads != 2u || probe.parity == 0u;
    probe.reads = probe.parity = 0u;

    /* Reset preview selects high ROM without A20 wrap or callback side effects. */
    failed |= !core_machine_cpu_execution_preview_lexeme(
        machine->executor_cpu_execution, &lexeme) || !lexeme.available ||
        lexeme.byte_count != 1u || probe.reads != 0u || probe.parity != 0u;
    failed |= core_machine_memory_read_reset_physical(memory, 0xfffffff0u,
        (lib_uptr)bytes, 1u) != LIB_STATUS_OK || bytes[0] != 0x90u || probe.reads != 1u;
    probe.reads = 0u;
    probe.status = LIB_STATUS_IO_ERROR;
    failed |= core_machine_memory_inspect_physical(memory, 0x1001u,
        (lib_uptr)bytes, 1u, LIB_FALSE) != LIB_STATUS_IO_ERROR || probe.reads != 0u;
    failed |= core_machine_cpu_execution_preview_lexeme(
        machine->executor_cpu_execution, &lexeme) || lexeme.available ||
        probe.reads != 0u;
    probe.status = LIB_STATUS_OK;
    failed |= core_machine_memory_inspect_physical(memory, 0x101001u,
        (lib_uptr)bytes, 1u, LIB_FALSE) != LIB_STATUS_OK || bytes[0] != 0x90u;
    failed |= core_machine_memory_inspect_physical(memory, 0u,
        (lib_uptr)bytes, 0u, LIB_FALSE) != LIB_STATUS_INVALID_ARGUMENT;

    /* A paged preview must not publish accessed/dirty bits into either table. */
    {
        const lib_u32 directory = 0x4003u, page = 0x5003u;
        lib_u32 copied = 0u;
        const lib_u8 nop = 0x90u;
        const core_machine_debug_register_patch setup = {
            .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CR3),
            .values = { [CORE_MACHINE_DEBUG_CR3] = 0x3000u }
        };
        lib_u32 cr0;

        failed |= core_machine_memory_write_physical(memory, 0x3000u,
            (lib_uptr)&directory, sizeof(directory)) != LIB_STATUS_OK;
        failed |= core_machine_memory_write_physical(memory, 0x4000u,
            (lib_uptr)&page, sizeof(page)) != LIB_STATUS_OK;
        failed |= core_machine_memory_write_physical(memory, 0x5000u,
            (lib_uptr)&nop, sizeof(nop)) != LIB_STATUS_OK;
        failed |= core_machine_debug_patch_registers(machine, &setup) != LIB_STATUS_OK ||
            core_machine_debug_read_register(machine, CORE_MACHINE_DEBUG_CR0,
                &cr0) != LIB_STATUS_OK ||
            core_machine_debug_write_register(machine, CORE_MACHINE_DEBUG_CR0,
                cr0 | VCPU_CR0_PG | VCPU_CR0_PE) != LIB_STATUS_OK;
        core_machine_cpu_execution_invalidate_prefetch(machine->executor_cpu_execution);
        failed |= !core_machine_cpu_execution_preview_lexeme(
            machine->executor_cpu_execution, &lexeme) || !lexeme.available ||
            lexeme.byte_count != 1u;
        failed |= core_machine_memory_inspect_physical(memory, 0x3000u,
            (lib_uptr)&copied, sizeof(copied), LIB_FALSE) != LIB_STATUS_OK ||
            copied != directory;
        failed |= core_machine_memory_inspect_physical(memory, 0x4000u,
            (lib_uptr)&copied, sizeof(copied), LIB_FALSE) != LIB_STATUS_OK ||
            copied != page;
    }
    core_machine_destroy(machine);
    return failed || !inspection_cache_execution() ? 1 : 0;
}
