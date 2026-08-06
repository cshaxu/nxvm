/* Copyright 2012-2014 Neko. */

#ifndef CORE_MACHINE_MEMORY_H
#define CORE_MACHINE_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

typedef struct t_port t_port;

#define CORE_MACHINE_DEVICE_RAM "Unknown Random-access Memory"

typedef struct {
    type_bool flagA20; /* 0 = disable, 1 = enable */
} t_ram_data;

typedef struct {
    type_unsigned_32 physical_start;
    type_unsigned_32 backing_start;
    type_native_unsigned bytes;
} core_machine_memory_mapping;

typedef C_VOID (*core_machine_memory_write_observer)(C_VOID *owner,
    type_unsigned_32 physical, type_native_unsigned bytes);

typedef struct {
    core_machine_memory_write_observer callback;
    C_VOID *owner;
} core_machine_memory_write_observer_slot;

#define CORE_MACHINE_MEMORY_MAPPING_CAPACITY 4u
#define CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY 4u

typedef struct {
    type_virtual_address backing;
    type_native_unsigned installed_bytes;
    type_native_unsigned backing_capacity;
    core_machine_memory_mapping mappings[CORE_MACHINE_MEMORY_MAPPING_CAPACITY];
    type_native_unsigned mapping_count;
    core_machine_memory_write_observer_slot
        write_observers[CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY];
    type_native_unsigned write_observer_count;
    type_bool mappings_frozen;
} t_ram_connect;

typedef struct t_ram {
    t_ram_data data;
    t_ram_connect connect;
} t_ram;

#define VRAM_BIT_A20  0x00100000
#define VRAM_FLAG_A20 0x02


type_status core_machine_memory_read_physical(t_ram *ram, type_unsigned_32 physical,
    type_virtual_address destination, type_native_unsigned size);
type_status core_machine_memory_write_physical(t_ram *ram, type_unsigned_32 physical,
    type_virtual_address source, type_native_unsigned size);
C_VOID core_machine_memory_initialize(t_ram *ram);
C_VOID core_machine_memory_reset(t_ram *ram);
C_VOID core_machine_memory_finalize(t_ram *ram);
C_VOID core_machine_memory_register_ports(t_ram *ram, t_port *port);


type_status core_machine_memory_allocate_for(t_ram *ram, STD_SIZE_T bytes);
type_status core_machine_memory_register_mapping(t_ram *ram,
    type_unsigned_32 physical_start,
    type_unsigned_32 backing_start, STD_SIZE_T bytes);
type_status core_machine_memory_register_write_observer(t_ram *ram,
    core_machine_memory_write_observer callback, C_VOID *owner);
C_VOID core_machine_memory_freeze_mappings(t_ram *ram);
type_status core_machine_memory_read_real_from(t_ram *ram, uint16_t segment,
    uint16_t offset, C_VOID *out_data, STD_SIZE_T size);
type_status core_machine_memory_write_real_to(t_ram *ram, uint16_t segment,
    uint16_t offset, const C_VOID *in_data, STD_SIZE_T size);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
