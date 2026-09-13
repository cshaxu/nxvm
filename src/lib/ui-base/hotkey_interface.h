#ifndef UI_BASE_HOTKEY_INTERFACE_H
#define UI_BASE_HOTKEY_INTERFACE_H

#include "lib/ui-base/event_interface.h"

#define UI_HOTKEY_CAPACITY 16u

enum {
    UI_HOTKEY_MODIFIER_CONTROL = UI_KEY_MODIFIER_CONTROL,
    UI_HOTKEY_MODIFIER_ALT = UI_KEY_MODIFIER_ALT,
    UI_HOTKEY_MODIFIER_SHIFT = UI_KEY_MODIFIER_SHIFT,
    UI_HOTKEY_KEY_CONTROL = UI_KEY_CONTROL,
    UI_HOTKEY_KEY_ALT = UI_KEY_ALT,
    UI_HOTKEY_KEY_SHIFT = UI_KEY_SHIFT
};

typedef struct ui_hotkey_registration {
    ui_key key;
    lib_u8 modifiers;
    char identifier[UI_HOTKEY_IDENTIFIER_CAPACITY];
} ui_hotkey_registration;

typedef struct ui_hotkey_registry {
    ui_hotkey_registration entries[UI_HOTKEY_CAPACITY];
    lib_u32 count;
} ui_hotkey_registry;

typedef enum ui_hotkey_key_state {
    UI_HOTKEY_PENDING,
    UI_HOTKEY_DELIVERED,
    UI_HOTKEY_CONSUMED
} ui_hotkey_key_state;

typedef struct ui_hotkey_held_key {
    ui_input_event make;
    ui_hotkey_key_state state;
    lib_bool allow_replay;
} ui_hotkey_held_key;

typedef struct ui_hotkey_matcher {
    ui_hotkey_registry registry;
    /* In make order; every held physical key has exactly one disposition. */
    ui_hotkey_held_key *held;
    lib_size held_count;
    lib_size held_capacity;
    lib_bool failed;
} ui_hotkey_matcher;

void ui_hotkey_registry_initialize(ui_hotkey_registry *registry);
lib_status ui_hotkey_registry_register(ui_hotkey_registry *registry,
    ui_key key, lib_u8 modifiers, const char *identifier);
void ui_hotkey_matcher_initialize(ui_hotkey_matcher *matcher,
    const ui_hotkey_registry *registry);
/* Emits ordinary events and matched UI_EVENT_HOTKEY values through sink.
 * Repeats retain their original disposition; delivered makes retain breaks.
 * Failure is terminal until discard; no partial replay is retried.
 * allow_replay is captured on the first make, not refreshed by repeats.
 * False suppresses only delayed ordinary make replay, never chord matching
 * or later releases. Initialize once; discard releases held storage. */
int ui_hotkey_matcher_submit(ui_hotkey_matcher *matcher,
    const ui_input_event *event, ui_input_sink sink, void *context,
    lib_bool allow_replay);
void ui_hotkey_matcher_discard(ui_hotkey_matcher *matcher);
/* Borrowed until the next submit/discard. Includes either physical side of a
 * modifier; synthesis must not release a key already owned by this ledger. */
const ui_input_event *ui_hotkey_matcher_held_key(
    const ui_hotkey_matcher *matcher, ui_key key);

#endif
