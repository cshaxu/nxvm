#include "core/machine/presentation_interface.h"

#include <string.h>

void core_machine_keyboard_queue_initialize(core_machine_keyboard_queue *queue)
{
    if (queue != NULL) memset(queue, 0, sizeof(*queue));
}

nxvm_core_status core_machine_keyboard_queue_push(
    core_machine_keyboard_queue *queue, uint16_t value)
{
    size_t index;
    if (queue == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    if (queue->count == CORE_MACHINE_KEYBOARD_QUEUE_CAPACITY) return NXVM_CORE_STATUS_NO_MEMORY;
    index = (queue->head + queue->count) % CORE_MACHINE_KEYBOARD_QUEUE_CAPACITY;
    queue->values[index] = value;
    ++queue->count;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status core_machine_keyboard_queue_pop(
    core_machine_keyboard_queue *queue, uint16_t *out_value)
{
    if (queue == NULL || out_value == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    if (queue->count == 0u) return NXVM_CORE_STATUS_UNSUPPORTED;
    *out_value = queue->values[queue->head];
    queue->head = (queue->head + 1u) % CORE_MACHINE_KEYBOARD_QUEUE_CAPACITY;
    --queue->count;
    return NXVM_CORE_STATUS_OK;
}

void core_machine_text_snapshot_initialize(core_machine_text_snapshot *snapshot)
{
    if (snapshot != NULL) {
        memset(snapshot, 0, sizeof(*snapshot));
        snapshot->columns = CORE_MACHINE_TEXT_COLUMNS;
        snapshot->rows = CORE_MACHINE_TEXT_ROWS;
    }
}

nxvm_core_status core_machine_text_snapshot_write(
    core_machine_text_snapshot *snapshot, uint16_t x, uint16_t y,
    uint8_t character, uint8_t attribute)
{
    size_t index;
    if (snapshot == NULL || x >= snapshot->columns || y >= snapshot->rows) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    index = (size_t)y * snapshot->columns + x;
    snapshot->characters[index] = character;
    snapshot->attributes[index] = attribute;
    ++snapshot->generation;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status core_machine_text_snapshot_copy(
    const core_machine_text_snapshot *source, core_machine_text_snapshot *destination)
{
    if (source == NULL || destination == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    *destination = *source;
    return NXVM_CORE_STATUS_OK;
}
