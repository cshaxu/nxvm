/* Copyright 2012-2014 Neko. */

#ifndef CORE_MACHINE_MEMORY_H
#define CORE_MACHINE_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"


#include "app-nxvm/devices/memory_interface.h"

typedef struct t_port t_port;
typedef struct core_machine_memory_test_allocation
    core_machine_memory_test_allocation;

#define CORE_MACHINE_DEVICE_RAM "Unknown Random-access Memory"

typedef struct {
    lib_u8 flagA20; /* 0 = disable, 1 = enable */
} t_ram_data;

typedef struct {
    lib_u32 physical_start;
    lib_u32 backing_start;
    lib_uptr bytes;
    lib_u8 selected;
} core_machine_memory_mapping;

typedef struct {
    core_machine_memory_write_observer callback;
    void *owner;
} core_machine_memory_write_observer_slot;

#define CORE_MACHINE_MEMORY_MAPPING_CAPACITY 4u
#define CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY 4u
#define CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY 12u
#define CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT 64u

typedef struct {
    lib_u32 physical_start;
    lib_uptr bytes;
    core_machine_memory_device_read read;
    core_machine_memory_device_write write;
    core_machine_memory_device_query query;
    void *owner;
    lib_u8 overlay;
    /* Immutable reset-ROM alias decoded at its raw physical address before
     * ordinary board A20 routing. */
    lib_u8 pre_a20;
    /* A selected board decode that replaces an otherwise valid lower route. */
    lib_u8 replacement;
    /* An open-bus board window used only when no installed device decodes the
     * access. It is never a reset-ROM source. */
    lib_u8 fallback;
} core_machine_memory_device_provider;

typedef struct {
    lib_uptr backing;
    lib_uptr parity;
    lib_uptr installed_bytes;
    lib_uptr backing_capacity;
    lib_uptr parity_bytes;
    core_machine_memory_parity_fault_observer parity_fault;
    void *parity_owner;
    core_machine_memory_mapping mappings[CORE_MACHINE_MEMORY_MAPPING_CAPACITY];
    lib_uptr mapping_count;
    core_machine_memory_write_observer_slot
        write_observers[CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY];
    lib_uptr write_observer_count;
    core_machine_memory_device_provider *device_providers;
    lib_uptr device_provider_count;
    lib_uptr device_provider_capacity;
    core_machine_memory_test_allocation *device_provider_test_allocation;
    lib_u8 mappings_frozen;
    core_machine_a20_wrap_policy a20_wrap_policy;
} t_ram_connect;

typedef struct t_ram {
    t_ram_data data;
    t_ram_connect connect;
} t_ram;

/* Private test-only observation for one Core-owned allocation. */
struct core_machine_memory_test_allocation {
    lib_u8 fail;
    lib_size attempts;
};

#define VRAM_BIT_A20  0x00100000
#define VRAM_FLAG_A20 0x02


lib_status core_machine_memory_read_physical(t_ram *ram, lib_u32 physical,
    lib_uptr destination, lib_uptr size);
/* CPU reset-cache fetches are the one architectural access which precedes
 * board-controlled A20 routing.  This route accepts only an already-registered
 * immutable/device provider at the raw physical address; it never falls back
 * to RAM or changes ordinary memory-access semantics. */
lib_status core_machine_memory_read_reset_physical(t_ram *ram,
    lib_u32 physical, lib_uptr destination,
    lib_uptr size);
lib_status core_machine_memory_write_physical(t_ram *ram, lib_u32 physical,
    lib_uptr source, lib_uptr size);
lib_status core_machine_memory_query_physical(const t_ram *ram,
    lib_u32 physical, lib_uptr size,
    core_machine_memory_access access, core_machine_memory_route *out_route);
lib_status core_machine_memory_initialize_for(t_ram *ram, lib_size bytes,
    core_machine_memory_test_allocation *test_allocation);
void core_machine_memory_reset(t_ram *ram);
void core_machine_memory_finalize(t_ram *ram);
void core_machine_memory_register_ports(t_ram *ram, t_port *port);
lib_status core_machine_memory_set_a20_wrap_policy(t_ram *ram,
    core_machine_a20_wrap_policy policy);


lib_status core_machine_memory_allocate_for(t_ram *ram, lib_size bytes);
lib_status core_machine_memory_enable_parity(t_ram *ram, lib_size bytes,
    core_machine_memory_parity_fault_observer fault, void *owner);
lib_status core_machine_memory_register_mapping(t_ram *ram,
    lib_u32 physical_start,
    lib_u32 backing_start, lib_size bytes, lib_u8 selected);
lib_status core_machine_memory_register_write_observer(t_ram *ram,
    core_machine_memory_write_observer callback, void *owner);
lib_status core_machine_memory_register_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, void *owner);
lib_status core_machine_memory_register_overlay_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, void *owner);
lib_status core_machine_memory_register_pre_a20_overlay_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, void *owner);
lib_status core_machine_memory_register_replacement_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, void *owner);
lib_status core_machine_memory_register_fallback_device_provider(t_ram *ram,
    lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, void *owner);
lib_status core_machine_memory_register_device_provider_and_write_observer(
    t_ram *ram, lib_u32 physical_start, lib_size bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, void *owner,
    core_machine_memory_write_observer callback);
void core_machine_memory_freeze_mappings(t_ram *ram);
lib_status core_machine_memory_read_real_from(t_ram *ram, lib_u16 segment,
    lib_u16 offset, void *out_data, lib_size size);
lib_status core_machine_memory_write_real_to(t_ram *ram, lib_u16 segment,
    lib_u16 offset, const void *in_data, lib_size size);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
