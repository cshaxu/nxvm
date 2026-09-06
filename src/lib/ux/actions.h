#ifndef UX_ACTIONS_H
#define UX_ACTIONS_H

#include "lib/base/base.h"

/* The value belongs to the product registration.  The library only matches a
 * chord, releases host input, and invokes the registered product callback. */
typedef lib_u32 ux_action;

enum {
    UX_ACTION_NONE = 0u,
    UX_MODIFIER_CONTROL = 0x01u,
    UX_MODIFIER_ALT = 0x02u,
    UX_MODIFIER_SHIFT = 0x04u,
    UX_ACTION_CAPACITY = 16u,
    /* Cross-native key identities used only for registered host actions. */
    UX_KEY_F1 = 0x00010001u,
    UX_KEY_F2 = 0x00010002u,
    UX_KEY_F3 = 0x00010003u,
    UX_KEY_F4 = 0x00010004u,
    UX_KEY_F5 = 0x00010005u,
    UX_KEY_F6 = 0x00010006u,
    UX_KEY_F7 = 0x00010007u,
    UX_KEY_F8 = 0x00010008u,
    UX_KEY_F9 = 0x00010009u,
    UX_KEY_F10 = 0x0001000au,
    UX_KEY_F11 = 0x0001000bu,
    UX_KEY_F12 = 0x0001000cu
};

typedef struct ux_action_chord {
    lib_u32 key;
    lib_u8 modifiers;
    ux_action action;
} ux_action_chord;

typedef struct ux_action_registry {
    ux_action_chord entries[UX_ACTION_CAPACITY];
    lib_u32 count;
} ux_action_registry;

void ux_actions_initialize(ux_action_registry *registry);
lib_status ux_actions_register(ux_action_registry *registry,
    lib_u32 key, lib_u8 modifiers, ux_action action);
ux_action ux_actions_match(const ux_action_registry *registry,
    lib_u32 key, lib_u8 modifiers);

#endif
