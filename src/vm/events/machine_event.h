#ifndef VM_EVENTS_MACHINE_EVENT_H
#define VM_EVENTS_MACHINE_EVENT_H

#include "type.h"

/* Value-only records exchanged across VM owners.  They retain neither an
 * executor, a session nor a Core object; each receiver owns its own queue and
 * decides how to consume the copied fact. */
typedef enum vm_machine_input_kind {
    VM_MACHINE_INPUT_KEY_EVENT,
    VM_MACHINE_INPUT_MOUSE_EVENT
} vm_machine_input_kind;

typedef struct vm_machine_key_event {
    type_unsigned_16 scan_code;
    type_unsigned_16 virtual_key;
    C_INT pressed;
} vm_machine_key_event;

typedef struct vm_machine_mouse_event {
    type_signed_16 delta_x;
    type_signed_16 delta_y;
    type_unsigned_8 buttons;
} vm_machine_mouse_event;

typedef struct vm_machine_input {
    vm_machine_input_kind kind;
    union {
        vm_machine_key_event key_event;
        vm_machine_mouse_event mouse_event;
    } data;
} vm_machine_input;

/* Every mutation requested of the Core executor is copied into its one FIFO.
 * Product control chooses which request to send; only vm/machine consumes it. */
typedef enum vm_machine_request_kind {
    VM_MACHINE_REQUEST_INPUT,
    VM_MACHINE_REQUEST_PAUSE,
    VM_MACHINE_REQUEST_RESET,
    VM_MACHINE_REQUEST_RESUME,
    VM_MACHINE_REQUEST_STEP,
    VM_MACHINE_REQUEST_STOP
} vm_machine_request_kind;

typedef struct vm_machine_request {
    vm_machine_request_kind kind;
    /* Product-neutral machine pause classification; its numeric meaning is
     * interpreted only by vm/machine's executor. */
    type_unsigned_8 pause_reason;
    vm_machine_input input;
} vm_machine_request;

typedef enum vm_machine_result_kind {
    VM_MACHINE_RESULT_RUNNING,
    VM_MACHINE_RESULT_PAUSED,
    VM_MACHINE_RESULT_RESET,
    VM_MACHINE_RESULT_STOPPED,
    VM_MACHINE_RESULT_DEBUG_COMPLETED,
    VM_MACHINE_RESULT_FAULT,
    VM_MACHINE_RESULT_DISPLAY
} vm_machine_result_kind;

#define VM_MACHINE_EVENT_TEXT_COLUMNS 80u
#define VM_MACHINE_EVENT_TEXT_ROWS 25u
#define VM_MACHINE_EVENT_TEXT_CELLS \
    (VM_MACHINE_EVENT_TEXT_COLUMNS * VM_MACHINE_EVENT_TEXT_ROWS)
#define VM_MACHINE_EVENT_MAX_PIXELS (640u * 350u)
#define VM_MACHINE_EVENT_PALETTE_ENTRIES 256u
#define VM_MACHINE_EVENT_GLYPH_BYTES (256u * 16u)

typedef struct vm_machine_display_event {
    C_INT graphics;
    type_unsigned_8 characters[VM_MACHINE_EVENT_TEXT_CELLS];
    type_unsigned_8 attributes[VM_MACHINE_EVENT_TEXT_CELLS];
    type_unsigned_16 columns;
    type_unsigned_16 rows;
    type_unsigned_8 cursor_top;
    type_unsigned_8 cursor_bottom;
    type_unsigned_8 cursor_x;
    type_unsigned_8 cursor_y;
    C_INT cursor_visible;
    C_INT buffer_changed;
    C_INT cursor_changed;
    type_bool glyphs_present;
    type_unsigned_8 glyphs[VM_MACHINE_EVENT_GLYPH_BYTES];
    type_unsigned_16 pixel_width;
    type_unsigned_16 pixel_height;
    type_unsigned_8 pixels[VM_MACHINE_EVENT_MAX_PIXELS];
    type_unsigned_32 palette_rgb[VM_MACHINE_EVENT_PALETTE_ENTRIES];
    type_unsigned_64 generation;
} vm_machine_display_event;

typedef struct vm_machine_result {
    vm_machine_result_kind kind;
    type_status status;
    union {
        vm_machine_display_event display;
    } value;
} vm_machine_result;

typedef C_VOID (*vm_machine_result_sink)(C_VOID *context,
    const vm_machine_result *result);

#endif
