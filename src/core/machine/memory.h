/* Copyright 2012-2014 Neko. */

#ifndef NXVM_CORE_MEMORY_H
#define NXVM_CORE_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "core/machine/vglobal.h"

typedef struct t_port t_port;

#define NXVM_DEVICE_RAM "Unknown Random-access Memory"

typedef struct {
    t_bool flagA20; /* 0 = disable, 1 = enable */
} t_ram_data;

typedef struct {
    t_vaddrcc pBase; /* memory base address is 20 bit */
    t_nubitcc size; /* memory size in byte */
} t_ram_connect;

typedef struct t_ram {
    t_ram_data data;
    t_ram_connect connect;
} t_ram;

#define VRAM_BIT_A20  0x00100000
#define VRAM_FLAG_A20 0x02


void core_machine_memory_read_physical(t_ram *ram, t_nubit32 physical,
    t_vaddrcc destination, t_nubitcc size);
void core_machine_memory_write_physical(t_ram *ram, t_nubit32 physical,
    t_vaddrcc source, t_nubitcc size);
void core_machine_memory_initialize(t_ram *ram);
void core_machine_memory_reset(t_ram *ram);
void core_machine_memory_finalize(t_ram *ram);
void core_machine_memory_register_ports(t_ram *ram, t_port *port);


void core_machine_memory_allocate_for(t_ram *ram, size_t bytes);
void core_machine_memory_read_real_from(t_ram *ram, uint16_t segment,
    uint16_t offset, void *out_data, size_t size);
void core_machine_memory_write_real_to(t_ram *ram, uint16_t segment,
    uint16_t offset, const void *in_data, size_t size);
void *core_machine_memory_real_address(t_ram *ram, uint16_t segment,
    uint16_t offset);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
