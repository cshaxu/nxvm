#ifndef UI_BASE_HOTKEY_INTERFACE_H
#define UI_BASE_HOTKEY_INTERFACE_H

#include "lib/ui-base/event_interface.h"

#define UI_HOTKEY_CAPACITY 16u
#define UI_HOTKEY_PENDING_CAPACITY 4u
#define UI_HOTKEY_SUPPRESSED_CAPACITY (UI_HOTKEY_PENDING_CAPACITY + 1u)

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

typedef struct ui_hotkey_suppressed_key {
    ui_key key;
    lib_u16 scan_code;
} ui_hotkey_suppressed_key;

typedef struct ui_hotkey_matcher {
    ui_hotkey_registry registry;
    ui_input_event pending[UI_HOTKEY_PENDING_CAPACITY];
    lib_u32 pending_count;
    /* A matched chord suppresses every make and every later break belonging
     * to that chord.  Each physical pending make is retained: key identity
     * alone is not an identity because left/right modifiers share it. */
    ui_hotkey_suppressed_key suppressed_keys[UI_HOTKEY_SUPPRESSED_CAPACITY];
    lib_u32 suppressed_count;
} ui_hotkey_matcher;

void ui_hotkey_registry_initialize(ui_hotkey_registry *registry);
lib_status ui_hotkey_registry_register(ui_hotkey_registry *registry,
    ui_key key, lib_u8 modifiers, const char *identifier);
void ui_hotkey_matcher_initialize(ui_hotkey_matcher *matcher,
    const ui_hotkey_registry *registry);
/* Emits ordinary events and matched UI_EVENT_HOTKEY values through `sink`.
 * A false return means the sink rejected an event; no background retry path
 * exists, so caller owns its component-local failure policy. */
int ui_hotkey_matcher_submit(ui_hotkey_matcher *matcher,
    const ui_input_event *event, ui_input_sink sink, void *context);
void ui_hotkey_matcher_discard(ui_hotkey_matcher *matcher);

#endif
