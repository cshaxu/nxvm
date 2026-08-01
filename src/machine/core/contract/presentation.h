#ifndef NXVM_CORE_PRESENTATION_H
#define NXVM_CORE_PRESENTATION_H

#include <stddef.h>
#include <stdint.h>

#include "machine/core/contract/status.h"

#define NXVM_CORE_KEYBOARD_QUEUE_CAPACITY 32u
#define NXVM_CORE_TEXT_COLUMNS 80u
#define NXVM_CORE_TEXT_ROWS 25u
#define NXVM_CORE_TEXT_CELLS (NXVM_CORE_TEXT_COLUMNS * NXVM_CORE_TEXT_ROWS)

typedef struct nxvm_core_keyboard_queue {
    uint16_t values[NXVM_CORE_KEYBOARD_QUEUE_CAPACITY];
    size_t head;
    size_t count;
} nxvm_core_keyboard_queue;

typedef struct nxvm_core_text_snapshot {
    uint8_t characters[NXVM_CORE_TEXT_CELLS];
    uint8_t attributes[NXVM_CORE_TEXT_CELLS];
    uint16_t columns;
    uint16_t rows;
    uint16_t cursor_x;
    uint16_t cursor_y;
    uint64_t generation;
} nxvm_core_text_snapshot;

void nxvm_core_keyboard_queue_initialize(nxvm_core_keyboard_queue *queue);
nxvm_core_status nxvm_core_keyboard_queue_push(
    nxvm_core_keyboard_queue *queue, uint16_t value);
nxvm_core_status nxvm_core_keyboard_queue_pop(
    nxvm_core_keyboard_queue *queue, uint16_t *out_value);
void nxvm_core_text_snapshot_initialize(nxvm_core_text_snapshot *snapshot);
nxvm_core_status nxvm_core_text_snapshot_write(
    nxvm_core_text_snapshot *snapshot, uint16_t x, uint16_t y,
    uint8_t character, uint8_t attribute);
nxvm_core_status nxvm_core_text_snapshot_copy(
    const nxvm_core_text_snapshot *source, nxvm_core_text_snapshot *destination);

#endif
