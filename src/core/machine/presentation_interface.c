#include "type.h"

#include "core/machine/presentation_interface.h"



C_VOID core_machine_keyboard_queue_initialize(core_machine_keyboard_queue *queue)
{
    if (queue != STD_NULL) STD_MEMSET(queue, 0, sizeof(*queue));
}

type_status core_machine_keyboard_queue_push(
    core_machine_keyboard_queue *queue, type_unsigned_16 value)
{
    STD_SIZE_T index;
    if (queue == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (queue->count == CORE_MACHINE_KEYBOARD_QUEUE_CAPACITY) return TYPE_STATUS_NO_MEMORY;
    index = (queue->head + queue->count) % CORE_MACHINE_KEYBOARD_QUEUE_CAPACITY;
    queue->values[index] = value;
    ++queue->count;
    return TYPE_STATUS_OK;
}

type_status core_machine_keyboard_queue_pop(
    core_machine_keyboard_queue *queue, type_unsigned_16 *out_value)
{
    if (queue == STD_NULL || out_value == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (queue->count == 0u) return TYPE_STATUS_UNSUPPORTED;
    *out_value = queue->values[queue->head];
    queue->head = (queue->head + 1u) % CORE_MACHINE_KEYBOARD_QUEUE_CAPACITY;
    --queue->count;
    return TYPE_STATUS_OK;
}

C_VOID core_machine_text_snapshot_initialize(core_machine_text_snapshot *snapshot)
{
    if (snapshot != STD_NULL) {
        STD_MEMSET(snapshot, 0, sizeof(*snapshot));
        snapshot->columns = CORE_MACHINE_TEXT_COLUMNS;
        snapshot->rows = CORE_MACHINE_TEXT_ROWS;
    }
}

type_status core_machine_text_snapshot_write(
    core_machine_text_snapshot *snapshot, type_unsigned_16 x, type_unsigned_16 y,
    type_unsigned_8 character, type_unsigned_8 attribute)
{
    STD_SIZE_T index;
    if (snapshot == STD_NULL || x >= snapshot->columns || y >= snapshot->rows) return TYPE_STATUS_INVALID_ARGUMENT;
    index = (STD_SIZE_T)y * snapshot->columns + x;
    snapshot->characters[index] = character;
    snapshot->attributes[index] = attribute;
    ++snapshot->generation;
    return TYPE_STATUS_OK;
}

type_status core_machine_text_snapshot_copy(
    const core_machine_text_snapshot *source, core_machine_text_snapshot *destination)
{
    if (source == STD_NULL || destination == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *destination = *source;
    return TYPE_STATUS_OK;
}
