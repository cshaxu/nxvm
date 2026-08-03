/* Copyright 2012-2014 Neko. */

#ifndef NXVM_CORE_MEMORY_H
#define NXVM_CORE_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

typedef struct t_port t_port;

#define NXVM_DEVICE_RAM "Unknown Random-access Memory"

typedef struct {
    ntvdm64_type_bool flagA20; /* 0 = disable, 1 = enable */
} t_ram_data;

typedef struct {
    ntvdm64_type_virtual_address pBase; /* memory base address is 20 bit */
    ntvdm64_type_native_unsigned size; /* memory size in byte */
} t_ram_connect;

typedef struct t_ram {
    t_ram_data data;
    t_ram_connect connect;
} t_ram;

#define VRAM_BIT_A20  0x00100000
#define VRAM_FLAG_A20 0x02


C_VOID core_machine_memory_read_physical(t_ram *ram, ntvdm64_type_unsigned_32 physical,
    ntvdm64_type_virtual_address destination, ntvdm64_type_native_unsigned size);
C_VOID core_machine_memory_write_physical(t_ram *ram, ntvdm64_type_unsigned_32 physical,
    ntvdm64_type_virtual_address source, ntvdm64_type_native_unsigned size);
C_VOID core_machine_memory_initialize(t_ram *ram);
C_VOID core_machine_memory_reset(t_ram *ram);
C_VOID core_machine_memory_finalize(t_ram *ram);
C_VOID core_machine_memory_register_ports(t_ram *ram, t_port *port);


C_VOID core_machine_memory_allocate_for(t_ram *ram, STD_SIZE_T bytes);
C_VOID core_machine_memory_read_real_from(t_ram *ram, uint16_t segment,
    uint16_t offset, C_VOID *out_data, STD_SIZE_T size);
C_VOID core_machine_memory_write_real_to(t_ram *ram, uint16_t segment,
    uint16_t offset, const C_VOID *in_data, STD_SIZE_T size);
C_VOID *core_machine_memory_real_address(t_ram *ram, uint16_t segment,
    uint16_t offset);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
