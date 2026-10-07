#ifndef TEST_CPU_STACK_PROBE_FIXTURE_H
#define TEST_CPU_STACK_PROBE_FIXTURE_H

#include "cpu_instruction_fixture.h"

typedef struct cpu_stack_probe_fixture {
    cpu_instruction_fixture instruction;
    lib_u32 frame_accesses;
    lib_u32 frame_base;
    lib_u32 frame_size;
    lib_u32 reject_at;
} cpu_stack_probe_fixture;

static lib_status cpu_stack_probe_read(void *opaque, lib_u32 address,
    void *destination, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch)
{
    cpu_stack_probe_fixture *fixture = opaque;
    if (!observe_only && address >= fixture->frame_base &&
        address - fixture->frame_base < fixture->frame_size) {
        ++fixture->frame_accesses;
        if (fixture->frame_accesses == fixture->reject_at) return LIB_STATUS_IO_ERROR;
    }
    return cpu_instruction_read(&fixture->instruction, address, destination,
        bytes, provenance, observe_only, reset_fetch);
}

static lib_status cpu_stack_probe_write(void *opaque, lib_u32 address,
    const void *source, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance)
{
    cpu_stack_probe_fixture *fixture = opaque;
    if (address >= fixture->frame_base &&
        address - fixture->frame_base < fixture->frame_size) {
        ++fixture->frame_accesses;
        if (fixture->frame_accesses == fixture->reject_at) return LIB_STATUS_IO_ERROR;
    }
    return cpu_instruction_write(&fixture->instruction, address, source,
        bytes, provenance);
}

#endif
