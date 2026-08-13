#ifndef CORE_MACHINE_PRESENTATION_INTERFACE_H
#define CORE_MACHINE_PRESENTATION_INTERFACE_H


#include "type.h"

#define CORE_MACHINE_KEYBOARD_QUEUE_CAPACITY 32u
#define CORE_MACHINE_TEXT_COLUMNS 80u
#define CORE_MACHINE_TEXT_ROWS 25u
#define CORE_MACHINE_TEXT_CELLS (CORE_MACHINE_TEXT_COLUMNS * CORE_MACHINE_TEXT_ROWS)

typedef struct core_machine_keyboard_queue {
    type_unsigned_16 values[CORE_MACHINE_KEYBOARD_QUEUE_CAPACITY];
    STD_SIZE_T head;
    STD_SIZE_T count;
} core_machine_keyboard_queue;

typedef struct core_machine_text_snapshot {
    type_unsigned_8 characters[CORE_MACHINE_TEXT_CELLS];
    type_unsigned_8 attributes[CORE_MACHINE_TEXT_CELLS];
    type_unsigned_16 columns;
    type_unsigned_16 rows;
    type_unsigned_16 cursor_x;
    type_unsigned_16 cursor_y;
    type_unsigned_64 generation;
} core_machine_text_snapshot;

C_VOID core_machine_keyboard_queue_initialize(core_machine_keyboard_queue *queue);
type_status core_machine_keyboard_queue_push(
    core_machine_keyboard_queue *queue, type_unsigned_16 value);
type_status core_machine_keyboard_queue_pop(
    core_machine_keyboard_queue *queue, type_unsigned_16 *out_value);
C_VOID core_machine_text_snapshot_initialize(core_machine_text_snapshot *snapshot);
type_status core_machine_text_snapshot_write(
    core_machine_text_snapshot *snapshot, type_unsigned_16 x, type_unsigned_16 y,
    type_unsigned_8 character, type_unsigned_8 attribute);
type_status core_machine_text_snapshot_copy(
    const core_machine_text_snapshot *source, core_machine_text_snapshot *destination);

#endif
