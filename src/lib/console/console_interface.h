#ifndef LIB_BASE_CONSOLE_INTERFACE_H
#define LIB_BASE_CONSOLE_INTERFACE_H

/* Sink callbacks must not synchronously reenter sink replacement or destroy
 * on the same Console. Detach waits for callbacks; output replacement waits
 * for writes. These gates block instead of busy-waiting. */

#include "lib/types/types_interface.h"

/* Console-only status; the established numeric ABI is unchanged. */
enum { LIB_STATUS_NOT_CURRENT = 6 };

/* A logical Console is a copied-value endpoint.  It deliberately has no
 * native handle, process ownership, reader, or presentation policy.  Host is
 * the only component that binds one logical object to native Console I/O. */

#define LIB_CONSOLE_TEXT_COLUMNS 80u
#define LIB_CONSOLE_TEXT_ROWS 25u

typedef struct lib_console lib_console;

#define LIB_CONSOLE_LINE_MAX 1024u

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

enum {
    LIB_CONSOLE_MODIFIER_CONTROL = 0x01u,
    LIB_CONSOLE_MODIFIER_ALT = 0x02u,
    LIB_CONSOLE_MODIFIER_SHIFT = 0x04u
};

typedef struct lib_console_raw_key {
    /* Opaque platform key value, interpreted by the matching UI adapter.
     * Windows producers use the SDK virtual-key vocabulary wrapped in types. */
    lib_u32 key;
    lib_u32 unicode;
    lib_u16 scan_code;
    lib_u8 modifiers;
    lib_bool extended;
    lib_bool pressed;
    /* Copied native repetitions; zero denotes a single record.
     * The input consumer, not host, interprets characters and expands counts. */
    lib_u16 repeat_count;
} lib_console_raw_key;

typedef struct lib_console_raw_mouse {
    /* Raw Console cell position (historical field names), not UI motion.
     * buttons is the platform input-record mask. The matching UI adapter
     * computes motion and translates buttons before emitting a UI event. */
    lib_i32 delta_x;
    lib_i32 delta_y;
    lib_u32 buttons;
} lib_console_raw_mouse;

typedef struct lib_console_line {
    lib_u32 length;
    char text[LIB_CONSOLE_LINE_MAX];
} lib_console_line;

typedef struct lib_console_event {
    lib_console_event_kind kind;
    lib_u32 binding_generation;
    union {
        lib_console_raw_key raw_key;
        lib_console_raw_mouse raw_mouse;
        lib_console_line line;
    } value;
} lib_console_event;

/* Copied text output for a logical Console. It is deliberately a
 * Console value rather than a UI/window frame: native host renderers consume
 * it without learning an application or Window protocol. Palette values use
 * 0x00RRGGBB. */
typedef struct lib_console_text_frame {
    lib_u16 columns;
    lib_u16 rows;
    lib_u8 text[LIB_CONSOLE_TEXT_COLUMNS * LIB_CONSOLE_TEXT_ROWS];
    lib_u16 attributes[LIB_CONSOLE_TEXT_COLUMNS * LIB_CONSOLE_TEXT_ROWS];
    lib_u32 palette[16u];
    lib_i32 cursor_column;
    lib_i32 cursor_row;
    lib_u8 cursor_top;
    lib_u8 cursor_bottom;
    lib_u8 cursor_visible;
    lib_u8 cursor_phase;
    lib_u32 font_height;
} lib_console_text_frame;

typedef void (*lib_console_event_sink)(void *context,
    const lib_console_event *event);

/* Fixed PC display-byte mapping (CP437 graphics), independent of host code page.
 * Zero is a blank cell; custom raster fonts cannot be reproduced by this map. */
lib_u16 lib_console_pc_glyph(lib_u8 value);

lib_status lib_console_create(lib_console **out_console);
lib_console *lib_console_retain(lib_console *console);
void lib_console_release(lib_console *console);
void lib_console_destroy(lib_console *console);
lib_status lib_console_set_event_sink(lib_console *console,
    lib_console_event_sink sink, void *context);
lib_status lib_console_write_text(lib_console *console,
    const char *text, lib_size length);
lib_status lib_console_write_text_frame(lib_console *console,
    const lib_console_text_frame *frame);

#endif
