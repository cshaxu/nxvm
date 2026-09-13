#ifndef UI_INPUT_INTERFACE_H
#define UI_INPUT_INTERFACE_H

/* Leaf-support contract for copied platform records, not an application ABI.
 * Raw key values are decoded by the selected component-local implementation.
 * Callers own one zero-initialized normalizer per input source. */

#include "lib/ui-base/event_interface.h"
#include "lib/ui-base/hotkey_interface.h"

/* Copied adapter flags, never a raw platform control-state word. */
enum {
    UI_INPUT_MODIFIER_CONTROL = 0x01u,
    UI_INPUT_MODIFIER_ALT = 0x02u,
    UI_INPUT_MODIFIER_SHIFT = 0x04u,
    UI_INPUT_FLAG_EXTENDED = 0x01u
};

/* Common normalization owns state and delivery. Platform decoding owns layout
 * and scan recovery. A platform without a physical text mapping emits TEXT. */

typedef struct ui_keyboard_normalizer {
    lib_u16 pending_high_surrogate;
    lib_u16 pending_repeat_count;
} ui_keyboard_normalizer;

/* These functions only normalize host packets.  A project binding maps each
   physical record to its own input protocol and owns its input queue. */
int ui_keyboard_submit_utf16(
    ui_keyboard_normalizer *state, const ui_hotkey_matcher *held_keys, void *context,
    ui_input_sink sink, lib_u16 code_unit, lib_u16 repeat_count);
/* Native adapters copy either separate transition/character messages or one
 * combined record. Translate a separate native transition into characters only
 * on UI_KEYBOARD_UNMAPPED; accepted physical keys must not generate a second
 * character record. CHARACTER is independent text, not a physical echo.
 * held_keys is the same matcher used by sink, NULL only for isolated decoding. */
enum { UI_KEYBOARD_REJECTED = 0, UI_KEYBOARD_ACCEPTED = 1, UI_KEYBOARD_UNMAPPED = 2 };
typedef enum ui_keyboard_record_kind {
    UI_KEYBOARD_TRANSITION,
    UI_KEYBOARD_CHARACTER,
    UI_KEYBOARD_COMBINED
} ui_keyboard_record_kind;

typedef struct ui_keyboard_record {
    ui_keyboard_record_kind kind;
    lib_u16 scan;
    lib_u16 key;
    lib_u16 utf16;
    lib_u8 flags;
    lib_u8 modifiers;
    lib_bool pressed;
    /* Zero means one. Decode complete characters before expanding repeats.
     * Physical breaks are single. Surrogate halves must have equal counts. */
    lib_u16 repeat_count;
} ui_keyboard_record;

int ui_keyboard_submit_record(ui_keyboard_normalizer *state,
    const ui_hotkey_matcher *held_keys, void *context,
    ui_input_sink sink, const ui_keyboard_record *record);

#endif
