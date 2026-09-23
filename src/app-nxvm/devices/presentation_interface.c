#include "lib/types/types_interface.h"
#include "type.h"

#include "app-nxvm/devices/presentation_interface.h"



C_VOID core_machine_keyboard_queue_initialize(core_machine_keyboard_queue *queue)
{
    if (queue != LIB_NULL) lib_memory_set(queue, 0, sizeof(*queue));
}

type_status core_machine_keyboard_queue_push(
    core_machine_keyboard_queue *queue, lib_u16 value)
{
    lib_size index;
    if (queue == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (queue->count == CORE_MACHINE_KEYBOARD_QUEUE_CAPACITY) return TYPE_STATUS_NO_MEMORY;
    index = (queue->head + queue->count) % CORE_MACHINE_KEYBOARD_QUEUE_CAPACITY;
    queue->values[index] = value;
    ++queue->count;
    return TYPE_STATUS_OK;
}

type_status core_machine_keyboard_queue_pop(
    core_machine_keyboard_queue *queue, lib_u16 *out_value)
{
    if (queue == LIB_NULL || out_value == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (queue->count == 0u) return TYPE_STATUS_UNSUPPORTED;
    *out_value = queue->values[queue->head];
    queue->head = (queue->head + 1u) % CORE_MACHINE_KEYBOARD_QUEUE_CAPACITY;
    --queue->count;
    return TYPE_STATUS_OK;
}

C_VOID core_machine_text_snapshot_initialize(core_machine_text_snapshot *snapshot)
{
    if (snapshot != LIB_NULL) {
        lib_memory_set(snapshot, 0, sizeof(*snapshot));
        snapshot->columns = CORE_MACHINE_TEXT_COLUMNS;
        snapshot->rows = CORE_MACHINE_TEXT_ROWS;
    }
}

type_status core_machine_text_snapshot_write(
    core_machine_text_snapshot *snapshot, lib_u16 x, lib_u16 y,
    lib_u8 character, lib_u8 attribute)
{
    lib_size index;
    if (snapshot == LIB_NULL || x >= snapshot->columns || y >= snapshot->rows) return TYPE_STATUS_INVALID_ARGUMENT;
    index = (lib_size)y * snapshot->columns + x;
    snapshot->characters[index] = character;
    snapshot->attributes[index] = attribute;
    ++snapshot->generation;
    return TYPE_STATUS_OK;
}

type_status core_machine_text_snapshot_copy(
    const core_machine_text_snapshot *source, core_machine_text_snapshot *destination)
{
    if (source == LIB_NULL || destination == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *destination = *source;
    return TYPE_STATUS_OK;
}
