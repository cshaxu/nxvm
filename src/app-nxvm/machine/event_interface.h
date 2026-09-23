#ifndef VM_MACHINE_EVENT_INTERFACE_H
#define VM_MACHINE_EVENT_INTERFACE_H
#include "lib/types/types_interface.h"

#include "type.h"

#include "common/machine/machine_interface.h"

/* Value-only records exchanged across VM owners.  They retain neither an
 * executor, a session nor a Core object; each receiver owns its own queue and
 * decides how to consume the copied fact. */
typedef enum vm_machine_input_kind {
    VM_MACHINE_INPUT_KEY_EVENT,
    VM_MACHINE_INPUT_MOUSE_EVENT
} vm_machine_input_kind;

typedef struct vm_machine_key_event {
    lib_u16 scan_code;
    lib_u16 virtual_key;
    C_INT pressed;
} vm_machine_key_event;

typedef struct vm_machine_mouse_event {
    lib_i16 delta_x;
    lib_i16 delta_y;
    lib_u8 buttons;
} vm_machine_mouse_event;

typedef struct vm_machine_input {
    vm_machine_input_kind kind;
    union {
        vm_machine_key_event key_event;
        vm_machine_mouse_event mouse_event;
    } data;
} vm_machine_input;

#define VM_MACHINE_EVENT_TEXT_COLUMNS 80u
#define VM_MACHINE_EVENT_TEXT_ROWS 25u
#define VM_MACHINE_EVENT_TEXT_CELLS \
    (VM_MACHINE_EVENT_TEXT_COLUMNS * VM_MACHINE_EVENT_TEXT_ROWS)
#define VM_MACHINE_EVENT_MAX_PIXELS (640u * 350u)
#define VM_MACHINE_EVENT_PALETTE_ENTRIES 256u
#define VM_MACHINE_EVENT_GLYPH_BYTES (256u * 16u)

typedef struct vm_machine_display_event {
    C_INT graphics;
    lib_u8 characters[VM_MACHINE_EVENT_TEXT_CELLS];
    lib_u8 attributes[VM_MACHINE_EVENT_TEXT_CELLS];
    lib_u16 columns;
    lib_u16 rows;
    lib_u8 text_cell_height;
    lib_u8 cursor_top;
    lib_u8 cursor_bottom;
    lib_u8 cursor_x;
    lib_u8 cursor_y;
    C_INT cursor_visible;
    C_INT buffer_changed;
    C_INT cursor_changed;
    type_bool glyphs_present;
    lib_u8 glyphs[VM_MACHINE_EVENT_GLYPH_BYTES];
    lib_u16 pixel_width;
    lib_u16 pixel_height;
    lib_u8 pixels[VM_MACHINE_EVENT_MAX_PIXELS];
    lib_u32 palette_rgb[VM_MACHINE_EVENT_PALETTE_ENTRIES];
    lib_u64 generation;
} vm_machine_display_event;

/* Removable-media presentation is a VM product fact.  Common's public
 * media call intentionally carries only a path, so no Common-private media
 * request record crosses this boundary. */
#endif
