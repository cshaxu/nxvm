#ifndef CORE_MACHINE_PRESENTATION_INTERFACE_H
#define CORE_MACHINE_PRESENTATION_INTERFACE_H
#include "lib/types/types_interface.h"


#include "type.h"

#define CORE_MACHINE_KEYBOARD_QUEUE_CAPACITY 32u
#define CORE_MACHINE_TEXT_COLUMNS 80u
#define CORE_MACHINE_TEXT_ROWS 25u
#define CORE_MACHINE_TEXT_CELLS (CORE_MACHINE_TEXT_COLUMNS * CORE_MACHINE_TEXT_ROWS)

typedef struct core_machine_keyboard_queue {
    lib_u16 values[CORE_MACHINE_KEYBOARD_QUEUE_CAPACITY];
    lib_size head;
    lib_size count;
} core_machine_keyboard_queue;

typedef struct core_machine_text_snapshot {
    lib_u8 characters[CORE_MACHINE_TEXT_CELLS];
    lib_u8 attributes[CORE_MACHINE_TEXT_CELLS];
    lib_u16 columns;
    lib_u16 rows;
    lib_u16 cursor_x;
    lib_u16 cursor_y;
    lib_u64 generation;
} core_machine_text_snapshot;

C_VOID core_machine_keyboard_queue_initialize(core_machine_keyboard_queue *queue);
type_status core_machine_keyboard_queue_push(
    core_machine_keyboard_queue *queue, lib_u16 value);
type_status core_machine_keyboard_queue_pop(
    core_machine_keyboard_queue *queue, lib_u16 *out_value);
C_VOID core_machine_text_snapshot_initialize(core_machine_text_snapshot *snapshot);
type_status core_machine_text_snapshot_write(
    core_machine_text_snapshot *snapshot, lib_u16 x, lib_u16 y,
    lib_u8 character, lib_u8 attribute);
type_status core_machine_text_snapshot_copy(
    const core_machine_text_snapshot *source, core_machine_text_snapshot *destination);

#endif
