#ifndef NTVDM64_CORE_MACHINE_PRESENTATION_INTERFACE_H
#define NTVDM64_CORE_MACHINE_PRESENTATION_INTERFACE_H

#include <stddef.h>
#include <stdint.h>

#include "type.h"

#define CORE_MACHINE_KEYBOARD_QUEUE_CAPACITY 32u
#define CORE_MACHINE_TEXT_COLUMNS 80u
#define CORE_MACHINE_TEXT_ROWS 25u
#define CORE_MACHINE_TEXT_CELLS (CORE_MACHINE_TEXT_COLUMNS * CORE_MACHINE_TEXT_ROWS)

typedef struct core_machine_keyboard_queue {
    uint16_t values[CORE_MACHINE_KEYBOARD_QUEUE_CAPACITY];
    size_t head;
    size_t count;
} core_machine_keyboard_queue;

typedef struct core_machine_text_snapshot {
    uint8_t characters[CORE_MACHINE_TEXT_CELLS];
    uint8_t attributes[CORE_MACHINE_TEXT_CELLS];
    uint16_t columns;
    uint16_t rows;
    uint16_t cursor_x;
    uint16_t cursor_y;
    uint64_t generation;
} core_machine_text_snapshot;

void core_machine_keyboard_queue_initialize(core_machine_keyboard_queue *queue);
ntvdm64_status core_machine_keyboard_queue_push(
    core_machine_keyboard_queue *queue, uint16_t value);
ntvdm64_status core_machine_keyboard_queue_pop(
    core_machine_keyboard_queue *queue, uint16_t *out_value);
void core_machine_text_snapshot_initialize(core_machine_text_snapshot *snapshot);
ntvdm64_status core_machine_text_snapshot_write(
    core_machine_text_snapshot *snapshot, uint16_t x, uint16_t y,
    uint8_t character, uint8_t attribute);
ntvdm64_status core_machine_text_snapshot_copy(
    const core_machine_text_snapshot *source, core_machine_text_snapshot *destination);

#endif
