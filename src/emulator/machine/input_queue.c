#include "emulator/machine/input_queue.h"

lib_status emulator_machine_input_queue_initialize(emulator_machine_input_queue *queue)
{
    if (queue == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *queue = (emulator_machine_input_queue) { 0 };
    return base_sync_mutex_create(&queue->lock);
}

void emulator_machine_input_queue_dispose(emulator_machine_input_queue *queue)
{
    if (queue == NULL) return;
    base_sync_mutex_destroy(queue->lock);
    queue->lock = NULL;
}

lib_bool emulator_machine_input_queue_push(emulator_machine_input_queue *queue,
    const kvm_input_event *event)
{
    lib_size next;
    if (queue == NULL || event == NULL) return LIB_FALSE;
    base_sync_mutex_lock(queue->lock);
    next = (queue->head + 1u) % EMULATOR_MACHINE_INPUT_QUEUE_CAPACITY;
    if (next == queue->tail) {
        base_sync_mutex_unlock(queue->lock);
        return LIB_FALSE;
    }
    queue->entries[queue->head] = *event;
    queue->head = next;
    base_sync_mutex_unlock(queue->lock);
    return LIB_TRUE;
}

lib_bool emulator_machine_input_queue_pop(emulator_machine_input_queue *queue,
    kvm_input_event *event)
{
    if (queue == NULL || event == NULL) return LIB_FALSE;
    base_sync_mutex_lock(queue->lock);
    if (queue->tail == queue->head) {
        base_sync_mutex_unlock(queue->lock);
        return LIB_FALSE;
    }
    *event = queue->entries[queue->tail];
    queue->tail = (queue->tail + 1u) % EMULATOR_MACHINE_INPUT_QUEUE_CAPACITY;
    base_sync_mutex_unlock(queue->lock);
    return LIB_TRUE;
}

lib_bool emulator_machine_input_queue_pending(emulator_machine_input_queue *queue)
{
    lib_bool pending;
    if (queue == NULL) return LIB_FALSE;
    base_sync_mutex_lock(queue->lock);
    pending = queue->tail != queue->head;
    base_sync_mutex_unlock(queue->lock);
    return pending;
}

void emulator_machine_input_queue_clear(emulator_machine_input_queue *queue)
{
    if (queue == NULL) return;
    base_sync_mutex_lock(queue->lock);
    queue->head = 0u;
    queue->tail = 0u;
    base_sync_mutex_unlock(queue->lock);
}
