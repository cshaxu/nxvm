#ifndef NTVDM64_VM_PLATFORM_DISPLAY_FRAME_H
#define NTVDM64_VM_PLATFORM_DISPLAY_FRAME_H

#include <stdint.h>

#define VM_PLATFORM_DISPLAY_MAX_COLUMNS 80u
#define VM_PLATFORM_DISPLAY_MAX_ROWS 25u
#define VM_PLATFORM_DISPLAY_MAX_CELLS \
    (VM_PLATFORM_DISPLAY_MAX_COLUMNS * VM_PLATFORM_DISPLAY_MAX_ROWS)

typedef struct vm_platform_display_frame {
    uint8_t characters[VM_PLATFORM_DISPLAY_MAX_CELLS];
    uint8_t attributes[VM_PLATFORM_DISPLAY_MAX_CELLS];
    uint16_t columns;
    uint16_t rows;
    uint8_t cursor_top;
    uint8_t cursor_bottom;
    uint8_t cursor_x;
    uint8_t cursor_y;
    int cursor_visible;
    int buffer_changed;
    int cursor_changed;
    uint64_t generation;
} vm_platform_display_frame;

void vm_platform_display_initialize(void);
void vm_platform_display_publish(const vm_platform_display_frame *frame);
void vm_platform_display_capture(vm_platform_display_frame *out_frame);

#endif
