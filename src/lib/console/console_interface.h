#ifndef LIB_CONSOLE_INTERFACE_H
#define LIB_CONSOLE_INTERFACE_H

/* Sink callbacks must not synchronously reenter sink replacement or destroy
 * on the same Console. Detach waits for callbacks; output replacement waits
 * for writes. These gates block instead of busy-waiting. */

#include "lib/base/input_interface.h"

/* A logical Console is a copied-value endpoint.  It deliberately has no
 * native handle, process ownership, reader, or presentation policy. Console
 * Broker alone binds a logical object to native Console I/O. */

typedef struct lib_console lib_console;

#define LIB_CONSOLE_LINE_CAPACITY 1024u

typedef enum lib_console_event_kind {
    LIB_CONSOLE_EVENT_RAW_KEY,
    LIB_CONSOLE_EVENT_RAW_MOUSE,
    LIB_CONSOLE_EVENT_COOKED_LINE,
    LIB_CONSOLE_EVENT_REJECTED_LINE,
    /* Unexpected reader failure; cancellation during replacement is not failure. */
    LIB_CONSOLE_EVENT_IO_FAILURE,
    /* Binding succeeded. Receivers may wake output workers; callbacks must
     * not synchronously reenter the broker or render into the binding. */
    LIB_CONSOLE_EVENT_ACTIVATED,
    /* New binding input begins after this synchronous callback returns.
     * Clear local input state only: no output, broker reentry or destruction.
     * This is not activation success or permanent source retirement. */
    LIB_CONSOLE_EVENT_INPUT_RESET
} lib_console_event_kind;

typedef struct lib_console_raw_mouse {
    /* Absolute zero-based screen-buffer cells, not relative pixel motion.
     * buttons uses BASE_MOUSE_BUTTON_*; no native input-record bits escape. */
    lib_i32 column;
    lib_i32 row;
    lib_u32 buttons;
} lib_console_raw_mouse;

typedef struct lib_console_line {
    /* UTF-8 bytes excluding NUL and CR/LF, at most CAPACITY-1. Invalid or
     * oversized native lines are drained and delivered as REJECTED_LINE. */
    lib_u32 length;
    char text[LIB_CONSOLE_LINE_CAPACITY];
} lib_console_line;

typedef struct lib_console_event {
    lib_console_event_kind kind;
    lib_u32 binding_generation;
    union {
        base_key_input raw_key;
        lib_console_raw_mouse raw_mouse;
        lib_console_line line;
    } value;
} lib_console_event;

/* Synchronously borrowed text output for a logical Console. All arrays remain
 * valid until write returns; sinks must not retain them. It is deliberately a
 * Console value rather than a KVM/window frame: native host renderers consume
 * it without learning an application or Window protocol. Palette values use
 * 0x00RRGGBB. */
typedef struct lib_console_text_frame {
    lib_u32 columns;
    lib_u32 rows;
    lib_u32 stride; /* Array elements per row. */
    lib_size capacity; /* Element capacity of each array. */
    /* Cells equal the preceding successful frame on this logical Console.
     * The caller serializes that comparison and its frame writes. This only
     * permits reuse: binding/geometry changes still require complete output.
     * False is always safe. Palette and cursor are applied independently. */
    lib_bool cells_unchanged;
    lib_u16 *text; /* BMP characters; read-only during write. */
    lib_u8 *foreground; /* palette index 0..15 */
    lib_u8 *background; /* palette index 0..15 */
    lib_u8 *underline; /* native text decoration, not a scanline */
    lib_u32 palette[16u];
    lib_i32 cursor_column;
    lib_i32 cursor_row;
    lib_u8 cursor_top;
    lib_u8 cursor_bottom;
    lib_bool cursor_visible;
    lib_bool cursor_phase;
    lib_u32 font_height;
} lib_console_text_frame;

typedef void (*lib_console_event_sink)(void *context,
    const lib_console_event *event);

lib_status lib_console_create(lib_console **out_console);
lib_console *lib_console_retain(lib_console *console);
/* Reference release is private ownership bookkeeping.  It never creates a
 * caller-visible half-Console; binding and I/O operations carry statuses. */
void lib_console_release(lib_console *console);
lib_status lib_console_set_event_sink(lib_console *console,
    lib_console_event_sink sink, void *context);
/* Each write is a complete UTF-8 sequence, length in bytes; malformed input
 * returns INVALID_ARGUMENT before output. Native I/O failure may have emitted
 * a prefix; no rollback/replay is performed. Empty writes emit nothing. */
lib_status lib_console_write_text(lib_console *console,
    const char *text, lib_size length);
lib_status lib_console_write_text_frame(lib_console *console,
    const lib_console_text_frame *frame);

#endif
