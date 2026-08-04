/* Copyright 2012-2014 Neko. */

/* QDDISK implements quick and dirty hard drive control routines. */

#include "type.h"

#include "core/product/utils.h"


#include "core/machine/cpu.h"

#include "core/machine/memory.h"

#include "core/machine/block_interface.h"


#include "vm/profile/default_profile/firmware/context.h"

#include "vm/profile/default_profile/firmware/qdx.h"

#include "qddisk.h"

static C_VOID set_hdd_status(vm_profile_default_context *profile, type_unsigned_8 status)
{
    core_machine_memory_write_real_to(vm_profile_default_context_memory(profile), 0x0040, 0x0074,
        &status, sizeof(status));
}

static C_VOID int_13_02_hdd_read_sector(vm_profile_default_context *profile) {
    t_cpu *cpu = vm_profile_default_context_execution(profile)->cpu;
    type_unsigned_8 drive  = cpu->data.dl;
    type_unsigned_8 head   = cpu->data.dh;
    type_unsigned_8 cyl    = cpu->data.ch | ((cpu->data.cl & 0xc0) << 8);
    type_unsigned_8 sector = cpu->data.cl & 0x3f;
    STD_SIZE_T transfer_bytes;
    C_VOID *buffer;
    C_INT failed;
    drive &= 0x7f;
    core_machine_block_geometry geometry;
    core_machine_block_get_geometry_from(profile->block_provider, &geometry);
    transfer_bytes = (STD_SIZE_T)cpu->data.al * geometry.bytes_per_sector;
    buffer = transfer_bytes == 0u ? STD_NULL : STD_MALLOC(transfer_bytes);
    failed = drive || !sector || head >= geometry.heads || sector > geometry.sectors ||
        cyl >= geometry.cylinders || buffer == STD_NULL ||
        !core_machine_block_read_from(profile->block_provider, cyl, head, sector,
            buffer, transfer_bytes) ||
        core_machine_memory_write_real_to(vm_profile_default_context_memory(profile), cpu->data.es.selector,
            cpu->data.bx, buffer, transfer_bytes) != TYPE_STATUS_OK;
    STD_FREE(buffer);
    if (failed) {
        /* sector not found */
        cpu->data.ah = 0x04;
        TYPE_SET_BIT(cpu->data.eflags, VCPU_EFLAGS_CF);
    } else {
        cpu->data.ah = 0x00;
        TYPE_CLEAR_BIT(cpu->data.eflags, VCPU_EFLAGS_CF);
    }
    set_hdd_status(profile, cpu->data.ah);
}

static C_VOID int_13_03_hdd_write_sector(vm_profile_default_context *profile) {
    t_cpu *cpu = vm_profile_default_context_execution(profile)->cpu;
    type_unsigned_8 drive  = cpu->data.dl;
    type_unsigned_8 head   = cpu->data.dh;
    type_unsigned_8 cyl    = cpu->data.ch | ((cpu->data.cl & 0xc0) << 8);
    type_unsigned_8 sector = cpu->data.cl & 0x3f;
    STD_SIZE_T transfer_bytes;
    C_VOID *buffer;
    C_INT failed;
    drive &= 0x7f;
    core_machine_block_geometry geometry;
    core_machine_block_get_geometry_from(profile->block_provider, &geometry);
    transfer_bytes = (STD_SIZE_T)cpu->data.al * geometry.bytes_per_sector;
    buffer = transfer_bytes == 0u ? STD_NULL : STD_MALLOC(transfer_bytes);
    failed = drive || !sector || head >= geometry.heads || sector > geometry.sectors ||
        cyl >= geometry.cylinders || buffer == STD_NULL ||
        core_machine_memory_read_real_from(vm_profile_default_context_memory(profile), cpu->data.es.selector,
            cpu->data.bx, buffer, transfer_bytes) != TYPE_STATUS_OK ||
        !core_machine_block_write_from(profile->block_provider, cyl, head, sector,
            buffer, transfer_bytes);
    STD_FREE(buffer);
    if (failed) {
        /* sector not found */
        cpu->data.ah = 0x04;
        TYPE_SET_BIT(cpu->data.eflags, VCPU_EFLAGS_CF);
    } else {
        cpu->data.ah = 0x00;
        TYPE_CLEAR_BIT(cpu->data.eflags, VCPU_EFLAGS_CF);
    }
    set_hdd_status(profile, cpu->data.ah);
}

C_VOID vm_profile_default_disk_initialize(t_qdx *qdx) {
    if (qdx == STD_NULL) return;
    qdx->table[0xa2] = int_13_02_hdd_read_sector;
    qdx->table[0xa3] = int_13_03_hdd_write_sector;
}
