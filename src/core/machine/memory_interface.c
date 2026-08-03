#include "type.h"

#include "core/machine/machine.h"




static C_INT core_machine_memory_translate(
    const core_machine_memory *memory,
    uint32_t physical,
    STD_SIZE_T size,
    STD_SIZE_T *out_offset)
{
    uint64_t offset = physical;

    if (!memory->a20_enabled) {
        offset &= ~(UINT64_C(1) << 20);
    }

    if (size > memory->size || offset > memory->size - size) {
        return 0;
    }

    *out_offset = (STD_SIZE_T)offset;
    return 1;
}

ntvdm64_status core_machine_instance_memory_initialize(core_machine *machine)
{
    STD_SIZE_T size;

    if (machine == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    size = machine->config.memory_bytes;
    if (size == 0u) {
        size = CORE_MACHINE_DEFAULT_MEMORY_BYTES;
    }

    if (size < CORE_MACHINE_MINIMUM_MEMORY_BYTES ||
        size > CORE_MACHINE_MAXIMUM_MEMORY_BYTES) {
        return NTVDM64_STATUS_UNSUPPORTED;
    }

    machine->memory.bytes = (uint8_t *)STD_CALLOC(size, sizeof(uint8_t));
    if (machine->memory.bytes == STD_NULL) {
        return NTVDM64_STATUS_NO_MEMORY;
    }

    machine->memory.size = size;
    machine->memory.a20_enabled = 0;
    return NTVDM64_STATUS_OK;
}

C_VOID core_machine_instance_memory_finalize(core_machine *machine)
{
    if (machine != STD_NULL) {
        STD_FREE(machine->memory.bytes);
        machine->memory.bytes = STD_NULL;
        machine->memory.size = 0u;
        machine->memory.a20_enabled = 0;
    }
}

ntvdm64_status core_machine_instance_memory_reset(core_machine *machine)
{
    if (machine == STD_NULL || machine->memory.bytes == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    STD_MEMSET(machine->memory.bytes, 0, machine->memory.size);
    machine->memory.a20_enabled = 0;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_machine_memory_read(
    const core_machine *machine,
    uint32_t physical,
    C_VOID *out_data,
    STD_SIZE_T size)
{
    STD_SIZE_T offset;

    if (machine == STD_NULL || out_data == STD_NULL || size == 0u) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    if (!core_machine_memory_translate(&machine->memory, physical, size, &offset)) {
        return NTVDM64_STATUS_FAULT;
    }

    STD_MEMCPY(out_data, machine->memory.bytes + offset, size);
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_machine_memory_write(
    core_machine *machine,
    uint32_t physical,
    const C_VOID *data,
    STD_SIZE_T size)
{
    STD_SIZE_T offset;

    if (machine == STD_NULL || data == STD_NULL || size == 0u) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    if (!core_machine_memory_translate(&machine->memory, physical, size, &offset)) {
        return NTVDM64_STATUS_FAULT;
    }

    STD_MEMCPY(machine->memory.bytes + offset, data, size);
    return NTVDM64_STATUS_OK;
}

ntvdm64_status core_machine_set_a20(
    core_machine *machine,
    C_INT enabled)
{
    if (machine == STD_NULL) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }

    machine->memory.a20_enabled = enabled != 0;
    return NTVDM64_STATUS_OK;
}
