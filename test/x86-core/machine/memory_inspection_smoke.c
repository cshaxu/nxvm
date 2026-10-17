#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "x86-core/machine/machine.h"
#include "x86-core/machine/debug_interface.h"

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
    x86_core_cpu_cache_operation cache_operations[2];
    lib_bool page_mode;
    lib_bool replace_page_entry;
    lib_bool page_lock_seen;
    lib_bool page_protocol_error;
    lib_u32 second_page_entry;
    lib_u32 pages[4];
    lib_u32 page_reads[4];
    lib_u32 page_writes[4];
    x86_core_machine *page_machine;
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
                    X86_CORE_MACHINE_TRANSACTION_OWNER_CPU &&
                    probe->page_machine->transaction.kind == X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK;
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
        if (probe->page_machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_CPU ||
            probe->page_machine->transaction.kind != X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK)
            probe->page_protocol_error = LIB_TRUE;
        lib_memory_copy((lib_u8 *)probe->pages + (address - 0x5000u),
            (const void *)source, bytes);
    }
    return probe->write_status;
}

static lib_status probe_query(void *opaque, lib_u32 address, lib_uptr bytes,
    x86_core_machine_memory_access access)
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
    x86_core_cpu_cache_operation operation)
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
            X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK) {
        lib_u16 value = 0x9999u;
        const lib_status status = x86_core_machine_dma_memory_cycle(probe->page_machine,
            0x3000u, 2u, 2u, X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, &value,
            LIB_NULL, LIB_NULL, LIB_NULL);
        ++probe->denied_dma_cycles;
        if (status == LIB_STATUS_OK || probe->page_machine->transaction.owner !=
                X86_CORE_MACHINE_TRANSACTION_OWNER_CPU || probe->page_machine->transaction.kind !=
                X86_CORE_MACHINE_TRANSACTION_CPU_MEMORY_LOCK) probe->dma_lock_error = LIB_TRUE;
    }
    return probe->control_status;
}

static void inspection_cpu_set(x86_core_cpu_execution_context *cpu,
    x86_core_cpu_debug_register id, lib_u32 value, lib_bool *failed)
{
    x86_core_cpu_debug_register_patch patch = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(id)
    };
    patch.values[id] = value;
    *failed |= x86_core_cpu_debug_patch_registers(cpu, &patch) != LIB_STATUS_OK;
}

static lib_bool inspection_cpu_is(x86_core_cpu_execution_context *cpu,
    x86_core_cpu_debug_register id, lib_u32 expected)
{
    lib_u32 value = 0u;
    return x86_core_cpu_debug_read_register(cpu, id, &value) == LIB_STATUS_OK &&
        value == expected;
}

static void inspection_cpu_reset(x86_core_cpu_execution_context *cpu,
    lib_bool *failed)
{
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR0)
    };
    x86_core_cpu_state_reset(cpu);
    *failed |= x86_core_cpu_debug_patch_registers(cpu, &entry) != LIB_STATUS_OK;
}

/* Machine owns this RAM: unsnooped test writes create stale hardware cache
 * contents after real CPU accesses, without borrowing CPU cache layout. */
static void inspection_unsnooped_write(x86_core_machine *machine, lib_u32 address,
    const void *data, lib_size bytes)
{
    lib_memory_copy((lib_u8 *)machine->executor_memory.connect.backing + address,
        data, bytes);
}

static lib_bool inspection_cache_execution(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 16u * 1024u * 1024u, .cpu_profile = X86_CORE_CPU_PROFILE_80486SX
    };
    const lib_u8 read[] = {0xa1u, 0u, 0x30u};
    const lib_u8 write[] = {0xa3u, 0u, 0x30u};
    const lib_u32 data_descriptor[] = {0x0000ffffu, 0x00009200u};
    x86_core_machine *machine = LIB_NULL;
    x86_core_cpu_execution_context *cpu;
    lib_u16 value = 0x1234u;
    lib_bool failed = LIB_FALSE;
    inspection_probe device = {0};
    const char *phase = "initial fill";

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return LIB_FALSE;
    const x86_core_machine_attachment attachment = {.context = &device,
        .finalize_devices = probe_finalize, .memory_cache_control = probe_memory_control};
    device.page_machine = machine;
    const x86_core_machine_memory_alias_config alias = {
        .physical_start = 0x8000u, .backing_start = 0x3000u, .bytes = 16u};
    const x86_core_machine_memory_device_route route = {.physical_start = 0x5000u,
        .bytes = 16u, .callbacks = {probe_read, probe_write, probe_query}};
    if (x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK ||
        x86_core_machine_install_memory_device_routes(machine, &route, 1u,
            LIB_NULL, LIB_NULL, &device) != LIB_STATUS_OK ||
        x86_core_machine_install_memory_aliases(machine, &alias, 1u, LIB_TRUE) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_reconfigure_memory(machine, 2u * 1024u * 1024u) != LIB_STATUS_OK ||
        machine->executor_memory.connect.cpu_snoop != x86_core_machine_cpu_bus_snoop_write ||
        machine->executor_memory.connect.cpu_snoop_context != machine) {
        x86_core_machine_destroy(machine);
        return LIB_FALSE;
    }
    cpu = machine->executor_cpu_execution;
    inspection_cpu_reset(cpu, &failed);
    failed |= x86_core_machine_memory_write(machine, 0u, read, sizeof(read)) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x3000u, &value, sizeof(value)) != LIB_STATUS_OK;
    x86_core_cpu_execution_refresh(cpu);
    failed |= !inspection_cpu_is(cpu, X86_CORE_CPU_DEBUG_EAX, value);
    if (failed) goto done;

    /* Prime real RAM before the provider claims it. Both failed and accepted
     * provider writes must invalidate that previously cached physical line. */
    for (lib_u8 fail_write = 0u; fail_write < 2u; ++fail_write) {
        phase = fail_write ? "failed provider write" : "accepted provider write";
        lib_u16 cached = 0u;
        device.query_status = LIB_STATUS_UNSUPPORTED;
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_CR0, 0u, &failed);
        failed |= x86_core_machine_memory_write(machine, 0x5000u, &value,
            sizeof(value)) != LIB_STATUS_OK ||
            x86_core_cpu_read_linear(cpu, 0x5000u, &cached, sizeof(cached)) != 0 ||
            cached != value;
        device.query_status = LIB_STATUS_OK;
        device.write_status = fail_write ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
        failed |= x86_core_machine_memory_write(machine, 0x5000u, &value,
            sizeof(value)) != device.write_status;
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_CR0, VCPU_CR0_CD | VCPU_CR0_NW, &failed);
        failed |= x86_core_cpu_read_linear(cpu, 0x5000u, &cached, sizeof(cached)) != 0 ||
            cached != 0x9090u;
        if (failed) goto done;
    }
    device.write_status = LIB_STATUS_OK;
    phase = "alias and DMA";
    inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_CR0, 0u, &failed);
    value = 0x4321u;
    failed |= x86_core_machine_memory_write(machine, 0x8000u, &value, sizeof(value)) != LIB_STATUS_OK;
    inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_EIP, 0u, &failed);
    x86_core_cpu_execution_refresh(cpu);
    failed |= !inspection_cpu_is(cpu, X86_CORE_CPU_DEBUG_EAX, value);
    value = 0x5678u;
    failed |= x86_core_machine_dma_memory_cycle(machine, 0x3000u, 2u, 2u,
        X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, &value, LIB_NULL, LIB_NULL, LIB_NULL) != LIB_STATUS_OK;
    inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_EIP, 0u, &failed);
    x86_core_cpu_execution_refresh(cpu);
    failed |= !inspection_cpu_is(cpu, X86_CORE_CPU_DEBUG_EAX, value);
    {
        const lib_u8 write_alias[] = {0xa3u, 0u, 0x80u};
        failed |= x86_core_machine_memory_write(machine, 0u, write_alias,
            sizeof(write_alias)) != LIB_STATUS_OK;
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_EIP, 0u, &failed);
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_EAX, 0x1357u, &failed);
        x86_core_cpu_execution_refresh(cpu);
        failed |= x86_core_machine_memory_write(machine, 0u, read, sizeof(read)) != LIB_STATUS_OK;
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_EIP, 0u, &failed);
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_EAX, 0u, &failed);
        x86_core_cpu_execution_refresh(cpu);
        failed |= !inspection_cpu_is(cpu, X86_CORE_CPU_DEBUG_EAX, 0x1357u);
    }
    failed |= x86_core_machine_memory_write(machine, 0u, write, sizeof(write)) != LIB_STATUS_OK;
    inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_EAX, 0xabcdu, &failed);
    inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_EIP, 0u, &failed);
    x86_core_cpu_execution_refresh(cpu);
    failed |= x86_core_machine_memory_read(machine, 0x3000u, &value, sizeof(value)) != LIB_STATUS_OK ||
        value != 0xabcdu;
    failed |= x86_core_machine_memory_write(machine, 0u, read, sizeof(read)) != LIB_STATUS_OK;
    inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_CR0, VCPU_CR0_CD | VCPU_CR0_NW, &failed);
    value = 0x8765u;
    failed |= x86_core_machine_memory_write(machine, 0x3000u, &value, sizeof(value)) != LIB_STATUS_OK;
    inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_EAX, 0x1357u, &failed);
    inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_EIP, 0u, &failed);
    x86_core_cpu_execution_refresh(cpu);
    /* The old value demonstrates a retained hit, not a new RAM fill. */
    failed |= !inspection_cpu_is(cpu, X86_CORE_CPU_DEBUG_EAX, 0xabcdu) ||
        x86_core_cpu_execution_consume_stop_request(cpu);
    if (failed) goto done;
    phase = "paged provider re-read";
    {
        const lib_u32 directory = 0x5023u;
        const lib_u16 old_value = 0x1234u, new_value = 0x5678u;
        const lib_u64 cancelled = machine->transaction.cancelled_count;

        inspection_cpu_reset(cpu, &failed);
        device.page_mode = LIB_TRUE;
        device.pages[0] = 0x23u;
        device.pages[3] = 0x9003u;
        device.replace_page_entry = LIB_TRUE;
        device.second_page_entry = 0xa003u;
        failed |= x86_core_machine_memory_write(machine, 0u, read, sizeof(read)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x10u, data_descriptor, sizeof(data_descriptor)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x4000u, &directory, sizeof(directory)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x9000u, &old_value, sizeof(old_value)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0xa000u, &new_value, sizeof(new_value)) != LIB_STATUS_OK;
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_CR3, 0x4000u, &failed);
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_CR0, VCPU_CR0_PE | VCPU_CR0_PG, &failed);
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_DS, 0x10u, &failed);
        x86_core_cpu_execution_refresh(cpu);
        failed |= !inspection_cpu_is(cpu, X86_CORE_CPU_DEBUG_EAX, new_value) ||
            !inspection_cpu_is(cpu, X86_CORE_CPU_DEBUG_EIP, sizeof(read)) ||
            x86_core_cpu_execution_consume_stop_request(cpu) || !device.page_lock_seen || device.page_protocol_error ||
            device.page_reads[3] != 2u || device.page_writes[3] != 1u || device.pages[3] != 0xa023u ||
            machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE ||
            machine->transaction.cancelled_count != cancelled;
        value = 0x6789u;
        failed |= x86_core_machine_memory_write(machine, 0xa000u, &value, sizeof(value)) != LIB_STATUS_OK;
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_EIP, 0u, &failed);
        x86_core_cpu_execution_refresh(cpu);
        failed |= !inspection_cpu_is(cpu, X86_CORE_CPU_DEBUG_EAX, value) ||
            device.page_reads[3] != 2u || device.page_writes[3] != 1u;
        failed |= x86_core_machine_memory_write(machine, 0u, write, sizeof(write)) != LIB_STATUS_OK;
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_EIP, 0u, &failed);
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_EAX, 0xbeefu, &failed);
        x86_core_cpu_execution_refresh(cpu);
        failed |= x86_core_machine_memory_read(machine, 0xa000u, &value, sizeof(value)) != LIB_STATUS_OK ||
            value != 0xbeefu || device.page_reads[3] != 4u || device.page_writes[3] != 2u ||
            device.pages[3] != 0xa063u || device.page_protocol_error ||
            x86_core_cpu_execution_consume_stop_request(cpu) ||
            machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE ||
            machine->transaction.cancelled_count != cancelled;
        failed |= x86_core_machine_memory_read(machine, 0x9000u, &value, sizeof(value)) != LIB_STATUS_OK ||
            value != old_value;
        if (failed) goto done;
    }
    /* Prime the stale PTE through a real CPU fill, then change owned RAM
     * without a snoop. The locked re-read must select RAM and update the hit. */
    for (lib_u8 cache_ram = 0u; cache_ram < 2u; ++cache_ram) {
        phase = cache_ram ? "cached PTE NW" : "cached PTE write-through";
        const lib_u32 directory = 0x11023u;
        const lib_u32 stale[4] = {0x23u, 0u, 0u, 0x9003u};
        const lib_u32 current = 0xa003u, poison = 0xcccc0023u;
        lib_u32 updated = 0u, cached = 0u;

        inspection_cpu_reset(cpu, &failed);
        failed |= x86_core_machine_memory_write(machine, 0u, read, sizeof(read)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x10u, data_descriptor, sizeof(data_descriptor)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x4000u, &directory, sizeof(directory)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x11000u, stale, sizeof(stale)) != LIB_STATUS_OK ||
            x86_core_cpu_read_linear(cpu, 0x1100cu, &cached, sizeof(cached)) != 0 || cached != stale[3];
        inspection_unsnooped_write(machine, 0x1100cu, &current, sizeof(current));
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_CR3, 0x4000u, &failed);
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_CR0, VCPU_CR0_PE | VCPU_CR0_PG |
            (cache_ram ? VCPU_CR0_CD | VCPU_CR0_NW : 0u), &failed);
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_DS, 0x10u, &failed);
        x86_core_cpu_execution_refresh(cpu);
        failed |= !inspection_cpu_is(cpu, X86_CORE_CPU_DEBUG_EAX, 0xbeefu) ||
            !inspection_cpu_is(cpu, X86_CORE_CPU_DEBUG_EIP, sizeof(read)) ||
            x86_core_cpu_execution_consume_stop_request(cpu) ||
            machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE;
        failed |= x86_core_machine_memory_read(machine, 0x1100cu, &updated, sizeof(updated)) != LIB_STATUS_OK ||
            updated != 0xa023u;
        inspection_unsnooped_write(machine, 0x1100cu, &poison, sizeof(poison));
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_CR0, VCPU_CR0_CD | VCPU_CR0_NW, &failed);
        failed |= x86_core_cpu_read_linear(cpu, 0x1100cu, &cached, sizeof(cached)) != 0 || cached != 0xa023u;
        if (failed) goto done;
    }
    for (lib_u8 cache_ram = 0u; cache_ram < 2u; ++cache_ram) {
        phase = cache_ram ? "locked alias NW" : "locked alias write-through";
        const lib_u8 exchange[] = {0x87u, 0x06u, 0u, 0x80u};
        const lib_u16 original = 0x1111u, stale_alias = 0x2222u, stale_backing = 0x4444u;
        const lib_u16 poison = 0x7777u;
        x86_core_cpu_instruction_observation operation;
        lib_u16 cached = 0u;

        /* Publish the real alias only after filling the two physical lines.
         * A selected=false RAM alias still maps backing; it is not disabled. */
        x86_core_machine_destroy(machine);
        machine = LIB_NULL;
        if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
            x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK) {
            failed = LIB_TRUE;
            goto done;
        }
        device.page_machine = machine;
        device.request_dma_locked = LIB_FALSE;
        cpu = machine->executor_cpu_execution;
        inspection_cpu_reset(cpu, &failed);
        inspection_unsnooped_write(machine, 0u, exchange, sizeof(exchange));
        inspection_unsnooped_write(machine, 0x8000u, &stale_alias, sizeof(stale_alias));
        inspection_unsnooped_write(machine, 0x3000u, &stale_backing, sizeof(stale_backing));
        failed |= x86_core_cpu_read_linear(cpu, 0x8000u, &cached, sizeof(cached)) != 0 || cached != stale_alias ||
            x86_core_cpu_read_linear(cpu, 0x3000u, &cached, sizeof(cached)) != 0 || cached != stale_backing ||
            x86_core_machine_install_memory_aliases(machine, &alias, 1u, LIB_TRUE) != LIB_STATUS_OK ||
            x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
        inspection_unsnooped_write(machine, 0x3000u, &original, sizeof(original));
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_CR0,
            cache_ram ? VCPU_CR0_CD | VCPU_CR0_NW : 0u, &failed);
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_EAX, 0x3333u, &failed);
        device.request_dma_locked = LIB_TRUE;
        device.denied_dma_cycles = 0u;
        device.dma_lock_error = LIB_FALSE;
        x86_core_cpu_execution_refresh(cpu);
        x86_core_cpu_execution_copy_observation(cpu, &operation);
        failed |= !inspection_cpu_is(cpu, X86_CORE_CPU_DEBUG_EAX, original) ||
            !inspection_cpu_is(cpu, X86_CORE_CPU_DEBUG_EIP, sizeof(exchange)) ||
            x86_core_cpu_execution_consume_stop_request(cpu) || operation.lock_prefix ||
            machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE ||
            device.denied_dma_cycles != 2u || device.dma_lock_error;
        failed |= x86_core_machine_memory_read_physical(&machine->executor_memory,
            0x3000u, (lib_uptr)&value, sizeof(value)) != LIB_STATUS_OK ||
            value != 0x3333u;
        inspection_unsnooped_write(machine, 0x3000u, &poison, sizeof(poison));
        inspection_cpu_set(cpu, X86_CORE_CPU_DEBUG_CR0, VCPU_CR0_CD | VCPU_CR0_NW, &failed);
        failed |= x86_core_cpu_read_linear(cpu, 0x8000u, &cached, sizeof(cached)) != 0 || cached != 0x3333u ||
            x86_core_cpu_read_linear(cpu, 0x3000u, &cached, sizeof(cached)) != 0 ||
            cached != (cache_ram ? stale_backing : poison) || device.denied_dma_cycles != 2u;
        if (failed) goto done;
    }
done:
    if (failed) lib_c_printf("INSPECTION:CACHE:%s:reads=%u:writes=%u:locked=%u\n",
        phase, device.page_reads[3], device.page_writes[3], device.denied_dma_cycles);
    x86_core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 16u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .address_decode = {0u, LIB_UINT32_MAX, 0x00100000u}
    };
    x86_core_machine *machine = LIB_NULL;
    x86_core_cpu_instruction_lexeme lexeme;
    inspection_probe probe = {0};
    lib_u8 bytes[4] = {0};
    lib_u8 expected[4] = {0x11u, 0x90u, 0x90u, 0x22u};
    t_ram *memory;
    lib_bool failed = LIB_FALSE;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
    memory = &machine->executor_memory;
    /* With no external cache, Core still completes the special transaction. */
    failed |= x86_core_machine_cpu_bus.cache_operation(machine,
        X86_CORE_CPU_CACHE_FLUSH) != LIB_STATUS_OK ||
        machine->transaction.committed_count != 1u;
    {
        const x86_core_machine_attachment attachment = {.context = &probe,
            .finalize_devices = probe_finalize, .cache_operation = probe_cache_operation,
            .memory_cache_control = probe_memory_control};
        failed |= x86_core_machine_bind_attachment(machine, &attachment) != LIB_STATUS_OK;
    }
    failed |= x86_core_machine_memory_register_device_provider(memory, 0x1001u, 2u,
        probe_read, probe_write, probe_query, &probe) != LIB_STATUS_OK;
    failed |= x86_core_machine_memory_register_overlay_device_provider(memory,
        0xfffffff0u, 16u, probe_read, probe_write, probe_query, &probe) != LIB_STATUS_OK;
    failed |= x86_core_machine_memory_enable_parity(memory, 0x2000u,
        probe_parity, &probe) != LIB_STATUS_OK;
    {
        const x86_core_machine_memory_alias_config alias = {
            .physical_start = 0x8000u, .backing_start = 0x3000u, .bytes = 16u};
        failed |= x86_core_machine_install_memory_aliases(machine, &alias, 1u,
            LIB_TRUE) != LIB_STATUS_OK;
    }
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    if (failed) { x86_core_machine_destroy(machine); return 1; }

    {
        const lib_u8 value = 0xa5u;
        lib_u8 observed = 0xffu;
        const lib_u64 cancelled = machine->transaction.cancelled_count;
        probe.control_status = LIB_STATUS_IO_ERROR;
        failed |= x86_core_machine_cpu_bus.write_memory(machine, 0x3010u, &value, 1u,
            X86_CORE_CPU_MEMORY_ACCESS_DATA, 0x18u) != LIB_STATUS_IO_ERROR ||
            probe.last_control != 0x18u || probe.control_count != 1u ||
            machine->transaction.cancelled_count != cancelled + 1u;
        probe.control_status = LIB_STATUS_OK;
        failed |= x86_core_machine_cpu_bus.read_memory(machine, 0x3010u, &observed, 1u,
            X86_CORE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 0x10u) != LIB_STATUS_OK ||
            observed != 0u || probe.last_control != 0x10u || probe.control_count != 2u;
        failed |= x86_core_machine_cpu_bus.read_memory(machine, 0x3010u, &observed, 1u,
            X86_CORE_CPU_MEMORY_ACCESS_DATA, LIB_TRUE, LIB_FALSE, 0x18u) != LIB_STATUS_OK ||
            probe.control_count != 2u;
        failed |= x86_core_machine_cpu_bus.write_memory(machine, 0x3010u, &value, 1u,
            X86_CORE_CPU_MEMORY_ACCESS_DATA, 1u) != LIB_STATUS_INVALID_ARGUMENT ||
            probe.control_count != 2u ||
            machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE;
    }

    {
        const lib_u64 committed = machine->transaction.committed_count;
        const lib_u64 cancelled = machine->transaction.cancelled_count;
        probe.cache_fail = LIB_TRUE;
        failed |= x86_core_machine_cpu_bus.cache_operation(machine,
            X86_CORE_CPU_CACHE_WRITEBACK) != LIB_STATUS_IO_ERROR ||
            machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE ||
            machine->transaction.cancelled_count != cancelled + 1u ||
            machine->transaction.committed_count != committed;
        probe.cache_fail = LIB_FALSE;
        failed |= x86_core_machine_cpu_bus.cache_operation(machine,
            X86_CORE_CPU_CACHE_FLUSH) != LIB_STATUS_OK ||
            machine->transaction.committed_count != committed + 1u ||
            probe.cache_count != 2u ||
            probe.cache_operations[0] != X86_CORE_CPU_CACHE_WRITEBACK ||
            probe.cache_operations[1] != X86_CORE_CPU_CACHE_FLUSH;
        failed |= x86_core_machine_cpu_bus.cache_operation(machine,
            (x86_core_cpu_cache_operation)2) != LIB_STATUS_INVALID_ARGUMENT ||
            probe.cache_count != 2u;
    }

    /* Use the production CPU bus query: a narrow provider inside the line
     * disables fills even when the requested first byte is ordinary RAM. */
    {
        x86_core_cpu_cache_memory cache;
        const lib_u32 queries = probe.queries;

        failed |= x86_core_machine_cpu_bus.query_cache_memory(machine, 0x103010u,
            X86_CORE_CPU_CACHE_LOOKUP, &cache) != LIB_STATUS_OK ||
            cache.address != 0x3010u || cache.fill_enabled || probe.queries != queries;
        failed |= x86_core_machine_cpu_bus.query_cache_memory(machine, 0xfffffff0u,
            X86_CORE_CPU_CACHE_RESET_LOOKUP, &cache) != LIB_STATUS_OK ||
            cache.address != 0xfffffff0u || cache.fill_enabled || probe.queries != queries;
        failed |= x86_core_machine_cpu_bus.query_cache_memory(machine, 0x1000u,
            (x86_core_cpu_cache_query)3, &cache) != LIB_STATUS_INVALID_ARGUMENT ||
            probe.queries != queries;

        failed |= x86_core_machine_cpu_bus.query_cache_memory(machine, 0x1000u,
            X86_CORE_CPU_CACHE_FILL_PROBE, &cache) != LIB_STATUS_OK || cache.address != 0x1000u ||
            cache.fill_enabled;
        failed |= x86_core_machine_cpu_bus.query_cache_memory(machine, 0x3010u,
            X86_CORE_CPU_CACHE_FILL_PROBE, &cache) != LIB_STATUS_OK || cache.address != 0x3010u ||
            !cache.fill_enabled;
        failed |= x86_core_machine_cpu_bus.query_cache_memory(machine, 0x103010u,
            X86_CORE_CPU_CACHE_FILL_PROBE, &cache) != LIB_STATUS_OK || cache.address != 0x3010u ||
            !cache.fill_enabled;
        failed |= x86_core_machine_cpu_bus.query_cache_memory(machine, 0xfffffff0u,
            X86_CORE_CPU_CACHE_RESET_LOOKUP, &cache) != LIB_STATUS_OK || cache.address != 0xfffffff0u ||
            cache.fill_enabled || probe.reads != 0u || probe.parity != 0u;
        failed |= x86_core_machine_cpu_bus.query_cache_memory(machine, 0x8000u,
            X86_CORE_CPU_CACHE_FILL_PROBE, &cache) != LIB_STATUS_OK || cache.fill_enabled;
        probe.query_status = LIB_STATUS_IO_ERROR;
        failed |= x86_core_machine_cpu_bus.query_cache_memory(machine, 0x1000u,
            X86_CORE_CPU_CACHE_FILL_PROBE, &cache) != LIB_STATUS_OK || cache.fill_enabled;
        failed |= x86_core_machine_memory_read_physical(memory, 0x1001u,
            (lib_uptr)bytes, 1u) != LIB_STATUS_IO_ERROR || probe.reads != 0u;
        probe.query_status = LIB_STATUS_OK;
        failed |= x86_core_machine_signal_address_clear_mask(machine, 0u) != LIB_STATUS_OK;
        failed |= x86_core_machine_cpu_bus.query_cache_memory(machine, 0x103010u,
            X86_CORE_CPU_CACHE_FILL_PROBE, &cache) != LIB_STATUS_OK || cache.address != 0x103010u;
        failed |= x86_core_machine_signal_address_clear_mask(machine, 0x00100000u) != LIB_STATUS_OK;
    }

    /* The real Core transaction owns the entire page-table read/write pair. */
    {
        const lib_u64 committed = machine->transaction.committed_count;
        const lib_u64 cancelled = machine->transaction.cancelled_count;
        lib_u32 entry = 0u;
        const lib_u32 updated = 0x12345623u;

        failed |= x86_core_machine_cpu_bus.begin_memory_lock(machine) != LIB_STATUS_OK;
        failed |= x86_core_machine_cpu_bus.read_memory(machine, 0x3010u, &entry, 4u,
            X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ, LIB_FALSE, LIB_FALSE, 0u) != LIB_STATUS_OK ||
            machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_CPU ||
            machine->transaction.committed_count != committed;
        failed |= x86_core_machine_transaction_begin(&machine->transaction,
            X86_CORE_MACHINE_TRANSACTION_OWNER_DMA, X86_CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE,
            0x3010u, 4u, 0u) == LIB_STATUS_OK;
        failed |= x86_core_machine_cpu_bus.read_memory(machine, 0x3010u, &entry, 4u,
            X86_CORE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 0u) != LIB_STATUS_OK;
        failed |= x86_core_machine_cpu_bus.write_memory(machine, 0x3010u, &updated, 4u,
            X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE, 0u) != LIB_STATUS_OK ||
            machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_CPU ||
            machine->transaction.committed_count != committed;
        x86_core_machine_cpu_bus.complete_memory_lock(machine, LIB_TRUE);
        failed |= machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE ||
            machine->transaction.committed_count != committed + 1u ||
            machine->transaction.cancelled_count != cancelled;
        failed |= x86_core_machine_memory_inspect_physical(memory, 0x3010u,
            (lib_uptr)&entry, 4u, LIB_FALSE) != LIB_STATUS_OK || entry != updated;

        for (lib_u8 write = 0u; write < 2u; ++write) {
            const lib_u64 before_cancel = machine->transaction.cancelled_count;
            failed |= x86_core_machine_cpu_bus.begin_memory_lock(machine) != LIB_STATUS_OK;
            probe.control_status = LIB_STATUS_IO_ERROR;
            if (write)
                failed |= x86_core_machine_cpu_bus.write_memory(machine, 0x3010u, &updated, 4u,
                    X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE, 0u) != LIB_STATUS_IO_ERROR;
            else
                failed |= x86_core_machine_cpu_bus.read_memory(machine, 0x3010u, &entry, 4u,
                    X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ, LIB_FALSE, LIB_FALSE, 0u) != LIB_STATUS_IO_ERROR;
            failed |= machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_CPU ||
                machine->transaction.cancelled_count != before_cancel;
            x86_core_machine_cpu_bus.complete_memory_lock(machine, LIB_FALSE);
            failed |= machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE ||
                machine->transaction.cancelled_count != before_cancel + 1u;
            probe.control_status = LIB_STATUS_OK;
        }
        failed |= x86_core_machine_cpu_bus.begin_memory_lock(LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
            x86_core_machine_cpu_bus.begin_memory_lock(machine) != LIB_STATUS_OK ||
            x86_core_machine_cpu_bus.begin_memory_lock(machine) != LIB_STATUS_INVALID_ARGUMENT;
        failed |= x86_core_machine_cpu_bus.read_memory(machine, 0x3011u, &entry, 1u,
            X86_CORE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 1u) != LIB_STATUS_INVALID_ARGUMENT ||
            machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_CPU;
        x86_core_machine_cpu_bus.complete_memory_lock(machine, LIB_FALSE);
        failed |= machine->transaction.owner != X86_CORE_MACHINE_TRANSACTION_OWNER_NONE;
    }

    /* Corrupt backing without updating parity, as a same-owner hardware fixture. */
    ((lib_u8 *)memory->connect.backing)[0x1000u] = expected[0];
    ((lib_u8 *)memory->connect.backing)[0x1003u] = expected[3];
    ((lib_u8 *)memory->connect.parity)[0x1000u] ^= 1u;
    failed |= x86_core_machine_memory_inspect_physical(memory, 0x1000u,
        (lib_uptr)bytes, sizeof(bytes), LIB_FALSE) != LIB_STATUS_OK;
    failed |= lib_memory_compare(bytes, expected, sizeof(bytes)) != 0 ||
        probe.reads != 0u || probe.parity != 0u;
    failed |= x86_core_machine_memory_read_physical(memory, 0x1000u,
        (lib_uptr)bytes, sizeof(bytes)) != LIB_STATUS_OK;
    failed |= lib_memory_compare(bytes, expected, sizeof(bytes)) != 0 ||
        probe.reads != 2u || probe.parity == 0u;
    probe.reads = probe.parity = 0u;

    /* Reset preview selects high ROM without A20 wrap or callback side effects. */
    failed |= !x86_core_cpu_execution_preview_lexeme(
        machine->executor_cpu_execution, &lexeme) || !lexeme.available ||
        lexeme.byte_count != 1u || probe.reads != 0u || probe.parity != 0u;
    failed |= x86_core_machine_memory_read_reset_physical(memory, 0xfffffff0u,
        (lib_uptr)bytes, 1u) != LIB_STATUS_OK || bytes[0] != 0x90u || probe.reads != 1u;
    probe.reads = 0u;
    probe.status = LIB_STATUS_IO_ERROR;
    failed |= x86_core_machine_memory_inspect_physical(memory, 0x1001u,
        (lib_uptr)bytes, 1u, LIB_FALSE) != LIB_STATUS_IO_ERROR || probe.reads != 0u;
    failed |= x86_core_cpu_execution_preview_lexeme(
        machine->executor_cpu_execution, &lexeme) || lexeme.available ||
        probe.reads != 0u;
    probe.status = LIB_STATUS_OK;
    failed |= x86_core_machine_memory_inspect_physical(memory, 0x101001u,
        (lib_uptr)bytes, 1u, LIB_FALSE) != LIB_STATUS_OK || bytes[0] != 0x90u;
    failed |= x86_core_machine_memory_inspect_physical(memory, 0u,
        (lib_uptr)bytes, 0u, LIB_FALSE) != LIB_STATUS_INVALID_ARGUMENT;

    /* A paged preview must not publish accessed/dirty bits into either table. */
    {
        const lib_u32 directory = 0x4003u, page = 0x5003u;
        lib_u32 copied = 0u;
        const lib_u8 nop = 0x90u;
        const x86_core_cpu_debug_register_patch setup = {
            .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
                X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
                X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
                X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
                X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
                X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR3),
            .values = { [X86_CORE_CPU_DEBUG_CR3] = 0x3000u }
        };
        lib_u32 cr0;

        failed |= x86_core_machine_memory_write_physical(memory, 0x3000u,
            (lib_uptr)&directory, sizeof(directory)) != LIB_STATUS_OK;
        failed |= x86_core_machine_memory_write_physical(memory, 0x4000u,
            (lib_uptr)&page, sizeof(page)) != LIB_STATUS_OK;
        failed |= x86_core_machine_memory_write_physical(memory, 0x5000u,
            (lib_uptr)&nop, sizeof(nop)) != LIB_STATUS_OK;
        failed |= x86_core_machine_debug_patch_registers(machine, &setup) != LIB_STATUS_OK ||
            x86_core_machine_debug_read_register(machine, X86_CORE_CPU_DEBUG_CR0,
                &cr0) != LIB_STATUS_OK ||
            x86_core_machine_debug_write_register(machine, X86_CORE_CPU_DEBUG_CR0,
                cr0 | VCPU_CR0_PG | VCPU_CR0_PE) != LIB_STATUS_OK;
        x86_core_cpu_execution_invalidate_prefetch(machine->executor_cpu_execution);
        failed |= !x86_core_cpu_execution_preview_lexeme(
            machine->executor_cpu_execution, &lexeme) || !lexeme.available ||
            lexeme.byte_count != 1u;
        failed |= x86_core_machine_memory_inspect_physical(memory, 0x3000u,
            (lib_uptr)&copied, sizeof(copied), LIB_FALSE) != LIB_STATUS_OK ||
            copied != directory;
        failed |= x86_core_machine_memory_inspect_physical(memory, 0x4000u,
            (lib_uptr)&copied, sizeof(copied), LIB_FALSE) != LIB_STATUS_OK ||
            copied != page;
    }
    x86_core_machine_destroy(machine);
    return failed || !inspection_cache_execution() ? 1 : 0;
}
