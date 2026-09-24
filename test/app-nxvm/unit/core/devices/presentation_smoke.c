#include "lib/types/types_interface.h"
#include <stdio.h>



#include "app-nxvm/devices/presentation_interface.h"

lib_i32 main(void)
{
    core_machine_keyboard_queue queue;
    core_machine_text_snapshot source;
    core_machine_text_snapshot copy;
    lib_u16 key;
    lib_u32 index;

    core_machine_keyboard_queue_initialize(&queue);
    if (core_machine_keyboard_queue_push(&queue, 0x1eu) != LIB_STATUS_OK ||
        core_machine_keyboard_queue_push(&queue, 0x30u) != LIB_STATUS_OK ||
        core_machine_keyboard_queue_pop(&queue, &key) != LIB_STATUS_OK || key != 0x1eu ||
        core_machine_keyboard_queue_pop(&queue, &key) != LIB_STATUS_OK || key != 0x30u ||
        core_machine_keyboard_queue_pop(&queue, &key) != LIB_STATUS_UNSUPPORTED) return 1;
    for (index = 0u; index < CORE_MACHINE_KEYBOARD_QUEUE_CAPACITY; ++index) {
        if (core_machine_keyboard_queue_push(&queue, (lib_u16)index) != LIB_STATUS_OK) return 1;
    }
    if (core_machine_keyboard_queue_push(&queue, 1u) != LIB_STATUS_NO_MEMORY) return 1;
    core_machine_keyboard_queue_initialize(&queue);
    core_machine_text_snapshot_initialize(&source);
    if (source.columns != 80u || source.rows != 25u ||
        core_machine_text_snapshot_write(&source, 79u, 24u, 'X', 7u) != LIB_STATUS_OK ||
        core_machine_text_snapshot_copy(&source, &copy) != LIB_STATUS_OK ||
        copy.characters[CORE_MACHINE_TEXT_CELLS - 1u] != 'X' ||
        core_machine_text_snapshot_write(&source, 0u, 0u, 'Y', 7u) != LIB_STATUS_OK ||
        copy.characters[0] == 'Y' ||
        core_machine_text_snapshot_write(&source, 80u, 0u, 0u, 0u) != LIB_STATUS_INVALID_ARGUMENT) return 1;
    puts("M5:T3:S2:PRESENTATION:OK");
    return 0;
}
