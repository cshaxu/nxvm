#include <stdio.h>

#include "core/machine/presentation_interface.h"

int main(void)
{
    core_machine_keyboard_queue queue;
    core_machine_text_snapshot source;
    core_machine_text_snapshot copy;
    uint16_t key;
    unsigned index;

    core_machine_keyboard_queue_initialize(&queue);
    if (core_machine_keyboard_queue_push(&queue, 0x1eu) != NXVM_CORE_STATUS_OK ||
        core_machine_keyboard_queue_push(&queue, 0x30u) != NXVM_CORE_STATUS_OK ||
        core_machine_keyboard_queue_pop(&queue, &key) != NXVM_CORE_STATUS_OK || key != 0x1eu ||
        core_machine_keyboard_queue_pop(&queue, &key) != NXVM_CORE_STATUS_OK || key != 0x30u ||
        core_machine_keyboard_queue_pop(&queue, &key) != NXVM_CORE_STATUS_UNSUPPORTED) return 1;
    for (index = 0u; index < CORE_MACHINE_KEYBOARD_QUEUE_CAPACITY; ++index) {
        if (core_machine_keyboard_queue_push(&queue, (uint16_t)index) != NXVM_CORE_STATUS_OK) return 1;
    }
    if (core_machine_keyboard_queue_push(&queue, 1u) != NXVM_CORE_STATUS_NO_MEMORY) return 1;
    core_machine_keyboard_queue_initialize(&queue);
    core_machine_text_snapshot_initialize(&source);
    if (source.columns != 80u || source.rows != 25u ||
        core_machine_text_snapshot_write(&source, 79u, 24u, 'X', 7u) != NXVM_CORE_STATUS_OK ||
        core_machine_text_snapshot_copy(&source, &copy) != NXVM_CORE_STATUS_OK ||
        copy.characters[CORE_MACHINE_TEXT_CELLS - 1u] != 'X' ||
        core_machine_text_snapshot_write(&source, 0u, 0u, 'Y', 7u) != NXVM_CORE_STATUS_OK ||
        copy.characters[0] == 'Y' ||
        core_machine_text_snapshot_write(&source, 80u, 0u, 0u, 0u) != NXVM_CORE_STATUS_INVALID_ARGUMENT) return 1;
    puts("M5:T3:S2:PRESENTATION:OK");
    return 0;
}
