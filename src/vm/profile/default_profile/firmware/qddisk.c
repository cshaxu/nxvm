/* Copyright 2012-2014 Neko. */

/* QDDISK implements quick and dirty hard drive control routines. */

#include "core/product/utils.h"

#include "core/machine/cpu.h"
#include "core/machine/memory.h"
#include "core/machine/block_interface.h"

#include "vm/profile/default_profile/firmware/context.h"
#include "vm/profile/default_profile/firmware/qdx.h"
#include "qddisk.h"

static void set_hdd_status(vm_profile_default_context *profile, t_nubit8 status)
{
    core_machine_memory_write_real_to(profile->ram, 0x0040, 0x0074,
        &status, sizeof(status));
}

static void int_13_02_hdd_read_sector(vm_profile_default_context *profile) {
    t_cpu *cpu = profile->execution->cpu;
    t_nubit8 drive  = cpu->data.dl;
    t_nubit8 head   = cpu->data.dh;
    t_nubit8 cyl    = cpu->data.ch | ((cpu->data.cl & 0xc0) << 8);
    t_nubit8 sector = cpu->data.cl & 0x3f;
    drive &= 0x7f;
    core_machine_block_geometry geometry;
    core_machine_block_get_geometry_from(profile->block_provider, &geometry);
    if (drive || !sector || head >= geometry.heads || sector > geometry.sectors || cyl >= geometry.cylinders || !core_machine_block_read_from(profile->block_provider, cyl, head, sector, core_machine_memory_real_address(profile->ram, cpu->data.es.selector, cpu->data.bx), cpu->data.al * geometry.bytes_per_sector)) {
        /* sector not found */
        cpu->data.ah = 0x04;
        SetBit(cpu->data.eflags, VCPU_EFLAGS_CF);
    } else {
        cpu->data.ah = 0x00;
        ClrBit(cpu->data.eflags, VCPU_EFLAGS_CF);
    }
    set_hdd_status(profile, cpu->data.ah);
}

static void int_13_03_hdd_write_sector(vm_profile_default_context *profile) {
    t_cpu *cpu = profile->execution->cpu;
    t_nubit8 drive  = cpu->data.dl;
    t_nubit8 head   = cpu->data.dh;
    t_nubit8 cyl    = cpu->data.ch | ((cpu->data.cl & 0xc0) << 8);
    t_nubit8 sector = cpu->data.cl & 0x3f;
    drive &= 0x7f;
    core_machine_block_geometry geometry;
    core_machine_block_get_geometry_from(profile->block_provider, &geometry);
    if (drive || !sector || head >= geometry.heads || sector > geometry.sectors || cyl >= geometry.cylinders || !core_machine_block_write_from(profile->block_provider, cyl, head, sector, core_machine_memory_real_address(profile->ram, cpu->data.es.selector, cpu->data.bx), cpu->data.al * geometry.bytes_per_sector)) {
        /* sector not found */
        cpu->data.ah = 0x04;
        SetBit(cpu->data.eflags, VCPU_EFLAGS_CF);
    } else {
        cpu->data.ah = 0x00;
        ClrBit(cpu->data.eflags, VCPU_EFLAGS_CF);
    }
    set_hdd_status(profile, cpu->data.ah);
}

void vm_profile_default_disk_initialize(t_qdx *qdx) {
    if (qdx == NULL) return;
    qdx->table[0xa2] = int_13_02_hdd_read_sector;
    qdx->table[0xa3] = int_13_03_hdd_write_sector;
}
