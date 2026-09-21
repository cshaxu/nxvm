#ifndef COMMON_UI_INTERFACE_H
#define COMMON_UI_INTERFACE_H

#include "lib/console/console_interface.h"
#include "lib/kvm-base/event_interface.h"
#include "lib/kvm-window/frame_interface.h"
#include "lib/kvm-console/frame_interface.h"
#include "lib/kvm-base/hotkey_interface.h"
#include "lib/types/types_interface.h"

typedef struct common_ui common_ui;

typedef enum common_ui_state {
    COMMON_UI_STATE_STOPPED,
    COMMON_UI_STATE_RUNNING,
    COMMON_UI_STATE_PAUSED,
    COMMON_UI_STATE_ERROR
} common_ui_state;

typedef enum common_ui_action {
    COMMON_UI_ACTION_NONE,
    COMMON_UI_ACTION_CREATE_WINDOW,
    COMMON_UI_ACTION_CREATE_VM_CONSOLE,
    COMMON_UI_ACTION_BIND_VM_CONSOLE,
    COMMON_UI_ACTION_BIND_MONITOR,
    COMMON_UI_ACTION_DESTROY_VM_CONSOLE,
    COMMON_UI_ACTION_DESTROY_WINDOW
} common_ui_action;

typedef enum common_ui_component {
    COMMON_UI_COMPONENT_WINDOW,
    COMMON_UI_COMPONENT_VM_CONSOLE
} common_ui_component;

typedef enum common_ui_event_kind {
    COMMON_UI_EVENT_KVM_INPUT,
    COMMON_UI_EVENT_MONITOR_LINE,
    COMMON_UI_EVENT_COMPONENT_COMPLETED,
    COMMON_UI_EVENT_BROKER_COMPLETED,
    COMMON_UI_EVENT_KVM_DELIVERY_FAILED,
    COMMON_UI_EVENT_CONSOLE_FAILED
} common_ui_event_kind;

typedef struct common_ui_event {
    common_ui_event_kind kind;
    lib_u32 run_generation;
    lib_bool monitor_line_rejected;
    union {
        kvm_input_event kvm;
        lib_console_line line;
        struct { common_ui_component component; lib_bool exists; } component;
        lib_bool broker_vm_console_current;
        struct { lib_u64 source_identity; lib_status status; } delivery_failure;
    } value;
} common_ui_event;

typedef int (*common_ui_event_sink)(void *context, const common_ui_event *event);

typedef struct common_ui_options {
    void *event_context;
    common_ui_event_sink event_sink;
    kvm_hotkey_registry hotkeys;
    const char *running_window_title;
    const char *paused_window_title;
    /* Product-owned text shown by a raw Console while graphics runs beside it. */
    const char *graphics_console_status_text;
} common_ui_options;

lib_status common_ui_create(common_ui **out_ui, const common_ui_options *options);
/* Failure retains ui and remaining resources; callback dependencies must stay
 * alive. Treat unjoined workers as terminal, not as a completed destruction. */
lib_status common_ui_destroy(common_ui *ui);
void common_ui_set_run_generation(common_ui *ui, lib_u32 run_generation);
lib_status common_ui_apply_action(common_ui *ui, common_ui_action action,
    common_ui_state state);
lib_status common_ui_set_state(common_ui *ui, common_ui_state state);
lib_status common_ui_publish_frame(common_ui *ui, const kvm_window_frame *frame,
    const kvm_console_character_map *characters, lib_u32 sequence,
    lib_bool window_actual, lib_bool vm_console_current,
    lib_bool console_status_surface);
lib_status common_ui_release_window_mouse(common_ui *ui);
lib_status common_ui_write_monitor(common_ui *ui, const char *text);
lib_status common_ui_request_monitor_line(common_ui *ui);
lib_status common_ui_cancel_monitor_line(common_ui *ui, lib_bool *out_completed);

#endif
