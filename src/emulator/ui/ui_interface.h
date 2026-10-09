#ifndef EMULATOR_UI_INTERFACE_H
#define EMULATOR_UI_INTERFACE_H

#include "lib/console/console_interface.h"
#include "lib/kvm-base/event_interface.h"
#include "lib/kvm-window/frame_interface.h"
#include "lib/kvm-console/frame_interface.h"
#include "lib/kvm-base/hotkey_interface.h"
#include "lib/types/types_interface.h"

typedef struct emulator_ui emulator_ui;

typedef enum emulator_ui_state {
    EMULATOR_UI_STATE_STOPPED,
    EMULATOR_UI_STATE_RUNNING,
    EMULATOR_UI_STATE_PAUSED,
    EMULATOR_UI_STATE_ERROR
} emulator_ui_state;

typedef enum emulator_ui_action {
    EMULATOR_UI_ACTION_NONE,
    EMULATOR_UI_ACTION_CREATE_WINDOW,
    EMULATOR_UI_ACTION_CREATE_VM_CONSOLE,
    EMULATOR_UI_ACTION_BIND_VM_CONSOLE,
    EMULATOR_UI_ACTION_BIND_MONITOR,
    EMULATOR_UI_ACTION_DESTROY_VM_CONSOLE,
    EMULATOR_UI_ACTION_DESTROY_WINDOW
} emulator_ui_action;

typedef enum emulator_ui_component {
    EMULATOR_UI_COMPONENT_WINDOW,
    EMULATOR_UI_COMPONENT_VM_CONSOLE
} emulator_ui_component;

typedef enum emulator_ui_event_kind {
    EMULATOR_UI_EVENT_KVM_INPUT,
    EMULATOR_UI_EVENT_MONITOR_LINE,
    EMULATOR_UI_EVENT_COMPONENT_COMPLETED,
    EMULATOR_UI_EVENT_BROKER_COMPLETED,
    EMULATOR_UI_EVENT_KVM_DELIVERY_FAILED,
    EMULATOR_UI_EVENT_CONSOLE_FAILED
} emulator_ui_event_kind;

typedef struct emulator_ui_event {
    emulator_ui_event_kind kind;
    lib_u32 run_generation;
    lib_bool monitor_line_rejected;
    union {
        kvm_input_event kvm;
        lib_console_line line;
        struct { emulator_ui_component component; lib_bool exists; } component;
        lib_bool broker_vm_console_current;
        struct { lib_u64 source_identity; lib_status status; } delivery_failure;
    } value;
} emulator_ui_event;

typedef lib_bool (*emulator_ui_event_sink)(void *context, const emulator_ui_event *event);

typedef struct emulator_ui_options {
    void *event_context;
    emulator_ui_event_sink event_sink;
    kvm_hotkey_registry hotkeys;
    const char *running_window_title;
    const char *paused_window_title;
    /* Product-owned text shown by a raw Console while graphics runs beside it. */
    const char *graphics_console_status_text;
} emulator_ui_options;

lib_status emulator_ui_create(emulator_ui **out_ui, const emulator_ui_options *options);
/* Failure retains ui and remaining resources; callback dependencies must stay
 * alive. Treat unjoined workers as terminal, not as a completed destruction. */
lib_status emulator_ui_destroy(emulator_ui *ui);
void emulator_ui_set_run_generation(emulator_ui *ui, lib_u32 run_generation);
lib_status emulator_ui_apply_action(emulator_ui *ui, emulator_ui_action action,
    emulator_ui_state state);
lib_status emulator_ui_set_state(emulator_ui *ui, emulator_ui_state state);
lib_status emulator_ui_publish_frame(emulator_ui *ui, const kvm_window_frame *frame,
    const kvm_console_character_map *characters, lib_u32 sequence,
    lib_bool window_actual, lib_bool vm_console_current,
    lib_bool console_status_surface);
lib_status emulator_ui_release_window_mouse(emulator_ui *ui);
lib_status emulator_ui_write_monitor(emulator_ui *ui, const char *text);
lib_status emulator_ui_request_monitor_line(emulator_ui *ui);
lib_status emulator_ui_cancel_monitor_line(emulator_ui *ui, lib_bool *out_completed);

#endif
