/* Copyright 2012-2014 Neko. */

#ifndef X86_CORE_MACHINE_MEMORY_H
#define X86_CORE_MACHINE_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"


#include "x86-core/machine/memory_interface.h"

typedef struct x86_core_machine_memory_test_allocation
    x86_core_machine_memory_test_allocation;

#define X86_CORE_MACHINE_DEVICE_RAM "Unknown Random-access Memory"

typedef struct {
    lib_u32 address_clear_mask;
} t_ram_data;

typedef struct {
    lib_u32 physical_start;
    lib_u32 backing_start;
    lib_uptr bytes;
    lib_u8 selected;
    lib_bool writable;
} x86_core_machine_memory_mapping;

typedef struct {
    x86_core_machine_memory_write_observer callback;
    void *owner;
} x86_core_machine_memory_write_observer_slot;

#define X86_CORE_MACHINE_MEMORY_MAPPING_CAPACITY 4u
#define X86_CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY 4u
#define X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY 12u
#define X86_CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT 64u

typedef struct {
    lib_u32 physical_start;
    lib_uptr bytes;
    x86_core_machine_memory_device_read read;
    x86_core_machine_memory_device_instruction_read read_instruction;
    x86_core_machine_memory_device_write write;
    x86_core_machine_memory_device_query query;
    x86_core_machine_memory_device_failure_observer failure;
    void *owner;
    lib_u8 overlay;
    /* Immutable reset-ROM alias decoded at its raw physical address before
     * ordinary address-line decoding. */
    lib_u8 raw_decode;
    lib_bool cpu_only;
    /* A selected board decode that replaces an otherwise valid lower route. */
    lib_u8 replacement;
    /* An open-bus board window used only when no installed device decodes the
     * access. It is never a reset-ROM source. */
    lib_u8 fallback;
} x86_core_machine_memory_device_provider;

typedef struct {
    lib_uptr backing;
    lib_uptr parity;
    lib_uptr installed_bytes;
    lib_uptr backing_capacity;
    lib_uptr parity_bytes;
    x86_core_machine_memory_parity_fault_observer parity_fault;
    void *parity_owner;
    x86_core_machine_memory_mapping mappings[X86_CORE_MACHINE_MEMORY_MAPPING_CAPACITY];
    lib_uptr mapping_count;
    x86_core_machine_memory_write_observer_slot
        write_observers[X86_CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY];
    lib_uptr write_observer_count;
    x86_core_machine_memory_write_observer cpu_snoop;
    void *cpu_snoop_context;
    x86_core_machine_memory_device_provider *device_providers;
    lib_uptr device_provider_count;
    lib_uptr device_provider_capacity;
    x86_core_machine_memory_test_allocation *device_provider_test_allocation;
    lib_u8 mappings_frozen;
    x86_core_machine_address_decode address_decode;
} t_ram_connect;

typedef struct t_ram {
    t_ram_data data;
    t_ram_connect connect;
} t_ram;

/* Private test-only observation for one Core-owned allocation. */
struct x86_core_machine_memory_test_allocation {
    lib_u8 fail;
    lib_size attempts;
};



lib_status x86_core_machine_memory_read_physical(t_ram *ram, lib_u32 physical,
    lib_uptr destination, lib_uptr size, lib_bool cpu_access);
/* CPU instruction fetches select a provider's optional immutable executable
 * source.  Providers without one retain ordinary read behavior. */
lib_status x86_core_machine_memory_read_instruction_physical(t_ram *ram,
    lib_u32 physical, lib_uptr destination, lib_uptr size);
/* Same physical route and bytes, without device or parity side effects.
 * reset_fetch first selects the architected reset alias, as CPU fetch does. */
lib_status x86_core_machine_memory_inspect_physical(t_ram *ram, lib_u32 physical,
    lib_uptr destination, lib_uptr size, lib_bool reset_fetch);
/* CPU reset-cache fetches are the one architectural access which precedes
 * configured address-line decoding.  This route accepts only an already-registered
 * immutable/device provider at the raw physical address; it never falls back
 * to RAM or changes ordinary memory-access semantics. */
lib_status x86_core_machine_memory_read_reset_physical(t_ram *ram,
    lib_u32 physical, lib_uptr destination,
    lib_uptr size);
lib_status x86_core_machine_memory_write_physical(t_ram *ram, lib_u32 physical,
    lib_uptr source, lib_uptr size, lib_bool cpu_access);
lib_status x86_core_machine_memory_query_physical(const t_ram *ram,
    lib_u32 physical, lib_uptr size,
    x86_core_machine_memory_access access, x86_core_machine_memory_route *out_route,
    lib_bool cpu_access);
lib_status x86_core_machine_memory_query_cpu_cache(const t_ram *ram,
    lib_u32 physical, lib_bool reset_fetch, lib_bool check_fill,
    lib_u32 *out_address, lib_bool *out_fill_enabled);
lib_status x86_core_machine_memory_initialize_for(t_ram *ram, lib_size bytes,
    x86_core_machine_memory_test_allocation *test_allocation);
void x86_core_machine_memory_reset(t_ram *ram);
void x86_core_machine_memory_finalize(t_ram *ram);


lib_status x86_core_machine_memory_allocate_for(t_ram *ram, lib_size bytes);
lib_status x86_core_machine_memory_enable_parity(t_ram *ram, lib_size bytes,
    x86_core_machine_memory_parity_fault_observer fault, void *owner);
void x86_core_machine_memory_release_parity(t_ram *ram);
lib_status x86_core_machine_memory_register_mapping(t_ram *ram,
    lib_u32 physical_start,
    lib_u32 backing_start, lib_size bytes, lib_u8 selected);
lib_status x86_core_machine_memory_register_write_observer(t_ram *ram,
    x86_core_machine_memory_write_observer callback, void *owner);
lib_status x86_core_machine_memory_register_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    x86_core_machine_memory_device_read read, x86_core_machine_memory_device_write write,
    x86_core_machine_memory_device_query query, void *owner,
    x86_core_machine_memory_device_failure_observer failure);
lib_status x86_core_machine_memory_register_overlay_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    x86_core_machine_memory_device_read read, x86_core_machine_memory_device_write write,
    x86_core_machine_memory_device_query query, void *owner,
    x86_core_machine_memory_device_failure_observer failure);
lib_status x86_core_machine_memory_register_raw_overlay_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    x86_core_machine_memory_device_read read, x86_core_machine_memory_device_write write,
    x86_core_machine_memory_device_query query, void *owner,
    x86_core_machine_memory_device_failure_observer failure);
lib_status x86_core_machine_memory_register_replacement_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    x86_core_machine_memory_device_read read, x86_core_machine_memory_device_write write,
    x86_core_machine_memory_device_query query, void *owner,
    x86_core_machine_memory_device_failure_observer failure);
lib_status x86_core_machine_memory_register_fallback_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    x86_core_machine_memory_device_read read, x86_core_machine_memory_device_write write,
    x86_core_machine_memory_device_query query, void *owner,
    x86_core_machine_memory_device_failure_observer failure);
void x86_core_machine_memory_freeze_mappings(t_ram *ram);
/* Exclusive construction/teardown only; remove this owner's device routes and
 * write observers before destroying the callback context. */
void x86_core_machine_memory_unregister_owner(t_ram *ram, const void *owner);
lib_status x86_core_machine_memory_read_real_from(t_ram *ram, lib_u16 segment,
    lib_u16 offset, void *out_data, lib_size size);
lib_status x86_core_machine_memory_write_real_to(t_ram *ram, lib_u16 segment,
    lib_u16 offset, const void *in_data, lib_size size);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
