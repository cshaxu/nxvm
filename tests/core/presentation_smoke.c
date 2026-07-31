#include <stdio.h>

#include "core/presentation.h"

int main(void)
{
    nxvm_core_keyboard_queue queue;
    nxvm_core_text_snapshot source;
    nxvm_core_text_snapshot copy;
    uint16_t key;
    unsigned index;

    nxvm_core_keyboard_queue_initialize(&queue);
    if (nxvm_core_keyboard_queue_push(&queue, 0x1eu) != NXVM_CORE_STATUS_OK ||
        nxvm_core_keyboard_queue_push(&queue, 0x30u) != NXVM_CORE_STATUS_OK ||
        nxvm_core_keyboard_queue_pop(&queue, &key) != NXVM_CORE_STATUS_OK || key != 0x1eu ||
        nxvm_core_keyboard_queue_pop(&queue, &key) != NXVM_CORE_STATUS_OK || key != 0x30u ||
        nxvm_core_keyboard_queue_pop(&queue, &key) != NXVM_CORE_STATUS_UNSUPPORTED) return 1;
    for (index = 0u; index < NXVM_CORE_KEYBOARD_QUEUE_CAPACITY; ++index) {
        if (nxvm_core_keyboard_queue_push(&queue, (uint16_t)index) != NXVM_CORE_STATUS_OK) return 1;
    }
    if (nxvm_core_keyboard_queue_push(&queue, 1u) != NXVM_CORE_STATUS_NO_MEMORY) return 1;
    nxvm_core_keyboard_queue_initialize(&queue);
    nxvm_core_text_snapshot_initialize(&source);
    if (source.columns != 80u || source.rows != 25u ||
        nxvm_core_text_snapshot_write(&source, 79u, 24u, 'X', 7u) != NXVM_CORE_STATUS_OK ||
        nxvm_core_text_snapshot_copy(&source, &copy) != NXVM_CORE_STATUS_OK ||
        copy.characters[NXVM_CORE_TEXT_CELLS - 1u] != 'X' ||
        nxvm_core_text_snapshot_write(&source, 0u, 0u, 'Y', 7u) != NXVM_CORE_STATUS_OK ||
        copy.characters[0] == 'Y' ||
        nxvm_core_text_snapshot_write(&source, 80u, 0u, 0u, 0u) != NXVM_CORE_STATUS_INVALID_ARGUMENT) return 1;
    puts("M5:T3:S2:PRESENTATION:OK");
    return 0;
}
