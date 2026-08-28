/* Copyright 2012-2014 Neko. */

#ifndef CORE_MACHINE_MEMORY_H
#define CORE_MACHINE_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/machine/memory_interface.h"

typedef struct t_port t_port;
typedef struct core_machine_memory_test_allocation
    core_machine_memory_test_allocation;

#define CORE_MACHINE_DEVICE_RAM "Unknown Random-access Memory"

typedef struct {
    type_bool flagA20; /* 0 = disable, 1 = enable */
} t_ram_data;

typedef struct {
    type_unsigned_32 physical_start;
    type_unsigned_32 backing_start;
    type_native_unsigned bytes;
} core_machine_memory_mapping;

typedef struct {
    core_machine_memory_write_observer callback;
    C_VOID *owner;
} core_machine_memory_write_observer_slot;

#define CORE_MACHINE_MEMORY_MAPPING_CAPACITY 4u
#define CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY 4u
#define CORE_MACHINE_MEMORY_DEVICE_PROVIDER_INITIAL_CAPACITY 12u
#define CORE_MACHINE_MEMORY_DEVICE_PROVIDER_LIMIT 64u

typedef struct {
    type_unsigned_32 physical_start;
    type_native_unsigned bytes;
    core_machine_memory_device_read read;
    core_machine_memory_device_write write;
    core_machine_memory_device_query query;
    C_VOID *owner;
    type_bool overlay;
} core_machine_memory_device_provider;

typedef struct {
    type_virtual_address backing;
    type_virtual_address parity;
    type_native_unsigned installed_bytes;
    type_native_unsigned backing_capacity;
    type_native_unsigned parity_bytes;
    core_machine_memory_parity_fault_observer parity_fault;
    C_VOID *parity_owner;
    core_machine_memory_mapping mappings[CORE_MACHINE_MEMORY_MAPPING_CAPACITY];
    type_native_unsigned mapping_count;
    core_machine_memory_write_observer_slot
        write_observers[CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY];
    type_native_unsigned write_observer_count;
    core_machine_memory_device_provider *device_providers;
    type_native_unsigned device_provider_count;
    type_native_unsigned device_provider_capacity;
    core_machine_memory_test_allocation *device_provider_test_allocation;
    type_bool mappings_frozen;
    core_machine_a20_wrap_policy a20_wrap_policy;
} t_ram_connect;

typedef struct t_ram {
    t_ram_data data;
    t_ram_connect connect;
} t_ram;

/* Private test-only observation for one Core-owned allocation. */
struct core_machine_memory_test_allocation {
    type_bool fail;
    STD_SIZE_T attempts;
};

#define VRAM_BIT_A20  0x00100000
#define VRAM_FLAG_A20 0x02


type_status core_machine_memory_read_physical(t_ram *ram, type_unsigned_32 physical,
    type_virtual_address destination, type_native_unsigned size);
/* CPU reset-cache fetches are the one architectural access which precedes
 * board-controlled A20 routing.  This route accepts only an already-registered
 * immutable/device provider at the raw physical address; it never falls back
 * to RAM or changes ordinary memory-access semantics. */
type_status core_machine_memory_read_reset_physical(t_ram *ram,
    type_unsigned_32 physical, type_virtual_address destination,
    type_native_unsigned size);
type_status core_machine_memory_write_physical(t_ram *ram, type_unsigned_32 physical,
    type_virtual_address source, type_native_unsigned size);
type_status core_machine_memory_query_physical(const t_ram *ram,
    type_unsigned_32 physical, type_native_unsigned size,
    core_machine_memory_access access, core_machine_memory_route *out_route);
C_VOID core_machine_memory_initialize(t_ram *ram);
type_status core_machine_memory_initialize_for(t_ram *ram, STD_SIZE_T bytes,
    core_machine_memory_test_allocation *test_allocation);
C_VOID core_machine_memory_reset(t_ram *ram);
C_VOID core_machine_memory_finalize(t_ram *ram);
C_VOID core_machine_memory_register_ports(t_ram *ram, t_port *port);
type_status core_machine_memory_set_a20_wrap_policy(t_ram *ram,
    core_machine_a20_wrap_policy policy);


type_status core_machine_memory_allocate_for(t_ram *ram, STD_SIZE_T bytes);
type_status core_machine_memory_enable_parity(t_ram *ram, STD_SIZE_T bytes,
    core_machine_memory_parity_fault_observer fault, C_VOID *owner);
type_status core_machine_memory_register_mapping(t_ram *ram,
    type_unsigned_32 physical_start,
    type_unsigned_32 backing_start, STD_SIZE_T bytes);
type_status core_machine_memory_register_write_observer(t_ram *ram,
    core_machine_memory_write_observer callback, C_VOID *owner);
type_status core_machine_memory_register_device_provider(t_ram *ram,
    type_unsigned_32 physical_start, STD_SIZE_T bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner);
type_status core_machine_memory_register_overlay_device_provider(t_ram *ram,
    type_unsigned_32 physical_start, STD_SIZE_T bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner);type_status core_machine_memory_register_device_provider_and_write_observer(
    t_ram *ram, type_unsigned_32 physical_start, STD_SIZE_T bytes,
    core_machine_memory_device_read read, core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner,
    core_machine_memory_write_observer callback);
C_VOID core_machine_memory_freeze_mappings(t_ram *ram);
type_status core_machine_memory_read_real_from(t_ram *ram, type_unsigned_16 segment,
    type_unsigned_16 offset, C_VOID *out_data, STD_SIZE_T size);
type_status core_machine_memory_write_real_to(t_ram *ram, type_unsigned_16 segment,
    type_unsigned_16 offset, const C_VOID *in_data, STD_SIZE_T size);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
