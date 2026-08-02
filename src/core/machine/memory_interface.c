#include "core/machine/machine.h"

#include <stdlib.h>
#include <string.h>

static int core_machine_memory_translate(
    const core_machine_memory *memory,
    uint32_t physical,
    size_t size,
    size_t *out_offset)
{
    uint64_t offset = physical;

    if (!memory->a20_enabled) {
        offset &= ~(UINT64_C(1) << 20);
    }

    if (size > memory->size || offset > memory->size - size) {
        return 0;
    }

    *out_offset = (size_t)offset;
    return 1;
}

nxvm_core_status core_machine_memory_initialize(core_machine *machine)
{
    size_t size;

    if (machine == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    size = machine->config.memory_bytes;
    if (size == 0u) {
        size = CORE_MACHINE_DEFAULT_MEMORY_BYTES;
    }

    if (size < CORE_MACHINE_MINIMUM_MEMORY_BYTES ||
        size > CORE_MACHINE_MAXIMUM_MEMORY_BYTES) {
        return NXVM_CORE_STATUS_UNSUPPORTED;
    }

    machine->memory.bytes = (uint8_t *)calloc(size, sizeof(uint8_t));
    if (machine->memory.bytes == NULL) {
        return NXVM_CORE_STATUS_NO_MEMORY;
    }

    machine->memory.size = size;
    machine->memory.a20_enabled = 0;
    return NXVM_CORE_STATUS_OK;
}

void core_machine_memory_finalize(core_machine *machine)
{
    if (machine != NULL) {
        free(machine->memory.bytes);
        machine->memory.bytes = NULL;
        machine->memory.size = 0u;
        machine->memory.a20_enabled = 0;
    }
}

nxvm_core_status core_machine_memory_reset(core_machine *machine)
{
    if (machine == NULL || machine->memory.bytes == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    memset(machine->memory.bytes, 0, machine->memory.size);
    machine->memory.a20_enabled = 0;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status core_machine_memory_read(
    const core_machine *machine,
    uint32_t physical,
    void *out_data,
    size_t size)
{
    size_t offset;

    if (machine == NULL || out_data == NULL || size == 0u) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    if (!core_machine_memory_translate(&machine->memory, physical, size, &offset)) {
        return NXVM_CORE_STATUS_FAULT;
    }

    memcpy(out_data, machine->memory.bytes + offset, size);
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status core_machine_memory_write(
    core_machine *machine,
    uint32_t physical,
    const void *data,
    size_t size)
{
    size_t offset;

    if (machine == NULL || data == NULL || size == 0u) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    if (!core_machine_memory_translate(&machine->memory, physical, size, &offset)) {
        return NXVM_CORE_STATUS_FAULT;
    }

    memcpy(machine->memory.bytes + offset, data, size);
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status core_machine_set_a20(
    core_machine *machine,
    int enabled)
{
    if (machine == NULL) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }

    machine->memory.a20_enabled = enabled != 0;
    return NXVM_CORE_STATUS_OK;
}
