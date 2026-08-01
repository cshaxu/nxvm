#include "machine/core/contract/presentation.h"

#include <string.h>

void nxvm_core_keyboard_queue_initialize(nxvm_core_keyboard_queue *queue)
{
    if (queue != NULL) memset(queue, 0, sizeof(*queue));
}

nxvm_core_status nxvm_core_keyboard_queue_push(
    nxvm_core_keyboard_queue *queue, uint16_t value)
{
    size_t index;
    if (queue == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    if (queue->count == NXVM_CORE_KEYBOARD_QUEUE_CAPACITY) return NXVM_CORE_STATUS_NO_MEMORY;
    index = (queue->head + queue->count) % NXVM_CORE_KEYBOARD_QUEUE_CAPACITY;
    queue->values[index] = value;
    ++queue->count;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_core_keyboard_queue_pop(
    nxvm_core_keyboard_queue *queue, uint16_t *out_value)
{
    if (queue == NULL || out_value == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    if (queue->count == 0u) return NXVM_CORE_STATUS_UNSUPPORTED;
    *out_value = queue->values[queue->head];
    queue->head = (queue->head + 1u) % NXVM_CORE_KEYBOARD_QUEUE_CAPACITY;
    --queue->count;
    return NXVM_CORE_STATUS_OK;
}

void nxvm_core_text_snapshot_initialize(nxvm_core_text_snapshot *snapshot)
{
    if (snapshot != NULL) {
        memset(snapshot, 0, sizeof(*snapshot));
        snapshot->columns = NXVM_CORE_TEXT_COLUMNS;
        snapshot->rows = NXVM_CORE_TEXT_ROWS;
    }
}

nxvm_core_status nxvm_core_text_snapshot_write(
    nxvm_core_text_snapshot *snapshot, uint16_t x, uint16_t y,
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

nxvm_core_status nxvm_core_text_snapshot_copy(
    const nxvm_core_text_snapshot *source, nxvm_core_text_snapshot *destination)
{
    if (source == NULL || destination == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    *destination = *source;
    return NXVM_CORE_STATUS_OK;
}
